//
// FILE NAME: ZWaveUSBS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2005
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
//  This file provides the implementation of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We have to bring in Z-Wave specific headers. The project setup for this
//  driver will set up the include path needed.
// ---------------------------------------------------------------------------
#include    "ZWaveUSBS_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUSBSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSBSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUSBSDriver::TZWaveUSBSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bInNetwork(kCIDLib::False)
    , m_bIsSUC(kCIDLib::False)
    , m_bRepMode(kCIDLib::False)
    , m_bsNodeMap(kZWaveUSBSh::c1MaxUnits)
    , m_c1CtrlId(0)
    , m_c1NextCBId(1)
    , m_c1SerAPIVersion(0)
    , m_c4FirstGrpId(0)
    , m_c4FldId_Command(0)
    , m_c4FWVersion(0)
    , m_c4HomeId(0)
    , m_c4PollableCnt(0)
    , m_c4LastGrpId(0)
    , m_colPollQ(kZWaveUSBSh::c1MaxUnits, tCIDLib::EAdoptOpts::NoAdopt)
    , m_enctLastMsg(0)
    , m_enctNextData(0)
    , m_enctNextMsg(0)
    , m_eRepRes(tZWaveUSBSh::ERepRes_Count)
    , m_pcommZWave(0)
    , m_splBufs(L"ZWave Buf Mgr", 64, 32, 8)
    , m_thrReplicate
      (
          TString(L"ZWRepThread") + cqcdcToLoad.strMoniker()
          , TMemberFunc<TZWaveUSBSDriver>(this, &TZWaveUSBSDriver::eReplicationThread)
      )
{
}

TZWaveUSBSDriver::~TZWaveUSBSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommZWave)
    {
        try
        {
            if (m_pcommZWave->bIsOpen())
                m_pcommZWave->Close();

            delete m_pcommZWave;
            m_pcommZWave = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  A standard backdoor method that lets the client query us for a text
//  value.
//
tCIDLib::TBoolean
TZWaveUSBSDriver::bQueryTextVal(const   TString&    strQueryType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    // Reject if we are currently replicating
    CheckReplicating();

    tCIDLib::TBoolean bRetVal = kCIDLib::True;
    if (strQueryType == kZWaveUSBSh::strValId_QueryAssoc)
        QueryUnitAssoc(strDataName, strToFill);
    else
        bRetVal = kCIDLib::False;

    return bRetVal;

}


//
//  Standard driver backdoor method that lets the client send us a command
//  with data and for us to optionally send back some kind of response. We use
//  this one for the more complex types of commands that are best done by
//  flattening the data objects and sending them as a memory buffer.
//
tCIDLib::TBoolean
TZWaveUSBSDriver::bSendData(const   TString&            strDataType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufData)
{
    // Reject if we are currently replicating
    CheckReplicating();

    tCIDLib::TBoolean bRetVal = kCIDLib::True;
    if (strDataType == kZWaveUSBSh::strDrvCmd_SetConfig)
        UpdateConfig(mbufData, c4Bytes);
    else
        bRetVal = kCIDLib::False;

    //
    //  BE SURE to zero the byte value, since the buffer is an output parm!
    //  Otherwise, he'll stream back unused info. In our case we don't send
    //  anything back, so we can just zero it here.
    //
    c4Bytes = 0;

    return bRetVal;
}


//
//  We need to send a callback id any time we do something that will invoke
//  a callback, so that we can check that the callback is for the transaction
//  we are doing, and not some delayed response. We just use a Card1 value
//  that we bump for each such function id required. We never let it be zero
//  since that would tell the controller we don't want a callback.
//
tCIDLib::TCard1 TZWaveUSBSDriver::c1NextCallbackId()
{
    m_c1NextCBId++;
    if (!m_c1NextCBId)
        m_c1NextCBId++;
    return m_c1NextCBId;
}


//
//  Standard driver backdoor method to return the driver configuration info.
//  The format is purely between the client and server driver sides.
//
tCIDLib::TCard4 TZWaveUSBSDriver::c4QueryCfg(THeapBuf& mbufToFill)
{
    // Create a bin output stream over the passed buffer
    TBinMBufOutStream strmOut(&mbufToFill);

    strmOut << tCIDLib::EStreamMarkers::StartObject
            << m_dcfgCurrent
            << tCIDLib::EStreamMarkers::EndObject
            << kCIDLib::FlushIt;

    return strmOut.c4CurSize();
}


//
//  Standard driver backdoor function to return some unsigned status info. We
//  use this one to return any numeric values to the client side driver.
//
tCIDLib::TCard4 TZWaveUSBSDriver::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4RetVal = 0;

    if (strValId == kZWaveUSBSh::strValId_DrvStatus)
    {
        //
        //  The replication mode flag goes into the low word, and the
        //  replication result goes into the high.
        //
        c4RetVal = m_eRepRes;
        c4RetVal <<= 16;
        c4RetVal |= m_bRepMode ? 1 : 0;
    }
     else
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_BadValueId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
            , strMoniker()
        );
    };
    return c4RetVal;
}


