//
// FILE NAME: ZWaveLeviS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//   CLASS: TZWaveLeviSDriver
//  PREFIX: orbs
// ---------------------------------------------------------------------------
class TZWaveLeviSDriver : public TCQCServerBase, public MZWaveCtrl
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWaveLeviSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TZWaveLeviSDriver();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
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
            const   TString&                strCmd
            , const TString&                strParms
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

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        );

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
        TZWaveLeviSDriver();
        TZWaveLeviSDriver(const TZWaveLeviSDriver&);
        tCIDLib::TVoid operator=(const TZWaveLeviSDriver&);


        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        typedef TRefQueue<TZWaveUnit>       TUnitPollQ;


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

        tCIDLib::TBoolean bParseGenRepport
        (
            const   TString&                strMsg
            ,       tCIDLib::TCard4* const  pc4ToFill
            ,       tCIDLib::TCard4&        c4Count
            , const tCIDLib::TCard4         c4MaxBytes
            ,       tCIDLib::TCard4&        c4NodeId
        );

        tCIDLib::TBoolean bProcessCmdFld
        (
            const   TString&                strCmdText
        );

        tCIDLib::TBoolean bSendAndWaitAck
        (
            const   TString&                strToSend
            , const tCIDLib::TCard4         c4WaitMS
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitTransAck
        (
            const   tCIDLib::TCard4         c4WaitMS
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsg
        (
            const   tZWaveLeviS::ERepTypes  eWaitFor
            , const tCIDLib::TCard4         c4WaitMS
            ,       TString&                strToFill
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TBoolean bWaitMsg
        (
            const   tZWaveLeviS::ERepTypes  eWaitFor
            , const tCIDLib::TCard4         c4WaitMS
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCIDLib::TCard4 c4FindVRCOP
        (
            const   TZWaveDrvConfig&        dcfgSrc
            , const tCIDLib::TBoolean       bThrowIfNot
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

        tCIDLib::TVoid DoPollRound();

        tZWaveLeviS::ERepTypes eGetLine
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tZWaveLeviS::ERepTypes eGetMsg
        (
                    TString&                strToFill
            , const tCIDLib::TEncodedTime   enctEnd
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tZWaveLeviS::ERepTypes eGetMsgMSs
        (
                    TString&                strToFill
            , const tCIDLib::TCard4         c4WaitMS
            ,       tCIDLib::TCard4&        c4ResCode
            , const tCIDLib::TBoolean       bThrowErrs = kCIDLib::True
        );

        tCQCKit::ECommResults eSendFldWrite
        (
            const   TString&                strToSend
            , const tCIDLib::TCh* const     pszType
            , const TZWaveUnit&             unittar
        );

        tCIDLib::TVoid EnumeratedDevices
        (
                    TZWaveDrvConfig&        dcfgUpdate
        );

        tCIDLib::TVoid EnumeratedDevType
        (
            const   tCIDLib::TCh* const     pszType
            , const tCIDLib::TCard4         c4GenId
            ,       tZWaveLeviSh::TValArray& fcolTypes
        );

        tCIDLib::TVoid GetInitVals();

        const tCIDLib::TCh* pszXlatLeviErr
        (
            const   tCIDLib::TCard4         c4ResCode
        )   const;

        tCIDLib::TVoid ProcessMsg
        (
            const   tZWaveLeviS::ERepTypes  eType
            , const TString&                strMsg
        );

        tCIDLib::TVoid ProcessBasicReport
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid ProcessGenReport
        (
            const   tZWaveLeviS::ERepTypes  eType
            , const TString&                strMsg
        );

        tCIDLib::TVoid ProcessSceneReport
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid RegisterFields
        (
                    TZWaveDrvConfig&        dcfgNew
        );

        tCIDLib::TVoid SendMsg
        (
            const   TString&                strToSend
        );

        tCIDLib::TVoid SendUnitCmd
        (
            const   TString&                strToSend
            , const tCIDLib::TCard4         c4WaitMS
        );

        tCIDLib::TVoid LogLeviErr
        (
            const   tCIDLib::TCard4         c4Code
            , const tCQCKit::EVerboseLvls   eMinLevel
        );

        tCIDLib::TVoid ThrowLeviErr
        (
            const   tCIDLib::TCard4         c4Code
        );


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
        //  m_c4CfgSerialNum
        //      Every time we store new configuration (after a scan, not when
        //      the client actually uploads it), we bump this. We make it
        //      available to the client drivers so they can see if the config
        //      has changed.
        //
        //  m_c4FldId_XXX
        //      The field ids of any non-group/unit oriented fields that we
        //      access enough to want to pre-look up their ids.
        //
        //  m_c4PollableCnt
        //      When we get new config after a new connection or reconfiguration
        //      process, we count how many units are pollable.
        //
        //  m_colPollItems
        //      This is used by the poll method. It finds all the units that
        //      are ready to poll and puts them into this list sorted by the
        //      amount overdue they are (most overdue first.) This guy is
        //      non-adopting of course. It just points to the units that are
        //      in the config object.
        //
        //  m_cpcfgSerial
        //      We store away the serial port configuration data passed to us
        //      in the ctor, to avoid having to get it back out of the driver
        //      config ever time we might need it.
        //
        //  m_dcfgCurrent
        //      This is our current unit and group configuration info, loaded
        //      from driver configuration storage on startup, and sometimes
        //      new stuff is pushed up by the client.
        //
        //  m_dcfgUpload
        //      See m_bNewConfig above. This is where we store new config,
        //      uploaded from the client, till it is loaded by the poll
        //      method.
        //
        //  m_enctNextTrans
        //      We impose a minimum inter-transmission delay, because the
        //      VRCOP is pretty fragile. So this is hopefully to prevent
        //      over-running it. This is set to the next time we could send
        //      a message.
        //
        //  m_enctNextPoll
        //      We limit how often we look for new units to poll, so as not
        //      to beat the network to death.
        //
        //  m_mbufOut
        //      Used by the SendMsg method to build up the actual outgoing
        //      message. Shouldn't be used by anyone else.
        //
        //  m_pcommZWave
        //      Our comm port that we use to talk to the Z-Wave controller.
        //
        //  m_strCfgScope
        //      The path to our configuration in the config server. We set it
        //      up and keep it around for use.
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
        tCIDLib::TBoolean       m_bLogZWMsgs;
        tCIDLib::TBoolean       m_bNewConfig;
        tCIDLib::TBoolean       m_bNoProcess;
        TBitset                 m_bsNodeMap;
        tCIDLib::TCard4         m_c4CfgSerialNum;
        tCIDLib::TCard4         m_c4FldId_Command;
        tCIDLib::TCard4         m_c4FldId_PollsDue;
        tCIDLib::TCard4         m_c4PollableCnt;
        tCIDLib::TCard4         m_c4TOCount;
        tZWaveLeviSh::TUnitList m_colPollItems;
        TCommPortCfg            m_cpcfgSerial;
        TZWaveDrvConfig         m_dcfgCurrent;
        TZWaveDrvConfig         m_dcfgUpload;
        tCIDLib::TEncodedTime   m_enctNextTrans;
        tCIDLib::TEncodedTime   m_enctNextPoll;
        THeapBuf                m_mbufOut;
        TCommPortBase*          m_pcommZWave;
        TString                 m_strCfgScope;
        TString                 m_strPort;
        TString                 m_strTmpIn;
        TString                 m_strTmpOut;
        TString                 m_strTmpPoll1;
        TString                 m_strTmpPoll2;
        TString                 m_strTmpRep1;
        TString                 m_strTmpRep2;
        TString                 m_strTmpRep3;
        TString                 m_strTmpWait;
        TUSASCIIConverter       m_tcvtOut;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TZWaveLeviSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


