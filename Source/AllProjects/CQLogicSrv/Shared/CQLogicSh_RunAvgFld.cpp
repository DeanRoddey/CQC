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
AdvRTTIDecls(TCQSLLDRunAvg,TCQLSrvFldType);



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQLogicSh_RunAvgFld
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
//   CLASS: TCQSLLDRunAvg
//  PREFIX: clsft
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Indicate we always want to be called for bBuildValue, even if the value
//  hasn't changed, so that we can do our averaging. We indicate we only take
//  one source field.
//
TCQSLLDRunAvg::TCQSLLDRunAvg(const TString& strName) :

    TCQLSrvFldType
    (
        strName
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , kCIDLib::True
        , kCIDLib::True
        , 1
    )
    , m_c4AvgHours(1)
    , m_c4AvgSampleCnt(0)
    , m_c4CurMinute(0)
    , m_c4MinSampleCnt(0)
{
}

TCQSLLDRunAvg::TCQSLLDRunAvg(const TCQSLLDRunAvg& clsftSrc) :

    TCQLSrvFldType(clsftSrc)
    , m_c4AvgHours(clsftSrc.m_c4AvgHours)
    , m_c4AvgSampleCnt(clsftSrc.m_c4AvgSampleCnt)
    , m_c4CurMinute(clsftSrc.m_c4CurMinute)
    , m_c4MinSampleCnt(clsftSrc.m_c4MinSampleCnt)
{
    // No need to copy the buffer data
}