//
//  Standard driver backdoor function to let the client ask us to do some
//  things. This one is used for pretty simple commands where we just need
//  a command id and either no other info, or some simple info that can be
//  easily passed in a parameter string.
//
tCIDLib::TCard4
TZWaveUSBSDriver::c4SendCmd(const   TString&    strCmdId
                            , const TString&    strParms)
{
    //
    //  If we are in replication mode, then reject it unless it's a
    //  replication mode command with a value of Stop. Otherwise, gotta
    //  let it through so that they can cancel a failed replication.
    //
    if ((strCmdId != kZWaveUSBSh::strDrvCmd_SetRepMode) || (strParms != L"Stop"))
        CheckReplicating();

    if ((strCmdId == kZWaveUSBSh::strDrvCmd_AddAssoc)
    ||  (strCmdId == kZWaveUSBSh::strDrvCmd_DelAssoc))
    {
        SetAssociation(strCmdId == kZWaveUSBSh::strDrvCmd_AddAssoc, strParms);
    }
     else if (strCmdId == kZWaveUSBSh::strDrvCmd_DelGroup)
    {
        // The parm string is the id of the group
        DeleteGroup(strParms);
    }
     else if (strCmdId == kZWaveUSBSh::strDrvCmd_SetCfgParam)
    {
        SetCfgParam(strParms);
    }
     else if (strCmdId == kZWaveUSBSh::strDrvCmd_SetRepMode)
    {
        // The parameter string is Start or Stop
        if (strParms == L"Start")
        {
            //
            //  Make sure that we aren't already replicating. If so, then
            //  throw an error. Else, start up the replication thread.
            //
            if (m_bRepMode)
            {
                facZWaveUSBS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZWErrs::errcProto_AlreadyInRep
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Already
                    , strMoniker()
                );
            }
            m_thrReplicate.Start();
        }
         else
        {
            // If we are in rep mode, then ask the thread to stop
            if (m_bRepMode && m_thrReplicate.bIsRunning())
            {
                m_thrReplicate.ReqShutdownSync(5000);
                m_thrReplicate.eWaitForDeath(2500);
            }
        }
    }
     else if (strCmdId == kZWaveUSBSh::strDrvCmd_ResetCtrl)
    {
        //
        //  Just invoke the reset method. This will reset the controller,
        //  reset our configuration, and reload the fields to reflect
        //  the new default configuration.
        //
        ResetCtrl();
    }
     else
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_BadCmdId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmdId
            , strMoniker()
        );
    }
    return 0;
}


//
//  Sends the message in m_bufOut and waits for an ack. If the frame is
//  ack'd, it will wait for for up to c4WaitFor milliseconds for the
//  indicated message to arrive.
//
//  It's for sending something that is expected to return some data, not
//  just to be acknowledged.
//
tCIDLib::TCard4
TZWaveUSBSDriver::c4SendAndWaitForReply(const   TMemBuf&            mbufOut
                                        , const tCIDLib::TCard4     c4WaitFor
                                        , const tCIDLib::TCard4     c4ExpRepLen
                                        ,       TMemBuf&            mbufIn)
{
    // Send the frame and wait for the ack
    SendAndWaitForAck(mbufOut);

    // And now wait for the reply to the indicated message
    return c4WaitForReply
    (
        mbufOut[2], c4WaitFor, c4ExpRepLen, kCIDLib::False, mbufIn
    );
}


tCIDLib::TCard4
TZWaveUSBSDriver::c4SendReqAndWaitForReply(         tCIDLib::TCard1 c1Req
                                            , const tCIDLib::TCard4 c4WaitFor
                                            , const tCIDLib::TCard4 c4ExpRepLen
                                            ,       TMemBuf&        mbufIn)
{
    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 256);
    TMemBuf& mbufOut = janPool.objGet();

    mbufOut.PutCard1(3, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(c1Req, 2);
    return c4SendAndWaitForReply(mbufOut, c4WaitFor, c4ExpRepLen, mbufIn);
}


tCIDLib::TCard4
TZWaveUSBSDriver::c4SendReqAndWaitForReply(         tCIDLib::TCard1 c1Req
                                            , const tCIDLib::TCard4 c4WaitFor
                                            , const tCIDLib::TCard4 c4ExpRepLen
                                            , const tCIDLib::TCard1 c1Parm
                                            ,       TMemBuf&        mbufIn)
{
    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 256);
    TMemBuf& mbufOut = janPool.objGet();

    mbufOut.PutCard1(4, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(c1Req, 2);
    mbufOut.PutCard1(c1Parm, 3);
    return c4SendAndWaitForReply(mbufOut, c4WaitFor, c4ExpRepLen, mbufIn);
}


tCIDLib::TCard4
TZWaveUSBSDriver::c4SendReqAndWaitForReply(         tCIDLib::TCard1 c1Req
                                            , const tCIDLib::TCard4 c4WaitFor
                                            , const tCIDLib::TCard4 c4ExpRepLen
                                            , const tCIDLib::TCard1 c1Parm
                                            , const tCIDLib::TCard1 c1Parm2
                                            ,       TMemBuf&        mbufIn)
{
    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 256);
    TMemBuf& mbufOut = janPool.objGet();

    mbufOut.PutCar1(5, 0);
    mbufOut.PutCar1(REQUEST, 1);
    mbufOut.PutCar1(c1Req, 2);
    mbufOut.PutCar1(c1Parm, 3);
    mbufOut.PutCar1(c1Parm2, 4);
    return c4SendAndWaitForReply(mbufOut, c4WaitFor, c4ExpRepLen, mbufIn);
}


//
//  Waits for a given reply to come in, handling other messages that arrive
//  in the meantime.
//
tCIDLib::TCard4
TZWaveUSBSDriver::c4WaitForReply(const  tCIDLib::TCard1     c1Expected
                                , const tCIDLib::TCard4     c4WaitFor
                                , const tCIDLib::TCard4     c4ExpRepLen
                                , const tCIDLib::TBoolean   bCallBack
                                ,       TMemBuf&            mbufIn)
{
    const tZWaveUSBS::EMsgTypes eExpType
    (
        bCallBack ? tZWaveUSBS::EMsgType_CallBack : tZWaveUSBS::EMsgType_Response
    );

    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd(enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec));

    // And now wait for the reply to the indicated message
    tCIDLib::TCard1 c1CmdId;
    tCIDLib::TCard1 c1CBId;
    tCIDLib::TCard4 c4Read;
    while (enctCur < enctEnd)
    {
        // Tell him to throw on a timeout or failure
        const tZWaveUSBS::EMsgTypes eRet = eGetReply
        (
            c4Read, mbufIn, enctEnd - enctCur, c1CmdId, c1CBId, kCIDLib::True
        );

        if ((eRet == eExpType) && (c1CmdId == c1Expected))
            break;

        // Call the aysnc msg handler and keep waiting
        if (eRet == tZWaveUSBS::EMsgType_CallBack)
            HandleCallBack(c1CmdId, mbufIn, c4Read);
        else if (eRet == tZWaveUSBS::EMsgType_Response)
            HandleResponse(c1CmdId, mbufIn, c4Read);

        enctCur = TTime::enctNow();
    }

    // If we timed out, then give up
    if (enctCur >= enctEnd)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_Timeout2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , TCardinal(c1Expected, tCIDLib::ERadices::Hex)
        );
    }

    // If the indicated an expected number of bytes we it's not, then throw
    if (c4ExpRepLen && (c4Read != c4ExpRepLen))
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_BadRepLen
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4ExpRepLen)
            , TCardinal(c4Read)
            , TCardinal(c1Expected)
        );
    }
    return c4Read;
}


