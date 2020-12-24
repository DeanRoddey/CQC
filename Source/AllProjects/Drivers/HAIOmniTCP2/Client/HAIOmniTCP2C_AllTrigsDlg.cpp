//
// FILE NAME: HAIOmniTCP2C_AllTrigsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2014
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
#include    "HAIOmniTCP2C_.hpp"


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
TAllTrigsDlg::RunDlg(const TWindow& wndOwner, TOmniTCPDrvCfg& dcfgCurrent)
{
    // Save the incoming stuff for later use
    m_pdcfgCurrent = &dcfgCurrent;
    c4RunDlg(wndOwner, facHAIOmniTCP2C(), kHAIOmniTCP2C::ridDlg_AllTrigs);
}


// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAllTrigsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_AllTrigs_Close, m_pwndClose);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_AllTrigs_ClrAll, m_pwndClrAll);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_AllTrigs_SetAll, m_pwndSetAll);
    CastChildWnd(*this, kHAIOmniTCP2C::ridDlg_AllTrigs_Types, m_pwndTypes);

    // Load up the list with the available item types that could send triggers
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facHAIOmniTCP2C().strMsg(kOmniTCP2CMsgs::midDlg_AllTrigs_Type));
    m_pwndTypes->SetColumns(colCols);

    colCols[0] = L"Do Units";
    m_pwndTypes->c4AddItem(colCols, 0);
    colCols[0] = L"Do Zones";
    m_pwndTypes->c4AddItem(colCols, 1);

    // Register handlers now that we've set up initial values
    m_pwndClose->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);
    m_pwndClrAll->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);
    m_pwndSetAll->pnothRegisterHandler(this, &TAllTrigsDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TAllTrigsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TAllTrigsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_AllTrigs_ClrAll)
    ||  (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_AllTrigs_SetAll))
    {
        const tCIDLib::TCard4 c4Index = m_pwndTypes->c4CurItem();

        if (c4Index != kCIDLib::c4MaxCard)
        {
            tHAIOmniTCP2Sh::EItemTypes eType = tHAIOmniTCP2Sh::EItemTypes::Unknown;

            if (c4Index == 0)
                eType = tHAIOmniTCP2Sh::EItemTypes::Unit;
            else
                eType = tHAIOmniTCP2Sh::EItemTypes::Zone;

            m_pdcfgCurrent->SetAllTriggers
            (
                eType, (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_AllTrigs_SetAll)
            );
        }
    }
     else if (wnotEvent.widSource() == kHAIOmniTCP2C::ridDlg_AllTrigs_Close)
    {
        EndDlg(kHAIOmniTCP2C::ridDlg_AllTrigs_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

