//
// FILE NAME: CQCEventSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This file implements the more generic parts of the facility class for the
//  CQC Data Server. Some private methods, which implement various specific
//  pieces of the server, are implemented in other files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We have to bring in the CML event class header, which is otherwise private
//  to the CQCMEng facility.
// ---------------------------------------------------------------------------
#include    "CQCEventSrv.hpp"
#include    "CQCMEng_EventClass_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCEventSrv, TCQCSrvCore)



// ---------------------------------------------------------------------------
//   CLASS: TEventQItem
//  PREFIX: qitem
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEventQItem: Constructors and Destructore
// ---------------------------------------------------------------------------
TEventQItem::TEventQItem() :

    m_bLoggable(kCIDLib::False)
    , m_bScheduled(kCIDLib::False)
    , m_bSerialized(kCIDLib::False)
{
}


//
//  The triggered event info will include the trigger info that caused it. Since we
//  store by a common base class, we have to get it out now while we still have the
//  typed pointer. It's a counted pointer to so no real overhead having two copies.
//
TEventQItem::TEventQItem(const  TString&                strPath
                        ,       TCQCTrgEventEx* const   pcsrcToAdopt) :

    m_bLoggable(pcsrcToAdopt->bLoggable())
    , m_bScheduled(kCIDLib::False)
    , m_bSerialized(pcsrcToAdopt->bSerialized())
    , m_cptrTrigger(pcsrcToAdopt->cptrTrigger())
    , m_cptrSrc(pcsrcToAdopt)
    , m_strEvPath(strPath)
{
}

TEventQItem::TEventQItem(const TString& strPath, TCQCSchEvent* const pcsrcToAdopt) :

    m_bLoggable(pcsrcToAdopt->bLoggable())
    , m_bScheduled(kCIDLib::True)
    , m_bSerialized(kCIDLib::False)
    , m_cptrSrc(pcsrcToAdopt)
    , m_strEvPath(strPath)
{
}


// ---------------------------------------------------------------------------
//  TEventQItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TEventQItem::Reset()
{
    m_bLoggable = kCIDLib::False;
    m_bScheduled = kCIDLib::False;
    m_bSerialized = kCIDLib::False;
    m_strEvPath.Clear();

    m_cptrTrigger.DropRef();
    m_cptrSrc.DropRef();
}




// ---------------------------------------------------------------------------
//  CLASS: TEventMacroEngine
// PREFIX: me
//
//  We do a simple derivative of the macro engine that let's us slip incoming
//  info into the invoked macros in some cases. We want to get incoming events
//  into macro if it derives from a particular class. So we just override the
//  method that gets called just before the macro is processed. It lets us
//  check the type of the macro and if it's our special class, we set our
//  event info on it.
// ---------------------------------------------------------------------------
class TEventMacroEngine : public TCIDMacroEngine
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TEventMacroEngine()  = default;

        TEventMacroEngine(const TEventMacroEngine&) = delete;
        TEventMacroEngine(TEventMacroEngine&&) = delete;

        ~TEventMacroEngine() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventMacroEngine& operator=(const TEventMacroEngine&) = delete;
        TEventMacroEngine& operator=(TEventMacroEngine&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearEvent()
        {
            m_cptrEvData.DropRef();
        }

        tCIDLib::TVoid SetEvent(const tCQCKit::TCQCEvPtr& cptrEvData)
        {
            m_cptrEvData = cptrEvData;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AdjustStart
        (
                    TMEngClassVal&          mecvTarget
            ,       TParmList&              colParms
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_cptrEvData
        //      The event data that we set on the top level object if it is
        //      one of our special event handler macros. It's a counted
        //      pointer so that we don't have to replicate the event data
        //      every time. It's const and only needs to live until everyone
        //      has finished with it.
        // -------------------------------------------------------------------
        tCQCKit::TCQCEvPtr  m_cptrEvData;
};



// ---------------------------------------------------------------------------
//   CLASS: TWorkerThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWorkerThread: Constructors and destructor
// ---------------------------------------------------------------------------
TWorkerThread::TWorkerThread(const  tCIDLib::TCard4     c4Index
                            ,       TMutex* const       pmtxSync
                            ,       TEvent* const       pevWaitWorkers
                            ,       TStdVarsTar* const  pctarGVars
                            , const TCQCUserCtx&        cuctxToUse) :

    TThread(TString(L"EvSrvWorkerThread%(1)", TCardinal(c4Index)))
    , m_acteToUse(cuctxToUse)
    , m_bActive(kCIDLib::False)
    , m_colList()
    , m_evControl(tCIDLib::EEventStates::Reset)
    , m_pctarGVars(pctarGVars)
    , m_pmeEvent(nullptr)
    , m_pevWaitWorkers(pevWaitWorkers)
    , m_pmtxSync(pmtxSync)
{
    //
    //  Set up a macro engine we'll use for any macro processing. We have to set it on
    //  the macro processing target of the command engine. We use this special engine
    //  so that we can support CML based event handler macros. The target will own it
    //  now. If we didn't set this one, it would fault in a standard macro engine.
    //
    //  We keepa pointer since we need to set event trigger inf on it before processing
    //  a triggered event that might invoke CML.
    //
    m_pmeEvent = new TEventMacroEngine;
    m_acteToUse.ctarMacro().SetEngine(m_pmeEvent);
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by the dispatcher thread. Adds this event to this thread's queue. If it
//  is not already running, then we post his event.
//
//  The shared sync mutex will be locked when this is called, so our thread cannot mess
//  with the list or change his active state while we are in here.
//
//  To avoid a race condition, if he is not active, we set his active flag ourself. Else
//  we could return and the calling (dispatcher) thread might think him inactive before
//  this guy wakes up and sets it.
//
tCIDLib::TVoid TWorkerThread::AddEvent(const TEventQItem& qitemNew)
{
    // Unlikely but could happen
    if (m_colList.bIsFull(c4MaxEventblock))
    {
        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvErrs::errcEv_WorkerFull
            , m_qitemCur.m_strEvPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource

        );
        return;
    }

    m_colList.objAdd(qitemNew);
    if (!m_bActive)
    {
        m_bActive = kCIDLib::True;
        m_evControl.Trigger();
    }
}


//
//  Called by the dispatcher thread when it gets a serialized triggered event. He will
//  call us here to ask if we are processing a previous instance of that event. If so,
//  we return true and he calls bAddEvent on us, which we will just add to our queue. Else
//  he will treat it normally and give it to any non-active worker.
//
//  The shared sync mutex will be locked when this is called, so our thread cannot mess
//  with the list or change his state while we are in here.
//
tCIDLib::TBoolean TWorkerThread::bIsHandlingEvType(const TString& strPath)
{
    // If not active, or not a triggered event, obviously not
    if (!m_bActive || m_qitemCur.m_bScheduled)
        return kCIDLib::False;

    return m_qitemCur.m_strEvPath.bCompareI(strPath);
}



// ---------------------------------------------------------------------------
//  TWorkerThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TWorkerThread::eProcess()
{
    // Don't log the same error more than once per five minutes
    TLogLimiter loglWorker(60 * 5);

    //
    //  Set up a list of extra targets beyond the standard ones. We setup
    //  an event target.
    //
    TTrigEvCmdTarget ctarEvent;
    TRefVector<MCQCCmdTarIntf> colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);
    colExtraTars.Add(&ctarEvent);

    // We need a little state machine
    enum class EStates
    {
        WaitForWork
        , GetEvent
        , ProcessEvent
        , Reset
    };
    EStates eState = EStates::WaitForWork;
    while (!bCheckShutdownRequest())
    {
        try
        {
            if (eState == EStates::WaitForWork)
            {
                //
                //  This provides our throttling. If we are signaled, go to the next
                //  state. Else we just go around again.
                //
                if (m_evControl.bWaitFor(500))
                    eState = EStates::GetEvent;
            }
             else if (eState == EStates::GetEvent)
            {
                //
                //  Get an event, don't wait (since we have to lock during this and
                //  we want to give up as soon as there's no more anyway.
                //
                //  If we don't get one, go to reset mode. If we do, go to process mode.
                //
                TLocker lockrSync(m_pmtxSync);
                if (m_colList.bGetNext(m_qitemCur, 0, kCIDLib::False))
                    eState = EStates::ProcessEvent;
                else
                    eState = EStates::Reset;
            }
             else if (eState == EStates::ProcessEvent)
            {
                //
                //  Catch and log here, since we want to log the id of the failed event
                //  and this is where something is likey to fail.
                try
                {
                    if (m_qitemCur.m_bScheduled)
                        ProcessSchedEv();
                    else
                        ProcessTrigEv();
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facCQCEventSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kEvSrvMsgs::midStatus_EventFailed
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_qitemCur.m_strEvPath
                    );
                }

                catch(...)
                {
                    facCQCEventSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kEvSrvMsgs::midStatus_EventFailed
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_qitemCur.m_strEvPath
                    );
                }

                // One way or another we now go back to get event state
                eState = EStates::GetEvent;
            }
             else if (eState == EStates::Reset)
            {
                //
                //  There is a possibility we might get another event slipped in on
                //  us if we are processing a serialized event and another of that
                //  type came in. So lock first, and then check the list.
                //
                TLocker lockrSync(m_pmtxSync);
                if (m_colList.bIsEmpty())
                {
                    //
                    //  We are going down, so clear our flag and reset our event so that we
                    //  will block when we get to the top again.
                    //
                    m_bActive = kCIDLib::False;
                    m_evControl.Reset();

                    //
                    //  Now trigger the wait workers event. If the dispatcher thread is
                    //  waiting for a thread to dispatch to, this will wake him up.
                    //
                    m_pevWaitWorkers->Trigger();

                    // And we are back to wait for work state
                    eState = EStates::WaitForWork;
                }
                 else
                {
                    // We were beat to the punch, so go bakc to get event mode
                    eState = EStates::GetEvent;
                }
            }
        }

        catch(TError& errToCatch)
        {
            // Try to recover
            {
                TLocker lockrSync(m_pmtxSync);
                if (eState != EStates::WaitForWork)
                    eState = EStates::Reset;
            }

            if (loglWorker.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                facCQCEventSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kEvSrvMsgs::midStatus_ExceptInThread
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"event worker")
                );
            }
        }

        catch(...)
        {
            // Try to recover
            {
                TLocker lockrSync(m_pmtxSync);
                if (eState != EStates::WaitForWork)
                    eState = EStates::Reset;
            }

            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_ExceptInThread
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"event worker")
            );
        }
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TWorkerThread::ProcessSchedEv()
{
    try
    {
        if (facCQCEventSrv.bLogInfo() || m_qitemCur.m_bLoggable)
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_SchEventStarted
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_qitemCur.m_strEvPath
            );
        }

        // We need an empty extra targets list here
        tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);

        // Clear the event trigger info on the macro engine for a scheduled event
        m_pmeEvent->ClearEvent();

        m_acteToUse.eInvokeOps
        (
            *m_qitemCur.m_cptrSrc.pobjData()
            , kCQCKit::strEvId_OnScheduledTime
            , *m_pctarGVars
            , colExtraTars
            , 0
            , TString::strEmpty()
        );

        if (facCQCEventSrv.bLogInfo() || m_qitemCur.m_bLoggable)
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_SchEventDone
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_qitemCur.m_strEvPath
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TWorkerThread::ProcessTrigEv()
{
    try
    {
        if (facCQCEventSrv.bLogInfo() || m_qitemCur.m_bLoggable)
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_TrgEventStarted
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_qitemCur.m_strEvPath
            );
        }

        // Set the event trigger info on our macro engine for a triggerd event
        m_pmeEvent->SetEvent(m_qitemCur.m_cptrTrigger);

        //
        //  And set up the triggered event extr target, with the trigger  info set on
        //  it also, so that it's available via action commands.
        //
        tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 1);
        m_ctarEvent.SetEvent(m_qitemCur.m_cptrTrigger);
        colExtraTars.Add(&m_ctarEvent);

        m_acteToUse.eInvokeOps
        (
            *m_qitemCur.m_cptrSrc.pobjData()
            , kCQCKit::strEvId_OnTrigger
            , *m_pctarGVars
            , colExtraTars
            , 0
            , TString::strEmpty()
        );

        if (facCQCEventSrv.bLogInfo() || m_qitemCur.m_bLoggable)
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_TrgEventDone
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_qitemCur.m_strEvPath
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



