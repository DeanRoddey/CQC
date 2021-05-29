//
// FILE NAME: CQCIntfEng_BooleanText.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/13/2002
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
//  This file implements a derivative of the base single line text display
//  widget, which displays one of two text strings, driven by the boolean
//  state of the field it is attached to.
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
#include    "CQCIntfEng_BooleanText.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfBooleanTextBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldBooleanText,TCQCIntfBooleanTextBase)
AdvRTTIDecls(TCQCIntfVarBooleanText,TCQCIntfBooleanTextBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_BooleanText
    {
        // -----------------------------------------------------------------------
        //  The base class persistent format version
        //
        //  Version 2 -
        //      We added the m_bUseExtraClrs flag to allow for state based text
        //      colors.
        //
        //  Version 3 -
        //      No data change but we need to change some colors around to make
        //      our usage of them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  The field based bool text format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The var based bool text format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion = 1;



        // -----------------------------------------------------------------------
        //  Our capabilities flags. We use the same ones for field and variable
        //  based versions.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfBooleanTextBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfBooleanTextBase::~TCQCIntfBooleanTextBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfBooleanTextBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfBooleanTextBase& iwdgOther
    (
        static_cast<const TCQCIntfBooleanTextBase&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfBooleanIntf::bSameBool(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfBooleanTextBase::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    // If we have any mixins that do their own commands, give them a chance
    if (MCQCIntfBooleanIntf::bDoBoolCmd(ccfgToDo, c4Index))
        return tCQCKit::ECmdRes::Ok;

    // Pass on to our parent
    return TParent::eDoCmd(ccfgToDo, c4Index, strUserId, strEventId, ctarGlobals, bResult, acteTar);
}


tCIDLib::TVoid TCQCIntfBooleanTextBase::
Initialize( TCQCIntfContainer* const    piwdgParent
            ,       TDataSrvClient&     dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If we are in Designer state, then force the state to the false
    //  state, so that we'll draw one of the required images.
    //
    if (TFacCQCIntfEng::bDesMode())
        m_eCurState = tCQCIntfEng::EBoolStates::False;
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfBooleanTextBase::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Do any mixins that have their own commands
    MCQCIntfBooleanIntf::QueryBoolCmds(colCmds, eContext);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfBooleanTextBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't support
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Update the names of any colors we keep that don't default to our needs
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"True Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"True Shadow/FX");

    // Add our attributes
    adatTmp.Set(L"Text Options:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"False Text"
        , kCQCIntfEng::strAttr_Bool_FalseText
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strFalseText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"True Text"
        , kCQCIntfEng::strAttr_Bool_TrueText
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strTrueText);
    colAttrs.objAdd(adatTmp);

    // Do our mixins
    MCQCIntfBooleanIntf::QueryBoolAttrs(colAttrs, adatTmp);
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr);

    adatTmp.Set
    (
        L"State Based Colors"
        , kCQCIntfEng::strAttr_Bool_StateBasedClrs
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bUseExtraClrs);
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfBooleanTextBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Expressions))
        MCQCIntfBooleanIntf::ReplaceBool(strSrc, strTar, bRegEx, bFull, regxFind);

    // The caption in this case is our false/true text
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Caption))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strFalseText, bRegEx, bFull, regxFind
        );
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strTrueText, bRegEx, bFull, regxFind
        );
    }
}


