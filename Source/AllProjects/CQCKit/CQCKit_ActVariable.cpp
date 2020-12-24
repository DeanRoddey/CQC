//
// FILE NAME: CQCKit_ActVariable.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/10/2008
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
//  This is the implementation for the action variable class
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCActVar,TObject)



// ---------------------------------------------------------------------------
//  Due to the nature of this class, there's a lot of repetitive code that
//  only changes by type, so we use some macros to vastly reduce the amount
//  of typingwhen testing new values to store, storing them, and handling
//  the success or failure of that.
// ---------------------------------------------------------------------------
#define TestAndStore(t,n) \
if (bTestLimits(static_cast<TCQCFld##t##Limit*>(m_pfldlTest)->eValidate(n), pszOp)) \
{ \
    if (m_bReadOnly) \
        ThrowReadOnly(pszOp, CID_LINE); \
    bRet = pfv##t->bSetValue(n); \
    if (bRet) \
        m_c4SerialNum++; \
}

#define TestAndStore2(v) \
{ \
    if (m_bReadOnly) \
        ThrowReadOnly(pszOp, CID_LINE); \
    const tCQCKit::EValResults eRes = m_pfvCurrent->eStoreValue(v, *m_pfldlTest); \
    tCIDLib::TBoolean bRet = kCIDLib::False; \
    if (eRes == tCQCKit::EValResults::OK) \
    { \
        bRet = kCIDLib::True; \
        m_c4SerialNum++; \
    } \
     else if (eRes == tCQCKit::EValResults::Unchanged) \
    { \
        bRet = kCIDLib::False; \
    } \
     else if (eRes == tCQCKit::EValResults::Unconvertable) \
    { \
        ThrowBadParm(pszOp, CID_LINE); \
    } \
     else \
    { \
        ThrowLimits(pszOp, CID_LINE); \
    } \
    return bRet; \
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCActVar
//  PREFIX: var
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCActVar: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::ESortComps TCQCActVar::eComp(const TCQCActVar& var1, const TCQCActVar& var2)
{
    return var1.m_strName.eCompare(var2.m_strName);
}

tCIDLib::ESortComps TCQCActVar::eIComp(const TCQCActVar& var1, const TCQCActVar& var2)
{
    return var1.m_strName.eCompareI(var2.m_strName);
}


const TString& TCQCActVar::strKey(const TCQCActVar& varSrc)
{
    return varSrc.strName();
}


// ---------------------------------------------------------------------------
//  TCQCActVar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCActVar::TCQCActVar( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const TString&            strInitValue
                        , const tCIDLib::TCard4     c4VarId) :

    m_bPermanent(kCIDLib::False)
    , m_bReadOnly(kCIDLib::False)
    , m_c4Digits(0)
    , m_c4FmtSerialNum(0)
    , m_c4SerialNum(1)
    , m_c4SrcVal(0)
    , m_c4VarId(c4VarId)
    , m_c8SrcVal(0)
    , m_eRadix(tCIDLib::ERadices::Dec)
    , m_f8SrcVal(0)
    , m_eType(eType)
    , m_i4SrcVal(0)
    , m_i8SrcVal(0)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();

    // If floating point, initially set 2 digits of precision
    if (eType == tCQCKit::EFldTypes::Float)
        m_c4Digits = 2;

    // Allocate the value storage and set to error state until proven otherwise
    m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_eType);
    m_pfvCurrent->bInError(kCIDLib::True);

    // Allocate a default limit for the type
    m_pfldlTest = TCQCFldLimit::pfldlMakeNew(m_strName, m_strLimits, m_eType);

    // Store an initial value
    if (strInitValue.bIsEmpty())
    {
        TString strVal;
        m_pfldlTest->QueryDefVal(strVal);
        m_pfvCurrent->eStoreValue(strVal, *m_pfldlTest);
    }
     else
    {
        // This could fail, in which case we need to release stuff and throw
        tCQCKit::EValResults eRes = m_pfvCurrent->eStoreValue(strInitValue, *m_pfldlTest);

        if ((eRes != tCQCKit::EValResults::OK)
        &&  (eRes != tCQCKit::EValResults::Unchanged))
        {
            delete m_pfldlTest;
            delete m_pfvCurrent;

            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_VarLimitErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strName
            );
        }
    }
}

TCQCActVar::TCQCActVar( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const TString&            strLimits
                        , const TString&            strInitValue
                        , const tCIDLib::TCard4     c4VarId) :

    m_bPermanent(kCIDLib::False)
    , m_bReadOnly(kCIDLib::False)
    , m_c4Digits(0)
    , m_c4FmtSerialNum(0)
    , m_c4SerialNum(1)
    , m_c4SrcVal(0)
    , m_c4VarId(c4VarId)
    , m_eRadix(tCIDLib::ERadices::Dec)
    , m_f8SrcVal(0)
    , m_eType(eType)
    , m_i4SrcVal(0)
    , m_i8SrcVal(0)
    , m_strLimits(strLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();

    // If floating point, initially set 2 digits of precision
    if (eType == tCQCKit::EFldTypes::Float)
        m_c4Digits = 2;

    // Allocate the value storage
    m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_eType);

    //
    //  We will take the default value so clear the initially set error state.
    //  It may get overridden below by an initial value requested.
    //
    m_pfvCurrent->bInError(kCIDLib::False);

    // Allocate a limit object for the type
    m_pfldlTest = TCQCFldLimit::pfldlMakeNew(m_strName, m_strLimits, m_eType);

    // Store an initial value
    if (strInitValue.bIsEmpty())
    {
        TString strVal;
        m_pfldlTest->QueryDefVal(strVal);
        m_pfvCurrent->eStoreValue(strVal, *m_pfldlTest);
    }
     else
    {
        // This could fail, in which case we need to release stuff and throw
        tCQCKit::EValResults eRes = m_pfvCurrent->eStoreValue(strInitValue, *m_pfldlTest);

        if ((eRes != tCQCKit::EValResults::OK)
        &&  (eRes != tCQCKit::EValResults::Unchanged))
        {
            delete m_pfldlTest;
            delete m_pfvCurrent;

            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_VarLimitErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strName
            );
        }
    }
}

