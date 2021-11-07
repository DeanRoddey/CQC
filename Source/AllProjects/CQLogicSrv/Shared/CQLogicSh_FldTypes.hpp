//
// FILE NAME: CQLogicSh_FldTypes.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This is the header for the CQLogicSh_FieldTypes.cpp file, which
//  implements the base class for all of the types of fields that the logic
//  server implements, and some of the more basic classes that derive from
//  it.
//
//  At the base class level we manage a list of source fields from which the
//  derived classes will create their own values for the virtual fields that
//  they represent. We have methods for the client side driver to manage that
//  list, and we have methods that the server side driver can use to keep
//  all the virtual fields updated and such.
//
//  The following simple derivatives are defined here. Others are done
//  in other files since they take more code.
//
//  1. TCQSLLDPatFmt - Let's the set a pattern with replacement tokens, where
//      %(1) is the first field, %(2) is the second, and so on.
//  2. TCQSLLDMinMaxAvg - Accepts numeric fields (of the same type it is set
//      to have) and it's value will be either the lowest, highest, or average
//      of those source fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQLSrvFldType
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQLSrvFldType

    : public TObject, public MStreamable, public MDuplicable
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static tCIDLib::ESortComps eComp
        (
            const   TCQLSrvFldType&         clsft1
            , const TCQLSrvFldType&         clsft2
            , const tCIDLib::TCard4         c4UserData
        );

        static const TString& strKey
        (
            const   TCQLSrvFldType&         clsftSrc
        );


        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQLSrvFldType();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bAddField
        (
            const   TString&                strToAdd
        );

        virtual tCIDLib::TBoolean bIsEqual
        (
            const   TCQLSrvFldType&         clsftComp
        )   const;

        virtual tCIDLib::TBoolean bIsValidSrcFld
        (
            const   TCQCFldDef&             flddToCheck
        )   const = 0;

        virtual tCIDLib::TBoolean bMoveField
        (
            const   tCIDLib::TCard4         c4At
            , const tCIDLib::TBoolean       bUp
        );

        virtual tCIDLib::TBoolean bValidate
        (
                    TString&                strErr
            ,       tCIDLib::TCard4&        c4SrcFldInd
            , const TCQCFldCache&           cfcData
        );

        virtual tCIDLib::TVoid Initialize
        (
                    TCQCFldCache&           cfcInit
        ) = 0;

        [[nodiscard]] virtual TCQCFldFilter* pffiltToUse();

        virtual tCIDLib::TVoid RemoveField
        (
            const   tCIDLib::TCard4         c4At
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAlwaysStore() const;

        tCIDLib::TBoolean bFindByName
        (
            const   TString&                strName
            ,       tCIDLib::TCard4&        c4Index
        )   const;

        tCIDLib::TBoolean bNormalFld() const;

        tCIDLib::TCard4 c4FldId() const;

        tCIDLib::TCard4 c4FldId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4MaxSrcFields() const;

        tCIDLib::TCard4 c4SrcFldCount() const;

        const tCQLogicSh::TSrcList& colSrcFields() const;

        tCQCKit::EFldTypes eType() const;

        tCQCKit::EFldTypes eType
        (
            const   tCQCKit::EFldTypes      eToSet
        );

        tCIDLib::TVoid Evaluate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TCard4         c4HourInd
            , const tCIDLib::TCard4         c4MinInd
        );

        const TCQCFldDef& flddCfg() const;

        const TCQCFldValue& fvCurrent() const;

        TCQCFldValue& fvCurrent();

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        const TString& strFldName() const;

        const TString& strFldName
        (
            const   TString&                strToSet
        );

        const TString& strSrcFldAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQLSrvFldType
        (
            const   tCIDLib::TBoolean       bNormalFld = kCIDLib::True
            , const tCIDLib::TBoolean       bAlwaysStore = kCIDLib::False
            , const tCIDLib::TCard4         c4MaxSrcFlds = kCQLogicSh::c4MaxSrcFields
        );

        TCQLSrvFldType
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const TString&                strLimits
            , const tCIDLib::TBoolean       bNormalFld = kCIDLib::True
            , const tCIDLib::TBoolean       bAlwaysStore = kCIDLib::False
            , const tCIDLib::TCard4         c4MaxSrcFlds = kCQLogicSh::c4MaxSrcFields
        );

        TCQLSrvFldType
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
            , const tCQCKit::EFldAccess     eAccess
            , const tCIDLib::TBoolean       bNormalFld = kCIDLib::True
            , const tCIDLib::TBoolean       bAlwaysStore = kCIDLib::False
            , const tCIDLib::TCard4         c4MaxSrcFlds = kCQLogicSh::c4MaxSrcFields
        );

        TCQLSrvFldType
        (
            const   TCQLSrvFldType&         clsftSrc
        );

        TCQLSrvFldType& operator=
        (
            const   TCQLSrvFldType&         clsftSrc
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCQLogicSh::EEvalRes eBuildValue
        (
            const   tCQLogicSh::TInfoList&  colVals
            ,       TCQCFldValue&           fldvToFill
            , const tCIDLib::TCard4         c4Hour
            , const tCIDLib::TCard4         c4Minute
        ) = 0;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid MakeValueObj();


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        //
        //  m_bAlwaysStore
        //      Some derivatives want the server to always call them with the new value
        //      on every round of gathering values, even if the value is in error or hasn't
        //      changed. Most don't, so the ctor defaults to that. This is runtime only,
        //      based on the derived class time, so it's not persisted.
        //
        //  m_bNormalFld
        //      Some of the derivatives are not to be treated as normal flds, i.e. not used
        //      by the associated logic server driver to expose as fields. The derived class
        //      ctor passes this in. The server uses it to know which ones to expose via
        //      the driver and which ones not. This is runtime only.
        //
        //  m_c4FldId
        //      A convenience for the driver, so that he can store the field id of the
        //      field he creates to hold the value generated by this virtual fields.
        //      Runtime only, not persisted. It can also be used at config time as a map
        //      back to the original field from a list window.
        //
        //  m_c4MaxSrcFlds
        //      Defaults to the hard maximum.
        //
        //  m_colPollInfo
        //      This is only actually used on the server driver side. For each source field,
        //      we set up one of these to handle keeping our fields registered with the
        //      polling engine and to track the state of our fields. We can handle field
        //      polling at this level and then call our derived class wtih any changes so that
        //      they can update their value.
        //
        //      Since this is only a runtime thing, none of our methods that deal with
        //      managing the source field list have to worry about this one. Not persisted.
        //
        //  m_colSrcFields
        //      The names of the fields that this field uses to build up its actual value.
        //
        //  m_flddCfg
        //      The derived class provides us enough info to set up a field def object for
        //      this field.
        //
        //  m_pfvCurrent
        //      A pointer to a value object that's of the type of this field. When the
        //      derived class builds the value, it's built into here, and it's marked as
        //      in error if the building of the value fails. This really isn't used except
        //      in the server, and isn't persisted.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bAlwaysStore;
        tCIDLib::TBoolean       m_bNormalFld;
        tCIDLib::TCard4         m_c4FldId;
        tCIDLib::TCard4         m_c4MaxSrcFlds;
        tCQLogicSh::TInfoList   m_colPollInfo;
        tCQLogicSh::TSrcList    m_colSrcFields;
        TCQCFldDef              m_flddCfg;
        TCQCFldValue*           m_pfvCurrent;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQLSrvFldType,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDPatFmt
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDPatFmt : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDPatFmt
        (
            const   TString&                strName
        );

        TCQSLLDPatFmt
        (
            const   TCQSLLDPatFmt&          cllftSrc
        );

        TCQSLLDPatFmt(TCQSLLDPatFmt&&) = delete;

        ~TCQSLLDPatFmt();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDPatFmt& operator=
        (
            const   TCQSLLDPatFmt&          clsftSRc
        );

        TCQSLLDPatFmt& operator=(TCQSLLDPatFmt&&) = delete;


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
        const TString& strPattern() const;

        const TString& strPattern
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQSLLDPatFmt();


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


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


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strFmt
        //  m_strFmt2
        //      Temp strings we use to do our formatting. They aren't
        //      persisted.
        //
        //  m_strPattern
        //      The pattern that we use to format out our source fields.
        // -------------------------------------------------------------------
        TString m_strFmt;
        TString m_strFmt2;
        TString m_strPattern;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDPatFmt,TCQLSrvFldType)
        DefPolyDup(TCQSLLDPatFmt)
        BefriendFactory(TCQSLLDPatFmt)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQSLLDMinMaxAvg
