//
// FILE NAME: CQCIntfView_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2002
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
// ---------------------------------------------------------------------------
#include    "CQCIntfView.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCIntfView,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfView
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCIntfView: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCIntfView::TFacCQCIntfView() :

    TGUIFacility
    (
        L"CQCIntfView"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_bKioskMode(kCIDLib::False)
    , m_bSignageMode(kCIDLib::False)
    , m_colSpeech(tCIDLib::EMTStates::Safe)
    , m_ippnListen(0)
    , m_plgrLogSrv(nullptr)
    , m_porbsCtrlImpl(nullptr)
    , m_thrSpeech
      (
        TString(L"CQCIVSpeechThr")
        , TMemberFunc<TFacCQCIntfView>(this, &TFacCQCIntfView::eSpeechThread)
      )
{
}

TFacCQCIntfView::~TFacCQCIntfView()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfView: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the full screen setting
tCIDLib::TBoolean TFacCQCIntfView::bFullScreen() const
{
    return m_wndMain.bInFSMode();
}

// Get the kiosk mode flag
tCIDLib::TBoolean TFacCQCIntfView::bKioskMode() const
{
    return m_bKioskMode;
}


//
//  Called by our IV control ORB interface, to queue up text to be spoken
//  in the background. If the queue is full, we just return false and ignore
//  the new text.
//
tCIDLib::TBoolean TFacCQCIntfView::bQueueTTS(const TString& strToSpeak)
{
    //
    //  We have to lock explicitly since we have to do more than one atomic
    //  operation here.
    //
    TLocker lockrSpeech(&m_colSpeech);
    if (m_colSpeech.c4ElemCount() < 64)
    {
        m_colSpeech.objAdd(strToSpeak);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TFacCQCIntfView::bSignageMode() const
{
    return m_bSignageMode;
}


// Provide access to our user context to those bits that need it.
const TCQCUserCtx& TFacCQCIntfView::cuctxToUse() const
{
    return m_cuctxToUse;
}


//
//  The program entry point
//
tCIDLib::EExitCodes
TFacCQCIntfView::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
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
            strMsg(kIViewMsgs::midWnd_AppName2)
            , strMsg(kIViewMsgs::midStatus_BadEnv, strFailReason)
        );
        msgbTest.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    tCIDLib::EExitCodes eExit = tCIDLib::EExitCodes::RuntimeError;
    try
    {
        const TString strAppText(strMsg(kIViewMsgs::midWnd_AppName2));

        //
        //  See if we are in signage mode. If so we need to skip the single
        //  instance check below.
        //
        CheckSignage();

        //
        //  If not in signage mode, check to see if there is already an instance
        //  running. If so, activate it and just exit.
        //
        if (!m_bSignageMode)
        {
            TResourceName rsnInstance(L"CQSL", L"CQCIntfView", L"SingleInstanceInfo");
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
        m_plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(m_plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        // If verbose logging, then log that we are starting up
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midApp_Startup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"CQCIntfView")
                , strVersion()
            );
        }

        //
        //  Parse the parameters. Some we just need here or we will just
        //  be passing on to the users of those values. For other parms this
        //  guy will set members of ours directly.
        //
        tCIDLib::TBoolean   bFSMode = kCIDLib::False;
        tCIDLib::TBoolean   bNoState = kCIDLib::False;
        tCIDLib::TCard4     c4StartDelay;
        TString             strDefTmpl;
        if (!bParseParams(strDefTmpl, c4StartDelay, bNoState, bFSMode))
            return tCIDLib::EExitCodes::BadParameters;

        //
        //  If we are in signage mode and no binding was provided or no port,
        //  we have to fail.
        //
        if (m_bSignageMode && (m_strCtrlBinding.bIsEmpty() || !m_ippnListen))
        {
            TErrBox msgbErr
            (
                strAppText
                , L"You must provide the /Binding= and /Port= parameters when "
                  L"invoking signage mode!"
            );
            msgbErr.ShowIt(TWindow::wndDesktop());
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Tell the windowing interface engine, which is what we use to
        //  create window based template displays, that he should be in
        //  no input mode if we are being invoked for signage. This will
        //  prevent any interface windows from accepting touch input.
        //
        //  We also force kiosk mode mode on, even if not explicitly set, and
        //  we'll later force the window full screen.
        //
        if (m_bSignageMode)
        {
            facCQCIntfWEng().bNoInputMode(kCIDLib::True);
            m_bKioskMode = kCIDLib::True;
        }

        //
        //  Start up our polling engine. We hand this out to any underlying
        //  code that needs one, so that we have a single shared one for
        //  maximum efficiency.
        //
        m_polleToUse.StartEngine(m_cuctxToUse.sectUser());

        //
        //  We have to install some class loaders on the macro engine
        //  facility, so that the standard CQC runtime and media access
        //  CML classes.
        //
        facCQCMEng().InitCMLRuntime(sectUser());
        facCQCMedia().InitCMLRuntime();

        //
        //  If they indicated a startup delay, then let's wait for that
        //  number of seconds. The delay is in seconds.
        //
        if (c4StartDelay)
            facCIDCtrls().MsgYield(c4StartDelay * 1000);

        // Wait for the back end to come up, in case it hasn't yet
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
            return tCIDLib::EExitCodes::PermissionLevel;

        //
        //  Initialize the local data store if not in signage mode. Else
        //  put the CQCGKit facility into no local store mode, so that it
        //  will dummy out any attempts to use the local store.
        //
        if (m_bSignageMode)
        {
            facCQCGKit().SetNoLocalStoreMode();
        }
         else
        {
            facCQCGKit().InitObjectStore
            (
                kCQCIntfView::pszStoreAppKey, m_cuctxToUse.strUserName(), bNoState
            );
        }

        // If it's a limited user, then ignore any command line template
        if (m_uaccLogin.eRole() == tCQCKit::EUserRoles::LimitedUser)
            strDefTmpl.Clear();

        //
        //  If no default template and its a limited user and we are not in
        //  signage mode, then we can't load.
        //
        if (m_uaccLogin.strDefInterfaceName().bIsEmpty()
        &&  (m_uaccLogin.eRole() == tCQCKit::EUserRoles::LimitedUser)
        &&  !m_bSignageMode)
        {
            TErrBox msgbWarn
            (
                strMsg(kIViewMsgs::midWnd_AppName2)
                , strMsg(kIViewMsgs::midStatus_NoDefIntfForLimUser)
            );
            msgbWarn.ShowIt(TWindow::wndDesktop());
            return tCIDLib::EExitCodes::PermissionLevel;
        }

        // Start up the media database cacher
        facCQCMedia().StartMDBCacher();

        // Set up a default initial window state
        TCQCIntfViewState fstInit
        (
            facCIDCtrls().areaDefWnd(), TString::strEmpty()
        );

        //
        //  Load up any previous window state stuff if not in signage mode.
        //  If in signage mode, fake some info into the state data to make
        //  it be fully contained within the target display, so that it'll
        //  go full screen on that monitor.
        //
        if (m_bSignageMode)
        {
            //
            //  Get the area of the monitor we were told to use. If we cannot,
            //  then fail and exit.
            //
            TArea areaInit;
            if (!facCIDCtrls().bQueryNamedMonArea(m_strDisplayName, areaInit, kCIDLib::False))
            {
                TErrBox msgbErr
                (
                    strAppText
                    , L"Could not obtain monitor area info for the target signage display"
                );
                msgbErr.ShowIt(TWindow::wndDesktop());
                return tCIDLib::EExitCodes::Normal;
            }

            // We got it so fake up the init state info
            fstInit.SetArea(areaInit, tCIDCtrls::EPosStates::Maximized);

            //
            //  Set the template to be a special one in the system area that
            //  is just our logo on a white ackground.
            //
            fstInit.strLastTmpl(L"\\System\\CQC\\Logo");
        }
         else
        {
            tCIDLib::TCard4 c4Ver = 0;
            facCQCGKit().eReadStoreObj
            (
                kCQCIntfView::pszCfgKey_MainFrame, c4Ver, fstInit, kCIDLib::False
            );
        }

        //
        //  Load any stored app attach info. AFTER init of the data store! Only
        //  do this if not in signage mode.
        //
        if (!m_bSignageMode)
            facCQCGKit().LoadAppAttachInfo();

        //
        //  If we got no override on the command line, then take the user's
        //  default acount template.  Note that above we ignored any
        //  command line template if our user is a limited user, and we
        //  exited if he's limited and had no default.
        //
        //  So at this point, we will not enter here if it's a limited
        //  user.
        //
        if (strDefTmpl.bIsEmpty())
        {
            // No command line, try the default
            strDefTmpl = m_uaccLogin.strDefInterfaceName();

            // If still nothing, see if the window has a previously loaded
            if (strDefTmpl.bIsEmpty())
                strDefTmpl = fstInit.strLastTmpl();
        }

        // Set up the IV engine. We have to provide him with a polling engine
        facCQCIntfEng().StartEngine(&m_polleToUse);

        // Try to create the main frame
        m_wndMain.Create(strAppText, fstInit);

        //
        //  If full screen mode, then force it full screen to start, and hide the pointer
        //  if kiosk mode.
        //
        if (bFSMode)
            m_wndMain.StartFSMode();

        //
        //  Enable the event processing system for receiving events, so that the interface
        //  windows will see incoming events and process them. We also need to support
        //  sending since we allow actions to send event triggers, and one might be run
        //  by this process.
        //
        facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Both, m_cuctxToUse.sectUser());

        //
        //  If we got an initial template, try to open it. If it fails, and it's
        //  a previous one that we are reloading, then remove that previous
        //  template reference so it won't happen again (if not in signage
        //  mode.)
        //
        if (!strDefTmpl.bIsEmpty())
        {
            if (!m_wndMain.bLoadTemplate(strDefTmpl, m_bKioskMode))
            {
                if ((strDefTmpl == fstInit.strLastTmpl()) && !m_bSignageMode)
                {
                    fstInit.ClearLastTmpl();

                    tCIDLib::TCard4 c4Version = 0;
                    facCQCGKit().bAddUpdateStoreObj
                    (
                        kCQCIntfView::pszCfgKey_MainFrame, c4Version, fstInit, 64
                    );
                }
            }
        }

        // And show our main window
        // facCIDCtrls().ShowGUI(m_wndMain);

        //
        //  If they set a remote control port, then register our control
        //  protocol server object. So we need to crank up the server side
        //  of the ORB, then create one of our protocol objects, register
        //  it with our rebinder thread, and then start up the rebinder.
        //
        //  We also spin up the TTS thread since remote TTS commands can
        //  be received through this interface.
        //
        //  The binding by default just uses the local host name, since the
        //  IV control driver assumes it runs on the same machine as the
        //  IV it is controlling and just gets the local host name to create
        //  the binding. But, if the are in Signage mode, they had to provide
        //  a binding name and we use that instead.
        //
        if (m_ippnListen)
        {
            m_thrSpeech.Start();

            TString strBindPath(TIntfCtrlServerBase::strBinding);
            if (m_bSignageMode)
                strBindPath.eReplaceToken(m_strCtrlBinding, kCIDLib::chLatin_b);
            else
                strBindPath.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);

            facCIDOrb().InitServer(m_ippnListen, 16);
            m_porbsCtrlImpl = new TIntfCtrlServer;
            facCIDOrb().RegisterObject(m_porbsCtrlImpl, tCIDLib::EAdoptOpts::Adopt);
            facCIDOrbUC().RegRebindObj
            (
                m_porbsCtrlImpl->ooidThis()
                , strBindPath
                , L"CQC Interface Controller"
                , TSysInfo::strIPHostName()
                , TString::strEmpty()
                , TString::strEmpty()
                , TString::strEmpty()
            );
            facCIDOrbUC().StartRebinder();
        }
    }

    catch(const TError& errToCatch)
    {
        // If its not already logged, then log it if we got the logger installed
        if (m_plgrLogSrv)
            TModule::LogEventObj(errToCatch);

        //
        //  Display the error to the user. If we have a main frame yet, use
        //  it, else use a null frame.
        //
        if (m_wndMain.bIsValid())
        {
            TExceptDlg dlgbFail
            (
                m_wndMain
                , strMsg(kIViewMsgs::midWnd_AppName2)
                , errToCatch.strErrText()
                , errToCatch
            );
        }
         else
        {
            TExceptDlg dlgbFail
            (
                TWindow::wndDesktop()
                , strMsg(kIViewMsgs::midWnd_AppName2)
                , errToCatch.strErrText()
                , errToCatch
            );
        }
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        // All we can do is pop up a message box
        TErrBox msgbTest
        (
            strMsg(kIViewMsgs::midWnd_AppName2)
            , strMsg(kIViewMsgs::midStatus_Exception)
        );
        msgbTest.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Call the window processing loop. We won't come back until the program exits.
    //  Note that we didn't show the window yet. The window will do a one shot timer
    //  to show itself after a short delay (and some message pumping that allows.)
    //
    facCIDCtrls().uMainMsgLoop(m_wndMain);

    // If verbose logging, then log that we are terminating
    if (bLogInfo())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_Shutdown
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"CQCIntfView")
        );
    }

    // Do the cleanup
    return eCleanUp();
}


