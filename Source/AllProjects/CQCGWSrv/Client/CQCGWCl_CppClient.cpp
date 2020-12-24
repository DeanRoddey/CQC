//
// FILE NAME: CQCGWCl_CpPClient.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/2/2013
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
//  This file implements a C++ wrapper around the XML Gateway interface.
//
// CAVEATS/GOTCHAS:
//
//  1)  We call CheckConnected() in the low level get and send message
//      interfaces, i.e. the places where we directly assume that the socket
//      exists and will reference it.
//
//      So any public methods that would involve sending/getting messages
//      don't have to explicitly check for having been connected. They'll
//      get checked in the process of communications.
//
//      For some others that don't do that, but wouldn't make sense to call
//      before being connected, we call it explcitly.
//
//      This means we have to store the socket immediately in the Connect()
//      method, and just clean it back up if the connect fails, since we
//      have to get/send messages during the processing of logging on.
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes.
// ---------------------------------------------------------------------------
#include    "CQCGWCl_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCGWSrvClient,TObject)



// ---------------------------------------------------------------------------
//  CLASS: TCQCGWSrvClient
// PREFIX: gwsc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCGWSrvClient: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCGWSrvClient::TCQCGWSrvClient() :

    m_c4SeqNum(1)
    , m_eUserRole(tCQCKit::EUserRoles::Count)
    , m_pmbufData(nullptr)
    , m_pcdsClient(nullptr)
    , m_pxtprsMsgs(new TXMLTreeParser)
    , m_strmReply(4096UL)
{
    //
    //  Set up the buffer we will dump each incoming request packet into
    //  and then have the parser parse from. This avoids have to allocate
    //  and it each time. We just have to set up a entity source object
    //  around it, but passing it in via counted pointer will mean that
    //  the buffer stays alive across parsing calls.
    //
    m_pmbufData = new THeapBuf(0x8192, kCQCGWSrv::c4MaxDataBytes);
    m_cptrBuf.SetPointer(m_pmbufData);

    //
    //  Add a mapping for the inline DTD that we use. This will make the
    //  parser automatically use this DTD when it sees the public id in the
    //  incoming messages.
    //
    m_pxtprsMsgs->AddMapping
    (
        new TMemBufEntitySrc
        (
            kCQCGWSrv::pszSysId, kCQCGWSrv::pszPubId, TString(kCQCGWSrv::pszDTDText)
        )
    );
}

TCQCGWSrvClient::~TCQCGWSrvClient()
{
    // Clean up the socket if we weren't closed properly
    if (m_pcdsClient)
    {
        try
        {
            if (m_pcdsClient->bIsConnected())
                m_pcdsClient->Terminate(TTime::enctNowPlusSecs(2), kCIDLib::True);
        }

        catch(const TError& errToCatch)
        {
            if (facCQCKit().bShouldLog(errToCatch))
                facCQCKit().LogEventObj(errToCatch);
        }

        try
        {
            delete m_pcdsClient;
            m_pcdsClient = nullptr;
        }

        catch(const TError& errToCatch)
        {
            if (facCQCKit().bShouldLog(errToCatch))
                facCQCKit().LogEventObj(errToCatch);
        }
    }

    // Clean up the XML parser
    if (m_pxtprsMsgs)
    {
        delete m_pxtprsMsgs;
        m_pxtprsMsgs = nullptr;
    }
}


// ---------------------------------------------------------------------------
//  TCQCGWSrvClient: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Checks to see if our socket is connected. This lets the client, after
//  any exception, explicitly see if it's because of loss of connection.
//
tCIDLib::TBoolean TCQCGWSrvClient::bIsConnected() const
{
    return ((m_pcdsClient != nullptr) && m_pcdsClient->bIsConnected());
}


//
//  The client can call this in a loop after a call to bPollFields(), to
//  see which fields changed.
//
tCIDLib::TBoolean
TCQCGWSrvClient::bCheckField(const  tCIDLib::TCard4     c4Index
                            ,       tCIDLib::TBoolean&  bState
                            ,       TString&            strValue)
{
    // Make sure they've called Connect()
    CheckConnected();

    // Make sure the index is valid
    if (c4Index >= m_colFields.c4ElemCount())
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index)
            , TString(L"XMLGWClient")
            , TCardinal(m_colFields.c4ElemCount())
        );
    }

    TFieldInfo& fldiTar = m_colFields[c4Index];

    // If changed, get the info out
    if (fldiTar.m_bChanged)
    {
        bState = fldiTar.m_bState;

        // If the state is good, return the value
        if (bState)
            strValue = fldiTar.m_strValue;
        else
            strValue.Clear();

        fldiTar.m_bChanged = kCIDLib::False;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCGWSrvClient::bDoGlobalAct(  const   TString&            strActPath
                                , const tCIDLib::TStrList&  colParms
                                ,       TString&            strFailReason)
{
    //
    //  Build up a new message to run a macro. Probably not a problem but
    //  escape the path just in case.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:DoGlobalAct CQCGW:ActPath='";

    facCIDXML().EscapeFor
    (
        strActPath, m_strmReply, tCIDXML::EEscTypes::Attribute
    );
    m_strmReply << L"'>\n";

    // For each of the parms, add a child
    const tCIDLib::TCard4 c4PCount = colParms.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PCount; c4Index++)
    {
        m_strmReply << L"    <CQCGW:ActParm>";

        // We have to escape it for element content
        facCIDXML().EscapeFor
        (
            colParms[c4Index], m_strmReply, tCIDXML::EEscTypes::ElemText
        );

        m_strmReply << L"</CQCGW:ActParm>\n";
    }

    // Cap it all off now and send it
    m_strmReply << L"    </CQCGW:DoGlobalAct>\n</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    // And we should get back an ack or nack back
    return bGetAckNak(strFailReason, 15000);
}


//
//  Called to poll the current poll list for any changes. If we get any,
//  we will mark those in our field list for the client come and get later
//  by calling bCheckField().
//
tCIDLib::TBoolean TCQCGWSrvClient::bPollFields()
{
    // Send a standard query message with a poll op
    SendQueryOp(L"CQCGW:Poll");

    // And we should get a poll reply back
    const TXMLTreeElement& xtnodePoll = xtnodeGetMsg(L"CQCGW:PollReply", 8000);

    //
    //  Iterate it and show the values of all of the fields in the list. It
    //  only returns changed fields.
    //
    const tCIDLib::TCard4 c4Count = xtnodePoll.c4ChildCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeElement& xtnodeCur
                                = xtnodePoll.xtnodeChildAtAsElement(c4Index);
            const TString& strFld = xtnodeCur.xtattrNamed(L"CQCGW:Field").strValue();

            // Look up this field in the list
            const tCIDLib::TCard4 c4FldInd = c4FindFld(strFld, kCIDLib::True);
            TFieldInfo& fldiCur = m_colFields[c4FldInd];

            // Mark it changed and store the state and value
            fldiCur.m_bChanged = kCIDLib::True;
            fldiCur.m_bState =
            (
                xtnodeCur.xtattrNamed(L"CQCGW:Status").strValue() == L"CQCGW:Online"
            );
            fldiCur.m_strValue = xtnodeCur.xtattrNamed(L"CQCGW:Value").strValue();
        }
    }

    // Return true if any changes
    return (c4Count != 0);
}



//
//  Do a backdoor driver text query
//
tCIDLib::TBoolean
TCQCGWSrvClient::bQueryDrvText( const   TString&    strMoniker
                                , const TString&    strQType
                                , const TString&    strDataName
                                ,       TString&    strOutput)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QueryDriverText CQCGW:Moniker=\""
                << strMoniker << L"\" CQCGW:QType=\""
                << strQType << L"\" CQCGW:DataName=\""
                << strDataName << L"\"/>\n"
                << L"</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // And we should get back a driver text response
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:DriverText", 10000);

    // Get the text of the message out as the driver text
    strOutput = xtnodeRes.xtnodeChildAtAsText(0).strText();

    return xtnodeRes.xtattrNamed(L"CQCGW:Status").bValueAs();
}


