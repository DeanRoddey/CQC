//
// FILE NAME: CQCEvMonEng_Monitor.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This file implements the actual runtime monitor class that provides the monitor
//  thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEvMonEng_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCEvMonitor,TThread)


// ---------------------------------------------------------------------------
//   CLASS: TCQCEvMonitor
//  PREFIX: emonc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEvMonitor: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEvMonitor::TCQCEvMonitor(const  TString&        strPath
                            , const TCQCEvMonCfg&   emoncSrc) :

    TThread(facCIDLib().strNextThreadName(L"EvMonitorThread_"))
    , m_bSimMode(kCIDLib::False)
    , m_c4IdleSecs(30)
    , m_c4SerialNum(0)
    , m_emoncThis(emoncSrc)
    , m_pmecvHandler(0)
    , m_pmeciHandler(0)
    , m_pmeTarget(0)
    , m_pmefrData(0)
    , m_strPath(strPath)
{
}

TCQCEvMonitor::~TCQCEvMonitor()
{
}


// ---------------------------------------------------------------------------
//  TCQCEvMonitor: Public, non-virtual methods
// ---------------------------------------------------------------------------

// A convenience pass through to the failed status of the monitor config object
tCIDLib::TBoolean TCQCEvMonitor::bFailed() const
{
    return m_emoncThis.bFailed();
}

tCIDLib::TBoolean TCQCEvMonitor::bFailed(const tCIDLib::TBoolean bToSet)
{
    m_emoncThis.bFailed(bToSet);
    return bToSet;
}


// A convenience pass through to the paused status of the monitor config object
tCIDLib::TBoolean TCQCEvMonitor::bPaused() const
{
    return m_emoncThis.bPaused();
}

tCIDLib::TBoolean TCQCEvMonitor::bPaused(const tCIDLib::TBoolean bToSet)
{
    m_emoncThis.bPaused(bToSet);
    return bToSet;
}


tCIDLib::TCard4 TCQCEvMonitor::c4SerialNum() const
{
    return m_c4SerialNum;
}

tCIDLib::TCard4 TCQCEvMonitor::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


