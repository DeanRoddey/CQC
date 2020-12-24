//
// FILE NAME: CQCMQTT_Msg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/21/2018
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
//  This is the implementation of the MQTT in and output message classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMQTT_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTTopicInfo, TObject)
RTTIDecls(TMQTTMsg, TObject)
RTTIDecls(TMQTTInMsg, TMQTTMsg)
RTTIDecls(TMQTTOutMsg, TMQTTMsg)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCMQTT_Msg
{
    namespace
    {
        constexpr tCIDLib::TCard2   c2FmtTopicReq = 1;

        //
        //  A text converter for all of the UTF8 stuff we need to do. These are stateless
        //  and so we can use them from however many threads.
        //
        static TUTF8Converter   tcvtUTF8;
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TMQTTTopicInfo
//  PREFIX: mqti
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTTopicInfo: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMQTTTopicInfo::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Stream in the format version and make sure its good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCMQTT_Msg::c2FmtTopicReq))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_strTopicPath
                    >> m_eQOS;

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TMQTTTopicInfo::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCMQTT_Msg::c2FmtTopicReq

                    << m_strTopicPath
                    << m_eQOS

                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------

//
//  Based on msg type we return a rough guess as to a likely appropriate size for
//  the buffer. For most of them we can get it right. Others we just give a
//  reasonable, middle of the road guess. If it ends up being expanded, then it
//  will.
//
tCIDLib::TCard4 TMQTTMsg::c4CalcEstSize(const tCQCMQTT::EPacketTypes eType)
{
    tCIDLib::TCard4 c4Ret = 0;
    switch(eType)
    {
        case tCQCMQTT::EPacketTypes::Publish :
        case tCQCMQTT::EPacketTypes::Subscribe :
        case tCQCMQTT::EPacketTypes::Unsubscribe :
            // These can both have arbitrary numbers of topics
            c4Ret = kCIDLib::c4Sz_4K;
            break;

        case tCQCMQTT::EPacketTypes::Connect :
            //
            //  A sepecial case one, with will, login info, client id, but not
            //  as big as a publish or subscribe can be.
            //
            c4Ret = kCIDLib::c4Sz_1K;
            break;

        case tCQCMQTT::EPacketTypes::SubAck :
            // Somewhat special in that it can have X number of result codes
            c4Ret = 256;
            break;

        case tCQCMQTT::EPacketTypes::ConnAck :
        case tCQCMQTT::EPacketTypes::PubAck :
        case tCQCMQTT::EPacketTypes::PubComp :
        case tCQCMQTT::EPacketTypes::PubRec :
        case tCQCMQTT::EPacketTypes::PubRel :
        case tCQCMQTT::EPacketTypes::UnsubAck    :
            //
            //  All of these are fixed header plus possible couple of flag bytes and
            //  and packet id.
            //
            c4Ret = 16;
            break;

        case tCQCMQTT::EPacketTypes::PingReq :
        case tCQCMQTT::EPacketTypes::PingResp :
        case tCQCMQTT::EPacketTypes::Disconnect :
            // The simplest ones
            c4Ret = 8;
            break;

        default :
            CIDAssert2(L"Unknown packet type to size");
            break;
    };
    return c4Ret;
}


//
//  This is a helper to read out the header and remaining length of a msg. Often
//  in order to know how to set up the msg or to allocate appropriate size, you need
//  this info up front. So it can be called separately, and one of the input msg
//  class' parsing methods accepts this info and picks up from there. Another will
//  calls this and then call the other version.
//
//  If anything goes wrong, this guy throw.
//
//  Generally the caller knows something is available and only calls us if so, but
//  it can block, using the end time.
//
tCIDLib::TVoid
TMQTTMsg::ReadHeaderLen(        TCIDDataSrc&            cdsMQTT
                        ,       tCQCMQTT::EPacketTypes& eType
                        ,       tCIDLib::TCard1&        c1Flags
                        ,       tCIDLib::TCard4&        c4RemLen
                        , const tCIDLib::TEncodedTime   enctEnd)
{
    tCIDLib::TCard1 c1First;
    cdsMQTT.c4ReadBytes(&c1First, 1, enctEnd, kCIDLib::True);
    eType = tCQCMQTT::EPacketTypes(c1First >> 4);
    c1Flags = c1First & 0xF;

    // Check the type to make sure it makes sense
    if ((eType < tCQCMQTT::EPacketTypes::FirstGood)
    ||  (eType > tCQCMQTT::EPacketTypes::LastGood))
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadPacketType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(tCIDLib::c4EnumOrd(eType))
        );
    }

    // Pull out the remaining length, which is encoded
    tCIDLib::TCard4 c4Multi = 1;
    tCIDLib::TCard1 c1EncByte;
    c4RemLen = 0;
    do
    {
        cdsMQTT.c4ReadBytes(&c1EncByte, 1, enctEnd, kCIDLib::True);
        c4RemLen += (c1EncByte & 0x7F) * c4Multi;
        c4Multi *= 0x80;
        if (c4Multi > kCQCMQTT::c4Proto_MaxRemLen)
        {
            facCQCMQTT().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCMQTTErrs::errcProto_BadRemLen
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , tCQCMQTT::strXlatEPacketTypes(eType)
            );
        }
    }   while ((c1EncByte & 0x80) != 0);
}


// ---------------------------------------------------------------------------
//  TMQTTMsg: Constructors and destructors
// ---------------------------------------------------------------------------
TMQTTMsg::~TMQTTMsg()
{
}


// ---------------------------------------------------------------------------
//  TMQTTMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Search our topic list for a match. We have one that just returns the search
//  result and another that returns the associated QOS if found.
//
tCIDLib::TBoolean TMQTTMsg::bHasTopicId(const TString& strToFind) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    m_colTopics.bForEach
    (
        [&strToFind, &bRet](const TMQTTTopicInfo& mqtiCur) -> tCIDLib::TBoolean
        {
            if (mqtiCur.strTopicPath().bCompareI(strToFind))
                bRet = kCIDLib::True;

            // Stop if we ge ta match
            return !bRet;
        }
    );
    return bRet;
}

