//
// FILE NAME: CQCIntfEng_AdvMItemBrowser.cpp
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
//  This file implements the advanced media item list browser. We bring in
//  the regular item browser's header so that we can us his RTV class. And
//  we bring in the headers for the types of widgets that we allow in the
//  layout, since we have to deal with them in a typed way.
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

// The widget types we'll use in our layout, so we have to import them
#include    "CQCIntfEng_BooleanImg.hpp"
#include    "CQCIntfEng_MItemBrowser.hpp"
#include    "CQCIntfEng_StaticImg.hpp"
#include    "CQCIntfEng_StaticText.hpp"

#include    "CQCIntfEng_AdvMItemBrowser.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfAdvMItemBrowser,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_MItemBrowser
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2
        //      Conversion of paths to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The caps flags for our class
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  Some special widget ids we look for
        // -----------------------------------------------------------------------
        const TString    strWID_CurItem(L"CurItem");
        const TString    strWID_Focus(L"FocusImage");
        const TString    strWID_Pressed(L"PressedImage");


        // -----------------------------------------------------------------------
        //  The field mixin indices for our two fields
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4FldInd_PLIndex = 0;
        constexpr tCIDLib::TCard4   c4FldInd_PLSerialNum = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAdvMItemBrowser
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAdvMItemBrowser::TCQCIntfAdvMItemBrowser() :

    TCQCIntfWidget
    (
        CQCIntfEng_MItemBrowser::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_AdvMItemBrowser)
    )
    , MCQCCmdSrcIntf()
    , MCQCIntfMultiFldIntf(2)
    , m_bHasFocusInd(kCIDLib::False)
    , m_bSafePanBgn(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4LastCurItem(kCIDLib::c4MaxCard)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
{
    // We have one action event, when the user makes a selection
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );

    //
    //  We have to support the runtime values that the simpler media item
    //  browser's runtime values class provides, since we use his RTV class.
    //
    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemArtist)
        , kCQCMedia::strRTVId_ItemArtist
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ItemCookie)
        , kCQCMedia::strRTVId_ItemCookie
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_LocInfo)
        , kCQCMedia::strRTVId_LocInfo
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_MediaRepoMoniker)
        , kCQCMedia::strRTVId_MediaRepoMoniker
        , tCQCKit::ECmdPTypes::Driver
    );

    AddRTValue
    (
        facCQCMedia().strMsg(kMedMsgs::midRTVal_ListItemId)
        , kCQCMedia::strRTVId_ListItemId
        , tCQCKit::ECmdPTypes::Driver
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaItemBrowserAdv");
}

