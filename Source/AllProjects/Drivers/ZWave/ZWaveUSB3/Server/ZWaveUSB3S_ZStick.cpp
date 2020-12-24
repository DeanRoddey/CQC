//
// FILE NAME: ZWaveUSB3S_ZStick.cpp
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
//  This file wraps the fundamental stuff required to access the Z-Stick.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3S_.hpp"


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveUSB3_ZStick
{
    const tCIDLib::TCard4   c4CANQSz    = 32;
    const tCIDLib::TCard4   c4OutQSz    = 256;
    const tCIDLib::TCard4   c4InQSz     = 128;
}


// ---------------------------------------------------------------------------
//   CLASS: TZStick::TNonceInfo
//  PREFIX: ni
// ---------------------------------------------------------------------------

// This is for when we get an incoming nonce and we need to copy away the bytes
TZStick::TNonceInfo::TNonceInfo(const   tCIDLib::TCard1         c1UnitId
                                , const TMemBuf&                mbufBytes) :

    m_c1UnitId(c1UnitId)
    , m_enctValidTill(TTime::enctNowPlusSecs(kZWaveUSB3Sh::c4InNonceValidSecs))
    , m_mbufNonce(kZWaveUSB3Sh::c4NonceBytes, kZWaveUSB3Sh::c4NonceBytes)
{
    m_mbufNonce.CopyIn(mbufBytes, kZWaveUSB3Sh::c4NonceBytes);
}

// This is for when we create an ougoing nonce and need to create the bytes
TZStick::TNonceInfo::TNonceInfo(const   tCIDLib::TCard1         c1UnitId
                                , const tCIDLib::TCard1         c1NonceId) :

    m_c1UnitId(c1UnitId)
    , m_enctValidTill(TTime::enctNowPlusSecs(kZWaveUSB3Sh::c4InNonceValidSecs))
    , m_mbufNonce(kZWaveUSB3Sh::c4NonceBytes, kZWaveUSB3Sh::c4NonceBytes)
{
    // The nonce id is the first byte, then generate 7 more random ones
    m_mbufNonce.PutCard1(c1NonceId, 0);
    facCIDCrypto().GetRandomBytes(m_mbufNonce, 1, 7);
}


// ---------------------------------------------------------------------------
//   CLASS: TZStick
//  PREFIX: zst
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZStick: Constructors and destructor
// ---------------------------------------------------------------------------
TZStick::TZStick(const TString& strMoniker) :

    m_aenctStateTO(tCIDLib::TEncodedTime(0))
    , m_bWaitAckRes(kCIDLib::False)
    , m_c1NextNonceId(1)
    , m_c4CANQUsed(0)
    , m_c4WaitAckId(0)
    , m_c8ManIds(0)
    , m_colCANQ(tCIDLib::EMTStates::Unsafe)
    , m_colInQ(tCIDLib::EMTStates::Safe)
    , m_colOutQ(tCIDLib::EMTStates::Safe)
    , m_colInNonces(kZWaveUSB3Sh::c4MaxUnits)
    , m_colOutNonces(kZWaveUSB3Sh::c4MaxUnits)
    , m_enctNextMsg(0)
    , m_eIOState(tZWaveUSB3S::EIOStates::Idle)
    , m_fcolLastPing(kZWaveUSB3Sh::c4MaxUnits + 1)
    , m_mbufOut(128, 255)
    , m_pcommZWave(nullptr)
    , m_thrIO
      (
          TString(L"ZWIOThread") + strMoniker
          , TMemberFunc<TZStick>(this, &TZStick::eIOThread)
      )
{
    //
    //  Set up the per-state timeouts. These will be extended for frequent listener type
    //  modules. If you change the ack times, you have to update the eConnect() method
    //  of the driver to set appropriate timeouts on the Z-Stick itself. We currently
    //  set it to 2.5 seconds, and our timeout is slightly longer at three.
    //
    m_aenctStateTO[tZWaveUSB3S::EIOStates::Idle] = 0;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitPingAck]       = kCIDLib::enctThreeSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitPingTransAck]  = kCIDLib::enctFiveSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitNonceAck]      = kCIDLib::enctThreeSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitNonceTransAck] = kCIDLib::enctFiveSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitNonce]         = kCIDLib::enctFourSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitAck]           = kCIDLib::enctThreeSeconds;
    m_aenctStateTO[tZWaveUSB3S::EIOStates::WaitCallback]      = kCIDLib::enctFourSeconds;

    // Randomly select an initial next nonce id. Can't be 0 or 0xFF
    m_c1NextNonceId = (TTime::c4Millis() >> 4) & 0xFE;
    if (!m_c1NextNonceId)
        m_c1NextNonceId = 132;
}

TZStick::~TZStick()
{
}


// ---------------------------------------------------------------------------
//  TZStick: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Lets the driver know if we are connected to the serial port.
//
tCIDLib::TBoolean TZStick::bIsConnected() const
{
    TLocker lockrSync(&m_mtxSync);
    return m_pcommZWave && m_pcommZWave->bIsOpen();
}