//  PREFIX: clsft
// ---------------------------------------------------------------------------
class CQLOGICSHEXPORT TCQSLLDMinMaxAvg : public TCQLSrvFldType
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLLDMinMaxAvg
        (
            const   TString&                strName
            , const tCQCKit::EFldTypes      eType
        );

        TCQSLLDMinMaxAvg
        (
            const   TCQSLLDMinMaxAvg&       cllftSrc
        );

        TCQSLLDMinMaxAvg(TCQSLLDMinMaxAvg&&) = delete;

        ~TCQSLLDMinMaxAvg();


        // -------------------------------------------------------------------
        //  Public oeprators
        // -------------------------------------------------------------------
        TCQSLLDMinMaxAvg& operator=
        (
            const   TCQSLLDMinMaxAvg&       clsftSrc
        );

        TCQSLLDMinMaxAvg& operator=(TCQSLLDMinMaxAvg&&) = delete;


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

        [[nodiscard]] TCQCFldFilter* pffiltToUse() final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQLogicSh::EMMATypes eValType() const;

        tCQLogicSh::EMMATypes eValType
        (
            const   tCQLogicSh::EMMATypes eToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TCQSLLDMinMaxAvg();


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;


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


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eValType
        //      Indicates whether we are doing min, max, or average to derive
        //      our value.
        // -------------------------------------------------------------------
        tCQLogicSh::EMMATypes m_eValType;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLLDMinMaxAvg,TCQLSrvFldType)
        DefPolyDup(TCQSLLDMinMaxAvg)
        BefriendFactory(TCQSLLDMinMaxAvg)
};


#pragma CIDLIB_POPPACK