TCQSLLDRunAvg::~TCQSLLDRunAvg()
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Public operators
// ---------------------------------------------------------------------------
TCQSLLDRunAvg& TCQSLLDRunAvg::operator=(const TCQSLLDRunAvg& clsftSrc)
{
    if (this != &clsftSrc)
    {
        TParent::operator=(clsftSrc);

        // No need to copy the buffer data
        m_c4AvgHours      = clsftSrc.m_c4AvgHours;
        m_c4AvgSampleCnt  = clsftSrc.m_c4AvgSampleCnt;
        m_c4CurMinute     = clsftSrc.m_c4CurMinute;
        m_c4MinSampleCnt  = clsftSrc.m_c4MinSampleCnt;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Public, inherited methods
// ---------------------------------------------------------------------------

// See if we are equal to the passed one
tCIDLib::TBoolean
TCQSLLDRunAvg::bIsEqual(const TCQLSrvFldType& clsftRHS) const
{
    if (!TParent::bIsEqual(clsftRHS))
        return kCIDLib::False;

    // Make sure it's of our type or beyond
    CIDAssert
    (
        clsftRHS.bIsDescendantOf(clsThis())
        , L"The RHS logic server field is not the same type as this field"
    );
    const TCQSLLDRunAvg& clsftAvg = static_cast<const TCQSLLDRunAvg&>(clsftRHS);

    // The hours count is the same, then they are equal
    return (m_c4AvgHours == clsftAvg.m_c4AvgHours);
}


tCIDLib::TBoolean TCQSLLDRunAvg::bIsValidSrcFld(const TCQCFldDef& flddToCheck) const
{
    // We only support numerics
    return flddToCheck.bIsNumericType();
}


tCIDLib::TBoolean
TCQSLLDRunAvg::bValidate(       TString&            strErr
                        ,       tCIDLib::TCard4&    c4SrcFldInd
                        , const TCQCFldCache&       cfcData)
{
    if (!TParent::bValidate(strErr, c4SrcFldInd, cfcData))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDLib::TVoid TCQSLLDRunAvg::Initialize(TCQCFldCache& cfcInit)
{
    // Call our parent first
    TParent::Initialize(cfcInit);

    // Initialize our sampling variables
    m_c4AvgSampleCnt  = 0;
    m_c4CurMinute     = 0;
    m_c4MinSampleCnt  = 0;
    m_f8CurVal        = 0;
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the number of hours to average
tCIDLib::TCard4 TCQSLLDRunAvg::c4AvgHours() const
{
    return m_c4AvgHours;
}

tCIDLib::TCard4
TCQSLLDRunAvg::c4AvgHours(const tCIDLib::TCard4 c4ToSet)
{
    m_c4AvgHours = c4ToSet;
    return m_c4AvgHours;
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Hidden Construtors
// ---------------------------------------------------------------------------

// This is supported only for polymorphic streaming purposes
TCQSLLDRunAvg::TCQSLLDRunAvg() :

    TCQLSrvFldType(kCIDLib::True, kCIDLib::True, 1)
    , m_c4AvgHours(1)
    , m_c4AvgSampleCnt(0)
    , m_c4CurMinute(0)
    , m_c4MinSampleCnt(0)
    , m_f8CurVal(0)
{
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQSLLDRunAvg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard1 c1FmtVersion;
    strmToReadFrom  >> c1FmtVersion;
    if (!c1FmtVersion || (c1FmtVersion > CQLogicSh_RunAvgFld::c1FmtVersion))
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
    strmToReadFrom >> m_c4AvgHours;

    // If, byt some chance, it should be more than that max, clip it back
    if (m_c4AvgHours > c4AvgMaxCount)
        m_c4AvgHours = c4AvgMaxCount;

    // It should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Init any runtime stuff
    m_c4AvgSampleCnt  = 0;
    m_c4CurMinute     = 0;
    m_c4MinSampleCnt  = 0;
}


tCIDLib::TVoid TCQSLLDRunAvg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQLogicSh_RunAvgFld::c1FmtVersion;

    // Do our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_c4AvgHours
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQSLLDRunAvg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called periodically. We will take any changes that happen in the
//  most recent minute. We get the hour and minute (zero based) so we can just
//  watch for the minute to change from the last one we stored. Then we take
//  one sample for that minute.
//
tCQLogicSh::EEvalRes
TCQSLLDRunAvg::eBuildValue( const   tCQLogicSh::TInfoList&  colVals
                            ,       TCQCFldValue&           fldvToFill
                            , const tCIDLib::TCard4         c4Hour
                            , const tCIDLib::TCard4         c4Minute)
{
    // Check for some psycho scenarios
    CIDAssert
    (
        (m_c4MinSampleCnt < c4MinMaxCount) && (m_c4AvgSampleCnt <= m_c4AvgHours)
        , L"The minute or average sample count is invalid"
    );

    //
    //  If the minute has changed from what we last saw, then we need to
    //  do something. Else just return no change.
    //
    if (c4Minute == m_c4CurMinute)
        return tCQLogicSh::EEvalRes::NoChange;

    // Store the new minute
    m_c4CurMinute = c4Minute;

    // We only have one value, so get it out now
    const TCQCFldPollInfo& cfpiCur = colVals[0];

    // If we don't have a good value, then we are in error
    if (!cfpiCur.bHasGoodValue())
        return tCQLogicSh::EEvalRes::Error;

    // Looks ok, so get the value out
    const TCQCFldValue& fvCur = cfpiCur.fvCurrent();

    //
    //  Add another sample to the current hour samples list. Make sure we
    //  haven't gone over our limit. Shouldn't happen, but just in case.
    //  Eventually we'll hit 59 minutes and recover.
    //
    tCIDLib::TFloat8 f8New;
    if (m_c4MinSampleCnt < c4MinMaxCount)
    {
        //
        //  Get a value as a float, which may require conversion, depending
        //  on the type of the source field.
        //
        switch(cfpiCur.flddAssoc().eType())
        {
            case tCQCKit::EFldTypes::Card :
                f8New = tCIDLib::TFloat8
                (
                    static_cast<const TCQCCardFldValue&>(fvCur).c4Value()
                );
                break;

            case tCQCKit::EFldTypes::Float :
                f8New = static_cast<const TCQCFloatFldValue&>(fvCur).f8Value();
                break;

            case tCQCKit::EFldTypes::Int :
                f8New = tCIDLib::TFloat8
                (
                    static_cast<const TCQCIntFldValue&>(fvCur).i4Value()
                );
                break;

            default :
                return tCQLogicSh::EEvalRes::Error;
        };

        // Add another sample to the list
        m_af8MinSamples[m_c4MinSampleCnt++] = f8New;
    }

    // Calculate a new average so far for this hour
    f8New = 0;
    if (m_c4MinSampleCnt)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4MinSampleCnt; c4Index++)
            f8New += m_af8MinSamples[c4Index];
        f8New /= m_c4MinSampleCnt;
    }

    //
    //  Calculate a new overall average, not committing the new hour yet
    //  which we'll only do below if we are on the last minute of the hour.
    //  We just add the temp hourly average from above and divide by one more
    //  than the current count.
    //
    m_f8CurVal = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4AvgSampleCnt; c4Index++)
        m_f8CurVal += m_af8AvgSamples[c4Index];
    m_f8CurVal += f8New;
    m_f8CurVal /= m_c4AvgSampleCnt + 1;

    //
    //  If the minute is 59, then we have an hours worth and need to commit
    //  it and reset for the next time.
    //
    if (c4Minute == 59)
    {
        //
        //  If our overall average list isn't full, just add this one at the
        //  end. Else we need to shift it down to make room first.
        //
        if (m_c4AvgSampleCnt == m_c4AvgHours)
        {
            for (tCIDLib::TCard4 c4Index = 1; c4Index < m_c4AvgSampleCnt; c4Index++)
                m_af8AvgSamples[c4Index - 1] = m_af8AvgSamples[c4Index];
            m_c4AvgSampleCnt--;
        }
        m_af8AvgSamples[m_c4AvgSampleCnt++] = f8New;

        //
        //  Reset our current sample count for the next round. NOTE that
        //  we don't reset our current minute. We have to wait until the
        //  minute changes (back to 0) before we want to do anything again.
        //
        m_c4MinSampleCnt  = 0;
    }

    //
    //  Return whatever the last overall average we calculated was. It
    //  may have just changed, or it may be another hour before it does.
    //
    //  In this case we need to switch on our virtual field's type and
    //  get the value into it.
    //
    tCIDLib::TBoolean bChanged = kCIDLib::False;
    try
    {
        switch(flddCfg().eType())
        {
            case tCQCKit::EFldTypes::Card :
                // If it goes negative we can't store it on this field
                if (m_f8CurVal < 0)
                    return tCQLogicSh::EEvalRes::Error;

                bChanged = static_cast<TCQCCardFldValue&>(fldvToFill).bSetValue
                (
                    tCIDLib::TCard4(m_f8CurVal)
                );
                break;

            case tCQCKit::EFldTypes::Float :
                bChanged = static_cast<TCQCFloatFldValue&>(fldvToFill).bSetValue(m_f8CurVal);
                break;

            case tCQCKit::EFldTypes::Int :
                bChanged = static_cast<TCQCIntFldValue&>(fldvToFill).bSetValue
                (
                    tCIDLib::TInt4(m_f8CurVal)
                );
                break;

            default :
                bChanged = kCIDLib::False;
                break;
        };


    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && facCQLogicSh().bLogWarnings())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Put us into error state
        return tCQLogicSh::EEvalRes::Error;
    }

    if (bChanged)
        return tCQLogicSh::EEvalRes::NewValue;

    return tCQLogicSh::EEvalRes::NoChange;
}


