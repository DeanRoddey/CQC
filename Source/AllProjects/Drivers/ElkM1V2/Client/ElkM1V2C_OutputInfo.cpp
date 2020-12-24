//
// FILE NAME: ElkM1V2C_OutputInfo.cpp
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
//  This file implements the class that we use to track the info for an output.
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
RTTIDecls(TOutputInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TOutputInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TOutputInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TOutputInfo::TOutputInfo() :

    TItemInfo(tElkM1V2C::EItemTypes::Output)
{
}

TOutputInfo::TOutputInfo(const TOutputInfo& iiSrc) :

    TItemInfo(iiSrc)
{
}


// ---------------------------------------------------------------------------
//  TOutputInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TOutputInfo::operator=(const TOutputInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
    }
}

tCIDLib::TBoolean TOutputInfo::operator==(const TOutputInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TOutputInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TOutputInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TOutputInfo&>(iiToComp));
}


tCIDLib::TVoid TOutputInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // Nothing for us for now
}


tCIDLib::TVoid
TOutputInfo::FormatSettings(        TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);
}


tCIDLib::TVoid
TOutputInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);
}


