//
// FILE NAME: CQCIntfEng_VUMeter.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2002
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
//  This file implements a derivative of the base widget class. This one will
//  display a numeric value in a classic digital VU meter format.
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
#include    "CQCIntfEng_VUMeter.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfVUMeterBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldVUMeter,TCQCIntfVUMeterBase)
AdvRTTIDecls(TCQCIntfVarVUMeter,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_VUMeter
    {
        // -----------------------------------------------------------------------
        //  The format version for the base class
        //
        //  Version 2 -
        //      We moved to using the image list mixin instead of having a fixed set
        //      of images. So we have to now get the mixin initialized with the
        //      original images upon being streamed in for the first time after the
        //      upgrade. And we added various other options at the same time.
        //
        //  Version 3 -
        //      Moved from support just left-right and bottom-up, to supporting all
        //      four directions. So we replaced a boolean with an EDir enum.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2BaseFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  The format version for the field derivative
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FldFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The format version for the variable derivative
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2VarFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The min and max field range that we will accept, else we reject the
        //  field.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TFloat8      f8MinRange = 4.0;
        constexpr tCIDLib::TFloat8      f8MaxRange = 4096.0;


        // -----------------------------------------------------------------------
        //  The caps flags that we use for field and variable based meters
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  The names of the images we support
        // -----------------------------------------------------------------------
        const TString   strImg_DarkLow("DarkLow");
        const TString   strImg_LightLow("LightLow");
        const TString   strImg_DarkMid("DarkMid");
        const TString   strImg_LightMid("LightMid");
        const TString   strImg_DarkHigh("DarkHigh");
        const TString   strImg_LightHigh("LightHigh");
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVUMeterBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVUMeterBase::~TCQCIntfVUMeterBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfVUMeterBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVUMeterBase& iwdgOther(static_cast<const TCQCIntfVUMeterBase&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther))
        return kCIDLib::False;

    // Do our own stuff
    return
    (
        (m_bLightOnly == iwdgOther.m_bLightOnly)
        && (m_c4HighPer == iwdgOther.m_c4HighPer)
        && (m_c4MidPer == iwdgOther.m_c4MidPer)
        && (m_c4Spacing == iwdgOther.m_c4Spacing)
        && (m_eDir == iwdgOther.m_eDir)
    );
}


//
//  We initialize ourself. If in desiger mode, push some bogus data into our
//  min/max/value fields so that we'll show both lit and unlit LEDs.
//
tCIDLib::TVoid
TCQCIntfVUMeterBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Tell the image list mixin to load up its images
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    //
    //  If in designer mode, set some useful values on us, so that they
    //  can see most of the LEDs light up. And force off the value error
    //  state which normally is on by default until we get a good value
    //  from the derived class.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        m_f8Min = 0;
        m_f8Max = 100;
        m_f8Value = 85;
        m_bValError = kCIDLib::False;
    }

    //
    //  Do an initial area calc to get some numbers set up. We won't get an initial
    //  area/size change to cause this.
    //
    CalcAreas(areaActual());
}


tCIDLib::TVoid TCQCIntfVUMeterBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfVUMeterBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
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
        L"Mid LED Range"
        , kCQCIntfEng::strAttr_VUMeter_MidPer
        , L"Range: 0, 100"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4MidPer);

    colAttrs.objPlace
    (
        L"High LED Range"
        , kCQCIntfEng::strAttr_VUMeter_HighPer
        , L"Range: 0, 100"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4HighPer);

    colAttrs.objPlace
    (
        L"Light Only"
        , kCQCIntfEng::strAttr_VUMeter_LOnly
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetBool(m_bLightOnly);

    colAttrs.objPlace
    (
        L"Spacing"
        , kCQCIntfEng::strAttr_VUMeter_Spacing
        , L"Range: 0, 24"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4Spacing);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfVUMeterBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


// Handle search and replace requeests
tCIDLib::TVoid
TCQCIntfVUMeterBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If doing images, teh pass on to our image list mixn
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Return a default initial size
TSize TCQCIntfVUMeterBase::szDefaultSize() const
{
    if (bHorizontal())
        return TSize(128, 64);

    return TSize(64, 128);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVUMeterBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_VUMeter_MidPer)
        m_c4MidPer = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_VUMeter_HighPer)
        m_c4HighPer = adatNew.c4Val();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_VUMeter_LOnly)
        m_bLightOnly = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_VUMeter_Spacing)
        m_c4Spacing= adatNew.c4Val();
}

