//
// FILE NAME: CQCIntfEng_AnimatedImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2005
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
//  This file implements a
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
#include    "CQCIntfEng_AnimatedImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfAnimatedImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldAnimatedImg, TCQCIntfAnimatedImgBase)
AdvRTTIDecls(TCQCIntfStaticAnimatedImg, TCQCIntfAnimatedImgBase)
AdvRTTIDecls(TCQCIntfVarAnimatedImg, TCQCIntfAnimatedImgBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_AnimatedImg
    {
        // -----------------------------------------------------------------------
        //  The base class persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 2;


        // -----------------------------------------------------------------------
        //  Derived class persistent format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2StaticFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2VarFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAnimatedImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAnimatedImgBase::
TCQCIntfAnimatedImgBase(const   tCQCIntfEng::ECapFlags  eCaps
                        , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
    , m_bSkipFirst(kCIDLib::False)
    , m_c4CurIndex(0)
    , m_enctLast(0)
    , m_eCurState(tCQCIntfEng::EBoolStates::Error)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    //
    //  Load up 8 possible images that they can set up for us to cycle through.
    //  They don't have to enable them all.
    //
    TString strKey;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < 8; c4Index++)
    {
        strKey = L"#";
        strKey.AppendFormatted(c4Index + 1);
        AddNewImgKey(strKey, tCQCIntfEng::EImgReqFlags::Optional);
    }

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/AnimatedImg");
}

TCQCIntfAnimatedImgBase::
TCQCIntfAnimatedImgBase(const TCQCIntfAnimatedImgBase& iwdgToCopy) :

    TCQCIntfWidget(iwdgToCopy)
    , MCQCIntfBooleanIntf(iwdgToCopy)
    , MCQCIntfImgListIntf(iwdgToCopy)
    , m_bSkipFirst(iwdgToCopy.m_bSkipFirst)
    , m_c4CurIndex(iwdgToCopy.m_c4CurIndex)
    , m_enctLast(iwdgToCopy.m_enctLast)
    , m_eCurState(iwdgToCopy.m_eCurState)
{
}

TCQCIntfAnimatedImgBase::~TCQCIntfAnimatedImgBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Public operators
// ---------------------------------------------------------------------------
TCQCIntfAnimatedImgBase&
TCQCIntfAnimatedImgBase::operator=(const TCQCIntfAnimatedImgBase& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfBooleanIntf::operator=(iwdgToAssign);
        MCQCIntfImgListIntf::operator=(iwdgToAssign);

        m_bSkipFirst = iwdgToAssign.m_bSkipFirst;
        m_c4CurIndex = iwdgToAssign.m_c4CurIndex;
        m_enctLast   = iwdgToAssign.m_enctLast;
        m_eCurState  = iwdgToAssign.m_eCurState;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfAnimatedImgBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfAnimatedImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfAnimatedImgBase& iwdgOther
    (
        static_cast<const TCQCIntfAnimatedImgBase&>(iwdgSrc)
    );

    if (m_bSkipFirst != iwdgOther.m_bSkipFirst)
        return kCIDLib::False;

    // Do our mixins
    if (!MCQCIntfBooleanIntf::bSameBool(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfAnimatedImgBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    //
    //  If in designer mode, then just set ourself to active and the index
    //  to zero, so it'll display the first image in the set. If we are set
    //  up to skip the first image, then set it to 1 if we have more than
    //  one.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        m_eCurState = tCQCIntfEng::EBoolStates::True;
        m_c4CurIndex = 0;
        if (m_bSkipFirst && (c4ImgKeyCount() > 1))
            m_c4CurIndex = 1;
    }
}


tCIDLib::TVoid TCQCIntfAnimatedImgBase::QueryContentSize(TSize& szToFill)
{
    // Get the area of the largest of our images
    TArea areaBounds = areaImgBounds();

    // If we have a border, then increase by one
    if (bHasBorder())
        areaBounds.Inflate(1);
    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfAnimatedImgBase::
QueryReferencedImgs(
                            tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfAnimatedImgBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);

    //
    //  Do our mixins. Only do boolean stuff if not a static. Statics don't have any
    //  source value to evaluate.
    //
    if (!bIsA(TCQCIntfStaticAnimatedImg::clsThis()))
        MCQCIntfBooleanIntf::QueryBoolAttrs(colAttrs, adatTmp);

    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    // Add our own stuff
    adatTmp.Set(L"Animation Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Skip First"
        , kCQCIntfEng::strAttr_Animate_SkipFirst
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bSkipFirst);
    colAttrs.objAdd(adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfAnimatedImgBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfAnimatedImgBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Expressions))
        MCQCIntfBooleanIntf::ReplaceBool(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfAnimatedImgBase::szDefaultSize() const
{
    return MCQCIntfImgListIntf::szImgDefault();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfAnimatedImgBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfBooleanIntf::SetBoolAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Animate_SkipFirst)
        m_bSkipFirst = adatNew.bVal();
}


tCIDLib::TVoid
TCQCIntfAnimatedImgBase::Validate(  const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfAnimatedImgBase::bSkipFirst() const
{
    return m_bSkipFirst;
}

tCIDLib::TBoolean
TCQCIntfAnimatedImgBase::bSkipFirst(const tCIDLib::TBoolean bToSet)
{
    m_bSkipFirst = bToSet;

    // If in designer mode, update our default display info
    if (TFacCQCIntfEng::bDesMode())
    {
        m_c4CurIndex = 0;
        if (m_bSkipFirst && (c4ImgKeyCount() > 1))
            m_c4CurIndex = 1;
    }
    return m_bSkipFirst;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfAnimatedImgBase::ActiveUpdate(  const   tCIDLib::TBoolean   bNoRedraw
                                        , const TGUIRegion&         grgnClip
                                        , const tCIDLib::TBoolean   bInTopLayer)
{
    //
    //  If it's been 500ms since our last one, then we want to move to the
    //  next image.
    //
    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    if ((enctNow - m_enctLast) > (500 * kCIDLib::enctOneMilliSec))
    {
        // Store now as the new last time and move forward
        m_enctLast = enctNow;

        const tCIDLib::TCard4 c4InitIndex = m_c4CurIndex;

        // If in true state, animate, else just stay on the 0th one
        if (m_eCurState == tCQCIntfEng::EBoolStates::True)
        {
            m_c4CurIndex++;

            //
            //  If we've wrapped, then go back to zero. We wrap if we either
            //  hit the image count or we hit an unenabled image key.
            //
            const tCIDLib::TCard4 c4ImgCount = c4ImgKeyCount();
            if ((m_c4CurIndex >= c4ImgCount) || !imgiAt(m_c4CurIndex).m_bEnabled)
                m_c4CurIndex = 0;

            //
            //  If we are skipping the first image, and we ended up on zero
            //  then move forward if we can.
            //
            if (m_bSkipFirst && !m_c4CurIndex && (c4ImgCount > 1))
                m_c4CurIndex++;
        }
         else
        {
            m_c4CurIndex = 0;
        }

        //
        //  If we selected a new image and we are told we can redraw, and
        //  our current state indicates we can draw, and we are not hidden
        //  manually or by states, then redraw.
        //
        if ((m_c4CurIndex != c4InitIndex)
        &&  !bNoRedraw
        &&  bInTopLayer
        &&  (eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden))
        {
            Redraw();
        }
    }
}


tCIDLib::TVoid TCQCIntfAnimatedImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_AnimatedImg::c2BaseFmtVersion))
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
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // If V2 or greater, read in the skip first flag, else skip it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bSkipFirst;
    else
        m_bSkipFirst = kCIDLib::False;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfAnimatedImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_AnimatedImg::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfBooleanIntf::WriteOutBool(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // Stream out our stuff and the end marker
    strmToWriteTo   << m_bSkipFirst
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfAnimatedImgBase::Update(        TGraphDrawDev&      gdevTarget
                                , const TArea&              areaInvalid
                                ,       TGUIRegion&         grgnClip)
{
    // Call our parent first
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Get the remaining area left to draw within any border
    TArea areaDraw(grgnClip.areaBounds());

    // Get the count of defined images. If none, then we are done
    const tCIDLib::TCard4 c4ImgCount = c4ImgKeyCount();
    if (c4ImgCount)
    {
        // Get our current image info
        TImgInfo& imgiCur = imgiAt(m_c4CurIndex);

        //
        //  If it has a good value, then display the image. When in
        //  designer mode, we forced a default value to the first image
        //  image key (or second if igore first image is set) during
        //  init and forced off the initial error state.
        //
        if (m_eCurState != tCQCIntfEng::EBoolStates::Error)
        {
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , kCIDLib::False
                , imgiCur.m_cptrImg
                , areaDraw
                , areaInvalid
                , imgiCur.m_i4HOffset
                , imgiCur.m_i4VOffset
                , imgiCur.m_c1Opacity
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfAnimatedImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes tell us what our current state should be
tCIDLib::TVoid
TCQCIntfAnimatedImgBase::SetState(  const   tCQCIntfEng::EBoolStates eToSet
                                    , const tCIDLib::TBoolean        bNoRedraw)
{
    if (m_eCurState != eToSet)
    {
        m_eCurState = eToSet;

        // If our current state is false, we got bake to the 0th image
        if (m_eCurState == tCQCIntfEng::EBoolStates::False)
            m_c4CurIndex = 0;

        if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
            Redraw();
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldAnimatedImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldAnimatedImg::TCQCIntfFldAnimatedImg() :

    TCQCIntfAnimatedImgBase
    (
        CQCIntfEng_AnimatedImg::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_AnimatedImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldAnimatedImg::
TCQCIntfFldAnimatedImg(const TCQCIntfFldAnimatedImg& iwdgToCopy) :

    TCQCIntfAnimatedImgBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldAnimatedImg::~TCQCIntfFldAnimatedImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldAnimatedImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldAnimatedImg&
TCQCIntfFldAnimatedImg::operator=(const TCQCIntfFldAnimatedImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldAnimatedImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept a field, and optionally to have us
//  update to conform to that field. In our case we can pretty much accept
//  anything since we are driven by expressions.
//
tCIDLib::TBoolean
TCQCIntfFldAnimatedImg::bCanAcceptField(const   TCQCFldDef&
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfFldAnimatedImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldAnimatedImg& iwdgOther
    (
        static_cast<const TCQCIntfFldAnimatedImg&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFldAnimatedImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldAnimatedImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldAnimatedImg::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfFldAnimatedImg::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldAnimatedImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldAnimatedImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Find the expression

    // Handle our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs
    (
        colAttrs, adatTmp, L"Start/Stop Field"
    );
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldAnimatedImg::RegisterFields(         TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldAnimatedImg::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldAnimatedImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  At this level, we just call our parent and any mixins, to let them
//  do any required validation.
//
tCIDLib::TVoid
TCQCIntfFldAnimatedImg::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Sall our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixines
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldAnimatedImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFldAnimatedImg::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
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
TCQCIntfFldAnimatedImg::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    bDesQueryFldInfo(eType, strLimits);
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfFldAnimatedImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_AnimatedImg::c2FldFmtVersion))
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
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfFldAnimatedImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_AnimatedImg::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}



// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldAnimatedImg::ValueUpdate(        TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticAnimatedImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticAnimatedImg::TCQCIntfStaticAnimatedImg() :

    TCQCIntfAnimatedImgBase
    (
        CQCIntfEng_AnimatedImg::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_AnimatedImg)
    )
    , MCQCIntfStaticIntf(tCQCKit::EFldTypes::Boolean)
{
}

TCQCIntfStaticAnimatedImg::
TCQCIntfStaticAnimatedImg(const TCQCIntfStaticAnimatedImg& iwdgToCopy) :

    TCQCIntfAnimatedImgBase(iwdgToCopy)
    , MCQCIntfStaticIntf(iwdgToCopy)
{
}

TCQCIntfStaticAnimatedImg::~TCQCIntfStaticAnimatedImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticAnimatedImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticAnimatedImg&
TCQCIntfStaticAnimatedImg::operator=(const TCQCIntfStaticAnimatedImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfStaticIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticAnimatedImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticAnimatedImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticAnimatedImg& iwdgOther
    (
        static_cast<const TCQCIntfStaticAnimatedImg&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfStaticIntf::bSameStatic(iwdgOther);
}


// Polymorphically copy us
tCIDLib::TVoid
TCQCIntfStaticAnimatedImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticAnimatedImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfStaticAnimatedImg::eDoCmd(const TCQCCmdCfg&             ccfgToDo
                                , const tCIDLib::TCard4         c4Index
                                , const TString&                strUserId
                                , const TString&                strEventId
                                ,       TStdVarsTar&            ctarGlobals
                                ,       tCIDLib::TBoolean&      bResult
                                ,       TCQCActEngine&          acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetValue)
    {
        // The value has to be true or false, else we reject it
        tCQCIntfEng::EBoolStates eNewState = tCQCIntfEng::EBoolStates::Error;
        if (ccfgToDo.piAt(0).m_strValue == kCQCKit::pszFld_True)
            eNewState = tCQCIntfEng::EBoolStates::True;
        else if (ccfgToDo.piAt(0).m_strValue == kCQCKit::pszFld_False)
            eNewState = tCQCIntfEng::EBoolStates::False;

        if (eNewState == tCQCIntfEng::EBoolStates::Error)
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcCmd_BadParmValue
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TCardinal(c4Index + 1)
                , kCQCKit::strCmdId_SetValue
            );
        }
        SetState(eNewState, kCIDLib::False);
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


// Called to initialize us after loading
tCIDLib::TVoid
TCQCIntfStaticAnimatedImg::Initialize(  TCQCIntfContainer* const    piwdgParent
                                        , TDataSrvClient&           dsclInit
                                        , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Initialize our parent up front since we don't ever get actually
    //  updated other than via action command.
    //
    SetState(tCQCIntfEng::EBoolStates::True, kCIDLib::True);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfStaticAnimatedImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We don't support anything in a preload context
    if (eContext == tCQCKit::EActCmdCtx::Preload)
        return;

    // And add our commands
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_SetValue
                , facCQCKit().strMsg(kKitMsgs::midCmd_SetValue)
                , tCQCKit::ECmdPTypes::Boolean
                , facCQCKit().strMsg(kKitMsgs::midCmdP_State)
            )
        );
    }
}


tCIDLib::TVoid
TCQCIntfStaticAnimatedImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    //
    //  Note we don't load up the static interface, because our type is boolean
    //  and has to stay that way, so there's nothing that can be changed.
    //
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticAnimatedImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                        , const TAttrData&      adatNew
                                        , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticAnimatedImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Handle streaming us in and out
tCIDLib::TVoid TCQCIntfStaticAnimatedImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion
    ||  (c2FmtVersion > CQCIntfEng_AnimatedImg::c2StaticFmtVersion))
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
    MCQCIntfStaticIntf::ReadInStatic(strmToReadFrom);

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfStaticAnimatedImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_AnimatedImg::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfStaticIntf::WriteOutStatic(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarAnimatedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarAnimatedImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarAnimatedImg::TCQCIntfVarAnimatedImg() :

    TCQCIntfAnimatedImgBase
    (
        CQCIntfEng_AnimatedImg::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_AnimatedImg)
    )
{
}

TCQCIntfVarAnimatedImg::TCQCIntfVarAnimatedImg(const TCQCIntfVarAnimatedImg& iwdgToCopy) :

    TCQCIntfAnimatedImgBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarAnimatedImg::~TCQCIntfVarAnimatedImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarAnimatedImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarAnimatedImg&
TCQCIntfVarAnimatedImg::operator=(const TCQCIntfVarAnimatedImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarAnimatedImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarAnimatedImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarAnimatedImg& iwdgOther
    (
        static_cast<const TCQCIntfVarAnimatedImg&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarAnimatedImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarAnimatedImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarAnimatedImg::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfVarAnimatedImg::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarAnimatedImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                        , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Handle our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}




tCIDLib::TVoid
TCQCIntfVarAnimatedImg::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarAnimatedImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarAnimatedImg::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarAnimatedImg: Protected, inherited methods
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
tCIDLib::TBoolean TCQCIntfVarAnimatedImg::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarAnimatedImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_AnimatedImg::c2VarFmtVersion))
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


tCIDLib::TVoid
TCQCIntfVarAnimatedImg::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    eType = tCQCKit::EFldTypes::String;
    strLimits.Clear();
}


tCIDLib::TVoid TCQCIntfVarAnimatedImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_AnimatedImg::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}



// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarAnimatedImg::ValueUpdate(        TCQCPollEngine&
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
TCQCIntfVarAnimatedImg::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetState(tCQCIntfEng::EBoolStates::Error, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarAnimatedImg::VarValChanged(  const   TCQCActVar&         varNew
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