tCIDLib::TBoolean
TMQTTMsg::bHasTopicId(const TString& strToFind, tCQCMQTT::EQOSLevs& eQOS) const
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    m_colTopics.bForEach
    (
        [&strToFind, &bRet, &eQOS](const TMQTTTopicInfo& mqtiCur) -> tCIDLib::TBoolean
        {
            if (mqtiCur.strTopicPath().bCompareI(strToFind))
            {
                eQOS = mqtiCur.eQOS();
                bRet = kCIDLib::True;
            }

            // Stop if we ge ta match
            return !bRet;
        }
    );
    return bRet;
}


// See if we have a generic keyed value
tCIDLib::TBoolean TMQTTMsg::bHasValue(const TString& strToFind, TString& strValue) const
{
    const TKeyValuePair* pkvalFind = m_colValues.pobjFindByKey(strToFind);
    if (!pkvalFind)
        return kCIDLib::False;

    strValue = pkvalFind->strValue();
    return kCIDLib::True;
}


//
//  See if this msg matches the packet type and id passed. The packet id is only
//  considered if this msg type uses one.
//
tCIDLib::TBoolean
TMQTTMsg::bIsThisMsg(const  tCQCMQTT::EPacketTypes  ePacketType
                    , const tCIDLib::TCard2         c2PacketId) const
{
    // Check the obvious first
    if (m_ePacketType != ePacketType)
        return kCIDLib::False;

    // Check the packet id if important
    if (m_bHasPacketId && (m_c2PacketId != c2PacketId))
        return kCIDLib::False;

    return kCIDLib::True;
}


// Access any of the stored result codes
tCIDLib::TCard1 TMQTTMsg::c1CodeAt(const tCIDLib::TCard4 c4At) const
{
    // Check the index and throw our own error here
    if (c4At >= m_fcolCodes.c4ElemCount())
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_NoCodeAt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4At)
            , tCQCMQTT::strXlatEPacketTypes(ePacketType())
        );
    }
    return m_fcolCodes[c4At];
}



//
//  If they set the packet id, check that it is valid for the packet type and
//  flags combination. If it's valid, the 'has packet id' flag would have
//  already been set.
//
tCIDLib::TCard2 TMQTTMsg::c2PacketId(const tCIDLib::TCard2 c2ToSet)
{
    if (!m_bHasPacketId)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcMsg_NotIdType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , tCQCMQTT::strXlatEPacketTypes(m_ePacketType)
        );
    }

    m_c2PacketId = c2ToSet;
    return m_c2PacketId;
}


// Return the number of seconds between now and the start time stamp of this msgs
tCIDLib::TCard4 TMQTTMsg::c4ElapsedSecs() const
{
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    CIDAssert(enctNow > m_enctStart, L"Msg start time underflow");

    return tCIDLib::TCard4((enctNow - m_enctStart) / kCIDLib::enctOneSecond);
}


tCIDLib::TCard4 TMQTTMsg::c4TopicCnt() const
{
    return m_colTopics.c4ElemCount();
}


// Return the QOS associated with the topic at the indicated index
tCQCMQTT::EQOSLevs TMQTTMsg::eTopicQOSAt(const tCIDLib::TCard4 c4At) const
{
    return m_colTopics[c4At].eQOS();
}


tCIDLib::TVoid TMQTTMsg::LogTo(TTextOutStream& strmTar, TTime& tmToUse) const
{
    tmToUse.SetToNow();
    if (m_bIn)
        strmTar << L"[INF:MIN:";
    else
        strmTar << L"[INF:MOT:";

    strmTar << tmToUse << L"] - \n{\n";
    {
        TStreamIndentJan janIndent(&strmTar, 3);
        strmTar << L"Type: " << m_ePacketType
                << L"\nHdr Flags: " << TCardinal(m_c1FixedFlags, tCIDLib::ERadices::Hex)
                << L"\nPacket Flags: " << TCardinal(m_c1PacketFlags, tCIDLib::ERadices::Hex);

        if (m_bHasPacketId)
            strmTar << L"\n   Packet Id: " << m_c2PacketId;

        // If any codes, show those
        const tCIDLib::TCard4 c4CodeCnt = m_fcolCodes.c4ElemCount();
        if (c4CodeCnt)
        {
            strmTar << L"\nCodes: ";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CodeCnt; c4Index++)
                strmTar << m_fcolCodes[c4Index] << L" ";
        }
        strmTar << kCIDLib::NewLn;

        // If any topics, show those
        if (!m_colTopics.bIsEmpty())
        {
            strmTar << L"\nTopics: \n";
            tCQCMQTT::TTopicCursor cursTopics(&m_colTopics);
            for (; cursTopics; ++cursTopics)
            {
                const TMQTTTopicInfo& mqtiCur = *cursTopics;
                strmTar << mqtiCur.strTopicPath() << L"  QOS="
                        << mqtiCur.eQOS() << kCIDLib::NewLn;
            }
        }

        // Log the msg specific info
        strmTar << L"\n{";
        {
            TStreamIndentJan janIndent(&strmTar, 3);
            LogMsgInfo(strmTar);
        }
        strmTar << L"\n}";
    }
    strmTar << L"\n}" << kCIDLib::EndLn;
}


// Return one of our topic paths by index
const TString& TMQTTMsg::strTopicPathAt(const tCIDLib::TCard4 c4At) const
{
    return m_colTopics[c4At].strTopicPath();
}


// ---------------------------------------------------------------------------
//  TMQTTMsg: Hidden constructors and operators
// ---------------------------------------------------------------------------
TMQTTMsg::TMQTTMsg(const tCIDLib::TBoolean bIn) :

    m_bIn(bIn)
    , m_bHasPacketId(kCIDLib::False)
    , m_c1FixedFlags(0)
    , m_c1PacketFlags(0)
    , m_c2PacketId(0)
    , m_colTopics(16)
    , m_colValues
      (
        29, TStringKeyOps(kCIDLib::False), TKeyValuePair::strExtractKey
      )
    , m_ePacketType(tCQCMQTT::EPacketTypes::Count)
    , m_enctStart(0)
    , m_fcolCodes(16UL)
{
}

// ---------------------------------------------------------------------------
//  TMQTTMsg: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Let the derived class store an incoming/outgoing result code
tCIDLib::TVoid TMQTTMsg::AddResultCode(const tCIDLib::TCard1 c1Code)
{
    m_fcolCodes.c4AddElement(c1Code);
}


