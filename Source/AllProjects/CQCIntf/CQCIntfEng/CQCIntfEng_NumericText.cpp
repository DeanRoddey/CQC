//
// FILE NAME: CQCIntfEng_NumericText.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/24/2004
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
//  widget, which displays a line of text from an associated driver field.
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
#include    "CQCIntfEng_NumericText.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfNumericText,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_NumericText
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our current format version
        //
        //  Version 2 -
        //      We moved the support for single field association from the base
        //      widget class to a mixin, so we need to know if we have already
        //      converted.
        //
        //  Version 3 -
        //      No data change, but we need to adjust our use of colors to make
        //      them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 3;


        // -----------------------------------------------------------------------
        //  Our capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfNumericText
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfNumericText::TCQCIntfNumericText() :

    TCQCIntfWidget
    (
        CQCIntfEng_NumericText::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_NumericText)
    )
    , MCQCIntfSingleFldIntf
      (
        new TCQCFldFiltNumeric(tCQCKit::EReqAccess::MReadCWrite, kCIDLib::True)
      )
    , m_bUseGroups(kCIDLib::False)
    , m_c4DecDigits(2)
    , m_c4FldWidth(0)
    , m_chFill(kCIDLib::chSpace)
    , m_eJustify(tCIDLib::EHJustify::Right)
    , m_eStyle(tCQCIntfEng::ENumTxtStyles::Locale)
    , m_strLeadingText()
    , m_strTrailingText()
    , m_strLiveText(facCQCIntfEng().strErrText())
{
    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/NumericText");
}

TCQCIntfNumericText::TCQCIntfNumericText(const TCQCIntfNumericText& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
    , m_bUseGroups(iwdgSrc.m_bUseGroups)
    , m_c4DecDigits(iwdgSrc.m_c4DecDigits)
    , m_c4FldWidth(iwdgSrc.m_c4FldWidth)
    , m_chFill(iwdgSrc.m_chFill)
    , m_eJustify(iwdgSrc.m_eJustify)
    , m_eStyle(iwdgSrc.m_eStyle)
    , m_strLeadingText(iwdgSrc.m_strLeadingText)
    , m_strTrailingText(iwdgSrc.m_strTrailingText)
    , m_strLiveText(iwdgSrc.m_strLiveText)
{
}

TCQCIntfNumericText::~TCQCIntfNumericText()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Public operators
// ---------------------------------------------------------------------------
TCQCIntfNumericText&
TCQCIntfNumericText::operator=(const TCQCIntfNumericText& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);

        m_bUseGroups      = iwdgSrc.m_bUseGroups;
        m_c4DecDigits     = iwdgSrc.m_c4DecDigits;
        m_c4FldWidth      = iwdgSrc.m_c4FldWidth;
        m_chFill          = iwdgSrc.m_chFill;
        m_eJustify        = iwdgSrc.m_eJustify;
        m_eStyle          = iwdgSrc.m_eStyle;
        m_strLeadingText  = iwdgSrc.m_strLeadingText;
        m_strTrailingText = iwdgSrc.m_strTrailingText;
        m_strLiveText     = iwdgSrc.m_strLiveText;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Called to let us indicate if we can accept, and to optionally conform our
//  self to, a new field. In our case we just check that it's numreic field.
//  We don't need to do anything special to update ourself if storing.
//
tCIDLib::TBoolean
TCQCIntfNumericText::bCanAcceptField(const  TCQCFldDef& flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean)
{
    return
    (
        (flddTest.eType() == tCQCKit::EFldTypes::Card)
        || (flddTest.eType() == tCQCKit::EFldTypes::Int)
        || (flddTest.eType() == tCQCKit::EFldTypes::Float)
    );
}


tCIDLib::TBoolean TCQCIntfNumericText::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfNumericText& iwdgOther(static_cast<const TCQCIntfNumericText&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfSingleFldIntf::bSameField(iwdgOther))
    {
        return kCIDLib::False;
    }

    return
    (
        (m_bUseGroups == iwdgOther.m_bUseGroups)
        && (m_c4DecDigits == iwdgOther.m_c4DecDigits)
        && (m_chFill == iwdgOther.m_chFill)
        && (m_eJustify == iwdgOther.m_eJustify)
        && (m_eStyle == iwdgOther.m_eStyle)
        && (m_strLeadingText == iwdgOther.m_strLeadingText)
        && (m_strTrailingText == iwdgOther.m_strTrailingText)
    );
}


