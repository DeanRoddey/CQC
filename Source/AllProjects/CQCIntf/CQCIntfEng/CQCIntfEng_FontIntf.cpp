//
// FILE NAME: CQCIntfEng_FontIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/26/2002
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
//  This file implements the font attributes mixin. Though we are a mixin, we
//  really don't provide any virtual methods, we provide concrete functionality
//  for the classes that mix us in.
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
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_FontIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 1 -
        //      Reset to 1 for 2.0 since it has to do a manual conversion during
        //      the install anyway.
        //
        //  Version 2 -
        //      Added support for text direction
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The name under which we put our attributes into an attribute pack
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszAttrPackKey = L"Font Attributes";
    }
}


// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfFontIntf
//  PREFIX: miwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfFontIntf: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCIntfFontIntf::MCQCIntfFontIntf()
{
    // Just call the rest method, so we can keep it all in one place
    ResetFont();
}

MCQCIntfFontIntf::MCQCIntfFontIntf(const MCQCIntfFontIntf& miwdgToCopy) :

    m_c4Flags(miwdgToCopy.m_c4Flags)
    , m_c4FontHeight(miwdgToCopy.m_c4FontHeight)
    , m_eDir(miwdgToCopy.m_eDir)
    , m_eEffect(miwdgToCopy.m_eEffect)
    , m_eHJustify(miwdgToCopy.m_eHJustify)
    , m_eVJustify(miwdgToCopy.m_eVJustify)
    , m_pgfontText(0)
    , m_pntOffset(miwdgToCopy.m_pntOffset)
    , m_strFaceName(miwdgToCopy.m_strFaceName)
{
}

MCQCIntfFontIntf::~MCQCIntfFontIntf()
{
    delete m_pgfontText;
}


// ---------------------------------------------------------------------------
//  MCQCIntfFontIntf: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TVoid
MCQCIntfFontIntf::operator=(const MCQCIntfFontIntf& miwdgToAssign)
{
    if (&miwdgToAssign != this)
    {
        m_c4Flags       = miwdgToAssign.m_c4Flags;
        m_c4FontHeight  = miwdgToAssign.m_c4FontHeight;
        m_eDir          = miwdgToAssign.m_eDir;
        m_eEffect       = miwdgToAssign.m_eEffect;
        m_eHJustify     = miwdgToAssign.m_eHJustify;
        m_eVJustify     = miwdgToAssign.m_eVJustify;
        m_pntOffset     = miwdgToAssign.m_pntOffset;
        m_strFaceName   = miwdgToAssign.m_strFaceName;

        // Force the font to fault back in
        delete m_pgfontText;
        m_pgfontText = 0;
    }
}


// ---------------------------------------------------------------------------
//  MCQCIntfFontIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean MCQCIntfFontIntf::bBold() const
{
    return (m_c4Flags & kCQCIntfEng::c4FontFlag_Bold) != 0;
}

tCIDLib::TBoolean MCQCIntfFontIntf::bBold(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
        m_c4Flags |= kCQCIntfEng::c4FontFlag_Bold;
    else
        m_c4Flags &= ~kCQCIntfEng::c4FontFlag_Bold;

    // Force an update of the font
    delete m_pgfontText;
    m_pgfontText = 0;

    return bToSet;
}


tCIDLib::TBoolean MCQCIntfFontIntf::bItalic() const
{
    return (m_c4Flags & kCQCIntfEng::c4FontFlag_Italic) != 0;
}

tCIDLib::TBoolean MCQCIntfFontIntf::bItalic(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
        m_c4Flags |= kCQCIntfEng::c4FontFlag_Italic;
    else
        m_c4Flags &= ~kCQCIntfEng::c4FontFlag_Italic;

    // Force an update of the font
    delete m_pgfontText;
    m_pgfontText = 0;

    return bToSet;
}


tCIDLib::TBoolean MCQCIntfFontIntf::bNoTextWrap() const
{
    return (m_c4Flags & kCQCIntfEng::c4FontFlag_NoWrap) != 0;
}

