//
// FILE NAME: ZWaveUSB3Sh_OutMsg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/20/2017
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
//  This is the implementation for the outgoing message wrapper class.
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
//   CLASS: TZWOutMsg
//  PREFIX: zwm
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWOutMsg: Constructors and Destructor
// ---------------------------------------------------------------------------

TZWOutMsg::TZWOutMsg() :

    m_bFreqListener(kCIDLib::False)
    , m_bIsCCMsg(kCIDLib::False)
    , m_bReqNonce(kCIDLib::False)
    , m_bSecure(kCIDLib::False)
    , m_bTransOpts(kCIDLib::False)
    , m_c1CBIdOfs(0)
    , m_c1Count(0)
    , m_c1CCReplyClass(0)
    , m_c1CCReplyCmd(0)
    , m_c1MsgId(0)
    , m_c1ReplyMsgId(0)
    , m_c1TarId(0)
    , m_c1TransOpts(0)
    , m_c1TransOptsOfs(0)
    , m_c4AckId(0)
    , m_c4SendCost(0)
    , m_c4SendCount(0)
    , m_ePriority(tZWaveUSB3Sh::EMsgPrios::Query)
    , m_eState(tZWaveUSB3Sh::EOMState_Working)
    , m_eType(tZWaveUSB3Sh::EMsgTypes::Unknown)
    , m_fcolCounters(kZWaveUSB3Sh::c4OutMsgCounters)
    , m_mbufData(32, 128)
    , m_pzwomOrg(nullptr)
{
}


// Send count/cost isn't copied
TZWOutMsg::TZWOutMsg(const TZWOutMsg& zwomSrc) :

    m_bFreqListener(zwomSrc.m_bFreqListener)
    , m_bIsCCMsg(zwomSrc.m_bIsCCMsg)
    , m_bReqNonce(zwomSrc.m_bReqNonce)
    , m_bSecure(zwomSrc.m_bSecure)
    , m_bTransOpts(zwomSrc.m_bTransOpts)
    , m_c1CBIdOfs(zwomSrc.m_c1CBIdOfs)
    , m_c1Count(zwomSrc.m_c1Count)
    , m_c1CCReplyClass(zwomSrc.m_c1CCReplyClass)
    , m_c1CCReplyCmd(zwomSrc.m_c1CCReplyCmd)
    , m_c1MsgId(zwomSrc.m_c1MsgId)
    , m_c1ReplyMsgId(zwomSrc.m_c1ReplyMsgId)
    , m_c1TarId(zwomSrc.m_c1TarId)
    , m_c1TransOpts(zwomSrc.m_c1TransOpts)
    , m_c1TransOptsOfs(zwomSrc.m_c1TransOptsOfs)
    , m_c4AckId(zwomSrc.m_c4AckId)
    , m_c4SendCost(0)
    , m_c4SendCount(0)
    , m_ePriority(zwomSrc.m_ePriority)
    , m_eState(zwomSrc.m_eState)
    , m_eType(zwomSrc.m_eType)
    , m_fcolCounters(kZWaveUSB3Sh::c4OutMsgCounters)
    , m_mbufData(zwomSrc.m_mbufData)
    , m_pzwomOrg(nullptr)
{
    if (zwomSrc.m_pzwomOrg)
        m_pzwomOrg = new TZWOutMsg(*zwomSrc.m_pzwomOrg);
}


