//
// FILE NAME: CQCInst_IVCtrlPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 5/03/2008
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
//  This file implements the info panel that lets the user set the Intf.Viewer
//  remove control interface options.
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
RTTIDecls(TInstIVCtrlPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstIVCtrlPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstIVCtrlPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstIVCtrlPanel::TInstIVCtrlPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Interface Viewer Control Options", pinfoCur, pwndParent)
    , m_pwndPort(nullptr)
    , m_pwndInstall(nullptr)
{
}

TInstIVCtrlPanel::~TInstIVCtrlPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstIVCtrlPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstIVCtrlPanel::bPanelIsVisible() const
{
    // If the IV is being installed and not using previous options
    return  infoCur().viiNewInfo().bIntfViewer() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstIVCtrlPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstIVCtrlPanel::Entered()
{
    // Load the incoming info
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bIntfViewerCtrl());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnIVCtrl());
}


tCIDLib::TVoid TInstIVCtrlPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bIntfViewerCtrl())
        fcolPorts[infoCur().viiNewInfo().ippnIVCtrl()]++;
}


tCIDLib::TVoid TInstIVCtrlPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bIntfViewerCtrl(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnIVCtrl(wndParent().ippnExtractPort(*m_pwndPort));
}


// ---------------------------------------------------------------------------
//  TInstIVCtrlPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstIVCtrlPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_IVCtrl_Title));

    // Create our child widgets
    LoadDlgItems(kCQCInst::ridPan_IVCtrl);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_IVCtrl_CtrlPort, m_pwndPort);
    CastChildWnd(*this, kCQCInst::ridPan_IVCtrl_DoInstall, m_pwndInstall);

    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bIntfViewerCtrl());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnIVCtrl());

    return kCIDLib::True;
}

