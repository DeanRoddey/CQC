//
// FILE NAME: CQCKit_DriverCfgDTD.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/20/2001
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
//  This file contains the text for the DTD that govern the definition of a
//  driver configuration (a driver manifest file as they are called.) When the
//  TCQCDriverCfg class parses an XML file that defines a driver installation
//  config, it uses this DTD.
//
//  This string is a static member of the TCQCDriverCfg class.
//
//  As of the pre-4.0 betas, there is now a version 2 driver architecture,
//  which standardizes many more things about drivers than was previously
//  the case. There is now a version attributes on the main driver config
//  element. It's defaulted to 1. If set to 2, then we expect to see the
//  version 2 form of the manifest.
//
//  The differences are fairly small. Instead of a single device class,
//  they can now indicate multiple device classes they implement.
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
#include    "CQCKit_.hpp"


const TString TCQCDriverCfg::s_strEmbeddedDTDText =
L"<?xml encoding='$NativeWideChar$'?>\n"

//
//  Define some version entities, that can be used by manifest files. In many
//  cases, a build tree will build multiple drivers, all of which want to
//  version themselves the same as the release version. We have to play some
//  tricks ehre, because these are provided as defines from the build
//  environment. We have to turn them into wide string constants, using some
//  CIDLib provided macros, and depending upon the automatic concatenation of
//  contiguous string constants.
//
L"<!ENTITY  Build_Version '"
            CIDLib_MakeLStr3(CID_MAJVER) L"." CIDLib_MakeLStr3(CID_MINVER) L"'>\n"

//
//  Define some entities we'll use below. These have to be defined before
//  they are used.
//

// The types of drivers we have
L"<!ENTITY % DrvTypes 'CppLib | Macro | GenProto'>\n"

//  The available baud rates
L"<!ENTITY % BaudRates  'Baud_110 | Baud_300 | Baud_600 | Baud_1200 | Baud_2400\n"
L"                      | Baud_4800 | Baud_9600 | Baud_14400 | Baud_19200 \n"
L"                      | Baud_38400 | Baud_56000 | Baud_57600 | Baud_115200'>\n"

// The types of connections we support
L"<!ENTITY % ConnTypes  'CQCCfg:SerialConn | CQCCfg:IPConn\n"
L"                       | CQCCfg:OtherConn | CQCCfg:UPnPConn'>\n"

// The content model for the root element
L"<!ENTITY % DrvCfgSections 'CQCCfg:ServerCfg, CQCCfg:CommonCfg\n"
L"                          , CQCCfg:ClientCfg?, CQCCfg:ConnCfg\n"
L"                          , CQCCfg:OtherCfg?'>\n"


//
//  An IP style device provides a port, protocol, address/host, and address
//  family type. The address is given by the user when the driver is installed,
//  so it's not indicated here as part of the fixed configuration. If it
//  really is a fixed address, the driver will just code it internally.
//
//  The family defaults to IPV4 for backwards compatibility, since this was
//  recently added and most manifests won't include it explicitly.
//
L"<!ENTITY % AddrTypes 'Unspecified | IPV4 | IPV6'>\n"
L"<!ENTITY % SockProtos 'SockProto_IP | SockProto_UDP | SockProto_TCP'>\n"
L"<!ELEMENT CQCCfg:IPConn EMPTY>\n"
L"<!ATTLIST CQCCfg:IPConn\n"
L"          CQCCfg:Port CDATA #REQUIRED\n"
L"          CQCCfg:AddrType (%AddrTypes;) 'Unspecified'\n"
L"          CQCCfg:SockProto (%SockProtos;) #REQUIRED>\n"