TZWOutMsg::~TZWOutMsg()
{
    // If there was an original unencrypted version, delete it
    try
    {
        delete m_pzwomOrg;
        m_pzwomOrg = nullptr;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TZWOutMsg: Public operators
// ---------------------------------------------------------------------------

TZWOutMsg& TZWOutMsg::operator=(const TZWOutMsg& zwomSrc)
{
    if (&zwomSrc != this)
    {
        m_bFreqListener     = zwomSrc.m_bFreqListener;
        m_bIsCCMsg          = zwomSrc.m_bIsCCMsg;
        m_bReqNonce         = zwomSrc.m_bReqNonce;
        m_bSecure           = zwomSrc.m_bSecure;
        m_bTransOpts        = zwomSrc.m_bTransOpts;
        m_c1CBIdOfs         = zwomSrc.m_c1CBIdOfs;
        m_c1Count           = zwomSrc.m_c1Count;
        m_c1CCReplyClass    = zwomSrc.m_c1CCReplyClass;
        m_c1CCReplyCmd      = zwomSrc.m_c1CCReplyCmd;
        m_c1MsgId           = zwomSrc.m_c1MsgId;
        m_c1ReplyMsgId      = zwomSrc.m_c1ReplyMsgId;
        m_c1TarId           = zwomSrc.m_c1TarId;
        m_c4AckId           = zwomSrc.m_c4AckId;
        m_c1TransOpts       = zwomSrc.m_c1TransOpts;
        m_c1TransOptsOfs    = zwomSrc.m_c1TransOptsOfs;
        m_ePriority         = zwomSrc.m_ePriority;
        m_eState            = zwomSrc.m_eState;
        m_eType             = zwomSrc.m_eType;
        m_mbufData          = zwomSrc.m_mbufData;

        // Counters are not copied or send count/cost
        m_c4SendCost = 0;
        m_c4SendCount = 0;
        m_fcolCounters.RemoveAll();
        if (zwomSrc.m_pzwomOrg)
        {
            // If the source has an original msg, then copy it
            if (!m_pzwomOrg)
                m_pzwomOrg = new TZWOutMsg(*zwomSrc.m_pzwomOrg);
            else
                *m_pzwomOrg = *zwomSrc.m_pzwomOrg;
        }
         else
        {
            delete m_pzwomOrg;
            m_pzwomOrg = nullptr;
        }
    }
    return *this;
}


// Adds another bytes to the message
tCIDLib::TVoid TZWOutMsg::operator+=(const tCIDLib::TCard1 c1ToAdd)
{
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in working state to add bytes"
    );
    m_mbufData.PutCard1(c1ToAdd, m_c1Count++);
}


//
//  Access msg bytes. It doesn't have to be complete. the caller just has to be sure
//  not to access it beyond what is there.
//
tCIDLib::TCard1 TZWOutMsg::operator[](const tCIDLib::TCard4 c4At) const
{
    return m_mbufData[c4At];
}


// ---------------------------------------------------------------------------
//  TZWOutMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Add the passed bytes to this message
tCIDLib::TVoid TZWOutMsg::AppendBytes(const TMemBuf& mbufSrc, const tCIDLib::TCard4 c4Count)
{
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in working state to add bytes"
    );
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_mbufData.PutCard1(mbufSrc[c4Index], m_c1Count++);
}


//
//  Append the next available callback id to this msg. There is a special case, currently only
//  replication, where we need a non-blocking callback. If that is asked for, we just set it
//  to 0xFF, which is treated by the I/O thread as not needing a callback so it won't wait.
//
tCIDLib::TVoid TZWOutMsg::AppendCallback(const tCIDLib::TBoolean bNonBlocking)
{
    tCIDLib::TCard1 c1NewCBId;
    if (bNonBlocking)
        c1NewCBId = 0xFF;
    else
        c1NewCBId = c1NextCBId();

    // Remember the offset where we put it
    m_c1CBIdOfs = m_c1Count;

    // Store the callback id in the message now as well
    m_mbufData.PutCard1(c1NewCBId, m_c1Count++);
}


//
//  If we have to have a callback byte, but don't want a callback, this is used. We don't
//  set the callback id member to zero as well.
//
tCIDLib::TVoid TZWOutMsg::AppendNullCallback()
{
    // Remember the offset where we put it
    m_c1CBIdOfs = m_c1Count;

    m_mbufData.PutCard1(0, m_c1Count++);
}


//
//  Append transmit options to the msg.
//
tCIDLib::TVoid TZWOutMsg::AppendTransOpts(const tCIDLib::TBoolean bReceiptAck)
{
    m_c1TransOpts = kZWaveUSB3Sh_::c1DefTransOpts;
//    if (bReceiptAck)
        m_c1TransOpts |= TRANSMIT_OPTION_ACK;

    // Store the options and remember we have them
    m_c1TransOptsOfs = m_c1Count;
    m_mbufData.PutCard1(m_c1TransOpts, m_c1Count++);
    m_bTransOpts = kCIDLib::True;
}


//
//  A ping is a special case in some places so we have an easy way to check for it. If
//  it is a CC msg and the command class and command are both zero, it's a ping.
//
tCIDLib::TBoolean TZWOutMsg::bIsPing() const
{
    return (m_bIsCCMsg && !m_mbufData[5] && !m_mbufData[6]);
}


//
//  If a callback id was ever set, and it is non-zero, then we should be getting a
//  callback. There is a special case of 0xFF, for a couple special case msgs where
//  we are going to get a callback, but not in a block way, such as during the learn
//  mode replication loop. 0xFF is a special callback id for that. We report that we
//  don't need to wait for a callback, but later there will be some coming in with
//  this callback id.
//
tCIDLib::TBoolean TZWOutMsg::bNeedsCallback() const
{
    if (!m_c1CBIdOfs)
        return kCIDLib::False;

    const tCIDLib::TCard1 c1CBId = m_mbufData[m_c1CBIdOfs];
    return (c1CBId != 0) && (c1CBId != 0xFF);
}


