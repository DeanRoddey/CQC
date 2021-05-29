//
// FILE NAME: CQCIntfEng_PushButton.cpp
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
//  This file implements a derivative of the base button widget, which
//  implemented in this case a standard push button.
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
#include    "CQCIntfEng_PushButton.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfPushButton,TCQCIntfButtonBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_PushButton
    {
        // -----------------------------------------------------------------------
        //  Our format version
        //
        //  Version 2 -
        //      The Xlats stuff was moved into a mixin and we now support it at
        //      this class level, so that all pushbutton derivatives can support
        //      translation if desired.
        //
        //  Version 3 -
        //      No data change, but we need to adjust our use of colors to make
        //      them consistent.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 3;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfPushButton
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Public, static data
// ---------------------------------------------------------------------------
const TString   TCQCIntfPushButton::strFocusImg(L"Focus");
const TString   TCQCIntfPushButton::strPressedImg(L"Pressed");
const TString   TCQCIntfPushButton::strUnpressedImg(L"Unpressed");
const TString   TCQCIntfPushButton::strOverlayImg(L"Overlay");


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfPushButton::
TCQCIntfPushButton( const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfButtonBase(eCapFlags, strTypeName)
    , m_bErrState(kCIDLib::False)
{
    // Default our text justification to centered
    eHJustify(tCIDLib::EHJustify::Center);
    eVJustify(tCIDLib::EVJustify::Center);


    // We allow multi-line text, so turn off the single line flag
    ResetCaption(kCIDLib::False, TString::strEmpty());

    //
    //  Load up the keys into the image list mixin, initially all disabled
    //  since by default we don't use them. These default settings will be
    //  overridden if we are being created to stream in a widget.
    //
    AddNewImgKey(strUnpressedImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(strPressedImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(strFocusImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(strOverlayImg, tCQCIntfEng::EImgReqFlags::Optional);

    // Set our default appearance
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    strCaption(L"Text");
    SetImgKey
    (
        strUnpressedImg
        , L"/System/ImgSets/Antique/Med Butt Unpressed"
        , tCQCIntfEng::EImgReqFlags::Optional
        , 0xFF
        , kCIDLib::True
    );
    SetImgKey
    (
        strPressedImg
        , L"/System/ImgSets/Antique/Med Butt Pressed"
        , tCQCIntfEng::EImgReqFlags::Optional
        , 0xFF
        , kCIDLib::True
    );
}

TCQCIntfPushButton::TCQCIntfPushButton(const TCQCIntfPushButton& iwdgSrc) :

    TCQCIntfButtonBase(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , MCQCIntfXlatIntf(iwdgSrc)
    , m_bErrState(iwdgSrc.m_bErrState)
    , m_strDisplayText(iwdgSrc.m_strDisplayText)
    , m_strLiveText(iwdgSrc.m_strLiveText)
{
}

TCQCIntfPushButton::~TCQCIntfPushButton()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Public operators
// ---------------------------------------------------------------------------
TCQCIntfPushButton&
TCQCIntfPushButton::operator=(const TCQCIntfPushButton& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        MCQCIntfXlatIntf::operator=(iwdgSrc);

        m_bErrState         = iwdgSrc.m_bErrState;
        m_strDisplayText    = iwdgSrc.m_strDisplayText;
        m_strLiveText       = iwdgSrc.m_strLiveText;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfPushButton::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfPushButton::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfPushButton& iwdgOther(static_cast<const TCQCIntfPushButton&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther)
    ||  !MCQCIntfXlatIntf::bSameXlats(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Nothing else at this level for now
    return kCIDLib::True;
}


// Our callback to handle command invocation requests
tCQCKit::ECmdRes
TCQCIntfPushButton::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetText)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        if (varText.bSetValue(m_strDisplayText))
        {
            if (acteTar.pcmdtDebug())
                acteTar.pcmdtDebug()->ActVarSet(strKey, varText.strValue());
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetNamedImage)
    {
        TDataSrvClient dsclLoad;
        UpdateImg
        (
            civOwner(), dsclLoad, ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
        );

        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
    {
        SetImgOpacity
        (
            ccfgToDo.piAt(0).m_strValue
            , tCIDLib::TCard1(ccfgToDo.piAt(1).m_strValue.c4Val())
        );

        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetText)
    {
        //
        //  Store the new caption. The caption changed callback will be
        //  called and it will update our display text if needed.
        //
        strCaption(ccfgToDo.piAt(0).m_strValue);

        if (bCanRedraw(strEventId))
            Redraw();
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


tCIDLib::TVoid TCQCIntfPushButton::
Initialize( TCQCIntfContainer* const    piwdgParent
            , TDataSrvClient&           dsclInit
            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);
}


// Return our list of commands we support
tCIDLib::TVoid
TCQCIntfPushButton::QueryCmds(          TCollection<TCQCCmd>&   colToFill
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first
    TParent::QueryCmds(colToFill, eContext);

    // Get the text of the button to a variable
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_GetText
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetText)
            , tCQCKit::ECmdPTypes::VarName
            , facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey)
        )
    );

    // Change the image for one of the named slots
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetNamedImage
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetNamedImage)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
            , tCQCKit::ECmdPTypes::ImgPath
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_ImagePath)
        )
    );

    // Set the opacity of one of our images
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetOpacity
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetOpacity)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_ToSet)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_Opacity)
        )
    );

    // Set our text to the passed text
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetText
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetText)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_NewText)
        )
    );
}