TCQCActVar::TCQCActVar( const   TString&            strName
                        , const tCQCKit::EFldTypes  eType
                        , const tCIDLib::TCh* const pszLimits
                        , const TString&            strInitValue
                        , const tCIDLib::TCard4     c4VarId) :

    m_bPermanent(kCIDLib::False)
    , m_bReadOnly(kCIDLib::False)
    , m_c4Digits(0)
    , m_c4FmtSerialNum(0)
    , m_c4SerialNum(1)
    , m_c4SrcVal(0)
    , m_c4VarId(c4VarId)
    , m_eRadix(tCIDLib::ERadices::Dec)
    , m_f8SrcVal(0)
    , m_eType(eType)
    , m_i4SrcVal(0)
    , m_i8SrcVal(0)
    , m_strLimits(pszLimits)
    , m_strName(strName)
{
    // Can't have any leading/trailing whitespace
    m_strName.StripWhitespace();

    // If floating point, initially set 2 digits of precision
    if (eType == tCQCKit::EFldTypes::Float)
        m_c4Digits = 2;

    // Allocate the value storage
    m_pfvCurrent = facCQCKit().pfvMakeNewFor(0, 0, m_eType);

    //
    //  We will take the default value so clear the initially set error state.
    //  It may get overridden below by an initial value requested.
    //
    m_pfvCurrent->bInError(kCIDLib::False);

    // Allocate a limit for the type
    m_pfldlTest = TCQCFldLimit::pfldlMakeNew(m_strName, m_strLimits, m_eType);

    // Store an initial value
    if (strInitValue.bIsEmpty())
    {
        TString strVal;
        m_pfldlTest->QueryDefVal(strVal);
        m_pfvCurrent->eStoreValue(strVal, *m_pfldlTest);
    }
     else
    {
        // This could fail, in which case we need to release stuff and throw
        tCQCKit::EValResults eRes = m_pfvCurrent->eStoreValue(strInitValue, *m_pfldlTest);

        if ((eRes != tCQCKit::EValResults::OK)
        &&  (eRes != tCQCKit::EValResults::Unchanged))
        {
            delete m_pfldlTest;
            delete m_pfvCurrent;

            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_VarLimitErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strName
            );
        }
    }
}

TCQCActVar::~TCQCActVar()
{
    // Clean up the value and limits objects if created
    delete m_pfvCurrent;
    delete m_pfldlTest;
}


// ---------------------------------------------------------------------------
//  TCQCActVar: Public operators
// ---------------------------------------------------------------------------

