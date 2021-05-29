//
// FILE NAME: CQCIntfView_MainFrameWnd.cpp
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
//  This file implements the main frame window.
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
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWindow)


// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfView_MainFameWnd
    {
        // -----------------------------------------------------------------------
        //  A code we post to ourself when we get a special action request from
        //  our client interface window and he want's us to do it async. We just
        //  post this to ourself and turn around call the method without the
        //  async request.
        //
        //  And another for a special blanker dismiss command
        //
        //  And another for extra async redrawing, which we do when forcing full screen
        //  mode. We want to make sure we've gotten ourself all straight and then
        //  force a redraw.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TInt4        i4AsyncSpecAct  = 3013;
        constexpr tCIDLib::TInt4        i4AsyncDisBlank = 3014;
        constexpr tCIDLib::TInt4        i4AsyncCmd      = 3015;
        constexpr tCIDLib::TInt4        i4AsyncRedraw   = 3016;


        // -----------------------------------------------------------------------
        //  To support a generic async callback. The IV engine can pass us some
        //  data, which we will turn around and call back on the provided view
        //  object with.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4       c4AsyncData     = 3020;


        // -----------------------------------------------------------------------
        //  The window id we give to our client window (the interface display
        //  window.)
        // -----------------------------------------------------------------------
        constexpr tCIDCtrls::TWndId     widClient = 1;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_bCachingEnabled(kCIDLib::True)
    , m_bInitShow(kCIDLib::False)
    , m_bStateChanged(kCIDLib::False)
    , m_c4StateId(0)
    , m_colEventQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_eLastState(tCIDCtrls::EPosStates::Count)
    , m_piwdgCmdParent(nullptr)
    , m_widNextClientId(1)
    , m_pwndClient(nullptr)
    , m_wndBlanker(facCQCIntfView.cuctxToUse())
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TMainFrameWnd::AsyncCallback(   const   tCQCIntfEng::EAsyncCmds eCmd
                                ,       TCQCIntfView&           )
{
    // Post it back to ourself
    SendCode
    (
        CQCIntfView_MainFameWnd::i4AsyncCmd
        , tCIDLib::TCard4(eCmd)
        , tCIDLib::ESyncTypes::Asynchronous
    );
}


//
//  Pass the object along to be passed back in to the calling view asychronously.
//
tCIDLib::TVoid TMainFrameWnd::AsyncDataCallback(TCQCIntfADCB* const padcbInfo)
{
    SendData
    (
        CQCIntfView_MainFameWnd::c4AsyncData
        , padcbInfo
        , tCIDLib::ESyncTypes::Asynchronous
    );
}


tCIDLib::TBoolean
TMainFrameWnd::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    tCIDLib::TBoolean bHandled = TParent::bProcessAccel(pMsgData);
    return bHandled;
}


//
//  We implement a mixin defined by the interface engine that allows it to pass
//  special commands up to the contining host. We register ourselves with the
//  engine via this interface and it will call us here when the user asks to
//  dismiss the blanker, which only we can do.
//
tCIDLib::TVoid TMainFrameWnd::DismissBlanker(TCQCIntfView&)
{
    //
    //  We need to do this asynchronously. Sometimes it might be ok to do it
    //  directly, but it's not worth worrying about.
    //
    SendCode(CQCIntfView_MainFameWnd::i4AsyncDisBlank, 0);
}


//
//  This is the method that we inherit from the interface engine's app level
//  handler. We implement this mixin and register ourself with the engine. He
//  calls us on it whenever he needs to post commands or have us do a special
//  action.
//
tCIDLib::TVoid
TMainFrameWnd::DoSpecialAction( const   tCQCIntfEng::ESpecActs  eAct
                                , const tCIDLib::TBoolean       bAsync
                                ,       TCQCIntfView&           civSender)
{
    //
    //  They can request that we do this asynchronously, to avoid many
    //  complications when this is invoked from an action. In that case
    //  we just post an event to ourself. The handler will turn around
    //  and call us here again, without the async flag
    //
    if (bAsync)
    {
        SendCode
        (
            CQCIntfView_MainFameWnd::i4AsyncSpecAct
            , tCIDLib::TCard4(eAct)
            , tCIDLib::ESyncTypes::Asynchronous
        );
        return;
    }

    if ((eAct == tCQCIntfEng::ESpecActs::Blanker)
    ||  (eAct == tCQCIntfEng::ESpecActs::ClockBlanker)
    ||  (eAct == tCQCIntfEng::ESpecActs::ShowBlanker))
    {
        // Invoke the blanker window if we are the active process
        if (bIsActive())
        {
            // Translate to the blanker mode type
            tCQCIGKit::EBlankModes eMode = tCQCIGKit::EBlankModes::Normal;
            if (eAct == tCQCIntfEng::ESpecActs::ClockBlanker)
                eMode = tCQCIGKit::EBlankModes::Clock;
            else if (eAct == tCQCIntfEng::ESpecActs::ShowBlanker)
                eMode = tCQCIGKit::EBlankModes::SlideShow;
            InvokeBlanker(kCIDLib::False, eMode);
        }
    }
     else if (eAct == tCQCIntfEng::ESpecActs::Exit)
    {
        //
        //  In this case, we assume that the caller has already confirmed
        //  the shutdown action, so we just do it.
        //
        facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if (eAct == tCQCIntfEng::ESpecActs::ExitFS)
    {
        // Exit full screen mode if we are in it
        if (facCQCIntfView.bFullScreen())
        {
            EndFSMode();

            // Update some menu items

        }
    }
     else if (eAct == tCQCIntfEng::ESpecActs::FSMode)
    {
        // Enter full screen mode if not already
        if (!facCQCIntfView.bFullScreen())
            StartFSMode();
    }
     else if (eAct == tCQCIntfEng::ESpecActs::DisplayOff)
    {
        // Turn off the monitor power.
        facCIDCtrls().SetMonitorPowerState(*this, tCIDCtrls::EMonPowerSt::Off);
    }
     else if (eAct == tCQCIntfEng::ESpecActs::BlankAndBlanker)
    {
        // We both turn off the monitor power and invoke the blanker in normal mode
        if (bIsActive())
            InvokeBlanker(kCIDLib::True, tCQCIGKit::EBlankModes::Normal);
    }
}


//
//  This is called by the interface engine when a new template is loaded
//  via an internal link operation inside the viewer window
//
tCIDLib::TVoid
TMainFrameWnd::NewTemplate( const   TCQCIntfTemplate&   iwdgNew
                            ,       TCQCIntfView&       civSender
                            , const tCIDLib::TCard4     c4StackPos)
{
    // Just update our title bar with the new name if it's the base template
    if (c4StackPos == 0)
    {
        TString strTitle
        (
            kIViewMsgs::midWnd_AppName, facCQCIntfView, iwdgNew.strTemplateName()
        );
        strWndText(strTitle);
    }
}


// The engine will call this to get the action tracer we want him to use
MCQCCmdTracer* TMainFrameWnd::pmcmdtTrace()
{
    // If our tracer window is active, then return it
    if (m_wndActTrace.bIsValid())
        return &m_wndActTrace;
    return 0;
}


//
//  Report back some possible application flags that the IV engine needs to know
//  about so that it can adjust for them. WE currently only could be in signage
//  mode, but may have to support other later.
//
tCQCIntfEng::EAppFlags TMainFrameWnd::eAppFlags() const
{
    tCQCIntfEng::EAppFlags eRet = tCQCIntfEng::EAppFlags::None;

    if (facCQCIntfView.bSignageMode())
        eRet = tCIDLib::eOREnumBits(eRet, tCQCIntfEng::EAppFlags::Signage);

    return eRet;
}



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMainFrameWnd::AddCtrlEvent(TIntfCtrlEv* const piceToAdopt)
{
    if (facCQCIntfView.bLogInfo())
    {
        facCQCIntfView.LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Adding remote ctrl cmd (type=%(1)) to the input queue"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , piceToAdopt->clsIsA()
        );
    }

    // Just in case, start ignoring them if the queue is big
    if (m_colEventQ.bIsFull(64))
        m_colEventQ.Add(piceToAdopt);
    else
        delete piceToAdopt;
}


