//
// FILE NAME: CQCMQTT_Msg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/11/2018
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
//  This is the header for the message related classes. We have different classes
//  for incoming and outgoing messages, since we have substantially different needs
//  for them. One parses and one builds. But they do share a common base class since
//  they have a good bit of stuff in common.
//
//  Neither in nor out message classes provide any copy/assignment stuff. They are
//  either used as temps locally, or a pool of them is used to load and queue up for
//  separate processing. So there's no real need to ever copy them around.
//
//  We also define a very simple class that is a topic path and requested QOS value.
//  This something that will be used in various places and is also used in the base
//  class to allow the in/out derivates to store topic info.
//
//  Note that we have to keep some information separately stored, even for output
//  msgs where we are building up a buffer to send. For in msgs we have to parse out
//  from a buffer and store the broken out info to examine. But, even for out msgs,
//  after it's sent, we have to deal with responses to that msg and possibly retries,
//  and that will involve looking at the data.
//
//  So in the base class we provide storage for the packet type, packet id, the fixed
//  header flags, a set of packet specific flags, a list of topics and associated QOS,
//  and a list of more general values (see the next section), and storage for one or more
//  result codes. That's almost all of it.
//
//  General Msg Data
//
//  To avoid a lot of fields that are only used sometimes, the base msg class has
//  a key/value pair list that can be used to store arbitrary values by key. As
//  mentioned above, the important stuff is stored as typed data, but some is just
//  find stored generically like this.
//
//  It's used for things like username/password, client id, and stuff like that,
//  which are textual and only used in single msgs, and so not worth storing
//  separately. We provide defined keys in the IDL file.
//
//  Payload Data
//
//  The only other storage required is that input msgs have a 'payload' buffer for
//  any unparsed data, such as the data content of publish msgs, which we don't know
//  the actual format of when we receive the msg. And the will msg of a subscribe
//  msg which is just raw data. It will later be interpreted based on user config
//  or built into knowledge that says this topic provides this type of data.
//
//  Out msgs could have the same payload data when going the other way, but it's just
//  dumped into the output msg buffer since it's unknown format data anyway so we have
//  no reason to keep it around separately for out msgs.
//
//  Message Initialization
//
//  MQTT msgs are messy in that we don't know how much space is needed for incoming
//  until they are partly parsed, and for outs we wouldn't know until the msg is
//  built. And some functionality is enabled/disabled according to the packet type
//  and fixed header flags.
//
//  For input msgs we don't know anything, so we always just default construct those.
//  The reset does take packet type and fixed flags, plus another that doesn't take
//  anything. The former is for parsing, where the caller parses out the header and
//  remaining length, then reads the remainder and calls us here to parse the rest.
//  That is the most efficient way to do it. For in msgs, a pool would not sort them,
//  and would just treat them all the same.
//
//  For output msgs, the init info also includes a size, which can either be indicated
//  explicitly by the code creating the msg but we also provide a static method to
//  estimate a likely reasonable size based on packet type. The size represents the
//  bytes needed to flatten out the post-fixed header content.
//
//  Packet Id
//
//  We need to know what msgs need a packet id. As mentioned above, to create an in
//  our out msg (or to reset one for reuse) the caller has to provide the packet
//  type and flags. That is enough to figure out if a packet id is required, and so
//  m_bHasPacketId is set at that point and can be used during later parsing or
//  building. If a call is made to set the packet id and that flag is false, an
//  error is thrown.
//
// CAVEATS/GOTCHAS:
//
//  1)  For the most basic out msg types, just the ctor/reset is enough to fill in
//      the info needs (type and flags) and no further building is required.
//
//  2)  Always call Reset before reusing a msg. If using a pool the pool should do
//      this.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTTopicInfo
//  PREFIX: mqti
// ---------------------------------------------------------------------------
class CQCMQTTEXPORT TMQTTTopicInfo : public TObject, public MStreamable
{
    public :
        TMQTTTopicInfo() :

            m_eQOS(tCQCMQTT::EQOSLevs::Q0)
        {
        }

        TMQTTTopicInfo(  const  TString&            strTopicPath
                        , const tCQCMQTT::EQOSLevs  eQOS) :

            m_eQOS(eQOS)
            , m_strTopicPath(strTopicPath)
        {
        }

