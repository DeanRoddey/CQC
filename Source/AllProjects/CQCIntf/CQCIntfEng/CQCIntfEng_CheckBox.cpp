//
// FILE NAME: CQCIntfEng_CheckBox.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2002
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
//  This file implements a boolean check box widget.
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
#include    "CQCIntfEng_CheckBox.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfCheckBoxBase,TCQCIntfButtonBase)
AdvRTTIDecls(TCQCIntfFieldCheckBox,TCQCIntfCheckBoxBase)
AdvRTTIDecls(TCQCIntfStaticCheckBox,TCQCIntfCheckBoxBase)
AdvRTTIDecls(TCQCIntfVarCheckBox,TCQCIntfCheckBoxBase)




// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_CheckBox
    {
        // -----------------------------------------------------------------------
        //  The base class format version.
        //
        //  Version 2 -
        //      We added a new Error image, to show when the check box is in
        //      error, so we need to know when reading in an old one so that we
        //      can force the extra key into the image list mixin.
        //
        //  Version 3 -
        //      Added a new 'use extra colors' flag which lets us optionally use
        //      the Fgn colors for false state but the Extra colors for true state,
        //      so that we can change text color depending on the state.
        //
        //  Version 4 -
        //      No data change, but we needed to change some color usage to make
        //      it consistent.
        //
        //  Version 5 -
        //      Get rid of the pressed image, which doesn't make any sense anymore.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2BaseFmtVersion    = 5;


        // -----------------------------------------------------------------------
        //  The field based class format version.
        //
        //  Version 2 -
        //      Field stuff was moved to a new mixin, so we have to know whehter
        //      to stream it in or to set it up from old info.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FldFmtVersion     = 2;


        // -----------------------------------------------------------------------
        //  The static check box class format version.
        //
        //  Version 2 -
        //      Added an auto-toggle
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2StaticFmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The variable check box class format version.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2VarFmtVersion  = 1;


        // -----------------------------------------------------------------------
        //  The space between the text and image if left or right layout is used
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TInt4        i4TextOfs = 4;


        // -----------------------------------------------------------------------
        //  Our capabilities flags. We use the same ones for field and variable
        //  based versions. And we have a separate set for the static
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags
        (
            tCQCIntfEng::ECapFlags::TakesFocus
        );


        // -----------------------------------------------------------------------
        //  The key under which we store the optional images in the image
        //  list interface.
        // -----------------------------------------------------------------------
        const TString strFalseFocusImg(L"False Focus");
        const TString strTrueFocusImg(L"True Focus");
        const TString strFalseOverlayImg(L"False Overlay");
        const TString strTrueOverlayImg(L"True Overlay");
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCheckBoxRTV
//  PREFIX: crtvs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxRTV: Constructor and Destructor
// ---------------------------------------------------------------------------
TCQCIntfCheckBoxRTV::TCQCIntfCheckBoxRTV(const  TString&        strStateText
                                        , const TCQCUserCtx&    cuctxToUse) :
    TCQCCmdRTVSrc(cuctxToUse)
    , m_strStateText(strStateText)
{
}

TCQCIntfCheckBoxRTV::~TCQCIntfCheckBoxRTV()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxRTV: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCIntfCheckBoxRTV::bRTValue(const TString& strId, TString& strToFill) const
{
    // Call our parent first
    if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
        return kCIDLib::True;

    // If it's one of ours, then handle it
    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (strId == kCQCIntfEng::strRTVId_StateText)
        strToFill = m_strStateText;
    else
        bRet = kCIDLib::False;
    return bRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfCheckBoxBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfCheckBoxBase::~TCQCIntfCheckBoxBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfCheckBoxBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfCheckBoxBase::bIsSame(const  TCQCIntfWidget& iwdgSrc) const
{
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfCheckBoxBase& iwdgOther
    (
        static_cast<const TCQCIntfCheckBoxBase&>(iwdgSrc)
    );

    // Do our mixins
    if (!MCQCIntfBooleanIntf::bSameBool(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther)
    ||  !MCQCIntfFontIntf::bSameFont(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }

    // Compare our stuff
    return
    (
        (m_eHLayout == iwdgOther.m_eHLayout)
        && (m_strFalseText == iwdgOther.m_strFalseText)
        && (m_strTrueText == iwdgOther.m_strTrueText)
    );
}


//
//  We were clicked on. The point doesn't matter since we have no internal
//  structure, therefore this is the same as Invoke() below in terms of what
//  it does. So we just call that.
//
tCIDLib::TVoid TCQCIntfCheckBoxBase::Clicked(const TPoint&)
{
    Invoke();
}


// Our callback to handle command invocation requests
tCQCKit::ECmdRes
TCQCIntfCheckBoxBase::eDoCmd(const  TCQCCmdCfg&             ccfgToDo
                            , const tCIDLib::TCard4         c4Index
                            , const TString&                strUserId
                            , const TString&                strEventId
                            ,       TStdVarsTar&            ctarGlobals
                            ,       tCIDLib::TBoolean&      bResult
                            ,       TCQCActEngine&          acteTar)
{
    // If we have any mixins that do their own commands, give them a chance
    if (MCQCIntfBooleanIntf::bDoBoolCmd(ccfgToDo, c4Index))
        return tCQCKit::ECmdRes::Ok;

    //
    //  Get the current state of the check box. It has no parameters, it just
    //  returns a result.
    //
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetState)
    {
        // We just return our current result as the status of the command
        bResult = bIsChecked();
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetText)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        tCIDLib::TBoolean bChanged;
        if (m_eCurState == tCQCIntfEng::EBoolStates::False)
            bChanged = varText.bSetValue(m_strFalseText);
        else if (m_eCurState == tCQCIntfEng::EBoolStates::True)
            bChanged = varText.bSetValue(m_strTrueText);
        else
            bChanged = varText.bSetValue(facCQCIntfEng().strErrText());

        if (bChanged && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(strKey, varText.strValue());
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetNamedImage)
    {
        TDataSrvClient dsclUp;
        UpdateImg
        (
            civOwner(), dsclUp, ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue
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
        if (ccfgToDo.piAt(0).m_strValue == kCQCKit::pszFld_True)
            m_strTrueText = ccfgToDo.piAt(1).m_strValue;
        else
            m_strFalseText = ccfgToDo.piAt(1).m_strValue;

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


tCIDLib::TVoid
TCQCIntfCheckBoxBase::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    //
    //  If we are in Designer state, then force the state to the false
    //  state, so that we'll draw one of the required images.
    //
    if (TFacCQCIntfEng::bDesMode())
        m_eCurState = tCQCIntfEng::EBoolStates::False;
}


//
//  This is called whenever anything happened that would invoke the button. A click, the
//  user pressing enter when focus was on the button, etc...
//
tCIDLib::TVoid TCQCIntfCheckBoxBase::Invoke()
{
    // Do click feedback
    DoClickFeedback();

    // ANd post the appropriate event for our state
    PostOps
    (
        *this
        , bIsChecked() ? kCQCKit::strEvId_OnTrueClick : kCQCKit::strEvId_OnFalseClick
        , tCQCKit::EActOrders::NonNested
    );

    //
    //  Don't do anything else unless you check that we weren't killed by
    //  this action.
    //
}


//
//  Just gen up one of our runtime value objects. If we are in error state,
//  provide an empty string as the current state. Note that we are returning
//  what the new state is going to be, so we return the opposite of the
//  current state.
//
TCQCCmdRTVSrc*
TCQCIntfCheckBoxBase::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    TCQCCmdRTVSrc* pcrtsRet = 0;
    if (m_eCurState == tCQCIntfEng::EBoolStates::True)
        pcrtsRet = new TCQCIntfCheckBoxRTV(m_strFalseText, cuctxToUse);
    else if (m_eCurState == tCQCIntfEng::EBoolStates::False)
        pcrtsRet = new TCQCIntfCheckBoxRTV(m_strTrueText, cuctxToUse);
    else
        pcrtsRet = new TCQCIntfCheckBoxRTV(TString::strEmpty(), cuctxToUse);
    return pcrtsRet;
}


// Return the list of commands we support
tCIDLib::TVoid
TCQCIntfCheckBoxBase::QueryCmds(        TCollection<TCQCCmd>&   colToFill
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first
    TParent::QueryCmds(colToFill, eContext);

    // Do any mixins that have their own commands
    MCQCIntfBooleanIntf::QueryBoolCmds(colToFill, eContext);

    // Get the current state of the check box
    {
        TCQCCmd& cmdGetState = colToFill.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetState
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetState)
                , 0
            )
        );
        cmdGetState.eType(tCQCKit::ECmdTypes::Conditional);
    }

    // Get the text for the current check box state
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

    // Update one of our images
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


    // Set the true or false text
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetText
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetText)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            , tCQCKit::ECmdPTypes::Text
            , facCQCKit().strMsg(kKitMsgs::midCmdP_NewText)
        )
    );
}