tCIDLib::TBoolean MCQCIntfFontIntf::bNoTextWrap(const tCIDLib::TBoolean bToSet)
{
    if (bToSet)
        m_c4Flags |= kCQCIntfEng::c4FontFlag_NoWrap;
    else
        m_c4Flags &= ~kCQCIntfEng::c4FontFlag_NoWrap;
    return bToSet;
}


tCIDLib::TBoolean
MCQCIntfFontIntf::bSameFont(const MCQCIntfFontIntf& miwdgSrc) const
{
    if ((m_c4Flags       != miwdgSrc.m_c4Flags)
    ||  (m_c4FontHeight  != miwdgSrc.m_c4FontHeight)
    ||  (m_eDir          != miwdgSrc.m_eDir)
    ||  (m_eEffect       != miwdgSrc.m_eEffect)
    ||  (m_eHJustify     != miwdgSrc.m_eHJustify)
    ||  (m_eVJustify     != miwdgSrc.m_eVJustify)
    ||  (m_pntOffset     != miwdgSrc.m_pntOffset)
    ||  (m_strFaceName   != miwdgSrc.m_strFaceName))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TCard4 MCQCIntfFontIntf::c4FontHeight() const
{
    return m_c4FontHeight;
}

tCIDLib::TCard4
MCQCIntfFontIntf::c4FontHeight(const tCIDLib::TCard4 c4NewHeight)
{
    // If it changed, then force an update of the font
    if (c4NewHeight != m_c4FontHeight)
    {
        m_c4FontHeight = c4NewHeight;
        delete m_pgfontText;
        m_pgfontText = 0;
    }
    return m_c4FontHeight;
}


// Get or set the font direction
tCIDGraphDev::EFontDirs MCQCIntfFontIntf::eDir() const
{
    return m_eDir;
}

tCIDGraphDev::EFontDirs MCQCIntfFontIntf::eDir(const tCIDGraphDev::EFontDirs eToSet)
{
    m_eDir = eToSet;
    return m_eDir;
}


// Get or set the horizontal justification member
tCIDLib::EHJustify MCQCIntfFontIntf::eHJustify() const
{
    return m_eHJustify;
}

tCIDLib::EHJustify
MCQCIntfFontIntf::eHJustify(const tCIDLib::EHJustify eToSet)
{
    m_eHJustify = eToSet;
    return m_eHJustify;
}


// Get or set the text effect member
tCIDGraphDev::ETextFX MCQCIntfFontIntf::eTextFX() const
{
    return m_eEffect;
}

tCIDGraphDev::ETextFX MCQCIntfFontIntf::eTextFX(const tCIDGraphDev::ETextFX eToSet)
{
    m_eEffect = eToSet;
    return m_eEffect;
}


// Get or set the veritcal justification member
tCIDLib::EVJustify MCQCIntfFontIntf::eVJustify() const
{
    return m_eVJustify;
}

tCIDLib::EVJustify MCQCIntfFontIntf::eVJustify(const tCIDLib::EVJustify eToSet)
{
    m_eVJustify = eToSet;
    return m_eVJustify;
}


// Get the font that represents this object's settings, faulting in if needed
TGUIFont& MCQCIntfFontIntf::gfontText(TGraphDrawDev&) const
{
    // Fault in the font if required
    if (!m_pgfontText)
    {
        //
        //  Set up a font selection attribute object, which reflects our
        //  current settings
        //
        TFontSelAttrs fselCur;
        QueryFontSelAttrs(fselCur);

        m_pgfontText = new TGUIFont(fselCur);
    }
    return *m_pgfontText;
}


// Get or set the text effect offset
TPoint& MCQCIntfFontIntf::pntOffset()
{
    return m_pntOffset;
}

const TPoint& MCQCIntfFontIntf::pntOffset() const
{
    return m_pntOffset;
}

TPoint& MCQCIntfFontIntf::pntOffset(const TPoint& pntToSet)
{
    m_pntOffset = pntToSet;
    return m_pntOffset;
}


tCIDLib::TVoid
MCQCIntfFontIntf::QueryFontAttrs(       tCIDMData::TAttrList&   colAttrs
                                ,       TAttrData&              adatTmp
                                , const TString* const          pstrCaption
                                , const tCIDLib::TBoolean       bMultiLine) const
{
    adatTmp.Set(L"Text Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    //
    //  The caption text if they indicate this one wants it. Some have special
    //  requirements and provide the text themselves or dynamically.
    //
    if (pstrCaption)
    {
        // If they don't indicate it can be multi-line, set single line limit
        TString strLim;
        if (!bMultiLine)
            strLim = kCIDMData::strAttrLim_SingleLine;

        adatTmp.Set
        (
            L"Text"
            , kCQCIntfEng::strAttr_Widget_Caption
            , strLim
            , tCIDMData::EAttrTypes::String
            , tCIDMData::EAttrEdTypes::Both
        );
        adatTmp.SetString(*pstrCaption);
        colAttrs.objAdd(adatTmp);
    }

    // The font face
    adatTmp.Set
    (
        L"Face"
        , kCQCIntfEng::strAttr_Font_Face
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Visual
    );
    adatTmp.SetString(m_strFaceName);
    colAttrs.objAdd(adatTmp);

    // The text height
    adatTmp.Set
    (
        L"Height"
        , kCQCIntfEng::strAttr_Font_Height
        , L"Range: 4, 92"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(m_c4FontHeight);
    colAttrs.objAdd(adatTmp);


    // The various flags
    adatTmp.Set
    (
        L"Bold", kCQCIntfEng::strAttr_Font_Bold, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(bBold());
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Italic", kCQCIntfEng::strAttr_Font_Italic, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(bItalic());
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"No Wrap", kCQCIntfEng::strAttr_Font_NoWrap, tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(bNoTextWrap());
    colAttrs.objAdd(adatTmp);


    // Horizontal justification
    TString strLims(L"Enum: ");
    {
        tCIDLib::EHJustify eVal = tCIDLib::EHJustify::Min;
        while (eVal <= tCIDLib::EHJustify::Max)
        {
            if (eVal > tCIDLib::EHJustify::Min)
                strLims.Append(L", ");
            strLims.Append(tCIDLib::pszAltXlatEHJustify(eVal));
            eVal++;
        }
    }
    adatTmp.Set
    (
        L"Horz Justify"
        , kCQCIntfEng::strAttr_Font_HJustify
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCIDLib::pszAltXlatEHJustify(m_eHJustify));
    colAttrs.objAdd(adatTmp);

    // Vertical justification
    strLims = L"Enum: ";
    {
        tCIDLib::EVJustify eVal = tCIDLib::EVJustify::Min;
        while (eVal <= tCIDLib::EVJustify::Max)
        {
            if (eVal > tCIDLib::EVJustify::Min)
                strLims.Append(L", ");
            strLims.Append(tCIDLib::pszAltXlatEVJustify(eVal));
            eVal++;
        }
    }
    adatTmp.Set
    (
        L"Vert Justify"
        , kCQCIntfEng::strAttr_Font_VJustify
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCIDLib::pszAltXlatEVJustify(m_eVJustify));
    colAttrs.objAdd(adatTmp);


    tCIDGraphDev::FormatETextFX(strLims, L"Enum: ", kCIDLib::chComma);
    adatTmp.Set
    (
        L"Effect"
        , kCQCIntfEng::strAttr_Font_Effect
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCIDGraphDev::strXlatETextFX(m_eEffect));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Effect Ofs"
        , kCQCIntfEng::strAttr_Font_EffectOfs
        , L"Area: -20, -20, 40, 40"
        , tCIDMData::EAttrTypes::Point
    );
    adatTmp.SetPoint(m_pntOffset);
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
MCQCIntfFontIntf::QueryFontSelAttrs(TFontSelAttrs& fselToFill) const
{
    fselToFill.bScalable(kCIDLib::True);
    fselToFill.c4Height(m_c4FontHeight);
    fselToFill.bItalic(bItalic());
    fselToFill.eDir(m_eDir);

    if (bBold())
        fselToFill.eWeight(tCIDGraphDev::EFontWeights::Bold);
    else
        fselToFill.eWeight(tCIDGraphDev::EFontWeights::Normal);

    if (!m_strFaceName.bIsEmpty())
        fselToFill.strFaceName(m_strFaceName);
}


// Stream ourself in from the passed stream
tCIDLib::TVoid MCQCIntfFontIntf::ReadInFont(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_FontIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfFontIntf")
        );
    }

    // Looks ok, so stream in our stuff
    strmToReadFrom  >> m_c4Flags
                    >> m_c4FontHeight
                    >> m_eHJustify
                    >> m_eVJustify
                    >> m_strFaceName
                    >> m_eEffect
                    >> m_pntOffset;

    // If V2 or beyond read in the direction, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_eDir;
    else
        m_eDir = tCIDGraphDev::EFontDirs::D0;

    // And it should all end with another frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid MCQCIntfFontIntf::ResetFont()
{
    m_c4Flags       = 0;
    m_c4FontHeight  = 12;
    m_eDir          = tCIDGraphDev::EFontDirs::D0;
    m_eEffect       = tCIDGraphDev::ETextFX::None;
    m_eHJustify     = tCIDLib::EHJustify::Left;
    m_eVJustify     = tCIDLib::EVJustify::Center;
    m_pgfontText    = 0;
    m_pntOffset.Zero();
    m_strFaceName   = L"Verdana";
}


const TString& MCQCIntfFontIntf::strFaceName() const
{
    return m_strFaceName;
}

const TString& MCQCIntfFontIntf::strFaceName(const TString& strToSet)
{
    // If it changed, then force an update of the font
    if (m_strFaceName != strToSet)
    {
        m_strFaceName = strToSet;
        delete m_pgfontText;
        m_pgfontText = 0;
    }
    return m_strFaceName;
}


// Called during font scalling if the widget implements our interface
tCIDLib::TVoid
MCQCIntfFontIntf::ScaleFont(const   tCIDLib::TFloat8    f8XScale
                            , const tCIDLib::TFloat8    f8YScale)
{
    m_c4FontHeight = TMathLib::c4Round(m_c4FontHeight * f8YScale);
    m_pntOffset.Scale(f8XScale, f8YScale);
}


// Use the SETTER METHODS because they update the font for the next redraw
tCIDLib::TVoid
MCQCIntfFontIntf::SetFontAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&)
{
    // -------------------
    //  The core font flags and face stuff
    // -------------------
    if (adatNew.strId() == kCQCIntfEng::strAttr_Font_Bold)
        bBold(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_Italic)
        bItalic(adatNew.bVal());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_Face)
        strFaceName(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_Height)
        c4FontHeight(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_NoWrap)
        bNoTextWrap(adatNew.bVal());

    // -------------------
    //  Horz/vertical justification
    // -------------------
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_HJustify)
        eHJustify(tCIDLib::eAltXlatEHJustify(adatNew.strValue()));
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_VJustify)
        eVJustify(tCIDLib::eAltXlatEVJustify(adatNew.strValue()));

    // -------------------
    //  Effects stuff
    // -------------------
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_Effect)
        m_eEffect = tCIDGraphDev::eXlatETextFX(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Font_EffectOfs)
        m_pntOffset = adatNew.pntVal();
}


tCIDLib::TVoid
MCQCIntfFontIntf::WriteOutFont(TBinOutStream& strmToWriteTo) const
{
    // Frame our content, and stream out out a version format value
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_FontIntf::c2FmtVersion
                    << m_c4Flags
                    << m_c4FontHeight
                    << m_eHJustify
                    << m_eVJustify
                    << m_strFaceName
                    << m_eEffect
                    << m_pntOffset

                    // V2 stuff
                    << m_eDir

                    << tCIDLib::EStreamMarkers::Frame;
}



