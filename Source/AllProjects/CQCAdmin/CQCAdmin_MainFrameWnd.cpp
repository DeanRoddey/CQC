//
// FILE NAME: CQCAdmin_MainFrameWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This file implements the main frame window. It is split into more than
//  one file, since it is kind of large. This guy it pretty much the center of the
//  CQC universe.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_ClientDrvTab.hpp"
#include    "CQCAdmin_EvMonTab.hpp"
#include    "CQCAdmin_SchedEvTab.hpp"
#include    "CQCAdmin_TrigEvTab.hpp"
#include    "CQCAdmin_ScaleTmplsDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWnd)



// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_bSizePosChanged(kCIDLib::False)
    , m_colSkipWnds(tCIDLib::EAdoptOpts::NoAdopt)
    , m_pwndBrowserTab(nullptr)
    , m_pwndClient(nullptr)
    , m_pwndEditPane(nullptr)
    , m_pwndProjPane(nullptr)
    , m_tmidCheckChanges(kCIDCtrls::tmidInvalid)
    , m_tmidDelayInit(kCIDCtrls::tmidInvalid)
    , m_tmidStateFlush(kCIDCtrls::tmidInvalid)
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Let the parent frame class see it. It will process it based on the hot keys of
//  the menu bar. If not one of his, then pass it to the active tab if any.
//
tCIDLib::TBoolean
TMainFrameWnd::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    tCIDLib::TBoolean bHandled = TParent::bProcessAccel(pMsgData);

    if (m_pwndClient && !bHandled)
        bHandled = m_pwndClient->bProcessAccel(pMsgData);

    return bHandled;
}



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Called to create the window and get it initialized sized as desired
tCIDLib::TBoolean TMainFrameWnd::bCreateMain(const TCQCCAppState& fstInit)
{
    // Save the init info since we have to do a delayed show via a timer
    m_fstInit = fstInit;

    tCIDLib::TCardList fcolAreas;
    TParent::CreateFrame
    (
        nullptr
        , fstInit.areaFrame()
        , facCQCAdmin.strMsg(kCQCAMsgs::midWnd_AppName)
        , fcolAreas
        , facCQCAdmin
        , kCQCAdmin::ridMenu_Main
    );
    return kCIDLib::True;
}


// Register us to get events from the browser
tCIDLib::TVoid
TMainFrameWnd::RegisterBrowserEvs(TCQCTreeBrowser& wndBrowser)
{
    wndBrowser.pnothRegisterBrowserHandler(this, &TMainFrameWnd::eBrowseHandler);
}


//
//  The facility class calls this once setup is done. We do any stuff we need to get
//  going. At this point all windows and controls have been created.
//
tCIDLib::TVoid TMainFrameWnd::SetupComplete()
{
    //
    //  Start a delayed show timer. It's the only way to make sure that everything draws
    //  correctly initially. It will be stopped after the first shot.
    //
    m_tmidDelayInit = tmidStartTimer(100);

    //
    //  Start up a timer which we use to periodically flush our current state info out
    //  to our per-app/user object store.
    //
    m_tmidStateFlush = tmidStartTimer(5000);


    // A timer to update the current tab to indicate changes have been made
    m_tmidCheckChanges = tmidStartTimer(4000);
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// Set ourself as the active frame window when we are activated
tCIDLib::TVoid
TMainFrameWnd::ActivationChange(const   tCIDLib::TBoolean       bState
                                , const tCIDCtrls::TWndHandle   hwndOther)
{
    TParent::ActivationChange(bState, hwndOther);
    if (bState)
        facCIDCtrls().SetActiveTopWnd(this);
}



tCIDLib::TVoid
TMainFrameWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && (m_pwndClient != nullptr))
    {
        TArea areaClient;
        QueryClientArea(areaClient, kCIDLib::True, m_colSkipWnds);

        if (bSizeChanged && bOrgChanged)
            m_pwndClient->SetSizePos(areaClient, kCIDLib::True);
        else if (bSizeChanged)
            m_pwndClient->SetSize(areaClient.szArea(), kCIDLib::True);
        else if (bOrgChanged)
            m_pwndClient->SetPos(areaClient.pntOrg());
    }

    // Remember a change so our state flushing timer will store new data next time
    m_bSizePosChanged = kCIDLib::True;
}


//
//  This is called to ask if it's ok to shut down. We run through our current tabs and
//  see if any have changes and try to save them. If any fail, we ask if they want to
//  stop or continue the shutdown.
//
tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    //
    //  Call a helper, since there are multiple places where we might need to save the
    //  all changes. If it returns false, then the user cancelled, so we do nothing,
    //  else it returns true and the shutdown continues.
    //
    return bSaveAllTabs();
}


//
//  We need to set up our main client window, which is a pane window manager, and add the
//  main panes to it, which are the project tabs pane on the left and the main content pane
//  on the right. We add the browser tab to the project tabs pane.
//
//  We register ourself to get events from editor pane, mostly so that we can respond to
//  clicks on the X icon we put in the tabs, and close them down.
//
//  The browser tab also sets us up to get notifications from him, so that we can
//  react to the user invoking actions on browsed content.
//
tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    if (!TParent::bCreated())
        return kCIDLib::False;

    // Set our icon
    facCQCAdmin.SetFrameIcon(*this, L"AdminIntf");

    //
    //  Get our client area now, before we create any other windows. It won't
    //  include any status bar, menu etc... We'll use this to initially set the
    //  area of the main client pane container.
    //
    TArea areaClient;
    QueryClientArea(areaClient, kCIDLib::True);

    // Set a minimum track size. We don't limit the max.
    SetMinMaxSize(TSize(640, 480), TSize(0, 0));

    // Create the main client pane container window and set it as our client.
    m_pwndClient = new TPaneWndCont();
    m_pwndClient->CreatePaneCont
    (
        *this
        , areaClient
        , widNext()
        , tCIDCtrls::EWndStyles::PaneContainer
        , tCIDCtrls::EExWndStyles::Composited
    );

    //  Put the client window into a skip list that we'll pass into subsequent
    //  queries to get the client area.
    //
    m_colSkipWnds.Add(m_pwndClient);

    // Set its background to the app workspace color, for contrast
    m_pwndClient->SetBgnColor
    (
        facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::AppWorkspace), kCIDLib::True
    );

    // Set our layout, which is the two vertical panes scheme
    m_pwndClient->SetLayout(new TPaneLayout2Vert);

    //
    //  Create the tabbed window and add it as the left pane, with a minimum
    //  width of the calculated tab width.
    //
    m_pwndProjPane = new TTabbedWnd();
    m_pwndProjPane->CreateTabbedWnd
    (
        *m_pwndClient
        , widNext()
        , TArea(0, 0, 128, areaClient.c4Height())
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::ClipChildren
          )
        , tCIDCtrls::ETabWStyles::None
    );
    m_pwndProjPane->SetBgnColor
    (
        facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window), kCIDLib::True
    );
    m_pwndClient->SetPane(m_pwndProjPane, 0, TSize(128, 0), TSize(180, 0));

    //
    //  Add a tree editor tab to the project pane. We keep a pointer around since
    //  we have to make some calls to it.
    //
    m_pwndBrowserTab = new TBrowserTab();
    m_pwndBrowserTab->CreateTab(*m_pwndProjPane);

    //
    //  And another tabbed window for the right side, where we'll add tabs for each opened
    //  file. We set an image list on it, which contains a small X, which we'll use for a
    //  close button.
    //
    //  Indicate it is a top level one, so that it'll register alternate F4/F6 accel table
    //  entries.
    //
    m_pwndEditPane = new TTabbedWnd();
    m_pwndEditPane->CreateTabbedWnd
    (
        *m_pwndClient
        , widNext()
        , TArea(0, 0, 128, areaClient.c4Height())
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::ClipChildren
          )
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::ETabWStyles::TwoLineHdr, tCIDCtrls::ETabWStyles::TopLevel
          )
        , tCIDCtrls::EExWndStyles::ControlParent
    );
    m_pwndEditPane->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window), kCIDLib::True);

    // And set the edit pane as the right hand pane
    m_pwndClient->SetPane(m_pwndEditPane, 1, TSize(480, 0), TSize(380, 0));

    // Initialize the panes now
    m_pwndClient->InitPanes(kCIDLib::True);

    // Register us for event notifications from the editor pane
    m_pwndEditPane->pnothRegisterHandler(this, &TMainFrameWnd::eEditTabHandler);

    // Get the focus initially on the browser
    m_pwndBrowserTab->wndBrowser().TakeFocus();

    return kCIDLib::True;
}



