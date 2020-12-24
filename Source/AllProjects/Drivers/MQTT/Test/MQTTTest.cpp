//
// FILE NAME: MQTTTest.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2019
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
//  We need a little test program to help test out the MQTT driver by publishing
//  some values to the test server that we can then set up the server to subscribe to.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CIDEncode.hpp"
#include    "CIDSock.hpp"
#include    "CIDSChan.hpp"
#include    "CQCKit.hpp"
#include    "CQCMQTT.hpp"
#include    "MQTTSh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid* pData);


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_MainModule(TThread(L"MQTTTestMainThread", eMainThreadFunc))


tCIDLib::TCard2             c2NextPacketId = 1;
TInConsole                  conIn(kCIDLib::True, 16);
TOutConsole                 conOut;
tCIDLib::TEncodedTime       enctNextPing;
TMQTTInMsg                  msgWaitRep(2048);
TTextFileOutStream          strmTrace;
TCIDSockStreamBasedDataSrc* pcdsMQTT;
TTime                       tmTrace;


static tCIDLib::TVoid SendMsg(TMQTTOutMsg& msgToSend)
{
    msgToSend.SendOn(*pcdsMQTT);
    msgToSend.LogTo(strmTrace, tmTrace);
}


// To process async msgs
static tCIDLib::TVoid ProcessMsg(TMQTTInMsg& msgNew)
{
    if (msgNew.ePacketType() == tCQCMQTT::EPacketTypes::Publish)
    {
        //
        //  If it's a publish msg, and the QOS is 1, we need to ack it. If it's
        //  QOS 2 we need to send a received msg
        //
        if (msgNew.eExtractPubQOS() == tCQCMQTT::EQOSLevs::Q1)
        {
            TMQTTOutMsg msgAck(16);
            msgAck.BuildPubAckMsg(msgNew.c2PacketId());
            SendMsg(msgAck);
        }
          else if (msgNew.eExtractPubQOS() == tCQCMQTT::EQOSLevs::Q2)
        {
            TMQTTOutMsg msgAck(16);
            msgAck.BuildPubRecMsg(msgNew.c2PacketId());
            SendMsg(msgAck);
        }
    }
     else if (msgNew.ePacketType() == tCQCMQTT::EPacketTypes::PubRel)
    {
        //
        //  We don't bother to check whether we have such a pending publish
        //  for this little test
        //
        TMQTTOutMsg msgAck(16);
        msgAck.BuildPubCompMsg(msgNew.c2PacketId());
        SendMsg(msgAck);
    }
}


static tCIDLib::TBoolean
bWaitReply( const   tCQCMQTT::EPacketTypes  eWaitType
            , const tCIDLib::TCard2         c2WaitId
            ,       TMQTTInMsg&             msgToFill
            , const tCIDLib::TCard4         c4WaitMSs)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitMSs * kCIDLib::enctOneMilliSec);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (!bRet && (enctCur < enctEnd))
    {
        // For for up to the remaining time for data
        if (pcdsMQTT->bDataAvail(enctEnd))
        {
            msgToFill.ParseFrom(*pcdsMQTT);
            msgToFill.LogTo(strmTrace, tmTrace);
            enctNextPing = TTime::enctNowPlusSecs(45);

            //
            //  If either they aren't waiting for any particular type or we
            //  got the one they were waiting for, return true.
            //
            bRet = (eWaitType == tCQCMQTT::EPacketTypes::Count)
                    || msgToFill.bIsThisMsg(eWaitType, c2WaitId);

            // Process it either way
            ProcessMsg(msgToFill);
        }
        enctCur = TTime::enctNow();
    }
    return bRet;
}


