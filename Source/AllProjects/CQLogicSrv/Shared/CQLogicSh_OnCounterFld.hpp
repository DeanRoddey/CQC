//
// FILE NAME: CQLogicSh_OnCounterFld.hpp
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
//  This is the header for the CQLogicSh_OnCounterFld.cpp file, which implements
//  one of our virtual field types. This one has an expression per source field, and
//  it keeps a count of how many have evaluated to true. Or, it can be flipped to count
//  how many have evaluated to false.
//
//  We only need to be called when one of our fields change, at which time we just
//  update our value by evaluating all our source fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDOnCounter
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDOnCounter : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDOnCounter
        (
            const   TString&                strName
        );

        TCQSLLDOnCounter
        (
            const   TCQSLLDOnCounter&       cllftSrc
        );

        TCQSLLDOnCounter(TCQSLLDOnCounter&&) = delete;

        ~TCQSLLDOnCounter();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDOnCounter& operator=
        (
            const   TCQSLLDOnCounter&       clsftSrc
        );

        TCQSLLDOnCounter& operator=(TCQSLLDOnCounter&&) = delete;


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

        tCIDLib::TBoolean bMoveField
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bUp
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
        //  Public, non-virtaul methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInverse() const;

        tCIDLib::TBoolean bInverse
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TCQCExpression& exprAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        TCQCExpression& exprAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid SetExpressionAt
        (
            const   tCIDLib::TCard4         c4At
            , const TCQCExpression&         exprToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQSLLDOnCounter();


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
        //  m_bInverse
        //      If this is set, we store the number of failing fields, where normally
        //      we'd store the number of passing fields.
        //
        //  m_colExprList
        //      A list of expressions, one per source field.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bInverse;
        TVector<TCQCExpression> m_colExprList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDOnCounter, TCQLSrvFldType)
        DefPolyDup(TCQSLLDOnCounter)
        BefriendFactory(TCQSLLDOnCounter)
};

#pragma CIDLIB_POPPACK