        TMQTTTopicInfo(const TMQTTTopicInfo&) = default;
        TMQTTTopicInfo(TMQTTTopicInfo&&) = default;

        ~TMQTTTopicInfo() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTTopicInfo& operator=(const TMQTTTopicInfo&) = default;
        TMQTTTopicInfo& operator=(TMQTTTopicInfo&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCMQTT::EQOSLevs eQOS() const
        {
            return m_eQOS;
        }

        const TString& strTopicPath() const
        {
            return m_strTopicPath;
        }

        tCIDLib::TVoid Set( const   TString&            strTopicPath
                            , const tCQCMQTT::EQOSLevs  eQOS)
        {
            m_eQOS = eQOS;
            m_strTopicPath = strTopicPath;
        }


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eQOS
        //      The QOS for this topic. Could be requested or granted.
        //
        //  m_strTopicPath
        //      The path for this topic.
        // -------------------------------------------------------------------
        tCQCMQTT::EQOSLevs  m_eQOS;
        TString             m_strTopicPath;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTTopicInfo, TObject)
};


// Slip in a type into the facility types namespace
namespace tCQCMQTT
{
    using TTopicCol     = TCollection<TMQTTTopicInfo>;
    using TTopicList    = TVector<TMQTTTopicInfo>;
    using TTopicCursor  = TTopicList::TCursor;
}



