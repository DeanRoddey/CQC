//
// FILE NAME: ZWaveLevi2S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
//  This is the header file for the actual driver class. We just do the usual thing
//  of deriving from the base server driver class and overriding the virtuals.
//
//  In order to expose driver specific functionality to stuff down in the shared
//  directory, it defines an abstrct mixin interface, which we implement. We just
//  override it's virtuals and pass on the calls to the actual methods inside us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWaveLevi2SDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TZWaveLevi2SDriver : public TCQCServerBase, public MZWLeviSrvFuncs
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveLevi2SDriver() = delete;

        TZWaveLevi2SDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TZWaveLevi2SDriver(const TZWaveLevi2SDriver&) = delete;

        ~TZWaveLevi2SDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWaveLevi2SDriver& operator=(const TZWaveLevi2SDriver&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryTextVal
        (
            const   TString&                strQueryType
            , const TString&                strDataName
            ,       TString&                strToFill
        )   override;

        tCIDLib::TCard4 c4QueryCfg
        (
                    THeapBuf&               mbufToFill
        )   override;

        tCIDLib::TCard4 c4QueryVal
        (
            const   TString&                strValId
        )   override;

        tCIDLib::TCard4 c4QueryVal2
        (
            const   TString&                strValId
            , const TString&                strValName
        )   override;

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        )   override;

        tCIDLib::TVoid SetConfig
        (
            const   tCIDLib::TCard4         c4Bytes
            , const THeapBuf&               mbufNewCfg
        )   override;


        // Inherited from the shared facility mixin
        tCIDLib::TBoolean bLeviFldExists
        (
            const   TString&                strName
        )   const override;

        tCIDLib::TBoolean bLeviQueryFldName
        (
            const   tCIDLib::TCard4         c4FldId
            ,       TString&                strToFill
        )   const override;

        tCIDLib::TBoolean bLeviStoreBoolFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bToStore
        )   override;

        tCIDLib::TBoolean bLeviStoreCardFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4ToStore
        )   override;

        tCIDLib::TBoolean bLeviStoreFloatFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8ToStore
        )   override;

        tCIDLib::TBoolean bLeviStoreIntFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4ToStore
        )   override;

        tCIDLib::TBoolean bLeviStoreStrFld
        (
            const   tCIDLib::TCard4         c4FldId
            , const TString&                strToStore
        )   override;

        tCIDLib::TBoolean bLeviTraceEnabled() const override;

        tCIDLib::TBoolean bLeviWaitUnitMsg
        (
                    TZWaveUnit&             unitSrc
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4CmdId
            ,       tCIDLib::TCardList&     fcolVals
        )   override;

        tCIDLib::TCard4 c4LeviFldIdFromName
        (
            const   TString&                strName
        )   const override;

        tCIDLib::TCard4 c4LeviVRCOPId() const override;

        tCQCKit::EVerboseLvls eLeviVerboseLevel() const override;

        tCIDLib::TVoid LeviQEventTrig
        (
            const   tCQCKit::EStdDrvEvs     eEvent
            , const TString&                strFld
            , const TString&                strVal1
            , const TString&                strVal2
            , const TString&                strVal3
            , const TString&                strVal4
        )   override;

        virtual tCIDLib::TVoid LeviSendMsg
        (
            const   TZWaveUnit&             unitTar
            , const tCIDLib::TCardList&     fcolVals
        )   override;

        virtual tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4Cmd
        )   override;

        virtual tCIDLib::TVoid LeviSendUnitMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
        )   override;

        tCIDLib::TVoid LeviSetFldErr
        (
            const   tCIDLib::TCard4         c4FldId
        )   override;

        const TCQCFldLimit* pfldlLeviLimsFor
        (
            const   tCIDLib::TCard4         c4FldId
            , const TClass&                 clsType
        )   const override;

        const TString& strLeviMoniker() const override;

        TTextOutStream& strmLeviTrace() override;


    protected :
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


    private :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TUnitPollQ = TRefQueue<TZWaveUnit>       ;
        using TGTypeMap = TEArray
        <
            tCIDLib::TCard4, tZWaveLevi2Sh::EGenTypes, tZWaveLevi2Sh::EGenTypes::Count
        >;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckMsgLen
        (
            const   tCIDLib::TCh* const     pszType
            , const tCIDLib::TCard4         c4MsgLen
            , const tCIDLib::TCard4         c4MinLegal
        );

        tCIDLib::TBoolean bLoadConfig
        (
                    TZWaveDrvConfig&        dcfgNew
            , const tCIDLib::TBoolean       bOnConnect
        );

        tCIDLib::TBoolean bParseGenReport
        (
            const   tCIDLib::TBoolean       bSecure
            , const TString&                strMsg
            ,       tCIDLib::TCard4&        c4NodeId
            ,       tCIDLib::TCard4&        c4InstId
            ,       tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TBoolean bReadCfgFile();

        tCIDLib::TBoolean bSendWaitRep
        (
            const   tCIDLib::TCard4         c4ForUnitId
            , const tCIDLib::TCard4         c4ForInstId
            , const tCIDLib::TCard4         c4ForClass
            , const tCIDLib::TCard4         c4ForCmd
            ,       tCIDLib::TCardList&     fcolVals
            , const tZWaveLevi2Sh::EMultiTypes eMultiType = tZWaveLevi2Sh::EMultiTypes::Single
            , const tCIDLib::TBoolean       bSecure = kCIDLib::False
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bSendWaitAck
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bSendWaitAck
        (
            const   tCIDLib::TCard4         c4UnitId
            , const tCIDLib::TCard4         c4InstId
            , const tCIDLib::TCardList&     fcolVals
            , const tZWaveLevi2Sh::EMultiTypes eMultiType = tZWaveLevi2Sh::EMultiTypes::Single
            , const tCIDLib::TBoolean       bSecure = kCIDLib::False
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bSendWaitAck
        (
            const   tCIDLib::TCard4         c4UnitId
            , const tCIDLib::TCard4         c4InstId
            , const tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitTransAck
        (
            const   tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsg
        (
            const   tZWaveLevi2S::ERepTypes eWaitFor
            , const tCIDLib::TCard4         c4ForNode
            , const tCIDLib::TCard4         c4ForInstId
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsg
        (
            const   tZWaveLevi2S::ERepTypes eWaitFor
            , const tCIDLib::TCard4         c4ForNode
            , const tCIDLib::TCard4         c4ForInstId
            ,       tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsg
        (
            const   tZWaveLevi2S::ERepTypes eWaitFor
            , const tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4ForNode
            , const tCIDLib::TCard4         c4ForInstId
            , const tCIDLib::TCard4         c4ForClass
            , const tCIDLib::TCard4         c4ForCmd
            ,       tZWaveLevi2S::ERepTypes& eGot
            ,       tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsgMS
        (
            const   tZWaveLevi2S::ERepTypes eWaitFor
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::TCard4         c4ForNode
            , const tCIDLib::TCard4         c4ForInstId
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsgMS
        (
            const   tZWaveLevi2S::ERepTypes eWaitFor
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::TCard4         c4ForNode
            , const tCIDLib::TCard4         c4ForInstId
            , const tCIDLib::TCard4         c4ForClass
            , const tCIDLib::TCard4         c4ForCmd
            ,       tZWaveLevi2S::ERepTypes& eGot
            ,       tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TVoid CheckNumericVal
        (
            const   TString&                strVal
            , const tCIDLib::TCard4         c4ParmNum
            ,       tCIDLib::TCard4&        c4ToFill
            , const tCIDLib::TCh* const     pszName
            , const tCIDLib::TCard4         c4Min = 0
            , const tCIDLib::TCard4         c4Max = kCIDLib::c4MaxCard
        );

        tCIDLib::TVoid CheckCmdFldParmCnt
        (
            const   TString&                strCmd
            , const tCIDLib::TCard4         c4ParmCnt
            , const tCIDLib::TCard4         c4ExpCnt
            , const tCIDLib::TBoolean       bOrMore = kCIDLib::False
        );

        tCIDLib::TVoid DoAutoCfg
        (
                    TZWaveUnit&             unitTar
        );

        tZWaveLevi2S::ERepTypes eGetLine
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tZWaveLevi2S::ERepTypes eGetMsg
        (
            const   tCIDLib::TEncodedTime   enctEnd
            ,       tCIDLib::TCard4&        c4NodeId
            ,       tCIDLib::TCard4&        c4InstId
            ,       tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tZWaveLevi2S::ERepTypes eGetMsgMSs
        (
            const   tCIDLib::TCard4         c4Millis
            ,       tCIDLib::TCard4&        c4NodeId
            ,       tCIDLib::TCard4&        c4InstId
            ,       tCIDLib::TCardList&     fcolVals
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCQCKit::ECommResults eProcessCmdFld
        (
            const   TString&                strCmdText
        );

        tCIDLib::TVoid EatMsgs
        (
            const   tCIDLib::TCard4         c4WaitMS
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TVoid EnumerateDevices
        (
                    TZWaveDrvConfig&        dcfgUpdate
        );

        tCIDLib::TVoid EnumerateDevType
        (
            const   tCIDLib::TCh* const     pszType
            , const tZWaveLevi2Sh::EGenTypes eGType
            ,       tZWaveLevi2Sh::TGTypeArray& fcolGTypes
        );

        tCIDLib::TVoid GetInitVals();

        tCIDLib::TVoid LogLeviErr
        (
            const   tCIDLib::TCard4         c4Code
            , const tCQCKit::EVerboseLvls   eMinLevel
        );

        const tCIDLib::TCh* pszXlatLeviErr
        (
            const   tCIDLib::TCard4         c4ResCode
        )   const;

        tCIDLib::TVoid ProbeUnit
        (
            const   TString&                strUnitId
            ,       TTextOutStream&         strmTar
        );

        tCIDLib::TVoid ProcessMsg
        (
            const   tZWaveLevi2S::ERepTypes eType
            , const tCIDLib::TCard4         c4NodeId
            , const tCIDLib::TCard4         c4InstId
            ,       tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid ProcessGenReport
        (
            const   tZWaveLevi2S::ERepTypes eType
            , const tCIDLib::TCard4         c4NodeId
            , const tCIDLib::TCard4         c4InstId
            ,       tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid ProcessSceneReport
        (
            const   tCIDLib::TCard4         c4NodeId
            , const tCIDLib::TCard4         c4InstId
            ,       tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid RegisterFields
        (
                    TZWaveDrvConfig&        dcfgNew
        );

        tCIDLib::TVoid SendMsg
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid SendMsg
        (
            const   tCIDLib::TCard4         c4UnitId
            , const tCIDLib::TCard4         c4InstId
            , const tCIDLib::TBoolean       bSecure
            , const tZWaveLevi2Sh::EMultiTypes eMultiType
            , const tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid SendVRCOPMsg
        (
            const   TString&                strMsg
            , const tCIDLib::TBoolean       bTransAck
        );

        tCIDLib::TVoid SendUnitCmd
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCardList&     fcolVals
        );

        tCIDLib::TVoid SendUnitCmd
        (
            const   TZWaveUnit&             unitTar
            , const TZWCmdClass&            zwccSrc
            , const tCIDLib::TCard4         c4CmdClass
            , const tCIDLib::TCard4         c4Cmd
        );

        tCIDLib::TVoid StartTrace
        (
            const   tCIDLib::TBoolean       bStart
        );

        tCIDLib::TVoid ThrowLeviErr
        (
            const   tCIDLib::TCard4         c4Code
        );

        tCIDLib::TVoid WriteCfgFile();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLogZWMsgs
        //      Using the Command field the logging of the raw controller
        //      msgs going out and coming in can be enabled. This is used
        //      in in the field debugging of issues.
        //
        //  m_bNewConfig
        //      When the client uploads a new configuration, we store it in
        //      m_dcfgUpload, and set this flag. The poll method will see that
        //      and load the new config. This way, we don't lock out the client
        //      while we get initial values.
        //
        //  m_bNoProcess
        //      Until we get up through the bulk of the connect process, we
        //      disable the processing of async messages. We only pay attention
        //      to stuff that we are expecting.
        //
        //  m_bsNodeMap
        //      This is the actual node map that we got during the last query
        //      of initial data (which happens on connect or when forced to
        //      re-sync.) For unit id X, the bit at index X-1 indicates
        //      whether that unit is known to our controller.
        //
        //      We keep this because our actual configuration data doesn't
        //      necessarily match what's out there all the time.
        //
        //  m_c4FldId_XXX
        //      The field ids of any non-group/unit oriented fields that we
        //      access enough to want to pre-look up their ids.
        //
        //  m_c4CfgSerialNum
        //      A configuration serial number. Every time new configuration data is
        //      stored we bump this. This lets the client side driver know if new
        //      config is available.
        //
        //  m_c4VerMaj
        //  m_c4VerMin
        //      We get these out during init, from the driver config, for use when
        //      we start a new trace file. THis way we don't have to worry about
        //      locking to get the data out later when the trace file is created.
        //
        //  m_cpcfgSerial
        //      We store away the serial port configuration data passed to us
        //      in the ctor, to avoid having to get it back out of the driver
        //      config ever time we might need it.
        //
        //  m_dcfgCurrent
        //      This is our current unit and group configuration info, loaded from our
        //      configuration file on startup, and sometimes new stuff is pushed up
        //      by the client.
        //
        //  m_dcfgUpload
        //      See m_bNewConfig above. This is where we store new config,
        //      uploaded from the client, till it is loaded by the poll
        //      method.
        //
        //  m_enctNextPoll
        //      The next time we can try to do an active poll on a unit. The
        //      m_c4NextPollInd member is used to round robin through them.
        //
        //  m_enctNextTrans
        //      We impose a minimum inter-transmission delay, because the
        //      VRCOP is pretty fragile. So this is hopefully to prevent
        //      over-running it. This is set to the next time we could send
        //      a message.
        //
        //  m_fcolGTypeMap
        //      A map from our generic type enum to the Z-Wave generic type id.
        //      This makes some stuff more efficient to do.
        //
        //  m_mbufOut
        //      Used by the SendMsg method to build up the actual outgoing
        //      message. Shouldn't be used by anyone else.
        //
        //  m_pcommZWave
        //      Our comm port that we use to talk to the Z-Wave controller.
        //
        //  m_strmTrace
        //      A text file output stream we use for tracing purposes. The protocol
        //      is complex and time sensitive, so it's often difficult to debug,
        //      and figuring things out in the field will be very complex without
        //      this (even with it for that matter, but easier.)
        //
        //  m_strCfgFile
        //  m_strCfgPathm_strCfgPath
        //      The path to our configuration file. We pre-build it and keep it around
        //      since it can't change while we are running. WE keep the path separately
        //      for convenience.
        //
        //  m_strPort
        //      The port we were told to use, provided to us in the ctor.
        //
        //  m_strTmpXXX
        //      String used by the eGetMsg() SendMsg() methods. They shouldn't
        //      be used by anyone else. There's another used by the second layer
        //      waiting methods that call bWaitMsg() to wait for a reply. There's
        //      some for the methods that handing incoming unit reports. And
        //      a couple for the poll method.
        //
        //  m_tcvtOut
        //      Used to transcode outgoing message into the output buffer for
        //      transmission.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bEnableTrace;
        tCIDLib::TBoolean           m_bNewConfig;
        tCIDLib::TBoolean           m_bNoProcess;
        TBitset                     m_bsNodeMap;
        tCIDLib::TCard4             m_c4CfgSerialNum;
        tCIDLib::TCard4             m_c4FldId_InvokeCmd;
        tCIDLib::TCard4             m_c4NextPollInd;
        tCIDLib::TCard4             m_c4TOCount;
        tCIDLib::TCard4             m_c4VerMaj;
        tCIDLib::TCard4             m_c4VerMin;
        TCommPortCfg                m_cpcfgSerial;
        TZWaveDrvConfig             m_dcfgCurrent;
        TZWaveDrvConfig             m_dcfgUpload;
        tCIDLib::TEncodedTime       m_enctNextPoll;
        tCIDLib::TEncodedTime       m_enctNextTrans;
        TGTypeMap                   m_fcolGTypeMap;
        THeapBuf                    m_mbufOut;
        TCommPortBase*              m_pcommZWave;
        TTextFileOutStream          m_strmTrace;
        TString                     m_strCfgFile;
        TString                     m_strCfgPath;
        TString                     m_strPort;
        TString                     m_strTmpIn;
        TString                     m_strTmpOut;
        TString                     m_strTmpPoll1;
        TString                     m_strTmpPoll2;
        TString                     m_strTmpRep1;
        TString                     m_strTmpRep2;
        TString                     m_strTmpRep3;
        TString                     m_strTmpWait;
        TUSASCIIConverter           m_tcvtOut;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveLevi2SDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


