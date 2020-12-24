//
// FILE NAME: ZWaveUSBS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/04/2007
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
//  This file contains some overflow grunt work methods from the driver
//  implementation class. This one mostly contains stuff related to reading
//  and writing messages, waiting for replies and so forth.
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

// #define  ZWSHOWIO


// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Does the standard Z-Wave checksum calculation. It sets a Card1 to 0xFF,
//  then XORs each byte between the SOF and checksum (non-inclusive.) The
//  buffer passed already has the SOF and checksum removed, so we do the
//  whole buffer.
//
tCIDLib::TCard1 TZWaveUSBSDriver::c1CalcSum(const TMemBuf& mbufMsg) const
{
    tCIDLib::TCard1 c1Ret = 0xFF;
    const tCIDLib::TCard4 c4Count = mbufMsg[0];
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        c1Ret ^= mbufMsg[c4Index];
    return c1Ret;
}


//
//  Sometimes we just want to pause and let things settle, but we need to
//  process messages so that we will ack and process any incoming stuff.
//  This method handles that. Caller provides us with a buffer to use.
//
tCIDLib::TVoid
TZWaveUSBSDriver::DelayAndProcess(  const   tCIDLib::TCard4 c4WaitFor
                                    ,       TMemBuf&        mbufIn)
{
    //
    //  If it's a very small number, and it might be because sometimes they'll
    //  call us to eat up the remainder of a minimum inter-whatever interval,
    //  then just sleep. If zero, then do nothing.
    //
    if (!c4WaitFor)
        return;

    if (c4WaitFor < 25)
    {
        TThread::Sleep(c4WaitFor);
        return;
    }

    // It's long enough to bother with
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec);

    tCIDLib::TCard1 c1CmdId;
    tCIDLib::TCard1 c1CBId;
    tCIDLib::TCard4 c4Read;
    while (enctCur < enctEnd)
    {
        // Tell him not to throw on a timeout or failure
        const tZWaveUSBS::EMsgTypes eRet = eGetReply
        (
            c4Read, mbufIn, enctEnd - enctCur, c1CmdId, c1CBId, kCIDLib::False
        );

        // If a data packet, then process it
        if (eRet == tZWaveUSBS::EMsgType_CallBack)
            HandleCallBack(c1CmdId, mbufIn, c4Read);
        else if (eRet == tZWaveUSBS::EMsgType_Response)
            HandleResponse(c1CmdId, mbufIn, c4Read);

        enctCur = TTime::enctNow();
    }
}


