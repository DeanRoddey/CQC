//
// FILE NAME: CQCInst_WebSrvPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2004
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
//  This file implements the info panel that lets the user set the WebSrvs
//  Server options.
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
RTTIDecls(TInstWebSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstWebSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstWebSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstWebSrvPanel::TInstWebSrvPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Web Server Options", pinfoCur, pwndParent)
    , m_pwndAdminPort(nullptr)
    , m_pwndDoInsecure(nullptr)
    , m_pwndDoSecure(nullptr)
    , m_pwndHelpPort(nullptr)
    , m_pwndHelpPref(nullptr)
    , m_pwndInstall(nullptr)
    , m_pwndInsecurePort(nullptr)
    , m_pwndInsecurePref(nullptr)
    , m_pwndSecurePort(nullptr)
    , m_pwndSecurePref(nullptr)
{
}

TInstWebSrvPanel::~TInstWebSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstWebSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstWebSrvPanel::bPanelIsVisible() const
{
    // We want to be seen if installing the web server or not using previous opts
    return infoCur().viiNewInfo().bWebServer() || !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstWebSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Maker sure the ports are good
    if (!wndParent().bValidatePort(*m_pwndAdminPort, strErrText))
    {
        widFailed = m_pwndAdminPort->widThis();
        return kCIDLib::False;
    }

    if (m_pwndDoInsecure->bCheckedState())
    {
        if (!wndParent().bValidatePort(*m_pwndInsecurePort, strErrText))
        {
            widFailed = m_pwndInsecurePort->widThis();
            return kCIDLib::False;
        }
    }

    // Only check the secure port if that is enabled
    if (m_pwndDoSecure->bCheckedState())
    {
        if (!wndParent().bValidatePort(*m_pwndSecurePort, strErrText))
        {
            widFailed = m_pwndSecurePort->widThis();
            return kCIDLib::False;
        }

        //
        //  Certificate support must be enabled. It shouldn't be possible
        //  to get wrong, but just in case.
        //
        if (!infoCur().viiNewInfo().bCertSupport())
        {
            widFailed = m_pwndDoSecure->widThis();
            strErrText = facCQCInst.strMsg(kCQCInstErrs::errcVal_CertRequired);

            // Force it to be unchecked
            m_pwndDoSecure->SetCheckedState(kCIDLib::False);
            return kCIDLib::False;
        }
    }

    // If the web server is enabled, at least one of the ports has to be
    if (infoCur().viiNewInfo().bWebServer()
    &&  (!m_pwndDoInsecure->bCheckedState() && !m_pwndDoSecure->bCheckedState()))
    {
        widFailed = m_pwndInstall->widThis();
        strErrText = facCQCInst.strMsg(kCQCInstErrs::errcVal_NoPortsEnabled);
        return kCIDLib::False;
    }

    // Make sure the selected help port is available if this is the MS
    if (infoCur().viiNewInfo().bMasterServer() &&  infoCur().viiNewInfo().bWebServer())
    {
        if (infoCur().viiNewInfo().bSecureHelp())
        {
            if (!tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure))
            {
                widFailed = m_pwndHelpPort->widThis();
                strErrText = facCQCInst.strMsg(kCQCInstErrs::errcVal_MSHelpPort);
                return kCIDLib::False;
            }
        }
         else
        {
            if (!tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Insecure))
            {
                widFailed = m_pwndHelpPort->widThis();
                strErrText = facCQCInst.strMsg(kCQCInstErrs::errcVal_MSHelpPort);
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


// Load up the current content
tCIDLib::TVoid TInstWebSrvPanel::Entered()
{
    // It has to be installed if this is the master server
    if (infoCur().viiNewInfo().bWebServer())
        m_pwndInstall->SetCheckedState(kCIDLib::True);

    wndParent().LoadPortOpts(*this, *m_pwndAdminPort, infoCur().viiNewInfo().ippnWebServer());
    wndParent().LoadPortOpts(*this, *m_pwndInsecurePort, infoCur().viiNewInfo().ippnWebServerHTTP());
    wndParent().LoadPortOpts(*this, *m_pwndSecurePort, infoCur().viiNewInfo().ippnWebServerHTTPS());

    m_pwndDoInsecure->SetCheckedState
    (
        tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Insecure)
    );

    m_pwndDoSecure->SetCheckedState
    (
        tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure)
    );

    //
    //  If the master server is being installed, disable the install check box because
    //  it has to be installed. We are just letting them set the port.
    //
    m_pwndInstall->SetEnable(!infoCur().viiNewInfo().bMasterServer());

    // Select the configured help port if on the MS
    if (infoCur().viiNewInfo().bMasterServer())
    {
        m_pwndHelpPort->SelectByIndex(infoCur().viiNewInfo().bSecureHelp() ? 1 : 0);
    }
}


tCIDLib::TVoid TInstWebSrvPanel::ReactToChanges()
{
    // IF the master server is being installed, we have to be installed
    if (infoCur().viiNewInfo().bMasterServer())
        infoCur().viiNewInfo().bWebServer(kCIDLib::True);

    // If certificate info is not enabled, make sure we disable secure comms
    const tCIDLib::TBoolean bCertSupport = infoCur().viiNewInfo().bCertSupport();

    if (!bCertSupport)
    {
        infoCur().viiNewInfo().eWebSecureOpts
        (
            tCIDLib::eClearEnumBits(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure)
        );
    }

    m_pwndSecurePort->SetEnable(bCertSupport);
    m_pwndDoSecure->SetEnable(bCertSupport);
    m_pwndDoSecure->SetCheckedState(bCertSupport);

    // If this leaves us with neither port enabled, force the insecure port on
    if (infoCur().viiNewInfo().eWebSecureOpts() == tCIDSock::ESecureOpts::None)
        infoCur().viiNewInfo().eWebSecureOpts(tCIDSock::ESecureOpts::Insecure);

    // If the previously selected help port is not enabled, pick the other if available
    if (infoCur().viiNewInfo().bMasterServer())
    {
        if (infoCur().viiNewInfo().bSecureHelp())
        {
            // Just select the insecure in this case, we have to have something
            if (!tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure))
                infoCur().viiNewInfo().bSecureHelp(kCIDLib::False);
        }
        else
        {
            // If insecure is no longer configured but secure is, then assume that
            if (!tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Insecure)
            &&  tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure))
            {
                infoCur().viiNewInfo().bSecureHelp(kCIDLib::True);
            }
        }
    }
}


