//
// FILE NAME: ZWaveUSB3S_ZStick.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  Sending Messages:
//
//  Send a msg can request a number of steps. Note that all we deal with here are acks
//  and trans acks (as well as errors and retries where needed.) The driver thread has to
//  deal with any ultimate replies from the target unit. But there still can be
//  a number of steps that have to acked and trans acked.
//
//  1.  If the target is a frequent listener, we have to ping first to wake it up. So we
//      have to wait for the ack and trans ack of that. There is no further response. This
//      is done in m_zwomPing.
//  2.  If the msg must be sent securely, and we don't already have a nonce for that unit,
//      then we have to first send a nonce request and wait for the ack, trans ack and
//      the final nonce. This is done in m_zwomNonceReq.
//  3.  Finally, or perhaps only if non-secure and and always on device, we now have a
//      msg (possibly now encrypted), and we have to send that and wait for the ack
//      and possibly the trans ack if the caller set a callback id.
//
//  We have a state machine that runs us through these transitions as required. We don't
//  need to send a ping for every msg we send. If we are sending a sequence of msgs the
//  device will stay awake. So we remember the last node id and time stamp that we got
//  a ping transack from. If it's been less than a few seconds, we don't bother doing it.
//
//  If we don't have a needed nonce at #2, then we send a request. If the nonce arrives,
//  then the current msg is encrypted and we move to stage #3. If the nonce request times
//  out after retrying, we just go back to idle state and the current msg basically fails
//  and cannot be sent.
//
//  Msg Priority
//
//  We use queues for in and out msgs. For the outgoing, we use the priority system that
//  the queue supports. We define our own priority values, which we map to these queue
//  priority levels (which are 0 for lowest to 10 for highest.)
//
// 10.  Local. This is really just for the node protocol info query, which we make
//      during initialization and after a replication. We need to make sure it gets
//      processed fast because if we time out waiting for it back in the driver
//      and there's an extra one stuck in the queue, we have no way to know. The msg
//      doesn't have an identifying node in it to let us know which node it is sending
//      back info for. Since it's only used in those special times, it's not going to
//      interfere with nonces or security messages.
//  9.  Nonces. These need to be dealt with as quickly as possible.
//  8.  Security. Non-nonce security oriented msgs, mostly just done during replication.
//  7.  SpecialCmd. These are low level, non-CC msgs, housekeeping stuff, mostly done
//      during replication and startup. They are synchronous because they have to get
//      done before the next step and we want to expedite things during these special
//      operations.
//  6.  Wakeup. We have gotten a wakeup from a non-listener, and we have queued up msgs to
//      send to him. He will only stay awake so long so we need to prioritize these msgs.
//      The driver will send these synchronously, at least up to a certain number of them.
//      The other unit should stay awake while we are talking to it.
//  5.  Reply. If another node is waiting for us to respond to something, we need to
//      get that back quickly, but only after taking care of security stuff. As a controller
//      we don't get many of these, but they happen, particularly during the replication
//      process.
//  3.  Command. A normal priority msg for outgoing commands. These get priority over
//      queries since they are user invoked. It is assumed these will request a transmission
//      ack from the target, since that is often the only way to know it worked. There may
//      be no actual response from the target unit. The caller will wait synchronously for
//      the trans ack.
//  2.  Query. A normal priority msg for queries, i.e. getting info from a unit, which will
//      be waited on synchronously. Since a reply is ultimately expected, no trans ack
//      is generally used here, just the ack that it has been queued to send. This may
//      be done async or sync, see below.
//  0.  Async. Msgs that are queued up on us and the caller is just going to asynchronously
//      wait for a reply to come back. This is typically done by units when they are getting
//      their initial values, or perhaps if they need to do some slow periodic ping.
//
//  We define our own priority value for the outside world to use. They set it on the out
//  msgs. We convert that to the correct queue priority when we we get the msgs to queue
//  up.
//
//  Async Msg Sending
//
//  Note that #0 and #2 above indicate that some messages are just queued up and no one
//  is watching for the acks. The Z-Stick will handle those as usual. Eventually, it is
//  expected in a working system, that the unit will respond and the incoming msgs will
//  get routed to the unit object, which will update his state based on that response.
//  Or, it will see that the response is not forthcoming after a generous wait and try
//  it again.
//
//  The timeout should be pretty long, because various other msgs may need to be sent
//  first and these could involve nonce exchanges and maybe a unit croaks and isn't
//  responding. So 20 to 30 seconds or more is a reasonable time scale for waiting
//  for such low priority responses.
//
//  Sync Msg Sending
//
//  Waiting for msgs synchronously is more difficult. There may be other activity in the
//  queue. So the driver can't just send and wait a fixed time. It will be waiting for an
//  ultimate response from the unit, but it needs first to know when the msg actually got
//  to the top of the queue and was sent and acked (and trans acked if appropriate.) It
//  needs to know if the trans ack failed and so the reply will enver come back.
//
//  There is a callback id at the Z-Wave level that comes back in the trans ack. But that
//  can change if the msg needs to be retried in some cases, and there is nothing in the
//  initial, low level ack. So the driver needs some way to wait on those things to happen
//  and get a status back. If successful, then it can start waiting for the unit to
//  respond if that is needed (not always if it's a synchronous command (and most commands
//  will be.)
//
//  So, we assign an 'ack id' of our own to each outgoing msg. And we provide a method
//  for the driver to block until the last ack (of step #3 in the first list above) is
//  done or, if any of the steps fail, after retries. He may still give up before we get
//  there, but that's fine. It gives him a means to wait for his msg to be sent and to
//  know that it worked or not. If it did, and there's a subsequent response from the unit
//  expected, he can then wait for that with a known timeout since at that point he knows
//  that his msg is in play.
//
//  Locking
//
//  Other than the wee bit of stuff related to the ack id discussed above and the queuing
//  and dequeing of msgs by the driver, there's almost no connection between our internal
//  processing thread and the driver thread.
//
//  So synchronization issues are minimal. We need to sync the driver thread getting
//  set up to wait for his msg to be acked, and our thread setting up the return status
//  and and waking him up. So we have mutex for our bit of locking, and an event for the
//  driver thread to block on until we store a status for him.
//
//  We don't use that mutex for the in/out msg queues. They are also points of contact
//  between the driver thread and our thread; but, these we mark thread safe and let them
//  handle their own affairs because they are doing more than simple coordination. They
//  are allowing for blocking while waiting for input, waiting for space in the queue, and
//  those sorts of things.
//
//  CAN Replies
//
//  Z-Wave is sort of bad about being able to do more than one thing at a time. If we
//  queue up an out msg on the stick, but another msg comes in before ours is sent, we
//  get a CANcel msg. We have to read and ack the new msg then we can try to resend ours.
//  This could happen some number of times in a row.
//
//  But we are currently in a state where we can't just read and process the msg because
//  that might try to do something that would interfere. And we can't just ack it and leave
//  it in the stick's queue because we'd end up acking it again later. And we can't just
//  stick them into the input queue, because that's stuff intended for the driver thread,
//  and not all msgs are for him to see.
//
//  So we have a separate CAN queue of in msgs. If we get cancelled we will read the msg,
//  push it into that CAN queue then ack it. We can't just stick it in the input queue,
//  since that's only for msgs that are to be seen by the driver. It may be one that we
//  process internally. When it's next time to check for incoming msgs, we first check the
//  can queue. If anything there we process it internally or put it in the input queue.
//  If not, we read from the stick. Though it's unlikely we would ever need storage for
//  more than one msg, we use a queue anyway to just be safe.
//
//  Z-Stick Errors
//
//  There's something strange with the Z-Stick for some users. It will after some days
//  start giving write errors and never recover. So we have to try to cycle the port if
//  that happens. We won't report being offline at first. We'll recycle and try it again.
//  If that fails, then we'll go offline.
//
// CAVEATS/GOTCHAS:
//
//  1)  WE DO NOT have any access to Z-Wave units or to the predefined device info stuff
//      provided by the shared facility class. We deal with msg I/O and that's it.
//
//      If we started accessing that stuff, the configuration info would then require
//      sync that it doesn't currently need because it's only accessed by the driver
//      (or its replication) thread.
//
//      So quite a bit of info is placed into outgoing msgs, to let us know how to deal
//      with them. Incoming msgs aren't an issue, the driver thread will pass them onto
//      the correct handler, usually a unit.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TZWaveUSB3SDriver;

