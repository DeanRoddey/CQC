//
// FILE NAME: BarcoCRTS_Msg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
#include    "BarcoCRTS.hpp"


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace BarcoCRTS_Msg
{
    // -----------------------------------------------------------------------
    //  The mapping array between our command enum and the actual values used
    //  by the projector.
    //
    // NOTE: Update this if the commands enum is changed!
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1 ac1Cmds[tBarcoCRTS::ECmds_Count] =
    {
        0x0E            // IR Standby
        , 0x10          // IR keypad 0
        , 0x11          // IR keypad 1
        , 0x12          // IR keypad 2
        , 0x13          // IR keypad 3
        , 0x14          // IR keypad 4
        , 0x15          // IR keypad 5
        , 0x16          // IR keypad 6
        , 0x17          // IR keypad 7
        , 0x18          // IR keypad 8
        , 0x19          // IR keypad 9
        , 0x51          // ECmd_ForceToggle
        , 0x4A          // ECmd_QueryCurSource
        , 0x4B          // ECmd_QueryStatus
        , 0x4C          // ECmd_QueryVersion
    };


    // -----------------------------------------------------------------------
    //  A translation table for our command enum
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* apszCmdNames[tBarcoCRTS::ECmds_Count] =
    {
        L"IR Num0"
        , L"IR Num1"
        , L"IR Num2"
        , L"IR Num3"
        , L"IR Num4"
        , L"IR Num5"
        , L"IR Num6"
        , L"IR Num7"
        , L"IR Num8"
        , L"IR Num9"
        , L"ForceToggle"
        , L"QueryCurSource"
        , L"QueryStatus"
        , L"QueryVersion"
    };
}




// ---------------------------------------------------------------------------
//   CLASS: TBarcoMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TBarcoMsg::TBarcoMsg() :

    m_bChanges(kCIDLib::True)
    , m_c4Len(0)
    , m_c4XlatedLen(0)
{
}

TBarcoMsg::TBarcoMsg(const  tCIDLib::TCard1     c1Address
                    , const tBarcoCRTS::ECmds   eCmd) :

    m_bChanges(kCIDLib::True)
    , m_c4Len(0)
    , m_c4XlatedLen(0)
{
    Set(c1Address, eCmd);
}

TBarcoMsg::TBarcoMsg(const  tCIDLib::TCard1     c1Address
                    , const tBarcoCRTS::ECmds   eCmd
                    , const tCIDLib::TCard1     c1Data1
                    , const tCIDLib::TCard1     c1Data2
                    , const tCIDLib::TCard1     c1Data3
                    , const tCIDLib::TCard1     c1Data4) :

    m_bChanges(kCIDLib::True)
    , m_c4Len(0)
    , m_c4XlatedLen(0)
{
    Set(c1Address, eCmd, c1Data1, c1Data2, c1Data3, c1Data4);
}

TBarcoMsg::TBarcoMsg(const TBarcoMsg& msgToCopy) :

    m_bChanges(msgToCopy.m_bChanges)
    , m_c4Len(msgToCopy.m_c4Len)
    , m_c4XlatedLen(msgToCopy.m_c4XlatedLen)
{
    TRawMem::CopyMemBuf(m_ac1Msg, msgToCopy.m_ac1Msg, tCIDLib::c4ArrayElems(m_ac1Msg));
    TRawMem::CopyMemBuf(m_ac1Xlated, msgToCopy.m_ac1Xlated, tCIDLib::c4ArrayElems(m_ac1Msg));
}

TBarcoMsg::~TBarcoMsg()
{
}


// ---------------------------------------------------------------------------
//  TBarcoMsg: Public operators
// ---------------------------------------------------------------------------
TBarcoMsg& TBarcoMsg::operator=(const TBarcoMsg& msgToAssign)
{
    if (this == &msgToAssign)
        return *this;

    m_c4Len = msgToAssign.m_c4Len;
    m_c4XlatedLen = msgToAssign.m_c4XlatedLen;
    TRawMem::CopyMemBuf(m_ac1Msg, msgToAssign.m_ac1Msg, tCIDLib::c4ArrayElems(m_ac1Msg));
    TRawMem::CopyMemBuf(m_ac1Xlated, msgToAssign.m_ac1Xlated, tCIDLib::c4ArrayElems(m_ac1Msg));

    return *this;
}