//
//  This method does the actual monitor loop. In the normal course of events it is
//  called from the thread that this class creates when the event server creates us
//  upon start up of the monitor.
//
//  When being simulated when running within the IDE this is called from the Simulate
//  method below. In either case, we run until asked to shutdown.
//
tCIDLib::TVoid TCQCEvMonitor::DoMonitorLoop()
{
    //
    //  Get the calling thread. We can't assume assume it's us. In simulator mode
    //  it's the IDE's thread.
    //
    TThread* pthrCaller = TThread::pthrCaller();

    //
    //  We implement a small state machine to keep track of our state. We start trying
    //  to get the handler initialized, then move on to letting it try to load any
    //  config it needs, then regular processing, then exit processing if we ever got
    //  initialized.
    //
    enum EStates
    {
        EState_WaitInit
        , EState_LoadConfig
        , EState_Ready
    };

    //
    //  Let's now enter the main processing loop where we will stay until we are asked
    //  to stop.
    //
    tCIDLib::TEncodedTime enctNextFldPoll = 0;
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctNextIdle = enctCur + (m_c4IdleSecs * kCIDLib::enctOneSecond);
    EStates               eCurState = EState_WaitInit;
    while (kCIDLib::True)
    {
        try
        {
            //
            //  If not in simulator mode, then watch for shutdown requests. We don't
            //  do this in sim mode, since we are not running in our own thread in that
            //  case.
            //
            if (!m_bSimMode)
            {
                if (pthrCaller->bCheckShutdownRequest())
                    break;
            }

            if (eCurState == EState_WaitInit)
            {
                if (bDoInit())
                {
                    // Move to the next state
                    eCurState = EState_LoadConfig;
                }
                 else
                {
                    //
                    //  Wait a while then try it again, If asked to stop, then
                    //  break out.
                    //
                    if (!bMonWait(*pthrCaller, 5000))
                        break;
                }
            }
             else if (eCurState == EState_LoadConfig)
            {
                if (bDoLoadConfig())
                {
                    // Move to the next state
                    eCurState = EState_Ready;
                }
                 else
                {
                    //
                    //  Wait a while then try it again. If asked to stop, then break
                    //  out.
                    //
                    if (!bMonWait(*pthrCaller, 5000))
                        break;
                }
            }
             else if (eCurState == EState_Ready)
            {
                // If time for a poll, then do that
                enctCur = TTime::enctNow();
                if (enctCur > enctNextFldPoll)
                {
                    try
                    {
                        PollFields();
                    }

                    catch(TError& errToCatch)
                    {
                        if (facCQCEvMonEng().bShouldLog(errToCatch))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    enctCur = TTime::enctNow();
                    enctNextFldPoll = enctCur + kCIDLib::enctOneSecond;
                }

                // If time for an idle time callback, then do that
                if (enctCur > enctNextIdle)
                {
                    try
                    {
                        DoIdleCallback();
                    }

                    catch(TError& errToCatch)
                    {
                        if (facCQCEvMonEng().bShouldLog(errToCatch))
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    enctCur = TTime::enctNow();
                    enctNextIdle = enctCur + (m_c4IdleSecs * kCIDLib::enctOneSecond);
                }
            }

            //
            //  Sleep a little bit so we don't go crazy. We don't have any blocking
            //  type operations going on here, so there's nothing to slow us down
            //  otherwise.
            //
            //  In in the IDE this will check for shutdown requests from the IDE, else
            //  it will check for shutdown requests at the thread level.
            //
            if (!bMonWait(*pthrCaller, 150))
                break;
        }

        catch(TError& errToCatch)
        {
            if (facCQCEvMonEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCEvMonEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unhandled exception from event monitor, pausing for a while. Path=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strPath
            );

            if (m_bSimMode)
            {
                // If in simulator mode, rethrow errors back to the IDE
                throw;
            }
             else
            {
                // Make sure we don't get caught in a freakout
                if (!bMonWait(*pthrCaller, 30000))
                    break;
            }
        }

        catch(const TExceptException&)
        {
            //
            //  A CML level exception leaked out, so log it and log a C++ level
            //  error.
            //
            m_pmeTarget->LogLastExcept();
            facCQCEvMonEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Unhandled CML exception from event monitor, pausing for a while. Path=%(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strPath
            );

            if (m_bSimMode)
            {
                // If in simulator mode, rethrow errors back to the IDE
                throw;
            }
             else
            {
                // Make sure we don't get caught in a freakout
                if (!bMonWait(*pthrCaller, 30000))
                    break;
            }
        }

        //
        //  If in simulator mode, we have to give the IDE a chance to stop us. So
        //  we call a method that lets it do that. So we just call it each time
        //  we go through.
        //
        if (m_bSimMode)
        {
            try
            {
                m_pmeTarget->CheckIDEReq();
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }

            catch(const TExceptException&)
            {
                // A CML level exception leaked out
                m_pmeTarget->LogLastExcept();
                facCQCEvMonEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCEvMonErrs::errcMacro_CMLExcept
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , TString(L"IDE Request Check")
                );
            }
        }
    }

    //
    //  Call terminate to give the derived class a chance to clean up, if we ever
    //  got initialized.
    //
    if (eCurState > EState_WaitInit)
    {
        try
        {
            // Make sure we get the stack cleaned back up
            TMEngCallStackJan janStack(m_pmeTarget);

            // No return value so push a void object
            m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

            tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"Terminate");
            m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
            m_pmeciHandler->Invoke
            (
                *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
            );
        }

        catch(TError& errToCatch)
        {
            if (facCQCEvMonEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(const TExceptException&)
        {
            // A CML level exception leaked out
            m_pmeTarget->LogLastExcept();

            facCQCEvMonEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCEvMonErrs::errcMacro_CMLExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TString(L"Terminate callback")
            );
        }
    }
}


// Provide access to our configuration
const TCQCEvMonCfg& TCQCEvMonitor::emoncThis() const
{
    return m_emoncThis;
}

TCQCEvMonCfg& TCQCEvMonitor::emoncThis()
{
    return m_emoncThis;
}


const TString& TCQCEvMonitor::strPath() const
{
    return m_strPath;
}

const TString& TCQCEvMonitor::strPath(const TString& strPath)
{
    return m_strPath;
}


//
//  Update our configuration data. We don't allow this while we are running.
//
tCIDLib::TVoid
TCQCEvMonitor::SetConfig(const TString& strPath, const TCQCEvMonCfg& emoncToSet)
{
    if (bIsRunning())
    {
        facCQCEvMonEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCEvMonErrs::errcCfg_NotWhileRunning
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , m_strPath
        );
    }

    m_strPath = strPath;
    m_emoncThis = emoncToSet;

    //
    //  Clear any potential previous failure, so that we have another shot at it upon
    //  next start.
    //
    m_emoncThis.bFailed(kCIDLib::False);
}