TCQCIntfAdvMItemBrowser::
TCQCIntfAdvMItemBrowser(const TCQCIntfAdvMItemBrowser& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfMultiFldIntf(iwdgSrc)
    , m_bHasFocusInd(kCIDLib::False)
    , m_bSafePanBgn(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4LastCurItem(kCIDLib::c4MaxCard)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_strMoniker(iwdgSrc.m_strMoniker)
    , m_strTemplate(iwdgSrc.m_strTemplate)
{
    //
    //  No need to copy the item list, scrolling position, focus info,
    //  and some other info.
    //
    //  That's purely a viewing time thing
    //
}

TCQCIntfAdvMItemBrowser::~TCQCIntfAdvMItemBrowser()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Public operators
// ---------------------------------------------------------------------------
TCQCIntfAdvMItemBrowser&
TCQCIntfAdvMItemBrowser::operator=(const TCQCIntfAdvMItemBrowser& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfMultiFldIntf::operator=(iwdgSrc);

        m_strMoniker    = iwdgSrc.m_strMoniker;
        m_strTemplate   = iwdgSrc.m_strTemplate;

        // These are meaningless until set at viewing time
        m_areaContent.ZeroAll();
        m_areaAdjDisplay.ZeroAll();
        m_areaDisplay.ZeroAll();
        m_bHasFocusInd = kCIDLib::False;
        m_bSafePanBgn = kCIDLib::False;
        m_c4FocusIndex = 0;
        m_c4LastCurItem = kCIDLib::c4MaxCard;
        m_c4SelIndex = kCIDLib::c4MaxCard;
        m_colItems.RemoveAll();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  In our case this is kind of meaningless. We work in terms of a moniker
//  and set up the field mixin ourself. So we know we only will be set up
//  with fields we accepted. We do a basic check just in case.
//
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bCanAcceptField(const  TCQCFldDef&     flddCheck
                                        , const tCIDLib::TCard4 c4FldInd
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    if ((c4FldInd == CQCIntfEng_MItemBrowser::c4FldInd_PLIndex)
    ||  (c4FldInd == CQCIntfEng_MItemBrowser::c4FldInd_PLSerialNum))
    {
        // Make sure they are readable and card fields
        if ((flddCheck.eType() == tCQCKit::EFldTypes::Card)
        &   tCIDLib::bAllBitsOn(flddCheck.eAccess(), tCQCKit::EFldAccess::Read))
        {
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


// We can size to as long as we have a layout template set
tCIDLib::TBoolean TCQCIntfAdvMItemBrowser::bCanSizeTo() const
{
    return !m_strTemplate.bIsEmpty();
}


// We only care about runtime stfff
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfAdvMItemBrowser& iwdgOther
    (
        static_cast<const TCQCIntfAdvMItemBrowser&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfMultiFldIntf::bSameFields(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    return ((m_strTemplate == iwdgOther.m_strTemplate)
           && (m_strMoniker == iwdgOther.m_strMoniker));
}


//
//  Handle focus movement reqeuests.
//
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bMoveFocus(const tCQCKit::EScrNavOps eOp)
{
    // If the list is empty, just do nothing and say move past us
    if (m_colItems.bIsEmpty())
        return kCIDLib::True;

    tCIDLib::TBoolean bRet = kCIDLib::True;

    if ((eOp == tCQCKit::EScrNavOps::End)
    ||  (eOp == tCQCKit::EScrNavOps::Home)
    ||  (eOp == tCQCKit::EScrNavOps::PrevPage)
    ||  (eOp == tCQCKit::EScrNavOps::NextPage))
    {
        //
        //  Call the list scrolling helper and translate our op into the
        //  action command parameter that it expects.
        //
        if (eOp == tCQCKit::EScrNavOps::End)
            bScrollList(L"Bottom", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::Home)
            bScrollList(L"Top", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::PrevPage)
            bScrollList(L"Prev", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::NextPage)
            bScrollList(L"Next", kCIDLib::True);

        // Indicate we kept the focus internal
        bRet = kCIDLib::False;
    }
     else
    {
        tCIDLib::TCard4 c4NewFocus = m_c4FocusIndex;
        switch(eOp)
        {
            case tCQCKit::EScrNavOps::Up :
                // If not on the top-most item, then move up
                if (c4NewFocus)
                    c4NewFocus--;
                break;

            case tCQCKit::EScrNavOps::Down :
                if (c4NewFocus + 1 < m_colItems.c4ElemCount())
                    c4NewFocus++;
                break;

            default :
                break;
        };

        // Make sure it's visible enough to take
        if ((c4NewFocus != m_c4FocusIndex) && bVisibleEnough(c4NewFocus, 0.4F))
        {
            TArea areaOld;
            AreaForIndex(m_c4FocusIndex, areaOld);
            TArea areaUpdate;
            AreaForIndex(c4NewFocus, areaUpdate);
            areaUpdate |= areaOld;

            m_c4FocusIndex = c4NewFocus;
            bRet = kCIDLib::False;

            Redraw(areaUpdate);
        }
    }
    return bRet;
}



//
//  We allow vertical, single finger panning only, but only if we have enough
//  content to even pan. If our no inertia flag is set or we are in remote
//  mode, we indicate we want a flick instead.
//
//  If it's two fingered, or it's not up or down, then we disable but allow it
//  to propogate.
//
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bPerGestOpts(  const   TPoint&                 pntAt
                                        , const tCIDLib::EDirs          eDir
                                        , const tCIDLib::TBoolean       bTwoFingers
                                        ,       tCIDCtrls::EGestOpts&   eToSet
                                        ,       tCIDLib::TFloat4&       f4VScale)
{
    tCIDLib::TBoolean bProp = kCIDLib::False;

    if (!bTwoFingers
    &&  ((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Down)))
    {
        if (m_areaContent.c4Height() > m_areaDisplay.c4Height())
        {
            if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;

            //
            //  If the layout is fairly short, we reduce the intensity of the inertia
            //  a bit.
            //
            const TArea& areaLayout = m_iwdgLayout.areaActual();

            if (areaLayout.c4Height() < 25)
                f4VScale = 0.80F;
            else if (areaLayout.c4Height() < 64)
                f4VScale = 0.85F;
            else if (areaLayout.c4Height() < 128)
                f4VScale = 0.9F;
            else
                f4VScale = 1.0F;
        }
    }
     else
    {
        // It's not one we react to, so allow it to propogate to the parent
        bProp = kCIDLib::True;
    }

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bProcessGestEv(const   tCIDCtrls::EGestEvs eEv
                                        , const TPoint&             pntStart
                                        , const TPoint&             pntAt
                                        , const TPoint&             pntDelta
                                        , const tCIDLib::TBoolean   bTwoFinger)
{
    tCIDLib::TBoolean bContinue = kCIDLib::True;
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        const TArea& areaUs = areaActual();

        //
        //  Decide now big our scrollable memory image is going to be. Get
        //  a copy of the content area to start.
        //
        m_areaPanCont = m_areaContent;
        const TArea& areaLayout = m_iwdgLayout.areaActual();

        {
            TArea areaScreen = TGUIFacility::areaVirtualScreen();
            const tCIDLib::TInt4 i4Span(tCIDLib::TInt4(areaScreen.c4Height()));

            // Set the size to five spans
            m_areaPanCont.c4Height(i4Span * 5);

            // Put the display info on the middel one
            m_areaPanCont.i4Y(m_areaAdjDisplay.i4Y() - (i4Span * 2));

            // And clip to the content area
            if (m_areaPanCont.i4Y() < m_areaContent.i4Y())
                m_areaPanCont.i4Y(m_areaContent.i4Y());

            // Now set the size to two spans and clip again
            if (m_areaPanCont.i4Bottom() > m_areaContent.i4Bottom())
                m_areaPanCont.i4Bottom(m_areaContent.i4Bottom());
        }

        //
        //  Now set up the actual bit of that pan content that we will initially
        //  draw. We will do just enough to handle the worst case actual move
        //  of the finger by the user, which will be two times our width. This is
        //  as far as they could possibly move us in the actual drag, before
        //  inertia.
        //
        //  Note it's 3x our whole height, not the display height, since dragging
        //  continues to the edge of the whole widget.
        //
        tCIDLib::TCard4 c4InitialHeight = tCIDLib::TCard4(areaUs.c4Height() * 3);

        //
        //  Set up an area of that size, such that the middle page of the three
        //  widths is laided over the current display content. This sill be the
        //  initial 'drawn so far' area, by starting with the display area nd
        //  moving it left one width.
        //
        m_areaPanSoFar = m_areaAdjDisplay;
        m_areaPanSoFar.c4Height(c4InitialHeight);
        m_areaPanSoFar.AdjustOrg(0, -tCIDLib::TInt4(areaUs.c4Height()));

        // Clip to the pan content area if needed
        if (m_areaPanSoFar.i4Y() < m_areaPanCont.i4Y())
            m_areaPanSoFar.i4Y(m_areaPanCont.i4Y());
        if (m_areaPanSoFar.i4Bottom() > m_areaPanCont.i4Bottom())
            m_areaPanSoFar.i4Bottom(m_areaPanCont.i4Bottom());

        //
        //  We now have a pan content area that is sitting over the zero
        //  based content area at the right place. So we should be able to
        //  draw the stuff that falls into that area into our scroll area.
        //  Create a content bitmap of that size to draw into.
        //
        m_bmpCont = TBitmap
        (
            m_areaPanCont.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
        );

        //
        //  Get a version of the initial 'so far' area adjusted relative to the
        //  pan content, so we can do some drawing into it below. The bitmap is
        //  the zero origined pan content area.
        //
        TArea areaInitial(m_areaPanSoFar);
        areaInitial -= m_areaPanCont.pntOrg();

        TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
        if (m_bSafePanBgn)
        {
            //
            //  We have a safe bgn so we can put our background into the
            //  contents itself. We have to insure that any gradients
            //  line up, but since the content is always left justified
            //  in our display area, then we can just do the fill on the
            //  initial area as is.
            //
            DoBgnFill(gdevCont, areaInitial);
        }
         else
        {
            // We have to get the underlying content
            m_bmpBgn = bmpGatherBgn();

            //
            //  We need one for double buffering during the pan in this
            //  case, forced to memory. It only needs to be as big as
            //  the display area.
            //
            m_bmpDBuf = TBitmap
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
            );
        }

        //
        //  Draw the items now. The invalid area is the zero based pan
        //  content area. If we subtract the pan content origin from the
        /// original content area, this will create a relative to
        //  area, which will go up above the zero origin by however far
        //  the pan content is adjusted. That'll make the correct first
        //  item show up within our pan content bitmap.
        //
        TArea areaRelTo(m_areaContent);
        areaRelTo -= m_areaPanCont.pntOrg();
        DrawItems
        (
            gdevCont
            , areaRelTo
            , areaInitial
            , TPoint::pntOrigin
            , !m_bSafePanBgn
        );
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        // Remember the current adjusted display position
        const tCIDLib::TInt4 i4OrgPos = m_areaAdjDisplay.i4Y();

        //
        //  Apply the delta and clip it to the pan content area. We
        //  have to flip the delta since we are moving the display area
        //  forward, not the content area back.
        //
        m_areaAdjDisplay.AdjustOrg(0, -pntDelta.i4Y());

        // Clip it to the pan content area
        if (m_areaAdjDisplay.i4Y() < m_areaPanCont.i4Y())
            m_areaAdjDisplay.i4Y(m_areaPanCont.i4Y());
        else if (m_areaAdjDisplay.i4Bottom() > m_areaPanCont.i4Bottom())
            m_areaAdjDisplay.BottomJustifyIn(m_areaPanCont);

        //
        //  If we actually moved, then do the movement. If not, and we
        //  are getting inertia at this point, cancel any more inertia.
        //
        if (m_areaAdjDisplay.i4Y() != i4OrgPos)
        {
            tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

            // Calculate the actual delta
            tCIDLib::TInt4 i4NewDelta = m_areaAdjDisplay.i4Y() - i4OrgPos;

            //
            //  Clip to within our display plus the most restrictive of
            //  any of our ancestor containers.
            //
            TGUIRegion grgnClip;
            bFindMostRestrictiveClip(grgnClip, m_areaDisplay);
            TRegionJanitor janClip
            (
                cptrDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
            );

            //
            //  We now need to adjust the adjusted display area, because
            //  it is currently relative to the overall content origin,
            //  and we need ot make it relative to the pan content. Now
            //  it's over the actual pan content at the appropriate place
            //  to blit content from. So we can blit from here to our
            //  display area.
            //
            TArea areaBlitSrc = m_areaAdjDisplay;
            areaBlitSrc -= m_areaPanCont.pntOrg();

            if (m_bSafePanBgn)
            {
                //
                //  In this case we can just blit the contents directly
                //  since we have the bgn in the content. But we do a
                //  more efficient scheme where we just move the screen
                //  contents and fill in the uncovered bits.
                //
                TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpCont);

                // Scroll the display area over
                TArea areaTmpTar;
                cptrDev->ScrollBits
                (
                    m_areaDisplay
                    , m_areaDisplay
                    , 0
                    , i4NewDelta * -1
                    , areaTmpTar
                    , kCIDLib::True
                );

                //
                //  Create a source area from the target update area the scroll gave
                //  us. It needs to be either at the left or right edge of the src
                //  blit area. Vertcally center it as well, since the value we get
                //  from the scroll is relative to the window, and this is zero
                //  based.
                //
                TArea areaTmpSrc(areaTmpTar);
                if (i4NewDelta < 0)
                    areaTmpSrc.TopJustifyIn(areaBlitSrc, kCIDLib::True);
                 else
                    areaTmpSrc.BottomJustifyIn(areaBlitSrc, kCIDLib::True);

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
             else
            {
                //
                //  We have to do the composite scenario. Set up a zero
                //  based version of our own area. We need this to blit from
                //  the background to our double buffer, and from that to
                //  our display area.
                //
                TArea areaZDisp = m_areaDisplay;
                areaZDisp.ZeroOrg();

                // Copy in the background content to the double buffer
                TGraphMemDev gdevDBuf(civOwner().gdevCompat(), m_bmpDBuf);
                {
                    TGraphMemDev gdevBgn(civOwner().gdevCompat(), m_bmpBgn);
                    gdevDBuf.CopyBits(gdevBgn, areaZDisp, areaZDisp);
                }

                // And alpha blit the content over that
                {
                    TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
                    gdevDBuf.AlphaBlit(gdevCont, areaBlitSrc, areaZDisp, 0xFF, kCIDLib::True);
                }

                // And finally copy the result to the target
                cptrDev->CopyBits
                (
                    gdevDBuf
                    , areaZDisp
                    , m_areaDisplay
                    , tCIDGraphDev::EBmpModes::SrcCopy
                    , kCIDLib::True
                );
            }
        }
         else if (eEv == tCIDCtrls::EGestEvs::Inertia)
        {
            bContinue = kCIDLib::False;
        }

        // If an end, then we can clean up and do a final redraw
        if (eEv == tCIDCtrls::EGestEvs::End)
        {
            // Release the bitmap memory we used
            m_bmpBgn.Reset();
            m_bmpDBuf.Reset();
            m_bmpCont.Reset();

            // Keep the focus index visible
            KeepFocusVisible();

            // Do a final draw in the end position
            Redraw();
        }
    }

    return bContinue;
}


//
//  Callled if we are clicked on
//
tCIDLib::TVoid TCQCIntfAdvMItemBrowser::Clicked(const TPoint& pntAt)
{
    // See if we hit an item. If not, we are done
    const tCIDLib::TCard4 c4HitInd = c4HitTest(pntAt);
    if (c4HitInd == kCIDLib::c4MaxCard)
        return;

    //
    //  Make sure that this one is at least 40% visible. If not, we'll
    //  ignore it.
    //
    tCIDLib::TCard4 c4Dummy = c4HitInd;
    if (!bVisibleEnough(c4Dummy, 0.4F))
        return;

    TArea areaUpdate;
    if (m_c4FocusIndex != c4HitInd)
    {
        AreaForIndex(m_c4FocusIndex, areaUpdate);

        // Move the focus index and redraw the old position
        m_c4FocusIndex = c4HitInd;
        Redraw(areaUpdate);
    }

    // Draw the target pressed, pause, and then unpressed
    {
        AreaForIndex(c4HitInd, areaUpdate);

        // We use the played marker as a pressed indicator
        m_colItems[c4HitInd].bPlayed(kCIDLib::True);
        Redraw(areaUpdate);

        TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs);
        m_colItems[c4HitInd].bPlayed(kCIDLib::False);
        Redraw(areaUpdate);
    }

    // And now invoke the onselect
    TCardJanitor janOver(&m_c4SelIndex, c4HitInd);
    PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::NonNested);

    //
    //  Don't do anything else unless you check that we weren't
    //  killed by this action.
    //
}



tCIDLib::TVoid TCQCIntfAdvMItemBrowser::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfAdvMItemBrowser&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfAdvMItemBrowser::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetMoniker)
    {
        //
        //  Accept this guy if we can. If it's a media renderer driver, with
        //  the required fields, we'll update our multi-field interface
        //  mixin to use these new fields.
        //
        TString strReason;
        bResult = bSetMoniker(ccfgToDo.piAt(0).m_strValue, strReason);
        if (bResult && bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollList)
    {
        bResult = bScrollList
        (
            ccfgToDo.piAt(0).m_strValue, bCanRedraw(strEventId)
        );
    }
     else
    {
        // Not one of ours, so let our parent try it
        return TParent::eDoCmd
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
    return tCQCKit::ECmdRes::Ok;
}


tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::GestInertiaIdle(const  tCIDLib::TEncodedTime   enctEnd
                                        , const tCIDLib::TCard4         c4Millis
                                        , const tCIDLib::EDirs          eDir)
{
    //
    //  If we have have already draw all the way to the edge in the direction of
    //  inertia, then we are done. Our pan content and so far areas are relative
    //  to the zero origin of the bitmap, so going left if we hit zero we are done.
    //
    //  Else, see how close the display area is to how far we've drawn. If it's
    //  within two widget widths, we will draw out up to another width.
    //
    const tCIDLib::TInt4    i4Span(areaActual().c4Height() / 4);
    const tCIDLib::TInt4    i42Span(i4Span * 2);

    TArea   areaUpdate;
    TArea   areaNewSoFar(m_areaPanSoFar);

    if (eDir == tCIDLib::EDirs::Up)
    {
        if (areaNewSoFar.i4Bottom() >= m_areaPanCont.i4Bottom())
            return;

        // As long as we have more than two widths, don't do anything
        if (areaNewSoFar.i4Bottom() - m_areaAdjDisplay.i4Bottom() > i42Span)
            return;

        areaNewSoFar.AdjustSize(0, i4Span);

        // If it goes too far. clip it back
        if (areaNewSoFar.i4Bottom() > m_areaPanCont.i4Bottom())
            areaNewSoFar.i4Bottom(m_areaPanCont.i4Bottom());

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4Y(m_areaPanSoFar.i4Bottom(), kCIDLib::True);
    }
     else if (eDir == tCIDLib::EDirs::Down)
    {
        if (!areaNewSoFar.i4Y())
            return;

        if (m_areaAdjDisplay.i4Y() - areaNewSoFar.i4Y() > i42Span)
            return;

        // Keep the right where it is, move the top up by a span
        areaNewSoFar.i4Top(areaNewSoFar.i4Y() - i4Span, kCIDLib::True);

        // If it goes below the origin, clip it back
        if (areaNewSoFar.i4Y() < 0)
            areaNewSoFar.i4Y(0, kCIDLib::True);

        // The update area is now the new so far minus the old one
        areaUpdate = areaNewSoFar;
        areaUpdate.i4Bottom(m_areaPanSoFar.i4Y() - 1);
    }

    // If by some chance, it ended up empty, then do nothing
    if (areaUpdate.bIsEmpty())
        return;

    // Update the so far area
    m_areaPanSoFar = areaNewSoFar;

    //
    //  Subtract the pan content area's origin from teh update area. This will put
    //  it at the right place in the zero based content area bitmap.
    //
    areaUpdate -= m_areaPanCont.pntOrg();

    //
    //  And create a correctly adjusted content area so that the pan content area
    //  will fall at zero origin relative to it. This is our relative to drawing
    //  area.
    //
    TArea areaRelTo(m_areaContent);
    areaRelTo.ZeroOrg();
    areaRelTo -= m_areaPanCont.pntOrg();

    //
    //  Reset the remaining bit before we draw into it. For the safe bgn scenario
    //  we need to do a background fill. Since we keep the pannable content left
    //  justified and it fills the whole area, so we don't have to worry about
    //  gradient alignment.
    //
    const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();
    TGraphMemDev gdevCont(gdevCompat, m_bmpCont);

    if (m_bSafePanBgn)
        DoBgnFill(gdevCont, areaUpdate);

    //
    //  We have to clip the output to the actual update area. Otherwise, we could
    //  allow content to be drawn over twice. If that content was drawn with alpha,
    //  it would just add up.
    //
    TRegionJanitor janClip(&gdevCont, areaUpdate, tCIDGraphDev::EClipModes::And);

    DrawItems
    (
        gdevCont
        , areaRelTo
        , areaUpdate
        , TPoint::pntOrigin
        , !m_bSafePanBgn
    );
}


tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::Initialize(TCQCIntfContainer* const    piwdgParent
                                    ,TDataSrvClient&            dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Remember if we have a safe bgn for scrolling. If so, we only
    //  need to create one scrollable bitmap, which will have the bgn
    //  in it. Else, we need the content plus the bgn, which we will
    //  composite together at each new position.
    //
    m_bSafePanBgn = bScrollSafeBgn(kCIDLib::True);

    //
    //  We have to download our template that acts as our layout guide and
    //  process it to remove anything that isn't legal. If we can't find one
    //  this guy will create an empty one.
    //
    LoadTemplate(dsclInit, colErrs);
}



tCIDLib::TVoid TCQCIntfAdvMItemBrowser::Invoke()
{
    //
    //  If we have the focus then we want to invoke commands for the select
    //  event. Set the select index long enough for the RTV object to get
    //  it.
    //
    //  Make sure that the focus slot is a valid item!
    //
    if (bHasFocus() && (m_c4FocusIndex < m_colItems.c4ElemCount()))
    {
        TCardJanitor janOver(&m_c4SelIndex, m_c4FocusIndex);
        PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::NonNested);

        //
        //  Don't do anything else unless you check that we weren't
        //  killed by this action.
        //
    }
}


//
//  Create a new runtime values object. We use one that is in the more basic
//  media item browser class, since it serves our purposes as well.
//
TCQCCmdRTVSrc*
TCQCIntfAdvMItemBrowser::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    if (m_c4SelIndex == kCIDLib::c4MaxCard)
        return new TCQCIntfMIBrwRTV(cuctxToUse);

    return new TCQCIntfMIBrwRTV
    (
        m_colItems[m_c4SelIndex], m_c4SelIndex, cuctxToUse
    );
}


//
//  For now we still have to handle flicks from RIVA clients. We just
//  translates it into a list scroll command to make use of that existing
//  scrolling code.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::ProcessFlick(  const   tCIDLib::EDirs  eDir
                                        , const TPoint&         pntStart)
{
    if ((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Down))
    {
        const tCIDLib::TCh* pszCmd = 0;

        if (eDir == tCIDLib::EDirs::Down)
            pszCmd = L"Prev";
        else if (eDir == tCIDLib::EDirs::Up)
            pszCmd = L"Next";

        if (pszCmd)
            bScrollList(pszCmd, kCIDLib::True);
    }

    //
    //  Do do anything else without making sure the above didn't kill us!
    //
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Change the field we are linked to
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetMoniker
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetMoniker)
                , tCQCKit::ECmdPTypes::Driver
                , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
            )
        );
    }

    // Scroll the list
    if ((eContext != tCQCKit::EActCmdCtx::IVEvent)
    &&  (eContext != tCQCKit::EActCmdCtx::Preload))
    {
        // Scroll the list in various ways
        {
            TCQCCmd& cmdScroll = colCmds.objAdd
            (
                TCQCCmd
                (
                    kCQCKit::strCmdId_ScrollList
                    , facCQCKit().strMsg(kKitMsgs::midCmd_ScrollList)
                    , facCQCKit().strMsg(kKitMsgs::midCmdP_ScrollDir)
                    , L"Bottom, Down, Top, Up, Prev, Next"
                )
            );
            cmdScroll.eType(tCQCKit::ECmdTypes::Both);
        }
    }
}