//
//  Our parent class, the macro engine class, will call this just before it
//  invokes the Start() method of the main class. We will see if it is derived
//  from the event handler class and if so, we'll slip our stored event data
//  into it, so that it is there for the derived handler class to get.
//
tCIDLib::TVoid
TEventMacroEngine::AdjustStart(TMEngClassVal& mecvTarget, TParmList& colParms)
{
    //
    //  If it's derived from the event handler class, then get access to the
    //  event data member (which is a CML level data member), and set our stored
    //  event data on it.
    //
    const tCIDLib::TCard2 c2HandlerId = c2FindClassId
    (
        TEventHandlerInfo::strClassPath(), kCIDLib::False
    );

    if (c2HandlerId != kCIDMacroEng::c2BadId)
    {
        TMEngClassInfo& meciTar = meciFind(mecvTarget.c2ClassId());
        if (bIsDerivedFrom(meciTar.c2Id(), c2HandlerId))
        {
            // Get the event member
            TMEngClassVal* pmecvEvMem
            (
                mecvTarget.pmecvFind(TEventHandlerInfo::strHandlerData())
            );

            if (pmecvEvMem)
            {
                // Cast it to it's actual type and set our event data on it
                TEventValue* pmecvEvent = static_cast<TEventValue*>(pmecvEvMem);
                pmecvEvent->cevValue(*m_cptrEvData.pobjData());
            }
             else
            {
                // Log something here
            }
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TFacCQCEventSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacNameSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We ask server core for a login and to enable the sending of events. We need
//  the former since events can do things that require credentials. And we of course
//  need to both send and receive event triggers.
//
TFacCQCEventSrv::TFacCQCEventSrv() :

    TCQCSrvCore
    (
        L"CQCEventSrv"
        , L"CQC Event Server"
        , kCQCKit::ippnEventDefPort
        , kCQCEventSrv::pszEvEventSrv
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::AllEvents | tCQCSrvFW::ESrvOpts::LogIn
    )
    , m_c4SerChanges(1)
    , m_c4SerEvMonList(1)
    , m_c4SerSchEvList(1)
    , m_c4SerTrgEvList(1)
    , m_colActQ(tCIDLib::EMTStates::Safe)
    , m_colEvMonitors(tCIDLib::EAdoptOpts::Adopt)
    , m_colTimeView(tCIDLib::EAdoptOpts::NoAdopt)
    , m_colWorkerThreads(tCIDLib::EAdoptOpts::Adopt, kCQCEventSrv::c4EvWorkerThreads)
    , m_ctarGVars(tCIDLib::EMTStates::Safe, kCIDLib::False)
    , m_eCfgState(tCQCEventSrv::ECfgStates::Min)
    , m_evWaitWorkers()
    , m_f8Lat(0)
    , m_f8Long(0)
    , m_mhashTrigKey(TUniqueId::mhashMakeId())
    , m_porbsProtoImpl(nullptr)
    , m_thrDispatch
      (
          L"CQCEvDispatchThread"
          , TMemberFunc<TFacCQCEventSrv>(this, &TFacCQCEventSrv::eDispatchThread)
      )
    , m_thrEv
      (
          L"CQCEventEvThread"
          , TMemberFunc<TFacCQCEventSrv>(this, &TFacCQCEventSrv::eEvThread)
      )
    , m_thrQR
      (
          L"CQCEventQRThread"
          , TMemberFunc<TFacCQCEventSrv>(this, &TFacCQCEventSrv::eQRThread)
      )
{
}


TFacCQCEventSrv::~TFacCQCEventSrv()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCEventSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TFacCQCEventSrv::AddEvent(  const   tCQCKit::EEvSrvTypes    eEvType
                            , const TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4ChangeSerNum
                            , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_EvAdding
        , strRelPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    try
    {
        // We require system admin rights
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

        // We need to read in the event so get a data server client
        TDataSrvClient dsclAdd(kCIDLib::True);

        // And based on the event type, let's do the work
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            //
            //  Let's try to read in the event monitor config before we lock. If this
            //  fails, no need to go further, adn we avoid locking across a network
            //  call.
            //
            TCQCEvMonCfg emoncNew;
            tCIDLib::TKVPFList colMeta;
            dsclAdd.bReadEventMon(strRelPath, c4SerNum, enctLast, emoncNew, colMeta, sectUser);

            // Lock and see if we have this this path already. If so, that's bad
            TLocker lockrSync(&m_mtxOuter);
            c4FindEvMon(strRelPath, tCIDLib::EFindFlags::ThrowIfExists);

            // And now add a new event monitor object
            TCQCEvMonitor* pthrNew = new TCQCEvMonitor(strRelPath, emoncNew);
            m_colEvMonitors.Add(pthrNew);

            // Bump the changes serial number and set it on this monitor
            m_c4SerChanges++;
            pthrNew->c4SerialNum(m_c4SerChanges);

            // Give back the new value
            c4ChangeSerNum = m_c4SerChanges;

            // We also need to store this as the list serial number for monitors
            m_c4SerEvMonList = m_c4SerChanges;

            // Try to start it if not paused. If this fails, it'll be marked as failed.
            if (!emoncNew.bPaused())
            {
                try
                {
                    pthrNew->StartMonitor(cuctxToUse());
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }
            }
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            //
            //  Let's try to read in the event before we lock. If this fails, no need
            //  to go further, and we avoid locking across a network call.
            //
            TCQCSchEvent csrcNew;
            tCIDLib::TKVPFList colMeta;
            dsclAdd.bReadSchedEvent(strRelPath, c4SerNum, enctLast, csrcNew, colMeta, sectUser);

            // Lock and see if we have this this path already. If so, that's bad
            TLocker lockrSync(&m_mtxOuter);
            c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfExists);

            TKeyedCQCSchEvent& csrcAdded = m_colSchEvents.objAdd
            (
                TKeyedCQCSchEvent(strRelPath, csrcNew)
            );

            // Bump the changes serial number and set it on this new event
            m_c4SerChanges++;
            csrcAdded.c4SerialNum(m_c4SerChanges);

            // Give back the new value
            c4ChangeSerNum = m_c4SerChanges;

            // We also need to store this as the list serial number for scheduled events
            m_c4SerSchEvList = m_c4SerChanges;

            // And update the sorted scheduled event view
            MakeSortedSchedule();
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            //
            //  Let's try to read in the event before we lock. If this fails, no need
            //  to go further, and we avoid locking across a network call.
            //
            tCIDLib::TKVPFList colMeta;
            TCQCTrgEvent csrcNew;
            dsclAdd.bReadTrigEvent(strRelPath, c4SerNum, enctLast, csrcNew, colMeta, sectUser);

            // Lock and see if we have this this path already. If so, that's bad
            TLocker lockrSync(&m_mtxOuter);
            c4FindTrgEv(strRelPath, tCIDLib::EFindFlags::ThrowIfExists);

            TKeyedCQCTrgEvent& csrcAdded = m_colTrgEvents.objAdd
            (
                TKeyedCQCTrgEvent(strRelPath, csrcNew)
            );

            // Bump the changes serial number and set it on this new event
            m_c4SerChanges++;
            csrcAdded.c4SerialNum(m_c4SerChanges);

            // Give back the new value
            c4ChangeSerNum = m_c4SerChanges;

            // We also need to store this as the list serial number for triggered events
            m_c4SerTrgEvList = m_c4SerChanges;
        }
         else
        {
            // It's not a valid event path
            facCQCEvCl().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kEvClErrs::errcGen_NotPathType
                , strRelPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
             );
        }

        // Log that we've done that
        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvMsgs::midStatus_EvAdded
            , strRelPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TFacCQCEventSrv::DumpEvents(const TString& strTarFile)
{
    TTextFileOutStream strmTar
    (
        strTarFile
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Write
        , new TUSASCIIConverter
    );

    // The file open worked, so lock and dump the info
    TLocker lockrSync(&m_mtxOuter);

    tCIDLib::TCard4 c4Count;
    strmTar << L"{\n";

    strmTar.c4Indent(4);
    strmTar << L"\"EvMonitors\" : \n"
            << L"[\n";
    strmTar.c4Indent(8);
    c4Count = m_colEvMonitors.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCEvMonitor* pthrMon = m_colEvMonitors[c4Index];
        const TCQCEvMonCfg& emoncCur = pthrMon->emoncThis();

        if (c4Index)
            strmTar << L",\n";

        strmTar << L"{";
        {
            TStreamIndentJan janContent(&strmTar, 4);
            strmTar << L"\"Path\" : \"" << pthrMon->strPath() << L"\",\n"
                    << L"\"SerialNum\" : " << pthrMon->c4SerialNum() << L",\n"
                    << L"\"Paused\" : " << (emoncCur.bPaused() ? L"true" : L"false") << L",\n"
                    << L"\"Failed\" : " << (emoncCur.bFailed() ? L"true" : L"false") << L",\n"
                    << L"\"ClassPath\" : \"" << emoncCur.strClassPath() << L"\",\n";
        }
        strmTar << L"}";

        strmTar << kCIDLib::DNewLn;
    }
    strmTar.c4Indent(4);
    strmTar << L"]," << kCIDLib::DNewLn;

    strmTar << L"\"SchEvents\" : \n"
            << L"[\n";
    strmTar.c4Indent(8);
    c4Count = m_colSchEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyedCQCSchEvent& csrcCur= m_colSchEvents[c4Index];

        if (c4Index)
            strmTar << L",\n";

        strmTar << L"{\n";
        {
            TStreamIndentJan janContent(&strmTar, 4);
            strmTar << L"\"Path\" : \"" << csrcCur.strPath() << L"\",\n"
                    << L"\"SerialNum\" : " << csrcCur.c4SerialNum() << L",\n"
                    << L"\"Paused\" : " << (csrcCur.bPaused() ? L"true" : L"false") << L"\n";
        }
        strmTar << L"}";
    }
    strmTar.c4Indent(4);
    strmTar << L"\n]," << kCIDLib::DNewLn;


    strmTar << L"\"TrgEvents\" : \n"
            << L"[\n";
    strmTar.c4Indent(8);
    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyedCQCTrgEvent& csrcCur= m_colTrgEvents[c4Index];

        if (c4Index)
            strmTar << L",\n";

        strmTar << L"{\n";
        {
            TStreamIndentJan janContent(&strmTar, 4);
            strmTar << L"\"Path\" : \"" << csrcCur.strPath() << L"\",\n"
                    << L"\"SerialNum\" : " << csrcCur.c4SerialNum() << L",\n"
                    << L"\"Paused\" : " << (csrcCur.bPaused() ? L"true" : L"false") << L"\n";
        }
        strmTar << L"}";
    }
    strmTar.c4Indent(4);
    strmTar << L"\n]\n";

    strmTar.c4Indent(0);
    strmTar << L"\n}" << kCIDLib::DNewLn;
}



