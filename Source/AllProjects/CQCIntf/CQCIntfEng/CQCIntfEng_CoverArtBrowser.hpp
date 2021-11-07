//
// FILE NAME: CQCIntfEng_CoverArtBrowser.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/02/2005
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
//  This is the header for the CQCIntfEng_CvrArtBrowser.cpp file, which
//  implements a scrollable text and/or graphics browser for media repo data.
//
//  It has no look of its own, it just displays images in regularly spaced
//  slots, which can be placed over any background. In designer mode, we just
//  display outlines of the slots.
//
//  Each slot can display image only, text and image, or just text. Since this
//  is a scrollable widget, we have special issues in that we have to be able
//  to display the items in their normal position, or to draw them into a
//  scrollable memory image for inertial dragging.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfMediaRepoText;


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCvrArtBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfCvrArtBrowser

    : public TCQCIntfWidget
    , public MCQCCmdSrcIntf
    , public MCQCIntfFontIntf
    , public MCQCIntfImgListIntf
{
    public :
        // -------------------------------------------------------------------
        //  The info we need to keep track of for each item in the list. It's
        //  either a top level one (in which the title set point is set), or
        //  it's a collection level one (in which both title and collection
        //  pointers are set.)
        // -------------------------------------------------------------------
        class CQCINTFENGEXPORT TListItem
        {
            public :
                // -----------------------------------------------------------
                //  Public, static methods
                // -----------------------------------------------------------
                static tCIDLib::ESortComps eCompByArtist
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByBitDepth
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByBitRate
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByChannels
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByDateAdded
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompById
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompBySampleRate
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompBySeqNum
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByTitle
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompBySortTitle
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByUserRating
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );

                static tCIDLib::ESortComps eCompByYear
                (
                    const   TListItem&              li1
                    , const TListItem&              li2
                );


                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TListItem() = delete;

                TListItem
                (
                    const   TMediaTitleSet* const   pmtsThis
                    , const TString&                strCookie
                );

                TListItem
                (
                    const   TMediaTitleSet* const   pmtsThis
                    , const TMediaCollect* const    pmcolThis
                    , const TString&                strCookie
                );

                TListItem(const TListItem&) = delete;

                ~TListItem();


                // -----------------------------------------------------------
                //  Public operators
                // -----------------------------------------------------------
                TListItem& operator=(const TListItem&) = delete;


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                tCIDLib::TBoolean bIsTopLevel() const;


                // -----------------------------------------------------------
                //  Public data members
                //
                //  m_bImgLoaded
                //      Used to remember whether we successfully loaded an image for
                //      this one or not.
                //
                //  m_bPressed
                //      Just used during a Clicked() callback, to tell this one to
                //      show itself pressed or not, so that it can set this, redraw,
                //      pause, clear this and redraw.
                //
                //  m_bmpImg
                //      The image for this item. Which will be scaled to the
                //      m_areaImage size when we load it, so it's already
                //      the right size. It's actually referring to a bitmap
                //      in our own little cache that we use. Bitmaps ref cnt
                //      their image contents, so this lets us insure that any
                //      we have already loaded stay alive, even if they have
                //      been pushed out of the cache.
                //
                //  m_pmcolThis
                //  m_pmtsThis
                //      The title set or collection this item represents. Only one
                //      will be set, the other will be null. These cannot go away
                //      while we are alive, since they are from the media DB cache
                //      (from CQCMedia) and we get a ref to the DB for our repo. As
                //      long as we hold onto that, these are good.
                //
                //  m_strCookie
                //      The title or collection cookie that this list item
                //      represents.
                //
                //  m_szOrgImg
                //      When we do drawing in remote mode, we have to use the original
                //      image size, because that's what the RIVA client will have.
                //      It doesn't get our cached and pre-scaled image.
                // -----------------------------------------------------------
                tCIDLib::TBoolean       m_bImgLoaded;
                tCIDLib::TBoolean       m_bPressed;
                TBitmap                 m_bmpImg;
                const TMediaCollect*    m_pmcolThis;
                const TMediaTitleSet*   m_pmtsThis;
                TString                 m_strCookie;
                TSize                   m_szOrgImg;
        };


        // -------------------------------------------------------------------
        //  We maintain a stack of lists. Each list displays some set of
        //  title sets, or the collections. We have an enum to indicate the
        //  list type, so we know what the items in the list represent.
        // -------------------------------------------------------------------
        enum class EListTypes
        {
            Cat
            , TitleSearch
            , TitleCols
            , Random
        };

        class CQCINTFENGEXPORT TItemList
        {
            public :
                // -----------------------------------------------------------
                //  Constructors and Destructor
                // -----------------------------------------------------------
                TItemList
                (
                    const   TString&                strContentDescr
                    , const tCIDLib::TCard4         c4ParentSel
                    , const tCQCMedia::EMediaTypes  eMediaType
                    , const EListTypes              eListType
                    , const tCQCMedia::ESortOrders  eSortOrder
                    , const TString&                strCookie
                    , const TMediaTitleSet* const   pmtsCont
                );

                ~TItemList();


                // -----------------------------------------------------------
                //  Public, non-virtual methods
                // -----------------------------------------------------------
                TArea areaAdjusted
                (
                    const   TArea&                  areaRelTo
                )   const;

                tCIDLib::TBoolean bIsTopLevel() const;

                tCIDLib::TBoolean bResort
                (
                    const   tCQCMedia::ESortOrders  eNewOrder
                );

                tCIDLib::TBoolean bScrollToCol
                (
                    const   tCIDLib::TCard4         c4ColNum
                    , const tCIDLib::TCard4         c4ColWidth
                    , const tCIDLib::TCard4         c4Spacing
                    , const tCIDLib::TCard4         c4RowCnt
                );

                tCIDLib::TCard4 c4FirstCharColumn
                (
                    const   tCIDLib::TCh            chFind
                    , const tCIDLib::TCard4         c4RowCount
                )   const;

                tCIDLib::TVoid FinishSetup
                (
                    const   tCIDLib::TCard4         c4RowsHeight
                    , const tCIDLib::TCard4         c4RowCount
                    , const tCIDLib::TCard4         c4HSpacing
                    , const tCQCMedia::ESortOrders  eInitOrder
                    , const TArea&                  areaLayout
                    , const TArea&                  areaDisplay
                );

                tCIDLib::TVoid QueryVisColumns
                (
                    const   TArea&                  areaLayout
                    , const tCIDLib::TCard4         c4Spacing
                    ,       tCIDLib::TCard4&        c4FirstCol
                    ,       tCIDLib::TCard4&        c4LastCol
                    , const TArea&                  areaVisTest
                )   const;

                tCIDLib::TVoid QueryVisIndices
                (
                    const   TArea&                  areaLayout
                    , const tCIDLib::TCard4         c4Spacing
                    , const tCIDLib::TCard4         c4RowCnt
                    ,       tCIDLib::TCard4&        c4FirstInd
                    ,       tCIDLib::TCard4&        c4LastInd
                    , const TArea&                  areaVisTest
                )   const;

                tCIDLib::TVoid Resize
                (
                    const   tCIDLib::TCard4         c4RowsHeight
                    , const tCIDLib::TCard4         c4RowCount
                    , const tCIDLib::TCard4         c4HSpacing
                    , const TArea&                  areaLayout
                    , const TArea&                  areaDisplay
                );


                // -----------------------------------------------------------
                //  Public data members
                //
                //  m_areaContent
                //      This is the calculated area for our whole list. It is used
                //      during scrolling to know how far we can go. Here it is zero
                //      based, so that we can return a scroll adjusted area relative
                //      to whatever the caller wants.
                //
                //  m_areaAdjDisp
                //      This is where we track how far this list is actually
                //      scrolled. This is also zero based, the same size as
                //      the CAB's display area (or possibly smaller if we
                //      don't have enough to fill a whole one) and we move
                //      it around over the content area. The x positoin of
                //      this guy is how far we are scrolled. It can't go
                //      out of the content area.
                //
                //  m_c4ColumnCnt
                //      The number of colums that this list has in total, given the
                //      number of rows the browser is set up for.
                //
                //  m_c4FocusIndex
                //      When a new list is pushed, we safe the focus index of the
                //      current one so that it can be restored later if the list is
                //      popped.
                //
                //  m_c4NextArtIndex
                //      The CAB uses the active update callback to incrementally
                //      load cover art in the background, in order to try to stay
                //      head of the user. They are just local files, so it's a light
                //      operation and it ony does one at a time. It uses this index
                //      to move incrementally through them. Upon initially load of
                //      the first batch of art, the CAB will set this to where it
                //      left off. Then the active update will just keep running it
                //      up.
                //
                //  m_c4ParentSel
                //      If we got here by a selection in the parent, typically only
                //      if m_bTitle is false, i.e. we are showing cols from a selected
                //      title, then this is the index within the previous list that was
                //      selected.
                //
                //  m_colItems
                //      Our list of list items, either title sets or collects depending
                //      on the level we are at.
                //
                //  m_eListType
                //      Indicates what our list contains.
                //
                //  m_eMediaType
                //      The media type of the content being displayed in this
                //      list.
                //
                //  m_eSortOrder
                //      Our current sort order, so we can know if we need to re-sort if
                //      the sort order is changed by the user.
                //
                //  m_pmtsCont
                //      For a collection list, this is the title whose collections are
                //      being displayed. Else its zero.
                //
                //  m_strCookie
                //      The cookie represented by this list:
                //
                //          Cat       - Category cookie
                //          Search    - Not used
                //          Collects  - Title cookie
                //
                //  m_strDescr
                //      Indicates what our contents represents. So it'll be the name
                //      of a category, a kind of search that was done, etc... It's
                //      provided as an RTV so the user can display it if desired.
                // -----------------------------------------------------------
                TArea                   m_areaAdjDisp;
                TArea                   m_areaContent;
                tCIDLib::TCard4         m_c4ColumnCnt;
                tCIDLib::TCard4         m_c4FocusIndex;
                tCIDLib::TCard4         m_c4NextArtIndex;
                tCIDLib::TCard4         m_c4ParentSel;
                TRefVector<TListItem>   m_colItems;
                EListTypes              m_eListType;
                tCQCMedia::EMediaTypes  m_eMediaType;
                tCQCMedia::ESortOrders  m_eSortOrder;
                const TMediaTitleSet*   m_pmtsCont;
                TString                 m_strCookie;
                TString                 m_strDescr;
        };

        using TListStack = TRefStack<TItemList>;



        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfCvrArtBrowser();

        TCQCIntfCvrArtBrowser
        (
            const   TCQCIntfCvrArtBrowser&  iwdgToCopy
        );

        ~TCQCIntfCvrArtBrowser();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfCvrArtBrowser& operator=
        (
            const   TCQCIntfCvrArtBrowser&  iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanSizeTo() const final;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const final;

        tCIDLib::TBoolean bMoveFocus
        (
            const   tCQCKit::EScrNavOps     eOp
        )   final;

        tCIDLib::TBoolean bPerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        )   final;

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        )   final;

        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        )   final;

        tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   final;

        tCQCKit::ECmdRes eDoCmd
        (
            const   TCQCCmdCfg&             ccfgToDo
            , const tCIDLib::TCard4         c4Index
            , const TString&                strUserId
            , const TString&                strEventId
            ,       TStdVarsTar&            ctarGlobals
            ,       tCIDLib::TBoolean&      bResult
            ,       TCQCActEngine&          acteTar
        )   final;

        tCIDLib::TVoid GestInertiaIdle
        (
            const   tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
        )   final;

        tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        )   final;

        tCIDLib::TVoid Invoke() final;

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const final;

        tCIDLib::TVoid PostInit() final;

        tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        )   final;

        tCIDLib::TVoid QueryCmds
        (
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const final;

        tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const final;

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const final;

        tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const final;

        tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        )   final;

        TSize szDefaultSize() const final;

        tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        )   final;

        tCIDLib::TVoid SetDefParms
        (
                    TCQCCmdCfg&             ccfgToSet
        )   const final;

        tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsValidTemplate
        (
            const   TCQCIntfTemplate&       iwdgTest
            ,       TString&                strReason
        )   const;

        tCIDLib::TCard4 c4HSpacing() const;

        tCIDLib::TCard4 c4HSpacing
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4VSpacing() const;

        tCIDLib::TCard4 c4VSpacing
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCQCMedia::EMediaTypes eInitMediaType() const;

        tCQCMedia::EMediaTypes eInitMediaType
        (
            const   tCQCMedia::EMediaTypes  eToSet
        );

        const TString& strLayoutTmpl() const;

        const TString& strLayoutTmpl
        (
            const   TString&                strToSet
        );

        const TString& strRepoMoniker() const;

        const TString& strRepoMoniker
        (
            const   TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActiveUpdate
        (
            const   tCIDLib::TBoolean       bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        )   final;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   final;

        tCIDLib::TVoid GotFocus() final;

        tCIDLib::TVoid LostFocus() final;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        )   final;

        tCIDLib::TVoid StyleFlagChanged
        (
            const   tCIDLib::TCard4         c4OldFlags
            , const tCIDLib::TCard4         c4NewFlags
        )   final;

        tCIDLib::TVoid StreamFrom
        (
                    TBinInStream&           strmToReadFrom
        )   final;

        tCIDLib::TVoid StreamTo
        (
                    TBinOutStream&          strmToWriteTo
        )   const final;

        tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaForIndex
        (
            const   TArea&                  areaRelTo
            , const tCIDLib::TCard4         c4Index
            ,       TArea&                  areaToFill
        );

        TBitmap bmpGatherBgn();

        tCIDLib::TBoolean bPushActorSearch
        (
            const   TString&                strActorName
            , const tCIDLib::TBoolean       bRegEx
            , const tCQCMedia::ESortOrders  eSortOrder
        );

        tCIDLib::TBoolean bPushArtistSearch
        (
            const   TString&                strArtist
            , const tCIDLib::TBoolean       bRegEx
            , const tCQCMedia::ESortOrders  eSortOrder
            , const tCQCMedia::EMediaTypes  eMType
        );

        tCIDLib::TBoolean bPushAudioFmtSearch
        (
            const   tCQCMedia::ECompTypes   eCompType
            , const tCIDLib::TCard4         c4BitDepth
            , const tCIDLib::TCard4         c4SampleRate
            , const tCQCMedia::ESortOrders  eSortOrder
        );

        tCIDLib::TBoolean bPushRandomTitleSearch
        (
            const   tCQCMedia::ESortOrders  eSortOrder
            , const tCQCMedia::EMediaTypes  eMType
            , const tCIDLib::TCard4         c4MaxRet
        );

        tCIDLib::TBoolean bPushTitleSearch
        (
            const   TString&                strMatch
            , const tCIDLib::TBoolean       bRegEx
            , const tCQCMedia::ESortOrders  eSortOrder
            , const tCQCMedia::EMediaTypes  eMType
        );

        tCIDLib::TCard4 c4FirstVisColumn
        (
            const   TArea&                  areaVis
            , const TArea&                  areaRelTo
            ,       tCIDLib::TCard4&        c4Index
        ) const;

        tCIDLib::TCard4 c4HitTest
        (
            const   TPoint&                 pntTest
            ,       tCIDLib::TCard4&        c4Row
            ,       tCIDLib::TCard4&        c4Col
        )   const;

        tCIDLib::TVoid DoSizeCalcs
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid DrawListItem
        (
                    TGraphDrawDev&          gdevTarget
            ,       TListItem&              liDraw
            , const TArea&                  areaAt
            , const TArea&                  areaInvalid
            , const tCIDLib::TBoolean       bAlpha
            , const tCIDLib::TBoolean       bDoFocus
        );

        tCIDLib::TVoid DrawList
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaRelTo
            , const TArea&                  areaInvalid
            , const TPoint&                 pntAdjust
            ,       TItemList&              ilDraw
            , const tCIDLib::TBoolean       bAlpha
        );

        tCIDLib::TVoid DrawRepoText
        (
                    TGraphDrawDev&          gdevTarget
            , const TCQCIntfMediaRepoText&  iwdgDraw
            , const TListItem&              liData
            , const tCIDLib::TCard4         c4Index
            ,       TArea&                  areaDraw
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCIDLib::TVoid DrawText
        (
                    TGraphDrawDev&          gdevTarget
            , const TCQCIntfWidget&         iwdgSrc
            , const MCQCIntfFontIntf&       miwdgFont
            , const TString&                strText
            ,       TArea&                  areaTar
            , const tCIDLib::TBoolean       bDoAlpha
        );

        tCQCMedia::ECompTypes eXlatCompType
        (
            const   TCQCCmdCfg&             ccfgSrc
            , const tCIDLib::TCard4         c4ParmInd
        )   const;

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

        tCIDLib::TVoid IndexToRowCol
        (
            const   tCIDLib::TCard4         c4Index
            ,       tCIDLib::TCard4&        c4Row
            ,       tCIDLib::TCard4&        c4Col
        )   const;

        tCIDLib::TVoid KeepFocusVisible();

        tCIDLib::TVoid LoadCurPageArt();

        tCIDLib::TVoid LoadDefCategory();

        tCIDLib::TVoid LoadItemArt
        (
            const   TGraphDrawDev&          gdevCompat
            ,       TListItem&              liCur
            ,       TString&                strKey
        );

        tCIDLib::TVoid LoadListArt
        (
            const   TGraphDrawDev&          gdevCompat
            ,       TItemList&              ilTar
            , const tCIDLib::TCard4         c4StartInd
            , const tCIDLib::TCard4         c4EndInd
        );

        tCIDLib::TVoid LoadTemplate
        (
                    TDataSrvClient&         dsclToUse
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        tCIDLib::TVoid PushCatList
        (
            const   TString&                strCatCookie
        );

        tCIDLib::TVoid PushColList
        (
            const   tCIDLib::TCard4         c4SelIndex
            ,       TListItem&              liSel
        );

        tCIDLib::TVoid PushPopStack
        (
                    TItemList* const        pilToPush = 0
            , const tCQCMedia::ESortOrders  eSortOrder = tCQCMedia::ESortOrders::Count
        );

        tCIDLib::TVoid ResortTop();

        tCIDLib::TVoid ScrollList
        (
            const   TString&                strScrollOp
        );

        tCIDLib::TVoid SlideList
        (
                    TItemList&              ilTop
            , const TArea&                  areaOrg
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaDisplay
        //      For convenience, we set up this area, which is the height of
        //      the rows we can display, centered vertically within us, and
        //      our width. The slot info always goes within here.
        //
        //  m_areaFocusInd
        //      Our layout template can have a focus indicator in it. When we
        //      move the focus around, we want to not move to a partially
        //      displayed item if its focus indicator isn't visible. So we
        //      store the area (relative to the layout's zero origin) of the
        //      indicator if any, when loading a layout. We use this later
        //      in selecting new focus targets. If we don't have one, we set
        //      this to an empty area.
        //
        //  m_areaPanCont
        //      We need to keep up with the current scrolling position of the
        //      content during drags. We set this up during the gesture start
        //      and update it during the moves.
        //
        //  m_areaPanSoFar
        //      This is only used once we get into inertial mode. It is initially
        //      set to the first chunk of content we draw into the pan content area
        //      just enough to handle dragging plus a little spare. Once we start
        //      inertia, we just keep expanding it as we draw more.
        //
        //  m_bmpArtMissing
        //  m_bmpArtPlayList
        //      If the user defined missing and playlist art images, we presize
        //      them to our needs, for speed. They will only be used if the repo
        //      does provide playlist art or titles don't have art.
        //
        //  m_bmpXXX
        //      We need some bitmaps during panning and flicking. The content
        //      one is what we draw the actual contents into. The bgn and dbuf
        //      ones are required if we don't have a bgn fill that is safe
        //      for the direction we are scrolling, to composite the bgn
        //      and the (alpha based) content into the content buffer.
        //
        //  m_bmpTBuf
        //  m_bmpTMask
        //      These are passed to the alpha and f/x text drawing methods for them
        //      to be used as temps. These only need to be the size of the layout,
        //      so they are allocated when the layout is loaded or reloaded. This
        //      also means that they are not affected when our size changes, so it's
        //      pretty safe to pre-allocate them like this.
        //
        //  m_bHavePressedInd
        //      When loading the layout template, we remember if we have a pressed
        //      indicator, for later use during drawing.
        //
        //  m_bLargeArt
        //      To avoid having to dig into the layout template every time we
        //      need to load art, we remember this when we are loading the
        //      layout, for later use.
        //
        //  m_bNewLayout
        //      We allow the layout template to be changed via command. This just
        //      updates the m_strLayoutTmpl field, and sets this flag. The next time
        //      we need to load a new list, we'll download the new layout and use
        //      that.
        //
        //  m_bSafePanBgn
        //      We save this in the drag gesture start, so that we know
        //      how to react during the moves. If safe, then we can do the
        //      more efficient scroll operation, where the background is in
        //      the scrollable content. Else we have to composite the bgn
        //      and content together.
        //
        //  m_c4ColCount
        //      This is how many columns would fit across our width. This is
        //      not exact since it can be partial, but we need this in some
        //      calculations. This will include any partial one.
        //
        //  m_c4FocusIndex
        //      If focus is not suppressed, then we have to be able to move
        //      the focus through our rows and columns. This is the index
        //      within the current list of the item with the focus. It's
        //      initially set to zero, and adjusted as needed to keep it
        //      legal for the current list.
        //
        //  m_c4HSpacing
        //      The horizontal inter-value spacing between the display areas.
        //      This is persisted.
        //
        //  m_c4RowCount
        //      For runtime use. We calculate how many rows we can display
        //      given our pattern template's height.
        //
        //  m_c4RowsHeight
        //      We figure out how many rows we can display, and then that
        //      area is centered vertically within our area. So we pre-calc
        //      the height of the rows we are displaying, when our size
        //      changes. This is runtimeonly.
        //
        //  m_c4SelIndex
        //      When we kick off a selection event, which posts commands, this
        //      is set to the index of the item that caused it. This is used
        //      by the runtime value gathering code to know which item they
        //      should get the values from. It's only set long enough for the
        //      runtime value object creation callback to see it, then it is
        //      set back to c4MaxCard again. We have to clear the tracking
        //      index so that we can redraw the tracked item unpressed, so
        //      we have to have another place to store this info until
        //      picked up by the RTV object.
        //
        //  m_c4VSpacing
        //      The horizontal inter-value spacing between the display areas.
        //      This is persisted.
        //
        //  m_c4UpdateCnt
        //      We use the active update callback to load art ahead of time. We
        //      don't want to get crazy, so we only load a new image every couple
        //      times through.
        //
        //  m_colStack
        //      This is our list stack. It starts with a default category
        //      list (default cat for our associated repo moniker.) After
        //      that new searches or new categories or drill downs into the
        //      collections under a title set are pushed onto the stack. We
        //      support a Back command that will pop back to the previous
        //      level and display that stack again.
        //
        //      If the repo is changed at viewing time via a command, this
        //      stack is cleared and we start over again.
        //
        //  m_cptrInfo
        //      We store the media database for our current repository here.
        //      As long as we keep this around, the database won't go away.
        //
        //  m_eInitMediaType
        //      This is persisted, and is used to set the initial media type
        //      that the browser should start on. If this is not supported
        //      by the repository, then a default is used based on what it
        //      does support.
        //
        //  m_eSortOrder
        //      This is the order that will be used for any new top level
        //      lists that are created. It defaults to sort by title (name)
        //      but can be changed via a command, which will both resort
        //      the current list and save that order here for future lists.
        //
        //  m_imgLoadArt
        //      We need a bitmap image object for loading art. This is too heavy
        //      to want to create/destroy at the rate we need to do it.
        //
        //  m_iwdgLayout
        //      A template that we load to provide the pattern for each
        //      title or collection we are displaying.
        //
        //  m_strInitCat
        //      Normally we figure out an initial deafult category to show.
        //      But they can call our set category method in the on preload,
        //      so we just store it away and use that instead when we finally
        //      get ready to load the initial list. It's not persisted.
        //
        //  m_strLayoutTmpl
        //      This is the pattern template we use. This is used to load
        //      the pattern template when we initialize.
        //
        //  m_strRepoMoniker
        //      The moniker of the media repository driver we get cover art
        //      from. This is persisted and used at runtime to lookup media
        //      data as required. This can be changed at viewing time via a
        //      cmd as well.
        //
        //  m_strTmp1
        //      Temp string for formatting during drawing
        //
        //  m_szArt
        //      Each time we load a layout template, this is set to the size of
        //      the cover art widget. If none, it's zeroed, else it's the size of
        //      the art, since we need that in a few places.
        //
        //  m_tmAddedFmt
        //      We set this guy up appropriately to format the date added
        //      stamps for display.
        // -------------------------------------------------------------------
        TArea                       m_areaDisplay;
        TArea                       m_areaFocusInd;
        TArea                       m_areaPanCont;
        TArea                       m_areaPanSoFar;
        TBitmap                     m_bmpArtMissing;
        TBitmap                     m_bmpArtPlayList;
        TBitmap                     m_bmpBgn;
        TBitmap                     m_bmpCont;
        TBitmap                     m_bmpDBuf;
        tCIDLib::TBoolean           m_bHavePressedInd;
        tCIDLib::TBoolean           m_bLargeArt;
        tCIDLib::TBoolean           m_bNewLayout;
        tCIDLib::TBoolean           m_bSafePanBgn;
        tCIDLib::TCard4             m_c4ColCount;
        tCIDLib::TCard4             m_c4FocusIndex;
        tCIDLib::TCard4             m_c4HSpacing;
        tCIDLib::TCard4             m_c4RowCount;
        tCIDLib::TCard4             m_c4RowsHeight;
        tCIDLib::TCard4             m_c4SelIndex;
        tCIDLib::TCard4             m_c4VSpacing;
        tCIDLib::TCard4             m_c4UpdateCnt;
        TListStack                  m_colStack;
        TFacCQCMedia::TMDBPtr       m_cptrMediaDB;
        tCQCMedia::EMediaTypes      m_eInitMediaType;
        tCQCMedia::ESortOrders      m_eSortOrder;
        TBitmapImage                m_imgLoadArt;
        TCQCIntfTemplate            m_iwdgLayout;
        TString                     m_strInitCat;
        TString                     m_strLayoutTmpl;
        TString                     m_strRepoMoniker;
        TString                     m_strTmp1;
        TSize                       m_szArt;
        TTime                       m_tmAddedFmt;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfCvrArtBrowser,TCQCIntfWidget)
        DefPolyDup(TCQCIntfCvrArtBrowser)
};

#pragma CIDLIB_POPPACK


