//
// FILE NAME: CQCIntfEng_Overlay.cpp
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
//  This file implements a template overlay class, which displays a smaller
//  template inside a main parent template.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEng_.hpp"
#include    "CQCIntfEng_CoverArtBrowser.hpp"
#include    "CQCIntfEng_WebBrowser.hpp"



// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfOverlay,TCQCIntfContainer)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_Overlay
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 2
        //      Added the OnPreload event (at viewing time) so that we can pull
        //      in those action commands from the templates loaded into us. So we
        //      need to know, upon streaming in, if we need to force this new
        //      event in.
        //
        //  Version 3
        //      No change in format, but we moved to hierarchical storage for
        //      templates, so we have to add a /User/ prefix to the initial
        //      template name.
        //
        //  Version 4
        //      Added support for non-velocity sensitive scrolling. Normally it
        //      is velocity sensitive, but can be turned off and they can scroll
        //      by 10 percent increments of the page width/height.
        //
        //  Version 5
        //      We always implemented the cmd src mixin, but never streamed it,
        //      since we only handled redirected commands at viewing time. Now
        //      we support scrolling and added an OnScroll, so we have to start
        //      streaming it.
        //
        //  Version 6
        //      In retrospect, moving forward towards native scrolling on all
        //      platforms, the stuff from V4 above is too flexible. We reduce this
        //      to just a single 'page oriented scrolling' flag. We we either get
        //      native velocity scroll, or up to a page at a time.
        //
        //  Version 7
        //      No data change, but we want to move the border color to F3, to be
        //      consistent with other widgets.
        //
        //  Version 8
        //      Added configurable parameters to the overlay.
        //
        //  Version 9
        //      Convert paths to 5.x format
        //
        //  Version 10
        //      Added the 'manual flick' option
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 10;


        // -----------------------------------------------------------------------
        //  The margin we move in for clipping, to leave a little margin. It's used
        //  multiple places, so just to be able to change it once if we want to
        //  try different things.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4Margin = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use.
        //
        //  Note that we don't actually have any associated field/vars or do any
        //  active updates, but we indicate we want such updates. This insures
        //  that our parent (container) class correctly recurses on us during
        //  update scans, and we can in turn then recurse to our children.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::NameReq
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfOverlayRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class to provide some extra
//  overlay specific RTVs.
// ---------------------------------------------------------------------------
class TCQCIntfOverlayRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfOverlayRTV( const   tCIDLib::TCard4 c4HPageInd
                            , const tCIDLib::TCard4 c4VPageInd
                            , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_c4HPageInd(c4HPageInd)
            , m_c4VPageInd(c4VPageInd)
            , m_eFlickDir(tCIDLib::EDirs::Count)
        {
        }

        ~TCQCIntfOverlayRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue( const   TString&    strId
                                    ,       TString&    strToFill) const final
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            // If it's one of ours, then handle it
            if (strId == kCQCIntfEng::strRTVId_HPageInd)
            {
                strToFill.AppendFormatted(m_c4HPageInd);
            }
             else if (strId == kCQCIntfEng::strRTVId_VPageInd)
            {
                strToFill.AppendFormatted(m_c4VPageInd);
            }
             else if (strId == kCQCIntfEng::strRTVId_FlickDir)
            {
                if (m_eFlickDir == tCIDLib::EDirs::Down)
                    strToFill = L"Down";
                else if (m_eFlickDir == tCIDLib::EDirs::Left)
                    strToFill = L"Left";
                else if (m_eFlickDir == tCIDLib::EDirs::Right)
                    strToFill = L"Right";
                else if (m_eFlickDir == tCIDLib::EDirs::Up)
                    strToFill = L"Up";
                else
                    strToFill = L"???";
            }
             else
            {
                return kCIDLib::False;
            }

            return kCIDLib::True;
        }


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetFlickDir(const tCIDLib::EDirs eToSet)
        {
            m_eFlickDir = eToSet;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4HPageInd
        //      The currently displayed pages in each direction. Really only
        //      meaningful if in paged scrolling mode.
        //
        //  m_eFlickDir
        //      If in manual flick mode, this will be set on us.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4HPageInd;
        tCIDLib::TCard4 m_c4VPageInd;
        tCIDLib::EDirs  m_eFlickDir;
};



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfOverlay
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfOverlay::TCQCIntfOverlay() :

    TCQCIntfContainer
    (
        CQCIntfEng_Overlay::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Overlay)
        , kCQCIntfEng::c4StyleFlag_HasBorder
    )
    , MCQCIntfImgIntf(kCIDLib::False)
    , MCQCCmdSrcIntf()
    , m_bHadImage(kCIDLib::False)
    , m_bManualFlick(kCIDLib::False)
    , m_bPageMode(kCIDLib::False)
{
    // Do common init
    CommonInit(kCIDLib::True);
}

//
//  For derived classes to use, so that they can indicate a different type and
//  use different capabilities flags.
//
TCQCIntfOverlay::TCQCIntfOverlay(const  tCQCIntfEng::ECapFlags  eCapFlags
                                , const TString&                strType) :

    TCQCIntfContainer(eCapFlags, strType, kCQCIntfEng::c4StyleFlag_HasBorder)
    , MCQCIntfImgIntf(kCIDLib::False)
    , MCQCCmdSrcIntf()
    , m_bHadImage(kCIDLib::False)
    , m_bManualFlick(kCIDLib::False)
    , m_bPageMode(kCIDLib::False)
    , m_colHotKeyActions()
{
    // Do common init. Tell it not to include the OnScroll
    CommonInit(kCIDLib::False);
}

TCQCIntfOverlay::TCQCIntfOverlay(const TCQCIntfOverlay& iwdgSrc) :

    TCQCIntfContainer(iwdgSrc)
    , MCQCIntfImgIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_areaContent(iwdgSrc.m_areaContent)
    , m_bHadImage(iwdgSrc.m_bHadImage)
    , m_bManualFlick(iwdgSrc.m_bManualFlick)
    , m_bPageMode(iwdgSrc.m_bPageMode)
    , m_colActions(iwdgSrc.m_colActions)
    , m_colHotKeyActions(iwdgSrc.m_colHotKeyActions)
    , m_colParams(iwdgSrc.m_colParams)
    , m_colTrgEvents(iwdgSrc.m_colTrgEvents)
    , m_strInitTemplate(iwdgSrc.m_strInitTemplate)

    // Runtime stuff but needed for design time if we have an image
    , m_areaDisplay(iwdgSrc.m_areaDisplay)
{
}

TCQCIntfOverlay::~TCQCIntfOverlay()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Public operators
// ---------------------------------------------------------------------------
TCQCIntfOverlay&
TCQCIntfOverlay::operator=(const TCQCIntfOverlay& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bHadImage         = iwdgSrc.m_bHadImage;
        m_bManualFlick      = iwdgSrc.m_bManualFlick;
        m_bPageMode         = iwdgSrc.m_bPageMode;
        m_colActions        = iwdgSrc.m_colActions;
        m_colHotKeyActions  = iwdgSrc.m_colHotKeyActions;
        m_colParams         = iwdgSrc.m_colParams;
        m_colTrgEvents      = iwdgSrc.m_colTrgEvents;
        m_strInitTemplate   = iwdgSrc.m_strInitTemplate;

        //
        //  This stuff is viewing time but we still want to display a background
        //  image if one is set, so we keep the display area.
        //
        m_areaDisplay       = iwdgSrc.m_areaDisplay;

        m_areaAdjDisplay.ZeroAll();
        m_areaContent.ZeroAll();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this from our parent class. We call our parent first to see
//  if any nested overlay has a handler. If not, we see if we do, or if we
//  have an Otherwise clause.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bHandleHotKey(const tCQCIntfEng::EHotKeys eKey)
{
    // See if a nested overlay has a handler. If so, we are done
    if (TParent::bHandleHotKey(eKey))
        return kCIDLib::True;

    // Just in case, make sure it's legal for our list of hot keys
    if (tCIDLib::TCard4(eKey) >= m_colHotKeyActions.c4ElemCount())
        return kCIDLib::False;

    // Our list may be empty since we only have them if the loaded template has them
    if (!m_colHotKeyActions.bIsEmpty())
    {
        TCQCStdCmdSrc& csrcKey = m_colHotKeyActions[eKey];
        if (csrcKey.bUsed())
        {
            // It's defined so let's post it and return true
            PostOps(csrcKey, kCQCKit::strEvId_OnTrigger, tCQCKit::EActOrders::NonNested);
            return kCIDLib::True;
        }
    }

    // No handler at this level
    return kCIDLib::False;
}


//
//  We override this from our parent class. This is called to give us a chance
//  to process an incoming triggered event.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bInvokeTrgEvents(  const   tCIDLib::TBoolean   bIsNight
                                    , const tCQCKit::TCQCEvPtr& cptrEv
                                    , const tCIDLib::TCard4     c4Hour
                                    , const tCIDLib::TCard4     c4Minute
                                    ,       TTrigEvCmdTarget&   ctarEvent)
{
    //
    //  We just call a helper on our parent class to do the work, passing it
    //  our list of triggered events, if any.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (!m_colTrgEvents.bIsEmpty())
        bRet = bRunTrgEvents(m_colTrgEvents, bIsNight, cptrEv, c4Hour, c4Minute, ctarEvent);

    // And pass it on to our parent class, which will in turn do any of his children
    if (TParent::bInvokeTrgEvents(bIsNight, cptrEv, c4Hour, c4Minute, ctarEvent))
        bRet = kCIDLib::True;

    return bRet;
}


