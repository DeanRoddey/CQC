//
// FILE NAME: GenProtoS_StrFuncs.cpp
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
//  This file implements some string processing oriented expression nodes.
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
RTTIDecls(TGenProtoCatStrNode,TGenProtoExprNode)
RTTIDecls(TGenProtoSetCaseNode,TGenProtoExprNode)
RTTIDecls(TGenProtoExtractStrBaseNode,TGenProtoExprNode)
RTTIDecls(TGenProtoExtractASCIIValNode,TGenProtoExtractStrBaseNode)
RTTIDecls(TGenProtoExtractStrNode,TGenProtoExtractStrBaseNode)
RTTIDecls(TGenProtoExtractTokNode,TGenProtoExprNode)
RTTIDecls(TGenProtoFormatNode,TGenProtoExprNode)
RTTIDecls(TGenProtoIsASCIIXNode,TGenProtoExprNode)




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoCatStrNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoCatStrNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoCatStrNode::TGenProtoCatStrNode() :

    TGenProtoArbChildNode(tGenProtoS::ETypes::String, L"CatStr")
{
}

TGenProtoCatStrNode::TGenProtoCatStrNode(const TGenProtoCatStrNode& nodeToCopy) :

    TGenProtoArbChildNode(nodeToCopy)
{
}

TGenProtoCatStrNode::~TGenProtoCatStrNode()
{
}


// ---------------------------------------------------------------------------
//  TGenProtoCatStrNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoCatStrNode&
TGenProtoCatStrNode::operator=(const TGenProtoCatStrNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoCatStrNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoCatStrNode::Evaluate(TGenProtoCtx& ctxThis)
{
    //
    //  For each child node in our list, evaluate it, then get its value
    //  as a string and cat it onto a temp string. At the end, set this temp
    //  string as our value.
    //
    TString strTmp;

    TChildList& colList = colChildren();
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TGenProtoExprNode* pnodeCur = colList[c4Index];
        pnodeCur->Evaluate(ctxThis);
        strTmp.Append(pnodeCur->evalCur().strValue());
    }

    // Now set our value from the result
    evalCur().strValue(strTmp);
}


tCIDLib::TVoid TGenProtoCatStrNode::PostParseValidation()
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
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoSetCaseNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoSetCaseNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoSetCaseNode
::TGenProtoSetCaseNode(         TGenProtoExprNode* const    pnodeStr
                        , const tCIDLib::TBoolean           bToUpper) :
    TGenProtoExprNode
    (
        tGenProtoS::ETypes::String, bToUpper ? L"ToUpper" : L"ToLower"
    )
    , m_bToUpper(bToUpper)
    , m_pnodeStr(pnodeStr)
{
}

TGenProtoSetCaseNode::TGenProtoSetCaseNode(const TGenProtoSetCaseNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_bToUpper(nodeToCopy.m_bToUpper)
    , m_pnodeStr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeStr))
{
}

TGenProtoSetCaseNode::~TGenProtoSetCaseNode()
{
    delete m_pnodeStr;
}


// ---------------------------------------------------------------------------
//  TGenProtoSetCaseNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoSetCaseNode&
TGenProtoSetCaseNode::operator=(const TGenProtoSetCaseNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_bToUpper = nodeToAssign.m_bToUpper;

        delete m_pnodeStr;
        m_pnodeStr = 0;
        m_pnodeStr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeStr);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoSetCaseNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoSetCaseNode::bIsConst() const
{
    // If our child node is const, then we are
    return m_pnodeStr->bIsConst();
}


tCIDLib::TVoid TGenProtoSetCaseNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the child node to get the value to format
    m_pnodeStr->Evaluate(ctxThis);

    // Get the value of the string node into our value, then adjust the case
    evalCur().strValue(m_pnodeStr->evalCur().strValue());
    if (m_bToUpper)
        evalCur().ToUpper();
    else
        evalCur().ToLower();
}