// Return our content size, for 'size to' type operations
tCIDLib::TVoid TCQCIntfCheckBoxBase::QueryContentSize(TSize& szToFill)
{
    //
    //  For us, this can be a bit complex. We want to basically calculate
    //  the bounds of any images (which can be offset) and the text.
    //
    //  So first ask the image list mixin to give us the bounds of any images,
    //  including offsets. It returns an origin centered area, which is what
    //  we want.
    //
    TArea areaBounds = areaImgBounds();

    //
    //  And now get the size of the text strings and take the larger of the
    //  two. Set our font before we do this.
    //
    tCQCIntfEng::TGraphIntfDev cptrGraphDev = cptrNewGraphDev();
    TArea areaFalse;
    TArea areaTrue;
    {
        TCQCIntfFontJan janFont(cptrGraphDev.pobjData(), *this);
        areaFalse = cptrGraphDev->areaString(m_strFalseText, kCIDLib::False);
        areaFalse.Inflate(4, 2);
        areaTrue = cptrGraphDev->areaString(m_strTrueText, kCIDLib::False);
        areaTrue.Inflate(4, 2);
    }

    // Center them on the origin
    const TArea areaCenterIn(-1000, -1000, 2000, 2000);
    areaFalse.CenterIn(areaCenterIn);
    areaTrue.CenterIn(areaCenterIn);

    //
    //  If the style is centered, we take the union of the text and image
    //  areas. Else, the horizontal gets added, plus the margin between
    //  the image and text.
    //
    if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::Center)
    {
        areaBounds |= areaFalse;
        areaBounds |= areaTrue;
    }
     else
    {
        TSize szBiggestText(areaFalse.szArea());
        szBiggestText.TakeLarger(areaTrue.szArea());

        areaBounds.AdjustSize
        (
            CQCIntfEng_CheckBox::i4TextOfs + szBiggestText.c4Width(), 0
        );

        if (szBiggestText.c4Height() > areaBounds.c4Height())
            areaBounds.c4Height(szBiggestText.c4Height());
    }

    // If we have a border, inflate one for it
    if (bHasBorder())
        areaBounds.Inflate(1);

    //
    //  Add the margin that the drawing code always uses, plus one for any
    //  border. Even if we end up not having a border (and don't use one for
    //  focus), the drawing code adds this extra margin of 1. If we do use
    //  the focus border, this means that the text doesn't move back and
    //  forth (when left/right justified) as the focus comes and goes.
    //
    areaBounds.Inflate(2, 1);
    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfCheckBoxBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Pass on to our image list mixin
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfCheckBoxBase::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Set our names for the colors we keep (bgn 1/2 are already ok)
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow/FX");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1, L"Checked Text");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2, L"Checked Shadow/FX");

    //
    //  Do our mixins. Boolean is not needed for statics which just have an internal
    //  state, so no source value to evaluate.
    //
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Click Actions"
    );
    if (!bIsA(TCQCIntfStaticCheckBox::clsThis()))
        MCQCIntfBooleanIntf::QueryBoolAttrs(colAttrs, adatTmp);
    MCQCIntfFontIntf::QueryFontAttrs(colAttrs, adatTmp, nullptr, kCIDLib::False);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    // Set our attributes
    adatTmp.Set(L"CheckBox Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // The true/false text values
    adatTmp.Set
    (
        L"False Text"
        , kCQCIntfEng::strAttr_Bool_FalseText
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strFalseText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"True Text"
        , kCQCIntfEng::strAttr_Bool_TrueText
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strTrueText);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"State-Based Colors"
        , kCQCIntfEng::strAttr_Bool_StateBasedClrs
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bUseExtraClrs);
    colAttrs.objAdd(adatTmp);


    TString strLims;
    tCQCIntfEng::FormatEHTxtImgLayouts
    (
        strLims, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"Text/Image Layout"
        , kCQCIntfEng::strAttr_CheckBox_Layout
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatEHTxtImgLayouts(m_eHLayout));
    colAttrs.objAdd(adatTmp);

    // We don't honor h/v justification, we do them ourselves, so remove those
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Font_HJustify);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Font_VJustify);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid TCQCIntfCheckBoxBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfCheckBoxBase::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);

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


