//
// FILE NAME: CQCIntfEng_ImgIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/17/2003
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
//  This file implements the image options mixin.
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
namespace
{
    namespace CQCIntfEng_ImgIntf
    {
        // -----------------------------------------------------------------------
        //  Our persistence format
        //
        //  Version 2 -
        //      Added the 'hit transparent' flag for 4.4.908.
        //
        //  Version 3 -
        //      Conversion of paths to 5.x format
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 3;


        // -----------------------------------------------------------------------
        //  The key we store our attributes in when asked to put them into an
        //  attribute pack.
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszAttrPackKey = L"Image Attributes";
    }
}



// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfImgIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfImgIntf: Constructors and Destructor
// ---------------------------------------------------------------------------
MCQCIntfImgIntf::MCQCIntfImgIntf(const tCIDLib::TBoolean bDefaultUse) :

    m_bDefaultUse(bDefaultUse)
    , m_bHitTrans(kCIDLib::False)
    , m_bPrescale(kCIDLib::False)
    , m_bUseImage(bDefaultUse)
    , m_c1Opacity(0xFF)
    , m_cptrImage(facCQCIntfEng().cptrEmptyImg())
    , m_ePlacement(tCIDGraphDev::EPlacement::UpperLeft)
{
}

MCQCIntfImgIntf::MCQCIntfImgIntf(const MCQCIntfImgIntf& iwdgSrc) :

    m_bDefaultUse(iwdgSrc.m_bDefaultUse)
    , m_bHitTrans(iwdgSrc.m_bHitTrans)
    , m_bPrescale(iwdgSrc.m_bPrescale)
    , m_bUseImage(iwdgSrc.m_bUseImage)
    , m_c1Opacity(iwdgSrc.m_c1Opacity)
    , m_cptrImage(iwdgSrc.m_cptrImage)
    , m_ePlacement(iwdgSrc.m_ePlacement)
    , m_strImageName(iwdgSrc.m_strImageName)
    , m_szImage(iwdgSrc.m_szImage)
    , m_szPrescaled(iwdgSrc.m_szPrescaled)
    , m_szPreTarget(iwdgSrc.m_szPreTarget)
{
}

MCQCIntfImgIntf::~MCQCIntfImgIntf()
{
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgIntf: Public operators
// ---------------------------------------------------------------------------

//
//  In boof these, we don't really have to worry about the bitmap members. Those
//  are not in use at the point where these operators are invoked. They are set up
//  after the template content is in its final location.
//
MCQCIntfImgIntf& MCQCIntfImgIntf::operator=(const MCQCIntfImgIntf& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        m_bDefaultUse   = iwdgSrc.m_bDefaultUse;
        m_bHitTrans     = iwdgSrc.m_bHitTrans;
        m_bPrescale     = iwdgSrc.m_bPrescale;
        m_bUseImage     = iwdgSrc.m_bUseImage;
        m_c1Opacity     = iwdgSrc.m_c1Opacity;
        m_cptrImage     = iwdgSrc.m_cptrImage;
        m_ePlacement    = iwdgSrc.m_ePlacement;
        m_strImageName  = iwdgSrc.m_strImageName;
        m_szImage       = iwdgSrc.m_szImage;
        m_szPrescaled   = iwdgSrc.m_szPrescaled;
        m_szPreTarget   = iwdgSrc.m_szPreTarget;
    }
    return *this;
}


