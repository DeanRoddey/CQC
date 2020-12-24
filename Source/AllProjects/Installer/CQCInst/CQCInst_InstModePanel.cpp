//
// FILE NAME: CQCInst_InstModePanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This file implements the info panel that asks the user what components they
//  want to install.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstModePanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstModePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstModePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstModePanel::TInstModePanel( TCQCInstallInfo* const  pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Install Mode", pinfoCur, pwndParent)
    , m_pwndComps(nullptr)
    , m_pwndCompDescr(nullptr)
    , m_pwndUsePrevOpts(nullptr)
    , m_strInstall(L"Install")
    , m_strNotInstalled(L"Not Installed")
    , m_strUninstall(L"Uninstall")
    , m_strUpgrade(L"Upgrade")
{
}

TInstModePanel::~TInstModePanel()
{
}


// ---------------------------------------------------------------------------
//  TInstModePanel: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  When they commit to this panel, we just update the install info to reflect
//  the selected install mode.
//
tCIDLib::TBoolean
TInstModePanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    //
    //  Make sure that a reasonable combination of components are installed. We'll set up
    //  a list of boolean flags in the order of the component enum.
    //
    TFundVector<tCIDLib::TBoolean, tCQCInst::EInstComps> fcolFlags;
    TString strVal;

    tCIDLib::TCard4 c4SetCount = 0;
    tCQCInst::EInstComps eComp = tCQCInst::EInstComps::Min;
    while(eComp < tCQCInst::EInstComps::Count)
    {
        m_pwndComps->QueryColText(tCIDLib::c4EnumOrd(eComp), 0, strVal);
        const tCIDLib::TBoolean bInstall = bCompState(eComp, kCIDLib::True);
        fcolFlags.c4AddElement(bInstall);
        eComp++;

        if (bInstall)
            c4SetCount++;
    }

    // If no components were enabled, then obviously not
    if (!c4SetCount)
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoComps, facCQCInst);
        return kCIDLib::False;
    }

    //
    //  If any components are on that only can be installed on the master server, then make
    //  sure that the master server component is enabled.
    //
    if (!fcolFlags[tCQCInst::EInstComps::MasterSrv])
    {
        if (fcolFlags[tCQCInst::EInstComps::EventSrv])
        {
            strErrText.LoadFromMsg
            (
                kCQCInstErrs::errcVal_RequiresMS, facCQCInst, TString(L"Event Server")
            );
            return kCIDLib::False;
        }

        if (fcolFlags[tCQCInst::EInstComps::LogicSrv])
        {
            strErrText.LoadFromMsg
            (
                kCQCInstErrs::errcVal_RequiresMS, facCQCInst, TString(L"Logic Server")
            );
            return kCIDLib::False;
        }
    }

    //
    //  If not installing the master server and using previous settings, we go ahead and force
    //  them to log in now, to prove they have the right to install a client on this system. If
    //  not using previous settings they might change (or it might not be set yet), and they'll
    //  be forced to do it in the master server panel validation.
    //
    if (!infoCur().viiNewInfo().bMasterServer() && infoCur().bUsePrevOpts())
    {
        TCQCUserCtx cuctxUser;
        if (!facCQCInst.wndMain().bLogin(*this, facCQCInst.strMsg(kCQCInstMsgs::midGen_LogIn), cuctxUser))
            return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  We can only go back from here if it's a clean install. Otherwise, this is the first
//  panel they see and as far back as we want to allow them to go.
//
tCQCInst::ENavFlags TInstModePanel::eNavFlags() const
{
    if (infoCur().eTargetType() == tCQCInst::ETargetTypes::Clean)
        return tCQCInst::ENavFlags::AllowAll;

    return tCQCInst::ENavFlags::NoBack;
}


//
//  We load the install/no-install flags upon each entry, so as to pick up any changes that
//  might have been caused by other panels.
//
tCIDLib::TVoid TInstModePanel::Entered()
{
    LoadCompIndicators();
}


tCIDLib::TVoid TInstModePanel::SaveContents()
{
    // It's already stored, since we just update it live
}


// ---------------------------------------------------------------------------
//  TInstModePanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstModePanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Mode_Title));

    // Create our child controls
    LoadDlgItems(kCQCInst::ridPan_Mode);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Mode_Comps, m_pwndComps);
    CastChildWnd(*this, kCQCInst::ridPan_Mode_CompDescr, m_pwndCompDescr);
    CastChildWnd(*this, kCQCInst::ridPan_Mode_UsePrevOpts, m_pwndUsePrevOpts);

    // Set the columns on the components list
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Component Status");
    colCols.objAdd(L"Component Description");
    m_pwndComps->SetColumns(colCols);

    //
    //  Now load the available components. Initially leave the 0th column empty since
    //  we'll se that later.
    //
    tCQCInst::EInstComps eComp = tCQCInst::EInstComps::Min;
    colCols[0].Clear();
    while(eComp < tCQCInst::EInstComps::Count)
    {
        colCols[1] = tCQCInst::strXlatEInstComps(eComp);
        m_pwndComps->c4AddItem(colCols, tCIDLib::c4EnumOrd(eComp));
        eComp++;
    }

    // Size the first column to fit the header
    m_pwndComps->AutoSizeCol(0, kCIDLib::True);

    // Install a list change handler on our main window, before we do the initial selection!
    m_pwndComps->pnothRegisterHandler(this,&TInstModePanel::eListHandler);
    m_pwndUsePrevOpts->pnothRegisterHandler(this,&TInstModePanel::eClickHandler);

    //
    //  If a clean install, then hide the previous options check box, else check
    //  as the default state it if upgrading. They can uncheck it to make changes.
    //
    if (infoCur().eTargetType() == tCQCInst::ETargetTypes::Clean)
    {
        m_pwndUsePrevOpts->SetVisibility(kCIDLib::False);
        infoCur().viiNewInfo().EnableAll();
    }
     else
    {
        m_pwndUsePrevOpts->SetCheckedState(infoCur().bUsePrevOpts());
    }

    // Select an initial item
    m_pwndComps->SelectByIndex(0, kCIDLib::True);

    // Indicate we want to see child focus changes
    bWantsChildFocusNot(kCIDLib::True);

    return kCIDLib::True;
}


