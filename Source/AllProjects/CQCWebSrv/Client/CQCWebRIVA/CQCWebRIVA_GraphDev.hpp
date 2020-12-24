//
// FILE NAME: CQCWebRIVA_GraphDev.hpp
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
//  This is the header for the CQCWebSrvC_GraphDev.cpp file, which implements our special
//  derivative of the graphic device class that we need to intercept graphical output from
//  the interface engine. This is a modification of the one created for the original RIVA
//  server (which is being superseded by this web server based one.
//
//  We do a few key things here.
//
//  1.  We maintain a local graphical bitmap based device the max size of the target display
//      client and echo all commands to it, so that we keep a copy of the remote content. This
//      allows us to correctly return current position or other graphical device info.
//  2.  All operations (relevant to template display) are intercepted and passed to the owning
//      worker thread who queues them up and (async) sends them out.
//  3.  We handle all queries locally, so the remote client doesn't need to return any sort
//      of information. Since we maintain a local shadow of the remote content, we can do this.
//
//  Note that we don't have to actually draw anything to our wrapped real graphics device. We
//  only need to do anything that would change the state of the device in a way that would
//  affect any query that might be made later (like setting the text or background colors and
//  doing things like pushing/popping fonts and regions and contexts.) Any operations that
//  would just cause graphics output don't need to actually be done. The interface engine
//  would never, in any remoted interface scenario, query our pixel contents. The kinds of
//  things that would cause that to happen are things not supported for remote viewer clients,
//  such as popout interfaces or fancy cover art browser display modes.
//
//  We get a size, which represents the largest the client could be, so that we can draw any
//  template he could ask us to draw (or as much of it that would matter.) We create our real
//  shadow graphics device that size.
//
//  We don't care about the client's pixel format. We create a true/alpha memory device for
//  our local shadow output. We are sending the commands to the client to draw, so his format
//  isn't really important. But, these days, most likely the client will be 32 bit format
//  anyway.
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

