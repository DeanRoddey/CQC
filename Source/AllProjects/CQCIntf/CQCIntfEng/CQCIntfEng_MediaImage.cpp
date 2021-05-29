//
// FILE NAME: CQCIntfEng_MediaImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/25/2005
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
#include    "CQCIntfEng_MediaImage.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfMediaImg, TCQCIntfSpecialImg)
AdvRTTIDecls(TCQCIntfMediaRepoImg, TCQCIntfSpecialImg)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_MediaImg
    {
        // -----------------------------------------------------------------------
        //  Our media image persistent format version
        //
        //  Version 2 -
        //      Support for association with a field was moved to a mixin, so we
        //      need to know if we've already converted over to that or not.
        //
        //  Version 3 -
        //      We changed from being associated with a current cookie field in
        //      the renderer to a specialized playlist item key field. This is
        //      needed since each item can now be assocaited with a different
        //      repo. This field provides both repo and item cookie for the
        //      current item.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 3;


        // -----------------------------------------------------------------------
        //  Our media repo image persistent format version
        //
        //  Version 1 -
        //      Initial version
        //
        //  Version 2 -
        //      We added a new value to the media image attributes enum, and don't
        //      want to deal with having to artificially reorder them for display.
        //      So, since this is the only place where this enum is persisted,
        //      we just translate the old values upon load.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2RepoFmtVersion = 2;


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

        constexpr tCQCIntfEng::ECapFlags eRepoCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::None
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaImg: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the copy ctor
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfMediaImg::TCQCIntfMediaImg() :

    TCQCIntfSpecialImg
    (
        CQCIntfEng_MediaImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MediaImg)
    )
    , MCQCIntfSingleFldIntf(new TCQCCookeFldFilter)
    , m_c1Opacity(0xFF)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/MediaImage");
}

TCQCIntfMediaImg::TCQCIntfMediaImg(const TCQCIntfMediaImg& iwdgSrc) :

    TCQCIntfSpecialImg(iwdgSrc)
    , MCQCIntfSingleFldIntf(iwdgSrc)
    , m_bmpBgn(iwdgSrc.m_bmpBgn)
    , m_c1Opacity(iwdgSrc.m_c1Opacity)
{
}

