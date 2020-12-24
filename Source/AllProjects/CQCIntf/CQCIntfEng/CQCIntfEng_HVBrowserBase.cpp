//
// FILE NAME: CQCIntfEng_HVBrowserBase.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/13/2005
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
//  This file implements the horz/vert list browser base class that implements
//  the vast bulk of the function of a number of browser variations.
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


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfHVBrowserBase,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_HVBrowserBase
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage format version
        //
        //  Version 2 -
        //      The horz/vertical value became a persistent value now that it's
        //      just a setting and not something that's implied by the derived
        //      class (and therefore just set by them on ctor.)
        //
        //  Version 3 -
        //      There's no actual data format change, but we needed to correct
        //      inconsistent color usage, so we move some colors around on load.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion  = 3;


        // -----------------------------------------------------------------------
        //  The key under which we store the images in the image list interface
        // -----------------------------------------------------------------------
        const TString strFocus(L"Focus");
        const TString strPressed(L"Pressed");
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfHVBrwRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfHVBrwRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfHVBrwRTV::TCQCIntfHVBrwRTV( const   TString&        strItemText
                                    , const tCIDLib::TCard4 c4SelectIndex
                                    , const TString&        strTextKey
                                    , const TString&        strIndexKey
                                    , const TCQCUserCtx&    cuctxToUse) :
    TCQCCmdRTVSrc(cuctxToUse)
    , m_c4SelectIndex(c4SelectIndex)
    , m_strIndexKey(strIndexKey)
    , m_strItemText(strItemText)
    , m_strTextKey(strTextKey)
{
}

TCQCIntfHVBrwRTV::~TCQCIntfHVBrwRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrwRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfHVBrwRTV::bRTValue(const  TString&    strId
                                            ,       TString&    strToFill) const
{
    // Call our parent first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // If it's one of ours, then handle it
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (strId == m_strTextKey)
        strToFill = m_strItemText;
    else if (strId == m_strIndexKey)
        strToFill.SetFormatted(m_c4SelectIndex);
    else
        bRet = kCIDLib::False;
    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrwRTV: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TCQCIntfHVBrwRTV::c4SelIndex() const
{
    return m_c4SelectIndex;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfHVBrowserBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfHVBrowserBase::~TCQCIntfHVBrowserBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfHVBrowserBase& iwdgOther
    (
        static_cast<const TCQCIntfHVBrowserBase&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }

    return
    (
        (m_bHorizontal == iwdgOther.m_bHorizontal)
        && (m_bNumbered == iwdgOther.m_bNumbered)
        && (m_c4Size == iwdgOther.m_c4Size)
        && (m_c4Spacing == iwdgOther.m_c4Spacing)
        && (m_i4TextOfs == iwdgOther.m_i4TextOfs)
    );
}


tCIDLib::TBoolean TCQCIntfHVBrowserBase::bCanSizeTo() const
{
    // Our contents is driven by size, not the other way around
    return kCIDLib::False;
}


//
//  If moving up./eft and we are on the top.eft edge, or if moving down/right
//  and we are on the bottom/right edge, then return true to let the focus
//  move off us. Else, move internally and return false.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bMoveFocus(const tCQCKit::EScrNavOps eOp)
{
    // If the list is empty, just return true to move past us
    const tCIDLib::TCard4 c4Count = c4NameCount();
    if (!c4Count)
        return kCIDLib::True;

    tCIDLib::TBoolean bRet = kCIDLib::True;

    // We special case these, which we translate into scroll operations
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
            bScrollList(L"End", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::Home)
            bScrollList(L"Home", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::PrevPage)
            bScrollList(L"Prev Page", kCIDLib::True);
        else if (eOp == tCQCKit::EScrNavOps::NextPage)
            bScrollList(L"Next Page", kCIDLib::True);

        // Indicate we kept the focus internal
        bRet = kCIDLib::False;
    }
     else
    {
        // It's not one of those, so it's a focus movement op
        tCIDLib::TCard4 c4NewFocus = m_c4FocusIndex;
        switch(eOp)
        {
            case tCQCKit::EScrNavOps::Up :
            case tCQCKit::EScrNavOps::Left :
                if (c4NewFocus)
                    c4NewFocus--;
                break;

            case tCQCKit::EScrNavOps::Down :
            case tCQCKit::EScrNavOps::Right :
                if (c4NewFocus + 1 < c4Count)
                    c4NewFocus++;
                break;

            default :
                break;
        };

        //
        //  See if it's visible enough, if so keep it. If this returns
        //  true, then it didn't change the value, so we don't have to
        //  pass it a copy.
        //
        if ((c4NewFocus != m_c4FocusIndex) && bVisibleEnough(c4NewFocus, 0.4F))
        {
            TArea areaUpdate;
            AreaForIndex(m_c4FocusIndex, areaUpdate);
            TArea areaNew;
            AreaForIndex(c4NewFocus, areaNew);
            areaUpdate |= areaNew;

            m_c4FocusIndex = c4NewFocus;
            bRet = kCIDLib::False;

            Redraw(areaUpdate);
        }
    }
    return bRet;
}


//
//  We allow inertial panning in either axis, single finger. If not single finger, we
//  let it propogate. Else, even if ignoring it, we don't let it propogate.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bPerGestOpts(const   TPoint&             pntAt
                                    , const tCIDLib::EDirs      eDir
                                    , const tCIDLib::TBoolean   bTwoFingers
                                    ,       tCIDCtrls::EGestOpts& eToSet
                                    ,       tCIDLib::TFloat4&   f4VScale)
{
    if (bTwoFingers)
        return kCIDLib::True;

    if ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right))
    {
        if (m_areaContent.c4Width() > m_areaDisplay.c4Width())
        {
            if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;
        }
    }
     else
    {
        if (m_areaContent.c4Height() > m_areaDisplay.c4Height())
        {
            if (bNoInertia() || TFacCQCIntfEng::bRemoteMode())
                eToSet = tCIDCtrls::EGestOpts::Flick;
            else
                eToSet = tCIDCtrls::EGestOpts::PanInertia;
        }
    }

    //
    //  Reduce the velocity a bit for this one, since there's so little
    //  data, they will go overly far at the default level. If vertical
    //  we reduce it a bit more.
    //
    if (m_bHorizontal)
        f4VScale = 0.95F;
    else
        f4VScale = 0.80F;

    return kCIDLib::False;
}