// Provide access to our memory buffer pool
TSimplePool<THeapBuf>& TZWaveUSBSDriver::splBufs()
{
    return m_splBufs;
}


//
//  Sends the passed message, and waits for an ack message. If it sees
//  anything else while waiting, it calls an async handler.
//
tCIDLib::TVoid TZWaveUSBSDriver::SendAndWaitForAck(const TMemBuf& mbufOut)
{
    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 256);
    TMemBuf& mbufIn = janPool.objGet();

    // If the minimum inter-send interval hasn't expired, then wait for it
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur < m_enctNextMsg)
    {
        const tCIDLib::TCard4 c4Millis = tCIDLib::TCard4
        (
            (m_enctNextMsg - enctCur) / kCIDLib::enctOneMilliSec
        );
        DelayAndProcess(c4Millis, mbufIn);
    }

    // And now we can send the message
    SendMsg(mbufOut);

    const tCIDLib::TCard4 c4MaxRetries = 8;
    const tCIDLib::TCard4 c4MaxNaks    = 8;

    tCIDLib::TCard1 c1CmdId;
    tCIDLib::TCard1 c1CBId;
    tCIDLib::TCard4 c4Retries = 0;
    tCIDLib::TCard4 c4Naks = 0;
    TThread*        pthrCaller = 0;
    while ((c4Retries < c4MaxRetries) && (c4Naks < c4MaxNaks))
    {
        tCIDLib::TCard4 c4Read;
        tZWaveUSBS::EMsgTypes eRet = eGetReply
        (
            c4Read, mbufIn, kCIDLib::enctOneSecond, c1CmdId, c1CBId, kCIDLib::False
        );

        // If we got an ack, we are done
        if (eRet == tZWaveUSBS::EMsgType_Ack)
            break;

        //
        //  If it's some other message, then handle it. If he Nak'd us, then
        //  retry.
        //
        if (eRet == tZWaveUSBS::EMsgType_CallBack)
        {
            HandleCallBack(c1CmdId, mbufIn, c4Read);
        }
         else if (eRet == tZWaveUSBS::EMsgType_Response)
        {
            HandleResponse(c1CmdId, mbufIn, c4Read);
        }
         else if (eRet == tZWaveUSBS::EMsgType_Nak)
        {
            // Delay a bit before retrying
            c4Naks++;
            m_enctNextMsg = 0;
            DelayAndProcess(200, mbufIn);
            SendMsg(mbufOut);
        }
         else if (eRet == tZWaveUSBS::EMsgType_Timeout)
        {
            c4Retries++;
        }

        // Watch for shutdown requests
        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
        {
            // We were asked to shutdown, so break out with forced failure
            c4Retries = c4MaxRetries;
            break;
        }
    }

    // Reset the next outgoing message time now, whether it worked or not
    m_enctNextMsg = TTime::enctNow() + kZWaveUSBS::enctSendInt;

    // We  didn't get the ack, then throw
    if (c4Retries >= c4MaxRetries)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_NoAck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , TCardinal(mbufOut[2], tCIDLib::ERadices::Hex)
        );
    }
     else if (c4Naks >= c4MaxNaks)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_MaxNaks
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , TCardinal(mbufOut[2], tCIDLib::ERadices::Hex)
        );
    }
}