//
//  The CML class calls us here to set its list of fields it wants to monitor. We
//  register them with the polling engine and maintain the polling list.
//
tCIDLib::TVoid TCQCEvMonitor::SetFieldList(const tCIDLib::TStrList& colToSet)
{
    // Pass this to the facility object and let him fill in our poll info list
    facCQCEvMonEng().AddFields(colToSet, m_colFieldList);
}


tCIDLib::TVoid TCQCEvMonitor::SetIdleSecs(const tCIDLib::TCard4 c4ToSet)
{
    m_c4IdleSecs = c4ToSet;
    if (m_c4IdleSecs < 5)
        m_c4IdleSecs = 5;
    else if (m_c4IdleSecs > 300)
        m_c4IdleSecs = 300;
}


//
//  When invoked in the IDE, this is called from the CML class in the IDE, we call
//  the same method that our thread would normally call to kick off the processing
//  that the thread normally would.
//
tCIDLib::TVoid
TCQCEvMonitor::Simulate(        TCIDMacroEngine&    meOwner
                        ,       TMEngClassVal&      mecvInstance
                        , const TString&            strClassPath)
{
    // Set the simulator flag
    m_bSimMode = kCIDLib::True;

    //
    //  And now point our pointer at the value and info class objects and the macro
    //  engine. In this case we don't own them, but the sim mode flag makes it clear
    //  that that is the case.
    //
    m_pmecvHandler = &mecvInstance;
    m_pmeTarget = &meOwner;
    m_pmeciHandler = meOwner.pmeciFind(strClassPath);

    //
    //  Set us in the extra storage of the value object. Normally this is done by  us when
    //  we create the value object in our internal thread, but in this case we didn't
    //  create it.
    //
    m_pmecvHandler->SetExtra(this);

    // And now invoke the processing loop
    DoMonitorLoop();
}



