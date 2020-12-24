//
// FILE NAME: CQCTrayMon_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2004
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
//  This file implements the facility class for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We need to bring the platform specific service header here
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"
#include    "CIDKernel_ServiceWin32.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCTrayMon,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTrayMon
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCTrayMon: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCTrayMon::TFacCQCTrayMon() :

    TGUIFacility
    (
        L"CQCTrayMon"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_eLastAppSrvState(tCQCTrayMon::ESrvStates::Count)
    , m_eLastClSrvState(tCQCTrayMon::ESrvStates::Count)
    , m_eVerboseLvl(tCQCKit::EVerboseLvls::Off)
    , m_c4MaxClients(0)
    , m_ippnListen(kCQCKit::ippnTrayMonDefPort)
    , m_plgrLogSrv(nullptr)
    , m_porbsCoreAdmin(nullptr)
    , m_porbsBrowser(nullptr)
    , m_thrMonitor
      (
        TString(L"CQCTrayMonitor")
        , TMemberFunc<TFacCQCTrayMon>(this, &TFacCQCTrayMon::eMonThread)
      )
{
}

TFacCQCTrayMon::~TFacCQCTrayMon()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCTrayMon: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The main program entry point. We do all the setup, create the tray and
//  main windows, and then wait for a close request.
//
tCIDLib::EExitCodes
TFacCQCTrayMon::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        return tCIDLib::EExitCodes::BadEnvironment;

    const TString strAppText(kTrayMonMsgs::midMain_Title, *this);
    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit. Tell it not try to bring
        //  the old one forward, since it's not a normal window and would
        //  fail anyway.
        //
        TResourceName rsnInstance(L"CQSL", L"CQCTrayMon", L"SingleInstanceInfo");
        if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::False))
            return tCIDLib::EExitCodes::Normal;

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
        m_plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(m_plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        // Log that we are starting up
        if (bLogInfo())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_Starting
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , facCQCTrayMon.strVersion()
            );
        }

        // And now we parse our parameters
        tCIDLib::TCard4 bAppCtrl;
        tCIDLib::TCard4 biTunes;
        tCIDLib::TCard4 c4StartDelay;
        ParseParams(c4StartDelay, bAppCtrl, biTunes);

        // If no binding is set, default to the local host
        if (m_strBinding.bIsEmpty())
            m_strBinding = TSysInfo::strIPHostName();

        //
        //  If they indicated a startup delay, then let's wait for that
        //  number of seconds. The delay is in seconds.
        //
        if (c4StartDelay)
            facCIDCtrls().MsgYield(c4StartDelay * 1000);


        // Now init the server side of the ORB
        facCIDOrb().InitServer(m_ippnListen, m_c4MaxClients);

        //
        //  Load the icon we'll use. We'll pass it to the tray window which
        //  needs to pass it in each time we update the tray icon and we'll
        //  set it on the main frame.
        //
        TIcon iconTray(facCQCTrayMon, kCQCTrayMon::ridIco_Tray);

        // Create and register our server side interfaces
        CreateServerObjects();

        //
        //  And let's create the main frame window. It'll be initially
        //  invisible. It will load up and create all of the tabs required.
        //  They will create any other server side interfaces they need
        //  to have.
        //
        m_wndMain.Create(bAppCtrl, biTunes, iconTray);

        //
        //  And now create the main tray app. Give it a known text value
        //  so that the installer can find the (otherwise not very easily
        //  findable) main window to force a shutdown.
        //
        m_wndTray.CreateTrayApp
        (
            TArea(10, 10, 100, 100), iconTray, L"CQCTrayMonWnd"
        );

        //
        //  Start up the service  monitoring thread. First, get an initial, non-
        //  displaying state as our stating point.
        //
        StoreSrvState(kCIDLib::False);
        m_thrMonitor.Start();

        // Start the auto-rebinder now
        facCIDOrbUC().StartRebinder();

        //
        //  Call the window processing loop. We won't come back until the
        //  program exits.
        //
        facCIDCtrls().uMainMsgLoop(m_wndTray);

        if (facCQCTrayMon.bLogInfo())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_NormalExit
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCTrayMon.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        TExceptDlg dlgbFail
        (
            TWindow::wndDesktop()
            , strAppText
            , strMsg(kTrayMonMsgs::midStatus_InitFailed)
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    //
    //  We first need to shut down any of our pluggable interfaces, before
    //  we destroy the main frame window, so they won't try to interact
    //  with windows that aren't there anymore.
    //
    Cleanup();

    // Now we can destroy the main frame
    m_wndMain.Destroy();

    //
    //  And finally destroy the tray window, which will remove us from the
    //  system tray.
    //
    m_wndTray.Destroy();

    // Force the log server logger to local logging and terminate the ORB
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->bForceLocal(kCIDLib::True);
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        if (bLogInfo())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }

    //
    //  Shut the logger object's thread down. We told TModule to adopt it so he
    //  will clean it up.
    //
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->Cleanup();
    }

    catch(TError& errToCatch)
    {
        if (bLogInfo())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    return tCIDLib::EExitCodes::Normal;
}


