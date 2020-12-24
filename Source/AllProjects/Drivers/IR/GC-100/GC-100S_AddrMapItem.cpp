//
// FILE NAME: GC100S_AddrMapItem.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/18/2003
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
//  This file implements a mapping item that maps between the CQC field ids
//  and the GC-100 specific addresses.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GC-100S.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGC100Addr,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGC100Addr
//  PREFIX: gca
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGC100Addr: Constructors and Destructor
// ---------------------------------------------------------------------------
TGC100Addr::TGC100Addr() :

    m_c4ConnNum(0)
    , m_c4FldId(0)
    , m_c4ModuleNum(0)
    , m_eModType(tGC100::EModTypes::Unknown)
{
}

TGC100Addr::TGC100Addr(const TGC100Addr& gcaToCopy) :

    m_c4ConnNum(gcaToCopy.m_c4ConnNum)
    , m_c4FldId(gcaToCopy.m_c4FldId)
    , m_c4ModuleNum(gcaToCopy.m_c4ModuleNum)
    , m_eModType(gcaToCopy.m_eModType)
    , m_strName(gcaToCopy.m_strName)
{
}

TGC100Addr::~TGC100Addr()
{
}


// ---------------------------------------------------------------------------
//  TGC100Addr: Public operators
// ---------------------------------------------------------------------------
TGC100Addr& TGC100Addr::operator=(const TGC100Addr& gcaToAssign)
{
    if (this != &gcaToAssign)
    {
        m_c4ConnNum   = gcaToAssign.m_c4ConnNum;
        m_c4FldId     = gcaToAssign.m_c4FldId;
        m_c4ModuleNum = gcaToAssign.m_c4ModuleNum;
        m_eModType    = gcaToAssign.m_eModType;
        m_strName     = gcaToAssign.m_strName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGC100Addr: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TGC100Addr::c4ConnNum() const
{
    return m_c4ConnNum;
}


tCIDLib::TCard4 TGC100Addr::c4FldId() const
{
    return m_c4FldId;
}

tCIDLib::TCard4 TGC100Addr::c4FldId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4FldId = c4ToSet;
    return m_c4FldId;
}


tCIDLib::TCard4 TGC100Addr::c4ModuleNum() const
{
    return m_c4ModuleNum;
}


tGC100::EModTypes TGC100Addr::eModType() const
{
    return m_eModType;
}


const TString& TGC100Addr::strName() const
{
    return m_strName;
}


tCIDLib::TVoid TGC100Addr::Set( const   TString&            strName
                                , const tCIDLib::TCard4     c4ConnNum
                                , const tCIDLib::TCard4     c4ModuleNum
                                , const tGC100::EModTypes   eModType)
{
    m_c4FldId     = kCIDLib::c4MaxCard;

    m_c4ConnNum   = c4ConnNum;
    m_c4ModuleNum = c4ModuleNum;
    m_eModType    = eModType;
    m_strName     = strName;
}