//
//  Remove the event with the indicated path. The caller has already removed it from
//  the configuration, so we just need to remove it from our live list.
//
tCIDLib::TVoid
TFacCQCEventSrv::DeleteEvent(   const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_EvDeleting
        , strRelPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
    );

    try
    {
        // We require at least system admin rights
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

        TLocker lockrSync(&m_mtxOuter);

        // Find this guy in our list and remove it if there
        tCIDLib::TCard4 c4Index = kCIDLib::c4MaxCard;
        if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            if (c4Index != kCIDLib::c4MaxCard)
            {
                m_colSchEvents.RemoveAt(c4Index);
                m_c4SerChanges++;
                m_c4SerSchEvList = m_c4SerChanges;

                // Update our sorted scheduled event view to get rid of this guy
                MakeSortedSchedule();
            }
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            c4Index = c4FindTrgEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            if (c4Index != kCIDLib::c4MaxCard)
            {
                m_colTrgEvents.RemoveAt(c4Index);
                m_c4SerChanges++;
                m_c4SerTrgEvList = m_c4SerChanges;
            }
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            c4Index = c4FindEvMon(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            if (c4Index != kCIDLib::c4MaxCard)
            {
                // We have to stop it if running, then remove it from our list
                m_colEvMonitors[c4Index]->StopMonitor();
                m_colEvMonitors.RemoveAt(c4Index);

                m_c4SerChanges++;
                m_c4SerEvMonList = m_c4SerChanges;

            }
        }
         else
        {
            // Shouldn't happen but just in case
            CIDAssert2(L"Unknown event type in pause/resume method");
        }

        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvMsgs::midStatus_EvDeleted
            , strRelPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Give back the latest change serial number
    c4ChangeSerNum = m_c4SerChanges;
}