//
//  Does a publish operation with a binary value. We expect to get a topic, a
//  value which has to be convertable to an integer, and a number of bytes (1, 2, 4).
//  Float values have to be sent as text.
//
//      pubbin Sensors/Sensor10 [signed|unsigned] 133 2 [big|little]
//
//  You can optionally indicate the endianness. If not indicated it is big since
//  MQTT is normally big.
//
static tCIDLib::TVoid DoBinPublish(const tCIDLib::TStrList& colParms)
{
    tCIDLib::TInt8 i8Val;
    tCIDLib::TCard4 c4Bytes;

    if ((colParms.c4ElemCount() < 5)
    ||  (colParms.c4ElemCount() > 6)
    ||  !colParms[4].bToCard4(c4Bytes, tCIDLib::ERadices::Dec)
    ||  ((c4Bytes != 1) && (c4Bytes != 2) && (c4Bytes != 4))
    ||  !colParms[3].bToInt8(i8Val, tCIDLib::ERadices::Auto)
    ||  (!colParms[2].bCompareI(L"signed") && !colParms[2].bCompareI(L"unsigned")))
    {
        conOut  << L"\n  Form is: PubBin [topic] [signed|unsigned] [int value] [bytes]\n"
                << L"  where bytes is 1, 2 or 4.\n\n"
                << L"  An optional final parameter of [big|little] can be passed to indicate\n"
                << L"  the endianness. Defaults to big if not indicated.\n"
                << kCIDLib::EndLn;
        return;
    }

    // If we got an explicit endianness get that, else big
    tCIDLib::TBoolean bBigEndian = kCIDLib::True;
    if (colParms.c4ElemCount() == 6)
    {
        if (colParms[5].bCompareI(L"big"))
            bBigEndian = kCIDLib::True;
        else if (colParms[5].bCompareI(L"little"))
            bBigEndian = kCIDLib::False;
        else
        {
            conOut << L"The last parameter must be big or little" << kCIDLib::NewEndLn;
            return;
        }
    }

    //
    //  We need to create the binary payload, so create a stream and set its
    //  endianness based on the above. That will insure it gets set up to correctly
    //  write out the payload we want.
    //
    TBinMBufOutStream strmTar(16UL);
    if (bBigEndian)
        strmTar.eEndianMode(tCIDLib::EEndianModes::Big);
    else
        strmTar.eEndianMode(tCIDLib::EEndianModes::Little);

    // For convenience get the signed flag to a bool
    const tCIDLib::TBoolean bSigned = colParms[2].bCompareI(L"signed");

    //
    //  Based on the foramt, set up the min/max values and format out the new
    //  value cast to its actual type. We go ahead and stream it out even though
    //  we might reject it as out of range below, since that is get's us where
    //  we want to be with the last fuss.
    //
    tCIDLib::TInt8 i8MinVal = 0;
    tCIDLib::TInt8 i8MaxVal;
    if (bSigned)
    {
        if (c4Bytes == 1)
        {
            i8MinVal = kCIDLib::i1MinInt;
            i8MaxVal = kCIDLib::i1MaxInt;
            strmTar << tCIDLib::TInt1(i8Val);
        }
         else if (c4Bytes == 2)
        {
            i8MinVal = kCIDLib::i2MinInt;
            i8MaxVal = kCIDLib::i2MaxInt;
            strmTar << tCIDLib::TInt2(i8Val);
        }
         else if (c4Bytes == 4)
        {
            i8MinVal = kCIDLib::i4MinInt;
            i8MaxVal = kCIDLib::i4MaxInt;
            strmTar << tCIDLib::TInt4(i8Val);
        }
    }
     else
    {
        if (c4Bytes == 1)
        {
            i8MaxVal = kCIDLib::c1MaxCard;
            strmTar << tCIDLib::TCard1(i8Val);
        }
         else if (c4Bytes == 2)
        {
            i8MaxVal = kCIDLib::c2MaxCard;
            strmTar << tCIDLib::TCard2(i8Val);
        }
         else if (c4Bytes == 4)
        {
            i8MaxVal = kCIDLib::c4MaxCard;
            strmTar << tCIDLib::TCard4(i8Val);
        }
    }
    strmTar.Flush();

    // If the value is out of range, reject it
    if ((i8Val < i8MinVal) || (i8Val > i8MaxVal))
    {
        conOut  << L"\n  The value is too large for its format"
                << kCIDLib::NewEndLn;
        return;
    }

    // Set up the publish msg
    TMQTTOutMsg msgPub(256);
    msgPub.BuildPublishMsg
    (
        colParms[1]
        , tCQCMQTT::EQOSLevs::Q1
        , kCIDLib::True
        , c2NextPacketId++
        , strmTar.mbufData()
        , strmTar.c4CurSize()
    );
    SendMsg(msgPub);

    // It was set up for Q0, so we should just get a pub ack
    const tCIDLib::TBoolean bRes = bWaitReply
    (
        tCQCMQTT::EPacketTypes::PubAck, msgPub.c2PacketId(), msgWaitRep, 2500
    );

    if (bRes)
        conOut << L"Got ack for publish request";
     else
        conOut << L"No ack for publish request!";
    conOut << kCIDLib::NewLn;
}


