//
// FILE NAME: GenProtoS_FldValue.cpp
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
//  This file implements the smart union class that os used to hold field
//  values at runtime.
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
RTTIDecls(TGenProtoFldVal,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldVal
//  PREFIX: fval
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoFldVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoFldVal::TGenProtoFldVal(const tCQCKit::EFldTypes eType) :

    m_eType(eType)
    , m_pstrValue(0)
{
    // Initialize our data members according to our given type
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
            m_uValue.bValue = kCIDLib::False;
            break;

        case tCQCKit::EFldTypes::Card :
            m_uValue.c4Value = 0;
            break;

        case tCQCKit::EFldTypes::Int :
            m_uValue.i4Value = 0;
            break;

        case tCQCKit::EFldTypes::Float :
            m_uValue.f8Value = 0;
            break;

        case tCQCKit::EFldTypes::String :
            m_pstrValue = new TString;
            break;

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_eType))
                , TString(L"tCQCKit::EFldTypes")
            );
            #endif
            break;
    }
}

TGenProtoFldVal::TGenProtoFldVal(const TGenProtoFldVal& fvalToCopy) :

    m_eType(fvalToCopy.m_eType)
    , m_pstrValue(0)
{
    //
    //  If our type is string, we have to deep copy the string. Else just
    //  assign the union value.
    //
    if (fvalToCopy.m_eType == tCQCKit::EFldTypes::String)
        m_pstrValue = new TString(*fvalToCopy.m_pstrValue);
    else
        m_uValue = fvalToCopy.m_uValue;
}

TGenProtoFldVal::~TGenProtoFldVal()
{
    delete m_pstrValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoFldVal: Public operators
// ---------------------------------------------------------------------------
TGenProtoFldVal& TGenProtoFldVal::operator=(const TGenProtoFldVal& fvalToAssign)
{
    if (this != &fvalToAssign)
    {
        if (m_eType == tCQCKit::EFldTypes::String)
        {
            //
            //  We are current a string type. So if the source is not, then
            //  get rid of our string. If the source is, then just copy his
            //  text to our string.
            //
            if (fvalToAssign.m_eType != tCQCKit::EFldTypes::String)
            {
                // Just shallow copy the union and clear our string
                m_uValue = fvalToAssign.m_uValue;
                m_pstrValue->Clear();
            }
             else
            {
                // We are both string, so just assing the text
                *m_pstrValue = *fvalToAssign.m_pstrValue;
            }
        }
         else if (fvalToAssign.m_eType == tCQCKit::EFldTypes::String)
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
//  TGenProtoFldVal: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenProtoFldVal::bCheckFldType(const tCQCKit::EFldTypes eToCheck) const
{
    return (m_eType == eToCheck);
}


tCIDLib::TBoolean TGenProtoFldVal::bSetFrom(const TGenProtoExprVal& evalSrc)
{
    tCIDLib::TBoolean bChangedValue = kCIDLib::False;

    //
    //  According to the type of the field, get the expression value as that
    //  type and store it on the target field.
    //
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            const tCIDLib::TBoolean bNew = evalSrc.bValue();
            if (m_uValue.bValue != bNew)
            {
                bChangedValue = kCIDLib::True;
                m_uValue.bValue = bNew;
            }
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            const tCIDLib::TCard4 c4New = evalSrc.c4Value();
            if (m_uValue.c4Value != c4New)
            {
                bChangedValue = kCIDLib::True;
                m_uValue.c4Value = c4New;
            }
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            const tCIDLib::TInt4 i4New = evalSrc.i4Value();
            if (m_uValue.i4Value != i4New)
            {
                bChangedValue = kCIDLib::True;
                m_uValue.i4Value = i4New;
            }
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            const tCIDLib::TFloat8 f8New = evalSrc.f8Value();
            if (m_uValue.f8Value != f8New)
            {
                bChangedValue = kCIDLib::True;
                m_uValue.f8Value = f8New;
            }
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            if (*m_pstrValue != evalSrc.strValue())
            {
                bChangedValue = kCIDLib::True;
                *m_pstrValue = evalSrc.strValue();
            }
            break;
        }

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_eType))
                , TString(L"tCQCKit::EFldTypes")
            );
            #endif
            break;
    }
    return bChangedValue;
}


tCIDLib::TBoolean TGenProtoFldVal::bValue() const
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Boolean);
    return m_uValue.bValue;
}

tCIDLib::TBoolean TGenProtoFldVal::bValue(const tCIDLib::TBoolean bToSet)
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Boolean);
    m_uValue.bValue = bToSet;
    return m_uValue.bValue;
}


tCIDLib::TCard4 TGenProtoFldVal::c4Value() const
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Card);
    return m_uValue.c4Value;
}

tCIDLib::TCard4 TGenProtoFldVal::c4Value(const tCIDLib::TCard4 c4ToSet)
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Card);
    m_uValue.c4Value = c4ToSet;
    return m_uValue.c4Value;
}


tCIDLib::TFloat8 TGenProtoFldVal::f8Value() const
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Float);
    return m_uValue.f8Value;
}

tCIDLib::TFloat8 TGenProtoFldVal::f8Value(const tCIDLib::TFloat8 f8ToSet)
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Float);
    m_uValue.f8Value = f8ToSet;
    return m_uValue.f8Value;
}


