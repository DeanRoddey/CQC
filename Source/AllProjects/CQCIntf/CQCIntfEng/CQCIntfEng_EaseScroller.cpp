//
// FILE NAME: CIDWnd_EaseScroller.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/05/2013
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
//  This
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
RTTIDecls(TCIDEaseScroller,TObject)



// ---------------------------------------------------------------------------
//   CLASS: TCIDEaseScrCBD
//  PREFIX: escrcbd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCIDEaseScrCBD: Destructor
// ---------------------------------------------------------------------------
TCIDEaseScrCBD::~TCIDEaseScrCBD()
{
}


// ---------------------------------------------------------------------------
//  TCIDEaseScrollerCBD: Hidden constructors
// ---------------------------------------------------------------------------
TCIDEaseScrCBD::TCIDEaseScrCBD()
{
}




// ---------------------------------------------------------------------------
//   CLASS: MCIDEaseScrollerCB
//  PREFIX: escrcb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCIDEaseScrollerCB: Destructor
// ---------------------------------------------------------------------------
MCIDEaseScrollerCB::~MCIDEaseScrollerCB()
{
}


// ---------------------------------------------------------------------------
//  MCIDEaseScrollerCB: Hidden constructors
// ---------------------------------------------------------------------------
MCIDEaseScrollerCB::MCIDEaseScrollerCB()
{
}



// ---------------------------------------------------------------------------
//   CLASS: TCIDEaseScroller
//  PREFIX: eases
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCIDEaseScroller: Constructors and Destructor
// ---------------------------------------------------------------------------

TCIDEaseScroller::TCIDEaseScroller()
{
}

TCIDEaseScroller::~TCIDEaseScroller()
{
}


// ---------------------------------------------------------------------------
//  TCIDEaseScroller: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method does a version of the ease scroll where the caller can
//  provide us with the scrollable material in an alpha blit'able form. In
//  case we can do the whole thing here ourself efficiently, since we can
//  just blit out of the scrollable content at increasing or decreasing
//  origins. It's not alway s possible to get the content into a reasonably
//  alpha blittable format.
//
//  The caller provides:
//
//  1. Pre-captured background content that we can blit the scrolling content
//     on top of each round.
//  2. The scrollable content image, which will be some amount larger than the
//     bgn in the direction of the scroll.
//  3. A temp buffer bitmap to use for the double buffering
//
//  1 and 3 are the same size and represent the visible area. areaTar is this
//  size and positioned at the actual display position. areaScroll is the area
//  of the scrollable content (zero based.)
//
tCIDLib::TVoid
TCIDEaseScroller::Scroll1(          TGraphDrawDev&  gdevTar
                            , const TArea&          areaTar
                            , const TArea&          areaScroll
                            ,       TGraphDrawDev&  gdevBuf
                            ,       TGraphDrawDev&  gdevBgn
                            ,       TGraphDrawDev&  gdevScroll
                            , const tCIDLib::TCard4 c4Millis
                            , const tCIDLib::EDirs  eDir
                            , const tCIDLib::TCard4 c4Power)
{
    // This one is just in terms of a private helper
    Scroll1_2
    (
        gdevTar
        , areaTar
        , areaScroll
        , &gdevBuf
        , &gdevBgn
        , gdevScroll
        , c4Millis
        , eDir
        , c4Power
    );
}



//
//  This is a version of the algorithm that's more efficient when the scrollable
//  content is displayed on a consistent background, so that there's no need
//  to continually composite the the content over a pre-captured background.
//  So the scrollable content is just drawn over the background and we can
//  just directly scroll it.
//
tCIDLib::TVoid
TCIDEaseScroller::Scroll2(          TGraphDrawDev&  gdevTar
                            , const TArea&          areaTar
                            , const TArea&          areaScroll
                            ,       TGraphDrawDev&  gdevScroll
                            , const tCIDLib::TCard4 c4Millis
                            , const tCIDLib::EDirs  eDir
                            , const tCIDLib::TCard4 c4Power)
{
    // This one is just in terms of a private helper
    Scroll1_2
    (
        gdevTar
        , areaTar
        , areaScroll
        , 0
        , 0
        , gdevScroll
        , c4Millis
        , eDir
        , c4Power
    );
}



