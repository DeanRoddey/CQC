//
// FILE NAME: CQCIntfEng_TimeText.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/17/2007
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
//  This file implements a derivative of the base widget which can display
//  date/time informaiton in a flexible way, either from a field or form
//  the local host time.
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
#include    "CQCIntfEng_TimeText.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfTimeTextBase, TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFieldTimeText, TCQCIntfTimeTextBase)
AdvRTTIDecls(TCQCIntfLocalTimeText, TCQCIntfTimeTextBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_TimeText
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version for the based class
        //
        //  In 2.4.17 we refactored out to a base class plus derivatives, so our
        //  old class is in the Refactored.cpp file, and just started over at V1
        //  for this new class and the derivatives.
        //
        //  Version 2 -
        //      No data change, but we need to adjust our use of colors to make
        //      them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion    = 2;


        // -----------------------------------------------------------------------
        //  Our persistent format version for field based derivative
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  Our persistent format version for local derivative
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2LocalFmtVersion   = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use. One set for the field based and one for
        //  the local version.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFieldCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eLocalCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTimeTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfTimeTextBase::~TCQCIntfTimeTextBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfTimeTextBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfTimeTextBase& iwdgOther(static_cast<const TCQCIntfTimeTextBase&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther))
        return kCIDLib::False;

    // Do our own stuff
    return
    (
        (m_bHideIfZero == iwdgOther.m_bHideIfZero)
        && (m_i4Offset == iwdgOther.m_i4Offset)
        && (m_tmCurTime.strDefaultFormat() == iwdgOther.m_tmCurTime.strDefaultFormat())
    );
}


tCIDLib::TVoid
TCQCIntfTimeTextBase::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // If in designer mode, generate a default value for viewing
    if (TFacCQCIntfEng::bDesMode())
    {
        m_tmCurTime.SetToNow();
        m_tmCurTime.FormatToStr(m_strValue);
    }
}


// Return configuration summary info about us
tCIDLib::TVoid
TCQCIntfTimeTextBase::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove any base attributes we don't honor
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Set any kept color names that aren't already defaulted correctly
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, &strCaption());

    // We actually don't use the caption, we use generated text, so remove it
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Widget_Caption);

    colAttrs.objPlace
    (
        L"Time Text Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // Do our stuff
    colAttrs.objPlace
    (
        L"Format"
        , kCQCIntfEng::strAttr_TimeText_Format
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    ).SetString(m_tmCurTime.strDefaultFormat());

    colAttrs.objPlace
    (
        L"Hide If Zero"
        , kCQCIntfEng::strAttr_TimeText_HideIfZ
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetBool(m_bHideIfZero);

    colAttrs.objPlace
    (
        L"Offset"
        , kCQCIntfEng::strAttr_TimeText_Offset
        , L"Range: -12, 11"
        , tCIDMData::EAttrTypes::Int
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetInt(m_i4Offset);
}


// We just return a size for a short string in the set font
TSize TCQCIntfTimeTextBase::szDefaultSize() const
{
    // Base on the default text with the current font
    TGraphDesktopDev gdevToUse;
    TCQCIntfFontJan janFont(&gdevToUse, *this);
    TArea areaText;
    areaText = gdevToUse.areaString(L"Some Sample Text", kCIDLib::False);
    areaText.AdjustSize(16, 4);
    return areaText.szArea();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfTimeTextBase::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);

    // And do our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_TimeText_Format)
        strFormat(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TimeText_HideIfZ)
        m_bHideIfZero = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TimeText_Offset)
        m_i4Offset = adatNew.i4Val();
}


// Validate any of our configuration
tCIDLib::TVoid
TCQCIntfTimeTextBase::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);
}



// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the 'hide if zero' value
tCIDLib::TBoolean TCQCIntfTimeTextBase::bHideIfZero() const
{
    return m_bHideIfZero;
}

