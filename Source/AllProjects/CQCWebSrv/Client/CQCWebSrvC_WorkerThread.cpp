//
// FILE NAME: CQCWebSrvC_WorkerThread.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This file implements the worker thread class. We spin up a set of these
//  and the listener thread drops incoming connection HTTP requests into a
//  queue that these threads block on.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CQCWebSrvC_CMLBinHandler_.hpp"
#include    "CQCWebSrvC_CQCImgHandler_.hpp"
#include    "CQCWebSrvC_EchoHandler_.hpp"
#include    "CQCWebSrvC_FileHandler_.hpp"



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

//
//  Note that using the CurrentUTC special time will cause the default format
//  for the m_tmHeader object to be the ctime format, which is what we need
//  to use.
//
TWorkerThread::TWorkerThread(const TString& strName) :

    TThread(strName)
    , m_mbufData(kCIDLib::c4Sz_64K, kCIDLib::c4Sz_8M, kCIDLib::c4Sz_32K)
    , m_mbufData2(kCIDLib::c4Sz_16K, kCIDLib::c4Sz_1M, kCIDLib::c4Sz_8K)
    , m_purlhEcho(nullptr)
    , m_purlhCQCImg(nullptr)
    , m_purlhCMLBin(nullptr)
    , m_purlhFile(nullptr)
    , m_strmOutput(kCIDLib::c4Sz_4K, kCIDLib::c4Sz_8M)
    , m_strServer(L"CQC Web Server")
    , m_tmHeader(tCIDLib::ESpecialTimes::CurrentUTC)
{
    //
    //  Make sure our line end type is what HTTP expects. This should be the default,
    //  but don't take chances.
    //
    m_strmOutput.eNewLineType(tCIDLib::ENewLineTypes::CRLF);
}

