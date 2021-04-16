//
// FILE NAME: GenProtoS_RuntimeExpression.cpp
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
//  This file implements the basic derivatives of the expression node class
//  that represent simple constant values.
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
RTTIDecls(TGenProtoCheckSumNode, TGenProtoExprNode)
RTTIDecls(TGenProtoCRC16Node, TGenProtoExprNode)
RTTIDecls(TGenProtoFldValNode, TGenProtoExprNode)
RTTIDecls(TGenProtoInputByteNode, TGenProtoExprNode)
RTTIDecls(TGenProtoVarValNode, TGenProtoExprNode)
RTTIDecls(TGenProtoWriteValNode, TGenProtoExprNode)



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCheckSumNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCheckSumNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCheckSumNode::
TGenProtoCheckSumNode(  const   tGenProtoS::ESpecNodes      eExprType
                        , const tGenProtoS::ETypes          eType
                        ,       TGenProtoExprNode* const    pnodeOfs
                        ,       TGenProtoExprNode* const    pnodeLen
                        , const tGenProtoS::ECheckSums      eCheckSumType) :

    TGenProtoExprNode(eType, L"CheckSum()")
    , m_eCheckSumType(eCheckSumType)
    , m_eExprType(eExprType)
    , m_pnodeLen(pnodeLen)
    , m_pnodeOfs(pnodeOfs)
{
}

TGenProtoCheckSumNode::
TGenProtoCheckSumNode(const TGenProtoCheckSumNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eCheckSumType(nodeToCopy.m_eCheckSumType)
    , m_eExprType(nodeToCopy.m_eExprType)
    , m_pnodeOfs(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeOfs))
    , m_pnodeLen(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLen))
{
}

TGenProtoCheckSumNode::~TGenProtoCheckSumNode()
{
    delete m_pnodeOfs;
    delete m_pnodeLen;
}


// ---------------------------------------------------------------------------
//  TGenProtoCheckSumNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoCheckSumNode&
TGenProtoCheckSumNode::operator=(const TGenProtoCheckSumNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_eCheckSumType = nodeToAssign.m_eCheckSumType;
        m_eExprType     = nodeToAssign.m_eExprType;

        delete m_pnodeOfs;
        m_pnodeOfs = nullptr;
        delete m_pnodeLen;
        m_pnodeLen = nullptr;

        m_pnodeOfs      = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeOfs);
        m_pnodeLen      = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLen);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoCheckSumNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoCheckSumNode::bIsConst() const
{
    // We are never const
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoCheckSumNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Get the buffer that we are working with
    const TMemBuf* pmbufSrc = (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
                              ? &ctxThis.mbufReply()
                              : &ctxThis.mbufSend();
    const tCIDLib::TCard4 c4BufLen = (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
                                     ? ctxThis.c4ReplyBytes()
                                     : ctxThis.c4SendBytes();

    // Evaluate the two expressions and get the offset/len values
    m_pnodeOfs->Evaluate(ctxThis);
    m_pnodeLen->Evaluate(ctxThis);

    tCIDLib::TCard4         c4Index = m_pnodeOfs->evalCur().c4Value();
    const tCIDLib::TCard4   c4End   = c4Index + m_pnodeLen->evalCur().c4Value();

    //
    //  Make sure we won't exceed the available bytes for the buffer we
    //  are working on.
    //
    if (c4End > c4BufLen)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_BadBufRange
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index)
            , TCardinal(m_pnodeLen->evalCur().c4Value())
            , TCardinal(c4BufLen)
        );
    }

    const tGenProtoS::ETypes eRetType = eType();

    #if CID_DEBUG_ON
    if ((eRetType != tGenProtoS::ETypes::Card1)
    &&  (eRetType != tGenProtoS::ETypes::Card2)
    &&  (eRetType != tGenProtoS::ETypes::Card4))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcDebug_UnsupportedType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
            , TCardinal(tCIDLib::c4EnumOrd(eRetType))
            , strDescription()
        );
    }
    #endif

    //
    //  Now, according to the type we were told to use, run the algoritm and
    //  store the result.
    //
    if (m_eCheckSumType == tGenProtoS::ECheckSums::AddWithOverflow)
    {
        if (eRetType == tGenProtoS::ETypes::Card1)
        {
            tCIDLib::TCard1 c1Sum = 0;
            for (; c4Index < c4End; c4Index++)
                c1Sum += pmbufSrc->c1At(c4Index);
            evalCur().c1Value(c1Sum);
        }
         else if (eRetType == tGenProtoS::ETypes::Card2)
        {
            tCIDLib::TCard2 c2Sum = 0;
            for (; c4Index < c4End; c4Index++)
                c2Sum += pmbufSrc->c1At(c4Index);
            evalCur().c2Value(c2Sum);
        }
         else if (eRetType == tGenProtoS::ETypes::Card4)
        {
            tCIDLib::TCard4 c4Sum = 0;
            for (; c4Index < c4End; c4Index++)
                c4Sum += pmbufSrc->c1At(c4Index);
            evalCur().c4Value(c4Sum);
        }
    }
     else if (m_eCheckSumType == tGenProtoS::ECheckSums::Xor1)
    {
        if (eRetType == tGenProtoS::ETypes::Card1)
        {
            tCIDLib::TCard1 c1Sum = pmbufSrc->c1At(c4Index);
            c4Index++;
            for (; c4Index < c4End; c4Index++)
                c1Sum ^= pmbufSrc->c1At(c4Index);
            evalCur().c1Value(c1Sum);
        }
         else if (eRetType == tGenProtoS::ETypes::Card2)
        {
            tCIDLib::TCard2 c2Sum = pmbufSrc->c2At(c4Index);
            for (; c4Index < c4End; c4Index++)
                c2Sum ^= pmbufSrc->c2At(c4Index);
            evalCur().c2Value(c2Sum);
        }
         else if (eRetType == tGenProtoS::ETypes::Card4)
        {
            tCIDLib::TCard4 c4Sum = pmbufSrc->c4At(c4Index);
            for (; c4Index < c4End; c4Index++)
                c4Sum ^= pmbufSrc->c4At(c4Index);
            evalCur().c4Value(c4Sum);
        }
    }
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
            , TCardinal(tCIDLib::c4EnumOrd(m_eCheckSumType))
            , TString(L"tGenProtoS::ECheckSums")
        );
        #endif
    }
}