MCQCIntfImgIntf& MCQCIntfImgIntf::operator=(MCQCIntfImgIntf&& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        tCIDLib::Swap(m_bDefaultUse   , iwdgSrc.m_bDefaultUse);
        tCIDLib::Swap(m_bHitTrans     , iwdgSrc.m_bHitTrans);
        tCIDLib::Swap(m_bPrescale     , iwdgSrc.m_bPrescale);
        tCIDLib::Swap(m_bUseImage     , iwdgSrc.m_bUseImage);
        tCIDLib::Swap(m_c1Opacity     , iwdgSrc.m_c1Opacity);
        tCIDLib::Swap(m_cptrImage     , iwdgSrc.m_cptrImage);
        tCIDLib::Swap(m_ePlacement    , iwdgSrc.m_ePlacement);
        tCIDLib::Swap(m_strImageName  , iwdgSrc.m_strImageName);
        tCIDLib::Swap(m_szImage       , iwdgSrc.m_szImage);
        tCIDLib::Swap(m_szPrescaled   , iwdgSrc.m_szPrescaled);
        tCIDLib::Swap(m_szPreTarget   , iwdgSrc.m_szPreTarget);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  MCQCIntfImgIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the hit transparency flag
tCIDLib::TBoolean MCQCIntfImgIntf::bImgHitTrans() const
{
    return m_bHitTrans;
}

tCIDLib::TBoolean MCQCIntfImgIntf::bImgHitTrans(const tCIDLib::TBoolean bToSet)
{
    m_bHitTrans = bToSet;
    return m_bHitTrans;
}


// Compare us to the passed image mixin object
tCIDLib::TBoolean MCQCIntfImgIntf::bSameImg(const MCQCIntfImgIntf& miwdgSrc) const
{
    return ((m_ePlacement == miwdgSrc.m_ePlacement)
        &&  (m_strImageName == miwdgSrc.m_strImageName)
        &&  (m_bUseImage == miwdgSrc.m_bUseImage)
        &&  (m_c1Opacity == miwdgSrc.m_c1Opacity)
        &&  (m_bHitTrans == miwdgSrc.m_bHitTrans));
}


// Tell the caller if our image has alpha channel
tCIDLib::TBoolean MCQCIntfImgIntf::bSrcAlpha() const
{
    return m_cptrImage->bHasAlpha();
}


// Tell the caller if our image is a color transparency based image
tCIDLib::TBoolean MCQCIntfImgIntf::bTransparentImg() const
{
    return m_cptrImage->bTransparent();
}


// Get/set the flag that controls whether our image is displayed or not
tCIDLib::TBoolean MCQCIntfImgIntf::bUseImage() const
{
    return m_bUseImage;
}

tCIDLib::TBoolean MCQCIntfImgIntf::bUseImage(const tCIDLib::TBoolean bToSet)
{
    m_bUseImage = bToSet;

    // If no longer using an image, then reset the image info
    if (!m_bUseImage)
        ResetImage();
    return m_bUseImage;
}


//
//  Provide access to the pre-scaled image and mask. If no pre-scaling has been
//  asked for, then these are just the originals.
//
const TBitmap& MCQCIntfImgIntf::bmpPreImage() const
{
    return m_bmpPreImage;
}

const TBitmap& MCQCIntfImgIntf::bmpPreMask() const
{
    return m_bmpPreMask;
}


tCIDLib::TCard1 MCQCIntfImgIntf::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 MCQCIntfImgIntf::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


TIntfImgCachePtr MCQCIntfImgIntf::cptrImage() const
{
    return m_cptrImage;
}


//
//  This is a convenience for the use of the containing widget, to draw our image.
//  Since we have most of the info required, we can do it most easily. Keep imind
//  that the pre-scaled stuff has the original image info if prescaling is not
//  enabled, so we can always use those values in either case.
//
tCIDLib::TVoid
MCQCIntfImgIntf::DrawImgOn(         TGraphDrawDev&      gdevTar
                            , const tCIDLib::TBoolean   bForceAlpha
                            , const TArea&              areaDrawIn
                            , const TArea&              areaInvalid
                            , const tCIDLib::TInt4      i4HOffset
                            , const tCIDLib::TInt4      i4VOffset
                            , const tCIDLib::TBoolean   bPressed
                            , const tCIDLib::TBoolean   bNoOpacity)
{
    //
    //  If pre-scaling is enabled, then we don't need to do any scaling, so we'll
    //  force the placement to centered.
    //
    tCIDGraphDev::EPlacement eRealPlace = m_ePlacement;
    if (m_bPrescale && bHasScalingPlacement())
        eRealPlace = tCIDGraphDev::EPlacement::Center;

    // Calculate the target area
    TArea areaSrc(TPoint::pntOrigin, m_szPrescaled);
    TArea areaTarget;
    const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
    (
        areaDrawIn, areaSrc, areaTarget, m_ePlacement, bPressed
    );

    // Do any offset
    areaTarget.AdjustOrg(i4HOffset, i4VOffset);

    // If pressed adjust down/right
    if (bPressed)
        areaTarget.AdjustOrg(1, 1);

    //
    //  And draw it. The caller can ask us to igore any set constant opacity and
    //  just force it opaque.
    //
    m_bmpPreImage.AdvDrawOn
    (
        gdevTar
        , areaSrc
        , areaTarget
        , m_cptrImage->bTransparent()
        , m_cptrImage->c4TransClr()
        , m_bmpPreMask
        , eBlt
        , bNoOpacity ? 0xFF : m_c1Opacity
        , bForceAlpha
    );
}


//
//  For now, we assume this only changes at design time, so we don't have to worry
//  about pre-scaling being enabled.
//
tCIDGraphDev::EPlacement MCQCIntfImgIntf::ePlacement() const
{
    return m_ePlacement;
}

tCIDGraphDev::EPlacement
MCQCIntfImgIntf::ePlacement(const tCIDGraphDev::EPlacement eToSet)
{
    m_ePlacement = eToSet;
    return m_ePlacement;
}


//
//  Get the image into the cache if not already, and get our pointer to it. Set up
//  the pre-scaled images to just point to the original until we are asked to
//  prescale.
//
tCIDLib::TVoid
MCQCIntfImgIntf::InitImage( const   TCQCIntfView&           civOwner
                            ,       TDataSrvClient&         dsclInit
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TCQCIntfWidget&         iwdgThis)
{
    // If no image was ever picked, just do nothing
    if (!m_bUseImage || m_strImageName.bIsEmpty())
        return;

    try
    {
        m_cptrImage = facCQCIntfEng().cptrGetImage(m_strImageName, civOwner, dsclInit);
        m_szImage = m_cptrImage->szImage();

        // Start off with the pre-scaled the same as the originals
        m_bPrescale = kCIDLib::False;
        m_bmpPreImage = m_cptrImage->bmpImage();
        m_bmpPreMask = m_cptrImage->bmpMask();
        m_szPrescaled = m_szImage;
    }

    catch(const TError& errToCatch)
    {
        // If not in designer mode, log it
        if (!TFacCQCIntfEng::bDesMode())
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
                TModule::LogEventObj(errToCatch);
        }

        // Put an error in the list
        TString strErr
        (
            kIEngErrs::errcVal_ImgNotFound, facCQCIntfEng(), m_strImageName
        );
        colErrs.objPlace
        (
            iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
        );

        //
        //  Clear out any image info if we are in viewer mode, since we
        //  won't likey ever get anything now.
        //
        if (!TFacCQCIntfEng::bDesMode())
            ResetImage();
    }
}