//
//  If the component list loses the focus, clear out the component description text. When
//  it gets the focus, we need to put it back.
//
tCIDLib::TVoid
TInstModePanel::ChildFocusChange(const  TWindow&            wndParent
                                , const tCIDCtrls::TWndId   widChild
                                , const tCIDLib::TBoolean   bGotFocus)
{
    if (wndParent.bIsThisWindow(*this) && (widChild == kCQCInst::ridPan_Mode_Comps))
    {
        if (bGotFocus)
        {
            m_pwndCompDescr->strWndText
            (
                facCQCInst.strMsg(kCQCInstMsgs::midCompD_CTools + m_pwndComps->c4CurItemId())
            );
        }
         else
        {
           m_pwndCompDescr->ClearText();
        }
    }
}



// ---------------------------------------------------------------------------
//  TInstModePanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to get the new or old install state flag for a given component. We work
//  in terms of compnents here, so this avoids a lot of grunt code.
//
tCIDLib::TBoolean
TInstModePanel::bCompState( const   tCQCInst::EInstComps    eSrcComp
                            , const tCIDLib::TBoolean       bNew)
{
    const TCQCVerInstallInfo& viiSrc = bNew ? infoCur().viiNewInfo() : infoCur().viiOldInfo();

    tCIDLib::TBoolean bRet;
    switch(eSrcComp)
    {
        case tCQCInst::EInstComps::CTools :
            bRet = viiSrc.bCQCClient();
            break;

        case tCQCInst::EInstComps::DevCtrl :
            bRet = viiSrc.bCQCServer();
            break;

        case tCQCInst::EInstComps::EventSrv :
            bRet = viiSrc.bEventServer();
            break;

        case tCQCInst::EInstComps::IntfViewer :
            bRet = viiSrc.bIntfViewer();
            break;

        case tCQCInst::EInstComps::CQCVoice :
            bRet = viiSrc.bCQCVoice();
            break;

        case tCQCInst::EInstComps::LogicSrv :
            bRet = viiSrc.bLogicServer();
            break;

        case tCQCInst::EInstComps::MasterSrv :
            bRet = viiSrc.bMasterServer();
            break;

        case tCQCInst::EInstComps::TrayMon :
            bRet = viiSrc.bTrayMon();
            break;

        case tCQCInst::EInstComps::WebSrv :
            bRet = viiSrc.bWebServer();
            break;

        case tCQCInst::EInstComps::XMLGW :
            bRet = viiSrc.bXMLGWServer();
            break;

        default :
            CIDAssert2(L"Unknown component enum");
            break;
    };

    return bRet;
}