//
//  If there's a template set we'll query it and get it's size. We'll size our
//  self to its widget (if we can) and to the closest multiple of it's height.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::QueryContentSize(TSize& szToFill)
{
    const TArea& areaCur = areaActual();

    // If not template, just give back our current size
    if (m_strTemplate.bIsEmpty())
    {
        szToFill = areaCur.szArea();
        return;
    }

    // See if we can query the template
    try
    {
        // Expand the path if it's relative
        TString strToLoad;
        piwdgParent()->ExpandTmplPath(m_strTemplate, strToLoad);

        TCQCIntfTemplate iwdgLayout;
        facCQCIntfEng().QueryTemplate
        (
            strToLoad, iwdgLayout, civOwner().cuctxToUse(), kCIDLib::False
        );

        const tCIDLib::TCard4 c4SlotHeight = iwdgLayout.areaActual().c4Height();

        tCIDLib::TCard4 c4Width = iwdgLayout.areaActual().c4Width();
        tCIDLib::TCard4 c4Height = (areaCur.c4Height() / c4SlotHeight) * c4SlotHeight;
        if (!c4Height)
            c4Height = c4SlotHeight;
        szToFill.Set(c4Width, c4Height);
    }

    catch(TError& errToCatch)
    {
        civOwner().ShowErr
        (
            civOwner().strTitle()
            , L"Couldn't access the layout template information"
            , errToCatch
        );

        // Just give back our current size
        szToFill = areaCur.szArea();
    }
}


// Report any monikers we reference
tCIDLib::TVoid TCQCIntfAdvMItemBrowser::
QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);

    // We don't need to pass on to the field mixin. We know what the moniker is
    if (!m_strMoniker.bIsEmpty())
        colToFill.objAdd(m_strMoniker);
}


//
//  Supports the interface editor
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins. Our action isn not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Browser Actions"
    );
    MCQCIntfMultiFldIntf::QueryMFieldAttrs(colAttrs, adatTmp);

    // Add our own stuff
    adatTmp.Set(L"Browser Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);


    // Do our attributes
    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_Media_Repo
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Layout Template"
        , kCQCIntfEng::strAttr_Brws_Layout
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strTemplate);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_Template);
    colAttrs.objAdd(adatTmp);
}


// We just pass this on to our field mixin
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::RegisterFields(        TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfMultiFldIntf::RegisterMultiFields(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfMultiFldIntf::ReplaceMultiFields(strSrc, strTar, bRegEx, bFull, regxFind);

    // If they are replacing template paths, we have to honor that one
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Template))
        facCQCKit().bDoSearchNReplace(strSrc, strTar, m_strTemplate, bRegEx, bFull, regxFind);

    // Do any of our commands
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);

    // And if doing associated devices, update our associated media player moniker
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocDevice))
        facCQCKit().bDoSearchNReplace(strSrc, strTar, m_strMoniker, bRegEx, bFull, regxFind);
}


TSize TCQCIntfAdvMItemBrowser::szDefaultSize() const
{
    return TSize(256, 128);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfMultiFldIntf::SetMFieldAttr(wndAttrEd, adatNew, adatOld);

    // And our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Repo)
        m_strMoniker = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Brws_Layout)
        m_strTemplate = adatNew.strValue();
}


