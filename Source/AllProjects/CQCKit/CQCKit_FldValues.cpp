//
// FILE NAME: CQCKit_FldValues.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2003
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFldValue,TObject)
RTTIDecls(TCQCBoolFldValue,TCQCFldValue)
RTTIDecls(TCQCCardFldValue,TCQCFldValue)
RTTIDecls(TCQCFloatFldValue,TCQCFldValue)
RTTIDecls(TCQCIntFldValue,TCQCFldValue)
RTTIDecls(TCQCStringFldValue,TCQCFldValue)
RTTIDecls(TCQCStrListFldValue,TCQCFldValue)
RTTIDecls(TCQCTimeFldValue,TCQCFldValue)



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldValue: Public Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldValue::~TCQCFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the in error indicator
tCIDLib::TBoolean TCQCFldValue::bInError() const
{
    return m_bInError;
}

tCIDLib::TBoolean TCQCFldValue::bInError(const tCIDLib::TBoolean bToSet)
{
    if (bToSet != m_bInError)
    {
        m_bInError = bToSet;
        c4IncSerialNum();
    }

    //
    //  Either way clear the first value flag since either we have gotten a good
    //  value or been explicitly put into error state.
    //
    m_bGotFirstVal = kCIDLib::True;

    return m_bInError;
}


// Return the driver and field ids
tCIDLib::TCard4 TCQCFldValue::c4DriverId() const
{
    return m_c4DriverId;
}


tCIDLib::TCard4 TCQCFldValue::c4FieldId() const
{
    return m_c4FieldId;
}


// Increment the serial number, and return the new value
tCIDLib::TCard4 TCQCFldValue::c4IncSerialNum()
{
    return ++m_c4SerialNum;
}


// Get or set the serial number
tCIDLib::TCard4 TCQCFldValue::c4SerialNum() const
{
    return m_c4SerialNum;
}

tCIDLib::TCard4 TCQCFldValue::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


// Returns the field type, which is fixed once constructed
tCQCKit::EFldTypes TCQCFldValue::eFldType() const
{
    return m_eFldType;
}


// Store new driver and field ids
tCIDLib::TVoid
TCQCFldValue::SetIds(const  tCIDLib::TCard4 c4NewDriverId
                    , const tCIDLib::TCard4 c4NewFldId)
{
    m_c4DriverId = c4NewDriverId;
    m_c4FieldId  = c4NewFldId;

    // Reset some other stuff
    m_bInError = kCIDLib::False;
    m_c4SerialNum = 1;

    // And clear the first value field since we are being reused
    m_bGotFirstVal = kCIDLib::False;
}


//
//  In some cases the default value for a field is fine and it's high overhead
//  to have to set the same thing explicitly again. So this can be called. All we
//  do is just set the 'got first val' flag and clear the error flag.
//
tCIDLib::TVoid TCQCFldValue::UseDefValue()
{
    m_bGotFirstVal = kCIDLib::True;
    m_bInError = kCIDLib::False;
}



// ---------------------------------------------------------------------------
//  TCQCFldValue: Hidden Constructors and operators
// ---------------------------------------------------------------------------

// The error state and first value flags are NOT copied
TCQCFldValue::TCQCFldValue(const tCQCKit::EFldTypes eFldType) :

    m_bGotFirstVal(kCIDLib::False)
    , m_bInError(kCIDLib::False)
    , m_c4DriverId(0)
    , m_c4FieldId(0)
    , m_c4SerialNum(1)
    , m_eFldType(eFldType)
{
}

TCQCFldValue::TCQCFldValue( const   tCIDLib::TCard4     c4DriverId
                            , const tCIDLib::TCard4     c4FieldId
                            , const tCQCKit::EFldTypes  eFldType) :

    m_bGotFirstVal(kCIDLib::False)
    , m_bInError(kCIDLib::False)
    , m_c4DriverId(c4DriverId)
    , m_c4FieldId(c4FieldId)
    , m_c4SerialNum(1)
    , m_eFldType(eFldType)
{
}

TCQCFldValue::TCQCFldValue(const TCQCFldValue& fvToCopy) :

    m_bGotFirstVal(kCIDLib::False)
    , m_bInError(kCIDLib::False)
    , m_c4DriverId(fvToCopy.m_c4DriverId)
    , m_c4FieldId(fvToCopy.m_c4FieldId)
    , m_c4SerialNum(fvToCopy.m_c4SerialNum)
    , m_eFldType(fvToCopy.m_eFldType)
{
}

tCIDLib::TVoid TCQCFldValue::operator=(const TCQCFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        m_bGotFirstVal  = kCIDLib::False;
        m_bInError      = kCIDLib::False;
        m_c4DriverId    = fvToAssign.m_c4DriverId;
        m_c4FieldId     = fvToAssign.m_c4FieldId;
        m_c4SerialNum   = fvToAssign.m_c4SerialNum;
        m_eFldType      = fvToAssign.m_eFldType;
    }
}


