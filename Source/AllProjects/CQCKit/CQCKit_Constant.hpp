//
// FILE NAME: CQCKit_Constant.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2000
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
//  Toolkit constants namespace
// ---------------------------------------------------------------------------
namespace kCQCKit
{
    // -----------------------------------------------------------------------
    //  These represent the CQC release version, and are used anywhere in
    //  the CQC code that version is stored, displayed, etc... These come
    //  in from the build environment. All the reset of CQC use these and the
    //  encoded form of it below.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    constexpr tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    constexpr tCIDLib::TCard4   c4Revision      = CID_REVISION;
    const tCIDLib::TCh* const   pszVersion      = CIDLib_MakeLStr3(CID_VERSTRING);
    const tCIDLib::TCh* const   pszVerSuffix    = CIDLib_MakeLStr3(CID_VERSUFF);


    // -----------------------------------------------------------------------
    //  Some environment variables that holds our data directory and the unique
    //  system id. These are often passed in as a command line parm, but can
    //  be set as an env var.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const   pszDataDirVarName  = L"CQC_DATADIR";


    // -----------------------------------------------------------------------
    //  Our current version, and some important previous versions, in the
    //  combined format that we use for comparisons internally. It is in the
    //  form:
    //
    //      0x0000 0000 00000000
    //         maj min  revision
    //
    //  So it's a 64 bit value, with the maj/min values in the two halfs of
    //  the top word, and the revision in the lower word.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard8     c8Ver_Current
    (
        (tCIDLib::TCard8(CID_MAJVER) << 48)
        | (tCIDLib::TCard8(CID_MINVER) << 32)
        | tCIDLib::TCard8(CID_REVISION)
    );


    // -----------------------------------------------------------------------
    //  Driver related constants
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4               c4Modulus       = 23;
    constexpr const tCIDLib::TSCh* const    pszSrvFunc      = "psdrvMakeNewSrv";
    constexpr tCIDLib::TCard4               c4MaxArchVersion= 2;


    // -----------------------------------------------------------------------
    //  Some well known name server scopes and bindings for CQC. Some of these
    //  are stored on a per-host basis, so we have a replacement token for the
    //  host name.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const   pszCQCSlSrvIntfsScope   = L"/CQC/CQCClSrv/Intfs";
    constexpr const tCIDLib::TCh* const   pszCQCSlSrvIntf         = L"/CQC/CQCClSrv/Intfs/%(h)";
    constexpr const tCIDLib::TCh* const   pszCQCDataSrvScope      = L"/CQC/CQCDataServer";
    constexpr const tCIDLib::TCh* const   pszCQCDataSrvAdmin      = L"/CQC/CQCDataServer/CoreAdmin";
    constexpr const tCIDLib::TCh* const   pszCQCEventSrvAdmin     = L"/CQC/CQCEventSrv/CoreAdmin";
    constexpr const tCIDLib::TCh* const   pszCQCGWSrvCoreAdmin    = L"/CQC/CQCGWServer/CoreAdmins/%(h)";
    constexpr const tCIDLib::TCh* const   pszCQCVoiceSrvAdmin     = L"/CQC/CQCVoiceServer/CoreAdmin/%(h)";
    constexpr const tCIDLib::TCh* const   pszCQCVoiceSrvScope     = L"/CQC/CQCVoiceServer/CoreAdmin";
    constexpr const tCIDLib::TCh* const   pszCQCLogicSrvAdmin     = L"/CQC/LogicServer/CoreAdmin";
    constexpr const tCIDLib::TCh* const   pszCQCWebSrvCoreAdmin   = L"/CQC/CQCWebServer/CoreAdmins/%(h)";
    constexpr const tCIDLib::TCh* const   pszCQCWebSrvCoreAdmins  = L"/CQC/CQCWebServer/CoreAdmins";
    constexpr const tCIDLib::TCh* const   pszCQCTrayMonScope      = L"/CQC/CQCTrayMon/CoreAdmins";
    constexpr const tCIDLib::TCh* const   pszCQCTrayMonCoreAdmin  = L"/CQC/CQCTrayMon/CoreAdmins/%(b)";
    constexpr const tCIDLib::TCh* const   pszCQCTrayMonBrwsScope  = L"/CQC/CQCTrayMon/Browsers";
    constexpr const tCIDLib::TCh* const   pszCQCTrayMonBrowser    = L"/CQC/CQCTrayMon/Browsers/%(b)";


