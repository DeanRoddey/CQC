//
// FILE NAME: CQCInst_SrvModePanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/22/2009
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
//  This file implements the info panel that lets the user set the service
//  mode.
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
RTTIDecls(TInstSrvModePanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstSrvModePanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstSrvModePanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstSrvModePanel::TInstSrvModePanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Service Mode", pinfoCur, pwndParent)
    , m_pwndAutomatic(nullptr)
    , m_pwndAutoDelayed(nullptr)
    , m_pwndDisabled(nullptr)
    , m_pwndOnDemand(nullptr)
{
}

TInstSrvModePanel::~TInstSrvModePanel()
{
}


// ---------------------------------------------------------------------------
//  TInstSrvModePanel: Public, inherited methods
// ---------------------------------------------------------------------------

// If we need the app shell, this one is visible
tCIDLib::TBoolean TInstSrvModePanel::bPanelIsVisible() const
{
    return infoCur().bNeedsAppShell();
}


tCIDLib::TVoid TInstSrvModePanel::Entered()
{
}


tCIDLib::TVoid TInstSrvModePanel::SaveContents()
{
    // Store the mode in the facility
    if (m_pwndDisabled->bCheckedState())
        facCQCInst.eStartMode(tCIDKernel::EWSrvStarts::Disabled);
    else if (m_pwndOnDemand->bCheckedState())
        facCQCInst.eStartMode(tCIDKernel::EWSrvStarts::Demand);
    else if (m_pwndAutomatic->bCheckedState())
        facCQCInst.eStartMode(tCIDKernel::EWSrvStarts::Auto);
    else
        facCQCInst.eStartMode(tCIDKernel::EWSrvStarts::Delayed);
}


// ---------------------------------------------------------------------------
//  TInstSrvModePanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstSrvModePanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_SrvMode);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_SrvMode_Automatic, m_pwndAutomatic);
    CastChildWnd(*this, kCQCInst::ridPan_SrvMode_AutoDelayed, m_pwndAutoDelayed);
    CastChildWnd(*this, kCQCInst::ridPan_SrvMode_Disabled, m_pwndDisabled);
    CastChildWnd(*this, kCQCInst::ridPan_SrvMode_OnDemand, m_pwndOnDemand);

    // Get the current startup mode and check the appropriate one
    tCIDKernel::TWSvcHandle hsvcCQC = 0;
    hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszCQCShellName);
    if(hsvcCQC)
    {
        tCIDKernel::EWSrvStarts eStart;
        if(TKrnlWin32Service::bQueryStartType(hsvcCQC,eStart))
        {
            if(eStart == tCIDKernel::EWSrvStarts::Disabled)
                m_pwndDisabled->SetCheckedState(kCIDLib::True);
            else if(eStart == tCIDKernel::EWSrvStarts::Demand)
                m_pwndOnDemand->SetCheckedState(kCIDLib::True);
            else if(eStart == tCIDKernel::EWSrvStarts::Auto)
                m_pwndAutomatic->SetCheckedState(kCIDLib::True);
            else
                m_pwndAutoDelayed->SetCheckedState(kCIDLib::True);

            // Store this in the facility
            facCQCInst.eStartMode(eStart);
        }
        TKrnlWin32Service::bClose(hsvcCQC);
    }
     else
    {
        // Default to delayed
        m_pwndAutoDelayed->SetCheckedState(kCIDLib::True);
    }

    return kCIDLib::True;
}