//
//  The derived classes can add topics to our topic list. This includes the path
//  and a QOS level. The QOS level may be just a bogus value if it's not required
//  for the msg type.
//
tCIDLib::TVoid
TMQTTMsg::AddTopic(const TString& strPath, const tCQCMQTT::EQOSLevs eLevel)
{
    // Don't allow too many
    m_colTopics.CheckIsFull(128, L"MQTT msg topic list");
    m_colTopics.objAdd(TMQTTTopicInfo(strPath, eLevel));
}


// Add a genreic value by key
tCIDLib::TVoid TMQTTMsg::AddValue(const TString& strKey, const TString& strValue)
{
    m_colValues.objAdd(TKeyValuePair(strKey, strValue));
}


// Just a safety need to make sure the packet type is set before parsing/building
tCIDLib::TVoid TMQTTMsg::CheckTypeSet(const tCIDLib::TCard4 c4Line) const
{
    if (m_ePacketType == tCQCMQTT::EPacketTypes::Count)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcMsg_TypeNotSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}


//
//  The derived classes shoould call us and then do any reset of their own. We
//  have one for general reset, which sets it so a bad packet type so we can catch
//  any attempt to send without setting up the msg, or a not read into in msg.
//
//  The other is for the out msg class to set up the initial header stuff before
//  building up the rest of the msg. And for the in msg to set the header stuff
//  that he first parses out. This stuff needs to be set quickly becasue we
//  set up here whether a packet id is needed, and the packet type or flags may
//  influence other stuff during build/parse.
//
tCIDLib::TVoid TMQTTMsg::ResetBase()
{
    ResetBase(tCQCMQTT::EPacketTypes::Count, 0);
}


tCIDLib::TVoid
TMQTTMsg::ResetBase(const   tCQCMQTT::EPacketTypes  eType
                    , const tCIDLib::TCard1         c1FixedFlags)
{
    m_c1FixedFlags  = c1FixedFlags;
    m_ePacketType   = eType;

    m_c1PacketFlags = 0;
    m_c2PacketId    = 0;
    m_enctStart     = TTime::enctNow();

    m_colTopics.RemoveAll();
    m_fcolCodes.RemoveAll();

    switch(m_ePacketType)
    {
        case tCQCMQTT::EPacketTypes::PubAck      :
        case tCQCMQTT::EPacketTypes::PubComp     :
        case tCQCMQTT::EPacketTypes::PubRel      :
        case tCQCMQTT::EPacketTypes::PubRec      :
        case tCQCMQTT::EPacketTypes::Subscribe   :
        case tCQCMQTT::EPacketTypes::SubAck      :
        case tCQCMQTT::EPacketTypes::Unsubscribe :
        case tCQCMQTT::EPacketTypes::UnsubAck    :
            m_bHasPacketId = kCIDLib::True;
            break;

        case tCQCMQTT::EPacketTypes::Publish     :
            // Only true if QOS is > 0
            m_bHasPacketId = ((c1FixedFlags & 0x6) >> 1) != 0;
            break;

        default :
            m_bHasPacketId = kCIDLib::False;
            break;
    };
}


// Let's the derived classes store the packet flags if any
tCIDLib::TVoid TMQTTMsg::SetPacketFlags(const tCIDLib::TCard1 c1ToSet)
{
    m_c1PacketFlags = c1ToSet;
}


//
//  The derived classes call this upon parse or send and it's used to check for
//  timeouts.
//
tCIDLib::TVoid TMQTTMsg::SetTimeStamp()
{
    m_enctStart = TTime::enctNow();
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTInMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTInMsg: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TMQTTInMsg::eCompSize(  const   TMQTTInMsg&     msgFirst
                        , const TMQTTInMsg&     msgSec
                        , const tCIDLib::TCard4)
{
    if (msgFirst.m_mbufPayload.c4Size() < msgSec.m_mbufPayload.c4Size())
        return tCIDLib::ESortComps::FirstLess;
    else if (msgFirst.m_mbufPayload.c4Size() > msgSec.m_mbufPayload.c4Size())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TMQTTInMsg::eCompKeySize(const   tCIDLib::TCard4&   c4Size
                         , const TMQTTInMsg&        msgSec
                         , const tCIDLib::TCard4)
{
    if (c4Size < msgSec.m_mbufPayload.c4Size())
        return tCIDLib::ESortComps::FirstLess;
    else if (c4Size > msgSec.m_mbufPayload.c4Size())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


//
//  Our class has its own string parser, but there are some it can't do because it
//  doesn't know they are there, i.e. the payload of publish msgs. They may contain
//  anything and we don't know it at this level. Only the consumer of the msgs would
//  know it (if even then.)
//
//  So we need to provide the outside world with a parser.
//
//  In theory we know how many bytes, since it's in the data. But we get the actual
//  bytes are in the payload as a sanity check. So the buffer includes the two MQ
//  encoded string length bytes. We will eat that many plus two bytes from the buffer
//  so it has to have at least that many.
//
tCIDLib::TVoid
TMQTTInMsg::ParseMQString(          TString&        strToFill
                            , const TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4Bytes)
{
    // Cannot be less than three no matter what (2 length plus 1 character)
    if (c4Bytes < 3)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_StrBufTooSmall
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    tCIDLib::TCard2 c2Len = mbufData.c2At(0);
    #if defined(CIDLIB_LITTLEENDIAN)
    c2Len = TRawBits::c2SwapBytes(c2Len);
    #endif

    // Make sure the buffer is big enough
    if (c2Len + 2UL > c4Bytes)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadStrBufLen
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2Len + 2)
            , TCardinal(c4Bytes)
        );
    }

    //
    //  And convert the remainder. We have to do it via the raw pointer here
    //  since we need to skip over the two length bytes. If the transcode
    //  doesn't process all of the bytes, that's an error.
    //
    const tCIDLib::TCard4 c4BytesEaten = CQCMQTT_Msg::tcvtUTF8.c4ConvertFrom
    (
        mbufData.pc1DataAt(2), c2Len, strToFill
    );

    if (c4BytesEaten != c2Len)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadStrData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
}


