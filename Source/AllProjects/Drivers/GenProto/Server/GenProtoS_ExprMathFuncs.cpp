//
// FILE NAME: GenProtoS_ExprMathFuncs.cpp
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
//  This file implements the math related expression functions
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
RTTIDecls(TGenProtoAddNode,TGenProtoExprNode)
RTTIDecls(TGenProtoDivNode,TGenProtoExprNode)
RTTIDecls(TGenProtoEqualsNode,TGenProtoExprNode)
RTTIDecls(TGenProtoMulNode,TGenProtoExprNode)
RTTIDecls(TGenProtoNumMagNode,TGenProtoExprNode)
RTTIDecls(TGenProtoRangeRotNode,TGenProtoExprNode)
RTTIDecls(TGenProtoRoundFloatNode,TGenProtoExprNode)
RTTIDecls(TGenProtoScaleRngNode,TGenProtoExprNode)
RTTIDecls(TGenProtoSubNode,TGenProtoExprNode)
RTTIDecls(TGenProtoTranscNode,TGenProtoExprNode)



// ---------------------------------------------------------------------------
//  Local functions
// ---------------------------------------------------------------------------
static tGenProtoS::ETypes
eCalcOpRes( const   TGenProtoExprNode* const m_pnodeLHS
            , const TGenProtoExprNode* const m_pnodeRHS
            , const TGenProtoExprNode* const m_pnodeTarget)
{
    //
    //  Get the larger of the two types. The enum is arranged so that types
    //  with more capacity are greater.
    //
    tGenProtoS::ETypes eRes = m_pnodeLHS->eType();
    if (m_pnodeRHS->evalCur().eType() > eRes)
        eRes = m_pnodeRHS->eType();
    if (m_pnodeTarget && (m_pnodeTarget->eType() > eRes))
        eRes = m_pnodeTarget->eType();

    // Now pick the largest of the category of the largest of the two
    switch(eRes)
    {
        case tGenProtoS::ETypes::Card1 :
        case tGenProtoS::ETypes::Card2 :
        case tGenProtoS::ETypes::Card4 :
            eRes = tGenProtoS::ETypes::Card4;
            break;

        case tGenProtoS::ETypes::Float4 :
        case tGenProtoS::ETypes::Float8 :
            eRes = tGenProtoS::ETypes::Float8;
            break;

        case tGenProtoS::ETypes::Int1 :
        case tGenProtoS::ETypes::Int2 :
        case tGenProtoS::ETypes::Int4 :
            eRes = tGenProtoS::ETypes::Int4;
            break;
    };
    return eRes;
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoAddNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoAddNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoAddNode::TGenProtoAddNode( TGenProtoExprNode* const    pnodeLHS
                                    , TGenProtoExprNode* const  pnodeRHS) :

    TGenProtoExprNode(eCalcOpRes(pnodeLHS, pnodeRHS, 0), L"Add()")
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoAddNode::TGenProtoAddNode(const TGenProtoAddNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoAddNode::~TGenProtoAddNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoAddNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoAddNode&
TGenProtoAddNode::operator=(const TGenProtoAddNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoAddNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoAddNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoAddNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    //
    //  Determine the resolution at which to do the operation. It will be the
    //  highest capacity type of the two source nodes or our type.
    //
    const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

    switch(eDoAt)
    {
        case tGenProtoS::ETypes::Int4 :
            evalCur().i4Value
            (
                m_pnodeLHS->evalCur().i4Value() + m_pnodeRHS->evalCur().i4Value()
            );
            break;

        case tGenProtoS::ETypes::Card4 :
            evalCur().c4Value
            (
                m_pnodeLHS->evalCur().c4Value() + m_pnodeRHS->evalCur().c4Value()
            );
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value
            (
                m_pnodeLHS->evalCur().f8Value() + m_pnodeRHS->evalCur().f8Value()
            );
            break;
    };
}


tCIDLib::TVoid TGenProtoAddNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoAddNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They must be of numeric types
    CheckIsNumericType(*m_pnodeLHS, 1);
    CheckIsNumericType(*m_pnodeRHS, 2);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoDivNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoDivNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoDivNode::TGenProtoDivNode( TGenProtoExprNode* const    pnodeLHS
                                    , TGenProtoExprNode* const  pnodeRHS) :

    TGenProtoExprNode(eCalcOpRes(pnodeLHS, pnodeRHS, 0), L"Div()")
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoDivNode::TGenProtoDivNode(const TGenProtoDivNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoDivNode::~TGenProtoDivNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoDivNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoDivNode&
TGenProtoDivNode::operator=(const TGenProtoDivNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoDivNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoDivNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoDivNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    //
    //  Determine the resolution at which to do the operation. It will be the
    //  highest capacity type of the two source nodes or our type.
    //
    const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

    switch(eDoAt)
    {
        case tGenProtoS::ETypes::Int4 :
            evalCur().i4Value
            (
                m_pnodeLHS->evalCur().i4Value() / m_pnodeRHS->evalCur().i4Value()
            );
            break;

        case tGenProtoS::ETypes::Card4 :
            evalCur().c4Value
            (
                m_pnodeLHS->evalCur().c4Value() / m_pnodeRHS->evalCur().c4Value()
            );
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value
            (
                m_pnodeLHS->evalCur().f8Value() / m_pnodeRHS->evalCur().f8Value()
            );
            break;
    };
}


tCIDLib::TVoid TGenProtoDivNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoDivNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They both must be nureric types
    CheckIsNumericType(*m_pnodeLHS, 1);
    CheckIsNumericType(*m_pnodeRHS, 2);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoEqualsNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoEqualsNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoEqualsNode::TGenProtoEqualsNode(TGenProtoExprNode* const   pnodeLHS
                                        , TGenProtoExprNode* const  pnodeRHS) :

    TGenProtoExprNode(tGenProtoS::ETypes::Boolean, L"Equals()")
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoEqualsNode::TGenProtoEqualsNode(const TGenProtoEqualsNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoEqualsNode::~TGenProtoEqualsNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoEqualsNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoEqualsNode&
TGenProtoEqualsNode::operator=(const TGenProtoEqualsNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoEqualsNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoEqualsNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoEqualsNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    // Special case for strings. In this case they both have to be strings
    if (m_pnodeLHS->evalCur().eType() == tGenProtoS::ETypes::String)
    {
        evalCur().bValue
        (
            m_pnodeLHS->evalCur().strValue() == m_pnodeRHS->evalCur().strValue()
        );
    }
     else
    {
        //
        //  Determine the resolution at which to do the operation. It will be the
        //  highest capacity type of the two source nodes or our type.
        //
        const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

        switch(eDoAt)
        {
          case tGenProtoS::ETypes::Int4 :
            evalCur().bValue
            (
                m_pnodeLHS->evalCur().i4Value() == m_pnodeRHS->evalCur().i4Value()
            );
            break;

          case tGenProtoS::ETypes::Card4 :
            evalCur().bValue
            (
                m_pnodeLHS->evalCur().c4Value() == m_pnodeRHS->evalCur().c4Value()
            );
            break;

          case tGenProtoS::ETypes::Float8 :
            evalCur().bValue
            (
                m_pnodeLHS->evalCur().f8Value() == m_pnodeRHS->evalCur().f8Value()
            );
            break;
        };
    }
}


tCIDLib::TVoid TGenProtoEqualsNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoEqualsNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They both either must be numeric type, or both must be string type.
    const tGenProtoS::ETypes eLHS = m_pnodeLHS->evalCur().eType();
    const tGenProtoS::ETypes eRHS = m_pnodeRHS->evalCur().eType();

    if ((eLHS == tGenProtoS::ETypes::String)
    ||  (eRHS == tGenProtoS::ETypes::String))
    {
        if (eLHS != eRHS)
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcParse_CantCompare
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::TypeMatch
            );
        }
    }
     else
    {
        CheckIsNumericType(*m_pnodeLHS, 1);
        CheckIsNumericType(*m_pnodeRHS, 2);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMulNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMulNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoMulNode::TGenProtoMulNode( TGenProtoExprNode* const    pnodeLHS
                                    , TGenProtoExprNode* const  pnodeRHS) :

    TGenProtoExprNode(eCalcOpRes(pnodeLHS, pnodeRHS, 0), L"Mul()")
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoMulNode::TGenProtoMulNode(const TGenProtoMulNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoMulNode::~TGenProtoMulNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoMulNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoMulNode& TGenProtoMulNode::operator=(const TGenProtoMulNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoMulNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoMulNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoMulNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    //
    //  Determine the resolution at which to do the operation. It will be the
    //  highest capacity type of the two source nodes or our type.
    //
    const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

    switch(eDoAt)
    {
        case tGenProtoS::ETypes::Int4 :
            evalCur().i4Value
            (
                m_pnodeLHS->evalCur().i4Value() * m_pnodeRHS->evalCur().i4Value()
            );
            break;

        case tGenProtoS::ETypes::Card4 :
            evalCur().c4Value
            (
                m_pnodeLHS->evalCur().c4Value() * m_pnodeRHS->evalCur().c4Value()
            );
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value
            (
                m_pnodeLHS->evalCur().f8Value() * m_pnodeRHS->evalCur().f8Value()
            );
            break;
    };
}


tCIDLib::TVoid TGenProtoMulNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoMulNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They both must be numeric types
    CheckIsNumericType(*m_pnodeLHS, 1);
    CheckIsNumericType(*m_pnodeRHS, 2);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoNumMagNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoNumMagNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoNumMagNode::TGenProtoNumMagNode(TGenProtoExprNode* const   pnodeLHS
                                        , TGenProtoExprNode* const  pnodeRHS
                                        , const tCIDLib::TBoolean   bGreater) :
    TGenProtoExprNode
    (
        tGenProtoS::ETypes::Boolean
        , (bGreater ? L"GreaterThan()" : L"LessThan()")
    )
    , m_bGreater(bGreater)
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoNumMagNode::TGenProtoNumMagNode(const TGenProtoNumMagNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_bGreater(nodeToCopy.m_bGreater)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoNumMagNode::~TGenProtoNumMagNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoNumMagNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoNumMagNode&
TGenProtoNumMagNode::operator=(const TGenProtoNumMagNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_bGreater = nodeToAssign.m_bGreater;
        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoNumMagNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoNumMagNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoNumMagNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    //
    //  Determine the resolution at which to do the operation. It will be the
    //  highest capacity type of the two source nodes or our type.
    //
    const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

    switch(eDoAt)
    {
        case tGenProtoS::ETypes::Int4 :
            if (m_bGreater)
                evalCur().bValue(m_pnodeLHS->evalCur().i4Value() > m_pnodeRHS->evalCur().i4Value());
            else
                evalCur().bValue(m_pnodeLHS->evalCur().i4Value() < m_pnodeRHS->evalCur().i4Value());
            break;

        case tGenProtoS::ETypes::Card4 :
            if (m_bGreater)
                evalCur().bValue(m_pnodeLHS->evalCur().c4Value() > m_pnodeRHS->evalCur().c4Value());
            else
                evalCur().bValue(m_pnodeLHS->evalCur().c4Value() < m_pnodeRHS->evalCur().c4Value());
            break;

        case tGenProtoS::ETypes::Float8 :
            if (m_bGreater)
                evalCur().bValue(m_pnodeLHS->evalCur().f8Value() > m_pnodeRHS->evalCur().f8Value());
            else
                evalCur().bValue(m_pnodeLHS->evalCur().f8Value() < m_pnodeRHS->evalCur().f8Value());
            break;
    };
}


tCIDLib::TVoid TGenProtoNumMagNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoNumMagNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They must be of numeric types
    CheckIsNumericType(*m_pnodeLHS, 1);
    CheckIsNumericType(*m_pnodeRHS, 2);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoRangeRotNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoRangeRotNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoRangeRotNode::
TGenProtoRangeRotNode(  TGenProtoExprNode* const    pnodeOrgVal
                        , TGenProtoExprNode* const  pnodeDirection
                        , TGenProtoExprNode* const  pnodeRangeLow
                        , TGenProtoExprNode* const  pnodeRangeHigh
                        , TGenProtoExprNode* const  pnodeWrapAround) :

    TGenProtoExprNode(pnodeOrgVal->eType(), L"RangeRotate()")
    , m_pnodeDirection(pnodeDirection)
    , m_pnodeOrgVal(pnodeOrgVal)
    , m_pnodeRangeHigh(pnodeRangeHigh)
    , m_pnodeRangeLow(pnodeRangeLow)
    , m_pnodeWrapAround(pnodeWrapAround)
{
}

TGenProtoRangeRotNode::
TGenProtoRangeRotNode(const TGenProtoRangeRotNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeDirection(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeDirection))
    , m_pnodeOrgVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeOrgVal))
    , m_pnodeRangeHigh(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRangeHigh))
    , m_pnodeRangeLow(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRangeLow))
    , m_pnodeWrapAround(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeWrapAround))
{
}

