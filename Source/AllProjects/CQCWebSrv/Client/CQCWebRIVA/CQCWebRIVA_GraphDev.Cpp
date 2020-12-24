//
// FILE NAME: CQCWebRIVA_GraphDev.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/24/2009
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
//  This file implements the remoted grraphics device. We wrap a real device
//  and intercept any commands that the interface engine uses and queue them
//  up for transmission to the remote client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCWebRIVA_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TWebRIVAGraphDev,TGraphDrawDev)

// Map the underlying CIDLib graphics engine bitmap mode to the RIVA protocol vresion
static tWebRIVA::EBmpModes eMapBmpMode(const tCIDGraphDev::EBmpModes eMode)
{
    tWebRIVA::EBmpModes eRet;
    switch(eMode)
    {
        case tCIDGraphDev::EBmpModes::DstInvert :
            eRet = tWebRIVA::EBmpModes::DstInvert;
            break;

        case tCIDGraphDev::EBmpModes::PatCopy :
            eRet = tWebRIVA::EBmpModes::PatCopy;
            break;

        case tCIDGraphDev::EBmpModes::PatInvert :
            eRet = tWebRIVA::EBmpModes::PatInvert;
            break;

        case tCIDGraphDev::EBmpModes::SrcAnd :
            eRet = tWebRIVA::EBmpModes::SrcAnd;
            break;

        case tCIDGraphDev::EBmpModes::SrcCopy :
            eRet = tWebRIVA::EBmpModes::SrcCopy;
            break;

        case tCIDGraphDev::EBmpModes::SrcErase :
            eRet = tWebRIVA::EBmpModes::SrcErase;
            break;

        case tCIDGraphDev::EBmpModes::SrcPaint :
            eRet = tWebRIVA::EBmpModes::SrcPaint;
            break;

        default :
            eRet = tWebRIVA::EBmpModes::SrcCopy;
            break;
    };
    return eRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TWebRIVAGraphDev
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWebRIVAGraphDev: Constructors and Destructor
// ---------------------------------------------------------------------------
TWebRIVAGraphDev::TWebRIVAGraphDev( const   TSize&                  szMax
                                    ,       TWebRIVAWIntf* const    pwrwiOwner) :
    m_c4StartEndDepth(0)
    , m_gdevShadow(szMax, tCIDImage::EBitDepths::Eight, tCIDImage::EPixFmts::TrueAlpha)
    , m_pwrwiOwner(pwrwiOwner)
    , m_szMax(szMax)
{
}

TWebRIVAGraphDev::~TWebRIVAGraphDev()
{
}


// ---------------------------------------------------------------------------
//  TWebRIVAGraphDev: Public, inherited methods
// ---------------------------------------------------------------------------

TArea TWebRIVAGraphDev::areaBounds(const tCIDLib::TBoolean bReset) const
{
    return m_gdevShadow.areaBounds(bReset);
}


TArea
TWebRIVAGraphDev::areaMLText(const  TString&        strText
                            , const tCIDLib::TCard4 c4Width
                            , const tCIDLib::TCard4 c4Start
                            , const tCIDLib::TCard4 c4Len)
{
    return m_gdevShadow.areaMLText(strText, c4Width, c4Start, c4Len);
}


TArea
TWebRIVAGraphDev::areaString(const  TString&            strText
                            , const tCIDLib::TBoolean   bSkipMnemonic) const
{
    return m_gdevShadow.areaString(strText, bSkipMnemonic);
}

TArea
TWebRIVAGraphDev::areaString(const  TString&            strText
                            , const tCIDLib::TCard4     c4Start
                            , const tCIDLib::TCard4     c4Len
                            , const tCIDLib::TBoolean   bSkipMnemonic) const
{
    return m_gdevShadow.areaString(strText, c4Start, c4Len, bSkipMnemonic);
}


//
//  All of these alpha blits basically come down to one scenario, which is
//  a source area and destination area, and that's the only one variation
//  that the remote protocol supports. So, these all turn around can call the
//  first one after figuring out what both areas are (based on the image
//  size.)
//
//  Note that we cannot support the last one, which provides a source
//  graphics device to blit from. There's no way to figure out what image
//  is to be drawn, unlike the others where we get a bitmap and it contains
//  user info that we can pass to the client to let him download the image
//  and display it. But it would only be used in things that we don't support
//  remotely anyway, such as popouts or fancy cover art browser effects.
//
//  The wait retrace parameter is not used here, since there's no scenario
//  supportable in a remote viewing sort of way that would use it.
//
tCIDLib::TVoid
TWebRIVAGraphDev::AlphaBlit(const   TBitmap&            bmpSrc
                            , const TArea&              areaSrc
                            , const TArea&              areaDest
                            , const tCIDLib::TCard1     c1ConstAlpha
                            , const tCIDLib::TBoolean   bSrcAlpha
                            , const tCIDLib::TBoolean)
{
    //
    //  See if it has any user data. It might not if no real image has been
    //  set yet on the widget.
    //
    const TString& strUserData = bmpSrc.strUserData();
    if (strUserData.bIsEmpty())
        return;

    tCIDLib::TCard1 c1Flags = 0;
    if (bSrcAlpha)
        c1Flags |= kWebRIVA::c1BltFlag_SrcAlpha;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpSrc, strFullPath);

    // We have to use the src/tar version which takes source/dest areas
    const tCIDLib::TCard4 c4SerialNum = bmpSrc.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatAlphaBlitST
    (
        c4Bytes, strFullPath, areaSrc, areaDest, c1Flags, c1ConstAlpha, c4SerialNum
    );

    // We have to call the image oriented sender here!
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpSrc, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::AlphaBlit(const   TBitmap&            bmpSrc
                            , const TArea&              areaDest
                            , const tCIDLib::TCard1     c1ConstAlpha
                            , const tCIDLib::TBoolean   bSrcAlpha
                            , const tCIDLib::TBoolean)
{
    //
    //  This one is the whole source image area to the dest area, so create
    //  a source area that represents the image (at it's origin.)
    //
    TArea areaSrc(TPoint::pntOrigin, bmpSrc.szBitmap());
    AlphaBlit(bmpSrc, areaSrc, areaDest, c1ConstAlpha, bSrcAlpha);
}

//
//  We handle this one separately, because it can be done more efficiently. THe
//  others just case the main version above.
tCIDLib::TVoid
TWebRIVAGraphDev::AlphaBlit(const   TBitmap&            bmpSrc
                            , const TPoint&             pntDest
                            , const tCIDLib::TCard1     c1ConstAlpha
                            , const tCIDLib::TBoolean   bSrcAlpha
                            , const tCIDLib::TBoolean)
{

    //
    //  See if it has any user data. It might not if no real image has been
    //  set yet on the widget.
    //
    const TString& strUserData = bmpSrc.strUserData();
    if (strUserData.bIsEmpty())
        return;

    tCIDLib::TCard1 c1Flags = 0;
    if (bSrcAlpha)
        c1Flags |= kWebRIVA::c1BltFlag_SrcAlpha;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpSrc, strFullPath);

    // Here we can use the regular version, that just sends an origin point
    const tCIDLib::TCard4 c4SerialNum = bmpSrc.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatAlphaBlit
    (
        c4Bytes, strFullPath, pntDest, c1Flags, c1ConstAlpha, c4SerialNum
    );

    //
    //  We have to call the image oriented sender here! We pass the image's own size
    //  as the target size.
    //
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpSrc, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::AlphaBlit(const   TBitmap&            bmpSrc
                            , const TArea&              areaSource
                            , const TPoint&             pntDestUL
                            , const tCIDLib::TCard1     c1ConstAlpha
                            , const tCIDLib::TBoolean   bSrcAlpha
                            , const tCIDLib::TBoolean   bWaitRetrace)
{
    //
    //  This one draws the indicate area of the source to the dest point
    //  (at the same size, i.e. no stretching.) So we create dest area
    //  with the dest point origin and the source area size.
    //
    TArea areaDest(pntDestUL, areaSource.szArea());
    AlphaBlit(bmpSrc, areaSource, areaDest, c1ConstAlpha, bSrcAlpha);
}

tCIDLib::TVoid
TWebRIVAGraphDev::AlphaBlit(const   TGraphDrawDev&
                            , const TArea&
                            , const TArea&
                            , const tCIDLib::TCard1
                            , const tCIDLib::TBoolean
                            , const tCIDLib::TBoolean)
{
    // We don't use this in the interface system
    CIDAssert2(L"Alpha blit from a src device is not supported");
}


tCIDLib::TBoolean TWebRIVAGraphDev::bBoundsEnabled() const
{
    return m_gdevShadow.bBoundsEnabled();
}


tCIDLib::TBoolean TWebRIVAGraphDev::bCapConstAlpha() const
{
    return m_gdevShadow.bCapConstAlpha();
}


tCIDLib::TBoolean TWebRIVAGraphDev::bCapSrcAlpha() const
{
    return m_gdevShadow.bCapSrcAlpha();
}


tCIDLib::TBoolean TWebRIVAGraphDev::bIsPaletteBased() const
{
    return m_gdevShadow.bIsPaletteBased();
}


tCIDLib::TVoid TWebRIVAGraphDev::BeginPath()
{
    m_gdevShadow.BeginPath();
}


tCIDLib::TCard4 TWebRIVAGraphDev::c4BitsPerPixel() const
{
    return m_gdevShadow.c4BitsPerPixel();
}


tCIDLib::TCard4 TWebRIVAGraphDev::c4Extra() const
{
    return m_gdevShadow.c4Extra();
}

tCIDLib::TCard4 TWebRIVAGraphDev::c4Extra(const tCIDLib::TCard4 c4ToSet)
{
    return m_gdevShadow.c4Extra(c4ToSet);
}


tCIDLib::TCard4 TWebRIVAGraphDev::c4HPelsPerInch() const
{
    return m_gdevShadow.c4HPelsPerInch();
}

tCIDLib::TCard4 TWebRIVAGraphDev::c4VPelsPerInch() const
{
    return m_gdevShadow.c4VPelsPerInch();
}


tCIDLib::TVoid TWebRIVAGraphDev::ClearClipRegion()
{
    CIDAssert2(L"Clearing the clip region is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::CopyBits(         TGraphDrawDev&          gdevSrc
                            , const TPoint&                 pntSrc
                            , const TArea&                  areaTarget
                            , const tCIDGraphDev::EBmpModes eMode
                            , const tCIDLib::TBoolean       bWaitRetrace)
{
    CIDAssert2(L"Device based bit copying is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::CopyBits(         TGraphDrawDev&          gdevSrc
                            , const TArea&                  areaSrc
                            , const TArea&                  areaTarget
                            , const tCIDGraphDev::EBmpModes eMode
                            , const tCIDLib::TBoolean       bWaitRetrace)
{
    CIDAssert2(L"Device based bit copying is not supported");
}


tCIDLib::TVoid TWebRIVAGraphDev::DisableBoundsAccumulation()
{
    CIDAssert2(L"Bounds accumulation is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawArc(  const   TPoint&         pntCenter
                            , const tCIDLib::TCard4 c4Radius
                            , const tCIDLib::TCard4 c4StartDegree
                            , const tCIDLib::TCard4 c4EndDegree)
{
    CIDAssert2(L"Arc drawing is not supported");
}


//
//  Most of these are converted to calls to the most generic one that takes
//  the source and destination area, but we can be more efficient by handling
//  the basic 'draw it full size at this target point' one separately.
//
tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmap(const  TBitmap&                bmpToDraw
                            , const TPoint&                 pntDestUL
                            , const tCIDGraphDev::EBmpModes eMode)
{
    //
    //  See if it has any user data. It might not if no real image has
    //  been set yet on the widget.
    //
    const TString& strUserData = bmpToDraw.strUserData();
    if (strUserData.bIsEmpty())
        return;

    // Map the mode to our mode
    tWebRIVA::EBmpModes eRIVAMode;
    switch(eMode)
    {
        case tCIDGraphDev::EBmpModes::DstInvert :
            eRIVAMode = tWebRIVA::EBmpModes::DstInvert;
            break;

        case tCIDGraphDev::EBmpModes::PatCopy :
            eRIVAMode = tWebRIVA::EBmpModes::PatCopy;
            break;

        case tCIDGraphDev::EBmpModes::PatInvert :
            eRIVAMode = tWebRIVA::EBmpModes::PatInvert;
            break;

        case tCIDGraphDev::EBmpModes::SrcAnd :
            eRIVAMode = tWebRIVA::EBmpModes::SrcAnd;
            break;

        case tCIDGraphDev::EBmpModes::SrcCopy :
            eRIVAMode = tWebRIVA::EBmpModes::SrcCopy;
            break;

        case tCIDGraphDev::EBmpModes::SrcErase :
            eRIVAMode = tWebRIVA::EBmpModes::SrcErase;
            break;

        case tCIDGraphDev::EBmpModes::SrcPaint :
            eRIVAMode = tWebRIVA::EBmpModes::SrcPaint;

        // Unsupported, just set it to source copy
        default :
            eRIVAMode = tWebRIVA::EBmpModes::SrcCopy;
            break;
    };

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpToDraw, strFullPath);

    // Here we can use the regular version, that just sends an origin point
    const tCIDLib::TCard4 c4SerialNum = bmpToDraw.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawBitmap
    (
        c4Bytes, strFullPath, pntDestUL, eRIVAMode, c4SerialNum
    );

    //
    //  We have to call the image oriented sender here! We pass the image's own size
    //  as the target size.
    //
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpToDraw, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmap(const  TBitmap&                bmpToDraw
                            , const TPoint&                 pntSrcUL
                            , const TPoint&                 pntDestUL
                            , const tCIDGraphDev::EBmpModes eMode)
{
    TArea areaSrc(pntSrcUL, bmpToDraw.szBitmap());
    TArea areaDest(pntDestUL, areaSrc.szArea());
    DrawBitmap(bmpToDraw, areaSrc, areaDest, eMode);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmap(const  TBitmap&                bmpToDraw
                            , const TArea&                  areaDest
                            , const tCIDGraphDev::EBmpModes eMode)
{
    TArea areaSrc(TPoint::pntOrigin, bmpToDraw.szBitmap());
    DrawBitmap(bmpToDraw, areaSrc, areaDest, eMode);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmap(const  TBitmap&                bmpToDraw
                            , const TArea&                  areaSrc
                            , const TArea&                  areaTar
                            , const tCIDGraphDev::EBmpModes eMode)
{
    //
    //  See if it has any user data. It might not if no real image has
    //  been set yet on the widget.
    //
    const TString& strUserData = bmpToDraw.strUserData();
    if (strUserData.bIsEmpty())
        return;

    // Map the mode to our mode
    tWebRIVA::EBmpModes eRIVAMode;
    switch(eMode)
    {
        case tCIDGraphDev::EBmpModes::DstInvert :
            eRIVAMode = tWebRIVA::EBmpModes::DstInvert;
            break;

        case tCIDGraphDev::EBmpModes::PatCopy :
            eRIVAMode = tWebRIVA::EBmpModes::PatCopy;
            break;

        case tCIDGraphDev::EBmpModes::PatInvert :
            eRIVAMode = tWebRIVA::EBmpModes::PatInvert;
            break;

        case tCIDGraphDev::EBmpModes::SrcAnd :
            eRIVAMode = tWebRIVA::EBmpModes::SrcAnd;
            break;

        case tCIDGraphDev::EBmpModes::SrcCopy :
            eRIVAMode = tWebRIVA::EBmpModes::SrcCopy;
            break;

        case tCIDGraphDev::EBmpModes::SrcErase :
            eRIVAMode = tWebRIVA::EBmpModes::SrcErase;
            break;

        case tCIDGraphDev::EBmpModes::SrcPaint :
            eRIVAMode = tWebRIVA::EBmpModes::SrcPaint;

        // Unsupported, just set it to source copy
        default :
            eRIVAMode = tWebRIVA::EBmpModes::SrcCopy;
            break;
    };

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpToDraw, strFullPath);

    // Here we we have to use the source/target area version
    const tCIDLib::TCard4 c4SerialNum = bmpToDraw.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawBitmapST
    (
        c4Bytes, strFullPath, areaSrc, areaTar, eRIVAMode, c4SerialNum
    );

    // We have to call the image oriented sender here
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpToDraw, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmap(const  TBitmap&                bmpToDraw
                            , const TArea&                  areaSrc
                            , const TPoint&                 pntDestUL
                            , const tCIDGraphDev::EBmpModes eMode)
{
    TArea areaDest(pntDestUL, areaSrc.szArea());
    DrawBitmap(bmpToDraw, areaSrc, areaDest, eMode);
}


//
//  These are for drawing color based transparency images. So the image
//  has to be drawn in a sprite fashion, masking it into the background.
//
//  We still just use the draw bitmap command, since the client will have
//  cached the image and it will be marked already as a color based
//  image or not. If it is, he will pre-mask it and draw it that way.
//
tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&
                                    , const TArea&
                                    , const TArea&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TBoolean)
{
    CIDAssert2(L"This form of masked bitmap draw is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&
                                    , const TArea&
                                    , const TPoint&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TBoolean)
{
    CIDAssert2(L"This form of masked bitmap draw is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&
                                    , const TPoint&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TBoolean)
{
    CIDAssert2(L"This form of masked bitmap draw is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&            bmpToDraw
                                    , const TBitmap&            bmpMask
                                    , const TArea&              areaSrc
                                    , const TArea&              areaDest
                                    , const tCIDLib::TCard4     c4TransClr
                                    , const tCIDLib::TBoolean   bForceAlpha)
{
    //
    //  See if it has any user data. It might not if no real image has
    //  been set yet on the widget.
    //
    const TString& strUserData = bmpToDraw.strUserData();
    if (strUserData.bIsEmpty())
        return;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpToDraw, strFullPath);

    //
    //  The client has no means to efficiently create a color masked image, so we will
    //  do it here and send him the already masked image. So create a bitmap of the
    //  appropriate format. Query the pixel data from the source, convert it to
    //  RGBA format, and create the new bitmap from that.
    //
    //  It's already been masked which will leave the transparency color black, so
    //  we pass zero, not the passed value.
    //
    TPixelArray pixaData;
    TClrPalette palDummy;
    bmpToDraw.QueryImgData(*this, pixaData, palDummy);
    pixaData.bRGBToRGBA(0, kCIDLib::False, TRGBClr(0, 0, 0));
    TBitmap bmpTrans(*this, pixaData, palDummy);

    // Set the original image's serial number on our temp one so it gets sent
    bmpTrans.c4SerialNum(bmpToDraw.c4SerialNum());

    //
    //  In this case we have to see the separate src/tar areas since it may be scaled.
    //  These are always source copy mode
    //
    //  Note we convert this to an alpha blit, since it's now alpha based
    //
    const tCIDLib::TCard4 c4SerialNum = bmpTrans.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatAlphaBlitST
    (
        c4Bytes
        , strFullPath
        , areaSrc
        , areaDest
        , kWebRIVA::c1BltFlag_SrcAlpha
        , 0xFF
        , c4SerialNum
    );

    // We have to call the image oriented sender here
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpTrans, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&            bmpToDraw
                                    , const TBitmap&            bmpMask
                                    , const TArea&              areaSrc
                                    , const TPoint&             pntDestUL
                                    , const tCIDLib::TCard4     c4TransClr
                                    , const tCIDLib::TBoolean   bForceAlpha)
{
    // Just set up the target area and call the other version
    TArea areaDest(pntDestUL, areaSrc.szArea());
    DrawBitmapMasked(bmpToDraw, bmpMask, areaSrc, areaDest, c4TransClr, bForceAlpha);
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBitmapMasked( const   TBitmap&            bmpToDraw
                                    , const TBitmap&            bmpMask
                                    , const TPoint&             pntDestUL
                                    , const tCIDLib::TCard4     c4TransClr
                                    , const tCIDLib::TBoolean   bForceAlpha)
{
    const TString& strUserData = bmpToDraw.strUserData();
    if (strUserData.bIsEmpty())
        return;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpToDraw, strFullPath);

    //
    //  The client has no means to efficiently create a color masked image, so we will
    //  do it here and send him the already masked image. So create a bitmap of the
    //  appropriate format. Query the pixel data from the source, convert it to
    //  RGBA format, and create the new bitmap from that.
    //
    //  It's already been masked which will leave the transparency color black, so
    //  we pass zero, not the passed value.
    //
    TPixelArray pixaData;
    TClrPalette palDummy;
    bmpToDraw.QueryImgData(*this, pixaData, palDummy);
    pixaData.bRGBToRGBA(0, kCIDLib::False, TRGBClr(0, 0, 0));
    TBitmap bmpTrans(*this, pixaData, palDummy);

    // Set the original image's serial number on our temp one so it gets sent
    bmpTrans.c4SerialNum(bmpToDraw.c4SerialNum());

    //
    //  We can do the version that just draws at an origin point in the natural size
    //  These are always source copy mode.
    //
    //  NOTE that we convert it to an alpha operation now.
    //
    const tCIDLib::TCard4 c4SerialNum = bmpTrans.c4SerialNum();
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatAlphaBlit
    (
        c4Bytes
        , strFullPath
        , pntDestUL
        , kWebRIVA::c1BltFlag_SrcAlpha
        , 0xFF
        , c4SerialNum
    );

    // We have to call the image oriented sender here, sending the transparent version
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpTrans, pmbufNew, c4Bytes);
}


// Not currently supported
tCIDLib::TVoid
TWebRIVAGraphDev::DrawBox(  const   TPoint&
                            , const TPoint&
                            , const tCIDLib::TCard4)
{
    CIDAssert2(L"Box drawing is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawBox(  const   TArea&          areaBox
                            , const tCIDLib::TCard4 c4Rounding)
{
    CIDAssert2(L"Box drawing is not supported");
}


// Not currently supported
tCIDLib::TVoid
TWebRIVAGraphDev::DrawCircle(const  tCIDLib::TCard4
                            , const TPoint&)
{
    CIDAssert2(L"Circle drawing is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawFilledCircle( const   tCIDLib::TCard4
                                    , const TPoint&)
{
    CIDAssert2(L"Filled circle drawing is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawFocusArea(const TArea& areaToDraw)
{
    CIDAssert2(L"Focus area drawing is not supported");
}


// Not currently supported
tCIDLib::TVoid
TWebRIVAGraphDev::DrawGradientCircle(const  tCIDLib::TCard4
                                    , const TPoint&
                                    , const TRGBClr&
                                    , const TRGBClr&)
{
    CIDAssert2(L"Gradient circle drawing is not supported");
}


// These aren't supported in the remote viewer
tCIDLib::TVoid
TWebRIVAGraphDev::DrawIcon(const TIcon&, const TPoint&)
{
    CIDAssert2(L"Icon drawing is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawIcon(const TIcon&, const TPoint&, const TSize&)
{
    CIDAssert2(L"Icon drawing is not supported");
}

tCIDLib::TVoid TWebRIVAGraphDev::DrawIcon(const TIcon&, const TArea&)
{
    CIDAssert2(L"Icon drawing is not supported");
}


// These aren't supported in the remote viewer
tCIDLib::TVoid
TWebRIVAGraphDev::DrawIconPart( const   TIcon&
                                , const TPoint&
                                , const tCIDGraphDev::EBmpModes
                                , const tCIDLib::TBoolean)
{
    CIDAssert2(L"Icon part drawing is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawIconPart( const   TIcon&
                                , const TPoint&
                                , const TSize&
                                , const tCIDGraphDev::EBmpModes
                                , const tCIDLib::TBoolean)
{
    CIDAssert2(L"Icon part drawing is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawIconPart( const   TIcon&
                                , const TArea&
                                , const tCIDGraphDev::EBmpModes
                                , const tCIDLib::TBoolean)
{
    CIDAssert2(L"Icon part drawing is not supported");
}


// Draw a line from one point to another, using a given color
tCIDLib::TVoid
TWebRIVAGraphDev::DrawLine( const   TPoint&     pntFrom
                            , const TPoint&     pntTo
                            , const TRGBClr&    rgbClr)
{
    // Draw to our shadow
    m_gdevShadow.DrawLine(pntFrom, pntTo, rgbClr);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawLine(c4Bytes, pntFrom, pntTo, rgbClr);
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// Draw a text string, wrapped within an area
tCIDLib::TVoid
TWebRIVAGraphDev::DrawMText(const   TString&            strText
                            , const TArea&              areaFormat
                            , const tCIDLib::EHJustify  eHJustify
                            , const tCIDLib::EVJustify  eVJustify
                            , const tCIDLib::TBoolean   bWordBreak)
{
    // Optimize for an empty string
    if (strText.bIsEmpty())
        return;

    tCIDLib::TCard4 c4Flags = kWebRIVA::c4TextFlag_None;
    if (bWordBreak)
        c4Flags |= kWebRIVA::c4MTextFlag_WordBreak;

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawMultiText
    (
        c4Bytes
        , strText
        , areaFormat
        , tWebRIVA::EHJustifys(eHJustify)
        , tWebRIVA::EVJustifys(eVJustify)
        , c4Flags
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawPBar( const   TBitmap&                bmpMask
                            , const tCIDLib::TFloat4        f4Percent
                            , const TArea&                  areaSrc
                            , const TArea&                  areaTar
                            , const tCIDGraphDev::EBltTypes eBlt
                            , const tCIDLib::EDirs          eDir
                            , const TRGBClr&                rgbStart
                            , const TRGBClr&                rgbEnd
                            , const TRGBClr&                rgbFill
                            , const tCIDLib::TCard1         c1Opacity)
{
    const TString& strUserData = bmpMask.strUserData();
    if (strUserData.bIsEmpty())
        return;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpMask, strFullPath);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawPBar
    (
        c4Bytes
        , strFullPath
        , c1Opacity
        , f4Percent
        , areaSrc
        , areaTar
        , tWebRIVA::EDirs(eDir)
        , rgbStart
        , rgbEnd
        , rgbFill
    );
    m_pwrwiOwner->SendImgMsg(strFullPath, bmpMask, pmbufNew, c4Bytes);
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawPolyLine( const   TPointArray&
                                , const tCIDLib::TCard4)
{
    CIDAssert2(L"Polyline drawing is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawPolyLine( const   TPointArray&
                                , const TRGBClr&
                                , const tCIDLib::TCard4)
{
    CIDAssert2(L"Polyline drawing is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawPolySegments( const   TPointArray&
                                    , const tCIDLib::TCard4)
{
    CIDAssert2(L"Polyline drawing is not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::DrawPolySegments( const   TPointArray&
                                    , const TRGBClr&
                                    , const tCIDLib::TCard4)
{
    CIDAssert2(L"Polyline drawing is not supported");
}


// We can't support this
tCIDLib::TVoid TWebRIVAGraphDev::DrawString(const TString&, const TPoint&)
{
   CIDAssert2(L"DrawString (point align) is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::DrawString(const TString&, const tCIDLib::TCard4, const TPoint& pntAlign)
{
   CIDAssert2(L"DrawString (point align, substring) is not supported");
}


// The most general purpose single line text drawing method
tCIDLib::TVoid
TWebRIVAGraphDev::DrawString(const  TString&                strText
                            , const TArea&                  areaFormat
                            , const tCIDLib::EHJustify      eHJustify
                            , const tCIDLib::EVJustify      eVJustify
                            , const tCIDGraphDev::ETextFmts eFormat
                            , const TRGBClr&                rgbBgnFill)
{
    // Optimize for an empty string
    if (strText.bIsEmpty())
        return;

    // Our flags are the same as the bitmapped enum
    const tCIDLib::TCard4 c4Flags = tCIDLib::TCard4(eFormat);

    // Queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawText
    (
        c4Bytes
        , strText
        , areaFormat
        , tWebRIVA::EHJustifys(eHJustify)
        , tWebRIVA::EVJustifys(eVJustify)
        , rgbBgnFill
        , c4Flags
    );

    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


//
//  A more elaborate single line text drawing method. We reduce it down
//  to the same outgoing structure as the simpler one above by doing some
//  of the work for the remote side.
//
tCIDLib::TVoid
TWebRIVAGraphDev::DrawString(const  TString&                strText
                            , const tCIDLib::TCard4         c4StartInd
                            , const tCIDLib::TCard4         c4Len
                            , const TArea&                  areaFormat
                            , const tCIDLib::EHJustify      eHJustify
                            , const tCIDLib::EVJustify      eVJustify
                            , const tCIDGraphDev::ETextFmts eFormat
                            , const TRGBClr&                rgbBgnFill)
{
    // Optimize for an empty string
    if (strText.bIsEmpty())
        return;

    // Shouldn't happen, but if so, do nothing
    const tCIDLib::TCard4 c4SrcLen = strText.c4Length();
    if (c4StartInd >= c4SrcLen)
        return;

    // Calculate the text chars we'll be sending
    tCIDLib::TCard4 c4ToSend;
    if (c4Len == kCIDLib::c4MaxCard)
    {
        c4ToSend = c4SrcLen - c4StartInd;
    }
     else
    {
        if (c4StartInd + c4Len >= c4SrcLen)
            c4ToSend = c4SrcLen - c4StartInd;
        else
            c4ToSend = c4Len;
    }

    // Our flags are the same as the bitmapped enum
    const tCIDLib::TCard4 c4Flags = tCIDLib::TCard4(eFormat);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = nullptr;
    if (c4ToSend < c4SrcLen)
    {
        TString strTmp(c4ToSend);
        strTmp.CopyInSubStr(strText, c4StartInd, c4ToSend);
        pmbufNew = m_wrtToUse.pmbufFormatDrawText
        (
            c4Bytes
            , strTmp
            , areaFormat
            , tWebRIVA::EHJustifys(eHJustify)
            , tWebRIVA::EVJustifys(eVJustify)
            , rgbBgnFill
            , c4Flags
        );
    }
     else
    {
        pmbufNew = m_wrtToUse.pmbufFormatDrawText
        (
            c4Bytes
            , strText
            , areaFormat
            , tWebRIVA::EHJustifys(eHJustify)
            , tWebRIVA::EVJustifys(eVJustify)
            , rgbBgnFill
            , c4Flags
        );
    }
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


//
//  Draw text using the defined special text effects, justified within a
//  given area.
//
tCIDLib::TVoid
TWebRIVAGraphDev::DrawStringFX( const   TString&                strText
                                , const TArea&                  areaFormat
                                , const tCIDGraphDev::ETextFX  eEffect
                                , const TRGBClr&                rgbClr1
                                , const TRGBClr&                rgbClr2
                                , const tCIDLib::EHJustify      eHJustify
                                , const tCIDLib::EVJustify      eVJustify
                                , const tCIDLib::TBoolean       bNoTextWrap
                                , const TPoint&                 pntOffset
                                ,       TBitmap&                bmpBuffer
                                ,       TBitmap&                bmpMask
                                ,       TPixelArray&            pixaBlur
                                , const tCIDLib::TCard1         c1ConstAlpha)
{
    // Optimize for an empty string
    if (strText.bIsEmpty())
        return;

    tCIDLib::TCard4 c4Flags = 0;
    if (bNoTextWrap)
        c4Flags |= kWebRIVA::c4TextFXFlag_NoWrap;

    // Force the 2nd color (if doing drop shadow or blur, to have alpha
    TRGBClr rgbReal2(rgbClr2);
    if ((eEffect == tCIDGraphDev::ETextFX::DropShadow)
    ||  (eEffect == tCIDGraphDev::ETextFX::GaussianBlur))
    {
        rgbReal2.c1Alpha(0xC0);
    }
     else
    {
        rgbReal2.c1Alpha(0xFF);
    }

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatDrawTextFX
    (
        c4Bytes
        , strText
        , tWebRIVA::ETextFXs(eEffect)
        , areaFormat
        , rgbClr1
        , rgbReal2
        , tWebRIVA::EHJustifys(eHJustify)
        , tWebRIVA::EVJustifys(eVJustify)
        , c4Flags
        , pntOffset
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


tCIDLib::EAdoptOpts TWebRIVAGraphDev::eAdopted() const
{
    return m_gdevShadow.eAdopted();
}


// Get/set a new background mix mode
tCIDGraphDev::EBackMixModes TWebRIVAGraphDev::eBackMixMode() const
{
    return m_gdevShadow.eBackMixMode();
}

tCIDGraphDev::EBackMixModes
TWebRIVAGraphDev::eBackMixMode(const tCIDGraphDev::EBackMixModes eToSet)
{
    // Set it on our shadow
    m_gdevShadow.eBackMixMode(eToSet);

    // Queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatSetBackMixMode(c4Bytes, tWebRIVA::EBackMixModes(eToSet));
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
    return eToSet;
}


// Get/set a new foreground mix mode
tCIDGraphDev::EMixModes TWebRIVAGraphDev::eMixMode() const
{
    return m_gdevShadow.eMixMode();
}

tCIDGraphDev::EMixModes
TWebRIVAGraphDev::eMixMode(const tCIDGraphDev::EMixModes eToSet)
{
    // Set it on our shadow
    m_gdevShadow.eMixMode(eToSet);

    // Queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatSetMixMode(c4Bytes, tWebRIVA::EMixModes(eToSet));
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
    return eToSet;
}


// Fill an area with a color. The points are assumed to be non-inclusive
tCIDLib::TVoid
TWebRIVAGraphDev::Fill(const    TPoint&     pntFrom
                        , const TPoint&     pntTo
                        , const TRGBClr&    rgbToUse)
{
    // We only have a single msg that takes an area
    TArea areaFill(pntFrom, pntTo);

    // Queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatFillArea
    (
        c4Bytes, 0, areaFill, rgbToUse
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::Fill(const TArea& areaFill, const TRGBClr& rgbToUse)
{
    // Queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatFillArea
    (
        c4Bytes, 0, areaFill, rgbToUse
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid TWebRIVAGraphDev::FillPath()
{
    CIDAssert2(L"Path filling is not supported");
}


// These are all converted to the most generic version
tCIDLib::TVoid
TWebRIVAGraphDev::FillWithBmpPattern(const  TArea&                  areaToFill
                                    , const TBitmap&                bmpToUse
                                    , const tCIDGraphDev::EBmpModes eMode)
{
    FillWithBmpPattern(areaToFill, bmpToUse, eMode, TPoint::pntOrigin);
}

tCIDLib::TVoid
TWebRIVAGraphDev::FillWithBmpPattern(const  TPoint&                 pntFrom
                                    , const TPoint&                 pntTo
                                    , const TBitmap&                bmpToUse
                                    , const tCIDGraphDev::EBmpModes eMode)
{
    TArea areaToFill(pntFrom, pntTo);
    FillWithBmpPattern(areaToFill, bmpToUse, eMode, TPoint::pntOrigin);
}

tCIDLib::TVoid
TWebRIVAGraphDev::FillWithBmpPattern(const  TArea&                  areaToFill
                                    , const TBitmap&                bmpToUse
                                    , const tCIDGraphDev::EBmpModes eMode
                                    , const TPoint&                 pntPatOrg)
{
    //
    //  See if it has any user data. It might not if no real image has been
    //  set yet on the widget.
    //
    if (bmpToUse.strUserData().bIsEmpty())
        return;

    // Build up the full path
    TString strFullPath;
    BuildImgPath(bmpToUse, strFullPath);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatFillWithBmp
    (
        c4Bytes
        , strFullPath
        , areaToFill
        , pntPatOrg
        , eMapBmpMode(eMode)
        , bmpToUse.c4SerialNum()
    );

    m_pwrwiOwner->SendImgMsg(strFullPath, bmpToUse, pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::FillWithBmpPattern(const  TPoint&                 pntFrom
                                    , const TPoint&                 pntTo
                                    , const TBitmap&                bmpToUse
                                    , const tCIDGraphDev::EBmpModes eMode
                                    , const TPoint&                 pntPatOrg)
{
    TArea areaToFill(pntFrom, pntTo);
    FillWithBmpPattern(areaToFill, bmpToUse, eMode, pntPatOrg);
}


// Not currently supported
tCIDLib::TVoid
TWebRIVAGraphDev::Gradient3DFill(const  TArea&      areaFill
                                , const TRGBClr&    rgbLight
                                , const TRGBClr&    rgbDark)
{
    CIDAssert2(L"Gradient 3D fills are not supported");
}


tCIDLib::TVoid
TWebRIVAGraphDev::Gradient3DFill2(const TArea& areaFill, const TRGBClr& rgbFill)
{
}


// Do a gradient fill of an area in the indicated direction
tCIDLib::TVoid
TWebRIVAGraphDev::GradientFill( const   TArea&                  areaFill
                                , const TRGBClr&                rgbLeft
                                , const TRGBClr&                rgbRight
                                , const tCIDGraphDev::EGradDirs eDir)
{
    //
    //  We don't use the rounding through it is in the protocol. The IV engine will just
    //  set a clip path and do a fill if it wants a rounded fill.
    //
    //  <TBD> At some point add a rounding option to the the graphics device interface
    //  for more efficiency.
    //
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatGradientFill
    (
        c4Bytes, 0, areaFill, rgbLeft, rgbRight, tWebRIVA::EGradDirs(eDir)
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::GradientFill( const   TPoint&                 pntUL
                                , const TPoint&                 pntLR
                                , const TRGBClr&                rgbLeft
                                , const TRGBClr&                rgbRight
                                , const tCIDGraphDev::EGradDirs eDir)
{
    //
    //  Create an area from the two points and call the other version. We assume the
    //  points are non-inclusive.
    //
    TArea areaFill(pntUL, pntLR);
    GradientFill(areaFill, rgbLeft, rgbRight, eDir);
}


tCIDGraphDev::TDeviceHandle TWebRIVAGraphDev::hdevThis() const
{
    return m_gdevShadow.hdevThis();
}


// Set a new font on the device and return the old one
tCIDGraphDev::TFontHandle TWebRIVAGraphDev::hfontSetFont(const TGUIFont& gfontToSet)
{
    // Set it on the shadow and save the return for our return
    tCIDGraphDev::TFontHandle hfontRet = m_gdevShadow.hfontSetFont(gfontToSet);
    const TFontSelAttrs& fselSel = gfontToSet.fselCurrent();

    tCIDLib::TCard1 c1Flags = 0;
    if (fselSel.bItalic())
        c1Flags |= kWebRIVA::c1FontFlag_Italic;
    if (fselSel.eWeight() == tCIDGraphDev::EFontWeights::Bold)
        c1Flags |= kWebRIVA::c1FontFlag_Bold;

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPushFont
    (
        c4Bytes, fselSel.strFaceName(), c1Flags, tCIDLib::TCard1(fselSel.c4Height())
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);

    return hfontRet;
}


//
//  We don't support setting pens here. Only methods that explicitly
//  indicate a colors and line types are used.
//
tCIDGraphDev::TPenHandle TWebRIVAGraphDev::hpenSetPen(TGUIPen&)
{
    // We don't use this in the interface system
    CIDAssert2(L"Setting pens is not supported");
    return 0;
}


// Set a new area based clip region, and return the old one
tCIDGraphDev::TRegionHandle
TWebRIVAGraphDev::hrgnSetClipArea(  const   tCIDGraphDev::EClipModes    eMode
                                    , const TArea&                      areaToSet
                                    , const tCIDLib::TCard4             c4Rounding)
{
    // We can only support a couple modes, which should be all that the IV engine uses
    tWebRIVA::EClipModes eRIVAMode;
    if (eMode == tCIDGraphDev::EClipModes::And)
        eRIVAMode = tWebRIVA::EClipModes::And;
    else if (eMode == tCIDGraphDev::EClipModes::Copy)
        eRIVAMode = tWebRIVA::EClipModes::Copy;
    else
    {
        CIDAssert2(L"Got an unusupported clip mode");
    }

    // Pass it to our shadow and keep the handle for return
    tCIDGraphDev::TRegionHandle hrgnRet = m_gdevShadow.hrgnSetClipArea
    (
        eMode, areaToSet, c4Rounding
    );

    // And queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPushClipArea
    (
        c4Bytes, eRIVAMode, areaToSet, tCIDLib::TCard1(c4Rounding)
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);

    return hrgnRet;
}


// We turn this into the same thing as set clip area above
tCIDGraphDev::TRegionHandle
TWebRIVAGraphDev::hrgnSetClipRegion(const   tCIDGraphDev::EClipModes    eMode
                                    , const TGUIRegion&                 rgnToSet)
{
    // We can only support a couple modes, which should be all that the IV engine uses
    tWebRIVA::EClipModes eRIVAMode;
    if (eMode == tCIDGraphDev::EClipModes::And)
        eRIVAMode = tWebRIVA::EClipModes::And;
    else if (eMode == tCIDGraphDev::EClipModes::Copy)
        eRIVAMode = tWebRIVA::EClipModes::Copy;
    else
    {
        CIDAssert2(L"Got an unusupported clip mode");
    }

    // Pass it to our shadow and keep the handle for return
    tCIDGraphDev::TRegionHandle hrgnRet = m_gdevShadow.hrgnSetClipRegion(eMode, rgnToSet);


    // And queue it up
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPushClipArea
    (
        c4Bytes, eRIVAMode, rgnToSet.areaBounds(), tCIDLib::TCard1(rgnToSet.c4Rounding())
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);


    return hrgnRet;

}


//
//  These all get translated to the most generic version that takes the
//  source and target areas, and it is the only one queued up.
//
//  In our case, we don't need the passed transparency color. The client
//  will download the image and that will tell him if there's any color
//  based transparency.
//
tCIDLib::TVoid
TWebRIVAGraphDev::MaskedAlphaBlit(  const   TBitmap&            bmpSrc
                                    , const TBitmap&            bmpMask
                                    , const TArea&              areaSrc
                                    , const TPoint&             pntTarget
                                    , const tCIDLib::TCard1     c1Alpha
                                    , const tCIDLib::TBoolean   bSrcAlpha
                                    , const tCIDLib::TCard4     c4TransClr)
{
    // Build up the target area and call the other version
    const TArea areaTar(pntTarget, areaSrc.szArea());
    MaskedAlphaBlit(bmpSrc, bmpMask, areaSrc, areaTar, c1Alpha, bSrcAlpha, c4TransClr);
}

tCIDLib::TVoid
TWebRIVAGraphDev::MaskedAlphaBlit(  const   TBitmap&            bmpSrc
                                    , const TBitmap&            bmpMask
                                    , const TPoint&             pntTarget
                                    , const tCIDLib::TCard1     c1Alpha
                                    , const tCIDLib::TBoolean   bSrcAlpha
                                    , const tCIDLib::TCard4     c4TransClr)
{
    // Build up the source and target area and call the other version
    const TArea areaSrc(TPoint::pntOrigin, bmpSrc.szBitmap());
    const TArea areaTar(pntTarget, areaSrc.szArea());
    MaskedAlphaBlit(bmpSrc, bmpMask, areaSrc, areaTar, c1Alpha, bSrcAlpha, c4TransClr);
}

tCIDLib::TVoid
TWebRIVAGraphDev::MaskedAlphaBlit(  const   TBitmap&            bmpSrc
                                    , const TBitmap&            bmpMask
                                    , const TArea&              areaSrc
                                    , const TArea&              areaTar
                                    , const tCIDLib::TCard1     c1Alpha
                                    , const tCIDLib::TBoolean   bSrcAlpha
                                    , const tCIDLib::TCard4     c4TransClr)
{
    //
    //  See if it has any user data. It might not if no real image
    //  has been set yet on the widget.
    //
    const TString& strUserData = bmpSrc.strUserData();
    if (strUserData.bIsEmpty())
        return;

    #if 0
    // Queue up the command
    TRemVPoolJan janBuf(m_pthrOwner, strUserData.c4Length() * 4);
    tCQCRemVComm::TMaskedAlphaBlit* pData
    (
        janBuf.pPrepAsWithText<tCQCRemVComm::TMaskedAlphaBlit>
        (
            kCQCRemVComm::c1OpCode_MaskedAlphaBlit, strUserData
        )
    );

    ToRVArea(areaTar, pData->areaTar);
    ToRVArea(areaSrc, pData->areaSrc);
    pData->c1Flags = bSrcAlpha ? 1 : 0;
    pData->c1ConstAlpha = c1Alpha;
    pData->c4ImgSerial = bmpSrc.c4SerialNum();

    janBuf.QueueUpItem();
    #endif
}

tCIDLib::TVoid
TWebRIVAGraphDev::MaskedAlphaBlit(  const   TGraphDrawDev&      gdevSrc
                                    , const TBitmap&            bmpMask
                                    , const TArea&              areaSrc
                                    , const TArea&              areaTarget
                                    , const tCIDLib::TCard1     c1Alpha
                                    , const tCIDLib::TBoolean   bSrcAlpha
                                    , const tCIDLib::TCard4     c4TransClr)
{
    // We can't support this form of the call
    CIDAssert2(L"This form of masked alph blit is not supported");
}


// Not currently supported
tCIDLib::TVoid TWebRIVAGraphDev::MoveRelative(const TPoint&)
{
    CIDAssert2(L"Drawing position movement not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::MoveRelative( const   tCIDLib::TInt4
                                , const tCIDLib::TInt4)
{
    CIDAssert2(L"Drawing position movement not supported");
}


// Not currently supported
tCIDLib::TVoid TWebRIVAGraphDev::MoveTo(const TPoint& pntNewPos)
{
    CIDAssert2(L"Drawing position movement not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::MoveTo(const  tCIDLib::TInt4, const tCIDLib::TInt4)
{
    CIDAssert2(L"Drawing position movement not supported");
}


TPoint TWebRIVAGraphDev::pntCurPosition() const
{
    CIDAssert2(L"Drawing position movement not supported");
    return TPoint::pntOrigin;
}


// Not currently supported
tCIDLib::TVoid TWebRIVAGraphDev::PatternFill(const TArea&, const tCIDGraphDev::EBmpModes)
{
    CIDAssert2(L"Pattern fills are not supported");
}


// Go back to the previous clip region
tCIDLib::TVoid TWebRIVAGraphDev::PopClipArea(tCIDGraphDev::TRegionHandle hrgnToPutBack)
{
    // Pass it to our shadow
    m_gdevShadow.PopClipArea(hrgnToPutBack);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPopClipArea(c4Bytes);
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// Go back to the previously saved context
tCIDLib::TVoid TWebRIVAGraphDev::PopContext()
{
    // Pass it to our shadow
    m_gdevShadow.PopContext();

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPopContext(c4Bytes);
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// Go back to the previous set font
tCIDLib::TVoid
TWebRIVAGraphDev::PopFont(  const   TGUIFont&                   gfontCur
                            ,       tCIDGraphDev::TFontHandle   hfontToPutBack)
{
    // Pass it to our shadow
    m_gdevShadow.PopFont(gfontCur, hfontToPutBack);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPopFont(c4Bytes);
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// We don't support pens here, so this is not supported
tCIDLib::TVoid TWebRIVAGraphDev::PopPen(TGUIPen&, tCIDGraphDev::TPenHandle)
{
    // We don't use this in the interface system
    CIDAssert2(L"Setting pens is not supported");
}


// Push the current graphics state for later pop
tCIDLib::TVoid TWebRIVAGraphDev::PushContext()
{
    // Pass it to our shadow
    m_gdevShadow.PushContext();

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatPushContext(c4Bytes);
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid
TWebRIVAGraphDev::QueryCharOffsets(const    TString&            strSource
                                    , const tCIDLib::TCard4     c4Index
                                    ,       tCIDLib::TInt4&     i4Offset
                                    ,       tCIDLib::TCard4&    c4Width)
{
    return m_gdevShadow.QueryCharOffsets(strSource, c4Index, i4Offset, c4Width);
}


tCIDLib::TVoid TWebRIVAGraphDev::
QueryCharWidths(const   TString&                        strSource
                ,       TFundVector<tCIDLib::TCard4>&   fcolToFill)
{
    m_gdevShadow.QueryCharWidths(strSource, fcolToFill);
}


tCIDLib::TVoid
TWebRIVAGraphDev::QueryFontMetrics(TFontMetrics& fmtrToFill)
{
    m_gdevShadow.QueryFontMetrics(fmtrToFill);
}


TRGBClr TWebRIVAGraphDev::rgbAt(const TPoint& pntAt) const
{
    return m_gdevShadow.rgbAt(pntAt);
}


TRGBClr TWebRIVAGraphDev::rgbBgnColor() const
{
    return m_gdevShadow.rgbBgnColor();
}


TRGBClr TWebRIVAGraphDev::rgbTextColor() const
{
    return m_gdevShadow.rgbTextColor();
}


tCIDLib::TVoid TWebRIVAGraphDev::ResetBoundsArea()
{
}


TSize TWebRIVAGraphDev::szPixels() const
{
    return m_gdevShadow.szPixels();
}


TSize TWebRIVAGraphDev::szPixelsPerInch() const
{
    return m_gdevShadow.szPixelsPerInch();
}


TSize TWebRIVAGraphDev::szPelsPerInch() const
{
    return m_gdevShadow.szPelsPerInch();
}


// Set a new background color
tCIDLib::TVoid TWebRIVAGraphDev::SetBgnColor(const TRGBClr& rgbToSet)
{
    // Pass it to our shadow
    m_gdevShadow.SetBgnColor(rgbToSet);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatSetColor
    (
        c4Bytes, tWebRIVA::EDrawingColors::Background, rgbToSet
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// Set a new text foreground color
tCIDLib::TVoid TWebRIVAGraphDev::SetTextColor(const TRGBClr& rgbToSet)
{
    // Pass it to our shadow
    m_gdevShadow.SetTextColor(rgbToSet);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatSetColor
    (
        c4Bytes, tWebRIVA::EDrawingColors::Text, rgbToSet
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


// Not suppported here
tCIDLib::TVoid
TWebRIVAGraphDev::SetWorldTranslation(  const   tCIDLib::TFloat4
                                        , const tCIDLib::TFloat4)
{
    CIDAssert2(L"Setting world translation is not supported");
}


//
//  Stroke around an area with a given color and line width, optionally with
//  rounded corners.
//
tCIDLib::TVoid TWebRIVAGraphDev::Stroke(const TArea&)
{
    CIDAssert2(L"Area stroking with an implicit color is not supported");
}

tCIDLib::TVoid
TWebRIVAGraphDev::Stroke(const  TArea&          areaToStroke
                        , const TRGBClr&        rgbToUse
                        , const tCIDLib::TCard4 c4Rounding)
{
    // Pass it to our shadow
    m_gdevShadow.Stroke(areaToStroke, rgbToUse, c4Rounding);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatStrokeArea
    (
        c4Bytes
        , tCIDLib::TCard1(c4Rounding)
        , 1
        , areaToStroke
        , rgbToUse
    );

    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}

tCIDLib::TVoid
TWebRIVAGraphDev::Stroke(const  TGUIRegion&     grgnToStroke
                        , const TRGBClr&        rgbToUse
                        , const tCIDLib::TCard4 c4Width)
{
    // Pass it to our shadow
    m_gdevShadow.Stroke(grgnToStroke, rgbToUse);

    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatStrokeArea
    (
        c4Bytes
        , tCIDLib::TCard1(grgnToStroke.c4Rounding())
        , tCIDLib::TCard1(c4Width)
        , grgnToStroke.areaBounds()
        , rgbToUse
    );
    m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
}


tCIDLib::TVoid TWebRIVAGraphDev::StrokeAndFillPath()
{
    CIDAssert2(L"Path stroke/fill not supported");
}


tCIDLib::TVoid TWebRIVAGraphDev::StrokePath()
{
    CIDAssert2(L"Path stroking not supported");
}


// ---------------------------------------------------------------------------
//  TWebRIVAGraphDev: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  To deal with different resolution versions of an image, we include the resolution
//  in the iamge path. That is the path that the client stores them in his cache as
//  and the form that he expects us to send.
//
//  We expect that the caller has already checked that the image has a path set on it
//  and will just do nothing if not.
//
tCIDLib::TVoid TWebRIVAGraphDev::BuildImgPath(const TBitmap& bmpImg, TString& strToFill)
{
    strToFill = bmpImg.strUserData();
    strToFill.Append(kCIDLib::chForwardSlash);
    const TSize& szImg = bmpImg.szBitmap();
    strToFill.AppendFormatted(szImg.c4Width());
    strToFill.Append(kCIDLib::chLatin_x);
    strToFill.AppendFormatted(szImg.c4Height());
}


//
//  Queues up end drawing commands, which lets the other side know when to
//  complete a drawing round and the area that needs to be update.
//
tCIDLib::TVoid TWebRIVAGraphDev::EndDraw(const TArea& areaUpdate)
{
    // Bump down the counter first. Watch for underflow
    if (m_c4StartEndDepth == 0)
    {
        facCQCWebRIVA().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kWRIVAErrs::errcDbg_StartEndUnderflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }

    m_c4StartEndDepth--;

    // If we are at the 0 depth point, then we can send it
    if (!m_c4StartEndDepth)
    {
        if (facCQCWebRIVA().bLogInfo())
        {
            facCQCWebRIVA().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWRIVACMsgs::midStatus_EndDraw
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , areaUpdate
            );
        }

        tCIDLib::TCard4 c4Bytes;
        TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatEndDraw(c4Bytes, areaUpdate);
        m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
    }
}


//
//  Our real graphics device is returned from the view as the compatible
//  graphics device. It's never drawn to in that case, it's just passed to
//  methods that need a compatible device.
//
const TGraphDrawDev& TWebRIVAGraphDev::gdevReal() const
{
    return m_gdevShadow;
}


//
//  Queues up start drawing commands, which lets the other side know when to allocate
//  and de-allocate drawing resources and prep for a redraw. They can be nested so we
//  handle that and only send on the first start and last end.
//
//  Nested ones should only be targeting areas within the outermost update area, so
//  sending the initial area is sufficiently for the target to know what he needs
//  to update for a given round.
//
tCIDLib::TVoid TWebRIVAGraphDev::StartDraw(const TArea& areaUpdate)
{
    // If we are at the 0 depth point, then we can send it
    if (!m_c4StartEndDepth)
    {
        tCIDLib::TCard4 c4Bytes;
        TMemBuf* pmbufNew = m_wrtToUse.pmbufFormatStartDraw(c4Bytes, areaUpdate);
        m_pwrwiOwner->SendGraphicsMsg(pmbufNew, c4Bytes);
    }

    // Bump the depth up
    m_c4StartEndDepth++;
}

