//
// FILE NAME: CQLogicSh_BoolFld.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/18/2009
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
//  result from one or more source fields, by evaluting their values.
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
AdvRTTIDecls(TCQSLLDBool,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQLogicSh_BoolFld
    {
        // -----------------------------------------------------------------------
        //  Our persistent format
        //
        //  Version 2 -
        //      We failed to stream the logical operator in the original version,
        //      so we need to deal with that.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard1   c1FmtVersion = 2;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDBool
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDBool: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLLDBool::TCQSLLDBool(const TString& strName) :

    TCQLSrvFldType(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read)
    , m_eLogOp(tCQCKit::ELogOps::AND)
{
}

TCQSLLDBool::TCQSLLDBool(const TCQSLLDBool& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_eLogOp(clsftSrc.m_eLogOp)
    , m_colExprList(clsftSrc.m_colExprList)
{
}

TCQSLLDBool::~TCQSLLDBool()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Public oeprators
// ---------------------------------------------------------------------------
TCQSLLDBool& TCQSLLDBool::operator=(const TCQSLLDBool& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);
        m_eLogOp = clsftSrc.m_eLogOp;
        m_colExprList = clsftSrc.m_colExprList;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Public, inherited methods
// ---------------------------------------------------------------------------

// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean TCQSLLDBool::bAddField(const TString& strToAdd)
{
    //
    //  Call our parent first. If he adds it, then we add an expression. We default it
    //  to a statement type.
    //
    if (TParent::bAddField(strToAdd))
    {
        m_colExprList.objPlace
        (
            L"Boolean Test"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsTrue
            , TString::strEmpty()
            , kCIDLib::False
        );
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDBool::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );

    const TCQSLLDBool& clsftAct = static_cast<const TCQSLLDBool&>(clsftRHS);

    // Compare the logical op
    if (m_eLogOp != clsftAct.m_eLogOp)
        return kCIDLib::False;

    // That's ok, so compare the expressions. If not the same count, then not
    const tCIDLib::TCard4 c4Count = m_colExprList.c4ElemCount();
    if (c4Count != clsftAct.m_colExprList.c4ElemCount())
        return kCIDLib::False;

    // They are both the same size lists, so compare them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colExprList[c4Index] != clsftAct.m_colExprList[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQSLLDBool::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // We can do any type
    return kCIDLib::True;
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean
TCQSLLDBool::bMoveField(const   tCIDLib::TCard4     c4At
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
TCQSLLDBool::bValidate(         TString&            strErr
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


tCIDLib::TVoid TCQSLLDBool::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    // A no-op for us
}


// We override in order to keep our expression list in sync with source fields
tCIDLib::TVoid TCQSLLDBool::RemoveField(const tCIDLib::TCard4 c4At)
{
    // Call our parent first. He'll validate the index
    TParent::RemoveField(c4At);
    m_colExprList.RemoveAt(c4At);
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Set the negated flag on the indicated expression
tCIDLib::TBoolean TCQSLLDBool::bNegatedAt(const tCIDLib::TCard4 c4At) const
{
    return m_colExprList[c4At].bNegated();
}


// Get/set our logical operator
tCQCKit::ELogOps TCQSLLDBool::eLogOp() const
{
    return m_eLogOp;
}

tCQCKit::ELogOps TCQSLLDBool::eLogOp(const tCQCKit::ELogOps eToSet)
{
    m_eLogOp = eToSet;
    return m_eLogOp;
}


// Provide access to our expressions for the client side driver
const TCQCExpression& TCQSLLDBool::exprAt(const tCIDLib::TCard4 c4At) const
{
    return m_colExprList[c4At];
}

TCQCExpression& TCQSLLDBool::exprAt(const tCIDLib::TCard4 c4At)
{
    return m_colExprList[c4At];
}

tCIDLib::TVoid
TCQSLLDBool::SetExpressionAt(const  tCIDLib::TCard4 c4At
                            , const TCQCExpression& exprToSet)
{
    m_colExprList[c4At] = exprToSet;
}


// Set the negated flag on a given expression
tCIDLib::TVoid
TCQSLLDBool::SetNegatedAt(const tCIDLib::TCard4 c4At, const tCIDLib::TBoolean bToSet)
{
    m_colExprList[c4At].bNegated(bToSet);
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Hidden Construtors
// ---------------------------------------------------------------------------

// This is supported only for polymorphic streaming and creation purposes.
TCQSLLDBool::TCQSLLDBool() :

    TCQLSrvFldType()
    , m_eLogOp(tCQCKit::ELogOps::AND)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQSLLDBool::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_BoolFld::c1FmtVersion))
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
    strmToReadFrom >> m_colExprList;

    // If V1, fault in the logical opeator, else stream it in
    if (c1FmtVersion == 1)
        m_eLogOp = tCQCKit::ELogOps::AND;
    else
        strmToReadFrom >> m_eLogOp;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDBool::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_BoolFld::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_colExprList
                    << m_eLogOp
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQSLLDBool: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just run through each of our source fields and evaluate each one and
//  build up the resulting value.
//
tCQLogicSh::EEvalRes
TCQSLLDBool::eBuildValue(const  tCQLogicSh::TInfoList&  colVals
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
    CIDAssert(c4Count == colVals.c4ElemCount(), L"Expr and src fld count are inconsistent");

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Evaluate the expression for this source field, passing it the field value
        TCQCExpression& exprCur = m_colExprList[c4Index];
        if (exprCur.bEvaluate(colVals[c4Index].fvCurrent(), 0))
        {
            if (m_eLogOp == tCQCKit::ELogOps::XOR)
            {
                // If we had any previous true, then we can return false now
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

    //
    //  Store the result in the provided value, which should be our own
    //  value object (stored in the parent class.)
    //
    if (static_cast<TCQCBoolFldValue&>(fldvToFill).bSetValue(bRes))
        return tCQLogicSh::EEvalRes::NewValue;

    return tCQLogicSh::EEvalRes::NoChange;
}

