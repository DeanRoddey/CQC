//
// FILE NAME: CppDrvTest_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CppDrvTest.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCppDrvTestMainFrame,TFrameWnd)


// ---------------------------------------------------------------------------
//   CLASS: TCppDrvTestMainFrame
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCppDrvTestMainFrame: Constructors and Destructor
// ---------------------------------------------------------------------------
TCppDrvTestMainFrame::TCppDrvTestMainFrame() :

    m_c4ClDrvTabId(0)
    , m_pfactRemote(nullptr)
    , m_pwndClientTabs(nullptr)
    , m_pwndClDrvTab(nullptr)
    , m_pwndDrvInfoTab(nullptr)
    , m_pwndSrvDrvTab(nullptr)
    , m_pwndTrigMonTab(nullptr)
    , m_strTitleText(facCppDrvTest.strMsg(kCppDTMsgs::midGen_Title))
    , m_wstateNoSession(L"No Session")
    , m_wstateSessionNR(L"Session NR")
    , m_wstateSessionR(L"Session R")
{

}

TCppDrvTestMainFrame::~TCppDrvTestMainFrame()
{
}


// ---------------------------------------------------------------------------
//  TCppDrvTestMainFrame: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the client window contains the focus, let it have first crack. Else see if
//  it's ours.
//
tCIDLib::TBoolean
TCppDrvTestMainFrame::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    if (m_pwndClientTabs && m_pwndClientTabs->bContainsFocus())
    {
        if (m_pwndClientTabs->bProcessAccel(pMsgData))
            return kCIDLib::True;
    }
    return m_accelMain.bProcessAccel(*this, pMsgData);
}


// ---------------------------------------------------------------------------
//  TCppDrvTestMainFrame: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCppDrvTestMainFrame::CreateFrame(const TCQCFrameState& fstMain)
{
    // Set up our accelerator table
    {
        tCIDLib::TStrList colKeyStrs;
        tCIDLib::TCardList fcolCmds;

        colKeyStrs.objAdd(L"Ctrl-Shift-N");
        fcolCmds.c4AddElement(kCppDrvTest::ridMenu_Session_Exit);

        colKeyStrs.objAdd(L"Ctrl-S");
        fcolCmds.c4AddElement(kCppDrvTest::ridMenu_Driver_Save);

        m_accelMain.Create(colKeyStrs, fcolCmds);
    }

    TParent::CreateFrame
    (
        nullptr
        , fstMain.areaFrame()
        , m_strTitleText
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdSBFrame
        , kCIDLib::False
    );
}


// ---------------------------------------------------------------------------
//  TCppDrvTestMainFrame: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCppDrvTestMainFrame::bAllowShutdown()
{
    //
    //  Make sure that they really want to exit. If so, try to stop the driver if it
    //  is running. If that fails, don't exit.
    //
    TYesNoBox msgbAsk(m_strTitleText, facCppDrvTest.strMsg(kCppDTMsgs::midQ_Exit));
    if (msgbAsk.bShowIt(*this))
    {
        // If there is a client side driver, stop it
        StopClientIntf();
        return facCppDrvTest.bStopDriver();
    }

    return kCIDLib::False;
}


// Keep our pane container sized to fit our client area
tCIDLib::TVoid
TCppDrvTestMainFrame::AreaChanged(  const   TArea&                  areaPrev
                                    , const TArea&                  areaNew
                                    , const tCIDCtrls::EPosStates   ePosState
                                    , const tCIDLib::TBoolean       bOrgChanged
                                    , const tCIDLib::TBoolean       bSizeChanged
                                    , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged
    &&  (ePosState != tCIDCtrls::EPosStates::Minimized)
    &&  (m_pwndClientTabs != nullptr))
    {
        m_pwndClientTabs->SetSize(areaClient().szArea(), kCIDLib::True);
    }
}