TSize TCQCIntfCheckBoxBase::szDefaultSize() const
{
    return TSize(92, 28);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfCheckBoxBase::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Store our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_FalseText)
        m_strFalseText = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_StateBasedClrs)
        m_bUseExtraClrs = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_TrueText)
        m_strTrueText = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_CheckBox_Layout)
        m_eHLayout = tCQCIntfEng::eXlatEHTxtImgLayouts(adatNew.strValue());

    // Handle our mixins
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);
    MCQCIntfBooleanIntf::SetBoolAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfFontIntf::SetFontAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfCheckBoxBase::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(1).m_strValue = L"255";
}


tCIDLib::TVoid
TCQCIntfCheckBoxBase::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the user extra colors flag
tCIDLib::TBoolean TCQCIntfCheckBoxBase::bUseExtraClrs() const
{
    return m_bUseExtraClrs;
}


tCIDLib::TBoolean
TCQCIntfCheckBoxBase::bUseExtraClrs(const tCIDLib::TBoolean bToSet)
{
    m_bUseExtraClrs = bToSet;
    return m_bUseExtraClrs;
}


// Set us up with layout and text info
tCIDLib::TVoid
TCQCIntfCheckBoxBase::Set(  const   tCQCIntfEng::EHTxtImgLayouts eLayout
                            , const TString&                     strFalseText
                            , const TString&                     strTrueText)
{
    m_eHLayout = eLayout;
    m_strFalseText = strFalseText;
    m_strTrueText = strTrueText;
}


// Get/set the false and true text
const TString& TCQCIntfCheckBoxBase::strFalseText() const
{
    return m_strFalseText;
}

const TString& TCQCIntfCheckBoxBase::strFalseText(const TString& strToSet)
{
    m_strFalseText = strToSet;
    return m_strFalseText;
}


const TString& TCQCIntfCheckBoxBase::strTrueText() const
{
    return m_strTrueText;
}

const TString& TCQCIntfCheckBoxBase::strTrueText(const TString& strToSet)
{
    m_strTrueText = strToSet;
    return m_strTrueText;
}



// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Hidden Constructors and Operators
// ---------------------------------------------------------------------------
TCQCIntfCheckBoxBase::
TCQCIntfCheckBoxBase(const  tCQCIntfEng::ECapFlags  eCaps
                    , const TString&                strTypeName) :

    TCQCIntfButtonBase(eCaps, strTypeName)
    , m_bUseExtraClrs(kCIDLib::False)
    , m_bWriteable(kCIDLib::False)
    , m_eCurState(tCQCIntfEng::EBoolStates::Error)
    , m_eHLayout(tCQCIntfEng::EHTxtImgLayouts::TextRight)
    , m_strFalseText(L"False")
    , m_strTrueText(L"True")
{
    // Load up the true/false keys into the image list mixin
    AddNewImgKey(kCQCKit::pszFld_False, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(kCQCKit::pszFld_True, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_CheckBox::strFalseFocusImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_CheckBox::strTrueFocusImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_CheckBox::strFalseOverlayImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_CheckBox::strTrueOverlayImg, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(kCQCIntfEng_::pszImgKey_Error, tCQCIntfEng::EImgReqFlags::Optional);

    // Add the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrueClick)
        , kCQCKit::strEvId_OnTrueClick
        , tCQCKit::EActCmdCtx::Standard
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnFalseClick)
        , kCQCKit::strEvId_OnFalseClick
        , tCQCKit::EActCmdCtx::Standard
    );

    // Add RTVs that we support
    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_StateText)
        , kCQCIntfEng::strRTVId_StateText
        , tCQCKit::ECmdPTypes::Text
    );

    // Set out default look
    bBold(kCIDLib::True);
    bIsTransparent(kCIDLib::True);
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);
    rgbExtra(kCQCIntfEng_::rgbDef_EmphText);
    rgbExtra2(kCQCIntfEng_::rgbDef_EmphTextShadow);

    SetImgKey
    (
        kCQCKit::pszFld_False
        , L"/System/ImgSets/Antique/Check Box Off"
        , tCQCIntfEng::EImgReqFlags::Optional
        , 0xFF
        , kCIDLib::True
    );

    SetImgKey
    (
        kCQCKit::pszFld_True
        , L"/System/ImgSets/Antique/Check Box On"
        , tCQCIntfEng::EImgReqFlags::Optional
        , 0xFF
        , kCIDLib::True
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/CheckBox");
}