// ---------------------------------------------------------------------------
//  TMQTTInMsg: Constructors and Destructor
// ---------------------------------------------------------------------------
TMQTTInMsg::TMQTTInMsg(const tCIDLib::TCard4 c4Size) :

    TMQTTMsg(kCIDLib::True)
    , m_c4PLBytes(0)
    , m_mbufPayload(c4Size, tCIDLib::MaxVal(kCIDLib::c4Sz_4K, c4Size))
    , m_mbufTmp(128, kCIDLib::c4Sz_4K)
{
    //
    //  Call reset to store bogus info at this point. Pass the initial size we set
    //  the payload to above, so it won't reallocate.
    //
    Reset(c4Size);
}

TMQTTInMsg::~TMQTTInMsg()
{
}


// ---------------------------------------------------------------------------
//  TMQTTInMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Makes sure this is a publish msg. If so, extracts the QOS bits from the fixed
//  header and returns them as our QOS level enum.
//
tCQCMQTT::EQOSLevs TMQTTInMsg::eExtractPubQOS() const
{
    if (ePacketType() != tCQCMQTT::EPacketTypes::Publish)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcMsg_WrongType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , tCQCMQTT::strXlatEPacketTypes(ePacketType())
        );
    }

    tCIDLib::TCard1 c1Bits = c1FixedFlags() & 0x6;
    c1Bits >>= 1;

    CIDAssert(c1Bits < 3, L"Publish flag QOS bits were invalid");

    return tCQCMQTT::EQOSLevs(c1Bits);
}


//
//  This will parse this msg in. We have one that assumes the caller has already
//  read the data. He passes us the header and remaining length plus a stream that
//  contains the remaining bytes. That is the most efficient since the caller can
//  reuse the same buffer/stream over and over.
//
//  We have a convience method that takes a data source, does that up front work
//  using locals, and then calls the first method.
//
tCIDLib::TVoid
TMQTTInMsg::ParseFrom(  const   tCQCMQTT::EPacketTypes  eType
                        , const tCIDLib::TCard1         c1FixedFlags
                        , const tCIDLib::TCard4         c4RemLen
                        ,       TBinMBufInStream&       strmBody)
{
    // Reset ourself with the main header stuff
    ResetBase(eType, c1FixedFlags);

    CIDAssert
    (
        strmBody.eEndianMode() == tCIDLib::EEndianModes::Big
        , L"The temp stream should be set to big endian mode"
    );

    try
    {
        tCIDLib::TCard2 c2PId = 0;
        switch(ePacketType())
        {
            // These are not ones we should see
            case tCQCMQTT::EPacketTypes::Connect :
                ParseConnect(strmBody);
                break;

            case tCQCMQTT::EPacketTypes::Subscribe :
                CheckFixedFlags(2);
                strmBody >> c2PId;
                c2PacketId(c2PId);
                ParseSubscribe(strmBody);
                break;

            case tCQCMQTT::EPacketTypes::Unsubscribe :
                CheckFixedFlags(2);
                strmBody >> c2PId;
                c2PacketId(c2PId);
                ParseUnsubscribe(strmBody);
                break;

            case tCQCMQTT::EPacketTypes::ConnAck :
            {
                CheckFixedFlags(0);
                CheckRemLen(2, c4RemLen);
                tCIDLib::TCard1 c1Code, c1PacketFlags;
                strmBody >> c1PacketFlags >> c1Code;
                AddResultCode(c1Code);
                SetPacketFlags(c1PacketFlags);
                break;
            }

            case tCQCMQTT::EPacketTypes::Publish :
                ParsePublish(strmBody, c4RemLen);
                break;

            case tCQCMQTT::EPacketTypes::SubAck :
                CheckFixedFlags(0);
                strmBody >> c2PId;
                c2PacketId(c2PId);

                // Get the codes out of it
                ParseSubAckCodes(strmBody, c4RemLen);
                break;

            case tCQCMQTT::EPacketTypes::PubAck :
            case tCQCMQTT::EPacketTypes::PubRec :
            case tCQCMQTT::EPacketTypes::PubRel :
            case tCQCMQTT::EPacketTypes::PubComp :
            case tCQCMQTT::EPacketTypes::UnsubAck :
                // These are all fixed flags of zero, and 2 rem bytes with the id
                CheckFixedFlags(0);
                CheckRemLen(2, c4RemLen);
                strmBody >> c2PId;
                c2PacketId(c2PId);
                break;

            case tCQCMQTT::EPacketTypes::Disconnect  :
            case tCQCMQTT::EPacketTypes::PingResp :
            case tCQCMQTT::EPacketTypes::PingReq :
                // These are all just the fixed header with zero flags
                CheckFixedFlags(0);
                break;

            default :
                CIDAssert2(L"Unknown packet type to parse");
                break;
        };

        //
        //  We should have now done all of the remaining bytes. If there was no
        //  remaining length we don't check this since the stream may not even
        //  be set up.
        //
        if (c4RemLen && (strmBody.c4CurPos() != c4RemLen))
        {
            // This cannot be right. At worst they are equal
            facCQCMQTT().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCMQTTErrs::errcMsg_BadParseLen
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c4RemLen)
                , TCardinal(strmBody.c4CurPos())
                , tCQCMQTT::strXlatEPacketTypes(ePacketType())
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    // Set the start stamp to reflect when we got this one
    SetTimeStamp();
}

tCIDLib::TVoid TMQTTInMsg::ParseFrom(TCIDDataSrc& cdsMQTT)
{
    tCIDLib::TCard1             c1Flags;
    tCIDLib::TCard4             c4RemLen;
    tCQCMQTT::EPacketTypes      eType;
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(250);

    try
    {
        // Read the header info first and reset our base class with that
        ReadHeaderLen(cdsMQTT, eType, c1Flags, c4RemLen, enctEnd);
        ResetBase(eType, c1Flags);

        //
        //  And now we can create a buffer and read that data into it, assuming
        //  there is any, which there won't be for the simplest msgs types.
        //
        if (c4RemLen)
        {
            THeapBuf mbufTmp(c4RemLen, c4RemLen);
            cdsMQTT.c4ReadBytes(mbufTmp, c4RemLen, enctEnd, kCIDLib::True);
            TBinMBufInStream strmSrc(&mbufTmp, c4RemLen);
            strmSrc.eEndianMode(tCIDLib::EEndianModes::Big);

            // And call the other version to continue the parsing
            ParseFrom(eType, c1Flags, c4RemLen, strmSrc);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TMQTTInMsg::Reset(const tCIDLib::TCard4 c4Size)
{
    TParent::ResetBase();
    m_c4PLBytes = 0;

    // If we aren't big enough, then reallocate
    if (c4Size > m_mbufPayload.c4Size())
        m_mbufPayload.Reallocate(c4Size, kCIDLib::False);
}




// ---------------------------------------------------------------------------
//  TMQTTInMsg: Protected, inherited methods
// ---------------------------------------------------------------------------

// We log our in msg specific stuff
tCIDLib::TVoid TMQTTInMsg::LogMsgInfo(TTextOutStream& strmTar) const
{
    strmTar << L"\nPL Bytes: " << m_c4PLBytes;

    // Display up to 16 bytes
    strmTar << L"\n   Bytes: ";
    const tCIDLib::TCard4 c4Count = tCIDLib::MinVal(m_c4PLBytes, 16UL);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmTar << TCardinal(m_mbufPayload[c4Index], tCIDLib::ERadices::Hex)
                << kCIDLib::chSpace;
    }
}


// ---------------------------------------------------------------------------
//  TMQTTInMsg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  There are various required forms for the fixed flags, so we provide this
//  helper to avoid replicating this check and throw.
//
tCIDLib::TVoid TMQTTInMsg::CheckFixedFlags(const tCIDLib::TCard1 c1MustBe) const
{
    if (c1FixedFlags() != c1MustBe)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadFixedFlags
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , tCQCMQTT::strXlatEPacketTypes(ePacketType())
            , TCardinal(c1MustBe)
            , TCardinal(c1FixedFlags())
        );
    }
}


//
//  Called to make sure that we have at least the minimum required remaining
//  length, which means most of the msgs can then just safely be parsed the
//  rest of the way with no more checks.
//
tCIDLib::TVoid
TMQTTInMsg::CheckRemLen(const   tCIDLib::TCard4 c4MinReq
                        , const tCIDLib::TCard4 c4Got)
{
    if (c4Got < c4MinReq)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_RemLenTooSmall
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c4MinReq)
            , TCardinal(c4Got)
            , tCQCMQTT::strXlatEPacketTypes(ePacketType())
        );
    }
}