//
//  This is design time only, but we still need to recurse because it's being used
//  most of the time to check at the template level for changes, which includes the
//  child widgets.
//
tCIDLib::TBoolean TCQCIntfOverlay::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfOverlay& iwdgOther(static_cast<const TCQCIntfOverlay&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgIntf::bSameImg(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Check our simple stuffer
    if ((m_bPageMode != iwdgOther.m_bPageMode)
    ||  (m_bManualFlick != iwdgOther.m_bManualFlick)
    ||  (m_strInitTemplate != iwdgOther.m_strInitTemplate)
    ||  (m_colActions.c4ElemCount() != iwdgOther.m_colActions.c4ElemCount())
    ||  (m_colHotKeyActions.c4ElemCount() != iwdgOther.m_colHotKeyActions.c4ElemCount())
    ||  (m_colTrgEvents.c4ElemCount() != iwdgOther.m_colTrgEvents.c4ElemCount()))
    {
        return kCIDLib::False;
    }

    // Compare the parameters
    tCIDLib::TCard4 c4Count = m_colParams.c4ElemCount();
    CIDAssert
    (
        c4Count == iwdgOther.m_colParams.c4ElemCount()
        , L"The overlay parameter count is wrong"
    );

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colParams[c4Index] != iwdgOther.m_colParams[c4Index])
            return kCIDLib::False;
    }

    // Compare the various actions
    c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colActions[c4Index] != iwdgOther.m_colActions[c4Index])
            return kCIDLib::False;
    }

    //
    //  We only have hot key sif the template loaded into us does. Since we
    //  use the enum and not the element count, we have to check this first.
    //
    if (!m_colHotKeyActions.bIsEmpty())
    {
        tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
        for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
        {
            if (m_colHotKeyActions[eKey] != iwdgOther.m_colHotKeyActions[eKey])
                return kCIDLib::False;
        }
    }

    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colTrgEvents[c4Index] != iwdgOther.m_colTrgEvents[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Assuming that we are not transparent and have no non-scrollables, and that
//  we have scrollable content in the axis indicated and our background is scroll
//  safe....
//
//  If in page mode or our no inertia flag is set or in manual flick mode, then
//  we accept flicks only. Otherwise, we allow inertial panning, one or two fingers.
//
//  If manual flicks are enabled, we don't care if the content is bigger than the
//  overlay, since they are often using it to load a new template instead of just
//  move the existing content.
//
//  We never allow propogation.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bPerGestOpts(  const   TPoint&                 pntAt
                                , const tCIDLib::EDirs          eDir
                                , const tCIDLib::TBoolean
                                ,       tCIDCtrls::EGestOpts&   eToSet
                                ,       tCIDLib::TFloat4&       f4VScale)
{
    // Assume the worst until proven otherwise
    eToSet = tCIDCtrls::EGestOpts::Disable;

    //
    //  If we are not transparent and have no non-scrollable children,
    //  and we have a safe background for the direction, and we are
    //  smaller than the content in the direction, we can do pans.
    //
    if (!bIsTransparent() && !bCheckForNonScrollables())
    {
        // Check the background fill type for each direction
        if (((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
        &&  bScrollSafeBgn(kCIDLib::False)
        &&  ((m_areaDisplay.c4Width() < m_areaContent.c4Width()) || m_bManualFlick))
        {
            if (bPageMode() || bNoInertia() || TFacCQCIntfEng::bRemoteMode() || m_bManualFlick)
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;

            m_bHorzDrag = kCIDLib::True;
        }
         else if (((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Down))
              &&  bScrollSafeBgn(kCIDLib::True)
              &&  ((m_areaDisplay.c4Height() < m_areaContent.c4Height()) || m_bManualFlick))
        {
            if (bPageMode() || bNoInertia() || TFacCQCIntfEng::bRemoteMode() || m_bManualFlick)
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;

            m_bHorzDrag = kCIDLib::False;
        }

        f4VScale = 1.0F;
    }

    // If we didn't handle it, let it propagate
    return (eToSet == tCIDCtrls::EGestOpts::Disable);
}



tCIDLib::TBoolean
TCQCIntfOverlay::bProcessGestEv(const   tCIDCtrls::EGestEvs eEv
                                , const TPoint&             pntStart
                                , const TPoint&             pntAt
                                , const TPoint&             pntDelta
                                , const tCIDLib::TBoolean   bTwoFinger)
{
    tCIDLib::TBoolean bContinue = kCIDLib::True;

    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        //
        //  Create the bitmap for the scrollable content. We just make it the size
        //  of the overlay content. This may be a little wasteful in some cases
        //  but it's not worth making it more complex.
        //
        //  Note that the display area could be bigger in either axis, so we take
        //  the larger of the two.
        //
        TSize szScroll(m_areaContent.szArea());
        szScroll.TakeLarger(m_areaDisplay.szArea());
        m_bmpPanCont = TBitmap
        (
            szScroll, tCIDImage::EPixFmts::TrueAlpha, tCIDImage::EBitDepths::Eight
        );

        // Draw the content into the bitmap
        TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpPanCont);
        bDrawZContent(gdevCont);
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        // Remember the current scroll pos
        const TPoint pntOrgPos = m_areaAdjDisplay.pntOrg();

        // And now apply the delta to it and clip it back if needed
        m_areaAdjDisplay -= pntDelta;

        if (m_areaAdjDisplay.i4X() < m_areaContent.i4X())
            m_areaAdjDisplay.i4X(0);
        if (m_areaAdjDisplay.i4Y() < m_areaContent.i4Y())
            m_areaAdjDisplay.i4Y(0);

        if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);
        if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
            m_areaAdjDisplay.BottomJustifyIn(m_areaContent);

        //
        //  If we actually moved, then update. If not, and we are getting
        //  inertia now, then cancel the rest of the inertia.
        //
        if (pntOrgPos != m_areaAdjDisplay.pntOrg())
        {
            tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

            // Figure out the actual amount we moved
            TPoint pntMoved(pntOrgPos - m_areaAdjDisplay.pntOrg());

            //
            //  Clip to the display area plus the most restrictive clipping
            //  area of our ancestor containers.
            //
            TGUIRegion grgnClip;
            bFindMostRestrictiveClip(grgnClip, m_areaDisplay);
            TRegionJanitor janClip
            (
                cptrDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
            );

            TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpPanCont);

            // Move the window contents by the inverse of the new offset.
            TArea areaTmpTar;
            cptrDev->ScrollBits
            (
                m_areaDisplay
                , m_areaDisplay
                , pntMoved.i4X()
                , pntMoved.i4Y()
                , areaTmpTar
                , kCIDLib::True
            );

            // Calculate source/dest aras for the uncovered bits.
            TArea areaTmpSrc(areaTmpTar);

            if (pntMoved.i4X())
            {
                if (pntMoved.i4X() < 0)
                    areaTmpSrc.RightJustifyIn(m_areaAdjDisplay);
                else
                    areaTmpSrc.LeftJustifyIn(m_areaAdjDisplay);
                areaTmpSrc.i4Y(m_areaAdjDisplay.i4Y());
            }
             else
            {
                if (pntMoved.i4Y() < 0)
                    areaTmpSrc.BottomJustifyIn(m_areaAdjDisplay);
                else
                    areaTmpSrc.TopJustifyIn(m_areaAdjDisplay);
                areaTmpSrc.i4X(m_areaAdjDisplay.i4X());
            }

            // And blit the uncovered area
            cptrDev->CopyBits
            (
                gdevSrc
                , areaTmpSrc
                , areaTmpTar
                , tCIDGraphDev::EBmpModes::SrcCopy
                , kCIDLib::False
            );
        }
         else if (eEv == tCIDCtrls::EGestEvs::Inertia)
        {
            bContinue = kCIDLib::False;
        }

        // If an end, then we can clean up and do a final redraw
        if (eEv == tCIDCtrls::EGestEvs::End)
        {
            // Release the bitmap memory we used
            m_bmpPanCont.Reset();

            //
            //  Fake a call to our own area changed, so that we force all
            //  of the chlid widgets to adjust to the final scroll offset.
            //
            AreaChanged(areaActual(), areaActual());

            // Do a final draw in the end position
            Redraw();

            // And post an OnScroll
            PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::NonNested);
        }
    }
    return bContinue;
}


// Copy (any applicable) attributes from the passed widget
tCIDLib::TVoid
TCQCIntfOverlay::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfOverlay&>(iwdgSrc);
}


//
//  Note that some of our commands are really configured on the interface
//  viewer target, but are mapped to use at runtime (we do it during our
//  init call below.) So we don't report we support them, but we handle
//  them.
//
tCQCKit::ECmdRes
TCQCIntfOverlay::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                        , const tCIDLib::TCard4     c4Index
                        , const TString&            strUserId
                        , const TString&            strEventId
                        ,       TStdVarsTar&        ctarGlobals
                        ,       tCIDLib::TBoolean&  bResult
                        ,       TCQCActEngine&      acteTar)
{
    tCQCKit::ECmdRes eRes = tCQCKit::ECmdRes::Ok;

    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_DynOverLoad)
    {
        DynOverLoad
        (
            ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
            , ccfgToDo.piAt(2).m_strValue
            , ccfgToDo.piAt(3).m_strValue.bCompareI(L"Horizontal")
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_GetOverlayParam)
    {
        // Make sure the index is valid
        const TString& strIndex = ccfgToDo.piAt(1).m_strValue;
        tCIDLib::TCard4 c4Index;
        if (!strIndex.bToCard4(c4Index, tCIDLib::ERadices::Auto))
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcOvrly_BadParamInd
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Index)
            );
        }

        // Bump down the value to an index, since we get 1 to 4
        c4Index--;

        // Get the target variable and set it with the requested value
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(m_colParams[c4Index]) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadAncestor)
    {
        //
        //  If we are in an OnTimeout, we only will allow this if in digital
        //  signage mode, since it is non-interactive, so there's no chance we will
        //  whack something that the user is interacting with.
        //
        if ((strEventId != kCQCKit::strEvId_OnTimeout)
        ||  tCIDLib::bAllBitsOn(civOwner().miahOwner().eAppFlags(), tCQCIntfEng::EAppFlags::Signage))
        {
            TString strToLoad;
            piwdgParent()->ExpandTmplPath(ccfgToDo.piAt(0).m_strValue, strToLoad);

            // These are queued up async
            TAncOverLoad* padcbInfo = new TAncOverLoad
            (
                &civOwner(), *this, ccfgToDo.piAt(1).m_strValue, strToLoad
            );
            civOwner().miahOwner().AsyncDataCallback(padcbInfo);
            eRes = tCQCKit::ECmdRes::Stop;
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadOverlay)
    {
        //
        //  If we are in an OnTimeout, we only will allow this if in digital
        //  signage mode, since it is non-interactive, so there's no chance we will
        //  whack something that the user is interacting with.
        //
        if ((strEventId != kCQCKit::strEvId_OnTimeout)
        ||  tCIDLib::bAllBitsOn(civOwner().miahOwner().eAppFlags(), tCQCIntfEng::EAppFlags::Signage))
        {
            //
            //  Give any widgets a change to pre-cleanup before we destroy them.
            //  Some will take advantage of this.
            //
            TermWidgets();

            tCQCIntfEng::TErrList colErrs;
            bResult = bLoadNewTemplate(ccfgToDo.piAt(0).m_strValue, colErrs);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadSiblingOverlay)
    {
        // Ignore this for an ontimeout
        if (strEventId != kCQCKit::strEvId_OnTimeout)
        {
            //
            //  Expand the template path, in case it's relative. This is always
            //  done relative to the template loaded into the original parent
            //  overlay.
            //
            TString strToLoad;
            piwdgParent()->ExpandTmplPath(ccfgToDo.piAt(1).m_strValue, strToLoad);

            //
            //  If the target container path/name starts with ., then it's a path.
            //  Else, it's the name of a child of this overlay.
            //
            const TString& strTarPath = ccfgToDo.piAt(0).m_strValue;
            TCQCIntfContainer* piwdgTar = 0;
            if (strTarPath.bStartsWith(L"."))
            {
                tCIDLib::TBoolean   bIsAncestor;
                TCQCIntfContainer*  piwdgTar;

                const tCIDLib::TBoolean bRes = facCQCIntfEng().bFindTarContainer
                (
                    *this, strTarPath, bIsAncestor, piwdgTar
                );

                // If it failed or the target is not an overlay, then throw
                if (!bRes || !piwdgTar || !piwdgTar->bIsA(TCQCIntfOverlay::clsThis()))
                {
                    facCQCIntfEng().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcTmpl_BadContPath
                        , strTarPath
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppError
                    );
                }

                // If it's an ancestor of ours, then reject it
                if (bIsAncestor)
                {
                    facCQCIntfEng().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcTmpl_UseLoadAncestor
                        , strTarPath
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppError
                    );
                }

                tCQCIntfEng::TErrList colErrs;
                TCQCIntfOverlay* piwdgOver = static_cast<TCQCIntfOverlay*>(piwdgTar);

                //
                //  Give any widgets a change to pre-cleanup before we destroy them.
                //  Some will take advantage of this.
                //
                piwdgOver->TermWidgets();
                bResult = piwdgOver->bLoadNewTemplate(strToLoad, colErrs);
            }
             else
            {
                //
                //  We just call our parent to ask him to do this. Make sure that the
                //  overlay name is not our name.
                //
                if (strTarPath == strWidgetId())
                {
                    facCQCIntfEng().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kIEngErrs::errcTmpl_SelfTarget
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppError
                        , kCQCIntfEng::strCmdId_LoadSiblingOverlay
                    );
                }

                piwdgTar = piwdgParent();
                CIDAssert(piwdgTar != 0, L"The overlay has no parent container");

                tCQCIntfEng::TErrList colErrs;
                bResult = piwdgTar->bLoadSiblingOverlay
                (
                    ccfgToDo.piAt(0).m_strValue, strToLoad, colErrs
                );
            }
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_RunTmplAction)
    {
        const TString& strTitle = ccfgToDo.piAt(0).m_strValue;

        //
        //  Look up the list of template level actions and see if the
        //  indicated one is present. If so, invoke it as a nested action.
        //
        const tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            if (m_colActions[c4Index].strTitle() == strTitle)
                break;
            c4Index++;
        }

        // If we found it, invoke it
        if (c4Index < c4Count)
        {
            civOwner().PostOps
            (
                *this
                , m_colActions[c4Index]
                , kCQCKit::strEvId_OnTrigger
                , iwdgRootTemplate().c4HandlerId()
                , tCQCKit::EActOrders::Nested
            );
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_ScrollParent)
    {
        // Pass on to the view to be asynchronously done
        TScrollOverADCB* padcbInfo = new TScrollOverADCB
        (
            &civOwner()
            , *this
            , ccfgToDo.piAt(0).m_strValue
            , ccfgToDo.piAt(1).m_strValue
        );
        civOwner().miahOwner().AsyncDataCallback(padcbInfo);
    }
     else if ((ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_LoadDefTmpl)
          ||  (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetTimeout))
    {
        //
        //  If these get called, we just ignore them. They aren't meaningful
        //  for an overlay, but there's no way to know when it's defined
        //  if the template will be loaded into an overlay.
        //
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_Scroll)
    {
        //
        //  Ignore in any non-drawable events, or if it's an ontimeout type of
        //  event.
        //
        if (bCanRedraw(strEventId) && (strEventId != kCQCKit::strEvId_OnTimeout))
        {
            // Just fake a flick
            const TString& strDir = ccfgToDo.piAt(0).m_strValue;

            // Pass in a fake start/end point at our center
            const TPoint    pntStart = areaActual().pntCenter();
            tCIDLib::EDirs  eDir = tCIDLib::EDirs::Count;
            TPoint          pntEnd(pntStart);

            if (strDir.bCompareI(L"Up"))
                eDir = tCIDLib::EDirs::Up;
            else if (strDir.bCompareI(L"Down"))
                eDir = tCIDLib::EDirs::Down;
            else if (strDir.bCompareI(L"Left"))
                eDir = tCIDLib::EDirs::Left;
            else if (strDir.bCompareI(L"Right"))
                eDir = tCIDLib::EDirs::Right;

            CIDAssert
            (
                eDir != tCIDLib::EDirs::Count, TString(L"Unknown scroll direction: ") + strDir
            );
            ProcessFlick(eDir, pntStart);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetInitTemplate)
    {
        //
        //  Normally, in a command called in OnPreload, we'd just store the
        //  new info for later use during the normal loading process. But, in
        //  this case, we can't do it. The recursive loading process that
        //  happens when the template is loaded happens before this is called,
        //  so we have to load and initialize the contents here when the
        //  call is made, else we'd never get this loaded.
        //
        //  So we have to do a recursive load of this template to get all
        //  the hiearchy under us set up.
        //
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));

        // And we need an data server client for this
        TDataSrvClient dsclLoad;

        // We need to access our owning view a few times below so get it out
        TCQCIntfView& civMyOwner = civOwner();

        //
        //  Kick off the recursive load of the widgets. Tell this guy
        //  to throw on error since we don't want popups during commands
        //  if we can avoid it. We get back the area of the template we loaded,
        //  which we put into our m_areaContents member for later use during
        //  scrolling.
        //
        //  This guy will expand the path if needed, but we need it below for
        //  the init call, so just do it ourself in this case.
        //
        TString strExpPath;
        piwdgParent()->ExpandTmplPath(ccfgToDo.piAt(0).m_strValue, strExpPath);
        bLoadNew(&civMyOwner, dsclLoad, cfcData, strExpPath, kCIDLib::True);

        // The load worked, so initialize all the widgets
        tCQCIntfEng::TErrList colErrs;
        InitOverlay
        (
            &civMyOwner, strExpPath, facCQCIntfEng().polleThis(), dsclLoad, colErrs
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetOverlayParam)
    {
        // Make sure the index is valid
        const TString& strIndex = ccfgToDo.piAt(1).m_strValue;
        tCIDLib::TCard4 c4Index;
        if (!strIndex.bToCard4(c4Index, tCIDLib::ERadices::Auto))
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcOvrly_BadParamInd
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Index
                , TCardinal(c4Index)
            );
        }

        // Bump down the value to an index, since we get 1 to 4
        c4Index--;

        // And set the value
        m_colParams[c4Index] = ccfgToDo.piAt(0).m_strValue;
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetParentState)
    {
        const tCIDLib::TBoolean bRedraw(strEventId != kCQCKit::strEvId_OnPreload);

        if (ccfgToDo.piAt(0).m_strValue == L"Hidden")
            SetDisplayState(tCQCIntfEng::EDispStates::Hidden, bRedraw);
        else if (ccfgToDo.piAt(0).m_strValue == L"Disabled")
            SetDisplayState(tCQCIntfEng::EDispStates::Disabled, bRedraw);
         else
            SetDisplayState(tCQCIntfEng::EDispStates::Normal, bRedraw);
    }
     else
    {
        // Not one of ours, so let our parent try it
        eRes = TParent::eDoCmd
        (
            ccfgToDo
            , c4Index
            , strUserId
            , strEventId
            , ctarGlobals
            , bResult
            , acteTar
        );
    }
    return eRes;
}


