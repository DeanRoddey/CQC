//
// FILE NAME: CQCMedia_QTAudioPlayer.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/10/2007
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
//  This is the header for the CQCMedia_QTAudioPlayer.cpp file, which
//  implements a wrapper around the Quick Time 7 SDK. So, like the WMP
//  based player, it provides a headless audio player by talking directly
//  to the Quick Time engine. They both derive from the same base class so
//  that we treat them generically.
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
//  CLASS: TCQCQTAudioPlayer
// PREFIX: aupl
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCQTAudioPlayer : public TCQCAudioPlayer
{
    public  :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bQTAvailable
        (
                    tCIDLib::TInt4&         i4Version
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCQTAudioPlayer() = delete;

        TCQCQTAudioPlayer
        (
            const   tCIDLib::TIPPortNum     ippnServer
            , const TString&                strUniqueName
            , const tCIDLib::TCard4         c4DefVolume
        );

        TCQCQTAudioPlayer(const TCQCQTAudioPlayer&) = delete;
        TCQCQTAudioPlayer(TCQCQTAudioPlayer&&) = delete;

        ~TCQCQTAudioPlayer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCQTAudioPlayer& operator=(const TCQCQTAudioPlayer&) = delete;
        TCQCQTAudioPlayer& operator=(TCQCQTAudioPlayer&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bMute() const final;

        tCIDLib::TBoolean bMute
        (
            const   tCIDLib::TBoolean       bToSet
        )   final;

        tCIDLib::TCard4 c4Volume() const final;

        tCIDLib::TCard4 c4Volume
        (
            const   tCIDLib::TCard4         c4ToPercent
        )   final;

        tCIDLib::TEncodedTime enctCurPos() const final;

        tCIDLib::TVoid GetStatusInfo
        (
                    tCQCMedia::EMediaStates& eState
            ,       tCQCMedia::EEndStates&  eEndState
            ,       tCIDLib::TEncodedTime&  enctPlayPos
        )   final;

        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid Pause() final;

        tCIDLib::TVoid Play() final;

        tCIDLib::TVoid LoadFile
        (
            const   TString&                strToPlay
            , const TString&                strTargetDev
        )   final;

        tCIDLib::TVoid Reset() final;

        tCIDLib::TVoid Terminate() final;

        tCIDLib::TVoid Stop() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckServerProc();

        tCIDLib::TVoid StartServer();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_extpServer
        //      The external server process that manages our Quick Time player
        //      engine.
        //
        //  m_ippnServer
        //      The caller must provide a port number that we give to the
        //      server app to listen on via the ORB. We have to save it away
        //      until we can get it into the shared memory area.
        //
        //  m_ooidServer
        //      We extract the object id for the server ahead of time, to
        //      avoid having to unflatten it every time.
        //
        //  m_pevSync
        //  m_pmbufShared
        //  m_pmtxSync
        //      We share an event, mutex, and shared memory buffer with the
        //      QT server program. We create them and he opens them.
        //
        //  m_strUniqueName
        //      The caller has to provide a unique name (unique among any
        //      instances of this class) which we use to give to the server
        //      process as a unique id.
        // -------------------------------------------------------------------
        TExternalProcess            m_extpServer;
        tCIDLib::TIPPortNum         m_ippnServer;
        TOrbObjId                   m_ooidServer;
        TEvent*                     m_pevSync;
        tCQCMedia::TQTSharedBuf     m_mbufShared;
        TMutex*                     m_pmtxSync;
        TString                     m_strUniqueName;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCQTAudioPlayer,TCQCQTAudioPlayer)
};

#pragma CIDLIB_POPPACK



