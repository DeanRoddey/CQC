//
// FILE NAME: ElkM1V2C_AreaInfo.cpp
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
//  This file implements the class that we use to track the info for an area.
//  .
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
RTTIDecls(TAreaInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TAreaInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAreaInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TAreaInfo::TAreaInfo() :

    TItemInfo(tElkM1V2C::EItemTypes::Area)
{
}

TAreaInfo::TAreaInfo(const TAreaInfo& iiSrc) :

    TItemInfo(iiSrc)
{
}


// ---------------------------------------------------------------------------
//  TAreaInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TAreaInfo::operator=(const TAreaInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
    }
}

tCIDLib::TBoolean TAreaInfo::operator==(const TAreaInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TAreaInfo: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TAreaInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TAreaInfo&>(iiToComp));
}


tCIDLib::TVoid TAreaInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // Nothing for us for now
}


tCIDLib::TVoid
TAreaInfo::FormatSettings(          TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);
}


tCIDLib::TVoid
TAreaInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent. We don't have anything else
    TParent::ParseCfg(c4Id, colTokens);
}


