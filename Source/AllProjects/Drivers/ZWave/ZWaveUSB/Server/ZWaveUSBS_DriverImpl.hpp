//
// FILE NAME: ZWaveUSBS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2007
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
//  This is the header file for the actual driver class. We just do the usual
//  thing of deriving from the base server driver class and overriding the
//  virtuals.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSBSDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TZWaveUSBSDriver : public TCQCServerBase, public MZWaveCtrl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUSBSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TZWaveUSBSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        //
        //  Some of these are inherited from teh MZWaveCtrl mixin class. This
        //  allows us to be passed into the unit classes in the shared
        //  facility, so that they can interact with modules via us.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        );

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TCard1 c1NextCallbackId();

        tCIDLib::TCard4 c4QueryCfg
        (
                    THeapBuf&               mbufToFill
        );

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        );

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
        );

        THeapBufPool& splBufs();

        tCIDLib::TVoid SendAndWaitForAck
        (
            const   TMemBuf&                mbufToSend
        );

        tCIDLib::TCard4 c4SendAndWaitForReply
        (
            const   TMemBuf&                mbufOut
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            ,       TMemBuf&                mbufIn
        );

        tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            ,       TMemBuf&                mbufIn
        );

        tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TCard1         c1Parm
            ,       TMemBuf&                mbufIn
        );

        tCIDLib::TCard4 c4SendReqAndWaitForReply
        (
                    tCIDLib::TCard1         c1Req
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TCard1         c1Parm
            , const tCIDLib::TCard1         c1Parm2
            ,       TMemBuf&                mbufIn
        );

        tCIDLib::TCard4 c4WaitForReply
        (
            const   tCIDLib::TCard1         c1Expected
            , const tCIDLib::TCard4         c4WaitFor
            , const tCIDLib::TCard4         c4ExpRepLen
            , const tCIDLib::TBoolean       bCallback
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid SendData
        (
            const   TMemBuf&                mbufToSend
            , const tCIDLib::TBoolean       bGetAck = kCIDLib::True
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

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TZWaveUSBSDriver();
        TZWaveUSBSDriver(const TZWaveUSBSDriver&);
        tCIDLib::TVoid operator=(const TZWaveUSBSDriver&);


        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TRefQueue<TZWaveUnit>       TUnitPollQ;



        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateNodeCfg
        (
                    TZWaveDrvConfig&        dcfgToValidate
            , const tCIDLib::TBoolean       bUpdateMap
            , const tCIDLib::TBoolean       bCheckCurTypes
            , const tCIDLib::TBoolean       bDiscoverMode
        );

        tCIDLib::TCard1 c1CalcSum
        (
            const   TMemBuf&                mbufMsg
        )   const;

        tCIDLib::TVoid BuildUnitGetMsg
        (
                    TMemBuf&                mbufToFill
            , const TZWaveUnit&             unitTar
            ,       tCIDLib::TCard1&        c1RetClass
            ,       tCIDLib::TCard1&        c1RetCmd
            ,       tCIDLib::TCard1&        c1RetLen
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUGetCmds  eCmd
        )   const;

        tCIDLib::TVoid BuildUnitSetMsg
        (
                    TMemBuf&                mbufToFill
            , const TZWaveUnit&             unitTar
            , const tCIDLib::TCard1         c1Val1
            , const tCIDLib::TCard1         c1Val2
            , const tCIDLib::TCard1         c1CBId
            , const tZWaveUSBSh::EUSetCmds  eCmd
        )   const;

        tCIDLib::TVoid CheckParmCount
        (
            const   TString&                strCmd
            , const tCIDLib::TCard4         c4PCount
            , const tCIDLib::TCard4         c4Expected
        );

        tCIDLib::TVoid CheckReplicating();

        tCIDLib::TVoid DelayAndProcess
        (
            const   tCIDLib::TCard4         c4WaitFor
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid DeleteGroup
        (
            const   TString&                strGroupId
        );

        tCIDLib::TVoid DoGroupLevel
        (
            const   TZWaveGroupInfo&        grpiTar
            , const tCIDLib::TCard1         c1Lev
        );

        tCIDLib::TVoid DoGroupOnOff
        (
            const   TZWaveGroupInfo&        grpiTar
            , const tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid DoGroupRamp
        (
            const   TZWaveGroupInfo&        grpiTar
            , const TString&                strOp
        );

        tCIDLib::TVoid DoPollCycle
        (
                    TThread&                thrCaller
        );

        tCIDLib::TVoid DoUnitSet
        (
            const   TZWaveUnit&             unitTar
            , const tZWaveUSBSh::EUSetCmds  eCmd
            , const tCIDLib::TCard1         c1Value
        );

        tZWaveUSBS::EMsgTypes eGetReply
        (
                    tCIDLib::TCard4&        c4Read
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TEncodedTime   enctWait
            ,       tCIDLib::TCard1&        c1CmdId
            ,       tCIDLib::TCard1&        c1CBId
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::EExitCodes eReplicationThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid HandleCallBack
        (
            const   tCIDLib::TCard1         c1CmdId
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid HandleResponse
        (
            const   tCIDLib::TCard1         c1CmdId
            ,       TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid ZWLogDbg
        (
            const   tCIDLib::TCh* const     pszFile
            , const tCIDLib::TCard4         c4Line
            , const tCIDLib::TCh* const     pszToLog
            , const MFormattable&           fmtbl1
            , const MFormattable&           fmtbl2
            , const MFormattable&           fmtbl3
        );

        tCIDLib::TVoid ProcessAppCmd
        (

        );

        tCIDLib::TVoid QueryInitData();

        tCIDLib::TVoid QueryUnitAssoc
        (
            const   TString&                strUnitName
            ,       TString&                strParms
        );

        tCIDLib::TVoid RegisterFields
        (
                    TZWaveDrvConfig&        dcfgNew
        );

        tCIDLib::TVoid RequestReport
        (
                    TZWaveUnit&             unitSrc
        );

        tCIDLib::TVoid ResetCtrl();

        tCIDLib::TVoid SendAckNak
        (
            const   tCIDLib::TBoolean       bAck
        );

        tCIDLib::TVoid SendMsg
        (
            const   TMemBuf&                mbufToSend
        );

        tCIDLib::TVoid SetAssociation
        (
            const   tCIDLib::TBoolean       bAdd
            , const TString&                strParams
        );

        tCIDLib::TVoid SetCfgParam
        (
            const   TString&                strParams
        );

        tCIDLib::TVoid UpdateConfig
        (
            const   TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid WaitForNextInterval
        (
                    TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid WaitForCmdCB
        (
            const   tCIDLib::TCard1         c1UnitId
            , const tCIDLib::TCard1         c1Class
            , const tCIDLib::TCard1         c1Cmd
            , const tCIDLib::TCard1         c1Len
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TMemBuf&                mbufToUse
        );

        tCIDLib::TVoid WaitForCmdCB
        (
            const   tCIDLib::TCard1         c1Cmd
            , const tCIDLib::TCard1         c1ExpectedCB
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TMemBuf&                mbufToUse
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bInNetwork
        //      This is set if we are set up as a secondary controller, which
        //      means that we have been added to some network. We get this
        //      during connection.
        //
        //  m_bIsSUC
        //      Gotten during connection. Indicates if the controller we are
        //      connected to is a SUC.
        //
        //  m_bRepMode
        //      This is set when we are doing a replication. During this,
        //      the poll callback just watches for replication to end, and
        //      incoming commands are rejected because the replication thread
        //      owns the controller. The thread will set m_eRepRes to indicate
        //      success or failure, and will update the m_strRepStatus string
        //      with some descriptive text.
        //
        //  m_bsNodeMap
        //      This is the actual node map that we got during the last query
        //      of initial data (which happens on connect and as part of each
        //      replication.) For unit id X, the bit at index X-1 indicates
        //      whether that unit is known to our controller.
        //
        //      We keep this because our actual configuration data doesn't
        //      necessarily match what's out there all the time. This lets us know
        //      what has shown up or gone away since the last replication.
        //
        //  m_c1CtrlId
        //      Gotten during connection. Indicates our connected controller's
        //      id on the network.
        //
        //  m_c1NextCBId
        //      We assign callback ids in a round robin, just allowing this
        //      value to wrap around (but skipping zero when we wrap around.)
        //      This prevents us from accidentally seeing a delayed response
        //      and thinking its for the current one.
        //
        //  m_c1SerAPIVersion
        //      The serial API version of the controller. This is gotten
        //      during connection.
        //
        //  m_c4FirstGrpId
        //  m_c4LastGrpId
        //      We store the first and last field id for groups. Anything
        //      not in this range must be one owned by a unit.
        //
        //  m_c4FldId_XXX
        //      The field ids of any non-group/unit oriented fields that we/
        //      access enough to want to pre-look up their ids.
        //
        //  m_c4HomeId
        //      Gotten during connection. Indicates our home id on the network.
        //
        //  m_c4PollableCnt
        //      When we get new config and have to re-register our fields, we
        //      flush the poll queue and as we go through the units, we count
        //      the pollable ones. We then use this value and the size of the
        //      poll queue to know how much backlog we have on polling.
        //
        //  m_colPollQ
        //      When a unit's poll time comes up, if it is not already on this
        //      list, it is placed at the end of the list. Units are pulled
        //      off the head of the queue and processed, one per poll callback.
        //      We can tell how behind we are in polling by comparing the
        //      number of items in this queue to the number of pollable units
        //      configured, which is stored in m_c4PollableCnt.
        //
        //      This guy is non-adopting, since the units belong to the
        //      configuration object.
        //
        //  m_cpcfgSerial
        //      We store away the serial port configuration data passed to us
        //      in the ctor, to avoid having to get it back out of the driver
        //      config ever time we might need it.
        //
        //  m_dcfgCurrent
        //      This is our current module and group configuration info.
        //
        //  m_eDrvState
        //      This is the overall driver state indicator. It indicates whether
        //      we are connected, polling, replicating, waiting to be added
        //      to the network and so forth.
        //
        //  m_enctLastMsg
        //      If we don't have any interaction with the controller for a
        //      while, the poll loop will actively ping it. So we set this
        //      each time we receive a legal message.
        //
        //  m_enctNextData
        //      We will wait a certain amount of time between any two SendData
        //      operations (which sends data to a specific module.) We set it,
        //      after the last callback has been seen, to the minimum next
        //      time that we can send another. So basically this imposed a
        //      minimum time between things like asking for reports or sending
        //      on/off, ramp, etc... commands.
        //
        //  m_enctNextMsg
        //      We enforce a minimum inter-message interval, so that we will
        //      not send two messages to the controller within a minimum time.
        //      This is for any types of messages, but m_enctNextData above
        //      also is involved if the message is a SendData operation.
        //
        //  m_pcommZWave
        //      Our comm port that we use to talk to the Z-Wave controller.
        //
        //  m_splBufs
        //      We need a variable number of I/O buffers, so we use a pre-fab
        //      variation of the TSimplePool class to keep a set of them around.
        ///     We can't use a fixed set of buffers, because we have to deal with
        //      async messages coming in that can require sending back data and
        //      so forth.
        //
        //  m_strCfgScope
        //      The path to our configuration in the config server. We set it
        //      up and keep it around for use.
        //
        //  m_strCtrlVersion
        //      We get a controller version string during connect, which we
        //      store here for reporting purposes. And we also put it into a
        //      field once we get connected.
        //
        //  m_strPort
        //      The port we were told to use, provided to us in the ctor.
        //
        //  m_strRepStatus
        //      The replication thread will leave some descriptive text about
        //      any error that occurs, so that it can be passed back to the
        //      client driver.
        //
        //  m_thrReplicate
        //      When we are added to the network, we spin up a thread to handle
        //      the replication from the remote and the discovery of all the
        //      available nodes. We set the m_bReplicating member while the
        ///     thread is running.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bInNetwork;
        tCIDLib::TBoolean       m_bIsSUC;
        tCIDLib::TBoolean       m_bRepMode;
        TBitset                 m_bsNodeMap;
        tCIDLib::TCard1         m_c1CtrlId;
        tCIDLib::TCard1         m_c1NextCBId;
        tCIDLib::TCard1         m_c1SerAPIVersion;
        tCIDLib::TCard4         m_c4FirstGrpId;
        tCIDLib::TCard4         m_c4FldId_Command;
        tCIDLib::TCard4         m_c4FldId_PollingLoad;
        tCIDLib::TCard4         m_c4LastGrpId;
        tCIDLib::TCard4         m_c4FWVersion;
        tCIDLib::TCard4         m_c4HomeId;
        tCIDLib::TCard4         m_c4PollableCnt;
        TUnitPollQ              m_colPollQ;
        TCommPortCfg            m_cpcfgSerial;
        TZWaveDrvConfig         m_dcfgCurrent;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        tCIDLib::TEncodedTime   m_enctNextData;
        tCIDLib::TEncodedTime   m_enctNextMsg;
        tZWaveUSBSh::ERepRes    m_eRepRes;
        TCommPortBase*          m_pcommZWave;
        THeapBufPool            m_splBufs;
        TString                 m_strCfgScope;
        TString                 m_strCtrlVersion;
        TString                 m_strPort;
        TString                 m_strRepStatus;
        TThread                 m_thrReplicate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUSBSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