//
//  A convenince wrapper, which we just pass on to our user context object.
//
tCQCKit::EUserRoles TFacCQCIntfView::eUserRole() const
{
    return m_cuctxToUse.eUserRole();
}


//
//  The main menu provides a menu item to flush the TTS queue, in case
//  they accidentally put a lot of text into it. We just do a remove all
//  on the queue collection. This is a thread safe collection, so a single
//  op requires no explicit lock.
//
tCIDLib::TVoid TFacCQCIntfView::FlushTTSQ()
{
    m_colSpeech.RemoveAll();
}


// Get our listening port
tCIDLib::TIPPortNum TFacCQCIntfView::ippnListen() const
{
    return m_ippnListen;
}


// A convenience wrapper that we just pass on to the user context
const TCQCSecToken& TFacCQCIntfView::sectUser() const
{
    return m_cuctxToUse.sectUser();
}


// A conveniene wrapper that we just pass on to our user context
const TString& TFacCQCIntfView::strUserName() const
{
    return m_cuctxToUse.strUserName();
}



//
//  The main frame calls us here to store status on the external control
//  interface object, which we own. We provide some and pass on the stuff
//  that he passes in.
//
tCIDLib::TVoid
TFacCQCIntfView::StoreStatus(const  TString&        strBaseTmpl
                            , const TString&        strTopTmpl
                            , const tCIDLib::TCard4 c4LayerCnt)
{
    if (m_porbsCtrlImpl)
    {
        m_porbsCtrlImpl->StoreStatus
        (
            strBaseTmpl
            , strTopTmpl
            , c4LayerCnt
            , m_wndMain.bInFSMode() || m_wndMain.bIsMaximized()
        );
    }
}