//
//  This method will get a reply from the controller, waiting for up to the
//  indicated time. It can optionally throw if no input is available. It
//  returns an enum that indicates the type of reply (or lack thereof.)
//
tZWaveUSBS::EMsgTypes
TZWaveUSBSDriver::eGetReply(        tCIDLib::TCard4&        c4Read
                            ,       TMemBuf&                mbufToFill
                            , const tCIDLib::TEncodedTime   enctWaitFor
                            ,       tCIDLib::TCard1&        c1CmdId
                            ,       tCIDLib::TCard1&        c1CBId
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    // Do a little state machine to get the reply
    enum EStates
    {
        EState_WaitSOF
        , EState_Length
        , EState_DataBytes
        , EState_CheckSum
        , EState_Done
    };

    tCIDLib::TBoolean       bExtended = kCIDLib::False;
    tCIDLib::TCard1         c1Cur;
    tCIDLib::TCard1         c1Sum;
    tCIDLib::TCard4         c4Expected;
    EStates                 eState = EState_WaitSOF;
    tCIDLib::TEncodedTime   enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime   enctEnd = enctCur + enctWaitFor;

    while (eState < EState_Done)
    {
        if (m_pcommZWave->c4ReadRawBuf(&c1Cur, 1, enctEnd - enctCur))
        {
            switch(eState)
            {
                case EState_WaitSOF :
                {
                    if (c1Cur == SOF)
                    {
                        // Reset the byte counter and move to length state
                        eState = EState_Length;
                        c4Read = 0;
                    }
                     else if (c1Cur == ACK)
                    {
                        return tZWaveUSBS::EMsgType_Ack;
                    }
                     else if (c1Cur == NAK)
                    {
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(L"Got a NAK message");
                        {
                            IncNakCounter();
                        }
                        return tZWaveUSBS::EMsgType_Nak;
                    }
                    break;
                }

                case EState_Length :
                {
                    // Store the expected length and move to the next state
                    c4Expected = tCIDLib::TCard4(c1Cur);
                    mbufToFill.PutCard1(c1Cur, c4Read++);
                    if (c4Expected)
                        eState = EState_DataBytes;
                    else
                        eState = EState_CheckSum;
                    break;
                }

                case EState_DataBytes :
                {
                    // Collect data bytes until we get them all
                    mbufToFill.PutCard1(c1Cur, c4Read++);
                    if (c4Read == c4Expected)
                        eState = EState_CheckSum;
                    break;
                }

                case EState_CheckSum :
                {
                    // Calc the sum on our data and compare
                    c1Sum = c1CalcSum(mbufToFill);

                    // If not the same, then we failed right now
                    if (c1Sum != c1Cur)
                    {
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(L"Got bad checksum");

                        {
                            IncBadMsgCounter();
                        }

                        // Send a NAK and keep trying
                        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                            ZWLogDbgMsg(L"Bad message checksum, sending Nak");
                        SendAckNak(kCIDLib::False);
                        eState = EState_WaitSOF;
                    }
                     else
                    {
                        eState = EState_Done;
                    }
                    break;
                }
            };
        }

        if (eState != EState_Done)
        {
            // Update the current time
            enctCur = TTime::enctNow();

            //
            //  If we are out of time, but we have got a partial message, then
            //  extend the time a bit. We'll just do this once. If we are still
            //  on WaitSOF or we've already extended, then we failed.
            //
            if (enctCur >= enctEnd)
            {
                if ((eState != EState_WaitSOF) && !bExtended)
                {
                    enctEnd += (250 * kCIDLib::enctOneMilliSec);
                    bExtended = kCIDLib::True;
                }
                 else
                {
                    // Oh well, give up
                    break;
                }
            }
        }
    }

    //
    //  If we got here and the state is 'done', then we got a data frame
    //  that needs to be acked.
    //
    tZWaveUSBS::EMsgTypes eRet;
    if (eState == EState_Done)
    {
        SendAckNak(kCIDLib::True);

        // Update the last message time
        m_enctLastMsg = TTime::enctNow();

        // Fill in the command id
        c1CmdId = mbufToFill[2];

        //
        //  Figure out if it's a response or a callback. If it's a callback
        //  then fill in the callback id, else zero it.
        //
        if (mbufToFill[1] == 0)
        {
            eRet = tZWaveUSBS::EMsgType_CallBack;
            c1CBId = mbufToFill[3];
        }
         else
        {
            eRet = tZWaveUSBS::EMsgType_Response;
            c1CBId = 0;
        }

        #if CID_DEBUG_ON
        #if defined(ZWSHOWIO)
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            TString strIOLog(L"ZW-> ");
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Read; c4Index++)
            {
                strIOLog.AppendFormatted(mbufToFill[c4Index], tCIDLib::ERadices::Hex);
                strIOLog.Append(L" ");
            }
            ZWLogDbgMsg(strIOLog.pszBuffer());
        }
        #endif
        #endif
    }
     else
    {
        // We timed out, so throw if they said to, else return failure
        if (bThrowIfNot)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                ZWLogDbgMsg(L"Timed out waiting for a reply, throwing");
            facZWaveUSBS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcProto_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::LostConnection
            );
        }
        eRet = tZWaveUSBS::EMsgType_Timeout;
    }
    return eRet;
}



//
//  Just sends an ack or nack frame, and makes sure it goes out ok.
//
tCIDLib::TVoid TZWaveUSBSDriver::SendAckNak(const tCIDLib::TBoolean bAck)
{
    tCIDLib::TCard1 c1Send = bAck ? ACK : NAK;
    if (m_pcommZWave->c4WriteRawBufMS(&c1Send, 1, 1000) != 1)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_SendFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , TCardinal(ACK, tCIDLib::ERadices::Hex)
        );
    }
}


//
//  Just sends the passed message If the message cannot be sent, it throws.
//
tCIDLib::TVoid TZWaveUSBSDriver::SendMsg(const TMemBuf& mbufOut)
{
    // Calculate the check sum for the message
    const tCIDLib::TCard1 c1Sum = c1CalcSum(mbufOut);

    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufSend = janPool.objGet();

    // Build up the message
    tCIDLib::TCard1 c1ToSend = mbufOut[0];
    mbufSend.PutCard1(SOF, 0);
    mbufSend.CopyIn(mbufOut, c1ToSend, 1);
    mbufSend.PutCard1(c1Sum, c1ToSend + 1);
    c1ToSend += 2;

    #if CID_DEBUG_ON
    #if defined(ZWSHOWIO)
    TString strIOLog(L"CQ-> ");
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c1ToSend; c4Index++)
    {
        strIOLog.AppendFormatted(mbufSend[c4Index], tCIDLib::ERadices::Hex);
        strIOLog.Append(L" ");
    }
    ZWLogDbgMsg(strIOLog.pszBuffer());
    #endif
    #endif

    // Send the buffer
    if (m_pcommZWave->c4WriteMBufMS(mbufSend, c1ToSend, 1000) != c1ToSend)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_SendFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::LostConnection
            , TCardinal(mbufOut[1], tCIDLib::ERadices::Hex)
        );
    }
}