//
//  This is called for a SendData or SendMultiData type message. It does the
//  underlying SendAndWaitForAck() call, then it waits for the callback that
//  indicates the data has been sent. If we fail to wait for that, then we
//  will overflow the controller's buffer.
//
//  They can tell us not to wait for the secondary ACK (which only comes
//  back if they set the TRANSMIT_OPTION_ACK.)
//
//  This is one of the controller mixin calls, so it has to be public so
//  that it an be called by the units.
//
tCIDLib::TVoid
TZWaveUSBSDriver::SendData( const   TMemBuf&            mbufOut
                            , const tCIDLib::TBoolean   bGetAck)
{
    // Get a buffer for reading replies into
    THeapBuf mbufIn(32, 255);

    //
    //  We enforce a minimum time between SendData operations, since they
    //  tend to put the system into various busy states. So if that time
    //  hasn't expired since the last one, let's wait for it and process
    //  messages while we wait.
    //
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur < m_enctNextData)
    {
        const tCIDLib::TCard4 c4Millis = tCIDLib::TCard4
        (
            ((m_enctNextData - enctCur) / kCIDLib::enctOneMilliSec)
        );
        DelayAndProcess(c4Millis, mbufIn);
    }

    //
    //  In case we fail below, go ahead and do a preliminary reset of the
    //  next data time.
    //
    m_enctNextData = TTime::enctNow() + kZWaveUSBS::enctPollInt;

    // Get out the cmd and callback id for convenience
    const tCIDLib::TCard1 c1Cmd = mbufOut[2];
    const tCIDLib::TCard1 c1CBId = mbufOut[mbufOut[0] - 1];

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
    {
        ZWLogDbgMsg2
        (
            L"Sending message Cmd=%(1), CBId=%(2)"
            , TCardinal(c1Cmd, tCIDLib::ERadices::Hex)
            , TCardinal(c1CBId)
        );
    }

    SendAndWaitForAck(mbufOut);
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg(L"Waiting for first reply");

    // We should get back a return value
    c4WaitForReply(c1Cmd, 3000, 4, kCIDLib::False, mbufIn);
    if (!mbufIn[3])
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_SendMsgRetVal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::XmitFailed
        );
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg(L"Waiting for xmit status");

    //
    //  If they indicate we should get a transmit status ack, then
    //  wait for that.
    //
    if (bGetAck)
    {
        c4WaitForReply(c1Cmd, 5000, 5, kCIDLib::True, mbufIn);

        // Make sure it's for our function id
        if (mbufIn[3] != c1CBId)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(L"Bad callback id in reply");

            facZWaveUSBS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcProto_BadCBId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::XmitFailed
            );
        }

        // If an xmit error, then throw
        if (mbufIn[4])
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                ZWLogDbgMsg2
                (
                    L"Send data failed with xmit status error %(1). Cmd=%(2)"
                    , TCardinal(mbufIn[4])
                    , TCardinal(c1Cmd, tCIDLib::ERadices::Hex)
                );
            }
            facZWaveUSBS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcProto_SendFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::XmitFailed
                , TCardinal(c1Cmd, tCIDLib::ERadices::Hex)
            );

            // Bump the failed write counter field
            IncFailedWriteCounter();
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg(L"Data sent ok");

    // And reset the next send data time
    m_enctNextData = TTime::enctNow() + kZWaveUSBS::enctPollInt;
}


// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't need to lock here, because no incoming client calls will be
//  seen until we are online, and we aren't yet.
//
tCIDLib::TBoolean TZWaveUSBSDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Create the port object if needed, else close it just in case
        if (!m_pcommZWave)
            m_pcommZWave = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommZWave->bIsOpen())
            m_pcommZWave->Close();

        // Open the port now, and set the configuration
        m_pcommZWave->Open(tCIDComm::EOpenFlags::WriteThrough, 1024, 1024);
        m_pcommZWave->SetCfg(m_cpcfgSerial);
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//
//  See if we can load any config. If there isn't any, that's fine, we just
//  come up with not groups and units. If we cannot get to the config server,
//  we indicate that we are still waiting.
//
tCIDLib::TBoolean TZWaveUSBSDriver::bWaitConfig(TThread&)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        //
        //  Get a config server client proxy. Use a fairly short timeout.
        //  If it's not there, we don't want to hang around and wait.
        //
        TCfgServerClient cfgscLoad(2500);

        // See if our config key exists in the repository
        if (cfgscLoad.bKeyExists(m_strCfgScope))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
               ZWLogDbgMsg(L"Trying to load existing configuration");

            //
            //  It does, so read it in. We don't need to keep the version
            //  around, since we only read it at startup and after that we
            //  only will write to it if we get a new replication.
            //
            tCIDLib::TCard4 c4Version = 0;
            tCIDLib::ELoadRes eRes = cfgscLoad.eReadObject
            (
                m_strCfgScope, m_dcfgCurrent, c4Version
            );

            //
            //  If we failed, then keep waiting. If we got new data or
            //  not data, either way we are now in sync and are good to
            //  go.
            //
            if (eRes != tCIDLib::ELoadRes::NotFound)
                bRes = kCIDLib::True;
        }
         else
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(L"No existing configuration");

            // None to load, so we are good
            bRes = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        // Just eat it, and return false to indicate we are still waiting
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    if (bRes && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        ZWLogDbgMsg(L"Got existing configuration, updating field list");
    return bRes;
}


tCQCKit::ECommResults
TZWaveUSBSDriver::eBoolFldValChanged(const  TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    // Make sure we aren't replicating
    CheckReplicating();

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg2(L"Got bool field write. Name=%(1) Value=%(2)", strName, TCardinal(bNewValue));

    //
    //  This could be a group or a unit. We stored the first/last group id
    //  so we can tell.
    //
    if ((c4FldId >= m_c4FirstGrpId) && (c4FldId <= m_c4LastGrpId))
    {
        // Look for the group that contains this field id
        TZWaveGroupInfo* pgrpiTar = m_dcfgCurrent.pgrpiFindByFldId(c4FldId);
        if (!pgrpiTar)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                ZWLogDbgMsg1
                (
                    L"Field id to group id xlat failed. Fld Id=%(1)"
                    , TCardinal(c4FldId)
                );
            }
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }

        // Ok, looks reasonable, so let's send the message
        DoGroupOnOff(*pgrpiTar, bNewValue);
    }
     else
    {
        tZWaveUSBSh::TUnitList& colCurUnits = m_dcfgCurrent.colUnits();
        tCIDLib::TCard4 c4UnitCount = colCurUnits.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4UnitCount; c4Index++)
        {
            TZWaveUnit* punitCur = colCurUnits[c4Index];

            // Look for the unit that owns this field
            if (punitCur->bOwnsFld(c4FldId))
            {
                // Looks ok, so send a unit on/off to it
                DoUnitSet
                (
                    *punitCur, tZWaveUSBSh::EUSetCmd_OffOn, bNewValue ? 1 : 0
                );
                break;
            }
        }

        // If we didn't find anybody who claims this field, then reject
        if (c4Index == c4UnitCount)
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg1(L"Field write successed for field %(1)", strName);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TZWaveUSBSDriver::eCardFldValChanged(const   TString&       strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)
{
    // Make sure we aren't replicating
    CheckReplicating();

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg2(L"Got card field write. Name=%(1) Value=%(2)", strName, TCardinal(c4NewValue));

    //
    //  This could be a group or a unit. We stored the first/last group id
    //  so we can tell.
    //
    if ((c4FldId >= m_c4FirstGrpId) && (c4FldId <= m_c4LastGrpId))
    {
        // Look for the group that contains this field id
        TZWaveGroupInfo* pgrpiTar = m_dcfgCurrent.pgrpiFindByFldId(c4FldId);
        if (!pgrpiTar)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg1(L"Field id to group id xlat failed. Fld Id=%(1)", TCardinal(c4FldId));

            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }

        // Ok, looks reasonable, so let's send the message
        DoGroupLevel(*pgrpiTar, tCIDLib::TCard1(c4NewValue));
    }
     else
    {
        tZWaveUSBSh::TUnitList& colCurUnits = m_dcfgCurrent.colUnits();
        tCIDLib::TCard4 c4UnitCount = colCurUnits.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4UnitCount; c4Index++)
        {
            TZWaveUnit* punitCur = colCurUnits[c4Index];

            // Look for the unit that owns this field
            if (punitCur->bOwnsFld(c4FldId))
            {
                // Looks ok, so do a level set on it
                DoUnitSet
                (
                    *punitCur
                    , tZWaveUSBSh::EUSetCmd_SetLevel
                    , tCIDLib::TCard1(c4NewValue)
                );
                break;
            }
        }

        // If we didn't find anybody who claims this field, then reject
        if (c4Index == c4UnitCount)
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg1(L"Field write successed for field %(1)", strName);

    return tCQCKit::ECommResults::Success;
}