//
//  If an send data, this will return true and fill in the CC ids. It must be finalized.
//
tCIDLib::TBoolean
TZWOutMsg::bQueryCCIds(tCIDLib::TCard1& c1ClassId, tCIDLib::TCard1& c1CmdId) const
{
    CIDAssert
    (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be finalized to get CC ids"
    );


    // If this is a command class msg, this will return the command class and cmd id
    if (m_bIsCCMsg)
    {
        c1ClassId = m_mbufData[5];
        c1CmdId = m_mbufData[6];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This must be a finalized command class msg.
//
tCIDLib::TCard1 TZWOutMsg::c1CallbackId(const tCIDLib::TBoolean  bThrowIfNot) const
{
    CIDAssert
    (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in complete state to access callback id"
    );
    CIDAssert(m_c1CBIdOfs != kCIDLib::c4MaxCard, L"No callback was ever set on this msg");

    return m_mbufData[m_c1CBIdOfs];
}


// If this is a command class msg it returns the command class id
tCIDLib::TCard1 TZWOutMsg::c1ClassId() const
{
    // Make sure this is a a command class message
    if (!m_bIsCCMsg)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return m_mbufData[5];
}


// If this is a command class msg it returns the command id
tCIDLib::TCard1 TZWOutMsg::c1CmdId() const
{
    // Make sure this is a a command class message
    if (!m_bIsCCMsg)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return m_mbufData[6];
}


//
//  Return a byte at the indicated index, relative to the CC bytes, so relative to the
//  CC command class id byte.
//
tCIDLib::TCard1 TZWOutMsg::c1CCByteAt(const tCIDLib::TCard4 c4At) const
{
    // Make sure this is a a command class message
    if (!m_bIsCCMsg)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return m_mbufData[5 + c4At];
}


//
//  Return the target id, if it has one, else zero. If the msg type of this msg is one
//  that includes a unit id.
//
tCIDLib::TCard1 TZWOutMsg::c1TarId(const tCIDLib::TBoolean bThrowIfNot) const
{
    CIDAssert
    (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in complete state to access target id"
    );

    if (!m_c1TarId)
    {
        if (bThrowIfNot)
        {
            facZWaveUSB3Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcMsg_NoUnitId
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c1MsgId, tCIDLib::ERadices::Hex)
            );
        }
        return 0;
    }
    return m_c1TarId;
}


//
//  This must be a finalized command class msg.
//
tCIDLib::TCard1 TZWOutMsg::c1TransOpts(const tCIDLib::TBoolean bThrowIfNot) const
{
   CIDAssert
   (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in complete state to access trans opts"
    );

    if (!m_bTransOpts)
    {
        if (bThrowIfNot)
        {
            facZWaveUSB3Sh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcMsg_NoTransOpts
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(m_c1MsgId, tCIDLib::ERadices::Hex)
            );
        }
        return 0;
    }
    return m_c1TransOpts;
}


// Returns the number of command class bytes, assuming this is a cc msg. It must be finalized
tCIDLib::TCard4 TZWOutMsg::c4CCBytes() const
{
    CIDAssert
    (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in complete state to access target id"
    );

    if (!m_bIsCCMsg)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return m_mbufData[4];
}


//
//  Copy out all of the command class bytes, and reutrn how many. It must be finalized
tCIDLib::TCard4
TZWOutMsg::c4CopyOutCCBytes(TMemBuf& mbufToFill, const tCIDLib::TCard4 c4TarInd) const
{
    CIDAssert
    (
        m_eState > tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in complete state to access CC bytes"
    );

    if (!m_bIsCCMsg)
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcMsg_NotCmdClassMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    const tCIDLib::TCard4 c4Count = m_mbufData[4];
    if (c4TarInd)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            mbufToFill.PutCard1(m_mbufData[c4Index + 5], c4Index + c4TarInd);
    }
     else
    {
        // DO the quick version
        m_mbufData.CopyOut(mbufToFill, c4Count, 5);
    }

    return c4Count;
}


// Bump the send cost by the increment passed
tCIDLib::TCard4 TZWOutMsg::c4IncSendCost(const tCIDLib::TCard4 c4IncBy)
{
    m_c4SendCost += c4IncBy;
    return m_c4SendCost;
}


