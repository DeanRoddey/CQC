//
// FILE NAME: CQCDrvDev_SendDrvCmdDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2012
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
//  This file implements the dialog that allows the user to invoke ClientCmd()
//  calls on the driver.
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
#include    "CQCDrvDev.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSendDrvCmdDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TCQCSendDrvCmdDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSendDrvCmdDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSendDrvCmdDlg::TCQCSendDrvCmdDlg() :

    m_pdcmdWait(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndCmdId(nullptr)
    , m_pwndData(nullptr)
    , m_pwndSend(nullptr)
{
}

TCQCSendDrvCmdDlg::~TCQCSendDrvCmdDlg()
{
    // Just in case we didn't get the command cleaned up, do that now
    if (m_pdcmdWait)
        TCQCServerBase::ReleaseCmd(m_pdcmdWait, kCIDLib::False);

    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TCQCSendDrvCmdDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSendDrvCmdDlg::RunDlg(const TWindow& wndOwner)
{
    c4RunDlg(wndOwner, facCQCDrvDev, kCQCDrvDev::ridDlg_SendDrvCmd);
}


// ---------------------------------------------------------------------------
//  TCQCSendDrvCmdDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSendDrvCmdDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCDrvDev::ridDlg_SendDrvCmd_Close, m_pwndClose);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_SendDrvCmd_CmdId, m_pwndCmdId);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_SendDrvCmd_Data, m_pwndData);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_SendDrvCmd_Send, m_pwndSend);

    // And register our handlers
    m_pwndClose->pnothRegisterHandler(this, &TCQCSendDrvCmdDlg::eClickHandler);
    m_pwndSend->pnothRegisterHandler(this, &TCQCSendDrvCmdDlg::eClickHandler);

    return bRet;
}


tCIDLib::TVoid TCQCSendDrvCmdDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // If not our timer, pass it to our parent and return
    if (tmidToDo != m_tmidUpdate)
        return;

    // Do a short wait on the command if we have one
    if (m_pdcmdWait && m_pdcmdWait->m_evWait.bWaitFor(5))
    {
        // Put a janitor on the command to insure it gets cleaned up
        TCQCSrvCmdJan janCmd(m_pdcmdWait);

        //
        //  And now clear our pointer just in case since we need to make sure
        //  it's zeroed when there's not a command. Use the pointer in the
        //  janitor for the rest below.
        //
        m_pdcmdWait = nullptr;

        // Disable the timer now
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;

        // If an error, show it to the user
        if (janCmd.pdcmdWait()->m_bError)
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"The send command operation failed"
                , janCmd.pdcmdWait()->m_errReturn
            );
        }

        // Re-enable the buttons
        m_pwndClose->SetEnable(kCIDLib::True);
        m_pwndSend->SetEnable(kCIDLib::True);
    }
}


// ---------------------------------------------------------------------------
//  TCQCSendDrvCmdDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCSendDrvCmdDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_SendDrvCmd_Send)
    {
        //
        //  Queue up a send driver command to the driver. Tell it we are going
        //  to wait, and our timer will watch it.
        //
        TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);
        if (TCQCGenDrvGlue::s_psdrvCurrent)
        {
            m_pdcmdWait = TCQCGenDrvGlue::s_psdrvCurrent->pdcmdQSendCmd
            (
                m_pwndCmdId->strWndText()
                , m_pwndData->strWndText()
                , tCQCKit::EDrvCmdWaits::Wait
            );

            // Disable the buttons now until it completes
            m_pwndClose->SetEnable(kCIDLib::False);
            m_pwndSend->SetEnable(kCIDLib::False);

            // Start up an update timer to watch for completion
            m_tmidUpdate = tmidStartTimer(200);
        }
    }
     else if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_SendDrvCmd_Close)
    {
        EndDlg(kCQCDrvDev::ridDlg_SendDrvCmd_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