//
//  A serial style device provides the usual port config info. For 422 type
//  connections, the specific address is not indicated here since, like the
//  port number it is instance specific and is indicated during install of
//  each instance. However we do need to let them indicate the 232/422 type
//  here so we'll know to ask for it.
//
L"<!ELEMENT CQCCfg:SerialConn EMPTY>\n"
L"<!ATTLIST CQCCfg:SerialConn\n"
L"          CQCCfg:Type (RS232 | RS422) 'RS232'\n"
L"          CQCCfg:Bits (Bits4 | Bits5 | Bits6 | Bits7 | Bits8) #REQUIRED\n"
L"          CQCCfg:Baud (%BaudRates;) #REQUIRED\n"
L"          CQCCfg:Parity (Odd | Even | Mark | Space | None) #REQUIRED\n"
L"          CQCCfg:StopBits (One | OnePtFive | Two) #REQUIRED\n"
L"          CQCCfg:DTRFlag (Disable | Enable | Handshake) 'Enable'\n"
L"          CQCCfg:RTSFlag (Disable | Enable | Handshake) 'Enable'\n"
L"          CQCCfg:ParityCheck (Yes | No) 'Yes'\n"
L"          CQCCfg:CTSOut (Yes | No) 'No'\n"
L"          CQCCfg:DSROut (Yes | No) 'No'\n"
L"          CQCCfg:XIn (Yes | No) 'No'\n"
L"          CQCCfg:XOut (Yes | No) 'No'>\n"


//
//  A fallback connection configuration type, for devices that use something
//  that's not explicitly covered by a dedicated derived class. It just holds
//  PCDATA content that is passed along as is.
//
L"<!ELEMENT   CQCCfg:OtherConn (#PCDATA)>\n"


//
//  An UPnP style device provides a unique device id for the device. The
//  associated driver install wizard panel lets them do a search for the
//  type of device indicated in the DevType attribute. So the dev type is
//  fixed for the type of driver, and it is used to look up the UID which is
//  a specific instance of the device type. The type desc is a very short
//  one or two words generally) description of the type, like "Media Player"
//  and so forth. This is used by the UPnP connection configuration dialog
//  for display purposes.
//
//  In some cases the value that they want us to display when selecting
//  from the available UPnP devices during driver installation is not the
//  name of the actual devices, but that gotten from some contained service.
//  So they can provide the UID of the service and the name of a state
//  variable to be queried to get the name. If not provided, then the basic
//  name of the actual device will be displayed.
//
L"<!ELEMENT CQCCfg:UPnPConnDisp EMPTY>\n"
L"<!ATTLIST CQCCfg:UPnPConnDisp\n"
L"          CQCCfg:SvcType CDATA #REQUIRED\n"
L"          CQCCfg:VarName CDATA #REQUIRED>\n"

L"<!ELEMENT CQCCfg:UPnPConn (CQCCfg:UPnPConnDisp?)>\n"
L"<!ATTLIST CQCCfg:UPnPConn\n"
L"          CQCCfg:DevUID CDATA ''\n"
L"          CQCCfg:DevName CDATA ''\n"
L"          CQCCfg:DevType CDATA #REQUIRED\n"
L"          CQCCfg:TypeDescr CDATA #REQUIRED>\n"


