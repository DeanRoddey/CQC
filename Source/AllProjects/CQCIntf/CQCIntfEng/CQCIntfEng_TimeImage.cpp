//
// FILE NAME: CQCIntfEng_TimeImage.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/10/2014
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
//  This file implements a specialized image widget, which displays images for
//  minutes and hours of a time given to it by a couple of derived classes.
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
#include    "CQCIntfEng_TimeImage.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfTimeImgBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldTimeImg,TCQCIntfTimeImgBase)
AdvRTTIDecls(TCQCIntfLocalTimeImg,TCQCIntfTimeImgBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_TimeImage
    {
        // -----------------------------------------------------------------------
        //  Our base persistent format version
        //
        //  Version 2
        //      Convert image paths to 5.0 format.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The field based persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The local based persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2LocalFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eLocCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfTimeImgBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfTimeImgBase::~TCQCIntfTimeImgBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfTimeImgBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfTimeImgBase& iwdgOther(static_cast<const TCQCIntfTimeImgBase&>(iwdgSrc));

    // Do our bits
    return
    (
        (m_bUseHourImg    == iwdgOther.m_bUseHourImg)
        && (m_bUseMinImg  == iwdgOther.m_bUseMinImg)
        && (m_c1Opacity   == iwdgOther.m_c1Opacity)
        && (m_strNPatHour == iwdgOther.m_strNPatHour)
        && (m_strNPatMin  == iwdgOther.m_strNPatMin)
        && (m_strPathHour == iwdgOther.m_strPathHour)
        && (m_strPathMin  == iwdgOther.m_strPathMin)
    );
}


tCIDLib::TVoid
TCQCIntfTimeImgBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Init our parent. We have no init to do
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If in designer mode, set us up to display some default info. We just set
    //  a time as though a derived class had set it.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        if (m_bUseHourImg && !m_strPathHour.bIsEmpty())
            LoadImg(dsclInit, 7, kCIDLib::True);

        if (m_bUseMinImg && !m_strPathMin.bIsEmpty())
            LoadImg(dsclInit, 13, kCIDLib::False);
    }
}


tCIDLib::TVoid TCQCIntfTimeImgBase::QueryContentSize(TSize& szToFill)
{
    // Get the size of our default images we load during viewing time
    szToFill = m_cptrMinImg->szImage();
    szToFill.TakeLarger(m_cptrHourImg->szImage());

    // If we have a border, then increase by one
    if (bHasBorder())
        szToFill.Adjust(1);
}


//
//  This is a special case, We could reference any image in either of our source
//  image repo scopes. So, though it's a bit piggy, we have to query all the images
//  in those scopes and return them all.
//
tCIDLib::TVoid TCQCIntfTimeImgBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&   colScopes
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       bIncludeScope) const
{
    if (bIncludeScope)
    {
        tCIDLib::TBoolean bAdded;

        if (m_bUseHourImg)
            colScopes.objAddIfNew(m_strPathHour, bAdded);

        if (m_bUseMinImg)
            colScopes.objAddIfNew(m_strPathMin, bAdded);
    }
}


// Provide widget palette summary info
tCIDLib::TVoid
TCQCIntfTimeImgBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove any base attributes we don't honor
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    // Set any kept color names that aren't already defaulted correctly
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // And do our stuff
    colAttrs.objPlace
    (
        L"Use Hour Image"
        , kCQCIntfEng::strAttr_TmImg_UseHour
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetBool(m_bUseHourImg);

    colAttrs.objPlace
    (
        L"Use Min Image"
        , kCQCIntfEng::strAttr_TmImg_UseMin
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetBool(m_bUseMinImg);

    colAttrs.objPlace
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_TmImg_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(tCIDLib::TCard4(m_c1Opacity));

    colAttrs.objPlace
    (
        L"Hour Pattern"
        , kCQCIntfEng::strAttr_TmImg_HourPat
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetString(m_strNPatHour);

    colAttrs.objPlace
    (
        L"Hour Path"
        , kCQCIntfEng::strAttr_TmImg_HourPath
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetString(m_strPathHour);

    colAttrs.objPlace
    (
        L"Min Pattern"
        , kCQCIntfEng::strAttr_TmImg_MinPat
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetString(m_strNPatMin);

    colAttrs.objPlace
    (
        L"Minute Path"
        , kCQCIntfEng::strAttr_TmImg_MinPath
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetString(m_strPathMin);
}


tCIDLib::TVoid
TCQCIntfTimeImgBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    // Let our parent process it
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If doing images, then we apply this to our source and target paths
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strPathHour, bRegEx, bFull, regxFind
        );
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strPathMin, bRegEx, bFull, regxFind
        );
    }
}


// Return a default size for our type
TSize TCQCIntfTimeImgBase::szDefaultSize() const
{
    return TSize(128, 128);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfTimeImgBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_UseHour)
        m_bUseHourImg = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_UseMin)
        m_bUseMinImg = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_Opacity)
        m_c1Opacity = tCIDLib::TCard1(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_HourPat)
        m_strNPatHour = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_HourPath)
        m_strPathHour = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_MinPat)
        m_strNPatMin = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_TmImg_MinPath)
        m_strPathMin = adatNew.strValue();
}