// Provide access to the main window
const TMainFrameWnd& TFacCQCIntfView::wndMain() const
{
    return m_wndMain;
}

TMainFrameWnd& TFacCQCIntfView::wndMain()
{
    return m_wndMain;
}


// ---------------------------------------------------------------------------
//  TFacCQCIntfView: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Just a convenience to wrap around the login process and set up our user
//  contents info that we pass into any actions or other calls that require
//  user context info.
//
tCIDLib::TBoolean TFacCQCIntfView::bDoLogon()
{
    TCQCSecToken sectTmp;
    if (facCQCGKit().bLogon(TWindow::wndDesktop()
                            , sectTmp
                            , m_uaccLogin
                            , tCQCKit::EUserRoles::LimitedUser
                            , strMsg(kIViewMsgs::midWnd_AppName2)
                            , kCIDLib::False
                            , L"IntfViewer"))
    {
        m_cuctxToUse.Set(m_uaccLogin, sectTmp);
        m_cuctxToUse.LoadEnvRTVsFromHost();

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Parses the command line parms. Most of them are only needed at startup,
//  or they are just passed on to other objects that hold them so they are
//  out parameters. Some are long term keepers for us, so we set members for
//  those.
//
tCIDLib::TBoolean
TFacCQCIntfView::bParseParams(  TString&                strInitTmpl
                                , tCIDLib::TCard4&      c4StartDelay
                                , tCIDLib::TBoolean&    bNoState
                                , tCIDLib::TBoolean&    bFSMode)
{
    c4StartDelay    = 0;
    bNoState        = kCIDLib::False;
    bFSMode         = kCIDLib::False;

    m_bKioskMode    = kCIDLib::False;
    m_ippnListen    = 0;
    m_strCtrlBinding.Clear();

    //
    //  Most of our parms are handled by the common parsing in CQCKit, which
    //  we invoked above via the LoadEnvInfo() call, but we have some to deal
    //  with.
    //
    TString strCurParm;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        if (strCurParm.bStartsWithI(L"/Binding="))
        {
            m_strCtrlBinding.CopyInSubStr(strCurParm, 9);
        }
         else if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);

            if (strCurParm.bCompareI(L"Default"))
                m_ippnListen = kCQCKit::ippnIntfCtrlDefPort;
            else
                m_ippnListen = strCurParm.c4Val();
        }
         else if (strCurParm.bStartsWithI(L"/Delay="))
        {
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
         else if (strCurParm.bCompareI(L"/FullScreen"))
        {
            bFSMode = kCIDLib::True;
        }
         else if (strCurParm.bCompareI(L"/NoState"))
        {
            bNoState = kCIDLib::True;
        }
         else if (strCurParm.bCompareI(L"/Kiosk"))
        {
            m_bKioskMode = kCIDLib::True;
        }
         else if (strCurParm.bStartsWithI(L"/Signage="))
        {
            m_strDisplayName.CopyInSubStr(strCurParm, 9);
        }
         else if (strCurParm.bStartsWithI(L"/InitTmpl="))
        {
            strInitTmpl.CopyInSubStr(strCurParm, 10);
        }
         else
        {
            if (bLogWarnings())
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_BadParm
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::BadParms
                    , strCurParm
                );
            }
        }
    }

    // If kiosk mode, force full screen on
    if (m_bKioskMode)
        bFSMode = kCIDLib::True;

    return kCIDLib::True;
}