//
//  Note that the value isn't counted. It's a transient charactersitics of the
//  variable.
//
tCIDLib::TBoolean TCQCActVar::operator==(const TCQCActVar& varToComp) const
{
    // Do them in the order most likely to differ if not equal
    if ((m_strName      != varToComp.m_strName)
    ||  (m_eType        != varToComp.m_eType)
    ||  (m_strLimits    != varToComp.m_strLimits))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCActVar::operator!=(const TCQCActVar& varToComp) const
{
    return !operator==(varToComp);
}


// ---------------------------------------------------------------------------
//  TCQCActVar: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  These all do some operation on our value. They all convert the passed
//  value to an appropriate format to match our type, then do some operation
//  on the value. We return whether the value changed or not.
//
tCIDLib::TBoolean TCQCActVar::bAdd(const TString& strToAdd)
{
    static const tCIDLib::TCh* const pszOp = L"add";

    //
    //  Evalute the value and convert to our type. If it is zero, then we
    //  don't need to do anything.
    //
    if (bCvtNumType(pszOp, CID_LINE, strToAdd))
        return kCIDLib::False;

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            const tCIDLib::TCard4 c4New = pfvCard->c4Value() + m_c4SrcVal;
            TestAndStore(Card, c4New);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            const tCIDLib::TFloat8 f8New = pfvFloat->f8Value() + m_f8SrcVal;
            TestAndStore(Float, f8New);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            const tCIDLib::TInt4 i4New = pfvInt->i4Value() + m_i4SrcVal;
            TestAndStore(Int, i4New);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // For this one, we have to try to convert our value first
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            tCIDLib::TInt8 i8Tmp;
            if (!pfvStr->strValue().bToInt8(i8Tmp))
                ThrowBadTarVal(pszOp, CID_LINE);
            i8Tmp += m_i8SrcVal;
            m_strFmt.SetFormatted(i8Tmp);
            m_c4FmtSerialNum = 0;

            TestAndStore(Str, m_strFmt)
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            const tCIDLib::TCard8 c8New = pfvTime->c8Value() + m_c8SrcVal;
            TestAndStore(Time, c8New);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


tCIDLib::TBoolean TCQCActVar::bAppendValue(const TString& strToAppend)
{
    static const tCIDLib::TCh* const pszOp = L"append";

    // If the value to append is empty, it can't change us
    if (strToAppend.bIsEmpty())
        return kCIDLib::False;

    // Use the format string as a temp and build up the new value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);
    m_strFmt.Append(strToAppend);

    // See if this is accepted by the limit object
    TestAndStore2(m_strFmt);
}


tCIDLib::TBoolean
TCQCActVar::bBooleanOp( const   TString&            strValue
                        , const tCQCKit::ELogOps    eLogOp)
{
    const tCIDLib::TCh* pszOp = L"XOR";
    if (eLogOp == tCQCKit::ELogOps::AND)
        pszOp = L"AND";
    else if (eLogOp == tCQCKit::ELogOps::OR)
        pszOp = L"OR";

    // Evalute the value and convert to our type
    bCvtNumType(pszOp, CID_LINE, strValue, kCIDLib::True);

    // Only the card and string types are valid for this
    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            tCIDLib::TCard4 c4New = 0;
            if (eLogOp == tCQCKit::ELogOps::AND)
                c4New = pfvCard->c4Value() & m_c4SrcVal;
            else if (eLogOp == tCQCKit::ELogOps::OR)
                c4New = pfvCard->c4Value() | m_c4SrcVal;
            else
                c4New = pfvCard->c4Value() ^ m_c4SrcVal;
            TestAndStore(Card, c4New);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // For this one, we have to try to convert our value first
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            tCIDLib::TCard8 c8Tmp = 0;
            if (!pfvStr->strValue().bToCard8(c8Tmp))
                ThrowBadTarVal(pszOp, CID_LINE);

            if (eLogOp == tCQCKit::ELogOps::AND)
                c8Tmp &= m_c8SrcVal;
            else if (eLogOp == tCQCKit::ELogOps::OR)
                c8Tmp |= m_c8SrcVal;
            else
                c8Tmp ^= m_c8SrcVal;

            m_strFmt.SetFormatted(c8Tmp);
            m_c4FmtSerialNum = 0;

            TestAndStore(Str, m_strFmt);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


tCIDLib::TBoolean TCQCActVar::bCapAt(const tCIDLib::TCard4 c4Index)
{
    static const tCIDLib::TCh* const pszOp = L"cap at";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // If the index is at or beyond the end, then we do nothing
    if (c4Index >= m_strFmt.c4Length())
        return kCIDLib::False;

    // Looks ok so cap it
    m_strFmt.CapAt(c4Index);

    TestAndStore2(m_strFmt);
}


tCIDLib::TBoolean
TCQCActVar::bDelSubStr( const   tCIDLib::TCard4 c4Start
                        , const tCIDLib::TCard4 c4Count)
{
    static const tCIDLib::TCh* const pszOp = L"delete sub-string";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // If the start index is at or beyond the end, then we do nothing
    if (c4Start >= m_strFmt.c4Length())
        return kCIDLib::False;

    // Looks ok so do it
    m_strFmt.Cut(c4Start, c4Count);

    TestAndStore2(m_strFmt);
}


tCIDLib::TBoolean TCQCActVar::bDivide(const TString& strDivisor)
{
    static const tCIDLib::TCh* const pszOp = L"divide";

    //
    //  Evalute the value and convert to our type. If it's zero, that's
    //  an error.
    //
    if (bCvtNumType(pszOp, CID_LINE, strDivisor))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_DivByZero
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            const tCIDLib::TCard4 c4New = pfvCard->c4Value() / m_c4SrcVal;
            TestAndStore(Card, c4New);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            const tCIDLib::TFloat8 f8New = pfvFloat->f8Value() / m_f8SrcVal;
            TestAndStore(Float, f8New);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            const tCIDLib::TInt4 i4New = pfvInt->i4Value() / m_i4SrcVal;
            TestAndStore(Int, i4New);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // For this one, we have to try to convert our value first
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            tCIDLib::TInt8 i8Tmp;
            if (!pfvStr->strValue().bToInt8(i8Tmp))
                ThrowBadTarVal(pszOp, CID_LINE);
            i8Tmp /= m_i8SrcVal;
            m_strFmt.SetFormatted(i8Tmp);
            m_c4FmtSerialNum = 0;

            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            const tCIDLib::TCard8 c8New = pfvTime->c8Value() / m_c8SrcVal;
            TestAndStore(Time, c8New);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


// Insert a new value into this variable
tCIDLib::TBoolean
TCQCActVar::bInsertValue(const TString& strValue, const tCIDLib::TCard4 c4At)
{
    static const tCIDLib::TCh* const pszOp = L"insert";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // If the index is at or beyond the end, then we do nothing
    if (c4At >= m_strFmt.c4Length())
        return kCIDLib::False;

    // Looks ok so do it
    m_strFmt.Insert(strValue, c4At);

    TestAndStore2(m_strFmt);
}


// Indicates whether this variable is of a numeric type or not
tCIDLib::TBoolean TCQCActVar::bIsNumericType() const
{
    return (m_eType == tCQCKit::EFldTypes::Card)
           || (m_eType == tCQCKit::EFldTypes::Float)
           || (m_eType == tCQCKit::EFldTypes::Int);
}


// Multiply this variable by the passed value
tCIDLib::TBoolean TCQCActVar::bMultiply(const TString& strMultiplier)
{
    static const tCIDLib::TCh* const pszOp = L"multiply";

    //
    //  Evalute the value and convert to our type. The return indicates if it is zero
    //  or not, but in our case we have to process zero values.
    //
    bCvtNumType(pszOp, CID_LINE, strMultiplier);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            const tCIDLib::TCard4 c4New = pfvCard->c4Value() * m_c4SrcVal;
            TestAndStore(Card, c4New);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            const tCIDLib::TFloat8 f8New = pfvFloat->f8Value() * m_f8SrcVal;
            TestAndStore(Float, f8New);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            const tCIDLib::TInt4 i4New = pfvInt->i4Value() * m_i4SrcVal;
            TestAndStore(Int, i4New);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // For this one, we have to try to convert our value first
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            tCIDLib::TInt8 i8Tmp;
            if (!pfvStr->strValue().bToInt8(i8Tmp))
                ThrowBadTarVal(pszOp, CID_LINE);
            i8Tmp *= m_i8SrcVal;
            m_strFmt.SetFormatted(i8Tmp);
            m_c4FmtSerialNum = 0;

            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            const tCIDLib::TCard8 c8New = pfvTime->c8Value() * m_c8SrcVal;
            TestAndStore(Time, c8New);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


// Negate the value of this variable if it's relevant to the type
tCIDLib::TBoolean TCQCActVar::bNegate()
{
    static const tCIDLib::TCh* const pszOp = L"negate";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            // An easy one, just negate the current boolean value
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            bRet = pfvBool->bSetValue(!pfvBool->bValue());

            // If it changed, bump the serial number
            if (bRet)
                m_c4SerialNum++;
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            //
            //  If the string holds a 1 or 0 or True or False, set it to the
            //  opposite.
            //
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt = pfvStr->strValue();
            if (m_strFmt == L"1")
                m_strFmt = L"0";
            else if (m_strFmt.bCompareI(L"True"))
                m_strFmt = L"False";
            else if (m_strFmt == L"0")
                m_strFmt = L"1";
            else if (m_strFmt.bCompareI(L"False"))
                m_strFmt = L"True";

            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            tCIDLib::TCard4 c4New;
            if (pfvCard->c4Value())
                c4New = 0;
            else
                c4New = 1;
            TestAndStore(Card, c4New);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return kCIDLib::True;
}


// Get/set the permanent flag
tCIDLib::TBoolean TCQCActVar::bPermanent() const
{
    return m_bPermanent;
}

tCIDLib::TBoolean TCQCActVar::bPermanent(const tCIDLib::TBoolean bToSet)
{
    m_bPermanent = bToSet;
    return m_bPermanent;
}


//
//  If the variable has an enumerated limit type, we return true and fill
//  in the passed list.
//
tCIDLib::TBoolean
TCQCActVar::bQueryEnumVals(TVector<TString>& colToFill) const
{
    if (m_pfldlTest && m_pfldlTest->bIsA(TCQCFldEnumLimit::clsThis()))
    {
        m_pfldlTest->c4QueryValues(colToFill);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// Get/set the read only flag
tCIDLib::TBoolean TCQCActVar::bReadOnly() const
{
    return m_bReadOnly;
}

tCIDLib::TBoolean TCQCActVar::bReadOnly(const tCIDLib::TBoolean bToSet)
{
    m_bReadOnly = bToSet;
    return m_bReadOnly;
}


// Do a character replacement operation on the value
tCIDLib::TBoolean
TCQCActVar::bReplaceChars(  const   tCIDLib::TCh    chReplace
                            , const tCIDLib::TCh    chReplaceWith)
{
    static const tCIDLib::TCh* const pszOp = L"replace";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // Do the replacing in the temp. If anything was replaced then update
    if (m_strFmt.bReplaceChar(chReplace, chReplaceWith))
    {
        TestAndStore2(m_strFmt);
    }
    return kCIDLib::False;
}


// Replace all instances of a substring with another
tCIDLib::TBoolean
TCQCActVar::bReplaceSubStr( const   TString&            strRep
                            , const TString&            strRepWith
                            , const tCIDLib::TBoolean   bCase)
{
    static const tCIDLib::TCh* const pszOp = L"replace substr";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // Do the replacing in the temp. If anything was replaced then update
    tCIDLib::TCard4 c4Index = 0;
    if (m_strFmt.bReplaceSubStr(strRep, strRepWith, c4Index, kCIDLib::True, bCase))
    {
        TestAndStore2(m_strFmt);
    }
    return kCIDLib::False;
}


// Do a token replacement operation on the value
tCIDLib::TBoolean
TCQCActVar::bReplaceToken(  const   tCIDLib::TCh    chToken
                            , const TString&        strReplaceWith)
{
    static const tCIDLib::TCh* const pszOp = L"replace token";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // Do the replacing in the temp. If anything was replaced then update
    if (m_strFmt.eReplaceToken(strReplaceWith, chToken) == tCIDLib::EFindRes::Found)
    {
        TestAndStore2(m_strFmt);
    }
    return kCIDLib::False;
}


//
//  Set a character in the value at a particular index. We'll only allow this
//  if it's a text variable.
//
tCIDLib::TBoolean
TCQCActVar::bSetCharAt( const   tCIDLib::TCh    chToSet
                        , const tCIDLib::TCard4 c4At)
{
    static const tCIDLib::TCh* const pszOp = L"set char at";

    if (m_eType != tCQCKit::EFldTypes::String)
    {
        ThrowBadOp(L"set char at", CID_LINE);
        return kCIDLib::False;
    }

    // Get it as its actual type so we can test the index
    TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
    if (c4At >= pfvStr->strValue().c4Length())
        return kCIDLib::False;

    // Looks ok so get the new value created and test and store
    m_strFmt = pfvStr->strValue();
    m_strFmt.PutAt(c4At, chToSet);
    TestAndStore2(m_strFmt);

    return kCIDLib::True;
}


// A special case for setting a time stamp
tCIDLib::TBoolean TCQCActVar::bSetTime(const tCIDLib::TCard8 c8Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";
    tCIDLib::TBoolean bRet = kCIDLib::False;

    switch(m_eType)
    {
        case tCQCKit::EFldTypes::String :
        {
            // Format it to hex
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt = L"0x";
            m_strFmt.AppendFormatted(c8Value, tCIDLib::ERadices::Hex);
            TestAndStore(Str, m_strFmt);
            m_c4FmtSerialNum = 0;
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, c8Value);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };

    return bRet;
}


//
//  These all set the value of this variable from various types. This is a
//  pain for us, since passing in direct types requires a switch for every
//  variation, to provide the correct casting of values. But it makes the
//  outside world's job far easier.
//
tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TBoolean bValue)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    // Mostly belowe we just store 1 or zero
    const tCIDLib::TCard1 c1Num = bValue ? 1 : 0;

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            TestAndStore(Bool, bValue);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, c1Num);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, c1Num);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, c1Num);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            const tCIDLib::TCh* pszNew = bValue ? kCQCKit::pszFld_True : kCQCKit::pszFld_False;
            TestAndStore(Str, pszNew);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, c1Num);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}

tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TCard4 c4Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            const tCIDLib::TBoolean bNew = c4Value ? kCIDLib::True : kCIDLib::False;
            TestAndStore(Bool, bNew);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, c4Value);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, tCIDLib::TFloat8(c4Value));
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, tCIDLib::TInt4(c4Value));
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt.SetFormatted(c4Value);
            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, c4Value);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}

tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TCard8 c8Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            const tCIDLib::TBoolean bNew = c8Value ? kCIDLib::True : kCIDLib::False;
            TestAndStore(Bool, bNew);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, tCIDLib::TCard4(c8Value));
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, tCIDLib::TFloat8(c8Value));
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, tCIDLib::TInt4(c8Value));
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt.SetFormatted(c8Value);
            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, c8Value);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}

tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TFloat8 f8Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            const tCIDLib::TBoolean bNew = f8Value ? kCIDLib::True : kCIDLib::False;
            TestAndStore(Bool, bNew);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, tCIDLib::TCard4(f8Value));
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, f8Value);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, tCIDLib::TInt4(f8Value));
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt.SetFormatted(f8Value);
            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, tCIDLib::TCard8(f8Value));
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}

tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TInt4 i4Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            const tCIDLib::TBoolean bNew = i4Value ? kCIDLib::True : kCIDLib::False;
            TestAndStore(Bool, bNew);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, tCIDLib::TCard4(i4Value));
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, tCIDLib::TFloat8(i4Value));
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, i4Value);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt.SetFormatted(i4Value);
            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, tCIDLib::TCard8(i4Value));
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}

tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TInt8 i8Value)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool(static_cast<TCQCBoolFldValue*>(m_pfvCurrent));
            const tCIDLib::TBoolean bNew = i8Value ? kCIDLib::True : kCIDLib::False;
            TestAndStore(Bool, bNew);
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            TestAndStore(Card, tCIDLib::TCard4(i8Value));
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            TestAndStore(Float, tCIDLib::TFloat8(i8Value));
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            TestAndStore(Int, tCIDLib::TInt4(i8Value));
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            m_strFmt.SetFormatted(i8Value);
            m_c4FmtSerialNum = 0;
            TestAndStore(Str, m_strFmt);
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            TestAndStore(Time, tCIDLib::TCard8(i8Value));
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