tCIDLib::TBoolean TCppDrvTestMainFrame::bCreated()
{
    TParent::bCreated();

    // Set an icon for us
    facCppDrvTest.SetFrameIcon(*this, L"DriverIDE");

    // Load up our resource based main menu BEFORE we create the client window
    SetMenuBar(facCppDrvTest, kCppDrvTest::ridMenu_Main);

    //
    //  Create our main client window, which is a tabbed window. Put it in the client
    //  area.
    //
    const TArea areaOurClient = areaClient();
    m_pwndClientTabs = new TTabbedWnd();
    m_pwndClientTabs->CreateTabbedWnd
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaOurClient
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::ClipChildren
          )
        , tCIDCtrls::ETabWStyles::None
    );

    // Set a minimum track size. We don't limit the max.
    SetMinMaxSize(TSize(640, 480), TSize(0, 0));

    // Initially check the Off verbosity level
    menuCur().SetItemCheck(kCppDrvTest::ridMenu_Tools_Verbose_Off, kCIDLib::True);

    // Create the server side driver tab, which we always need
    m_pwndSrvDrvTab = new TSrvDrvTab();
    m_pwndClientTabs->c4CreateTab(m_pwndSrvDrvTab, L"Driver");

    //
    //  Create the driver info tab which we set up and keep around around. We
    //  load it upon start of a session. Indicate it cannot be closed.
    //
    m_pwndDrvInfoTab = new TCQCDrvInfoTab(kCIDLib::False);
    m_pwndClientTabs->c4CreateTab(m_pwndDrvInfoTab, L"Info");

    // Create the event trigger monitor tab. It also stays around
    m_pwndTrigMonTab = new TTrigMonTab();
    m_pwndClientTabs->c4CreateTab(m_pwndTrigMonTab, L"Trig Mon");

    // Set up our window states
    m_wstateNoSession.AddMenuEnable(kCppDrvTest::ridMenu_Session_New);
    m_wstateNoSession.AddMenuEnable(kCppDrvTest::ridMenu_Tools_RemotePort);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Session_Close);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Session_Start);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Session_Stop);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Tools_V2Validation);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Tools_QueryDrvText);
    m_wstateNoSession.AddMenuDisable(kCppDrvTest::ridMenu_Driver_Save);

    //
    //  Open but not running is the opposite of no session, but stop is not available,
    //  so we have to get the stop and save disables put back
    //
    m_wstateSessionNR = m_wstateNoSession;
    m_wstateSessionNR.RemoveMenuDisable(kCppDrvTest::ridMenu_Session_Stop);
    m_wstateSessionNR.RemoveMenuDisable(kCppDrvTest::ridMenu_Driver_Save);
    m_wstateSessionNR.Invert();
    m_wstateSessionNR.AddMenuDisable(kCppDrvTest::ridMenu_Session_Stop);
    m_wstateSessionNR.AddMenuDisable(kCppDrvTest::ridMenu_Driver_Save);


    // If running, then a few things are available but not are not
    m_wstateSessionR.AddMenuEnable(kCppDrvTest::ridMenu_Session_Stop);
    m_wstateSessionR.AddMenuEnable(kCppDrvTest::ridMenu_Tools_V2Validation);
    m_wstateSessionR.AddMenuEnable(kCppDrvTest::ridMenu_Tools_QueryDrvText);
    m_wstateSessionR.AddMenuEnable(kCppDrvTest::ridMenu_Driver_Save);
    m_wstateSessionR.AddMenuDisable(kCppDrvTest::ridMenu_Session_New);
    m_wstateSessionR.AddMenuDisable(kCppDrvTest::ridMenu_Session_Close);
    m_wstateSessionR.AddMenuDisable(kCppDrvTest::ridMenu_Session_Start);
    m_wstateSessionR.AddMenuDisable(kCppDrvTest::ridMenu_Tools_RemotePort);

    // Set us up initially in no-session state
    m_wstateNoSession.ApplyState(*this, pmenuCur());

    //
    //  We have to handle the remote chat one manually since it depends on remote port
    //  support being enabled. Initially disable it
    //
    menuCur().SetItemEnable(kCppDrvTest::ridMenu_Tools_RemoteChat, kCIDLib::False);

    return kCIDLib::True;
}


tCIDLib::TVoid TCppDrvTestMainFrame::Destroyed()
{
    // Save any window states for use next time
    SaveState();

    //
    //  We have to clean up the chat window since it's not a pointer. Else the base
    //  window class would try to destroy and delete it.
    //
    if (m_wndChat.bIsValid())
        m_wndChat.Destroy();

    // And call our parent last
    TParent::Destroyed();
}


