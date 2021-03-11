//
// FILE NAME: CQCTrayMon_iTPlayerIntfImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/14/2012
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
//  This is the header for the CQCTrayMon_iTunesPlayerIntfImpl.cpp file, which
//  provides the implementation of the IDL generated server stub for the
//  iTunes player control portion of the tray monitor app.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


class TCQCTrayiTunesTab;

// ---------------------------------------------------------------------------
//  CLASS: TiTunesPlCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------
class TiTunesPlCtrlImpl : public TiTPlayerIntfServerBase
{
    public :
        // --------------------------------------------------------------------
        //  Get a short cut type name
        // --------------------------------------------------------------------
        using TPlBase = TiTPlayerIntfServerBase;


        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TiTunesPlCtrlImpl
        (
                    TCQCTrayiTunesTab* const pwndTab
        );

        TiTunesPlCtrlImpl(const TiTunesPlCtrlImpl&) = delete;
        TiTunesPlCtrlImpl(TiTunesPlCtrlImpl&&) = delete;

        ~TiTunesPlCtrlImpl();


        // --------------------------------------------------------------------
        // Unimplemented ctors and operators
        // --------------------------------------------------------------------
        TiTunesPlCtrlImpl& operator=(const TiTunesPlCtrlImpl&) = delete;
        TiTunesPlCtrlImpl& operator=(TiTunesPlCtrlImpl&&) = delete;


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bGetPlayerState
        (
            COP     tCIDLib::TCard4&        c4SerialNum
            , COP   tCIDLib::TBoolean&      bConnected
            , COP   tCIDLib::TCard4&        c4Volume
            , COP   TiTPlayerIntfServerBase::EPlStates& ePlState
            , COP   tCIDLib::TBoolean&      bMute
            , COP   tCIDLib::TEncodedTime&  enctTotal
            , COP   tCIDLib::TEncodedTime&  enctCur
            , COP   TString&                strArtist
            , COP   TString&                strAlbum
            , COP   TString&                strTrack
        )   final;

        tCIDLib::TVoid DoPlayerCmd
        (
            const   TiTPlayerIntfServerBase::EPlCmds eCmd
        )   final;

        tCIDLib::TVoid Initialize() final;

        tCIDLib::TVoid SelPLByCookie
        (
            const   TString&                strTitleCookie
        )   final;

        tCIDLib::TVoid SelTrackByCookie
        (
            const   TString&                strItemCookie
        )   final;

        tCIDLib::TVoid SetMute
        (
            const   tCIDLib::TBoolean       bToSet
        )   final;

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        )   final;

        tCIDLib::TVoid Terminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndTab
        //      The tab that owns us gives us a pointer to himself since we
        //      have to pass on the incoming calls to him.
        // --------------------------------------------------------------------
        TCQCTrayiTunesTab*  m_pwndTab;


        // --------------------------------------------------------------------
        // Magic macros
        // --------------------------------------------------------------------
        RTTIDefs(TiTunesPlCtrlImpl,TiTPlayerIntfServerBase)
};


