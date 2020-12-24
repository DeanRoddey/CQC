//
// FILE NAME: MQTTS_IOThread.cpp
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Bring in our own public header
// ---------------------------------------------------------------------------
#include    "MQTTS_.hpp"


// ---------------------------------------------------------------------------
//  CLASS: TMQTTIOThread
// PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTIOThread: Constructors and Destructors.
// ---------------------------------------------------------------------------
TMQTTIOThread::TMQTTIOThread(TMQTTS* const psdrvMQTT) :

    TThread(TStrCat(L"MQTTSIOThread_", psdrvMQTT->strMoniker()))
    , m_c2NextPacketId(1)
    , m_colEventQ()
    , m_eCurState(tMQTTSh::EClStates::LoadConfig)
    , m_evEventQ(tCIDLib::EEventStates::Reset, kCIDLib::True)
    , m_evDataSrc(tCIDLib::EEventStates::Reset, kCIDLib::True)
    , m_mbufRead(kCIDLib::c4Sz_4K, kCIDLib::c4Sz_64K, kCIDLib::c4Sz_1K)
    , m_loglToUse(30000)
    , m_evptrCur(psdrvMQTT->spptrDrvEvent())
    , m_pcdsMQTT(nullptr)
    , m_psdrvMQTT(psdrvMQTT)
    , m_strmReadBuf(&m_mbufRead)
{
    // Set the stream to big endian for MQTT stuff
    m_strmReadBuf.eEndianMode(tCIDLib::EEndianModes::Big);
}

TMQTTIOThread::~TMQTTIOThread()
{
}


// ---------------------------------------------------------------------------
//  TCQCIRActionsThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once we report to the main driver that we have the configuration loaded and
//  set up, he calls us here to query all the field defs so that he can set up
//  his fields.
//
//  We will already by then be subscribing and such, but since the results are
//  queued up, he can get this done before he starts processing those incoming
//  field values.
//
//  We have to lock here of course to protext the config data.
//
//  Since this is async, it's possible that we will lose connection before he
//  calls us. So we return False if he's too late, else true if we fill in
//  the list.
//
tCIDLib::TBoolean TMQTTIOThread::bQueryFldDefs(tCQCKit::TFldDefList& colToFill)
{
    TLocker lockrSync(&m_mtxSync);
    if (m_eCurState != tMQTTSh::EClStates::Ready)
        return kCIDLib::False;

    colToFill.RemoveAll();
    TCQCFldDef flddCur;
    TMQTTCfg::TFldCursor cursFlds = m_mqcfgCurrent.cursFldList();
    for (; cursFlds; ++cursFlds)
    {
        cursFlds->QueryFldDef(flddCur);
        colToFill.objAdd(flddCur);
    }

    return kCIDLib::True;
}


