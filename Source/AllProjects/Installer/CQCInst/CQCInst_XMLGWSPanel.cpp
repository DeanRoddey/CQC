//
// FILE NAME: CQCInst_XMLGWSPanel.cpp
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
//  This file implements the info panel that lets the user set the XML
//  Gateway Server options.
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
RTTIDecls(TInstXMLGWSrvPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstXMLGWSrvPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstXMLGWSrvPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstXMLGWSrvPanel::TInstXMLGWSrvPanel( TCQCInstallInfo* const  pinfoCur
                                        , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"XML GW Server Options", pinfoCur, pwndParent)
    , m_pwndAdminPort(nullptr)
    , m_pwndDoInsecure(nullptr)
    , m_pwndDoSecure(nullptr)
    , m_pwndInsecurePort(nullptr)
    , m_pwndInstall(nullptr)
    , m_pwndSecurePort(nullptr)
{
}

TInstXMLGWSrvPanel::~TInstXMLGWSrvPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstXMLGWSrvPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstXMLGWSrvPanel::bPanelIsVisible() const
{
    // If the GW server is being installed and not using previous opts
    return infoCur().viiNewInfo().bXMLGWServer() && !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstXMLGWSrvPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    // Maker sure the ports are good
    if (!wndParent().bValidatePort(*m_pwndAdminPort, strErrText))
    {
        widFailed = m_pwndAdminPort->widThis();
        return kCIDLib::False;
    }

    if (!wndParent().bValidatePort(*m_pwndInsecurePort, strErrText))
    {
        widFailed = m_pwndInsecurePort->widThis();
        return kCIDLib::False;
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
            return kCIDLib::False;
        }
    }

    // If the web server is enabled, at least one of the ports has to be
    if (infoCur().viiNewInfo().bXMLGWServer()
    &&  (!m_pwndDoInsecure->bCheckedState() && !m_pwndDoSecure->bCheckedState()))
    {
        widFailed = m_pwndInstall->widThis();
        strErrText = facCQCInst.strMsg(kCQCInstErrs::errcVal_NoPortsEnabled);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TInstXMLGWSrvPanel::Entered()
{
    // Load up the incoming stuff
    m_pwndInstall->SetCheckedState(infoCur().viiNewInfo().bXMLGWServer());
    wndParent().LoadPortOpts(*this, *m_pwndAdminPort, infoCur().viiNewInfo().ippnXMLGWServer());
    wndParent().LoadPortOpts(*this, *m_pwndInsecurePort, infoCur().viiNewInfo().ippnXMLGW());
    wndParent().LoadPortOpts(*this, *m_pwndSecurePort, infoCur().viiNewInfo().ippnXMLGWSecure());

    m_pwndDoInsecure->SetCheckedState
    (
        tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Insecure)
    );

    m_pwndDoSecure->SetCheckedState
    (
        tCIDLib::bAllBitsOn(infoCur().viiNewInfo().eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Secure)
    );

}


tCIDLib::TVoid TInstXMLGWSrvPanel::ReactToChanges()
{
    // If certificate info is not enabled, make sure we disable secure comms
    const tCIDLib::TBoolean bCertSupport = infoCur().viiNewInfo().bCertSupport();

    if (!bCertSupport)
    {
        infoCur().viiNewInfo().eXMLGWSecureOpts
        (
            tCIDLib::eClearEnumBits(infoCur().viiNewInfo().eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Secure)
        );
    }

    m_pwndSecurePort->SetEnable(bCertSupport);
    m_pwndDoSecure->SetEnable(bCertSupport);
}


tCIDLib::TVoid
TInstXMLGWSrvPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    if (infoCur().viiNewInfo().bXMLGWServer())
    {
        fcolPorts[infoCur().viiNewInfo().ippnXMLGW()]++;
        fcolPorts[infoCur().viiNewInfo().ippnXMLGWSecure()]++;
        fcolPorts[infoCur().viiNewInfo().ippnXMLGWServer()]++;
    }
}


tCIDLib::TVoid TInstXMLGWSrvPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bXMLGWServer(m_pwndInstall->bCheckedState());

    tCIDSock::ESecureOpts eOpts = tCIDSock::ESecureOpts::None;
    if (m_pwndDoSecure->bCheckedState())
        eOpts |= tCIDSock::ESecureOpts::Secure;
    if (m_pwndDoInsecure->bCheckedState())
        eOpts |= tCIDSock::ESecureOpts::Insecure;

    infoCur().viiNewInfo().eXMLGWSecureOpts(eOpts);

    infoCur().viiNewInfo().ippnXMLGW(wndParent().ippnExtractPort(*m_pwndInsecurePort));
    infoCur().viiNewInfo().ippnXMLGWSecure(wndParent().ippnExtractPort(*m_pwndSecurePort));
    infoCur().viiNewInfo().ippnXMLGWServer(wndParent().ippnExtractPort(*m_pwndAdminPort));
}


// ---------------------------------------------------------------------------
//  TInstXMLGWSrvPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstXMLGWSrvPanel::bCreated()
{
    TParent::bCreated();

    // Create our controls
    LoadDlgItems(kCQCInst::ridPan_XMLGWSrv);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_DoInstall, m_pwndInstall);
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_DoInsecure, m_pwndDoInsecure);
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_DoSecure, m_pwndDoSecure);
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_InsecurePort, m_pwndInsecurePort);
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_SecurePort, m_pwndSecurePort);
    CastChildWnd(*this, kCQCInst::ridPan_XMLGWSrv_AdminPort, m_pwndAdminPort);

    // If certificate info is not enabled, then disable the check box and combo box
    if (!infoCur().viiNewInfo().bCertSupport())
    {
        m_pwndDoSecure->SetCheckedState(kCIDLib::False);
        m_pwndDoSecure->SetEnable(kCIDLib::False);
        m_pwndSecurePort->SetEnable(kCIDLib::False);
    }
    return kCIDLib::True;
}

