//
// FILE NAME: CQCIntfEng_LogoImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/02/2004
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
//  This file implements a specialized static image widget, which displays
//  the appropriate image for the current channel field of various devices.
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
#include    "CQCIntfEng_LogoImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfDTVLogoImg,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_DTVLogoImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        //
        //  Version 2 -
        //      Support for field association was moved from the base widget class
        //      to a mixin, so we need to know if we have already converted or not.
        //
        //  Version 3 -
        //      Update path for 5.x format.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 3;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfDTVLogoImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Private, static data
// ---------------------------------------------------------------------------
const TString TCQCIntfDTVLogoImg::strDTVScope(L"/System/Logos/TVChannels/");
const TString TCQCIntfDTVLogoImg::strXMScope(L"/System/Ripper/Logos/XMRadio/");


// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfDTVLogoImg::TCQCIntfDTVLogoImg() :

    TCQCIntfWidget
    (
        CQCIntfEng_DTVLogoImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_LogoImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
    , m_c1Opacity(0xFF)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
    , m_strRepoPath(strDTVScope)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/LogoImage");
}

TCQCIntfDTVLogoImg::TCQCIntfDTVLogoImg(const TCQCIntfDTVLogoImg& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
    , m_c1Opacity(iwdgSrc.m_c1Opacity)
    , m_cptrImg(iwdgSrc.m_cptrImg)
    , m_strRepoPath(iwdgSrc.m_strRepoPath)
{
}

TCQCIntfDTVLogoImg::~TCQCIntfDTVLogoImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfDTVLogoImg&
TCQCIntfDTVLogoImg::operator=(const TCQCIntfDTVLogoImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
        m_c1Opacity     = iwdgSrc.m_c1Opacity;
        m_cptrImg       = iwdgSrc.m_cptrImg;
        m_strRepoPath   = iwdgSrc.m_strRepoPath;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfDTVLogoImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfDTVLogoImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfDTVLogoImg& iwdgOther(static_cast<const TCQCIntfDTVLogoImg&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    return
    (
        (m_c1Opacity == iwdgOther.m_c1Opacity)
        && (m_strRepoPath == iwdgOther.m_strRepoPath)
    );
}


tCIDLib::TVoid
TCQCIntfDTVLogoImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfDTVLogoImg&>(iwdgSrc);
}



// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfDTVLogoImg::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetRepoPath)
    {
        //
        //  Update the repo path that we use. We just store the path,
        //  it'll get used next time we update. Make sure it ends with a
        //  slash, and that it uses forward slashes.
        //
        m_strRepoPath = ccfgToDo.piAt(0).m_strValue;
        m_strRepoPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);
        if (m_strRepoPath.chLast() != kCIDLib::chForwardSlash)
            m_strRepoPath.Append(kCIDLib::chForwardSlash);

        // Call a helper to updpate as required
        NewLogoImgVal(!bCanRedraw(strEventId));
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
TCQCIntfDTVLogoImg::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Init our parent. We have no init to do
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // If in designer mode, set us up to display a default image
    if (TFacCQCIntfEng::bDesMode())
        LoadLogoImg(L"0");
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfDTVLogoImg::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Set a new repository path dynamically at viewing time
    {
        TCQCCmd& cmdSetRepoPath = colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCIntfEng::strCmdId_SetRepoPath
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetRepoPath)
                , tCQCKit::ECmdPTypes::ImgPath
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_ImagePath)
            )
        );
    }


    // Change the field that drivers us
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


tCIDLib::TVoid TCQCIntfDTVLogoImg::QueryContentSize(TSize& szToFill)
{
    // We know our images are 62x46
    szToFill.Set(62, 46);

    // If we have a border, then increase by one
    if (bHasBorder())
        szToFill.Adjust(1);
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfDTVLogoImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


//
//  This is a special case, in that we could potentially reference any of the images
//  in our source scope.
//
tCIDLib::TVoid TCQCIntfDTVLogoImg::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       bIncludeScope) const
{
    // If doing scope based images, then return our source scope
    if (bIncludeScope)
    {
        tCIDLib::TBoolean bAdded;
        colScopes.objAddIfNew(m_strRepoPath, bAdded);
    }
}


// Provide widget palette summary info
tCIDLib::TVoid
TCQCIntfDTVLogoImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);

    // Update the names of any colors we keep
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Image Name Field");

    // Add ours
    adatTmp.Set(L"Logo Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // Do our stuff
    adatTmp.Set
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_LogoImg_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(tCIDLib::TCard4(m_c1Opacity));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Repo Path"
        , kCQCIntfEng::strAttr_LogoImg_RepoPath
        , TString::strEmpty()
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_ImageScope);
    adatTmp.SetString(m_strRepoPath);
    colAttrs.objAdd(adatTmp);
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfDTVLogoImg::RegisterFields(         TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfDTVLogoImg::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocField))
        MCQCIntfSingleFldIntf::ReplaceField(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strRepoPath, bRegEx, bFull, regxFind
        );
    }
}