//
//  The driver thread calls us here to ask to do things. We queue up the requests
//  for later processing. We trigger the queue event which is what the driver blocks
//  on when in ready state.
//
tCIDLib::TBoolean TMQTTIOThread::bQueueDrvEvent(TMQTTDrvEvPtr& evptrToQ)
{
    TLocker lockrSync(&m_mtxSync);

    if (m_colEventQ.bIsFull(64))
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread, L"I/O thread ev queue is full"
            );
        }
    }
     else
    {
        m_colEventQ.objAdd(evptrToQ);
        m_evEventQ.Trigger();
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCQCIRActionsThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TMQTTIOThread::eProcess()
{
    Sync();

    // Make sure we start in the right state and then enter the loop
    m_eCurState = tMQTTSh::EClStates::LoadConfig;
    while (!bCheckShutdownRequest() && (m_eCurState < tMQTTSh::EClStates::Disconnected))
    {
        // Based on the state we do this or that
        tCIDLib::TCard4 c4Sleep = 0;
        switch(m_eCurState)
        {
            case tMQTTSh::EClStates::LoadConfig :
                if (bLoadConfig())
                    SetState(tMQTTSh::EClStates::Initialize);
                else
                    c4Sleep = 30000;
                break;

            case tMQTTSh::EClStates::Initialize :
                if (bInitialize())
                    SetState(tMQTTSh::EClStates::WaitServer);
                else
                    c4Sleep = 30000;
                break;

            case tMQTTSh::EClStates::WaitServer :
                if (bConnectSocket())
                    SetState(tMQTTSh::EClStates::Connecting);
                else
                    c4Sleep = 8000;
                break;

            case tMQTTSh::EClStates::Connecting :
                if (bConnectMQTT())
                {
                    SetState(tMQTTSh::EClStates::Subscribing);
                }
                 else
                {
                    // Something went wrong, sleep a while and try again
                    SetState(tMQTTSh::EClStates::WaitServer);
                    c4Sleep = 15000;
                }
                break;

            case tMQTTSh::EClStates::Subscribing :
                if (bSubscribe())
                {
                    SetState(tMQTTSh::EClStates::Ready);
                }
                 else
                {
                    // Something went wrong, sleep a while and try again
                    SetState(tMQTTSh::EClStates::WaitServer);
                    c4Sleep = 30000;
                }
                break;

            case tMQTTSh::EClStates::Ready :
                ProcessDrvEvents();
                break;

            case tMQTTSh::EClStates::Disconnecting :
                Disconnect();
                break;

            case tMQTTSh::EClStates::Disconnected :
                // We will fall out of the loop now
                break;

            default :
                CIDAssert2(L"Unknown MQTT client state");
                break;
        };

        // If asked to sleep, do that, while watching for shutdown requests
        if (c4Sleep)
        {
            if (!bSleep(c4Sleep))
                break;
        }
    }

    // If we are connected try to disconnect
    if ((m_eCurState > tMQTTSh::EClStates::Connecting)
    &&  (m_eCurState < tMQTTSh::EClStates::Disconnecting))
    {
        Disconnect();
    }

    // Make sure we release any trailing event pointer
    m_evptrCur.Release();

    return tCIDLib::EExitCodes::Normal;
}




// ---------------------------------------------------------------------------
//  TMQTTIOThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TMQTTIOThread::bConnectMQTT() noexcept
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        TMQTTOutMsgPtr mptrConn = m_psdrvMQTT->spptrOutMsg(512);
        mptrConn->BuildConnectMsg
        (
            TStrCat(kMQTTS::strClientId, L"_", m_psdrvMQTT->strMoniker())
            , kCIDLib::True
            , kMQTTS::c2KeepAliveSecs
            , m_mqcfgCurrent.strUserName()
            , m_mqcfgCurrent.strPassword()
        );
        SendMsg(mptrConn);

        // And now wait for a connection ack reply
        TMQTTInMsgPtr mptrRet = mptrWaitReply(tCQCMQTT::EPacketTypes::ConnAck, 0, 3000);
        if (mptrRet)
            bRet = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        if (m_psdrvMQTT->eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
    }

    if (facMQTTSh().bTraceMode())
    {
        if (bRet)
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread, L"MQTT connection exchange completed"
            );
        }
         else
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"No connection act received, recycling connection"
            );
        }
    }
    return bRet;
}


tCIDLib::TBoolean TMQTTIOThread::bConnectSocket() noexcept
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // If the end point is bad, this will fail
        TIPEndPoint ipepTar
        (
            m_mqcfgCurrent.strMQTTAddr()
            , m_mqcfgCurrent.ippnMQTT()
            , tCIDSock::EAddrTypes::Unspec
        );

        //
        //  Based on config, we create a secure or non-secure socket based data
        //  source. He will create and own the socket.
        //
        TCIDSockStreamBasedDataSrc* pcdsNew = nullptr;
        if (m_mqcfgCurrent.bSecureConn())
        {
            // Set up the ALPN list and add their preferred protocol if one was set
            tCIDLib::TStrList colALPN;
            if (!m_mqcfgCurrent.strPrefProtocol().bIsEmpty())
                colALPN.objAdd(m_mqcfgCurrent.strPrefProtocol());

            pcdsNew = new TCIDSChanClDataSrc
            (
                L"MQTT Driver"
                , ipepTar
                , tCIDSock::ESockProtos::TCP
                , m_mqcfgCurrent.strCertInfo()
                , colALPN
                , tCIDSChan::EConnOpts::None
                , m_mqcfgCurrent.strSecPrincipal()
            );
        }
         else
        {
            pcdsNew = new TCIDSockStreamDataSrc(ipepTar);
        }

        //
        //  Put a janitor on it who will try to initialize it for us. If it goes
        //  south, he will clean it up for us.
        //
        TCIDDataSrcJan janSrc
        (
            pcdsNew, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True, TTime::enctNowPlusSecs(2)
        );

        //
        //  It didn't throw, so try to set the read event. If that works, orphan
        //  it out and keep it.
        //
        pcdsNew->AssociateReadEvent(m_evDataSrc);
        m_pcdsMQTT = pcdsNew;
        janSrc.Orphan();

        bRet = kCIDLib::True;
    }

    catch(TError& errToCatch)
    {
        if (m_psdrvMQTT->eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
    }

    if (facMQTTSh().bTraceMode())
    {
        if (bRet)
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread, L"Socket connection completed"
            );
        }
         else
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread, L"Socket connection failed"
            );
        }
    }
    return bRet;
}