// Updates transmit options after the fact
tCIDLib::TVoid TZWOutMsg::ChangeTransOpts(const tCIDLib::TCard1 c1ToSet)
{
    if (!m_c1TransOptsOfs)
        m_mbufData.PutCard1(c1ToSet, m_c1TransOptsOfs);
}

//
//  In some cases a msgs priority will need to be changed after it has been created
//  and finalized. A common one is that msgs that can't be sent because the unit is
//  not a listener will be queued up for later sending. But, when we finally get a
//  Wakeup and need to send them, we want them sent quickly before it goes back to
//  sleep, so the priority is updated. There may be other reasons.
//
tCIDLib::TVoid TZWOutMsg::ChangePriority(const tZWaveUSB3Sh::EMsgPrios eNewPrio)
{
    m_ePriority = eNewPrio;
}


// Return our msg type which would alwys be request or response, and almost always request
tZWaveUSB3Sh::EMsgTypes TZWOutMsg::eType() const
{
    CIDAssert
    (
        m_c1Count >= 3, L"Msg not larger enough to have a type indicator"
    );

    tZWaveUSB3Sh::EMsgTypes eRet = tZWaveUSB3Sh::EMsgTypes::Unknown;
    if (m_mbufData[1] == REQUEST)
        eRet = tZWaveUSB3Sh::EMsgTypes::Request;
    else if (m_mbufData[1] == RESPONSE)
        eRet = tZWaveUSB3Sh::EMsgTypes::Response;

    return eRet;
}


//
//  If this message needs to be sent secure, this is called before it is sent to
//  encrypt it.
//
tCIDLib::TVoid TZWOutMsg::Encrypt(const TMemBuf& mbufNonce)
{
    // We have to be in WaitEncrypt state
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_WaitEncrypt
        , L"Msg must be in wait nonce state to encrypt"
    );

    // Get the target id out since we need it a few times below
    const tCIDLib::TCard1 c1TargetId = c1TarId();

    // Save the callback. CC msgs always have them
    CIDAssert(m_c1CBIdOfs != 0, L"CC msg has no callback id offset stored")
    const tCIDLib::TCard1 c1OrgCBId = m_mbufData[m_c1CBIdOfs];

    // Create our initialization vector
    THeapBuf mbufIV(16, 16);
    facCIDCrypto().GetRandomBytes(mbufIV, 0, 8);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
        mbufIV.PutCard1(mbufNonce[c4Index], c4Index + 8);

    //
    //  Pull out just the command class bytes to another buffer, so that we can
    //  encrypt that. We have to put a zero before that for the reserved byte.
    //
    const tCIDLib::TCard4 c4OrgBytes(c4CCBytes());
    THeapBuf mbufPlain(c4OrgBytes + 1, c4OrgBytes + 1);

    // Copy out the CC bytes starting at 1, then put in the leading zero reserved byte
    c4CopyOutCCBytes(mbufPlain, 1);
    mbufPlain.PutCard1(0, 0);

    // Now encrypt the plain text stuff
    THeapBuf mbufSecure;
    facZWaveUSB3Sh().Encrypt(mbufPlain, c4OrgBytes + 1, mbufSecure, mbufIV);

    //
    //  Generate the MAC data on the encrypted part of the message. Pass it the security
    //  command that will be in the new message, since that's part of it.
    //
    THeapBuf mbufMAC(8, 8);
    facZWaveUSB3Sh().CalculateMAC
    (
        m_bReqNonce ? SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET : SECURITY_MESSAGE_ENCAPSULATION
        , mbufSecure.pc1Data()
        , c4OrgBytes + 1
        , facZWaveUSB3Sh().c1CtrlId()
        , c1TargetId
        , mbufIV
        , mbufMAC
    );

    // Save the original
    if (m_pzwomOrg)
        *m_pzwomOrg = *this;
    else
        m_pzwomOrg = new TZWOutMsg(*this);

    //
    //  Now build up the new message. We haven't changed anything yet. We keep the
    //  same msg type, function id, and target id. We'll update the 0th (msg length)
    //  byte at the end.
    //
    //  Now we start on byte 4 with the number of CC bytes, then the new encapsulating
    //  CC ids.
    //
    tCIDLib::TCard1 m_c1Count = 4;
    m_mbufData.PutCard1(tCIDLib::TCard1(20 + c4OrgBytes), m_c1Count++);
    m_mbufData.PutCard1(COMMAND_CLASS_SECURITY, m_c1Count++);
    m_mbufData.PutCard1
    (
        m_bReqNonce ? SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET : SECURITY_MESSAGE_ENCAPSULATION
        , m_c1Count++
    );

    // Now the first 8 IV bytes
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
        m_mbufData.PutCard1(mbufIV[c4Index], m_c1Count++);

    // Now the encrypt bytes (including the leading reserved byte)
    for (tCIDLib::TCard1 c4Index = 0; c4Index < c4OrgBytes + 1; c4Index++)
        m_mbufData.PutCard1(mbufSecure[c4Index], m_c1Count++);

    // Put the nonce id into it
    m_mbufData.PutCard1(mbufNonce[0], m_c1Count++);

    // Add the MAC bytes
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
        m_mbufData.PutCard1(mbufMAC[c4Index], m_c1Count++);

    // Put back the trans opts
    if (m_bTransOpts)
    {
        m_c1TransOptsOfs = m_c1Count;
        m_mbufData.PutCard1(m_c1TransOpts, m_c1Count++);
    }

    //
    //  CC msgs always have a callback id, even if it is zero. We saved it above so
    //  put it back, and update the callback id offset.
    //
    m_c1CBIdOfs = m_c1Count;
    m_mbufData.PutCard1(c1OrgCBId, m_c1Count++);

    // Update the 0th byte with the full length now
    m_mbufData.PutCard1(m_c1Count, 0);

    //
    //  We don't need the secure flag anymore, since it's encrypted now. The original
    //  msg pointer we set above will tell anyone who cares that it has been encrypted.
    //
    m_bSecure = kCIDLib::False;

    // Now we can move up to ready to send state
    m_eState = tZWaveUSB3Sh::EOMState_ReadyToSend;
}


