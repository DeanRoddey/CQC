//
// FILE NAME: ElkM1V2C_ItemInfo.cpp
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
//  This file implements the bass class for all of the item config classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ElkM1V2C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TItemInfo,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TItemInfo
//  PREFIX: ii
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TItemInfo: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps
TItemInfo::eSortByNum(  const   TItemInfo&      ii1
                        , const TItemInfo&      ii2)
{
    if (ii1.m_c4ElkId < ii2.m_c4ElkId)
        return tCIDLib::ESortComps::FirstLess;
    else if (ii1.m_c4ElkId > ii2.m_c4ElkId)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TItemInfo: Destructor
// ---------------------------------------------------------------------------
TItemInfo::~TItemInfo()
{
}


// ---------------------------------------------------------------------------
//  TItemInfo: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TItemInfo::operator==(const TItemInfo& iiSrc) const
{
    return
    (
        (m_c4ElkId == iiSrc.m_c4ElkId)
        && (m_strName == iiSrc.m_strName)
    );
}


// ---------------------------------------------------------------------------
//  TItemInfo: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  This has to be the same as what is expected by the server side driver
//  as well since it's used in the formatting out of the config string
//  we send.
//
tCIDLib::TVoid TItemInfo::FormatCfg(TTextOutStream& strmTar) const
{
    strmTar << L"    "
            << m_c4ElkId
            << L'='
            << m_strName;
}


//
//  This will format for display purposes. THe derived class calls us first, then
//  formats out his own stuff.
//
tCIDLib::TVoid
TItemInfo::FormatSettings(          TTextOutStream& strmTar
                            , const TStreamFmt&     sfmtName
                            , const TStreamFmt&     sfmtValue) const
{
    strmTar << sfmtName << L"Name = "
            << sfmtValue << m_strName << kCIDLib::NewLn

            << sfmtName << L"Elk ID = "
            << sfmtValue << m_c4ElkId << kCIDLib::NewLn;
}


tCIDLib::TVoid
TItemInfo::ParseCfg(const tCIDLib::TCard4 c4Id, tCIDLib::TStrList& colTokens)
{
    // The first parm is the id
    m_c4ElkId = c4Id;

    // And next is the name
    m_strName = colTokens[0];

    // Now remove the one we processed
    colTokens.RemoveAt(0);
}


// ---------------------------------------------------------------------------
//  TItemInfo: Hidden constructors and operators
// ---------------------------------------------------------------------------

TItemInfo::TItemInfo(const tElkM1V2C::EItemTypes eType) :

    m_c4ElkId(kCIDLib::c4MaxCard)
    , m_eType(eType)
{
}

TItemInfo::TItemInfo(const TItemInfo& iiSrc) :

    m_c4ElkId(iiSrc.m_c4ElkId)
    , m_eType(iiSrc.m_eType)
    , m_strName(iiSrc.m_strName)
{
}

tCIDLib::TVoid TItemInfo::operator=(const TItemInfo& iiSrc)
{
    if (this != &iiSrc)
    {
        m_c4ElkId = iiSrc.m_c4ElkId;
        m_eType   = iiSrc.m_eType;
        m_strName = iiSrc.m_strName;
    }
}


