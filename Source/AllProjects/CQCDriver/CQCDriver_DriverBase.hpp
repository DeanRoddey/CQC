//
// FILE NAME: CQCDriver_DriverBase.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/22/2001
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2020
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and
//  the author (Dean Roddey.) It is licensed under the MIT Open Source
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This is the base class for all server side device drivers. The way this
//  class works ties in pretty heavily with CQCServer, which hosts them, so
//  see the CQCServer facility class header comments for more info. The same
//  thing applies to any driver IDE (or other driver hosting app.)
//
//  Drivers run on their own thread. That thread's primary job is to take
//  the driver up through a set of states until it's connected to its device,
//  at which point it's exposed to clients and providing field data and
//  accepting commands. The steps include initalizing, getting any config
//  info loaded that's required, getting connected to the device (mostly getting
//  initial field data), and then it is connected and starts regular polling
//  of the device.
//
//  This class provides a set of callbacks that derived driver classes override
//  in order to provide driver specific functionality. There's one associated
//  with most of the driver states (the responsibility of which is to get it
//  to the next state), plus a couple other special case ones. See the driver
//  development technical docs for more info.
//
//  This class supports two types of calls:
//
//  1.  Simple calls that just get data already stored in this class. These
//      are protected by mutex member of each driver. None of them lock for
//      more than fractions of a second at most. These are called directly
//      by remote client threads (by way of CQCServer.)
//  2.  Calls that would cause a call to one of the driver callback methods.
//      These are not protected by the mutex or called directly by remote
//      client threads, because they could take some time (on the order of
//      seconds) to complete.
//
//  The #2 type calls are not called directly by clients, they are only
//  called by the driver thread. They are queued up and the driver thread
//  handles them as it is able. We provide a cmd class for queuing up these
//  commands, and a pool from which all drivers temporarily obtain these
//  commands to queue. Clients can choose to wait for the command to be
//  completed or just return after queing it up. Commands in the queue can
//  be udpated by another thread making the same call. We update the ref
//  count (if the new client is going to wait), and the data. If the new
//  client waits, then multiple client threads are blocked on the command.
//
//  This means that derived classes don't need to synchronzie access to any
//  of their data since only the driver thread ever accesses it. If they call
//  down into this class, those calls are type #1 calls as well, and are
//  protected by the mutex just as in calls from clients to those methods. The
//  only exception is if the driver provides a direct interface of some other
//  type that would allow independent calls into the driver, but this is
//  discouraged. Use the standard backdoor commands where possible.
//
//  We have two queues, one for special commands and one for normal commands.
//  The special commands are ones that are going to cause the driver to
//  recycle or stop processing. If we get one of these, we want to just
//  stop processing regular commands, because there's no need to do any
//  of the ones not yet processed. When doing normal command processing, if
//  we see anything in the special commands queue, we stop and go back to the
//  main loop which will see the special command (erroring out any queued up
//  regular commands.)
//
//  Locking:
//
//  Though it would be nice to have a single mutex, the two queues are
//  separately thread safe. This does of course mean potential for deadlocks
//  but it's unlikely. The two locks are not really ever used at the same
//  time, though we need to be careful of course.
//
//  So we don't have to keep the mutex locked just to queue up or release cmds
//  , or for the driver thread to wait for commands to show up in the queue. This
//  is necessary for fast reaction to incoming commands. We need to block on the
//  queues to wait for a command, but we can't keep the mutex locked while we do
//  that. All of the public methods that queue up commands just lock the appropriate
//  queue and that's all that's required. Once the command is out of the queue,
//  clients can't see it and any clients waiting for it will block until the driver
//  completes the command. To release commands only the command itself and the
//  (global) pool are involved, so the driver doesn't have to be locked.
//
//  If we replace commands in the queue, potentially linking more than one
//  client thread to the same command, we have to update the value and
//  possibly the ref count if the new client is going to wait. This is only
//  done while the queue is locked and while the command is still in the
//  queue, so the driver can't be processing it, and multiple clients can't
//  do this at once because the queue is locked while we update it. If the
//  driver sets error info or return info, that's done while the object is
//  not in the queue anymore, so only any waiting clients reference it and
//  they are blocked waiting on it to complete.
//
//  For releasing cnds , and keeping the ref count right, that's all done
//  though the ReleaseCmd() method, which is a static method shared by all
//  drivers. We just use the commmand pool's mutex to protect that since
//  we are going to also need to release the command back to the pool again
//  anyway, and the whole process if very short.
//
//  Driver Shutdown:
//
//  To stop a driver we do a thread shutdown request on the driver's thread
//  object. This will allow control to return immediately to the caller,
//  but any CIDLib based code that blocks watches for the thread shutdown
//  flag and will back out. Our main driver thread loop does as well, and
//  will start the thread shutdown process.
//
//  Doing it this way, instead of as a queued command, means that all code
//  running in the context of the driver thread will see it immediately,
//  instead of having to wait for whatever is going on to complete. It also
//  allows for parallel shutdown of threads when CQCServer stops.
//
//  Any queued up commands will be released with an error return, so the
//  clients will wake up and return errors to their respective clients.
//
//  Threading Issues:
//
//  Normally we start our own thread, which means that any code inside the driver
//  that is only called by driver code can assume that our thread obj member is
//  their thread. However, we can also be run from an IDE on an external thread.
//  To deal with that we use a pointer for the thread member. If our own start
//  method is called we allocate our own thread and set the member, and remember
//  we created it. If we get to our thread method and our pointer is not set, then
//  clearly we got called on an external thread, so we point our pointer at that,
//  and remember we didn't create it.
//
//  This way, we can can consistently internally use our thread member for purely
//  driver invoked code, and know we are returning status information for the right
//  thread in those methods we provide that allow our thread status to be accessed.
//  In our dtor, if we created the thread, we clean it up.
//
//  Name Server Alias:
//
//  CQCServer hosts drivers and provides the ORB interface by which its drivers are
//  managed by clients, and by which clients interact with drivers. Each CQCServer
//  registers a name server entry for its management interface.
//
//  However, most access to drivers is via the moniker, and there is also a need to
//  quickly find out what drivers are available and what hosts they are on and some
//  other high level info. So, each driver also registers an entry in the name server
//  but it registers the same OOID. So all of them ultimately point back to the CQC
//  Server host that handles that driver.
//
//  The extra info that can be associted with name server entries allows us to put
//  some user information into the name server entry. So clients can find out the most
//  important stuff about drivers purely by enumerating the scope in the name server
//  where drivers register these aliases. We store the make/model info (the unique
//  key for a driver), the manifest name, the device category, and any V2 device
//  classes it implements.
//
//  CQCServer will call SetSrvAdminOId() with the object id it wants us to register
//  under our alias. NOTE THAT this also allows the C++ test harness to load both
//  client and driver side servers into itself, and to give us an OOID for a faux
//  admin interface it creates. So the client driver, and any other stuff that talks to
//  the server side driver, will get redirected to the test harness' driver.
//
//  THIS MEANS that the test hardness can't load a driver that is currently loaded
//  in CQCServer because it would override that driver's aliasn entry. So it checks
//  first and makes you unload the driver.
//
// CAVEATS/GOTCHAS:
//
//  1)  The driver MUST NEVER lock the sync mutex across any call out to the device
//      or anything that might block, even for a short time. It must ONLY lock long
//      enough to update fields or driver state, then unlock again, so that clients
//      can get to state info at any time. This is easy enough these days since the
//      driver runs on a single thread and incoming calls from clients are queued up
//      for processing. So while one cmd is being processed, no other commands can
//      be processed. Reading field data doesn't require a queued up cmd, just a lock
//      to make sure the field data isn't changing and a read out of the data.
//
//  2)  Setting verbosity level is one we don't do as a queued command, even
//      though it causes a call to the derived driver (potentially if they
//      overrode that call.) In this case, we just keep a current and new
//      verbosity. Client calls to set it just update the new one, and if the
//      driver thread sees a different it calls the callback and copies the new
//      one to the current one. There's no need for clients ever to wait for this
//      cmd, and it doesn't fit cleanly into either the regular or special cmd types.
//
//  3)  Note that we currently don't allow client threads to time out before
//      the driver completes the command. So a thread cannot release the cmd
//      before the driver is done with it. And the driver has already reduced
//      the ref count before it posts the cmd's completion event. If we later
//      allow clients to do this, we will have to adjust for that.
//
//  4)  Drivers can be paused, and as of 4.5.9 that is persisted. It's stored in
//      the driver config info. When the driver is reloaded later, CQC server always
//      calls the ctor that takes the config object, so we set our initial state
//      to Paused if we were last paused. If it ever becomes possible that CQCServer
//      would call the other ctor, then it would have to get our paused state to
//      us some other way.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


