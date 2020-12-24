//
// FILE NAME: CQCIntfEng_BooleanIntf.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/19/2002
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
//  A number of widget types share the characteristic of having a boolean
//  value which is driven by whether the source field's current value matches
//  a true or false comparison value. These widget types cannot derive from
//  a common class, but we need to have a common interface via which the
//  configuration tab for the true/false values can store user entered data
//  on them.
//
//  This mixin interface provides that ability.
//
//  QueryBoolAttrLims allows us to ask the including class for information about
//  the value driving the true/false state. This is used when our attributes are
//  queried. We encode this info into the expression attributes. The attribute editor
//  window knows this is provided by expression type attributes and passes that info
//  on to the expression editing dialog, which can allow us to provide interactive
//  comparison value selection.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfBooleanIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT MCQCIntfBooleanIntf
{
    public :
        // -------------------------------------------------------------------
        //  Public static methods
        // -------------------------------------------------------------------
        static tCIDLib::TVoid FormatUserData
        (
            const   TCQCFldDef&             flddSrc
            ,       TString&                strToFill
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        MCQCIntfBooleanIntf();

        MCQCIntfBooleanIntf
        (
            const   MCQCIntfBooleanIntf&    miwdgToCopy
        );

        virtual ~MCQCIntfBooleanIntf();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=
        (
            const   MCQCIntfBooleanIntf&    miwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoBoolCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
        );

        tCIDLib::TBoolean bSameBool
        (
            const   MCQCIntfBooleanIntf&    miwdgSrc
        )   const;

        tCQCIntfEng::EExprResults eCompareValue
        (
            const   TCQCFldValue&           fvToCompare
            , const TStdVarsTar&            ctarGlobalVars
        )   const;

        const TCQCExpression& exprFalse() const;

        const TCQCExpression& exprTrue() const;

        tCIDLib::TVoid QueryBoolAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        tCIDLib::TVoid QueryBoolCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const;

        tCIDLib::TVoid ReadInBool
        (
                    TBinInStream&           strmToReadFrom
        );

        tCIDLib::TVoid ReplaceBool
        (
            const   TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        tCIDLib::TVoid ResetBool();

        tCIDLib::TVoid SetBoolAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        tCIDLib::TVoid SetFalseVal
        (
            const   TString&                strVal
            , const tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
        );

        tCIDLib::TVoid SetTrueVal
        (
            const   TString&                strVal
            , const tCQCKit::EExprTypes     eType
            , const tCQCKit::EStatements    eStatement
        );

        tCIDLib::TVoid WriteOutBool
        (
                    TBinOutStream&          strmToWriteTo
        )   const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_exprFalse
        //  m_exprTrue
        //      The two expressions that we use to decide what state to
        //      set on ourself.
        // -------------------------------------------------------------------
        TCQCExpression  m_exprFalse;
        TCQCExpression  m_exprTrue;
};

#pragma CIDLIB_POPPACK