//
//  Does a publish operation with a text string. We expect to get a topic, an encoding,
//  and a string to send. We send it as binary format, just the encoded bytes of the text.
//
//      pubbintext [encoding] Sensors/Temp1 45.1
//      pubbintext [encoding] Lights/Kitchen on
//
static tCIDLib::TVoid DoBinTextPublish(const tCIDLib::TStrList& colParms)
{
    if (colParms.c4ElemCount() != 4)
    {
        conOut << L"\n  Form is: PubBinText [encoding] [topic] [value]" << kCIDLib::EndLn;
        return;
    }

    // Try to create a converter for the requested encoding
    TTextConverter* pcvtEncode = facCIDEncode().ptcvtMake(colParms[1]);
    if (!pcvtEncode)
    {
        conOut  << L"The encoding '" << colParms[1] << L"' is not supported"
                << kCIDLib::NewLn;
        return;
    }
    TJanitor<TTextConverter> janCvt(pcvtEncode);

    // It might be multi-line, so replace any \n characters with new lines
    TString strExpText(colParms[3]);
    tCIDLib::TCard4 c4At(0);
    strExpText.bReplaceSubStr(L"\\n", L"\n", c4At, kCIDLib::True);

    // Now let's try to transcode it, do a liberal guess at space needed
    THeapBuf mbufText(strExpText.c4Length() * 2);
    tCIDLib::TCard4 c4PLBytes;
    pcvtEncode->c4ConvertTo(strExpText, mbufText, c4PLBytes);

    // Seemed to work so let's send it
    TMQTTOutMsg msgPub(c4PLBytes + colParms[2].c4Length() + 64);
    msgPub.BuildPublishMsg
    (
        colParms[2]
        , tCQCMQTT::EQOSLevs::Q1
        , kCIDLib::True
        , c2NextPacketId++
        , mbufText
        , c4PLBytes
    );
    SendMsg(msgPub);

    // It was set up for Q0, so we should just get a pub ack
    const tCIDLib::TBoolean bRes = bWaitReply
    (
        tCQCMQTT::EPacketTypes::PubAck, msgPub.c2PacketId(), msgWaitRep, 2500
    );

    if (bRes)
        conOut << L"Got ack for publish request";
     else
        conOut << L"No ack for publish request!";
    conOut << kCIDLib::NewLn;
}



