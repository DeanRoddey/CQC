//
// FILE NAME: VantageQS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This file handles some of the grunt work overflow from the main impl
//  file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VantageQS.hpp"


// ---------------------------------------------------------------------------
//  TVantageQSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will check for the arrival of message. This device uses
//  simple CR terminated messages, so we can use a standard helper method
//  to read them. It takes a parameter that indicates whether failure to
//  get a reply is an error or not. If so, then we throw a timeout if we
//  don't get a message. If not throwing, then it returns a boolean that
//  indicates whether a message was received or not.
//
tCIDLib::TBoolean
TVantageQSDriver::bGetMsg(  const   tCIDLib::TCard4     c4Timeout
                            , const tCIDLib::TBoolean   bThrowIfNot
                            ,       TString&            strToFill)
{
    if (!bGetASCIITermMsg(*m_pcommVantage, c4Timeout, 0xD, 0, strToFill))
    {
        if (bThrowIfNot)
        {
            IncTimeoutCounter();
            facVantageQS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kVQSeriesErrs::errcProto_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Various replies from the device consist of space or comma seprated
//  tokens. This method will break them out into separate strings.
//
tCIDLib::TCard4
TVantageQSDriver::c4BreakOutReply(  const   TString&            strReply
                                    ,       TVector<TString>&   colTokens
                                    , const TString&            strSepChars
                                    , const tCIDLib::TCard4     c4ExpToks)
{
    TString strTok;
    TStringTokenizer stokReply(&strReply, strSepChars);

    while (stokReply.bGetNextToken(strTok))
        colTokens.objAdd(strTok);

    const tCIDLib::TCard4 c4Ret = colTokens.c4ElemCount();
    if ((c4ExpToks != kCIDLib::c4MaxCard) && (c4ExpToks != c4Ret))
    {
        facVantageQS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVQSeriesErrs::errcProto_BadRepTokenCnt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4ExpToks)
            , TCardinal(c4Ret)
        );
    }
    return c4Ret;
}


//
//  During the probing of the configuration, this is called when a floor
//  entry is seen in the qlink file contents. We parse until we see the end
//  of the floor data.
//
tCIDLib::TCard4
TVantageQSDriver::c4ProcessFloorCfg(const   TVector<TString>&   colConfig
                                    , const tCIDLib::TCard4     c4StartIndex)
{
    const tCIDLib::TCard4 c4Count = colConfig.c4ElemCount();
    tCIDLib::TCard4 c4Index = c4StartIndex;
    while (c4Index < c4Count)
    {
        const TString& strCur = colConfig[c4Index];

        //
        //  If it doesn't start with a space, it's the end of this block,
        //  so breka out without incrementing the index.
        //
        if (strCur[0] != kCIDLib::chSpace)
            break;

        // It's still one of ours so increment the index and process it
        c4Index++;


    }
    return c4Index;
}


//
//  This method is called to handle any messages that come in while waiting
//  for something else, since this device does send async notifications if
//  configured to do so, and we do configure it to so that we can respond
//  to events.
//
tCIDLib::TVoid TVantageQSDriver::HandleMsg(const TString& strMsg)
{

}


//
//  This method will ask for a dump of the backup flash memory, which contains
//  the contents of the qlink file. This contains all the info about the system
//  setup. We use this to create various item objects that we store away and
//  later use to create fields.
//
tCIDLib::TVoid TVantageQSDriver::ProbeConfig()
{
    TString strMsg;
    TString strReply;

    //
    //  To test the connection, we'll send a VCL to set the message term and
    //  tell it to use detailed responses and that we want to use CR
    //  terminators.
    //
    SendMsg(L"VCL 0 1");
    WaitForMsg(2000, L"RCL", strReply);

    //
    //  Ok, he's there so let's get the config file. We need a vector of
    //  strings to break out response info into.
    //
    TVector<TString> colConfig(256);

    //
    //  Find out which master we are actually connected to. We need this so
    //  that we can ask for the file contents from that master. The second
    //  token of the response has the master number.
    //
    SendMsg(L"VQA");
    WaitForMsg(2000, L"RQA", strReply);
    c4BreakOutReply(strReply, colConfig, m_strSepSpace, 3);

    // Now build up the message with the master number and ask for the file
    strMsg = L"VBF ";
    strMsg.Append(colConfig[1]);
    SendMsg(strMsg);

    // Clear out the list and start collecting lines till we see the end
    colConfig.RemoveAll();
    do
    {
        bGetMsg(2000, kCIDLib::True, strReply);
        colConfig.objAdd(strReply);
    }   while (strReply != L"##EndFlash");

    //
    //  Now we can go back through the file. It's a very simple, minimally
    //  structured file. It uses indentation as a nesting indication. So we
    //  keep up with nesting as a means to know when a particular section
    //  we've entered has ended.
    //
    const tCIDLib::TCard4 c4Count = colConfig.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        const TString& strCur = colConfig[c4Index++];

        // If we see the start of a floor, then process it
        if (strCur.bStartsWith(L"Floor:"))
            c4Index = c4ProcessFloorCfg(colConfig, c4Index);
    }
}


//
//  Sends a message. Taking the passed string, converting to ASCII, sending
//  it, and then sending the required trailing CR.
//
tCIDLib::TVoid TVantageQSDriver::SendMsg(const TString& strToSend)
{
    tCIDLib::TCard4 c4Count;

    //
    //  Transcode it to ASCII into our memory buffer and add the CR term.
    //  We know all our characters are just ASCII compatible stuff, so we
    //  can do a cheap transcode.
    //
    c4Count = strToSend.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_mbufWrite.PutCard1(tCIDLib::TCard1(strToSend[c4Index]), c4Index);
    m_mbufWrite.PutCard1(0x0D, c4Count++);

    // And now send it. If it fails, throw
    if (m_pcommVantage->c4WriteMBufMS(m_mbufWrite, c4Count, 100) != c4Count)
    {
        IncFailedWriteCounter();
        facVantageQS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVQSeriesErrs::errcProto_SendFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
        );
    }
}



tCIDLib::TVoid
TVantageQSDriver::WaitForMsg(const  tCIDLib::TCard4 c4Timeout
                            , const TString&        strExpRep
                            ,       TString&        strToFill)
{
    tCIDLib::TEncodedTime enctCur(TTime::enctNow());
    tCIDLib::TEncodedTime enctEnd(enctCur + (c4Timeout * kCIDLib::enctOneMilliSec));

    while(enctCur < enctEnd)
    {
        if (bGetMsg(250, kCIDLib::False, strToFill))
        {
            if (strToFill.bStartsWith(strExpRep))
                return;

            // If we are connected, then handle the message
            if (eState() == tCQCKit::EDrvStates::Connected)
                HandleMsg(strToFill);
        }
        enctCur = TTime::enctNow();
    }

    IncTimeoutCounter();
    facVantageQS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kVQSeriesErrs::errcProto_Timeout
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Timeout
    );
}
