//
// FILE NAME: CQCInst_SummaryPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2004
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
//  This file implements the info panel that shows the user summary of
//  options.
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
RTTIDecls(TInstSummaryPanel,TInstInfoPanel)



// ---------------------------------------------------------------------------
//   CLASS: TInstSummaryPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstSummaryPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstSummaryPanel::TInstSummaryPanel(TCQCInstallInfo* const pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Summary", pinfoCur, pwndParent)
    , m_pwndText(nullptr)
{
}

TInstSummaryPanel::~TInstSummaryPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstSummaryPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TInstSummaryPanel::Entered()
{
    // Load up the summary info, to pick up any changes
    LoadSummary();
}


// ---------------------------------------------------------------------------
//  TInstSummaryPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstSummaryPanel::bCreated()
{
    // Load up our controls
    LoadDlgItems(kCQCInst::ridPan_Summary);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Sum_Text, m_pwndText);

    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TInstSummaryPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TInstSummaryPanel::LoadSummary()
{
    // Go through and format out all of the current settings to a text stream
    const TCQCVerInstallInfo& viiOld = infoCur().viiOldInfo();
    const TCQCVerInstallInfo& viiNew = infoCur().viiNewInfo();

    TTextStringOutStream strmOut(2048UL);
    strmOut << L"General\n"
            << L"--------------------------------\n"
            << L"Service Mode: ";

    switch(facCQCInst.eStartMode())
    {
        case tCIDKernel::EWSrvStarts::Auto :
            strmOut << facCQCInst.strMsg(kCQCInstMsgs::midPan_SrvMode_Automatic);
            break;

        case tCIDKernel::EWSrvStarts::Delayed :
            strmOut << facCQCInst.strMsg(kCQCInstMsgs::midPan_SrvMode_Delayed);
            break;

        case tCIDKernel::EWSrvStarts::Demand :
            strmOut << facCQCInst.strMsg(kCQCInstMsgs::midPan_SrvMode_OnDemand );
            break;

        case tCIDKernel::EWSrvStarts::Disabled :
            strmOut << facCQCInst.strMsg(kCQCInstMsgs::midPan_SrvMode_Disabled);
            break;

        default :
            strmOut << L"???";
            break;
    };
    strmOut << kCIDLib::NewLn;

    strmOut << L"        Path: " << infoCur().strPath() << kCIDLib::NewLn;

    // Only display the old version if there was one
    if (viiOld.c8Version())
    {
        strmOut << L"Old Version: " << viiOld.c4MajVersion() << L"."
                << viiOld.c4MinVersion() << L"." << viiOld.c4Revision() << kCIDLib::NewLn;
    }

    if (viiNew.bCertSupport())
        strmOut << L"  Certificate: " << viiNew.strCertInfo() << kCIDLib::NewLn;

    strmOut << L"  New Version: " << viiNew.c4MajVersion() << L"."
            << viiNew.c4MinVersion() << L"." << viiNew.c4Revision() << kCIDLib::DNewLn
            << L"Master Server\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bMasterServer() << kCIDLib::NewLn;

    if (!viiNew.bMasterServer())
        strmOut << L"  DNS Address: " << viiNew.strMSAddr() << kCIDLib::NewLn;

    strmOut << L"  Log Server Port: " << viiNew.ippnLogServer() << kCIDLib::NewLn
            << L"  Cfg Server Port: " << viiNew.ippnCfgServer() << kCIDLib::NewLn
            << L"  Name Server Port: " << viiNew.ippnNameServer() << kCIDLib::NewLn
            << L"  Data Server Port: " << viiNew.ippnDataServer() << kCIDLib::NewLn
            << L"  Ev Broadcast Port: " << viiNew.ippnEventBroadcast() << kCIDLib::DNewLn

            << L"Client Service\n"
            << L"--------------------------------\n"
            << L"  Server Port: " << viiNew.ippnClService() << kCIDLib::DNewLn

            << L"Device Control\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bCQCServer() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnCQCServer() << kCIDLib::DNewLn

            << L"CQC Client Tools\n"
            << L"--------------------------------\n"
            << L"  Cmd Prompt: " << viiNew.bCmdPrompt() << kCIDLib::NewLn
            << L"  Admin Interface: " << viiNew.bCQCClient() << kCIDLib::NewLn
            << L"  Std Viewer Client: " << viiNew.bIntfViewer();

    if (viiNew.bIntfViewerCtrl())
        strmOut << L"  (Ctrl Port=" << viiNew.ippnIVCtrl() << L")";
    strmOut << kCIDLib::DNewLn;

    strmOut << L"XML Gateway Server\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bXMLGWServer() << kCIDLib::NewLn
            << L"  Gateway Port: " << viiNew.ippnXMLGW() << kCIDLib::DNewLn;

    if (tCIDLib::bAllBitsOn(viiNew.eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Secure))
        strmOut << L"  Secure Port: " << viiNew.ippnXMLGWSecure() << kCIDLib::NewLn;

    if (tCIDLib::bAllBitsOn(viiNew.eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Insecure))
        strmOut << L"  Insecure Port: " << viiNew.ippnXMLGWServer() << kCIDLib::NewLn;

    strmOut << L"Tray Monitor\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bTrayMon() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnTrayMon() << kCIDLib::NewLn
            << L"     App Ctrl: " << viiNew.bAppCtrl() << kCIDLib::NewLn
            << L"       iTunes: " << viiNew.biTunes() << kCIDLib::DNewLn

            << L"CQC Voice\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bCQCVoice() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnCQCVoice() << kCIDLib::NewLn
            << L"      Account: " << viiNew.strCQCVoiceUserName() << kCIDLib::DNewLn

            << L"Event System\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bEventServer() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnEventServer() << kCIDLib::DNewLn

            << L"Logic Server\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bLogicServer() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnLogicServer() << kCIDLib::DNewLn

            << L"Web Server\n"
            << L"--------------------------------\n"
            << L"  Install: " << viiNew.bWebServer() << kCIDLib::NewLn
            << L"  Server Port: " << viiNew.ippnWebServer() << kCIDLib::NewLn;

    if (tCIDLib::bAllBitsOn(viiNew.eWebSecureOpts(), tCIDSock::ESecureOpts::Secure))
        strmOut << L"  HTTP Port: " << viiNew.ippnWebServerHTTPS() << kCIDLib::NewLn;
    else
        strmOut << L"  HTTP Port: [Disabled]";

    if (tCIDLib::bAllBitsOn(viiNew.eWebSecureOpts(), tCIDSock::ESecureOpts::Insecure))
        strmOut << L"  HTTPS Port: " << viiNew.ippnWebServerHTTP() << kCIDLib::NewLn;
    else
        strmOut << L"  HTTPS Port: [Disabled]" << kCIDLib::NewLn;

    if (viiNew.bMasterServer())
    {
        strmOut << L"    Help Via: " << (viiNew.bSecureHelp() ? L"HTTPS" : L"HTTP") << kCIDLib::NewLn;
    }

    strmOut << kCIDLib::NewEndLn;
    m_pwndText->strWndText(strmOut.strData());
}
