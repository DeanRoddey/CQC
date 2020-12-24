//
// FILE NAME: CQCWebSrvC_WorkerThread.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 091/17/2014
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
//  This class implements a base class for Websocket based handlers on the Web Server
//  side. We currently have two derivatives, one C++ one for our browser based RIVA
//  client, and one that is for user written CML based handlers.
//
//  Because of the event driven nature of Websockets, and the fact that CML handlers
//  must be handled in a single threaded nature, while C++ ones may create various
//  threads, all of which are sending messages, this class just handles the sending
//  and receiving of messages on behalf of the derived class.
//
//  1. We have an output msg queue
//  2. This base handler's thread manages the socket and the queue.
//  3. We associate an event with the socket and have another event for the output queue.
//  4. Our thread will then block on both those events all the time, waiting for either
//     data available on the socket, or messages available in the output queue.
//  5. When it wakes up, if it didn't timeout, it will process either an incoming msg
//     our outgoing, depending on which event was triggered. It only does one per round
//     so that it keeps things smooth and balances the load.
//  6. All sending of messages happens through us, so our public message sending methods
//     will add an item to the queue and trigger the out msg event. This tells our thread
//     that there is data to send.
//  7. For each incoming msg we call ProcessMsg(). This is done synchronously so the derived
//     class must handle it very quickly or queue it for later processing.
//
//  Though most handlers will never need to worry about it, since they tend to be more
//  call and response style, if the handler sends a lot of unacked messages it needs to
//  make sure there's room in the outgoing queue and wait a bit for room to become available
//  if not. We provide a method to do that.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TWebSockBuf
//  PREFIX: wsb
// ---------------------------------------------------------------------------
class TWebSockBuf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWebSockBuf();

        TWebSockBuf
        (
            const   tCIDLib::TCard4         c4InitSz
        );

        TWebSockBuf
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        );

        TWebSockBuf(const TWebSockBuf&) = delete;
        TWebSockBuf(TWebSockBuf&&) = delete;

        ~TWebSockBuf();


        // -------------------------------------------------------------------
        //  Public constructors
        // -------------------------------------------------------------------
        TWebSockBuf& operator=(const TWebSockBuf&) = delete;
        TWebSockBuf& operator=(TWebSockBuf&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_c1Type
        //      The Websocket msg type indicator, so that we can support the various
        //      types, but have them all just flattened down to buffers and queued up
        //      to be sent. The sender just passes this flag along.
        //
        //  m_c4CurBytes
        //      The number of bytes in the buffer actually used.
        //
        //  m_pmbufData
        //      The data buffer, which we own and will release.
        // -------------------------------------------------------------------
        tCIDLib::TCard1 m_c1Type;
        tCIDLib::TCard4 m_c4CurBytes;
        TMemBuf*        m_pmbufData;
};



