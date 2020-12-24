//
// FILE NAME: CQCInst_StopSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/16/2005
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
//  This file implements the info panel that lets the user decide to back out
//  or to shut down the service and continue.
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
RTTIDecls(TInstStopSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstStopSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstStopSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstStopSrvPanel::TInstStopSrvPanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Stop Service", pinfoCur, pwndParent)
    , m_eStatus(EResults::Idle)
    , m_enctStart(0)
    , m_pwndProgress(nullptr)
    , m_thrRunner
      (
        TString(L"CQCInstStopSrvThread")
        , TMemberFunc<TInstStopSrvPanel>(this, &TInstStopSrvPanel::eRunThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TInstStopSrvPanel::~TInstStopSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstStopSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This page is only visible if there is an existing service and it is
//  running.
//
tCIDLib::TBoolean TInstStopSrvPanel::bPanelIsVisible() const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    // Check the app shell
    tCIDKernel::TWSvcHandle hsvcCQC = 0;
    hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszCQCShellName);
    if (hsvcCQC)
    {
        // It's there, so query the state then close the handle
        tCIDKernel::EWSrvStates eState;
        TKrnlWin32Service::bQueryState(hsvcCQC, eState);
        TKrnlWin32Service::bClose(hsvcCQC);

        // If it's not in stopped state, then we need to stop it
        bRet = (eState != tCIDKernel::EWSrvStates::Stopped);
    }

    // If not already returning true, check the client service
    if (!bRet)
    {
        hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszClServiceName);
        if (hsvcCQC)
        {
            // It's there, so query the state then close the handle
            tCIDKernel::EWSrvStates eState;
            TKrnlWin32Service::bQueryState(hsvcCQC, eState);
            TKrnlWin32Service::bClose(hsvcCQC);

            // If it's not in stopped state, then we need to stop it
            bRet = (eState != tCIDKernel::EWSrvStates::Stopped);
        }
    }
    return bRet;
}


tCIDLib::TBoolean
TInstStopSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    try
    {
        //
        //  Let's try to stop the service. We use a thread for this, and we
        //  have to disable all the main window's navigation buttons while we
        //  do it.
        //
        wndParent().SetNavFlags(tCQCInst::ENavFlags::None);

        //
        //  Set the waiting status, which will make the time start updating the
        //  progress bar.
        //
        m_eStatus = EResults::Waiting;

        // Store the start time, so the timer can keep the progress bar right
        m_enctStart = TTime::enctNow();

        // And start up the timer
        m_tmidUpdate = tmidStartTimer(500);

        // And start the runner thread that does the work
        m_thrRunner.Start();

        //
        //  Now we have to wait until the thread completes. The timer will
        //  force the status if the thread seems stuck.
        //
        while (m_eStatus == EResults::Waiting)
            facCIDCtrls().MsgYield(250);

        // Stop the timer
        StopTimer(m_tmidUpdate);

    }

    catch(TError& errToCatch)
    {
        StopTimer(m_tmidUpdate);
        facCQCInst.LogInstallMsg(L"Exception starting service shutdown process");
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Wait for the thread to completely shutdown
    try
    {
        if (m_thrRunner.bIsRunning())
            m_thrRunner.ReqShutdownSync();
        m_thrRunner.eWaitForDeath(10000);
    }

    catch(TError& errToCatch)
    {
        facCQCInst.LogInstallMsg(L"Exception stopping service shutdown thread");
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Re-enable the buttons
    wndParent().SetNavFlags(tCQCInst::ENavFlags::AllowAll);

    // If it failed, indicate that
    if (m_eStatus != EResults::Stopped)
    {
        facCQCInst.LogInstallMsg(L"Service could not be stopped");
        strErrText = facCQCInst.strMsg(kCQCInstErrs::errcFail_StopService);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TInstStopSrvPanel::Entered()
{
    // Zero our progress bar
    m_pwndProgress->SetValue(0);
}


// ---------------------------------------------------------------------------
//  TInstStopSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TInstStopSrvPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_StopSrv);

    // Get typed pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_StopSrv_Progress, m_pwndProgress);

    return kCIDLib::True;
}


tCIDLib::TVoid TInstStopSrvPanel::Destroyed()
{
    // Clean up our timer if we created it
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }
}


tCIDLib::TVoid TInstStopSrvPanel::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    static const tCIDLib::TCard8 c8FullRange = 60 * kCIDLib::enctOneSecond;

    //
    //  The thread goes for 60 seconds before giving up. So we want to move
    //  the progress bar so that it goes all the way in the max of 60 secs.
    //
    if (m_eStatus == EResults::Waiting)
    {
        // Calc the elapsed seconds so far
        const tCIDLib::TCard4 c4ElapsedSec = tCIDLib::TCard4
        (
            (TTime::enctNow() - m_enctStart) / kCIDLib::enctOneSecond
        );

        //
        //  If it's been 70 seconds, then something is wrong and the thread
        //  did not end right, so for us out.
        //
        if (c4ElapsedSec > 70)
        {
            facCQCInst.LogInstallMsg(L"Service shutdown thread stuck, forcing us out");
            m_eStatus = EResults::DoneWithError;
        }

        // Calc the percent of the 30 seconds, clip and set the bar
        const tCIDLib::TFloat8 f8Per = tCIDLib::TFloat8(c4ElapsedSec) / 30.0;
        tCIDLib::TCard4 c4Per = tCIDLib::TCard4(f8Per * 100.0);
        if (c4Per > 100)
            c4Per = 100;
        m_pwndProgress->SetValue(c4Per);
    }
}



// ---------------------------------------------------------------------------
//  TInstStopSrvPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The service class doesn't throw any CQC eerrors, since it's a CIDKernel level API,
//  but we'll handle any unknown system exception, though it's highly unlikely it'll
//  happen.
//
tCIDLib::EExitCodes TInstStopSrvPanel::eRunThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    EResults                eNewStatus = m_eStatus;
    tCIDKernel::TWSvcHandle hsvcCQC = 0;

    // Stop the client service
    try
    {
        hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszClServiceName);
        if (hsvcCQC)
        {
            if (TKrnlWin32Service::bStop(hsvcCQC, 60000))
                eNewStatus = EResults::Stopped;
            else
                eNewStatus = EResults::DoneWithError;
        }
        TKrnlWin32Service::bClose(hsvcCQC);
    }

    catch(...)
    {
        TKrnlWin32Service::bClose(hsvcCQC);
        facCQCInst.LogInstallMsg(L"Unknown exception in client service shutdown thread");
        eNewStatus = EResults::DoneWithError;
    }

    // Stop the app shell service if it's present
    try
    {
        hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszCQCShellName);
        if (hsvcCQC)
        {
            if (TKrnlWin32Service::bStop(hsvcCQC, 60000))
                eNewStatus = EResults::Stopped;
            else
                eNewStatus = EResults::DoneWithError;
        }
        TKrnlWin32Service::bClose(hsvcCQC);
    }

    catch(...)
    {
        TKrnlWin32Service::bClose(hsvcCQC);
        facCQCInst.LogInstallMsg(L"Unknown exception in app shell shutdown thread");
        eNewStatus = EResults::DoneWithError;
    }

    // Store the new status and return
    m_eStatus = eNewStatus;

    return tCIDLib::EExitCodes::Normal;
}