// This is called to parse a connect msg and validate it, starting at the protocol
tCIDLib::TVoid TMQTTInMsg::ParseConnect(TBinInStream& strmSrc)
{
    // First get the protocol name out and verify it
    TString strTmp;
    ParseString(strTmp, strmSrc);
    if (strTmp != kCQCMQTT::strProto_Name)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadProtoName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , strTmp
        );
    }

    // Get the level out and make sure it's the level we support (4)
    tCIDLib::TCard1 c1Level;
    strmSrc >> c1Level;
    if (c1Level != kCQCMQTT::c1Proto_OurVersion)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
            , TCardinal(c1Level)
        );
    }

    // Pull out the packet flags and store them on our parent class
    tCIDLib::TCard1 c1PacketFlags;
    strmSrc >> c1PacketFlags;
    SetPacketFlags(c1PacketFlags);

    // Get a version as our connection flags enum
    const tCQCMQTT::EConnFlags eFlags = tCQCMQTT::EConnFlags(c1PacketFlags);

    // The client id always has to be provided
    ParseString(strTmp, strmSrc);
    AddValue(kCQCMQTT::strValKey_ClientId, strTmp);

    // If enabled, next should be the Will topic and will msg
    if (tCIDLib::bAllBitsOn(eFlags, tCQCMQTT::EConnFlags::WillFlag))
    {
        ParseString(strTmp, strmSrc);
        AddValue(kCQCMQTT::strValKey_WillTopic, strTmp);

        // Get the next two bytes as the payload length and read that in
        tCIDLib::TCard2 c2WillLen;
        strmSrc >> c2WillLen;
        m_c4PLBytes = c2WillLen;
        strmSrc.c4ReadBuffer(m_mbufPayload, m_c4PLBytes);
    }

    // If user name is enabled in the flags, read that in
    if (tCIDLib::bAllBitsOn(eFlags, tCQCMQTT::EConnFlags::UserName))
    {
        ParseString(strTmp, strmSrc);
        AddValue(kCQCMQTT::strValKey_UserName, strTmp);
    }

    // And finally if password is enabled in the flags, read that in
    if (tCIDLib::bAllBitsOn(eFlags, tCQCMQTT::EConnFlags::Password))
    {
        ParseString(strTmp, strmSrc);
        AddValue(kCQCMQTT::strValKey_Password, strTmp);
    }
}


tCIDLib::TVoid
TMQTTInMsg::ParsePublish(TBinInStream& strmSrc, const tCIDLib::TCard4 c4RemLen)
{
    //
    //  Parse out the topic path and add to our parent class. There is no QOS
    //  associated directly with the path in this case (it's in the fixed header
    //  flags), so pass zero. It will be ignored since this is a publish msgs.
    //
    TString strPath;
    ParseString(strPath, strmSrc);
    AddTopic(strPath, tCQCMQTT::EQOSLevs::Q0);

    // If we need a packet id for this one (QOS > 0), then get it
    tCIDLib::TCard2 c2Id;
    if (bHasPacketId())
    {
        strmSrc >> c2Id;
        c2PacketId(c2Id);
    }

    //
    //  And now we should have payload content. So the remaining length minus
    //  the stream position should tell us how many bytes.
    //
    const tCIDLib::TCard4 c4CurPos = strmSrc.c4CurPos();

    //
    //  If it would be negative just return. There's a check in the caller for
    //  this scenario that's done for all msgs.
    //
    if (c4CurPos > c4RemLen)
        return;

    // Else, calc the bytes left and read them, which might be zero
    m_c4PLBytes = c4RemLen - c4CurPos;
    if (m_c4PLBytes)
        strmSrc.c4ReadBuffer(m_mbufPayload, m_c4PLBytes);
}


//
//  Strings are encoded as two bytes of length, plus a UTF8 encoded string of that
//  many bytes.
//
tCIDLib::TVoid TMQTTInMsg::ParseString(TString& strToFill, TBinInStream& strmSrc)
{
    // Get the length, and then read out that many bytes
    tCIDLib::TCard2 c2Len;
    strmSrc >> c2Len;

    // Now read that many bytes into the temp buffer and transcode
    strmSrc.c4ReadBuffer(m_mbufTmp, c2Len);
    CQCMQTT_Msg::tcvtUTF8.c4ConvertFrom(m_mbufTmp, c2Len, strToFill);
}