tCIDLib::TVoid
TCQCIntfTimeImgBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // At least one image must be enabled
    if (!m_bUseHourImg && !m_bUseMinImg)
    {
        TString strErr(kIEngErrs::errcTImg_NotConfig, facCQCIntfEng());
        colErrs.objAdd(TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId()));
    }

    // Our paths can't be empty if configured
    if ((m_bUseHourImg && m_strPathHour.bIsEmpty())
    ||  (m_bUseMinImg && m_strPathMin.bIsEmpty()))
    {
        TString strErr(kIEngErrs::errcTImg_EmptyPath, facCQCIntfEng());
        colErrs.objAdd(TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::False, strWidgetId()));
    }

    // If configured and non-empty, the patterns must be legal
    if (m_bUseHourImg && !m_strNPatHour.bIsEmpty())
    {
        TString strTest = m_strNPatHour;
        try
        {
            if ((strTest.eReplaceToken(L"Test", L'n') != tCIDLib::EFindRes::Found)
            ||  (strTest.eReplaceToken(L"Test", L'n') != tCIDLib::EFindRes::NoMore))
            {
                TString strErr(kIEngErrs::errcTImg_BadPat, facCQCIntfEng());
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::False, strWidgetId())
                );
            }
        }

        catch(...)
        {
            TString strErr(kIEngErrs::errcTImg_BadPat, facCQCIntfEng());
            colErrs.objAdd
            (
                TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::False, strWidgetId())
            );
        }
    }

    if (m_bUseMinImg && !m_strNPatMin.bIsEmpty())
    {
        TString strTest = m_strNPatMin;
        try
        {
            if ((strTest.eReplaceToken(L"Test", L'n') != tCIDLib::EFindRes::Found)
            ||  (strTest.eReplaceToken(L"Test", L'n') != tCIDLib::EFindRes::NoMore))
            {
                TString strErr(kIEngErrs::errcTImg_BadPat, facCQCIntfEng());
                colErrs.objAdd
                (
                    TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::False, strWidgetId())
                );
            }
        }

        catch(...)
        {
            TString strErr(kIEngErrs::errcTImg_BadPat, facCQCIntfEng());
            colErrs.objAdd
            (
                TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::False, strWidgetId())
            );
        }
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the use image flags
tCIDLib::TBoolean TCQCIntfTimeImgBase::bUseHourImg() const
{
    return m_bUseHourImg;
}

tCIDLib::TBoolean TCQCIntfTimeImgBase::bUseHourImg(const tCIDLib::TBoolean bToSet)
{
    m_bUseHourImg = bToSet;
    return m_bUseHourImg;
}

tCIDLib::TBoolean TCQCIntfTimeImgBase::bUseMinImg() const
{
    return m_bUseMinImg;
}

tCIDLib::TBoolean TCQCIntfTimeImgBase::bUseMinImg(const tCIDLib::TBoolean bToSet)
{
    m_bUseMinImg = bToSet;
    return m_bUseMinImg;
}


// Get/set the opacity
tCIDLib::TCard1 TCQCIntfTimeImgBase::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfTimeImgBase::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


// Get or set the pattern used to create hour image names
const TString& TCQCIntfTimeImgBase::strNPatHour() const
{
    return m_strNPatHour;
}

const TString& TCQCIntfTimeImgBase::strNPatHour(const TString& strToSet)
{
    m_strNPatHour = strToSet;

    // If in des mode, update our sample image
    if (TFacCQCIntfEng::bDesMode())
    {
        Invalidate(areaActual());
    }
    return m_strNPatHour;
}



// Get or set the path used to load hour images
const TString& TCQCIntfTimeImgBase::strPathHour() const
{
    return m_strPathHour;
}

const TString& TCQCIntfTimeImgBase::strPathHour(const TString& strToSet)
{
    m_strPathHour = strToSet;

    // If in des mode, update our sample image
    if (TFacCQCIntfEng::bDesMode())
    {
        Invalidate(areaActual());
    }
    return m_strPathHour;
}