// This is called to do any needed init after config is loaded
tCIDLib::TBoolean TMQTTIOThread::bInitialize() noexcept
{
    try
    {
    }

    catch(TError& errToCatch)
    {
        if (m_psdrvMQTT->eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We try to load the config. If it fails, then we stay in this state. If the
//  file doesn't exist, that is considered a failure since the config contains
//  the MQTT server and port.
//
tCIDLib::TBoolean TMQTTIOThread::bLoadConfig() noexcept
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (m_strCfgFilePath.bIsEmpty())
        {
            // Build up the path to the config file
            TPathStr pathCfg = facCQCKit().strServerDataDir();
            pathCfg.AddLevels(L"MQTT", m_psdrvMQTT->strMoniker());
            pathCfg.AppendExt(L"cfg");
            m_strCfgFilePath = pathCfg;
        }

        tCIDLib::TStrList colErrs;
        TMQTTCfg mqcfgTest;
        bRet = mqcfgTest.bParseFrom(m_strCfgFilePath, m_psdrvMQTT->strMoniker(), colErrs);
        if (bRet)
        {
            //
            //  The shared facility trace/verbosity settings in the config will have
            //  been pushed out during the parse above. But we also need to set the
            //  driver's verbosity.
            //
            if (mqcfgTest.bVerboseMode())
                m_psdrvMQTT->SetNewVerbosity(tCQCKit::EVerboseLvls::High);

            //
            //  Lock while we do this, since the main driver thread can call in
            //  to query config related info.
            //
            TLocker lockrSync(&m_mtxSync);
            m_mqcfgCurrent = mqcfgTest;

            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceMsg
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Config loaded. Connecting to %(1):%(2)"
                    , m_mqcfgCurrent.strMQTTAddr()
                    , TCardinal(m_mqcfgCurrent.ippnMQTT())
                );
            }
        }
         else
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Failed to load configuration file: %(1)"
                , colErrs[0]
            );

            if (m_psdrvMQTT->eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            {
                facMQTTS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kMQTTSErrs::errcCfg_LoadErr
                    , colErrs[0]
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (m_psdrvMQTT->eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);

        facMQTTSh().LogTraceMsg
        (
            tMQTTSh::EMsgSrcs::IOThread
            , L"Failed to load configuration file, see the logs"
        );
    }
    return bRet;
}


//
//  We do all of our subscriptions, based on config.
//
tCIDLib::TBoolean TMQTTIOThread::bSubscribe() noexcept
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (m_mqcfgCurrent.c4FldCount())
        {
            //
            //  Multiple fields can refer to the same topic, so we want to only
            //  subscribe to each one once. So we build up a list of unique topic
            //  paths.
            //
            tCIDLib::TStrHashSet colUTopics(23, TStringKeyOps(kCIDLib::False));
            {
                TMQTTCfg::TFldCursor cursFlds = m_mqcfgCurrent.cursFldList();
                for (; cursFlds; ++cursFlds)
                {
                    if (!colUTopics.bHasElement(cursFlds->strTopicPath()))
                        colUTopics.objAdd(cursFlds->strTopicPath());
                }
            }

            const tCQCMQTT::EQOSLevs eDefQOS = tCQCMQTT::EQOSLevs::Q1;
            tCQCMQTT::TTopicList colTopics;
            tCIDLib::TStrHashSet::TCursor cursUTopics(&colUTopics);
            while (cursUTopics && !bCheckShutdownRequest())
            {
                // We do up to a 1K'ish payload size per round
                tCIDLib::TCard4 c4Size = 0;
                colTopics.RemoveAll();
                while ((c4Size < kCIDLib::c4Sz_1K) && cursUTopics)
                {
                    colTopics.objAdd(TMQTTTopicInfo(*cursUTopics, eDefQOS));

                    //
                    //  Assume no probably multi-byte UTF-8 values plus 2 length bytes
                    //  and 1 for QOS.
                    //
                    c4Size += cursUTopics->c4Length() + 3;
                    ++cursUTopics;
                }

                //
                //  OK, let's build this one and send it, using the rough size
                //  we calculated plus some extra for header and such.
                //
                TMQTTOutMsgPtr  mptrOut = m_psdrvMQTT->spptrOutMsg(c4Size + 16);
                mptrOut->BuildSubscribeMsg(colTopics, c2GetNextPacketId());
                SendMsg(mptrOut);

                // And wait for the reply
                TMQTTInMsgPtr mptrRep = mptrWaitReply
                (
                    tCQCMQTT::EPacketTypes::SubAck, mptrOut->c2PacketId(), 2500
                );

                // If no reply, then either we failed or were asked to shut down
                if (!mptrRep)
                {
                    bRet = kCIDLib::False;
                    break;
                }

                //
                //  Let's process this one. We need to store the resulting codes
                //  on the field config objects. We have to find all of the fields
                //  that refer to each topic and initialize them with the returned
                //  code.
                //
                const tCIDLib::TCard4 c4TopicCnt = mptrOut->c4TopicCnt();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TopicCnt; c4Index++)
                {
                    m_mqcfgCurrent.InitFldsForTopic
                    (
                        mptrOut->strTopicPathAt(c4Index), mptrRep->c1CodeAt(c4Index)
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        bRet = kCIDLib::False;

        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Medium)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);

        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"An exception occurred during subsription: %(1)"
                , errToCatch.strErrText()
            );
        }
    }

    //
    //  If the above worked OK, then let's check for any that have OnConnect values
    //  and send those. If any of these fail we won't give up, we'll just log trace
    //  messages and the field will probably remain in error state.
    //
    if (bRet)
    {
        tCIDLib::TCard4 c4Bytes;
        THeapBuf mbufTmp(128, kCIDLib::c4Sz_4K);

        TMQTTCfg::TFldCursor cursFlds = m_mqcfgCurrent.cursFldList();
        for (; cursFlds; ++cursFlds)
        {
            if (cursFlds->bOnConnect())
            {
                if (cursFlds->bPackageOutVal(cursFlds->strOnConnect(), mbufTmp, c4Bytes))
                {
                    try
                    {
                        TMQTTOutMsgPtr mptrOut = m_psdrvMQTT->spptrOutMsg(c4Bytes + 16);
                        mptrOut->BuildPublishMsg
                        (
                            cursFlds->strTopicPath()
                            , tCQCMQTT::EQOSLevs::Q1
                            , kCIDLib::False
                            , c2GetNextPacketId()
                            , mbufTmp
                            , c4Bytes
                        );
                        mptrOut->SendOn(*m_pcdsMQTT);
                    }

                    catch(TError& errToCatch)
                    {
                        if (m_psdrvMQTT->eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        if (facMQTTSh().bTraceMode())
                        {
                            facMQTTSh().LogTraceErr
                            (
                                tMQTTSh::EMsgSrcs::IOThread
                                , L"Exception sending OnConnect value for field %(1)"
                                , cursFlds->strFldName()
                            );
                        }
                    }
                }
                 else
                {
                    if (facMQTTSh().bTraceMode())
                    {
                        facMQTTSh().LogTraceErr
                        (
                            tMQTTSh::EMsgSrcs::IOThread
                            , L"Failed to send OnConnect value for field %(1)"
                            , cursFlds->strFldName()
                        );
                    }
                }
            }
        }
    }

    if (!bRet && facMQTTSh().bTraceMode())
    {
        facMQTTSh().LogTraceErr
        (
            tMQTTSh::EMsgSrcs::IOThread, L"The subscription process failed"
        );
    }
    return bRet;
}



