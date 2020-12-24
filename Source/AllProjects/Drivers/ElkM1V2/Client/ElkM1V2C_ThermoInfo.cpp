//
// FILE NAME: ElkM1V2C_ThermoInfo.cpp
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
RTTIDecls(TThermoInfo,TLimInfo)



// ---------------------------------------------------------------------------
//   CLASS: TThermoInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TThermoInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

// Tell our praent signed limits
TThermoInfo::TThermoInfo() :

    TLimInfo(tElkM1V2C::EItemTypes::Thermo, kCIDLib::True, -10, 100)
{
}

TThermoInfo::TThermoInfo(const TThermoInfo& iiSrc) :

    TLimInfo(iiSrc)
{
}


// ---------------------------------------------------------------------------
//  TThermoInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TThermoInfo::operator=(const TThermoInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
    }
}

tCIDLib::TBoolean TThermoInfo::operator==(const TThermoInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TThermoInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TThermoInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TThermoInfo&>(iiToComp));
}


tCIDLib::TVoid TThermoInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);
}


tCIDLib::TVoid
TThermoInfo::FormatSettings(        TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);
}


tCIDLib::TVoid
TThermoInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);
}


