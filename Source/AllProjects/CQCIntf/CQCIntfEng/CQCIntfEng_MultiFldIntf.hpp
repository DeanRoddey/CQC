//
// FILE NAME: CQCIntfEng_MultiFldIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/26/2012
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
//  This is the header for the CQCIntfEng_MultiFldIntf.cpp file, which
//  implements a simple mixin interface that all dynamic widgets based on
//  multiple field values will use. There is a single field mixin, which is
//  simpler and so is used by the bulk of field based widgets, since they
//  only use a single field. This one is less used, since not that many fields
//  require more than one field.
//
//  The interface is very similar to the single field mixin, it mostly differs
//  in that it uses an index to indicate which of the fields are being set
//  or gotten or referred to. The calling class indicates the number of fields
//  that are to be supported and it can't change once set.
//
//  It would make not sense to do so, but don't use this and the single field
//  mixin on the same class.
//
// CAVEATS/GOTCHAS:
//
//  1)  Classes that use this interface must include the 'wants value updates'
//      capabilities flag, so that the container class will call the widget's
//      ValueUpdate() method, which it should then use to call our FieldUpdate()
//      method so that we can poll the field values and report changes.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfMultiFldIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfMultiFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MCQCIntfMultiFldIntf();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const tCIDLib::TCard4         c4FldInd
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AssociateField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const tCIDLib::TCard4         c4FldInd
        );

        tCIDLib::TBoolean bGoodFieldValue
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        tCIDLib::TBoolean bHasField
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        tCIDLib::TBoolean bSameFields
        (
            const   MCQCIntfMultiFldIntf&   miwdgSrc
        )   const;

        tCIDLib::TVoid ChangeField
        (
            const   TString&                strField
            , const tCIDLib::TCard4         c4FldInd
        );

        tCIDLib::TVoid ClearFieldAssoc
        (
            const   tCIDLib::TCard4         c4FldInd
        );

        tCQCPollEng::EFldStates eFieldState
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        tCQCKit::EFldTypes eFieldType
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        const TCQCFldFilter& ffiltToUse
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        const TCQCFldDef& flddAssoc
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        const TCQCFldValue& fvCurrent
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        tCIDLib::TVoid FieldUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        );

        tCIDLib::TVoid QueryMFieldAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid QueryFieldMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const;

        tCIDLib::TVoid ReadInFields
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid RegisterMultiFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid ReplaceMultiFields
        (
            const   TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        const TString& strFieldName
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        const TString& strMoniker
        (
            const   tCIDLib::TCard4         c4FldInd
        )   const;

        tCIDLib::TVoid SetMFieldAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetFieldFilter
        (
                    TCQCFldFilter* const    pffiltToAdopt
            , const tCIDLib::TCard4         c4FldInd
        );

        tCIDLib::TVoid ValidateFields
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
            , const TCQCIntfWidget&         iwdgThis
        )   const;

        tCIDLib::TVoid WriteOutFields
        (
                    TBinOutStream&          strmToWriteTo
        )   const;



    protected :
        // -------------------------------------------------------------------
        //  A simple class, used internally, to track the info for each of
        //  the fields we are set up for.
        // -------------------------------------------------------------------
        class CQCINTFENGEXPORT TDFldInfo
        {
            public :
                TDFldInfo();
                TDFldInfo(const TDFldInfo& dfldiSrc);
                TDFldInfo& operator=(const TDFldInfo& dfldiSrc);
                ~TDFldInfo();

                TCQCFldPollInfo     m_cfpiAssoc;
                TCQCFldFilter*      m_pffiltToUse;
        };


        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCIntfMultiFldIntf
        (
            const   tCIDLib::TCard4         c4FldCount
        );

        MCQCIntfMultiFldIntf
        (
            const   MCQCIntfMultiFldIntf&   iwdgToCopy
        );

        MCQCIntfMultiFldIntf& operator=
        (
            const   MCQCIntfMultiFldIntf&   iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TCard4         c4FldIndex
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        ) = 0;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_objaFlds
        //      The list of fields we are tracking. It is sized in the ctor
        //      based on the field count indicated, or when streamed back
        //      in. They may not all be used.
        // -------------------------------------------------------------------
        TObjArray<TDFldInfo>    m_objaFlds;
};

#pragma CIDLIB_POPPACK