//
//  Return the next available packet id, and moving past zero if needed. We have
//  to sync this since we have to build packets in response to incoming calls from
//  ORB threads.
//
tCIDLib::TCard2 TMQTTIOThread::c2GetNextPacketId() noexcept
{
    TLocker lockrSync(&m_mtxSync);
    tCIDLib::TCard2 c2Ret = m_c2NextPacketId++;
    if (!m_c2NextPacketId)
        m_c2NextPacketId++;
    return c2Ret;
}




//
//  The worker thread calls this periodically when in Ready state. If we have
//  gone past the keepalive point, we send a ping
//
tCIDLib::TVoid TMQTTIOThread::CheckKeepalive() noexcept
{
    if (m_enctNextKeepalive < TTime::enctNow())
    {
        try
        {
            // This is just a small one, so minimmal size
            TMQTTOutMsgPtr mptrSend = m_psdrvMQTT->spptrOutMsg(8);
            mptrSend->BuildPingReqMsg();
            SendMsg(mptrSend);
        }

        catch(TError& errToCatch)
        {
            if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Low)
                m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
        }
    }
}


// If we are connected, this will do a disconnect and data source shutdown
tCIDLib::TVoid TMQTTIOThread::Disconnect() noexcept
{
    if (m_pcdsMQTT && m_pcdsMQTT->bIsConnected())
        ShutdownConn();
    SetState(tMQTTSh::EClStates::Disconnected);
}


// To avoid redundancy, since we do a lot of catching of exceptions here
tCIDLib::TVoid
TMQTTIOThread::LogSysExcept(const tCIDLib::TCh* const pszIn, const tCIDLib::TCard4 c4Line)
{
    facMQTTSh().LogMsg
    (
        CID_FILE
        , c4Line
        , kMQTTShErrs::errcGen_SysExcept
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , TString(pszIn)
    );
}