//
//  Here we try to communicate with the controller. If we can connect, then
//  we figure out what units we are dealing with and set up our fields.
//
tCQCKit::ECommResults TZWaveUSBSDriver::eConnectToDevice(TThread& thrThis)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg(L"Trying to connect to Z-Wave controller");

    try
    {
        //
        //  Try to communicate with the controller and getting basic
        //  info from it.
        //
        QueryInitData();

        //
        //  We can talk to the controller. We need to call a method that will
        //  go look at all of the existing modules defined on the network
        //  and validate our existing unit configuration (if any) against
        //  what we found.
        //
        //  Tell it not to get a new node map, since we just got one above.
        //  Tell it to re-verify all existing units against the actual type
        //  of their nodes. And tell it not to discover new nodes, just validate
        //  our existing config.
        //
        if (bValidateNodeCfg(m_dcfgCurrent, kCIDLib::False, kCIDLib::True, kCIDLib::False))
            RegisterFields(m_dcfgCurrent);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TZWaveUSBSDriver::eInitializeImpl()
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg(L"Initializing Z-Wave driver");

    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCSerialConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
        = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Clean up any existing port so it'll get redone with new config
    delete m_pcommZWave;
    m_pcommZWave = 0;

    // Setup our cfg scope by putting in our moniker
    m_strCfgScope = kZWaveUSBS::pszCfgScope;
    m_strCfgScope.eReplaceToken(cqcdcOurs.strMoniker(), kCIDLib::chLatin_m);

    //
    //  Set our poll and reconnect times to something desirable for us. We
    //  set the poll time to the lowest possible, so that we can get incoming
    //  messages out of the controller ASAP. It has a limited buffer size
    //  and we don't want it to overflow.
    //
    SetPollTimes(10, 5000);

    // Indicate that we need to load configuration
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TZWaveUSBSDriver::ePollDevice(TThread& thrCaller)
{
    try
    {
        //
        //  If we aren't replicating, then check to see if have any polling
        //  to do. If we go for over 15 seconds without anything to poll,
        //  do a trivial query against the controller, to make sure it's
        //  still there.
        //
        if (!m_bRepMode)
        {
            DoPollCycle(thrCaller);

            // Update the polling load indicator
            const tCIDLib::TCard4 c4Count = m_colPollQ.c4ElemCount();
            if (c4Count > m_c4PollableCnt)
            {
                // Something is wrong, put it into error state
                SetFieldErr(m_c4FldId_PollingLoad, kCIDLib::True);
            }
             else
            {
                const tCIDLib::TCard4 c4PercentLoad = tCIDLib::TCard4
                (
                    (tCIDLib::TFloat4(c4Count) / m_c4PollableCnt) * 100.0
                );
                bStoreCardFld
                (
                    m_c4FldId_PollingLoad, c4PercentLoad, kCIDLib::True
                );
            }

            //
            //  If we go more than 60 seconds since we've had any
            //  interactions with the controller, then ping it to make
            //  sure he's alive.
            //
            const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
            if ((enctCur - m_enctLastMsg) > (kCIDLib::enctOneSecond * 60))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                    ZWLogDbgMsg(L"No activity recently, doing active ping");

                // Get some buffers from the pool
                TSimplePoolJan<THeapBuf> janOut(&m_splBufs, 512);
                TMemBuf& mbufOut = janOut.objGet();
                TSimplePoolJan<THeapBuf> janIn(&m_splBufs, 512);
                TMemBuf& mbufIn = janIn.objGet();

                // Just get the tx counter, which is a trivial operation
                mbufOut.PutCard1(5, 0);
                mbufOut.PutCard1(REQUEST, 1);
                mbufOut.PutCard1(FUNC_ID_MEMORY_GET_BYTE, 2);
                mbufOut.PutCard1(0, 3);
                mbufOut.PutCard1(0, 4);
                c4SendAndWaitForReply(mbufOut, 500, 4, mbufIn);

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                    ZWLogDbgMsg(L"Back from active ping");
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            ZWLogDbgMsg(L"Exception in polling");
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        //
        //  Assume the worst, since if they unplug it, and plug it back in,
        //  our currently opened port will not be valid anymore, so we want
        //  to recycle the port.
        //
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  We support a 'command' field that lets them send various commands via
//  a formatted string.
//
//  The commands are:
//
//      ALLON
//      ALLOFF
//      GROUP [name] (ON|OFF)|(RAMPUP|RAMPDN|RAMPEND)
//      UNIT [name] (ON|OFF)|(RAMPUP|RAMPDN|RAMPEND)
//      GROUPLEV [name] %level
//
tCQCKit::ECommResults
TZWaveUSBSDriver::eStringFldValChanged( const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    // Make sure we aren't replicating
    CheckReplicating();

    if (c4FldId == m_c4FldId_Command)
    {
        // Process the command
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg2(L"Got string field write. Name=%(1) Value=%(2)", strName, strNewValue);

        // We only have one string field, the command field, so reject if not that
        if (c4FldId != m_c4FldId_Command)
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }

        //
        //  Parse out the bits of the command. They are in the form:
        //
        //      Cmd [Target [options]]
        //
        //  where Target is the target group or unit, if applicable (which it is
        //  not for things like 'all on/off', and options are optional parameters
        //  that depend on the command. They are just space separated, so any
        //  parms with spaces must be quoted.
        //
        TVector<TString> colParms;
        const tCIDLib::TCard4 c4Count = TExternalProcess::c4BreakOutParms
        (
            strNewValue, colParms
        );

        // We have to have at least one parm
        if (!c4Count)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(L"No parameters provided to Command field");
            return tCQCKit::ECommResults::ValueRejected;
        }

        // Get the command out and see if it's a valid command
        TString& strCmd = colParms[0];
        strCmd.ToUpper();

        if ((strCmd == L"ALLON") || (strCmd == L"ALLOFF"))
        {
            CheckParmCount(strCmd, c4Count - 1, 0);

            //
            //  See if there is a group named AllOnOff. If so, we do that
            //  group. Else, we reject it.
            //
            TZWaveGroupInfo* pgrpiAll = m_dcfgCurrent.pgrpiFindByName
            (
                kZWaveUSBSh::strGrp_AllOnOff
            );
            if (!pgrpiAll)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    ZWLogDbgMsg(L"There is no AllOnOff group defined for ALLON or ALLOFF operations");
                return tCQCKit::ECommResults::ValueRejected;
            }

            // We found it. If it's empty, then we do nothing
            const tCIDLib::TCard4 c4UCount = pgrpiAll->c4UnitCount();
            if (!c4UCount)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    ZWLogDbgMsg(L"The AllOnOff group is empty, doing nothing");
                return tCQCKit::ECommResults::Success;
            }

            // Get a buffer from the pool
            TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
            TMemBuf& mbufOut = janPool.objGet();

            const tCIDLib::TBoolean bOn = (strCmd == L"ALLON");
            mbufOut.PutCard1(0, 0);
            mbufOut.PutCard1(REQUEST, 1);
            mbufOut.PutCard1(FUNC_ID_ZW_SEND_DATA_MULTI, 2);
            mbufOut.PutCard1(tCIDLib::TCard1(c4UCount), 3);

            // Put in the target units from the
            tCIDLib::TCard1 c1Cnt = 4;
            const tZWaveUSBSh::TIdList& fcolAll = pgrpiAll->fcolUnits();
            for (tCIDLib::TCard4 c4UInd = 0; c4UInd < c4UCount; c4UInd++)
                mbufOut.PutCard1(fcolAll[c4UInd], c1Cnt++);

            //
            //  DO NOT set the transmit ack option here. There are usually too
            //  many units and causes trouble.
            //
            mbufOut.PutCard1(sizeof(ZW_SWITCH_ALL_ON_FRAME), c1Cnt++);
            mbufOut.PutCard1(COMMAND_CLASS_SWITCH_ALL, c1Cnt++);
            mbufOut.PutCard1(bOn ? SWITCH_ALL_ON : SWITCH_ALL_OFF, c1Cnt++);
            mbufOut.PutCard1(TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
            mbufOut.PutCard1(c1NextCallbackId(), c1Cnt++);

            // Put the full length in now and send
            mbufOut.PutCard1(c1Cnt, 0);

            //
            //  Send, and tell it not to wait for secondary acks, since we
            //  did not set the transmit ack option above.
            //
            SendData(mbufOut, kCIDLib::False);
        }
         else if (strCmd == L"GROUP")
        {
            CheckParmCount(strCmd, c4Count - 1, 2);
            const TString& strName = colParms[1];
            TString& strOp = colParms[2];
            strOp.ToUpper();

            // Find the group
            const TZWaveGroupInfo& grpiTar = m_dcfgCurrent.grpiFindByName(strName);
            if ((strOp == L"ON") || (strOp == L"OFF"))
            {
                DoGroupOnOff(grpiTar, strOp == L"ON");
            }
             else if ((strOp == L"RAMPUP")
                  || (strOp == L"RAMPDN")
                  || (strOp == L"RAMPEND"))
            {
                DoGroupRamp(grpiTar, strOp);
            }
        }
         else if (strCmd == L"GROUPLEV")
        {
            // We expect a group name and percent level
            CheckParmCount(strCmd, c4Count - 1, 2);
            const TString& strName = colParms[1];
            tCIDLib::TCard4 c4Per = colParms[2].c4Val();

            //
            //  The legal values for Z-Wave multi-levels are 0 to 99, so
            //  we treat 100 and 99 the same by just clipping back to 99.
            //  We'll also clip anything larger than 100% back to 99 as
            //  well, without complaint.
            //
            if (c4Per > 99)
                c4Per = 99;

            // Find the group
            const TZWaveGroupInfo& grpiTar = m_dcfgCurrent.grpiFindByName(strName);
            DoGroupLevel(grpiTar, tCIDLib::TCard1(c4Per));
        }
         else if (strCmd == L"UNIT")
        {
            // We expect the unit name and the command
            CheckParmCount(strCmd, c4Count - 1, 2);
            const TString& strName = colParms[1];
            TString& strOp = colParms[2];
            strOp.ToUpper();

            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(strName);

            tZWaveUSBSh::EUSetCmds eCmd = tZWaveUSBSh::EUSetCmds_Count;
            tCIDLib::TCard1 c1Value = 0;
            if ((strOp == L"ON") || (strOp == L"OFF"))
            {
                eCmd = tZWaveUSBSh::EUSetCmd_OffOn;
                if (strOp == L"ON")
                    c1Value = 1;
            }
             else if ((strOp == L"RAMPUP") || (strOp == L"RAMPDN"))
            {
                eCmd = tZWaveUSBSh::EUSetCmd_RampStart;
                if (strOp == L"RAMPUP")
                    c1Value = 1;
            }
             else if (strOp == L"RAMPEND")
            {
                eCmd = tZWaveUSBSh::EUSetCmd_RampEnd;
            }
             else
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    ZWLogDbgMsg(L"Expected ON/OFF or a RAMPXX command");
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }

            // And send it out
            DoUnitSet(unitTar, eCmd, c1Value);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::ValueRejected;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TZWaveUSBSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommZWave->bIsOpen())
            m_pcommZWave->Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TZWaveUSBSDriver::TerminateImpl()
{
    try
    {
        //
        //  Make sure the replication thread isn't running. If it is
        //  then we need to stop it.
        //
        if (m_thrReplicate.bIsRunning())
        {
            m_thrReplicate.ReqShutdownSync(10000);
            m_thrReplicate.eWaitForDeath(5000);
        }
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called from the string field change callback, to throw an exception if
//  the parameters for the command field are wrong, to avoid duplicating
//  the code over and over for each command.
//
tCIDLib::TVoid
TZWaveUSBSDriver::CheckParmCount(const  TString&        strCmd
                                , const tCIDLib::TCard4 c4PCount
                                , const tCIDLib::TCard4 c4Expected)
{
    if (c4PCount != c4Expected)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            ZWLogDbgMsg2
            (
                L"Invalid parm count. Count=%(1), Expected=%(2)"
                , TCardinal(c4PCount), TCardinal(c4Expected)
            );
        }

        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_BadCmdParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4Expected)
            , strCmd
            , TCardinal(c4PCount)
        );
    }
}


