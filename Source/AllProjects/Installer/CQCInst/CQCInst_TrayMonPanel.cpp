//
// FILE NAME: CQCInst_ACSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2004
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
//  This file implements the info panel that lets the user set the App Control
//  Server options.
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
RTTIDecls(TInstACSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstACSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstACSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstACSrvPanel::TInstACSrvPanel(TCQCInstallInfo* const pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Tray Monitor Server Options", pinfoCur, pwndParent)
    , m_bAutoStartAvail(kCIDLib::False)
    , m_pwndAppCtrl(nullptr)
    , m_pwndAutoStart(nullptr)
    , m_pwndBinding(nullptr)
    , m_pwndInstall(nullptr)
    , m_pwndiTunes(nullptr)
    , m_pwndPort(nullptr)
{
}

TInstACSrvPanel::~TInstACSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstACSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstACSrvPanel::bPanelIsVisible() const
{
    // If the tray monitor is being installed and not using previous opts
    return infoCur().viiNewInfo().bTrayMon() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstACSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // If not installing this component, then just say we are fine
    if (!infoCur().viiNewInfo().bTrayMon())
        return kCIDLib::True;

    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }

    // Make sure that they entered a binding name.
    if (infoCur().viiNewInfo().strTrayBinding().bIsEmpty())
    {
        widFailed = kCQCInst::ridPan_TrayMon_Binding;
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoTrayBinding, facCQCInst);
        return kCIDLib::False;
    }

    // Make sure it has no spaces
    tCIDLib::TCard4 c4Pos = 0;
    if (infoCur().viiNewInfo().strTrayBinding().bFirstOccurrence(kCIDLib::szWhitespace, c4Pos))
    {
        widFailed = kCQCInst::ridPan_TrayMon_Binding;
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_BadTrayBinding, facCQCInst);
        return kCIDLib::False;
    }

    //
    //  Make sure that either there is a service running, or app control or
    //  iTunes is enabled. Else, there's nothing for the tray monitor to do.
    //
    if (!infoCur().viiNewInfo().bAppCtrl()
    &&  !infoCur().viiNewInfo().biTunes()
    &&  !infoCur().viiNewInfo().bAnyServer())
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_UselessTrayMon, facCQCInst);
        widFailed = m_pwndInstall->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstACSrvPanel::Entered()
{
    m_pwndBinding->strWndText(infoCur().viiNewInfo().strTrayBinding());
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bTrayMon());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnTrayMon());

    m_pwndAppCtrl->SetCheckedState(infoCur().viiNewInfo().bAppCtrl());
    m_pwndiTunes->SetCheckedState(infoCur().viiNewInfo().biTunes());

    // Remember if task scheduling is available
    m_bAutoStartAvail = TKrnlTaskSched::bSupportAvail();

    // Auto-start may not be available
    if (m_bAutoStartAvail)
    {
        m_pwndAutoStart->SetEnable(kCIDLib::True);
        m_pwndAutoStart->SetCheckedState(infoCur().viiNewInfo().bTrayAutoStart());
    }
     else
    {
        m_pwndAutoStart->SetEnable(kCIDLib::False);
        m_pwndAutoStart->SetCheckedState(kCIDLib::False);
    }
}


tCIDLib::TVoid
TInstACSrvPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bTrayMon())
        fcolPorts[infoCur().viiNewInfo().ippnTrayMon()]++;
}


tCIDLib::TVoid TInstACSrvPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().strTrayBinding(m_pwndBinding->strWndText());
    infoCur().viiNewInfo().bTrayMon(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnTrayMon(wndParent().ippnExtractPort(*m_pwndPort));

    infoCur().viiNewInfo().bAppCtrl(m_pwndAppCtrl->bCheckedState());
    infoCur().viiNewInfo().biTunes(m_pwndiTunes->bCheckedState());

    if (m_bAutoStartAvail)
        infoCur().viiNewInfo().bTrayAutoStart(m_pwndAutoStart->bCheckedState());
    else
        infoCur().viiNewInfo().bTrayAutoStart(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TInstACSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstACSrvPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_TrayMon);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_AppCtrl, m_pwndAppCtrl);
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_AutoStart, m_pwndAutoStart);
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_Binding, m_pwndBinding);
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_iTunes, m_pwndiTunes);
    CastChildWnd(*this, kCQCInst::ridPan_TrayMon_Port, m_pwndPort);

    // Register a click handler for our enable/disable button
    m_pwndInstall->pnothRegisterHandler(this, &TInstACSrvPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstACSrvPanel: Private, non-virtual
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstACSrvPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_TrayMon_DoInstall)
    {
        // If enabled, and there's no binding name, default to local host
        if (m_pwndInstall->bCheckedState() && m_pwndBinding->bIsEmpty())
            m_pwndBinding->strWndText(TSysInfo::strIPHostName());
    }
    return tCIDCtrls::EEvResponses::Handled;
}

