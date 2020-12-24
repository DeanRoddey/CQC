//
// FILE NAME: ElkM1V2C_LoadInfo.cpp
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
//  lighting load.
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
RTTIDecls(TLoadInfo,TItemInfo)



// ---------------------------------------------------------------------------
//   CLASS: TLoadInfo
//  PREFIX: vi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLoadInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TLoadInfo::TLoadInfo() :

    TItemInfo(tElkM1V2C::EItemTypes::Load)
    , m_bDimmer(kCIDLib::False)
{
}

TLoadInfo::TLoadInfo(const TLoadInfo& iiSrc) :

    TItemInfo(iiSrc)
    , m_bDimmer(iiSrc.m_bDimmer)
{
}


// ---------------------------------------------------------------------------
//  TLoadInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid TLoadInfo::operator=(const TLoadInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        TParent::operator=(iiSrc);
        m_bDimmer = iiSrc.m_bDimmer;
    }
}

tCIDLib::TBoolean TLoadInfo::operator==(const TLoadInfo& iiSrc) const
{
    if (!TParent::operator==(iiSrc))
        return kCIDLib::False;

    return(m_bDimmer == iiSrc.m_bDimmer);
}


// ---------------------------------------------------------------------------
//  TLoadInfo: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLoadInfo::bComp(const TItemInfo& iiToComp) const
{
    return operator==(static_cast<const TLoadInfo&>(iiToComp));
}


tCIDLib::TVoid TLoadInfo::FormatCfg(TTextOutStream& strmTar) const
{
    // Call our parent first
    TParent::FormatCfg(strmTar);

    // And format our stuff
    strmTar << L','
            << (m_bDimmer ? L"Dim" : L"Switch");
}


tCIDLib::TVoid
TLoadInfo::FormatSettings(          TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    TParent::FormatSettings(strmTar, sfmtName, sfmtValue);

    strmTar << sfmtName << L"Type = "
            << sfmtValue << (m_bDimmer ? L"Dimmer" : L"Switch") << kCIDLib::NewLn;
}


tCIDLib::TVoid
TLoadInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // Call our parent
    TParent::ParseCfg(c4Id, colTokens);

    m_bDimmer = colTokens[0].bCompareI(L"Dim");
}



