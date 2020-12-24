//
// FILE NAME: HAIOmniTCP2S_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  This file contains yet still more grunt work code to keep the main
//  file clean and the second from getting too large. This one contains
//  just low level stuff, related to session creation and reading and
//  writing messages.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2S.hpp"



// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will establish an encrypted session with the Omni. This is
//  all special case stuff very different from the normal conversation.
//
tCIDLib::TBoolean THAIOmniTCP2S::bCreateSession()
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniTCP2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Attempting to create session"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    tCIDLib::TCard1 c1PacketType;
    tCIDLib::TCard4 c4MsgBytes;
    try
    {
        // Get a buffer to use
        THeapBufPool::TElemJan janPool(&m_splBufs, 512);
        TMemBuf& mbufIn = *janPool;

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCP2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Requesting new session"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Send a new session request
        SendPacket(kHAIOmniTCP2S::c1PackType_ReqNewSess);

        // Wait for an ack or refusal
        bGetPacket
        (
            mbufIn
            , c1PacketType
            , c4MsgBytes
            , 5000
            , m_c2LastSeq
            , kCIDLib::True
            , kCIDLib::False
        );

        if (c1PacketType != kHAIOmniTCP2S::c1PackType_AckNewSess)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCP2S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCP2SErrs::errcProto_NoStartSesAck
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c1PacketType)
                );
            }
            return kCIDLib::False;
        }

        //
        //  It should be 7 bytes, the last 5 of which need to be XORd with
        //  the last five bytes of the private key, and that creates the
        //  actual session key.
        //
        if (c4MsgBytes != 7)
        {
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_BadSessAckSize
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4MsgBytes)
            );
        }

        // Create the session key
        tCIDLib::TCard1 ac1Key[16];
        m_mhashKey.c4ToRawArray(ac1Key, 16);
        const tCIDLib::TCard1* pc1Src = mbufIn.pc1Data() + 2;
        tCIDLib::TCard1* pc1Tar = ac1Key + 11;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < 5; c4Index++)
            *pc1Tar++ ^= *pc1Src++;

        // Set up the key now and set it on the encrypter
        TCryptoKey ckeySession(ac1Key, 16);
        m_crypComm.SetNewKey(ckeySession);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCP2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Attempting to start encrypted session"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // Ask to start the encrypted session, and get the ack/nak
        SendPacket(mbufIn.pc1Data() + 2, kHAIOmniTCP2S::c1PackType_ReqSecSess, 5);

        // Wait for an ack or refusal
        bGetPacket
        (
            mbufIn
            , c1PacketType
            , c4MsgBytes
            , 5000
            , m_c2LastSeq
            , kCIDLib::True
            , kCIDLib::False
        );

        if (c1PacketType != kHAIOmniTCP2S::c1PackType_AckSecSess)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCP2S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCP2SErrs::errcProto_NoSecSesAck
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c1PacketType)
                );
            }
            return kCIDLib::False;
        }

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCP2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Session was established"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCP2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"An exception occured while attempting to start session"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This method watches for a new msg to arrive, makes sure it's not a
//  dup or that we haven't missed any msgs, and if it's an Omni level msg
//  it decrypts the payload and verifies the CRC.
//
//  It'll wait for up to c4WaitFor millis, then either throw or return false
//  according to the bThrowIfNot flag. It returns the type of msg and the
//  payload bytes of the msg, which normally will either be nothing or the
//  Omni message recieved.
//
//  The protocol is kind of strange in that it has no sync byte or anything.
//  The first thing we get is a sequence number, which must match the one
//  we were provided. And there's nothing in the header to indicate how
//  much data to read. For the non-Omni level messages, which all happen
//  during the early session setup stuff, all we can do is just wait for
//  data to arrive and read what's there. He should never send us more than
//  one message at a time, so anything that shows up should be a message,
//  and we can only hope that their packets are never fragmented, i.e. that
//  once anything shows up, the whole thing is there.
//
//  Once we get the session going, then it's all Omni level messages, and we
//  can get more tahn one in the queue, because of async notifications. In
//  this case, it's even worse. All we can do is issue an initial read for
//  20 bytes. This gets us the header and enough to decode one block of the
//  the encrypted Omni payload data. That has the number of bytes in it,
//  so we can use that to read the rest of the message (accounting for the
//  16 byte AES block size so we have to pad up the expected byte count
//  to an even 16 bytes.)
//
//  To make this easier for us, the caller tells us if he is expecting an
//  Omni level packet or not.
//
tCIDLib::TBoolean
THAIOmniTCP2S::bGetPacket(          TMemBuf&            mbufToFill
                            ,       tCIDLib::TCard1&    c1PacketType
                            ,       tCIDLib::TCard4&    c4MsgBytes
                            , const tCIDLib::TCard4     c4WaitFor
                            , const tCIDLib::TCard2     c2ExpSeq
                            , const tCIDLib::TBoolean   bThrowIfNot
                            , const tCIDLib::TBoolean   bOmniMsg)
{
    tCIDLib::TCard2 c2Seq;
    tCIDLib::TCard4 c4PacketSz;

    //
    //  Wait for data to become available. If something shows up, we should
    //  get the whole message (see comments above.)
    //
    if (!m_sockOmni.bWaitForDataReadyMS(c4WaitFor))
    {
        if (!bThrowIfNot)
            return kCIDLib::False;

        IncTimeoutCounter();
        facHAIOmniTCP2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcProto_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
        );

    }

    // Get a buffer for the raw data we read in
    THeapBufPool::TElemJan janPool(&m_splBufs, 1024);
    TMemBuf& mbufRaw = *janPool;

    //
    //  Now we can do a read. Out of paranoia, we still do a non-blocking
    //  read, just in case. In theory we know data is there, but never take
    //  any chance on flakey TCP/IP stacks and such.
    //
    if (bOmniMsg)
    {
        //
        //  Just read 20 byte's worth and it has to all be there. Given that
        //  it's too little to be fragmented, if we saw any data ready, it
        //  has to all be available.
        //
        c4PacketSz = m_sockOmni.c4ReceiveMBufTOMS
        (
            mbufRaw, 100, 20, tCIDLib::EAllData::FailIfNotAll
        );
    }
     else
    {
        //
        //  It's not an Omni level message, so we just want to read what's
        //  there, so issue a non-blocking read with a short timeout for the
        //  maximum packet size, and take whatever's there.
        //
        c4PacketSz = m_sockOmni.c4ReceiveMBufTOMS
        (
            mbufRaw
            , 100
            , kHAIOmniTCP2S::c4MaxPacketSize
            , tCIDLib::EAllData::OkIfNotAll
        );

        // If nothing, then we timed out
        if (!c4PacketSz)
        {
            if (!bThrowIfNot)
                return kCIDLib::False;

            IncTimeoutCounter();
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }

        // If less than a possibly whole packet, that's bad
        if (c4PacketSz < kHAIOmniTCP2S::c4MinPacketSize)
        {
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_PartialPacket
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
    }

    //
    //  We got one, so pull out the sequence number and see if we need to
    //  reject it.
    //
    #if defined(CIDLIB_LITTLEENDIAN)
    c2Seq = TRawBits::c2SwapBytes(mbufRaw.c2At(0));
    #else
    c2Seq = mbufRaw.c2At(0));
    #endif

    //
    //  If the sequence number is 0, then it's an async, so we always
    //  take it. Otherwise, we will make sure it's the requested sequence
    //  number.
    //
    if (c2Seq && (c2Seq != c2ExpSeq))
    {
        facHAIOmniTCP2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcProto_OutOfSync
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutOfSync
            , TCardinal(c2ExpSeq)
            , TCardinal(c2Seq)
        );
    }

    //
    //  If we got here, then we got a valid new packet, so let's process it.
    //  Get the packet type out
    //
    c1PacketType = mbufRaw.c1At(2);

    //
    //  If it's an Omni message, then we have to decrypt it. We read just
    //  one packet's worth above, so first we have to decrypt that one
    //  packet, which tells us how much more we need to read.
    //
    if (c1PacketType == kHAIOmniTCP2S::c1PackType_OmniMsg)
    {
        //
        //  Get the two sequence bytes out since we'll use them in each
        //  round below.
        //
        const tCIDLib::TCard1 c1HighSeq = mbufRaw[0];
        const tCIDLib::TCard1 c1LowSeq = mbufRaw[1];

        //
        //  Get a raw pointer to the source buffer that we are going to
        //  from.
        //
        const tCIDLib::TCard1* pc1Src = mbufRaw.pc1Data();

        // Decrypt a first block (skipping the header bytes in the src!)
        m_crypComm.c4Decrypt(pc1Src + 4, m_ac1Plain, 16, 16);
        m_ac1Plain[0] ^= c1HighSeq;
        m_ac1Plain[1] ^= c1LowSeq;

        mbufToFill.CopyIn(m_ac1Plain, 16, 0);
        tCIDLib::TCard4 c4BytesSoFar = 16;

        //
        //  OK, now we can get the size of the Omni payload out (minus any
        //  padding.) We have to add four to include the start byte, msg size
        //  byte, and CRC bytes, which are not included in this value.
        //
        c4MsgBytes = m_ac1Plain[1] + 4;

        //
        //  If it's more than 16 bytes, then we have to read some more. We'll
        //  read back into the start of the raw buffer since we've already
        //  processed the first block that we read before, so we don't have to
        //  adjust the pc1Src pointer either.
        //
        if (c4MsgBytes > 16)
        {
            //
            //  We also have to adjust for padding. If it's not an even multiple
            //  of 16, then pad up to the next 16. This minus the header we already
            //  read will be what we need to read in now.
            //
            const tCIDLib::TCard4 c4SzModulus = c4MsgBytes % 16;
            tCIDLib::TCard4 c4RawSz = c4MsgBytes;
            if (c4SzModulus)
                c4RawSz += (16 - c4SzModulus);

            //
            //  We know how much we need, so we say fail if not all and we
            //  don't care about the return value. We've already read 16
            //  bytes so subtract that much.
            //
            m_sockOmni.c4ReceiveMBufTOMS
            (
                mbufRaw, 1000, c4RawSz - 16, tCIDLib::EAllData::FailIfNotAll
            );

            //
            //  If we got it all in the first block above, then we won't even
            //  enter this loop. If there's more to do, we'll do it now.
            //
            while(c4BytesSoFar < c4RawSz)
            {
                // Decrypt a block
                m_crypComm.c4Decrypt(pc1Src, m_ac1Plain, 16, 16);

                // And swizzle it with the sequence number bytes
                m_ac1Plain[0] ^= c1HighSeq;
                m_ac1Plain[1] ^= c1LowSeq;

                mbufToFill.CopyIn(m_ac1Plain, 16, c4BytesSoFar);
                c4BytesSoFar += 16;
                pc1Src += 16;
            }
        }

        //
        //  Calc the CRC and compare it to what we got.
        //
        //  NOTE:   There was a bug in the 2.16 firmware, which sends the CRC
        //          in the wrong order. So check it both ways just to be sure.
        //
        //          ALSO, it sometimes sends message without the CRC, just
        //          zeros. So we only verify the CRC if the one in the msg
        //          is non-zero.
        //
        const tCIDLib::TCard2 c2CalcedCRC = c2CalcCRC(mbufToFill);
        const tCIDLib::TCard2 c2FlippedCRC = TRawBits::c2SwapBytes(c2CalcedCRC);
        const tCIDLib::TCard2 c2MsgCRC = mbufToFill.c2At(c4MsgBytes - 2);
        if (c2MsgCRC && (c2MsgCRC != c2CalcedCRC) && (c2MsgCRC != c2FlippedCRC))
        {
            IncBadMsgCounter();
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_BadCRC
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // If it's a nack, then throw
        if (mbufToFill[kHAIOmniTCP2S::c4MsgTypeOfs] == kHAIOmniTCP2S::c1MsgType_Nak)
        {
            IncNakCounter();
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_Nak
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
            );
        }
    }
     else
    {
        // Make sure it's not larger than the maximum message size
        c4MsgBytes = c4PacketSz - 4;
        if (c4MsgBytes > kHAIOmniTCP2S::c4MaxMsgSize)
        {
            IncBadMsgCounter();
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcProto_MsgTooBig
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4MsgBytes)
            );
        }

        // Not any Omni message, so just copy out any message bytes
        if (c4MsgBytes)
            mbufRaw.CopyOut(mbufToFill, c4MsgBytes, 4);
    }

    return kCIDLib::True;
}


