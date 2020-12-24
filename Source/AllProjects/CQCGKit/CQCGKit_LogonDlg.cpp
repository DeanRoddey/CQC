//
// FILE NAME: CQCGKit_LogonDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/02/2002
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
//  This file implements the dialog that allows the user to logon to the
//  CQC system and get a security token that the calling app needs in order
//  to do anything useful.
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
#include    "CQCGKit_LogonDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCLogonDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCLogonDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCLogonDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCLogonDlg::TCQCLogonDlg() :

    m_eMinRole(tCQCKit::EUserRoles::Count)
    , m_eMode(tCQCGKit::ELogonModes::Initial)
    , m_psectTarget(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndLoginButton(nullptr)
    , m_pwndLoginName(nullptr)
    , m_pwndPassword(nullptr)
    , m_puaccTarget(nullptr)
{
}

TCQCLogonDlg::~TCQCLogonDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCLogonDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCLogonDlg::bRunDlg(  const   TWindow&            wndOwner
                        ,       TCQCSecToken&       sectToFill
                        ,       TCQCUserAccount&    uaccToFill
                        , const tCQCKit::EUserRoles eMinRole
                        , const TString&            strAppTitle
                        , const tCIDLib::TBoolean   bNoEnvLogon
                        , const TString&            strIconName)
{
    //
    //  If environmental logon is not supressed on this call, see if we can
    //  do an environmental logon, in which case we don't even need to run
    //  the dialog.
    //
    if (!bNoEnvLogon)
    {
        tCQCKit::ELoginRes eRes;
        if (facCQCKit().bDoEnvLogin(sectToFill, uaccToFill, eRes))
        {
            //
            //  Make sure it's at least at the minimum role we were asked
            //  to accept. If so, return success, else fall through and
            //  require a manual login.
            //
            if (uaccToFill.eRole() >= eMinRole)
                return kCIDLib::True;
        }
         else if (eRes != tCQCKit::ELoginRes::Ok)
        {
            //
            //  If eRes is anything other than Ok, that means that there was
            //  environmental info set, but it failed to work. So tell the user
            //  that it failed, then fall through to the regular login.
            //
            TErrBox msgbFail(m_strAppTitle, tCQCKit::strXlatELoginRes(eRes));
            msgbFail.ShowIt(wndOwner);
        }
    }

    //
    //  Store any incoming info we will need later.
    //
    m_eMinRole     = eMinRole;
    m_eMode        = tCQCGKit::ELogonModes::Initial;
    m_psectTarget  = &sectToFill;
    m_puaccTarget  = &uaccToFill;
    m_strAppTitle  = strAppTitle;
    m_strIconName  = strIconName;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCGKit(), kCQCGKit::ridDlg_Logon
    );

    // If they logged in successfully, return true
    return (c4Res == kCQCGKit::ridDlg_Logon_Login);
}


// ---------------------------------------------------------------------------
//  TCQCLogonDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCLogonDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    //
    //  Set our icon. Normally this is done using the executable's facility since the
    //  icon is in the same directory as it. But, in our case, we have to do it for our
    //  self, but we know this facility is in the same directory as the executable that
    //  is using us. The application tells us which icon to us.
    //
    facCQCGKit().SetFrameIcon(*this, m_strIconName);

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCGKit::ridDlg_Logon_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_Logon_Login, m_pwndLoginButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_Logon_Name, m_pwndLoginName);
    CastChildWnd(*this, kCQCGKit::ridDlg_Logon_Password, m_pwndPassword);

    // And register our event handlers
    m_pwndLoginButton->pnothRegisterHandler(this, &TCQCLogonDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCLogonDlg::eClickHandler);

    //
    //  If not in initial mode, then put the incoming logon name into the
    //  logon field and disable it.
    //
    if (m_eMode != tCQCGKit::ELogonModes::Initial)
    {
        m_pwndLoginName->strWndText(m_strCurLogon);
        m_pwndLoginName->SetEnable(kCIDLib::False);
    }
    return bRes;
}


// ---------------------------------------------------------------------------
//  TCQCLogonDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCLogonDlg::bAttemptLogon()
{
    // Get the two text field values
    const TString& strLogin = m_pwndLoginName->strWndText();
    const TString& strPassword = m_pwndPassword->strWndText();

    // If either is empty, then just beep at them
    if (strLogin.bIsEmpty() || strPassword.bIsEmpty())
    {
        TAudio::Beep(660, 100);
        return kCIDLib::False;
    }

    // Get a reference to the security server
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

    // And ask it to give us a challenge
    TCQCSecChallenge seccLogon;
    if (!orbcSS->bLoginReq(strLogin, seccLogon))
    {
        // Tell the user this is an unknown login name
        TErrBox msgbBadName
        (
            m_strAppTitle
            , facCQCGKit().strMsg(kGKitErrs::errcLogon_UnknownUser, strLogin)
        );
        msgbBadName.ShowIt(*this);

        // Put the focus back on the user name and select all
        m_pwndLoginName->TakeFocus();
        m_pwndLoginName->SelectAll();
        return kCIDLib::False;
    }

    // Lets do a hash of the user's password
    TMD5Hash mhashPW;
    {
        TMessageDigest5 mdigTmp;
        mdigTmp.DigestStr(strPassword);
        mdigTmp.Complete(mhashPW);
    }

    // And use that to validate the challenge
    seccLogon.Validate(mhashPW);

    // And send that back to get a security token, assuming its legal.
    tCQCKit::ELoginRes  eRes;
    tCIDLib::TBoolean bOK = orbcSS->bGetSToken
    (
        seccLogon, *m_psectTarget, *m_puaccTarget, eRes
    );

    if (!bOK)
    {
        // Tell the user that it failed
        TErrBox msgbBadPW(m_strAppTitle, tCQCKit::strXlatELoginRes(eRes));
        msgbBadPW.ShowIt(*this);

        // Put the focus back on the password and select all
        m_pwndPassword->TakeFocus();
        m_pwndPassword->SelectAll();
        return kCIDLib::False;
    }

    // Make sure it's at least at the minimum level we were told to accept
    if (m_puaccTarget->eRole() < m_eMinRole)
    {
        TErrBox msgbPerm
        (
            m_strAppTitle
            , facCQCKit().strMsg(kKitErrs::errcSec_InsufficientRole, strLogin)
        );
        msgbPerm.ShowIt(*this);
        return kCIDLib::False;
    }

    // It all worked, so return success
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses
TCQCLogonDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCGKit::ridDlg_Logon_Login)
    {
        // Call the method that does the logon sequence
        if (bAttemptLogon())
            EndDlg(wnotEvent.widSource());
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_Logon_Cancel)
    {
        EndDlg(wnotEvent.widSource());
    }
    return tCIDCtrls::EEvResponses::Handled;
}