TGenProtoRangeRotNode::~TGenProtoRangeRotNode()
{
    // Delete the children
    delete m_pnodeDirection;
    delete m_pnodeOrgVal;
    delete m_pnodeRangeHigh;
    delete m_pnodeRangeLow;
    delete m_pnodeWrapAround;
}


// ---------------------------------------------------------------------------
//  TGenProtoRangeRotNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoRangeRotNode&
TGenProtoRangeRotNode::operator=(const TGenProtoRangeRotNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeDirection;
        m_pnodeDirection = 0;
        delete m_pnodeOrgVal;
        m_pnodeOrgVal = 0;
        delete m_pnodeRangeHigh;
        m_pnodeRangeLow = 0;
        delete m_pnodeWrapAround;
        m_pnodeWrapAround= 0;

        m_pnodeDirection  = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeDirection);
        m_pnodeOrgVal     = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeOrgVal);
        m_pnodeRangeHigh  = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRangeHigh);
        m_pnodeRangeLow   = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRangeLow);
        m_pnodeWrapAround = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeWrapAround);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoRangeRotNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoRangeRotNode::bIsConst() const
{
    // If all our children are const, then we will be const
    return  m_pnodeDirection->bIsConst()
            && m_pnodeOrgVal->bIsConst()
            && m_pnodeRangeHigh->bIsConst()
            && m_pnodeRangeLow->bIsConst()
            && m_pnodeWrapAround->bIsConst();
}