//
//  Validates a passed code and returns the number of the code, which is
//  the form required for passing to the Omni in commands.
//
tCIDLib::TBoolean
THAIOmniTCP2S::bValidateCode(const  TString&            strCode
                            , const tCIDLib::TCard4     c4Area
                            ,       tCIDLib::TCard1&    c1CodeNum)
{
    // We need a buffer to format the outgoing, and wait for a response
    THeapBufPool::TElemJan janWrite(&m_splBufs, 512);
    TMemBuf& mbufWrite = *janWrite;

    THeapBufPool::TElemJan janRead(&m_splBufs, 512);
    TMemBuf& mbufRead = *janRead;

    //
    //  This is a very specialized message, so we just build it ourselves
    //  manually. We do cheap Unicode to ASCII transcoding via truncation
    //  since we know the codes can only be ASCII digits.
    //
    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(6, 1);
    mbufWrite.PutCard1(kHAIOmniTCP2S::c1MsgType_ValidateCodeReq, 2);
    mbufWrite.PutCard1(tCIDLib::TCard1(c4Area), 3);
    mbufWrite.PutCard1(tCIDLib::TCard1(strCode[0] - 0x30), 4);
    mbufWrite.PutCard1(tCIDLib::TCard1(strCode[1] - 0x30), 5);
    mbufWrite.PutCard1(tCIDLib::TCard1(strCode[2] - 0x30), 6);
    mbufWrite.PutCard1(tCIDLib::TCard1(strCode[3] - 0x30), 7);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 8);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 9);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 10);

    // And we should get a code validation response