tCIDLib::TVoid TGenProtoSetCaseNode::Optimize(TGenProtoCtx& ctxThis)
{
    // Do the usual optimization call
    m_pnodeStr = pnodeOptimize(ctxThis, m_pnodeStr);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractStrBaseNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoExtractStrBaseNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExtractStrBaseNode::
TGenProtoExtractStrBaseNode(const   TString&                    strName
                            , const tGenProtoS::ESpecNodes      eSrcBuf
                            , const tGenProtoS::ETypes          eType
                            ,       TGenProtoExprNode* const    pnodeOfsExpr
                            ,       TGenProtoExprNode* const    pnodeLenExpr) :

    TGenProtoExprNode(eType, strName)
    , m_eSrcBuf(eSrcBuf)
    , m_pnodeOfsExpr(pnodeOfsExpr)
    , m_pnodeLenExpr(pnodeLenExpr)
{
}

TGenProtoExtractStrBaseNode::
TGenProtoExtractStrBaseNode(const TGenProtoExtractStrBaseNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eSrcBuf(nodeToCopy.m_eSrcBuf)
    , m_pnodeOfsExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeOfsExpr))
    , m_pnodeLenExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeLenExpr))
{
}

TGenProtoExtractStrBaseNode::~TGenProtoExtractStrBaseNode()
{
    delete m_pnodeOfsExpr;
    delete m_pnodeLenExpr;
}


// ---------------------------------------------------------------------------
//  TGenProtoExtractStrBaseNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoExtractStrBaseNode& TGenProtoExtractStrBaseNode::
operator=(const TGenProtoExtractStrBaseNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TGenProtoExprNode::operator=(nodeToAssign);

        // Clean up our existing nodes first
        delete m_pnodeOfsExpr;
        m_pnodeOfsExpr = 0;
        delete m_pnodeLenExpr;
        m_pnodeLenExpr = 0;

        m_eSrcBuf        = nodeToAssign.m_eSrcBuf;
        m_pnodeOfsExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeOfsExpr);
        m_pnodeLenExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeLenExpr);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoExtractStrBaseNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoExtractStrBaseNode::bIsConst() const
{
    // We are never constant
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoExtractStrBaseNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeOfsExpr  = pnodeOptimize(ctxThis, m_pnodeOfsExpr);
    m_pnodeLenExpr  = pnodeOptimize(ctxThis, m_pnodeLenExpr);
}


tCIDLib::TVoid TGenProtoExtractStrBaseNode::PostParseValidation()
{
    // Make sure that our two child expressions resolve to cardinal types
    CheckIsCardType(*m_pnodeOfsExpr, 1);
    CheckIsCardType(*m_pnodeLenExpr, 2);
}


