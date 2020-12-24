//
// FILE NAME: GenProtoS_ExprValues.cpp
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
//  This file implements the expression value smart union class.
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
RTTIDecls(TGenProtoExprVal,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExprVal
//  PREFIX: fval
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoExprVal:UValStorage: Constructors
// ---------------------------------------------------------------------------
TGenProtoExprVal::UValStorage::UValStorage() : f8Value(0) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TBoolean bInit) : bValue(bInit) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TCard1 c1Init) : c1Value(c1Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TCard2 c2Init) : c2Value(c2Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TCard4 c4Init) : c4Value(c4Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TFloat4 f4Init) : f4Value(f4Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TFloat8 f8Init) : f8Value(f8Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TInt1 i1Init) : i1Value(i1Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TInt2 i2Init) : i2Value(i2Init) {}
TGenProtoExprVal::UValStorage::UValStorage(const tCIDLib::TInt4 i4Init) : i4Value(i4Init) {}


// ---------------------------------------------------------------------------
//  TGenProtoExprVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExprVal::TGenProtoExprVal(const tGenProtoS::ETypes eType) :

    m_eType(eType)
    , m_pstrValue(0)
{
    InitVal();
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TBoolean bInitVal) :

    m_eType(tGenProtoS::ETypes::Boolean)
    , m_pstrValue(0)
    , m_uValue(bInitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TCard1 c1InitVal) :

    m_eType(tGenProtoS::ETypes::Card1)
    , m_pstrValue(0)
    , m_uValue(c1InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TCard2 c2InitVal) :

    m_eType(tGenProtoS::ETypes::Card2)
    , m_pstrValue(0)
    , m_uValue(c2InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TCard4 c4InitVal) :

    m_eType(tGenProtoS::ETypes::Card4)
    , m_pstrValue(0)
    , m_uValue(c4InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TFloat4 f4InitVal) :

    m_eType(tGenProtoS::ETypes::Float4)
    , m_pstrValue(0)
    , m_uValue(f4InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TFloat8 f8InitVal) :

    m_eType(tGenProtoS::ETypes::Float8)
    , m_pstrValue(0)
    , m_uValue(f8InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TInt1 i1InitVal) :

    m_eType(tGenProtoS::ETypes::Int1)
    , m_pstrValue(0)
    , m_uValue(i1InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TInt2 i2InitVal) :

    m_eType(tGenProtoS::ETypes::Int2)
    , m_pstrValue(0)
    , m_uValue(i2InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const tCIDLib::TInt4 i4InitVal) :

    m_eType(tGenProtoS::ETypes::Int4)
    , m_pstrValue(0)
    , m_uValue(i4InitVal)
{
}

TGenProtoExprVal::TGenProtoExprVal(const TString& strInitVal) :

    m_eType(tGenProtoS::ETypes::String)
    , m_pstrValue(new TString(strInitVal))
{
}

TGenProtoExprVal::TGenProtoExprVal(const tGenProtoS::ETokens eToken) :

    m_eType(TFacGenProtoS::eTokenToType(eToken))
    , m_pstrValue(0)
{
    InitVal();
}

TGenProtoExprVal::TGenProtoExprVal(const TGenProtoExprVal& fvalToCopy) :

    m_eType(fvalToCopy.m_eType)
    , m_pstrValue(0)
{
    //
    //  If our type is string, we have to deep copy the string. Else just
    //  assign the union value.
    //
    if (fvalToCopy.m_eType == tGenProtoS::ETypes::String)
        m_pstrValue = new TString(*fvalToCopy.m_pstrValue);
    else
        m_uValue = fvalToCopy.m_uValue;
}