//
//  Query the current cover art from a renderer.
//
tCIDLib::TBoolean
TCQCGWSrvClient::bQueryMediaRendArt(const   TString&            strMoniker
                                    ,       THeapBuf&           mbufData
                                    ,       tCIDLib::TCard4&    c4DataSz)
{
    // Assume no data until proven otherwise
    c4DataSz = 0;

    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QueryMediaRendArt CQCGW:Moniker=\""
                << strMoniker << L"\" CQCGW:Size=\"Lrg\"/></CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // And wait for the reply, which should be a media image data reply
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:MediaImgData", 10000);

    //
    //  Check the result. If no changes or failed, we an stop now. We should never
    //  get no changes in this call, but just in case.
    //
    const TString& strResult = xtnodeRes.xtattrNamed(L"CQCGW:Result").strValue();
    if (strResult.bCompareI(L"NoChanges") || strResult.bCompareI(L"Failed"))
        return kCIDLib::False;

    //
    //  We should have image data at this point, but check just in case and for
    //  and unknown image type.
    //
    const TString& strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    const TString& strImgData = xtnodeRes.xtnodeChildAtAsText(0).strText();
    if (strImgData.bIsEmpty() || (strType == L"Unknown"))
        return kCIDLib::False;

    //
    //  We just return the raw data to the caller. The data is base64 encoded
    //  binary data. So we need to set up an input stream on the data
    //  value, and and output binary stream on the caller's buffer. Then we
    //  can use a Base64 decoder to copy it over.
    //
    TTextStringInStream strmIn(&strImgData);
    TBinMBufOutStream   strmOut(&mbufData);
    TBase64 b64Decode;
    b64Decode.Decode(strmIn, strmOut);

    strmOut.Flush();
    c4DataSz = strmOut.c4CurSize();

    return kCIDLib::True;
}


//
//  Get a list of available scheduled events
//
tCIDLib::TBoolean
TCQCGWSrvClient::bQuerySchEvs(  tCIDLib::TCard4&                    c4SerialNum
                                , tCIDLib::TStrList&                colNames
                                , tCIDLib::TStrList&                colIds
                                , tCIDLib::TStrList&                colTypes
                                , TFundVector<tCIDLib::TBoolean>&   fcolStates)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QuerySchEvs CQCGW:SerialNum='"
                << c4SerialNum << L"'></CQCGW:QuerySchEvs>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // And we should get back a scheduled events reply
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:SchEvs", 10000);

    // Make sure all the lists are empty in any case
    colNames.RemoveAll();
    colIds.RemoveAll();
    colTypes.RemoveAll();
    fcolStates.RemoveAll();

    // Give back the new serial number if there is one
    c4SerialNum = xtnodeRes.xtattrNamed(L"CQCGW:SerialNum").c4ValueAs();

    // See if there are any changes to give back. If not, we are done
    if (!xtnodeRes.xtattrNamed(L"CQCGW:Changes").bValueAs())
        return kCIDLib::False;

    const tCIDLib::TCard4 c4Count = xtnodeRes.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur = xtnodeRes.xtnodeChildAtAsElement(c4Index);

        colNames.objAdd(xtnodeCur.xtattrNamed(L"CQCGW:Name").strValue());
        colIds.objAdd(xtnodeCur.xtattrNamed(L"CQCGW:Id").strValue());
        colTypes.objAdd(xtnodeCur.xtattrNamed(L"CQCGW:Type").strValue());
        fcolStates.c4AddElement(xtnodeCur.xtattrNamed(L"CQCGW:State").strValue() == L"Paused");
    }
    return kCIDLib::True;
}


//
//  For reading the value of an arbitrary field. We return true if the
//  field is reported to be online, else false if it's in error (which means
//  it might be in error state or just not found.)
//
tCIDLib::TBoolean
TCQCGWSrvClient::bReadField(const   TString&    strMoniker
                            , const TString&    strField
                            ,       TString&    strValue)
{
    // Build up the message to run a macro and send it
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf-8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                   L"    <CQCGW:ReadField CQCGW:Field='"
                << strMoniker << L"." << strField
                << L"'/>\n" << L"</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    // And we should get back a field value element
    const TXMLTreeElement& xtnodeVal = xtnodeGetMsg(L"CQCGW:FldValue", 10000);

    // Check the status
    const tCIDLib::TBoolean bRet
    (
        xtnodeVal.xtattrNamed(L"CQCGW:Status").strValue() == L"CQCGW:Online"
    );

    // Get the value out and give it ot the caller if online
    if (bRet)
        strValue = xtnodeVal.xtattrNamed(L"CQCGW:Value").strValue();

    return bRet;
}


//
//  Asks the GW to run a macro on our behalf. The caller has to provide us
//  with a list of separate macros, so that we don't have to parse them
//  out again.
//
tCIDLib::TBoolean
TCQCGWSrvClient::bRunMacro( const   TString&            strClassPath
                            , const tCIDLib::TStrList&  colParms
                            ,       TString&            strOutput
                            ,       tCIDLib::TInt4&     i4ResCode)
{
    //
    //  Build up a new message to run a macro. Probably not a problem but
    //  escape the macro path anyway.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:RunMacro CQCGW:ClassPath='";
    facCIDXML().EscapeFor
    (
        strClassPath, m_strmReply, tCIDXML::EEscTypes::Attribute
    );
    m_strmReply << L"'>\n";

    // For each of the parms, add a child
    const tCIDLib::TCard4 c4PCount = colParms.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PCount; c4Index++)
    {
        m_strmReply << L"    <CQCGW:MacroParm>";

        // We have to escape it for element content
        facCIDXML().EscapeFor
        (
            colParms[c4Index], m_strmReply, tCIDXML::EEscTypes::ElemText
        );

        m_strmReply << L"</CQCGW:MacroParm>\n";
    }

    // Cap it all off now and send it
    m_strmReply << L"    </CQCGW:RunMacro>\n</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    // And we should get back a macro results message
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:MacroResults", 10000);

    // Get the output from the macro (or error info) if any
    if (xtnodeRes.c4ChildCount())
        strOutput = xtnodeRes.xtnodeChildAtAsText(0).strText();

    const tCIDLib::TBoolean bRes
    (
        xtnodeRes.xtattrNamed(L"CQCGW:Status").strValue() == L"CQCGW:Success"
    );

    // Get the result code out, if a valid result
    i4ResCode = 0;
    if (bRes)
        i4ResCode = xtnodeRes.xtattrNamed(L"CQCGW:MacroRetVal").i4ValueAs();

    return bRes;
}



// Tell the caller how many items are in the poll list
tCIDLib::TCard4 TCQCGWSrvClient::c4PollListCount() const
{
    return m_colFields.c4ElemCount();
}


//
//  The caller provides us with an end point for an instance of the XML
//  gateway. He also passes a user name and password. We'll connect to the
//  server and do the login sequence. If anything fails, the socket is
//  cleaned up and an exception is thrown.
//
//  We'll send along the values of any of the standard environmentally
//  driven runtime values, by just grabbing them from the local
//  environment.
//
//  We can't already be connected.
//
tCIDLib::TVoid
TCQCGWSrvClient::Connect(const  TIPEndPoint&        ipepSrv
                        , const TString&            strUserName
                        , const TString&            strPassword
                        , const tCIDLib::TBoolean   bSecure)
{
    // Make sure we aren't already connected
    if (m_pcdsClient)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_AlreadyConnected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    //
    //  Get a client stream socket for the indicated end point, see if we
    //  can get connected to the gateway.
    //
    if (bSecure)
    {
        tCIDLib::TStrList colALPN;
        m_pcdsClient = new TCIDSChanClDataSrc
        (
            L"CQCGWSrvClient"
            , ipepSrv
            , tCIDSock::ESockProtos::TCP
            , TString::strEmpty()
            , colALPN
            , tCIDSChan::EConnOpts::None
            , L"CQSL-VM-HOST"
        );
    }
    else
    {
        m_pcdsClient = new TCIDSockStreamDataSrc(ipepSrv);
    }


    try
    {
        // Store the connection info
        m_strUserName = strUserName;
        m_strPassword = strPassword;

        // And try to connect to the server
        m_pcdsClient->Initialize(TTime::enctNowPlusSecs(2));

        // And now he will send us a ConnRes reply
        if (!bGetMsg(6000))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcXGWC_TimedOut
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
                , TString(L"connect ack")
            );
        }

        // Look at the reply reason
        const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
        const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

        if (xtnodeMsg.strQName() != L"CQCGW:ConnRes")
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcXGWC_ExpectedMsg
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
                , TString(L"CQCGW:ConnRes")
                , xtnodeMsg.strQName()
            );
        }

        // Get the result attribute and see if he accepted
        const TString& strReason = xtnodeMsg.xtattrNamed(L"CQCGW:ConnStatus").strValue();
        if (strReason != L"CQCGW:Accepted")
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcXGWC_ConnRejected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , strReason
            );
        }

        // And do the logon sequence
        LogOn();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (facCQCKit().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        Disconnect();
        throw;
    }

    catch(...)
    {
        // It failed for some reason, so clean up and rethrow
        Disconnect();
        throw;
    }
}