tCIDLib::TBoolean TCQCActVar::bSetValue(const tCIDLib::TCh* const pszToSet)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";
    m_strFmt = pszToSet;
    m_c4FmtSerialNum = 0;
    TestAndStore2(m_strFmt);
}


tCIDLib::TBoolean TCQCActVar::bSetValue(const TString& strValue)
{
    static const tCIDLib::TCh* const pszOp = L"assignment";
    TestAndStore2(strValue);
}


//
//  Different from above in that it takes another variable. So we don't
//  know at compile time what the type of the source variable is.
//
tCIDLib::TBoolean TCQCActVar::bSetValueFrom(const TCQCActVar& varSrc)
{
    static const tCIDLib::TCh* const pszOp = L"set value from";

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(varSrc.m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            TCQCBoolFldValue* pfvBool = static_cast<TCQCBoolFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvBool->bValue());
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard = static_cast<TCQCCardFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvCard->c4Value());
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat = static_cast<TCQCFloatFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvFloat->f8Value());
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt = static_cast<TCQCIntFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvInt->i4Value());
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            TCQCStringFldValue* pfvStr = static_cast<TCQCStringFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvStr->strValue());
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>
            (
                varSrc.m_pfvCurrent
            );
            bRet = bSetValue(pfvTime->c8Value());
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}



