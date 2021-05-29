//
// FILE NAME: CQCIntfEng_DynamicText.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/12/2002
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
//  This file implements a derivative of the base widget which does a basic
//  format to text of the value of any widget type.
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
#include    "CQCIntfEng_DynamicText.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfDynTextBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldText,TCQCIntfDynTextBase)
AdvRTTIDecls(TCQCIntfVarText,TCQCIntfDynTextBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_DynamicText
    {
        // -----------------------------------------------------------------------
        //  Base class persistent format version
        //
        //  Version 2 -
        //      We refactored the Xlats stuff out to a separate mixin.
        //
        //  Version 3 -
        //      No data change but we have to change our use of colors to be
        //      consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion    = 3;


        // -----------------------------------------------------------------------
        // Field class persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion     = 1;


        // -----------------------------------------------------------------------
        // Variable class persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use for normal and marquee modes. Marquees
        //  need active updating and have fewer graphical/font options. In both
        //  cases we need value updates, since we are going to to get our value
        //  from either a field or variable.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eMarqCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStdCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfDynTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfDynTextBase::~TCQCIntfDynTextBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfDynTextBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfDynTextBase& iwdgOther
    (
        static_cast<const TCQCIntfDynTextBase&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfXlatIntf::bSameXlats(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther))
    {
        return kCIDLib::False;
    }

    // None of our our stuff is persistent
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfDynTextBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Set our font and get the width our text, and the average character
    //  width. If we are a marguee mode static text widget, we'll use this
    //  in the scrolling code. At this level we only need to get the avergage
    //  width. The text content specific stuff is done when we set the
    //  text.
    //
    if (m_bMarquee)
    {
        tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
        TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
        TFontMetrics fmtrCur(*cptrGraphDev);
        m_c4AvgCharWidth = fmtrCur.c4AverageWidth();
    }

    // Until we get live data, get ourself formatted with an error status
    SetText(TString::strEmpty(), kCIDLib::True, kCIDLib::True);
}


tCIDLib::TVoid
TCQCIntfDynTextBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove base attributes we don't support
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfXlatIntf::QueryXlatAttrs(colAttrs, adatTmp);
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, &strCaption());
}


//
//  We just return a size required to fit a short sample text string at the
//  current font settings.
//
TSize TCQCIntfDynTextBase::szDefaultSize() const
{
    // Base on the default text with the current font
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
    TArea areaText;
    areaText = cptrGraphDev->areaString(L"Some Sample Text", kCIDLib::False);
    areaText.AdjustSize(16, 4);
    return areaText.szArea();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfDynTextBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfXlatIntf::SetXlatAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
}



//
//  Called to ask us to validate any configuration we have. At this level,
//  currently we don't have any open ended configuration, so we don't need
//  to do anything yet. We just pass it on to our parent.
//
tCIDLib::TVoid
TCQCIntfDynTextBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Nothing else of our own for now
}


// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfDynTextBase::
TCQCIntfDynTextBase(const   tCIDLib::TBoolean   bMarqueMode
                    , const TString&            strTypeName) :

    TCQCIntfWidget
    (
        bMarqueMode ? CQCIntfEng_DynamicText::eMarqCapFlags
                    : CQCIntfEng_DynamicText::eStdCapFlags
        , strTypeName
    )
    , m_bMarquee(bMarqueMode)
    , m_bValError(kCIDLib::False)
    , m_c4AvgCharWidth(0)
    , m_c4TextWidth(0)
    , m_enctNextScroll(0)
    , m_i4LastOfs(0)
    , m_strLiveText(facCQCIntfEng().strErrText())
{
    // We allow multi-line text, so turn off the single line flag if not marquee
    if (!bMarqueMode)
        ResetCaption(kCIDLib::False, TString::strEmpty());

    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Text");
}