//
//  The worst case scenario is where the client code cannot reasonably create
//  an alpha blittable version of the scrollable content. This isn't that
//  unusual necessarily, given Window's lack of good support for alpha
//  channel in graphics operations.
//
//  In this case the caller provides us with a callback object. We have to
//  call it on new position, to let it draw the content into a buffer that
//  we can then display. The caller handles drawing the background and the
//  scrollable content
//
tCIDLib::TVoid
TCIDEaseScroller::Scroll3(          TGraphDrawDev&          gdevTar
                            , const TArea&                  areaTar
                            , const TArea&                  areaScroll
                            ,       TGraphDrawDev&          gdevUser
                            ,       MCIDEaseScrollerCB&     escrcbDraw
                            ,       TCIDEaseScrCBD&         escrcbdCtx
                            , const tCIDLib::TCard4         c4Millis
                            , const tCIDLib::EDirs          eDir
                            , const tCIDLib::TCard4         c4Power)
{
    // The size of the area we are scrolling and of the combined content
    const TSize& szImg = areaTar.szArea();
    const TSize& szComb = areaScroll.szArea();

    // The zero based area of the bgn image
    const TArea areaImg(TPoint::pntOrigin, szImg);

    //
    //  Now figure out the amount we have to travel in pixels, based on the
    //  direction. And we set up the starting offset within the source scroll
    //  area. And we set up a max offset, to use for clipping back in case
    //  of rounding issues.
    //
    tCIDLib::TInt4 i4LastOfs;
    tCIDLib::TInt4 i4Travel;

    switch(eDir)
    {
        case tCIDLib::EDirs::Left :
            i4Travel = tCIDLib::TInt4(szComb.c4Width() - szImg.c4Width());
            i4LastOfs = 0;
            break;

        case tCIDLib::EDirs::Right :
            i4Travel = tCIDLib::TInt4(szComb.c4Width() - szImg.c4Width());
            i4LastOfs = i4Travel;
            break;

        case tCIDLib::EDirs::Up :
            i4Travel = tCIDLib::TInt4(szComb.c4Height() - szImg.c4Height());
            i4LastOfs = 0;
            break;

        case tCIDLib::EDirs::Down :
            i4Travel = tCIDLib::TInt4(szComb.c4Height() - szImg.c4Height());
            i4LastOfs = i4Travel;
            break;

        default :
            break;
    };

    //
    //  OK, now we loop, with a couple millis per loop. Each time we wake up
    //  and calculate the new pixel position. If we get a new pixel, then we
    //  build up another composite image and blit it to the screen.
    //
    //  If the millis is less than a hundred, we use that.
    //
    const tCIDLib::TEncodedTime enctRange
    (
        (c4Millis < 100 ? 100 : c4Millis) * kCIDLib::enctOneMilliSec
    );
    tCIDLib::TEncodedTime       enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd(enctCur + enctRange);
    const tCIDLib::TFloat8      f8Range = tCIDLib::TFloat8(enctRange);
    tCIDLib::TFloat8            f8Per = 1.0;

    //
    //  Loop until we get down to zero percent of time left. We will
    //  do zero before we come back to the top here and see we have hit
    //  it. Else we wouldn't get all the way to the end.
    //
    TArea areaCurSrc;
    while (f8Per > 0.0)
    {
        // Figure out the percentage of the whole time we'd done so far
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
            f8Per = 0;
        else
            f8Per = tCIDLib::TFloat8(enctEnd - enctCur) / f8Range;

        //
        //  Calculate the new amount of travel (in the direction of travel.)
        //  This is where we do the easing algorthm.
        //
        const tCIDLib::TFloat8 f8Pwr = TMathLib::f8Power(f8Per, c4Power);
        tCIDLib::TInt4 i4NewOfs;
        i4NewOfs = tCIDLib::TInt4((1.0 - f8Pwr) * tCIDLib::TFloat8(i4Travel));

        // Handle a possible rounding issue
        if (i4NewOfs < 0)
            i4NewOfs = 0;
        else if (i4NewOfs > i4Travel)
            i4NewOfs = i4Travel;

        //
        //  If we didn't change position, then sleep a bit then go back to the top
        //
        if (i4NewOfs == i4LastOfs)
        {
            facCIDCtrls().MsgYield(4);
            continue;
        }

        // It changed, so store the new one
        i4LastOfs = i4NewOfs;

        //
        //  Now we can use the new offset to set up an area within the scrollable
        //  source that we want to blit to the screen next.
        //
        areaCurSrc = areaImg;
        switch(eDir)
        {
            case tCIDLib::EDirs::Left :
                areaCurSrc.i4X(i4NewOfs);
                break;

            case tCIDLib::EDirs::Right :
                areaCurSrc.i4X(i4Travel - i4NewOfs);
                break;

            case tCIDLib::EDirs::Up :
                areaCurSrc.i4Y(i4NewOfs);
                break;

            case tCIDLib::EDirs::Down :
                areaCurSrc.i4Y(i4Travel - i4NewOfs);
                break;

            default :
                break;
        };

        // Ask the callback object to draw this part of the source
        escrcbDraw.EaseDrawAt(gdevUser, areaCurSrc, areaImg, &escrcbdCtx);

        //
        //  And copy to the screen, waiting for the retrace. This also provides
        //  throttling as well.
        //
        gdevTar.CopyBits
        (
            gdevUser
            , areaImg
            , areaTar
            , tCIDGraphDev::EBmpModes::SrcCopy
            , kCIDLib::True
        );
    }
}