// ---------------------------------------------------------------------------
//   CLASS: TMQTTMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------
class CQCMQTTEXPORT TMQTTMsg : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4CalcEstSize
        (
            const   tCQCMQTT::EPacketTypes  eType
        );

        static tCIDLib::TVoid ReadHeaderLen
        (
                    TCIDDataSrc&            cdsMQTT
            ,       tCQCMQTT::EPacketTypes& eType
            ,       tCIDLib::TCard1&        c1Flags
            ,       tCIDLib::TCard4&        c4RemLen
            , const tCIDLib::TEncodedTime   enctEnd
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTMsg() = delete;

        TMQTTMsg(const TMQTTMsg&) = default;
        TMQTTMsg(TMQTTMsg&&) = default;

        virtual ~TMQTTMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTMsg& operator=(const TMQTTMsg& msgSrc) = default;
        TMQTTMsg& operator=(TMQTTMsg&& msgSrc) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasPacketId() const
        {
            return m_bHasPacketId;
        }

        tCIDLib::TBoolean bHasTopicId
        (
            const   TString&                strToFind
        )   const;

        tCIDLib::TBoolean bHasTopicId
        (
            const   TString&                strToFind
            ,       tCQCMQTT::EQOSLevs&     eQOS
        )   const;

        tCIDLib::TBoolean bHasValue
        (
            const   TString&                strToFind
            ,       TString&                strValue
        )   const;

        tCIDLib::TBoolean bIsInMsg() const
        {
            return m_bIn;
        }

        // Packet id is ony checked if this msg has one
        tCIDLib::TBoolean bIsThisMsg
        (
            const   tCQCMQTT::EPacketTypes  ePacketType
            , const tCIDLib::TCard2         c2PacketId
        )   const;

        tCIDLib::TCard1 c1CodeAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TCard1 c1FixedFlags() const
        {
            return m_c1FixedFlags;
        }

        tCIDLib::TCard1 c1FixedFlags(const tCIDLib::TCard1 c1ToSet)
        {
            m_c1FixedFlags = c1ToSet;
            return m_c1FixedFlags;
        }

        tCIDLib::TCard2 c2PacketId() const
        {
            return m_c2PacketId;
        }

        tCIDLib::TCard2 c2PacketId
        (
            const   tCIDLib::TCard2         c2ToSet
        );

        tCIDLib::TCard4 c4CodeCount() const
        {
            return m_fcolCodes.c4ElemCount();
        }

        tCIDLib::TCard4 c4IncRetryCnt()
        {
            return ++m_c4RetryCnt;
        }

        tCIDLib::TCard4 c4ElapsedSecs() const;

        tCIDLib::TCard4 c4TopicCnt() const;

        tCQCMQTT::EPacketTypes ePacketType() const
        {
            return m_ePacketType;
        }

        tCQCMQTT::EQOSLevs eTopicQOSAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TVoid LogTo
        (
                    TTextOutStream&         strmTar
            ,       TTime&                  tmToUse
        )   const;

        const TString& strTopicPathAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        explicit TMQTTMsg
        (
            const   tCIDLib::TBoolean       bIn
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid LogMsgInfo
        (
                    TTextOutStream&         strmTar
        )   const = 0;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddResultCode
        (
            const   tCIDLib::TCard1         c1Code
        );

        tCIDLib::TVoid AddTopic
        (
            const   TString&                strPath
            , const tCQCMQTT::EQOSLevs      eLevel
        );

        tCIDLib::TVoid AddValue
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        tCIDLib::TVoid CheckTypeSet
        (
            const   tCIDLib::TCard4         c4Line
        )   const;

        tCIDLib::TVoid ResetBase();

        tCIDLib::TVoid ResetBase
        (
            const   tCQCMQTT::EPacketTypes  eType
            , const tCIDLib::TCard1         c1FixedFlags
        );

        tCIDLib::TVoid SetPacketFlags
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TVoid SetTimeStamp();


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bHasPacketId
        //      When the packet type is set, we set this based on the type. It may
        //      be undone later in the case of the publish msg, since it is only
        //      provided when QOS is > 0.
        //
        //  m_bIn
        //      The derived class sets this to allow for quick testing of the type
        //      of msg via this base interface.
        //
        //  m_c1FixedFlags
        //      The fixed flags that we put into the msg, which we may need for
        //      the full processing of the message.
        //
        //  m_c2PacketId
        //      The packet id for this packet, if any.
        //
        //  m_c4RetryCnt
        //      This can be used by implementations to keep track of retries on
        //      msgs.
        //
        //  m_colTopics
        //      If any topics are involved, they are stored here. If only a
        //      single one, like a publish, it's just the 0th one. Depending on the
        //      type of msg, the QOS values in each entry may not be used.
        //
        //  m_colValues
        //      Generic storage for less common values in a key/value format.
        //
        //  m_enctStart
        //      This is set to now by the derived class after a successful parse
        //      for in msgs, or a successful send for out msgs.
        //
        //  m_ePacketType
        //      The type of packet this msg represents.
        //
        //  m_fcolCodes
        //      A msg can involve one or more result codes, which are put here. If
        //      only one it will be the 0th one.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bHasPacketId;
        tCIDLib::TBoolean       m_bIn;
        tCIDLib::TCard1         m_c1FixedFlags;
        tCIDLib::TCard1         m_c1PacketFlags;
        tCIDLib::TCard2         m_c2PacketId;
        tCIDLib::TCard4         m_c4RetryCnt;
        tCQCMQTT::TTopicList    m_colTopics;
        tCIDLib::TKVHashSet     m_colValues;
        tCIDLib::TEncodedTime   m_enctStart;
        tCQCMQTT::EPacketTypes  m_ePacketType;
        tCIDLib::TByteList      m_fcolCodes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTMsg, TObject)
};