tCIDLib::TVoid
TCQCIntfOverlay::GestInertiaIdle(const  tCIDLib::TEncodedTime   enctEnd
                                , const tCIDLib::TCard4         c4Millis
                                , const tCIDLib::EDirs          eDir)
{

}


tCIDLib::TVoid
TCQCIntfOverlay::Initialize(TCQCIntfContainer* const    piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Load any background image we have. The image is handled by the
    //  image mixin, so we just delegate to it.
    //
    InitImage(civOwner(), dsclInit, colErrs, *this);

    // Do an initial size calculation. Pass a bogus old area so it'll update
    DoSizeCalcs(areaActual(), TArea::areaEmpty);
}


//
//  Gen up one of our runtime value objects. We calculate the currently
//  displayed page index and send that as a RTV. It really only makes
//  sense in paged mode or in manual flick mode. For manual flick mode we
//  store the last flick direction seen, so that we can pick it up here.
//
TCQCCmdRTVSrc*
TCQCIntfOverlay::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    tCIDLib::TCard4 c4HPageInd = 0;
    tCIDLib::TCard4 c4VPageInd = 0;

    if (m_areaDisplay.c4Width() && m_areaAdjDisplay.i4X())
        c4HPageInd = tCIDLib::TCard4(m_areaAdjDisplay.i4X()) / m_areaDisplay.c4Width();

    if (m_areaDisplay.c4Height() && m_areaAdjDisplay.i4Y())
        c4VPageInd = tCIDLib::TCard4(m_areaAdjDisplay.i4Y()) / m_areaDisplay.c4Height();

    TCQCIntfOverlayRTV* pcrtsRet = new TCQCIntfOverlayRTV(c4HPageInd, c4VPageInd, cuctxToUse);

    // If in manual flick mode, set the stored flick direction
    if (m_bManualFlick)
        pcrtsRet->SetFlickDir(m_eLastFlickDir);

    return pcrtsRet;
}


//
//  Called for post init loading.
//
//  KEEP IN MIND that this will be called each time a new template is loaded
//  into us, because that requires kicking off the whole initialization
//  sequence. So don't do anything here that should only be done once, or
//  at least insure it only happens once if so.
//
tCIDLib::TVoid TCQCIntfOverlay::PostInit()
{
    // Call our parent first
    TParent::PostInit();

    //
    //  Now that the image stuff is initialized, set up pre-scaling on our image.
    //  The size will end up getting changed again likely as we are sized to our
    //  final size, but just in case we need to set it up now.
    //
    SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), areaActual().szArea());

    // Do an initial scroll event
    PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::NonNested);
}



//
//  We have to process flicks for page mode or non-inertial mode, or for RIVA
//  RIVA clients. Our start gesture handler will tell the gesture engine if we
//  can only accept flicks.
//
//  The other scenario is when we are marked as 'manual flick'. This allows
//  users to react to flicks on their own. We will just queue up an OnFlick event
//  for them to process.
//
tCIDLib::TVoid
TCQCIntfOverlay::ProcessFlick(  const   tCIDLib::EDirs  eDir
                                , const TPoint&         pntStartPos)
{
    // If in manual flick mode, just post an event and return
    if (m_bManualFlick)
    {
        //
        //  Store the direction so that we runtime value query that comes after will
        //  be able to store it.
        //
        m_eLastFlickDir = eDir;

        PostOps(*this, kCQCKit::strEvId_OnFlick, tCQCKit::EActOrders::NonNested);
        return;
    }

    //
    //  Get the display area, zero origin it, and mvoe it forward to the
    //  current scroll position. This is the source area to blit from a
    //  zero origined content image.
    //
    TArea areaScroll(m_areaDisplay);
    areaScroll.ZeroOrg();
    areaScroll += m_areaAdjDisplay.pntOrg();

    tCIDLib::TInt4 i4Diff;
    switch(eDir)
    {
        case tCIDLib::EDirs::Down :
            //
            //  We need to grab content above our area, so see if the
            //  scroll area is not already at the top.
            //
            if (areaScroll.i4Top() > m_areaContent.i4Top())
            {
                // Move by up to a page's worth
                i4Diff = areaScroll.c4Height();
                if ((areaScroll.i4Y() - i4Diff) < 0)
                    i4Diff = areaScroll.i4Y();

                areaScroll.AdjustSize(0, i4Diff);
                areaScroll.AdjustOrg(0, -i4Diff);

                // Adjust our scroll offset
                m_areaAdjDisplay.AdjustOrg(0, -i4Diff);
            }
            break;

        case tCIDLib::EDirs::Left :
            //
            //  We need to grab content to the right of our area, so see if
            //  the scrolla area's right side is all the way to the right of
            //  the content area yet.
            //
            if (areaScroll.i4Right() < m_areaContent.i4Right())
            {
                // Move by up to a page's worth
                i4Diff = areaScroll.c4Width();
                if ((areaScroll.i4Right() + i4Diff) > m_areaContent.i4Right())
                    i4Diff = m_areaContent.i4Right() - areaScroll.i4Right();

                areaScroll.AdjustSize(i4Diff, 0);

                // Adjust our scroll offset
                m_areaAdjDisplay.AdjustOrg(i4Diff, 0);
            }
            break;

        case tCIDLib::EDirs::Right :
            //
            //  We need to grab content to the left of our area, so see
            //  if the scroll area is all the way to the left already.
            //
            if (areaScroll.i4Left() > m_areaContent.i4Left())
            {
                // Move by up to a page's worth
                i4Diff = areaScroll.c4Width();
                if ((areaScroll.i4X() - i4Diff) < 0)
                    i4Diff = areaScroll.i4X();

                areaScroll.AdjustSize(i4Diff, 0);
                areaScroll.AdjustOrg(-i4Diff, 0);

                // Adjust our scroll offset
                m_areaAdjDisplay.AdjustOrg(-i4Diff, 0);
            }
            break;

        case tCIDLib::EDirs::Up :
            //
            //  We need to grab content below our area, so see if we
            //  are at the bottom already.
            //
            if (areaScroll.i4Bottom() < m_areaContent.i4Bottom())
            {
                // Move by up to a page's worth
                i4Diff = areaScroll.c4Height();
                if ((areaScroll.i4Bottom() + i4Diff) > m_areaContent.i4Bottom())
                    i4Diff = m_areaContent.i4Bottom() - areaScroll.i4Bottom();

                areaScroll.AdjustSize(0, i4Diff);

                // Adjust our scroll offset
                m_areaAdjDisplay.AdjustOrg(0, i4Diff);
            }
            break;

        default :
            break;
    };

    // If not moving, we are dont
    if (eDir == tCIDLib::EDirs::Count)
        return;


    //
    //  If in remote mode or inertia is disabled, we just want to redraw
    //  now with the newly set scroll offset. Send a scroll event before
    //  we return.
    //
    if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
    {
        AreaChanged(areaActual(), areaActual());
        Redraw();

        PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::NonNested);
        return;
    }

    // Create a buffer image the size of our content
    TBitmap bmpContent
    (
        m_areaContent.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );
    TGraphMemDev gdevContent(civOwner().gdevCompat(), bmpContent);
    bDrawZContent(gdevContent);

    //
    //  Create an image the size of the scroll area we calculated and a
    //  zero origined version of it.
    //
    TArea areaZScroll(areaScroll);
    areaZScroll.ZeroOrg();
    TBitmap bmpScroll
    (
        areaZScroll.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );
    TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);

    //
    //  Now copy out the stuff we drew above. This gives us a zero based
    //  version of the scrollable content we need for the scroll operation.
    //
    gdevScroll.CopyBits(gdevContent, areaScroll, areaZScroll);

    //
    //  We have a special case here since we are doing this scroll. We need
    //  to make sure our slot area is clipped to within all our parent's
    //  areas, since the content of the template we are in may be bigger
    //  than the overlay we are in.
    //
    TGUIRegion grgnClip;
    if (!bFindMostRestrictiveClip(grgnClip, m_areaDisplay))
    {
        //
        //  We are completely clipped. Wierd since we shouldn't have seen
        //  the gesture, but anyway...
        //
        return;
    }

    // Get a device and do the scroll
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TRegionJanitor janClip
    (
        cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
    );

    TCIDEaseScroller escrSlide;
    escrSlide.Scroll2
    (
        *cptrGraphDev
        , m_areaDisplay
        , areaZScroll
        , gdevScroll
        , kCQCIntfEng_::c4EaseScrollMSs
        , eDir
        , kCQCIntfEng_::c4EaseOrder
    );

    // Do a final redraw
    Redraw(m_areaDisplay);

    PostOps(*this, kCQCKit::strEvId_OnScroll, tCQCKit::EActOrders::NonNested);
}