// Checks to see if we are in replication mode and throws an error if so
tCIDLib::TVoid TZWaveUSBSDriver::CheckReplicating()
{
    if (m_bRepMode)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            ZWLogDbgMsg(L"Operation rejected because of replication mode");

        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_BusyRepMode
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , strMoniker()
        );
    }
}


//
//  Called when we receive an unsolicited callback
//
tCIDLib::TVoid
TZWaveUSBSDriver::HandleCallBack(const  tCIDLib::TCard1 c1CmdId
                                ,       TMemBuf&        mbufData
                                , const tCIDLib::TCard4 c4Bytes)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        ZWLogDbgMsg1
        (
            L"Got async callback. Cmd Id=%(1)"
            , TCardinal(c1CmdId, tCIDLib::ERadices::Hex)
        );

        #if CID_DEBUG_ON
        TString strIOLog(L"ZWCB-> ");
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Bytes; c4Index++)
        {
            strIOLog.AppendFormatted(mbufData[c4Index], tCIDLib::ERadices::Hex);
            strIOLog.Append(L" ");
        }
        ZWLogDbgMsg(strIOLog.pszBuffer());
        #endif
    }

    CIDAssert(mbufData[0] >= 4, L"Got an invalidly short async callback");

    // Based on the command type, handle the
    const tCIDLib::TCard1 c1TarNode = mbufData[4];
    switch(c1CmdId)
    {
        case FUNC_ID_APPLICATION_COMMAND_HANDLER :
        {
            // The number of bytes cannot be less than 2 (the cmd class and the cmd)
            const tCIDLib::TCard4 c4CmdBytes = mbufData[5];
            if (c4CmdBytes < 2)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                    ZWLogDbgMsg(L"Got an async response shorter than 2 bytes, ignoring it");
                return;
            }

            // Pass it on to the target node if we have it
            TZWaveUnit* punitTar = m_dcfgCurrent.punitFindById(c1TarNode);
            if (punitTar)
                punitTar->HandleAppCmd(mbufData.pc1DataAt(6), c4CmdBytes, *this);
            break;
        }

        default :
            break;
    };
}


