//
// FILE NAME: CQCInst_MainFrameWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"



// ----------------------------------------------------------------------------
//  Do our RTTI macros
// ----------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWindow)



// -----------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// -----------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_bGotLogin(kCIDLib::False)
    , m_c4CurPanel(0)
    , m_colPanels(tCIDLib::EAdoptOpts::NoAdopt)
    , m_cuctxAdmin()
    , m_pwndBackButton(nullptr)
    , m_pwndCloseButton(nullptr)
    , m_pwndHideIn(nullptr)
    , m_pwndNextButton(nullptr)
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// -----------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// -----------------------------------------------------------------------------

//
//  Called by the facility class' main thread to create installers main window.
//  We return true if we get created successfully, else false. If we return
//  false, he'll just turn around and exit.
//
tCIDLib::TBoolean TMainFrameWnd::bCreateMainWnd()
{
    //
    //  Try to find any previous install. If we don't find one, we'll ask
    //  the user to validate this, and find one for us if needed. When we get
    //  back, if any previous install info was found, it's been stored in
    //  the 'old' data part of our m_infoInstall member. If it returns false,
    //  then they canceled and we just return false to exit the installer
    //  with no action.
    //
    if (!bFindOldVer())
        return kCIDLib::False;

    // If the target type is a version we cannot upgrade, we can't do it
    if ((m_infoInstall.eTargetType() == tCQCInst::ETargetTypes::BadOldVer)
    ||  (m_infoInstall.eTargetType() == tCQCInst::ETargetTypes::RetroVersion))
    {
        if (m_infoInstall.eTargetType() == tCQCInst::ETargetTypes::RetroVersion)
        {
            // They are trying to install an older version than what's there
            TErrBox msgbWarn(facCQCInst.strMsg(kCQCInstMsgs::midStatus_RetroVersion));
            msgbWarn.ShowIt(TWindow::wndDesktop());
        }
         else
        {
            //
            //  They are trying to install over a version older than we can
            //  upgrade from, so show them the minimum upgradeable version
            //  and return false.
            //
            TString strVersion;
            facCQCKit().FormatVersionStr(strVersion, kCQCInst::c8MinUpgradeVer);
            TErrBox msgbWarn
            (
                facCQCInst.strMsg(kCQCInstMsgs::midStatus_BadPrevVersion, strVersion)
            );
            msgbWarn.ShowIt(TWindow::wndDesktop());
        }
        return kCIDLib::False;
    }

    // Load the logo and icon images we use
    m_bmpLogo = facCQCGKit().bmpLoadLogo(L"CQS_Small");

    //
    //  Load the dialog description for our main window content. There is a place holder
    //  for the panels, which will be removed in bCreated() when we load everything up.
    //
    TDlgDesc dlgdChildren;
    facCQCInst.bCreateDlgDesc(kCQCInst::ridMain, dlgdChildren);

    // Calculate the area required to fit this content
    TArea areaInit;
    TFrameWnd::AreaForClient
    (
        dlgdChildren.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    // Set the origin to the min int value so that they will default
    areaInit.SetOrg(kCIDLib::i4MinInt, kCIDLib::i4MinInt);

    // And now create the window
    CreateFrame
    (
        nullptr
        , areaInit
        , facCQCInst.strMsg(kCQCInstMsgs::midGen_Title1)
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    // Disable sizing by setting the min/max size to our current size
    SetMinMaxSize(areaInit.szArea(), areaInit.szArea());

    return kCIDLib::True;
}


//
//  If we already have valid creds, return them and a true result, else ask the
//  user to log in. If successful, store the info away and return true.
//
tCIDLib::TBoolean
TMainFrameWnd::bLogin(  const   TWindow&        wndOwner
                        , const TString&        strTitle
                        ,       TCQCUserCtx&    cuctxToFill)
{
    //
    //  If we current have login info, then see if the MS end piont is the same as it was
    //  before. If so, we can return what we have.
    //
    if (m_bGotLogin)
    {
        if (m_infoInstall.viiNewInfo().strMSAddr() == m_strLastMSEP)
        {
            cuctxToFill = m_cuctxAdmin;
            return kCIDLib::True;
        }
    }

    // Didn't have it or it's changed
    m_bGotLogin = kCIDLib::False;

    // If the client ORB is already initialized, terminate it
    if (facCIDOrb().bIsInitialized(tCIDLib::ECSSides::Client))
        facCIDOrb().Terminate();

    //
    //  Set up the name server address in TSysInfo. This is where the ORB will pick
    //  it up. Normally he gets it from the command line or environment, but we are
    //  setting it after the fact. And then initialize the client.
    //
    TSysInfo::SetNSAddr
    (
        m_infoInstall.viiNewInfo().strMSAddr(), m_infoInstall.viiNewInfo().ippnNameServer()
    );
    facCIDOrb().InitClient();

    // And now we can try to to do the login
    TCQCSecToken    sectCreator;
    TCQCUserAccount uaccCreator;
    m_bGotLogin = facCQCGKit().bLogon
    (
        wndOwner
        , sectCreator
        , uaccCreator
        , tCQCKit::EUserRoles::SystemAdmin
        , strTitle
        , kCIDLib::True
        , L"CQCApp"
    );

    // If it worked, store the security info and the target end point for next time
    if (m_bGotLogin)
    {
        m_cuctxAdmin.Set(uaccCreator, sectCreator);
        m_strLastMSEP = m_infoInstall.viiNewInfo().strMSAddr();

        // WE directly give a copy of the user context back as well
        cuctxToFill = m_cuctxAdmin;
    }

    return m_bGotLogin;
}


//
//  This is called by the panels to control what buttons we present and or
//  enable/disable.
//
//  We want to have the default emphasis on next if possible, then previous
//  if possible, and finally close if the only one left.
//
tCIDLib::TVoid TMainFrameWnd::SetNavFlags(const tCQCInst::ENavFlags eToSet)
{
    if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowNext))
    {
        // Make sure default emphasis is here
        m_pwndNextButton->SetEnable(kCIDLib::True);
        m_pwndNextButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::True);
        m_pwndNextButton->SetDefButton();
        m_pwndBackButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
        m_pwndCloseButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
    }
     else
    {
        // If this one has the focus, give it to someone else, then disable
        if (m_pwndNextButton->bHasFocus())
        {
            if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowPrev))
                m_pwndBackButton->TakeFocus();
            else if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowClose))
                m_pwndCloseButton->TakeFocus();
        }
        m_pwndNextButton->SetEnable(kCIDLib::False);
    }

    if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowPrev))
    {
        m_pwndBackButton->SetEnable(kCIDLib::True);

        // Make sure default emphasis is here if next not enabled
        if (!tCIDLib::bAnyBitsOn(eToSet, tCQCInst::ENavFlags::AllowPrev))
        {
            m_pwndBackButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::True);
            m_pwndBackButton->SetDefButton();
            m_pwndNextButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
            m_pwndCloseButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
        }
    }
     else
    {
        // If this one has the focus, give it to someone else, then disable
        if (m_pwndBackButton->bHasFocus())
        {
            if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowNext))
                m_pwndNextButton->TakeFocus();
            else if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowClose))
                m_pwndCloseButton->TakeFocus();
        }
        m_pwndBackButton->SetEnable(kCIDLib::False);
    }

    if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowClose))
    {
        m_pwndCloseButton->SetEnable(kCIDLib::True);

        // Make sure default emphasis is here if neither of the others is enabled
        if (!tCIDLib::bAnyBitsOn(eToSet, tCQCInst::ENavFlags::AllowNext)
        &&  !tCIDLib::bAnyBitsOn(eToSet, tCQCInst::ENavFlags::AllowPrev))
        {
            m_pwndCloseButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::True);
            m_pwndCloseButton->SetDefButton();
            m_pwndNextButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
            m_pwndBackButton->SetWndStyle(tCIDCtrls::EWndStyles::TabStop, kCIDLib::False);
        }
    }
     else
    {
        // If this one has the focus, give it to someone else, then disable
        if (m_pwndCloseButton->bHasFocus())
        {
            if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowNext))
                m_pwndNextButton->TakeFocus();
            else if (tCIDLib::bAllBitsOn(eToSet, tCQCInst::ENavFlags::AllowPrev))
                m_pwndBackButton->TakeFocus();
        }
        m_pwndCloseButton->SetEnable(kCIDLib::False);
    }
}