//
//  We have a bit of a bootstrapping issue in the case of digital signage
//  mode. Generally we want to do a single instance check and exit if another
//  version is already running and we want to get some stuff done before
//  parsing the parameters so that bad parms can be logged. But for the digital
//  signage scenario we have to check first for that, and not do a signle instance
//  check. So we have this special check to do first that just scans for that
//  one value and won't log anything.
//
tCIDLib::TVoid TFacCQCIntfView::CheckSignage()
{
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        if (cursParms->bStartsWithI(L"/Signage="))
        {
            m_bSignageMode = kCIDLib::True;
            break;
        }
    }
}


// Called on program exit to do clean
tCIDLib::EExitCodes TFacCQCIntfView::eCleanUp()
{
    try
    {
        // Disable event processing now
        facCQCKit().StopEventProcessing();

        // If we started our remote control interface, clean that up
        if (m_ippnListen)
        {
            //
            //  Stop the rebinder, remove our binding, and deregister
            //  our server object so that no one else can try to
            //  connect as we shut down. It will also force out any
            //  threads in this object.
            //
            facCIDOrbUC().StopRebinder();

            try
            {
                tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
                TString strBinding(TIntfCtrlServerBase::strBinding);
                strBinding.eReplaceToken(m_strCtrlBinding, kCIDLib::chLatin_b);
                orbcNS->RemoveBinding(strBinding, kCIDLib::False);
            }

            catch(...)
            {
            }

            if (m_porbsCtrlImpl)
                facCIDOrb().DeregisterObject(m_porbsCtrlImpl);

            //
            //  Stop the speech thread. It could take a while if it is
            //  currently speaking text. If we time out, we just keep
            //  going. It'll get bashed down the hard way if so.
            //
            try
            {
                m_thrSpeech.ReqShutdownSync(10000);
                m_thrSpeech.eWaitForDeath(3000);
            }

            catch(...)
            {
            }
        }

        // Stop the media DB cacher thread
        facCQCMedia().StopMDBCacher();

        // Stop the window's timers
        m_wndMain.StopAllTimers();

        // Try to stop the logic server graph poller if it got started
        facCQLogicSh().StopPoller();

        // Close our current template by destroying the client window
        TCQCIntfViewWnd* pwndIntf = m_wndMain.pwndClient();
        if (pwndIntf)
            pwndIntf->Destroy();

        // Store any app attach info if not in signage mode
        if (!m_bSignageMode)
            facCQCGKit().StoreAppAttachInfo();

        // And now free them all up
        facCQCKit().FreeApps();

        //
        //  Stop the interface engine. This will make it clean up anything it
        //  is using. We already closed any current template above, so we know
        //  no one isusing it now.
        //
        facCQCIntfEng().StopEngine();

        //
        //  Now destroy the main frame window. It will try to store config
        //  data, so we have to do this before we clean up the object store.
        //
        m_wndMain.Destroy();

        // Stop the polling engine now that no one can be using it
        m_polleToUse.StopEngine();

        // Terminate the local object store if not in signage mode
        if (!m_bSignageMode)
            facCQCGKit().TermObjectStore();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        return tCIDLib::EExitCodes::RuntimeError;
    }

    try
    {
        //
        //  Force the log server logger to local logging and clean it up. We told
        //  TModule to adopt it so he will delete it.
        //
        if (m_plgrLogSrv)
        {
            m_plgrLogSrv->bForceLocal(kCIDLib::True);
            m_plgrLogSrv->Cleanup();
        }

        // Clean up the ORB support before we exit
        facCIDOrb().Terminate();
    }

    catch(...)
    {
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  The speech thread is started up on this method. We just wait for
//  text to speak.
//
tCIDLib::EExitCodes
TFacCQCIntfView::eSpeechThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    TString strText;
    TSpeech spchToUse;
    while(kCIDLib::True)
    {
        // Check for shutdown requests
        if (thrThis.bCheckShutdownRequest())
            break;

        // Block a bit for text to process.
        try
        {
            if (m_colSpeech.bGetNextMv(strText, 250))
            {
                // We got some, so speak it
                spchToUse.Speak(strText);
            }
        }

        catch(const TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIViewMsgs::midStatus_ExceptInSpeechThr
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
                LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIViewMsgs::midStatus_ExceptInSpeechThr
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


