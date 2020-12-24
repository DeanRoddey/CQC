//
// FILE NAME: ZWaveUSB3Sh_InMsg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2017
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
//  This is the implementation for the incoming message wrapper class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TZWInMsg
//  PREFIX: zwim
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWInMsg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWInMsg::TZWInMsg() :

    m_bSecure(kCIDLib::False)
    , m_bWasEncap(kCIDLib::False)
    , m_c1SrcEPId(0)
    , m_c1TarEPId(0)
    , m_c1MsgId(0)
    , m_c1PLLength(0)
    , m_enctReceived(0)
    , m_eType(tZWaveUSB3Sh::EMsgTypes::Unknown)
    , m_pzwimOrg(nullptr)
{
}

TZWInMsg::TZWInMsg(const TZWInMsg& zwimSrc) :

    m_bSecure(zwimSrc.m_bSecure)
    , m_bWasEncap(zwimSrc.m_bWasEncap)
    , m_c1SrcEPId(zwimSrc.m_c1SrcEPId)
    , m_c1TarEPId(zwimSrc.m_c1TarEPId)
    , m_c1MsgId(zwimSrc.m_c1MsgId)
    , m_c1PLLength(zwimSrc.m_c1PLLength)
    , m_enctReceived(zwimSrc.m_enctReceived)
    , m_eType(zwimSrc.m_eType)
    , m_mbufPayload(zwimSrc.m_mbufPayload)
    , m_pzwimOrg(nullptr)
{
    // Make sure the secure and original contents states agree
    CIDAssert
    (
        zwimSrc.m_bSecure == (zwimSrc.m_pzwimOrg != nullptr)
        , L"Nonsecure in msg had original msg content or vice versa"
    );

    if (zwimSrc.m_pzwimOrg)
        m_pzwimOrg = new TZWInMsg(*zwimSrc.m_pzwimOrg);
}


TZWInMsg::~TZWInMsg()
{
    delete m_pzwimOrg;
}


// ---------------------------------------------------------------------------
//  TZWInMsg: Public operators
// ---------------------------------------------------------------------------
TZWInMsg& TZWInMsg::operator=(const TZWInMsg& zwimSrc)
{
    if (&zwimSrc != this)
    {
        // Make sure the secure and original contents states agree
        CIDAssert
        (
            zwimSrc.m_bSecure == (zwimSrc.m_pzwimOrg != nullptr)
            , L"Nonsecure in msg had original msg content or vice versa"
        );

        m_bSecure       = zwimSrc.m_bSecure;
        m_bWasEncap     = zwimSrc.m_bWasEncap;
        m_c1SrcEPId     = zwimSrc.m_c1SrcEPId;
        m_c1TarEPId     = zwimSrc.m_c1TarEPId;
        m_c1MsgId       = zwimSrc.m_c1MsgId;
        m_c1PLLength    = zwimSrc.m_c1PLLength;
        m_enctReceived  = zwimSrc.m_enctReceived;
        m_eType         = zwimSrc.m_eType;
        m_mbufPayload   = zwimSrc.m_mbufPayload;

        if (zwimSrc.m_pzwimOrg)
        {
            // If the source has an original msg, then copy it
            if (!m_pzwimOrg)
                m_pzwimOrg = new TZWInMsg(*zwimSrc.m_pzwimOrg);
            else
                *m_pzwimOrg = *zwimSrc.m_pzwimOrg;
        }
         else
        {
            delete m_pzwimOrg;
            m_pzwimOrg = nullptr;
        }
    }
    return *this;
}

//
//  Get a payload byte at a particular index
//
tCIDLib::TCard1 TZWInMsg::operator[](const tCIDLib::TCard4 c4At) const
{
    CIDAssert(c4At < m_c1PLLength, L"Invalid payload index");
    return m_mbufPayload[c4At];
}