// Get or set the pattern used to create minute image names
const TString& TCQCIntfTimeImgBase::strNPatMin() const
{
    return m_strNPatMin;
}

const TString& TCQCIntfTimeImgBase::strNPatMin(const TString& strToSet)
{
    m_strNPatMin = strToSet;

    // If in des mode, update our sample image
    if (TFacCQCIntfEng::bDesMode())
    {
        Invalidate(areaActual());
    }
    return m_strNPatMin;
}


// Get or set the path used to load minute images
const TString& TCQCIntfTimeImgBase::strPathMin() const
{
    return m_strPathMin;
}

const TString& TCQCIntfTimeImgBase::strPathMin(const TString& strToSet)
{
    m_strPathMin = strToSet;

    // If in des mode, update our sample image
    if (TFacCQCIntfEng::bDesMode())
    {
        Invalidate(areaActual());
    }
    return m_strPathMin;
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Hidden constructors operators
// ---------------------------------------------------------------------------

TCQCIntfTimeImgBase::TCQCIntfTimeImgBase(const  tCQCIntfEng::ECapFlags  eCaps
                                        , const TString&                strTypeStr) :

    TCQCIntfWidget(eCaps, strTypeStr)
    , m_bUseHourImg(kCIDLib::True)
    , m_bUseMinImg(kCIDLib::True)
    , m_c1Opacity(0xFF)
    , m_c4LastHour(99)
    , m_c4LastMin(99)
    , m_strPathHour(L"/User/")
    , m_strPathMin(L"/User/")
    , m_strNPatHour(L"%(n)")
    , m_strNPatMin(L"%(n)")
{
    // Set out default appearance
    bIsTransparent(kCIDLib::True);

    // Set the action command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/TimeImage");
}

TCQCIntfTimeImgBase::TCQCIntfTimeImgBase(const TCQCIntfTimeImgBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bUseHourImg(iwdgSrc.m_bUseHourImg)
    , m_bUseMinImg(iwdgSrc.m_bUseMinImg)
    , m_c1Opacity(iwdgSrc.m_c1Opacity)
    , m_c4LastHour(iwdgSrc.m_c4LastHour)
    , m_c4LastMin(iwdgSrc.m_c4LastMin)
    , m_cptrHourImg(iwdgSrc.m_cptrHourImg)
    , m_cptrMinImg(iwdgSrc.m_cptrMinImg)
    , m_strNPatHour(iwdgSrc.m_strNPatHour)
    , m_strNPatMin(iwdgSrc.m_strNPatMin)
    , m_strPathHour(iwdgSrc.m_strPathHour)
    , m_strPathMin(iwdgSrc.m_strPathMin)
{
}

TCQCIntfTimeImgBase&
TCQCIntfTimeImgBase::operator=(const TCQCIntfTimeImgBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_bUseHourImg   = iwdgSrc.m_bUseHourImg;
        m_bUseMinImg    = iwdgSrc.m_bUseMinImg;
        m_c1Opacity     = iwdgSrc.m_c1Opacity;
        m_c4LastHour    = iwdgSrc.m_c4LastHour;
        m_c4LastMin     = iwdgSrc.m_c4LastMin;
        m_cptrHourImg   = iwdgSrc.m_cptrHourImg;
        m_cptrMinImg    = iwdgSrc.m_cptrMinImg;
        m_strNPatHour   = iwdgSrc.m_strNPatHour;
        m_strNPatMin    = iwdgSrc.m_strNPatMin;
        m_strPathHour   = iwdgSrc.m_strPathHour;
        m_strPathMin    = iwdgSrc.m_strPathMin;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfTimeImgBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeImage::c2FmtVersion))
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

    // Read in our stuff
    strmToReadFrom  >> m_bUseHourImg
                    >> m_strNPatHour
                    >> m_bUseMinImg
                    >> m_strNPatMin
                    >> m_c1Opacity
                    >> m_strPathHour
                    >> m_strPathMin;

    // Convert paths if needed
    if (c2FmtVersion < 2)
    {
        facCQCKit().Make50Path(m_strPathHour);
        facCQCKit().Make50Path(m_strPathMin);
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfTimeImgBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_TimeImage::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do our stuff and the end marker
    strmToWriteTo   << m_bUseHourImg
                    << m_strNPatHour
                    << m_bUseMinImg
                    << m_strNPatMin
                    << m_c1Opacity
                    << m_strPathHour
                    << m_strPathMin
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfTimeImgBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip and draw into this remaining area
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    TArea areaDraw = grgnClip.areaBounds();

    // If we are set up for hour images and it's loaded yet, display that
    if (m_bUseHourImg && m_cptrHourImg.pobjData())
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , m_cptrHourImg
            , areaDraw
            , areaInvalid
            , tCIDGraphDev::EPlacement::Center
            , m_c1Opacity
        );
    }

    // And do minutes if set up for that
    if (m_bUseMinImg  && m_cptrMinImg.pobjData())
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , m_cptrMinImg
            , areaDraw
            , areaInvalid
            , tCIDGraphDev::EPlacement::Center
            , m_c1Opacity
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  For the derived class to update us with a new hour and minute. If either has
//  changed, and either is different from what we are displaying (and we are
//  configured to use that image), then we update and redraw.
//
tCIDLib::TVoid
TCQCIntfTimeImgBase::SetTime(const  tCIDLib::TEncodedTime   enctNew
                            , const tCIDLib::TBoolean       bNoRedraw)
{
    //
    //  Get out the new hour and minute, or just set them to 99 if the passed
    //  time stamp is zero.
    //
    tCIDLib::TCard4 c4Hour, c4Min;
    if (!enctNew)
    {
        c4Hour = 99;
        c4Min = 99;
    }
     else
    {
        tCIDLib::TCard4 c4Sec;
        TTime tmNew(enctNew);
        tmNew.c4AsTimeInfo(c4Hour, c4Min, c4Sec);

        // Make sure the hour is in 12 hour format
        if (c4Hour >= 12)
            c4Hour -= 12;
    }

    //
    //  For either one, if we ar eusing it and the value has changed, then we either
    //  try to load a new image or reset the image if the passed stamp is zero.
    //
    tCIDLib::TBoolean bRedraw = kCIDLib::False;
    if (m_bUseHourImg && (c4Hour != m_c4LastHour))
    {
        m_c4LastHour = c4Hour;

        if (enctNew)
            LoadImg(m_c4LastHour, kCIDLib::True);
        else
            m_cptrHourImg->DropImage();

        bRedraw = kCIDLib::True;
    }

    if (m_bUseMinImg && (c4Min != m_c4LastMin))
    {
        m_c4LastMin = c4Min;

        if (enctNew)
            LoadImg(m_c4LastMin, kCIDLib::False);
        else
            m_cptrMinImg->DropImage();

        bRedraw = kCIDLib::True;
    }

    // If something changed and we are allowed to redraw now, do so
    if (bRedraw && !bNoRedraw)
        Redraw();
}