// We format out some summary info about us
tCIDLib::TVoid
MCQCIntfImgIntf::QueryImgAttrs(         tCIDMData::TAttrList&   colAttrs
                                ,       TAttrData&              adatTmp
                                , const tCIDLib::TBoolean       bImageOpt) const
{
    colAttrs.objPlace
    (
        L"Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // If the image is optional, then add the 'use image' flag
    if (bImageOpt)
    {
        colAttrs.objPlace
        (
            L"Use Image", kCQCIntfEng::strAttr_Img_UseImage, tCIDMData::EAttrTypes::Bool
        ).SetBool(m_bUseImage);
    }


    // The rest are only editable if use image is true
    adatTmp.Set
    (
        L"Image"
        , kCQCIntfEng::strAttr_Img_Image
        , tCIDMData::EAttrTypes::String
        , m_bUseImage ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
    );
    adatTmp.SetString(m_strImageName);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_Image);
    colAttrs.objAddMove(tCIDLib::ForceMove(adatTmp));


    colAttrs.objPlace
    (
        L"Hit Transparent"
        , kCQCIntfEng::strAttr_Img_HitTransparent
        , tCIDMData::EAttrTypes::Bool
        , m_bUseImage ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
    ).SetBool(m_bHitTrans);

    colAttrs.objPlace
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_Img_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , m_bUseImage ? tCIDMData::EAttrEdTypes::InPlace : tCIDMData::EAttrEdTypes::None
    ).SetCard(tCIDLib::TCard4(m_c1Opacity));


    TString strLim;
    tCIDGraphDev::FormatEPlacement(strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
    colAttrs.objPlace
    (
        L"Placement"
        , kCQCIntfEng::strAttr_Img_Placement
        , strLim
        , tCIDMData::EAttrTypes::String
        , m_bUseImage ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
    ).SetString(tCIDGraphDev::strXlatEPlacement(m_ePlacement));
}