TCQCIntfMediaImg::~TCQCIntfMediaImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaImg: Public operators
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the assignment
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfMediaImg& TCQCIntfMediaImg::operator=(const TCQCIntfMediaImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfSingleFldIntf::operator=(iwdgSrc);
        m_bmpBgn    = iwdgSrc.m_bmpBgn;
        m_c1Opacity = iwdgSrc.m_c1Opacity;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaImg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Called to let us indicate if we can accept, and to optionally conform our
//  self to, a new field. We don't need to do anything special to update
//  ourself if storing.
//
//  We check for the playlist item key field in V1 or V2 form.
//
tCIDLib::TBoolean
TCQCIntfMediaImg::bCanAcceptField(  const   TCQCFldDef&     flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean)
{
    if ((flddTest.eType() == tCQCKit::EFldTypes::String)
    &&  ((flddTest.strName() == TCQCStdMediaRendDrv::strRendFld1(tCQCMedia::ERendFlds::PLItemKey))
    ||   (flddTest.strName() == TCQCStdMediaRendDrv::strRendFld2(tCQCMedia::ERendFlds::PLItemKey))))
    {
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// We can size to the image
tCIDLib::TBoolean TCQCIntfMediaImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfMediaImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMediaImg& iwdgOther(static_cast<const TCQCIntfMediaImg&>(iwdgSrc));

    if (!MCQCIntfSingleFldIntf::bSameField(iwdgOther))
        return kCIDLib::False;

    return (m_c1Opacity == iwdgOther.m_c1Opacity);
}


TBitmap TCQCIntfMediaImg::bmpSample() const
{
    return m_bmpBgn;
}


tCIDLib::TCard1 TCQCIntfMediaImg::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfMediaImg::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


tCIDLib::TVoid
TCQCIntfMediaImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMediaImg&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfMediaImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
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
TCQCIntfMediaImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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


tCIDLib::TVoid
TCQCIntfMediaImg::Initialize(TCQCIntfContainer* const   piwdgParent
                             , TDataSrvClient&          dsclInit
                             , tCQCIntfEng::TErrList&   colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // If in designer mode, load a dummy image
    if (TFacCQCIntfEng::bDesMode())
        m_bmpBgn = TBitmap(facCQCIntfEng(), kCQCIntfEng::ridJpg_SampleCvr);
}


tCIDLib::TVoid TCQCIntfMediaImg::QueryContentSize(TSize& szToFill)
{
    szToFill.Set(48, 64);
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Report any monikers we reference
tCIDLib::TVoid
TCQCIntfMediaImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfMediaImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
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

    // For colors we keep, set the names of hte ones that aren't already defaulted
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"PL Key Field");

    // And add ours
    colAttrs.objPlace
    (
        L"Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    colAttrs.objPlace
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_MediaImg_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
    ).SetCard(tCIDLib::TCard4(m_c1Opacity));
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfMediaImg::RegisterFields(       TCQCPollEngine& polleToUse
                                , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfMediaImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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


TSize TCQCIntfMediaImg::szDefaultSize() const
{
    return TSize(48, 64);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMediaImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    // And store our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_MediaImg_Opacity)
        m_c1Opacity = tCIDLib::TCard1(adatNew.c4Val());
}


tCIDLib::TVoid
TCQCIntfMediaImg::Validate( const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And pass it on to our field mixin
    MCQCIntfSingleFldIntf::bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaImg: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by our field mixin when the field value changes
tCIDLib::TVoid
TCQCIntfMediaImg::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
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
        //  The current playlist index has changed, so we need to update
        try
        {
            if (cfpiAssoc.fvCurrent().eFldType() == tCQCKit::EFldTypes::String)
            {
                const TCQCStringFldValue& fvStr
                (
                    static_cast<const TCQCStringFldValue&>(cfpiAssoc.fvCurrent())
                );

                DownloadArt(fvStr.strValue());
            }
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


tCIDLib::TVoid TCQCIntfMediaImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with an object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MediaImg::c2FmtVersion))
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

    //
    //  If before version 3, then we need to adjust the associated field. It
    //  used to be a cookie field, but now it is a special playlist item key
    //  field.
    //
    if (c2FmtVersion < 3)
    {
        TString strFld = strFieldName();
        TString strMon = strMoniker();

        // Be careful to replace with the same V1/V2 type field name
        if (strFld.bStartsWithI(L"MREND#"))
            strFld = L"MREND#PLItemKey";
        else
            strFld = L"PLItemKey";

        AssociateField(strMon, strFld);
    }

    // Reset our stuff in
    strmToReadFrom >> m_c1Opacity;

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfMediaImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_MediaImg::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    strmToWriteTo << m_c1Opacity
                  << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfMediaImg::Update(       TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    //
    //  If we have a good field value, or we are in designer mode, then
    //  we can draw the image.
    //
    if (bGoodFieldValue() || TFacCQCIntfEng::bDesMode())
    {
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
TCQCIntfMediaImg::ValueUpdate(          TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}



// ---------------------------------------------------------------------------
//  TCQCIntfMediaImg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When we see a new playlist item having been set on our associated renderer
//  driver, we try to get new image data. The caller already cleared the bitmap,
//  so if we don't get anything it ends up empty.
//
tCIDLib::TVoid TCQCIntfMediaImg::DownloadArt(const TString& strKey)
{
    //
    //  See if it's standard format playlist item key. If so, we can get the art
    //  locally. If not, we'll query it from the renderer.
    //
    TString strCookie;
    TString strKeyMon;
    if (facCQCMedia().bParsePLItemKey(strKey, strKeyMon, strCookie))
    {
        //
        //  Call a helper to look up the image and create our bitmap. This will
        //  also run it through the cache. We pass zero size to let it just create
        //  the image at its natural size. Since we may be resized, we don't try
        //  to resize it. We just scale when we draw.
        //
        //  If this works, the correct persistent art key will be set on the
        //  image.
        //
        TString strDBKey;
        TSize szOrg;
        const tCIDLib::TBoolean bRes = facCQCIntfEng().bLoadLocalArt
        (
            gdevCompat()
            , strKeyMon
            , strCookie
            , TSize(0, 0)
            , kCIDLib::True
            , strDBKey
            , m_bmpBgn
            , szOrg
        );
    }
     else
    {
        //
        //  Call a helper to query the raw art buffer. If that fails, return
        //  with our image left reset.
        //
        THeapBuf mbufArt;
        tCIDLib::TCard4 c4Bytes;
        if (!facCQCMedia().bQueryCurRendArt(strMoniker(), c4Bytes, mbufArt, kCIDLib::True))
            return;

        // Convert the buffer to an image object. If that fails, return
        TCIDImage* pimgNew;
        if (!facCIDImgFact().bDecodeImg(mbufArt, c4Bytes, pimgNew, kCIDLib::False))
            return;

        // Make sure it gets cleaned up
        TJanitor<TCIDImage> janImg(pimgNew);

        // Convert to a bitmap
        m_bmpBgn = TBitmap(gdevCompat(), *pimgNew);

        //
        //  Build up a special non-standard renderer art key that the RIVA server will
        //  know means go to the renderer always.
        //
        TString strUserData = kCQCKit::strWRIVA_NSRendArtPref;
        strUserData.Append(strMoniker());
        strUserData.Append(kCIDLib::chSpace);
        strUserData.Append(strKey);

        // And set it on the image so that it will get passed to the RIVA client
        m_bmpBgn.strUserData(strUserData);
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfMediaRepoImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoImg: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the copy ctor
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfMediaRepoImg::TCQCIntfMediaRepoImg() :

    TCQCIntfSpecialImg
    (
        CQCIntfEng_MediaImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_MediaRepoImg)
    )
    , m_c1Opacity(0xFF)
    , m_eAttr(tCQCMedia::EMImgAttrs::CoverArtLrg)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));
}

TCQCIntfMediaRepoImg::TCQCIntfMediaRepoImg(const TCQCIntfMediaRepoImg& iwdgToCopy) :

    TCQCIntfSpecialImg(iwdgToCopy)
    , m_bmpBgn(iwdgToCopy.m_bmpBgn)
    , m_c1Opacity(iwdgToCopy.m_c1Opacity)
    , m_eAttr(iwdgToCopy.m_eAttr)
    , m_strRepoMoniker(iwdgToCopy.m_strRepoMoniker)
{
}

TCQCIntfMediaRepoImg::~TCQCIntfMediaRepoImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoImg: Public operators
// ---------------------------------------------------------------------------

//
//  Note that we don't technically need to copy the bitmap in the assignment
//  but in designer mode we do because the designer sets up a faux image to
//  display and this insures it gets passed on during editing.
//
TCQCIntfMediaRepoImg&
TCQCIntfMediaRepoImg::operator=(const TCQCIntfMediaRepoImg& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        m_bmpBgn        = iwdgToAssign.m_bmpBgn;
        m_c1Opacity     = iwdgToAssign.m_c1Opacity;
        m_eAttr         = iwdgToAssign.m_eAttr;
        m_strRepoMoniker= iwdgToAssign.m_strRepoMoniker;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfMediaRepoImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfMediaRepoImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfMediaRepoImg& iwdgOther(static_cast<const TCQCIntfMediaRepoImg&>(iwdgSrc));
    return
    (
        (m_c1Opacity == iwdgOther.m_c1Opacity)
        && (m_eAttr == iwdgOther.m_eAttr)
        && (m_strRepoMoniker == iwdgOther.m_strRepoMoniker)
    );
}


TBitmap TCQCIntfMediaRepoImg::bmpSample() const
{
    return m_bmpBgn;
}


tCIDLib::TCard1 TCQCIntfMediaRepoImg::c1Opacity() const
{
    return m_c1Opacity;
}

tCIDLib::TCard1 TCQCIntfMediaRepoImg::c1Opacity(const tCIDLib::TCard1 c1ToSet)
{
    m_c1Opacity = c1ToSet;
    return m_c1Opacity;
}


tCIDLib::TVoid
TCQCIntfMediaRepoImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfMediaRepoImg&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfMediaRepoImg::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if ((ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetColCookie)
    ||  (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetTitleCookie))
    {
        //
        //  We get one parameter, which is either the collection cookie or
        //  the title cookie. If we are setting the title cookie, then we
        //  need to create the collection cookie from that. We just add a
        //  ,1 to the title cookie to get to the first collection.
        //
        TString strColCookie = ccfgToDo.piAt(0).m_strValue;
        if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetTitleCookie)
            strColCookie.Append(L",1");

        //
        //  If we are showing cover art, then we can get that from the
        //  local cache. If showing poster art, we go to the repo driver
        //  and download that, since it's not something that will change
        //  often. If showing rating images, then those are just regular
        //  image repo images.
        //
        if ((m_eAttr == tCQCMedia::EMImgAttrs::CoverArtLrg)
        ||  (m_eAttr == tCQCMedia::EMImgAttrs::CoverArtSml))
        {
            // Get the art from the local cache
            TSize szOrg;
            TString strKey;
            facCQCIntfEng().bLoadLocalArt
            (
                gdevCompat()
                , m_strRepoMoniker
                , strColCookie
                , TSize(0, 0)
                , m_eAttr == tCQCMedia::EMImgAttrs::CoverArtLrg
                , strKey
                , m_bmpBgn
                , szOrg
            );
        }
         else if (m_eAttr == tCQCMedia::EMImgAttrs::PosterArt)
        {
            LoadPosterArt(strColCookie);
        }
         else
        {
            // Gotta be a rating image
            LoadRatingImg(strColCookie, civOwner().sectUser());
        }

        // Force a redraw if not in the preload phase
        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCMedia::strCmdId_SetRepository)
    {
        //
        //  Just store the new repository. It'll be used the next time they
        //  set a new cookie on us.
        //
        m_strRepoMoniker = ccfgToDo.piAt(0).m_strValue;
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
TCQCIntfMediaRepoImg::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // If in designer mode, load a dummy image
    if (TFacCQCIntfEng::bDesMode())
        m_bmpBgn = TBitmap(facCQCIntfEng(), kCQCIntfEng::ridJpg_SampleCvr);
}


tCIDLib::TVoid
TCQCIntfMediaRepoImg::QueryCmds(        TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We don't support anything in an event context
    if (eContext == tCQCKit::EActCmdCtx::IVEvent)
        return;

    // We can have a collection or title cookie set on us
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetColCookie
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetColCookie)
            , tCQCKit::ECmdPTypes::Text
            , facCQCMedia().strMsg(kMedMsgs::midCmdP_CollectCookie)
        )
    );

    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCMedia::strCmdId_SetTitleCookie
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetTitleCookie)
            , tCQCKit::ECmdPTypes::Text
            , facCQCMedia().strMsg(kMedMsgs::midCmdP_TitleCookie)
        )
    );

    // They can associate us with another repository
    {
        TCQCCmd cmdSetRepo
        (
            kCQCMedia::strCmdId_SetRepository
            , facCQCMedia().strMsg(kMedMsgs::midCmd_SetRepository)
            , tCQCKit::ECmdPTypes::Driver
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Driver)
        );
        colCmds.objAdd(cmdSetRepo);
    }
}


