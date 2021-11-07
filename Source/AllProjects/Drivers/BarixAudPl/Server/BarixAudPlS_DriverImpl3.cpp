//
// FILE NAME: BarixAudPlS_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/07/2007
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
//  This is a spillover file of the main driver class. In this file we have
//  all the stuff directly related to the spooling thread, which is the bulk
//  of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "BarixAudPlS.hpp"



// ---------------------------------------------------------------------------
//   CLASS: BarixAudPlS::TBufInfo
//  PREFIX: bi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  BarixAudPlS::TBufInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TBarixAudPlSDriver::TBufInfo::TBufInfo() :

    m_bLastBuf(kCIDLib::False)
    , m_c4SpoolBufInd(0)
    , m_c4SpoolBufSz(0)
    , m_eState(EBufStates::Empty)
    , m_mbufData
      (
        kBarixAudPlS::c4SpoolBufSz + 8192
        , kBarixAudPlS::c4SpoolBufSz * 2
      )
    , m_pdaedSrc(0)
{
}

TBarixAudPlSDriver::TBufInfo::~TBufInfo()
{
}


// ---------------------------------------------------------------------------
//  BarixAudPlS::TBufInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Indicates if we have used up all our bytes or not
tCIDLib::TBoolean TBarixAudPlSDriver::TBufInfo::bIsEmpty()
{
    return (m_eState == EBufStates::Empty);
}


// Returns the last buffer flag
tCIDLib::TBoolean TBarixAudPlSDriver::TBufInfo::bLastBuffer()
{
    return m_bLastBuf;
}


//
//  Load ourself up from the decoder that is set on us. We set the last
//  buffer member if we hit the end of the data source, and update our
//  size/index info to reflect the amount of data available. As a convenince
//  we also return the last buffer status, i.e. end of stream.
//
//  We cannot control the size of the chunks we get back from the codec,
//  so we have to be able to deal with going over our nominal buffer size.
//  They can grow to twice the size we stop loading them at. We'd be in
//  trouble if we got a really big chunk over the nominal size when it was
//  just below nominal size on the last round, but that doesn't seem to be
//  possible based on imperical evidence so far.
//
//  The buffer state should already be set to Loading before this is called
//  since this is normally called by the loader thread who is getting queued
//  up buffers to fill.
//
tCIDLib::TBoolean TBarixAudPlSDriver::TBufInfo::bLoadFromSrc()
{
    m_c4SpoolBufInd = 0;
    m_c4SpoolBufSz = 0;

    //
    //  Loop until we hit the end of source or fill up the buffer. We
    //  can go past the buffer size because we don't control how much
    //  we'll get back on each round, but the buffer has enough expansion
    //  size to cover it.
    //
    tCIDLib::TCard4 c4ThisTime = 0;
    while (m_c4SpoolBufSz < kBarixAudPlS::c4SpoolBufSz)
    {
        // And now decode another chunk
        try
        {
            c4ThisTime = m_pdaedSrc->c4LoadChunk
            (
                m_mbufData, m_c4SpoolBufSz
            );

            // If no data, then we hit the end of source, so we are done
            if (!c4ThisTime)
            {
                m_pdaedSrc->EndDecode();
                m_bLastBuf = kCIDLib::True;
                break;
            }
        }

        //
        //  If we get an exception, we end the stream by reporting this
        //  as the end of stream buffer and ending the decode.
        //
        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facBarixAudPlS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kBrxPlErrs::errcInt_DecodeErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );

            try
            {
                m_pdaedSrc->EndDecode();
            }

            catch(...)
            {
            }

            m_c4SpoolBufInd = 0;
            m_c4SpoolBufSz = 0;
            m_bLastBuf = kCIDLib::True;
            break;
        }

        catch(...)
        {
            facBarixAudPlS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kBrxPlErrs::errcInt_DecodeErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );

            try
            {
                m_pdaedSrc->EndDecode();
            }

            catch(...)
            {
            }

            m_c4SpoolBufInd = 0;
            m_c4SpoolBufSz = 0;
            m_bLastBuf = kCIDLib::True;
            break;
        }

        m_c4SpoolBufSz += c4ThisTime;
    }

    //
    //  Set ourselves to loaded status now if we got some bytes. This has
    //  to be the last thing we do because as soon as we do this, the
    //  spooling thread can grab the buffer. So get the return value out
    //  first adn tehn set the state.
    //
    tCIDLib::TBoolean bRet = m_bLastBuf;
    if (m_c4SpoolBufSz)
        m_eState = EBufStates::Ready;
    else
        m_eState = EBufStates::Empty;

    return bRet;
}