// ---------------------------------------------------------------------------
//  TGenProtoExtractStrBaseNode: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TGenProtoExtractStrBaseNode::GetText(TGenProtoCtx&  ctxThis
                                    , TString&      strToFill)
{
    // Evaluate our two nodes, and get out the values for use below
    m_pnodeOfsExpr->Evaluate(ctxThis);
    m_pnodeLenExpr->Evaluate(ctxThis);

    // Get the length of the bufffer we are doing
    const tCIDLib::TCard4 c4SrcLen = (m_eSrcBuf == tGenProtoS::ESpecNodes::ReplyBuf)
                                     ? ctxThis.c4ReplyBytes()
                                     : ctxThis.c4SendBytes();

    //
    //  The offset we can take as is. But, if its past the end of the buffer,
    //  then just set ourselves as an empty string.
    //
    const tCIDLib::TCard4 c4Ofs = m_pnodeOfsExpr->evalCur().c4Value();
    if (c4Ofs >= c4SrcLen)
    {
        strToFill.Clear();
        return;
    }

    //
    //  Ok, get the length. if needed, shorten it so that it doesn't go past
    //  the end of the buffer. If zero len, return an empty string.
    //
    tCIDLib::TCard4 c4Len = m_pnodeLenExpr->evalCur().c4Value();
    if (!c4Len)
    {
        strToFill.Clear();
        return;
    }

    // Looks like we have something, clip the length if needed
    if (c4Ofs + c4Len > c4SrcLen)
        c4Len = c4SrcLen - c4Ofs;

    // And get a pointer to the buffer that we act on
    const TMemBuf* pmbufSrc = (m_eSrcBuf == tGenProtoS::ESpecNodes::ReplyBuf)
                              ? &ctxThis.mbufReply()
                              : &ctxThis.mbufSend();

    //
    //  Get the text converter from the context. We'll use this to do our
    //  extraction. And get a pointer to the position in the reply buffer
    //  that we are supposed to start reading at.
    //
    TTextConverter& tcvtToUse = ctxThis.tcvtDevice();

    try
    {
        //
        //  In this one, we can just go for it all at once. We know how
        //  many bytes are available.
        //
        tCIDLib::TCard4 c4BytesEaten = tcvtToUse.c4ConvertFrom
        (
            pmbufSrc->pc1DataAt(c4Ofs)
            , c4Len
            , strToFill
        );
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
                , TCardinal(c4SrcLen)
            );
        }

        // Not one of the buffer overflow errors, so just rethrow as is
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractASCIIValNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExtractASCIIValNode::
TGenProtoExtractASCIIValNode(const  tGenProtoS::ETypes          eType
                            , const tCIDLib::ERadices           eRadix
                            , const tGenProtoS::ESpecNodes      eSrcBuf
                            ,       TGenProtoExprNode* const    pnodeOfsExpr
                            ,       TGenProtoExprNode* const    pnodeLenExpr) :

    TGenProtoExtractStrBaseNode
    (
        L"ExtractASCIIxxx()"
        , eSrcBuf
        , eType
        , pnodeOfsExpr
        , pnodeLenExpr
    )
    , m_eRadix(eRadix)
{
}

TGenProtoExtractASCIIValNode::
TGenProtoExtractASCIIValNode(const TGenProtoExtractASCIIValNode& nodeToCopy) :

    TGenProtoExtractStrBaseNode(nodeToCopy)
    , m_eRadix(nodeToCopy.m_eRadix)
{
}

TGenProtoExtractASCIIValNode::~TGenProtoExtractASCIIValNode()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TGenProtoExtractASCIIValNode& TGenProtoExtractASCIIValNode::
operator=(const TGenProtoExtractASCIIValNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);
        m_eRadix = nodeToAssign.m_eRadix;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoExtractASCIIValNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Get our parent class to extract the value for us
    TString strVal;
    GetText(ctxThis, strVal);

    //
    //  And convert to binary according to our type. Note that we limit
    //  the types to the largest of each category so that we can be sure to
    //  handle the results parsed out. They can then cast down if they want.
    //
    switch(evalCur().eType())
    {
        case tGenProtoS::ETypes::Card4 :
            evalCur().c4Value(strVal.c4Val(m_eRadix));
            break;

        case tGenProtoS::ETypes::Int4 :
            evalCur().i4Value(strVal.i4Val(m_eRadix));
            break;

        case tGenProtoS::ETypes::Float8 :
            evalCur().f8Value(strVal.f8Val());
            break;

        default :
            #if CID_DEBUG_ON
            facGenProtoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kGPDErrs::errcDebug_UnsupportedType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TCardinal(tCIDLib::c4EnumOrd(evalCur().eType()))
                , strDescription()
            );
            #endif
            break;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractStrNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExtractStrNode::
TGenProtoExtractStrNode(const   tGenProtoS::ESpecNodes      eSrcBuf
                        ,       TGenProtoExprNode* const    pnodeOfsExpr
                        ,       TGenProtoExprNode* const    pnodeLenExpr) :
    TGenProtoExtractStrBaseNode
    (
        L"ExtractStr()"
        , eSrcBuf
        , tGenProtoS::ETypes::String
        , pnodeOfsExpr
        , pnodeLenExpr
    )
{
}

TGenProtoExtractStrNode::
TGenProtoExtractStrNode(const TGenProtoExtractStrNode& nodeToCopy) :

    TGenProtoExtractStrBaseNode(nodeToCopy)
{
}