tCIDLib::TVoid
TCQCIntfVUMeterBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return true if one of the horizontal directions
tCIDLib::TBoolean TCQCIntfVUMeterBase::bHorizontal() const
{
    return ((m_eDir == tCIDLib::EDirs::Left) || (m_eDir == tCIDLib::EDirs::Right));
}


// Get/set the 'light only' flag
tCIDLib::TBoolean TCQCIntfVUMeterBase::bLightOnly() const
{
    return m_bLightOnly;
}

tCIDLib::TBoolean TCQCIntfVUMeterBase::bLightOnly(const tCIDLib::TBoolean bToSet)
{
    m_bLightOnly = bToSet;
    return m_bLightOnly;
}



// Get/set our high and mid percentages
tCIDLib::TCard4 TCQCIntfVUMeterBase::c4HighPer() const
{
    return m_c4HighPer;
}

tCIDLib::TCard4
TCQCIntfVUMeterBase::c4HighPer(const tCIDLib::TCard4 c4ToSet)
{
    m_c4HighPer = c4ToSet;
    return m_c4HighPer;
}


tCIDLib::TCard4 TCQCIntfVUMeterBase::c4MidPer() const
{
    return m_c4MidPer;
}

tCIDLib::TCard4
TCQCIntfVUMeterBase::c4MidPer(const tCIDLib::TCard4 c4ToSet)
{
    m_c4MidPer = c4ToSet;
    return m_c4MidPer;
}


// Get/set the spacing value
tCIDLib::TCard4 TCQCIntfVUMeterBase::c4Spacing() const
{
    return m_c4Spacing;
}

tCIDLib::TCard4
TCQCIntfVUMeterBase::c4Spacing(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Spacing = c4ToSet;

    // This will affect the LED count, so we have to re-calc sizes
    CalcAreas(areaActual());

    return m_c4Spacing;
}


// Get/set the direction value
tCIDLib::EDirs TCQCIntfVUMeterBase::eDir() const
{
    return m_eDir;
}

tCIDLib::EDirs TCQCIntfVUMeterBase::eDir(const tCIDLib::EDirs eToSet)
{
    if (eToSet != m_eDir)
    {
        m_eDir = eToSet;

        // Recalculate to get the new led count and what setup
        CalcAreas(areaActual());
    }
    return m_eDir;
}




// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfVUMeterBase::
TCQCIntfVUMeterBase(const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName)
    , m_bLightOnly(kCIDLib::False)
    , m_bValError(kCIDLib::True)
    , m_c4LEDCount(1)
    , m_c4Spacing(4)
    , m_c4HighPer(90)
    , m_c4MidPer(75)
    , m_eDir(tCIDLib::EDirs::Up)
    , m_f8Max(1)
    , m_f8Min(0)
    , m_f8Value(0)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    //
    //  Since this is a new object, set up the image list mixin for the images
    //  we support. We have a helper to do that since we also need to do it if
    //  upgrading from a version 1 object. It will initially set up the default
    //  set of LEDs.
    //
    AddImageKeys();

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/DigitalVU");
}

TCQCIntfVUMeterBase::TCQCIntfVUMeterBase(const TCQCIntfVUMeterBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_bLightOnly(iwdgSrc.m_bLightOnly)
    , m_bValError(iwdgSrc.m_bValError)
    , m_c4LEDCount(iwdgSrc.m_c4LEDCount)
    , m_c4Spacing(iwdgSrc.m_c4Spacing)
    , m_c4HighPer(iwdgSrc.m_c4HighPer)
    , m_c4MidPer(iwdgSrc.m_c4MidPer)
    , m_eDir(iwdgSrc.m_eDir)
    , m_f8Max(iwdgSrc.m_f8Max)
    , m_f8Min(iwdgSrc.m_f8Min)
    , m_f8Value(iwdgSrc.m_f8Value)
{
}

TCQCIntfVUMeterBase&
TCQCIntfVUMeterBase::operator=(const TCQCIntfVUMeterBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);

        m_bLightOnly    = iwdgSrc.m_bLightOnly;
        m_bValError     = iwdgSrc.m_bValError;
        m_c4LEDCount    = iwdgSrc.m_c4LEDCount;
        m_c4Spacing     = iwdgSrc.m_c4Spacing;
        m_c4HighPer     = iwdgSrc.m_c4HighPer;
        m_c4MidPer      = iwdgSrc.m_c4MidPer;
        m_eDir          = iwdgSrc.m_eDir;
        m_f8Max         = iwdgSrc.m_f8Max;
        m_f8Min         = iwdgSrc.m_f8Min;
        m_f8Value       = iwdgSrc.m_f8Value;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfVUMeterBase::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);
    CalcAreas(areaNew);
}


