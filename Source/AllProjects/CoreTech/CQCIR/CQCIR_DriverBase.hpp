//
// FILE NAME: CQCIR_DriverBase.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/16/2003
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
//  This file defines a base class for all of the server side IR device
//  drivers. They all share a lot of common functionality, so it saves a
//  lot of grunt work to factor it out.
//
//  At this level we can manage the device model configuration related
//  stuff, writes to the command invocation field, configuration loading,
//  and the invocation of IR reciever actions, which is a good bit of the
//  functionality, sometimes almost all of it for some derived drivers.
//
//  The IR driver architecture makes a good bit of use of the 'back door'
//  driver methods that allow the client side driver to talk to them outside
//  of field interface. This guy also handles all the translation of data
//  going back and forth (which is not typed data since these are generic
//  methods) and even in cases where the derived class might need to handle
//  the call, we handle the incoming method, translate the data, call a
//  protected virtual that we define (and the derived class overrides) and
//  then package up the return data if any.
//
//  Processing reciever commands is done on a background thread. Derived
//  classes call us to queue up incoming command strings. We put them on
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TBaseIRSrvDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class CQCIREXPORT TBaseIRSrvDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  COnstructors and destructor
        // -------------------------------------------------------------------
        TBaseIRSrvDriver(const TBaseIRSrvDriver&) = delete;
        TBaseIRSrvDriver(TBaseIRSrvDriver&&) = delete;

        ~TBaseIRSrvDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBaseIRSrvDriver& operator=(const TBaseIRSrvDriver&) = delete;
        TBaseIRSrvDriver& operator=(TBaseIRSrvDriver&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryData
        (
            const   TString&                strQType
            , const TString&                strDataName
            ,       tCIDLib::TCard4&        c4OutBytes
            ,       THeapBuf&               mbufToFill
        )   override;

        tCIDLib::TBoolean bQueryData2
        (
            const   TString&                strQType
            ,       tCIDLib::TCard4&        c4IOBytes
            ,       THeapBuf&               mbufIO
        )   override;

        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQType
            , const TString&                strDataName
            ,       TString&                strToFill
        )   override;

        tCIDLib::TBoolean bQueryVal
        (
            const   TString&                strValId
        )   override;

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufData
        )   override;

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCh chSepChar() const;

        const TIRBlasterCfg& irbcData() const;

        const TIRReceiverCfg& irrcData() const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TBaseIRSrvDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::EDrvInitRes eInitializeImpl() override;

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        )   override;

        tCIDLib::TVoid TerminateImpl() override;

        tCIDLib::TVoid VerboseModeChanged
        (
            const   tCQCKit::EVerboseLvls   eNewState
        )   override;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------

        // These are blaster specific
        virtual tCIDLib::TBoolean bBlastTrainingMode() const;

        virtual tCIDLib::TBoolean bCheckBlastTrainingData
        (
                    tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
        );

        virtual tCIDLib::TBoolean bCvtManualBlastData
        (
            const   TString&                strText
            ,       tCIDLib::TCard4&        c4DataBytes
            ,       THeapBuf&               mbufToFill
            ,       TString&                strError
        );

        virtual tCIDLib::TBoolean bRecTrainingMode() const;

        virtual tCIDLib::TCard4 c4ZoneCount() const;

        virtual tCIDLib::TCard4 c4InvokeFldId() const;

        virtual tCIDLib::TVoid ClearBlastTrainingData();

        virtual tCIDLib::TVoid EnterBlastTrainingMode();

        virtual tCIDLib::TVoid ExitBlastTrainingMode();

        virtual tCIDLib::TVoid FormatBlastData
        (
            const   TIRBlasterCmd&          irbcFmt
            ,       TString&                strToFill
        );

        virtual tCIDLib::TVoid InvokeBlastCmd
        (
            const   TString&                strDevice
            , const TString&                strCmd
            , const tCIDLib::TCard4         c4ZoneNum
        );

        virtual tCIDLib::TVoid SendBlasterData
        (
            const   tCIDLib::TCard4         c4DataBytes
            , const TMemBuf&                mbufToSend
            , const tCIDLib::TCard4         c4ZoneNum
            , const tCIDLib::TCard4         c4RepeatCount
        );

        // These are receiver only
        virtual tCIDLib::TBoolean bCheckRecTrainingData
        (
                    TString&                strKeyToFill
        );

        virtual tCIDLib::TVoid ClearRecTrainingData();

        virtual tCIDLib::TVoid EnterRecTrainingMode();

        virtual tCIDLib::TVoid ExitRecTrainingMode();

        // These are common to blasters and receivers
        virtual tCIDLib::TBoolean bResetConnection() = 0;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearEventQ();

        const TIRBlasterCmd& irbcFromName
        (
            const   TString&                strDevice
            , const TString&                strCmd
            ,       tCIDLib::TCard4&        c4RepeatCount
        );

        tCIDLib::TVoid QueueRecEvent
        (
            const   TString&                strEventName
        );

        tCIDLib::TVoid RegWithRebinder
        (
            const   TOrbObjId&              ooidSrvObject
            , const TString&                strNodePath
            , const TString&                strDescription
        );

        tCIDLib::TVoid SetKeySepChar
        (
            const   tCIDLib::TCh            chSepChar
        );

        tCIDLib::TVoid StartActionsThread();

        tCIDLib::TVoid WaitForActions();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFormatCmds
        (
                    TString&                strToFill
        );

        tCIDLib::TBoolean bIsRecKeyUsed
        (
            const   TString&                strKeyToCheck
            ,       TString&                strTitleToFill
        );

        tCIDLib::TBoolean bStoreRecTrainingData
        (
            const   TCQCStdKeyedCmdSrc&     csrcToStore
        );

        tCIDLib::TVoid DeleteAllEvents();

        tCIDLib::TVoid DeleteRecEvent
        (
            const   TString&                strKeyToDelete
        );

        tCIDLib::TVoid LoadBlasterDevModel
        (
            const   TString&                strModelName
        );

        tCIDLib::TVoid QueryBlasterConfig
        (
                    TVector<TIRBlasterDevModelInfo>& colList
            ,       tCIDLib::TCard4&                 c4ZoneCountToFill
        );

        tCIDLib::TVoid QueryMappedRecEvents
        (
                    TVector<TCQCStdKeyedCmdSrc>& colToFill
        );

        tCIDLib::TVoid StoreBlastConfig
        (
            const   TIRBlasterPerData&      perdToStore
        );

        tCIDLib::TVoid StoreRecCfg
        (
            const   TIRReceiverCfg&         irrcToStore
        );

        tCIDLib::TVoid ThrowUnknownId
        (
            const   TString&                strId
            , const tCIDLib::TCh* const     pszMethod
        );

        tCIDLib::TVoid UnloadBlasterDevModel
        (
            const   TString&                strModelName
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4BlastCfgVerId
        //      The configuration repository version id for our blaster
        //      config data, if we have any.
        //
        //  m_c4RecCfgVerId
        //      The configuration repository version id for our receiver
        //      config data, if we have any.
        //
        //  m_chSepChar
        //      This defaults to a null character, but it can be set by derived
        //      classes. We'll use this to break incoming trigger strings into
        //      two parts. The part before the separator is the key to match
        //      with the actions. The part after is passed to the action as an
        //      action parameter for it to use as it wishes.
        //
        //  m_colRecEventQ
        //      A queue into which receiver events are queued. The derived
        //      class calls us to queue them up, and our action thread pulls
        //      them off and processes them.
        //
        //  m_enctNextRebindTime
        //      Used to periodically cycle the rebinder. We want to do it
        //      every five seconds, or thereabouts, since the rebinder
        //      is set up for that cycle time.
        //
        //  m_eDevCaps
        //      The derived type gives us a set of capabilities flags in the
        //      ctor so that we know what he's capable of (and can pass that
        //      on to the client.)
        //
        //  m_irbcData
        //  m_irrcData
        //      This is our runtime blaster and receiver config data.
        //
        //      We don't persist the blaster data structure. We always go
        //      back to the data server at load time to get the files we
        //      need. So we run through this list and get the names of the
        //      device models out and put them into a temporary object and
        //      persist that. When we reload, we load up the temporary object
        //      and use it to load up the files and build this config data
        //      back up.
        //
        //      The receiver data is just loaded and stored as is, and
        //      contains the list of mapped events we are to respond to.
        //
        //  m_strRepoDescr
        //      The derived class gives us a short, human readable name for
        //      the blaster device, for display purposes. The client can ask
        //      for this for that reason.
        //
        //  m_strRepoName
        //      The derived class gives us a repository name that we can
        //      use for loading IR models of his sort and whatnot.
        //
        //  m_thrActions
        //      We need a thread that pulls actions out of the event queue
        //      and runs them, so that we can remain responsive to the other
        //      events coming in.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4BlastCfgVerId;
        tCIDLib::TCard4         m_c4RecCfgVerId;
        tCIDLib::TCh            m_chSepChar;
        tCIDLib::TStringQ       m_colRecEventQ;
        tCIDLib::TEncodedTime   m_enctNextRebindTime;
        tCQCIR::EIRDevCaps      m_eDevCaps;
        TIRBlasterCfg           m_irbcData;
        TIRReceiverCfg          m_irrcData;
        TString                 m_strRepoDescr;
        TString                 m_strRepoName;
        TCQCIRActionsThread     m_thrActions;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBaseIRSrvDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


