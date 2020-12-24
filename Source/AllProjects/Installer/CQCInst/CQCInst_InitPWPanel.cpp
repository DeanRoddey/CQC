//
// FILE NAME: CQCInst_InitPWPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/12/2004
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
//  This file implements the info panel that gets the initial admin password
//  from the user.
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
RTTIDecls(TInstInitPWPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstInitPWPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstInitPWPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstInitPWPanel::TInstInitPWPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Initial PW", pinfoCur, pwndParent)
    , m_bPWSetDone(kCIDLib::False)
    , m_pwndPassword(nullptr)
    , m_pwndSetPW(nullptr)
    , m_strCurPW(L"Welcome")
{
}

TInstInitPWPanel::~TInstInitPWPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstInitPWPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstInitPWPanel::bPanelIsVisible() const
{
    //
    //  We are only visible if a new install and installing the master server and the
    //  install succeeded.
    //
    return
    (
        (infoCur().eTargetType() == tCQCInst::ETargetTypes::Clean)
        && (infoCur().eInstStatus() == tCQCInst::EInstStatus::Done)
        && infoCur().viiNewInfo().bMasterServer()
    );
}


tCIDLib::TBoolean
TInstInitPWPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // They can't go forward till they set the password
    if (!m_bPWSetDone)
    {
        widFailed = kCQCInst::ridPan_InitPW_Password;
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoPassword, facCQCInst);
    }
    return m_bPWSetDone;
}


// ---------------------------------------------------------------------------
//  TInstInitPWPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstInitPWPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_InitPW_Title));

    // Load our child controls
    LoadDlgItems(kCQCInst::ridPan_InitPW);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_InitPW_Password, m_pwndPassword);
    CastChildWnd(*this, kCQCInst::ridPan_InitPW_Set, m_pwndSetPW);

    // Register a click handler on the main frame for our Set button
    m_pwndSetPW->pnothRegisterHandler(this, &TInstInitPWPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstInitPWPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstInitPWPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    // If not our guy, then return
    if (wnotEvent.widSource() != kCQCInst::ridPan_InitPW_Set)
        return tCIDCtrls::EEvResponses::Handled;

    // If no password entered, then return
    TString strInitPW = m_pwndPassword->strWndText();
    if (strInitPW.bIsEmpty())
        return tCIDCtrls::EEvResponses::Handled;

    try
    {
        //
        //  Set up the ORB using the master server info we were given.
        //  To be safe, terminate it first, since some earlier code might
        //  have cranked it up to test entered code and not shut it back
        //  down.
        //
        facCIDOrb().Terminate();
        TSysInfo::SetNSAddr
        (
            infoCur().viiNewInfo().strMSAddr(), infoCur().viiNewInfo().ippnNameServer()
        );
        facCIDOrb().InitClient();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Tell the user we couldn't initialize the ORB
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCInst.strMsg(kCQCInstErrs::errcFail_InitORB)
            , errToCatch
        );

        if (!errToCatch.bLogged())
            TModule::LogEventObj(errToCatch);
        return tCIDCtrls::EEvResponses::Handled;
    }

    // Get a reference to the security server
    tCQCKit::TSecuritySrvProxy orbcSS;
    try
    {
        orbcSS = facCQCKit().orbcSecuritySrvProxy(10000);
    }

    catch(const TError& errToCatch)
    {
        // Tell the user we couldn't contact the back end
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCInst.strMsg(kCQCInstErrs::errcFail_ContactCQC)
            , errToCatch
        );

        // Then shut the ORB back down and return
        try
        {
            facCIDOrb().Terminate();
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return tCIDCtrls::EEvResponses::Handled;
        }
    }

    // Looks ok, so let's try to set the password
    try
    {
        // We have to log in first so that we can do the change
        TCQCSecChallenge seccChangePW;
        if (!orbcSS->bLoginReq(L"Admin", seccChangePW))
        {
            facCQCInst.LogInstallMsg(L"Could not do login challenge");
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Lets do a hash of the last password that was set
        TMD5Hash mhashPW;
        TMessageDigest5 mdigTmp;
        mdigTmp.DigestStr(m_strCurPW);
        mdigTmp.Complete(mhashPW);

        // And use that to validate the challenge
        seccChangePW.Validate(mhashPW);

        // And send that back to get a security token, assuming its legal
        tCQCKit::ELoginRes  eRes;
        TCQCSecToken        sectTmp;
        TCQCUserAccount     uaccTmp;
        if (!orbcSS->bGetSToken(seccChangePW, sectTmp, uaccTmp, eRes))
        {
            facCQCInst.LogInstallMsg(L"Could not get security token");
            return tCIDCtrls::EEvResponses::Handled;
        }

        //
        //  Ok, they know the current password, so lets do the password change
        //  event. So create a key from the current password hash and then a
        //  Blowfish encrypter from that key. We'll use this to encrypt the
        //  new password before we send it over. The server knows the old PW
        //  hash and will use it to decrypt what we send.
        //
        TCryptoKey ckeyPW(mhashPW);
        TBlowfishEncrypter crypPW(ckeyPW);

        //
        //  Flatten a hash of the password into a buffer, and then encrypt
        //  that buffer using the key.
        //
        THeapBuf mbufCypher(4192UL);
        tCIDLib::TCard4 c4CypherBytes;
        {
            mdigTmp.StartNew();
            mdigTmp.DigestStr(strInitPW);
            mdigTmp.Complete(mhashPW);

            TBinMBufOutStream strmOut(4192UL);
            strmOut << mhashPW << kCIDLib::FlushIt;

            c4CypherBytes = crypPW.c4Encrypt
            (
                strmOut.mbufData(), mbufCypher, strmOut.c4CurSize()
            );
        }

        //
        //  And finally, call the security server to change the PW. The 2nd
        //  parm indicates we are changing the CQC password, not the web access
        //  password.
        //
        orbcSS->ChangePassword(L"Admin", kCIDLib::True, sectTmp, c4CypherBytes, mbufCypher);

        //
        //  It worked, so set the flag, store the new password to use as
        //  the old password if they do another set, and tell teh user that
        //  it worked and they can move forward.
        //
        m_bPWSetDone = kCIDLib::True;
        m_strCurPW = strInitPW;

        TOkBox msgbSuccess(facCQCInst.strMsg(kCQCInstMsgs::midStatus_PasswordSet));
        msgbSuccess.ShowIt(*this);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        // Tell the user we couldn't set the password
        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCInst.strMsg(kCQCInstErrs::errcFail_SetPassword)
            , errToCatch
        );

        // Then shut the ORB back down and return
        try
        {
            facCIDOrb().Terminate();
        }

        catch(TError& errNested)
        {
            if (!errNested.bLogged())
            {
                errNested.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errNested);
            }
            return tCIDCtrls::EEvResponses::Handled;
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