// ---------------------------------------------------------------------------
//  TZWInMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given the appropriate nonce, we decrypt ourself and update our contents to reflect
//  the real message that we extract.
//
tCIDLib::TBoolean TZWInMsg::bDecrypt(const TMemBuf& mbufNonce)
{
    // It has to be an app command msg
    tCIDLib::TCard1 c1OrgClass;
    tCIDLib::TCard1 c1OrgCmd;
    if (!bIsAppCmd(c1OrgClass, c1OrgCmd))
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcMsg_NotCmdClassMsg
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        return kCIDLib::False;
    }

    //
    //  Build up the IV. The first 8 bytes are the IV bytes in the message, and
    //  the second 8 are the nonce bytes.
    //
    THeapBuf mbufIV(16, 16);
    CopyOutPLBytes(mbufIV, c4FirstCCIndex() + 2, 8);
    mbufIV.CopyIn(mbufNonce, 8, 8);

    //
    //  Figure out how many encrypted bytes. We remove
    //
    //  1. Security msgs's flags, unit it, class bytes count, class and id (5)
    //  2. IV bytes (8)
    //  3. Nonce id (1)
    //  4. MAC bytes (8)
    //
    const tCIDLib::TCard4 c4EncBytes(m_c1PLLength - 22);

    // Check for obviously bad length (class, cmd id, and the reserved security byte)
    if (c4EncBytes < 3)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcProto_EncPayloadSz
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        return kCIDLib::False;
    }

    //
    //  Get those bytes out. Zero the buffer first so any unused bytes will be
    //  zero padded.
    //
    THeapBuf mbufEncBytes(32, 32);
    mbufEncBytes.Set(0);
    CopyOutPLBytes(mbufEncBytes, c4FirstCCIndex() + 10, c4EncBytes);

    // And decrypt them
    THeapBuf mbufPlainBytes(32, 32);
    facZWaveUSB3Sh().Decrypt(mbufEncBytes, c4EncBytes, mbufPlainBytes, mbufIV);

    //
    //  Generate the MAC for this guy and compare it the sent MAC. We pass him the
    //  encrypted part of the message.
    //
    const tCIDLib::TCard1 c1ClassBytes = tCIDLib::TCard1(c4CCBytes());
    THeapBuf mbufMAC(8, 8);
    facZWaveUSB3Sh().CalculateMAC
    (
        c1OrgCmd
        , mbufEncBytes.pc1Data()
        , c4EncBytes
        , c1SrcId()
        , facZWaveUSB3Sh().c1CtrlId()
        , mbufIV
        , mbufMAC
    );

    // The MAC starts 8 bytes back from the last CC byte
    const tCIDLib::TCard4 c4MACInd = c4LastCCIndex() - 7;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
    {
        if (mbufMAC[c4Index] != m_mbufPayload[c4MACInd + c4Index])
        {
            if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcProto_BadMAC
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(c1SrcId())
                );
            }
            return kCIDLib::False;
        }
    }

    // Save a copy to the original msg member
    if (m_pzwimOrg)
        *m_pzwimOrg = *this;
    else
        m_pzwimOrg = new TZWInMsg(*this);

    //
    //  Up to now we've not change our current contents. But we need to update our
    //  state now to reflect the new msg. We keep the original receipt flags and
    //  source id bytes, and start on byte 2.
    //
    tCIDLib::TCard1 c1Cnt = 2;

    // Skip the first decrypted byte which is part of the encapsulation
    m_mbufPayload.PutCard1(tCIDLib::TCard1(c4EncBytes - 1), c1Cnt++);
    for (tCIDLib::TCard4 c4Index = 1; c4Index < c4EncBytes; c4Index++)
        m_mbufPayload.PutCard1(mbufPlainBytes[c4Index], c1Cnt++);

    // Indicate it was received secure
    m_bSecure = kCIDLib::True;
    m_c1MsgId = FUNC_ID_APPLICATION_COMMAND_HANDLER;
    m_c1PLLength = c1Cnt;
    m_eType = tZWaveUSB3Sh::EMsgTypes::Request;

    return kCIDLib::True;
}