TGenProtoExtractStrNode::~TGenProtoExtractStrNode()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TGenProtoExtractStrNode&
TGenProtoExtractStrNode::operator=(const TGenProtoExtractStrNode& nodeToAssign)
{
    TParent::operator=(nodeToAssign);
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TGenProtoExtractStrNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Get our parent class to extract the value for us
    TString strVal;
    GetText(ctxThis, strVal);
    evalCur().strValue(strVal);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoExtractTokNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoExtractTokNode::
TGenProtoExtractTokNode(TGenProtoExprNode* const    pnodeSrcExpr
                        , TGenProtoExprNode* const  pnodeTokNumExpr
                        , TGenProtoExprNode* const  pnodeSepCharExpr
                        , TGenProtoExprNode* const  pnodeStripExpr) :

    TGenProtoExprNode(tGenProtoS::ETypes::String, L"ExtractToken()")
    , m_pnodeSepCharExpr(pnodeSepCharExpr)
    , m_pnodeSrcExpr(pnodeSrcExpr)
    , m_pnodeStripExpr(pnodeStripExpr)
    , m_pnodeTokNumExpr(pnodeTokNumExpr)
{
}

TGenProtoExtractTokNode::
TGenProtoExtractTokNode(const TGenProtoExtractTokNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_pnodeSepCharExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeSepCharExpr))
    , m_pnodeSrcExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeSrcExpr))
    , m_pnodeStripExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeStripExpr))
    , m_pnodeTokNumExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeTokNumExpr))
{
}

TGenProtoExtractTokNode::~TGenProtoExtractTokNode()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TGenProtoExtractTokNode&
TGenProtoExtractTokNode::operator=(const TGenProtoExtractTokNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        // Clean up our existing nodes
        delete m_pnodeSepCharExpr;
        m_pnodeSepCharExpr = 0;
        delete m_pnodeSrcExpr;
        m_pnodeSrcExpr = 0;
        delete m_pnodeStripExpr;
        m_pnodeStripExpr = 0;
        delete m_pnodeTokNumExpr;
        m_pnodeTokNumExpr = 0;

        m_pnodeSepCharExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeSepCharExpr);
        m_pnodeSrcExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeSrcExpr);
        m_pnodeStripExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeStripExpr);
        m_pnodeTokNumExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeTokNumExpr);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoExtractTokNode::bIsConst() const
{
    // We are neve constant
    return kCIDLib::False;
}


tCIDLib::TVoid TGenProtoExtractTokNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate our child nodes, and get out the values for use below
    m_pnodeSepCharExpr->Evaluate(ctxThis);
    m_pnodeSrcExpr->Evaluate(ctxThis);
    m_pnodeStripExpr->Evaluate(ctxThis);
    m_pnodeTokNumExpr->Evaluate(ctxThis);

    // And get the values out
    const tCIDLib::TCard4 c4TokNum = m_pnodeTokNumExpr->evalCur().c4Value();
    const tCIDLib::TCh chSepChar = m_pnodeSepCharExpr->evalCur().strValue()[0];
    const TString strSrc = m_pnodeSrcExpr->evalCur().strValue();

    // And pull out the token into our value object's string value
    strSrc.bExtractNthToken
    (
        c4TokNum
        , chSepChar
        , evalCur().strValue()
        , m_pnodeStripExpr->evalCur().bValue()
    );
}


tCIDLib::TVoid TGenProtoExtractTokNode::Optimize(TGenProtoCtx& ctxThis)
{
    //
    //  Just call a base class provided helper method on each of our children.
    //  It handles the recursion if required. If the node can't be optimized,
    //  we get it back again (though some of its children still may have
    //  be optimized.)
    //
    m_pnodeSepCharExpr  = pnodeOptimize(ctxThis, m_pnodeSepCharExpr);
    m_pnodeSrcExpr  = pnodeOptimize(ctxThis, m_pnodeSrcExpr);
    m_pnodeStripExpr  = pnodeOptimize(ctxThis, m_pnodeStripExpr);
    m_pnodeTokNumExpr  = pnodeOptimize(ctxThis, m_pnodeTokNumExpr);
}


