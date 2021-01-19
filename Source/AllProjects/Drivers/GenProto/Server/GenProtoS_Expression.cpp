//
// FILE NAME: GenProtoS_Expression.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/23/2003
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
//  This file implements the basic expression classes.
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
RTTIDecls(TGenProtoExprNode,TObject)
RTTIDecls(TGenProtoArbChildNode,TGenProtoExprNode)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExprNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoExprNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExprNode::~TGenProtoExprNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoExprNode: Public, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoExprNode::Optimize(TGenProtoCtx& ctxToUse)
{
    // A purposefully empty default implementation
}


tCIDLib::TVoid TGenProtoExprNode::PostParseValidation()
{
    // A purposefully empty default implementation
}


// ---------------------------------------------------------------------------
//  TGenProtoExprNode: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoExprNode::bIsCardType() const
{
    // Just a convenience passthrough to the value object
    return m_evalCur.bIsCardType();
}


tCIDLib::TBoolean TGenProtoExprNode::bIsCharType() const
{
    // Just a convenience passthrough to the value object
    return m_evalCur.bIsCharType();
}


tCIDLib::TBoolean TGenProtoExprNode::bIsFloatType() const
{
    // Just a convenience passthrough to the value object
    return m_evalCur.bIsFloatType();
}


tCIDLib::TBoolean TGenProtoExprNode::bIsIntType() const
{
    // Just a convenience passthrough to the value object
    return m_evalCur.bIsIntType();
}


const TGenProtoExprVal& TGenProtoExprNode::evalCur() const
{
    return m_evalCur;
}

TGenProtoExprVal& TGenProtoExprNode::evalCur()
{
    return m_evalCur;
}


tGenProtoS::ETypes TGenProtoExprNode::eType() const
{
    // Just a convenience method that gets the type from the value object
    return m_evalCur.eType();
}


const TString& TGenProtoExprNode::strDescription() const
{
    return m_strDescr;
}


// ---------------------------------------------------------------------------
//  TGenProtoExprNode: Hidden constructors and operators
// ---------------------------------------------------------------------------
TGenProtoExprNode::TGenProtoExprNode(const  tGenProtoS::ETypes  eType
                                    , const TString&            strDescr) :
    m_evalCur(eType)
    , m_strDescr(strDescr)
{
}

TGenProtoExprNode::TGenProtoExprNode(const TGenProtoExprNode& nodeSrc) :

    m_evalCur(nodeSrc.m_evalCur)
    , m_strDescr(nodeSrc.m_strDescr)
{
}