//
//  These operators return data bytes only. So the index passed in is an
//  index is into the set of data bytes. We we add two to it in order to get
//  it up to where the data bytes start.
//
tCIDLib::TCard1 TBarcoMsg::operator[](const tCIDLib::TCard4 c4Index) const
{
    if (c4Index + 2 >= m_c4Len)
    {
        // We are throwing a CIDLib error here, not one of ours!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index + 2)
            , TString(L"TBarcoMsg")
            , TCardinal(m_c4Len)
        );
    }
    return m_ac1Msg[c4Index + 2];
}

tCIDLib::TCard1& TBarcoMsg::operator[](const tCIDLib::TCard4 c4Index)
{
    if (c4Index + 2 >= m_c4Len)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_IndexError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index + 2)
            , TString(L"TBarcoMsg")
            , TCardinal(m_c4Len)
        );
    }

    // We have to assume they are going to change it
    m_bChanges = kCIDLib::True;

    return m_ac1Msg[c4Index + 2];
}



// ---------------------------------------------------------------------------
//  TBarcoMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TBarcoMsg::bIsReplyTo(const TBarcoMsg& msgToCheck) const
{
    // If the address and command is the same, then we are good
    return  (m_ac1Msg[0] == msgToCheck.m_ac1Msg[0])
            && (m_ac1Msg[1] == msgToCheck.m_ac1Msg[1]);
}


tCIDLib::TBoolean TBarcoMsg::bSendOn(TCommPortBase* const pcommTarget)
{
    // Make sure that the translated version is up to date
    MakeXlated();

    // It has a habit of sending junk, so flush first
    pcommTarget->PurgeReadData();

    // And lets send the translated bytes
    const tCIDLib::TCard4 c4Sent = pcommTarget->c4WriteRawBufMS
    (
        m_ac1Xlated, m_c4XlatedLen, 250
    );

    if (c4Sent != m_c4XlatedLen)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NotAllSent
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
        );
    }

    //
    //  We should now either get an ack or nack. In this version, we just
    //  return a false for no reply.
    //
    tCIDLib::TCard1 c1Val;
    if (pcommTarget->c4ReadRawBufMS(&c1Val, 1, 4000) != 1)
        return kCIDLib::False;

    //
    //  If its not an ack or nack, then flush the input buffer and, throw
    //  an exception.
    //
    if ((c1Val != kBarcoCRTS::c1Ack) && (c1Val != kBarcoCRTS::c1Nak))
    {
        pcommTarget->PurgeReadData();
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_ExpectedAck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszXlatCmd(eCmd()))
        );
    }

    // If its a nack, then throw for that
    if (c1Val == kBarcoCRTS::c1Nak)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_GotNack
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszXlatCmd(eCmd()))
        );
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TBarcoMsg::Clear()
{
    m_c4Len = 0;
    m_bChanges = kCIDLib::True;
}


tBarcoCRTS::ECmds TBarcoMsg::eCmd() const
{
    // Make sure that there is something in the raw message.
    if (!m_c4Len)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NoCmdPresent
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }

    //
    //  Search the mapping array. The index we find it at can be cast to the
    //  command enum.
    //
    const tBarcoCRTS::ECmds eRet = eXlatCmd(m_ac1Msg[1]);

    // If we didn't find it, that's an error
    if (eRet == tBarcoCRTS::ECmds_Count)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_BadCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(m_ac1Msg[1])
        );
    }
    return eRet;
}