TWorkerThread::~TWorkerThread()
{
    // Clean up any URL handlers we faulted in
    try
    {
        delete m_purlhEcho;
        m_purlhEcho = nullptr;
    }

    catch(...)
    {
    }

    try
    {
        delete m_purlhCQCImg;
        m_purlhCQCImg = nullptr;
    }

    catch(...)
    {
    }

    try
    {
        delete m_purlhCMLBin;
        m_purlhCMLBin = nullptr;
    }

    catch(...)
    {
    }

    try
    {
        delete m_purlhFile;
        m_purlhFile = nullptr;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes TWorkerThread::eProcess()
{
    // Let our caller go
    Sync();

    tCIDLib::TBoolean   bSecure;
    TIPEndPoint         ipepClient;
    while (kCIDLib::True)
    {
        try
        {
            // Check for shutdown requests
            if (bCheckShutdownRequest())
                break;

            //
            //  Wait for a client to serve, if nothing, go back to the top
            //
            //  NOTE:   We are responsible for the socket at this point, until we get
            //          it stored away below into a data source.
            //
            TServerStreamSocket* psockSrv = nullptr;
            if (!facCQCWebSrvC().bWaitConn(psockSrv, bSecure, ipepClient))
                continue;

            // For our purposes, any Nagle buffering is not optimal
            psockSrv->bNagleOn(kCIDLib::False);

            //
            //  Create a data source over this socket, and then put that in a
            //  janitor. We have to create a data source dynamically, because it
            //  may end up having to be handed off to a web socket handler, which
            //  will run on a separate thread. We have to create different types of
            //  data sources for secure vs. non-secure clients.
            //
            TCIDSockStreamBasedDataSrc* pcdsSrv = nullptr;
            if (bSecure)
            {
                pcdsSrv = new TCIDSChanSrvDataSrc
                (
                    L"CQC Web Server"
                    , psockSrv
                    , tCIDLib::EAdoptOpts::Adopt
                    , facCQCWebSrvC().strCertInfo()
                    , tCIDSChan::EConnOpts::None
                );
            }
             else
            {
                pcdsSrv = new TCIDSockStreamDataSrc(psockSrv, tCIDLib::EAdoptOpts::Adopt);
            }

            //
            //  This will initialize the data source as well, and terminate it at the end
            //  if the source has not been orphaned away (which would be the case for
            //  a web socket connection.) In that case the web socket thread has to
            //  handle termination and cleaning up the source.
            //
            TCIDDataSrcJan janSrv(pcdsSrv, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True);

            try
            {
                ServiceClient(janSrv);
            }

            catch(const TError& errToCatch)
            {
                if (pcdsSrv)
                {
                    THTTPClient::SendErrReply
                    (
                        *pcdsSrv
                        , m_strServer
                        , kCIDNet::c4HTTPStatus_SrvError
                        , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_ExceptInReq)
                        , errToCatch.strErrText()
                    );
                }
            }

            catch(...)
            {
                if (pcdsSrv)
                {
                    THTTPClient::SendErrReply
                    (
                        *pcdsSrv
                        , m_strServer
                        , kCIDNet::c4HTTPStatus_SrvError
                        , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_UnknownExceptInReq)
                        , L"Unknown exception while processing request"
                    );
                }

                // <TBD> We should log this in debug mode perhaps?
            }
        }

        catch(const TError& errToCatch)
        {
            if (facCQCWebSrvC().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);

            if (facCQCWebSrvC().bLogFailures())
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerError
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (facCQCWebSrvC().bLogFailures())
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerError
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when we get a request for a secure URL, to check for
//  authentication info and to verify it if found.
//
//  The caller expects us to return OK, BadRequest, or Unauthorized, for
//  authenticated, bad auth data, or no auth data. If the data is stale,
//  then we return kCIDLib::c4MaxCard.
//
tCIDLib::TCard4
TWorkerThread::c4Authenticate(  const   TString&                strURLPath
                                , const TString&                strType
                                ,       TString&                strUserName
                                ,       tCQCKit::EUserRoles&    eRole)
{
    //
    //  There has to be an Authorization header value. If no, then not
    //  authorized.
    //
    TString strAuthData;
    const tCIDLib::TCard4 c4Count = m_colInHdrLines.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colInHdrLines[c4Index];
        if (kvalCur.strKey().bCompareI(L"Authorization"))
        {
            strAuthData = kvalCur.strValue();
            break;
        }
    }
    if (strAuthData.bIsEmpty())
        return kCIDNet::c4HTTPStatus_Unauthorized;

    // Put a tokenizer over the data and pull out tokens
    TStringTokenizer stokAuth(&strAuthData, L" ");

    // It should start with Digest
    TString strVal;
    if (!stokAuth.bGetNextToken(strVal) || !strVal.bCompare(L"Digest"))
        return kCIDNet::c4HTTPStatus_BadRequest;

    stokAuth.strWhitespace(L",");
    TString strCNonce;
    TString strNonce;
    TString strNonceCnt;
    TString strQOp;
    TString strRealm;
    TString strResponse;
    TString strURI;

    tCIDLib::TCard4 c4Found = 0;
    while (stokAuth.bGetNextToken(strVal))
    {
        strVal.StripWhitespace();
        if (strVal.bStartsWithI(L"cnonce="))
        {
            strVal.CopyOutSubStr(strCNonce, 7);
            strCNonce.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"username="))
        {
            strVal.CopyOutSubStr(strUserName, 9);
            strUserName.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"nonce="))
        {
            strVal.CopyOutSubStr(strNonce, 6);
            strNonce.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"nc="))
        {
            strVal.CopyOutSubStr(strNonceCnt, 3);
            strNonceCnt.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"qop="))
        {
            // This one is optional so don't bump count
            strVal.CopyOutSubStr(strQOp, 4);
            strQOp.Strip(L"'\"");
        }
         else if (strVal.bStartsWithI(L"realm="))
        {
            strVal.CopyOutSubStr(strRealm, 6);
            strRealm.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"response="))
        {
            strVal.CopyOutSubStr(strResponse, 9);
            strResponse.Strip(L"'\"");
            c4Found++;
        }
         else if (strVal.bStartsWithI(L"uri="))
        {
            strVal.CopyOutSubStr(strURI, 4);
            strURI.Strip(L"'\"");
            c4Found++;
        }
    }

    // If none of them, then we just need challenge them
    if (!c4Found)
        return kCIDNet::c4HTTPStatus_Unauthorized;

    // Make sure we got the ones we have to
    if (c4Found < 7)
        return kCIDNet::c4HTTPStatus_BadRequest;

    //
    //  Ok, we have to do the test. First, we have to generate our nonce, and
    //  see if the caller's nonce is now out of date.
    //
    TString strCurNonce;
    GenerateNonce(strURLPath, strCurNonce);
    if (strCurNonce != strNonce)
        return kCIDLib::c4MaxCard;

    // Ask the security server for web password/user role info
    TString strPassword;
    {
        // We have to pass our own security creds in order to get the web password.
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy(4000);
        tCIDLib::TCard4 c4Size = 0;
        if (!orbcSS->bQueryWAccount(strUserName, c4Size, m_mbufData, eRole, facCQCWebSrvC().sectUser()))
            return kCIDNet::c4HTTPStatus_Unauthorized;

        //
        //  We have to decrypt the password data if here is any. If it encrypted with the
        //  hash of the server account passowrd.
        //
        if (c4Size)
        {
            m_crypAuth.SetNewKey(TCryptoKey(facCQCWebSrvC().mhashSrvPW()));
            c4Size = m_crypAuth.c4Decrypt(m_mbufData, m_mbufData2, c4Size);

            // Now put a stream over the decrypted data and stream in the password
            TBinMBufInStream strmSrc(&m_mbufData2, c4Size);
            strmSrc >> strPassword;
        }
    }

    // If there is no web password set for this user, then reject
    if (strPassword.bIsEmpty())
        return kCIDNet::c4HTTPStatus_Unauthorized;

    // Ok, looks like all the bits are in place, so calc A1
    TString strA1;
    strA1 = strUserName;
    strA1.Append(kCIDLib::chColon);
    strA1.Append(strRealm);
    strA1.Append(kCIDLib::chColon);
    strA1.Append(strPassword);

    m_mdigAuth.StartNew();
    m_mdigAuth.DigestStr(strA1);
    m_mdigAuth.Complete(m_mhashAuth1);

    // And calc A2
    TString strA2;
    if ((strType == kCIDNet::pszHTTP_GET) || (strType == kCIDNet::pszHTTP_HEAD))
        strA2 = L"GET:";
    else
        strA2 = L"POST:";
    strA2.Append(strURI);

    m_mdigAuth.StartNew();
    m_mdigAuth.DigestStr(strA2);
    m_mdigAuth.Complete(m_mhashAuth2);

    //
    //  Now format both of them back out as raw hex strings, and lower
    //  case them since our format is all upper case and the Digest
    //  assumes lower.
    //
    m_mhashAuth1.RawFormat(strA1);
    m_mhashAuth2.RawFormat(strA2);
    strA1.ToLower();
    strA2.ToLower();

    // And build up a new combined string
    strA1.Append(kCIDLib::chColon);
    strA1.Append(strNonce);
    strA1.Append(kCIDLib::chColon);
    if (!strQOp.bIsEmpty())
    {
        strA1.Append(strNonceCnt);
        strA1.Append(kCIDLib::chColon);
        strA1.Append(strCNonce);
        strA1.Append(kCIDLib::chColon);
        strA1.Append(strQOp);
        strA1.Append(kCIDLib::chColon);
    }
    strA1.Append(strA2);

    // Now hash this string and again format it back out
    m_mdigAuth.StartNew();
    m_mdigAuth.DigestStr(strA1);
    m_mdigAuth.Complete(m_mhashAuth1);
    m_mhashAuth1.RawFormat(strA1);

    // And this should be equal to the response
    if (!strA1.bCompareI(strResponse))
        return kCIDNet::c4HTTPStatus_Unauthorized;

    return kCIDNet::c4HTTPStatus_OK;
}


