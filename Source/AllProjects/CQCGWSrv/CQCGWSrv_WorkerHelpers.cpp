//
// FILE NAME: CQCGWSrv_WorkerHelpers.cpp
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
//  This file handles some grunt work overflow from the main worker thread
//  file. These are just related to the mechanics of sending and receiving
//  XML messages.
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
//  TWorkerThread: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TWorkerThread::bGetMsg(const tCIDLib::TCard4 c4WaitFor)
{
    CIDAssert(m_pcdsClient != nullptr, L"The connection data source is not allocated");
    if (!m_pcdsClient)
        return kCIDLib::False;

    // Wait for up to the indicated time for data to arrive
    if (!m_pcdsClient->bDataAvailMS(c4WaitFor))
        return kCIDLib::False;

    // Set up the flags and options we'll use for parsing
    const tCIDXML::EParseFlags eFlags = tCIDXML::EParseFlags::TagsNText;
    const tCIDXML::EParseOpts  eOpts  = tCIDXML::EParseOpts::Validate;

    //
    //  Read a packet header first. This will tell us how much more data we
    //  have to read to get the data. The default is to throw if all requested
    //  bytes aren't receieved.
    //
    const tCIDLib::TCard4 c4Bytes = m_pcdsClient->c4ReadBytes
    (
        &m_hdrCur, sizeof(tCQCGWSrv::TPacketHdr), kCIDLib::enctOneSecond, kCIDLib::True
    );
    if (!c4Bytes)
        return kCIDLib::False;

    // Make sure that the packet header is basically sound
    CheckHeader(m_hdrCur);

    //
    //  Read the indicated number of bytes. Again, it will throw if we don't
    //  get what we expect.
    //
    m_pcdsClient->c4ReadBytes(*m_pmbufData, m_hdrCur.c4DataSize, kCIDLib::enctFourSeconds, kCIDLib::True);
    const tCIDLib::TCard4 c4Sum = m_pmbufData->c4CheckSum(0, m_hdrCur.c4DataSize) & kCIDLib::c2MaxCard;

    // If the sum isn't right, then throw
    if (c4Sum != m_hdrCur.c2CheckSum)
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_BadCheckSum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  Ok, let's parse the XML using the XML tree parser, which will give us
    //  an XML document structure. Update the entity source with the data
    //  bytes that are now in the buffer before we parse.
    //
    m_pxesMsgs->c4SrcBytes(m_hdrCur.c4DataSize);
    if (!m_xtprsMsgs.bParseRootEntity(m_esrMsgs, eOpts, eFlags))
        ThrowParseErrReply();

    return kCIDLib::True;
}


