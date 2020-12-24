//
// FILE NAME: CQLogicSh_MathFld.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/12/2009
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
//  This is the header for the CQLogicSh_MathFld.cpp file, which
//  implements one of our virtual field types. This is one of the more
//  complex ones. It allows the user to provide a simple mathematical expression
//  that consists of parenthesized clauses, each of which resolves to left/
//  right side values separated by an operator. The values can be sub-clauses,
//  constant values, or tokens that represent the associated field values, where
//  %(1) is the first field, %(2) is the second field, and so forth.
//
//  When we are called to build the value, we evaluate the expression and
//  give back the result. The expression is parsed into a simple tree graph
//  that eventually resolves down to literal values or field tokens.
//
//  The expression has to at least have start and end parens, so the minimal
//  expression is something like "(x + y)" (without quotes) which simplifies
//  the parsing a lot, and insures that the root node will always be an
//  operation type node and that the end of input can only ever be seen after
//  getting back to the root node again.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDMath
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDMath : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDMath
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
        );

        TCQSLLDMath
        (
            const   TCQSLLDMath&            cllftSrc
        );

        TCQSLLDMath(TCQSLLDMath&&) = delete;

        ~TCQSLLDMath();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLLDMath& operator=
        (
            const   TCQSLLDMath&            clsftSrc
        );

        TCQSLLDMath& operator=(TCQSLLDMath&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsEqual
        (
            const   TCQLSrvFldType&         clsftComp
        )   const final;

        tCIDLib::TBoolean bIsValidSrcFld
        (
            const   TCQCFldDef&             flddToCheck
        )   const final;

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErr
            ,       tCIDLib::TCard4&        c4SrcFldInd
            , const TCQCFldCache&           cfcData
        )   final;

        tCIDLib::TVoid Initialize
        (
                    TCQCFldCache&           cfcInit
        )   final;

        [[nodiscard]] TCQCFldFilter* pffiltToUse() final;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bParseExpression
        (
                    TString&                strErr
        );

        const TString& strFormula() const;

        const TString& strFormula
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  This simple class is used to create our evaluation tree. It can
        //  refer to two other nodes (with a math operation to apply to them),
        //  or it can be a function node that has some amount of tree inside
        //  it, or it can be a terminal node with the values to operate on.
        //
        //  The terminal nodes can be a literal or a field ref. If a literal
        //  we store the value and the type. If a field ref, the type is
        //  determined at evaluation time based on the field type.
        // -------------------------------------------------------------------
        class TNode
        {
            public :
                enum class ENodeTypes
                {
                    Abs
                    , Add
                    , AND
                    , Cosine
                    , Div
                    , Float
                    , FldRef
                    , ModDiv
                    , Mul
                    , NLog
                    , OR
                    , Power
                    , Signed
                    , Sine
                    , SqRoot
                    , Sub
                    , ToCard
                    , ToFloat
                    , ToInt
                    , Unsigned
                    , XOR
                };


                TNode
                (
                    const   ENodeTypes      eOp
                    ,       TNode* const    pnodeLeft
                    ,       TNode* const    pnodeRight
                );

                TNode
                (
                    const   ENodeTypes      eOp
                    ,       TNode* const    pnodeLeft
                    ,       TNode* const    pnodeRight
                    , const tCQCKit::EFldTypes eFldType
                );

                TNode
                (
                    const   ENodeTypes      eFunc
                    ,       TNode* const    pnodeInside
                    , const tCQCKit::EFldTypes eFldType
                );

                TNode
                (
                    const   tCIDLib::TCard4 c4Literal
                );

                TNode
                (
                    const   tCIDLib::TFloat8 f8Literal
                );

                TNode
                (
                    const   tCIDLib::TInt4  i4Literal
                );

                TNode
                (
                    const   ENodeTypes      eType
                    , const tCIDLib::TCard4 c4FldIndex
                );

                TNode(const TNode&) = delete;
                TNode(TNode&&) = delete;

                ~TNode();

                TNode& operator=(const TNode&) = delete;
                TNode& operator=(TNode&&) = delete;

                tCIDLib::TVoid Evaluate
                (
                    const   tCQLogicSh::TInfoList& colVals
                );

                tCIDLib::TCard4 c4AsCard() const;

                tCIDLib::TFloat8 f8AsFloat() const;

                tCIDLib::TInt4 i4AsInt() const;


                tCIDLib::TCard4     m_c4FldIndex;
                tCIDLib::TCard4     m_c4Value;
                tCQCKit::EFldTypes  m_eFldType;
                ENodeTypes          m_eType;
                tCIDLib::TFloat8    m_f8Value;
                tCIDLib::TInt4      m_i4Value;
                TNode*              m_pnodeLeft;
                TNode*              m_pnodeRight;
        };


        // -------------------------------------------------------------------
        //  Hidden Constructor
        // -------------------------------------------------------------------
        TCQSLLDMath();


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCQLogicSh::EEvalRes eBuildValue
        (
            const   tCQLogicSh::TInfoList&  colVals
            ,       TCQCFldValue&           fldvToFill
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        )   final;

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
        tCIDLib::TBoolean bParseOp
        (
                    TTextStringInStream&    strmSrc
            ,       TNode::ENodeTypes&      eToFill
            , const tCIDLib::TBoolean       bThrowIfNot
        );

        tCIDLib::TBoolean bCheckNextChar
        (
                    TTextStringInStream&    strmSrc
            , const tCIDLib::TCh            chToCheck
            , const tCIDLib::TBoolean       bThrowIfNot = kCIDLib::True
        );

        tCIDLib::TCh chGetDigit
        (
                    TTextStringInStream&    strmSrc
        );

        tCIDLib::TCh chGetNext
        (
                    TTextStringInStream&    strmSrc
            , const tCIDLib::TBoolean       bEndOk = kCIDLib::False
        );

        tCIDLib::TCh chPeekNext
        (
                    TTextStringInStream&    strmSrc
        );

        [[nodiscard]] TNode* pnodeParseClause
        (
                    TTextStringInStream&    strmSrc
            , const tCIDLib::TBoolean       bCanBeFactor = kCIDLib::False
        );

        [[nodiscard]] TNode* pnodeParseFactor
        (
                    TTextStringInStream&    strmSrc
        );

        [[nodiscard]] TNode* pnodeParseFunc
        (
                    TTextStringInStream&    strmSrc
        );

        tCIDLib::TVoid ParseIdent
        (
                    TTextStringInStream&    strmSrc
            ,       TString&                strToFill
        );

        tCIDLib::TVoid SkipSpace
        (
                    TTextStringInStream&    strmSrc
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_chPush
        //      We need a place to push back a character to be seen upstream
        //      after returning back from a method. If null, nothing has been
        //      pushed back. chNext() will check this first and return it, else
        //      get another.
        //
        //  m_pnodeRoot
        //      The root of our expression tree. It represents the top-most
        //      clause.
        //
        //  m_strFormula
        //      The formula string that we parse to build our tree. This is
        //      the only persisted value.
        //
        //  m_strTmp
        //      For tmp use, parsing out stuff
        // -------------------------------------------------------------------
        tCIDLib::TCh    m_chPush;
        TNode*          m_pnodeRoot;
        TString         m_strFormula;
        TString         m_strTmp;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDMath,TCQLSrvFldType)
        DefPolyDup(TCQSLLDMath)
        BefriendFactory(TCQSLLDMath)
};


#pragma CIDLIB_POPPACK

