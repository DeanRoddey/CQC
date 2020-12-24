//
// FILE NAME: CQCAppCtrl_TrayWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/03/2012
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
//  This file implements the tray application window. This one is never
//  visible. It exists just to interact with the system tray. We have a
//  separate frame window that can be brought up when the user clicks on
//  the tray icon.
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
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTrayMonWnd,TTrayAppWnd)



// ---------------------------------------------------------------------------
//  CLASS: TTrayAppWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTrayAppWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TTrayMonWnd::TTrayMonWnd() :

    m_eAppState(tCQCTrayMon::ESrvStates::Count)
    , m_eClState(tCQCTrayMon::ESrvStates::Count)
    , m_menuPopup(L"Tray Monitor Popup")
    , m_strSrvName_AppShell(L"App Shell")
    , m_strSrvName_Client(L"Client")
{
}


TTrayMonWnd::~TTrayMonWnd()
{
}



//
//  A specialized notification display to show the state of the CQC services.
//  We have a thread that monitors whether it is running or not and it will
//  call this. We also store the status to be used for enabling/disabling
//  menu options.
//
//  We also set this status info as the tip text, whether doing the popup or
//  not.
//
tCIDLib::TVoid
TTrayMonWnd::ShowServiceStatus( const   tCQCTrayMon::ESrvStates eClState
                                , const tCQCTrayMon::ESrvStates eAppShState
                                , const tCIDLib::TBoolean       bNoPopup)
{
    // Remember the last state we saw
    m_eAppState = eAppShState;
    m_eClState = eClState;

    if (bIsValid())
    {
        TString strStatus;
        TString strState;

        FormatState(eClState, kCIDLib::True, strState);
        strStatus.Append(L"\n  ");
        strStatus.Append(strState);

        FormatState(eAppShState, kCIDLib::False, strState);
        strStatus.Append(L"\n  ");
        strStatus.Append(strState);

        //
        //  The tip text has to include the title text in the text, because it's
        //  just a single string. The balloon text doesn't need that because it
        //  takes the title separately and displays it separately.
        //
        TString strTip(TStrCat(m_strStateTitle, strStatus));
        SetTipText(strTip);

        if (!bNoPopup)
            ShowBalloonText(m_strStateTitle, strStatus);
    }
}


// ---------------------------------------------------------------------------
//  TTrayAppWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the tray app framework when it's time for us to show our menu
tCIDLib::TBoolean TTrayMonWnd::bShowTrayMenu(const TPoint& pntAt)
{
    // Based on the last service state we say, enable/disable items
    if (m_eAppState == tCQCTrayMon::ESrvStates::Started)
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStart, kCIDLib::False);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStop, kCIDLib::True);
    }
     else if (m_eAppState == tCQCTrayMon::ESrvStates::Stopped)
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStart, kCIDLib::True);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStop, kCIDLib::False);
    }
     else
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStart, kCIDLib::False);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_AppStop, kCIDLib::False);
    }

    if (m_eClState == tCQCTrayMon::ESrvStates::Started)
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStart, kCIDLib::False);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStop, kCIDLib::True);
    }
     else if (m_eClState == tCQCTrayMon::ESrvStates::Stopped)
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStart, kCIDLib::True);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStop, kCIDLib::False);
    }
     else
    {
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStart, kCIDLib::False);
        m_menuPopup.SetItemEnable(kCQCTrayMon::ridMenu_Srv_ClStop, kCIDLib::False);
    }

    // Check the currently set verbosity level
    TSubMenu menuVerbose(m_menuPopup, kCQCTrayMon::ridMenu_Verbosity);
    menuVerbose.SetItemCheck
    (
        tCIDLib::TCard4(kCQCTrayMon::ridMenu_Verbose_Off)
         + tCIDLib::TCard4(facCQCTrayMon.eVerboseLvl())
        , kCIDLib::True
        , kCIDLib::True
    );

    // Make us the foreground window
    facCIDCtrls().SetForeground(hwndThis(), kCIDLib::False);

    // We want it bottom and right aligned
    const tCIDLib::TCard4 c4Cmd = m_menuPopup.c4Process
    (
        *this, pntAt, tCIDLib::EVJustify::Bottom
    );

    // If no selection, just return now with false
    if (c4Cmd == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    // Do the requested operation
    switch(c4Cmd)
    {
        case kCQCTrayMon::ridMenu_Srv_AppStart :
            StartStopSrv(kCIDLib::True, kCIDLib::False);
            break;

        case kCQCTrayMon::ridMenu_Srv_AppStop :
            StartStopSrv(kCIDLib::False, kCIDLib::False);
            break;

        case kCQCTrayMon::ridMenu_Exit :
            // Exit the program
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
            break;

        case kCQCTrayMon::ridMenu_Srv_ClStart :
            StartStopSrv(kCIDLib::True, kCIDLib::True);
            break;

        case kCQCTrayMon::ridMenu_Srv_ClStop :
            StartStopSrv(kCIDLib::False, kCIDLib::True);
            break;

        case kCQCTrayMon::ridMenu_Verbose_Off :
            facCQCTrayMon.eVerboseLvl(tCQCKit::EVerboseLvls::Off);
            break;

        case kCQCTrayMon::ridMenu_Verbose_Low :
            facCQCTrayMon.eVerboseLvl(tCQCKit::EVerboseLvls::Low);
            break;

        case kCQCTrayMon::ridMenu_Verbose_Medium :
            facCQCTrayMon.eVerboseLvl(tCQCKit::EVerboseLvls::Medium);
            break;

        case kCQCTrayMon::ridMenu_Verbose_High :
            facCQCTrayMon.eVerboseLvl(tCQCKit::EVerboseLvls::High);
            break;

        default :
            break;
    };

    return kCIDLib::True;
}