//
//  Pause an event (so that it doesn't run even if ready.) The caller should have
//  already updated the configuration for this event to mark it as such. We are just
//  being asked to do the actual change to our live instance of it.
//
//  Note this is only for scheduled/triggered events. Event monitors have their own
//  method since they have other complications.
//
tCIDLib::TVoid
TFacCQCEventSrv::PauseEvent(const   tCQCKit::EEvSrvTypes    eEvType
                            , const TString&                strRelPath
                            ,       tCIDLib::TCard4&        c4ChangeSerNum
                            , const tCIDLib::TBoolean       bPaused
                            , const TCQCSecToken&           sectUser)
{
    try
    {
        // For now, just some valid user account is all we require
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::LimitedUser);

        // Lock and do the pause operation
        TLocker lockrSync(&m_mtxOuter);

        //
        //  Find this guy in our list and update it. If we find it, we'll put info
        //  in the strName string and use that below to log a msg.
        //
        tCIDLib::TCard4 c4Index = kCIDLib::c4MaxCard;
        if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCSchEvent& csrcTar = m_colSchEvents[c4Index];
            csrcTar.bPaused(bPaused);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            c4Index = c4FindTrgEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCTrgEvent& csrcTar = m_colTrgEvents[c4Index];
            csrcTar.bPaused(bPaused);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            c4Index = c4FindEvMon(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TCQCEvMonitor* pthrTar = m_colEvMonitors[c4Index];

            //
            //  If the current state is different from the passed state we have
            //  to start or stop the event monitor.
            //
            if (pthrTar->bPaused() != bPaused)
            {
                // Start or stop it as requested
                if (bPaused)
                    pthrTar->StopMonitor();
                else
                    pthrTar->StartMonitor(cuctxToUse());

                // Bump the changes serial number and set it on this event
                m_c4SerChanges++;
                pthrTar->c4SerialNum(m_c4SerChanges);
            }
        }
         else
        {
            // Shouldn't happen but just in case
            CIDAssert2(L"Unknown event type in pause/resume method");
        }

        // Log what we did
        TString strEVPath = facCQCRemBrws().strFlTypePref
        (
            facCQCEvCl().eEvTypeToBrwsDType(eEvType)
        );
        strEVPath.Append(strRelPath);
        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , bPaused ? kEvSrvMsgs::midStatus_EvPaused
                      : kEvSrvMsgs::midStatus_EvResumed
            , strEVPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This one handles a request to rename an individual event. We just need to find it
//  and update its path to the new path.
//
tCIDLib::TVoid
TFacCQCEventSrv::RenameEvent(   const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelParPath
                                , const TString&                strOldName
                                , const TString&                strNewName
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_EvRenaming
        , strRelParPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , strOldName
        , strNewName
    );

    try
    {
        // We require system admin rights
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

        TLocker lockrSync(&m_mtxOuter);

        // Build up the old and new relative paths
        TString strOldPath(strRelParPath);
        TString strNewPath(strOldPath);

        facCQCRemBrws().AddPathComp(strOldPath, strOldName);
        facCQCRemBrws().AddPathComp(strNewPath, strNewName);

        // Now just find the event and update its path
        tCIDLib::TCard4 c4Index;
        if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            c4Index = c4FindEvMon(strOldPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TCQCEvMonitor* pthrTar = m_colEvMonitors[c4Index];
            pthrTar->strPath(strNewPath);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            pthrTar->c4SerialNum(m_c4SerChanges);
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            c4Index = c4FindSchEv(strOldPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCSchEvent& csrcTar = m_colSchEvents[c4Index];
            csrcTar.strPath(strNewPath);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            c4Index = c4FindTrgEv(strOldPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCTrgEvent& csrcTar = m_colTrgEvents[c4Index];
            csrcTar.strPath(strNewPath);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);
        }

        // And give back the latest serial number
        c4ChangeSerNum = m_c4SerChanges;

        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvMsgs::midStatus_EvRenamed
            , strRelParPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strOldName
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This one handles a request to rename a scope. In this case we have to find all of
//  the events of the indicated type whose paths start wtih the indicated old path.
//  We update that part of the path with the new path.
//
tCIDLib::TVoid
TFacCQCEventSrv::RenameScope(   const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelParPath
                                , const TString&                strOldName
                                , const TString&                strNewName
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_EvScopeRenaming
        , strRelParPath
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , strOldName
        , strNewName
    );

    //
    //  Build up the old and new relative paths. We make sure they end with slashs
    //  so that we are sure that any matches are really matching the path part.
    //
    TString strOldPath(strRelParPath);
    TString strNewPath(strOldPath);

    facCQCRemBrws().AddPathComp(strOldPath, strOldName);
    facCQCRemBrws().AddPathComp(strNewPath, strNewName);

    strOldPath.Append(kCIDLib::chForwardSlash);
    strNewPath.Append(kCIDLib::chForwardSlash);


    try
    {
        // We require system admin
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::SystemAdmin);

        TLocker lockrSync(&m_mtxOuter);

        tCIDLib::TBoolean   bChanges = kCIDLib::False;
        tCIDLib::TCard4     c4Count;
        TString             strCurPath;
        if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            c4Count = m_colEvMonitors.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TCQCEvMonitor* pthrTar = m_colEvMonitors[c4Index];
                if (pthrTar->strPath().bStartsWithI(strOldPath))
                {
                    strCurPath = pthrTar->strPath();
                    strCurPath.Cut(0, strOldPath.c4Length());
                    strCurPath.Insert(strNewPath, 0);

                    pthrTar->strPath(strCurPath);
                    bChanges = kCIDLib::True;
                }
            }

            if (bChanges)
            {
                m_c4SerChanges++;
                m_c4SerEvMonList = m_c4SerChanges;
            }
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            c4Count = m_colSchEvents.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TKeyedCQCSchEvent& csrcCur= m_colSchEvents[c4Index];
                if (csrcCur.strPath().bStartsWithI(strOldPath))
                {
                    strCurPath = csrcCur.strPath();
                    strCurPath.Cut(0, strOldPath.c4Length());
                    strCurPath.Insert(strNewPath, 0);

                    csrcCur.strPath(strCurPath);
                    bChanges = kCIDLib::True;
                }
            }

            if (bChanges)
            {
                m_c4SerChanges++;
                m_c4SerSchEvList = m_c4SerChanges;
            }
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            c4Count = m_colTrgEvents.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TKeyedCQCTrgEvent& csrcCur= m_colTrgEvents[c4Index];
                if (csrcCur.strPath().bStartsWithI(strOldPath))
                {
                    strCurPath = csrcCur.strPath();
                    strCurPath.Cut(0, strOldPath.c4Length());
                    strCurPath.Insert(strNewPath, 0);

                    csrcCur.strPath(strCurPath);
                    bChanges = kCIDLib::True;
                }
            }

            if (bChanges)
            {
                m_c4SerChanges++;
                m_c4SerTrgEvList = m_c4SerChanges;
            }
        }

        // And give back the latest serial number
        c4ChangeSerNum = m_c4SerChanges;

        // Log what we did
        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvMsgs::midStatus_EvScopeRenamed
            , strRelParPath
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strOldName
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  The client has already modified and stored an event, and is telling us to query
//  the new contents and update our in-memory version of it.
//
tCIDLib::TVoid
TFacCQCEventSrv::UpdateEvent(   const   tCQCKit::EEvSrvTypes    eEvType
                                , const TString&                strRelPath
                                ,       tCIDLib::TCard4&        c4ChangeSerNum
                                , const TCQCSecToken&           sectUser)
{
    if (facCQCEventSrv.bLogInfo())
    {
        facCQCEventSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kEvSrvMsgs::midStatus_EvUpdating
            , strRelPath
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    try
    {
        //
        //  We require just a valid login. This needs to be called in the context of
        //  actions within the IV where we can't require any particular user type. This
        //  can only update what has been configured, so it's not a security issue.
        //
        facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::LimitedUser);

        // We need to read in the event so get a data server client
        TDataSrvClient dsclAdd(kCIDLib::True);

        //
        //  Find this guy in our list and update it. If we find it, we'll put info
        //  in the strName string and use that below to log a msg.
        //
        tCIDLib::TCard4         c4Index = kCIDLib::c4MaxCard;
        tCIDLib::TCard4         c4SerNum = 0;
        tCIDLib::TEncodedTime   enctLast;
        TString                 strName;
        tCIDLib::TKVPFList      colMeta;
        if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        {
            // Read in the new config before we lock
            TCQCEvMonCfg emoncNew;
            dsclAdd.bReadEventMon(strRelPath, c4SerNum, enctLast, emoncNew, colMeta, sectUser);

            // Now lock and complete it
            TLocker lockrSync(&m_mtxOuter);
            c4Index = c4FindEvMon(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TCQCEvMonitor* pthrTar = m_colEvMonitors[c4Index];

            //
            //  We have to stop it before we can update his config. We have to set
            //  the path as well, but we know it's not going to actually change the
            //  path.
            //
            pthrTar->StopMonitor();
            pthrTar->SetConfig(strRelPath, emoncNew);

            // Now, if the config doesn't indicate its paused, we start it back up
            if (!pthrTar->bPaused())
                pthrTar->StartMonitor(cuctxToUse());

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            pthrTar->c4SerialNum(m_c4SerChanges);

            // And give it back
            c4ChangeSerNum = m_c4SerChanges;
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        {
            // Read into a temp before we lock
            TCQCSchEvent csrcNew;
            dsclAdd.bReadSchedEvent(strRelPath, c4SerNum, enctLast, csrcNew, colMeta, sectUser);

            // Lock now and update
            TLocker lockrSync(&m_mtxOuter);

            c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCSchEvent& csrcTar = m_colSchEvents[c4Index];
            csrcTar.SetConfig(csrcNew);

            // Update our sorted scheduld event view
            MakeSortedSchedule();

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);

            // And give it back
            c4ChangeSerNum = m_c4SerChanges;
        }
         else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        {
            // Read into a temp before we lock
            TCQCTrgEvent csrcNew;
            dsclAdd.bReadTrigEvent(strRelPath, c4SerNum, enctLast, csrcNew, colMeta, sectUser);

            // Lock now and update
            TLocker lockrSync(&m_mtxOuter);
            c4Index = c4FindTrgEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
            TKeyedCQCTrgEvent& csrcTar = m_colTrgEvents[c4Index];
            csrcTar.SetConfig(csrcNew);

            // Bump the changes serial number and set it on this event
            m_c4SerChanges++;
            csrcTar.c4SerialNum(m_c4SerChanges);

            // And give it back
            c4ChangeSerNum = m_c4SerChanges;
        }
         else
        {
            // Shouldn't happen but just in case
            CIDAssert2(L"Unknown event type in pause/resume method");
        }

        // Log what we did
        if (facCQCEventSrv.bLogInfo())
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_EvUpdated
                , strRelPath
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



//
//  Return some info about the indicated periodic type scheduled event.
//  The caller is responsible for only calling this for an event for
//  which it makes sense.
//
tCIDLib::TVoid TFacCQCEventSrv
::QueryPerEvInfo(const  TString&                strRelPath
                ,       tCQCKit::ESchTypes&     eType
                ,       tCIDLib::TEncodedTime&  enctAt
                ,       tCIDLib::TCard4&        c4Period
                ,       tCIDLib::TBoolean&      bPaused)
{
    // It worked, so lock and try to do the update
    TLocker lockrSync(&m_mtxOuter);

    //
    //  Find this guy in our list, to make sure it exists and to see if
    //  setting the time on it makes any sense.
    //
    tCIDLib::TCard4 c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
    TKeyedCQCSchEvent& csrcSrc = m_colSchEvents[c4Index];

    eType = csrcSrc.eType();
    bPaused = csrcSrc.bPaused();
    c4Period = csrcSrc.c4Period();
    enctAt = csrcSrc.enctAt();
}


//
//  Return some info about the indicated scheduled type scheduled event.
//  The caller is responsible for only calling this for an event for
//  which it makes sense.
//
tCIDLib::TVoid TFacCQCEventSrv
::QuerySchEvInfo(const  TString&            strRelPath
                ,       tCQCKit::ESchTypes& eType
                ,       tCIDLib::TCard4&    c4Day
                ,       tCIDLib::TCard4&    c4Hour
                ,       tCIDLib::TCard4&    c4Minute
                ,       tCIDLib::TCard4&    c4Mask
                ,       tCIDLib::TBoolean&  bPaused)
{
    // Lock and look up the info requested
    TLocker lockrSync(&m_mtxOuter);

    //
    //  Find this guy in our list, to make sure it exists and to see if
    //  setting the time on it makes any sense.
    //
    tCIDLib::TCard4 c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
    TKeyedCQCSchEvent& csrcSrc = m_colSchEvents[c4Index];

    eType = csrcSrc.eType();
    bPaused = csrcSrc.bPaused();
    c4Day = csrcSrc.c4Day();
    c4Hour = csrcSrc.c4Hour();
    c4Minute = csrcSrc.c4Minute();
    c4Mask = csrcSrc.c4Mask();
}


//
//  We return some info about a sunrise/sunset based event.
//
tCIDLib::TVoid
TFacCQCEventSrv::QuerySunBasedEvInfo(const  TString&            strRelPath
                                    ,       tCIDLib::TInt4&     i4Offset
                                    ,       tCIDLib::TBoolean&  bPaused)
{
    // Lock and look up the info requested
    TLocker lockrSync(&m_mtxOuter);

    //
    //  Find this guy in our list, to make sure it exists and to see if
    //  setting the time on it makes any sense.
    //
    tCIDLib::TCard4 c4Index = c4FindSchEv(strRelPath, tCIDLib::EFindFlags::ThrowIfNotFnd);
    TKeyedCQCSchEvent& csrcSrc = m_colSchEvents[c4Index];

    // OK, we found it so get the info out
    bPaused = csrcSrc.bPaused();
    i4Offset = csrcSrc.i4Offset();
}


//
//  If the client does something that would make substantial changes to one of the lists
//  of configured events, they call this to get us to reload the list, instead of trying
//  to do a lot of non-atomic operations.
//
tCIDLib::TVoid TFacCQCEventSrv::ReloadList(const tCQCKit::EEvSrvTypes eEvType)
{
    // We need a data server client. Get it before we lock in case it has issues
    TDataSrvClient dsclLoad(kCIDLib::True);

    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_ReloadingList
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , tCQCKit::strXlatEEvSrvTypes(eEvType)
    );

    // Lock and look up the info requested
    TLocker lockrSync(&m_mtxOuter);

    // Reload the requested list
    if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
        LoadEvMonitors(dsclLoad);
    else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
        LoadSchedEvs(dsclLoad);
    else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
        LoadTrigEvs(dsclLoad);

    facCQCEventSrv.LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_ReloadedList
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , tCQCKit::strXlatEEvSrvTypes(eEvType)
    );
}


//
//  This is called from our ORB interface, which just delegates to us on
//  this call. It allows the client to set our lat/long info. Note that
//  we do not store it. The client is responsible for updating the stored
//  lat/long info in the config server.
//
tCIDLib::TVoid
TFacCQCEventSrv::SetLocInfo(const   tCIDLib::TFloat8    f8Lat
                            , const tCIDLib::TFloat8    f8Long
                            , const TCQCSecToken&       sectUser)
{
    // We require at least power user
    facCQCKit().TestPrivileges(sectUser, tCQCKit::EUserRoles::PowerUser);

    //
    //  Confirm that the values are reasonable by trying a set. If the
    //  passed values aren't coherent, this will cause an exception because
    //  it will create an invalid time.
    //
    TTime tmTest(tCIDLib::ESpecialTimes::CurrentDate);
    tmTest.SetToSunset(f8Lat, f8Long);

    // Now we can lock and update
    TLocker lockrSync(&m_mtxOuter);

    m_f8Lat = f8Lat;
    m_f8Long = f8Long;

    //
    //  Go through and update the time of any sunrise/sunset type actions to take
    //  this into account.
    //
    tCIDLib::TBoolean bChanges = kCIDLib::False;
    const tCIDLib::TCard4 c4Count = m_colSchEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TKeyedCQCSchEvent& csrcCur = m_colSchEvents[c4Index];
        if ((csrcCur.eType() == tCQCKit::ESchTypes::Sunrise)
        ||  (csrcCur.eType() == tCQCKit::ESchTypes::Sunset))
        {
            csrcCur.CalcNextTime(f8Lat, f8Long);
            bChanges = kCIDLib::True;
        }
    }

    //
    //  If any changes we need to resort the time based view and update the list
    //  serial number so that clients will pick up any changes.
    //
    if (bChanges)
    {

        MakeSortedSchedule();
        m_c4SerChanges++;
        m_c4SerSchEvList = m_c4SerChanges;
    }


    facCQCKit().LogMsg
    (
        CID_FILE
        , CID_LINE
        , kEvSrvMsgs::midStatus_LocInfoSet
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , TFloat(f8Lat)
        , TFloat(f8Long)
    );
}



