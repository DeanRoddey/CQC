//
// FILE NAME: CQCWebSrvC_WebSockHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This file implements the websocket oriented parts of the worker thread class.
//  The main file handles regular HTTP stuff, but also checks for websocket upgrade
//  requests. If it gets one, it makes sure it is basically sound, i.e. correct URL
//  type and the upgrade type is websocket. If so, it calls us here,
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"


// Some helper macros for optional logging
#define LogStateInfo(msg) facCQCWebSrvC().LogMsg \
( \
    CID_FILE \
    , CID_LINE \
    , TString(L"Websock Trace: ") + msg \
    , tCIDLib::ESeverities::Status \
    , tCIDLib::EErrClasses::AppStatus \
)
#define LogStateInfo2(msg,p1) facCQCWebSrvC().LogMsg \
( \
    CID_FILE \
    , CID_LINE \
    , TString(L"Websock Trace: ") + msg \
    , tCIDLib::ESeverities::Status \
    , tCIDLib::EErrClasses::AppStatus \
    , p1 \
)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCWebSrvC_WebSockHandler
{
    namespace
    {
        //
        //  The time we'll wait before giving up if we don't see a ping from the other
        //  side, or some other activity.
        //
        constexpr tCIDLib::TEncodedTime enctActivityTO(kCIDLib::enctOneMinute * 2);

        //
        //  A buffer we allocate to read in data in smaller chunks, which we will then
        //  copy over to the caller's buffer.
        //
        constexpr tCIDLib::TCard4 c4ReadBufSz = 0x10000;
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TWebSockBuf
//  PREFIX: wsb
// ---------------------------------------------------------------------------
TWebSockBuf::TWebSockBuf() :

    m_c1Type(0xFF)
    , m_c4CurBytes(0)
    , m_pmbufData(nullptr)
{
}

TWebSockBuf::TWebSockBuf(const tCIDLib::TCard4 c4InitSz) :

    m_c1Type(0xFF)
    , m_c4CurBytes(c4InitSz)
    , m_pmbufData(new THeapBuf(c4InitSz, kCQCWebSrvC::c4MaxWebsockMsgSz))
{
}

TWebSockBuf::TWebSockBuf(TMemBuf* const pmbufToAdopt, const tCIDLib::TCard4 c4Size) :

    m_c1Type(0xFF)
    , m_c4CurBytes(c4Size)
    , m_pmbufData(pmbufToAdopt)
{
}

TWebSockBuf::~TWebSockBuf()
{
    try
    {
        delete m_pmbufData;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// Just reset the current bytes to zero
tCIDLib::TVoid TWebSockBuf::Reset()
{
    m_c1Type = 0xFF;
    m_c4CurBytes = 0;
}




// ---------------------------------------------------------------------------
//   CLASS: TWebsockThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWebsockThread: Public, static methods
// ---------------------------------------------------------------------------

// Indicate whether the msg type is a control type message
tCIDLib::TBoolean TWebsockThread::bControlType(const tCIDLib::TCard1 c1Type)
{
    return
    (
        (c1Type == kCQCWebSrvC::c1WSockMsg_Close)
        || (c1Type == kCQCWebSrvC::c1WSockMsg_Ping)
        || (c1Type == kCQCWebSrvC::c1WSockMsg_Pong)
    );
}


//
//  This method will do basic validation on a Websockets connection request, making
//  sure that required header lines are there and have the correct values. If not,
//  it will send back an error reply and return false. If valid, we return the
//  security key header line, which will be needed later.
//
tCIDLib::TBoolean
TWebsockThread::bValidateReqInfo(       TCIDDataSrc&        cdsData
                                , const TString&            strResource
                                , const tCIDLib::TKVPList&  colHdrLines
                                , const TString&            strReqType
                                ,       TString&            strSecKey)
{
    // If not a GET or it starts with the wrong prefix, then reject it
    if ((strReqType != kCIDNet::pszHTTP_GET)
    ||  !strResource.bStartsWithI(L"/Websock"))

    {
        THTTPClient::SendErrReply
        (
            cdsData
            , L"CQC Web Server"
            , kCIDNet::c4HTTPStatus_BadRequest
            , L"Bad Request"
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_WebsockNotGET)
        );
        return kCIDLib::False;
    }

    // Check for required header lines. Sec key goes into the caller's parm
    TString strConn;
    TString strUpgrade;
    if (!TNetCoreParser::bFindHdrLine(colHdrLines, THTTPClient::strHdr_Upgrade, strUpgrade)
    ||  !TNetCoreParser::bFindHdrLine(colHdrLines, THTTPClient::strHdr_Connection, strConn)
    ||  !TNetCoreParser::bFindHdrLine(colHdrLines, THTTPClient::strHdr_SecWSKey, strSecKey))

    {
        THTTPClient::SendErrReply
        (
            cdsData
            , L"CQC Web Server"
            , kCIDNet::c4HTTPStatus_BadRequest
            , L"Bad Request"
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_WSockUpgradeReq)
        );
        return kCIDLib::False;
    }

    // One of the tokens in the connection string must be upgrade
    tCIDLib::TBoolean bUpgradeType = kCIDLib::False;
    {
        TStringTokenizer stokConn(&strConn, L", ");
        TString strTok;
        while (stokConn.bGetNextToken(strTok) && !bUpgradeType)
            bUpgradeType = (strTok.eCompareI(L"upgrade") == tCIDLib::ESortComps::Equal);
    }

    // And now check that result and the upgrade type indicator
    if (!bUpgradeType || !strUpgrade.bCompareI(L"websocket"))
    {
        THTTPClient::SendErrReply
        (
            cdsData
            , L"CQC Web Server"
            , kCIDNet::c4HTTPStatus_BadRequest
            , L"Bad Request"
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_WSockUpgradeVals)
        );
        return kCIDLib::False;
    }

    // It has to be either a (possibly) valid user URL or our WebRIVA client
    if (!strResource.bStartsWithI(L"/Websock/User/")
    &&  !strResource.bCompareI(kCQCWebRIVA::strURL_WebWSRIVA))
    {
        THTTPClient::SendErrReply
        (
            cdsData
            , L"CQC Web Server"
            , kCIDNet::c4HTTPStatus_BadRequest
            , L"Bad Request"
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_BadWebsockURL, strResource)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TWebsockThread: Destructor
// ---------------------------------------------------------------------------
TWebsockThread::~TWebsockThread()
{
}


// ---------------------------------------------------------------------------
//  TWebsockThread: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TWebsockThread::bFindQParam(const TString& strKey, TString& strValue) const
{
    return TNetCoreParser::bFindHdrLine(m_colQParams, strKey, strValue);
}


// Return whether we are ready or not
tCIDLib::TBoolean TWebsockThread::bIsReady() const
{
    return (m_eState > EStates::Connecting) && (m_eState < EStates::WaitClientEnd);
}


// Let others know if we are in simulator mode or not
tCIDLib::TBoolean TWebsockThread::bSimMode() const
{
    return m_bSimMode;
}


//
//  A helper to allow web socket handlers to easily support client validation. If
//  they are using a secure connection, then they can just pass over a user name
//  and password and the handler can validate it using this method.
//
tCIDLib::TBoolean
TWebsockThread::bValidateLogin( const   TString&            strUser
                                , const TString&            strPassword
                                ,       TCQCSecToken&       sectToFill
                                ,       TCQCUserAccount&    uaccToFill
                                ,       TString&            strFailReason)
{
    // Assume failure until proven otherwise
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        // Get a reference to the security server
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

        // And ask it to give us a challenge
        TCQCSecChallenge seccLogon;
        if (orbcSS->bLoginReq(strUser, seccLogon))
        {
            // Lets do a hash of the user's password
            TMD5Hash mhashPW;
            TMessageDigest5 mdigTmp;
            mdigTmp.StartNew();
            mdigTmp.DigestStr(strPassword);
            mdigTmp.Complete(mhashPW);

            // And use that to validate the challenge
            seccLogon.Validate(mhashPW);

            // And send that back to get a security token, assuming its legal
            tCQCKit::ELoginRes eRes;
            bRes = orbcSS->bGetSToken(seccLogon, sectToFill, uaccToFill, eRes);

            if (!bRes)
            {
                strFailReason = L"Websocket client login failed. Reason=";
                strFailReason.Append(tCQCKit::strXlatELoginRes(eRes));
            }
        }
         else
        {
            strFailReason = L"Invalid login credentials supplied by Websocket client";
            strFailReason.Append(strUser);
        }
    }

    catch(TError& errToCatch)
    {
        strFailReason = L"Exception in Websocket client login. Error=";
        strFailReason.Append(errToCatch.strErrText());

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // Fall through with false status
    }

    return bRes;
}


tCIDLib::TCard4 TWebsockThread::c4QParamCnt() const
{
    return m_colQParams.c4ElemCount();
}


//
//  This method implements the server side support for a websockets session. When
//  running normally within the web server, this is called from eProcess() below, i.e.
//  on the worker thread that this class represents. But it can also be called from
//  the CML IDE when debugging a CML web sockets handler. In that case, this object
//  is not started as a thread, but this is just called in the context of the CML
//  IDE thread, and we cause callbacks into the CML class in the normal sort of way,
//  via the derived CML handler class's overrides.
//
tCIDLib::EExitCodes TWebsockThread::eServiceClient()
{
    //
    //  Let the derived class intitialize. If it fails, then we just send back a
    //  failure and are done.
    //
    {
        tCIDLib::TCard4 c4ResCode = 0;
        TString         strRepText;
        TString         strErrText;
        try
        {
            c4ResCode = c4WSInitialize(m_urlReq, strRepText, strErrText);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // Fall through after setting error info
            c4ResCode = kCIDNet::c4HTTPStatus_SrvError;
            strRepText = L"Initialization error";
            strErrText = errToCatch.strErrText();
        }

        catch(...)
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_SysExcept
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::InitFailed
                , TString(L"initializing connection")
            );

            // Fall through after setting error info
            c4ResCode = kCIDNet::c4HTTPStatus_SrvError;
            strRepText = L"Initialization error";
            strErrText = L"Unknown exception occurred";
        }

        // If it's not OK, then send an error reply
        if (c4ResCode != kCIDNet::c4HTTPStatus_OK)
        {
            THTTPClient::SendErrReply
            (
                *m_pcdsServer
                , L"CQC Web Server"
                , c4ResCode
                , strRepText
                , strErrText
            );
            return tCIDLib::EExitCodes::RuntimeError;
        }
    }

    // We got this far, so send the acceptance of the upgrade
    try
    {
        SendAccept();
    }

    catch(TError& errToCatch)
    {
        if (m_bLogStateInfo)
            LogStateInfo(L"Acceptance msg failed");

        //
        //  Just give up, don't try to send anything since we obviously just failed
        //  to send something.
        //
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        if (m_bLogStateInfo)
            LogStateInfo(L"Acceptance msg failed");

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWRIVA_SysExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
            , TString(L"sending connection acceptance")
        );
        return tCIDLib::EExitCodes::RuntimeError;
    }


    // Looks like we are going to do it, so log it if verbose
    if (facCQCWebSrvC().bLogInfo())
    {
        TString strInfo;
        m_pcdsServer->FormatSrcInfo(strInfo);
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midDbg_StartWebsock
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strInfo
        );
    }

    //
    //  And now we are ready to enter our main loop. Here we wait for new msgs
    //  from the client, which we pass on to the derived class. And we want for msgs
    //  from the derived class to show up in the output queue to send.
    //
    //  We also watch for changes in fields that the caller has asked to keep track
    //  of. If we get any, we call him with those changes.
    //
    //  NOTE THAT after this point we need to be sure that WSTerminate gets called.
    //
    tCIDLib::TBoolean bCalledConn = kCIDLib::False;
    try
    {
        //
        //  Call the connected method to let the derived class set up and go to ready state
        //  if that doesn't throw.
        //
        //  NOTE THAT after this point we need to insure that Disconnected() gets called,
        //  so we set a flag indicating this. We can't use the state because it doesn't
        //  necessarily indicate (down at the end) that we ever called this.
        //
        if (m_bLogStateInfo)
            LogStateInfo(L"Connect");
        Connected();
        bCalledConn = kCIDLib::True;
        m_eState = EStates::Ready;

        //
        //  We accumulate msgs in frames. So we need to track how much we gotten so
        //  far and the original msg type.
        //
        tCIDLib::TBoolean   bFinal;
        tCIDLib::TCard1     c1MsgType = 0xFF;
        tCIDLib::TCard4     c4MsgLen = 0;

        //
        //  Initialize some timers we use to decide it's time to do things. Do the
        //  initial fld poll time a little longer than we subsequently do. To give
        //  things time to settle down before we start checking.
        //
        m_enctLastInMsg = TTime::enctNow();
        m_enctLastOutMsg = m_enctLastInMsg;
        tCIDLib::TEncodedTime   enctNextFldPoll
        (
            m_enctLastInMsg + (kCIDLib::enctOneSecond * 3)
        );

        //
        //  Now we can loop until it's time to close down.
        //
        m_enctEndTimer = 0;
        while(m_eState != EStates::End)
        {
            //
            //  If not in sim mode check for a shutdown. We can't do it in sim mode because
            //  we aren't running as a separate thread and so we were never started up as a
            //  thread, and this would cause an error. So in that scenario, if not in idle
            //  mode, we just call the idle callback.
            //
            //  In sim mode we call the CheckShutdownReq() method, which lets the derived
            //  class watch for shutdown requests. If so, they will thrown a debug exit
            //  exception.
            //
            if (!m_bSimMode)
            {
                if (bCheckShutdownRequest())
                {
                    //
                    //  Send a close message indicating going away. Don't wait for reply.
                    //  We need to stop and get shutdown quickly.
                    //
                    try
                    {
                        if (m_bLogStateInfo)
                            LogStateInfo(L"Asked to stop, shutting down session");
                        SendClose(kCQCWebSrvC::c2WSockErr_Exiting);
                    }

                    catch(...)
                    {
                    }
                    break;
                }
            }
             else
            {
                CheckShutdownReq();
            }

            //
            //  See if we need to do some time driven stuff.
            //
            {
                tCIDLib::TEncodedTime enctCur = TTime::enctNow();

                //
                //  If we haven't gotten a message from the client within the inactivity,
                //  timeout then give up.
                //
                if ((enctCur - m_enctLastInMsg) > CQCWebSrvC_WebSockHandler::enctActivityTO)
                {
                    if (m_bLogStateInfo)
                        LogStateInfo(L"Connection timeout, closing hard");

                    //
                    //  We don't do the normal thing since we assume he's not going
                    //  respond and it would just waste time to wait.
                    //
                    SendClose(kCQCWebSrvC::c2WSockErr_Timeout);
                    break;
                }

                //
                //  If it's been over 25 seconds since the last incoming message, and we
                //  aren't currently waiting on a pong reply, then send a ping.
                //
                if (((enctCur - m_enctLastInMsg) > (kCIDLib::enctOneSecond * 25))
                &&  !m_bWaitPong)
                {
                    SendPing();
                    enctCur = TTime::enctNow();
                }

                // Do a poll if it's time to
                if (enctCur > enctNextFldPoll)
                {
                    enctNextFldPoll = enctCur + kCIDLib::enctOneSecond;
                    PollFields();
                }
            }

            //
            //  Now we block on the socket and output msg queue events, waiting for
            //  something to process. This provides our throttling. But we can't block
            //  for long periods of time since we need to do the periodic processing
            //  above, and we need to call the Idle() callback. And we need to be able to
            //  quickly respond to shutdown requests.
            //
            const tCIDLib::TCard4 c4Which = TEvent::c4WaitMultiple(m_evOutMsgQ, m_evSock, 250);
            if (c4Which == 0)
            {
                // We have output to send
                if (m_bPauseOutput)
                {
                    //
                    //  The derived class has asked us to pause output. So reset the event
                    //  again so that we don't keep coming back into here at a rapid pace.
                    //  Once the derived class is ready, and calls in to clear the pause
                    //  flag, it will trigger this even if the queue is not empty.
                    //
                    m_evOutMsgQ.Reset();
                }
                 else
                {
                    //
                    //  Get the next available msg. A zero wait time since we know one
                    //  should be there.
                    //
                    TWebSockBuf* pwsbNext = m_colOutMsgQ.pobjGetNext(0, kCIDLib::False);
                    if (pwsbNext)
                    {
                        try
                        {
                            TJanitor<TWebSockBuf> janBuf(pwsbNext);

                            //
                            //  Call the fundamental msg sender to send it. The caller
                            //  marked it as the appropriate WS msg type when he queued i
                            //  up so we pass that along.
                            //
                            SendMsg
                            (
                                pwsbNext->m_c1Type
                                , *pwsbNext->m_pmbufData
                                , pwsbNext->m_c4CurBytes
                            );

                            // facCQCWebSrvC().DecWSOBufCount();
                        }

                        catch(TError& errToCatch)
                        {
                            if (facCQCWebSrvC().bLogWarnings())
                            {
                                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                                TModule::LogEventObj(errToCatch);
                            }

                            // Nothing here could be good, so just give up
                            m_eState = EStates::End;
                        }

                        catch(...)
                        {
                            if (facCQCWebSrvC().bLogWarnings())
                            {
                                facCQCWebSrvC().LogMsg
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , L"Uknown exception sending websocket message, ending session"
                                    , tCIDLib::ESeverities::Warn
                                );
                            }

                            // Nothing here could be good, so just give up
                            m_eState = EStates::End;
                        }
                    }
                     else
                    {
                        //
                        //  Nothing left so reset the event. We have to avoid possible
                        //  race conditions here, so we lock and then reset, after checking
                        //  that another thread hasn't slipped something in just now.
                        //
                        TLocker lockrSync(&m_mtxSync);
                        if (m_colOutMsgQ.bIsEmpty())
                            m_evOutMsgQ.Reset();
                    }
                }

            }
             else if (c4Which == 1)
            {
                //
                //  Reset the event for the next round. It will get triggered again below
                //  if more data is available after we finish reading what we read this
                //  time around.
                //
                const tCIDSock::ESockEvs eEvents = m_pcdsServer->eEnumEvents(m_evSock);

                // If it's a close event, then we are done
                if (tCIDLib::bAllBitsOn(eEvents, tCIDSock::ESockEvs::Close))
                {
                    if (m_bLogStateInfo)
                        LogStateInfo(L"Socket close event received");
                    m_eState = EStates::End;
                    break;
                }

                //
                //  We have input to process. Depending on the current state, we
                //  handle it. If not in one of these states, then we ignore it since
                //  that could only mean we are on the way down right now anyway.
                //
                //  Note that, unlike with a normal socket, we have to keep reading
                //  because the data source buffers data. Otherwise, we'd end up with
                //  messages stuck in the buffer. This won't hold us up, we
                //  just get what is available since we do a trivial wait time.
                //
                //  It can't be zero because that would only do a select, and we need
                //  to do a read or the socket event won't get updated correctly. Having
                //  at least a 1 millisecond time means it'll actually do a read.
                //
                //  NOTE that normally we'd check the events for a read, but we are
                //  looping here and checking for data available, so if we get a
                //  non-read event here, we just won't have any data available and
                //  will do nothing.
                //
                while (m_pcdsServer->bDataAvailMS(1))
                {
                    if (m_eState == EStates::Ready)
                    {
                        // Check for an incoming msg
                        tCIDLib::TCard1 c1FragType;
                        tCIDLib::TCard4 c4FragLen;

                        if (bGetFragment(c1FragType, bFinal, m_mbufReadFrag, c4FragLen))
                        {
                            if (c1FragType == kCQCWebSrvC::c1WSockMsg_Cont)
                            {
                                // This can't be valid
                                if (m_bLogStateInfo)
                                    LogStateInfo(L"Unstarted continuation");
                                StartShutdown(kCQCWebSrvC::c2WSockErr_UnstartedCont);
                            }
                             else if (bFinal)
                            {
                                // It's a single fragment msg
                                HandleMsg(c1FragType, m_mbufReadFrag, c4FragLen);
                            }
                             else
                            {
                                // It has to be a binary or text msg
                                if ((c1FragType == kCQCWebSrvC::c1WSockMsg_Text)
                                ||  (c1FragType == kCQCWebSrvC::c1WSockMsg_Bin))
                                {
                                    if (m_bLogStateInfo)
                                        LogStateInfo(L"Got first fragment");

                                    m_mbufReadMsg.CopyIn(m_mbufReadFrag, c4FragLen, 0);
                                    c1MsgType = c1FragType;
                                    c4MsgLen = c4FragLen;

                                    // Move to in message state to wait for remainder
                                    m_eState = EStates::InMsg;
                                }
                                 else
                                {
                                    if (m_bLogStateInfo)
                                        LogStateInfo(L"Bad fragment, close");

                                    StartShutdown(kCQCWebSrvC::c2WSockErr_BadFragMsg);
                                }
                            }
                        }
                    }
                     else if (m_eState == EStates::InMsg)
                    {
                        //
                        //  We are accumulating a msg, get another frame. THough it may
                        //  be a control message arriving inside a fragmented data msg.
                        //
                        tCIDLib::TCard1 c1FragType;
                        tCIDLib::TCard4 c4FragLen;
                        if (bGetFragment(c1FragType, bFinal, m_mbufReadFrag, c4FragLen))
                        {
                            if (c1FragType == kCQCWebSrvC::c1WSockMsg_Cont)
                            {
                                if (m_bLogStateInfo)
                                {
                                    if (bFinal)
                                        LogStateInfo(L"Got final fragment");
                                    else
                                        LogStateInfo(L"Got cont fragment");
                                }

                                // Add it to our accumulating msg
                                m_mbufReadMsg.CopyIn(m_mbufReadFrag, c4FragLen, c4MsgLen);
                                c4MsgLen += c4FragLen;

                                // If this is the end, then
                                if (bFinal)
                                {
                                    // Go back to Ready state
                                    m_eState = EStates::Ready;

                                    // And let the derived class deal with this message
                                    HandleMsg(c1FragType, m_mbufReadMsg, c4MsgLen);
                                }
                            }
                             else
                            {
                                // It has to be a nested control frame
                                if (bFinal)
                                {
                                    if (!bControlType(c1FragType))
                                    {
                                        if (m_bLogStateInfo)
                                            LogStateInfo(L"New message within previous msg");
                                        StartShutdown(kCQCWebSrvC::c2WSockErr_Nesting);
                                    }
                                     else
                                    {
                                        HandleMsg(c1FragType, m_mbufReadFrag, c4FragLen);
                                    }
                                }
                                 else
                                {
                                    if (m_bLogStateInfo)
                                        LogStateInfo(L"Non-final ctrl msg");
                                    StartShutdown(kCQCWebSrvC::c2WSockErr_NonFinalCtrl);
                                }
                            }
                        }
                    }
                     else if (m_eState == EStates::WaitClientEnd)
                    {
                        //
                        //  We are waiting for the client to send the return close after we
                        //  sent one first.
                        //
                        tCIDLib::TCard1 c1FragType;
                        tCIDLib::TCard4 c4FragLen;

                        if (bGetFragment(c1FragType, bFinal, m_mbufReadFrag, c4FragLen))
                        {
                            // Break out when we get the close. Ignore everything else
                            if (bFinal && (c1FragType == kCQCWebSrvC::c1WSockMsg_Close))
                            {
                                if (m_bLogStateInfo)
                                    LogStateInfo(L"Got client close response, exiting");
                                HandleMsg(c1FragType, m_mbufReadFrag, c4FragLen);
                                m_eState = EStates::End;
                            }
                        }
                         else if (TTime::enctNow() > m_enctEndTimer)
                        {
                            if (m_bLogStateInfo)
                                LogStateInfo(L"No response to Close, giving up");
                            m_eState = EStates::End;
                        }
                    }
                }
            }
             else
            {
                // Neither, we just timed out, so just call the the idle callback
                Idle();
            }
        }
    }

    catch(TError& errToCatch)
    {
        Cleanup(bCalledConn);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_WebsockExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );

        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(const TDbgExitException&)
    {
        Cleanup(bCalledConn);

        // Propagate these out to the CML IDE
        throw;
    }

    catch(...)
    {
        Cleanup(bCalledConn);

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_WebsockExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );

        return tCIDLib::EExitCodes::RuntimeError;
    }

    // Make sure we get cleaned up
    Cleanup(bCalledConn);

    return tCIDLib::EExitCodes::Normal;
}