//
//  The driver calls this to get us started. We try to open the comm port. If we
//  can't we fail. If we can, we start up the I/O thread.
//
tCIDLib::TBoolean TZStick::bOpenCtrl()
{
    TLocker lockrSync(&m_mtxSync);

    try
    {
        // Clear up any previous content
        m_colCANQ.RemoveAll();
        m_c4CANQUsed = 0;

        m_colOutQ.RemoveAll();
        m_colInQ.RemoveAll();
        m_colInNonces.RemoveAll();
        m_colOutNonces.RemoveAll();

        // If we currently have a port, clean it up
        if (!m_pcommZWave)
            m_pcommZWave = facCIDComm().pcommMakeNew(m_strPort);
        if (m_pcommZWave->bIsOpen())
            m_pcommZWave->Close();

        // And now try to set it up
        m_pcommZWave->Open(tCIDComm::EOpenFlags::WriteThrough, 2048, 2048);
        m_pcommZWave->SetCfg(m_cpcfgSerial);

        // Send an initial nak
        tCIDLib::TCard1 c1Send = NAK;
        if (m_pcommZWave->c4WriteRawBufMS(&c1Send, 1, 100) != 1)
        {
            facZWaveUSB3S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3Errs::errcComm_InitMsgFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::InitFailed
            );
        }

        // Start the thread now that the port is available
        m_thrIO.Start();
    }

    catch(TError& errToCatch)
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcComm_InitPort
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::InitFailed
        );

        if (m_pcommZWave)
        {
            try
            {
                m_pcommZWave->Close();
                delete m_pcommZWave;
                m_pcommZWave = nullptr;
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  The driver thread calls here to wait for a specific msg to be sent and acked. Our
//  internal thread will update the shared members when a new msg is sent or fails to
//  be sent.
//
tCIDLib::TBoolean
TZStick::bWaitTrans(const tCIDLib::TCard4 c4WaitForId, const tCIDLib::TCard4 c4WaitMSs)
{
    //
    //  Lock and set up. We have to unlock in order to block on the event. So there is
    //  a short opportunity for it to already have triggered it. But that's ok. We'll
    //  just fall through.
    //
    {
        TLocker lockrSync(&m_mtxSync);
        m_c4WaitAckId = c4WaitForId;
        m_evWaitAck.Reset();

        // If we time out, this will be the result still in place for return
        m_bWaitAckRes = kCIDLib::False;
    }

    // Now let's wait,. while waiting for a shutdown request
    TThread* pthrCaller = nullptr;
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitMSs * kCIDLib::enctOneMilliSec);
    while (enctCur < enctEnd)
    {
        if (m_evWaitAck.bWaitFor(150))
            break;

        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
            break;

        enctCur = TTime::enctNow();
    }

    //
    //  Lock again and clean up. NOTE that he could still do it between our coming back
    //  from the wait above and getting locked below. That's OK, if so, then the result
    //  got set. Else it is still set to false as we set it above.
    //
    tCIDLib::TBoolean bRet;
    {
        TLocker lockrSync(&m_mtxSync);

        //
        //  Make sure he knows no one is waiting now, and trigger the event for consistency
        //  so that it's always triggered if we are not waiting.
        //
        m_c4WaitAckId = 0;
        m_evWaitAck.Trigger();

        //
        //  Set up the return value. If it never got set, then it's still timeout, which
        //  is appropriate.
        //
        bRet = m_bWaitAckRes;
    }

    return bRet;
}


//
//  The driver polls these and keeps some field updated, so that we can monitor how
//  much output queue usage is going on.
//
tCIDLib::TCard4 TZStick::c4CANQUsedPercent() const
{
    //
    //  Just grab a copy of the value it leaves for the driver thread to read. This
    //  avoids the need to make the CAN queue thread safe.
    //
    tCIDLib::TCard4 c4Count = m_c4CANQUsed;
    c4Count *= 100;
    c4Count /= ZWaveUSB3_ZStick::c4CANQSz;

    if (c4Count > 100)
        c4Count = 100;
    return c4Count;
}

tCIDLib::TCard4 TZStick::c4OutQUsedPercent() const
{
    // This one is thread safe so no need to explicitly lock just to get the count
    tCIDLib::TCard4 c4Count = m_colOutQ.c4ElemCount();
    c4Count *= 100;
    c4Count /= ZWaveUSB3_ZStick::c4OutQSz;

    if (c4Count > 100)
        c4Count = 100;
    return c4Count;
}


// The driver calls this to shut us down when it's time to stop the driver
tCIDLib::TVoid TZStick::Close()
{
    // Stop the processing thread if it's running
    if (m_thrIO.bIsRunning())
    {
        try
        {
            m_thrIO.ReqShutdownSync(10000);
            m_thrIO.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    //
    //  Close the serial port if it is open. No sync required, we just stopped the
    //  I/O thread above.
    //
    if (m_pcommZWave)
    {
        try
        {
            if (m_pcommZWave->bIsOpen())
                m_pcommZWave->Close();

            delete m_pcommZWave;
            m_pcommZWave = nullptr;
        }

        catch(TError& errToCatch)
        {
            if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  The calling (driver) thread is blocked on the queue until either a msg is available
//  to read or it times out.
//
tZWaveUSB3Sh::EMsgTypes
TZStick::eGetMsg(           TZWInMsg&           zwimToFill
                    , const tCIDLib::TCard4     c4WaitMS
                    , const tCIDLib::TBoolean   bThrowIfNot)
{
    // Nothing there, so do a real read
    TThread* pthrCaller = nullptr;
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();

    //
    //  Note that we add 1 because having end and current start off the same
    //  complicates things below. This will make no effective difference but
    //  make things a lot easier for us.
    //
    const tCIDLib::TEncodedTime enctEnd
    (
        enctCur + (c4WaitMS * kCIDLib::enctOneMilliSec) + 1
    );
    while (enctCur < enctEnd)
    {
        const tCIDLib::TCard4 c4Left
        (
            tCIDLib::TCard4((enctEnd - enctCur) / kCIDLib::enctOneMilliSec)
        );

        // Tell it not to throw on timeout, we want to throw our own error
        const tCIDLib::TBoolean bRes = m_colInQ.bGetNext
        (
            zwimToFill, c4Left, kCIDLib::False
        );

        // If we got one, then break out
        if (bRes)
            break;

        enctCur = TTime::enctNow();

        // If we don't get it first time, start checking for shutdown requests
        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
        {
            // We were asked to stop, so force a timeout
            enctCur = enctEnd;
            break;
        }
    }

    //
    //  If we timed out, then throw or return timeout status. The times could be
    //  the same if it was a zero timeout.
    //
    if (enctCur >= enctEnd)
    {
        if (bThrowIfNot)
        {
            facZWaveUSB3S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW3Errs::errcProto_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }

        zwimToFill.Reset(0);
        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Timeout;
        return tZWaveUSB3Sh::EMsgTypes::Timeout;
    }

    return zwimToFill.m_eType;
}


//
//  Called by the main driver on shutdown to give us a chance to log any final trace info
//  we want. We can assume here that our thread is stopped.
//
tCIDLib::TVoid TZStick::LogTermTrace()
{
    tCIDLib::TCard4 c4PendingCnt = m_colOutQ.c4ElemCount();
    if (c4PendingCnt)
    {
        facZWaveUSB3Sh().LogTraceMsgNL
        (
            L"Final outstanding out msg count was %(1)", TCardinal(c4PendingCnt)
        );
        TZWOutMsg zwomCur;
        while (m_colOutQ.bGetNext(zwomCur, 0))
            facZWaveUSB3Sh().LogOutMsg(nullptr, zwomCur);
    }

    c4PendingCnt = m_colInQ.c4ElemCount();
    if (c4PendingCnt)
    {
        facZWaveUSB3Sh().LogTraceMsgNL
        (
            L"Final outstanding in msg count was %(1)", TCardinal(c4PendingCnt)
        );
        TZWInMsg zwimCur;
        while (m_colInQ.bGetNext(zwimCur, 0))
            facZWaveUSB3Sh().LogInMsg(nullptr, zwimCur);
    }
}


//
//  Just queues up the passed message to be sent as soon as the I/O thread can get
//  it sent.
//
tCIDLib::TVoid TZStick::QueueOutMsg(const TZWOutMsg& zwomToSend)
{
    TLocker lockrSync(&m_colOutQ);

    // If it's full, then something is wrong and we are backing up badly, so flush it
    if (m_colOutQ.bIsFull(ZWaveUSB3_ZStick::c4OutQSz))
    {
        m_colOutQ.RemoveAll();
        facZWaveUSB3Sh().LogTraceErr
        (
            L"The output msg queue has become full. It is being flushed so msgs have been lost"
        );

        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcComm_OutQFull
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
        );
    }

    m_colOutQ.objPut(zwomToSend, eXlatPriority(zwomToSend.m_ePriority));
}


//
//  The driver calls this upon startup to give us the info we need to do our thing.
//
tCIDLib::TVoid TZStick::SetConfig(const TString& strPort, const TCommPortCfg& cpcfgToSet)
{
    // Assume reuse here so close any previous serial port
    Close();

    m_strPort = strPort;
    m_cpcfgSerial = cpcfgToSet;
}


tCIDLib::TVoid TZStick::SetManIds(const tCIDLib::TCard8 c8ToSet)
{
    m_c8ManIds = c8ToSet;
}


// ---------------------------------------------------------------------------
//  TZStick: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Sees if we have the indicated in msg nonce. This is used when we receive an encrypted
//  message. We have to have issued a nonce for that unit to use to send one back to
//  us. That will have been stored away. If found, we give back the bytes and delete
//  it from the list.
//
tCIDLib::TBoolean
TZStick::bFindInNonce(  const   tCIDLib::TCard1     c1NonceId
                        , const tCIDLib::TCard1     c1UnitId
                        ,       THeapBuf&           mbufToFill)
{
    //
    //  If the list size has gotten large, let's for a pruning pass. That generally
    //  means there are expired nonces that never got used an so they have expired.
    //
    TNonceList::TNCCursor cursFind(&m_colInNonces);
    if (m_colInNonces.c4ElemCount() > 64)
    {
        const tCIDLib::TEncodedTime enctPrune = TTime::enctNow();
        for (; cursFind; ++cursFind)
        {
            if (cursFind->m_enctValidTill < enctPrune)
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Removed timed out nonce for unit %(1)\n", TCardinal(c1UnitId)
                    );
                }
                m_colInNonces.RemoveAt(cursFind);
            }
        }
    }

    // Search our nonce list for one that matches the passed ids
    tCIDLib::TBoolean bRet = kCIDLib::False;
    for (cursFind.bReset(); cursFind; ++cursFind)
    {
        if ((cursFind->m_mbufNonce[0] == c1NonceId)
        &&  (cursFind->m_c1UnitId == c1UnitId))
        {
            // If we haven't found one yet, take this one
            if (!bRet)
            {
                mbufToFill.CopyIn(cursFind->m_mbufNonce, kZWaveUSB3Sh::c4NonceBytes);
                bRet = kCIDLib::True;

                // And remove it from the list
                m_colInNonces.RemoveAt(cursFind);
            }
        }
    }

    // If we got it, log so info if appropriate
    if (bRet)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Using stored in nonce for unit %(1). %(2) in nonces left in list"
                , TCardinal(c1UnitId)
                , TCardinal(m_colInNonces.c4ElemCount())
            );
        }
    }

    return bRet;
}


//
//  See if we have a nonce for the indicated unit and it's still valid. We could have
//  more than one, so use the one that has the least time left.
//
tCIDLib::TBoolean
TZStick::bFindOutNonce(const tCIDLib::TCard1 c1UnitId, THeapBuf& mbufToFill)
{
    tCIDLib::TCard4 c4Count = m_colOutNonces.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;

    const tCIDLib::TEncodedTime enctPrune = TTime::enctNow();
    tCIDLib::TCard4 c4BestIndex = kCIDLib::c4MaxCard;
    while (c4Index < c4Count)
    {
        const TNonceInfo& niCur = m_colOutNonces[c4Index];

        // If it's for our guy, let's check it
        if (niCur.m_c1UnitId == c1UnitId)
        {
            // If it's valid time is before now, consider it dead and remove it
            if (niCur.m_enctValidTill < enctPrune)
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Removed timed out nonce for unit %(1). Looking for more...\n"
                        , TCardinal(c1UnitId)
                    );
                }

                // Remove this guy, reduce the count, and keep trying
                m_colOutNonces.RemoveAt(c4Index);
                c4Count--;
            }
             else if (c4BestIndex == kCIDLib::c4MaxCard)
            {
                // This is the first one, so take it provisionally
                c4BestIndex = c4Index;
                c4Index++;
            }
             else
            {
                // If this one has less time left, take it
                const TZStick::TNonceInfo& niBest = m_colOutNonces[c4BestIndex];
                const TZStick::TNonceInfo& niNew = m_colOutNonces[c4Index];
                if (niNew.m_enctValidTill < niBest.m_enctValidTill)
                    c4BestIndex = c4Index;

                c4Index++;
            }
        }
         else
        {
            // Not for our guy, so just move forward
            c4Index++;
        }
    }

    // If we never found one, return false
    if (c4BestIndex != kCIDLib::c4MaxCard)
    {
        const TZStick::TNonceInfo& niBest = m_colOutNonces[c4BestIndex];
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            const tCIDLib::TCard4 c4MSLeft
            (
                tCIDLib::TCard4
                (
                    (niBest.m_enctValidTill - enctPrune) / kCIDLib::enctOneMilliSec
                )
            );

            //
            //  Sub 1 from count since we've not removed it yet, and can't do this after
            //  we remove since that would trash niBest.
            //
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Using stored out nonce for unit %(1). MSs left on nonce=%(2). "
                L"%(3) out nonces left"
                , TCardinal(c1UnitId)
                , TCardinal(c4MSLeft)
                , TCardinal(m_colOutNonces.c4ElemCount() - 1)
            );
        }

        // Copy out the bytes and remove the one we chose
        mbufToFill.CopyIn(niBest.m_mbufNonce, kZWaveUSB3Sh::c4NonceBytes);
        m_colOutNonces.RemoveAt(c4BestIndex);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  The I/O thread will call this on each incoming thread. We will check for some
