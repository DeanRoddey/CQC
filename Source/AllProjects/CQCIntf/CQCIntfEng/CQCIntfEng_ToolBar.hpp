//
// FILE NAME: CQCIntfEng_Toolbar.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/29/2005
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
//  This is the header for the CQCIntfEng_Toolbar.cpp file, which implements
//  a horizontal tool bar. It manages a set of image/text pairs that it display
//  in a scrollable horizontal layout. They can control whether the text is
//  on top of or below the image, or if they both are centered.
//
//  Each item in the list is configured to send widget commands, so they are
//  pretty flexible as to what they can do. We define an internal class that
//  holds the config info for each button defined. It implements the widget
//  command mixin so that the widget command info can be configured via the
//  standard dialog.
//
//  We implement the images interface so that they can select images to use
//  for focus and pressed emphasis.
//
//  As of 4.0.17 one of the buttons can be marked for special display emphasis,
//  so we added in a new image key for a marked image, and we created our own
//  RTV derivative so we can pass along the marked and selected button
//  indicies when a button is clicked.
//
//  We support all the usual scrolling and dragging stuff.
//
//  For 5.1.909 we added a new variable based derivative, that uses the value of
//  the variable to set the marked button index. The variable must be an enumerated
//  string type. The ordinal of it is used to set the index, so the values must be
//  in the same order as the toolbar buttons. Other than handling the variable
//  interface stuff, the base class does all the work.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfToolbarTab;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfToolbar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfToolbar

    : public TCQCIntfWidget
    , public MCQCIntfFontIntf
    , public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  We need a class to hold the per-button info. We mix in the widget
        //  command interface, so it manages the command the button sends. We
        //  store the text and the image here.
        // -------------------------------------------------------------------
        class CQCINTFENGEXPORT TTBButton

            : public TObject, public MCQCCmdSrcIntf, public MStreamable
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TTBButton();

                TTBButton
                (
                    const   TString&                strText
                    ,       TCQCIntfToolbar* const  piwdgParent
                );

                TTBButton(const TTBButton& tbbToCopy);

                ~TTBButton();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                tCIDLib::TVoid operator=(const TTBButton& tbbSrc);

                tCIDLib::TBoolean operator==(const TTBButton& tbbSrc) const;

                tCIDLib::TBoolean operator!=(const TTBButton& tbbSrc) const;


                // -----------------------------------------------------------
                //  Public, inherited methods
                // -----------------------------------------------------------
                [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
                (
                    const   TCQCUserCtx&            cuctxToUse
                )   const override;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bClrTrans() const;

                tCIDLib::TBoolean bPressed() const;

                tCIDLib::TBoolean bPressed
                (
                    const   tCIDLib::TBoolean       bToSet
                );

                tCIDLib::TCard1 c1Opacity() const;

                tCIDLib::TCard1 c1Opacity
                (
                    const   tCIDLib::TCard1         c1ToSet
                );

                tCIDLib::TVoid ClearImage();

                tCIDLib::TVoid DrawOn
                (
                            TGraphDrawDev&          gdevToUse
                    , const TArea&                  areaTar
                );

                const TString& strImagePath() const;

                const TString& strImagePath
                (
                    const   TString&                strToSet
                );

                const TString& strText() const;

                const TString& strText
                (
                    const   TString&                strToSet
                );

                const TSize& szImage() const;

                tCIDLib::TVoid LoadImage
                (
                    const   TCQCIntfView&           civOwner
                );

                tCIDLib::TVoid LoadImage
                (
                    const   TCQCIntfView&           civOwner
                    ,       TDataSrvClient&         dsclInit
                );

                tCIDLib::TVoid SetParent
                (
                            TCQCIntfToolbar* const  piwdgParent
                );


            protected :
                // -----------------------------------------------------------
                //  Protected, inherited methods
                // -----------------------------------------------------------
                tCIDLib::TVoid StreamFrom(TBinInStream& strmToReadFrom) override;

                tCIDLib::TVoid StreamTo(TBinOutStream& strmToWriteTo) const override;


            private :
                // -----------------------------------------------------------
                //  Private, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TVoid SetupCmdSrc();


                // -----------------------------------------------------------
                //  Private data members
                //
                //  m_bPressed
                //      This is just used by the Clicked() method of the
                //      toolbar, to be able to cause a particular button to
                //      be drawn pressed or not. It's only temporarily set
                //      long enough to do the simulated press/release feedback.
                //
                //  m_c1Opacity
                //      The opacity level for the image associated with this
                //      button.
                //
                //  m_cptrImg
                //      A counted pointer to the image cache item of the image
                //      we are configured to dispay.
                //
                //  m_piwdgParent
                //      We need a pointer to our parent toolbar, so that we
                //      can get info from it in some cases.
                //
                //  m_strImagePath
                //      The path to the image for this button. This one is
                //      persisted. If this is empty, then we don't have an
                //      image for this button.
                //
                //  m_szImage
                //      The size of the image, which we use often, so we get
                //      it out when we load the image.
                // -----------------------------------------------------------
                tCIDLib::TBoolean   m_bPressed;
                tCIDLib::TCard1     m_c1Opacity;
                TIntfImgCachePtr    m_cptrImg;
                TCQCIntfToolbar*    m_piwdgParent;
                TString             m_strImagePath;
                TString             m_strText;
                TSize               m_szImage;


                // -----------------------------------------------------------
                //  Magic Macros
                // -----------------------------------------------------------
                RTTIDefs(TCQCIntfToolbar::TTBButton,TObject)
        };
        using TButtonList = TVector<TTBButton>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfToolbar();

        TCQCIntfToolbar
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
        );

        TCQCIntfToolbar
        (
            const   TCQCIntfToolbar&        iwdgToCopy
        );

        ~TCQCIntfToolbar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfToolbar& operator=
        (
            const   TCQCIntfToolbar&        iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bMoveFocus
        (
            const   tCQCKit::EScrNavOps     eOp
        )   override;

        tCIDLib::TBoolean bPerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        )   override;

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        )   override;

        tCIDLib::TBoolean bReferencesTarId
        (
            const   TString&                strId
        )   const override;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TCard4 c4QueryEmbeddedCmdSrcs
        (
                    tCQCKit::TCmdSrcList&   colToFill
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

        tCIDLib::TVoid Invoke() override;

        tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        )   override;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const override;

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

        tCIDLib::TVoid RefreshImages
        (
                    TDataSrvClient&         dsclToUse
        )   override;

        TSize szDefaultSize() const override;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   override;

        tCIDLib::TVoid ScaleInternal
        (
            const   tCIDLib::TFloat8       f8XScale
            , const tCIDLib::TFloat8       f8YScale
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


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddButton
        (
            const   TTBButton&              tbbToAdd
        );

        tCIDLib::TBoolean bFillSlots() const;

        tCIDLib::TBoolean bFillSlots
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4ButtonCnt() const;

        tCIDLib::TCard4 c4Spacing() const;

        tCIDLib::TCard4 c4Spacing
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Width() const;

        tCIDLib::TCard4 c4Width
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCIntfEng::ETBLayouts eLayout() const;

        tCQCIntfEng::ETBLayouts eLayout
        (
            const   tCQCIntfEng::ETBLayouts eToSet
        );

        tCIDLib::TInt4 i4VertAdjust() const;

        tCIDLib::TInt4 i4VertAdjust
        (
            const   tCIDLib::TInt4          i4ToSet
        );

        tCIDLib::TVoid MarkButton
        (
            const   tCIDLib::TCard4         c4ButtInd
            , const tCIDLib::TBoolean       bState
            , const tCIDLib::TBoolean       bRedraw = kCIDLib::False
        );

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const;

        TTBButton* ptbbWithText
        (
            const   TString&                strToFind
            ,       tCIDLib::TCard4&        c4Index
        );

        tCIDLib::TVoid RemoveButtonAt
        (
            const   tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid StreamButtons
        (
                    TBinOutStream&          strmTar
        );

        tCIDLib::TVoid SwapButtons
        (
            const   tCIDLib::TCard4         c4FirstInd
            , const tCIDLib::TCard4         c4SecInd
        );

        TTBButton& tbbAt
        (
            const   tCIDLib::TCard4         c4At
        );

        const TTBButton& tbbAt
        (
            const   tCIDLib::TCard4         c4At
        )   const;


        tCIDLib::TVoid Unmark
        (
            const   tCIDLib::TBoolean       bRedraw
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid GotFocus() override;

        tCIDLib::TVoid LostFocus() override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   override;

        tCIDLib::TVoid StyleFlagChanged
        (
            const   tCIDLib::TCard4         c4OldFlags
            , const tCIDLib::TCard4         c4NewFlags
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
        tCIDLib::TVoid AdjustBySlot
        (
                    TArea&                  areaAdjust
            , const tCIDLib::TBoolean       bIncludeSpace
        )   const;

        tCIDLib::TVoid AreaForIndex
        (
            const   tCIDLib::TCard4         c4Index
            ,       TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid AreaForIndex
        (
            const   tCIDLib::TCard4         c4Index
            , const TArea&                  areaRelTo
            ,       TArea&                  areaToFill
        )   const;

        tCIDLib::TBoolean bScrollList
        (
            const   TString&                strScrollOp
            , const tCIDLib::TBoolean       bRedraw
        );

        tCIDLib::TBoolean bVisibleEnough
        (
                    tCIDLib::TCard4&        c4Index
            , const tCIDLib::TFloat4        f4PerLimit
        )   const;

        TBitmap bmpGatherBgn();

        tCIDLib::TCard4 c4ClosestVisIndex
        (
            const   tCIDLib::TCard4         c4FromHere
        )   const;

        tCIDLib::TCard4 c4FirstVisIndex() const;

        tCIDLib::TCard4 c4FirstVisIndex
        (
            const   TArea&                  areaVis
            , const TArea&                  areaRelTo
        ) const;

        tCIDLib::TCard4 c4HitTest
        (
            const   TPoint&                 pntTest
        )   const;

        tCIDLib::TVoid CalcMaxAndHeight
        (
                    TGraphDrawDev&          gdevTarget
            ,       tCIDLib::TCard4&        c4MaxImgVS
            ,       tCIDLib::TCard4&        c4TextHeight
        );

        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid DrawAButton
        (
                    TGraphDrawDev&          gdevTarget
            ,       TTBButton&              tbbToDraw
            , const TArea&                  areaSlot
            , const tCIDLib::TCard4         c4MaxImgVS
            , const tCIDLib::TCard4         c4TextHeight
            , const tCIDLib::TBoolean       bDoFocus
            , const tCIDLib::TBoolean       bMarked
            , const tCIDLib::TBoolean       bForceAlpha
        );

        tCIDLib::TVoid DrawButtons
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaRelTo
            , const TArea&                  areaInvalid
            , const TPoint&                 pntAdjust
            , const tCIDLib::TBoolean       bForceAlpha
        );

        tCIDLib::TVoid KeepFocusVisible();

        tCIDLib::TVoid SlideList
        (
            const   TArea&                  areaOrg
        );

        tCIDLib::TVoid UpdateButtonIfVisible
        (
            const   tCIDLib::TCard4         c4Index
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaContent
        //      The current content area, which is the size of all of our
        //      buttons as displayed. We keep it at zero origin so that we
        //      don't have to worry about moving it around if we move.
        //
        //  m_areaAdjDisplay
        //      This is the display area, relative to our content area and
        //      scrolled around over it to track where we currently are
        //      scrolled to. So, when scrolling, this is where we blit from
        //      to show the current scrolled contents.
        //
        //  m_areaDisplay
        //      The area within our widget area where we actually display the
        //      buttons. We move it in for border, any margin we want, a bit
        //      more margin if rounded corners, etc... It's centered in both
        //      directions within our widget area. Take this and move it right
        //      by the scroll offset, and that's the part of the content area
        //      currently visible. Or, vice versa, move the content area
        //      left.
        //
        //  m_bFillSlots
        //      If this is set, and the Base tab's Transparent option is checked,
        //      then we apply the background fill attributes to the slots instead
        //      of the toolbar as a whole.
        //
        //  m_bSafePanBgn
        //      When we get a start of gesture, we remember if the bgn is of
        //      a type that is safe to build all into a single bitmap image
        //      and scroll (or if we we need to composite on each round.) So
        //      this is then used later during the move events.
        //
        //  m_bmpXXX
        //      We need some bitmaps during panning and flicking. The content
        //      one is what we draw the actual contents into. The bgn and dbuf
        //      ones are required if we don't have a bgn fill that is safe
        //      for the direction we are scrolling, to composite the bgn
        //      and the (alpha based) content into the content buffer.
        //
        //  m_bmpSlot
        //      If we are in 'fill slots' mode, then we need a temp bitmap to
        //      do the fill in an alpha friendly way.
        //
        //  m_c4FocusIndex
        //      If our containing template is focus oriented, this is the
        //      button with the focus.
        //
        //  m_c4Spacing
        //      The inter-value spacing between the genre name display areas.
        //
        //  m_c4MarkedIndex
        //      We allow one of the items to be 'marked', which will cause it
        //      to use alternate text colors and optionally a background image.
        //      We provide a method to mark/unmark an item by index. Only one
        //      can be marked so any existing marked item is unmarked when a
        //      new one is marked. If none is marked this is c4MaxCard.
        //
        //  m_c4SelectedIndex
        //      Once we've tracked a button and need to invoke the actions
        //      associated, we have to clear the tracking index, since we
        //      are no longer track, but we need to remember the selected
        //      button for later use in the RTVs. So we store it here.
        //
        //  m_c4Size
        //      The configured width of each button.
        //
        //  m_colButtons
        //      These are the TTBButton objects that we are configured for.
        //      Each one represents a button.
        //
        //  m_eLayout
        //      Indicates the layout of the tool bar, whether it's text above
        //      and image below, or vice versa, or centered, etc...
        //
        //  m_i4VertAdjust
        //      They can adjust layout vertically to get text/images layed
        //      out better.
        // -------------------------------------------------------------------
        TArea                   m_areaContent;
        TArea                   m_areaAdjDisplay;
        TArea                   m_areaDisplay;
        tCIDLib::TBoolean       m_bFillSlots;
        tCIDLib::TBoolean       m_bSafePanBgn;
        TBitmap                 m_bmpBgn;
        TBitmap                 m_bmpCont;
        TBitmap                 m_bmpDBuf;
        TBitmap                 m_bmpSlot;
        tCIDLib::TCard4         m_c4FocusIndex;
        tCIDLib::TCard4         m_c4MarkedIndex;
        tCIDLib::TCard4         m_c4Spacing;
        tCIDLib::TCard4         m_c4SelectedIndex;
        tCIDLib::TCard4         m_c4Size;
        TButtonList             m_colButtons;
        tCQCIntfEng::ETBLayouts m_eLayout;
        tCIDLib::TInt4          m_i4VertAdjust;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfToolbar,TCQCIntfWidget)
        DefPolyDup(TCQCIntfToolbar)
};


class CQCINTFENGEXPORT TCQCIntfVarToolbar : public TCQCIntfToolbar , public MCQCIntfVarIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVarToolbar();

        TCQCIntfVarToolbar
        (
            const   TCQCIntfVarToolbar&     iwdgSrc
        );

        ~TCQCIntfVarToolbar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfVarToolbar& operator=
        (
            const   TCQCIntfVarToolbar&     iwdgSrc
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
        // -----------------------------------------------------------
        //  Protected, inherited methods
        // -----------------------------------------------------------
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
        RTTIDefs(TCQCIntfVarToolbar, TCQCIntfToolbar)
        DefPolyDup(TCQCIntfVarToolbar)
};

#pragma CIDLIB_POPPACK

