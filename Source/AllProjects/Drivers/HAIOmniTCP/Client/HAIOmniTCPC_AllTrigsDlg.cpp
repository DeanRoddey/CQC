//
// FILE NAME: HAIOmniTCPC_AllTrigsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/14/2013
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
//  This file implements a small dialog that allows the user to set or clear
//  the 'send trigger' setting on all items of given types at once.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TAllTrigsDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TAllTrigsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TAllTrigsDlg::TAllTrigsDlg() :

    m_pdcfgCurrent(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndClrAll(nullptr)
    , m_pwndSetAll(nullptr)
    , m_pwndTypes(nullptr)
{
}

TAllTrigsDlg::~TAllTrigsDlg()
{
}


// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TAllTrigsDlg::Run(const TWindow& wndOwner, TOmniTCPDrvCfg& dcfgCurrent)
{
    // Save the incoming stuff for later use
    m_pdcfgCurrent = &dcfgCurrent;

    c4RunDlg(wndOwner, facHAIOmniTCPC(), kHAIOmniTCPC::ridDlg_AllTrigs);
}


// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAllTrigsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_AllTrigs_Close, m_pwndClose);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_AllTrigs_ClrAll, m_pwndClrAll);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_AllTrigs_SetAll, m_pwndSetAll);
    CastChildWnd(*this, kHAIOmniTCPC::ridDlg_AllTrigs_Types, m_pwndTypes);

    // Load up the list with the available item types that could send triggers
    m_pwndTypes->c4AddItem(L"Do Units", 0);
    m_pwndTypes->c4AddItem(L"Do Zones", 1);

    // Register handlers now that we've set up initial values
    m_pwndClose->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);
    m_pwndClrAll->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);
    m_pwndSetAll->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);

    return bRes;
}


// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TAllTrigsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_AllTrigs_ClrAll)
    ||  (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_AllTrigs_SetAll))
    {
        const tCIDLib::TCard4 c4ListInd = m_pwndTypes->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            tHAIOmniTCPSh::EItemTypes eType = tHAIOmniTCPSh::EItem_Unknown;
            if (m_pwndTypes->c4IndexToId(c4ListInd) == 0)
                eType = tHAIOmniTCPSh::EItem_Unit;
            else if (m_pwndTypes->c4IndexToId(c4ListInd) == 1)
                eType = tHAIOmniTCPSh::EItem_Zone;

            m_pdcfgCurrent->SetAllTriggers
            (
                eType, (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_AllTrigs_SetAll)
            );
        }
    }
     else if (wnotEvent.widSource() == kHAIOmniTCPC::ridDlg_AllTrigs_Close)
    {
        EndDlg(kHAIOmniTCPC::ridDlg_AllTrigs_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