//
//  If we are connected, this will disconnect from the server. This will
//  leave our socket zeroed out, which is what indicates we are now
//  disconnected.
//
tCIDLib::TVoid TCQCGWSrvClient::Disconnect()
{
    if (m_pcdsClient)
    {
        try
        {
            //
            //  Try to send a disconnect message first, if the socket is connected.
            //  We expect an ack back.
            //
            if (m_pcdsClient->bIsConnected())
            {
                try
                {
                    m_strmReply.Reset();
                    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                                << kCQCGWSrv::pszDTD
                                << L"<CQCGW:Msg><CQCGW:Disconnect/></CQCGW:Msg>"
                                << kCIDLib::EndLn;
                    SendMsg();
                    GetAck(8000);
                }

                catch(TError& errToCatch)
                {
                    if (facCQCKit().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                }
            }

            try
            {
                m_pcdsClient->Terminate(TTime::enctNowPlusSecs(2), kCIDLib::True);
            }

            catch(TError& errToCatch)
            {
                if (facCQCKit().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }

            try
            {
                delete m_pcdsClient;
            }

            catch(TError& errToCatch)
            {
                if (facCQCKit().bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }
        }

        catch(...)
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"A system exception occured during socket cleanup"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        m_pcdsClient = nullptr;
    }

    // Clear up any connection oriented stuff
    m_colFields.RemoveAll();
}


//
//  Query an image from the media cache on the machine where the GW server is
//  running.
//
tCIDLib::ELoadRes
TCQCGWSrvClient::eQueryMediaArt(const   TString&                strRepoMoniker
                                , const TString&                strUID
                                , const TString&                strCurPerId
                                , const tCQCMedia::EMediaTypes  eMType
                                , const tCIDLib::TBoolean       bLarge
                                ,       tCIDLib::TCard4&        c4Bytes
                                ,       TMemBuf&                mbufToFill
                                ,       TString&                strNewPerId)
{
    // Assume no bytes until provent otherwise, to be safe
    c4Bytes = 0;

    // Make sure they've called Connect()
    CheckConnected();

    tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::NoNewData;
    c4Bytes = 0;

    // Build up the message and send it
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:QueryMediaArt CQCGW:Moniker='" << strRepoMoniker
                << L"' CQCGW:SID='" << strCurPerId
                << L"' CQCGW:UID='" << strUID
                << L"' CQCGW:MType='" << tCQCMedia::strXlatEMediaTypes(eMType)
                << L"' CQCGW:Size='" << (bLarge ? L"Lrg" : L"Sml")
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // And wait for the reply, which should be a media image data reply
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:MediaImgData", 10000);

    // Check the result. If no changes or failed, we an stop now
    const TString& strResult = xtnodeRes.xtattrNamed(L"CQCGW:Result").strValue();
    if (strResult.bCompareI(L"NoChanges"))
        return tCIDLib::ELoadRes::NoNewData;
    if (strResult.bCompareI(L"Failed"))
        return tCIDLib::ELoadRes::NotFound;

    //
    //  We should have image data at this point, but check just in case and for
    //  and unknown image type.
    //
    const TString& strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    const TString& strImgData = xtnodeRes.xtnodeChildAtAsText(0).strText();
    if (strImgData.bIsEmpty() || (strType == L"Unknown"))
        return tCIDLib::ELoadRes::NotFound;

    // We got something, so be sure to give back the size specific persistent id
    strNewPerId = xtnodeRes.xtattrNamed(L"CQCGW:SID").strValue();

    //
    //  We just return the raw data to the caller. The data is base64 encoded
    //  binary data. So we need to set up an input stream on the data
    //  value, and and output binary stream on the caller's buffer. Then we
    //  can use a Base64 decoder to copy it over.
    //
    TTextStringInStream strmIn(&strImgData);
    TBinMBufOutStream   strmOut(&mbufToFill);
    TBase64 b64Decode;
    b64Decode.Decode(strmIn, strmOut);

    strmOut.Flush();
    c4Bytes = strmOut.c4CurSize();

    return tCIDLib::ELoadRes::NewData;
}


//
//  Downloads a media database, if newer data is available. Pass in an empty
//  serial number initially. Pass back in the previous serial number for subsequent
//  queries. If changes have been made, new data and a new serial number will be
//  returned.
//
tCIDLib::ELoadRes
TCQCGWSrvClient::eQueryMediaDB( const   TString&            strRepoMon
                                , const TString&            strSerialNum
                                ,       TString&            strNewSerialNum
                                ,       THeapBuf&           mbufData
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       TString&            strErrMsg)
{
    // Make sure they've called Connect()
    CheckConnected();

    tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::NoNewData;
    c4Bytes = 0;

    // Build up the message and send it
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:QueryMediaDB CQCGW:Moniker='" << strRepoMon
                << L"' CQCGW:DBSerialNum='" << strSerialNum
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // And wait for the reply, which should be a media DB info msg
    const TXMLTreeElement& xtnodeDB = xtnodeGetMsg(L"CQCGW:MediaDBInfo", 10000);

    // Get out the main info
    strNewSerialNum = xtnodeDB.xtattrNamed(L"CQCGW:DBSerialNum").strValue();
    const TString& strResult = xtnodeDB.xtattrNamed(L"CQCGW:Result").strValue();

    // Set up the result return value
    if (strResult == L"Failed")
    {
        eRes = tCIDLib::ELoadRes::NotFound;

        // The body text is an error messge
        strErrMsg = xtnodeDB.xtnodeChildAtAsText(0).strText();
    }
     else if (strResult == L"NewData")
    {
        eRes = tCIDLib::ELoadRes::NewData;

        const TString& strImgData = xtnodeDB.xtnodeChildAtAsText(0).strText();

        //
        //  We just return the raw data to the caller. The data is base64 encoded
        //  binary data. So we need to set up an input stream on the data
        //  value, and and output binary stream on the caller's buffer. Then we
        //  can use a Base64 decoder to copy it over.
        //
        THeapBuf mbufXML(0x200000, 0xA00000);
        {
            TTextStringInStream strmIn(&strImgData);
            TBinMBufOutStream strmOut(&mbufXML);
            TBase64 b64Decode;
            b64Decode.Decode(strmIn, strmOut);

            strmOut.Flush();
            c4Bytes = strmOut.c4CurSize();
        }

        // Now we need to take that and decompress it to the caller's buffer
        TZLibCompressor zlibComp;
        TBinMBufInStream strmSrc(&mbufXML, c4Bytes);
        TBinMBufOutStream strmTar(&mbufData);
        c4Bytes = zlibComp.c4Decompress(strmSrc, strmTar);
    }
     else if (strResult == L"NoChanges")
    {
        eRes = tCIDLib::ELoadRes::NoNewData;
    }
    return eRes;
}


//
//  Return the user role. This value is meaningless if we are not currently
//  connected.
//
tCQCKit::EUserRoles TCQCGWSrvClient::eUserRole() const
{
    return m_eUserRole;
}


// Provide access to the field info objects
const TCQCGWSrvClient::TFieldInfo&
TCQCGWSrvClient::fldiAt(const tCIDLib::TCard4 c4At) const
{
    // Make sure they've called Connect()
    CheckConnected();

    // Make sure the index is valid
    if (c4At >= m_colFields.c4ElemCount())
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , TString(L"XMLGWClient")
            , TCardinal(m_colFields.c4ElemCount())
        );
    }

    return m_colFields[c4At];
}


// Pause/unpause a scheduled event by id
tCIDLib::TVoid
TCQCGWSrvClient::PauseSchEv(const   TString&            strId
                            , const tCIDLib::TBoolean   bState)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:PauseSchEv CQCGW:EvId='" << strId
                << L"' CQCGW:State='" << (bState ? L"Paused" : L"Resumed")
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send and we should get an ack or error back
    SendMsg();
    GetAck(10000);
}


// Just sends a ping and gets an ack
tCIDLib::TVoid TCQCGWSrvClient::Ping()
{
    SendQueryOp(L"CQCGW:Ping");
    GetAck(10000);
}



