//
// FILE NAME: CQCIntfEng_EnumImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/01/2003
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
//  This file implements a enumerated image widget. It displays one of a
//  list of images based on the value of an enumerated field.
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
#include    "CQCIntfEng_EnumImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfEnumImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldEnumImg, TCQCIntfEnumImgBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_EnumImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent formats
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  Our capabilities flags. We use the same for field and variables based
        //  widgets.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfEnumImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfEnumImgBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfEnumImgBase::~TCQCIntfEnumImgBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfEnumImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfEnumImgBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfEnumImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfEnumImgBase& iwdgOther
    (
        static_cast<const TCQCIntfEnumImgBase&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther))
        return kCIDLib::False;

    return kCIDLib::True;
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfEnumImgBase::eDoCmd(const   TCQCCmdCfg&             ccfgToDo
                            , const tCIDLib::TCard4         c4Index
                            , const TString&                strUserId
                            , const TString&                strEventId
                            ,       TStdVarsTar&            ctarGlobals
                            ,       tCIDLib::TBoolean&      bResult
                            ,       TCQCActEngine&          acteTar)
{
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
TCQCIntfEnumImgBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    //
    //  If in designer mode, then set our current value to the first enum
    //  value so that we'll display an image.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        const MCQCIntfImgListIntf::TImgInfo* pimgiFirst = pimgiFirstKey();
        if (pimgiFirst)
            m_strCurValue = pimgiFirst->m_strKey;
    }
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfEnumImgBase::QueryCmds(         TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

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


tCIDLib::TVoid TCQCIntfEnumImgBase::QueryContentSize(TSize& szToFill)
{
    //
    //  For us, this can be a bit complex. We want to basically calculate
    //  the bounds of any images (which can be offset) and the text.
    //
    //  So ask the image list mixin to do give us the bounds of any images,
    //  including offsets. It returns an origin centered area, which is what
    //  we want.
    //
    TArea areaBounds = areaImgBounds();

    // If none, then provide a little default size
    if (areaBounds.bIsEmpty())
        areaBounds.Set(0, 0, 32, 32);

    // If we have a border, then increase by one
    if (bHasBorder())
        areaBounds.Inflate(1);
    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfEnumImgBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfEnumImgBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove base attributes we don't use

    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfEnumImgBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfEnumImgBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfEnumImgBase::szDefaultSize() const
{
    return MCQCIntfImgListIntf::szImgDefault();
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfEnumImgBase::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(1).m_strValue = L"255";
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfEnumImgBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfEnumImgBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfEnumImgBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfEnumImgBase::
TCQCIntfEnumImgBase(const   tCQCIntfEng::ECapFlags  eCaps
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCaps, strTypeName)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/EnumeratedImage");
}

TCQCIntfEnumImgBase::TCQCIntfEnumImgBase(const TCQCIntfEnumImgBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_strCurValue(iwdgSrc.m_strCurValue)
{
}

TCQCIntfEnumImgBase&
TCQCIntfEnumImgBase::operator=(const TCQCIntfEnumImgBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        m_strCurValue = iwdgSrc.m_strCurValue;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfEnumImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfEnumImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion != CQCIntfEng_EnumImg::c2BaseFmtVersion))
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
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // We don't have any stuff of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime stuff
    m_strCurValue.Clear();
}


tCIDLib::TVoid TCQCIntfEnumImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and foramt version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_EnumImg::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // We don't have any of our own now, so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfEnumImgBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Get the remaining area, and draw into that
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    //
    //  If it has a good value, then display the image. When in designer mode,
    //  we forced a default value to the first keyimage key during init.
    //
    TArea areaDraw = grgnClip.areaBounds();
    if (!m_strCurValue.bIsEmpty())
    {
        // Get the appropriate image info from the image list mixin
        MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForKeyEO(m_strCurValue);

        if (pimgiCur)
        {
            // We found it, so draw it
            facCQCIntfEng().DrawImage
            (
                gdevTarget
                , kCIDLib::False
                , pimgiCur->m_cptrImg
                , areaDraw
                , areaInvalid
                , tCIDGraphDev::EPlacement::Center
                , pimgiCur->m_i4HOffset
                , pimgiCur->m_i4VOffset
                , pimgiCur->m_c1Opacity
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfEnumImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The derived classes call us here with the new enumerated limit string.
//  We parse out the values and load them into the image list.
//
tCIDLib::TVoid TCQCIntfEnumImgBase::SetEnum(const TString& strEnum)
{
    // And now reload our image list
    TVector<TString> colNewKeys(16);
    TCQCFldEnumLimit fldlTmp;
    tCIDLib::IgnoreRet(fldlTmp.bParseLimits(strEnum));
    fldlTmp.c4QueryValues(colNewKeys);

    //
    //  If the list is the same as before, it won't do anything. If they
    //  are different, it will clear the existing image list and load up
    //  a new one.
    //
    LoadNewImgKeys(colNewKeys);

    //
    //  If in designer mode, then set our current value to the first enum
    //  value so that we'll display an image.
    //
    if (TFacCQCIntfEng::bDesMode() && !colNewKeys.bIsEmpty())
        m_strCurValue = colNewKeys[0];
}


//
//  The derived class calls this at viewing time to set up a new value when
//  it changes. We just look up the image and if we find it, we store this
//  new value. Else, we store an empty string to indicate error state.
//
tCIDLib::TVoid
TCQCIntfEnumImgBase::SetValue(  const   TString&            strToSet
                                , const tCIDLib::TBoolean   bNoRedraw)
{
    //
    //  See if we have an image for this one. If not, then put us into error
    //  state, else put us into non-error state.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForKey(strToSet);
    if (pimgiCur)
        m_strCurValue = strToSet;
    else
        m_strCurValue.Clear();

    //
    //  If not hidden or supressed, then update with our new value, even if
    //  we got a bad value, since we want to display in error state.
    //
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldEnumImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldEnumImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldEnumImg::TCQCIntfFldEnumImg() :

    TCQCIntfEnumImgBase
    (
        CQCIntfEng_EnumImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_EnumImg)
    )
{
    // Set the field filter on the field interface
    TVector<TString> colLimitTypes;
    colLimitTypes.objAdd(TString(L"Enum"));
    SetFieldFilter
    (
        new TCQCFldFiltLimTypes(tCQCKit::EReqAccess::MReadCWrite, colLimitTypes)
    );
}

TCQCIntfFldEnumImg::TCQCIntfFldEnumImg(const TCQCIntfFldEnumImg& iwdgToCopy) :

    TCQCIntfEnumImgBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldEnumImg::~TCQCIntfFldEnumImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldEnumImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldEnumImg&
TCQCIntfFldEnumImg::operator=(const TCQCIntfFldEnumImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldEnumImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called when the designer needs to either see if we can accept a
//  field or to validate we can accept it and ask us to store it as our
//  new associated field.
//
tCIDLib::TBoolean
TCQCIntfFldEnumImg::bCanAcceptField(const   TCQCFldDef&         flddCheck
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean   bStore)
{
    // If not an enumerated string field, then we cannot accept it
    if ((flddCheck.eType() != tCQCKit::EFldTypes::String)
    ||  !flddCheck.strLimits().bStartsWith(L"Enum"))
    {
        return kCIDLib::False;
    }

    // Pass the limit string on to our parent if we are storing
    if (bStore)
        SetEnum(flddCheck.strLimits());

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfFldEnumImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldEnumImg& iwdgOther
    (
        static_cast<const TCQCIntfFldEnumImg&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    return kCIDLib::True;
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFldEnumImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldEnumImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldEnumImg::eDoCmd( const   TCQCCmdCfg&             ccfgToDo
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
TCQCIntfFldEnumImg::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldEnumImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}



tCIDLib::TVoid
TCQCIntfFldEnumImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Selector Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldEnumImg::RegisterFields(         TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldEnumImg::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldEnumImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
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
TCQCIntfFldEnumImg::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Sall our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixines
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldEnumImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFldEnumImg::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        SetValue
        (
            static_cast<const TCQCStringFldValue&>(fvCurrent()).strValue()
            , bNoRedraw
        );
    }
     else
    {
        SetValue(TString::strEmpty(), bNoRedraw);
    }
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfFldEnumImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_EnumImg::c2FldFmtVersion))
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

tCIDLib::TVoid TCQCIntfFldEnumImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_EnumImg::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldEnumImg::ValueUpdate(            TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}