//
//  The event server calls this to start us up when we are running in the normal mode
//  (i.e. not in the IDE.) We try to get the macro loaded, compiled, check that it
//  derives from the correct base class, and the default ctor works.
//
//  One way or another we set the state to not paused, even if we can't start up. In
//  the latter case we also set the failed state.
//
tCIDLib::TVoid TCQCEvMonitor::StartMonitor(const TCQCUserCtx& cuctxToUse)
{
    // One way or another we are no longer marked paused now
    m_emoncThis.bPaused(kCIDLib::False);

    // And until we fail again, assume we will not be failed
    m_emoncThis.bFailed(kCIDLib::False);

    //
    //  Create our macro engine and try to compile the target class that we were given
    //  in the configuration.
    //
    m_pmecvHandler = 0;
    m_pmeciHandler = 0;
    try
    {
        //
        //  This is non-sim mode, so we have to allocate our own macro engine, if not
        //  already (this can get caused again if paused/resumed.
        //
        if (!m_pmeTarget)
            m_pmeTarget = new TCIDMacroEngine;

        //
        //  Set up our macro engine and the handlers we need and get them plugged
        //  into the macro engine.
        //
        TCQCMEngErrHandler          meehLogger;
        TMacroEngParser             meprsToUse;
        m_pmeTarget->SetErrHandler(&meehLogger);

        //
        //  Set up a file loading handler to load from our standard macro root path.
        //  This has to be kept around since it will be used as the macro runs.
        //
        if (!m_pmefrData)
            m_pmefrData = new TMEngFixedBaseFileResolver(facCQCKit().strMacroRootPath());
        m_pmeTarget->SetFileResolver(m_pmefrData);

        //
        //  Try to download and parse the macro. If this fails, we return an error page.
        //  The class object will be owned by the macro engine, added to it by the
        //  parsing process.
        //
        TMEngClassInfo* pmeciTar;
        {
            // Convert the path to dotted for the parse operation
            TString strExpPath;
            facCQCRemBrws().RelPathToCMLClassPath(m_emoncThis.strClassPath(), strExpPath);

            TCQCMEngClassMgr    mecmCQC(cuctxToUse.sectUser());
            TCQCPrsErrHandler   meehParse;
            if (!meprsToUse.bParse(strExpPath, pmeciTar, m_pmeTarget, &meehParse, &mecmCQC))
            {
                facCQCEvMonEng().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCEvMonErrs::errcMacro_CantLoad
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , m_emoncThis.strClassPath()
                );
            }
        }

        //
        //  Make sure it derives from the correct base class, else we can't
        //  use it. If not, then return an error page.
        //
        TEvMonBaseInfo* pmeciParent = m_pmeTarget->pmeciFindAs<TEvMonBaseInfo>
        (
            TEvMonBaseInfo::strPath(), kCIDLib::False
        );

        if (!pmeciParent || (pmeciTar->c2ParentClassId() != pmeciParent->c2Id()))
        {
            facCQCEvMonEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCEvMonErrs::errcMacro_BaseClass
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TEvMonBaseInfo::strPath()
            );
        }

        //
        //  Looks ok, so create the value object for this class. We own this guy so
        //  make sure it gets cleaned up if we throw before we we get out of here.
        //
        TMEngClassVal* pmecvTarget = pmeciTar->pmecvMakeStorage
        (
            L"EventMonitor", *m_pmeTarget, tCIDMacroEng::EConstTypes::NonConst
        );
        TJanitor<TMEngClassVal> janVal(pmecvTarget);

        //
        //  Store us on the value object, in the extra storage. This provides the
        //  needed link back when the CML class makes outgoing calls.
        //
        pmecvTarget->SetExtra(this);

        //
        //  Store the value and info classes in members for later use when invoking
        //  callbacks.
        //
        //  NOTE: We need to set these before we call the ctor, but the value object
        //  is also in a janitor, and so we need to be sure it gets cleared if we
        //  throw. Else our cleanup will try to delete it again!
        //
        m_pmecvHandler = pmecvTarget;
        m_pmeciHandler = pmeciTar;

        // Call its default constructor. If it fails, return an error page
        if (!m_pmeTarget->bInvokeDefCtor(*pmecvTarget, &cuctxToUse))
        {
            facCQCEvMonEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCEvMonErrs::errcMacro_DefCtor
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , m_emoncThis.strClassPath()
            );
        }

        //
        //  Release the value object janitor now since we are going to start up
        //  the thread. At this point the monitor thread owns it.
        //
        janVal.Orphan();
    }

    catch(TError& errToCatch)
    {
        // Mark us failed
        m_emoncThis.bFailed(kCIDLib::True);

        // Clear object pointers. the janitor will have cleaned up the value
        m_pmecvHandler = 0;
        m_pmeciHandler = 0;

        // Now cleanup any other stuff
        Cleanup();

        // Log a followup msg
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        facCQCEvMonEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCEvMonErrs::errcMacro_ExceptLoad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    catch(...)
    {
        // Mark us failed
        m_emoncThis.bFailed(kCIDLib::True);

        // Clear object pointers. the janitor will have cleaned up the value
        m_pmecvHandler = 0;
        m_pmeciHandler = 0;

        // Now cleanup any other stuff
        Cleanup();

        facCQCEvMonEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCEvMonErrs::errcMacro_ExceptLoad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Start us up at the thread level
    Start();
}


//
//  If this monitor is running, we ask it to stop
//
tCIDLib::TVoid TCQCEvMonitor::StopMonitor()
{
    if (bIsRunning())
    {
        try
        {
            ReqShutdownSync(8000);
            eWaitForDeath(5000);

            // We are now paused
            m_emoncThis.bPaused(kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            throw;
        }
    }
     else
    {
        // Make sure our paused state is in sync with the thread state
        m_emoncThis.bPaused(kCIDLib::False);
    }
}



// ---------------------------------------------------------------------------
//  TCQCEvMonitor: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  The monitor thread is started here. This is never called if we are being run in
//  simulator mode. Instead Simulate() above is called by the CML class when it starts.
//  When running normally, the event server will call our StartMon() method which will
//  make sure the monitor can be compiled and initialized, then it starts up the
//  thread here.
//
tCIDLib::EExitCodes TCQCEvMonitor::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        DoMonitorLoop();
    }

    // Be extra safe
    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCEvMonEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"An unhandled exception occurred in the event monitor thread"
            , tCIDLib::ESeverities::Info
        );
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        facCQCEvMonEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"An unhandled exception occurred in the event monitor thread"
            , tCIDLib::ESeverities::Info
        );
        return tCIDLib::EExitCodes::RuntimeError;
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TCQCEvMonitor: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCEvMonitor::bDoInit()
{
    // Make sure we get the stack cleaned back up
    TMEngCallStackJan janStack(m_pmeTarget);

    const tCIDLib::TCard4 c4RetInd = m_pmeTarget->c4StackTop();

    // Push the return value
    m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);

    //
    //  We have to break out the parameters and pass them in as a CML level vector
    //  of strings. So we have to create a CML level vector for temporary use and
    //
    tCIDLib::TStrList colParams;
    try
    {
        if (!m_emoncThis.strParams().bIsEmpty())
            TExternalProcess::c4BreakOutParms(m_emoncThis.strParams(), colParams);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        return kCIDLib::False;
    }

    //
    //  Create the CML level vector and load it up. We have to look up the vector
    //  type id and then create a new one.
    //
    const tCIDLib::TCard2 c2VecId
    (
        m_pmeTarget->c2FindClassId(TMEngVectorInfo::strPath())
    );
    TMEngVectorVal* pmecvParams = new TMEngVectorVal
    (
        L"Params"
        , c2VecId
        , tCIDLib::TCard2(tCIDMacroEng::EIntrinsics::String)
        , tCIDMacroEng::EConstTypes::Const
        , m_pmeTarget->pmeciFindAs<TMEngCollectInfo>(TMEngCollectInfo::strPath())
    );
    const tCIDLib::TCard4 c4Count = colParams.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pmecvParams->AddObject
        (
            new TMEngStringVal
            (
                TString::strEmpty(), tCIDMacroEng::EConstTypes::Const, colParams[c4Index]
            )
        );
    }

    // Push it as A LOCAL! This insures it gets cleaned up when the stack is popped
    m_pmeTarget->PushValue(pmecvParams, tCIDMacroEng::EStackItems::Local);

    tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"Initialize");
    m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
    m_pmeciHandler->Invoke
    (
        *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
    );

    // Get the return value
    return m_pmeTarget->mecvStackAtAs<TMEngBooleanVal>(c4RetInd).bValue();
}