// Return our list of commands we support
tCIDLib::TVoid
TCQCIntfOverlay::QueryCmds(         TCollection<TCQCCmd>&   colToFill
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first
    TParent::QueryCmds(colToFill, eContext);

    // Don't allow loading the overlay in an IV event, or preload scenarios
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        TCQCCmd cmdDynOverLoad
        (
            kCQCIntfEng::strCmdId_DynOverLoad
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_DynOverLoad)
            , 4
        );

        cmdDynOverLoad.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
                , tCQCKit::ECmdPTypes::Template
            )
        );

        cmdDynOverLoad.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_FMList)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdDynOverLoad.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_NameList)
                , tCQCKit::ECmdPTypes::Text
            )
        );

        cmdDynOverLoad.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Direction)
                , TString(L"Horizontal, Vertical")
            )
        );

        colToFill.objAdd(cmdDynOverLoad);
    }

    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_LoadOverlay
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_LoadOverlay)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
            )
        );
    }

    // Don't allow loading the overlay in an IV event, or preload scenarios
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_Scroll
                , facCQCKit().strMsg(kKitMsgs::midCmd_Scroll)
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ScrollDir)
                , L"Up, Down, Left, Right"
            )
        );
    }

    // In preload, allow them to set an initial template to load
    if (eContext == tCQCKit::EActCmdCtx::Preload)
    {
        colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetInitTemplate
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetInitTemplate)
                , tCQCKit::ECmdPTypes::Template
                , facCQCKit().strMsg(kKitMsgs::midCmdP_TemplateName)
            )
        );
    }

    //
    //  Set one of this overlay's parameters.
    //
    {
        TCQCCmd cmdGetOverParam
        (
            kCQCIntfEng::strCmdId_SetOverlayParam
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetOverlayParam)
            , 2
        );

        cmdGetOverParam.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Value), tCQCKit::ECmdPTypes::Text
            )
        );

        cmdGetOverParam.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Number), TString(L"1, 2, 3, 4")
            )
        );

        colToFill.objAdd(cmdGetOverParam);
    }
}


tCIDLib::TVoid TCQCIntfOverlay::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       bIncludeScope) const
{
    // Call our parent to handle recursion on children
    TParent::QueryReferencedImgs
    (
        colToFill, colScopes, bIncludeSysImgs, bIncludeScope
    );

    // And call our image mixin
    MCQCIntfImgIntf::QueryImgPath(colToFill, bIncludeSysImgs);
}


//
//  We call our parent first, then add a few bits of our own.
//
//  NOTE: Hot keys and triggered events are not done here. We only get those at run
//        time from the templates loaded into us. We also get OnLoad, OnPreload and
//        so forth actions from the template. But we don have our own OnScroll
//        command at design time, so we do the action cmd mixin.
//
tCIDLib::TVoid
TCQCIntfOverlay::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove attributes we don't support
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Update the name sof those we keep if needed
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Update the limits of the widget id attribute, which is required in our case
    facCIDMData().SetAttrLimits
    (
        colAttrs, kCQCIntfEng::strAttr_Base_WidgetId, kCIDMData::strAttrLim_Required
    );


    //
    //  Do our mixins. NOTE THAT we do not allow them to edit hot keys and triggered
    //  events. These come at runtime from the template loaded into the overlay.
    //
    MCQCIntfImgIntf::QueryImgAttrs(colAttrs, adatTmp, kCIDLib::True);
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Actions"
    );

    // Add our own stuff
    colAttrs.objPlace
    (
        L"Overlay Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // This one we need to construct then add since we have to set two things
    adatTmp.Set
    (
        L"Init Load"
        , kCQCIntfEng::strAttr_Overlay_InitTemplate
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strInitTemplate);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_Template);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));

    colAttrs.objPlace
    (
        L"Manual Flick"
        , kCQCIntfEng::strAttr_Overlay_ManualFlick
        , tCIDMData::EAttrTypes::Bool
    ).SetBool(m_bManualFlick);

    colAttrs.objPlace
    (
        L"Page Mode"
        , kCQCIntfEng::strAttr_Overlay_PageMode
        , tCIDMData::EAttrTypes::Bool
    ).SetBool(m_bPageMode);

    // The overlay parameters
    colAttrs.objPlace
    (
        L"Parameter 1"
        , kCQCIntfEng::strAttr_Overlay_P1
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_colParams[0]);

    colAttrs.objPlace
    (
        L"Parameter 2"
        , kCQCIntfEng::strAttr_Overlay_P2
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_colParams[1]);

    colAttrs.objPlace
    (
        L"Parameter 3"
        , kCQCIntfEng::strAttr_Overlay_P3
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_colParams[2]);

    colAttrs.objPlace
    (
        L"Parameter 4"
        , kCQCIntfEng::strAttr_Overlay_P4
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_colParams[3]);
}



//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfOverlay::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Call our parent to do child widgets
    TParent::RefreshImages(dsclToUse);

    // And handle our background image if any
    MCQCIntfImgIntf::UpdateImage(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfOverlay::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFull
                        ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Template))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strInitTemplate, bRegEx, bFull, regxFind
        );
    }

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfOverlay::szDefaultSize() const
{
    // Return a default size
    return TSize(128, 128);
}


//
//  Update ourself with the new attribute. Note that hot keys and triggered events are runtime
//  only and come from the loaded template, so we never get them during editing.
//
tCIDLib::TVoid
TCQCIntfOverlay::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle the mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfImgIntf::SetImgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_InitTemplate)
        m_strInitTemplate = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_ManualFlick)
        m_bManualFlick = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_PageMode)
        m_bPageMode = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_P1)
        m_colParams[0] = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_P2)
        m_colParams[1] = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_P3)
        m_colParams[2] = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Overlay_P4)
        m_colParams[3] = adatNew.strValue();
}


//
//  When the designer lets the user rename a widget, it needs to update any
//  commands that target that widget id. So we have to override this in order
//  to handle any actions taht only exist at our level. The container parent
//  class will handle any that are done via our command source mixin
//  interface, and all of our child widgets.
//
tCIDLib::TVoid
TCQCIntfOverlay::UpdateCmdTarget(const  TString&                    strOldTargetId
                                , const TString&                    strNewTargetName
                                , const TString&                    strNewTargetId
                                ,       tCQCIntfEng::TDesIdList&    fcolAffected)
{
    CIDAssert
    (
        TFacCQCIntfEng::bDesMode()
        , L"UpdateCmdTarget should only be called at design time"
    );

    // Do any template level actions
    tCIDLib::TBoolean bChanged = kCIDLib::False;
    tCIDLib::TCard4 c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        bChanged |= m_colActions[c4Index].bUpdateCmdTarget
        (
            strOldTargetId, strNewTargetName, strNewTargetId
        );
    }

    //
    //  We only have hot keys if the template loaded into us does. Since we
    //  use the enum and not the element count, we have to check this first.
    //
    if (!m_colHotKeyActions.bIsEmpty())
    {
        tCIDLib::ForEachE<tCQCIntfEng::EHotKeys>
        (
            [&, this](const auto eKey)
            {
                if (m_colHotKeyActions[eKey].bUsed())
                {
                    bChanged |= m_colHotKeyActions[eKey].bUpdateCmdTarget
                    (
                        strOldTargetId, strNewTargetName, strNewTargetId
                    );
                }
            }
        );
    }

    //
    //  Add oursef to the list of affected widgets if we changed anything and are
    //  not already in the list.
    //
    if (bChanged)
    {
        if (!fcolAffected.bElementPresent(c8DesId()))
            fcolAffected.c4AddElement(c8DesId());
    }

    //
    //  Now call our parent class to handle any child widgets and our own
    //  regular commands.
    //
    TParent::UpdateCmdTarget(strOldTargetId, strNewTargetName, strNewTargetId, fcolAffected);
}