//
//  This is the top level element. It defines all of the components that
//  make up the installation configuration for a driver. These are:
//
//  1.  The server driver config. This is required because if there is no
//      server driver, there's no point.
//          - The driver type, which indicates whether its a C++ binary
//            library, or a macro based one.
//          - The server library/macro class path, according to the type
//            field. If a Cpp driver, it's the library name. If a macro
//            based driver, its the class path.
//          - Extra device config. This is an open ended string that will
//            just be passed along to the driver who can parse it.
//          - For V2 and beyond drivers, a set of capabilities flags, which
//            can be one or more of these, separated by commas. For V1 drivers
//            it defaults to None, so no special capabilities.
//
//              - AutoLoc  : This driver needs no help finding it's associated
//                           device. So something like a known web address
//                           or a fixed IP address, or it's an internal
//                           device or something.
//              - DevProbe : This driver supports the device sensing driver
//                           callback method so the system can use it to try
//                           to automatically find the device by trying each
//                           serial port or IP port available. Ignored if
//                           AutoLoc is set.
//
//  2.  Common config. This section holds stuff common to both server and
//      client side drivers. Its required.
//          - The library version info
//          - The make of the device controlled
//          - The device category of the device controlled
//          - The model of the device controlled
//          - A display string for display to the user when selecting from
//            a list of drivers
//          - A longer descriptive string that describes the driver
//
//  3.  The client driver info. This is optional, since you don't need a
//      client side driver if you just want to make it available for macro
//      access or custom program access.
//          - The driver type, which indicates whether its a C++ binary
//            library, or a macro based one.
//          - The client library/macro class path, according to the type
//            field. If it's a macro, we know what the library name is since
//            its got to the be generic driver client.
//          - Extra device config. This is an open ended string that will
//            just be passed along to the driver who can parse it.
//
//  4.  Connection type info. This is optional, though usually present. It
//      tells the server driver how to configure itself to communicate to
//      its device. Its optional because some drivers have no options so
//      the connection parameters are just hard coded into the driver. Even
//      so, drivers are encouraged to provide this section, and just indicate
//      that they cannot be edited. This allows the user to see the config
//      during device load time.
//
//      This info can have one of a set of device configurations, each of
//      which describes the config for a particular type of connection, e.g.
//      serial, USB, IP, etc...
//
//      This does not indicate specific connection points (e.g. ports) since
//      this is just to indicate the connection info, but each specific
//      instance of the device will be set up to talk to a particular point
//      when its loaded.
//
//  5.  Other stuff. This contains some miscellaneous stuff. It is optional
//      since many to most drivers won't use it.
//          - A list of other files that must be installed along with the
//            server driver. It indicates the name of the file from the
//            installation package, and whether it goes to the client or
//            server directory.
//          - Common facilities. This allows the client and server to share
//            another DLL/SharedLib facility that holds code, or data, or
//            resources they both need. Its just a list of facility names.
//            It is assumed that these are not dynamically loaded, so they
//            are just put somewhere in the path, so that any CQC drivers
//            loaded within the client or server will be able to do implicit
//            loading of them (optional)
//
//  As of CQC 4.4 (actually the betas leading up to it), there is a new version 2 of
//  the driver architecture, which standardizes stuff more. So we added a version
//  attribute, which defaults to 1 for backwards compability. New drivers will set
//  it to 2. And there is more info provided for V2 drivers on the server config
//  element, which are just
//  defaulted for V1 driver compabilitity.
//
L"<!ELEMENT CQCCfg:DriverCfg (%DrvCfgSections;)>\n"
L"<!ATTLIST CQCCfg:DriverCfg\n"
L"          CQCCfg:ArchVer CDATA '1'>\n"


// The server config element
L"<!ELEMENT CQCCfg:ServerCfg EMPTY>\n"
L"<!ATTLIST CQCCfg:ServerCfg\n"
L"          CQCCfg:DrvType (%DrvTypes;) #REQUIRED\n"
L"          CQCCfg:LibName CDATA #REQUIRED\n"
L"          CQCCfg:ExtraCfg CDATA ''\n"
L"          CQCCfg:DynRef CDATA ''>\n"


// The common config element
L"<!ELEMENT CQCCfg:CommonCfg EMPTY>\n"
L"<!ATTLIST CQCCfg:CommonCfg\n"
L"          CQCCfg:Author CDATA 'Dean Roddey'\n"
L"          CQCCfg:Category CDATA #REQUIRED\n"
L"          CQCCfg:Contact CDATA 'support@charmedquark.com'\n"
L"          CQCCfg:Description CDATA #REQUIRED\n"
L"          CQCCfg:DisplayName CDATA #REQUIRED\n"
L"          CQCCfg:Make CDATA #REQUIRED\n"
L"          CQCCfg:Model CDATA #REQUIRED\n"
L"          CQCCfg:Version CDATA #REQUIRED\n"
L"          CQCCfg:DevClasses CDATA 'None'\n"
L"          CQCCfg:Caps CDATA ''\n"
L"          CQCCfg:AsyncMode (Yes | No) 'No'\n"
L"          CQCCfg:AsyncFldOver (Yes | No) 'Yes'>\n"


// The client config element
L"<!ELEMENT CQCCfg:ClientCfg EMPTY>\n"
L"<!ATTLIST CQCCfg:ClientCfg\n"
L"          CQCCfg:DrvType (%DrvTypes;) #REQUIRED\n"
L"          CQCCfg:LibName CDATA #REQUIRED\n"
L"          CQCCfg:ExtraCfg CDATA ''>\n"