//
//  A helper to check if this is a app command (a command class) message. If so
//  it returns various bits of useful data.
//
tCIDLib::TBoolean TZWInMsg::bIsAppCmd() const
{
    if (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
    {
        if (m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TZWInMsg::bIsAppCmd(tCIDLib::TCard1& c1ClassId, tCIDLib::TCard1& c1CmdId) const
{
    if (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
    {
        if ((m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
        &&  (m_c1PLLength >= kZWaveUSB3Sh::c4MinCCPayload))
        {
            c1ClassId = m_mbufPayload[3];
            c1CmdId = m_mbufPayload[4];
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}

tCIDLib::TBoolean
TZWInMsg::bIsAppCmd(tCIDLib::TCard1&    c1ClassId
                    , tCIDLib::TCard1&  c1CmdId
                    , tCIDLib::TCard1&  c1UnitId
                    , tCIDLib::TCard1&  c1ReceiptFlags) const
{
    if (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
    {
        if ((m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
        &&  (m_c1PLLength >= kZWaveUSB3Sh::c4MinCCPayload))
        {
            c1ReceiptFlags = m_mbufPayload[0];
            c1UnitId = m_mbufPayload[1];
            c1ClassId = m_mbufPayload[3];
            c1CmdId = m_mbufPayload[4];
            return kCIDLib::True;
        }
    }

    return kCIDLib::False;
}


// Returns true if this is an application update message
tCIDLib::TBoolean TZWInMsg::bIsAppUpdate() const
{
    if (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
    {
        if (m_c1MsgId == FUNC_ID_ZW_APPLICATION_UPDATE)
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  A helper to check if it's a basic report and return the state if so. We also
//  handle a set, since sometimes we get a message by something that thinks it is
//  controlling us, not sending us a report.
//
tCIDLib::TBoolean TZWInMsg::bIsBasicReport(tCIDLib::TBoolean& bState) const
{
    // Call the app command checker first
    tCIDLib::TCard1 c1ClassId, c1CmdId;
    if (bIsAppCmd(c1ClassId, c1CmdId))
    {
        if ((c1ClassId == COMMAND_CLASS_BASIC)
        &&  ((c1CmdId == BASIC_SET) || (c1CmdId == BASIC_REPORT)))
        {
            // We have to have min playload plus 1 bytes at least
            if (m_c1PLLength >= kZWaveUSB3Sh::c4MinCCPayload + 1)
            {
                bState = m_mbufPayload[5] != 0;
                return kCIDLib::True;
            }
        }
    }
    return kCIDLib::False;
}


//
//  If it's an app command the rx flags indicate single-cast, return true. If not
//  an app command, this is meaningless. So if it's not an app command, or it is but
//  it's a single cast.
//
tCIDLib::TBoolean TZWInMsg::bIsSingleCast() const
{
    const tCIDLib::TBoolean bIsAC = bIsAppCmd();
    CIDAssert(bIsAC, L"bIsSingleCast is only meaningful for app commands");
    return
    (
        !bIsAC || ((m_mbufPayload[0] & RECEIVE_STATUS_TYPE_MASK) == RECEIVE_STATUS_TYPE_SINGLE)
    );
}


tCIDLib::TBoolean TZWInMsg::bIsWakeUp() const
{
    // Call the app command checker first
    tCIDLib::TCard1 c1ClassId, c1CmdId;
    if (bIsAppCmd(c1ClassId, c1CmdId))
    {
        if ((c1ClassId == COMMAND_CLASS_WAKE_UP) &&  (c1CmdId == WAKE_UP_NOTIFICATION))
            return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  If it's a message type that has a callback id, return it. Else return zero.
//
//  NOTE THAT SEND_DATA has one, but that's only sent out. We never get those back. We
//  only get an application command msg.
//
tCIDLib::TCard1 TZWInMsg::c1CallbackId() const
{
    CIDAssert(m_c1PLLength > 0, L"Can't get CB id from zero length payload");

    //  Only requests have callback ids, or the transmit ack
    if ((m_eType != tZWaveUSB3Sh::EMsgTypes::Request)
    &&  (m_eType != tZWaveUSB3Sh::EMsgTypes::TransAck))
    {
        return kCIDLib::False;
    }

    // See if it's a msg type that has a callback id
    switch(m_c1MsgId)
    {
        case FUNC_ID_STORE_NODEINFO :

        case FUNC_ID_ZW_ADD_NODE_TO_NETWORK :
        case FUNC_ID_ZW_ASSIGN_RETURN_ROUTE :
        case FUNC_ID_ZW_ASSIGN_SUC_RETURN_ROUTE :
        case FUNC_ID_ZW_CONTROLLER_CHANGE :
        case FUNC_ID_ZW_CREATE_NEW_PRIMARY :
        case FUNC_ID_ZW_DELETE_RETURN_ROUTE :
        case FUNC_ID_ZW_DELETE_SUC_RETURN_ROUTE :
        case FUNC_ID_ZW_REMOVE_FAILED_NODE_ID :
        case FUNC_ID_ZW_REMOVE_NODE_FROM_NETWORK :
        case FUNC_ID_ZW_REPLACE_FAILED_NODE :
        case FUNC_ID_ZW_REPLICATION_SEND_DATA :
        case FUNC_ID_ZW_REQUEST_NODE_NEIGHBOR_UPDATE :
        case FUNC_ID_ZW_REQUEST_NETWORK_UPDATE :
        case FUNC_ID_ZW_SEND_DATA :
        case FUNC_ID_ZW_SEND_DATA_META :
        case FUNC_ID_ZW_SEND_DATA_MULTI :
        case FUNC_ID_ZW_SEND_NODE_INFORMATION :
        case FUNC_ID_ZW_SEND_SUC_ID :
        case FUNC_ID_ZW_SEND_TEST_FRAME :
        case FUNC_ID_ZW_SET_DEFAULT :
        case FUNC_ID_ZW_SET_LEARN_MODE :
        case FUNC_ID_ZW_SET_SUC_NODE_ID :
            return m_mbufPayload[0];

        default :
            break;
    };
    return 0;
}


//
//  Calculate a checksum on the message we contain. This is used when reading in
//  a message, to validate the received checksum. We don't store it all in a flat
//  buffer, so we have to build it up from bits.
//
tCIDLib::TCard1 TZWInMsg::c1CalcSum() const
{
    CIDAssert
    (
        (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
        || (m_eType == tZWaveUSB3Sh::EMsgTypes::Response)
        , L"Check sum calculation is only valid for request or response messages"
    );

    // We start off with FF and XOR the bytes against that
    tCIDLib::TCard1 c1Ret = 0xFF;

    // First is the overall msg length, which is our payload length plus 3
    c1Ret ^= (m_c1PLLength + 3);

    // Then the message type
    if (m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
        c1Ret ^= tCIDLib::TCard1(REQUEST);
    else
        c1Ret ^= tCIDLib::TCard1(RESPONSE);

    // Then the function id
    c1Ret ^= m_c1MsgId;

    // And then any remaining payload bytes
    for (tCIDLib::TCard4 c4Index = 0; c4Index < tCIDLib::TCard4(m_c1PLLength); c4Index++)
        c1Ret ^= m_mbufPayload[c4Index];

    return c1Ret;
}


// Return the class id or cmd id of a command class msg
tCIDLib::TCard1 TZWInMsg::c1ClassId() const
{
    tCIDLib::TCard1 c1Class, c1Cmd;
    if (bIsAppCmd(c1Class, c1Cmd))
        return c1Class;

    return 0;
}

tCIDLib::TCard1 TZWInMsg::c1CmdId() const
{
    tCIDLib::TCard1 c1Class, c1Cmd;
    if (bIsAppCmd(c1Class, c1Cmd))
        return c1Cmd;

    return 0;
}


//
//  If it's an application command msg, this will get a particular byte of the command
//  class part of the payload. It starts at byte 3 in the actual message (the command
//  class id.) This allows you to use offsets as they are defined in the command
//  class msg format docs.
//
tCIDLib::TCard1 TZWInMsg::c1CCByteAt(const tCIDLib::TCard4 c4At) const
{
    if (!bIsAppCmd())
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
        );
    }
    return m_mbufPayload[c4At + 3];
}


//
//  Returns the msg id. All incoming messages other than ack/nak have one, and ack/nack don't
//  get reported via an input msg object.
//
tCIDLib::TCard1 TZWInMsg::c1MsgId() const
{
    return m_c1MsgId;
}


//
//  Return the source unit id. If it isn't a msg type that has one, then we return
//  zero.
//
tCIDLib::TCard1 TZWInMsg::c1SrcId(const tCIDLib::TBoolean bThrowIfNot) const
{
    // Only request msgs have unit ids in them
    if (m_eType != tZWaveUSB3Sh::EMsgTypes::Request)
        return 0;

    switch(m_c1MsgId)
    {
        case FUNC_ID_APPLICATION_COMMAND_HANDLER :
        case FUNC_ID_ZW_APPLICATION_CONTROLLER_UPDATE :
            // In both cases they come after a status byte
            return m_mbufPayload[1];

        default :
            break;
    };

    if (bThrowIfNot)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NoUnitId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1MsgId(), tCIDLib::ERadices::Hex)
        );
    }
    return 0;


}


tCIDLib::TCard4 TZWInMsg::c4CCBytes() const
{
    if (!bIsAppCmd())
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
        );
    }
    return m_mbufPayload[2];
}


tCIDLib::TCard4 TZWInMsg::c4FirstCCIndex() const
{
    // This is always just 3, skipping the flags, src id, and byte count
    return 3;
}


//
//  Return the index of the last command class byte. So that's 3 plus the byte at 2
//  minus 1. The shorted CC msg would be just the class id and cmd id, in which case
//  this would point at the cmd id.
//
tCIDLib::TCard4 TZWInMsg::c4LastCCIndex() const
{
    if (!bIsAppCmd())
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
        );
    }
    return 3 + (m_mbufPayload[2] - 1);
}


//
//  This will convert an multi-channel encapsulated messages to a regular one, which just
//  requires removing some bytes. It'll stores the end point id, and returns the new
//  class and cmd ids.
//
//  We return the new
//
tCIDLib::TVoid TZWInMsg::ConvertEncap(tCIDLib::TCard1& c1ClassId, tCIDLib::TCard1& c1CmdId)
{
    // Make sure not already done
    CIDAssert(m_bWasEncap == kCIDLib::False, L"InMsg is already de-encapsulated");

    //
    //  Get out the end point byte first, then cut the encapsulation bytes out. That
    //  will adjust the payload size appropriately. Set the was encapsulated flag.
    //
    m_bWasEncap = kCIDLib::True;
    m_c1SrcEPId = c1CCByteAt(2);
    m_c1TarEPId = c1CCByteAt(3);
    CutPLBytes(3, 4);

    // Give the caller back the actual ids
    c1ClassId = m_mbufPayload[3];
    c1CmdId = m_mbufPayload[4];
}


// Copy bytes out from our payload to the caller's buffer
tCIDLib::TVoid
TZWInMsg::CopyOutPLBytes(       TMemBuf&        mbufToFill
                        , const tCIDLib::TCard4 c4Start
                        , const tCIDLib::TCard4 c4Count) const
{
    m_mbufPayload.CopyOut(mbufToFill, c4Count, c4Start);
}


//
//  This will remove payload bytes from the msg. The most common reason is to convert
//  an encapsulated multi-channel command to a regular format one, after pulling the
//  multi-channel leading stuff out.
//
tCIDLib::TVoid
TZWInMsg::CutPLBytes(const tCIDLib::TCard4 c4Start, const tCIDLib::TCard4 c4Count)
{
    if (c4Start + c4Count >= tCIDLib::TCard4(m_c1PLLength))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_CutRange
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Count)
            , TCardinal(c4Start)
            , TCardinal(m_c1PLLength)
        );
    }

    tCIDLib::TCard4 c4SrcIndex = c4Start + c4Count;;
    tCIDLib::TCard4 c4TarIndex = c4Start;
    while (c4SrcIndex < m_c1PLLength)
        m_mbufPayload.PutCard1(m_mbufPayload[c4SrcIndex++], c4TarIndex++);

    // Adjust the lengths
    m_c1PLLength -= tCIDLib::TCard1(c4Count);

    // If a CC msg, adjust the CC byte count
    if (m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
        m_mbufPayload.PutCard1(m_mbufPayload[2] - tCIDLib::TCard1(c4Count), 2);
}


// For now we just set the received time
tCIDLib::TVoid TZWInMsg::Finalize()
{
    m_enctReceived = TTime::enctNow();
}


tCIDLib::TEncodedTime TZWInMsg::enctReceived() const
{
    return m_enctReceived;
}


//
//  Provide access to the command class oriented payload bytes. One returns the start
//  of it, and another the byte past the end.
//
const tCIDLib::TCard1* TZWInMsg::pc1CCPayload() const
{
    CIDAssert(m_c1MsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER, L"Not a command class msg");

    // It starts on the third byte of a command class message
    return m_mbufPayload.pc1DataAt(3);
}


//
//  Provide access to the raw payload bytes. One returns the start of it, and another
//  the byte past the end.
//
const tCIDLib::TCard1* TZWInMsg::pc1Payload() const
{
    return m_mbufPayload.pc1Data();
}

const tCIDLib::TCard1* TZWInMsg::pc1PayloadEnd() const
{
    return m_mbufPayload.pc1DataAt(m_c1PLLength);
}



// Provide access to the payload array at a particular index
const tCIDLib::TCard1* TZWInMsg::pc1PayloadAt(const tCIDLib::TCard4 c4At) const
{
    CIDAssert(c4At < m_c1PLLength, L"Invalid payload index");
    return m_mbufPayload.pc1DataAt(c4At);
}



//
//  Reset this object in preparation for reuse. It takes a payload bytes because it's
//  used by the message reading method, which will call this when it gets the message
//  size byte, to set the payload length that it's going to fill in.
//
tCIDLib::TVoid TZWInMsg::Reset(const tCIDLib::TCard1 c1PLBytes)
{
    m_bSecure = kCIDLib::False;
    m_bWasEncap = kCIDLib::False;
    m_c1MsgId = 0;
    m_c1SrcEPId = 0;
    m_c1TarEPId = 0;
    m_eType = tZWaveUSB3Sh::EMsgTypes::Unknown;
    m_c1PLLength = c1PLBytes;
    delete m_pzwimOrg;
    m_pzwimOrg = nullptr;
}


// Set a particular payload byte
tCIDLib::TVoid
TZWInMsg::SetPLByteAt(const tCIDLib::TCard4 c4At, const tCIDLib::TCard1 c1ToSet)
{
    if (c4At >= tCIDLib::TCard4(m_c1PLLength))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_PLByteIndex
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , TCardinal(m_c1PLLength)
        );
    }
    m_mbufPayload.PutCard1(c1ToSet, c4At);
}
