//
// FILE NAME: CQCKit_Expression.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2004
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
//  This is the implementation file for the expression class.
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
RTTIDecls(TCQCExpression,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_Expression
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        //
        //  Version 2 -
        //      Add support for dynamic comparison values (i.e. includes standard
        //      replacement tokens.)
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 2;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCExpression
//  PREFIX: expr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCExpression: Public, static methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCExpression::bNeedsCompVal(const tCQCKit::EStatements eToCheck)
{
    tCIDLib::TBoolean bRet;
    switch(eToCheck)
    {
        // These act just on the field value
        case tCQCKit::EStatements::None :
        case tCQCKit::EStatements::IsTrue :
        case tCQCKit::EStatements::IsFalse :
        case tCQCKit::EStatements::IsNonNull :
        case tCQCKit::EStatements::IsNegative :
        case tCQCKit::EStatements::IsPositive :
        case tCQCKit::EStatements::IsEven :
        case tCQCKit::EStatements::IsOdd :
        case tCQCKit::EStatements::IsAlpha:
        case tCQCKit::EStatements::IsAlphaNum:
            bRet = kCIDLib::False;
            break;

        // These compare the field value to the entered value
        case tCQCKit::EStatements::IsEqual :
        case tCQCKit::EStatements::NotEqual :
        case tCQCKit::EStatements::IsGThan :
        case tCQCKit::EStatements::IsGThanEq :
        case tCQCKit::EStatements::IsLsThan :
        case tCQCKit::EStatements::IsLsThanEq :
        case tCQCKit::EStatements::IsMultipleOf :
            bRet = kCIDLib::True;
            break;

        default :
            CIDAssert2(L"Unknown expression statement type")
            bRet = kCIDLib::False;
            break;
    };
    return bRet;
}


tCIDLib::TBoolean
TCQCExpression::bNeedsCompVal(  const   tCQCKit::EExprTypes     eType
                                , const tCQCKit::EStatements    eStatement)
{
    if (eType == tCQCKit::EExprTypes::None)
        return kCIDLib::False;

    if (eType == tCQCKit::EExprTypes::RegEx)
        return kCIDLib::True;

    return bNeedsCompVal(eStatement);
}


tCIDLib::TBoolean
TCQCExpression::bTakesFld(  const   tCQCKit::EStatements    eToCheck
                            , const tCQCKit::EFldTypes      eType)
{
    // If it's not a valid field type, then obviously not
    if (eType == tCQCKit::EFldTypes::Count)
        return kCIDLib::False;

    //
    //  To maximize speed and minimize clutter, we just do a simple
    //  adjacency graph. A one in a cell indicates that that statement
    //  supports that data type.
    //

    //
    //  !! Couldn't use tCIDLib::c4EnumOrd(x) here, the compiler says it's not a
    //  const expression, when clearly it should be, since it's inline and evaluates
    //  to a simple cast. But, oh well, just casted them to TCard4 directly instead.
    //
    constexpr tCIDLib::TCard4 c4FCnt = tCIDLib::c4EnumOrd(tCQCKit::EFldTypes::Count);
    constexpr tCIDLib::TCard4 c4SCnt = tCIDLib::c4EnumOrd(tCQCKit::EStatements::Count);
    constexpr tCIDLib::TCard1 ac1Graph[c4SCnt][c4FCnt] =
    {
        // Bool  Card  Float   Int  String  SList  Time
        {     0,    0,     0,    0,      0,     0,    0 }  // None
      , {     1,    1,     1,    1,      1,     0,    1 }  // IsEqual
      , {     1,    1,     1,    1,      1,     0,    1 }  // NotEqual
      , {     1,    1,     1,    1,      1,     0,    1 }  // True
      , {     1,    1,     1,    1,      1,     0,    1 }  // False
      , {     0,    1,     1,    1,      1,     0,    1 }  // GreaterThan
      , {     0,    1,     1,    1,      1,     0,    1 }  // GreaterThanEq
      , {     0,    1,     1,    1,      1,     0,    1 }  // LessThan
      , {     0,    1,     1,    1,      1,     0,    1 }  // LessThanEq
      , {     0,    1,     1,    1,      1,     1,    1 }  // NonNull
      , {     0,    0,     1,    1,      0,     0,    0 }  // Negative
      , {     0,    0,     1,    1,      0,     0,    0 }  // Positive
      , {     0,    1,     0,    1,      0,     0,    0 }  // IsEven
      , {     0,    1,     0,    1,      0,     0,    0 }  // IsOdd
      , {     0,    1,     0,    1,      0,     0,    1 }  // IsMultipleOf
      , {     0,    0,     0,    0,      1,     0,    0 }  // IsAlpha
      , {     0,    0,     0,    0,      1,     0,    0 }  // IsAlphaNum
    };
    return ac1Graph[tCIDLib::c4EnumOrd(eToCheck)][tCIDLib::c4EnumOrd(eType)] != 0;
}


const TString& TCQCExpression::strKey(const TCQCExpression& exprSrc)
{
    return exprSrc.strDescription();
}


// ---------------------------------------------------------------------------
//  TCQCExpression: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCExpression::TCQCExpression() :

    m_bDynComp(kCIDLib::False)
    , m_bNegated(kCIDLib::False)
    , m_eType(tCQCKit::EExprTypes::None)
    , m_eStatement(tCQCKit::EStatements::None)
{
}

TCQCExpression::TCQCExpression(const TString& strDescr) :

    m_bDynComp(kCIDLib::False)
    , m_bNegated(kCIDLib::False)
    , m_eType(tCQCKit::EExprTypes::None)
    , m_eStatement(tCQCKit::EStatements::None)
    , m_strDescr(strDescr)
{
}

TCQCExpression::TCQCExpression( const   TString&                strDescr
                                , const tCQCKit::EExprTypes     eType
                                , const tCQCKit::EStatements    eStatement
                                , const TString&                strCompVal
                                , const tCIDLib::TBoolean       bNegated) :
    m_bDynComp(kCIDLib::False)
    , m_bNegated(bNegated)
    , m_eType(eType)
    , m_eStatement(eStatement)
    , m_strCompVal(strCompVal)
    , m_strDescr(strDescr)
{
    // Remember if the comp value has any replacement tokens
    CheckDynComp(strCompVal);

    // If it's a reg ex, then compile the expression
    if (m_eType == tCQCKit::EExprTypes::RegEx)
        m_regxPattern.SetExpression(m_strCompVal);
}

TCQCExpression::~TCQCExpression()
{
}


// ---------------------------------------------------------------------------
//  TCQCExpression: Public oeprators
// ---------------------------------------------------------------------------

//
//  We don't compare the regular expression object, since it is created from the comp value
//  in the case of a regular expressions. So if the comp values are equal, the regular
//  epxression would be as well.
//
tCIDLib::TBoolean TCQCExpression::operator==(const TCQCExpression& exprSrc) const
{
    return
    (
        (m_bDynComp == exprSrc.m_bDynComp)
        && (m_bNegated == exprSrc.m_bNegated)
        && (m_eType == exprSrc.m_eType)
        && (m_eStatement == exprSrc.m_eStatement)
        && (m_strCompVal == exprSrc.m_strCompVal)
        && (m_strDescr == exprSrc.m_strDescr)
    );
}

tCIDLib::TBoolean TCQCExpression::operator!=(const TCQCExpression& exprSrc) const
{
    return !operator==(exprSrc);
}


// ---------------------------------------------------------------------------
//  TCQCExpression: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Checks to see if this expression can accept a specific field type, i.e. the
//  data type of the field can be converted to something this expression can use.
//
tCIDLib::TBoolean
TCQCExpression::bCanAcceptField(const   tCQCKit::EFldTypes  eFldType
                                , const tCIDLib::TBoolean   bUnusedOK) const
{
    //
    //  If not set, then we return the bUnusedOK parameter.
    //
    if (m_eType == tCQCKit::EExprTypes::None)
        return bUnusedOK;

    // If a reg ex, it can potentially take anything
    else if (m_eType == tCQCKit::EExprTypes::RegEx)
        return kCIDLib::True;

    // Else call a static helper to test the statement type against the fld type
    return bTakesFld(m_eStatement, eFldType);
}


//
//  This method evaluates this expression by comparing it to the value of
//  the passed field value.
//
tCIDLib::TBoolean
TCQCExpression::bEvaluate(  const   TCQCFldValue&       fvToCompare
                            , const TStdVarsTar* const  pctarGVars) const
{
    // If not set, that's an error
    if (m_eType == tCQCKit::EExprTypes::None)
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcExpr_NoExprSet
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_eType == tCQCKit::EExprTypes::Statement)
    {
        //
        //  It's a statement so ask the value object to evaluate the statement
        //  and return a pass/fail result.
        //

        // If a dynamic comparison value, then do the replacement
        tCQCKit::EStmtRes eRes = tCQCKit::EStmtRes::Count;
        if (m_bDynComp && !m_strCompVal.bIsEmpty())
        {
            TString strCompVal;
            const tCQCKit::ECmdPrepRes eTokRes = facCQCKit().eStdTokenReplace
            (
                m_strCompVal
                , 0
                , pctarGVars
                , 0
                , strCompVal
                , tCQCKit::ETokRepFlags::None
            );

            if (eTokRes == tCQCKit::ECmdPrepRes::Changed)
                eRes = fvToCompare.eTestStatement(m_eStatement, strCompVal);
            else if (eTokRes == tCQCKit::ECmdPrepRes::Unchanged)
                eRes = fvToCompare.eTestStatement(m_eStatement, m_strCompVal);
            else
            {
                facCQCKit().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcTokR_TokenExpansion
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strCompVal
                );
            }
        }
         else
        {
            // Not dymamic, so use the literal comparison value
            eRes = fvToCompare.eTestStatement(m_eStatement, m_strCompVal);
        }

        if ((eRes == tCQCKit::EStmtRes::BadValue)
        ||  (eRes == tCQCKit::EStmtRes::BadStatement))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcExpr_BadStatement
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , m_strDescr
            );
        }
         else if (eRes == tCQCKit::EStmtRes::True)
        {
            bRet = kCIDLib::True;
        }
    }
     else
    {
        // Its a regular expression, so evaluate it
        fvToCompare.Format(m_strFmt);
        if (m_regxPattern.bFullyMatches(m_strFmt, kCIDLib::True))
            bRet = kCIDLib::True;
    }

    // If negated, flip if appropriate
    if (m_bNegated)
        bRet = !bRet;
    return bRet;
}


