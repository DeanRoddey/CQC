//
// FILE NAME: CQCSerUtils_Maps.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This file implements the map related classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TGenProtoMapItem,TObject)
RTTIDecls(TGenProtoMap,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapItem
//  PREFIX: mapi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMapItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoMapItem::TGenProtoMapItem() :

    m_evalItem(tGenProtoS::ETypes::Boolean)
    , m_hshKey(0)
    , m_strKey()
{
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&            strKey
                                    , const tCIDLib::TBoolean   bValue) :

    m_evalItem(tGenProtoS::ETypes::Boolean)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.bValue(bValue);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TCard1 c1Value) :

    m_evalItem(tGenProtoS::ETypes::Card1)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.c1Value(c1Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TCard2 c2Value) :

    m_evalItem(tGenProtoS::ETypes::Card2)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.c2Value(c2Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TCard4 c4Value) :

    m_evalItem(tGenProtoS::ETypes::Card2)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.c4Value(c4Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&            strKey
                                    , const tCIDLib::TFloat4    f4Value) :

    m_evalItem(tGenProtoS::ETypes::Float4)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.f4Value(f4Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&            strKey
                                    , const tCIDLib::TFloat8    f8Value) :

    m_evalItem(tGenProtoS::ETypes::Float8)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.f8Value(f8Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TInt1  i1Value) :

    m_evalItem(tGenProtoS::ETypes::Int1)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.i1Value(i1Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TInt2  i2Value) :

    m_evalItem(tGenProtoS::ETypes::Int2)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.i2Value(i2Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString&        strKey
                                    , const tCIDLib::TInt4  i4Value) :

    m_evalItem(tGenProtoS::ETypes::Int4)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.i4Value(i4Value);
}

TGenProtoMapItem::TGenProtoMapItem( const   TString& strKey
                                    , const TString& strValue) :

    m_evalItem(tGenProtoS::ETypes::String)
    , m_hshKey(strKey.hshCalcHash(kGenProtoS_::c4Modulus))
    , m_strKey(strKey)
{
    m_evalItem.strValue(strValue);
}

