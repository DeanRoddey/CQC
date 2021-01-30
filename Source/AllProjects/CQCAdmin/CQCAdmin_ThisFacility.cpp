//
// FILE NAME: CQCAdmin_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2000
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
//  CQC client. Some private methods, which implement various specific pieces
//  of the server, are implemented in other files.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCAdmin,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAdmin
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCAdmin: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCAdmin::TFacCQCAdmin() :

    TGUIFacility
    (
        L"CQCAdmin"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
{
}

TFacCQCAdmin::~TFacCQCAdmin()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCAdmin: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Provide access to our user context to those bits that need it.
const TCQCUserCtx& TFacCQCAdmin::cuctxToUse() const
{
    return m_cuctxToUse;
}


//
//  The program entry point.
//
tCIDLib::EExitCodes
TFacCQCAdmin::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
    {
        TErrBox msgbTest
        (
            strMsg(kCQCAMsgs::midWnd_AppName)
            , strMsg(kCQCAMsgs::midStatus_BadEnv, strFailReason)
        );
        msgbTest.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    tCIDLib::EExitCodes eExit = tCIDLib::EExitCodes::RuntimeError;
    TLogSrvLogger* plgrLogSrv = nullptr;
    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit.
        //
        {
            TResourceName rsnInstance(L"CQSL", L"CQCAdmin", L"SingleInstanceInfo");
            if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::True))
                return tCIDLib::EExitCodes::Normal;
        }

        //
        //  The very first thing we want to do is the most fundamental boot-
        //  strapping. We need to crank up the Orb client support (so that
        //  we can find the name server.) And then install a log server
        //  logger, which will allow anything that goes wrong after that to
        //  go to the log server.
        //
        facCIDOrb().InitClient();

        //
        //  The next thing we want to do is to install a log server logger. We
        //  just use the standard one that's provided by CIDLib. It logs to
        //  the standard CIDLib log server, and uses a local file for fallback
        //  if it cannot connect.
        //
        plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        if (bLogInfo())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midApp_Startup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"CQCAdmin")
                , facCQCAdmin.strVersion()
            );
        }

        // Parse the parameters
        tCIDLib::TBoolean bNoState = kCIDLib::False;
        if (!bParseParams(bNoState))
            return tCIDLib::EExitCodes::BadParameters;

        //
        //  Wait for the back end to come up, in case it hasn't yet. Just
        //  make it wait for the data server's security server.
        //
        const TString strAppText(kCQCAMsgs::midWnd_AppName, facCQCAdmin);
        if (!facCQCIGKit().bWaitForCQC( TWindow::wndDesktop()
                                        , strAppText
                                        , TCQCSecureClientProxy::strBinding
                                        , 60000))
        {
            return tCIDLib::EExitCodes::PermissionLevel;
        }

        // Do a version check against the master server and exit if not valid
        if (!facCQCIGKit().bCheckCQCVersion(TWindow::wndDesktop(), strAppText))
            return tCIDLib::EExitCodes::InitFailed;

        //
        //  Before we create any window, lets ask the user to log on. We put
        //  up a modal dialog for this. If they can't log on ok, then we
        //  get back out now. If they do, this method will store the security
        //  info with the CQCKit facility, where everyone can get to it.
        //
        if (!bDoLogon())
        {
            // For this scenario, we just clean up the bits we've done and exit
            plgrLogSrv->bForceLocal(kCIDLib::True);
            facCIDOrb().Terminate();
            return tCIDLib::EExitCodes::Normal;
        }

        // Start up our polling engine
        m_polleToUse.StartEngine(m_cuctxToUse.sectUser());

        //
        //  We have to install some class loaders on the macro engine facility, so that
        //  various CML classes are available to the code we load into ourself.
        //
        //  For the event monitor we have to give him access to our polling engine for
        //  his polling use. This keeps it efficient, i.e. no duplicate polling for the
        //  same field info.
        //
        facCQCMEng().InitCMLRuntime(m_cuctxToUse.sectUser());
        facCQCMedia().InitCMLRuntime();
        facCQCEvMonEng().Initialize(&m_polleToUse, m_cuctxToUse.sectUser());
        facCQCWebSrvC().InitCMLRuntime();
        facCQCGenDrvS().InitCMLRuntime();

        //
        //  We want the interface engine to be designer mode inside this process. This
        //  prevents it from assigning viewing time ids to any widgets have not have not
        //  been given ids by the user. We only allow editing of templates, so we can just
        //  set it and leave it.
        //
        facCQCIntfEng().bDesMode(kCIDLib::True);

        //
        //  Init the object store now, sinced it looks like we are going to run now.
        //  We will check for some standard stuff and add default info if not found.
        //  This way, never have to check later when we want to update it, because we
        //  know we've already added them.
        //
        //  If it finds our main state info, it will return it to us.
        //
        //  NOTE: MUST BE DONE after login, since it uses the current logged
        //          in user's name.
        //
        TCQCCAppState fstInit;
        InitObjectStore(fstInit, bNoState);

        // Now that the object store is ready, load any stored app attach info
        facCQCGKit().LoadAppAttachInfo();

        //
        //  Register any extended COM port factories, e.g. GC-100 or remote
        //  port server as required.
        //
        RegisterCOMFactories();

        // Start up the media database cacher
        facCQCMedia().StartMDBCacher();

        // Set up the IV engine. We have to provide him with a polling engine
        facCQCIntfEng().StartEngine(&m_polleToUse);

        //
        //  Try to create the main frame (which will create an MDI client
        //  window as its client.
        //
        if (m_wndMain.bCreateMain(fstInit))
        {
            //
            //  Though we have no objects to rebind, we start the rebinder since
            //  it pings the name server periodically for us to insure we recover
            //  quickly if it cycles.
            //
            facCIDOrbUC().StartRebinder();

            //
            //  Enable the event processing system for sending events. We need to support
            //  sending since we allow actions to send event triggers, and one might be
            //  run by this process.
            //
            //  We do use receiving in the event trigger monitoring tab. But we only turn
            //  on reading when we have that tab running.
            //
            facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Send, m_cuctxToUse.sectUser());

            //
            //  And activate our main frame and tell it to start a timer that
            //  periodically flushes our state to the object store.
            //
            //  NOTE:   We have to start the timer after the init work is fully
            //          done, because that way we can do all the startup stuff
            //          without worrying about locking the object store.
            //
            m_wndMain.SetupComplete();
        }
    }

    catch(TError& errToCatch)
    {
        // If its not already logged, then log it
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (m_wndMain.bIsValid())
        {
            TExceptDlg dlgErr
            (
                m_wndMain.bIsValid() ? m_wndMain : TWindow::wndDesktop()
                , strMsg(kCQCAMsgs::midWnd_AppName)
                , errToCatch.strErrText()
                , errToCatch
            );
        }
         else
        {
            TExceptDlg dlgErr
            (
                m_wndMain.bIsValid() ? m_wndMain : TWindow::wndDesktop()
                , facCQCAdmin.strMsg(kCQCAMsgs::midWnd_AppName)
                , errToCatch.strErrText()
                , errToCatch
            );
        }
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        // All we can do is pop up a message box
        TErrBox msgbTest
        (
            strMsg(kCQCAMsgs::midWnd_AppName)
            , strMsg(kCQCAMsgs::midStatus_SysException)
        );
        msgbTest.ShowIt(m_wndMain.bIsValid() ? m_wndMain : TWindow::wndDesktop());
        return tCIDLib::EExitCodes::InitFailed;
    }

    //
    //  Call the window processing loop. We won't come back until the
    //  program exits. No exceptions will come out of here, so we don't
    //  have to protect it.
    //
    if (m_wndMain.bIsValid())
        facCIDCtrls().uMainMsgLoop(m_wndMain);

    if (bLogInfo())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_NormalExit
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    eExit = tCIDLib::EExitCodes::Normal;

    // Do the cleanup
    try
    {
        // Stop the ORB's rebinder thread
        facCIDOrbUC().StopRebinder();

        // Try to stop the logic server graph poller if it got started
        facCQLogicSh().StopPoller();

        // Stop the media DB cacher thread
        facCQCMedia().StopMDBCacher();

        // Store any app attach info
        facCQCGKit().StoreAppAttachInfo();

        // And now free them all up
        facCQCKit().FreeApps();

        // Disable event processing now
        facCQCKit().StopEventProcessing();

        //
        //  And finally destroy our main window. This will clean up all of the
        //  currently opened tabs. We have to pump some messages after this so that
        //  destroy messages get dispatched.
        //
        m_wndMain.Destroy();
        facCIDCtrls().MsgYield(100);

        //
        //  Clean up the interface engine. Do this after we kill the window so
        //  that we are sure windows that reference it are gone.
        //
        facCQCIntfEng().StopEngine();

        //
        //  Clean up the event monitor facility. It's referencing our polling engine
        //  as well.
        //
        facCQCEvMonEng().Terminate();

        //
        //  Stop the polling engine now that no one could be using it (including
        //  the facilities we just stopped above.)
        //
        m_polleToUse.StopEngine();

        // Terminate the local object store
        facCQCGKit().TermObjectStore();

        //
        //  Force the log server logger to local logging. We need to do this before
        //  we terminate the ORB.
        //
        if (plgrLogSrv)
            plgrLogSrv->bForceLocal(kCIDLib::True);

        //
        //  Terminate the Orb support. We've cleaned any users of it at this point, so we
        //  should be safe doing it.
        //
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eExit = tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        eExit = tCIDLib::EExitCodes::RuntimeError;
    }

    // And finally make the log server logger stop logging
    try
    {
        if (plgrLogSrv)
            plgrLogSrv->Cleanup();
    }

    catch(...)
    {
        eExit = tCIDLib::EExitCodes::RuntimeError;
    }
    return eExit;
}


