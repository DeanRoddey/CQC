//
// FILE NAME: ZWaveUSB3Sh_OutMsg.hpp
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
//  This is the header file for the ZWaveUSB3Sh_OutMsg.cpp file, which implements
//  the TZOutWMsg class. Outgoing messages are just built up as a memory buffer, but
//  we provide helpers designed to make that easier. And we separately store some stuff
//  that is added to the msg as well because it's not always easy to get back to that
//  information in the raw buffer due to the variety of msg layouts.
//
//  Since the messages are queued and sent async by the Z-Stick controller class, we
//  really have to put everything required for the controller class to handle the
//  sending of the message into the msg itself. So we have a good bit of stuff that is
//  not actually part of the message at all, such as some stuff about the target unit
//  (such as is it a frequent listener, if it a secure msg, if we need to request a
//  nonce for it, and so forth.)
//
//  If a message gets encrypted, the original is stored as a member, so that they can
//  both be logged together. Otherwise it's very difficult to log from a single, safe
//  place and have it know how a msg being sent came to be.
//
//
//  Waiting for Transmission
//
//  The driver queues up msgs to be sent. Sometimes he needs to do this synchronously and
//  so he needs to wait for the msg to be processed. He may also wait for a specific
//  response from the target unit, but Z-Wave doesn't provide any means to link that
//  response to an outgoing msg. But he also needs a way to wait for the msg to get to
//  the top of the outgoing queue, to be sent, and acked. He can't just wait for some
//  fixed time, because other activity may be going on.
//
//  So we assign a unique 'ack id' to each out msg. The Z-Stick class provides a method
//  to call in and wait for the ack of a msg with the indicated id, returning ack status
//  if appropriate. It does do retries and will wait until it gives up before returning
//  a failure status. If the ack is successful, and happens before the driver thread gives
//  up waiting, it can then start to wait for any response. If none is expected then it
//  is done and can move on.
//
//  Priority
//
//  Because Z-Wave is not fast and some things have to get done ASAP, such as talking to
//  battery powered units during the short period they are available during their wakeup
//  period, and dealing with nonces for secure messages, and so forth, we have a hierarcy
//  of outgoing message priorities. The sender marks them appropriately. The Z-Stick class
//  will grab the next available one of the highest priority to send next. See the Z-Stick
//  class for a description of the priorities.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TZOutWMsg
//  PREFIX: zowm
// ---------------------------------------------------------------------------
class ZWUSB3SHEXPORT TZWOutMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TZWOutMsg();

        TZWOutMsg
        (
            const   TZWOutMsg&              zwomSrc
        );

        ~TZWOutMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZWOutMsg& operator=
        (
            const   TZWOutMsg&              zwomSrc
        );

        tCIDLib::TVoid operator+=
        (
            const   tCIDLib::TCard1         c1ToAdd
        );

        tCIDLib::TCard1 operator[]
        (
            const   tCIDLib::TCard4         c4At
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AppendBytes
        (
            const   TMemBuf&                mbufSrc
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TVoid AppendCallback
        (
            const   tCIDLib::TBoolean       bNonBlocking = kCIDLib::False
        );

        tCIDLib::TVoid AppendNullCallback();

        tCIDLib::TVoid AppendTransOpts
        (
            const   tCIDLib::TBoolean       bReceiptAck = kCIDLib::False
        );

        tCIDLib::TBoolean bCanQueue() const
        {
            // I.e. it's been finalized
            return (m_eState > tZWaveUSB3Sh::EOMState_Working);
        }

        tCIDLib::TBoolean bFreqListener() const
        {
            return m_bFreqListener;
        }

        tCIDLib::TBoolean bIsCCMsg() const
        {
            return m_bIsCCMsg;
        }

        tCIDLib::TBoolean bIsPing() const;

        tCIDLib::TBoolean bNeedsCallback() const;

        tCIDLib::TBoolean bQueryCCIds
        (
                    tCIDLib::TCard1&        c1ClassId
            ,       tCIDLib::TCard1&        c1CmdId
        )   const;

        tCIDLib::TBoolean bReadyToSend() const
        {
            // Any final work is done, such as required encryption
            return (m_eState == tZWaveUSB3Sh::EOMState_ReadyToSend);
        }

        tCIDLib::TBoolean bSecure() const
        {
            return m_bSecure;
        }

        tCIDLib::TCard1 c1CallbackId
        (
            const   tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        tCIDLib::TCard1 c1ClassId() const;

        tCIDLib::TCard1 c1CmdId() const;

        tCIDLib::TCard1 c1CCByteAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCIDLib::TCard1 c1CCReplyClass() const
        {
            return m_c1CCReplyClass;
        }

        tCIDLib::TCard1 c1CCReplyCmd() const
        {
            return m_c1CCReplyCmd;
        }

        tCIDLib::TCard1 c1MsgId() const
        {
            return m_c1MsgId;
        }

        tCIDLib::TCard1 c1ReplyMsgId() const
        {
            return m_c1ReplyMsgId;
        }

        tCIDLib::TCard1 c1TarId
        (
            const   tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        tCIDLib::TCard1 c1TransOpts
        (
            const   tCIDLib::TBoolean       bThrowIfNot = kCIDLib::False
        )   const;

        tCIDLib::TCard4 c4AckId() const
        {
            return m_c4AckId;
        }

        tCIDLib::TCard4 c4CCBytes() const;

        tCIDLib::TCard4 c4CopyOutCCBytes
        (
                    TMemBuf&                mbufToFill
            , const tCIDLib::TCard4         c4StartAt
        )   const;

        tCIDLib::TCard4 c4IncSendCost
        (
            const   tCIDLib::TCard4         c4IncBy
        );

        tCIDLib::TCard4 c4MsgBytes() const
        {
            return m_c1Count;
        }

        tCIDLib::TVoid ChangeTransOpts
        (
            const   tCIDLib::TCard1         c1ToSet
        );

        tCIDLib::TVoid ChangePriority
        (
            const   tZWaveUSB3Sh::EMsgPrios eNewPrio
        );

        tZWaveUSB3Sh::EMsgTypes eType() const;

        tCIDLib::TVoid Encrypt
        (
            const   TMemBuf&                mbufNonce
        );

        tCIDLib::TVoid EndCounter();

        tCIDLib::TVoid EndPointEncap
        (
            const   tCIDLib::TCard1         c1SrcEP
            , const tCIDLib::TCard1         c1TarEP
        );

        tCIDLib::TVoid Finalize
        (
            const   tCIDLib::TBoolean       bFreqListener
            , const tCIDLib::TBoolean       bSecure
            , const tCIDLib::TBoolean       bReqNonce = kCIDLib::False
        );

        tCIDLib::TVoid Finalize
        (
            const   TZWUnitInfo&            unitiTar
            , const tCIDLib::TBoolean       bReqNonce = kCIDLib::False
        );

        tCIDLib::TVoid MakePing
        (
            const   tCIDLib::TCard1         c1TarId
            , const tZWaveUSB3Sh::EMsgPrios ePrio
        );

        const tCIDLib::TCard1* pc1DataAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const tCIDLib::TCard1* pc1CCBytes() const;

        const TZWOutMsg* pzwomOrg() const;

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1MsgId
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tZWaveUSB3Sh::EMsgTypes eType = tZWaveUSB3Sh::EMsgTypes::Request
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1TarId
            , const tCIDLib::TCard1         c1MsgId
            , const tZWaveUSB3Sh::EMsgPrios ePriority
            , const tZWaveUSB3Sh::EMsgTypes eType = tZWaveUSB3Sh::EMsgTypes::Request
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1TarId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1OutCmdId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1CCBytes
            , const tZWaveUSB3Sh::EMsgPrios ePriority
        );

        tCIDLib::TVoid Reset
        (
            const   tCIDLib::TCard1         c1TarId
            , const tCIDLib::TCard1         c1ClassId
            , const tCIDLib::TCard1         c1OutCmdId
            , const tCIDLib::TCard1         c1ReplyClassId
            , const tCIDLib::TCard1         c1ReplyCmdId
            , const tCIDLib::TCard1         c1CCBytes
            , const tZWaveUSB3Sh::EMsgPrios ePriority
        );

        tCIDLib::TVoid StartCounter();

        tCIDLib::TVoid UpdateCallback();


    protected :
        // -------------------------------------------------------------------
        //  The Z-Stick class is our friend, which lets us avoid a lot of public methods
        //  that only he would use anyway.
        // -------------------------------------------------------------------
        friend class TZStick;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        typedef TFundStack<tCIDLib::TCard1> TCounterStack;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCallbackId
        //      If this is true, then m_c1CBId holds the callback id to use.

        //  m_bFreqListener
        //      The target unit is a frequent listener, so it's always there but
        //      requires some extra work to make sure he's listening.
        //
        //  m_bIsCCMsg
        //      Indicates if this is a command class message, so that we don't have to
        //      depend on the content to figure this out. We set it if they call the
        //      ctor that sets up a CC msg.
        //
        //  m_bNonceReq
        //      If it's a secure message they can indicate we should pre-ask for a nonce,
        //      which is done if there is subsequent secure comms to do. This gets us back
        //      a nonce a lot faster that continually having to ask and wait.
        //
        //  m_bReqNonce
        //      If it's a secure msg, this indicates if we should request a nonce up front
        //      when we know we are doing multiple secure messages in a row.
        //
        //  m_bSecure
        //      This message needs to be sent securely. Since messages get queued up
        //      this info needs to be included in the queued info so that the
        //      transmission thread can know this. Note that this only applies to the
        //      original message. It's not set on the encrypted version, which is ready
        //      to send. But, if it was originally encrypted, m_pzwomOrg is set.
        //
        //   m_bTranOpts
        //      If they want transmit options, this is true and m_c1TransOpts holds the
        //      options they want.
        //
        //  m_c1CBIdOffset
        //      The offset in the msg where the callback id is set. If this is zero then
        //      none has been added to this msg.
        //
        //      If it is set to 0xFF, that is a special one that allows call backs, but the
        //      I/O thread will not wait for the callback. This is needed for replication
        //      where multiple callbacks happen and the first one may be a while. Normally
        //      0xFF will never be used.
        //
        //  m_c1Count
        //      The current count of bytes. We could do it in the 0th byte, but it's
        //      more convenient to keep it separate and save it at the end.
        //
        //  m_c1CCReplyClass
        //  m_c1CCReplyCmd
        //      If this is a command class msg, this is the reply to expect in return. If
        //      none, then these should be zero. The class is usually the same as what
        //      was sent, but not always (such as when sending multi-channel msgs, but we
        //      see the encapsulated msg when we get the reply since it's extracted out
        //      by the I/O thread.)
        //
        //  m_c1MsgId
        //      The id of the message being sent.
        //
        //  m_c1ReplyMsgId
        //      If not zero this msg needs to get a reply of this msg id. Defaults
        //      to the outgoing id, but needs to be changed sometimes.
        //
        //  m_c1TarId
        //      The target node for this msg, zero if none.
        //
        //  m_c1TransOpts
        //      If m_bTransOpts is true, these are added to the message.
        //
        //  m_c1TransOptOffset
        //      The offset in the msgs where the transmit options were put. If this is
        //      zero, then they have not been added.
        //
        //  m_c4AckId
        //      See the class comments above, used by the driver thread to wait do sync
        //      waiting for msg transmission.
        //
        //  m_c4SendCost
        //  m_c4SendCount
        //      Send count is the number of times we have sent this message. Used in the
        //      trace logging mostly, to make it clear we are re-sending something.
        //      SendCost is the 'cost' of each retry, which is different for different
        //      things, so that we can do low cost retries more often. The Z-Stick will
        //      update the send cost based on what he thinks each retry should cost, and
        //      he will give up when it hits some value he thinks is too much.
        //
        //  m_ePriority
        //      The priority at which to queue up the msg for transmission. The correct
        //      priority is very important to correct flow of data. The Z-Stick maps these
        //      to an underlying queue priority when it adds the msg to the queue.
        //
        //  m_eState
        //      We need to keep up with our state to know when it's OK to send it or
        //      to add bytes to it and so forth. It's updated as calls are made to add
        //      stuff to our content. Finalize must be called ultimately to make it
        //      ready to send.
        //
        //  m_eType
        //      The msg type, whihc is request or response for these outgoing msgs.
        //
        //  m_fcolCounters
        //      To make things easier, we have some byte counters. They can start one
        //      at any point before the message is complete. We push the current
        //      size. They can then end it later, and we will store (at the original
        //      start position) the number of bytes added since then. It's an error
        //      if they call CompleteMsg while a counter is active.
        //
        //      They can be inclusive (includes the byte count byte) or exclusive
        //      (only includes the bytes after it to the end of the counter.)
        //
        //  m_mbufData
        //      The message data being built up.
        //
        //  m_pzwomOrg
        //      If this message was encrypted from an original msg, this is that original
        //      one. Else it is null.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bFreqListener;
        tCIDLib::TBoolean       m_bIsCCMsg;
        tCIDLib::TBoolean       m_bNonceReq;
        tCIDLib::TBoolean       m_bReqNonce;
        tCIDLib::TBoolean       m_bSecure;
        tCIDLib::TBoolean       m_bTransOpts;
        tCIDLib::TCard1         m_c1CBIdOfs;
        tCIDLib::TCard1         m_c1Count;
        tCIDLib::TCard1         m_c1CCReplyClass;
        tCIDLib::TCard1         m_c1CCReplyCmd;
        tCIDLib::TCard1         m_c1MsgId;
        tCIDLib::TCard1         m_c1ReplyMsgId;
        tCIDLib::TCard1         m_c1TarId;
        tCIDLib::TCard1         m_c1TransOpts;
        tCIDLib::TCard1         m_c1TransOptsOfs;
        tCIDLib::TCard4         m_c4AckId;
        tCIDLib::TCard4         m_c4SendCost;
        tCIDLib::TCard4         m_c4SendCount;
        tZWaveUSB3Sh::EMsgPrios m_ePriority;
        tZWaveUSB3Sh::EMsgTypes m_eType;
        tZWaveUSB3Sh::EOMStates m_eState;
        TCounterStack           m_fcolCounters;
        THeapBuf                m_mbufData;
        TZWOutMsg*              m_pzwomOrg;



        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TCard1 c1NextCBId();

        static tCIDLib::TCard4 c4NextAckId();


        // -------------------------------------------------------------------
        //  Private, static members
        //
        //  s_c1NextCBId
        //      We want to assign a new callback id to each new command class msg that
        //      is going to be sent, within the one byte limit of course. So we use a
        //      counter protected by a critical section.
        //
        //  s_c4NextAckId
        //      A counter used to assign a unique id to every new outgoing msg. SEe the
        //      class docs above for details.
        //
        //  s_crsCBId
        //      A criticial section to protect the counters
        // -------------------------------------------------------------------
        static tCIDLib::TCard1  s_c1NextCBId;
        static tCIDLib::TCard4  s_c4NextAckId;
        static TCriticalSection s_crsCBId;
};

#pragma CIDLIB_POPPACK