//
//  The actual low level msg reading. It is assumed that the caller knows that
//  data is ready before calling this, else this guy is going to waste time
//  because it will block for a while waiting for data from the stream socket
//  and it will recycle the connection if the read actually fails.
//
TMQTTInMsgPtr TMQTTIOThread::mptrReadMsg() noexcept
{
    //
    //  If we fail, we return this unset msg pointer. If it works we set this guy
    //  to the new msg.
    //
    TMQTTInMsgPtr mptrRet = m_psdrvMQTT->spptrNullInMsg();
    try
    {
        const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(250);

        //
        //  Use a msg helper to get the header/rem length info. A msg can parse itself,
        //  but this let's us know what the remaining length is so we can get a msg
        //  from our in pool based on that, and then let him just parse the rest of
        //  it.
        //
        tCIDLib::TCard1 c1Flags;
        tCIDLib::TCard4 c4RemLen;
        tCQCMQTT::EPacketTypes eType;
        TMQTTMsg::ReadHeaderLen(*m_pcdsMQTT, eType, c1Flags, c4RemLen, enctEnd);

        // If there's any more data, then load it to a stream
        if (c4RemLen)
        {
            m_pcdsMQTT->c4ReadBytes(m_mbufRead, c4RemLen, enctEnd, kCIDLib::True);
            m_strmReadBuf.Reset();
            m_strmReadBuf.SetEndIndex(c4RemLen);
        }

        //
        //  We can get a msg from the in msg pool for this remaining length. Not all
        //  of that will usually end up being part of the payload buffer that this is
        //  sizing, but this insures it will hold the worst case.
        //
        TMQTTInMsgPtr mptrParse = m_psdrvMQTT->spptrInMsg(c4RemLen);
        mptrParse->ParseFrom(eType, c1Flags, c4RemLen, m_strmReadBuf);

        if (facMQTTSh().bTraceMode())
            facMQTTSh().LogInMsg(*mptrParse);

        // It's ok, so assign to the return pointer
        mptrRet = mptrParse;
    }

    catch(TError& errToCatch)
    {
        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::High)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);

        // Fall through
    }

    catch(...)
    {
        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::High)
            LogSysExcept(L"msg reading", CID_LINE);
        // Fall through
    }

    if (!mptrRet)
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread, L"Read failed, recycling connection"
            );
        }
        SetState(tMQTTSh::EClStates::WaitServer);
    }
    return mptrRet;
}



//
//  We have to process msgs recieved while waiting for responses. So we us this
//  method. It will wait for up to a given time for the indicated msg to arrive,
//  processing other msgs until then.
//
//  The packet id is only compared if the incoming msg contains one. If you just
//  want to wait a while and process anything that comes in, then pass ::Count
//  for the type.
//
//  The msg is processed whether it's the one being waited on or not, though
//  typically msgs that are waited for are just acks and nothing will be done
//  in response.
//
TMQTTInMsgPtr
TMQTTIOThread::mptrWaitReply(const  tCQCMQTT::EPacketTypes  eWaitType
                            , const tCIDLib::TCard2         c2WaitId
                            , const tCIDLib::TCard4         c4WaitMSs)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitMSs * kCIDLib::enctOneMilliSec);

    TMQTTInMsgPtr mptrRet = m_psdrvMQTT->spptrNullInMsg();
    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (!bRet && (enctCur < enctEnd) && !bCheckShutdownRequest())
    {
        // Wait for the remaining time for data
        if (m_pcdsMQTT->bDataAvail(enctEnd))
        {
            // It if fails give up with the failure state still set
            mptrRet = mptrReadMsg();
            if (!mptrRet)
                break;

            bRet = mptrRet->bIsThisMsg(eWaitType, c2WaitId);
            ProcessMsg(mptrRet);
        }
        enctCur = TTime::enctNow();
    }
    return mptrRet;
}


//
//  When we get a new msg, either on a call to ProcessMsgs, or when waiting
//  for some reply, this is called to handle it.
//
tCIDLib::TVoid TMQTTIOThread::ProcessMsg(const TMQTTInMsgPtr& mptrNew) noexcept
{
    switch(mptrNew->ePacketType())
    {
        case tCQCMQTT::EPacketTypes::Publish :
            ProcessPublish(mptrNew);
            break;

        case tCQCMQTT::EPacketTypes::PubRel :
        {
            TMQTTOutMsgPtr mptrOut = m_psdrvMQTT->spptrOutMsg(8);
            mptrOut->BuildPubCompMsg(mptrNew->c2PacketId());
            SendMsg(mptrOut);
            break;
        }

        default :
            // Is this bad?
            break;
    };
}


