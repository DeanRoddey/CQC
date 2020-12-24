//
// FILE NAME: ZWaveUSB3S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  Device Information Files
//
//  If we can get manufacturer ids, we can look those up in our available list of device
//  info files. If we have that, then we have all of the information, including unit
//  options with default values. So we can set up the unit. If we can figure out that
//  out on our own then we can usually set it up find by ourself. Else the user has to
//  intervene to tell us the unit type and/or select non-default option values.
//
//  It also includes setup information, such as groups to add our controller to so that we
//  get appropriate notifications, and configuration options to make it work like we (and
//  the user's selections in the options mentioned above) require.
//
//  Generic Handlers
//
//  We also have some generic handlers. We cannot auto-assign them to a unit. But the
//  user can select one for us. When they do that, then we will have make/model info.
//  If they chose something wrong, that's their problem.
//
// CAVEATS/GOTCHAS:
//
//  1)  When we have a network scan or replication going on, then we reject all commands
//      from the outside, because the replication thread is using the Z-Stick object. We
//      also ignore any app/cmd messages targeting the units, because we don't want the
//      background thread accessing the driver configuration which is our current stuff,
//      not the new stuff that the new replication pass is creating. Some msgs related
//      to the replication process (and therefore targeting us) are dealt with during
//      this time.
//
//      After such a scan we are going to re-register fields and get initial values
//      anyway, so it doesn't matter if we miss some notifications to units.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSB3SDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TZWaveUSB3SDriver : public TCQCServerBase, public MZWaveCtrlIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveUSB3SDriver() = delete;

        TZWaveUSB3SDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TZWaveUSB3SDriver(const TZWaveUSB3SDriver&) = delete;

        ~TZWaveUSB3SDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveUSB3SDriver& operator=(const TZWaveUSB3SDriver&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUSB3IsUniqueUnitName
        (
            const   TString&                strToCheck
        )   const override;

        tCIDLib::TBoolean bUSB3QueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   override;

        tCIDLib::TBoolean bSendData
        (
            const   TString&                strSendType
            ,       TString&                strDataName
            ,       tCIDLib::TCard4&        c4Bytes
            ,       THeapBuf&               mbufData
        )   override;

        tCIDLib::TBoolean bUSB3WaitForMsg
        (
            const   tZWaveUSB3Sh::EMsgTypes eType
            , const tCIDLib::TCard1         c1UnitId
            , const tCIDLib::TCard1         c1MsgId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        tCIDLib::TBoolean bUSB3WaitForMsg
        (
            const   TZWaveUnit&             unitTar
            , const tZWaveUSB3Sh::EMsgTypes eType
            , const tCIDLib::TCard1         c1MsgId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1CmdId
            , const tCIDLib::TEncodedTime   enctWaitFor
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        )   override;

        tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        )   override;

        tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        )   override;

        tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        )   override;

        tCIDLib::TBoolean bUSB3WriteFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        )   override;

        tCIDLib::TCard4 c4USB3LookupFldId
        (
            const   TString&                strName
        )   const   override;

        const TString& strUSB3Moniker() const;

        tCIDLib::TVoid USB3QEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        )   override;

        tCIDLib::TVoid USB3Send
        (
            const   TZWOutMsg&              zwomToSend
        )   override;

        tCIDLib::TVoid USB3SendCCMsg
        (
            const   TZWOutMsg&              zwomToSend
        )   override;

        tCIDLib::TVoid USB3SendCCMsg
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TCard4         c4WaitFor = 3500
        )   override;

        tCIDLib::TVoid USB3SendCtrlRes
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TCard4         c4WaitFor = 3000
        )   override;

        tCIDLib::TVoid USB3SendUnitRes
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TCard4         c4WaitFor = 3500
        )   override;

        tCIDLib::TVoid USB3SendReq
        (
            const   TZWOutMsg&              zwomToSend
            ,       TZWInMsg&               zwimReply
            , const tCIDLib::TCard4         c4WaitFor = 3500
        )   override;

        tCIDLib::TVoid USB3SetFieldErr
        (
            const   tCIDLib::TCard4         c4FldId
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-methods
        // -------------------------------------------------------------------
        const TString& strCfgPath() const
        {
            return m_strCfgPath;
        }


    protected :
        // -------------------------------------------------------------------
        //  The Z-stick class is our friend so that he can do some stuff that he
        //  needs to do.
        // -------------------------------------------------------------------
        friend class TZStick;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   override;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   override;

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        )   override;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::EDrvInitRes eInitializeImpl() override;

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        )   override;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   override;

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        )   override;

        tCIDLib::TVoid ReleaseCommResource() override;

        tCIDLib::TVoid TerminateImpl() override;

        tCIDLib::TVoid VerboseModeChanged
        (
            const   tCQCKit::EVerboseLvls   eNewState
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-methods
        // -------------------------------------------------------------------
        tZWaveUSB3Sh::EMsgTypes eGetReply
        (
                    TZWInMsg&               zwimToFill
            , const tCIDLib::TEncodedTime   enctWait
            , const tCIDLib::TBoolean       bThrowIfNot
        );

    private :
        // -------------------------------------------------------------------
        //  Private, non-methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppCmdHandler
        (
            const   TZWInMsg&               zwimReceived
        );

        tCIDLib::TVoid AppUpdateHandler
        (
            const   TZWInMsg&               zwimReceived
        );

        tCIDLib::TBoolean bDoReplication();

        tCIDLib::TBoolean bQueryUnitInfo
        (
            const   TZWUnitInfo&            unitiSrc
            ,       TString&                strErr
            ,       tCIDLib::TCard8&        c8ManIds
            ,       tZWaveUSB3Sh::TCInfoList& colCCs
            ,       tZWaveUSB3Sh::TEPInfoList& colEPs
            ,       tCIDLib::TCard4&        c4FanModes
            ,       tCIDLib::TCard4&        c4ThermoModes
            ,       tCIDLib::TCard4&        c4SetPoints
        );

        tCIDLib::TBoolean bSaveConfig
        (
            const   TZWaveDrvCfg&           dcfgNew
        );

        tCIDLib::TCard4 c4QueryUnitReport
        (
            const   TZWUnitInfo&            unitiSrc
            ,       TMemBuf&                mbufToFill
        );

        tCIDLib::TVoid CheckReplicating() const;

        tCQCKit::ECommResults eInvokeCmd
        (
            const   TString&                strCmdText
        );

        tCIDLib::EExitCodes eRepScanThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid GetNonce
        (
            const   tCIDLib::TCard1         c4FromId
            ,       THeapBuf&               mbufToFill
        );

        TZWaveUnit* punitFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        );

        TZWaveUnit* punitFindFldOwner
        (
            const   tCIDLib::TCard4         c4FldId
            ,       tCIDLib::TCard4&        c4Index
        );

        TZWaveUnit* punitFindByName
        (
            const   TString&                strToFind
        );

        TZWUnitInfo* punitiFindById
        (
            const   tCIDLib::TCard1         c1ToFind
        );

        TZWUnitInfo* punitiFindByName
        (
            const   TString&                strToFind
        );

        tCIDLib::TVoid ProcessNextUnit();

        tCIDLib::TVoid RegisterFields();

        tCIDLib::TVoid ResetCtrl();

        tCIDLib::TVoid SetDrvState
        (
            const   tZWaveUSB3Sh::EDrvStates eToSet
        );

        tCIDLib::TVoid SetNodeInfo
        (
            const   tCIDLib::TBoolean       bSecureMode
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c1GotSecSchemeGet
        //      This is set when we see a security scheme get msg from the master. We
        //      cannot respond immediately, because we have to get some other things
        //      done first, post replication. So the replication thread will check this
        //      after it gets those housekeeping bits done. If not seen yet, it will
        //      wait for it. The value is the unit id of the master controller, since
        //      this is the first msg we can be sure came from him. There's no other
        //      way to know who is that I'm aware of. It will be set to zero before the
        //      replication loop starts. If non-zer, then we got the msg.
        //
        //  m_c1NextSeq
        //      The network management command classes require a monotonic byte sized
        //      sequence number that is upped for each transaction.
        //
        //  m_c4DrvVerMaj
        //  m_c4DrvVerMin
        //      We get these out during init, from the driver config, for use when
        //      we start a new trace file. This way we don't have to worry about
        //      locking to get the data out later when the trace file is created.
        //
        //  m_c4FldId_XXX
        //      Storage fof the ids of fields that we both don't just update by name
        //      and that are not owned by units.
        //
        //  m_c4NextProcUnit
        //      We round robin through the units, for the ones that haven't failed in
        //      some way, giving them a chance to move their state forward or do any
        //      periodic processing they need.
        //
        //  m_colUnits
        //      This is our actual list of unit objects. For each fully configured
        //      unit info object in the driver config we create a unit object to
        //      handle it.
        //
        //      We want this sorted by id. That happens naturally initially when we
        //      add any fully configured ones from the config object since it has them
        //      sorted by id already. But, if we add any others on the fly, we have to
        //      make sure they get added in id sorted order!
        //
        //  m_dcfgCurrent
        //      The current active configuration.
        //
        //  m_dcfgReplicate
        //      A copy of the configuration that the background scan/replication thread
        //      fills in. It starts with a copy of the current config and modifies it.
        //      If it all goes well, then it will be copied back to the current config
        //      above and the whole process of getting the units up and going starts
        //      again.
        //
        //  m_eRepState
        //      For the client driver's benefit, we maintain a separate state during
        //      the replication process. The reason being that the main m_eZWDrvState
        //      doesn't stay at the final success/failure status, the main driver
        //      picks up and moves forward after the bgn thread completes the
        //      replication.
        //
        //  m_eZWDrvState
        //      This is an overall driver state enum that we keep. This is separate from
        //      the very widely used driver state that is kept by the base server side
        //      driver class, so we name this to make it clear this is not that. We also
        //      echo the text of the set state to a field for the outside world to see.
        //
        //  m_strCfgFile
        //  m_strCfgPath
        //      The path to our configuration file. We pre-build it and keep it around
        //      since it can't change while we are running. WE keep the path separately
        //      for convenience.
        //
        //  m_thrRepScan
        //      This thread is started up on the eRepScan method and does background
        //      replication and scanning.
        //
        //  m_zwdxdIn
        //  m_zwdxdReply
        //      A couple data exchange objects that we use to read in the incoming cmd
        //      or query and one we fill in to send back.
        //
        //  m_zstCtrl
        //      This object wraps the z-stick. It is effectively our comm resource and
        //      it manages the serial port. It provides the threads that read/write
        //      messages and the queues that hold ingoing and outgoing messages. So it
        //      does all of the I/O async from us.
        //
        //  m_zwimPoll
        //      This is used by the poll callback to read in asyncs.
        // -------------------------------------------------------------------
        tCIDLib::TCard1             m_c1GotSecSchemeGet;
        tCIDLib::TCard1             m_c1NextSeq;
        tCIDLib::TCard4             m_c4DrvVerMaj;
        tCIDLib::TCard4             m_c4DrvVerMin;
        tCIDLib::TCard4             m_c4FldId_InvokeCmd;
        tCIDLib::TCard4             m_c4FldId_NewConfig;
        tCIDLib::TCard4             m_c4FldId_OutQPercent;
        tCIDLib::TCard4             m_c4FldId_PendingQPercent;
        tCIDLib::TCard4             m_c4FldId_State;
        tCIDLib::TCard4             m_c4NextProcUnit;
        TZWaveDrvCfg                m_dcfgCurrent;
        TZWaveDrvCfg                m_dcfgReplicate;
        tZWaveUSB3Sh::EDrvStates    m_eRepState;
        tZWaveUSB3Sh::EDrvStates    m_eZWDrvState;
        TString                     m_strCfgFile;
        TString                     m_strCfgPath;
        TString                     m_strNodeName;
        TThread                     m_thrRepScan;
        TZWDriverXData              m_zwdxdIn;
        TZWDriverXData              m_zwdxdReply;
        TZStick                     m_zstCtrl;
        TZWInMsg                    m_zwimPoll;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveUSB3SDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