tCIDLib::TVoid TGenProtoRangeRotNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the child nodes
    m_pnodeOrgVal->Evaluate(ctxThis);
    m_pnodeDirection->Evaluate(ctxThis);
    m_pnodeRangeLow->Evaluate(ctxThis);
    m_pnodeRangeHigh->Evaluate(ctxThis);
    m_pnodeWrapAround->Evaluate(ctxThis);

    // The wrap around flag is the same for both scenarios
    const tCIDLib::TBoolean bWrap = m_pnodeWrapAround->evalCur().bValue();

    if (m_pnodeOrgVal->eType() == tGenProtoS::ETypes::Int4)
    {
        tCIDLib::TInt4 i4New = m_pnodeOrgVal->evalCur().i4Value();
        const tCIDLib::TInt4 i4Low = m_pnodeRangeLow->evalCur().i4Value();
        const tCIDLib::TInt4 i4High = m_pnodeRangeHigh->evalCur().i4Value();

        // Has to be in the range
        if ((i4New < i4Low) || (i4New > i4High))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_OutofRange
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , TInteger(i4New)
                , TInteger(i4Low)
                , TInteger(i4High)
            );
        }

        if (m_pnodeDirection->evalCur().bValue())
        {
            if (i4New == i4High)
            {
                if (bWrap)
                    i4New = i4Low;
            }
             else
            {
                i4New++;
            }
        }
         else
        {
            if (i4New == i4Low)
            {
                if (bWrap)
                    i4New = i4High;
            }
             else
            {
                i4New--;
            }
        }

        evalCur().i4Value(i4New);
    }
     else if (m_pnodeOrgVal->eType() == tGenProtoS::ETypes::Card4)
    {
        tCIDLib::TCard4 c4New = m_pnodeOrgVal->evalCur().c4Value();
        const tCIDLib::TCard4 c4Low = m_pnodeRangeLow->evalCur().c4Value();
        const tCIDLib::TCard4 c4High = m_pnodeRangeHigh->evalCur().c4Value();

        // Has to be in the range
        if ((c4New < c4Low) || (c4New > c4High))
        {
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_OutofRange
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Range
                , TCardinal(c4New)
                , TCardinal(c4Low)
                , TCardinal(c4High)
            );
        }

        if (m_pnodeDirection->evalCur().bValue())
        {
            if (c4New == c4High)
            {
                if (bWrap)
                    c4New = c4Low;
            }
             else
            {
                c4New++;
            }
        }
         else
        {
            if (c4New == c4Low)
            {
                if (bWrap)
                    c4New = c4High;
            }
             else
            {
                c4New--;
            }
        }

        evalCur().c4Value(c4New);
    }
     else
    {
        // Shouldn't happen since the validation checked, but just in case
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_RangeRotTypes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }
}