tCIDLib::TVoid TMainFrameWnd::Destroyed()
{
    // Stop any timers that we started and didn't get stopped
    if (m_tmidCheckChanges)
    {
        StopTimer(m_tmidCheckChanges);
        m_tmidCheckChanges = kCIDCtrls::tmidInvalid;
    }

    if (m_tmidDelayInit)
    {
        StopTimer(m_tmidDelayInit);
        m_tmidDelayInit = kCIDCtrls::tmidInvalid;
    }

    if (m_tmidStateFlush)
    {
        StopTimer(m_tmidStateFlush);
        m_tmidStateFlush = kCIDCtrls::tmidInvalid;
    }

    //
    //  Do a last store of the state info, to get anything that the timer didn't get
    //  at the end. Do this before we start closing stuff down, so that we capture the
    //  actual state at the point of shutdown.
    //
    StoreStateInfo();

    // Close all of our tabs
    try
    {
        if (m_pwndEditPane && m_pwndEditPane->bIsValid())
            m_pwndEditPane->CloseAllTabs();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // And pass on to our parent
    TParent::Destroyed();
}


tCIDLib::TVoid TMainFrameWnd::DragEnd()
{
    //
    //  Originally done to deal with redrawing issues on the list view control, but we
    //  appear to have gotten rid of those and shouldn't need this anymore.
    //
//    m_pwndClient->Redraw(tCIDCtrls::ERedrawFlags::Full);
}


//
//  Our base class will handle any hot keys mapped by our menu bar, and that will
//  cause us to get this call.
//
tCIDLib::TVoid
TMainFrameWnd::MenuCommand( const   tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    //
    //  Some of these work on the current tab or any tabs open, so get that. If none,
    //  then all of those are ignored.
    //
    TContentTab* pwndCurTab = static_cast<TContentTab*>(m_pwndEditPane->pwndSelected());

    if ((ridItem == kCQCAdmin::ridMenu_Main_File_Close)
    ||  (ridItem == kCQCAdmin::ridMenu_Main_File_CloseAll)
    ||  (ridItem == kCQCAdmin::ridMenu_Main_File_Save)
    ||  (ridItem == kCQCAdmin::ridMenu_Main_File_SaveAll))
    {
        // Can't do these if there's no tab(s)
        if (pwndCurTab == nullptr)
            return;
    }

    if (ridItem == kCQCAdmin::ridMenu_Main_File_About)
    {
        // Show our generic about dialog
        facCQCIGKit().AboutDlg(*this);
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_Close)
    {
        //
        //  Call a helper, since we have to do this from other places as well. If he
        //  says it's ok, we can close the tab.
        //
        if (bCloseTab(pwndCurTab))
            m_pwndEditPane->CloseTabById(pwndCurTab->c4TabId());
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_CloseAll)
    {
        // If all changes are saved successfully, then close all tabs
        if (bSaveAllTabs())
        {
            m_pwndEditPane->CloseAllTabs();

            // Make sure the status bar is cleared out
            ClearStatusBar();
        }
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_ChangePW)
    {
        facCQCGKit().ChangePassword
        (
            *this, facCQCAdmin.strUserName(), facCQCAdmin.sectUser()
        );
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_Next)
    {
        // Just tell our editor pane to move to the next tab if he has one
        m_pwndEditPane->ActivateNextTab();
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_Save)
    {
        bSaveTab(*pwndCurTab);
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_SaveAll)
    {
        bSaveAllTabs();
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_Tools_FlushLogs)
    {
        FlushLogs();
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_Tools_ImportPack)
    {
        // Call a helper to do this
        ImportPackage();
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_Tools_ScaleTmpls)
    {
        TCQCScaleTmplsDlg dlgScale;
        dlgScale.Run(*this);
    }
     else if (ridItem == kCQCAdmin::ridMenu_Main_File_Exit)
    {
        // Call the shutdown ok checker. If OK, exit the main loop
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
}


tCIDLib::TVoid TMainFrameWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo == m_tmidCheckChanges)
    {
        if (m_pwndEditPane)
        {
            TContentTab* pwndCurTab = static_cast<TContentTab*>(m_pwndEditPane->pwndSelected());
            if (pwndCurTab)
                CheckTabChanges(*pwndCurTab);
        }
    }
     else if (tmidToDo == m_tmidDelayInit)
    {
        // First of all, stop the timer so it doesn't keep going off
        StopTimer(m_tmidDelayInit);
        m_tmidDelayInit = kCIDCtrls::tmidInvalid;

        // Now pause a bit to pump some messages
        facCIDCtrls().MsgYield(150);

        //
        //  Change our size to insure that everything shows up. We just change it
        //  by a pixel, then change it back. If we don't do this, the stupid status
        //  bar will not be visible.
        //
        TArea areaNew = areaWnd();
        areaNew.Deflate(1, 1);
        SetSizePos(areaNew, kCIDLib::False);
        areaNew.Inflate(1, 1);
        SetSizePos(areaNew, kCIDLib::False);

        //
        //  Now finally we can set restore any previous state (or the default) if this
        //  is our first run.
        //
        RestorePosState(m_fstInit.ePosState(), areaWnd(), kCIDLib::False);

        //
        //  Check to see if the lat/long information has been set yet. If not,
        //  then get it.
        //
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCIDLib::TFloat8 f8Lat, f8Long;
            facCQCIGKit().bGetLocInfo(*this, f8Lat, f8Long, kCIDLib::False, facCQCAdmin.sectUser());
        }

        catch(TError& errToCatch)
        {
            // Tell them we couldn't do it
            if (facCQCAdmin.bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCGKit().ShowError
            (
                *this
                , strWndText()
                , facCQCIGKit().strMsg(kIGKitErrs::errcCfg_GetLatLong)
                , errToCatch
            );
        }
    }
     else if (tmidToDo == m_tmidStateFlush)
    {
        //
        //  If we are currently in motion, then don't do anything, because the
        //  disk I/O will make it jerky.
        //
        if (!bInDragMode())
            StoreStateInfo();
    }
     else
    {
        // Nothing of ours, so pass it on
        TParent::Timer(tmidToDo);
    }
}



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If any changes, ask if they want to save them. If so, then try to do so. If there
//  are validation errors, show the error and ask if they want to stop the closing of the
//  tab or lose changes.
//
//  WE DO NOT actually close the tab, since this is called from both the proactive close
//  by the user via menu, and via a close by click on the X in the tab. In the latter case
//  the tabbed window itself closes the tab, if we say it can.
//
tCIDLib::TBoolean TMainFrameWnd::bCloseTab(TContentTab* const pwndToClose)
{
    // First do a 'test only' save. WE don't use the last parameter here
    tCIDLib::TBoolean bDummy = kCIDLib::False;
    TString strErrMsg;
    tCIDLib::ESaveRes eRes = pwndToClose->eSaveChanges
    (
        strErrMsg, tCQCAdmin::ESaveModes::Test, bDummy
    );

    if (eRes == tCIDLib::ESaveRes::Errors)
    {
        //
        //  Show him the error and ask if changes should be discarded. If not, then
        //  we don't close the tab. The discard all flag is ignore here since we invoke
        //  the dialog in single item mode.
        //
        tCIDLib::TBoolean bDiscardAll = kCIDLib::False;
        const tCIDWUtils::EErrOpts eErrRes = facCIDWUtils().eGetErrOpt2
        (
            *this
            , bDiscardAll
            , pwndToClose->strContName()
            , strErrMsg
            , kCIDLib::False
            , kCIDLib::True
        );

        // If they choose to cancel, then we say, no, don't close it
        if (eErrRes == tCIDWUtils::EErrOpts::Cancel)
            return kCIDLib::False;
    }
     else if (eRes == tCIDLib::ESaveRes::OK)
    {
        // There are valid changes, so ask if he wants to save them
        TYesNoBox msgbSave
        (
            pwndToClose->strContName(), facCQCAdmin.strMsg(kCQCAMsgs::midQ_SaveChanges)
        );

        if (msgbSave.bShowIt(*this))
        {
            // If not, then don't close the tab
            if (!bSaveTab(*pwndToClose))
                return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  We check all opened tabs. For any that have changes we give them a chance to
//  save. If any report errors, we give the user a chance to cancel, and to indicate
//  that any subsequent errors should be ignored.
//
tCIDLib::TBoolean TMainFrameWnd::bSaveAllTabs()
{
    tCIDLib::TBoolean bDiscardAll = kCIDLib::False;

    TString strTarget;
    TString strErrMsg;
    tCIDLib::TCard4 c4TabCnt = m_pwndEditPane->c4TabCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
    {
        TContentTab* pwndCur = static_cast<TContentTab*>(m_pwndEditPane->pwndTabAt(c4Index));

        // Do a save. If errors or changes saved, handle those
        tCIDLib::TBoolean bChangesSaved = kCIDLib::False;
        const tCIDLib::ESaveRes eRes = pwndCur->eSaveChanges
        (
            strErrMsg, tCQCAdmin::ESaveModes::Save, bChangesSaved
        );

        if (eRes == tCIDLib::ESaveRes::Errors)
        {
            // If they haven't already decided to discard all errors, then ask what to do
            tCIDWUtils::EErrOpts eOpt = tCIDWUtils::EErrOpts::Ignore;
            if (!bDiscardAll)
            {
                eOpt = facCIDWUtils().eGetErrOpt2
                (
                    *this
                    , bDiscardAll
                    , pwndCur->strContName()
                    , strErrMsg
                    , kCIDLib::True
                    , kCIDLib::True
                );
            }
            else
            {
                // If changes were saved, let the browser tab know
                if (bChangesSaved)
                    m_pwndBrowserTab->wndBrowser().FileUpdated(pwndCur->strPath());
            }

            // If they say to cancel, then give up now and activate this one
            if (eOpt == tCIDWUtils::EErrOpts::Cancel)
            {
                m_pwndEditPane->c4ActivateById(pwndCur->c4TabId());
                return kCIDLib::False;
            }
        }
    }

    // Indicate they all went fine or nothing to do
    return kCIDLib::True;
}


//
//  We unconditionally store away any changes, or display errors if there are
/// any.
//
tCIDLib::TBoolean TMainFrameWnd::bSaveTab(TContentTab& wndToSave)
{
    // Stop the periodic check for changes timer while we are in here
    TTimerJanitor janChangeTimer(this, m_tmidCheckChanges);

    tCIDLib::TBoolean bChangesSaved = kCIDLib::False;
    TString strErrMsg;
    tCIDLib::ESaveRes eRes;
    try
    {
        eRes = wndToSave.eSaveChanges(strErrMsg, tCQCAdmin::ESaveModes::Save, bChangesSaved);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , strWndText()
            , facCQCAdmin.strMsg(kCQCAErrs::errcGen_SaveChanges)
            , errToCatch
        );
        return kCIDLib::False;
    }

    if (eRes == tCIDLib::ESaveRes::Errors)
    {
        TErrBox msgbErr(wndToSave.strContName(), strErrMsg);
        msgbErr.ShowIt(*this);
    }
    else
    {
        // Let it immediately update it's changes indicator
        CheckTabChanges(wndToSave);

        // Let the browser handler know we've updated this guy
        if (bChangesSaved)
            m_pwndBrowserTab->wndBrowser().FileUpdated(wndToSave.strPath());
    }
    return (eRes != tCIDLib::ESaveRes::Errors);
}


// We need to do this from a couple places, so break it out.
tCIDLib::TVoid TMainFrameWnd::CheckTabChanges(TContentTab& wndTar)
{
    // Stop the periodic change check timer while we are in here
    TTimerJanitor janChangeTimer(this, m_tmidCheckChanges);

    try
    {
        // We don't use the last parameter here, it's only used in a real save
        tCIDLib::TBoolean bDummy;
        TString strMsg;
        wndTar.eSaveChanges(strMsg, tCQCAdmin::ESaveModes::Test, bDummy);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            *this
            , strWndText()
            , facCQCAdmin.strMsg(kCQCAErrs::errcGen_CheckChanges)
            , errToCatch
        );
    }
}


//
//  Clears out the status bar. We have to do this when the last tab is closed, or all tabs
//  are closed at once.
//
tCIDLib::TVoid TMainFrameWnd::ClearStatusBar()
{
    tCIDLib::TCardList fcolAreas;
    fcolAreas.c4AddElement(100);
    SetStatusBarAreas(fcolAreas, kCIDLib::True);
    SetStatusBarText(0, TString::strEmpty());
}


//
//  Called when the user requests the creation of a package via the menu. This should only
//  be seen for images, templates or macros, in the User area.
//
tCIDLib::TVoid TMainFrameWnd::CreatePackage(const TTreeBrowseInfo& wnotBrowser)
{

    tCQCKit::EPackTypes ePackType = tCQCKit::EPackTypes::Count;
    tCQCKit::EPackFlTypes eFlType = tCQCKit::EPackFlTypes::Count;
    TPathStr pathPackFlName;

    // Get a target name for the package
    {
        //
        //  Let them pick the correct type for the selected file. And, while we are about
        //  it, translate the incoming data type to a package type and file type.
        //
        TString strExt;
        tCIDLib::TKVPList colFileTypes(1);
        if (wnotBrowser.eType() == tCQCRemBrws::EDTypes::Image)
        {
            colFileTypes.objAdd(TKeyValuePair(L"CQC Image Pack", L"*.CQCImgPack"));
            ePackType = tCQCKit::EPackTypes::ImgBundle;
            eFlType = tCQCKit::EPackFlTypes::PNGImage;
            strExt = L"CQCImgPack";
        }
        else if (wnotBrowser.eType() == tCQCRemBrws::EDTypes::Macro)
        {
            colFileTypes.objAdd(TKeyValuePair(L"CQC Macro Pack", L"*.CQCMacroPack"));
            ePackType = tCQCKit::EPackTypes::MacroBundle;
            eFlType = tCQCKit::EPackFlTypes::CMLMacro;
            strExt = L"CQCMacroPack";
        }
        else if (wnotBrowser.eType() == tCQCRemBrws::EDTypes::Template)
        {
            colFileTypes.objAdd(TKeyValuePair(L"CQC Template Pack", L"*.CQCTmplPack"));
            ePackType = tCQCKit::EPackTypes::IntfTemplate;
            eFlType = tCQCKit::EPackFlTypes::IntfTmpl;
            strExt = L"CQCTmplPack";
        }
         else
        {
            CIDAssert2(L"Unsupported package type");

            // Make analyzer happy
            return;
        }

        tCIDLib::TStrList colFiles;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facCQCKit().strMsg(kKitMsgs::midMsg_SelectPackName)
            , TString::strEmpty()
            , colFiles
            , colFileTypes
            , tCIDCtrls::EFOpenOpts::StrictTypes
        );

        if (!bRes)
            return;

        // They committed so save the file
        pathPackFlName = colFiles[0];

        // If no extension, or the wrong one, then add it ourself
        TString strTmpExt;
        if (!pathPackFlName.bQueryExt(strTmpExt) || !strTmpExt.bCompareI(strExt))
            pathPackFlName.AppendExt(strExt);
    }

    CIDAssert
    (
        (ePackType != tCQCKit::EPackTypes::Count)
        && (eFlType != tCQCKit::EPackFlTypes::Count)
        , L"Failed to initialize package or package file type"
    )

    // If it exists, make sure they want to overwrite
    if (TFileSys::bExists(pathPackFlName, tCIDLib::EDirSearchFlags::AllFiles))
    {
        TYesNoBox msgbOver(L"This file already exists. Do you want to overwrite it?");
        if (!msgbOver.bShowIt(*this))
            return;
    }

    // Get the relative path of the selected thing
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(wnotBrowser.strPath(), wnotBrowser.eType(), strRelPath);

    //
    //  OK, let's do it. Depending on the data type and item/scope selected, get the
    //  initial contents into the package.
    //
    const tCIDLib::TBoolean bIsScope(m_pwndBrowserTab->wndBrowser().bIsScope(wnotBrowser.strPath()));
    TCQCPackage packNew(ePackType, kCQCKit::c8Ver_Current);
    try
    {
        TDataSrvClient dsclLoad;

        //
        //  Get the relative parent scope. If it's a scope, then already have it. Else remove
        //  the last path item.
        //
        TString strRelScope(strRelPath);
        if (!bIsScope)
            facCQCRemBrws().bRemoveLastPathItem(strRelScope);

        //
        //  If a scope, get all of the names of the things in that scope. Else just add the
        //  name of the one thing selected. In either case we want just the name parts in
        //  the list, relative to the parent scope we got above.
        //
        tCIDLib::TStrList colImages;
        if (bIsScope)
        {
            tCIDLib::TCard4 c4ScopeId = 0;
            dsclLoad.bQueryScopeNames
            (
                c4ScopeId
                , strRelScope
                , wnotBrowser.eType()
                , colImages
                , tCQCRemBrws::EQSFlags::Items
                , facCQCAdmin.sectUser()
            );
        }
         else
        {
            TString strName(strRelPath);
            strName.Cut(0, strRelScope.c4Length() + 1);
            colImages.objAdd(strName);
        }

        //
        //  If an image or macro, we can handle them generically and just add all fo the
        //  files to the package. The template is a special case.
        //
        if ((wnotBrowser.eType() == tCQCRemBrws::EDTypes::Image)
        ||  (wnotBrowser.eType() == tCQCRemBrws::EDTypes::Macro))
        {
            // Loop through all the files and add them to the package
            const tCIDLib::TCard4 c4Count = colImages.c4ElemCount();
            TString strCurImg;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strCurImg = strRelScope;
                strCurImg.Append(kCIDLib::chForwardSlash);
                strCurImg.Append(colImages[c4Index]);

                // It's just a single file
                facCQCIGKit().AddFileToPack
                (
                    dsclLoad
                    , packNew
                    , eFlType
                    , strCurImg
                    , wnotBrowser.eType()
                    , facCQCAdmin.sectUser()
                );
            }
        }
         else if (wnotBrowser.eType() == tCQCRemBrws::EDTypes::Template)
        {
            // It has to be an item in this case, which the caller should have checked
            CIDAssert(!bIsScope, L"The selected item must be a single template");

            // This one takes a bit of work, so call a separate method
            CreateTmplPackage(strRelPath, packNew, dsclLoad);
        }

    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , L"Package Creation Error"
            , L"An error occured while set up the initial package contents"
            , errToCatch
        );
        return;
    }

    //
    //  Now invoke the packaging dialog to let them make more changes and save. For
    //  template packs, we only let them add more images, so we have to be sure to update
    //  the data type in some cases.
    //
    tCQCRemBrws::EDTypes eAddType = wnotBrowser.eType();
    if (eAddType == tCQCRemBrws::EDTypes::Template)
        eAddType = tCQCRemBrws::EDTypes::Image;

    const tCIDLib::TBoolean bRes = facCQCIGKit().bManagePackage
    (
        *this
        , packNew
        , strRelPath
        , kCIDLib::False
        , eAddType
        , facCQCAdmin.cuctxToUse()
    );

    // If the committed, then create the package
    if (bRes)
        TCQCPackage::MakePackage(pathPackFlName, packNew);
}