tCIDCtrls::EEvResponses TInstModePanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Mode_UsePrevOpts)
    {
        if (wnotEvent.bState())
        {
            // In upgrade mode, so revert to the old stuff, and reload the list
            infoCur().RevertToOld();
            LoadCompIndicators();
        }

        // Store the new state to the install info
        infoCur().bUsePrevOpts(wnotEvent.bState());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// When the component combo box selection changes, we display some info about the new one
tCIDCtrls::EEvResponses TInstModePanel::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        m_pwndCompDescr->strWndText
        (
            facCQCInst.strMsg(kCQCInstMsgs::midCompD_CTools + wnotEvent.c4Index())
        );
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        // If the 'use previous options' check box is checked, then we ignore these
        if (m_pwndUsePrevOpts->bCheckedState())
            return tCIDCtrls::EEvResponses::Handled;

        // Check for seem special cases
        TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();
        const TCQCVerInstallInfo& viiOld = infoCur().viiOldInfo();

        // Get the enum of the selected component
        const tCQCInst::EInstComps eCurComp(tCQCInst::EInstComps(m_pwndComps->c4CurItemId()));

        // If it's the web server and the MS is being installed here, don't allow a change
        if ((eCurComp == tCQCInst::EInstComps::WebSrv) && viiNew.bMasterServer())
        {
            TErrBox msgbErrWS(facCQCInst.strMsg(kCQCInstErrs::errcFail_WebSrvReq));
            msgbErrWS.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  If they enable something that requires the master server but the master server
        //  isn't enabled, ask if they want to do this.
        //
        if (((eCurComp == tCQCInst::EInstComps::EventSrv)
        ||   (eCurComp == tCQCInst::EInstComps::LogicSrv))
        &&  !viiNew.bMasterServer())
        {
            TYesNoBox msgbAsk(facCQCInst.strMsg(kCQCInstMsgs::midQ_NeedsMS));
            if (!msgbAsk.bShowIt(*this))
                return tCIDCtrls::EEvResponses::Handled;

            // Force the master server on
            viiNew.bMasterServer(kCIDLib::True);

            m_pwndComps->SetColText
            (
                tCIDLib::c4EnumOrd(tCQCInst::EInstComps::MasterSrv)
                , 0
                , strStatusText(viiOld.bMasterServer(), kCIDLib::True)
            );
        }

        // Get whether it was previously installed or not installed
        const tCIDLib::TCard4 c4Index = m_pwndComps->c4CurItem();
        const tCIDLib::TBoolean bWasInstalled(bCompState(eCurComp, kCIDLib::False));

        //
        //  If it was the master server being disabled, and it was enabled in the
        //  previous config, then make sure they really want to do this.
        //
        if ((eCurComp == tCQCInst::EInstComps::MasterSrv)
        &&  bWasInstalled
        &&  viiOld.bMasterServer())
        {
            TYesNoBox msgbAsk(facCQCInst.strMsg(kCQCInstMsgs::midQ_DisableMS));
            if (!msgbAsk.bShowIt(*this))
                return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  If they disabled the master server, then make sure anything that requires it
        //  is disabled. If being enabled, make sure anything it requires is enabled.
        //
        if (eCurComp == tCQCInst::EInstComps::MasterSrv)
        {
            if (bWasInstalled)
            {
                // Event server and logic server can only be on MS
                if (viiNew.bEventServer())
                {
                    viiNew.bEventServer(kCIDLib::False);
                    m_pwndComps->SetColText
                    (
                        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::EventSrv)
                        , 0
                        , strStatusText(viiOld.bEventServer(), kCIDLib::False)
                    );
                }

                if (viiNew.bLogicServer())
                {
                    viiNew.bLogicServer(kCIDLib::False);
                    m_pwndComps->SetColText
                    (
                        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::LogicSrv)
                        , 0
                        , strStatusText(viiOld.bEventServer(), kCIDLib::False)
                    );
                }
            }
             else
            {
                // Web sever has to be be installed on the MS
                if (!viiNew.bWebServer())
                {
                    viiNew.bWebServer(kCIDLib::True);
                    m_pwndComps->SetColText
                    (
                        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::WebSrv)
                        , 0
                        , strStatusText(viiOld.bWebServer(), kCIDLib::True)
                    );
                }
            }
        }

        // Flip the state of the target component in the new config
        SetCompState(eCurComp, !bCompState(eCurComp, kCIDLib::True));

        // And update the status text
        m_pwndComps->SetColText
        (
            tCIDLib::c4EnumOrd(eCurComp)
            , 0
            , strStatusText
              (
                bCompState(eCurComp, kCIDLib::False)
                , bCompState(eCurComp, kCIDLib::True)
              )
        );

        return tCIDCtrls::EEvResponses::Handled;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called when we our panel has to load up its contents, to get the installed/not