//  special cases. Some we handle ourself. If so we return True and the driver will
//  not queue it up for the driver to see.
//
//  If it is an encrypted message. We will decrypt it and then process the actual message
//  so all decryption happens here in one place.
//
//  If it was encapsulated in the 'nonce get' command or a request for a nonce, we'll
//  immediately send a nonce back to the caller, not queue it up.
//
//  We also watch here for msgs that we know came from another node, and we store a last
//  time stamp for that unit id, so that we can track the last time we heard from any
//  given unit. In some cases we can't tell here, such as trans acks because whether it
//  is from another unit depends on the msg type and the options used to send what it
//  is responding to. But mostly we can handle this here by just saying if it's a CC
//  msg, it's an obvious candidate. Our caller can handle some special cases.
//
//  If we see a start of replication msg, we will flush our nonce queues and force us
//  to get all new ones.
//
tCIDLib::TBoolean TZStick::bSpecialMsg(TZWInMsg& zwimIn)
{
    // If it's a timeout, do nothing, say it's special
    if (zwimIn.m_eType == tZWaveUSB3Sh::EMsgTypes::Timeout)
        return kCIDLib::True;

    // Check for some special ones
    if ((zwimIn.m_eType == tZWaveUSB3Sh::EMsgTypes::Ack)
    ||  (zwimIn.m_eType == tZWaveUSB3Sh::EMsgTypes::Can)
    ||  (zwimIn.m_eType == tZWaveUSB3Sh::EMsgTypes::Nak)
    ||  (zwimIn.m_eType == tZWaveUSB3Sh::EMsgTypes::TransAck))
    {
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogInMsg(nullptr, zwimIn);

        // We don't pass these to the driver
        return kCIDLib::True;
    }

    //
    //  If not an app command (cc msg), then not special, so let it propagate, but
    //  we do check for a start of replication and flush our nonce queues. If we
    //  are starting a new replication then most likely any stored nonces would not
    //  be good once we get on the other side, particularly if this is the initial
    //  inclusion.
    //
    tCIDLib::TCard1 c1ClassId, c1CmdId;
    if (!zwimIn.bIsAppCmd(c1ClassId, c1CmdId))
    {
        if ((zwimIn.m_c1MsgId == FUNC_ID_ZW_SET_LEARN_MODE)
        &&  (zwimIn[1] == LEARN_MODE_STARTED))
        {
            m_colInNonces.RemoveAll();
            m_colOutNonces.RemoveAll();
        }

        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogInMsg(nullptr, zwimIn);
        return kCIDLib::False;
    }

    const tCIDLib::TCard1 c1SrcId = zwimIn.c1SrcId();

    //
    //  Handle encrypted msgs up front, since the code below needs to see the actual
    //  msg that was sent. Do this before end point encapsulated msg handling below,
    //  since an end point encap msg can be secure.
    //
    if ((c1ClassId == COMMAND_CLASS_SECURITY)
    &&  ((c1CmdId == SECURITY_MESSAGE_ENCAPSULATION)
        || (c1CmdId == SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET)))
    {
        // We have to have a nonce for this msg
        const tCIDLib::TCard1 c1NonceId = zwimIn[zwimIn.c4LastCCIndex() - 8];
        if (bFindInNonce(c1NonceId, c1SrcId, m_mbufNonce))
        {
            if (facZWaveUSB3Sh().bHighTrace())
            {
                facZWaveUSB3Sh().LogTraceMsg
                (
                    L"Decrypting with nonce %(1)/0x%(2)"
                    , TCardinal(m_mbufNonce[0])
                    , TCardinal(m_mbufNonce[0], tCIDLib::ERadices::Hex)
                );
            }
            const tCIDLib::TBoolean bDecrypted = zwimIn.bDecrypt(m_mbufNonce);
            if (!bDecrypted)
            {
                if (facZWaveUSB3Sh().bLowTrace())
                    facZWaveUSB3Sh().LogTraceErr(L"Decryption Failed");
                return kCIDLib::True;
            }

            // Remember if we owe a nonce before we update the command id
            const tCIDLib::TBoolean bOweNonce(c1CmdId == SECURITY_MESSAGE_ENCAPSULATION_NONCE_GET);

            // Update the class/cmd ids we got above with the actual ones
            zwimIn.bIsAppCmd(c1ClassId, c1CmdId);

            // Send back a nonce if asked to
            if (bOweNonce)
                SendNonceReport(c1SrcId);
        }
         else if (facZWaveUSB3Sh().bTraceEnabled())
        {
            facZWaveUSB3Sh().LogInMsg(L"No nonce available for incoming msg\n", zwimIn);
        }
    }

    //
    //  Handle a special case of multi-channel encapsulated commands. We convert it
    //  to a regular msg (with end point id stored in it now) and fall through to
    //  below to handle it.
    //
    if ((c1ClassId == COMMAND_CLASS_MULTI_CHANNEL_V2)
    &&  (c1CmdId == MULTI_CHANNEL_CMD_ENCAP_V2))
    {
        // We get the new, actual class/cmd ids
        zwimIn.ConvertEncap(c1ClassId, c1CmdId);

        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Expanded end pnt msg. SrcEP=%(1), TarEp=%(2)\n"
                , TCardinal(zwimIn.c1SrcEPId())
                , TCardinal(zwimIn.c1TarEPId())
            );
        }
    }

    // Log the message now if tracing
    if (facZWaveUSB3Sh().bTraceEnabled())
        facZWaveUSB3Sh().LogInMsg(nullptr, zwimIn);

    if (c1ClassId == COMMAND_CLASS_MANUFACTURER_SPECIFIC)
    {
        //
        //  We want to respond to this because it'll typically get sent during the
        //  replication process.
        //
        if (c1CmdId == MANUFACTURER_SPECIFIC_GET)
        {
            // Send a report using the ids we were given, securely if we got it so
            TZWOutMsg zwomRep;
            zwomRep.Reset
            (
                c1SrcId
                , COMMAND_CLASS_MANUFACTURER_SPECIFIC
                , MANUFACTURER_SPECIFIC_REPORT
                , 0
                , 8
                , tZWaveUSB3Sh::EMsgPrios::Reply
            );
            zwomRep += tCIDLib::TCard1(m_c8ManIds >> 40);
            zwomRep += tCIDLib::TCard1(m_c8ManIds >> 32);
            zwomRep += tCIDLib::TCard1(m_c8ManIds >> 24);
            zwomRep += tCIDLib::TCard1(m_c8ManIds >> 16);
            zwomRep += tCIDLib::TCard1(m_c8ManIds >> 8);
            zwomRep += tCIDLib::TCard1(m_c8ManIds);

            zwomRep.AppendTransOpts(kCIDLib::True);
            zwomRep.AppendCallback();

            zwomRep.Finalize(kCIDLib::False, zwimIn.m_bSecure);
            QueueOutMsg(zwomRep);

            // Let the driver see it as well just in case
        }
    }
     else if (c1ClassId == COMMAND_CLASS_SECURITY)
    {
        if (c1CmdId == NETWORK_KEY_SET)
        {
            //
            //  Store it as our driver config string, and then set it on the shared
            //  facility so that it's in place for secure ops moving forward.
            //
            THeapBuf mbufKey(16, 16);
            zwimIn.CopyOutPLBytes(mbufKey, zwimIn.c4FirstCCIndex() + 2, 16);
            facZWaveUSB3Sh().SetNetworkKey(mbufKey);


            //
            //  But let it go to the driver as well, since he watches for this as one
            //  of the steps in the process.
            //
        }
         else if (c1CmdId == SECURITY_NONCE_GET)
        {
            //
            //  Send a nonce report. If we are idle, this will immediately send it, else it
            //  it is queued to send.
            //
            SendNonceReport(c1SrcId);
            return kCIDLib::True;
        }
         else if (c1CmdId == SECURITY_NONCE_REPORT)
        {
            //
            //  If we are waiting for a nonce from this node, then decrypt the staged
            //  msg and move it to the current msg. It will now be in ready to send
            //  state.
            //
            //  If not waiting for one, or it's not for our current msg, then just store
            //  it for potential future use if it's not timed out by then.
            //
            THeapBuf mbufNonce(8, 8);
            zwimIn.CopyOutPLBytes(mbufNonce, 5, 8);

            if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitNonce)
            &&  (m_zwomCur.c1TarId() == zwimIn.c1SrcId()))
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"SEC: Got nonce (Id=%(1)), encrypting staged msg\n"
                        , TCardinal(mbufNonce[0], tCIDLib::ERadices::Hex)
                    );
                }

                // Encrypt it and move to send encrypted state so it'll get sent
                m_zwomCur.Encrypt(mbufNonce);
                m_eIOState = tZWaveUSB3S::EIOStates::SendEncrypted;
            }
             else
            {
                StoreOutNonce(zwimIn.c1SrcId(), mbufNonce);
            }
            return kCIDLib::True;
        }
    }

    // Nothing special
    return kCIDLib::False;
}


