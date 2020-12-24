//
// FILE NAME: BarixAudPlS_DriverImpl.hpp
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
//  This is the header file for the actual driver class. The methods are
//  spread out in a couple of implementation files.
//
//  There are thread threads involved:
//
//  1.  A loader thread which is given a source file to load and will just block
//      on a queue waiting for available buffers. When it gets one it puts the
//      next chunk of data into it, until it hits the end of the file. It marks
//      them to indicate more data or end of data.
//
//  2.  A spooler thread which spools out the data to the Barix. It gives the
//      loader thread buffers to load, which it then grabs and spools out. So it
//      drives the playback process. It watches a command queue for commands from
//      the driver thread.
//
//  3.  The main driver thread does its usual thing. In response to commands from
//      the clients, it queues up commands to the spooling thread. These include
//      both transport commands and commands to load a new file.
//
//      In response to status callsbacks from our parent class, it monitors the
//      media/end states that the spooler thread keeps updated and it performs the
//      playback process.
//
//
//  The two queues (buffer and command) are thread safe. However we also have to sync
//  access to the data shared between the driver and spooler threads. WE CANNOT just
//  use an overall mutex for all sync. The two queues must use their own internal
//  locks, so that they can allow one thread to wait for data while another queues
//  up data.
//
//  So we create a critical section just to handle the very light locking required
//  between the driver thread and spooler thread to update/read playback status
//  info. The spooler thread updates it, and the driver thread reads it. So the
//  spooler thread doesn't need any sync to look at the states, since it is the only
//  one changing them. It only needs to lock to update them. The driver thread has
//  to lock to read them.
//
// CAVEATS/GOTCHAS:
//
//  This device doesn't allow us to read volume or mute, and doesn't even offer a
//  discrete mute control. So, we have to fake this stuff. We set the volume during
//  connect, to the user configured initial setting, and store that. After that we
//  just assume any non-failng volume commands set that volume.
//
//  For mute, we keep a boolean m_bMute and set/clear it as the mute field is
//  written to. When it goes true we set the device's volume to zero, but don't
//  update our stored volume. When unmuted we set the volume back to that. If we
//  get a volume setting command, we unmute automatically because the volume is
//  no longer zero (and we clear our m_bMute flag.)
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: BarixAudPlS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TBarixAudPlSDriver : public TCQCStdMediaRendDrv
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TBarixAudPlSDriver() = delete;

        TBarixAudPlSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TBarixAudPlSDriver(const TBarixAudPlSDriver&) = delete;

        ~TBarixAudPlSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TBarixAudPlSDriver& operator=(const TBarixAudPlSDriver&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bPlayNewItem
        (
            const   TCQCMediaPLItem&        mpliNew
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eAdjustVolume
        (
            const   tCIDLib::TBoolean       bUp
        );

        tCQCKit::ECommResults eCheckPlayerStatus
        (
                    tCIDLib::TEncodedTime&  enctCurPos
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TCard4&        c4Volume
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eDoTransportCmd
        (
            const   tCQCMedia::ETransCmds   eToSet
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults eSetMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCQCKit::ECommResults eSetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCKit::ECommResults eStartPowerTrans
        (
            const   tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid ResetPlayer();

        tCIDLib::TVoid TerminateImpl();


    private :
        //
        //  A little class to track the info for one spooling buffer. The
        //  loader thread loads these up and the spooler thread spools out
        //  the data.
        //
        //  Note that we set a pointer to a decoder on it so that that can
        //  be queued up as well, and the loader thread uses whichever
        //  decoder is set on the buffer. We don't own it, and it might
        //  be set to a different decoder if another type of file is being
        //  streamed.
        //
        enum class EBufStates
        {
            Empty
            , Loading
            , Ready
        };

        class TBufInfo
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TBufInfo();

                ~TBufInfo();


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bIsEmpty();

                tCIDLib::TBoolean bLastBuffer();

                tCIDLib::TBoolean bLoadFromSrc();

                tCIDLib::TCard4 c4BytesLeft() const;

                tCIDLib::TCard4 c4IncIndex
                (
                    const   tCIDLib::TCard4         c4By
                );

                tCIDLib::TVoid CopyOut
                (
                            tCIDLib::TCard1* const  pc1Target
                    , const tCIDLib::TCard4         c4Count
                );

                EBufStates eState() const;

                tCIDLib::TVoid Reset
                (
                    const   EBufStates              eState
                );

                tCIDLib::TVoid SetDecoder
                (
                            TCIDDAEDecoder* const   pdaedSrc
                );


            private :
                // -----------------------------------------------------------
                //  Private data members
                // -----------------------------------------------------------
                tCIDLib::TBoolean   m_bLastBuf;
                tCIDLib::TCard4     m_c4SpoolBufInd;
                tCIDLib::TCard4     m_c4SpoolBufSz;
                TCIDDAEDecoder*     m_pdaedSrc;
                EBufStates          m_eState;
                THeapBuf            m_mbufData;
        };
        using TSpoolBufList = TObjArray<TBufInfo>;
        using TSpoolBufQ = TRefQueue<TBufInfo>;

        //
        //  An RTP type packet. This is what we send via UDP to the Barix
        //  The payload size is fixed. For a partial last packet we just
        //  zero fill.
        //
        #pragma pack(push,1)
        struct TRTPPacket
        {
            tCIDLib::TCard1 c1Start;
            tCIDLib::TCard1 c1Flags;
            tCIDLib::TCard2 c2SeqNum;
            tCIDLib::TCard4 c4TimeSeq;
            tCIDLib::TCard4 c4SSrc;
            tCIDLib::TCard1 ac1Buf[kBarixAudPlS::c4PayloadSz];
        };
        #pragma pack(pop)


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bExtractPacketData
        (
                    TRTPPacket&             RTPHdr
            ,       TSpoolBufList&          colBufs
            ,       tCIDLib::TCard4&        c4CurIndex
            ,       tCIDLib::TBoolean&      bEndOfSrc
        );

        tCIDLib::TBoolean bPreloadSpoolBufs
        (
                    TCIDDAEWMADec&          daedSrc
            ,       TSpoolBufList&          colBufs
        );

        tCIDLib::TVoid ClearSpoolBufs
        (
                    TSpoolBufList&          colBufs
        );

        tCIDLib::EExitCodes eLoaderThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::EExitCodes eSpoolThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid HandleCmd
        (
                    TCQCAudioPlayer::TQElem& cptrCmd
        );

        tCIDLib::TVoid QueueSpoolerCmd
        (
            const   TCQCAudioPlayer::EEvTypes eType
        );

        tCIDLib::TVoid QueueSpoolerCmd
        (
            const   TCQCAudioPlayer::EEvTypes eType
            , const tCIDLib::TBoolean       bParm1
        );

        tCIDLib::TVoid QueueSpoolerCmd
        (
            const   TCQCAudioPlayer::EEvTypes eType
            , const TString&                strParm1
        );

        tCIDLib::TVoid QueueSpoolerCmd
        (
            const   TCQCAudioPlayer::EEvTypes eType
            , const tCIDLib::TCard4         c4Parm2
        );

        tCIDLib::TVoid SendCmd
        (
            const   tCIDLib::TCh            chCmd
            , const tCIDLib::TCard4         c4Val
        );

        tCIDLib::TVoid StoreSpoolState
        (
            const   tCQCMedia::EMediaStates eMediaState
            , const tCQCMedia::EEndStates   eEndState = tCQCMedia::EEndStates::Count
        );

        tCIDLib::TVoid WaitLoadingComplete
        (
                    TThread&                thrThis
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMute
        //      We track the last mute state seen, so that we can report it back
        //      to our parent class.
        //
        //      See the file header comments for details!
        //
        //  m_c4PacketsSent
        //      The spooling thread keeps this up to date as it sends out
        //      each packet. We know how much time each outgoing packet
        //      represents, so the main thread can easily calculate the
        //      elapsed time. This is easier than having the spooling thread
        //      store the elapsed time, since that would require sync
        //      to read a 64 bit value on 32 bit systems, whereas this does
        //      not.
        //
        //  m_c4Volume
        //  m_c4VolumeMute
        //      We track the last volume state seen, so that we can report it back
        //      to our parent class.
        //
        //      See the file header comments for details!
        //
        //  m_colBufQ
        //      This is the queue that the spooling thread queues up requests
        //      for the loading thread to load buffers for him on. This is a thread
        //      safe queue.
        //
        //      See the file header comments for details!
        //
        //  m_colCmdQ
        //      The queue that the driver  drops commands into to ask
        //      the spooler thread to do things. We use one that is provided
        //      by the audio player classes in CQCMedia. We don't derive from
        //      the media player class, we just use the types it defines for
        //      the queue and the command items.
        //
        //      See the file header comments for details!
        //
        //  m_conncfgSock
        //      The socket based connection configuration info we get during
        //      driver installation.
        //
        //  m_eEndState
        //  m_eMediaState
        //      We have to keep around current state info to give back to our
        //      parent class when asked.
        //
        //      See the file header comments for details!
        //
        //  m_ipepCtrl
        //  m_ipepStream
        //      The end points of the Barix, one for control and one for
        //      streaming. The one we get from the user during installation
        //      is the streaming end point. We assume that the control
        //      end point is at the next sequential IP port.
        //
        //      This is our socket that we use for control of the Barix,
        //      not for streaming of media. This is the one we consider
        //      the connection to the device since it's a stream socket.
        //
        //  m_crsSync
        //      Used to sync access to the data that is shared between the main
        //      driver thread and the background threads, currently only the spooler
        //      thread which maintains current media states.
        //
        //      See the file header comments for details!
        //
        //  m_strRepl_XXX
        //      Some reply strings we look for from the Barix, so we go ahead
        //      create strings for them for efficiency.
        //
        //  m_strTmp
        //  m_strTmp2
        //      Some temp strings. Don't assume that they will be unaffected
        //      across any calls.
        //
        //  m_thrSpooler
        //      This is the thread that drives the spooling process. It is
        //      the one that we queue up user commands on and it pushes data
        //      out to the Barix. It depends on the loader thread to do the
        //      actual decoding.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bMute;
        tCIDLib::TCard4             m_c4PacketsSent;
        tCIDLib::TCard4             m_c4Volume;
        TSpoolBufQ                  m_colBufQ;
        TCQCAudioPlayer::TCmdQueue  m_colCmdQ;
        TCQCIPConnCfg               m_conncfgSock;
        TCriticalSection            m_crsSync;
        tCQCMedia::EEndStates       m_eEndState;
        tCQCMedia::EMediaStates     m_eMediaState;
        TIPEndPoint                 m_ipepCtrl;
        TIPEndPoint                 m_ipepStream;
        TClientStreamSocket         m_sockCtrl;
        TString                     m_strRepl_Ack;
        TString                     m_strTmp;
        TString                     m_strTmp2;
        TThread                     m_thrLoader;
        TThread                     m_thrSpool;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBarixAudPlSDriver,TCQCStdMediaRendDrv)
};

#pragma CIDLIB_POPPACK


