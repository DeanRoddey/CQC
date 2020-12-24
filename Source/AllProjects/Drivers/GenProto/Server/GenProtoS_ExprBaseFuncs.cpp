//
// FILE NAME: GenProtoS_ExprFunctions.cpp
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
//  This file implements the most common and simple function expressions,
//  mostly for bit fiddling and type conversion.
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
RTTIDecls(TGenProtoArrayLenNode,TGenProtoExprNode)
RTTIDecls(TGenProtoArrayExtractNode,TGenProtoExprNode)
RTTIDecls(TGenProtoBitOpsNode,TGenProtoExprNode)
RTTIDecls(TGenProtoBitsAreSetNode,TGenProtoExprNode)
RTTIDecls(TGenProtoBoolSelNode,TGenProtoExprNode)
RTTIDecls(TGenProtoIfAnyNode,TGenProtoExprNode)
RTTIDecls(TGenProtoIfAllNode,TGenProtoExprNode)
RTTIDecls(TGenProtoIsZeroNode,TGenProtoExprNode)
RTTIDecls(TGenProtoMapFromNode,TGenProtoExprNode)
RTTIDecls(TGenProtoMapToNode,TGenProtoExprNode)
RTTIDecls(TGenProtoNOTNode,TGenProtoExprNode)
RTTIDecls(TGenProtoShiftNode,TGenProtoExprNode)




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArrayExtractNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoArrayExtractNode::
TGenProtoArrayExtractNode(  const   tGenProtoS::ESpecNodes  eExprType
                            ,       TGenProtoExprNode*      pnodeOffset
                            , const tGenProtoS::ETypes      eType) :

    TGenProtoExprNode(eType, L"Extract()")
    , m_eExprType(eExprType)
    , m_pnodeOffset(pnodeOffset)
{
}

TGenProtoArrayExtractNode::
TGenProtoArrayExtractNode(const TGenProtoArrayExtractNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eExprType(nodeToCopy.m_eExprType)
    , m_pnodeOffset(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeOffset))
{
}