// Handle validating the info at our level
tCIDLib::TVoid
TCQCIntfOverlay::Validate(  const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImage(colErrs, dsclVal, *this);

    // Do various checks on the initial template path if not empty
    if (!m_strInitTemplate.bIsEmpty())
    {
        // Relative paths are valid here
        TString strErr;
        if (!facCQCKit().bIsValidRemTypeRelPath(m_strInitTemplate, strErr, kCIDLib::True))
        {
            colErrs.objPlace
            (
                c4UniqueId()
                , facCQCIntfEng().strMsg
                  (
                    kIEngErrs::errcVal_BadPath, TString(L"template"), strErr
                  )
                , kCIDLib::True
                , strWidgetId()
            );
        }
         else
        {
            // It looks reasonable, so expand it if needed
            TString strExpPath;
            piwdgParent()->ExpandTmplPath(m_strInitTemplate, strExpPath);

            if (!dsclVal.bFileExists(strExpPath, tCQCRemBrws::EDTypes::Template))
            {
                // Put an error in the list
                strErr.LoadFromMsg
                (
                    kIEngErrs::errcVal_InitOvrNotFound, facCQCIntfEng(), m_strInitTemplate
                );
                colErrs.objPlace(c4UniqueId(), strErr, kCIDLib::True, strWidgetId());
            }

            //
            //  Get the path of our parent template. Make sure they aren't trying to load us
            //  into ourself.
            //
            if (civOwner().strBaseTmplName().bCompareI(strExpPath))
            {
                colErrs.objPlace
                (
                    TCQCIntfValErrInfo(c4UniqueId()
                    , facCQCIntfEng().strMsg(kIEngErrs::errcTmpl_CircularRef)
                    , kCIDLib::False
                    , strWidgetId())
                );
            }
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is part of the recursive load sequence. This is not what you'd call
//  to load a new template from the outside, i.e. from the LoadOverlay action
//  above. This is called from the RecursiveLoad() method in our parent
//  (container) class. So the parent template loads, and he calls recursive
//  load to handle any nested overlays. So that calls us here. We do the basic
//  loading of the template into ourself, then we call RecursiveLoad again
//  to continue the process.
//
//  So this is just getting the stuff initially into memory during the load
//  process.
//
//  Since we are loading to a temporary, we can be more efficient and move
//  some stuff into ourself instead of copying.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bLoadNew(          TCQCIntfView* const     pcivOwner
                            ,       TDataSrvClient&         dsclLoad
                            , const TCQCFldCache&           cfcData
                            , const TString&                strToLoad
                            , const tCIDLib::TBoolean       bThrowOnFail)
{
    TCQCIntfTemplate iwdgLoaded;

    try
    {
        // It may be relative
        TString strFullPath;
        piwdgParent()->ExpandTmplPath(strToLoad, strFullPath);
        facCQCIntfEng().QueryTemplate
        (
            strFullPath
            , dsclLoad
            , iwdgLoaded
            , pcivOwner->cuctxToUse()
            , pcivOwner->bEnableTmplCaching()
        );

        // Store the full path on us
        strTemplateName(strFullPath);
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // If asked to throw on fail, then do that, else do a popup
        if (bThrowOnFail)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }

        pcivOwner->ShowErr
        (
            civOwner().strTitle()
            , facCQCIntfEng().strMsg(kIEngErrs::errcTmpl_LoadFailed, strToLoad)
            , errToCatch
        );
        return kCIDLib::False;
    }

    // Set up our content area to the zero origined area of the loaded widget
    m_areaContent = iwdgLoaded.areaActual();
    m_areaContent.ZeroOrg();

    //
    //  And now steal his children, states, actions, etc... The last parm tells
    //  him to update all the widgets' parent origin to our origin. He will also
    //  set the view on all of the widgets taken. This will actually happen again
    //  in the recursive load method, but we go ahead and set them now in case we
    //  need to anything below that would require it, which is probably likely.
    //
    TakeFrom(iwdgLoaded, kCIDLib::True);

    // Move some stuff of his to our lists.
    m_colActions = tCIDLib::ForceMove(iwdgLoaded.colActions());
    m_colHotKeyActions = tCIDLib::ForceMove(iwdgLoaded.colHotKeyActions());
    m_colTrgEvents = tCIDLib::ForceMove(iwdgLoaded.colTrgEvents());

    //
    //  If we didn't originally have an explicitly set image, then take
    //  whatever is set on the template, which may be nothing.
    //
    if (!m_bHadImage)
        MCQCIntfImgIntf::operator=(iwdgLoaded);

    //
    //  Copy over his commands to us so that we can invoke them after all the loading
    //  is done. We cannot invoke them here both because we don't even know if the
    //  load is going to succeed, and because the widgets aren't set up yet. They'll
    //  get run later.
    //
    //  We have to retain our own OnScroll and OnFlick commands across this, and we
    //  also check for an OnClick which the Live Tile derivative uses. So save the
    //  opcodes, and then add the event back and set the opcodes on them.
    //
    TOpcodeBlock colOnFlick;
    bQueryOpsForEvent(kCQCKit::strEvId_OnFlick, colOnFlick);
    TOpcodeBlock colOnScroll;
    bQueryOpsForEvent(kCQCKit::strEvId_OnScroll, colOnScroll);

    TOpcodeBlock colOnClick;
    const tCIDLib::TBoolean bOnClick = bQueryOpsForEvent
    (
        kCQCKit::strEvId_OnClick, colOnClick
    );

    // Now update our version of this command information
    MCQCCmdSrcIntf::operator=(iwdgLoaded);

    // And restore commands that we saved
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnFlick)
        , kCQCKit::strEvId_OnFlick
        , tCQCKit::EActCmdCtx::NoContChange
    );
    SetEventOps(kCQCKit::strEvId_OnFlick, colOnFlick);

    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
        , kCQCKit::strEvId_OnScroll
        , tCQCKit::EActCmdCtx::NoContChange
    );
    SetEventOps(kCQCKit::strEvId_OnScroll, colOnScroll);

    if (bOnClick)
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnClick)
            , kCQCKit::strEvId_OnClick
            , tCQCKit::EActCmdCtx::Standard
        );
        SetEventOps(kCQCKit::strEvId_OnClick, colOnClick);
    }

    //
    //  Now remove any recursive links. These are overlays that have initial
    //  loads that load a template currently in our parent chain, or overlay
    //  buttons that would do the same.
    //
    //  Note that by doing this here before we do the next step, we avoid
    //  loading up parts of the tree that we would just end up pruning back
    //  out anyway.
    //
    RemoveRecursiveLinks();

    //
    //  If in viewer mode, then swizzle the commands of any of the children
    //  we reloaded, since some of their commands need to be modified to
    //  change the target to us (from the interface viewer.)
    //
    if (!TFacCQCIntfEng::bDesMode())
        SwizzleCmds();

    //
    //  Call the recursive loader method to continue the recursive loading
    //  process. If it hits other overlays that are configured to initially
    //  load a template, it'll recurse back on us here and so forth. Note
    //  that this guy handles recursive links so we don't have to worry
    //  about that.
    //
    RecursiveLoad(pcivOwner, dsclLoad, cfcData);

    return kCIDLib::True;
}


//
//  This is the starting point for loading a new template into us. This is invoked
//  initially and when the user sends a command to load a new template.
//
//  Within here we invoke a recursive process to handle any nested overlays that
//  need to load an so forth. But it doesn't recurse back to here, so we can do
//  any initial setup and any post-load work.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bLoadNewTemplate(const TString&                strTmplName
                                    ,   tCQCIntfEng::TErrList&  colErrs)
{
    try
    {
        TCQCIntfTimerJan janTimers(&civOwner());

        //
        //  Give any widgets a change to pre-cleanup before we destroy them.
        //  Some will take advantage of this.
        //
        TermWidgets();

        // We need a field info cache to initialize the widgtes
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));

        // And we need a data server client for this
        TDataSrvClient dsclLoad;

        // We need to access our owning view a few times below so get it out
        TCQCIntfView& civMyOwner = civOwner();

        //
        //  Tell our containing viewer window that we are about to replace
        //  our contents, so that he can deal with any references he
        //  might have had to our descendants.
        //
        civMyOwner.StartBranchReplace(*this);

        //
        //  Kick off the recursive load of the widgets. Tell this guy to throw
        //  an error since we don't want popups during commands if we can avoid
        //  it.
        //
        //  This guy will expand the path if needed. But we need it for the init
        //  call below, so we do it ourself and pass in the expanded path so he
        //  doesn't do it again redundantly.
        //
        //  NOTE this also releases any previously contained widgets correctly as
        //  well.
        //
        TString strExpPath;
        piwdgParent()->ExpandTmplPath(strTmplName, strExpPath);
        bLoadNew(&civMyOwner, dsclLoad, cfcData, strExpPath, kCIDLib::True);

        // The load worked, so initialize all the widgets we loaded into us
        InitOverlay
        (
            &civMyOwner, strExpPath, facCQCIntfEng().polleThis(), dsclLoad, colErrs
        );

        // Kick off any preloads if not in designer mode
        if (!TFacCQCIntfEng::bDesMode())
            DoPreloads(civMyOwner, TString::strEmpty());

        //
        //  If not in designer mode, then get all of the widgets under us to
        //  register their fields.
        //
        if (!TFacCQCIntfEng::bDesMode())
            RegisterFields(facCQCIntfEng().polleThis(), cfcData);

        //
        //  Pause slightly to give the polling engine some time to get any
        //  new fields polled. DO NOT msg yield here, since that would
        //  allow the update timer to kick in and cause all kind of nasty
        //  ragged redraw problems, if not flat out errors.
        //
        TThread::Sleep(150);

        //
        //  Force an initial field update so that we can at least get the values
        //  of any fields that are in the polling engine so far, and have the
        //  widgets that access those fields valid before we even show up. We tell
        //  them not to redraw, so that we can just update all at once afterwards.
        //  No need to do this if in designer mode.
        //
        if (!TFacCQCIntfEng::bDesMode())
            InitialScan(facCQCIntfEng().polleThis(), civMyOwner.ctarGlobalVars());

        // Do post init on all templates that want it
        DoPostInits();

        // And force a redraw with as much status as we have
        Redraw(areaActual());

        // And do OnLoads now that we are fully setup and displayed
        DoOnLoads(civMyOwner);
    }

    catch(TError& errToCatch)
    {
        // Put our branch of tree back to ground zero and redraw
        ResetOverlay();
        Invalidate();

        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcTmpl_CantLoad
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strTmplName
            , errToCatch.strErrText()
        );
    }

    //
    //  If we got any validation errors. Not sure if we need to do aything here.
    //  Validation is design time and overlayd don't get loaded at design time.
    //
    if (!colErrs.bIsEmpty())
    {
        // <TBD>
    }

    return kCIDLib::True;
}


// Get or set the manual flick setting
tCIDLib::TBoolean TCQCIntfOverlay::bManualFlick() const
{
    return m_bManualFlick;
}

tCIDLib::TBoolean TCQCIntfOverlay::bManualFlick(const tCIDLib::TBoolean bToSet)
{
    m_bManualFlick = bToSet;
    return m_bManualFlick;
}


// Get or set the page mode setting
tCIDLib::TBoolean TCQCIntfOverlay::bPageMode() const
{
    return m_bPageMode;
}

tCIDLib::TBoolean TCQCIntfOverlay::bPageMode(const tCIDLib::TBoolean bToSet)
{
    m_bPageMode = bToSet;
    return m_bPageMode;
}



tCIDLib::TVoid
TCQCIntfOverlay::InitOverlay(       TCQCIntfView* const         pcivOwner
                            , const TString&                    strName
                            ,       TCQCPollEngine&             polleToUse
                            ,       TDataSrvClient&             dsclInit
                            ,       tCQCIntfEng::TErrList&      colErrs)
{
    //
    //  Kick off the recursive initialization of all the widgets we
    //  loaded. Pass our parent as the owning widget, since the one we
    //  pass here will get set as our parent. Subsequent recursion will
    //  pass us as the parent of the new widgets.
    //
    Initialize(piwdgParent(), dsclInit, colErrs);

    //
    //  Tell our containing viewer window that we've replaced our
    //  contents, so that he can update any 'mouse over' or or focus or
    //  other stuff that needs to be dealt with.
    //
    pcivOwner->EndBranchReplace(*this);
}



const TString& TCQCIntfOverlay::strCurTemplate() const
{
    return m_strCurTemplate;
}


const TString& TCQCIntfOverlay::strInitTemplate() const
{
    return m_strInitTemplate;
}

const TString& TCQCIntfOverlay::strInitTemplate(const TString& strToSet)
{
    m_strInitTemplate = strToSet;
    return m_strInitTemplate;
}


// Get or set the preload parameters
const TString& TCQCIntfOverlay::strParam(const tCIDLib::TCard4 c4Index) const
{
    return m_colParams[c4Index];
}

tCIDLib::TVoid
TCQCIntfOverlay::SetParam(const tCIDLib::TCard4 c4Index, const TString& strToSet)
{
    m_colParams[c4Index] = strToSet;
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  The container class, when it's position is changed, will update the
//  positions of its contained child widgets. It will call us here for each
//  of them, and we can adjust them. We use this to support scrolling. We
//  have to lie to the widgets and get them to draw such that the correct
//  current part of the contained content shows up in our display area when
//  we update.
//
//  We just apply our scroll offset to the point, plus the display area
//  origin, to adjust for the fact that the display area (to which we
//  position children) isn't necessarily at our origin.
//
tCIDLib::TVoid
TCQCIntfOverlay::AdjustChildPos(const TPoint& pntNew, TPoint& pntAdj)
{
    pntAdj = pntNew;
    pntAdj -= m_areaAdjDisplay.pntOrg();
}


//
//  We just call our parent and then update our display area.
//
//  WE HAVE TO do our size calls first, becasue our parent is going to call the
//  AdjustChildPos() callback above, which will use the adjusted display area, so it
//  has to be updated first.
//
tCIDLib::TVoid
TCQCIntfOverlay::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    DoSizeCalcs(areaNew, areaOld);

    // Gotta call our parent so that children are resized if needed
    TParent::AreaChanged(areaNew, areaOld);

    // Update the pre-scaling on our image mixin if the size changed
    if (!areaNew.bSameSize(areaOld))
        SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), areaNew.szArea());
}