// We want to recalc some stuff on size change
tCIDLib::TVoid
TCQCIntfVUMeterBase::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);
    CalcAreas(areaActual());
}


//
//  Stream ourself in and out
//
tCIDLib::TVoid TCQCIntfVUMeterBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VUMeter::c2BaseFmtVersion))
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
    //  If V1, we have to initialize the field list mixin to refer to the original
    //  set of images we used to be hard coded for. Else, stream in the mixin.
    //
    if (c2FmtVersion == 1)
    {
        // Default some other stuff
        m_eDir = tCIDLib::EDirs::Up;
        m_bLightOnly = kCIDLib::False;
        m_c4Spacing = 4;
    }
     else
    {
        // Do any mixins
        MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

        // If V2, read in the boolean direction and convert, else the enum
        if (c2FmtVersion == 2)
        {
            tCIDLib::TBoolean bHorz;
            strmToReadFrom  >>  bHorz;
            if (bHorz)
                m_eDir = tCIDLib::EDirs::Right;
            else
                m_eDir = tCIDLib::EDirs::Up;
        }
         else
        {
            strmToReadFrom >> m_eDir;
        }

        strmToReadFrom  >>  m_bLightOnly
                        >>  m_c4Spacing;
    }

    // And pull in the rest of the values
    strmToReadFrom  >> m_c4HighPer
                    >> m_c4MidPer;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfVUMeterBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Write out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VUMeter::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // And stream our stuff and our end marker
    strmToWriteTo   << m_eDir
                    << m_bLightOnly
                    << m_c4Spacing
                    << m_c4HighPer
                    << m_c4MidPer
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfVUMeterBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    //
    //  Draw the VU meter contents if we have a good value. If in designer
    //  mode, we set the value error off so we always seem to have a good
    //  value in designer mode.
    //
    if (!m_bValError)
    {
        TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
        DrawLEDs(gdevTarget, areaInvalid);
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------


//
//  Called by derived classes to tell us what value to display. The return is True
//  if we have a good value and the new value is different.
//
tCIDLib::TBoolean
TCQCIntfVUMeterBase::bSetValue( const   tCIDLib::TFloat8    f8ToSet
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bValError)
{
    tCIDLib::TBoolean   bOldErr = m_bValError;
    tCIDLib::TFloat8    f8OldVal = m_f8Value;

    m_bValError = bValError;
    if (bValError)
    {
        // Just put the value on the minimum
        m_f8Value = m_f8Min;
    }
     else
    {
        if ((f8ToSet < m_f8Min) || (f8ToSet > m_f8Max))
        {
            m_bValError = kCIDLib::True;
            m_f8Value = m_f8Min;
        }
         else
        {
            m_bValError = kCIDLib::False;
            m_f8Value = f8ToSet;
        }
    }

    //
    //  Now, if its changed from the current value and not in error, redraw
    //  unless asked not to.
    //
    const tCIDLib::TBoolean bNewError(m_bValError != bOldErr);
    const tCIDLib::TBoolean bNewVal(m_f8Value != f8OldVal);
    if (bNewVal || bNewError)
    {
        // If not hidden or supressed, then update with our new value
        if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
            Redraw();
    }

    // If no error and a new value, then return true
    return !bValError && bNewVal;
}


//
//  This method is called by derivatives to tell us what our range is and
//  whether we should set the current value to the minimum value or just
//  force it within the range at either end if not already.
//
tCIDLib::TVoid
TCQCIntfVUMeterBase::SetValueAttrs( const   tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCIDLib::TBoolean   bInitToMin)
{
    // Update our current value so that it's within the new min/max
    if (m_f8Value > f8Max)
        m_f8Value = f8Max;
    else if (m_f8Value < f8Min)
        m_f8Value = f8Min;

    // Store our min/max and calc the new per-unit degrees
    m_f8Max = f8Max;
    m_f8Min = f8Min;

    //
    //  If asked, set the value to the min value. If in designer mode, we always
    //  set it to an 85% value, so that we always display something useful during
    //  design.
    //
    if (TFacCQCIntfEng::bDesMode())
        m_f8Value = m_f8Min + ((m_f8Max - m_f8Min) * 0.85);
    else if (bInitToMin)
        m_f8Value = m_f8Min;
}



// ---------------------------------------------------------------------------
//  TCQCIntfVUMeterBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to set up the image list mixin wiht the keys we support, to avoid
//  doing it redundantly from multiple places. It will also set them up with the
//  default initial images. These are the ones that would have been used before
//  we change it to allow arbitrary images.
//
tCIDLib::TVoid TCQCIntfVUMeterBase::AddImageKeys()
{
    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_DarkLow
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Green Dark"
    );
    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_LightLow
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Green"
    );

    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_DarkMid
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Yellow Dark"
    );
    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_LightMid
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Yellow"
    );

    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_DarkHigh
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Red Dark"
    );
    AddNewImgKey
    (
        CQCIntfEng_VUMeter::strImg_LightHigh
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
        , L"/System/LEDs/Plastic/VUMeter/Set 1/Red"
    );
}