//
//  End the last pushed counter. We put the count of bytes between the original start
//  position and the new position.
//
tCIDLib::TVoid TZWOutMsg::EndCounter()
{
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in working state to complete"
    );
    CIDAssert(!m_fcolCounters.bIsEmpty(), L"No out msg counter to end");

    // Get the last stored counter index
    tCIDLib::TCard1 c1At = m_fcolCounters.tPop();

    //
    //  And now let's put the bytes since the start into the original start position. Sub
    //  one because m_c1Count is just past the last character added. If nothing has been
    //  added since then, it would come out zero.
    //
    m_mbufData.PutCard1((m_c1Count - c1At) - 1, c1At);
}


//
//  This is called to make this msg an end point encapsulated msg. It's a lot more
//  convenient for the driver if it can just build msgs normally and then encapsulate
//  them if that is appropriate. Sometimes it can be done generically by higher level
//  code.
//
tCIDLib::TVoid
TZWOutMsg::EndPointEncap(const tCIDLib::TCard1 c1SrcEP, const tCIDLib::TCard1 c1TarEP)
{
    // This has to be a CC msg
    CIDAssert(m_bIsCCMsg, L"Cannot end point encap a non-CC msg");

    // And it cannot already be encapsulated
    CIDAssert
    (
        c1ClassId() != COMMAND_CLASS_MULTI_CHANNEL_V2
        , L"This msg is already end point enscapsulated"
    );

    //
    //  And it has to have been finalized, but not encrypted. If encapsulated, that
    //  has to be done first and then that is encrypted.
    //
    CIDAssert
    (
        (m_eState != tZWaveUSB3Sh::EOMState_Working) && !m_pzwomOrg
        , L"Cannot encap a msg that is in working state or already encrypted"
    )


    // Remember the current byte count
    const tCIDLib::TCard1 c1OrgCnt = m_c1Count;

    //
    //  We have to insert four bytes, the CC class and cmd, and the two end points.
    //  They go in at offset 5 which is where the actual command class is now.
    //
    //  So we need to move everything from offset 5 up four bytes to make room.
    //
    m_mbufData.MakeSpace(5, 4, m_c1Count);

    // And slip the new stuff in
    m_mbufData.PutCard1(COMMAND_CLASS_MULTI_CHANNEL_V2, 5);
    m_mbufData.PutCard1(MULTI_CHANNEL_CMD_ENCAP_V2, 6);
    m_mbufData.PutCard1(c1SrcEP, 7);
    m_mbufData.PutCard1(c1TarEP, 8);

    // If there is a callback id offset, adjust it up by four
    if (m_c1CBIdOfs)
        m_c1CBIdOfs += 4;

    //
    //  And bump the count appropriately, both our member and the one in the 0th byte
    //  of the msg since it will have been finalized already.
    //
    m_c1Count += 4;
    m_mbufData.PutCard1(m_mbufData[0] + 4, 0);

    // And, we now need to bump the CC payload byte count as well
    m_mbufData.PutCard1(m_mbufData[4] + 4, 4);
}