tCIDLib::TVoid TGenProtoRangeRotNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeDirection = pnodeOptimize(ctxThis, m_pnodeDirection);
    m_pnodeOrgVal = pnodeOptimize(ctxThis, m_pnodeOrgVal);
    m_pnodeRangeHigh = pnodeOptimize(ctxThis, m_pnodeRangeHigh);
    m_pnodeRangeLow = pnodeOptimize(ctxThis, m_pnodeRangeLow);
    m_pnodeWrapAround = pnodeOptimize(ctxThis, m_pnodeWrapAround);
}


tCIDLib::TVoid TGenProtoRangeRotNode::PostParseValidation()
{
    // Make sure that we got our four parameters
    if (!m_pnodeOrgVal || !m_pnodeDirection
    ||  !m_pnodeRangeLow || !m_pnodeRangeHigh
    ||  !m_pnodeWrapAround)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(4)
        );
    }

    // The numeric values must all be int or cardinal
    tCIDLib::TBoolean bOk = kCIDLib::True;
    if (m_pnodeOrgVal->bIsIntType())
    {
        bOk = m_pnodeRangeLow->bIsIntType() && m_pnodeRangeHigh->bIsIntType();
    }
     else if (m_pnodeOrgVal->bIsCardType())
    {
        bOk = m_pnodeRangeLow->bIsCardType() && m_pnodeRangeHigh->bIsCardType();
    }
     else
    {
        bOk = kCIDLib::False;
    }

    // The wraparound has to be a boolean
    if (m_pnodeWrapAround->eType() != tGenProtoS::ETypes::Boolean)
        bOk = kCIDLib::False;

    if (!bOk)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_RangeRotTypes
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoRoundFloatNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoRoundFloatNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoRoundFloatNode::
TGenProtoRoundFloatNode(        TGenProtoExprNode* const    pnodeValue
                        , const tCIDLib::ERoundTypes        eType) :

    TGenProtoExprNode(pnodeValue->evalCur().eType(), L"RoundFloat()")
    , m_eType(eType)
    , m_pnodeValue(pnodeValue)
{
}

