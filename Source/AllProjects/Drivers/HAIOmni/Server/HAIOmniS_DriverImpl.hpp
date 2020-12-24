//
// FILE NAME: HAIOmniS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2005
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
//  This is the header file for the driver implementation class. We derive
//  from the base server side class and override callbacks.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: THAIOmniSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class THAIOmniSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniSDriver() = delete;

        THAIOmniSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        THAIOmniSDriver(const THAIOmniSDriver&) = delete;

        ~THAIOmniSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        );

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TFundArray<THAIOmniItem*>       TItemList;
        typedef TFundArray<tCIDLib::TCard4>     TFldIdList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreateSession();

        tCIDLib::TBoolean bDoLogin();

        tCIDLib::TBoolean bGetPacket
        (
                    TMemBuf&                mbufToFill
            ,       tCIDLib::TCard1&        c1PacketType
            ,       tCIDLib::TCard4&        c4MsgBytes
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard2         c2ExpSeq
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TBoolean bInitFields();

        tCIDLib::TBoolean bReadCfgFile();

        tCIDLib::TBoolean bValidateCode
        (
            const   TString&                strCode
            , const tCIDLib::TCard4         c4Area
            ,       tCIDLib::TCard1&        c1CodeNum
        );

        tCIDLib::TBoolean bWaitAck
        (
            const   tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TBoolean       bThrowIfTimeout
        );

        tCIDLib::TCard1 c1GetOmniMsg
        (
                    TMemBuf&                mbufRead
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1ExpRep
        );

        tCIDLib::TCard2 c2CalcCRC
        (
            const   TMemBuf&                mbufSrc
        );

        tCIDLib::TCard4 c4EncryptPayload
        (
            const   tCIDLib::TCard1* const  pc1Payload
            , const tCIDLib::TCard4         c4PayloadBytes
            ,       TMemBuf&                mbufPacket
        );

        tCIDLib::TCard4 c4ToOmniTemp
        (
            const   tCIDLib::TInt4          i4OmniTemp
            , const tCIDLib::TBoolean       bCelsiusFmt
        );

        tCIDLib::TVoid CheckItemNum
        (
            const   tCIDLib::TCard2         c2Num
            , const tCIDLib::TCard4         c4MaxNum
            , const tCIDLib::TCh* const     pszTypeName
        );

        tCIDLib::TVoid CheckSysEvents();

        tCIDLib::TInt4 i4FromOmniTemp
        (
            const   tCIDLib::TCard4         c4OmniTemp
            , const tCIDLib::TBoolean       bCelsiusFmt
            , const tCIDLib::TCard4         c4MinVal
            , const tCIDLib::TCard4         c4MaxVal
        );

        THAIOmniArea* pitemFindArea
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        THAIOmniThermo* pitemFindOwningThermo
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniUnit* pitemFindOwningUnit
        (
            const   tCIDLib::TCard4         c4FldId
        )   const;

        THAIOmniUnit* pitemFindUnit
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        THAIOmniZone* pitemFindZone
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TVoid PollThermos
        (
            const   tCIDLib::TBoolean       bDoAll
        );

        tCIDLib::TVoid PollUnits
        (
            const   tCIDLib::TBoolean       bDoAll
        );

        tCIDLib::TVoid PollZones
        (
            const   tCIDLib::TBoolean       bDoAll
        );

        tCIDLib::TVoid ProcessUserCmd
        (
            const   TString&                strCmdStr
        );

        tCIDLib::TVoid ProcessMsg
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid ReleaseNamedItems();

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid SendOmniCmd
        (
            const   tCIDLib::TCard1         c1Cmd
            , const tCIDLib::TCard1         c1Parm1
            , const tCIDLib::TCard2         c2Parm2
        );

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1Type
        );

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TCard1         c1Data
        );

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
        );

        tCIDLib::TVoid SendUnitReq
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4End
        );

        tCIDLib::TVoid SendPacket
        (
            const   TMemBuf&                mbufToSend
            , const tCIDLib::TCard1         c1PacketType
            , const tCIDLib::TCard4         c4SendBytes
        );

        tCIDLib::TVoid SendPacket
        (
            const   tCIDLib::TCard1* const  pc1ToSend
            , const tCIDLib::TCard1         c1PacketType
            , const tCIDLib::TCard4         c4SendBytes
        );

        tCIDLib::TVoid SendPacket
        (
            const   tCIDLib::TCard1         c1PacketType
        );

        tCIDLib::TVoid StoreAlarmFields
        (
            const   THAIOmniArea&           itemSrc
            , const tCIDLib::TCard4         c4Bits
        );

        tCIDLib::TVoid StoreAuxStatus
        (
            const   tCIDLib::TCard4         c4Low
            , const tCIDLib::TCard4         c4High
        );

        tCIDLib::TVoid StoreSysInfo();

        tCIDLib::TVoid StoreSysStatus
        (
            const   tCIDLib::TBoolean       bInitPoll
        );

        tCIDLib::TVoid StoreThermoStatus
        (
            const   tCIDLib::TCard4         c4Low
            , const tCIDLib::TCard4         c4High
        );

        tCIDLib::TVoid StoreUnitStatus
        (
            const   tCIDLib::TCard4         c4Low
            , const tCIDLib::TCard4         c4High
        );

        tCIDLib::TVoid StoreZoneStatus
        (
            const   tCIDLib::TCard4         c4Low
            , const tCIDLib::TCard4         c4High
        );

        tCIDLib::TVoid WaitReply
        (
            const   tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1Expected
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ac4FldId_XXX
        //      The field ids for the fields that are always created, but
        //      which come in sets.
        //
        //  m_bDoCelsius
        //      We get a command line prompt that indicates whether the user
        //      wants to deal with degress in terms of celsius or fahrenheit.
        //
        //  m_c2LastSeq
        //      The last packet sequence number we sent out. This is used to
        //      assign sequential packet numbers, which the Omni echos back
        //      in replies. This way we can reject dup packets and know if
        //      we don't get the reply packet to our sent packet.
        //
        //  m_c4ArmCountdown
        //      When an arming event occurs, we back off on polling for a
        //      couple seconds, because of the amount of activity that can
        //      occur when that happens. So this will be set to a value that
        //      will be counted down in the poll callback. Only event polling
        //      will be done until it hits zero.
        //
        //  m_c4CntXXX
        //      The counts of named items that we actually found. We have
        //      maximums for the named item types, but this is the number of
        //      them we found that were actually named.
        //
        //  m_c4FirstThermoFldId
        //  m_c4FirstZoneFldId
        //      We store the first fields ids of various things that come
        //      in groups, to help us map from incoming field writes to the
        //      thing being written to.
        //
        //  m_c4FldXXX
        //      The looked up field ids for the fields that are singles and
        //      always present.
        //
        //  m_c4LastThermoFldId
        //  m_c4LasttZoneFldId
        //      We store the last fields ids of various things that come
        //      in groups, to help us map from incoming field writes to the
        //      thing being written to.
        //
        //  m_c4LastXXXPoll
        //      We round robin through the various sets of items that we
        //      poll, and these are used to keep up with the last block
        //      that we polled of each time. The poll callback calls each
        //      of the polling method twice a second and passes a poll
        //      index value that they can use to decide to move forward to
        //      a new block of items to poll at a particular rate.
        //
        //  m_c4ReadBytes
        //      When an Omni message is read, teh number of bytes stored in
        //      m_mbufRead is put.
        //
        //  m_c4TimeoutCnt
        //      We are a little lenient on timeouts during polling, since the
        //      protocol is UDP based and it's not guaranteed delivery. So we
        //      let up to 4 timeouts occur before we decide we've lost him.
        //
        //  m_c4ZoneHigh
        //  m_c4ZoneLow
        //      The low and high zone numbers that we found. We use this to
        //      minimize the amount of data we ask for. If the user groups
        //      zones together, we can be as efficient as possible.
        //
        //  m_colTmpList
        //      This is a temp vector of strings used for writing out to any
        //      string list fields where we need to build up a temporary list
        //      of strings in order to store them to the list field.
        //
        //  m_crypComm
        //      The IP-based Omnis use AES to encrypt the actual message
        //      data, so we have to keep an encrypter around.
        //
        //  m_dcfgCurrent
        //      The current driver configuration data. We load it from the
        //      config server initially, but the client can send us new data
        //      which we write to the config server and then store here.
        //
        //  m_enctNextXXX
        //      These are timers to track the next time to poll another
        //      block of each type of pollable entity. We set them ahead
        //      to the next time. So when the current time is at or beyond
        //      these times, it's time to poll them.
        //
        //  m_fcolXXX
        //      These are arrays that hold pointers to each of the types
        //      of named items we deal wtih. If an entry is zero, then that
        //      item type with that id was not defined.
        //
        //  m_ipepFrom
        //      Used when reading packets, to recieve the source addr.
        //
        //  m_ipepOmni
        //      We get out the IP end point from the config. It's all we need
        //      since we know the prtocol. We pass this each time we send a
        //      packet.
        //
        //  m_mbufAck
        //  m_mbufRead
        //  m_mbufPacket
        //  m_mbufWrite
        //      Used to for reading and writing packets. The Packet buffer is
        //      used to read/write the actual UDP packets, which have some
        //      wrapper around the actual Omni message that was sent or
        //      received.
        //
        //  m_mhashKey
        //      The key we get from the user during driver install, which
        //      contains the 128 bit key we need to talk to the Omni. We store
        //      it as an MD5 hash since it has the same format.
        //
        //  m_sockOmni
        //      The UDP port that we use to talk to the Omni.
        //
        //  m_strCfgPath
        //  m_strCfgFile
        //      The path to our configuration file, which we build up during init
        //      and keep around. We also separately keep the path it is in.
        //
        //  m_strEvXXX
        //      Some event strings we use, so we pre-set them up.
        //
        //  m_strLogin
        //      The four character login id we get from the user during driver
        //      install. This is used to log into the Omni.
        //
        //  m_strTmpX+
        //      A couple of temporary strings to use for formatting stuff.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_ac4FldId_EnclBattery[kHAIOmniSh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclACOff[kHAIOmniSh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclBattLow[kHAIOmniSh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclCommFail[kHAIOmniSh::c4MaxEnclosures];
        tCIDLib::TBoolean       m_bDoCelsius;
        tCIDLib::TCard2         m_c2LastSeq;
        tCIDLib::TCard4         m_c4CntAreas;
        tCIDLib::TCard4         m_c4CntThermos;
        tCIDLib::TCard4         m_c4CntUnits;
        tCIDLib::TCard4         m_c4CntZones;
        tCIDLib::TCard4         m_c4FirstAreaFldId;
        tCIDLib::TCard4         m_c4FirstThermoFldId;
        tCIDLib::TCard4         m_c4FirstUnitFldId;
        tCIDLib::TCard4         m_c4FirstZoneFldId;
        tCIDLib::TCard4         m_c4FldId_InvokeCmd;
        tCIDLib::TCard4         m_c4FldId_MainBattery;
        tCIDLib::TCard4         m_c4LastAreaFldId;
        tCIDLib::TCard4         m_c4LastThermoFldId;
        tCIDLib::TCard4         m_c4LastUnitFldId;
        tCIDLib::TCard4         m_c4LastZoneFldId;
        tCIDLib::TCard4         m_c4NextThermoBlk;
        tCIDLib::TCard4         m_c4NextUnitBlk;
        tCIDLib::TCard4         m_c4NextZoneBlk;
        tCIDLib::TCard4         m_c4ReadBytes;
        tCIDLib::TCard4         m_c4TimeoutCnt;
        tCIDLib::TCard4         m_c4ThermoHigh;
        tCIDLib::TCard4         m_c4ThermoLow;
        tCIDLib::TCard4         m_c4ZoneHigh;
        tCIDLib::TCard4         m_c4ZoneLow;
        tCIDLib::TStrList       m_colTmpList;
        TAESEncrypter           m_crypComm;
        TOmniDrvCfg             m_dcfgCurrent;
        tCIDLib::TEncodedTime   m_enctNextEvent;
        tCIDLib::TEncodedTime   m_enctNextSys;
        tCIDLib::TEncodedTime   m_enctNextThermo;
        tCIDLib::TEncodedTime   m_enctNextUnit;
        tCIDLib::TEncodedTime   m_enctNextZone;
        TItemList               m_fcolAreas;
        TItemList               m_fcolThermos;
        TItemList               m_fcolUnits;
        TItemList               m_fcolZones;
        TIPEndPoint             m_ipepFrom;
        TIPEndPoint             m_ipepOmni;
        THeapBuf                m_mbufAck;
        THeapBuf                m_mbufPacket;
        THeapBuf                m_mbufRead;
        THeapBuf                m_mbufWrite;
        TMD5Hash                m_mhashKey;
        TClientDatagramSocket   m_sockOmni;
        TString                 m_strEvButton;
        TString                 m_strEvUPBLink;
        TString                 m_strEvX10Code;
        TString                 m_strCfgPath;
        TString                 m_strCfgFile;
        TString                 m_strLogin;
        TString                 m_strTmp1;
        TString                 m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniSDriver,TCQCServerBase)
};


#pragma CIDLIB_POPPACK