//
//  Finalize by updating the leading msg len byte and storing some info for later use.
//  At some point we may also want to do some validation.
//
tCIDLib::TVoid
TZWOutMsg::Finalize(const TZWUnitInfo& unitiTar, const tCIDLib::TBoolean bReqNonce)
{
    // Get the command class and ask the unit if that is a secure class for him
    const tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(m_mbufData[5]);
    tCIDLib::TBoolean bSecure = kCIDLib::False;
    if (eClass != tZWaveUSB3Sh::ECClasses::Count)
        bSecure = unitiTar.deviUnit().bSecureClass(eClass);

    // Call the other version
    Finalize(unitiTar.bFreqListener(), bSecure, bReqNonce);
}

tCIDLib::TVoid
TZWOutMsg::Finalize(const   tCIDLib::TBoolean   bFreqListener
                    , const tCIDLib::TBoolean   bSecure
                    , const tCIDLib::TBoolean   bReqNonce)
{
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in working state to finalize"
    );
    CIDAssert(m_fcolCounters.bIsEmpty(), L"Can't finalize out msg when counters are active");

    // We can store the overall msg length
    m_mbufData.PutCard1(m_c1Count, 0);

    // Remember the frequent listener flag
    m_bFreqListener = bFreqListener;

    // If secure set us to wait encrypt state, else ready to send
    m_bSecure = bSecure;
    if (m_bSecure)
    {
        m_eState = tZWaveUSB3Sh::EOMState_WaitEncrypt;
        m_bReqNonce = bReqNonce;
    }
     else
    {
        m_eState = tZWaveUSB3Sh::EOMState_ReadyToSend;
        m_bReqNonce = kCIDLib::False;
    }
}


//
//  A helper to build up the command no-op ping message.
//
tCIDLib::TVoid TZWOutMsg::MakePing( const   tCIDLib::TCard1         c1TarId
                                    , const tZWaveUSB3Sh::EMsgPrios ePrio)
{
    Reset
    (
        c1TarId
        , COMMAND_CLASS_NO_OPERATION
        , 0
        , 0
        , 0
        , 2
        , ePrio
    );

    AppendTransOpts(kCIDLib::True);
    AppendCallback();
    Finalize(kCIDLib::False, kCIDLib::False);
}


// Provide a const pointer to some point in our existing data
const tCIDLib::TCard1* TZWOutMsg::pc1DataAt(const tCIDLib::TCard4 c4At) const
{
    return m_mbufData.pc1DataAt(c4At);
}


//
//  Provide access to the command class bytes. Doesn't have to be finalized yet, since
//  it may be needed to generate a MAC on a msg being built. This will be pointed at the
//  the command class byte, skipping over the CC bytes count byte.
//
const tCIDLib::TCard1* TZWOutMsg::pc1CCBytes() const
{
    return m_mbufData.pc1DataAt(5);
}


// Return our orignal, unencrypted, content if we have any
const TZWOutMsg* TZWOutMsg::pzwomOrg() const
{
    return m_pzwomOrg;
}



//
//  These methods reset this message for reuse, for the various msg format scenarios
//  we have to support. This is never used for CC msgs, so we assume normal priority.
//
tCIDLib::TVoid TZWOutMsg::Reset(const   tCIDLib::TCard1         c1MsgId
                                , const tZWaveUSB3Sh::EMsgPrios ePriority
                                , const tZWaveUSB3Sh::EMsgTypes eType)
{
    m_bFreqListener     = kCIDLib::False;
    m_bIsCCMsg          = kCIDLib::False;
    m_bSecure           = kCIDLib::False;
    m_bTransOpts        = kCIDLib::False;
    m_c1CBIdOfs         = 0;
    m_c1Count           = 0;
    m_c1CCReplyClass    = 0;
    m_c1CCReplyCmd      = 0;
    m_c1MsgId           = c1MsgId;
    m_c1ReplyMsgId      = c1MsgId;
    m_c1TarId           = 0;
    m_c1TransOpts       = 0;
    m_c1TransOptsOfs    = 0;
    m_c4AckId           = c4NextAckId();
    m_c4SendCost        = 0;
    m_c4SendCount       = 0;
    m_ePriority         = ePriority;
    m_eState            = tZWaveUSB3Sh::EOMState_Working;
    m_eType             = eType;

    m_fcolCounters.RemoveAll();

    delete m_pzwomOrg;
    m_pzwomOrg       = nullptr;

    // Place holder in the 0th byte till we comlete
    m_mbufData.PutCard1(0, m_c1Count++);

    if (eType == tZWaveUSB3Sh::EMsgTypes::Request)
        m_mbufData.PutCard1(REQUEST, m_c1Count++);
    else
        m_mbufData.PutCard1(RESPONSE, m_c1Count++);

    m_mbufData.PutCard1(c1MsgId, m_c1Count++);
}

