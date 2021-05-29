//
// FILE NAME: CQCIntfEng_DigitalClock.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/06/2002
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
//  This file implements a digital clock widget.
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
#include    "CQCIntfEng_DigitalClock.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfDigitalClock,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_DigitalClock
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions:
        //
        //  Version 1 -
        //
        //      Reset to 1 for V2.0 since the installer has to do a manual upgrade
        //      anyway.
        //
        //  Version 2 -
        //      No data change, but we need to adjust our usage of colors to make
        //      them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );


        // -----------------------------------------------------------------------
        //  An enum that defines our 4 'time zones' controllng the am/pm icon we
        //  display.
        // -----------------------------------------------------------------------
        enum class EZones
        {
            MoonUp
            , SunUp
            , SunDown
            , MoonDown
        };
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfDigitalClock
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfDigitalClock::TCQCIntfDigitalClock() :

    TCQCIntfWidget
    (
        CQCIntfEng_DigitalClock::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_DigitalClock)
    )
    , m_bRawText(kCIDLib::True)
    , m_bShowBorders(kCIDLib::False)
    , m_bShowPanels(kCIDLib::False)
    , m_eFmt(tCQCIntfEng::EClockFmts::Hour24)
{
    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/DigitalClock");
}

TCQCIntfDigitalClock::TCQCIntfDigitalClock(const TCQCIntfDigitalClock& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , m_bRawText(iwdgSrc.m_bRawText)
    , m_bShowBorders(iwdgSrc.m_bShowBorders)
    , m_bShowPanels(iwdgSrc.m_bShowPanels)
    , m_eFmt(iwdgSrc.m_eFmt)
{
}

TCQCIntfDigitalClock::~TCQCIntfDigitalClock()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Public operators
// ---------------------------------------------------------------------------
TCQCIntfDigitalClock&
TCQCIntfDigitalClock::operator=(const TCQCIntfDigitalClock& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        m_bRawText     = iwdgSrc.m_bRawText;
        m_bShowBorders = iwdgSrc.m_bShowBorders;
        m_bShowPanels  = iwdgSrc.m_bShowPanels;
        m_eFmt         = iwdgSrc.m_eFmt;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfDigitalClock::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfDigitalClock& iwdgOther
    (
        static_cast<const TCQCIntfDigitalClock&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther))
        return kCIDLib::False;

    return (m_bRawText == iwdgOther.m_bRawText)
           && (m_bShowBorders == iwdgOther.m_bShowBorders)
           && (m_bShowPanels == iwdgOther.m_bShowPanels)
           && (m_eFmt == iwdgOther.m_eFmt);
}


tCIDLib::TVoid
TCQCIntfDigitalClock::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfDigitalClock&>(iwdgSrc);
}


tCIDLib::TVoid TCQCIntfDigitalClock::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfDigitalClock::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);


    // Remove any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Set our specific names on some of the numbers
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    //
    //  We don't use the caption text, and we don't do text wrap so remove that option once
    //  the font mixin adds it.
    //
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Font_NoWrap);

    // And do our stuff
    adatTmp.Set(L"Clock Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // Do our stuff
    TString strLims;
    tCQCIntfEng::FormatEClockFmts(strLims, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
    adatTmp.Set
    (
        L"Display Format"
        , kCQCIntfEng::strAttr_DClock_Format
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatEClockFmts(m_eFmt));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Raw Text", kCQCIntfEng::strAttr_DClock_RawText, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bRawText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Show Borders", kCQCIntfEng::strAttr_DClock_Borders, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bShowBorders);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Show Panels", kCQCIntfEng::strAttr_DClock_Panels, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bShowPanels);
    colAttrs.objAdd(adatTmp);
}


TSize TCQCIntfDigitalClock::szDefaultSize() const
{
    return TSize(92, 24);
}


tCIDLib::TVoid
TCQCIntfDigitalClock::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);

    // Handle our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_DClock_Format)
        m_eFmt = tCQCIntfEng::eXlatEClockFmts(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_DClock_RawText)
        m_bRawText = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_DClock_Borders)
        m_bShowBorders = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_DClock_Panels)
        m_bShowPanels = adatNew.bVal();
}