// -----------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// -----------------------------------------------------------------------------
tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    //
    //  If we've not actually started, just aks if they really want to
    //  exist before installing and give them a chance to back out.
    //
    if (m_infoInstall.eInstStatus() == tCQCInst::EInstStatus::Setup)
    {
        TYesNoBox msgbExit(facCQCInst.strMsg(kCQCInstMsgs::midQ_Exit));
        return msgbExit.bShowIt(*this);
    }


    if (m_infoInstall.eInstStatus() >= tCQCInst::EInstStatus::Done)
    {
        //
        //  If the install completed or failed, then we can exit. If it
        //  completed, then run the release notes.
        //
        if (m_infoInstall.eInstStatus() == tCQCInst::EInstStatus::Done)
        {
            TPathStr pathNotes(m_infoInstall.strPath());
            pathNotes.AddLevel(L"\\ReleaseNotes.htm");
            if (TFileSys::bExists(pathNotes))
            {
                pathNotes.Prepend(L"file:///");
                TGUIShell::InvokeDefBrowser(*this, pathNotes);
            }
        }
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    // Pass it on first
    TParent::bCreated();

    // Set our icon
    facCQCInst.SetFrameIcon(*this, L"CQCApp");

    // Load our dialog description and create the controls
    TDlgDesc dlgdChildren;
    facCQCInst.bCreateDlgDesc(kCQCInst::ridMain, dlgdChildren);
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Look up our children
    CastChildWnd(*this, kCQCInst::ridMain_Back, m_pwndBackButton);
    CastChildWnd(*this, kCQCInst::ridMain_Next, m_pwndNextButton);
    CastChildWnd(*this, kCQCInst::ridMain_Close, m_pwndCloseButton);
    CastChildWnd(*this, kCQCInst::ridMain_HideIn, m_pwndHideIn);

    // Hide the hide in window
    m_pwndHideIn->SetVisibility(kCIDLib::False);

    // Register handlers for the buttons
    m_pwndBackButton->pnothRegisterHandler(this, &TMainFrameWnd::eMFClickHandler);
    m_pwndNextButton->pnothRegisterHandler(this, &TMainFrameWnd::eMFClickHandler);
    m_pwndCloseButton->pnothRegisterHandler(this, &TMainFrameWnd::eMFClickHandler);

    // Get the panel placeholder window's area then destroy it
    TWindow* pwndArea = pwndChildById(kCQCInst::ridMain_Panels);
    const TArea areaPanels = pwndArea->areaWnd();
    pwndArea->Destroy();
    delete pwndArea;

    //
    //  Load our logo into the logo control. And size it to fit horizontally so that
    //  the image is left justified.
    //
    TStaticImg* pwndLogo = pwndChildAs<TStaticImg>(kCQCInst::ridMain_Logo);
    TArea areaNew = pwndLogo->areaWnd();
    areaNew.c4Width(m_bmpLogo.szBitmap().c4Width());
    pwndLogo->SetSize(areaNew.szArea(), kCIDLib::False);
    pwndLogo->SetBitmap(m_bmpLogo);

    //
    //  Load up the list of info panels in the correct order, and then let
    //  each of them initialize themselves. The summary panel is the only
    //  one we really need to know specifically is being selected so save
    //  its index.
    //
    m_colPanels.Add(new TInstIntroPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstModePanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstAgreePanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstMSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstCheckMSPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstSrvModePanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstStopGUIAppsPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstStopSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstBackupPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstPrereqPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstPathPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstClientPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstCertInfoPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstIVCtrlPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstMSPortsPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstCQCSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstXMLGWSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstEventPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstLogicSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstWebSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstACSrvPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstCQCVoicePanel(&m_infoInstall, this));
    m_c4PanInd_Summary = m_colPanels.c4ElemCount();
    m_colPanels.Add(new TInstSummaryPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstStatusPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstInitPWPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstCQCVoiceAcctPanel(&m_infoInstall, this));
    m_colPanels.Add(new TInstSuccessPanel(&m_infoInstall, this));


    //
    //  Create the panels, we just assign them sequential ids in a range that won't
    //  conflict with any other controls we create.
    //
    const tCIDLib::TCard4 c4Count = m_colPanels.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index< c4Count; c4Index++)
    {
        m_colPanels[c4Index]->CreateInfoPanel
        (
            *m_pwndHideIn, areaPanels, kCIDCtrls::widFirstCtrl + 1000 + c4Index
        );
    }

    // Find the first visible panel
    m_c4CurPanel = 0;
    if (!m_colPanels[0]->bPanelIsVisible())
        FindNextPanel();
    SetNavFlags(m_colPanels[m_c4CurPanel]->eNavFlags());

    // Make this initial one visible
    m_colPanels[m_c4CurPanel]->SetParent(*this);
    m_colPanels[m_c4CurPanel]->Entered();

    return kCIDLib::True;
}


