//
// FILE NAME: CQLogicSh_MathFld.cpp
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
//  This file implements a virtual field derivative
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQSLLDMath,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQLogicSh_MathFld
{
    namespace
    {
        // -------------------------------------------------------------------
        //  Our persistent format
        // -------------------------------------------------------------------
        const tCIDLib::TCard1   c1FmtVersion = 1;


        // -------------------------------------------------------------------
        //  Some names of things we look for
        // -------------------------------------------------------------------
        const TString   strFunc_Abs(L"Abs");
        const TString   strFunc_Cosine(L"Cosine");
        const TString   strFunc_NLog(L"NLog");
        const TString   strFunc_Power(L"Power");
        const TString   strFunc_Sine(L"Sine");
        const TString   strFunc_SqRoot(L"SqRoot");
        const TString   strFunc_ToCard(L"ToCard");
        const TString   strFunc_ToFloat(L"ToFloat");
        const TString   strFunc_ToInt(L"ToInt");
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDMath::TNode
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDMath::TNode: Constructors and Destructor
// ---------------------------------------------------------------------------

TCQSLLDMath::TNode::TNode(  const   ENodeTypes                  eOp
                            ,       TCQSLLDMath::TNode* const   pnodeLeft
                            ,       TCQSLLDMath::TNode* const   pnodeRight) :

    m_eType(eOp)
    , m_pnodeLeft(pnodeLeft)
    , m_pnodeRight(pnodeRight)
{
    //
    //  The data type won't be known until evaluation time, when we ask our
    //  child nodes what their types are. And the value of course can't be
    //  set until then.
    //
}

TCQSLLDMath::TNode::TNode(  const   ENodeTypes                  eOp
                            ,       TCQSLLDMath::TNode* const   pnodeLeft
                            ,       TCQSLLDMath::TNode* const   pnodeRight
                            , const tCQCKit::EFldTypes          eFldType) :
    m_eFldType(eFldType)
    , m_eType(eOp)
    , m_pnodeLeft(pnodeLeft)
    , m_pnodeRight(pnodeRight)
{
}

TCQSLLDMath::TNode::TNode(  const   ENodeTypes                  eFunc
                            ,       TCQSLLDMath::TNode* const   pnodeInside
                            , const tCQCKit::EFldTypes          eFldType) :

    m_eFldType(eFldType)
    , m_eType(eFunc)
    , m_pnodeLeft(pnodeInside)
    , m_pnodeRight(nullptr)
{
}

TCQSLLDMath::TNode::TNode(const tCIDLib::TCard4 c4Literal) :

    m_c4Value(c4Literal)
    , m_eFldType(tCQCKit::EFldTypes::Card)
    , m_eType(ENodeTypes::Unsigned)
    , m_pnodeLeft(nullptr)
    , m_pnodeRight(nullptr)
{
}

TCQSLLDMath::TNode::TNode(const tCIDLib::TFloat8 f8Literal) :

    m_eFldType(tCQCKit::EFldTypes::Float)
    , m_eType(ENodeTypes::Float)
    , m_f8Value(f8Literal)
    , m_pnodeLeft(nullptr)
    , m_pnodeRight(nullptr)
{
}

TCQSLLDMath::TNode::TNode(const tCIDLib::TInt4 i4Literal) :

    m_eFldType(tCQCKit::EFldTypes::Int)
    , m_eType(ENodeTypes::Signed)
    , m_i4Value(i4Literal)
    , m_pnodeLeft(nullptr)
    , m_pnodeRight(nullptr)
{
}

TCQSLLDMath::TNode::TNode(  const   ENodeTypes      eType
                            , const tCIDLib::TCard4 c4Value) :

    m_c4FldIndex(0)
    , m_c4Value(c4Value)
    , m_eType(eType)
    , m_pnodeLeft(nullptr)
    , m_pnodeRight(nullptr)
{
    //
    //  For a field ref node, the data type isn't known until evaluation type
    //  when we know what the type of the field we reference is. And it could
    //  change over time if the field is redefined somehow (such as the
    //  variables driver.)
    //
    //  And, we get a 1 based indicator, so we have to sub one, making sure
    //  it doesn't underflow, and isn't too big.
    //
    if (eType == ENodeTypes::FldRef)
    {
        if (!c4Value || (c4Value > kCQLogicSh::c4MaxSrcFields))
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcCfg_BadFldIndex2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Value)
            );
        }

        //
        //  Looks ok, so decrement it and store in the field index. The
        //  value field doesn't matter then until it's set at evaluation
        //  time.
        //
        m_c4FldIndex = m_c4Value - 1;
    }
}

