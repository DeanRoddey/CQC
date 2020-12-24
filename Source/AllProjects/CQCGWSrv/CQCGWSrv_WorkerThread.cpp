//
// FILE NAME: CQCGWSrv_WorkerThread.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2002
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
//  This file implements the worker thread method, which will be used by a
//  number of worker thread instances, and some closely related grunt work
//  methods that it uses. Once it finds out what an incoming request is, it
//  will call the appropriate method in CQCGWSrv_ReqHandlers.cpp to handle
//  the request and return the reply.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWorkerThread,TThread)



// ---------------------------------------------------------------------------
//   CLASS: TWorkerThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWorkerThread: Constructors and Destructors
// ---------------------------------------------------------------------------
TWorkerThread::TWorkerThread(const TString& strName) :

    TThread(strName)
    , m_bNewFields(kCIDLib::False)
    , m_bSecure(kCIDLib::False)
    , m_c4LogOnTO(4000)
    , m_c4SysCfgSerNum(0)
    , m_colFields()
    , m_eOptFlags(tCQCGWSrv::EOptFlags::None)
    , m_enctLastMsg(TTime::enctNow())
    , m_esrMsgs()
    , m_mefrData(facCQCKit().strMacroRootPath())
    , m_pcdsClient(nullptr)
    , m_pmbufData(nullptr)
    , m_pxesMsgs(nullptr)
    , m_strmOutput(tCIDLib::TCard4(4196))
    , m_strmReply(4096, kCIDLib::c4DefMaxBufferSz, new TUTF8Converter)
    , m_xtprsMsgs()
    , m_uaccClient()
{
    //
    //  Set up the entity source reference that we will dump each incoming
    //  reqiest packet into and then have the parse parse from. This avoids
    //  have to allocate and delete an entity source for each request.
    //
    m_pmbufData = new THeapBuf(0x8192, kCQCGWSrv::c4MaxDataBytes);
    m_pxesMsgs = new TMemBufEntitySrc(L"CQCGWOpBuf", TCntPtr<const TMemBuf>(m_pmbufData), 8);
    m_esrMsgs.SetPointer(m_pxesMsgs);

    // Add a mapping for the inline DTD that we use
    m_xtprsMsgs.AddMapping
    (
        new TMemBufEntitySrc
        (
            kCQCGWSrv::pszSysId, kCQCGWSrv::pszPubId, TString(kCQCGWSrv::pszDTDText)
        )
    );

    //
    //  Set up the macro engine by installing our handlers on it. It does
    //  not adopt these, so we don't have to dynamically allocate them. We
    //  set up the file resolver above in the initializer list to point to
    //  the appointed location for macro created files.
    //
    m_meTarget.SetErrHandler(&m_meehLogger);
    m_meTarget.SetFileResolver(&m_mefrData);

    // Install the string stream as the console output stream
    m_meTarget.SetConsole(&m_strmOutput);

    //
    //  We want to get the DTD parsed and then lock the validator so that
    //  we don't have to parse the DTD over and over. It saves a lot of
    //  overhead and heap churn. So we just do a dummy parse of a simple
    //  but legal message, then lock the validator.
    //
    TString strDummyMsg(L"<?xml version='1.0' encoding='");
    strDummyMsg.Append(kCIDLib::pszNativeWCEncoding);
    strDummyMsg.Append(L"'?>\n");
    strDummyMsg.Append(kCQCGWSrv::pszDTD);
    strDummyMsg.Append
    (
        L"<CQCGW:Msg>\n<CQCGW:AckReply/>\n</CQCGW:Msg>\n"
    );

    tCIDXML::TEntitySrcRef esrToParse
    (
        new TMemBufEntitySrc(L"RootEntity", strDummyMsg)
    );

    const tCIDLib::TBoolean bRet = m_xtprsMsgs.bParseRootEntity
    (
        esrToParse, tCIDXML::EParseOpts::None, tCIDXML::EParseFlags::All
    );
    CIDAssert(bRet, L"XML parser pump prime failed");
    m_xtprsMsgs.bLockValidator(kCIDLib::True);
}