tCIDLib::TVoid TInstWebSrvPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bWebServer())
    {
        fcolPorts[infoCur().viiNewInfo().ippnWebServer()]++;
        fcolPorts[infoCur().viiNewInfo().ippnWebServerHTTP()]++;
        fcolPorts[infoCur().viiNewInfo().ippnWebServerHTTPS()]++;
    }
}


tCIDLib::TVoid TInstWebSrvPanel::SaveContents()
{
    // Store the current settings away
    tCIDSock::ESecureOpts eOpts = tCIDSock::ESecureOpts::None;
    if (m_pwndDoSecure->bCheckedState())
        eOpts |= tCIDSock::ESecureOpts::Secure;
    if (m_pwndDoInsecure->bCheckedState())
        eOpts |= tCIDSock::ESecureOpts::Insecure;

    infoCur().viiNewInfo().eWebSecureOpts(eOpts);

    infoCur().viiNewInfo().bWebServer(m_pwndInstall->bCheckedState());
    infoCur().viiNewInfo().ippnWebServer(wndParent().ippnExtractPort(*m_pwndAdminPort));
    infoCur().viiNewInfo().ippnWebServerHTTP(wndParent().ippnExtractPort(*m_pwndInsecurePort));
    infoCur().viiNewInfo().ippnWebServerHTTPS(wndParent().ippnExtractPort(*m_pwndSecurePort));

    // Only on the MS
    if (infoCur().viiNewInfo().bMasterServer())
        infoCur().viiNewInfo().bSecureHelp(m_pwndHelpPort->c4CurItem() == 1);
}


// ---------------------------------------------------------------------------
//  TInstWebSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstWebSrvPanel::bCreated()
{
    TParent::bCreated();

    // Load our dialog items
    LoadDlgItems(kCQCInst::ridPan_WebSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_AdminPort, m_pwndAdminPort);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_DoInsecure, m_pwndDoInsecure);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_HelpPort, m_pwndHelpPort);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_HelpPref, m_pwndHelpPref);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_DoSecure, m_pwndDoSecure);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_InsecurePort, m_pwndInsecurePort);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_InsecurePref, m_pwndInsecurePref);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_SecurePort, m_pwndSecurePort);
    CastChildWnd(*this, kCQCInst::ridPan_WebSrv_SecurePref, m_pwndSecurePref);


    // If certificate info is not enabled, then disable the HTTPS stuff
    if (!infoCur().viiNewInfo().bCertSupport())
    {
        infoCur().viiNewInfo().eWebSecureOpts(tCIDSock::ESecureOpts::Insecure);

        m_pwndDoSecure->SetCheckedState(kCIDLib::False);
        m_pwndDoSecure->SetEnable(kCIDLib::False);
        m_pwndSecurePort->SetEnable(kCIDLib::False);
    }

    //
    //  Load up the two port type names for the help port. If not the master server, just
    //  hide the prefix and combo box
    //
    if (infoCur().viiNewInfo().bMasterServer())
    {
        m_pwndHelpPort->c4AddItem(facCQCInst.pszLoadCIDMsg(kCQCInstMsgs::midPan_WebSrv_HTTPPort));
        m_pwndHelpPort->c4AddItem(facCQCInst.pszLoadCIDMsg(kCQCInstMsgs::midPan_WebSrv_HTTPSPort));
    }
     else
    {
        m_pwndHelpPort->SetVisibility(kCIDLib::False);
        m_pwndHelpPref->SetVisibility(kCIDLib::False);
    }

    return kCIDLib::True;
}