//
//  If we haven't retried the current msg too many times, then we'll try to send it again.
//  Else, we'll go back to idle state.
//
//  The caller can tell us whether to generate a new callback id or not. If the msg never
//  got sent (CAN), then we can just re-send it. If it times out or is NAK'd by the other
//  side, we need a new callback id.
//
//  They also tell us a retry increment. Cancels don't count as much as other failures,
//  for instance. But, we still need to give up eventually if we somehow got a bunch
//  of cancels.
//
tCIDLib::TBoolean
TZStick::bTryResend(const   tCIDLib::TBoolean   bNewCBId
                    , const tCIDLib::TCard4     c4RetryInc)
{
    TZWOutMsg* pzwomOut = pzwomForState();
    CIDAssert(pzwomOut != nullptr, L"Got to resend without any output msg");

    const tCIDLib::TCard4 c4Cost = pzwomOut->c4IncSendCost(c4RetryInc);
    if (c4Cost > 9)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Too many failures for state: %(1), giving up"
                , tZWaveUSB3S::strXlatEIOStates(m_eIOState)
            );
        }

        m_eIOState = tZWaveUSB3S::EIOStates::Idle;

        // Trigger the driver thread if he's waiting for this guy
        TriggerMsgDone(pzwomOut->c4AckId(), kCIDLib::False);
        return kCIDLib::False;
    }

    // Set the new state first
    if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitPingAck)
    ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitPingTransAck))
    {
        m_eIOState = tZWaveUSB3S::EIOStates::WaitPingAck;
    }
     else if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceTransAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonce))
    {
        m_eIOState = tZWaveUSB3S::EIOStates::WaitNonceAck;
    }
     else if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitCallback))
    {
        m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
    }

    // If asked and this guy has one, generate a new callback id
    if (bNewCBId && pzwomOut->bNeedsCallback())
    {
        pzwomOut->UpdateCallback();
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Retry #%(1) for state: %(2), new CBId=%(3)"
                , TCardinal(pzwomOut->m_c4SendCount)
                , tZWaveUSB3S::strXlatEIOStates(m_eIOState)
                , TCardinal(pzwomOut->c1CallbackId())
            );
        }
    }
     else
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Retry #%(1) for state: %(2), same CBId=%(3)"
                , TCardinal(pzwomOut->m_c4SendCount)
                , tZWaveUSB3S::strXlatEIOStates(m_eIOState)
                , TCardinal(pzwomOut->c1CallbackId())
            );
        }
    }

    // And now transmit the message again
    TransmitMsg(*pzwomOut);

    // If we went back to idle state, then it failed
    return (m_eIOState != tZWaveUSB3S::EIOStates::Idle);
}


//
//  This will build a nonce request into the passed msg.
//
//  NOTE THAT we don't care about frequent listener issues here. If the target is a
//  frequent listener we will have sent a ping to him before this request is sent.
//
tCIDLib::TVoid TZStick::BuildNonceRequest(const tCIDLib::TCard1 c1SrcId, TZWOutMsg& zwomTar)
{
    zwomTar.Reset
    (
        c1SrcId
        , COMMAND_CLASS_SECURITY
        , SECURITY_NONCE_GET
        , 0
        , 2
        , tZWaveUSB3Sh::EMsgPrios::Nonce
    );
    zwomTar.AppendTransOpts();
    zwomTar.AppendCallback();
    zwomTar.Finalize(kCIDLib::False, kCIDLib::False);
}



//
//  Does the standard Z-Wave checksum calculation. It sets a Card1 to 0xFF,
//  then XORs each byte between the SOF and checksum (non-inclusive.) The buffer
//  does not include the SOF or the checksum, it's just the msg bytes, so we just
//  do the whole thing.
//
//  This is only used for outgoing messages since incoming ones are put into a
//  TZWMsg object and it does the checksum for incoming validation, since the
//  msg is not stored as a contiguous buffer.
//
tCIDLib::TCard1 TZStick::c1CalcSum(const TMemBuf& mbufMsg) const
{
    tCIDLib::TCard1 c1Ret = 0xFF;
    const tCIDLib::TCard4 c4Count = mbufMsg[0];
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        c1Ret ^= mbufMsg[c4Index];
    return c1Ret;
}