#if CID_DEBUG_ON
    WaitReply(30000, kHAIOmniTCP2S::c1MsgType_ValidateCodeReply, 0, mbufRead);
#else
    WaitReply(0, kHAIOmniTCP2S::c1MsgType_ValidateCodeReply, 0, mbufRead);
#endif

    c1CodeNum = mbufRead[kHAIOmniTCP2S::c4FirstDBOfs];
    return (c1CodeNum != 0);
}


// Waits up to the indicated millis for an ack message
tCIDLib::TBoolean
THAIOmniTCP2S::bWaitAck(const tCIDLib::TBoolean bThrowIfTimeout)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janRead(&m_splBufs, 1024);
    TMemBuf& mbufRead = *janRead;

    try
    {
        //
        //  Call WaitReply so that we process asyncs while we are waiting
        //  for an ack.
        //
        WaitReply
        (
            kHAIOmniTCP2S::c4WaitAck, kHAIOmniTCP2S::c1MsgType_Ack, 0, mbufRead
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        //
        //  If it's just a timeout, then either rethrow it or just return
        //  false, according to the caller's wishes. Else log it and rethrow
        //  it.
        //
        if (errToCatch.bCheckEvent( facHAIOmniTCP2S().strName()
                                    , kOmniTCP2SErrs::errcProto_Timeout))
        {
            if (bThrowIfTimeout)
                throw;
            return kCIDLib::False;
        }
        LogError(errToCatch, tCQCKit::EVerboseLvls::High);
        throw;
    }
    return kCIDLib::True;
}