// Let the web server know what type we are
tCQCWebSrvC::EWSockTypes TWebsockThread::eType() const
{
    return m_eType;
}


//
//  The derived class can tell us one to log msgs.
//
tCIDLib::TVoid TWebsockThread::EnableMsgLogging(const tCIDLib::TBoolean bState)
{
    // Whether false or true, close any existing output file
    if (m_pstrmLog)
    {
        try
        {
            delete m_pstrmLog;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        m_pstrmLog = nullptr;
    }

    // If true, then open up the file again, truncating any existing content
    if (bState)
    {
        // Set up the log file path
        TPathStr pathLog(facCQCKit().strDataDir());
        pathLog.AddLevel(L"Logs");
        pathLog.AddLevel(L"WebsockLog");
        pathLog.AppendExt(L"Txt");

        try
        {
            m_pstrmLog = new TTextFileOutStream
            (
                pathLog
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Write
                , new TUTF8Converter()
            );

            m_tmLog.SetToNow();
            *m_pstrmLog << L"Log opened at " << m_tmLog << kCIDLib::NewEndLn;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// Access a query parameter (from the URL that invoked us) at the indicated index
const TKeyValuePair& TWebsockThread::kvalQParamAt(const tCIDLib::TCard4 c4At) const
{
    return m_colQParams[c4At];
}


//
//  The derived class can call this to pause/unpause the sending of messages. If unpausing,
//  then we have to see if the output queue is empty or not. If not, then we have to trigger
//  the output queue event to start msgs being processed again.
//
tCIDLib::TVoid TWebsockThread::PauseOutput(const tCIDLib::TBoolean bState)
{
    TLocker lockrSync(&m_colOutMsgQ);

    if (!bState && !m_colOutMsgQ.bIsEmpty())
        m_evOutMsgQ.Trigger();

    m_bPauseOutput = bState;
}


//
//  This is called by the facility class when a new websockets handler needs to be
//  started up. It finds a fallow one from its list, or creates a new one, then calls
//  this to get us the new data. Then it starts us up.
//
tCIDLib::TVoid
TWebsockThread::Reset(          TCIDDataSrcJan& janSrc
                        , const TURL&           urlReq
                        , const TString&        strSecKey)
{
    m_bPauseOutput = kCIDLib::False;
    m_bWaitPong = kCIDLib::False;
    m_evOutMsgQ.Reset();
    m_evSock.Reset();
    m_eState = EStates::Connecting;
    m_strSecKey = strSecKey;
    m_urlReq = urlReq;

    // Get the expanded query parameters
    urlReq.QueryExpQParams(m_colQParams);

    // Set our state logging flag
    TString strVal;
    if (bFindQParam(L"logstateinfo", strVal))
        m_bLogStateInfo = kCIDLib::True;
    else
        m_bLogStateInfo = kCIDLib::False;

    //
    //  OK, we can safely take responsibility for the source. It's really a socket based
    //  data source, but the janitor works via the base class, so we have to cast it.
    //
    //  The actual type is a socket based data source, so we have to cast it so that we
    //  can see it as the real type and get access to the socket. It could be a secure or
    //  non-secure, so we look at it via a common base class.
    //
    m_pcdsServer = static_cast<TCIDSockStreamBasedDataSrc*>(janSrc.pcdsOrphan());

    // Associate our socket event with this guy's socket
    m_pcdsServer->AssociateReadEvent(m_evSock);

    // And start us running if not in simulator mode
    if (!m_bSimMode)
        Start();
}


//
//  The derived class can tell us to monitor a list of fields and we'll tell him any
//  time they change. Some derivatives may use this, some many not. Some may just want
//  to use our polling engine directly.
//
tCIDLib::TVoid TWebsockThread::SetFieldList(const tCIDLib::TStrList& colToSet)
{
    //
    //  Update our list of poll info fields. Any that we did use and now don't (and any
    //  other sessions don't) will eventually time out of the engine.
    //
    m_colFields.RemoveAll();

    TString strMoniker;
    TString strFldName;

    TCQCFldCache cfcFields;
    tCIDLib::TCard4 c4Count = colToSet.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (facCQCKit().bParseFldName(colToSet[c4Index], strMoniker, strFldName))
        {
            TCQCFldPollInfo cfpiCur(strMoniker, strFldName);
            m_colFields.objAdd(cfpiCur);
        }
         else
        {
            if (m_bLogStateInfo)
                LogStateInfo(L"Invalid field name registered");
        }
    }

    //
    //  Go back through the ones that we kept (no errors in the field name), and register
    //  them with the poll engine in the facility object.
    //
    c4Count = m_colFields.c4ElemCount();
    if (c4Count)
    {
        TCQCPollEngine& polleTar = facCQCWebSrvC().polleThis();
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCFldPollInfo& cfpiCur = m_colFields[c4Index];
            if (!cfpiCur.bRegister(polleTar, cfcData))
            {
            }
        }
    }
}


//
//  The derived class can call this if he wants to start the shutdown process, initiated
//  from our side. If we are not a state that indicate the client already started the
//  close, then we send the close and then
//
tCIDLib::TVoid TWebsockThread::StartShutdown(const tCIDLib::TCard2 c2Code)
{
    SendClose(c2Code);
    m_eState = EStates::WaitClientEnd;

    // Set our timer for how long we will wait for the reply
    m_enctEndTimer = TTime::enctNowPlusSecs(10);
}


//
//  Allows the derived class (or any of its threads) to wait for space to be available in
//  the outgoing message queue. For most derivatives this will never block. But for some
//  like the RIVA derivate, it blasts a lot of stuff at once sometimes.
//
tCIDLib::TVoid
TWebsockThread::WaitOutSpaceAvail(const tCIDLib::TCard4 c4WaitMSs)
{
}




// ---------------------------------------------------------------------------
//  TWebsockThread: Hidden constructors
// ---------------------------------------------------------------------------
TWebsockThread::TWebsockThread( const   tCQCWebSrvC::EWSockTypes    eType
                                , const tCIDLib::TBoolean           bSimMode) :

    TThread(facCIDLib().strNextThreadName(L"WebSockHandThread"))
    , m_bLogStateInfo(kCIDLib::False)
    , m_bPauseOutput(kCIDLib::False)
    , m_bSimMode(bSimMode)
    , m_bWaitPong(kCIDLib::False)
    , m_c4PingVal(TTime::c4Millis())
    , m_colOutMsgQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_eState(EStates::Connecting)
    , m_eType(eType)
    , m_enctEndTimer(0)
    , m_enctLastInMsg(0)
    , m_enctLastOutMsg(0)
    , m_evOutMsgQ(tCIDLib::EEventStates::Reset, kCIDLib::True)
    , m_evSock(tCIDLib::EEventStates::Reset, kCIDLib::True)
    , m_mbufReadFrag(kCQCWebSrvC::c4MaxWebsockMsgSz / 4, kCQCWebSrvC::c4MaxWebsockMsgSz)
    , m_mbufReadMsg(kCQCWebSrvC::c4MaxWebsockMsgSz / 4, kCQCWebSrvC::c4MaxWebsockMsgSz)
    , m_pcdsServer(nullptr)
    , m_pstrmLog(nullptr)
{
    // Set the default time format for our verbose logging stuff
    m_tmLog.strDefaultFormat(TTime::strMMDD_HHMMSS());
}



// ---------------------------------------------------------------------------
//  TWebsockThread: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Thread processing starts here.
//
tCIDLib::EExitCodes TWebsockThread::eProcess()
{
    //
    //  Put a janitor on the source, to insure it's cleaned up. Do this before we
    //  let the calling thread go.
    //
    TCIDDataSrcJan janSrc(m_pcdsServer, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True);

    // Let the calling thread go
    Sync();

    //
    //  Call the service method. This is separated out, because it needs to be called
    //  in simulator mode (from the IDE) as well, in which case it's not being called
    //  from another thread.
    //
    tCIDLib::EExitCodes eRet = tCIDLib::EExitCodes::Normal;
    try
    {
        eRet = eServiceClient();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        eRet = tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWRIVA_SysExcept
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , TString(L"processing client")
        );
        eRet = tCIDLib::EExitCodes::SystemException;
    }

    return eRet;
}


// ---------------------------------------------------------------------------
//  TWebsockThread: Protected, virtual methods
// ---------------------------------------------------------------------------

//
//  The derived class will override and call here. We reset any per-session members.
//
//  NOTE that the socket will get cleaned up upon return back from the client servicing
//  method. At this point, we are called at the end of that, so it is still valid
//  here.
//
tCIDLib::TVoid TWebsockThread::WSTerminate()
{

}


// ---------------------------------------------------------------------------
//  TWebsockThread: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to queue a text msg for later transmission. We transcode it to a buffer
//  in UTF-8 format, and then queue it up.
//
//  This is mostly for low volume scenarios. For higher volume stuff, use the other
//  version that lets you pass in the already flattened out message, which hopefully
//  you do in some more efficient manner. It should be in UTF8 format.
//
//  If the queue is full, we throw. The caller should wait for space to be sure.
//
tCIDLib::TVoid TWebsockThread::QueueTextMsg(const TString& strText)
{
    // If we have been told to log msgs, then do that
    if (m_pstrmLog)
    {
        m_tmLog.SetToNow();
        *m_pstrmLog << L"----- Sending (" << m_tmLog << L") -----\n"
                    << strText << kCIDLib::NewEndLn;
    }

    try
    {
        // Allocate a buffer object of this initial size
        TWebSockBuf* pwsbNew = new TWebSockBuf(strText.c4Length());
        TJanitor<TWebSockBuf> janBuf(pwsbNew);

        // Make sure the queue isn't growing out of control
        m_colOutMsgQ.CheckIsFull
        (
            (m_eType == tCQCWebSrvC::EWSockTypes::RIVA) ? 8192 : 256
            , L"web socket out msg queue"
        );

        // Convert the message to UTF into this buffer
        tCIDLib::TCard4 c4Bytes = 0;
        m_tcvtMsgs.c4ConvertTo(strText, *pwsbNew->m_pmbufData, c4Bytes);

        // Makes sure it's not larger than our max message size
        if (c4Bytes > kCQCWebSrvC::c4MaxWebsockMsgSz)
        {
            facCQCWebSrvC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWSock_MsgTooBig
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , TCardinal(c4Bytes)
            );
        }

        // Update the buffer with the actual bytes of the message
        pwsbNew->m_c4CurBytes = c4Bytes;

        // Mark this one as a text message
        pwsbNew->m_c1Type = kCQCWebSrvC::c1WSockMsg_Text;

        //
        //  And queue it up, orphaning it out of the janitor now that we are handing it off.
        //  The collection is thread safe, but we lock explicitly because we have to avoid
        //  race conditions wrt to the event we we trigger next.
        //
        {
            TLocker lockrSync(&m_colOutMsgQ);
            m_colOutMsgQ.Add(janBuf.pobjOrphan());

            //
            //  Post the event to indicate data is available in the queue. Have to do this
            //  while the mutex is locked to avoid race conditions.
            //
            m_evOutMsgQ.Trigger();
        }

        // Bump the count of used web socket output buffers now
        // facCQCWebSrvC().IncWSOBufCount();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}

tCIDLib::TVoid
TWebsockThread::QueueTextMsg(TMemBuf* const pmbufToAdopt, const tCIDLib::TCard4 c4Size)
{
    try
    {
        // Allocate a buffer object of this initial size
        TWebSockBuf* pwsbNew = new TWebSockBuf(pmbufToAdopt, c4Size);
        TJanitor<TWebSockBuf> janBuf(pwsbNew);

        // Makes sure it's not larger than our max message size
        if (c4Size > kCQCWebSrvC::c4MaxWebsockMsgSz)
        {
            facCQCWebSrvC().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWSock_MsgTooBig
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , TCardinal(c4Size)
            );
        }

        // Make sure the queue isn't growing out of control
        m_colOutMsgQ.CheckIsFull
        (
            (m_eType == tCQCWebSrvC::EWSockTypes::RIVA) ? 8192 : 256
            , L"web socket out msg queue"
        );

        // Mark this one as a text message
        pwsbNew->m_c1Type = kCQCWebSrvC::c1WSockMsg_Text;

        //
        //  And queue it up, orphaning it out of the janitor now that we are handing it off.
        //  The collection is thread safe, but we lock explicitly because we have to avoid
        //  race conditions wrt to the event we we trigger next.
        //
        {
            TLocker lockrSync(&m_colOutMsgQ);
            m_colOutMsgQ.Add(janBuf.pobjOrphan());

            //
            //  Post the event to indicate data is available in the queue. Have to do this
            //  while the mutex is locked to avoid race conditions.
            //
            m_evOutMsgQ.Trigger();
        }

        // Bump the count of used web socket output buffers now
        // facCQCWebSrvC().IncWSOBufCount();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  A helper for sending 16 bit payloads. We just adjust the payload for byte order as
//  required and call the base sender. This is used for control messages almost always.
//
//  NOTE that these are immediately sent, they are not queued. We need these to get to
//  the other side immediately in most cases.
//
tCIDLib::TVoid
TWebsockThread::SendCtrlMsg(const tCIDLib::TCard1 c1Type, const tCIDLib::TCard2 c2Payload)
{
    tCIDLib::TCard2 c2Send;
    #if defined(CID_LITTLEENDIAN)
    c2Send = tCIDLib::c2SwapBytes(c2Payload);
    #else
    c2Send = c2Payload;
    #endif

    // Call the low level sender
    SendMsg(c1Type, &c2Send, 2);
}


//
//  A simple helper that takes a memory buffer. We just turn around can call the base
//  message sender, passing the raw buffer pointer from the memory buffer.
//
tCIDLib::TVoid
TWebsockThread::SendMsg(const   tCIDLib::TCard1 c1Type
                        , const TMemBuf&        mbufData
                        , const tCIDLib::TCard4 c4DataCnt)
{
    // Pass it to the lower level method
    SendMsg(c1Type, mbufData.pc1Data(), c4DataCnt);
}


//
//  This is the base message sender. It handles message fragmentation if needed. The
//  other senders call this guy.
//
tCIDLib::TVoid
TWebsockThread::SendMsg(const   tCIDLib::TCard1         c1Type
                        , const tCIDLib::TVoid* const   pData
                        , const tCIDLib::TCard4         c4DataCnt)
{
    tCIDLib::TCard1 ac1Hdr[16];

    // For outgoing, we limit fragments to 0xFFFF
    const tCIDLib::TCard4 c4MaxFragSz = 0xFFFF;

    //
    //  We never send a message longer than our max packet size, fragmenting it if so.
    //  It's not too likely any will be that long anyway.
    //
    const tCIDLib::TCard1* pac1Data = static_cast<const tCIDLib::TCard1*>(pData);
    tCIDLib::TCard4 c4SoFar = 0;
    while (c4SoFar < c4DataCnt)
    {
        const tCIDLib::TCard4 c4Left = c4DataCnt - c4SoFar;

        //
        //  See how many bytes this time around, and set up a flags byte based on the
        //  size info, for use below.
        //
        tCIDLib::TCard4 c4ThisTime;
        tCIDLib::TCard1 c1Flags = 0;
        if (c4Left < c4MaxFragSz)
        {
            c4ThisTime = c4Left;
            c1Flags = 0x80;
        }
         else
        {
            c4ThisTime = c4MaxFragSz;
        }

        //
        //  First fragment is the actual type, else a continuation packet, and add in
        //  the flags byte we set up above.
        //
        tCIDLib::TCard4 c4Cnt = 0;
        if (c4SoFar)
            ac1Hdr[c4Cnt++] = kCQCWebSrvC::c1WSockMsg_Cont | c1Flags;
        else
            ac1Hdr[c4Cnt++] = c1Type | c1Flags;

        //
        //  Based on the size, set up the variable length size stuff. We only have to
        //  deal with two of the scenarios since we limit the fragments to 0xFFFF.
        //
        if (c4ThisTime < 126)
        {
            ac1Hdr[c4Cnt++] = tCIDLib::TCard1(c4ThisTime);
        }
         else
        {
            ac1Hdr[c4Cnt++] = 126;
            ac1Hdr[c4Cnt++] = tCIDLib::TCard1(c4ThisTime >> 8);
            ac1Hdr[c4Cnt++] = tCIDLib::TCard1(c4ThisTime);
        }

        // We never mask, so there's no masking data to add. So the header is done
        m_pcdsServer->WriteRawBytes(ac1Hdr, c4Cnt);

        //
        //  And then the payload data, so that we don't have to take the overhead hit
        //  of copying the data the data just to send it.
        //
        if (c4DataCnt)
            m_pcdsServer->WriteRawBytes(&pac1Data[c4SoFar], c4ThisTime);

        // Flush the data source output now
        m_pcdsServer->FlushOut(TTime::enctNowPlusSecs(8));

        // Move the so far up by the bytes we did this time
        c4SoFar += c4ThisTime;
    }

    // Update the last out message timestamp
    m_enctLastOutMsg = TTime::enctNow();
}


// Make the orignal URL available to the derived class if he wants to see it
const TURL& TWebsockThread::urlReq() const
{
    return m_urlReq;
}




// ---------------------------------------------------------------------------
//  TWebsockThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to get a web socket fragment. If not, we just return and the main
//  loop can do other things. The main loop handles putting fragments back together.
//
tCIDLib::TBoolean
TWebsockThread::bGetFragment(tCIDLib::TCard1&       c1Type
                            , tCIDLib::TBoolean&    bFinal
                            , TMemBuf&              mbufToFill
                            , tCIDLib::TCard4&      c4DataLen)
{
    const tCIDLib::TEncodedTime enctEnd(TTime::enctNowPlusSecs(2));
    c4DataLen = 0;
    try
    {
        //
        //  It has to be at least two bytes. If we get nothing, then return. We
        //  do a tolerant read of one byte first. If we get nothing, then we can just
        //  return and know we didn't get anytying out of sync. We do a very short
        //  wait time.
        //
        tCIDLib::TCard1 ac1Hdr[2];
        if (!m_pcdsServer->c4ReadBytes(ac1Hdr, 1, TTime::enctNowPlusMSs(1), kCIDLib::False))
            return kCIDLib::False;;

        //
        //  We must get a fragment now, so read the 2nd header byte. From here on
        //  out, we can indicate we must read all requested bytes or an error.
        //
        m_pcdsServer->c4ReadBytes(&ac1Hdr[1], 1, enctEnd, kCIDLib::True);

        //
        //  We check that none of the reserved bits are on and that the payload length
        //  first byte is not greater than 127.
        //
        if (ac1Hdr[0] & 0x70)
        {
            StartShutdown(kCQCWebSrvC::c2WSockErr_ResrvBits);
            return kCIDLib::False;
        }

        // Get the first length value, masking off the masking bit
        const tCIDLib::TCard1 c1LenByte1(ac1Hdr[1] & 0x7F);

        // Get the type and final flag out
        c1Type = ac1Hdr[0] & 0xF;
        bFinal = (ac1Hdr[0] & 0x80) != 0;

        // Figure out the actual length
        if (c1LenByte1 < 126)
        {
            c4DataLen = c1LenByte1;
        }
         else if (c1LenByte1 == 126)
        {
            // Read two more bytes and combine them
            tCIDLib::TCard1 ac1Len[2];
            m_pcdsServer->c4ReadBytes(ac1Len, 2, enctEnd, kCIDLib::True);

            c4DataLen = ac1Len[0];
            c4DataLen <<= 8;
            c4DataLen |= ac1Len[1];
        }
         else if (c1LenByte1 == 127)
        {
            // Read 8 more bytes and combine them
            tCIDLib::TCard1 ac1Len[8];
            m_pcdsServer->c4ReadBytes(ac1Len, 8, enctEnd, kCIDLib::True);

            for (tCIDLib::TCard4 c4BInd = 0; c4BInd < 8; c4BInd++)
            {
                if (c4BInd)
                    c4DataLen <<= 8;
                c4DataLen |= ac1Len[c4BInd];
            }
        }

        // If larger than our max fragment, then we close the connection.
        if (c4DataLen > kCQCWebSrvC::c4MaxWebsockFragSz)
        {
            StartShutdown(kCQCWebSrvC::c2WSockErr_FragTooBig);
            return kCIDLib::False;
        }

        // If the masking bit is on, then read out the mask
        const tCIDLib::TBoolean bMasked = (ac1Hdr[1] & 0x80) != 0;
        tCIDLib::TCard1 ac1Mask[4];
        if (bMasked)
            m_pcdsServer->c4ReadBytes(ac1Mask, 4, enctEnd, kCIDLib::True);

        //
        //  OK, let's read the indicated number of bytes to our fragment buffer,
        //  if there are any. We use an intermediate read buffer
        //
        if (c4DataLen)
        {
            // Expand the caller's fragment buffer if needed
            if (mbufToFill.c4Size() < c4DataLen)
                mbufToFill.Reallocate(c4DataLen, kCIDLib::False);

            // Read in the data
            m_pcdsServer->c4ReadBytes(mbufToFill, c4DataLen, enctEnd, kCIDLib::True);

            // If it's masked, then we have to unmask it
            if (bMasked)
            {
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DataLen; c4Index++)
                    mbufToFill.PutCard1(mbufToFill[c4Index] ^ ac1Mask[c4Index % 4], c4Index);
            }
        }

        //
        //  Update the last in message time stamp. It may only be a fragment, but
        //  basically it's just activity from the client.
        //
        m_enctLastInMsg = TTime::enctNow();
    }

    catch(TError& errToCatch)
    {
        // If the source is not connected, we have lost him
        if (!m_pcdsServer->bIsConnected())
        {
            if (m_bLogStateInfo)
                LogStateInfo(L"Socket connection lost, terminating...");

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            m_eState = EStates::End;
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWRIVA_SysExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Term
            , TString(L"getting websocket msg fragment")
        );
        m_eState = EStates::End;
    }
    return kCIDLib::True;
}


//
//  This is called when the connection is closed or lost, get cleaned up. We give the
//  derived class a chance to clean up first, then we do our own stuff.
//
tCIDLib::TVoid TWebsockThread::Cleanup(const tCIDLib::TBoolean bCallDisconn)
{
    //
    //  If indicated, call disconnect. This will be true if we got far enough to call
    //  connected.
    //
    if (bCallDisconn)
    {
        try
        {
            Disconnected();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_SysExcept
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Term
                , TString(L"Websocket disconnect")
            );
        }

        catch(...)
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_SysExcept
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Term
                , TString(L"Websocket disconnect")
            );
        }
    }

    // Let the derived class clean up first
    try
    {
        WSTerminate();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWRIVA_SysExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Term
            , TString(L"Websocket termination")
        );
    }

    catch(...)
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWRIVA_SysExcept
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Term
            , TString(L"Websocket termination")
        );
    }

    // Clean out the output message queue, returning all buffers to the poool
    ClearOutQ();

    m_bWaitPong         = kCIDLib::False;
    m_bPauseOutput      = kCIDLib::False;
    m_eState            = EStates::Connecting;
    m_enctEndTimer      = 0;
    m_enctLastInMsg     = 0;
    m_enctLastOutMsg    = 0;
    m_pcdsServer        = nullptr;

    // Clear our field list
    m_colFields.RemoveAll();

    // If the logging stream was created, make sure it gets cleaned up
    if (m_pstrmLog)
    {
        try
        {
            delete m_pstrmLog;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
        m_pstrmLog = nullptr;
    }
}


