//
// FILE NAME: MQTTS_IOThread.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2019
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
//  This is is the header for the I/O thread, which does all of the socket
//  connection and msg I/O to the MQTT server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// We need to forward ref the driver since we both reference each other.
class TMQTTS;


// ---------------------------------------------------------------------------
//  CLASS: TMQTTIOThread
// PREFIX: fac
// ---------------------------------------------------------------------------
class TMQTTIOThread : public TThread
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructors.
        // -------------------------------------------------------------------
        TMQTTIOThread() = delete;

        TMQTTIOThread
        (
                    TMQTTS* const           psdrvMQTT
        );

        TMQTTIOThread(const TMQTTIOThread&) = delete;

        ~TMQTTIOThread();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTIOThread& operator=(const TMQTTIOThread&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryFldDefs
        (
                    tCQCKit::TFldDefList&   colToFill
        );

        tCIDLib::TBoolean bQueueDrvEvent
        (
                    TMQTTDrvEvPtr&          evptrToQ
        );

        tMQTTSh::EClStates eCurState() const
        {
            return m_eCurState;
        }



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() override;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TDrvEventQ = TQueue<TMQTTDrvEvPtr>;


        // -------------------------------------------------------------------
        //  Private data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bConnectMQTT() noexcept;

        tCIDLib::TBoolean bConnectSocket() noexcept;

        tCIDLib::TBoolean bInitialize() noexcept;

        tCIDLib::TBoolean bLoadConfig() noexcept;

        tCIDLib::TBoolean bSubscribe() noexcept;

        tCIDLib::TCard2 c2GetNextPacketId() noexcept;

        tCIDLib::TVoid CheckKeepalive() noexcept;

        tCIDLib::TVoid Disconnect() noexcept;

        tCIDLib::TVoid LogSysExcept
        (
            const   tCIDLib::TCh* const     pszIn
            , const tCIDLib::TCard4         c4Line
        );

        TMQTTInMsgPtr mptrReadMsg() noexcept;

        TMQTTInMsgPtr mptrWaitReply
        (
            const   tCQCMQTT::EPacketTypes  eWaitType
            , const tCIDLib::TCard2         c2WaitId
            , const tCIDLib::TCard4         c4WaitMSs
        );

        tCIDLib::TVoid ProcessDrvEvent
        (
                    TMQTTDrvEvent&          drvevNew
        );

        tCIDLib::TVoid ProcessDrvEvents() noexcept;

        tCIDLib::TVoid ProcessMsg
        (
            const   TMQTTInMsgPtr&          mptrSrc
        )   noexcept;

        tCIDLib::TVoid ProcessPublish
        (
            const   TMQTTInMsgPtr&          mptrSrc
        )   noexcept;

        tCIDLib::TVoid SendMsg
        (
                    TMQTTOutMsgPtr&         mptrSend
        )   noexcept;

        tCIDLib::TVoid SetState
        (
            const   tMQTTSh::EClStates      eToSet
        )   noexcept;

        tCIDLib::TVoid ShutdownConn() noexcept;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSecureConn
        //      This driven by configuration data. If true, we do a secure connection
        //      to the server.
        //
        //  m_c2NextPacketId
        //      We assign a new packet id to each outgoing msg that needs one.
        //
        //      WE DON'T ever use zero, we skip it. This can be used in our out
        //      msgs to know if a packet id ever got set.
        //
        //      This is synchronized by the mutex, since the main thread will call
        //      us to ask us to build msgs to send.
        //
        //  m_colEventQ
        //      The main thread queues up events for us to process here. We pull them
        //      out and process them as required. Most are msgs to send, but some are
        //      requests like reload config.
        //
        //      It's not thread safe since we have to manipulate the related queue
        //      queue, in the process. So our mutex is used to sync it.
        //
        //  m_enctNextKeepalive
        //      We handle keepalive pings internaly since that's not something the
        //      that's really MQTT logic and we know when each msgs is sent. So we
        //      just update this to the next time that a keep alive would need to
        //      be sent each time we send one.
        //
        //  m_enctLastPing
        //      The last time that we sent a ping. If zero, then we have gotten the
        //      reply. Else we can use it to see if the server is responding.
        //
        //  m_evptrCur
        //      A temp driver event used by ProcessEvents(), so that it doesn't
        //      have to continually get a one from the pool and release it. It just
        //      needs one to read and then process.
        //
        //      WE HAVE to release this in the Terminate otherwise the pool could have
        //      been destroyed before this guy goes away!
        //
        //  m_eCurState
        //      Our current state, which is always trying to get us up to ready,
        //      moving up through the states.
        //
        //  m_evEventQ
        //  m_evDataSrc
        //      Once we get to ready state we are always blocked on these two events.
        //      One we set on the data source and he will trigger it any time data is
        //      ready. The other is posted any time a msg is put into the output
        //      queue by an incoming call from the driver. They are manual mode.
        //      The data source manges his and we synchronize the output queue one
        //      via the m_mtxSync mutex to avoid race conditions between us and the
        //      facility object sending msgs to be sent.
        //
        //      We will read the event queue manually before we get to ready state, to
        //      watch for some requests we need to handle even then. We don't use the
        //      queue event in that case.
        //
        //  m_loglToUse
        //      A log limiter for our internal use, since we are constantly active.
        //
        //  m_mbufRead
        //      This is a temp to read the remaining length bytes of incoming msgs.
        //      We pass this to an in msg object (along with the header stuff we
        //      read out) to parse from. It's indirect via the m_strmReadBuf stream
        //      which we set up over this buffer and reuse.
        //
        //  m_mqcfgCurrent
        //      Our current configuration, so what we loaded from the configuration
        //      file. We won't continue forward until we successfully load some
        //      config.
        //
        //  m_mtxSync
        //      We have some synchronization requirements internally. We need to sync
        //      the out msg queue and related triggering event to avoid race conditions.
        //
        //  m_pcdsMQTT
        //      We use a data source so that we can support secure or non-secure
        //      clients. We have to use a pointer since we create one type or another
        //      based on config. The rest of the code just uses this base class ptr
        //      so we could change it to something else in the future as well. We use
        //      the stream-based data source base class, which lets us use either
        //      secure or non-secure socket based derivatives.
        //
        //  m_psdrvMQTT
        //      We need a pointer to the driver so that we can hand off I/O events
        //      to him.
        //
        //  m_strCfgFilePath
        //      We pre-build the path to the config file, so that if we don't load it
        //      first time, we don't have ot keep rebuildin git.
        //
        //  m_strTmpPL
        //      A temp string to use for decoding incoming publish payloads that our
        //      config tells us are in text form.
        //
        //  m_strmReadBuf
        //      This is set up over m_mbufRead and reused each time a msg is ready by
        //      just resettings the end of stream position. This is passed to in msgs
        //      to parse from. It is set to big endian of course.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bSecureConn;
        tCIDLib::TCard2             m_c2NextPacketId;
        TDrvEventQ                  m_colEventQ;
        tMQTTSh::EClStates          m_eCurState;
        TMQTTDrvEvPtr               m_evptrCur;
        tCIDLib::TEncodedTime       m_enctNextKeepalive;
        tCIDLib::TEncodedTime       m_enctLastPing;
        TEvent                      m_evEventQ;
        TEvent                      m_evDataSrc;
        TLogLimiter                 m_loglToUse;
        THeapBuf                    m_mbufRead;
        TMQTTCfg                    m_mqcfgCurrent;
        TMutex                      m_mtxSync;
        TCIDSockStreamBasedDataSrc* m_pcdsMQTT;
        TMQTTS*                     m_psdrvMQTT;
        TString                     m_strCfgFilePath;
        TString                     m_strTmpPL;
        TBinMBufInStream            m_strmReadBuf;
};