// Indicates whether this experssion uses a comparison value or not
tCIDLib::TBoolean TCQCExpression::bHasCompVal() const
{
    return bNeedsCompVal(m_eType, m_eStatement);
}


// Get/set the negated flag
tCIDLib::TBoolean TCQCExpression::bNegated() const
{
    return m_bNegated;
}

tCIDLib::TBoolean TCQCExpression::bNegated(const tCIDLib::TBoolean bToSet)
{
    m_bNegated = bToSet;
    return m_bNegated;
}


tCIDLib::TBoolean
TCQCExpression::bValidate(          TString&            strErrText
                            , const tCQCKit::EFldTypes  eFldType) const
{
    // If no expression set, then we always pass
    if (m_eType == tCQCKit::EExprTypes::None)
        return kCIDLib::True;

    //
    //  It's either a reg expression, which must have text, or a statement,
    //  which may or may not, according to the type.
    //
    if (m_eType == tCQCKit::EExprTypes::RegEx)
    {
        if (m_strCompVal.bIsEmpty())
        {
            strErrText.LoadFromMsg
            (
                kKitErrs::errcExpr_EmptyRegEx, facCQCKit(), m_strDescr
            );
            return kCIDLib::False;
        }

        try
        {
            m_regxPattern.SetExpression(m_strCompVal);
        }

        catch(const TError& errToCatch)
        {
            strErrText.LoadFromMsg
            (
                kKitErrs::errcExpr_RegExErr
                , facCQCKit()
                , m_strDescr
                , errToCatch.strErrText()
            );
            return kCIDLib::False;
        }
    }
     else
    {
        //
        //  Gotta be a statement. Each statement type either requires a value
        //  or doesn't want one. We can't pass judgement on it beyond that.
        //  First though, see if the selected field is legal for the kind of
        //  statement selected.
        //
        //  If the field isn't set yet, just accept it.
        //
        if (eFldType != tCQCKit::EFldTypes::Count)
        {
            if (!bTakesFld(m_eStatement, eFldType))
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcExpr_BadFldForStatement
                    , facCQCKit()
                    , tCQCKit::strXlatEStatements(m_eStatement)
                    , tCQCKit::strXlatEFldTypes(eFldType)
                );
                return kCIDLib::False;
            }
        }

        if (bNeedsCompVal(m_eStatement))
        {
            if (m_strCompVal.bIsEmpty())
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcExpr_StatementNeedsText
                    , facCQCKit()
                    , tCQCKit::strXlatEStatements(m_eStatement)
                );
                return kCIDLib::False;
            }

            //
            //  See if it has any replacement tokens in it. If so, they can only
            //  be global variable references.
            //
            try
            {
                const tCQCKit::ECmdPrepRes eTokRes = facCQCKit().eStdTokenReplace
                (
                    m_strCompVal
                    , 0
                    , 0
                    , 0
                    , strErrText
                    , tCIDLib::eOREnumBits
                      (
                        tCQCKit::ETokRepFlags::GVarsOnly
                        , tCQCKit::ETokRepFlags::TestOnly
                      )
                );

                // The output will be the error text if it failed, so just return
                if (eTokRes == tCQCKit::ECmdPrepRes::Failed)
                    return kCIDLib::False;
            }

            catch(const TError& errToCatch)
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcExpr_ExpandFailed
                    , facCQCKit()
                    , m_strDescr
                    , errToCatch.strErrText()
                );
                return kCIDLib::False;
            }
        }
         else
        {
            if (!m_strCompVal.bIsEmpty())
            {
                strErrText.LoadFromMsg
                (
                    kKitErrs::errcExpr_StatementHasText
                    , facCQCKit()
                    , m_strDescr
                );
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


tCQCKit::EExprTypes TCQCExpression::eType() const
{
    return m_eType;
}

tCQCKit::EExprTypes TCQCExpression::eType(const tCQCKit::EExprTypes eToSet)
{
    m_eType = eToSet;

    //
    //  If setting a regular expression, set the statement to none. If setting
    //  to statement make sure we have a valid initial statement type selected.
    //
    if (eToSet == tCQCKit::EExprTypes::RegEx)
    {
        m_eStatement = tCQCKit::EStatements::None;
    }
     else if (eToSet == tCQCKit::EExprTypes::Statement)
    {
        m_eStatement = tCQCKit::EStatements::None;
    }

    //
    //  Either way reset the regular expression. Either it's not needed or they need to
    //  set one now.
    //
    m_regxPattern = TRegEx();

    // If the setup doesn't support a comp value, then clear it
    if (!bHasCompVal())
        m_strCompVal.Clear();

    return m_eType;
}


tCQCKit::EStatements TCQCExpression::eStatement() const
{
    return m_eStatement;
}

tCQCKit::EStatements
TCQCExpression::eStatement(const tCQCKit::EStatements eToSet)
{
    m_eStatement = eToSet;

    // If the setup doesn't support a comp value, then clear it
    if (!bHasCompVal())
        m_strCompVal.Clear();

    return m_eStatement;
}


tCIDLib::TVoid TCQCExpression::FormatToXML(TTextOutStream& strmTarget) const
{
    // If the type has not been set, then do nothing
    if (m_eType == tCQCKit::EExprTypes::None)
        return;

    // Open the element, then indent and do the attributes
    strmTarget << L"\n<Expr ";
    {
        TStreamIndentJan janIndent(&strmTarget, 6);

        strmTarget  << L"Neg='" << m_bNegated << L"' Type='"
                    << tCQCKit::strXlatEExprTypes(m_eType) << L"'"
                    << L"\nDesc='" << m_strDescr << L"'";

        if (m_eType == tCQCKit::EExprTypes::Statement)
        {
            strmTarget  << L"\nStmt='"
                        << tCQCKit::strXlatEStatements(m_eStatement)
                        << L"'";

            // Only do the comp value if this type of statement needs it
            if (bNeedsCompVal(m_eStatement))
                strmTarget << L"\nComp='" << m_strCompVal << L"'";
        }
         else
        {
            strmTarget << L"\nExpr='" << m_strCompVal << L"'";
        }
    }
    strmTarget << L"/>";
}


tCIDLib::TVoid
TCQCExpression::QueryExprAttrs(         tCIDMData::TAttrList&   colAttrs
                                ,       TAttrData&              adatTmp
                                , const tCIDLib::TBoolean       bSkipDescr) const
{
    if (!bSkipDescr)
    {
        adatTmp.Set
        (
            L"Description"
            , kCQCKit::strAttr_Expr_Descr
            , tCIDMData::EAttrTypes::String
            , tCIDMData::EAttrEdTypes::Both
        );
        adatTmp.SetString(m_strDescr);
        colAttrs.objAdd(adatTmp);
    }

    // It's either an regular expression or a statement
    TString strLims(L"Enum: Regular Expr, Statement");
    adatTmp.Set
    (
        L"Type"
        , kCQCKit::strAttr_Expr_Type
        , strLims
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );

    if (m_eType == tCQCKit::EExprTypes::RegEx)
        adatTmp.SetString(L"Regular Expr");
    else
        adatTmp.SetString(L"Statement");
    colAttrs.objAdd(adatTmp);


    // If a statement type, it's one of these
    strLims = L"Enum: ";
    tCQCKit::EStatements eStmnt = tCQCKit::EStatements::FirstUsed;
    while (eStmnt <= tCQCKit::EStatements::Max)
    {
        if (eStmnt > tCQCKit::EStatements::FirstUsed)
            strLims.Append(L", ");
        strLims.Append(tCQCKit::strXlatEStatements(eStmnt));
        eStmnt++;
    }

    //
    //  Disabled if in reg ex mode. The using code is responsible for updating this
    //  in the attribute editor if that changes during editing.
    //
    adatTmp.Set
    (
        L"Statement"
        , kCQCKit::strAttr_Expr_Statement
        , strLims
        , tCIDMData::EAttrTypes::String
        , (m_eType == tCQCKit::EExprTypes::RegEx) ? tCIDMData::EAttrEdTypes::None
                                            : tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(tCQCKit::strXlatEStatements(m_eStatement));
    colAttrs.objAdd(adatTmp);

    //
    //  We enable visual editing here as well as inplace. We assume that anyone that
    //  uses this call will implement the interactive field value selection mechanism
    //  for visual editing. Otherwise, they'll just get a generic text entry.
    //
    //  We disable it if we currently don't support a comparison value.
    //
    adatTmp.Set
    (
        L"Comp Value"
        , kCQCKit::strAttr_Expr_CompVal
        , tCIDMData::EAttrTypes::String
        , bHasCompVal() ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
    );
    adatTmp.SetString(m_strCompVal);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set(L"Negated", kCQCKit::strAttr_Expr_Negated, tCIDMData::EAttrTypes::Bool);
    adatTmp.SetBool(m_bNegated);
    colAttrs.objAdd(adatTmp);
}


const TString& TCQCExpression::strCompVal() const
{
    return m_strCompVal;
}

const TString& TCQCExpression::strCompVal(const TString& strToSet)
{
    // Remember if the comp value has any replacement tokens
    CheckDynComp(strToSet);

    m_strCompVal = strToSet;
    return m_strCompVal;
}


const TString& TCQCExpression::strDescription() const
{
    return m_strDescr;
}

const TString& TCQCExpression::strDescription(const TString& strToSet)
{
    m_strDescr = strToSet;
    return m_strDescr;
}


tCIDLib::TVoid TCQCExpression::Set( const   TString&                strDescr
                                    , const tCQCKit::EExprTypes     eType
                                    , const tCQCKit::EStatements    eStatement
                                    , const TString&                strCompVal
                                    , const tCIDLib::TBoolean       bNegated)
{
    m_bNegated   = bNegated;
    m_eType      = eType;
    m_eStatement = eStatement;
    m_strCompVal = strCompVal;
    m_strDescr   = strDescr;

    // If the setup doesn't support a comp value, then override
    if (!bHasCompVal())
        m_strCompVal.Clear();

    // Remember if the comp value has any replacement tokens
    CheckDynComp(strCompVal);

    //
    //  If it's a regular expression, then set up the engine and validate
    //  the expression by setting it on the engine.
    //
    if (m_eType == tCQCKit::EExprTypes::RegEx)
        m_regxPattern.SetExpression(m_strCompVal);
}

tCIDLib::TVoid TCQCExpression::Set( const   tCQCKit::EExprTypes     eType
                                    , const tCQCKit::EStatements    eStatement
                                    , const TString&                strCompVal
                                    , const tCIDLib::TBoolean       bNegated)
{
    m_bNegated   = bNegated;
    m_eType      = eType;
    m_eStatement = eStatement;
    m_strCompVal = strCompVal;

    // If the setup doesn't support a comp value, then override
    if (!bHasCompVal())
        m_strCompVal.Clear();

    // Remember if the comp value has any replacement tokens
    CheckDynComp(strCompVal);

    //
    //  If it's a regular expression, then set up the engine and validate
    //  the expression by setting it on the engine.
    //
    if (m_eType == tCQCKit::EExprTypes::RegEx)
        m_regxPattern.SetExpression(m_strCompVal);
}


//
//  For convenience when setting up an expression via an attribute editor, which is
//  not uncommon.
//
tCIDLib::TVoid
TCQCExpression::SetExprAttr(const   TAttrData&          adatNew
                            , const TAttrData&          adatOld)
{
    if (adatNew.strId() == kCQCKit::strAttr_Expr_CompVal)
    {
        m_strCompVal = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCKit::strAttr_Expr_Descr)
    {
        m_strDescr = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCKit::strAttr_Expr_Negated)
    {
        m_bNegated = adatNew.bVal();
    }
     else if (adatNew.strId() == kCQCKit::strAttr_Expr_Statement)
    {
        m_eStatement = tCQCKit::eXlatEStatements(adatNew.strValue());
    }
     else if (adatNew.strId() == kCQCKit::strAttr_Expr_Type)
    {
        if (adatNew.strValue().bCompareI(L"Statement"))
            eType(tCQCKit::EExprTypes::Statement);
        else
            eType(tCQCKit::EExprTypes::RegEx);
    }

    // If we no longer support a comp value, clear it
    if(!bHasCompVal())
        m_strCompVal.Clear();
}


// ---------------------------------------------------------------------------
//  TCQCExpression: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCExpression::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_Expression::c2FmtVersion))
    {
        // We are throwing a CIDLib error here, not one of ours!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    strmToReadFrom  >> m_bNegated
                    >> m_eType
                    >> m_eStatement
                    >> m_strCompVal
                    >> m_strDescr;

    // If V2 or beyond, read in the dynamic comparison value flag, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bDynComp;
    else
        m_bDynComp = kCIDLib::False;

    // And the end object marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // If it's a regular expression, then set up the engine
    if (m_eType == tCQCKit::EExprTypes::RegEx)
    {
        m_regxPattern.SetExpression(m_strCompVal);
    }

    //
    //  Deal with a previous error in one of the ctors. If the type is none, make sure the
    //  statement type is none.
    //
    if (m_eType == tCQCKit::EExprTypes::None)
        m_eStatement = tCQCKit::EStatements::None;

    // Also we now internally enforce the comp value, which wasn't the case before
    if (!bHasCompVal())
        m_strCompVal.Clear();
}


tCIDLib::TVoid TCQCExpression::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_Expression::c2FmtVersion
                    << m_bNegated
                    << m_eType
                    << m_eStatement
                    << m_strCompVal
                    << m_strDescr

                    // V2 stuff
                    << m_bDynComp

                    << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//  TCQCExpression: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Checks to see if the passed comparison value has any replacement tokens. If so,
//  then we set the m_bDynComp flag, else we clear it. This lets us know later if
//  we need to expand the value upon the evaluate call, instead of just doing it
//  every time the evaluation is done.
//
tCIDLib::TVoid TCQCExpression::CheckDynComp(const TString& strToCheck)
{
    TString strRes;
    const tCQCKit::ECmdPrepRes eRes = facCQCKit().eStdTokenReplace
    (
        strToCheck
        , 0
        , 0
        , 0
        , strRes
        , tCIDLib::eOREnumBits
          (
            tCQCKit::ETokRepFlags::TestOnly, tCQCKit::ETokRepFlags::GVarsOnly
          )
    );

    m_bDynComp = (eRes == tCQCKit::ECmdPrepRes::Changed);
}