TCQCIntfCheckBoxBase::TCQCIntfCheckBoxBase(const TCQCIntfCheckBoxBase& iwdgSrc) :

    TCQCIntfButtonBase(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , MCQCIntfBooleanIntf(iwdgSrc)
    , MCQCIntfFontIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_bUseExtraClrs(iwdgSrc.m_bUseExtraClrs)
    , m_bWriteable(iwdgSrc.m_bWriteable)
    , m_eCurState(iwdgSrc.m_eCurState)
    , m_eHLayout(iwdgSrc.m_eHLayout)
    , m_strFalseText(iwdgSrc.m_strFalseText)
    , m_strTrueText(iwdgSrc.m_strTrueText)
{
}

TCQCIntfCheckBoxBase&
TCQCIntfCheckBoxBase::operator=(const TCQCIntfCheckBoxBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        MCQCIntfBooleanIntf::operator=(iwdgSrc);
        MCQCIntfFontIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);

        m_bUseExtraClrs = iwdgSrc.m_bUseExtraClrs;
        m_bWriteable    = iwdgSrc.m_bWriteable;
        m_eCurState     = iwdgSrc.m_eCurState;
        m_eHLayout       = iwdgSrc.m_eHLayout;
        m_strFalseText  = iwdgSrc.m_strFalseText;
        m_strTrueText   = iwdgSrc.m_strTrueText;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfCheckBoxBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CheckBox::c2BaseFmtVersion))
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

    // Stream in the mixins' stuff
    MCQCIntfFontIntf::ReadInFont(strmToReadFrom);
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);
    MCQCIntfBooleanIntf::ReadInBool(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Looks ok, so lets stream in our stuff
    strmToReadFrom  >> m_eHLayout
                    >> m_strFalseText
                    >> m_strTrueText;

    // If version 3 or beyond, read in the use extra colors flag, else default it
    if (c2FmtVersion > 2)
        strmToReadFrom  >> m_bUseExtraClrs;
    else
        m_bUseExtraClrs = kCIDLib::False;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // If a V1, force the new error image key in
    if (c2FmtVersion == 1)
        AddNewImgKey(kCQCIntfEng_::pszImgKey_Error, tCQCIntfEng::EImgReqFlags::Optional);

    //
    //  If V3 or less, then we need to adjust some colors, so that the F2
    //  color is always used for shadow/fx. Currently F3 is being used for
    //  shadow and F2 for FX, and F2 is also being used for the border which we
    //  move to F3.
    //
    if (c2FmtVersion < 4)
    {
        // Save the original values we are going to change
        TRGBClr rgbF2 = rgbFgn2();
        TRGBClr rgbF3 = rgbFgn3();

        // Put F2 into F3, which is now the border color
        rgbFgn3(rgbF2);

        //
        //  If in not in text fx mode, then move F3 to F2, so that whatever
        //  was being used for the shadow will continue to be.
        //
        if (eTextFX() == tCIDGraphDev::ETextFX::None)
            rgbFgn2(rgbF3);
    }

    // If prior to 5, then get rid of the pressed image
    if (c2FmtVersion < 5)
        bDeleteImgKey(L"Pressed");

    // Reset any runtime stuff
    m_eCurState = tCQCIntfEng::EBoolStates::Error;
}


tCIDLib::TVoid TCQCIntfCheckBoxBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CheckBox::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream out all the mixins
    MCQCIntfFontIntf::WriteOutFont(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);
    MCQCIntfBooleanIntf::WriteOutBool(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << m_eHLayout
                    << m_strFalseText
                    << m_strTrueText
                    << m_bUseExtraClrs
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfCheckBoxBase::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do the basic stuff
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // And draw the box with current settings. Move in an clip again
    grgnClip.Deflate(1, 1);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    DrawBox(gdevTarget, areaInvalid);
}



// ---------------------------------------------------------------------------
//  TCQCIntfCheckBoxBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// A convenience that indicates wether we are in checked state or not
tCIDLib::TBoolean TCQCIntfCheckBoxBase::bIsChecked() const
{
    return (m_eCurState == tCQCIntfEng::EBoolStates::True);
}


//
//  Probably writeability will only ever apply to the field based derivative,
//  it wasn't worth re-factoring this up to that level. So we just left it down
//  and provide a method for derivatives to use it. It might prove useful in
//  another derivatve at some point.
//
tCIDLib::TBoolean TCQCIntfCheckBoxBase::bWriteable() const
{
    return m_bWriteable;
}