//
//  THis one is pretty heavy. It handles gesture processing. Detailed
//  comments are below, but in general we do this:
//
//  1.  On start of gesture we figure out how much scrollable content we
//      are going to allow. The whole list may be too long. It will be
//      within the exiting m_areaContent area. For smaller lists it will
//      be the whole thing. For big ones, it'll be part of it.
//
//      We set up the bitmaps we require for the pan and draw the correct
//      list items and bgn content into them.
//
//  2.  During moves we see how far since the last time and move the pannable
//      content that much. We then blit the new visible bit of that content
//      to our display area, and update our display area member.
//
//  3.  At the end, we clean up the bitmaps and do a final redraw at the
//      position we ended up at.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bProcessGestEv(  const   tCIDCtrls::EGestEvs   eEv
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
        //  Decide now big our scrollable memory image is going to be, and set up
        //  the area where we track how much of our scrollable area we've drawn
        //  contents into yet. The latter is only really used if we have a pan
        //  safe background.
        //
        m_areaPanCont = m_areaContent;
        {
            TArea areaScreen = TGUIFacility::areaVirtualScreen();
            if (m_bHorizontal)
            {
                const tCIDLib::TInt4 i4Span(tCIDLib::TInt4(areaScreen.c4Width()));

                // Set the size to five spans
                m_areaPanCont.c4Width(i4Span * 5);

                // Put our display area in the middle span
                m_areaPanCont.i4X(m_areaAdjDisplay.i4X() - (i4Span * 2));

                // And clip to the content area
                if (m_areaPanCont.i4X() < m_areaContent.i4X())
                    m_areaPanCont.i4X(m_areaContent.i4X());
                if (m_areaPanCont.i4Right() > m_areaContent.i4Right())
                    m_areaPanCont.i4Right(m_areaContent.i4Right());

                //
                //  Set up the initial bit we'll draw to start. It's three pages
                //  of our content, and we'll set it up so that our display area
                //  is in the middle one. Note that it's not 3x the display width,
                //  since dragging will continue to the edge of the widget itself.
                //
                tCIDLib::TCard4 c4InitialWidth = tCIDLib::TCard4(areaUs.c4Width() * 3);

                m_areaPanSoFar = m_areaAdjDisplay;
                m_areaPanSoFar.c4Width(c4InitialWidth);
                m_areaPanSoFar.AdjustOrg(-tCIDLib::TInt4(areaUs.c4Width()), 0);

                // Clip to the pan content area if needed
                if (m_areaPanSoFar.i4X() < m_areaPanCont.i4X())
                    m_areaPanSoFar.i4X(m_areaPanCont.i4X());
                if (m_areaPanSoFar.i4Right() > m_areaPanCont.i4Right())
                    m_areaPanSoFar.i4Right(m_areaPanCont.i4Right());
            }
             else
            {
                const tCIDLib::TInt4 i4Span(tCIDLib::TInt4(areaScreen.c4Height()));

                // Set the size to five spans
                m_areaPanCont.c4Height(i4Span * 5);

                // Put our display area in the middle span
                m_areaPanCont.i4Y(m_areaAdjDisplay.i4Y() - (i4Span * 2));

                // And clip to the content area
                if (m_areaPanCont.i4Y() < m_areaContent.i4Y())
                    m_areaPanCont.i4Y(m_areaContent.i4Y());
                if (m_areaPanCont.i4Bottom() > m_areaContent.i4Bottom())
                    m_areaPanCont.i4Bottom(m_areaContent.i4Bottom());

                //
                //  Set up the initial bit we'll draw to start. It's three pages
                //  of our content, and we'll set it up so that our adisplay area
                //  is in the middle one.
                //
                tCIDLib::TCard4 c4InitialHeight = tCIDLib::TCard4(areaUs.c4Height() * 3);

                m_areaPanSoFar = m_areaAdjDisplay;
                m_areaPanSoFar.c4Height(c4InitialHeight);
                m_areaPanSoFar.AdjustOrg(0, -tCIDLib::TInt4(areaUs.c4Height()));

                // Clip to the pan content area if needed
                if (m_areaPanSoFar.i4Y() < m_areaPanCont.i4Y())
                    m_areaPanSoFar.i4Y(m_areaPanCont.i4Y());
                if (m_areaPanSoFar.i4Bottom() > m_areaPanCont.i4Bottom())
                    m_areaPanSoFar.i4Bottom(m_areaPanCont.i4Bottom());
            }
        }

        // Create the scrolling content bitmap for the pannable content
        m_bmpCont = TBitmap
        (
            m_areaPanCont.szArea()
            , tCIDImage::EPixFmts::TrueAlpha
            , tCIDImage::EBitDepths::Eight
        );

        // Get an initial area to draw.
        TArea areaInitial = m_areaPanSoFar;
        areaInitial -= m_areaPanCont.pntOrg();

        TGraphMemDev gdevCont(civOwner().gdevCompat(), m_bmpCont);
        if (m_bSafePanBgn)
        {
            //
            //  We have a safe bgn so we can put our background into the
            //  contents itself. We have to insure that any gradients line
            //  up. So we need to set up a content area that is our own size
            //  in the opposite direction of travel and center it on our
            //  zerod content area in that transverse axis. Then we can fill
            //  that and insure that the fill within our display/content area
            //  sized slice will line up with any gradient in the overall
            //  widget area.
            //
            TArea areaFill(areaInitial);
            if (m_bHorizontal)
            {
                areaFill.c4Height(areaActual().c4Height());
                areaFill.JustifyIn
                (
                    areaInitial
                    , tCIDLib::EHJustify::Left
                    , tCIDLib::EVJustify::Center
                );
            }
             else
            {
                areaFill.c4Width(areaActual().c4Width());
                areaFill.JustifyIn
                (
                    areaInitial
                    , tCIDLib::EHJustify::Center
                    , tCIDLib::EVJustify::Top
                );
            }

            DoBgnFill(gdevCont, areaFill);
        }
         else
        {
            //
            //  We need a separate background image to composite the items
            //  over on each round. It only needs to be our size.
            //
            m_bmpBgn = bmpGatherBgn();

            //
            //  We need one for double buffering during the pan in this case, the
            //  size of the display area.
            //
            m_bmpDBuf = TBitmap
            (
                m_areaDisplay.szArea()
                , tCIDImage::EPixFmts::TrueClr
                , tCIDImage::EBitDepths::Eight
                , kCIDLib::False
            );
        }

        //  Draw the initial items now.
        TArea areaRelTo(m_areaContent);
        areaRelTo -= m_areaPanCont.pntOrg();

        DrawItems(gdevCont, areaRelTo, areaInitial, TPoint::pntOrigin, !m_bSafePanBgn);
    }
     else if ((eEv == tCIDCtrls::EGestEvs::Drag)
          ||  (eEv == tCIDCtrls::EGestEvs::Inertia)
          ||  (eEv == tCIDCtrls::EGestEvs::End))
    {
        //
        //  Remember the current adjusted display position, move the
        //  adjusted display and then clip back as needed. Then remember
        //  what the actual delta (if any) was.
        //
        tCIDLib::TInt4 i4NewDelta;
        tCIDLib::TInt4 i4OrgPos;
        if (m_bHorizontal)
        {
            i4OrgPos = m_areaAdjDisplay.i4X();

            //
            //  Apply the delta and clip it to the pan content area. We
            //  have to flip the delta since we are moving the display area
            //  forward, not the content area back.
            //
            m_areaAdjDisplay.AdjustOrg(-pntDelta.i4X(), 0);

            if (m_areaAdjDisplay.i4X() < m_areaPanCont.i4X())
                m_areaAdjDisplay.i4X(m_areaPanCont.i4X());
            if (m_areaAdjDisplay.i4Right() > m_areaPanCont.i4Right())
                m_areaAdjDisplay.RightJustifyIn(m_areaPanCont);

            i4NewDelta = m_areaAdjDisplay.i4X() - i4OrgPos;
        }
         else
        {
            i4OrgPos = m_areaAdjDisplay.i4Y();

            //
            //  Apply the delta and clip it to the pan content area. We
            //  have to flip the delta since we are moving the display area
            //  forward, not the content area back.
            //
            m_areaAdjDisplay.AdjustOrg(0, -pntDelta.i4Y());

            // Clip it to the pan content area
            if (m_areaAdjDisplay.i4Y() < m_areaPanCont.i4Y())
                m_areaAdjDisplay.i4Y(m_areaPanCont.i4Y());
            if (m_areaAdjDisplay.i4Bottom() > m_areaPanCont.i4Bottom())
                m_areaAdjDisplay.BottomJustifyIn(m_areaPanCont);

            i4NewDelta = m_areaAdjDisplay.i4Y() - i4OrgPos;
        }

        //
        //  If we actually moved, then do the movement. If not, and we
        //  are getting inertia at this point, cancel any more inertia.
        //
        if (i4NewDelta)
        {
            tCQCIntfEng::TGraphIntfDev cptrDev = cptrNewGraphDev();

            //
            //  Clip to within our display plus the most restrictive of
            //  any of our ancestor containers.
            //
            TGUIRegion grgnClip;
            bFindMostRestrictiveClip(grgnClip, m_areaDisplay);
            TRegionJanitor janClip
            (
                cptrDev.pobjData(), grgnClip, tCIDGraphDev::EClipModes::And
            );

            //
            //  We now need to adjust the adjusted display area, because
            //  it is currently relative to the overall content origin,
            //  and we need ot make it relative to the pan content. Now
            //  it's over the actual pan content at the appropriate place
            //  to blit content from.
            //
            TArea areaBlitSrc = m_areaAdjDisplay;
            areaBlitSrc -= m_areaPanCont.pntOrg();

            if (m_bSafePanBgn)
            {
                //
                //  In this case we can just blit the contents directly since bgn
                //  and content are together. We do an efficient scheme where we
                //  move the displayed contents and only blit the uncovered holes.
                //
                TGraphMemDev gdevSrc(civOwner().gdevCompat(), m_bmpCont);

                //
                //  Scroo the window contents of the display area and then blit
                //  in the missing bit.
                //
                TArea areaTmpSrc;
                TArea areaTmpTar;

                if (m_bHorizontal)
                {
                    cptrDev->ScrollBits
                    (
                        m_areaDisplay
                        , m_areaDisplay
                        , i4NewDelta * -1
                        , 0
                        , areaTmpTar
                        , kCIDLib::True
                    );

                    areaTmpSrc = areaTmpTar;
                    if (i4NewDelta < 0)
                        areaTmpSrc.LeftJustifyIn(areaBlitSrc, kCIDLib::True);
                     else
                        areaTmpSrc.RightJustifyIn(areaBlitSrc, kCIDLib::True);
                }
                 else
                {
                    cptrDev->ScrollBits
                    (
                        m_areaDisplay
                        , m_areaDisplay
                        , 0
                        , i4NewDelta * -1
                        , areaTmpTar
                        , kCIDLib::True
                    );

                    areaTmpSrc = areaTmpTar;
                    if (i4NewDelta < 0)
                        areaTmpSrc.TopJustifyIn(areaBlitSrc);
                     else
                        areaTmpSrc.BottomJustifyIn(areaBlitSrc);
                    areaTmpSrc.i4X(areaBlitSrc.i4X());
                }

                // And blit the uncovered area, waiting for the retrace
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
                //  We have to do the composite scenario. Set up a zero based
                //  version of our own area. We need this to blit from the bgn to
                //  our double buffer, draw the new display area content over that
                //  bgn, then copy the result to the display area.
                //
                TArea areaZDisp = m_areaDisplay;
                areaZDisp.ZeroOrg();

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

                // Copy to the screen, waiting for the retrace
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
//  Called when the mouse is released at the end of a tracking scenario.
//  If released within the the item we started on, then we invoke that
//  item.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::Clicked(const TPoint& pntAt)
{
    // See if we hit an item
    const tCIDLib::TCard4 c4InvokeInd = c4HitTest(pntAt);

    // If not, we are done
    if (c4InvokeInd == kCIDLib::c4MaxCard)
        return;

    //
    //  Make sure that this one is at least 40% visible. If not, we'll
    //  ignore it.
    //
    tCIDLib::TCard4 c4Dummy = c4InvokeInd;
    if (!bVisibleEnough(c4Dummy, 0.4F))
        return;

    TArea areaUpdate;
    if (m_c4FocusIndex != c4InvokeInd)
    {
        AreaForIndex(m_c4FocusIndex, areaUpdate);
        m_c4FocusIndex = c4InvokeInd;
        Redraw(areaUpdate);
    }

    // Draw the target pressed, pause, and then unpressed
    {
        AreaForIndex(c4InvokeInd, areaUpdate);
        {
            TCardJanitor janPressInd(&m_c4PressIndex, c4InvokeInd);
            Redraw(areaUpdate);
        }
        TThread::Sleep(kCQCIntfEng_::c4PressEmphMSs);
        Redraw(areaUpdate);
    }

    //
    //  Temporarily set the selection index so that the RTV object
    //  will pick it up.
    //
    TCardJanitor janTIndex(&m_c4SelIndex, c4InvokeInd);
    PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::NonNested);

    // Don't do anything else unless you check it's not dead
}



tCQCKit::ECmdRes
TCQCIntfHVBrowserBase::eDoCmd(  const   TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollList)
    {
        bResult = bScrollList
        (
            ccfgToDo.piAt(0).m_strValue,  bCanRedraw(strEventId)
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_ScrollToIndex)
    {
        bResult = bScrollToIndex
        (
            ccfgToDo.piAt(0).m_strValue.c4Val(), bCanRedraw(strEventId)
        );
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetCount)
    {
        const tCIDLib::TCard4 c4Count(c4NameCount());

        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(c4Count) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
        bResult = (c4Count != 0);
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetIndex)
    {
        // Return true if we have some items
        bResult = (c4NameCount() != 0);
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(c4FirstVisIndex()) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetText)
    {
        // The index is the first parm
        const tCIDLib::TCard4 c4PIndex = ccfgToDo.piAt(0).c4Val(tCIDLib::ERadices::Auto);

        // If the index is good, get the text and return true, else false
        if (c4PIndex < c4NameCount())
        {
            const TString& strKey = ccfgToDo.piAt(1).m_strValue;
            TCQCActVar& varText = TStdVarsTar::varFind
            (
                strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
            );
            if (varText.bSetValue(strDisplayTextAt(c4PIndex)) && acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
            bResult = kCIDLib::True;
        }
         else
        {
            bResult = kCIDLib::False;
            if (acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActDebug(L"Bad vert browser index, no text returned");
        }
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


//
//  If we are not in pan safe bgn mode, then this is not used currently. If we are
//  pan safe, then this is used to draw ahead into our pan content buffer. We see
//  how much more drawn content we have left in the direction of travel. If it's
//  less than a certain amount, we draw some more in that direction and adjust the
//  'so far' area out to reflect that.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::GestInertiaIdle( const   tCIDLib::TEncodedTime   enctEnd
                                        , const tCIDLib::TCard4         c4Millis
                                        , const tCIDLib::EDirs          eDir)
{
    TArea                   areaNewSoFar(m_areaPanSoFar);
    TArea                   areaUpdate;
    const tCIDLib::TCard4   c4SlotSz = m_c4Size + m_c4Spacing;

    if (m_bHorizontal)
    {
        const tCIDLib::TInt4 i4Span(areaActual().c4Width());
        const tCIDLib::TInt4 i42Span(i4Span * 2);

        if (eDir == tCIDLib::EDirs::Left)
        {
            if (areaNewSoFar.i4Right() >= m_areaPanCont.i4Right())
                return;

            if (areaNewSoFar.i4Right() - m_areaAdjDisplay.i4Right() > i42Span)
                return;

            areaNewSoFar.AdjustSize(i4Span, 0);

            // If it goes too far. clip it back
            if (areaNewSoFar.i4Right() > m_areaPanCont.i4Right())
                areaNewSoFar.i4Right(m_areaPanCont.i4Right());

            // The update area is now the new so far minus the old one
            areaUpdate = areaNewSoFar;
            areaUpdate.i4X(m_areaPanSoFar.i4Right(), kCIDLib::True);
        }
         else if (eDir == tCIDLib::EDirs::Right)
        {
            if (!areaNewSoFar.i4X())
                return;

            if (m_areaAdjDisplay.i4X() - areaNewSoFar.i4X() > i42Span)
                return;

            // Keep the right where it is, move the left down by a span
            areaNewSoFar.i4X(areaNewSoFar.i4Left() - i4Span, kCIDLib::True);

            // If it goes below the origin, clip it back
            if (areaNewSoFar.i4X() < 0)
                areaNewSoFar.i4X(0, kCIDLib::True);

            // The update area is now the new so far minus the old one
            areaUpdate = areaNewSoFar;
            areaUpdate.i4Right(m_areaPanSoFar.i4X() - 1);
        }
    }
     else
    {
        const tCIDLib::TInt4 i4Span(areaActual().c4Height());
        const tCIDLib::TInt4 i42Span(i4Span * 2);

        if (eDir == tCIDLib::EDirs::Up)
        {
            if (areaNewSoFar.i4Bottom() >= m_areaPanCont.i4Bottom())
                return;

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
            areaNewSoFar.i4Y(areaNewSoFar.i4Y() - i4Span, kCIDLib::True);

            // If it goes below the origin, clip it back, but keep the bottom fixed
            if (areaNewSoFar.i4Y() < 0)
                areaNewSoFar.i4Y(0, kCIDLib::True);

            // The update area is now the new so far minus the old one
            areaUpdate = areaNewSoFar;
            areaUpdate.i4Bottom(m_areaPanSoFar.i4Y() - 1);
        }
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
    //  we need to do a background fill (making sure gradients align.)
    //
    const TGraphDrawDev& gdevCompat = civOwner().gdevCompat();
    TGraphMemDev gdevCont(gdevCompat, m_bmpCont);

    //
    //  We have to clip the output to the actual update area. Otherwise, we could
    //  allow content to be drawn over twice. If that content was drawn with alpha,
    //  it would just add up.
    //
    TRegionJanitor janClip(&gdevCont, areaUpdate, tCIDGraphDev::EClipModes::And);

    // Handle bgn fill
    if (m_bSafePanBgn)
    {
        TArea areaFill(areaUpdate);
        if (m_bHorizontal)
        {
            areaFill.c4Height(areaActual().c4Height());
            areaFill.CenterIn(areaUpdate);
        }
         else
        {
            areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaUpdate);
        }
        DoBgnFill(gdevCont, areaFill);
    }
    DrawItems(gdevCont, areaRelTo, areaUpdate, TPoint::pntOrigin, !m_bSafePanBgn);
}


//
//  We just call our parent, and then calculate where our slots are going to
//  go, based on the loaded width/spacing values.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    // Do an initial calculation of area info. Pass a bogus old area so it'll update
    CalcAreas(areaActual(), TArea::areaEmpty);

    //
    //  Remember if we have a safe bgn for scrolling. If so, we only need
    //  to create one scrollable bitmap, which will have the bgn in it. Else,
    //  we need the content plus the bgn, which we will composite together
    //  at each new position.
    //
    m_bSafePanBgn = bScrollSafeBgn(!m_bHorizontal);
}


//
//  We get alled here on an enter key or an enter command from the control
//  interface. If we have the focus, then we invoke any OnSelect commands
//  with the current focus item as the selected item.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::Invoke()
{
    //
    //  If we have the focus then we want to invoke commands for the select
    //  event. Set the select index long enough for the RTV object to get
    //  it.
    //
    //  Make sure that the focus slot is a valid item!
    //
    if (bHasFocus() && (m_c4FocusIndex < c4NameCount()))
    {
        TCardJanitor janOver(&m_c4SelIndex, m_c4FocusIndex);
        PostOps(*this, kCQCKit::strEvId_OnSelect, tCQCKit::EActOrders::NonNested);

        //
        //  Don't do anything else unless you check that we weren't
        //  killed by this action.
        //
    }
}


// We still have to handle flicks for RIVA 1 clients
tCIDLib::TVoid
TCQCIntfHVBrowserBase::ProcessFlick(const   tCIDLib::EDirs  eDir
                                    , const TPoint&         pntStart)
{
    //
    //  See if it's an edge or center flick, and map to the appropriate
    //  scrolling command. We have to deal in this case with the fact
    //  that we can be either vertical or horizontal as well.
    //
    const tCIDLib::TCh* pszCmd = 0;
    if (eDir == tCIDLib::EDirs::Right)
        pszCmd = L"Prev";
    else if (eDir == tCIDLib::EDirs::Left)
        pszCmd = L"Next";
    else if (eDir == tCIDLib::EDirs::Down)
        pszCmd = L"Prev";
    else if (eDir == tCIDLib::EDirs::Up)
        pszCmd = L"Next";

    // If we got a command, do it, and redraw if we change anything
    if (pszCmd)
        bScrollList(pszCmd, kCIDLib::True);

    //
    //  Don't do anything else unless you check that we were not killed
    //  by actions invoked above.
    //
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Get the count of items in the list. Returns true if non-zero, else false
    {
        TCQCCmd& cmdGetCount = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetCount
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetCount)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdGetCount.eType(tCQCKit::ECmdTypes::Both);
    }

    // Get the index of the top item in the list. Return true if not empty
    {
        TCQCCmd& cmdGetIndex = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetIndex
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetIndex)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdGetIndex.eType(tCQCKit::ECmdTypes::Both);
    }

    // Get the text of given slot
    {
        TCQCCmd& cmdGetText = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetText
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetText)
                , tCQCKit::ECmdPTypes::Unsigned
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
            )
        );
        cmdGetText.eType(tCQCKit::ECmdTypes::Both);
    }

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
                    , m_bHorizontal ? L"End, Home, Left, Right, Next Page, Prev Page"
                                    : L"Bottom, Down, Top, Up, Prev, Next"
                )
            );
            cmdScroll.eType(tCQCKit::ECmdTypes::Both);
        }

        // Scroll to a specific index at the top of the list, if we can
        {
            TCQCCmd& cmdScroll = colCmds.objAdd
            (
                TCQCCmd
                (
                    kCQCKit::strCmdId_ScrollToIndex
                    , facCQCKit().strMsg(kKitMsgs::midCmd_ScrollToIndex)
                    , tCQCKit::ECmdPTypes::Unsigned
                    , facCQCKit().strMsg(kKitMsgs::midCmdP_Index)
                )
            );
            cmdScroll.eType(tCQCKit::ECmdTypes::Both);
        }
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Focus Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Focus Shadow/FX");

    // Do our mixins. Our actions are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Browser Actions"
    );
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    // And add our stuff
    adatTmp.Set(L"Browser Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Horizontal"
        , kCQCIntfEng::strAttr_HVBrowser_Horz
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bHorizontal);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Numbered"
        , kCQCIntfEng::strAttr_HVBrowser_Numbered
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bNumbered);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Size"
        , kCQCIntfEng::strAttr_HVBrowser_Size
        , L"Range: 5, 256"
        , tCIDMData::EAttrTypes::Card
    );
    adatTmp.SetCard(m_c4Size);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Spacing"
        , kCQCIntfEng::strAttr_HVBrowser_Spacing
        , L"Range: 0, 100"
        , tCIDMData::EAttrTypes::Card
    );
    adatTmp.SetCard(m_c4Spacing);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Text Offset"
        , kCQCIntfEng::strAttr_HVBrowser_TextOfs
        , L"Range: -256, 256"
        , tCIDMData::EAttrTypes::Int
    );
    adatTmp.SetInt(m_i4TextOfs);
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfHVBrowserBase::szDefaultSize() const
{
    if (m_bHorizontal)
        return TSize(160, 38);
    return TSize(120, 160);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfHVBrowserBase::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_HVBrowser_Horz)
        m_bHorizontal = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_HVBrowser_Numbered)
        m_bNumbered = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_HVBrowser_Size)
        m_c4Size = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_HVBrowser_Spacing)
        m_c4Spacing = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_HVBrowser_TextOfs)
        m_i4TextOfs = adatNew.i4Val();
}