TGenProtoRoundFloatNode::
TGenProtoRoundFloatNode(const TGenProtoRoundFloatNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eType(nodeToCopy.m_eType)
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeValue))
{
}

TGenProtoRoundFloatNode::~TGenProtoRoundFloatNode()
{
    // Delete the child node
    delete m_pnodeValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoRoundFloatNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoRoundFloatNode&
TGenProtoRoundFloatNode::operator=(const TGenProtoRoundFloatNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeValue;
        m_pnodeValue = 0;

        m_eType = nodeToAssign.m_eType;
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeValue);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoRoundFloatNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoRoundFloatNode::bIsConst() const
{
    // If our child expression is const, then we are
    return m_pnodeValue->bIsConst();
}


tCIDLib::TVoid TGenProtoRoundFloatNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the child node
    m_pnodeValue->Evaluate(ctxThis);

    //
    //  And now get the value, perform the rounding on it, and store it as
    //  our value. Do it at the native resolution of our expression.
    //
    if (evalCur().eType() == tGenProtoS::ETypes::Float4)
    {
        tCIDLib::TFloat4 f4Val = m_pnodeValue->evalCur().f4Value();
        TMathLib::Round(f4Val, m_eType);
        evalCur().f4Value(f4Val);
    }
     else
    {
        tCIDLib::TFloat8 f8Val = m_pnodeValue->evalCur().f8Value();
        TMathLib::Round(f8Val, m_eType);
        evalCur().f8Value(f8Val);
    }
}


tCIDLib::TVoid TGenProtoRoundFloatNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on our child node.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  been optimized.)
    //
    m_pnodeValue = pnodeOptimize(ctxThis, m_pnodeValue);
}


tCIDLib::TVoid TGenProtoRoundFloatNode::PostParseValidation()
{
    // Make sure that we got our value node parm
    if (!m_pnodeValue)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // The source node must be floating point
    CheckIsFloatType(*m_pnodeValue, 1);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoScaleRngNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoScaleRngNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoScaleRngNode::
TGenProtoScaleRngNode(  TGenProtoExprNode* const    pnodeVal
                        , TGenProtoExprNode* const  pnodeMinVal
                        , TGenProtoExprNode* const  pnodeMaxVal
                        , TGenProtoExprNode* const  pnodeMinRng
                        , TGenProtoExprNode* const  pnodeMaxRng) :

    TGenProtoExprNode(pnodeVal->eType(), L"ScaleRng()")
    , m_pnodeVal(pnodeVal)
    , m_pnodeMaxRng(pnodeMaxRng)
    , m_pnodeMinRng(pnodeMinRng)
    , m_pnodeMaxVal(pnodeMaxVal)
    , m_pnodeMinVal(pnodeMinVal)
{
}

TGenProtoScaleRngNode::TGenProtoScaleRngNode(const TGenProtoScaleRngNode& nodeSrc) :

    TGenProtoExprNode(nodeSrc)
    , m_pnodeVal(::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeVal))
    , m_pnodeMaxRng(::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMaxRng))
    , m_pnodeMinRng(::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMinRng))
    , m_pnodeMaxVal(::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMaxVal))
    , m_pnodeMinVal(::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMinVal))
{
}

