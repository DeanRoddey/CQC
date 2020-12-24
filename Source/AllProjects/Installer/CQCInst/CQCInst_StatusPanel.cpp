//
// FILE NAME: CQCInst_StatusPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/02/2004
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
//  This file implements the info panel that shows the status of the install process.
//  We monitor posted notifications from the bgn thread and display the status that
//  they contain.
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
RTTIDecls(TInstStatusPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstStatusPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstStatusPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstStatusPanel::TInstStatusPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Install Status", pinfoCur, pwndParent)
    , m_colStatusQ(tCIDLib::EMTStates::Safe)
    , m_eStep(tCQCInst::EInstSteps::Count)
    , m_pwndMeter(nullptr)
    , m_pwndText(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TInstStatusPanel::~TInstStatusPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstStatusPanel: Public, inherited methods
// ---------------------------------------------------------------------------

// We don't allow navigation off this panel during the installation
tCQCInst::ENavFlags TInstStatusPanel::eNavFlags() const
{
    return tCQCInst::ENavFlags::None;
}


tCQCInst::EProcRes TInstStatusPanel::eProcess()
{
    // Set the current install status to show in progress
    infoCur().eInstStatus(tCQCInst::EInstStatus::InProgress);

    // And kick off the installer thread
    try
    {
        m_eStep = tCQCInst::EInstSteps::Start;
        facCQCInst.LogInstallMsg(L"Invoking installer thread");
        m_thrInstaller.StartInstall(&wndParent(), this, &infoCur());
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        facCQCInst.LogInstallMsg(L"CIDLib exception thrown when starting thread");
        infoCur().eInstStatus(tCQCInst::EInstStatus::Failed);
        return tCQCInst::EProcRes::Next;
    }

    catch(...)
    {
        facCQCInst.LogInstallMsg(L"Exception thrown when starting thread");
        infoCur().eInstStatus(tCQCInst::EInstStatus::Failed);
        return tCQCInst::EProcRes::Next;
    }

    //
    //  Now we just wait here until the install completes. The timer will keep updating
    //  the step as the install thread posts messages to us.
    //
    while (m_eStep < tCQCInst::EInstSteps::Complete)
        facCIDCtrls().MsgYield(250);

    // Stop the timer
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    //
    //  And wait for the install thread to stop. He will set the install status to
    //  indicate the success or failure.
    //
    WaitInstallThread();

    // Indicate we want to move forward immediately
    return tCQCInst::EProcRes::Next;
}


tCIDLib::TVoid TInstStatusPanel::Entered()
{
    // Make sure we reset the meter back to normal mode if we failed before
    m_pwndMeter->SetNormal();
    m_pwndMeter->SetValue(0);

    // Start our timer
    m_tmidUpdate = tmidStartTimer(250);
}



// ---------------------------------------------------------------------------
//  TInstStatusPanel: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TInstStatusPanel::EnterFailureMode()
{
    m_pwndMeter->SetError();
}


//
//  The installer thread calls this to queue up status packets on the queue that our
//  timer processes. Currently the only status packet is one that indicates the step
//  of the process about to be started.
//
tCIDLib::TVoid
TInstStatusPanel::PostEvent(const tCQCInst::EInstSteps eStep)
{
    // Log a message to the installation log
    switch(eStep)
    {
        case tCQCInst::EInstSteps::Start :
            facCQCInst.LogInstallMsg(L"Starting installation");
            break;

        case tCQCInst::EInstSteps::CreateTmpDir :
            facCQCInst.LogInstallMsg(L"Creating temporary out directory");
            break;

        case tCQCInst::EInstSteps::CreateNewDirs :
            facCQCInst.LogInstallMsg(L"Creating new output directories");
            break;

        case tCQCInst::EInstSteps::CopyBase :
            facCQCInst.LogInstallMsg(L"Copying base files");
            break;

        case tCQCInst::EInstSteps::CopyClient :
            facCQCInst.LogInstallMsg(L"Copying client files");
            break;

        case tCQCInst::EInstSteps::CopyCQCServer :
            facCQCInst.LogInstallMsg(L"Copying server files");
            break;

        case tCQCInst::EInstSteps::CopyMaster :
            facCQCInst.LogInstallMsg(L"Copying master server files");
            break;

        case tCQCInst::EInstSteps::GenConfig :
            facCQCInst.LogInstallMsg(L"Generating configuration files");
            break;

        case tCQCInst::EInstSteps::GenCmdFiles :
            facCQCInst.LogInstallMsg(L"Generating command files");
            break;

        case tCQCInst::EInstSteps::UpgradeData :
            facCQCInst.LogInstallMsg(L"Upgrading data");
            break;

        case tCQCInst::EInstSteps::InstClService :
            facCQCInst.LogInstallMsg(L"Installing Client Service");
            break;

        case tCQCInst::EInstSteps::InstAppShell :
            facCQCInst.LogInstallMsg(L"Installing App Shell Service");
            break;

        case tCQCInst::EInstSteps::StartSwapDirs :
            facCQCInst.LogInstallMsg(L"Swapping in new directory");
            break;

        case tCQCInst::EInstSteps::SaveBin :
            facCQCInst.LogInstallMsg(L"Saving old Bin to SaveInst");
            break;

        case tCQCInst::EInstSteps::SaveData :
            facCQCInst.LogInstallMsg(L"Saving old CQCData to SaveInst");
            break;

        case tCQCInst::EInstSteps::SaveVer :
            facCQCInst.LogInstallMsg(L"Saving old version info to SaveInst");
            break;

        case tCQCInst::EInstSteps::SaveNotes :
            facCQCInst.LogInstallMsg(L"Saving old release notes to SaveInst");
            break;

        case tCQCInst::EInstSteps::NewBin :
            facCQCInst.LogInstallMsg(L"Moving up new Bin from TmpDir");
            break;

        case tCQCInst::EInstSteps::NewData :
            facCQCInst.LogInstallMsg(L"Moving up new CQCData from TmpDir");
            break;

        case tCQCInst::EInstSteps::GenLinks :
            facCQCInst.LogInstallMsg(L"Generating start menu links");
            break;

        case tCQCInst::EInstSteps::Recovering :
            facCQCInst.LogInstallMsg(L"Installation failed, recovering");
            break;

        case tCQCInst::EInstSteps::Complete :
            facCQCInst.LogInstallMsg(L"Installation is complete");
            break;

        default :
            break;
    };

    // And post a status packet
    m_colStatusQ.objAdd(TCQCStatusPacket(eStep));
}


// ---------------------------------------------------------------------------
//  TInstStatusPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstStatusPanel::bCreated()
{
    TParent::bCreated();

    // Create our controls
    LoadDlgItems(kCQCInst::ridPan_Status);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Status_Meter, m_pwndMeter);
    CastChildWnd(*this, kCQCInst::ridPan_Status_Msg, m_pwndText);

    return kCIDLib::True;
}


tCIDLib::TVoid TInstStatusPanel::Destroyed()
{
    // Stop the timer if not already
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    TParent::Destroyed();
}


tCIDLib::TVoid TInstStatusPanel::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    //
    //  If the status is beyond complete then the install process has not
    //  started or is done.
    //
    if (m_eStep >= tCQCInst::EInstSteps::Count)
        return;

    //
    //  While there are items in the queue, get them out and put them in
    //  the list box.
    //
    TCQCStatusPacket spackCur;
    while (kCIDLib::True)
    {
        if (!m_colStatusQ.bGetNext(spackCur, 10))
            break;

        tCIDLib::TCard4 c4Percent;
        tCIDLib::TMsgId midStatus = kCIDLib::c4MaxCard;
        switch(spackCur.eStep())
        {
            case tCQCInst::EInstSteps::Start :
                c4Percent = 0;
                midStatus = 0;
                break;

            case tCQCInst::EInstSteps::CreateTmpDir :
                c4Percent = 5;
                midStatus = kCQCInstMsgs::midStatus_CreateTmpDir;
                break;

            case tCQCInst::EInstSteps::CreateNewDirs :
                c4Percent = 10;
                midStatus = kCQCInstMsgs::midStatus_CreatingDirs;
                break;

            case tCQCInst::EInstSteps::CopyBase :
                c4Percent = 25;
                midStatus = kCQCInstMsgs::midStatus_CopyingBaseDirs;
                break;

            case tCQCInst::EInstSteps::CopyClient :
                c4Percent = 30;
                midStatus = kCQCInstMsgs::midStatus_InstallingClient;
                break;

            case tCQCInst::EInstSteps::CopyCQCServer :
                c4Percent = 35;
                midStatus = kCQCInstMsgs::midStatus_InstallingServer;
                break;

            case tCQCInst::EInstSteps::CopyMaster :
                c4Percent = 40;
                midStatus = kCQCInstMsgs::midStatus_InstallingMaster;
                break;

            case tCQCInst::EInstSteps::CopyGWServer :
                c4Percent = 41;
                midStatus = kCQCInstMsgs::midStatus_InstallingGWSrv;
                break;

            case tCQCInst::EInstSteps::CopyWebServer :
                c4Percent = 42;
                midStatus = kCQCInstMsgs::midStatus_InstallingWebSrv;
                break;

            case tCQCInst::EInstSteps::GenConfig :
                c4Percent = 45;
                midStatus = kCQCInstMsgs::midStatus_GenConfig;
                break;

            case tCQCInst::EInstSteps::GenCmdFiles :
                c4Percent = 50;
                midStatus = kCQCInstMsgs::midStatus_GenCmdFiles;
                break;

            case tCQCInst::EInstSteps::UpgradeData :
                c4Percent = 65;
                midStatus = kCQCInstMsgs::midStatus_UpgradingData;
                break;

            case tCQCInst::EInstSteps::InstAppShell :
                c4Percent = 70;
                midStatus = kCQCInstMsgs::midStatus_InstallAppShell;
                break;

            case tCQCInst::EInstSteps::InstClService :
                c4Percent = 75;
                midStatus = kCQCInstMsgs::midStatus_InstallClService;
                break;

            case tCQCInst::EInstSteps::StartSwapDirs :
                c4Percent = 80;
                midStatus = kCQCInstMsgs::midStatus_SwapDirs;
                break;

            case tCQCInst::EInstSteps::GenLinks :
                c4Percent = 85;
                midStatus = kCQCInstMsgs::midStatus_GenLinks;
                break;

            case tCQCInst::EInstSteps::Recovering :
                c4Percent = 95;
                midStatus = kCQCInstMsgs::midStatus_Recovering;
                EnterFailureMode();
                break;

            case tCQCInst::EInstSteps::Complete :
                c4Percent = 100;

                // Stop the timer now if not already
                if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
                {
                    StopTimer(m_tmidUpdate);
                    m_tmidUpdate = kCIDCtrls::tmidInvalid;
                }
                break;

            default :
                //
                //  Some are just intermediate steps that we don't display
                //  any indicator for or change the percent. The status
                //  value will remain c4MaxCard, so we won't set any new
                //  status below.
                //
                break;
        };

        // If we got a new useful status, set it
        if (midStatus != kCIDLib::c4MaxCard)
        {
            m_pwndMeter->SetValue(c4Percent);
            if (midStatus)
            {
                m_strMsg.LoadFromMsg(midStatus, facCQCInst);
                m_pwndText->strWndText(m_strMsg);
            }
             else
            {
                m_pwndText->ClearText();
            }
        }

        // Store the current step that we set
        m_eStep = spackCur.eStep();
    }
}


// ---------------------------------------------------------------------------
//  TInstStatusPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When the timer sees the completion event, it calls here to wait for the
//  thread to end. When it ends, we get the termination result and put up
//  either a failure or success panel.
//
tCIDLib::TVoid TInstStatusPanel::WaitInstallThread()
{
    tCIDLib::EExitCodes eRes;
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    try
    {
        facCQCInst.LogInstallMsg(L"Waiting for installer thread to stop");
        eRes = m_thrInstaller.eWaitForDeath(10000);
    }

    catch(TError& errToCatch)
    {
        facCQCInst.LogInstallMsg(L"Exception stopping install thread");
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    //
    //  Store the status results. If it worked, and this is the MS, then block for
    //  a little bit to give the servers time to come up.
    //
    if (eRes == tCIDLib::EExitCodes::Normal)
    {
        if (infoCur().viiNewInfo().bMasterServer())
            facCIDCtrls().MsgYield(5000);

        infoCur().eInstStatus(tCQCInst::EInstStatus::Done);
    }
     else
    {
        infoCur().eInstStatus(tCQCInst::EInstStatus::Failed);
    }
}