// ---------------------------------------------------------------------------
//   CLASS: TWebRIVAGraphDev
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCWEBRIVAEXP TWebRIVAGraphDev : public TGraphDrawDev
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWebRIVAGraphDev
        (
            const   TSize&                  szMax
            ,       TWebRIVAWIntf* const    pwrwiOwner
        );

        TWebRIVAGraphDev(const TWebRIVAGraphDev&) = delete;
        TWebRIVAGraphDev(TWebRIVAGraphDev&&) = delete;

        ~TWebRIVAGraphDev();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TWebRIVAGraphDev& operator=(const TWebRIVAGraphDev&) = delete;
        TWebRIVAGraphDev& operator=(TWebRIVAGraphDev&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods (from the base device class)
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCapConstAlpha() const;

        tCIDLib::TBoolean bCapSrcAlpha() const;

        tCIDLib::TBoolean bIsPaletteBased() const;

        tCIDLib::TCard4 c4BitsPerPixel() const;

        tCIDLib::EAdoptOpts eAdopted() const;

        tCIDGraphDev::TDeviceHandle hdevThis() const;

        TSize szPixels() const;

        TSize szPixelsPerInch() const;


        // -------------------------------------------------------------------
        //  Public, inherited methods (from our immediate parent class)
        // -------------------------------------------------------------------
        TArea areaBounds
        (
            const   tCIDLib::TBoolean       bReset = kCIDLib::False
        )   const final;

        TArea areaMLText
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4Width
            , const tCIDLib::TCard4         c4Start = 0
            , const tCIDLib::TCard4         c4Len = kCIDLib::c4MaxCard
        )   final;

        TArea areaString
        (
            const   TString&                strText
            , const tCIDLib::TBoolean       bSkipMnemonic = kCIDLib::True
        )   const final;

        TArea areaString
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Len = kCIDLib::c4MaxCard
            , const tCIDLib::TBoolean       bSkipMnemonic = kCIDLib::True
        )   const final;

        tCIDLib::TVoid AlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TArea&                  areaSource
            , const TArea&                  areaDest
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid AlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TArea&                  areaDest
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid AlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TPoint&                 pntDest
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid AlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TArea&                  areaSource
            , const TPoint&                 pntDestUL
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid AlphaBlit
        (
            const   TGraphDrawDev&          gdevSrc
            , const TArea&                  areaSource
            , const TArea&                  areaDest
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TBoolean bBoundsEnabled() const final;

        tCIDLib::TVoid BeginPath() final;

        tCIDLib::TCard4 c4Extra() const final;

        tCIDLib::TCard4 c4Extra
        (
            const   tCIDLib::TCard4         c4ToSet
        )   final;

        tCIDLib::TCard4 c4HPelsPerInch() const final;

        tCIDLib::TCard4 c4VPelsPerInch() const final;

        tCIDLib::TVoid ClearClipRegion() final;

        tCIDLib::TVoid CopyBits
        (
                    TGraphDrawDev&          gdevSrc
            , const TPoint&                 pntSrc
            , const TArea&                  areaTarget
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid CopyBits
        (
                    TGraphDrawDev&          gdevSrc
            , const TArea&                  areaSrc
            , const TArea&                  areaTarget
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
            , const tCIDLib::TBoolean       bWaitRetrace = kCIDLib::False
        )   final;

        tCIDLib::TVoid DisableBoundsAccumulation() final;

        tCIDLib::TVoid DrawArc
        (
            const   TPoint&                 pntCenter
            , const tCIDLib::TCard4         c4Radius
            , const tCIDLib::TCard4         c4StartDegree
            , const tCIDLib::TCard4         c4EndDegree
        )   final;

        tCIDLib::TVoid DrawBitmap
        (
            const   TBitmap&                bmpToDraw
            , const TPoint&                 pntDestUL
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
        )   final;

        tCIDLib::TVoid DrawBitmap
        (
            const   TBitmap&                bmpToDraw
            , const TPoint&                 pntSrcUL
            , const TPoint&                 pntDestUL
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
        )   final;

        tCIDLib::TVoid DrawBitmap
        (
            const   TBitmap&                bmpToDraw
            , const TArea&                  areaDest
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
        )   final;

        tCIDLib::TVoid DrawBitmap
        (
            const   TBitmap&                bmpToDraw
            , const TArea&                  areaSource
            , const TArea&                  areaDest
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
        )   final;

        tCIDLib::TVoid DrawBitmap
        (
            const   TBitmap&                bmpToDraw
            , const TArea&                  areaSource
            , const TPoint&                 pntDestUL
            , const tCIDGraphDev::EBmpModes eMode = tCIDGraphDev::EBmpModes::SrcCopy
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TArea&                  areaSource
            , const TArea&                  areaDest
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TArea&                  areaSource
            , const TPoint&                 pntDestUL
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TPoint&                 pntDestUL
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TBitmap&                bmpMask
            , const TArea&                  areaSource
            , const TArea&                  areaDest
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TBitmap&                bmpMask
            , const TArea&                  areaSource
            , const TPoint&                 pntDestUL
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBitmapMasked
        (
            const   TBitmap&                bmpToDraw
            , const TBitmap&                bmpMask
            , const TPoint&                 pntDestUL
            , const tCIDLib::TCard4         c4TransClr
            , const tCIDLib::TBoolean       bForceAlpha = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawBox
        (
            const   TPoint&                 pntFrom
            , const TPoint&                 pntTo
            , const tCIDLib::TCard4         c4Rounding = 0
        )   final;

        tCIDLib::TVoid DrawBox
        (
            const   TArea&                  areaBox
            , const tCIDLib::TCard4         c4Rounding = 0
        )   final;

        tCIDLib::TVoid DrawCircle
        (
            const   tCIDLib::TCard4         c4Radius
            , const TPoint&                 pntCenter
        )   final;

        tCIDLib::TVoid DrawFilledCircle
        (
            const   tCIDLib::TCard4         c4Radius
            , const TPoint&                 pntCenter
        )   final;

        tCIDLib::TVoid DrawFocusArea
        (
            const   TArea&                  areaToDraw
        )   final;

        tCIDLib::TVoid DrawGradientCircle
        (
            const   tCIDLib::TCard4         c4Radius
            , const TPoint&                 pntCenter
            , const TRGBClr&                rgbLight
            , const TRGBClr&                rgbDark
        )   final;

        tCIDLib::TVoid DrawIcon
        (
            const   TIcon&                  icoToDraw
            , const TPoint&                 pntDestUL
        )   final;

        tCIDLib::TVoid DrawIcon
        (
            const   TIcon&                  icoToDraw
            , const TPoint&                 pntDestUL
            , const TSize&                  szStretchTo
        )   final;

        tCIDLib::TVoid DrawIcon
        (
            const   TIcon&                  icoToDraw
            , const TArea&                  areaToFill
        )   final;

        tCIDLib::TVoid DrawIconPart
        (
            const   TIcon&                  icoToDraw
            , const TPoint&                 pntDestUL
            , const tCIDGraphDev::EBmpModes eMode
            , const tCIDLib::TBoolean       bMask = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawIconPart
        (
            const   TIcon&                  icoToDraw
            , const TPoint&                 pntDestUL
            , const TSize&                  szStretchTo
            , const tCIDGraphDev::EBmpModes eMode
            , const tCIDLib::TBoolean       bMask = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawIconPart
        (
            const   TIcon&                  icoToDraw
            , const TArea&                  areaToFill
            , const tCIDGraphDev::EBmpModes eMode
            , const tCIDLib::TBoolean       bMask = kCIDLib::False
        )   final;

        tCIDLib::TVoid DrawLine
        (
            const   TPoint&                 pntFrom
            , const TPoint&                 pntTo
            , const TRGBClr&                rgbClr
        )   final;

        tCIDLib::TVoid DrawMText
        (
            const   TString&                strText
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Top
            , const tCIDLib::TBoolean       bWordBreak = kCIDLib::True
        )   final;

        tCIDLib::TVoid DrawPBar
        (
            const   TBitmap&                bmpMask
            , const tCIDLib::TFloat4        f4Percent
            , const TArea&                  areaSrc
            , const TArea&                  areaTar
            , const tCIDGraphDev::EBltTypes eBlt
            , const tCIDLib::EDirs          eDir
            , const TRGBClr&                rgbStart
            , const TRGBClr&                rgbEnd
            , const TRGBClr&                rgbFill
            , const tCIDLib::TCard1         c1Opacity
        )   final;

        tCIDLib::TVoid DrawPolyLine
        (
            const   TPointArray&            pntaToDraw
            , const tCIDLib::TCard4         c4Count = kCIDLib::c4MaxCard
        )   final;

        tCIDLib::TVoid DrawPolyLine
        (
            const   TPointArray&            pntaToDraw
            , const TRGBClr&                rgbClr
            , const tCIDLib::TCard4         c4Count = kCIDLib::c4MaxCard
        )   final;

        tCIDLib::TVoid DrawPolySegments
        (
            const   TPointArray&            pntaToDraw
            , const tCIDLib::TCard4         c4Count = kCIDLib::c4MaxCard
        )   final;

        tCIDLib::TVoid DrawPolySegments
        (
            const   TPointArray&            pntaToDraw
            , const TRGBClr&                rgbClr
            , const tCIDLib::TCard4         c4Count = kCIDLib::c4MaxCard
        )   final;

        tCIDLib::TVoid DrawString
        (
            const   TString&                strText
            , const TPoint&                 pntAlign
        )   final;

        tCIDLib::TVoid DrawString
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4StartInd
            , const TPoint&                 pntAlign
        )   final;

        tCIDLib::TVoid DrawString
        (
            const   TString&                strText
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Center
            , const tCIDGraphDev::ETextFmts eFormat = tCIDGraphDev::ETextFmts::None
            , const TRGBClr&                rgbBgnFill = TRGBClr::Nul_TRGBClr()
        )   final;

        tCIDLib::TVoid DrawString
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4StartInd
            , const tCIDLib::TCard4         c4Len
            , const TArea&                  areaFormat
            , const tCIDLib::EHJustify      eHJustify = tCIDLib::EHJustify::Left
            , const tCIDLib::EVJustify      eVJustify = tCIDLib::EVJustify::Center
            , const tCIDGraphDev::ETextFmts eFormat = tCIDGraphDev::ETextFmts::None
            , const TRGBClr&                rgbBgnFill = TRGBClr::Nul_TRGBClr()
        )   final;

        tCIDLib::TVoid DrawStringFX
        (
            const   TString&                strText
            , const TArea&                  areaDraw
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
            , const tCIDLib::TCard1         c1ConstAlpha = 0xFF
        )   final;

        tCIDGraphDev::EBackMixModes eBackMixMode() const final;

        tCIDGraphDev::EBackMixModes eBackMixMode
        (
            const   tCIDGraphDev::EBackMixModes  eToSet
        )   final;

        tCIDGraphDev::EMixModes eMixMode() const final;

        tCIDGraphDev::EMixModes eMixMode
        (
            const   tCIDGraphDev::EMixModes eToSet
        )   final;

        tCIDLib::TVoid Fill
        (
            const   TPoint&                 pntFrom
            , const TPoint&                 pntTo
            , const TRGBClr&                rgbToUse
        )   final;

        tCIDLib::TVoid Fill
        (
            const   TArea&                  areaFill
            , const TRGBClr&                rgbToUse
        )   final;

        tCIDLib::TVoid FillPath() final;

        tCIDLib::TVoid FillWithBmpPattern
        (
            const   TArea&                  areaToFill
            , const TBitmap&                bmpToUse
            , const tCIDGraphDev::EBmpModes eMode
        )   final;

        tCIDLib::TVoid FillWithBmpPattern
        (
            const   TPoint&                 pntFrom
            , const TPoint&                 pntTo
            , const TBitmap&                bmpToUse
            , const tCIDGraphDev::EBmpModes eMode
        )   final;

        tCIDLib::TVoid FillWithBmpPattern
        (
            const   TArea&                  areaToFill
            , const TBitmap&                bmpToUse
            , const tCIDGraphDev::EBmpModes eMode
            , const TPoint&                 pntPatOrg
        )   final;

        tCIDLib::TVoid FillWithBmpPattern
        (
            const   TPoint&                 pntFrom
            , const TPoint&                 pntTo
            , const TBitmap&                bmpToUse
            , const tCIDGraphDev::EBmpModes eMode
            , const TPoint&                 pntPatOrg
        )   final;

        tCIDLib::TVoid Gradient3DFill
        (
            const   TArea&                  areaFill
            , const TRGBClr&                rgbLight
            , const TRGBClr&                rgbDark
        )   final;

        tCIDLib::TVoid Gradient3DFill2
        (
            const   TArea&                  areaFill
            , const TRGBClr&                rgbFill
        )   final;

        tCIDLib::TVoid GradientFill
        (
            const   TArea&                  areaFill
            , const TRGBClr&                rgbLeft
            , const TRGBClr&                rgbRight
            , const tCIDGraphDev::EGradDirs eDir = tCIDGraphDev::EGradDirs::Horizontal
        )   final;

        tCIDLib::TVoid GradientFill
        (
            const   TPoint&                 pntUL
            , const TPoint&                 pntLR
            , const TRGBClr&                rgbLeft
            , const TRGBClr&                rgbRight
            , const tCIDGraphDev::EGradDirs eDir = tCIDGraphDev::EGradDirs::Horizontal
        )   final;

        tCIDGraphDev::TFontHandle hfontSetFont
        (
            const   TGUIFont&                   gfontToSet
        )   final;

        tCIDGraphDev::TPenHandle hpenSetPen
        (
                    TGUIPen&                gpenNew
        )   final;

        tCIDGraphDev::TRegionHandle hrgnSetClipArea
        (
            const   tCIDGraphDev::EClipModes eMode
            , const TArea&                  areaToSet
            , const tCIDLib::TCard4         c4Rounding
        )   final;

        tCIDGraphDev::TRegionHandle hrgnSetClipRegion
        (
            const   tCIDGraphDev::EClipModes eMode
            , const TGUIRegion&             hrgnToSet
        )   final;

        tCIDLib::TVoid MaskedAlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TBitmap&                bmpMask
            , const TArea&                  areaSource
            , const TPoint&                 pntTarget
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TCard4         c4TransClr
        )   final;

        tCIDLib::TVoid MaskedAlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TBitmap&                bmpMask
            , const TPoint&                 pntTarget
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TCard4         c4TransClr
        )   final;

        tCIDLib::TVoid MaskedAlphaBlit
        (
            const   TBitmap&                bmpSrc
            , const TBitmap&                bmpMask
            , const TArea&                  areaSource
            , const TArea&                  areaTarget
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TCard4         c4TransClr
        )   final;

        tCIDLib::TVoid MaskedAlphaBlit
        (
            const   TGraphDrawDev&          gdevSrc
            , const TBitmap&                bmpMask
            , const TArea&                  areaSource
            , const TArea&                  areaTarget
            , const tCIDLib::TCard1         c1Alpha
            , const tCIDLib::TBoolean       bSrcAlpha
            , const tCIDLib::TCard4         c4TransClr
        )   final;

        tCIDLib::TVoid MoveRelative
        (
            const   TPoint&                 pntNewPos
        )   final;

        tCIDLib::TVoid MoveRelative
        (
            const   tCIDLib::TInt4          i4X
            , const tCIDLib::TInt4          i4Y
        )   final;

        tCIDLib::TVoid MoveTo
        (
            const   TPoint&                 pntNewPos
        )   final;

        tCIDLib::TVoid MoveTo
        (
            const   tCIDLib::TInt4          i4X
            , const tCIDLib::TInt4          i4Y
        )   final;

        TPoint pntCurPosition() const final;

        tCIDLib::TVoid PatternFill
        (
            const   TArea&                  areaToFill
            , const tCIDGraphDev::EBmpModes eMode
        )   final;

        tCIDLib::TVoid PopClipArea
        (
                    tCIDGraphDev::TRegionHandle  hrgnToPutBack
        )   final;

        tCIDLib::TVoid PopContext() final;

        tCIDLib::TVoid PopFont
        (
            const   TGUIFont&                   gfontCur
            ,       tCIDGraphDev::TFontHandle   hfontToPutBack
        )   final;

        tCIDLib::TVoid PopPen
        (
                    TGUIPen&                gpenCur
            ,       tCIDGraphDev::TPenHandle hpenOld
        )   final;

        tCIDLib::TVoid PushContext() final;

        tCIDLib::TVoid QueryCharOffsets
        (
            const   TString&                strSource
            , const tCIDLib::TCard4         c4Index
            ,       tCIDLib::TInt4&         i4Offset
            ,       tCIDLib::TCard4&        c4Width
        )   final;

        tCIDLib::TVoid QueryCharWidths
        (
            const   TString&                strSource
            ,       tCIDLib::TCardList&     fcolToFill
        )   final;

        tCIDLib::TVoid QueryFontMetrics
        (
                    TFontMetrics&           fmtrToFill
        )   final;

        TRGBClr rgbAt
        (
            const   TPoint&                 pntAt
        )   const final;

        TRGBClr rgbBgnColor() const final;

        TRGBClr rgbTextColor() const final;

        tCIDLib::TVoid ResetBoundsArea() final;

        TSize szPelsPerInch() const final;

        tCIDLib::TVoid SetBgnColor
        (
            const   TRGBClr&                rgbToSet
        )   final;

        tCIDLib::TVoid SetTextColor
        (
            const   TRGBClr&                rgbToSet
        )   final;

        tCIDLib::TVoid SetWorldTranslation
        (
            const   tCIDLib::TFloat4        f4XOfs
            , const tCIDLib::TFloat4        f4YOfs
        )   final;

        tCIDLib::TVoid Stroke
        (
            const   TArea&                  areaToStroke
        )   final;

        tCIDLib::TVoid Stroke
        (
            const   TArea&                  areaToStroke
            , const TRGBClr&                rgbClr
            , const tCIDLib::TCard4         c4Rounding
        )   final;

        tCIDLib::TVoid Stroke
        (
            const   TGUIRegion&             grgnToStroke
            , const TRGBClr&                rgbToUse
            , const tCIDLib::TCard4         c4Width = 1
        )   final;

        tCIDLib::TVoid StrokeAndFillPath() final;

        tCIDLib::TVoid StrokePath() final;



        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BuildImgPath
        (
            const   TBitmap&                bmpImg
            ,       TString&                strToFill
        );

        tCIDLib::TVoid EndDraw
        (
            const   TArea&                  areaUpdate
        );

        const TGraphDrawDev& gdevReal() const;

        tCIDLib::TVoid PlayWAV
        (
            const   TString&                strToPlay
            , const tCIDLib::EWaitModes     eWait
        );

        tCIDLib::TVoid StartDraw
        (
            const   TArea&                  areaUpdate
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4StartEndDepth
        //      We send the client start/end drawing cmds. But they can be nested when
        //      widgets force a redraw within the context of larger redraw. So we only
        //      want to send a start on the first one and and end when they are all done.
        //      So we use a depth indicator to track that. The Start/EndDraw() methods
        //      above takes care of that.
        //
        //  m_gdevShadow
        //      Our local shadow of the remote display. It's a memory based graphics device,
        //      which we've derived from so that we can intercept all the output.
        //
        //  m_pwrwiOwner
        //      A back link to our owning thread so that we can interact with it, mainly to
        //      queue up outgoing graphics commands for him to send. We look at this via our
        //      worker interface mixin defined above.
        //
        //  m_szMax
        //      The remote client has to tell us what the max size their display area is.
        //      That's the biggest size we'll ever have to deal with for this particular
        //      connection. And it's the size we'll allocate our local bitmap based device
        //      to.
        //
        //  m_wrtToUse
        //      An instance of the tools class that we generate from WebRIVACmp, for shared
        //      client/server stuff. We use this guy to format out our msgs that we are
        //      going to queue up.
        // -------------------------------------------------------------------
        tCIDLib::TCard4 m_c4StartEndDepth;
        TGraphMemDev    m_gdevShadow;
        TWebRIVAWIntf*  m_pwrwiOwner;
        TSize           m_szMax;
        TString         m_strTmp;
        TWebRIVATools   m_wrtToUse;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TWebRIVAGraphDev,TGraphDrawDev)
};

#pragma CIDLIB_POPPACK