// Invoke any OnExit action if we have one configured
tCIDLib::TVoid TCQCIntfOverlay::DoOnExit(TCQCIntfView& civOwner)
{
    if (c4OpCount(kCQCKit::strEvId_OnExit))
    {
        tCQCKit::TCmdTarList colWdgTars(tCIDLib::EAdoptOpts::NoAdopt);
        FindAllTargets(colWdgTars, kCIDLib::False);

        // All we expose is the global variables target

        // Now create the engine and run it
        TCQCStdActEngine actePreload(civOwner.cuctxToUse());
        const tCQCKit::ECmdRes eRes = actePreload.eInvokeOps
        (
            *this
            , kCQCKit::strEvId_OnPreload
            , civOwner.ctarGlobalVars()
            , colWdgTars
            , civOwner.pmcmdtTrace()
            , TString::strEmpty()
        );

        if (eRes > tCQCKit::ECmdRes::Exit)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_PreloadFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strTemplateName()
            );
        }

    }
}


// Invoke any OnLoad action if we have one configured
tCIDLib::TVoid TCQCIntfOverlay::DoOnLoad(TCQCIntfView& civOwner)
{
    if (c4OpCount(kCQCKit::strEvId_OnLoad))
        PostOps(*this, kCQCKit::strEvId_OnLoad, tCQCKit::EActOrders::NonNested);
}


// Invoke any OnPreload action if we have one configured
tCIDLib::TVoid
TCQCIntfOverlay::DoOnPreload(       TCQCIntfView&   civOwner
                            , const TString&        )
{
    //
    //  NOTE: It's possible for us to get called here before any template
    //  is loaded into an overlay, hence there will be no events and
    //  we might get a null pointer back. If they set no initial template,
    //  nothing will have been set.
    //
    TOpcodeBlock* pcolPre = pcolOpsForEvent(kCQCKit::strEvId_OnPreload);
    if (pcolPre && !pcolPre->bIsEmpty())
    {
        //
        //  Looks ok. Build up a list of non-standard targets, which
        //  will be all our children that implement the command target
        //  mixin.
        //
        tCQCKit::TCmdTarList colWdgTars(tCIDLib::EAdoptOpts::NoAdopt);
        FindAllTargets(colWdgTars, kCIDLib::False);

        // Add ourself
        colWdgTars.Add(this);

        // And the view
        colWdgTars.Add(&civOwner);

        // Now create the engine and run it
        TCQCStdActEngine actePreload(civOwner.cuctxToUse());
        const tCQCKit::ECmdRes eRes = actePreload.eInvokeOps
        (
            *this
            , kCQCKit::strEvId_OnPreload
            , civOwner.ctarGlobalVars()
            , colWdgTars
            , civOwner.pmcmdtTrace()
            , TString::strEmpty()
        );

        if (eRes > tCQCKit::ECmdRes::Exit)
        {
            facCQCIntfEng().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIEngErrs::errcTmpl_PreloadFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strTemplateName()
            );
        }
    }
}


//
//  We just call our parent and then do any size calcs.
//
tCIDLib::TVoid
TCQCIntfOverlay::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    // Gotta call our parent so that children are resized if needed
    TParent::SizeChanged(szNew, szOld);

    // Redo size oriented calculations
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    DoSizeCalcs(areaNew, areaOld);

    // Update the pre-scaling on our image mixin
    SetImgPrescale(civOwner(), !TFacCQCIntfEng::bRemoteMode(), szNew);
}


// Stream us in from the passed stream
tCIDLib::TVoid TCQCIntfOverlay::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Overlay::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Call our parent. This will clean out any existing child widgets
    TParent::StreamFrom(strmToReadFrom);

    //
    //  And handle any mixins. We only started streaming the cmd src mixin
    //  as of V5. Else, reset it.
    //
    MCQCIntfImgIntf::ReadInImage(strmToReadFrom);
    if (c2FmtVersion < 5)
        ResetAllOps(kCIDLib::False);
    else
        MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Stream in our own stuff now
    strmToReadFrom >> m_strInitTemplate;

    // Convert the path if needed
    if (c2FmtVersion < 9)
        facCQCKit().Make50Path(m_strInitTemplate);

    // If less than version 4, then default the page mode flag else read it in.
    if (c2FmtVersion < 4)
        m_bPageMode = kCIDLib::False;
    else
        strmToReadFrom  >> m_bPageMode;

    //
    //  If V4 or V5 read in and discard the abortive percentage scrolling values
    //  which we got rid of.
    //
    if ((c2FmtVersion == 5) || (c2FmtVersion == 4))
    {
        tCIDLib::TCard4 c4Dummy;
        strmToReadFrom  >> c4Dummy
                        >> c4Dummy;
    }

    // If less than 8, then default the parameters, else read read them in
    if (c2FmtVersion < 8)
    {
        m_colParams[0].Clear();
        m_colParams[1].Clear();
        m_colParams[2].Clear();
        m_colParams[3].Clear();
    }
     else
    {
        strmToReadFrom >> m_colParams;
    }

    // If less than 10, then default the manual flick mode, else read it in
    if (c2FmtVersion < 10)
        m_bManualFlick = kCIDLib::False;
    else
        strmToReadFrom >> m_bManualFlick;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If not in designer mode, then force in the onpreload if we loaded
    //  a V1 overlay, since it won't have been added before.
    //
    if (!TFacCQCIntfEng::bDesMode() && (c2FmtVersion == 1))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPreload)
            , kCQCKit::strEvId_OnLoad
            , tCQCKit::EActCmdCtx::Preload
        );
    }

    // If less than version 3, move the initial template if any to the user area
    if ((c2FmtVersion < 3) && !m_strInitTemplate.bIsEmpty())
        m_strInitTemplate.Insert(kCQCKit::strRepoScope_UserTS, 0);

    //
    //  If less than V7, then move F2 to F3, to make our use of border color
    //  consistent with other widgets.
    //
    if (c2FmtVersion < 7)
        rgbFgn3(rgbFgn2());

    // Reset runtime stuff
    m_areaAdjDisplay.ZeroAll();
    m_areaContent.ZeroAll();
    m_areaDisplay.ZeroAll();
    m_colActions.RemoveAll();
    m_colTrgEvents.RemoveAll();
    m_strCurTemplate.Clear();

    //
    //  Remember at this point if we have an image, so that we can still
    //  know after loading a template if we originally had one or not.
    //
    m_bHadImage = bUseImage();

    //
    //  We added a scroll event in 4.3.929, in response to scrollable overlay
    //  support. So force it into any that don't have one.
    //
    if (!pcolOpsForEvent(kCQCKit::strEvId_OnScroll))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
            , kCQCKit::strEvId_OnScroll
            , tCQCKit::EActCmdCtx::NoContChange
        );
    }

    // And the same for the OnFlick that was added in 5.15
    if (!pcolOpsForEvent(kCQCKit::strEvId_OnFlick))
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnFlick)
            , kCQCKit::strEvId_OnFlick
            , tCQCKit::EActCmdCtx::NoContChange
        );
    }

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(m_strInitTemplate);
}


tCIDLib::TVoid TCQCIntfOverlay::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out a start marker and format vresion
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_Overlay::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfImgIntf::WriteOutImage(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Do our stuff and end marker
    strmToWriteTo   << m_strInitTemplate

                    // V4 stuff
                    << m_bPageMode

                    // V8 stuff
                    << m_colParams

                    // V9 is just a format conversion, no new data

                    // V10 stuff
                    << m_bManualFlick

                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfOverlay::Update(        TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{

    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Add our display area to the clip region
    grgnClip.CombineWith(m_areaDisplay, tCIDGraphDev::EClipModes::And);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    //
    //  If in design mode, it's not going to have any template to display,
    //  so if it has the transparent background setting and no image, draw a
    //  little outline around it, else just do the usual thing.
    //
    const tCIDLib::TBoolean bTrans = bIsTransparent();
    if (TFacCQCIntfEng::bDesMode())
    {
        //
        //  At design time we will only draw an image if one is set on
        //  on us explicitly.
        //
        if (bUseImage())
        {
            DrawImgOn
            (
                gdevTarget
                , kCIDLib::False
                , m_areaDisplay
                , areaInvalid
                , 0
                , 0
                , kCIDLib::False
                , kCIDLib::False
            );
        }
        return;
    }

    //
    //  NOTE the rest is about the CONTENT, which is relative to the
    //  scroll adjusted display area, not our own area.
    //
    {
        TArea areaCont = m_areaContent;
        areaCont.SetOrg(m_areaDisplay.pntOrg());
        areaCont -= m_areaAdjDisplay.pntOrg();

        //
        //  If the image interface mixin indicates we are using an image, then
        //  draw it. At viewing time it's either one explicitly set on us, or
        //  it's one set on us indirectly via a loaded template.
        //
        if (bUseImage())
        {
            DrawImgOn
            (
                gdevTarget
                , kCIDLib::False
                , areaCont
                , areaInvalid
                , 0
                , 0
                , kCIDLib::False
                , kCIDLib::False
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If something goes wrong and we need to insure that our contents gets back to
//  a safe, default, empty overlay, this will do the work.
//
tCIDLib::TVoid TCQCIntfOverlay::ResetOverlay()
{
    // We aren't displaying a template anymore
    m_strCurTemplate.Clear();

    m_colParams[0].Clear();
    m_colParams[1].Clear();
    m_colParams[2].Clear();
    m_colParams[3].Clear();

    //
    //  Reset the content and adjusted display areas back to small but non-zoer
    //  sizes. This way, no scrolling is going to occur, but we don't have to
    //  worry about divide by zero problems and such.
    //
    m_areaContent.Set(0, 0, 1, 1);
    m_areaAdjDisplay.Set(0, 0, 1, 1);

    // Remove any actions we got from the loaded template
    m_colActions.RemoveAll();
    m_colHotKeyActions.RemoveAll();
    m_colTrgEvents.RemoveAll();
    ResetAllOps(kCIDLib::False);

    // And clear out all of the child widgets
    RemoveAllChildren();
}


// ---------------------------------------------------------------------------
//  TCQCIntfOverlay: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Handles drawing our content into a zero origined memory image, for panning or
//  flicking. Since we need to do this in both pan and flick scenarios, we want to
//  break it out.
//
tCIDLib::TBoolean
TCQCIntfOverlay::bDrawZContent(TGraphDrawDev& gdevTar)
{
    // Get a copy since we are going to cause it to change temporarily
    const TArea areaUs = areaActual();

    //
    //  Temporarily force our widgets to think that we are positioned such that the
    //  output would overlay a zero based display area, inside any border.
    //
    //  If we take our area, zero origined, then move it back by the amount that our
    //  display area is moved inwards from our border, that should make the actual
    //  widgets fall into the content area, which doesn't include borders or margins
    //  that we might use.
    //
    //  This will take effect below when we call AreaChanged(). We want to do that
    //  inside the try block so it is sure to be undone.
    //
    const TPoint pntSaveOfs(m_areaAdjDisplay.pntOrg());
    m_areaAdjDisplay.ZeroOrg();

    TArea areaZUs(areaUs);
    areaZUs.ZeroOrg();
    areaZUs -= (m_areaDisplay.pntOrg() - areaUs.pntOrg());

    //
    //  Do the background fill on the memory image. Do this on the whole pan content
    //  bitmap area, which might be larger than the content area (which only contains
    //  the actual content), and we might be better than that in the transver dir to
    //  the scroll. It is created the larger (in either axis) of the content and
    //  display areas.
    //
    DoBgnFill(gdevTar, TArea(TPoint::pntOrigin, m_bmpPanCont.szBitmap()));

    try
    {
        // Force the temporary movement of our child widgets
        AreaChanged(areaZUs, areaUs);

        //
        //  We will only get called here if using a simple fill type bgn,
        //  so do that fill on our temp image. Then draw our widgets over
        //  that.
        //
        DoBgnFill(gdevTar, m_areaContent);
        TRegionJanitor janClip(&gdevTar, m_areaContent, tCIDGraphDev::EClipModes::And);

        //
        //  If the image interface mixin indicates we are using an image, then
        //  draw it. This won't be an issue since we know we have a solid
        //  background fill, so there's no transparency involved ultimately.
        //
        if (bUseImage())
        {
            facCQCIntfEng().DrawImage
            (
                gdevTar
                , kCIDLib::False
                , cptrImage()
                , m_areaContent
                , m_areaContent
                , ePlacement()
                , 0
                , 0
                , c1Opacity()
            );
        }

        // Call the container helper to draw the children into the content area
        bDrawChildren(gdevTar, m_areaContent, m_areaContent);
    }

    catch(TError& errToCatch)
    {
        // Put the widget origin back to our area
        m_areaAdjDisplay.SetOrg(pntSaveOfs);
        AreaChanged(areaUs, areaZUs);

        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // And don't try to do the gesture
        return kCIDLib::False;
    }

    catch(...)
    {
        // Put the widget origin back to our area
        m_areaAdjDisplay.SetOrg(pntSaveOfs);
        AreaChanged(areaUs, areaZUs);

        // And don't try to do the gesture
        return kCIDLib::False;
    }

    // Put the widget origin back to our area
    m_areaAdjDisplay.SetOrg(pntSaveOfs);
    AreaChanged(areaUs, areaZUs);

    return kCIDLib::True;
}


//
//  To avoid replication of this in multiple ctors. WE can optionally not load
//  the scrolling related events. Derived classes may not want those.
//
tCIDLib::TVoid TCQCIntfOverlay::CommonInit(const  tCIDLib::TBoolean bScrollable)
{
    // Make sure we have four parameters, even if empty
    if (m_colParams.bIsEmpty())
    {
        m_colParams.objAdd(TString::strEmpty());
        m_colParams.objAdd(TString::strEmpty());
        m_colParams.objAdd(TString::strEmpty());
        m_colParams.objAdd(TString::strEmpty());
    }
     else
    {
        m_colParams[0].Clear();
        m_colParams[1].Clear();
        m_colParams[2].Clear();
        m_colParams[3].Clear();
    }

    //
    //  Add the events that we support, but only if we are not in designer
    //  mode. When viewing, we have to pull over any of the events of the
    //  loaded template into ourselves, so we have to have these events
    //  available.
    //
    if (!TFacCQCIntfEng::bDesMode())
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnLoad)
            , kCQCKit::strEvId_OnLoad
            , tCQCKit::EActCmdCtx::NoContChange
        );

        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPreload)
            , kCQCKit::strEvId_OnLoad
            , tCQCKit::EActCmdCtx::Preload
        );
    }

    //
    //  Add OnScroll and OnFlick events that we send when the overlay is scrolled,
    //  or flicked if in Manual Flick mode, if we can.
    //
    if (bScrollable)
    {
        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnScroll)
            , kCQCKit::strEvId_OnScroll
            , tCQCKit::EActCmdCtx::NoContChange
        );

        AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnFlick)
            , kCQCKit::strEvId_OnFlick
            , tCQCKit::EActCmdCtx::NoContChange
        );
    }

    // Add the current page RTVs, only used in paged mode
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_HPageInd)
        , kCQCIntfEng::strRTVId_HPageInd
        , tCQCKit::ECmdPTypes::Unsigned
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_VPageInd)
        , kCQCIntfEng::strRTVId_VPageInd
        , tCQCKit::ECmdPTypes::Unsigned
    );

    // Add the flick direction, only used in manual flick mode
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_FlickDir)
        , kCQCIntfEng::strRTVId_FlickDir
        , tCQCKit::ECmdPTypes::Text
    );

    // Remember if we originally had an image
    m_bHadImage = bUseImage();

    //
    //  Set our default appearance, which we want to be the same as the default template
    //  background, and we'll set a black border.
    //
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn3(kCQCIntfEng_::rgbDef_Border);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Overlay");

    // Reset some other bits
    m_eLastFlickDir = tCIDLib::EDirs::Count;
}