tCIDLib::TInt4 TGenProtoFldVal::i4Value() const
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Int);
    return m_uValue.i4Value;
}

tCIDLib::TInt4 TGenProtoFldVal::i4Value(const tCIDLib::TInt4 i4ToSet)
{
    // Make sure our value is of the right type
    ValidateType(tCQCKit::EFldTypes::Int);
    m_uValue.i4Value = i4ToSet;
    return m_uValue.i4Value;
}


const TString& TGenProtoFldVal::strValue() const
{
    //
    //  Fault the string field in, which we'll have to do if we aren't a
    //  string type naturally.
    //
    if (!m_pstrValue)
        m_pstrValue = new TString;

    // And, if not a string type, format our binary value to the string field
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
            if (m_uValue.bValue)
                *m_pstrValue = kCQCKit::pszFld_True;
            else
                *m_pstrValue = kCQCKit::pszFld_False;
            break;

        case tCQCKit::EFldTypes::Card :
            m_pstrValue->SetFormatted(m_uValue.c4Value);
            break;

        case tCQCKit::EFldTypes::Float :
            m_pstrValue->SetFormatted(m_uValue.f8Value);
            break;

        case tCQCKit::EFldTypes::Int :
            m_pstrValue->SetFormatted(m_uValue.i4Value);
            break;

        case tCQCKit::EFldTypes::String :
            // Do nothing. Its already in the correct format
            break;

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_eType))
                , TString(L"tCQCKit::EFldTypes")
            );
            #endif
            break;
    };

    return *m_pstrValue;
}

const TString& TGenProtoFldVal::strValue(const TString& strToSet)
{
    tCIDLib::TBoolean bOk = kCIDLib::True;
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
        {
            if (strToSet == kCQCKit::pszFld_False)
                m_uValue.bValue = kCIDLib::False;
            else if (strToSet == kCQCKit::pszFld_True)
                m_uValue.bValue = kCIDLib::True;
            else
                bOk = kCIDLib::False;
            break;
        }

        case tCQCKit::EFldTypes::Card :
        {
            const tCIDLib::TCard4 c4Val = TRawStr::c4AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );
            m_uValue.c4Value = c4Val;
            break;
        }

        case tCQCKit::EFldTypes::Float :
        {
            const tCIDLib::TFloat8 f8Val = TRawStr::f8AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );
            m_uValue.f8Value = f8Val;
            break;
        }

        case tCQCKit::EFldTypes::Int :
        {
            const tCIDLib::TInt4 i4Val = TRawStr::i4AsBinary
            (
                strToSet.pszBuffer()
                , bOk
            );
            m_uValue.i4Value = i4Val;
            break;
        }

        case tCQCKit::EFldTypes::String :
            // Just a straight assignment in this case
            *m_pstrValue = strToSet;
            break;

        default :
            BadCast(tCQCKit::EFldTypes::String, m_eType);
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
            , tCQCKit::strXlatEFldTypes(m_eType)
        );
    }

    return *m_pstrValue;
}


tCIDLib::TVoid TGenProtoFldVal::StreamValue(TBinOutStream& strmTarget)
{
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
            strmTarget << m_uValue.bValue;
            break;

        case tCQCKit::EFldTypes::Card :
            strmTarget << m_uValue.c4Value;
            break;

        case tCQCKit::EFldTypes::Float :
            strmTarget << m_uValue.f8Value;
            break;

        case tCQCKit::EFldTypes::Int :
            strmTarget << m_uValue.i4Value;
            break;

        case tCQCKit::EFldTypes::String :
            strmTarget << *m_pstrValue;
            break;

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_eType))
                , TString(L"tCQCKit::EFldTypes")
            );
            #endif
            break;
    };
}


// ---------------------------------------------------------------------------
//  TCQCSerFldFal: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoFldVal::FormatTo(TTextOutStream& strmTarget) const
{
    switch(m_eType)
    {
        case tCQCKit::EFldTypes::Boolean :
            strmTarget << m_uValue.bValue;
            break;

        case tCQCKit::EFldTypes::Card :
            strmTarget << m_uValue.c4Value;
            break;

        case tCQCKit::EFldTypes::Float :
            strmTarget << m_uValue.f8Value;
            break;

        case tCQCKit::EFldTypes::Int :
            strmTarget << m_uValue.i4Value;
            break;

        case tCQCKit::EFldTypes::String :
            strmTarget << *m_pstrValue;
            break;

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(m_eType))
                , TString(L"tCQCKit::EFldTypes")
            );
            #endif
            break;
    };
}


// ---------------------------------------------------------------------------
//  TGenProtoFldVal: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoFldVal::BadCast(const  tCQCKit::EFldTypes  eSrcType
                        , const tCQCKit::EFldTypes  eDestType)
{
    facGenProtoS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kGPDErrs::errcRunT_BadCast
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::TypeMatch
        , tCQCKit::strXlatEFldTypes(eSrcType)
        , tCQCKit::strXlatEFldTypes(eDestType)
    );
}


tCIDLib::TVoid
TGenProtoFldVal::ValidateType(const tCQCKit::EFldTypes eToCheck) const
{
    if (m_eType != eToCheck)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_WrongFldType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , tCQCKit::strXlatEFldTypes(m_eType)
            , tCQCKit::strXlatEFldTypes(eToCheck)
        );
    }
}