// Cleans up entries in the output queue, releasing them back to the pool
tCIDLib::TVoid TWebsockThread::ClearOutQ()
{
    try
    {
        m_colOutMsgQ.RemoveAll();
    }

    catch(...)
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcWSock_ClearOutQ
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  The main loop calls us here when it has a full msg buffered up. We process it.
//  Some we respond to ourself. If it's a text data msg, we dipatch it to the derived
//  class.
//
tCIDLib::TVoid
TWebsockThread::HandleMsg(  const   tCIDLib::TCard1 c1Type
                            , const THeapBuf&       mbufData
                            , const tCIDLib::TCard4 c4DataCnt)
{
    // Depending on the type
    switch(c1Type)
    {
        case kCQCWebSrvC::c1WSockMsg_Text :
        {
            //
            //  Transcode it to our string format. Catch any errors since we are
            //  required to fail the connection if so.
            //
            if (m_bLogStateInfo)
                LogStateInfo(L"Dispatching text msg");
            try
            {
                m_tcvtMsgs.c4ConvertFrom(mbufData.pc1Data(), c4DataCnt, m_strTextDispatch);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                if (m_bLogStateInfo)
                    LogStateInfo(L"Bad UTF-8 data, failing connection");
                SendClose(kCQCWebSrvC::c2WSockErr_BadData);
                return;
            }

            try
            {
                // If logging, then spit it out
                if (m_pstrmLog)
                {
                    m_tmLog.SetToNow();
                    *m_pstrmLog << L"----- Receiving (" << m_tmLog << L") -----\n"
                                << m_strTextDispatch << kCIDLib::NewEndLn;
                }

                // Pass it on to the derived class for processing
                ProcessMsg(m_strTextDispatch);
            }

            catch(TError& errToCatch)
            {
                if (facCQCWebSrvC().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    // Log the info and close the connection
                    facCQCWebSrvC().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCWSCErrs::errcMacro_ExceptCB
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Internal
                        , TString(L"ProcesMsg")
                    );
                }
                SendClose(kCQCWebSrvC::c2WSockErr_HandlerErr);
            }
            break;
        }

        case kCQCWebSrvC::c1WSockMsg_Bin :
        {
            // We don't process these, so just eat it
            if (m_bLogStateInfo)
                LogStateInfo(L"Ignoring binary msg");
            break;
        }

        case kCQCWebSrvC::c1WSockMsg_Close :
        {
            //
            //  The client sent a close. If we are not in waitclientend, then it's
            //  the client who started it, so we need to send a response. Otherwise,
            //  it's him responding to our close. Either way we are done.
            //
            tCIDLib::TCard2 c2Code = 0xFFFF;
            if (c4DataCnt >= 2)
            {
                c2Code = mbufData.c2At(0);
                #if defined(CID_LITTLEENDIAN)
                c2Code = tCIDLib::c2SwapBytes(c2Code);
                #endif
            }

            if (m_eState != EStates::WaitClientEnd)
            {
                if (m_bLogStateInfo)
                    LogStateInfo2(L"Client closed, sending response. Code=%(1)", TCardinal(c2Code));
                SendClose(0);
            }
             else
            {
                if (m_bLogStateInfo)
                    LogStateInfo2(L"Client returned close. Code=%(1)", TCardinal(c2Code));
            }
            m_eState = EStates::End;
            break;
        }

        case kCQCWebSrvC::c1WSockMsg_Ping :
        {
            // Send back a pong
            if (m_bLogStateInfo)
                LogStateInfo(L"Got ping, sending pong");

            // Send back the exact same data but change the opcode.
            SendMsg(kCQCWebSrvC::c1WSockMsg_Pong, mbufData, c4DataCnt);
            break;
        }

        case kCQCWebSrvC::c1WSockMsg_Pong :
        {
            //
            //  It will update the last incomng message time stamp, which is all we
            //  need it to do. If we aren't getting them back, then eventually we will
            //  time out and close the connection.
            //
            //  We also clear our waiting flag if set
            //
            if (m_bWaitPong)
            {
                m_bWaitPong = kCIDLib::False;

                if (m_bLogStateInfo)
                {
                    LogStateInfo(L"Got expected pong");

                    // See if the data is the same as what we would have sent
                    if ((c4DataCnt != 4) || (mbufData.c4At(0) != m_c4PingVal))
                        LogStateInfo(L"Did not get expected ping data back");
                }
            }
             else
            {
                if (m_bLogStateInfo)
                    LogStateInfo(L"Got unexpected pong");
            }
            break;
        }

        default :
            break;
    };
}



