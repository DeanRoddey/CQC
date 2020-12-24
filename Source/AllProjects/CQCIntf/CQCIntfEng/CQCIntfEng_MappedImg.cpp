//
// FILE NAME: CQCIntfEng_MappedImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/25/2004
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
//  This file implements a mapped image widget. It displays one of a set of
//  images based on which of a set of expressions the current field value
//  matches.
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
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfMappedImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldMappedImg,TCQCIntfMappedImgBase)
AdvRTTIDecls(TCQCIntfVarMappedImg,TCQCIntfMappedImgBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_MappedImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent storage formats
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2VarFmtVersion = 1;


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
//   CLASS: TCQCIntfMappedImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfMappedImgBase::~TCQCIntfMappedImgBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfMappedImgBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfMappedImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMappedImgBase& iwdgOther
    (
        static_cast<const TCQCIntfMappedImgBase&>(iwdgSrc)
    );

    // Do our mixins.
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther))
        return kCIDLib::False;

    if (m_strDefKey != iwdgOther.m_strDefKey)
        return kCIDLib::False;

    // Compare the key/expression list
    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    if (c4Count != iwdgOther.m_colKeys.c4ElemCount())
        return kCIDLib::False;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TExprPair& kobjUs = m_colKeys[c4Index];
        const TExprPair& kobjThem = iwdgOther.m_colKeys[c4Index];

        if ((kobjUs.objKey() != kobjThem.objKey())
        ||  (kobjUs.objValue() != kobjThem.objValue()))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::Initialize(  TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up it's images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);
}