    // -----------------------------------------------------------------------
    //  Field related constants
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const   pszFld_False    = L"False";
    constexpr const tCIDLib::TCh* const   pszFld_True     = L"True";


    // -----------------------------------------------------------------------
    //  The default ORB related port numbers for our servers
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TIPPortNum   ippnDataSrvDefPort      = 13505;
    constexpr tCIDLib::TIPPortNum   ippnSrvShellDefPort     = 13506;
    constexpr tCIDLib::TIPPortNum   ippnCQCSrvDefPort       = 13507;
    constexpr tCIDLib::TIPPortNum   ippnCQCGWSrvDefPort     = 13508;
    constexpr tCIDLib::TIPPortNum   ippnCQCGWSrvPort        = 13509;
    constexpr tCIDLib::TIPPortNum   ippnTrayMonDefPort      = 13510;
    constexpr tCIDLib::TIPPortNum   ippnEvSrvDefPort        = 13511;
    constexpr tCIDLib::TIPPortNum   ippnIntfCtrlDefPort     = 13512;
    constexpr tCIDLib::TIPPortNum   ippnEventDefPort        = 13513;
    constexpr tCIDLib::TIPPortNum   ippnWebSrvDefPort       = 13514;
    constexpr tCIDLib::TIPPortNum   ippnLogicSrvDefPort     = 13519;
    constexpr tCIDLib::TIPPortNum   ippnClSrvDefPort        = 13520;
    constexpr tCIDLib::TIPPortNum   ippnCQCVoiceSrvDefPort  = 13521;
    constexpr tCIDLib::TIPPortNum   ippnCQCGWSrvSecPort     = 13522;
    constexpr tCIDLib::TIPPortNum   ippnIVRemCtrlDefPort    = 20000;


    // -----------------------------------------------------------------------
    //  The key names we install the app shell and client services under.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const   pszClServiceName    = L"CQCClService";
    constexpr const tCIDLib::TCh* const   pszCQCShellName     = L"CQCAppShell";


    // -----------------------------------------------------------------------
    //  The Windows registry stuff we use to keep up with the installed
    //  version. And, for servers, we need to store a user name and password that
    //  the server processes running on that machine can log in with. This is
    //  set by the installer (getting the info from the Master Server where it is
    //  stored by the Security Server interface.)
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszReg_Key      = L"Charmed Quark Systems, Ltd";
    constexpr const tCIDLib::TCh* const pszReg_Path     = L"TargetPath";
    constexpr const tCIDLib::TCh* const pszReg_Version  = L"Version";

    constexpr const tCIDLib::TCh* const pszReg_SrvUser  = L"ServerUser";
    constexpr const tCIDLib::TCh* const pszReg_SrvPass  = L"ServerPass";


    // -----------------------------------------------------------------------
    //  Server side, local cfg server keys. These are used by CQCServer or other servers
    //  besides the master server processes, and are stored on the local config server
    //  instance.
    //
    //  The IR config is used by IR related device drivers to store their config data.
    //  The moniker it appended to these paths to create per driver paths.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszOSKey_CQCSrvDriverCfgs = L"/CQC/CQCServer/Drivers/CfgStrs/";
    constexpr const tCIDLib::TCh* const pszOSKey_IRBlasterCfg     = L"/CQC/IRCfg/Blaster";
    constexpr const tCIDLib::TCh* const pszOSKey_IRReceiverCfg    = L"/CQC/IRCfg/Receiver";