tCIDLib::TVoid
TCQCIntfNumericText::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfNumericText&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfNumericText::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
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


// Initialize this widget
tCIDLib::TVoid
TCQCIntfNumericText::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If in designer mode, put some faux info into the live text value
    //  for display. We use the same method that the designer does to
    //  provide preview text, since it uses current values.
    //
    //  We don't know the field type at this point, so we just indicate
    //  float.
    //
    if (TFacCQCIntfEng::bDesMode())
        Reformat(tCQCKit::EFldTypes::Float);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfNumericText::QueryCmds(         TCollection<TCQCCmd>&   colCmds
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
TCQCIntfNumericText::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


// Returns summary panel info
tCIDLib::TVoid
TCQCIntfNumericText::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins. We don't use the caption text
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr, kCIDLib::False);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Value Field");

    // We also are always single line, so remove the line wrap option that the font mixin added
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Font_NoWrap);

    adatTmp.Set(L"Num. Text Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    TString strLim;
    tCQCIntfEng::FormatENumTxtStyles
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"Format Style"
        , kCQCIntfEng::strAttr_NumText_FormatStyle
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatENumTxtStyles(m_eStyle));
    colAttrs.objAdd(adatTmp);

    //
    //  Some of the others are only used if in custom mode.
    //

    adatTmp.Set
    (
        L"Dec Digits"
        , kCQCIntfEng::strAttr_NumText_DecDigits
        , L"Range: 0, 16"
        , tCIDMData::EAttrTypes::Card
        , (m_eStyle == tCQCIntfEng::ENumTxtStyles::Custom) ? tCIDMData::EAttrEdTypes::InPlace
                                                         : tCIDMData::EAttrEdTypes::None
    );
    adatTmp.SetCard(m_c4DecDigits);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Field Width"
        , kCQCIntfEng::strAttr_NumText_FldWidth
        , L"Range: 2, 64"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(m_c4FldWidth);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Fill Char"
        , kCQCIntfEng::strAttr_NumText_FillChar
        , tCIDMData::EAttrTypes::Char
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetChar(m_chFill);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Leading Text"
        , kCQCIntfEng::strAttr_NumText_LeadText
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strLeadingText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Trailing Text"
        , kCQCIntfEng::strAttr_NumText_TrailText
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strTrailingText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Use Groups"
        , kCQCIntfEng::strAttr_NumText_UseGroups
        , tCIDMData::EAttrTypes::Bool
        , (m_eStyle == tCQCIntfEng::ENumTxtStyles::Custom) ? tCIDMData::EAttrEdTypes::InPlace
                                                         : tCIDMData::EAttrEdTypes::None
    );
    adatTmp.SetBool(m_bUseGroups);
    colAttrs.objAdd(adatTmp);
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfNumericText::RegisterFields(        TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfNumericText::Replace(const  tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfNumericText::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    // And do our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_DecDigits)
        m_c4DecDigits = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_FldWidth)
        m_c4FldWidth = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_FillChar)
        m_chFill = adatNew.chVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_FormatStyle)
        m_eStyle = tCQCIntfEng::eXlatENumTxtStyles(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_LeadText)
        m_strLeadingText = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_TrailText)
        m_strTrailingText = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_NumText_UseGroups)
        m_bUseGroups = adatNew.bVal();
}


// We calcualte a default size for the designer
TSize TCQCIntfNumericText::szDefaultSize() const
{
    // Base on the default text with the current font
    TGraphDesktopDev gdevToUse;
    TCQCIntfFontJan janFont(&gdevToUse, *this);
    TArea areaText = gdevToUse.areaString(m_strLiveText, kCIDLib::False);
    areaText.AdjustSize(16, 4);
    return areaText.szArea();
}


tCIDLib::TVoid
TCQCIntfNumericText::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And pass it to our field mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfNumericText::Set(const  tCIDLib::TBoolean           bUseGroups
                        , const tCIDLib::TCard4             c4DecDigits
                        , const tCIDLib::TCard4             c4FldWidth
                        , const tCIDLib::TCh                chFill
                        , const tCIDLib::EHJustify          eJustify
                        , const tCQCIntfEng::ENumTxtStyles  eStyle
                        , const TString&                    strLeadingText
                        , const TString&                    strTrailingText)
{
    m_bUseGroups      = bUseGroups;
    m_c4DecDigits     = c4DecDigits;
    m_c4FldWidth      = c4FldWidth;
    m_chFill          = chFill;
    m_eJustify        = eJustify;
    m_eStyle          = eStyle;
    m_strLeadingText  = strLeadingText;
    m_strTrailingText = strTrailingText;
}


// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfNumericText::FieldChanged(          TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        //
        //  Ok, we have to reformat the live text value to show the new value.
        //  So clean out the target string and first put in any leading fixed
        //  text.
        //
        m_strLiveText = m_strLeadingText;

        // Now format the value in the appropriate style
        const tCQCKit::EFldTypes eType = eFieldType();

        TString strVal;
        TString strVal2;
        if (eType == tCQCKit::EFldTypes::Card)
        {
            FormatCard
            (
                static_cast<const TCQCCardFldValue&>(fvCurrent()).c4Value()
                , strVal
            );
        }
         else if (eType == tCQCKit::EFldTypes::Float)
        {
            FormatFloat
            (
                static_cast<const TCQCFloatFldValue&>(fvCurrent()).f8Value()
                , strVal
            );
        }
         else if (eType == tCQCKit::EFldTypes::Int)
        {
            FormatInt
            (
                static_cast<const TCQCIntFldValue&>(fvCurrent()).i4Value()
                , strVal
            );
        }
         else
        {
            strVal.Append(facCQCIntfEng().strErrText());
        }

        // Format it into the field width
        strVal2.FormatToFld
        (
            strVal
            , m_c4FldWidth
            , m_eJustify
            , m_chFill
        );

        m_strLiveText.Append(strVal2);

        // And append any trailing text
        m_strLiveText.Append(m_strTrailingText);
    }
     else
    {
        m_strLiveText = facCQCIntfEng().strErrText();
    }

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


//
//  Stream ourself in and out
//
tCIDLib::TVoid TCQCIntfNumericText::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Make sure the format version is good
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_NumericText::c2FmtVersion))
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

    //
    //  Do any mixins
    //
    //  In theory version 1 objects would require some adjustment to move over
    //  to the single field mixing scheme for association of fields, but we will
    //  never see such versions anymore. They cannot upgrade to anything post
    //  3.0 until they go to 3.0 first and it does all the refactoring and
    //  conversion.
    //
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // And pull in our stuff
    strmToReadFrom >> m_bUseGroups
                   >> m_c4DecDigits
                   >> m_c4FldWidth
                   >> m_chFill
                   >> m_eJustify
                   >> m_eStyle
                   >> m_strLeadingText
                   >> m_strTrailingText;

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
}

tCIDLib::TVoid TCQCIntfNumericText::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_NumericText::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream out the mixin's stuff
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << m_bUseGroups
                    << m_c4DecDigits
                    << m_c4FldWidth
                    << m_chFill
                    << m_eJustify
                    << m_eStyle
                    << m_strLeadingText
                    << m_strTrailingText
                    << tCIDLib::EStreamMarkers::EndObject;
}


// Called when we need to redraw all or part of ourself
tCIDLib::TVoid
TCQCIntfNumericText::Update(        TGraphDrawDev&      gdevTarget
                            , const TArea&              areaInvalid
                            ,       TGUIRegion&         grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    DrawText(gdevTarget, areaInvalid, areaDraw);
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfNumericText::ValueUpdate(       TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//  TCQCIntfNumericText: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfNumericText::DrawText(          TGraphDrawDev&  gdevTarget
                                , const TArea&          areaUpdate
                                , const TArea&          areaDraw)
{
    if (areaDraw.bIntersects(areaUpdate))
    {
        // Set our font
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civOwner().DrawTextFX
            (
                gdevTarget
                , m_strLiveText
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
            // If no good value, show disabled
            tCIDGraphDev::ETextFmts eFmt = tCIDGraphDev::ETextFmts::None;
            if (!bGoodFieldValue() && !TFacCQCIntfEng::bDesMode())
                eFmt = tCIDGraphDev::ETextFmts::Disabled;

            //
            //  Now draw our text. If the shadow style is enabled, then draw
            //  first a drop version in fgn3, then draw the regular one in
            //  fgn1 in the normal position.
            //
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);
            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaDraw);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(rgbFgn2());
                gdevTarget.DrawString
                (
                    m_strLiveText
                    , areaDrop
                    , eHJustify()
                    , eVJustify()
                    , eFmt
                );
            }

            gdevTarget.SetTextColor(rgbFgn());
            gdevTarget.DrawString
            (
                m_strLiveText
                , areaDraw
                , eHJustify()
                , eVJustify()
                , eFmt
            );
        }
    }
}