//
//  The connection configuration element. It holds one of the possible
//  connection configuration elements. You can indicate whether it can be
//  edited during instance installation.
//
L"<!ELEMENT CQCCfg:ConnCfg (%ConnTypes;)>\n"
L"<!ATTLIST CQCCfg:ConnCfg\n"
L"          CQCCfg:Editable (True | False) 'False'>\n"


// The 'other stuff' element
L"<!ELEMENT CQCCfg:OtherCfg (CQCCfg:ExtraFiles*\n"
L"                          , CQCCfg:ExtraFacs*\n"
L"                          , CQCCfg:CfgPrompts?\n"
L"                          , CQCCfg:FixedCfgVals?\n"
L"                          , CQCCfg:AutoSenseClass?)>\n"

L"<!ELEMENT CQCCfg:AutoSenseClass (#PCDATA)>\n"

L"<!ELEMENT CQCCfg:ExtraFiles EMPTY>\n"
L"<!ATTLIST CQCCfg:ExtraFiles\n"
L"          CQCCfg:Path CDATA #REQUIRED\n"
L"          CQCCfg:Location (Client | Server) #REQUIRED>\n"


L"<!ELEMENT CQCCfg:ExtraFacs EMPTY>\n"
L"<!ATTLIST CQCCfg:ExtraFacs\n"
L"          CQCCfg:BaseName NMTOKEN #REQUIRED\n"
L"          CQCCfg:Version CDATA ''"
L"          CQCCfg:Type (Exe | SharedObj | SharedLib) #REQUIRED\n"
L"          CQCCfg:UsedBy (Client | Server | Shared) #REQUIRED\n"
L"          CQCCfg:MsgFile (Yes | No) #REQUIRED\n"
L"          CQCCfg:ResFile (Yes | No) #REQUIRED>\n"


//
//  The user prompts. These are used to get information from the user during
//  driver installation. The driver install wizard understands these and will
//  get the info and pass to the driver's initialization method.
//
//  Each prompt has attributes that indicate the title and name, and some
//  flags of various sorts. And each one contains a prompt text object that
//  holds the text that should be displayed to the user to explain what
//  the value is, and an optional prompt defaults object that provides defaults
//  for any of the sub-values that that type of prompt contains.
//
L"<!ELEMENT CQCCfg:PromptDef EMPTY>\n"
L"<!ATTLIST CQCCfg:PromptDef\n"
L"          CQCCfg:ValName CDATA #REQUIRED\n"
L"          CQCCfg:Default CDATA #IMPLIED>\n"

L"<!ELEMENT CQCCfg:PromptText (#PCDATA)>\n"

//
//  Application control/binding prompt for app control drivers. Gets the
//  remote path for the target file. The driver manifest must provide the
//  correct search criteria to only find the target file(s). Selecting a
//  target path can be option or required.
//
L"<!ELEMENT CQCCfg:CfgACBPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgACBPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:SearchCrit CDATA #REQUIRED\n"
L"          CQCCfg:Optional (Yes | No) 'No'\n"
L"          CQCCfg:PathOnly (Yes | No) 'Yes'>\n"

// Select from the available DirectShow audio renderer devices
L"<!ELEMENT CQCCfg:CfgADPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgADPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

//
//  Boolean choice, value is true or false. The driver manifest provides the
//  text for the check box.
//
L"<!ELEMENT CQCCfg:CfgBPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgBPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:CheckText CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

// Single selection list. The value is the selected item from the item list
L"<!ELEMENT CQCCfg:CfgLPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgLPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:ItemList CDATA #REQUIRED>\n"

//
//  A file prompt, which lets them select a file. Search criteria can be
//  indicate to limit the search to particular files. Selecting a file can
//  be optional or required.
//
L"<!ELEMENT CQCCfg:CfgFPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgFPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Optional (Yes | No) 'No'\n"
L"          CQCCfg:SearchCrit CDATA '*.*'\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

