//
// FILE NAME: CQCKit_FieldFilter.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2008
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
//  This file implements a set of filters that filter out fields based on
//  various critera.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldFilter,TObject)
RTTIDecls(TCQCFldFiltNull, TCQCFldFilter)
RTTIDecls(TCQCFldFiltNumeric, TCQCFldFilter)
RTTIDecls(TCQCFldFiltNumRange, TCQCFldFilter)
RTTIDecls(TCQCFldFiltLimTypes, TCQCFldFilter)
RTTIDecls(TCQCFldFiltTypeName, TCQCFldFilter)
RTTIDecls(TCQCFldFiltDevCat, TCQCFldFilter)
RTTIDecls(TCQCFldFiltDevCls, TCQCFldFilter)
RTTIDecls(TCQCFldFiltEnumLim, TCQCFldFilter)



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFilter
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFilter::TCQCFldFilter(const tCQCKit::EReqAccess eReqAccess) :

    m_eReqAccess(eReqAccess)
{
}

TCQCFldFilter::TCQCFldFilter(const TCQCFldFilter& ffiltSrc) :

    m_eReqAccess(ffiltSrc.m_eReqAccess)
    , m_strFldRej(ffiltSrc.m_strFldRej)
    , m_strMonRej(ffiltSrc.m_strMonRej)
{
}

TCQCFldFilter::~TCQCFldFilter()
{
}



// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public operators
// ---------------------------------------------------------------------------

TCQCFldFilter&
TCQCFldFilter::operator=(const TCQCFldFilter& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        m_eReqAccess = ffiltSrc.m_eReqAccess;
        m_strFldRej  = ffiltSrc.m_strFldRej;
        m_strMonRej  = ffiltSrc.m_strMonRej;
    }
    return *this;
}


tCIDLib::TBoolean
TCQCFldFilter::operator==(const TCQCFldFilter& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    return (m_eReqAccess == ffiltSrc.m_eReqAccess)
        && (m_strFldRej == ffiltSrc.m_strFldRej)
        && (m_strMonRej == ffiltSrc.m_strMonRej);
}