//
//  Called from CreatePackage() above, when it's a template package. This involves a lot
//  more work than the other types. We have to:
//
//  1. Read the template
//  2. Add that to the package
//  3. Query all of the monikers referenced in the template, generate a device map, flatten
//     it and add that to the package
//  4. Query all of the referenced images in the template, and add those to the package
//
tCIDLib::TVoid
TMainFrameWnd::CreateTmplPackage(const  TString&        strRelPath
                                ,       TCQCPackage&    packNew
                                ,       TDataSrvClient& dsclLoad)
{
    TChunkedFile chflTmpl;
    {
        tCIDLib::TCard4 c4SerNum = 0;
        tCIDLib::TEncodedTime enctLast;
        tCIDLib::TKVPList colMeta;
        dsclLoad.bReadFile
        (
            strRelPath
            , tCQCRemBrws::EDTypes::Template
            , c4SerNum
            , enctLast
            , chflTmpl
            , facCQCAdmin.sectUser()
        );

        //
        //  OK, let's add this guy to the package, while we have the raw buffer data
        //  that we just read. Let it compress the data.
        //
        packNew.c4AddFile
        (
            tCQCKit::EPackFlTypes::IntfTmpl
            , strRelPath
            , chflTmpl
            , kCIDLib::True
            , kCIDLib::False
        );
    }

    // STream in the template from the data chunk
    TCQCIntfTemplate iwdgTmpl;
    {
        TBinMBufInStream strmSrc(&chflTmpl.mbufData(), chflTmpl.c4DataBytes());
        strmSrc >> iwdgTmpl;
    }

    // OK, now let's query the monikers and create the map
    tCIDLib::TStrHashSet colMonikers(29, TStringKeyOps());
    iwdgTmpl.QueryMonikers(colMonikers);

    tCIDLib::TStrHashSet::TCursor cursMonikers(&colMonikers);
    if (cursMonikers.bIsValid())
    {
        // Query moniker, make and model info for all of our configured drivers
        tCIDLib::TStrList colCfgMons;
        tCIDLib::TStrList colCfgMakes;
        tCIDLib::TStrList colCfgModels;
        tCQCKit::TDevCatList fcolCfgCats;

        {
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
            orbcIS->DrvCfgQueryDrvsInfo
            (
                colCfgMons, colCfgMakes, colCfgModels, fcolCfgCats, facCQCAdmin.sectUser()
            );
        }
        const tCIDLib::TCard4 c4CfgDrvCnt = colCfgMons.c4ElemCount();

        TCQCFldDef              flddCur;
        TCQCDriverObjCfg        cqcdcCur;
        TTextStringOutStream    strmMap(2048UL);
        TString                 strNotFnd;

        for (; cursMonikers; ++cursMonikers)
        {
            const TString& strCurMon = *cursMonikers;

            // Find this one in our list
            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4CfgDrvCnt)
            {
                if (colCfgMons[c4Index].bCompareI(strCurMon))
                    break;
                c4Index++;
            }

            if (c4Index < c4CfgDrvCnt)
            {
                strmMap << strCurMon << L" : "
                        << colCfgMakes[c4Index] << L" , "
                        << colCfgModels[c4Index] << kCIDLib::NewLn;
            }
             else
            {
                if (!strNotFnd.bIsEmpty())
                    strNotFnd.Append(L", ");
                strNotFnd.Append(strCurMon);
            }
        }

        // If any were not found, then warn them
        if (!strNotFnd.bIsEmpty())
        {
            TErrBox msgbWarn
            (
                facCQCAdmin.strMsg(kCQCAErrs::errcPack_MonNotFound, strNotFnd)
            );
            msgbWarn.ShowIt(*this);
        }

        // Flush the stream and write this out as as a file to the package
        strmMap.Flush();
        packNew.c4AddFile
        (
            tCQCKit::EPackFlTypes::DevMap
            , kCQCIntfEng::pszExpMonMapFld
            , strmMap.strData()
            , kCIDLib::False
        );
    }
     else
    {
        // No monikers, so just write out an empty map file
        packNew.c4AddFile
        (
            tCQCKit::EPackFlTypes::DevMap
            , kCQCIntfEng::pszExpMonMapFld
            , TString::strEmpty()
            , kCIDLib::False
        );
    }

    //
    //  Ask the template for a list of all of the images it references. It will not
    //  return any duplicates. Tell it we do not want any system images.
    //
    //  Set up the key ops objects to do non-case sensitive hashing/comparing, so that
    //  differences in case don't cause us to get dups.
    //
    tCIDLib::TStrHashSet    colImgs(109, TStringKeyOps(kCIDLib::False));
    tCIDLib::TStrHashSet    colScopes(109, TStringKeyOps(kCIDLib::False));
    iwdgTmpl.QueryReferencedImgs
    (
        colImgs, colScopes, kCIDLib::False, kCIDLib::True
    );

    // Do the individual images
    tCIDLib::TStrHashSet::TCursor cursImgs(&colImgs);
    for (; cursImgs; ++cursImgs)
    {
        try
        {
            facCQCIGKit().AddFileToPack
            (
                dsclLoad
                , packNew
                , tCQCKit::EPackFlTypes::PNGImage
                , cursImgs.objRCur()
                , tCQCRemBrws::EDTypes::Image
                , facCQCAdmin.sectUser()
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Now iterate any scopes and get all images in those scopes and add those
    tCIDLib::TStrHashSet::TCursor cursScopes(&colScopes);
    for (; cursScopes; ++cursScopes)
    {
        tCIDLib::TStrList colImages;

        tCIDLib::TCard4 c4ScopeId = 0;
        dsclLoad.bQueryScopeNames
        (
            c4ScopeId
            , cursScopes.objRCur()
            , tCQCRemBrws::EDTypes::Image
            , colImages
            , tCQCRemBrws::EQSFlags::Items
            , facCQCAdmin.sectUser()
        );

        const tCIDLib::TCard4 c4Count = colImages.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            try
            {
                facCQCIGKit().AddFileToPack
                (
                    dsclLoad
                    , packNew
                    , tCQCKit::EPackFlTypes::PNGImage
                    , colImages[c4Index]
                    , tCQCRemBrws::EDTypes::Image
                    , facCQCAdmin.sectUser()
                );
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  When we create the browser tab, it will register us to get notifcations from
//  the brwoser window it creates.
//
tCIDCtrls::EEvResponses TMainFrameWnd::eBrowseHandler(TTreeBrowseInfo& wnotBrowser)
{
    switch(wnotBrowser.eEvent())
    {
        case tCQCTreeBrws::EEvents::CanRename :
        case tCQCTreeBrws::EEvents::IsOpen :
        {
            //
            //  The browser window sends this before renaming something. And it also
            //  can ask if something is open. For us, the work is the same. If we have
            //  an open tab, we say no to the can rename or yes to the isopen.
            //
            //  If the path is a scope, then we see if any current tabs have a path that
            //  start with this scope path. If so, we say it is open
            //
            tCIDLib::TBoolean bIsOpen = kCIDLib::False;
            if (m_pwndBrowserTab->wndBrowser().bIsScope(wnotBrowser.strPath()))
            {
                //
                //  Make sure the path ends with a slash, so that we can't accidentally match
                //  it partially.
                //
                TString strTestPath(wnotBrowser.strPath());
                if (strTestPath.chLast() != kCIDLib::chForwardSlash)
                    strTestPath.Append(kCIDLib::chForwardSlash);

                // Iterate all the open tabs and see if any of them start with the path
                tCIDLib::TCard4 c4TabCnt = m_pwndEditPane->c4TabCount();
                TContentTab* pwndCur = nullptr;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TabCnt; c4Index++)
                {
                    pwndCur = static_cast<TContentTab*>(m_pwndEditPane->pwndTabAt(c4Index));
                    if (pwndCur->strPath().bStartsWith(strTestPath))
                    {
                        // We have one, so indicate its 'open', or busy in our case
                        bIsOpen = kCIDLib::True;
                        break;
                    }
                }
            }
             else
            {
                // We have to check for either edit or viewing mode tabs!
                bIsOpen =
                (
                    (pwndFindContTab(wnotBrowser.strPath(), kCIDLib::False) != nullptr)
                    || (pwndFindContTab(wnotBrowser.strPath(), kCIDLib::True) != nullptr)
                );
            }

            if (wnotBrowser.eEvent() == tCQCTreeBrws::EEvents::IsOpen)
                wnotBrowser.bFlag(bIsOpen);
            else
                wnotBrowser.bFlag(!bIsOpen);
            break;
        }

        case tCQCTreeBrws::EEvents::ClientDrv :
        {
            LoadClientDrv(wnotBrowser);
            break;
        }

        case tCQCTreeBrws::EEvents::Deleted :
        {
            // A file has been deleted. We need to close it and discard any changes
            TContentTab* pwndTab = pwndFindContTab(wnotBrowser.strPath(), kCIDLib::False);
            if (pwndTab)
                m_pwndEditPane->CloseTabById(pwndTab->c4TabId());

            pwndTab = pwndFindContTab(wnotBrowser.strPath(), kCIDLib::True);
            if (pwndTab)
                m_pwndEditPane->CloseTabById(pwndTab->c4TabId());
            break;
        }

        case tCQCTreeBrws::EEvents::Edit :
        case tCQCTreeBrws::EEvents::View :
            //
            //  Call a helper that will load up the indicated content in either view
            //  or edit mode.
            //
            LoadFile(wnotBrowser, wnotBrowser.eEvent() == tCQCTreeBrws::EEvents::Edit);
            break;


        case tCQCTreeBrws::EEvents::NewFile :
            //
            //  No need to deal with this one currently. The browser is just informing us
            //  that he has added a new file to the browser.
            //
            break;

        case tCQCTreeBrws::EEvents::NewScope :
            //
            //  No need to deal with this one currently. The browser is just informing us
            //  that he has added a new scope to the browser.
            //
            break;

        case tCQCTreeBrws::EEvents::PausedResumed :
        {
            //
            //  If it's an event, and we have it open, we need to update our live config
            //  with the new status. Otherwise, if they make changes and safe, those
            //  changes won't reflect this change in paused status.
            //
            UpdatePauseState(wnotBrowser);
            break;
        }

        case tCQCTreeBrws::EEvents::Renamed :
            //
            //  No need to deal with this one currently. The browser is just informing us
            //  that he has renamed a file. If we had it open, we rejected the rename
            //  attemp above (in CanRename.) So if we get here we allowed it and therefore
            //  have nothing to worry about.
            //
            break;

        case tCQCTreeBrws::EEvents::Package :
            //
            //  The user requested that we make a package based on the selection.
            //  This should only be received for templates, images, or macros. We just
            //  call a helper to do the work.
            //
            CreatePackage(wnotBrowser);
            break;

        default :
            break;
    };

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We get notifications from the tabs in the editing area tabbed window.
//
//  If we get a hit on the icon (an x mark) we will close the tab down, after checking to
//  see if there are changes. If any changes, we ask the user if they want to save or close
//  without saving or cancel.
//
//  When we see a change in active tab, we update our status bar with the tab's status bar
//  slot contents. If no tab active, we clear out the status bar.
//
tCIDCtrls::EEvResponses TMainFrameWnd::eEditTabHandler(TTabEventInfo& wnotEdit)
{
    if (wnotEdit.eEvent() == tCIDCtrls::ETabWEvents::Change)
    {
        if (wnotEdit.c4TabId() == kCIDLib::c4MaxCard)
        {
            // No new tab, so clear out the status bar.
            ClearStatusBar();
        }
         else
        {
            //
            //  Get the new tab and let him update our status bar. We can get one of
            //  these during shutdown after the tab is gone, so check for that.
            //
            TContentTab* pwndTab = static_cast<TContentTab*>
            (
                m_pwndEditPane->pwndFindById(wnotEdit.c4TabId())
            );
            if (pwndTab)
                pwndTab->UpdateStatusBar(*this);

            //
            //  Since we use a timer to update the changed status of the active tab, the
            //  user can make a change and quickly move to another tab and we wouldn't
            //  catch it. So it update it upon change.
            //
            if (wnotEdit.c4PrevTabId() != kCIDLib::c4MaxCard)
            {
                pwndTab = static_cast<TContentTab*>
                (
                    m_pwndEditPane->pwndFindById(wnotEdit.c4PrevTabId())
                );
                CheckTabChanges(*pwndTab);
            }
        }
    }
     else if (wnotEdit.eEvent() == tCIDCtrls::ETabWEvents::Close)
    {
        // Get the tab, cast to our base edit tab class
        TContentTab* pwndTab = static_cast<TContentTab*>
        (
            m_pwndEditPane->pwndFindById(wnotEdit.c4TabId())
        );
        CIDAssert(pwndTab != nullptr, L"The reported tab was not found");

        //
        //  Call a helper, since we have to do this from other places as well. If it
        //  says everything is ok, we set the allow flag and the tabbed window will
        //  close the tab.
        //
        wnotEdit.bAllow(bCloseTab(pwndTab));
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// The user requested that the logs be flush, so this is called do the work
tCIDLib::TVoid TMainFrameWnd::FlushLogs()
{
    // Make sure that they really want to do this
    TYesNoBox msgbAsk(facCQCAdmin.strMsg(kCQCAMsgs::midQ_FlushLogs));

    if (msgbAsk.bShowIt(*this))
    {
        // Get a log server proxy
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCIDOrbUC::TLSrvProxy orbcLS = facCIDOrbUC().orbcLogSrvProxy();
            orbcLS->RemoveAll();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"The logs could not be flushed. See the logs for details"
                , errToCatch
            );
        }
    }
}


//
//  This is called when the user asks to import a package. We let them select the pacakge,
//  then invoke a dialog that displays the contents and lets the choose to import it or
//  not. If they choose to, the dialog provides the import functionality. CQCIGKit() provides
//  this dialog for us.
//
tCIDLib::TVoid TMainFrameWnd::ImportPackage()
{
    // Set up a file types collection to set
    tCIDLib::TKVPList colFileTypes(4);
    colFileTypes.objAdd(TKeyValuePair(L"All Packages", L"*.CQC*Pack"));
    colFileTypes.objAdd(TKeyValuePair(L"Driver Pack", L"*.CQCDrvPack"));
    colFileTypes.objAdd(TKeyValuePair(L"Image Pack", L"*.CQCImgPack"));
    colFileTypes.objAdd(TKeyValuePair(L"Template Pack", L"*.CQCTmplPack"));

    tCIDLib::TStrList colFiles;
    tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , facCQCAdmin.strMsg(kCQCAMsgs::midTitle_SelPackage)
        , TString::strEmpty()
        , colFiles
        , colFileTypes
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EFOpenOpts::FileMustExist
            , tCIDCtrls::EFOpenOpts::StrictTypes
          )
    );

    if (!bRes)
        return;

    // OK, let's pass it off to the helper dialog
    bRes = facCQCIGKit().bImportPackage(*this, colFiles[0], facCQCAdmin.cuctxToUse());
}


//
//  This is called when we get a notification from the browser that the user wants to load
//  a client side driver.
//
tCIDLib::TVoid TMainFrameWnd::LoadClientDrv(const TTreeBrowseInfo& wnotBrowser)
{
    //
    //  Get the path and first see if there's a tab already open for this path. These
    //  are always edit mode, so we pass true for that.
    //
    const TString& strPath = wnotBrowser.strPath();
    const TContentTab* pwndTab = pwndFindContTab(strPath, kCIDLib::True);
    if (pwndTab)
    {
        m_pwndEditPane->c4ActivateById(pwndTab->c4TabId());
        return;
    }

    // Get the host and moniker out of the path
    TString strHost;
    TString strMoniker;
    strHost.CopyInSubStr(strPath, kCQCRemBrws::strPath_Devices.c4Length() + 1);
    strHost.bSplit(strMoniker, kCIDLib::chForwardSlash);

    TCQCDriverObjCfg cqcdcDrv;
    TCQCDriverCfg    cqcdcManifest;
    try
    {
        TString strTmpHost(strHost);
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        tCIDLib::TCard4 c4Ver = 0;
        if (!orbcIS->bDrvCfgQueryConfig(c4Ver, strMoniker, strTmpHost, cqcdcDrv, facCQCAdmin.sectUser()))
        {
            TErrBox msgbErr(facCQCAdmin.strMsg(kCQCAErrs::errcDrv_GetConfig, strMoniker));
            msgbErr.ShowIt(*this);
            return;
        }

        //
        //  We also get the current manifest contents. There could have been changes in the
        //  prompts available, and we also will need to update the version info for any
        //  shared facilities.
        //
        if (!orbcIS->bQueryDrvManifest(cqcdcDrv.strMake(), cqcdcDrv.strModel(), cqcdcManifest))
        {
            TErrBox msgbErr
            (
                facCQCAdmin.strMsg
                (
                    kCQCAErrs::errcDrv_GetManifestInfo
                    , cqcdcDrv.strMake()
                    , cqcdcDrv.strModel()
                )
            );
            msgbErr.ShowIt(*this);
            return;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TString strMsg(kCQCAErrs::errcDrv_GetConfig, facCQCAdmin, strMoniker);
        TExceptDlg dlgErr(*this, L"Driver Client Error", strMsg, errToCatch);
        return;
    }

    // Update the current config with the latest manifest info
    try
    {
        cqcdcDrv.UpdateBase(cqcdcManifest);
    }

    catch(TError& errToCatch)
    {
        facCQCGKit().ShowError
        (
            *this
            , strWndText()
            , facCQCAdmin.strMsg(kCQCAErrs::errcDrv_UpdateDrvCfg, strMoniker)
            , errToCatch
        );
        return;
    }

    //
    //  We need to remove the prefix part of the path, since the IDE works in terms
    //  of type relative paths.
    //
    TString strRelPath;
    facCQCRemBrws().CreateRelPath(strPath, tCQCRemBrws::EDTypes::Driver, strRelPath);

    TString strTabText;
    MakeDisplayName(strPath, strTabText);

    TClientDrvTab* pwndNewTab = new TClientDrvTab(strPath, strRelPath, strHost, strMoniker);
    pwndNewTab->CreateTab(*m_pwndEditPane, strTabText, cqcdcDrv);
}


//
//  This is called when we get a command from the browser to view or edit something.
//  Based on the mode and type.
//
//  For data server based content, which has a system/user distinction, we don't just
//  invoke an editor if invoked in view mode. We only do anything if there's a viewer for
//  that data. We will only get called here in edit mode for such things if it's actually
//  editable (i.e. user stuff.) So we don't want to let them edit system content, and that
//  would happen if we just did an edit even if in view mode.
//
//  For other stuff, there's no such distinction so we invoke a viewer if we have it and
//  view mode is requested, else we invoke an editor.
//
tCIDLib::TVoid
TMainFrameWnd::LoadFile(const   TTreeBrowseInfo&    wnotBrowser
                        , const tCIDLib::TBoolean   bEditMode)
{
    //
    //  Get the path and first see if there's a tab already open for this path and
    //  this mode.
    //
    const TString& strPath = wnotBrowser.strPath();
    const TContentTab* pwndTab = pwndFindContTab(strPath, bEditMode);
    if (pwndTab)
    {
        m_pwndEditPane->c4ActivateById(pwndTab->c4TabId());
        return;
    }

    if (bEditMode)
    {
        switch(wnotBrowser.eType())
        {
            case tCQCRemBrws::EDTypes::EMailAccount :
                EditEMailAcct(strPath);
                break;

            case tCQCRemBrws::EDTypes::EvMonitor :
                EditEventMon(strPath);
                break;

            case tCQCRemBrws::EDTypes::GC100Ports:
                EditGC100Ports();
                break;

            case tCQCRemBrws::EDTypes::JAPwrPorts:
                EditJAPwrPorts();
                break;

            case tCQCRemBrws::EDTypes::GlobalAct :
                EditGlobalAction(strPath);
                break;

            case tCQCRemBrws::EDTypes::Image :
                EditImage(strPath);
                break;

            case tCQCRemBrws::EDTypes::Location :
                EditLocation();
                break;

            case tCQCRemBrws::EDTypes::LogicSrv :
                EditLogicSrv();
                break;

            case tCQCRemBrws::EDTypes::Macro :
                EditMacro(strPath);
                break;

            case tCQCRemBrws::EDTypes::SchEvent :
                EditSchedEvent(strPath);
                break;

            case tCQCRemBrws::EDTypes::SystemCfg :
                EditSystemCfg(strPath);
                break;

            case tCQCRemBrws::EDTypes::Template :
                EditTemplate(strPath);
                break;

            case tCQCRemBrws::EDTypes::TrgEvent :
                EditTrigEvent(strPath);
                break;

            case tCQCRemBrws::EDTypes::User :
                EditUserAccount(strPath);
                break;

            default :
            {
                // Not something we can edit
                TErrBox msgbView
                (
                    strWndText()
                    , facCQCAdmin.strMsg
                      (
                        kCQCAMsgs::midStatus_CantEdit
                        , tCQCRemBrws::strLoadEDTypes(wnotBrowser.eType())
                      )
                );
                msgbView.ShowIt(*this);
                break;
            }
        }
    }
     else
    {
        switch(wnotBrowser.eType())
        {
            case tCQCRemBrws::EDTypes::Driver :
                ViewDriver(strPath);
                break;

            case tCQCRemBrws::EDTypes::EvMonitor :
                // There's no viewer for event monitors
                break;

            case tCQCRemBrws::EDTypes::Help :
                ViewHelp(strPath);
                break;

            case tCQCRemBrws::EDTypes::Image :
                ViewImage(strPath);
                break;

            case tCQCRemBrws::EDTypes::Macro :
                ViewMacro(strPath);
                break;

            case tCQCRemBrws::EDTypes::SchEvent :
                // There's no viewer for scheduled events
                break;

            case tCQCRemBrws::EDTypes::System :
                //
                //  This type is used for all system info display stuff. This guy will
                //  look at the path and decide what to do.
                //
                ViewSystemInfo(strPath);
                break;

            case tCQCRemBrws::EDTypes::Template :
                // There's no viewer for templates
                break;

            case tCQCRemBrws::EDTypes::TrgEvent :
                // There's no viewer for triggered events
                break;

            default :
            {
                // Not something we can view
                TErrBox msgbView
                (
                    strWndText()
                    , facCQCAdmin.strMsg
                      (
                        kCQCAMsgs::midStatus_CantView
                        , tCQCRemBrws::strLoadEDTypes(wnotBrowser.eType())
                      )
                );
                msgbView.ShowIt(*this);
                break;
            }
        }
    }
}


tCIDLib::TVoid
TMainFrameWnd::MakeDisplayName(const TString& strFullPath, TString& strToFill)
{
    tCIDLib::TCard4 c4At;
    if (!strFullPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
    {
        facCQCAdmin.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCAErrs::errcGen_BadContPath
            , strFullPath
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    //
    //  If the last character is a slash, which it normally shouldn't be
    //  the we have to go back enother one.
    //
    if (strToFill.chLast() == kCIDLib::chForwardSlash)
    {
        if (!strFullPath.bLastOccurrence(kCIDLib::chForwardSlash, c4At))
        {
            facCQCAdmin.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCAErrs::errcGen_BadContPath
                , strFullPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
    }

    // Copy from the next char forward
    strFullPath.CopyOutSubStr(strToFill, c4At + 1);

    // Get rid of a trailing slash
    if (strToFill.chLast() == kCIDLib::chForwardSlash)
        strToFill.DeleteLast();
}


//
//  See if there is an existing content tab that has the indicated path and is for
//  the indicated edit/view mode.
//
TContentTab*
TMainFrameWnd::pwndFindContTab( const   TString&            strPath
                                , const tCIDLib::TBoolean   bEditMode)
{
    TContentTab* pwndRet = nullptr;

    // Build up the actual window name , which includes the mode
    TString strFind;
    if (bEditMode)
        strFind = kCQCAdmin::strTabPref_Edit;
    else
        strFind = kCQCAdmin::strTabPref_View;
    strFind.Append(strPath);

    const tCIDLib::TCard4 c4Count = m_pwndEditPane->c4TabCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TTabWindow* pwndCur = m_pwndEditPane->pwndTabAt(c4Index);
        if (pwndCur->strName().bCompareI(strFind))
        {
            // It's the right path, see if the right mode
            TContentTab* pwndCont = static_cast<TContentTab*>(pwndCur);
            if (pwndCont->bEditMode() == bEditMode)
            {
                pwndRet = pwndCont;
                break;
            }
        }
    }
    return pwndRet;
}


//
//  This is currently only used for events. We respond to notifications of pause resume
//  operations by the user in the tree browser, so that we can see if we have them open
//  and update our live config.
//
tCIDLib::TVoid TMainFrameWnd::UpdatePauseState(const TTreeBrowseInfo& wnotEvent)
{

    //  For now, there are only edit mode tabs for events, so we only have to
    //  search for that.
    //
    const tCQCRemBrws::EDTypes eDType
    (
        facCQCRemBrws().eXlatPathType(wnotEvent.strPath(), kCIDLib::True)
    );
    const tCQCKit::EEvSrvTypes eEvType
    (
        facCQCEvCl().eBrwsDTypeToEvType(eDType, kCIDLib::False)
    );

    if (eEvType != tCQCKit::EEvSrvTypes::None)
    {
        TContentTab* pwndTab = pwndFindContTab(wnotEvent.strPath(), kCIDLib::True);
        if (pwndTab)
        {
            if (eEvType == tCQCKit::EEvSrvTypes::EvMonitor)
            {
                TEvMonTab* pwndEvMon = static_cast<TEvMonTab*>(pwndTab);
                pwndEvMon->UpdatePauseState(wnotEvent.bFlag());
            }
             else if (eEvType == tCQCKit::EEvSrvTypes::SchEvent)
            {
                TSchedEvTab* pwndSchEv = static_cast<TSchedEvTab*>(pwndTab);
                pwndSchEv->UpdatePauseState(wnotEvent.bFlag());
            }
             else if (eEvType == tCQCKit::EEvSrvTypes::TrgEvent)
            {
                TTrigEvTab* pwndTrigEv = static_cast<TTrigEvTab*>(pwndTab);
                pwndTrigEv->UpdatePauseState(wnotEvent.bFlag());
            }
        }
    }
}


//
//  Called periodically to store any program state info. It is called from our flushing
//  timer if the size/pos changed flag is set, or directly when certain things are done
//  that change some state we want to persist.
//
tCIDLib::TVoid TMainFrameWnd::StoreStateInfo()
{
    try
    {
        // If the main frame window size/pos has changed, store it
        if (m_bSizePosChanged)
        {
            // Clear the flag first before we do anything
            m_bSizePosChanged = kCIDLib::False;

            tCIDLib::TCard4 c4Ver = 0;
            TCQCCAppState   fstNew;

            // Read in the current object, update it, and store it back
            facCQCGKit().bReadStoreObj(kCQCAdmin::pszCfgKey_MainFrame, c4Ver, fstNew);
            fstNew.SetFrom(*this);
            facCQCGKit().c4UpdateStoreObj(kCQCAdmin::pszCfgKey_MainFrame, fstNew);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCAdmin.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }
}