//
//  Called when we receive an unsolicited response
//
tCIDLib::TVoid
TZWaveUSBSDriver::HandleResponse(const  tCIDLib::TCard1 c1CmdId
                                ,       TMemBuf&        mbufData
                                , const tCIDLib::TCard4 c4Bytes)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        ZWLogDbgMsg1
        (
            L"Got async response. Cmd Id=%(1)"
            , TCardinal(c1CmdId, tCIDLib::ERadices::Hex)
        );

        #if CID_DEBUG_ON
        TString strIOLog(L"ZWRES-> ");
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Bytes; c4Index++)
        {
            strIOLog.AppendFormatted(mbufData[c4Index], tCIDLib::ERadices::Hex);
            strIOLog.Append(L" ");
        }
        ZWLogDbgMsg(strIOLog.pszBuffer());
        #endif
    }

}


tCIDLib::TVoid
TZWaveUSBSDriver::ZWLogDbg( const   tCIDLib::TCh* const pszFile
                            , const tCIDLib::TCard4     c4Line
                            , const tCIDLib::TCh* const pszToLog
                            , const MFormattable&       fmtbl1
                            , const MFormattable&       fmtbl2
                            , const MFormattable&       fmtbl3)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        TString strMsg(L"ZWDebug: ", pszToLog);
        if (!MFormattable::bIsNullObject(fmtbl1))
            strMsg.eReplaceToken(fmtbl1, L'1');
        if (!MFormattable::bIsNullObject(fmtbl2))
            strMsg.eReplaceToken(fmtbl2, L'2');
        if (!MFormattable::bIsNullObject(fmtbl3))
            strMsg.eReplaceToken(fmtbl2, L'3');

        facZWaveUSBS().LogMsg
        (
            pszFile
            , c4Line
            , strMsg
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}