//
//  This method will get the next msg from the controller. It will do a very short
//  wait in order to provide some throttling.
//
tZWaveUSB3Sh::EMsgTypes TZStick::eGetNext(TZWInMsg& zwimToFill)
{
    //
    //  This shouldn't happen since we shouldn't get called if the comm port is not
    //  ready, but just in case, just return a timeout, after a short sleep just
    //  to insure we can't get into a fast loop because of this.
    //
    if (!m_pcommZWave)
    {
        zwimToFill.Reset(0);
        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Timeout;
        TThread::Sleep(25);
        return tZWaveUSB3Sh::EMsgTypes::Timeout;
    }

    //
    //  These are the states of a little state machine we use to read in valid
    //  Z-Wave messages. There are some special cases of course, ACK, CAN, etc...
    //
    enum EStates
    {
        EState_WaitSOF
        , EState_Length
        , EState_Type
        , EState_MsgId
        , EState_DataBytes
        , EState_CheckSum
        , EState_Done
    };

    tCIDLib::TBoolean       bExtended = kCIDLib::False;
    tCIDLib::TCard1         c1Cur;
    tCIDLib::TCard1         c1Sum;
    tCIDLib::TCard4         c4SoFar = 0;
    EStates                 eState = EState_WaitSOF;
    tCIDLib::TEncodedTime   enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime   enctEnd = enctCur + (kCIDLib::enctOneMilliSec * 25);

    while ((eState < EState_Done) && !m_thrIO.bCheckShutdownRequest())
    {
        if (m_pcommZWave->c4ReadRawBuf(&c1Cur, 1, enctEnd - enctCur))
        {
            switch(eState)
            {
                case EState_WaitSOF :
                {
                    // Special cases for ack and nak
                    if (c1Cur == ACK)
                    {
                        // Special case type so set it and return
                        zwimToFill.Reset(0);
                        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Ack;
                        return tZWaveUSB3Sh::EMsgTypes::Ack;
                    }
                     else if (c1Cur == CAN)
                    {
                        // Special case type so set it and return
                        zwimToFill.Reset(0);
                        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Can;
                        return tZWaveUSB3Sh::EMsgTypes::Can;
                    }
                     else if (c1Cur == NAK)
                    {
                        // Special case type so set it and return
                        // m_psdrvOwner->IncNakCounter();

                        if (facZWaveUSB3Sh().bMediumTrace())
                            facZWaveUSB3Sh().LogTraceMsg(L"Got a NAK message");

                        zwimToFill.Reset(0);
                        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Nak;
                        return tZWaveUSB3Sh::EMsgTypes::Nak;
                    }

                    // Else has to be a SOF
                    if (c1Cur != SOF)
                    {
                        // m_psdrvOwner->IncBadMsgCounter();

                        // Just a junk byte presumably, so keep waiting
                        eState = EState_WaitSOF;
                    }
                     else
                    {
                        // Move to length state
                        eState = EState_Length;
                    }
                    break;
                }

                case EState_Length :
                {
                    //
                    //  Reset the mesasge for the payload length this represents, so
                    //  the msg length minus the length itself, the type and function id
                    //  all of which we store separately.
                    //
                    zwimToFill.Reset(c1Cur - 3);
                    eState = EState_Type;
                    break;
                }

                case EState_Type :
                {
                    // This has to be either request or response
                    if (c1Cur == REQUEST)
                    {
                        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Request;
                        eState = EState_MsgId;
                    }
                     else if (c1Cur == RESPONSE)
                    {
                        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Response;
                        eState = EState_MsgId;
                    }
                     else
                    {
                        // m_psdrvOwner->IncBadMsgCounter();

                        // Send a NAK and keep trying
                        if (facZWaveUSB3Sh().bMediumTrace())
                            facZWaveUSB3Sh().LogTraceMsg(L"Got bad message type, ignoring received bytes");

                        // Just go back to ground zero
                        eState = EState_WaitSOF;
                    }
                    break;
                }

                case EState_MsgId :
                {
                    // Just store the message id
                    zwimToFill.m_c1MsgId = c1Cur;

                    // If this message has any payload bytes, get those, else checksum
                    if (zwimToFill.m_c1PLLength)
                        eState = EState_DataBytes;
                    else
                        eState = EState_CheckSum;
                    break;
                }

                case EState_DataBytes :
                {
                    // Collect payload bytes until we get them all
                    zwimToFill.SetPLByteAt(c4SoFar++, c1Cur);
                    if (c4SoFar == zwimToFill.m_c1PLLength)
                        eState = EState_CheckSum;
                    break;
                }

                case EState_CheckSum :
                {
                    // Calc the sum on our data and compare
                    c1Sum = zwimToFill.c1CalcSum();

                    // If not the same, then we failed right now
                    if (c1Sum != c1Cur)
                    {
                        // m_psdrvOwner->IncBadMsgCounter();

                        // Send a NAK and keep trying
                        if (facZWaveUSB3Sh().bMediumTrace())
                        {
                            facZWaveUSB3Sh().LogInMsg
                            (
                                L"Bad checksum, nak'ing. Msg Content=", zwimToFill
                            );
                        }
                        tCIDLib::TCard1 c1Send = NAK;
                        m_pcommZWave->c4WriteRawBufMS(&c1Send, 1, 100);

                        // Go back to waiting
                        eState = EState_WaitSOF;
                    }
                     else
                    {
                        eState = EState_Done;
                    }
                    break;
                }
            };
        }

        if (eState != EState_Done)
        {
            // Update the current time
            enctCur = TTime::enctNow();

            //
            //  Since we only set a very short initial timeout, because we only
            //  want to get what's available right now, if we hit the end time but
            //  have gotten some input, we extend the time once, to allow time to
            //  get the rest of the messages.
            //
            if (enctCur >= enctEnd)
            {
                if ((eState != EState_WaitSOF) && !bExtended)
                {
                    enctEnd = enctCur + (100 * kCIDLib::enctOneMilliSec);
                    bExtended = kCIDLib::True;
                }
                 else
                {
                    // Oh well, give up
                    break;
                }
            }
        }
    }

    if (eState != EState_Done)
    {
        zwimToFill.Reset(0);
        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::Timeout;
        return tZWaveUSB3Sh::EMsgTypes::Timeout;
    }

    //
    //  Watch for the special case msg of the 2nd trans/receipt ack for SEND_DATA. It is
    //  marked as a SEND_DATA msgs and has a 4 byte payload. If a transmit ack was requested
    //  this came from the remote unit, else from our controller when it passed off the
    //  msg successfully.
    //
    if ((zwimToFill.m_c1MsgId == FUNC_ID_ZW_SEND_DATA) && (zwimToFill.m_c1PLLength == 4))
        zwimToFill.m_eType = tZWaveUSB3Sh::EMsgTypes::TransAck;

    zwimToFill.Finalize();

    //
    //  If it's a kind of msg that requires an ack, then ack it now. Doing it here
    //  guarantees we don't have any weird issues of timing.
    //
    if ((zwimToFill.m_eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
    ||  (zwimToFill.m_eType == tZWaveUSB3Sh::EMsgTypes::Request)
    ||  (zwimToFill.m_eType == tZWaveUSB3Sh::EMsgTypes::Response))
    {
        tCIDLib::TCard1 c1Send = ACK;
        m_pcommZWave->c4WriteRawBufMS(&c1Send, 1, 100);
    }

    // Return the type that the msg object stored
    return zwimToFill.m_eType;
}


//
//  This thread manages the input and output queues and the serial port.
//
tCIDLib::EExitCodes TZStick::eIOThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let he invoking thread go
    thrThis.Sync();

    tCIDLib::TEncodedTime   enctLast = 0;
    THeapBuf                mbufNonce(8, 8);
    TZWInMsg                zwimIn;
    TZWInMsg                zwimCAN;

    m_eIOState = tZWaveUSB3S::EIOStates::Idle;
    while (kCIDLib::True)
    {
        if (thrThis.bCheckShutdownRequest())
            break;

        //
        //  Shouldn't happen since we will break out of this thread at the bottom
        //  of this loop if we lose the comm port, but just in case of a race
        //  condition of some sort.
        //
        if (!m_pcommZWave)
        {
            if (!thrThis.bSleep(100))
                break;
            continue;
        }

        // If we get here we have the port open so we can try to do some I/O
        try
        {
            //
            //  Check for any message coming in first. This will wait a very short time
            //  for a message, which provides some of our throttling.
            //
            //  We first check the cancel queue. If it's not empty we take the next
            //  msg from that. Else we wait for a real msgs.
            //
            tZWaveUSB3Sh::EMsgTypes eType;
            if (m_colCANQ.bGetNext(zwimIn, 0, kCIDLib::False))
            {
                // Keep our used elements count up to date
                m_c4CANQUsed = m_colCANQ.c4ElemCount();
                eType = zwimIn.m_eType;
            }
             else
            {
                eType = eGetNext(zwimIn);
            }


            // Remember the current state so we can know if it changed
            tZWaveUSB3S::EIOStates ePrevState = m_eIOState;

            // Handle some stuff generically here to simplify code below
            if (eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
            {
                TZWOutMsg* pzwomCheck = pzwomForState();
                if (pzwomCheck && (pzwomCheck->c1CallbackId() != zwimIn.c1CallbackId()))
                {
                    //
                    //  It's a transack for something that we are not waiting for, so
                    //  log a msg if tracing.
                    //
                    if (facZWaveUSB3Sh().bMediumTrace())
                    {
                        facZWaveUSB3Sh().LogTraceMsg
                        (
                            L"Got trans ack for unexpected id=%(1)"
                            , TCardinal(zwimIn.c1CallbackId())
                        );
                    }
                }

                // If the status isn't success, log a msg if tracing.
                if (zwimIn[1] != 0)
                {
                    if (facZWaveUSB3Sh().bLowTrace())
                    {
                        facZWaveUSB3Sh().LogTraceMsg
                        (
                            L"Got error for callback id=%(1), status=%(2)"
                            , TCardinal(zwimIn.c1CallbackId())
                            , TCardinal(zwimIn[1])
                        );
                    }
                }
            }

            if (eType == tZWaveUSB3Sh::EMsgTypes::Timeout)
            {
                if (m_eIOState != tZWaveUSB3S::EIOStates::Idle)
                {
                    //
                    //  We are not in idle mode, so see how long since we last sent a
                    //  msg. If it's too long for the current state, retry if we can,
                    //  else we'll fall back to idle. The retry method will log
                    //  appropriately if needed.
                    //
                    tCIDLib::TEncodedTime enctDiff = TTime::enctNow() - enctLast;
                    if (enctDiff > m_aenctStateTO[m_eIOState])
                        bTryResend(kCIDLib::True, 3);
                }
            }


            //
            //  See if it's something we handle completely internally. If so, this method
            //  will process it. It may also process it but let it pass on to the driver
            //  thread.
            //
            //  This guy also logs the incoming msgs if tracing is enabled, because it
            //  handles decryption and expanding out end point based issues, and we want
            //  to log them decrypted and expanded.
            //
            //  We may also set the special flag below to prevent other stuff from
            //  getting to the driver.
            //
            tCIDLib::TBoolean bIsSpecial = bSpecialMsg(zwimIn);

            //
            //  We have a couple special cases driven by the incoming msg if indicates
            //  we got an outgoing failure. We need to deal with that first.
            //
            //  Else we are driven by our current state first, and within those we
            //  react to msgs received.
            //
            if (eType == tZWaveUSB3Sh::EMsgTypes::Can)
            {
                //
                //  The gotcha here is that we are going to continue to get CANs unless
                //  we read and ack the packet that just came in. But we can't process
                //  the msg right now. So we read it, add it to a cancel queue, then ack
                //  it, then we can try ours again.
                //
                //  Though the input msg object shouldn't be active now, we use a separate
                //  can msg to read into, just to be extra safe.
                //
                //  It's possible we already read the message or it's some bogus delayed
                //  CAN or something, so don't do anything if we fail to get a reply.
                //  That isn't something we consider an error.
                //
                while (eGetNext(zwimCAN) != tZWaveUSB3Sh::EMsgTypes::Timeout)
                {
                    m_colCANQ.objAdd(zwimCAN);
                    m_c4CANQUsed = m_colCANQ.c4ElemCount();
                }

                //
                //  It's possible we will be in idle state here if we get some bogus or
                //  delayed cancel, so don't resend in that case.
                //
                if (m_eIOState != tZWaveUSB3S::EIOStates::Idle)
                {
                    // Delay it a bit
                    m_enctNextMsg = TTime::enctNowPlusMSs(100);
                    bTryResend(kCIDLib::False, 1);
                }
            }
             else if (eType == tZWaveUSB3Sh::EMsgTypes::Nak)
            {
                //
                //  Resend if not too many retries, no new callback id for this one since
                //  it never got sent. If too many retries, just goes back to idle.
                //
                //  Special msg check above set indicates a special one so it doesn't go
                //  to the driver.
                //
                //  It's possible we will be in idle state here, so don't do anything
                //  in that case, it's some bogus or delayed NAK.
                //
                if (m_eIOState != tZWaveUSB3S::EIOStates::Idle)
                {
                    // Delay it a bit
                    m_enctNextMsg = TTime::enctNowPlusMSs(100);
                    bTryResend(kCIDLib::False, 2);
                }
            }
             else
            {
                // Now, depending on current state, react to the msg we got
                if (m_eIOState == tZWaveUSB3S::EIOStates::Idle)
                {
                    //
                    //  See if there's a msg to read. We do a slight timout which
                    //  provides us with some more throttling. If we got something,
                    //  then call a helper to look at the msg and its flags and
                    //  start the process.
                    //
                    if (m_colOutQ.bGetHighestPrio(m_zwomCur, 15))
                        StartNewOutMsg();
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitPingAck)
                {
                    // If we get an ack, then moves to waiting for the transmit ack
                    if (eType == tZWaveUSB3Sh::EMsgTypes::Ack)
                    {
                        m_eIOState = tZWaveUSB3S::EIOStates::WaitPingTransAck;
                        enctLast = TTime::enctNow();

                        // Driver doesn't need to see these
                        bIsSpecial = kCIDLib::True;
                    }
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitPingTransAck)
                {
                    if (eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
                    {
                        if (zwimIn.c1CallbackId() == m_zwomPing.c1CallbackId())
                        {
                            //
                            //  It's for a ping that we sent internally to wake up a
                            //  frequent listener. If successful, we move forward.
                            //  Else, retry if we we can.
                            //
                            if (zwimIn[1] == 0)
                            {
                                //
                                //  OK. If the target is secure we need to get ourself a
                                //  nonce if we don't already have one. Else we send the
                                //  current msg and wait for that.
                                //
                                if (m_zwomCur.bSecure())
                                {
                                    // Do a secure transmit, or ask for a nonce
                                    StartSecureTrans();
                                }
                                 else
                                {
                                    // Send the current msg
                                    m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
                                    TransmitMsg(m_zwomCur);
                                    enctLast = TTime::enctNow();
                                }
                            }
                             else
                            {
                                //
                                //  Use a new callback id since this one failed, and
                                //  delay the resend a bit.
                                //
                                m_enctNextMsg = TTime::enctNowPlusMSs(150);
                                bTryResend(kCIDLib::True, 3);
                            }
                        }

                        // Driver doesn't need to see these
                        bIsSpecial = kCIDLib::True;
                    }
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceAck)
                {
                    // If we get an ack, move forward to waiting for the transmit ack
                    if (eType == tZWaveUSB3Sh::EMsgTypes::Ack)
                    {
                        m_eIOState = tZWaveUSB3S::EIOStates::WaitNonceTransAck;
                        enctLast = TTime::enctNow();

                        // Driver doesn't need to see these
                        bIsSpecial = kCIDLib::True;
                    }
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceTransAck)
                {
                    // If we get a transack, and it's for our nonce msg's callback id
                    if (eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
                    {
                        if (zwimIn.c1CallbackId() == m_zwomNonceReq.c1CallbackId())
                        {
                            // If successful, we move forward. Else, retry if we we can.
                            if (zwimIn[1] == 0)
                            {
                                // Now we are waiting for the nonce
                                m_eIOState = tZWaveUSB3S::EIOStates::WaitNonce;
                                enctLast = TTime::enctNow();
                            }
                             else
                            {
                                //
                                //  Use a new callback id since this one failed, and
                                //  delay the resend a bit.
                                //
                                m_enctNextMsg = TTime::enctNowPlusMSs(150);
                                bTryResend(kCIDLib::True, 3);
                            }
                        }

                        // Driver doesn't need to see these
                        bIsSpecial = kCIDLib::True;
                    }
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonce)
                {
                    //
                    //  We won't actually do anything here. bSpecialMsg() will see the
                    //  nonce and see that we are waiting on it, and will encrypt our
                    //  msg and set us to SendEncrypted state.
                    //
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::SendEncrypted)
                {
                    if (facZWaveUSB3Sh().bMediumTrace())
                        facZWaveUSB3Sh().LogOutMsg(L"Encrypted transmission", m_zwomCur);

                    //
                    //  Let's send our current msg now and move forward. Force previous
                    //  state to send encrypted, since it probably won't get seen in a
                    //  lot of cases.
                    //
                    ePrevState = tZWaveUSB3S::EIOStates::SendEncrypted;
                    m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
                    TransmitMsg(m_zwomCur);
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitAck)
                {
                    if (eType == tZWaveUSB3Sh::EMsgTypes::Ack)
                    {
                        //
                        //  If our current msg needs a callback, then we move to wait
                        //  for that. Else we go back to idle since we are done.
                        //
                        if (m_zwomCur.bNeedsCallback())
                        {
                            m_eIOState = tZWaveUSB3S::EIOStates::WaitCallback;
                            enctLast = TTime::enctNow();
                        }
                         else
                        {
                            // Trigger the driver thread if he's waiting for this one
                            TriggerMsgDone(m_zwomCur.c4AckId(), kCIDLib::True);
                            m_eIOState = tZWaveUSB3S::EIOStates::Idle;
                        }
                    }
                }
                 else if (m_eIOState == tZWaveUSB3S::EIOStates::WaitCallback)
                {
                    // If we get a transack, and it's for our msg's callback id
                    if (eType == tZWaveUSB3Sh::EMsgTypes::TransAck)
                    {
                        if (zwimIn.c1CallbackId() == m_zwomCur.c1CallbackId())
                        {
                            //
                            //  If successful, are done. Else, retry if we we can. Use
                            //  a new callback id on the resend in this case.
                            //
                            if (zwimIn[1] == 0)
                            {
                                //
                                //  If this is a trans ack to a ping that the client
                                //  sent, which it would have to be if were are in
                                //  this state, then we make things more consistent
                                //  by generating a faux ping response so that he can
                                //  wait async for such things.
                                //
                                //  So we update the in msg to represent a CC response
                                //  to a no-op, which doesn't really exist but would
                                //  if ZW wasn't an inconsistent piece of junk. Then
                                //  we turn off the special flag so it gets queued up
                                //  for the driver below.
                                //
                                if (m_zwomCur.bIsPing())
                                {
                                    zwimIn.Reset(5);
                                    zwimIn.m_mbufPayload.PutCard1(0, 0);
                                    zwimIn.m_mbufPayload.PutCard1(m_zwomCur.c1TarId(), 1);
                                    zwimIn.m_mbufPayload.PutCard1(2, 2);
                                    zwimIn.m_mbufPayload.PutCard1(COMMAND_CLASS_NO_OPERATION, 3);
                                    zwimIn.m_mbufPayload.PutCard1(0, 4);
                                    zwimIn.m_c1MsgId = FUNC_ID_APPLICATION_COMMAND_HANDLER;
                                    zwimIn.m_eType = tZWaveUSB3Sh::EMsgTypes::Request;
                                    eType = tZWaveUSB3Sh::EMsgTypes::Request;
                                    bIsSpecial = kCIDLib::False;
                                }

                                //
                                //  Trigger the driver thread if he's waiting for this
                                //  one to be acked.
                                //
                                TriggerMsgDone(m_zwomCur.c4AckId(), kCIDLib::True);
                                m_eIOState = tZWaveUSB3S::EIOStates::Idle;
                            }
                             else
                            {
                                //
                                //  If we can resend, don't let the driver see the failure
                                //  since we may still get there. Delay the resend a
                                //  bit.
                                //
                                m_enctNextMsg = TTime::enctNowPlusMSs(150);
                                bIsSpecial = bTryResend(kCIDLib::True, 3);
                            }
                        }
                    }
                }
                 else
                {
                    // Just for debugging
                }

                // If we changed state do some things
                if (ePrevState != m_eIOState)
                {
                    if (facZWaveUSB3Sh().bTraceEnabled())
                    {
                        facZWaveUSB3Sh().LogTraceMsg
                        (
                            L"Transitioned from %(1) to %(2) state"
                            , tZWaveUSB3S::strXlatEIOStates(ePrevState)
                            , tZWaveUSB3S::strXlatEIOStates(m_eIOState)
                        );
                    }
                }

                //
                //  If not marked special, then put it in the queue since the driver
                //  may be interested.
                //
                if (!bIsSpecial)
                {
                    m_colInQ.CheckIsFull(ZWaveUSB3_ZStick::c4InQSz, L"Z-Stick input queue");
                    m_colInQ.objAdd(zwimIn);
                }
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            // Format our some info
            TString strFailInfo(L"State: ", 256UL);
            strFailInfo.Append(tZWaveUSB3S::strXlatEIOStates(m_eIOState));
            strFailInfo.Append(L", MsgId: ");
            strFailInfo.AppendFormatted(m_zwomCur.c1MsgId());
            strFailInfo.Append(L", TarId: ");
            strFailInfo.AppendFormatted(m_zwomCur.c1TarId());
            strFailInfo.Append(L", MsgBytes: ");
            strFailInfo.AppendFormatted(m_zwomCur.c4MsgBytes());
            if (m_zwomCur.bIsCCMsg())
            {
                strFailInfo.Append(L", Out CC/Cmd: 0x");
                strFailInfo.AppendFormatted(m_zwomCur.c1ClassId(), tCIDLib::ERadices::Hex);
                strFailInfo.Append(kCIDLib::chForwardSlash);
                strFailInfo.AppendFormatted(m_zwomCur.c1CmdId());
                strFailInfo.Append(L", CCBytes: ");
                strFailInfo.AppendFormatted(m_zwomCur.c4CCBytes());
            }

            facZWaveUSB3S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3Errs::errcDbg_ExceptInIOThread
                , strFailInfo
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );

            TLocker lockrSync(&m_mtxSync);

            // Assume the worst and reset everything
            m_eIOState = tZWaveUSB3S::EIOStates::Idle;
            m_c4CANQUsed = 0;
            m_colCANQ.RemoveAll();
            m_colInQ.RemoveAll();
            m_colInNonces.RemoveAll();
            m_colOutNonces.RemoveAll();
            m_colOutQ.RemoveAll();
            m_evWaitAck.Trigger();

            //
            //  If a com error try to cycle the serial port. If that works, keep going.
            //  Else we give up and let the driver start reconnecting us. If we keep
            //  going we stay in the same state. That will let us retry the previous msg
            //  again
            //
            if (errToCatch.strFacName() == facCIDComm().strName())
            {
                facZWaveUSB3Sh().LogTraceErr(L"Recycling serial port...");

                try
                {
                    m_pcommZWave->Cycle(tCIDComm::EOpenFlags::WriteThrough, 1024, 1024);

                    //
                    //  Try to send a nak to see if we can write to it. If this fails,
                    //  we will close the port below.
                    //
                    tCIDLib::TCard1 c1Send = NAK;
                    if (m_pcommZWave->c4WriteRawBufMS(&c1Send, 1, 500) != 1)
                    {
                        facZWaveUSB3S().ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kZW3Errs::errcComm_StillBadAfterCycle
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::LostConnection
                        );
                    }
                }

                catch(TError& errToCatch)
                {
                    // Oh well, give up
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facZWaveUSB3Sh().LogTraceErr(L"Recycle failed, closing port");
                    try
                    {
                        m_pcommZWave->Close();
                        delete m_pcommZWave;
                        m_pcommZWave = nullptr;
                    }

                    catch(TError& errToCatch)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    // And break out to stop this thread and start the reconnect
                    break;
                }
            }
        }

        catch(...)
        {
            facZWaveUSB3S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3Errs::errcDbg_ExceptInIOThread
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


// Return a timeout for a given I/O thread state, adjusting a bit if a frequent listener
tCIDLib::TEncodedTime
TZStick::enctStateTimeout(  const   tZWaveUSB3S::EIOStates  eIOState
                            , const tCIDLib::TBoolean       bFreqListener) const
{
    CIDAssert
    (
        (eIOState > tZWaveUSB3S::EIOStates::Idle) && (eIOState < tZWaveUSB3S::EIOStates::Count)
        , L"Invalid I/O State to get timeout for"
    );

    tCIDLib::TEncodedTime enctRet = m_aenctStateTO[eIOState];
    if (bFreqListener)
        enctRet += kCIDLib::enctTwoSeconds;

    return enctRet;
}



//
//  Translates our driver specific msg priority to the underlying CIDLib queue priority
//  that will be used to actually queue up the msgs.
//
tCIDLib::EQPrios TZStick::eXlatPriority(const tZWaveUSB3Sh::EMsgPrios eToXlat) const
{
    tCIDLib::EQPrios eRet;
    switch(eToXlat)
    {
        case tZWaveUSB3Sh::EMsgPrios::Async :
            eRet = tCIDLib::EQPrios::P0;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Query :
            eRet = tCIDLib::EQPrios::P2;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Command :
            eRet = tCIDLib::EQPrios::P3;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Reply :
            eRet = tCIDLib::EQPrios::P5;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Wakeup :
            eRet = tCIDLib::EQPrios::P6;
            break;

        case tZWaveUSB3Sh::EMsgPrios::SpecialCmd :
            eRet = tCIDLib::EQPrios::P7;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Security :
            eRet = tCIDLib::EQPrios::P8;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Nonce :
            eRet = tCIDLib::EQPrios::P9;
            break;

        case tZWaveUSB3Sh::EMsgPrios::Local :
            eRet = tCIDLib::EQPrios::P10;
            break;

        default :
            // Shouldn't happen, but just in case
            eRet = tCIDLib::EQPrios::P2;
            break;
    };
    return eRet;
}


//
//  Creates a new incoming msg nonce and stores it in our list. This is called when
//  we need to issue a nonce to a remote node so that he can send us back a msg. It
//  needs to be kept around in order to validate the eventual reply.
//
TZStick::TNonceInfo* TZStick::pniMakeNewInNonce(const tCIDLib::TCard1 c1UnitId)
{
    // Flush any out of date nonces
    {
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        tCIDLib::TCard4 c4Count = m_colInNonces.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            const TNonceInfo& niCur = m_colInNonces[c4Index];
            if (niCur.m_enctValidTill <= enctCur)
            {
                if (facZWaveUSB3Sh().bMediumTrace())
                {
                    facZWaveUSB3Sh().LogTraceMsg
                    (
                        L"Cleaned up unused In nonce. Id=%(1)", TCardinal(niCur.m_mbufNonce[0])
                    );
                }

                m_colInNonces.RemoveAt(c4Index);
                c4Count--;
            }
             else
            {
                c4Index++;
            }
        }
    }

    //
    //  If we still don't have any space, then something is wrong, so just flush it.
    //  This is going to cause messages to fail that might be using them, but we have
    //  to get back to sanity.
    //
    if (m_colInNonces.bIsFull(32))
    {
        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcDbg_NonceListFull
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::OutResource
        );
        m_colInNonces.RemoveAll();
    }

    // Add the new one
    TNonceInfo& niNew = m_colInNonces.objPlace(c1UnitId, m_c1NextNonceId++);

    // Don't ever use zero or 0xFF for the next nonce id
    if (m_c1NextNonceId == 0xFF)
        m_c1NextNonceId = 1;

    return &niNew;
}


// Returns the out msg for the current state. Throws if there is not one
TZWOutMsg* TZStick::pzwomForState()
{
    if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitPingAck)
    ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitPingTransAck))
    {
        return &m_zwomPing;
    }
     else if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonceTransAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitNonce))
    {
        return &m_zwomNonceReq;
    }
     else if ((m_eIOState == tZWaveUSB3S::EIOStates::WaitAck)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::WaitCallback)
          ||  (m_eIOState == tZWaveUSB3S::EIOStates::SendEncrypted))
    {
        return &m_zwomCur;
    }
    return nullptr;
}