TGenProtoScaleRngNode::~TGenProtoScaleRngNode()
{
    // Delete the children
    delete m_pnodeVal;
    delete m_pnodeMaxRng;
    delete m_pnodeMinRng;
    delete m_pnodeMaxVal;
    delete m_pnodeMinVal;
}


// ---------------------------------------------------------------------------
//  TGenProtoScaleRngNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoScaleRngNode&
TGenProtoScaleRngNode::operator=(const TGenProtoScaleRngNode& nodeSrc)
{
    if (this != &nodeSrc)
    {
        TParent::operator=(nodeSrc);

        delete m_pnodeVal;
        delete m_pnodeMaxRng;
        delete m_pnodeMinRng;
        delete m_pnodeMaxVal;
        delete m_pnodeMinVal;

        m_pnodeVal = 0;
        m_pnodeMaxRng = 0;
        m_pnodeMinRng = 0;
        m_pnodeMaxVal = 0;
        m_pnodeMinVal = 0;

        m_pnodeVal    = ::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeVal);
        m_pnodeMaxRng = ::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMaxRng);
        m_pnodeMinRng = ::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMinRng);
        m_pnodeMaxVal = ::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMaxVal);
        m_pnodeMinVal = ::pDupObject<TGenProtoExprNode>(nodeSrc.m_pnodeMinVal);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoScaleRngNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoScaleRngNode::bIsConst() const
{
    // If all our chldren are, we are
    return
    (
        m_pnodeVal->bIsConst()
        && m_pnodeMaxRng->bIsConst()
        && m_pnodeMinRng->bIsConst()
        && m_pnodeMaxVal->bIsConst()
        && m_pnodeMinVal->bIsConst()
    );
}


tCIDLib::TVoid TGenProtoScaleRngNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the child nodes
    m_pnodeVal->Evaluate(ctxThis);
    m_pnodeMaxRng->Evaluate(ctxThis);
    m_pnodeMinRng->Evaluate(ctxThis);
    m_pnodeMaxVal->Evaluate(ctxThis);
    m_pnodeMinVal->Evaluate(ctxThis);

    //
    //  We do the calculates in floating point and only store the value in the
    //  source value's resolution at the end.
    //
    tCIDLib::TFloat4 f4Val    = m_pnodeVal->evalCur().f4Value();
    tCIDLib::TFloat4 f4MinRng = m_pnodeMinRng->evalCur().f4Value();
    tCIDLib::TFloat4 f4MaxRng = m_pnodeMaxRng->evalCur().f4Value();
    tCIDLib::TFloat4 f4MinVal = m_pnodeMinVal->evalCur().f4Value();
    tCIDLib::TFloat4 f4MaxVal = m_pnodeMaxVal->evalCur().f4Value();

    // Make sure they are not flipped
    if (f4MaxRng < f4MinRng)
    {
        const tCIDLib::TFloat4 f4Tmp = f4MinRng;
        f4MinRng = f4MaxRng;
        f4MaxRng = f4Tmp;
    }

    if (f4MaxVal < f4MinVal)
    {
        const tCIDLib::TFloat4 f4Tmp = f4MinVal;
        f4MinVal = f4MaxVal;
        f4MaxVal = f4Tmp;
    }

    tCIDLib::TFloat4 f4RngPer = 0;
    if ((f4Val >= f4MinVal) && (f4Val <= f4MaxVal))
        f4RngPer = f4Val / (f4MaxVal - f4MinVal);

    // Calculate the position of the new value in in the target range range
    tCIDLib::TFloat4 f4New = (f4MaxRng - f4MinRng) * f4RngPer;
    f4New += f4MinRng;

    // Clip if needed
    if (f4New < f4MinRng)
        f4New = f4MinRng;
    else if (f4New > f4MaxRng)
        f4New = f4MaxRng;

    //
    //  Determine the resolution and store the value in that format.
    //
    switch(m_pnodeVal->eType())
    {
        case tGenProtoS::ETypes::Int4 :
            TMathLib::Round(f4New, tCIDLib::ERoundTypes::Down);
            evalCur().i4Value(tCIDLib::TInt4(f4New));
            break;

        case tGenProtoS::ETypes::Card4 :
            TMathLib::Round(f4New, tCIDLib::ERoundTypes::Down);
            evalCur().c4Value(tCIDLib::TCard4(f4New));
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value(f4New);
            break;
    };
}