//
//  When the ProcessDrvEvents() method below gets an event from the event queue, we
//  are called to process it. There could be a number of these eventually, so we
//  need to break it out.
//
tCIDLib::TVoid TMQTTIOThread::ProcessDrvEvent(TMQTTDrvEvent& drvevNew)
{
    if (drvevNew.m_eEvent == tMQTTS::EDrvEvents::FieldWrite)
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Driver thread sent field write for %(1)"
                , drvevNew.m_strFldName
            );
        }

        //
        //  We need to build up a publish msg for this field. So let's try to find
        //  the owner.
        //
        const TMQTTFldCfg* pmqfldcTar = m_mqcfgCurrent.pmqfldcFindFld(drvevNew.m_strFldName);
        if (pmqfldcTar)
        {
            // Map the fld value to an outgoing MQTT payload value
            tCIDLib::TCard4 c4Bytes;
            THeapBuf mbufTmp(128, kCIDLib::c4Sz_4K);
            if (!pmqfldcTar->bMapOutVal(drvevNew.m_strValue, mbufTmp, c4Bytes))
            {
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceErr
                    (
                        tMQTTSh::EMsgSrcs::IOThread
                        , L"Failed to map new value for '%(1)' to MQTT value"
                        , drvevNew.m_strFldName
                    );
                }

                // Send back a write failure event
                TMQTTIOEvPtr mptrFailed = m_psdrvMQTT->spptrIOEvent();
                mptrFailed->SetFldWriteRes(*pmqfldcTar, tCQCKit::ECommResults::ValueRejected);
                m_psdrvMQTT->QueueIOEvent(mptrFailed);
            }
             else
            {
                // Let's build up an outgoing msg for this guy and send it
                TMQTTOutMsgPtr mptrSend = m_psdrvMQTT->spptrOutMsg(c4Bytes);
                mptrSend->BuildPublishMsg
                (
                    pmqfldcTar->strTopicPath()
                    , tCQCMQTT::EQOSLevs::Q1
                    , pmqfldcTar->bRetainWrite()
                    , c2GetNextPacketId()
                    , mbufTmp
                    , c4Bytes
                );
                SendMsg(mptrSend);

                // It worked so send back a write done event
                TMQTTIOEvPtr mptrSuccess = m_psdrvMQTT->spptrIOEvent();
                mptrSuccess->SetFldWriteRes(*pmqfldcTar, tCQCKit::ECommResults::Success);
                m_psdrvMQTT->QueueIOEvent(mptrSuccess);
            }
        }
         else
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Could not find field '%(1)' in the topic list"
                    , drvevNew.m_strFldName
                );
            }

            // Send back a write failure event
            TMQTTIOEvPtr mptrFailed = m_psdrvMQTT->spptrIOEvent();
            mptrFailed->SetFldWriteRes(*pmqfldcTar, tCQCKit::ECommResults::FieldNotFound);
            m_psdrvMQTT->QueueIOEvent(mptrFailed);
        }
    }
     else if (drvevNew.m_eEvent == tMQTTS::EDrvEvents::ReloadCfg)
    {
        //
        //  If we are in a state that this makes sense, then let's do it. For now
        //  we do the cheap way and just force a reset of everything.
        //
        if ((m_eCurState > tMQTTSh::EClStates::LoadConfig)
        &&  (m_eCurState <= tMQTTSh::EClStates::Ready))
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceMsg
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Driver thread requested config reload"
                );
            }
            SetState(tMQTTSh::EClStates::LoadConfig);
        }
         else
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceMsg
                (
                    tMQTTSh::EMsgSrcs::IOThread
                    , L"Not in a state where config reload can be done"
                );
            }
        }
    }
}


//
//  Once we get to ready state, there are no proactive operations to deal with,
//  so we are just watching for commands from the main driver or incoming msgs from
//  the MQTT server. So we blcok on two events to wait for either to occur.
//
//  We'll block on the events for a little. If nothing, we just go back so as to
//  check for shutdown requests and will then come around again.
//
tCIDLib::TVoid TMQTTIOThread::ProcessDrvEvents() noexcept
{
    //
    //  We are connected, so we just wait for either input or output
    //  to process.
    //
    static const tCIDLib::TCard4 c4EvSend = 0;
    static const tCIDLib::TCard4 c4EvRec = 1;
    tCIDLib::TCard4 c4Which = kCIDLib::c4MaxCard;
    try
    {
        c4Which = TEvent::c4WaitMultiple(m_evEventQ, m_evDataSrc, 1000);

        if (c4Which == c4EvSend)
        {
            //
            //  Lock and get the event out. If the collection ends up empty, reset the
            //  event.
            //
            {
                TLocker lockrSync(&m_mtxSync);
                m_colEventQ.bGetNext(m_evptrCur, 0, kCIDLib::True);
                if (m_colEventQ.bIsEmpty())
                    m_evEventQ.Reset();
            }

            // Based on the type of event, let's process it.
            ProcessDrvEvent(*m_evptrCur);
        }
         else if (c4Which == c4EvRec)
        {
            //
            //  Reset the socket event, and get the flags If it indicates a close,
            //  then we are hosed, so we need to reset the connection.
            //
            const tCIDSock::ESockEvs eEvents = m_pcdsMQTT->eEnumEvents(m_evDataSrc);
            if (tCIDLib::bAllBitsOn(eEvents, tCIDSock::ESockEvs::Close))
            {
                SetState(tMQTTSh::EClStates::WaitServer);
            }
            else if (tCIDLib::bAllBitsOn(eEvents, tCIDSock::ESockEvs::Read))
            {
                //
                //  There's data available, so let's read a msg. We have to do this because
                //  this isn't a raw socket, it's a data source, which buffers. So msgs
                //  could get stuck unread in its buffers.
                //
                while (m_pcdsMQTT->bDataAvailMS(1))
                {
                    TMQTTInMsgPtr mptrNew = mptrReadMsg();
                    if (mptrNew)
                        ProcessMsg(mptrNew);
                }
            }
        }
         else
        {
            // If nothing to do, check if we need to send a keepalive ping
            CheckKeepalive();
        }
    }

    catch(TError& errToCatch)
    {
        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::High)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
    }
}