//
//  We override this in order to scale our slot size so that it correctly fits the
//  adjusted font size after rescaling. We scale by the factor associated with our
//  orientation.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::ScaleInternal(const  tCIDLib::TFloat8 f8XScale
                                    , const tCIDLib::TFloat8 f8YScale)
{
    if (m_bHorizontal)
        m_c4Size = TMathLib::c4Round(m_c4Size * f8XScale);
    else
        m_c4Size = TMathLib::c4Round(m_c4Size * f8YScale);
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                        , TDataSrvClient&       dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the horz/vertical orientation attribute
tCIDLib::TBoolean TCQCIntfHVBrowserBase::bHorizontal() const
{
    return m_bHorizontal;
}

tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bHorizontal(const tCIDLib::TBoolean bToSet)
{
    m_bHorizontal = bToSet;

    // We have to recalculate our content area. Pass a bogus old area so it'll update
    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_bHorizontal;
}


// Get/set the numbered list attribute
tCIDLib::TBoolean TCQCIntfHVBrowserBase::bNumbered() const
{
    return m_bNumbered;
}

tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bNumbered(const tCIDLib::TBoolean bToSet)
{
    m_bNumbered = bToSet;
    return m_bNumbered;
}


// Get/set the item size (height or width, based on browser type)
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4Size() const
{
    return m_c4Size;
}

tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4Size(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Size = c4ToSet;

    // We have to recalculate our content area. Pass a bogus old area so it'll update
    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_c4Size;
}


// Get/set the item spacing
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4Spacing() const
{
    return m_c4Spacing;
}

tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4Spacing(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Spacing = c4ToSet;

    // We have to recalculate our content area. Pass a bogus old area so it'll update
    CalcAreas(areaActual(), TArea::areaEmpty);
    return m_c4Spacing;
}


// Get/set the text offset
tCIDLib::TInt4 TCQCIntfHVBrowserBase::i4TextOfs() const
{
    return m_i4TextOfs;
}

tCIDLib::TInt4 TCQCIntfHVBrowserBase::i4TextOfs(const tCIDLib::TInt4 i4ToSet)
{
    m_i4TextOfs = i4ToSet;
    return m_i4TextOfs;
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Hidden Constructor and Operators
// ---------------------------------------------------------------------------

TCQCIntfHVBrowserBase::
TCQCIntfHVBrowserBase(  const   tCQCIntfEng::ECapFlags  eCapFlags
                        , const TString&                strTypeName) :

    // Force gestures and scrollability on for all derivatives
    TCQCIntfWidget
      (
        tCIDLib::eOREnumBits(eCapFlags, tCQCIntfEng::ECapFlags::DoesGestures)
        , strTypeName
        , kCQCIntfEng::c4StyleFlag_HasBorder
      )
    , MCQCIntfFontIntf()
    , MCQCCmdSrcIntf()
    , MCQCIntfImgListIntf()
    , m_bHorizontal(kCIDLib::False)
    , m_bNumbered(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4PrefWidth(0)
    , m_c4PressIndex(kCIDLib::c4MaxCard)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_c4Size(12)
    , m_c4Spacing(16)
    , m_i4TextOfs(0)
{
    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );

    //
    //  In this one, they don't have specific RTV names/keys they want used
    //  for the index and selected text RTVs, so we use the defaults.
    //
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_SelectedIndex)
        , kCQCKit::strRTVId_SelectIndex
        , tCQCKit::ECmdPTypes::Unsigned
    );
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_SelectedText)
        , kCQCIntfEng::strRTVId_ListItemText
        , tCQCKit::ECmdPTypes::Text
    );

    // Load up the pressed, focus keys into the image list mixin
    AddNewImgKey(CQCIntfEng_HVBrowserBase::strFocus, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_HVBrowserBase::strPressed, tCQCIntfEng::EImgReqFlags::Optional);

    //
    //  Set out default appearance. We set the border style flag in the ctor above because
    //  if we did it here it would attempt to report the style change before we are ready
    //  to deal with it.
    //
    bNoTextWrap(kCIDLib::True);
    m_c4Size = 16;
    m_c4Spacing = 4;
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn3(kCQCIntfEng_::rgbDef_Border);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/ListBrowser");
}