//
//  Query a list of drivers available, and a parallel list of their device
//  categories.
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryDrivers(  tCIDLib::TStrList&      colMonikers
                                , tCQCKit::TDevCatList& fcolCats)
{
    // We send a generic query message in this case
    SendQueryOp(L"CQCGW:QueryDrvList");

    // And wait for the reply, which should be a device list
    const TXMLTreeElement& xtnodeList = xtnodeGetMsg(L"CQCGW:DeviceList", 10000);

    // Go through them and pull out the info
    colMonikers.RemoveAll();
    fcolCats.RemoveAll();

    tCQCKit::EDevCats eDevCat;
    const tCIDLib::TCard4 c4DevCnt = xtnodeList.c4ChildCount();
    for (tCIDLib::TCard4 c4DevInd = 0; c4DevInd < c4DevCnt; c4DevInd++)
    {
        // Get the current device child and get a ref to the moniker
        const TXMLTreeElement& xtnodeDev = xtnodeList.xtnodeChildAtAsElement(c4DevInd);
        const TString& strDevMon = xtnodeDev.xtattrNamed(L"CQCGW:Moniker").strValue();

        eDevCat = tCQCKit::eXlatEDevCats
        (
            xtnodeDev.xtattrNamed(L"CQCGW:DevCat").strValue()
        );

        // If for some reason it's wrong, set it to misc
        if (eDevCat == tCQCKit::EDevCats::Count)
            eDevCat = tCQCKit::EDevCats::Miscellaneous;

        colMonikers.objAdd(strDevMon);
        fcolCats.c4AddElement(eDevCat);
    }
}


tCIDLib::TVoid
TCQCGWSrvClient::QueryDrvInfo(  const   TString&                strMoniker
                                ,       tCIDLib::TBoolean&      bOnline
                                ,       tCIDLib::TCard4&        c4ArchVer
                                ,       TString&                strMake
                                ,       TString&                strModel)
{
    // Build up the message
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:QueryDriverInfo CQCGW:Moniker='" << strMoniker
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send and we should get a field info reply or an error if not found
    SendMsg();
    const TXMLTreeElement& xtnodeInfo = xtnodeGetMsg(L"CQCGW:DriverInfo", 10000);

    strMake = xtnodeInfo.xtattrNamed(L"CQCGW:Make").strValue();
    strModel = xtnodeInfo.xtattrNamed(L"CQCGW:Model").strValue();
    bOnline = xtnodeInfo.xtattrNamed(L"CQCGW:Model").strValue() == L"Online";
    c4ArchVer = xtnodeInfo.xtattrNamed(L"CQCGW:Model").c4ValueAs();
}


// Query info on a single field
tCIDLib::TVoid
TCQCGWSrvClient::QueryFieldInfo(const TString& strMonFld, TCQCFldDef& flddToFill)
{
    // Make sure it's a valid moniker.field type format
    TString strMon;
    TString strFld;
    facCQCKit().ParseFldName(strMonFld, strMon, strFld);

    // Build up the message
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:QueryFldInfo CQCGW:Field='" << strMonFld
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send and we should get a field info reply or an error if not found
    SendMsg();
    const TXMLTreeElement& xtnodeFld = xtnodeGetMsg(L"CQCGW:FldInfo", 10000);

    // Pull the info out and set up the caller's field def
    TString strAcc = L"EFldAccess_";
    strAcc.Append(xtnodeFld.xtattrNamed(L"CQCGW:Access").strValue());
    TString strSem = L"EFldSType_";
    strSem.Append(xtnodeFld.xtattrNamed(L"CQCGW:SemType").strValue());

    flddToFill.Set
    (
        strFld
        , eXlatFldType(xtnodeFld.xtattrNamed(L"CQCGW:Type").strValue())
        , tCQCKit::eXlatEFldAccess(strAcc)
        , tCQCKit::eXlatEFldSTypes(strSem)
        , xtnodeFld.xtattrNamed(L"CQCGW:Limits").strValue()
    );
}


// Get a list of all drivers and their field definitions
tCIDLib::TVoid
TCQCGWSrvClient::QueryFields(TNamedValMap<TCQCFldDef>& colToFill)
{
    // We send a generic query message in this case
    SendQueryOp(L"CQCGW:QueryFieldList");

    // And wait for the reply, which should be a device list
    const TXMLTreeElement& xtnodeList = xtnodeGetMsg(L"CQCGW:DeviceList", 10000);

    //
    //  For each device we add a top level entry to the map, and then iterate
    //  all of the fields for that device and add second level entries.
    //
    colToFill.Reset();
    TCQCFldDef flddNew;
    TString    strAcc;
    TString    strSem;
    const tCIDLib::TCard4 c4DevCnt = xtnodeList.c4ChildCount();
    for (tCIDLib::TCard4 c4DevInd = 0; c4DevInd < c4DevCnt; c4DevInd++)
    {
        // Get the current device child and get a ref to the moniker
        const TXMLTreeElement& xtnodeDev = xtnodeList.xtnodeChildAtAsElement(c4DevInd);
        const TString& strDevMon = xtnodeDev.xtattrNamed(L"CQCGW:Moniker").strValue();

        // Add a top level entry for this guy
        colToFill.AddItem(strDevMon);

        // Now let's do the children of this guy
        const tCIDLib::TCard4 c4FldCnt = xtnodeDev.c4ChildCount();
        for (tCIDLib::TCard4 c4FldInd = 0; c4FldInd < c4FldCnt; c4FldInd++)
        {
            // Get the current field def
            const TXMLTreeElement& xtnodeFld = xtnodeDev.xtnodeChildAtAsElement(c4FldInd);
            const TString& strFldName = xtnodeFld.xtattrNamed(L"CQCGW:Name").strValue();

            // Load up the field def object with this one's info
            strAcc = L"EFldAccess_";
            strAcc.Append(xtnodeFld.xtattrNamed(L"CQCGW:AccessType").strValue());
            strSem = L"EFldSType_";
            strSem.Append(xtnodeFld.xtattrNamed(L"CQCGW:SemType").strValue());

            flddNew.Set
            (
                strFldName
                , eXlatFldType(xtnodeFld.xtattrNamed(L"CQCGW:Type").strValue())
                , tCQCKit::eXlatEFldAccess(strAcc)
                , tCQCKit::eXlatEFldSTypes(strSem)
                , xtnodeFld.xtattrNamed(L"CQCGW:Limits").strValue()
            );

            // And add this one as a second level entry
            colToFill.AddValue(strDevMon, strFldName, flddNew);
        }
    }
}



//
//  Get a tree that represents the scoped layout of the available global
//  actions.
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryGlobActs(TCQCGWSrvClient::TNameTreeCol& colToFill)
{
    // We send a generic query message in this case
    SendQueryOp(L"CQCGW:QueryGlobActs");

    // And wait for the reply, which should be a global actions scope msg
    const TXMLTreeElement& xtnodeList = xtnodeGetMsg(L"CQCGW:GActScope", 10000);

    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4Count = xtnodeList.c4ChildCount();
    if (c4Count)
    {
        //
        //  Add the initial non-terminal for the root, This makes things
        //  easier in the recursion below.
        //
        colToFill.RemoveAll();
        colToFill.pnodeCreateNTPath(L"/", TString::strEmpty());

        //
        //  This is a tree structure, so we need to pull it out and fill in
        //  the passed tree collection.
        //
        TString strCurPath(L"/");
        GetHierarchy
        (
            xtnodeList
            , colToFill
            , strCurPath
            , L"CQCGW:GActScope"
            , L"CQCGW:GActName"
        );
    }
}


//
//  Get a tree that represents the scoped layout of the available macros.
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryMacros(TCQCGWSrvClient::TNameTreeCol& colToFill)
{
    // We send a generic query message in this case
    SendQueryOp(L"CQCGW:QueryMacros");

    // And wait for the reply, which should be a macro scope msg
    const TXMLTreeElement& xtnodeList = xtnodeGetMsg(L"CQCGW:MacroScope", 10000);

    colToFill.RemoveAll();
    const tCIDLib::TCard4 c4Count = xtnodeList.c4ChildCount();
    if (c4Count)
    {
        //
        //  Add the initial non-terminal for the root, This makes things
        //  easier in the recursion below.
        //
        colToFill.RemoveAll();
        colToFill.pnodeCreateNTPath(L"/", TString::strEmpty());

        //
        //  This is a tree structure, so we need to pull it out and fill in
        //  the passed tree collection.
        //
        TString strCurPath(L"/");
        GetHierarchy
        (
            xtnodeList
            , colToFill
            , strCurPath
            , L"CQCGW:MacroScope"
            , L"CQCGW:MacroName"
        );
    }
}


//
//  Query a list of monikers of any media repository drivers in the system.
//
tCIDLib::TVoid TCQCGWSrvClient::QueryMediaRepoDrvs(tCIDLib::TStrCollect& colToFill)
{
    colToFill.RemoveAll();

    // We use the generic query message for this one
    SendQueryOp(L"CQCGW:QueryRepoDrvs");

    //
    //  We'll get back a CQCGW:DrvList reply, which will contain zero or more
    //  CQCGW:DrvMon elements.
    //
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:DrvList", 10000);

    const tCIDLib::TCard4 c4Count = xtnodeRes.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colToFill.objAdd
        (
            xtnodeRes.xtnodeChildAtAsElement(c4Index).xtattrNamed(L"CQCGW:Moniker").strValue()
        );
    }
}