//
//  This is called any time we are resized or moved. We have to update our display
//  and content areas appropriately to support scrolling and redrawing.
//
tCIDLib::TVoid
TCQCIntfOverlay::DoSizeCalcs(const TArea& areaNew, const TArea& areaOld)
{
    // Start with our own area
    m_areaDisplay = areaNew;

    // If a border move in one for the border and one for a margin
    if (bHasBorder())
        m_areaDisplay.Deflate(2);

    // If any rounding move in one more to provide more margin
    if (c4Rounding())
        m_areaDisplay.Deflate(1);

    //
    //  Set up the adjusted diplay area to the smaller of the display size
    //  or the content size in each direction.
    //
    TSize szAD(m_areaDisplay.szArea());
    szAD.TakeSmaller(m_areaContent.szArea());
    m_areaAdjDisplay.SetSize(szAD);

    // If we are scrolled, then see if we need to adjust
    if (m_areaAdjDisplay.i4X())
    {
        if (m_areaDisplay.c4Width() > m_areaContent.c4Width())
        {
            // No need to scroll anymore. Reset back to the origin
            m_areaAdjDisplay.i4X(0);
        }
         else
        {
            // Make sure we aren't hanging of the end now
            if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
                m_areaAdjDisplay.RightJustifyIn(m_areaContent);
        }
    }

    if (m_areaAdjDisplay.i4Y())
    {
        if (m_areaDisplay.c4Height() > m_areaContent.c4Height())
        {
            // No need to scroll anymore. Reset back to the origin
            m_areaAdjDisplay.i4Y(0);
        }
         else
        {
            // Make sure we aren't hanging of the end now
            if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
                m_areaAdjDisplay.BottomJustifyIn(m_areaContent);
        }
    }
}