tCIDLib::TVoid TGenProtoExtractTokNode::PostParseValidation()
{
    // The sep char must be a single charactera nd the token num a card
    CheckIsBoolType(*m_pnodeStripExpr, 2);
    CheckIsCharType(*m_pnodeSepCharExpr, 2);
    CheckIsCardType(*m_pnodeTokNumExpr, 3);
}




// ---------------------------------------------------------------------------
//   CLASS: TGenProtoFormatNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoFormatNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoFormatNode::
TGenProtoFormatNode(        TGenProtoExprNode* const    pnodeToFormat
                    , const tCIDLib::TCard4             c4Width
                    , const tCIDLib::EHJustify          eJustify
                    , const tCIDLib::ERadices           eRadix
                    , const tCIDLib::TCh                chFill) :

    TGenProtoExprNode(tGenProtoS::ETypes::String, L"ToString()")
    , m_c4Width(c4Width)
    , m_chFill(chFill)
    , m_eJustify(eJustify)
    , m_eRadix(eRadix)
    , m_pnodeToFormat(pnodeToFormat)
{
}

TGenProtoFormatNode::TGenProtoFormatNode(const TGenProtoFormatNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_c4Width(nodeToCopy.m_c4Width)
    , m_chFill(nodeToCopy.m_chFill)
    , m_eJustify(nodeToCopy.m_eJustify)
    , m_eRadix(nodeToCopy.m_eRadix)
    , m_pnodeToFormat(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeToFormat))
{
}

TGenProtoFormatNode::~TGenProtoFormatNode()
{
    delete m_pnodeToFormat;
}


// ---------------------------------------------------------------------------
//  TGenProtoFormatNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoFormatNode&
TGenProtoFormatNode::operator=(const TGenProtoFormatNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        m_c4Width       = nodeToAssign.m_c4Width;
        m_chFill        = nodeToAssign.m_chFill;
        m_eJustify      = nodeToAssign.m_eJustify;
        m_eRadix        = nodeToAssign.m_eRadix;

        delete m_pnodeToFormat;
        m_pnodeToFormat = 0;
        m_pnodeToFormat = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeToFormat);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoFormatNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoFormatNode::bIsConst() const
{
    // If our child expression is const, then we are
    return m_pnodeToFormat->bIsConst();
}


tCIDLib::TVoid TGenProtoFormatNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the child node to get the value to format
    m_pnodeToFormat->Evaluate(ctxThis);

    //
    //  And format it's value into a temp string. We have to do it differently
    //  according to the type of our child node.
    //
    TString strTmp;
    m_pnodeToFormat->evalCur().FormatToStr
    (
        strTmp
        , m_eRadix
        , m_c4Width
        , m_eJustify
        , m_chFill
    );

    // Now set our value from the result
    evalCur().strValue(strTmp);
}


tCIDLib::TVoid TGenProtoFormatNode::Optimize(TGenProtoCtx& ctxThis)
{
    m_pnodeToFormat = pnodeOptimize(ctxThis, m_pnodeToFormat);
}



// ---------------------------------------------------------------------------
//   CLASS: TGenProtoIsASCIIXNode
//  PREFIX: node
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGenProtoIsASCIIXNode: Constructors and Destructor
// ---------------------------------------------------------------------------
TGenProtoIsASCIIXNode::
TGenProtoIsASCIIXNode(          TGenProtoExprNode* const    pnodeToCheckExpr
                        , const EASCIITypes                 eType) :

    TGenProtoExprNode
    (
        tGenProtoS::ETypes::Boolean
        , pszTypeToName(eType)
    )
    , m_eType(eType)
    , m_pnodeToCheckExpr(pnodeToCheckExpr)
{
}