// ---------------------------------------------------------------------------
//  TFacCQCEventSrv: Protected, inherited methods
// ---------------------------------------------------------------------------

// WE just need to deregister our event server interfce
tCIDLib::TVoid TFacCQCEventSrv::DeregisterSrvObjs()
{
    facCIDOrb().DeregisterObject(m_porbsProtoImpl);
}


tCQCSrvFW::EStateRes
TFacCQCEventSrv::eLoadConfig(tCIDLib::TCard4& c4WaitNext, const tCIDLib::TCard4 c4Count)
{
    try
    {
        switch(m_eCfgState)
        {
            case tCQCEventSrv::ECfgStates::GetLocInfo :
            {
                // Get system lat/long info
                TCfgServerClient cfscRepo;
                facCQCKit().bGetLocationInfo
                (
                    tCIDLib::ECSSides::Server, m_f8Lat, m_f8Long, kCIDLib::True, cuctxToUse().sectUser()
                );
                break;
            }

            case tCQCEventSrv::ECfgStates::LoadSched :
            {
                TDataSrvClient dsclToUse(kCIDLib::True);
                LoadSchedEvs(dsclToUse);
                break;
            }

            case tCQCEventSrv::ECfgStates::LoadTrig :
            {
                TDataSrvClient dsclToUse(kCIDLib::True);
                LoadTrigEvs(dsclToUse);
                break;
            }

            case tCQCEventSrv::ECfgStates::LoadEvMons :
            {
                TDataSrvClient dsclToUse(kCIDLib::True);
                LoadEvMonitors(dsclToUse);
                break;
            }

            default :
                break;
        };

        // If it didn't throw, then it worked, so we move forward
        if (facCQCEventSrv.bLogInfo())
        {
            facCQCEventSrv.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_CfgStage
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::Init
                , tCQCEventSrv::strXlatECfgStates(m_eCfgState)
            );
        }
        m_eCfgState++;

        // Set the minimum period to do next state, ignored if we are done
        c4WaitNext = 1000;
    }

    catch(TError& errToCatch)
    {
        if (facCQCEventSrv.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Set a longer retry and fall through
        c4WaitNext = 5000;
    }

    // If not done yet, ask to try again, else indicate we are done
    if (m_eCfgState < tCQCEventSrv::ECfgStates::Success)
        return tCQCSrvFW::EStateRes::Retry;
    return tCQCSrvFW::EStateRes::Success;
}


