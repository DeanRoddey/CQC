//
// FILE NAME: CQCIntfEng_SingleFldIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/20/2008
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
//  This is the header for the CQCIntfEng_SingleFldIntf.cpp file, which
//  implements a simple mixin interface that all dynamic widgets based on
//  a single field value will use. This serves two purposes:
//
//  1)  It allows us to keep the old Field tab for selecting the associated
//      field, because it can provide a simple interface via which that tab
//      can interface to the widgets that use it. This is the simplest way
//      to support single field based widgets.
//  2)  It can handle a lot of common functionality that all such field based
//      widgets need to handle, leaving the amount of field related goop in
//      the actual widgets extremely small.
//
//  The SetField method is virtual because in some cases, the configuration
//  of something else changes when a new field is selected, such as with the
//  enumerated image widget, which puts an entry in the images list for each
//  value of the enumerated value. So it needs to watch for field changes.
//  But otherwise widgets don't care about design time changes to the
//  field and just adapt to it at viewing time.
//
// CAVEATS/GOTCHAS:
//
//  1)  Classes that use this interface must include the 'wants value updates'
//      capabilities flag, so that the container class will call us regularly
//      to update our field value.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfSingleFldIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~MCQCIntfSingleFldIntf();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
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
        );

        tCIDLib::TBoolean bDesQueryFldDef
        (
                    TCQCFldDef&             flddTar
        )   const;

        tCIDLib::TBoolean bDesQueryFldInfo
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const;

        tCIDLib::TBoolean bGoodFieldValue() const;

        tCIDLib::TBoolean bHasField() const;

        tCIDLib::TBoolean bNoAutoWrite() const;

        tCIDLib::TBoolean bNoAutoWrite
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bSameField
        (
            const   MCQCIntfSingleFldIntf&  miwdgSrc
        )   const;

        tCIDLib::TBoolean bValidateField
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
            , const TCQCIntfWidget&         iwdgThis
        )   const;

        tCIDLib::TVoid ChangeField
        (
            const   TString&                strField
        );

        tCIDLib::TVoid ClearFieldAssoc();

        tCQCPollEng::EFldStates eFieldState() const;

        tCQCKit::EFldTypes eFieldType() const;

        const TCQCFldFilter& ffiltToUse() const;

        const TCQCFldDef& flddAssoc() const;

        const TCQCFldValue& fvCurrent() const;

        tCIDLib::TVoid FieldUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        );

        tCIDLib::TVoid QueryFieldMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const;

        tCIDLib::TVoid QueryFieldAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
            , const TString&                strItemText
        )   const;

        tCIDLib::TVoid ReadInField
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid RegisterSingleField
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        tCIDLib::TVoid ReplaceField
        (
            const   TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        const TString& strFieldName() const;

        const TString& strFullFieldName() const;

        const TString& strMoniker() const;

        tCIDLib::TVoid SetFieldAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetFieldFilter
        (
                    TCQCFldFilter* const    pffiltToAdopt
        );

        tCIDLib::TVoid WriteOutField
        (
                    TBinOutStream&          strmToWriteTo
        )   const;



    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        MCQCIntfSingleFldIntf();

        MCQCIntfSingleFldIntf
        (
                    TCQCFldFilter* const    pffiltToAdopt
        );

        MCQCIntfSingleFldIntf
        (
            const   MCQCIntfSingleFldIntf&  iwdgToCopy
        );

        MCQCIntfSingleFldIntf& operator=
        (
            const   MCQCIntfSingleFldIntf&  iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        ) = 0;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNoAutoWrite
        //      Some widgets associated with fields may not always want the widget to
        //      automatically write to a field when the user releases, but they still want
        //      to track the field from a read only point of view. So they can make use
        //      of this value. We just store it and load it, but don't make use of it
        //      ourself.
        //
        //  m_cfpiAssoc
        //      This is the polling info object that we use to interface to the polling
        //      engine. Mostly it's viewing time info, but it holds the moniker and field
        //      name of the associated field. They are streamed in and set up on the poll
        //      info object when we are loaded.
        //
        //  m_pffiltToUse
        //      The including class must provide us with a field filter that will be used
        //      when the list of possible fields are presented to the user. This isn't a
        //      persistent field, it's just used for editing purposes.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNoAutoWrite;
        TCQCFldPollInfo     m_cfpiAssoc;
        TCQCFldFilter*      m_pffiltToUse;
};

#pragma CIDLIB_POPPACK