tCIDLib::TBoolean TWorkerThread::bLogonSeq()
{
    //
    //  Get a message, which must be the logon request. It provides us the
    //  info we need to do a logon request to the security security on behalf
    //  of the caller.
    //
    const TXMLTreeElement* pxtnodeReq = nullptr;
    try
    {
        pxtnodeReq = &xtnodeGetMsg(L"CQCGW:LogonReq", m_c4LogOnTO);
    }

    catch(TError& errToCatch)
    {
        if (errToCatch.bCheckEvent( facCQCGWSrv.strName()
                                    , kCQCGWSErrs::errcProto_TimedOut))
        {
            // Log that we timed out, and just return
            if (facCQCGWSrv.bLogWarnings())
            {
                facCQCGWSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSErrs::errcProto_NoLogonReq
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::Protocol
                );
            }
            return kCIDLib::False;
        }

        // Not a timeout, so throw it back
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    //
    //  Get a security server client proxy and do the request for them. We'll
    //  get a security challenge back.
    //
    //  GET A COPY of the name, since the XML data will get blown away below
    //  when we reuse the parser!
    //
    const TString strUserName = pxtnodeReq->xtattrNamed(L"CQCGW:UserName").strValue();
    tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();
    if (!orbcSS->bLoginReq(strUserName, m_seccLogon))
    {
        TString strReason(kCQCGWSErrs::errcProto_UnknownUser, facCQCGWSrv, strUserName);
        SendNakReply(strReason);
        return kCIDLib::False;
    }

    // Format up the two data buffers and send the response back
    TString strData;
    TString strKey;
    TString strSessKey;
    FormatChallengeData(m_seccLogon, strKey, strData, strSessKey);

    // Ok, build up this response and send it back
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:LogonChallenge CQCGW:ChallengeData=\""
                << strData << L"\" CQCGW:ChallengeKey=\"" << strKey
                << L"\" CQCGW:SessKey=\"" << strSessKey
                << L"\"/>\n</CQCGW:Msg>\n"
                << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());

    // And now we have to get a 'get security token' message
    const TXMLTreeElement* pxtnodeToken = nullptr;
    try
    {
        pxtnodeToken = &xtnodeGetMsg(L"CQCGW:GetSecurityToken", m_c4LogOnTO * 2);
    }

    catch(TError& errToCatch)
    {
        if (errToCatch.bCheckEvent( facCQCGWSrv.strName()
                                    , kCQCGWSErrs::errcProto_TimedOut))
        {
            // Log that we timed out, and just return
            if (facCQCGWSrv.bLogWarnings())
            {
                facCQCGWSrv.LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCGWSErrs::errcProto_NoSeqTokReq
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::Protocol
                );
            }
            return kCIDLib::False;
        }

        // Not a timeout, so throw it back
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    //
    //  This one provides us with the re-encrypted challenge data. We can set
    //  this on the security challenge object and send it back in to get a
    //  security token which we will store on behalf of the client.
    //
    //  So we need to parse out the text formatted buffer data and get it into
    //  a temp buffer. Make sure that the string is of the required length. XML
    //  should have stripped any leading or trailing whitespace.
    //
    tCIDLib::TCard4 c4Len = 16;
    const TString& strRespData = pxtnodeToken->xtattrNamed(L"CQCGW:ResponseData").strValue();
    if (strRespData.c4Length() != c4Len * 2)
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_BadChallengeRepLen
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    THeapBuf mbufData(c4Len, c4Len);
    TString strTmp;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        //
        //  Pull out two characters each time through, and convert that to a
        //  byte and store.
        //
        strRespData.CopyOutSubStr(strTmp, c4Index * 2, 2);
        mbufData.PutCard1(tCIDLib::TCard1(strTmp.c4Val(tCIDLib::ERadices::Hex)), c4Index);
    }

    //
    //  Set this on the security challenge object. It has a special API to
    //  support this kind of proxying security.
    //
    m_seccLogon.SetValidatedData(mbufData, c4Len);

    // And try to get a security token
    TCQCSecToken        sectTmp;
    tCQCKit::ELoginRes  eRes = tCQCKit::ELoginRes::Count;
    if (!orbcSS->bGetSToken(m_seccLogon, sectTmp, m_uaccClient, eRes))
    {
        SendNakReply(tCQCKit::strXlatELoginRes(eRes));
        return kCIDLib::False;
    }

    //
    //  It worked, so send back an ack. We send back the user role type so
    //  that the client can react to that.
    //
    strTmp = tCQCKit::strXlatEUserRoles(m_uaccClient.eRole());
    SendAckReply(strTmp);

    //
    //  Set up our user context. If any environmental variables were
    //  passed along, then store them away.
    //
    m_cuctxClient.Set(m_uaccClient, sectTmp);
    const tCIDLib::TCard4 c4VarCnt = pxtnodeToken->c4ChildCount();
    tCIDLib::TCard4 c4VarNum;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4VarCnt; c4Index++)
    {
        const TXMLTreeElement& xtnodeVar = pxtnodeToken->xtnodeChildAtAsElement(c4Index);

        // Get the var number out and convert it
        const TString& strNum = xtnodeVar.xtattrNamed(L"CQCGW:VarNum").strValue();
        if (!strNum.bToCard4(c4VarNum) || !c4VarNum || (c4VarNum > 9))
            continue;

        // Store the body text of this element as the value if it has any
        if (xtnodeVar.c4ChildCount())
            m_cuctxClient.SetEnvRTVAt(c4VarNum - 1, xtnodeVar.xtnodeChildAtAsText(0).strText());
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TWorkerThread::CheckHeader(const tCQCGWSrv::TPacketHdr& hdrToCheck) const
{
    if ((hdrToCheck.c4MagicVal1 != kCQCGWSrv::c4MagicVal1)
    ||  (hdrToCheck.c4MagicVal2 != kCQCGWSrv::c4MagicVal2))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_BadMagicVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    if (hdrToCheck.c4DataSize > kCQCGWSrv::c4MaxDataBytes)
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_BadDataBytes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(hdrToCheck.c4DataSize)
        );
    }
}