TCQCIntfDynTextBase::TCQCIntfDynTextBase(const TCQCIntfDynTextBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfXlatIntf(iwdgSrc)
    , m_bMarquee(iwdgSrc.m_bMarquee)
    , m_c4AvgCharWidth(iwdgSrc.m_c4AvgCharWidth)
    , m_c4TextWidth(iwdgSrc.m_c4TextWidth)
    , m_enctNextScroll(iwdgSrc.m_enctNextScroll)
    , m_i4LastOfs(iwdgSrc.m_i4LastOfs)
    , m_strDisplayText(iwdgSrc.m_strDisplayText)
    , m_strLiveText(iwdgSrc.m_strLiveText)
{
    // Always goes back to false
    m_bValError = kCIDLib::False;
}

TCQCIntfDynTextBase&
TCQCIntfDynTextBase::operator=(const TCQCIntfDynTextBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfXlatIntf::operator=(iwdgSrc);
        m_bMarquee       = iwdgSrc.m_bMarquee;
        m_c4AvgCharWidth = iwdgSrc.m_c4AvgCharWidth;
        m_c4TextWidth    = iwdgSrc.m_c4TextWidth;
        m_enctNextScroll = iwdgSrc.m_enctNextScroll;
        m_i4LastOfs      = iwdgSrc.m_i4LastOfs;
        m_strDisplayText = iwdgSrc.m_strDisplayText;

        // Always goes back to false and no available live text until set
        m_bValError = kCIDLib::False;
        m_strLiveText = facCQCIntfEng().strErrText();
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If we are in marquee mode, we register for active updates so that we can
//  scroll our text value.
//
tCIDLib::TVoid
TCQCIntfDynTextBase::ActiveUpdate(  const   tCIDLib::TBoolean bNoRedraw
                                    , const TGUIRegion&
                                    , const tCIDLib::TBoolean bInTopLayer)
{
    // If no text, do nothing
    if (m_strDisplayText.bIsEmpty())
        return;

    // We only scroll if the text is shorter than the available area
    if (m_c4TextWidth + 4 > areaActual().c4Width())
    {
        const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        if (enctCur >= m_enctNextScroll)
        {
            m_enctNextScroll = enctCur + (kCIDLib::enctOneMilliSec * 50);
            const TArea& areaDraw = areaActual();
            tCIDLib::TInt4 i4Adjustment(m_c4AvgCharWidth / 6);
            if (!i4Adjustment)
                i4Adjustment = 1;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < 2; c4Index++)
            {
                m_i4LastOfs -= i4Adjustment;
                if (m_i4LastOfs < 0)
                {
                    //
                    //  We've pushed the whole thing off, so back to the far right,
                    //  and start with 1 char visible.
                    //
                    if (tCIDLib::TCard4(m_i4LastOfs * -1) > m_c4TextWidth)
                        m_i4LastOfs = tCIDLib::TInt4(areaDraw.c4Width() - m_c4AvgCharWidth);
                }

                // Redraw if if we can
                if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden)
                &&  !bNoRedraw
                &&  bInTopLayer)
                {
                   Redraw();
                }
            }
        }
    }
}


// If the caption text changes we may need to update our displayed text
tCIDLib::TVoid TCQCIntfDynTextBase::CaptionChanged()
{
    FmtXlatText(strCaption(), m_strLiveText, m_bValError, m_strDisplayText);
}


tCIDLib::TVoid TCQCIntfDynTextBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DynamicText::c2BaseFmtVersion))
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

    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixins
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    //
    //  THe xlats stuff was refactored out to a mixin, so we have to either
    //  let it read in the list of ours, or let it stream its own stuff in.
    //
    if (c2FmtVersion > 1)
        MCQCIntfXlatIntf::ReadInXlats(strmToReadFrom);
     else
        MCQCIntfXlatIntf::RefactorRead(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom  >> m_bMarquee;

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
        //  If not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }

    // Rest the runtime members
    m_c4AvgCharWidth = 0;
    m_c4TextWidth = 0;
    m_i4LastOfs = 0;
    m_strDisplayText.Clear();
    m_strLiveText.Clear();
}


tCIDLib::TVoid TCQCIntfDynTextBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_DynamicText::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfXlatIntf::WriteOutXlats(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << m_bMarquee
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfDynTextBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Set any adjusted clip
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    TArea areaDraw(grgnClip.areaBounds());
    DrawText(gdevTarget, areaInvalid, areaDraw);
}


// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase:: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The derived classes call this to give us a new value to display.
//
tCIDLib::TVoid
TCQCIntfDynTextBase::SetText(const  TString&            strToSet
                            , const tCIDLib::TBoolean   bNoRedraw
                            , const tCIDLib::TBoolean   bErrState)
{
    m_bValError = bErrState;

    // Handle any translation and such
    m_strLiveText = strToSet;
    FmtXlatText(strCaption(), m_strLiveText, m_bValError, m_strDisplayText);

    //
    //  If in marquee mode, update our text width info, and reset the last
    //  offset value in case it's now not valid.
    //
    if (m_bMarquee)
    {
        tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
        TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
        m_c4TextWidth = cptrGraphDev->areaString
        (
            m_strDisplayText, kCIDLib::False
        ).szArea().c4Width();
        m_i4LastOfs = 0;
    }

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


// ---------------------------------------------------------------------------
//  TCQCIntfDynTextBase: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfDynTextBase::DrawText(          TGraphDrawDev&      gdevTarget
                                , const TArea&              areaInvalid
                                ,       TArea&              areaDraw)
{
    //
    //  Now move the clip in further for the drawing part. If the text area
    //  intersects the invalid area, then draw it.
    //
    if (areaDraw.bIntersects(areaInvalid))
    {
        TCQCIntfView& civUs = civOwner();

        // If no good value, show disabled
        tCIDGraphDev::ETextFmts eFmt = tCIDGraphDev::ETextFmts::None;
        if (m_bValError && !TFacCQCIntfEng::bDesMode())
            eFmt = tCIDGraphDev::ETextFmts::Disabled;

        // Set our font
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        //
        //  According to whether we are in marquee mode, we will do different
        //  drawing. Else it's either either just normal text, or it might
        //  use one of the text effects.
        //
        gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
        if (m_bMarquee)
        {
            gdevTarget.SetTextColor(rgbFgn());
            areaDraw.AdjustOrg(m_i4LastOfs, 0);
            areaDraw.AdjustSize(-m_i4LastOfs, 0);
            gdevTarget.DrawString
            (
                m_strDisplayText, areaDraw, tCIDLib::EHJustify::Left, eVJustify(), eFmt
            );
        }
         else if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civUs.DrawTextFX
            (
                gdevTarget
                , m_strDisplayText
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
         else
        {
            //
            //  Now draw our text. If the shadow style is enabled, then draw
            //  first a drop version in fgn3, then draw the regular one in
            //  fgn1 in the normal position.
            //
            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaDraw);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(rgbFgn2());

                // If no wrap, use single line draw, else multi-line
                if (bNoTextWrap())
                {
                    gdevTarget.DrawString
                    (
                        m_strDisplayText, areaDrop, eHJustify(), eVJustify(), eFmt
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        m_strDisplayText, areaDrop, eHJustify(), eVJustify(), kCIDLib::True
                    );
                }
            }

            //
            //  Now draw our text
            //
            //  <TBD> When mutlti-text supports format flags, we need to use
            //  the format flag set above to show disabled status.
            //
            gdevTarget.SetTextColor(rgbFgn());

            // If no wrap, use single line draw, else multi-line
            if (bNoTextWrap())
            {
                gdevTarget.DrawString
                (
                    m_strDisplayText, areaDraw, eHJustify(), eVJustify(), eFmt
                );
            }
             else
            {
                gdevTarget.DrawMText
                (
                    m_strDisplayText, areaDraw, eHJustify(), eVJustify(), kCIDLib::True
                );
            }
        }
    }

    // If underline is enabled, then draw a line along the bottom of the area
    if (bUnderline())
    {
        const TArea areaLine = areaActual();
        gdevTarget.DrawLine(areaLine.pntLL(), areaLine.pntLR(), rgbFgn());
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldText::TCQCIntfFldText() :

    TCQCIntfDynTextBase
    (
        kCIDLib::False
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Text)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldText::TCQCIntfFldText(const tCIDLib::TBoolean bMarqueeMode) :

    TCQCIntfDynTextBase
    (
        bMarqueeMode
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg
            (
                bMarqueeMode ? kIEngMsgs::midWidget_Marquee
                             : kIEngMsgs::midWidget_Text
            )
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldText::TCQCIntfFldText(const TCQCIntfFldText& iwdgToCopy) :

    TCQCIntfDynTextBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldText::~TCQCIntfFldText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldText&
TCQCIntfFldText::operator=(const TCQCIntfFldText& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfFldText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldText& iwdgOther
    (
        static_cast<const TCQCIntfFldText&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    // None of our our stuff is persistent
    return kCIDLib::True;
}


// Copy ourself polymorphically from another widget of the same type
tCIDLib::TVoid
TCQCIntfFldText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldText::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
                        , const tCIDLib::TCard4         c4Index
                        , const TString&                strUserId
                        , const TString&                strEventId
                        ,       TStdVarsTar&            ctarGlobals
                        ,       tCIDLib::TBoolean&      bResult
                        ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToField)
    {
        // Just pass the field name on to our field mixin
        ChangeField(ccfgToDo.piAt(0).m_strValue);
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


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFldText::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the field
    {
        TCQCCmd& cmdLinkToFld = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_LinkToField
                , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToField)
                , tCQCKit::ECmdPTypes::Field
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Field)
            )
        );

        cmdLinkToFld.cmdpAt(0).eFldAccess(tCQCKit::EReqAccess::MReadCWrite);
    }
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfFldText::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldText::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Text Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldText::RegisterFields(        TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldText::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFull
                        ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfFldText::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  Called to ask us to validate any configuration we have. At this level,
//  we call our parent, then validate the field mixin
//
tCIDLib::TVoid
TCQCIntfFldText::Validate(  const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclToUse) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclToUse);

    // Call any mixins
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclToUse, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldText: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If our field mixin sees a change in our state or value, then we need
//  to update, so he'll call us here.
//
tCIDLib::TVoid
TCQCIntfFldText::FieldChanged(          TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();

    TString strNewVal;
    fvCurrent().Format(strNewVal);
    SetText
    (
        strNewVal
        , bNoRedraw
        , (eNewState != tCQCPollEng::EFldStates::Ready)
    );
}