// Strip the passed characters from the value of this variable
tCIDLib::TBoolean
TCQCActVar::bStrip( const   TString&                strStripChars
                    , const tCIDLib::EStripModes    eMode)
{
    static const tCIDLib::TCh* const pszOp = L"Strip";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // And do the strip operation, saving the current length first
    const tCIDLib::TCard4 c4OrgLen = m_strFmt.c4Length();
    m_strFmt.Strip(strStripChars, eMode);

    // If the string is the same length, we couldn't have done anything
    if (m_strFmt.c4Length() == c4OrgLen)
        return kCIDLib::False;

    TestAndStore2(m_strFmt);
}


// Subtract the passed value from this value
tCIDLib::TBoolean TCQCActVar::bSubtract(const TString& strToSub)
{
    static const tCIDLib::TCh* const pszOp = L"subtract";

    //
    //  Evalute the value and convert to our type. If it is zero, then we
    //  don't need to do anything.
    //
    if (bCvtNumType(pszOp, CID_LINE, strToSub))
        return kCIDLib::False;

    tCIDLib::TBoolean bRet = kCIDLib::False;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
            const tCIDLib::TCard4 c4New = pfvCard->c4Value() - m_c4SrcVal;
            TestAndStore(Card, c4New);
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
            const tCIDLib::TFloat8 f8New = pfvFloat->f8Value() - m_f8SrcVal;
            TestAndStore(Float, f8New);
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
            const tCIDLib::TInt4 i4New = pfvInt->i4Value() - m_i4SrcVal;
            TestAndStore(Int, i4New);
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            // For this one, we have to try to convert our value first
            TCQCStringFldValue* pfvStr(static_cast<TCQCStringFldValue*>(m_pfvCurrent));
            tCIDLib::TInt8 i8Tmp;
            if (!pfvStr->strValue().bToInt8(i8Tmp))
                ThrowBadTarVal(pszOp, CID_LINE);
            i8Tmp -= m_i8SrcVal;
            m_strFmt.SetFormatted(i8Tmp);
            m_c4FmtSerialNum = 0;

            TestAndStore(Str, m_strFmt)
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
            const tCIDLib::TCard8 c8New = pfvTime->c8Value() - m_c8SrcVal;
            TestAndStore(Time, c8New);
            break;
        }

        default :
            ThrowBadOp(pszOp, CID_LINE);
            break;
    };
    return bRet;
}


tCIDLib::TBoolean TCQCActVar::bUpLow(const tCIDLib::TBoolean bUpper)
{
    const tCIDLib::TCh* pszOp = L"lower case";
    if (bUpper)
        pszOp = L"upper case";

    // Use the format string as a temp to format out the value
    m_c4FmtSerialNum = 0;
    m_pfvCurrent->Format(m_strFmt);

    // And do the operation and store it
    if (bUpper)
        m_strFmt.ToUpper();
    else
        m_strFmt.ToLower();
    TestAndStore2(m_strFmt);
}


// Return the current serial number
tCIDLib::TCard4 TCQCActVar::c4SerialNum() const
{
    return m_c4SerialNum;
}


// Return our variable id
tCIDLib::TCard4 TCQCActVar::c4VarId() const
{
    return m_c4VarId;
}


// Return the type of this variable
tCQCKit::EFldTypes TCQCActVar::eType() const
{
    return m_eType;
}


// Provide read only access to our field limits
const TCQCFldLimit& TCQCActVar::fldlVar() const
{
    CIDAssert(m_pfldlTest != 0, L"The action variable limit object is not set");
    return *m_pfldlTest;
}


// Provide read only access to our raw value object
const TCQCFldValue& TCQCActVar::fvCurrent() const
{
    return *m_pfvCurrent;
}


// Make sure this guy has enumerated limits. If so, get the nth value
tCIDLib::TVoid
TCQCActVar::QueryNthEnumVal(const   tCIDLib::TCard4 c4Index
                            ,       TString&        strToFill) const
{
    if (!m_pfldlTest || !m_pfldlTest->bIsA(TCQCFldEnumLimit::clsThis()))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcCmd_MustBeEnum
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , m_strName
        );
    }
    else
    {
        // Cast the limits to its actual type and get out the value
        const TCQCFldEnumLimit* pfldlEnum = static_cast<const TCQCFldEnumLimit*>(m_pfldlTest);
        strToFill = pfldlEnum->strValueAt(c4Index);
    }
}


//
//  Query the min/max legal values (as set per any limits) on a numeric
//  typed variable. If not numeric type, we throw.
//
tCIDLib::TVoid
TCQCActVar::QueryNumericLimits(TString& strMinVal, TString& strMaxVal) const
{
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Card :
        {
            TCQCFldCardLimit* pfldlCard(static_cast<TCQCFldCardLimit*>(m_pfldlTest));
            strMinVal.SetFormatted(pfldlCard->c4Min());
            strMaxVal.SetFormatted(pfldlCard->c4Max());
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            TCQCFldFloatLimit* pfldlCard(static_cast<TCQCFldFloatLimit*>(m_pfldlTest));
            strMinVal.SetFormatted(pfldlCard->f8Min());
            strMaxVal.SetFormatted(pfldlCard->f8Max());
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            TCQCFldIntLimit* pfldlCard(static_cast<TCQCFldIntLimit*>(m_pfldlTest));
            strMinVal.SetFormatted(pfldlCard->i4Min());
            strMaxVal.SetFormatted(pfldlCard->i4Max());
            break;
        }

        default :
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_NotNumeric
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
                , m_strName
            );
            break;
    };
}



