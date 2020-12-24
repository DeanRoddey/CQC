//
// FILE NAME: HAIOmniTCP2S_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//   CLASS: THAIOmniTCPS
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class THAIOmniTCP2S : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        THAIOmniTCP2S
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~THAIOmniTCP2S();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TCard4 c4QueryCfg
        (
                    THeapBuf&               mbufToFill
        );

        tCIDLib::TVoid SetConfig
        (
            const   tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
        );


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
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        THAIOmniTCP2S();
        THAIOmniTCP2S(const THAIOmniTCP2S&);
        tCIDLib::TVoid operator=(const THAIOmniTCP2S&);


        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TFundArray<tCIDLib::TCard4> TFldIdList;


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
            , const tCIDLib::TBoolean       bOmniMsg = kCIDLib::True
        );

        tCIDLib::TBoolean bInitFields();

        tCIDLib::TBoolean bNormalizeName
        (
                    TString&                strName
        );

        tCIDLib::TBoolean bReadCfgFile();

        tCIDLib::TBoolean bValidateCode
        (
            const   TString&                strCode
            , const tCIDLib::TCard4         c4Area
            ,       tCIDLib::TCard1&        c1CodeNum
        );

        tCIDLib::TBoolean bWaitAck
        (
            const   tCIDLib::TBoolean       bThrowIfTimeout
        );

        tCIDLib::TBoolean bWaitEnd
        (
            const   tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1ExpMsg
            , const tCIDLib::TCard1         c1ExpDataType
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TCard2 c2CalcCRC
        (
            const   TMemBuf&                mbufSrc
        );

        tCIDLib::TCard2 c2CheckItemNum
        (
            const   tCIDLib::TCard2         c2Num
            , const tCIDLib::TCard4         c4MaxNum
            , const tCIDLib::TCh* const     pszTypeName
        );

        tCIDLib::TCard4 c4EncryptPayload
        (
            const   tCIDLib::TCard1* const  pc1Payload
            , const tCIDLib::TCard4         c4PayloadBytes
            ,       TMemBuf&                mbufMsg
        );

        tCIDLib::TCard4 c4EnumObjs
        (
            const   tCIDLib::TCard1         c1Type
            ,       tCIDLib::TStrList&      colNames
            ,       TFundVector<tCIDLib::TCard2>& fcolIds
            ,       TMemBuf&                mbufToUse
        );

        tCIDLib::TCard4 c4QueryObjTypeMax
        (
            const   tCIDLib::TCard1         c1ObjType
        );

        tCIDLib::TCard4 c4ToOmniTemp
        (
            const   tCIDLib::TInt4          i4OmniTemp
            , const tCIDLib::TBoolean       bCelsiusFmt
        );

        tCIDLib::TVoid CheckSysEvents();

        tCIDLib::TVoid DoCommonInit();

        tCQCKit::ECommResults eProcessUserCmd
        (
            const   TString&                strCmdStr
        );

        tCIDLib::TInt4 i4FromOmniTemp
        (
            const   tCIDLib::TCard4         c4OmniTemp
            , const tCIDLib::TBoolean       bCelsiusFmt
            , const tCIDLib::TCard4         c4MinVal
            , const tCIDLib::TCard4         c4MaxVal
        );

        tCIDLib::TVoid ImportOmniCfg
        (
                    TOmniTCPDrvCfg&         dcfgToFill
        );

        const tCIDLib::TCh* pszXlatZEvStatus
        (
            const   tCIDLib::TCard4         c4Status
        )   const;

        tCIDLib::TVoid PollAreas();

        tCIDLib::TVoid PollEnclosures();

        tCIDLib::TVoid PollThermos();

        tCIDLib::TVoid PollUnits();

        tCIDLib::TVoid PollZones();

        tCIDLib::TVoid ProcessAsync
        (
            const   tCIDLib::TCard1         c1Type
            , const TMemBuf&                mbufMsg
        );

        tCIDLib::TVoid ProcessAsyncs();

        tCIDLib::TVoid ProcessOtherNotification
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid ReleaseNamedItems();

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid SendObjStatusQuery
        (
            const   tCIDLib::TCard1         c1ObjType
            , const tCIDLib::TCard2         c2StartNum
            , const tCIDLib::TCard2         c2EndNum
        );

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

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
            , const tCIDLib::TCard1         c1Data3
        );

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TCard1         c1Data1
            , const tCIDLib::TCard1         c1Data2
            , const tCIDLib::TCard1         c1Data3
            , const tCIDLib::TCard1         c1Data4
        );

        tCIDLib::TVoid SendOmniMsg
        (
            const   tCIDLib::TCard1         c1MsgType
            , const tCIDLib::TCard1         c1ObjType
            , const tCIDLib::TCard2         c2ItemNum
            , const tCIDLib::TCard1         ac1Data[]
            , const tCIDLib::TCard4         c4DataCnt
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

        tCIDLib::TVoid SendZoneTrig
        (
            const   TString&                strFld
            , const tCIDLib::TCard1         c1Status
            , const tCIDLib::TCard2         c2Num
            , const THAIOmniZone&           itemZone
        );

        tCIDLib::TVoid StoreAlarmFields
        (
            const   THAIOmniArea&           itemSrc
            , const tCIDLib::TCard4         c4Bits
        );

        tCIDLib::TVoid StoreAreaStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreAuxStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreEnclosureStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreSysInfo
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreSysStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreThermoStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreUnitStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid StoreZoneStatus
        (
            const   TMemBuf&                mbufData
        );

        tCIDLib::TVoid UpdateAreaZoneArm
        (
            const   THAIOmniArea&           itemCur
            , const tCIDLib::TBoolean       bArmed
        );

        tCIDLib::TVoid WaitReply
        (
            const   tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard1         c1ExpMsg
            , const tCIDLib::TCard1         c1DataType
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid WriteCfgFile();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_ac1Plain
        //  m_ac1Cypher
        //      Some AES block size buffers we use when encrypting and
        //      decrypting messages. The source comes from the memory buffer,
        //      but we need to decrypt into something first, then copy that
        //      to the target buffer.
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
        //  m_c4MaxXXX
        //      The maximum number of each type of object that we were told
        //      by the device he has. This tells us how many to poll during
        //      init to get initial values of.
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
        //  m_conncfgSock
        //      We pull our comm configuration out of the incoming driver
        //      config during ctor, and just keep it around for easy
        //      access during connection.
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
        //      This also holds the item objects we use at runtime. Most of
        //      it isn't streamed, but is just for us at runtime.
        //
        //  m_enctLastPing
        //      The last time we actively pinged the device. We do this
        //      periodically to make sure it's still there and that it's
        //      not just sending asyncs but also seeing our traffic.
        //
        //  m_mhashKey
        //      The key we get from the user during driver install, which
        //      contains the 128 bit key we need to talk to the Omni. We store
        //      it as an MD5 hash since it has the same format.
        //
        //  m_sockOmni
        //      The TCP socket that we use to talk to the Omni.
        //
        //  m_splBuffers
        //      Because of the fact that we have to both deal with async
        //      messages and sometimes ask the device for info while we are
        //      dealing with other replies and so forth, we cannot just have
        //      a fixed set of buffers. So we create a pool and they are gotten
        //      out as required.
        //
        //  m_strCfgFile
        //      We pre-build the config file in the init, so we don't have
        //      to keep building it.
        //
        //  m_strEvButton
        //      Some strings we use, so we pre-set them up.
        //
        //  m_strTmp1
        //      Some strings for temporary use. Don't ever expect them to stay
        //      the same across a call. Just use them for local formatting.
        // -------------------------------------------------------------------
        tCIDLib::TCard1         m_ac1Cypher[16];
        tCIDLib::TCard1         m_ac1Plain[16];
        tCIDLib::TCard4         m_ac4FldId_EnclBattery[kHAIOmniTCP2Sh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclACOff[kHAIOmniTCP2Sh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclBattLow[kHAIOmniTCP2Sh::c4MaxEnclosures];
        tCIDLib::TCard4         m_ac4FldId_EnclCommFail[kHAIOmniTCP2Sh::c4MaxEnclosures];
        tCIDLib::TBoolean       m_bDoCelsius;
        tCIDLib::TCard2         m_c2LastSeq;
        tCIDLib::TCard4         m_c4CntEnclosures;
        tCIDLib::TCard4         m_c4FirstAreaFldId;
        tCIDLib::TCard4         m_c4FirstThermoFldId;
        tCIDLib::TCard4         m_c4FirstUnitFldId;
        tCIDLib::TCard4         m_c4FirstZoneFldId;
        tCIDLib::TCard4         m_c4FldId_Firmware;
        tCIDLib::TCard4         m_c4FldId_InvokeCmd;
        tCIDLib::TCard4         m_c4FldId_MainBattery;
        tCIDLib::TCard4         m_c4FldId_Model;
        tCIDLib::TCard4         m_c4FldId_TimeValid;
        tCIDLib::TCard4         m_c4LastAreaFldId;
        tCIDLib::TCard4         m_c4LastThermoFldId;
        tCIDLib::TCard4         m_c4LastUnitFldId;
        tCIDLib::TCard4         m_c4LastZoneFldId;
        tCIDLib::TCard4         m_c4MaxAreas;
        tCIDLib::TCard4         m_c4MaxEnclosures;
        tCIDLib::TCard4         m_c4MaxThermos;
        tCIDLib::TCard4         m_c4MaxUnits;
        tCIDLib::TCard4         m_c4MaxZones;
        tCIDLib::TCard4         m_c4ThermoHigh;
        tCIDLib::TCard4         m_c4ThermoLow;
        tCIDLib::TCard4         m_c4ZoneHigh;
        tCIDLib::TCard4         m_c4ZoneLow;
        tCIDLib::TStrList       m_colTmpList;
        TCQCIPConnCfg           m_conncfgSock;
        TAESEncrypter           m_crypComm;
        TOmniTCPDrvCfg          m_dcfgCurrent;
        tCIDLib::TEncodedTime   m_enctLastPing;
        TMD5Hash                m_mhashKey;
        TClientStreamSocket     m_sockOmni;
        THeapBufPool            m_splBufs;
        TString                 m_strCfgFile;
        TString                 m_strCfgPath;
        TString                 m_strEvButton;
        TString                 m_strEvUPBLink;
        TString                 m_strEvX10Code;
        TString                 m_strTmp1;
        TString                 m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(THAIOmniTCP2S,TCQCServerBase)
};


#pragma CIDLIB_POPPACK
