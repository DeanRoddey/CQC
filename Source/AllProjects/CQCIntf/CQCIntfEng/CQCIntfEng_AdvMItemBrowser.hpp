//
// FILE NAME: CQCIntfEng_AdvMItemBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/21/2012
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
//  This is the header for the CQCIntfEng_AdcMItemBrowser.cpp file, which
//  is an advanced version of a media item browser (which is used to monitor
//  the contents of a media renderer's current playlist.) There is simple
//  version of this which is based on the standard horz/vert list browser
//  class. It just displays the name of the items.
//
//  This one allows the user to design the look of the items in the list by
//  using template mechanism. They can use static text and static image
//  widgets to define the layout of the items. They can give specific names
//  to the widgets to have them display item information. This makes for a
//  a very flexible browsing scheme.
//
//  We will discard anything from the loaded template that is not acceptable
//  for our purposes. We don't actually use the widgets either. It would be
//  too kludgey to try to have them actually do their own drawing, ecause they
//  are not real widgets and aren't children of the container and so forth.
//  We just grab their attributes (colors, text settings, transparency and
//  opacity, drawing areas and so forth, and use that to do our own drawing.
//
//  Since we may use this in other widget types in the future, we have a base
//  advanced browser class that provides some functionality that any such
//  type of widget would need.
//
// CAVEATS/GOTCHAS:
//
//  1)  To avoid redundancy we share the basic media item browser's runtime
//      values class, sine it serves our purposes well enough.
//
//  2)  We have to implement our own list scrolling and such here since we
//      don't derive from the standard list classes. At some point it would
//      probably be nice to factor out the abstract aspects of lists and
//      derive all the lists from that. But, for now, we don't have that. We
//      do have a basic h/v browser class but it assumes simple text display.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfMediaRepoText;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAdvMItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfAdvMItemBrowser

    : public TCQCIntfWidget, public MCQCIntfMultiFldIntf, public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfAdvMItemBrowser();

        TCQCIntfAdvMItemBrowser
        (
            const   TCQCIntfAdvMItemBrowser& iwdgToCopy
        );

        ~TCQCIntfAdvMItemBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfAdvMItemBrowser& operator=
        (
            const   TCQCIntfAdvMItemBrowser& iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             fldTest
            , const tCIDLib::TCard4         c4FldInd
            , const TString&                strMake
            , const TString&                strModel
            , const tCIDLib::TBoolean       bStore
        )   override;

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

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
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

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colCmds
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;

        tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        )   override;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const override;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const override;

        tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        )   override;

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
        tCIDLib::TBoolean bIsValidTemplate
        (
            const   TCQCIntfTemplate&       iwdgTest
            ,       TString&                strReason
        )   const;

        tCIDLib::TBoolean bSetMoniker
        (
            const   TString&                strToSet
            ,       TString&                strErrReason
        );

        const TString& strMoniker() const;

        const TString& strTemplate() const;

        const TString& strTemplate
        (
            const   TString&                strToSet
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

        tCIDLib::TVoid FieldChanged
        (
                    TCQCFldPollInfo&        cfpiAssoc
            , const tCIDLib::TCard4         c4FldIndex
            , const tCIDLib::TBoolean       bNoRedraw
            , const TStdVarsTar&            ctarGlobalVars
        )   override;

        tCIDLib::TVoid GotFocus() override;

        tCIDLib::TVoid LostFocus() override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        );

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

        TBitmap bmpGatherBgn();

        tCIDLib::TBoolean bLoadItems();

        tCIDLib::TBoolean bScrollList
        (
            const   TString&                strScrollOp
            , const tCIDLib::TBoolean       bCanRedraw
        );

        tCIDLib::TBoolean bVisibleEnough
        (
                    tCIDLib::TCard4&        c4Index
            , const tCIDLib::TFloat4        f4PerLimit
        )   const;

        tCIDLib::TCard4 c4ClosestVisIndex
        (
            const   tCIDLib::TCard4         c4FromHere
        )   const;

        tCIDLib::TCard4 c4FirstVisIndex() const;

        tCIDLib::TCard4 c4FirstVisIndex
        (
            const   TArea&                  areaVis
            , const TArea&                  areaRelTo
        )   const;

        tCIDLib::TCard4 c4HitTest
        (
            const   TPoint&                 pntTest
        )   const;

        tCIDLib::TVoid CalcAreas
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid DrawRepoText
        (
                    TGraphDrawDev&          gdevTarget
            ,       TCQCIntfMediaRepoText&  iwdgDraw
            , const tCIDLib::TCard4         c4Index
            ,       TArea&                  areaDraw
            , const tCIDLib::TBoolean       bDoAlpha
            ,       TBitmap&                bmpBuf
            ,       TBitmap&                bmpMask
        );

        tCIDLib::TVoid DrawItem
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaTar
            , const TArea&                  areaInvalid
            , const tCIDLib::TCard4         c4ItemInd
            , const tCIDLib::TBoolean       bDoAlpha
            ,       TBitmap&                bmpBuf
            ,       TBitmap&                bmpMask
            , const tCIDLib::TBoolean       bDoFocus
        );

        tCIDLib::TVoid DrawItems
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaRelTo
            , const TArea&                  areaInvalid
            , const TPoint&                 pntOffset
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TCQCIntfWidget&         iwdgDraw
            , const MCQCIntfFontIntf&       miwdgFont
            , const TString&                strText
            ,       TArea&                  areaDraw
            , const tCIDLib::TBoolean       bDoAlpha
            ,       TBitmap&                bmpBuf
            ,       TBitmap&                bmpMask
        );

        tCIDLib::TVoid KeepFocusVisible();

        tCIDLib::TVoid LoadTemplate
        (
                    TDataSrvClient&         dsclLoad
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        tCIDLib::TVoid MakeScrolledContArea
        (
                    TArea&                  areaToFill
        )   const;

        tCIDLib::TVoid SlideList
        (
            const   TArea&                  areaOrg
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
        //      This is our display area, moved downwards across the content
        //      area (so relative to the zero based content) the amount that
        //      we are actually scrolled. So this is where we should pull
        //      stuff from the content to display. We move it as the user
        //      drags or scrolls us.
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
        //      Because the lists can be potentially quite large, we can't
        //      just create the whole thing in memory when doing a pan
        //      gesture. This is unfortunate, because it makes things a lot
        //      more annoying. What we have to do is figure out some range
        //      within the content area that we will render into memory in
        //      order to pan around. For small lists it will be the whole
        //      thing, but it's not worth special casing for that.
        //
        //      This is relative to the content area, and can never go below
        //      its origin or beyond its end. It's always some subset of the
        //      content area. During pans, we keep the display area within
        //      this area.
        //
        //  m_areaPanSoFar
        //      We draw incrementally during pans. Initially this is set to the size
        //      of what we first draw into the bitmap, which is just enough to
        //      handle the actual drag bit. After that, during inertial moves,
        //      we use the idle callback to incrementally draw data ahead of the
        //      inertial movement. We update this to keep up with how much we've
        //      drawn.
        //
        //  m_bHasFocusInd
        //      When we load the layout, we remember if we have some kinds
        //      of widgets. This lets us be more efficient while drawing.
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
        //  m_c4FocusIndex
        //      The item that has the focus. We have internal focus movement
        //      and need to keep up with where it is. If focus is not being
        //      used in the parent template, then we'll never get the focus
        //      and this will never come into play. it is not persisted.
        //
        //  m_c4LastCurItem
        //      We have to keep up with the last item that was reported as
        //      being the current playlist item, so that we can unmark it
        //      when the associated field changes. And of course so that we
        //      can correctly redraw with the active item marked.
        //
        //  m_c4SelIndex
        //      If we get clicked, we have to store the selected item
        //      temporarily while that is going on, so that the later call
        //      back to us to gen up a runtime value object will now which
        //      item it's dealing with.
        //
        //  m_colItems
        //      We get a list of playlist items, basically a copy of the
        //      player's playlist. This is obviously not persisted. We use
        //      the bPlayed() value as a 'pressed' marker, since it's not
        //      something we display.
        //
        //  m_iwdgLayout
        //      This is the widget that we were told to use as the layout
        //      guide. We will strip anything out of it that is not valid for
        //      our purposes. This is runtime only, used for drawing purposes.
        //
        //  m_strMoniker
        //      We use the multi-field mixin, but we don't expose it directly
        //      in terms of widget configuration. Instead we just accept a
        //      moniker for a media renderer. We then set up the field mixin
        //      for the two appropriate fields.
        //
        //  m_strTemplate
        //      The configured template that we are to use as the layout
        //      guide. This is persisted.
        //
        //  m_strTmp1
        //  m_strTmp2
        //      A couple strings to use during the drawing process.
        // -------------------------------------------------------------------
        TArea                       m_areaContent;
        TArea                       m_areaAdjDisplay;
        TArea                       m_areaDisplay;
        TArea                       m_areaPanCont;
        TArea                       m_areaPanSoFar;
        TBitmap                     m_bmpBgn;
        TBitmap                     m_bmpCont;
        TBitmap                     m_bmpDBuf;
        tCIDLib::TBoolean           m_bHasFocusInd;
        tCIDLib::TBoolean           m_bSafePanBgn;
        tCIDLib::TCard4             m_c4FocusIndex;
        tCIDLib::TCard4             m_c4LastCurItem;
        tCIDLib::TCard4             m_c4SelIndex;
        TVector<TCQCMediaPLItem>    m_colItems;
        tCIDLib::TInt4              m_i4ScrollOfs;
        TCQCIntfTemplate            m_iwdgLayout;
        TString                     m_strMoniker;
        TString                     m_strTemplate;
        TString                     m_strTmp1;
        TString                     m_strTmp2;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfAdvMItemBrowser,TCQCIntfWidget)
        DefPolyDup(TCQCIntfAdvMItemBrowser)
};

#pragma CIDLIB_POPPACK