TCQSLLDMath::TNode::~TNode()
{
    //
    //  Delete our left/right nodes if we have them. This will recursively
    //  clean up the tree.
    //
    try
    {
        delete m_pnodeLeft;
        delete m_pnodeRight;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// Some helpers to get the current value, cast to the indicated type
tCIDLib::TCard4 TCQSLLDMath::TNode::c4AsCard() const
{
    switch(m_eFldType)
    {
        case tCQCKit::EFldTypes::Card :
            return m_c4Value;

        case tCQCKit::EFldTypes::Float :
            return tCIDLib::TCard4(m_f8Value);

        case tCQCKit::EFldTypes::Int :
            return tCIDLib::TCard4(m_i4Value);
    };

    // We shouldn't get here
    CIDAssert2(L"Non-numeric field type seen");
    return 0;
}

tCIDLib::TFloat8 TCQSLLDMath::TNode::f8AsFloat() const
{
    switch(m_eFldType)
    {
        case tCQCKit::EFldTypes::Card :
            return tCIDLib::TFloat8(m_c4Value);

        case tCQCKit::EFldTypes::Float :
            return m_f8Value;

        case tCQCKit::EFldTypes::Int :
            return tCIDLib::TFloat8(m_i4Value);
    };

    // We shouldn't get here
    CIDAssert2(L"Non-numeric field type seen");
    return 0;
}

tCIDLib::TInt4 TCQSLLDMath::TNode::i4AsInt() const
{
    switch(m_eFldType)
    {
        case tCQCKit::EFldTypes::Card :
            return tCIDLib::TInt4(m_c4Value);

        case tCQCKit::EFldTypes::Float :
            return tCIDLib::TInt4(m_f8Value);

        case tCQCKit::EFldTypes::Int :
            return m_i4Value;
    };

    // We shouldn't get here
    CIDAssert2(L"Non-numeric field type seen");
    return 0;
}


//
//  Handles recursive evaluation of the value. We drill down to the terminal
//  nodes, then start building back up and combining.
//
tCIDLib::TVoid TCQSLLDMath::TNode::Evaluate(const tCQLogicSh::TInfoList& colVals)
{
    if (m_pnodeLeft && !m_pnodeRight)
    {
        //
        //  This one operates on a single child node, which may be a single factor or
        //  a whole tree. So ask it to evaluate itself recursively before we do our thing.
        //
        m_pnodeLeft->Evaluate(colVals);

        // Perform our configured operation on the result
        switch(m_eType)
        {
            case ENodeTypes::Abs :
                m_f8Value = TMathLib::f8Abs(m_pnodeLeft->f8AsFloat());
                break;

            case ENodeTypes::Cosine :
                m_f8Value = TMathLib::f8Cosine(m_pnodeLeft->f8AsFloat());
                break;

            case ENodeTypes::NLog :
                m_f8Value = TMathLib::f8Log(m_pnodeLeft->f8AsFloat());
                break;

            case ENodeTypes::Sine :
                m_f8Value = TMathLib::f8Sine(m_pnodeLeft->f8AsFloat());
                break;

            case ENodeTypes::SqRoot :
                m_f8Value = TMathLib::f8SqrRoot(m_pnodeLeft->f8AsFloat());
                break;

            case ENodeTypes::ToCard :
                m_c4Value = m_pnodeLeft->c4AsCard();
                break;

            case ENodeTypes::ToFloat :
                m_f8Value = m_pnodeLeft->f8AsFloat();
                break;

            case ENodeTypes::ToInt :
                m_i4Value = m_pnodeLeft->i4AsInt();
                break;

            default :
                CIDAssert2(L"Unknown function node type");
                break;
        }
    }
     else if (m_pnodeLeft && m_pnodeRight)
    {
        //
        //  We are a left/right operation type of node, so ask both of
        //  our contained nodes to evaluate themselves. This includes
        //  functions that have two parameters, as well as operators.
        //
        m_pnodeLeft->Evaluate(colVals);
        m_pnodeRight->Evaluate(colVals);

        //
        //  For some operations we figure out our data type based on the
        //  children. For others, it's fixed.
        //
        switch(m_eType)
        {
            case ENodeTypes::Add :
            case ENodeTypes::Div :
            case ENodeTypes::ModDiv :
            case ENodeTypes::Mul :
            case ENodeTypes::Sub :
                if ((m_pnodeLeft->m_eFldType == tCQCKit::EFldTypes::Float)
                ||  (m_pnodeRight->m_eFldType == tCQCKit::EFldTypes::Float))
                {
                    m_eFldType = tCQCKit::EFldTypes::Float;
                }
                 else if ((m_pnodeLeft->m_eFldType == tCQCKit::EFldTypes::Int)
                      ||  (m_pnodeRight->m_eFldType == tCQCKit::EFldTypes::Int))
                {
                    m_eFldType = tCQCKit::EFldTypes::Int;
                }
                 else
                {
                    m_eFldType = tCQCKit::EFldTypes::Card;
                }
                break;

            case ENodeTypes::AND :
            case ENodeTypes::OR :
            case ENodeTypes::XOR :
                m_eFldType = tCQCKit::EFldTypes::Card;
                break;

            case ENodeTypes::Power :
                m_eFldType = tCQCKit::EFldTypes::Float;
                break;

            default :
                CIDAssert2(L"Node is not op type");
                break;
        };

        //
        //  And combine their values appropriately to create our value,
        //  depending on the type we decided on above.
        //
        if (m_eFldType == tCQCKit::EFldTypes::Card)
        {
            const tCIDLib::TCard4 c4Left = m_pnodeLeft->c4AsCard();
            const tCIDLib::TCard4 c4Right = m_pnodeRight->c4AsCard();
            switch(m_eType)
            {
                case ENodeTypes::Add :
                    m_c4Value = c4Left + c4Right;
                    break;

                case ENodeTypes::Div :
                    m_c4Value = c4Left / c4Right;
                    break;

                case ENodeTypes::ModDiv :
                    m_c4Value = c4Left % c4Right;
                    break;

                case ENodeTypes::Mul :
                    m_c4Value = c4Left * c4Right;
                    break;

                case ENodeTypes::Sub :
                    m_c4Value = c4Left - c4Right;
                    break;

                case ENodeTypes::AND :
                    m_c4Value = c4Left & c4Right;
                    break;

                case ENodeTypes::OR :
                    m_c4Value = c4Left | c4Right;
                    break;

                case ENodeTypes::XOR :
                    m_c4Value = c4Left ^ c4Right;
                    break;

                default :
                    CIDAssert2(L"Node is not op type");
                    break;
            };
        }
         else if (m_eFldType == tCQCKit::EFldTypes::Float)
        {
            const tCIDLib::TFloat8 f8Left = m_pnodeLeft->f8AsFloat();
            const tCIDLib::TFloat8 f8Right = m_pnodeRight->f8AsFloat();
            switch(m_eType)
            {
                case ENodeTypes::Add :
                    m_f8Value = f8Left + f8Right;
                    break;

                case ENodeTypes::Div :
                    m_f8Value = f8Left / f8Right;
                    break;

                case ENodeTypes::ModDiv :
                    m_f8Value = TMathLib::f8Mod(f8Left, f8Right);
                    break;

                case ENodeTypes::Mul :
                    m_f8Value = f8Left * f8Right;
                    break;

                case ENodeTypes::Power :
                    m_f8Value = TMathLib::f8Power(f8Left, f8Right);
                    break;

                case ENodeTypes::Sub :
                    m_f8Value = f8Left - f8Right;
                    break;

                default :
                    CIDAssert2(L"Node is not op type");
                    break;
            };
        }
         else if (m_eFldType == tCQCKit::EFldTypes::Int)
        {
            const tCIDLib::TInt4 i4Left = m_pnodeLeft->i4AsInt();
            const tCIDLib::TInt4 i4Right = m_pnodeRight->i4AsInt();
            switch(m_eType)
            {
                case ENodeTypes::Add :
                    m_i4Value = i4Left + i4Right;
                    break;

                case ENodeTypes::Div :
                    m_i4Value = i4Left / i4Right;
                    break;

                case ENodeTypes::ModDiv :
                    m_i4Value = i4Left % i4Right;
                    break;

                case ENodeTypes::Mul :
                    m_i4Value = i4Left * i4Right;
                    break;

                case ENodeTypes::Sub :
                    m_i4Value = i4Left - i4Right;
                    break;

                default :
                    CIDAssert2(L"Node is not op type");
                    break;
            };
        }
         else
        {
            CIDAssert2(L"Non-numeric field type seen");
        }
    }
     else if (m_eType == ENodeTypes::FldRef)
    {
        // We are a field ref. So set our value and type to the field's
        const TCQCFldPollInfo& cfpiUs = colVals[m_c4FldIndex];
        m_eFldType = cfpiUs.flddAssoc().eType();
        switch(m_eFldType)
        {
            case tCQCKit::EFldTypes::Card :
                m_c4Value = static_cast<const TCQCCardFldValue&>(cfpiUs.fvCurrent()).c4Value();
                break;

            case tCQCKit::EFldTypes::Float :
                m_f8Value = static_cast<const TCQCFloatFldValue&>(cfpiUs.fvCurrent()).f8Value();
                break;

            case tCQCKit::EFldTypes::Int :
                m_i4Value = static_cast<const TCQCIntFldValue&>(cfpiUs.fvCurrent()).i4Value();
                break;

            default :
                CIDAssert2(L"Non-numeric field type seen");
                break;
        };
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDMath
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDMath: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLLDMath::TCQSLLDMath(const TString& strName, const tCQCKit::EFldTypes eType) :

    TCQLSrvFldType(strName, eType, tCQCKit::EFldAccess::Read)
    , m_pnodeRoot(nullptr)
{
}

TCQSLLDMath::TCQSLLDMath(const TCQSLLDMath& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_pnodeRoot(nullptr)
    , m_strFormula(clsftSrc.m_strFormula)
{
}

TCQSLLDMath::~TCQSLLDMath()
{
    // Clean up our evaluation tree
    delete m_pnodeRoot;
    m_pnodeRoot = nullptr;
}


// ---------------------------------------------------------------------------
//  TCQSLLDMath: Public oeprators
// ---------------------------------------------------------------------------
TCQSLLDMath& TCQSLLDMath::operator=(const TCQSLLDMath& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);
        m_strFormula = clsftSrc.m_strFormula;

        // Clean up our evaluation tree
        delete m_pnodeRoot;
        m_pnodeRoot = nullptr;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDMath: Public, inherited methods
// ---------------------------------------------------------------------------

// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDMath::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );
    const TCQSLLDMath& clsftAct = static_cast<const TCQSLLDMath&>(clsftRHS);

    //
    //  We just compare the formula text. Yeh, it's possible that it's the
    //  same effective formula if we parsed it out, but it's not worth
    //  dealing with.
    //
    return (m_strFormula == clsftAct.m_strFormula);
}


tCIDLib::TBoolean
TCQSLLDMath::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // We can do any numeric type
    return flddToCheck.bIsNumericType();
}


tCIDLib::TBoolean
TCQSLLDMath::bValidate(         TString&            strErr
                        ,       tCIDLib::TCard4&    c4SrcFldInd
                        , const TCQCFldCache&       cfcData)
{
    // Call our parent first
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    // We don't have any source field related stuff that could be in error
    c4SrcFldInd = kCIDLib::c4MaxCard;

    // Just parse the formula and return its return
    return bParseExpression(strErr);
}


tCIDLib::TVoid TCQSLLDMath::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    //
    //  We need to get the formula parsed, so that the tree is in place.
    //  This is only called when loaded into the server side driver, to give
    //  nodes like this a chance between the field getting registered and
    //  beginning the evaluation of values.
    //
    //  In theory this can't fail because the client would not let them save
    //  any formula that's not parseable. But, if it does, it'll throw
    //  back to the driver, which will remove us from the list.
    //
    TString strErr;
    bParseExpression(strErr);
}


TCQCFldFilter* TCQSLLDMath::pffiltToUse()
{
    // We only can accept numeric fields
    return new TCQCFldFiltNumeric(tCQCKit::EReqAccess::MReadCWrite, kCIDLib::True);
}



// ---------------------------------------------------------------------------
//  TCQSLLDMath: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Parses the formula and returns a success or failure. This is used by
//  the client side to do validation. On the server side it is used to create
//  the tree for value evaluation. In theory it can't fail on the server side
//  since the client won't let them save one that doesn' parse.
//
tCIDLib::TBoolean TCQSLLDMath::bParseExpression(TString& strError)
{
    // Start the recusrive parsing process
    TTextStringInStream strmSrc(&m_strFormula);
    try
    {
        // Clean up any existing tree
        delete m_pnodeRoot;
        m_pnodeRoot = nullptr;

        // Clear the pushback char
        m_chPush = kCIDLib::chNull;

        //
        //  To make things easier, we check here for any outer-most parens.
        //  If we see an opening one first, then we have to see a close one
        //  at the end.
        //
        SkipSpace(strmSrc);
        const tCIDLib::TBoolean bNeedParen = bCheckNextChar
        (
            strmSrc, kCIDLib::chOpenParen, kCIDLib::False
        );

        //
        //  Now parse the first clause to get us a root node. That might
        //  get the whole thing if it's all parenthesized, or there is only
        //  one clause in all. It might also only be a factor, e.g. a single
        //  function call and though it wouldn't make sense it could just be
        //  a single field reference of literal value. So we indicate that
        //  it can be a factor.
        //
        m_pnodeRoot = pnodeParseClause(strmSrc, kCIDLib::True);

        //
        //  Now just keep looking forward for another op. If we get one, then
        //  parse a right side factor for it and make the old root and the
        //  new right children of a new root.
        //
        TNode::ENodeTypes eType;
        while (bParseOp(strmSrc, eType, kCIDLib::False))
        {
            //
            //  Parse a right side factor. This could create another tree
            //  underneath this node if it's a parenthesized clause that makes
            //  up this factor.
            //
            TNode* pnodeRight = pnodeParseFactor(strmSrc);

            // And create a new root that includes these
            m_pnodeRoot = new TNode(eType, m_pnodeRoot, pnodeRight);
        }

        // If we saw an open paren, we have to see a close one
        if (bNeedParen)
            bCheckNextChar(strmSrc, kCIDLib::chCloseParen);

        // Skip any space and we have to be at the end of the stream then
        SkipSpace(strmSrc);

        if (!strmSrc.bEndOfStream())
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcMath_ExpectedOp
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
    }

    catch(const TError& errTCatch)
    {
        // Clean up the tree we built
        delete m_pnodeRoot;
        m_pnodeRoot = nullptr;

        // Give the caller back the error text
        strError = errTCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  Get/set the formula string. The set is only ever called from the client
//  side driver during config, so we don't have to worry about updating the
//  evaluation tree.
//
const TString& TCQSLLDMath::strFormula() const
{
    return m_strFormula;
}

const TString& TCQSLLDMath::strFormula(const TString& strToSet)
{
    m_strFormula = strToSet;
    return m_strFormula;
}


// ---------------------------------------------------------------------------
//  TCQSLLDMath: Hidden Constructors
// ---------------------------------------------------------------------------

// Needed for polymorphic streaming
TCQSLLDMath::TCQSLLDMath() :

    TCQLSrvFldType()
    , m_pnodeRoot(nullptr)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDMath: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just run through the pattern string and build up a new value from the
//  source fields. This won't be called if any of the fields are in error
//  state, so we don't have to worry about that. Legal values should be
//  available for all of the fields. However, that could have changed since
//  the caller checked, in which case an exception will be thrown and the
//  caller has to deal with it.
//
//  We make a recursive call on the root node that will do a depth first
//  evaluation of all the nodes. Field ref nodes will have their type set
//  as they are evaluated, to match that of the field.
//
tCQLogicSh::EEvalRes
TCQSLLDMath::eBuildValue(const  tCQLogicSh::TInfoList&  colVals
                        ,       TCQCFldValue&           fldvToFill
                        , const tCIDLib::TCard4
                        , const tCIDLib::TCard4         )
{
    //
    //  If the pointer is null, then we couldn't parse the formula for
    //  some reason, so we indicate failure
    //
    if (!m_pnodeRoot)
        return tCQLogicSh::EEvalRes::Error;

    // Ask the root to evaluate
    m_pnodeRoot->Evaluate(colVals);

    // And get the resulting value as our type and put it into the value
    tCIDLib::TBoolean bChanged = kCIDLib::False;
    switch(flddCfg().eType())
    {
        case tCQCKit::EFldTypes::Card :
            bChanged = static_cast<TCQCCardFldValue&>(fldvToFill).bSetValue
            (
                m_pnodeRoot->c4AsCard()
            );
            break;

        case tCQCKit::EFldTypes::Float :
            bChanged = static_cast<TCQCFloatFldValue&>(fldvToFill).bSetValue
            (
                m_pnodeRoot->f8AsFloat()
            );
            break;

        case tCQCKit::EFldTypes::Int :
            bChanged = static_cast<TCQCIntFldValue&>(fldvToFill).bSetValue
            (
                m_pnodeRoot->i4AsInt()
            );
            break;

        default :
            bChanged = kCIDLib::False;
            break;
    };

    if (bChanged)
        return tCQLogicSh::EEvalRes::NewValue;
    return tCQLogicSh::EEvalRes::NoChange;
}


tCIDLib::TVoid TCQSLLDMath::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_MathFld::c1FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c1FmtVersion)
            , clsThis()
        );
    }

    // Do our parent's stuff
    TParent::StreamFrom(strmToReadFrom);

    // Stream our stuff
    strmToReadFrom >> m_strFormula;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDMath::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_MathFld::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_strFormula
                    << tCIDLib::EStreamMarkers::EndObject;
}



