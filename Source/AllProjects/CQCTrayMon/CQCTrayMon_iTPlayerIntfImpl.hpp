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
        typedef TiTPlayerIntfServerBase     TPlBase;


        // --------------------------------------------------------------------
        // Constructors and Destructor
        // --------------------------------------------------------------------
        TiTunesPlCtrlImpl
        (
                    TCQCTrayiTunesTab* const pwndTab
        );

        ~TiTunesPlCtrlImpl();


        // --------------------------------------------------------------------
        // Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TBoolean bGetPlayerState
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       tCIDLib::TBoolean&      bConnected
            ,       tCIDLib::TCard4&        c4Volume
            ,       TiTPlayerIntfServerBase::EPlStates& ePlState
            ,       tCIDLib::TBoolean&      bMute
            ,       tCIDLib::TEncodedTime&  enctTotal
            ,       tCIDLib::TEncodedTime&  enctCur
            ,       TString&                strArtist
            ,       TString&                strAlbum
            ,       TString&                strTrack
        );

        tCIDLib::TVoid DoPlayerCmd
        (
            const   TiTPlayerIntfServerBase::EPlCmds eCmd
        );

        tCIDLib::TVoid Initialize();

        tCIDLib::TVoid SelPLByCookie
        (
            const   TString&                strTitleCookie
        );

        tCIDLib::TVoid SelTrackByCookie
        (
            const   TString&                strItemCookie
        );

        tCIDLib::TVoid SetMute
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TVoid Terminate();


    private :
        // --------------------------------------------------------------------
        // Unimplemented ctors and operators
        // --------------------------------------------------------------------
        TiTunesPlCtrlImpl(const TiTunesPlCtrlImpl&);
        tCIDLib::TVoid operator=(const TiTunesPlCtrlImpl&);


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