tCIDLib::TVoid TTrayMonWnd::CleanupTrayApp()
{
    // Nothing for us to do right now
}


//
//  Handle any initialization that we need to do
//
tCIDLib::TVoid TTrayMonWnd::InitTrayApp()
{
    // Create our popup menu for later use
    m_menuPopup.Create(facCQCTrayMon, kCQCTrayMon::ridMenu_Main);

    // Preload the popup title text
    m_strStateTitle = facCQCTrayMon.strMsg(kTrayMonMsgs::midMsg_SrvState_Title);
}


//
//  Our icon was clicked on. If we are hidden, we show ourself. If we are
//  visible, we hide ourself. The facility provides a helper to do this.
//
tCIDLib::TVoid TTrayMonWnd::Invoked()
{
    facCQCTrayMon.ToggleFrameVis();
}


//
//  Since we run in the tray, and often run in client machines, we have
//  to deal cleanly with the system going into sleep mode and coming back
//  out. When we go into sleep mode, we clean our stuff up, and then crank
//  it all back up when we we come back awake.
//
tCIDLib::TVoid TTrayMonWnd::PowerStateChange(const tCIDLib::TBoolean bResume)
{
    // Pass this on to the facility object
    facCQCTrayMon.SleepWake(bResume);
}




// ---------------------------------------------------------------------------
//  TTrayAppWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Set up the popup display text for a given service
tCIDLib::TVoid
TTrayMonWnd::FormatState(const  tCQCTrayMon::ESrvStates eState
                        , const tCIDLib::TBoolean       bClient
                        ,       TString&                strToFill)
{
    // Figure out the state message text to load
    tCIDLib::TMsgId midState;
    switch(eState)
    {
        case tCQCTrayMon::ESrvStates::NotFound :
            midState = kTrayMonMsgs::midMsg_SrvState_NotInstalled;
            break;

        case tCQCTrayMon::ESrvStates::Starting :
            midState = kTrayMonMsgs::midMsg_SrvState_Starting;
            break;

        case tCQCTrayMon::ESrvStates::Started :
            midState = kTrayMonMsgs::midMsg_SrvState_Started;
            break;

        case tCQCTrayMon::ESrvStates::Stopping :
            midState = kTrayMonMsgs::midMsg_SrvState_Stopping;
            break;

        case tCQCTrayMon::ESrvStates::Stopped :
            midState = kTrayMonMsgs::midMsg_SrvState_Stopped;
            break;

        default :
            midState = kTrayMonMsgs::midMsg_SrvState_Unknown;
            break;
    };

    // Load it and put the service name into it
    strToFill.LoadFromMsg
    (
        midState
        , facCQCTrayMon
        , bClient ? m_strSrvName_Client : m_strSrvName_AppShell
    );
}


//
//  This is called from the menu handler above, to start or stop the service.
//  Note that we don't hang around and wait because we are in the GUI thread.
//  So all we do is start the process.
//
tCIDLib::TVoid
TTrayMonWnd::StartStopSrv(  const   tCIDLib::TBoolean bStart
                            , const tCIDLib::TBoolean bIsClient)
{
    tCIDKernel::TWSvcHandle hsvcShell = TKrnlWin32Service::hsvcOpen
    (
        bIsClient ? kCQCKit::pszClServiceName : kCQCKit::pszCQCShellName
    );

    // If we didn't open it, show an error and return
    if (!hsvcShell)
    {
        TErrBox msgbOpen
        (
            facCQCTrayMon.strMsg(kTrayMonMsgs::midMain_Title)
            , facCQCTrayMon.strMsg(kTrayMonErrs::errcSrv_OpenFailed)
        );
        msgbOpen.ShowIt(TWindow::wndDesktop());
        return;
    }

    tCIDLib::TBoolean bRes;
    if (bStart)
        bRes = TKrnlWin32Service::bStart(hsvcShell, 0, 0, 2000);
     else
        bRes = TKrnlWin32Service::bStop(hsvcShell, 2000);

    TKrnlWin32Service::bClose(hsvcShell);
}

