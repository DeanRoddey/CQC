//
// FILE NAME: CppDrvTest_QDrvTextDlg.cpp
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
#include    "CppDrvTest.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TQueryDrvTextDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TQueryDrvTextDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TQueryDrvTextDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TQueryDrvTextDlg::TQueryDrvTextDlg() :

    m_pdcmdWait(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDataName(nullptr)
    , m_pwndOutput(nullptr)
    , m_pwndSend(nullptr)
    , m_pwndValId(nullptr)
{
}

TQueryDrvTextDlg::~TQueryDrvTextDlg()
{
    // Just in case we didn't get the command cleaned up, do that now
    if (m_pdcmdWait)
        TCQCServerBase::ReleaseCmd(m_pdcmdWait, kCIDLib::False);

    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TQueryDrvTextDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TQueryDrvTextDlg::RunDlg(const TWindow& wndOwner)
{
    c4RunDlg(wndOwner, facCppDrvTest, kCppDrvTest::ridDlg_QDrvText);
}


// ---------------------------------------------------------------------------
//  TQueryDrvTextDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TQueryDrvTextDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCppDrvTest::ridDlg_QDrvText_Close, m_pwndClose);
    CastChildWnd(*this, kCppDrvTest::ridDlg_QDrvText_ValId, m_pwndValId);
    CastChildWnd(*this, kCppDrvTest::ridDlg_QDrvText_DataName, m_pwndDataName);
    CastChildWnd(*this, kCppDrvTest::ridDlg_QDrvText_Send, m_pwndSend);
    CastChildWnd(*this, kCppDrvTest::ridDlg_QDrvText_Output, m_pwndOutput);

    // And register our handlers
    m_pwndClose->pnothRegisterHandler(this, &TQueryDrvTextDlg::eClickHandler);
    m_pwndSend->pnothRegisterHandler(this, &TQueryDrvTextDlg::eClickHandler);

    // Make sure our command is initially null
    m_pdcmdWait = 0;

    return bRet;
}


// ---------------------------------------------------------------------------
//  TQueryDrvTextDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TQueryDrvTextDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCppDrvTest::ridDlg_QDrvText_Send)
    {
        // Clear any currently displayed output
        m_pwndOutput->ClearText();

        if (facCppDrvTest.psdrvTest())
        {
            m_pdcmdWait = facCppDrvTest.psdrvTest()->pdcmdQQueryTextVal
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
    }
     else if (wnotEvent.widSource() == kCppDrvTest::ridDlg_QDrvText_Close)
    {
        EndDlg(kCppDrvTest::ridDlg_QDrvText_Close);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDLib::TVoid TQueryDrvTextDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
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

