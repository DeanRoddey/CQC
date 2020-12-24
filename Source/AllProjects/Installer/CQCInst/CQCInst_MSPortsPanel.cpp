//
// FILE NAME: CQCInst_MSPortsPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2004
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
//  This file implements the info panel that gets server IP ports from the
//  user.
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
RTTIDecls(TInstMSPortsPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstMSPortsPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstMSPortsPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstMSPortsPanel::TInstMSPortsPanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Misc. Server Ports", pinfoCur, pwndParent)
    , m_pwndClS(nullptr)
    , m_pwndCS(nullptr)
    , m_pwndDS(nullptr)
    , m_pwndEvents(nullptr)
    , m_pwndNS(nullptr)
    , m_pwndLS(nullptr)
{
}

TInstMSPortsPanel::~TInstMSPortsPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstMSPortsPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstMSPortsPanel::bPanelIsVisible() const
{
    // If not using previous options
    return !infoCur().bUsePrevOpts();
}


tCIDLib::TBoolean
TInstMSPortsPanel::bValidate(tCIDCtrls::TWndId& widFailed, TString& strErrText)
{
    if (!wndParent().bValidatePort(*m_pwndClS, strErrText))
    {
        widFailed = m_pwndClS->widThis();
        return kCIDLib::False;
    }

    // Make sure the ports are valid
    if (!wndParent().bValidatePort(*m_pwndCS, strErrText))
    {
        widFailed = m_pwndCS->widThis();
        return kCIDLib::False;
    }

    if (!wndParent().bValidatePort(*m_pwndDS, strErrText))
    {
        widFailed = m_pwndDS->widThis();
        return kCIDLib::False;
    }

    if (!wndParent().bValidatePort(*m_pwndEvents, strErrText))
    {
        widFailed = m_pwndEvents->widThis();
        return kCIDLib::False;
    }

    if (!wndParent().bValidatePort(*m_pwndLS, strErrText))
    {
        widFailed = m_pwndLS->widThis();
        return kCIDLib::False;
    }

    if (!wndParent().bValidatePort(*m_pwndNS, strErrText))
    {
        widFailed = m_pwndNS->widThis();
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TInstMSPortsPanel::Entered()
{
    TCQCInstallInfo& infoToLoad = infoCur();
    wndParent().LoadPortOpts(*this, *m_pwndClS, infoToLoad.viiNewInfo().ippnClService());
    wndParent().LoadPortOpts(*this, *m_pwndCS, infoToLoad.viiNewInfo().ippnCfgServer());
    wndParent().LoadPortOpts(*this, *m_pwndDS, infoToLoad.viiNewInfo().ippnDataServer());
    wndParent().LoadPortOpts(*this, *m_pwndEvents, infoToLoad.viiNewInfo().ippnEventBroadcast());
    wndParent().LoadPortOpts(*this, *m_pwndLS, infoToLoad.viiNewInfo().ippnLogServer());
    wndParent().LoadPortOpts(*this, *m_pwndNS, infoToLoad.viiNewInfo().ippnNameServer());
}


tCIDLib::TVoid
TInstMSPortsPanel::ReportPortUsage(tCQCInst::TPortCntList& fcolPorts)
{
    TCQCInstallInfo& infoCurVals = infoCur();

    fcolPorts[infoCurVals.viiNewInfo().ippnClService()]++;
    fcolPorts[infoCurVals.viiNewInfo().ippnCfgServer()]++;
    fcolPorts[infoCurVals.viiNewInfo().ippnEventBroadcast()]++;

    if (infoCurVals.viiNewInfo().bMasterServer())
    {
        fcolPorts[infoCurVals.viiNewInfo().ippnDataServer()]++;
        fcolPorts[infoCurVals.viiNewInfo().ippnLogServer()]++;
        fcolPorts[infoCurVals.viiNewInfo().ippnNameServer()]++;
    }
}


tCIDLib::TVoid TInstMSPortsPanel::SaveContents()
{
    // Store the current port settings away
    TCQCInstallInfo& infoStoreIn = infoCur();
    infoStoreIn.viiNewInfo().ippnClService(wndParent().ippnExtractPort(*m_pwndClS));
    infoStoreIn.viiNewInfo().ippnDataServer(wndParent().ippnExtractPort(*m_pwndDS));
    infoStoreIn.viiNewInfo().ippnCfgServer(wndParent().ippnExtractPort(*m_pwndCS));
    infoStoreIn.viiNewInfo().ippnEventBroadcast(wndParent().ippnExtractPort(*m_pwndEvents));
    infoStoreIn.viiNewInfo().ippnLogServer(wndParent().ippnExtractPort(*m_pwndLS));
    infoStoreIn.viiNewInfo().ippnNameServer(wndParent().ippnExtractPort(*m_pwndNS));
}


// ---------------------------------------------------------------------------
//  TInstMSPortsPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstMSPortsPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_Ports);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Ports_ClService, m_pwndClS);
    CastChildWnd(*this, kCQCInst::ridPan_Ports_CfgSrv, m_pwndCS);
    CastChildWnd(*this, kCQCInst::ridPan_Ports_DataSrv, m_pwndDS);
    CastChildWnd(*this, kCQCInst::ridPan_Ports_Events, m_pwndEvents);
    CastChildWnd(*this, kCQCInst::ridPan_Ports_NameSrv, m_pwndNS);
    CastChildWnd(*this, kCQCInst::ridPan_Ports_LogSrv, m_pwndLS);

    return kCIDLib::True;
}