tCIDLib::TVoid
TWorkerThread::FormatChallengeData( const   TCQCSecChallenge&   seccToFmt
                                    ,       TString&            strKey
                                    ,       TString&            strChallenge
                                    ,       TString&            strSessKey)
{
    //
    //  Set up a temp output text stream to do the formatting. And set a
    //  stream formatter on it that will cause it to do 2 character wide
    //  fields, right justified, with a zero character fill. This will make
    //  sure that all our formatted bytes are two hex digits.
    //
    static const TStreamFmt sfmtHexNum(2, 0, tCIDLib::EHJustify::Right, kCIDLib::chDigit0);
    TTextStringOutStream strmTmp(1024UL);
    strmTmp << sfmtHexNum;

    // Format out the challenge data
    const TMemBuf& mbufChal = seccToFmt.mbufChallenge();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < seccToFmt.c4ChallengeBytes(); c4Index++)
        strmTmp << TCardinal(mbufChal[c4Index], tCIDLib::ERadices::Hex);
    strmTmp.Flush();
    strChallenge = strmTmp.strData();

    // And reset the stream and do the key
    strmTmp.Reset();
    const TMemBuf& mbufKey = seccToFmt.mbufKey();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < seccToFmt.c4KeyBytes(); c4Index++)
        strmTmp << TCardinal(mbufKey[c4Index], tCIDLib::ERadices::Hex);
    strmTmp.Flush();
    strKey = strmTmp.strData();

    // And reset the stream and do the session key
    strmTmp.Reset();
    const TMemBuf& mbufSKey = seccToFmt.mbufSKey();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < seccToFmt.c4SKeyBytes(); c4Index++)
        strmTmp << TCardinal(mbufSKey[c4Index], tCIDLib::ERadices::Hex);
    strmTmp.Flush();
    strSessKey = strmTmp.strData();
}


//
//  This is a helper to format out a scheduled event. There are a couple places
//  we need to do it, and it's way too much to want to duplicate.
//
tCIDLib::TVoid
TWorkerThread::FormatSchEv( const   TString&        strPath
                            , const TCQCSchEvent&   csrcSrc
                            ,       TTextOutStream& strmTar)
{
    //
    //  We have a different element type and contents depending on the basic
    //  type of scheduled event. There's also common stuff, so we remember the
    //  correct closing element name for use at the end after the common
    //  stuff.
    //
    strmTar << L"      <";
    TString strElemName;
    if ((csrcSrc.eType() == tCQCKit::ESchTypes::MinPeriod)
    ||  (csrcSrc.eType() == tCQCKit::ESchTypes::HrPeriod)
    ||  (csrcSrc.eType() == tCQCKit::ESchTypes::DayPeriod))
    {
        // It's a periodic one
        strElemName = L"CQCGW:PerEvInfo";

        strmTar << strElemName << L" CQCGW:Period='" << csrcSrc.c4Period()
                << L"' CQCGW:Type='";

        if (csrcSrc.eType() == tCQCKit::ESchTypes::MinPeriod)
            strmTar << L"Min'";
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::HrPeriod)
            strmTar << L"Hour'";
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::DayPeriod)
            strmTar << L"Day'";
    }
     else if ((csrcSrc.eType() == tCQCKit::ESchTypes::Daily)
          ||  (csrcSrc.eType() == tCQCKit::ESchTypes::Weekly)
          ||  (csrcSrc.eType() == tCQCKit::ESchTypes::Monthly))
    {
        // Format out the mask and day/hour/minute value
        TString strMask;
        TString strDHM;

        strDHM.AppendFormatted(csrcSrc.c4Day());
        strDHM.Append(L' ');
        strDHM.AppendFormatted(csrcSrc.c4Hour());
        strDHM.Append(L' ');
        strDHM.AppendFormatted(csrcSrc.c4Minute());
        strDHM.Append(L' ');

        tCIDLib::TCard4 c4MaskBits = 0;
        if (csrcSrc.eType() == tCQCKit::ESchTypes::Weekly)
            c4MaskBits = 7;
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::Monthly)
            c4MaskBits = 12;

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4MaskBits; c4Index++)
        {
            if (csrcSrc.c4Mask() & (0x1UL << c4Index))
                strMask.Append(kCIDLib::chDigit1);
            else
                strMask.Append(kCIDLib::chDigit0);
        }

        // A regular scheduled type event
        strElemName = L"CQCGW:SchEvInfo";
        strmTar << strElemName << L" CQCGW:Mask='" << strMask
                << L"' CQCGW:DHM='" << strDHM << L"' CQCGW:Type='";

        if (csrcSrc.eType() == tCQCKit::ESchTypes::Daily)
            strmTar << L"Daily'";
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::Weekly)
            strmTar << L"Weekly'";
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::Monthly)
            strmTar << L"Monthly'";
    }
     else if ((csrcSrc.eType() == tCQCKit::ESchTypes::Sunrise)
          ||  (csrcSrc.eType() == tCQCKit::ESchTypes::Sunset))
    {
        // It's a sunrise/sunset based one
        strElemName = L"CQCGW:SunEvInfo";

        strmTar << strElemName << L" CQCGW:Offset='" << csrcSrc.i4Offset()
                << L"' CQCGW:Type='";

        if (csrcSrc.eType() == tCQCKit::ESchTypes::Sunrise)
            strmTar << L"Rise'";
        else if (csrcSrc.eType() == tCQCKit::ESchTypes::Sunset)
            strmTar << L"Set'";
    }
     else
    {
        CIDAssert2(L"Thepassed event is not a known scheduled event type")
    }

    // Put out any common stuff. The name we have to escape
    strmTar << L" CQCGW:Name='";
    facCIDXML().EscapeFor(csrcSrc.strTitle(), strmTar, tCIDXML::EEscTypes::Attribute);
    strmTar << L"' CQCGW:Id='" << strPath
            << L"' CQCGW:State='";

    if (csrcSrc.bPaused())
        strmTar << L"Paused'";
    else
        strmTar << L"Resumed'";

    // Now close it off
    strmTar << L"/>\n";
}