TCQCIntfHVBrowserBase::
TCQCIntfHVBrowserBase(  const   tCQCIntfEng::ECapFlags  eCapFlags
                        , const TString&                strTextName
                        , const TString&                strTextKey
                        , const TString&                strIndexName
                        , const TString&                strIndexKey
                        , const TString&                strTypeName) :

    // Force gestures on for all derivatives
    TCQCIntfWidget
    (
        tCIDLib::eOREnumBits(eCapFlags, tCQCIntfEng::ECapFlags::DoesGestures)
        , strTypeName
        , kCQCIntfEng::c4StyleFlag_HasBorder
    )
    , MCQCIntfFontIntf()
    , MCQCCmdSrcIntf()
    , MCQCIntfImgListIntf()
    , m_bHorizontal(kCIDLib::False)
    , m_bNumbered(kCIDLib::False)
    , m_c4FocusIndex(0)
    , m_c4PrefWidth(0)
    , m_c4PressIndex(kCIDLib::c4MaxCard)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_c4Size(12)
    , m_c4Spacing(16)
    , m_i4TextOfs(0)
{
    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSelect)
        , kCQCKit::strEvId_OnSelect
        , tCQCKit::EActCmdCtx::Standard
    );

    //
    //  In this ctor, they tell us what they want us to report the text and
    //  index RTVs as, since the meaning can be different based on the type
    //  of derived class.
    //
    AddRTValue(strIndexName, strIndexKey, tCQCKit::ECmdPTypes::Unsigned);
    AddRTValue(strTextName, strTextKey, tCQCKit::ECmdPTypes::Text);

    // Load up the pressed, focus keys into the image list mixin
    AddNewImgKey(CQCIntfEng_HVBrowserBase::strFocus, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_HVBrowserBase::strPressed, tCQCIntfEng::EImgReqFlags::Optional);

    //
    //  Set out default appearance. We set the border style flag in the ctor above because
    //  if we did it here it would attempt to report the style change before we are ready
    //  to deal with it.
    //
    bNoTextWrap(kCIDLib::True);
    m_c4Size = 16;
    m_c4Spacing = 4;
    rgbBgn(kCQCIntfEng_::rgbDef_Bgn);
    rgbFgn3(kCQCIntfEng_::rgbDef_Border);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/ListBrowser");
}

// Notice that some runtime only stuff is not copied over!
TCQCIntfHVBrowserBase::TCQCIntfHVBrowserBase(const TCQCIntfHVBrowserBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_areaAdjDisplay(iwdgSrc.m_areaAdjDisplay)
    , m_areaContent(iwdgSrc.m_areaContent)
    , m_areaDisplay(iwdgSrc.m_areaDisplay)
    , m_bHorizontal(iwdgSrc.m_bHorizontal)
    , m_bNumbered(iwdgSrc.m_bNumbered)
    , m_c4FocusIndex(0)
    , m_c4PrefWidth(iwdgSrc.m_c4PrefWidth)
    , m_c4PressIndex(kCIDLib::c4MaxCard)
    , m_c4SelIndex(kCIDLib::c4MaxCard)
    , m_c4Size(iwdgSrc.m_c4Size)
    , m_c4Spacing(iwdgSrc.m_c4Spacing)
    , m_i4TextOfs(iwdgSrc.m_i4TextOfs)
{
}

TCQCIntfHVBrowserBase&
TCQCIntfHVBrowserBase::operator=(const TCQCIntfHVBrowserBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);

        m_areaAdjDisplay    = iwdgSrc.m_areaAdjDisplay;
        m_areaContent       = iwdgSrc.m_areaContent;
        m_areaDisplay       = iwdgSrc.m_areaDisplay;

        m_bHorizontal       = iwdgSrc.m_bHorizontal;
        m_bNumbered         = iwdgSrc.m_bNumbered;
        m_c4PrefWidth       = iwdgSrc.m_c4PrefWidth;
        m_c4Size            = iwdgSrc.m_c4Size;
        m_c4Spacing         = iwdgSrc.m_c4Spacing;
        m_i4TextOfs         = iwdgSrc.m_i4TextOfs;

        // These are meaningless until set at viewing time
        m_c4FocusIndex      = 0;
        m_c4PressIndex      = kCIDLib::c4MaxCard;
        m_c4SelIndex        = kCIDLib::c4MaxCard;

        // We don't bother copying the scrolling bitmaps here
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfHVBrowserBase::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    // Call our area calculating method to update
    CalcAreas(areaNew, areaOld);
}


//
//  If we are getting focus, then we show the emphasis from theslot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we have the focus.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::GotFocus()
{
    // Just force a redraw of the slot with the focus
    RedrawItem(m_c4FocusIndex);
}


//
//  If we are losing the focus, we remove the emphasis from the slot that
//  currently has the focus. All we have to do is redraw the focus slot,
//  now that we don't have the focus.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::LostFocus()
{
    // Just force a redraw of the slot with the focus
    RedrawItem(m_c4FocusIndex);
}


//
//  This can now happen both at design and viewing time since we now support
//  dynamic resizing.
//
//  If we get smaller, that's not an issue, but if we get larger, and we
//  were already all the way to the right, or suddenly we got bigger than
//  the list, we have to adjust.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);

    // Recalculate our areas
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    CalcAreas(areaNew, areaOld);
}