//
//  This one is used in interations where we listen for a specific reply for a specific
//  data type, or the end of data message. We return false when we get the end of
//  data, so the caller knows he can stop the interation.
//
tCIDLib::TBoolean
THAIOmniTCP2S::bWaitEnd(const   tCIDLib::TCard4     c4WaitFor
                        , const tCIDLib::TCard1     c1ExpMsg
                        , const tCIDLib::TCard1     c1ExpDataType
                        ,       TMemBuf&            mbufToFill)
{
    tCIDLib::TCard1 c1OmniType = 0;
    tCIDLib::TCard1 c1Type = 0;
    tCIDLib::TCard4 c4MsgBytes = 0;

    //
    //  We will keep waiting until we hit the end time. If they pass zero for
    //  the wait, then use the default.
    //
    const tCIDLib::TCard4 c4ActualWait
    (
        c4WaitFor ? c4WaitFor : kHAIOmniTCP2S::c4WaitReply
    );
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd(enctCur + (c4ActualWait * kCIDLib::enctOneMilliSec));

    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (enctCur < enctEnd)
    {
        const tCIDLib::TBoolean bGotOne = bGetPacket
        (
            mbufToFill, c1Type, c4MsgBytes, 250, m_c2LastSeq, kCIDLib::False
        );

        if (bGotOne)
        {
            // Break out if we get the expected message
            c1OmniType = mbufToFill[kHAIOmniTCP2S::c4MsgTypeOfs];
            if (c1Type == kHAIOmniTCP2S::c1PackType_OmniMsg)
            {
                // If the end of data, break out with the false result
                if (c1OmniType == kHAIOmniTCP2S::c1MsgType_EndOfData)
                    break;

                // If teh expected msg and data type, then break out with true
                if ((c1OmniType == c1ExpMsg) && (mbufToFill[3] ==  c1ExpDataType))
                {
                    bRet = kCIDLib::True;
                    break;
                }
            }

            // It's not what we are waiting for, so process it as an async
            ProcessAsync(c1OmniType, mbufToFill);

            // A little makeup for the time spent processing this async
            enctEnd += (kCIDLib::enctOneMilliSec * 5);
        }

        // Bump the time
        enctCur = TTime::enctNow();
    }

    // If we timed out, then we never got the expected message
    if (enctCur >= enctEnd)
    {
        facHAIOmniTCP2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcProto_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return bRet;
}



//
//  Calculate the Omni CRC for the passed message, which is assumed to be
//  well formed, since we get the length out of it.
//
tCIDLib::TCard2 THAIOmniTCP2S::c2CalcCRC(const TMemBuf& mbufSrc)
{
    tCIDLib::TBoolean       bFlag;
    const tCIDLib::TCard2   c2Poly = 0xA001;
    tCIDLib::TCard2         c2CRC = 0;
    const tCIDLib::TCard1*  pc1Src = mbufSrc.pc1Data() + 1;
    const tCIDLib::TCard1*  pc1End = pc1Src + (*pc1Src);
    while (pc1Src <= pc1End)
    {
        c2CRC ^= *(pc1Src++);
        for (tCIDLib::TCard4 c4Rnd = 0; c4Rnd < 8; c4Rnd++)
        {
            bFlag = (c2CRC & 1) != 0;
            c2CRC >>= 1;
            if (bFlag)
                c2CRC ^= c2Poly;
        }
    }
    return c2CRC;
}



// Converts from a C or F temp to an Omni temp
tCIDLib::TCard4
THAIOmniTCP2S::c4ToOmniTemp(const   tCIDLib::TInt4      i4Convert
                            , const tCIDLib::TBoolean   bCelsiusFmt)
{
    //
    //  First convert to C if needed.
    //
    tCIDLib::TFloat8 f8CTemp = i4Convert;
    if (!bCelsiusFmt)
        f8CTemp = ((f8CTemp - 32.0) * 5.0) / 9.0;

    // Find the nearest half integral degree
    tCIDLib::TFloat8 f8Int;
    tCIDLib::TFloat8 f8Frac = TMathLib::f8Split(f8CTemp, f8Int);
    f8CTemp = f8Int;

    if (f8Frac >= 0.75)
        f8CTemp += 1.0;
    else if (f8Frac >= 0.25)
        f8CTemp += 0.5;

    //
    //  And now convert to an Omni temp. We bias it first by adding 40
    //  which makes it a zero-based, positive number, then multiple by two
    //  to convert it to half degree increments.
    //
    return tCIDLib::TInt4(TMathLib::f8Ceil((f8CTemp + 40.0) * 2.0));
}



//
//  This method takes an Omni message payload, and encrypts it into a
//  msg buffer, and returns the actual full msg size (we might have
//  to pad up to a full AES block.)
//
tCIDLib::TCard4
THAIOmniTCP2S::c4EncryptPayload(const   tCIDLib::TCard1* const  pc1Payload
                                , const tCIDLib::TCard4         c4PLBytes
                                ,       TMemBuf&                mbufMsg)
{
    // We start after the msg header bytes and add our bytes to that
    tCIDLib::TCard4 c4RetSize = 4;
    if (c4PLBytes)
    {
        const tCIDLib::TCard1* pc1Src = pc1Payload;
        const tCIDLib::TCard1 c1HighSeq = mbufMsg[0];
        const tCIDLib::TCard1 c1LowSeq = mbufMsg[1];

        //
        //  Figure out how many full blocks we can do. We'll have to handle
        //  the last one specially by padding it.
        //
        const tCIDLib::TCard4 c4Blocks = c4PLBytes / 16;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Blocks; c4Index++)
        {
            TRawMem::CopyMemBuf(m_ac1Plain, pc1Src, 16);
            m_ac1Plain[0] ^= c1HighSeq;
            m_ac1Plain[1] ^= c1LowSeq;

            // Decrypt a block
            m_crypComm.c4Encrypt(m_ac1Plain, m_ac1Cypher, 16, 16);

            pc1Src += 16;
            mbufMsg.CopyIn(m_ac1Cypher, 16, c4RetSize);
            c4RetSize += 16;
        }

        // If there's a partial block do that
        const tCIDLib::TCard4 c4Partial = (c4PLBytes % 16);
        if (c4Partial)
        {
            TRawMem::SetMemBuf(m_ac1Plain, tCIDLib::TCard1(0), 16);
            TRawMem::CopyMemBuf(m_ac1Plain, pc1Src, c4Partial);
            m_ac1Plain[0] ^= c1HighSeq;
            m_ac1Plain[1] ^= c1LowSeq;
            m_crypComm.c4Encrypt(m_ac1Plain, m_ac1Cypher, 16, 16);
            mbufMsg.CopyIn(m_ac1Cypher, 16, c4RetSize);
            c4RetSize += 16;
        }
    }
    return c4RetSize;
}


