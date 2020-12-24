//
// FILE NAME: CQLogicSh_GraphFld.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2011
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
//  This file implements a virtual field derivative that supports graphing
//  of numeric fields.
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
AdvRTTIDecls(TCQSLLDGraph,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQLogicSh_GraphFld
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format
        // -----------------------------------------------------------------------
        const tCIDLib::TCard1   c1FmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our min/max numeric field limit ranges we'll accept
        // -----------------------------------------------------------------------
        const tCIDLib::TFloat4  f4MinRange = 4.0;
        const tCIDLib::TFloat4  f4MaxRange = 65535.0;


        // -----------------------------------------------------------------------
        //  The max number of sub-samples we'll need to accumulate
        // -----------------------------------------------------------------------
        const tCIDLib::TCard4   c4MaxSubSamples
        (
            ((kCQLogicSh::c4MaxGraphCol * 60) / kCQLogicSh::c4SubSamplSecs) + 4
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDGraph
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We tell the parent class that we are not a normal field to be exposed via
//  the logic server driver. And we tell him to call us even if there's no
//  change, since we need to sample the values and put out a new sample
//  even if nothing has changed.
//
TCQSLLDGraph::TCQSLLDGraph(const TString& strName) :

    TCQLSrvFldType
    (
        strName
        , tCQCKit::EFldTypes::Float
        , tCQCKit::EFldAccess::Read
        , kCIDLib::False
        , kCIDLib::True
        , 1
    )
    , m_c4Minutes(1)
    , m_enctNextSub(0)
    , m_enctNextSample(0)
    , m_fcolSubSamples(CQLogicSh_GraphFld::c4MaxSubSamples)
    , m_grdatSamples(kCQLogicSh::c4GraphSampleCnt)
{
}

TCQSLLDGraph::TCQSLLDGraph(const TCQSLLDGraph& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_c4Minutes(clsftSrc.m_c4Minutes)
    , m_enctNextSub(0)
    , m_enctNextSample(0)
    , m_fcolSubSamples(CQLogicSh_GraphFld::c4MaxSubSamples)
    , m_grdatSamples(kCQLogicSh::c4GraphSampleCnt)
{
    // The samples and times are runtime only and don't need to be copied
}

TCQSLLDGraph::~TCQSLLDGraph()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Public operators
// ---------------------------------------------------------------------------
TCQSLLDGraph& TCQSLLDGraph::operator=(const TCQSLLDGraph& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);
        m_c4Minutes   = clsftSrc.m_c4Minutes;

        // The samples and times are runtime only and don't need to be copied
        m_enctNextSub = 0;
        m_enctNextSample = 0;
        m_fcolSubSamples.RemoveAll();
        m_grdatSamples.Reset(1UL);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Public, inherited methods
// ---------------------------------------------------------------------------

// We override in order to keep our expression list in sync with source fields
tCIDLib::TBoolean TCQSLLDGraph::bAddField(const TString& strToAdd)
{
    // If we already have one field we shouldn't be getting another
    if (c4SrcFldCount())
        return kCIDLib::False;

    return TParent::bAddField(strToAdd);
}


// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDGraph::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );
    const TCQSLLDGraph& clsftGraph = static_cast<const TCQSLLDGraph&>(clsftRHS);
    return (m_c4Minutes == clsftGraph.m_c4Minutes);
}


// Indicate whether the passed field is legal for our purposes
tCIDLib::TBoolean TCQSLLDGraph::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // Has to be numeric
    return flddToCheck.bIsNumericType();
}


tCIDLib::TBoolean
TCQSLLDGraph::bValidate(        TString&            strErr
                        ,       tCIDLib::TCard4&    c4SrcFldInd
                        , const TCQCFldCache&       cfcData)
{
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    // No source field error so set it to max card
    c4SrcFldInd = kCIDLib::c4MaxCard;

    return kCIDLib::True;
}


tCIDLib::TVoid TCQSLLDGraph::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    // Calc our two sample periods
    m_enctSubPeriod = kCQLogicSh::c4SubSamplSecs * kCIDLib::enctOneSecond;
    m_enctSamplePeriod = m_c4Minutes * kCIDLib::enctOneMinute;

    //
    //  Set the next full sample time to now plus the sample time. Zero the
    //  sub-sample next time so we start sampling immediately.
    //
    m_enctNextSample = TTime::enctNow() + m_enctSamplePeriod;
    m_enctNextSub = 0;

    // Reset the sample index and clear our lists
    m_fcolSubSamples.RemoveAll();
    m_grdatSamples.Reset(1UL);
}