// Provide access to our limits string for human consumption purposes
const TString& TCQCActVar::strLimits() const
{
    return m_strLimits;
}


// Proivde access to our variable name
const TString& TCQCActVar::strName() const
{
    return m_strName;
}


//
//  If the value has changed since we last formatted out the value, then
//  re-format it. Either way, return the formatted value.
//
const TString& TCQCActVar::strValue() const
{
    if (m_c4FmtSerialNum != m_c4SerialNum)
    {
        switch(m_eType)
        {
            case tCQCKit::EFldTypes::Boolean :
            case tCQCKit::EFldTypes::String :
            {
                // These we just let the value object doing it
                m_pfvCurrent->Format(m_strFmt);
                break;
            }

            case tCQCKit::EFldTypes::Card :
            {
                TCQCCardFldValue* pfvCard(static_cast<TCQCCardFldValue*>(m_pfvCurrent));
                m_strFmt.SetFormatted(pfvCard->c4Value(), m_eRadix);
                break;
            }

            case tCQCKit::EFldTypes::Float :
            {
                TCQCFloatFldValue* pfvFloat(static_cast<TCQCFloatFldValue*>(m_pfvCurrent));
                m_strFmt.SetFormatted(pfvFloat->f8Value(), m_c4Digits);
                break;
            }

            case tCQCKit::EFldTypes::Int :
            {
                TCQCIntFldValue* pfvInt(static_cast<TCQCIntFldValue*>(m_pfvCurrent));
                m_strFmt.SetFormatted(pfvInt->i4Value(), m_eRadix);
                break;
            }

            case tCQCKit::EFldTypes::Time :
            {
                TCQCTimeFldValue* pfvTime = static_cast<TCQCTimeFldValue*>(m_pfvCurrent);
                pfvTime->Format(m_strFmt);
                break;
            }

            default :
                m_strFmt = L"[Unknown Var Type]";
                break;
        };
        m_c4FmtSerialNum = m_c4SerialNum;
    }
    return m_strFmt;
}


//
//  Set some formatting characterstics of the variable. We invalidate the
//  serial number because this might affect the format.
//
tCIDLib::TVoid
TCQCActVar::SetFormat(  const   tCIDLib::ERadices   eRadix
                        , const tCIDLib::TCard4     c4Digits)
{
    m_c4FmtSerialNum = 0;
    m_c4Digits = c4Digits;
    m_eRadix = eRadix;
}