tCIDLib::TBoolean
TCQCFldFilter::operator!=(const TCQCFldFilter& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFilter::bCanAcceptField( const   TCQCFldDef&     flddToCheck
                                , const TString&        strMoniker
                                , const TString&
                                , const TString&
                                , const tCQCKit::EDevCats
                                , const tCQCKit::TDevClassList&) const
{
    // Check the rejects if we have them
    if (!m_strMonRej.bIsEmpty() && !m_strFldRej.bIsEmpty())
    {
        // They both have to match to be rejected
        if ((strMoniker == m_strMonRej) && (flddToCheck.strName() == m_strFldRej))
            return kCIDLib::False;
    }
     else if (!m_strMonRej.bIsEmpty())
    {
        // Just the moniker has to match to be rejected
        if (strMoniker == m_strMonRej)
            return kCIDLib::False;
    }
     else if (!m_strFldRej.bIsEmpty())
    {
        // Just the field has to match to be rejected
        if (flddToCheck.strName() == m_strFldRej)
            return kCIDLib::False;
    }

    // Test for requested access
    return facCQCKit().bCheckFldAccess(m_eReqAccess, flddToCheck.eAccess());
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Set the moniker and/or field to reject
tCIDLib::TVoid
TCQCFldFilter::SetReject(const  TString&    strMoniker
                        , const TString&    strField)
{
    m_strFldRej = strField;
    m_strMonRej = strMoniker;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNull
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFiltNull::TCQCFldFiltNull() :

    TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite)
{
}

TCQCFldFiltNull::
TCQCFldFiltNull(const TCQCFldFiltNull& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
{
}

TCQCFldFiltNull::~TCQCFldFiltNull()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltNull&
TCQCFldFiltNull::operator=(const TCQCFldFiltNull& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltNull::operator==(const TCQCFldFiltNull& ffiltSrc) const
{
    // We can just say it always matches
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCFldFiltNull::operator!=(const TCQCFldFiltNull& ffiltSrc) const
{
    // We can just say it always matches
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltNull::bCanAcceptField(const TCQCFldDef&
                                , const TString&
                                , const TString&
                                , const TString&
                                , const tCQCKit::EDevCats
                                , const tCQCKit::TDevClassList&) const
{
    // We just always say yes
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNumeric
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFiltNumeric::TCQCFldFiltNumeric( const   tCQCKit::EReqAccess eReqAccess
                                        , const tCIDLib::TBoolean   bAllowFloat) :

    TCQCFldFilter(eReqAccess)
    , m_bAllowFloat(bAllowFloat)
{
}

TCQCFldFiltNumeric::TCQCFldFiltNumeric(const TCQCFldFiltNumeric& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_bAllowFloat(ffiltSrc.m_bAllowFloat)
{
}

TCQCFldFiltNumeric::~TCQCFldFiltNumeric()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltNumeric&
TCQCFldFiltNumeric::operator=(const TCQCFldFiltNumeric& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_bAllowFloat = ffiltSrc.m_bAllowFloat;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltNumeric::operator==(const TCQCFldFiltNumeric& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_bAllowFloat == ffiltSrc.m_bAllowFloat);
}

tCIDLib::TBoolean
TCQCFldFiltNumeric::operator!=(const TCQCFldFiltNumeric& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltNumeric::bCanAcceptField(const   TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Let our parent check first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    // We passd that so check that it's a numeric type
    return
    (
        (flddCheck.eType() == tCQCKit::EFldTypes::Card)
        ||  (flddCheck.eType() == tCQCKit::EFldTypes::Int)
        ||  (m_bAllowFloat && (flddCheck.eType() == tCQCKit::EFldTypes::Float))
    );
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltNumRange
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFilter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFiltNumRange::TCQCFldFiltNumRange(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCIDLib::TFloat8    f8MinRange
                                        , const tCIDLib::TFloat8    f8MaxRange
                                        , const tCIDLib::TBoolean   bAllowFloat) :

    TCQCFldFilter(eReqAccess)
    , m_bAllowFloat(bAllowFloat)
    , m_eSemType(tCQCKit::EFldSTypes::Count)
    , m_f8MaxRange(f8MaxRange)
    , m_f8MinRange(f8MinRange)
{
}

TCQCFldFiltNumRange::TCQCFldFiltNumRange(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCIDLib::TFloat8    f8MinRange
                                        , const tCIDLib::TFloat8    f8MaxRange
                                        , const tCQCKit::EFldSTypes eSemType
                                        , const tCIDLib::TBoolean   bAllowFloat) :

    TCQCFldFilter(eReqAccess)
    , m_bAllowFloat(bAllowFloat)
    , m_eSemType(eSemType)
    , m_f8MaxRange(f8MaxRange)
    , m_f8MinRange(f8MinRange)
{
}

TCQCFldFiltNumRange::
TCQCFldFiltNumRange(const TCQCFldFiltNumRange& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_bAllowFloat(ffiltSrc.m_bAllowFloat)
    , m_eSemType(ffiltSrc.m_eSemType)
    , m_f8MaxRange(ffiltSrc.m_f8MaxRange)
    , m_f8MinRange(ffiltSrc.m_f8MinRange)
{
}

TCQCFldFiltNumRange::~TCQCFldFiltNumRange()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltNumRange&
TCQCFldFiltNumRange::operator=(const TCQCFldFiltNumRange& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_bAllowFloat = ffiltSrc.m_bAllowFloat;
        m_eSemType    = ffiltSrc.m_eSemType;
        m_f8MaxRange  = ffiltSrc.m_f8MaxRange;
        m_f8MinRange  = ffiltSrc.m_f8MinRange;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltNumRange::operator==(const TCQCFldFiltNumRange& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_bAllowFloat == ffiltSrc.m_bAllowFloat)
        && (m_eSemType    == ffiltSrc.m_eSemType)
        && (m_f8MaxRange  == ffiltSrc.m_f8MaxRange)
        && (m_f8MinRange  == ffiltSrc.m_f8MinRange);
}

tCIDLib::TBoolean
TCQCFldFiltNumRange::operator!=(const TCQCFldFiltNumRange& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  TCQCFldFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltNumRange::bCanAcceptField(const  TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    // If we are checking semantic type, do that
    if ((m_eSemType != tCQCKit::EFldSTypes::Count)
    &&  (m_eSemType != flddCheck.eSemType()))
    {
        return kCIDLib::False;
    }

    // Check that it's a numeric field with a range within the min/max
    tCIDLib::TFloat8 f8Min;
    tCIDLib::TFloat8 f8Max;
    return facCQCKit().bCheckNumRangeFld
    (
        flddCheck, m_f8MinRange, m_f8MaxRange, m_bAllowFloat, f8Min, f8Max
    );
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltLimTypes
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFiltLimTypes: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFiltLimTypes::TCQCFldFiltLimTypes(const  tCQCKit::EReqAccess eReqAccess
                                        , const TVector<TString>&   colLimitTypes) :

    TCQCFldFilter(eReqAccess)
    , m_colLimitTypes(colLimitTypes)
{
}

TCQCFldFiltLimTypes::TCQCFldFiltLimTypes(const TCQCFldFiltLimTypes& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_colLimitTypes(ffiltSrc.m_colLimitTypes)
{
}

TCQCFldFiltLimTypes::~TCQCFldFiltLimTypes()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltLimTypes&
TCQCFldFiltLimTypes::operator=(const TCQCFldFiltLimTypes& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_colLimitTypes = ffiltSrc.m_colLimitTypes;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltLimTypes::operator==(const TCQCFldFiltLimTypes& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_colLimitTypes == ffiltSrc.m_colLimitTypes);
}

tCIDLib::TBoolean
TCQCFldFiltLimTypes::operator!=(const TCQCFldFiltLimTypes& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltLimTypes::bCanAcceptField(const  TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    const tCIDLib::TCard4 c4LimitCount = m_colLimitTypes.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4LimitCount; c4Index++)
    {
        const TString strCur = m_colLimitTypes[c4Index];
        if (flddCheck.strLimits().bStartsWith(strCur))
            break;
    }

    // If we didn't go all the way through the list, then we got a match
    return (c4Index < c4LimitCount);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltTypeName
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFiltTypeName: Constructors and Destructor
// ---------------------------------------------------------------------------

// Just access and type
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCQCKit::EFldTypes  eType) :
    TCQCFldFilter(eReqAccess)
    , m_eType(eType)
{
}

// Access type and sem type
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCQCKit::EFldTypes  eType
                                        , const tCQCKit::EFldSTypes eSemType) :
    TCQCFldFilter(eReqAccess)
    , m_eType(eType)
{
    m_fcolSTypes.c4AddElement(eSemType);
}

// Just acess type and a set of semantic types
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess     eReqAccess
                                        , const tCQCKit::TFldSTypeList& fcolSTypes) :
    TCQCFldFilter(eReqAccess)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_fcolSTypes(fcolSTypes)
{
}

// Filter on regular field type, ignore semantic type
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCQCKit::EFldTypes  eType
                                        , const TString&            strName1
                                        , const TString&            strName2) :
    TCQCFldFilter(eReqAccess)
    , m_eType(eType)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
}

// Filter on semantic type(s), ignore regular type
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCQCKit::EFldSTypes eSemType
                                        , const TString&            strName1
                                        , const TString&            strName2) :
    TCQCFldFilter(eReqAccess)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_fcolSTypes(1UL)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
    m_fcolSTypes.c4AddElement(eSemType);
}

TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess     eReqAccess
                                        , const tCQCKit::TFldSTypeList& fcolSTypes
                                        , const TString&                strName1
                                        , const TString&                strName2) :
    TCQCFldFilter(eReqAccess)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_fcolSTypes(fcolSTypes)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
}

// Filter on both types
TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess eReqAccess
                                        , const tCQCKit::EFldTypes  eType
                                        , const tCQCKit::EFldSTypes eSemType
                                        , const TString&            strName1
                                        , const TString&            strName2) :
    TCQCFldFilter(eReqAccess)
    , m_eType(eType)
    , m_fcolSTypes(1UL)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
    m_fcolSTypes.c4AddElement(eSemType);
}

TCQCFldFiltTypeName::TCQCFldFiltTypeName(const  tCQCKit::EReqAccess     eReqAccess
                                        , const tCQCKit::EFldTypes      eType
                                        , const tCQCKit::TFldSTypeList& fcolSTypes
                                        , const TString&                strName1
                                        , const TString&                strName2) :
    TCQCFldFilter(eReqAccess)
    , m_eType(eType)
    , m_fcolSTypes(fcolSTypes)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
}

TCQCFldFiltTypeName::TCQCFldFiltTypeName(const TCQCFldFiltTypeName& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_eType(ffiltSrc.m_eType)
    , m_fcolSTypes(ffiltSrc.m_fcolSTypes)
    , m_strName1(ffiltSrc.m_strName1)
    , m_strName2(ffiltSrc.m_strName2)
{
}

TCQCFldFiltTypeName::~TCQCFldFiltTypeName()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltTypeName&
TCQCFldFiltTypeName::operator=(const TCQCFldFiltTypeName& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_eType = ffiltSrc.m_eType;
        m_fcolSTypes = ffiltSrc.m_fcolSTypes;
        m_strName1 = ffiltSrc.m_strName1;
        m_strName2 = ffiltSrc.m_strName2;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltTypeName::operator==(const TCQCFldFiltTypeName& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_strName1 == ffiltSrc.m_strName1)
            && (m_strName2 == ffiltSrc.m_strName2)
            && (m_fcolSTypes == ffiltSrc.m_fcolSTypes)
            && (m_eType == ffiltSrc.m_eType);
}

tCIDLib::TBoolean
TCQCFldFiltTypeName::operator!=(const TCQCFldFiltTypeName& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltTypeName::bCanAcceptField(const  TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Check the base class criteria first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    //
    //  If the semantic types list is not empty, then see if we have this
    //  type. If not, we failed.
    //
    if (!m_fcolSTypes.bIsEmpty() && !m_fcolSTypes.bElementPresent(flddCheck.eSemType()))
        return kCIDLib::False;

    // If the regular field type isn't 'count', then compare that
    if ((m_eType != tCQCKit::EFldTypes::Count)
    &&  (flddCheck.eType() != m_eType))
    {
        return kCIDLib::False;
    }

    //
    //  If we have any names, then the passed field must match one of them
    //  or it fails. Only check against our non-empty ones.
    //
    tCIDLib::TBoolean bRet(m_strName1.bIsEmpty() && m_strName2.bIsEmpty());
    if (!bRet)
    {
        bRet = !m_strName1.bIsEmpty() && (flddCheck.strName() == m_strName1);
        if (!bRet)
            bRet = !m_strName2.bIsEmpty() && (flddCheck.strName() == m_strName2);
    }
    return bRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltDevCat
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCat: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCFldFiltDevCat::TCQCFldFiltDevCat(const  TString&            strName1
                                    , const TString&            strName2
                                    , const tCQCKit::EDevCats   eDevCat
                                    , const tCQCKit::EFldTypes  eType
                                    , const tCQCKit::EReqAccess eReqAccess) :
    TCQCFldFilter(eReqAccess)
    , m_eDevCat(eDevCat)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_strName1(strName1)
    , m_strName2(strName2)
{
}

TCQCFldFiltDevCat::TCQCFldFiltDevCat(const TCQCFldFiltDevCat& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_eDevCat(ffiltSrc.m_eDevCat)
    , m_eType(ffiltSrc.m_eType)
    , m_strName1(ffiltSrc.m_strName1)
    , m_strName2(ffiltSrc.m_strName2)
{
}

TCQCFldFiltDevCat::~TCQCFldFiltDevCat()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltDevCat&
TCQCFldFiltDevCat::operator=(const TCQCFldFiltDevCat& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_eDevCat   = ffiltSrc.m_eDevCat;
        m_eType     = ffiltSrc.m_eType;
        m_strName1  = ffiltSrc.m_strName1;
        m_strName2  = ffiltSrc.m_strName2;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltDevCat::operator==(const TCQCFldFiltDevCat& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_strName1 == ffiltSrc.m_strName1)
            && (m_strName2 == ffiltSrc.m_strName2)
            && (m_eDevCat == ffiltSrc.m_eDevCat)
            && (m_eType == ffiltSrc.m_eType);
}

tCIDLib::TBoolean
TCQCFldFiltDevCat::operator!=(const TCQCFldFiltDevCat& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltDevCat::bCanAcceptField( const   TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Check the base class criteria first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    // Check the device category
    if (m_eDevCat != eCat)
        return kCIDLib::False;

    // If the regular field type isn't 'count', then compare that
    if ((m_eType != tCQCKit::EFldTypes::Count)
    &&  (flddCheck.eType() != m_eType))
    {
        return kCIDLib::False;
    }

    //
    //  If we have any names, then the passed field must match one of them
    //  or it fails. Only check against our non-empty ones.
    //
    tCIDLib::TBoolean bRet(m_strName1.bIsEmpty() && m_strName2.bIsEmpty());
    if (!bRet)
    {
        bRet = !m_strName1.bIsEmpty() && (flddCheck.strName() == m_strName1);
        if (!bRet)
            bRet = !m_strName2.bIsEmpty() && (flddCheck.strName() == m_strName2);
    }
    return bRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltDevCls
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCls: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFiltDevCls::TCQCFldFiltDevCls(const  tCQCKit::EDevClasses    eDevCls
                                    , const tCQCKit::EFldSTypes     eSType
                                    , const tCQCKit::EFldTypes      eType
                                    , const tCQCKit::EReqAccess     eReqAccess) :
    TCQCFldFilter(eReqAccess)
    , m_bMatchAll(kCIDLib::True)
    , m_eType(tCQCKit::EFldTypes::Count)
{
    m_fcolClasses.c4AddElement(eDevCls);
    m_fcolSTypes.c4AddElement(eSType);
}

TCQCFldFiltDevCls::TCQCFldFiltDevCls(const  tCQCKit::EDevClasses    eDevCls
                                    , const tCQCKit::TFldSTypeList& fcolSTypes
                                    , const tCQCKit::EFldTypes      eType
                                    , const tCQCKit::EReqAccess     eReqAccess) :
    TCQCFldFilter(eReqAccess)
    , m_bMatchAll(kCIDLib::True)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_fcolSTypes(fcolSTypes)
{
    m_fcolClasses.c4AddElement(eDevCls);
}

TCQCFldFiltDevCls::TCQCFldFiltDevCls(const  tCQCKit::TDevClassList& fcolList
                                    , const tCIDLib::TBoolean       bMatchAll
                                    , const tCQCKit::TFldSTypeList& fcolSTypes
                                    , const tCQCKit::EFldTypes      eType
                                    , const tCQCKit::EReqAccess     eReqAccess) :
    TCQCFldFilter(eReqAccess)
    , m_bMatchAll(bMatchAll)
    , m_eType(tCQCKit::EFldTypes::Count)
    , m_fcolClasses(fcolList)
    , m_fcolSTypes(fcolSTypes)
{
}

TCQCFldFiltDevCls::TCQCFldFiltDevCls(const TCQCFldFiltDevCls& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_bMatchAll(ffiltSrc.m_bMatchAll)
    , m_eType(ffiltSrc.m_eType)
    , m_fcolClasses(ffiltSrc.m_fcolClasses)
    , m_fcolSTypes(ffiltSrc.m_fcolSTypes)
    , m_strMoniker(ffiltSrc.m_strMoniker)
{
}

TCQCFldFiltDevCls::~TCQCFldFiltDevCls()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCls: Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltDevCls& TCQCFldFiltDevCls::operator=(const TCQCFldFiltDevCls& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_bMatchAll     = ffiltSrc.m_bMatchAll;
        m_eType         = ffiltSrc.m_eType;
        m_fcolClasses   = ffiltSrc.m_fcolClasses;
        m_fcolSTypes    = ffiltSrc.m_fcolSTypes;
        m_strMoniker    = ffiltSrc.m_strMoniker;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltDevCls::operator==(const TCQCFldFiltDevCls& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_eType == ffiltSrc.m_eType)
           && (m_fcolClasses == ffiltSrc.m_fcolClasses)
           && (m_fcolSTypes == ffiltSrc.m_fcolSTypes)
           && (m_strMoniker == ffiltSrc.m_strMoniker);
}

tCIDLib::TBoolean
TCQCFldFiltDevCls::operator!=(const TCQCFldFiltDevCls& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCls: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltDevCls::bCanAcceptField( const   TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Check the base class criteria first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    // If our monike is not empty, it has to match
    if (!m_strMoniker.bIsEmpty() && (m_strMoniker != strMon))
        return kCIDLib::False;

    // If the field type isn't 'count', then compare that
    if ((m_eType != tCQCKit::EFldTypes::Count)
    &&  (flddCheck.eType() != m_eType))
    {
        return kCIDLib::False;
    }

    // If our semantic type list isn't empty, make sure it is in that list
    if (!m_fcolSTypes.bIsEmpty() && !m_fcolSTypes.bElementPresent(flddCheck.eSemType()))
        return kCIDLib::False;

    //
    //  And lastly, see if the device implements the required device classes
    //
    tCIDLib::TCard4 c4Matches = 0;

    const tCIDLib::TCard4 c4Count = m_fcolClasses.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (fcolCls.bElementPresent(m_fcolClasses[c4Index]))
        {
            // If not matching all, then one match is sufficent
            c4Matches++;
            if (!m_bMatchAll)
                break;
        }
         else
        {
            // If matching all, one failure is sufficient
            if (m_bMatchAll)
                break;
        }
    }

    // If matching all, then matches has to equal count
    if (m_bMatchAll)
        return (c4Matches == c4Count);

    // Else it just has to be greater than 0
    return (c4Matches > 0);
}


// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCls: Public, non-virtual methods
// ---------------------------------------------------------------------------

const TString& TCQCFldFiltDevCls::strMoniker() const
{
    return m_strMoniker;
}

const TString& TCQCFldFiltDevCls::strMoniker(const TString& strToSet)
{
    m_strMoniker = strToSet;
    return m_strMoniker;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFiltEnumLim
//  PREFIX: ffilt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFiltDevCat: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQCFldFiltEnumLim::TCQCFldFiltEnumLim( const   tCIDLib::TCard4     c4MinVals
                                        , const tCIDLib::TCard4     c4MaxVals
                                        , const tCQCKit::EReqAccess eReqAccess) :
    TCQCFldFilter(eReqAccess)
    , m_c4MaxVals(c4MaxVals)
    , m_c4MinVals(c4MinVals)
{
    // Flip the values if they are backwards
    if (m_c4MinVals > m_c4MaxVals)
    {
        const tCIDLib::TCard4 c4Tmp = m_c4MinVals;
        m_c4MinVals = m_c4MaxVals;
        m_c4MaxVals = c4Tmp;
    }
}

TCQCFldFiltEnumLim::TCQCFldFiltEnumLim(const TCQCFldFiltEnumLim& ffiltSrc) :

    TCQCFldFilter(ffiltSrc)
    , m_c4MaxVals(ffiltSrc.m_c4MaxVals)
    , m_c4MinVals(ffiltSrc.m_c4MinVals)
{
}

TCQCFldFiltEnumLim::~TCQCFldFiltEnumLim()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TCQCFldFiltEnumLim&
TCQCFldFiltEnumLim::operator=(const TCQCFldFiltEnumLim& ffiltSrc)
{
    if (&ffiltSrc != this)
    {
        TParent::operator=(ffiltSrc);
        m_c4MaxVals = ffiltSrc.m_c4MaxVals;
        m_c4MinVals = ffiltSrc.m_c4MinVals;
    }
    return *this;
}

tCIDLib::TBoolean
TCQCFldFiltEnumLim::operator==(const TCQCFldFiltEnumLim& ffiltSrc) const
{
    if (&ffiltSrc == this)
        return kCIDLib::True;

    if (!TParent::operator==(ffiltSrc))
        return kCIDLib::False;

    return (m_c4MaxVals == ffiltSrc.m_c4MaxVals)
        && (m_c4MinVals == ffiltSrc.m_c4MinVals);
}

tCIDLib::TBoolean
TCQCFldFiltEnumLim::operator!=(const TCQCFldFiltEnumLim& ffiltSrc) const
{
    return !operator==(ffiltSrc);
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldFiltEnumLim::bCanAcceptField( const  TCQCFldDef&             flddCheck
                                    , const TString&                strMon
                                    , const TString&                strMake
                                    , const TString&                strModel
                                    , const tCQCKit::EDevCats       eCat
                                    , const tCQCKit::TDevClassList& fcolCls) const
{
    // Check the base class criteria first
    if (!TParent::bCanAcceptField(flddCheck, strMon, strMake, strModel, eCat, fcolCls))
        return kCIDLib::False;

    // It has to be an enumerated string type
    if (!flddCheck.bIsEnumType())
        return kCIDLib::False;

    //
    //  If either of the min or max values need to be checked then we have to
    //  the more piggy work of parsing out the limit and checking the count.
    //
    if ((m_c4MinVals != kCIDLib::c4MaxCard)
    ||  (m_c4MaxVals != kCIDLib::c4MaxCard))
    {
        // If we can't parse the limits, then just say no
        if (!m_fldlTmp.bParseLimits(flddCheck.strLimits()))
            return kCIDLib::False;

        // See if the count of values is acceptable
        const tCIDLib::TCard4 c4ValCnt = m_fldlTmp.c4ValCount();
        if (((m_c4MinVals != kCIDLib::c4MaxCard) && (c4ValCnt < m_c4MinVals))
        ||  ((m_c4MaxVals != kCIDLib::c4MaxCard) && (c4ValCnt > m_c4MaxVals)))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


