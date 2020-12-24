//
// FILE NAME: ZWaveLevi2C_DummyLevi.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/06/2014
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
//  This file implements the dummy server side driver mixin implementation
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2C_.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TDummyLevi
//  PREFIX: mzwsf
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDummyLevi: Constructors and Destructor
// ---------------------------------------------------------------------------
TDummyLevi::TDummyLevi() :

    m_strmDummy(16UL)
{
}

TDummyLevi::~TDummyLevi()
{
}


// ---------------------------------------------------------------------------
//  TDummyLevi: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TDummyLevi::bLeviFldExists(const TString&) const
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviQueryFldName(const tCIDLib::TCard4, TString&) const
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviStoreBoolFld(  const   tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bToStore)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviStoreCardFld(const tCIDLib::TCard4, const tCIDLib::TCard4)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviStoreFloatFld(const tCIDLib::TCard4, const tCIDLib::TFloat8)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviStoreIntFld(const tCIDLib::TCard4, const tCIDLib::TInt4)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviStoreStrFld(const tCIDLib::TCard4, const TString&)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean TDummyLevi::bLeviTraceEnabled() const
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviWaitBasicRep(  const   TZWaveUnit&
                                , const TZWCmdClass&
                                ,       tCIDLib::TCard4&)
{
    return kCIDLib::False;
}

tCIDLib::TBoolean
TDummyLevi::bLeviWaitUnitMsg(       TZWaveUnit&
                            , const TZWCmdClass&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4
                            ,       tCIDLib::TCardList&)
{
    return kCIDLib::False;
}

tCIDLib::TCard4 TDummyLevi::c4LeviFldIdFromName(const TString& strName) const
{
    return kCIDLib::c4MaxCard;
}

tCIDLib::TCard4 TDummyLevi::c4LeviVRCOPId() const
{
    return kCIDLib::c4MaxCard;
}

tCQCKit::EVerboseLvls TDummyLevi::eLeviVerboseLevel() const
{
    return tCQCKit::EVerboseLvls::Off;
}

tCIDLib::TVoid
TDummyLevi::LeviQEventTrig( const   tCQCKit::EStdDrvEvs
                            , const TString&
                            , const TString&
                            , const TString&
                            , const TString&
                            , const TString&)
{
}


tCIDLib::TVoid
TDummyLevi::LeviSendMsg(const TZWaveUnit&, const tCIDLib::TCardList&)
{
}

tCIDLib::TVoid
TDummyLevi::LeviSendUnitMsg(const   TZWaveUnit&
                            , const TZWCmdClass&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4)
{
}

tCIDLib::TVoid
TDummyLevi::LeviSendUnitMsg(const   TZWaveUnit&
                            , const TZWCmdClass&
                            , const tCIDLib::TCardList&)
{
}

tCIDLib::TVoid TDummyLevi::LeviSetFldErr(const tCIDLib::TCard4)
{
}

const TCQCFldLimit*
TDummyLevi::pfldlLeviLimsFor(const tCIDLib::TCard4, const TClass&) const
{
    return 0;
}

const TString& TDummyLevi::strLeviMoniker() const
{
    return TString::strEmpty();
}


TTextOutStream& TDummyLevi::strmLeviTrace()
{
    return m_strmDummy;
}


