//
// FILE NAME: CQLogicSh_RunAvgFld.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/27/2013
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
//  This is the header for the CQLogicSh_RunAvgFld.cpp file, which implements
//  one of our virtual field types. This one takes a single source field, which
//  it samples and maintains a running average over time. The new field type
//  will be the same as the source field.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDRunAvg
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDRunAvg : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDRunAvg
        (
            const   TString&                strName
        );

        TCQSLLDRunAvg
        (
            const   TCQSLLDRunAvg&          cllftSrc
        );

        TCQSLLDRunAvg(TCQSLLDRunAvg&&) = delete;

        ~TCQSLLDRunAvg();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDRunAvg& operator=
        (
            const   TCQSLLDRunAvg&          clsftSrc
        );

        TCQSLLDRunAvg& operator=(TCQSLLDRunAvg&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsEqual
        (
            const   TCQLSrvFldType&         clsftComp
        )   const final;

        tCIDLib::TBoolean bIsValidSrcFld
        (
            const   TCQCFldDef&             flddToCheck
        )   const final;

        tCIDLib::TBoolean bValidate
        (
                    TString&                strErr
            ,       tCIDLib::TCard4&        c4SrcFldInd
            , const TCQCFldCache&           cfcData
        )   final;

        tCIDLib::TVoid Initialize
        (
                    TCQCFldCache&           cfcInit
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4AvgHours() const;

        tCIDLib::TCard4 c4AvgHours
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQSLLDRunAvg();


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCQLogicSh::EEvalRes eBuildValue
        (
            const   tCQLogicSh::TInfoList&  colVals
            ,       TCQCFldValue&           fldvToFill
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        )   final;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


    private :
        // -------------------------------------------------------------------
        //  Private class constants
        // -------------------------------------------------------------------
        static constexpr tCIDLib::TCard4 c4AvgMaxCount = 24;
        static constexpr tCIDLib::TCard4 c4MinMaxCount = 60;

        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_af8AvgSamples
        //      The sample array, which we allocate to the apporpriate size
        //      based on the hour count. This is runtime only of course.
        //      There are m_c4AvgHours entries in it.
        //
        //      To make it simpler we use floats for the samples and just
        //      convert back to the actual field type at the end.
        //
        //      It can be up to 24 hours in an average, so we just allocate
        //      this guy to 24 and that's enough for worst case.
        //
        //  m_af8MinSamples
        //      This is used to collect an hour's worth of samplesm, one per
        //      minute. One the hour changes, we average that and add it as
        //      one of the hour averages in m_pf8AvgSamples. This guy is 60
        //      entries since we store one per minute.
        //
        //  m_c4AvgHours
        //      The number of hours over which we do the average. This is
        //      persisted. The max is 24.
        //
        //  m_c4AvgSampleCnt
        //      The number of samples we've put into the overall sample list
        //      so far. It could take up to a day to fill it, since it's
        //      one per hour up to 24 hours.
        //
        //  m_c4CurMinute
        //      The last minute we stored a sample for. When bBuildValue is
        //      called, if the minute has changed, we store another one in
        //      the current current samples
        //
        //  m_c4MinSampleCnt
        //      We may miss samples, particularly initially when we come up
        //      in the middle of an hour. So we don't assume we get 60 samples
        //      per hour. This is the actual number of samples we've put into
        //      the current samples list.
        //
        //  m_f8CurVal
        //      We don't want to calculate this every time bBuildValue() is
        //      called since it requires averaging all the values. So we just
        //      update this when a new sample is added to the overall samples
        //      list. We average what we have and store the value here as our
        //      value for the next hour.
        // -------------------------------------------------------------------
        tCIDLib::TFloat8    m_af8AvgSamples[c4AvgMaxCount];
        tCIDLib::TFloat8    m_af8MinSamples[c4MinMaxCount];
        tCIDLib::TCard4     m_c4AvgHours;
        tCIDLib::TCard4     m_c4AvgSampleCnt;
        tCIDLib::TCard4     m_c4CurMinute;
        tCIDLib::TCard4     m_c4MinSampleCnt;
        tCIDLib::TFloat8    m_f8CurVal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDRunAvg,TCQLSrvFldType)
        DefPolyDup(TCQSLLDRunAvg)
        BefriendFactory(TCQSLLDRunAvg)
};

#pragma CIDLIB_POPPACK



