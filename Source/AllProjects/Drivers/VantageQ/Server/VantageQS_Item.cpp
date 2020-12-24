//
// FILE NAME: VantageQS_Item.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This file implements the classes that model the named items we query
//  from the Vantage and make available for control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VantageQS.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TVantageQSItem,TObject)
RTTIDecls(TVantageQSButton,TVantageQSItem)
RTTIDecls(TVantageQSLoad,TVantageQSItem)


// ---------------------------------------------------------------------------
//   CLASS: TVantageQSItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVantageQSItem: Destructor
// ---------------------------------------------------------------------------
TVantageQSItem::~TVantageQSItem()
{
}


// ---------------------------------------------------------------------------
//  TVantageQSItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard8 TVantageQSItem::c8Addr() const
{
    return m_c8Addr;
}


tVantageQS::EAddrTypes TVantageQSItem::eAddrType() const
{
    return m_eAddrType;
}


const TString& TVantageQSItem::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TVantageQSItem: Hidden constructors and operators
// ---------------------------------------------------------------------------
TVantageQSItem::TVantageQSItem( const   TString&        strName
                                , const tCIDLib::TCard4 c4Master
                                , const tCIDLib::TCard4 c4Station
                                , const tCIDLib::TCard4 c4ItemNum) :

    m_eAddrType(tVantageQS::EAddrTypes::Station)
    , m_strName(strName)
{
    // Set up the address info
    m_ac4Addrs[0] = c4Master;
    m_ac4Addrs[1] = c4Station;
    m_ac4Addrs[2] = c4ItemNum;

    m_c8Addr = c4Master;
    m_c8Addr <<= 16;
    m_c8Addr |= c4Station;
    m_c8Addr <<= 16;
    m_c8Addr |= c4ItemNum;
    m_c8Addr <<= 16;
}

TVantageQSItem::TVantageQSItem( const   TString&        strName
                                , const tCIDLib::TCard4 c4Master
                                , const tCIDLib::TCard4 c4Enclosure
                                , const tCIDLib::TCard4 c4Module
                                , const tCIDLib::TCard4 c4LoadNum) :

    m_eAddrType(tVantageQS::EAddrTypes::Enclosure)
    , m_strName(strName)
{
    // Set up the address info
    m_ac4Addrs[0] = c4Master;
    m_ac4Addrs[1] = c4Enclosure;
    m_ac4Addrs[2] = c4Module;
    m_ac4Addrs[3] = c4LoadNum;

    m_c8Addr = c4Master;
    m_c8Addr <<= 16;
    m_c8Addr |= c4Enclosure;
    m_c8Addr <<= 16;
    m_c8Addr |= c4Module;
    m_c8Addr <<= 16;
    m_c8Addr |= c4LoadNum;
}

TVantageQSItem::TVantageQSItem(const TVantageQSItem& itemToCopy) :

    m_c8Addr(itemToCopy.m_c8Addr)
    , m_eAddrType(itemToCopy.m_eAddrType)
    , m_strName(itemToCopy.m_strName)
{
    m_ac4Addrs[0] = itemToCopy.m_ac4Addrs[0];
    m_ac4Addrs[1] = itemToCopy.m_ac4Addrs[1];
    m_ac4Addrs[2] = itemToCopy.m_ac4Addrs[2];
    m_ac4Addrs[3] = itemToCopy.m_ac4Addrs[3];
}

TVantageQSItem& TVantageQSItem::operator=(const TVantageQSItem& itemToAssign)
{
    if (this != &itemToAssign)
    {
        m_c8Addr    = itemToAssign.m_c8Addr;
        m_eAddrType = itemToAssign.m_eAddrType;
        m_strName   = itemToAssign.m_strName;

        m_ac4Addrs[0] = itemToAssign.m_ac4Addrs[0];
        m_ac4Addrs[1] = itemToAssign.m_ac4Addrs[1];
        m_ac4Addrs[2] = itemToAssign.m_ac4Addrs[2];
        m_ac4Addrs[3] = itemToAssign.m_ac4Addrs[3];
    }
    return *this;
}





// ---------------------------------------------------------------------------
//   CLASS: TVantageQSButton
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVantageQSButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TVantageQSButton::TVantageQSButton( const   TString&        strName
                                    , const tCIDLib::TCard4 c4Master
                                    , const tCIDLib::TCard4 c4Station
                                    , const tCIDLib::TCard4 c4ItemNum) :

    TVantageQSItem(strName, c4Master, c4Station, c4ItemNum)
{
}

TVantageQSButton::TVantageQSButton(const TVantageQSButton& itemToCopy) :

    TVantageQSItem(itemToCopy)
{
}

TVantageQSButton::~TVantageQSButton()
{
}


// ---------------------------------------------------------------------------
//  TVantageQSButton: Public operators
// ---------------------------------------------------------------------------
TVantageQSButton&
TVantageQSButton::operator=(const TVantageQSButton& itemToAssign)
{
    if (this != &itemToAssign)
        TParent::operator=(itemToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TVantageQSButton: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVantageQSButton::bOwnsField(const tCIDLib::TCard4) const
{
    // We have no fields, so just say no
    return kCIDLib::False;
}




// ---------------------------------------------------------------------------
//   CLASS: TVantageQSLoad
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVantageQSLoad: Constructors and Destructor
// ---------------------------------------------------------------------------
TVantageQSLoad::TVantageQSLoad( const   TString&                strName
                                , const tCIDLib::TCard4         c4Master
                                , const tCIDLib::TCard4         c4Station
                                , const tCIDLib::TCard4         c4ItemNum
                                , const tVantageQS::ELoadTypes  eType) :

    TVantageQSItem(strName, c4Master, c4Station, c4ItemNum)
    , m_eType(eType)
{
}

TVantageQSLoad::TVantageQSLoad( const   TString&                strName
                                , const tCIDLib::TCard4         c4Master
                                , const tCIDLib::TCard4         c4Enclosure
                                , const tCIDLib::TCard4         c4Module
                                , const tCIDLib::TCard4         c4Load
                                , const tVantageQS::ELoadTypes  eType) :

    TVantageQSItem(strName, c4Master, c4Enclosure, c4Module, c4Load)
    , m_eType(eType)
{
}

TVantageQSLoad::TVantageQSLoad(const TVantageQSLoad& itemToCopy) :

    TVantageQSItem(itemToCopy)
    , m_eType(itemToCopy.m_eType)
{
}

TVantageQSLoad::~TVantageQSLoad()
{
}


// ---------------------------------------------------------------------------
//  TVantageQSLoad: Public operators
// ---------------------------------------------------------------------------
TVantageQSLoad&
TVantageQSLoad::operator=(const TVantageQSLoad& itemToAssign)
{
    if (this != &itemToAssign)
    {
        TParent::operator=(itemToAssign);
        m_eType = itemToAssign.m_eType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TVantageQSLoad: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVantageQSLoad::bOwnsField(const tCIDLib::TCard4) const
{
    // We have no fields, so just say no
    return kCIDLib::False;
}