// Get/set the program logging verbosity
tCQCKit::EVerboseLvls TFacCQCTrayMon::eVerboseLvl() const
{
    return m_eVerboseLvl;
}

tCQCKit::EVerboseLvls
TFacCQCTrayMon::eVerboseLvl(const tCQCKit::EVerboseLvls eToSet)
{
    return m_eVerboseLvl = eToSet;
}


//
//  Provide access to our configured binding name, so that any code that
//  needs to doing name server bindings can build up the binding names.
//
const TString& TFacCQCTrayMon::strBinding() const
{
    return m_strBinding;
}


//
//  The main window calls this when we go to sleep. We need to shut down
//  certain stuff.
//
//  Note taht the sleep time is pretty draconian and we may not be able
//  to get all our stuff cleaned up. So we have to be very tolerant of
//  this. So we call the cleanup method again before we start up.
//
tCIDLib::TVoid TFacCQCTrayMon::SleepWake(const tCIDLib::TBoolean bWakingUp)
{
    if (bWakingUp)
    {
        Cleanup();

        // Create and register our server side objects
        CreateServerObjects();

        // Ask the main window to crank it's interfaces back up
        m_wndMain.CreateTabs();

        // Get ourself back in sync with the service status
        StoreSrvState(kCIDLib::False);
        m_thrMonitor.Start();

        // Start the auto-rebinder now
        facCIDOrbUC().StartRebinder();
    }
     else
    {
        // Call Cleanup to do most of the work
        Cleanup();
    }
}


//
//  A wrapper that the other windows or plugins or threads can use to cause a
//  tray popup notification to be displayed when needed.
//
tCIDLib::TVoid
TFacCQCTrayMon::ShowNotification(const  TString&            strTitle
                                , const TString&            strText
                                , const tCIDLib::TBoolean   bError)
{
    // Just pass it on to the tray window if it exists
    if (m_wndTray.bIsValid())
        m_wndTray.ShowBalloonText(strTitle, strText, bError, 5000);
}


//
//  Allow the core admin remote interface to call this to make us exit
//
tCIDLib::TVoid TFacCQCTrayMon::Shutdown(const tCIDLib::EExitCodes eExit)
{
    facCIDCtrls().ExitLoop(eExit);
}


//
//  When the (hidden) tray window is notified that the user clicked on our
//  tray icon, this is called. If the main frame is hidden, we show it, and
//  vice versa.
//
tCIDLib::TVoid TFacCQCTrayMon::ToggleFrameVis()
{
    m_wndMain.SetVisibility(!m_wndMain.bIsVisible());
}


