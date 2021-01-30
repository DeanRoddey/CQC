//
// FILE NAME: CQCMedia_AudioPlayer.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2007
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
//  implements an abstract base class for audio players, from which various
//  audio player engine wrappers can be derived.
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
//  CLASS: TCQCAudioPlayer
// PREFIX: aupl
// ---------------------------------------------------------------------------
class CQCMEDIAEXPORT TCQCAudioPlayer : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  A simple class, used internally by our derivatives, to queue up
        //  commands for an internal thread that manages the player engines.
        //  We use a counted pointer in the queue, so that the caller can hold
        //  onto the object as well and to avoid the need for copying the
        //  object, so that we can put an event object in it for the caller
        //  to block on.
        //
        //  Not all player derivatives have use this, but because the info
        //  that can be passed in is defined by our interface here, it's
        //  something that could be used by most derivatives.
        // -------------------------------------------------------------------
        enum class EEvTypes
        {
            Load
            , Mute
            , Pause
            , Play
            , Reset
            , Stop
            , Volume
        };

        class CQCMEDIAEXPORT TEvData
        {
            public :
                TEvData() = delete;
                TEvData(const EEvTypes  eType);
                TEvData
                (
                    const   EEvTypes    eType
                    , const TString&    strVal1
                    , const TString&    strVal2
                );

                TEvData(const TEvData&) = delete;
                TEvData(TEvData&&) = delete;

                ~TEvData();

                TEvData& operator=(const TEvData&) = delete;
                TEvData& operator=(TEvData&&) = delete;

                EEvTypes            m_eType;
                tCIDLib::TBoolean   m_bVal;
                tCIDLib::TCard4     m_c4Val;
                TEvent              m_evWait;
                TString*            m_pstrVal1;
                TString*            m_pstrVal2;
                TError*             m_perrFailure;
        };

        using TQElem = TCntPtr<TEvData>;
        using TCmdQueue = TQueue<TQElem>;


        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bFindDevGUID
        (
            const   TString&                strName
            ,       TString&                strGUID
        );


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TCQCAudioPlayer() = delete;

        TCQCAudioPlayer(const TCQCAudioPlayer&) = delete;
        TCQCAudioPlayer(TCQCAudioPlayer&&) = delete;

        ~TCQCAudioPlayer();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCAudioPlayer& operator=(const TCQCAudioPlayer&) = delete;
        TCQCAudioPlayer& operator=(TCQCAudioPlayer&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bMute() const = 0;

        virtual tCIDLib::TBoolean bMute
        (
            const   tCIDLib::TBoolean       bToSet
        ) = 0;

        virtual tCIDLib::TCard4 c4Volume() const = 0;

        virtual tCIDLib::TCard4 c4Volume
        (
            const   tCIDLib::TCard4         c4ToPercent
        ) = 0;

        virtual tCIDLib::TEncodedTime enctCurPos() const = 0;

        virtual tCIDLib::TVoid GetStatusInfo
        (
            COP     tCQCMedia::EMediaStates& eState
            , COP   tCQCMedia::EEndStates&  eEndState
            , COP   tCIDLib::TEncodedTime&  enctPlayPos
        ) = 0;

        virtual tCIDLib::TVoid Initialize() = 0;

        virtual tCIDLib::TVoid Pause() = 0;

        virtual tCIDLib::TVoid Play() = 0;

        virtual tCIDLib::TVoid LoadFile
        (
            const   TString&                strToPlay
            , const TString&                strTargetDev
        ) = 0;

        virtual tCIDLib::TVoid Reset() = 0;

        virtual tCIDLib::TVoid Terminate() = 0;

        virtual tCIDLib::TVoid Stop() = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4DefVolume() const noexcept;


    protected :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCAudioPlayer
        (
            const   tCIDLib::TCard4         c4DefVolume
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4DefVolume
        //      A default volume that we get in the ctor, which is used to
        //      set an initial volume upon load or reset of the driver.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4DefVolume;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCAudioPlayer,TObject)
};

#pragma CIDLIB_POPPACK