// ---------------------------------------------------------------------------
//  TCQSLLDMath: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Checks that the next character is the indicated one, and eats it if so.
//  If not, it either returns false or throws, depending on the bMustBe parm.
//
tCIDLib::TBoolean
TCQSLLDMath::bCheckNextChar(        TTextStringInStream&    strmSrc
                            , const tCIDLib::TCh            chToCheck
                            , const tCIDLib::TBoolean       bMustBe)
{
    tCIDLib::TCh chNext = chGetNext(strmSrc, kCIDLib::True);

    // If it's our guy, then return true
    if (chNext == chToCheck)
        return kCIDLib::True;

    // It's not, so throw or push it back and return false
    if (bMustBe)
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_ExpectedChar
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TString(chToCheck)
            , TString(chNext)
        );
    }

    m_chPush = chNext;
    return kCIDLib::False;
}


//
//  Parses out an operation and returns it. If not found it will return false
//  or throw, depending on the last parm.
//
tCIDLib::TBoolean
TCQSLLDMath::bParseOp(          TTextStringInStream&    strmSrc
                        ,       TNode::ENodeTypes&      eToFill
                        , const tCIDLib::TBoolean       bThrowIfNot)
{
    //
    //  Skip any space. If not at the end of stream, get the next char. If we
    //  are, then the char will remain null, and we'll either fail or return
    //  false below.
    //
    SkipSpace(strmSrc);
    tCIDLib::TCh chOp = kCIDLib::chNull;
    if (!strmSrc.bEndOfStream())
        chOp = chGetNext(strmSrc);

    switch(chOp)
    {
        case kCIDLib::chPlusSign :
            eToFill = TNode::ENodeTypes::Add;
            break;

        case kCIDLib::chAmpersand :
            eToFill = TNode::ENodeTypes::AND;
            break;

        case kCIDLib::chForwardSlash :
            eToFill = TNode::ENodeTypes::Div;
            break;

        case kCIDLib::chPercentSign :
            eToFill = TNode::ENodeTypes::ModDiv;
            break;

        case kCIDLib::chAsterisk :
            eToFill = TNode::ENodeTypes::Mul;
            break;

        case kCIDLib::chVerticalBar :
            eToFill = TNode::ENodeTypes::OR;
            break;

        case kCIDLib::chHyphenMinus :
            eToFill = TNode::ENodeTypes::Sub;
            break;

        case kCIDLib::chCircumflex :
            eToFill = TNode::ENodeTypes::XOR;
            break;


        default :
        {
            if (bThrowIfNot)
            {
                facCQLogicSh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLShErrs::errcMath_ExpectedOp
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}



// Return the next available character
tCIDLib::TCh
TCQSLLDMath::chGetNext(         TTextStringInStream&    strmSrc
                        , const tCIDLib::TBoolean       bEndOk)
{
    // If we have a pushed back char, then return it and clear it
    if (m_chPush)
    {
        const tCIDLib::TCh chRet = m_chPush;
        m_chPush = kCIDLib::chNull;
        return chRet;
    }

    // Not one, so get one from the stream. If no more, then throw or return null
    if (strmSrc.bEndOfStream())
    {
        if (bEndOk)
            return kCIDLib::chNull;

        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_UnexpectedEnd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    tCIDLib::TCh chRet;
    strmSrc >> chRet;

    return chRet;
}


// Peak the next available character
tCIDLib::TCh TCQSLLDMath::chPeekNext(TTextStringInStream& strmSrc)
{
    // If we have a pushed back char, then return it
    if (m_chPush)
        return m_chPush;

    // If at the end of stream, return a null
    if (strmSrc.bEndOfStream())
        return kCIDLib::chNull;

    // Get the next char, then push it back and return it
    tCIDLib::TCh chRet;
    strmSrc >> chRet;
    m_chPush = chRet;

    return chRet;
}


// Get's the next char, insures it's a digit and returns it if so, else throws
tCIDLib::TCh TCQSLLDMath::chGetDigit(TTextStringInStream& strmSrc)
{
    const tCIDLib::TCh chCur = chGetNext(strmSrc);
    if (!TRawStr::bIsDigit(chCur))
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_ExpectedDigit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
    return chCur;
}


//
//  A recursive method that parses the formula and builds the evaluation
//  tree. At this point we have to be dealing with a clause. So we parse a
//  left side, an operator, and a right side. If that works, we create a
//  new operation node, set the info on it, and return it.
//
//  We have to deal wtih the special case where it could really be just a
//  factor by itself. In the case of inside a function it could be a factor
//  or clause. The caller tells us if this should be dealt with. Otherwise,
//  it has to really be a clause with two sides.
//
TCQSLLDMath::TNode*
TCQSLLDMath::pnodeParseClause(          TTextStringInStream&    strmSrc
                                , const tCIDLib::TBoolean       bCanBeFactor)
{
    // Parse the left side factor and put a janitor on it until it's stored away
    TJanitor<TNode> janLeft(pnodeParseFactor(strmSrc));

    //
    //  If it's a closed paren or comma, or end of stream, we are done if
    //  that's legal.
    //
    if (bCanBeFactor
    &&  (bCheckNextChar(strmSrc, kCIDLib::chCloseParen, kCIDLib::False)
    ||   bCheckNextChar(strmSrc, kCIDLib::chComma, kCIDLib::False)
    ||   !chPeekNext(strmSrc)))
    {
        return janLeft.pobjOrphan();
    }

    // Next we have to get an operator, or
    TNode::ENodeTypes eType;
    bParseOp(strmSrc, eType, kCIDLib::True);

    // And skip any space and parse the right hand factor
    SkipSpace(strmSrc);
    TJanitor<TNode> janRight(pnodeParseFactor(strmSrc));

    // Ok, we are good, so let's create a new node and return it
    return new TNode(eType, janLeft.pobjOrphan(), janRight.pobjOrphan());
}


//
//  Parses one of the left or right side factors in a clause. At this point,
//  we either see an open paren, in which case this factor is really a clause
//  itself, so we recurse and return that sub-clause as the factor.
//
//  Otherwise, we expect to see one of these:
//
//  1. A field reference, in the form %(x), where x is 1 to 8.
//  2. A literal numeric value
//  3. A function, in the form nnn(x,y,...), where nnn is the name of
//      the function, and x, y, etc... can be either a factor or clause,
//      so we recurse and add the child expressions.
//
TCQSLLDMath::TNode*
TCQSLLDMath::pnodeParseFactor(TTextStringInStream& strmSrc)
{
    tCIDLib::TCh chCur;

    // Skip any space first
    SkipSpace(strmSrc);

    // Check the first character. If it's an open paren, we recurse
    if (bCheckNextChar(strmSrc, kCIDLib::chOpenParen, kCIDLib::False))
    {
        // Put a janitor on the resulting node until we get the close paren
        TNode* pnodeRet = pnodeParseClause(strmSrc);
        TJanitor<TNode> janRet(pnodeRet);

        // We have to see a close paren
        SkipSpace(strmSrc);
        bCheckNextChar(strmSrc, kCIDLib::chCloseParen);

        return janRet.pobjOrphan();
    }

    // If it's a percent sign, then it has to be a field ref
    if (bCheckNextChar(strmSrc, kCIDLib::chPercentSign, kCIDLib::False))
    {
        //
        //  It's got to be a field ref. So the next character has to be a
        //  an open paren, then a digit 1 to 9, then a close paren.
        //
        bCheckNextChar(strmSrc, kCIDLib::chOpenParen);
        const tCIDLib::TCh chIndex = chGetDigit(strmSrc);
        bCheckNextChar(strmSrc, kCIDLib::chCloseParen);

        //
        //  Get the field index and let's look up this field. We need to make
        //  sure the index is valid for the number of source fields we have.
        //
        const tCIDLib::TCard4 c4Index(chIndex - kCIDLib::chDigit0);
        if (c4Index > c4SrcFldCount())
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcCfg_BadFldIndex2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(c4Index)
            );
        }
        return new TNode(TNode::ENodeTypes::FldRef, c4Index);
    }

    // If it's alpha, then it has to be a function name
    chCur = chPeekNext(strmSrc);
    if (TRawStr::bIsAlpha(chCur))
        return pnodeParseFunc(strmSrc);

    //
    //  It's nothing special, so it has to be a literal numeric. If we got a
    //  leading sign, append it and move forward. Then we ony have to deal
    //  with digits and potentially a decimal for a float value.
    //
    tCIDLib::TBoolean bSawSign = kCIDLib::False;
    TString strVal;
    chCur = chGetNext(strmSrc);
    if ((chCur == kCIDLib::chHyphenMinus)
    ||  (chCur == kCIDLib::chPlusSign))
    {
        strVal.Append(chCur);
        chCur = chGetNext(strmSrc);
        bSawSign = kCIDLib::True;
    }

    tCIDLib::TBoolean bSawDecimal = kCIDLib::False;
    tCIDLib::TCard4   c4DigitCnt = 0;
    while (kCIDLib::True)
    {
        if (TRawStr::bIsDigit(chCur))
        {
            // Bump the digit count
            c4DigitCnt++;
        }
         else if (chCur == kCIDLib::chPeriod)
        {
            // We can't have already seen a period
            if (bSawDecimal)
            {
                facCQLogicSh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLShErrs::errcMath_MultipleDecimals
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }

            // Indicate we've seen it now
            bSawDecimal = kCIDLib::True;
        }
         else if (TRawStr::bIsSpace(chCur)
              ||  (chCur == kCIDLib::chAsterisk)
              ||  (chCur == kCIDLib::chCloseParen)
              ||  (chCur == kCIDLib::chForwardSlash)
              ||  (chCur == kCIDLib::chHyphenMinus)
              ||  (chCur == kCIDLib::chPercentSign)
              ||  (chCur == kCIDLib::chPlusSign)
              ||  (chCur == kCIDLib::chComma))
        {
            // We are done. If no digits, it's not legal
            if (!c4DigitCnt)
            {
                facCQLogicSh().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLShErrs::errcMath_ExpectedNumLit
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }

            // If not a space, push it back to be seen later
            if (!TRawStr::bIsSpace(chCur))
                m_chPush = chCur;
            break;
        }
         else
        {
            // Can't be a valid numeric literal
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcMath_ExpectedNumLit
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        //
        //  Seems ok, so store and get another. This is one place where
        //  we need to be tolerant of end of stream. Even if it's not really
        //  legal, when it does end legally at a non-pre-kknown point, it
        //  would have to be at the end of one of these.
        //
        //  So we return whatever we get. If there was really more to parse,
        //  it ca be more easily dealt with in the various other places that
        //  will be called after this.
        //
        strVal.Append(chCur);
        chCur = chGetNext(strmSrc, kCIDLib::True);

        // So if we hit the end, break out
        if (chCur == kCIDLib::chNull)
            break;
    }

    //
    //  If we saw a decimal, it has to be a float. If we saw a sign, make
    //  it an int, else a card.
    //
    if (bSawDecimal)
    {
        tCIDLib::TFloat8 f8Val;
        if (!strVal.bToFloat8(f8Val))
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcMath_CantCvLit
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strVal
            );
        }
        return new TNode(f8Val);
    }
     else if (bSawSign)
    {
        tCIDLib::TInt4 i4Val;
        if (!strVal.bToInt4(i4Val))
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcMath_CantCvLit
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , strVal
            );
        }
        return new TNode(i4Val);
    }

    tCIDLib::TCard4 c4Val;
    if (!strVal.bToCard4(c4Val))
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_CantCvLit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strVal
        );
    }
    return new TNode(c4Val);
}


