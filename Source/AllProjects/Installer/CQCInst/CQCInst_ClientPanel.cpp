//
// FILE NAME: CQCInst_ClientPanel.cpp
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
//  This file implements the info panel that lets the user select which
//  client tools to install.
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
RTTIDecls(TInstClientPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstClientPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstClientPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstClientPanel::TInstClientPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Client Options", pinfoCur, pwndParent)
    , m_pwndAdmin(nullptr)
    , m_pwndAllOff(nullptr)
    , m_pwndAllOn(nullptr)
    , m_pwndCmdPrompt(nullptr)
    , m_pwndIntfView(nullptr)
    , m_pwndMediaRepoMgr(nullptr)
{
}

TInstClientPanel::~TInstClientPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstClientPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstClientPanel::bPanelIsVisible() const
{
    // We are only visible if installing the client tools and not using previous opts
    return infoCur().viiNewInfo().bCQCClient() && !infoCur().bUsePrevOpts();
}


tCIDLib::TVoid TInstClientPanel::Entered()
{
    // Set all the check boxes based on incoming data
    TCQCInstallInfo& infoToLoad = infoCur();
    m_pwndAdmin->SetCheckedState(infoToLoad.viiNewInfo().bCQCClient());
    m_pwndCmdPrompt->SetCheckedState(infoToLoad.viiNewInfo().bCmdPrompt());
    m_pwndIntfView->SetCheckedState(infoToLoad.viiNewInfo().bIntfViewer());
    m_pwndMediaRepoMgr->SetCheckedState(infoToLoad.viiNewInfo().bMediaRepoMgr());
}


tCIDLib::TVoid TInstClientPanel::SaveContents()
{
    // Store all the boolean flags
    TCQCInstallInfo& infoStoreIn = infoCur();
    infoStoreIn.viiNewInfo().bCQCClient(m_pwndAdmin->bCheckedState());
    infoStoreIn.viiNewInfo().bCmdPrompt(m_pwndCmdPrompt->bCheckedState());
    infoStoreIn.viiNewInfo().bIntfViewer(m_pwndIntfView->bCheckedState());
    infoStoreIn.viiNewInfo().bMediaRepoMgr(m_pwndMediaRepoMgr->bCheckedState());
}



// ---------------------------------------------------------------------------
//  TInstClientPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstClientPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Client_Title));

    // Create the child controls
    LoadDlgItems(kCQCInst::ridPan_Client);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Client_AllOff, m_pwndAllOff);
    CastChildWnd(*this, kCQCInst::ridPan_Client_AllOn, m_pwndAllOn);
    CastChildWnd(*this, kCQCInst::ridPan_Client_DoAdmin, m_pwndAdmin);
    CastChildWnd(*this, kCQCInst::ridPan_Client_DoCmdPrompt, m_pwndCmdPrompt);
    CastChildWnd(*this, kCQCInst::ridPan_Client_DoIntfView, m_pwndIntfView);
    CastChildWnd(*this, kCQCInst::ridPan_Client_DoMediaRepo, m_pwndMediaRepoMgr);

    // Register click handlers on the main frame for our buttons
    m_pwndAllOff->pnothRegisterHandler(this, &TInstClientPanel::eClickHandler);
    m_pwndAllOn->pnothRegisterHandler(this, &TInstClientPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstClientPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstClientPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    const tCIDLib::TBoolean bNewState(wnotEvent.widSource() == kCQCInst::ridPan_Client_AllOn);
    m_pwndAdmin->SetCheckedState(bNewState);
    m_pwndCmdPrompt->SetCheckedState(bNewState);
    m_pwndIntfView->SetCheckedState(bNewState);
    m_pwndMediaRepoMgr->SetCheckedState(bNewState);

    return tCIDCtrls::EEvResponses::Handled;
}