// ---------------------------------------------------------------------------
//   CLASS: TMQTTInMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------
class CQCMQTTEXPORT TMQTTInMsg : public TMQTTMsg
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eCompSize
        (
            const   TMQTTInMsg&             msgFirst
            , const TMQTTInMsg&             msgSec
            , const tCIDLib::TCard4
        );

        static tCIDLib::ESortComps eCompKeySize
        (
            const   tCIDLib::TCard4&        c4Size
            , const TMQTTInMsg&             msgSec
            , const tCIDLib::TCard4
        );

        static tCIDLib::TVoid ParseMQString
        (
                    TString&                strToFill
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTInMsg() = delete;

        TMQTTInMsg
        (
            const   tCIDLib::TCard4         c4Size
        );

        TMQTTInMsg(const TMQTTInMsg&) = default;
        TMQTTInMsg(TMQTTInMsg&&) = default;

        ~TMQTTInMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTInMsg& operator=(const TMQTTInMsg& msgSrc) = default;
        TMQTTInMsg& operator=(TMQTTInMsg&& msgSrc) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4BufSize() const
        {
            return m_mbufPayload.c4Size();
        }

        tCIDLib::TCard4 c4PLBytes() const
        {
            return m_c4PLBytes;
        }

        tCQCMQTT::EQOSLevs eExtractPubQOS() const;

        const TMemBuf& mbufPayload() const
        {
            return m_mbufPayload;
        }

        tCIDLib::TVoid ParseFrom
        (
            const   tCQCMQTT::EPacketTypes  eType
            , const tCIDLib::TCard1         c1FixedFlags
            , const tCIDLib::TCard4         c4RemLen
            ,       TBinMBufInStream&       strmBody
        );

        tCIDLib::TVoid ParseFrom
        (
                    TCIDDataSrc&            cdsMQTT
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard4         c4Size
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LogMsgInfo
        (
                    TTextOutStream&         strmTar
        )   const   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckFixedFlags
        (
            const   tCIDLib::TCard1         c1MustBe
        )   const;

        tCIDLib::TVoid CheckRemLen
        (
            const   tCIDLib::TCard4         c4MinReq
            , const tCIDLib::TCard4         c4Got
        );

        tCIDLib::TVoid ParseConnect
        (
                    TBinInStream&           strmSrc
        );

        tCIDLib::TVoid ParsePublish
        (
                    TBinInStream&           strmSrc
            , const tCIDLib::TCard4         c4RemLen
        );

        tCIDLib::TVoid ParseString
        (
                    TString&                strToFill
            ,       TBinInStream&           strmSrc
        );

        tCIDLib::TVoid ParseSubscribe
        (
                    TBinInStream&           strmSrc
        );

        tCIDLib::TVoid ParseSubAckCodes
        (
                    TBinInStream&           strmSrc
            , const tCIDLib::TCard4         c4RemLen
        );

        tCIDLib::TVoid ParseUnsubscribe
        (
                    TBinInStream&           strmSrc
        );


        // -------------------------------------------------------------------
        //  Private, data members
        //
        //  m_c1PacketFlags
        //      A separate set of flags that a packet may send in addition to the fixed
        //      header ones.
        //
        //  m_c4PLBytes
        //      The number of bytes in m_mbufPayload, which is always zero except for:
        //      1. Connect  - holds the will msg content
        //
        //  m_mbufPayload
        //      For those msgs where we don't actually parse out all of the data,
        //      and some is just kept as is, it's stored here. See m_c4PLBytes
        //      above. It's not much used.
        //
        //  m_mbufTmp
        //      For temp use during parsing of incoming data
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4PLBytes;
        THeapBuf                m_mbufPayload;
        THeapBuf                m_mbufTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTInMsg, TMQTTMsg)
};