//
//  We need to create a new nonce for our node, and send to to the return node. It will
//  be stored away to use when the reply comes back.
//
//  If we are currently idle, we send it right now, else we queue it to be sent, at
//  highest priority, since nonces have limited lifetime.
//
tCIDLib::TVoid TZStick::SendNonceReport(const tCIDLib::TCard1 c1RetId)
{
    TNonceInfo* pniNew = pniMakeNewInNonce(c1RetId);

    TZWOutMsg zwomRep;
    zwomRep.Reset
    (
        c1RetId
        , COMMAND_CLASS_SECURITY
        , SECURITY_NONCE_REPORT
        , 0
        , 0
        , tZWaveUSB3Sh::EMsgPrios::Nonce
    );
    zwomRep.AppendBytes(pniNew->m_mbufNonce, 8);
    zwomRep.EndCounter();
    zwomRep.AppendTransOpts(kCIDLib::True);
    zwomRep.AppendCallback();
    zwomRep.Finalize(kCIDLib::False, kCIDLib::False);

    if (m_eIOState == tZWaveUSB3S::EIOStates::Idle)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"SEC: Idle, sending nonce %(1) to node %(2) immediately"
                , TCardinal(pniNew->m_mbufNonce[0], tCIDLib::ERadices::Hex)
                , TCardinal(c1RetId, tCIDLib::ERadices::Hex)
            );
        }

        m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
        m_zwomCur = zwomRep;
        TransmitMsg(m_zwomCur);
    }
     else
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"SEC: Queued outgoing nonce %(1) for node %(2)"
                , TCardinal(pniNew->m_mbufNonce[0])
                , TCardinal(c1RetId, tCIDLib::ERadices::Hex)
            );
        }

        // Queue it up to be sent. Note that we queue it at nonce priority
        QueueOutMsg(zwomRep);
    }
}