tCIDLib::TVoid TGenProtoCheckSumNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeOfs = pnodeOptimize(ctxThis, m_pnodeOfs);
    m_pnodeLen = pnodeOptimize(ctxThis, m_pnodeLen);
}


tCIDLib::TVoid TGenProtoCheckSumNode::PostParseValidation()
{
    // Our type has to be cardinal
    if ((eType() != tGenProtoS::ETypes::Card1)
    &&  (eType() != tGenProtoS::ETypes::Card2)
    &&  (eType() != tGenProtoS::ETypes::Card4))
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcExpr_PMustBeCardinal
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::TypeMatch
            , TCardinal(2)
            , strDescription()
        );
    }

    // Make sure our offset/lenexpressions are a cardinal type
    CheckIsCardType(*m_pnodeOfs, 3);
    CheckIsCardType(*m_pnodeLen, 4);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCRC16Node
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCRC16Node: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCRC16Node::
TGenProtoCRC16Node( const   tGenProtoS::ESpecNodes      eExprType
                    ,       TGenProtoExprNode* const    pnodeOfs
                    ,       TGenProtoExprNode* const    pnodeLen
                    ,       TGenProtoExprNode* const    pnodePoly
                    ,       TGenProtoExprNode* const    pnodeCount) :

    TGenProtoExprNode(tGenProtoS::ETypes::Card2, L"CRC16()")
    , m_eExprType(eExprType)
    , m_pnodeCount(pnodeCount)
    , m_pnodeLen(pnodeLen)
    , m_pnodeOfs(pnodeOfs)
    , m_pnodePoly(pnodePoly)
{
}

TGenProtoCRC16Node::
TGenProtoCRC16Node(const TGenProtoCRC16Node& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eExprType(nodeToCopy.m_eExprType)
    , m_pnodeCount(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeCount))
    , m_pnodeOfs(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeOfs))
    , m_pnodeLen(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLen))
    , m_pnodePoly(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodePoly))
{
}

