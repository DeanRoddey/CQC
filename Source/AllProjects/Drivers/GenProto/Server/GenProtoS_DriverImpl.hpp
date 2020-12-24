//
// FILE NAME: GenProtoS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/22/2003
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
//  from the base server side class and override callbacks. We have to parse
//  the protocol file and build up the data structures that we use to
//  process the protocol, and then implement the class callbacks in terms
//  of those data structures.
//
//  The parsing code is in the GenProtoS_Parse.cpp file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


class TGenProtoTokenizer;


// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------
class GENPROTOSEXPORT TGenProtoSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoSDriver() = delete;

        TGenProtoSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TGenProtoSDriver(const TGenProtoSDriver&) = delete;
        TGenProtoSDriver(TGenProtoSDriver&&) = delete;

        ~TGenProtoSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoSDriver& operator=(const TGenProtoSDriver&) = delete;
        TGenProtoSDriver& operator=(TGenProtoSDriver&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestMode() const;

        tCIDLib::TBoolean bTestMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tGenProtoS::EDbgMsgs eMsgToShow() const;

        tGenProtoS::EDbgMsgs eMsgToShow
        (
            const   tGenProtoS::EDbgMsgs    eToSet
        );

        tCIDLib::TVoid HideMsgs
        (
            const   tGenProtoS::EDbgMsgs    eToHide
        );

        tCIDLib::TVoid ParseProtocol
        (
            const   THeapBuf&               mbufSrc
            , const tCIDLib::TCard4         c4SrcBytes
        );

        tCIDLib::TVoid SetDebugCallback
        (
                    MGenProtoDebug* const   pmdbgToUse
        );

        tCIDLib::TVoid ShowMsgs
        (
            const   tGenProtoS::EDbgMsgs    eToShow
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bShouldLog
        (
            const   TError&                 errToCatch
            , const tCQCKit::EVerboseLvls   eMinLevel = tCQCKit::EVerboseLvls::Medium
        )   const final;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   final;

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        )   final;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::EDrvInitRes eInitializeImpl() final;

        tCQCKit::ECommResults eIntFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TInt4          i4NewValue
        )   final;

        tCQCKit::ECommResults eFloatFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TFloat8        f8NewValue
        )   final;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strValue
        )   final;

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoFullExchange
        (
                    TThread&                thrCalling
            ,       TGenProtoCallRep&       clrpToDo
            , const tCIDLib::TBoolean       bConnecting
        );

        tCIDLib::TBoolean bGetReply
        (
                    TThread&                thrCalling
            , const tCIDLib::TCard4         c4EndTime
            , const tCIDLib::TBoolean       bFastCheck
        );

        tCIDLib::TVoid CheckAsync
        (
                    TThread&                thrCalling
        );

        tCQCKit::ECommResults eDoConnect
        (
                    TThread&                thrCalling
        );

        tCQCKit::ECommResults eWriteField
        (
                    TGenProtoFldInfo&       fldiTar
        );

        TGenProtoFldInfo* pfldiFindByName
        (
            const   TString&                strNameToFind
        );

        const TGenProtoFldInfo* pfldiFindByName
        (
            const   TString&                strNameToFind
        )   const;

        TGenProtoFldInfo* pfldiValidateId
        (
            const   TString&                strFldName
            , const tCIDLib::TCard4         c4FldId
        );

        TGenProtoMap* pmapFindByName
        (
            const   TString&                strNameToFind
        );

        TGenProtoExprNode* pnodeMakeNumericConst
        (
                    TGenProtoTokenizer&     tokToUse
            , const TString&                strTokenText
        )   const;

        TGenProtoExprNode* pnodeOptimizeNode
        (
                    TGenProtoExprNode* const pnodeToOpt
        );

        TGenProtoExprNode* pnodeParseExpression
        (
                    TGenProtoTokenizer&     tokToUse
            ,       TString&                strToUse
            , const tCIDLib::TCard4         c4Level
            , const TGenProtoFldInfo* const pfldiCur
            , const tGenProtoS::ESpecNodes  eExprType
        );

        TGenProtoReply* prepFindByName
        (
            const   TString&                strNameToFind
        );

        TGenProtoReply* prepMatchReply
        (
                    TGenProtoCtx&           ctxToUse
            ,       TGenProtoReply* const   prepExpected
        );

        TGenProtoVarInfo* pvariFindByName
        (
            const   TString&                strNameToFind
        );

        tCIDLib::TVoid ParseFields
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseMapsInfo
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseMsgMatches
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParsePollEvents
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseProtocolInfo
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseQueries
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseReplies
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseTryConnect
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseReconnect
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid ParseVariables
        (
                    TGenProtoTokenizer&     tokToUse
            , const tCIDLib::TBoolean       bConst
        );

        tCIDLib::TVoid ParseWriteCmds
        (
                    TGenProtoTokenizer&     tokToUse
        );

        tCIDLib::TVoid SetDeadMode
        (
            const   tCIDLib::TBoolean       bToSet
            ,       TThread&                thrThis
        );

        tCIDLib::TVoid WaitForMinSendInterval
        (
            const   tCIDLib::TCard4         c4PerMsg
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDeadMode
        //      Used in 'Dead If Off' protocols, to remember whether or not
        //      we are in dead mode, so that we don't waste time redoing the
        //      work again and again if we are already in the requested mode.
        //
        //  m_bTestMode
        //      When we are being run within the test harness, we need to
        //      avoid doing some things, such as trying to load the protocol
        //      from the data server. The test harness will give us the
        //      protocol data.
        //
        //  m_c4AcceptStateInd
        //      The index of the state in the state machine that indicates an
        //      acceptance of a legal message. This is set during the parse of
        //      the state machine block.
        //
        //  m_c4LastSendTime
        //      The millisecond time since the last message we sent to the
        //      device. This is used in conjunction with m_c4MinSendInterval
        //      to providing throttling when devices require that.
        //
        //  m_c4MinSendInterval
        //      The minimum interval between sending messages to the device,
        //      in milliseconds. It defaults to zero if not provided.
        //
        //  m_c4PostConnWait
        //      They can indicate an optional period of time to wait after
        //      a successful connect, since some devices can need a little
        //      'settle' time after a connection. By default it's zero.
        //
        //  m_c4Timeouts
        //  m_c4TimeoutMax
        //      They can tell us to be forgiving of timeouts since some
        //      devices won't respond if they are struggling. So they can
        //      set a timeout max value. If we see that many timeouts in
        //      a row, we assume the connection is lost. Each time we get
        //      a good reply, we zero the counter back out.
        //
        //  m_clrpTryConnect
        //      The call/response object that we use when we try to connect
        //      to the protice on startup or after being disconnected.
        //
        //  m_colFields
        //      A list of field definitions, driven by the Fields= block of
        //      the protocol file. For each one we put a TGenProtoFldInfo object
        //      into this list. It contains the field definition info, the
        //      storage for the runtime value, and the expression list for
        //      building a write command to update the field.
        //
        //  m_colMaps
        //      This is our list of maps, where we store the maps defined by
        //      the protocol file in the Map= block. They are named and must
        //      be uniquely named, so we use a hash set to store them.
        //
        //  m_colMatches
        //      A list of msg match objects. When we get a valid message, we
        //      run through these to see if the message matches any defined
        //      reply.
        //
        //  m_colPoll
        //      This contains our list of call/response items to do in our
        //      polling round robin.
        //
        //  m_colQueries
        //      Our list of queries defined in the protocol file. They are
        //      named and referenced by later blocks in the file, so we store
        //      them in a hash set for fast lookup and uniqueness.
        //
        //  m_colReconnect
        //      This contains our list of call/response items to do after a
        //      reconnect.
        //
        //  m_colReplies
        //      Our list of replies defined in the protocol file. They are
        //      named and referenced by later blocks in the file, so we store
        //      them in a hash set for fast lookup and uniqueness.
        //
        //  m_colStateMachine
        //      The list of states and their transitions that is used to parse
        //      legal messages out of the stream of com port data.
        //
        //  m_colVariables
        //      The list of variables and constants that the user created.
        //
        //  m_commDev
        //      The comm port object that we use to talk to the protice on. The
        //      user tells us which port to use, and the protocol file gives
        //      us the port config, stored in m_cpcfgDev.
        //
        //  m_ctxQuery
        //  m_ctxReply
        //  m_ctxWriteCmd
        //      When expressions are evaluated, they require an expression
        //      context, which provides them runtime info. We pre-create some
        //      for use in various expression contexts, to avoid having to
        //      do them over and over. Since we sync between the poll thread
        //      and incoming remote calls, its ok to do this.
        //
        //      The Reply context is used for all of the processing that's
        //      done on the incoming reply, i.e. msg matching, validation,
        //      and storing results. The Query is for building the outgoing
        //      query messages. The WriteCmd is for building out field write
        //      messages.
        //
        //  m_eMsgToShow
        //      In debug mode (under the test harness) we send debugg messages
        //      for various types of events according to the bit flags in
        //      here.
        //
        //  m_eProtoType
        //      Some devices are one way, meaning that they provide no status
        //      information back. In that case, we don't do any connect or
        //      poll operations. So they can indicate this so that we know
        //      that no poll or connect blocks are needed and will just
        //      immediately say we are on-line and do nothing in polling
        //      callbacks.
        //
        //      Some are two way, but the stupid device will only respond
        //      to power on when it powered off. So we have a special
        //      type for that. For those, instead of taking the driver
        //      offline when we lose touch, we put the readable fields
        //      into error state. This way, writeable fields are still
        //      available to force the device back online.
        //
        //  m_gpinfoThis
        //      The general protocol information that we parse from the file.
        //
        //  m_pdevcTar
        //      A pointer to the device connection object that we use to
        //      talk to the device. It hides the details of what kind of
        //      device we are talking to. We are given one of these by the
        //      driver, and adopt it.
        //
        //  m_pmdbgCallback
        //      This is used mostly by the test harness program that lets
        //      folks test out protocols. If this is set, then we will call
        //      back on this interface's methods. We don't own it, since it
        //      is usually mixed into some other object and so we cannot
        //      delete it.
        //
        //  m_strEncoding
        //      The text encoding used by this protice, if any. It will be set
        //      to ASCII if its not indicated in the protocol file.
        //
        //  m_strTargetModel
        //      The target model that we are working on. If the protocol only
        //      has one model, then it has to be that. Else it will be one
        //      of the models in the model list given in the protocol file.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDeadMode;
        tCIDLib::TBoolean       m_bTestMode;
        tCIDLib::TCard4         m_c4AcceptStateInd;
        tCIDLib::TCard4         m_c4LastSendTime;
        tCIDLib::TCard4         m_c4MinSendInterval;
        tCIDLib::TCard4         m_c4PostConnWait;
        tCIDLib::TCard4         m_c4Timeouts;
        tCIDLib::TCard4         m_c4TimeoutMax;
        TGenProtoCallRep        m_clrpTryConnect;
        tGenProtoS::TFldList    m_colFields;
        tGenProtoS::TMapList    m_colMaps;
        tGenProtoS::TMatchList  m_colMatches;
        tGenProtoS::TCRList     m_colPollEvents;
        tGenProtoS::TQueryList  m_colQueries;
        tGenProtoS::TCRList     m_colReconnect;
        tGenProtoS::TReplyList  m_colReplies;
        tGenProtoS::TStateList  m_colStateMachine;
        tGenProtoS::TVarList    m_colVariables;
        TGenProtoCtx            m_ctxQuery;
        TGenProtoCtx            m_ctxReply;
        TGenProtoCtx            m_ctxWriteCmd;
        tGenProtoS::EDbgMsgs    m_eMsgToShow;
        tGenProtoS::EProtoTypes m_eProtoType;
        TGenProtoInfo           m_gpinfoThis;
        TDevConn*               m_pdevcTar;
        MGenProtoDebug*         m_pmdbgCallback;
        TString                 m_strEncoding;
        TString                 m_strTargetModel;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

