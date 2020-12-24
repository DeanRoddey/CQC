//
// FILE NAME: ZWaveUSB3C_ResetUnitDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/22/2018
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
//  This file implements a small dialog that lets the user reset a unit.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWUSB3ResetUnitDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3ResetUnitDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3ResetUnitDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3ResetUnitDlg::TZWUSB3ResetUnitDlg() :

    m_punitiTar(nullptr)
    , m_pwndAccept(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndIsAwake(nullptr)
{
}

TZWUSB3ResetUnitDlg::~TZWUSB3ResetUnitDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3ResetUnitDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3ResetUnitDlg::RunDlg(TZWaveUSB3CWnd& wndDriver, const TZWUnitInfo& unitiTar)
{
    // Save pointers to the incoming for later use
    m_punitiTar = &unitiTar;
    m_pwndDriver = &wndDriver;

    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_ResetUnit);
}


// ---------------------------------------------------------------------------
//  TZWUSB3ResetUnitDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3ResetUnitDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_ResetUnit_Accept, m_pwndAccept);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_ResetUnit_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_ResetUnit_IsAwake, m_pwndIsAwake);

    // Register our button event handlers
    m_pwndAccept->pnothRegisterHandler(this, &TZWUSB3ResetUnitDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TZWUSB3ResetUnitDlg::eClickHandler);

    // If the unit is always on, then check and disable the awake check box
    if (m_punitiTar->bAlwaysOn())
    {
        m_pwndIsAwake->SetCheckedState(kCIDLib::True);
        m_pwndIsAwake->SetEnable(kCIDLib::False);
    }

    //
    //  Update the title text to include the unit name. Save the original text for
    //  use in popups.
    //
    m_strTitle = strWndText();
    TString strTitle = m_strTitle;
    strTitle.Append(L" [");
    strTitle.Append(m_punitiTar->strName());
    strTitle.Append(L"]");
    strWndText(strTitle);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWUSB3ResetUnitDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3ResetUnitDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_ResetUnit_Accept)
    {
        TZWDriverXData zwdxdComm;
        zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_ForceRescan;
        zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
        zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
        THeapBuf mbufComm(2048, 8192);

        TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
        if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
        {
            TOkBox msgbDone(m_strTitle, L"The reset was begun successfully");
            msgbDone.ShowIt(*this);

            EndDlg(kZWaveUSB3C::ridDlg_ResetUnit_Accept);
        }
         else
        {
            TErrBox msgbErr(m_strTitle, zwdxdComm.m_strMsg);
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_ResetUnit_Cancel)
    {
        EndDlg(kZWaveUSB3C::ridDlg_ResetUnit_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}