// Report any validation errors in our configuration
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::Validate(  const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And pass on to our field mixin
    MCQCIntfMultiFldIntf::ValidateFields(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bSetMoniker(const TString& strToSet, TString& strReason)
{
    // It has to exist, it has to be a media renderer
    try
    {
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strToSet);

        TCQCDriverObjCfg cqcdcTest;
        if (!orbcSrv->bQueryDriverInfo(strToSet, cqcdcTest))
        {
            strReason = L"Couldn't query driver information for driver ";
            strReason.Append(strToSet);
            return kCIDLib::False;
        }

        //
        //  Get the standard names of the two renderer fields we care about,
        //  in the correct form for the target driver.
        //
        const tCIDLib::TBoolean bV2(cqcdcTest.c4ArchVersion() > 1);
        const TString& strPLIndexFld = TCQCStdMediaRendDrv::strRendFld
        (
            tCQCMedia::ERendFlds::PLIndex, bV2
        );
        const TString& strPLSerNumFld = TCQCStdMediaRendDrv::strRendFld
        (
            tCQCMedia::ERendFlds::PLSerialNum, bV2
        );


        //
        //  If a V2 driver, we just make sure it implements the renderer
        //  class, else we do old style check of the device category.
        //
        if (bV2)
        {
            if (!cqcdcTest.bImplementsClass(tCQCKit::EDevClasses::MediaRenderer))
            {
                strReason = L"Driver ";
                strReason.Append(strToSet);
                strReason.Append(L" does not implement the MediaRenderer class");
                return kCIDLib::False;
            }
        }
         else
        {
            if (cqcdcTest.eCategory() != tCQCKit::EDevCats::MediaRenderer)
            {
                strReason = L"Driver ";
                strReason.Append(strToSet);
                strReason.Append(L" is not a media renderer driver");
                return kCIDLib::False;
            }
        }

        // Make sure the two required fields exist
        tCIDLib::TBoolean bDevFail, bFldFail;
        if (!orbcSrv->bCheckFieldExists(strToSet
                                        , bDevFail
                                        , strPLIndexFld
                                        , bFldFail))
        {
            strReason = L"Driver ";
            strReason.Append(strToSet);
            strReason.Append(L" has no ");
            strReason.Append(strPLIndexFld);
            strReason.Append(L" field");
            return kCIDLib::False;
        }

        if (!orbcSrv->bCheckFieldExists(strToSet
                                        , bDevFail
                                        , strPLSerNumFld
                                        , bFldFail))
        {
            strReason = L"Driver ";
            strReason.Append(strToSet);
            strReason.Append(L" has no ");
            strReason.Append(strPLSerNumFld);
            strReason.Append(L" field");
            return kCIDLib::False;
        }

        //
        //  Looks OK, so set up our field mixin, setting the right names
        //  for V1 vs. V2 mode.
        //
        AssociateField(strToSet, strPLIndexFld, 0);
        AssociateField(strToSet, strPLSerNumFld, 1);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        strReason = errToCatch.strErrText();
        return kCIDLib::False;
    }

    // And store the moniker and indicate we accepted it
    m_strMoniker = strToSet;
    return kCIDLib::True;
}



//
//  Allows us to pass judgement on a template selected by the user during
//  editing. Our attributes tab will call this.
//
tCIDLib::TBoolean TCQCIntfAdvMItemBrowser::
bIsValidTemplate(const  TCQCIntfTemplate&   iwdgTest
                ,       TString&            strReason) const
{
    const tCIDLib::TCard4 c4Count = iwdgTest.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfWidget& iwdgCur = iwdgTest.iwdgAt(c4Index);

        if (!iwdgCur.bIsA(TCQCIntfStaticBooleanImg::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfStaticImg::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfStaticText::clsThis())
        &&  !iwdgCur.bIsA(TCQCIntfMediaRepoText::clsThis()))
        {
            strReason = L"The layout template contains unsupported widget types.";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}



// Get our currently set media renderer moniker
const TString& TCQCIntfAdvMItemBrowser::strMoniker() const
{
    return m_strMoniker;
}

// Get or set the layout template
const TString& TCQCIntfAdvMItemBrowser::strTemplate() const
{
    return m_strTemplate;
}

const TString& TCQCIntfAdvMItemBrowser::strTemplate(const TString& strToSet)
{
    m_strTemplate = strToSet;
    return m_strTemplate;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    // Call our area calculating method to update
    CalcAreas(areaNew, areaOld);
}


//
//  Our field mixin saw a change in our field, so react to that
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::FieldChanged(          TCQCFldPollInfo&    cfpiAssoc
                                        , const tCIDLib::TCard4     c4FldInd
                                        , const tCIDLib::TBoolean   bNoRedraw
                                        , const TStdVarsTar&        ctarGlobalVars)
{
    if (c4FldInd == CQCIntfEng_MItemBrowser::c4FldInd_PLSerialNum)
    {
        // It's the playlist serial number, so reload
        const tCQCPollEng::EFldStates eNewState = eFieldState(c4FldInd);
        tCIDLib::TBoolean bReloaded = kCIDLib::False;
        if (eNewState == tCQCPollEng::EFldStates::Ready)
        {
            if (bLoadItems())
                bReloaded = kCIDLib::True;
        }
         else
        {
            // We lost the field, so clear the list
            m_colItems.RemoveAll();
            bReloaded = kCIDLib::True;
        }

        // If we reloaded and not hidden or supressed, then redraw
        if (bReloaded
        &&  (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
        &&  !bNoRedraw)
        {
            Redraw();
        }
    }
     else if (c4FldInd == CQCIntfEng_MItemBrowser::c4FldInd_PLIndex)
    {
        //
        //  The current playlist item number changed. So we have to redraw
        //  any previously marked one and mark the new one and redraw it.
        //
        TArea areaTar;
        if (m_c4LastCurItem != kCIDLib::c4MaxCard)
        {
            //
            //  Make absolutely sure it's valid. If below the first index, then
            //  it's legal but no need to redraw, and it insures our slot calc
            //  won't underflow. Even if legal, we may still do nothing if the
            //  slot is off the bottom.
            //
            if (m_c4LastCurItem < m_colItems.c4ElemCount())
            {
                AreaForIndex(m_c4LastCurItem, areaTar);

                //
                //  If not empty, then it's at least partially intersecting
                //  the visible area, so update it. We indicate no selection
                //  before we redraw, so that it will unmark.
                //
                //  If there's a new selection, it'll be handled below.
                //
                if (!areaTar.bIsEmpty())
                {
                    m_c4LastCurItem = kCIDLib::c4MaxCard;
                    Redraw(areaTar);
                }
            }
             else
            {
                // Can't be visible, just clear the last current
                m_c4LastCurItem = kCIDLib::c4MaxCard;
            }
        }

        // Store the new current item and draw it if needed
        if (eFieldState(c4FldInd) == tCQCPollEng::EFldStates::Ready)
        {
            // We have a valid value, so store it
            const TCQCCardFldValue& fvCurItem = static_cast<const TCQCCardFldValue&>
            (
                cfpiAssoc.fvCurrent()
            );
            m_c4LastCurItem = fvCurItem.c4Value();

            //
            //  If not max card, then update the new current item. We only need
            //  to do it if it's legal and not above the top. even then we may
            //  still not do anything if it's off the bottom. But this insures
            //  that our calculation of the slot can't go negative as well.
            //
            if (m_c4LastCurItem < m_colItems.c4ElemCount())
            {
                AreaForIndex(m_c4LastCurItem, areaTar);

                //
                //  If not empty, then it's at least partially intersecting
                //  the visible area, so update it.
                //
                if (!areaTar.bIsEmpty())
                    Redraw(areaTar);
            }
        }
    }
}


//
//  If we are getting focus, then we show the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we have the focus.
//
tCIDLib::TVoid TCQCIntfAdvMItemBrowser::GotFocus()
{
    // Just force a redraw of the slot with the focus
    TArea areaUpdate;
    AreaForIndex(m_c4FocusIndex, areaUpdate);
    Redraw(areaUpdate);
}


//
//  If we are losing the focus, we remove the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we don't have the focus.
//
tCIDLib::TVoid TCQCIntfAdvMItemBrowser::LostFocus()
{
    // Just force a redraw of the slot with the focus
    TArea areaUpdate;
    AreaForIndex(m_c4FocusIndex, areaUpdate);
    Redraw(areaUpdate);
}


//
//  This is called if our size changes.
//
//  If we get smaller, that's not an issue, but if we get larger, and we
//  were already all the way to the right, or suddenly we got bigger than
//  the list, we have to adjust.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);

    // Recalculate our areas and adjust scrolling if needed
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    CalcAreas(areaNew, areaOld);
}


tCIDLib::TVoid TCQCIntfAdvMItemBrowser::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MItemBrowser::c2FmtVersion))
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

    // Call our parent
    TParent::StreamFrom(strmToReadFrom);

    // Do any mixins
    MCQCIntfMultiFldIntf::ReadInFields(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom  >> m_strTemplate
                    >> m_strMoniker;

    // Convert the path if needed
    if (c2FmtVersion < 2)
        facCQCKit().Make50Path(m_strTemplate);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Clear the list until we get good data
    m_colItems.RemoveAll();
}


tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_MItemBrowser::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do our mixins
    MCQCIntfMultiFldIntf::WriteOutFields(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Do our stuff and end the end marker
    strmToWriteTo   << m_strTemplate
                    << m_strMoniker
                    << tCIDLib::EStreamMarkers::EndObject;
}



tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::Update(        TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    // Call our parent first to do the standard stuff
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // If in designer mode, then nothing more to do
    if (TFacCQCIntfEng::bDesMode())
        return;

    // Add our display area to the clip region
    TRegionJanitor janClip(&gdevTarget, m_areaDisplay, tCIDGraphDev::EClipModes::And);

    // If we have any items, do an update
    const tCIDLib::TCard4 c4ItemCnt = m_colItems.c4ElemCount();
    if (c4ItemCnt)
    {
        TArea areaRelTo;
        MakeScrolledContArea(areaRelTo);

        // The udpate area is the intersect of the display and invalid areas
        TArea areaUpdate(m_areaDisplay);
        areaUpdate &= areaInvalid;

        DrawItems
        (
            gdevTarget
            , areaRelTo
            , areaUpdate
            , TPoint::pntOrigin
            , kCIDLib::False
        );
    }
}


// We just pass this on to our field mixin
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::ValueUpdate(       TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}



// ---------------------------------------------------------------------------
//  TCQCIntfAdvMItemBrowser: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Calculate the position for the passed index. We have to be able to do it
//  both for display coordinates and scroll buffer coordinates. We hve one
//  that defaults to the display coordinate stuff, which calls the other one
//  that takes arbitrary coordinate systems.
//
//  The relative to is the area where the content is being drawn. It's either
//  the content area, or whatever zero based buffer we are drawing scrollable
//  content into.
//
tCIDLib::TVoid TCQCIntfAdvMItemBrowser
::AreaForIndex(const tCIDLib::TCard4 c4Index, TArea& areaToFill) const
{
    // Create a scrolled version of the content area and pass to the other one
    TArea areaCont;
    MakeScrolledContArea(areaCont);
    AreaForIndex(c4Index, areaCont, areaToFill);
}

