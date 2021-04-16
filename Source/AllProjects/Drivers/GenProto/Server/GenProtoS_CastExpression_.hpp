//
// FILE NAME: GenProtoS_CastExpression_.hpp
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
//  This is the header file for the GenProtoS_CastExpression.cpp file, which
//  defines an expression node that will cast its single child expression's
//  value to another type.
//
//  If the child node's value cannot be cast to the cast node's type, a
//  runtime exception will be thrown. There is no compile time check, but
//  we can issue warnings if possible overflows might occur.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCastNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoCastNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoCastNode
        (
                    TGenProtoExprNode* const pnodeToCast
            , const tGenProtoS::ETypes      eToType
        );

        TGenProtoCastNode
        (
                    TGenProtoExprNode* const pnodeToCast
            , const tGenProtoS::ETokens     eToToken
        );

        TGenProtoCastNode
        (
            const   TGenProtoCastNode&      nodeSrc
        );

        ~TGenProtoCastNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoCastNode& operator=
        (
            const   TGenProtoCastNode&      nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid Optimize
        (
                    TGenProtoCtx&           ctxThis
        )  final;

        tCIDLib::TVoid PostParseValidation()  final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pnodeToCast
        //      Our child node whose value we are to cast. In must be one of
        //      the unsigned types. The type we cast to is passed on to our
        //      parent class, i.e. its our node's return type.
        // -------------------------------------------------------------------
        TGenProtoExprNode*  m_pnodeToCast;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoCastNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoCastNode)
};

#pragma CIDLIB_POPPACK