//
//  Returns the image path we are configured for, if it is wanted and not
//  already in the list.
//
tCIDLib::TVoid MCQCIntfImgIntf::
QueryImgPath(       tCIDLib::TStrHashSet&   colToFill
            , const tCIDLib::TBoolean       bIncludeSysImgs) const
{
    if (!m_bUseImage || m_strImageName.bIsEmpty())
        return;

    if (!bIncludeSysImgs && m_strImageName.bStartsWith(L"/System/"))
        return;

    if (colToFill.bHasElement(m_strImageName))
        return;

    colToFill.objAdd(m_strImageName);
}


tCIDLib::TVoid MCQCIntfImgIntf::ReadInImage(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ImgIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfImgIntf")
        );
    }

    strmToReadFrom  >> m_ePlacement
                    >> m_strImageName
                    >> m_bUseImage
                    >> m_c1Opacity;

    // If V2 or beyond, read in the hit trans flag, else default it
    if (c2FmtVersion < 2)
        m_bHitTrans = kCIDLib::False;
    else
        strmToReadFrom >> m_bHitTrans;

    // Convert the path to 5.x format if need
    if (c2FmtVersion < 3)
        facCQCKit().Make50Path(m_strImageName);

    // And it should all end with another frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    m_bPrescale = kCIDLib::False;
    m_bmpPreImage.Reset();
    m_bmpPreMask.Reset();

    // Fix up the path if required (slashes, double slashes, etc...)
    if (!m_strImageName.bIsEmpty())
        facCQCKit().PrepRemBrwsPath(m_strImageName);
}


// Supports the search and replace system
tCIDLib::TVoid
MCQCIntfImgIntf::ReplaceImage(  const   TString&            strSrc
                                , const TString&            strTar
                                , const tCIDLib::TBoolean   bRegEx
                                , const tCIDLib::TBoolean   bFull
                                ,       TRegEx&             regxFind)
{
    facCQCKit().bDoSearchNReplace
    (
        strSrc, strTar, m_strImageName, bRegEx, bFull, regxFind
    );

    // Fix up the path if required (slashes, double slashes, etc...)
    facCQCKit().PrepRemBrwsPath(m_strImageName);
}


// Resets the image info to defaults
tCIDLib::TVoid MCQCIntfImgIntf::ResetImage()
{
    //
    //  Note that cannot just clear this flag since some derivatives assume
    //  that it is set to whatever they set it to and don't allow it to be
    //  changed, so if we changed it here, we'd effectively permanently
    //  modify the state and it couldn't be put back right.
    //
    //  So the classes that use this mixin provide a default use state. This
    //  isn't persisted, they provide it to use when they construct use.
    //
    m_bUseImage = m_bDefaultUse;

    m_bPrescale = kCIDLib::False;
    m_bHitTrans = kCIDLib::False;
    m_c1Opacity = 0xFF;
    m_ePlacement = tCIDGraphDev::EPlacement::UpperLeft;
    m_strImageName.Clear();
    m_szImage.Zero();
    m_szPrescaled.Zero();
    m_szPreTarget.Zero();

    // Drop our local pre-scaled stuff first, which may be pointing at the original
    m_bmpPreImage.Reset();
    m_bmpPreMask.Reset();

    // And drop the reference to any cache item itself
    facCQCIntfEng().DropImgRef(m_cptrImage);
}


const TString& MCQCIntfImgIntf::strImageName() const
{
    return m_strImageName;
}

const TString& MCQCIntfImgIntf::strImageName(const TString& strToSet)
{
    m_strImageName = strToSet;
    return m_strImageName;
}


TSize MCQCIntfImgIntf::szImgDefault() const
{
    TSize szRet = m_szImage;
    szRet.TakeLarger(TSize(32, 32));
    return szRet;
}


//
//  The original and pre-scaled sizes, which may be the same if prescaling is not
//  enabled.
//
const TSize& MCQCIntfImgIntf::szImage() const
{
    return m_szImage;
}

const TSize& MCQCIntfImgIntf::szImgPrescaled() const
{
    return m_szPrescaled;
}


