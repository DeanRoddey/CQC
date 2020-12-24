//
// FILE NAME: HAIOmniS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2006
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
#include    "HAIOmniS.hpp"



// ---------------------------------------------------------------------------
//  THAIOmniSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method will establish an encrypted session with the Omni. This is
//  all special case stuff very different from the normal conversation.
//
tCIDLib::TBoolean THAIOmniSDriver::bCreateSession()
{
    tCIDLib::TCard1 c1PacketType;
    tCIDLib::TCard4 c4MsgBytes;

    try
    {
        // Send a new session request
        SendPacket(kHAIOmniS::c1PackType_ReqNewSess);

        // Wait for an ack or refusal
        bGetPacket
        (
            m_mbufRead
            , c1PacketType
            , c4MsgBytes
            , 4000
            , m_c2LastSeq
            , kCIDLib::True
        );

        if (c1PacketType != kHAIOmniS::c1PackType_AckNewSess)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniSErrs::errcProto_NoStartSesAck
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
        #if CID_DEBUG_ON
        if (c4MsgBytes != 7)
        {
            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcProto_BadSessAckSize
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4MsgBytes)
            );
        }
        #endif

        // Create the session key
        tCIDLib::TCard1 ac1Key[16];
        m_mhashKey.c4ToRawArray(ac1Key, 16);
        const tCIDLib::TCard1* pc1Src = m_mbufRead.pc1Data() + 2;
        tCIDLib::TCard1* pc1Tar = ac1Key + 11;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < 5; c4Index++)
            *pc1Tar++ ^= *pc1Src++;

        // Set up the key now and set it on the encrypter
        TCryptoKey ckeySession(ac1Key, 16);
        m_crypComm.SetNewKey(ckeySession);

        // Ask to start the encrypted session, and get the ack/nak
        SendPacket(m_mbufRead.pc1Data() + 2, kHAIOmniS::c1PackType_ReqSecSess, 5);
        bGetPacket
        (
            m_mbufRead
            , c1PacketType
            , c4MsgBytes
            , 4000
            , m_c2LastSeq
            , kCIDLib::True
        );

        if (c1PacketType != kHAIOmniS::c1PackType_AckSecSess)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniSErrs::errcProto_NoSecSesAck
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c1PacketType)
                );
            }
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
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
//  Uses the login id we got during driver install to try to log into the
//  Omni. If we get past this, we are ready to start talking to it.
//
tCIDLib::TBoolean THAIOmniSDriver::bDoLogin()
{
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(5, 1);
    m_mbufWrite.PutCard1(kHAIOmniS::c1MsgType_Login, 2);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(m_strLogin[0]), 3);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(m_strLogin[1]), 4);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(m_strLogin[2]), 5);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(m_strLogin[3]), 6);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 7);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 8);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 9);

    // We should get an ack back
    return bWaitAck(4000, kCIDLib::False);
}