//
//  Prep a message with just the function id and target unit. This is never used for
//  CC msgs, so we assume normal priority.
//
tCIDLib::TVoid TZWOutMsg::Reset(const   tCIDLib::TCard1         c1TarId
                                , const tCIDLib::TCard1         c1MsgId
                                , const tZWaveUSB3Sh::EMsgPrios ePriority
                                , const tZWaveUSB3Sh::EMsgTypes eType)
{
    m_bFreqListener     = kCIDLib::False;
    m_bIsCCMsg          = kCIDLib::False;
    m_bSecure           = kCIDLib::False;
    m_bTransOpts        = kCIDLib::False;
    m_c1CBIdOfs         = 0;
    m_c1Count           = 0;
    m_c1CCReplyClass    = 0;
    m_c1CCReplyCmd      = 0;
    m_c1MsgId           = c1MsgId;
    m_c1ReplyMsgId      = c1MsgId;
    m_c1TarId           = c1TarId;
    m_c1TransOpts       = 0;
    m_c1TransOptsOfs    = 0;
    m_c4AckId           = c4NextAckId();
    m_c4SendCost        = 0;
    m_c4SendCount       = 0;
    m_ePriority         = ePriority;
    m_eState            = tZWaveUSB3Sh::EOMState_Working;
    m_eType             = eType;

    m_fcolCounters.RemoveAll();

    delete m_pzwomOrg;
    m_pzwomOrg       = nullptr;

    // Place holder in the 0th byte till we comlete
    m_mbufData.PutCard1(0, m_c1Count++);

    if (eType == tZWaveUSB3Sh::EMsgTypes::Request)
        m_mbufData.PutCard1(REQUEST, m_c1Count++);
    else
        m_mbufData.PutCard1(RESPONSE, m_c1Count++);

    m_mbufData.PutCard1(c1MsgId, m_c1Count++);
    m_mbufData.PutCard1(c1TarId, m_c1Count++);
}

// These always have the 2nd ack flag set
tCIDLib::TVoid TZWOutMsg::Reset(const   tCIDLib::TCard1         c1TarId
                                , const tCIDLib::TCard1         c1ClassId
                                , const tCIDLib::TCard1         c1OutCmdId
                                , const tCIDLib::TCard1         c1ReplyCmdId
                                , const tCIDLib::TCard1         c1CCBytes
                                , const tZWaveUSB3Sh::EMsgPrios ePriority)

{
    m_bFreqListener  = kCIDLib::False;
    m_bIsCCMsg       = kCIDLib::True;
    m_bSecure        = kCIDLib::False;
    m_bTransOpts     = kCIDLib::False;
    m_c1CBIdOfs      = 0;
    m_c1Count        = 0;
    m_c1CCReplyClass = c1ClassId;
    m_c1CCReplyCmd   = c1ReplyCmdId;
    m_c1MsgId        = FUNC_ID_ZW_SEND_DATA;
    m_c1ReplyMsgId   = FUNC_ID_APPLICATION_COMMAND_HANDLER;
    m_c1TarId        = c1TarId;
    m_c1TransOpts    = 0;
    m_c1TransOptsOfs = 0;
    m_c4AckId        = c4NextAckId();
    m_c4SendCost     = 0;
    m_c4SendCount    = 0;
    m_ePriority      = ePriority;
    m_eState         = tZWaveUSB3Sh::EOMState_Working;
    m_eType          = tZWaveUSB3Sh::EMsgTypes::Request;

    m_fcolCounters.RemoveAll();

    delete m_pzwomOrg;
    m_pzwomOrg       = nullptr;

    // Place holder in the 0th byte till we comlete
    m_mbufData.PutCard1(0, m_c1Count++);

    // These are always requests
    m_mbufData.PutCard1(REQUEST, m_c1Count++);

    m_mbufData.PutCard1(FUNC_ID_ZW_SEND_DATA, m_c1Count++);
    m_mbufData.PutCard1(c1TarId, m_c1Count++);

    // If the CC bytes was provided store it, else start a counter
    if (c1CCBytes)
        m_mbufData.PutCard1(c1CCBytes, m_c1Count++);
    else
        StartCounter();

    // Put in the outgoing class and cmd id
    m_mbufData.PutCard1(c1ClassId, m_c1Count++);
    m_mbufData.PutCard1(c1OutCmdId, m_c1Count++);
}