// ---------------------------------------------------------------------------
//  TFacCQCTrayMon: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TFacCQCTrayMon::bCheckAService( const   tCIDLib::TCh* const         pszName
                                , const tCQCTrayMon::ESrvStates     ePrev
                                ,       tCQCTrayMon::ESrvStates&    eToSet)
{
    // Try to open the service
    tCIDKernel::TWSvcHandle hsvcSrv = TKrnlWin32Service::hsvcOpen(pszName);

    tCIDLib::TBoolean bRes = kCIDLib::False;
    if (hsvcSrv)
    {
        // Get the stopped/started status of the service and close the handle
        tCIDKernel::EWSrvStates eState;
        if (TKrnlWin32Service::bQueryState(hsvcSrv, eState))
        {
            if (eState == tCIDKernel::EWSrvStates::Running)
                eToSet = tCQCTrayMon::ESrvStates::Started;
            else if (eState == tCIDKernel::EWSrvStates::Start_Pending)
                eToSet = tCQCTrayMon::ESrvStates::Starting;
            else if (eState == tCIDKernel::EWSrvStates::Stop_Pending)
                eToSet = tCQCTrayMon::ESrvStates::Stopping;
            else if (eState == tCIDKernel::EWSrvStates::Stopped)
                eToSet= tCQCTrayMon::ESrvStates::Stopped;
        }
    }
     else
    {
        // Couldn't open it, so go with not found status
        eToSet = tCQCTrayMon::ESrvStates::NotFound;
    }

    // Return if it changed
    return (eToSet != ePrev);
}


//
//  A helper to check the CQC service state. We will store the new status
//  and if asked we'll tell the tray app to show a status popup if the
//  status changed.
//
tCIDLib::TVoid TFacCQCTrayMon::StoreSrvState(const tCIDLib::TBoolean bShow)
{
    // Check the app shell
    tCQCTrayMon::ESrvStates eAppShState;
    tCIDLib::TBoolean bAppShChange = bCheckAService
    (
        kCQCKit::pszCQCShellName, m_eLastAppSrvState, eAppShState
    );

    // Check the client service
    tCQCTrayMon::ESrvStates eClState;
    tCIDLib::TBoolean bClChange = bCheckAService
    (
        kCQCKit::pszClServiceName, m_eLastClSrvState, eClState
    );

    // If either changed, then update
    if (bAppShChange || bClChange)
    {
        m_eLastClSrvState = eClState;
        m_eLastAppSrvState = eAppShState;
        m_wndTray.ShowServiceStatus(eClState, eAppShState, !bShow);
    }
}


//
//  Called during shutdown to clean up. It's also called when the system
//  is going to sleep. That's why this guy doesn't destroy the main window
//  or the tray window, but it does ask the main window to clean up the
//  tabs, to shut down all our functionality. We'll have to start it back
//  up again when the system comes bakc to life. We need the tray window
//  in order to receive the power up notification.
//
//  We also don't remove the logger. It can stay there easily enough,
//  and will insure that any errors that happen on the way down or back
//  up get logged. And we don't terminate the ORB, we just make sure all
//  our server side objects are cleaned up and deregistered.
//
//  The extra bits are done on the way out of main thread method after
//  the main window exists.
//
tCIDLib::TVoid TFacCQCTrayMon::Cleanup()
{
    //
    //  Stop the ORB autobinder so it won't start trying to rebind stuff as
    //  we are trying to unbind them.
    //
    facCIDOrbUC().StopRebinder();

    // Stop the monitor thread if we started it
    if (m_thrMonitor.bIsRunning())
    {
        try
        {
            m_thrMonitor.ReqShutdownSync(10000);
            m_thrMonitor.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (bLogInfo())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }
        }
    }


    //
    //  Get a name server proxy and clean up all bindings. If the name
    //  server client proxy creation fails, we'll throw to down past all
    //  this unbinding stuff.
    //
    try
    {
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

        // Do the tab based ones first
        if (m_wndMain.bIsValid())
            m_wndMain.RemoveBindings(orbcNS);

        // Now remove our app level bindings, starting with the core admin
        TString strBind;
        try
        {
            strBind = kCQCKit::pszCQCTrayMonCoreAdmin;
            strBind.eReplaceToken(m_strBinding, kCIDLib::chLatin_b);
            orbcNS->RemoveBinding(strBind, kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            if (bLogInfo())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_UnbindErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"core admin")
            );
        }

        // Remove the remote browsing interface
        try
        {
            strBind = kCQCKit::pszCQCTrayMonBrowser;
            strBind.eReplaceToken(m_strBinding, kCIDLib::chLatin_b);
            orbcNS->RemoveBinding(strBind, kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_UnbindErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"remote browser")
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kTrayMonMsgs::midStatus_NSCleanup
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
    }


    //
    //  Ask the main window to cleanup. It can now destroy all the tabs,
    //  which will also unregister any server side ORB objects they have
    //  registered with the ORB.
    //
    if (m_wndMain.bIsValid())
        m_wndMain.Cleanup();

    // And remove our own objects
    try
    {
        if (m_porbsCoreAdmin)
        {
            facCIDOrb().DeregisterObject(m_porbsCoreAdmin);
            m_porbsCoreAdmin = 0;
        }

        if (m_porbsBrowser)
        {
            facCIDOrb().DeregisterObject(m_porbsBrowser);
            m_porbsBrowser = 0;
        }
    }

    catch(TError& errToCatch)
    {
        if (bLogInfo())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }
}