// ---------------------------------------------------------------------------
//  TCQCIntfTimeImgBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Load up an hour or minute image by its number. We have one for when the caller
//  already has an interface server client proxy, and another convenience one that
//  creates the proxy for the caller and then calls the other version.
//
tCIDLib::TVoid
TCQCIntfTimeImgBase::LoadImg(const  tCIDLib::TCard4     c4Number
                            , const tCIDLib::TBoolean   bHour)
{
    try
    {
        TDataSrvClient dsclLoad;
        LoadImg(dsclLoad, c4Number, bHour);
    }

    catch(...)
    {
        // We don't log this, we just clear out the image
        if (bHour)
            facCQCIntfEng().DropImgRef(m_cptrHourImg);
        else
            facCQCIntfEng().DropImgRef(m_cptrMinImg);
    }
}

tCIDLib::TVoid
TCQCIntfTimeImgBase::LoadImg(       TDataSrvClient&         dsclLoad
                            ,       tCIDLib::TCard4         c4Number
                            , const tCIDLib::TBoolean       bHour)
{
    try
    {
        // Build up the image path, starting with the path for the image type
        TString strPath(bHour ? m_strPathHour : m_strPathMin);

        // Make sure it ends with a separator
        if (strPath.chLast() != kCIDLib::chForwardSlash)
            strPath.Append(kCIDLib::chForwardSlash);

        // Format out the image file name
        TString strName(bHour ? m_strNPatHour : m_strNPatMin);
        strName.eReplaceToken(c4Number, L'n');

        // And add the name to the path
        strPath.Append(strName);

        if (bHour)
            m_cptrHourImg = facCQCIntfEng().cptrGetImage(strPath, civOwner(), dsclLoad);
        else
            m_cptrMinImg = facCQCIntfEng().cptrGetImage(strPath, civOwner(), dsclLoad);
    }

    catch(...)
    {
        // We don't log this, we just clear out the image
        if (bHour)
            facCQCIntfEng().DropImgRef(m_cptrHourImg);
        else
            facCQCIntfEng().DropImgRef(m_cptrMinImg);
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldTimeImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldTimeImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldTimeImg::TCQCIntfFldTimeImg() :

    TCQCIntfTimeImgBase
    (
        CQCIntfEng_TimeImage::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_TimeImg)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltTypeName
          (
              tCQCKit::EReqAccess::MReadCWrite, tCQCKit::EFldTypes::Time
          )
      )
{
}

TCQCIntfFldTimeImg::TCQCIntfFldTimeImg(const TCQCIntfFldTimeImg& iwdgToCopy) :

    TCQCIntfTimeImgBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldTimeImg::~TCQCIntfFldTimeImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldTimeImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldTimeImg&
TCQCIntfFldTimeImg::operator=(const TCQCIntfFldTimeImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldTimeImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to let us verify that we can be associated with the passed
//  field. It can optionally tell us to set ourselves up for this new field
//  if it is acceptable, in our case storing the range info on our parent.
//
tCIDLib::TBoolean
TCQCIntfFldTimeImg::bCanAcceptField(const   TCQCFldDef&         flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean   )
{
    return (flddTest.eType() == tCQCKit::EFldTypes::Time);
}


tCIDLib::TBoolean TCQCIntfFldTimeImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldTimeImg& iwdgOther(static_cast<const TCQCIntfFldTimeImg&>(iwdgSrc));

    // We just add the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfFldTimeImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldTimeImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldTimeImg::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfFldTimeImg::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldTimeImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldTimeImg::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Time Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldTimeImg::RegisterFields(         TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldTimeImg::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldTimeImg::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfFldTimeImg::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variables mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldTimeImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFldTimeImg::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        SetTime
        (
            static_cast<const TCQCTimeFldValue&>(fvCurrent()).c8Value()
            , bNoRedraw
        );
    }
     else
    {
        SetTime(0, bNoRedraw);
    }
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfFldTimeImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeImage::c2FldFmtVersion))
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

    // We currently don't have any

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFldTimeImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_TimeImage::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfFldTimeImg::ValueUpdate(        TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLocalTimeImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfLocalTimeImg::TCQCIntfLocalTimeImg() :

    TCQCIntfTimeImgBase
    (
        CQCIntfEng_TimeImage::eLocCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_TimeImg)
    )
    , m_enctNextUpdate(0)
{
}