//
//  Our threads are stopped and server side objects cleaned up, so we can stop the
//  polling engine.
//
tCIDLib::TVoid TFacCQCEventSrv::PostDeregTerm()
{
    m_polleToUse.StopEngine();
}


//
//  We use this to set our stats cache objects before the stuff that would use them
//  gets going. This is before the server side objects are created and way before the
//  we start our various threads.
//
tCIDLib::TVoid TFacCQCEventSrv::PreRegInit()
{
    TStatsCache::RegisterItem
    (
        L"/Stats/CQCEventSrv/CurState"
        , tCIDLib::EStatItemTypes::Counter
        , s_sciCurSrvState
    );
    TStatsCache::SetValue(s_sciCurSrvState, 0);

    TStatsCache::RegisterItem
    (
        L"/Stats/CQCEventSrv/TrigsReceived"
        , tCIDLib::EStatItemTypes::Counter
        , s_sciTrigsReceived
    );
    TStatsCache::SetValue(s_sciTrigsReceived, 0);

    TStatsCache::RegisterItem
    (
        L"/Stats/CQCEventSrv/WorkersBusy"
        , tCIDLib::EStatItemTypes::Value
        , s_sciWorkersBusy
    );
    TStatsCache::SetValue(s_sciWorkersBusy, 0);

    // Start our polling engine
    m_polleToUse.StartEngine(cuctxToUse().sectUser());

    //
    //  We have to install some class loaders so that the standard CQC runtime
    //  and some other CML classes that we allow access to will be loadable. We
    //  also initialize our own event monitor engine, which gets it ready for
    //  use.
    //
    facCQCMEng().InitCMLRuntime(cuctxToUse().sectUser());
    facCQCMedia().InitCMLRuntime();
    facCQCEvMonEng().Initialize(&m_polleToUse, cuctxToUse().sectUser());
}


// We don't use any of the extra values
tCIDLib::TVoid
TFacCQCEventSrv::QueryCoreAdminInfo(TString&    strCoreAdminBinding
                                    , TString&  strCoreAdminDesc
                                    , TString&
                                    , TString&
                                    , TString&
                                    , TString&  )
{
    strCoreAdminBinding = TEventSrvServerBase::strCoreAdminBinding;
    strCoreAdminDesc = L"Scheduled Event Server Core Admin Object";
}



tCIDLib::TVoid TFacCQCEventSrv::RegisterSrvObjs()
{
    // Create and register the actual scheduled event interface server
    m_porbsProtoImpl = new TEventServerImpl;
    facCIDOrb().RegisterObject(m_porbsProtoImpl, tCIDLib::EAdoptOpts::Adopt);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsProtoImpl->ooidThis()
        , TEventSrvServerBase::strBinding
        , L"Scheduled Event Server Protocol"
    );
}


//
//  We have various threads other than the ORB thread farm. So we need to override
//  this and start those up.
//
tCIDLib::TVoid TFacCQCEventSrv::StartWorkerThreads()
{
    // Create and start the event worker threads.
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < kCQCEventSrv::c4EvWorkerThreads; c4Index++)
    {
        m_colWorkerThreads.Add
        (
            new TWorkerThread(c4Index + 1, &m_mtxInner, &m_evWaitWorkers, &m_ctarGVars, cuctxToUse())
        );
        m_colWorkerThreads[c4Index]->Start();
    }

    // Start up the dispatching and queueing threads
    m_thrDispatch.Start();
    m_thrQR.Start();
    m_thrEv.Start();

    //
    //  Go through the registered event monitors and start them running if not paused.
    //  If any fail, they will be marked as having failed.
    //
    m_colEvMonitors.bForEachNC
    (
        [this](TCQCEvMonitor& thrCur) -> tCIDLib::TBoolean
        {
            // Skip it if paused
            if (thrCur.bPaused())
                return kCIDLib::True;

            try
            {
                thrCur.StartMonitor(cuctxToUse());
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCEventSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kEvSrvErrs::errcCfg_StartMonitor
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Init
                    , thrCur.strName()
                );
            }
            return kCIDLib::True;
        }
    );
}


