//
// FILE NAME: CQCIntfEng_NumericText.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/24/2004
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
//  This is the header for the CQCIntfEng_NumericText.cpp file, which
//  implements a simple derivative of the base widget. It only accepts
//  Float, Card, and Int fields, and provides flexible formatting for
//  numeric values. It supports raw or locale sensitive formatting, and
//  leading/trailing fixed text.
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
//   CLASS: TCQCIntfNumericText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfNumericText

    : public TCQCIntfWidget
    , public MCQCIntfFontIntf
    , public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfNumericText();

        TCQCIntfNumericText
        (
            const   TArea&                  areaInit
            , const TPoint&                 pntParent
        );

        TCQCIntfNumericText
        (
            const   TCQCIntfNumericText&    iwdgToCopy
        );

        ~TCQCIntfNumericText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfNumericText& operator=
        (
            const   TCQCIntfNumericText&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

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
        tCIDLib::TVoid Set
        (
            const   tCIDLib::TBoolean       bUseGroups
            , const tCIDLib::TCard4         c4DecDigits
            , const tCIDLib::TCard4         c4FldWidth
            , const tCIDLib::TCh            chFill
            , const tCIDLib::EHJustify      eJustify
            , const tCQCIntfEng::ENumTxtStyles eStyle
            , const TString&                strLeadingText
            , const TString&                strTrailingText
        );


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TCQCIntfNumericTextTab;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   override;

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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaUpdate
            , const TArea&                  areaDraw
        );

        tCIDLib::TVoid FormatCard
        (
            const   tCIDLib::TCard4         c4Val
            ,       TString&                strToFill
        );

        tCIDLib::TVoid FormatFloat
        (
            const   tCIDLib::TFloat8        f8Val
            ,       TString&                strToFill
        );

        tCIDLib::TVoid FormatInt
        (
            const   tCIDLib::TInt4          i4Val
            ,       TString&                strToFill
        );

        tCIDLib::TVoid Reformat
        (
            const   tCQCKit::EFldTypes      eType
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bUseGroups
        //      Indicates, when custom formatting is used, whether group
        //      separators should be used.
        //
        //  m_c4DecDigits
        //      For floating point values, indicates number of decimal digits
        //      to display, when custom formatting.
        //
        //  m_eStyle
        //      Indicates the basic style of display.
        //
        //  m_strLeadingText
        //  m_strTrailingText
        //      Any fixed leading or trailng text we need.
        //
        //  m_strLiveText
        //      This is the live text, which we update any time we are told
        //      our value is changed, so that we don't have to reformat it
        //      on demand every time. This is not peristed.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bUseGroups;
        tCIDLib::TCard4             m_c4DecDigits;
        tCIDLib::TCard4             m_c4FldWidth;
        tCIDLib::TCh                m_chFill;
        tCIDLib::EHJustify          m_eJustify;
        tCQCIntfEng::ENumTxtStyles  m_eStyle;
        TString                     m_strLeadingText;
        TString                     m_strTrailingText;
        TString                     m_strLiveText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfNumericText,TCQCIntfWidget)
        DefPolyDup(TCQCIntfNumericText)
};

#pragma CIDLIB_POPPACK


