//
// FILE NAME: CQCQTAudSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/11/2007
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
//  This is the header for the facility class for the program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCQTAudSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCQTAudSrv : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCQTAudSrv();

        ~TFacCQCQTAudSrv();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        );

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Volume
        (
            const   tCIDLib::TCard4         c4ToPercent
        );

        tCIDLib::TVoid LoadFile
        (
            const   TString&                strToPlay
            , const TString&                strAudioDevice
        );

        tCIDLib::TVoid Pause();

        tCIDLib::TVoid Play();

        tCIDLib::TVoid Reset();

        tCIDLib::TVoid Stop();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ApplyMovieVolume
        (
            const   tCIDLib::TCard4         c4Volume
        );

        tCIDLib::TVoid CheckQTAvail();

        tCIDLib::TVoid CleanupPlayer
        (
            const   tCIDLib::TBoolean       bErr
        );

        tCIDLib::TVoid DoCleanup();

        tCIDLib::TVoid LoadNewURL
        (
            const   TString&                strToLoad
            , const TString&                strAudioDev
        );

        tCIDLib::TVoid ParseParms();

        tCIDLib::TVoid ProcessCmd
        (
                    TCQCAudioPlayer::TQElem& cptrEv
        );

        tCIDLib::TVoid ProcessRequests();

        tCIDLib::TVoid StoreOID
        (
            const   TOrbObjId&              ooidToStore
        );

        tCIDLib::TVoid ThrowIfNotReady();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_colCmdQ
        //      The commands sent to us by the QT audio player object are
        //      queued up for us to process asynchronously, while he waits.
        //
        //  m_i4TimeScale
        //      When we start up a new movie, we query the time scale, which
        //      shouldn't change during the playback. This keeps us from
        //      having to get it constantly to store the current time.
        //
        //  m_mbufShared
        //      A typed shared memory buffer that we share with the calling
        //      process. We store some info here for him to get to directly.
        //
        //  m_Movie
        //      The QT movie object we create to do our work. It's allocated
        //      when there's something playing.
        //
        //  m_pmtxSync
        //      A named mutex that we share with the calling program, to
        //      synchronize both between our internal thread and ORB
        //      threads calling into us, but also to sync access to the
        //      shared memory buffer we make some info available via.
        //
        //  m_pevWait
        //      The main thread just waits on this event once it has done
        //      the init. It is named so that, worst case, the app shell can
        //      use this to make it exist if it cannot get to the admin
        //      interface.
        //
        //  m_plgrLogSrv
        //      We install a log server logger. We don't own it after we've
        //      installed it, but we need to keep the pointer around so that
        //      we can force it back to local logging before we toast the
        //      client ORB support.
        //
        //  m_strTmp
        //      A temp string to use for formatting things, to avoid creating
        //      it over and over.
        //
        //  m_strUniqueID
        //      The unique id we were given in the command line parms, which
        //      we use to create some named resources via which the calling
        //      process can get info from us and get our ORB object id.
        // -------------------------------------------------------------------
        TCQCAudioPlayer::TCmdQueue  m_colCmdQ;
        tCIDLib::TInt4              m_i4TimeScale;
        tCQCMedia::TQTSharedBuf     m_mbufShared;
        Movie                       m_Movie;
        TMutex*                     m_pmtxSync;
        TEvent*                     m_pevWait;
        TLogSrvLogger*              m_plgrLogSrv;
        TString                     m_strTmp;
        TString                     m_strUniqueID;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCQTAudSrv,TFacility)
};

#pragma CIDLIB_POPPACK