TGenProtoArrayExtractNode::~TGenProtoArrayExtractNode()
{
    delete m_pnodeOffset;
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TGenProtoArrayExtractNode& TGenProtoArrayExtractNode::
operator=(const TGenProtoArrayExtractNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TGenProtoExprNode::operator=(nodeToAssign);
        m_eExprType = nodeToAssign.m_eExprType;

        // Clean up existing node before creating the new one
        delete m_pnodeOffset;
        m_pnodeOffset = 0;

        m_pnodeOffset = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeOffset);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoArrayExtractNode::bIsConst() const
{
    // We are never constant
    return kCIDLib::False;
}

tCIDLib::TVoid TGenProtoArrayExtractNode::Evaluate(TGenProtoCtx& ctxThis)
{
    //
    //  The array that we access is in the context object, since it can change
    //  every time we are evaluated. If the memory buffer is not set on the
    //  context, then this expression was used in an illegal place and this
    //  call will throw.
    //
    //  The expression type tells us which one.
    //
    const TMemBuf* pmbufSrc = (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
                              ? &ctxThis.mbufReply()
                              : &ctxThis.mbufSend();

    // Evaluate the offset expression to get the offset to use
    m_pnodeOffset->Evaluate(ctxThis);
    tCIDLib::TCard4 c4Ofs = m_pnodeOffset->evalCur().c4Value();

    //
    //  According to the type we were told to have, extract a value of that
    //  type from the buffer at our given offset. The memory buffer will
    //  throw if this exceeds the available bytes, so watch for that and
    //  convert it into a more application specific exception.
    //
    try
    {
        switch(eType())
        {
            case tGenProtoS::ETypes::Boolean :
                evalCur().bValue(pmbufSrc->bAt(c4Ofs));
                break;

            case tGenProtoS::ETypes::Card1 :
                evalCur().c1Value(pmbufSrc->c1At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Card2 :
                evalCur().c2Value(pmbufSrc->c2At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Card4 :
                evalCur().c4Value(pmbufSrc->c4At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Float4 :
                evalCur().f4Value(pmbufSrc->f4At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Float8 :
                evalCur().f8Value(pmbufSrc->f8At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Int1 :
                evalCur().i1Value(pmbufSrc->i1At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Int2 :
                evalCur().i2Value(pmbufSrc->i2At(c4Ofs));
                break;

            case tGenProtoS::ETypes::Int4 :
                evalCur().i4Value(pmbufSrc->i4At(c4Ofs));
                break;

            default :
                facGenProtoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kGPDErrs::errcExpr_UseStrExtract
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
                break;
        }
    }

    catch(TError& errToCatch)
    {
        if (errToCatch.bCheckEvent(facCIDLib().strName(), kCIDErrs::errcGen_IndexError)
        ||  errToCatch.bCheckEvent(facCIDLib().strName(), kCIDErrs::errcMBuf_BufOverflow))
        {
            // We overflowed the buffer, so translate into our error
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcRunT_BufIndex
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Ofs)
                , TCardinal(ctxThis.c4ReplyBytes())
            );
        }

        // Not one of the buffer overflow errors, so just rethrow as is
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


tCIDLib::TVoid TGenProtoArrayExtractNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeOffset = pnodeOptimize(ctxThis, m_pnodeOffset);
}


tCIDLib::TVoid TGenProtoArrayExtractNode::PostParseValidation()
{
    //
    //  Don't allow our type to be string. They have to use a special
    //  string exact method for that.
    //
    if (eType() == tGenProtoS::ETypes::String)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_UseStrExtract
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppError
        );
    }

    // Make sure our offset expression is a cardinal type
    CheckIsCardType(*m_pnodeOffset, 3);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoArrayLenNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoArrayLenNode::
TGenProtoArrayLenNode(const tGenProtoS::ESpecNodes eExprType) :

    TGenProtoExprNode(tGenProtoS::ETypes::Card4, L"ArrayLen()")
    , m_eExprType(eExprType)
{
}

TGenProtoArrayLenNode::
TGenProtoArrayLenNode(const TGenProtoArrayLenNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eExprType(nodeToCopy.m_eExprType)
{
}

TGenProtoArrayLenNode::~TGenProtoArrayLenNode()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TGenProtoArrayLenNode&
TGenProtoArrayLenNode::operator=(const TGenProtoArrayLenNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_eExprType = nodeToAssign.m_eExprType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoArrayLenNode::bIsConst() const
{
    // We are never constant
    return kCIDLib::False;
}

tCIDLib::TVoid TGenProtoArrayLenNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Just set our value to the length of the appropriate array
    if (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
        evalCur().c4Value(ctxThis.c4ReplyBytes());
    else
        evalCur().c4Value(ctxThis.c4SendBytes());
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBitOpsNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoBitOpsNode: Public, static methods
// ---------------------------------------------------------------------------
const tCIDLib::TCh* TGenProtoBitOpsNode::pszXlatBitOp(const EBitOps eOp)
{
    const tCIDLib::TCh* pszRet;
    switch(eOp)
    {
        case EBitOps::And :
            pszRet = L"ANDBits()";
            break;

        case EBitOps::Or :
            pszRet = L"ORBits()";
            break;

        case EBitOps::Xor :
            pszRet = L"XORBits()";
            break;

        default :
            pszRet = L"???";
            break;
    };
    return pszRet;
}


TGenProtoBitOpsNode::EBitOps
TGenProtoBitOpsNode::eTokenToOp(const tGenProtoS::ETokens eToXlat)
{
    EBitOps eRet;
    switch(eToXlat)
    {
        case tGenProtoS::ETokens::ANDBits :
            eRet = EBitOps::And;
            break;

        case tGenProtoS::ETokens::ORBits :
            eRet = EBitOps::Or;
            break;

        case tGenProtoS::ETokens::XORBits :
            eRet = EBitOps::Xor;
            break;

        default :
            #if CID_DEBUG_ON
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(eToXlat))
                , TString(L"tGenProtoS::ETokens")
            );
            #endif
            break;
    };
    return eRet;
}



// ---------------------------------------------------------------------------
//  TGenProtoBitOpsNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoBitOpsNode::TGenProtoBitOpsNode(const EBitOps eOp) :

    TGenProtoArbChildNode(tGenProtoS::ETypes::Card4, pszXlatBitOp(eOp))
    , m_eOp(eOp)
{
}

TGenProtoBitOpsNode::TGenProtoBitOpsNode(const tGenProtoS::ETokens eToken) :

    TGenProtoArbChildNode
    (
        tGenProtoS::ETypes::Card4
        , pszXlatBitOp(eTokenToOp(eToken))
    )
    , m_eOp(eTokenToOp(eToken))
{
}

TGenProtoBitOpsNode::TGenProtoBitOpsNode(const TGenProtoBitOpsNode& nodeToCopy) :

    TGenProtoArbChildNode(nodeToCopy)
{
}

TGenProtoBitOpsNode::~TGenProtoBitOpsNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoBitOpsNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoBitOpsNode&
TGenProtoBitOpsNode::operator=(const TGenProtoBitOpsNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoBitOpsNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoBitOpsNode::Evaluate(TGenProtoCtx& ctxThis)
{
    //
    //  For each child node in our list, evaluate it, then get its value
    //  as a Card4, and AND it into our return value. We start the value
    //  off with the value of the first child.
    //
    tCIDLib::TCard4 c4Ret = 0;

    TChildList& colList = colChildren();
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoExprNode* pnodeCur = colList[c4Index];

        pnodeCur->Evaluate(ctxThis);

        if (c4Index)
        {
            // After the first child, do the bit op against the return value
            if (m_eOp == EBitOps::And)
                c4Ret &= pnodeCur->evalCur().c4Value();
            else if (m_eOp == EBitOps::Or)
                c4Ret |= pnodeCur->evalCur().c4Value();
            else if (m_eOp == EBitOps::Xor)
                c4Ret ^= pnodeCur->evalCur().c4Value();
            else
            {
                #if CID_DEBUG_ON
                facCIDLib().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCIDErrs::errcGen_BadEnumValue
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , TCardinal(tCIDLib::c4EnumOrd(m_eOp))
                    , TString(L"TGenProtoBitOpsNode::EBitOps")
                );
                #endif
            }
        }
         else
        {
            // This is the first child, so make it the initial value
            if (m_eOp == EBitOps::And)
                c4Ret = pnodeCur->evalCur().c4Value();
            else if (m_eOp == EBitOps::Or)
                c4Ret = pnodeCur->evalCur().c4Value();
            else if (m_eOp == EBitOps::Xor)
                c4Ret = pnodeCur->evalCur().c4Value();
            else
            {
                #if CID_DEBUG_ON
                facCIDLib().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCIDErrs::errcGen_BadEnumValue
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , TCardinal(tCIDLib::c4EnumOrd(m_eOp))
                    , TString(L"TGenProtoBitOpsNode::EBitOps")
                );
                #endif
            }
        }
    }

    // Now set our value from the result
    evalCur().c4Value(c4Ret);
}


tCIDLib::TVoid TGenProtoBitOpsNode::PostParseValidation()
{
    TChildList& colList = colChildren();

    // Make sure that we got at least two parameters
    if (colList.c4ElemCount() < 2)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XOrMoreParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strDescription()
            , TCardinal(2)
        );
    }

    // They all must be of numeric cardinal type
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        CheckIsCardType(*colList[c4Index], c4Index + 1);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBitsAreSetNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoBitsAreSetNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoBitsAreSetNode::
TGenProtoBitsAreSetNode(TGenProtoExprNode* const    pnodeValue
                        , TGenProtoExprNode* const  pnodeMask) :

    TGenProtoExprNode(tGenProtoS::ETypes::Boolean, L"BitsAreSet()")
    , m_pnodeMask(pnodeMask)
    , m_pnodeValue(pnodeValue)
{
}

TGenProtoBitsAreSetNode::
TGenProtoBitsAreSetNode(const TGenProtoBitsAreSetNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeMask(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeMask))
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeValue))
{
}

TGenProtoBitsAreSetNode::~TGenProtoBitsAreSetNode()
{
    // Delete the value child expressions
    delete m_pnodeMask;
    delete m_pnodeValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoBitsAreSetNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoBitsAreSetNode&
TGenProtoBitsAreSetNode::operator=(const TGenProtoBitsAreSetNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        delete m_pnodeMask;
        m_pnodeMask = nullptr;
        delete m_pnodeValue;
        m_pnodeValue = nullptr;

        m_pnodeMask  = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeMask);
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeValue);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoBitsAreSetNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoBitsAreSetNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeValue->bIsConst() && m_pnodeMask->bIsConst());
}