//
//  This method is called with a new driver configuration. We use this info
//  to register our fields. There are a set of them that we always do. But
//  the bulk of them are driven by the unit and group configuration info in
//  the passed object.
//
tCIDLib::TVoid
TZWaveUSBSDriver::RegisterFields(TZWaveDrvConfig& dcfgNew)
{
    // Build up a field list based on the new config
    TVector<TCQCFldDef> colFlds(64, kCQCKit::c4MaxDrvFields);
    TCQCFldDef flddNew;

    // Flush our poll queue any time this happens
    m_colPollQ.RemoveAll();

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        ZWLogDbgMsg(L"Updating field list...");

    //
    //  Add some fixed fields. We need a command field to take free form
    //  commands, and some that provide read-only access to some device
    //  or network info.
    //
    flddNew.Set
    (
        kZWaveUSBSh::strFld_Command
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , kCIDLib::True
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"ZW_HomeId"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"ZW_UnitId"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"PollingLoad"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range: 0, 100"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"ZW_SerialAPIVer"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"ZW_CtrlVersion"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Create the fields for all of the units and groups that we find in
    //  the passed configuration.
    //
    //  We update the pollable unit count while doing the units.
    //
    tZWaveUSBSh::TGroupList& colGroups = dcfgNew.colGroups();
    tZWaveUSBSh::TUnitList& colUnits = dcfgNew.colUnits();
    const tCIDLib::TCard4 c4GroupCnt = colGroups.c4ElemCount();
    const tCIDLib::TCard4 c4UnitCnt = colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4GroupInd = 0; c4GroupInd < c4GroupCnt; c4GroupInd++)
    {
        const TZWaveGroupInfo& grpiCur = colGroups[c4GroupInd];

        //
        //  Add a boolean for the group. It's write only and write always,
        //  just to allow the group to be turned on and off as a whole.
        //
        flddNew.Set
        (
            grpiCur.strName()
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Write
            , kCIDLib::True
        );
        colFlds.objAdd(flddNew);
    }

    m_c4PollableCnt = 0;
    for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
    {
        const TZWaveUnit& unitCur = *colUnits[c4UnitInd];

        // Allow the unit object to add its fields to the list
        unitCur.c4QueryFldDefs(colFlds, flddNew);

        if (unitCur.bHasCap(tZWaveUSBSh::EUnitCap_Pollable))
            m_c4PollableCnt++;
    }

    // Ok, let's set our fields and look them up
    {
        // This guy will do a field lock for us
        SetFields(colFlds);

        // Bump the reconfigured field while we are locked
        IncReconfigCounter();

        //
        //  Go back and look up all the group and unit field ids and store
        //  them in their respective group/unit objects. In order to allow
        //  the unit objects to find their fields, we have to look up all
        //  of the ids and put them into a vector. They can then go through
        //  the field list and find their fields, and the id for that field
        //  will be in the same index in the id list.
        //
        tCIDLib::TCard4 c4CurId;
        const tCIDLib::TCard4 c4FldCnt = colFlds.c4ElemCount();
        tZWaveUSBSh::TFldIdList fcolIds(c4FldCnt);
        for (tCIDLib::TCard4 c4FInd = 0; c4FInd < c4FldCnt; c4FInd++)
        {
            fcolIds.c4AddElement
            (
                pflddFind(colFlds[c4FInd].strName(), kCIDLib::True)->c4Id()
            );
        }

        for (tCIDLib::TCard4 c4UnitInd = 0; c4UnitInd < c4UnitCnt; c4UnitInd++)
        {
            TZWaveUnit& unitCur = *colUnits[c4UnitInd];
            if (!unitCur.bStoreFldIds(colFlds, fcolIds))
            {
                // Log this and mark it as failed
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    ZWLogDbgMsg1
                    (
                        L"Unit '%(1)' did not find its field ids. Marking it failed"
                        , unitCur.strName()
                    );
                }
                unitCur.SetFailedState();
            }
        }

        m_c4FirstGrpId = kCIDLib::c4MaxCard;
        m_c4LastGrpId = 0;
        for (tCIDLib::TCard4 c4GroupInd = 0; c4GroupInd < c4GroupCnt; c4GroupInd++)
        {
            TZWaveGroupInfo& grpiCur = colGroups[c4GroupInd];
            c4CurId = pflddFind(grpiCur.strName(), kCIDLib::True)->c4Id();
            grpiCur.c4FldId(c4CurId);

            // Remember the first/last group field id
            if (c4CurId < m_c4FirstGrpId)
                m_c4FirstGrpId = c4CurId;
            if (c4CurId > m_c4LastGrpId)
                m_c4LastGrpId = c4CurId;
        }

        // Look up any others that we do by id
        m_c4FldId_Command = pflddFind(kZWaveUSBSh::strFld_Command, kCIDLib::True)->c4Id();
        m_c4FldId_PollingLoad = pflddFind(L"PollingLoad", kCIDLib::True)->c4Id();

        //
        //  Go ahead and set the home/unit id now, since they never change.
        //  We can just set them by name and not look up their ids.
        //
        bStoreCardFld(L"ZW_HomeId", m_c4HomeId, kCIDLib::True);
        bStoreCardFld(L"ZW_UnitId", m_c1CtrlId, kCIDLib::True);
        bStoreCardFld(L"ZW_SerialAPIVer", m_c1SerAPIVersion, kCIDLib::True);
        bStoreStringFld(L"ZW_CtrlVersion", m_strCtrlVersion, kCIDLib::True);
    }

    //
    //  If the passed object isn't our active configuration data object, then
    //  we need to store the passed info as the current info.
    //
    if (&dcfgNew != &m_dcfgCurrent)
    {
        m_dcfgCurrent = dcfgNew;
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg(L"Stored new field configuration");
    }
}


