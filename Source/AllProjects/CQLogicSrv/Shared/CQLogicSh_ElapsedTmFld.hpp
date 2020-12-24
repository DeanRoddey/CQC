//
// FILE NAME: CQLogicSh_ElapsedTmFld.hpp
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
//  This is the header for the CQLogicSh_ElapsedTmFld.cpp file, which
//  implements one of our virtual field types. This one allows, for each
//  associated source field a standard CQC expression is used to evaluate
//  the field value for a true/false result. The various results are
//  combined using a selected boolean operation.
//
//  When the combined result is true, we start counting time, tracking how
//  long that state has been true. We can either be an auto-reset field or
//  manual reset. Auto-resets reset the time accumulator when we transition
//  from false to true state. So it measures how long the state has been true
//  since it went true. Manual reset ones just start counting again, so those
//  measure overall time that the state has been true. It continues to count
//  elapsed time until reset by the user.
//
//  We override the source field management methods so that we can keep our
//  list of expressions in sync with the source field list (which is
//  maintained by the base class.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDElapsedTm
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDElapsedTm : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDElapsedTm
        (
            const   TString&                strName
        );

        TCQSLLDElapsedTm
        (
            const   TCQSLLDElapsedTm&       cllftSrc
        );

        TCQSLLDElapsedTm(TCQSLLDElapsedTm&&) = delete;

        ~TCQSLLDElapsedTm();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDElapsedTm& operator=
        (
            const   TCQSLLDElapsedTm&       clsftSrc
        );

        TCQSLLDElapsedTm& operator=(TCQSLLDElapsedTm&&) = delete;


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

        tCIDLib::TBoolean bMoveField
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bUp
        )   final;

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

        tCIDLib::TVoid RemoveField
        (
            const   tCIDLib::TCard4         c4At
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAutoReset() const;

        tCIDLib::TBoolean bAutoReset
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNegatedAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        const TCQCExpression& exprAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCExpression& exprAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCQCKit::ELogOps eLogOp() const;

        tCQCKit::ELogOps eLogOp
        (
            const   tCQCKit::ELogOps        eToSet
        );

        tCIDLib::TVoid ResetTime();

        tCIDLib::TVoid SetExpressionAt
        (
            const   tCIDLib::TCard4         c4At
            , const TCQCExpression&         exprToSet
        );

        tCIDLib::TVoid SetNegatedAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQSLLDElapsedTm();


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
        //  m_bAutoReset
        //      Indicates if we are auto or manual-reset.
        //
        //  m_bPrevState
        //      The last state we saw, so that we know when we transition
        //      between true and false.
        //
        //  m_eLogOp
        //      The logical operation we use to combine the source field
        //      expression results.
        //
        //  m_colExprList
        //      The list of expressions, one per source field. We override
        //      the source field list manipulation methods so that we can
        //      keep our list in sync with the source field list.
        //
        //  m_enctElapsed
        //      This is our field value where we accumulate our elapsed
        //      time.
        //
        //  m_enctLast
        //      The last time we saw. We need to remember this, so we can
        //      figure out how much time add to the elapsed time each time we
        //      come back in and evaluate the expressions and find it's true.
        //      Each time it's reset to now, so that next time we can figure
        //      out how long we've been away.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAutoReset;
        tCIDLib::TBoolean       m_bPrevState;
        tCQCKit::ELogOps        m_eLogOp;
        TVector<TCQCExpression> m_colExprList;
        tCIDLib::TEncodedTime   m_enctElapsed;
        tCIDLib::TEncodedTime   m_enctLast;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDElapsedTm,TCQLSrvFldType)
        DefPolyDup(TCQSLLDElapsedTm)
        BefriendFactory(TCQSLLDElapsedTm)
};

#pragma CIDLIB_POPPACK