//
//  This one is for receiving short type packets, and will obviously fail if
//  it is used to read a long one, because the checksum will fail.
//
tBarcoCRTS::EMsgResults
TBarcoMsg::eReadShortFrom(          TCommPortBase* const    pcommTarget
                            , const tCIDLib::TCard4         c4WaitMillis)
{
    //
    //  Read into the message buffer. If the read works, this will put the
    //  data in the same spots as they would go as they are stored when outside
    //  code builds up messages to send.
    //
    //  We'll use the xlated array as a temp, and only store it if its good.
    //
    const tCIDLib::TCard4 c4Got = pcommTarget->c4ReadRawBufMS
    (
        m_ac1Xlated, kBarcoCRTS::c4ShortRecLen, c4WaitMillis
    );

    // Indicate we got squat
    if (!c4Got)
        return tBarcoCRTS::EMsgResults::NoReply;

    // If some but not all, that's an error
    if (c4Got != kBarcoCRTS::c4ShortRecLen)
        return tBarcoCRTS::EMsgResults::PartialReply;

    //
    //  Calc the checksum of the message and compare to what we got. Be sure
    //  not to include the checksum itself!
    //
    tCIDLib::TCard4 c4CheckSum = 0;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < kBarcoCRTS::c4ShortRecLen - 1; c4Index++)
        c4CheckSum += m_ac1Xlated[c4Index];
    c4CheckSum %= 256;

    // If not, then return a failure
    if (c4CheckSum != m_ac1Xlated[kBarcoCRTS::c4ShortRecLen - 1])
        return tBarcoCRTS::EMsgResults::BadCheckSum;

    // Check that its a valid command
    if (eXlatCmd(m_ac1Xlated[1]) == tBarcoCRTS::ECmds_Count)
        return tBarcoCRTS::EMsgResults::BadCmd;

    // Seems like it worked, so store it
    TRawMem::CopyMemBuf(m_ac1Msg, m_ac1Xlated, tCIDLib::c4ArrayElems(m_ac1Msg));
    m_c4Len = kBarcoCRTS::c4ShortRecLen - 1;

    // Indicate changes and return success
    m_bChanges = kCIDLib::True;
    return tBarcoCRTS::EMsgResults::Ok;
}


tCIDLib::TVoid TBarcoMsg::SendOn(TCommPortBase* const pcommTarget)
{
    // Make sure that the translated version is up to date
    MakeXlated();

    // And lets send the translated bytes
    const tCIDLib::TCard4 c4Sent = pcommTarget->c4WriteRawBufMS
    (
        m_ac1Xlated, m_c4XlatedLen, 250
    );

    if (c4Sent != m_c4XlatedLen)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NotAllSent
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
        );
    }

    // We should now either get an ack or nack
    tCIDLib::TCard1 c1Val;
    if (pcommTarget->c4ReadRawBufMS(&c1Val, 1, 4000) != 1)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NoAck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NoReply
            , TString(pszXlatCmd(eCmd()))
        );
    }

    //
    //  If its not an ack or nack, then flush the input buffer and, throw
    //  an exception.
    //
    if ((c1Val != kBarcoCRTS::c1Ack) && (c1Val != kBarcoCRTS::c1Nak))
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_ExpectedAck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszXlatCmd(eCmd()))
        );
    }

    // If its a nack, then throw for that
    if (c1Val == kBarcoCRTS::c1Nak)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_GotNack
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TString(pszXlatCmd(eCmd()))
        );
    }
}


tCIDLib::TVoid TBarcoMsg::Set(  const   tCIDLib::TCard1     c1Address
                                , const tBarcoCRTS::ECmds   eCmd)
{
    m_c4Len = 0;
    m_ac1Msg[m_c4Len++] = c1Address;
    m_ac1Msg[m_c4Len++] = c1XlatCmd(eCmd);
    m_ac1Msg[m_c4Len++] = 0;
    m_ac1Msg[m_c4Len++] = 0;
    m_ac1Msg[m_c4Len++] = 0;
    m_ac1Msg[m_c4Len++] = 0;

    m_bChanges = kCIDLib::True;
}

