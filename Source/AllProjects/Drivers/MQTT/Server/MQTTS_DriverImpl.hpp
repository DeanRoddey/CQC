//
// FILE NAME: MQTTS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2019
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
//  This is the header file for the driver implementation class. We derive
//  from the base server side class and override callbacks.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TMQTTS
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class TMQTTS : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TMQTTS() = delete;

        TMQTTS(const TMQTTS&) = delete;

        TMQTTS
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TMQTTS();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTS& operator=(const TMQTTS&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueueIOEvent
        (
                    TMQTTIOEvPtr&           evptrToQ
        );

        TMQTTDrvEvPtr spptrDrvEvent()
        {
            return m_splDrvEvents.spptrReserveElem();
        }

        TMQTTIOEvPtr spptrIOEvent()
        {
            return m_splIOEvents.spptrReserveElem();
        }

        TMQTTInMsgPtr spptrNullInMsg()
        {
            return m_splInMsgs.spptrEmpty();
        }

        TMQTTOutMsgPtr spptrNullOutMsg()
        {
            return m_splOutMsgs.spptrEmpty();
        }

        TMQTTInMsgPtr spptrInMsg(const tCIDLib::TCard4 c4Size)
        {
            return m_splInMsgs.spptrReserveElem(c4Size);
        }

        TMQTTOutMsgPtr spptrOutMsg(const tCIDLib::TCard4 c4Size)
        {
            return m_splOutMsgs.spptrReserveElem(c4Size);
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   override;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   override;

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        )   override;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::EDrvInitRes eInitializeImpl() override;

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        )   override;

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        )   override;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   override;

        tCIDLib::TVoid ReleaseCommResource() override;

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


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::ECommResults eProcessIOEvents
        (
            const   tCIDLib::TCard4         c4WaitMS
        );

        tCQCKit::ECommResults eQueueFldWrite
        (
            const   TString&                strFldName
            , const TString&                strNewVal
        );

        tMQTTSh::EClStates eWaitForIOStateChange();

        tCIDLib::TVoid SendEvTrigger
        (
            const   TMQTTIOEvent&           ioevSrc
        );

        tCIDLib::TVoid ShutdownIO() noexcept;

        tCIDLib::TVoid StoreFldVal
        (
            const   TMQTTIOEvPtr&           evptrStore
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldId_ReloadCfg
        //      We have some driver specific fields, which we look up by id.
        //
        //  m_splXXX
        //      Our various pools that we use for in/out messages and driver/IO thread
        //      events.
        //
        //  m_thrIO
        //      This is the thread that does the actual work. We start it up and let
        //      it do its thing until we are asked to stop. It gets a pointer to
        //      us so that it can store field data
        //
        //
        //  These must be after the pools, since they will reference elements from the
        //  IO event pool. WE MUST release/flush these in Terminate() to make sure
        //  they don't try to do releases in the driver dtor, just to be safe.
        //
        //  m_colIOEvQ
        //      The queue that we give to the I/O thread for him to post I/O events
        //      to us. It's thread safe. The I/O events are via smart pointer so we
        //      don't have to worry about managing them.
        //
        //  m_evptrCur
        //      We need an event pointer to read into during the poll callback, and
        //      we do a fast poll so we don't to create one every time. That means it
        //      will hang onto the last event until the next one is stored, but that's
        //      fine.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldId_ReloadCfg;
        TDrvEventPool               m_splDrvEvents;
        TIOEventPool                m_splIOEvents;
        TInMsgPool                  m_splInMsgs;
        TOutMsgPool                 m_splOutMsgs;
        TMQTTIOThread               m_thrIO;

        // Order is important, see comments above
        TQueue<TMQTTIOEvPtr>        m_colIOEvQ;
        TMQTTIOEvPtr                m_evptrCur;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTS,TCQCServerBase)
};

#pragma CIDLIB_POPPACK