tCIDLib::TVoid
TCQCIntfDigitalClock::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                , TDataSrvClient&               dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfDigitalClock::Set(  const   tCQCIntfEng::EClockFmts eFmt
                            , const tCIDLib::TBoolean       bRawText
                            , const tCIDLib::TBoolean       bShowBorders
                            , const tCIDLib::TBoolean       bShowPanels)
{
    m_bRawText = bRawText;
    m_bShowBorders = bShowBorders;
    m_bShowPanels = bShowPanels;
    m_eFmt = eFmt;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfDigitalClock::ActiveUpdate( const   tCIDLib::TBoolean bNoRedraw
                                    , const TGUIRegion&
                                    , const tCIDLib::TBoolean bInTopLayer)
{
    //
    //  Get the current time and clip to the minute. If it's the same as
    //  the stored value, then we don't need to do anything. If it's different,
    //  then store it as our new value and redraw unless told not to.
    //
    TTime tmNew(tCIDLib::ESpecialTimes::CurrentTime);
    tmNew.ClipToMinute();
    if (tmNew != m_tmValue)
    {
        m_tmValue = tmNew;
        if (!bNoRedraw)
            Redraw();
    }
}


tCIDLib::TVoid TCQCIntfDigitalClock::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DigitalClock::c2FmtVersion))
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
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    // And now stream in our own stuff when/if we have some
    strmToReadFrom  >> m_bShowBorders
                    >> m_bShowPanels
                    >> m_eFmt
                    >> m_bRawText;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    //
    //  If V1, then we need to adjust some colors, so that the F2 color is
    //  always used for shadow/fx. Currently F3 is being used for shadow and
    //  F2 for FX, and F2 is also being used for the border.
    //
    if (c2FmtVersion == 1)
    {
        // Save the original values we are going to change
        TRGBClr rgbF2 = rgbFgn2();
        TRGBClr rgbF3 = rgbFgn3();

        // Put F2 into F3, which is now the border color
        rgbFgn3(rgbF2);

        //
        //  If not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }
}

tCIDLib::TVoid TCQCIntfDigitalClock::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our stuff, then an end object marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_DigitalClock::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);

    // Stream our stuff and a marker
    strmToWriteTo   << m_bShowBorders
                    << m_bShowPanels
                    << m_eFmt
                    << m_bRawText
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfDigitalClock::Update(       TGraphDrawDev&      gdevTarget
                            , const TArea&              areaInvalid
                            ,       TGUIRegion&         grgnClip)
{
    // Do any bgn/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Set any updated clip region and draw the contents
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    DrawText(gdevTarget, areaInvalid, kCIDLib::True);
}