// ---------------------------------------------------------------------------
//  TCQCActVar: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is a helper which takes an incoming parameter that should be
//  numeric. It will probe the value and see what numeric type it is. It will
//  return the type.
//
//  We use the format string as a temp, and just zero the format serial number
//  to force a re-format next time. If it works, the value is going to change
//  anyway.
//
//  Once we figure out what format it's in, we convert it to a local temp of
//  that type. Then, we look at our type and store/cast the value as required
//  to get it into the right local format. The caller will then look at the
//  member that's correct for his type.
//
//  We rreturn whether it is zero or not, which is useful to know in the
//  calling methods.
//
tCIDLib::TBoolean
TCQCActVar::bCvtNumType(const   tCIDLib::TCh* const pszOp
                        , const tCIDLib::TCard4     c4Line
                        , const TString&            strValue
                        , const tCIDLib::TBoolean   bStrUnsigned)
{
    //
    //  We use a helper from the macro engine parser. It will scan a numeric
    //  literal and tell us the type. This also allows the user to user the
    //  CML suffixes to force a particular type.
    //
    tCIDLib::ERadices eRadix = tCIDLib::ERadices::Auto;
    tCIDLib::TBoolean bExplicit = kCIDLib::False;
    m_c4FmtSerialNum = 0;
    m_strFmt = strValue;
    const tCIDMacroEng::ENumTypes eSrcType = TMacroEngParser::eCheckNumericLiteral
    (
        m_strFmt, eRadix, bExplicit
    );

    // Translate the CML types to field types
    tCIDLib::TBoolean bIsZero = kCIDLib::False;
    switch(eSrcType)
    {
        case tCIDMacroEng::ENumTypes::Card1 :
        case tCIDMacroEng::ENumTypes::Card2 :
        case tCIDMacroEng::ENumTypes::Card4 :
        case tCIDMacroEng::ENumTypes::Card8 :
        {
            const tCIDLib::TCard8 c8Src = m_strFmt.c8Val(eRadix);
            bIsZero = (c8Src == 0);
            switch(m_eType)
            {
                case tCQCKit::EFldTypes::Card :
                    m_c4SrcVal = tCIDLib::TCard4(c8Src);
                    break;

                case tCQCKit::EFldTypes::Float :
                    m_f8SrcVal = tCIDLib::TFloat8(c8Src);
                    break;

                case tCQCKit::EFldTypes::Int :
                    m_i4SrcVal = tCIDLib::TInt4(c8Src);
                    break;

                case tCQCKit::EFldTypes::String :
                    if (bStrUnsigned)
                        m_c8SrcVal = c8Src;
                    else
                        m_i8SrcVal = tCIDLib::TInt8(c8Src);
                    break;

                case tCQCKit::EFldTypes::Time :
                    m_c8SrcVal = c8Src;
                    break;
            };
            break;
        }

        case tCIDMacroEng::ENumTypes::Float4 :
        case tCIDMacroEng::ENumTypes::Float8 :
        {
            const tCIDLib::TFloat8 f8Src = m_strFmt.f8Val();
            bIsZero = (f8Src == 0);
            switch(m_eType)
            {
                case tCQCKit::EFldTypes::Card :
                    m_c4SrcVal = tCIDLib::TCard4(f8Src);
                    break;

                case tCQCKit::EFldTypes::Float :
                    m_f8SrcVal = f8Src;
                    break;

                case tCQCKit::EFldTypes::Int :
                    m_i4SrcVal = tCIDLib::TInt4(f8Src);
                    break;

                case tCQCKit::EFldTypes::String :
                    if (bStrUnsigned)
                        m_c8SrcVal = tCIDLib::TCard4(f8Src);
                    else
                        m_i8SrcVal = tCIDLib::TInt8(f8Src);
                    break;

                case tCQCKit::EFldTypes::Time :
                    m_c8SrcVal = tCIDLib::TCard8(f8Src);
                    break;
            };
            break;
        }

        case tCIDMacroEng::ENumTypes::Int1 :
        case tCIDMacroEng::ENumTypes::Int2 :
        case tCIDMacroEng::ENumTypes::Int4 :
        {
            const tCIDLib::TInt8 i8Src = m_strFmt.i8Val(eRadix);
            bIsZero = (i8Src == 0);
            switch(m_eType)
            {
                case tCQCKit::EFldTypes::Card :
                    m_c4SrcVal = tCIDLib::TCard4(i8Src);
                    break;

                case tCQCKit::EFldTypes::Float :
                    m_f8SrcVal = tCIDLib::TFloat8(i8Src);
                    break;

                case tCQCKit::EFldTypes::Int :
                    m_i4SrcVal = tCIDLib::TInt4(i8Src);
                    break;

                case tCQCKit::EFldTypes::String :
                    if (bStrUnsigned)
                        m_c8SrcVal = tCIDLib::TCard4(i8Src);
                    else
                        m_i8SrcVal = i8Src;
                    break;

                case tCQCKit::EFldTypes::Time :
                    m_c8SrcVal = tCIDLib::TCard8(i8Src);
                    break;
            };
            break;
        }

        default :
            ThrowNotNum(pszOp, c4Line);
            break;
    };
    return bIsZero;
}


tCIDLib::TBoolean
TCQCActVar::bTestLimits(const   tCQCKit::EValResults    eRes
                        , const tCIDLib::TCh* const     pszOpName)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (eRes == tCQCKit::EValResults::OK)
        bRet = kCIDLib::True;
    else if (eRes == tCQCKit::EValResults::Unchanged)
        bRet = kCIDLib::False;
    else if (eRes == tCQCKit::EValResults::Unconvertable)
        ThrowBadParm(pszOpName, CID_LINE);
    else
        ThrowLimits(pszOpName, CID_LINE);
    return bRet;
}



// Called when an operation is valid for this variable's type
tCIDLib::TVoid
TCQCActVar::ThrowBadOp( const   tCIDLib::TCh* const pszOpName
                        , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , c4Line
        , kKitErrs::errcCmd_InvalidVarOp
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , TString(pszOpName)
        , m_strName
    );
}


// Called when a parameter is not convertable to the required type for the op
tCIDLib::TVoid
TCQCActVar::ThrowBadParm(const  tCIDLib::TCh* const pszOpName
                        , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , c4Line
        , kKitErrs::errcCmd_BadVarOpParm
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , TString(pszOpName)
        , m_strName
    );
}


// Called when a parameter is not convertable to the required type for the op
tCIDLib::TVoid
TCQCActVar::ThrowBadTarVal( const   tCIDLib::TCh* const pszOpName
                            , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , c4Line
        , kKitErrs::errcCmd_BadVarTarVal
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , m_strName
        , TString(pszOpName)
    );
}


// Called if a limit violation occurs
tCIDLib::TVoid
TCQCActVar::ThrowLimits(const   tCIDLib::TCh* const pszOpName
                        , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , c4Line
        , kKitErrs::errcCmd_VarLimitErr
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , TString(pszOpName)
        , m_strName
    );
}


// Called when a parameter must be numeric but it doesn't seem to be
tCIDLib::TVoid
TCQCActVar::ThrowNotNum(const   tCIDLib::TCh* const pszOpName
                        , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , c4Line
        , kKitErrs::errcCmd_VarParmNotNum
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , TString(pszOpName)
        , m_strName
    );
}


// Called if they attempt to write to a read only variable
tCIDLib::TVoid
TCQCActVar::ThrowReadOnly(  const   tCIDLib::TCh* const pszOpName
                            , const tCIDLib::TCard4     c4Line)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcCmd_ReadOnlyVar
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::CantDo
        , m_strName
        , TString(pszOpName)
    );
}