//
//  Does a publish operation with a text string. We expect to get a topic and a
//  string to publish to the topic. We send the text as standard MQTT text format.
//
//      pubtext Sensors/Temp1 45.1
//      pubtext Lights/Kitchen on
//
static tCIDLib::TVoid DoTextPublish(const tCIDLib::TStrList& colParms)
{
    if (colParms.c4ElemCount() != 3)
    {
        conOut << L"\n  Form is: PubText [topic] [value]" << kCIDLib::EndLn;
        return;
    }

    // It might be multi-line, so replace any \n characters with new lines
    TString strExpText(colParms[2]);
    tCIDLib::TCard4 c4At(0);
    strExpText.bReplaceSubStr(L"\\n", L"\n", c4At, kCIDLib::True);

    TMQTTOutMsg msgPub(256);
    msgPub.BuildPublishMsg
    (
        colParms[1]
        , tCQCMQTT::EQOSLevs::Q1
        , kCIDLib::True
        , c2NextPacketId++
        , strExpText
    );
    SendMsg(msgPub);

    // It was set up for Q0, so we should just get a pub ack
    const tCIDLib::TBoolean bRes = bWaitReply
    (
        tCQCMQTT::EPacketTypes::PubAck, msgPub.c2PacketId(), msgWaitRep, 2500
    );

    if (bRes)
        conOut << L"Got ack for publish request";
     else
        conOut << L"No ack for publish request!";
    conOut << kCIDLib::NewLn;
}


//
//  Does a subscribe operation. We expect to get a topic filter and associated
//  QOS value.
//
static tCIDLib::TVoid DoSubscribe(const tCIDLib::TStrList& colParms)
{
    if (colParms.c4ElemCount() != 3)
    {
        conOut << L"Publish form is: Subscribe [topicfilter] [QOS]" << kCIDLib::EndLn;
        return;
    }

    // Make sure the QOS can be converted
    tCQCMQTT::EQOSLevs eQOS = tCQCMQTT::eXlatEQOSLevs(colParms[2]);
    if (eQOS == tCQCMQTT::EQOSLevs::Count)
    {
        conOut << L"Invalid QOS" << kCIDLib::EndLn;
        return;
    }

    tCQCMQTT::TTopicList colTopics(1);
    colTopics.objAdd(TMQTTTopicInfo(colParms[1], eQOS));

    TMQTTOutMsg msgSub(256);
    msgSub.BuildSubscribeMsg(colTopics, c2NextPacketId++);
    SendMsg(msgSub);

    const tCIDLib::TBoolean bRes = bWaitReply
    (
        tCQCMQTT::EPacketTypes::SubAck, msgSub.c2PacketId(), msgWaitRep, 2500
    );

    if (bRes)
    {
        if (msgWaitRep.c1CodeAt(0) == 0x80)
            conOut << L"Got a failure code";
        else
            conOut << L"Got ack for subscribe, at QOS level " << msgWaitRep.c1CodeAt(0);
    }
     else
    {
        conOut << L"No ack for subscribe request!";
    }
    conOut << kCIDLib::NewLn;
}


//
//  This is passed as an idle time callback to the console so that we can
//  handle keep alive msgs. And we also check for asyncs and process those.
//
static tCIDLib::TVoid IdleCallback(TObject* const)
{
    if (TTime::enctNow() > enctNextPing)
    {
        enctNextPing = TTime::enctNowPlusSecs(45);

        TMQTTOutMsg msgPing(8);
        msgPing.BuildPingReqMsg();
        SendMsg(msgPing);
    }
     else
    {
        if (pcdsMQTT->bDataAvailMS(1))
        {
            TMQTTInMsg msgAsync(2048);
            bWaitReply(tCQCMQTT::EPacketTypes::Count, 0, msgAsync, 5);
        }
    }
}