tCIDLib::TVoid TGenProtoScaleRngNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeVal    = pnodeOptimize(ctxThis, m_pnodeVal);
    m_pnodeMaxRng = pnodeOptimize(ctxThis, m_pnodeMaxRng);
    m_pnodeMinRng = pnodeOptimize(ctxThis, m_pnodeMinRng);
    m_pnodeMaxVal = pnodeOptimize(ctxThis, m_pnodeMaxVal);
    m_pnodeMinVal = pnodeOptimize(ctxThis, m_pnodeMinVal);
}


tCIDLib::TVoid TGenProtoScaleRngNode::PostParseValidation()
{
    // Make sure that we got our parameters
    if (!m_pnodeVal
    ||  !m_pnodeMaxRng
    ||  !m_pnodeMinRng
    ||  !m_pnodeMaxVal
    ||  !m_pnodeMinVal)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(5)
        );
    }

    // They all must be numeric types
    CheckIsNumericType(*m_pnodeVal, 1);
    CheckIsNumericType(*m_pnodeMaxRng, 2);
    CheckIsNumericType(*m_pnodeMinRng, 3);
    CheckIsNumericType(*m_pnodeMaxVal, 4);
    CheckIsNumericType(*m_pnodeMinVal, 5);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSubNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoSubNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoSubNode::TGenProtoSubNode( TGenProtoExprNode* const    pnodeLHS
                                    , TGenProtoExprNode* const  pnodeRHS) :

    TGenProtoExprNode(eCalcOpRes(pnodeLHS, pnodeRHS, 0), L"Sub()")
    , m_pnodeLHS(pnodeLHS)
    , m_pnodeRHS(pnodeRHS)
{
}

TGenProtoSubNode::TGenProtoSubNode(const TGenProtoSubNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeLHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLHS))
    , m_pnodeRHS(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeRHS))
{
}

TGenProtoSubNode::~TGenProtoSubNode()
{
    // Delete the children
    delete m_pnodeLHS;
    delete m_pnodeRHS;
}


// ---------------------------------------------------------------------------
//  TGenProtoSubNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoSubNode&
TGenProtoSubNode::operator=(const TGenProtoSubNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        delete m_pnodeLHS;
        m_pnodeLHS = 0;
        delete m_pnodeRHS;
        m_pnodeRHS = 0;

        m_pnodeLHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLHS);
        m_pnodeRHS = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeRHS);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoSubNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoSubNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeLHS->bIsConst() && m_pnodeRHS->bIsConst());
}


tCIDLib::TVoid TGenProtoSubNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeLHS->Evaluate(ctxThis);
    m_pnodeRHS->Evaluate(ctxThis);

    //
    //  Determine the resolution at which to do the operation. It will be the
    //  highest capacity type of the two source nodes or our type.
    //
    const tGenProtoS::ETypes eDoAt = eCalcOpRes(m_pnodeLHS, m_pnodeRHS, this);

    switch(eDoAt)
    {
        case tGenProtoS::ETypes::Int4 :
            evalCur().i4Value
            (
                m_pnodeLHS->evalCur().i4Value() - m_pnodeRHS->evalCur().i4Value()
            );
            break;

        case tGenProtoS::ETypes::Card4 :
            evalCur().c4Value
            (
                m_pnodeLHS->evalCur().c4Value() - m_pnodeRHS->evalCur().c4Value()
            );
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value
            (
                m_pnodeLHS->evalCur().f8Value() - m_pnodeRHS->evalCur().f8Value()
            );
            break;
    };
}


tCIDLib::TVoid TGenProtoSubNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeLHS = pnodeOptimize(ctxThis, m_pnodeLHS);
    m_pnodeRHS = pnodeOptimize(ctxThis, m_pnodeRHS);
}


tCIDLib::TVoid TGenProtoSubNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeLHS || !m_pnodeRHS)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // They both must be numeric types
    CheckIsNumericType(*m_pnodeLHS, 1);
    CheckIsNumericType(*m_pnodeRHS, 2);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoTranscNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoTranscNode: Constructors and Destructor
// ---------------------------------------------------------------------------

TGenProtoTranscNode::
TGenProtoTranscNode(TGenProtoExprNode* const    pnodeVal
                    ,TGenProtoExprNode* const   pnodeType) :

    TGenProtoExprNode(pnodeVal->eType(), L"TranscFunc()")
    , m_eType(tGenProtoS::ETransTypes::Count)
    , m_pnodeType(pnodeType)
    , m_pnodeVal(pnodeVal)
{
}