//
//  The background thread goes through us to ask questions of the user, or
//  to tell him things, so as to avoid sync issues and the complications of
//  multiple threads doing windows. So it fills in a query packet, then does
//  a synchronous post to us. We handle the operation, and put any results
//  back into the packet and return. He's blocked during this time.
//
tCIDLib::TVoid
TMainFrameWnd::DataReceived(const   tCIDLib::TCard4         c4DataId
                            ,       tCIDLib::TVoid* const   pData)
{
    // Make sure it's the data id we expect. If not, just pass it on
    if (c4DataId != kCQCInst::wmsgQueryId)
        return TParent::DataReceived(c4DataId, pData);

    // The data is a query packet
    TInstQueryPacket* pqpackToDo = static_cast<TInstQueryPacket*>(pData);

    try
    {
        switch(pqpackToDo->eOp())
        {
            case tCQCInst::EQueryOps::MsgBox :
            {
                //
                //  It's a message box request. We use the generic msg box support to
                //  invoke the requested type, and give back the result. We select an
                //  icon based on the msg box type.
                //
                TMsgBox msgbShow(pqpackToDo->strMsg());

                tCIDCtrls::EMsgBoxIcons eIcon = tCIDCtrls::EMsgBoxIcons::Warning;
                switch(pqpackToDo->eMsgBoxType())
                {
                    case tCIDCtrls::EMsgBoxTypes::AbortRetryIgnore :
                    case tCIDCtrls::EMsgBoxTypes::CancelRetryContinue :
                        eIcon = tCIDCtrls::EMsgBoxIcons::Error;
                        break;

                    case tCIDCtrls::EMsgBoxTypes::Help :
                        eIcon = tCIDCtrls::EMsgBoxIcons::Asterisk;
                        break;

                    case tCIDCtrls::EMsgBoxTypes::OK :
                    case tCIDCtrls::EMsgBoxTypes::OKCancel :
                        eIcon = tCIDCtrls::EMsgBoxIcons::Information;
                        break;

                    case tCIDCtrls::EMsgBoxTypes::RetryCancel :
                    case tCIDCtrls::EMsgBoxTypes::YesNo :
                    case tCIDCtrls::EMsgBoxTypes::YesNoCancel :
                        eIcon = tCIDCtrls::EMsgBoxIcons::Question;
                        break;

                    default :
                        break;
                };

                pqpackToDo->eMsgBoxRet
                (
                    msgbShow.eShow(*this, pqpackToDo->eMsgBoxType(), eIcon)
                );
                break;
            }

            case tCQCInst::EQueryOps::ShowWarning :
            {
                TErrBox msgbWarn(pqpackToDo->strMsg());
                msgbWarn.ShowIt(*this);
                break;
            }

            case tCQCInst::EQueryOps::ShowFatalErr :
            {
                TExceptDlg dlgErr
                (
                    *this
                    , strWndText()
                    , pqpackToDo->strMsg()
                    , pqpackToDo->errException()
                );
                break;
            }
        };
    }

    //
    //  We definitely do NOT want to let any exception propogate out of here
    //  because it's invoked from a background thread, which gets blocked and
    //  the operation takes place in the GUI thread. If this throws, it throws
    //  in the GUI thread, and not back to the caller that would otherwise
    //  be expected to handle it. So we would deadlock.
    //
    catch(const TError& errToCatch)
    {
        // This will also set the result to indicate an exception
        pqpackToDo->errException(errToCatch);
    }

    catch(...)
    {
        pqpackToDo->eResult(tCQCInst::EQueryRes::UnknownException);
    }
}