//  installed flags in the list box set up to match the initial values.
//
tCIDLib::TVoid TInstModePanel::LoadCompIndicators()
{
    const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();
    const TCQCVerInstallInfo& viiOld = infoCur().viiOldInfo();

    // Tray monitor
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::TrayMon)
        , 0
        , strStatusText(viiOld.bTrayMon(), viiNew.bTrayMon())
    );

    // Device control
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::DevCtrl)
        , 0
        , strStatusText(viiOld.bCQCServer(), viiNew.bCQCServer())
    );


    // Event server
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::EventSrv)
        , 0
        , strStatusText(viiOld.bEventServer(), viiNew.bEventServer())
    );

    // Any of the client side management tools
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::CTools)
        , 0
        , strStatusText(viiOld.bCQCClient(), viiNew.bCQCClient())
    );

    // CQCVoice
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::CQCVoice)
        , 0
        , strStatusText(viiOld.bCQCVoice(), viiNew.bCQCVoice())
    );

    // Logic server
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::LogicSrv)
        , 0
        , strStatusText(viiOld.bLogicServer(), viiNew.bLogicServer())
    );

    // Interface viewer
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::IntfViewer)
        , 0
        , strStatusText(viiOld.bIntfViewer(), viiNew.bIntfViewer())
    );

    // The master server
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::MasterSrv)
        , 0
        , strStatusText(viiOld.bMasterServer(), viiNew.bMasterServer())
    );

    // The web server (which is always installed on the MS)
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::WebSrv)
        , 0
        , strStatusText(viiOld.bWebServer(), viiNew.bWebServer())
    );

    // The XML GW server
    m_pwndComps->SetColText
    (
        tCIDLib::c4EnumOrd(tCQCInst::EInstComps::XMLGW)
        , 0
        , strStatusText(viiOld.bXMLGWServer(), viiNew.bXMLGWServer())
    );
}


//
//  For a give component, return the string that best reflects it's installation
//  statatus.
//
const TString&
TInstModePanel::strStatusText(  const   tCIDLib::TBoolean   bOldState
                                , const tCIDLib::TBoolean   bNewState)
{
    // Was installed and still will be, so say upgrade
    if (bOldState && bNewState)
        return m_strUpgrade;

    // Was installed now not, so say uninstall
    if (bOldState && !bNewState)
        return m_strUninstall;

    // Wasn't installed and now will be, so say install
    if (!bOldState && bNewState)
        return m_strInstall;

    // Else has to be was not installed and still not
    return m_strNotInstalled;
}


// Set the new state of a component
tCIDLib::TVoid
TInstModePanel::SetCompState(const  tCQCInst::EInstComps    eTarComp
                            , const tCIDLib::TBoolean       bNewState)
{
    TCQCVerInstallInfo& viiTar = infoCur().viiNewInfo();

    switch(eTarComp)
    {
        case tCQCInst::EInstComps::CTools :
            viiTar.bCQCClient(bNewState);
            break;

        case tCQCInst::EInstComps::CQCVoice :
            viiTar.bCQCVoice(bNewState);
            break;

        case tCQCInst::EInstComps::DevCtrl :
            viiTar.bCQCServer(bNewState);
            break;

        case tCQCInst::EInstComps::EventSrv :
            viiTar.bEventServer(bNewState);
            break;

        case tCQCInst::EInstComps::IntfViewer :
            viiTar.bIntfViewer(bNewState);
            break;

        case tCQCInst::EInstComps::LogicSrv :
            viiTar.bLogicServer(bNewState);
            break;

        case tCQCInst::EInstComps::MasterSrv :
            viiTar.bMasterServer(bNewState);
            break;

        case tCQCInst::EInstComps::TrayMon :
            viiTar.bTrayMon(bNewState);
            break;

        case tCQCInst::EInstComps::WebSrv :
            viiTar.bWebServer(bNewState);
            break;

        case tCQCInst::EInstComps::XMLGW :
            viiTar.bXMLGWServer(bNewState);
            break;

        default :
            CIDAssert2(L"Unknown component enum");
            break;
    };
}
