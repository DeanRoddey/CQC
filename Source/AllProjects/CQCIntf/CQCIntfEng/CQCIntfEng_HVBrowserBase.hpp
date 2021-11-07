//
// FILE NAME: CQCIntfEng_HVBrowserBase.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/08/2007
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
//  This is the header for the CQCIntfEng_HVBrowserBase.cpp file, which
//  implements the core parts of horz/vert browser widgets. There are a
//  number of derivatives of it. There used to be separate base classes for
//  horz and vert browsers but they ended up with 95% the same code so they
//  were consolidated. The user now just selects which orientation they want
//  for any given list. We handle that at our level.
//
//  A browser has no look of its own, it just displays text (and optionally
//  focus/pressed images) in regularly spaced slots, which can be placed over
//  any background. In designer mode, we just display some sample text, and
//  an outline of the area, so that they can see the text appearance and
//  know where the slot areas are.
//
//  The derived calsses only have to provide the list of values to display.
//  We do everything else. So derived classes can provide lists that they
//  get from various sources, and will only require generally very little
//  code above and beyond what they inherit from us.
//
//  The size member means height for verticals and width for horizontals,
//  and the text offset is a horizontal offset for verticals and vice versa.
//  The derived class sets a horz/vert indicator on us so that we know
//  which direction we are going.
//
//  We expose our RTV class because we require the derived classes to
//  handle the RTV class creation and they often need to add more values
//  to the ones we support at this level. They provide the keys that we use
//  for the selected items RT values, which allows them to provide more
//  self-documenting RTV names instead of us just providing generic ones.
//
//  This is a scrollable type, so we have special concerns wrt to drawing
//  of the content. We have to be able to draw into the normal display area
//  for this widget, but we also have to support drawing into a scrollable
//  memory image that can be inertially scrolled into place, which will be
//  larger than the display and zero origined.
//
//  If in remote mode (RIVA) or just doing a regular refresh draw, we just
//  draw the items normally to the display position. Else we do the scroll
//  scenario.
//
//  If we don't have a scroll safe bgn in the direction of travel, we have
//  to be able to capture our background underneath us, for use by the
//  scroller object to composite the transparent text over on each round
//  as it slides the text into place. If we do, then we build the bgn into
//  the scrollable content and use the most efficient scrolling scheme.
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
//   CLASS: TCQCIntfHVBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfHVBrwRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfHVBrwRTV() = delete;

        TCQCIntfHVBrwRTV
        (
            const   TString&                strItemText
            , const tCIDLib::TCard4         c4Index
            , const TString&                strTextKey
            , const TString&                strIndexKey
            , const TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfHVBrwRTV(const TCQCIntfHVBrwRTV&) = delete;

        ~TCQCIntfHVBrwRTV();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfHVBrwRTV& operator=(const TCQCIntfHVBrwRTV&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue
        (
            const   TString&                strId
            ,       TString&                strToFill
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SelIndex() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4SelectIndex
        //      The index of the selected item.
        //
        //  m_strIndexKey
        //      The RTV name that we should give back the index value for.
        //
        //  m_strItemText
        //      The text of the selected item in the list.
        //
        //  m_strTextKey
        //      The RTV name that we should give back the text value for.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4SelectIndex;
        TString         m_strIndexKey;
        TString         m_strItemText;
        TString         m_strTextKey;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfHVBrowserBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfHVBrowserBase

    : public TCQCIntfWidget
    , public MCQCIntfFontIntf
    , public MCQCCmdSrcIntf
    , public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfHVBrowserBase();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

        tCIDLib::TBoolean bCanSizeTo() const override;

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

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
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

        tCIDLib::TVoid GestInertiaIdle
        (
            const   tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
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

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

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
        tCIDLib::TBoolean bHorizontal() const;

        tCIDLib::TBoolean bHorizontal
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNumbered() const;

        tCIDLib::TBoolean bNumbered
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TCard4 c4Size() const;

        tCIDLib::TCard4 c4Size
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Spacing() const;

        tCIDLib::TCard4 c4Spacing
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TInt4 i4TextOfs() const;

        tCIDLib::TInt4 i4TextOfs
        (
            const   tCIDLib::TInt4          i4ToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors and Operators
        // -------------------------------------------------------------------
        TCQCIntfHVBrowserBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTypeName
        );

        TCQCIntfHVBrowserBase
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strTextName
            , const TString&                strTextKey
            , const TString&                strIndexName
            , const TString&                strIndexKey
            , const TString&                strTypeName
        );

        TCQCIntfHVBrowserBase
        (
            const   TCQCIntfHVBrowserBase&  iwdgToCopy
        );

        TCQCIntfHVBrowserBase& operator=
        (
            const   TCQCIntfHVBrowserBase&  iwdgToAssign
        );


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


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TCard4 c4NameCount() const = 0;

        virtual const TString& strDisplayTextAt
        (
            const   tCIDLib::TCard4         c4Index
        )   const = 0;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4FocusIndex() const;

        tCIDLib::TCard4 c4SelectedIndex() const;

        tCIDLib::TVoid RedrawItem
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid ReloadedList();


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

        tCIDLib::TBoolean bDrawImg
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaItem
            , const TString&                strImagePath
        );

        tCIDLib::TBoolean bScrollList
        (
            const   TString&                strScrollOp
            , const tCIDLib::TBoolean       bRedraw
        );

        tCIDLib::TBoolean bScrollToIndex
        (
            const   tCIDLib::TCard4         c4Index
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

        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid DrawItems
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaRelTo
            , const TArea&                  areaInvalid
            , const TPoint&                 pntOffset
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCIDLib::TVoid DrawItem
        (
                    TGraphDrawDev&          gdevTarget
            , const TString&                strText
            , const TArea&                  areaTar
            , const tCIDLib::TCard4         c4Index
            , const tCIDLib::TBoolean       bDoFocus
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCIDLib::TVoid DrawItemText
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaCur
            , const TString&                strText
            , const tCIDLib::TBoolean       bFocus
            , const tCIDLib::TBoolean       bDoAlpha
            , const tCIDLib::TBoolean       bNumPref = kCIDLib::False
        );

        tCIDLib::TVoid DrawNumPref
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaCur
            , const TString&                strText
            , const tCIDLib::TBoolean       bFocus
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCIDLib::TVoid KeepFocusVisible();

        tCIDLib::TVoid MakeScrolledContArea
        (
                    TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid SlideList
        (
            const   tCIDLib::TInt4          i4OrgPos
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaContent
        //      This is the content area. It is always set set at zero origin
        //      and stays there, and it's the size of the list content.
        //
        //  m_areaAdjDisplay
        //      This is the actual scroll position. We move it forward (or
        //      downward) over the content area, and it's the area that we
        //      can blit from the zero based pan content to get the currently
        //      visible info.
        //
        //  m_areaDisplay
        //      The area within our widget area where we actually display the
        //      content. We move it in for border, any margin we want, a bit
        //      more margin if rounded corners, or if the layout isn't as
        //      wide as we are, etc...
        //
        //      It's centered in both directions within our widget area. Take
        //      this and move it right by the scroll offset, and that's the
        //      part of the content area currently visible. Or, vice versa,
        //      move the content area left.
        //
        //  m_areaPanCont
        //      Because the lists can be potentially quite large, we can't just
        //      create the whole thing in memory when doing a pan gesture. This is
        //      unfortunate, because it makes things a lot more annoying. What we
        //      have to do is figure out some range within the content area that
        //      we will render into memory in order to pan around. For small lists
        //      it will be the whole thing, but it's not worth special casing.
        //
        //      This is always some subset of the content area. During pans, we
        //      move the display area (above) within this area.
        //
        //  m_areaPanSoFar
        //      This is a subset of the pan content area above, and represents the
        //      amount of it we have drawn so far. We initially only draw enough
        //      to handle the actual finger drag (no inertia) plus a little extra.
        //      So this starts off representing that much of it. We do a page before
        //      and a page after the displayed contents.
        //
        //      One inertia starts, we use this to draw ahead of the movement,
        //      using the gesture idle callback. We expand this out to remember how
        //      far we've drawn so far.
        //
        //  m_bmpXXX
        //      We need some bitmaps during panning and flicking. The content
        //      one is what we draw the actual contents into. The bgn and dbuf
        //      ones are required if we don't have a bgn fill that is safe
        //      for the direction we are scrolling, to composite the bgn
        //      and the (alpha based) content into the content buffer.
        //
        //  m_bHorizontal
        //      The configured horz/vert setting, which controls what orientation
        //      we display our content.
        //
        //  m_bNumbered
        //      The derived classes can tell us that they want a numbered list, in
        //      which case we'll prefix each line with a 1. type prefix number.
        //
        //  m_bSafePanBgn
        //      We remember during init if the bgn is of a type that is safe to
        //      build all into a single bitmap image and scroll (or if we we need
        //      to composite on each round.) So this is then used later during the
        //      gesture events.
        //
        //  m_c4FocusIndex
        //      The index that has the focus. We have internal focus movement and
        //      need to keep up with where it is. If focus is not being used in the
        //      parent template, then we'll never get the focus and this will never
        //      come into play. it is not persisted. It is an index within the overall
        //      list of items to display.
        //
        //  m_c4PrefWidth
        //      We can be told to do numbered lists. If so, we pre-figure out the
        //      width for a "xxx. " type prefix in our font. This is used during
        //      drawing when we are vertical only. It is faulted in as required,
        //      with zero indicating it's not been set yet.
        //
        //  m_c4PressIndex
        //      This is used only during the Clicked() method to indicate to the hit
        //      item to show itself pressed during the simulated press and release
        //      feedback.
        //
        //  m_c4SelIndex
        //      When we kick off a selection event, which posts commands, this is
        //      set to the index of the item that caused it. This is used by the
        //      runtime value gathering code to know which item they should get the
        //      values from. It's only set long enough for the runtime value object
        //      creation callback to see it, then it is set back to c4MaxCard again.
        //      We have to clear the tracking index so that we can redraw the tracked
        //      item unpressed, so we have to have another place to store this info
        //      until picked up by the RTV object. It's not persisted. This is an
        //      index into the overall list of items to display.
        //
        //  m_c4Size
        //      The height or width of the slots. This plus the spacing is how many
        //      pixels high or wide each one is. The other direction is just as wide
        //      as the content area.
        //
        //  m_c4Spacing
        //      The inter-value spacing between the display areas.
        //
        //  m_i4TextOfs
        //      They can offset the text position (horizontally or vertically
        //      according to the orientation of the browser.)
        //      bitmap.
        // -------------------------------------------------------------------
        TArea               m_areaAdjDisplay;
        TArea               m_areaContent;
        TArea               m_areaDisplay;
        TArea               m_areaPanCont;
        TArea               m_areaPanSoFar;
        TBitmap             m_bmpBgn;
        TBitmap             m_bmpCont;
        TBitmap             m_bmpDBuf;
        tCIDLib::TBoolean   m_bHorizontal;
        tCIDLib::TBoolean   m_bNumbered;
        tCIDLib::TBoolean   m_bSafePanBgn;
        tCIDLib::TCard4     m_c4FocusIndex;
        tCIDLib::TCard4     m_c4PrefWidth;
        tCIDLib::TCard4     m_c4PressIndex;
        tCIDLib::TCard4     m_c4SelIndex;
        tCIDLib::TCard4     m_c4Size;
        tCIDLib::TCard4     m_c4Spacing;
        tCIDLib::TInt4      m_i4TextOfs;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfHVBrowserBase,TCQCIntfWidget)
};


#pragma CIDLIB_POPPACK