TWorkerThread::~TWorkerThread()
{
    // Unhook the various things we plugged into the macro engine
    m_meTarget.SetConsole(nullptr);
    m_meTarget.SetErrHandler(nullptr);
    m_meTarget.SetFileResolver(nullptr);

    // Drop the entity source reference to clean up now, while we can catch
    try
    {
        m_esrMsgs.DropRef();
    }

    catch( TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TWorkerThread::eProcess()
{
    // Let our caller go
    Sync();


    while (kCIDLib::True)
    {
        try
        {
            // Check for shutdown requests
            if (bCheckShutdownRequest())
                break;

            // Wait a while for a client to serve
            TUniquePtr<TSockLEngConn> uptrSock = facCQCGWSrv.uptrWait(500);

            // If nothign this time, just go back to top to check for shutdown and try again
            if (!uptrSock)
                continue;

            //
            //  Depending on which socket was connected to, we need to create the correct
            //  type of data source. In either case we orphan the socket out to the
            //  data source which now owns it.
            //
            TCIDDataSrc* pcdsDataSrc = nullptr;
            m_bSecure = uptrSock->bSecure();
            if (m_bSecure)
            {
                // If no cert info is available, this will fail and throw
                pcdsDataSrc = new TCIDSChanSrvDataSrc
                (
                    L"CQC XML GW Server"
                    , uptrSock->psockOrphan()
                    , tCIDLib::EAdoptOpts::Adopt
                    , facCQCGWSrv.strCertInfo()
                    , tCIDSChan::EConnOpts::None
                );
            }
            else
            {
                m_bSecure = kCIDLib::False;
                pcdsDataSrc = new TCIDSockStreamDataSrc(uptrSock->psockOrphan(), tCIDLib::EAdoptOpts::Adopt);
            }

            // Put a janitor on the data source to initialize/clean it up
            TCIDDataSrcJan janSrc
            (
                pcdsDataSrc, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True, TTime::enctNowPlusSecs(2), 500
            );

            // Store a pointer to it as a member, for convenience
            m_pcdsClient = pcdsDataSrc;

            // Send the acknowledgement. If that fails, it'll throw and we'll drop the client
            SendConnAck(uptrSock->ipepClient());

            // Store the end point of the client for logging msgs
            m_ipepClient = uptrSock->ipepClient();

            // And how we can service the client until he drops us
            ServiceClient();
        }

        catch(const TError& errToCatch)
        {
            if (facCQCGWSrv.bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            if (facCQCGWSrv.bLogFailures())
            {
                facCQCGWSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSMsgs::midStatus_WorkerError
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (facCQCGWSrv.bLogFailures())
            {
                facCQCGWSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSMsgs::midStatus_WorkerError
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        try
        {
            //
            //  Clean out the user context and poll list in preperation for
            //  the next round. And reset the XML parser and macro enginer
            //  to keep them from holding onto lots of memory potentially.
            //
            m_cuctxClient.Reset();
            m_colFields.RemoveAll();
            m_xtprsMsgs.Reset();
            m_meTarget.Reset();
        }

        catch(const TError& errToCatch)
        {
            if (facCQCGWSrv.bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }

        // Clear the data source pointer again now
        m_pcdsClient = nullptr;
    }
    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called upon first connection from the client, to ack our acceptance of his
//  connection. This allows us later, if need be, reject connections for some
//  reason.
//
tCIDLib::TVoid TWorkerThread::SendConnAck(const TIPEndPoint& ipepClient)
{
    // Log that we got this connection
    if (facCQCGWSrv.bLogInfo())
    {
        facCQCGWSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSMsgs::midStatus_GotGWConnect
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::Protocol
            , ipepClient
        );
    }

    //
    //  We need to send an ack or nack now. Reset the stream and
    //  build the common initial msg stuff.
    //
    m_strmReply.Reset();
    m_strmReply    << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                    << kCQCGWSrv::pszDTD
                    << L"<CQCGW:Msg>\n";

    m_strmReply    << L"    <CQCGW:ConnRes CQCGW:ConnStatus=";

    // For now we have no reason so reject, so we just send accepted
    //    if (bAccepted)
    m_strmReply << L"\"CQCGW:Accepted\" ";
    //    else
    //        strmReply << L"\"CQCGW:MaxClients\" ";

    // Stream the protocol version we implement
    m_strmReply << L"CQCGW:CQCVer=\"" << kCQCKit::pszVersion << L"\"/>\n";

    // Finish it off, flush it, and send it
    m_strmReply << L"</CQCGW:Msg>\n" << kCIDLib::FlushIt;

    //
    //  This is the one time we send something preemmptively, so set up some things
    //  in the header that normally we have from the incoming and just echo back.
    //
    m_hdrCur.c4MagicVal1 = kCQCGWSrv::c4MagicVal1;
    m_hdrCur.c4MagicVal2 = kCQCGWSrv::c4MagicVal2;
    m_hdrCur.c4SeqNum = 0;

    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  Called to format out the value of a field for the polling reply when the
//  field is ready and we have a good value to return. We return the index
//  that the field is from in the poll list as well.
//
tCIDLib::TVoid
TWorkerThread::FormatGoodValue(         TTextOutStream&     strmTarget
                                , const TCQCFldPollInfo&    cfpiSrc
                                , const tCIDLib::TCard4     c4PLIndex)
{
    TString strValue;
    cfpiSrc.fvCurrent().Format(strValue);
    strmTarget  << L"        <CQCGW:FldValue CQCGW:Field='"
                << cfpiSrc.strMoniker() << L"." << cfpiSrc.strFieldName()
                << L"' CQCGW:Status='CQCGW:Online'"
                << L" CQCGW:Index='" << c4PLIndex << L"' CQCGW:Value='";

    // Have to escape the field value as an attribute
    facCIDXML().EscapeFor(strValue, strmTarget, tCIDXML::EEscTypes::Attribute);
    strmTarget  << L"'/>\n";
}


//
//  Called to format out the value of a field for the polling reply when the
//  field is not ready, so we have to return an error indicator. We return
//  the index that the field is from in the poll list as well.
//
tCIDLib::TVoid
TWorkerThread::FormatBadValue(          TTextOutStream&     strmTarget
                                , const TCQCFldPollInfo&    cfpiSrc
                                , const tCIDLib::TCard4     c4PLIndex)
{
    strmTarget  << L"        <CQCGW:FldValue CQCGW:Field='"
                << cfpiSrc.strMoniker() << L"." << cfpiSrc.strFieldName()
                << L"' CQCGW:Status='CQCGW:InError' CQCGW:Index='"
                << c4PLIndex << L"' CQCGW:Value=\"\"/>\n";
}


//
//  The main client servicing method. The main thread entry point waits for
//  a client to service and then calls here if the connection is accepted.
//  We keep working until the client goes away or we get a comm error.
//
//  At this point m_uptrSocket is set, and m_ipepClient holds the end point of
//  the other side.
//
tCIDLib::TVoid TWorkerThread::ServiceClient()
{
    // Set any session oriented flags
    m_eOptFlags = tCQCGWSrv::EOptFlags::None;

    //
    //  The first thing we have to do is to force the client to do the logon
    //  sequence. So we wait for a logon request, and if we don't get it, or
    //  we get some other msg, we reject the client and drop the connection.
    //
    if (!bLogonSeq())
        return;

    // Reset the last message time to now to start off with
    m_enctLastMsg = TTime::enctNow();

    // And loop till we die or he drops the connection
    while (kCIDLib::True)
    {
        try
        {
            if (bCheckShutdownRequest())
                break;

            //
            //  Try to get a msg document. Use a short timeout since we are
            //  not necessarily expecting anything, just checking for input.
            //  But not too short since this also provides our throttling.
            //
            if (!bGetMsg(250))
            {
                //
                //  See how long since the last message. If too long, then
                //  time out this connection.
                //
                if (m_enctLastMsg + kCQCGWSrv::enctIdleTime < TTime::enctNow())
                {
                    // It has to have a class of LostConnection!
                    facCQCGWSrv.ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCGWSMsgs::midStatus_IdleTimeDrop
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::LostConnection
                        , m_ipepClient
                    );
                }
                continue;
            }

            // We got a message, so update the last message time
            m_enctLastMsg = TTime::enctNow();

            //
            //  There should be a single child of the root, which is the
            //  actual request node.
            //
            const TXMLTreeElement& xtnodeRoot = m_xtprsMsgs.xtdocThis().xtnodeRoot();
            const TXMLTreeElement& xtnodeReq = xtnodeRoot.xtnodeChildAtAsElement(0);

            if (xtnodeReq.strQName() == L"CQCGW:Disconnect")
            {
                // Just send back an ack, and we are done
                SendAckReply();
                break;
            }
             else if (xtnodeReq.strQName() == L"CQCGW:DoGlobalAct")
            {
                DoGlobalAct(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:MWriteField")
            {
                MWriteField(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:Query")
            {
                //
                //  It's the generic query message. The op attribute tells us
                //  which exact query it is. We'll call a method to handle each
                //  op.
                //
                const TString& strOp = xtnodeReq.xtattrNamed(L"CQCGW:QueryType").strValue();
                if (strOp == L"CQCGW:Ping")
                {
                    SendAckReply();
                }
                 else if (strOp == L"CQCGW:Poll")
                {
                    PollFields();
                }
                 else if (strOp == L"CQCGW:QueryDrvList")
                {
                    QueryDrvList();
                }
                 else if (strOp == L"CQCGW:QueryFieldList")
                {
                    QueryFields();
                }
                 else if (strOp == L"CQCGW:QueryGlobActs")
                {
                    QueryGlobalActs();
                }
                 else if (strOp == L"CQCGW:QueryMacros")
                {
                    QueryMacros();
                }
                 else if (strOp == L"CQCGW:QueryRmCfgList")
                {
                    QueryRoomCfgList();
                }
                 else if (strOp == L"CQCGW:QueryRepoDrvs")
                {
                    QueryMediaRepoDrvs();
                }
                 else if (strOp == L"CQCGW:QueryPollList")
                {
                    QueryPollList();
                }
                 else
                {
                    //
                    //  Dunno what this is. Throw an exception, which will be
                    //  caught below and logged and sent back to the caller.
                    //
                    facCQCGWSrv.ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCGWSErrs::errcProto_UnknownQueryOp
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Protocol
                        , strOp
                    );
                }
            }
             else if (xtnodeReq.strQName() == L"CQCGW:PauseSchEv")
            {
                PauseSchEv(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryDriverInfo")
            {
                QueryDrvInfo(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryDriverStatus")
            {
                QueryDrvStatus(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryDriverText")
            {
                QueryDrvText(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryFldInfo")
            {
                QueryFldInfo(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryFldInfoList")
            {
                QueryFldInfoList(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryImage")
            {
                QueryImage(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryMediaArt")
            {
                QueryMediaArt(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryMediaDB")
            {
                QueryMediaDB(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryMediaRendArt")
            {
                QueryMediaRendArt(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:RoomCfgReq")
            {
                QueryRoomCfg(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QuerySchEv")
            {
                QuerySchEv(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:ReadField")
            {
                ReadField(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:QueryRendPL")
            {
                QueryRendPL(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:RunMacro")
            {
                // Get the macro name attribute out and pass it
                RunMacro(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:SetOpts")
            {
                SetOption(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:SetPerEv")
            {
                SetPerEvInfo(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:SetPollList")
            {
                SetPollList(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:SetSchEv")
            {
                SetSchEvInfo(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:SetSunEv")
            {
                SetSunEvInfo(xtnodeReq);
            }
             else if (xtnodeReq.strQName() == L"CQCGW:WriteField")
            {
                WriteField(xtnodeReq, kCIDLib::True);
            }
             else
            {
                //
                //  Dunno what this is. Throw an exception, which will be
                //  caught below and logged and sent back to the caller.
                //
                facCQCGWSrv.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSErrs::errcProto_UnknownMsg
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , xtnodeReq.strQName()
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            //
            //  If it's that we've lost the connection, then break out. Else,
            //  send the exception to the client
            //
            if (!m_pcdsClient->bIsConnected())
            {
                if (facCQCGWSrv.bLogWarnings())
                {
                    facCQCGWSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCGWSMsgs::midStatus_GWClientDropped
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                        , m_ipepClient
                    );
                }
                break;
            }
             else
            {
                if (facCQCGWSrv.bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCGWSrv.bLogWarnings())
                {
                    facCQCGWSrv.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCGWSMsgs::midStatus_ServiceLoopError
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::AppStatus
                        , m_ipepClient
                    );
                }
                SendExceptionReply(errToCatch);
            }
        }

        catch(...)
        {
            if (facCQCGWSrv.bLogWarnings())
            {
                facCQCGWSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSMsgs::midStatus_ServiceLoopError
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::AppStatus
                    , m_ipepClient
                );
            }

            // Send the exception to the client
            SendUnknownExceptionReply();

            // Assume the worst and break out to drop the connection
            break;
        }
    }
}


tCIDLib::TVoid TWorkerThread::SetPollList(const TXMLTreeElement& xtnodeReq)
{
    //
    //  Go through all of the child nodes, which represent the fields to
    //  add to the poll list, and set up poll info objects for them and
    //  register those with the polling engine.
    //
    m_colFields.RemoveAll();
    if (xtnodeReq.c4ChildCount())
    {
        // Iterate the children and add a widget for each one
        const tCIDLib::TCard4 c4ChildCount = xtnodeReq.c4ChildCount();
        TString strMon;
        TString strFld;
        try
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
            {
                const TXMLTreeElement& xtnodeFld = xtnodeReq.xtnodeChildAtAsElement(c4Index);

                //
                //  Create the new widget and tell it about the field it's going
                //  to be connected to.
                //
                facCQCKit().ParseFldName(xtnodeFld.xtattrNamed(L"CQCGW:Name").strValue(), strMon, strFld);

                // Add a new poll info object
                m_colFields.objAdd(TCQCFldPollInfo(strMon, strFld));
            }

            //
            //  Ok, we now go through the poll info objects and ask each of
            //  them register themselves on our polling engine.
            //
            TCQCPollEngine& polleTar = facCQCGWSrv.polleThis();
            TCQCFldCache cfcData;
            cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
            {
                TCQCFldPollInfo& cfpiCur = m_colFields[c4Index];
                cfpiCur.bRegister(polleTar, cfcData);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // If we failed, then get rid of any widgets we got done
            m_colFields.RemoveAll();
            throw;
        }
    }

    // Set the new fields flag to indicate a change in the field list
    m_bNewFields = kCIDLib::True;

    //
    //  We either send an ack back, or if they asked for field info to be
    //  returned, we call QueryPollList() which returns field info on all
    //  of the fields currently in the poll list.
    //
    const TString& strFlag = xtnodeReq.xtattrNamed(L"CQCGW:RetInfo").strValue();
    if (strFlag.bCompareI(L"Yes"))
        QueryPollList();
    else
        SendAckReply();
}