//
//  This is a special case one, the simplest of all, where we need to slide in or
//  out a non-transparent area. We get the new stuff and the old stuff, which is
//  new content and underlying bgn stuff. We are also told if this is a push or pop,
//  since we do slightly different things between scrolling new content in and
//  scrolling that new content back out to expose the underlying stuff.
//
//  We do this pretty simply. We create a temporary bitmap. On each round we can
//  put the old stuff in, and then blit the new stuff in increasing/decreasing
//  positions, so that the new stuff appears to slide in or out.
//
tCIDLib::TVoid
TCIDEaseScroller::Scroll4(          TGraphDrawDev&      gdevTar
                            , const TArea&              areaTar
                            ,       TGraphDrawDev&      gdevNew
                            ,       TGraphDrawDev&      gdevBgn
                            , const tCIDLib::TCard4     c4Millis
                            , const tCIDLib::EDirs      eDir
                            , const tCIDLib::TCard4     c4Power
                            , const tCIDLib::TBoolean   bPush)
{
    // The size of the area we are scrolling
    const TSize& szImg = areaTar.szArea();

    // The source blit area is always the target area at zero origin
    TArea areaSrcBlt = areaTar;
    areaSrcBlt.ZeroOrg();

    // Create a target blit, which we will adjust as we go
    TArea areaTarBlt = areaSrcBlt;

    //
    //  Remember the full travel distance in the scroll direction, and in some cases
    //  we start the target blit area at the far end of the target area, so we need
    //  to move it.
    //
    tCIDLib::TInt4 i4Travel;
    switch(eDir)
    {
        case tCIDLib::EDirs::Down :
            i4Travel = tCIDLib::TInt4(areaTar.c4Height());

            if (bPush)
                areaTarBlt.Move(tCIDLib::EDirs::Up);
            break;

        case tCIDLib::EDirs::Left :
            i4Travel = tCIDLib::TInt4(areaTar.c4Width());

            if (bPush)
                areaTarBlt.Move(tCIDLib::EDirs::Right);
            break;

        case tCIDLib::EDirs::Right :
            i4Travel = tCIDLib::TInt4(areaTar.c4Width());

            if (bPush)
                areaTarBlt.Move(tCIDLib::EDirs::Left);
            break;

        case tCIDLib::EDirs::Up :
            i4Travel = tCIDLib::TInt4(areaTar.c4Height());

            if (bPush)
                areaTarBlt.Move(tCIDLib::EDirs::Down);
            break;

        default :
            break;
    };

    //
    //  Create a temp buffer bitmap to composite into on each round, no alpha is
    //  required and it doesn't have to be in memory.
    //
    TBitmap bmpBuf
    (
        szImg, tCIDImage::EPixFmts::TrueClr, tCIDImage::EBitDepths::Eight
    );
    TGraphMemDev gdevBuf(gdevTar, bmpBuf);


    //
    //  OK, now we loop, with a couple millis per loop. Each time we wake up
    //  and calculate the new pixel position. If we get a new position, then we
    //  do another blit.
    //
    //  If the millis is less than a hundred, we use that. WE wil loop until we
    //  get to zero percent of the available travel, which means there's nothing
    //  left to do.
    //
    const tCIDLib::TEncodedTime enctRange
    (
        (c4Millis < 100 ? 100 : c4Millis) * kCIDLib::enctOneMilliSec
    );
    tCIDLib::TEncodedTime       enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd(enctCur + enctRange);
    const tCIDLib::TFloat8      f8Range = tCIDLib::TFloat8(enctRange);
    tCIDLib::TFloat8            f8Per = 1.0;

    //
    //  Loop until we get down to zero percent of time left. We will do zero before
    //  we come back to the top here and see we have hit it. Else we wouldn't get all
    //  the way to the end.
    //
    tCIDLib::TBoolean bFirstTime = kCIDLib::True;
    tCIDLib::TInt4 i4LastOfs = i4Travel;
    while (f8Per > 0.0)
    {
        // Figure out the percentage of the whole time we'd done so far
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
            f8Per = 0;
        else
            f8Per = tCIDLib::TFloat8(enctEnd - enctCur) / f8Range;

        //
        //  Calculate the new amount of travel (in the direction of travel.)
        //  This is where we do the easing algorthm.
        //
        const tCIDLib::TFloat8 f8Pwr = TMathLib::f8Power(f8Per, c4Power);
        tCIDLib::TInt4 i4NewOfs;

        // Use that to calculate a new position (percentage of the full travel)
        i4NewOfs = tCIDLib::TInt4(f8Pwr * tCIDLib::TFloat8(i4Travel));

        // Handle a possible rounding issue
        if (i4NewOfs < 0)
            i4NewOfs = 0;
        else if (i4NewOfs > i4Travel)
            i4NewOfs = i4Travel;

        //
        //  If we didn't change offset, then go back to the top after sleeping a
        //  short amount of time.
        //
        if (i4NewOfs == i4LastOfs)
        {
            facCIDCtrls().MsgYield(4);
            continue;
        }

        //
        //  Remember a delta from the last offset before we save it. Because
        //  of the way we do the new offset, this is always positive, even
        //  if we are moving up or left!
        //
        const tCIDLib::TInt4 i4Delta = i4LastOfs - i4NewOfs;
        i4LastOfs = i4NewOfs;

        //
        //  If not pushing, or this is the first time, put the old content into
        //  the buffer bitmap. When pushing we only need to do it once, since the
        //  new stuff is growing and just takes over more of the buffer.
        //
        //  When popping, the new stuff is shrinking so we have to keep putting
        //  the background content back to fill in that bit.
        //
        if (!bPush || bFirstTime)
        {
            gdevBuf.CopyBits
            (
                gdevBgn
                , areaSrcBlt
                , areaSrcBlt
                , tCIDGraphDev::EBmpModes::SrcCopy
                , kCIDLib::False
            );

            bFirstTime = kCIDLib::True;
        }

        // Move the target blit area appropriately for the direction
        switch(eDir)
        {
            case tCIDLib::EDirs::Down :
                areaTarBlt.AdjustOrg(0, i4Delta);
                break;

            case tCIDLib::EDirs::Left :
                areaTarBlt.AdjustOrg(-i4Delta, 0);
                break;

            case tCIDLib::EDirs::Right :
                areaTarBlt.AdjustOrg(i4Delta, 0);
                break;

            case tCIDLib::EDirs::Up :
                areaTarBlt.AdjustOrg(0, -i4Delta);
                break;

            default :
                break;
        };

        //
        //  Copy the new stuff into the buffer at the right spot. It will automatically
        //  clip since we are drawing into a memory buffer of the required size. So
        //  this isn't as inefficient as it might otherwise be. Only the required
        //  amount will actually get blitted.
        //
        gdevBuf.CopyBits
        (
            gdevNew
            , areaSrcBlt
            , areaTarBlt
            , tCIDGraphDev::EBmpModes::SrcCopy
            , kCIDLib::False
        );

        //
        //  And blit the who buffer to the screen, waiting for retrace. This provides
        //  trottling as well.
        //
        gdevTar.CopyBits
        (
            gdevBuf
            , areaSrcBlt
            , areaTar
            , tCIDGraphDev::EBmpModes::SrcCopy
            , kCIDLib::True
        );
    }
}



