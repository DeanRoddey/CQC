//
// FILE NAME: CQCIntfEng_DynamicText.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/12/2002
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
//  This is the header for the CQCIntfEng_DynamicText.cpp file, which
//  displays the value of a field or variable textually. We have a base
//  class that does the bulk of the work, and derivatives for field or
//  variable based ones that just handle figuring out what the value is and
//  telling the base class.
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
//   CLASS: TCQCIntfDynTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfDynTextBase

    : public TCQCIntfWidget, public MCQCIntfFontIntf, public MCQCIntfXlatIntf
{
    public :
        // -------------------------------------------------------------------
        //  Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfDynTextBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

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


    protected :
        // -------------------------------------------------------------------
        //  Give our config tab classes access to our internals
        // -------------------------------------------------------------------
        friend class TCQCIntfXlatTab;


        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfDynTextBase
        (
            const   tCIDLib::TBoolean       bMarqueeMode
            , const TString&                strTypeName
        );

        TCQCIntfDynTextBase
        (
            const   TCQCIntfDynTextBase&    iwdgToCopy
        );

        TCQCIntfDynTextBase& operator=
        (
            const   TCQCIntfDynTextBase&    iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   override;

        tCIDLib::TVoid CaptionChanged() override;

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
        tCIDLib::TVoid SetText
        (
            const   TString&                strToSet
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bErrState
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TArea&                  areaDraw
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bMarquee
        //      We can be used in a Marquee mode, in which we will rotate the
        //      string contents through the available text area. When in this
        //      mode, we won't allow transparent background and we get only
        //      a single line of text.
        //
        //      We don't persist this, it's defaulted to false and only set
        //      if a derived class sets it.
        //
        //  m_bValError
        //      Let's the derived class tell us if we are in error state or
        //      not, i.e. he can't give us a good value.
        //
        //  m_c4AvgCharWidth
        //  m_c4TextWidth
        //      The width of our text string and the average width of a
        //      character, in our configured font. If we are a marquee mode
        //      widget, we'll use this to do the scrolling of the text. If
        //      we aren't marquee mode, these won't get set. They are runtime
        //      only and are not persisted.
        //
        //  m_enctNextScroll
        //      This is used in marquee mode, to provide smoother scrolling.
        //      We need to time each scroll.
        //
        //  m_i4LastOfs
        //      When in marquee mode, we need to keep up with where we last
        //      drew the string. We slowly pan it to the left until we've
        //      displayed the whole thing, and then pick up again at the far
        //      right. This is not persisted.
        //
        //  m_strDisplayText
        //  m_strLiveText
        //      The actual text we display, and the 'live' text that a derived
        //      class may provide us to display. The display text is possibly
        //      the live value formatted into the caption text, or just the
        //      caption. We have to store the live text as well, since the
        //      caption might change and we'll need it to update.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bMarquee;
        tCIDLib::TBoolean       m_bValError;
        tCIDLib::TCard4         m_c4AvgCharWidth;
        tCIDLib::TCard4         m_c4TextWidth;
        tCIDLib::TEncodedTime   m_enctNextScroll;
        tCIDLib::TInt4          m_i4LastOfs;
        TString                 m_strDisplayText;
        TString                 m_strLiveText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfDynTextBase,TCQCIntfWidget)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfFldText

    : public TCQCIntfDynTextBase, public MCQCIntfSingleFldIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfFldText();

        TCQCIntfFldText
        (
            const   tCIDLib::TBoolean       bMarqueeMode
        );

        TCQCIntfFldText
        (
            const   TCQCIntfFldText&        iwdgToCopy
        );

        ~TCQCIntfFldText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfFldText& operator=
        (
            const   TCQCIntfFldText&        iwdgToAssign
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
        RTTIDefs(TCQCIntfFldText,TCQCIntfDynTextBase)
        DefPolyDup(TCQCIntfFldText)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfVarText

    : public TCQCIntfDynTextBase
    , public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarText();

        TCQCIntfVarText
        (
            const   tCIDLib::TBoolean       bMarqueeMode
        );

        TCQCIntfVarText
        (
            const   TCQCIntfVarText&        iwdgToCopy
        );

        ~TCQCIntfVarText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarText& operator=
        (
            const   TCQCIntfVarText&        iwdgToAssign
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
        RTTIDefs(TCQCIntfVarText,TCQCIntfDynTextBase)
        DefPolyDup(TCQCIntfVarText)
};

#pragma CIDLIB_POPPACK