tCIDLib::TBoolean
TMainFrameWnd::bPaint(TGraphDrawDev& gdevTarget, const TArea& areaUpdate)
{
    //
    //  Lets draw our logo. Make sure we really got it loaded or this will fail
    //  because we'll have gotten a default (non-Alpha based) default 1x1 mono
    //  images.
    //
    if (!m_areaLogo.bIsEmpty())
        gdevTarget.AlphaBlit(m_bmpLogo, m_areaLogo, 0xFF, kCIDLib::True);

    return kCIDLib::True;
}


// -----------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// -----------------------------------------------------------------------------

//
//  We get clicks from our back/next/cancel buttons here.
//
tCIDCtrls::EEvResponses TMainFrameWnd::eMFClickHandler(TButtClickInfo& wnotEvent)
{
    const tCIDCtrls::TWndId widEv = wnotEvent.widSource();
    switch(widEv)
    {
        case  kCQCInst::ridMain_Back :
        {
            //
            //  If we are already on the 0th panel, this should be disabled
            //  but be extra careful.
            //
            if (m_c4CurPanel)
            {
                TInstInfoPanel* ppanCur = m_colPanels[m_c4CurPanel];

                // Ask the current panel to save it's contents and hide
                ppanCur->SaveContents();

                // Allow downstream panels to react to changes
                ReportChanges();

                // And hide this current one
                ppanCur->SetParent(*m_pwndHideIn);

                // Find the previous panel and let it load
                FindPrevPanel();
                ppanCur = m_colPanels[m_c4CurPanel];
                ppanCur->SetParent(*this);
                ppanCur->Entered();

                SetNavFlags(ppanCur->eNavFlags());
            }
            break;
        }

        case kCQCInst::ridMain_Close:
        {
            if (bAllowShutdown())
                facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
            break;
        }

        case kCQCInst::ridMain_Next :
        {
            TInstInfoPanel* ppanCur = m_colPanels[m_c4CurPanel];

            // Ask the current panel to save its contents
            ppanCur->SaveContents();

            // Ask it to validate the current contents
            tCIDCtrls::TWndId widFailed = 0;
            TString strErrText;
            tCIDLib::TBoolean bRes = ppanCur->bValidate(widFailed, strErrText);

            // Mark this panel as validated or not, based on result
            ppanCur->bValidated(bRes);

            //
            //  If it passed, then save the panel's contents and move forward.
            //  Else, show the error and stay put.
            //
            if (bRes)
            {
                // Hide the current panel's contents
                ppanCur->SetParent(*m_pwndHideIn);

                // Allow downstream panels to react to changes
                ReportChanges();

                //
                //  Move up and load the new one. We can't just increment
                //  the count since some panels don't want to be seen in
                //  certain circumstances.
                //
                FindNextPanel();

                //
                //  If this would take us to the summary panel, then first
                //  run all the final global validation stuff, and if we
                //  get a failure, go back to that panel.
                //
                if (m_c4CurPanel == m_c4PanInd_Summary)
                    m_c4CurPanel = c4GlobalValidation();

                ppanCur = m_colPanels[m_c4CurPanel];
                SetNavFlags(ppanCur->eNavFlags());

                // Show this guy's contents
                ppanCur->SetParent(*this);
                ppanCur->Entered();

                //
                //  Now call the eProcess() method. Some panels just immediatley
                //  do something upon load and then ask us to move immediately
                //  to the next panel. If they indicate to move forward, we
                //  assume that no validation is required.
                //
                //  We currently assume that we will not have more than one
                //  of these in a row!
                //
                tCQCInst::EProcRes eRes = ppanCur->eProcess();
                if (eRes == tCQCInst::EProcRes::Next)
                {
                    ppanCur->SaveContents();
                    ppanCur->SetParent(*m_pwndHideIn);
                    ReportChanges();
                    FindNextPanel();
                    ppanCur = m_colPanels[m_c4CurPanel];
                    SetNavFlags(ppanCur->eNavFlags());
                    ppanCur->SetParent(*this);
                    ppanCur->Entered();
                }
            }
             else
            {
                TErrBox msgbFailed(strErrText);
                msgbFailed.ShowIt(*this);

                if (ppanCur && widFailed)
                    ppanCur->pwndChildById(widFailed)->TakeFocus();
            }
            break;
        }

        default :
            break;
    };
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Find the next or previous visible panel and set the current panel index to that.
//  We skip any that don't want to be seen in the current circumstances.
//
//  We should not be called here if already at the far ends, but of course we need
//  to check to be careful.
//
tCIDLib::TVoid TMainFrameWnd::FindNextPanel()
{
    const tCIDLib::TCard4 c4Count = m_colPanels.c4ElemCount();
    while (m_c4CurPanel < c4Count - 1)
    {
        m_c4CurPanel++;
        if (m_colPanels[m_c4CurPanel]->bPanelIsVisible())
            break;
    }
}

tCIDLib::TVoid TMainFrameWnd::FindPrevPanel()
{
    // We go back till we find one that is visible
    while (m_c4CurPanel)
    {
        m_c4CurPanel--;
        if (m_colPanels[m_c4CurPanel]->bPanelIsVisible())
            break;
    }
}


//
//  In the next/previous handling code (in the button click handler above),
//  after the current panel is given a change to save it's contents, this is
//  called. We call all downstream panels and let them react to the changed
//  configuration contents.
//
tCIDLib::TVoid TMainFrameWnd::ReportChanges()
{
    const tCIDLib::TCard4 c4Count = m_colPanels.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = m_c4CurPanel; c4Index < c4Count; c4Index++)
        m_colPanels[c4Index]->ReactToChanges();
}