tCIDLib::TVoid TCQCIntfFldText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DynamicText::c2FldFmtVersion))
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

    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixins
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // Nothing of our own for now

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFldText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_DynamicText::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldText::ValueUpdate(       TCQCPollEngine&     polleToUse
                            , const tCIDLib::TBoolean   bNoRedraw
                            , const tCIDLib::TBoolean
                            , const TStdVarsTar&        ctarGlobalVars
                            , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarText::TCQCIntfVarText() :

    TCQCIntfDynTextBase
    (
        kCIDLib::False
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Text)
    )
    , MCQCIntfVarIntf()
{
}

TCQCIntfVarText::TCQCIntfVarText(const tCIDLib::TBoolean bMarqueeMode) :

    TCQCIntfDynTextBase
    (
        bMarqueeMode
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg
            (
                bMarqueeMode ? kIEngMsgs::midWidget_Marquee
                             : kIEngMsgs::midWidget_Text
            )
    )
    , MCQCIntfVarIntf()
{
}

TCQCIntfVarText::TCQCIntfVarText(const TCQCIntfVarText& iwdgToCopy) :

    TCQCIntfDynTextBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarText::~TCQCIntfVarText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarText&
TCQCIntfVarText::operator=(const TCQCIntfVarText& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarText& iwdgOther
    (
        static_cast<const TCQCIntfVarText&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfVarIntf::bSameVariable(iwdgOther))
        return kCIDLib::False;

    // None of our our stuff is persistent
    return kCIDLib::True;
}


// Copy ourself polymorphically from another widget of the same type
tCIDLib::TVoid TCQCIntfVarText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarText::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
                        , const tCIDLib::TCard4         c4Index
                        , const TString&                strUserId
                        , const TString&                strEventId
                        ,       TStdVarsTar&            ctarGlobals
                        ,       tCIDLib::TBoolean&      bResult
                        ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_LinkToVar)
    {
        SetVarInfo(ccfgToDo.piAt(0).m_strValue);
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


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfVarText::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the variable
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_LinkToVar
            , facCQCKit().strMsg(kKitMsgs::midCmd_LinkToVar)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );
}


tCIDLib::TVoid
TCQCIntfVarText::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}



tCIDLib::TVoid
TCQCIntfVarText::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                        , const TString&                strSrc
                        , const TString&                strTar
                        , const tCIDLib::TBoolean       bRegEx
                        , const tCIDLib::TBoolean       bFullMatch
                        ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
    {
        MCQCIntfVarIntf::ReplaceVar
        (
            strSrc, strTar, bRegEx, bFullMatch, regxFind
        );
    }
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarText::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarText::Validate(  const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarText: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  When our variable shows up, our variable mixin will call us here to see
//  if we can accept it, so that we can set ourselves up to match it's
//  type and limits.
//
//  For us, we just say yeh, because we can handle any field type.
//
tCIDLib::TBoolean TCQCIntfVarText::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfVarText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DynamicText::c2VarFmtVersion))
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

    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixins
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // We don't currently have any stuff of our own

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfVarText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_DynamicText::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  We look up our variable in the passed global vars and see if the serial
//  number has changed. If so, we pass the new value on to our parent class
//  as the current value.
//
tCIDLib::TVoid
TCQCIntfVarText::ValueUpdate(       TCQCPollEngine&
                            , const tCIDLib::TBoolean   bNoRedraw
                            , const tCIDLib::TBoolean   bVarUpdate
                            , const TStdVarsTar&        ctarGlobalVars
                            , const TGUIRegion&)
{
    // If no variables could have changed, don't bother
    if (!bVarUpdate)
        return;

    // Pass it on to our mixing
    VarUpdate(ctarGlobalVars, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the variable state
//  changes out of ready state, to let us know we need to update our display
//  to indicate we don't have a good value.
//
tCIDLib::TVoid
TCQCIntfVarText::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetText(TString::strEmpty(), bNoRedraw, kCIDLib::True);
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarText::VarValChanged( const   TCQCActVar&         varNew
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    // Tell our parent class about the new value
    SetText(varNew.strValue(), bNoRedraw, kCIDLib::False);
}


