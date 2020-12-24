//
// FILE NAME: CQLogicSh_GraphFld.hpp
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
//  This is the header for the CQLogicSh_GraphFld.cpp file, which implements
//  one of our virtual field types. This one allows a single int, card, or
//  float field (which has a numeric range limit of some reasonable range),
//  to be graphed by collecting values and storing them in a circular buffer.
//
//  These are not updated often, one minute at the fastest, and are treated
//  separately from the other fields because they aren't exposed to the
//  associated logic server driver. These types of fields are accessed at
//  runtime by clients going directly to the logic server, since they have
//  special requirements because of the multi-valued nature of these types
//  of fields. The facility class provides a nice helper (and cache) to make
//  this easier.
//
//  Note that though we only actually store a sample every X minutes we have
//  to actually access the field in the polling engine more often than that
//  or it might get dropped due to inactivity. And we want to provide a
//  bit of an average of the value across the sample period. So every ten
//  seconds we'll grab the value and store it in a list of sub-samples, to
//  be actually averaged and stored every X minutes. So we need two time
//  stamps for this.
//
//  If the field is not available for the whole X minutes, we'll store a not
//  available value. Otherwise, we'll average what we have. If the field isn't
//  availble we just don't store a pre-sample, so if at the end we have no
//  sub-samples, we know that we need to store a bad value sample. Otherwise,
//  we can just sample as many as we have.
//
// CAVEATS/GOTCHAS:
//
//  1)  Note that we tell the logic server to call our build value method
//      even if nothing has changed or if the field is in error. This is because
//      we have to store a new sample even if nothing has changed. This also
//      means that we have to be careful to check our source field to make
//      sure it's valid before we try to access it, since in our case, unlike
//      most of the other logic server field types, we can get called when
//      the value hasn't been set yet or is in error.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDGraph
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDGraph : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDGraph
        (
            const   TString&                strName
        );

        TCQSLLDGraph
        (
            const   TCQSLLDGraph&           cllftSrc
        );

        TCQSLLDGraph(TCQSLLDGraph&&) = delete;

        ~TCQSLLDGraph();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLLDGraph& operator=
        (
            const   TCQSLLDGraph&           clsftSrc
        );

        TCQSLLDGraph& operator=(TCQSLLDGraph&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAddField
        (
            const   TString&                strToAdd
        )   final;

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

        [[nodiscard]] TCQCFldFilter* pffiltToUse() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQuerySamples
        (
                    tCIDLib::TCard4&        c4SerialNum
            ,       TFundVector<tCIDLib::TFloat4>& fcolToFill
            ,       tCIDLib::TCard4&        c4NewSamples
        )   const;

        tCIDLib::TCard4 c4Minutes() const;

        tCIDLib::TCard4 c4Minutes
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQSLLDGraph();


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
        //  Private data members
        //
        //  m_c4Minutes
        //      The number of minutes between samples. This is a user configured
        //      values and persisted.
        //
        //  m_enctNextSample
        //  m_enctNextSub
        //      These are used to do our sampling and sub-sampling at given
        //      intervals. See the class notes above for details. Runtime only.
        //
        //  m_enctSamplePeriod
        //  m_enctSubPeriod
        //      We pre-calculate the sample period and sub-sample period.
        //
        //  m_fcolSubSamples
        //      A list used to accumulate sub-samples that we will average and
        //      store as the actual sample. We set the size on this one to the
        //      max minutes between samples times 60 divided by the seconds
        //      between sub-samples, plus a few for slop.
        //
        //  m_grdatSamples
        //      This is a runtime value only, to store the actual sample data.
        //      If the source field is in error, we store a special value
        //      which clients will treat as invalid date in any graph display.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4Minutes;
        tCIDLib::TEncodedTime   m_enctNextSample;
        tCIDLib::TEncodedTime   m_enctNextSub;
        tCIDLib::TEncodedTime   m_enctSamplePeriod;
        tCIDLib::TEncodedTime   m_enctSubPeriod;
        tCQLogicSh::TSampleList m_fcolSubSamples;
        TGraphData              m_grdatSamples;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDGraph,TCQLSrvFldType)
        DefPolyDup(TCQSLLDGraph)
        BefriendFactory(TCQSLLDGraph)
};

#pragma CIDLIB_POPPACK


