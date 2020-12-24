//
// FILE NAME: CQCGWCl_GWStructs.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/29/2002
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
//  This header defines some structures used by the 'gateway server', a
//  non-ORB based server that provides raw socket access to PocketPC or other
//  third party servers who want to access CQC data.
//
//  The interface is very simple. Basically, it is implemented in terms of a
//  simple packet structure, with mostly text based data. Each packet has
//  the following structure:
//
//      - c4MagicVal1
//      - Packet sequence number
//      - Data Size
//      - Data checksum
//      - MagicVal2
//      - DataBytes[]
//
//  The details of these fields will be described below where the structure is
//  defined. The structure is byte packed.
//
// CAVEATS/GOTCHAS:
//
//  1)  Because of the nature of the users of this header, i.e. they are not
//      CIDLib based, this header is completely standalone and does not use
//      any other CIDLib or CQC Toolkit headers. It provides it's own little
//      set of typedefs, which must be modified by the user of the header in
//      order to create the correct data types for it's platform.
//
//  2)  All header fields are assumed to be in 'little endian' format.
//
// LOG:
//

// The types namespace
namespace tCQCGWSrv
{
    // -----------------------------------------------------------------------
    //  Modify these typedefs to create the appropriate data types for your
    //  platform. They are:
    //
    //  TCh     - Native wide character type
    //  TCard1  - Cardinal (unsigned), 8 bit value
    //  TCard2  - Cardinal (unsigned), 16 bit value
    //  TCard4  - Cardinal (unsigned), 32 bit value
    // -----------------------------------------------------------------------
    using TCh  = wchar_t;
    using TCard1  = unsigned char;
    using TCard2  = unsigned short;
    using TCard4  = unsigned int;


    // -----------------------------------------------------------------------
    //  The packet structure, with a faux array for the data bytes field, so
    //  that this structure can be conveniently pointed at a raw buffer and
    //  used to create the required structure.
    //
    //  NOTE:   It is byte packed for on the wire compabitility.
    //
    //  The field details are:
    //
    //  c4MagicVal1, c4MagicVal2 - Self explanatory really. Mainly just there
    //  as a sanity check to help recognize a legal packet.
    //
    //  c4SeqNum - An ever increasing packet number, to help watch for missed
    //  packets. It should start at 1 on a new connect, and be bumped for each
    //  new packet sent. Replies will contain the sequence number being
    //  replied to.
    //
    //  c4DataSize - The number of bytes in the data bytes array.
    //
    //  c1Flags - Some flags that are used to indicate attributes of the
    //  packet data. These flags are not used yet, but will be soon for
    //  supporting things like encryption of the data stream.
    //
    //  c2CheckSum - The sum of all of the data bytes, allowing it to just
    //  naturally overflow if the sum is greater than the field can hold, i.e.
    //  the sum mod 0xFFFF.
    //
    //  ac1DataBytes[] - The actual payload data. It will hold XML text, the
    //  meaning of which is dependent upon the particular message being sent.
    //  The text encoding of the XML is up to the sender, as long as the
    //  encoding="" setting is set correctly to allow CQC to transcode it to
    //  it's internal Unicode format.
    // -----------------------------------------------------------------------
    #pragma pack(push,1)
    struct TPacketHdr
    {
        TCard4  c4MagicVal1;
        TCard4  c4SeqNum;
        TCard4  c4DataSize;
        TCard2  c2CheckSum;
        TCard1  c1Flags;
        TCard4  c4MagicVal2;
    };

    struct TPacket
    {
        TCard4  c4MagicVal1;
        TCard4  c4SeqNum;
        TCard4  c4DataSize;
        TCard2  c2CheckSum;
        TCard1  c1Flags;
        TCard4  c4MagicVal2;
        TCard1  ac1DataBytes[1];
    };
    #pragma pack(pop)
}


// The constants namespace
namespace kCQCGWSrv
{
    // -----------------------------------------------------------------------
    //  The magic values used in the header
    // -----------------------------------------------------------------------
    constexpr tCQCGWSrv::TCard4 c4MagicVal1 = 0xDEADBEEF;
    constexpr tCQCGWSrv::TCard4 c4MagicVal2 = 0xFEEDBEAD;


    // -----------------------------------------------------------------------
    //  The maximum data bytes in a single packet
    // -----------------------------------------------------------------------
    constexpr tCQCGWSrv::TCard4 c4MaxDataBytes  = 0x400000;


    // -----------------------------------------------------------------------
    //  Bits for the c1Flags member of the packet header
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard1   c1Flag_Encrypted    = 0x0001;


    // -----------------------------------------------------------------------
    //  The document type statement that should be used in all passed documents.
    // -----------------------------------------------------------------------
    const tCQCGWSrv::TCh* const  pszDTD   = L"<!DOCTYPE CQCGW:Msg PUBLIC 'urn:charmedquark.com:CQC-GWProtocol.DTD' 'CQCGWProtocol.DTD'>\n";
    const tCQCGWSrv::TCh* const  pszPubId = L"urn:charmedquark.com:CQC-GWProtocol.DTD";
    const tCQCGWSrv::TCh* const  pszSysId = L"CQCGWProtocol.DTD";