// ---------------------------------------------------------------------------
//  TCQCIntfDigitalClock: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfDigitalClock::DrawText(         TGraphDrawDev&      gdevTarget
                                , const TArea&              areaUpdate
                                , const tCIDLib::TBoolean   bDrawBorder)
{
    // Draw the border if needed, and adjust the drawing area
    TArea areaDraw(areaActual());
    if (bHasBorder())
    {
        if (bDrawBorder)
            gdevTarget.Stroke(areaDraw, rgbFgn3());
        areaDraw.Deflate(1);
    }

    //
    //  If not transparent, then let our parent do the standard fill. Set the
    //  current drawing area left as the clip.
    //
    if (!bIsTransparent())
    {
        TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);
        DoBgnFill(gdevTarget, areaUpdate);
    }

    // Now move the clip in further for the drawing part and set a new clip
    areaDraw.Deflate(3, 2);
    TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);

    const tCIDLib::TBoolean bDoShadow = bShadow();

    // Format according to our flags
    TString strNew;
    m_tmValue.FormatToStr
    (
        strNew
        , (m_eFmt == tCQCIntfEng::EClockFmts::Hour24) ? TTime::str24HHMM()
                                                     : TTime::strHHMMap()
    );

    // Set our font
    TCQCIntfFontJan janFont(&gdevTarget, *this);

    //
    //  If the raw text flag is set, then we just display the text as is
    //  and it's very simple. Else, we have to do the panel type display.
    //
    if (m_bRawText)
    {
        // Throw away a leading zero if in 12 hours mode
        if ((m_eFmt == tCQCIntfEng::EClockFmts::Hour12)
        &&  (strNew.chFirst() == kCIDLib::chDigit0))
        {
            strNew.Cut(0,1);
        }

        if (eTextFX() == tCIDGraphDev::ETextFX::None)
        {
            // If we have a drop shadow, draw it in the shadow color and shifted
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
            if (bDoShadow)
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaDraw);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(rgbFgn2());
                gdevTarget.DrawString(strNew, areaDrop, eHJustify(), eVJustify());
            }

            // And then in the regular foreground color
            gdevTarget.SetTextColor(rgbFgn());
            gdevTarget.DrawString(strNew, areaDraw, eHJustify(), eVJustify());
        }
         else
        {
            civOwner().DrawTextFX
            (
                gdevTarget
                , strNew
                , areaDraw
                , eTextFX()
                , rgbFgn()
                , rgbFgn2()
                , eHJustify()
                , eVJustify()
                , bNoTextWrap()
                , pntOffset()
            );
        }
    }
     else
    {
        //
        //  Get the AM/PM icon we'll use (assuming we use one), and set the
        //  zone indicator, for use below when (if) we display it.
        //
        tCIDLib::TCard4 c4Hour;
        tCIDLib::TCard4 c4Minute;
        tCIDLib::TCard4 c4Second;
        m_tmValue.c4AsTimeInfo(c4Hour, c4Second, c4Minute);

        TIcon icoAMPM;
        CQCIntfEng_DigitalClock::EZones eZone;
        if (c4Hour < 6)
        {
            eZone = CQCIntfEng_DigitalClock::EZones::MoonUp;
            icoAMPM.Set(facCQCIntfEng(), kCQCIntfEng::ridIco_Moon);
        }
         else if (c4Hour < 12)
        {
            eZone = CQCIntfEng_DigitalClock::EZones::SunUp;
            icoAMPM.Set(facCQCIntfEng(), kCQCIntfEng::ridIco_Sun);
        }
         else if (c4Hour < 18)
        {
            eZone = CQCIntfEng_DigitalClock::EZones::SunDown;
            icoAMPM.Set(facCQCIntfEng(), kCQCIntfEng::ridIco_Sun);
        }
         else
        {
            eZone = CQCIntfEng_DigitalClock::EZones::MoonDown;
            icoAMPM.Set(facCQCIntfEng(), kCQCIntfEng::ridIco_Moon);
        }
        const TSize szIcon = icoAMPM.szBitmap();

        //
        //  Get the maximimum text box for our font. The max width is more than
        //  big enough, but add a couple to the vertical to allow for the panel
        //  border.
        //
        //  <TBD> We could get this at create time and watch for config changes
        //  to update it during design phase.
        //
        TFontMetrics fmtrCur(gdevTarget);
        TArea areaOneDigit
        (
            TPoint::pntOrigin
            , fmtrCur.c4MaxWidth()
            , fmtrCur.c4Height() + fmtrCur.c4Descender() + 2
        );

        //
        //  Create another area that will enclose all of the panels. It is
        //  4 panels for the numbers, plus half of one for the colon, and if
        //  in 12 hours mode, another one wide enough for the icon.
        //
        TArea areaAll
        (
            0
            , 0
            , (areaOneDigit.c4Width() * 4) + (areaOneDigit.c4Width() / 2)
            , areaOneDigit.c4Height()
        );

        if (m_eFmt == tCQCIntfEng::EClockFmts::Hour12)
            areaAll.AdjustSize(szIcon.c4Width() + 2, 0);

        //
        //  Justify the all area within the widget area according to our
        //  justification settings. This will then give us the starting
        //  origin to correctly position the panels.
        //
        areaAll.JustifyIn(areaDraw, eHJustify(), eVJustify());

        // Start off to the left of this area
        areaOneDigit.LeftJustifyIn(areaAll, kCIDLib::True);

        //
        //  If 12 hours mode, then 5 panels, else 6 since we have the am/pm
        //  indicator.
        //
        const tCIDLib::TCard4 c4PanCount
        (
            (m_eFmt == tCQCIntfEng::EClockFmts::Hour24) ? 5 : 6
        );

        // And now let's do the appropriate update for our mode
        TArea areaTmp;
        gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
        gdevTarget.SetTextColor(rgbFgn());
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PanCount; c4Index++)
        {
            //
            //  If we aren't in text mode, and the current panel doesn't
            //  intersect the update area, it does no good to update it.
            //
            if (bDrawBorder && !areaOneDigit.bIntersects(areaUpdate))
            {
                if ((c4Index == 2) || (c4Index == 5))
                    areaOneDigit.AdjustOrg(tCIDLib::TInt4(areaOneDigit.c4Width() / 2), 0);
                else
                    areaOneDigit.Move(tCIDLib::EDirs::Right);
                continue;
            }

            //
            //  Get a temp area to work with. If we are on the colon or am/pm
            //  indicator, then set it to half size or it based on the icon size.
            //
            TArea areaCur(areaOneDigit);
            if (c4Index == 2)
                areaCur.ScaleSize(0.5, 1.0);
            else if (c4Index == 5)
                areaCur.c4Width(szIcon.c4Width() + 2);

            //
            //  Fill the area inside the border of the panel. We cannot just
            //  fill the whole thing and depend on the border drawing back
            //  over it, because we don't draw the border in text only mode.
            //
            //  Skip this if not drawing panels.
            //
            if (m_bShowPanels)
            {
                areaTmp = areaCur;
                areaTmp.Deflate(1);
                gdevTarget.GradientFill
                (
                    areaTmp
                    , rgbBgn()
                    , rgbBgn2()
                    , tCIDGraphDev::EGradDirs::Vertical
                );
            }

            //
            //  Draw the current character, centered in it's panel area,
            //  unless it's the am/pm indicator, in which case we draw a
            //  little sun or moon to indicate that. If we are in 24 hours
            //  mode and we are on the 0th char and it'sa zero, then just
            //  skip it.
            //
            if (c4Index == 5)
            {
                //
                //  The deal is that we have 4 six hour zones. From midnight
                //  to 6am, we display a moon at the top. From 6am to noon
                //  we display a sun at the top. From noon to 6pm we display
                //  a sun at the bottom, and from 6pm to midnight we display
                //  a moon at the bottom.
                //
                areaTmp.Set(TPoint::pntOrigin, szIcon);
                if ((eZone == CQCIntfEng_DigitalClock::EZones::MoonDown)
                ||  (eZone == CQCIntfEng_DigitalClock::EZones::SunDown))
                {
                    areaTmp.BottomJustifyIn(areaCur, kCIDLib::True);
                    areaTmp.AdjustOrg(0, -3);
                }
                 else
                {
                    areaTmp.TopJustifyIn(areaCur, kCIDLib::True);
                    areaTmp.AdjustOrg(0, 3);
                }
                gdevTarget.DrawIcon(icoAMPM, areaTmp.pntOrg());
            }
             else
            {
                if (c4Index || (strNew[c4Index] != kCIDLib::chDigit0))
                {
                    gdevTarget.DrawString
                    (
                        strNew
                        , c4Index
                        , 1
                        , areaCur
                        , tCIDLib::EHJustify::Center
                        , tCIDLib::EVJustify::Center
                    );
                }
            }

            //
            //  If not just text, and showing borders, draw the border around
            //  the panel.
            //
            if (bDrawBorder && m_bShowBorders)
            {
                {
                    gdevTarget.DrawLine
                    (
                        areaCur.pntLL(), areaCur.pntUL(), facCIDGraphDev().rgbPaleGrey
                    );
                    gdevTarget.DrawLine
                    (
                        areaCur.pntUL(), areaCur.pntUR(), facCIDGraphDev().rgbPaleGrey
                    );
                }

                {
                    gdevTarget.DrawLine
                    (
                        areaCur.pntLL(), areaCur.pntLR(), facCIDGraphDev().rgbDarkGrey
                    );
                    gdevTarget.DrawLine
                    (
                        areaCur.pntLR(), areaCur.pntUR(), facCIDGraphDev().rgbDarkGrey
                    );
                }
            }

            // Move over to the next position
            areaOneDigit.AdjustOrg(tCIDLib::TInt4(areaCur.c4Width()), 0);
        }
    }
}