tCIDLib::TBoolean TCQCIntfCheckBoxBase::bWriteable(const tCIDLib::TBoolean bToSet)
{
    m_bWriteable = bToSet;
    return m_bWriteable;
}


// Get/set the text/image layout style
tCQCIntfEng::EHTxtImgLayouts TCQCIntfCheckBoxBase::eHLayout() const
{
    return m_eHLayout;
}

tCQCIntfEng::EHTxtImgLayouts
TCQCIntfCheckBoxBase::eHLayout(const tCQCIntfEng::EHTxtImgLayouts eToSet)
{
    m_eHLayout = eToSet;
    return m_eHLayout;
}


// Give access to our current check state
tCQCIntfEng::EBoolStates TCQCIntfCheckBoxBase::eCheckState() const
{
    return m_eCurState;
}


//
//  The derived classes tell us whether we are in true or false or error state.
//  Only they know how to do determine that.
//
tCIDLib::TVoid
TCQCIntfCheckBoxBase::SetState( const   tCQCIntfEng::EBoolStates    eToSet
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
//  TCQCIntfCheckBoxBase: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfCheckBoxBase::DrawBox(          TGraphDrawDev&      gdevTarget
                                , const TArea&              areaInvalid)
{
    // If we are hidden, then do nothing
    if (eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
        return;

    //
    //  Get the current state out so we don't have to constantly call it
    //  below and set up a convenience for checking whether it's currently
    //  true.
    //
    const tCIDLib::TBoolean bChecked(m_eCurState == tCQCIntfEng::EBoolStates::True);

    //
    //  We want to know below whether the focus image for our current
    //  state is enabled.
    //
    const tCIDLib::TBoolean bUseFocus
    (
        (!bChecked && bImgEnabled(CQCIntfEng_CheckBox::strFalseFocusImg))
        || (bChecked && bImgEnabled(CQCIntfEng_CheckBox::strTrueFocusImg))
    );

    //
    //  Draw the border if needed, and adjust the drawing area. We draw it
    //  if we either have a border literally, or we have the focus and do not
    //  have a focus image to use. If we have a focus image, we'll use those
    //  (respectively) to indicate focus.
    //
    const tCIDLib::TBoolean bDrawBorder
    (
        bHasBorder() || (bHasFocus() && !bUseFocus)
    );

    // Get our area and set a region with it
    TArea areaDraw(areaActual());

    if (bDrawBorder)
    {
        if (bHasFocus())
            gdevTarget.Stroke(areaDraw, facCIDGraphDev().rgbRed);
        else
            gdevTarget.Stroke(areaDraw, rgbFgn3());
        areaDraw.Deflate(1);
    }

    // Set a clip region inside the border
    TRegionJanitor janClip(&gdevTarget, areaDraw, tCIDGraphDev::EClipModes::And);

    // If not transparent, fill the background
    if (!bIsTransparent())
        DoBgnFill(gdevTarget, areaInvalid);

    // Remember if we have a good value
    const tCIDLib::TBoolean bGotGoodValue
    (
        eCheckState() != tCQCIntfEng::EBoolStates::Error
    );

    // Move in again now to provide some extra margin for the text
    areaDraw.Deflate(1);

    //
    //  Get the appropriate image info from the image list mixin. If pressed,
    //  we always use that. Otherwise, if we have the focus and we have a
    //  focus image for the current state, then use the focus image. Else,
    //  use the true or false image according to our state, if we one one.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiCur = nullptr;
    if (bHasFocus() && bUseFocus)
    {
        if (bChecked)
            pimgiCur = pimgiForKeyEO(CQCIntfEng_CheckBox::strTrueFocusImg);
        else
            pimgiCur = pimgiForKeyEO(CQCIntfEng_CheckBox::strFalseFocusImg);
    }
     else
    {
        if (m_eCurState == tCQCIntfEng::EBoolStates::True)
            pimgiCur = pimgiForKeyEO(kCQCKit::pszFld_True);
        else if (m_eCurState == tCQCIntfEng::EBoolStates::False)
            pimgiCur = pimgiForKeyEO(kCQCKit::pszFld_False);
        else
            pimgiCur = pimgiForKeyEO(kCQCIntfEng_::pszImgKey_Error);
    }

    // Decide what text to display
    const TString* pstrToDraw;
    if (m_eCurState == tCQCIntfEng::EBoolStates::True)
        pstrToDraw = &m_strTrueText;
    else if (m_eCurState == tCQCIntfEng::EBoolStates::False)
        pstrToDraw = &m_strFalseText;
    else
        pstrToDraw = &facCQCIntfEng().strErrText();

    // Set up our font
    TCQCIntfFontJan janFont(&gdevTarget, *this);


    // Get the width of the text as a single line, AFTER the font is set
//    const tCIDLib::TCard4 c4LineWidth = gdevTarget.areaString(*pstrToDraw).c4Width();

    // And draw the image if we found one
    TArea areaText = areaDraw;
    if (pimgiCur)
    {
        TArea areaImgSrc;
        TArea areaImgTar;
        {
            //
            //  Calculate the two areas, based on the layout setting, adjust the
            //  calculated text width by a bit to account for the fact that it might
            //  be a blurred text which is wider.
            //
            //  The image wins if not enough space for both fully.
            //
            areaImgSrc.Set(TPoint::pntOrigin, pimgiCur->m_cptrImg->szImage());
            areaImgTar = areaImgSrc;

            if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::TextLeft)
            {
                //
                //  The image gets the right part. The text gets the remainder minus
                //  the offset.
                //
                areaImgTar.RightJustifyIn(areaDraw, kCIDLib::True);

                if (areaImgTar.i4Left() > areaDraw.i4Left() + CQCIntfEng_CheckBox::i4TextOfs)
                {
                    //
                    //  There's space for at least some text, so clip it at the left of
                    //  the image plus the offset.
                    //
                    areaText.i4Right
                    (
                        areaImgTar.i4Left() - tCIDLib::TInt4(CQCIntfEng_CheckBox::i4TextOfs)
                    );
                }
                 else
                {
                    areaText.c4Width(0);
                }
            }
             else if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::Center)
            {
                //
                //  Deflate the text in slightly for more margin, but otherwise the same as
                //  the drawing area.
                //
                areaText.Deflate(2, 0);
                areaImgTar.CenterIn(areaDraw);
            }
             else if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::TextRight)
            {
                // Put the image on the left
                areaImgTar.LeftJustifyIn(areaDraw, kCIDLib::True);

                // Move the text area over past it plus the offset
                areaText.i4Left(areaImgTar.i4Right() + CQCIntfEng_CheckBox::i4TextOfs);

                //
                //  If that moved it past the drawing area, zero the width. Else clip
                //  the right side to the drawing area.
                //
                if (areaText.i4Left() >= areaDraw.i4Right())
                    areaText.c4Width(0);
                else
                    areaText.i4Right(areaDraw.i4Right());
            }
        }

        // Offset the value if an offset was applied
        areaImgTar.AdjustOrg(pimgiCur->m_i4HOffset, pimgiCur->m_i4VOffset);

        // And if pressed, then simulate a press down
        if (bPressed())
            areaImgTar.AdjustOrg(1, 1);

        // If the target area intersects the invalid area, then draw it.
        if (areaImgTar.bIntersects(areaInvalid))
        {
            //
            //  In this case, we don't call the facility helper method
            //  that usually is called to draw images, because we want
            //  to control the blit type explicitly and have special
            //  justification issues.
            //
            pimgiCur->m_cptrImg->bmpImage().AdvDrawOn
            (
                gdevTarget
                , areaImgSrc
                , areaImgTar
                , pimgiCur->m_cptrImg->bTransparent()
                , pimgiCur->m_cptrImg->c4TransClr()
                , pimgiCur->m_cptrImg->bmpMask()
                , tCIDGraphDev::EBltTypes::Clip
                , pimgiCur->m_c1Opacity
            );
        }
    }

    //
    //  If we have an overlay image, then handle that. We just center it,
    //  then apply offsets.
    //
    {
        MCQCIntfImgListIntf::TImgInfo* pimgiOver = 0;
        if (m_eCurState == tCQCIntfEng::EBoolStates::True)
            pimgiOver = pimgiForKeyEO(CQCIntfEng_CheckBox::strTrueOverlayImg);
        else if (m_eCurState == tCQCIntfEng::EBoolStates::False)
            pimgiOver = pimgiForKeyEO(CQCIntfEng_CheckBox::strFalseOverlayImg);

        if (pimgiOver)
        {
            TArea areaImg(areaDraw);
            if (bPressed())
                areaImg.AdjustOrg(1, 1);
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , kCIDLib::False
                , pimgiOver->m_cptrImg
                , areaImg
                , areaInvalid
                , pimgiOver->m_i4HOffset
                , pimgiOver->m_i4VOffset
                , pimgiOver->m_c1Opacity
            );
        }
    }

    //
    //  Set up the colors we'll use. Since we can use the extra colors for
    //  True state, the color we use depends on our state and whether we are
    //  configured to use state based text colors.
    //
    const TRGBClr* prgbText;
    const TRGBClr* prgbText2;
    if (m_bUseExtraClrs && bChecked)
    {
        prgbText = &rgbExtra();
        prgbText2 = &rgbExtra2();
    }
     else
    {
        prgbText = &rgbFgn();
        prgbText2 = &rgbFgn2();
    }

    //
    //  If we have some text to draw, and the text area intersects the
    //  invalid area, and we have a good value or we are in designer mode
    //  (which provides a fake good value) then draw it.
    //
    //  NOTE:   We don't use the font mixin's justification settings here.
    //          We have pre-calculated the text area based on the
    //          check box specific horizontal justification settings
    //          so we always do it centered.
    //
    if ((!pstrToDraw->bIsEmpty() && areaText.bIntersects(areaInvalid))
    &&  bGotGoodValue)
    {
        // If pressed, adjust the area
        if (bPressed())
            areaText.AdjustOrg(1, 1);

        tCIDLib::EHJustify eHJust = tCIDLib::EHJustify::Left;
        if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::Center)
            eHJust = tCIDLib::EHJustify::Center;
        else if (m_eHLayout == tCQCIntfEng::EHTxtImgLayouts::TextLeft)
            eHJust = tCIDLib::EHJustify::Right;

        if (eTextFX() != tCIDGraphDev::ETextFX::None)
        {
            civOwner().DrawTextFX
            (
                gdevTarget
                , *pstrToDraw
                , areaText
                , eTextFX()
                , *prgbText
                , *prgbText2
                , eHJust
                , tCIDLib::EVJustify::Center
                , bNoTextWrap()
                , pntOffset()
            );
        }
         else
        {
            gdevTarget.eBackMixMode(tCIDGraphDev::EBackMixModes::Transparent);

            // If we have drop text, then do that
            if (bShadow())
            {
                tCIDLib::TCard4 c4Drop = 1;
                if (c4FontHeight() > 24)
                      c4Drop = 2;
                TArea areaDrop(areaText);
                areaDrop.AdjustOrg(c4Drop, c4Drop);
                gdevTarget.SetTextColor(*prgbText2);

                if (bNoTextWrap())
                {
                    gdevTarget.DrawString
                    (
                        *pstrToDraw
                        , areaDrop
                        , eHJust
                        , tCIDLib::EVJustify::Center
                    );
                }
                 else
                {
                    gdevTarget.DrawMText
                    (
                        *pstrToDraw
                        , areaDrop
                        , eHJust
                        , tCIDLib::EVJustify::Center
                        , kCIDLib::True
                    );
                }
            }

            // If we have focus and no focus image, use the other color.
            if (bHasFocus() && !bUseFocus)
                gdevTarget.SetTextColor(*prgbText2);
            else
                gdevTarget.SetTextColor(*prgbText);

            if (bNoTextWrap())
            {
                gdevTarget.DrawString
                (
                    *pstrToDraw
                    , areaText
                    , eHJust
                    , tCIDLib::EVJustify::Center
                );
            }
             else
            {
                gdevTarget.DrawMText
                (
                    *pstrToDraw
                    , areaText
                    , eHJust
                    , tCIDLib::EVJustify::Center
                    , kCIDLib::True
                );
            }
        }
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFieldCheckBox: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFieldCheckBox::TCQCIntfFieldCheckBox() :

    TCQCIntfCheckBoxBase
    (
        CQCIntfEng_CheckBox::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CheckBox)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFieldCheckBox::TCQCIntfFieldCheckBox(const TCQCIntfFieldCheckBox& iwdgToCopy) :

    TCQCIntfCheckBoxBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFieldCheckBox::~TCQCIntfFieldCheckBox()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldCheckBox: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFieldCheckBox&
