//
// FILE NAME: CQCKit_FldLimits.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2003
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
//  This is the implementation file for the field limits classes.
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
RTTIDecls(TCQCFldLimit,TObject)
RTTIDecls(TCQCFldBoolLimit,TCQCFldLimit)
RTTIDecls(TCQCFldCardLimit,TCQCFldLimit)
RTTIDecls(TCQCFldFloatLimit,TCQCFldLimit)
RTTIDecls(TCQCFldIntLimit,TCQCFldLimit)
RTTIDecls(TCQCFldStrLimit,TCQCFldLimit)
RTTIDecls(TCQCFldCRangeLimit,TCQCFldCardLimit)
RTTIDecls(TCQCFldFRangeLimit,TCQCFldFloatLimit)
RTTIDecls(TCQCFldIRangeLimit,TCQCFldIntLimit)
RTTIDecls(TCQCFldEnumLimit,TCQCFldStrLimit)
RTTIDecls(TCQCFldMediaImgLimit,TCQCFldStrLimit)
RTTIDecls(TCQCFldRegExLimit,TCQCFldStrLimit)
RTTIDecls(TCQCFldStrListLimit,TCQCFldStrLimit)
RTTIDecls(TCQCFldTimeLimit,TCQCFldStrLimit)



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldLimit: Public, static methods
// ---------------------------------------------------------------------------

//
//  A helper that will take a limit string and parse it out
TCQCFldLimit*
TCQCFldLimit::pfldlMakeNew( const   TString&            strName
                            , const TString&            strLim
                            , const tCQCKit::EFldTypes  eType)
{
    // Start tokenizing the limit string
    TStringTokenizer stokLim(&strLim, L": ");
    TString strType;
    stokLim.bGetNextToken(strType);

    TCQCFldLimit* pfldlRet = 0;
    switch(eType)
    {
        case tCQCKit::EFldTypes::Card :
        case tCQCKit::EFldTypes::Float :
        case tCQCKit::EFldTypes::Int :
        {
            //
            //  All of these wil use a range object. If the limit string is
            //  empty, then we'll get the full range by default, i.e. no
            //  limits.
            //
            if (strLim.bIsEmpty())
            {
                if (eType == tCQCKit::EFldTypes::Card)
                    pfldlRet = new TCQCFldCardLimit;
                else if (eType == tCQCKit::EFldTypes::Float)
                    pfldlRet = new TCQCFldFloatLimit;
                else if (eType == tCQCKit::EFldTypes::Int)
                    pfldlRet = new TCQCFldIntLimit;
                else
                    CantTakeLimitType(strName, strLim);
            }
             else if (strType.bCompareI(L"Range")
                  ||  strType.bCompareI(L"GtThan")
                  ||  strType.bCompareI(L"LsThan"))
            {
                if (eType == tCQCKit::EFldTypes::Card)
                    pfldlRet = new TCQCFldCRangeLimit;
                else if (eType == tCQCKit::EFldTypes::Float)
                    pfldlRet = new TCQCFldFRangeLimit;
                else if (eType == tCQCKit::EFldTypes::Int)
                    pfldlRet = new TCQCFldIRangeLimit;
                else
                    CantTakeLimitType(strName, strLim);
            }
             else
            {
                CantTakeLimitType(strName, strLim);
            }
            break;
        }

        case tCQCKit::EFldTypes::String :
        {
            if (strLim.bIsEmpty())
                pfldlRet = new TCQCFldStrLimit;
            else if (strType.bCompareI(L"Enum"))
                pfldlRet = new TCQCFldEnumLimit;
            else if (strType.bCompareI(L"RegEx"))
                pfldlRet = new TCQCFldRegExLimit;
            else if (strType.bCompareI(L"MediaImg"))
                pfldlRet = new TCQCFldMediaImgLimit;
            else
                CantTakeLimitType(strName, strLim);
            break;
        }

        case tCQCKit::EFldTypes::Boolean :
        {
            pfldlRet = new TCQCFldBoolLimit;
            break;
        }

        case tCQCKit::EFldTypes::Time :
        {
            pfldlRet = new TCQCFldTimeLimit;
            break;
        }

        case tCQCKit::EFldTypes::StringList :
        {
            pfldlRet = new TCQCFldStrListLimit;
            break;
        }

        default :
        {
            #if CID_DEBUG_ON
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcFld_UnknownType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::i4EnumOrd(eType))
            );
            #endif
        }
    };

    // Try to parse the limits
    TJanitor<TCQCFldLimit> janLimits(pfldlRet);
    if (!pfldlRet->bParseLimits(strLim))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcFld_BadLimitValue
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strName
        );
    }

    // We made it, so return the new object, if any
    return janLimits.pobjOrphan();
}


TCQCFldLimit* TCQCFldLimit::pfldlMakeNew(const TCQCFldDef& flddSrc)
{
    // Just call the other version
    return pfldlMakeNew(flddSrc.strName(), flddSrc.strLimits(), flddSrc.eType());
}


// ---------------------------------------------------------------------------
//  TCQCFldLimit: Destructor
// ---------------------------------------------------------------------------
TCQCFldLimit::~TCQCFldLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldLimit: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Returns the next or previous value generically. Those limit types for which this
//  makes sense override and return the appropriate value. Others do nothing and we
//  return false to say that we are not returning a new value (so the current value is
//  still the active one.)
//
tCIDLib::TBoolean
TCQCFldLimit::bNextPrevVal( const   TString&            strCurVal
                            ,       TString&            strNewVal
                            , const tCIDLib::TBoolean   bNext
                            , const tCIDLib::TBoolean   bWrapOK) const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCFldLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // For those with no limits, just say they are the same
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCFldLimit::bValidate(const TString&)
{
    // For those with no limits, this default just says it's good
    return kCIDLib::True;
}