tCIDLib::TVoid TCQCIntfMappedImgBase::QueryContentSize(TSize& szToFill)
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
        areaBounds.Set(-8, -8, 16, 16);

    // If we have a border, then increase by one
    if (bHasBorder())
        areaBounds.Inflate(1);
    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfMappedImgBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);

    // For colors we keep, set the names of the ones that aren't already defaulted
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    //
    //  The default key, which we set up as an enum that has values from all of
    //  the currently defined keys.
    //
    TString strLims(kCIDMData::strAttrLim_EnumPref);
    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strLims.Append(kCIDLib::chComma);
        strLims.Append(m_colKeys[c4Index].objKey());
    }

    adatTmp.Set
    (
        L"Default Image"
        , kCQCIntfEng::strAttr_MappedImg_DefKey
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strDefKey);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));


    // For the mappings, we have to do that as a binary
    TBinMBufOutStream strmOut(1024UL);
    strmOut << m_colKeys << kCIDLib::FlushIt;
    colAttrs.objPlace
    (
        L"Mappings"
        , kCQCIntfEng::strAttr_MappedImg_Mappings
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , tCIDMData::EAttrEdTypes::Visual
    );
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfMappedImgBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::Replace( const   tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfMappedImgBase::szDefaultSize() const
{
    return MCQCIntfImgListIntf::szImgDefault();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMappedImgBase::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_MappedImg_DefKey)
    {
        m_strDefKey = adatNew.strValue();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_MappedImg_Mappings)
    {
        // Stream the content in
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_colKeys;

        //
        //  We also need to update the images list to set the images. If the actual
        //  names didn't end up changing, this will do nothing.
        //
        const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
        tCIDLib::TStrList colNewKeys(c4Count);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            colNewKeys.objAdd(m_colKeys[c4Index].objKey());

        MCQCIntfImgListIntf::LoadNewImgKeys(colNewKeys);
    }
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::Validate(const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfMappedImgBase::AddExprKey(  const   TString&        strKey
                                    , const TCQCExpression& exprToAdd)
{
    m_colKeys.objPlace(strKey, exprToAdd);
}


const TString& TCQCIntfMappedImgBase::strDefKey() const
{
    return m_strDefKey;
}

const TString& TCQCIntfMappedImgBase::strDefKey(const TString& strToSet)
{
    m_strDefKey = strToSet;
    return m_strDefKey;
}



// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfMappedImgBase::
TCQCIntfMappedImgBase(  const   tCQCIntfEng::ECapFlags  eCaps
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
    strCmdHelpId(L"/Reference/IntfWidgets/MappedImage");
}

TCQCIntfMappedImgBase::
TCQCIntfMappedImgBase(const TCQCIntfMappedImgBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_colKeys(iwdgSrc.m_colKeys)
    , m_strCurKey(iwdgSrc.m_strCurKey)
    , m_strDefKey(iwdgSrc.m_strDefKey)
{
}

TCQCIntfMappedImgBase&
TCQCIntfMappedImgBase::operator=(const TCQCIntfMappedImgBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        m_colKeys   = iwdgSrc.m_colKeys;
        m_strCurKey = iwdgSrc.m_strCurKey;
        m_strDefKey = iwdgSrc.m_strDefKey;
    }

    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfMappedImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCIntfEng_MappedImg::c2BaseFmtVersion)
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

    // And stream our stuff out
    strmToReadFrom  >> m_strDefKey
                    >> m_colKeys;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_MappedImg::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << m_strDefKey
                    << m_colKeys
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfMappedImgBase::Update(          TGraphDrawDev&  gdevTarget
                                , const TArea&          areaInvalid
                                ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    //
    //  If it has a good value, then display the image. When in designer mode,
    //  we force a default value to the first key image key.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        const MCQCIntfImgListIntf::TImgInfo* pimgiFirst = pimgiFirstKey();
        if (pimgiFirst)
            m_strCurKey = pimgiFirst->m_strKey;
    }

    if (!m_strCurKey.bIsEmpty())
    {
        MCQCIntfImgListIntf::TImgInfo* pimgiCur = 0;
        pimgiCur = pimgiForKeyEO(m_strCurKey);

        if (pimgiCur)
        {
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
//  TCQCIntfMappedImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The derived class calls this method when he finds out that his field or
//  variable is no longer available or in error state. We just clear the
//  current key.
//
tCIDLib::TVoid
TCQCIntfMappedImgBase::SetError(const tCIDLib::TBoolean bNoRedraw)
{
    m_strCurKey.Clear();

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


//
//  The derived classes call this to set a new value when they see their field
//  or variable change.
//
tCIDLib::TVoid
TCQCIntfMappedImgBase::SetValue(const   TCQCFldValue&       fvNew
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        ctarGlobalVars)
{
    //
    //  Run through our list of expressions and take the first one that
    //  matches and store it's key in our current key value. If none match,
    //  the set hte current key to the default key. If no default key was
    //  selected, then it'll get set to an empty string and we won't display
    //  anything.
    //
    m_strCurKey.Clear();
    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TExprPair& kobjCur = m_colKeys[c4Index];
        if (kobjCur.objValue().bEvaluate(fvNew, &ctarGlobalVars))
        {
            m_strCurKey = kobjCur.objKey();
            break;
        }
    }

    // If we didn't find one, then take the default
    if (m_strCurKey.bIsEmpty())
        m_strCurKey = m_strDefKey;

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


// ---------------------------------------------------------------------------
//  TCQCIntfMappedImgBase: Private, non-virtual methods
// ---------------------------------------------------------------------------
TCQCIntfMappedImgBase::TExprPair*
TCQCIntfMappedImgBase::pkobjFind(const TString& strKey)
{
    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCIntfMappedImgBase::TExprPair& kobjCur = m_colKeys[c4Index];
        if (kobjCur.objKey() == strKey)
            return &kobjCur;
    }
    return 0;
}


tCIDLib::TVoid TCQCIntfMappedImgBase::RemoveExprKey(const TString& strKey)
{
    const tCIDLib::TCard4 c4Count = m_colKeys.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfMappedImgBase::TExprPair& kobjCur = m_colKeys[c4Index];
        if (kobjCur.objKey() == strKey)
        {
            m_colKeys.RemoveAt(c4Index);
            break;
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldMappedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldMappedImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldMappedImg::TCQCIntfFldMappedImg() :

    TCQCIntfMappedImgBase
    (
        CQCIntfEng_MappedImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MappedImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
{
}

TCQCIntfFldMappedImg::TCQCIntfFldMappedImg(const TCQCIntfFldMappedImg& iwdgToCopy) :

    TCQCIntfMappedImgBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldMappedImg::~TCQCIntfFldMappedImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldMappedImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldMappedImg&
TCQCIntfFldMappedImg::operator=(const TCQCIntfFldMappedImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldMappedImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfFldMappedImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldMappedImg& iwdgOther
    (
        static_cast<const TCQCIntfFldMappedImg&>(iwdgSrc)
    );

    // Do our mixins.
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    // Nothing more at our level
    return kCIDLib::True;
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfFldMappedImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldMappedImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldMappedImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
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
TCQCIntfFldMappedImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldMappedImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldMappedImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Mapping Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldMappedImg::RegisterFields(       TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldMappedImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldMappedImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


//
//  At this level, we just call our parent and any mixins, to let them
//  do any required validation.
//
tCIDLib::TVoid
TCQCIntfFldMappedImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Sall our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our mixines
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFldMappedImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the field mixin when our field changes state or value
tCIDLib::TVoid
TCQCIntfFldMappedImg::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
        SetValue(fvCurrent(), bNoRedraw, ctarGlobalVars);
    else
        SetError(bNoRedraw);
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfFldMappedImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MappedImg::c2FldFmtVersion))
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

tCIDLib::TVoid TCQCIntfFldMappedImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_MappedImg::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldMappedImg::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarMappedImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarMappedImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarMappedImg::TCQCIntfVarMappedImg() :

    TCQCIntfMappedImgBase
    (
        CQCIntfEng_MappedImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MappedImg)
    )
{
}

TCQCIntfVarMappedImg::TCQCIntfVarMappedImg(const TCQCIntfVarMappedImg& iwdgToCopy) :

    TCQCIntfMappedImgBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarMappedImg::~TCQCIntfVarMappedImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarMappedImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarMappedImg&
TCQCIntfVarMappedImg::operator=(const TCQCIntfVarMappedImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarMappedImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarMappedImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarMappedImg& iwdgOther
    (
        static_cast<const TCQCIntfVarMappedImg&>(iwdgSrc)
    );

    // Do our mixins.
    if (!MCQCIntfVarIntf::bSameVariable(iwdgOther))
        return kCIDLib::False;

    // Nothing more at our level
    return kCIDLib::True;
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarMappedImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarMappedImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarMappedImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
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
TCQCIntfVarMappedImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarMappedImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarMappedImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfVarMappedImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarMappedImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarMappedImg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  When our variable shows up, our variable mixin will call us here to see
//  if we can accept it, so that we can set ourselves up to match it's
//  type and limits.
//
//  For us, we can basically accept anything.
//
tCIDLib::TBoolean TCQCIntfVarMappedImg::bCanAcceptVar(const TCQCActVar& varTest)
{
    return kCIDLib::True;
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarMappedImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MappedImg::c2VarFmtVersion))
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

tCIDLib::TVoid TCQCIntfVarMappedImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_MappedImg::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarMappedImg::ValueUpdate(          TCQCPollEngine&
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
TCQCIntfVarMappedImg::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    SetError(bNoRedraw);
}


//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that the value is good, so we were
//  either already read or have transitioned to ready state.
//
tCIDLib::TVoid
TCQCIntfVarMappedImg::VarValChanged(const   TCQCActVar&         varNew
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        ctarGlobalVars)
{
    SetValue(varNew.fvCurrent(), bNoRedraw, ctarGlobalVars);
}

