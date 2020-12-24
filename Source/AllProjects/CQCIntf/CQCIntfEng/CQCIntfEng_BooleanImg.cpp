//
// FILE NAME: CQCIntfEng_BooleanImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/20/2002
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
//  This file implements a boolean image widget. It displays one of two
//  images based on whether a field matches a true or false statement.
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
#include    "CQCIntfEng_BooleanImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfBooleanImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldBooleanImg,TCQCIntfBooleanImgBase)
AdvRTTIDecls(TCQCIntfStaticBooleanImg,TCQCIntfBooleanImgBase)
AdvRTTIDecls(TCQCIntfVarBooleanImg,TCQCIntfBooleanImgBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_BooleanImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Base class persistent versions
        //
        //      As of 2.4.17, we broke out two derivatives, one for field based
        //      and one for locally based images and one for variable based
        //      images. So these are all new classes with version 1.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion    = 1;
        constexpr tCIDLib::TCard2   c2FldFmtVersion     = 1;
        constexpr tCIDLib::TCard2   c2StaticFmtVersion  = 1;
        constexpr tCIDLib::TCard2   c2VarFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  Our capabilities flags. We use the same ones for field and variable
        //  based versions. STatics don't need the value update.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::None
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfBooleanImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfBooleanImgBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfBooleanImgBase::~TCQCIntfBooleanImgBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

// We can size to our contents at design time
tCIDLib::TBoolean TCQCIntfBooleanImgBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfBooleanImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfBooleanImgBase& iwdgOther
    (
        static_cast<const TCQCIntfBooleanImgBase&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfBooleanIntf::bSameBool(iwdgOther)
    ||  !MCQCIntfImgListIntf::bSameImgList(iwdgOther))
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfBooleanImgBase::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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

    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
    {
        SetImgOpacity
        (
            ccfgToDo.piAt(0).m_strValue
            , tCIDLib::TCard1(ccfgToDo.piAt(1).m_strValue.c4Val())
        );

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
TCQCIntfBooleanImgBase::Initialize( TCQCIntfContainer* const    piwdgParent
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


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfBooleanImgBase::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Do any mixins that have their own commands
    MCQCIntfBooleanIntf::QueryBoolCmds(colCmds, eContext);

    // Set the opacity of one of our images
    colCmds.objAdd
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
}


//
//  For us, this can be a bit complex. We want to basically calculate thje
//  bounds of any images (which can be offset).
//
//  So ask the image list mixin to do give us the bounds of any images,
//  including offsets. It returns an origin centered area, which is what
//  we want.
//
tCIDLib::TVoid
TCQCIntfBooleanImgBase::QueryContentSize(TSize& szToFill)
{
    TArea areaBounds = areaImgBounds();

    // If none, then provide a little default size
    if (areaBounds.bIsEmpty())
        areaBounds.Set(-8, -8, 16, 16);

    // If we have a border, then increase by one
    if (bHasBorder())
        areaBounds.Inflate(1);
    szToFill = areaBounds.szArea();
}



tCIDLib::TVoid TCQCIntfBooleanImgBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfBooleanImgBase::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
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
    //  Do our mixins. Boolean isn't required for statics since it has no source value
    //  to evaluate.
    //
    if (!bIsA(TCQCIntfStaticBooleanImg::clsThis()))
        MCQCIntfBooleanIntf::QueryBoolAttrs(colAttrs, adatTmp);
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfBooleanImgBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfBooleanImgBase::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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


// We just return a simple, arbitrary default size
TSize TCQCIntfBooleanImgBase::szDefaultSize() const
{
    return TSize(48, 48);
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfBooleanImgBase::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(1).m_strValue = L"255";
}


tCIDLib::TVoid
TCQCIntfBooleanImgBase::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfBooleanIntf::SetBoolAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
}


//
//  At this level, we just call our parent and any mixins, to let them
//  do any required validation.
//
tCIDLib::TVoid
TCQCIntfBooleanImgBase::Validate(const  TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixines
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanImgBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfBooleanImgBase::
TCQCIntfBooleanImgBase( const   tCQCIntfEng::ECapFlags  eCaps
                        , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
    , m_eCurState(tCQCIntfEng::EBoolStates::Error)
{
    // Load up the image keys into the image list mixin
    AddNewImgKey(kCQCKit::pszFld_False, tCQCIntfEng::EImgReqFlags::Required);
    AddNewImgKey(kCQCKit::pszFld_True, tCQCIntfEng::EImgReqFlags::Required);
    AddNewImgKey(kCQCIntfEng_::pszImgKey_Error, tCQCIntfEng::EImgReqFlags::Optional);

    // Set out default look
    bIsTransparent(kCIDLib::True);
    SetSize(TSize(48, 48), kCIDLib::False);

    // Set some default images
    SetImgKey
    (
        kCQCKit::pszFld_False
        , L"/System/Hardware/Icons/Light Off"
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
    );

    SetImgKey
    (
        kCQCKit::pszFld_True
        , L"/System/Hardware/Icons/Light On"
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/BooleanImage");
}

TCQCIntfBooleanImgBase::TCQCIntfBooleanImgBase(const TCQCIntfBooleanImgBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfBooleanIntf(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_eCurState(iwdgSrc.m_eCurState)
{
}

TCQCIntfBooleanImgBase&
TCQCIntfBooleanImgBase::operator=(const TCQCIntfBooleanImgBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfBooleanIntf::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        m_eCurState = iwdgSrc.m_eCurState;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------

// Stream in/out our contents
tCIDLib::TVoid TCQCIntfBooleanImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanImg::c2BaseFmtVersion))
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

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime stuff
    m_eCurState = tCQCIntfEng::EBoolStates::Error;
}

tCIDLib::TVoid TCQCIntfBooleanImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanImg::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfBooleanIntf::WriteOutBool(strmToWriteTo);
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// Called when we need to redraw
tCIDLib::TVoid
TCQCIntfBooleanImgBase::Update(         TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    // Call our parent first to do the standard stuff
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip to what's left, get the area and draw into that
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    TArea areaDraw = grgnClip.areaBounds();

    // Decide which image to display and display it if it's set
    const tCIDLib::TCh* pszKey;
    if (m_eCurState == tCQCIntfEng::EBoolStates::False)
        pszKey = kCQCKit::pszFld_False;
    else if (m_eCurState == tCQCIntfEng::EBoolStates::True)
        pszKey = kCQCKit::pszFld_True;
    else
        pszKey = kCQCIntfEng_::pszImgKey_Error;

    //
    //  Get the appropriate image info from the image list mixin. If it's
    //  enabled, then draw it.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForKeyEO(pszKey);
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
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfBooleanImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// The derived classes tell us what our current value should
tCIDLib::TVoid
TCQCIntfBooleanImgBase::SetState(const  tCQCIntfEng::EBoolStates   eToSet
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
//   CLASS: TCQCIntfFldBooleanImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldBooleanImg::TCQCIntfFldBooleanImg() :

    TCQCIntfBooleanImgBase
    (
        CQCIntfEng_BooleanImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_BooleanImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldBooleanImg::TCQCIntfFldBooleanImg(const TCQCIntfFldBooleanImg& iwdgToCopy) :

    TCQCIntfBooleanImgBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldBooleanImg::~TCQCIntfFldBooleanImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldBooleanImg&
TCQCIntfFldBooleanImg::operator=(const TCQCIntfFldBooleanImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfFldBooleanImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldBooleanImg& iwdgOther
    (
        static_cast<const TCQCIntfFldBooleanImg&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFldBooleanImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldBooleanImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldBooleanImg::eDoCmd(  const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfFldBooleanImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldBooleanImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldBooleanImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"State Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldBooleanImg::RegisterFields(          TCQCPollEngine& polleToUse
                                        , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldBooleanImg::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldBooleanImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
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
TCQCIntfFldBooleanImg::Validate(const   TCQCFldCache&           cfcData
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
            colErrs.objPlace(c4UniqueId(), strErr, kCIDLib::True, strWidgetId());
        }
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldBooleanImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFldBooleanImg::FieldChanged(        TCQCFldPollInfo&    cfpiAssoc
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
TCQCIntfFldBooleanImg::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    bDesQueryFldInfo(eType, strLimits);
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfFldBooleanImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanImg::c2FldFmtVersion))
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

tCIDLib::TVoid TCQCIntfFldBooleanImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanImg::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldBooleanImg::ValueUpdate(         TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticBooleanImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticBooleanImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticBooleanImg::TCQCIntfStaticBooleanImg() :

    TCQCIntfBooleanImgBase
    (
        CQCIntfEng_BooleanImg::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_BooleanImg)
    )
{
}

TCQCIntfStaticBooleanImg::
TCQCIntfStaticBooleanImg(const TCQCIntfStaticBooleanImg& iwdgToCopy) :

    TCQCIntfBooleanImgBase(iwdgToCopy)
{
}

TCQCIntfStaticBooleanImg::~TCQCIntfStaticBooleanImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticBooleanImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticBooleanImg&
TCQCIntfStaticBooleanImg::operator=(const TCQCIntfStaticBooleanImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticBooleanImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfStaticBooleanImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // For now we just have to pass it on to the parent
    return TParent::bIsSame(iwdgSrc);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfStaticBooleanImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticBooleanImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfStaticBooleanImg::eDoCmd(const  TCQCCmdCfg&             ccfgToDo
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

        //
        //  If we are in the preloda, then tell it not to redraw, just store
        //  the new state.
        //
        SetState(eNewState, strEventId == kCQCKit::strEvId_OnPreload);
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
TCQCIntfStaticBooleanImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                    , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Set the state of the image
    colCmds.objAdd
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
TCQCIntfStaticBooleanImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                    , const TString&                strSrc
                                    , const TString&                strTar
                                    , const tCIDLib::TBoolean       bRegEx
                                    , const tCIDLib::TBoolean       bFullMatch
                                    ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFullMatch, regxFind);
}


tCIDLib::TVoid
TCQCIntfStaticBooleanImg::Validate( const   TCQCFldCache&           cfcData
                                    ,       tCQCIntfEng::TErrList&  colErrs
                                    ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}



// ---------------------------------------------------------------------------
//  TCQCIntfStaticBooleanImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Stream in/out our contents
tCIDLib::TVoid
TCQCIntfStaticBooleanImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanImg::c2StaticFmtVersion))
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

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid
TCQCIntfStaticBooleanImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanImg::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarBooleanImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarBooleanImg::TCQCIntfVarBooleanImg() :

    TCQCIntfBooleanImgBase
    (
        CQCIntfEng_BooleanImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_BooleanImg)
    )
{
}

TCQCIntfVarBooleanImg::TCQCIntfVarBooleanImg(const TCQCIntfVarBooleanImg& iwdgToCopy) :

    TCQCIntfBooleanImgBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarBooleanImg::~TCQCIntfVarBooleanImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarBooleanImg&
TCQCIntfVarBooleanImg::operator=(const TCQCIntfVarBooleanImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarBooleanImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarBooleanImg& iwdgOther
    (
        static_cast<const TCQCIntfVarBooleanImg&>(iwdgSrc)
    );

    // Do our mixin, which is all that there is here
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarBooleanImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarBooleanImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarBooleanImg::eDoCmd(  const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfVarBooleanImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarBooleanImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarBooleanImg::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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


tCIDLib::TVoid
TCQCIntfVarBooleanImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarBooleanImg::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfVarBooleanImg: Protected, inherited methods
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
tCIDLib::TBoolean TCQCIntfVarBooleanImg::bCanAcceptVar(const TCQCActVar&)
{
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfVarBooleanImg::QueryBoolAttrLims(tCQCKit::EFldTypes& eType, TString& strLimits) const
{
    eType = tCQCKit::EFldTypes::String;
    strLimits.Clear();
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarBooleanImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_BooleanImg::c2VarFmtVersion))
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

tCIDLib::TVoid TCQCIntfVarBooleanImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_BooleanImg::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarBooleanImg::ValueUpdate(         TCQCPollEngine&
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
TCQCIntfVarBooleanImg::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetState(tCQCIntfEng::EBoolStates::Error, bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarBooleanImg::VarValChanged(const  TCQCActVar&         varNew
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