    // -----------------------------------------------------------------------
    //  These are used by the Installation Server interface on the Master Server, and
    //  are keys into its private repository. These are things related to configuration
    //  of the system, which can't just be stored in the config server repo because they
    //  could have sensitive info in them. They are accessed via the Installation Server
    //  remote API.
    //
    //  The email accounts key holds the defined e-mail accounts, which are used when
    //  sending out e-mail from actions or automatically.
    //
    //  The drivers key contains a list of host names. Under those are the list of
    //  driver monikers for that host (the value of which is the driver config data.)
    //
    //  The LatLong key is where latitude and longitude values are stored, as a string in
    //  the form "1.1, 2.2", i.e. two floating point values formatted and separated by
    //  a comma.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszInstKey_Drivers              = L"/CQC/Drivers";
    constexpr const tCIDLib::TCh* const pszInstKey_EMailAcctScope       = L"/CQC/Security/EMailAccounts/";
    constexpr const tCIDLib::TCh* const pszInstKey_GC100Cfg             = L"/CQC/GC-100Cfg";
    constexpr const tCIDLib::TCh* const pszInstKey_JAPwrCfg             = L"/CQC/JAPwrCfg";
    constexpr const tCIDLib::TCh* const pszInstKey_LatLong              = L"/CQC/SysConfig/LatLong";


    // -----------------------------------------------------------------------
    //  'Roaming' user configuration. This is per-user stuff, and is also stored
    //  in the installation server's private repository and accessed via the
    //  installation server's API.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszInstKey_RoamBaseScope    = L"/CQC/Roam/Users/";
    constexpr const tCIDLib::TCh* const pszInstKey_RoamUserScope    = L"/CQC/Roam/Users/%(1)/";
    constexpr const tCIDLib::TCh* const pszInstKey_KeyMaps          = L"/CQC/Roam/Users/%(1)/KeyMaps";
    constexpr const tCIDLib::TCh* const pszInstKey_UserVarList      = L"/CQC/Roam/Users/%(1)/VarList";


    // -----------------------------------------------------------------------
    //  These are used by the Security Server interface on the Master and are keys into
    //  its private repository, and of course it contains sensitive information. These
    //  values are accessed via the Security Server remote API.
    //
    //  The master hash key holds a hash that the security server uses to encrypt user
    //  credentials it hands out (and decrypt them when it gets them back.)
    //
    //  The user account info is of course the CQC user accounts.
    //
    //  The event key is the key that event triggers are encrypted with when sent
    //  and decrypted with when received. And the event system id is used to uniquely
    //  identify the triggers sent out from a given system.
    // -----------------------------------------------------------------------
    constexpr const tCIDLib::TCh* const pszSecKey_CQCSecurityScope  = L"/CQC/Security";
    constexpr const tCIDLib::TCh* const pszSecKey_UserAccountInfo   = L"/CQC/Security/UserAccounts";
    constexpr const tCIDLib::TCh* const pszSecKey_MasterHash        = L"/CQC/Security/MasterHash";
    constexpr const tCIDLib::TCh* const pszSecKey_EventKey          = L"/CQC/Security/EventKey";
    constexpr const tCIDLib::TCh* const pszSecKey_EventSysId        = L"/CQC/Security/EventSysId";


    // -----------------------------------------------------------------------
    //  Some markers used in field I/O polling. These are put into the return
    //  data to indicate whether a field's data changed or not, and to indicate
    //  what type of record is next in the buffer. We also put a format version
    //  in the buffer, to allow for future multiple versions, and define here
    //  the version for the current version of CQC>
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard1   c1FldData_Unchanged         = 0xAC;
    constexpr tCIDLib::TCard1   c1FldData_Changed           = 0xCA;
    constexpr tCIDLib::TCard1   c1FldData_InError           = 0x9C;

    constexpr tCIDLib::TCard1   c1FldType_DriverOnline      = 0xB7;
    constexpr tCIDLib::TCard1   c1FldType_DriverOffline     = 0xD3;
    constexpr tCIDLib::TCard1   c1FldType_Field             = 0x7B;

    const tCIDLib::TCard1       c1FldFmtVersion             = 1;


    // -----------------------------------------------------------------------
    //  The size of thumbnail images stored in the image repository. This is
    //  mostly a user interface related value, but other stuff needs to know
    //  this size as a convenience, so it's stored here.
    // -----------------------------------------------------------------------
    const TSize szThumbnail(64, 48);


    // -----------------------------------------------------------------------
    //  The max number of filters a triggered event can have
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MaxTEvFilters(4);


    // -----------------------------------------------------------------------
    //  The maximum number of fields a driver can have
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4    c4MaxDrvFields = 8192;
}

#pragma CIDLIB_POPPACK