TGenProtoMapItem::~TGenProtoMapItem()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoMapItem: Public operators
// ---------------------------------------------------------------------------
TGenProtoMapItem&
TGenProtoMapItem::operator=(const TGenProtoMapItem& mapiToAssign)
{
    if (this != &mapiToAssign)
    {
        m_evalItem  = mapiToAssign.m_evalItem;
        m_hshKey    = mapiToAssign.m_hshKey;
        m_strKey    = mapiToAssign.m_strKey;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapItem: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TGenProtoExprVal& TGenProtoMapItem::evalItem() const
{
    return m_evalItem;
}


tGenProtoS::ETypes TGenProtoMapItem::eType() const
{
    return m_evalItem.eType();
}


tCIDLib::THashVal TGenProtoMapItem::hshKey() const
{
    return m_hshKey;
}


const TString& TGenProtoMapItem::strKey() const
{
    return m_strKey;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapItem: Hidden constructors and destructors
// ---------------------------------------------------------------------------
TGenProtoMapItem::TGenProtoMapItem(const TGenProtoMapItem& mapiToCopy) :

    m_evalItem(mapiToCopy.m_evalItem)
    , m_hshKey(mapiToCopy.m_hshKey)
    , m_strKey(mapiToCopy.m_strKey)
{
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMap
//  PREFIX: map
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMap: Public, static methods
// ---------------------------------------------------------------------------
const TString& TGenProtoMap::strKey(const TGenProtoMap& mapSrc)
{
    return mapSrc.m_strName;
}


// ---------------------------------------------------------------------------
//  TGenProtoMap: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoMap::TGenProtoMap( const   TString&            strName
                            , const tGenProtoS::ETypes  eItemType) :

    m_bHaveElseClause(kCIDLib::False)
    , m_colItems(64)
    , m_eItemType(eItemType)
    , m_strName(strName)
{
}

TGenProtoMap::TGenProtoMap(const TGenProtoMap& mapToCopy) :

    m_bHaveElseClause(kCIDLib::False)
    , m_colItems(mapToCopy.m_colItems)
    , m_eItemType(mapToCopy.m_eItemType)
    , m_mapiElse(mapToCopy.m_mapiElse)
    , m_strName(mapToCopy.m_strName)
{
}

TGenProtoMap::~TGenProtoMap()
{
    m_colItems.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TGenProtoMap: Public operators
// ---------------------------------------------------------------------------
TGenProtoMap& TGenProtoMap::operator=(const TGenProtoMap& mapToAssign)
{
    if (this != &mapToAssign)
    {
        m_bHaveElseClause   = mapToAssign.m_bHaveElseClause;
        m_colItems          = mapToAssign.m_colItems;
        m_eItemType         = mapToAssign.m_eItemType;
        m_mapiElse          = mapToAssign.m_mapiElse;
        m_strName           = mapToAssign.m_strName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoMap: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoMap::AddItem(const TGenProtoMapItem& mapiToAdd)
{
    // Make sure its of the correct type
    if (mapiToAdd.eType() != m_eItemType)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcData_BadMapItemType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , m_strName
            , mapiToAdd.clsIsA()
        );
    }

    // Looks ok, so add it
    m_colItems.objAdd(mapiToAdd);
}


tCIDLib::TVoid TGenProtoMap::AddElseItem(const TGenProtoMapItem& mapiToAdd)
{
    // Make sure its of the correct type
    if (mapiToAdd.eType() != m_eItemType)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcData_BadMapItemType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , m_strName
            , mapiToAdd.clsIsA()
        );
    }

    // Looks ok, so set it as the else item, and indicate we have an else
    m_mapiElse = mapiToAdd;
    m_bHaveElseClause = kCIDLib::True;
}


// All of these will throw if the item isn't found
tCIDLib::TBoolean TGenProtoMap::bItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Boolean);
    return pmapiVal->evalItem().bValue();
}


//
//  Builds an enum limit into the passed string, based on the keys of
//  of our items.
//
tCIDLib::TVoid TGenProtoMap::BuildLimString(TString& strToFill)
{
    strToFill = L"Enum: ";
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strToFill.Append(L", ");
        strToFill.Append(m_colItems[c4Index].strKey());
    }

    // If there's an 'else' clause, add it also.
    if (m_bHaveElseClause)
    {
        if (c4Count)
            strToFill.Append(L", ");
        strToFill.Append(m_mapiElse.strKey());
    }
}


tCIDLib::TCard1 TGenProtoMap::c1ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Card1);
    return pmapiVal->evalItem().c1Value();
}


tCIDLib::TCard2 TGenProtoMap::c2ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Card2);
    return pmapiVal->evalItem().c2Value();
}


tCIDLib::TCard4 TGenProtoMap::c4ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Card4);
    return pmapiVal->evalItem().c4Value();
}


tGenProtoS::ETypes TGenProtoMap::eItemType() const
{
    return m_eItemType;
}


tCIDLib::TFloat4 TGenProtoMap::f4ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Float4);
    return pmapiVal->evalItem().f4Value();
}


tCIDLib::TFloat8 TGenProtoMap::f8ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Float8);
    return pmapiVal->evalItem().f8Value();
}


tCIDLib::TInt1 TGenProtoMap::i1ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Int1);
    return pmapiVal->evalItem().i1Value();
}


tCIDLib::TInt2 TGenProtoMap::i2ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Int2);
    return pmapiVal->evalItem().i2Value();
}


tCIDLib::TInt4 TGenProtoMap::i4ItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::Int4);
    return pmapiVal->evalItem().i4Value();
}