tCIDLib::TVoid TGenProtoBitsAreSetNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evalute the two child nodes
    m_pnodeMask->Evaluate(ctxThis);
    m_pnodeValue->Evaluate(ctxThis);

    // Get the mask out for convenience, as the largest unsigned type
    const tCIDLib::TCard4 c4Mask = m_pnodeMask->evalCur().c4Value();

    //
    //  And now set our boolean value according to whether the indicated
    //  bits are all set.
    //
    evalCur().bValue((m_pnodeValue->evalCur().c4Value() & c4Mask) == c4Mask);
}


tCIDLib::TVoid TGenProtoBitsAreSetNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeMask = pnodeOptimize(ctxThis, m_pnodeMask);
    m_pnodeValue = pnodeOptimize(ctxThis, m_pnodeValue);
}


tCIDLib::TVoid TGenProtoBitsAreSetNode::PostParseValidation()
{
    // Make sure that we got our required parameters
    if (!m_pnodeValue || !m_pnodeMask)
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

    // They must both be cardinal
    CheckIsCardType(*m_pnodeValue, 1);
    CheckIsCardType(*m_pnodeMask, 2);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoBoolSelNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoBoolSelNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoBoolSelNode::
TGenProtoBoolSelNode(TGenProtoExprNode* const   pnodeSelVal
                    , TGenProtoExprNode* const  pnodeTrueVal
                    , TGenProtoExprNode* const  pnodeFalseVal) :

    TGenProtoExprNode(pnodeTrueVal->evalCur().eType(), L"BoolSel()")
    , m_pnodeFalseVal(pnodeFalseVal)
    , m_pnodeSelVal(pnodeSelVal)
    , m_pnodeTrueVal(pnodeTrueVal)
{
}

TGenProtoBoolSelNode::
TGenProtoBoolSelNode(const TGenProtoBoolSelNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeFalseVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeFalseVal))
    , m_pnodeSelVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeSelVal))
    , m_pnodeTrueVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeTrueVal))
{
}

