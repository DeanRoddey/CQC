//
// FILE NAME: CQCTrayMon_iTunesPlayerIntfImpl.cpp
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
//  This implements the IDL generated server stub for the protocol interface
//  that the iTunes player contorl driver uses to talk to the tray monitor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesPlCtrlImpl,TiTPlayerIntfServerBase)



// ---------------------------------------------------------------------------
//  CLASS: TiTunesPlCtrlImpl
// PREFIX: orbs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TiTunesPlCtrlImpl: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesPlCtrlImpl::TiTunesPlCtrlImpl(TCQCTrayiTunesTab* const pwndTab) :

    m_pwndTab(pwndTab)
{
}


TiTunesPlCtrlImpl::~TiTunesPlCtrlImpl()
{
}


// ---------------------------------------------------------------------------
//  TiTunesPlCtrlImpl: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TiTunesPlCtrlImpl::bGetPlayerState( COP     tCIDLib::TCard4&                    c4SerialNum
                                    , COP tCIDLib::TBoolean&                    bConnected
                                    , COP tCIDLib::TCard4&                      c4Volume
                                    , COP TiTPlayerIntfServerBase::EPlStates&   ePlState
                                    , COP tCIDLib::TBoolean&                    bMute
                                    , COP tCIDLib::TEncodedTime&                enctTotal
                                    , COP tCIDLib::TEncodedTime&                enctCur
                                    , COP TString&                              strArtist
                                    , COP TString&                              strAlbum
                                    , COP TString&                              strTrack)
{
    return m_pwndTab->bGetPlayerState
    (
        c4SerialNum
        , bConnected
        , c4Volume
        , ePlState
        , bMute
        , enctTotal
        , enctCur
        , strArtist
        , strAlbum
        , strTrack
    );
}


tCIDLib::TVoid
TiTunesPlCtrlImpl::DoPlayerCmd(const TiTPlayerIntfServerBase::EPlCmds eCmd)
{
    return m_pwndTab->DoPlayerCmd(eCmd);
}


tCIDLib::TVoid TiTunesPlCtrlImpl::Initialize()
{
}


tCIDLib::TVoid
TiTunesPlCtrlImpl::SelPLByCookie(const TString& strTitleCookie)
{
    m_pwndTab->SelPLByCookie(strTitleCookie);
}


tCIDLib::TVoid
TiTunesPlCtrlImpl::SelTrackByCookie(const TString& strItemCookie)
{
    m_pwndTab->SelTrackByCookie(strItemCookie);
}


tCIDLib::TVoid
TiTunesPlCtrlImpl::SetMute(const tCIDLib::TBoolean bToSet)
{
    m_pwndTab->SetMute(bToSet);
}


tCIDLib::TVoid
TiTunesPlCtrlImpl::SetVolume(const tCIDLib::TCard4 c4ToSet)
{
    m_pwndTab->SetVolume(c4ToSet);
}


tCIDLib::TVoid TiTunesPlCtrlImpl::Terminate()
{
}


