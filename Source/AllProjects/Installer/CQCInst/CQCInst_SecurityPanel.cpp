//
// FILE NAME: CQCInst_SecurityPanel.cpp
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
//  This file implements the info panel that shows the user a reminder about
//  security concerns.
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
RTTIDecls(TInstSecurityPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstSecurityPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstSecurityPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstSecurityPanel::TInstSecurityPanel( TCQCInstallInfo* const  pinfoCur
                                        , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Summary", pinfoCur, pwndParent)
    , m_pwndText(nullptr)
{
}

TInstSecurityPanel::~TInstSecurityPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstSecurityPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstSecurityPanel::bCreated()
{
    // Load up our controls
    LoadDlgItems(kCQCInst::ridPan_Security);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Secure_Text, m_pwndText);

    return kCIDLib::True;
}


tCIDLib::TBoolean TInstSecurityPanel::bPanelIsVisible() const
{
    // We are only visible if the install succeeded
    return infoCur().eInstStatus() == tCQCInst::EInstStatus::Done;
}

