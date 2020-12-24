//
// FILE NAME: CQCInst_CQCSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2004
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
//  This file implements the info panel that lets the user set the CQCServer
//  options.
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
RTTIDecls(TInstCQCSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstCQCSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstCQCSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstCQCSrvPanel::TInstCQCSrvPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"CQCServer Options", pinfoCur, pwndParent)
    , m_pwndInstall(nullptr)
    , m_pwndPort(nullptr)
{
}

TInstCQCSrvPanel::~TInstCQCSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstCQCSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCQCSrvPanel::bPanelIsVisible() const
{
    // We are only visible if device control is being installed and not using previous opts
    return infoCur().viiNewInfo().bCQCServer() &&  !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstCQCSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstCQCSrvPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bCQCServer())
        fcolPorts[infoCur().viiNewInfo().ippnCQCServer()]++;
}


tCIDLib::TVoid TInstCQCSrvPanel::Entered()
{
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bCQCServer());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnCQCServer());
}


tCIDLib::TVoid TInstCQCSrvPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bCQCServer(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnCQCServer(wndParent().ippnExtractPort(*m_pwndPort));
}


// ---------------------------------------------------------------------------
//  TInstCQCSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCQCSrvPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_CQCSrv_Title));

    // Create our child widgets
    LoadDlgItems(kCQCInst::ridPan_CQCSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_CQCSrv_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_CQCSrv_Port, m_pwndPort);

    return kCIDLib::True;
}