tCIDLib::TBoolean
TMainFrameWnd::bLoadTemplate(const  TString&            strName
                            , const tCIDLib::TBoolean   bSupressWarn)
{
    try
    {
        // Download the selected one and stream it in
        TDataSrvClient dsclLoad;
        TCQCIntfTemplate iwdgTmp;
        facCQCIntfEng().QueryTemplate
        (
            strName
            , dsclLoad
            , iwdgTmp
            , facCQCIntfView.cuctxToUse()
            , m_bCachingEnabled
        );

        //
        //  Watch for the special one that is just our logo on a white
        //  screen. The IV will use this when in signage mode and there's
        //  no predefined template in the user account. If it's that one,
        //  then adjust the size to fit our client window area.
        //
        if (strName == L"/System/CQC/Logo")
        {
            TArea areaNew;

            if (m_pwndClient)
                areaNew = m_pwndClient->areaWnd();
            else
                QueryClientArea(areaNew, kCIDLib::True);
            areaNew.ZeroOrg();

            //
            //  Tell it not to report this to child widgets. We are doing this before
            //  the template is loaded into the window, so the view won't be set yet.
            //
            iwdgTmp.SetArea(areaNew, kCIDLib::False);
        }

        //
        //  If no current client window, then create one. Else, we just reload into the
        //  existing one. If we are in signage mode, then we always create a new window,
        //  so that we can recover from a failed load by just keeping the old one.
        //
        if (m_pwndClient && !facCQCIntfView.bSignageMode())
        {
            tCQCIntfEng::TErrList colErrs;
            m_pwndClient->civTarget().bLoadNewTemplate(dsclLoad, iwdgTmp, colErrs);

            // If there were validation type errors, show them unless suppressed
            if (!colErrs.bIsEmpty() && !bSupressWarn)
                facCQCIntfWEng().ShowErrors(*this, 0, colErrs, iwdgTmp.strTemplateName());
        }
         else
        {
            m_widNextClientId++;
            TCQCIntfViewWnd* pwndNew = facCQCIntfWEng().pwndMakeNewInterface
            (
                *this
                , iwdgTmp
                , m_widNextClientId
                , dsclLoad
                , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                , this
                , kCIDLib::True
                , facCQCIntfView.cuctxToUse()
            );

            // If it worked, then let's keep this guy
            if (pwndNew)
            {
                m_pwndClient = pwndNew;
                pwndNew->TakeFocus();

                // Tell him to propagate keys
                m_pwndClient->bPropagateKeys(kCIDLib::True);

                // Make sure it's synced to our current template caching selection
                m_pwndClient->civTarget().bEnableTmplCaching(m_bCachingEnabled);
            }
        }

        // It worked, so set it on the persistent state
        m_fstCur.strLastTmpl(strName);

        // Update the title bar
        TString strTitle
        (
            kIViewMsgs::midWnd_AppName
            , facCQCIntfView
            , iwdgTmp.strTemplateName()
        );
        strWndText(strTitle);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfView.bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        //
        //  If in signage mode, don't display anything since that will prevent
        //  further loading of interfaces.
        //
        if (!facCQCIntfView.bSignageMode())
        {
            facCQCGKit().ShowError
            (
                *this
                , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                , facCQCIntfEng().strMsg(kIEngMsgs::midStatus_CantOpenTmpl, strName)
                , errToCatch
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called by our startup code in the main cpp file to create the
//  main frame window.
//
tCIDLib::TVoid
TMainFrameWnd::Create(  const   TString&            strText
                        , const TCQCIntfViewState&  fstInit)
{
    //
    //  Store the initial state, but clear the initial template. The
    //  problem is that if the initial load fails after we are created,
    //  we'll have the bad previous template referenced and will store
    //  it back again. The facility class will load the initial template
    //  and we'll store it as the last one accessed at that point.
    //
    m_fstCur = fstInit;
    m_fstCur.ClearLastTmpl();

    // And call our parent to create the window
    TParent::CreateFrame
    (
        nullptr
        , m_fstCur.areaFrame()
        , strText
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );

    //
    //  Restore our saved position state, though using our new current area as our restore
    //  area. If we were maximized this will get us back again and set our current size as
    //  the restore size.
    //
    RestorePosState(m_fstCur.ePosState(), areaWnd(), kCIDLib::False);

    // If we are starting full screen, then do that now
    if (facCQCIntfView.bFullScreen())
        StartFSMode();

    // Update the state info with the actual area we ended up at
    m_fstCur.areaFrame(areaWnd());

    // Make sure we have the focus
    TakeFocus();
}


//
//  This are called (indirectly) by our timer handler below. If it sees any
//  remote control events coming in from our external control server, it
//  will invoke that queued event. Some of those event handlers will call
//  us back on these methods.
//
tCIDLib::TVoid
TMainFrameWnd::DoRemLoadOp( const   tCIDLib::TBoolean   bOverlay
                            , const TString&            strOvrName
                            , const TString&            strToLoad)
{
    //
    //  They want to load a specific template either as a new top level
    //  template or into a named top level overlay. We just pass this on to
    //  the viewer window.
    //
    if (bOverlay)
    {
        //
        //  This one we can only do if there is already a client loaded. And
        //  it still might not do anything if he cannot find such an overlay
        //  or template.
        //
        if (m_pwndClient)
        {
            tCQCIntfEng::TErrList colErrs;
            if (!m_pwndClient->civTarget().bLoadOverlay(strOvrName, strToLoad, colErrs))
            {
                //
                //  Display the error window to let the user view or edit the
                //  errors.
                //
                TCQCValErrDlg dlgErr;
                dlgErr.RunDlg(*this, 0, colErrs, strToLoad);
            }
        }
    }
     else
    {
        //
        //  Just call our own local method that does the work, since we have
        //  to deal with all the usual issues of creating a client window if
        //  we don't have one yet, else creating a new one.
        //
        bLoadTemplate(strToLoad, kCIDLib::True);
    }
}

tCIDLib::TVoid TMainFrameWnd::DoRemMiscOp(const tCQCKit::EIVMiscOps eOp)
{
    //
    //  Do the requested miscellaneous remote op, all of which ar pretty
    //  simple.
    //
    switch (eOp)
    {
        case tCQCKit::EIVMiscOps::Blanker :
            InvokeBlanker(kCIDLib::False, tCQCIGKit::EBlankModes::Normal);
            break;

        case tCQCKit::EIVMiscOps::BlankDisplay :
            facCIDCtrls().SetMonitorPowerState(*this, tCIDCtrls::EMonPowerSt::Off);
            break;

        case tCQCKit::EIVMiscOps::BlankAndBlanker :
            InvokeBlanker(kCIDLib::True, tCQCIGKit::EBlankModes::Normal);
            break;

        case tCQCKit::EIVMiscOps::ClockBlanker :
            InvokeBlanker(kCIDLib::False, tCQCIGKit::EBlankModes::Clock);
            break;

        case tCQCKit::EIVMiscOps::ShowBlanker :
            InvokeBlanker(kCIDLib::False, tCQCIGKit::EBlankModes::SlideShow);
            break;

        case tCQCKit::EIVMiscOps::Unblank :
            CloseBlanker();
            break;
    }
}


//
//  Removes us from full screen mode, upon user request either via key combo,
//  menu selection, or remotely via the control interface.
//
tCIDLib::TVoid TMainFrameWnd::EndFSMode()
{
    //
    //  If we are in kiosk mode, and the current user is less than a power
    //  user, then put up the logon dialog and make them provide a system
    //  admin account.
    //
    if (facCQCIntfView.bKioskMode()
    &&  (facCQCIntfView.eUserRole() < tCQCKit::EUserRoles::PowerUser))
    {
        TCQCSecToken    sectToFill;
        TCQCUserAccount uaccToFill;

        //
        //  Suppress environmental logon because we want to insure this person,
        //  is a system admin. We set Limited as the minimum role, so that we can
        //  display the dialog if it's not sufficient privs.
        //
        if (!facCQCGKit().bLogon(*this
                                , sectToFill
                                , uaccToFill
                                , tCQCKit::EUserRoles::LimitedUser
                                , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                                , kCIDLib::True
                                , L"IntfViewer"))
        {
            return;
        }

        // See if the account is sufficient. If not, tell them no go
        if (uaccToFill.eRole() < tCQCKit::EUserRoles::PowerUser)
        {
            TErrBox msgbWarn
            (
                facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                , facCQCKit().strMsg(kKitMsgs::midStatus_MustBePowerUser)
            );
            msgbWarn.ShowIt(*this);
            return;
        }
    }

    //
    //  If we are in kiosk mode, then we hide the cursor when we went full
    //  screen, so show it again now.
    //
    if (facCQCIntfView.bKioskMode())
        facCIDCtrls().ShowPointer();

    ExitFullScreenMode();

    //
    //  If we have an interface window, then tell him to reposition his
    //  template since the size may have changed.
    //
    ResizeView();
}


// Invokes the blanker in the indicated mode
tCIDLib::TVoid
TMainFrameWnd::InvokeBlanker(const  tCIDLib::TBoolean       bPowerOff
                            , const tCQCIGKit::EBlankModes  eMode)
{
    if (!m_wndBlanker.bIsValid())
    {
        m_wndBlanker.eMode(eMode);
        m_wndBlanker.Create(*this, strWndText());
        m_wndBlanker.TakeFocus();

        // Allow any messages to be processed
        facCIDCtrls().MsgYield(100);
    }

    if (bPowerOff)
        facCIDCtrls().SetMonitorPowerState(*this, tCIDCtrls::EMonPowerSt::Off);
}


//
//  Called from the remote control protocol interface to allow clients to
//  directly update the value of a global variable.
//
tCIDLib::TVoid
TMainFrameWnd::SetVariable( const   TString&    strVarName
                            , const TString&    strValue)
{
    m_pwndClient->civTarget().SetVariable(strVarName, strValue);
}



// Takes us into full screen mode, upon user request
tCIDLib::TVoid TMainFrameWnd::StartFSMode()
{
    // And now go into full screen mode
    FullScreenMode();

    // Tell the client to re-center the template
    ResizeView();

    // If we are in kiosk mode, then hide the pointer
    if (facCQCIntfView.bKioskMode())
        facCIDCtrls().HidePointer();

    // Do an extra async redraw
    SendCode
    (
        CQCIntfView_MainFameWnd::i4AsyncRedraw, 0, tCIDLib::ESyncTypes::Asynchronous
    );
}



// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMainFrameWnd::ActivationChange(const   tCIDLib::TBoolean       bNewState
                                , const tCIDCtrls::TWndHandle   hwndOther)
{
    // Call our parent first
    TParent::ActivationChange(bNewState, hwndOther);

    const tCIDLib::TBoolean bPtrVisible(facCIDCtrls().bIsPointerVisible());
    if (bNewState
    ||  (hwndOther && facCIDCtrls().bIsOwnedBy(hwndOther, hwndThis(), kCIDLib::False)))
    {
        //
        //  If we are in kiosk mode, and we have no title bar (which means we are in full
        //  screen mode), and the pointer is visible, then hide it.
        //
        if (facCQCIntfView.bKioskMode()
        &&  tCIDLib::bAllBitsOn(eWndStyles(), tCIDCtrls::EWndStyles::Caption)
        &&  bPtrVisible)
        {
            facCIDCtrls().HidePointer();
        }

        //
        //  Update the activity time stamp on our client, which will pass it to the
        //  view it owns. This insures that, when the blanker is dismissed and we
        //  get reactivated, we'll update the stamp, so we won't just get reblanked
        //  again.
        //
        if (m_pwndClient)
            m_pwndClient->UpdateLastActivityStamp();

        //
        //  We need to make ourself the active top window if getting activated, since we
        //  have multiple top level windows if the action trace or blanker is up.
        //
        if (bNewState)
            facCIDCtrls().SetActiveTopWnd(this);
    }
     else
    {
        //
        //  Someone else won, and it's not owned by us. If it's not our blanker that has
        //  become activated, then show the pointer if not already visible.
        //
        if (hwndOther
        &&  m_wndBlanker.bIsValid()
        &&  (m_wndBlanker.hwndThis() != hwndOther)
        &&  !bPtrVisible)
        {
            facCIDCtrls().ShowPointer();
        }
    }
}


//
//  Handle a size or move of the frame window. We need to get our interface
//  window resized/positioned accordingly.
//
tCIDLib::TVoid
TMainFrameWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Pass on to our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    //
    //  If the size changed and we are maximized or restored, then we need to
    //  update the client window for our new size.
    //
    if (ePosState != tCIDCtrls::EPosStates::Minimized)
    {
        if (facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
        {
            facCQCIntfEng().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"IV frame window got area change"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        if (bSizeChanged)
            ResizeView();

        //
        //  This is a special case issue becase the web browser widget manages an external
        //  process that runs each browser instance. So we have to report any change that
        //  would require the repositioning of those external windows (and the clipping of
        //  them as well, to keep them within the view window.)
        //
        if ((bOrgChanged || bSizeChanged || bStateChanged) && m_pwndClient)
        {
            if (facCQCIntfEng().eLogThreshold() <= tCIDLib::ESeverities::Warn)
            {
                facCQCIntfEng().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Passing the change on to the IV window"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            m_pwndClient->ScrPosChanged();
        }
    }

    // And let the timer know there is new data
    m_eLastState = ePosState;
    m_bStateChanged = kCIDLib::True;
}


// We have to deal with taking focus and ending FS mode if double clicked
tCIDLib::TBoolean
TMainFrameWnd::bClick(  const   tCIDCtrls::EMouseButts  eButton
                        , const tCIDCtrls::EMouseClicks eClickType
                        , const TPoint&                 pntAt
                        , const tCIDLib::TBoolean       bCtrlShift
                        , const tCIDLib::TBoolean       bShift)
{
    // Take the focus on a down click
    if (eClickType == tCIDCtrls::EMouseClicks::Down)
        TakeFocus();

    //
    //  If we get a double click, and we are in full screen mode, then back
    //  out of it back to restored mode.
    //
    if ((eClickType == tCIDCtrls::EMouseClicks::Double)
    &&  facCQCIntfView.bFullScreen())
    {
        EndFSMode();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    TParent::bCreated();

    // Initially we set the template name in the title bar to empty
    TString strTitle(kIViewMsgs::midWnd_AppName, facCQCIntfView, TString::strEmpty());
    strWndText(strTitle);

    // Set our icon
    facCQCIntfView.SetFrameIcon(*this, L"IntfViewer");

    // Load up our resource based main menu
    LoadMenu();

    //
    //  Enable the persistent state flushing timer. Let it go off every 5 seconds. Enable
    //  one for handling queued control protocol events. It needs to happen quickly. It
    //  lso handles checking for incoming events. And, via a one shot flag, is used to do
    //  the initial display of our window.
    //
    m_tmidFlush = tmidStartTimer(5000);
    m_tmidCtrl = tmidStartTimer(100);

    // Set the min/max size we'll allow ourself to be sized to
    SetMinMaxSize(TSize(320, 240), TGUIFacility::szPrimaryMonitor());

    return kCIDLib::True;
}


// Watch for key-mapped actions we need to invoke
tCIDLib::TBoolean
TMainFrameWnd::bExtKey( const   tCIDCtrls::EExtKeys eExtKey
                        , const tCIDLib::TBoolean   bAltShift
                        , const tCIDLib::TBoolean   bCtrlShift
                        , const tCIDLib::TBoolean   bShift
                        , const TWindow* const      )
{
    //
    //  Handle the special case scenario of the Ctrl-Shift-F1 key sequence. We
    //  don't do this via the menu/accel table because it only has any meaning
    //  when we are in full screen mode (and therefore there's no menu.)
    //
    if (!bAltShift && bCtrlShift && bShift && (eExtKey == tCIDCtrls::EExtKeys::F1))
    {
        // If we aren't in FS mode, this will just not do anything
        EndFSMode();
        return kCIDLib::True;
    }

    //
    //  If its one of the mappable keys, then lets invoke the mapped macro
    //  or field write.
    //
    tCIDLib::TBoolean bInvoked = kCIDLib::False;
    if (TCQCKeyMap::bIsLegalKey(eExtKey))
    {
        //
        //  Its a candidate, so pass it on, and say we handled it if it was
        //  actually mapped. Only pass in the action trace if it currently
        //  is created.
        //
        TActTraceJan janTrace(m_wndActTrace.bIsValid() ? &m_wndActTrace : 0);
        bInvoked = facCQCIGKit().bInvokeKeyMap
        (
            eExtKey
            , *this
            , *m_pwndClient
            , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
            , bAltShift
            , bCtrlShift
            , bShift
            , facCQCIntfView.cuctxToUse()
            , janTrace.pmcmdtTrace()
        );
    }

    // If not invoked, then see if it's one of the common hot keys we want to honor
    if (bAltShift && !bCtrlShift && !bShift && (eExtKey == tCIDCtrls::EExtKeys::F4))
    {
        // It's an exit key
        facCIDCtrls().ExitLoop(0);
    }

    return bInvoked;
}


// Watch for key-mapped actions we need to invoke
tCIDLib::TBoolean
TMainFrameWnd::bMediaKey(const tCIDCtrls::EExtKeys  eExtKey
                        , const tCIDLib::TBoolean   bAltShift
                        , const tCIDLib::TBoolean   bCtrlShift
                        , const tCIDLib::TBoolean   bShift
                        , const TWindow* const      )
{
    //
    //  Its a candidate, so pass it on to the facility object for processing.
    //  If he doesn't process it, then let it propogate, because these keys
    //  get passed to a system wide hook if not handled, and its important
    //  to let any propogate that we weren't told to map.
    //
    TActTraceJan janTrace(m_wndActTrace.bIsValid() ? &m_wndActTrace : 0);
    return facCQCIGKit().bInvokeKeyMap
    (
        eExtKey
        , *this
        , *m_pwndClient
        , strWndText()
        , bAltShift
        , bCtrlShift
        , bShift
        , facCQCIntfView.cuctxToUse()
        , janTrace.pmcmdtTrace()
    );
}


//
//  If we are in full screen mode, then we need to flip back before we
//  allow the minimize to go forward.
//
tCIDLib::TBoolean TMainFrameWnd::bMinimizing()
{
    if (facCQCIntfView.bFullScreen())
        EndFSMode();

    // And tell him to go ahead and do the restore now
    return kCIDLib::True;
}



tCIDLib::TVoid
TMainFrameWnd::CodeReceived(const   tCIDLib::TInt4  i4Code
                            , const tCIDLib::TCard4 c4Data)
{
    if (i4Code == CQCIntfView_MainFameWnd::i4AsyncSpecAct)
    {
        //
        //  We were asked to do an async special action. So we posted to
        //  ourself from DoSpecialAction(). Now we just turn around can call
        //  it again without the async request. It passed us the action enum
        //  value as the data, which we have to cast back.
        //
        DoSpecialAction
        (
            tCQCIntfEng::ESpecActs(c4Data), kCIDLib::False, m_pwndClient->civTarget()
        );
    }
     else if (i4Code == CQCIntfView_MainFameWnd::i4AsyncDisBlank)
    {
        // If the blanker window is valid, destroy it
        CloseBlanker();
    }
     else if (i4Code == CQCIntfView_MainFameWnd::i4AsyncCmd)
    {
        // Just call our view back with the posted command
        if (m_pwndClient)
        {
            m_pwndClient->civTarget().AsyncCmdCallback
            (
                tCQCIntfEng::EAsyncCmds(c4Data)
            );
        }
    }
     else if (i4Code == CQCIntfView_MainFameWnd::i4AsyncRedraw)
    {
        // We just force an async repaint, to get a doubly async redraw
        ForceRepaint();
    }
     else
    {
        TParent::CodeReceived(i4Code, c4Data);
    }
}



//
//  Handle data passing async callbacks that we do on behalf of the interface view
//  we installed ourself on as the app handler for. It sometimes needs us to do things
//  in an async way, so that it can avoid issues with things being destroyed while
//  they are being processed.
//
tCIDLib::TVoid
TMainFrameWnd::DataReceived(const   tCIDLib::TCard4         c4DataId
                            ,       tCIDLib::TVoid* const   pRawData)
{
    //
    //  Can't allow any errors to propogate here, since we are in the GUI thread
    //  context.
    //
    try
    {
        // Just pass the data back to the calling view
        TCQCIntfADCB* padcbInfo = reinterpret_cast<TCQCIntfADCB*>(pRawData);

        // Make sure it gets cleaned up after the caller returns
        TJanitor<TCQCIntfADCB> janData(padcbInfo);
        padcbInfo->m_pcivCaller->AsyncDataCallback(padcbInfo);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    catch(...)
    {
    }
}


// Handle the destroyed callback so that we can clean up and store state
tCIDLib::TVoid TMainFrameWnd::Destroyed()
{
    // Stop our timers
    if (m_tmidFlush != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidFlush);
        m_tmidFlush = kCIDCtrls::tmidInvalid;
    }

    if (m_tmidCtrl != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidCtrl);
        m_tmidCtrl = kCIDCtrls::tmidInvalid;
    }

    // Do a last save of any state changes if not in signage mode
    if (!facCQCIntfView.bSignageMode())
    {
        try
        {
            TArea areaSave;
            m_fstCur.SetFrom(*this);
            tCIDLib::TCard4 c4Version = 0;
            facCQCGKit().bAddUpdateStoreObj
            (
                kCQCIntfView::pszCfgKey_MainFrame, c4Version, m_fstCur, 64
            );
        }

        catch(const TError& errToCatch)
        {
            if (facCQCIntfView.bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }
    }

    // Destroy the action trace window if it's active
    if (m_wndActTrace.bIsValid())
        m_wndActTrace.Destroy();

    //
    //  And pass on to our parent. He will clean up the client and menu bar
    //  and other child windows.
    //
    TParent::Destroyed();

    // We cannot any longer be the active top level window
    if (this == facCIDCtrls().pwndActiveTopWnd())
        facCIDCtrls().SetActiveTopWnd(nullptr);
}


//
//  If the display resolution changes, we have to inform the template window about this.
//  And, if we are in full screen mode, we have to adjust our position to stay full
//  screen.
//
tCIDLib::TVoid
TMainFrameWnd::DisplayChanged(  const   tCIDLib::TCard4 c4NewBitsPerPel
                                , const TSize&          szNew)
{
    if (facCQCIntfView.bFullScreen())
    {
        // Get our new monitor area. It will reflect the orientation
        TArea areaNew;
        facCIDCtrls().QueryMonArea(*this, areaNew, kCIDLib::False);

        //
        //  We have to flip the min/max size settings as well or we couldn't size up
        //  in the new larger direction.
        //
        if (szNew.c4Width() > szNew.c4Height())
            SetMinMaxSize(TSize(240, 320), szNew);
        else
            SetMinMaxSize(TSize(320, 240), szNew);

        SetSizePos(areaNew, kCIDLib::True);
    }

    // If we have a client, tell him about this
    ResizeView();
}


tCIDLib::TVoid TMainFrameWnd::DragEnd()
{
    //
    //  Tell the interface window it might have a new size now. To keep the
    //  overhead down, we don't do this during size change events.
    //
    ResizeView();
}


tCIDLib::TVoid
TMainFrameWnd::MenuCommand( const   tCIDLib::TResId     ridItem
                            , const tCIDLib::TBoolean   bChecked
                            , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == kCQCIntfView::ridMenu_File_About)
    {
        facCQCIGKit().AboutDlg(*this);
    }
     else if (ridItem == kCQCIntfView::ridMenu_File_Open)
    {
        SelectTemplate();
    }
     else if (ridItem == kCQCIntfView::ridMenu_File_Exit)
    {
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if (ridItem == kCQCIntfView::ridMenu_File_ChangePW)
    {
        facCQCGKit().ChangePassword
        (
            *this, facCQCIntfView.strUserName(), facCQCIntfView.sectUser()
        );
    }
     else if ((ridItem == kCQCIntfView::ridMenu_File_Reload)
          ||  (ridItem == kCQCIntfView::ridMenu_File_ReloadReset))
    {
        // If we have a current template, then reload it
        if (m_pwndClient)
        {
            // If a reload/reset, then clear the global vars cache first
            if (ridItem == kCQCIntfView::ridMenu_File_ReloadReset)
                m_pwndClient->civTarget().ctarGlobalVars().DeleteAllVars();

            //
            //  Make sure to COPY the text, because we are going to kill the
            //  template it's in.
            //
            TString strCur(m_pwndClient->civTarget().strCurTmplName());
            bLoadTemplate(strCur, kCIDLib::False);
        }
    }
     else if (ridItem == kCQCIntfView::ridMenu_Tools_EnableCaching)
    {
        //
        //  Flip the current setting, and update the menu item. If the window exists
        //  then update it as well.
        //
        m_bCachingEnabled = !m_bCachingEnabled;
        menuCur().SetItemCheck(kCQCIntfView::ridMenu_Tools_EnableCaching, m_bCachingEnabled);
        if (m_pwndClient)
            m_pwndClient->civTarget().bEnableTmplCaching(m_bCachingEnabled);
    }
     else if (ridItem == kCQCIntfView::ridMenu_Tools_FlushTTSQ)
    {
        facCQCIntfView.FlushTTSQ();
        TOkBox msgbFlushed
        (
            facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
            , facCQCIntfView.strMsg(kIViewMsgs::midStatus_TTSQFlushed)
        );
        msgbFlushed.ShowIt(*this);
    }
     else if ((ridItem == kCQCIntfView::ridMenu_Tools_SnapJPEG)
          ||  (ridItem == kCQCIntfView::ridMenu_Tools_SnapPNG))
    {
        DoScreenCapture(ridItem == kCQCIntfView::ridMenu_Tools_SnapPNG);
    }
     else if (ridItem == kCQCIntfView::ridMenu_View_FullScreen)
    {
        // Remember we are in full screen mode, and invoke it
        StartFSMode();
    }
     else if (ridItem == kCQCIntfView::ridMenu_View_ShowFlds)
    {
        TShowFldsDlg dlgShowFlds;
        dlgShowFlds.RunDlg(*this);
    }
     else if (ridItem == kCQCIntfView::ridMenu_View_ActionTrace)
    {
        //
        //  If the action trace window is not created, create it. If it
        //  is, bring it forward.
        //
        if (!m_wndActTrace.bIsValid())
            m_wndActTrace.CreateWindow(L"IntfViewer");
        else
            m_wndActTrace.ToTop(kCIDLib::True);
    }
}


tCIDLib::TVoid TMainFrameWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo == m_tmidFlush)
    {
        //
        //  Just see if the persistent state has changed and we aren't moving
        //  or sizing. If so, then flush it out, if not in signage mode.
        //
        if (m_bStateChanged && !bInDragMode()
        &&  !facCQCIntfView.bFullScreen()
        &&  !facCQCIntfView.bSignageMode())
        {
            try
            {
                m_fstCur.SetFrom(*this);
                tCIDLib::TCard4 c4Ver;
                facCQCGKit().bAddUpdateStoreObj
                (
                    kCQCIntfView::pszCfgKey_MainFrame, c4Ver, m_fstCur, 1024
                );
                m_bStateChanged = kCIDLib::False;
            }

            catch(const TError& errToCatch)
            {
                if (facCQCIntfView.bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCIntfView.bLogWarnings())
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcApp_StoreState
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }

            catch(...)
            {
                if (facCQCIntfView.bLogWarnings())
                {
                    facCQCKit().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kKitErrs::errcApp_StoreState
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }
        }
    }
     else if (tmidToDo == m_tmidCtrl)
    {
        if (!m_bInitShow)
        {
            m_bInitShow = kCIDLib::True;
            facCIDCtrls().ShowGUI(*this);
        }

        //
        //  If there is an event in our control protocol event queue, then
        //  process that if we can.
        //
        TIntfCtrlEv* piceCur;
        if (!m_colEventQ.bIsEmpty())
        {
            while (kCIDLib::True)
            {
                // Wait for the next event for just a little bit
                piceCur = m_colEventQ.pobjGetNext(5, kCIDLib::False);

                // If no more, then break out
                if (!piceCur)
                    break;

                // If no client window, just eat it
                if (!m_pwndClient)
                    continue;

                try
                {
                    // Put a janitor on it to make sure it's cleaned up
                    TJanitor<TIntfCtrlEv> janEv(piceCur);

                    // if the blanker is up, then toss it
                    CloseBlanker();

                    //
                    //  If there is any owned popup active or if we are
                    //  currently processing commands or are currently
                    //  tracking the mouse, then we will ignore this.
                    //
                    if (!bAnyActivePopup(kCIDLib::True)
                    &&  !m_pwndClient->civTarget().bIsBusy()
                    &&  !m_pwndClient->bHasMouseCapture())
                    {
                        piceCur->ProcessEv(*this);
                    }
                     else if (facCQCIntfView.bLogInfo())
                    {
                        // If verbose, log why we ignored it
                        const tCIDLib::TCh* pszMsg = L" of unknown reason";
                        if (bAnyActivePopup(kCIDLib::True))
                            pszMsg = L" a popup is active";
                        else if (m_pwndClient->civTarget().bIsBusy())
                            pszMsg = L" the view is busy";
                        else if (m_pwndClient->bHasMouseCapture())
                            pszMsg = L" the mouse is captured";

                        if (facCQCIntfView.bLogInfo())
                        {
                            facCQCIntfView.LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"The incoming remote ctrl cmd (type=%(1)) was ignored because %(2)"
                                , tCIDLib::ESeverities::Info
                                , tCIDLib::EErrClasses::AppStatus
                                , piceCur->clsIsA()
                                , TString(pszMsg)
                            );
                        }
                    }
                }

                catch(const TError& errToCatch)
                {
                    if (facCQCIntfView.bShouldLog(errToCatch))
                        TModule::LogEventObj(errToCatch);
                }

                catch(...)
                {
                    // <TBD>
                }
            }
        }

        // Update our current status on the external control object
        if (m_pwndClient)
        {
            TString strBaseTmpl;
            TString strTopTmpl;
            tCIDLib::TCard4 c4LayerCnt = m_pwndClient->civTarget().c4QueryStatus
            (
                strBaseTmpl, strTopTmpl
            );
            facCQCIntfView.StoreStatus(strBaseTmpl, strTopTmpl, c4LayerCnt);
        }
         else
        {
            facCQCIntfView.StoreStatus(TString::strEmpty(), TString::strEmpty(), 0);
        }
    }
     else
    {
        // Not one of ours, so pass it on
        TParent::Timer(tmidToDo);
    }
}


//
//  Helper to invoke and dismiss the blanker window and make sure the monitor
//  powered back up.
//
tCIDLib::TVoid TMainFrameWnd::CloseBlanker()
{
    if (m_wndBlanker.bIsValid())
    {
        m_wndBlanker.Destroy();

        //
        //  Just in case the power was off, power on the monitor. The blanker
        //  also does it upon being destroyed, but sometimes it doesn't
        //  seem sufficient. So this is just an extra kick.
        //
        facCIDCtrls().SetMonitorPowerState(*this, tCIDCtrls::EMonPowerSt::On);

        //
        //  Make sure that the interface engine's activity timer gets reset, since
        //  the input is often just on the blanker window, not on the IV window, so
        //  the timer wouldn't have gotten reset and it could just re-blank again.
        //
        if (m_pwndClient)
            m_pwndClient->UpdateLastActivityStamp();
    }
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMainFrameWnd::bCheckShutdown()
{
    //
    //  If we are in kiosk mode and full screen mode, and the current user
    //  isn't a system admin, then put up the logon dialog and make them
    //  provide a system admin account.
    //
    if (facCQCIntfView.bKioskMode()
    &&  facCQCIntfView.bFullScreen()
    &&  (facCQCIntfView.eUserRole() != tCQCKit::EUserRoles::SystemAdmin))
    {
        TCQCSecToken    sectToFill;
        TCQCUserAccount uaccToFill;

        //
        //  Suppress environmental logon because we want to insure this person,
        //  is a system admin.
        //
        //  We set limited as the minimum role because we want to display
        //  the dialog if it's not of sufficient privs.
        //
        if (!facCQCGKit().bLogon(*this
                                , sectToFill
                                , uaccToFill
                                , tCQCKit::EUserRoles::LimitedUser
                                , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                                , kCIDLib::True
                                , L"IntfViewer"))
        {
            return kCIDLib::False;
        }

        // See if the account is a system admin. If not, tell them no go
        if (uaccToFill.eRole() != tCQCKit::EUserRoles::SystemAdmin)
        {
            TErrBox msgbWarn
            (
                facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                , facCQCKit().strMsg(kKitMsgs::midStatus_MustBeAdmin)
            );
            msgbWarn.ShowIt(*this);
            return kCIDLib::False;
        }
    }

    tCIDLib::TBoolean bExit = kCIDLib::False;
    TYesNoBox msgbAsk
    (
        facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
        , facCQCIntfView.strMsg(kIViewMsgs::midQ_Exit)
    );

    if (msgbAsk.bShowIt(*this))
    {
        facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  The user can ask us to do a screen capture for them. We grab an image
//  of the current template contents, and allow them to save it as a PNG
//  or JPEG image. We are told which they wanted.
//
tCIDLib::TVoid
TMainFrameWnd::DoScreenCapture(const tCIDLib::TBoolean bDoPNG)
{
    try
    {
        //
        //  OK, let's see if there's a template loaded. If not, then
        //  there's nothing to do.
        //
        TString strBase;
        TString strTop;
        if (!m_pwndClient || !m_pwndClient->civTarget().c4QueryStatus(strBase, strTop))
        {
            // No templates are loaded or not client window
            TOkBox msgbNoTempl
            (
                facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
                , facCQCIntfView.strMsg(kIViewMsgs::midStatus_NoTemplateLoaded)
            );
            msgbNoTempl.ShowIt(*this);
            return;
        }

        //
        //  There's something there, so let's get the area of the base
        //  template. That's the area we'll capture.
        //
        const TCQCIntfTemplate& iwdgBase = m_pwndClient->civTarget().iwdgBaseTmpl();

        //
        //  WE need to create a bitmp that we'll copy the data into
        //  and later use to create the output image. WE create a true
        //  alpha bitmap and force it to be memory based, so that we can
        //  directly access the bits later.
        //
        TArea areaCapture = iwdgBase.areaActual();
        TClrPalette palDummy;
        TBitmap bmpCapture
        (
            areaCapture.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
            , palDummy
            , kCIDLib::True

        );
        TGraphWndDev gdevClient(*m_pwndClient);

        //
        //  OK, now we can just copy bits from the client window device.
        //  The target is the same size, but at zero origin, sine our
        //  capture bitmap is the size of the base template, not of the
        //  window.
        //
        //  Do this in a faux scope, because we want the memory device
        //  to release the bitmap once we've got our data.
        //
        {
            TGraphMemDev gdevCapture(gdevClient, bmpCapture);
            TArea areaTarget(areaCapture);
            areaTarget.ZeroOrg();
            gdevCapture.CopyBits(gdevClient, areaCapture, areaTarget);
        }

        // Set up the file type filter for the file save dialog and name
        TString strFileName(L"IntfSnap");
        tCIDLib::TKVPList colFileTypes(1);
        if (bDoPNG)
        {
            colFileTypes.objPlace(L"PNG Files", L"*.PNG");
            strFileName.Append(L".PNG");
        }
         else
        {
            colFileTypes.objPlace(L"JPEG Files", L"*.JPEG");
            strFileName.Append(L".JPG");
        }

        //
        //  If no previously selected path, then start with something
        //  useful.
        //
        static TPathStr pathLastUsed;
        TString strStartPath = pathLastUsed;
        if (strStartPath.bIsEmpty())
            strStartPath = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData);

        // And get a selection.
        TString strSelected;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bSaveFileDlg
        (
            *this
            , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
            , strStartPath
            , strFileName
            , strSelected
            , colFileTypes
            , tCIDCtrls::EFSaveOpts::FilesOnly
        );

        if (bRes)
        {
            //
            //  Use the bits from the capture device to create a PNG or
            //  JPEG image object. We have ask the bitmap to flip the
            //  contents vertically, because it is otherwise in the
            //  wrong order for the needs of the images we create.
            //
            bmpCapture.pixaData().FlipVertically();
            TCIDImage* pimgOut = 0;
            if (bDoPNG)
                pimgOut = new TPNGImage(bmpCapture.pixaData());
            else
                pimgOut = new TJPEGImage(bmpCapture.pixaData());
            TJanitor<TCIDImage> janImage(pimgOut);

            // Create an output file stream to dump it to and dump it
            TBinFileOutStream strmTar
            (
                strSelected
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SafeStream
                , tCIDLib::EAccessModes::Write
            );
            strmTar << *pimgOut << kCIDLib::FlushIt;

            // Save this as the last accessed path
            pathLastUsed = strSelected;
            pathLastUsed.bRemoveNameExt();
        }
    }

    catch(const TError& errToCatch)
    {
        facCQCGKit().ShowError
        (
            *this
            , facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
            , facCQCIntfWEng().strMsg(kIViewMsgs::midStatus_CantSaveCapture)
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbFail
        (
            facCQCIntfView.strMsg(kIViewMsgs::midWnd_AppName2)
            , facCQCIntfWEng().strMsg(kIViewMsgs::midStatus_CantSaveCapture)
        );
        msgbFail.ShowIt(*this);
    }
}


//
//  Loads up our main menu bar and set some items based on the role of
//  the logged in user.
//
tCIDLib::TVoid TMainFrameWnd::LoadMenu()
{
    // Set our main menu resource as our frame menu
    SetMenuBar(facCQCIntfView, kCQCIntfView::ridMenu_Main);

    //
    //  We don't let limited users open any template themselves. They only
    //  get the one set up for them by the admin.
    //
    const tCQCKit::EUserRoles eRole = facCQCIntfView.eUserRole();
    if (eRole == tCQCKit::EUserRoles::LimitedUser)
        menuCur().SetItemEnable(kCQCIntfView::ridMenu_File_Open, kCIDLib::False);

    //
    //  Initialize the enable caching option to match our member that we use to track it.
    //  It is set initially to true.
    //
    menuCur().SetItemCheck(kCQCIntfView::ridMenu_Tools_EnableCaching, m_bCachingEnabled);
}


//
//  We call this any time our window size might have changed. We get our client
//  area and size the view window client to fit.
//
tCIDLib::TVoid TMainFrameWnd::ResizeView()
{
    // Size the client window to fit the new client area, if it's been created
    if (m_pwndClient)
        m_pwndClient->NewSize(areaClient().szArea());
}


//
//  When the user asks to select a template, this is called, and we call a
//  helper to let the user select one. If they do, we call another helper
//  method to load the selected one.
//
tCIDLib::TVoid TMainFrameWnd::SelectTemplate()
{
    TString strName;

    const tCIDLib::TBoolean bSelRes = facCQCTreeBrws().bSelectFile
    (
        *this
         , facCQCIntfView.strMsg(kIViewMsgs::midPrompt_SelectTmpl)
         , tCQCRemBrws::EDTypes::Template
         , L"/User"
         , facCQCIntfView.cuctxToUse()
         , tCQCTreeBrws::EFSelFlags::SelectItems
         , strName
    );

    if (bSelRes)
        bLoadTemplate(strName, kCIDLib::False);
}