tCIDLib::TVoid TCQCIntfHVBrowserBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_HVBrowserBase::c2FmtVersion))
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

    // Let any mixins stream in
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // Stream our stuff
    strmToReadFrom  >> m_c4Spacing
                    >> m_c4Size
                    >> m_bNumbered
                    >> m_i4TextOfs;

    //
    //  For a while the size was allowed to go to zero which was bad and
    //  made no sense. Now it's limited to 5 minimum, so adjust if it's
    //  less.
    //
    if (m_c4Size < 5)
        m_c4Size = 5;

    //
    //  If V1, default the horizontal value, else read it in. Before V2
    //  this wasn't persisted and was just part of the derived class which
    //  would set it on us during ctor. So if we are still V1, it's and old
    //  version and it will be set by a derived class during the conversion
    //  to the latest format.
    //
    if (c2FmtVersion < 2)
        m_bHorizontal = kCIDLib::False;
    else
        strmToReadFrom >> m_bHorizontal;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If V1/2, then we need to adjust some colors, so that the F2 color is
    //  always used for shadow/fx. Currently F3 is being used for shadow and
    //  F2 for FX, and F2 is also being used for the border.
    //
    if (c2FmtVersion < 3)
    {
        // Save the original values we are going to change
        TRGBClr rgbF2 = rgbFgn2();
        TRGBClr rgbF3 = rgbFgn3();

        // Put F2 into F3, which is now the border color
        rgbFgn3(rgbF2);

        //
        //  If in not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }

    // Reset any runtime stuff
    m_c4FocusIndex  = 0;
    m_c4PressIndex  = kCIDLib::c4MaxCard;
    m_c4SelIndex    = kCIDLib::c4MaxCard;
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our start marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_HVBrowserBase::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // And do our stuff and our end marker
    strmToWriteTo   << m_c4Spacing
                    << m_c4Size
                    << m_bNumbered
                    << m_i4TextOfs
                    << m_bHorizontal
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::StyleFlagChanged(const   tCIDLib::TCard4 c4OldFlags
                                        , const tCIDLib::TCard4 c4NewFlags)
{
    const tCIDLib::TCard4 c4Old(c4OldFlags & kCQCIntfEng::c4StyleFlag_HasBorder);
    const tCIDLib::TCard4 c4New(c4NewFlags & kCQCIntfEng::c4StyleFlag_HasBorder);

    // If the border flag changed, recalc our areas
    if (c4Old != c4New)
        CalcAreas(areaActual(), TArea::areaEmpty);
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::Update(          TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Add our display area to the clip region
    grgnClip.CombineWith(m_areaDisplay, tCIDGraphDev::EClipModes::And);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    //
    //  And draw the invalidated items. Create a relative content area to
    //  pass in, and pass our display area as the target. So it will draw
    //  the items of the content area that fall under our area.
    //
    TArea areaRelTo;
    MakeScrolledContArea(areaRelTo);

    // The update area is the intersection of our display and the invalid
    TArea areaUpdate(areaInvalid);
    areaUpdate &= m_areaDisplay;

    DrawItems(gdevTarget , areaRelTo, areaUpdate, TPoint::pntOrigin, kCIDLib::False);
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Lets the drived class get the index of the item with the focus. Using
//  this and bHasFocus() from the base class, he can draw the focus
//  appropriately for however he indicates focus.
//
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4FocusIndex() const
{
    return m_c4FocusIndex;
}


//
//  Let's the derived class get the index of the item that is selected. This
//  means we are tracking it and the mouse is inside the button. Else, we
//  return c4MaxCard to indicate no selection.
//
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4SelectedIndex() const
{
    return m_c4SelIndex;
}


// Force the item at the indicated index to redraw, if it's visible
tCIDLib::TVoid
TCQCIntfHVBrowserBase::RedrawItem(const tCIDLib::TCard4 c4Index)
{
    // Get the area for this item, if valid
    if (c4Index < c4NameCount())
    {
        TArea areaUpdate;
        AreaForIndex(c4Index, areaUpdate);

        // If it intersects our display area, redraw it
        if (areaUpdate.bIntersects(m_areaDisplay))
            Redraw(areaUpdate);
    }
}


//
//  Lets the derived class tell us that he's reloaded his name list, so we
//  have to update any of our stuff that would be affected by that.
//
//  We recalculate our content area and reset our scroll offset back to
//  zero.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::ReloadedList()
{
    // Make sure we keep any focus item within the new max index
    const tCIDLib::TCard4 c4MaxIndex = c4NameCount();
    if (c4MaxIndex)
    {
        if (m_c4FocusIndex >= c4MaxIndex)
            m_c4FocusIndex = c4MaxIndex - 1;
    }
     else
    {
        m_c4FocusIndex = 0;
    }

    // If numbered, force a re-calc of the numeric prefix area
    if (m_bNumbered)
        m_c4PrefWidth = 0;

    // Update the content area
    CalcAreas(areaActual(), TArea::areaEmpty);
}


// ---------------------------------------------------------------------------
//  TCQCIntfHVBrowserBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Adjust the passed area by the item slot size. Which can include spacing
//  or not. Once we figure out a starting area, this can be called to just
//  adjust it for each item.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::AdjustBySlot(        TArea&              areaAdjust
                                    , const tCIDLib::TBoolean   bInclSpace) const
{
    tCIDLib::TInt4 i4Ofs = tCIDLib::TInt4(m_c4Size);
    if (bInclSpace)
        i4Ofs += tCIDLib::TInt4(m_c4Spacing);

    if (m_bHorizontal)
        areaAdjust.AdjustOrg(i4Ofs, 0);
    else
        areaAdjust.AdjustOrg(0, i4Ofs);
}


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
tCIDLib::TVoid
TCQCIntfHVBrowserBase::AreaForIndex(const   tCIDLib::TCard4     c4Index
                                    ,       TArea&              areaToFill) const
{
    // Create a scrolled version of the content area and pass to the other one
    TArea areaCont;
    MakeScrolledContArea(areaCont);
    AreaForIndex(c4Index, areaCont, areaToFill);
}

tCIDLib::TVoid
TCQCIntfHVBrowserBase::AreaForIndex(const   tCIDLib::TCard4     c4Index
                                    , const TArea&              areaRelTo
                                    ,       TArea&              areaToFill) const
{
    //
    //  Start with the target area. However, it may be empty, so we only do
    //  this to get the origin. If empty, its origin will be our return origin.
    //
    areaToFill = areaRelTo;

    // How far from the content origin is this one
    const tCIDLib::TInt4 i4Pos((c4Index * m_c4Size) + (c4Index * m_c4Spacing));

    // Adjust up from the origin by this much
    if (m_bHorizontal)
    {
        areaToFill.AdjustOrg(i4Pos, 0);
        areaToFill.SetSize(m_c4Size, m_areaDisplay.c4Height());
    }
     else
    {
        areaToFill.AdjustOrg(0, i4Pos);
        areaToFill.SetSize(m_areaDisplay.c4Width(), m_c4Size);
    }
}


//
//  Called to draw one of the images on a slot. We return a status to idnicate
//  if we did so or not.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bDrawImg(         TGraphDrawDev&  gdevTarget
                                , const TArea&          areaSlot
                                , const TString&        strImagePath)
{
    // See if we have this image and it's enabled. If not, then nothing to do
    if (!bImgEnabled(strImagePath))
        return kCIDLib::False;

    MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForKeyEO(strImagePath);
    if (!pimgiCur)
        return kCIDLib::False;

    facCQCIntfEng().DrawImage
    (
        gdevTarget
        , !TFacCQCIntfEng::bRemoteMode()
        , pimgiCur->m_cptrImg
        , areaSlot
        , areaSlot
        , pimgiCur->m_i4HOffset
        , pimgiCur->m_i4VOffset
        , pimgiCur->m_c1Opacity
    );
    return kCIDLib::True;
}



//
//  This is called when we get a scroll command. We have to adjust the current
//  position appropriately to achieve the requested scroll operation. The
//  operation values we get are the ones we provided as the possible values
//  of our scroll command.
//
//  We return true if we actually changed the left/top coordinate, just in
//  case it matters.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bScrollList( const   TString&            strScrollOp
                                    , const tCIDLib::TBoolean   bRedraw)
{
    // Remember the current offset
    tCIDLib::TInt4 i4OldPos;
    tCIDLib::TInt4 i4Span;
    if (m_bHorizontal)
    {
        if (m_areaDisplay.c4Width() > m_areaContent.c4Width())
            return kCIDLib::False;
        i4Span = tCIDLib::TInt4(m_areaDisplay.c4Width());
        i4OldPos = m_areaAdjDisplay.i4X();
    }
     else
    {
        if (m_areaDisplay.c4Height() > m_areaContent.c4Height())
            return kCIDLib::False;
        i4Span = tCIDLib::TInt4(m_areaDisplay.c4Height());
        i4OldPos = m_areaAdjDisplay.i4Y();
    }

    //
    //  Calc a new scroll position according to the command.
    //
    if ((strScrollOp == L"Top") || (strScrollOp == L"Home"))
    {
        // Move the content to our origin
        if (m_bHorizontal)
            m_areaAdjDisplay.LeftJustifyIn(m_areaContent);
        else
            m_areaAdjDisplay.TopJustifyIn(m_areaContent);
    }
     else if ((strScrollOp == L"Bottom") || (strScrollOp == L"End"))
    {
        // Move the content so it's at our right/botttom
        if (m_bHorizontal)
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);
        else
            m_areaAdjDisplay.BottomJustifyIn(m_areaContent);
    }
     else if ((strScrollOp == L"Up") || (strScrollOp == L"Left"))
    {
        if (m_bHorizontal)
            m_areaAdjDisplay.AdjustOrg(-(i4Span / 2), 0);
        else
            m_areaAdjDisplay.AdjustOrg(0, -(i4Span / 2));
    }
     else if ((strScrollOp == L"Down") || (strScrollOp == L"Right"))
    {
        if (m_bHorizontal)
            m_areaAdjDisplay.AdjustOrg(i4Span / 2, 0);
        else
            m_areaAdjDisplay.AdjustOrg(0, i4Span / 2);
    }
     else if ((strScrollOp == L"Next") || (strScrollOp == L"Next Page")
          ||  (strScrollOp == L"Prev") || (strScrollOp == L"Prev Page"))
    {
        const tCIDLib::TBoolean bNext
        (
            (strScrollOp == L"Next") || (strScrollOp == L"Next Page")
        );

        if (m_bHorizontal)
        {
            if (bNext)
                m_areaAdjDisplay.AdjustOrg(i4Span, 0);
            else
                m_areaAdjDisplay.AdjustOrg(-i4Span, 0);
        }
         else
        {
            if (bNext)
                m_areaAdjDisplay.AdjustOrg(0, i4Span);
            else
                m_areaAdjDisplay.AdjustOrg(0, -i4Span);
        }
    }

    //
    //  Clip the movement to the limits, and see if we actually moved
    //  or not.
    //
    tCIDLib::TInt4 i4NewPos;
    if (m_bHorizontal)
    {
        if (m_areaAdjDisplay.i4X() < m_areaContent.i4X())
            m_areaAdjDisplay.i4X(m_areaContent.i4X());
        if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);

        i4NewPos = m_areaAdjDisplay.i4X();
    }
     else
    {
        if (m_areaAdjDisplay.i4Y() < m_areaContent.i4Y())
            m_areaAdjDisplay.i4Y(m_areaContent.i4Y());
        if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
            m_areaAdjDisplay.BottomJustifyIn(m_areaContent);

        i4NewPos = m_areaAdjDisplay.i4Y();
    }

    // If the position changed, and we can redraw, then do it
    if (i4OldPos != i4NewPos)
    {
        //
        //  If remote mode, just redraw at the new position, else do a
        //  smooth scroll.
        //
        if (bRedraw)
        {
            if (TFacCQCIntfEng::bRemoteMode() || bNoInertia())
            {
                // Keep the focus index visible
                KeepFocusVisible();
                Redraw();
            }
             else
            {
                SlideList(i4OldPos);
            }
        }
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  Scrolls the list such that the indicated index is at the top slot, or as
//  close to that as we can get. If remote mode, we just update the first
//  index and redraw, else we do a sliding scroll.
//
tCIDLib::TBoolean
TCQCIntfHVBrowserBase::bScrollToIndex(  const   tCIDLib::TCard4     c4Index
                                        , const tCIDLib::TBoolean   bRedraw)
{
    //
    //  Update our adjusted display area such that the indicated index's
    //  column would be on the left/top. Then clip back if that's too far.
    //  Remember the original position so we can know if we changed.
    //
    tCIDLib::TInt4 i4OrgPos;
    tCIDLib::TInt4 i4NewPos;
    if (m_bHorizontal)
    {
        i4OrgPos = m_areaAdjDisplay.i4X();

        m_areaAdjDisplay.i4X(c4Index * (m_c4Size + m_c4Spacing));
        if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
            m_areaAdjDisplay.RightJustifyIn(m_areaContent);

        i4NewPos = m_areaAdjDisplay.i4X();
    }
     else
    {
        i4OrgPos = m_areaAdjDisplay.i4Y();

        m_areaAdjDisplay.i4Y(c4Index * (m_c4Size + m_c4Spacing));
        if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
            m_areaAdjDisplay.BottomJustifyIn(m_areaContent);

        i4NewPos = m_areaAdjDisplay.i4Y();
    }

    if (i4NewPos != i4OrgPos)
    {
        // Keep the focus index visible
        KeepFocusVisible();

        //
        //  If remote mode, just redraw at the new position, else do a
        //  smooth scroll.
        //
        if (bRedraw)
        {
            if (TFacCQCIntfEng::bRemoteMode())
                Redraw();
            else
                SlideList(i4OrgPos);
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
TCQCIntfHVBrowserBase::bVisibleEnough(          tCIDLib::TCard4&    c4Index
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
    const tCIDLib::TInt4 i4Limit = tCIDLib::TInt4(m_c4Size * (1.0 - f4Per));

    // Remember if we modified it
    const tCIDLib::TCard4 c4OrgVal = c4Index;

    // if not enough is visible, then move inwards if possible
    if (m_bHorizontal)
    {
        if (areaTest.i4X() < m_areaDisplay.i4X())
        {
            if (m_areaDisplay.i4X() - areaTest.i4X() >= i4Limit)
            {
                if (c4Index + 1 < c4NameCount())
                    c4Index++;
            }
        }
         else if (areaTest.i4Right() > m_areaDisplay.i4Right())
        {
            if ((areaTest.i4Right() - m_areaDisplay.i4Right() >= i4Limit)
            &&  c4Index)
            {
                c4Index--;
            }
        }
    }
     else
    {
        if (areaTest.i4Y() < m_areaDisplay.i4Y())
        {
            if (m_areaDisplay.i4Y() - areaTest.i4Y() >= i4Limit)
            {
                if (c4Index + 1 < c4NameCount())
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
    }

    // If we didn't have to change it, return true
    return (c4OrgVal == c4Index);
}


//
//  We have a special case issue when doing our inertial scrolls that we
//  need to capture any content under the items. That includes any popup
//  layers under us, and any widgets under us in our own layer, and
//  including our own background stuff (focus and slot backing images)
//  but not the items themselves. So it's sort of a special case.
//
//  We use a special method on the parent container to draw up to, but
//  not including us. Then we draw our background stuff over the top of
//  that. We return a bitmap with that content.
//
//  NOTE: This cannot be called from the regular update, since the big
//  background bitmap is already in use in that case. This is only to
//  be called from the scrolling method.
//
TBitmap TCQCIntfHVBrowserBase::bmpGatherBgn()
{
    //
    //  Get the stuff underneath us. WE tell it to stop when it hits our
    //  widget, and not to draw it (stop before the one we indicate.)
    //
    TBitmap bmpBgn = civOwner().bmpQueryContentUnder
    (
        m_areaDisplay, this, kCIDLib::True
    );

    //
    //  Now let's draw our bgn contents on top of that. WE need to get
    //  an area that is the full widget size in the transverse direction
    //  and center over the display area, to insure that any gradients
    //  line up.
    //
    TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);

    TArea areaFill(m_areaDisplay);
    if (m_bHorizontal)
    {
        areaFill.c4Height(areaActual().c4Height());
        areaFill.JustifyIn
        (
            m_areaDisplay, tCIDLib::EHJustify::Left, tCIDLib::EVJustify::Center
        );
    }
     else
    {
        areaFill.c4Width(areaActual().c4Width());
        areaFill.JustifyIn
        (
            m_areaDisplay, tCIDLib::EHJustify::Center, tCIDLib::EVJustify::Top
        );
    }
    DoBgnFill(gdevBgn, areaFill);

    // Create a bitmap our display size
    TBitmap bmpRet
    (
        m_areaDisplay.szArea()
        , tCIDImage::EPixFmts::TrueClr
        , tCIDImage::EBitDepths::Eight
    );

    // And copy the bgn contents over into our return bitmap
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
TCQCIntfHVBrowserBase::c4ClosestVisIndex(const tCIDLib::TCard4 c4FromHere) const
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
    const tCIDLib::TCard4 c4PerSlot = m_c4Size + m_c4Spacing;

    tCIDLib::TCard4 c4Ret = c4FromHere;
    if (!m_areaDisplay.bIntersects(areaTest))
    {
        //
        //  Not we have to use the ADJUSTED dislay area here, which
        //  calculating the new index!
        //
        if (m_bHorizontal)
        {
            if (areaTest.i4X() < m_areaDisplay.i4X())
                c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4X()) / c4PerSlot;
            else
                c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Right()) / c4PerSlot;
        }
         else
        {
            if (areaTest.i4Y() < m_areaDisplay.i4Y())
                c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Y()) / c4PerSlot;
            else
                c4Ret = tCIDLib::TCard4(m_areaAdjDisplay.i4Bottom()) / c4PerSlot;
        }
    }

    //
    //  Call the visible enough method which will modify the index if needed.
    //  We don't care if it does or doesn't. We require it to be at least
    //  40% visible.
    //
    bVisibleEnough(c4Ret, 0.4F);

    // And return whatever we ended up with
    return c4Ret;
}


//
//  Calc the first (at least partially) visible index. We have one that
//  takes a visible area and an area it is relative to, which we need in
//  order to deal with the scrollable content. We have another for our
//  display coordinate system.
//
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4FirstVisIndex() const
{
    // If no names, it has to be zero
    if (!c4NameCount())
        return 0;

    //
    //  In this case, we can just use the scroll offset, since it tells
    //  us how far we are scrolled, independent of orientation. It's always
    //  a positive number.
    //
    tCIDLib::TInt4 i4CurOfs;
    if (m_bHorizontal)
        i4CurOfs = m_areaAdjDisplay.i4X();
    else
        i4CurOfs = m_areaAdjDisplay.i4Y();

    return tCIDLib::TCard4(i4CurOfs) / (m_c4Size + m_c4Spacing);
}

tCIDLib::TCard4
TCQCIntfHVBrowserBase::c4FirstVisIndex( const   TArea&  areaVis
                                        , const TArea&  areaRelTo) const
{
    // If no names, it has to be zero
    if (!c4NameCount())
        return 0;

    // See how far we are scrolled from the content origin
    tCIDLib::TCard4 c4Diff;
    if (m_bHorizontal)
        c4Diff = tCIDLib::TCard4(areaVis.i4X() - areaRelTo.i4X());
    else
        c4Diff = tCIDLib::TCard4(areaVis.i4Y() - areaRelTo.i4Y());

    // And we can just divide to get the indx
    return (c4Diff / (m_c4Size + m_c4Spacing));
}


//
//  Tests to see if the passed point hits one of our items. Since we could
//  only get this call for an item within our visible area, we know it's going
//  to be one of those. This always works in terms of our display coordinates.
//  It's never used during scrolling.
//
tCIDLib::TCard4 TCQCIntfHVBrowserBase::c4HitTest(const TPoint& pntTest) const
{
    //
    //  Get the count of names. If none, then can't have hit a used slot.
    //  Or, if not within our area.
    //
    const tCIDLib::TCard4 c4Names = c4NameCount();
    if (!c4Names || !m_areaDisplay.bContainsPoint(pntTest))
        return kCIDLib::c4MaxCard;

    //
    //  Calc the difference between our display origin and the point, then
    //  add the scroll offset to get the difference.
    //
    tCIDLib::TCard4 c4Diff;
    if (m_bHorizontal)
    {
        c4Diff = tCIDLib::TCard4
        (
            (pntTest.i4X() - m_areaDisplay.i4X()) + m_areaAdjDisplay.i4X()
        );
    }
     else
    {
        c4Diff = tCIDLib::TCard4
        (
            (pntTest.i4Y() - m_areaDisplay.i4Y()) + m_areaAdjDisplay.i4Y()
        );
    }

    // And we can now just divide
    const tCIDLib::TCard4 c4Index = (c4Diff / (m_c4Size + m_c4Spacing));

    // Make sure we hit a valid one
    if (c4Index >= c4NameCount())
        return kCIDLib::c4MaxCard;

    return c4Index;
}


//
//  Recalculates our content and display areas and anything else that is
//  size/pos related. It is called any time our list changes or our size
//  pos changes. It also adjusts if we are already scrolled, to make sure
//  we still have enough content to be scrollable and that we aren't hanging
//  off the edge now, if the content area has gotten smaller.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::CalcAreas(const TArea& areaNew, const TArea& areaOld)
{
    //
    //  Figure out our display area. Start with our area, then move inwards
    //  for the border if we have one, else by one.
    //
    m_areaDisplay = areaNew;
    if (bHasBorder())
        m_areaDisplay.Deflate(2);
    else
        m_areaDisplay.Deflate(1);

    // If we have any rounding, then move in by one more.
    if (c4Rounding())
        m_areaDisplay.Deflate(1);

    //
    //  If no content, then we are zero sized at zero origin, so start with
    //  that as the default until proven otherwise below.
    //
    m_areaContent.ZeroAll();

    // If we have any names, then we may need to adjust it
    const tCIDLib::TCard4 c4Count = c4NameCount();
    if (c4Count)
    {
        //
        //  The area in the orientation direction is the number of names
        //  times the item size, plus that number of spaces minus one (don't
        //  need one past the last name.)
        //
        const tCIDLib::TCard4 c4Size
        (
            (m_c4Size * c4Count) + (m_c4Spacing * (c4Count - 1))
        );

        //
        //  Set the content to this size in the orientation direction, and
        //  the display area size in the other.
        //
        if (m_bHorizontal)
            m_areaContent.SetSize(c4Size, m_areaDisplay.c4Height());
        else
            m_areaContent.SetSize(m_areaDisplay.c4Width(), c4Size);
    }

    //
    //  Update the adjusted display area size to the new display area size
    //  unless the content is smaller, then take that. And see if we need
    //  to adjust.
    //
    if (m_bHorizontal)
    {
        m_areaAdjDisplay.c4Height(m_areaDisplay.c4Height());
        if (m_areaDisplay.c4Width() > m_areaContent.c4Width())
        {
            // No need to scroll anymore. Reset back to the origin
            m_areaAdjDisplay.i4X(0);
            m_areaAdjDisplay.c4Width(m_areaContent.c4Width());
        }
         else
        {
            // Make sure we aren't hanging of the end now
            m_areaAdjDisplay.c4Width(m_areaDisplay.c4Width());
            if (m_areaAdjDisplay.i4Right() > m_areaContent.i4Right())
                m_areaAdjDisplay.RightJustifyIn(m_areaContent);
        }
    }
     else
    {
        m_areaAdjDisplay.c4Width(m_areaDisplay.c4Width());
        if (m_areaDisplay.c4Height() > m_areaContent.c4Height())
        {
            // No need to scroll anymore. Reset back to the origin
            m_areaAdjDisplay.i4Y(0);
            m_areaAdjDisplay.c4Height(m_areaContent.c4Height());
        }
         else
        {
            // Make sure we aren't hanging of the end now
            m_areaAdjDisplay.c4Height(m_areaDisplay.c4Height());
            if (m_areaAdjDisplay.i4Bottom() > m_areaContent.i4Bottom())
                m_areaAdjDisplay.BottomJustifyIn(m_areaContent);
        }
    }

    // Keep the focus index visible
    KeepFocusVisible();
}


//
//  We get an invalid area, in terms of our zero based content area. We
//  go through and redraw any of the items that intersect that area. The
//  caller can pass an offset that we'll add to each one before we draw
//  it. This allows this same method to be used to draw directly into the
//  common double buffering bitmap when doing a regular update, but also
//  to be used to draw into a memory based zero origined bitmap when
//  doing a scroll or pan.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::DrawItems(       TGraphDrawDev&      gdevTarget
                                , const TArea&              areaRelTo
                                , const TArea&              areaInvalid
                                , const TPoint&             pntOffset
                                , const tCIDLib::TBoolean   bDoAlpha)
{
    const tCIDLib::TCard4 c4ItemCnt = c4NameCount();

    // Set our widget's font for this scope, and turn off bgn fill for text
    TCQCIntfFontJan janFont(&gdevTarget, *this);
    gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

    //
    //  Fault in our prefix width if not done that yet, and if we are numbered,
    //  and if we are vertical. Otherwise, we don't need it.
    //
    if (!m_c4PrefWidth && m_bNumbered && !m_bHorizontal)
    {
        // Figure out how many digits we have to display for the numbering
        tCIDLib::TCard4 c4Digits = 1;
        tCIDLib::TCard4 c4Remainder = c4ItemCnt;
        while (c4Remainder >= 10)
        {
            c4Digits++;
            c4Remainder /= 10;
        }

        // Get the widths of the digits, plus space and period
        static const tCIDLib::TCh* pszDigits(L"0123456789 .");
        TFundVector<tCIDLib::TCard4> fcolWidths(12);
        gdevTarget.QueryCharWidths(pszDigits, fcolWidths);

        //
        //  Find the largest one within the number of digits that we will
        //  have to actually display.
        //
        tCIDLib::TCard4 c4WidestDig = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < 12; c4Index++)
        {
            const tCIDLib::TCard4 c4Cur = fcolWidths[c4Index];
            if (c4Cur > c4WidestDig)
                c4WidestDig = c4Cur;
        }

        // Now we can calculate the width
        m_c4PrefWidth = (c4Digits * c4WidestDig)
                        + fcolWidths[10]
                        + fcolWidths[11];

        // If we are doing text f/x, then give a little more leeway
        if (eTextFX() != tCIDGraphDev::ETextFX::None)
            m_c4PrefWidth += 10;
    }

    // Remember if focus is enabled
    const tCIDLib::TBoolean bDoFocus = !iwdgRootTemplate().bNoFocus();

    //
    //  In designer mode, we just draw enough sample text values to
    //  fill the list. Else we draw real stuff.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        const TString strSampleText(L"Sample Text");

        // Get the content based based area of the first item
        tCIDLib::TCard4 c4Index = 0;
        TArea           areaCur;
        AreaForIndex(c4Index, areaRelTo, areaCur);

        // Set up a pen for stroking the item outlines
        const tCIDGraphDev::EMixModes eSaveMode = gdevTarget.eMixMode();
        TCosmeticPen gpenOutline(facCIDGraphDev().rgbWhite);
        TPenJanitor janPen(&gdevTarget, &gpenOutline);
        gdevTarget.eMixMode(tCIDGraphDev::EMixModes::XorPen);

        // Draw items until we go beyound the invalid area
        TArea areaStroke;
        while (kCIDLib::True)
        {
            if ((m_bHorizontal && (areaCur.i4X() > areaInvalid.i4Right()))
            ||  (!m_bHorizontal && (areaCur.i4Y() > areaInvalid.i4Bottom())))
            {
                break;
            }

            DrawItem
            (
                gdevTarget
                , (c4Index == 2) ? L"An extra long item in order to test right side alighment"
                                 : strSampleText
                , areaCur
                , c4Index
                , kCIDLib::False
                , bDoAlpha
            );

            areaStroke = areaCur;
            areaStroke.Deflate(3, 0);
            gdevTarget.Stroke(areaStroke);

            // Move the index and area forward (including spacing)
            c4Index++;
            AdjustBySlot(areaCur, kCIDLib::True);
        }
    }
     else if (c4ItemCnt)
    {
        //
        //  Figure out which item will be the first one visible within this
        //  invalid area. It might only be partially so. And get its area.
        //  We'll move forward from there until we go beyond the invalid
        //  area.
        //
        tCIDLib::TCard4 c4Index = c4FirstVisIndex(areaInvalid, areaRelTo);
        TArea areaCur;
        AreaForIndex(c4Index, areaRelTo, areaCur);

        // Loop till we move past the target area or run out of items
        TArea areaOffset;
        while (c4Index < c4ItemCnt)
        {
            if ((m_bHorizontal && (areaCur.i4X() > areaInvalid.i4Right()))
            ||  (!m_bHorizontal && (areaCur.i4Y() > areaInvalid.i4Bottom())))
            {
                break;
            }

            if (areaCur.bIntersects(areaInvalid))
            {
                areaOffset = areaCur;
                areaOffset += pntOffset;
                DrawItem
                (
                    gdevTarget
                    , strDisplayTextAt(c4Index)
                    , areaOffset
                    , c4Index
                    , bDoFocus && bHasFocus() && (m_c4FocusIndex == c4Index)
                    , bDoAlpha
                );
            }

            // Move the index and area forward (including spacing)
            c4Index++;
            AdjustBySlot(areaCur, kCIDLib::True);
        }
    }
     else
    {
        //
        //  No items. If we have the focus, draw the focus image on
        //  the 0th slot position, if we have a focus image.
        //
        if (bDoFocus && bHasFocus())
        {
            // Get the area of the 0th item, display area based
            TArea areaCur;
            AreaForIndex(0, areaRelTo, areaCur);
            bDrawImg(gdevTarget, areaCur, CQCIntfEng_HVBrowserBase::strFocus);
        }
    }
}