tCIDLib::TVoid TCQCIntfVUMeterBase::CalcAreas(const TArea& areaNew)
{
    // Move inwards if we have a border
    TArea areaContent(areaNew);
    if (bHasBorder())
        areaContent.Deflate(1);

    //
    //  Get the size of one of the LED images. They have to be the same size
    //  in the travel direction, so doesn't matter which.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiLED
    (
        pimgiForKey(CQCIntfEng_VUMeter::strImg_DarkHigh)
    );

    TSize szImg = pimgiLED->m_cptrImg->szImage();

    // Get a local horz/vert flag for use below
    const tCIDLib::TBoolean bHorz = bHorizontal();

    // Update our calculated number of LEDs we can display
    const tCIDLib::TCard4 c4LEDSz((bHorz ? szImg.c4Width() : szImg.c4Height()));
    const tCIDLib::TCard4 c4Travel
    (
        bHorz ? areaContent.c4Width() : areaContent.c4Height()
    );

    if (facCQCIntfEng().bLogInfo())
    {
        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Calculating size/pos info for new area %(1)"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , areaNew
        );
    }

    //
    //  If the LED image isn't set, then we just go with one LED. Else do the
    //  calculation.
    //
    tCIDLib::TCard4 c4Accum = c4LEDSz;
    m_c4LEDCount = 1;
    if (c4LEDSz)
    {
        //
        //  We have a special case in that we have to include the spacing between
        //  the LEDs, but we can't just add it to the LED size and divide, because
        //  the spacing after the last one shouldn't count.
        //
        //  So we just do it the old fashioned way and loop until we go over the
        //  max available size. We start with one, and the accum/count were set
        //  above to reflect one LED without spacing.
        //
        //  While there is room for one more plus a space, add it
        //
        const tCIDLib::TCard4 c4OneStep = c4LEDSz + m_c4Spacing;
        while ((c4Accum + c4OneStep) < c4Travel)
        {
            m_c4LEDCount++;
            c4Accum += c4OneStep;
        }
    }

    if (facCQCIntfEng().bLogInfo())
    {
        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Calculated %(1) LEDs maximum"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(m_c4LEDCount)
        );
    }

    //
    //  We are the content area size in the transverse direction. In the direction
    //  of travel, we use the accumulated size from above where we figured out how
    //  many LEDs.
    //
    if (bHorz)
    {
        m_areaLEDs.c4Height(areaContent.c4Height());
        m_areaLEDs.c4Width(c4Accum);
    }
     else
    {
        m_areaLEDs.c4Width(areaContent.c4Width());
        m_areaLEDs.c4Height(c4Accum);
    }

    // And center this within the full new area
    m_areaLEDs.CenterIn(areaNew);
}