TGenProtoMapItem*
TGenProtoMap::pmapiFind(const   TString&            strName
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    if (!c4Count)
        return 0;

    // Hash the passed name
    const tCIDLib::THashVal hshName = strName.hshCalcHash(kGenProtoS_::c4Modulus);

    // See if we can find this guy
    tCIDLib::TCard4 c4Index;
    TGenProtoMapItem* pmapiRet;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pmapiRet = &m_colItems[c4Index];
        if ((hshName == pmapiRet->hshKey()) && (strName == pmapiRet->strKey()))
            break;
    }

    // If not found, and we have an else clause, use the else item
    if (c4Index == c4Count)
    {
        if (m_bHaveElseClause)
        {
            pmapiRet = &m_mapiElse;
        }
         else
        {
            if (bThrowIfNot)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_MapItemNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , m_strName
                    , strName
                );
            }
            pmapiRet = 0;
        }
    }
    return pmapiRet;
}

const TGenProtoMapItem*
TGenProtoMap::pmapiFind(const   TString&            strName
                        , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    if (!c4Count)
        return 0;

    // Hash the passed name
    const tCIDLib::THashVal hshName = strName.hshCalcHash(kGenProtoS_::c4Modulus);

    // See if we can find this guy
    const TGenProtoMapItem* pmapiRet = 0;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pmapiRet = &m_colItems[c4Index];
        if ((hshName == pmapiRet->hshKey()) && (strName == pmapiRet->strKey()))
            break;
    }

    // If not found, and we have an else clause, use the else item
    if (c4Index == c4Count)
    {
        if (m_bHaveElseClause)
        {
            pmapiRet = &m_mapiElse;
        }
         else
        {
            if (bThrowIfNot)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_MapItemNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , m_strName
                    , strName
                );
            }
            pmapiRet = 0;
        }
    }
    return pmapiRet;
}


const TGenProtoMapItem*
TGenProtoMap::pmapiFindByValue( const   TGenProtoExprVal&   evalToFind
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    if (!c4Count)
        return 0;

    // See if we can find this guy
    const TGenProtoMapItem* pmapiRet = 0;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pmapiRet = &m_colItems[c4Index];
        if (pmapiRet->evalItem().bIsEqual(evalToFind))
            break;
    }

    if (c4Index == c4Count)
    {
        if (m_bHaveElseClause)
        {
            pmapiRet = &m_mapiElse;
        }
         else
        {
            if (bThrowIfNot)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_MapValueNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , m_strName
                    , evalToFind.strValue()
                );
            }
            pmapiRet = 0;
        }
    }
    return pmapiRet;
}

TGenProtoMapItem*
TGenProtoMap::pmapiFindByValue( const   TGenProtoExprVal&   evalToFind
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    if (!c4Count)
        return 0;

    // See if we can find this guy
    TGenProtoMapItem* pmapiRet = 0;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        pmapiRet = &m_colItems[c4Index];
        if (pmapiRet->evalItem().bIsEqual(evalToFind))
            break;
    }

    if (c4Index == c4Count)
    {
        if (m_bHaveElseClause)
        {
            pmapiRet = &m_mapiElse;
        }
         else
        {
            if (bThrowIfNot)
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_MapValueNotFound
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotFound
                    , m_strName
                    , evalToFind.strValue()
                );
            }
            pmapiRet = 0;
        }
    }
    return pmapiRet;
}


const TString& TGenProtoMap::strItemValue(const TString& strKey) const
{
    const TGenProtoMapItem* pmapiVal = pmapiFind(strKey, kCIDLib::True);
    CheckType(*pmapiVal, tGenProtoS::ETypes::String);
    return pmapiVal->evalItem().strValue();
}


const TString& TGenProtoMap::strName() const
{
    return m_strName;
}


// ---------------------------------------------------------------------------
//  TGenProtoMap: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoMap::CheckType(const   TGenProtoMapItem&   mapiToCheck
                        , const tGenProtoS::ETypes  eToCheck) const
{
    facGenProtoS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kGPDErrs::errcRunT_WrongMapType
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::TypeMatch
        , m_strName
        , TFacGenProtoS::strXlatType(eToCheck)
    );
}