tCIDLib::TVoid
TCQCIntfHVBrowserBase::DrawItem(        TGraphDrawDev&      gdevTarget
                                , const TString&            strText
                                , const TArea&              areaTar
                                , const tCIDLib::TCard4     c4Index
                                , const tCIDLib::TBoolean   bDoFocus
                                , const tCIDLib::TBoolean   bDoAlpha)
{
    // Get a copy of the area we can modify
    TArea areaCur = areaTar;

    // Clip to this slot area
    TRegionJanitor janSlotClip(&gdevTarget, areaTar, tCIDGraphDev::EClipModes::And);

    //
    //  Adjust the current area by the text offset, if any, and deflate in a
    //  bit so that we have some margin on the edges.
    //
    areaCur = areaTar;
    areaCur.AdjustOrg(m_i4TextOfs, 0);
    areaCur.AdjustSize(-m_i4TextOfs, 0);
    areaCur.Deflate(3, 0);

    //
    //  If this is pressed item, display that, else if it's the focus item,
    //  display that.
    //
    if (c4Index == m_c4PressIndex)
    {
        // If no pressed image, do a fake depress
        if (!bDrawImg(gdevTarget, areaCur, CQCIntfEng_HVBrowserBase::strPressed))
            areaCur.AdjustOrg(1, 1);
    }
     else if (bDoFocus)
    {
        bDrawImg(gdevTarget, areaCur, CQCIntfEng_HVBrowserBase::strFocus);
    }

    // And now draw the text
    if (m_bNumbered)
    {
        TString strNum;
        strNum.SetFormatted(c4Index + 1);
        strNum.Append(L". ");

        if (m_bHorizontal)
        {
            strNum.Append(strText);
            DrawItemText(gdevTarget, areaCur, strNum, bDoFocus, bDoAlpha, kCIDLib::True);
        }
         else
        {
            // Draw this part first, in the prefix area
            TArea areaPref = areaCur;
            areaPref.c4Width(m_c4PrefWidth);
            DrawNumPref(gdevTarget, areaPref, strNum, bDoFocus, bDoAlpha);

            // Adjust the area over and draw the text
            areaCur.AdjustAndShrink(m_c4PrefWidth, 0);
            DrawItemText(gdevTarget, areaCur, strText, bDoFocus, bDoAlpha, kCIDLib::False);
        }
    }
     else
    {
        // Not numbered, so just draw the text
        DrawItemText(gdevTarget, areaCur, strText, bDoFocus, bDoAlpha, kCIDLib::False);
    }
}



