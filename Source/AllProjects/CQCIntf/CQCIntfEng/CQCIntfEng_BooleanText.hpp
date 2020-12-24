//
// FILE NAME: CQCIntfEng_BooleanText.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/13/2002
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
//  This is the header for the CQCIntfEng_BooleanText.cpp file, which
//  implements a widget that will be associated with either a field or
//  variable and, based on the evaluate of the field or variable driver via
//  true/false expressions, will display one of two text values, one configured
//  for the false result and one configured for the true result.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfBooleanTextTab;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfBooleanTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfBooleanTextBase

    : public TCQCIntfWidget
    , public MCQCIntfBooleanIntf
    , public MCQCIntfFontIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfBooleanTextBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bUseExtraClrs() const;

        tCIDLib::TBoolean bUseExtraClrs
        (
            const   tCIDLib::TBoolean       bToSet
        );

        const TString& strFalseText() const;

        const TString& strFalseText
        (
            const   TString&                strToSet
        );

        const TString& strTrueText() const;

        const TString& strTrueText
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Set
        (
            const   TString&                strFalseText
            , const TString&                strTrueText
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfBooleanTextBase
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfBooleanTextBase
        (
            const   TCQCIntfBooleanTextBase& iwdgSrc
        );

        TCQCIntfBooleanTextBase& operator=
        (
            const   TCQCIntfBooleanTextBase& iwdgSrc
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

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIntfEng::EBoolStates eCurState() const;

        tCIDLib::TVoid SetState
        (
            const   tCQCIntfEng::EBoolStates eToSet
            , const tCIDLib::TBoolean       bNoRedraw
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bDrawBorder
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseExtraClrs
        //      This lets us optionally use different text colors for false
        //      and true state. This was added in 4.1.901 and uses the recently
        //      added 'extra' colors. It defaults to false for backwards
        //      compabilitty. This is persisted.
        //
        //  m_eCurState
        //      This is the current state that drives the display, and is
        //      kept up to date by the derived classes. This is runtime only.
        //
        //  m_strFalseText
        //  m_strTrueText
        //      If the current value of the field is equal to the false value,
        //      we display the false text string. If its equal to the true
        //      value, we display the true text. Else its in error.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bUseExtraClrs;
        tCQCIntfEng::EBoolStates    m_eCurState;
        TString                     m_strFalseText;
        TString                     m_strTrueText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfBooleanTextBase,TCQCIntfWidget)
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldBooleanText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldBooleanText

    : public TCQCIntfBooleanTextBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldBooleanText();

        TCQCIntfFldBooleanText
        (
            const   TCQCIntfFldBooleanText& iwdgSrc
        );

        ~TCQCIntfFldBooleanText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldBooleanText& operator=
        (
            const   TCQCIntfFldBooleanText& iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        )   override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfFldBooleanText,TCQCIntfBooleanTextBase)
        DefPolyDup(TCQCIntfFldBooleanText)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarBooleanText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarBooleanText

    : public TCQCIntfBooleanTextBase, public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarBooleanText();

        TCQCIntfVarBooleanText
        (
            const   TCQCIntfVarBooleanText& iwdgSrc
        );

        ~TCQCIntfVarBooleanText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarBooleanText& operator=
        (
            const   TCQCIntfVarBooleanText& iwdgSrc
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   override;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptVar
        (
            const   TCQCActVar&             varToTest
        )   override;

        tCIDLib::TVoid QueryBoolAttrLims
        (
                    tCQCKit::EFldTypes&     eType
            ,       TString&                strLimits
        )   const override;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   override;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const override;

        tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
        )   override;

        tCIDLib::TVoid VarInError
        (
            const   tCIDLib::TBoolean       bNoRedraw
        )   override;

        tCIDLib::TVoid VarValChanged
        (
            const   TCQCActVar&             varNew
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfVarBooleanText,TCQCIntfBooleanTextBase)
        DefPolyDup(TCQCIntfVarBooleanText)
};


#pragma CIDLIB_POPPACK