//
//  When we get a publish msg from the server, this is called to  process it.
//
//  If we get a publish msg, we need to ack it. We never ask for more than
//  level 1 QOS, so we should only have to do single level acks at most. But
//  we deal with Q2 as well (though we don't actually confirm that we have such a
//  publish outstanding, we just ack the PubRel msg with a PubComp.)
//
tCIDLib::TVoid TMQTTIOThread::ProcessPublish(const TMQTTInMsgPtr& mptrSrc) noexcept
{
    tCQCMQTT::EQOSLevs eQOS = mptrSrc->eExtractPubQOS();
    if (eQOS == tCQCMQTT::EQOSLevs::Q1)
    {
        TMQTTOutMsgPtr mptrOut = m_psdrvMQTT->spptrOutMsg(8);
        mptrOut->BuildPubAckMsg(mptrSrc->c2PacketId());
        SendMsg(mptrOut);
    }
      else if (eQOS == tCQCMQTT::EQOSLevs::Q2)
    {
        TMQTTOutMsgPtr mptrOut = m_psdrvMQTT->spptrOutMsg(8);
        mptrOut->BuildPubRecMsg(mptrSrc->c2PacketId());
        SendMsg(mptrOut);
    }

    // We ignore empty payloads
    if (!mptrSrc->c4PLBytes())
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Ignoring empty payload for topic '%(1)'"
                , mptrSrc->strTopicPathAt(0)
            );
        }
        return;
    }

    //
    //  We need to find any fields with the reported topic path and let them grab
    //  the data out of the payload so that we can pass it on to the main driver
    //  thread.
    //
    tCIDLib::TCard4 c4Matches = 0;
    TMQTTCfg::TFldCursor cursFlds = m_mqcfgCurrent.cursFldList();
    const TString& strSrcTopic = mptrSrc->strTopicPathAt(0);
    for (; cursFlds; ++cursFlds)
    {
        if (cursFlds->bMatchesInTopic(strSrcTopic))
        {
            c4Matches++;

            // If this field is write only, ignore any incomng values
            if (!cursFlds->bReadable())
            {
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceErr
                    (
                        tMQTTSh::EMsgSrcs::IOThread
                        , L"Ignoring new value for write-only field %(1)"
                        , cursFlds->strFldName()
                    );
                }
            }
            else if (cursFlds->bRejected())
            {
                //
                //  This field was rejected by the server, or the sub was never acknowledged.
                //  so this can't be right.
                //
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceErr
                    (
                        tMQTTSh::EMsgSrcs::IOThread
                        , L"Got new value for rejected topic: %(1)"
                        , strSrcTopic
                    );
                }
            }
            else
            {
                const TString& strFldName = cursFlds->strFldName();
                TString strVal;
                const tMQTTSh::EInMapRes eRes = cursFlds->eMapInVal
                (
                    mptrSrc->mbufPayload(), mptrSrc->c4PLBytes(), strVal
                );

                if (eRes == tMQTTSh::EInMapRes::GoodVal)
                {
                    TMQTTIOEvPtr evptrNew = m_psdrvMQTT->spptrIOEvent();;
                    evptrNew->SetNewValEvent(*cursFlds, strVal);
                    m_psdrvMQTT->QueueIOEvent(evptrNew);
                }
                else if ((eRes == tMQTTSh::EInMapRes::NotFound)
                     ||  (eRes == tMQTTSh::EInMapRes::BadVal))
                {
                    TMQTTIOEvPtr evptrNew = m_psdrvMQTT->spptrIOEvent();;
                    evptrNew->SetBadValEvent(*cursFlds);
                    m_psdrvMQTT->QueueIOEvent(evptrNew);
                }
				 else if (eRes == tMQTTSh::EInMapRes::Ignore)
				{
					// Just pretend it didn't happen
				}
                 else
                {
                    CIDAssert2(L"Unknown input map result");
                }
            }
        }
    }

    if (c4Matches)
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"%(1) fields matched topic: %(2)"
                , TCardinal(c4Matches)
                , strSrcTopic
            );
        }
    }
     else if (facMQTTSh().bTraceMode())
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Got publish for unknown topic: %(1)"
                , strSrcTopic
            );
        }
    }
}