tCIDLib::TVoid
TCppDrvTestMainFrame::MenuCommand(  const   tCIDLib::TResId     ridItem
                                    , const tCIDLib::TBoolean   bChecked
                                    , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == kCppDrvTest::ridMenu_Session_Close)
    {
        facCppDrvTest.CloseSession();

        // If it worked, then apply the no session state
        if (!facCppDrvTest.bHaveSession())
            m_wstateNoSession.ApplyState(*this, pmenuCur());
    }
     else if (ridItem == kCppDrvTest::ridMenu_Session_New)
    {
        facCppDrvTest.DoNewSession();

        //
        //  Though it's possible nothing happened, update the tabs just in case.
        //  Even if the session open may have not gone through, the closing of the
        //  current one may have happened.
        //
        if (facCppDrvTest.bHaveSession())
        {
            // Tell the trigger monitor tab the new moniker
            m_pwndTrigMonTab->SetMoniker(facCppDrvTest.strMoniker());

            // Tell the driver info tab to display the new session info
            m_pwndDrvInfoTab->UpdateInfo
            (
                facCppDrvTest.strMoniker(), facCppDrvTest.cqcdcSession()
            );

            // And tell the server side driver that we have a new session
            m_pwndSrvDrvTab->SessionStartStop(kCIDLib::True);

            // We are in not-running session state
            m_wstateSessionNR.ApplyState(*this, pmenuCur());
        }
         else
        {
            m_pwndDrvInfoTab->ClearInfo();
            m_pwndSrvDrvTab->SessionStartStop(kCIDLib::False);

            // We are in no-session state
            m_wstateNoSession.ApplyState(*this, pmenuCur());
        }
    }
     else if (ridItem == kCppDrvTest::ridMenu_Session_Start)
    {
        // Indicate not test mode
        if (facCppDrvTest.bStartDriver())
        {
            m_pwndSrvDrvTab->DriverStartStop(kCIDLib::True);

            //
            //  If there is a client side driver for this guy, let's get him
            //  started up.
            //
            StartClientIntf();

            // We are in running session state
            m_wstateSessionR.ApplyState(*this, pmenuCur());
        }
    }
     else if (ridItem == kCppDrvTest::ridMenu_Session_Stop)
    {
        if (facCppDrvTest.bStopDriver())
        {
            m_pwndSrvDrvTab->DriverStartStop(kCIDLib::False);

            // We are in not-running session state
            m_wstateSessionNR.ApplyState(*this, pmenuCur());

            // If there's a client side driver, get it shut down
            StopClientIntf();
        }
    }
     else if (ridItem == kCppDrvTest::ridMenu_Session_Exit)
    {
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if (ridItem == kCppDrvTest::ridMenu_Tools_RemotePort)
    {
        SetupRemotePort();
    }
     else if (ridItem == kCppDrvTest::ridMenu_Tools_RemoteChat)
    {
        //
        //  We shouldn't be callable unless the remote interface is enabled, but
        //  check just in case. If the window isn't already created, then create it
        //
        if (m_pfactRemote && !m_wndChat.bIsValid())
        {
            if (!m_wndChat.bIsValid())
                m_wndChat.Create(*this, kCppDrvTest::strCfgKey_ChatState);

            // Tell it to enable, giving it the remote end point
            m_wndChat.EnableChat(m_pfactRemote->ipepServer());
        }
    }
     else if ((ridItem >= kCppDrvTest::ridMenu_Tools_Verbose_Off)
          &&  (ridItem <= kCppDrvTest::ridMenu_Tools_Verbose_High))
    {
        //
        //  Set the verbosity level of the driver. The items in the submenu are in
        //  their natural enum order so we can just sub the first value and cast to
        //  the enum.
        //
        const tCQCKit::EVerboseLvls eLvl = tCQCKit::EVerboseLvls
        (
            ridItem - kCppDrvTest::ridMenu_Tools_Verbose_Off
        );

        // Store this as the new live and default verbosity
        facCppDrvTest.SetNewVerbosity(eLvl);

        // And let's update the menu to check this one
        TMenuBar& menuBar = menuCur();
        TSubMenu menuVerbosity(menuBar, kCppDrvTest::ridMenu_Tools_Verbosity);
        menuVerbosity.SetItemCheck(ridItem, kCIDLib::True, kCIDLib::True);
    }
     else if (ridItem == kCppDrvTest::ridMenu_Tools_QueryDrvText)
    {
        TQueryDrvTextDlg dlgQueryText;
        dlgQueryText.RunDlg(*this);
    }
     else if (ridItem == kCppDrvTest::ridMenu_Tools_V2Validation)
    {
        facCppDrvTest.DoV2Validation();
    }
     else if (ridItem == kCppDrvTest::ridMenu_Driver_Save)
    {
        if (m_pwndClDrvTab->pwndDriver()->bChanges())
        {
            TString strErr;
            if (!m_pwndClDrvTab->pwndDriver()->bSaveChanges(strErr))
            {
                TErrBox msgbErr(m_strTitleText, strErr);
                msgbErr.ShowIt(*this);
            }
             else
            {
                TOkBox msgbDone(m_strTitleText, L"Changes were saved successfully");
                msgbDone.ShowIt(*this);
            }
        }
    }
}



