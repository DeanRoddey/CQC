//
// FILE NAME: CQCInst_IntroPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2016
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
//  This file implements an initial introductory panel that is only shown if this is a
//  new installation. It helps a new user indicate some basic options so that we can default
//  the components to install roughly for them.
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
RTTIDecls(TInstIntroPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstIntroPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstIntroPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstIntroPanel::TInstIntroPanel(TCQCInstallInfo* const pinfoCur
                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Intro", pinfoCur, pwndParent)
    , m_pwndDevices(nullptr)
    , m_pwndMaster(nullptr)
    , m_pwndNonMaster(nullptr)
{
}

TInstIntroPanel::~TInstIntroPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstIntroPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstIntroPanel::bPanelIsVisible() const
{
    // We are only visible if doing a clean (new) install
    return (infoCur().eTargetType() == tCQCInst::ETargetTypes::Clean);
}


tCIDLib::TBoolean
TInstIntroPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    return kCIDLib::True;
}


tCIDLib::TVoid TInstIntroPanel::SaveContents()
{
    //
    //  We go through the component flags of the new install info, and set them up based on
    //  the flags.
    //
    TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    // Set them all, then we'll turn off some if needed
    viiNew.EnableAll();

    // This will turn off anything that can only be installed on the MS
    if (!m_pwndMaster->bCheckedState())
        viiNew.bMasterServer(kCIDLib::False);

    // If no device support, disable that
    if (!m_pwndDevices->bCheckedState())
        viiNew.bCQCServer(kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TInstIntroPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstIntroPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Intro_Title));

    // Load up our child controls
    LoadDlgItems(kCQCInst::ridPan_Intro);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Intro_Devices, m_pwndDevices);
    CastChildWnd(*this, kCQCInst::ridPan_Intro_Master, m_pwndMaster);
    CastChildWnd(*this, kCQCInst::ridPan_Intro_NonMaster, m_pwndNonMaster);

    // Get one of the radio buttons initially selected
    m_pwndMaster->SetCheckedState(kCIDLib::True);

    // And default to installing device support
    m_pwndDevices->SetCheckedState(kCIDLib::True);

    return kCIDLib::True;
}

