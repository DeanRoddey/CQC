//
// FILE NAME: CQCTrayMon_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCTrayMonFrameWnd,TFrameWnd)



// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayMonFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTrayMonFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCTrayMonFrameWnd::TCQCTrayMonFrameWnd() :

    m_bAppCtrl(kCIDLib::False)
    , m_biTunes(kCIDLib::False)
    , m_colTabs(tCIDLib::EAdoptOpts::NoAdopt, 16)
    , m_pwndTabs(0)
{
}

TCQCTrayMonFrameWnd::~TCQCTrayMonFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCTrayMonFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the program is shutting down. We ask each tab to clean
//  up any server side interfaces it created, and then we destroy the tabs.
//  Name server unbinding is done in a separate step below.
//
tCIDLib::TVoid TCQCTrayMonFrameWnd::Cleanup()
{
    const tCIDLib::TCard4 c4Count = m_colTabs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            m_colTabs[c4Index]->Cleanup();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            facCQCTrayMon.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_TabCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_colTabs[c4Index]->strName()
            );
        }
    }

    //
    //  Clear out our list, which doesn't own them, and then ask the
    //  tabbed window to clean them up.
    //
    try
    {
        m_colTabs.RemoveAll();
        m_pwndTabs->CloseAllTabs();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


//
//  Create ourself, and then load up any tabs that we need to.
//
tCIDLib::TVoid
TCQCTrayMonFrameWnd::Create(const   tCIDLib::TCard4 bAppCtrl
                            , const tCIDLib::TCard4 biTunes
                            , const TIcon&          iconTray)
{
    // Save awawy the options flags
    m_bAppCtrl = bAppCtrl;
    m_biTunes  = biTunes;

    //
    //  Try to create the main frame. We are initially invisible. The iniital
    //  size doesn't matter. We'll size up to fit the dialog resource driven
    //  contents.
    //
    TParent::CreateFrame
    (
        nullptr
        , TArea(10, 10, 100, 100)
        , facCQCTrayMon.strMsg(kTrayMonMsgs::midMain_Title)
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    // Set the icon on us
//    SetFrameIcons(iconTray, iconTray);

    // Do the initial creation of our tabs
    CreateTabs();
}


//
//  Load up our tabs. They will create and register server side ORB
//  objects as required.
//
tCIDLib::TVoid TCQCTrayMonFrameWnd::CreateTabs()
{
    if (m_bAppCtrl)
        m_colTabs.Add(new TCQCTrayAppCtrlTab);

    if (m_biTunes)
        m_colTabs.Add(new TCQCTrayiTunesTab);

    const tCIDLib::TCard4 c4Count = m_colTabs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colTabs[c4Index]->CreateTab(*m_pwndTabs);
}


//
//  This is called during shutdown to remove any name server bindings that
//  any of our tabs might have created. We ask each of them in turn to clean
//  any bindings.
//
tCIDLib::TVoid
TCQCTrayMonFrameWnd::RemoveBindings(tCIDOrbUC::TNSrvProxy& orbcNS)
{
    const tCIDLib::TCard4 c4Count = m_colTabs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            m_colTabs[c4Index]->RemoveBindings(orbcNS);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCTrayMon.LogMsg
            (
                CID_FILE
                , CID_LINE
                , kTrayMonMsgs::midStatus_UnbindErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_colTabs[c4Index]->strName()
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCTrayMonFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this and instead of closing, we just hide ourself. We only
//  ever exit via the tray icon's popup menu or when sent a shutdown request
//  by the app tray.
//
tCIDLib::TBoolean TCQCTrayMonFrameWnd::bAllowShutdown()
{
    if (bIsVisible())
        SetVisibility(kCIDLib::False);
    return kCIDLib::False;
}


//
//  We need to create our dialog resource driven contents and load up
//  our tabs.
//
tCIDLib::TBoolean TCQCTrayMonFrameWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // We load our child widgets from a dialog description
    TDlgDesc dlgdMain;
    facCQCTrayMon.bCreateDlgDesc(kCQCTrayMon::ridDlg_Main, dlgdMain);

    //
    //  Set our background color appropriately. This insures that all of the
    //  children also pick up our background color.
    //
    SetBgnColor(TDlgBox::rgbDlgBgn(dlgdMain.eTheme()));

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    //
    //  Size us to the area indicated in the dialog resource. We have to get a frame
    //  area that will fit that client area content.
    //
    SizeToClient(dlgdMain.areaPos());

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kCQCTrayMon::ridDlg_Main_Tabs, m_pwndTabs);

    // Set min/max tracking sizes
    SetMinMaxSize(TSize(360, 320), TWindow::wndDesktop().areaWndSize().szArea());

    return kCIDLib::True;
}