tCIDLib::TBoolean
TCQCIntfTimeTextBase::bHideIfZero(const tCIDLib::TBoolean bToSet)
{
    m_bHideIfZero = bToSet;
    return m_bHideIfZero;
}


// Get or set the hour offset value
tCIDLib::TInt4 TCQCIntfTimeTextBase::i4Offset() const
{
    return m_i4Offset;
}

tCIDLib::TInt4 TCQCIntfTimeTextBase::i4Offset(const tCIDLib::TInt4 i4ToSet)
{
    m_i4Offset = i4ToSet;
    return m_i4Offset;
}


// Get or set our format string
const TString& TCQCIntfTimeTextBase::strFormat() const
{
    return m_tmCurTime.strDefaultFormat();
}

const TString& TCQCIntfTimeTextBase::strFormat(const TString& strToSet)
{
    // Update the format. If in designer mode, update our dummy text
    m_tmCurTime.strDefaultFormat(strToSet);
    if (TFacCQCIntfEng::bDesMode())
    {
        try
        {
            m_tmCurTime.SetToNow();
            m_tmCurTime.FormatToStr(m_strValue);
        }

        catch(...)
        {
            m_strValue = facCQCIntfEng().strErrText();
            m_tmCurTime = 0;
        }
    }
    return m_tmCurTime.strDefaultFormat();
}



// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Hidden Constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfTimeTextBase::
TCQCIntfTimeTextBase(const  tCQCIntfEng::ECapFlags  eCapflags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapflags, strTypeName)
    , m_bHideIfZero(kCIDLib::False)
    , m_i4Offset(0)
{
    // Set an initial format
    m_tmCurTime.strDefaultFormat(TTime::strCTime());

    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    rgbExtra(kCQCIntfEng_::rgbDef_EmphText);
    rgbExtra2(kCQCIntfEng_::rgbDef_EmphTextShadow);

    // Set the action command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/TiomeDateText");
}

TCQCIntfTimeTextBase::
TCQCIntfTimeTextBase(const TCQCIntfTimeTextBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , m_bHideIfZero(iwdgSrc.m_bHideIfZero)
    , m_i4Offset(iwdgSrc.m_i4Offset)
    , m_tmCurTime(iwdgSrc.m_tmCurTime)
{
    // We don't copy the value string, which is a purely runtime thing
    m_strValue.Clear();
}

TCQCIntfTimeTextBase&
TCQCIntfTimeTextBase::operator=(const TCQCIntfTimeTextBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        m_bHideIfZero   = iwdgSrc.m_bHideIfZero;
        m_i4Offset      = iwdgSrc.m_i4Offset;
        m_tmCurTime     = iwdgSrc.m_tmCurTime;

        //
        //  The value string is a purely runtime thing, but make sure we have
        //  something to display in designer mode.
        //
        m_strValue.Clear();

        // If in designer mode, generate a default value for viewing
        if (TFacCQCIntfEng::bDesMode())
        {
            m_tmCurTime.SetToNow();
            m_tmCurTime.FormatToStr(m_strValue);
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Protected, inherited methods
// ---------------------------------------------------------------------------

// Stream ourself in and out
tCIDLib::TVoid TCQCIntfTimeTextBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeText::c2BaseFmtVersion))
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

    // Do our parent class
    TParent::StreamFrom(strmToReadFrom);

    // Stream in any mixin's stuff
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    TString strFormat;
    strmToReadFrom  >> strFormat
                    >> m_i4Offset
                    >> m_bHideIfZero;

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

    // Store the default format
    m_tmCurTime.strDefaultFormat(strFormat);

    // If in designer mode, generate a default value for display
    if (TFacCQCIntfEng::bDesMode())
    {
        m_tmCurTime.SetToNow();
        m_tmCurTime.FormatToStr(m_strValue);
    }
}


tCIDLib::TVoid TCQCIntfTimeTextBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_TimeText::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << m_tmCurTime.strDefaultFormat()
                    << m_i4Offset
                    << m_bHideIfZero
                    << tCIDLib::EStreamMarkers::EndObject;
}


