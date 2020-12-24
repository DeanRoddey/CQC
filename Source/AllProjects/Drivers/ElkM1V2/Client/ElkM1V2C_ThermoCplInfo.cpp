//
// FILE NAME: ElkM1V2C_ThermoCplInfo.cpp
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
//  thermo.
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
RTTIDecls(TThermoCplInfo,TLimInfo)



// ---------------------------------------------------------------------------
//   CLASS: TThermoCplInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TThermoCplInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

// Tell our parent signed limits
TThermoCplInfo::TThermoCplInfo() :

    TLimInfo(tElkM1V2C::EItemTypes::ThermoCpl, kCIDLib::True, -10, 100)
{
}

TThermoCplInfo::TThermoCplInfo(const TThermoCplInfo& iiSrc) :

    TLimInfo(iiSrc)
{
}


// ---------------------------------------------------------------------------
//  TThermoCplInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TThermoCplInfo::operator=(const TThermoCplInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
    }
}


// For now just pass through to our parent
tCIDLib::TBoolean TThermoCplInfo::operator==(const TThermoCplInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TThermoCplInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TThermoCplInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TThermoCplInfo&>(iiToComp));
}


tCIDLib::TVoid TThermoCplInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);
}


tCIDLib::TVoid
TThermoCplInfo::FormatSettings(         TTextOutStream& strmTar
                                , const TStreamFmt&     sfmtName
                                , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);
}


tCIDLib::TVoid
TThermoCplInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);
}