TGenProtoIsASCIIXNode::
TGenProtoIsASCIIXNode(          TGenProtoExprNode* const    pnodeToCheckExpr
                        , const tGenProtoS::ETokens         eToken) :
    TGenProtoExprNode
    (
        tGenProtoS::ETypes::Boolean
        , pszTokToName(eToken)
    )
    , m_eType(eTokToType(eToken))
    , m_pnodeToCheckExpr(pnodeToCheckExpr)
{
}

TGenProtoIsASCIIXNode::
TGenProtoIsASCIIXNode(const TGenProtoIsASCIIXNode& nodeToCopy) :

    TGenProtoExprNode(nodeToCopy)
    , m_eType(nodeToCopy.m_eType)
    , m_pnodeToCheckExpr(::pDupObject<TGenProtoExprNode>(nodeToCopy.m_pnodeToCheckExpr))
{
}

TGenProtoIsASCIIXNode::~TGenProtoIsASCIIXNode()
{
    delete m_pnodeToCheckExpr;
}


// ---------------------------------------------------------------------------
//  TGenProtoIsASCIIXNode: Public operators
// ---------------------------------------------------------------------------
TGenProtoIsASCIIXNode&
TGenProtoIsASCIIXNode::operator=(const TGenProtoIsASCIIXNode& nodeToAssign)
{
    if (this != &nodeToAssign)
    {
        TParent::operator=(nodeToAssign);

        m_eType = nodeToAssign.m_eType;

        delete m_pnodeToCheckExpr;
        m_pnodeToCheckExpr = 0;
        m_pnodeToCheckExpr = ::pDupObject<TGenProtoExprNode>(nodeToAssign.m_pnodeToCheckExpr);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TGenProtoIsASCIIXNode: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGenProtoIsASCIIXNode::bIsConst() const
{
    // If our child expression is const, then we are
    return m_pnodeToCheckExpr->bIsConst();
}


tCIDLib::TVoid TGenProtoIsASCIIXNode::Evaluate(TGenProtoCtx& ctxThis)
{
    // Evaluate the expression and get the value to check
    m_pnodeToCheckExpr->Evaluate(ctxThis);
    evalCur().bValue(bCheckIt(m_pnodeToCheckExpr->evalCur().c4Value(), m_eType));
}


tCIDLib::TVoid TGenProtoIsASCIIXNode::Optimize(TGenProtoCtx& ctxThis)
{
    m_pnodeToCheckExpr  = pnodeOptimize(ctxThis, m_pnodeToCheckExpr);
}


tCIDLib::TVoid TGenProtoIsASCIIXNode::PostParseValidation()
{
    CheckIsCardType(*m_pnodeToCheckExpr, 1);
}


// ---------------------------------------------------------------------------
//  TGenProtoIsASCIIXNode: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TGenProtoIsASCIIXNode::bCheckIt(const   tCIDLib::TCard4 c4ToCheck
                                , const EASCIITypes     eType) const
{
    tCIDLib::TBoolean bRet;

    if (eType == EASCIITypes::DecDigit)
        bRet = TRawStr::bIsDigit(tCIDLib::TCh(c4ToCheck));
    else if (eType == EASCIITypes::HexDigit)
        bRet = TRawStr::bIsHexDigit(tCIDLib::TCh(c4ToCheck));
    else if (eType == EASCIITypes::Alpha)
        bRet = TRawStr::bIsAlpha(tCIDLib::TCh(c4ToCheck));
    else if (eType == EASCIITypes::AlphaNum)
        bRet = TRawStr::bIsAlphaNum(tCIDLib::TCh(c4ToCheck));
    else if (eType == EASCIITypes::Punct)
        bRet = TRawStr::bIsPunct(tCIDLib::TCh(c4ToCheck));
    else if (eType == EASCIITypes::Space)
        bRet = TRawStr::bIsSpace(tCIDLib::TCh(c4ToCheck));
    else
    {
        // Note that we are logging a CIDLib error here, not one of our own!
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_BadEnumValue
            , tCIDLib::ESeverities::ProcFatal
            , tCIDLib::EErrClasses::BadParms
            , TCardinal(tCIDLib::c4EnumOrd(eType))
            , TString(L"TGenProtoISASCIIXNode::EASCIITypes")
        );
    }
    return bRet;
}