TGenProtoTranscNode::TGenProtoTranscNode(const TGenProtoTranscNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eType(nodeToCopy.m_eType)
    , m_pnodeVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeVal))
    , m_pnodeType(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeType))
{
}

TGenProtoTranscNode::~TGenProtoTranscNode()
{
    // Delete the children
    delete m_pnodeVal;
    delete m_pnodeType;
}


// ---------------------------------------------------------------------------
//  TGenProtoTranscNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoTranscNode&
TGenProtoTranscNode::operator=(const TGenProtoTranscNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_eType = nodeToAssign.m_eType;

        // Delete our nodes and dup the new ones
        delete m_pnodeVal;
        m_pnodeVal = 0;
        delete m_pnodeType;
        m_pnodeType = 0;

        m_pnodeVal = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeVal);
        m_pnodeType = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeType);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoTranscNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoTranscNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeVal->bIsConst() && m_pnodeType->bIsConst());
}


tCIDLib::TVoid TGenProtoTranscNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two child nodes
    m_pnodeVal->Evaluate(ctxThis);
    m_pnodeType->Evaluate(ctxThis);

    // We can do either in Float8, so get the value to that
    tCIDLib::TFloat8 f8Val;
    if (m_pnodeVal->eType() == tGenProtoS::ETypes::Float4)
        f8Val = m_pnodeVal->evalCur().f4Value();
    else
        f8Val = m_pnodeVal->evalCur().f8Value();

    // Based on the type of the function, get the result
    switch(m_eType)
    {
        case tGenProtoS::ETransTypes::ArcCos :
            f8Val = TMathLib::f8ArcCosine(f8Val);
            break;

        case tGenProtoS::ETransTypes::ArcSine :
            f8Val = TMathLib::f8ArcSine(f8Val);
            break;

        case tGenProtoS::ETransTypes::ArcTangent :
            f8Val = TMathLib::f8ArcTangent(f8Val);
            break;

        case tGenProtoS::ETransTypes::Cosine :
            f8Val = TMathLib::f8Cosine(f8Val);
            break;

        case tGenProtoS::ETransTypes::Log :
            f8Val = TMathLib::f8Log(f8Val);
            break;

        case tGenProtoS::ETransTypes::Log10 :
            f8Val = TMathLib::f8Log10(f8Val);
            break;

        case tGenProtoS::ETransTypes::Sine :
            f8Val = TMathLib::f8Sine(f8Val);
            break;

        case tGenProtoS::ETransTypes::Tangent :
            f8Val = TMathLib::f8Tangent(f8Val);
            break;

        default :
            // Something is very wrong

            break;
    };

    // And now store it back as the correct type
    if (m_pnodeVal->eType() == tGenProtoS::ETypes::Float4)
        evalCur().f4Value(tCIDLib::TFloat4(f8Val));
    else
        evalCur().f8Value(f8Val);
}


tCIDLib::TVoid TGenProtoTranscNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeVal = pnodeOptimize(ctxThis, m_pnodeVal);
    m_pnodeType = pnodeOptimize(ctxThis, m_pnodeType);
}


tCIDLib::TVoid TGenProtoTranscNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeVal || !m_pnodeType)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(2)
        );
    }

    // The value must be float4 or float8
    CheckIsFloatType(*m_pnodeVal, 1);

    // The type must be a string and must be constant
    CheckIsStringType(*m_pnodeType, 2);
    CheckIsConst(*m_pnodeType, 2);

    // It is a constant value, so get it and set our enum (or throw)
    const TString& strFunc = m_pnodeType->evalCur().strValue();

    if (strFunc == L"ArcCosine")
        m_eType = tGenProtoS::ETransTypes::ArcCos;
    else if (strFunc == L"ArcSine")
        m_eType = tGenProtoS::ETransTypes::ArcSine;
    else if (strFunc == L"ArcTangent")
        m_eType = tGenProtoS::ETransTypes::ArcTangent;
    else if (strFunc == L"Cosine")
        m_eType = tGenProtoS::ETransTypes::Cosine;
    else if (strFunc == L"Log")
        m_eType = tGenProtoS::ETransTypes::Log;
    else if (strFunc == L"Log10")
        m_eType = tGenProtoS::ETransTypes::Log10;
    else if (strFunc == L"Sine")
        m_eType = tGenProtoS::ETransTypes::Sine;
    else if (strFunc == L"Tangent")
        m_eType = tGenProtoS::ETransTypes::Tangent;
    else
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_BadTransType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strFunc
        );
    }
}



