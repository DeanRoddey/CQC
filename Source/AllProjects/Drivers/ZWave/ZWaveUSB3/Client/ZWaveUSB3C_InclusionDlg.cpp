//
// FILE NAME: ZWaveUSB3C_InclusionDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/10/2018
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
//  This file implements a small dialog that lets the user force our driver to do a
//  replication process.
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
RTTIDecls(TZWUSB3IncludeDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWUSB3IncludeDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWUSB3IncludeDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWUSB3IncludeDlg::TZWUSB3IncludeDlg() :

    m_eLastRepState(tZWaveUSB3Sh::EDrvStates::Count)
    , m_pwndWait(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndState(nullptr)
    , m_pwndStart(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TZWUSB3IncludeDlg::~TZWUSB3IncludeDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWUSB3IncludeDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3IncludeDlg::bRunDlg(TZWaveUSB3CWnd& wndDriver)
{
    // Save the driver pointer for later use
    m_pwndDriver = &wndDriver;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndDriver, facZWaveUSB3C(), kZWaveUSB3C::ridDlg_Include
    );
    return (c4Res == kZWaveUSB3C::ridDlg_Include_Start);
}


// ---------------------------------------------------------------------------
//  TZWUSB3IncludeDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWUSB3IncludeDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Include_Cancel, m_pwndCancel);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Include_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Include_State, m_pwndState);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Include_Start, m_pwndStart);
    CastChildWnd(*this, kZWaveUSB3C::ridDlg_Include_Wait, m_pwndWait);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TZWUSB3IncludeDlg::eClickHandler);
    m_pwndStart->pnothRegisterHandler(this, &TZWUSB3IncludeDlg::eClickHandler);

    return bRet;
}


tCIDLib::TVoid TZWUSB3IncludeDlg::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    // Call our parent last
    TParent::Destroyed();
}


tCIDLib::TVoid TZWUSB3IncludeDlg::Timer(const tCIDCtrls::TTimerId  tmidSrc)
{
    // If not ours, pass it on and return
    if (tmidSrc != m_tmidUpdate)
    {
        TParent::Timer(tmidSrc);
        return;
    }

    tZWaveUSB3Sh::EDrvStates eRepState = eGetRepState();
    m_pwndState->strWndText(tZWaveUSB3Sh::strLoadEDrvStates(eRepState));

    // If the state changed, then let's deal with that
    if (eRepState != m_eLastRepState)
    {
        m_eLastRepState = eRepState;

        //
        //  Pause the timer in case we do a dialog box. Otherwise we could get layered
        //  dialogs potentially.
        //
        PauseTimer(m_tmidUpdate, kCIDLib::True);

        //
        //  If we failed to get status, we give up. If we get to success or failure
        //  state, then we are done.
        //
        tCIDLib::TBoolean bStopTimer = kCIDLib::False;
        if (eRepState == tZWaveUSB3Sh::EDrvStates::Count)
        {
            m_pwndInstruct->strWndText
            (
                L"Connection to the driver was lost. The status of the operations is "
                L"unknown. You should cancel the operation on the master, and try again "
                L"when the driver is available."
            );
            bStopTimer = kCIDLib::True;
        }
         else if (eRepState == tZWaveUSB3Sh::EDrvStates::RepFailed)
        {
            m_pwndInstruct->strWndText
            (
                L"The operation appears to have failed. The master controller may provide "
                L"more information. You can attempt the process again, since sometimes it "
                L"can fail for temporary reasons."
            );
            bStopTimer = kCIDLib::True;
        }
         else if (eRepState == tZWaveUSB3Sh::EDrvStates::RepSucceeded)
        {
            m_pwndInstruct->strWndText
            (
                L"The operation appears to have succeeded. If so the driver will now "
                L"perform some followup work to reconcile previous configuration with "
                L"what it just received. The driver will become available again when that "
                L"process has completed.  Individual units may take a little longer to "
                L"re-evaluate their status."
            );
            bStopTimer = kCIDLib::True;
        }

        //
        //  If we are done, then stop the timer, else resume it. If done, disable
        //  the cancel button, enable the start button but change the text to done.
        //  And we stop the activity bar.
        //
        if (bStopTimer)
        {
            StopTimer(m_tmidUpdate);

            m_pwndWait->StartStop(kCIDLib::False);
            if (m_pwndCancel->bHasFocus())
                m_pwndStart->TakeFocus();
            m_pwndCancel->SetEnable(kCIDLib::False);
            m_pwndStart->SetEnable(kCIDLib::True);
            m_pwndStart->strWndText(facZWaveUSB3C().strMsg(kZWU3CMsgs::midDlg_Done));
        }
        else
        {
            PauseTimer(m_tmidUpdate, kCIDLib::False);
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWUSB3IncludeDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWUSB3IncludeDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Include_Cancel)
    {
        // Stop the timer if we started it
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;

            // This means we actually were trying to do it
            TOkBox msgbCan
            (
                L"The operation is being cancelled, but that only means that this client "
                L"interface is no longer waiting for the operation to complete. We cannot "
                L"stop the actual operation. If it has not completed or seems to be stuck "
                L"you will need to cancel it via the master controller."
            );
            msgbCan.ShowIt(*this);
        }

        EndDlg(kZWaveUSB3C::ridDlg_Include_Cancel);
    }
     else if (wnotEvent.widSource() == kZWaveUSB3C::ridDlg_Include_Start)
    {
        //
        //  If the rep status is already set, then this is the final close. Else
        //  we need to start the process.
        //
        if (m_eLastRepState == tZWaveUSB3Sh::EDrvStates::Count)
        {
            try
            {
                //
                //  Let's try to start the process. If it starts, start the activity bar
                //  and disable start button, and we start a timer to watch for the
                //  results and keep our status updated.
                //
                m_zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_Replicate;
                if (m_pwndDriver->bSendSrvMsg(*this, m_zwdxdComm, m_mbufComm))
                {
                    if (m_pwndStart->bHasFocus())
                        m_pwndCancel->TakeFocus();
                    m_pwndStart->SetEnable(kCIDLib::False);
                    m_pwndWait->StartStop(kCIDLib::True);

                    m_eLastRepState = tZWaveUSB3Sh::EDrvStates::Count;
                    m_tmidUpdate = tmidStartTimer(500);
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TExceptDlg dlgbFail
                (
                    *this
                    , strWndText()
                    , facZWaveUSB3C().strMsg(kZWU3CErrs::errcCmd_IncludeFailed)
                    , errToCatch
                );
            }
        }
         else
        {
            EndDlg(kZWaveUSB3C::ridDlg_Include_Start);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  A helper to poll the driver for current replication state. We need it in two places
//  above so we break it out.
//
tZWaveUSB3Sh::EDrvStates TZWUSB3IncludeDlg::eGetRepState()
{
    tZWaveUSB3Sh::EDrvStates eRepState = tZWaveUSB3Sh::EDrvStates::Count;
    try
    {
        // Ask the driver for basic status
        m_zwdxdComm.Reset();
        m_zwdxdComm.m_strCmd = kZWaveUSB3Sh::strSendData_QueryRepStatus;
        if (m_pwndDriver->bSendSrvMsg(*this, m_zwdxdComm, m_mbufComm))
            eRepState = tZWaveUSB3Sh::EDrvStates(m_zwdxdComm.m_c4Val1);
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3C().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Fall through with default return
    }
    return eRepState;
}