tCIDLib::TVoid
TCQCIntfNumericText::FormatCard(const   tCIDLib::TCard4 c4Val
                                ,       TString&        strToFill)
{
    switch(m_eStyle)
    {
        case tCQCIntfEng::ENumTxtStyles::Locale :
        {
            strToFill.AppendFormatted
            (
                c4Val
                , tCIDLib::ERadices::Dec
                , TLocale::c4GroupSize()
                , TLocale::chGroupSeparator()
            );
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Raw :
        {
            strToFill.AppendFormatted(c4Val);
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Custom :
        {
            strToFill.AppendFormatted
            (
                c4Val
                , tCIDLib::ERadices::Dec
                , m_bUseGroups ? TLocale::c4GroupSize() : 0
                , m_bUseGroups ? TLocale::chGroupSeparator() : 0
            );
            break;
        }

        default :
            strToFill.Append(facCQCIntfEng().strErrText());
        break;
    };
}

tCIDLib::TVoid
TCQCIntfNumericText::FormatFloat(const  tCIDLib::TFloat8 f8Val
                                ,       TString&         strToFill)
{
    switch(m_eStyle)
    {
        case tCQCIntfEng::ENumTxtStyles::Locale :
        {
            strToFill.AppendFormatted
            (
                f8Val
                , TLocale::c4DecimalDigits()
                , TLocale::chNegativeSign()
                , TLocale::chDecimalSymbol()
                , TLocale::c4GroupSize()
                , TLocale::chGroupSeparator()
            );
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Raw :
        {
            strToFill.AppendFormatted(f8Val);
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Custom :
        {
            strToFill.AppendFormatted
            (
                f8Val
                , m_c4DecDigits
                , TLocale::chNegativeSign()
                , TLocale::chDecimalSymbol()
                , m_bUseGroups ? TLocale::c4GroupSize() : 0
                , m_bUseGroups ? TLocale::chGroupSeparator() : 0
            );
            break;
        }

        default :
            strToFill.Append(facCQCIntfEng().strErrText());
        break;
    };
}

tCIDLib::TVoid
TCQCIntfNumericText::FormatInt( const   tCIDLib::TInt4  i4Val
                                ,       TString&        strToFill)
{
    switch(m_eStyle)
    {
        case tCQCIntfEng::ENumTxtStyles::Locale :
        {
            strToFill.AppendFormatted
            (
                i4Val
                , tCIDLib::ERadices::Dec
                , TLocale::chNegativeSign()
                , TLocale::c4GroupSize()
                , TLocale::chGroupSeparator()
            );
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Raw :
        {
            strToFill.AppendFormatted(i4Val);
            break;
        }

        case tCQCIntfEng::ENumTxtStyles::Custom :
        {
            strToFill.AppendFormatted
            (
                i4Val
                , tCIDLib::ERadices::Dec
                , TLocale::chNegativeSign()
                , m_bUseGroups ? TLocale::c4GroupSize() : 0
                , m_bUseGroups ? TLocale::chGroupSeparator() : 0
            );
            break;
        }

        default :
            strToFill.Append(facCQCIntfEng().strErrText());
        break;
    };
}


//
//  This is to support design time display of test data. The attrs tab
//  calls this.
//
tCIDLib::TVoid TCQCIntfNumericText::Reformat(const tCQCKit::EFldTypes eType)
{
    //
    //  Ok, we have to reformat the live text value to show the new value.
    //  So clean out the target string and first put in any leading fixed
    //  text.
    //
    m_strLiveText = m_strLeadingText;

    //
    //  If our field has been set, then we can display something semi-
    //  meaningful. Else just show the error state.
    //
    TString strVal;
    TString strVal2;
    switch(eType)
    {
        case tCQCKit::EFldTypes::Card :
            FormatCard(12345, strVal);
            break;

        case tCQCKit::EFldTypes::Float :
            FormatFloat(12345.6789, strVal);
            break;

        case tCQCKit::EFldTypes::Int :
            FormatInt(-12345, strVal);
            break;

        default :
            strVal.Append(facCQCIntfEng().strErrText());
            break;
    };

    // Format it into the field width and append that to the value
    strVal2.FormatToFld
    (
        strVal
        , m_c4FldWidth
        , m_eJustify
        , m_chFill
    );
    m_strLiveText.Append(strVal2);

    // And append any trailing text
    m_strLiveText.Append(m_strTrailingText);
}