//
//  Converts from an Omni temp to C or F temp. The caller provides the valid
//  min/max values (in raw Omni temp units so we don't have to do the
//  conversion to find out if it's bad and the caller doesn't have to figure
//  how what range to send the min/max in.) If the value is out of range,
//  we return i4MaxInt to tell the caller it's bad.
//
tCIDLib::TInt4
THAIOmniTCP2S::i4FromOmniTemp(  const   tCIDLib::TCard4     c4OmniTemp
                                , const tCIDLib::TBoolean   bCelsiusFmt
                                , const tCIDLib::TCard4     c4MinVal
                                , const tCIDLib::TCard4     c4MaxVal)
{
    // If out of range, then return i4MaxInt
    if ((c4OmniTemp < c4MinVal) || (c4OmniTemp > c4MaxVal))
        return kCIDLib::i4MaxInt;

    //
    //  First do celsius. The Omni temp is in half degrees, based on -40
    //  degrees. So divide by 2 for degrees, and subtract 40.
    //
    tCIDLib::TFloat8 f8Ret = (c4OmniTemp / 2.0) - 40.0;

    // If they don't want to Celsius, then convert to F
    if (!bCelsiusFmt)
        f8Ret = ((f8Ret * 9.0) / 5.0) + 32.0;

    // Round to the nearest integral value
    TMathLib::Round(f8Ret, tCIDLib::ERoundTypes::Closest);
    return tCIDLib::TInt4(f8Ret);
}