class TSrvDrvCmdPool;


// ---------------------------------------------------------------------------
//   CLASS: TCQCServerBase
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class CQCDRIVEREXPORT TCQCServerBase : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TQueryFieldList = tCQCKit::TFldDefList;
        using TSetFieldList = TVector<TCQCFldDef>;

        using TFieldNameList = TRefKeyedHashSet<TCQCFldStore,TString,TStringKeyOps>;
        using TFieldIdList = TRefVector<TCQCFldStore>;

        // Used to manage queued timed field changes
        struct TTimedFldChange
        {
            TTimedFldChange() = delete;
            TTimedFldChange(const   TString&                strField
                            , const TString&                strFinalVal
                            , const tCIDLib::TEncodedTime   enctWaitTill) :

                m_enctWaitTill(enctWaitTill)
                , m_strField(strField)
                , m_strFinalValue(strFinalVal)
            {
            }
            TTimedFldChange(const TTimedFldChange&) = default;
            TTimedFldChange(TTimedFldChange&&) = default;

            TTimedFldChange& operator=(const TTimedFldChange&) = default;
            TTimedFldChange& operator=(TTimedFldChange&&) = default;

            tCIDLib::TEncodedTime m_enctWaitTill;
            TString               m_strField;
            TString               m_strFinalValue;
        };
        using TTimedChgList = TVector<TCQCServerBase::TTimedFldChange>;

        // The commands we can queue up
        enum class EDrvCmds
        {
            None

            // General control commands
            , Pause
            , Reconfig
            , Resume

            // Field write operations
            , WrtBoolById
            , WrtBoolByName
            , WrtCardById
            , WrtCardByName
            , WrtFieldById
            , WrtFieldByName
            , WrtFloatById
            , WrtFloatByName
            , WrtIntById
            , WrtIntByName
            , WrtStrById
            , WrtStrByName
            , WrtStrListById
            , WrtStrListByName
            , WrtTimeById
            , WrtTimeByName

            // Some miscellaneous ones
            , TimedFldWrt

            // Backdoor commands
            , QueryBoolVal
            , QueryCardVal
            , QueryConfig
            , QueryData
            , QueryIntVal
            , QueryTextVal
            , SendCmd
            , SendData
            , SetConfig
        };
        class TDrvCmd
        {
            public :
                static tCIDLib::TBoolean bIsASpecialCmd
                (
                    const   EDrvCmds            eToCheck
                );

                static tCIDLib::ESortComps eComp
                (
                    const   TDrvCmd&            dcmd1
                    , const TDrvCmd&            dcmd2
                    , const tCIDLib::TCard4     c4UserData
                );

                static tCIDLib::ESortComps eKeyComp
                (
                    const   tCIDLib::TCard4     c4Val
                    , const TDrvCmd&            dcmdComp
                    , const tCIDLib::TCard4     c4UserData
                );

                TDrvCmd() = delete;
                TDrvCmd(const tCIDLib::TCard4 c4BufSz);
                TDrvCmd(const TDrvCmd&) = delete;
                virtual ~TDrvCmd();
                TDrvCmd& operator=(const TDrvCmd&) = delete;
                TDrvCmd& operator=(TDrvCmd&&) = delete;

                tCIDLib::TBoolean bIsSpecial() const
                {
                    // Just pass to the static one
                    return bIsASpecialCmd(m_eCmd);
                }

                tCIDLib::TVoid Reset
                (
                    const   tCIDLib::TCard4     c4MinBufSz
                );

                tCIDLib::TVoid SetError
                (
                    const   TError&             errRet
                );


                // The command to perform
                EDrvCmds                m_eCmd;

                // The wait event for clients to wait on
                TEvent                  m_evWait;

                //
                //  This is used to know when to release the command
                //  back to the pool. When it goes zero it can be
                //  released.
                //
                tCIDLib::TCard4         m_c4RefCnt;

                // This is set if an exception occurred
                tCIDLib::TBoolean       m_bError;
                TError                  m_errReturn;

                //
                //  The moniker on whom the command is queued. It could go
                //  away while we wait, so we need to store it for later
                //  error msgs and such.
                //
                TString                 m_strMoniker;

                //
                //  The various values that we might need to get info in or
                //  out.
                //
                tCIDLib::TBoolean       m_bVal;
                tCIDLib::TCard4         m_c4Count;
                tCIDLib::TCard4         m_c4Id1;
                tCIDLib::TCard4         m_c4Id2;
                tCIDLib::TCard4         m_c4Val;
                tCIDLib::TStrList       m_colStrList;
                tCIDLib::TEncodedTime   m_enctVal;
                THeapBuf                m_mbufVal;
                tCIDLib::TFloat8        m_f8Val;
                tCIDLib::TInt4          m_i4Val;
                TCQCDriverObjCfg*       m_pcqcdcNew;
                TString                 m_strName;
                TString                 m_strParams;
                TString                 m_strType;
                TString                 m_strVal;
        };
        using TCmdQ = TRefQueue<TDrvCmd>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bWaitCmd
        (
                    TDrvCmd* const          pdcmdWaitFor
            ,       TError&                 errToFill
            , const tCIDLib::TBoolean       bRelease
        );

        static TCQCServerBase::TDrvCmd* pdcmdReserve
        (
            const   tCIDLib::TCard4         c4Size
            , const TString&                strMoniker
        );

        static tCIDLib::TVoid ReleaseCmd
        (
                    TDrvCmd* const          pdcmdRel
            , const tCIDLib::TBoolean       bFromDrv
        );



        // -------------------------------------------------------------------
        //  Constructors Destructor
        // -------------------------------------------------------------------
        TCQCServerBase(const TCQCServerBase&) = delete;
        TCQCServerBase(TCQCServerBase&&) = delete;

        ~TCQCServerBase();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCServerBase& operator=(const TCQCServerBase&) = delete;
        TCQCServerBase& operator=(TCQCServerBase&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        );

        virtual tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQueryType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        );

        virtual tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        virtual tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strValId
        );

        virtual tCIDLib::TBoolean bQueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        virtual tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufToSend
        );

        virtual tCIDLib::TCard4 c4QueryCfg
        (
                    THeapBuf&               mbufToFill
        );

        virtual tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        virtual tCIDLib::TCard4 c4QueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        virtual tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        );

        virtual tCIDLib::TInt4 i4QueryVal
        (
            const   TString&                strValId
        );

        virtual tCIDLib::TInt4 i4QueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        virtual tCIDLib::TVoid SetConfig
        (
            const   tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckFldListChanged
        (
            const   tCIDLib::TCard4         c4ToCheck
        )   const;

        tCIDLib::TBoolean bDeleteCfgData();

        tCIDLib::TBoolean bFieldInErr
        (
            const   tCIDLib::TCard4         c4Id
        );

        tCIDLib::TBoolean bFieldExists
        (
            const   TString&                strToFind
        )   const;

        tCIDLib::TBoolean bGetASCIITermMsg
        (
                    TObject&                objSrc
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1Term1
            , const tCIDLib::TCard1         c1Term2
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bGetASCIITermMsg2
        (
                    TObject&                objSrc
            , const tCIDLib::TEncodedTime   enctEndTime
            , const tCIDLib::TCard1         c1Term1
            , const tCIDLib::TCard1         c1Term2
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bGetASCIIStartStopMsg
        (
                    TObject&                objSrc
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard1         c1StopByte
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bGetASCIIStartStopMsg2
        (
                    TObject&                objSrc
            , const tCIDLib::TEncodedTime   enctEndTime
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard1         c1StopByte
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bHasQueuedTimedWrite
        (
            const   TString&                strField
            ,       tCIDLib::TCard8&        c8EndTime
        );

        tCIDLib::TBoolean bImplementsClass
        (
            const   tCQCKit::EDevClasses    eClass
            , const tCIDLib::TBoolean       bThrowIfNotV2
        )   const;

        tCIDLib::TBoolean bIncUsageCount();

        tCIDLib::TBoolean bIsAsyncMode() const;

        tCIDLib::TBoolean bIsValidFldId
        (
            const   tCIDLib::TCard4         c4ToCheck
        )   const;

        tCIDLib::TBoolean bQueryCfgData
        (
                    MStreamable&            mstrmToFill
        );

        tCIDLib::TBoolean bQueryChangedFlds
        (
            const   tCIDLib::TCard4         c4FldListId
            ,       tCIDLib::TCardArray&    fcolSerialNums
            ,       tCIDLib::TBoolArray&    fcolChanged
            ,       tCIDLib::TBoolArray&    fcolErrors
            ,       tCIDLib::TStrList&      colValues
            ,       tCIDLib::TBoolean&      bResync
        );

        tCIDLib::TBoolean bQueryFldId
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4Id
            ,       tCQCKit::EFldTypes&     eType
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCIDLib::TBoolean bQueryFldId
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4Id
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCIDLib::TBoolean bQueryFldName
        (
            const   tCIDLib::TCard4         c4Id
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bQueryFldTrigger
        (
            const   TString&                strFldName
            ,       TCQCFldEvTrigger&       fetToFill
            ,       TCQCFldDef&             flddToFill
        )   const;

        tCIDLib::TBoolean bQueryFldTriggers
        (
                    tCIDLib::TStrList&      colToFill
        )   const;

        tCIDLib::TBoolean bReadBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TBoolean&      bToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadBoolFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TBoolean&      bToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadCardFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strValue
            ,       tCQCKit::EFldTypes&     eType
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadField
        (
            const   tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       TMemBuf&                mbufData
            ,       tCQCKit::EFldTypes&     eType
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadFieldByName
        (
            const   TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strValue
            ,       tCQCKit::EFldTypes&     eType
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadFieldByName
        (
            const   TString&                strFldName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard4&        c4RetBytes
            ,       TMemBuf&                mbufData
            ,       tCQCKit::EFldTypes&     eType
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TFloat8&       f8ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadFloatFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TFloat8&       f8ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TInt4&         i4ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadIntFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TInt4&         i4ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadStringFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadStringFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       TString&                strToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadSListFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadSListFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TStrList&      colToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadTimeFld
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard8&        c8ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bReadTimeFld
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TCard8&        c8ToFill
            , const tCIDLib::TBoolean       bCheckOnline = kCIDLib::True
        );

        tCIDLib::TBoolean bSetCfgData
        (
            const   MStreamable&            mstrmToSet
            , const tCIDLib::TCard4         c4Extra = 64
        );

        tCIDLib::TBoolean bStoreBoolFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TBoolean       bToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreBoolFld
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreCardFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard4         c4ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreCardFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreFldValue
        (
            const   tCIDLib::TCard4         c4Id
            , const TCQCFldValue&           fldvToStore
        );

        tCIDLib::TBoolean bStoreFloatFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TFloat8        f8ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreFloatFld
        (
            const   TString&                strName
            , const tCIDLib::TFloat8        f8ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreIntFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TInt4          i4ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreIntFld
        (
            const   TString&                strName
            , const tCIDLib::TInt4          i4ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreStringFld
        (
            const   tCIDLib::TCard4         c4Id
            , const TString&                strToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreStringFld
        (
            const   TString&                strName
            , const TString&                strToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreSListFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TStrList&      colToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreSListFld
        (
            const   TString&                strName
            , const tCIDLib::TStrList&      colToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreTimeFld
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TCard8&        c8ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bStoreTimeFld
        (
            const   TString&                strName
            , const tCIDLib::TCard8&        c8ToSet
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TBoolean bWaitTillDead
        (
            const   tCIDLib::TCard4         c4Millis
        )   const;

        tCIDLib::TCard4 c4ArchVersion() const;

        tCIDLib::TCard4 c4FieldListId() const
        {
            return m_c4FieldListId;
        }

        tCIDLib::TCard4 c4GetStartLenMsg
        (
                    TObject&                objSrc
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard4         c4LenOfs
            , const tCIDLib::TCard4         c4LenSub
            , const tCIDLib::TCard4         c4TrailBytes
            , const tCIDLib::TCard4         c4MaxBytes
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4GetStartLenMsg2
        (
                    TObject&                objSrc
            , const tCIDLib::TEncodedTime   enctEndTime
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard4         c4LenOfs
            , const tCIDLib::TCard4         c4LenSub
            , const tCIDLib::TCard4         c4TrailBytes
            , const tCIDLib::TCard4         c4MaxBytes
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4GetStartStopMsg
        (
                    TObject&                objSrc
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard1         c1StopByte
            , const tCIDLib::TCard4         c4MaxBytes
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4GetStartStopMsg2
        (
                    TObject&                objSrc
            , const tCIDLib::TEncodedTime   enctEndTime
            , const tCIDLib::TCard1         c1StartByte
            , const tCIDLib::TCard1         c1StopByte
            , const tCIDLib::TCard4         c4MaxBytes
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4GetTermedMsg
        (
                    TObject&                objSrc
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1Term1
            , const tCIDLib::TCard1         c1Term2
            , const tCIDLib::TBoolean       bTwoTerms
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4GetTermedMsg2
        (
                    TObject&                objSrc
            , const tCIDLib::TEncodedTime   enctEndTime
            , const tCIDLib::TCard1         c1Term1
            , const tCIDLib::TCard1         c1Term2
            , const tCIDLib::TBoolean       bTwoTerms
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard4 c4QueryFields
        (
                    TQueryFieldList&        colToFill
            ,       tCIDLib::TCard4&        c4ListIdToFill
        )   const;

        tCIDLib::TCard4 c4QueryFieldNames
        (
                    TVector<TString>&       colToFill
            , const tCQCKit::EReqAccess     eAccess
        );

        tCIDLib::TCard4 c4QueryFieldNamesRX
        (
                    TVector<TCQCFldDef>&    colToFill
            , const TString&                strNameRegEx
            , const tCQCKit::EReqAccess     eAccess
        );

        tCIDLib::TCard4 c4QuerySemFields
        (
            const   tCQCKit::TFldSTypeList& fcolTypes
            , const tCQCKit::EReqAccess     eReqAccess
            ,       TQueryFieldList&        colToFill
        )   const;

        tCIDLib::TCard4 c4ThreadId() const;

        const TCQCUserCtx& cuctxDrv() const;

        tCIDLib::TVoid CancelTimedWrite
        (
            const   TString&                strField
        );

        tCIDLib::TVoid ClearStats();

        tCIDLib::TEncodedTime enctExtend() const;

        tCIDLib::TEncodedTime enctExtend
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCIDLib::EExitCodes eDevPollThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::EFldTypes eFldTypeById
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCQCKit::EFldTypes eFldTypeByName
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4FldId
            , const tCIDLib::TBoolean       bThrowIfNot
        )   const;

        tCQCKit::EDrvInitRes eInitialize();

        tCQCKit::EDrvStates eState() const;

        tCQCKit::EVerboseLvls eVerboseLevel() const;

        const TCQCFldDef& flddFind
        (
            const   tCQCKit::EDevClasses    eClass
            , const TString&                strName
        )   const;

        const TCQCFldDef& flddFind
        (
            const   tCQCKit::EDevClasses    eClass
            , const TString&                strSubUnit
            , const TString&                strName
        )   const;

        const TCQCFldDef& flddFind
        (
            const   tCQCKit::EDevClasses    eClass
            , const TString&                strPrefix
            , const TString&                strName
            , const TString&                strSuffix
        )   const;

        const TCQCFldDef& flddFind
        (
            const   tCQCKit::EDevClasses    eClass
            , const tCIDLib::TCh* const     pszPrefix
            , const TString&                strName
            , const tCIDLib::TCh* const     pszSuffix
        )   const;

        const TCQCFldDef& flddFromId
        (
            const   tCIDLib::TCard4         c4Id
        )   const;

        tCIDLib::TVoid FormatStrListFld
        (
            const   tCIDLib::TCard4         c4Id
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid IncBadMsgCounter();

        tCIDLib::TVoid IncFailedWriteCounter();

        tCIDLib::TVoid IncNakCounter();

        tCIDLib::TVoid IncReconfigCounter();

        tCIDLib::TVoid IncTimeoutCounter();

        tCIDLib::TVoid IncUnknownMsgCounter();

        tCIDLib::TVoid IncUnknownWriteCounter();

        TDrvCmd* pdcmdQBoolFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQCardFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQFloatFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQIntFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQStringFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQSListFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TStrList&      colToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQTimeFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8&        c8ToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQWriteFld
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQQueryBoolVal
        (
            const   TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bTwoVer
        );

        TDrvCmd* pdcmdQQueryCardVal
        (
            const   TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bTwoVer
        );

        TDrvCmd* pdcmdQQueryConfig();

        TDrvCmd* pdcmdQQueryData
        (
            const   TString&                strType
            , const TString&                strName
            , const tCIDLib::TCard4&        c4Bytes
            , const THeapBuf&               mbufData
            , const tCIDLib::TBoolean       bTwoVer
        );

        TDrvCmd* pdcmdQQueryIntVal
        (
            const   TString&                strValId
            , const TString&                strValName
            , const tCIDLib::TBoolean       bTwoVer
        );

        TDrvCmd* pdcmdQQueryTextVal
        (
            const   TString&                strValId
            , const TString&                strValName
        );

        TDrvCmd* pdcmdQSendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQSendData
        (
            const   TString&                strType
            , const TString&                strName
            , const tCIDLib::TCard4&        c4Bytes
            , const THeapBuf&               mbufData
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQPauseResumeCmd
        (
            const   tCIDLib::TBoolean       bPause
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQReconfig
        (
            const   TCQCDriverObjCfg&       cqcdcToSet
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQSetConfig
        (
            const   tCIDLib::TCard4&        c4Bytes
            , const THeapBuf&               mbufCfg
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        TDrvCmd* pdcmdQTimedFldWrt
        (
            const   TString&                strField
            , const TString&                strNewValue
            , const TString&                strEndValue
            , const tCIDLib::TCard4         c4Minutes
            , const tCQCKit::EDrvCmdWaits   eWait
        );

        const TCQCFldDef* pflddFind
        (
            const   TString&                strNameToFind
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        const TCQCFldLimit* pfldlLimsFor
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        const TCQCFldLimit* pfldlLimsFor
        (
            const   tCIDLib::TCard4         c4FldId
            , const TClass&                 clsType
        )   const;

        tCIDLib::TVoid QueueEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        );

        tCIDLib::TVoid QueryDrvConfigObj
        (
                    TCQCDriverObjCfg&       cqcdcToFill
        )   const;

        tCIDLib::TVoid QueryDrvInfo
        (
                    TString&                strMoniker
            ,       TString&                strMake
            ,       TString&                strModel
            ,       tCIDLib::TCard4&        c4MajVersion
            ,       tCIDLib::TCard4&        c4MinVersion
            ,       tCQCKit::EDevCats&      eDevCat
        )   const;

        tCIDLib::TVoid QueryDrvInfo
        (
                    tCQCKit::EDrvStates&    eStatus
            ,       tCIDLib::TCard4&        c4ArchVer
            ,       tCQCKit::TDevClassList& fcolDevClasses
            ,       TString&                strMake
            ,       TString&                strModel
        )   const;

        tCIDLib::TVoid QueryFldDef
        (
            const   TString&                strFldName
            ,       TCQCFldDef&             flddToFill
        )   const;

        tCIDLib::TVoid QueryFldInfo
        (
            const   TString&                strFldName
            ,       tCIDLib::TCard4&        c4FieldListId
            ,       tCIDLib::TCard4&        c4FieldId
            ,       tCQCKit::EFldTypes&     eType
        )   const;

        tCIDLib::TVoid QueryFldName
        (
            const   tCIDLib::TCard4         c4Id
            ,       tCQCKit::EFldTypes&     eType
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid QueryState
        (
                    tCQCKit::EDrvStates&    eStatus
            ,       tCQCKit::EVerboseLvls&  eVerbose
            ,       tCIDLib::TCard4&        c4ThreadId
        )   const;

        tCIDLib::TVoid QueryStats
        (
                    TCQCDrvStats&           cdstatsToFill
        )   const;

        tCIDLib::TVoid RemoveAllTriggers();

        tCIDLib::TVoid RemoveTrigger
        (
            const   TString&                strFldName
        );

        const TString& strMoniker() const
        {
            return m_strMoniker;
        }

        tCIDLib::TVoid SetAllErrStates();

        tCIDLib::TVoid SetFieldErr
        (
            const   tCIDLib::TCard4         c4Id
            , const tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetNewVerbosity
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid SetPollTimes
        (
            const   tCIDLib::TCard4         c4PollTime
            , const tCIDLib::TCard4         c4ReconnTime
        );

        tCIDLib::TVoid SetSrvAdminOId
        (
            const   TOrbObjId&              ooidToSet
        );

        tCIDLib::TVoid SetTriggers
        (
            const   TCQCDriverObjCfg::TTriggerList& colToSet
        );

        tCIDLib::TVoid SetUC
        (
            const   TCQCUserCtx&            cuctxToSet
        );

        tCIDLib::TVoid StartPoll();

        tCIDLib::TVoid StartShutdown();

        tCIDLib::TVoid StreamValue
        (
            const   tCIDLib::TCard4         c4FieldId
            , const tCIDLib::TCard4         c4SerialNum
            ,       TBinOutStream&          strmOut
        )   const;

        tCIDLib::TVoid StreamValue
        (
            const   TString&                strName
            ,       TBinOutStream&          strmOut
        )   const;

        tCIDLib::TVoid UseDefFldValue
        (
            const   tCIDLib::TCard4         c4FldId
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructor
        // -------------------------------------------------------------------
        TCQCServerBase();

        TCQCServerBase
        (
            const   TCQCDriverObjCfg&       cqcdcInfo
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        ) = 0;

        virtual tCIDLib::TBoolean bProbeDevice
        (
            const   TCQCDriverObjCfg&       cqcdcInfo
        );

        virtual tCIDLib::TBoolean bShouldLog
        (
            const   TError&                 errToCatch
            , const tCQCKit::EVerboseLvls   eMinLevel
        )   const;

        virtual tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        ) = 0;

        virtual tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        virtual tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        virtual tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        ) = 0;

        virtual tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        );

        virtual tCQCKit::EDrvInitRes eInitializeImpl() = 0;

        virtual tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

        virtual tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        ) = 0;

        virtual tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        virtual tCQCKit::ECommResults eSListFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TStrList&      colNewValue
        );

        virtual tCQCKit::ECommResults eTimeFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard8&        c8NewValue
        );

        virtual tCIDLib::TVoid Idle();

        virtual tCIDLib::TVoid LogError
        (
            const   TError&                 errToCatch
            , const tCQCKit::EVerboseLvls   eMinLevel
        )   const;

        virtual tCIDLib::TVoid LogError
        (
                    TError&                 errToCatch
            , const tCQCKit::EVerboseLvls   eMinLevel
            , const tCIDLib::TCh* const     pszSrcFile
            , const tCIDLib::TCard4         c4SrcLine
        )   const;

        virtual tCIDLib::TVoid ReleaseCommResource() = 0;

        virtual tCIDLib::TVoid TerminateImpl() = 0;

        virtual tCIDLib::TVoid VerboseModeChanged
        (
            const   tCQCKit::EVerboseLvls   eNewState
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        //
        //  These are just helper methods for all of the drivers derived from
        //  this class.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAutoLock() const;

        tCIDLib::TBoolean bAutoLock
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bFldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TBoolean bSleep
        (
            const   tCIDLib::TCard4         c4Sleep
        );

        tCIDLib::TCard4 c4FldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TCard8 c8TimeFldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        const tCIDLib::TStrList& colFldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TVoid CheckOnline
        (
            const   TString&                strFile
            , const tCIDLib::TCard4         c4Line
        )   const;

        tCIDLib::TFloat8 f8FldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TInt4 i4FldValue
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TVoid LogMsg
        (
            const   tCIDLib::TCh* const     pszMsg
            , const tCQCKit::EVerboseLvls   eMinLevel
            , const tCIDLib::TCh* const     pszSrcFile
            , const tCIDLib::TCard4         c4SrcLine
            , const MFormattable&           mfmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken4 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogMsg
        (
            const   tCIDLib::TCh* const     pszMsg
            , const tCQCKit::EVerboseLvls   eMinLevel
            , const tCIDLib::ESeverities    eSev
            , const tCIDLib::TCh* const     pszSrcFile
            , const tCIDLib::TCard4         c4SrcLine
            , const MFormattable&           mfmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken4 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogMsg
        (
            const   TString&                strMsg
            , const tCQCKit::EVerboseLvls   eMinLevel
            , const tCIDLib::TCh* const     pszSrcFile
            , const tCIDLib::TCard4         c4SrcLine
            , const MFormattable&           mfmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken4 = MFormattable::Nul_MFormattable()
        )   const;

        tCIDLib::TVoid LogMsg
        (
            const   TString&                strMsg
            , const tCQCKit::EVerboseLvls   eMinLevel
            , const tCIDLib::ESeverities    eSev
            , const tCIDLib::TCh* const     pszSrcFile
            , const tCIDLib::TCard4         c4SrcLine
            , const MFormattable&           mfmtblToken1 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken2 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken3 = MFormattable::Nul_MFormattable()
            , const MFormattable&           mfmtblToken4 = MFormattable::Nul_MFormattable()
        )   const;

        const TCQCDriverObjCfg& cqcdcThis() const
        {
            return m_cqcdcThis;
        }

        TString strFldVal
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        tCIDLib::TVoid SetConfigObj
        (
            const   TCQCDriverObjCfg&       cqcdcInfo
        );

        tCIDLib::TVoid SetFields
        (
                    TSetFieldList&          colToSet
            , const tCIDLib::TBoolean       bInternal = kCIDLib::False
        );

        tCIDLib::TVoid SetState
        (
            const   tCQCKit::EDrvStates     eToSet
        );

        tCIDLib::TVoid SetVerboseLevel
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        tCIDLib::TVoid WriteField
        (
            const   tCIDLib::TCard4         c4FieldListId
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
            , const tCIDLib::TBoolean       bFromDriver
        );

        tCIDLib::TVoid WriteFieldByName
        (
            const   TString&                strFldName
            , const TString&                strValue
            , const tCIDLib::TBoolean       bFromDriver
        );


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TStTimesArray = TEArray<tCIDLib::TCard4, tCQCKit::EDrvStates, tCQCKit::EDrvStates::Count> ;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckSpecialCmds
        (
                    tCQCKit::EDrvStates&    ePrevState
        );

        tCIDLib::TBoolean bDefCmdRefCnt
        (
            const   tCQCKit::EDrvCmdWaits   eWait
            ,       tCIDLib::TCard4&        c4DefRef
        )   const;

        tCIDLib::TBoolean bFldWriteChecks
        (
                    TCQCFldStore&           cfsCheck
            , const tCIDLib::TCard4         c4Line
            , const tCQCKit::EValResults    eRes
            , const tCIDLib::TBoolean       bFromDrv
            , const TString&                strName
        );

        tCIDLib::TVoid CheckAccess
        (
            const   TCQCFldStore&           cfsToCheck
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        tCIDLib::TVoid CheckCommRes
        (
            const   tCQCKit::ECommResults   eToCheck
            , const TString&                strFldName
        );

        tCIDLib::TVoid CheckErrState
        (
            const   TCQCFldStore&           cfsToCheck
        )   const;

        tCIDLib::TVoid CheckFldListId
        (
            const   tCIDLib::TCard4         c4ToCheck
        )   const;

        tCIDLib::TVoid CheckTimedChanges();

        tCIDLib::TVoid CheckTypeAccess
        (
            const   TCQCFldStore&           cfsToCheck
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        tCIDLib::TVoid CheckValRes
        (
            const   tCQCKit::EValResults    eRes
            , const TString&                strName
            , const tCIDLib::TCard4         c4Line
        )   const;

        tCIDLib::TVoid CommonInit
        (
            const   tCIDLib::TBoolean       bSimMode
        );

        tCIDLib::TVoid DoTimedWrite
        (
            const   TString&                strField
            , const TString&                strNewValue
            , const TString&                strEndValue
            , const tCIDLib::TCard4         c4Seconds
        );

        tCIDLib::TVoid LogBadInitVals();

        tCIDLib::TVoid LostConnection
        (
            const   tCQCKit::EDrvStates     ePrevState
        );

        [[nodiscard]] TDrvCmd* pdcmdFindQFldWrt
        (
                    TCmdQ&                  colSrcQ
            , const EDrvCmds                eCmd
            , const tCIDLib::TCard4         c4Id
            , const TString&                strName
            , const tCIDLib::TBoolean       bByName
            , const tCIDLib::TBoolean       bWait
        );

        [[nodiscard]] TDrvCmd* pdcmdFindQCmd
        (
                    TCmdQ&                  colSrcQ
            , const EDrvCmds                eCmd
            , const tCIDLib::TBoolean       bWait
        );

        [[nodiscard]] TCQCServerBase::TDrvCmd* pdcmdGetFldWrtCmd
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldListId
            , const tCIDLib::TCard4         c4FldId
            , const tCQCKit::EDrvCmdWaits   eWait
            , const EDrvCmds                eNameCmd
            , const EDrvCmds                eIdCmd
            ,       tCIDLib::TBoolean&      bNew
            ,       tCIDLib::TBoolean&      bWait
        );

        [[nodiscard]] const TCQCFldStore* pcfsFind
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        [[nodiscard]] TCQCFldStore* pcfsFind
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        [[nodiscard]] const TCQCFldStore* pcfsFind
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        )   const;

        [[nodiscard]] TCQCFldStore* pcfsFind
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        [[nodiscard]] TCQCFldStoreBool* pcfsGetBoolStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreBool* pcfsGetBoolStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreBool* pcfsGetBoolStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreBool* pcfsGetBoolStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreCard* pcfsGetCardStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreCard* pcfsGetCardStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreCard* pcfsGetCardStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreCard* pcfsGetCardStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreFloat* pcfsGetFloatStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreFloat* pcfsGetFloatStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreFloat* pcfsGetFloatStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreFloat* pcfsGetFloatStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreInt* pcfsGetIntStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreInt* pcfsGetIntStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreInt* pcfsGetIntStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreInt* pcfsGetIntStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreString* pcfsGetStringStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreString* pcfsGetStringStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreString* pcfsGetStringStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreString* pcfsGetStringStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreSList* pcfsGetSListStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreSList* pcfsGetSListStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreSList* pcfsGetSListStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreSList* pcfsGetSListStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] TCQCFldStoreTime* pcfsGetTimeStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] TCQCFldStoreTime* pcfsGetTimeStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        );

        [[nodiscard]] const TCQCFldStoreTime* pcfsGetTimeStore
        (
            const   tCIDLib::TCard4         c4Id
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        [[nodiscard]] const TCQCFldStoreTime* pcfsGetTimeStore
        (
            const   TString&                strName
            , const tCQCKit::EFldAccess     eAccess
        )   const;

        tCIDLib::TVoid QueueCmd
        (
                    TCmdQ&                  colTarQ
            ,       TDrvCmd* const          pdcmdToAdopt
        );

        tCIDLib::TVoid ProcessCmds
        (
            const   tCIDLib::TCard4         c4WaitMSs
        );

        tCIDLib::TVoid RegDefFields();

        tCIDLib::TVoid NSRegistration();

        tCIDLib::TVoid ReleaseQCmds
        (
            const   TError&                 errRet
        );

        tCIDLib::TVoid UnhandledFldId
        (
            const   tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid UnknownFldId
        (
            const   tCIDLib::TCard4         c4FldId
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAsyncMode
        //  m_bAsyncFldOver
        //      This info comes from the driver config, but we don't want to
        //      to have to access it there (which requires a lock.) So we pull
        //      these out any time the driver config is stored, while we still
        //      have the lock.
        //
        //  m_bOnExtThread
        //      We have to remember if we are on an external thread (driver
        //      IDE probably) or if we created our own (within CQCServer
        //      most likely), so that we can clean up the thread in our dtor
        //      if we created it. If this is set, then m_pthrDevPoll points
        //      to the thread that was passed to eDevPollThread(), which
        //      is the thread that the caller passed in to simulate us
        //      running on our own thread.
        //
        //  m_bSimMode
        //      We can be used in a simulator mode, under a driver dev test
        //      harness, or under the real CQC scenario. It's initialized to false
        //      and the thread method sets it based on whether the m_pthrDevPoll
        //      member is set or not, so see the comments on that below.
        //
        //  m_ac4LoopTimes
        //      These are the millisecond periods that our poll thread will
        //      use to call back to the derived class if it needs to call
        //      the same callback multiple times in a row, which it always
        //      does for the polling loop, but also if the driver fails to
        //      initialize or get it's comm resource or config, etc... the
        //      first time through. The indices are provided by the driver
        //      states enumeration, so we can do a quick lookup based on
        //      state to see how long to wait each time around the loop.
        //
        //  m_c4FieldListId
        //      This is the field list id. Its bumped up every time that our
        //      derived class updates its field list. Its given back to anyone
        //      who asks for the field list, and must be handed back when fields
        //      are read/written. If theirs out of sync with ours, we throw an
        //      exception which tells them they must re-query the fields.
        //
        //  m_cqcdcThis
        //      The driver instance config info for this driver. This is the
        //      object that gets handed out to CQC clients. It tells them what
        //      they need to know about each available server side driver and
        //      how to connect to it, what fields it has, etc...
        //
        //      This is protected by the driver lock, and not references to
        //      it or any of its contents are exposed publically, since we
        //      could not guarantee it wouldn't change while outsiders were
        //      accessing it.
        //
        //      Any time this is updated, get out the async flags and the
        //      moniker and store them in m_bAsyncXXX and m_strMoniker, because
        //      we don't want to have to get a lock in order to access those
        //      very commonly used bits of info.
        //
        //  m_colCmdQ
        //  m_colCmdQSpec
        //      We have two command queues. One is for regular queued cmds,
        //      and the other is for special ones. The regular ones are only
        //      processed when we are in connected state, during the sleep
        //      period between connected state Poll callbacks. The special
        //      ones can be queued up any time since they affect the driver
        //      state.
        //
        //  m_colFldIdList
        //  m_colFldNameList
        //      These are polymorphic collections of storage objects for field
        //      values. The derived class makes us create these when he
        //      calls SetFldDefs() to register his fields. The base class for
        //      all of these objects is derived from the TCQCFldStore class but
        //      hold a TCQCFldDef object, so these lists provide both the field
        //      definition info and field storage. In order to support maximum
        //      speed for both by name and by id lookup, we have two lists. The
        //      by name list is a ref hash set, and it owns the objects. The by
        //      id list is a ref vector that just references the fields.
        //
        //  m_colTimedChanges
        //      This is where we store any pending timed field changes. This
        //      is treated like field data, and the field lock is used to
        //      synchronize access to it. This means that derivers only need
        //      to get the usual field lock when they write a field (which
        //      requires checking this list and cancelling any queued up
        //      write for that field.)
        //
        //  m_cuctxDrv
        //      Drives often need to do something that requires a user
        //      context. So we set up one for the derived class to use.
        //      Currently there's no actual need to allow each driver to have
        //      a separate one, but later we might allow the system admin
        //      to set up drivers to log in on a paritcular account or some
        //      such thing, which this will allow for.
        //
        //  m_enctExtend
        //      We will extend the timeout of message reads if we hit the
        //      original timeout but have gotten part of a message. It
        //      defaults to 300ms, but can be set by the driver.
        //
        //  m_enctVerbose
        //      When we go into verbose mode, this is set. We will only stay
        //      in verbose mode for 10 minutes at a time, in order to avoid
        //      the danger of a driver being left in this mode.
        //
        //  m_eState
        //      The state of the driver. This is the 10,000 foot level status
        //      of the driver which is maintained by this class. It allows
        //      the outside world to understand what's going on at a glance.
        //
        //  m_eVerboseLevel
        //  m_eVerboseNew
        //      This is used to set the driver into a verbose mode where we
        //      will log more information (and the derived driver class can
        //      do so as well), to help with diagnosis in the field. We have
        //      a 'new' value as well. Our SetNewVerbosity() method just
        //      updates the new one. If the driver thread sees they are
        //      different, it stores the ne one and calls the driver callback.
        //
        //  m_mtxSync
        //      This is used to synchronize all reads and updates of the
        //      driver state and field values and such. It is never locked
        //      longer than required to get or set the info. It should never
        //      be locked across any call out to the derived class or any
        //      remote call.
        //
        //      All sync is done with this, so none of the collections need
        //      to be thread safe.
        //
        //  m_ooidSrvAdmin
        //      If we are not in sim mode (CML IDE running the generic CML driver
        //      wrapper), then this is the object id of the CQCServer admin interface.
        //      When we register ourself in the name server, we give this as the target.
        //      That's the interface that clients use to access drivers. If in sim mode,
        //      we just ignore this and don't register ourself.
        //
        //      If loaded into the C++ test harness, this will be a faux implementation
        //      of the driver admin interface that points back to the test harness so
        //      that it can link a client and server side driver loaded into itself.
        //
        //  m_pthrDevPoll
        //      This is the thread that does all of the actual driver work. It is
        //      a pointer becasue of the special needs of IDEs that can't allow the
        //      driver to run on its own thread. When we start up our own thread,
        //      then this will be non-zero by the time our main thread entry point
        //      is called. If we don't, then we know that we have been called on an
        //      external thread, so we set this to point at that. This way, we can
        //      use this thread object reliably in either case. We set m_bSimMode
        //      flag to remember which it was.
        //
        //      It's a bit hackey, but no way around it.
        //
        //  m_strmFmt
        //      We keep a stream around that we give to field storage objects
        //      when we want them to format their value. This avoids them
        //      having to create and destroy them or each keep one of their own.
        //
        //  m_strMoniker
        //      Note that the moniker comes in in the drv config, and we
        //      wouldn't want to let anyone public access that since the
        //      driver thread can update it any time. But it's accessed a
        //      lot and having to return a copy would be annoying.
        //
        //      But, once loaded, a driver's moniker cannot change. So we
        //      store a copy into a separate member upon init (before any
        //      locking issues are involved) and it stays there forever
        //      more, and therefore the public can access it quickly. We
        //      check that no newly set config has a different moniker
        //      from this and refused to accept it if so.
        //
        //  m_tidThis
        //      To avoid calling into the thread object when asked for our
        //      thread id, we store it when the thread is started up. It
        //      never stops unless the driver is unloaded, so this is fine.
        //
        //  m_colTmp
        //  m_mbufTmp
        //      We need some temp objects during some write operations, to
        //      hold a value converted from string format, until we are sure
        //      it's acceptable to store.
        // -------------------------------------------------------------------
        TStTimesArray           m_ac4LoopTimes;
        tCIDLib::TBoolean       m_bAsyncMode;
        tCIDLib::TBoolean       m_bAsyncFldOver;
        tCIDLib::TBoolean       m_bOnExtThread;
        tCIDLib::TBoolean       m_bSimMode;
        tCIDLib::TCard4         m_c4FieldListId;
        TCmdQ                   m_colCmdQ;
        TCmdQ                   m_colCmdQSpec;
        TFieldIdList            m_colFldIdList;
        TFieldNameList          m_colFldNameList;
        TTimedChgList           m_colTimedChanges;
        TCQCDriverObjCfg        m_cqcdcThis;
        TCQCUserCtx             m_cuctxDrv;
        tCIDLib::TEncodedTime   m_enctExtend;
        tCIDLib::TEncodedTime   m_enctVerbose;
        TError                  m_errWait;
        tCQCKit::EDrvStates     m_eState;
        tCQCKit::EVerboseLvls   m_eVerboseLevel;
        tCQCKit::EVerboseLvls   m_eVerboseNew;
        TMutex                  m_mtxSync;
        TOrbObjId               m_ooidSrvAdmin;
        TThread*                m_pthrDevPoll;
        TTextStringOutStream    m_strmFmt;
        TString                 m_strMoniker;
        tCIDLib::TThreadId      m_tidThis;

        tCIDLib::TStrList       m_colTmp;
        THeapBuf                m_mbufTmp;


        // -------------------------------------------------------------------
        //  Private, static data members
        //
        //  m_splCmds
        //      A simple pool we use to keep around driver commands for client code
        //      to queue up commands on us. It's static so that we can have a single
        //      pool for all drivers, so it has to be a thread safe one of course.
        // -------------------------------------------------------------------
        static TSrvDrvCmdPool*  m_psplCmds;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCServerBase,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvCmdJan
//  PREFIX: jan
//
//  A janitor for CQCServer (and any other clients that queue up commands),
//  to insure that a command gets released. The command may be null if the
//  caller isn't waiting, since the driver will return nothing so that the
//  caller can't do anything dumb.
//
//  WE default to do a not from the driver release, which is what we want
//  everyone but the driver to do.
// ---------------------------------------------------------------------------
class CQCDRIVEREXPORT TCQCSrvCmdJan
{
    public :
        TCQCSrvCmdJan
        (
                    TCQCServerBase::TDrvCmd* const pdcmdWait
            , const tCIDLib::TBoolean       bFromDriver = kCIDLib::False
        );

        TCQCSrvCmdJan(const TCQCSrvCmdJan&) = delete;
        TCQCSrvCmdJan(TCQCSrvCmdJan&&) = delete;

        ~TCQCSrvCmdJan();

        TCQCSrvCmdJan& operator=(const TCQCSrvCmdJan&) = delete;
        TCQCSrvCmdJan& operator=(TCQCSrvCmdJan&&) = delete;

        TCQCServerBase::TDrvCmd* pdcmdWait();

        const TCQCServerBase::TDrvCmd* pdcmdWait() const;

    private :
        tCIDLib::TBoolean           m_bFromDriver;
        TCQCServerBase::TDrvCmd*    m_pdcmdWait;
};


#pragma CIDLIB_POPPACK