//
//  A convenince wrapper, which we just pass on to our user context object.
//
tCQCKit::EUserRoles TFacCQCAdmin::eUserRole() const
{
    return m_cuctxToUse.eUserRole();
}


// A convenience wrapper that we just pass on to the user context
const TCQCSecToken& TFacCQCAdmin::sectUser() const
{
    return m_cuctxToUse.sectUser();
}


// A conveniene wrapper that we just pass on to our user context
const TString& TFacCQCAdmin::strUserName() const
{
    return m_cuctxToUse.strUserName();
}


//
//  Provide access to our polling engine. Some of the tabs will make use of it
//  if they need to monitor drivers.
//
const TCQCPollEngine& TFacCQCAdmin::polleToUse() const
{
    return m_polleToUse;
}

TCQCPollEngine& TFacCQCAdmin::polleToUse()
{
    return m_polleToUse;
}


// ---------------------------------------------------------------------------
//  TFacCQCAdmin: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Just a convenience to wrap around the login process and set up our user
//  contents info that we pass into any actions or other calls that require
//  user context info.
//
tCIDLib::TBoolean TFacCQCAdmin::bDoLogon()
{
    TCQCSecToken sectTmp;
    if (facCQCGKit().bLogon(TWindow::wndDesktop()
                            , sectTmp
                            , m_uaccLogin
                            , tCQCKit::EUserRoles::LimitedUser
                            , facCQCAdmin.strMsg(kCQCAMsgs::midWnd_AppName)
                            , kCIDLib::False
                            , L"AdminIntf"))
    {
        m_cuctxToUse.Set(m_uaccLogin, sectTmp);
        m_cuctxToUse.LoadEnvRTVsFromHost();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



tCIDLib::TBoolean TFacCQCAdmin::bParseParams(tCIDLib::TBoolean& bNoState)
{
    //
    //  Most of the parms we deal with are handled by CQCKit generically during
    //  startup. But we do watch for a /NoState parm, to tell us not to try to
    //  load our persistent state.
    //
    bNoState = kCIDLib::False;

    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    if (cursParms)
        bNoState = cursParms->bCompareI(L"/NoState");

    return kCIDLib::True;
}