TGenProtoBoolSelNode::~TGenProtoBoolSelNode()
{
    // Delete the child expressions
    delete m_pnodeFalseVal;
    delete m_pnodeSelVal;
    delete m_pnodeTrueVal;
}


// ---------------------------------------------------------------------------
//  TGenProtoBoolSelNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoBoolSelNode&
TGenProtoBoolSelNode::operator=(const TGenProtoBoolSelNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        delete m_pnodeFalseVal;
        m_pnodeFalseVal = nullptr;
        delete m_pnodeSelVal;
        m_pnodeSelVal = nullptr;
        delete m_pnodeTrueVal;
        m_pnodeTrueVal = nullptr;

        m_pnodeFalseVal  = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeFalseVal);
        m_pnodeSelVal = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeSelVal);
        m_pnodeTrueVal = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeTrueVal);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoBoolSelNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoBoolSelNode::bIsConst() const
{
    // If all our children are const, then we are
    return (m_pnodeFalseVal->bIsConst()
    &&      m_pnodeSelVal->bIsConst()
    &&      m_pnodeTrueVal->bIsConst());
}


tCIDLib::TVoid TGenProtoBoolSelNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the selection value node
    m_pnodeSelVal->Evaluate(ctxThis);

    //
    //  According to whether its true or false, evalute the true or false
    //  value node and set our value from it's value.
    //
    if (m_pnodeSelVal->evalCur().bValue())
    {
        m_pnodeTrueVal->Evaluate(ctxThis);
        evalCur().SetFrom(m_pnodeTrueVal->evalCur());
    }
     else
    {
        m_pnodeFalseVal->Evaluate(ctxThis);
        evalCur().SetFrom(m_pnodeFalseVal->evalCur());
    }
}


