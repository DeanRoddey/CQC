//
// FILE NAME: CQCInst_CheckMSPanel.cpp
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
//  This file implements the info panel that does the master server check
//  if it's not being installed here. We have to contact it and we have to
//  get the system id from it and store it as the system id for this system.
//
//  If we are installing any background services, we need to get an admin
//  login so that we can get some required info.
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
RTTIDecls(TInstCheckMSPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstCheckMSPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstCheckMSPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstCheckMSPanel::TInstCheckMSPanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Master Server Test", pinfoCur, pwndParent)
    , m_pwndImage(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndTest(nullptr)
    , m_strLastTest()
{
}

TInstCheckMSPanel::~TInstCheckMSPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstCheckMSPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCheckMSPanel::bPanelIsVisible() const
{
    //
    //  We only want to be seen if the master server is not being installed. If not, then
    //  we have to verify that we can contact the Master Server at the location indicated
    //  by the user.
    //
    return !infoCur().viiNewInfo().bMasterServer();
}


tCIDLib::TBoolean
TInstCheckMSPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    //
    //  If not installing the master server and they've not passed the connect
    //  test yet, then reject it.
    //
    if (!infoCur().viiNewInfo().bMasterServer() && !m_bPassed)
    {
        strErrText.LoadFromMsg(kCQCInstErrs::errcVal_NoMSTest, facCQCInst);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TInstCheckMSPanel::ReactToChanges()
{
    //
    //  If we had previously passed, but the address has changed, clear the status
    //  stuff. We also tell the facility object to clear any stored creds.
    //
    if (m_bPassed && (m_strLastTest != infoCur().viiNewInfo().strMSAddr()))
    {
        m_pwndImage->SetBitmap(m_bmpBad);
        m_pwndStatus->ClearText();
        m_bPassed = kCIDLib::False;
    }
}


// ---------------------------------------------------------------------------
//  TInstCheckMSPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCheckMSPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_MSTest_Title));

    // Load our child controls
    LoadDlgItems(kCQCInst::ridPan_MSTest);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_MSTest_Image, m_pwndImage);
    CastChildWnd(*this, kCQCInst::ridPan_MSTest_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCInst::ridPan_MSTest_Test, m_pwndTest);

    // Register a click handler on the main frame for our test button
    m_pwndTest->pnothRegisterHandler(this, &TInstCheckMSPanel::eClickHandler);

    // Load our app images
    m_bmpBad = facCQCInst.bmpLoadLargeAppImg(L"CQCAppGray");
    m_bmpGood = facCQCInst.bmpLoadLargeAppImg(L"CQCApp");

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstCheckMSPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstCheckMSPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_MSTest_Test)
    {
        try
        {
            // Assume failure
            m_bPassed = kCIDLib::False;
            m_pwndImage->SetBitmap(m_bmpBad);
            m_pwndStatus->strWndText(L"Contacting Master Server");

            // Resolve the master server address and log it for later use
            {
                TIPAddress ipaLog
                (
                    infoCur().viiNewInfo().strMSAddr(), tCIDSock::EAddrTypes::Unspec
                );
                facCQCInst.LogInstallMsg(L"Master server address resolves to: %(1)", ipaLog);
            }

            //
            //  The user has to log in as an admin on the target MS, to prove he has the right
            //  to install a new client, and so that we can get some info we need. We use a
            //  helper which will do the login if needed or just return the credentials if we
            //  already got them in a previous step.
            //
            TString strMsg;
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midTitle_AdminLogin, strMsg);
            TCQCUserCtx cuctxTmp;
            if (facCQCInst.wndMain().bLogin(*this, strMsg, cuctxTmp))
            {
                // Put up a wait pointer while we are doing this part
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

                //
                //  Get a security server proxy, and get the secondary server credentials,
                //  we have to provide our admin security token we just got. We have to store
                //  this away in the install info.
                //
                tCQCKit::TSecuritySrvProxy orbcSec = facCQCKit().orbcSecuritySrvProxy(5000);
                TString strSrvUser;
                TString strSrvPassword;
                if (orbcSec->bQuerySrvCreds(strSrvUser, strSrvPassword, cuctxTmp.sectUser()))
                {
                    //
                    //  Store a success and the address we tested against, and update it to use the
                    //  system id we got.
                    //
                    m_pwndImage->SetBitmap(m_bmpGood);
                    facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midStatus_MSFound, strMsg);
                    m_pwndStatus->strWndText(strMsg);

                    m_strLastTest = infoCur().viiNewInfo().strMSAddr();

                    // Just to be sure, make sure the reported account exists
                    if (orbcSec->bLoginExists(strSrvUser, cuctxTmp.sectUser()))
                    {
                        //
                        //  Store away the server credentials stuff on the install info. The install
                        //  thread will get it from there.
                        //
                        infoCur().SetSrvCreds(strSrvUser, strSrvPassword);
                        m_bPassed = kCIDLib::True;
                    }
                     else
                    {
                        TErrBox msgbFail(facCQCInst.strMsg(kCQCInstErrs::errcVal_MSCheckFailed));
                        msgbFail.ShowIt(*this);
                        m_pwndStatus->strWndText(facCQCInst.strMsg(kCQCInstErrs::errcVal_SrvCreds));
                    }
                }
                else
                {
                    facCQCInst.bLoadCIDMsg(kCQCInstErrs::errcFail_NoSrvCreds, strMsg);
                    m_pwndStatus->strWndText(strMsg);
                    m_bPassed = kCIDLib::False;
                }

                if (m_bPassed)
                {
                    // Get an installation server proxy and ping it to insure connectivity
                    tCQCKit::TInstSrvProxy orbcInst = facCQCKit().orbcInstSrvProxy(10000);
                    orbcInst->Ping();
                }
            }
            else
            {
                facCQCInst.bLoadCIDMsg(kCQCInstErrs::errcFail_AdminReq, strMsg);
                m_pwndStatus->strWndText(strMsg);
                m_bPassed = kCIDLib::False;
            }
        }

        catch(TError& errToCatch)
        {
            m_bPassed = kCIDLib::False;
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            const TString strErr = facCQCInst.strMsg(kCQCInstErrs::errcVal_MSCheckFailed);
            TErrBox msgbFail(strErr);
            msgbFail.ShowIt(*this);
            m_pwndStatus->strWndText(errToCatch.strErrText());

            return tCIDCtrls::EEvResponses::Handled;
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