// ---------------------------------------------------------------------------
//  TCppDrvTestMainFrame: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Called when the user selects the remote port option
tCIDLib::TVoid TCppDrvTestMainFrame::SetupRemotePort()
{
    // Do the remote port configuration dialog
    tCIDLib::TBoolean bEnable = (m_pfactRemote != nullptr);

    //
    //  See if we have any last end point stored. If already enabled, use what's already
    //  there. If not, see if we have a stored one. Else default it.
    //
    tCIDLib::TCard4 c4Ver;
    TIPEndPoint     ipepInit;
    if (bEnable)
    {
        ipepInit = m_pfactRemote->ipepServer();
    }
     else
    {
        try
        {
            c4Ver = 0;
            const tCIDLib::ELoadRes eRes = facCQCGKit().eReadStoreObj
            (
                kCppDrvTest::strCfgKey_LastRemEP, c4Ver, ipepInit, kCIDLib::False
            );

            if (eRes != tCIDLib::ELoadRes::NewData)
                ipepInit = facCIDSock().ipepDefLocalAddr(TRemSerialSrvClientProxy::ippnDefault);
        }

        catch(...)
        {
            facCQCGKit().bDeleteStoreObj(kCppDrvTest::strCfgKey_LastRemEP);
        }
    }

    if (facCQCIGKit().bCfgRemotePort(*this, bEnable, ipepInit))
    {
        // Deregister any remote port factory we have registered
        facCIDComm().DeregisterFactory(TRemSerialSrvClientProxy::strSerialFactoryId);
        m_pfactRemote = nullptr;

        // And now if they enabled, register a new one
        if (bEnable)
        {
            m_pfactRemote = new TCQCRemSrvPortFactory(ipepInit);
            facCIDComm().RegisterFactory(m_pfactRemote);

            // Store this as the new last end point
            facCQCGKit().bAddUpdateStoreObj
            (
                kCppDrvTest::strCfgKey_LastRemEP, c4Ver, ipepInit, 16
            );

            // And enable the chat setup menu
            menuCur().SetItemEnable(kCppDrvTest::ridMenu_Tools_RemoteChat, kCIDLib::True);

            // If the chat window is up, let it know about this
            if (m_wndChat.bIsValid())
                m_wndChat.EnableChat(ipepInit);
        }
         else
        {
            // Disable the chat menu item
            menuCur().SetItemEnable(kCppDrvTest::ridMenu_Tools_RemoteChat, kCIDLib::False);

            // If the chat window is up, let it know about this
            if (m_wndChat.bIsValid())
                m_wndChat.DisableChat();
        }
    }
}


//
//  When the server side driver is started up successfully, this is called to see if
//  there is a client side interface for this one and to load it up if so.
//
//  We create a new tab for it. The client side interface stuff is pretty generic
//  to deal with. We just have to create the window, and set it on the tab which will
//  keep it sized appropriately.
//
tCIDLib::TVoid TCppDrvTestMainFrame::StartClientIntf()
{
    const TCQCDriverCfg& cqcdcDrv = facCppDrvTest.cqcdcOrg();
    const TString& strLib = cqcdcDrv.strClientLibName();
    if (strLib.bIsEmpty())
        return;

    //
    //  We have to dynamically load this guy. CQCIGKit has a helper that will do
    //  this for us. We have to have the tab window ready first, since we have to
    //  pass in the parent window that he will create his main window as a child of.
    //
    //  We pass in true for the 'test mode' flag so that he will not try to download
    //  the facilities, he'll load them from the build results directory, which is what
    //  we want so that we pick up change immediately.
    //
    m_pwndClDrvTab = new TClDrvTab();
    m_c4ClDrvTabId = m_pwndClientTabs->c4CreateTab(m_pwndClDrvTab, L"Client");

    // Tell it to load the driver and start it
    if (!m_pwndClDrvTab->bLoadDriver())
    {
        TErrBox msgbErr(L"The driver could not be loaded");
        msgbErr.ShowIt(*this);
    }
}


//
//  This is called when the driver is stopped. If we have a client side driver interface
//  tab, we shut it down.
//
tCIDLib::TVoid TCppDrvTestMainFrame::StopClientIntf()
{
    if (m_pwndClDrvTab)
    {
        if (m_pwndClDrvTab->bStopDriver())
        {
            m_pwndClientTabs->CloseTabById(m_c4ClDrvTabId);

            // Clean some stuff up that is per-run
            m_c4ClDrvTabId = 0;
            m_pwndClDrvTab = nullptr;
        }
    }
}



//
//  Saves our window position info and any other state we want to store away for later
//  use on the next start up.
//
tCIDLib::TVoid TCppDrvTestMainFrame::SaveState()
{
    try
    {
        //
        //  Store our various window states. This helper will check to see if the window
        //  is valid before storing anything.
        //
        facCQCGKit().StoreFrameState(*this, kCppDrvTest::strCfgKey_Frame);
        facCQCGKit().StoreFrameState(m_wndChat, kCppDrvTest::strCfgKey_ChatState);
    }

    catch(TError& errToCatch)
    {
        // Oh well couldn't save it, but not much we can do about it but log
        if (facCppDrvTest.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}
