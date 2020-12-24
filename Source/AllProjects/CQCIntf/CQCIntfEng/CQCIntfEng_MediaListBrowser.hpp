//
// FILE NAME: CQCIntfEng_MediaListBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2010
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
//  This is the header for the CQCIntfEng_MediaListBrowser.cpp file, which implements a
//  list based media browser of the sort used by various small music players, with a set
//  of text lists that replace each other as selections are made. Since it only displays
//  text, this one can be pretty light weight and fast.
//
//  When displaying movies or music, a selection at the collection level will invoke an
//  OnColSelect command. Otherwise, in music mode, which allows them to expand out to the
//  item level, a selection will invoke an OnSelect command. This effectively tells the user
//  which type of selection was made implicitly. Of course the media type is provided as an
//  RTV, so they can make more decisions based on that.
//
//  We keep a number of lists, each with 4 slots, one per possible level (artist/category,
//  title sets, collection, and items.) We have to track the current list for each level,
//  the current index at the top of each list (so that we restore it to the previously
//  selected position when we com back up out of more nested lists), and the item selected
//  when the user clicks on a list.
//
//  In music mode, the top level list is artists. For movies it is categories.
//
//  We have special concerns here due to the need to support inertial type scrolling. We
//  have to slide up/down when scrolling around in the currently displayed list, and
//  right/left to drill down or come back up. We have to be able to display items at their
//  actual position, and in a zero origin scenario to build up the image we scroll. And,
//  in the case of going from one list to another we have to draw stuff from from different
//  lists into the image that we scroll. So our item drawing methods all work based on
//  a 'relative to' area.
//
//  Also, since the lists can be far too large for us to build up into memory in total,
//  we have to figure out a subset of the actual content area that will we use for scrolling,
//  which complicates things a lot more.
//
//  We have to keep up with a 'scroll offset' for lists, so that we know how far from their
//  top origin they are scrolled. Since have nested lists and need to be able to move back
//  to previous lists in the stack, we have to keep a list of such offsets, one per list level.
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
//   CLASS: TCQCIntfMediaListBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfMediaListBrowser

    : public TCQCIntfWidget, public MCQCCmdSrcIntf, public MCQCIntfFontIntf
{
    public :
        // -------------------------------------------------------------------
        //  Some helper classes we use to track the data that we display.
        //  We don't need any kind of stack, since it's a fixed structure.
        //  We have a list of artists, a list of title sets, a list of
        //  collections, and a list of items. All we need to do is track
        //  where we are in each list (what index is at the top), and the
        //  bit of info we need for display at each level.
        //
        //  Since there's just a bit of info, we just use one class. So in
        //  some cases, not all the info is used. But there differences
        //  aren't worth having a hierarchy. We know what level we are
        //  displaying anyway and can react accordingly.
        //
        //  We need lists of these things for the four layers of lists that
        //  we display. And we need a small class to track info about each
        //  list.
        //
        //  The list info class can store a cookie and id for the list contents
        //  it contains, i.e. a list of items contains the collection cookie
        //  for the collection that contains those items. And it also contains
        //  current scroll position info. We keep a zero origined content
        //  area, and a scroll adjusted display area relative to that.
        //
        //  And for any but the current top level one, we keep up with the
        //  selection index that got us to the next list.
        // -------------------------------------------------------------------
        class TSlotInfo
        {
            public :
                static tCIDLib::ESortComps eCompI
                (
                    const   TSlotInfo&          si1
                    , const TSlotInfo&          si2
                );

                TSlotInfo();

                TSlotInfo
                (
                    const   TString&            strArtist
                    , const TString&            strCookie
                    , const TString&            strName
                    , const tCIDLib::TCard2     c2Id
                    , const tCIDLib::TCard4     c4ChildCount = 0
                );

                ~TSlotInfo();

                tCIDLib::TBoolean bPressed() const;

                tCIDLib::TBoolean bPressed
                (
                    const   tCIDLib::TBoolean   bToSet
                );

                tCIDLib::TCard2 c2Id() const;

                tCIDLib::TCard4 c4ChildCount() const;

                tCIDLib::TCard4 c4ChildCount
                (
                    const   tCIDLib::TCard4     c4ToSet
                );

                const TString& strArtist() const;

                const TString& strCookie() const;

                const TString& strName() const;


            private :
                // -----------------------------------------------------------
                //  Private data types
                //
                //  m_c2Id
                //      Not used in the case of the top level list for music, since we are
                //      browsing artists at that point, not in the media hierarchy.
                //
                //  m_strCookie
                //      In the case of the top level list for music, this is just set to All
                //      Movies/Music just for consistency so that there's always a category
                //      cookie that the next level can build on.
                //
                //  m_strName
                //      In the case of the top level list for music, this is the same as the
                //      m_strArtist value.
                // -----------------------------------------------------------
                tCIDLib::TBoolean   m_bPressed;
                tCIDLib::TCard2     m_c2Id;
                tCIDLib::TCard4     m_c4ChildCount;
                TString             m_strArtist;
                TString             m_strCookie;
                TString             m_strName;
        };
        using TInfoList = TVector<TSlotInfo>;

        enum class EDispLvls
        {
            Top
            , Title
            , Col
            , Item

            , Count
            , Min = Top
            , Max = Item
        };

        class TListInfo
        {
            public :
                TListInfo
                (
                    const   EDispLvls           eLevel
                    , const TString&            strMoniker
                    , const TString&            strCookie
                );

                ~TListInfo();

                TArea areaContentsRelTo
                (
                    const   TArea&              areaRelTo
                )   const;

                tCIDLib::TCard4 c4Count() const;

                tCIDLib::TVoid Resize
                (
                    const   tCIDLib::TCard4     c4SlotHeight
                    , const TArea&              areaDisplay
                );

                TSlotInfo& siAt
                (
                    const   tCIDLib::TCard4     c4At
                );

                const TSlotInfo& siAt
                (
                    const   tCIDLib::TCard4     c4At
                )   const;

                const TSlotInfo& siSelected() const;

                tCIDLib::TVoid Sort();

                TArea           m_areaAdjDisp;
                TArea           m_areaContent;
                tCIDLib::TCard4 m_c4FocusIndex;
                tCIDLib::TCard4 m_c4SelIndex;
                EDispLvls       m_eLevel;
                TInfoList       m_colList;
                TString         m_strCookie;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfMediaListBrowser();

        TCQCIntfMediaListBrowser
        (
            const   TCQCIntfMediaListBrowser& iwdgSrc
        );

        ~TCQCIntfMediaListBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfMediaListBrowser& operator=
        (
            const   TCQCIntfMediaListBrowser& iwdgSrc
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

        tCIDLib::TVoid PostInit() override;

        virtual tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
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

        TSize szDefaultSize() const override;

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

        tCIDLib::TVoid ScaleInternal
        (
            const   tCIDLib::TFloat8       f8XScale
            , const tCIDLib::TFloat8       f8YScale
        )   override;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-ritual methods
        // -------------------------------------------------------------------
        tCQCMedia::EMediaTypes eInitMediaType() const;

        tCQCMedia::EMediaTypes eInitMediaType
        (
            const   tCQCMedia::EMediaTypes  eToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid Recalc
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoOnClick
        (
            const   tCIDLib::TCard4         c4TarIndex
        );


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TListPtr = TCntPtr<TListInfo>;
        using TLevelsList = TObjArray<TListPtr, EDispLvls>;


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

        tCIDLib::TBoolean bHandleSelection
        (
                    tCIDLib::TCard4&        c4SelIndex
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

        tCIDLib::TCard4 c4FirstVisIndex
        (
            const   TArea&                  areaVis
            , const TArea&                  areaRelTo
        ) const;

        tCIDLib::TCard4 c4HitTest
        (
            const   TPoint&                 pntTest
        )   const;

        TListPtr cptrLoadCols
        (
            const   TSlotInfo&              siSel
        );

        TListPtr cptrLoadItems
        (
            const   TSlotInfo&              siSel
        );

        TListPtr cptrLoadTitles
        (
            const   TSlotInfo&              siSel
            , const tCQCMedia::ESortOrders  eSortOrder
        );

        tCIDLib::TVoid CheckListLevel
        (
            const   EDispLvls               eMinLevel
        )   const;

        tCIDLib::TVoid ClickItem
        (
            const   tCIDLib::TCard4         c4TarInd
        );

        tCIDLib::TVoid DrawItems
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaRelTo
            , const TArea&                  areaInvalid
            , const TPoint&                 pntOffset
            , const TListPtr                cptrList
            , const tCIDLib::TBoolean       bForceAlpha
        );

        tCIDLib::TVoid DrawItemText
        (
                    TGraphDrawDev&          gdevTar
            , const TArea&                  areaTar
            , const TSlotInfo&              siToDraw
            , const tCIDLib::TBoolean       bForceAlpha
            , const tCIDLib::TBoolean       bDoFocus
        );

        tCQCMedia::EMediaTypes eXlatMediaType
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const tCIDLib::TCard4         c4ParmInd
        )   const;

        tCQCMedia::ESortOrders eXlatSortOrder
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const tCIDLib::TCard4         c4ParmInd
        )   const;

        tCIDLib::TVoid GoBack();

        tCIDLib::TVoid ItemInvoked
        (
            const   tCIDLib::TCard4         c4Index
        );

        tCIDLib::TVoid KeepFocusVisible();

        tCIDLib::TVoid LoadTopList();

        tCIDLib::TVoid SetDefaults();

        tCIDLib::TVoid ScrollList
        (
            const   TString&                strScrollOp
        );

        tCIDLib::TVoid SlideList
        (
            const   TArea&                  areaOrg
        );

        tCIDLib::TVoid SlideListLevel
        (
            const   EDispLvls               eOrgLevel
            , const EDispLvls               eNewLevel
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaDisplay
        //      The area within our widget area where we actually display the
        //      content. We move it in for border, any margin we want, a bit
        //      more margin if rounded corners, etc...
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
        //      content area. During pans, we move a copy of the display area
        //      within this area, based on the scroll offset, and that's the
        //      bit to blit out of the memory image.
        //
        //  m_areaPanSoFar
        //      We only initially draw enough content to handle the actual drag
        //      (no inertia.) As inertia starts, we use the gesture idle callback
        //      to incrementally draw more stuff to stay ahead of the movement.
        //
        //  m_ac4SelIndex
        //      When the user selects an item, we put that index into the
        //      correct slot here. This is used particularly when the user
        //      makes a selection and we have to invoke an action. We need
        //      to get the selection index into the runtime values object
        //      later when asked to gen it up (by the action running code.)
        //
        //  m_ai4TopOffset
        //      For each of the lists we have to keep the current top scroll
        //      offset, so that when we come back up to a higher level list
        //      we can go back to the previously selected page of items that
        //      were there before drilling down. So we store the position
        //      when drilling down, and use the positions on the way back
        //      up.
        //
        //  m_bSafePanBgn
        //      When we get a start of gesture, we remember if the bgn is of
        //      a type that is safe to build all into a single bitmap image
        //      and scroll (or if we we need to composite on each round.) So
        //      this is then used later during the move events.
        //
        //  m_c4SlotHeight
        //      The height of the individual display slots. This is based
        //      on the font size plus some margin.
        //
        //  m_cptrMetaData
        //      When we start up, we try to get a ref to the media data for
        //      our associated repo. Until we get that, we can't display any
        //      info. Once we get it, we get all our info from it.
        //
        //  m_eCurMediaType
        //  m_eCurSortOrder
        //      Our currently media type that we are displaying and the current
        //      sort order that was last set by the user. These both start off
        //      set to the configured initial type and order.
        //
        //  m_eDispLevel
        //      The current display level, which tells us which info to
        //      draw from and how to react to selections.
        //
        //  m_eInitMediaType
        //      This is persisted, and is used to set the initial media type
        //      that the browser should start on. If this is not supported
        //      by the repository, then a default is used based on what it
        //      does support.
        //
        //  m_eInitSortOrder
        //      We allow the sort order to be set in the OnPreLoad event, so
        //      that it can be set before the init is done. That way it does
        //      not require a load then a reload as it would if it were done
        //      in OnLoad. So we set the normal default in the ctor and use
        //      that on initial load, and if they override it, we use whatever
        //      they set.
        //
        //  m_eSelectLevel
        //      Because the handling of a click happens after the loading of
        //      any new list that the click invoked, and because if already
        //      at item level no new list is invoked, the DoOnClick method
        //      cannot figure out what actualy happened. So we set this in
        //      the bRelease method, if we figured out we need to invoke
        //      an action (and tell the engine so, and he will call DoOnClick.)
        //      This way, DoOnClick can know exactly which list was clicked.
        //
        //  m_objaData
        //      A list of list info objects, indexed by the levels enum. SO we use
        //      this to track the levels that are pushed.
        //
        //  m_strDBSerialNum
        //      We keep the most recent serial number for the media database
        //      info we get from the media database cache.
        //
        //  m_strRepoMoniker
        //      The moniker of the media repository driver we get cover art
        //      from. This is persisted and used at runtime to send backdoor
        //      type commands to the driver.
        // -------------------------------------------------------------------
        TArea                   m_areaDisplay;
        TArea                   m_areaPanCont;
        TArea                   m_areaPanSoFar;
        tCIDLib::TBoolean       m_bSafePanBgn;
        TBitmap                 m_bmpBgn;
        TBitmap                 m_bmpCont;
        TBitmap                 m_bmpDBuf;
        tCIDLib::TCard4         m_c4SlotHeight;
        TFacCQCMedia::TMDBPtr   m_cptrMetaData;
        tCQCMedia::EMediaTypes  m_eCurMediaType;
        tCQCMedia::ESortOrders  m_eCurSortOrder;
        EDispLvls               m_eDispLevel;
        tCQCMedia::EMediaTypes  m_eInitMediaType;
        tCQCMedia::ESortOrders  m_eInitSortOrder;
        EDispLvls               m_eSelectLevel;
        TLevelsList             m_objaData;
        TString                 m_strMDBSerialNum;
        TString                 m_strRepoMoniker;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfMediaListBrowser,TCQCIntfWidget)
        DefPolyDup(TCQCIntfMediaListBrowser)
};

#pragma CIDLIB_POPPACK