tCIDLib::TCard4 TCQCFldLimit::c4QueryValues(TVector<TString>&) const
{
    // Default is to return no values if not overridden
    return 0;
}


tCIDLib::TVoid TCQCFldLimit::FormatToText(TString& strToFill)
{
    // If not overridden, we reutrn an empty string
    strToFill.Clear();
}


// ---------------------------------------------------------------------------
//  TCQCFldLimit: Hidden constructors
// ---------------------------------------------------------------------------
TCQCFldLimit::TCQCFldLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldLimit: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldLimit::bBreakOutLimits(  const   TString&    strLimits
                                , const TString&    strExpected)
{
    // Set up a tokenizer for the limits
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which is all that's expected in this version
    if (!stokLimits.bGetNextToken(strTmp))
        return kCIDLib::False;

    // Make sure it matches the expected prefix, and no more token
    if ((strTmp != strExpected) || stokLimits.bMoreTokens())
        return kCIDLib::False;
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCFldLimit::bBreakOutLimits(  const   TString&    strLimits
                                , const TString&    strExpected
                                ,       TString&    strParm1)
{
    // Set up a tokenizer for the limits
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which is all that's expected in this versino
    if (!stokLimits.bGetNextToken(strTmp))
        return kCIDLib::False;

    // Make sure it matches the expected prefix
    strTmp.StripWhitespace();
    if (strTmp != strExpected)
        return kCIDLib::False;

    // Update the whitespace
    stokLimits.strWhitespace(L",");

    // Get the one parm requested, then there shouldn't be any more
    if (!stokLimits.bGetNextToken(strParm1) || stokLimits.bMoreTokens())
    {
        strParm1.StripWhitespace();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TCQCFldLimit::bBreakOutLimits(  const   TString&    strLimits
                                , const TString&    strExpected
                                ,       TString&    strParm1
                                ,       TString&    strParm2)
{
    // Set up a tokenizer for the limits
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which is all that's expected in this versino
    if (!stokLimits.bGetNextToken(strTmp))
        return kCIDLib::False;

    // Make sure it matches the expected prefix
    strTmp.StripWhitespace();
    if (strTmp != strExpected)
        return kCIDLib::False;

    // Update the whitespace
    stokLimits.strWhitespace(L",");

    // Get the two parms requested and there should be no more
    if (!stokLimits.bGetNextToken(strParm1)
    ||  !stokLimits.bGetNextToken(strParm2)
    ||  stokLimits.bMoreTokens())
    {
        strParm1.StripWhitespace();
        strParm2.StripWhitespace();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TCard2
TCQCFldLimit::c2CheckFmtVersion(const   tCIDLib::TCard2 c2Expected
                                ,       TBinInStream&   strmSrc)
{
    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (c2FmtVersion != c2Expected)
    {
        // We are throwing a CIDLib error here!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2Expected)
            , clsIsA()
        );
    }
    return c2FmtVersion;
}


tCIDLib::TVoid TCQCFldLimit::ValidateTypes(const TClass& clsSrc) const
{
    if (clsIsA() != clsSrc)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDbg_NotSameTypes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , clsIsA()
            , clsSrc
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCFldLimit: Private, static methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCFldLimit::CantTakeLimitType(const TString& strName, const TString& strLimits)
{
    facCQCKit().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kKitErrs::errcFld_WrongLimitType
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Internal
        , strName
        , strLimits
    );
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldBoolLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldBoolLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldBoolLimit::TCQCFldBoolLimit()
{
}

TCQCFldBoolLimit::~TCQCFldBoolLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldBoolLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldBoolLimit::bParseLimits(const TString& strLimits)
{
    return strLimits.bIsEmpty();
}


tCIDLib::TBoolean TCQCFldBoolLimit::bValidate(const TString& strToValidate)
{
    tCIDLib::TBoolean bTmp;
    return (eValidate(strToValidate, bTmp) == tCQCKit::EValResults::OK);
}


tCIDLib::TCard4 TCQCFldBoolLimit::c4QueryValues(TVector<TString>& colToFill) const
{
    colToFill.objAdd(kCQCKit::pszFld_False);
    colToFill.objAdd(kCQCKit::pszFld_True);
    return 2;
}


tCQCKit::EOptFldReps TCQCFldBoolLimit::eOptimalRep() const
{
    // A check box is the best for us
    return tCQCKit::EOptFldReps::Check;
}


tCIDLib::TVoid TCQCFldBoolLimit::QueryDefVal(TString& strToFill)
{
    strToFill = L"False";
}


// ---------------------------------------------------------------------------
//  TCQCFldBoolLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// Don't change the out parm if the value isn't valid!
tCQCKit::EValResults
TCQCFldBoolLimit::eValidate(const   TString&            strToValidate
                            ,       tCIDLib::TBoolean&  bToFill) const
{
    // Try to convert it to the needed formta
    if ((strToValidate == L"0") || strToValidate.bCompareI(L"false"))
        bToFill = kCIDLib::False;
    else if ((strToValidate == L"1") || strToValidate.bCompareI(L"true"))
        bToFill = kCIDLib::True;
    else
        return tCQCKit::EValResults::Unconvertable;

    return tCQCKit::EValResults::OK;
}


tCQCKit::EValResults TCQCFldBoolLimit::eValidate(const tCIDLib::TBoolean) const
{
    // For this base class, just say it's all good
    return tCQCKit::EValResults::OK;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCardLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldCardLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldCardLimit::TCQCFldCardLimit() :

    m_c4Max(kCIDLib::c4MaxCard)
    , m_c4Min(0)
{
}

TCQCFldCardLimit::~TCQCFldCardLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldCardLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldCardLimit::bNextPrevVal( const   TString&            strCurVal
                                ,       TString&            strNewVal
                                , const tCIDLib::TBoolean   bNext
                                , const tCIDLib::TBoolean   bWrapOK) const
{
    // Convert the value to card
    const tCIDLib::TCard4 c4Org = strCurVal.c4Val();
    tCIDLib::TCard4 c4New = c4Org;

    // And move it up or down if we can
    if (bNext)
    {
        if (c4New < m_c4Max)
            c4New++;
        else if (bWrapOK)
            c4New = m_c4Min;
    }
     else if (!bNext)
    {
        if (c4New > m_c4Min)
            c4New--;
        else
            c4New = m_c4Max;
    }

    if (c4New != c4Org)
    {
        strNewVal.SetFormatted(c4New);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCFldCardLimit::bParseLimits(const TString& strLimits)
{
    // Reset our limits, which are always the max
    m_c4Min = 0;
    m_c4Max = kCIDLib::c4MaxCard;

    // Must be empty since we support no limits
    return strLimits.bIsEmpty();
}


tCIDLib::TBoolean TCQCFldCardLimit::bValidate(const TString& strToValidate)
{
    tCIDLib::TCard4 c4Tmp;
    return (eValidate(strToValidate, c4Tmp) == tCQCKit::EValResults::OK);
}


tCIDLib::TCard4 TCQCFldCardLimit::c4QueryValues(TVector<TString>& colToFill) const
{
    TString strValue;
    for (tCIDLib::TCard4 c4Index = m_c4Min; c4Index <= m_c4Max; c4Index++)
    {
        strValue.SetFormatted(c4Index);
        colToFill.objAdd(strValue);
    }
    return colToFill.c4ElemCount();
}


tCQCKit::EOptFldReps TCQCFldCardLimit::eOptimalRep() const
{
    //
    //  If the range is full, then just a text entry field. If 16 or less,
    //  then a spin, else if 256 or less a slider.
    //
    const tCIDLib::TCard8 c8Range = tCIDLib::TCard8(m_c4Max - m_c4Min) + 1;
    if (c8Range <= 16)
        return tCQCKit::EOptFldReps::Spin;
    else if (c8Range <= 256)
        return tCQCKit::EOptFldReps::Slider;

    return tCQCKit::EOptFldReps::Text;
}


tCIDLib::TVoid TCQCFldCardLimit::QueryDefVal(TString& strToFill)
{
    // Take the min value
    strToFill.SetFormatted(m_c4Min);
}


// ---------------------------------------------------------------------------
//  TCQCFldCardLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// We don't modify the out parm unless the value is storable!
tCQCKit::EValResults
TCQCFldCardLimit::eValidate(const   TString&            strToValidate
                            ,       tCIDLib::TCard4&    c4ToFill) const
{
    // Try to convert it to the needed format
    tCIDLib::TCard4 c4Tmp;
    if (!strToValidate.bToCard4(c4Tmp))
        return tCQCKit::EValResults::Unconvertable;

    // Make sure it meets any range limits
    if ((c4Tmp < m_c4Min) || (c4Tmp > m_c4Max))
        return tCQCKit::EValResults::LimitViolation;

    // Looks ok so give it back
    c4ToFill = c4Tmp;
    return tCQCKit::EValResults::OK;
}


tCQCKit::EValResults
TCQCFldCardLimit::eValidate(const tCIDLib::TCard4 c4ToVal) const
{
    if ((c4ToVal < m_c4Min) || (c4ToVal > m_c4Max))
        return tCQCKit::EValResults::LimitViolation;
    return tCQCKit::EValResults::OK;
}



// ---------------------------------------------------------------------------
//  TCQCFldCardLimit: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCFldCardLimit::TCQCFldCardLimit( const   tCIDLib::TCard4 c4Min
                                    , const tCIDLib::TCard4 c4Max) :
    m_c4Max(c4Max)
    , m_c4Min(c4Min)
{
}


// ---------------------------------------------------------------------------
//  TCQCFldCardLimit: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldCardLimit::SetRange(  const   tCIDLib::TCard4 c4Min
                                            , const tCIDLib::TCard4 c4Max)
{
    m_c4Max = c4Max;
    m_c4Min = c4Min;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFloatLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFloatLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFloatLimit::TCQCFldFloatLimit() :

    m_f8Max(kCIDLib::f8MaxFloat)
    , m_f8Min(-(kCIDLib::f8MaxFloat - 1))
{
}

TCQCFldFloatLimit::~TCQCFldFloatLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFloatLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldFloatLimit::bParseLimits(const TString& strLimits)
{
    // Reset our limits, which are always the max
    m_f8Max = kCIDLib::f8MaxFloat;
    m_f8Min = -(kCIDLib::f8MaxFloat - 1);

    // Must be empty since we support no limits
    return strLimits.bIsEmpty();
}


tCIDLib::TBoolean TCQCFldFloatLimit::bValidate(const TString& strToValidate)
{
    tCIDLib::TFloat8 f8Tmp;
    return (eValidate(strToValidate, f8Tmp) == tCQCKit::EValResults::OK);
}


tCQCKit::EOptFldReps TCQCFldFloatLimit::eOptimalRep() const
{
    // For floating point, always just a text field
    return tCQCKit::EOptFldReps::Text;
}


tCIDLib::TVoid TCQCFldFloatLimit::QueryDefVal(TString& strToFill)
{
    // If zero is in the range, then take that, else take the one closest to it
    if ((m_f8Min <= 0.0) && (m_f8Max >= 0.0))
        strToFill.SetFormatted(tCIDLib::TFloat8(0));
    else if (m_f8Min > 0.0)
        strToFill.SetFormatted(m_f8Min);
    else
        strToFill.SetFormatted(m_f8Max);
}


// ---------------------------------------------------------------------------
//  TCQCFldFloatLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// We don't chagne the out parm unless the value is storable!
tCQCKit::EValResults
TCQCFldFloatLimit::eValidate(const  TString&            strToValidate
                            ,       tCIDLib::TFloat8&   f8ToFill) const
{
    // Try to convert it to the needed formta
    tCIDLib::TFloat8 f8Tmp;
    if (!strToValidate.bToFloat8(f8Tmp))
        return tCQCKit::EValResults::Unconvertable;

    // Make sure it's within any range limits
    if ((f8Tmp < m_f8Min) || (f8Tmp > m_f8Max))
        return tCQCKit::EValResults::LimitViolation;

    // Looks ok so give it back
    f8ToFill = f8Tmp;
    return tCQCKit::EValResults::OK;
}


tCQCKit::EValResults
TCQCFldFloatLimit::eValidate(const tCIDLib::TFloat8& f8ToVal) const
{
    if ((f8ToVal < m_f8Min) || (f8ToVal > m_f8Max))
        return tCQCKit::EValResults::LimitViolation;
    return tCQCKit::EValResults::OK;
}


// ---------------------------------------------------------------------------
//  TCQCFldFloatLimit: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCFldFloatLimit::TCQCFldFloatLimit(const  tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max) :
    m_f8Max(f8Max)
    , m_f8Min(f8Min)
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFloatLimit: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldFloatLimit::SetRange( const   tCIDLib::TFloat8 f8Min
                                            , const tCIDLib::TFloat8 f8Max)
{
    m_f8Max = f8Max;
    m_f8Min = f8Min;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldIntLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldIntLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldIntLimit::TCQCFldIntLimit() :

    m_i4Max(kCIDLib::i4MaxInt)
    , m_i4Min(kCIDLib::i4MinInt)
{
}

TCQCFldIntLimit::~TCQCFldIntLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldIntLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCFldIntLimit::bNextPrevVal(  const   TString&            strCurVal
                                ,       TString&            strNewVal
                                , const tCIDLib::TBoolean   bNext
                                , const tCIDLib::TBoolean   bWrapOK) const
{
    // Convert the value to card
    const tCIDLib::TInt4 i4Org = strCurVal.i4Val();
    tCIDLib::TInt4 i4New = i4Org;

    // And move it up or down if we can
    if (bNext)
    {
        if (i4New < m_i4Max)
            i4New++;
        else if (bWrapOK)
            i4New = m_i4Min;
    }
     else if (!bNext)
    {
        if (i4New > m_i4Min)
            i4New--;
        else if (bWrapOK)
            i4New = m_i4Max;
    }

    if (i4New != i4Org)
    {
        strNewVal.SetFormatted(i4New);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TCQCFldIntLimit::bParseLimits(const TString& strLimits)
{
    // Reset our limits, which are always the max
    m_i4Max = kCIDLib::i4MaxInt;
    m_i4Min = kCIDLib::i4MinInt;

    // Must be empty since we support no limits
    return strLimits.bIsEmpty();
}


tCIDLib::TBoolean TCQCFldIntLimit::bValidate(const TString& strToValidate)
{
    tCIDLib::TInt4 i4Tmp;
    return (eValidate(strToValidate, i4Tmp) == tCQCKit::EValResults::OK);
}


tCIDLib::TCard4 TCQCFldIntLimit::c4QueryValues(TVector<TString>& colToFill) const
{
    TString strValue;
    for (tCIDLib::TInt4 i4Index = m_i4Min; i4Index <= m_i4Max; i4Index++)
    {
        strValue.SetFormatted(i4Index);
        colToFill.objAdd(strValue);
    }
    return colToFill.c4ElemCount();
}


tCQCKit::EOptFldReps TCQCFldIntLimit::eOptimalRep() const
{
    //
    //  If the range is full, then just a text entry field. If 16 or less,
    //  then a spin, else if 256 or less a slider.
    //
    const tCIDLib::TInt8 i8Range = tCIDLib::TInt8(m_i4Max)
                                   - tCIDLib::TInt8(m_i4Min);
    if (i8Range <= 16)
        return tCQCKit::EOptFldReps::Spin;
    else if (i8Range <= 256)
        return tCQCKit::EOptFldReps::Slider;
    return tCQCKit::EOptFldReps::Text;
}


tCIDLib::TVoid TCQCFldIntLimit::QueryDefVal(TString& strToFill)
{
    // If zero is in the range, then take that, else take the one closest to it
    if ((m_i4Min <= 0) && (m_i4Max >= 0))
        strToFill.SetFormatted(tCIDLib::TInt4(0));
    else if (m_i4Min > 0)
        strToFill.SetFormatted(m_i4Min);
    else
        strToFill.SetFormatted(m_i4Max);
}



// ---------------------------------------------------------------------------
//  TCQCFldIntLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// We don't change the out parm unlss the value is storable!
tCQCKit::EValResults
TCQCFldIntLimit::eValidate( const   TString&        strToValidate
                            ,       tCIDLib::TInt4& i4ToFill) const
{
    // Try to convert it to the needed format
    tCIDLib::TInt4 i4Tmp;
    if (!strToValidate.bToInt4(i4Tmp))
        return tCQCKit::EValResults::Unconvertable;

    // Make sure it meets any range limits
    if ((i4Tmp < m_i4Min) || (i4Tmp > m_i4Max))
        return tCQCKit::EValResults::LimitViolation;

    // Looks ok so store it
    i4ToFill = i4Tmp;
    return tCQCKit::EValResults::OK;
}

tCQCKit::EValResults
TCQCFldIntLimit::eValidate(const tCIDLib::TInt4 i4ToVal) const
{
    if ((i4ToVal < m_i4Min) || (i4ToVal > m_i4Max))
        return tCQCKit::EValResults::LimitViolation;
    return tCQCKit::EValResults::OK;
}



// ---------------------------------------------------------------------------
//  TCQCFldIntLimit: Hidden Constructors
// ---------------------------------------------------------------------------
TCQCFldIntLimit::TCQCFldIntLimit(const  tCIDLib::TInt4    i4Min
                                , const tCIDLib::TInt4    i4Max) :
    m_i4Max(i4Max)
    , m_i4Min(i4Min)
{
}


// ---------------------------------------------------------------------------
//  TCQCFldIntLimit: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFldIntLimit::SetRange(const  tCIDLib::TInt4    i4Min
                                        , const tCIDLib::TInt4    i4Max)
{
    m_i4Max = i4Max;
    m_i4Min = i4Min;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStrLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStrLimit: Destructor
// ---------------------------------------------------------------------------
TCQCFldStrLimit::TCQCFldStrLimit()
{
}

TCQCFldStrLimit::~TCQCFldStrLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldStrLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldStrLimit::bParseLimits(const TString& strLimits)
{
    return strLimits.bIsEmpty();
}


tCQCKit::EOptFldReps TCQCFldStrLimit::eOptimalRep() const
{
    // With no limits, text form, just a play text entry field is best
    return tCQCKit::EOptFldReps::Text;
}


tCIDLib::TVoid TCQCFldStrLimit::QueryDefVal(TString& strToFill)
{
    strToFill.Clear();
}


// ---------------------------------------------------------------------------
//  TCQCFldStrLimit: Public, virtual methods
// ---------------------------------------------------------------------------
tCQCKit::EValResults TCQCFldStrLimit::eValidate(const TString&) const
{
    // If not overridden, say it's good
    return tCQCKit::EValResults::OK;
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCFldStrListLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldStrListLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldStrListLimit::TCQCFldStrListLimit()
{
}

TCQCFldStrListLimit::~TCQCFldStrListLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldStrListLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldStrListLimit::bParseLimits(const TString& strLimits)
{
    return strLimits.bIsEmpty();
}


tCQCKit::EOptFldReps TCQCFldStrListLimit::eOptimalRep() const
{
    //
    //  We need to have a value entered as text, since they have to enter the
    //  separate lines for the values.
    //
    return tCQCKit::EOptFldReps::Text;
}


tCIDLib::TVoid TCQCFldStrListLimit::QueryDefVal(TString& strToFill)
{
    strToFill.Clear();
}


// ---------------------------------------------------------------------------
//  TCQCFldStrListLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// Default action is to parse out the values and if formatted ok, say its ok
tCQCKit::EValResults
TCQCFldStrListLimit::eValidate( const   TString&            strToValidate
                                ,       TVector<TString>&   colToFill) const
{
    //
    //  It's in the form of a quoted comma list so we can use a helper to parse
    //  it.
    //
    tCIDLib::TCard4 c4ErrInd;
    if (!TStringTokenizer::bParseQuotedCommaList(strToValidate, colToFill, c4ErrInd))
        return tCQCKit::EValResults::Unconvertable;
    return tCQCKit::EValResults::OK;
}


tCQCKit::EValResults
TCQCFldStrListLimit::eValidate(const TVector<TString>& strNew) const
{
    return tCQCKit::EValResults::OK;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldTimeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldTimeLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldTimeLimit::TCQCFldTimeLimit()
{
}

TCQCFldTimeLimit::~TCQCFldTimeLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldTimeLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldTimeLimit::bParseLimits(const TString& strLimits)
{
    return strLimits.bIsEmpty();
}


tCQCKit::EOptFldReps TCQCFldTimeLimit::eOptimalRep() const
{
    //
    //  Indicate time, which will be used to provide some appropriate time
    //  entry scheme.
    //
    return tCQCKit::EOptFldReps::Time;
}


tCIDLib::TVoid TCQCFldTimeLimit::QueryDefVal(TString& strToFill)
{
    // Just return zero, the base time
    strToFill = L"0";
}


// ---------------------------------------------------------------------------
//  TCQCFldTimeLimit: Public, virtual methods
// ---------------------------------------------------------------------------

// We don't modify the output parm unless the value is valid!
tCQCKit::EValResults
TCQCFldTimeLimit::eValidate(const   TString&            strToValidate
                            ,       tCIDLib::TCard8&    c8ToFill) const
{
    //
    //  The value should be the hex foramtted encoded time value, so we should
    //  be able to convert it to binary.
    //
    tCIDLib::TCard8 c8Tmp;
    if (!strToValidate.bToCard8(c8Tmp, tCIDLib::ERadices::Hex))
        return tCQCKit::EValResults::Unconvertable;

    // Looks ok, so give it back
    c8ToFill = c8Tmp;
    return tCQCKit::EValResults::OK;
}

tCQCKit::EValResults TCQCFldTimeLimit::eValidate(const tCIDLib::TCard8&) const
{
    // Default is to say it's fine
    return tCQCKit::EValResults::OK;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldCRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldCRangeLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldCRangeLimit::TCQCFldCRangeLimit()
{
}

TCQCFldCRangeLimit::TCQCFldCRangeLimit( const   tCIDLib::TCard4 c4Min
                                        , const tCIDLib::TCard4 c4Max) :
    TCQCFldCardLimit(c4Min, c4Max)
{
}

TCQCFldCRangeLimit::~TCQCFldCRangeLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldCRangeLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldCRangeLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldCRangeLimit& fldlReal = static_cast<const TCQCFldCRangeLimit&>(fldlSrc);

    return (c4Min() == fldlReal.c4Min()) && (c4Max() == fldlReal.c4Max());
}


tCIDLib::TBoolean TCQCFldCRangeLimit::bParseLimits(const TString& strLimits)
{
    // We expect two parms, the min/max values
    tCIDLib::TBoolean   bValid1;
    tCIDLib::TBoolean   bValid2;
    tCIDLib::TCard4     c4Max = 0;
    tCIDLib::TCard4     c4Min = 0;
    TString             strParm1;
    TString             strParm2;

    if (bBreakOutLimits(strLimits, L"Range", strParm1, strParm2))
    {
        c4Min = TRawStr::c4AsBinary(strParm1.pszBuffer(), bValid1);
        c4Max = TRawStr::c4AsBinary(strParm2.pszBuffer(), bValid2);
        if (!bValid1 || !bValid2)
            return kCIDLib::False;

        // And they cannot be inverted, or equal. If not, then it's good
        if (c4Min >= c4Max)
            return kCIDLib::False;
    }
     else if (bBreakOutLimits(strLimits, L"GtThan", strParm1))
    {
        c4Min = TRawStr::c4AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        c4Max = kCIDLib::c4MaxCard;
    }
     else if (bBreakOutLimits(strLimits, L"LsThan", strParm1))
    {
        c4Max = TRawStr::c4AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        c4Min = 0;
    }
     else
    {
        return kCIDLib::False;
    }

    SetRange(c4Min, c4Max);
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCFldCRangeLimit::FormatToText(TString& strToFill)
{
    strToFill = L"Range: ";
    strToFill.AppendFormatted(c4Min());
    strToFill.Append(kCIDLib::chComma);
    strToFill.AppendFormatted(c4Max());
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldFRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldFRangeLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldFRangeLimit::TCQCFldFRangeLimit()
{
}

TCQCFldFRangeLimit::TCQCFldFRangeLimit( const   tCIDLib::TFloat8    f8Min
                                        , const tCIDLib::TFloat8    f8Max) :
    TCQCFldFloatLimit(f8Min, f8Max)
{
}

TCQCFldFRangeLimit::~TCQCFldFRangeLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldFRangeLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldFRangeLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldFRangeLimit& fldlReal = static_cast<const TCQCFldFRangeLimit&>(fldlSrc);

    return (f8Min() == fldlReal.f8Min()) && (f8Max() == fldlReal.f8Max());
}


tCIDLib::TBoolean TCQCFldFRangeLimit::bParseLimits(const TString& strLimits)
{
    // We expect two parms, the min/max values
    tCIDLib::TBoolean   bValid1;
    tCIDLib::TBoolean   bValid2;
    tCIDLib::TFloat8    f8Max = 0;
    tCIDLib::TFloat8    f8Min = 0;
    TString             strParm1;
    TString             strParm2;

    if (bBreakOutLimits(strLimits, L"Range", strParm1, strParm2))
    {
        f8Min = TRawStr::f8AsBinary(strParm1.pszBuffer(), bValid1);
        f8Max = TRawStr::f8AsBinary(strParm2.pszBuffer(), bValid2);
        if (!bValid1 || !bValid2)
            return kCIDLib::False;

        // And they cannot be inverted, or equal. If not, then it's good
        if (f8Min >= f8Max)
            return kCIDLib::False;
    }
     else if (bBreakOutLimits(strLimits, L"GtThan", strParm1))
    {
        f8Min = TRawStr::f8AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        f8Max = kCIDLib::f8MaxFloat;
    }
     else if (bBreakOutLimits(strLimits, L"LsThan", strParm1))
    {
        f8Max = TRawStr::f8AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        f8Min = -kCIDLib::f8MaxFloat;
    }
     else
    {
        return kCIDLib::False;
    }

    SetRange(f8Min, f8Max);
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCFldFRangeLimit::FormatToText(TString& strToFill)
{
    strToFill = L"Range: ";
    strToFill.AppendFormatted(f8Min());
    strToFill.Append(kCIDLib::chComma);
    strToFill.AppendFormatted(f8Max());
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldIRangeLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldIRangeLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldIRangeLimit::TCQCFldIRangeLimit()
{
}

TCQCFldIRangeLimit::TCQCFldIRangeLimit( const   tCIDLib::TInt4    i4Min
                                        , const tCIDLib::TInt4    i4Max) :
    TCQCFldIntLimit(i4Min, i4Max)
{
}

TCQCFldIRangeLimit::~TCQCFldIRangeLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldIRangeLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldIRangeLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldIRangeLimit& fldlReal = static_cast<const TCQCFldIRangeLimit&>(fldlSrc);

    return (i4Min() == fldlReal.i4Min()) && (i4Max() == fldlReal.i4Max());
}


tCIDLib::TBoolean TCQCFldIRangeLimit::bParseLimits(const TString& strLimits)
{
    // We expect two parms, the min/max values
    tCIDLib::TBoolean   bValid1;
    tCIDLib::TBoolean   bValid2;
    tCIDLib::TInt4      i4Max = 0;
    tCIDLib::TInt4      i4Min = 0;
    TString             strParm1;
    TString             strParm2;

    if (bBreakOutLimits(strLimits, L"Range", strParm1, strParm2))
    {
        i4Min = TRawStr::i4AsBinary(strParm1.pszBuffer(), bValid1);
        i4Max = TRawStr::i4AsBinary(strParm2.pszBuffer(), bValid2);
        if (!bValid1 || !bValid2)
            return kCIDLib::False;

        // And they cannot be inverted, or equal. If not, then it's good
        if (i4Min >= i4Max)
            return kCIDLib::False;
    }
     else if (bBreakOutLimits(strLimits, L"GtThan", strParm1))
    {
        i4Min = TRawStr::i4AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        i4Max = kCIDLib::i4MaxInt;
    }
     else if (bBreakOutLimits(strLimits, L"LsThan", strParm1))
    {
        i4Max = TRawStr::i4AsBinary(strParm1.pszBuffer(), bValid1);
        if (!bValid1)
            return kCIDLib::False;
        i4Min = -kCIDLib::i4MinInt;
    }
     else
    {
        return kCIDLib::False;
    }
    SetRange(i4Min, i4Max);
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCFldIRangeLimit::FormatToText(TString& strToFill)
{
    strToFill = L"Range: ";
    strToFill.AppendFormatted(i4Min());
    strToFill.Append(kCIDLib::chComma);
    strToFill.AppendFormatted(i4Max());
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCFldEnumLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldEnumLimit: Public, static methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCFldEnumLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldEnumLimit& fldlReal = static_cast<const TCQCFldEnumLimit&>(fldlSrc);

    return (m_colList == fldlReal.m_colList);
}


//
//  Parses the values out of an enumerated limit field. Our override of
//  bParseLimits() below just calls this. This way, outsides can parse them
//  without having to create one of these objects.
//
//  DO NOT clear the list until we know we are parsing some reasonable
//  enum limit.
//
tCIDLib::TBoolean
TCQCFldEnumLimit::bParseEnumLim(const   TString&                strLimits
                                ,       TCollection<TString>&   colToFill)
{
    // We need to tokenize it
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which must be "Enum"
    if (!stokLimits.bGetNextToken(strTmp)
    ||  (strTmp != L"Enum"))
    {
        return kCIDLib::False;
    }

    // If no more tokens, it can't be valid
    if (!stokLimits.bMoreTokens())
        return kCIDLib::False;

    // Now change the whitespace to a comma
    stokLimits.strWhitespace(L",");

    // Now we get tokens till we run out
    colToFill.RemoveAll();
    while (stokLimits.bGetNextToken(strTmp))
    {
        strTmp.StripWhitespace();
        colToFill.objAdd(strTmp);
    }

    // It can't be an empty list
    if (colToFill.bIsEmpty())
        return kCIDLib::False;

    return kCIDLib::True;

}


// ---------------------------------------------------------------------------
//  TCQCFldEnumLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldEnumLimit::TCQCFldEnumLimit() :

    m_colList()
{
}

TCQCFldEnumLimit::~TCQCFldEnumLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldEnumLimit: Public, inherited methods
// ---------------------------------------------------------------------------

// Parse the passed limit string and store the results
tCIDLib::TBoolean TCQCFldEnumLimit::bParseLimits(const TString& strLimits)
{
    // Call our public static method, with our list
    return bParseEnumLim(strLimits, m_colList);
}


tCIDLib::TBoolean TCQCFldEnumLimit::bValidate(const TString& strToValidate)
{
    return (eValidate(strToValidate) == tCQCKit::EValResults::OK);
}


// Returns the optimal represntation for this limit type
tCQCKit::EOptFldReps TCQCFldEnumLimit::eOptimalRep() const
{
    // If 16 or less, then a combo, else a select dialog
    if (m_colList.c4ElemCount() <= 16)
        return tCQCKit::EOptFldReps::Combo;
    return tCQCKit::EOptFldReps::SelDialog;
}


tCQCKit::EValResults
TCQCFldEnumLimit::eValidate(const TString& strToValidate) const
{
    const tCIDLib::TCard4 c4ValCnt = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
    {
        // If we find it, we are done
        if (strToValidate == m_colList[c4Index])
            return tCQCKit::EValResults::OK;
    }

    // Never found it, so a limit problem
    return tCQCKit::EValResults::LimitViolation;
}


tCIDLib::TVoid TCQCFldEnumLimit::QueryDefVal(TString& strToFill)
{
    if (!m_colList.bIsEmpty())
        strToFill = m_colList[0];
    else
        strToFill.Clear();
}


tCIDLib::TVoid TCQCFldEnumLimit::FormatToText(TString& strToFill)
{
    strToFill = L"Enum: ";

    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strToFill.Append(kCIDLib::chComma);
        strToFill.Append(m_colList[c4Index]);
    }
}



// ---------------------------------------------------------------------------
//  TCQCFldEnumLimit: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the next or previous value from the passed current value
tCIDLib::TBoolean
TCQCFldEnumLimit::bNextPrevVal( const   TString&            strCurVal
                                ,       TString&            strNewVal
                                , const tCIDLib::TBoolean   bNext
                                , const tCIDLib::TBoolean   bWrapOK) const
{
    const tCIDLib::TCard4 c4ValCnt = m_colList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4ValCnt; c4Index++)
    {
        if (strCurVal == m_colList[c4Index])
            break;
    }

    // Leave the index at count if not found or can't move due to wrap
    if (c4Index < c4ValCnt)
    {
        if (bNext)
        {
            c4Index++;
            if (c4Index == c4ValCnt)
            {
                if (bWrapOK)
                    c4Index = 0;
            }
        }
         else
        {
            if (c4Index)
            {
                c4Index--;
            }
             else
            {
                if (bWrapOK)
                    c4Index = c4ValCnt - 1;
                else
                    c4Index = c4ValCnt;
            }
        }
    }

    // If not found or can't change due to wrap, we return the current value
    if (c4Index == c4ValCnt)
    {
        strNewVal = strCurVal;
        return kCIDLib::False;
    }

    strNewVal = m_colList[c4Index];
    return kCIDLib::True;
}


// Get the ordinal for the passed value
tCIDLib::TCard4
TCQCFldEnumLimit::c4QueryOrdinal(const  TString&            strForVal
                                , const tCIDLib::TBoolean   bThrowIfNot) const
{
    const tCIDLib::TCard4 c4ValCnt = m_colList.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4ValCnt; c4Index++)
    {
        if (strForVal == m_colList[c4Index])
            break;
    }

    if (c4Index == c4ValCnt)
    {
        if (bThrowIfNot)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_BadEnumVal
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::ObjState
                , strForVal
            );
        }

        // Just return max card
        return kCIDLib::c4MaxCard;
    }
    return c4Index;
}


tCIDLib::TCard4 TCQCFldEnumLimit::c4QueryValues(TVector<TString>& colToFill) const
{
    // Just copy our list over
    colToFill = m_colList;
    return colToFill.c4ElemCount();
}


tCIDLib::TCard4 TCQCFldEnumLimit::c4ValCount() const
{
    return m_colList.c4ElemCount();
}


TCQCFldEnumLimit::TEnumCursor TCQCFldEnumLimit::cursEnum() const
{
    return TEnumCursor(&m_colList);
}


// Get the value at a given enumerated ordinal
const TString& TCQCFldEnumLimit::strValueAt(const tCIDLib::TCard4 c4At) const
{
    return m_colList[c4At];
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCFldRegExLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldRegExLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldRegExLimit::TCQCFldRegExLimit() :

    m_pregxLimit(new TRegEx)
{
}

TCQCFldRegExLimit::~TCQCFldRegExLimit()
{
    delete m_pregxLimit;
}


// ---------------------------------------------------------------------------
//  TCQCFldRegExLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldRegExLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldRegExLimit& fldlReal = static_cast<const TCQCFldRegExLimit&>(fldlSrc);

    return (m_pregxLimit->strExpression() == fldlReal.m_pregxLimit->strExpression());
}


tCIDLib::TBoolean TCQCFldRegExLimit::bParseLimits(const TString& strLimits)
{
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which must be "RegEx"
    if (!stokLimits.bGetNextToken(strTmp) || (strTmp != L"RegEx"))
        return kCIDLib::False;

    // And we take the rest of the string as the expression
    if (!stokLimits.bPeekRestOfLine(strTmp))
        return kCIDLib::False;

    // Ask the engine to set this as the expression
    try
    {
        m_pregxLimit->SetExpression(strTmp);
    }

    catch(const TError& errToCatch)
    {
        if (facCQCKit().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCFldRegExLimit::bValidate(const TString& strToValidate)
{
    return (eValidate(strToValidate) == tCQCKit::EValResults::OK);
}


tCQCKit::EOptFldReps TCQCFldRegExLimit::eOptimalRep() const
{
    // It's always just a text input field
    return tCQCKit::EOptFldReps::Text;
}


tCQCKit::EValResults
TCQCFldRegExLimit::eValidate(const TString& strToValidate) const
{
    // If it fully matches the pattern, case sensitive, it's good
    if (!m_pregxLimit->bFullyMatches(strToValidate, kCIDLib::True))
        return tCQCKit::EValResults::LimitViolation;
    return tCQCKit::EValResults::OK;
}


tCIDLib::TVoid TCQCFldRegExLimit::QueryDefVal(TString& strToFill)
{
    //
    //  We cannot really create a value that we know will match the
    //  regular expression, so we just set it to an empty value.
    //
    strToFill.Clear();
}


tCIDLib::TVoid TCQCFldRegExLimit::FormatToText(TString& strToFill)
{
    strToFill = L"RegEx: ";
    strToFill.Append(m_pregxLimit->strExpression());
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFldMediaImgLimit
//  PREFIX: fldl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFldMediaImgLimit: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFldMediaImgLimit::TCQCFldMediaImgLimit()
{
}

TCQCFldMediaImgLimit::~TCQCFldMediaImgLimit()
{
}


// ---------------------------------------------------------------------------
//  TCQCFldMediaImgLimit: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCFldMediaImgLimit::bSameLimits(const TCQCFldLimit& fldlSrc) const
{
    // Check that the passed value is our type and has our same list
    ValidateTypes(fldlSrc.clsIsA());
    const TCQCFldMediaImgLimit& fldlReal = static_cast<const TCQCFldMediaImgLimit&>(fldlSrc);

    return (m_colTypes == fldlReal.m_colTypes);
}


tCIDLib::TBoolean TCQCFldMediaImgLimit::bParseLimits(const TString& strLimits)
{
    TString strTmp;
    TStringTokenizer stokLimits(&strLimits, L": ");

    // Get the first token, which must be "MediaImg"
    if (!stokLimits.bGetNextToken(strTmp) || (strTmp != L"MediaImg"))
        return kCIDLib::False;

    m_colTypes.RemoveAll();

    // And break out the rest of the string to get the types
    stokLimits.strWhitespace(L" ,");
    while (stokLimits.bGetNextToken(strTmp))
        m_colTypes.objAdd(strTmp);

    // If no types, return false to fail it
    return !m_colTypes.bIsEmpty();
}


tCIDLib::TBoolean TCQCFldMediaImgLimit::bValidate(const TString& strToValidate)
{
    return (eValidate(strToValidate) == tCQCKit::EValResults::OK);
}


tCIDLib::TCard4 TCQCFldMediaImgLimit::c4QueryValues(TVector<TString>& colToFill) const
{
    // Our internal type happens to be the same as the parm, so just assign
    colToFill = m_colTypes;
    return m_colTypes.c4ElemCount();
}


tCQCKit::EOptFldReps TCQCFldMediaImgLimit::eOptimalRep() const
{
    return tCQCKit::EOptFldReps::Text;
}


tCQCKit::EValResults
TCQCFldMediaImgLimit::eValidate(const TString& strToValidate) const
{
    // <TBD> Why aren't we validating this?
    return tCQCKit::EValResults::OK;
}


tCIDLib::TVoid TCQCFldMediaImgLimit::QueryDefVal(TString& strToFill)
{
    // Just return an empty string
    strToFill.Clear();
}


tCIDLib::TVoid TCQCFldMediaImgLimit::FormatToText(TString& strToFill)
{
    strToFill = L"MediaImg: ";

    const tCIDLib::TCard4 c4Count = m_colTypes.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strToFill.Append(kCIDLib::chComma);
        strToFill.Append(m_colTypes[c4Index]);
    }
}