//
//  This is called to draw the text for a single item, since there are a
//  number of things that need to be taken into account, and it clutters up
//  the main text drawing method below otherwise.
//
//  There's a special case where the caller wants us to display a numerical
//  prefix for a vertical list, in which we do separate columns for the
//  numbers and the text, so we will right justify the text in that case.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::DrawItemText(        TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaCur
                                    , const TString&            strText
                                    , const tCIDLib::TBoolean   bFocus
                                    , const tCIDLib::TBoolean   bDoAlpha
                                    , const tCIDLib::TBoolean   bNumPrefix)
{
    // Clip the output to this area
    TRegionJanitor janClip(&gdevTarget, areaCur, tCIDGraphDev::EClipModes::And);

    const tCIDLib::TBoolean bNoWrap = bNumPrefix ? kCIDLib::True : bNoTextWrap();
    const tCIDLib::EHJustify eHJust
    (
        bNumPrefix ? tCIDLib::EHJustify::Right : eHJustify()
    );
    const tCIDLib::EVJustify eVJust = eVJustify();
    const tCIDLib::TBoolean  bPremul = kCIDLib::False;

    // Decide which colors to use
    TRGBClr rgbText;
    TRGBClr rgbShadow;
    if (bFocus)
    {
        rgbText = rgbExtra();
        rgbShadow = rgbExtra2();
    }
     else
    {
        rgbText = rgbFgn();
        rgbShadow = rgbFgn2();
    }

    TCQCIntfView& civUs = civOwner();
    if (eTextFX() != tCIDGraphDev::ETextFX::None)
    {
        civUs.DrawTextFX
        (
            gdevTarget
            , strText
            , areaCur
            , eTextFX()
            , rgbText
            , rgbShadow
            , eHJust
            , eVJust
            , bNoWrap
            , pntOffset()
        );
    }
     else
    {
        TArea areaTmp;
        if (bShadow())
        {
            //
            //  The drop amount is scaled by the font height. From 0
            //  to 24, we do one pixel. From there we do (height/12).
            //
            tCIDLib::TCard4 c4Drop = 1;
            if (c4FontHeight() > 24)
                  c4Drop = (c4FontHeight() / 12);
            areaTmp = areaCur;
            areaTmp.AdjustOrg(c4Drop, c4Drop);
            gdevTarget.SetTextColor(rgbShadow);
            if (bDoAlpha)
            {
                if (bNoWrap)
                    civUs.DrawTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);
                else
                    civUs.DrawMTextAlpha(gdevTarget, strText, areaTmp, eHJust, eVJust);

            }
             else
            {
                if (bNoWrap)
                    gdevTarget.DrawString(strText, areaTmp, eHJust, eVJust);
                else
                    gdevTarget.DrawMText(strText, areaTmp, eHJust, eVJust);
            }
        }

        gdevTarget.SetTextColor(rgbText);
        if (bDoAlpha)
        {
            if (bNoWrap)
                civUs.DrawTextAlpha(gdevTarget, strText, areaCur, eHJust, eVJust);
            else
                civUs.DrawMTextAlpha(gdevTarget, strText, areaCur, eHJust, eVJust);
        }
         else
        {
            if (bNoWrap)
                gdevTarget.DrawString(strText, areaCur, eHJust, eVJust);
            else
                gdevTarget.DrawMText(strText, areaCur, eHJust, eVJust);
        }
    }
}


