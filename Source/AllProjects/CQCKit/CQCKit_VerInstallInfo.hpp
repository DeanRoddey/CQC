//
// FILE NAME: CQCKit_VerInstallInfo.hpp
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
//  This class holds some installation information that is written to the
//  main install directory during installation, and read back during installs
//  to see what version and components are currently installed.
//
//  As of 5.x, we consolidated almost all client tools into a single Admin Interface
//  program. So some the previous separate client options went away.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TCQCVerInstallInfo
//  PREFIX: vii
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCVerInstallInfo : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCVerInstallInfo();

        TCQCVerInstallInfo(const TCQCVerInstallInfo&) = default;
        TCQCVerInstallInfo(TCQCVerInstallInfo&&) = default;

        ~TCQCVerInstallInfo();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCVerInstallInfo& operator=(const TCQCVerInstallInfo&) = default;
        TCQCVerInstallInfo& operator=(TCQCVerInstallInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAnyClient() const;

        tCIDLib::TBoolean bAnyServer() const;

        tCIDLib::TBoolean bAppCtrl() const;

        tCIDLib::TBoolean bAppCtrl
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCertSupport() const;

        tCIDLib::TBoolean bCertSupport
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCmdPrompt() const;

        tCIDLib::TBoolean bCmdPrompt
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCQCClient() const;

        tCIDLib::TBoolean bCQCClient
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCQCServer() const;

        tCIDLib::TBoolean bCQCServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCQCVoice() const;

        tCIDLib::TBoolean bCQCVoice
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bCQCVoiceAutoStart() const;

        tCIDLib::TBoolean bCQCVoiceAutoStart
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bEventServer() const;

        tCIDLib::TBoolean bEventServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean biTunes() const;

        tCIDLib::TBoolean biTunes
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIntfViewer() const;

        tCIDLib::TBoolean bIntfViewer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIntfViewerCtrl() const;

        tCIDLib::TBoolean bIntfViewerCtrl
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bLogicServer() const;

        tCIDLib::TBoolean bLogicServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bMasterServer() const;

        tCIDLib::TBoolean bMasterServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bMediaRepoMgr() const;

        tCIDLib::TBoolean bMediaRepoMgr
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bTrayAutoStart() const;

        tCIDLib::TBoolean bTrayAutoStart
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bTrayMon() const;

        tCIDLib::TBoolean bTrayMon
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bWebServer() const;

        tCIDLib::TBoolean bWebServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bXMLGWServer() const;

        tCIDLib::TBoolean bXMLGWServer
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4MajVersion() const;

        tCIDLib::TCard4 c4MajVersion
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4MinVersion() const;

        tCIDLib::TCard4 c4MinVersion
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Revision() const;

        tCIDLib::TCard4 c4Revision
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard8 c8MajMinVersion() const;

        tCIDLib::TCard8 c8Version() const;

        tCIDLib::TVoid DisableAll();

        tCIDLib::TEncodedTime enctLastUpdate() const;

        tCIDLib::TEncodedTime enctLastUpdate
        (
            const   tCIDLib::TEncodedTime   enctToSet
        );

        tCQCKit::EReplyStyles eCQCVoiceRepStyle() const;

        tCQCKit::EReplyStyles eCQCVoiceRepStyle
        (
            const   tCQCKit::EReplyStyles   eToSet
        );

        tCIDSock::ESecureOpts eWebSecureOpts() const;

        tCIDSock::ESecureOpts eWebSecureOpts
        (
            const   tCIDSock::ESecureOpts   eToSet
        );

        tCIDSock::ESecureOpts eXMLGWSecureOpts() const;

        tCIDSock::ESecureOpts eXMLGWSecureOpts
        (
            const   tCIDSock::ESecureOpts   eToSet
        );

        tCIDLib::TVoid EnableAll();

        tCIDLib::TIPPortNum ippnAppShell() const;

        tCIDLib::TIPPortNum ippnAppShell
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnCfgServer() const;

        tCIDLib::TIPPortNum ippnCfgServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnClService() const;

        tCIDLib::TIPPortNum ippnClService
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnCQCServer() const;

        tCIDLib::TIPPortNum ippnCQCServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnCQCVoice() const;

        tCIDLib::TIPPortNum ippnCQCVoice
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnDataServer() const;

        tCIDLib::TIPPortNum ippnDataServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnEventBroadcast() const;

        tCIDLib::TIPPortNum ippnEventBroadcast
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnEventServer() const;

        tCIDLib::TIPPortNum ippnEventServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnIVCtrl() const;

        tCIDLib::TIPPortNum ippnIVCtrl
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnLogServer() const;

        tCIDLib::TIPPortNum ippnLogServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnLogicServer() const;

        tCIDLib::TIPPortNum ippnLogicServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnNameServer() const;

        tCIDLib::TIPPortNum ippnNameServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnTrayMon() const;

        tCIDLib::TIPPortNum ippnTrayMon
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnWebServer() const;

        tCIDLib::TIPPortNum ippnWebServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnWebServerHTTP() const;

        tCIDLib::TIPPortNum ippnWebServerHTTP
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnWebServerHTTPS() const;

        tCIDLib::TIPPortNum ippnWebServerHTTPS
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnXMLGWServer() const;

        tCIDLib::TIPPortNum ippnXMLGWServer
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnXMLGW() const;

        tCIDLib::TIPPortNum ippnXMLGW
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TIPPortNum ippnXMLGWSecure() const;

        tCIDLib::TIPPortNum ippnXMLGWSecure
        (
            const   tCIDLib::TIPPortNum     ippnToSet
        );

        tCIDLib::TVoid Reset();

        const TString& strBackupDir() const;

        const TString& strBackupDir
        (
            const   TString&                strToSet
        );

        const TString& strCertInfo() const;

        const TString& strCertInfo
        (
            const   TString&                strToSet
        );

        const TString& strCQCVoicePassword() const;

        const TString& strCQCVoicePassword
        (
            const   TString&                strToSet
        );

        const TString& strCQCVoiceUserName() const;

        const TString& strCQCVoiceUserName
        (
            const   TString&                strToSet
        );

        const TString& strMSAddr() const;

        const TString& strMSAddr
        (
            const   TString&                strToSet
        );

        const TString& strTrayBinding() const;

        const TString& strTrayBinding
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bXXXX
        //      These indicate which components are to be installed (or which one
        //      were installed last time if this object represents previous install
        //      info.)
        //
        //  m_bCertSupport
        //      If true, then m_strCertInfo has certificate info to be used. Only used by
        //      servers that have secure comms enabled.
        //
        //  m_c4MajVer
        //  m_c4MinVer
        //      The major and minor version that is currently installed.
        //
        //  m_c4Revision
        //      The revision number of the currently installed version. This servces
        //      to distinguish between patch releases which cannot be figured out by
        //      looking at the files.
        //
        //  m_enctLastUpdate
        //      The last time this info was updated.
        //
        //  m_eCQCVoiceRepStyle
        //      If CQC Voice is enabled, this indicates the types of replies it will
        //      provide. It can be spoken, visual, or both. At least one is required.
        //
        //  m_eWebSrvSecureOpts
        //  m_eXMLGWSecureOpts
        //      Options for the servers that support secure and/or non-secure connections, to
        //      track which ones are enabled. This replaces some old booleans, and at that
        //      time only the secure ones were optional.
        //
        //  m_ippnXXX
        //      The listening ports of the various CQC servers. They will be initialized to
        //      their defaults as defined by CQCKit and CIDOrb.
        //
        //  m_strBackupDir
        //      We offer to do a backup of the current system before doing an upgrade.
        //      This is the last backup directory they asked us to use, if any. If not,
        //      it's empty.
        //
        //  m_strCertInfo
        //      If certificate based secure connections are supported, this must be provided
        //      and point to a valid certificate. The web server and XML gateway can enable use
        //      of this certificate. If this is used, m_bCertSupport is set.
        //
        //  m_strMSAddr
        //      The master server address, if any. Its only used if the m_bMasterServer
        //      flag is clear, i.e. if the master server is not being installed on the
        //      current target machine.
        //
        //  strTrayBinding
        //      The name server binding for the tray monitor program, if it is installed.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAppCtrl;
        tCIDLib::TBoolean       m_bCertSupport;
        tCIDLib::TBoolean       m_bCmdPrompt;
        tCIDLib::TBoolean       m_bCQCClient;
        tCIDLib::TBoolean       m_bCQCServer;
        tCIDLib::TBoolean       m_bEventServer;
        tCIDLib::TBoolean       m_biTunes;
        tCIDLib::TBoolean       m_bIntfViewer;
        tCIDLib::TBoolean       m_bIntfViewerCtrl;
        tCIDLib::TBoolean       m_bCQCVoice;
        tCIDLib::TBoolean       m_bCQCVoiceAutoStart;
        tCIDLib::TBoolean       m_bLogicServer;
        tCIDLib::TBoolean       m_bMasterServer;
        tCIDLib::TBoolean       m_bMediaRepoMgr;
        tCIDLib::TBoolean       m_bTrayAutoStart;
        tCIDLib::TBoolean       m_bTrayMon;
        tCIDLib::TBoolean       m_bWebServer;
        tCIDLib::TBoolean       m_bXMLGWServer;
        tCIDLib::TCard4         m_c4MajVer;
        tCIDLib::TCard4         m_c4MinVer;
        tCIDLib::TCard4         m_c4Revision;
        tCIDLib::TEncodedTime   m_enctLastUpdate;
        tCQCKit::EReplyStyles   m_eCQCVoiceRepStyle;
        tCIDSock::ESecureOpts   m_eWebSrvSecureOpts;
        tCIDSock::ESecureOpts   m_eXMLGWSecureOpts;
        tCIDLib::TIPPortNum     m_ippnAppShell;
        tCIDLib::TIPPortNum     m_ippnCfgServer;
        tCIDLib::TIPPortNum     m_ippnClService;
        tCIDLib::TIPPortNum     m_ippnDataServer;
        tCIDLib::TIPPortNum     m_ippnEventBroadcast;
        tCIDLib::TIPPortNum     m_ippnEventServer;
        tCIDLib::TIPPortNum     m_ippnIVCtrl;
        tCIDLib::TIPPortNum     m_ippnCQCVoice;
        tCIDLib::TIPPortNum     m_ippnLogServer;
        tCIDLib::TIPPortNum     m_ippnLogicServer;
        tCIDLib::TIPPortNum     m_ippnNameServer;
        tCIDLib::TIPPortNum     m_ippnCQCServer;
        tCIDLib::TIPPortNum     m_ippnTrayMon;
        tCIDLib::TIPPortNum     m_ippnWebSrv;
        tCIDLib::TIPPortNum     m_ippnWebSrvHTTP;
        tCIDLib::TIPPortNum     m_ippnWebSrvHTTPS;
        tCIDLib::TIPPortNum     m_ippnXMLGWServer;
        tCIDLib::TIPPortNum     m_ippnXMLGW;
        tCIDLib::TIPPortNum     m_ippnXMLGWSecure;
        TString                 m_strBackupDir;
        TString                 m_strCertInfo;
        TString                 m_strCQCVoicePassword;
        TString                 m_strCQCVoiceUserName;
        TString                 m_strMSAddr;
        TString                 m_strTrayBinding;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVerInstallInfo,TObject)
};

#pragma CIDLIB_POPPACK