tCIDLib::TVoid
TCQCIntfAdvMItemBrowser
::AreaForIndex( const   tCIDLib::TCard4 c4Index
                , const TArea&          areaRelTo
                ,       TArea&          areaToFill) const
{
    //
    //  Start with the target area. However, it may be empty, so we only do
    //  this to get the origin. If empty, its origin will be our return origin.
    //
    areaToFill = areaRelTo;

    // How far from the content origin is this one
    const tCIDLib::TCard4 c4Size(m_iwdgLayout.areaRelative().c4Height());
    const tCIDLib::TInt4 i4Pos(c4Index * c4Size);

    //
    //  Adjust up from the display area origin by this much. SEt it to the
    //  display area width and the layout height.
    //
    areaToFill.AdjustOrg(0, i4Pos);
    areaToFill.SetSize(m_areaDisplay.c4Width(), c4Size);
}


//
//  This is called to get us to download our list of items from the
//  associated renderer.
//
tCIDLib::TBoolean TCQCIntfAdvMItemBrowser::bLoadItems()
{
    try
    {
        // Get a CQCServer proxy for our renderer driver
        tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);

        //
        //  Query the item list data. The collection cookie field is
        //  not used in this case, so we pass an empty string.
        //
        const tCIDLib::TCard4 c4RetItems = facCQCMedia().c4QueryPLItems
        (
            orbcSrv, m_strMoniker, m_colItems
        );

        //
        //  Go through them and clear the played field, because we use this as
        //  as a pressed indicator.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4RetItems; c4Index++)
            m_colItems[c4Index].bPlayed(kCIDLib::False);

        //
        //  Reload the current active item value now so we'll redraw with it
        //  set correctly. Otherwise we wouldn't update it until it changed,
        //  and our current value may be invalid for the new list.
        //
        if (bGoodFieldValue(CQCIntfEng_MItemBrowser::c4FldInd_PLIndex))
        {
            const TCQCCardFldValue& fvCurItem = static_cast<const TCQCCardFldValue&>
            (
                fvCurrent(CQCIntfEng_MItemBrowser::c4FldInd_PLIndex)
            );
            m_c4LastCurItem = fvCurItem.c4Value();
        }
         else
        {
            m_c4LastCurItem = kCIDLib::c4MaxCard;
        }

        // Adjust the focus index so that it's at least legal
        if (c4RetItems)
        {
            if (m_c4FocusIndex >= c4RetItems)
                m_c4FocusIndex = c4RetItems - 1;
        }
         else
        {
            m_c4FocusIndex = 0;
        }

        //
        //  Update our content area. This will also update the focus index to
        //  keep it visible if needed. Pass a bogus old area, so that it'll do
        //  something.
        //
        CalcAreas(areaActual(), TArea::areaEmpty);
    }

    catch(const TError& errToCatch)
    {
        // Clear the list in case it got partially set
        m_c4LastCurItem = kCIDLib::c4MaxCard;
        m_colItems.RemoveAll();
        m_c4FocusIndex = 0;

        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Tell the user it failed
        civOwner().ShowErr
        (
            civOwner().strTitle()
            , facCQCIntfEng().strMsg
              (
                kIEngMsgs::midStatus_CantLoadItems, strMoniker()
              )
            , errToCatch
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called when we get a scroll command. We have to adjust the
//  content area position appropriately.
//
//  We return true if we actually changed the top index.
//
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bScrollList(const  TString&            strScrollOp
                                    , const tCIDLib::TBoolean   bCanRedraw)
{
    // If the content area no bigger than us, then nothing to do.
    if (m_areaDisplay.c4Height() >= m_areaContent.c4Height())
        return kCIDLib::False;

    // Remember some info for later
    const TArea areaOrg = m_areaAdjDisplay;
    const tCIDLib::TInt4 i4Span = tCIDLib::TInt4(m_areaDisplay.c4Height());;

    //
    //  Calc a new scroll position according to the command.
    //
    if (strScrollOp == L"Top")
    {
        m_areaAdjDisplay.TopJustifyIn(m_areaContent);
    }
     else if (strScrollOp == L"Bottom")
    {
        m_areaAdjDisplay.BottomJustifyIn(m_areaContent);
    }
     else if (strScrollOp == L"Up")
    {
        // Move the display area upwards a quarter page
        m_areaAdjDisplay.AdjustOrg(0, -(i4Span / 4));
    }
     else if (strScrollOp == L"Down")
    {
        // Move the display area downwards a quarter page
        m_areaAdjDisplay.AdjustOrg(0, i4Span / 4);
    }
     else if ((strScrollOp == L"Next") ||  (strScrollOp == L"Prev"))
    {
        const tCIDLib::TBoolean bNext(strScrollOp == L"Next");

        // Go almost a full page but not quite
        if (bNext)
            m_areaAdjDisplay.AdjustOrg(0, i4Span - (i4Span >> 2));
        else
            m_areaAdjDisplay.AdjustOrg(0, -(i4Span - (i4Span >> 2)));
    }

    //
    //  Clip the movement to the limits, and see if we actually moved
    //  or not.
    //
    if (m_areaAdjDisplay.i4Y() < m_areaContent.i4Y())
        m_areaAdjDisplay.i4Y(m_areaContent.i4Y());
    if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
        m_areaAdjDisplay.BottomJustifyIn(m_areaContent);

    //
    //  If the position changed, and we can redraw, then do it. If in
    //  remote mode, just redraw where we are. Else do a sliding scroll.
    //
    if (m_areaAdjDisplay != areaOrg)
    {
        if (TFacCQCIntfEng::bRemoteMode())
        {
            // Keep the focus index visible
            KeepFocusVisible();
            Redraw();
        }
         else
        {
            SlideList(areaOrg);
        }
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This method will test whether the indicated index is 'visible enough',
//  which is used to decide whether to allow a click on an item or to move
//  the focus to it. We don't want to allow items that might only have a
//  pixel or two visible to be selected in these types of cases.
//
//  It is assumed that the index at least within one item's size of the
//  display area. Else it won't get up to a usefully visible item index.
//
//  If it's not, then the passed index will be adjusted if that is possible
//  to reflect the next closest one that is visible enough.
//
tCIDLib::TBoolean
TCQCIntfAdvMItemBrowser::bVisibleEnough(        tCIDLib::TCard4&    c4Index
                                        , const tCIDLib::TFloat4    f4Per) const
{
    TArea areaTest;
    AreaForIndex(c4Index, areaTest);

    //
    //  How much of an item has to be visible before we consider it a
    //  visible one for our purposes.
    //
    //  They give us a percent that has to be visible. Our stuff below
    //  measures how much is not visible, so flip it.
    //
    const tCIDLib::TCard4 c4LOSize = m_iwdgLayout.areaActual().c4Height();
    const tCIDLib::TInt4 i4Limit = tCIDLib::TInt4(c4LOSize * (1.0 - f4Per));

    // Remember if we modified it
    const tCIDLib::TCard4 c4OrgVal = c4Index;

    // if not enough is visible, then move inwards if possible
    if (areaTest.i4Y() < m_areaDisplay.i4Y())
    {
        if (m_areaDisplay.i4Y() - areaTest.i4Y() >= i4Limit)
        {
            if (c4Index + 1 < m_colItems.c4ElemCount())
               c4Index++;
        }
    }
     else if (areaTest.i4Bottom() > m_areaDisplay.i4Bottom())
    {
        if ((areaTest.i4Bottom() - m_areaDisplay.i4Bottom() >= i4Limit)
        &&  c4Index)
        {
            c4Index--;
        }
    }

    // If we didn't have to change it, return true
    return (c4OrgVal == c4Index);
}



//
//  Capture the background under us, plus our background, but no foreground
//  list contents. This is used in the inertial scrolling methods.
//
//  We only capture the display area, and we have to insure that any gradients
//  align with our overall area background fill.
//
TBitmap TCQCIntfAdvMItemBrowser::bmpGatherBgn()
{
    //
    //  Get the stuff underneath our display area. We tell it to stop when
    //  it hits our widget, and not to draw it (stop before the one we
    //  indicate.)
    //
    TBitmap bmpBgn = civOwner().bmpQueryContentUnder
    (
        m_areaDisplay, this, kCIDLib::True
    );

    // Draw our background content (if any) over that
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

    //
    //  Do our background fill if any over that. We insure alighment of
    //  gradients by centering an area of our overall size over our display
    //  area and filling that.
    //
    TArea areaFill = areaActual();
    areaFill.CenterIn(m_areaDisplay);
    DoBgnFill(gdevBgn, areaFill);

    // Create a bitmap the size of our display area
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );

    //
    //  And copy the bgn contents within the drawing area over into our
    //  return bitmap.
    //
    {
        TGraphMemDev gdevRet(civOwner().gdevCompat(), bmpRet);

        TArea areaTar(m_areaDisplay);
        areaTar.ZeroOrg();
        gdevRet.CopyBits(gdevBgn, m_areaDisplay, areaTar);
    }
    return bmpRet;
}


//
//  Return the closest index to the passed from here index that is mostly
//  visible within our display area. This is for keeping a certain index
//  visible.
//
tCIDLib::TCard4
TCQCIntfAdvMItemBrowser::c4ClosestVisIndex(const tCIDLib::TCard4 c4FromHere) const
{
    // Get the area of the from index
    TArea areaTest;
    AreaForIndex(c4FromHere, areaTest);

    // If it is fully contained within the display area, then it's keeper as is
    if (m_areaDisplay.bContainsArea(areaTest))
        return c4FromHere;

    //
    //  If it doesn't intersect, then it's completely outside, so move to the
    //  first one that's at least reasonably visible. If it does intersect,
    //  then it's a partially visible one, so either way we'll then check
    //  below to make sure it's visible enough.
    //
    const tCIDLib::TCard4 c4SlotHeight = m_iwdgLayout.areaActual().c4Height();
    tCIDLib::TCard4 c4Ret = c4FromHere;
    if (!m_areaDisplay.bIntersects(areaTest))
    {
        if (areaTest.i4Y() < m_areaDisplay.i4Y())
            c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Y()) / c4SlotHeight;
        else
            c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Bottom()) / c4SlotHeight;
    }

    //
    //  Call the visible enough method which will modify the index if needed.
    //  We don't care if it does or doesn't. We require it to be at least
    //  40% visible.
    //
    bVisibleEnough(c4Ret, 0.4F);

    return c4Ret;
}



//
//  Calc the first (at least partially) visible index. We have one that
//  takes a visible area and an area it is relative to, which we need in
//  order to deal with both regular display and drawing into scrollable
//  buffers. The first just passes into the second the normal values.
//
tCIDLib::TCard4 TCQCIntfAdvMItemBrowser::c4FirstVisIndex() const
{
    // If no items, it has to be zero
    if (m_colItems.bIsEmpty())
        return 0;

    //
    //  In this case, we can just use the scroll offset, since it tells
    //  us how far we are scrolled, independent of orientation. It's always
    //  a positive number.
    //
    return tCIDLib::TCard4(m_areaAdjDisplay.i4Y())
           / m_iwdgLayout.areaRelative().c4Height();
}

tCIDLib::TCard4
TCQCIntfAdvMItemBrowser::c4FirstVisIndex(const  TArea&  areaVis
                                        , const TArea&  areaRelTo) const
{
    // If no items, it has to be zero
    if (m_colItems.bIsEmpty())
        return 0;

    // See how far we are scrolled from the verical content origin
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4(areaVis.i4Y() - areaRelTo.i4Y());

    // And we can just divide to get the indx
    return (c4Diff / m_iwdgLayout.areaRelative().c4Height());
}


//
//  Given a point, figure out which item was hit.
//
tCIDLib::TCard4
TCQCIntfAdvMItemBrowser::c4HitTest(const TPoint& pntTest) const
{
    //
    //  Get the count of items. If none, then can't have hit a used slot.
    //  Or, if not within our content area.
    //
    const tCIDLib::TCard4 c4Names = m_colItems.c4ElemCount();
    if (!c4Names || !m_areaDisplay.bContainsPoint(pntTest))
        return kCIDLib::c4MaxCard;

    //
    //  See how far this point is from our content origin. We get the point's
    //  distance from the display area top, and then add the scroll offset
    //  which is how much further it is from the actual origin of the
    //  content.
    //
    tCIDLib::TCard4 c4Diff = tCIDLib::TCard4
    (
        (pntTest.i4Y() - m_areaDisplay.i4Y()) + m_areaAdjDisplay.i4Y()
    );

    // And we can now just divide
    tCIDLib::TCard4 c4Index(c4Diff / m_iwdgLayout.areaRelative().c4Height());

    // If it's beyond the end of the list, we failed, else return it
    if (c4Index >= m_colItems.c4ElemCount())
        return kCIDLib::c4MaxCard;

    return c4Index;
}



//
//  This is called when the list is reloaded or some other configuration
//  changes that would affect our content or display size. We re-calc our
//  areas.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::CalcAreas(const TArea& areaNew, const TArea& areaOld)
{
    //
    //  Figure out our display area. Start with our area, then move inwards
    //  for the border if we have one.
    //
    m_areaDisplay = areaNew;
    if (bHasBorder())
        m_areaDisplay.Deflate(2);

    // If we have any rounding, then move in by one more.
    if (c4Rounding())
        m_areaDisplay.Deflate(1);

    //
    //  If the layout is not as wide as us, then adjust to only that wide
    //  so that we don't scroll stuff we don't need to.
    //
    if (m_iwdgLayout.areaRelative().c4Width() < m_areaDisplay.c4Width())
        m_areaDisplay.c4Width(m_iwdgLayout.areaRelative().c4Width());

    //
    //  Calculate the content area. If no content, then we are zero sized
    //  at zero origin, so start with that as the default until proven
    //  otherwise below.
    //
    m_areaContent.ZeroAll();

    // If we have any names, then we may need to adjust it
    const tCIDLib::TCard4 c4Count = m_colItems.c4ElemCount();
    if (c4Count)
    {
        //
        //  Set the content to the width of the display area, and the full
        //  height of all of the layouts to contain all the items. If the
        //  layout is wider than the display area, it doesn't matter. We
        //  will never display any of the remainder and don't want to bother
        //  drawing anything out there.
        //
        m_areaContent.SetSize
        (
            m_areaDisplay.c4Width()
            , m_iwdgLayout.areaRelative().c4Height() * c4Count
        );
    }

    //
    //  Update our adjusted display to the same size as the regular
    //  display area, or the content size if that is smaller in the
    //  vertical direction.
    //
    if (m_areaDisplay.c4Height() > m_areaContent.c4Height())
    {
        m_areaAdjDisplay.SetSize
        (
            m_areaDisplay.c4Width(), m_areaContent.c4Height()
        );

        // No need to scroll anymore. Reset back to the origin
        m_areaAdjDisplay.i4Y(0);
    }
     else
    {
        m_areaAdjDisplay.SetSize(m_areaDisplay.szArea());

        // Make sure we aren't hanging of the end now
        if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
            m_areaAdjDisplay.BottomJustifyIn(m_areaContent);
    }

    // Keep the focus index visible
    KeepFocusVisible();
}


//
//  When we need to draw a repo text widget, this is called. We look at the
//  attributes of the widget, get the value formatted out, and draw it using
//  the text settings of the widget.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::DrawRepoText(          TGraphDrawDev&          gdevTarget
                                        ,       TCQCIntfMediaRepoText&  iwdgDraw
                                        , const tCIDLib::TCard4         c4Index
                                        ,       TArea&                  areaDraw
                                        , const tCIDLib::TBoolean       bDoAlpha
                                        ,       TBitmap&                bmpBuf
                                        ,       TBitmap&                bmpMask)
{
    const TCQCMediaPLItem& mpliCur = m_colItems[c4Index];

    // Get the correct text, leave ??? in it if not available
    m_strTmp1 = L"???";
    switch(iwdgDraw.eAttr())
    {
        case tCQCMedia::EMTextAttrs::Artist :
            m_strTmp1 = mpliCur.strItemArtist();
            break;

        case tCQCMedia::EMTextAttrs::AspectRatio :
            m_strTmp1 = mpliCur.strColAspectRatio();
            break;

        case tCQCMedia::EMTextAttrs::BitDepth :
            m_strTmp1.SetFormatted(mpliCur.c4BitDepth());
            break;

        case tCQCMedia::EMTextAttrs::BitRate :
            m_strTmp1.SetFormatted(mpliCur.c4BitRate());
            break;

        case tCQCMedia::EMTextAttrs::Cast :
            m_strTmp1 = mpliCur.strColCast();
            break;

        case tCQCMedia::EMTextAttrs::Channels :
            m_strTmp1.SetFormatted(mpliCur.c4Channels());
            break;

        case tCQCMedia::EMTextAttrs::DateAdded :
            break;

        case tCQCMedia::EMTextAttrs::Description :
            m_strTmp1 = mpliCur.strColDescription();
            break;

        case tCQCMedia::EMTextAttrs::Label :
            m_strTmp1 = mpliCur.strColLabel();
            break;

        case tCQCMedia::EMTextAttrs::LeadActor :
            break;

        case tCQCMedia::EMTextAttrs::MediaFmt :
            m_strTmp1 = mpliCur.strMediaFormat();
            break;

        case tCQCMedia::EMTextAttrs::Name :
            m_strTmp1 = mpliCur.strItemName();
            break;

        case tCQCMedia::EMTextAttrs::Rating :
            m_strTmp1 = mpliCur.strColRating();
            break;

        case tCQCMedia::EMTextAttrs::RawRunTime :
            m_strTmp1.SetFormatted(mpliCur.c4ItemSeconds());
            break;

        case tCQCMedia::EMTextAttrs::RunTime :
        {
            const tCIDLib::TCard4 c4Hours = (mpliCur.c4ItemSeconds() / 3600);
            const tCIDLib::TCard4 c4Mins
            (
                (mpliCur.c4ItemSeconds() - (c4Hours * 3600)) / 60
            );

            if (!c4Hours)
            {
                // There's no hours, so format it as xx mins, xx secs
                m_strTmp1.SetFormatted(c4Mins);
                m_strTmp1.Append(L" mins, ");

                const tCIDLib::TCard4 c4Secs = mpliCur.c4ItemSeconds() - (c4Mins * 60);
                m_strTmp1.AppendFormatted(c4Secs);
                m_strTmp1.Append(L" secs");
            }
             else
            {
                // We have at least one hours so do H:MM format
                m_strTmp1.SetFormatted(c4Hours);
                m_strTmp1.Append(TLocale::chTimeSeparator());
                if (c4Mins < 10)
                    m_strTmp1.Append(kCIDLib::chDigit0);
                m_strTmp1.AppendFormatted(c4Mins);
            }
            break;
        }

        case tCQCMedia::EMTextAttrs::SampleRate :
            break;

        case tCQCMedia::EMTextAttrs::SeqNum :
            break;

        case tCQCMedia::EMTextAttrs::Title :
            m_strTmp1 = mpliCur.strTitleName();

            // If the collection name isn't the same, then append it
            if (m_strTmp1 != mpliCur.strColName())
            {
                m_strTmp1.Append(L" - ");
                m_strTmp1.Append(mpliCur.strColName());
            }

            break;

        case tCQCMedia::EMTextAttrs::UserRating :
            m_strTmp1 = mpliCur.strColRating();
            break;

        case tCQCMedia::EMTextAttrs::Year :
            m_strTmp1.SetFormatted(mpliCur.c4ColYear());
            break;

        default :
            break;
    };


    // Draw the text using the text settings of the guide widget
    DrawText
    (
        gdevTarget
        , iwdgDraw
        , static_cast<MCQCIntfFontIntf&>(iwdgDraw)
        , m_strTmp1
        , areaDraw
        , bDoAlpha
        , bmpBuf
        , bmpMask
    );
}