TCQCIntfFieldCheckBox::operator=(const TCQCIntfFieldCheckBox& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldCheckBox: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept a new field, and optionally to
//  adjust ourself to it. In our case we are expression driven and so can
//  basically accept about anything. We just tell our parent class if the
//  field is writeable or not if we are storing.
//
tCIDLib::TBoolean
TCQCIntfFieldCheckBox::bCanAcceptField( const   TCQCFldDef&         flddNew
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean   bStore)
{
    // Tell our parent if our new field is writeable
    if (bStore)
        bWriteable(flddNew.bIsWriteable());
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfFieldCheckBox::bIsSame(const  TCQCIntfWidget& iwdgSrc) const
{
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFieldCheckBox& iwdgOther
    (
        static_cast<const TCQCIntfFieldCheckBox&>(iwdgSrc)
    );

    // Do our mixins
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFieldCheckBox::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFieldCheckBox&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFieldCheckBox::eDoCmd(  const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfFieldCheckBox::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFieldCheckBox::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFieldCheckBox::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"State Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldCheckBox::RegisterFields(          TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFieldCheckBox::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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


tCIDLib::TVoid
TCQCIntfFieldCheckBox::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfFieldCheckBox::Validate(const   TCQCFldCache&           cfcData
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
//  TCQCIntfFieldCheckBox: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFieldCheckBox::FieldChanged(        TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    tCQCIntfEng::EBoolStates eCheckState;
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
            eCheckState = tCQCIntfEng::EBoolStates::False;
        else if (eRes == tCQCIntfEng::EExprResults::True)
            eCheckState = tCQCIntfEng::EBoolStates::True;
        else
            eCheckState = tCQCIntfEng::EBoolStates::Error;
    }
     else
    {
        eCheckState = tCQCIntfEng::EBoolStates::Error;
    }

    // And set the check state on our parent
    SetState(eCheckState, bNoRedraw);
}


tCIDLib::TVoid
TCQCIntfFieldCheckBox::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    bDesQueryFldInfo(eType, strLimits);
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfFieldCheckBox::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CheckBox::c2FldFmtVersion))
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
    //  In theory version 1 objects would require some adjustment to move over
    //  to the single field mixing scheme for association of fields, but we will
    //  never see such versions anymore. They cannot upgrade to anything post
    //  3.0 until they go to 3.0 first and it does all the refactoring and
    //  conversion.
    //
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid
TCQCIntfFieldCheckBox::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CheckBox::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any of our own stuff right now, so just the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldCheckBox::ValueUpdate(         TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticCheckBox: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticCheckBox::TCQCIntfStaticCheckBox() :

    TCQCIntfCheckBoxBase
    (
        CQCIntfEng_CheckBox::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CheckBox)
    )
    , m_bAutoToggle(kCIDLib::False)
{
}

TCQCIntfStaticCheckBox::
TCQCIntfStaticCheckBox(const TCQCIntfStaticCheckBox& iwdgSrc) :

    TCQCIntfCheckBoxBase(iwdgSrc)
    , m_bAutoToggle(iwdgSrc.m_bAutoToggle)
{
}

TCQCIntfStaticCheckBox::~TCQCIntfStaticCheckBox()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCheckBox: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticCheckBox&
TCQCIntfStaticCheckBox::operator=(const TCQCIntfStaticCheckBox& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_bAutoToggle = iwdgSrc.m_bAutoToggle;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCheckBox: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfStaticCheckBox::bIsSame(const  TCQCIntfWidget& iwdgSrc) const
{
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticCheckBox& iwdgOther
    (
        static_cast<const TCQCIntfStaticCheckBox&>(iwdgSrc)
    );

    return (m_bAutoToggle == iwdgOther.m_bAutoToggle);
}


// Our callback to handle command invocation requests
tCQCKit::ECmdRes
TCQCIntfStaticCheckBox::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetState)
    {
        // The passed state has to be true or false
        tCQCIntfEng::EBoolStates eNewState;
        if (ccfgToDo.piAt(0).m_strValue == kCQCKit::pszFld_True)
            eNewState = tCQCIntfEng::EBoolStates::True;
        else if (ccfgToDo.piAt(0).m_strValue == kCQCKit::pszFld_False)
            eNewState = tCQCIntfEng::EBoolStates::False;
        else
            eNewState = tCQCIntfEng::EBoolStates::Error;

        // Set the state, redrawing if the event allows for that
        SetState(eNewState, !bCanRedraw(strEventId));
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


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfStaticCheckBox::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticCheckBox&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfStaticCheckBox::Initialize( TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Tell our parent class that the value is 'writeable'. We won't do
    //  anything with it. And set the initial state so it will get out of
    //  the initial error state.
    //
    bWriteable(kCIDLib::True);
    SetState(tCQCIntfEng::EBoolStates::False, kCIDLib::True);
}


//
//  We override this to handle auto-toggle. We toggle the state, then call our parent to
//  let him invoke any actions.
//
tCIDLib::TVoid TCQCIntfStaticCheckBox::Invoke()
{
    if (eCheckState() == tCQCIntfEng::EBoolStates::False)
        SetState(tCQCIntfEng::EBoolStates::True, kCIDLib::True);
    else if (eCheckState() == tCQCIntfEng::EBoolStates::True)
        SetState(tCQCIntfEng::EBoolStates::False, kCIDLib::True);

    TParent::Invoke();
}


// Return the list of commands we support
tCIDLib::TVoid
TCQCIntfStaticCheckBox::QueryCmds(          TCollection<TCQCCmd>&   colToFill
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first
    TParent::QueryCmds(colToFill, eContext);

    // Set the state of the check box
    colToFill.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetState
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetState)
            , tCQCKit::ECmdPTypes::Boolean
            , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
        )
    );
}


