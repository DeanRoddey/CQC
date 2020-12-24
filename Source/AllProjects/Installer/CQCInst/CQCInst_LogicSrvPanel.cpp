//
// FILE NAME: CQCInst_LogicSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/06/2009
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
//  This file implements the info panel that lets the user set the Logic
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
RTTIDecls(TInstLogicSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstLogicSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstLogicSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstLogicSrvPanel::TInstLogicSrvPanel( TCQCInstallInfo* const  pinfoCur
                                        , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Logic Server Options", pinfoCur, pwndParent)
    , m_pwndInstall(nullptr)
    , m_pwndPort(nullptr)
{
}

TInstLogicSrvPanel::~TInstLogicSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstLogicSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstLogicSrvPanel::bPanelIsVisible() const
{
    // If the logic server is being installed and not using previous opts
    return infoCur().viiNewInfo().bLogicServer() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstLogicSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Make sure the port is valid
    if (!wndParent().bValidatePort(*m_pwndPort, strErrText))
    {
        widFailed = m_pwndPort->widThis();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstLogicSrvPanel::Entered()
{
    // Load up in incoming info
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bLogicServer());
    wndParent().LoadPortOpts(*this, *m_pwndPort, infoCur().viiNewInfo().ippnLogicServer());
}


tCIDLib::TVoid
TInstLogicSrvPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bLogicServer())
        fcolPorts[infoCur().viiNewInfo().ippnLogicServer()]++;
}


//
//  We react to the master server option being enabled or disabled. If the
//  master server is not being installed, we disable ourself (and we'll report
//  above that we shouldn't be shown.)
//
tCIDLib::TVoid TInstLogicSrvPanel::ReactToChanges()
{
    if (!infoCur().viiNewInfo().bMasterServer())
    {
        infoCur().viiNewInfo().bLogicServer(kCIDLib::False);
        m_pwndInstall->SetCheckedState(kCIDLib::False);
        m_pwndInstall->SetEnable(kCIDLib::False);
    }
     else
    {
        m_pwndInstall->SetEnable(kCIDLib::True);
    }
}


tCIDLib::TVoid TInstLogicSrvPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bLogicServer(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnLogicServer(wndParent().ippnExtractPort(*m_pwndPort));
}


// ---------------------------------------------------------------------------
//  TInstLogicSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstLogicSrvPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_LogicSrv_Title));

    // Create our controls
    LoadDlgItems(kCQCInst::ridPan_LogicSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_LogicSrv_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_LogicSrv_Port, m_pwndPort);

    return kCIDLib::True;
}