//
//  This method watches for a new packet to arrive, makes sure it's not a
//  dup or that we haven't missed any packets, and if it's an Omni level msg
//  it verifies the CRC.
//
//  It'll wait for up to c4WaitFor millis, then either throw or return false
//  according to the bThrowIfNot flag. It retursn the type of packet and the
//  payload bytes of the packet, which normally will either be nothing or the
//  Omni message recieved.
//
tCIDLib::TBoolean
THAIOmniSDriver::bGetPacket(        TMemBuf&            mbufToFill
                            ,       tCIDLib::TCard1&    c1PacketType
                            ,       tCIDLib::TCard4&    c4MsgBytes
                            , const tCIDLib::TCard4     c4WaitFor
                            , const tCIDLib::TCard2     c2ExpSeq
                            , const tCIDLib::TBoolean   bThrowIfNot)
{
    tCIDLib::TCard2 c2Seq;
    tCIDLib::TCard4 c4PacketSz;

    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec);
    while (enctCur < enctEnd)
    {
        c4PacketSz = m_sockOmni.c4ReceiveMBufFrom
        (
            m_ipepFrom
            , m_mbufPacket
            , enctEnd - enctCur
            , kHAIOmniS::c4MaxPacketSize
        );

        // If nothing or less than min packet, then we timed out
        if (c4PacketSz < kHAIOmniS::c4MinPacketSize)
        {
            if (!bThrowIfNot)
                return kCIDLib::False;

            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcProto_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }

        //
        //  We got one, so pull out the sequence number and see if we need to
        //  reject it.
        //
        #if defined(CIDLIB_LITTLEENDIAN)
        c2Seq = TRawBits::c2SwapBytes(m_mbufPacket.c2At(0));
        #else
        c2Seq = m_mbufPacket.c2At(0));
        #endif

        //
        //  If it's not the expected sequence number, ignore it, else we
        //  have our number.
        //
        if (c2ExpSeq == c2Seq)
            break;

        enctCur = TTime::enctNow();
    }

    // We got something, so make sure the timeout counter is reset
    m_c4TimeoutCnt = 0;

    //
    //  If we got here, then we got a valid new packet, so let's process it.
    //  Get the packet type out and the number of data bytes in the packet.
    //
    c1PacketType = m_mbufPacket.c1At(2);
    c4MsgBytes = c4PacketSz - 4;

    // Make sure it's not larger than the maximum message size
    if (c4MsgBytes > kHAIOmniS::c4MaxMsgSize)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcProto_MsgTooBig
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4MsgBytes)
        );
    }

    // If it's an Omni message, then validate the CRC
    if (c1PacketType == kHAIOmniS::c1PackType_OmniMsg)
    {
        //
        //  We have to decrypt the data at this point. We decrypt into the
        //  caller's buffer. This will not change the size of the message,
        //  since there is a one to one plain to cypher text size.
        //
        const tCIDLib::TCard1*  pc1Src = m_mbufPacket.pc1Data();
        const tCIDLib::TCard1*  pc1End = pc1Src + c4MsgBytes;
        tCIDLib::TCard1*        pc1Tar = mbufToFill.pc1Data();

        //
        //  Get the two sequence bytes out since we'll use them in each
        //  round below.
        //
        const tCIDLib::TCard1 c1HighSeq = m_mbufPacket[0];
        const tCIDLib::TCard1 c1LowSeq = m_mbufPacket[1];
        pc1Src += 4;
        while(pc1Src < pc1End)
        {
            // Decrypt a block
            m_crypComm.c4Decrypt(pc1Src, pc1Tar, 16, 16);

            // And swizzle it with the sequence number bytes
            *pc1Tar ^= c1HighSeq;
            *(pc1Tar + 1) ^= c1LowSeq;

            pc1Src += 16;
            pc1Tar += 16;
        }

        //
        //  Update the message bytes since it can be padded and in this
        //  case we have an actual length indicator in the message data.
        //
        c4MsgBytes = mbufToFill[1] + 4;
        const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufToFill);
        if (c2CRC != mbufToFill.c2At(c4MsgBytes - 2))
        {
            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcProto_BadCRC
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // If it's a nack, then throw
        if (mbufToFill[kHAIOmniS::c4MsgTypeOfs] == kHAIOmniS::c1MsgType_Nak)
        {
            facHAIOmniS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcProto_Nak
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
            );
        }
    }
     else
    {
        // Not any Omni message, so just copy out any message bytes
        if (c4MsgBytes)
            m_mbufPacket.CopyOut(mbufToFill, c4MsgBytes, 4);
    }
    return kCIDLib::True;
}


//
//  Validates a passed code and returns the number of the code, which is
//  the form required for passing to the Omni in commands.
//
tCIDLib::TBoolean
THAIOmniSDriver::bValidateCode( const   TString&            strCode
                                , const tCIDLib::TCard4     c4Area
                                ,       tCIDLib::TCard1&    c1CodeNum)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Validating user code"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  This is a very specialized message, so we just build it ourselves
    //  manually. We do cheap Unicode to ASCII transcoding via truncation
    //  since we know the codes can only be ASCII digits.
    //
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(6, 1);
    m_mbufWrite.PutCard1(kHAIOmniS::c1MsgType_CodeValid, 2);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Area), 3);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(strCode[0] - 0x30), 4);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(strCode[1] - 0x30), 5);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(strCode[2] - 0x30), 6);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(strCode[3] - 0x30), 7);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 8);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 9);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 10);

    // And we should get a code validation response
    WaitReply(4000, kHAIOmniS::c1MsgType_CodeValidReply);

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"User code validated"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    c1CodeNum = m_mbufRead[kHAIOmniS::c4FirstDBOfs];
    return (c1CodeNum != 0);
}