TGenProtoExprVal::~TGenProtoExprVal()
{
    delete m_pstrValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoExprVal: Public operators
// ---------------------------------------------------------------------------
TGenProtoExprVal& TGenProtoExprVal::operator=(const TGenProtoExprVal& fvalToAssign)
{
    if (this != &fvalToAssign)
    {
        if (m_eType == tGenProtoS::ETypes::String)
        {
            if (fvalToAssign.m_eType != tGenProtoS::ETypes::String)
            {
                // And just copy the union and clear our string value
                m_uValue = fvalToAssign.m_uValue;
                m_pstrValue->Clear();
            }
             else
            {
                // He is a string too, so copy his text to our string
                *m_pstrValue = *fvalToAssign.m_pstrValue;
            }
        }
         else if (fvalToAssign.m_eType == tGenProtoS::ETypes::String)
        {
            //
            //  We are not a string type, but the source is, so we need to
            //  allocate a string and store his text.
            //
            m_pstrValue = new TString(*fvalToAssign.m_pstrValue);
        }
         else
        {
            // No special case, so just copy the union as is
            m_uValue  = fvalToAssign.m_uValue;
        }

        // Now we can store our new type
        m_eType = fvalToAssign.m_eType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoExprVal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenProtoExprVal::bCheckType(const tGenProtoS::ETypes eToCheck) const
{
    return (m_eType == eToCheck);
}


tCIDLib::TBoolean TGenProtoExprVal::bIsCardType() const
{
    return ((m_eType == tGenProtoS::ETypes::Card1)
           || (m_eType == tGenProtoS::ETypes::Card2)
           || (m_eType == tGenProtoS::ETypes::Card4));
}


tCIDLib::TBoolean TGenProtoExprVal::bIsCharType() const
{
    // Make sure it's a string type and that it's a single character
    return (m_eType == tGenProtoS::ETypes::String) && (m_pstrValue->c4Length() == 1);
}


tCIDLib::TBoolean
TGenProtoExprVal::bIsEqual(const TGenProtoExprVal& evalToCheck) const
{
    tCIDLib::TBoolean bRet;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Card1 :
        case tGenProtoS::ETypes::Card2 :
        case tGenProtoS::ETypes::Card4 :
        {
            bRet = (c4Value() == evalToCheck.c4Value());
            break;
        }

        case tGenProtoS::ETypes::Float4 :
        case tGenProtoS::ETypes::Float8 :
        {
            bRet = (f8Value() == evalToCheck.f8Value());
            break;
        }

        case tGenProtoS::ETypes::Int1 :
        case tGenProtoS::ETypes::Int2 :
        case tGenProtoS::ETypes::Int4 :
        {
            bRet = (i4Value() == evalToCheck.i4Value());
            break;
        }

        case tGenProtoS::ETypes::String :
            bRet = (strValue() == evalToCheck.strValue());
            break;

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(m_eType))
                , TString(L"tGenProtoS::EType")
            );
            break;
    };
    return bRet;
}


tCIDLib::TBoolean TGenProtoExprVal::bIsFloatType() const
{
    return ((m_eType == tGenProtoS::ETypes::Float4)
           || (m_eType == tGenProtoS::ETypes::Float8));
}


tCIDLib::TBoolean TGenProtoExprVal::bIsIntType() const
{
    return ((m_eType == tGenProtoS::ETypes::Int1)
           || (m_eType == tGenProtoS::ETypes::Int2)
           || (m_eType == tGenProtoS::ETypes::Int4));
}


tCIDLib::TBoolean TGenProtoExprVal::bIsZero() const
{
    tCIDLib::TBoolean bRet;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            bRet = m_uValue.bValue == kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            bRet = (m_uValue.c1Value == 0);
            break;

        case tGenProtoS::ETypes::Card2 :
            bRet = (m_uValue.c2Value == 0);
            break;

        case tGenProtoS::ETypes::Card4 :
            bRet = (m_uValue.bValue == 0);
            break;

        case tGenProtoS::ETypes::Float4 :
            bRet = (m_uValue.bValue == 0);
            break;

        case tGenProtoS::ETypes::Float8 :
            bRet = (m_uValue.bValue == 0);
            break;

        case tGenProtoS::ETypes::Int1 :
            bRet = (m_uValue.bValue == 0);
            break;

        case tGenProtoS::ETypes::Int2 :
            bRet = (m_uValue.bValue == 0);
            break;

        case tGenProtoS::ETypes::Int4 :
            bRet = (m_uValue.bValue == 0);
            break;

        default :
            bRet = kCIDLib::False;
            break;
    }
    return bRet;
}


tCIDLib::TBoolean TGenProtoExprVal::bValue() const
{
    tCIDLib::TBoolean bRet;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            bRet = m_uValue.bValue;
            break;

        case tGenProtoS::ETypes::Card1 :
            bRet = (m_uValue.c1Value != 0);
            break;

        case tGenProtoS::ETypes::Card2 :
            bRet = (m_uValue.c2Value != 0);
            break;

        case tGenProtoS::ETypes::Card4 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::Float4 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::Float8 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::Int1 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::Int2 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::Int4 :
            bRet = (m_uValue.bValue != 0);
            break;

        case tGenProtoS::ETypes::String :
            if ((*m_pstrValue == L"True")
            ||  (*m_pstrValue == L"1")
            ||  (*m_pstrValue == kCQCKit::pszFld_True))
            {
                bRet = kCIDLib::True;
            }
             else if ((*m_pstrValue == L"False")
                  ||  (*m_pstrValue == L"0")
                  ||  (*m_pstrValue == kCQCKit::pszFld_False))
            {
                bRet = kCIDLib::True;
            }
             else
            {
                BadCast(m_eType, tGenProtoS::ETypes::Boolean);
            }
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Boolean);
            break;
    }
    return bRet;
}