//
//  This allows the user to dynamically generate a repeated sequence of widgets into
//  an overlay. Basically it allows the user to create a layout template that addresses
//  a single field or device. They often want to dynamically address a list of fields
//  or devices without having to hard code them. This method will take the layout and
//  re-generate it over and over for each field/moniker in the strFldsMons list.
//
//  They indicate whether we generate them horizontally or vertically.
//
//  We will ignore any of the fancier widgets or those that can't be scrolled or make
//  no sense for this type of operation. So there's never any recursion involved.
//
tCIDLib::TVoid
TCQCIntfOverlay::DynOverLoad(const  TString&            strLayout
                            , const TString&            strFldsMons
                            , const TString&            strNames
                            , const tCIDLib::TBoolean   bHorz)
{
    // We need to access our owning view a few times below so get it out
    TCQCIntfView& civMyOwner = civOwner();

    try
    {
        TDataSrvClient dsclLoad;

        //
        //  It can have an option P prefix, which means we can do partial (prefix)
        //  matching. Else we have to get a full match.
        //
        TString strFMList(strFldsMons);
        tCIDLib::TBoolean bFullMatch = kCIDLib::True;
        if (!strFMList.bIsEmpty()
        &&  ((strFMList[0] == L'P') || (strFMList[0] == L'p')))
        {
            bFullMatch = kCIDLib::False;
            strFMList.Cut(0, 1);
        }

        //
        //  Parse out the two quoted comma lists into their separate values. If not
        //  valid, or there are less than two entries, can't be good. Be sure to use
        //  the (potentially modified) version of the FM list here, not the original
        //  one.
        //
        tCIDLib::TCard4 c4ErrInd;
        tCIDLib::TStrList colFMs;
        if (!TStringTokenizer::bParseQuotedCommaList(strFMList, colFMs, c4ErrInd)
        ||  (colFMs.c4ElemCount() < 2))
        {
            civMyOwner.ShowMsg
            (
                L"Interface Engine"
                , L"The fields/monikers list was badly formed"
                , kCIDLib::True
            );
            return;
        }

        tCIDLib::TStrList colNames;
        if (!TStringTokenizer::bParseQuotedCommaList(strNames, colNames, c4ErrInd))
        {
            civMyOwner.ShowMsg
            (
                L"Interface Engine"
                , L"The names list was badly formed"
                , kCIDLib::True
            );
            return;
        }

        //
        //  The two lists have to be the same size, except that the FM list has an
        //  extra 'to find' initial entry.
        //
        if (colFMs.c4ElemCount() != colNames.c4ElemCount() + 1)
        {
            civMyOwner.ShowMsg
            (
                L"Interface Engine"
                , L"The fields/monikers and name lists had different sizes"
                , kCIDLib::True
            );
            return;
        }

        //
        //  If the first entry in the FMs list has a period, then we are dealing with
        //  fields. Else it's monikers.
        //
        const tCIDLib::TBoolean bDoFlds = colFMs[0].bContainsChar(kCIDLib::chPeriod);

        //
        //  Now get the first entry out. It's the one we are going to replace. The
        //  rest are the ones we will generate.
        //
        TString strOrgMon;
        TString strOrgFld;
        if (bDoFlds)
            facCQCKit().ParseFldName(colFMs[0], strOrgMon, strOrgFld);
        else
            strOrgMon = colFMs[0];
        colFMs.RemoveAt(0);

        // Now let's try to load the layout template
        TString strFullPath;
        piwdgParent()->ExpandTmplPath(strLayout, strFullPath);

        TCQCIntfTemplate iwdgLayout;
        facCQCIntfEng().QueryTemplate
        (
            strFullPath
            , dsclLoad
            , iwdgLayout
            , civMyOwner.cuctxToUse()
            , civMyOwner.bEnableTmplCaching()
        );

        // Go through it and remove any problematic widget types
        {
            tCIDLib::TCard4 c4Count = iwdgLayout.c4ChildCount();

            tCIDLib::TCard4 c4Index = 0;
            while (c4Index < c4Count)
            {
                TCQCIntfWidget& iwdgCur = iwdgLayout.iwdgAt(c4Index);

                if (iwdgCur.bIsDescendantOf(TCQCIntfContainer::clsThis())
                ||  iwdgCur.bIsA(TCQCIntfWebBrowser::clsThis())
                ||  iwdgCur.bIsA(TCQCIntfCvrArtBrowser::clsThis()))
                {
                    iwdgLayout.RemoveWidget(c4Index);
                    c4Count--;
                }
                 else
                {
                    c4Index++;
                }
            }

            // If no widgets in the layout, then we can't do anything
            if (!c4Count)
            {
                civMyOwner.ShowMsg
                (
                    L"Interface Engine"
                    , L"The layout template had no widgets"
                    , kCIDLib::True
                );
                return;
            }
        }

        //
        //  Figure out our final template size. We store this in m_areaContent,
        //  which indicates the size of the content we contain.
        //
        if (bHorz)
        {
            m_areaContent.Set
            (
                0
                , 0
                , iwdgLayout.areaRelative().c4Width() * colFMs.c4ElemCount()
                , iwdgLayout.areaRelative().c4Height()
            );
        }
         else
        {
            m_areaContent.Set
            (
                0
                , 0
                , iwdgLayout.areaRelative().c4Width()
                , iwdgLayout.areaRelative().c4Height() * colFMs.c4ElemCount()
            );
        }

        // Stop timers while do this
        TCQCIntfTimerJan janTimers(&civOwner());

        //
        //  Give any widgets a change to pre-cleanup before we destroy them.
        //  Some will take advantage of this.
        //
        TermWidgets();

        // We need a field info cache to initialize the widgtes
        TCQCFldCache cfcData;
        cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadOrWrite));

        //
        //  Tell our containing viewer window that we are about to replace
        //  our contents, so that he can deal with any references he
        //  might have had to our descendants.
        //
        civMyOwner.StartBranchReplace(*this);

        // Remove any existing children
        RemoveAllChildren();

        //
        //  And now let's loop though the list of fields or monikers, and generate
        //  the widgets and update them.
        //
        const TString& strFMNameId(L"CurrentName");
        TPoint pntCur;
        TArea areaOfs;
        TString strSave;
        TString strTmp;
        TString strTmp2;
        TString strCurFld;
        TString strCurMon;
        TRegEx regxDummy;
        TCQCIntfTemplate iwdgTmp;

        //
        //  We start off at the overlay's actual origin (zero in the diretion of
        //  travel), and the layout template's size. We'll move right or down from
        //  there as we go.
        //
        TArea areaCur
        (
            areaActual().pntOrg(), iwdgLayout.areaRelative().szArea()
        );

        if (bHorz)
            areaCur.i4X(0);
        else
            areaCur.i4Y(0);

        const TArea& areaUs = areaActual();
        const tCIDLib::TCard4 c4FMCount = colFMs.c4ElemCount();
        for (tCIDLib::TCard4 c4FMIndex = 0; c4FMIndex < c4FMCount; c4FMIndex++)
        {
            // Get a copy of the layout for this round
            iwdgTmp = iwdgLayout;

            // If doing fields parse it out, else just get the moniker part
            if (bDoFlds)
            {
                facCQCKit().ParseFldName(colFMs[c4FMIndex], strCurMon, strCurFld);
            }
             else
            {
                strCurMon = colFMs[c4FMIndex];
                strCurFld.Clear();
            }

            // And let's process them all
            const tCIDLib::TCard4 c4WCount = iwdgTmp.c4ChildCount();
            for (tCIDLib::TCard4 c4WIndex = 0; c4WIndex < c4WCount; c4WIndex++)
            {
                TCQCIntfWidget& iwdgCur = iwdgTmp.iwdgAt(c4WIndex);

                // Set the parent origin to our origin
                iwdgCur.SetParentOrg(areaUs.pntOrg(), kCIDLib::False);

                //
                //  Adjust the origin by our current area origin, in the direction
                //  of travel. It'll be zero for the first one, so it'll end up
                //  at our origin.
                //
                areaOfs = iwdgCur.areaActual();
                if (bHorz)
                    areaOfs.AdjustOrg(areaCur.i4X(), 0);
                else
                    areaOfs.AdjustOrg(0, areaCur.i4Y());
                iwdgCur.SetArea(areaOfs, kCIDLib::False);


                if (!iwdgCur.strWidgetId().bIsEmpty())
                {
                    //
                    //  If it's a static text and has the id "FMName", we will set
                    //  the text to the current configured name.
                    //
                    if (iwdgCur.strWidgetId().bCompareI(strFMNameId))
                        iwdgCur.strCaption(colNames[c4FMIndex]);

                    // And we have to update the widget id to be index specific
                    strSave = iwdgCur.strCmdTargetId();

                    strTmp = iwdgCur.strWidgetId();
                    strTmp.AppendFormatted(c4WIndex + 1);
                    iwdgCur.strWidgetId(strTmp);

                    //
                    //  Change any action commands that targeted the original name
                    //  to use the new one. We saved the original command target id
                    //  above, and now it will be the new one.
                    //
                    iwdgTmp.bUpdateCmdTarget
                    (
                        strSave, iwdgCur.strWidgetId(), iwdgCur.strCmdTargetId()
                    );
                }

                //
                //  Do a replacement on the widget to handle any field/moniker refs.
                //  If doing monikers, we first search for 'moniker.' to get the
                //  moniker part of any field names, then we search for the whole
                //  field name part.
                //
                if (bDoFlds)
                {
                    strTmp = strOrgMon;
                    strTmp.Append(kCIDLib::chPeriod);
                    strTmp.Append(strOrgFld);

                    strTmp2 = strCurMon;
                    strTmp2.Append(kCIDLib::chPeriod);
                    strTmp2.Append(strCurFld);

                    iwdgCur.Replace
                    (
                        tCIDLib::eOREnumBits
                        (
                            tCQCIntfEng::ERepFlags::AssocField
                            , tCQCIntfEng::ERepFlags::ActionParms
                            , tCQCIntfEng::ERepFlags::Expressions
                        )
                        , strTmp
                        , strTmp2
                        , kCIDLib::False
                        , bFullMatch
                        , regxDummy
                    );
                }
                 else
                {
                    // Get the moniker part of any field refs
                    strTmp = strOrgMon;
                    strTmp.Append(kCIDLib::chPeriod);
                    strTmp2 = strCurMon;
                    strTmp2.Append(kCIDLib::chPeriod);

                    iwdgCur.Replace
                    (
                        tCIDLib::eOREnumBits
                        (
                            tCQCIntfEng::ERepFlags::AssocField
                            , tCQCIntfEng::ERepFlags::ActionParms
                            , tCQCIntfEng::ERepFlags::Expressions
                        )
                        , strTmp
                        , strTmp2
                        , kCIDLib::False
                        , kCIDLib::False
                        , regxDummy
                    );

                    // Now do moniker associations, full match required here
                    iwdgCur.Replace
                    (
                        tCQCIntfEng::ERepFlags::AssocDevice
                        , strOrgMon
                        , strCurMon
                        , kCIDLib::False
                        , kCIDLib::True
                        , regxDummy
                    );
                }
            }

            // Add all of the children of this iteration to our own list
            for (tCIDLib::TCard4 c4WIndex = 0; c4WIndex < c4WCount; c4WIndex++)
            {
                TCQCIntfWidget* piwdgNew
                (
                    static_cast<TCQCIntfWidget*>(iwdgTmp.iwdgAt(c4WIndex).pobjDuplicate())
                );

                piwdgNew->SetOwner(&civMyOwner);
                AddChild(piwdgNew);
            }

            // Move the current area forward in the generation direction
            if (bHorz)
                areaCur.Move(tCIDLib::EDirs::Right);
            else
                areaCur.Move(tCIDLib::EDirs::Down);
        }

        //
        //  Now initialize them. We ignore any overlays in the layout template so
        //  there is no recursion involved. We don't have any path to pass so we
        //  just pass an empty string.
        //
        tCQCIntfEng::TErrList colErrs;
        InitOverlay
        (
            &civMyOwner
            , TString::strEmpty()
            , facCQCIntfEng().polleThis()
            , dsclLoad
            , colErrs
        );

        // Kick off any preloads
        DoPreloads(civMyOwner, TString::strEmpty());

        // Get all of the widgets under us to register their fields.
        RegisterFields(facCQCIntfEng().polleThis(), cfcData);

        //
        //  Pause slightly to give the polling engine some time to get any
        //  new fields polled. DO NOT msg yield here, since that would
        //  allow the update timer to kick in and cause all kind of nasty
        //  ragged redraw problems, if not flat out errors.
        //
        TThread::Sleep(150);

        //
        //  Force an initial field update so that we can at least get the values
        //  of any fields that are in the polling engine so far, and have the
        //  widgets that access those fields valid before we even show up.
        //
        InitialScan(facCQCIntfEng().polleThis(), civMyOwner.ctarGlobalVars());

        // Do post init on all templates that want it
        DoPostInits();

        // And force a redraw with as much status as we have
        Redraw(areaActual());

        // And do OnLoads now that we are fully setup and displayed
        DoOnLoads(civMyOwner);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        civMyOwner.ShowErr
        (
            L"Interface Engine"
            , L"An error occuring in the dynamic overlay loading process"
            , errToCatch
        );
    }
}


//
//  When a template is loaded into an overlay, a link operation now really
//  means an overlay load operation in the (now its parent) overlay. So we
//  have to convert such operations at runtime.
//
//  All link operations are set up with a generic 'interface viewer' command
//  target so they will go to the viewer window (which registers itself as
//  that command target.) It won't have enough info to recognize that the
//  wrong thing has happened, so we cannot let it just re-direct the operation
//  to the right overlay.
//
//  There's a similar situation with template level actions. They are configured
//  on the viewer target, but we map them to us at runtime. We steal the
//  actions of the templates that are loaded into us so that we can process
//  such commands.
//
//  So, when an overlay gets a load operation, it will call us here and we
//  have to search all the child widgets just loaded and if any of them
//  implement the command source, we have to iterate the commands. If any of
//  them are link operations, we convert them into overlay load operations on
//  ourself.
//
//  We don't have to recurse, we just do the children of this container. When
//  this is called, the next level down (if any) hasn't been loaded yet because
//  we are called during the recursive load. Most of the time the load isn't
//  even recursive, but if any overlays at this level have initial loads
//  configured, then the loader will call back on those as it goes.
//
//  NOTE we also have to do this for the template level action commands, in
//  case one of them invokes such a command.
//
tCIDLib::TVoid TCQCIntfOverlay::SwizzleCmds()
{
    CIDAssert
    (
        !TFacCQCIntfEng::bDesMode()
        , L"SwizzleCmds should only be called at viewing time"
    );

    // Build up a path to us
    TString strTarId = kCQCKit::strTarPref_Widgets;
    strTarId.Append(strWidgetId());

    //
    //  Do ourself, since these could be called from OnPreload or OnLoad
    //  or from a hot key in a RIVA client.
    //
    UpdateOps
    (
        kCQCIntfEng::strCmdId_SetTimeout
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_SetTimeout
    );

    UpdateOps
    (
        kCQCIntfEng::strCmdId_GetOverlayParam
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_GetOverlayParam
    );


    // Do all of our children
    tCQCKit::TCmdSrcList colSrcs(tCIDLib::EAdoptOpts::NoAdopt);
    tCIDLib::TCard4 c4Count = c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = iwdgAt(c4Index);

        //
        //  See if it implements the command source interface. If not, then
        //  ask it for a list of targets it might contain, since sometimes
        //  they contain targets but aren't one themselves.
        //
        MCQCCmdSrcIntf* pmcsrcCur = dynamic_cast<MCQCCmdSrcIntf*>(&iwdgCur);
        if (pmcsrcCur)
        {
            SwizzleOne(strTarId, *pmcsrcCur);
        }
         else
        {
            colSrcs.RemoveAll();
            const tCIDLib::TCard4 c4EmbCnt = iwdgCur.c4QueryEmbeddedCmdSrcs(colSrcs);
            for (tCIDLib::TCard4 c4EmbInd = 0; c4EmbInd < c4EmbCnt; c4EmbInd++)
                SwizzleOne(strTarId, *colSrcs[c4EmbInd]);
        }
    }

    // Do any template level actions
    c4Count = m_colActions.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        SwizzleOne(strTarId, m_colActions[c4Index]);

    //
    //  We only have hot key sif the template loaded into us does. Since we
    //  use the enum and not the element count, we have to check this first.
    //
    if (!m_colHotKeyActions.bIsEmpty())
    {
        tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
        for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
        {
            if (m_colHotKeyActions[eKey].bUsed())
                SwizzleOne(strTarId, m_colHotKeyActions[eKey]);
        }
    }

    // Do any triggered type actions
    c4Count = m_colTrgEvents.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        SwizzleOne(strTarId, m_colTrgEvents[c4Index]);
}


// A helper for SwizzleCmds above, to avoid a lot of redundancy
tCIDLib::TVoid
TCQCIntfOverlay::SwizzleOne(const   TString&        strTarId
                            ,       MCQCCmdSrcIntf& mcsrcToUpdate)
{
    // Change this one to be a load overlay command on us
    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_DoLink
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_LoadOverlay
    );


    // These all keep the same commands but just remap to us
    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_GetOverlayParam
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_GetOverlayParam
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_LoadAncestor
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_LoadAncestor
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_ScrollParent
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_ScrollParent
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_SetParentState
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_SetParentState
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_LoadDefTmpl
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_LoadDefTmpl
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_LoadSiblingOverlay
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_LoadSiblingOverlay
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_RunTmplAction
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_RunTmplAction
    );

    mcsrcToUpdate.UpdateOps
    (
        kCQCIntfEng::strCmdId_SetTimeout
        , *this
        , strTarId
        , kCQCIntfEng::strCmdId_SetTimeout
    );
}