//
//  A special case message sending helper for object status queries. They are
//  done enough to have a special one to handle the format.
//
tCIDLib::TVoid
THAIOmniTCP2S::SendObjStatusQuery(  const   tCIDLib::TCard1 c1ObjType
                                    , const tCIDLib::TCard2 c2StartNum
                                    , const tCIDLib::TCard2 c2EndNum)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(6, 1);
    mbufWrite.PutCard1(kHAIOmniTCP2S::c1MsgType_ExtObjStatusReq, 2);
    mbufWrite.PutCard1(c1ObjType, 3);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2StartNum >> 8), 4);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2StartNum & 0xFF), 5);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2EndNum >> 8), 6);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2EndNum & 0xFF), 7);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 8);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 9);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 10);
}


//
//  These create messages that contain varying common numbers of data
//  bytes. They create the Omni message in the m_mbufWrite, put the
//  CQC on it, then put it in a msg and send it via SendPacket().
//
tCIDLib::TVoid
THAIOmniTCP2S::SendOmniCmd( const   tCIDLib::TCard1 c1Cmd
                            , const tCIDLib::TCard1 c1Parm1
                            , const tCIDLib::TCard2 c2Parm2)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(5, 1);
    mbufWrite.PutCard1(kHAIOmniTCP2S::c1MsgType_SendCmd, 2);
    mbufWrite.PutCard1(c1Cmd, 3);
    mbufWrite.PutCard1(c1Parm1, 4);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2Parm2 >> 8), 5);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2Parm2 & 0xFF), 6);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 7);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 8);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 9);
}

tCIDLib::TVoid THAIOmniTCP2S::SendOmniMsg(const tCIDLib::TCard1 c1Type)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(1, 1);
    mbufWrite.PutCard1(c1Type, 2);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 3);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 4);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 5);
}