// ---------------------------------------------------------------------------
//  TCQCFldValue: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  For use by derived classes, to avoid multiple bumps of the serial number, since
//  they are going to bump if either the error state changes or the value changes.
//
//  THE CALLER MUST bump the serial number of this is a change!
//
tCIDLib::TVoid TCQCFldValue::ClearError()
{
    m_bInError = kCIDLib::False;

    //
    //  Clear the first value flag, since setting an explicit error state is
    //  a value by our reckoning.
    //
    m_bGotFirstVal = kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCBoolFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCBoolFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCBoolFldValue::TCQCBoolFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::Boolean)
    , m_bValue(kCIDLib::False)
{
}

TCQCBoolFldValue::TCQCBoolFldValue( const   tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::Boolean)
    , m_bValue(kCIDLib::False)
{
}

TCQCBoolFldValue::TCQCBoolFldValue(const TCQCBoolFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_bValue(fvToCopy.m_bValue)
{
}

TCQCBoolFldValue::~TCQCBoolFldValue()
{
}


// ---------------------------------------------------------------------------
//  : Public operators
// ---------------------------------------------------------------------------
TCQCBoolFldValue&
TCQCBoolFldValue::operator=(const TCQCBoolFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_bValue = fvToAssign.m_bValue;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCBoolFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCBoolFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected boolean src value");
    const TCQCBoolFldValue& fvBool = static_cast<const TCQCBoolFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_bValue != fvBool.m_bValue)
    {
        m_bValue = fvBool.m_bValue;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}


// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCBoolFldValue::eStoreValue(  const   TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some type of boolean limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldBoolLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldBoolLimit& fldlBool
    (
        static_cast<const TCQCFldBoolLimit&>(fldlTest)
    );

    tCIDLib::TBoolean bTmp;
    tCQCKit::EValResults eRes = fldlBool.eValidate(strValue, bTmp);

    // If it's good, then see if it's changed or not
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (m_bValue == bTmp)
        {
            // If not coming out of error state, say unchanged
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
        else
        {
            m_bValue = bTmp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCBoolFldValue::eTestStatement(const  tCQCKit::EStatements eToTest
                                , const TString&             strTestValue) const
{
    // To save a lot of code, assume a false reply unless set otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;
    tCIDLib::TBoolean bTmp;
    switch(eToTest)
    {
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        {
            // See if the test value holds a legal boolean value
            if (!facCQCKit().bCheckBoolVal(strTestValue, bTmp))
                return tCQCKit::EStmtRes::BadValue;

            switch(eToTest)
            {
                case tCQCKit::EStatements::IsEqual :
                {
                    if (m_bValue == bTmp)
                        eRes = tCQCKit::EStmtRes::True;
                    break;
                }

                case tCQCKit::EStatements::NotEqual :
                {
                    if (m_bValue != bTmp)
                        eRes = tCQCKit::EStmtRes::True;
                    break;
                }

                case tCQCKit::EStatements::IsGThan :
                {
                    if (m_bValue > bTmp)
                        eRes = tCQCKit::EStmtRes::True;
                    break;
                }

                case tCQCKit::EStatements::IsGThanEq :
                case tCQCKit::EStatements::IsLsThanEq :
                {
                    // There are only two values so these will always be true
                    eRes = tCQCKit::EStmtRes::True;
                    break;
                }

                case tCQCKit::EStatements::IsLsThan :
                {
                    if (m_bValue < bTmp)
                        eRes = tCQCKit::EStmtRes::True;
                    break;
                }
            };
            break;
        }

        case tCQCKit::EStatements::IsNonNull :
        {
            if (m_bValue)
                eRes = tCQCKit::EStmtRes::True;
            break;
        }

        case tCQCKit::EStatements::IsPositive :
        case tCQCKit::EStatements::IsTrue :
        {
            if (m_bValue)
                eRes = tCQCKit::EStmtRes::True;
            break;
        }

        case tCQCKit::EStatements::IsNegative :
        case tCQCKit::EStatements::IsFalse :
        {
            if (!m_bValue)
                eRes = tCQCKit::EStmtRes::True;
            break;
        }

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCBoolFldValue::Format(TString& strToFill) const
{
    if (m_bValue)
        strToFill = kCQCKit::pszFld_True;
    else
        strToFill = kCQCKit::pszFld_False;
}


tCIDLib::TVoid TCQCBoolFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_bValue;
}

tCIDLib::TVoid TCQCBoolFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_bValue;
}


// ---------------------------------------------------------------------------
//  TCQCBoolFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCBoolFldValue::bSetValue(const tCIDLib::TBoolean bToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (bToSet != m_bValue)
    {
        m_bValue = bToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}

tCIDLib::TBoolean TCQCBoolFldValue::bValue() const
{
    return m_bValue;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCCardFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCardFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCardFldValue::TCQCCardFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::Card)
    , m_c4Value(0)
{
}

TCQCCardFldValue::TCQCCardFldValue( const   tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::Card)
    , m_c4Value(0)
{
}

TCQCCardFldValue::TCQCCardFldValue(const TCQCCardFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_c4Value(fvToCopy.m_c4Value)
{
}

TCQCCardFldValue::~TCQCCardFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCCardFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCCardFldValue&
TCQCCardFldValue::operator=(const TCQCCardFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_c4Value = fvToAssign.m_c4Value;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCCardFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCCardFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected cardinal src value");
    const TCQCCardFldValue& fvCard = static_cast<const TCQCCardFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_c4Value != fvCard.m_c4Value)
    {
        m_c4Value = fvCard.m_c4Value;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}



// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCCardFldValue::eStoreValue(  const   TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some card type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldCardLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldCardLimit& fldlCard
    (
        static_cast<const TCQCFldCardLimit&>(fldlTest)
    );
    tCIDLib::TCard4 c4Tmp;
    tCQCKit::EValResults eRes = fldlCard.eValidate(strValue, c4Tmp);

    // If it's good, then see if it's changed or not
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (m_c4Value == c4Tmp)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_c4Value = c4Tmp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }

    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCCardFldValue::eTestStatement(const  tCQCKit::EStatements    eToTest
                                , const TString&                strTestValue) const
{
    // To save code, assume false until proven otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;
    switch(eToTest)
    {
        case tCQCKit::EStatements::IsTrue :
        case tCQCKit::EStatements::IsNonNull :
            if (m_c4Value != 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsFalse :
            if (m_c4Value == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        // These all require that the passed value can be converted to binary
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        case tCQCKit::EStatements::IsMultipleOf :
        {
            tCIDLib::TBoolean bRes;
            tCIDLib::TCard4 c4Val = TRawStr::c4AsBinary(strTestValue.pszBuffer(), bRes);
            if (bRes)
            {
                if (eToTest == tCQCKit::EStatements::IsEqual)
                    bRes = (m_c4Value == c4Val);
                else if (eToTest == tCQCKit::EStatements::NotEqual)
                    bRes = (m_c4Value != c4Val);
                else if (eToTest == tCQCKit::EStatements::IsGThan)
                    bRes = (m_c4Value > c4Val);
                else if (eToTest == tCQCKit::EStatements::IsGThanEq)
                    bRes = (m_c4Value >= c4Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThan)
                    bRes = (m_c4Value < c4Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThanEq)
                    bRes = (m_c4Value <= c4Val);
                else if (eToTest == tCQCKit::EStatements::IsMultipleOf)
                    bRes = ((m_c4Value % c4Val) == 0);

                if (bRes)
                    eRes = tCQCKit::EStmtRes::True;
            }
             else
            {
                eRes = tCQCKit::EStmtRes::BadValue;
            }
            break;
        }

        case tCQCKit::EStatements::IsAlpha :
        case tCQCKit::EStatements::IsNegative :
            // Inherently false
            break;

        case tCQCKit::EStatements::IsAlphaNum :
        case tCQCKit::EStatements::IsPositive :
            // Inherently true
            eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsEven :
            if ((m_c4Value & 0x1) == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsOdd :
            if ((m_c4Value & 0x1) == 1)
                eRes = tCQCKit::EStmtRes::True;
            break;

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCCardFldValue::Format(TString& strToFill) const
{
    strToFill.SetFormatted(m_c4Value, tCIDLib::ERadices::Dec);
}


tCIDLib::TVoid TCQCCardFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_c4Value;
}

tCIDLib::TVoid TCQCCardFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_c4Value;
}


// ---------------------------------------------------------------------------
//  TCQCCardFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCCardFldValue::bSetValue(const tCIDLib::TCard4 c4ToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (c4ToSet != m_c4Value)
    {
        m_c4Value = c4ToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();

    return bRet;
}

tCIDLib::TCard4 TCQCCardFldValue::c4Value() const
{
    return m_c4Value;
}


tCIDLib::TVoid TCQCCardFldValue::Inc()
{
    m_c4Value++;
    c4IncSerialNum();
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFloatFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFloatFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFloatFldValue::TCQCFloatFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::Float)
    , m_f8Value(0)
{
}

TCQCFloatFldValue::TCQCFloatFldValue(const  tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::Float)
    , m_f8Value(0)
{
}

TCQCFloatFldValue::TCQCFloatFldValue(const TCQCFloatFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_f8Value(fvToCopy.m_f8Value)
{
}

TCQCFloatFldValue::~TCQCFloatFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCFloatFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCFloatFldValue&
TCQCFloatFldValue::operator=(const TCQCFloatFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_f8Value = fvToAssign.m_f8Value;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCFloatFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCFloatFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected float src value");
    const TCQCFloatFldValue& fvFloat = static_cast<const TCQCFloatFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_f8Value != fvFloat.m_f8Value)
    {
        m_f8Value = fvFloat.m_f8Value;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}


// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCFloatFldValue::eStoreValue( const   TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some float type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldFloatLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldFloatLimit& fldlFloat
    (
        static_cast<const TCQCFldFloatLimit&>(fldlTest)
    );

    tCIDLib::TFloat8 f8Tmp;
    tCQCKit::EValResults eRes = fldlFloat.eValidate(strValue, f8Tmp);

    // If it's good, then see if it's changed or not
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (m_f8Value == f8Tmp)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_f8Value = f8Tmp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCFloatFldValue::eTestStatement(const  tCQCKit::EStatements eToTest
                                 , const TString&             strTestValue) const
{
    // To save code, assume false until proven otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;
    switch(eToTest)
    {
        case tCQCKit::EStatements::IsTrue :
        case tCQCKit::EStatements::IsNonNull :
            if (m_f8Value != 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsFalse :
            if (m_f8Value == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        // These all require that the passed value can be converted to binary
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        {
            tCIDLib::TBoolean bRes;
            tCIDLib::TFloat8 f8Val = TRawStr::f8AsBinary(strTestValue.pszBuffer(), bRes);
            if (bRes)
            {
                if (eToTest == tCQCKit::EStatements::IsEqual)
                    bRes = (m_f8Value == f8Val);
                else if (eToTest == tCQCKit::EStatements::NotEqual)
                    bRes = (m_f8Value != f8Val);
                else if (eToTest == tCQCKit::EStatements::IsGThan)
                    bRes = (m_f8Value > f8Val);
                else if (eToTest == tCQCKit::EStatements::IsGThanEq)
                    bRes = (m_f8Value >= f8Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThan)
                    bRes = (m_f8Value < f8Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThanEq)
                    bRes = (m_f8Value <= f8Val);

                if (bRes)
                    eRes = tCQCKit::EStmtRes::True;
            }
             else
            {
                eRes = tCQCKit::EStmtRes::BadValue;
            }
            break;
        }

        case tCQCKit::EStatements::IsNegative :
            if (m_f8Value < 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsPositive :
            if (m_f8Value >= 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsAlpha :
        case tCQCKit::EStatements::IsAlphaNum :
            // Inherently false (not even alpha num since we have a decimal point)
            break;

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCFloatFldValue::Format(TString& strToFill) const
{
    strToFill.SetFormatted(m_f8Value, 5UL);
}


tCIDLib::TVoid TCQCFloatFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_f8Value;
}

tCIDLib::TVoid TCQCFloatFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_f8Value;
}


// ---------------------------------------------------------------------------
//  TCQCFloatFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFloatFldValue::bSetValue(const tCIDLib::TFloat8 f8ToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (f8ToSet != m_f8Value)
    {
        m_f8Value = f8ToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}

tCIDLib::TFloat8 TCQCFloatFldValue::f8Value() const
{
    return m_f8Value;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntFldValue::TCQCIntFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::Int)
    , m_i4Value(0)
{
}

TCQCIntFldValue::TCQCIntFldValue(const  tCIDLib::TCard4 c4DriverId
                                , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::Int)
    , m_i4Value(0)
{
}

TCQCIntFldValue::TCQCIntFldValue(const TCQCIntFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_i4Value(fvToCopy.m_i4Value)
{
}

TCQCIntFldValue::~TCQCIntFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCIntFldValue& TCQCIntFldValue::operator=(const TCQCIntFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_i4Value = fvToAssign.m_i4Value;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected int src value");
    const TCQCIntFldValue& fvInt = static_cast<const TCQCIntFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_i4Value != fvInt.m_i4Value)
    {
        m_i4Value = fvInt.m_i4Value;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}


// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCIntFldValue::eStoreValue(const  TString&        strValue
                            , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some int type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldIntLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldIntLimit& fldlInt
    (
        static_cast<const TCQCFldIntLimit&>(fldlTest)
    );

    tCIDLib::TInt4 i4Tmp;
    tCQCKit::EValResults eRes = fldlInt.eValidate(strValue, i4Tmp);

    // If it's good, then see if it's changed or not
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (m_i4Value == i4Tmp)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_i4Value = i4Tmp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCIntFldValue::eTestStatement(const   tCQCKit::EStatements    eToTest
                                , const TString&                strTestValue) const
{
    // To save code, assume false until proven otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;
    switch(eToTest)
    {
        case tCQCKit::EStatements::IsTrue :
        case tCQCKit::EStatements::IsNonNull :
            if (m_i4Value != 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsFalse :
            if (m_i4Value == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        // These all require that the passed value can be converted to binary
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        case tCQCKit::EStatements::IsMultipleOf :
        {
            tCIDLib::TBoolean bRes;
            tCIDLib::TInt4 i4Val = TRawStr::i4AsBinary(strTestValue.pszBuffer(), bRes);
            if (bRes)
            {
                if (eToTest == tCQCKit::EStatements::IsEqual)
                    bRes = (m_i4Value == i4Val);
                else if (eToTest == tCQCKit::EStatements::NotEqual)
                    bRes = (m_i4Value != i4Val);
                else if (eToTest == tCQCKit::EStatements::IsGThan)
                    bRes = (m_i4Value > i4Val);
                else if (eToTest == tCQCKit::EStatements::IsGThanEq)
                    bRes = (m_i4Value >= i4Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThan)
                    bRes = (m_i4Value < i4Val);
                else if (eToTest == tCQCKit::EStatements::IsLsThanEq)
                    bRes = (m_i4Value <= i4Val);
                else if (eToTest == tCQCKit::EStatements::IsMultipleOf)
                    bRes = ((m_i4Value % i4Val) == 0);

                if (bRes)
                    eRes = tCQCKit::EStmtRes::True;
            }
             else
            {
                eRes = tCQCKit::EStmtRes::BadValue;
            }
            break;
        }

        case tCQCKit::EStatements::IsNegative :
            if (m_i4Value < 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsPositive :
            if (m_i4Value >= 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsEven :
            if ((m_i4Value % 1) == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsOdd :
            if ((m_i4Value % 1) == 1)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsAlpha :
            // Inherently false
            break;

        case tCQCKit::EStatements::IsAlphaNum :
            // True if not negative, else there's a sign
            if (m_i4Value >= 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCIntFldValue::Format(TString& strToFill) const
{
    strToFill.SetFormatted
    (
        m_i4Value, tCIDLib::ERadices::Dec, TLocale::chNegativeSign()
    );
}


tCIDLib::TVoid TCQCIntFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_i4Value;
}

tCIDLib::TVoid TCQCIntFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_i4Value;
}


// ---------------------------------------------------------------------------
//  TCQCIntFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntFldValue::bSetValue(const tCIDLib::TInt4 i4ToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (i4ToSet != m_i4Value)
    {
        m_i4Value = i4ToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}

tCIDLib::TInt4 TCQCIntFldValue::i4Value() const
{
    return m_i4Value;
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCStringFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStringFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStringFldValue::TCQCStringFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::String)
{
}

TCQCStringFldValue::TCQCStringFldValue( const   tCIDLib::TCard4 c4DriverId
                                        , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::String)
{
}

TCQCStringFldValue::TCQCStringFldValue(const TCQCStringFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_strValue(fvToCopy.m_strValue)
{
}

TCQCStringFldValue::~TCQCStringFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCStringFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCStringFldValue&
TCQCStringFldValue::operator=(const TCQCStringFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_strValue = fvToAssign.m_strValue;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCStringFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCStringFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected string src value");
    const TCQCStringFldValue& fvStr = static_cast<const TCQCStringFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_strValue != fvStr.m_strValue)
    {
        m_strValue = fvStr.m_strValue;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}



// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCStringFldValue::eStoreValue( const  TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some string type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldStrLimit::clsThis()), L"Got wrong limit type"
    );

    //
    //  It doesn't convert in this case, it just validates in place so we
    //  have to copy it over if good.
    //
    const TCQCFldStrLimit& fldlStr
    (
        static_cast<const TCQCFldStrLimit&>(fldlTest)
    );

    tCQCKit::EValResults eRes = fldlStr.eValidate(strValue);
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (strValue == m_strValue)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_strValue = strValue;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCStringFldValue::eTestStatement( const   tCQCKit::EStatements eToTest
                                    , const TString&             strTestValue) const
{
    // To save code, assume false until proven otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;
    switch(eToTest)
    {
        case tCQCKit::EStatements::IsEqual :
            if (m_strValue == strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::NotEqual :
            if (m_strValue != strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsTrue :
            if (m_strValue == kCQCKit::pszFld_True)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsFalse :
            if (m_strValue == kCQCKit::pszFld_False)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsGThan :
            if (m_strValue > strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsGThanEq :
            if (m_strValue >= strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsLsThan :
            if (m_strValue < strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsLsThanEq :
            if (m_strValue < strTestValue)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsNonNull :
            if (!m_strValue.bIsEmpty())
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsAlpha :
            if (m_strValue.bIsAlpha())
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsAlphaNum :
            if (m_strValue.bIsAlphaNum())
                eRes = tCQCKit::EStmtRes::True;
            break;

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCStringFldValue::Format(TString& strToFill) const
{
    strToFill = m_strValue;
}


tCIDLib::TVoid TCQCStringFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_strValue;
}

tCIDLib::TVoid TCQCStringFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_strValue;
}


// ---------------------------------------------------------------------------
//  TCQCStringFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCStringFldValue::bSetValue(const TString& strToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (strToSet != m_strValue)
    {
        m_strValue = strToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}

const TString& TCQCStringFldValue::strValue() const
{
    return m_strValue;
}

TString& TCQCStringFldValue::strValue()
{
    return m_strValue;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCStrListFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCStrListFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCStrListFldValue::TCQCStrListFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::StringList)
{
}

TCQCStrListFldValue::TCQCStrListFldValue(const  tCIDLib::TCard4 c4DriverId
                                        , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::StringList)
{
}

TCQCStrListFldValue::TCQCStrListFldValue(const TCQCStrListFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_colValue(fvToCopy.m_colValue)
{
    // The temp list doesn't need to be copied
}

TCQCStrListFldValue::~TCQCStrListFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCStrListFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCStrListFldValue&
TCQCStrListFldValue::operator=(const TCQCStrListFldValue& fvToAssign)
{
    // The temp list doesn't need to be copied
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_colValue = fvToAssign.m_colValue;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCStrListFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCStrListFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected stringlist src value");
    const TCQCStrListFldValue& fvStrL = static_cast<const TCQCStrListFldValue&>(fvSrc);

    return bSetValue(fvStrL.m_colValue);
}


// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCStrListFldValue::eStoreValue(const  TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some string list type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldStrListLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldStrListLimit& fldlList
    (
        static_cast<const TCQCFldStrListLimit&>(fldlTest)
    );

    tCQCKit::EValResults eRes = fldlList.eValidate(m_colComp);

    // If it's good, then see if it's changed or not
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (m_colComp == m_colValue)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_colValue = m_colComp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCStrListFldValue::eTestStatement(const   tCQCKit::EStatements eToTest
                                    , const TString&             strValue) const
{
    // To save code, assume false until proven otherwise
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;

    switch(eToTest)
    {
        case tCQCKit::EStatements::IsNonNull :
        {
            if (!m_colValue.bIsEmpty())
                eRes = tCQCKit::EStmtRes::True;
            break;
        }

        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        {
            //
            //  This one is kind of brutal. We have to parse out the values
            //  of the string list from the passed value, and see if they
            //  match our list values.
            //
            tCIDLib::TCard4 c4ErrIndex;
            if (TStringTokenizer::bParseQuotedCommaList(strValue, m_colComp, c4ErrIndex))
            {
                const tCIDLib::TCard4 c4Count = m_colValue.c4ElemCount();
                if (c4Count == m_colComp.c4ElemCount())
                {
                    //
                    //  They have the same count of elements. So set the return
                    //  to true, and that way we can just set it back to false
                    //  if we get any mismatch.
                    //
                    eRes = tCQCKit::EStmtRes::True;
                    tCIDLib::TCard4 c4Index = 0;
                    for (; c4Index < c4Count; c4Index++)
                    {
                        if (m_colValue[c4Index] != m_colComp[c4Index])
                        {
                            eRes = tCQCKit::EStmtRes::False;
                            break;
                        }
                    }
                }
            }
             else
            {
                // The value isn't correctly formatted
                eRes = tCQCKit::EStmtRes::BadValue;
            }

            // If it's NotEqual, then flip the status
            if (eToTest == tCQCKit::EStatements::NotEqual)
            {
                if (eRes == tCQCKit::EStmtRes::False)
                    eRes = tCQCKit::EStmtRes::True;
                else if (eRes == tCQCKit::EStmtRes::True)
                    eRes = tCQCKit::EStmtRes::False;
            }
            break;
        }

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };

    return eRes;
}


//
//  We spit out a quoted comma list in the standard form.
//
tCIDLib::TVoid TCQCStrListFldValue::Format(TString& strToFill) const
{
    strToFill.Clear();

    const tCIDLib::TCard4 c4Count = m_colValue.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        TStringTokenizer::BuildQuotedCommaList(m_colValue[c4Index], strToFill);
}


tCIDLib::TVoid TCQCStrListFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_colValue;
}

tCIDLib::TVoid TCQCStrListFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_colValue;
}


// ---------------------------------------------------------------------------
//  TCQCStrListFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCStrListFldValue::bClearList()
{
    tCIDLib::TBoolean bChanged = bInError();
    bInError(kCIDLib::False);

    if (!m_colValue.bIsEmpty())
    {
        m_colValue.RemoveAll();
        bChanged = kCIDLib::True;
    }

    if (bChanged)
        c4IncSerialNum();
    return bChanged;
}




//
//  Provide us a list of strings to set as our new value.
//
tCIDLib::TBoolean TCQCStrListFldValue::bSetValue(const TVector<TString>& colToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    const tCIDLib::TCard4 c4Count = m_colValue.c4ElemCount();
    if (c4Count == colToSet.c4ElemCount())
    {
        // It has the same number of elements, so check them
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4Count; c4Index++)
        {
            if (m_colValue[c4Index] != colToSet[c4Index])
            {
                // There is a difference
                bRet = kCIDLib::True;
                break;
            }
        }
    }
     else
    {
        // Different count so obviously different
        bRet = kCIDLib::True;
    }

    if (bRet)
    {
        m_colValue = colToSet;
        c4IncSerialNum();
    }
    return bRet;
}


//
//  A convenience method that takes the value in it's textual format. We parse
//  to a temp list, and then copy over if it's a new value.
//
tCIDLib::TBoolean
TCQCStrListFldValue::bSetValue(const TString& strText, tCIDLib::TCard4& c4ErrIndex)
{
    // If it doesn't convert, bad value
    if (!TStringTokenizer::bParseQuotedCommaList(strText, m_colComp, c4ErrIndex))
        return kCIDLib::False;

    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    // Compare the old and new
    const tCIDLib::TCard4 c4Count = m_colValue.c4ElemCount();
    if (c4Count == m_colComp.c4ElemCount())
    {
        // It has the same number of elements, so check them
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4Count; c4Index++)
        {
            if (m_colValue[c4Index] != m_colComp[c4Index])
            {
                bRet = kCIDLib::True;
                break;
            }
        }
    }
     else
    {
        // Different count so obviously different
        bRet = kCIDLib::True;
    }

    if (bRet)
    {
        m_colValue = m_colComp;
        c4IncSerialNum();
    }
    return bRet;
}


// Provide const and non-const access to our value
const TVector<TString>& TCQCStrListFldValue::colValue() const
{
    return m_colValue;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCTimeFldValue
//  PREFIX: fv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTimeFldValue: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTimeFldValue::TCQCTimeFldValue() :

    TCQCFldValue(tCQCKit::EFldTypes::Time)
    , m_c8Value(0)
{
}

TCQCTimeFldValue::TCQCTimeFldValue( const   tCIDLib::TCard4 c4DriverId
                                    , const tCIDLib::TCard4 c4FieldId) :

    TCQCFldValue(c4DriverId, c4FieldId, tCQCKit::EFldTypes::Time)
    , m_c8Value(0)
{
}

TCQCTimeFldValue::TCQCTimeFldValue(const TCQCTimeFldValue& fvToCopy) :

    TCQCFldValue(fvToCopy)
    , m_c8Value(fvToCopy.m_c8Value)
{
}

TCQCTimeFldValue::~TCQCTimeFldValue()
{
}


// ---------------------------------------------------------------------------
//  TCQCTimeFldValue: Public operators
// ---------------------------------------------------------------------------
TCQCTimeFldValue&
TCQCTimeFldValue::operator=(const TCQCTimeFldValue& fvToAssign)
{
    if (this != &fvToAssign)
    {
        TCQCFldValue::operator=(fvToAssign);
        m_c8Value = fvToAssign.m_c8Value;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCTimeFldValue: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCTimeFldValue::bPolyValueCopy(const TCQCFldValue& fvSrc)
{
    CIDAssert(fvSrc.eFldType() == eFldType(), L"Expected int time value");
    const TCQCTimeFldValue& fvTime = static_cast<const TCQCTimeFldValue&>(fvSrc);

    // If we were in error, this is a change, and we exit error state now
    tCIDLib::TBoolean bRet = bInError();
    if (bRet)
        ClearError();

    // If the value is different, store it and indicate a change
    if (m_c8Value != fvTime.m_c8Value)
    {
        m_c8Value = fvTime.m_c8Value;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}



// Limit test and validate a text value and store it if good
tCQCKit::EValResults
TCQCTimeFldValue::eStoreValue(  const   TString&        strValue
                                , const TCQCFldLimit&   fldlTest)
{
    // If debugging, make sure it's some time type limit
    CIDAssert
    (
        fldlTest.bIsDescendantOf(TCQCFldTimeLimit::clsThis()), L"Got wrong limit type"
    );

    // It is, so let it do the conversion for us and return his result
    const TCQCFldTimeLimit& fldlTime
    (
        static_cast<const TCQCFldTimeLimit&>(fldlTest)
    );

    tCIDLib::TCard8 c8Tmp;
    tCQCKit::EValResults eRes = fldlTime.eValidate(strValue, c8Tmp);
    if (eRes == tCQCKit::EValResults::OK)
    {
        if (c8Tmp == m_c8Value)
        {
            if (!bInError())
                eRes = tCQCKit::EValResults::Unchanged;
        }
         else
        {
            m_c8Value = c8Tmp;
        }

        if (eRes == tCQCKit::EValResults::OK)
        {
            ClearError();
            c4IncSerialNum();
        }
    }
     else
    {
        // A bad value, go into error state
        bInError(kCIDLib::True);
    }
    return eRes;
}


// Test a statement against our value and a possible comp value
tCQCKit::EStmtRes
TCQCTimeFldValue::eTestStatement(const  tCQCKit::EStatements eToTest
                                , const TString&             strTestValue) const
{
    //
    //  To save code, assume false until proven otherwise. For this type,
    //  only the equality and relative magnitude statements make any sense,
    //  so others are rejected.
    //
    tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::False;

    switch(eToTest)
    {
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        case tCQCKit::EStatements::IsMultipleOf :
        {
            // These require the comp value and that it be a numeric value
            tCIDLib::TBoolean bOk;
            const tCIDLib::TCard8 c8Test = TRawStr::c8AsBinary
            (
                strTestValue.pszBuffer(), bOk, tCIDLib::ERadices::Hex
            );

            // If it doesn't convert, bad value
            if (!bOk)
                return tCQCKit::EStmtRes::BadValue;

            bOk = kCIDLib::False;
            if (eToTest == tCQCKit::EStatements::IsEqual)
                bOk = (m_c8Value == c8Test);
            else if (eToTest == tCQCKit::EStatements::NotEqual)
                bOk = (m_c8Value != c8Test);
            else if (eToTest == tCQCKit::EStatements::IsGThan)
                bOk = (m_c8Value > c8Test);
            else if (eToTest == tCQCKit::EStatements::IsGThanEq)
                bOk = (m_c8Value >= c8Test);
            else if (eToTest == tCQCKit::EStatements::IsLsThan)
                bOk = (m_c8Value < c8Test);
            else if (eToTest == tCQCKit::EStatements::IsLsThanEq)
                bOk = (m_c8Value <= c8Test);
            else if (eToTest == tCQCKit::EStatements::IsMultipleOf)
                bOk = ((m_c8Value % c8Test) == 0);

            if (bOk)
                eRes = tCQCKit::EStmtRes::True;
            break;
        }

        case tCQCKit::EStatements::IsTrue :
        case tCQCKit::EStatements::IsNonNull :
            if (m_c8Value != 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsFalse :
            if (m_c8Value == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsEven :
            if ((m_c8Value & 0x1) == 0)
                eRes = tCQCKit::EStmtRes::True;
            break;

        case tCQCKit::EStatements::IsOdd :
            if ((m_c8Value & 0x1) == 1)
                eRes = tCQCKit::EStmtRes::True;
            break;


        case tCQCKit::EStatements::IsAlpha :
            // Inherently false
            break;

        case tCQCKit::EStatements::IsAlphaNum :
            // Inherently true since we are an unsigned value
            eRes = tCQCKit::EStmtRes::True;
            break;

        default :
            eRes = tCQCKit::EStmtRes::BadStatement;
            break;
    };
    return eRes;
}


tCIDLib::TVoid TCQCTimeFldValue::Format(TString& strToFill) const
{
    strToFill = L"0x";
    strToFill.AppendFormatted(m_c8Value, tCIDLib::ERadices::Hex);
}


tCIDLib::TVoid TCQCTimeFldValue::StreamInValue(TBinInStream& strmSrc)
{
    strmSrc >> m_c8Value;
}


tCIDLib::TVoid TCQCTimeFldValue::StreamOutValue(TBinOutStream& strmTar) const
{
    strmTar << m_c8Value;
}


// ---------------------------------------------------------------------------
//  TCQCTimeFldValue: Public, non-virtual methods
// ---------------------------------------------------------------------------

// This one is for when the caller wants to know if the value will change
tCIDLib::TBoolean TCQCTimeFldValue::bSetValue(const tCIDLib::TCard8& c8ToSet)
{
    tCIDLib::TBoolean bRet = bInError();
    bInError(kCIDLib::False);

    if (c8ToSet != m_c8Value)
    {
        m_c8Value = c8ToSet;
        bRet = kCIDLib::True;
    }

    if (bRet)
        c4IncSerialNum();
    return bRet;
}


tCIDLib::TCard8 TCQCTimeFldValue::c8Value() const
{
    return m_c8Value;
}