TGenProtoIsASCIIXNode::EASCIITypes
TGenProtoIsASCIIXNode::eTokToType(const tGenProtoS::ETokens eToXlat) const
{
    EASCIITypes eRet;
    switch(eToXlat)
    {
        case tGenProtoS::ETokens::IsASCIIAlpha :
            eRet = EASCIITypes::Alpha;
            break;

        case tGenProtoS::ETokens::IsASCIIAlphaNum :
            eRet = EASCIITypes::AlphaNum;
            break;

        case tGenProtoS::ETokens::IsASCIIDecDigit :
            eRet = EASCIITypes::DecDigit;
            break;

        case tGenProtoS::ETokens::IsASCIIHexDigit :
            eRet = EASCIITypes::HexDigit;
            break;

        case tGenProtoS::ETokens::IsASCIIPunct :
            eRet = EASCIITypes::Punct;
            break;

        case tGenProtoS::ETokens::IsASCIISpace :
            eRet = EASCIITypes::Space;
            break;

        default :
            // Note that we are logging a CIDLib error here, not one of our own!
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::ProcFatal
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(tCIDLib::c4EnumOrd(eToXlat))
                , TString(L"TGenProtoISASCIIXNode::EASCIITypes")
            );
            break;
    }
    return eRet;
}


const tCIDLib::TCh*
TGenProtoIsASCIIXNode::pszTypeToName(const EASCIITypes eToXlat) const
{
    const tCIDLib::TCh* pszRet;
    switch(eToXlat)
    {
        case EASCIITypes::Alpha :
            pszRet = L"IsASCIIAlpha";
            break;

        case EASCIITypes::AlphaNum :
            pszRet = L"IsASCIIAlphaNum";
            break;

        case EASCIITypes::DecDigit :
            pszRet = L"IsASCIIDecDigit";
            break;

        case EASCIITypes::HexDigit :
            pszRet = L"IsASCIIHexDigit";
            break;

        case EASCIITypes::Punct :
            pszRet = L"IsASCIIPunct";
            break;

        case EASCIITypes::Space :
            pszRet = L"IsASCIISpace";
            break;

        default :
            // Note that we are logging a CIDLib error here, not one of our own!
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::ProcFatal
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(tCIDLib::c4EnumOrd(eToXlat))
                , TString(L"TGenProtoISASCIIXNode::EASCIITypes")
            );
            break;
    };
    return pszRet;
}


const tCIDLib::TCh*
TGenProtoIsASCIIXNode::pszTokToName(const tGenProtoS::ETokens eToXlat) const
{
    const tCIDLib::TCh* pszRet;
    switch(eToXlat)
    {
        case tGenProtoS::ETokens::IsASCIIAlpha :
            pszRet = L"IsASCIIAlpha";
            break;

        case tGenProtoS::ETokens::IsASCIIAlphaNum :
            pszRet = L"IsASCIIAlphaNum";
            break;

        case tGenProtoS::ETokens::IsASCIIDecDigit :
            pszRet = L"IsASCIIDecDigit";
            break;

        case tGenProtoS::ETokens::IsASCIIHexDigit :
            pszRet = L"IsASCIIHexDigit";
            break;

        case tGenProtoS::ETokens::IsASCIIPunct :
            pszRet = L"IsASCIIPunct";
            break;

        case tGenProtoS::ETokens::IsASCIISpace :
            pszRet = L"IsASCIISpace";
            break;

        default :
            // Note that we are logging a CIDLib error here, not one of our own!
            facCIDLib().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCIDErrs::errcGen_BadEnumValue
                , tCIDLib::ESeverities::ProcFatal
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(tCIDLib::c4EnumOrd(eToXlat))
                , TString(L"tGenProtoS::ETokens")
            );
            break;
    }
    return pszRet;
}