tCIDLib::TVoid
THAIOmniTCP2S::SendOmniMsg( const   tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(2, 1);
    mbufWrite.PutCard1(c1Type, 2);
    mbufWrite.PutCard1(c1Data, 3);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 4);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 5);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 6);
}

tCIDLib::TVoid
THAIOmniTCP2S::SendOmniMsg( const   tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data1
                            , const tCIDLib::TCard1 c1Data2)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(3, 1);
    mbufWrite.PutCard1(c1Type, 2);
    mbufWrite.PutCard1(c1Data1, 3);
    mbufWrite.PutCard1(c1Data2, 4);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 5);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 6);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 7);
}

tCIDLib::TVoid
THAIOmniTCP2S::SendOmniMsg( const   tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data1
                            , const tCIDLib::TCard1 c1Data2
                            , const tCIDLib::TCard1 c1Data3)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(4, 1);
    mbufWrite.PutCard1(c1Type, 2);
    mbufWrite.PutCard1(c1Data1, 3);
    mbufWrite.PutCard1(c1Data2, 4);
    mbufWrite.PutCard1(c1Data3, 5);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 6);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 7);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 8);
}

tCIDLib::TVoid
THAIOmniTCP2S::SendOmniMsg( const   tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data1
                            , const tCIDLib::TCard1 c1Data2
                            , const tCIDLib::TCard1 c1Data3
                            , const tCIDLib::TCard1 c1Data4)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, 0);
    mbufWrite.PutCard1(5, 1);
    mbufWrite.PutCard1(c1Type, 2);
    mbufWrite.PutCard1(c1Data1, 3);
    mbufWrite.PutCard1(c1Data2, 4);
    mbufWrite.PutCard1(c1Data3, 5);
    mbufWrite.PutCard1(c1Data4, 6);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 7);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 8);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, 9);
}


tCIDLib::TVoid
THAIOmniTCP2S::SendOmniMsg( const   tCIDLib::TCard1 c1MsgType
                            , const tCIDLib::TCard1 c1ObjType
                            , const tCIDLib::TCard2 c2ItemNum
                            , const tCIDLib::TCard1 ac1Data[]
                            , const tCIDLib::TCard4 c4DataCnt)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    tCIDLib::TCard4 c4Index = 0;
    mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, c4Index++);
    mbufWrite.PutCard1(tCIDLib::TCard1(4 + c4DataCnt), c4Index++);
    mbufWrite.PutCard1(c1MsgType, c4Index++);
    mbufWrite.PutCard1(c1ObjType, c4Index++);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2ItemNum >> 8), c4Index++);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2ItemNum & 0xFF), c4Index++);

    for (tCIDLib::TCard4 c4DInd = 0; c4DInd < c4DataCnt; c4DInd++)
        mbufWrite.PutCard1(ac1Data[c4DInd], c4Index++);

    const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), c4Index++);
    mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), c4Index++);
    SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, c4Index);
}


// Sends a packet with payload data
tCIDLib::TVoid
THAIOmniTCP2S::SendPacket(  const   TMemBuf&        mbufToSend
                            , const tCIDLib::TCard1 c1MsgType
                            , const tCIDLib::TCard4 c4SendBytes)
{
    // Just call the other one and pass the raw pointer
    SendPacket(mbufToSend.pc1Data(), c1MsgType, c4SendBytes);
}

tCIDLib::TVoid
THAIOmniTCP2S::SendPacket(  const   tCIDLib::TCard1* const  pc1ToSend
                            , const tCIDLib::TCard1         c1MsgType
                            , const tCIDLib::TCard4         c4SendBytes)
{
    if (c4SendBytes > kHAIOmniTCP2S::c4MaxMsgSize)
    {
        facHAIOmniTCP2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcProto_MsgTooBig
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4SendBytes)
        );
    }

    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, c4SendBytes + 32);
    TMemBuf& mbufWrite = *janWrite;

    //
    //  We need to build the msg to send. Start with the sequence. We
    //  bump up to the next sequence number, wrapping if needed. This leaves
    //  m_c2LastSeq on the seq value we expect to come back.
    //
    m_c2LastSeq++;
    if (!m_c2LastSeq)
        m_c2LastSeq++;
    #if defined(CIDLIB_LITTLEENDIAN)
    mbufWrite.PutCard2(TRawBits::c2SwapBytes(m_c2LastSeq), 0);
    #else
    mbufWrite.PutCard2(m_c2LastSeq, 0);
    #endif

    // Put in the msg type and reserved byte
    mbufWrite.PutCard1(c1MsgType, 2);
    mbufWrite.PutCard1(0, 3);

    //
    //  Put the encrypted payload into the msg buffer which gets us the
    //  the actual size to send. It might be padded up to an even multiple
    //  of the AES block size.
    //
    const tCIDLib::TCard4 c4PackSz
    (
        c4EncryptPayload(pc1ToSend, c4SendBytes, mbufWrite)
    );

    // Now we can send the msg
    m_sockOmni.Send(mbufWrite, c4PackSz);
}