tCIDLib::TVoid
TCQCIntfStaticCheckBox::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // And add our own
    adatTmp.Set(L"Static Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Auto-Toggle"
        , kCQCIntfEng::strAttr_Bool_AutoToggle
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bAutoToggle);
    colAttrs.objAdd(adatTmp);
}



tCIDLib::TVoid
TCQCIntfStaticCheckBox::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Store our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Bool_AutoToggle)
        m_bAutoToggle = adatNew.bVal();
}


// Do any runtime validation of attributes
tCIDLib::TVoid
TCQCIntfStaticCheckBox::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticCheckBox: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfStaticCheckBox::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CheckBox::c2StaticFmtVersion))
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

    // If V2 or beyond, read in the auto-toggle, else default
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bAutoToggle;
    else
        m_bAutoToggle = kCIDLib::False;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfStaticCheckBox::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CheckBox::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    strmToWriteTo   << m_bAutoToggle
                    << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarCheckBox
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarCheckBox: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarCheckBox::TCQCIntfVarCheckBox() :

    TCQCIntfCheckBoxBase
    (
        CQCIntfEng_CheckBox::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_CheckBox)
    )
{
}

TCQCIntfVarCheckBox::TCQCIntfVarCheckBox(const TCQCIntfVarCheckBox& iwdgToCopy) :

    TCQCIntfCheckBoxBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarCheckBox::~TCQCIntfVarCheckBox()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCheckBox: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarCheckBox&