//
//  Called to draw a single slot. We get the area to draw, with is the slot
//  area clipped to the visible area. So anything not within that we don't
//  need to draw. We also get the invalid area. We are being drawn because our
//  draw area intersects that, but it might only be slightly. So anything not
//  in the invalid area we also don't have to draw.
//
//  The widgets are not real, initialized widgets, so the areaActual() isn't
//  useful. We use their relative areas and have to add that to the item
//  slot area. The areaDraw, though clipped to the widget area, is never clipped
//  on the left or top, so the original is always the correct slot origin.
//
//  If we are pressed (which means we are the slot being tracked and the mouse
//  is currently over us), then we move everything down another pixel.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::DrawItem(          TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaDraw
                                    , const TArea&              areaInvalid
                                    , const tCIDLib::TCard4     c4ItemInd
                                    , const tCIDLib::TBoolean   bDoAlpha
                                    ,       TBitmap&            bmpBuf
                                    ,       TBitmap&            bmpMask
                                    , const tCIDLib::TBoolean   bDoFocus)
{
    //
    //  We need to go through the widgets in the layout. For each one, we
    //  use its defined attributes to daw it, if it intersects both areas.
    //
    const tCIDLib::TBoolean bPressed = m_colItems[c4ItemInd].bPlayed();

    TArea areaCur;
    const tCIDLib::TCard4 c4Count = m_iwdgLayout.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfWidget& iwdgCur = m_iwdgLayout.iwdgAt(c4Index);
        areaCur = iwdgCur.areaRelative();
        areaCur.AdjustOrg(areaDraw.pntOrg());

        //
        //  If this doesn't intersect both areas, ignore it. It's more likely
        //  not to intersect the invalid area, so we do that first.
        //
        if (!areaCur.bIntersects(areaInvalid) || !areaCur.bIntersects(areaDraw))
            continue;

        // Save the device state around each round
        TGraphicDevJanitor janDev(&gdevTarget);

        // Not that these guys can modify the area
        if (iwdgCur.clsIsA() == TCQCIntfStaticBooleanImg::clsThis())
        {
            //
            //  These are used to show things like pressed state, or currenly
            //  playing item status. They have to have specific names to
            //  indicate what role they play. Any that didn't have these names
            //  were removed during the load.
            //
            //  So we have to look at the name and figure out which of their
            //  images to display.
            //
            TCQCIntfStaticBooleanImg& iwdgImg(static_cast<TCQCIntfStaticBooleanImg&>(iwdgCur));

            const tCIDLib::TCh* pszKey = kCQCKit::pszFld_False;
            if (iwdgImg.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_Pressed)
            {
                if (bPressed)
                    pszKey = kCQCKit::pszFld_True;
            }
             else if (iwdgImg.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_CurItem)
            {
                if (c4ItemInd == m_c4LastCurItem)
                    pszKey = kCQCKit::pszFld_True;
            }
             else if (iwdgImg.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_Focus)
            {
                // If htis is the focus guy, then show it
                if (bDoFocus)
                    pszKey = kCQCKit::pszFld_True;
            }

            // If we got a key, look it up. If we find it, display the image
            if (pszKey)
            {
                MCQCIntfImgListIntf::TImgInfo* pimgiCur = iwdgImg.pimgiForKeyEO(pszKey);
                if (pimgiCur)
                {
                    facCQCIntfEng().DrawImage
                    (
                        gdevTarget
                        , bDoAlpha
                        , pimgiCur->m_cptrImg
                        , areaCur
                        , areaInvalid
                        , pimgiCur->m_i4HOffset
                        , pimgiCur->m_i4VOffset
                        , pimgiCur->m_c1Opacity
                    );
                }
            }
        }
         else if (iwdgCur.clsIsA() == TCQCIntfMediaRepoText::clsThis())
        {
            DrawRepoText
            (
                gdevTarget
                , static_cast<TCQCIntfMediaRepoText&>(iwdgCur)
                , c4ItemInd
                , areaCur
                , bDoAlpha
                , bmpBuf
                , bmpMask
            );
        }
         else if (iwdgCur.clsIsA() == TCQCIntfStaticImg::clsThis())
        {
            TCQCIntfStaticImg& iwdgImg(static_cast<TCQCIntfStaticImg&>(iwdgCur));
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , bDoAlpha
                , iwdgImg.cptrImage()
                , areaCur
                , areaInvalid
                , iwdgImg.ePlacement()
                , 0
                , 0
                , iwdgImg.c1Opacity()
            );
        }
         else if (iwdgCur.clsIsA() == TCQCIntfStaticText::clsThis())
        {
            //
            //  Nothing special here, just pass it to the text drawing
            //  helper class.
            //
            TCQCIntfStaticText& iwdgText(static_cast<TCQCIntfStaticText&>(iwdgCur));
            DrawText
            (
                gdevTarget
                , iwdgText
                , static_cast<MCQCIntfFontIntf&>(iwdgText)
                , iwdgText.strCaption()
                , areaCur
                , bDoAlpha
                , bmpBuf
                , bmpMask
            );
        }
    }
}