tCIDLib::TBoolean TGenProtoExprVal::bValue(const tCIDLib::TBoolean bToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = bToSet;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = bToSet ? 1.0F : 0;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = bToSet ? 1 : 0;
            break;

        case tGenProtoS::ETypes::String :
            *m_pstrValue = bToSet ? kCQCKit::pszFld_True : kCQCKit::pszFld_False;
            break;

        default :
            BadCast(tGenProtoS::ETypes::Boolean, m_eType);
            break;
    }
    return bToSet;
}


tCIDLib::TCard1 TGenProtoExprVal::c1Value() const
{
    tCIDLib::TCard1 c1Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            c1Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            c1Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            c1Ret = tCIDLib::TCard1(m_uValue.c2Value);
            break;

        case tGenProtoS::ETypes::Card4 :
            c1Ret = tCIDLib::TCard1(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            c1Ret = tCIDLib::TCard1(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            c1Ret = tCIDLib::TCard1(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            c1Ret = tCIDLib::TCard1(m_uValue.i1Value);
            break;

        case tGenProtoS::ETypes::Int2 :
            c1Ret = tCIDLib::TCard1(m_uValue.i2Value);
            break;

        case tGenProtoS::ETypes::Int4 :
            c1Ret = tCIDLib::TCard1(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            c1Ret = tCIDLib::TCard1(m_pstrValue->c4Val());
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Card1);
            break;
    };
    return c1Ret;
}

tCIDLib::TCard1 TGenProtoExprVal::c1Value(const tCIDLib::TCard1 c1ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = c1ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = c1ToSet;
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = c1ToSet;
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = c1ToSet;
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = c1ToSet;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = c1ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(c1ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(c1ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = tCIDLib::TInt4(c1ToSet);
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(tCIDLib::TCard4(c1ToSet));
            break;

        default :
            BadCast(tGenProtoS::ETypes::Card1, m_eType);
            break;
    };
    return c1ToSet;
}



tCIDLib::TCard2 TGenProtoExprVal::c2Value() const
{
    tCIDLib::TCard2 c2Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            c2Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            c2Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            c2Ret = m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            c2Ret = tCIDLib::TCard2(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            c2Ret = tCIDLib::TCard2(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            c2Ret = tCIDLib::TCard2(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            c2Ret = tCIDLib::TCard2(m_uValue.i1Value);
            break;

        case tGenProtoS::ETypes::Int2 :
            c2Ret = tCIDLib::TCard2(m_uValue.i2Value);
            break;

        case tGenProtoS::ETypes::Int4 :
            c2Ret = tCIDLib::TCard2(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            c2Ret = tCIDLib::TCard2(m_pstrValue->c4Val());
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Card2);
            break;
    };
    return c2Ret;
}

tCIDLib::TCard2 TGenProtoExprVal::c2Value(const tCIDLib::TCard2 c2ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = c2ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(c2ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = c2ToSet;
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = c2ToSet;
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = c2ToSet;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = c2ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(c2ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(c2ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = tCIDLib::TInt4(c2ToSet);
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(c2ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Card2, m_eType);
            break;
    };
    return c2ToSet;
}


tCIDLib::TCard4 TGenProtoExprVal::c4Value() const
{
    tCIDLib::TCard4 c4Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            c4Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            c4Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            c4Ret = m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            c4Ret = m_uValue.c4Value;
            break;

        case tGenProtoS::ETypes::Float4 :
            c4Ret = tCIDLib::TCard4(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            c4Ret = tCIDLib::TCard4(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            c4Ret = tCIDLib::TCard4(m_uValue.i1Value);
            break;

        case tGenProtoS::ETypes::Int2 :
            c4Ret = tCIDLib::TCard4(m_uValue.i2Value);
            break;

        case tGenProtoS::ETypes::Int4 :
            c4Ret = tCIDLib::TCard4(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            c4Ret = m_pstrValue->c4Val();
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Card4);
            break;
    };
    return c4Ret;
}

tCIDLib::TCard4 TGenProtoExprVal::c4Value(const tCIDLib::TCard4 c4ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = c4ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(c4ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(c4ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = c4ToSet;
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = tCIDLib::TFloat4(c4ToSet);
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = c4ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(c4ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(c4ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = tCIDLib::TInt4(c4ToSet);
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(c4ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Card4, m_eType);
            break;
    };
    return c4ToSet;
}


tGenProtoS::ETypes TGenProtoExprVal::eType() const
{
    return m_eType;
}


tCIDLib::TFloat4 TGenProtoExprVal::f4Value() const
{
    tCIDLib::TFloat4 f4Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            f4Ret = m_uValue.bValue ? 1.0F : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            f4Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            f4Ret = m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            f4Ret = tCIDLib::TFloat4(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            f4Ret = m_uValue.f4Value;
            break;

        case tGenProtoS::ETypes::Float8 :
            f4Ret = tCIDLib::TFloat4(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            f4Ret = m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            f4Ret = m_uValue.i2Value;
            break;

        case tGenProtoS::ETypes::Int4 :
            f4Ret = tCIDLib::TFloat4(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            f4Ret = tCIDLib::TFloat4(m_pstrValue->f8Val());
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Float4);
            break;
    };
    return f4Ret;
}

tCIDLib::TFloat4 TGenProtoExprVal::f4Value(const tCIDLib::TFloat4 f4ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = f4ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(f4ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(f4ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = tCIDLib::TCard2(f4ToSet);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = f4ToSet;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = f4ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(f4ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(f4ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = tCIDLib::TInt4(f4ToSet);
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(f4ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Float4, m_eType);
            break;
    };
    return f4ToSet;
}


tCIDLib::TFloat8 TGenProtoExprVal::f8Value() const
{
    tCIDLib::TFloat8 f8Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            f8Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            f8Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            f8Ret = m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            f8Ret = m_uValue.c4Value;
            break;

        case tGenProtoS::ETypes::Float4 :
            f8Ret = m_uValue.f4Value;
            break;

        case tGenProtoS::ETypes::Float8 :
            f8Ret = m_uValue.f8Value;
            break;

        case tGenProtoS::ETypes::Int1 :
            f8Ret = m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            f8Ret = m_uValue.i2Value;
            break;

        case tGenProtoS::ETypes::Int4 :
            f8Ret = m_uValue.i4Value;
            break;

        case tGenProtoS::ETypes::String :
            f8Ret = m_pstrValue->f8Val();
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Float8);
            break;
    };
    return f8Ret;
}

tCIDLib::TFloat8 TGenProtoExprVal::f8Value(const tCIDLib::TFloat8 f8ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = f8ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(f8ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(f8ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = tCIDLib::TCard2(f8ToSet);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = tCIDLib::TFloat4(f8ToSet);
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = f8ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(f8ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(f8ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = tCIDLib::TInt4(f8ToSet);
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(f8ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Float8, m_eType);
            break;
    };
    return f8ToSet;
}


tCIDLib::TVoid
TGenProtoExprVal::FormatToStr(        TString&            strToFill
                            , const tCIDLib::ERadices   eRadix
                            , const tCIDLib::TCard4     c4Width
                            , const tCIDLib::EHJustify  eJustify
                            , const tCIDLib::TCh        chFill)
{
    TString strTmp;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
        {
            strTmp = m_uValue.bValue ? kCQCKit::pszFld_True : kCQCKit::pszFld_False;
            break;
        }

        case tGenProtoS::ETypes::Int1 :
        case tGenProtoS::ETypes::Int2 :
        case tGenProtoS::ETypes::Int4 :
        {
            strTmp.AppendFormatted(i4Value(), eRadix);
            break;
        }

        case tGenProtoS::ETypes::Card1 :
        case tGenProtoS::ETypes::Card2 :
        case tGenProtoS::ETypes::Card4 :
        {
            strTmp.AppendFormatted(c4Value(), eRadix);
            break;
        }

        case tGenProtoS::ETypes::Float4 :
        case tGenProtoS::ETypes::Float8 :
        {
            strTmp.AppendFormatted(f8Value());
            break;
        }

        case tGenProtoS::ETypes::String :
        {
            strTmp = *m_pstrValue;
            break;
        }
    }
    strToFill.FormatToFld(strTmp, c4Width, eJustify, chFill);
}


tCIDLib::TInt1 TGenProtoExprVal::i1Value() const
{
    tCIDLib::TInt1 i1Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            i1Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            i1Ret = tCIDLib::TInt1(m_uValue.c1Value);
            break;

        case tGenProtoS::ETypes::Card2 :
            i1Ret = tCIDLib::TInt1(m_uValue.c2Value);
            break;

        case tGenProtoS::ETypes::Card4 :
            i1Ret = tCIDLib::TInt1(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            i1Ret = tCIDLib::TInt1(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            i1Ret = tCIDLib::TInt1(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            i1Ret = m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            i1Ret = tCIDLib::TInt1(m_uValue.i2Value);
            break;

        case tGenProtoS::ETypes::Int4 :
            i1Ret = tCIDLib::TInt1(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            i1Ret = tCIDLib::TInt1(m_pstrValue->i4Val());
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Int1);
            break;
    };
    return i1Ret;
}

tCIDLib::TInt1 TGenProtoExprVal::i1Value(const tCIDLib::TInt1 i1ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = i1ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(i1ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(i1ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = tCIDLib::TCard4(i1ToSet);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = i1ToSet;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = i1ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = i1ToSet;
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = i1ToSet;
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = i1ToSet;
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(tCIDLib::TInt4(i1ToSet));
            break;

        default :
            BadCast(tGenProtoS::ETypes::Int1, m_eType);
            break;
    };
    return i1ToSet;
}


tCIDLib::TInt2 TGenProtoExprVal::i2Value() const
{
    tCIDLib::TInt2 i2Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            i2Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            i2Ret = tCIDLib::TInt2(m_uValue.c1Value);
            break;

        case tGenProtoS::ETypes::Card2 :
            i2Ret = tCIDLib::TInt2(m_uValue.c2Value);
            break;

        case tGenProtoS::ETypes::Card4 :
            i2Ret = tCIDLib::TInt2(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            i2Ret = tCIDLib::TInt2(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            i2Ret = tCIDLib::TInt2(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            i2Ret = m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            i2Ret = m_uValue.i2Value;
            break;

        case tGenProtoS::ETypes::Int4 :
            i2Ret = tCIDLib::TInt2(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            i2Ret = tCIDLib::TInt2(m_pstrValue->i4Val());
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Int2);
            break;
    };
    return i2Ret;
}

tCIDLib::TInt2 TGenProtoExprVal::i2Value(const tCIDLib::TInt2 i2ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = i2ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(i2ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(i2ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = tCIDLib::TCard4(i2ToSet);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = i2ToSet;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = i2ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(i2ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = i2ToSet;
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = i2ToSet;
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(i2ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Int2, m_eType);
            break;
    };
    return i2ToSet;
}


tCIDLib::TInt4 TGenProtoExprVal::i4Value() const
{
    tCIDLib::TInt4 i4Ret;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            i4Ret = m_uValue.bValue ? 1 : 0;
            break;

        case tGenProtoS::ETypes::Card1 :
            i4Ret = m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            i4Ret = m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            i4Ret = tCIDLib::TInt4(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            i4Ret = tCIDLib::TInt4(m_uValue.f4Value);
            break;

        case tGenProtoS::ETypes::Float8 :
            i4Ret = tCIDLib::TInt4(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            i4Ret = m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            i4Ret = m_uValue.i2Value;
            break;

        case tGenProtoS::ETypes::Int4 :
            i4Ret = m_uValue.i4Value;
            break;

        case tGenProtoS::ETypes::String :
            i4Ret = m_pstrValue->i4Val();
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::Int4);
            break;
    };
    return i4Ret;
}

tCIDLib::TInt4 TGenProtoExprVal::i4Value(const tCIDLib::TInt4 i4ToSet)
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = i4ToSet ? kCIDLib::True : kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = tCIDLib::TCard1(i4ToSet);
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = tCIDLib::TCard2(i4ToSet);
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = tCIDLib::TCard4(i4ToSet);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = tCIDLib::TFloat4(i4ToSet);
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = i4ToSet;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = tCIDLib::TInt1(i4ToSet);
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = tCIDLib::TInt2(i4ToSet);
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = i4ToSet;
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue->SetFormatted(i4ToSet);
            break;

        default :
            BadCast(tGenProtoS::ETypes::Int4, m_eType);
            break;
    };
    return i4ToSet;
}


tCIDLib::TVoid TGenProtoExprVal::Reset(const  tGenProtoS::ETypes    eNewType
                                    , const TString&                strInit)
{
    // Store the new type
    m_eType = eNewType;

    //
    //  If setting to string type, then see if the string is allocated yet.
    //  If so, just set it, else allocate a new one. If another type, then
    //  call the method that sets the value from a string.
    //
    if (eNewType == tGenProtoS::ETypes::String)
    {
        if (!m_pstrValue)
            m_pstrValue = new TString(strInit);
        else
            *m_pstrValue = strInit;
    }
     else
    {
        strValue(strInit);
    }
}

tCIDLib::TVoid TGenProtoExprVal::Reset(const tGenProtoS::ETypes eNewType)
{
    // Store the new type
    m_eType = eNewType;

    // Empty the string value if any
    if (m_pstrValue)
        m_pstrValue->Clear();
}


//
//  If we aren't a string type, we fault in a string pointer and format
//  our actual value into it.
//
TString& TGenProtoExprVal::strValue()
{
    FaultInStrVal();
    return *m_pstrValue;
}

const TString& TGenProtoExprVal::strValue() const
{
    FaultInStrVal();
    return *m_pstrValue;
}

const TString& TGenProtoExprVal::strValue(const TString& strToSet)
{
    tCIDLib::TBoolean bOk = kCIDLib::True;
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
        {
            if (strToSet.bCompareI(L"False")
            ||  (strToSet == kCQCKit::pszFld_False)
            ||  (strToSet == L"0"))
            {
                m_uValue.bValue = kCIDLib::False;

            }
             else if (strToSet.bCompareI(L"True")
                  ||  (strToSet == kCQCKit::pszFld_True)
                  ||  (strToSet == L"1"))
            {
                m_uValue.bValue = kCIDLib::True;
            }
             else
            {
                bOk = kCIDLib::False;
            }
            break;
        }

        case tGenProtoS::ETypes::Card1 :
        case tGenProtoS::ETypes::Card2 :
        case tGenProtoS::ETypes::Card4 :
        {
            const tCIDLib::TCard4 c4Val = TRawStr::c4AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );

            if (bOk)
            {
                if (m_eType == tGenProtoS::ETypes::Card1)
                    m_uValue.c1Value = tCIDLib::TInt1(c4Val);
                else if (m_eType == tGenProtoS::ETypes::Card2)
                    m_uValue.c2Value = tCIDLib::TInt2(c4Val);
                else
                    m_uValue.c4Value = c4Val;
            }
            break;
        }

        case tGenProtoS::ETypes::Float4 :
        case tGenProtoS::ETypes::Float8 :
        {
            const tCIDLib::TFloat8 f8Val = TRawStr::f8AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );

            if (bOk)
            {
                if (m_eType == tGenProtoS::ETypes::Float4)
                    m_uValue.f4Value = tCIDLib::TFloat4(f8Val);
                else
                    m_uValue.f8Value = f8Val;
            }
            break;
        }

        case tGenProtoS::ETypes::Int1 :
        case tGenProtoS::ETypes::Int2 :
        case tGenProtoS::ETypes::Int4 :
        {
            const tCIDLib::TInt4 i4Val = TRawStr::i4AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );

            if (bOk)
            {
                if (m_eType == tGenProtoS::ETypes::Int1)
                    m_uValue.i1Value = tCIDLib::TInt1(i4Val);
                else if (m_eType == tGenProtoS::ETypes::Int2)
                    m_uValue.i2Value = tCIDLib::TInt1(i4Val);
                else
                    m_uValue.i4Value = i4Val;
            }
            break;
        }

        case tGenProtoS::ETypes::String :
            // Just a straight assignment in this case
            *m_pstrValue = strToSet;
            break;

        default :
            BadCast(tGenProtoS::ETypes::String, m_eType);
            break;
    };

    // If we couldn't convert the text, then throw
    if (!bOk)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_CantCvtStr
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strToSet
            , TFacGenProtoS::strXlatType(m_eType)
        );
    }

    //
    //  Call the getter and return its return, which will handle faulting
    //  in our string version of the value if its not there already.
    //
    return strValue();
}


tCIDLib::TVoid TGenProtoExprVal::SetFrom(const TGenProtoExprVal& evalSrc)
{
    //
    //  We have to do a kind of matrix here. Our type specific setters help
    //  by handling one side of the matrix for us, so we just have to get
    //  the source value in its native format, and then call the setter for
    //  the same type on ourself, which will cast it as needed to fit into
    //  our value.
    //
    //  The setter methods will watch for undoable casts and throw a bad
    //  cast for us if it can't be done.
    //
    switch(evalSrc.m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            bValue(evalSrc.bValue());
            break;

        case tGenProtoS::ETypes::Card1 :
            c1Value(evalSrc.c1Value());
            break;

        case tGenProtoS::ETypes::Card2 :
            c2Value(evalSrc.c2Value());
            break;

        case tGenProtoS::ETypes::Card4 :
            c4Value(evalSrc.c4Value());
            break;

        case tGenProtoS::ETypes::Float4 :
            f4Value(evalSrc.f4Value());
            break;

        case tGenProtoS::ETypes::Float8 :
            f8Value(evalSrc.f8Value());
            break;

        case tGenProtoS::ETypes::Int1 :
            i1Value(evalSrc.i1Value());
            break;

        case tGenProtoS::ETypes::Int2 :
            i2Value(evalSrc.i2Value());
            break;

        case tGenProtoS::ETypes::Int4 :
            i4Value(evalSrc.i4Value());
            break;

        case tGenProtoS::ETypes::String :
            strValue(evalSrc.strValue());
            break;

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(evalSrc.m_eType))
                , TString(L"tGenProtoS::EType")
            );
            break;
    }
}


//
//  Does case setting on a string value. If it's not a string type, we can
//  ignore it. If it is a string value, then we know the string member has
//  been allocated.
//
tCIDLib::TVoid TGenProtoExprVal::ToLower()
{
    if (m_eType == tGenProtoS::ETypes::String)
        m_pstrValue->ToLower();
}

tCIDLib::TVoid TGenProtoExprVal::ToUpper()
{
    if (m_eType == tGenProtoS::ETypes::String)
        m_pstrValue->ToUpper();
}


//
//  Puts our current value into a memory buffer. For text this involved
//  transcoding the string using the encoding set in the protocol file.
//
tCIDLib::TVoid
TGenProtoExprVal::ValueToMemBuf(  TGenProtoCtx&     ctxTarget
                                , TMemBuf&          mbufTarget
                                , tCIDLib::TCard4&  c4AtOfs) const
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
        {
            mbufTarget.PutBool(m_uValue.bValue, c4AtOfs);
            c4AtOfs += sizeof(tCIDLib::TBoolean);
            break;
        }

        case tGenProtoS::ETypes::Card1 :
        {
            mbufTarget.PutCard1(m_uValue.c1Value, c4AtOfs);
            c4AtOfs++;
            break;
        }

        case tGenProtoS::ETypes::Card2 :
        {
            mbufTarget.PutCard2(m_uValue.c2Value, c4AtOfs);
            c4AtOfs += 2;
            break;
        }

        case tGenProtoS::ETypes::Card4 :
        {
            mbufTarget.PutCard4(m_uValue.c4Value, c4AtOfs);
            c4AtOfs += 4;
            break;
        }

        case tGenProtoS::ETypes::Float4:
        {
            mbufTarget.PutFloat4(m_uValue.f4Value, c4AtOfs);
            c4AtOfs += 4;
            break;
        }

        case tGenProtoS::ETypes::Float8 :
        {
            mbufTarget.PutFloat8(m_uValue.f8Value, c4AtOfs);
            c4AtOfs += 8;
            break;
        }

        case tGenProtoS::ETypes::Int1 :
        {
            mbufTarget.PutInt1(m_uValue.i1Value, c4AtOfs);
            c4AtOfs++;
            break;
        }

        case tGenProtoS::ETypes::Int2 :
        {
            mbufTarget.PutInt2(m_uValue.i2Value, c4AtOfs);
            c4AtOfs += 2;
            break;
        }

        case tGenProtoS::ETypes::Int4 :
        {
            mbufTarget.PutInt4(m_uValue.i4Value, c4AtOfs);
            c4AtOfs += 4;
            break;
        }

        case tGenProtoS::ETypes::String :
        {
            //
            //  We have to transcode this one according to the transcoder
            //  in the passed context. Since we cannot know ahead of time how
            //  many bytes will result, we'll just use a local buffer. For
            //  our application here, a 2K buffer will be massive overkill.
            //
            const tCIDLib::TCard4 c4BufSz = 2048;
            tCIDLib::TCard1 ac1Buf[c4BufSz];

            tCIDLib::TCard4 c4BytesDone;
            const tCIDLib::TCard4 c4CharsDone = ctxTarget.tcvtDevice().c4ConvertTo
            (
                m_pstrValue->pszBuffer()
                , m_pstrValue->c4Length()
                , ac1Buf
                , c4BufSz
                , c4BytesDone
            );

            // Make sure we did all the source chars
            if (c4CharsDone != m_pstrValue->c4Length())
            {
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcRunT_CantXcodeText
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::NotAllWritten
                    , *m_pstrValue
                );
            }

            // Ok, copy into the target buffer, and update the caller's offset
            mbufTarget.CopyIn(ac1Buf, c4BytesDone, c4AtOfs);

            c4AtOfs += c4BytesDone;
            break;
        }

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(m_eType))
                , TString(L"tGenProtoS::EType")
            );
            break;
    }
}



// ---------------------------------------------------------------------------
//  TGenProtoExprVal: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Format our value out to the passed text stream
tCIDLib::TVoid TGenProtoExprVal::FormatTo(TTextOutStream& strmTarget) const
{
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            strmTarget << m_uValue.bValue;
            break;

        case tGenProtoS::ETypes::Card1 :
            strmTarget << m_uValue.c1Value;
            break;

        case tGenProtoS::ETypes::Card2 :
            strmTarget << m_uValue.c2Value;
            break;

        case tGenProtoS::ETypes::Card4 :
            strmTarget << m_uValue.c4Value;
            break;

        case tGenProtoS::ETypes::Float4:
            strmTarget << m_uValue.f4Value;
            break;

        case tGenProtoS::ETypes::Float8 :
            strmTarget << m_uValue.f8Value;
            break;

        case tGenProtoS::ETypes::Int1 :
            strmTarget << m_uValue.i1Value;
            break;

        case tGenProtoS::ETypes::Int2 :
            strmTarget << m_uValue.i2Value;
            break;

        case tGenProtoS::ETypes::Int4 :
            strmTarget << m_uValue.i4Value;
            break;

        case tGenProtoS::ETypes::String :
            strmTarget << *m_pstrValue;
            break;

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(m_eType))
                , TString(L"tGenProtoS::EType")
            );
            break;
    }
}



// ---------------------------------------------------------------------------
//  TGenProtoExprVal: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoExprVal::BadCast(const   tGenProtoS::ETypes eSrcType
                        , const tGenProtoS::ETypes eDestType) const
{
    facGenProtoS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kGPDErrs::errcRunT_BadCast
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::TypeMatch
        , TFacGenProtoS::strXlatType(eSrcType)
        , TFacGenProtoS::strXlatType(eDestType)
    );
}


//
//  Fault the string field in, which we'll have to do if we aren't a
//  string type naturally. We are const and just affect the string memember
//  which is mutable.
//
tCIDLib::TVoid TGenProtoExprVal::FaultInStrVal() const
{
    if (!m_pstrValue)
        m_pstrValue = new TString;

    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            *m_pstrValue = m_uValue.bValue ? L"True" : L"False";
            break;

        case tGenProtoS::ETypes::Card1 :
            m_pstrValue->SetFormatted(tCIDLib::TCard4(m_uValue.c1Value));
            break;

        case tGenProtoS::ETypes::Card2 :
            m_pstrValue->SetFormatted(tCIDLib::TCard4(m_uValue.c2Value));
            break;

        case tGenProtoS::ETypes::Card4 :
            m_pstrValue->SetFormatted(m_uValue.c4Value);
            break;

        case tGenProtoS::ETypes::Float4 :
            m_pstrValue->SetFormatted(tCIDLib::TFloat8(m_uValue.f4Value));
            break;

        case tGenProtoS::ETypes::Float8 :
            m_pstrValue->SetFormatted(m_uValue.f8Value);
            break;

        case tGenProtoS::ETypes::Int1 :
            m_pstrValue->SetFormatted(tCIDLib::TInt4(m_uValue.i1Value));
            break;

        case tGenProtoS::ETypes::Int2 :
            m_pstrValue->SetFormatted(tCIDLib::TInt4(m_uValue.i2Value));
            break;

        case tGenProtoS::ETypes::Int4 :
            m_pstrValue->SetFormatted(m_uValue.i4Value);
            break;

        case tGenProtoS::ETypes::String :
            // Do nothing for this one
            break;

        default :
            BadCast(m_eType, tGenProtoS::ETypes::String);
            break;
    };
}


tCIDLib::TVoid TGenProtoExprVal::InitVal()
{
    // Initialize our union member according to our given type
    switch(m_eType)
    {
        case tGenProtoS::ETypes::Boolean :
            m_uValue.bValue = kCIDLib::False;
            break;

        case tGenProtoS::ETypes::Card1 :
            m_uValue.c1Value = 0;
            break;

        case tGenProtoS::ETypes::Card2 :
            m_uValue.c2Value = 0;
            break;

        case tGenProtoS::ETypes::Card4 :
            m_uValue.c4Value = 0;
            break;

        case tGenProtoS::ETypes::Float4 :
            m_uValue.f4Value = 0;
            break;

        case tGenProtoS::ETypes::Float8 :
            m_uValue.f8Value = 0;
            break;

        case tGenProtoS::ETypes::Int1 :
            m_uValue.i1Value = 0;
            break;

        case tGenProtoS::ETypes::Int2 :
            m_uValue.i2Value = 0;
            break;

        case tGenProtoS::ETypes::Int4 :
            m_uValue.i4Value = 0;
            break;

        case tGenProtoS::ETypes::String :
            m_pstrValue = new TString;
            break;

        default :
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(m_eType))
                , TString(L"tGenProtoS::EType")
            );
            break;
    }
}


tCIDLib::TVoid
TGenProtoExprVal::ValidateType(const tGenProtoS::ETypes eToCheck) const
{
    if (m_eType != eToCheck)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_WrongExprType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TFacGenProtoS::strXlatType(m_eType)
            , TFacGenProtoS::strXlatType(eToCheck)
        );
    }
}



