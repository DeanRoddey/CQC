//
// FILE NAME: CQCIntfEng_WeatherChImg.cpp
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
//  the appropriate image for the status fields of the Weather Channel
//  driver.
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
#include    "CQCIntfEng_SpecialImg.hpp"
#include    "CQCIntfEng_WeatherChImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfWeatherChImg,TCQCIntfSpecialImg)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_WeatherChImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        //
        //  Version 2 -
        //      Support for a field association was moved from the base widget
        //      class to a mixin, so we need to know if we've already converted
        //      over or not.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ValueUpdate
        );


        // -----------------------------------------------------------------------
        //  The base repository path to our images. Underneath these are 32x32,
        //  48x48, and 128x128 directories that hold images of those respective
        //  sizes.
        // -----------------------------------------------------------------------
        const TString strRepoBase(L"/System/Weather/Weather Channel/");
    }
}



// ---------------------------------------------------------------------------
//  Weather channel widgets have a special case need in terms of the field
//  field used to populate the available list of fields in the field access
//  tab.
// ---------------------------------------------------------------------------
class TCQCWCFldFilter : public TCQCFldFilter
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCWCFldFilter() :

            TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite)
        {
        }

        TCQCWCFldFilter(const TCQCWCFldFilter& ffltToCopy) :

            TCQCFldFilter(ffltToCopy)
        {
        }

        ~TCQCWCFldFilter()
        {
        }


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCWCFldFilter& operator=(const TCQCWCFldFilter& ffiltToAssign)
        {
            if (this != &ffiltToAssign)
                TCQCFldFilter::operator=(ffiltToAssign);
            return *this;
        }


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCanAcceptField
        (
            const   TCQCFldDef&             flddToCheck
            , const TString&                strMake
            , const TString&                strModel
        )   const;


    private :
        DefPolyDup(TCQCWCFldFilter)
};


tCIDLib::TBoolean
TCQCWCFldFilter::bCanAcceptField(const  TCQCFldDef& flddToCheck
                                , const TString&    strMake
                                , const TString&    strModel) const
{
    // Gotta be cardinal type
    if (flddToCheck.eType() != tCQCKit::EFldTypes::Card)
        return kCIDLib::False;

    // Has to have a range limit
    const TString& strLimit = flddToCheck.strLimits();
    TCQCFldLimit* pfldlFld = TCQCFldLimit::pfldlMakeNew(strLimit, strLimit, flddToCheck.eType());
    TJanitor<TCQCFldLimit> janLimit(pfldlFld);

    TCQCFldCRangeLimit* pfldlCR = static_cast<TCQCFldCRangeLimit*>(pfldlFld);
    if (!pfldlCR)
        return kCIDLib::False;

    // And it has to be 0 to 48
    if ((pfldlCR->c4Min() != 0) || (pfldlCR->c4Max() > 48))
        return kCIDLib::False;

    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfWeatherChImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfWeatherChImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfWeatherChImg::TCQCIntfWeatherChImg() :

    TCQCIntfSpecialImg
    (
        CQCIntfEng_WeatherChImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_WeatherChImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCWCFldFilter)
    , m_c1Opacity(0xFF)
    , m_cptrImg(facCQCIntfEng().cptrEmptyImg())
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));
}