//
//  We can format out a field type directly, since it's a language neutral
//  value (it maps to the CML equiv class), but the access is a loaded
//  text value, so we have to provide our own mapping to format them out
//  such that they map to the values used in the DTD.
//
const tCIDLib::TCh*
TWorkerThread::pszMapFldAccess(const tCQCKit::EFldAccess eToMap) const
{
    switch(eToMap)
    {
        case tCQCKit::EFldAccess::Read :
            return L"Read";

        case tCQCKit::EFldAccess::Write :
            return L"Write";

        case tCQCKit::EFldAccess::ReadWrite :
            return L"ReadWrite";

        case tCQCKit::EFldAccess::None :
        default :
            break;
    };
    return L"None";
}


//
//  Helpers for sending back the common ack reply. One that just sends the ack,
//  and another that sends it and includes the optional Info parameter value.
//
tCIDLib::TVoid TWorkerThread::SendAckReply()
{
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n    <CQCGW:AckReply/>\n</CQCGW:Msg>\n"
                << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}

tCIDLib::TVoid TWorkerThread::SendAckReply(const TString& strInfoText)
{
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n    <CQCGW:AckReply CQCGW:Info='";

    facCIDXML().EscapeFor(strInfoText, m_strmReply, tCIDXML::EEscTypes::Attribute);

    m_strmReply << L"'/>\n</CQCGW:Msg>\n" << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


// A helper to send an exception back to the client
tCIDLib::TVoid
TWorkerThread::SendExceptionReply(const TError& errToSend)
{
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:ExceptionReply "
                << L"CQCGW:ErrClass=\""
                << tCIDLib::pszXlatEErrClasses(errToSend.eClass()) << L"\" "
                << L"CQCGW:ErrSev=\""
                << tCIDLib::pszXlatESeverities(errToSend.eSeverity()) << L"\" "
                << L"CQCGW:File=\"" << errToSend.strFileName() << L"\" "
                << L"CQCGW:Line=\"" << errToSend.c4LineNum() << L"\" "
                << L"CQCGW:Process=\"" << errToSend.strProcess() << L"\" "
                << L"CQCGW:Thread=\"" << errToSend.strThread() << L"\">\n    ";

    facCIDXML().EscapeFor(errToSend.strErrText(), m_strmReply, tCIDXML::EEscTypes::ElemText);
    if (errToSend.bHasAuxText())
    {
        m_strmReply << kCIDLib::NewLn;
        facCIDXML().EscapeFor(errToSend.strAuxText(), m_strmReply, tCIDXML::EEscTypes::ElemText);
    }

    m_strmReply << L"\n    </CQCGW:ExceptionReply>\n</CQCGW:Msg>\n" << kCIDLib::FlushIt;

    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


// A helper to send a Nak replay, with the failure reason text attribute
tCIDLib::TVoid TWorkerThread::SendNakReply(const TString& strReason)
{
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:NakReply "
                << L"CQCGW:ReasonText=\"" << strReason
                << L"\"/>\n</CQCGW:Msg>\n"
                << kCIDLib::FlushIt;
    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


//
//  All packets sent back to the client will go through here, though there are
//  a number of methods layered over this one to send specific types of messages,
//  they all call here in the end.
//
tCIDLib::TVoid
TWorkerThread::SendReply(const  TMemBuf&            mbufData
                        , const tCIDLib::TCard4     c4Bytes
                        , const tCIDLib::TBoolean   bCanEncrypt)
{
    // Make sure we aren't trying to return too much data
    if (c4Bytes > kCQCGWSrv::c4MaxDataBytes)
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_TooMuchToReturn
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Overflow
            , TCardinal(c4Bytes)
        );
    }

    //
    //  Since we are purely call and response, in order to insure that we
    //  stay in sync and that no bad incoming messages leaves data in the
    //  socket buffer to confuse us later, we just flush the incoming data
    //  before we send any reply. There should never be any incoming data
    //  available at this point when we are about to reply.
    //
    tCIDLib::TCard1 ac1Buf[2048];
    tCIDLib::TCard4 c4Avail;
    while (m_pcdsClient->bDataAvailMS(100))
        m_pcdsClient->c4ReadBytes(ac1Buf, tCIDLib::MinVal(c4Avail, 2048UL), TTime::enctNowPlusMSs(10));

    m_hdrCur.c1Flags    = 0;
    m_hdrCur.c4DataSize = c4Bytes;
    m_hdrCur.c2CheckSum = tCIDLib::TCard2(mbufData.c4CheckSum(0, c4Bytes));

    // Send the header, and then send the data
    m_pcdsClient->WriteRawBytes(&m_hdrCur, sizeof(m_hdrCur));
    m_pcdsClient->WriteBytes(mbufData, c4Bytes);
    m_pcdsClient->FlushOut(TTime::enctNowPlusSecs(2));
}


tCIDLib::TVoid TWorkerThread::SendUnknownExceptionReply()
{
    m_strmReply.Reset();
    m_strmReply << L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                << kCQCGWSrv::pszDTD
                << L"<CQCGW:Msg>\n"
                << L"    <CQCGW:UnknownExceptionReply/>"
                << L"</CQCGW:Msg>\n"
                << kCIDLib::FlushIt;

    SendReply(m_strmReply.mbufData(), m_strmReply.c4CurSize());
}


tCIDLib::TVoid TWorkerThread::ThrowParseErrReply()
{
    const TXMLTreeParser::TErrInfo& erriToSend = m_xtprsMsgs.erriFirst();

    // Build up the non-text error info as the aux text
    TString strAuxText(L"System Id: ");
    strAuxText.Append(erriToSend.strSystemId());
    strAuxText.Append(L", Line/Col: ");
    strAuxText.AppendFormatted(erriToSend.c4Line());
    strAuxText.Append(L"/");
    strAuxText.AppendFormatted(erriToSend.c4Column());
    strAuxText.Append(L", Text: ");
    strAuxText.Append(erriToSend.strText());

    facCQCGWSrv.ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kCQCGWSErrs::errcProto_ParseFailed
        , strAuxText
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Protocol
    );
}


const TXMLTreeElement&
TWorkerThread::xtnodeGetMsg(const   TString&        strExpectedMsg
                            , const tCIDLib::TCard4 c4Timeout)
{
    if (!bGetMsg(c4Timeout))
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_TimedOut
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , strExpectedMsg
        );
    }

    const TXMLTreeElement& xtnodeRoot = m_xtprsMsgs.xtdocThis().xtnodeRoot();
    const TXMLTreeElement& xtnodeMsg = xtnodeRoot.xtnodeChildAtAsElement(0);

    if (xtnodeMsg.strQName() != strExpectedMsg)
    {
        facCQCGWSrv.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcProto_ExpectedMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , strExpectedMsg
            , xtnodeMsg.strQName()
        );
    }
    return xtnodeMsg;
}