// Provide the size of our contents (for 'size to' type operations)
tCIDLib::TVoid
TCQCIntfPushButton::QueryContentSize(TSize& szToFill)
{
    //
    //  For us, this can be a bit complex. We want to basically calculate
    //  the bounds of any images (which can be offset) and the text.
    //
    //  So first ask the image list mixin to do give us the bounds of any
    //  images, including offsets. It returns an origin centered area, which
    //  is what we want.
    //
    TArea areaBounds = areaImgBounds();

    // If we have some text, then do it also
    const TString& strText = m_strDisplayText;
    if (!strText.bIsEmpty())
    {
        const TArea areaCenterIn(-1000, -1000, 2000, 2000);
        TArea areaTmp;

        tCQCIntfEng::TGraphIntfDev cptrDev(cptrNewGraphDev());

        TCQCIntfFontJan janFont(cptrDev.pobjData(), *this);
        areaTmp = cptrDev->areaString(strText, kCIDLib::False);
        areaTmp.Inflate(4, 2);
        areaTmp.CenterIn(areaCenterIn);
        areaBounds |= areaTmp;
    }

    // If we had neither, just return a default size
    if (areaBounds.bIsEmpty())
    {
        areaBounds.Set(0, 0, 48, 22);
    }
     else
    {
        // If we have a border, inflate by 1
        if (bHasBorder())
            areaBounds.Inflate(1);
    }

    // And return the size
    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfPushButton::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Pass on to our image list mixin
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfPushButton::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove attributes we don't honor
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Update some color names to reflect our usage
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Focus Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Focus Shadow/FX");

    // Get the attributes for our mixins
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, &strCaption());
    MCQCIntfXlatIntf::QueryXlatAttrs(colAttrs, adatTmp);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfPushButton::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfPushButton::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfPushButton::szDefaultSize() const
{
    return TSize(84, 32);
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfPushButton::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(1).m_strValue = L"255";
}


// Store updated widget attributes
tCIDLib::TVoid
TCQCIntfPushButton::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Call our parent first
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our mixins
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
    MCQCIntfXlatIntf::SetXlatAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfPushButton::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Protected, inherited methods
// ---------------------------------------------------------------------------

// We may need to update our text if this happens, so reformat and update
tCIDLib::TVoid TCQCIntfPushButton::CaptionChanged()
{
    FmtXlatText(strCaption(), m_strLiveText, m_bErrState, m_strDisplayText);
}


tCIDLib::TVoid TCQCIntfPushButton::StreamFrom(TBinInStream& strmToReadFrom)
{
    // We should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version info
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_PushButton::c2FmtVersion))
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

    // Call our parent so he can do his stuff
    TParent::StreamFrom(strmToReadFrom);

    // Do any mixins
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // If V2 or beyond, we have an xlat mixin
    if (c2FmtVersion > 1)
        MCQCIntfXlatIntf::ReadInXlats(strmToReadFrom);

    // We don't have any of our own stuff yet

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

    // Init some runtime stuff
    m_bErrState = kCIDLib::False;
    m_strLiveText.Clear();
}


