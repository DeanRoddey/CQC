//
// FILE NAME: GenProtoS_ConstExpression_.hpp
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
//  This is the header file for the GenProtoS_ConstExpression.cpp file. This
//  file provides an expression node derivative that represents a constant
//  value of any type.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TGenProtoConstNode
//  PREFIX: node
// ---------------------------------------------------------------------------
class TGenProtoConstNode : public TGenProtoExprNode
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGenProtoConstNode
        (
            const   TGenProtoExprVal&       evalInit
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TBoolean       bVal
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TCard1         c1Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TCard2         c2Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TCard4         c4Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TFloat4        f4Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TFloat8        f8Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TInt1          i1Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TInt2          i2Val
        );

        TGenProtoConstNode
        (
            const   tCIDLib::TInt4          i4Val
        );

        TGenProtoConstNode
        (
            const   TString&                strVal
        );

        TGenProtoConstNode
        (
            const   TGenProtoConstNode&     nodeSrc
        );

        ~TGenProtoConstNode();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGenProtoConstNode& operator=
        (
            const   TGenProtoConstNode&     nodeSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsConst() const final;

        tCIDLib::TVoid Evaluate
        (
                    TGenProtoCtx&           ctxThis
        ) final;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGenProtoConstNode,TGenProtoExprNode)
        DefPolyDup(TGenProtoConstNode)
};

#pragma CIDLIB_POPPACK