TCQCIntfLocalTimeImg::TCQCIntfLocalTimeImg(const TCQCIntfLocalTimeImg& iwdgToCopy) :

    TCQCIntfTimeImgBase(iwdgToCopy)
{
}

TCQCIntfLocalTimeImg::~TCQCIntfLocalTimeImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfLocalTimeImg&
TCQCIntfLocalTimeImg::operator=(const TCQCIntfLocalTimeImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);

        // Nothing of our own at this point
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfLocalTimeImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfLocalTimeImg& iwdgOther(static_cast<const TCQCIntfLocalTimeImg&>(iwdgSrc));

    // Nothing to add for the moment
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfLocalTimeImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfLocalTimeImg&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfLocalTimeImg::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Make sure the next update value is reset
    m_enctNextUpdate = 0;
}


tCIDLib::TVoid
TCQCIntfLocalTimeImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // For now, nothing of our own
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfLocalTimeImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // For now, nothing of our own
}


// ---------------------------------------------------------------------------
//  TCQCIntfLocalTimeImg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just get the local time. If it's past the m_enctNextUpdate value, we will
//  pass it to our parent class and set a new next update. This way we don't beat
//  it to death when it really only ever has to update once a minute.
//
tCIDLib::TVoid
TCQCIntfLocalTimeImg::ActiveUpdate( const   tCIDLib::TBoolean   bNoRedraw
                                    , const TGUIRegion&         grgnClip
                                    , const tCIDLib::TBoolean   bInTopLayer)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur >= m_enctNextUpdate)
    {
        // Set the new update time to the next minute boundary
        m_enctNextUpdate = (enctCur % kCIDLib::enctOneSecond) + 1;
        m_enctNextUpdate *= kCIDLib::enctOneSecond;

        // And update our parent class
        SetTime(enctCur, bNoRedraw);
    }
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfLocalTimeImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_TimeImage::c2LocalFmtVersion))
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

    // We currently don't have any stuff of our own

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfLocalTimeImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_TimeImage::c2LocalFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}