//
//  When the processing thread gets a new msg out of the outgoing queue, it puts it
//  it in m_zwomCur and calls us here to look at it and start the process of sending
//  it.
//
//  NOTE THAT we set the new state before calling TransmitMsg since he will push us
//  back to idle state if he cannot transmit it.
//
tCIDLib::TVoid TZStick::StartNewOutMsg()
{
    if (facZWaveUSB3Sh().bTraceEnabled())
        facZWaveUSB3Sh().LogOutMsg(L"Sending a new msg:", m_zwomCur);

    //
    //  1. If it's to a frequent listener, the first thing is to ping it, if we need to.
    //  2. If it is a secure msg we see if we have nonce for it. If so we encrypt it
    //      and move to WaitAck state. Else we start the process of getting a nonce
    //      and go to WaitNonceAck state.
    //  3. Else nothing special we send it
    //
    const tCIDLib::TCard1 c1TarId = m_zwomCur.c1TarId();
    tCIDLib::TBoolean bWasPinged = kCIDLib::False;
    if (m_zwomCur.bFreqListener())
    {
        //
        //  If not a broadcast target and the msg itself is not a ping, then send
        //  it a ping to make sure it's still awake for us. The client will also
        //  sometimes send pings and we don't want to get things confused.
        //
        if ((c1TarId != NODE_BROADCAST) && !m_zwomCur.bIsPing())
        {
            //
            //  Only do this if we haven't sent a ping to this unit very recently.
            //
            const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
            if (m_fcolLastPing[c1TarId] + kCIDLib::enctOnePtFiveSec < enctNow)
            {
                m_fcolLastPing[c1TarId] = TTime::enctNow();

                if (facZWaveUSB3Sh().bHighTrace())
                    facZWaveUSB3Sh().LogTraceMsg(L"Target is frequent listener, sending ping");

                //
                //  We have to set this one upon specially since it's completely
                //  inconsistent and so our standard stuff doesn't work.
                //
                m_zwomPing.MakePing(c1TarId, tZWaveUSB3Sh::EMsgPrios::Command);

                //
                //  Set our state first. If this fails, he'll force it back to idle since
                //  we cannot process this one.
                //
                m_eIOState = tZWaveUSB3S::EIOStates::WaitPingAck;
                TransmitMsg(m_zwomPing);
                bWasPinged = kCIDLib::True;
            }
             else
            {
                if (facZWaveUSB3Sh().bHighTrace())
                    facZWaveUSB3Sh().LogTraceMsg(L"Ping sent very recently, not sending another");
            }
        }
         else
        {
            if (facZWaveUSB3Sh().bHighTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Target is broadcast, no ping needed");
        }
    }

    // If we didn't have to do the ping, do the normal thing
    if (!bWasPinged)
    {
        if (m_zwomCur.bSecure())
        {
            StartSecureTrans();
        }
         else
        {
            // Nothing special just send and we are waiting for the ack
            m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
            TransmitMsg(m_zwomCur);
        }
    }
}


//
//  When we need to send a secure msg, this is called. We see if we have a nonce already.
//  If so we encrypt and send. Else we start the process of getting a nonce.
//
//  AS ALWAYS we set the state first. If the transmission fails, it puts us back to idle
//  state.
//
tCIDLib::TVoid TZStick::StartSecureTrans()
{
    //
    //  If whe have a still valid nonce for this target, we can go ahead and encrypt
    //  and send it. Else we need to go through the steps to get a nonce.
    //
    const tCIDLib::TCard1 c1TarId = m_zwomCur.c1TarId();
    if (bFindOutNonce(c1TarId, m_mbufNonce))
    {
        // Get a copy of the nonce, remove it, then encrypt
        THeapBuf mbufNonce(m_mbufNonce);
        m_zwomCur.Encrypt(m_mbufNonce);

        // Send it and we are waiting for the basic ack now
        m_eIOState = tZWaveUSB3S::EIOStates::WaitAck;
        TransmitMsg(m_zwomNonceReq);
    }
     else
    {
        // Build a nonce request and send that
        BuildNonceRequest(c1TarId, m_zwomNonceReq);
        m_eIOState = tZWaveUSB3S::EIOStates::WaitNonceAck;
        TransmitMsg(m_zwomNonceReq);
    }
}


//
//  Stores a newly received output msg nonce to our list. Just to be safe we see if
//  any existing nonce in the list is for the same unit/nonce id combo and reuse it if
//  so. We also remove any out of date ones.
//
tCIDLib::TVoid
TZStick::StoreOutNonce( const   tCIDLib::TCard1 c1UnitId
                        , const THeapBuf&       mbufNonce)
{
    const tCIDLib::TCard1 c1NonceId = mbufNonce[0];
    if (facZWaveUSB3Sh().bMediumTrace())
    {
        facZWaveUSB3Sh().LogTraceMsg
        (
            L"Storing out nonce for later use. Unit=%(1), Nonce=%(2)"
            , TCardinal(c1UnitId)
            , TCardinal(c1NonceId)
        );
    }

    // Remove any out of date ones first
    tCIDLib::TCard4 c4Count = m_colOutNonces.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    while (c4Index < c4Count)
    {
        TNonceInfo& niCur = m_colOutNonces[c4Index];
        if (niCur.m_enctValidTill < enctNow)
        {
            m_colOutNonces.RemoveAt(c4Index);
            c4Count--;
        }
         else
        {
            c4Index++;
        }
    }

    //
    //  Of the ones left see if we already have this nonce/unit id combo. If so, let's
    //  reuse it.
    //
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TNonceInfo& niCur = m_colOutNonces[c4Index];
        if ((niCur.m_mbufNonce[0] == c1NonceId) && (niCur.m_c1UnitId == c1UnitId))
        {
            niCur.m_enctValidTill = TTime::enctNowPlusSecs(kZWaveUSB3Sh::c4OutNonceValidSecs);
            niCur.m_mbufNonce.CopyIn(mbufNonce, 8);
            return;
        }
    }

    // We need to add a new one
    m_colOutNonces.objPlace(c1UnitId, mbufNonce);
}