tCIDLib::TVoid TCQCIntfMediaRepoImg::QueryContentSize(TSize& szToFill)
{
    szToFill.Set(48, 64);
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Report any monikers we refrence
tCIDLib::TVoid
TCQCIntfMediaRepoImg::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    if (!m_strRepoMoniker.bIsEmpty())
    {
        tCIDLib::TBoolean bAdded;
        colToFill.objAddIfNew(m_strRepoMoniker, bAdded);
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfMediaRepoImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
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

    // Add ours
    adatTmp.Set(L"Image Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    // Do our stuff
    adatTmp.Set
    (
        L"Opacity"
        , kCQCIntfEng::strAttr_MediaImg_Opacity
        , L"Range: 0, 255"
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetCard(tCIDLib::TCard4(m_c1Opacity));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Repo Moniker"
        , kCQCIntfEng::strAttr_Media_Repo
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strRepoMoniker);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_RepoMoniker);
    colAttrs.objAdd(adatTmp);


    TString strLims;
    tCQCMedia::FormatEMImgAttrs(strLims, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma);
    adatTmp.Set
    (
        L"Image Type"
        , kCQCIntfEng::strAttr_MediaImg_Attr
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCMedia::strXlatEMImgAttrs(m_eAttr));
    colAttrs.objAdd(adatTmp);
}


tCIDLib::TVoid
TCQCIntfMediaRepoImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocDevice))
    {
        facCQCKit().bDoSearchNReplace
        (
            strSrc, strTar, m_strRepoMoniker, bRegEx, bFull, regxFind
        );
    }
}