tCIDLib::TVoid TBarcoMsg::Set(  const   tCIDLib::TCard1     c1Address
                                , const tBarcoCRTS::ECmds   eCmd
                                , const tCIDLib::TCard1     c1Data1
                                , const tCIDLib::TCard1     c1Data2
                                , const tCIDLib::TCard1     c1Data3
                                , const tCIDLib::TCard1     c1Data4)
{
    m_c4Len = 0;
    m_ac1Msg[m_c4Len++] = c1Address;
    m_ac1Msg[m_c4Len++] = c1XlatCmd(eCmd);
    m_ac1Msg[m_c4Len++] = c1Data1;
    m_ac1Msg[m_c4Len++] = c1Data2;
    m_ac1Msg[m_c4Len++] = c1Data3;
    m_ac1Msg[m_c4Len++] = c1Data4;

    m_bChanges = kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TBarcoMsg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard1 TBarcoMsg::c1XlatCmd(const tBarcoCRTS::ECmds eToXlat) const
{
    if (!bIsValidEnum(eToXlat))
    {
        // We are throwing a CIDLib error here, not one of ours!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_BadEnumValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TInteger(eToXlat)
            , TString(L"tBarcoCRTS::ECmds")
        );
    }
    return BarcoCRTS_Msg::ac1Cmds[eToXlat];
}


tBarcoCRTS::ECmds TBarcoMsg::eXlatCmd(const tCIDLib::TCard1 c1ToFind) const
{
    tBarcoCRTS::ECmds eCur = tBarcoCRTS::ECmds_Min;
    for (; eCur < tBarcoCRTS::ECmds_Count; eCur++)
    {
        if (BarcoCRTS_Msg::ac1Cmds[eCur] == c1ToFind)
            break;
    }
    return eCur;
}


tCIDLib::TVoid TBarcoMsg::MakeXlated()
{
    // Make sure that there is something in the raw message.
    if (!m_c4Len)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NoSrcToXlat
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // If there aren't any changes, just return
    if (!m_bChanges)
        return;

    //
    //  Zero out a flag array. For every value that we see in the current
    //  message, we'll set the byte at that offset, so we'll know all of the
    //  byte values used in the data.
    //
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(m_ac1Msg);
    tCIDLib::TCard1 ac1Tmp[c4Count];
    TRawMem::SetMemBuf(ac1Tmp, tCIDLib::TCard1(0), c4Count);

    tCIDLib::TCard4 c4CheckSum = 0;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < m_c4Len; c4Index++)
    {
        const tCIDLib::TCard1 c1Cur = m_ac1Msg[c4Index];
        ac1Tmp[c1Cur] = 1;
        c4CheckSum += c1Cur;
    }

    //
    //  Clip back the checksum to byte size and mark it's entry in the
    //  used values array.
    //
    c4CheckSum &= 0xFF;
    ac1Tmp[c4CheckSum] = 1;

    // Now lets find the first non-zero unused value
    for (c4Index = 1; ac1Tmp[c4Index] && (c4Index < 255); c4Index++);

    // Sanity check that we really did find one
    if (c4Index == 255)
    {
        facBarcoCRTS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBarcoSErrs::errcMsg_NoFreeValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Convert that index to a TCard1 for use as the offset
    const tCIDLib::TCard1 c1Ofs = 0; // tCIDLib::TCard1(c4Index);

    //
    //  Ok lets build up the translated message. We have to offset all of
    //  the original msg bytes by the offset value, then put the checksum
    //  on, also offset.
    //
    m_ac1Xlated[0] = kBarcoCRTS::c1STX;
    m_ac1Xlated[1] = c1Ofs;
    for (c4Index = 0; c4Index < m_c4Len; c4Index++)
        m_ac1Xlated[c4Index + 2] = m_ac1Msg[c4Index] + c1Ofs;
    m_ac1Xlated[c4Index + 2] = tCIDLib::TCard1(c4CheckSum) + c1Ofs;

    //
    //  Store the xlated len. Its the original, plus the STX, the offset, and
    //  the trailing checksum.
    //
    m_c4XlatedLen = m_c4Len + 3;

    // Clear the changes flag
    m_bChanges = kCIDLib::False;
}


const tCIDLib::TCh* TBarcoMsg::pszXlatCmd(const tBarcoCRTS::ECmds eCmd)
{
    if (!bIsValidEnum(eCmd))
    {
        // We are throwing a CIDLib error here, not one of ours!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_BadEnumValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , TInteger(eCmd)
            , TString(L"tBarcoCRTS::ECmds")
        );
    }
    return BarcoCRTS_Msg::apszCmdNames[eCmd];
}