//
//  Draws a number of slots, starting at the indicated index within the list,
//  at the indicated starting origin. The origin is required because we have
//  to support drawing into an origin based image for inertial scrolling
//  purposes.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::DrawItems(         TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaRelTo
                                    , const TArea&              areaInvalid
                                    , const TPoint&             pntOffset
                                    , const tCIDLib::TBoolean   bDoAlpha)
{
    // Set up for drawing as required
    gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

    //
    //  Figure out which item will be the first one visible within this
    //  invalid area. It might only be partially so. And get its area.
    //  We'll move forward from there until we go beyond the invalid
    //  area.
    //
    tCIDLib::TCard4 c4Index = c4FirstVisIndex(areaInvalid, areaRelTo);
    TArea areaCur;
    AreaForIndex(c4Index, areaRelTo, areaCur);

    //
    //  Remember if we should bother doing focus, If we have a focus indicator
    //  and focus is enabled, and we have the focus.
    //
    const tCIDLib::TBoolean bDoFocus
    (
        m_bHasFocusInd
        && !iwdgRootTemplate().bNoFocus()
        && bHasFocus()
    );

    //
    //  Create an adjusted invalid area so it's correct relative to the
    //  adjusted position.
    //
    TArea areaAdjInvalid(areaInvalid);
    areaAdjInvalid += pntOffset;

    //
    //  If we are doing alpha or f/x text, we have to create bitmaps for the
    //  text drawing methods to use.
    //
    const TArea& areaLayout = m_iwdgLayout.areaActual();
    TBitmap bmpBuf
    (
        areaLayout.szArea()
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
    );

    TBitmap bmpMask
    (
        areaLayout.szArea()
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
    );

    //
    //  Loop till we move past the target area or max out on items.
    //
    TArea areaAdj;
    const tCIDLib::TCard4 c4Count  = m_colItems.c4ElemCount();
    const tCIDLib::TCard4 c4Height = areaLayout.c4Height();
    while (c4Index < c4Count)
    {
        if (areaCur.i4Y() > areaInvalid.i4Bottom())
            break;

        if (areaCur.bIntersects(areaInvalid))
        {
            areaAdj = areaCur;
            areaAdj += pntOffset;
            DrawItem
            (
                gdevTarget
                , areaAdj
                , areaAdjInvalid
                , c4Index
                , bDoAlpha
                , bmpBuf
                , bmpMask
                , bDoFocus && (m_c4FocusIndex == c4Index)
            );
        }

        // Move the index and area downward by the slot heght
        c4Index++;
        areaCur.AdjustOrg(0, c4Height);
    }
}


//
//  This handles the text drawing grunt work and will work for any of the
//  text widgets. The caller has to get the font mixin out for us since
//  we can't access it due to getting the widget via the base widget class.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::DrawText(          TGraphDrawDev&      gdevTarget
                                    , const TCQCIntfWidget&     iwdgSrc
                                    , const MCQCIntfFontIntf&   miwdgFont
                                    , const TString&            strText
                                    ,       TArea&              areaTar
                                    , const tCIDLib::TBoolean   bDoAlpha
                                    ,       TBitmap&            bmpBuf
                                    ,       TBitmap&            bmpMask)
{
    //
    //  The static text widget does this so we need to in order for our
    //  output to match the layout template's appearance.
    //
    areaTar.Deflate(3, 2);

    // Set the widget's font while in here
    TCQCIntfFontJan janFont(&gdevTarget, miwdgFont);

    TCQCIntfView& civUs = civOwner();
    if (miwdgFont.eTextFX() != tCIDGraphDev::ETextFX::None)
    {
        civUs.DrawTextFX
        (
            gdevTarget
            , strText
            , areaTar
            , miwdgFont.eTextFX()
            , iwdgSrc.rgbFgn()
            , iwdgSrc.rgbFgn2()
            , miwdgFont.eHJustify()
            , miwdgFont.eVJustify()
            , miwdgFont.bNoTextWrap()
            , miwdgFont.pntOffset()
        );
    }
     else
    {
        //
        //  Now draw our text. If the shadow style is enabled, then draw
        //  first a drop version in fgn3, then draw the regular one in
        //  fgn1 in the normal position.
        //
        if (iwdgSrc.bShadow())
        {
            tCIDLib::TCard4 c4Drop = 1;
            if (miwdgFont.c4FontHeight() > 24)
                  c4Drop = 2;
            TArea areaDrop(areaTar);
            areaDrop.AdjustOrg(c4Drop, c4Drop);
            gdevTarget.SetTextColor(iwdgSrc.rgbFgn2());
            if (miwdgFont.bNoTextWrap())
            {
                if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
                {
                    gdevTarget.DrawString
                    (
                        strText, areaDrop, miwdgFont.eHJustify(), miwdgFont.eVJustify()
                    );
                }
                 else
                {
                    civUs.DrawTextAlpha
                    (
                        gdevTarget
                        , strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                    );
                }
            }
             else
            {
                if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
                {
                    gdevTarget.DrawMText
                    (
                        strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                    );
                }
                 else
                {
                    civUs.DrawMTextAlpha
                    (
                        gdevTarget
                        , strText
                        , areaDrop
                        , miwdgFont.eHJustify()
                        , miwdgFont.eVJustify()
                    );
                }
            }
        }

        gdevTarget.SetTextColor(iwdgSrc.rgbFgn());
        if (miwdgFont.bNoTextWrap())
        {
            if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
            {
                gdevTarget.DrawString
                (
                    strText, areaTar, miwdgFont.eHJustify(), miwdgFont.eVJustify()
                );
            }
             else
            {
                civUs.DrawTextAlpha
                (
                    gdevTarget
                    , strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                );
            }
        }
         else
        {
            if (!bDoAlpha || TFacCQCIntfEng::bRemoteMode())
            {
                gdevTarget.DrawMText
                (
                    strText, areaTar, miwdgFont.eHJustify(), miwdgFont.eVJustify()
                );
            }
             else
            {
                civUs.DrawMTextAlpha
                (
                    gdevTarget
                    , strText
                    , areaTar
                    , miwdgFont.eHJustify()
                    , miwdgFont.eVJustify()
                );
            }
        }
    }
}