tCIDLib::TCard4 TBarixAudPlSDriver::TBufInfo::c4BytesLeft() const
{
    return m_c4SpoolBufSz - m_c4SpoolBufInd;
}


tCIDLib::TCard4
TBarixAudPlSDriver::TBufInfo::c4IncIndex(const tCIDLib::TCard4 c4By)
{
    m_c4SpoolBufInd += c4By;
    return m_c4SpoolBufInd;
}


tCIDLib::TVoid
TBarixAudPlSDriver::TBufInfo::CopyOut(          tCIDLib::TCard1* const  pc1Target
                                        , const tCIDLib::TCard4         c4Count)
{
    m_mbufData.CopyOut(pc1Target, c4Count, m_c4SpoolBufInd);
    m_c4SpoolBufInd += c4Count;

    // If we are used up, then set ourselves empty
    if (m_c4SpoolBufInd == m_c4SpoolBufSz)
        m_eState = EBufStates::Empty;
}


TBarixAudPlSDriver::EBufStates TBarixAudPlSDriver::TBufInfo::eState() const
{
    return m_eState;
}


tCIDLib::TVoid TBarixAudPlSDriver::TBufInfo::Reset(const EBufStates eState)
{
    m_bLastBuf = kCIDLib::False;
    m_c4SpoolBufInd = 0;
    m_c4SpoolBufSz = 0;
    m_eState = eState;
}


tCIDLib::TVoid
TBarixAudPlSDriver::TBufInfo::SetDecoder(TCIDDAEDecoder* const pdaedSrc)
{
    m_pdaedSrc = pdaedSrc;
}




// ---------------------------------------------------------------------------
//  TBarixAudPlSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called to load up a new RTP payload. We get the buffer
//  list and we pull out the required data, moving forward to the next
//  buffer if we don't have enough in the current one.
//
//  We update index of the buffer that should be used next. So if we
//  use up the current one, we set it to the next one.
//
//  This is necessary because we cannot control the size of the buffers
//  we get back when we decode data from a codec. So the buffers won't be
//  the same size as the RTP buffers we spit out, or even an multiple of
//  them.
//
tCIDLib::TBoolean
TBarixAudPlSDriver::bExtractPacketData( TRTPPacket&             RTPHdr
                                        , TSpoolBufList&        colSpool
                                        , tCIDLib::TCard4&      c4CurIndex
                                        , tCIDLib::TBoolean&    bEndOfSrc)
{
    // Assume not the end until proven otherwise
    bEndOfSrc = kCIDLib::False;

    //
    //  Loop forward through buffers until we get our payload, or we get
    //  back to the starting buffer again without enough bytes.
    //
    const tCIDLib::TCard4 c4OrgIndex = c4CurIndex;
    tCIDLib::TCard4 c4SoFar = 0;
    do
    {
        // Get the current buffer and the bytes it still has left
        TBufInfo& biCur = *colSpool[c4CurIndex];
        const tCIDLib::TCard4 c4CurBytes = biCur.c4BytesLeft();

        // If any, then take some from it
        if (c4CurBytes)
        {
            tCIDLib::TCard4 c4LoadCnt
            (
                tCIDLib::MinVal(c4CurBytes, kBarixAudPlS::c4PayloadSz - c4SoFar)
            );
            biCur.CopyOut(&RTPHdr.ac1Buf[c4SoFar], c4LoadCnt);
            c4SoFar += c4LoadCnt;
        }

        //
        //  If we used up this buffer, then reset it back to empty state.
        //  If it's the last buffer from the current source, then we need
        //  to give up.
        //
        if (biCur.bIsEmpty())
        {
            // IF the last buffer, then set the end of source flag
            if (biCur.bLastBuffer())
                bEndOfSrc = kCIDLib::True;

            // Now reset it for reuse
            biCur.Reset(EBufStates::Empty);

            //
            //  If we just happened to hit right on the boundary and are
            //  going to break out before we bump the index below, then do
            //  that now since this one is no good anymore.
            //
            if (c4SoFar == kBarixAudPlS::c4PayloadSz)
            {
                c4CurIndex++;
                if (c4CurIndex == kBarixAudPlS::c4SpoolBufCnt)
                    c4CurIndex = 0;
            }
        }

        // If we got the bytes we need, then break out
        if (c4SoFar == kBarixAudPlS::c4PayloadSz)
            break;

        // If it was the last buffer, don't try any more
        if (bEndOfSrc)
            break;

        //
        //  Not enough bytes yet, so bump the index and try again, dealing
        //  with buffer index wraparound.
        //
        c4CurIndex++;
        if (c4CurIndex == kBarixAudPlS::c4SpoolBufCnt)
            c4CurIndex = 0;

    }   while (c4CurIndex != c4OrgIndex);

    //
    //  If we've still not gotten enough, but we got some, then zero fill
    //  the rest of the payload.
    //
    if (c4SoFar)
    {
        if (c4SoFar < kBarixAudPlS::c4PayloadSz)
        TRawMem::SetMemBuf
        (
            &RTPHdr.ac1Buf[c4SoFar]
            , kCIDLib::c1MinCard
            , kBarixAudPlS::c4PayloadSz - c4SoFar
        );
    }
    return (c4SoFar != 0);
}