//
//  Query the playlist of a media renderer. Currently the same XML GW msg
//  used to get a playlist or the items of a collection in a repo. We are
//  doing the playlist query, so we pass an empty collection cookie.
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryRendPL(const  TString&            strMoniker
                            ,       tCIDLib::TStrList&  colArtists
                            ,       tCIDLib::TStrList&  colNames
                            ,       tCIDLib::TStrList&  colCookies
                            ,       tCIDLib::TStrList&  colLocs
                            ,       tCIDLib::TCardList& fcolIds)
{
    // Build up the message.
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:QueryRendPL CQCGW:Moniker='" << strMoniker
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send and we should get a room cfg reply or an error if not found
    SendMsg();
    const TXMLTreeElement& xtnodeList = xtnodeGetMsg(L"CQCGW:RendPLInfo", 10000);

    colArtists.RemoveAll();
    colNames.RemoveAll();
    colCookies.RemoveAll();
    colLocs.RemoveAll();
    fcolIds.RemoveAll();

    // Now get the the two elements that contain the lists
    const TXMLTreeElement& xtnodeNames = xtnodeList.xtnodeChildAtAsElement(0);
    const TXMLTreeElement& xtnodeCookies = xtnodeList.xtnodeChildAtAsElement(1);
    const TXMLTreeElement& xtnodeLocs = xtnodeList.xtnodeChildAtAsElement(2);
    const TXMLTreeElement& xtnodeArtists = xtnodeList.xtnodeChildAtAsElement(3);
    const TXMLTreeElement& xtnodeIds = xtnodeList.xtnodeChildAtAsElement(5);

    //
    //  Go through the count of items returned and grab one item out of each
    //  list on each round.
    //
    const tCIDLib::TCard4 c4Count = xtnodeNames.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the appropriate elements from each child list
        const TXMLTreeElement& xtnodeName = xtnodeNames.xtnodeChildAtAsElement(c4Index);
        const TXMLTreeElement& xtnodeCookie = xtnodeCookies.xtnodeChildAtAsElement(c4Index);
        const TXMLTreeElement& xtnodeLoc = xtnodeLocs.xtnodeChildAtAsElement(c4Index);
        const TXMLTreeElement& xtnodeArtist = xtnodeArtists.xtnodeChildAtAsElement(c4Index);
        const TXMLTreeElement& xtnodeId = xtnodeIds.xtnodeChildAtAsElement(c4Index);

        colArtists.objAdd(xtnodeArtist.xtnodeChildAtAsText(0).strText());
        colCookies.objAdd(xtnodeCookie.xtnodeChildAtAsText(0).strText());
        colLocs.objAdd(xtnodeLoc.xtnodeChildAtAsText(0).strText());
        colNames.objAdd(xtnodeName.xtnodeChildAtAsText(0).strText());

        fcolIds.c4AddElement
        (
            xtnodeId.xtnodeChildAtAsText(0).strText().c4Val()
        );
    }
}


//
//  Query the configuration for a room. We fill in a room configuration object
//
tCIDLib::TVoid TCQCGWSrvClient::QueryRmCfg(const TString& strRmName)
{
    // Build up the message
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:RoomCfgReq CQCGW:Name='" << strRmName
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send and we should get a room cfg reply or an error if not found
    SendMsg();
    const TXMLTreeElement& xtnodeFld = xtnodeGetMsg(L"CQCGW:RoomCfgRep", 10000);
}



//
//  Query a list of rooms in the system configuration data.
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryRmCfgList(tCIDLib::TStrList& colToFill)
{
    colToFill.RemoveAll();

    // We use the generic query message for this one
    SendQueryOp(L"CQCGW:QueryRmCfgList");

    //
    //  We'll get back a CQCGW:RoomList reply, which will contain zero or more
    //  CQCGW:RoomInfo elements.
    //
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:RoomList", 10000);

    const tCIDLib::TCard4 c4Count = xtnodeRes.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Get the current room and then the name attribute of that
        colToFill.objAdd
        (
            xtnodeRes.xtnodeChildAtAsElement(c4Index).xtattrNamed(L"CQCGW:Name").strValue()
        );
    }
}



//
//  Query general info about a scheduled event by id
//
tCIDLib::TVoid
TCQCGWSrvClient::QuerySchEvInfo(const   TString&            strId
                                ,       TString&            strName
                                ,       tCIDLib::TBoolean&  bPaused
                                ,       TString&            strType)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QuerySchEv CQCGW:EvId='"
                << strId << L"'></CQCGW:QuerySchEv>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    //
    //  We will get back one of three different types of elements. But the
    //  info that we return is common to them all so we don't have to
    //  bother to check. We have to use the version of the message getter
    //  that doesn't check the return type.
    //
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(10000);

    strName = xtnodeRes.xtattrNamed(L"CQCGW:Name").strValue();
    strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    bPaused = xtnodeRes.xtattrNamed(L"CQCGW:State").strValue() == L"Paused";
}


//
//  Query general type specific details on a scheduled event by id
//
tCIDLib::TVoid
TCQCGWSrvClient::QueryPerEv(const   TString&            strId
                            ,       TString&            strName
                            ,       tCIDLib::TBoolean&  bPaused
                            ,       TString&            strType
                            ,       tCIDLib::TCard4&    c4Period)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QuerySchEv CQCGW:EvId='"
                << strId << L"'></CQCGW:QuerySchEv>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // We should get back a periodic event info type element
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:PerEvInfo", 10000);

    strName = xtnodeRes.xtattrNamed(L"CQCGW:Name").strValue();
    strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    bPaused = xtnodeRes.xtattrNamed(L"CQCGW:State").strValue() == L"Paused";
    c4Period = xtnodeRes.xtattrNamed(L"CQCGW:Period").c4ValueAs();
}

tCIDLib::TVoid
TCQCGWSrvClient::QuerySchEv(const   TString&            strId
                            ,       TString&            strName
                            ,       tCIDLib::TBoolean&  bPaused
                            ,       TString&            strType
                            ,       TString&            strMask
                            ,       tCIDLib::TCard4&    c4Day
                            ,       tCIDLib::TCard4&    c4Hour
                            ,       tCIDLib::TCard4&    c4Min)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QuerySchEv CQCGW:EvId='"
                << strId << L"'></CQCGW:QuerySchEv>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // We should get back a scheduled event info type element
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:SchEvInfo", 10000);

    strName = xtnodeRes.xtattrNamed(L"CQCGW:Name").strValue();
    strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    bPaused = xtnodeRes.xtattrNamed(L"CQCGW:State").strValue() == L"Paused";
    strMask = xtnodeRes.xtattrNamed(L"CQCGW:Mask").strValue();

    // We have to parse out the DHM values
    const TString& strDHM = xtnodeRes.xtattrNamed(L"CQCGW:DHM").strValue();
    if (!TCQCSchEvent::bParseDHM(strDHM, c4Day, c4Hour, c4Min))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvSys_BadTimeInfo
            , xtnodeRes.xtattrNamed(L"CQCGW:DHM").strValue()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
}

tCIDLib::TVoid
TCQCGWSrvClient::QuerySunEv(const   TString&            strId
                            ,       TString&            strName
                            ,       tCIDLib::TBoolean&  bPaused
                            ,       TString&            strType
                            ,       tCIDLib::TInt4&     i4Offset)
{
    // Build up the message, passing along the serial number
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:QuerySchEv CQCGW:EvId='"
                << strId << L"'></CQCGW:QuerySchEv>\n</CQCGW:Msg>"
                << kCIDLib::EndLn;
    SendMsg();

    // We should get back a sun based event info type element
    const TXMLTreeElement& xtnodeRes = xtnodeGetMsg(L"CQCGW:SunEvInfo", 10000);

    strName = xtnodeRes.xtattrNamed(L"CQCGW:Name").strValue();
    strType = xtnodeRes.xtattrNamed(L"CQCGW:Type").strValue();
    bPaused = xtnodeRes.xtattrNamed(L"CQCGW:State").strValue() == L"Paused";
    i4Offset = xtnodeRes.xtattrNamed(L"CQCGW:Offset").i4ValueAs();
}


//
//  Allow the client to change the configuration of a scheduled event of
//  each general type.
//
tCIDLib::TVoid
TCQCGWSrvClient::SetPerEv(  const   TString&                strId
                            , const tCIDLib::TCard4         c4Period
                            , const tCIDLib::TEncodedTime   enctStart)
{
    //
    //  Build up a new message to set the poll list. We'll put in all of the
    //  fields that are in the current field list.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version='1.0' encoding='utf_8'?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:SetPerEv CQCGW:EvId='" << strId
                << L"' CQCGW:Period='" << c4Period
                << L"' CQCGW:FromTime='" << enctStart
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send it and we should get back an ack or nack
    SendMsg();
    GetAck(10000);
}