//
//  This is called to parse a subscription msg. We have to pull out a list of
//  topics and related QOS values, which we add to our parent class' list of
//  topics.
//
tCIDLib::TVoid TMQTTInMsg::ParseSubscribe(TBinInStream& strmSrc)
{
    TString strTopic;
    tCIDLib::TCard1 c1QOS;
    while (!strmSrc.bEndOfStream())
    {
        ParseString(strTopic, strmSrc);
        strmSrc >> c1QOS;

        if (c1QOS > 2)
        {
            facCQCMQTT().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCMQTTErrs::errcProto_BadQOS
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Protocol
                , TCardinal(c1QOS)
            );
        }
        AddTopic(strTopic, tCQCMQTT::EQOSLevs(c1QOS));
    }
}


// Called to parse out the sub-ack codes from an incoming msgs
tCIDLib::TVoid
TMQTTInMsg::ParseSubAckCodes(TBinInStream& strmSrc, const tCIDLib::TCard4 c4RemLen)
{
    //
    //  Subtract the 2 bytes of the packet id from the remaining length, and
    //  that should be the number of codes we got. The remaining length obviously
    //  cannot be less than 3, because that we would mean we got no codes and a
    //  subscribe has to have one topic at least.
    //
    if (c4RemLen < 3)
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcProto_BadRemLen
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }
    const tCIDLib::TCard4 c4CodeCnt(c4RemLen - 2);

    // Read in that many codes and add them to our list of codes
    tCIDLib::TCard1 c1Code;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CodeCnt; c4Index++)
    {
        strmSrc >> c1Code;
        AddResultCode(c1Code);
    }
}