// ---------------------------------------------------------------------------
//   CLASS: TZStick
//  PREFIX: zst
// ---------------------------------------------------------------------------
class TZStick
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TZStick
        (
            const   TString&                strMoniker
        );

        TZStick(const TZStick&) = delete;

        ~TZStick();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TZStick& operator=(const TZStick&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConnected() const;

        tCIDLib::TBoolean bOpenCtrl();

        tCIDLib::TBoolean bWaitTrans
        (
            const   tCIDLib::TCard4         c4WaitForId
            , const tCIDLib::TCard4         c4WaitMSs
        );

        tCIDLib::TCard4 c4CANQUsedPercent() const;

        tCIDLib::TCard4 c4OutQUsedPercent() const;

        tCIDLib::TVoid Close();

        tZWaveUSB3Sh::EMsgTypes eGetMsg
        (
                    TZWInMsg&               zwmToFill
            , const tCIDLib::TCard4         c4WaitMSs
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TVoid LogTermTrace();

        tCIDLib::TVoid QueueOutMsg
        (
            const   TZWOutMsg&              zwomToSend
        );

        tCIDLib::TVoid SetConfig
        (
            const   TString&                strPort
            , const TCommPortCfg&           cpcfgToSet
        );

        tCIDLib::TVoid SetManIds
        (
            const   tCIDLib::TCard8         c8ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private types
        // -------------------------------------------------------------------
        using TCANQ = TQueue<TZWInMsg>;
        using TInQ = TQueue<TZWInMsg>;
        using TOutQ = TQueue<TZWOutMsg>;


        // -------------------------------------------------------------------
        //  When other nodes ask us for a nonce, we have to store them away so that
        //  we can access them when the response based on that nonce comes back. We
        //  need to track when the nonce was generated so that we can time it out. We
        //  keep up with the node that we sent it to as well, to validate that we get
        //  it back from that node.
        // -------------------------------------------------------------------
        class TNonceInfo
        {
            public :

                TNonceInfo() :

                    m_enctValidTill(0)
                    , m_mbufNonce(kZWaveUSB3Sh::c4NonceBytes, kZWaveUSB3Sh::c4NonceBytes)
                {
                }

                // For in nonces where we get the bytes
                TNonceInfo
                (
                    const   tCIDLib::TCard1         c1UnitId
                    , const TMemBuf&                mbufBytes
                );

                // For out nonces where we generate the bytes
                TNonceInfo
                (
                    const   tCIDLib::TCard1         c1UnitId
                    , const tCIDLib::TCard1         c1NonceId
                );

                ~TNonceInfo() {}

                tCIDLib::TCard1         m_c1UnitId;
                tCIDLib::TEncodedTime   m_enctValidTill;
                THeapBuf                m_mbufNonce;
        };
        using TNonceList        = TVector<TNonceInfo>                 ;
        using TTimeStampList    = TFundArray<tCIDLib::TEncodedTime>   ;
        using TTOArray          = TEArray
        <
            tCIDLib::TEncodedTime
            , tZWaveUSB3S::EIOStates
            , tZWaveUSB3S::EIOStates::Count
        >;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindInNonce
        (
            const   tCIDLib::TCard1         c1NonceId
            , const tCIDLib::TCard1         c1UnitId
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bFindOutNonce
        (
            const   tCIDLib::TCard1         c1UnitId
            ,       THeapBuf&               mbufToFill
        );

        tCIDLib::TBoolean bSpecialMsg
        (
                    TZWInMsg&               zwimToCheck
        );

        tCIDLib::TBoolean bTryResend
        (
            const   tCIDLib::TBoolean       bNewCBId
            , const tCIDLib::TCard4         c4RetryInc
        );

        tCIDLib::TVoid BuildNonceRequest
        (
            const   tCIDLib::TCard1         c1SrcId
            ,       TZWOutMsg&              zwomTar
        );

        tCIDLib::TCard1 c1CalcSum
        (
            const   TMemBuf&                mbufMsg
        )   const;

        tZWaveUSB3Sh::EMsgTypes eGetNext
        (
                    TZWInMsg&               zwimToFill
        );

        tCIDLib::EExitCodes eIOThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TEncodedTime enctStateTimeout
        (
            const   tZWaveUSB3S::EIOStates  eIOState
            , const tCIDLib::TBoolean       bFreqListener
        )   const;

        tCIDLib::EQPrios eXlatPriority
        (
            const   tZWaveUSB3Sh::EMsgPrios eToXlat
        )   const;

        [[nodiscard]] TNonceInfo* pniMakeNewInNonce
        (
            const   tCIDLib::TCard1         c1UnitId
        );

        TZWOutMsg* pzwomForState();

        tCIDLib::TVoid SendNonceReport
        (
            const   tCIDLib::TCard1         c1RetId
        );

        tCIDLib::TVoid StartNewOutMsg();

        tCIDLib::TVoid StartSecureTrans();

        tCIDLib::TVoid StoreOutNonce
        (
            const   tCIDLib::TCard1         c1UnitId
            , const THeapBuf&               mbufNonce
        );

        tCIDLib::TVoid TransmitMsg
        (
                    TZWOutMsg&              zwomSend
        );

        tCIDLib::TVoid TriggerMsgDone
        (
            const   tCIDLib::TCard4         c4AckId
            , const tCIDLib::TBoolean       bSuccess
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_aenctStateTO
        //      A list of time outs that we set up for each I/O state.
        //
        //  m_bWaitAckRes
        //      See m_evWaitAck. This is the result left for the driver thread. It is
        //      synced by m_mtxSync.
        //
        //  m_c1NextNonceId
        //      The next nonce id we will assign when we need to generate a nonce.
        //
        //  m_c4CanQUsed
        //      To avoid having to make the cancel queue thread safe, just so that the
        //      driver thread can poll the percentage used, we keep this up to date
        //      any time we modify the queue. He can then read this. It's the current
        //      elements in the queue.
        //
        //  m_c4Cancels
        //  m_c4Naks
        //  m_c4Timeouts
        //      Some stats we keep about outgoing msgs failures.
        //
        //  m_c4WaitAckId
        //      See class comments. This is the ack id of the msg that the driver thread
        //      is waiting to get sent. If zero, the driver is not waiting. It is synced
        //      with m_mtxSync.
        //
        //  m_c8ManIds
        //      The main driver passes us the manufacturer ids he got for us during the
        //      initial scan. We serve it up any time we get a request for that info.
        //
        //  m_colCANQ
        //      See the file comments above. This lets us deal with an otherwise messy
        //      situation related to the Z-Stick cancelling our sends due to an incoming
        //      msg. KEEP THE m_c4CanQUsed values updated with the count of elements in
        //      this collection any time it is modified.
        //
        //  m_colInQ
        //  m_colOutQ
        //      Our input and output queues, managed by the background thread. They are
        //      thread safe for maximum overlapping access. m_mtxSync is not used on
        //      these since they already handle their own sync.
        //
        //  m_colInNonces
        //      These are the nonces that we have issued to other nodes, which they
        //      use to send messages back to us (those for incoming messages.) So we
        //      have to keep them around until we get the response or the nonce times
        //      out.
        //
        //  m_colOutNonces
        //      These are nonces that we got got pre-sent to us from other nodes, because
        //      we sent them a secure message with GET_NONCE option so that it would go
        //      ahead and send one. We have to be able to store those. We may not use it
        //      before it times out. If we don't already have a nonce for the target,
        //      then we'll be waiting for it and use it immediately. So this is only for
        //      storing the pre-sent types.
        //
        //      We use the m_eReqState to know if one has already been requested (by
        //      sending the non-getting encrypted msg) and the time stamp will be when it
        //      was requested. Once we get it, it will be set to indicate it's available
        //      and the time stamp will be how long it's good for.
        //
        //  m_cpcfgSerial
        //      The main driver gives us the serial port configuration which we use
        //      to open it.
        //
        //  m_enctNextMsg
        //      We enforce a minimum inter-message interval, so that we will not send
        //      two messages to the controller within a minimum time. So, after we
        //      send a message, it is bumped to now plus that minimum interval.
        //
        //  m_eIOState
        //      Controls our I/O thread's state machine
        //
        //  m_fcolLastPing
        //      For each possible unit id, we store the last time we sent out a ping
        //      (of the internal type used to wake up frequent listeners.) THis lets
        //      us avoid doing it again too soon.
        //
        //  m_evWaitAck
        //      See the class comments above. The driver can block and wait until we have
        //      sent his msg and gotten appropriate acks. Access is controlled by
        //      m_mtxSync. See m_c4WaitAckId. It's manual mode.
        //
        //  m_mbufOut
        //      A temp buffer for sending out messages in some of our helpers that
        //      build and send them in one shot, or that add stuff to provided data
        //      and then sends the whole thing.
        //
        //  m_mbufNonce
        //      To get nonces out of the nonce list so that we can remove the nonce
        //      before we try to en/decrypt, to insure it's gone in case we get some
        //      sort of error in the process.
        //
        //  m_mtxSync
        //      See the class comments above. Not much to worry about, but a few things
        //      we need to sync with the driver thread.
        //
        //  m_pcommZWave
        //      Our comm port that we use to talk to the Z-Wave controller.
        //
        //  m_strPort
        //      The port we were told to use to talk to the Z-Stick
        //
        //  m_thrIO
        //      A background thread, started on eIOThread, which reads and writes the
        //      comm port. Filling in m_colInQ and draining m_colOutQ.
        //
        //  m_zwomCur
        //  m_zwomNonceReq
        //  m_zwomPing
        //      These are used for processing outgoing msgs. When we pull a new msg out
        //      of the out queue, it goes to Cur. If that indicates it's going to a
        //      frequent listener, we have to build a built into Ping and process that.
        //      If we need to send secure, then we need to build a nonce request and
        //      wait for the nonce. Ultimately we then finally send the current msg.
        // -------------------------------------------------------------------
        TTOArray                m_aenctStateTO;
        tCIDLib::TBoolean       m_bWaitAckRes;
        tCIDLib::TCard1         m_c1NextNonceId;
        tCIDLib::TCard4         m_c4CANQUsed;
        tCIDLib::TCard4         m_c4WaitAckId;
        tCIDLib::TCard8         m_c8ManIds;
        TCANQ                   m_colCANQ;
        TInQ                    m_colInQ;
        TNonceList              m_colInNonces;
        TNonceList              m_colOutNonces;
        TOutQ                   m_colOutQ;
        TCommPortCfg            m_cpcfgSerial;
        tCIDLib::TEncodedTime   m_enctNextMsg;
        tZWaveUSB3S::EIOStates  m_eIOState;
        TEvent                  m_evWaitAck;
        TTimeStampList          m_fcolLastPing;
        THeapBuf                m_mbufOut;
        THeapBuf                m_mbufNonce;
        TMutex                  m_mtxSync;
        TCommPortBase*          m_pcommZWave;
        TString                 m_strPort;
        TThread                 m_thrIO;
        TZWOutMsg               m_zwomCur;
        TZWOutMsg               m_zwomNonceReq;
        TZWOutMsg               m_zwomPing;
};

#pragma CIDLIB_POPPACK