tCIDLib::TVoid TFacCQCEventSrv::StopWorkerThreads()
{
    //
    //  Stop the queuing threads first. This will prevent any new
    //  events from getting queued.
    //
    if (m_thrQR.bIsRunning())
        m_thrQR.ReqShutdownNoSync();
    if (m_thrEv.bIsRunning())
        m_thrEv.ReqShutdownNoSync();

    try
    {
        if (m_thrQR.bIsRunning())
            m_thrQR.eWaitForDeath(5000);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    try
    {
        if (m_thrEv.bIsRunning())
            m_thrEv.eWaitForDeath(5000);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    m_colWorkerThreads.bForEachNC
    (
        [](TWorkerThread& thrCur)
        {
            if (thrCur.bIsRunning())
                thrCur.ReqShutdownNoSync();
            return kCIDLib::True;
        }
    );

    m_colEvMonitors.bForEachNC
    (
        [](TCQCEvMonitor& thrCur)
        {
            if (thrCur.bIsRunning())
                thrCur.ReqShutdownNoSync();
            return kCIDLib::True;
        }
    );

    m_colWorkerThreads.bForEachNC
    (
        [](TWorkerThread& thrCur)
        {
            try
            {
                thrCur.eWaitForDeath(8000);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return kCIDLib::True;
        }
    );

    m_colEvMonitors.bForEachNC
    (
        [](TCQCEvMonitor& thrCur)
        {
            try
            {
                // If was paused then it never ran, so this would fail
                if (!thrCur.bPaused())
                    thrCur.eWaitForDeath(8000);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return kCIDLib::True;
        }
    );
}


// We just need to unbind our event server interface
tCIDLib::TVoid TFacCQCEventSrv::UnbindSrvObjs(tCIDOrbUC::TNSrvProxy&  orbcNS)
{
    orbcNS->RemoveBinding(TEventSrvServerBase::strBinding, kCIDLib::False);
}



// ---------------------------------------------------------------------------
//  TFacCQCRemVComm: Private, static data members
// ---------------------------------------------------------------------------
TStatsCacheItem TFacCQCEventSrv::s_sciCurSrvState;
TStatsCacheItem TFacCQCEventSrv::s_sciTrigsReceived;
TStatsCacheItem TFacCQCEventSrv::s_sciWorkersBusy;



// ---------------------------------------------------------------------------
//  TFacCQCEventSrv: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This thread handles dispatching events from the main queue to the specific threads
//  to handle them. Normally this wouldn't be required bu tbecause of serialized
//  triggered events, we need this extra layer.
//
tCIDLib::EExitCodes TFacCQCEventSrv::eDispatchThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    // We need to do a little state machine
    enum class EStates
    {
        WaitItem
        , StoreSerialized
        , StoreNormal
        , WaitThread
    };

    //
    //  Every minutes or so we will survey the workers and update our workers busy
    //  stat.
    //
    tCIDLib::TEncodedTime enctNextStats = TTime::enctNowPlusMins(1);

    TEventQItem qitemCur;
    TLogLimiter loglimErrs(5 * 60);
    EStates eState = EStates::WaitItem;
    while (!thrThis.bCheckShutdownRequest())
    {
        try
        {
            if (eState == EStates::WaitItem)
            {
                //
                //  Wait for an event to be added to the main queue by the two source
                //  threads.
                //
                //  This provides our throttling as well. If we get one, move to next
                //  state based on whether it's a serialized one or not.
                //
                if (m_colActQ.bGetNext(qitemCur, 500))
                {
                    if (qitemCur.m_bSerialized)
                        eState = EStates::StoreSerialized;
                    else
                        eState = EStates::StoreNormal;
                }
                 else
                {
                    // If it's time, update our stats
                    if (TTime::enctNow() < enctNextStats)
                    {
                        // Reset the stamp first just in case we cause an error below
                        enctNextStats = TTime::enctNowPlusMins(1);

                        tCIDLib::TCard4 c4Count = 0;
                        TLocker lockrSync(&m_mtxInner);
                        for (tCIDLib::TCard4 c4Index = 0;
                            c4Index < kCQCEventSrv::c4EvWorkerThreads; c4Index++)
                        {
                            if (m_colWorkerThreads[c4Index]->bIsActive())
                                c4Count++;
                        }
                        TStatsCache::SetValue(s_sciWorkersBusy, c4Count);
                    }
                }
            }
             else if (eState == EStates::StoreSerialized)
            {
                //
                //  Assume we will not find an existing thread for this guy, and set
                //  the state to normal. If we find one, we'll give it to him and update
                //  bck to waiting for items, else we fall through to store it normally.
                //
                eState = EStates::StoreNormal;
                const TString& strType = qitemCur.m_strEvPath;

                TLocker lockrSync(&m_mtxInner);
                for (tCIDLib::TCard4 c4Index = 0;
                                c4Index < kCQCEventSrv::c4EvWorkerThreads; c4Index++)
                {
                    if (m_colWorkerThreads[c4Index]->bIsHandlingEvType(strType))
                    {
                        m_colWorkerThreads[c4Index]->AddEvent(qitemCur);
                        eState = EStates::WaitItem;
                        break;
                    }
                }
            }
             else if (eState == EStates::StoreNormal)
            {
                // If we have a free thread, then let's give it to hime
                TLocker lockrSync(&m_mtxInner);
                for (tCIDLib::TCard4 c4Index = 0;
                            c4Index < kCQCEventSrv::c4EvWorkerThreads; c4Index++)
                {
                    if (!m_colWorkerThreads[c4Index]->bIsActive())
                    {
                        m_colWorkerThreads[c4Index]->AddEvent(qitemCur);
                        eState = EStates::WaitItem;
                        break;
                    }
                }

                //
                //  If not, then we have to wait for an available thread, so reset the
                //  wait threads event, and go to wait thread state.
                //
                if (eState == EStates::StoreNormal)
                {
                    m_evWaitWorkers.Reset();
                    eState = EStates::WaitThread;
                }
            }
             else if (eState == EStates::WaitThread)
            {
                //
                //  Block a bit on the wait threads event. If it is triggered, then
                //  then we can go to store normal state. Else we stay in this state
                //  and we'll try again next time around.
                //
                //  Here again this keeps us throttled while we wait.
                //
                if (m_evWaitWorkers.bWaitFor(500))
                    eState = EStates::StoreNormal;
            }
        }

        catch(TError& errToCatch)
        {
            if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kEvSrvMsgs::midStatus_ExceptInThread
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"event dispatch")
                );
            }
        }

        catch(...)
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_ExceptInThread
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"event dispatch")
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}