tCIDLib::TVoid
TCQCIntfVUMeterBase::CalcIndices(const  tCIDLib::TFloat8    f8Val
                                ,       tCIDLib::TCard4&    c4HighInd
                                ,       tCIDLib::TCard4&    c4MidInd
                                ,       tCIDLib::TCard4&    c4TopInd)
{
    const tCIDLib::TFloat8 f8Range(m_f8Max - m_f8Min);

    // Calculate the percent for the top LED
    const tCIDLib::TFloat8 f8TopPer = (f8Val - m_f8Min) / f8Range;

    //
    //  And now using those percents and the stored ones, calc the
    //  indices.
    //
    c4HighInd = tCIDLib::TCard4
    (
        m_c4LEDCount * (tCIDLib::TFloat8(m_c4HighPer) / 100)
    );
    c4TopInd = tCIDLib::TCard4(m_c4LEDCount * f8TopPer);
    c4MidInd = tCIDLib::TCard4
    (
        m_c4LEDCount * (tCIDLib::TFloat8(m_c4MidPer) / 100)
    );

    if (facCQCIntfEng().bLogInfo())
    {
        facCQCIntfEng().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"New val is %(1). Mid/High/Top=%(2)/%(3)/%(4)"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , TFloat(f8Val)
            , TCardinal(c4MidInd)
            , TCardinal(c4HighInd)
            , TCardinal(c4HighInd)
        );
    }
}


tCIDLib::TVoid
TCQCIntfVUMeterBase::DrawLEDs(TGraphDrawDev& gdevTarget, const TArea& areaInvalid)
{
    //
    //  Get the low images. We'll use that to create the area that we'll use to
    //  draw into and move forward each time.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiDark
    (
        pimgiForKey(CQCIntfEng_VUMeter::strImg_DarkLow)
    );
    MCQCIntfImgListIntf::TImgInfo* pimgiLight
    (
        pimgiForKey(CQCIntfEng_VUMeter::strImg_LightLow)
    );

    TArea areaLED(TPoint::pntOrigin, pimgiLight->m_cptrImg->szImage());
    areaLED.ZeroOrg();

    // Keep a copy of the LED area zero origined, for drawing purposes
    const TArea areaImgSrc(areaLED);

    // Get a local horz/vert flag
    const tCIDLib::TBoolean bHorz = bHorizontal();

    // Start it off at the correct place
    switch(m_eDir)
    {
        case tCIDLib::EDirs::Down :
            areaLED.TopJustifyIn(m_areaLEDs, kCIDLib::True);
            break;

        case tCIDLib::EDirs::Left :
            areaLED.RightJustifyIn(m_areaLEDs, kCIDLib::True);
            break;

        case tCIDLib::EDirs::Right :
            areaLED.LeftJustifyIn(m_areaLEDs, kCIDLib::True);
            break;

        case tCIDLib::EDirs::Up :
            areaLED.BottomJustifyIn(m_areaLEDs, kCIDLib::True);
            break;

        default :
            CIDAssert2(L"Unknown VU direction")
            break;
    };

    // Based on the number of LEDs, decide where the mid and high will start
    tCIDLib::TCard4 c4HighInd;
    tCIDLib::TCard4 c4MidInd;
    tCIDLib::TCard4 c4TopLED;
    CalcIndices(m_f8Value, c4HighInd, c4MidInd, c4TopLED);

    //
    //  And loop through, drawing any that intersect the invalid area.
    //
    tCIDLib::TCard4 c4Level = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < m_c4LEDCount; c4Index++)
    {
        //
        //  If in light only mode, we can berak out when we have gone past the
        //  top-most lit up LED.
        //
        if (m_bLightOnly && (c4Index >= c4TopLED))
            break;

        // Decide which set of images we need to draw
        tCIDLib::TCard4 c4NewLevel = 0;
        if (c4Index >= c4HighInd)
            c4NewLevel = 2;
        else if (c4Index >= c4MidInd)
            c4NewLevel = 1;

        // If the set level value has changed, set up new image pointers
        if (c4NewLevel != c4Level)
        {
            if (c4NewLevel == 1)
            {
                pimgiDark = pimgiForKey(CQCIntfEng_VUMeter::strImg_DarkMid);
                pimgiLight = pimgiForKey(CQCIntfEng_VUMeter::strImg_LightMid);
            }
             else if (c4NewLevel == 2)
            {
                pimgiDark = pimgiForKey(CQCIntfEng_VUMeter::strImg_DarkHigh);
                pimgiLight = pimgiForKey(CQCIntfEng_VUMeter::strImg_LightHigh);
            }
        }

        // If this area intersects the invalid area, draw it
        if (areaLED.bIntersects(areaInvalid))
        {
            MCQCIntfImgListIntf::TImgInfo* pimgiDraw;
            if (c4Index >= c4TopLED)
                pimgiDraw = pimgiDark;
            else
                pimgiDraw = pimgiLight;

            pimgiDraw->m_cptrImg->bmpImage().AdvDrawOn
            (
                gdevTarget
                , areaImgSrc
                , areaLED
                , pimgiDraw->m_cptrImg->bTransparent()
                , pimgiDraw->m_cptrImg->c4TransClr()
                , pimgiDraw->m_cptrImg->bmpMask()
                , tCIDGraphDev::EBltTypes::None
                , pimgiDraw->m_c1Opacity
            );
        }

        // Move by the LED size plus spacing
        switch(m_eDir)
        {
            case tCIDLib::EDirs::Down :
                areaLED.AdjustOrg(0, tCIDLib::TInt4(areaLED.c4Height() + m_c4Spacing));
                break;

            case tCIDLib::EDirs::Left :
                areaLED.AdjustOrg(-tCIDLib::TInt4(areaLED.c4Width() + m_c4Spacing), 0);
                break;

            case tCIDLib::EDirs::Right :
                areaLED.AdjustOrg(tCIDLib::TInt4(areaLED.c4Width() + m_c4Spacing), 0);
                break;

            case tCIDLib::EDirs::Up :
                areaLED.AdjustOrg(0, -tCIDLib::TInt4(areaLED.c4Height() + m_c4Spacing));
                break;

            default :
                CIDAssert2(L"Unknown VU direction")
                break;
        };
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldVUMeter
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldVUMeter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldVUMeter::TCQCIntfFldVUMeter() :

    TCQCIntfVUMeterBase
    (
        CQCIntfEng_VUMeter::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_DigitalVUMeter)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltNumRange
          (
              tCQCKit::EReqAccess::MReadCWrite
              , CQCIntfEng_VUMeter::f8MinRange
              , CQCIntfEng_VUMeter::f8MaxRange
              , kCIDLib::True
          )
      )
{
}