// -----------------------------------------------------------------------------
//  Program entry point
// -----------------------------------------------------------------------------
tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    // Get the standard CQC environmental info
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        return tCIDLib::EExitCodes::BadEnvironment;

    try
    {
        // Create an output stream for a msg trace
        tmTrace.strDefaultFormat(TTime::strMMDD_24HHMMSS());
        TPathStr pathTrace = facCQCKit().strServerDataDir();
        pathTrace.AddLevel(L"MQTT");
        pathTrace.AddLevel(L"MQTTTestTrace");
        pathTrace.AppendExt(L".Txt");
        strmTrace.Open
        (
            pathTrace
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Write
        );

        // First we need to create the data source and initialize it
        TIPEndPoint ipepTar(L"test.mosquitto.org", 1883, tCIDSock::EAddrTypes::Unspec);
        pcdsMQTT = new TCIDSockStreamDataSrc(ipepTar);
        pcdsMQTT->Initialize(TTime::enctNowPlusSecs(3));

        // And let's send a connect msg and wait for the ack
        {
            TMQTTOutMsg msgConn(256);
            msgConn.BuildConnectMsg
            (
                L"CQCMQTTTester"
                , kCIDLib::True
                , 60
                , TString::strEmpty()
                , TString::strEmpty()
            );
            SendMsg(msgConn);
            enctNextPing  = TTime::enctNowPlusSecs(45);
        }

        // Packet id won't be used here since conn ack doesn't have one
        if (!bWaitReply(tCQCMQTT::EPacketTypes::ConnAck, 0, msgWaitRep, 2500))
        {
            conOut << L"Never got a connection ack" << kCIDLib::EndLn;
            return tCIDLib::EExitCodes::Normal;
        }

        //
        //  Looks reasonable so enter the user command loop. But we still have to
        //  wake up periodically and send a keep alive. We use the idle time
        //  callback of the input console line reader to do this.
        //
        tCIDLib::TStrList colParms;
        TString strCurLn;
        while (kCIDLib::True)
        {
            conOut << L"> " << kCIDLib::FlushIt;
            conIn.c4ReadLine(strCurLn, IdleCallback, nullptr);
            strCurLn.StripWhitespace();

            // Watch for the exit command
            if (strCurLn.bCompareI(L"exit"))
                break;

            // Parse it as a command line
            if (!TExternalProcess::c4BreakOutParms(strCurLn, colParms))
            {
                conOut << L"Invalid command" << kCIDLib::EndLn;
            }
             else
            {
                try
                {
                    if (colParms[0].bCompareI(L"pubbin"))
                    {
                        DoBinPublish(colParms);
                    }
                     else if (colParms[0].bCompareI(L"pubbintext"))
                    {
                        DoBinTextPublish(colParms);
                    }
                     else if (colParms[0].bCompareI(L"pubtext"))
                    {
                        DoTextPublish(colParms);
                    }
                     else if (colParms[0].bCompareI(L"subscribe"))
                    {
                        DoSubscribe(colParms);
                    }
                     else
                    {
                        conOut  << L"Unknown command: " << colParms[0]
                                << kCIDLib::EndLn;
                    }
                }

                catch(const TError& errToCatch)
                {
                    conOut  << L"An error occurred:\n" << errToCatch
                            << kCIDLib::NewEndLn;
                }
            }
        }

        // And let's disconnect
        TMQTTOutMsg msgDisconn(8);
        msgDisconn.BuildDisconnectMsg();
        SendMsg(msgDisconn);

        // Close the data source
        pcdsMQTT->Terminate(TTime::enctNowPlusSecs(2), kCIDLib::True);
        delete pcdsMQTT;
    }

    // Catch any CIDLib runtime errors
    catch(const TError& errToCatch)
    {
        conOut << L"\nA CIDLib runtime error occured during processing. Error:\n"
               << errToCatch << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Kernel errors should never propogate out of CIDLib, but test for
    //  them here just in case.
    //
    catch(const TKrnlError& kerrToCatch)
    {
        conOut << L"\nA kernel error occured during processing.\nError="
               << kerrToCatch.errcId() << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::FatalError;
    }

    // Catch a general exception
    catch(...)
    {
        conOut << L"\nA general exception occured during processing"
               << kCIDLib::NewLn << kCIDLib::EndLn;
        return tCIDLib::EExitCodes::SystemException;
    }
    return tCIDLib::EExitCodes::Normal;
}