// ---------------------------------------------------------------------------
//   CLASS: TMQTTOutMsg
//  PREFIX: msg
// ---------------------------------------------------------------------------
class CQCMQTTEXPORT TMQTTOutMsg : public TMQTTMsg
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard4 c4EncodeMQTTString
        (
            const   tCIDLib::TCh* const     pszSrc
            , const tCIDLib::TCard4         c4Len
            ,       TMemBuf&                mbufTar
        );

        static tCIDLib::TCard4 c4EncodeMQTTString
        (
            const   TString&                strSrc
            ,       TMemBuf&                mbufTar
        );

        static tCIDLib::ESortComps eCompSize
        (
            const   TMQTTOutMsg&            msgFirst
            , const TMQTTOutMsg&            msgSec
            , const tCIDLib::TCard4
        );

        static tCIDLib::ESortComps eCompKeySize
        (
            const   tCIDLib::TCard4&        c4Size
            , const TMQTTOutMsg&            msgSec
            , const tCIDLib::TCard4
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TMQTTOutMsg() = delete;

        TMQTTOutMsg
        (
            const   tCIDLib::TCard4         c4Size
        );

        TMQTTOutMsg(const TMQTTOutMsg&) = delete;
        TMQTTOutMsg(TMQTTOutMsg&&) = delete;

        ~TMQTTOutMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TMQTTOutMsg& operator=(const TMQTTOutMsg& msgSrc) = delete;
        TMQTTOutMsg& operator=(TMQTTOutMsg&& msgSrc) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bNeedsReply() const;

        tCIDLib::TBoolean bSent() const
        {
            return m_bSent;
        }

        tCIDLib::TVoid BuildConnectMsg
        (
           const    TString&                strClientId
            , const tCIDLib::TBoolean       bCleanSession
            , const tCIDLib::TCard2         c2KeepAliveSecs
            , const TString&                strUserName
            , const TString&                strPassword
        );

        tCIDLib::TVoid BuildDisconnectMsg();

        tCIDLib::TVoid BuildPingReqMsg();

        tCIDLib::TVoid BuildPublishMsg
        (
            const   TString&                strTopicPath
            , const tCQCMQTT::EQOSLevs      eQOS
            , const tCIDLib::TBoolean       bRetain
            , const tCIDLib::TCard2         c2PacketId
            , const TString&                strPayloadText
            , const tCIDLib::TBoolean       bDup = kCIDLib::False
        );

        tCIDLib::TVoid BuildPublishMsg
        (
            const   TString&                strTopicPath
            , const tCQCMQTT::EQOSLevs      eQOS
            , const tCIDLib::TBoolean       bRetain
            , const tCIDLib::TCard2         c2PacketId
            , const TMemBuf&                mbufPayload
            , const tCIDLib::TCard4         c4PLBytes
            , const tCIDLib::TBoolean       bDup = kCIDLib::False
        );

        tCIDLib::TVoid BuildPubAckMsg
        (
            const   tCIDLib::TCard2         c2PacketId
        );

        tCIDLib::TVoid BuildPubCompMsg
        (
            const   tCIDLib::TCard2         c2PacketId
        );

        tCIDLib::TVoid BuildPubRecMsg
        (
            const   tCIDLib::TCard2         c2PacketId
        );

        tCIDLib::TVoid BuildPubRelMsg
        (
            const   tCIDLib::TCard2         c2PacketId
        );

        tCIDLib::TVoid BuildSubscribeMsg
        (
            const   tCQCMQTT::TTopicCol&    colTopics
            , const tCIDLib::TCard2         c2PacketId
        );

        tCIDLib::TCard4 c4BufSize() const
        {
            return m_mbufBody.c4Size();
        }

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard4         c4Size
        );

        tCIDLib::TVoid SendOn
        (
                    TCIDDataSrc&            cdsMQTT
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LogMsgInfo
        (
                    TTextOutStream&         strmTar
        )   const   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Append
        (
            const   tCIDLib::TCard1         c1Value
        );

        tCIDLib::TVoid Append
        (
            const   tCIDLib::TCard2         c2Value
        );

        tCIDLib::TVoid Append
        (
            const   tCIDLib::TCh* const     pszVal
            , const tCIDLib::TCard4         c4Len
        );

        tCIDLib::TVoid Append
        (
            const   TString&                strValue
        );

        tCIDLib::TVoid AppendPacketId
        (
            const   tCIDLib::TCard2         c2Value
        );

        tCIDLib::TVoid AppendPayload
        (
            const   TMemBuf&                mbufPL
            , const tCIDLib::TCard4         c4PLBytes
        );

        tCIDLib::TVoid AppendResultCode
        (
            const   tCIDLib::TCard1         c1Value
        );

        tCIDLib::TVoid AppendTopicPath
        (
            const   TString&                strValue
            , const tCQCMQTT::EQOSLevs      eQOS
        );

        tCIDLib::TVoid CheckBuildType
        (
            const   tCQCMQTT::EPacketTypes  eCheck
        );

        tCIDLib::TVoid Reset
        (
            const   tCQCMQTT::EPacketTypes  eType
            , const tCIDLib::TCard1         c1Flags
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSent
        //      This is set once the msg is sent, and reset back to false when
        //      this msg is reset. This is for use by the client code to support
        //      whatever he needs.
        //
        //  m_mbufBody
        //      m_strmBody is set up over this, but we keep it separate so that we
        //      can resize and easily access it. We stream out all of the stuff
        //      past the fixed header to this.
        //
        //  m_mbufTmp
        //      A temp data buffer for formatting purposes, mostly text encoding.
        //
        //  m_strmBody
        //      An output stream, into which we output all of the post-fixed header
        //      stuff. It is set up over m_mbufBody above.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSent;
        THeapBuf                m_mbufBody;
        THeapBuf                m_mbufTmp;
        TBinMBufOutStream       m_strmBody;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMQTTOutMsg, TMQTTMsg)
};

#pragma CIDLIB_POPPACK