//
//  This is called on starting a new file, or starting back up after a
//  Stop command, to preload some of the buffers before we start streaming.
//  This way, we know we've got enough to get the streaming going, even if
//  there's some delay getting the background loading up and running, but
//  without spending too much time up front.
//
//  We return true if we hit end of source. This is highly unlikely but we
//  have to deal with it gracefully if it were to happen.
//
tCIDLib::TBoolean
TBarixAudPlSDriver::bPreloadSpoolBufs(  TCIDDAEWMADec&      daedSrc
                                        , TSpoolBufList&    colSpoolBufs)
{
    // Clear out any previous content that might have gotten left
    ClearSpoolBufs(colSpoolBufs);

    // We'll do have of them up front
    const tCIDLib::TCard4 c4Count = kBarixAudPlS::c4SpoolBufCnt / 2;

    tCIDLib::TBoolean bEndOfSrc = kCIDLib::False;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TBufInfo& biCur = *colSpoolBufs[c4Index];
        if (biCur.bLoadFromSrc())
            break;
    }

    //
    //  No idea why this is required, but it is. Any time we are starting a
    //  new sequence, if we start spitting out data on the 0th byte, then
    //  we get white noise, indicating that we are off by a byte in the
    //  stream. But it is only required for the first packet we send.
    //
    if (!colSpoolBufs[0]->bIsEmpty())
        colSpoolBufs[0]->c4IncIndex(1);

    return bEndOfSrc;
}


// This is called to reset all of the spool buffers back to empty state.
tCIDLib::TVoid TBarixAudPlSDriver::ClearSpoolBufs(TSpoolBufList& colBufs)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kBarixAudPlS::c4SpoolBufCnt; c4Index++)
        colBufs[c4Index]->Reset(EBufStates::Empty);
}


