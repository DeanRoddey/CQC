//
// FILE NAME: CQLogicSh_BoolFld.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2016
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
//  This file implements a virtual field derivative that has a cardinal value,
//  representing how many of the source fields are true/non-zero (or vice versa
//  if negated.)
//
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
AdvRTTIDecls(TCQSLLDOnCounter, TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQLogicSh_OnCounterFld
{
    namespace
    {
        // -------------------------------------------------------------------
        //  Our persistent format
        //
        // -------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDOnCounter
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Indicate we always want to be called for bBuildValue, even if the value
//  hasn't changed, so that we can do our averaging. We indicate we only take
//  one source field.
//
TCQSLLDOnCounter::TCQSLLDOnCounter(const TString& strName) :

    TCQLSrvFldType
    (
        strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read
    )
    , m_bInverse(kCIDLib::False)
{
}

TCQSLLDOnCounter::TCQSLLDOnCounter(const TCQSLLDOnCounter& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_bInverse(clsftSrc.m_bInverse)
    , m_colExprList(clsftSrc.m_colExprList)
{
}

TCQSLLDOnCounter::~TCQSLLDOnCounter()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Public operators
// ---------------------------------------------------------------------------
TCQSLLDOnCounter& TCQSLLDOnCounter::operator=(const TCQSLLDOnCounter& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);
        m_bInverse  = clsftSrc.m_bInverse;
        m_colExprList = clsftSrc.m_colExprList;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Public, inherited methods
// ---------------------------------------------------------------------------

// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean TCQSLLDOnCounter::bAddField(const TString& strToAdd)
{
    //
    //  Call our parent first. If he adds it, then we add an expression. Set it initially
    //  to statement type.
    //
    if (TParent::bAddField(strToAdd))
    {
        m_colExprList.objPlace
        (
            L"Counter Test"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsGThan
            , L"0"
            , kCIDLib::False
        );
        return kCIDLib::True;
    }
    return kCIDLib::False;
}



// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDOnCounter::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );
    const TCQSLLDOnCounter& clsftOC = static_cast<const TCQSLLDOnCounter&>(clsftRHS);

    // Compare our members, starting with the simple ones
    if (m_bInverse != clsftOC.m_bInverse)
        return kCIDLib::False;

    // That's ok, so compare the expressions. If not the same count, then not
    const tCIDLib::TCard4 c4Count = m_colExprList.c4ElemCount();
    if (c4Count != clsftOC.m_colExprList.c4ElemCount())
        return kCIDLib::False;

    // They are both the same size lists, so compare them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colExprList[c4Index] != clsftOC.m_colExprList[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQSLLDOnCounter::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // We are expression based so we can basically handle any type
    return kCIDLib::True;
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean
TCQSLLDOnCounter::bMoveField(const  tCIDLib::TCard4     c4At
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
TCQSLLDOnCounter::bValidate(        TString&            strErr
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


tCIDLib::TVoid TCQSLLDOnCounter::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TVoid TCQSLLDOnCounter::RemoveField(const tCIDLib::TCard4 c4At)
{
    // Call our parent first. He'll validate the index
    TParent::RemoveField(c4At);
    m_colExprList.RemoveAt(c4At);
}



// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the inversion flag
tCIDLib::TBoolean TCQSLLDOnCounter::bInverse() const
{
    return m_bInverse;
}

tCIDLib::TBoolean TCQSLLDOnCounter::bInverse(const tCIDLib::TBoolean bToSet)
{
    m_bInverse = bToSet;
    return m_bInverse;
}


// Provide access to our expressions for the client side driver
const TCQCExpression& TCQSLLDOnCounter::exprAt(const tCIDLib::TCard4 c4At) const
{
    return m_colExprList[c4At];
}

TCQCExpression& TCQSLLDOnCounter::exprAt(const tCIDLib::TCard4 c4At)
{
    return m_colExprList[c4At];
}

tCIDLib::TVoid
TCQSLLDOnCounter::SetExpressionAt(  const   tCIDLib::TCard4 c4At
                                    , const TCQCExpression& exprToSet)
{
    m_colExprList[c4At] = exprToSet;
}



// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Hidden Construtors
// ---------------------------------------------------------------------------

// This is supported only for polymorphic streaming purposes
TCQSLLDOnCounter::TCQSLLDOnCounter() :

    TCQLSrvFldType()
    , m_bInverse(kCIDLib::False)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQSLLDOnCounter::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_OnCounterFld::c1FmtVersion))
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
    strmToReadFrom  >> m_bInverse
                    >> m_colExprList;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDOnCounter::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_OnCounterFld::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_bInverse
                    << m_colExprList
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQSLLDOnCounter: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called periodically. We will take any changes that happen in the
//  most recent minute. We get the hour and minute (zero based) so we can just
//  watch for the minute to change from the last one we stored. Then we take
//  one sample for that minute.
//
tCQLogicSh::EEvalRes
TCQSLLDOnCounter::eBuildValue(  const   tCQLogicSh::TInfoList&  colVals
                                ,       TCQCFldValue&           fldvToFill
                                , const tCIDLib::TCard4
                                , const tCIDLib::TCard4         )
{
    const tCIDLib::TCard4 c4Count = colVals.c4ElemCount();

    tCIDLib::TCard4 c4OnCount = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Evaluate the expression for this source field, passing it the field value
        TCQCExpression& exprCur = m_colExprList[c4Index];
        if (exprCur.bEvaluate(colVals[c4Index].fvCurrent(), 0))
            c4OnCount++;
    }

    // If we are inverted, flip the count
    if (m_bInverse)
        c4OnCount = c4Count - c4OnCount;

    // Store this value, reporting if it's a change
    TCQCCardFldValue& fvCount = static_cast<TCQCCardFldValue&>(fvCurrent());
    if (fvCount.bSetValue(c4OnCount))
        return tCQLogicSh::EEvalRes::NewValue;

    return tCQLogicSh::EEvalRes::NoChange;
}


