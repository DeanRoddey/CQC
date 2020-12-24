//
// FILE NAME: ZWaveUSB3C_AssocDrvDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2018
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
//  This file implements a small dialog that lets the user easily set our driver
//  id into the common group 1 of the target unit.
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
RTTIDecls(TZWUSB3AssocToDrvDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3AssocToDrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3AssocToDrvDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3AssocToDrvDlg::TZWUSB3AssocToDrvDlg() :

    m_punitiTar(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndIsAwake(nullptr)
    , m_pwndSet(nullptr)
{
}

TZWUSB3AssocToDrvDlg::~TZWUSB3AssocToDrvDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3AssocToDrvDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3AssocToDrvDlg::RunDlg(       TZWaveUSB3CWnd& wndDriver
                            , const TZWUnitInfo&    unitiTar)
{
    // Save pointers to the incoming for later use
    m_punitiTar = &unitiTar;
    m_pwndDriver = &wndDriver;

    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_AssocDrvDlg);
}


// ---------------------------------------------------------------------------
//  TZWUSB3AssocToDrvDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3AssocToDrvDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AssocDrv_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AssocDrv_IsAwake, m_pwndIsAwake);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AssocDrv_Set, m_pwndSet);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TZWUSB3AssocToDrvDlg::eClickHandler);
    m_pwndSet->pnothRegisterHandler(this, &TZWUSB3AssocToDrvDlg::eClickHandler);

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
//  TZWUSB3AssocToDrvDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when the user invokes any action. We make sure that the unit is a listener
//  or they have told it is awake. If not, we warn them.
//
tCIDLib::TBoolean TZWUSB3AssocToDrvDlg::bCheckAwake() const
{
    if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
    {
        TYesNoBox msgbAck(m_strTitle, facZWaveUSB3C().strMsg(kZWU3CMsgs::midQ_MsgsWillBeQd));
        return msgbAck.bShowIt(*this);
    }
    return kCIDLib::True;
}


// Handle button presses
tCIDCtrls::EEvResponses TZWUSB3AssocToDrvDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_AssocDrv_Cancel)
    {
        EndDlg(kZWaveUSB3C::ridDlg_AssocDrv_Cancel);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_AssocDrv_Set)
    {
        //
        //  Check the awake situation. They may not mean to do this if the unit is
        //  not currently awake.
        //
        if (!bCheckAwake())
            return tCIDCtrls::EEvResponses::Handled;

        TZWDriverXData zwdxdComm;
        zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_AssocToDrv;
        zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
        zwdxdComm.m_c1Val1 = 1;
        zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
        THeapBuf mbufComm(512, 1024);

        // Whether it works or not we clear the list, if only to reload it
        TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
        if (m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm))
        {
            // If the command was queued, warn about that
            if (!m_punitiTar->bAlwaysOn() && !m_pwndIsAwake->bCheckedState())
            {
                TOkBox msgbWarn
                (
                    m_strTitle, facZWaveUSB3C().strMsg(kZWU3CMsgs::midWarn_OpPending)
                );
                msgbWarn.ShowIt(*this);
            }
            EndDlg(kZWaveUSB3C::ridDlg_AssocDrv_Set);
        }
         else
        {
            TErrBox msgbErr(m_strTitle, zwdxdComm.m_strMsg);
            msgbErr.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