//
//  Any time anything changes that might make the focus index go out of
//  visibility, we need to get it back visible. It's not a lot of code but
//  it's called from a number of places, so best to keep it in one place.
//
tCIDLib::TVoid TCQCIntfAdvMItemBrowser::KeepFocusVisible()
{
    TArea areaFocus;
    AreaForIndex(m_c4FocusIndex, areaFocus);

    //
    //  If it's not fully visible, then move to the closest one
    //  that is visible.
    //
    if (!m_areaDisplay.bContainsArea(areaFocus))
        m_c4FocusIndex = c4ClosestVisIndex(m_c4FocusIndex);
}


//
//  Called during initialization to get our template loaded, and cleaned up
//  if it has stuff we can't use. If we can't downlaod it, we create a default
//  empty one, which will basically have us displaying nothing.
//
//  If we load any boolean or static image widgets, though it's a little
//  kludgey, we initialize their image (or image list) mixins, so that it
//  will load the images referenced by those widgets. Otherwise we'd have to
//  do a lot of work to manage them ourself.
//
//  It's a litte kludgey because these are not real widgets, but the mixins
//  by definition don't have much knowledge of things they are mixed into. So
//  just trying to load the images won't do anything bad.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::LoadTemplate(  TDataSrvClient&             dsclToUse
                                        , tCQCIntfEng::TErrList&    colErrs)
{
    try
    {
        TCQCIntfView& civUs = civOwner();

        // Query the layout template. Expand the path in case it's relative
        TString strToLoad;
        piwdgParent()->ExpandTmplPath(m_strTemplate, strToLoad);
        facCQCIntfEng().QueryTemplate
        (
            strToLoad
            , dsclToUse
            , m_iwdgLayout
            , civUs.cuctxToUse()
            , civUs.bEnableTmplCaching()
        );

        // Assume no focus indicator until proven otherwise
        m_bHasFocusInd = kCIDLib::False;

        // Let's go through and remove anything that isn't kosher
        tCIDLib::TCard4 c4Count = m_iwdgLayout.c4ChildCount();
        tCIDLib::TCard4 c4Index = 0;
        while (c4Index < c4Count)
        {
            TCQCIntfWidget& iwdgCur = m_iwdgLayout.iwdgAt(c4Index);

            if ((iwdgCur.clsIsA() == TCQCIntfStaticBooleanImg::clsThis())
            &&  ((iwdgCur.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_CurItem)
            ||   (iwdgCur.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_Focus)
            ||   (iwdgCur.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_Pressed)))
            {
                //
                //  These have to be named with specific names, to indicate
                //  their function. Any that don't have those names are
                //  removed. These that we keep we have to get them to load
                //  their image list.
                //
                TCQCIntfStaticBooleanImg& iwdgImg
                (
                    static_cast<TCQCIntfStaticBooleanImg&>(iwdgCur)
                );
                iwdgImg.InitImgList(civUs, dsclToUse, colErrs, *this);
                c4Index++;

                // Remember if we have a focus indicator, for use during drawing
                if (iwdgCur.strWidgetId() == CQCIntfEng_MItemBrowser::strWID_Focus)
                    m_bHasFocusInd = kCIDLib::True;
            }
             else if (iwdgCur.clsIsA() == TCQCIntfStaticImg::clsThis())
            {
                // These we need to get to load their image
                TCQCIntfStaticImg& iwdgImg
                (
                    static_cast<TCQCIntfStaticImg&>(iwdgCur)
                );
                iwdgImg.InitImage(civUs, dsclToUse, colErrs, *this);
                c4Index++;
            }
             else if ((iwdgCur.clsIsA() == TCQCIntfStaticText::clsThis())
                  ||  (iwdgCur.clsIsA() == TCQCIntfMediaRepoText::clsThis()))
            {
                // Nothing special, just keep these
                c4Index++;
            }
             else
            {
                //
                //  Remove this one and reduce the count, and bump the index
                //  back down since we aren't moving forward.
                //
                m_iwdgLayout.RemoveWidget(c4Index);
                c4Count--;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Make sure it's emptied out
        m_iwdgLayout.ResetTemplate(&civOwner());
    }
}


//
//  There are a number of places where we need to get the content area,
//  scrolled up by the scroll offset, i.e. to move it so that it's
//  correctly relative to us for the current content to show through
//  our display area.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::MakeScrolledContArea(TArea& areaToFill) const
{
    areaToFill = m_areaContent;
    areaToFill.SetOrg(m_areaDisplay.pntOrg());
    areaToFill -= m_areaAdjDisplay.pntOrg();
}



//
//  This handles non-drag oriented sliding of the lists vertically, mostly
//  to support command driven list movement. It's sort of redundant with
//  the level slider below but too much different to want to bother with the
//  complexity of integrating them.
//
//  We have two basic scenarios, one where we are moving from one area to
//  another which are contiguous or overlap, and the other where we move
//  between two discontiguous areas. In the later case we just clear the
//  list content and scroll just the new content in from one side or the
//  other. We don't try to scroll current and new content because of the
//  issues of partial items being scrolled.
//
//  Note that the directions seem backwards here but it's because the
//  adjusted area is moved downwards over the content. But here we are
//  simulating the movement of the content in the other direction.
//
tCIDLib::TVoid
TCQCIntfAdvMItemBrowser::SlideList(const TArea& areaOrg)
{
    // Set up the whole area we need to scroll
    TArea areaScroll;

    // Figure out which scenario
    TArea areaFirst;
    TArea areaSec;

    tCIDLib::EDirs eDir;
    if (areaOrg.i4Y() > m_areaAdjDisplay.i4Y())
    {
        eDir = tCIDLib::EDirs::Down;
        areaFirst = m_areaAdjDisplay;
        areaSec = areaOrg;
    }
     else
    {
        eDir = tCIDLib::EDirs::Up;
        areaFirst = areaOrg;
        areaSec = m_areaAdjDisplay;
    }

    //
    //  If the two areas overlap (or just touch) we have a contiguous range,
    //  else we have to do two separate draws.
    //
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TBitmap bmpScroll;
    if (areaFirst.bIntersects(areaSec)
    ||  (areaFirst.i4Bottom() == areaSec.i4Y()))
    {
        //
        //  The scroll area is the intersected width, and the zero origined
        //  version of the combination is the scroll area.
        //
        TArea areaSrc = areaFirst;
        areaSrc |= areaSec;

        areaScroll = areaSrc;
        areaScroll.ZeroOrg();

        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);
        TRegionJanitor janClip(&gdevCont, areaScroll, tCIDGraphDev::EClipModes::And);

        // If a safe bgn, put it into the content buffer
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);
        }

        //
        //  And draw our contiguous range into the scroll buffer. We have
        //  to create a scrolled content area to use as the 'relative to'
        //  area. This will make the bit we care about come out in our
        //  zero origined buffer.
        //
        TArea areaRelTo = m_areaContent;
        areaRelTo -= areaSrc.pntOrg();
        DrawItems
        (
            gdevCont
            , areaRelTo
            , areaScroll
            , TPoint::pntOrigin
            , !m_bSafePanBgn
        );
    }
     else
    {
        // The scroll area is just twice the display area
        areaScroll.SetSize
        (
            m_areaDisplay.c4Width(), m_areaDisplay.c4Height() * 2
        );

        bmpScroll = TBitmap
        (
            areaScroll.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );

        // Copy the two src areas out to their appropriate target places
        TGraphMemDev gdevCont(civOwner().gdevCompat(), bmpScroll);

        TArea areaZDisp(m_areaDisplay);
        areaZDisp.ZeroOrg();


        //
        //  If a safe bgn, put it into the content buffer, but we also
        //  need to draw to the screen the stuff under us, since the
        //  efficient scroll will move the on-screen content, and that
        //  would probably scroll a partial item.
        //
        if (m_bSafePanBgn)
        {
            TArea areaFill(areaScroll);
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);

            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
            cptrGraphDev->CopyBits(gdevBgn, areaZDisp, m_areaDisplay);
        }

        TArea areaRelTo;
        if (eDir == tCIDLib::EDirs::Up)
        {
            // Put the second page in the first half and scroll opposite
            areaRelTo = m_areaContent;
            areaRelTo -= areaSec.pntOrg();

            DrawItems
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , TPoint::pntOrigin
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Down;
        }
         else
        {
            // Put the first page in the second half and scroll the opposite
            areaRelTo = m_areaContent;
            areaRelTo -= areaFirst.pntOrg();

            DrawItems
            (
                gdevCont
                , areaRelTo
                , areaZDisp
                , areaZDisp.pntLL(0, 1)
                , !m_bSafePanBgn
            );

            eDir = tCIDLib::EDirs::Up;
        }
    }

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

    // Get a device, clip, and do the scroll
    TRegionJanitor janClip
    (
        cptrGraphDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::Copy
    );

    TGraphMemDev gdevScroll(civOwner().gdevCompat(), bmpScroll);
    TCIDEaseScroller escrSlide;
    if (m_bSafePanBgn)
    {
        // Do the light weight slide
        escrSlide.Scroll2
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }
     else
    {
        //
        //  We have to do the heavier scroll, so set up a bitmap with our
        //  bgn fill in it, and provide a double buffer buffer.
        //
        TBitmap bmpBgn = bmpGatherBgn();
        TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

        // Do the double buffering bitmap
        TBitmap bmpBuf
        (
            m_areaDisplay.szArea()
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );
        TGraphMemDev gdevBuf(civOwner().gdevCompat(), bmpBuf);

        escrSlide.Scroll1
        (
            *cptrGraphDev
            , m_areaDisplay
            , areaScroll
            , gdevBuf
            , gdevBgn
            , gdevScroll
            , kCQCIntfEng_::c4EaseScrollMSs
            , eDir
            , kCQCIntfEng_::c4EaseOrder
        );
    }

    // Keep the focus index visible
    KeepFocusVisible();

    // And do a final redraw
    Redraw(m_areaDisplay);
}