tCIDLib::TVoid
TCQCGWSrvClient::SetSchEv(  const   TString&        strId
                            , const TString&        strMask
                            , const tCIDLib::TCard4 c4Day
                            , const tCIDLib::TCard4 c4Hour
                            , const tCIDLib::TCard4 c4Minute)
{
    //
    //  Build up a new message to set the poll list. We'll put in all of the
    //  fields that are in the current field list.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:SetSchEv CQCGW:EvId='" << strId
                << L"' CQCGW:Mask='" << strMask
                << L"' CQCGW:DHM='" << c4Day << L' ' << c4Hour << L' ' << c4Minute
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send it and we should get back an ack or nack
    SendMsg();
    GetAck(10000);
}

tCIDLib::TVoid
TCQCGWSrvClient::SetSunEv(const TString& strId, const tCIDLib::TInt4  i4Offset)
{
    //
    //  Build up a new message to set the poll list. We'll put in all of the
    //  fields that are in the current field list.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"  <CQCGW:SetSunEv CQCGW:EvId='" << strId
                << L"' CQCGW:Offset='" << i4Offset
                << L"'/></CQCGW:Msg>"
                << kCIDLib::EndLn;

    // Send it and we should get back an ack or nack
    SendMsg();
    GetAck(10000);
}


//
//  Let's the client code set a new list of fields that it will poll, or
//  to empty any outstanding list by passing an empty list.
//
tCIDLib::TVoid
TCQCGWSrvClient::SetPollList(const tCIDLib::TStrCollect& colList)
{
    //
    //  Build up a new message to set the poll list. We'll put in all of the
    //  fields that are in the current field list.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf_8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:SetPollList>\n";

    TColCursor<TString>* pcursFields = colList.pcursNew();
    TJanitor<TColCursor<TString> > janCurs(pcursFields);
    if (pcursFields->bReset())
    {
        do
        {
            m_strmReply << L"         <CQCGW:FldName CQCGW:Name=\""
                        << pcursFields->objRCur() << L"\"/>\n";
        }   while (pcursFields->bNext());
    }

    // Close out the message and send it
    m_strmReply << L"    </CQCGW:SetPollList>\n</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    //  We have to get an ack (or perhaps a nak) back
    GetAck(10000);

    // It worked so store our new field list, initially all marked as offline
    m_colFields.RemoveAll();
    pcursFields->bReset();
    do
    {
        m_colFields.objAdd(TFieldInfo(pcursFields->objRCur()));
    }   while (pcursFields->bNext());
}


//
//  For writing a value to an arbitrary field, or to one of the fields on
//  the poll list, which is more efficient.
//
tCIDLib::TVoid
TCQCGWSrvClient::WriteField(const   TString&    strMoniker
                            , const TString&    strField
                            , const TString&    strValue)
{
    // Build up the message to run a macro and send it
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                   L"    <CQCGW:WriteField CQCGW:Field=\""
                << strMoniker << L"." << strField
                << L"\" CQCGW:Value=\"";

    // We need to escape the value, which could have illegal chars
    facCIDXML().EscapeFor(strValue, m_strmReply, tCIDXML::EEscTypes::Attribute);

    m_strmReply << L"\"/>\n" << L"</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    // And we should get back either an ack or an exception
    GetAck(5000);
}


//
//  For writing multiple fields at once. We pass through a list of monikers,
//  fields, and values. They must be the same size and each slice through
//  them must refer to a single write.
//
tCIDLib::TVoid
TCQCGWSrvClient::MWriteField(const  tCIDLib::TStrList&  colMonikers
                            , const tCIDLib::TStrList&  colFields
                            , const tCIDLib::TStrList&  colValues)
{
    // Validate that they are the same size
    const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();

    if ((colFields.c4ElemCount() != c4Count)
    ||  (colValues.c4ElemCount() != c4Count))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_ListSizes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    // Build up the message to run a macro and send it
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                   L"    <CQCGW:MWriteField>\n";

    // For each of the values, put out a write field element
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_strmReply << L"        <CQCGW:WriteField CQCGW:Field=\""
                    << colMonikers[c4Index] << L"." << colFields[c4Index]
                    << L"\" CQCGW:Value=\"";

        // We need to escape the value, which could have illegal chars
        facCIDXML().EscapeFor
        (
            colValues[c4Index], m_strmReply, tCIDXML::EEscTypes::Attribute
        );
        m_strmReply << L"\"/>\n";
    }

    m_strmReply << L"</CQCGW:MWriteField></CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg();

    // And we should get back either an ack or an exception
    GetAck(5000);
}


// ---------------------------------------------------------------------------
//  TCQCGWSrvClient: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A convenience to wait for an ack or nak, which is fairly commonly done
//  from multiple places. It just calls bGetMsg() and then checks the reply.
/// If it's not an ack or nak, then an exception is thrown.
//
tCIDLib::TBoolean
TCQCGWSrvClient::bGetAckNak(        TString&        strReplyInfo
                            , const tCIDLib::TCard4 c4WaitFor)
{
    // Call the other version
    if (!bGetMsg(c4WaitFor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"ack/nak")
        );
    }

    // Make sure its' the ack or nack message
    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

    tCIDLib::TBoolean bRet;
    if (xtnodeMsg.strQName() == "CQCGW:AckReply")
    {
        // Get out the info string
        strReplyInfo = xtnodeMsg.xtattrNamed(L"CQCGW:Info").strValue();
        bRet = kCIDLib::True;
    }
     else if (xtnodeMsg.strQName() == "CQCGW:NakReply")
    {
        // Get out the reason string
        strReplyInfo = xtnodeMsg.xtattrNamed(L"CQCGW:ReasonText").strValue();
        bRet = kCIDLib::False;
    }
     else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_ExpectedMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"ack/nak")
            , xtnodeMsg.strQName()
        );
    }
    return bRet;
}


//
//  Wait for up to the indicated number of milliseconds for a message to
//  come in from the server. If we timeout, we return false. If we get a
//  message, we return true (and the message is in the tree parser), else
//  some error occurred and we throw.
//
tCIDLib::TBoolean TCQCGWSrvClient::bGetMsg(const tCIDLib::TCard4 c4Wait)
{
    // Make sure we are connected
    CheckConnected();

    // Wait for up to the indicated time for data to arrive
    if (!m_pcdsClient->bDataAvailMS(c4Wait))
        return kCIDLib::False;

    // Set up the flags and options we'll use for parsing
    const tCIDXML::EParseFlags eFlags = tCIDXML::EParseFlags::TagsNText;
    const tCIDXML::EParseOpts  eOpts  = tCIDXML::EParseOpts::Validate;

    //
    //  Read a packet header first. This will tell us how much more data we
    //  have to read to get the data. If we don't get a full header, it will
    //  throw.
    //
    const tCIDLib::TCard4 c4Bytes = m_pcdsClient->c4ReadBytes
    (
        &m_hdrCur, sizeof(tCQCGWSrv::TPacketHdr), TTime::enctNowPlusSecs(1), kCIDLib::True
    );
    if (!c4Bytes)
        return kCIDLib::False;

    // Make sure that the packet header is basically sound
    CheckHeader(m_hdrCur);

    //
    //  Read the indicated number of bytes. Again, it will throw if we don't
    //  get what we expect.
    //
    tCIDLib::TCard2 c2TestSum;
    m_pcdsClient->c4ReadBytes
    (
        *m_pmbufData, m_hdrCur.c4DataSize, TTime::enctNowPlusSecs(2), kCIDLib::True
    );
    c2TestSum = tCIDLib::TCard2(m_pmbufData->c4CheckSum(0, m_hdrCur.c4DataSize));

    if (c2TestSum != m_hdrCur.c2CheckSum)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadCheckSum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  Ok, let's parse the XML using the XML tree parser, which will give us
    //  an XML document structure. Update the entity source with the data
    //  bytes that are now in the buffer.
    //
    //  We have to set up an entity source reference on it before we pass
    //  it in.
    //
    tCIDXML::TEntitySrcRef esrMsg
    (
        new TMemBufEntitySrc(L"XMLGWOp", m_cptrBuf, m_hdrCur.c4DataSize)
    );
    if (!m_pxtprsMsgs->bParseRootEntity(esrMsg, eOpts, eFlags))
    {
        // Display the first error
        const TXMLTreeParser::TErrInfo& erriToShow = m_pxtprsMsgs->erriFirst();
        TString strErr(erriToShow.strText().c4Length() + 32UL);
        strErr.Append(L'[');
        strErr.AppendFormatted(erriToShow.c4Line());
        strErr.Append(L',');
        strErr.AppendFormatted(erriToShow.c4Column());
        strErr.Append(L"] - ");
        strErr.Append(erriToShow.strText());

        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_ParseFailed
            , strErr
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    // Get the root node to start parsing from
    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();

    //
    //  If it's an exception message, then build an exception from the info
    //  and throw it.
    //
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);
    if (xtnodeMsg.strQName() == L"CQCGW:ExceptionReply")
    {
        const TXMLTreeText& xtnodeText = xtnodeMsg.xtnodeChildAtAsText(0);
        facCQCKit().ThrowErr
        (
            xtnodeMsg.xtattrNamed(L"CQCGW:File").strValue()
            , xtnodeMsg.xtattrNamed(L"CQCGW:Line").c4ValueAs()
            , kKitErrs::errcXGWC_ServerExcept
            , xtnodeText.strText()
            , tCIDLib::eXlatESeverities(xtnodeMsg.xtattrNamed(L"CQCGW:ErrSev").strValue())
            , tCIDLib::eXlatEErrClasses(xtnodeMsg.xtattrNamed(L"CQCGW:ErrClass").strValue())
        );
    }

    // If a nack, show the reason text
    if (xtnodeMsg.strQName() == L"CQCGW:NakReply")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_NakFromGW
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Nak
            , xtnodeMsg.xtattrNamed(L"CQCGW:ReasonText").strValue()
        );
    }
    return kCIDLib::True;
}