//
//  If we need to challenge a client that is accessing a secure URL, this
//  method sends the challenge response.
//
tCIDLib::TVoid
TWorkerThread::SendAuthChallenge(       TCIDDataSrc&        cdsClient
                                , const TURL&               urlReq
                                , const TString&            strType
                                , const tCIDLib::TBoolean   bStale)
{
    // Update the time we use in headers
    m_tmHeader.SetTo(tCIDLib::ESpecialTimes::CurrentUTC);

    m_strmOutput.Reset();
    m_strmOutput    << L"HTTP/1.0 401 Unauthorized\n"
                    << L"Server: CQC Web Server\n"
                    << L"Date: " << m_tmHeader << kCIDLib::NewLn
                    << L"Connection: Close\n"
                    << L"WWW-Authenticate: Digest "
                    << L"  realm=\"CQC Automation System\", algorithm=\"MD5\","
                    << L"  qop=auth, domain=\"" << urlReq.strPath() << L"\",";

    // If asked, put a stale flag in
    if (bStale)
        m_strmOutput << L"stale=\"yes\",";

    //
    //  Generate the nonce. We include a time stamp in this, in which
    //  the minutes and seconds have been zeroed, effectively making
    //  this session legal until the top of the next hour.
    //
    //  WE MUST generate the path the same way here as in the main
    //  service method, since he will compare the returned nonce with one
    //  that he generates.
    //
    TString strNonce;
    GenerateNonce(urlReq.strPath(), strNonce);
    m_strmOutput    << L"nonce=\"" << strNonce << L"\","
                    << L"opaque=\"1234567890\""
                    << kCIDLib::DNewLn << kCIDLib::FlushIt;

    // And we can send the header and return
    cdsClient.WriteBytes(m_strmOutput.mbufData(), m_strmOutput.c4CurSize());
    cdsClient.FlushOut(TTime::enctNowPlusSecs(4));
}



