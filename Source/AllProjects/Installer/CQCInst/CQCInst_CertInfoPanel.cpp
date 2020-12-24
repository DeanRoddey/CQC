//
// FILE NAME: CQCInst_CertInfoPanel.cpp
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
//  This file implements the info panel that lets the user install certificate
//  support.
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
RTTIDecls(TInstCertInfoPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstCertInfoPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstCertInfoPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstCertInfoPanel::TInstCertInfoPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Certificate Options", pinfoCur, pwndParent)
    , m_pwndCertInfo(nullptr)
    , m_pwndCertSupport(nullptr)
{
}

TInstCertInfoPanel::~TInstCertInfoPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstCertInfoPanel: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If not using previous options and either the web server or XML GW server
//  are enabled for installation.
//
tCIDLib::TBoolean TInstCertInfoPanel::bPanelIsVisible() const
{
    return
    (
        !infoCur().bUsePrevOpts()
        && (infoCur().viiNewInfo().bWebServer() || infoCur().viiNewInfo().bXMLGWServer())
    );
}


tCIDLib::TBoolean
TInstCertInfoPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    if (m_pwndCertSupport->bCheckedState())
    {
        if (m_pwndCertInfo->bIsEmpty())
        {
            widFailed = m_pwndCertInfo->widThis();
            strErrText.LoadFromMsg(kCQCInstErrs::errcFail_NoCertInfo, facCQCInst);
            return kCIDLib::False;
        }

        // Make sure that the certificate info is basically valid
        TString strType, strStore, strName;
        if (!facCIDSChan().bParseCertInfo(m_pwndCertInfo->strWndText(), strType, strStore, strName))
        {
            widFailed = m_pwndCertInfo->widThis();
            strErrText.LoadFromMsg(kCQCInstErrs::errcFail_BadCertInfo, facCQCInst);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


// Load up the current content
tCIDLib::TVoid TInstCertInfoPanel::Entered()
{
    m_pwndCertSupport->SetCheckedState(infoCur().viiNewInfo().bCertSupport());
    m_pwndCertInfo->strWndText(infoCur().viiNewInfo().strCertInfo());
}


tCIDLib::TVoid TInstCertInfoPanel::SaveContents()
{
    // Store the current settings away
    infoCur().viiNewInfo().bCertSupport(m_pwndCertSupport->bCheckedState());
    infoCur().viiNewInfo().strCertInfo(m_pwndCertInfo->strWndText());
}


// ---------------------------------------------------------------------------
//  TInstCertInfoPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstCertInfoPanel::bCreated()
{
    TParent::bCreated();

    // Load our dialog items
    LoadDlgItems(kCQCInst::ridPan_CertInfo);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_CertInfo_Cert, m_pwndCertInfo);
    CastChildWnd(*this, kCQCInst::ridPan_CertInfo_Enable, m_pwndCertSupport);

    // Register a click handler on our secure connection support check box
    m_pwndCertSupport->pnothRegisterHandler(this, &TInstCertInfoPanel::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstACSrvPanel: Private, non-virtual
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TInstCertInfoPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_CertInfo_Enable)
    {
        // If disabling, then clear any previously set certificate info
        if (!m_pwndCertSupport->bCheckedState())
            m_pwndCertInfo->ClearText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}