TCQCFldFilter* TCQSLLDGraph::pffiltToUse()
{
    // We only can accept numeric fields of a certain range
    return new TCQCFldFiltNumeric(tCQCKit::EReqAccess::MReadCWrite, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When a client asks for the most recent data for a graph field, the server
//  calls us here. We just pass this on to the graph object that we use to
//  hold the sample data.
//
tCIDLib::TBoolean
TCQSLLDGraph::bQuerySamples(tCIDLib::TCard4&                    c4SerialNum
                            , TFundVector<tCIDLib::TFloat4>&    fcolToFill
                            , tCIDLib::TCard4&                  c4NewSamples) const
{
    return m_grdatSamples.bQuerySamples(c4SerialNum, fcolToFill, c4NewSamples);
}


// Get/set the minutes value
tCIDLib::TCard4 TCQSLLDGraph::c4Minutes() const
{
    return m_c4Minutes;
}

tCIDLib::TCard4 TCQSLLDGraph::c4Minutes(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Minutes = c4ToSet;
    return m_c4Minutes;
}


// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Hidden Constructors
// ---------------------------------------------------------------------------

// Needed for polymorphic streaming
TCQSLLDGraph::TCQSLLDGraph() :

    TCQLSrvFldType(kCIDLib::False, kCIDLib::True, 1)
    , m_c4Minutes(1)
    , m_enctNextSub(0)
    , m_enctNextSample(0)
    , m_fcolSubSamples(CQLogicSh_GraphFld::c4MaxSubSamples)
    , m_grdatSamples(kCQLogicSh::c4GraphSampleCnt)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDGraph: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  In our case, we aren't really building a value to give back since this
//  is a graph type field. We are just using this opporunity to sample the
//  value.
//
//  Note that we tell the server to call us even if the field value hasn't
//  changed or is in error, since we have to put out new samples even if
//  nothing changes. So we have to check the error state of our source
//  field ourself in our case.
//
tCQLogicSh::EEvalRes
TCQSLLDGraph::eBuildValue(  const   tCQLogicSh::TInfoList& colFlds
                            ,       TCQCFldValue&
                            , const tCIDLib::TCard4
                            , const tCIDLib::TCard4)
{
    // See if it's time to store a full sub or full sample yet
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();

    if (enctCur >= m_enctNextSample)
    {
        const tCIDLib::TCard4 c4SubCnt = m_fcolSubSamples.c4ElemCount();
        tCIDLib::TFloat4 f4NewVal = 0;
        if (c4SubCnt)
        {
            // Let's average the values
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SubCnt; c4Index++)
                f4NewVal += m_fcolSubSamples[c4Index];
            f4NewVal /= tCIDLib::TFloat4(c4SubCnt);
        }
         else
        {
            // The field was never available, so store an error value
            f4NewVal = kCQLogicSh::f4SampleErr;
        }

        m_grdatSamples.PushNewSample(f4NewVal);

        // Clear the sub-sample list and update the next sample stamp
        m_fcolSubSamples.RemoveAll();
        m_enctNextSample = enctCur + m_enctSamplePeriod;
    }
     else if (enctCur >= m_enctNextSub)
    {
        //
        //  We need to store a sub-sample. Make sure that our source field is OK
        //  before we store anything, and also check just in case to make sure
        //  we aren't storing too many sub-samples. It shouldn't happen, but just
        //  in case.
        //
        const TCQCFldPollInfo& cfpiSrc = colFlds[0];
        if (cfpiSrc.bHasGoodValue()
        &&  (m_fcolSubSamples.c4ElemCount() < CQLogicSh_GraphFld::c4MaxSubSamples))
        {
            const TCQCFldValue& fvNew = cfpiSrc.fvCurrent();

            tCIDLib::TFloat4 f4NewVal = kCQLogicSh::f4SampleErr;
            if (!fvNew.bInError())
            {
                if (fvNew.eFldType() == tCQCKit::EFldTypes::Card)
                    f4NewVal = tCIDLib::TFloat4(static_cast<const TCQCCardFldValue&>(fvNew).c4Value());
                else if (fvNew.eFldType() == tCQCKit::EFldTypes::Int)
                    f4NewVal = tCIDLib::TFloat4(static_cast<const TCQCIntFldValue&>(fvNew).i4Value());
                else if (fvNew.eFldType() == tCQCKit::EFldTypes::Float)
                    f4NewVal = tCIDLib::TFloat4(static_cast<const TCQCFloatFldValue&>(fvNew).f8Value());
            }

            // If we got a good value, then store it
            if (f4NewVal != kCQLogicSh::f4SampleErr)
                m_fcolSubSamples.c4AddElement(f4NewVal);
        }
        m_enctNextSub = enctCur + m_enctSubPeriod;
    }

    // A bogus return, we aren't really creating a value for output
    return tCQLogicSh::EEvalRes::NoChange;
}


tCIDLib::TVoid TCQSLLDGraph::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_GraphFld::c1FmtVersion))
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
    strmToReadFrom >> m_c4Minutes;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQSLLDGraph::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_GraphFld::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_c4Minutes
                    << tCIDLib::EStreamMarkers::EndObject;
}