tCIDLib::TVoid TFacCQCTrayMon::CreateServerObjects()
{
    //
    //  Create a core admin object for this process, so that we can
    //  support basic remote administration. register it for auto-
    //  rebinding.
    //
    m_porbsCoreAdmin = new TCIDCoreAdminImpl;
    facCIDOrb().RegisterObject(m_porbsCoreAdmin, tCIDLib::EAdoptOpts::Adopt);

    TString strBind = kCQCKit::pszCQCTrayMonCoreAdmin;
    strBind.eReplaceToken(m_strBinding, kCIDLib::chLatin_b);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsCoreAdmin->ooidThis(), strBind, L"CQC Tray Monitor Core Admin Object"
    );

    //
    //  We have to support remote file browsing for the driver install
    //  wizard's app control binding prompt, so register a standard
    //  remote file browser object.
    //
    m_porbsBrowser = new TRemDiskBrowserImpl;
    facCIDOrb().RegisterObject(m_porbsBrowser, tCIDLib::EAdoptOpts::Adopt);
    strBind = kCQCKit::pszCQCTrayMonBrowser;
    strBind.eReplaceToken(m_strBinding, kCIDLib::chLatin_b);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsBrowser->ooidThis(), strBind, L"CQC Tray Monitor Remote Browser"
    );
}



//
//  This thread monitors our services and displays changes in their states
//
tCIDLib::EExitCodes
TFacCQCTrayMon::eMonThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go
    thrThis.Sync();

    // Loop until asked to shut down
    while (kCIDLib::True)
    {
        StoreSrvState(kCIDLib::True);

        // Sleep a while, break out if we were interrupted by a shutdown req
        if (!thrThis.bSleep(5000))
            break;
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  Parse our command line parms and store them away for later use
//
tCIDLib::TVoid
TFacCQCTrayMon::ParseParams(tCIDLib::TCard4&    c4StartDelay
                            , tCIDLib::TCard4&  bAppCtrl
                            , tCIDLib::TCard4&  biTunes)
{
    // Set defaults for the ones we don't get
    bAppCtrl = kCIDLib::False;
    biTunes = kCIDLib::False;
    c4StartDelay = 0;

    TString strCurParm;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        if (strCurParm.bCompareI(L"/AppCtrl"))
        {
            bAppCtrl = kCIDLib::True;
        }
         else if (strCurParm.bStartsWithI(L"/Delay="))
        {
            //
            //  Startup delay to avoid issues with the system coming up
            //  and not responding to our requests.
            //
            try
            {
                strCurParm.Cut(0, 7);
                c4StartDelay = strCurParm.c4Val(tCIDLib::ERadices::Dec);
            }

            catch(...)
            {
            }

            // Clip it back to 120 seconds
            if (c4StartDelay > 120)
                c4StartDelay = 120;
        }
         else if (strCurParm.bStartsWithI(L"/Binding="))
        {
            // The name server binding to use
            strCurParm.Cut(0, 9);
            m_strBinding = strCurParm;
            m_strBinding.StripWhitespace();
        }
         else if (strCurParm.bCompareI(L"/iTunes"))
        {
            biTunes = kCIDLib::True;
        }
         else if (strCurParm.bStartsWithI(L"/Port="))
        {
            // The port to use for the server side ORB
            strCurParm.Cut(0, 6);
            m_ippnListen = strCurParm.c4Val();
        }
         else if (strCurParm.bStartsWithI(L"/Max="))
        {
            // The max ORB clients to accept at once
            strCurParm.Cut(0, 5);
            m_c4MaxClients = strCurParm.c4Val();
        }
         else
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_UnknownParm
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::AppStatus
                , strCurParm
            );
        }
    }
}