TCQCIntfVarCheckBox::operator=(const TCQCIntfVarCheckBox& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCheckBox: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfVarCheckBox::bIsSame(const  TCQCIntfWidget& iwdgSrc) const
{
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarCheckBox& iwdgOther
    (
        static_cast<const TCQCIntfVarCheckBox&>(iwdgSrc)
    );

    // Do our mixins
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarCheckBox::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarCheckBox&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarCheckBox::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfVarCheckBox::QueryCmds(         TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarCheckBox::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Tell our parent class that the value is 'writeable', since our
    //  variables are always writable, so we can do it once and just leave
    //  it that way.
    //
    bWriteable(kCIDLib::True);
}


tCIDLib::TVoid
TCQCIntfVarCheckBox::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarCheckBox::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFull, regxFind);
}


tCIDLib::TVoid
TCQCIntfVarCheckBox::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarCheckBox::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarCheckBox: Protected, inherited methods
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
tCIDLib::TBoolean TCQCIntfVarCheckBox::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfVarCheckBox::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    eType = tCQCKit::EFldTypes::String;
    strLimits.Clear();
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfVarCheckBox::StreamFrom(TBinInStream& strmToReadFrom)
{
    // And we should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_CheckBox::c2VarFmtVersion))
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
    MCQCIntfVarIntf::ReadInVar(strmToReadFrom);

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid
TCQCIntfVarCheckBox::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_CheckBox::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any of our own stuff right now, so just the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarCheckBox::ValueUpdate(           TCQCPollEngine&
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
TCQCIntfVarCheckBox::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetState(tCQCIntfEng::EBoolStates::Error, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarCheckBox::VarValChanged( const   TCQCActVar&         varNew
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    tCQCIntfEng::EBoolStates eToSet = tCQCIntfEng::EBoolStates::Error;

    // Ask the boolean interface to evalute the value based on configured statements
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