TCQCIntfFldVUMeter::TCQCIntfFldVUMeter(const TCQCIntfFldVUMeter& iwdgToCopy) :

    TCQCIntfVUMeterBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldVUMeter::~TCQCIntfFldVUMeter()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVUMeter: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldVUMeter&
TCQCIntfFldVUMeter::operator=(const TCQCIntfFldVUMeter& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVUMeter: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept, and optionally update ourselves
//  to conform to, a new field. In our case we check that it's a numeric
//  field and if so (and we are storing) we store the range info on our parent
//  class.
//
tCIDLib::TBoolean
TCQCIntfFldVUMeter::bCanAcceptField(const   TCQCFldDef&         flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean   bStore)
{
    tCIDLib::TFloat8 f8Min, f8Max;
    const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
    (
        flddTest
        , CQCIntfEng_VUMeter::f8MinRange
        , CQCIntfEng_VUMeter::f8MaxRange
        , kCIDLib::True
        , f8Min
        , f8Max
    );

    // If it's good, set up our parent class with this range info
    if (bRes && bStore)
        SetValueAttrs(f8Min, f8Max, kCIDLib::False);
    return bRes;
}


tCIDLib::TBoolean
TCQCIntfFldVUMeter::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldVUMeter& iwdgOther(static_cast<const TCQCIntfFldVUMeter&>(iwdgSrc));

    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfFldVUMeter::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldVUMeter&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldVUMeter::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfFldVUMeter::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldVUMeter::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFldVUMeter::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Value Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldVUMeter::RegisterFields(         TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldVUMeter::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldVUMeter::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


// Report any validation errors in our configuration
tCIDLib::TVoid
TCQCIntfFldVUMeter::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVUMeter: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFldVUMeter::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    tCIDLib::TFloat8 f8New;
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        const TCQCFldValue& fvVal = fvCurrent();

        //
        //  Get the value out. If the value is outside of our range, then set
        //  us into error state.
        //
        if (fvVal.eFldType() == tCQCKit::EFldTypes::Card)
        {
            const TCQCCardFldValue& fvCard
                        = static_cast<const TCQCCardFldValue&>(fvVal);
            f8New = tCIDLib::TFloat8(fvCard.c4Value());
        }
         else if (fvVal.eFldType() == tCQCKit::EFldTypes::Float)
        {
            const TCQCFloatFldValue& fvFloat
                        = static_cast<const TCQCFloatFldValue&>(fvVal);
            f8New = fvFloat.f8Value();
        }
         else
        {
            const TCQCIntFldValue& fvInt
                        = static_cast<const TCQCIntFldValue&>(fvVal);
            f8New = tCIDLib::TFloat8(fvInt.i4Value());
        }

        bSetValue(f8New, bNoRedraw, kCIDLib::False);
    }
     else
    {
        // Set him into error state
        bSetValue(0, bNoRedraw, kCIDLib::True);
    }
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfFldVUMeter::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FldFmtVersion;
    strmToReadFrom  >> c2FldFmtVersion;
    if (!c2FldFmtVersion || (c2FldFmtVersion > CQCIntfEng_VUMeter::c2FldFmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FldFmtVersion)
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

tCIDLib::TVoid TCQCIntfFldVUMeter::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VUMeter::c2FldFmtVersion;

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
TCQCIntfFldVUMeter::ValueUpdate(        TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}






// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarVUMeter
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarVUMeter: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarVUMeter::TCQCIntfVarVUMeter() :

    TCQCIntfVUMeterBase
    (
        CQCIntfEng_VUMeter::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_DigitalVUMeter)
    )
{
}

TCQCIntfVarVUMeter::TCQCIntfVarVUMeter(const TCQCIntfVarVUMeter& iwdgToCopy) :

    TCQCIntfVUMeterBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarVUMeter::~TCQCIntfVarVUMeter()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarVUMeter: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarVUMeter&
TCQCIntfVarVUMeter::operator=(const TCQCIntfVarVUMeter& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarVUMeter: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfVarVUMeter::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarVUMeter& iwdgOther(static_cast<const TCQCIntfVarVUMeter&>(iwdgSrc));

    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarVUMeter::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarVUMeter&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarVUMeter::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfVarVUMeter::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarVUMeter::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarVUMeter::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfVarVUMeter::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarVUMeter::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarVUMeter: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Our variable mixin calls us here when we acquire the associated variable,
//  to make sure it has the correct attributes for our use. We can only use
//  variables of numeric type with range limits that are within a specific
//  min/max range. There's a helper method in the kit facility that will
//  do this check for us. If it's good, we need to set up our parent class
//  with this range info.
//
tCIDLib::TBoolean
TCQCIntfVarVUMeter::bCanAcceptVar(const TCQCActVar& varToTest)
{
    tCIDLib::TFloat8 f8RealMin;
    tCIDLib::TFloat8 f8RealMax;

    const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
    (
        varToTest.strName()
        , varToTest.eType()
        , varToTest.strLimits()
        , CQCIntfEng_VUMeter::f8MinRange
        , CQCIntfEng_VUMeter::f8MaxRange
        , kCIDLib::True
        , f8RealMin
        , f8RealMax
    );

    // If it's good, store it on our parent class
    if (bRes)
        SetValueAttrs(f8RealMin, f8RealMax, kCIDLib::True);

    return bRes;
}


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarVUMeter::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VUMeter::c2VarFmtVersion))
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

    // We don't have any persistent data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfVarVUMeter::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_VUMeter::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarVUMeter::ValueUpdate(        TCQCPollEngine&
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
TCQCIntfVarVUMeter::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    bSetValue(0, bNoRedraw, kCIDLib::True);
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarVUMeter::VarValChanged(  const   TCQCActVar&         varNew
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    //
    //  Get the value out. If the value is outside of our range, then set
    //  us into error state.
    //
    tCIDLib::TFloat8 f8New;
    if (varNew.eType() == tCQCKit::EFldTypes::Card)
    {
        const TCQCCardFldValue& fvCard
                    = static_cast<const TCQCCardFldValue&>(varNew.fvCurrent());
        f8New = tCIDLib::TFloat8(fvCard.c4Value());
    }
     else if (varNew.eType() == tCQCKit::EFldTypes::Float)
    {
        const TCQCFloatFldValue& fvFloat
                    = static_cast<const TCQCFloatFldValue&>(varNew.fvCurrent());
        f8New = fvFloat.f8Value();
    }
     else
    {
        const TCQCIntFldValue& fvInt
                    = static_cast<const TCQCIntFldValue&>(varNew.fvCurrent());
        f8New = tCIDLib::TFloat8(fvInt.i4Value());
    }

    bSetValue(f8New, bNoRedraw, kCIDLib::False);
}

