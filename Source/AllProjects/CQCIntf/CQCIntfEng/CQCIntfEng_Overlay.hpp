//
// FILE NAME: CQCIntfEng_Overlay.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2003
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
//  This is the header for the CQCIntfEng_Overlay.cpp file, which implements the
//  TCQCIntfOverlay class. This class can dynamically be told to load up the contents
//  of other templates into it. It steals the widgets and some of teh config of
//  the loaded template.
//
//  This nesting is not arbitrarily limited, so it can go on forever, but recursive
//  links are culled out during the loading process by looking up the parent chain
//  to see what is already loaded.
//
//  Note that we are not a template, but we are loaded from templates. So we load
//  up the new stuff into a temporary template object and then steal the child
//  widgets and config from him.
//
//
//  Background -
//
//  With one exeption we always use our own background settings, i.e. we don't take
//  background fill/transparency config from the loaded templates. The one exceptoin
//  is that, if we don't have have an image set on us, but the loaded template does,
//  we'll take his image. So we remember if we had an image upon load and use that
//  flat to decide whether to take images from loaded templates.
//
//
//  Loading/Actions -
//
//  We have special concerns with loading. We have bLoadNewTemplate() to start off
//  the loading process, but we also have a bLoadNew() that is used within the
//  recursive loading process that goes on between us and the parent container. The
//  bLoadNewTemplate() kicks off the initialization pass, after the recursive load
//  is done.
//
//  We have our own overlay based actions, currently only for OnScroll. So that is
//  configured at design time. When a template is loaded into us, we take on his
//  OnPreload, OnLoad, etc...
//
//
//  In the case where we get an OnPreload call (from the containing template) to set
//  the initial template, overriding the one configured by the user, we have to at
//  least do the recursive load, so we call bLoadNew() directly there. The reason
//  being that the on preloads happen after the the recursive load.
//
//
//  Scrolling -
//
//  Overlays are scrollable, so if the area containing all of the widgets we load up
//  is larger than the overlay area, then we allow the user to scroll the contents.
//  Of course dynamic resizing could change this relationship at any time, so our
//  content may sometimes be larger than us and sometimes smaller as the user resizes
//  the view.
//
//  Because of many complications that would otherwise occur, we will only allow
//  scrolling if we have a non-transparent background that is unchanging in the dir
//  of travel. Otherwise, every widget would have to support drawing themselves in an
//  alpha aware way so that they could be slid across a non-solid background. That's
//  currently not possible.
//
//  Web browser widgets are a special consideration, because they cannot be slid, and
//  we cannot clip them to the overlay area without making a lot of changes to how
//  things work. So we check to see if any of them exist, and we won't scroll if
//  they are present.
//
//  Overlays can be nested, but since we of course adjust the positions of all of
//  the widgets once the scroll is complete, that will move any immediate child
//  overlays, which will cause a recursive position update.
//
//  We support a 'page mode' setting, which will cause us to only ever allow flicks
//  not drags, and the flicks will move a page at a time. This allows the user to
//  set up a template with pages of content they want discretely displayed and allow
//  the user to move between them with a flick. We provide the current page number
//  as a RTV for display. If we are running in the RIVA mode, then we also only allow
//  for flicks.
//
//  In order to allow the contents to be scrolled even if the only thing visible
//  in the overlay is itself scrollable, we support two fingered drags here, which
//  only this class responds to. So two fingers can always be used to move the
//  contents.
//
//  Manual Flick -
//
//  Overlays can be marked as 'manual flick' mode. In this case, as with page mode
//  and RIVA clients, we will tell the gesture handler that we only accept flicks.
//  And, instead of reacting to them ourself, we will just invoke an OnFlick event
//  which the user can respond to. The direction is passed as a parameter.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfTemplate;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfOverlay
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfOverlay

    : public TCQCIntfContainer
    , public MCQCIntfImgIntf
    , public MCQCCmdSrcIntf
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfOverlay();

        TCQCIntfOverlay
        (
            const   tCQCIntfEng::ECapFlags  eCapFlags
            , const TString&                strType
        );

        TCQCIntfOverlay
        (
            const   TCQCIntfOverlay&        iwdgToCopy
        );

        ~TCQCIntfOverlay();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfOverlay& operator=
        (
            const   TCQCIntfOverlay&        iwdgToAssign
        );


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleHotKey
        (
            const   tCQCIntfEng::EHotKeys   eKey
        )   override;

        tCIDLib::TBoolean bInvokeTrgEvents
        (
            const   tCIDLib::TBoolean        bIsNight
            , const tCQCKit::TCQCEvPtr&      cptrEv
            , const tCIDLib::TCard4          c4Hour
            , const tCIDLib::TCard4          c4Minute
            ,       TTrigEvCmdTarget&        ctarEvent
        )   override;

        tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const override;

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

        [[nodiscard]] TCQCCmdRTVSrc* pcrtsMakeNew
        (
            const   TCQCUserCtx&            cuctxToUse
        )   const override;

        tCIDLib::TVoid PostInit() override;

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

        tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
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

        tCIDLib::TVoid UpdateCmdTarget
        (
            const   TString&                strOldTargetId
            , const TString&                strNewTargetName
            , const TString&                strNewTargetId
            ,       tCQCIntfEng::TDesIdList& fcolAffected
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
        tCIDLib::TBoolean bLoadNew
        (
                    TCQCIntfView* const     pcivOwner
            ,       TDataSrvClient&         dsclInit
            , const TCQCFldCache&           cfcData
            , const TString&                strToLoad
            , const tCIDLib::TBoolean       bThrowOnFail
        );

        tCIDLib::TBoolean bLoadNewTemplate
        (
            const   TString&                strTmplName
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        tCIDLib::TBoolean bManualFlick() const;

        tCIDLib::TBoolean bManualFlick
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPageMode() const;

        tCIDLib::TBoolean bPageMode
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid InitOverlay
        (
                    TCQCIntfView* const     pcivOwner
            , const TString&                strName
            ,       TCQCPollEngine&         polleToUse
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        const TString& strCurTemplate() const;

        const TString& strInitTemplate() const;

        const TString& strInitTemplate
        (
            const   TString&                strToSet
        );

        const TString& strParam
        (
            const   tCIDLib::TCard4         c4Index
        )   const;

        tCIDLib::TVoid SetParam
        (
            const   tCIDLib::TCard4         c4Index
            , const TString&                strToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AdjustChildPos
        (
            const   TPoint&                 pntNew
            ,       TPoint&                 pntAdj
        )   override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        )   override;

        tCIDLib::TVoid DoOnExit
        (
                    TCQCIntfView&           civOwner
        )   override;

        tCIDLib::TVoid DoOnLoad
        (
                    TCQCIntfView&           civOwner
        )   override;

        tCIDLib::TVoid DoOnPreload
        (
                    TCQCIntfView&           civOwner
            , const TString&                strInvokeParms
        )   override;

        tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
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
        tCIDLib::TVoid ResetOverlay();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDrawZContent
        (
                    TGraphDrawDev&          gdevTar
        );

        tCIDLib::TVoid CommonInit
        (
            const   tCIDLib::TBoolean       bScrollable
        );

        tCIDLib::TVoid DoSizeCalcs
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        tCIDLib::TVoid DynOverLoad
        (
            const   TString&                strLayout
            , const TString&                strFldsMons
            , const TString&                strNames
            , const tCIDLib::TBoolean       bHorz
        );

        tCIDLib::TVoid SwizzleCmds();

        tCIDLib::TVoid SwizzleOne
        (
            const   TString&                strTarId
            ,       MCQCCmdSrcIntf&         mcsrcToUpdate
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaAdjDisplay
        //      This is our actual scroll position. This is relative to the
        //      m_areaContent area, and we move this around to keep up with our
        //      actual position. It's where we can blit from the zero based content
        //      to display the current info.
        //
        //  m_areaContent
        //      When we load a template, we remember the size of the template we
        //      loaded, for scrolling purposes. This tells us the maximum size of
        //      the contained content, which controls how far we can scroll in any
        //      given direction. It's at zero origin, and we don't move it.
        //
        //  m_areaDisplay
        //      This is our display area, which is inside any border, with any margin
        //      we want to use. This is what all of the scrolling and whatnot is done
        //      relative to.
        //
        //  m_bHadImage
        //      If an image is set on us, we want to use that and never update it
        //      based on a loaded template. Else we use the settings in the template.
        //      But, since we change our own setting when we load a template, we have
        //      to remember the original value of the Use Image setting, and use that
        //      to know whether to accept the image settings of loaded templates.
        //
        //  m_bHorzDrag
        //      When we get the pre-gesture callback, if we allow a drag, we set this
        //      to remember which direction it is. This lets us know better in the
        //      start gesture what area to allocate for our scroll.
        //
        //  m_bManualFlick
        //      See the class comments above. We only allow flicks and we let the user
        //      handle them himself.
        //
        //  m_bPageMode
        //      This used to be 'no velocity' but now we just treat it as a 'page mode
        //      scrolling' flag. If the overlay is scrollable, this flag will cause it
        //      to only move by page increments (in the direction of travel.)
        //
        //  m_bmpPanCont
        //      We need a bitmap to draw our contents into during pans. For overlays,
        //      we don't have to deal with potential compositing since we require a
        //      scroll safe bgn.
        //
        //  m_colActions
        //  m_colHotKeyActions
        //  m_colTrgEvents
        //      When a template is loaded into us, we adopt his template level actions,
        //      hot keys, and triggered events, so that we can handle those. None of these
        //      are persisted, they are just runtime stuff.
        //
        //  m_colParams
        //      Up to four parameters can be configured. They are available via an
        //      action command, for per-overlay specific setup.
        //
        //  m_eLastFlickDir
        //      If in manual flick mode, we have to store the direction of the flick
        //      before we post the command event, so that the subsequent call to
        //      get the runtime value object can set it. The action that handles the
        //      command will need to know the direction.
        //
        //  m_strCurTemplate
        //      The template we currently have loaded, runtime only.
        //
        //  m_strInitTemplate
        //      The overlay template to load initially when we are created.
        //      From configuration.
        // -------------------------------------------------------------------
        TArea                   m_areaAdjDisplay;
        TArea                   m_areaContent;
        TArea                   m_areaDisplay;
        tCIDLib::TBoolean       m_bHadImage;
        tCIDLib::TBoolean       m_bHorzDrag;
        tCIDLib::TBoolean       m_bManualFlick;
        tCIDLib::TBoolean       m_bPageMode;
        TBitmap                 m_bmpPanCont;
        tCQCIntfEng::TActList   m_colActions;
        tCQCIntfEng::THKActList m_colHotKeyActions;
        tCIDLib::TStrList       m_colParams;
        tCQCEvCl::TTrgEvList    m_colTrgEvents;
        tCIDLib::EDirs          m_eLastFlickDir;
        TString                 m_strCurTemplate;
        TString                 m_strInitTemplate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfOverlay,TCQCIntfContainer)
        DefPolyDup(TCQCIntfOverlay)
};

#pragma CIDLIB_POPPACK


