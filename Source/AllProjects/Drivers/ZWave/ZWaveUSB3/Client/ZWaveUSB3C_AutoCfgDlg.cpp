//
// FILE NAME: ZWaveUSB3C_AutoCfgDlg.cpp
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
//  This file implements a small dialog that lets the user auto-config a unit.
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
RTTIDecls(TZWUSB3AutoCfgDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3AutoCfgDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3AutoCfgDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3AutoCfgDlg::TZWUSB3AutoCfgDlg() :

    m_punitiTar(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndCmds(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndIsAwake(nullptr)
    , m_pwndStart(nullptr)
{
}

TZWUSB3AutoCfgDlg::~TZWUSB3AutoCfgDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3AutoCfgDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TZWUSB3AutoCfgDlg::RunDlg(TZWaveUSB3CWnd& wndDriver, const TZWUnitInfo& unitiTar)
{
    // Save the driver and unit pointers for later use
    m_punitiTar = &unitiTar;
    m_pwndDriver = &wndDriver;

    c4RunDlg(wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_AutoCfg);
}


// ---------------------------------------------------------------------------
//  TZWUSB3AutoCfgDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3AutoCfgDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AutoCfg_Close, m_pwndClose);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AutoCfg_Cmds, m_pwndCmds);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AutoCfg_IsAwake, m_pwndIsAwake);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_AutoCfg_Start, m_pwndStart);

    // And register our button event handlers
    m_pwndClose->pnothRegisterHandler(this, &TZWUSB3AutoCfgDlg::eClickHandler);
    m_pwndStart->pnothRegisterHandler(this, &TZWUSB3AutoCfgDlg::eClickHandler);

    // Format out the commands that we are going to do
    TTextStringOutStream strmTar(4096UL);
    m_punitiTar->FormatAutoCfg(strmTar);
    m_pwndCmds->strWndText(strmTar.strData());

    // If the unit is always on, then check and disable the awake check box
    if (m_punitiTar->bAlwaysOn())
    {
        m_pwndIsAwake->SetCheckedState(kCIDLib::True);
        m_pwndIsAwake->SetEnable(kCIDLib::False);
    }
    return bRet;
}


// ---------------------------------------------------------------------------
//  TZWUSB3AutoCfgDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWUSB3AutoCfgDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_AutoCfg_Close)
    {
        EndDlg(kZWaveUSB3C::ridDlg_AutoCfg_Close);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_AutoCfg_Start)
    {
        try
        {
            m_pwndClose->SetEnable(kCIDLib::False);
            m_pwndStart->SetEnable(kCIDLib::False);
            tCIDLib::TBoolean bRes = kCIDLib::False;
            TZWDriverXData zwdxdComm;
            {
                zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_DoAutoCfg;
                zwdxdComm.m_c1UnitId = m_punitiTar->c1Id();
                zwdxdComm.m_bAwake = m_pwndIsAwake->bCheckedState();
                THeapBuf mbufComm(2048, 8192);

                TWndPtrJanitor janWait(tCIDCtrls::ESysPtrs::Wait);
                bRes = m_pwndDriver->bSendSrvMsg(*this, zwdxdComm, mbufComm);
            }

            if (bRes)
            {
                if (!zwdxdComm.m_strMsg.bIsEmpty())
                {
                    TOkBox msgbDone(zwdxdComm.m_strMsg);
                    msgbDone.ShowIt(*this);
                }
            }
             else
            {
                TErrBox msgbErr(zwdxdComm.m_strMsg);
                msgbErr.ShowIt(*this);
            }

            m_pwndClose->SetEnable(kCIDLib::True);
            m_pwndStart->SetEnable(kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgbFail
            (
                *this
                , strWndText()
                , facZWaveUSB3C().strMsg(kZWU3CErrs::errcCmd_AutoCfgFailed)
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