TGenProtoCRC16Node::~TGenProtoCRC16Node()
{
    delete m_pnodeCount;
    delete m_pnodeOfs;
    delete m_pnodeLen;
    delete m_pnodePoly;
}


// ---------------------------------------------------------------------------
//  TGenProtoCRC16Node: Public operators
// ---------------------------------------------------------------------------
TGenProtoCRC16Node&
TGenProtoCRC16Node::operator=(const TGenProtoCRC16Node& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_eExprType = nodeToAssign.m_eExprType;

        delete m_pnodeCount;
        m_pnodeCount = nullptr;
        delete m_pnodeOfs;
        m_pnodeOfs   = nullptr;
        delete m_pnodeLen;
        m_pnodeLen   = nullptr;
        delete m_pnodePoly;
        m_pnodePoly  = nullptr;

        m_pnodeCount = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeCount);
        m_pnodeOfs   = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeOfs);
        m_pnodeLen   = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLen);
        m_pnodePoly  = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodePoly);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoCRC16Node: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoCRC16Node::bIsConst() const
{
    // We are never const
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoCRC16Node::Evaluate(TGenProtoCtx& ctxThis)
{
    // Get the buffer that we are working with
    const TMemBuf* pmbufSrc = (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
                              ? &ctxThis.mbufReply()
                              : &ctxThis.mbufSend();
    const tCIDLib::TCard4 c4BufLen = (m_eExprType == tGenProtoS::ESpecNodes::ReplyBuf)
                                     ? ctxThis.c4ReplyBytes()
                                     : ctxThis.c4SendBytes();

    // Evaluate the two expressions and get the offset/len values
    m_pnodeOfs->Evaluate(ctxThis);
    m_pnodeLen->Evaluate(ctxThis);

    tCIDLib::TCard4         c4Index  = m_pnodeOfs->evalCur().c4Value();
    const tCIDLib::TCard4   c4EndInd = c4Index + m_pnodeLen->evalCur().c4Value();
    const tCIDLib::TCard2   c2Poly   = m_pnodePoly->evalCur().c2Value();
    const tCIDLib::TCard4   c4Count  = m_pnodeCount->evalCur().c4Value();

    //
    //  Make sure we won't exceed the available bytes for the buffer we
    //  are working on.
    //
    if (c4EndInd > c4BufLen)
    {
        facGenProtoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kGPDErrs::errcRunT_BadBufRange
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Index
            , TCardinal(c4Index)
            , TCardinal(m_pnodeLen->evalCur().c4Value())
            , TCardinal(c4BufLen)
        );
    }

    tCIDLib::TCard2 c2CRCVal = 0;
    for (; c4Index <= c4EndInd; c4Index++)
    {
        c2CRCVal ^= tCIDLib::TCard2(pmbufSrc->c1At(c4Index));
        for (tCIDLib::TCard4 c4InInd = 0; c4InInd < c4Count; c4InInd++)
        {
            const tCIDLib::TBoolean bFlag((c2CRCVal & 0x1) != 0);
            c2CRCVal >>= 1;
            if (bFlag)
                c2CRCVal ^= c2Poly;
        }
    }

    // Set the result as our value
    evalCur().c2Value(c2CRCVal);
}



tCIDLib::TVoid TGenProtoCRC16Node::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeCount = pnodeOptimize(ctxThis, m_pnodeCount);
    m_pnodeOfs   = pnodeOptimize(ctxThis, m_pnodeOfs);
    m_pnodeLen   = pnodeOptimize(ctxThis, m_pnodeLen);
    m_pnodePoly  = pnodeOptimize(ctxThis, m_pnodePoly);
}


