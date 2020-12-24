//
// FILE NAME: CQCMedia_AudioPlayer.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/14/2007
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
//  This is the header for the CQCMedia_AudioPlayer.cpp file, which
//  implements a wrapper around MS DirectShow API that creates a
//  simple, headless audio player. This is intended for use within a CQC
//  device driver mostly, so that it can provide background audio playback.
//  But it can also be embedded in client apps for audio playback there.
//  Since it is headless, it is not of any use for video. Another class
//  will provide that.
//
//  Though the filter graph drives itself on its own threads, we still use
//  a separate thread internally to manage the player, because it provides
//  events that have to be pulled out regularly, and there are issues with
//  accessing COM objects from multiple threads, which would be the case
//  in a CQC driver where the calls come in from a thread farm. So, though
//  it's a bit of a pain, it's the safest way to do it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
struct IBaseFilter;
struct IBasicAudio;
struct IGraphBuilder;
struct IMediaControl;
struct IMediaEvent;
struct IMediaSeeking;


// ---------------------------------------------------------------------------
//  CLASS: TCQCAudioPlayer
// PREFIX: aupl
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCWMPAudioPlayer : public TCQCAudioPlayer
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCWMPAudioPlayer() = delete;

        TCQCWMPAudioPlayer
        (
            const   tCIDLib::TCard4         c4DefVolume
        );

        TCQCWMPAudioPlayer(const TCQCWMPAudioPlayer&) = delete;

        ~TCQCWMPAudioPlayer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCWMPAudioPlayer& operator=(const TCQCWMPAudioPlayer&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMute() const override;

        tCIDLib::TBoolean bMute
        (
            const   tCIDLib::TBoolean       bToSet
        )   override;

        tCIDLib::TCard4 c4Volume() const override;

        tCIDLib::TCard4 c4Volume
        (
            const   tCIDLib::TCard4         c4ToPercent
        )   override;

        tCIDLib::TEncodedTime enctCurPos() const override;

        tCIDLib::TVoid GetStatusInfo
        (
                    tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TEncodedTime&  enctPlayPos
        )   override;

        tCIDLib::TVoid Initialize() override;

        tCIDLib::TVoid Pause() override;

        tCIDLib::TVoid Play() override;

        tCIDLib::TVoid LoadFile
        (
            const   TString&                strToPlay
            , const TString&                strTargetDev
        )   override;

        tCIDLib::TVoid Reset() override;

        tCIDLib::TVoid Terminate() override;

        tCIDLib::TVoid Stop() override;


    private :
        // -------------------------------------------------------------------
        //  Private, static methods
        // -------------------------------------------------------------------
        IBaseFilter* pFindDevFilter
        (
            const   TString&                strName
        );


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CleanupGraph
        (
            const   tCIDLib::TBoolean       bErr
        );

        tCIDLib::EExitCodes ePlayerThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCMedia::EMediaStates eXlatFilterState
        (
            const   tCIDLib::TSInt          sStatus
        );

        tCIDLib::TVoid LoadNewURL
        (
            const   TString&                strToLoad
            , const TString&                strDevice
        );

        tCIDLib::TVoid PostCmdAndWait
        (
                    TQElem&                 cptrToPost
        );

        tCIDLib::TVoid ProcessCmd
        (
                    TCntPtr<TEvData>&       cptrEv
        );

        tCIDLib::TVoid ReleaseCmds
        (
            const   TError* const           perrToRet
        );

        tCIDLib::TVoid SetNewVolume
        (
            const   tCIDLib::TCard4         c4Percent
            , const tCIDLib::TBoolean       bOverride
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMuted
        //      There isn't any mute per se, so we just have to set the
        //      volume to minimum level.
        //
        //  m_c4CurVolume
        //      We can't set the volume until we create the filter graph and
        //      get it set up. So we have to remember the last volume set on
        //      us and set it each time after we set up the graph.
        //
        //  m_colCmdQ
        //      The public methods that need to make the player do something
        //      will queue up commands here. It is thread safe.
        //
        //  m_eEndState
        //      The end state. This is used to report end of media or failures
        //      separate from the current playing state.
        //
        //  m_eState
        //      The last state of the player we say (either from us setting
        //      it or by an event be reported by the player.) This is returned
        //      to the client to know what we are doing.
        //
        //  m_enctCurPos
        //      The current playback position, in 100ns unit, i.e. the
        //      standard CIDLib time stamp.
        //
        //  m_enctWatchDog
        //      Used to watch for files that seem to start but ultimately the
        //      filter graph never makes any progress. If the current pos
        //      hasn't moved beyond zero for some period of time, we'll try
        //      to kill the bad file and move forward.
        //
        //  m_mtxSync
        //      We have to sync between the thread that's running the player
        //      and the incoming calls to public methods.
        //
        //  m_strCurFile
        //      The current file we are playing, just used in log message.
        //      We set it on start up of a new file, and clear it when it
        //      ends.
        //
        //  m_thrPlayer
        //      The player is run in its own thread, which complicates things
        //      but also simplifies things in many other ways.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bMuted;
        tCIDLib::TCard4         m_c4CurVolume;
        TCmdQueue               m_colCmdQ;
        tCQCMedia::EEndStates   m_eEndState;
        tCQCMedia::EMediaStates m_eState;
        tCIDLib::TEncodedTime   m_enctCurPos;
        tCIDLib::TEncodedTime   m_enctWatchDog;
        TMutex                  m_mtxSync;
        TString                 m_strCurFile;
        TThread                 m_thrPlayer;


        //
        //  These are the COM objects we use internally. They are just pointers
        //  and we forward referenced them above, so we don't expose any
        //  actual COM stuff, but having these here makes it a lot easier to
        //  call helper methods from the player thread without having to
        //  pass them to every method.
        //
        IBasicAudio*            m_pBasicAudio;
        IGraphBuilder*          m_pGraphBuilder;
        IMediaControl*          m_pMediaControl;
        IMediaEvent*            m_pMediaEvent;
        IMediaSeeking*          m_pMediaSeeking;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCWMPAudioPlayer,TCQCWMPAudioPlayer)
};

#pragma CIDLIB_POPPACK