//
//  Parses out a function name. It has to be ASCII chars, ended by an open
//  paren or a comma.
//
tCIDLib::TVoid
TCQSLLDMath::ParseIdent(TTextStringInStream& strmSrc, TString& strToFill)
{
    strToFill.Clear();
    while(kCIDLib::True)
    {
        const tCIDLib::TCh chCur = chGetNext(strmSrc);
        if ((chCur == kCIDLib::chOpenParen) || (chCur == kCIDLib::chComma))
            break;

        if (!TRawStr::bIsAlpha(chCur))
        {
            facCQLogicSh().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQLShErrs::errcMath_ExpectedName
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }
        strToFill.Append(chCur);
    }

    // Make sure we got at least a character
    if (strToFill.bIsEmpty())
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_ExpectedName
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }
}


//
//  Parses a function. We get the identifier, the opening paren, the contained
//  clause or factor, and the closing paren.
//
TCQSLLDMath::TNode* TCQSLLDMath::pnodeParseFunc(TTextStringInStream& strmSrc)
{
    // Parse out an indentifer
    ParseIdent(strmSrc, m_strTmp);

    // Make sure it's a valid function
    TNode::ENodeTypes eType;
    tCQCKit::EFldTypes eFldType;
    if (m_strTmp == CQLogicSh_MathFld::strFunc_Abs)
    {
        eType = TNode::ENodeTypes::Abs;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_Cosine)
    {
        eType = TNode::ENodeTypes::Cosine;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_NLog)
    {
        eType = TNode::ENodeTypes::NLog;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_Power)
    {
        eType = TNode::ENodeTypes::Power;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_Sine)
    {
        eType = TNode::ENodeTypes::Sine;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_SqRoot)
    {
        eType = TNode::ENodeTypes::SqRoot;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_ToCard)
    {
        eType = TNode::ENodeTypes::ToCard;
        eFldType = tCQCKit::EFldTypes::Card;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_ToFloat)
    {
        eType = TNode::ENodeTypes::ToFloat;
        eFldType = tCQCKit::EFldTypes::Float;
    }
     else if (m_strTmp == CQLogicSh_MathFld::strFunc_ToInt)
    {
        eType = TNode::ENodeTypes::ToInt;
        eFldType = tCQCKit::EFldTypes::Int;
    }
     else
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcMath_UnknownFunc
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , m_strTmp
        );

        // Won't happen but makes the analyzer happy
        return nullptr;
    }

    //
    //  Now parse out the contents inside it. Tell it that it can be a
    //  clause or factor in the single value case. For the double each side
    //  has to be a factor.
    //
    TNode* pnodeRet = nullptr;
    if (eType == TNode::ENodeTypes::Power)
    {
        TNode* pnodeLeft = pnodeParseClause(strmSrc, kCIDLib::True);
        TNode* pnodeRight = pnodeParseClause(strmSrc, kCIDLib::True);

        pnodeRet = new TNode(eType, pnodeLeft, pnodeRight, eFldType);
    }
     else
    {
        TNode* pnodeInside = pnodeParseClause(strmSrc, kCIDLib::True);
        pnodeRet = new TNode(eType, pnodeInside, eFldType);
    }

    return pnodeRet;
}


//
//  Whitespace is not relevant, so we need to skip it anywhere it is found.
//  Whitspace can legitimately be at the end of input, so we are tolerant of
//  that here. If it's not really legal to end here, it'll be caught when
//  they try to parse whatever comes next.
//
tCIDLib::TVoid TCQSLLDMath::SkipSpace(TTextStringInStream& strmSrc)
{
    while (kCIDLib::True)
    {
        const tCIDLib::TCh chCur = chGetNext(strmSrc, kCIDLib::True);

        // If we hit the end of stream, we are done
        if (!chCur)
            break;

        // If not a space, push it back and break out
        if (!TRawStr::bIsSpace(chCur))
        {
            m_chPush = chCur;
            break;
        }
    }
}


