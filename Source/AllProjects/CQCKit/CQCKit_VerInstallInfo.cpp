//
// FILE NAME: CQCKit_VerInstallInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/30/2002
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
//  This class the installation info packet that is written to disk during
//  install and used in subsequent installs to know what version is present.
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
#include "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVerInstallInfo,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_VerInstallInfo
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2:
        //      Added app control server info
        //
        //  Version 3:
        //      Added events server info
        //
        //  Version 4:
        //  Version 5:
        //      Some stuff was added during 1.3.5 development, and then deprecated
        //      before release, so if it's version 4 we read it in and throw it
        //      away. if V5, we've already thrown it away. This was done to avoid
        //      making beta testers go back to 1.3.4 again and start over.
        //
        //  Version 6:
        //      Added the web server info and new port for triggered event
        //      broadcasts.
        //
        //  Version 7:
        //      Added stuff for the IV remote control interface, a boolean for
        //      enable/disable, and the port to use.
        //
        //  Version 8:
        //      Added stuff for the remote viewer, client and server booleans for
        //      enable/disable installation, and ports for the server to use. And
        //      added a new entry on the client options for the media repository
        //      manager.
        //
        //  Version 9:
        //      Added the stuff for the Logic server.
        //
        //  Version 10:
        //      Added support for the unique system id.
        //
        //  Version 11:
        //      Added support for a backup directory so that we can offer to do
        //      a backup of the current system before an upgrade.
        //
        //  Version 12:
        //      We replaced the app control server with the more general tray
        //      monitor. Mostly we just re-named fields, but we also added an
        //      option to have the tray monitor provide iTunes access and a new
        //      separate option for the overall tray monitor installed or not
        //      flag. The old app control and new iTunes options become sub-
        //      options under the tray monitor.
        //
        //  Version 13:
        //      We added the client service. It's always installed, but we have to
        //      allow for port selection, even though it'll be rarely used.
        //
        //  Version 14:
        //      Added support for secure socket connections to the web server, so a
        //      secure port to listen on and a certificate info string, and an enabling
        //      flag.
        //
        //  Version 15:
        //      As of 5.x, we consolidated most client tools into the new single Admin
        //      Client. So some of the separate client tool options have been removed.
        //
        //  Version 16:
        //      Support for the new CQC Voice program configuration.
        //
        //  Version 17 :
        //      Skipped because it was supposed to be the stuff in V18 below, but the
        //      value was not written out. But the new version was written out, so when
        //      we tried to read it back in, we failed because the value was not there
        //      to read.
        //
        //  Version 18 :
        //      Added a new reply style (visual or spoken or both) option for CQC Voice.
        //
        //  Version 19 :
        //      Finally dropping the old RIVA system, now that WebRIVA is will worked out
        //      and everyone's had time to move forward.
        //
        //  Version 20 :
        //      Added a new secure port for the XML GW server. Removed the system
        //      id since it was just for licensing info. Moved certificate info to be
        //      separate from the web server since it's shared by web and GW servers.
        //      Made both secure and non-secure optional and changed over to new secure
        //      connection options enum to store those flags.
        //
        //  Version 21 :
        //      Added the m_bSecureHelp flag, which lets the user tell us which port on
        //      MS' web server to use for help access if both are enabled.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 21;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCVerInstallInfo
//  PREFIX: vii
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  TCQCVerInstallInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCVerInstallInfo::TCQCVerInstallInfo() :

    m_c4MajVer(0)
    , m_c4MinVer(0)
    , m_c4Revision(0)
    , m_enctLastUpdate(0)
    , m_eCQCVoiceRepStyle(tCQCKit::EReplyStyles::Spoken)
{
    // Reset component settings to defaults
    Reset();
}

TCQCVerInstallInfo::~TCQCVerInstallInfo()
{
}


// ---------------------------------------------------------------------------
//  TCQCVerInstallInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Returns true if any of the client tools is selected for install.
//
tCIDLib::TBoolean TCQCVerInstallInfo::bAnyClient() const
{
    return m_bCmdPrompt
           | m_bIntfViewer
           | m_bMediaRepoMgr
           | m_bCQCClient
           | m_bTrayMon
           | m_bCQCVoice;
}


// Returns true if any of the servers are selected for install
tCIDLib::TBoolean TCQCVerInstallInfo::bAnyServer() const
{
    return m_bMasterServer
           | m_bCQCServer
           | m_bEventServer
           | m_bLogicServer
           | m_bWebServer
           | m_bXMLGWServer;
}


