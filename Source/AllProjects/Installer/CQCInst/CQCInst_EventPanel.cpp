//
// FILE NAME: CQCInst_EventPanel.cpp
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
//  This file implements the info panel that lets the user set the Events
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
RTTIDecls(TInstEventPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstEventPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstEventPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstEventPanel::TInstEventPanel(TCQCInstallInfo* const pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Event Server Options", pinfoCur, pwndParent)
    , m_pwndInstall(nullptr)
    , m_pwndPort(nullptr)
{
}

TInstEventPanel::~TInstEventPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstEventPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstEventPanel::bPanelIsVisible() const
{
    // If the event server is being installed and not using previous opts
    return infoCur().viiNewInfo().bEventServer() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstEventPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstEventPanel::Entered()
{
    // Load the incoming info
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bEventServer());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnEventServer());
}


//
//  We react to the master server option being enabled or disabled. If the
//  master server is not being installed, we disable ourself (and we'll report
//  above that we shouldn't be shown.)
//
tCIDLib::TVoid TInstEventPanel::ReactToChanges()
{
    if (!infoCur().viiNewInfo().bMasterServer())
    {
        infoCur().viiNewInfo().bEventServer(kCIDLib::False);
        m_pwndInstall->SetCheckedState(kCIDLib::False);
        m_pwndInstall->SetEnable(kCIDLib::False);
    }
     else
    {
        m_pwndInstall->SetEnable(kCIDLib::True);
    }
}


tCIDLib::TVoid TInstEventPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bEventServer())
        fcolPorts[infoCur().viiNewInfo().ippnEventServer()]++;
}


tCIDLib::TVoid TInstEventPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bEventServer(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnEventServer(wndParent().ippnExtractPort(*m_pwndPort));
}


// ---------------------------------------------------------------------------
//  TInstEventPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstEventPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_EventSrv_Title));

    // Create our child controls
    LoadDlgItems(kCQCInst::ridPan_EventSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_EventSrv_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_EventSrv_Port, m_pwndPort);

    return kCIDLib::True;
}