tCIDLib::TVoid TGenProtoBoolSelNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeFalseVal = pnodeOptimize(ctxThis, m_pnodeFalseVal);
    m_pnodeSelVal   = pnodeOptimize(ctxThis, m_pnodeSelVal);
    m_pnodeTrueVal  = pnodeOptimize(ctxThis, m_pnodeTrueVal);
}


tCIDLib::TVoid TGenProtoBoolSelNode::PostParseValidation()
{
    // Make sure that we got our required parameters
    if (!m_pnodeFalseVal || !m_pnodeSelVal || !m_pnodeTrueVal)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strDescription()
            , TCardinal(3)
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIfAllNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoIfAllNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoIfAllNode::TGenProtoIfAllNode() :

    TGenProtoArbChildNode(tGenProtoS::ETypes::Boolean, L"IfAll()")
{
}

TGenProtoIfAllNode::TGenProtoIfAllNode(const TGenProtoIfAllNode& nodeToCopy) :

    TGenProtoArbChildNode(nodeToCopy)
{
}

TGenProtoIfAllNode::~TGenProtoIfAllNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoIfAllNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoIfAllNode&
TGenProtoIfAllNode::operator=(const TGenProtoIfAllNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoIfAllNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoIfAllNode::Evaluate(TGenProtoCtx& ctxThis)
{
    //
    //  For each child node in our list, evaluate it, then get its value
    //  as a boolean. As soon as one comes up false, set our value to false.
    //  Else, if all of them are true, set our value to true.
    //
    TChildList& colList = colChildren();
    const tCIDLib::TCard4   c4Count = colList.c4ElemCount();
    tCIDLib::TCard4         c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoExprNode* pnodeCur = colList[c4Index];

        pnodeCur->Evaluate(ctxThis);
        if (!pnodeCur->evalCur().bValue())
            break;
    }

    // If we didn't get to the end, then somebody was false
    evalCur().bValue(c4Index == c4Count);
}


tCIDLib::TVoid TGenProtoIfAllNode::PostParseValidation()
{
    // Make sure that we got at least one parameter
    if (colChildren().c4ElemCount() < 1)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XOrMoreParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strDescription()
            , TCardinal(1)
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIfAnyNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoIfAnyNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoIfAnyNode::TGenProtoIfAnyNode() :

    TGenProtoArbChildNode(tGenProtoS::ETypes::Boolean, L"IfAny()")
{
}

TGenProtoIfAnyNode::TGenProtoIfAnyNode(const TGenProtoIfAnyNode& nodeToCopy) :

    TGenProtoArbChildNode(nodeToCopy)
{
}

TGenProtoIfAnyNode::~TGenProtoIfAnyNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoIfAnyNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoIfAnyNode&
TGenProtoIfAnyNode::operator=(const TGenProtoIfAnyNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoIfAnyNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoIfAnyNode::Evaluate(TGenProtoCtx& ctxThis)
{
    //
    //  For each child node in our list, evaluate it, then get its value
    //  as a boolean. As soon as one comes up true, set our value to true.
    //  Else, if none of them are true, set our value to false.
    //
    TChildList& colList = colChildren();
    const tCIDLib::TCard4   c4Count = colList.c4ElemCount();
    tCIDLib::TCard4         c4Index;
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoExprNode* pnodeCur = colList[c4Index];

        pnodeCur->Evaluate(ctxThis);
        if (pnodeCur->evalCur().bValue())
            break;
    }

    // If we didn't get to the end, then somebody was true
    evalCur().bValue(c4Index < c4Count);
}


tCIDLib::TVoid TGenProtoIfAnyNode::PostParseValidation()
{
    // Make sure that we got at least one parameter
    if (colChildren().c4ElemCount() < 1)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_XOrMoreParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , strDescription()
            , TCardinal(1)
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIsZeroNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoIsZeroNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoIsZeroNode::TGenProtoIsZeroNode(TGenProtoExprNode* const pnodeToAdopt) :

    TGenProtoExprNode(tGenProtoS::ETypes::Boolean, L"IsZero()")
    , m_pnodeValue(pnodeToAdopt)
{
}

TGenProtoIsZeroNode::TGenProtoIsZeroNode(const TGenProtoIsZeroNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeValue))
{
}

TGenProtoIsZeroNode::~TGenProtoIsZeroNode()
{
    // Delete the value child expression
    delete m_pnodeValue;
    m_pnodeValue = nullptr;
}


// ---------------------------------------------------------------------------
//  TGenProtoIsZeroNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoIsZeroNode&
TGenProtoIsZeroNode::operator=(const TGenProtoIsZeroNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        delete m_pnodeValue;
        m_pnodeValue = nullptr;
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeValue);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoIsZeroNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoIsZeroNode::bIsConst() const
{
    // If our child is const, then we are
    return m_pnodeValue->bIsConst();
}


tCIDLib::TVoid TGenProtoIsZeroNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evalute our value node
    m_pnodeValue->Evaluate(ctxThis);

    // Set our boolean value according to whether the value is zero or not
    evalCur().bValue(m_pnodeValue->evalCur().bIsZero());
}


tCIDLib::TVoid TGenProtoIsZeroNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeValue = pnodeOptimize(ctxThis, m_pnodeValue);
}


