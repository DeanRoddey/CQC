//
// FILE NAME: ElkM1V2C_CounterInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2014
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
//  This file implements the class that we use to track the info for a
//  counter.
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
#include    "ElkM1V2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCounterInfo,TLimInfo)



// ---------------------------------------------------------------------------
//   CLASS: TCounterInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCounterInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

// Tell our parent unsigned limits
TCounterInfo::TCounterInfo() :

    TLimInfo(tElkM1V2C::EItemTypes::Counter, kCIDLib::False, 0, 0x7FFF)
{
}

TCounterInfo::TCounterInfo(const TCounterInfo& iiSrc) :

    TLimInfo(iiSrc)
{
}


// ---------------------------------------------------------------------------
//  TCounterInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCounterInfo::operator=(const TCounterInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
    }
}

tCIDLib::TBoolean TCounterInfo::operator==(const TCounterInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TCounterInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCounterInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TCounterInfo&>(iiToComp));
}


tCIDLib::TVoid TCounterInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // Nothing of our own right now
}


tCIDLib::TVoid
TCounterInfo::FormatSettings(       TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);
}


tCIDLib::TVoid
TCounterInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);
}