// Sends a non-Omni level packet without payload
tCIDLib::TVoid THAIOmniTCP2S::SendPacket(const tCIDLib::TCard1 c1MsgType)
{
    // Get a buffer to use
    THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
    TMemBuf& mbufWrite = *janWrite;

    //
    //  We need to build the msg to send. Start with the sequence. We
    //  bump up to the next sequence number, wrapping if needed. This leaves
    //  m_c2LastSeq on the seq value we expect to come back.
    //
    m_c2LastSeq++;
    if (!m_c2LastSeq)
        m_c2LastSeq++;
    #if defined(CIDLIB_LITTLEENDIAN)
    mbufWrite.PutCard2(TRawBits::c2SwapBytes(m_c2LastSeq), 0);
    #else
    mbufWrite.PutCard2(m_c2LastSeq, 0);
    #endif

    // Put in the msg type and reserved byte
    mbufWrite.PutCard1(c1MsgType, 2);
    mbufWrite.PutCard1(0, 3);

    m_sockOmni.Send(mbufWrite, 4);
}


//
//  Waits up to the indicated millis for the indicated Omni message to come
//  in. In the meantime, we process any async messages that we care about.
//
//  Because the object status message is overloaded, we have to also be able
//  to watch for a particular data type, so that we don't return with the
//  wrong status reply. So they can indicate an expected data type as well. If
//  the message type indicates a status reply, we'll check that also.
//
tCIDLib::TVoid
THAIOmniTCP2S::WaitReply(const  tCIDLib::TCard4     c4WaitFor
                        , const tCIDLib::TCard1     c1ExpMsg
                        , const tCIDLib::TCard1     c1ExpDataType
                        ,       TMemBuf&            mbufToFill)
{
    tCIDLib::TCard1 c1OmniType = 0;
    tCIDLib::TCard1 c1Type = 0;
    tCIDLib::TCard4 c4MsgBytes = 0;

    //
    //  We will keep waiting until we hit the end time. If they pass zero for
    //  the wait, then use the default.
    //
    const tCIDLib::TCard4 c4ActualWait
    (
        c4WaitFor ? c4WaitFor : kHAIOmniTCP2S::c4WaitReply
    );
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd(enctCur + (c4ActualWait * kCIDLib::enctOneMilliSec));

    while (enctCur < enctEnd)
    {
        const tCIDLib::TBoolean bGotOne = bGetPacket
        (
            mbufToFill, c1Type, c4MsgBytes, 250, m_c2LastSeq, kCIDLib::False
        );

        if (bGotOne)
        {
            // Break out if we get the expected message
            c1OmniType = mbufToFill[kHAIOmniTCP2S::c4MsgTypeOfs];
            if ((c1Type == kHAIOmniTCP2S::c1PackType_OmniMsg) && (c1OmniType == c1ExpMsg))
            {
                //
                //  If it's a status reply, and we have a data type, check
                //  that too. So, if it's either not a status reply, or it
                //  is one and the data type matches, then it's our guy.
                //
                if ((c1OmniType != kHAIOmniTCP2S::c1MsgType_ExtObjStatusRep)
                ||  (mbufToFill[3] ==  c1ExpDataType))
                {
                    break;
                }
            }

            // It's not what we are waiting for, so process it as an async
            ProcessAsync(c1OmniType, mbufToFill);

            // A little makeup for the time spent processing this async
            enctEnd += (kCIDLib::enctOneMilliSec * 5);
        }

        // Bump the time
        enctCur = TTime::enctNow();
    }

    // If we timed out, then we never got the expected message
    if (enctCur >= enctEnd)
    {
        facHAIOmniTCP2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcProto_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
}