tCIDLib::TVoid TGenProtoIsZeroNode::PostParseValidation()
{
    // Make sure that we got our required parameter
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
            , TCardinal(1)
        );
    }

    // It must be a numeric type
    CheckIsNumericType(*m_pnodeValue, 1);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapFromNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMapFromNode: Constructors and Destructor
// ---------------------------------------------------------------------------

// Our type becomes that of the map's numeric type
TGenProtoMapFromNode::
TGenProtoMapFromNode(const  TGenProtoMap* const       pmapSrc
                    ,       TGenProtoExprNode* const  pnodeMapVal) :

    TGenProtoExprNode(tGenProtoS::ETypes::String, L"MapFrom()")
    , m_pmapSrc(pmapSrc)
    , m_pnodeMapVal(pnodeMapVal)
{
}

TGenProtoMapFromNode::
TGenProtoMapFromNode(const TGenProtoMapFromNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pmapSrc(nodeToCopy.m_pmapSrc)
    , m_pnodeMapVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeMapVal))
{
}

TGenProtoMapFromNode::~TGenProtoMapFromNode()
{
    // We don't own the map, so we just clean up our map value expression
    delete m_pnodeMapVal;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapFromNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoMapFromNode&
TGenProtoMapFromNode::operator=(const TGenProtoMapFromNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // The map pointer we can just shallow copy
        m_pmapSrc = nodeToAssign.m_pmapSrc;

        // The value node we have to duplicate
        delete m_pnodeMapVal;
        m_pnodeMapVal = nullptr;
        m_pnodeMapVal = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeMapVal);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapFromNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoMapFromNode::bIsConst() const
{
    // If our child is const, then we are
    return m_pnodeMapVal->bIsConst();
}


tCIDLib::TVoid TGenProtoMapFromNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the value node and use that to look up the value
    m_pnodeMapVal->Evaluate(ctxThis);

    //
    //  In this case we search the map for the item with the value node's
    //  numeric value.
    //
    const TGenProtoMapItem* pmapiNew = m_pmapSrc->pmapiFindByValue
    (
        m_pnodeMapVal->evalCur()
        , kCIDLib::True
    );

    // Set our value to the key of this item, since we are mapping backwards
    evalCur().strValue(pmapiNew->strKey());
}