//
//  For when we don't get back the same command class in the reply as what we sent.
//
tCIDLib::TVoid TZWOutMsg::Reset(const   tCIDLib::TCard1         c1TarId
                                , const tCIDLib::TCard1         c1ClassId
                                , const tCIDLib::TCard1         c1OutCmdId
                                , const tCIDLib::TCard1         c1ReplyClassId
                                , const tCIDLib::TCard1         c1ReplyCmdId
                                , const tCIDLib::TCard1         c1CCBytes
                                , const tZWaveUSB3Sh::EMsgPrios ePriority)

{
    m_bFreqListener  = kCIDLib::False;
    m_bIsCCMsg       = kCIDLib::True;
    m_bSecure        = kCIDLib::False;
    m_bTransOpts     = kCIDLib::False;
    m_c1CBIdOfs      = 0;
    m_c1Count        = 0;
    m_c1CCReplyClass = c1ReplyClassId;
    m_c1CCReplyCmd   = c1ReplyCmdId;
    m_c1MsgId        = FUNC_ID_ZW_SEND_DATA;
    m_c1ReplyMsgId   = FUNC_ID_APPLICATION_COMMAND_HANDLER;
    m_c1TarId        = c1TarId;
    m_c1TransOpts    = 0;
    m_c1TransOptsOfs = 0;
    m_c4AckId        = c4NextAckId();
    m_c4SendCost     = 0;
    m_c4SendCount    = 0;
    m_ePriority      = ePriority;
    m_eState         = tZWaveUSB3Sh::EOMState_Working;
    m_eType          = tZWaveUSB3Sh::EMsgTypes::Request;

    m_fcolCounters.RemoveAll();

    delete m_pzwomOrg;
    m_pzwomOrg       = nullptr;


    // Place holder in the 0th byte till we comlete
    m_mbufData.PutCard1(0, m_c1Count++);

    // These are always requests
    m_mbufData.PutCard1(REQUEST, m_c1Count++);

    m_mbufData.PutCard1(FUNC_ID_ZW_SEND_DATA, m_c1Count++);
    m_mbufData.PutCard1(c1TarId, m_c1Count++);

    // If the CC bytes was provided store it, else start a counter
    if (c1CCBytes)
        m_mbufData.PutCard1(c1CCBytes, m_c1Count++);
    else
        StartCounter();

    // Put in the outgoing class and cmd id
    m_mbufData.PutCard1(c1ClassId, m_c1Count++);
    m_mbufData.PutCard1(c1OutCmdId, m_c1Count++);
}


//
//  Start a byte counter, used when building up msgs to count how many bytes between two
//  points.
//
tCIDLib::TVoid TZWOutMsg::StartCounter()
{
    CIDAssert
    (
        m_eState == tZWaveUSB3Sh::EOMState_Working
        , L"Msg must be in working state to complete"
    );
    CIDAssert(!m_fcolCounters.bIsFull(), L"OutMsg counter stack is full");
    m_fcolCounters.Push(m_c1Count++);
}


// Updates this msg to the next callback id available. Needed for a resend on timeout
tCIDLib::TVoid TZWOutMsg::UpdateCallback()
{
    if (m_c1CBIdOfs)
        m_mbufData.PutCard1(c1NextCBId(), m_c1CBIdOfs);
}



// ---------------------------------------------------------------------------
//  TZWOutMsg: Private, static data members
// ---------------------------------------------------------------------------
tCIDLib::TCard1 TZWOutMsg::c1NextCBId()
{
    // Lock and get our value, adjusting the next id counter as required
    tCIDLib::TCard1 c1Ret;
    {
        TCritSecLocker crslSync(&s_crsCBId);
        c1Ret = ++s_c1NextCBId;

        // If 0 or 0xFF, move to 1
        if (!s_c1NextCBId || (s_c1NextCBId == 0xFF))
            s_c1NextCBId = 1;
    }
    return c1Ret;
}

tCIDLib::TCard4 TZWOutMsg::c4NextAckId()
{
    tCIDLib::TCard4 c4Ret;
    {
        TCritSecLocker crslSync(&s_crsCBId);
        c4Ret = ++s_c4NextAckId;

        // It's never going to wrap, but do the right thing
        if (!s_c4NextAckId)
            s_c4NextAckId = 1;
    }
    return c4Ret;
}


// ---------------------------------------------------------------------------
//  TZWOutMsg: Private, static data members
// ---------------------------------------------------------------------------
tCIDLib::TCard1     TZWOutMsg::s_c1NextCBId = 1;
tCIDLib::TCard4     TZWOutMsg::s_c4NextAckId = 1;
TCriticalSection    TZWOutMsg::s_crsCBId;
