//
// FILE NAME: CQCIntfEng_StaticText.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/06/2002
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
//  This is the header for the CQCIntfEng_StaticText.cpp file, which implements
//  a simple static text widget. It mixes in the font and caption interfaces
//  which provide all of the data storage and configuration tab support. We
//  just have to handle drawing here.
//
//  We also do an ultra-trivial derivative of the static text class which
//  exists just for theme support purposes. We want to have 'Titles' in the
//  theme, which have their own theme settings. They are just static text
//  widgets, but there has to be a separate class to let the theme system
//  know the difference when themes are applied.
//
//  And we do a derivative that is used for browsing media metadata. These
//  display some value for a selected title and are basically just static
//  text widgets until they are told to view a new title. So the derivative
//  just has to provide the DoCmd() and QueryCmds() overrides to add a new
//  command over what's already supported.
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
//   CLASS: TCQCIntfStaticText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfStaticText

    : public TCQCIntfWidget
    , public MCQCIntfFontIntf
    , public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfStaticText();

        TCQCIntfStaticText
        (
            const   tCIDLib::TBoolean       bMarquee
            , const tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfStaticText
        (
            const   TCQCIntfStaticText&     iwdgToCopy
        );

        ~TCQCIntfStaticText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfStaticText& operator=
        (
            const   TCQCIntfStaticText&     iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

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

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

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


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const tCIDLib::TBoolean         bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
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
        //  m_bPatternMode
        //  m_bSecretMode
        //      They can send us a SetSecretMode command, to put us into and
        //      out of secret mode. In secret mode, we display asterisks for
        //      the characters. The SetPattern command will set or clear
        //      an input pattern regular expression (empty string disables
        //      pattern checking.) If m_bPatternMode is set, then any Append
        //      operations will be tested against m_regxPattern.
        //
        //  m_c4AvgCharWidth
        //  m_c4TextWidth
        //      The width of our text string and the average width of a
        //      character, in our configured font. If we are a marquee mode
        //      widget, we'll use this to do the scrolling of the text. If
        //      we aren't marquee mode, these won't get set. They are runtime
        //      only and are not persisted.
        //
        //  m_c4MaxChars
        //      They can invoke our SetMaxChars command to set a maximum
        //      number of chars we'll accept via Append(). It's defaulted to
        //      c4MaxCard to have no limit.
        //
        //  m_enctNextScroll
        //      This is used in marquee mode, to provide smoother scrolling.
        //      We need to time each scroll.
        //
        //  m_i4LastOfs
        //      When in marquee mode, we need to keep up with where we last
        //      drew the string. We slowly pan it to the left until we've
        //      displayed the whole thing, and then pick up again at the far
        //      right.
        //
        //  m_regxPattern
        //      They can set an input pattern on us, which we use to accept
        //      or deny characters sent via the Append command. The pattern
        //      is a simple regular expression. If they've set a pattern,
        //      the m_bPatternMode flag will be set, so that we know to compare
        //      the new contents against the expression. They can clear the
        //      pattern by sending an empty string. They set this via the
        //      SetPattern command.
        //
        //      It's mutable so that we can do a test set of the expression
        //      during command validation.
        //
        //  m_strLastError
        //      If we get an error during a SendValue command, we store the
        //      error text here and post an OnError command.
        //
        //  m_strSecretText
        //      Used during drawing to create the *** type string that we
        //      display in secret mode. To keep from having to create and
        //      destroy a dynamic string on each display.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bMarquee;
        tCIDLib::TBoolean       m_bPatternMode;
        tCIDLib::TBoolean       m_bSecretMode;
        tCIDLib::TCard4         m_c4AvgCharWidth;
        tCIDLib::TCard4         m_c4MaxChars;
        tCIDLib::TCard4         m_c4TextWidth;
        tCIDLib::TEncodedTime   m_enctNextScroll;
        tCIDLib::TInt4          m_i4LastOfs;
        mutable TRegEx          m_regxPattern;
        TString                 m_strLastError;
        TString                 m_strSecretText;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfStaticText,TCQCIntfWidget)
        DefPolyDup(TCQCIntfStaticText)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTitleText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfTitleText : public TCQCIntfStaticText
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfTitleText();

        TCQCIntfTitleText
        (
            const   TCQCIntfTitleText&      iwdgToCopy
        );

        ~TCQCIntfTitleText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfTitleText& operator=
        (
            const   TCQCIntfTitleText&      iwdgToAssign
        );


    private :
        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfTitleText,TCQCIntfStaticText)
        DefPolyDup(TCQCIntfTitleText)
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaRepoText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMediaRepoText : public TCQCIntfStaticText
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMediaRepoText();

        TCQCIntfMediaRepoText
        (
            const   TCQCIntfMediaRepoText&  iwdgToCopy
        );

        ~TCQCIntfMediaRepoText();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMediaRepoText& operator=
        (
            const   TCQCIntfMediaRepoText&  iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

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
                    TCollection<TCQCCmd>&   colToFill
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

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCQCMedia::EMTextAttrs eAttr() const;

        tCQCMedia::EMTextAttrs eAttr
        (
            const   tCQCMedia::EMTextAttrs  eToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
             const  TString&                strToSet
        );


    protected :
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


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eAttr
        //      The particular attribute we are to display. This is a config
        //      time setting.
        //
        //  m_strRepoMoniker
        //      The moniker of the repository driver that we are associated
        //      with. This is a design time thing.
        // -------------------------------------------------------------------
        tCQCMedia::EMTextAttrs  m_eAttr;
        TString                 m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMediaRepoText,TCQCIntfStaticText)
        DefPolyDup(TCQCIntfMediaRepoText)
};

#pragma CIDLIB_POPPACK


