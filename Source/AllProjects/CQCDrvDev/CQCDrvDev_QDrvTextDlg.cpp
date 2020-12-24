//
// FILE NAME: CQCDrvDev_QDrvTextDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/01/2005
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
RTTIDecls(TCQCQDrvTextDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCQDrvTextDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCQDrvTextDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCQDrvTextDlg::TCQCQDrvTextDlg() :

    m_pdcmdWait(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDataName(nullptr)
    , m_pwndOutput(nullptr)
    , m_pwndSend(nullptr)
    , m_pwndValId(nullptr)
{
}

TCQCQDrvTextDlg::~TCQCQDrvTextDlg()
{
    // Just in case we didn't get the command cleaned up, do that now
    if (m_pdcmdWait)
        TCQCServerBase::ReleaseCmd(m_pdcmdWait, kCIDLib::False);

    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TCQCQDrvTextDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCQDrvTextDlg::RunDlg(const TWindow& wndOwner)
{
    c4RunDlg(wndOwner, facCQCDrvDev, kCQCDrvDev::ridDlg_QDrvText);
}


// ---------------------------------------------------------------------------
//  TCQCQDrvTextDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCQDrvTextDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCDrvDev::ridDlg_QDrvText_Close, m_pwndClose);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_QDrvText_ValId, m_pwndValId);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_QDrvText_DataName, m_pwndDataName);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_QDrvText_Send, m_pwndSend);
    CastChildWnd(*this, kCQCDrvDev::ridDlg_QDrvText_Output, m_pwndOutput);

    // And register our handlers
    m_pwndClose->pnothRegisterHandler(this, &TCQCQDrvTextDlg::eClickHandler);
    m_pwndSend->pnothRegisterHandler(this, &TCQCQDrvTextDlg::eClickHandler);

    // Make sure our command is initially null
    m_pdcmdWait = nullptr;

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCQDrvTextDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCQDrvTextDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_QDrvText_Send)
    {
        // Clear any currently displayed output
        m_pwndOutput->ClearText();

        //
        //  Queue up a driver text operation asynchronously on the driver.
        //
        TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);
        if (TCQCGenDrvGlue::s_psdrvCurrent)
        {
            m_pdcmdWait = TCQCGenDrvGlue::s_psdrvCurrent->pdcmdQQueryTextVal
            (
                 m_pwndValId->strWndText(), m_pwndDataName->strWndText()
            );

            // Disable the buttons now until it completes
            m_pwndClose->SetEnable(kCIDLib::False);
            m_pwndSend->SetEnable(kCIDLib::False);

            m_pwndOutput->strWndText(L"[Operating has been queued for processing...]");

            // Start up an update timer to watch for completion
            m_tmidUpdate = tmidStartTimer(200);
        }
         else
        {
            // Release the lock and tell the user the driver isn't ready
            lockrSync.Release();

        }
    }
     else if (wnotEvent.widSource() == kCQCDrvDev::ridDlg_QDrvText_Close)
    {
        EndDlg(kCQCDrvDev::ridDlg_QDrvText_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDLib::TVoid TCQCQDrvTextDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
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
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }

        // Re-enable the buttons
        m_pwndClose->SetEnable(kCIDLib::True);
        m_pwndSend->SetEnable(kCIDLib::True);

        if (janCmd.pdcmdWait()->m_bError)
        {
            TString strText = L"A failure occurred. The error is\n\n";
            strText.Append(janCmd.pdcmdWait()->m_errReturn.strErrText());
            m_pwndOutput->strWndText(strText);
        }
         else
        {
            if (janCmd.pdcmdWait()->m_strVal.bIsEmpty())
            {
                m_pwndOutput->strWndText(L"[Succeeded but returned value was empty]");
            }
             else
            {
                //
                //  To make debugging easier, replace CR and LF values with
                //  text.
                //
                tCIDLib::TCard4 c4Index = 0;
                TString strText = janCmd.pdcmdWait()->m_strVal;
                strText.bReplaceSubStr(L"\r", L"\\r", c4Index, kCIDLib::True);
                c4Index = 0;
                strText.bReplaceSubStr(L"\n", L"\\n", c4Index, kCIDLib::True);

                m_pwndOutput->strWndText(strText);
            }
        }
    }
}