//
//  Like a list prompt above, but it's a multi-select list, not single select.
//  The value is a comma separated list of the selected indices. No default
//  value for this one.
//
//  To support automatic generation of numered lists, the NumRange can be used.
//  If present it should have a value like "1, 4" and the ItemList value should
//  have a value like "Item %(1)". The values in this case would be Item 1,
//  Item 2, Item3, Item 4.
//
L"<!ELEMENT CQCCfg:CfgMLPrompt (CQCCfg:PromptText)>\n"
L"<!ATTLIST CQCCfg:CfgMLPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:ItemList CDATA #REQUIRED\n"
L"          CQCCfg:NumRange CDATA #IMPLIED>\n"

// An ODBC source selection prompt
L"<!ELEMENT CQCCfg:CfgODBCPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgODBCPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

//
//  A path prompt, which allows them enter a path. Can be forced to be fully
//  qualified.
//
L"<!ELEMENT CQCCfg:CfgPPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgPPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Optional (Yes | No) 'No'\n"
L"          CQCCfg:FQPath (Yes | No) 'Yes'\n"
L"          CQCCfg:Validate (Yes | No) 'No'\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

// A range prompt for numerics, with a high and low range limit.
L"<!ELEMENT CQCCfg:CfgRPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgRPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:LowVal CDATA #REQUIRED\n"
L"          CQCCfg:HighVal CDATA #REQUIRED>\n"

// Lets the select from the list of installed media repository drivers
L"<!ELEMENT CQCCfg:CfgRepoPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgRepoPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

// Just lets them enter text. A reg expression can be used to validate it
L"<!ELEMENT CQCCfg:CfgTPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgTPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:RegEx CDATA ''>\n"

// Select from the available TTS voices
L"<!ELEMENT CQCCfg:CfgTTSVPrompt (CQCCfg:PromptText, CQCCfg:PromptDef*)>\n"
L"<!ATTLIST CQCCfg:CfgTTSVPrompt\n"
L"          CQCCfg:Title CDATA #REQUIRED\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

//
//  A nameable list prompt. So this is simplar to MLPrompt, but instead of having
//  pre-provided names, the user can provide the names. By naming one, the user
//  effectively enables it, adding it to the resulting value. The max number of
//  values must be provided.
//
L"<!ELEMENT CQCCfg:CfgNMLPrompt (CQCCfg:PromptText)>\n"
L"<!ATTLIST CQCCfg:CfgNMLPrompt\n"
L"          CQCCfg:Title  CDATA #REQUIRED\n"
L"          CQCCfg:Count  CDATA #REQUIRED\n"
L"          CQCCfg:Spaces (Yes | No) 'No'\n"
L"          CQCCfg:Name   CDATA #REQUIRED>\n"

// The list of possible prompts
L"<!ELEMENT CQCCfg:CfgPrompts (CQCCfg:CfgACBPrompt\n"
L"                             | CQCCfg:CfgADPrompt\n"
L"                             | CQCCfg:CfgBPrompt\n"
L"                             | CQCCfg:CfgFPrompt\n"
L"                             | CQCCfg:CfgLPrompt\n"
L"                             | CQCCfg:CfgPPrompt\n"
L"                             | CQCCfg:CfgMLPrompt\n"
L"                             | CQCCfg:CfgNMLPrompt\n"
L"                             | CQCCfg:CfgODBCPrompt\n"
L"                             | CQCCfg:CfgRPrompt\n"
L"                             | CQCCfg:CfgRepoPrompt\n"
L"                             | CQCCfg:CfgTPrompt\n"
L"                             | CQCCfg:CfgTTSVPrompt)+>\n"

//
//  The fixed values, which are hard coded into the manifest file. The
//  overall value has a name, and contains one or more fixed values. The
//  overall fixed config values element contains one or more fixed values.
//
L"<!ELEMENT CQCCfg:FixedValItem EMPTY>\n"
L"<!ATTLIST CQCCfg:FixedValItem\n"
L"          CQCCfg:Name CDATA #REQUIRED\n"
L"          CQCCfg:Value CDATA #REQUIRED>\n"

L"<!ELEMENT CQCCfg:FixedVal (CQCCfg:FixedValItem+)>\n"
L"<!ATTLIST CQCCfg:FixedVal\n"
L"          CQCCfg:Name CDATA #REQUIRED>\n"

L"<!ELEMENT CQCCfg:FixedCfgVals (CQCCfg:FixedVal+)>\n";