tCIDLib::TVoid
MCQCIntfImgIntf::SetImgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatOld
                            , const TAttrData&)
{
    if (adatOld.strId() == kCQCIntfEng::strAttr_Img_UseImage)
    {
        bUseImage(adatOld.bVal());
    }
     else if (adatOld.strId() == kCQCIntfEng::strAttr_Img_Image)
    {
        m_strImageName = adatOld.strValue();

        //
        //  If clearing the image, then we can handle that here. If setting a new
        //  image then the edit window will update us after he gets the attribute
        //  change notification.
        //
        if (m_strImageName.bIsEmpty())
            ResetImage();
    }
     else if (adatOld.strId() == kCQCIntfEng::strAttr_Img_HitTransparent)
    {
        m_bHitTrans = adatOld.bVal();
    }
     else if (adatOld.strId() == kCQCIntfEng::strAttr_Img_Opacity)
    {
        m_c1Opacity = tCIDLib::TCard1(adatOld.c4Val());
    }
     else if (adatOld.strId() == kCQCIntfEng::strAttr_Img_Placement)
    {
        m_ePlacement = tCIDGraphDev::eXlatEPlacement(adatOld.strValue());
    }
}


//
//  Enable or disable pre-scaling. If disabling, then the size is ignored. If the
//  image use flag isn't set, then we ignore this since there isn't any image to
//  display. Also, if the placement type is such that it the image would not be
//  resized, we don't do anything and just use the original.
//
tCIDLib::TVoid
MCQCIntfImgIntf::SetImgPrescale(const   TCQCIntfView&       civOwner
                                , const tCIDLib::TBoolean   bState
                                , const TSize&              szPrescale)
{
    if (m_bUseImage)
    {
        if (bState && bHasScalingPlacement())
        {
            m_szPreTarget = szPrescale;
            DoPrescale(civOwner);
        }
         else
        {
            // Go back to the original
            m_bmpPreImage = m_cptrImage->bmpImage();
            m_bmpPreMask = m_cptrImage->bmpMask();
            m_szPrescaled = m_szImage;
        }
    }
    m_bPrescale = bState;
}


// Only used during the design phase
tCIDLib::TVoid
MCQCIntfImgIntf::SetNewImage(const  TCQCIntfView&           civOwner
                            ,       TDataSrvClient&         dsclToUse
                            , const TString&                strToSet
                            , const tCIDLib::TBoolean       bForce)
{
    // If not being forced and it's the same name, then do nothing
    if (!bForce && (strToSet == m_strImageName))
        return;

    // Try to load the new image
    m_cptrImage = facCQCIntfEng().cptrGetImage
    (
        strToSet, civOwner, dsclToUse
    );
    m_szImage = m_cptrImage->szImage();

    // During design time, just always use the original
    m_bmpPreImage = m_cptrImage->bmpImage();
    m_bmpPreMask = m_cptrImage->bmpMask();
    m_szPrescaled = m_szImage;

    // It worked, so store the new name
    m_strImageName = strToSet;
}


//
//  This is called to get us to update our image. We get a new cache item, and
//  we then need to update our pre-scaled one, by either just taking the original
//  or using the prescaled size we were told to use.
//
tCIDLib::TVoid
MCQCIntfImgIntf::UpdateImage(const  TCQCIntfView&   civOwner
                            ,       TDataSrvClient& dsclToUse)
{
    try
    {
        m_cptrImage = facCQCIntfEng().cptrGetImage(m_strImageName, civOwner, dsclToUse);
        m_szImage = m_cptrImage->szImage();

        //
        //  If using the image, deal with the pre-scaling stuff if that is enabled
        //  and our placement type is one that would change the size. Else, go
        //  with the original.
        //
        if (m_bPrescale && bHasScalingPlacement())
        {
            // Call the pre-scaling method, with any previously set target size
            DoPrescale(civOwner);
        }
         else
        {
            // Update to point to the new original content
            m_bmpPreImage = m_cptrImage->bmpImage();
            m_bmpPreMask = m_cptrImage->bmpMask();
            m_szPrescaled = m_szImage;
        }
    }

    catch(const TError& errToCatch)
    {
        if (facCQCIntfEng().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Clean this one out since it's not good
        ResetImage();
    }
}


tCIDLib::TVoid
MCQCIntfImgIntf::ValidateImage(         tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclToUse
                                , const TCQCIntfWidget&         iwdgThis) const
{
    // If we have a defined image, check it
    if (m_bUseImage && !m_strImageName.bIsEmpty())
    {
        TString strErr;

        // First make sure the path is reasonable, no relative paths in this case
        if (!facCQCKit().bIsValidRemTypeRelPath(m_strImageName, strErr, kCIDLib::False))
        {
            colErrs.objAdd
            (
                TCQCIntfValErrInfo
                (
                    iwdgThis.c4UniqueId()
                    , facCQCIntfEng().strMsg
                      (
                        kIEngErrs::errcVal_BadPath, TString(L"image"), strErr
                      )
                    , kCIDLib::True
                    , iwdgThis.strWidgetId()
                )
            );
        }
         else
        {
            // Looks reasonable, see if it exists
            tCIDLib::TCard4 c4SerNum = 0;
            if (!dsclToUse.bFileExists(m_strImageName, tCQCRemBrws::EDTypes::Image))
            {
                // Put an error in the list
                strErr.LoadFromMsg
                (
                    kIEngErrs::errcVal_ImgNotFound, facCQCIntfEng(), m_strImageName
                );
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo
                    (
                        iwdgThis.c4UniqueId(), strErr, kCIDLib::True, iwdgThis.strWidgetId()
                    )
                );
            }
        }
    }
}


