//
// FILE NAME: GenProtoS_CastExpression.cpp
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
//  This file implements the simple type casting expression nodes.
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
RTTIDecls(TGenProtoCastNode,TGenProtoExprNode)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace GenProtoS_CastExpression
{
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCastNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCastNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCastNode::
TGenProtoCastNode(          TGenProtoExprNode* const    pnodeToCast
                    , const tGenProtoS::ETypes          eToType) :

    TGenProtoExprNode
    (
        eToType
        , TString(L"To")
          + TGenProtoTokenizer::strTokenName(TFacGenProtoS::eTypeToToken(eToType))
          + TString(L"()")
    )
    , m_pnodeToCast(pnodeToCast)
{
}

TGenProtoCastNode::
TGenProtoCastNode(          TGenProtoExprNode* const    pnodeToCast
                    , const tGenProtoS::ETokens         eToToken) :

    TGenProtoExprNode
    (
        TFacGenProtoS::eTokenToType(eToToken)
        , TString(L"To")
          + TGenProtoTokenizer::strTokenName(eToToken)
          + TString(L"()")
    )
    , m_pnodeToCast(pnodeToCast)
{
}

TGenProtoCastNode::TGenProtoCastNode(const TGenProtoCastNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)

    , m_pnodeToCast(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeToCast))
{
}

TGenProtoCastNode::~TGenProtoCastNode()
{
    // Delete the value child expressions
    delete m_pnodeToCast;
}


// ---------------------------------------------------------------------------
//  TGenProtoCastNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoCastNode&
TGenProtoCastNode::operator=(const TGenProtoCastNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        delete m_pnodeToCast;
        m_pnodeToCast = nullptr;
        m_pnodeToCast = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeToCast);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoCastNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoCastNode::bIsConst() const
{
    // If our child is const, then we are
    return m_pnodeToCast->bIsConst();
}


tCIDLib::TVoid TGenProtoCastNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Ask our child node to evaluate himself
    m_pnodeToCast->Evaluate(ctxThis);

    //
    //  Set our value to set itself from our child's result, which will have
    //  the result of casting the value as needed.
    //
    evalCur().SetFrom(m_pnodeToCast->evalCur());
}


tCIDLib::TVoid TGenProtoCastNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeToCast = pnodeOptimize(ctxThis, m_pnodeToCast);
}


tCIDLib::TVoid TGenProtoCastNode::PostParseValidation()
{
    // Make sure that we got our one parm
    if (!m_pnodeToCast)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(1)
        );
    }
}