//
//  Looks up the passed field in our field list, and throws if it is not
//  found if they ask us to, else we return max card if not found.
//
tCIDLib::TCard4
TCQCGWSrvClient::c4FindFld( const   TString&            strToFind
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colFields.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colFields[c4Index].m_strName == strToFind)
            return c4Index;
    }

    if (bThrowIfNot)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_FldNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strToFind
        );
    }
    return kCIDLib::c4MaxCard;
}


//
//  Called to make sure we are connected before attempting various things.
//  We can't make them connect in the ctor, so we have to deal with the
//  possibility of calling something before Connect().
//
tCIDLib::TVoid TCQCGWSrvClient::CheckConnected() const
{
    if (!m_pcdsClient)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_NotConnected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}


//
//  Does some basic checks on the header of a message received from the
//  the server, and throws if anything is obviously wrong.
//
tCIDLib::TVoid
TCQCGWSrvClient::CheckHeader(const tCQCGWSrv::TPacketHdr& hdrToCheck) const
{
    if ((hdrToCheck.c4MagicVal1 != kCQCGWSrv::c4MagicVal1)
    ||  (hdrToCheck.c4MagicVal2 != kCQCGWSrv::c4MagicVal2))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadMagicVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    if (hdrToCheck.c4DataSize > kCQCGWSrv::c4MaxDataBytes)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadDataBytes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(hdrToCheck.c4DataSize)
        );
    }
}


//
//  Translate the field type represntations used by the gateway server to the
//  enum type. For stupid historical reasons it uses the CML class types, so we
//  can't use the usual means.
//
tCQCKit::EFldTypes TCQCGWSrvClient::eXlatFldType(const TString& strToXlat) const
{
    if (strToXlat == L"MEng.Boolean")
        return tCQCKit::EFldTypes::Boolean;
    else if (strToXlat == L"MEng.Card4")
        return tCQCKit::EFldTypes::Card;
    else if (strToXlat == L"MEng.Float8")
        return tCQCKit::EFldTypes::Float;
    else if (strToXlat == L"MEng.Int4")
        return tCQCKit::EFldTypes::Int;
    else if (strToXlat == L"MEng.String")
        return tCQCKit::EFldTypes::String;
    else if (strToXlat == L"MEng.StringList")
        return tCQCKit::EFldTypes::StringList;
    else if (strToXlat == L"MEng.Time")
        return tCQCKit::EFldTypes::Time;

    return tCQCKit::EFldTypes::Count;
}


// Waits for a reply and insures it's an ack reply, else it throws
tCIDLib::TVoid TCQCGWSrvClient::GetAck(const tCIDLib::TCard4 c4WaitFor)
{
    // Call the other version
    if (!bGetMsg(c4WaitFor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"ack")
        );
    }

    // Make sure its' the ack
    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

    if (xtnodeMsg.strQName() != "CQCGW:AckReply")
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_ExpectedMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"ack")
            , xtnodeMsg.strQName()
        );
    }
}


//
//  The queries that get back a tree of scopes and names of things within
//  those scopes can call this to break out the info into a tree collection
//  for return. This guy is recursive. We call back on ourself any time we
//  see a scope, so the element we get is always a scope.
//
tCIDLib::TVoid
TCQCGWSrvClient::GetHierarchy(  const   TXMLTreeElement&        xtnodeScope
                                ,       TBasicTreeCol<TString>& colToFill
                                ,       TString&                strCurPath
                                , const TString&                strScopeElem
                                , const TString&                strNameElem)
{
    // Remember the current path length so we can clip it back again later
    const tCIDLib::TCard4 c4PathLen = strCurPath.c4Length();

    //
    //  Let's search all this scope's children. For names we'll add them at
    //  this level. For scopes, we'll recurse.
    //
    const tCIDLib::TCard4 c4Count = xtnodeScope.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLTreeElement& xtnodeCur
                            = xtnodeScope.xtnodeChildAtAsElement(c4Index);

        const TString& strCurName = xtnodeCur.xtattrNamed(L"CQCGW:Name").strValue();
        if (xtnodeCur.strQName() == strScopeElem)
        {
            // Add this as a new scope
            colToFill.pnodeAddNonTerminal(strCurPath, strCurName);

            // Now append to the current path and recurse
            strCurPath.Append(kCIDLib::chForwardSlash);
            strCurPath.Append(strCurName);
            GetHierarchy
            (
                xtnodeCur
                , colToFill
                , strCurPath
                , strScopeElem
                , strNameElem
            );

            // Prune the scope back to our level again
            strCurPath.CapAt(c4PathLen);
        }
         else
        {
            //
            //  We just put an empty string in as the node data since
            //  we don't have anything to store as actual node data.
            //
            colToFill.objAddTerminal
            (
                strCurPath, strCurName, TString::strEmpty()
            );
        }
    }
}