//
//  This one takes a msg already gotten and sends it. This is so we can do things
//  like immediately send pings when needed.
//
//  NOTE that the msg can be modified, that the sent flag is set if the send is
//  successful.
//
tCIDLib::TVoid TMQTTIOThread::SendMsg(TMQTTOutMsgPtr& mptrSend) noexcept
{
    tCIDLib::TBoolean bSendOK = kCIDLib::False;
    try
    {
        mptrSend->SendOn(*m_pcdsMQTT);
        bSendOK = kCIDLib::True;

        if (facMQTTSh().bTraceMode())
            facMQTTSh().LogOutMsg(*mptrSend);

        // It worked so set the next keep alive stamp
        m_enctNextKeepalive = TTime::enctNowPlusSecs(kMQTTS::c2KeepAliveThresh);
    }

    catch(TError& errToCatch)
    {
        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Medium)
            m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
    }

    catch(...)
    {
        if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Medium)
            LogSysExcept(L"msg transmission", CID_LINE);
    }

    if (!bSendOK && facMQTTSh().bTraceMode())
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceErr
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Failed to send a %(1) message"
                , tCQCMQTT::strXlatEPacketTypes(mptrSend->ePacketType())
            );
        }
    }
}


// All changes to the state are done through this guy
tCIDLib::TVoid TMQTTIOThread::SetState(const tMQTTSh::EClStates eToSet) noexcept
{
    // Shouldn't generally happen, but just in case
    if (eToSet == m_eCurState)
        return;

    if (facMQTTSh().bTraceMode())
    {
        if (eToSet != m_eCurState)
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::IOThread
                , L"Transitioning from %(1) state to %(2) state"
                , tMQTTSh::strLoadEClStates(m_eCurState)
                , tMQTTSh::strLoadEClStates(eToSet)
            );
        }
    }

    //
    //  If we were beyond connect server and are now going to or before it,
    //  we need to shut down the data source.
    //
    if ((m_eCurState > tMQTTSh::EClStates::WaitServer)
    &&  (eToSet <= tMQTTSh::EClStates::WaitServer))
    {
        ShutdownConn();
    }

    //
    //  If moving in our out of ready state, flush the event queue and reset
    //  the associated event.
    //
    if ((m_eCurState == tMQTTSh::EClStates::Ready)
                                != (eToSet == tMQTTSh::EClStates::Ready))
    {
        TLocker lockrSync(&m_mtxSync);
        m_colEventQ.RemoveAll();
        m_evEventQ.Reset();
    }

    // And at the last set the new state
    m_eCurState = eToSet;

    // And post to the main thread that we changed states
    TMQTTIOEvPtr ioevNew = m_psdrvMQTT->spptrIOEvent();
    ioevNew->m_eEvent = tMQTTS::EIOEvents::StateChange;
    ioevNew->m_eIOState = eToSet;
    m_psdrvMQTT->QueueIOEvent(ioevNew);
}


//
//  This handles closing and cleaning up the data source, in as polite as way/
//  as is possible given the circumstances.
//
tCIDLib::TVoid TMQTTIOThread::ShutdownConn() noexcept
{
    if (m_pcdsMQTT && m_pcdsMQTT->bIsConnected())
    {
        if (facMQTTSh().bTraceMode())
            facMQTTSh().LogTraceMsg(tMQTTSh::EMsgSrcs::IOThread, L"Shutting down connection");

        // Send a disconnect
        try
        {
            TMQTTOutMsgPtr mptrOut = m_psdrvMQTT->spptrOutMsg(8);
            mptrOut->BuildDisconnectMsg();
            SendMsg(mptrOut);
        }

        catch(TError& errToCatch)
        {
            if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Low)
                m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
        }

        // And terminate the data source
        try
        {
            m_pcdsMQTT->Terminate(TTime::enctNowPlusSecs(2), kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Low)
                m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
        }
    }

    if (m_pcdsMQTT)
    {
        try
        {
            delete m_pcdsMQTT;
        }

        catch(TError& errToCatch)
        {
            if (facMQTTSh().eVerbosity() >= tCQCKit::EVerboseLvls::Low)
                m_loglToUse.bLogErr(errToCatch, CID_FILE, CID_LINE);
        }
        m_pcdsMQTT = nullptr;
    }
}