TSize TCQCIntfBooleanTextBase::szDefaultSize() const
{
    // Base on the default text with the current font
    tCQCIntfEng::TGraphIntfDev cptrpTmp = civOwner().cptrNewGraphDev();

    TCQCIntfFontJan janFont(cptrpTmp.pobjData(), *this);
    TArea areaText = cptrpTmp->areaString(m_strFalseText, kCIDLib::False);
    areaText.AdjustSize(16, 4);
    return areaText.szArea();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfBooleanTextBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_StateBasedClrs)
        m_bUseExtraClrs = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_FalseText)
        m_strFalseText = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_TrueText)
        m_strTrueText = adatNew.strValue();

    // Do our mixins
    MCQCIntfBooleanIntf::SetBoolAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfBooleanTextBase::Validate(  const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the user extra colors flag
tCIDLib::TBoolean TCQCIntfBooleanTextBase::bUseExtraClrs() const
{
    return m_bUseExtraClrs;
}


tCIDLib::TBoolean
TCQCIntfBooleanTextBase::bUseExtraClrs(const tCIDLib::TBoolean bToSet)
{
    m_bUseExtraClrs = bToSet;
    return m_bUseExtraClrs;
}


// Get/set the false and true text
const TString& TCQCIntfBooleanTextBase::strFalseText() const
{
    return m_strFalseText;
}

const TString& TCQCIntfBooleanTextBase::strFalseText(const TString& strToSet)
{
    m_strFalseText = strToSet;
    return m_strFalseText;
}


const TString& TCQCIntfBooleanTextBase::strTrueText() const
{
    return m_strTrueText;
}

const TString& TCQCIntfBooleanTextBase::strTrueText(const TString& strToSet)
{
    m_strTrueText = strToSet;
    return m_strTrueText;
}


// Set both text values at once
tCIDLib::TVoid
TCQCIntfBooleanTextBase::Set(const  TString&    strFalseText
                            , const TString&    strTrueText)
{
    m_strFalseText = strFalseText;
    m_strTrueText = strTrueText;
}



// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfBooleanTextBase::
TCQCIntfBooleanTextBase(const   tCQCIntfEng::ECapFlags  eCaps
                        , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
    , m_bUseExtraClrs(kCIDLib::False)
    , m_eCurState(tCQCIntfEng::EBoolStates::Error)
    , m_strFalseText(L"False Text")
    , m_strTrueText(L"True Text")
{
    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    rgbExtra(kCQCIntfEng_::rgbDef_EmphText);
    rgbExtra2(kCQCIntfEng_::rgbDef_EmphTextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/BooleanText");
}

TCQCIntfBooleanTextBase::
TCQCIntfBooleanTextBase(const TCQCIntfBooleanTextBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfBooleanIntf(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , m_bUseExtraClrs(iwdgSrc.m_bUseExtraClrs)
    , m_eCurState(iwdgSrc.m_eCurState)
    , m_strFalseText(iwdgSrc.m_strFalseText)
    , m_strTrueText(iwdgSrc.m_strTrueText)
{
}


TCQCIntfBooleanTextBase&
TCQCIntfBooleanTextBase::operator=(const TCQCIntfBooleanTextBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfBooleanIntf::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);

        m_bUseExtraClrs = iwdgSrc.m_bUseExtraClrs;
        m_eCurState     = iwdgSrc.m_eCurState;
        m_strFalseText  = iwdgSrc.m_strFalseText;
        m_strTrueText   = iwdgSrc.m_strTrueText;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfBooleanTextBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should start with a start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanText::c2BaseFmtVersion))
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
    MCQCIntfBooleanIntf::ReadInBool(strmToReadFrom);
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);

    // And now stream in our own stuff now
    strmToReadFrom  >> m_strFalseText
                    >> m_strTrueText;

    // If version 2 or beyond, read in the use extra colors flag, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bUseExtraClrs;
    else
        m_bUseExtraClrs = kCIDLib::False;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime stuff
    m_eCurState = tCQCIntfEng::EBoolStates::Error;

    //
    //  If V1 or 2, then we need to adjust some colors, so that the F2 color
    //  is always used for shadow/fx. Currently F3 is being used for shadow
    //  and F2 for FX, and F2 is also being used for the border.
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
}


tCIDLib::TVoid
TCQCIntfBooleanTextBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_BooleanText::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream out the mixin's stuff
    MCQCIntfBooleanIntf::WriteOutBool(strmToWriteTo);
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << m_strFalseText
                    << m_strTrueText
                    << m_bUseExtraClrs
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfBooleanTextBase::Update(        TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    // Let our parent handle the standard stuff
    TParent::Update(gdevTarget, areaInvalid, grgnClip);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // And now draw our text
    DrawText(gdevTarget, areaInvalid, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes tell us what our current value should
tCIDLib::TVoid
TCQCIntfBooleanTextBase::SetState(  const   tCQCIntfEng::EBoolStates   eToSet
                                    , const tCIDLib::TBoolean           bNoRedraw)
{
    if (m_eCurState != eToSet)
    {
        m_eCurState = eToSet;
        if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
            Redraw();
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanTextBase: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfBooleanTextBase::DrawText(          TGraphDrawDev&      gdevTarget
                                    , const TArea&              areaInvalid
                                    , const tCIDLib::TBoolean   bDrawBorder)
{
    // If we are hidden, then do nothing
    if (eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
        return;

    // Draw the border if needed, and adjust the drawing area
    TArea areaDraw(areaActual());
    if (bHasBorder())
    {
        if (bDrawBorder)
            gdevTarget.Stroke(areaDraw, rgbFgn3());
        areaDraw.Deflate(1);
    }

    // Set a clip area and do the background fill if needed
    if (!bIsTransparent())
    {
        TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);
        DoBgnFill(gdevTarget, areaInvalid);
    }

    // Now move the clip in further for the drawing part and set a new clip
    areaDraw.Deflate(3, 2);
    TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);

    // To avoid redundancy below, set up a flag for good value
    const tCIDLib::TBoolean bGoodValue(m_eCurState != tCQCIntfEng::EBoolStates::Error);

    //
    //  Set up the colors we'll use. Since we can use the extra colors for
    //  True state, the color we use depends on our state and whether we are
    //  configured to use state based text colors.
    //
    const TRGBClr* prgbText;
    const TRGBClr* prgbText2;
    if (m_bUseExtraClrs && (m_eCurState == tCQCIntfEng::EBoolStates::True))
    {
        prgbText = &rgbExtra();
        prgbText2 = &rgbExtra2();
    }
     else
    {
        prgbText = &rgbFgn();
        prgbText2 = &rgbFgn2();
    }

    // If this intersects the invalid area, then draw the text
    if (areaDraw.bIntersects(areaInvalid))
    {
        // Set our font
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        // Decide what to draw
        const TString* pstrToDraw = 0;
        if (bGoodValue)
        {
            if (m_eCurState == tCQCIntfEng::EBoolStates::True)
                pstrToDraw = &m_strTrueText;
            else
                pstrToDraw = &m_strFalseText;
        }
         else
        {
            pstrToDraw = &facCQCIntfEng().strErrText();
        }

        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civOwner().DrawTextFX
            (
                gdevTarget
                , *pstrToDraw
                , areaDraw
                , eTextFX()
                , *prgbText
                , *prgbText2
                , eHJustify()
                , eVJustify()
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            // If no good value, show disabled
            tCIDGraphDev::ETextFmts eFmt = tCIDGraphDev::ETextFmts::None;
            if (!bGoodValue)
                eFmt = tCIDGraphDev::ETextFmts::Disabled;

            // Now just draw our text
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaDraw);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(*prgbText2);
                gdevTarget.DrawString
                (
                    *pstrToDraw, areaDrop, eHJustify(), eVJustify(), eFmt
                );
            }

            gdevTarget.SetTextColor(*prgbText);
            gdevTarget.DrawString(*pstrToDraw, areaDraw, eHJustify(), eVJustify(), eFmt);
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldBooleanText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldBooleanText::TCQCIntfFldBooleanText() :

    TCQCIntfBooleanTextBase
    (
        CQCIntfEng_BooleanText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_BooleanText)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldBooleanText::TCQCIntfFldBooleanText(const TCQCIntfFldBooleanText& iwdgSrc) :

    TCQCIntfBooleanTextBase(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
{
}

TCQCIntfFldBooleanText::~TCQCIntfFldBooleanText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldBooleanText&
TCQCIntfFldBooleanText::operator=(const TCQCIntfFldBooleanText& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfFldBooleanText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldBooleanText& iwdgOther
    (
        static_cast<const TCQCIntfFldBooleanText&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFldBooleanText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldBooleanText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldBooleanText::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfFldBooleanText::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldBooleanText::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldBooleanText::QueryWdgAttrs( tCIDMData::TAttrList&    colAttrs
                                       , TAttrData&             adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mxins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"State Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldBooleanText::RegisterFields(         TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldBooleanText::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldBooleanText::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  At this level, we just call our parent and any mixins, to let them
//  do any required validation.
//
tCIDLib::TVoid
TCQCIntfFldBooleanText::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Sall our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixins
    if (MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this))
    {
        //
        //  The field is valid, so we can also do some checking on our expressions, to
        //  see if they can take the field type. So look up the field in the cache.
        //
        const TCQCFldDef& flddCur = cfcData.flddFor(strMoniker(), strFieldName());
        if (!exprFalse().bCanAcceptField(flddCur.eType(), kCIDLib::True)
        ||  !exprTrue().bCanAcceptField(flddCur.eType(), kCIDLib::True))
        {
            TString strErr(kIEngErrs::errcVal_BadFldExpr, facCQCIntfEng());
            colErrs.objAdd(TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId()));
        }
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanText: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFldBooleanText::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    tCQCIntfEng::EBoolStates eState;
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        //
        //  Ask the boolean interface to evalute the value based on
        //  it's configured statements.
        //
        const tCQCIntfEng::EExprResults eRes = eCompareValue(fvCurrent(), ctarGlobalVars);

        //
        //  It should evaluate to either true or false, else we have to go into
        //  error state, though the underlying field is fine.
        //
        if (eRes == tCQCIntfEng::EExprResults::False)
            eState = tCQCIntfEng::EBoolStates::False;
        else if (eRes == tCQCIntfEng::EExprResults::True)
            eState = tCQCIntfEng::EBoolStates::True;
        else
            eState = tCQCIntfEng::EBoolStates::Error;
    }
     else
    {
        eState = tCQCIntfEng::EBoolStates::Error;
    }

    // And set the value on our parent
    SetState(eState, bNoRedraw);
}


tCIDLib::TVoid
TCQCIntfFldBooleanText::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    bDesQueryFldInfo(eType, strLimits);
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfFldBooleanText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanText::c2FldFmtVersion))
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

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfFldBooleanText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanText::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldBooleanText::ValueUpdate(        TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarBooleanText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarBooleanText::TCQCIntfVarBooleanText() :

    TCQCIntfBooleanTextBase
    (
        CQCIntfEng_BooleanText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_BooleanText)
    )
{
}

TCQCIntfVarBooleanText::TCQCIntfVarBooleanText(const TCQCIntfVarBooleanText& iwdgSrc) :

    TCQCIntfBooleanTextBase(iwdgSrc)
    , MCQCIntfVarIntf(iwdgSrc)
{
}

TCQCIntfVarBooleanText::~TCQCIntfVarBooleanText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarBooleanText&
TCQCIntfVarBooleanText::operator=(const TCQCIntfVarBooleanText& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfVarIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanText: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarBooleanText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarBooleanText& iwdgOther
    (
        static_cast<const TCQCIntfVarBooleanText&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarBooleanText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarBooleanText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarBooleanText::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfVarBooleanText::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarBooleanText::QueryWdgAttrs( tCIDMData::TAttrList&    colAttrs
                                       , TAttrData&             adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mxins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarBooleanText::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFullMatch
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFullMatch, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarBooleanText::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarBooleanText::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanText: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  When our variable shows up, our variable mixin will call us here to see
//  if we can accept it, so that we can set ourselves up to match it's
//  type and limits.
//
//  For us, we just say yeh, it's ok, since we generally can accept any type.
//  It may cause the statement eval later to be in error, but that's ok. It's
//  hard to determine whether that would happen up front.
//
tCIDLib::TBoolean TCQCIntfVarBooleanText::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfVarBooleanText::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    eType = tCQCKit::EFldTypes::String;
    strLimits.Clear();
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarBooleanText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanText::c2VarFmtVersion))
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

    // Do any mixins, the field one if V2 or beyond
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfVarBooleanText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanText::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarBooleanText::ValueUpdate(        TCQCPollEngine&
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean   bVarUpdate
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    // If no variables could have changed, don't bother
    if (!bVarUpdate)
        return;

    // Pass it on to our mixin
    VarUpdate(ctarGlobalVars, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the variable state
//  changes out of ready state, to let us know we need to update our display
//  to indicate we don't have a good value.
//
tCIDLib::TVoid
TCQCIntfVarBooleanText::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetState(tCQCIntfEng::EBoolStates::Error, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarBooleanText::VarValChanged(  const   TCQCActVar&         varNew
                                        , const tCIDLib::TBoolean   bNoRedraw
                                        , const TStdVarsTar&        ctarGlobalVars)
{
    tCQCIntfEng::EBoolStates eToSet = tCQCIntfEng::EBoolStates::Error;

    //
    //  Ask the boolean interface to evalute the value based on it's
    //  configured statements.
    //
    const tCQCIntfEng::EExprResults eRes = eCompareValue(varNew.fvCurrent(), ctarGlobalVars);

    //
    //  It should evaluate to either true or false, else we have to go into
    //  error state, though the underlying field is fine.
    //
    if (eRes == tCQCIntfEng::EExprResults::False)
        eToSet = tCQCIntfEng::EBoolStates::False;
    else if (eRes == tCQCIntfEng::EExprResults::True)
        eToSet = tCQCIntfEng::EBoolStates::True;

    // And set the value on our parent
    SetState(eToSet, bNoRedraw);
}