//
//  This method is started up as our loading thread. We just watch for
//  buffers to show up in the buffer queue, and we load them up. When we
//  run out of data from the provdied decoder, we set a last buffer flag
//  on that buffer.
//
tCIDLib::EExitCodes
TBarixAudPlSDriver::eLoaderThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    //
    //  Preload an error object that we log if we get an unknown exception
    //  in our loop below, to avoid creating it again and again if errors
    //  are occuring.
    //
    TError errFailureInLoop
    (
        facBarixAudPlS().strName()
        , CID_FILE
        , CID_LINE
        , kBrxPlErrs::errcInt_UnknownLoadExcpt
        , facBarixAudPlS().strMsg(kBrxPlErrs::errcInt_UnknownLoadExcpt)
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Unknown
    );

    //
    //  Raise our priority so that we aren't interrupted by more mundane
    //  things. We have fairly tight tolerances for keeping ahead of the
    //  Barix.
    //
    thrThis.SetPriority(tCIDLib::EPrioLevels::Highest);

    while (kCIDLib::True)
    {
        try
        {
            // Watch for shutdown requests
            if (thrThis.bCheckShutdownRequest())
                break;

            // Wait a while for a buffer to load
            TBufInfo* pbiLoad = m_colBufQ.pobjGetNext(100, kCIDLib::False);
            if (!pbiLoad)
                continue;

            //
            //  We got one, so load it up. Here, we don't care about the
            //  the return value. If it was the last buffer, then that
            //  flag is set on this buffer and the spooler thread will
            //  see it.
            //
            try
            {
                pbiLoad->bLoadFromSrc();
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                // Release the buffer and then rethrow
                pbiLoad->Reset(EBufStates::Empty);
                throw;
            }
        }

        //
        //  Note that, since we are a high priority thread, we sleep
        //  after an exception. Should we in some horrible circumstance
        //  get caught in a loop where we are continously getting errors,
        //  we don't want to eat the whole machine alive.
        //
        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::High);
            TThread::Sleep(100);
        }

        catch(...)
        {
            LogError(errFailureInLoop, tCQCKit::EVerboseLvls::High);
            TThread::Sleep(100);
        }
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  The entry point for the spooling thread. This guy does two things.
//
//  - It checks a queue of buffers to see if any are empty. If so, it hands them off
//  to the loader thread above, by putting them into a queue he monitors. The loader
//  thread will load up the buffers and put them back into a queue for spooling out.
//
//  - If any buffers are full, he grabs the next one and spools it out to the
//  ExStreamer.
//
//  So this guy both spools and drives the loading of buffers, but he doesn't have to
//  block in order to get the buffers filled up. The loader thread does that.
//
//  NOTE: We don't have to lock to READ the current media/end states, since we are
//  the only one that modifies them. We only ahev to lock to update them, and we
//  have a helper method that we call to set the states.
//
tCIDLib::EExitCodes
TBarixAudPlSDriver::eSpoolThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    // The datagram socket that we use to send out our RTP datagrams
    TClientDatagramSocket sockStream;
    try
    {
        sockStream.Open(tCIDSock::ESockProtos::UDP, m_ipepStream.eAddrType());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
        return tCIDLib::EExitCodes::RuntimeError;
    }

    //
    //  Set up some parts of the RTP header that never change. We'll just
    //  use this same header over and over.
    //
    TRTPPacket RTPHdr = {0};
    RTPHdr.c1Start = 0x80;
    #if defined(CIDLIB_LITTLEENDIAN)
    RTPHdr.c4SSrc = TRawBits::c4SwapBytes(1);
    #else
    RTPHdr.c4SSrc = 1;
    #endif

    //
    //  Preload an error object that we log if we get an unknown exception
    //  in our loop below, to avoid creating it again and again if errors
    //  are occuring.
    //
    TError errFailureInLoop
    (
        facBarixAudPlS().strName()
        , CID_FILE
        , CID_LINE
        , kBrxPlErrs::errcInt_UnknownSpoolExcpt
        , facBarixAudPlS().strMsg(kBrxPlErrs::errcInt_UnknownSpoolExcpt)
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Unknown
    );

    //
    //  Set up an array of buffer info objects. We will round robin through
    //  these, loading them up as the outgoing spooling chases behind us. We
    //  preload the list, since we are treating it like an array here, but couldn't
    //  use an array since it requires copyable elements and these aren't.
    //
    TSpoolBufList colSpoolBufs(tCIDLib::EAdoptOpts::Adopt, kBarixAudPlS::c4SpoolBufCnt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kBarixAudPlS::c4SpoolBufCnt; c4Index++)
        colSpoolBufs.Add(new TBufInfo());

    //
    //  These are used to put out sequential sequence humber and time values
    //  into the RTP header and to keep up with when it's time to send the
    //  next buffer. We keep up with the time that the next packet should be
    //  sent, and we use that to wait until it's time.
    //
    tCIDLib::TCard2         c2SeqNum = 0;
    tCIDLib::TCard4         c4TimeSeq = 0;
    tCIDLib::TEncodedTime   enctNext = 0;

    //
    //  And we need a codec that we'll use to decode chunks of data out
    //  of the source file.
    //
    TCIDDAEWMADec daedSrc;

    //
    //  For now, since we only have one decoder, we'll pre-set the decode
    //  on each of the spool buffers. Later we have to change this if we
    //  select a different decoder based on source file type.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kBarixAudPlS::c4SpoolBufCnt; c4Index++)
        colSpoolBufs[c4Index]->SetDecoder(&daedSrc);

    //
    //  Raise our priority so that we aren't interrupted by more mundane
    //  things. We have fairly tight tolerances for keeping ahead of the
    //  Barix.
    //
    thrThis.SetPriority(tCIDLib::EPrioLevels::Highest);

    tCIDLib::TBoolean       bDone = kCIDLib::False;
    tCIDLib::TBoolean       bEndOfSrc = kCIDLib::False;
    tCIDLib::TCard4         c4CurBufInd = 0;
    TCQCAudioPlayer::TQElem cptrCmd;
    TString                 strCurSrcFile;
    while(!bDone)
    {
        try
        {
            // Watch for shutdown/sync requests
            if (thrThis.bCheckShutdownRequest())
                break;

            //
            //  Check for a command. According to what mode we are in, we'll
            //  either wait for a while or not. If we are not currently playing
            //  anything, we'll use this to throttle ourself back. Else that
            //  is done below in the spooling process.
            //
            tCIDLib::TCard4 c4WaitCmd = 0;
            if ((m_eEndState != tCQCMedia::EEndStates::Running)
            ||  (m_eMediaState == tCQCMedia::EMediaStates::Stopped)
            ||  (m_eMediaState == tCQCMedia::EMediaStates::Paused))
            {
                c4WaitCmd = 100;
            }

            if (m_colCmdQ.bGetNext(cptrCmd, c4WaitCmd))
            {
                try
                {
                    // We got one, so process it
                    switch(cptrCmd->m_eType)
                    {
                        case TCQCAudioPlayer::EEvTypes::Load :
                        {
                            // Start the decoder on this new file
                            daedSrc.EndDecode();
                            daedSrc.StartDecode(*cptrCmd->m_pstrVal1);
                            strCurSrcFile = *cptrCmd->m_pstrVal1;

                            // Preload all the buffers
                            bEndOfSrc = bPreloadSpoolBufs(daedSrc, colSpoolBufs);
                            if (!bEndOfSrc)
                            {
                                c4CurBufInd = 0;

                                c2SeqNum = 0;
                                c4TimeSeq = 0;
                                bEndOfSrc = kCIDLib::False;
                                enctNext = 0;
                                m_c4PacketsSent = 0;

                                // Update the satus
                                StoreSpoolState
                                (
                                    tCQCMedia::EMediaStates::Playing
                                    , tCQCMedia::EEndStates::Running
                                );
                            }
                            break;
                        }

                        case TCQCAudioPlayer::EEvTypes::Pause :
                        {
                            //
                            //  Reset the transmission control stuff. When
                            //  we are let go again, we'll start sending a
                            //  new sequence to the Barix as far as he is
                            //  concerned. We don't clear the buffers
                            //  sent count since we want it to pick back up
                            //  when we start again. Any data in our buffers
                            //  is left alone to be started on again when
                            //  we start back up.
                            //
                            c2SeqNum = 0;
                            c4TimeSeq = 0;
                            enctNext = 0;
                            StoreSpoolState(tCQCMedia::EMediaStates::Paused);
                            break;
                        }

                        case TCQCAudioPlayer::EEvTypes::Play :
                        {
                            //
                            //  Set the time to now so that we'll start
                            //  transmitting immediately. This only does
                            //  anything if we are in the stopped or paused
                            //  states. If stopped, we just start playing
                            //  the current file again. Else we pick up
                            //  where we were.
                            //
                            if ((m_eMediaState == tCQCMedia::EMediaStates::Stopped)
                            &&  TFileSys::bExists(strCurSrcFile))
                            {
                                daedSrc.EndDecode();
                                daedSrc.StartDecode(strCurSrcFile);

                                // Preload all of the buffers
                                bEndOfSrc = bPreloadSpoolBufs(daedSrc, colSpoolBufs);
                                c4CurBufInd = 0;

                                // Make this look like a new stream to the Barix
                                c2SeqNum = 0;
                                c4TimeSeq = 0;
                                bEndOfSrc = kCIDLib::False;
                                enctNext = 0;
                                StoreSpoolState(tCQCMedia::EMediaStates::Playing);
                            }
                             else if (m_eMediaState == tCQCMedia::EMediaStates::Paused)
                            {
                                //
                                //  In this case, we keep any data that was
                                //  already in the spool buffer and pick up
                                //  from there, but make it look like a new
                                //  stream to the Barix.
                                //
                                c2SeqNum = 0;
                                c4TimeSeq = 0;
                                enctNext = 0;
                                StoreSpoolState(tCQCMedia::EMediaStates::Playing);
                            }
                            break;
                        }

                        case TCQCAudioPlayer::EEvTypes::Reset :
                        case TCQCAudioPlayer::EEvTypes::Stop :
                        {
                            //
                            //  Just put us back to idle state, reset the packets sent
                            //  count and the various streaming related locals.
                            //
                            ClearSpoolBufs(colSpoolBufs);
                            c2SeqNum = 0;
                            c4CurBufInd = 0;
                            c4TimeSeq = 0;
                            enctNext = 0;
                            m_c4PacketsSent = 0;

                            //
                            //  We have to wait for the loading thread
                            //  to finish with all the buffers.
                            //
                            WaitLoadingComplete(thrThis);
                            daedSrc.EndDecode();

                            //
                            //  Note we don't set end of media. They want us to stop
                            //  and if we did that then our parent would just continue
                            //  to the next song.
                            //
                            StoreSpoolState(tCQCMedia::EMediaStates::Stopped);
                            break;
                        }

                        default :
                            // <TBD> Dunno what this is, should log something
                            break;
                    };

                    // Trigger the event, and then drop the reference
                    cptrCmd->m_evWait.Trigger();
                    cptrCmd.DropRef();
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                    cptrCmd->m_perrFailure = new TError(errToCatch);
                    cptrCmd->m_evWait.Trigger();
                    cptrCmd.DropRef();
                    throw;
                }

                catch(...)
                {
                    cptrCmd->m_perrFailure = new TError
                    (
                        facBarixAudPlS().strName()
                        , CID_FILE
                        , CID_LINE
                        , kBrxPlErrs::errcInt_CmdFailed
                        , facBarixAudPlS().strMsg(kBrxPlErrs::errcInt_CmdFailed)
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::Unknown
                    );
                    cptrCmd->m_evWait.Trigger();
                    cptrCmd.DropRef();
                    throw;
                }
            }

            // If we are not playing, then just go back to the top
            if (m_eMediaState != tCQCMedia::EMediaStates::Playing)
                continue;

            //
            //  See if it's time to send another packet yet. If not, then go
            //  back to the top after a short sleep.
            //
            //  If a new stream, then initialize the next time to now, so
            //  we'll start streaming. Actually, we initialize it to a bit
            //  before now, so we'll stream out some packets quickly and
            //  get going faster.
            //
            tCIDLib::TEncodedTime enctCur = TTime::enctNow();
            if (!c2SeqNum)
            {
                enctNext = enctCur - (kCIDLib::enctOneMilliSec * 50);
            }
             else if (enctNext > enctCur)
            {
                const tCIDLib::TCard4 c4Millis = tCIDLib::TCard4
                (
                    (enctNext - enctCur) / kCIDLib::enctOneMilliSec
                );

                TThread::Sleep(c4Millis);
                continue;
            }

            //
            //  It's time to send something, so we'll loop until we get caught back
            //  up or we run out of data. If we run out of data, then we'll just go
            //  back around and load more and come back here again and keep sending
            //  till we catch up. But it would be bad if that happened and we'll
            //  likely get glitches because we are running behind.
            //
            //  Not that we actually let the time go slightly over, so that if we are
            //  really close, we'll go ahead and send that last one. It means we may
            //  send one early once in a while, but the Barix buffers so that's ok.
            //
            while (enctNext + 200 < enctCur)
            {
                //
                //  Load up the RTP packet's payload. This method will
                //  put data out of the active buffer and then move to the
                //  other if necessary and will update the index to the
                //  that's active after this load.
                //
                //  If we don't get any data, break out. If because of the
                //  end of source, then pause for a bit to allow the
                //  Barix spool buffer to drain.
                //
                if (!bExtractPacketData(RTPHdr, colSpoolBufs, c4CurBufInd, bEndOfSrc))
                {
                    if (bEndOfSrc)
                    {
                        TThread::Sleep(100);
                        m_c4PacketsSent = 0;

                        // Set us to stopped and end of media
                        StoreSpoolState
                        (
                            tCQCMedia::EMediaStates::Stopped
                            , tCQCMedia::EEndStates::Ended
                        );
                    }
                    break;
                }

                // We have a playload, so set up the RTP header
                RTPHdr.c1Flags = kBarixAudPlS::c1PLType
                                 | (c2SeqNum ? 0 : 0x80);

                #if defined(CIDLIB_LITTLEENDIAN)
                RTPHdr.c2SeqNum = TRawBits::c2SwapBytes(c2SeqNum);
                RTPHdr.c4TimeSeq = TRawBits::c4SwapBytes(c4TimeSeq);
                #else
                RTPHdr.c2SeqNum = c2SeqNum;
                RTPHdr.c4TimeSeq = c4TimeSeq;
                #endif

                // And send this one
                m_c4PacketsSent++;
                sockStream.c4SendTo(m_ipepStream, &RTPHdr, sizeof(TRTPPacket));

                //
                //  Move up the packet header counters. The sequence number
                //  might wrap around on a long song, so make sure it doesn't
                //  go back to zero.
                //
                c2SeqNum++;
                if (!c2SeqNum)
                    c2SeqNum++;
                c4TimeSeq += kBarixAudPlS::c4PayloadTime;

                // Update the spool time and current time
                enctNext += kBarixAudPlS::enctRoundDelay;
                enctCur = TTime::enctNow();
            }

            //
            //  See if we need to load up any spool bufs. We search forward from the
            //  current one to find the first one that is empty, then we just queue
            //  up any from there that need loading, so that the loader thread will
            //  begin loading them.
            //
            if (!bEndOfSrc)
            {
                tCIDLib::TCard4 c4FillInd = c4CurBufInd;
                c4FillInd++;
                if (c4FillInd == kBarixAudPlS::c4SpoolBufCnt)
                    c4FillInd = 0;

                while (c4FillInd != c4CurBufInd)
                {
                    TBufInfo& biCur = *colSpoolBufs[c4FillInd];
                    if (biCur.bIsEmpty())
                    {
                        // Reset it to loading state and queue it
                        biCur.Reset(EBufStates::Loading);
                        m_colBufQ.Add(&biCur);
                    }

                    c4FillInd++;
                    if (c4FillInd == kBarixAudPlS::c4SpoolBufCnt)
                        c4FillInd = 0;
                }
            }
        }

        //
        //  Note that, since we are a high priority thread, we sleep
        //  after an exception. Should we in some horrible circumstance
        //  get caught in a loop where we are continously getting errors,
        //  we don't want to eat the whole machine alive.
        //
        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::High);

            // Put us back into some sane condition
            try
            {
                ClearSpoolBufs(colSpoolBufs);
                c2SeqNum = 0;
                c4TimeSeq = 0;
                enctNext = 0;
                m_c4PacketsSent = 0;
                WaitLoadingComplete(thrThis);


                // Set us to stopped and end of media
                StoreSpoolState
                (
                    tCQCMedia::EMediaStates::Stopped
                    , tCQCMedia::EEndStates::Ended
                );
                daedSrc.EndDecode();
            }

            catch(...)
            {
            }

            // Pause a bit so we can't get into a CPU chewing freakout
            TThread::Sleep(100);
        }

        catch(...)
        {
            LogError(errFailureInLoop, tCQCKit::EVerboseLvls::High);

            // Put us back into some sane condition
            try
            {
                ClearSpoolBufs(colSpoolBufs);
                c2SeqNum = 0;
                c4TimeSeq = 0;
                enctNext = 0;
                m_c4PacketsSent = 0;
                WaitLoadingComplete(thrThis);

                // Set us to stopped and end of media
                StoreSpoolState
                (
                    tCQCMedia::EMediaStates::Stopped
                    , tCQCMedia::EEndStates::Ended
                );
                daedSrc.EndDecode();
            }

            catch(...)
            {
            }

            TThread::Sleep(100);
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  These are called by the main thread to queue up commands on the spooler
//  thread and wait for him to acknowledge having received it.
//
tCIDLib::TVoid
TBarixAudPlSDriver::QueueSpoolerCmd(const TCQCAudioPlayer::EEvTypes eType)
{
    TCQCAudioPlayer::TQElem cptrEv(new TCQCAudioPlayer::TEvData(eType));
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(kBarixAudPlS::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}

tCIDLib::TVoid
TBarixAudPlSDriver::QueueSpoolerCmd(const   TCQCAudioPlayer::EEvTypes eType
                                    , const tCIDLib::TBoolean         bParm1)
{
    TCQCAudioPlayer::TQElem cptrEv(new TCQCAudioPlayer::TEvData(eType));
    cptrEv->m_bVal = bParm1;
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(kBarixAudPlS::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}

tCIDLib::TVoid
TBarixAudPlSDriver::QueueSpoolerCmd(const   TCQCAudioPlayer::EEvTypes eType
                                    , const TString&                  strParm1)
{
    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(eType, strParm1, TString::strEmpty())
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(kBarixAudPlS::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}

tCIDLib::TVoid
TBarixAudPlSDriver::QueueSpoolerCmd(const   TCQCAudioPlayer::EEvTypes eType
                                    , const tCIDLib::TCard4           c4Parm1)
{
    TCQCAudioPlayer::TQElem cptrEv(new TCQCAudioPlayer::TEvData(eType));
    cptrEv->m_c4Val = c4Parm1;
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(kBarixAudPlS::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}


//
//  This method will send a simple command to the Barix, most of which just
//  involve a character to indicate the command and a value. This can be
//  done from the main driver thread because it sends on a separate socket
//  than the media streaming.
//
tCIDLib::TVoid
TBarixAudPlSDriver::SendCmd(const tCIDLib::TCh chCmd, const tCIDLib::TCard4 c4Val)
{
    TTextMBufOutStream strmCmd(1024, 1024, new TUSASCIIConverter);
    strmCmd.eNewLineType(tCIDLib::ENewLineTypes::CR);
    strmCmd << chCmd << L'=' << c4Val << kCIDLib::chCR << kCIDLib::FlushIt;
    m_sockCtrl.Send(strmCmd.mbufData(), strmCmd.c4CurSize());

    TString strReply;
    if (!bGetASCIITermMsg(m_sockCtrl, 2000, 13, 10, strReply))
    {
        facBarixAudPlS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBrxPlErrs::errcProto_NoAck
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NoReply
            , TString(chCmd)
        );
    }

    strReply.StripWhitespace();
    if (strReply != m_strRepl_Ack)
    {
        facBarixAudPlS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBrxPlErrs::errcProto_BarixErr
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Nak
            , strReply
            , TString(chCmd)
        );
    }

    // It also sends a trailing null, so eat that
    if (m_sockCtrl.bWaitForDataReady(5))
    {
        tCIDLib::TCard1 c1Null;
        m_sockCtrl.c4ReceiveRaw(&c1Null, 1);
    }
}


//
//  Locks and updates our two state values. The main driver thread reports these
//  back to our parent class when it asks for player state. The end state is much
//  less likely to be changed, so it defaults to the _Count value and watch for that
//  and don't store it.
//
tCIDLib::TVoid
TBarixAudPlSDriver::StoreSpoolState(const   tCQCMedia::EMediaStates eMediaState
                                    , const tCQCMedia::EEndStates   eEndState)
{
    // Lock while we update
    TCritSecLocker crslSync(&m_crsSync);

    m_eMediaState = eMediaState;
    if (eEndState != tCQCMedia::EEndStates::Count)
        m_eEndState = eEndState;
}


tCIDLib::TVoid TBarixAudPlSDriver::WaitLoadingComplete(TThread& thrThis)
{
    //
    //  Spin a bit while we wait for the spool buffer queue to become
    //  empty. This should happen very quickly, but we have to get it
    //  empty, so we'll wait for a lot longer than it should take.
    //
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (kCIDLib::enctOneSecond * 4);
    while (!m_colBufQ.bIsEmpty() && (enctCur < enctEnd))
    {
        // Not yet, so sleep a little bit. Break out anyway if asked to stop
        if (!thrThis.bSleep(5))
            break;
        enctCur = TTime::enctNow();
    }

    if (!m_colBufQ.bIsEmpty())
    {
        // This is bad
        facBarixAudPlS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kBrxPlErrs::errcInt_WaitQEmpty
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }
}

