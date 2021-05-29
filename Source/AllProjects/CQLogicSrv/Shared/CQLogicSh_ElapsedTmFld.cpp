//
// FILE NAME: CQLogicSh_ElapsedTmFld.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2013
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
//  This file implements a virtual field derivative that creates a boolean
//  result from one or more source fields, by evaluting their values, and
//  tracks how long that result has been true.
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
AdvRTTIDecls(TCQSLLDElapsedTm,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQLogicSh_ElapsedTm
    {
        // -----------------------------------------------------------------------
        //  Our persistent format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDElapsedTm
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  When we call the parent, indicate we are a normal field and want to be
//  called to evaluate our value regularly, not just when our source fields
//  change, since we have to count elapsed time.
//
TCQSLLDElapsedTm::TCQSLLDElapsedTm(const TString& strName) :

    TCQLSrvFldType
    (
        strName
        , tCQCKit::EFldTypes::Time
        , tCQCKit::EFldAccess::Read
        , kCIDLib::True
        , kCIDLib::True
    )
    , m_bAutoReset(kCIDLib::True)
    , m_bPrevState(kCIDLib::False)
    , m_eLogOp(tCQCKit::ELogOps::AND)
    , m_enctElapsed(0)
    , m_enctLast(0)
{
}

TCQSLLDElapsedTm::TCQSLLDElapsedTm(const TCQSLLDElapsedTm& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_bAutoReset(clsftSrc.m_bAutoReset)
    , m_bPrevState(clsftSrc.m_bPrevState)
    , m_eLogOp(clsftSrc.m_eLogOp)
    , m_colExprList(clsftSrc.m_colExprList)
    , m_enctElapsed(clsftSrc.m_enctElapsed)
    , m_enctLast(clsftSrc.m_enctLast)
{
}

TCQSLLDElapsedTm::~TCQSLLDElapsedTm()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Public oeprators
// ---------------------------------------------------------------------------
TCQSLLDElapsedTm&
TCQSLLDElapsedTm::operator=(const TCQSLLDElapsedTm& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);
        m_bAutoReset    = clsftSrc.m_bAutoReset;
        m_bPrevState    = clsftSrc.m_bPrevState;
        m_eLogOp        = clsftSrc.m_eLogOp;
        m_colExprList   = clsftSrc.m_colExprList;
        m_enctElapsed   = clsftSrc.m_enctElapsed;
        m_enctLast      = clsftSrc.m_enctLast;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Public, inherited methods
// ---------------------------------------------------------------------------

// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean TCQSLLDElapsedTm::bAddField(const TString& strToAdd)
{
    //
    //  Call our parent first. If he adds it, then we add an expression. We just set it
    //  to statement type for the initial value.
    //
    if (TParent::bAddField(strToAdd))
    {
        m_colExprList.objPlace
        (
            L"Time Test"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsGThan
            , TString(L"0")
            , kCIDLib::False
        );
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDElapsedTm::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );
    const TCQSLLDElapsedTm& clsftAct = static_cast<const TCQSLLDElapsedTm&>(clsftRHS);

    // Compare the logical op
    if (m_eLogOp != clsftAct.m_eLogOp)
        return kCIDLib::False;

    // That's ok, so compare the expressions. If not the same count, then not
    const tCIDLib::TCard4 c4Count = m_colExprList.c4ElemCount();
    if (c4Count != clsftAct.m_colExprList.c4ElemCount())
        return kCIDLib::False;

    // See if they are both the same auto/manual reset mode
    if (m_bAutoReset != clsftAct.m_bAutoReset)
        return kCIDLib::False;

    // They are both the same size lists, so compare them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colExprList[c4Index] != clsftAct.m_colExprList[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQSLLDElapsedTm::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // We can do any type
    return kCIDLib::True;
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean
TCQSLLDElapsedTm::bMoveField(const  tCIDLib::TCard4     c4At
                            , const tCIDLib::TBoolean   bUp)
{
    //
    //  Call our parent first. If he moves it, then we move the same. WE don't
    //  have to verify the index, since he already proved it's ok.
    //
    if (TParent::bMoveField(c4At, bUp))
    {
        if (bUp)
            m_colExprList.SwapItems(c4At, c4At - 1);
        else
            m_colExprList.SwapItems(c4At, c4At + 1);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQSLLDElapsedTm::bValidate(        TString&            strErr
                            ,       tCIDLib::TCard4&    c4SrcFldInd
                            , const TCQCFldCache&       cfcData)
{
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    // Validate all of the expressions
    TString strField;
    TString strMoniker;
    const tCIDLib::TCard4 c4Count = m_colExprList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCExpression& exprCur = m_colExprList[c4Index];

        // Look up this source field
        facCQCKit().bParseFldName(strSrcFldAt(c4Index), strMoniker, strField);
        const TCQCFldDef& flddSrc = cfcData.flddFor
        (
            strMoniker, strField
        );

        if (!exprCur.bValidate(strErr, flddSrc.eType()))
        {
            c4SrcFldInd = c4Index;
            return kCIDLib::False;
        }
    }

    // No source field error so set it to max card
    c4SrcFldInd = kCIDLib::c4MaxCard;

    return kCIDLib::True;
}


tCIDLib::TVoid TCQSLLDElapsedTm::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    m_bPrevState = kCIDLib::False;
    m_enctElapsed = 0;
    m_enctLast = TTime::enctNow();
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TVoid TCQSLLDElapsedTm::RemoveField(const tCIDLib::TCard4 c4At)
{
    // Call our parent first. He'll validate the index
    TParent::RemoveField(c4At);
    m_colExprList.RemoveAt(c4At);
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQSLLDElapsedTm::bAutoReset() const
{
    return m_bAutoReset;
}

tCIDLib::TBoolean
TCQSLLDElapsedTm::bAutoReset(const tCIDLib::TBoolean bToSet)
{
    m_bAutoReset = bToSet;
    return m_bAutoReset;
}


// Set the negated flag on the indicated expression
tCIDLib::TBoolean
TCQSLLDElapsedTm::bNegatedAt(const tCIDLib::TCard4 c4At) const
{
    return m_colExprList[c4At].bNegated();
}


// Get/set our logical operator
tCQCKit::ELogOps TCQSLLDElapsedTm::eLogOp() const
{
    return m_eLogOp;
}

tCQCKit::ELogOps TCQSLLDElapsedTm::eLogOp(const tCQCKit::ELogOps eToSet)
{
    m_eLogOp = eToSet;
    return m_eLogOp;
}


// Provide access to our expressions for the client side driver
const TCQCExpression&
TCQSLLDElapsedTm::exprAt(const tCIDLib::TCard4 c4At) const
{
    return m_colExprList[c4At];
}

TCQCExpression& TCQSLLDElapsedTm::exprAt(const tCIDLib::TCard4 c4At)
{
    return m_colExprList[c4At];
}


//
//  Reset our elapsed time. Also update the last stored time, so that if
//  we are going to start counting again, that we start from this reset
//  time.
//
tCIDLib::TVoid TCQSLLDElapsedTm::ResetTime()
{
    m_enctElapsed = 0;
    m_enctLast = TTime::enctNow();
}


tCIDLib::TVoid
TCQSLLDElapsedTm::SetExpressionAt(const  tCIDLib::TCard4 c4At
                            , const TCQCExpression& exprToSet)
{
    m_colExprList[c4At] = exprToSet;
}


// Set the negated flag on a given expression
tCIDLib::TVoid
TCQSLLDElapsedTm::SetNegatedAt( const   tCIDLib::TCard4     c4At
                                , const tCIDLib::TBoolean   bToSet)
{
    m_colExprList[c4At].bNegated(bToSet);
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Hidden Construtors
// ---------------------------------------------------------------------------

//
//  This is supported only for polymorphic streaming purposes. When we call
//  the parent, indicate we are a normal field and want to be called to
//  evaluate our value regularly, not just when our source fields change.
//
TCQSLLDElapsedTm::TCQSLLDElapsedTm() :

    TCQLSrvFldType(kCIDLib::True, kCIDLib::True)
    , m_bAutoReset(kCIDLib::True)
    , m_bPrevState(kCIDLib::False)
    , m_eLogOp(tCQCKit::ELogOps::AND)
    , m_enctElapsed(0)
    , m_enctLast(0)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQSLLDElapsedTm::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_ElapsedTm::c1FmtVersion))
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
    strmToReadFrom  >> m_bAutoReset
                    >> m_colExprList
                    >> m_eLogOp;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset runtime stuff
    m_bPrevState = kCIDLib::False;
    m_enctLast = TTime::enctNow();
    m_enctElapsed = 0;
}


tCIDLib::TVoid TCQSLLDElapsedTm::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_ElapsedTm::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_bAutoReset
                    << m_colExprList
                    << m_eLogOp
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQSLLDElapsedTm: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just run through each of our source fields and evaluate each one and
//  build up the resulting status. Based on that we update the elapsed time
//  and that's what we give back.
//
tCQLogicSh::EEvalRes
TCQSLLDElapsedTm::eBuildValue(  const   tCQLogicSh::TInfoList&  colVals
                                ,       TCQCFldValue&           fldvToFill
                                , const tCIDLib::TCard4
                                , const tCIDLib::TCard4         )
{
    //
    //  We need to start with the correct initial value for the logical
    //  operator. For AND, we start with true and any false result will
    //  fail it, else we end up with the true. For the others we start with
    //  false. For OR any true result makes it go true, else we end up with
    //  false.
    //
    //  For XOR we use it to remember if we've seen a previous success. If
    //  none of them comes out true, then we end up with the desired false.
    //  When we see a true we set it to true. If we see another true, we
    //  set it back to false and break out. Otherwise, we'll end up at the
    //  with the true result because only one was true.
    //
    tCIDLib::TBoolean bRes;
    if (m_eLogOp == tCQCKit::ELogOps::AND)
        bRes = kCIDLib::True;
    else
        bRes = kCIDLib::False;

    const tCIDLib::TCard4 c4Count = m_colExprList.c4ElemCount();
    CIDAssert
    (
        c4Count == colVals.c4ElemCount()
        , L"Expr and src fld count are inconsistent"
    );

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Evaluate the expr for this source field, passing it the field value
        TCQCExpression& exprCur = m_colExprList[c4Index];
        if (exprCur.bEvaluate(colVals[c4Index].fvCurrent(), 0))
        {
            if (m_eLogOp == tCQCKit::ELogOps::XOR)
            {
                // If we had any previous true, then we can take false now
                if (bRes)
                {
                    bRes = kCIDLib::False;
                    break;
                }

                // Remember we had a previous success
                bRes = kCIDLib::True;
            }
             else if (m_eLogOp == tCQCKit::ELogOps::OR)
            {
                // If anything is true, then an OR will be true
                bRes = kCIDLib::True;
                break;
            }
        }
         else
        {
            if (m_eLogOp == tCQCKit::ELogOps::AND)
            {
                // Any failure means an AND fails
                bRes = kCIDLib::False;
                break;
            }
        }
    }

    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if (bRes)
    {
        if (m_bPrevState)
        {
            //
            //  We were already in true state, so just accumulate the
            //  time since last time.
            //
            m_enctElapsed += enctNow - m_enctLast;
        }
         else
        {
            //
            //  We are transitioning. If auto-reset, then reset it. Else,
            //  we just will pick up with the value we already have.
            //
            if (m_bAutoReset)
                m_enctElapsed = 0;
        }
    }
     else
    {
        //
        //  We've gone false, so reset if auto-reset. Else, we leave the
        //  current value there.
        //
        if (m_bAutoReset)
            m_enctElapsed = 0;
    }

    // Reset the last time and store the new last state
    m_bPrevState = bRes;
    m_enctLast = enctNow;

    // Store the result in the provided value
    if (static_cast<TCQCTimeFldValue&>(fldvToFill).bSetValue(m_enctElapsed))
        return tCQLogicSh::EEvalRes::NewValue;

    return tCQLogicSh::EEvalRes::NoChange;
}


