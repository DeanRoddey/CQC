//
// FILE NAME: GenProtoS_ConstExpression.cpp
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
//  This file implements the basic derivative of the expression node class
//  that represents a simple constant value.
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
RTTIDecls(TGenProtoConstNode, TCQCSerExprNode)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoConstNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoConstNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoConstNode::TGenProtoConstNode(const TGenProtoExprVal& evalInit) :

    TGenProtoExprNode(evalInit.eType(), L"Const Value")
{
    //
    //  Just go ahead and set our value now from the passed initial value.
    //  And from here on out, our Evaluate() method doesn't have to do
    //  anything.
    //
    evalCur().SetFrom(evalInit);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TBoolean bVal) :

    TGenProtoExprNode(tGenProtoS::ETypes::Boolean, L"Const Value")
{
    evalCur().bValue(bVal);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TCard1 c1Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Card1, L"Const Value")
{
    evalCur().c1Value(c1Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TCard2 c2Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Card2, L"Const Value")
{
    evalCur().c2Value(c2Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TCard4 c4Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Card4, L"Const Value")
{
    evalCur().c4Value(c4Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TFloat4 f4Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Float4, L"Const Value")
{
    evalCur().f4Value(f4Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TFloat8 f8Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Float8, L"Const Value")
{
    evalCur().f8Value(f8Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TInt1 i1Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Int1, L"Const Value")
{
    evalCur().i1Value(i1Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TInt2 i2Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Int2, L"Const Value")
{
    evalCur().i2Value(i2Val);
}

TGenProtoConstNode::TGenProtoConstNode(const tCIDLib::TInt4 i4Val) :

    TGenProtoExprNode(tGenProtoS::ETypes::Int4, L"Const Value")
{
    evalCur().i4Value(i4Val);
}

TGenProtoConstNode::TGenProtoConstNode(const TString& strVal) :

    TGenProtoExprNode(tGenProtoS::ETypes::String, L"Const Value")
{
    evalCur().strValue(strVal);
}

TGenProtoConstNode::TGenProtoConstNode(const TGenProtoConstNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
{
}

TGenProtoConstNode::~TGenProtoConstNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoConstNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoConstNode&
TGenProtoConstNode::operator=(const TGenProtoConstNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoConstNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoConstNode::bIsConst() const
{
    // Clearly we are always const
    return kCIDLib::True;
}


tCIDLib::TVoid TGenProtoConstNode::Evaluate(TGenProtoCtx&)
{
    //
    //  We don't have to do anything, since we set our const value in the
    //  ctor, and it stays that way forever.
    //
}