//
//  This is called to parse an unsubscribe msg. We have to pull out a list of
//  topics. These have no QOS so we just set a dummy 0 level on them when we
//  store them on the parent class.
//
tCIDLib::TVoid TMQTTInMsg::ParseUnsubscribe(TBinInStream& strmSrc)
{
    TString strTopic;
    while (!strmSrc.bEndOfStream())
    {
        ParseString(strTopic, strmSrc);
        AddTopic(strTopic, tCQCMQTT::EQOSLevs::Q0);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTOutMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TMQTTOutMsg::eCompSize( const   TMQTTOutMsg&    msgFirst
                        , const TMQTTOutMsg&    msgSec
                        , const tCIDLib::TCard4)
{
    if (msgFirst.c4BufSize() < msgSec.c4BufSize())
        return tCIDLib::ESortComps::FirstLess;
    else if (msgFirst.c4BufSize() > msgSec.c4BufSize())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::ESortComps
TMQTTOutMsg::eCompKeySize(  const   tCIDLib::TCard4&    c4Size
                            , const TMQTTOutMsg&        msgSec
                            , const tCIDLib::TCard4)
{
    if (c4Size < msgSec.c4BufSize())
        return tCIDLib::ESortComps::FirstLess;
    else if (c4Size > msgSec.c4BufSize())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

tCIDLib::TCard4
TMQTTOutMsg::c4EncodeMQTTString(const   tCIDLib::TCh* const pszSrc
                                , const tCIDLib::TCard4     c4Len
                                ,       TMemBuf&            mbufTar)
{
    // If the length is max card, get it ourself
    tCIDLib::TCard4 c4RealLen = c4Len;
    if (c4RealLen == kCIDLib::c4MaxCard)
        c4RealLen = TRawStr::c4StrLen(pszSrc);

    // Convert it into our temp buffer
    tCIDLib::TCard4 c4OutBytes;
    CQCMQTT_Msg::tcvtUTF8.c4ConvertTo(pszSrc, c4RealLen, mbufTar, c4OutBytes);
    return c4OutBytes;
}


tCIDLib::TCard4
TMQTTOutMsg::c4EncodeMQTTString(const   TString&        strSrc
                                ,       TMemBuf&        mbufTar)
{
    // Just call the other string related one to do the work
    return c4EncodeMQTTString(strSrc.pszBuffer(), strSrc.c4Length(), mbufTar);
}



// ---------------------------------------------------------------------------
//  TMQTTOutMsg: Constructors and destructor
// ---------------------------------------------------------------------------
TMQTTOutMsg::TMQTTOutMsg(const tCIDLib::TCard4 c4Size) :

    TMQTTMsg(kCIDLib::False)
    , m_bSent(kCIDLib::False)
    , m_mbufBody()
    , m_mbufTmp(128, kCIDLib::c4Sz_4K, 512)
    , m_strmBody(&m_mbufBody)
{
    m_strmBody.eEndianMode(tCIDLib::EEndianModes::Big);
    Reset(c4Size);
}

TMQTTOutMsg::~TMQTTOutMsg()
{
}


// ---------------------------------------------------------------------------
//  TMQTTOutMsg: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Returns true if this is a msg type that needs a reply. We just return true if
//  we have a packet id. These types of msgs, once sent, need to be kept around
//  until we get a reply with that packet id, or it times out (which might require
//  a retry and whatnot.)
//
tCIDLib::TBoolean TMQTTOutMsg::bNeedsReply() const
{
    return bHasPacketId();
}


//
//  Some convenience methods to build up specifc types of msgs all in one go.
//  The caller just needs to call complete to get it all formattted out in his
//  buffer of choice to transmit.
//
//  Even though some are very simple, just a packet id often, having separate
//  methods makes it harder to do the wrong thing.
//
tCIDLib::TVoid
TMQTTOutMsg::BuildConnectMsg(const  TString&            strClientId
                            , const tCIDLib::TBoolean   bCleanSession
                            , const tCIDLib::TCard2     c2KeepAliveSecs
                            , const TString&            strUserName
                            , const TString&            strPassword)
{
    Reset(tCQCMQTT::EPacketTypes::Connect, 0);

    Append(kCQCMQTT::strProto_Name);
    Append(kCQCMQTT::c1Proto_OurVersion);

    // Set up the flags
    tCQCMQTT::EConnFlags eFlags = tCQCMQTT::EConnFlags::None;
    if (bCleanSession)
        eFlags |= tCQCMQTT::EConnFlags::CleanSession;
    if (!strUserName.bIsEmpty())
        eFlags |= tCQCMQTT::EConnFlags::UserName;
    if (!strPassword.bIsEmpty())
        eFlags |= tCQCMQTT::EConnFlags::Password;
    Append(tCIDLib::TCard1(eFlags));
    Append(c2KeepAliveSecs);

    // Do the payload stuff, which are all provided as parameter
    Append(strClientId);
    if (!strUserName.bIsEmpty())
        Append(strUserName);
    if (!strPassword.bIsEmpty())
        Append(strPassword);

    m_strmBody.Flush();
}


tCIDLib::TVoid TMQTTOutMsg::BuildDisconnectMsg()
{
    Reset(tCQCMQTT::EPacketTypes::Disconnect, 0);

    m_strmBody.Flush();
}


tCIDLib::TVoid TMQTTOutMsg::BuildPingReqMsg()
{
    Reset(tCQCMQTT::EPacketTypes::PingReq, 0);

    m_strmBody.Flush();
}

//
//  There's no per-path QOS for Publish so just pass a default which will get
//  ignored in any subsequent work, based on the packet type when we store it
//  on our parent. We only append the packet id if the packet level QOS is
//  > 0.
//
//  Our Reset(size) will already have been called when we were extracted from
//  the pool (or manually.)
//
tCIDLib::TVoid
TMQTTOutMsg::BuildPublishMsg(const  TString&            strTopicPath
                            , const tCQCMQTT::EQOSLevs  eQOS
                            , const tCIDLib::TBoolean   bRetain
                            , const tCIDLib::TCard2     c2PacketId
                            , const TString&            strPayloadText
                            , const tCIDLib::TBoolean   bDup)
{
    // Set up the flags
    tCIDLib::TCard1 c1Flags = 0;
    if (eQOS > tCQCMQTT::EQOSLevs::Q0)
        c1Flags |= tCIDLib::TCard1(eQOS) << 1;
    if (bRetain)
        c1Flags |= 1;
    if (bDup)
        c1Flags |= 8;

    Reset(tCQCMQTT::EPacketTypes::Publish, c1Flags);

    AppendTopicPath(strTopicPath, tCQCMQTT::EQOSLevs::Q0);
    if (bHasPacketId())
        AppendPacketId(c2PacketId);
    Append(strPayloadText);

    m_strmBody.Flush();
}

tCIDLib::TVoid
TMQTTOutMsg::BuildPublishMsg(const  TString&            strTopicPath
                            , const tCQCMQTT::EQOSLevs  eQOS
                            , const tCIDLib::TBoolean   bRetain
                            , const tCIDLib::TCard2     c2PacketId
                            , const TMemBuf&            mbufPayload
                            , const tCIDLib::TCard4     c4PLBytes
                            , const tCIDLib::TBoolean   bDup)
{
    // Set up the flags
    tCIDLib::TCard1 c1Flags = 0;
    if (eQOS > tCQCMQTT::EQOSLevs::Q0)
        c1Flags |= tCIDLib::TCard1(eQOS) << 1;
    if (bRetain)
        c1Flags |= 1;
    if (bDup)
        c1Flags |= 8;

    Reset(tCQCMQTT::EPacketTypes::Publish, c1Flags);

    AppendTopicPath(strTopicPath, tCQCMQTT::EQOSLevs::Q0);
    if (bHasPacketId())
        AppendPacketId(c2PacketId);
    AppendPayload(mbufPayload, c4PLBytes);

    m_strmBody.Flush();
}


tCIDLib::TVoid TMQTTOutMsg::BuildPubAckMsg(const tCIDLib::TCard2 c2PacketId)
{
    Reset(tCQCMQTT::EPacketTypes::PubAck, 0);
    AppendPacketId(c2PacketId);

    m_strmBody.Flush();
}

tCIDLib::TVoid TMQTTOutMsg::BuildPubCompMsg(const tCIDLib::TCard2 c2PacketId)
{
    Reset(tCQCMQTT::EPacketTypes::PubComp, 0);
    AppendPacketId(c2PacketId);

    m_strmBody.Flush();
}

tCIDLib::TVoid TMQTTOutMsg::BuildPubRecMsg(const tCIDLib::TCard2 c2PacketId)
{
    Reset(tCQCMQTT::EPacketTypes::PubRec, 0);
    AppendPacketId(c2PacketId);

    m_strmBody.Flush();
}

tCIDLib::TVoid TMQTTOutMsg::BuildPubRelMsg(const tCIDLib::TCard2 c2PacketId)
{
    Reset(tCQCMQTT::EPacketTypes::PubRel, 0);
    AppendPacketId(c2PacketId);

    m_strmBody.Flush();
}


tCIDLib::TVoid
TMQTTOutMsg::BuildSubscribeMsg( const   tCQCMQTT::TTopicCol&    colTopics
                                , const tCIDLib::TCard2         c2PacketId)
{
    Reset(tCQCMQTT::EPacketTypes::Subscribe, 2);

    // Can't have empty subscription list
    if (colTopics.bIsEmpty())
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcMsg_NoSubTopics
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }

    AppendPacketId(c2PacketId);

    //
    //  Append the path and QOS for each topic and add them to our parent as
    //  the paths and QOS values for this subscription. Note that AppendTopicPath
    //  will add it to our parent automatically.
    //
    colTopics.bForEach
    (
        [this](const TMQTTTopicInfo& mqtiCur)
        {
            AppendTopicPath(mqtiCur.strTopicPath(), mqtiCur.eQOS());
            Append(tCIDLib::TCard1(mqtiCur.eQOS()));
            return kCIDLib::True;
        }
    );

    m_strmBody.Flush();
}


//
//  This is for the sake of pools used by the MQTT driver. It calls us to get us to
//  reset ourself and possibly update our buffer size.
//
tCIDLib::TVoid TMQTTOutMsg::Reset(const tCIDLib::TCard4 c4Size)
{
    TParent::ResetBase();

    m_bSent = kCIDLib::False;
    m_strmBody.Reset();

    // If we aren't big enough, then reallocate
    if (c4Size > m_mbufBody.c4Size())
        m_mbufBody.Reallocate(c4Size, kCIDLib::False);
}


//
//  This is called when the client who owns this msg wants to send it. We try the
//  send and, if it doesn't fail, we set the sent flag. Exceptions go back to the
//  caller.
//
tCIDLib::TVoid TMQTTOutMsg::SendOn(TCIDDataSrc& cdsMQTT)
{
    // Check that the type has been set (a msg has been built into us)
    CheckTypeSet(CID_LINE);

    // The packet and flags have be combined to a byte and sent
    tCIDLib::TCard1 c1First = (tCIDLib::TCard1(ePacketType()) << 4) | c1FixedFlags();
    cdsMQTT.WriteRawBytes(&c1First, 1);

    //
    //  The stored data is the remaining length, which has to be encoded, not just
    //  directly stored.
    //
    tCIDLib::TCard1 c1Next;
    tCIDLib::TCard4 c4Remainder = m_strmBody.c4CurSize();
    do
    {
        c1Next = tCIDLib::TCard1(c4Remainder % 0x80);

        c4Remainder /= 0x80;
        if (c4Remainder != 0)
            c1Next |= 0x80;

        cdsMQTT.WriteRawBytes(&c1Next, 1);

    }   while (c4Remainder != 0);

    // Now we can write out the accumulated body content
    if (m_strmBody.c4CurSize())
        cdsMQTT.WriteBytes(m_mbufBody, m_strmBody.c4CurSize());

    // Flush the data source now to make sure it's all sent
    cdsMQTT.FlushOut(TTime::enctNowPlusSecs(3));

    // And mark it sent
    m_bSent = kCIDLib::True;

    // Set the start stamp to reflect when sent this one
    SetTimeStamp();
}


// ---------------------------------------------------------------------------
//  TMQTTOutMsg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TMQTTOutMsg::LogMsgInfo(TTextOutStream& strmTar) const
{
    strmTar << L"\nPL Bytes: " << m_strmBody.c4CurSize();

    // Display up to 16 bytes
    const TMemBuf& mbufPL = m_strmBody.mbufData();

    strmTar << L"\n   Bytes: ";
    const tCIDLib::TCard4 c4Count = tCIDLib::MinVal(m_strmBody.c4CurSize(), 32UL);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strmTar << TCardinal(mbufPL[c4Index], tCIDLib::ERadices::Hex)
                << kCIDLib::chSpace;
    }

    if (m_strmBody.c4CurSize() > 32)
        strmTar << L"...";
}


// ---------------------------------------------------------------------------
//  TMQTTOutMsg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  These are helpers to do common data appending operations. Some are trivial but
//  it's worth it to be consistent and allow us to hook any of these we need to
//  later. Some are specialized and used for adding stuff that we also add separately
//  to the msg for housekeeping purposes.
//
tCIDLib::TVoid TMQTTOutMsg::Append(const tCIDLib::TCard1 c1Value)
{
    m_strmBody << c1Value;
}


tCIDLib::TVoid TMQTTOutMsg::Append(const tCIDLib::TCard2 c2Value)
{
    m_strmBody << c2Value;
}


tCIDLib::TVoid
TMQTTOutMsg::Append(const   tCIDLib::TCh* const pszVal
                    , const tCIDLib::TCard4     c4Len)
{
    // If the length is max card, get it ourself
    tCIDLib::TCard4 c4RealLen = c4Len;
    if (c4RealLen == kCIDLib::c4MaxCard)
        c4RealLen = TRawStr::c4StrLen(pszVal);

    // Convert it into our temp buffer
    tCIDLib::TCard4 c4OutBytes = c4EncodeMQTTString(pszVal, c4RealLen, m_mbufTmp);

    // Now output the length as a two byte value and then write the string bytes
    m_strmBody << tCIDLib::TCard2(c4RealLen);
    m_strmBody.c4WriteBuffer(m_mbufTmp, c4OutBytes);
}


tCIDLib::TVoid TMQTTOutMsg::Append(const TString& strVal)
{
    // Just call the other string related one to do the work
    Append(strVal.pszBuffer(), strVal.c4Length());
}


//
//  Append the packet id to our bondy content, and store it on our our parent.
//  This should only be called when the msg should have a packet id. The base
//  class will throw an error if not. He will know by this point whether it is
//  legal or not.
//
tCIDLib::TVoid TMQTTOutMsg::AppendPacketId(const tCIDLib::TCard2 c2Value)
{
    m_strmBody << c2Value;
    c2PacketId(c2Value);
}


//
//  Append payload content. This is something that we don't need to keep any
//  real info about since it is unparsed data. So we just dump it straight to
//  the body content.
//
tCIDLib::TVoid
TMQTTOutMsg::AppendPayload(const TMemBuf& mbufPL, const tCIDLib::TCard4 c4Bytes)
{
    // Store the bytes and store the size on our parent
    m_strmBody.c4WriteBuffer(mbufPL, c4Bytes);
}


// Append the code to our body content and add it to the code list on our parent
tCIDLib::TVoid TMQTTOutMsg::AppendResultCode(const tCIDLib::TCard1 c1Value)
{
    m_strmBody << c1Value;
    AddResultCode(c1Value);
}

//
//  The path may or may not have an associated QOS, depending on the msg type. They
//  can just pass zero level if it's not used. The packet type will tell us later
//  whether it's needed or not, but we have to have one to store on our parent
//  class.
//
tCIDLib::TVoid
TMQTTOutMsg::AppendTopicPath(const TString& strPath, const tCQCMQTT::EQOSLevs eQOS)
{
    // Just call the other string related one to do the work
    Append(strPath.pszBuffer(), strPath.c4Length());

    // And add to the list of topics in our parent calss
    AddTopic(strPath, eQOS);
}


//
//  When we get a build call, the header has already been set. This is called to
//  make sure that the caller didn't set one header packet type and call another
//  build method.
//
tCIDLib::TVoid TMQTTOutMsg::CheckBuildType(const tCQCMQTT::EPacketTypes eCheck)
{
    if (eCheck != ePacketType())
    {
        facCQCMQTT().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCMQTTErrs::errcMsg_BadBuildType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
            , tCQCMQTT::strXlatEPacketTypes(ePacketType())
            , tCQCMQTT::strXlatEPacketTypes(eCheck)
        );
    }
}


//
//  A private reset for the msg builders to call. We don't need to reset the buffer
//  size, but we need to reset our base class and reset the stream.
//
tCIDLib::TVoid
TMQTTOutMsg::Reset(const tCQCMQTT::EPacketTypes eType, const tCIDLib::TCard1 c1Flags)
{
    TParent::ResetBase(eType, c1Flags);

    m_bSent = kCIDLib::False;
    m_strmBody.Reset();
}