//
//  Does the logon challenge exchange
//
tCIDLib::TVoid TCQCGWSrvClient::HandleLogonChallenge(const TXMLTreeElement& xtnodeChal)
{
    const tCIDLib::TCard4 c4Len = 16;

    // Get the values from the key and data attributes
    const TString& strKey = xtnodeChal.xtattrNamed(L"CQCGW:ChallengeKey").strValue();
    const TString& strData = xtnodeChal.xtattrNamed(L"CQCGW:ChallengeData").strValue();
    const TString& strSKey = xtnodeChal.xtattrNamed(L"CQCGW:SessKey").strValue();

    //
    //  Check the lengths. They should be 16 bytes, each represented by two
    //  hex digits, so 32 characters in all.
    //
    if ((strKey.c4Length() != c4Len * 2)
    ||  (strData.c4Length() != c4Len * 2))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadChallengeData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  Create a crypto key using the password we were given. We first create
    //  an MD5 hash of the password. This will convert it to UTF-8 format,
    //  minus any BOM, and use the resulting bytes as the input to the MD5
    //  hash. Then create a Blowfish encrypter with that key.
    //
    TMD5Hash mhashPassword;
    {
        TMessageDigest5 mdigTmp;
        mdigTmp.DigestStr(m_strPassword);
        mdigTmp.Complete(mhashPassword);
    }
    TCryptoKey ckeyPassword(mhashPassword);
    TBlowfishEncrypter crypPW(ckeyPassword);

    // Convert the data string into a temp binary buffer
    TString strTmp;
    THeapBuf mbufTmp(c4Len, c4Len);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        strData.CopyOutSubStr(strTmp, c4Index * 2, 2);
        mbufTmp.PutCard1(tCIDLib::TCard1(strTmp.c4Val(tCIDLib::ERadices::Hex)), c4Index);
    }

    // Decrypt the temp buffer into the data buffer
    THeapBuf mbufData(c4Len, c4Len);
    if (crypPW.c4Decrypt(mbufTmp, mbufData, c4Len) != c4Len)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadChallengeData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    // And do the same for the key value
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        //
        //  Pull out two characters each time through, and convert that to a
        //  byte and store.
        //
        strKey.CopyOutSubStr(strTmp, c4Index * 2, 2);
        mbufTmp.PutCard1(tCIDLib::TCard1(strTmp.c4Val(tCIDLib::ERadices::Hex)), c4Index);
    }

    THeapBuf mbufKey(c4Len, c4Len);
    if (crypPW.c4Decrypt(mbufTmp, mbufKey, c4Len) != c4Len)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadChallengeData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    // And decrypt the session key and set up the session decrypter
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        strSKey.CopyOutSubStr(strTmp, c4Index * 2, 2);
        mbufTmp.PutCard1(tCIDLib::TCard1(strTmp.c4Val(tCIDLib::ERadices::Hex)), c4Index);
    }

    THeapBuf mbufSKey(c4Len, c4Len);
    if (crypPW.c4Decrypt(mbufTmp, mbufSKey, c4Len) != c4Len)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadSessKey
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }
    m_crypSess.SetNewKey(TCryptoKey(mbufSKey, c4Len));

    //
    //  Ok, now we can create a crypto key from the passed key data and a
    //  new encrypter based on that key.
    //
    TCryptoKey ckeyChallenge(mbufKey, c4Len);
    TBlowfishEncrypter crypChallenge(ckeyChallenge);

    //
    //  Now use that to reencrypt the challenge data buffer using the key
    //  we were given. This is what we send back.
    //
    if (crypChallenge.c4Encrypt(mbufData, mbufTmp, c4Len) != c4Len)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_BadChallengeData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  And format the resulting data back into the string format. We use a
    //  text stream and set a stream formatter on it that uses 2 char field
    //  width, zero fill char, right justified.
    //
    const TStreamFmt sfmtHexNum
    (
        2
        , 0
        , tCIDLib::EHJustify::Right
        , kCIDLib::chDigit0
    );
    TTextStringOutStream strmTmp(1024UL);
    strmTmp << sfmtHexNum;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        strmTmp << TCardinal(mbufTmp[c4Index], tCIDLib::ERadices::Hex);
    }
    strmTmp.Flush();

    //
    //  Now build and send the challenge reply. Force it not to encrypt
    //  this one, since the session isn't set up yet. We pass along any
    //  standard environmental runtime values that are set on this host.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"Utf-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:GetSecurityToken CQCGW:ResponseData=\""
                << strmTmp.strData() << L"\">\n";

    TString strName(L"CQC_EnvRTV0");
    TString strValue;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 9; c4Index++)
    {
        strName.AppendFormatted(c4Index + 1);
        if (TProcEnvironment::bFind(strName, strValue))
        {
            m_strmReply << L"        <CQCGW:EnvVar CQCGW:VarNum=\""
                        << (c4Index + 1)
                        << L"\">";
            facCIDXML().EscapeFor
            (
                strValue, m_strmReply, tCIDXML::EEscTypes::Attribute
            );
            m_strmReply << L"</CQCGW:EnvVar>\n";
        }

        // Get the index number back off of it
        strName.DeleteLast();
    }

    m_strmReply << L"</CQCGW:GetSecurityToken></CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg(kCIDLib::False);

    // And we should get back either an ack or nak reply
    if (bGetAckNak(strTmp, 6000))
    {
        // The return info has the user role in it
        m_eUserRole = tCQCKit::eXlatEUserRoles(strTmp);
    }
     else
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_LoginRejected
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , strTmp
        );
    }
}


//
//  Goes through the logon sequence
//
tCIDLib::TVoid TCQCGWSrvClient::LogOn()
{
    //
    //  Set up a login request op packet. We have to pass it the user logon
    //  name.
    //
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:LogonReq CQCGW:UserName=\""
                << m_strUserName << L"\"";

    // Finish it out and send.
    m_strmReply << L"/>\n</CQCGW:Msg>" << kCIDLib::EndLn;
    SendMsg(kCIDLib::False);

    //
    //  He is going to reply with a challenge, which has two text formatted
    //  buffers of binary data.
    //
    //  Or, it could be a nak reply if our user name is bad.
    //
    tCIDLib::TBoolean bGotExpected;
    const TXMLTreeElement& xtnodeChallenge = xtnodeGetMsg
    (
        L"CQCGW:LogonChallenge", 5000, bGotExpected
    );

    if (!bGotExpected)
    {
        if (xtnodeChallenge.strQName() == L"CQCGW:NakReply")
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcXGWC_LoginRejected
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , xtnodeChallenge.xtattrNamed(L"CQCGW:ReasonText").strValue()
            );
        }
         else
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcXGWC_ExpectedMsg
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TString(L"CQCGW:LogonChallenge")
                , xtnodeChallenge.strQName()
            );
        }
    }
     else
    {
        //
        //  Call a helper which does the challenge algorithm and sends back
        //  the reply. If it works, the server will tell us we are logged in.
        //  Else, he'll reject is as a bad password.
        //
        HandleLogonChallenge(xtnodeChallenge);
    }
}


//
//  Given a message formatted into the replay stream already, this method
//  will do the grunt work of setting up the header, handling any encryption,
//  and so forth and then sending the message.
//
tCIDLib::TVoid TCQCGWSrvClient::SendMsg(const tCIDLib::TBoolean bCanEncrypt)
{
    // Make sure they've called Connect()
    CheckConnected();

    // Just to be sure, flush the reply stream
    m_strmReply.Flush();

    //
    //  Convert the text to UTF-8 format now. We stream them to a text stream
    //  initially, so that we can display it if logging. But we need to send
    //  a UTF-8 flatted version of the text.
    //
    m_tcvtSend.c4ConvertTo
    (
        m_strmReply.strData().pszBuffer(), m_strmReply.strData().c4Length(), m_expbData
    );

    // Let's set up the packet header stuff that is the same either way
    tCQCGWSrv::TPacketHdr   hdrCur;
    hdrCur.c4MagicVal1 = kCQCGWSrv::c4MagicVal1;
    hdrCur.c4SeqNum = m_c4SeqNum++;
    hdrCur.c4MagicVal2 = kCQCGWSrv::c4MagicVal2;

    hdrCur.c4DataSize = m_expbData.c4Bytes();
    hdrCur.c2CheckSum = tCIDLib::TCard2(m_expbData.c4CheckSum());
    hdrCur.c1Flags    = 0;

    // Send the header, and then send the data
    m_pcdsClient->WriteRawBytes(&hdrCur, sizeof(hdrCur));
    m_pcdsClient->WriteRawBytes(m_expbData.pc1Buffer(), m_expbData.c4Bytes());
    m_pcdsClient->FlushOut(TTime::enctNowPlusSecs(2));
}


//
//  A helper that does the common operation of sending a query message, in
//  which the only thing that has to change is the query op being sent.
//
tCIDLib::TVoid TCQCGWSrvClient::SendQueryOp(const TString& strOpToSend)
{
    // Now build and send the challenge reply
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:Query "
                << L"CQCGW:QueryType=\"" << strOpToSend
                << L"\"/>\n</CQCGW:Msg>\n" << kCIDLib::FlushIt;
    SendMsg();
}


//
//  Some simple wrappers around the basic get message query. These will wait
//  for a specific expected message, and get the message node out of the
//  parsed message and return a reference to it. One will throw if it does
//  not get the expected message, another will just indicate whether it
//  did or not and let the caller decide what to do.
//
const TXMLTreeElement&
TCQCGWSrvClient::xtnodeGetMsg(  const   tCIDLib::TCh* const pszExpected
                                , const tCIDLib::TCard4     c4WaitFor
                                ,       tCIDLib::TBoolean&  bGotExpected)
{
    // Call the fundamental getter
    if (!bGetMsg(c4WaitFor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszExpected)
        );
    }

    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

    bGotExpected = (xtnodeMsg.strQName() == pszExpected);
    return xtnodeMsg;
}


const TXMLTreeElement&
TCQCGWSrvClient::xtnodeGetMsg(  const   tCIDLib::TCh* const pszExpected
                                , const tCIDLib::TCard4     c4WaitFor)
{
    // Call the fundamental getter
    if (!bGetMsg(c4WaitFor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszExpected)
        );
    }

    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

    if (xtnodeMsg.strQName() != pszExpected)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_ExpectedMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszExpected)
            , xtnodeMsg.strQName()
        );
    }
    return xtnodeMsg;
}

const TXMLTreeElement&
TCQCGWSrvClient::xtnodeGetMsg(const tCIDLib::TCard4 c4WaitFor)
{
    // Call the fundamental getter
    if (!bGetMsg(c4WaitFor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcXGWC_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(L"[any]")
        );
    }

    const TXMLTreeElement& xtnodeRoot = m_pxtprsMsgs->xtdocThis().xtnodeRoot();
    return xtnodeRoot.xtnodeChildAtAsElement(0);
}


