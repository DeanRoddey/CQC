//
// FILE NAME: CQCKit_Expression.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/01/2004
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
//  This file defines an expression, which is used in a number of places in
//  CQC. It allows the user to compare the value of a field to some fixed
//  value or regular expression or to check some aspect of it. The result
//  of the expression evaluation is a true or false result.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCFldValue;

// ---------------------------------------------------------------------------
//   CLASS: TCQCExpression
//  PREFIX: expr
// ---------------------------------------------------------------------------
class CQCKITEXPORT TCQCExpression : public TObject, public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::TBoolean bNeedsCompVal
        (
            const   tCQCKit::EStatements    eToCheck
        );

        static tCIDLib::TBoolean bNeedsCompVal
        (
            const   tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
        );

        static tCIDLib::TBoolean bTakesFld
        (
            const   tCQCKit::EStatements    eToCheck
            , const tCQCKit::EFldTypes      eType
        );

        static const TString& strKey(const TCQCExpression& exprSrc);


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCExpression();

        TCQCExpression
        (
            const   TString&                strDescr
        );

        TCQCExpression
        (
            const   TString&                strDescr
            , const tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
            , const TString&                strCompVal
            , const tCIDLib::TBoolean       bNegated
        );

        TCQCExpression
        (
            const   TCQCExpression&         exprSrc
        );

        TCQCExpression(TCQCExpression&&) = delete;

        ~TCQCExpression();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCExpression& operator=
        (
            const   TCQCExpression&         exprSrc
        );

        TCQCExpression& operator=(TCQCExpression&&) = delete;

        tCIDLib::TBoolean operator==
        (
            const   TCQCExpression&         exprSrc
        )   const;

        tCIDLib::TBoolean operator!=
        (
            const   TCQCExpression&         exprSrc
        )   const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   tCQCKit::EFldTypes      eFldType
            , const tCIDLib::TBoolean       bUnusedOK
        )   const;

        tCIDLib::TBoolean bEvaluate
        (
            const   TCQCFldValue&           fvToCompare
            , const TStdVarsTar* const      pctarGVars
        )   const;

        [[nodiscard]] tCIDLib::TBoolean bHasCompVal() const;

        [[nodiscard]] tCIDLib::TBoolean bNegated() const;

        tCIDLib::TBoolean bNegated
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErrText
            , const tCQCKit::EFldTypes      eFldType
        )   const;

        [[nodiscard]] tCQCKit::EExprTypes eType() const;

        tCQCKit::EExprTypes eType
        (
            const   tCQCKit::EExprTypes     eToSet
        );

        [[nodiscard]] tCQCKit::EStatements eStatement() const;

        tCQCKit::EStatements eStatement
        (
            const   tCQCKit::EStatements    eToSet
        );

        tCIDLib::TVoid FormatToXML
        (
                    TTextOutStream&         strmTarget
        )   const;

        tCIDLib::TVoid QueryExprAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
            , const tCIDLib::TBoolean       bSkipDescr
        )   const;

        [[nodiscard]] const TString& strCompVal() const;

        const TString& strCompVal
        (
            const   TString&                strToSet
        );

        [[nodiscard]] const TString& strDescription() const;

        const TString& strDescription
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strDescr
            , const tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
            , const TString&                strCompVal
            , const tCIDLib::TBoolean       bNegated
        );

        tCIDLib::TVoid Set
        (
            const   tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
            , const TString&                strCompVal
            , const tCIDLib::TBoolean       bNegated
        );

        tCIDLib::TVoid SetExprAttr
        (
            const   TAttrData&              adatNew
            , const TAttrData&              adatOld
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CheckDynComp
        (
            const   TString&                strToCheck
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDynComp
        //      The comparison value contains at least one replacement token.
        //      This means that, upon evaluation, we have to dynamically expand
        //      the comparison value at that point. And, if a regular expression,
        //      we have to set the expression at that point since only then do we
        //      know the actual expression contents.
        //
        //  m_bNegated
        //      Indicates whether the user wants the result of the expression
        //      to be negated. If so, the negated result is what is returned.
        //
        //  m_eType
        //  m_eStatement
        //      The expression type, and if a statement, then which statement
        //      to invoke.
        //
        //  m_pregxExpr
        //      If the expression is a regular expression, we have to have
        //      a reg ex engine around to evaluate it. We make this a pointer
        //      so as not to force the reg ex headers on folks who use this
        //      facility and because it's not always needed. So we allocate
        //      it as needed.
        //
        //  m_strCompVal
        //      If the statement takes a fixed value to compare against, then
        //      this is it. If it's a regular expression, then this is the
        //      expression text.
        //
        //  m_strDescr
        //      The human consumable description of the expression. This might
        //      not be used in places where there is only one.
        //
        //  m_strFmt
        //      We need a temp string sometimes in the evaluation method,
        //      so we keep one around, to avoid allocating and deleting it
        //      over and over. It's mutable because the evalute method is
        //      const.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDynComp;
        tCIDLib::TBoolean       m_bNegated;
        tCQCKit::EExprTypes     m_eType;
        tCQCKit::EStatements    m_eStatement;
        TRegEx*                 m_pregxExpr;
        TString                 m_strCompVal;
        TString                 m_strDescr;
        mutable TString         m_strFmt;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCExpression,TObject)
};

#pragma CIDLIB_POPPACK


