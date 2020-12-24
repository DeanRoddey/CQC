//
// FILE NAME: CQLogicSh_BoolFld.hpp
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
//  This is the header for the CQLogicSh_BoolFld.cpp file, which implements
//  one of our virtual field types. This one allows, for each associated
//  source field a standard CQC expression is used to evaluate the field
//  value for a true/false result. The various results are combined using a
//  selected boolean operation.
//
//  We final the source field management methods so that we can keep our
//  list of expressions in sync with the source field list (which is maintained
//  by the base class.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDBool
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDBool : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDBool
        (
            const   TString&                strName
        );

        TCQSLLDBool
        (
            const   TCQSLLDBool&            cllftSrc
        );

        TCQSLLDBool(TCQSLLDBool&&) = delete;

        ~TCQSLLDBool();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDBool& operator=
        (
            const   TCQSLLDBool&            clsftSrc
        );

        TCQSLLDBool& operator=(TCQSLLDBool&&) = delete;


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
        tCIDLib::TBoolean bNegatedAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;

        tCQCKit::ELogOps eLogOp() const;

        tCQCKit::ELogOps eLogOp
        (
            const   tCQCKit::ELogOps        eToSet
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

        tCIDLib::TVoid SetNegatedAt
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQSLLDBool();


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
        //  m_eLogOp
        //      The logical operation we use to combine the source field
        //      expression results.
        //
        //  m_colExprList
        //      The list of expressions, one per source field. We final
        //      the source field list manipulation methods so that we can
        //      keep our list in sync with the source field list.
        // -------------------------------------------------------------------
        tCQCKit::ELogOps        m_eLogOp;
        TVector<TCQCExpression> m_colExprList;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDBool,TCQLSrvFldType)
        DefPolyDup(TCQSLLDBool)
        BefriendFactory(TCQSLLDBool)
};

#pragma CIDLIB_POPPACK


