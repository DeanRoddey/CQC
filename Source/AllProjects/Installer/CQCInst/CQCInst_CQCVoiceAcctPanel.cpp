//
// FILE NAME: CQCInst_CQCVoiceAcctPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2017
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
//  This file implements the info panel that let's the user create the user account
//  that he indicated in the CQCVoice configuration panel.
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
RTTIDecls(TInstCQCVoiceAcctPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstCQCVoiceAcctPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstCQCVoiceAcctPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstCQCVoiceAcctPanel::TInstCQCVoiceAcctPanel( TCQCInstallInfo* const  pinfoCur
                                                , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"CQCVoice Account", pinfoCur, pwndParent)
    , m_bAcctDone(kCIDLib::False)
    , m_pwndCreate(nullptr)
{
}

TInstCQCVoiceAcctPanel::~TInstCQCVoiceAcctPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstCQCVoiceAcctPanel: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the configured user account already exists, then we don't nee dto show this
//  panel.
//
//  We have to deal with the fact that, if there was no MS password setup, that we
//  could get here as the bgn services are starting up, and they won't be ready yet.
//  So we allow for a good timeout on getting each of the server proxies we use.
//
tCIDLib::TBoolean TInstCQCVoiceAcctPanel::bPanelIsVisible() const
{
    const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    // If not installing CQCVoice, or it didn't complete, then obviously not
    if (!viiNew.bCQCVoice() || (infoCur().eInstStatus() != tCQCInst::EInstStatus::Done))
        return kCIDLib::False;

    // If they have already created the account, then no need
    if (m_bAcctDone)
        return kCIDLib::False;

    //
    //  Let's see if the account exists. If so, then set the account done flag and
    //  don't display this guy.
    //
    //  To do this, we need to log in as an admin. If the login works, the client
    //  side ORB will be initialized for what we need to do below.
    //
    TCQCUserCtx cuctxAdmin;
    if (facCQCInst.wndMain().bLogin(*this, strWndText(), cuctxAdmin))
    {
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy(10000);
            m_bAcctDone = orbcSS->bLoginExists(viiNew.strCQCVoiceUserName(), cuctxAdmin.sectUser());
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // Force it to display so they'll have to try a creation
            m_bAcctDone = kCIDLib::False;
        }
    }


    // If we don't have the account, then indicate we should be visible
    return !m_bAcctDone;
}


tCIDLib::TBoolean
TInstCQCVoiceAcctPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // They can't go forward till they set the password
    if (!m_bAcctDone)
    {
        widFailed = kCQCInst::ridPan_CQCVoiceAcct_Create;
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoCQCVoiceAccount, facCQCInst);
    }
    return m_bAcctDone;
}


// They can only go forward from here
tCQCInst::ENavFlags TInstCQCVoiceAcctPanel::eNavFlags() const
{
    return tCQCInst::ENavFlags::NoBack;
}


// ---------------------------------------------------------------------------
//  TInstCQCVoiceAcctPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCQCVoiceAcctPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_CQCVoiceAcct_Title));

    // Load our child controls
    LoadDlgItems(kCQCInst::ridPan_CQCVoiceAcct);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_CQCVoiceAcct_Create, m_pwndCreate);

    // Register a click handler on the main frame for our Create
    m_pwndCreate->pnothRegisterHandler(this, &TInstCQCVoiceAcctPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstCQCVoiceAcctPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstCQCVoiceAcctPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_CQCVoiceAcct_Create)
    {
        const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

        try
        {
            TSysInfo::SetNSAddr(viiNew.strMSAddr(), viiNew.ippnNameServer());
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

            // We can return in this case, because we have nothing to undo yet
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Get a reference to the security server
        tCQCKit::TSecuritySrvProxy orbcSS;
        try
        {
            orbcSS = facCQCKit().orbcSecuritySrvProxy();
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
        }

        //
        //  If we got the security server proxy, try to log in. They have to log in as
        //  as a system admin. The facility class provides a helper for this which will
        //  do the login if needed, or just give us back credentials if we have already
        //  gotten the credentials in a previous step.
        //
        if (orbcSS.pobjData())
        {
            try
            {
                TCQCUserCtx cuctxUser;
                if ( facCQCInst.wndMain().bLogin(*this, strWndText(), cuctxUser))
                {
                    //
                    //  OK, let's try to create the new user account. The incoming
                    //  password is ignored and set to Welcome. We need to change it
                    //  after we create it.
                    //
                    TCQCUserAccount uaccNew
                    (
                        tCQCKit::EUserRoles::LimitedUser
                        , L"CQCVoice login account"
                        , viiNew.strCQCVoiceUserName()
                        , TString::strEmpty()
                        , L"CQCVoice"
                        , L"Login"
                    );
                    orbcSS->CreateAccount(uaccNew, cuctxUser.sectUser());

                    const tCIDLib::TBoolean bPWChanged = facCQCGKit().bChangePassword
                    (
                        *this
                        , viiNew.strCQCVoiceUserName()
                        , L"Welcome"
                        , viiNew.strCQCVoicePassword()
                        , kCIDLib::True
                    );

                    //
                    //  If the password could not be set, it still worked. But tell them
                    //  that the PW will be Welcome until they fix it.
                    //
                    if (!bPWChanged)
                    {
                        TErrBox msgbDone(facCQCInst.strMsg(kCQCInstErrs::errcFail_CQCVoicePWNotSet));
                        msgbDone.ShowIt(*this);
                    }
                     else
                    {
                        TOkBox msgbDone(facCQCInst.strMsg(kCQCInstMsgs::midStatus_AcctCreated));
                        msgbDone.ShowIt(*this);
                    }

                    m_bAcctDone = kCIDLib::True;
                }
            }

            catch(TError& errToCatch)
            {
                //
                //  It's possible that, when we first checked, something went wrong and
                //  we failed when checking to see if the account exists. So, if we get
                //  an already exists error, just take it and move on, Else tell the user.
                //
                if (errToCatch.bCheckEvent(facCQCKit().strName(), kKitErrs::errcSec_UserExists))
                {
                    TOkBox msgbDone(facCQCInst.strMsg(kCQCInstMsgs::midStatus_AcctExists));
                    msgbDone.ShowIt(*this);
                    m_bAcctDone = kCIDLib::True;
                }
                 else
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                    // Tell the user we couldn't set the password
                    TExceptDlg dlgErr
                    (
                        *this
                        , strWndText()
                        , facCQCInst.strMsg(kCQCInstErrs::errcFail_CreateAccount)
                        , errToCatch
                    );
                }
            }
        }

        // Then shut the ORB back down and return
        try
        {
            facCIDOrb().Terminate();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