//
//  Generates one of our standard challenge nonces. We create them such that
//  they are valid for one hour at most, by including a time in the nonce
//  which is clipped to the top of the current hour.
//
tCIDLib::TVoid
TWorkerThread::GenerateNonce(const TString& strURL, TString& strToFill)
{
    // Update the time object we use to current UTC, then clip to the current hour
    m_tmHeader.SetTo(tCIDLib::ESpecialTimes::CurrentUTC);
    m_tmHeader.ClipToHour();

    // Use strToFill as a temp to build up the text, then hash it
    m_tmHeader.FormatToStr(strToFill, m_tmHeader.strDefaultFormat());
    strToFill.Append(strURL);
    strToFill.Append(TSysInfo::strIPHostName());

    m_mdigAuth.StartNew();
    m_mdigAuth.DigestStr(strToFill);
    m_mdigAuth.Complete(m_mhashAuth1);

    //
    //  We now have an MD5 hash of the input data. Format it out to raw
    //  format (no dash), and then lower case it.
    //
    m_mhashAuth1.RawFormat(strToFill);
    strToFill.ToLower();
}


//
//  This is the method that does the work of servicing the client request. We handle
//  the exchange and then return. If it happens to turn out to be a Websockets
//  handover, then the data source is orphaned out to the web sockets handler and
//  we return after sending back any replies needed.
//
tCIDLib::TVoid TWorkerThread::ServiceClient(TCIDDataSrcJan& janSrc)
{
    if (facCQCWebSrvC().bLogInfo())
    {
        TString strRemote;
        janSrc.cdsData().FormatSrcInfo(strRemote);
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midDbg_ServingClient
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strRemote
        );
    }

    tCIDLib::TCard4         c4ContLen;
    tCIDLib::TCard4         c4ProtoVer;
    tCIDNet::ENetPReadRes   eRes;
    TString                 strContType;
    TString                 strType;
    TString                 strURL;

    //
    //  This guy doesn't throw. It will return an error to the client
    //  for any failure.
    //
    eRes = m_httpcSrv.eGetClientMsg
    (
        janSrc.cdsData()
        , TTime::enctNowPlusSecs(10)
        , strType
        , m_colInHdrLines
        , strContType
        , strURL
        , c4ProtoVer
        , m_mbufData
        , c4ContLen
    );

    //
    //  If we didn't succeed, then the HTTP client object already sent
    //  back a failure error to the client, so we just return.
    //
    if (eRes != tCIDNet::ENetPReadRes::Success)
        return;

    //
    //  Parse out the URL we got in the GET/POST line. It may be a full
    //  URL as per eventual HTTP 1.1 rules, but for now the server part will
    //  be ignored and if the request is 1.1 there must be a server header
    //  value.
    //
    const TURL urlReq(strURL, tCIDSock::EQualified::DontCare);

    // If 1.1, then make sure the host header value is present
    if (c4ProtoVer == 0x0101)
    {
        // <TBD>
    }

    if (facCQCWebSrvC().bLogInfo())
    {
        facCQCWebSrvC().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midDbg_ServingURL
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , urlReq
        );
    }

    // Get the expanded resource name, so path without any suffix stuff
    TString strURLResource;
    urlReq.QueryResource(strURLResource, kCIDLib::True, kCIDLib::False);

    //
    //  Break out the content type and possible encoding. If none, default it to
    //  UTF-8 as the likely default.
    //
    TString strEncoding;
    if (!TNetCoreParser::bFindTextEncoding(m_colInHdrLines, m_mbufData, c4ContLen, strEncoding))
        strEncoding = L"UTF-8";

    //
    //  We need to get various bits out of the content type. The above call handled
    //  the encoding, if any was found, or probed the body content if not. But we need
    //  get get strContType down to just the actual content type. And we need to look
    //  for any boundary string.
    //
    TString strBoundary;
    {
        TString strOrgCT(strContType);
        TStringTokenizer stokType(&strOrgCT, L";");

        // Force it to plain text if not set
        if (!stokType.bGetNextToken(strContType))
            strContType = L"text/plain";
        else
            strContType.StripWhitespace();

        TString strToken;
        TString strValue;
        while (stokType.bGetNextToken(strToken))
        {
            if (strToken.bSplit(strValue, kCIDLib::chEquals))
            {
                strToken.StripWhitespace();
                strValue.StripWhitespace();

                if (strToken.bCompareI(L"boundary"))
                    strBoundary = strValue;
            }
        }
    }

    //
    //  We need to parse out any GET query or POST body key=value pairs, since
    //  they need to be passed into the macro.
    //
    m_colValues.RemoveAll();
    if ((strType == kCIDNet::pszHTTP_GET) || (strType == kCIDNet::pszHTTP_HEAD))
    {
        //
        //  In this case, we can just get a copy of the stuff the URL already
        //  parsed for us.
        //
        m_colValues = urlReq.colQParms();
    }
     else if (strType == kCIDNet::pszHTTP_POST)
    {
        //
        //  Parse the parameters out of the body text if the content type indicates
        //  they are URL encoded.
        //
        if (strContType.bCompareI(L"application/x-www-form-urlencoded"))
        {
            TTextMBufInStream strmSrc
            (
                &m_mbufData
                , c4ContLen
                , tCIDLib::EAdoptOpts::NoAdopt
                , facCIDEncode().ptcvtMake(strEncoding)
            );
            TURL::c4ParseQueryParms
            (
                strmSrc, m_colValues, kCIDLib::True, TURL::EExpTypes::Query
            );
        }
    }
     else
    {
        if (facCQCWebSrvC().bLogFailures())
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCMsgs::midDbg_BadAccessType
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // It's not a format we support
        THTTPClient::SendErrReply
        (
            janSrc.cdsData()
            , L"CQC Web Server"
            , kCIDNet::c4HTTPStatus_BadRequest
            , L"Request type not supported"
            , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_AccNotSupported)
        );
        return;
    }

    //
    //  For debugging purposes, when in debug mode, and when enabled, we'll
    //  dump out the info to the console window.
    //
    #if CID_DEBUG_ON
    #define WEBSRV_DUMPREQ
    #if defined(WEBSRV_DUMPREQ)
    static TMutex mtxSync;
    static TOutConsole conOut;
    {

        TLocker lockrSync(&mtxSync);
        conOut  << L"\n----------- New Request ------------------\n"
                << L"URL=" << strURL << kCIDLib::NewLn
                << L"ReqType=" << strType << kCIDLib::NewLn
                << L"ContType=" << strContType << kCIDLib::NewLn
                << L"Encoding=" << strEncoding << kCIDLib::NewLn
                << L"ProtoVer=" << TCardinal(c4ProtoVer, tCIDLib::ERadices::Hex)
                << kCIDLib::NewLn;

        const tCIDLib::TCard4 c4HdrCnt = m_colInHdrLines.c4ElemCount();
        conOut << L"Hdr Count=" << c4HdrCnt << kCIDLib::NewLn;
        if (c4HdrCnt)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4HdrCnt; c4Index++)
            {
                const TKeyValuePair& kvalCur = m_colInHdrLines[c4Index];
                conOut << L"    " << kvalCur.strKey() << L":"
                       << kvalCur.strValue() << kCIDLib::EndLn;
            }
        }
        const tCIDLib::TCard4 c4QPCnt = m_colValues.c4ElemCount();
        conOut << L"Q Parm Count=" << c4QPCnt << kCIDLib::NewLn;
        if (c4QPCnt)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4QPCnt; c4Index++)
            {
                const TKeyValuePair& kvalCur = m_colValues[c4Index];
                conOut << L"    " << kvalCur.strKey() << L"="
                       << kvalCur.strValue() << kCIDLib::EndLn;
            }
        }

        conOut << kCIDLib::NewEndLn;
    }
    #endif
    #endif

    //
    //  If it starts with /Secure/ or /CMLBin/User/Secure/ then we need to force a
    //  login. We'll put the user name and privilege level into the list of values
    //  passed to the handler.
    //
    if (strURLResource.bStartsWithI(L"/Secure/")
    ||  strURLResource.bStartsWithI(L"/CMLBin/User/Secure/"))
    {
        //
        //  See if this request has authentication information in it. If so,
        //  check it. For this we need to pass in the whole path part, since it's
        //  used in the Nonce generation.
        //
        tCQCKit::EUserRoles eRole;
        TString strUserName;
        tCIDLib::TCard4 c4AuthRes = c4Authenticate
        (
            urlReq.strPath(), strType, strUserName, eRole
        );

        // If bad, just send an error and we are done
        if (c4AuthRes == kCIDNet::c4HTTPStatus_BadRequest)
        {
            THTTPClient::SendErrReply
            (
                janSrc.cdsData()
                , L"CQC Web Server"
                , kCIDNet::c4HTTPStatus_BadRequest
                , L"Bad Request"
                , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_BadAuthInfo)
            );
            return;
        }

        //
        //  if there was good authorization info, so see if this user has access
        //  to this particular section. There are 3 sections under /Secure/,
        //  one for each user level above limited. A user can access anything
        //  at his level and below. So cut off the leading part and start with
        //  the / after Secure.
        //
        if (c4AuthRes == kCIDNet::c4HTTPStatus_OK)
        {
            TString strTmp(strURLResource);
            if (strTmp.bStartsWithI(L"/Secure/"))
                strTmp.Cut(0, 7);
            else if (strTmp.bStartsWithI(L"/CMLBin/User/Secure/"))
                strTmp.Cut(0, 19);
            else
                strTmp.Clear();

            if ((strTmp.bStartsWithI(L"/Admin/") && (eRole < tCQCKit::EUserRoles::SystemAdmin))
            ||  (strTmp.bStartsWithI(L"/Power/") && (eRole < tCQCKit::EUserRoles::PowerUser))
            ||  (strTmp.bStartsWithI(L"/Normal/") && (eRole < tCQCKit::EUserRoles::NormalUser)))
            {
                c4AuthRes = kCIDNet::c4HTTPStatus_Unauthorized;
            }
        }

        //
        //  Now if stale or unathorized, challenge them again, else they are
        //  authorized and we can fall through.
        //
        if ((c4AuthRes == kCIDNet::c4HTTPStatus_Unauthorized)
        ||  (c4AuthRes == kCIDLib::c4MaxCard))
        {
            // If it's c4MaxCard, then it's stale
            SendAuthChallenge
            (
                janSrc.cdsData(), urlReq, strType, (c4AuthRes == kCIDLib::c4MaxCard)
            );
            return;
        }

        //
        //  Put the user name and user role into the list of key/value pairs
        //  we provide to the handlers.
        //
        m_colValues.objAdd(TKeyValuePair(L"CQCWS_USERNAME", strUserName));
        m_colValues.objAdd(TKeyValuePair(L"CQCWS_USERROLE", tCQCKit::strXlatEUserRoles(eRole)));
    }


    //
    //  At this point, we have to check the special case of a web socket type request.
    //  If we get one, then we have to see if we are willing to accept it. If not, we
    //  return a special return failure. Else we call another method that
    //  will service the web socket connection and not return until the connection
    //  is dropped.
    //
    //  Otherwise, we assume a standard HTTP operation and find a handler for it
    //  to process the request and return the info or failure.
    //
    //
    //  !!!!!!!!
    //  NOTE !!!
    //  !!!!!!!!
    //
    //  The socket will be orphaned off to the web sockets handler if we go
    //  that route. DO NOT use the socket after calling StartWebsockSession().
    //
    if (strURLResource.bStartsWithI(L"/Websock"))
    {
        // Do basic validation. If bad, it sends an error reply and we just return
        TString strSecKey;
        if (!TWebsockThread::bValidateReqInfo(  janSrc.cdsData()
                                                , strURLResource
                                                , m_colInHdrLines
                                                , strType
                                                , strSecKey))
        {
            return;
        }

        //
        //  Looks reasonsable enough to try to do it, so let's ask the facility
        //  object to create a web sockets handler and start it up.
        //
        facCQCWebSrvC().StartWebsockSession(janSrc, urlReq, strSecKey, strURLResource);

        //
        //  DO NOT access the source anymore. It has been given to the websocket
        //  handler that was started. We orphan the pointer out just in case, to make
        //  sure.
        //
        janSrc.pcdsOrphan();
    }
     else
    {
        //
        //  These will get filled in by the handler of the URL. They will be used to
        //  set up the correct values in the outgoing HTML header. The body content
        //  will end up in m_bufData.
        //
        //  In case of an incoming body, we set the values we got for content length
        //  and type to pass in.
        //
        tCIDLib::TCard4 c4OutLen = c4ContLen;
        TString         strOutType = strContType;

        //
        //  Figure out which URL handler we should pass this request to. We just point
        //  a local base class at it so that we can then generically invoke it. We fault
        //  these in as required.
        //
        TWSURLHandler* purlhToUse = nullptr;

        if (strURLResource.bCompareI(L"/CQCEcho")
        ||  strURLResource.bCompareI(L"/CQCEcho/"))
        {
            //
            //  It's the special Amazon Echo URL, so we create a handler specialized
            //  for that.
            //
            if (!m_purlhEcho)
                m_purlhEcho = new TWSEchoHandler;

            purlhToUse = m_purlhEcho;
        }
         else if (strURLResource.bStartsWithI(L"/CQCImg/"))
        {
            if (strType == kCIDNet::pszHTTP_POST)
            {
                THTTPClient::SendErrReply
                (
                    janSrc.cdsData()
                    , m_strServer
                    , kCIDNet::c4HTTPStatus_BadRequest
                    , L"No image based POSTS"
                    , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_NoFilePOST)
                );
                return;
            }

            if (!m_purlhCQCImg)
                m_purlhCQCImg = new TWSCQCImgHandler;
            purlhToUse = m_purlhCQCImg;
        }
         else if (strURLResource.bStartsWithI(L"/CMLBin/User/"))
        {
            if (!m_purlhCMLBin)
                m_purlhCMLBin = new TWSCMLBinHandler;
            purlhToUse = m_purlhCMLBin;
        }
         else
        {
            if (strType == kCIDNet::pszHTTP_POST)
            {
                THTTPClient::SendErrReply
                (
                    janSrc.cdsData()
                    , m_strServer
                    , kCIDNet::c4HTTPStatus_BadRequest
                    , L"No file based POSTS"
                    , facCQCWebSrvC().strMsg(kCQCWSCErrs::errcGen_NoFilePOST)
                );
                return;
            }

            // Assume a file
            if (!m_purlhFile)
                m_purlhFile = new TWSFileHandler;
            purlhToUse = m_purlhFile;
        }

        //
        //  We lie to the handler and tell him a HEAD is a GET, then we just don't
        //  send back any content.
        //
        TString strHandlerType = strType;
        if (strHandlerType == kCIDNet::pszHTTP_HEAD)
            strHandlerType = kCIDNet::pszHTTP_GET;

        // Ask him to process this URL
        TString strRepText(L"OK");
        m_colOutHdrLines.RemoveAll();
        const tCIDLib::TCard4 c4Res = purlhToUse->c4ProcessURL
        (
            urlReq
            , strType
            , m_colInHdrLines
            , m_colOutHdrLines
            , m_colValues
            , strEncoding
            , strBoundary
            , m_mbufData
            , c4OutLen
            , strOutType
            , strRepText
        );

        // If we didn't get any reply text, provide something, else it's a syntax issue
        if (strRepText.bIsEmpty())
            strRepText = L"No reply text provided by server";

        // If a head, throw away any return data
        if (strHandlerType == kCIDNet::pszHTTP_HEAD)
            c4ContLen = 0;

        // Ok, we can format out the header now. Update the time object first
        m_tmHeader.SetTo(tCIDLib::ESpecialTimes::CurrentUTC);
        m_strmOutput.Reset();
        m_strmOutput    << L"HTTP/1.0 " << c4Res << L' ' << strRepText << kCIDLib::NewLn
                        << L"Server: CQC Web Server\n"
                        << L"Date: " << m_tmHeader << kCIDLib::NewLn
                        << L"Connection: Close\n"
                        << L"Content-Length: " << c4OutLen << kCIDLib::NewLn;

        // If any content type, return it
        if (!strOutType.bIsEmpty())
            m_strmOutput << L"Content-Type: " << strOutType << kCIDLib::NewLn;

        // If there are any outgoing header lines from the handler, add them
        const tCIDLib::TCard4 c4OutHLCount = m_colOutHdrLines.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4OutHLCount; c4Index++)
        {
            const TKeyValuePair& colCur = m_colOutHdrLines[c4Index];
            m_strmOutput << colCur.strKey() << L": " << colCur.strValue()
                         << kCIDLib::NewLn;
        }

        // Now add a final empty line and flush
        m_strmOutput << kCIDLib::NewLn;
        m_strmOutput.Flush();

        //
        //  As above for the incoming, if debugging, dump the header info out.
        //
        #if CID_DEBUG_ON
        #define WEBSRV_DUMPREQ
        #if defined(WEBSRV_DUMPREQ)
        {
            TLocker lockrSync(&mtxSync);
            conOut  << L"\n----------- New Reply ------------------\n"
                    << L"Result=" << c4Res << kCIDLib::NewLn
                    << L"RepText=" << strRepText << kCIDLib::NewLn
                    << L"ContType=" << strOutType << kCIDLib::NewLn
                    << L"ContLen=" << c4OutLen << kCIDLib::NewLn
                    << kCIDLib::NewLn;

            const tCIDLib::TCard4 c4HdrCnt = m_colOutHdrLines.c4ElemCount();
            conOut << L"Hdr Count=" << c4HdrCnt << kCIDLib::NewLn;
            if (c4HdrCnt)
            {
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4HdrCnt; c4Index++)
                {
                    const TKeyValuePair& kvalCur = m_colOutHdrLines[c4Index];
                    conOut << L"    " << kvalCur.strKey() << L":"
                           << kvalCur.strValue() << kCIDLib::NewLn;
                }
            }

            // For now, we alway send close
            conOut  << L"    Connection: Close"
                    << kCIDLib::NewEndLn;
        }
        #endif
        #endif

        // And we can send the header stuff and then the body contents
        janSrc.cdsData().WriteBytes(m_strmOutput.mbufData(), m_strmOutput.c4CurSize());

        // If a head request, don't send anything even if available
        if (c4OutLen && (strType != kCIDNet::pszHTTP_HEAD))
            janSrc.cdsData().WriteBytes(m_mbufData, c4OutLen);

        // And now flush it
        janSrc.cdsData().FlushOut(TTime::enctNowPlusSecs(8));
    }
}