// ---------------------------------------------------------------------------
//   CLASS: TWebsockThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWebsockThread : public TThread
{
    public :
        // --------------------------------------------------------------------
        //  Public class types
        //
        //  We need a current state. Because control frames can come in the middle
        //  of fragmented data msgs, and because we have to deal differently with
        //  close messages depending on whether we initiated or the other side did
        //  (via a ctrl msg that could also come in the middle of a fragmented
        //  msg, we need to be able to remember state changes for when we get back
        //  to the main loop.
        //
        //  Connecting
        //      We need to call the virtual method that lets the derived class know
        //      we are now connected and let him initialize as required. This is done
        //      before we enter our loop, so if it fails we never go further. If it
        //      works we enter the loop in Ready state.
        //
        //  Ready
        //      We are in ready state, which means we are waiting for msgs to come in
        //      or be ready to send out.
        //
        //  InMsg
        //      We are processing a non-final message, i.e. during Ready we got a non
        //      final frame, and we need to now wait for the remaining fragments. This
        //      is the same as Ready in terms of what we do.
        //
        //  WaitClientEnd
        //      Either we or a derivative has asked to shutdown. So we will have sent
        //      our close and need to wait for the client to respond with his own
        //      close before we exit. We will only wait so long.
        //
        //  End
        //      Causes the main loop to exit.
        // --------------------------------------------------------------------
        enum class EStates
        {
            Connecting
            , Ready
            , InMsg
            , WaitClientEnd
            , End
        };


        // --------------------------------------------------------------------
        //  Public, static methods
        // --------------------------------------------------------------------
        static tCIDLib::TBoolean bControlType
        (
            const   tCIDLib::TCard1         c1Type
        );

        static tCIDLib::TBoolean bValidateReqInfo
        (
                    TCIDDataSrc&            cdsData
            , const TString&                strResource
            , const tCIDLib::TKVPList&      colHdrLines
            , const TString&                strType
            ,       TString&                strSecKey
        );


        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TWebsockThread() = delete ;

        TWebsockThread(const TWebsockThread&) = delete;
        TWebsockThread(TWebsockThread&&) = delete;

        ~TWebsockThread();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWebsockThread& operator=(const TWebsockThread&) = delete;
        TWebsockThread& operator=(TWebsockThread&&) = delete;


        // --------------------------------------------------------------------
        //  Public, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bFindQParam
        (
            const   TString&                strKey
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bIsReady() const;

        tCIDLib::TBoolean bSimMode() const;

        tCIDLib::TBoolean bValidateLogin
        (
            const   TString&                strUser
            , const TString&                strPassword
            ,       TCQCSecToken&           sectToFill
            ,       TCQCUserAccount&        uaccToFill
            ,       TString&                strFailReason
        );

        tCIDLib::TCard4 c4QParamCnt() const;

        tCIDLib::EExitCodes eServiceClient();

        tCQCWebSrvC::EWSockTypes eType() const;

        tCIDLib::TVoid EnableMsgLogging
        (
            const   tCIDLib::TBoolean       bState
        );

        const TKeyValuePair& kvalQParamAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid PauseOutput
        (
            const   tCIDLib::TBoolean       bState
        );

        tCIDLib::TVoid Reset
        (
                    TCIDDataSrcJan&         janSrc
            , const TURL&                   urlReq
            , const TString&                strSecKey
        );

        tCIDLib::TVoid SetFieldList
        (
            const   tCIDLib::TStrList&      colToSet
        );

        tCIDLib::TVoid StartShutdown
        (
            const   tCIDLib::TCard2         c2Err
        );

        tCIDLib::TVoid WaitOutSpaceAvail
        (
            const   tCIDLib::TCard4         c4WaitMSs
        );


    protected :
        // --------------------------------------------------------------------
        //  Hidden constructors
        // --------------------------------------------------------------------
        TWebsockThread
        (
            const   tCQCWebSrvC::EWSockTypes eType
            , const tCIDLib::TBoolean       bSimMode
        );


        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


        // --------------------------------------------------------------------
        //  Protected, virtual methods
        // --------------------------------------------------------------------
        virtual tCIDLib::TCard4 c4WSInitialize
        (
            const   TURL&                   urlReq
            ,       TString&                strRepText
            ,       TString&                strErrText
        ) = 0;

        virtual tCIDLib::TVoid CheckShutdownReq() const = 0;

        virtual tCIDLib::TVoid Connected() = 0;

        virtual tCIDLib::TVoid Disconnected() = 0;

        virtual tCIDLib::TVoid FieldChanged
        (
            const   TString&                strMon
            , const TString&                strField
            , const tCIDLib::TBoolean       bGoodValue
            , const TCQCFldValue&           fvValue
        ) = 0;

        virtual tCIDLib::TVoid Idle() = 0;

        virtual tCIDLib::TVoid ProcessMsg
        (
            const   TString&                strMsg
        ) = 0;

        virtual tCIDLib::TVoid WSTerminate() = 0;


        // --------------------------------------------------------------------
        //  Protected, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid QueueTextMsg
        (
            const   TString&                strText
        );

        tCIDLib::TVoid QueueTextMsg
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        );

        tCIDLib::TVoid SendCtrlMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TCard2         c2Payload
        );

        tCIDLib::TVoid SendMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4DataCnt
        );

        tCIDLib::TVoid SendMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const tCIDLib::TVoid* const   pData
            , const tCIDLib::TCard4         c4DataCnt
        );

        const TURL& urlReq() const;


    private :
        // --------------------------------------------------------------------
        //  Private data types
        // --------------------------------------------------------------------
        using TPollInfoList = TVector<TCQCFldPollInfo>;
        using TOutMsgQ = TRefQueue<TWebSockBuf>;


        // --------------------------------------------------------------------
        //  Protected, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bGetFragment
        (
                    tCIDLib::TCard1&        c1Type
            ,       tCIDLib::TBoolean&      bFinal
            ,       TMemBuf&                mbufToFill
            ,       tCIDLib::TCard4&        c4MsgBytes
        );

        tCIDLib::TVoid Cleanup
        (
            const   tCIDLib::TBoolean       bCallDiscon
        );

        tCIDLib::TVoid ClearOutQ();

        tCIDLib::TVoid HandleMsg
        (
            const   tCIDLib::TCard1         c1Type
            , const THeapBuf&               mbufData
            , const tCIDLib::TCard4         c4DataCnt
        );

        tCIDLib::TVoid PollFields();

        tCIDLib::TVoid SendAccept();

        tCIDLib::TVoid SendClose
        (
            const   tCIDLib::TCard2         c2Code
        );

        tCIDLib::TVoid SendPing();


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLogStateInfo
        //      The client can set this via a query parameter, logstateinfo=1.
        //
        //  m_bSimMode
        //      We can be put into 'simulator' mode, which is used to support debugging
        //      CML based websocket handlers, and possibly other derived classes might
        //      make use of it. For us, we just avoid doing a few things when in
        //      simulator mode. For instance, we won't be spun up as a separate thread
        //      in that case, but our eServiceClient() method will just be called in
        //      the context of a simulator thread. So we can't do some thread oriented
        //      stuff.
        //
        //  m_bPauseOutput
        //      The derived class can ask us to stop sending out msgs from the output
        //      queue. For instance, in the RIVA handler it has to get an ack from the
        //      client at least every so many msgs sent. Once it hits that point it will
        //      ask us to stop sending until it sees an ack. If our queue fills up, then
        //      we will error out this session, so he can't do it for too long relative to
        //      the bulk he's sending.

        //  m_bWaitPong
        //  m_c4PingVal
        //      Remember if we are waiting for a pong response to a ping we have sent.
        //      We send a 32 bit value in each ping, which we increment each time, so
        //      this is the value we should get back in the pong. The byte order doesn't
        //      matter since the client doesn't interpret it, it just echoes it back.
        //
        //  m_colFields
        //      The derived class can ask us to monitor fields. We set up these fields
        //      on the poll engine. We get back a poll info object for each field
        //      which we use to check for changes.
        //
        //  m_colOutMsgQ
        //      The outgoing message queue. All sent messages are queued up here and
        //      then sent by out this thread. It is thread safe, and really the only thing
        //      that needs any sync.
        //
        //  m_colQParams
        //      Any query parms in the original URL. We make these available to the derived
        //      class in case it needs them.
        //
        //  m_enctEndTimer
        //      When we start a close from our side, we will wait for up to a certain
        //      amount of time for the other side to respond. Else we'll give up.
        //
        //  m_enctLastInMsg
        //  m_enctLastOutMsg
        //      We track the last time we sent a message (on behalf of the derived class
        //      primarily) and the last time we got one from the client. If we haven't
        //      sent a message in the last 30 seconds, we'll send a ping so that the
        //      client doesn't give up on us.
        //
        //  m_evOutQ
        //      This event is used to indicate when new data is available in the outgoing
        //      message queue. It is used in conjunction with the event socket below to
        //      provide smooth processing of incoming and outgoing messages. We have to
        //      synchronize access to it to avoid race conditions. We use the output msg
        //      queue's mutex for that, since they are used together.
        //
        //  m_evSock
        //      We associate an event with our socket so that we can block on either
        //      msgs to send entering the queue, or incoming data to process.
        //
        //  m_eState
        //      Our current state, which the main loop uses to know what to do. It
        //      starts out in Connecting state and moves towards eventually one of the
        //      close states.
        //
        //  m_eType
        //      The derived type provides us a type indicator, so that we don't have
        //      to use RTTI for the simple typing needs we have.
        //
        //  m_mbufReadFrag
        //      This is used for reading in fragments. It's passed to eGetFragment()
        //      get any incoming fragement content. This is used by the main processing
        //      loop.
        //
        //  m_mbufReadMsg
        //      We need a buffer to build up fragments into to create the final msg. This
        //      is used by the main processing loop, which just adds fragements into it
        //      until the final one.
        //
        //  m_pcdsServer
        //      We are given the data source. In the case of a normal (non-simulator)
        //      invocation, our eProcess() method immediately puts a janitor on it,
        //      which insures it gets cleaned up. In simulator mode, the base CML class
        //      handler, which creates us, insures it gets cleaned up.
        //
        //      Only this thread (or the simulator thread) deals with this object so it
        //      doesn't require any sync. Outgoing messages are queued on this thread for
        //      transmission by our thread. And thread does the reading of incoming msgs.
        //
        //  m_pstrmLog
        //      The derived class can enable a log of messages exchanged. If enabled
        //      this is set, else it's null.
        //
        //  m_strSecKey
        //      The websock security key that was sent by the client, which we need
        //      for the ugrade acceptance reply.
        //
        //  m_strTextDispatch
        //      A temp string to use for transcoding text messages to a string to pass
        //      on to the derived class.
        //
        //  m_tcvtMsgs
        //      A converter to convert messages to/from UTF-8.
        //
        //  m_tmLog
        //      A time object we use in logging. We pre-set it up for the desired time
        //      format, and just set it to the current time any time we want to output
        //      it to the log.
        //
        //  m_urlReq
        //      The original request URL from the first HTTP line. We keep it around so
        //      that we can make it available to the derived class if needed.
        // --------------------------------------------------------------------
        tCIDLib::TBoolean           m_bLogStateInfo;
        tCIDLib::TBoolean           m_bPauseOutput;
        tCIDLib::TBoolean           m_bSimMode;
        tCIDLib::TBoolean           m_bWaitPong;
        tCIDLib::TCard4             m_c4PingVal;
        TPollInfoList               m_colFields;
        TOutMsgQ                    m_colOutMsgQ;
        tCIDLib::TKVPList           m_colQParams;
        tCIDLib::TEncodedTime       m_enctEndTimer;
        tCIDLib::TEncodedTime       m_enctLastInMsg;
        tCIDLib::TEncodedTime       m_enctLastOutMsg;
        TEvent                      m_evOutMsgQ;
        TEvent                      m_evSock;
        EStates                     m_eState;
        tCQCWebSrvC::EWSockTypes    m_eType;
        THeapBuf                    m_mbufReadFrag;
        THeapBuf                    m_mbufReadMsg;
        TMutex                      m_mtxSync;
        TCIDSockStreamBasedDataSrc* m_pcdsServer;
        TTextFileOutStream*         m_pstrmLog;
        TString                     m_strSecKey;
        TString                     m_strTextDispatch;
        TUTF8Converter              m_tcvtMsgs;
        TTime                       m_tmLog;
        TURL                        m_urlReq;
};

#pragma CIDLIB_POPPACK


