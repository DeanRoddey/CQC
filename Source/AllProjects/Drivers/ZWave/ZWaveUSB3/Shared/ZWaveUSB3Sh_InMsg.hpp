//
// FILE NAME: ZWaveUSB3Sh_InMsg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2017
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
//  This is the header file for the ZWaveUSB3Sh_InMsg.cpp file, which implements
//  the TZWInMsg class. This class encapsulates an incoming Z-Wave message, making it
//  easier to deal with. When we get a message, it is parsed and the values broken
//  out for easy access.
//
//  This class is only used for receipt of msgs. Transmission is done by a separate
//  class, which is mostly just a wrapper around a memory buffer into which the
//  msg is built, along with some housekeeping flags.
//
//  All messages (with the exception of ACK/NAK see below) have four leading bytes
//  in common, and have a trailing checksum. In between goes any message type
//  specific stuff.
//
//      0 - Start of Fame  (discarded by us upon receipt)
//      1 - The overall length of the message (not including SOF and checksum)
//      2 - Whether it is a request or a response
//      3 - The msg id (what actual message it is)
//
//          [msg type specific stuff goes here]
//
//      len-1 - Checksum  (discarded by us on receipt)
//
//  All other bytes are considered 'payload' bytes and are stored separately as
//  a byte array which the caller can look through based on his knowledge of the
//  type of message it is.
//
//  In terms of msgs received there are two scenarios.
//
//  1.  It's some message from the controller itself (always a response to something
//      we sent to it.) These have arbitrary content as required for the operation
//      being responsed to.
//  2.  It's a command or response from another unit, in which case it is a command
//      class msg (app command), which has a defined payload format. This is always
//      marked as a request. It may be on response to something we sent to it, or
//      something it just sent to us that we need to respond to.
//
//  In the case of CC msgs, in addition to the standard stuff above there is:
//
//      4 - Receipt flags
//      5 - The source unit id
//      6 - Number of command class bytes
//      7 - The command class payload bytes
//      8 - Command class id
//      9 - Command id
//      10...x The rest of the CC payload bytes (so #7 minus two.)
//
//  This class is mostly designed for these command class messages, though we also use
//  it for other types. We know that all messages start with the standard four bytes
//  which are removed and discarded or separately (req/res and msg id.)
//
//  Everything after #3 is dependent on the msg type and is message payload. If we see
//  that it's a command class message based on the function id, see below) we will break
//  out the receipt flags and src id and store them separately, then the payload is the
//  CC msg stuff (starting with the number of cc bytes, #6 above.) Next will be the
//  class id and command id.
//
//  CC msgs can target a specific end point in multi-end point devices. If it's one of
//  those we de-encapsulate it and store the source/target end points separately, so again
//  the payload is just the CC msg stuff in that case.
//
//  We have helper methods to get the separately broken out stuff, and in some cases
//  specific values out what we store as the payload part of the message. We store the
//  number of bytes that we stored as payload.
//
//  When we get a CC msg, the function id is FUNC_ID_APPLICATION_COMMAND_HANDLER.
//  This is a special scenario so that we know we are getting a request or command
//  from another unit. These are always command class messages.
//
//  There are special cases for ACK, NAK or CAN, which are just single byte messages and
//  replace the SOF byte. But those are handled via the message type return value,
//  since they require no data.
//
//  When a message is decrypted from an encrypted msg, the encrypted message is stored
//  on the decrypted one, so that they can both be logged together later.
//
// CAVEATS/GOTCHAS:
//
//  1)  We have index operators to get the Nth byte of a message. As indicated above that
//      is relative to what we store as payload. For CC msgs that starts at the CC bytes
//      count. We also have methods that will get CC bytes at a particular index. In that
//      case it is relative to #8, so the 0th CC byte will be the command class id.
//
//  2)  We just make our members public and the Z-Stick class fills it in appropriately.
//      In this case, the knowledge of the format of incoming messages is better
//      encapsulated there than here because he's already using a state machine to pull
//      msgs out of the incoming stream. We don't want to duplicate that by parsing
//      ourself out a stored buffer of data.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TZWInMsg
//  PREFIX: zwim
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWInMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWInMsg();

        TZWInMsg
        (
            const   TZWInMsg&               zwimSrc
        );

        ~TZWInMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWInMsg& operator=
        (
            const   TZWInMsg&               zwimSrc
        );

        tCIDLib::TCard1 operator[]
        (
            const   tCIDLib::TCard4         c4At
        )   const;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDecrypt
        (
            const   TMemBuf&                mbufNonce
        );

        tCIDLib::TBoolean bIsBasicReport
        (
                    tCIDLib::TBoolean&      bState
        )   const;

        tCIDLib::TBoolean bIsAppCmd() const;

        tCIDLib::TBoolean bIsAppCmd
        (
                    tCIDLib::TCard1&        c1ClassId
            ,       tCIDLib::TCard1&        c1CmdId
        )   const;

        tCIDLib::TBoolean bIsAppCmd
        (
                    tCIDLib::TCard1&        c1ClassId
            ,       tCIDLib::TCard1&        c1CmdId
            ,       tCIDLib::TCard1&        c1UnitId
            ,       tCIDLib::TCard1&        c1ReceiptFlags
        )   const;

        tCIDLib::TBoolean bIsAppUpdate() const;

        tCIDLib::TBoolean bNeedsAck() const;

        tCIDLib::TBoolean bWasEncapsulated() const
        {
            return m_bWasEncap;
        }

        tCIDLib::TBoolean bIsSingleCast() const;

        tCIDLib::TBoolean bIsWakeUp() const;

        tCIDLib::TCard1 c1CallbackId() const;

        tCIDLib::TCard1 c1CalcSum() const;

        tCIDLib::TCard1 c1ClassId() const;

        tCIDLib::TCard1 c1CmdId() const;

        tCIDLib::TCard1 c1CCByteAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TCard1 c1SrcEPId() const
        {
            return m_c1SrcEPId;
        }

        tCIDLib::TCard1 c1TarEPId() const
        {
            return m_c1TarEPId;
        }

        tCIDLib::TCard1 c1MsgId() const;

        tCIDLib::TCard1 c1SrcId
        (
            const   tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        tCIDLib::TCard4 c4CCBytes() const;

        tCIDLib::TCard4 c4FirstCCIndex() const;

        tCIDLib::TCard4 c4LastCCIndex() const;

        tCIDLib::TVoid ConvertEncap
        (
                    tCIDLib::TCard1&        c1ClassId
            ,       tCIDLib::TCard1&        c1CmdId
        );

        tCIDLib::TVoid CopyOutPLBytes
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Count
        )   const;

        tCIDLib::TVoid CutPLBytes
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TEncodedTime enctReceived() const;

        tCIDLib::TVoid Finalize();

        const tCIDLib::TCard1* pc1CCPayload() const;

        const tCIDLib::TCard1* pc1Payload() const;

        const tCIDLib::TCard1* pc1PayloadAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const tCIDLib::TCard1* pc1PayloadEnd() const;

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1PayloadLen
        );

        tCIDLib::TVoid SetPLByteAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TCard1         c1Byte
        );


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bSecure
        //      Indicates whether it was received securely or non-securely. If securely
        //      the encrypted version is stored in m_pzimOrg;
        //
        //  m_bWasEncap
        //      If this msg was end point encapsulated, then this is true and the
        //      m_c1Src/TarEPId members are valid.
        //
        //  m_c1SrcEPId
        //  m_c1TarEPId
        //      If it was encapsulated, then the msg was extracted then these are the
        //      end points from the original msg. If so, then m_bWasEnap is true.
        //
        //  m_c1MsgId
        //      The message level id, i.e. message is it
        //
        //  m_c1PLLength
        //      The payload byte length
        //
        //  m_eType
        //      The overall type of the message. Special ones like ack, nak and can are
        //      just returned as an enum. So this is either request, response, or trans
        //      ack.
        //
        //  m_enctReceived
        //      The time this msg was received. This is for trace logging.
        //
        //  m_mbufPayload
        //      The payload bytes, which has m_c4PLLength bytes in it. It could
        //      be zero if the function id is all that is needed. This is whatever
        //      comes after the length, type and function id bytes.
        //
        //  m_pzwimOrg
        //      If we were decrypted from a secure message, then this is the original
        //      msg. Otherwise its null. The caller should check that m_bSecure is
        //      set before trying to access it.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSecure;
        tCIDLib::TBoolean       m_bWasEncap;
        tCIDLib::TCard1         m_c1SrcEPId;
        tCIDLib::TCard1         m_c1TarEPId;
        tCIDLib::TCard1         m_c1MsgId;
        tCIDLib::TCard1         m_c1PLLength;
        tZWaveUSB3Sh::EMsgTypes m_eType;
        tCIDLib::TEncodedTime   m_enctReceived;
        THeapBuf                m_mbufPayload;
        TZWInMsg*               m_pzwimOrg;
};

#pragma CIDLIB_POPPACK