tCIDLib::TVoid TGenProtoMapFromNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeMapVal = pnodeOptimize(ctxThis, m_pnodeMapVal);
}


tCIDLib::TVoid TGenProtoMapFromNode::PostParseValidation()
{
    // Make sure that we got our required parameters
    if (!m_pmapSrc || !m_pnodeMapVal)
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

    // The second must be a numeric type
    CheckIsNumericType(*m_pnodeMapVal, 2);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoMapToNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoMapToNode: Constructors and Destructor
// ---------------------------------------------------------------------------

// Our type becomes that of the map's numeric type
TGenProtoMapToNode::
TGenProtoMapToNode( const   TGenProtoMap* const       pmapSrc
                    ,       TGenProtoExprNode* const  pnodeMapVal) :

    TGenProtoExprNode(pmapSrc->eItemType(), L"MapTo()")
    , m_pmapSrc(pmapSrc)
    , m_pnodeMapVal(pnodeMapVal)
{
}

TGenProtoMapToNode::TGenProtoMapToNode(const TGenProtoMapToNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pmapSrc(nodeToCopy.m_pmapSrc)
    , m_pnodeMapVal(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeMapVal))
{
}

TGenProtoMapToNode::~TGenProtoMapToNode()
{
    // We don't own the map, so we just clean up our map value expression
    delete m_pnodeMapVal;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapToNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoMapToNode&
TGenProtoMapToNode::operator=(const TGenProtoMapToNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // The map pointer we can just shallow copy
        m_pmapSrc = nodeToAssign.m_pmapSrc;

        // The value node we have to duplicate
        delete m_pnodeMapVal;
        m_pnodeMapVal = nullptr;
        m_pnodeMapVal = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeMapVal);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoMapToNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoMapToNode::bIsConst() const
{
    // If our child is const, then we are
    return m_pnodeMapVal->bIsConst();
}


tCIDLib::TVoid TGenProtoMapToNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the value node and use that to look up the value
    m_pnodeMapVal->Evaluate(ctxThis);

    //
    //  Do the mapping using the string value of the value node. That gets
    //  us an expression value, which we use to set our value. Tell it to
    //  throw if the mapping value isn't found
    //
    const TGenProtoMapItem* pmapiNew = m_pmapSrc->pmapiFind
    (
        m_pnodeMapVal->evalCur().strValue()
        , kCIDLib::True
    );
    evalCur().SetFrom(pmapiNew->evalItem());
}


tCIDLib::TVoid TGenProtoMapToNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeMapVal = pnodeOptimize(ctxThis, m_pnodeMapVal);
}


tCIDLib::TVoid TGenProtoMapToNode::PostParseValidation()
{
    // Make sure that we got our required parameters
    if (!m_pmapSrc || !m_pnodeMapVal)
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

    // The second must be a string type
    CheckType(*m_pnodeMapVal, tGenProtoS::ETypes::String, 2);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoNOTNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoNOTNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoNOTNode::TGenProtoNOTNode(TGenProtoExprNode* const pnodeValue) :

    TGenProtoExprNode(tGenProtoS::ETypes::Boolean, L"Not()")
    , m_pnodeValue(pnodeValue)
{
}

TGenProtoNOTNode::TGenProtoNOTNode(const TGenProtoNOTNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeValue))
{
}