// This is called when we need to redraw some part of ourself
tCIDLib::TVoid
TCQCIntfTimeTextBase::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Move in a bit more, clip and udpate
    grgnClip.Deflate(3, 2);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    DrawText(gdevTarget, areaInvalid, areaDraw);
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes call this to tell us what time to display
tCIDLib::TVoid
TCQCIntfTimeTextBase::SetTime(  const   tCIDLib::TEncodedTime   enctToSet
                                , const tCIDLib::TBoolean       bNoRedraw
                                , const tCIDLib::TBoolean       bInError)
{
    const tCIDLib::TBoolean bCanRedraw
    (
        (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw
    );

    // Remember if our current time set was zero or not
    const tCIDLib::TBoolean bWasZero = (m_tmCurTime.enctTime() == 0);

    if (bInError)
    {
        m_strTmpFmt = facCQCIntfEng().strErrText();
        m_tmCurTime = 0;
    }
     else
    {
        try
        {
            // Store the time and format it out with offset if we have any
            m_tmCurTime = enctToSet;
            if (m_i4Offset)
                m_tmCurTime += (m_i4Offset * kCIDLib::enctOneHour);
            m_tmCurTime.FormatToStr(m_strTmpFmt, m_tmCurTime.strDefaultFormat());
        }

        catch(...)
        {
            m_strTmpFmt = facCQCIntfEng().strErrText();
            m_tmCurTime = 0;
        }
    }

    //
    //  If the value changed or we are a hide if zero and we transitioned
    //  from a non-zero value to a zero value, then redraw unless hidden or
    //  asked not to.
    //
    if ((m_strTmpFmt != m_strValue)
    ||  (m_bHideIfZero && !m_tmCurTime.enctTime() && !bWasZero))
    {
        m_strValue = m_strTmpFmt;
        if (bCanRedraw)
            Redraw();
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfTimeTextBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handles the actual drawing of our text
tCIDLib::TVoid
TCQCIntfTimeTextBase::DrawText(         TGraphDrawDev&      gdevTarget
                                , const TArea&              areaInvalid
                                ,       TArea&              areaDraw)
{
    //
    //  If our hide if zero state would keep us from showing up, we are done.
    //  We still would do any background fill, but won't show any time.
    //
    if (m_bHideIfZero && !m_tmCurTime.enctTime())
        return;

    //
    //  If the text area intersects the invalid area, then draw it. If we have
    //  no text, which can happen if the time is zero and we were told not to
    //  display zero times, then we don't have to bother doing any more.
    //
    if (areaDraw.bIntersects(areaInvalid) && !m_strValue.bIsEmpty())
    {
        // Set the new clip on the text area
        TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);

        // Set our font
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        // Drawn the text now, based on front settings
        gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            // Do an effects based draw
            civOwner().DrawTextFX
            (
                gdevTarget
                , m_strValue
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
                        m_strValue, areaDrop, eHJustify(), eVJustify()
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        m_strValue
                        , areaDrop
                        , eHJustify()
                        , eVJustify()
                        , kCIDLib::True
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
                    m_strValue, areaDraw, eHJustify(), eVJustify()
                );
            }
             else
            {
                gdevTarget.DrawMText
                (
                    m_strValue, areaDraw, eHJustify(), eVJustify(), kCIDLib::True
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
//   CLASS: TCQCIntfFieldTimeText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFieldTimeText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFieldTimeText::TCQCIntfFieldTimeText() :

    TCQCIntfTimeTextBase
    (
        CQCIntfEng_TimeText::eFieldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_TimeText)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltTypeName
          (
              tCQCKit::EReqAccess::MReadCWrite, tCQCKit::EFldTypes::Time
          )
      )
{
}

TCQCIntfFieldTimeText::
TCQCIntfFieldTimeText(const TCQCIntfFieldTimeText& iwdgToCopy) :

    TCQCIntfTimeTextBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldTimeText: Public operator
// ---------------------------------------------------------------------------
TCQCIntfFieldTimeText&
TCQCIntfFieldTimeText::operator=(const TCQCIntfFieldTimeText& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldTimeText: Public, inherited mehods
// ---------------------------------------------------------------------------

//
//  This is called to see if can accept, and optionally update ourself, to a
//  new field. In our case we just check it's a time field, and we don't need
//  to do anything for a store.
//
tCIDLib::TBoolean
TCQCIntfFieldTimeText::bCanAcceptField( const   TCQCFldDef&     fldTest
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    return (fldTest.eType() == tCQCKit::EFldTypes::Time);
}


tCIDLib::TBoolean TCQCIntfFieldTimeText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFieldTimeText& iwdgOther(static_cast<const TCQCIntfFieldTimeText&>(iwdgSrc));

    // We only have the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfFieldTimeText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFieldTimeText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFieldTimeText::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
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
TCQCIntfFieldTimeText::QueryCmds(       TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFieldTimeText::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


// Return configuration summary info about us
tCIDLib::TVoid
TCQCIntfFieldTimeText::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Time Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldTimeText::RegisterFields(          TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFieldTimeText::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFieldTimeText::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  At this level, we just call our parent and any mixins, to let them/
//  do any required validation.
//
tCIDLib::TVoid
TCQCIntfFieldTimeText::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixines
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldTimeText: Protected, inherited mehods
// ---------------------------------------------------------------------------

//
//  If our field mixin sees a change in the state or value or value, then we
//  need to update, so he'll call us here.
//
tCIDLib::TVoid
TCQCIntfFieldTimeText::FieldChanged(        TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        SetTime
        (
            static_cast<const TCQCTimeFldValue&>(fvCurrent()).c8Value()
            , bNoRedraw
            , kCIDLib::False
        );
    }
     else
    {
        SetTime(0, bNoRedraw, kCIDLib::True);
    }
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfFieldTimeText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker and version
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;

    // Check the format version
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeText::c2FldFmtVersion))
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
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFieldTimeText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_TimeText::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any other stuff right now, so just the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldTimeText::ValueUpdate(         TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLocalTimeText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfLocalTimeText::TCQCIntfLocalTimeText() :

    TCQCIntfTimeTextBase
    (
        CQCIntfEng_TimeText::eLocalCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_TimeText)
    )
{
}

TCQCIntfLocalTimeText::TCQCIntfLocalTimeText(const TCQCIntfLocalTimeText& iwdgToCopy) :

    TCQCIntfTimeTextBase(iwdgToCopy)
{
}


// ---------------------------------------------------------------------------
//  Public operator
// ---------------------------------------------------------------------------
TCQCIntfLocalTimeText&
TCQCIntfLocalTimeText::operator=(const TCQCIntfLocalTimeText& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeText: Public, inherited mehods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfLocalTimeText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfLocalTimeText& iwdgOther(static_cast<const TCQCIntfLocalTimeText&>(iwdgSrc));

    // Nothing of our own yet
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfLocalTimeText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfLocalTimeText&>(iwdgSrc);
}


// Return configuration summary info about us
tCIDLib::TVoid
TCQCIntfLocalTimeText::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // For now nothing of our own
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfLocalTimeText::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // For now nothing of our own
}


// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeText: Protected, inherited mehods
// ---------------------------------------------------------------------------

// We just set the current time on our parent class
tCIDLib::TVoid
TCQCIntfLocalTimeText::ActiveUpdate(const   tCIDLib::TBoolean bNoRedraw
                                    , const TGUIRegion&
                                    , const tCIDLib::TBoolean bInTopLayer)
{
    SetTime(TTime::enctNow(), bNoRedraw, kCIDLib::False);
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfLocalTimeText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker and version
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;

    // Check the format version
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeText::c2LocalFmtVersion))
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

    // We don't have anything of our own right now

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfLocalTimeText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_TimeText::c2LocalFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // We don't have any other stuff right now, so just the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}