tCIDLib::TBoolean TCQCEvMonitor::bDoLoadConfig()
{
    // Make sure we get the stack cleaned back up
    TMEngCallStackJan janStack(m_pmeTarget);

    const tCIDLib::TCard4 c4RetInd = m_pmeTarget->c4StackTop();
    m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Boolean, tCIDMacroEng::EConstTypes::NonConst);

    tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"LoadConfig");
    m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
    m_pmeciHandler->Invoke
    (
        *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
    );

    // Get the return value
    return m_pmeTarget->mecvStackAtAs<TMEngBooleanVal>(c4RetInd).bValue();
}



//
//  Called from the processing loop when we need to sleep. We have to do it a number
//  of places so we break it out. If we are asked to shutdown while we wait, then
//  we return false.
//
tCIDLib::TBoolean
TCQCEvMonitor::bMonWait(TThread& thrCaller, const tCIDLib::TCard4 c4Millis)
{
    //
    //  In sim mode we are running via the IDE, so just do the macro engine sleep
    //  call which can break out via IDE request. Else do a thread based sleep and
    //  watch for shutdown requests.
    //
    if (m_bSimMode)
    {
        m_pmeTarget->Sleep(c4Millis);
        return kCIDLib::True;
    }

    return thrCaller.bSleep(c4Millis);
}