tCIDLib::TVoid
MCQCIntfImgIntf::WriteOutImage(TBinOutStream& strmToWriteTo) const
{
    // Frame our content, and stream out out a version format value
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_ImgIntf::c2FmtVersion
                    << m_ePlacement
                    << m_strImageName
                    << m_bUseImage
                    << m_c1Opacity

                    // V2 stuff
                    << m_bHitTrans

                    << tCIDLib::EStreamMarkers::Frame;
}



// ---------------------------------------------------------------------------
//  MCQCIntfImgIntf: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Returns true if our placement is one that would scale the image if needed
tCIDLib::TBoolean MCQCIntfImgIntf::bHasScalingPlacement() const
{
    return
    (
        (m_ePlacement == tCIDGraphDev::EPlacement::SizeToFit)
        || (m_ePlacement == tCIDGraphDev::EPlacement::SizeIfTooBig)
        || (m_ePlacement == tCIDGraphDev::EPlacement::KeepAR)
        || (m_ePlacement == tCIDGraphDev::EPlacement::FitAR)
    );
}


//
//  This is only called if the placement type would affect the size of the image.
//
tCIDLib::TVoid MCQCIntfImgIntf::DoPrescale(const TCQCIntfView& civOwner)
{
    //
    //  Let's calculate the new size, given the target pre-scale size, the orignal
    //  image size, and the placement setting.
    //
    TArea areaDraw(TPoint::pntOrigin, m_szPreTarget);
    TArea areaSrc(TPoint::pntOrigin, m_szImage);
    TArea areaTarget;
    const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
    (
        areaDraw, areaSrc, areaTarget, m_ePlacement, kCIDLib::False
    );

    // Zero the target origin for our purposes here
    areaTarget.ZeroOrg();

    // Store the new pre-scaled size
    m_szPrescaled = areaTarget.szArea();

    // OK, let's now blit the original images to our pre-scaled ones
    const TGraphDrawDev& gdevCompat = civOwner.gdevCompat();

    // Do the mask if needed
    if (m_cptrImage->bTransparent())
    {
        m_bmpPreImage = TBitmap
        (
            m_szPrescaled
            , tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );

        TGraphMemDev gdevSrc(gdevCompat, m_bmpPreMask);
        gdevSrc.DrawBitmap(m_cptrImage->bmpMask(), areaSrc, areaTarget);
    }
     else
    {
        m_bmpPreMask.Reset();
    }

    // And do the image
    {
        const TBitmap& bmpSrc = m_cptrImage->bmpImage();

        // We have to create the pre-scaled bitmap first
        m_bmpPreImage = TBitmap
        (
            m_szPrescaled
            , m_cptrImage->bHasAlpha() ? tCIDImage::EPixFmts::TrueAlpha
                                       : tCIDImage::EPixFmts::TrueClr
            , tCIDImage::EBitDepths::Eight
        );

        TGraphMemDev gdevSrc(gdevCompat, m_bmpPreImage);
        if (m_cptrImage->bHasAlpha())
            gdevSrc.AlphaBlit(bmpSrc, areaSrc, areaTarget, 0xFF, kCIDLib::True);
        else
            gdevSrc.DrawBitmap(bmpSrc, areaSrc, areaTarget);
    }

    // And copy over the user data for RIVA clients
    m_bmpPreImage.strUserData(m_cptrImage->bmpImage().strUserData());
}