    // -----------------------------------------------------------------------
    //  The DTD for the command structure. The DTD is hard coded in this
    //  header in order to insure that the correct version is used. The encoding
    //  must be set on external entities, so we set it to a magic value
    //  intended to represent the native wide character type, which is what
    //  we should get from the L"" type strings we use here.
    // -----------------------------------------------------------------------
    const tCQCGWSrv::TCh* const pszDTDText =
    {
        L"<?xml encoding='$NativeWideChar$' ?>\n"

        // An entity that holds the valid data types of a field
        L"<!ENTITY % FldTypes 'MEng.Boolean | MEng.Card4 | MEng.Int4\n"
        L"                     | MEng.Float8 | MEng.String | MEng.StringList\n"
        L"                     | MEng.Time'>\n\n"

        // An entity for the read/write options
        L"<!ENTITY % AccessTypes 'None | Read | Write | ReadWrite'>\n"

        // An entity for the 'set options' flags
        L"<!ENTITY % OptFlags 'NoAlpha | ImgInfo'>\n"

        // An entity for any paused / resumed states
        L"<!ENTITY % PauseState 'Resumed | Paused'>\n"

        // An entity for the status options of a field
        L"<!ENTITY % FldStatusList 'CQCGW:Online | CQCGW:InError'>\n"

        // An entity for success/failure status attribute values
        L"<!ENTITY % SuccessVals 'CQCGW:Success | CQCGW:Failure'>\n"

        // An entity for common yes/no and true/false type attribute values
        L"<!ENTITY % YesNoVal 'Yes | No'>\n"
        L"<!ENTITY % BoolVal 'True | False'>\n"

        // An entity for driver architecture version
        L"<!ENTITY % DrvArchVer '1 | 2'>\n"

        //
        //  There is a generic query message for querying things that don't
        //  need any other data except what is being asked for. This provides
        //  the valid values for that message's attribute that indicates what
        //  is being queried.
        //
        L"\n"
        L"<!ENTITY % QueryList 'CQCGW:QueryDrvList | CQCGW:QueryFieldList\n"
        L"                      | CQCGW:QueryGlobActs | CQCGW:QueryMacros\n"
        L"                      | CQCGW:Poll | CQCGW:QueryRepoDrvs\n"
        L"                      | CQCGW:QueryTemplates | CQCGW:QueryRmCfgList\n"
        L"                      | CQCGW:Ping'>\n\n"

        // An entity that provides the list of client connection codes
        L"<!ENTITY % ConnCodes 'CQCGW:Accepted | CQCGW:MaxClients'>\n\n"

        // The list of possible elements in a message
        L"<!ENTITY % MsgList\n"
        L"           'CQCGW:ConnRes | CQCGW:DeviceList | CQCGW:Disconnect\n"
        L"          | CQCGW:SetPollList | CQCGW:PollReply\n"
        L"          | CQCGW:LogonReq | CQCGW:LogonChallenge\n"
        L"          | CQCGW:GetSecurityToken | CQCGW:ExceptionReply\n"
        L"          | CQCGW:AckReply | CQCGW:NakReply\n"
        L"          | CQCGW:Query | CQCGW:MacroScope | CQCGW:DoGlobalAct\n"
        L"          | CQCGW:GActScope | CQCGW:DrvList\n"
        L"          | CQCGW:RunMacro | CQCGW:MacroResults\n"
        L"          | CQCGW:FldValue\n"
        L"          | CQCGW:TemplateScope\n"
        L"          | CQCGW:UnknownExceptionReply | CQCGW:SetOpts\n"
        L"          | CQCGW:QueryFldInfo | CQCGW:FldInfo\n"
        L"          | CQCGW:QueryFldInfoList | CQCGW:FldInfoList\n"
        L"          | CQCGW:QueryImage | CQCGW:WriteField\n"
        L"          | CQCGW:ImageData | CQCGW:QueryDriverStatus\n"
        L"          | CQCGW:DriverStatus | CQCGW:ReadField\n"
        L"          | CQCGW:MWriteField | CQCGW:QueryDriverText\n"
        L"          | CQCGW:DriverText\n | CQCGW:QueryDriverInfo\n"
        L"          | CQCGW:DriverInfo\n"

        L"          | CQCGW:PerEvInfo | CQCGW:SchEvInfo | CQCGW:SunEvInfo \n"
        L"          | CQCGW:SetPerEv | CQCGW:SetSchEv | CQCGW:SetSunEv \n"
        L"          | CQCGW:QuerySchEv | CQCGW:PauseSchEv\n"
        L"          | CQCGW:QuerySchEvs\n"

        L"          | CQCGW:QueryMediaRendArt | CQCGW:QueryMediaArt\n"
        L"          | CQCGW:MediaImgData\n"
        L"          | CQCGW:QueryRendPL | CQCGW:RendPLInfo\n"
        L"          | CQCGW:QueryMediaDB | CQCGW:MediaDBInfo\n"

        L"          | CQCGW:RoomCfgReq | CQCGW:RoomCfgRep\n"
        L"          | CQCGW:RoomList'\n"
        L">\n"


        //
        //  Gemeric Ack/Nak replies that can be used by either side where
        //  appropriate. Sometimes no specific message is needed, just an
        //  agreement or disagreement. The nak can have an optional text
        //  attribute indicating the reason for the disagreement if needed.
        //  The ack also has an optional attribute where extra context can
        //  be returned, but mostly it's not used.
        //
        L"<!ELEMENT  CQCGW:AckReply EMPTY>\n"
        L"<!ATTLIST  CQCGW:AckReply\n"
        L"           CQCGW:Info CDATA ''>\n\n"
        L"<!ELEMENT  CQCGW:NakReply EMPTY>\n"
        L"<!ATTLIST  CQCGW:NakReply\n"
        L"           CQCGW:ReasonText CDATA ''>\n\n"

        //
        //  Upon connection, the server will send one of these back. The code
        //  will indicate whether the connection is accepted or not. Currently,
        //  the only denial code is that the server is maxed out on gateway
        //  clients.
        //
        //  The version info indicates the version of CQC that the server is
        //  running, so that the client can adapt or refuse to connect.
        //
        L"<!ELEMENT  CQCGW:ConnRes EMPTY>\n"
        L"<!ATTLIST  CQCGW:ConnRes\n"
        L"           CQCGW:ConnStatus (%ConnCodes;) #REQUIRED\n"
        L"           CQCGW:CQCVer CDATA #REQUIRED>\n\n"


        //
        //  The client should send this when it wants to disconnect, instead of
        //  just closing the socket. This lets the server know that it's not
        //  a connection failure, but a graceful shutdown. It will return an
        //  ack and drop the connection. Since the socket has linger set, you will
        //  be able to read the ack though the server has dropped it by the time
        //  you see it.
        //
        L"<!ELEMENT  CQCGW:Disconnect EMPTY>\n"


        // Defines a CQC field definition
        L"<!ELEMENT  CQCGW:FieldDef EMPTY>\n"
        L"<!ATTLIST  CQCGW:FieldDef\n"
        L"           CQCGW:Name CDATA #REQUIRED\n"
        L"           CQCGW:Type (%FldTypes;) #REQUIRED\n"
        L"           CQCGW:SemType CDATA 'Generic'\n"
        L"           CQCGW:AccessType (%AccessTypes;) #REQUIRED\n"
        L"           CQCGW:Limits CDATA ''>\n\n"

        //
        //  Defines a CQC device. It holds zero or more fields. For 4.0.9 we
        //  add the device category. Default it to misc, so that older servers
        //  will still work with newer clients.
        //
        L"<!ELEMENT  CQCGW:DeviceDef (CQCGW:FieldDef*)>\n"
        L"<!ATTLIST  CQCGW:DeviceDef\n"
        L"           CQCGW:Moniker NMTOKEN #REQUIRED\n"
        L"           CQCGW:DevCat CDATA 'Miscellaneous'>\n\n"

        // For when a field name is needed. The value is in the mon.fld format
        L"<!ELEMENT  CQCGW:FldName EMPTY>\n"
        L"<!ATTLIST  CQCGW:FldName\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n"

        // For when a driver name is needed. The value is just the driver moniker
        L"<!ELEMENT  CQCGW:DrvMon EMPTY>\n"
        L"<!ATTLIST  CQCGW:DrvMon\n"
        L"           CQCGW:Moniker CDATA #REQUIRED>\n"

        // When a list of monikers is needed
        L"<!ELEMENT  CQCGW:DrvList (CQCGW:DrvMon*)>\n"
        L"<!ATTLIST  CQCGW:DrvList\n"
        L"           CQCGW:Count CDATA #REQUIRED>\n"

        //
        //  Defines a full list of device drivers available and the fields
        //  that they provide. So it's a list of DeviceDef elements, each
        //  of which provides a list of their field defs. There might not be
        //  any loaded devices, so we use zero or more for the content model.
        //
        //  This is returned for the general queries QueryDrvList and QueryFieldList
        //  but in the former case it is empty, i.e. just the device definitions
        //  with no fields inside them.
        //
        L"<!ELEMENT  CQCGW:DeviceList (CQCGW:DeviceDef*)>\n\n"

        //
        //  This block defines a 'Set Poll List' operation. The client is
        //  usually monitoring some list of fields (one list at a time per
        //  client), so they must tell us which fields that they want to
        //  poll. Calling it again just overwrites the previous poll list.
        //  Calling it with an empty list, turns off polling for the client.
        //
        //  If the RetInfo flag isn't set, then they just get an Ack or
        //  Nak back. If they set the flag, then the return is the same as
        //  if they'd done a field info query on the whole list, i.e. a
        //  FldInfoList message with info on all the fields, in the order
        //  that they were in the poll list.
        //
        L"<!ELEMENT  CQCGW:SetPollList (CQCGW:FldName*)>\n"
        L"<!ATTLIST  CQCGW:SetPollList\n"
        L"           CQCGW:RetInfo (Yes|No) 'No'>\n\n"

        //
        //  This block defines the response to a "Poll Query" operation. It
        //  returns current value/status information on the fields in the
        //  current poll list.
        //
        //  Note that only those fields whose values have changed since the
        //  last call will be returned, for efficiency. If the list is reset
        //  by a SetPollList call, then the next poll query will return all
        //  the current values, and then subsequently start returning only
        //  changed values. So if the PollReply element has no children, there
        //  have been no changes.
        //
        //  NOTE:   A change in status online/offline, is a value change as
        //          well and will cause it to be returned.
        //
        L"<!ELEMENT  CQCGW:FldValue EMPTY>\n"
        L"<!ATTLIST  CQCGW:FldValue\n"
        L"           CQCGW:Field CDATA #REQUIRED\n"
        L"           CQCGW:Status (%FldStatusList;) #REQUIRED\n"
        L"           CQCGW:Value CDATA #REQUIRED\n"
        L"           CQCGW:Index CDATA #REQUIRED>\n"

        L"<!ELEMENT  CQCGW:PollReply (CQCGW:FldValue*)>\n\n"

        //
        //  This block defines the message sent to invoke a macro. It has an
        //  attribute for the name of the macro to run, and a child element
        //  to hold each parameter.
        //
        //  And it defines the reply message. We can't just send an ack/nack
        //  back, since we have to return any output from the macro. The Status
        //  attribute holds the success or failure status. If it indicates
        //  success, the MacroRetVal attribute holds the numeric return value
        //  from the macro. Either way, the body of the holds either the output
        //  from the macro engine (if it failed) or the output from the macro
        //  itself if it was successful and put out anything.
        //
        L"<!ELEMENT  CQCGW:MacroParm (#PCDATA)>\n"
        L"<!ELEMENT  CQCGW:RunMacro (CQCGW:MacroParm*)>\n"
        L"<!ATTLIST  CQCGW:RunMacro\n"
        L"           CQCGW:ClassPath CDATA #REQUIRED>\n"
        L"<!ELEMENT  CQCGW:MacroResults (#PCDATA)>\n"
        L"<!ATTLIST  CQCGW:MacroResults\n"
        L"           CQCGW:Status (%SuccessVals;) #REQUIRED\n"
        L"           CQCGW:MacroRetVal CDATA #REQUIRED>\n\n"

        //
        //  This block defines the message sent to read a field by name.
        //  It includes the name of the device and the name of the field.
        //  This is for one-off field reads, not for rapid polling. A
        //  CQCGW:FldValue element is returned. If the returned Status
        //  attribute indicates that the field is online, then the value
        //  returned is valid.
        //
        L"<!ELEMENT  CQCGW:ReadField EMPTY>\n"
        L"<!ATTLIST  CQCGW:ReadField\n"
        L"           CQCGW:Field CDATA #REQUIRED>\n\n"

        //
        //  This block defines the message sent to write a value to a field.
        //  It includes the name of the device, the name of the field, and
        //  the value to write.
        //
        L"<!ELEMENT  CQCGW:WriteField EMPTY>\n"
        L"<!ATTLIST  CQCGW:WriteField\n"
        L"           CQCGW:Field CDATA #REQUIRED\n"
        L"           CQCGW:Value CDATA #REQUIRED>\n\n"

        //
        //  This block defines the message sent to write multiple fields
        //  in a single operation. It contains one or more write field
        //  elements.
        //
        L"<!ELEMENT  CQCGW:MWriteField (CQCGW:WriteField+)>\n"

        //
        //  Query some core info about a driver. The body text of the
        //  reply is a comma separated list of device classes the
        //  driver implements.
        //
        L"<!ELEMENT  CQCGW:QueryDriverInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:QueryDriverInfo\n"
        L"           CQCGW:Moniker CDATA #REQUIRED>\n\n"

        L"<!ELEMENT  CQCGW:DriverInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:DriverInfo\n"
        L"           CQCGW:Status (Online | Offline) #REQUIRED\n"
        L"           CQCGW:ArchVer (%DrvArchVer;) #REQUIRED\n"
        L"           CQCGW:Make CDATA #REQUIRED\n"
        L"           CQCGW:Model CDATA #REQUIRED>\n\n"

        //
        //  This block defines the message sent to query the status of a
        //  given driver. You pass in the moniker of the device you want
        //  to check.
        //
        //  The return is a DriverStatus block, which returns the moniker
        //  and the status of the driver.
        //
        L"<!ELEMENT  CQCGW:QueryDriverStatus EMPTY>\n"
        L"<!ATTLIST  CQCGW:QueryDriverStatus\n"
        L"           CQCGW:Moniker CDATA #REQUIRED>\n\n"

        L"<!ELEMENT  CQCGW:DriverStatus EMPTY>\n"
        L"<!ATTLIST  CQCGW:DriverStatus\n"
        L"           CQCGW:Moniker CDATA #REQUIRED\n"
        L"           CQCGW:Status (Online | Offline) #REQUIRED>\n\n"

        //
        //  This is used to do a backdoor driver text value query, so
        //  that XML GW clients can make such queries of drivers where
        //  needed.
        //
        L"<!ELEMENT  CQCGW:QueryDriverText EMPTY>\n"
        L"<!ATTLIST  CQCGW:QueryDriverText\n"
        L"           CQCGW:Moniker CDATA #REQUIRED\n"
        L"           CQCGW:QType CDATA #REQUIRED\n"
        L"           CQCGW:DataName CDATA #REQUIRED>\n\n"

        L"<!ELEMENT  CQCGW:DriverText (#PCDATA)>\n"
        L"<!ATTLIST  CQCGW:DriverText\n"
        L"           CQCGW:Status (%BoolVal;) #REQUIRED>\n\n"


        //
        //  This block defines the message sent by clients to set or clear
        //  certain optional behaviours of the server. They will get an ACK
        //  back. It contains one of the options and a true/false flag to
        //  set or clear the option.
        //
        L"<!ELEMENT  CQCGW:SetOpts EMPTY>\n"
        L"<!ATTLIST  CQCGW:SetOpts\n"
        L"           CQCGW:OptName (%OptFlags;) #REQUIRED\n"
        L"           CQCGW:State (%BoolVal;) #REQUIRED>\n\n"


        //
        //  This block defines the message sent by clients to invoke a
        //  global action. They indicate the path of the action, plus any
        //  parameters they want to pass. So it contains zero or more
        //  ActParm elements. An AckReply or NakReply will come back.
        //
        L"<!ELEMENT  CQCGW:ActParm (#PCDATA)>\n"
        L"<!ELEMENT  CQCGW:DoGlobalAct (CQCGW:ActParm)*>\n"
        L"<!ATTLIST  CQCGW:DoGlobalAct\n"
        L"           CQCGW:ActPath CDATA #REQUIRED>\n\n"

        //
        //  This block defines the response to the CQCGW:QueryGlobActs query.
        //  It returns the list of currently defined global actions. This is a
        //  hierarchical structure. So it starts with a scope, which can in turn
        //  hold zero or more other scopes or global actions.
        //
        L"<!ELEMENT  CQCGW:GActName EMPTY>\n"
        L"<!ATTLIST  CQCGW:GActName\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n"
        L"<!ELEMENT  CQCGW:GActScope (CQCGW:GActScope*, CQCGW:GActName*)>\n"
        L"<!ATTLIST  CQCGW:GActScope\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n\n"

        //
        //  This block defines an logon request that the client sends to the
        //  server. The user name is passed. The server will send back a
        //  LogonChallenge, and the client must then send back a
        //  GetSecurityToken. If the returned challenge data is good, a
        //  AckReply is returned, else a NakReply is returned.
        //
        //  In the challenge and security token calls, the data is a buffer of
        //  binary data, formatted into a hex text format. So it is guaranteed
        //  to be all ASCII Hex digits. It will be randomly generated key,
        //  challenge data, and session key. See the protocol documentation
        //  for details.
        //
        //  The GetSecurityToken call must be made within 5 seconds of the
        //  LogonChallenge being provided to the client. Else, it will time
        //  out. This is to keep someone from working on breaking a challenge
        //  offline and playing it back later.
        //
        //  It can optionally include the values of the 1 through 9 standard
        //  environment variables that are available in CQC actions. The
        //  VarNum attribute should be a number 1 through 9 to indicate
        //  which variable it is, so you only need to pass the ones that
        //  actually have values.
        //
        L"<!ELEMENT  CQCGW:LogonReq EMPTY>\n"
        L"<!ATTLIST  CQCGW:LogonReq\n"
        L"           CQCGW:UserName CDATA #REQUIRED\n"
        L"           CQCGW:Encrypted (%YesNoVal;) 'No'\n"
        L"           CQCGW:CType CDATA #IMPLIED>\n\n"

        L"<!ELEMENT  CQCGW:LogonChallenge EMPTY>\n"
        L"<!ATTLIST  CQCGW:LogonChallenge\n"
        L"           CQCGW:ChallengeKey CDATA #REQUIRED\n"
        L"           CQCGW:ChallengeData CDATA #REQUIRED\n"
        L"           CQCGW:SessKey CDATA #IMPLIED>\n\n"

        L"<!ELEMENT  CQCGW:EnvVar (#PCDATA)>\n"
        L"<!ATTLIST  CQCGW:EnvVar\n"
        L"           CQCGW:VarNum CDATA #REQUIRED>\n"
        L"<!ELEMENT  CQCGW:GetSecurityToken (CQCGW:EnvVar*)>\n"
        L"<!ATTLIST  CQCGW:GetSecurityToken\n"
        L"           CQCGW:ResponseData CDATA #REQUIRED>\n\n"

        //
        //  Query info about a particular field in the current poll list, and
        //  get a response back.
        //
        L"<!ELEMENT  CQCGW:QueryFldInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:QueryFldInfo\n"
        L"           CQCGW:Field CDATA #REQUIRED>\n\n"

        L"<!ELEMENT  CQCGW:FldInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:FldInfo\n"
        L"           CQCGW:Type (%FldTypes;) #REQUIRED\n"
        L"           CQCGW:SemType CDATA 'Generic'\n"
        L"           CQCGW:Access CDATA #REQUIRED\n"
        L"           CQCGW:Limits CDATA ''>\n\n"

        //
        //  Query info about a list of fields, not necessarily in the polling
        //  list. It returns a list of field info elements, of the same sort
        //  as the single field query above returns. The return list is in
        //  the same order as the incoming list of names.
        //
        L"<!ELEMENT  CQCGW:QueryFldInfoList (CQCGW:FldName+)>\n"
        L"<!ELEMENT  CQCGW:FldInfoList (CQCGW:FldInfo*)>\n"

        //
        //  These define the query for an image, and the image data that is
        //  returned. The returned image data is a PNG image file in Base64
        //  format as the element content of the ImageData message. There are
        //  some optional attrs on the reply, based on the ImgInfo options
        //  flag being set or not.
        //
        L"<!ELEMENT  CQCGW:QueryImage EMPTY>\n"
        L"<!ATTLIST  CQCGW:QueryImage\n"
        L"           CQCGW:ImagePath CDATA #REQUIRED\n"
        L"           CQCGW:SerialNum CDATA #REQUIRED>\n\n"

        L"<!ELEMENT  CQCGW:ImageData (#PCDATA)>\n"
        L"<!ATTLIST  CQCGW:ImageData\n"
        L"           CQCGW:Flags CDATA #IMPLIED\n"
        L"           CQCGW:Depth CDATA #IMPLIED\n"
        L"           CQCGW:TransClr CDATA #IMPLIED\n"
        L"           CQCGW:SerialNum CDATA #REQUIRED>\n\n"


        //
        //  These define the reply info from a query for available templates,
        //  which is done via the generic CQCGW:Query message. This one is
        //  a recursive tree of the hierarchical scopes and template names.
        //
        L"<!ELEMENT  CQCGW:TemplateName EMPTY>\n"
        L"<!ATTLIST  CQCGW:TemplateName\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n"
        L"<!ELEMENT  CQCGW:TemplateScope (CQCGW:TemplateScope*, CQCGW:TemplateName*)>\n"
        L"<!ATTLIST  CQCGW:TemplateScope\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n\n"


        //
        //  This block defines the response to the CQCGW:QueryMacros query.
        //  It returns the list of currently defined macros. This is a
        //  hierarchical structure mapping to the CML classpath structure
        //  of the macro system. So it starts with a scope, which can in turn
        //  hold zero or more other scopes or macros. Scopes are returned
        //  first, then macros.
        //
        L"<!ELEMENT  CQCGW:MacroName EMPTY>\n"
        L"<!ATTLIST  CQCGW:MacroName\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n"
        L"<!ELEMENT  CQCGW:MacroScope (CQCGW:MacroScope*, CQCGW:MacroName*)>\n"
        L"<!ATTLIST  CQCGW:MacroScope\n"
        L"           CQCGW:Name CDATA #REQUIRED>\n\n"


        //
        //  These are all related to querying and updating the configuration of
        //  scheduled events.
        //

        //
        //  These define the gettable/settable aspects of the three kinds of
        //  scheduled events we support. They are used to query the list of
        //  events, and to get/set event information.
        //
        L"<!ELEMENT  CQCGW:PerEvInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:PerEvInfo\n"
        L"           CQCGW:Name CDATA #REQUIRED\n"
        L"           CQCGW:Id CDATA #REQUIRED\n"
        L"           CQCGW:State (%PauseState;) #REQUIRED\n"
        L"           CQCGW:Type (Min | Hour | Day) #REQUIRED\n"
        L"           CQCGW:Period CDATA #REQUIRED>\n"

        L"<!ELEMENT  CQCGW:SchEvInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:SchEvInfo\n"
        L"           CQCGW:Name CDATA #REQUIRED\n"
        L"           CQCGW:Id CDATA #REQUIRED\n"
        L"           CQCGW:Type (Daily | Weekly | Monthly) #REQUIRED\n"
        L"           CQCGW:State (%PauseState;) #REQUIRED\n"
        L"           CQCGW:DHM CDATA #REQUIRED\n"
        L"           CQCGW:Mask CDATA #REQUIRED>\n"

        L"<!ELEMENT  CQCGW:SunEvInfo EMPTY>\n"
        L"<!ATTLIST  CQCGW:SunEvInfo\n"
        L"           CQCGW:Name CDATA #REQUIRED\n"
        L"           CQCGW:Id CDATA #REQUIRED\n"
        L"           CQCGW:State (%PauseState;) #REQUIRED\n"
        L"           CQCGW:Type (Rise | Set) #REQUIRED\n"
        L"           CQCGW:Offset CDATA #REQUIRED>\n"

        // These are setters for each type, to update the configuration
        L"<!ELEMENT  CQCGW:SetPerEv EMPTY>\n"
        L"<!ATTLIST  CQCGW:SetPerEv \n"
        L"           CQCGW:EvId CDATA #REQUIRED\n"
        L"           CQCGW:FromTime CDATA #REQUIRED\n"
        L"           CQCGW:Period CDATA #REQUIRED>\n"


        L"<!ELEMENT  CQCGW:SetSchEv EMPTY>\n"
        L"<!ATTLIST  CQCGW:SetSchEv\n"
        L"           CQCGW:EvId CDATA #REQUIRED\n"
        L"           CQCGW:DHM CDATA #REQUIRED\n"
        L"           CQCGW:Mask CDATA #REQUIRED>\n"

        L"<!ELEMENT  CQCGW:SetSunEv EMPTY>\n"
        L"<!ATTLIST  CQCGW:SetSunEv\n"
        L"           CQCGW:EvId CDATA #REQUIRED\n"
        L"           CQCGW:Offset CDATA #REQUIRED>\n"

        //
        //  And a getter which just passes the id of the event to query. The response
        //  is the appropriate event info element above for the type of event
        //  queried. As of 5.0 the 'id' is the hierarchical path, relative to /User.
        //
        L"<!ELEMENT  CQCGW:QuerySchEv EMPTY>\n"
        L"<!ATTLIST  CQCGW:QuerySchEv\n"
        L"           CQCGW:EvId CDATA #REQUIRED>\n"

        // To pause/resume a scheduled event. They send the id and the new state
        L"<!ELEMENT  CQCGW:PauseSchEv EMPTY>\n"
        L"<!ATTLIST  CQCGW:PauseSchEv\n"
        L"           CQCGW:EvId CDATA #REQUIRED\n"
        L"           CQCGW:State (%PauseState;) #REQUIRED>\n"

        //
        //  These are media related operations. These days there's very little,
        //  since we just let them download a whole repo database. We mostly just
        //  need to provide them with a way to download art, and to download
        //  databases when changes haev occurred.
        //

        // Common elements
        L"<!ELEMENT CQCGW:Artist        (#PCDATA)>\n"
        L"<!ELEMENT CQCGW:Cookie        (#PCDATA)>\n"
        L"<!ELEMENT CQCGW:Duration      (#PCDATA)>\n"
        L"<!ELEMENT CQCGW:Id            (#PCDATA)>\n"
        L"<!ELEMENT CQCGW:Loc           (#PCDATA)>\n"
        L"<!ELEMENT CQCGW:Name          (#PCDATA)>\n"

        L"<!ELEMENT CQCGW:Artists       (CQCGW:Artist*)>\n"
        L"<!ELEMENT CQCGW:Cookies       (CQCGW:Cookie*)>\n"
        L"<!ELEMENT CQCGW:Durations     (CQCGW:Duration*)>\n"
        L"<!ELEMENT CQCGW:Ids           (CQCGW:Id*)>\n"
        L"<!ELEMENT CQCGW:Locs          (CQCGW:Loc*)>\n"
        L"<!ELEMENT CQCGW:Names         (CQCGW:Name*)>\n"


        //
        //  Query cover art from either from the client serivce cached data (though
        //  of course on the machine where the GW server is running) or the image
        //  currently showing in a renderer.
        //
        //  In the former case they must provide the unique id for the image, which
        //  is size independent, along with a size indicator and the size specific
        //  persistent id which is used to check for changes.
        //
        //  The SID attr of the reply is only meaningful for the QueryMediaArt call.
        //  For the renderer art call it's always empty.
        //
        //  The Result is used for both. For the former it indicates if art was not
        //  found or could not be loaded, was found but not changes, or was found and
        //  new art/SID is returned. For renderer art queries NoChanges means no
        //  currently playing media, and NewData means new image data is available.
        //
        L"<!ELEMENT CQCGW:QueryMediaArt EMPTY>\n"
        L"<!ATTLIST CQCGW:QueryMediaArt\n"
        L"          CQCGW:Moniker CDATA #REQUIRED\n"
        L"          CQCGW:MType (Music | Movie) #REQUIRED\n"
        L"          CQCGW:Size (Sml | Lrg) 'Sml'\n"
        L"          CQCGW:UID CDATA #REQUIRED\n"
        L"          CQCGW:SID CDATA #REQUIRED>\n\n"

        L"<!ELEMENT CQCGW:QueryMediaRendArt EMPTY>\n"
        L"<!ATTLIST CQCGW:QueryMediaRendArt\n"
        L"          CQCGW:Size (Sml | Lrg) 'Sml'\n"
        L"          CQCGW:Moniker CDATA #REQUIRED>\n\n"

        L"<!ELEMENT CQCGW:MediaImgData (#PCDATA)>\n"
        L"<!ATTLIST CQCGW:MediaImgData\n"
        L"          CQCGW:Bytes CDATA #REQUIRED\n"
        L"          CQCGW:Result (Failed | NoChanges | NewData) #REQUIRED\n"
        L"          CQCGW:SID CDATA ''\n"
        L"          CQCGW:Type (PNG | JPEG | BMP | Unknown) #REQUIRED>\n\n"

        //
        //  Query the media database for a specific repo. They pass in a preivous
        //  serial number, and if it's up to date, they get back an indication that
        //  they already have the most recent stuff. Else it's a new serial number
        //  and new data.
        //
        //  The reply, if changes have been made (serial number is different from
        //  that sent in the query), contains a Base64 encoded, ZLib compressed,
        //  XML formatted media database.
        //
        //  If the result value is Failed, then the body text is an error message
        //  instead of database info.
        //
        L"<!ELEMENT CQCGW:QueryMediaDB EMPTY>\n"
        L"<!ATTLIST CQCGW:QueryMediaDB\n"
        L"          CQCGW:DBSerialNum CDATA #REQUIRED\n"
        L"          CQCGW:Moniker CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCGW:MediaDBInfo (#PCDATA)>\n"
        L"<!ATTLIST CQCGW:MediaDBInfo\n"
        L"          CQCGW:Result (Failed | NoChanges | NewData) #REQUIRED\n"
        L"          CQCGW:DBSerialNum CDATA #REQUIRED>\n"


        // To get a renderer's current playlist
        L"<!ELEMENT CQCGW:QueryRendPL EMPTY>\n"
        L"<!ATTLIST CQCGW:QueryRendPL\n"
        L"          CQCGW:Moniker CDATA #REQUIRED>\n"
        L"<!ELEMENT CQCGW:RendPLInfo (CQCGW:Names, CQCGW:Cookies, CQCGW:Locs, CQCGW:Artists, CQCGW:Durations, CQCGW:Ids+)>\n"
        L"<!ATTLIST CQCGW:RendPLInfo\n"
        L"          CQCGW:Count CDATA #REQUIRED>\n"


        //
        //  For CIDLib exceptions, we return a specific reply that can return
        //  all of that info in a structured manner. The body of the element
        //  is the error text.
        //
        L"<!ELEMENT CQCGW:ExceptionReply (#PCDATA)>\n"
        L"<!ATTLIST CQCGW:ExceptionReply\n\n"
        L"          CQCGW:ErrClass CDATA #REQUIRED\n"
        L"          CQCGW:ErrSev CDATA #REQUIRED\n"
        L"          CQCGW:File CDATA #REQUIRED\n"
        L"          CQCGW:Line CDATA #REQUIRED\n"
        L"          CQCGW:Process CDATA #REQUIRED\n"
        L"          CQCGW:Thread CDATA #REQUIRED>\n\n"

        //
        //  If we get an unknown exception, we can't do much but indicate
        //  that that happened.
        //
        L"<!ELEMENT CQCGW:UnknownExceptionReply EMPTY>\n"

        //
        //  The generic query element. A number of queries from the client
        //  don't need any specific data other than to indicate what it is
        //  querying.
        //
        L"<!ELEMENT CQCGW:Query EMPTY>\n"
        L"<!ATTLIST CQCGW:Query\n"
        L"          CQCGW:QueryType (%QueryList;) #REQUIRED>\n\n"

        //
        //  The client can query XML formatted system configuration data.
        //  They ask for a specific room, or the list of room names. We
        //  return the formatted system config info embedded within the
        //  RoomCfgRep element, via an ANY content model.
        //
        //  The query for the list of room names is done via the generic
        //  query request message, and returned as a RoomList element. For
        //  now it's just a name attribute, there might be other info later
        //  if needed.
        //
        //  We also have to define the elements used by the system config
        //  stuff, which can be embedded in the RoomCfgRep element. If we
        //  want to be able to validate messages, we have to define those
        //  those elements.
        //
        L"<!ELEMENT CQCSysCfg:Music EMPTY>\n"
        L"<!ATTLIST CQCSysCfg:Music\n"
        L"          CQCSysCfg:Player CDATA #REQUIRED\n"
        L"          CQCSysCfg:Repo CDATA #REQUIRED\n"
        L"          CQCSysCfg:Audio CDATA #REQUIRED\n"
        L"          CQCSysCfg:AudioSub CDATA #REQUIRED\n>\n"

        L"<!ELEMENT CQCSysCfg:Movies EMPTY>\n"
        L"<!ATTLIST CQCSysCfg:Movies\n"
        L"          CQCSysCfg:Player CDATA #REQUIRED\n"
        L"          CQCSysCfg:Repo CDATA #REQUIRED\n"
        L"          CQCSysCfg:Audio CDATA #REQUIRED\n"
        L"          CQCSysCfg:AudioSub CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCSysCfg:Security (#PCDATA)>\n"
        L"<!ATTLIST CQCSysCfg:Security\n"
        L"          CQCSysCfg:Panel CDATA #REQUIRED\n"
        L"          CQCSysCfg:SecArea CDATA #REQUIRED\n"
        L"          CQCSysCfg:ZoneCnt CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCSysCfg:Weather EMPTY>\n"
        L"<!ATTLIST CQCSysCfg:Weather\n"
        L"          CQCSysCfg:Drv CDATA #REQUIRED\n"
        L"          CQCSysCfg:Scale (F|C) #REQUIRED>\n"

        L"<!ELEMENT CQCSysCfg:Load EMPTY>\n"
        L"<!ATTLIST CQCSysCfg:Load\n"
        L"          CQCSysCfg:Name CDATA #REQUIRED\n"
        L"          CQCSysCfg:Dimmer CDATA #IMPLIED\n"
        L"          CQCSysCfg:Switch CDATA #IMPLIED\n"
        L"          CQCSysCfg:Color CDATA #IMPLIED>\n"

        L"<!ELEMENT CQCSysCfg:Loads (CQCSysCfg:Load*)>\n"
        L"<!ATTLIST CQCSysCfg:Loads\n"
        L"          CQCSysCfg:DefLoad CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCSysCfg:GlobAct (CQCGW:ActParm*)>\n"
        L"<!ATTLIST CQCSysCfg:GlobAct\n"
        L"          CQCSysCfg:Name CDATA #REQUIRED\n"
        L"          CQCSysCfg:ActPath CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCSysCfg:GlobActs (CQCSysCfg:GlobAct*)>\n"

        L"<!ELEMENT CQCSysCfg:RoomVar (#PCDATA)>\n"
        L"<!ELEMENT CQCSysCfg:RoomVars (CQCSysCfg:RoomVar+)>\n"

        L"<!ELEMENT CQCSysCfg:Room (CQCSysCfg:RoomVars, CQCSysCfg:Music?, \n"
        L"                          CQCSysCfg:Movies?, \n"
        L"                          CQCSysCfg:Security?, CQCSysCfg:Weather?, \n"
        L"                          CQCSysCfg:Loads?, CQCSysCfg:GlobActs?)>\n"
        L"<!ATTLIST CQCSysCfg:Room\n"
        L"          CQCSysCfg:Name CDATA #REQUIRED\n"
        L"          CQCSysCfg:Music (%YesNoVal;) #REQUIRED\n"
        L"          CQCSysCfg:Movies (%YesNoVal;) #REQUIRED\n"
        L"          CQCSysCfg:Security (%YesNoVal;) #REQUIRED\n"
        L"          CQCSysCfg:Weather (%YesNoVal;) #REQUIRED>\n"

        L"<!ELEMENT CQCGW:RoomCfgReq EMPTY>\n"
        L"<!ATTLIST CQCGW:RoomCfgReq\n"
        L"          CQCGW:Name CDATA #REQUIRED>\n"

        L"<!ELEMENT CQCGW:RoomCfgRep (CQCSysCfg:Room+)>\n"

        L"<!ELEMENT CQCGW:RoomInfo EMPTY>\n"
        L"<!ATTLIST CQCGW:RoomInfo\n"
        L"          CQCGW:Name CDATA #REQUIRED>\n"

        L"<!ELEMENT  CQCGW:RoomList (CQCGW:RoomInfo*)>\n\n"


        //
        //  This is the overall element that is always sent. It's one child
        //  is always one of the above elements.
        //
        L"<!ELEMENT  CQCGW:Msg (%MsgList;)>"
    };
}