//
//  Waits up to the indicated millis for an ack message. As it's waiting,
//  it will also pull in any waiting system events, which can back up and
//  overflow the Omni's buffer if we don't pull them out very fast.
//
tCIDLib::TBoolean
THAIOmniSDriver::bWaitAck(const tCIDLib::TCard4     c4WaitFor
                        , const tCIDLib::TBoolean   bThrowIfTimeout)
{
    tCIDLib::TCard1 c1Type;
    tCIDLib::TCard4 c4Bytes;

    // If no reply, and bThrowIfTimeout was false, return false
    if (!bGetPacket(m_mbufRead, c1Type, c4Bytes, c4WaitFor, m_c2LastSeq, bThrowIfTimeout))
        return kCIDLib::False;

    // If not an ack, return false
    if ((c1Type != kHAIOmniS::c1PackType_OmniMsg)
    ||  (m_mbufRead[2] != kHAIOmniS::c1MsgType_Ack))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Gets a packet, makes sure it's an Omni msg and of the type expected
tCIDLib::TCard1
THAIOmniSDriver::c1GetOmniMsg(          TMemBuf&        mbufToFill
                                , const tCIDLib::TCard4 c4WaitFor
                                , const tCIDLib::TCard1 c1ExpRep)
{
    tCIDLib::TCard1 c1Type;

    bGetPacket(mbufToFill, c1Type, m_c4ReadBytes, c4WaitFor, m_c2LastSeq, kCIDLib::True);
    if ((c1Type != kHAIOmniS::c1PackType_OmniMsg)
    ||  (c1ExpRep && (mbufToFill[kHAIOmniS::c4MsgTypeOfs] != c1ExpRep)))
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcProto_UnexpectedReply
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c1ExpRep)
            , TCardinal(mbufToFill[kHAIOmniS::c4MsgTypeOfs])
        );
    }
    return mbufToFill[kHAIOmniS::c4MsgTypeOfs];
}


//
//  Calculate the Omni CRC for the passed message, which is assumed to be
//  well formed, since we get the length out of it.
//
tCIDLib::TCard2 THAIOmniSDriver::c2CalcCRC(const TMemBuf& mbufSrc)
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
THAIOmniSDriver::c4ToOmniTemp(  const   tCIDLib::TInt4      i4Convert
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
//  packet buffer, and returns the actual full packet size (we might have
//  to pad up to a full AES block.)
//
tCIDLib::TCard4
THAIOmniSDriver::c4EncryptPayload(  const   tCIDLib::TCard1* const  pc1Payload
                                    , const tCIDLib::TCard4         c4PLBytes
                                    ,       TMemBuf&                mbufPacket)
{
    // We start after the packet header bytes and add our bytes to that
    tCIDLib::TCard4 c4RetSize = 4;
    if (c4PLBytes)
    {
        const tCIDLib::TCard1* pc1Src = pc1Payload;
        tCIDLib::TCard1* pc1Tar = m_mbufPacket.pc1Data() + 4;

        const tCIDLib::TCard1 c1HighSeq = m_mbufPacket[0];
        const tCIDLib::TCard1 c1LowSeq = m_mbufPacket[1];

        //
        //  Figure out how many full blocks we can do. We'll have to handle
        //  the last one specially by padding it.
        //
        tCIDLib::TCard1 ac1Block[16];
        const tCIDLib::TCard4 c4Blocks = c4PLBytes / 16;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Blocks; c4Index++)
        {
            TRawMem::CopyMemBuf(ac1Block, pc1Src, 16);
            ac1Block[0] ^= c1HighSeq;
            ac1Block[1] ^= c1LowSeq;

            // Decrypt a block
            m_crypComm.c4Encrypt(ac1Block, pc1Tar, 16, 16);

            pc1Src += 16;
            pc1Tar += 16;
        }
        c4RetSize += c4Blocks * 16;

        // If there's a partial block do that
        const tCIDLib::TCard4 c4Partial = (c4PLBytes % 16);
        if (c4Partial)
        {
            TRawMem::SetMemBuf(ac1Block, tCIDLib::TCard1(0), 16);
            TRawMem::CopyMemBuf(ac1Block, pc1Src, c4Partial);
            ac1Block[0] ^= c1HighSeq;
            ac1Block[1] ^= c1LowSeq;
            m_crypComm.c4Encrypt(ac1Block, pc1Tar, 16, 16);
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
THAIOmniSDriver::i4FromOmniTemp(const   tCIDLib::TCard4     c4OmniTemp
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
//  These create messages that contain varying common numbers of data
//  bytes. They create the Omni message in the m_mbufSendBuf, put the
//  CQC on it, then put it in a packet and send it via SendPacket().
//
tCIDLib::TVoid
THAIOmniSDriver::SendOmniCmd(const  tCIDLib::TCard1 c1Cmd
                            , const tCIDLib::TCard1 c1Parm1
                            , const tCIDLib::TCard2 c2Parm2)
{
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(5, 1);
    m_mbufWrite.PutCard1(kHAIOmniS::c1MsgType_SendCmd, 2);
    m_mbufWrite.PutCard1(c1Cmd, 3);
    m_mbufWrite.PutCard1(c1Parm1, 4);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2Parm2 >> 8), 5);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2Parm2 & 0xFF), 6);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 7);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 8);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 9);
}