//
//  This is called to clean up macro oriented resources when we are invoked in the
//  normal way within the event server. In that case we created these things, not
//  the IDE, so we need to clean them up. The class info object the macro engine owns
//  either way.
//
tCIDLib::TVoid TCQCEvMonitor::Cleanup()
{
    // Clean up the stuff we created above
    try
    {
        delete m_pmecvHandler;
        m_pmecvHandler = 0;
    }

    catch(...)
    {
        m_pmecvHandler = 0;
    }

    try
    {
        delete m_pmefrData;
        m_pmefrData = 0;
    }

    catch(...)
    {
        m_pmefrData = 0;
    }

    try
    {
        delete m_pmeTarget;
        m_pmeTarget = 0;
    }

    catch(...)
    {
        m_pmeTarget = 0;
    }
}


//
//  Call the CML handler's Idle callback method
//
tCIDLib::TVoid TCQCEvMonitor::DoIdleCallback()
{
    // Make sure we get the stack cleaned back up
    TMEngCallStackJan janStack(m_pmeTarget);

    // No return so push a void
    m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

    tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"Idle");
    m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
    m_pmeciHandler->Invoke
    (
        *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
    );
}


//
//  This is called periodically from the monitor processing loop to watch for any
//  field changes and to inform the CML handler if so
//
tCIDLib::TVoid TCQCEvMonitor::PollFields()
{
    const tCIDLib::TCard4 c4Count = m_colFieldList.c4ElemCount();
    if (c4Count)
    {
        TString strVal;

        TCQCPollEngine& polleTar = facCQCEvMonEng().polleThis();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCFldPollInfo& cfpiCur = m_colFieldList[c4Index];

            // Remember the current state then do an update
            if (cfpiCur.bUpdateValue(polleTar))
            {
                try
                {
                    // Get the value formatted out if a good value, else an empty string
                    const tCIDLib::TBoolean bGoodValue = cfpiCur.bHasGoodValue();
                    if (bGoodValue)
                        cfpiCur.fvCurrent().Format(strVal);
                    else
                        strVal.Clear();

                    // Make sure we get the stack cleaned back up
                    TMEngCallStackJan janStack(m_pmeTarget);

                    // No return value so push a void object
                    m_pmeTarget->PushPoolValue(tCIDMacroEng::EIntrinsics::Void, tCIDMacroEng::EConstTypes::NonConst);

                    // Push the four values
                    m_pmeTarget->PushString(cfpiCur.strMoniker(), tCIDMacroEng::EConstTypes::Const);
                    m_pmeTarget->PushString(cfpiCur.strFieldName(), tCIDMacroEng::EConstTypes::Const);
                    m_pmeTarget->PushString(strVal, tCIDMacroEng::EConstTypes::Const);
                    m_pmeTarget->PushBoolean(bGoodValue, tCIDMacroEng::EConstTypes::Const);

                    tCIDLib::TCard2 c2MethId = m_pmeciHandler->c2FindMethod(L"FieldChanged");
                    m_pmeTarget->meciPushMethodCall(m_pmeciHandler->c2Id(), c2MethId);
                    m_pmeciHandler->Invoke
                    (
                        *m_pmeTarget, *m_pmecvHandler, c2MethId, tCIDMacroEng::EDispatch::Poly
                    );
                }

                catch(TError& errToCatch)
                {
                    if (facCQCEvMonEng().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                }

                catch(const TExceptException&)
                {
                    // A CML level exception leaked out
                    m_pmeTarget->LogLastExcept();

                    facCQCEvMonEng().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCEvMonErrs::errcMacro_CMLExcept
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Internal
                        , TString(L"FieldChanged callback")
                    );
                }
            }
        }
    }
}


