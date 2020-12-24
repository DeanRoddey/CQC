//
// FILE NAME: CQCIntfEng_DrvDBImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2013
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
//  This file implements a static image widget. It just displays an image
//  statically, with some placement options.
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
#include    "CQCIntfEng_DrvBDImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfDrvBDImg, TCQCIntfSpecialImg)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_DrvBDImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our driver backdoor image persistent format version
        //
        //  Version 1 -
        //      Initial version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2DrvBDFmtVersion = 1;


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
//   CLASS: TCQCIntfDrvBDImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the copy ctor
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfDrvBDImg::TCQCIntfDrvBDImg() :

    TCQCIntfSpecialImg
    (
        CQCIntfEng_DrvBDImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_DrvBDImage)
    )
    , MCQCIntfSingleFldIntf(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite))
    , m_c1Opacity(0xFF)
    , m_c4SerialNum(1)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/DriverImage");
}

TCQCIntfDrvBDImg::TCQCIntfDrvBDImg(const TCQCIntfDrvBDImg& iwdgSrc) :

    TCQCIntfSpecialImg(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
    , m_bmpBgn(iwdgSrc.m_bmpBgn)
    , m_c1Opacity(iwdgSrc.m_c1Opacity)
    , m_c4SerialNum(iwdgSrc.m_c4SerialNum)
    , m_strDName(iwdgSrc.m_strDName)
{
}

TCQCIntfDrvBDImg::~TCQCIntfDrvBDImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Public operators
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the assignment
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfDrvBDImg&
TCQCIntfDrvBDImg::operator=(const TCQCIntfDrvBDImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
        m_bmpBgn        = iwdgSrc.m_bmpBgn;
        m_c1Opacity     = iwdgSrc.m_c1Opacity;
        m_c4SerialNum   = iwdgSrc.m_c4SerialNum;
        m_strDName      = iwdgSrc.m_strDName;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Called to let us indicate if we can accept. In our case, we can accept
//  anything, since all we care is that it changed.
//
tCIDLib::TBoolean
TCQCIntfDrvBDImg::bCanAcceptField(  const   TCQCFldDef&
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean)
{
    return kCIDLib::True;
}


// We can size to the image
tCIDLib::TBoolean TCQCIntfDrvBDImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCIntfDrvBDImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfDrvBDImg& iwdgOther
    (
        static_cast<const TCQCIntfDrvBDImg&>(iwdgSrc)
    );

    // Check our mixins
    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    return
    (
        (m_c1Opacity == iwdgOther.m_c1Opacity)
        && m_strDName.bCompareI(iwdgOther.m_strDName)
    );
}


// Provide access to our sample image for design purposes
TBitmap TCQCIntfDrvBDImg::bmpSample() const
{
    return m_bmpBgn;
}


tCIDLib::TCard1 TCQCIntfDrvBDImg::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfDrvBDImg::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


tCIDLib::TVoid
TCQCIntfDrvBDImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfDrvBDImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfDrvBDImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
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
TCQCIntfDrvBDImg::Initialize(TCQCIntfContainer* const   piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // If in designer mode, load a dummy image
    if (TFacCQCIntfEng::bDesMode())
        m_bmpBgn = TBitmap(facCQCIntfEng(), kCQCIntfEng::ridJpg_SampleCvr);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfDrvBDImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // Change the field this widget is associated with
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


tCIDLib::TVoid TCQCIntfDrvBDImg::QueryContentSize(TSize& szToFill)
{
    szToFill.Set(48, 64);
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfDrvBDImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfDrvBDImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
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
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Trigger Field");

    // And do our stuff
    adatTmp.Set(L"Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Data Name", kCQCIntfEng::strAttr_DBImage_DName, tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(m_strDName);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_Img_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(tCIDLib::TCard4(m_c1Opacity));
    colAttrs.objAdd(adatTmp);
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfDrvBDImg::RegisterFields(       TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfDrvBDImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfDrvBDImg::szDefaultSize() const
{
    return TSize(48, 64);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfDrvBDImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    // And our attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_DBImage_DName)
        m_strDName = adatNew.strValue();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_DBImage_Opacity)
        m_c1Opacity = tCIDLib::TCard1(adatNew.c4Val());
}


tCIDLib::TVoid
TCQCIntfDrvBDImg::Validate( const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And pass it on to our field mixin
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the data name
const TString& TCQCIntfDrvBDImg::strDName() const
{
    return m_strDName;
}

const TString& TCQCIntfDrvBDImg::strDName(const TString& strToSet)
{
    m_strDName = strToSet;
    return m_strDName;
}


// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by our field mixin when the field value changes
tCIDLib::TVoid
TCQCIntfDrvBDImg::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();

    //
    //  Clear our image first. If we don't get a new one, we end up
    //  with an empty one.
    //
    m_bmpBgn.Reset();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        try
        {
            DownloadArt();
        }

        catch(TError& errToCatch)
        {
            // We'll end up with an empty image
            if (facCQCIntfEng().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    // If not hidden or supressed, then update with our new value
    if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
        Redraw();
}


tCIDLib::TVoid TCQCIntfDrvBDImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_DrvBDImg::c2DrvBDFmtVersion))
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

    // Reset our stuff in
    strmToReadFrom  >>  m_c1Opacity
                    >>  m_strDName;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfDrvBDImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_DrvBDImg::c2DrvBDFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    strmToWriteTo   << m_c1Opacity
                    << m_strDName
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfDrvBDImg::Update(       TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    // Do any bgn/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Set any adjusted clip
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    //
    //  If we have a good field value, or we are in designer mode, then
    //  we can draw the image.
    //
    if (bGoodFieldValue() || TFacCQCIntfEng::bDesMode())
    {
        const TArea areaDraw = grgnClip.areaBounds();

        // Calc our image position
        TArea areaSrc(TPoint::pntOrigin, m_bmpBgn.szBitmap());
        TArea areaTarget;
        const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
        (
            areaDraw, areaSrc, areaTarget, tCIDGraphDev::EPlacement::FitAR, kCIDLib::False
        );

        //
        //  If the target area intersects the invalid area, draw it. We don't
        //  need any fancy stuff here. But on the off chance that the source
        //  image might have alpha transparency, we'll check for that and draw
        //  correctly to use it. Probably they won't though.
        //
        if (areaTarget.bIntersects(areaInvalid))
        {
            if (m_bmpBgn.bHasAlpha() || (m_c1Opacity < 0xFF))
            {
                gdevTarget.AlphaBlit
                (
                    m_bmpBgn, areaSrc, areaTarget, m_c1Opacity, m_bmpBgn.bHasAlpha()
                );
            }
             else
            {
                gdevTarget.DrawBitmap(m_bmpBgn, areaSrc, areaTarget);
            }
        }
    }
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfDrvBDImg::ValueUpdate(          TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}



// ---------------------------------------------------------------------------
//  TCQCIntfDrvBDImg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When we see a change in the field, we know we need to go back and get art so this
//  is called.
//
tCIDLib::TVoid TCQCIntfDrvBDImg::DownloadArt()
{
    //
    //  Clear the image and bump the serial number. This way, if we fail, we'll
    //  end up with an empty image, and it avoids complicattions below to do this
    //  now.
    //
    m_bmpBgn.Reset();
    m_bmpBgn.c4SerialNum(m_c4SerialNum++);

    //
    //  Based on our size, we will ask for either small or large art. 22,500
    //  would be roughly 150x150.
    //
    TSize szUs = areaActual().szArea();
    const tCIDLib::TBoolean bLrg(szUs.c4Width() * szUs.c4Height() > 22500);

    //
    //  Get a client proxy for this driver. The polling engine will already
    //  have it.
    //
    tCQCKit::TCQCSrvProxy orbcSrv;
    if (!facCQCIntfEng().polleThis().bGetSrvProxy(strMoniker(), orbcSrv))
        return;

    // Query the image, using the user provided data name
    tCIDLib::TCard4 c4Bytes;
    THeapBuf        mbufArt;
    if (!orbcSrv->bQueryData(strMoniker()
                            , bLrg ? L"CQCQueryLrgDrvImg" : L"CQCQuerySmlDrvImg"
                            , m_strDName
                            , c4Bytes
                            , mbufArt))
    {
        return;
    }

    TCIDImage* pimgNew;
    if (facCIDImgFact().bDecodeImg(mbufArt, c4Bytes, pimgNew, kCIDLib::False))
    {
        // Make sure it gets cleaned up
        TJanitor<TCIDImage> janImg(pimgNew);

        // And convert to a bitmap via assignment
        TBitmapImage imgArt;
        imgArt = *pimgNew;

        //
        //  Because these are very static, and they often want to scale them
        //  up or down a good bit for preview, if we are not in designer or
        //  remote modes, we'll pre-scale it to the size that it will be
        //  displayed at.
        //
        //  The reason we don't do it in remote mode is because we will then
        //  send size info during the draw that is not the same as what the
        //  client will see when he downloads the image, and that will
        //  confuse him.
        //
        if (!TFacCQCIntfEng::bDesMode()
        &&  !TFacCQCIntfEng::bRemoteMode())
        {
            TArea areaDraw(areaActual());
            if (bHasBorder())
                areaDraw.Deflate(1);
            TArea areaSrc(TPoint::pntOrigin, imgArt.szImage());
            TArea areaTarget;
            facCIDGraphDev().eCalcPlacement
            (
                areaDraw
                , areaSrc
                , areaTarget
                , tCIDGraphDev::EPlacement::FitAR
                , kCIDLib::False
            );

            areaTarget.ZeroOrg();
            if (areaTarget.szArea() != imgArt.szImage())
                imgArt.Scale(areaTarget.szArea(), 3);
        }

        // And now store the image
        m_bmpBgn = TBitmap(gdevCompat(), imgArt);
    }

    //
    //  Set the user data and bump the serial number on it for RIVA support.
    //
    TString strUserData = kCQCKit::strWRIVA_DrvPref;
    strUserData.Append(strMoniker());
    strUserData.Append(L" ");
    strUserData.Append(m_strDName);
    strUserData.Append(L" ");
    strUserData.Append(bLrg ? L"Lrg" : L"Sml");
    m_bmpBgn.strUserData(strUserData);

    m_bmpBgn.c4SerialNum(m_c4SerialNum++);
}