tCIDLib::TVoid THAIOmniSDriver::SendOmniMsg(const tCIDLib::TCard1 c1Type)
{
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(1, 1);
    m_mbufWrite.PutCard1(c1Type, 2);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 3);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 4);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 5);
}

tCIDLib::TVoid
THAIOmniSDriver::SendOmniMsg(const  tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data)
{
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(2, 1);
    m_mbufWrite.PutCard1(c1Type, 2);
    m_mbufWrite.PutCard1(c1Data, 3);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 4);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 5);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 6);
}

tCIDLib::TVoid
THAIOmniSDriver::SendOmniMsg(const  tCIDLib::TCard1 c1Type
                            , const tCIDLib::TCard1 c1Data1
                            , const tCIDLib::TCard1 c1Data2)
{
    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(3, 1);
    m_mbufWrite.PutCard1(c1Type, 2);
    m_mbufWrite.PutCard1(c1Data1, 3);
    m_mbufWrite.PutCard1(c1Data2, 4);
    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), 5);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), 6);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, 7);
}


// Sends a packet with payload data
tCIDLib::TVoid
THAIOmniSDriver::SendPacket(const   TMemBuf&        mbufToSend
                            , const tCIDLib::TCard1 c1PacketType
                            , const tCIDLib::TCard4 c4SendBytes)
{
    SendPacket(mbufToSend.pc1Data(), c1PacketType, c4SendBytes);
}

tCIDLib::TVoid
THAIOmniSDriver::SendPacket(const   tCIDLib::TCard1* const  pc1ToSend
                            , const tCIDLib::TCard1         c1PacketType
                            , const tCIDLib::TCard4         c4SendBytes)
{
    if (c4SendBytes > kHAIOmniS::c4MaxPacketSize)
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcProto_PacketTooBig
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c4SendBytes)
        );
    }

    //
    //  We need to build the packet to send. Start with the sequence. We
    //  bump up to the next sequence number, wrapping if needed. This leaves
    //  m_c2LastSeq on the seq value we expect to come back.
    //
    m_c2LastSeq++;
    if (!m_c2LastSeq)
        m_c2LastSeq++;
    #if defined(CIDLIB_LITTLEENDIAN)
    m_mbufPacket.PutCard2(TRawBits::c2SwapBytes(m_c2LastSeq), 0);
    #else
    m_mbufPacket.PutCard2(m_c2LastSeq, 0);
    #endif

    // Put in the packet type and reserved byte
    m_mbufPacket.PutCard1(c1PacketType, 2);
    m_mbufPacket.PutCard1(0, 3);

    //
    //  Put the encrypted payload into the packet buffer which gets us the
    //  the actual size to send. It might be padded up to an even multiple
    //  of the AES block size.
    //
    const tCIDLib::TCard4 c4PackSz
    (
        c4EncryptPayload(pc1ToSend, c4SendBytes, m_mbufPacket)
    );

    // Now we can send the packet
    m_sockOmni.c4SendTo(m_ipepOmni, m_mbufPacket, c4PackSz);
}