// Just calls DrawItemText() above with the last parm set to true
tCIDLib::TVoid
TCQCIntfHVBrowserBase::DrawNumPref(         TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaCur
                                    , const TString&            strText
                                    , const tCIDLib::TBoolean   bFocus
                                    , const tCIDLib::TBoolean   bDoAlpha)
{
    DrawItemText(gdevTarget, areaCur, strText, bFocus, bDoAlpha, kCIDLib::True);
}


//
//  Any time anything changes that might make the focus index go out of
//  visibility, we need to get it back visible. It's not a lot of code but
//  it's called from a number of places, so best to keep it in one place.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::KeepFocusVisible()
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
//  There are a number of places where we need to get the content area,
//  scrolled up/left by the scroll offset, i.e. to move it so that it's
//  correctly relative to our area for the current content to show through
//  our area.
//
tCIDLib::TVoid
TCQCIntfHVBrowserBase::MakeScrolledContArea(TArea& areaToFill) const
{
    // Start with the content area and adjust to our display origin
    areaToFill = m_areaContent;
    areaToFill.SetOrg(m_areaDisplay.pntOrg());

    // And subtract the scroll offset, which is always positive
    areaToFill -= m_areaAdjDisplay.pntOrg();
}



//
//  This is called when we need to move the list in a non-drag way, i.e.
//  based on a command. We set up the content that we need to display and
//  slide it in.
//
//  On entry the adjusted display area has been updated to a new position,
//  and we get the orginal position before it was changed. We can use these
//  to bits to figure out what needs to be scrolled.
//
//  We have to be careful here since the meaning is sort of backwards since
//  the caller has moved the adjusted display area, whereas we have to slide
//  the content in the opposite direction to make that happen visually.
//
//  There are two scenarios:
//
//  1. We are scrolling a contiguous range, up to two full page's worth
//  2. We are moving from one to another discontiguous range.
//
//  It affects how large our scrollable area is and how we draw into it.
//
//  For scenario one, we just move that range from where it was to where it
//  is. For scenario two, we only bother to draw the new content and leave
//  the other side blank and just scroll in the new stuff.
//
tCIDLib::TVoid TCQCIntfHVBrowserBase::SlideList(const tCIDLib::TInt4 i4OrgPos)
{
    // Set up the whole area we need to scroll
    TArea areaScroll;

    // Figure out which scenario
    TArea areaFirst;
    TArea areaSec;

    tCIDLib::EDirs eDir;
    if (m_bHorizontal)
    {
        if (i4OrgPos > m_areaAdjDisplay.i4X())
        {
            eDir = tCIDLib::EDirs::Right;
            areaFirst = m_areaAdjDisplay;
            areaSec = m_areaAdjDisplay;
            areaSec.i4X(i4OrgPos);
        }
         else
        {
            eDir = tCIDLib::EDirs::Left;
            areaFirst = m_areaAdjDisplay;
            areaFirst.i4X(i4OrgPos);
            areaSec = m_areaAdjDisplay;
        }
    }
     else
    {
        if (i4OrgPos > m_areaAdjDisplay.i4Y())
        {
            eDir = tCIDLib::EDirs::Down;
            areaFirst = m_areaAdjDisplay;
            areaSec = m_areaAdjDisplay;
            areaSec.i4Y(i4OrgPos);
        }
         else
        {
            eDir = tCIDLib::EDirs::Up;
            areaFirst = m_areaAdjDisplay;
            areaFirst.i4Y(i4OrgPos);
            areaSec = m_areaAdjDisplay;
        }
    }

    //
    //  If the two areas overlap (or just touch) we have a contiguous range,
    //  else we have to do two separate draws.
    //
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TBitmap bmpScroll;
    if (areaFirst.bIntersects(areaSec)
    ||  (m_bHorizontal && (areaFirst.i4Right() == areaSec.i4X()))
    ||  (!m_bHorizontal && (areaFirst.i4Bottom() == areaSec.i4Y())))
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
            if (m_bHorizontal)
                areaFill.c4Height(areaActual().c4Height());
            else
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
        DrawItems(gdevCont, areaRelTo, areaScroll, TPoint::pntOrigin, !m_bSafePanBgn);
    }
     else
    {
        // The scroll area is just twice the display area
        if (m_bHorizontal)
        {
            areaScroll.SetSize
            (
                m_areaDisplay.c4Width() * 2, m_areaDisplay.c4Height()
            );
        }
         else
        {
            areaScroll.SetSize
            (
                m_areaDisplay.c4Width(), m_areaDisplay.c4Height() * 2
            );
        }

        bmpScroll = TBitmap
        (
            areaScroll.szArea(), tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight
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
            if (m_bHorizontal)
                areaFill.c4Height(areaActual().c4Height());
            else
                areaFill.c4Width(areaActual().c4Width());
            areaFill.CenterIn(areaScroll);
            DoBgnFill(gdevCont, areaFill);

            TBitmap bmpBgn = bmpGatherBgn();
            TGraphMemDev gdevBgn(civOwner().gdevCompat(), bmpBgn);
            cptrGraphDev->CopyBits(gdevBgn, areaZDisp, m_areaDisplay);
        }

        TArea areaRelTo;
        if ((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Left))
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

            if (eDir == tCIDLib::EDirs::Up)
                eDir = tCIDLib::EDirs::Down;
            else if (eDir == tCIDLib::EDirs::Left)
                eDir = tCIDLib::EDirs::Right;
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
                , m_bHorizontal ? areaZDisp.pntUR(1, 0) : areaZDisp.pntLL(0, 1)
                , !m_bSafePanBgn
            );

            if (eDir == tCIDLib::EDirs::Down)
                eDir = tCIDLib::EDirs::Up;
            else if (eDir == tCIDLib::EDirs::Right)
                eDir = tCIDLib::EDirs::Left;
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
        //  We have to do the heavy version, where it redraws each time.
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