tCIDLib::TVoid TGenProtoCRC16Node::PostParseValidation()
{
    // Make sure our offset/len, poly, and count expressions are a cardinal type
    CheckIsCardType(*m_pnodeOfs, 2);
    CheckIsCardType(*m_pnodeLen, 3);
    CheckIsCardType(*m_pnodePoly, 4);
    CheckIsCardType(*m_pnodeCount, 5);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFldValNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoFldValNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoFldValNode::TGenProtoFldValNode(TGenProtoFldInfo* const pfldiSrc) :

    TGenProtoExprNode
    (
        TFacGenProtoS::eFldTypeToExprType(pfldiSrc->flddInfo().eType())
        , L"Field Reference"
    )
    , m_c4SerialNum(0)
    , m_pfldiSrc(pfldiSrc)
{
}

TGenProtoFldValNode::TGenProtoFldValNode(const TGenProtoFldValNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_c4SerialNum(nodeToCopy.m_c4SerialNum)
    , m_pfldiSrc(nodeToCopy.m_pfldiSrc)
{
}

TGenProtoFldValNode::~TGenProtoFldValNode()
{
    // We don't own the field info pointer, so we do nothing here
}


// ---------------------------------------------------------------------------
//  TGenProtoFldValNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoFldValNode&
TGenProtoFldValNode::operator=(const TGenProtoFldValNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // We just reference the field info pointer, so we can just copy it
        m_pfldiSrc = nodeToAssign.m_pfldiSrc;

        // Reset the value fields
        m_c4SerialNum = 0;
        m_strValue.Clear();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoFldValNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoFldValNode::bIsConst() const
{
    // We are never const
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoFldValNode::Evaluate(TGenProtoCtx& ctxInfo)
{
    tCQCKit::EFldTypes eType;
    const tCIDLib::TBoolean bNew = ctxInfo.psdrvThis()->bReadFieldByName
    (
        m_pfldiSrc->flddInfo().strName()
        , m_c4SerialNum
        , m_strValue
        , eType
        , kCIDLib::False
    );

    if (bNew)
       evalCur().strValue(m_strValue);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoInputByteNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoInputByteNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoInputByteNode::TGenProtoInputByteNode() :

    TGenProtoExprNode(tGenProtoS::ETypes::Card1, L"Current input byte")
{
}

TGenProtoInputByteNode::
TGenProtoInputByteNode(const TGenProtoInputByteNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
{
}

TGenProtoInputByteNode::~TGenProtoInputByteNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoInputByteNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoInputByteNode&
TGenProtoInputByteNode::operator=(const TGenProtoInputByteNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoInputByteNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoInputByteNode::bIsConst() const
{
    // We are never const
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoInputByteNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // We just set our value from the input byte in the context
    evalCur().c1Value(ctxThis.c1InputByte());
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoVarValNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoVarValNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoVarValNode::TGenProtoVarValNode(TGenProtoVarInfo* const pvariSrc) :

    TGenProtoExprNode(pvariSrc->eType(), L"Variable Reference")
    , m_pvariSrc(pvariSrc)
{
}

TGenProtoVarValNode::TGenProtoVarValNode(const TGenProtoVarValNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pvariSrc(nodeToCopy.m_pvariSrc)
{
}

TGenProtoVarValNode::~TGenProtoVarValNode()
{
    // We don't own the variable info pointer, so we do nothing here
}


// ---------------------------------------------------------------------------
//  TGenProtoVarValNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoVarValNode&
TGenProtoVarValNode::operator=(const TGenProtoVarValNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // We just reference the var info, so we can just shallow copy it
        m_pvariSrc = nodeToAssign.m_pvariSrc;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoVarValNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoVarValNode::bIsConst() const
{
    // Return the const state of the variable node
    return m_pvariSrc->bIsConst();
}


tCIDLib::TVoid TGenProtoVarValNode::Evaluate(TGenProtoCtx&)
{
    // Just set our expression value from the variable value
    evalCur().SetFrom(m_pvariSrc->evalThis());
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoWriteValNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoWriteValNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoWriteValNode::TGenProtoWriteValNode(const tCQCKit::EFldTypes eType) :

    TGenProtoExprNode
    (
        TFacGenProtoS::eFldTypeToExprType(eType)
        , L"Field Write Value"
    )
{
}

TGenProtoWriteValNode::
TGenProtoWriteValNode(const TGenProtoWriteValNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
{
}

TGenProtoWriteValNode::~TGenProtoWriteValNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoWriteValNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoWriteValNode&
TGenProtoWriteValNode::operator=(const TGenProtoWriteValNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoWriteValNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoWriteValNode::bIsConst() const
{
    // We are never const
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoWriteValNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // We just set our value from the write value in the context
    evalCur().SetFrom(ctxThis.evalWrite());
}