//
//  Try to transmit the current outgoing message. If it fails, we just clear the active
//  out msg and this one is dropped. That should only happen in dire circumstances since
//  this is a serial connection.
//
tCIDLib::TVoid TZStick::TransmitMsg(TZWOutMsg& zwomTar)
{
    // We are going to be able to send it, so calc the checksum
    const tCIDLib::TCard1 c1Sum = c1CalcSum(zwomTar.m_mbufData);

    if (facZWaveUSB3Sh().bTraceEnabled())
    {
        facZWaveUSB3Sh().LogTraceMsg
        (
            L"Sending msg, AckId=%(1), Attempt #%(2)"
            , TCardinal(zwomTar.c4AckId())
            , TCardinal(zwomTar.m_c4SendCount + 1)
        );
    }

    // Bump the send count now that we are about to send this guy
    zwomTar.m_c4SendCount++;

    // If the minimium time hasn't ellapsed, wait
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur < m_enctNextMsg)
        TThread::Sleep(tCIDLib::TCard4(m_enctNextMsg - enctCur) / kCIDLib::enctOneMilliSec);

    // Send the SOF, the msg bytes, and the check sum
    const TMemBuf& mbufToSend = zwomTar.m_mbufData;
    tCIDLib::TCard1 c1Byte = SOF;
    if ((m_pcommZWave->c4WriteRawBufMS(&c1Byte, 1, 100) == 1)
    &&  (m_pcommZWave->c4WriteMBufMS(mbufToSend, mbufToSend[0], 250) == mbufToSend[0])
    &&  (m_pcommZWave->c4WriteRawBufMS(&c1Sum, 1, 100) == 1))
    {
        //
        //  Don't delay the next message since we send ok and it's all part of the normal
        //  flow.
        //
        m_enctNextMsg = TTime::enctNow();
    }
     else
    {
        if (facZWaveUSB3Sh().bMediumTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Message could not be transmitted, dropping it\n");
        m_eIOState = tZWaveUSB3S::EIOStates::Idle;

        // Trigger the driver thread if he's waiting for this guy
        TriggerMsgDone(zwomTar.c4AckId(), kCIDLib::False);

        // Delay the next message a bit
        m_enctNextMsg = TTime::enctNowPlusMSs(150);
    }
}


//
//  Called when a msg transmission is done and acked, or when it fails. If the passed id
//  is set in m_c4WaitAckId, then we store the result and trigger the wait event. Else
//  no one is waiting so nothing to do.
//
tCIDLib::TVoid
TZStick::TriggerMsgDone(const tCIDLib::TCard4 c4AckId, const tCIDLib::TBoolean bRes)
{
    // Check first, then lock and check again, for efficiency
    if (m_c4WaitAckId)
    {
        TLocker lockrSync(&m_mtxSync);
        if (m_c4WaitAckId)
        {
            m_bWaitAckRes = bRes;
            m_evWaitAck.Trigger();
        }
    }
}