TGenProtoExprNode&
TGenProtoExprNode::operator=(const TGenProtoExprNode& nodeSrc)
{
    if (this != &nodeSrc)
    {
        m_evalCur   = nodeSrc.m_evalCur;
        m_strDescr  = nodeSrc.m_strDescr;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoExprNode: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoExprNode::CheckIsBoolType( const   TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if (eCheck != tGenProtoS::ETypes::Boolean)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeBoolean
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}

tCIDLib::TVoid
TGenProtoExprNode::CheckIsCardType( const   TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if ((eCheck != tGenProtoS::ETypes::Card1)
    &&  (eCheck != tGenProtoS::ETypes::Card2)
    &&  (eCheck != tGenProtoS::ETypes::Card4))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeCardinal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckIsCharType( const   TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    // It has to be a string type, with a length of one
    if ((eCheck != tGenProtoS::ETypes::String)
    ||  (nodeToCheck.evalCur().strValue().c4Length() != 1))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeChar
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckIsConst(const   TGenProtoExprNode&  nodeToCheck
                                , const tCIDLib::TCard4     c4ParmNum)
{
    if (!nodeToCheck.bIsConst())
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeConst
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}



tCIDLib::TVoid
TGenProtoExprNode::CheckIsFloatType(const   TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if ((eCheck != tGenProtoS::ETypes::Float4)
    &&  (eCheck != tGenProtoS::ETypes::Float8))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeFloat
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckIsIntType(  const   TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if ((eCheck != tGenProtoS::ETypes::Int1)
    &&  (eCheck != tGenProtoS::ETypes::Int2)
    &&  (eCheck != tGenProtoS::ETypes::Int4))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeIntegral
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckIsNumericType(  const   TGenProtoExprNode&  nodeToCheck
                                        , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if ((eCheck != tGenProtoS::ETypes::Card1)
    &&  (eCheck != tGenProtoS::ETypes::Card2)
    &&  (eCheck != tGenProtoS::ETypes::Card4)
    &&  (eCheck != tGenProtoS::ETypes::Float4)
    &&  (eCheck != tGenProtoS::ETypes::Float8)
    &&  (eCheck != tGenProtoS::ETypes::Int1)
    &&  (eCheck != tGenProtoS::ETypes::Int2)
    &&  (eCheck != tGenProtoS::ETypes::Int4))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeNumeric
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckIsStringType(const  TGenProtoExprNode&  nodeToCheck
                                    , const tCIDLib::TCard4     c4ParmNum)
{
    const tGenProtoS::ETypes eCheck = nodeToCheck.eType();

    if (eCheck != tGenProtoS::ETypes::String)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_MustBeString
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
        );
    }
}


tCIDLib::TVoid
TGenProtoExprNode::CheckType(const  TGenProtoExprNode&  nodeToCheck
                            , const tGenProtoS::ETypes   eExpectedType
                            , const tCIDLib::TCard4     c4ParmNum)
{
    if (nodeToCheck.eType() != eExpectedType)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_BadParmType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(c4ParmNum)
            , m_strDescr
            , TFacGenProtoS::strXlatType(eExpectedType)
        );
    }
}


TGenProtoExprNode*
TGenProtoExprNode::pnodeOptimize(       TGenProtoCtx&            ctxToUse
                                ,       TGenProtoExprNode* const pnodeToOptimize
                                , const tCIDLib::TBoolean        bDeleteOrg)
{
    const tCIDLib::TBoolean bAlreadyConst =
    (
        pnodeToOptimize->bIsA(TGenProtoConstNode::clsThis())
    );

    //
    //  If the node is non-const, we can't do anything. However, it might
    //  have some children that are optimizable, so recurse if its not a
    //  const expression value already.
    //
    TGenProtoExprNode* pnodeRet = pnodeToOptimize;
    if (!pnodeToOptimize->bIsConst())
    {
        if (!bAlreadyConst)
            pnodeToOptimize->Optimize(ctxToUse);
    }
     else
    {
        //
        //  It is const, so see if it is already a const expression node.
        //  If not, then evaluate it and create a const expression node in
        //  its place, deleting the original if requested
        //
        if (!bAlreadyConst)
        {
            //
            //  Evaluate it so that we can get its value. Then create a new
            //  const expression node with its value/type, and delete the
            //  original.
            //
            pnodeToOptimize->Evaluate(ctxToUse);
            pnodeRet = new TGenProtoConstNode(pnodeToOptimize->evalCur());

            if (bDeleteOrg)
                delete pnodeToOptimize;
        }
    }
    return pnodeRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArbChildNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoArbChildNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoArbChildNode::TGenProtoArbChildNode(const  tGenProtoS::ETypes eType
                                            , const TString&           strDesc) :
    TGenProtoExprNode(eType, strDesc)
    , m_colChildren(tCIDLib::EAdoptOpts::Adopt)
{
}

TGenProtoArbChildNode::
TGenProtoArbChildNode(const TGenProtoArbChildNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_colChildren(tCIDLib::EAdoptOpts::Adopt, nodeToCopy.m_colChildren.c4ElemCount())
{
    const tCIDLib::TCard4 c4Count = nodeToCopy.m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_colChildren.Add
        (
            ::pDupObject<TGenProtoExprNode>(nodeToCopy.m_colChildren[c4Index])
        );
    }
}

TGenProtoArbChildNode::~TGenProtoArbChildNode()
{
    // Flush the children
    m_colChildren.RemoveAll();
}


// ---------------------------------------------------------------------------
//  TGenProtoArbChildNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoArbChildNode&
TGenProtoArbChildNode::operator=(const TGenProtoArbChildNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_colChildren.RemoveAll();

        const tCIDLib::TCard4 c4Count = nodeToAssign.m_colChildren.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_colChildren.Add
            (
                ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_colChildren[c4Index])
            );
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoArbChildNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoArbChildNode::AddChild(TGenProtoExprNode* const pnodeToAdopt)
{
    m_colChildren.Add(pnodeToAdopt);
}


tCIDLib::TBoolean TGenProtoArbChildNode::bIsConst() const
{
    // If all of our children are const, then we are
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!m_colChildren[c4Index]->bIsConst())
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TGenProtoArbChildNode::Optimize(TGenProtoCtx& ctxToUse)
{
    //
    //  Just call the optimizer method one each child. We don't have to
    //  deal with recursion, the helper does that.
    //
    //  This one is a little tricky, since the optimize call will delete
    //  the original node if it optimizes. But they are owned by the
    //  collection, so a double delete would occur.
    //
    const tCIDLib::TCard4 c4Count = m_colChildren.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoExprNode* pnodeOrg = m_colChildren[c4Index];
        TGenProtoExprNode* pnodeOpt = pnodeOptimize
        (
            ctxToUse
            , pnodeOrg
            , kCIDLib::False
        );

        //
        //  If we got a new node back, then it was optimized. We asked it
        //  not to delete the original, because the collection owns it. So
        //  now we just set the new one over the old one, which causes the
        //  old one to be cleaned up.
        //
        if (pnodeOrg != pnodeOpt)
            m_colChildren.SetAt(pnodeOpt, c4Index);
    }
}



// ---------------------------------------------------------------------------
//  TGenProtoArbChildNode: Protected, non-virtual methods
// ---------------------------------------------------------------------------
const TGenProtoArbChildNode::TChildList&
TGenProtoArbChildNode::colChildren() const
{
    return m_colChildren;
}


TGenProtoArbChildNode::TChildList& TGenProtoArbChildNode::colChildren()
{
    return m_colChildren;
}