tCIDLib::TVoid TCQCIntfPushButton::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our strat marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_PushButton::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Let the mixin's stream out
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);
    MCQCIntfXlatIntf::WriteOutXlats(strmToWriteTo);

    // We don't have any stuff yet, so just do the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfPushButton::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip to any remaining area, get that area,a nd draw into it
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    TArea areaDraw = grgnClip.areaBounds();

    DrawButton(gdevTarget, areaInvalid, areaDraw);
}


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes call this to give us a new value to display.
tCIDLib::TVoid
TCQCIntfPushButton::SetText(const   TString&            strToSet
                            , const tCIDLib::TBoolean   bNoRedraw
                            , const tCIDLib::TBoolean   bErrState)
{
    // Remember the live text and error state
    m_bErrState = bErrState;
    m_strLiveText = strToSet;

    //
    //  Handle any translation and such. The Xlats mixin handles that
    //  for us. Format it into our display text member.
    //
    FmtXlatText(strCaption(), strToSet, bErrState, m_strDisplayText);

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


// ---------------------------------------------------------------------------
//  TCQCIntfPushButton: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfPushButton::DrawButton(         TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TArea&          areaDraw)
{
    // Get the pressed and focus flags
    const tCIDLib::TBoolean bUseUnpressed(bImgEnabled(strUnpressedImg));
    const tCIDLib::TBoolean bUsePressed(bImgEnabled(strPressedImg));
    const tCIDLib::TBoolean bUseFocus(bImgEnabled(strFocusImg));

    //
    //  If in designer mode, force the display of some image if any are
    //  used. Else we are in viewing mode and must select one based on
    //  state.
    //
    if (bUseUnpressed || bUsePressed || bUseFocus)
    {
        TString strToLoad;
        if (TFacCQCIntfEng::bDesMode())
        {
            if (bUseUnpressed)
                strToLoad = strUnpressedImg;
            else if (bUseFocus)
                strToLoad = strFocusImg;
            else if (bUsePressed)
                strToLoad = strPressedImg;
        }
         else
        {
            //
            //  Decide which image we want to use. We just need to figure out
            //  which key from the image list mixin to load.
            //
            if (bPressed())
            {
                //
                //  We are pressed, which means that we are tracking the mouse
                //  and it is inside us, or the user has pressed the enter key.
                //  So we use the pressed image if we have one, else we use the
                //  unpressed.
                //
                if (bUsePressed)
                    strToLoad = strPressedImg;
                 else
                    strToLoad = strUnpressedImg;
            }
             else
            {
                //
                //  We aren't tracking or pressed, so show the focus img if
                //  we have one and we have the focus, else show the
                //  unpressed image.
                //
                if (bHasFocus() && bUseFocus)
                    strToLoad = strFocusImg;
                else
                    strToLoad = strUnpressedImg;
            }
        }

        // Ok, load the image from the list interface if we selected one
        MCQCIntfImgListIntf::TImgInfo* pimgiCur = 0;
        if (!strToLoad.bIsEmpty())
            pimgiCur = pimgiForKeyEO(strToLoad);

        // If we found one, and it's enabled, then draw it
        if (pimgiCur)
        {
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , kCIDLib::False
                , pimgiCur->m_cptrImg
                , areaDraw
                , areaInvalid
                , pimgiCur->m_i4HOffset
                , pimgiCur->m_i4VOffset
                , pimgiCur->m_c1Opacity
                , bPressed() && !bUsePressed
            );
        }
    }

    //
    //  If we have an overlay image, then we need to display it. We just
    //  center it, but adjust it down/right if we are pressed.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiOver = pimgiForKeyEO(strOverlayImg);
    if (pimgiOver)
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiOver->m_cptrImg
            , areaDraw
            , areaInvalid
            , pimgiOver->m_i4HOffset
            , pimgiOver->m_i4VOffset
            , pimgiOver->m_c1Opacity
            , bPressed()
        );
    }

    // If we have any text, then draw it
    const TString& strText = m_strDisplayText;
    if (!strText.bIsEmpty())
    {
        // If we are pressed, move the text over and down slightly
        if (bPressed())
            areaDraw.AdjustOrg(1, 1);

        const tCIDLib::TBoolean bDoFocus = bHasFocus();

        // Set the font on our device
        TCQCIntfFontJan janFont(&gdevTarget, *this);

        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            // We use regular or focus colors here
            civOwner().DrawTextFX
            (
                gdevTarget
                , strText
                , areaDraw
                , eTextFX()
                , bDoFocus ? rgbExtra() : rgbFgn()
                , bDoFocus ? rgbExtra2() : rgbFgn2()
                , eHJustify()
                , eVJustify()
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            //
            //  Set up the device attributes according to our settings.
            //  Then call the derived class to get the text to display.
            //
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

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
                gdevTarget.SetTextColor(bDoFocus ? rgbExtra2() : rgbFgn2());

                if (bNoTextWrap())
                    gdevTarget.DrawString(strText, areaDrop, eHJustify(), eVJustify());
                else
                    gdevTarget.DrawMText(strText, areaDrop, eHJustify(), eVJustify(), kCIDLib::True);
            }

            // Use regular or focus color
            gdevTarget.SetTextColor(bDoFocus ? rgbExtra() : rgbFgn());

            if (bNoTextWrap())
                gdevTarget.DrawString(strText, areaDraw, eHJustify(), eVJustify());
            else
                gdevTarget.DrawMText(strText, areaDraw, eHJustify(), eVJustify(), kCIDLib::True);
        }
    }
}

