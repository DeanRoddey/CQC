//
// FILE NAME: CQCIntfEng_StaticImg.cpp
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
#include    "CQCIntfEng_StaticImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfStaticImg,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_StaticImg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        // -----------------------------------------------------------------------
        const tCIDLib::TCard2   c2FmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        const tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::HitTrans
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticImg::TCQCIntfStaticImg() :

    TCQCIntfWidget
    (
        CQCIntfEng_StaticImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Image)
    )
    , MCQCIntfImgIntf(kCIDLib::True)
{
    // Set out default look
    bIsTransparent(kCIDLib::True);
    SetSize(TSize(122, 122), kCIDLib::False);
    strImageName(L"/System/Icons/Deans/Audio");

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/StaticImage");
}

TCQCIntfStaticImg::TCQCIntfStaticImg(const TCQCIntfStaticImg& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgIntf(iwdgSrc)
{
}

TCQCIntfStaticImg::~TCQCIntfStaticImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticImg&
TCQCIntfStaticImg::operator=(const TCQCIntfStaticImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgIntf::operator=(iwdgSrc);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfStaticImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticImg& iwdgOther(static_cast<const TCQCIntfStaticImg&>(iwdgSrc));

    // Nothing else here but the image mixin
    return MCQCIntfImgIntf::bSameImg(iwdgOther);
}


tCIDLib::TVoid TCQCIntfStaticImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticImg&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfStaticImg::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetImage)
    {
        // We need a data server client
        TDataSrvClient dsclLoad;

        // Set the new image and redraw
        SetNewImage
        (
            civOwner(), dsclLoad, ccfgToDo.piAt(0).m_strValue, kCIDLib::False
        );

        if (bCanRedraw(strEventId))
            Redraw();
    }
     else if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
    {
        c1Opacity(tCIDLib::TCard1(ccfgToDo.piAt(0).m_strValue.c4Val()));
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
TCQCIntfStaticImg::Initialize(  TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Load any background image we have. The image is handled by the
    //  image mixin, so we just delegate to it.
    //
    MCQCIntfImgIntf::InitImage(civOwner(), dsclInit, colErrs, *this);
}


tCIDLib::TVoid TCQCIntfStaticImg::QueryContentSize(TSize& szToFill)
{
    // Get the size of the bitmap
    szToFill = szImage();
    szToFill.TakeLarger(TSize(8, 8));

    // If we have a border, then increase by two
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Add any images that we reference to the passed list
tCIDLib::TVoid TCQCIntfStaticImg::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just pass it on to our image mixin
    MCQCIntfImgIntf::QueryImgPath(colToFill, bIncludeSysImgs);
}


// Give back our list of support action commands
tCIDLib::TVoid
TCQCIntfStaticImg::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                            , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // And add our commands
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetImage
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetImage)
            , tCQCKit::ECmdPTypes::ImgPath
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_ImagePath)
        )
    );

    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCIntfEng::strCmdId_SetOpacity
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetOpacity)
            , tCQCKit::ECmdPTypes::Unsigned
            , facCQCIntfEng().strMsg(kIEngMsgs::midCmdP_Opacity)
        )
    );
}


tCIDLib::TVoid
TCQCIntfStaticImg::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
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

    // Handle any of our mixins. Image is not optional for us
    MCQCIntfImgIntf::QueryImgAttrs(colAttrs, adatTmp, kCIDLib::False);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfStaticImg::RefreshImages(TDataSrvClient& dsclInit)
{
    // Just delegate to our image interface mixin
    MCQCIntfImgIntf::UpdateImage(civOwner(), dsclInit);
}


// Support search and replace on us
tCIDLib::TVoid
TCQCIntfStaticImg::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    // Pass on to our parent first
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If doing images, pass on to our image mixin
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);
}



TSize TCQCIntfStaticImg::szDefaultSize() const
{
    // Get the biggest image, or a default
    TSize szRet = szImage();

    // If empty then set a default that fits our default, else take the default
    if (szRet.bAllZero())
        szRet.Set(122, 122);
    else
        szRet = szImgDefault();
    return szRet;
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfStaticImg::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    if (ccfgToSet.strCmdId() == kCQCIntfEng::strCmdId_SetOpacity)
        ccfgToSet.piAt(0).m_strValue = L"255";
}


tCIDLib::TVoid
TCQCIntfStaticImg::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do any mixins
    MCQCIntfImgIntf::SetImgAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfStaticImg::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImage(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticImg: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We can optionally be transparent for hit testing purposes. See the header comments
//  as to why.
//
tCIDCtrls::EWndAreas TCQCIntfStaticImg::eHitTest(const TPoint& pntTest) const
{
    // The flag is on the image mixin
    if (bImgHitTrans())
        return tCIDCtrls::EWndAreas::Transparent;

    return TParent::eHitTest(pntTest);
}


// Stream our contents in from the source stream
tCIDLib::TVoid TCQCIntfStaticImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_StaticImg::c2FmtVersion))
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

    // Handle any mixins
    MCQCIntfImgIntf::ReadInImage(strmToReadFrom);

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


// Stream our contents out to the target stream
tCIDLib::TVoid TCQCIntfStaticImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_StaticImg::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins
    MCQCIntfImgIntf::WriteOutImage(strmToWriteTo);

    // Write out our own stuff (none currently) and and end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfStaticImg::Update(          TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip and draw
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();
    facCQCIntfEng().DrawImage
    (
        gdevTarget
        , kCIDLib::False
        , cptrImage()
        , areaDraw
        , areaInvalid
        , ePlacement()
        , 0
        , 0
        , c1Opacity()
    );
}