//
//  This is called regularly from the main loop to check for changes. We go through
//  the list of fields set on us and tell the derived class if any have changed.
//
tCIDLib::TVoid TWebsockThread::PollFields()
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    if (c4Count)
    {
        TCQCPollEngine& polleTar = facCQCWebSrvC().polleThis();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            TCQCFldPollInfo& cfpiCur = m_colFields[c4Index];

            // Remember the current state then do an update
            if (cfpiCur.bUpdateValue(polleTar))
            {
                try
                {
                    FieldChanged
                    (
                        cfpiCur.strMoniker()
                        , cfpiCur.strFieldName()
                        , cfpiCur.bHasGoodValue()
                        , cfpiCur.fvCurrent()
                    );
                }

                catch(TError& errToCatch)
                {
                    if (facCQCWebSrvC().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);

                        facCQCWebSrvC().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kCQCWSCErrs::errcMacro_ExceptCB
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Internal
                            , TString(L"FieldChanged")
                        );
                    }
                }
            }
        }
    }
}


//
//  If the derived class initializes OK, then this is called to send the acceptance
//  message. This is our last HTTP message. After this we move over to the Websocket
//  interface.
//
tCIDLib::TVoid TWebsockThread::SendAccept()
{
    if (m_bLogStateInfo)
        LogStateInfo(L"Sending acceptance");

    TString strSecKey = TString::strConcat(m_strSecKey, L"258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
    THeapBuf mbufKey(strSecKey.c4Length() * 2, strSecKey.c4Length() * 6);
    TUTF8Converter tcvtKey;
    tCIDLib::TCard4 c4KeyBytes;
    tcvtKey.c4ConvertTo(strSecKey, mbufKey, c4KeyBytes);

    // Hash the resulting buffer
    TSHA1Hash mhashKey;
    TSHA1Hasher mdigKey;
    mdigKey.StartNew();
    mdigKey.DigestBuf(mbufKey, c4KeyBytes);
    mdigKey.Complete(mhashKey);

    //
    //  Now we need to base 64 encode the resulting hash bytes. We reuse the
    //  security key string now since we don't need it anymore.
    //
    {
        TTextStringOutStream strmTar(&strSecKey);
        TBinMBufInStream strmKey(mhashKey.pc1Buffer(), mhashKey.c4Bytes());
        TBase64 b64Key;
        b64Key.Encode(strmKey, strmTar);
    }

    //
    //  OK, we have the key so we can build up the reply. Use one of our temp
    //  buffers to build it up. Make sure the line end type is what HTTP expects.
    //
    tCIDLib::TCard4 c4RepBytes = 0;
    TTextMBufOutStream strmOutput(512UL, 8192UL, facCIDEncode().ptcvtMake(L"ISO-8859-1"));
    strmOutput.eNewLineType(tCIDLib::ENewLineTypes::CRLF);

    strmOutput.Reset();
    strmOutput  << L"HTTP/1.1 101 Switching Protocols\n"
                << L"Upgrade: websocket\n"
                << L"Connection: Upgrade\n"
                << L"Content-Length: 0\n"
                << L"Content-Type: text/html\n"
                << L"Sec-WebSocket-Accept: " << strSecKey
                << kCIDLib::NewLn;

    // Now add a final empty line and flush
    strmOutput << kCIDLib::NewLn;
    strmOutput.Flush();

    // And we can send the header stuff and flush. There's no body
    m_pcdsServer->WriteBytes(strmOutput.mbufData(), strmOutput.c4CurSize());
    m_pcdsServer->FlushOut(TTime::enctNowPlusSecs(8));
}


// We send out a close message with the passed code.
tCIDLib::TVoid TWebsockThread::SendClose(const tCIDLib::TCard2 c2Code)
{
    if (m_bLogStateInfo)
        LogStateInfo2(L"Sending close message. Code=%(1)", TCardinal(c2Code));
    SendCtrlMsg(kCQCWebSrvC::c1WSockMsg_Close, c2Code);
}


//
//  If it's been over a period of tiem since any incoming message, we send a ping to
//  the client. We bump the ping value and send that as the data. We set the flag to
//  remember we are waiting. We don't have to worry about byte order since it just
//  sends back whatever we send to it.
//
tCIDLib::TVoid TWebsockThread::SendPing()
{
    if (m_bLogStateInfo)
        LogStateInfo(L"Sending ping");

    m_c4PingVal++;
    SendMsg(kCQCWebSrvC::c1WSockMsg_Ping, &m_c4PingVal, 4);
    m_bWaitPong = kCIDLib::True;
}

