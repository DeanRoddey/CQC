//
// FILE NAME: CQCGKit_ChangePWDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2002
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
//  This file implements the dialog that allows the user to change their
//  logon password.
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
#include    "CQCGKit_.hpp"
#include    "CQCGKit_ChangePWDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCChangePWDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCChangePWDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCChangePWDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCChangePWDlg::TCQCChangePWDlg() :

    m_pwndCancel(nullptr)
    , m_pwndChange(nullptr)
    , m_pwndOldPW(nullptr)
    , m_pwndNewPW1(nullptr)
    , m_pwndNewPW2(nullptr)
    , m_pwndCQCPW(nullptr)
    , m_pwndWebPW(nullptr)
{
}

TCQCChangePWDlg::~TCQCChangePWDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCChangePWDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCChangePWDlg::Run(const TWindow& wndOwner, const TString& strUserName)
{
    m_strUserName = strUserName;
    c4RunDlg(wndOwner, facCQCGKit(), kCQCGKit::ridDlg_ChangePW);
}


// ---------------------------------------------------------------------------
//  TCQCChangePWDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCChangePWDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_Change, m_pwndChange);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_OldPW, m_pwndOldPW);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_NewPW1, m_pwndNewPW1);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_NewPW2, m_pwndNewPW2);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_CQCPW, m_pwndCQCPW);
    CastChildWnd(*this, kCQCGKit::ridDlg_ChangePW_WebPW, m_pwndWebPW);

    // Put the current user login name into the prompt
    TString strTBText = strWndText();
    strTBText.eReplaceToken(m_strUserName, L'1');
    strWndText(strTBText);

    // Initially select the CQC password as the one to change
    m_pwndCQCPW->SetCheckedState(kCIDLib::True);

    // And register our handlers
    m_pwndCancel->pnothRegisterHandler(this, &TCQCChangePWDlg::eClickHandler);
    m_pwndChange->pnothRegisterHandler(this, &TCQCChangePWDlg::eClickHandler);
    m_pwndOldPW->pnothRegisterHandler(this, &TCQCChangePWDlg::eEFHandler);
    m_pwndNewPW1->pnothRegisterHandler(this, &TCQCChangePWDlg::eEFHandler);
    m_pwndNewPW2->pnothRegisterHandler(this, &TCQCChangePWDlg::eEFHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCChangePWDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCChangePWDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCGKit::ridDlg_ChangePW_Change)
    {
        // Make sure that the two new passwords are the same
        if (m_pwndNewPW1->strWndText() != m_pwndNewPW2->strWndText())
        {
            TErrBox msgbBadPW(facCQCGKit().strMsg(kGKitErrs::errcLogon_PWMismatch));
            msgbBadPW.ShowIt(*this);
        }
         else
        {
            // Call the helper method that does the change password sequence
            const tCIDLib::TBoolean bIsCQCPW = m_pwndCQCPW->bCheckedState();
            const tCIDLib::TBoolean bDone = facCQCGKit().bChangePassword
            (
                *this
                , m_strUserName
                , m_pwndOldPW->strWndText()
                , m_pwndNewPW1->strWndText()
                , bIsCQCPW
            );

            // If it worked, we are done. If not, the error was shown to the user
            if (bDone)
            {
                const TString strMsg
                (
                    bIsCQCPW
                    ? facCQCGKit().strMsg(kGKitMsgs::midStatus_PWChanged)
                    : facCQCGKit().strMsg(kGKitMsgs::midStatus_WebPWChanged)
                );
                TOkBox msgbSuccess(strMsg);
                msgbSuccess.ShowIt(*this);

                EndDlg(kCQCGKit::ridDlg_ChangePW_Change);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_ChangePW_Cancel)
    {
        EndDlg(kCQCGKit::ridDlg_ChangePW_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCChangePWDlg::eEFHandler(TEFChangeInfo& wnotEvent)
{
    // We only care about change events
    if (wnotEvent.eEvent() == tCIDCtrls::EEFEvents::Changed)
    {
        // Disable the change button if any of the values are empty
        const tCIDLib::TBoolean bAnyEmpty
        (
            m_pwndOldPW->strWndText().bIsEmpty()
            | m_pwndNewPW1->strWndText().bIsEmpty()
            | m_pwndNewPW2->strWndText().bIsEmpty()
        );

        m_pwndChange->SetEnable(!bAnyEmpty);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