TCQCIntfWeatherChImg::TCQCIntfWeatherChImg(const TCQCIntfWeatherChImg& iwdgToCopy) :

    TCQCIntfSpecialImg(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
    , m_c1Opacity(iwdgToCopy.m_c1Opacity)
    , m_cptrImg(iwdgToCopy.m_cptrImg)
{
}

TCQCIntfWeatherChImg::~TCQCIntfWeatherChImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfWeatherChImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfWeatherChImg&
TCQCIntfWeatherChImg::operator=(const TCQCIntfWeatherChImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
        m_c1Opacity = iwdgToAssign.m_c1Opacity;
        m_cptrImg = iwdgToAssign.m_cptrImg;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfWeatherChImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept a new field, and optionally to
//  update ourselves appropriately for it. In our case we just create one
//  of our field objects and let it test the field. Kind of piggy but it's
//  not something called a lot. We don't need to do anything special for
//  for the store mode.
//
tCIDLib::TBoolean
TCQCIntfWeatherChImg::bCanAcceptField(  const   TCQCFldDef&         flddTest
                                        , const TString&            strMake
                                        , const TString&            strModel
                                        , const tCIDLib::TBoolean)
{
    TCQCWCFldFilter ffiltTest;
    return ffiltTest.bCanAcceptField(flddTest, strMake, strModel);
}


tCIDLib::TBoolean TCQCIntfWeatherChImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfWeatherChImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfWeatherChImg& iwdgOther(static_cast<const TCQCIntfWeatherChImg&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    // And do our stuff
    return (m_c1Opacity == iwdgOther.m_c1Opacity);
}


TBitmap TCQCIntfWeatherChImg::bmpSample() const
{
    return m_cptrImg->bmpImage();
}


tCIDLib::TCard1 TCQCIntfWeatherChImg::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfWeatherChImg::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


tCIDLib::TVoid
TCQCIntfWeatherChImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfWeatherChImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfWeatherChImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
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


tCIDLib::TVoid
TCQCIntfWeatherChImg::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Init our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  If in designer mode, set us up to display one of the images, else we
    //  don't want to display anything till we get valid date.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        try
        {
            // Build up the path to the image
            TString strName;
            BuildImgName(14, areaActual().szArea(), strName);
            m_cptrImg = facCQCIntfEng().cptrGetImage(strName, civOwner(), dsclInit);
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            facCQCIntfEng().DropImgRef(m_cptrImg);
        }
    }
     else
    {
        facCQCIntfEng().DropImgRef(m_cptrImg);
    }
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfWeatherChImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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


tCIDLib::TVoid TCQCIntfWeatherChImg::QueryContentSize(TSize& szToFill)
{
    // Get the size of the current bitmap
    szToFill = m_cptrImg->szImage();

    // If 1x1, we have no bitmap, so set it to 64x64
    if (szToFill == TSize(1,1))
        szToFill.Set(64, 64);

    // If we have a border, then increase by two
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfWeatherChImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfWeatherChImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Cond. Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfWeatherChImg::RegisterFields(       TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfWeatherChImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfWeatherChImg::szDefaultSize() const
{
    // Assume the default
    TSize szRet(64, 64);

    // If we have a border, then increase by one
    if (bHasBorder())
        szRet.Adjust(1);
    return szRet;
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfWeatherChImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfWeatherChImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our field mixin
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfWeatherChImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfWeatherChImg::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        try
        {
            // We know we are a cardinal field
            const TCQCCardFldValue& fvImg
            (
                static_cast<const TCQCCardFldValue&>(fvCurrent())
            );

            //
            //  If beyond the defined state values, then it's not applicable,
            //  so don't display anything. Else, build up the path and load
            //  the image.
            //
            if (fvImg.c4Value() > 47)
            {
                facCQCIntfEng().DropImgRef(m_cptrImg);
            }
             else
            {
                // Build up the name of the image
                TString strName;
                BuildImgName(fvImg.c4Value(), areaActual().szArea(), strName);

                TDataSrvClient dsclLoad;
                m_cptrImg = facCQCIntfEng().cptrGetImage(strName, civOwner(), dsclLoad);
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            facCQCIntfEng().DropImgRef(m_cptrImg);
        }
    }
     else
    {
        facCQCIntfEng().DropImgRef(m_cptrImg);
    }

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


//
//  We use this callback to adjust which of the image sizes we will display.
//
tCIDLib::TVoid
TCQCIntfWeatherChImg::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TString strOldPath;
    TString strNewPath;
    BuildImgName(kCIDLib::c4MaxCard, szOld, strOldPath);
    BuildImgName(kCIDLib::c4MaxCard, szNew, strNewPath);

    // If the path changed, then we selected a new image size, so reload
    if (strOldPath != strNewPath)
    {
        TDataSrvClient dsclLoad;
        m_cptrImg = facCQCIntfEng().cptrGetImage(strNewPath, civOwner(), dsclLoad);
    }
}


tCIDLib::TVoid TCQCIntfWeatherChImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_WeatherChImg::c2FmtVersion))
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
    //  In theory version 1 objects would require some adjustment to
    //  move over to the single field mixing scheme for association of fields,
    //  but we will never see such versions anymore. They cannot upgrade to
    //  anything post 3.0 until they go to 3.0 first and it does all the
    //  refactoring and conversion.
    //
    MCQCIntfSingleFldIntf::ReadInField(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom >> m_c1Opacity;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfWeatherChImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_WeatherChImg::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_c1Opacity
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfWeatherChImg::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip and draw into this remaining area
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
    TArea areaDraw = grgnClip.areaBounds();

    // And now calc our image position and draw it
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
TCQCIntfWeatherChImg::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}



// ---------------------------------------------------------------------------
//  TCQCIntfWeatherChImg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfWeatherChImg::BuildImgName( const   tCIDLib::TCard4 c4Value
                                    , const TSize&          szToUse
                                    ,       TString&        strName)
{
    strName = CQCIntfEng_WeatherChImg::strRepoBase;
    const tCIDLib::TCard4 c4Width = szToUse.c4Width();
    const tCIDLib::TCard4 c4Height = szToUse.c4Height();
    if ((c4Width >= 128) && (c4Height >= 128))
        strName.Append(L"128x128/");
    else if ((c4Width >= 64) && (c4Height >= 64))
        strName.Append(L"64x64/");
    else
        strName.Append(L"32x32/");

    if (c4Value > 47)
        strName.Append(L"na");
    else
        strName.AppendFormatted(c4Value);
}