// Return a default size for our type
TSize TCQCIntfDTVLogoImg::szDefaultSize() const
{
    TSize szRet(62, 46);

    // If we have a border, then increase by two
    if (bHasBorder())
        szRet.Adjust(2);
    return szRet;
}


tCIDLib::TVoid
TCQCIntfDTVLogoImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    // And store our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_LogoImg_Opacity)
        m_c1Opacity = tCIDLib::TCard1(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_LogoImg_RepoPath)
        strRepoPath(adatNew.strValue());
}


tCIDLib::TVoid
TCQCIntfDTVLogoImg::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Call any mixins
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Public, non-virtual methods
// ---------------------------------------------------------------------------
TBitmap TCQCIntfDTVLogoImg::bmpSample() const
{
    return m_cptrImg->bmpImage();
}


tCIDLib::TCard1 TCQCIntfDTVLogoImg::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfDTVLogoImg::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


const TString& TCQCIntfDTVLogoImg::strRepoPath() const
{
    return m_strRepoPath;
}

const TString& TCQCIntfDTVLogoImg::strRepoPath(const TString& strToSet)
{
    m_strRepoPath = strToSet;

    // Make sure it uses forward slashes and ends with a slash
    m_strRepoPath.bReplaceChar(kCIDLib::chBackSlash, kCIDLib::chForwardSlash);
    if (m_strRepoPath.chLast() != kCIDLib::chForwardSlash)
        m_strRepoPath.Append(kCIDLib::chForwardSlash);

    // If in des mode, update our sample image
    if (TFacCQCIntfEng::bDesMode())
    {
        LoadLogoImg(L"0");
        Invalidate(areaActual());
    }
    return m_strRepoPath;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  If our field mixin sees a change in our state or value, then we need
//  to update, so he'll call us here.
//
tCIDLib::TVoid
TCQCIntfDTVLogoImg::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    // Call a helper to do the right thing
    NewLogoImgVal(bNoRedraw);
}



tCIDLib::TVoid TCQCIntfDTVLogoImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DTVLogoImg::c2FmtVersion))
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

    // Read in our stuff
    strmToReadFrom  >> m_c1Opacity
                    >> m_strRepoPath;

    if (c2FmtVersion < 3)
        facCQCKit().Make50Path(m_strRepoPath);

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfDTVLogoImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_DTVLogoImg::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Do our stuff and the endmarker
    strmToWriteTo   << m_c1Opacity
                    << m_strRepoPath
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfDTVLogoImg::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip and draw into this remaining area
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    TArea areaDraw = grgnClip.areaBounds();

    facCQCIntfEng().DrawImage
    (
        gdevTarget
        , kCIDLib::False
        , m_cptrImg
        , areaDraw
        , areaInvalid
        , tCIDGraphDev::EPlacement::Center
        , m_c1Opacity
    );
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfDTVLogoImg::ValueUpdate(        TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}



// ---------------------------------------------------------------------------
//  TCQCIntfDTVLogoImg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfDTVLogoImg::LoadLogoImg(const TString& strKey)
{
    try
    {
        TDataSrvClient dsclLoad;
        LoadLogoImg(dsclLoad, strKey);
    }

    catch(...)
    {
        // We don't log this, we just clear out the image
        facCQCIntfEng().DropImgRef(m_cptrImg);
    }
}

tCIDLib::TVoid
TCQCIntfDTVLogoImg::LoadLogoImg(TDataSrvClient& dsclInit, const TString& strKey)
{
    try
    {
        // Build up the path to the default image
        TString strPath(m_strRepoPath);
        if (strPath.bIsEmpty())
            strPath = strDTVScope;

        if (strPath.chLast() != kCIDLib::chForwardSlash)
            strPath.Append(kCIDLib::chForwardSlash);
        strPath.Append(strKey);

        m_cptrImg = facCQCIntfEng().cptrGetImage(strPath, civOwner(), dsclInit);
    }

    catch(...)
    {
        // We don't log this, we just clear out the image
        facCQCIntfEng().DropImgRef(m_cptrImg);
    }
}


//
//  Called when we have a new value we want to load up the image for. We
//  get the path formatted out, then call the image loading method above.
//
tCIDLib::TVoid
TCQCIntfDTVLogoImg::NewLogoImgVal(const tCIDLib::TBoolean bNoRedraw)
{
    if (bGoodFieldValue())
    {
        TString strNewVal;
        fvCurrent().Format(strNewVal);

        //
        //  This is going to end up with the locale specific group separator
        //  in it if it's larger than the local group size, i.e. 1,234 instead
        //  of 1234. We don't want that, so strip it out.
        //
        TString strWS;
        strWS.Append(TLocale::chGroupSeparator());
        strNewVal.Strip(strWS, tCIDLib::EStripModes::Complete);

        LoadLogoImg(strNewVal);
    }
     else
    {
        facCQCIntfEng().DropImgRef(m_cptrImg);
    }


    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}

