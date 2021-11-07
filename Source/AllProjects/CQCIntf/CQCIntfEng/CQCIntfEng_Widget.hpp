//
// FILE NAME: CQCIntfEng_Widget.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/16/2001
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
//  This is the header for the CQCIntfEng_Widget.cpp file, which implements the
//  base class that all of the widget description classes derive from.
//
//  This class provides a good number of attributes that all widgets are highly
//  likely to make use of, and the interfaces to access them. The Base tab of the
//  attribute cfg dialog works via this interface for that reason.
//
//  We also provide the fundamental interfaces via which the viewer window
//  interacts with widget, for things such as:
//
//      - Keyboard input from the user
//      - Gesture/mouse input from the user
//      - Focus movement (if that is enabled on a template)
//      - redrawing, moving, hiding, showing, enabling/disabling, etc... widgets.
//      - Copying common blocks of attriutes to other widgets
//      - Updating the status of widgets whose state is driven by various
//        things, such as field values, variables, states, etc... Actually
//        this stuff is all done in the container class (the base class for
//        tempatles and overlays, which contain other widgets and therefore
//        provide the methods to interate through them and cause standard
//        operations to be done.)
//      - Some virtual methods that let the view get get info or force operations
//        on any widget.
//
//  We take a very generic view of how widget values are updated, because they can
//  have their own internal values, values driven by fields, or values driven by
//  global variables, maybe other ways as well in the future. So all we define at
//  this level is a simple interface that the viewer window can use to insure that
//  widgets periodically (pretty quickly) are given a chance to update themselves
//  from their value sources. The widgets are responsible for dealing with invalid
//  or not availble field or variable values, etc...
//
//  Some widgets just update themselves periodically, e.g. an animationwidget. We
//  do a seperate pass to give those a chance to update, which is done more often
//  than the value updates.
//
//  Updates must be FAST and exception safe. The periodic value update callback
//  is provided with the polling engine and the global variables object, so that they
//  can quickly grab their values and update. None of the field based widgets should
//  actually try to read field values from drivers. They should all use the polling
//  engine for fast, and decoupled, access to fields.
//
//  We implement the command target mixin here, since all widgets will be targets
//  for at least the core action commands. we provide default implementations of
//  eDoCmd and QueryCmds to return the base commands that all widgets implement,
//  and that we therefore provide the handling for here. Derived classes call these
//  first to let us handle our stuff, else they handle their own commands.
//
//  In terms of input... Invoke() is called if focus is enabled and the user does an
//  invoke operation while the focus is on this widget, usually an Enter key but also
//  it can be done via our remote control interface. Clicked is called by the gesture
//  handler if they do a click, but essentially does the same thing. The difference
//  is just that Invoke works on whatever internal thing the focus is on, while
//  clicked sees what is under the click point and invokes that thing. We provide
//  basic support here for hit testing, but some widgets may override that if they
//  have special needs beyond the point just being within the widget boundaries.
//
//  If the caps flag indicates this widget supports gestures, then the derived class
//  will override the gesture methods. If they don't override bPerGestOpts() we just
//  return a disable gestures status and indicate that the gesture can propogate to
//  the parent container.
//
//  We have some methods that are purely for the designer, such as getting the size
//  of the content, querying values for the widget palette summary window, querying
//  all the referenced device monikers or images, querying the widget specific uses
//  of the generic colors we support, and so forth.
//
//  We provide a good number of methods that are just conveniences to get, set or
//  clear flags in the capabilities and runtime flags members.
//
//  We provide a DoBgnFill() which does the common functionality that almost all
//  widgets will want to do if they have a border or are non-transparent. It assumes
//  Fgn3 color is the border color and Bgn1/Bgn2 are the fill colors. It also handles
//  setting up the initial clip region and moving it inwards within the border, so
//  that the derived widget knows that it can use that region to clip all of its
//  subsequent drawing.
//
// CAVEATS/GOTCHAS:
//
//  1)  We store both absolute window coordinates and coordinates relative to our
//      parent container, so that we can provide whichever the caller needs.
//
//      We also store or original size, which is used by the dynamic resizing system.
//      Our original size, relative to the original size of our containing template
//      determins where we need to be in the new target after after resizing.
//
//  2)  Some widgets have more than one focus area within them, so there is a
//      bMoveFocus() method. It is called on the current focus widget before moving
//      the focus forward or back. If it returns true, then that means the widget is
//      ready to give up the focus. False means it moved the focus internally and so
//      we keep it as the current focus widget. The default impl of this just returns
//      true and a very small number of widgets need to implement it.
//
//  3)  We support move semantics so that the container and template classes can
//      do so, and save a lot of overhead. In our case, luckily we can use a default
//      generated method since we have a lot of members.
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TIntfEditWnd;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWidget
//  PREFIX: iwdg
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWidget :

    public TObject
    , public MCQCCmdTarIntf
    , public MDuplicable
    , public MStreamable
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TStateList    = TVector<TString>;
        using TStateIndices = TFundVector<tCIDLib::TCard4>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        ~TCQCIntfWidget();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bValidateParm
        (
            const   TCQCCmd&                cmdSrc
            , const TCQCCmdCfg&             ccfgTar
            , const tCIDLib::TCard4         c4Index
            , const TString&                strValue
            ,       TString&                strErrText
        )   override;

        tCIDLib::TVoid CmdTarCleanup() override;

        tCIDLib::TVoid CmdTarInitialize
        (
            const   TCQCActEngine&          acteTar
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
                    TCollection<TCQCCmd>&   colToFill
            , const tCQCKit::EActCmdCtx     eContext
        )   const override;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bCanSizeTo() const;

        virtual tCIDLib::TBoolean bIsSame
        (
            const   TCQCIntfWidget&         iwdgSrc
        )   const = 0;

        virtual tCIDLib::TBoolean bMoveFocus
        (
            const   tCQCKit::EScrNavOps     eOp
        );

        virtual tCIDLib::TBoolean bPerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        );

        virtual tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        );

        virtual tCIDLib::TBoolean bReferencesTarId
        (
            const   TString&                strId
        )   const;

        virtual tCIDLib::TVoid Clicked
        (
            const   TPoint&                 pntAt
        );

        virtual tCIDLib::TCard4 c4QueryEmbeddedCmdSrcs
        (
                    tCQCKit::TCmdSrcList&   colToFill
        );

        virtual tCIDLib::TVoid CopyFrom
        (
            const   TCQCIntfWidget&         iwdgSrc
        ) = 0;

        virtual tCIDLib::TVoid GestInertiaIdle
        (
            const   tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
        );

        virtual tCIDLib::TVoid Initialize
        (
                    TCQCIntfContainer* const piwdgParent
            ,       TDataSrvClient&         dsclInit
            ,       tCQCIntfEng::TErrList&  colErrs
        );

        virtual tCIDLib::TVoid Invalidate();

        virtual tCIDLib::TVoid Invalidate
        (
            const   TArea&                  areaInvalidate
        );

        virtual tCIDLib::TVoid Invoke();

        virtual tCIDLib::TVoid PostInit();

        virtual tCIDLib::TVoid ProcessFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        );

        virtual tCIDLib::TVoid QueryContentSize
        (
                    TSize&                  szToFill
        );

        virtual tCIDLib::TVoid QueryMonikers
        (
                    tCIDLib::TStrHashSet&   colToFill
        )   const;

        virtual tCIDLib::TVoid QueryReferencedImgs
        (
                    tCIDLib::TStrHashSet&   colToFill
            ,       tCIDLib::TStrHashSet&   colScopes
            , const tCIDLib::TBoolean       bIncludeSysImgs
            , const tCIDLib::TBoolean       bIncludeScopes
        )   const;

        virtual tCIDLib::TVoid QueryWdgAttrs
        (
                    tCIDMData::TAttrList&   colAttrs
            ,       TAttrData&              adatTmp
        )   const;

        virtual tCIDLib::TVoid RefreshImages
        (
                    TDataSrvClient&         dsclToUse
        );

        virtual tCIDLib::TVoid Replace
        (
            const   tCQCIntfEng::ERepFlags  eToRep
            , const TString&                strSrc
            , const TString&                strTar
            , const tCIDLib::TBoolean       bRegEx
            , const tCIDLib::TBoolean       bFullMatch
            ,       TRegEx&                 regxFind
        );

        virtual tCIDLib::TVoid ResetWidget();

        virtual TSize szDefaultSize() const = 0;

        virtual tCIDLib::TVoid ScaleInternal
        (
            const   tCIDLib::TFloat8       f8XScale
            , const tCIDLib::TFloat8       f8YScale
        );

        virtual tCIDLib::TVoid SetWdgAttr
        (
                    TAttrEditWnd&           wndAttrEd
            , const TAttrData&              adatNew
            , const TAttrData&              adatOld
        );

        virtual tCIDLib::TVoid Terminate();

        virtual tCIDLib::TVoid Validate
        (
            const   TCQCFldCache&           cfcData
            ,       tCQCIntfEng::TErrList&  colErrs
            ,       TDataSrvClient&         dsclVal
        )   const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TArea& areaActual() const;

        const TArea& areaOrg() const;

        const TArea& areaRelative() const;

        tCIDLib::TVoid AddState
        (
            const   TString&                strToAdd
        );

        tCIDLib::TBoolean bAcceptsFocus() const;

        tCIDLib::TBoolean bCanRedraw
        (
            const   TString&                strEventId
        )   const;

        tCIDLib::TBoolean bCheckStateRefs
        (
            const   TCQCIntfStateList* const pistlStates
            ,       tCIDLib::TStrHashSet&   colBadList
        )   const;

        tCIDLib::TBoolean bContainsPoint
        (
            const   TPoint&                 pntToTest
        )   const;

        tCIDLib::TBoolean bDoesGestures() const;

        tCIDLib::TBoolean bDoesGestures
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bFindMostRestrictiveClip
        (
                    TGUIRegion&             rgnToSet
        );

        tCIDLib::TBoolean bFindMostRestrictiveClip
        (
                    TGUIRegion&             rgnToSet
            , const TArea&                  areaStart
        );

        tCIDLib::TBoolean bFindMostRestrictiveClip
        (
                    TGUIRegion&             rgnToSet
            , const tCIDLib::TCard4         c4HDeflate
            , const tCIDLib::TCard4         c4VDeflate
        );

        tCIDLib::TBoolean bHasBorder() const;

        tCIDLib::TBoolean bHasBorder
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bHasFocus() const;

        tCIDLib::TBoolean bHasFocus
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bInitInvisible() const;

        tCIDLib::TBoolean bInitInvisible
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIntersects
        (
            const   TArea&                  areaToTest
        )   const;

        tCIDLib::TBoolean bInGesture() const;

        tCIDLib::TBoolean bInGesture
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsDead() const;

        tCIDLib::TBoolean bIsDead
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsLocked() const;

        tCIDLib::TBoolean bIsLocked
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bIsGrouped() const;

        tCIDLib::TBoolean bIsTransparent() const;

        tCIDLib::TBoolean bIsTransparent
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNegateStates() const;

        tCIDLib::TBoolean bNegateStates
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bNoInertia() const;

        tCIDLib::TBoolean bNoInertia
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bScrollSafeBgn
        (
            const   tCIDLib::TBoolean       bVert
        )   const;

        tCIDLib::TBoolean bShadow() const;

        tCIDLib::TBoolean bShadow
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bTrackBeyond() const;

        tCIDLib::TBoolean bUnderline() const;

        tCIDLib::TBoolean bUnderline
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bUpdateDisplayState
        (
            const   TCQCIntfStateList&      istlData
        );

        tCIDLib::TBoolean bUsesState
        (
            const   TString&                strToFind
        )   const;

        tCIDLib::TBoolean bWantsActiveUpdate() const;

        tCIDLib::TBoolean bWantsAnyUpdate() const;

        tCIDLib::TBoolean bWantsValueUpdate() const;

        tCIDLib::TCard4 c4GroupId() const;

        tCIDLib::TCard4 c4GroupId
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4Rounding() const;

        tCIDLib::TCard4 c4Rounding
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4RefCount() const;

        tCIDLib::TCard8 c8DesId() const;

        tCIDLib::TCard8 c8DesId
        (
            const   tCIDLib::TCard8         c8ToSet
        );

        TCQCIntfView& civOwner();

        const TCQCIntfView& civOwner() const;

        tCIDLib::TVoid DecRefCount();

        tCIDLib::TVoid DeleteState
        (
            const   TString&                strToDel
        );

        tCIDLib::TVoid DeleteStates
        (
            const   tCIDLib::TStrCollect&   colToRemove
        );

        tCIDLib::TVoid DrawOn
        (
                    TGraphDrawDev&          gevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        );

        tCQCIntfEng::EBgnStyles eBgnStyle() const;

        tCQCIntfEng::EBgnStyles eBgnStyle
        (
            const   tCQCIntfEng::EBgnStyles eToSet
        );

        tCQCIntfEng::ECapFlags eCapFlags() const;

        tCQCIntfEng::EDispStates eCurDisplayState() const;

        tCQCKit::EUserRoles eMinWriteRole() const;

        tCQCIntfEng::ERunFlags eRunFlags() const;

        tCQCIntfEng::ERunFlags eSetRunFlags
        (
            const   tCQCIntfEng::ERunFlags eToSet
            , const tCQCIntfEng::ERunFlags eMask
        );

        TCQCIntfTemplate& iwdgRootTemplate();

        const TCQCIntfTemplate& iwdgRootTemplate() const;

        tCIDLib::TVoid IncRefCount();

        tCIDLib::TVoid LookupStates
        (
            const   TCQCIntfContainer* const piwdgParent
        );

        TCQCIntfView* pcivOwner();

        const TCQCIntfView* pcivOwner() const;

        const TCQCIntfContainer* piwdgParent() const
        {
            return m_piwdgParent;
        }

        TCQCIntfContainer* piwdgParent()
        {
            return m_piwdgParent;
        }

        tCIDLib::TVoid PostOps
        (
                    MCQCCmdSrcIntf&         mcsrcAction
            , const TString&                strEvent
            , const tCQCKit::EActOrders     eOrder
        );

        tCIDLib::TVoid QueryNamePath
        (
                    TString&                strToFill
            , const tCIDLib::TBoolean       bIncludeMe
        )   const;

        tCIDLib::TVoid QueryClipRegion
        (
                    TGUIRegion&             grgnToUpdate
            , const tCIDLib::TBoolean       bSet = kCIDLib::True
        );

        tCIDLib::TVoid QuerySupportedClrs
        (
                    tCIDLib::TKVPCollect&   colToFill
        )   const;

        const TRGBClr& rgbBgn() const;

        const TRGBClr& rgbBgn
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbBgn2() const;

        const TRGBClr& rgbBgn2
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbExtra() const;

        const TRGBClr& rgbExtra
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbExtra2() const;

        const TRGBClr& rgbExtra2
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbExtra3() const;

        const TRGBClr& rgbExtra3
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbFgn() const;

        const TRGBClr& rgbFgn
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbFgn2() const;

        const TRGBClr& rgbFgn2
        (
            const   TRGBClr&                rgbToSet
        );

        const TRGBClr& rgbFgn3() const;

        const TRGBClr& rgbFgn3
        (
            const   TRGBClr&                rgbToSet
        );

        tCIDLib::TVoid ResetCaption
        (
            const   tCIDLib::TBoolean       bSingleLine
            , const TString&                strText
        );

        const TString& strCaption() const;

        const TString& strCaption
        (
            const   TString&                strToSet
        );

        const TString& strGroupName() const;

        const TString& strGroupName
        (
            const   TString&                strToSet
        );

        const TString& strTypeName() const;

        const TString& strWidgetId() const;

        const TString& strWidgetId
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaToSet
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetArea
        (
            const   TPoint&                 pntNewOrg
            , const TSize&                  szNew
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetDisplayState
        (
            const   tCQCIntfEng::EDispStates eToSet
            , const tCIDLib::TBoolean       bRedraw = kCIDLib::True
        );

        tCIDLib::TVoid SetOrg
        (
            const   TPoint&                 pntNew
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetOwner
        (
                    TCQCIntfView* const     pcivOwner
        );

        tCIDLib::TVoid SetParent
        (
                    TCQCIntfContainer* const piwdgToSet
        );

        tCIDLib::TVoid SetParentOrg
        (
            const   TPoint&                 pntToSet
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetPositions
        (
            const   TPoint&                 pntParentOrg
            , const TPoint&                 pntNew
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetPositions
        (
            const   TPoint&                 pntParentOrg
            , const TArea&                  areaNew
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetSize
        (
            const   TSize&                  szNew
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid SetSize
        (
            const   tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Height
            , const tCIDLib::TBoolean       bReport = kCIDLib::True
        );

        tCIDLib::TVoid Ungroup();


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TCQCIntfContainer;
        friend class TCQCIntfView;
        friend class TCQCIntfStdView;
        friend class TIntfEditWnd;
        friend class TWdgStatesDlg;


        // -------------------------------------------------------------------
        //  Hidden constructors and operators
        // -------------------------------------------------------------------
        TCQCIntfWidget
        (
            const   tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
            , const tCIDLib::TCard4         c4InitStyle = 0
        );

        TCQCIntfWidget
        (
            const   TArea&                  areaInit
            , const tCQCIntfEng::ECapFlags  eCaps
            , const TString&                strTypeName
            , const tCIDLib::TCard4         c4InitStyle = 0
        );

        TCQCIntfWidget
        (
            const   TCQCIntfWidget&         iwdgSrc
        );

        TCQCIntfWidget(TCQCIntfWidget&&) = delete;

        TCQCIntfWidget& operator=
        (
            const   TCQCIntfWidget&         iwdgSrc
        );

        TCQCIntfWidget& operator=(TCQCIntfWidget&&) = default;


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid ActiveUpdate
        (
            const   tCIDLib::TBoolean       bNoRedraw
            , const TGUIRegion&             grgnClip
            , const tCIDLib::TBoolean       bInTopLayer
        );

        virtual tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaNew
            , const TArea&                  areaOld
        );

        virtual tCIDLib::TVoid CaptionChanged();

        virtual tCIDLib::TVoid DisplayStateChanged();

        virtual tCIDCtrls::EWndAreas eHitTest
        (
            const   TPoint&                 pntTest
        )   const;

        virtual tCIDLib::TVoid GotFocus();

        virtual tCIDLib::TVoid LostFocus();

        virtual tCIDLib::TVoid ParentDisplayStateChanged
        (
            const   tCQCIntfEng::EDispStates eState
        );

        virtual tCIDLib::TVoid RegisterFields
        (
                    TCQCPollEngine&         polleToUse
            , const TCQCFldCache&           cfcData
        );

        virtual tCIDLib::TVoid ScrPosChanged
        (
            const   TArea&                  areaTmplWnd
        );

        virtual tCIDLib::TVoid SizeChanged
        (
            const   TSize&                  szNew
            , const TSize&                  szOld
        );

        virtual tCIDLib::TVoid StyleFlagChanged
        (
            const   tCIDLib::TCard4         c4OldFlags
            , const tCIDLib::TCard4         c4NewFlags
        );

        virtual tCIDLib::TVoid Update
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaInvalid
            ,       TGUIRegion&             grgnClip
        ) = 0;

        virtual tCIDLib::TVoid ValueUpdate
        (
                    TCQCPollEngine&         polleToUse
            , const tCIDLib::TBoolean       bNoRedraw
            , const tCIDLib::TBoolean       bVarsUpdate
            , const TStdVarsTar&            ctarGlobalVars
            , const TGUIRegion&             grgnClip
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


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCQCIntfEng::TGraphIntfDev cptrNewGraphDev() const;

        tCIDLib::TVoid DoBgnFill
        (
                    TGraphDrawDev&          gdevTarget
            , const TArea&                  areaToFill
        );

        const TGraphDrawDev& gdevCompat() const;

        tCIDLib::TVoid Redraw
        (
            const   TArea&                  areaToRedraw
        );

        tCIDLib::TVoid Redraw
        (
            const   TGUIRegion&             grgnToRedraw
        );

        tCIDLib::TVoid Redraw();

        tCIDLib::TVoid SetCapFlags
        (
            const   tCQCIntfEng::ECapFlags  eFlags
            , const tCQCIntfEng::ECapFlags  eMask
        );

        tCIDLib::TVoid ValidateTypes
        (
            const   TClass&                 clsSrc
        )   const;

        tCIDLib::TVoid WriteValue
        (
            const   TString&                strMoniker
            , const TString&                strFldName
            , const TString&                strValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetDefAttrs
        (
            const   tCIDLib::TCard4         c4DefStyles
        );

        tCIDLib::TVoid SetRelativeArea();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaActual
        //      Our absolute area, which is the m_areaAt area, plus the base
        //      origin of our parent template widget.
        //
        //  m_areaOrg
        //      This is our original area when we were loaded, which is required
        //      for dynamic resizing, so that we can maintain the same relationship
        //      to the new parent container size as what we originally had.
        //
        //  m_areaRelative
        //      The relative area of this widget, which is only for human
        //      consumption. We calculate it based on the actual set area or
        //      origin and the parent origin.
        //
        //  m_bLocked
        //      Only used by the designer. It prevents the widget from ever
        //      being selected, so that it won't be accidentally moved or
        //      affected.
        //
        //  m_bNegateStates
        //      If we have any associated states, this can be set to cause the overall
        //      result of state evaluation to be negated. This often avoids the need
        //      to create two states that just effectively reflect two sides of the
        //      same situation.
        //
        //  m_bSingleLine
        //      We are told by the derived class whether we should take a single line
        //      or multiple lines of caption text (if we take any at all.) This tells
        //      any config tabs which of two different text input widgets to present.
        //
        //  m_bUseStateDisplay
        //      We have two different display states. One that is what is set by the
        //      user via hide/show commands, or in some cases by the widget itself
        //      (such as 'hide if zero'.) The other is that imposed by template states.
        //      This tells us which of the states we should treat as the current one.
        //      See m_eCurDisplayState below.
        //
        //  m_c4GroupId
        //      To support widget grouping, this is set to a unique group id for all
        //      widgets in a group. They have to be sequential in the zorder to be
        //      grouped as well. The designer enforces this. Zero means ungrouped
        //      since that's never used as a group id.
        //
        //      The group ids are persisted course, but only ever actually used by
        //      the designer. They have no purpose at viewing time.
        //
        //  m_c4RefCount
        //      We have to deal with the situation where widgets invoke an action and
        //      that action may destroy the widget, but we have called into it and
        //      still need to get out safely. So we have a simple reference counting
        //      scheme and a janitor class that can be used to bump the count on a
        //      scoped basis. This way, the widget stays alive even if dropped by the
        //      container that owns it. The container will mark us as dead if the count
        //      is greater than 1 when the container is destroyed. The widgets are
        //      responsible for checking this dead flag and not doing further processing
        //      after invoking an action if it is set.
        //
        //  m_c4Rounding
        //      If not transparent, i.e. doing a background fill, this controls the
        //      corner rounding of the fill.
        //
        //  m_c4StyleFlags
        //      We use this to store some bit flags that control styles of a widget.
        //      These are initially set by the derived classes to some useful default.
        //      And then are modified by the user at design time. They have to be
        //      persisted of course.
        //
        //  m_c8DesId
        //      The design time id, only used in the designer. This is assigned by
        //      the designer to it's purposes, primarily to provide a unique id for
        //      undo purposes. The high 32 bits are assigned the id of the template
        //      it is in. The low 32 are assigned values out of the same range.
        //
        //      The ids are never reused, so this means that widgets cut from one
        //      template and pasted into another will have ids that cannot accidentally
        //      match those of any widgets in the target template. This way, the
        //      designer knows the difference between that and widgets cut from the
        //      current template and then pasted back in (potentially multiple times.)
        //
        //  m_colStateNames
        //      The list of names of any states that we are dependent on. It will often
        //      be empty. These are used during init to look up all our states and fill
        //      in the m_fcolStates list.
        //
        //  m_eBgnStyle
        //      Indicates the background fill style of this template.
        //
        //  m_eCapFlags
        //      The capabilities flags that are set on us by the derived classes.
        //      These flags indicate some things about the widget that the editor
        //      needs to know. These are persisted, they are just set on us by
        //      each derived class at runtime.
        //
        //  m_eCurDisplayState
        //      This is set to control whether this widget is visible, disabled, or
        //      hidden. But see m_bUseStateDisplay.
        //
        //  m_eLogOp
        //      If this widget uses any states, then this is the logical operator that
        //      is used to combine the results of the state values to determine the
        //      overall value, which is used to set the current display state.
        //
        //  m_eRunFlags
        //      The runtime flags for this widget. These are not persisted, they just
        //      allow the controlling app to set some aspects of our operating, such
        //      as designer vs. live mode.
        //
        //  m_eStateDisplay
        //      This is the display state to take if the states referenced by this
        //      widget resolve to True. If so, the m_bUseStateDisplay flag is set and
        //      this will be used instead of m_eCurDisplayState.
        //
        //  m_fcolStates
        //      We look up any states we have and put their states in here. This lets
        //      us do fast evaluation of our states. This isn't persisted of course. It's
        //      in the same order as m_colStatesNames.
        //
        //  m_piwdgParent
        //      A pointer to our parent container, so that we can work back up a
        //      nesting chain.
        //
        //  m_pntParent
        //      The origin of our parent area. This tells us how much to offset
        //      our relative area origin to get our absolute origin.
        //
        //  m_pcivOwner
        //      The back-link to the view that owns us. We need this in order to do
        //      some things, such as get a graphic device to draw on.
        //
        //  m_rgbXXX
        //      These are colors that are used for standard parts of each widget. The
        //      user can control these, but we default them in case they don't. Each
        //      widget can choose to use them as desired.
        //
        //  m_strCaption
        //      The caption text. This is persisted. Many don't have any caption
        //      text.
        //
        //  m_strGroupName
        //      If this is the root member of a group, this holds the group name. It
        //      will be empty unless the user sets it. Any time that this widget is
        //      ungrouped or grouped with a non-zero group index, the name is emptied
        //      out.
        //
        //  m_strTypeName
        //      The derived class will set this value on us during ctor. It provides
        //      us with a human readable type description of the widget for use in
        //      error messages and such.
        //
        //  m_strWidgetId
        //      Some widgets are named so that they can be addressed from other
        //      widgets, which can tell them to do something. At viewing time this
        //      id is used to create a more unique id which is set on our underlying
        //      command target mixin.
        // -------------------------------------------------------------------
        TArea                       m_areaActual;
        TArea                       m_areaOrg;
        TArea                       m_areaRelative;
        tCIDLib::TBoolean           m_bLocked;
        tCIDLib::TBoolean           m_bNegateStates;
        tCIDLib::TBoolean           m_bSingleLine;
        tCIDLib::TBoolean           m_bUseStateDisplay;
        tCIDLib::TCard4             m_c4GroupId;
        tCIDLib::TCard4             m_c4RefCount;
        tCIDLib::TCard4             m_c4Rounding;
        tCIDLib::TCard4             m_c4StyleFlags;
        tCIDLib::TCard8             m_c8DesId;
        TStateList                  m_colStateNames;
        tCQCIntfEng::EBgnStyles     m_eBgnStyle;
        tCQCIntfEng::ECapFlags      m_eCapFlags;
        tCQCIntfEng::EDispStates    m_eCurDisplayState;
        tCQCKit::ELogOps            m_eLogOp;
        tCQCIntfEng::ERunFlags      m_eRunFlags;
        tCQCIntfEng::EDispStates    m_eStateDisplay;
        TStateIndices               m_fcolStates;
        TCQCIntfContainer*          m_piwdgParent;
        TPoint                      m_pntParent;
        TCQCIntfView*               m_pcivOwner;
        TRGBClr                     m_rgbBgn1;
        TRGBClr                     m_rgbBgn2;
        TRGBClr                     m_rgbExtra1;
        TRGBClr                     m_rgbExtra2;
        TRGBClr                     m_rgbExtra3;
        TRGBClr                     m_rgbFgn1;
        TRGBClr                     m_rgbFgn2;
        TRGBClr                     m_rgbFgn3;
        TString                     m_strCaption;
        TString                     m_strGroupName;
        TString                     m_strTypeName;
        TString                     m_strWidgetId;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfWidget,TObject)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWdgJanitor
//  PREFIX: jan
// ---------------------------------------------------------------------------
class CQCINTFENGEXPORT TCQCIntfWdgJanitor
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfWdgJanitor();

        TCQCIntfWdgJanitor
        (
                    TCQCIntfWidget* const   piwdgToSanitize
        );

        TCQCIntfWdgJanitor(const TCQCIntfWdgJanitor&) = delete;

        ~TCQCIntfWdgJanitor();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfWdgJanitor& operator=(const TCQCIntfWdgJanitor&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHasRef() const;

        tCIDLib::TVoid DropRef();

        TCQCIntfWidget* piwdgOrphan();

        TCQCIntfWidget* piwdgRef();

        const TCQCIntfWidget* piwdgRef() const;

        tCIDLib::TVoid SetNew
        (
                    TCQCIntfWidget* const   piwdgNew
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_piwdgToSanitize
        //      This is the widget that we are keeping alive by bumping it's
        //      reference count.
        // -------------------------------------------------------------------
        TCQCIntfWidget* m_piwdgToSanitize;
};


#pragma CIDLIB_POPPACK