TSize TCQCIntfMediaRepoImg::szDefaultSize() const
{
    return TSize(48, 64);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfMediaRepoImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // And store our stuff
    if (adatNew.strId() == kCQCIntfEng::strAttr_MediaImg_Attr)
        m_eAttr = tCQCMedia::eXlatEMImgAttrs(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_MediaImg_Opacity)
        m_c1Opacity = tCIDLib::TCard1(adatNew.c4Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Media_Repo)
        m_strRepoMoniker = adatNew.strValue();
}


tCIDLib::TVoid TCQCIntfMediaRepoImg::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_SetColCookie)
    {
        // Default it to the collection cookie source value
        ccfgToSet.SetParmAtAsRTV(0, kCQCMedia::strRTVId_ColCookie);
    }
     else if (ccfgToSet.strCmdId() == kCQCMedia::strCmdId_SetTitleCookie)
    {
        // Default it to the collection cookie source value
        ccfgToSet.SetParmAtAsRTV(0, kCQCMedia::strRTVId_TitleCookie);
    }
     else
    {
        // Not one of ours, pass long to our parent
        TParent::SetDefParms(ccfgToSet);
    }
}


tCIDLib::TVoid
TCQCIntfMediaRepoImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // Make sure our referenced media repo is available
    if (m_strRepoMoniker.bIsEmpty() || !facCQCKit().bDrvIsLoaded(m_strRepoMoniker))
    {
        TString strErr
        (
            kIEngErrs::errcVal_DrvNotFound, facCQCIntfEng(), m_strRepoMoniker
        );
        colErrs.objAdd(TCQCIntfValErrInfo(c4UniqueId(), strErr, kCIDLib::True, strWidgetId()));
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoImg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCMedia::EMImgAttrs TCQCIntfMediaRepoImg::eAttr() const
{
    return m_eAttr;
}

tCQCMedia::EMImgAttrs
TCQCIntfMediaRepoImg::eAttr(const tCQCMedia::EMImgAttrs eToSet)
{
    m_eAttr = eToSet;
    return m_eAttr;
}


const TString& TCQCIntfMediaRepoImg::strRepoMoniker() const
{
    return m_strRepoMoniker;
}

const TString& TCQCIntfMediaRepoImg::strRepoMoniker(const TString& strToSet)
{
    m_strRepoMoniker = strToSet;
    return m_strRepoMoniker;
}



// ---------------------------------------------------------------------------
//  TCQCIntfMediaRepoImg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfMediaRepoImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_MediaImg::c2RepoFmtVersion))
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

    strmToReadFrom >> m_c1Opacity
                   >> m_eAttr
                   >> m_strRepoMoniker;

    // If pre-V2, we need to translate the attribute enum
    if (c2FmtVersion == 1)
    {
        //
        //  We inserted a new value after the first value. So we need to
        //  adjust those values up by one.
        //
        if (m_eAttr > tCQCMedia::EMImgAttrs::CoverArtLrg)
            m_eAttr++;
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfMediaRepoImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_MediaImg::c2RepoFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And stream out our stuff
    strmToWriteTo<< m_c1Opacity
                  << m_eAttr
                  << m_strRepoMoniker
                  << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfMediaRepoImg::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Move in a bit more, clip and udpate
    grgnClip.Deflate(1, 1);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    // Calc our image position
    TArea areaSrc(TPoint::pntOrigin, m_bmpBgn.szBitmap());
    TArea areaTarget;
    const tCIDGraphDev::EBltTypes eBlt = facCIDGraphDev().eCalcPlacement
    (
        areaDraw
        , areaSrc
        , areaTarget
        , ((m_eAttr == tCQCMedia::EMImgAttrs::CoverArtLrg)
           || (m_eAttr == tCQCMedia::EMImgAttrs::CoverArtSml))
           ? tCIDGraphDev::EPlacement::FitAR : tCIDGraphDev::EPlacement::Center
        , kCIDLib::False
    );

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


//
//  Just to break out some code from above and keep things cleaner. This
//  is called if we are asked to display poster art from a repo driver.
//  Poster art is not locally cached since they could be large and are
//  seldom needed. So they are just downloaded from the repo as required.
//
tCIDLib::TVoid
TCQCIntfMediaRepoImg::LoadPosterArt(const TString& strColCookie)
{
    TFacCQCMedia::TMDBPtr   cptrMediaDB;
    TString                 strDBSerialNum;
    facCQCMedia().eGetMediaDB(m_strRepoMoniker, strDBSerialNum, cptrMediaDB);

    // If we have no data for this repo, then give up
    if (!cptrMediaDB.pobjData())
    {
        m_bmpBgn.Reset();
        return;
    }

    // Look up the info for this cookie
    tCQCMedia::EMediaTypes  eMType;
    const TMediaTitleSet*   pmtsFnd;
    const TMediaCollect*    pmcolFnd;
    const TMediaItem*       pmitemFnd;

    const TMediaDB& mdbInfo = cptrMediaDB->mdbData();
    const tCIDLib::TBoolean bFound = mdbInfo.bFindByCookie
    (
        strColCookie, eMType, pmtsFnd, pmcolFnd, pmitemFnd
    );

    //
    //  If not found, or found but not a collection cookie, or it's a
    //  collection but it has no poster art path, then give up.
    //
    if (!bFound || !pmcolFnd || pmcolFnd->strPosterArt().bIsEmpty())
    {
        m_bmpBgn.Reset();
        return;
    }

    // Looks reasonable, so try to download the art
    try
    {
        TBitmapImage imgNew;
        const tCIDLib::TBoolean bRes = facCQCMedia().bQueryRepoDrvArt
        (
            m_strRepoMoniker, strColCookie, tCQCMedia::ERArtTypes::Poster, imgNew
        );

        if (bRes)
        {
            m_bmpBgn = TBitmap(gdevCompat(), imgNew);

            //
            //  Set up the path for RIVA clients to use to come back and
            //  get this image.
            //
            TString strUserData = kCQCKit::strWRIVA_PArtPref;
            strUserData.Append(m_strRepoMoniker);
            strUserData.Append(L' ');
            strUserData.Append(strColCookie);
            m_bmpBgn.strUserData(strUserData);
        }
         else
        {
            m_bmpBgn.Reset();
        }
    }

    catch(...)
    {
        // We don't log this, we just clear the image
        m_bmpBgn.Reset();
    }
}



//
//  Just to break out some code from above and keep things cleaner. This
//  is called if we are asked to display poster art from a repo driver.
//  Poster art is not locally cached since they could be large and are
//  seldom needed. So they are just downloaded from the repo as required.
//
tCIDLib::TVoid
TCQCIntfMediaRepoImg::LoadRatingImg(const   TString&        strColCookie
                                    , const TCQCSecToken&   sectUser)
{
    TFacCQCMedia::TMDBPtr   cptrMediaDB;
    TString                 strDBSerialNum;
    facCQCMedia().eGetMediaDB(m_strRepoMoniker, strDBSerialNum, cptrMediaDB);

    // If we have no data for this repo, then give up
    if (!cptrMediaDB.pobjData())
    {
        m_bmpBgn.Reset();
        return;
    }

    // Look up the info for this cookie
    tCQCMedia::EMediaTypes  eMType;
    const TMediaTitleSet*   pmtsFnd;
    const TMediaCollect*    pmcolFnd;
    const TMediaItem*       pmitemFnd;

    const TMediaDB& mdbInfo = cptrMediaDB->mdbData();
    const tCIDLib::TBoolean bFound = mdbInfo.bFindByCookie
    (
        strColCookie, eMType, pmtsFnd, pmcolFnd, pmitemFnd
    );


    //
    //  If not found, or found but not a collection cookie, or it's a
    //  collection but the rating description is not set, then give up.
    //
    if (!bFound || !pmcolFnd || pmcolFnd->strRating().bIsEmpty())
    {
        m_bmpBgn.Reset();
        return;
    }

    TString strPath = L"/System/Logos/MPAA";

    // If the white versions, go down another level
    if ((m_eAttr == tCQCMedia::EMImgAttrs::RatingLgWh)
    ||  (m_eAttr == tCQCMedia::EMImgAttrs::RatingSmWh))
    {
        strPath.Append(L"/White");
    }
    strPath.Append(L"/Rated");

    // Add the name and the small suffix if needed
    strPath.Append(pmcolFnd->strRating());
    if ((m_eAttr == tCQCMedia::EMImgAttrs::RatingSm)
    ||  (m_eAttr == tCQCMedia::EMImgAttrs::RatingSmWh))
    {
        strPath.Append(L"_Small");
    }

    // Call a helper method to download the image
    try
    {
        tCIDLib::TCard4         c4SerNum = 0;
        TDataSrvClient          dsclLoad;
        tCIDLib::TEncodedTime   enctLast;
        tCIDLib::TKVPFList      colMeta;
        TPNGImage               imgNew;
        dsclLoad.bReadImage(strPath, c4SerNum, enctLast, imgNew, colMeta, sectUser);
        m_bmpBgn = TBitmap(gdevCompat(), imgNew);

        // Set the user data on it for remote viewing
        TString strUserData = kCQCKit::strWRIVA_RepoPref;
        strUserData.Append(strPath);
        m_bmpBgn.strUserData(strUserData);
    }

    catch(...)
    {
        // We don't log this, we just clear the image
        m_bmpBgn.Reset();
    }
}