// ---------------------------------------------------------------------------
//  TCIDEaseScroller: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A private helper that can support the 1 and 2 modes. The only difference is
//  if there's background content in the scrollable image, or if it has to be
//  added in each round. They just call us and pass us a background or not.
//  We also don't need the buffer image in mode 2, since we don't have to do
//  any compositing.
//
tCIDLib::TVoid
TCIDEaseScroller::Scroll1_2(        TGraphDrawDev&          gdevTar
                            , const TArea&                  areaTar
                            , const TArea&                  areaScroll
                            ,       TGraphDrawDev* const    pgdevBuf
                            ,       TGraphDrawDev* const    pgdevBgn
                            ,       TGraphDrawDev&          gdevScroll
                            , const tCIDLib::TCard4         c4Millis
                            , const tCIDLib::EDirs          eDir
                            , const tCIDLib::TCard4         c4Power)
{
    // The size of the area we are scrolling and of the combined content
    const TSize& szImg = areaTar.szArea();
    const TSize& szComb = areaScroll.szArea();

    // The zero based area of the bgn image
    const TArea areaImg(TPoint::pntOrigin, szImg);

    //
    //  Now figure out the amount we have to travel in pixels, based on the
    //  direction. And we set up the starting offset within the source scroll
    //  area. And we set up a max offset, to use for clipping back in case
    //  of rounding issues.
    //
    tCIDLib::TInt4 i4LastOfs;
    tCIDLib::TInt4 i4Travel;

    switch(eDir)
    {
        case tCIDLib::EDirs::Left :
            i4Travel = tCIDLib::TInt4(szComb.c4Width() - szImg.c4Width());
            i4LastOfs = 0;
            break;

        case tCIDLib::EDirs::Right :
            i4Travel = tCIDLib::TInt4(szComb.c4Width() - szImg.c4Width());
            i4LastOfs = 0; // i4Travel;
            break;

        case tCIDLib::EDirs::Up :
            i4Travel = tCIDLib::TInt4(szComb.c4Height() - szImg.c4Height());
            i4LastOfs = 0;
            break;

        case tCIDLib::EDirs::Down :
            i4Travel = tCIDLib::TInt4(szComb.c4Height() - szImg.c4Height());
            i4LastOfs = 0; // i4Travel;
            break;

        default :
            break;
    };

    //
    //  OK, now we loop, with a couple millis per loop. Each time we wake up
    //  and calculate the new pixel position. If we get a new pixel, then we
    //  build up another composite image and blit it to the screen.
    //
    //  If the millis is less than a hundred, we use that.
    //
    const tCIDLib::TEncodedTime enctRange
    (
        (c4Millis < 100 ? 100 : c4Millis) * kCIDLib::enctOneMilliSec
    );
    tCIDLib::TEncodedTime       enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd(enctCur + enctRange);
    const tCIDLib::TFloat8      f8Range = tCIDLib::TFloat8(enctRange);
    tCIDLib::TFloat8            f8Per = 1.0;

    //
    //  Loop until we get down to zero percent of time left. We will
    //  do zero before we come back to the top here and see we have hit
    //  it. Else we wouldn't get all the way to the end.
    //
    TArea areaCurSrc;
    while (f8Per > 0.0)
    {
        // Figure out the percentage of the whole time we'd done so far
        enctCur = TTime::enctNow();
        if (enctCur >= enctEnd)
            f8Per = 0;
        else
            f8Per = tCIDLib::TFloat8(enctEnd - enctCur) / f8Range;

        //
        //  Calculate the new amount of travel (in the direction of travel.)
        //  This is where we do the easing algorthm.
        //
        const tCIDLib::TFloat8 f8Pwr = TMathLib::f8Power(f8Per, c4Power);
        tCIDLib::TInt4 i4NewOfs;
        i4NewOfs = tCIDLib::TInt4((1.0 - f8Pwr) * tCIDLib::TFloat8(i4Travel));

        // Handle a possible rounding issue
        if (i4NewOfs < 0)
            i4NewOfs = 0;
        else if (i4NewOfs > i4Travel)
            i4NewOfs = i4Travel;

        //
        //  If we didn't change position, then sleep a bit then go back to the top
        //
        if (i4NewOfs == i4LastOfs)
        {
            facCIDCtrls().MsgYield(4);
            continue;
        }

        //
        //  Now we can use the new ofs to set up an area within the scrollable
        //  source that we want to blit to the screen next.
        //
        areaCurSrc = areaImg;
        switch(eDir)
        {
            case tCIDLib::EDirs::Left :
                areaCurSrc.i4X(i4NewOfs);
                break;

            case tCIDLib::EDirs::Right :
                areaCurSrc.i4X(i4Travel - i4NewOfs);
                break;

            case tCIDLib::EDirs::Up :
                areaCurSrc.i4Y(i4NewOfs);
                break;

            case tCIDLib::EDirs::Down :
                areaCurSrc.i4Y(i4Travel - i4NewOfs);
                break;

            default :
                break;
        };

        //
        //  Remember a delta from the last offset before we save it. Because
        //  of the way we do the new offset, this is always positive, even
        //  if we are moving up or left!
        //
        tCIDLib::TInt4 i4Delta = i4NewOfs - i4LastOfs;
        i4LastOfs = i4NewOfs;

        //
        //  Now build up the image. We first blit it in the background
        //  image to the double buffer if we got one.
        //
        if (pgdevBgn)
        {
            CIDAssert(pgdevBuf != 0, L"The scroll buffer device must be provided");

            // Composite the background and foreground info
            pgdevBuf->CopyBits(*pgdevBgn, areaImg, areaImg);
            pgdevBuf->AlphaBlit(gdevScroll, areaCurSrc, areaImg, 0xFF, kCIDLib::True);

            // And finally copy the result to the target. Wait for the retrace
            gdevTar.CopyBits
            (
                *pgdevBuf
                , areaImg
                , areaTar
                , tCIDGraphDev::EBmpModes::SrcCopy
                , kCIDLib::True
            );
        }
         else
        {
            //
            //  Just blit straight to the target area, but in this case we can move
            //  the on-screen content and only blit the bit that we uncovered, which
            //  makes it far more efficient.
            //
            //  Tell it to wait for the retrace. This also provides some throttling
            //  as well.
            //
            tCIDLib::TInt4 i4XMove = 0;
            tCIDLib::TInt4 i4YMove = 0;

            // Remember delta is always positive!
            if (eDir == tCIDLib::EDirs::Left)
                i4XMove = i4Delta * -1;
            else if (eDir == tCIDLib::EDirs::Right)
                i4XMove = i4Delta;
            else if (eDir == tCIDLib::EDirs::Up)
                i4YMove = i4Delta * -1;
            else if (eDir == tCIDLib::EDirs::Down)
                i4YMove = i4Delta;

            TArea areaUpdateTar;
            gdevTar.ScrollBits
            (
                areaTar
                , areaTar
                , i4XMove
                , i4YMove
                , areaUpdateTar
                , kCIDLib::True
            );

            //
            //  Set up the correct source. Be sure to center in the transvese
            //  direction since the target area we get is relative to the window
            //  not to our widget origin.
            //
            TArea areaUpdateSrc(areaUpdateTar);
            if (eDir == tCIDLib::EDirs::Left)
                areaUpdateSrc.RightJustifyIn(areaCurSrc, kCIDLib::True);
            else if (eDir == tCIDLib::EDirs::Right)
                areaUpdateSrc.LeftJustifyIn(areaCurSrc, kCIDLib::True);
            else if (eDir == tCIDLib::EDirs::Up)
                areaUpdateSrc.BottomJustifyIn(areaCurSrc, kCIDLib::True);
            else if (eDir == tCIDLib::EDirs::Down)
                areaUpdateSrc.TopJustifyIn(areaCurSrc, kCIDLib::True);

            //
            //  And blit the uncovered area. In this case, we don't wait for the
            //  retrace. This will just be a little bit on the edge. The scrolling
            //  of the overall display area above is the important part. If we
            //  waited again here, it would double the wait time.
            //
            gdevTar.CopyBits
            (
                gdevScroll
                , areaUpdateSrc
                , areaUpdateTar
                , tCIDGraphDev::EBmpModes::SrcCopy
                , kCIDLib::False
            );
        }
    }
}