TGenProtoNOTNode::~TGenProtoNOTNode()
{
    // Delete the value child expression
    delete m_pnodeValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoNOTNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoNOTNode&
TGenProtoNOTNode::operator=(const TGenProtoNOTNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        delete m_pnodeValue;
        m_pnodeValue = nullptr;
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeValue);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoNOTNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoNOTNode::bIsConst() const
{
    // If our child is const, then we are
    return m_pnodeValue->bIsConst();
}


tCIDLib::TVoid TGenProtoNOTNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evalute the the child node and set our value as it's NOT
    m_pnodeValue->Evaluate(ctxThis);
    evalCur().bValue(!m_pnodeValue->evalCur().bValue());
}


tCIDLib::TVoid TGenProtoNOTNode::Optimize(TGenProtoCtx& ctxThis)
{
    // Just do the standard call to the optimize helper method
    m_pnodeValue = pnodeOptimize(ctxThis, m_pnodeValue);
}


tCIDLib::TVoid TGenProtoNOTNode::PostParseValidation()
{
    // Make sure that we got our required parameter
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
            , TCardinal(1)
        );
    }

    // It must be a boolean expression
    CheckType(*m_pnodeValue, tGenProtoS::ETypes::Boolean, 1);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoShiftNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoShiftNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoShiftNode::
TGenProtoShiftNode(         TGenProtoExprNode* const    pnodeValue
                    ,       TGenProtoExprNode* const    pnodeShift
                    , const tCIDLib::TBoolean           bRight) :

    TGenProtoExprNode
    (
        tGenProtoS::ETypes::Card4
        , (bRight ? L"ShiftRight()" : L"ShiftLeft()")
    )
    , m_bRight(kCIDLib::True)
    , m_pnodeShift(pnodeShift)
    , m_pnodeValue(pnodeValue)
{
}

TGenProtoShiftNode::TGenProtoShiftNode(const TGenProtoShiftNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeShift(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeShift))
    , m_pnodeValue(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeValue))
{
}

TGenProtoShiftNode::~TGenProtoShiftNode()
{
    // Delete the children
    delete m_pnodeShift;
    delete m_pnodeValue;
}


// ---------------------------------------------------------------------------
//  TGenProtoShiftNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoShiftNode&
TGenProtoShiftNode::operator=(const TGenProtoShiftNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // Clean up the child nodes
        delete m_pnodeShift;
        m_pnodeShift = nullptr;
        delete m_pnodeValue;
        m_pnodeValue = nullptr;

        // Copy the sources'
        m_pnodeShift = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeShift);
        m_pnodeValue = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeValue);

        // And any other stuff
        m_bRight = nodeToAssign.m_bRight;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoShiftNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoShiftNode::bIsConst() const
{
    // If both our children are const, then we are
    return (m_pnodeValue->bIsConst() && m_pnodeShift->bIsConst());
}


tCIDLib::TVoid TGenProtoShiftNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the two nodes
    m_pnodeValue->Evaluate(ctxThis);
    m_pnodeShift->Evaluate(ctxThis);

    // Get the value out so we can modify it
    tCIDLib::TCard4 c4Ret = m_pnodeValue->evalCur().c4Value();

    // And do the shift and use that to set our value
    if (m_bRight)
        c4Ret >>= (m_pnodeShift->evalCur().c4Value() % 32);
    else
        c4Ret <<= (m_pnodeShift->evalCur().c4Value() % 32);

    evalCur().c4Value(c4Ret);
}


tCIDLib::TVoid TGenProtoShiftNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeShift = pnodeOptimize(ctxThis, m_pnodeShift);
    m_pnodeValue = pnodeOptimize(ctxThis, m_pnodeValue);
}


tCIDLib::TVoid TGenProtoShiftNode::PostParseValidation()
{
    // Make sure that we got our two parameters
    if (!m_pnodeValue || !m_pnodeShift)
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

    // They both must be cardinal type
    CheckIsCardType(*m_pnodeValue, 1);
    CheckIsCardType(*m_pnodeShift, 2);
}