// Get or set the 'App Ctrl Tray Monitor' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bAppCtrl() const
{
    return m_bAppCtrl;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bAppCtrl(const tCIDLib::TBoolean bToSet)
{
    m_bAppCtrl = bToSet;
    return m_bAppCtrl;
}


// Indicate if certificate info is enabled
tCIDLib::TBoolean TCQCVerInstallInfo::bCertSupport() const
{
    return m_bCertSupport;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCertSupport(const tCIDLib::TBoolean bToSet)
{
    m_bCertSupport = bToSet;
    return m_bCertSupport;
}


// Get/set the 'install the CQC Command Prompt' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bCmdPrompt() const
{
    return m_bCmdPrompt;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCmdPrompt(const tCIDLib::TBoolean bToSet)
{
    m_bCmdPrompt = bToSet;
    return m_bCmdPrompt;
}


// Get/Set the 'install the admin client' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bCQCClient() const
{
    return m_bCQCClient;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCQCClient(const tCIDLib::TBoolean bToSet)
{
    m_bCQCClient = bToSet;
    return m_bCQCClient;
}


// Get/set the 'install CQCServer' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bCQCServer() const
{
    return m_bCQCServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCQCServer(const tCIDLib::TBoolean bToSet)
{
    m_bCQCServer = bToSet;
    return m_bCQCServer;
}


// Get/set the 'CQCVoice Auto-Start' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bCQCVoiceAutoStart() const
{
    return m_bCQCVoiceAutoStart;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCQCVoiceAutoStart(const tCIDLib::TBoolean bToSet)
{
    m_bCQCVoiceAutoStart = bToSet;
    return m_bCQCVoiceAutoStart;
}


// Get/set the 'CQC Voice' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bCQCVoice() const
{
    return m_bCQCVoice;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bCQCVoice(const tCIDLib::TBoolean bToSet)
{
    m_bCQCVoice = bToSet;
    return m_bCQCVoice;
}


// Get/set the 'install the event server' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bEventServer() const
{
    return m_bEventServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bEventServer(const tCIDLib::TBoolean bToSet)
{
    m_bEventServer = bToSet;
    return m_bEventServer;
}


// Get/set the 'iTunes Tray Monitor' flag
tCIDLib::TBoolean TCQCVerInstallInfo::biTunes() const
{
    return m_biTunes;
}

tCIDLib::TBoolean TCQCVerInstallInfo::biTunes(const tCIDLib::TBoolean bToSet)
{
    m_biTunes = bToSet;
    return m_biTunes;
}


// Get/set the 'install the interface viewer' flag (the native one)
tCIDLib::TBoolean TCQCVerInstallInfo::bIntfViewer() const
{
    return m_bIntfViewer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bIntfViewer(const tCIDLib::TBoolean bToSet)
{
    m_bIntfViewer = bToSet;
    return m_bIntfViewer;
}


// Get/set the 'do interface viewer remote control' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bIntfViewerCtrl() const
{
    return m_bIntfViewerCtrl;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bIntfViewerCtrl(const tCIDLib::TBoolean bToSet)
{
    m_bIntfViewerCtrl = bToSet;
    return m_bIntfViewerCtrl;
}


// Get/set the 'install the logic server' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bLogicServer() const
{
    return m_bLogicServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bLogicServer(const tCIDLib::TBoolean bToSet)
{
    m_bLogicServer = bToSet;
    return m_bLogicServer;
}


//
//  Get/set the 'install the master server here' flag. If not here, then
//  they have to provide the address of it whereever it is.
//
tCIDLib::TBoolean TCQCVerInstallInfo::bMasterServer() const
{
    return m_bMasterServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bMasterServer(const tCIDLib::TBoolean bToSet)
{
    m_bMasterServer = bToSet;

    //
    //  If they turn on the master server, then set the master server address
    //  to the current host. If they turn it off, make sure that any stuff
    //  only legal on the master server is turned off.
    //
    if (m_bMasterServer)
    {
        m_strMSAddr = facCIDSock().strIPHostName();
    }
     else
    {
        m_bEventServer = kCIDLib::False;
        m_bLogicServer = kCIDLib::False;
    }
    return m_bMasterServer;
}


// Get/set the 'do media repo manager' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bMediaRepoMgr() const
{
    return m_bMediaRepoMgr;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bMediaRepoMgr(const tCIDLib::TBoolean bToSet)
{
    m_bMediaRepoMgr = bToSet;
    return m_bMediaRepoMgr;
}


// Get/set the 'Secure Help' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bSecureHelp() const
{
    return m_bSecureHelp;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bSecureHelp(const tCIDLib::TBoolean bToSet)
{
    m_bSecureHelp = bToSet;
    return m_bSecureHelp;
}


// Get/set the 'Tray Monitor Auto-Start' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bTrayAutoStart() const
{
    return m_bTrayAutoStart;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bTrayAutoStart(const tCIDLib::TBoolean bToSet)
{
    m_bTrayAutoStart = bToSet;
    return m_bTrayAutoStart;
}


// Get/set the 'Tray Monitor' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bTrayMon() const
{
    return m_bTrayMon;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bTrayMon(const tCIDLib::TBoolean bToSet)
{
    m_bTrayMon = bToSet;
    return m_bTrayMon;
}


// Get/set the 'Web Server' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bWebServer() const
{
    return m_bWebServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bWebServer(const tCIDLib::TBoolean bToSet)
{
    m_bWebServer = bToSet;
    return m_bWebServer;
}


// Get/set the 'install the XML Gateway server' flag
tCIDLib::TBoolean TCQCVerInstallInfo::bXMLGWServer() const
{
    return m_bXMLGWServer;
}

tCIDLib::TBoolean TCQCVerInstallInfo::bXMLGWServer(const tCIDLib::TBoolean bToSet)
{
    m_bXMLGWServer = bToSet;
    return m_bXMLGWServer;
}


// Get/set the major, minor, and revision versions
tCIDLib::TCard4 TCQCVerInstallInfo::c4MajVersion() const
{
    return m_c4MajVer;
}

tCIDLib::TCard4 TCQCVerInstallInfo::c4MajVersion(const tCIDLib::TCard4 c4ToSet)
{
    m_c4MajVer = c4ToSet;
    return m_c4MajVer;
}


tCIDLib::TCard4 TCQCVerInstallInfo::c4MinVersion() const
{
    return m_c4MinVer;
}

tCIDLib::TCard4 TCQCVerInstallInfo::c4MinVersion(const tCIDLib::TCard4 c4ToSet)
{
    m_c4MinVer = c4ToSet;
    return m_c4MinVer;
}


tCIDLib::TCard4 TCQCVerInstallInfo::c4Revision() const
{
    return m_c4Revision;
}

tCIDLib::TCard4 TCQCVerInstallInfo::c4Revision(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Revision = c4ToSet;
    return m_c4Revision;
}


//
//  To vastly simplify comparing versions, we provide a combined version in
//  which the version values are encoded into a TCard8, such that a previous
//  version will be a smaller value than a later one.
//
tCIDLib::TCard8 TCQCVerInstallInfo::c8MajMinVersion() const
{
    //
    //  The maj/min versions are in the top two words of the double word. We
    //  don't set the revision in this one.
    //
    tCIDLib::TCard8 c8Ret = m_c4MajVer;
    c8Ret <<= 16;
    c8Ret |= m_c4MinVer;
    c8Ret <<= 32;

    return c8Ret;
}

tCIDLib::TCard8 TCQCVerInstallInfo::c8Version() const
{
    //
    //  The maj/min versions are in the top two words of the double word, and
    //  the revision is in the low double word.
    //
    tCIDLib::TCard8 c8Ret = m_c4MajVer;
    c8Ret <<= 16;
    c8Ret |= m_c4MinVer;
    c8Ret <<= 32;
    c8Ret |= m_c4Revision;

    return c8Ret;
}


// Just clear all the 'install it' flags
tCIDLib::TVoid TCQCVerInstallInfo::DisableAll()
{
    m_bAppCtrl = kCIDLib::False;
    m_bCmdPrompt = kCIDLib::False;
    m_bCQCClient = kCIDLib::False;
    m_bCQCServer = kCIDLib::False;
    m_bCQCVoice = kCIDLib::False;
    m_bEventServer = kCIDLib::False;
    m_biTunes = kCIDLib::False;
    m_bIntfViewer = kCIDLib::False;
    m_bIntfViewerCtrl = kCIDLib::False;
    m_bLogicServer = kCIDLib::False;
    m_bMasterServer = kCIDLib::False;
    m_bMediaRepoMgr = kCIDLib::False;
    m_bTrayAutoStart = kCIDLib::False;
    m_bTrayMon = kCIDLib::False;
    m_bWebServer = kCIDLib::False;
    m_bXMLGWServer = kCIDLib::False;

    // Some other stuff gets cleared as well
    m_bCertSupport = kCIDLib::False;
    m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::None;
    m_eXMLGWSecureOpts = tCIDSock::ESecureOpts::None;
    m_strCertInfo.Clear();
    m_bSecureHelp = kCIDLib::False;
}


// Get/set the time stamp of the last upgrade
tCIDLib::TEncodedTime TCQCVerInstallInfo::enctLastUpdate() const
{
    return m_enctLastUpdate;
}

tCIDLib::TEncodedTime TCQCVerInstallInfo::enctLastUpdate(const tCIDLib::TEncodedTime enctToSet)
{
    m_enctLastUpdate = enctToSet;
    return m_enctLastUpdate;
}


// Get/set the CQC Voice reply style
tCQCKit::EReplyStyles TCQCVerInstallInfo::eCQCVoiceRepStyle() const
{
    return m_eCQCVoiceRepStyle;
}

tCQCKit::EReplyStyles
TCQCVerInstallInfo::eCQCVoiceRepStyle(const tCQCKit::EReplyStyles eToSet)
{
    m_eCQCVoiceRepStyle = eToSet;
    return m_eCQCVoiceRepStyle;
}


// Get/set the web server secure and insecure comms option
tCIDSock::ESecureOpts TCQCVerInstallInfo::eWebSecureOpts() const
{
    return m_eWebSrvSecureOpts;
}

tCIDSock::ESecureOpts TCQCVerInstallInfo::eWebSecureOpts(const tCIDSock::ESecureOpts eToSet)
{
    m_eWebSrvSecureOpts = eToSet;

    //
    //  We have to insure that secure help flag stays conherent with the options. If only secure
    //  then it has to be that. If insecure or none, it has to be insecure. Else it can stay as
    //  it is since both are available.
    //
    if (m_eWebSrvSecureOpts == tCIDSock::ESecureOpts::Secure)
    {
        // We only have to secure so it has to be that
        m_bSecureHelp = kCIDLib::True;
    }
     else if ((m_eWebSrvSecureOpts == tCIDSock::ESecureOpts::Insecure)
          ||  (m_eWebSrvSecureOpts == tCIDSock::ESecureOpts::None))
    {
        // It can't be secure anymore
        m_bSecureHelp = kCIDLib::False;
    }

    return eToSet;
}


// Get/set the XML GW server secure and insecure comms option
tCIDSock::ESecureOpts TCQCVerInstallInfo::eXMLGWSecureOpts() const
{
    return m_eXMLGWSecureOpts;
}

tCIDSock::ESecureOpts TCQCVerInstallInfo::eXMLGWSecureOpts(const tCIDSock::ESecureOpts eToSet)
{
    m_eXMLGWSecureOpts = eToSet;
    return eToSet;
}


// Set all the 'install it' flags
tCIDLib::TVoid TCQCVerInstallInfo::EnableAll()
{
    m_bAppCtrl = kCIDLib::True;
    m_bCmdPrompt = kCIDLib::True;
    m_bCQCClient = kCIDLib::True;
    m_bCQCServer = kCIDLib::True;
    m_bCQCVoice = kCIDLib::True;
    m_bEventServer = kCIDLib::True;
    m_bIntfViewer = kCIDLib::True;
    m_bIntfViewerCtrl = kCIDLib::True;
    m_bLogicServer = kCIDLib::True;
    m_bMasterServer = kCIDLib::True;
    m_bMediaRepoMgr = kCIDLib::True;
    m_bTrayAutoStart = kCIDLib::True;
    m_bTrayMon = kCIDLib::True;
    m_bWebServer = kCIDLib::True;
    m_bXMLGWServer = kCIDLib::True;

    // Clear some other things back to the default
    m_bCertSupport = kCIDLib::False;
    m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::None;
    m_eXMLGWSecureOpts = tCIDSock::ESecureOpts::None;
    m_eCQCVoiceRepStyle = tCQCKit::EReplyStyles::Spoken;

    // By default not on, since it causes trouble if iTunes is not actually available
    m_biTunes = kCIDLib::False;
}

// Get or set the App Shell service's ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnAppShell() const
{
    return m_ippnAppShell;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnAppShell(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnAppShell = ippnToSet;
    return m_ippnAppShell;
}


// Get or set the Cfg Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCfgServer() const
{
    return m_ippnCfgServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCfgServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnCfgServer = ippnToSet;
    return m_ippnCfgServer;
}


// Get or set the client service ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnClService() const
{
    return m_ippnClService;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnClService(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnClService = ippnToSet;
    return m_ippnClService;
}


// Get or set the Data Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnDataServer() const
{
    return m_ippnDataServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnDataServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnDataServer = ippnToSet;
    return m_ippnDataServer;
}


// Get or set the triggered event broadcast port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnEventBroadcast() const
{
    return m_ippnEventBroadcast;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnEventBroadcast(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnEventBroadcast = ippnToSet;
    return m_ippnEventBroadcast;
}


// Get or set the Event Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnEventServer() const
{
    return m_ippnEventServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnEventServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnEventServer = ippnToSet;
    return m_ippnEventServer;
}


// Get or set the IV remote control port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnIVCtrl() const
{
    return m_ippnIVCtrl;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnIVCtrl(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnIVCtrl = ippnToSet;
    return m_ippnIVCtrl;
}


// Get or set the CQCVoice ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCQCVoice() const
{
    return m_ippnCQCVoice;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCQCVoice(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnCQCVoice = ippnToSet;
    return m_ippnCQCVoice;
}


// Get or set the Log Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnLogServer() const
{
    return m_ippnLogServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnLogServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnLogServer = ippnToSet;
    return m_ippnLogServer;
}


// Get or set the Logic Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnLogicServer() const
{
    return m_ippnLogicServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnLogicServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnLogicServer = ippnToSet;
    return m_ippnLogicServer;
}


// Get or set the Name Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnNameServer() const
{
    return m_ippnNameServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnNameServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnNameServer = ippnToSet;
    return m_ippnNameServer;
}


// Get or set the CQC Server ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCQCServer() const
{
    return m_ippnCQCServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnCQCServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnCQCServer = ippnToSet;
    return m_ippnCQCServer;
}



// Get or set the Tray Monitor ORB port
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnTrayMon() const
{
    return m_ippnTrayMon;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnTrayMon(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnTrayMon = ippnToSet;
    return m_ippnTrayMon;
}


// Set or get the Admin (ORB oriented) port for the web server
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServer() const
{
    return m_ippnWebSrv;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnWebSrv = ippnToSet;
    return m_ippnWebSrv;
}


// Set or get the HTTP and HTTPS ports for the Web Server
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServerHTTP() const
{
    return m_ippnWebSrvHTTP;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServerHTTP(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnWebSrvHTTP = ippnToSet;
    return m_ippnWebSrvHTTP;
}


tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServerHTTPS() const
{
    return m_ippnWebSrvHTTPS;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnWebServerHTTPS(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnWebSrvHTTPS = ippnToSet;
    return m_ippnWebSrvHTTPS;
}


// Get or set the ORB port for the XML Gateway
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGWServer() const
{
    return m_ippnXMLGWServer;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGWServer(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnXMLGWServer = ippnToSet;
    return m_ippnXMLGWServer;
}


// Get or set the secure and non-secure ports for the XML Gateway
tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGW() const
{
    return m_ippnXMLGW;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGW(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnXMLGW = ippnToSet;
    return m_ippnXMLGW;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGWSecure() const
{
    return m_ippnXMLGWSecure;
}

tCIDLib::TIPPortNum TCQCVerInstallInfo::ippnXMLGWSecure(const tCIDLib::TIPPortNum ippnToSet)
{
    m_ippnXMLGWSecure = ippnToSet;
    return m_ippnXMLGWSecure;
}


//
//  Reset all the component related info to defaults.
//
//  !Leave the global stuff alone, like system id, master server address, etc...
//
tCIDLib::TVoid TCQCVerInstallInfo::Reset()
{
    m_bAppCtrl          = kCIDLib::False;
    m_bCertSupport      = kCIDLib::False;
    m_bCmdPrompt        = kCIDLib::False;
    m_bEventServer      = kCIDLib::False;
    m_bCQCClient        = kCIDLib::False;
    m_bCQCServer        = kCIDLib::False;
    m_biTunes           = kCIDLib::False;
    m_bIntfViewer       = kCIDLib::False;
    m_bIntfViewerCtrl   = kCIDLib::False;
    m_bCQCVoice         = kCIDLib::False;
    m_bLogicServer      = kCIDLib::False;
    m_bMasterServer     = kCIDLib::False;
    m_bMediaRepoMgr     = kCIDLib::False;
    m_bSecureHelp       = kCIDLib::False;
    m_bTrayAutoStart    = kCIDLib::False;
    m_bTrayMon          = kCIDLib::False;
    m_bWebServer        = kCIDLib::False;
    m_bXMLGWServer      = kCIDLib::False;
    m_ippnAppShell      = kCQCKit::ippnSrvShellDefPort;
    m_ippnCfgServer     = kCIDOrbUC::ippnCfgSrvDefPort;
    m_ippnClService     = kCQCKit::ippnClSrvDefPort;
    m_ippnDataServer    = kCQCKit::ippnDataSrvDefPort;
    m_ippnEventBroadcast= kCQCKit::ippnEventDefPort;
    m_ippnEventServer   = kCQCKit::ippnEvSrvDefPort;
    m_ippnIVCtrl        = kCQCKit::ippnIVRemCtrlDefPort;
    m_ippnCQCVoice      = kCQCKit::ippnCQCVoiceSrvDefPort;
    m_ippnLogServer     = kCIDOrbUC::ippnLogSrvDefPort;
    m_ippnLogicServer   = kCQCKit::ippnLogicSrvDefPort;
    m_ippnNameServer    = kCIDOrbUC::ippnNameSrvDefPort;
    m_ippnCQCServer     = kCQCKit::ippnCQCSrvDefPort;
    m_ippnTrayMon       = kCQCKit::ippnTrayMonDefPort;
    m_ippnWebSrv        = kCQCKit::ippnWebSrvDefPort;
    m_ippnWebSrvHTTP    = kCIDNet::ippnHTTP;
    m_ippnWebSrvHTTPS   = kCIDNet::ippnHTTPS;
    m_ippnXMLGWServer   = kCQCKit::ippnCQCGWSrvDefPort;
    m_ippnXMLGW         = kCQCKit::ippnCQCGWSrvPort;
    m_ippnXMLGWSecure   = kCQCKit::ippnCQCGWSrvSecPort;

    m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::None;
    m_eXMLGWSecureOpts = tCIDSock::ESecureOpts::None;
    m_eCQCVoiceRepStyle = tCQCKit::EReplyStyles::Spoken;
    m_strCertInfo.Clear();
    m_strCQCVoicePassword.Clear();
    m_strCQCVoiceUserName.Clear();
    m_strTrayBinding    = TSysInfo::strIPHostName();
}


// Get or set the backup dir
const TString& TCQCVerInstallInfo::strBackupDir() const
{
    return m_strBackupDir;
}

const TString& TCQCVerInstallInfo::strBackupDir(const TString& strToSet)
{
    m_strBackupDir = strToSet;
    return m_strBackupDir;
}


// Get or set the certificate info
const TString& TCQCVerInstallInfo::strCertInfo() const
{
    return m_strCertInfo;
}

const TString& TCQCVerInstallInfo::strCertInfo(const TString& strToSet)
{
    m_strCertInfo = strToSet;
    return m_strCertInfo;
}


// Get or set the CQC Voice password and user name
const TString& TCQCVerInstallInfo::strCQCVoicePassword() const
{
    return m_strCQCVoicePassword;
}

const TString& TCQCVerInstallInfo::strCQCVoicePassword(const TString& strToSet)
{
    m_strCQCVoicePassword = strToSet;
    return m_strCQCVoicePassword;
}

const TString& TCQCVerInstallInfo::strCQCVoiceUserName() const
{
    return m_strCQCVoiceUserName;
}

const TString& TCQCVerInstallInfo::strCQCVoiceUserName(const TString& strToSet)
{
    m_strCQCVoiceUserName = strToSet;
    return m_strCQCVoiceUserName;
}


// Get or set the master server host address
const TString& TCQCVerInstallInfo::strMSAddr() const
{
    return m_strMSAddr;
}

const TString& TCQCVerInstallInfo::strMSAddr(const TString& strToSet)
{
    m_strMSAddr = strToSet;
    return m_strMSAddr;
}


// Get or set the tray monitor name server binding
const TString& TCQCVerInstallInfo::strTrayBinding() const
{
    return m_strTrayBinding;
}

const TString& TCQCVerInstallInfo::strTrayBinding(const TString& strToSet)
{
    m_strTrayBinding = strToSet;
    return m_strTrayBinding;
}


// ---------------------------------------------------------------------------
//  TCQCVerInstallInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCVerInstallInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start obj marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_VerInstallInfo::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    //
    //  We have to eat and discard some options if we are reading in from a pre-v15
    //  version.
    //
    tCIDLib::TBoolean bDiscard;
    strmToReadFrom  >> m_bCmdPrompt;

    // Discard the old options for the driver test harnesses and separate intf designer
    if (c2FmtVersion < 15)
        strmToReadFrom >> bDiscard >> bDiscard;

    strmToReadFrom  >> m_bIntfViewer;

    // Discard the old separate macro ws
    if (c2FmtVersion < 15)
        strmToReadFrom >> bDiscard;

    strmToReadFrom  >> m_bMasterServer
                    >> m_bCQCClient
                    >> m_bCQCServer
                    >> m_bXMLGWServer
                    >> m_c4MajVer
                    >> m_c4MinVer
                    >> m_c4Revision
                    >> m_enctLastUpdate
                    >> m_ippnAppShell
                    >> m_ippnCfgServer
                    >> m_ippnDataServer
                    >> m_ippnLogServer
                    >> m_ippnNameServer
                    >> m_ippnCQCServer
                    >> m_ippnXMLGWServer
                    >> m_ippnXMLGW
                    >> m_strMSAddr;

    //
    //  Due to a change in strategy, if the master server flag is set, i.e.
    //  we are installing the master server on this machine, set the master
    //  server address to the local machine.
    //
    if (m_bMasterServer)
        m_strMSAddr = facCIDSock().strIPHostName();

    //
    //  If version 2 or greater, app control was added, so either read it
    //  in or default it.
    //
    //  But, we later subsumed that into a tray monitor program. So the
    //  port and binding were renamed to be tray monitor oriented.
    //
    if (c2FmtVersion >= 2)
    {
        strmToReadFrom >> m_bAppCtrl
                       >> m_ippnTrayMon
                       >> m_strTrayBinding;
    }
     else
    {
        m_bAppCtrl = kCIDLib::False;
        m_ippnTrayMon = kCQCKit::ippnTrayMonDefPort;
        m_strTrayBinding.Clear();
    }

    //
    //  If it's version 3 or greater, read in the events server stuff, else
    //  default them to not install it.
    //
    if (c2FmtVersion >= 3)
    {
        strmToReadFrom >> m_bEventServer
                       >> m_ippnEventServer;
    }
     else
    {
        m_bEventServer = kCIDLib::False;
        m_ippnEventServer = kCQCKit::ippnEventDefPort;
    }

    //
    //  If it's version is 4, then read in and throw away the standalone and
    //  NIC values, since they were both added and then deprecated during
    //  the 1.3.5 development process. V5 and up have already discarded
    //  these.
    //
    if (c2FmtVersion == 4)
    {
        tCIDLib::TBoolean bStandalone;
        TString           strNIC;
        strmToReadFrom >> bStandalone
                       >> strNIC;
    }

    if (c2FmtVersion >= 6)
    {
        strmToReadFrom >> m_bWebServer
                       >> m_ippnWebSrv
                       >> m_ippnWebSrvHTTP
                       >> m_ippnEventBroadcast;
    }
     else
    {
        m_bWebServer = kCIDLib::False;
        m_ippnWebSrv = kCQCKit::ippnWebSrvDefPort;
        m_ippnWebSrvHTTP = kCIDNet::ippnHTTP;
        m_ippnEventBroadcast = kCQCKit::ippnEventDefPort;
    }

    // And if V7 or greater, we get the IV control stuff, else we default it
    if (c2FmtVersion >= 7)
    {
        strmToReadFrom >> m_bIntfViewerCtrl >> m_ippnIVCtrl;
    }
     else
    {
        m_bIntfViewerCtrl = kCIDLib::False;
        m_ippnIVCtrl = kCQCKit::ippnIVRemCtrlDefPort;
    }

    //
    //  V8 added the old RIVA system. But now as of V19, it's gone again. So
    //  if V8 or beyond but before the version where we removed it, we read
    //  the remote viewer stuff into temps and drop them.
    //
    //  It also introduced the CQSL repo manager. That we still have to deal with.
    //
    if (c2FmtVersion >= 8)
    {
        strmToReadFrom  >> m_bMediaRepoMgr;
        if (c2FmtVersion < 19)
        {
            tCIDLib::TBoolean bTmp;
            tCIDLib::TIPPortNum ippnTmp;
            TString strTmp;
            strmToReadFrom  >> bTmp     // m_bRemVClient
                            >> bTmp     // m_bRemVServer
                            >> ippnTmp  // m_ippnRemVCImg
                            >> ippnTmp  // m_ippnRemVCView
                            >> ippnTmp  // m_ippnRemVSrvAdmin
                            >> ippnTmp  // m_ippnRemVSrvImg
                            >> ippnTmp  // m_ippnRemVSrvView
                            >> strTmp;  // m_strRemVSrvAddr
        }
    }
     else
    {
        m_bMediaRepoMgr     = kCIDLib::True;
    }

    // If V9 or greater, get the logic server stuff, else default it
    if (c2FmtVersion >= 9)
    {
        strmToReadFrom  >> m_bLogicServer
                        >> m_ippnLogicServer;
    }
     else
    {
        m_bLogicServer = kCIDLib::False;
        m_ippnLogicServer = kCQCKit::ippnLogicSrvDefPort;
    }

    //
    //  If version 10 or greater, or less than 20, get the system id and discard it.
    //  It was only used for licensing purposes, which no longer exists.
    //
    if ((c2FmtVersion >= 10) && (c2FmtVersion < 20))
    {
        TString strDiscardId;
        strmToReadFrom >> strDiscardId;
    }

    // If version 11 or greater, read in the last used backup dir, else default
    if (c2FmtVersion >= 11)
        strmToReadFrom >> m_strBackupDir;
    else
        m_strBackupDir.Clear();

    //
    //  If version 12 or greater, read in the iTunes and tray monitor
    //  options, else default them.
    //
    if (c2FmtVersion >= 12)
    {
        strmToReadFrom  >> m_biTunes
                        >> m_bTrayMon
                        >> m_bTrayAutoStart;
    }
     else
    {
        //
        //  In this case, since the tray monitor now provides app control,
        //  if app control is enabled, then enable the tray monitor. In order
        //  for it to be enabled, the old port and binding had to have been
        //  set, and therefore read into the new TM oriented versions of
        //  those methods.
        //
        m_biTunes = kCIDLib::False;
        m_bTrayMon = m_bAppCtrl;
        m_bTrayAutoStart = kCIDLib::False;
    }

    // If V13 or greater, get the client service stuff, else default it
    if (c2FmtVersion >= 13)
        strmToReadFrom >> m_ippnClService;
    else
        m_ippnClService = kCQCKit::ippnClSrvDefPort;

    //
    //  If V14 or beyond, get the new cert support and secure web server
    //  secure port, else default.
    //
    if (c2FmtVersion >= 14)
    {
        strmToReadFrom  >> m_bCertSupport
                        >> m_ippnWebSrvHTTPS
                        >> m_strCertInfo;
    }
     else
    {
        m_bCertSupport = kCIDLib::False;
        m_ippnWebSrvHTTPS = kCIDNet::ippnHTTPS;
        m_strCertInfo.Clear();

        if (m_bWebServer)
        {
            if (m_bCertSupport)
                m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::Both;
            else
                m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::Insecure;
        }
        else
        {
            m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::None;
        }
    }

    // If V16 or beyond, get the CQCVoice info, else default
    if (c2FmtVersion > 15)
    {
        strmToReadFrom  >> m_bCQCVoice
                        >> m_bCQCVoiceAutoStart
                        >> m_strCQCVoiceUserName
                        >> m_strCQCVoicePassword
                        >> m_ippnCQCVoice;
    }
     else
    {
        m_bCQCVoice = kCIDLib::False;
        m_bCQCVoiceAutoStart = kCIDLib::False;
        m_ippnCQCVoice = kCQCKit::ippnCQCVoiceSrvDefPort;
        m_strCQCVoiceUserName.Clear();
        m_strCQCVoicePassword.Clear();
    }

    //
    //  If V18 or beyond, read in CQCVoice reply style, else default. V17 got skipped, see
    //  the version comments at the top of the file.
    //
    if (c2FmtVersion > 17)
        strmToReadFrom >> m_eCQCVoiceRepStyle;
    else
        m_eCQCVoiceRepStyle = tCQCKit::EReplyStyles::Spoken;

    //
    //  If version 20 or greater read in the secure/insecure options and the new secure port
    //  for the XMLGW server. Else we need to default some stuff.
    //
    if (c2FmtVersion > 19)
    {
        strmToReadFrom  >> m_eXMLGWSecureOpts >> m_ippnXMLGWSecure
                        >> m_eWebSrvSecureOpts;
    }
     else
    {
        m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::None;
        if (m_bWebServer)
            m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::Insecure;
        if (m_bCertSupport)
            m_eWebSrvSecureOpts |= tCIDSock::ESecureOpts::Secure;

        m_eXMLGWSecureOpts = tCIDSock::ESecureOpts::None;
        if (m_bXMLGWServer)
            m_eXMLGWSecureOpts = tCIDSock::ESecureOpts::Insecure;
        if (m_bCertSupport)
            m_eXMLGWSecureOpts |= tCIDSock::ESecureOpts::Secure;
    }

    // The secure help flag was added
    if (c2FmtVersion > 20)
    {
        strmToReadFrom >> m_bSecureHelp;
    }
    else
    {
            // Default to likely candidate
        m_bSecureHelp =
        (
            (m_eWebSrvSecureOpts == tCIDSock::ESecureOpts::Secure)
            || (m_eWebSrvSecureOpts == tCIDSock::ESecureOpts::Both)
        );
    }

    //
    //  To help with upgrading from pre-tray monitor, if there's any flags
    //  that would make us install the tray monitor, but the binding isn't
    //  set, then default it to the local host.
    //
    if ((m_biTunes || m_bAppCtrl || m_bTrayMon) && m_strTrayBinding.bIsEmpty())
        m_strTrayBinding = TSysInfo::strIPHostName();

    //
    //  As of 5.x, the web server is always installed on the MS. It may not have been installed
    //  in earlier versions, so force it on.
    //
    if (m_bMasterServer)
    {
        // If not already enabled, enable it and make sure default ports are reasonable
        if (!m_bWebServer)
        {
            m_ippnWebSrvHTTP    = kCIDNet::ippnHTTP;
            m_ippnWebSrvHTTPS   = kCIDNet::ippnHTTPS;
            m_bWebServer        = kCIDLib::True;
            m_eWebSrvSecureOpts = tCIDSock::ESecureOpts::Insecure;

            // If certificate support is there, enable secure
            if (m_bCertSupport)
                m_eWebSrvSecureOpts |= tCIDSock::ESecureOpts::Secure;
        }
    }


    // Make sure we get the end obj marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCVerInstallInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_VerInstallInfo::c2FmtVersion
                    << m_bCmdPrompt
                    << m_bIntfViewer
                    << m_bMasterServer
                    << m_bCQCClient
                    << m_bCQCServer
                    << m_bXMLGWServer
                    << m_c4MajVer
                    << m_c4MinVer
                    << m_c4Revision
                    << m_enctLastUpdate
                    << m_ippnAppShell
                    << m_ippnCfgServer
                    << m_ippnDataServer
                    << m_ippnLogServer
                    << m_ippnNameServer
                    << m_ippnCQCServer
                    << m_ippnXMLGWServer
                    << m_ippnXMLGW
                    << m_strMSAddr

                    //  Version 2 stuff
                    << m_bAppCtrl
                    << m_ippnTrayMon
                    << m_strTrayBinding

                    // Version 3 stuff
                    << m_bEventServer
                    << m_ippnEventServer

                    //
                    //  Version 4 stuff was removed in V5, so there's no
                    //  V4 stuff to write.
                    //

                    // Version 6 stuff
                    << m_bWebServer
                    << m_ippnWebSrv
                    << m_ippnWebSrvHTTP
                    << m_ippnEventBroadcast

                    // Version 7 stuff
                    << m_bIntfViewerCtrl
                    << m_ippnIVCtrl

                    //
                    //  Version 8 stuff (old RIVA stuff was here before but removed as
                    //  of V19.
                    //
                    << m_bMediaRepoMgr

                    // Version 9 stuff
                    << m_bLogicServer
                    << m_ippnLogicServer

                    // Version 10 stuff (system id was dropped)

                    // Version 11 stuff
                    << m_strBackupDir

                    // Version 12 stuff
                    << m_biTunes
                    << m_bTrayMon
                    << m_bTrayAutoStart

                    // Version 13 stuff
                    << m_ippnClService

                    // Version 14 stuff
                    << m_bCertSupport
                    << m_ippnWebSrvHTTPS
                    << m_strCertInfo

                    // Version 16 stuff
                    << m_bCQCVoice
                    << m_bCQCVoiceAutoStart
                    << m_strCQCVoiceUserName
                    << m_strCQCVoicePassword
                    << m_ippnCQCVoice

                    // Version 18 stuff, 17 got skipped due to an error
                    << m_eCQCVoiceRepStyle

                    // Version 19 was just to remove old 3rd party RIVA stuff

                    //
                    //  Version 20 adds secure port for XML GW (and discarded V10 stuff), and
                    //  separates certificate support for whether secure comms are enabled for
                    //  Web or XML GW servers.
                    //
                    << m_eXMLGWSecureOpts
                    << m_ippnXMLGWSecure
                    << m_eWebSrvSecureOpts

                    // Version 21
                    << m_bSecureHelp

                    << tCIDLib::EStreamMarkers::EndObject;
}