// Sends a packet without payload
tCIDLib::TVoid
THAIOmniSDriver::SendPacket(const tCIDLib::TCard1 c1PacketType)
{
    //
    //  We need to build the packet to send. Start with the sequence. We
    //  bump up to the next sequence number, wrapping if needed. This leaves
    //  m_c2LastSeq on the seq value we expect to come back.
    //
    m_c2LastSeq++;
    if (!m_c2LastSeq)
        m_c2LastSeq++;
    #if defined(CIDLIB_LITTLEENDIAN)
    m_mbufPacket.PutCard2(TRawBits::c2SwapBytes(m_c2LastSeq), 0);
    #else
    m_mbufPacket.PutCard2(m_c2LastSeq, 0);
    #endif

    // Put in the packet type and reserved byte
    m_mbufPacket.PutCard1(c1PacketType, 2);
    m_mbufPacket.PutCard1(0, 3);

    m_sockOmni.c4SendTo(m_ipepOmni, m_mbufPacket, 4);
}



//
//  Units are a special case. They can have numbers over 256, so we have
//  to use two bytes per number if either are larger than 256.
//
tCIDLib::TVoid
THAIOmniSDriver::SendUnitReq(const  tCIDLib::TCard4 c4Data1
                            , const tCIDLib::TCard4 c4Data2)
{
    const tCIDLib::TBoolean bBigNum((c4Data1 > 255) || (c4Data2 > 255));

    m_mbufWrite.PutCard1(kHAIOmniS::c1STX, 0);
    m_mbufWrite.PutCard1(bBigNum ? 5 : 3, 1);
    m_mbufWrite.PutCard1(kHAIOmniS::c1MsgType_ReqUnitStatus, 2);

    tCIDLib::TCard4 c4Count = 3;
    if (bBigNum)
    {
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data1 >> 8), c4Count++);
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data1 & 0xFF), c4Count++);
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data2 >> 8), c4Count++);
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data2 & 0xFF), c4Count++);
    }
     else
    {
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data1), c4Count++);
        m_mbufWrite.PutCard1(tCIDLib::TCard1(c4Data2), c4Count++);
    }

    const tCIDLib::TCard2 c2CRC = c2CalcCRC(m_mbufWrite);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), c4Count++);
    m_mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), c4Count++);
    SendPacket(m_mbufWrite, kHAIOmniS::c1PackType_OmniMsg, c4Count);
}



//
//  Waits up to the indicated millis for the indicated Omni message to come
//  in. As it is waiting, it will also poll for system events, which can
//  otherwise back up and overflow the Omni's buffer and we lose them. So
//  we check for anything to have arrived. If not, we do a query for system
//  events.
//
tCIDLib::TVoid
THAIOmniSDriver::WaitReply( const   tCIDLib::TCard4 c4WaitFor
                            , const tCIDLib::TCard1 c1Expected)
{
    tCIDLib::TCard1 c1OmniType;
    tCIDLib::TCard1 c1Type;

    bGetPacket
    (
        m_mbufRead, c1Type, m_c4ReadBytes, c4WaitFor, m_c2LastSeq, kCIDLib::True
    );

    //
    //  If not the expected reply, then throw. Note that a Nak will be thrown by
    //  bGetPacket() itself, so we don't have to check for that.
    //
    c1OmniType = m_mbufRead[kHAIOmniS::c4MsgTypeOfs];
    if ((c1Type != kHAIOmniS::c1PackType_OmniMsg) || (c1OmniType != c1Expected))
    {
        facHAIOmniS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniSErrs::errcProto_UnexpectedReply
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1Expected)
            , TCardinal(c1OmniType)
        );
    }
}