//
//  The event listener thread is started on this method. This guy is always blocked on
//  CQCKit's published event trigger topic. When an event comes in, this guy will run
//  the filters of all the defined triggered events and for any that match it will put
//  them on the queue to be processed.
//
tCIDLib::EExitCodes
TFacCQCEventSrv::eEvThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Pause a bit to let everything settle. In most cases, we are coming up along with
    //  everyone else and the config server will be coming up as well. If asked to stop
    //  while waiting, return now.
    //
    if (!thrThis.bSleep(2000))
        return tCIDLib::EExitCodes::Normal;

    //
    //  Subscribe to CQCKit's event trigger topic. We use an async subscription here since
    //  we are running on a separate thread and we don't want to block CQCKit's sending
    //  out of events. We indicate that the topic doesn't have to be there yet, though
    //  it really should, because this server's startup code should have enabled trigger
    //  publishing.
    //
    TPubSubAsyncSub psasubEvTrigs(kCIDLib::False);
    try
    {
        psasubEvTrigs.SubscribeTo(kCQCKit::strPubTopic_EvTriggers, kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCIDLib::EExitCodes::Normal;
    }

    // Just read events until we are asked to stop
    tCIDLib::TBoolean   bRes;
    tCIDLib::TCard4     c4Hour;
    tCIDLib::TCard4     c4Minute;
    tCIDLib::TCard4     c4Second;
    TPubSubMsg          psmsgTmp;
    TTime               tmNow;
    TTime               tmNoon;
    TTime               tmTmp;
    TLogLimiter         loglimErrs(60);

    while (kCIDLib::True)
    {
        try
        {
            // Check for a shutdown request and break out if so
            if (thrThis.bCheckShutdownRequest())
                break;

            // Block for a bit waiting for an event
            if (!psasubEvTrigs.bGetNextMsg(psmsgTmp, 1000))
                continue;

            // Bump our triggers received stat
            TStatsCache::c8IncCounter(s_sciTrigsReceived);

            //
            //  We got one, so let's see if it needs to be invoked. Put it in a
            //  counted pointer which is how we pass it on to the event processing
            //  stuff. However, we get it in a pub/sub msg which is already ref
            //  counting it. So we have to duplicate it.
            //
            tCQCKit::TCQCEvPtr cptrEvent
            (
                new TCQCEvent(psmsgTmp.objMsgAs<TCQCEvent>())
            );

            // Get the current time info
            tmNow.SetToNow();
            tmNow.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

            //
            //  Figure out if it's nighttime, which is something we have
            //  to provide to the filter evaluation.
            //
            tCIDLib::TBoolean bIsNight;
            {
                tmNoon.SetTo(tCIDLib::ESpecialTimes::NoonToday);
                if (tmNow < tmNoon)
                {
                    //
                    //  It is before noon today, so we are in 0 to 12 part
                    //  of the day. So get the sunrise time. If the current
                    //  time is before that time, then it's night.
                    //
                    tmTmp = tmNoon;
                    tmTmp.SetToSunrise(m_f8Lat, m_f8Long);
                    bIsNight = (tmNow < tmTmp);
                }
                 else
                {
                    //
                    //  It is after noon, so we are in the 12 to 23 part of
                    //  the day. So get the sunset time. If the current time
                    //  is after that time, then it's night.
                    //
                    tmTmp = tmNoon;
                    tmTmp.SetToSunset(m_f8Lat, m_f8Long);
                    bIsNight = (tmNow > tmTmp);
                }
            }

            //
            //  Loop through all our defined triggered events and evaluate
            //  the filters. If any match, load them up into the action queue
            //  for processing.
            //
            {
                // Lock the list while we do this
                TLocker lockrSync(&m_mtxOuter);
                const tCIDLib::TCard4 c4Count = m_colTrgEvents.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    TKeyedCQCTrgEvent& csrcCur = m_colTrgEvents[c4Index];
                    try
                    {
                        //
                        //  Evaluate the filters on this one to see if it
                        //  matches our event.
                        //
                        bRes = csrcCur.bEvaluate
                        (
                            *cptrEvent.pobjData()
                            , bIsNight
                            , tmNow.enctTime()
                            , c4Hour
                            , c4Minute
                        );

                        if (bRes)
                        {
                            //
                            //  It does, so add a queue item. Give it a copy of the
                            //  triggered event and the incoming trigger that triggered
                            //  us. We create an Ex type event, which takes the trigger
                            //  info, and will create an RTV value when the action
                            //  engine asks for one.
                            //
                            m_colActQ.objAdd
                            (
                                TEventQItem
                                (
                                    csrcCur.strPath()
                                    , new TCQCTrgEventEx(csrcCur, cptrEvent)
                                )
                            );
                        }
                    }

                    catch(TError& errToCatch)
                    {
                        if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
                        {
                            facCQCKit().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kKitMsgs::midStatus_ExceptInEval
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::AppStatus
                                , csrcCur.strTitle()
                            );
                        }
                    }

                    catch(...)
                    {
                        facCQCKit().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kKitMsgs::midStatus_ExceptInEval
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , csrcCur.strTitle()
                        );
                    }
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kEvSrvMsgs::midStatus_ExceptInThread
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"event trigger listener")
                );
            }
        }

        catch(...)
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_ExceptInThread
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"event trigger listener")
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  The "queue'er" thread is started up on this method. This guy just  periodically checks
//  the time sorted view of the action list and when any actions are ready, it queues those
//  actions on the worker action queue for worker threads to pick up and do.
//
tCIDLib::EExitCodes TFacCQCEventSrv::eQRThread( TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Pause a bit to let everything settle. In most cases, we are coming
    //  up along with everyone else and the config server will be coming
    //  up as well.
    //
    if (!thrThis.bSleep(3000))
        return tCIDLib::EExitCodes::Normal;

    //
    //  Set up an initial time based view of the events, don't have to lock at this
    //  point since we aren't accepting clients yet.
    //
    MakeSortedSchedule();

    TLogLimiter loglimErrs(60);
    while (kCIDLib::True)
    {
        try
        {
            TLocker lockrSync(&m_mtxOuter);

            // Get the current time, since anything beyond that is ready.
            const tCIDLib::TEncodedTime enctNow(TTime::enctNow());

            //
            //  Now loop through and add any ready ones to the action queue.
            //  Since we are working on a time sorted view, as soon as we
            //  see an action whose scheduled time is less than our now value,
            //  we know no more could be ready this time around.
            //
            tCIDLib::TCard4 c4Count = m_colTimeView.c4ElemCount();
            tCIDLib::TCard4 c4Done = 0;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TKeyedCQCSchEvent& csrcCur = *m_colTimeView[c4Index];

                //
                //  If the scheduled time, is beyond now, then we've done
                //  any that could be scheduled on this round, so break out.
                //  Else, we've got one to deal with.
                //
                if (csrcCur.enctAt() > enctNow)
                    break;

                //
                //  If not paused, then add a copy to the action queue. We update the
                //  next time either way. We use the base class ctor which will slice
                //  the keyed event. We pass in the path separately which is the only
                //  thing from that derived class that we need to keep.
                //
                //  Note that the time won't be updated if it is a one time action.
                //
                if (!csrcCur.bPaused())
                {
                    m_colActQ.objAdd
                    (
                        TEventQItem(csrcCur.strPath(), new TCQCSchEvent(csrcCur))
                    );
                }
                csrcCur.CalcNextTime(m_f8Lat, m_f8Long);

                // Keep up with whether we processed any
                c4Done++;

                // Bump the change serial number and set it on this event
                m_c4SerChanges++;
                csrcCur.c4SerialNum(m_c4SerChanges);
            }

            //
            //  If we did any, then remove any one shots whose time indicates
            //  that they would have been done. And recreate the sorted
            //  view.
            //
            if (c4Done)
            {
                TVector<TString> colRemoved;

                tCIDLib::TCard4 c4Index = 0;
                while (c4Index < c4Count)
                {
                    TKeyedCQCSchEvent& csrcCur = m_colSchEvents[c4Index];

                    // If it's a one-shot and was processed, then remove it
                    if (csrcCur.bIsOneShot() && (enctNow >= csrcCur.enctAt()))
                    {
                        // Remember the path
                        colRemoved.objAdd(m_colSchEvents[c4Index].strPath());

                        // Remove the action from our list now
                        m_colSchEvents.RemoveAt(c4Index);
                        c4Count--;

                        // Adjust the serial numbers appropriately
                        m_c4SerChanges++;
                        m_c4SerSchEvList = m_c4SerChanges;
                    }
                     else
                    {
                        c4Index++;
                    }
                }

                // Update the sorted view since we changed times
                MakeSortedSchedule();

                //
                //  If we removed any, then we need to remove them from the
                //  configuration repository. But there is a chance it won't
                //  be available. This isn't fatal, since we've removed them
                //  from our list, and the next time we run they will be
                //  removed during load if they've already expired. So try
                //  to do it, but don't stress if it doesn't work.
                //
                if (!colRemoved.bIsEmpty())
                {
                    // Bump the change serial number
                    m_c4SerChanges++;

                    try
                    {
                        // Create it in quick test mode
                        TDataSrvClient dsclTar(kCIDLib::True);
                        c4Count = colRemoved.c4ElemCount();
                        for (c4Index = 0; c4Index < c4Count; c4Index++)
                        {
                            dsclTar.DeleteFile
                            (
                                colRemoved[c4Index]
                                , tCQCRemBrws::EDTypes::SchEvent
                                , cuctxToUse().sectUser()
                            );
                        }
                    }

                    catch(TError& errToCatch)
                    {
                        if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
                        {
                            LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kEvSrvMsgs::midStatus_CantRemoveOneShot
                                , tCIDLib::ESeverities::Failed
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }
                    }
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (loglimErrs.bLogErr(errToCatch, CID_FILE, CID_LINE))
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kEvSrvMsgs::midStatus_ExceptInThread
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"sched event queuing")
                );
            }
        }

        catch(...)
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kEvSrvMsgs::midStatus_ExceptInThread
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"sched event queuing")
            );
        }

        //
        //  Sleep a bit. The minimum scheduling period is 1 minute, so it's
        //  not a problem if we sleep for 5 seconds at a time. We don't have
        //  to be super-accurate here.
        //
        //  We use the interruptable form, and break out if we are interrupted
        //  with a shutdown request.
        //
        if (!thrThis.bSleep(5000))
            break;
    }
    return tCIDLib::EExitCodes::Normal;
}

