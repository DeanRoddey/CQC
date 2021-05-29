//
// FILE NAME: CQCIntfEng_LinMPatchImg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/11/2013
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
//  This file implements a linear multi-patch image.
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
#include    "CQCIntfEng_LinMPatchImg.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfLinMPatchImg,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_LinMPatchImg
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::None
        );


        // -----------------------------------------------------------------------
        //  The image list keys we support
        // -----------------------------------------------------------------------
        constexpr const tCIDLib::TCh* const   pszKey_Center   = L"Center";
        constexpr const tCIDLib::TCh* const   pszKey_Fill     = L"Fill";
        constexpr const tCIDLib::TCh* const   pszKey_LT       = L"Left/Top";
        constexpr const tCIDLib::TCh* const   pszKey_RB       = L"RightBottom";
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLinMPatchImg
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfLinMPatchImg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfLinMPatchImg::TCQCIntfLinMPatchImg() :

    TCQCIntfWidget
    (
        CQCIntfEng_LinMPatchImg::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Image)
    )
    , MCQCIntfImgListIntf()
    , m_bHorizontal(kCIDLib::True)
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until one is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Add our image keys
    AddNewImgKey(CQCIntfEng_LinMPatchImg::pszKey_Center, tCQCIntfEng::EImgReqFlags::Optional);
    AddNewImgKey(CQCIntfEng_LinMPatchImg::pszKey_Fill, tCQCIntfEng::EImgReqFlags::Required);
    AddNewImgKey(CQCIntfEng_LinMPatchImg::pszKey_LT, tCQCIntfEng::EImgReqFlags::Required);
    AddNewImgKey(CQCIntfEng_LinMPatchImg::pszKey_RB, tCQCIntfEng::EImgReqFlags::Required);
}

TCQCIntfLinMPatchImg::TCQCIntfLinMPatchImg(const TCQCIntfLinMPatchImg& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , m_bHorizontal(iwdgSrc.m_bHorizontal)
{
}

TCQCIntfLinMPatchImg::~TCQCIntfLinMPatchImg()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfLinMPatchImg: Public operators
// ---------------------------------------------------------------------------
TCQCIntfLinMPatchImg&
TCQCIntfLinMPatchImg::operator=(const TCQCIntfLinMPatchImg& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        m_bHorizontal = iwdgSrc.m_bHorizontal;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLinMPatchImg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfLinMPatchImg::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfLinMPatchImg::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfLinMPatchImg& iwdgOther(static_cast<const TCQCIntfLinMPatchImg&>(iwdgSrc));

    // Do mixins
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther))
        return kCIDLib::False;

    // And do our stuff
    return (m_bHorizontal == iwdgOther.m_bHorizontal);
}


tCIDLib::TVoid TCQCIntfLinMPatchImg::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfLinMPatchImg&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfLinMPatchImg::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Load any images we are using
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);
}



tCIDLib::TVoid TCQCIntfLinMPatchImg::QueryContentSize(TSize& szToFill)
{
    // Get the image bounds. If not set, then take the default
    TArea areaBounds = areaImgBounds();
    if (areaBounds.bIsEmpty())
        areaBounds.SetSize(MCQCIntfImgListIntf::szImgDefault());

    //
    //  We primarily want to insure that we are sized to the images in the
    //  transverse axis, so that the tiling will only show the images once
    //  in each slot.
    //
    szToFill = areaActual().szArea();
    if (m_bHorizontal)
        szToFill.c4Height(areaBounds.c4Height());
    else
        szToFill.c4Width(areaBounds.c4Width());

    // If we have a border, then increase by one
    if (bHasBorder())
        szToFill.Adjust(1, 1);
}


// Add any images that we reference to the passed list
tCIDLib::TVoid TCQCIntfLinMPatchImg::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just pass it on to our image mixin
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfLinMPatchImg::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Transparent);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);

    // Update the names of any colors we keep
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do any mixins
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfLinMPatchImg::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


// Support search and replace on us
tCIDLib::TVoid
TCQCIntfLinMPatchImg::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
                                , const TString&                strSrc
                                , const TString&                strTar
                                , const tCIDLib::TBoolean       bRegEx
                                , const tCIDLib::TBoolean       bFull
                                ,       TRegEx&                 regxFind)
{
    // Pass on to our parent first
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);

    // If doing images, pass on to our image list mixin
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);
}



//
//  Return a default initial area for the designer to use. We get the deault
//  image size and scale it by three in our orientation direction, since
//  that's about what is likely to be a good starting point.
//
TSize TCQCIntfLinMPatchImg::szDefaultSize() const
{
    TSize szRet = MCQCIntfImgListIntf::szImgDefault();

    if (m_bHorizontal)
        szRet.Scale(3.0, 1.0);
    else
        szRet.Scale(1.0, 3.0);
    return szRet;
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfLinMPatchImg::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfLinMPatchImg::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfLinMPatchImg: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get or set the horz/vert orientation setting
tCIDLib::TBoolean TCQCIntfLinMPatchImg::bHorizontal() const
{
    return m_bHorizontal;
}

tCIDLib::TBoolean
TCQCIntfLinMPatchImg::bHorizontal(const tCIDLib::TBoolean bToSet)
{
    m_bHorizontal = bToSet;
    return m_bHorizontal;
}



// ---------------------------------------------------------------------------
//  TCQCIntfLinMPatchImg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCIntfLinMPatchImg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_LinMPatchImg::c2FmtVersion))
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
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    // Read in our stuff
    strmToReadFrom >> m_bHorizontal;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfLinMPatchImg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_LinMPatchImg::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);

    // Write out our stuff and the end object marker
    strmToWriteTo   << m_bHorizontal
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfLinMPatchImg::Update(       TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Set the remaining region as our clip region
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the required images. If any are missing, then do nothing
    MCQCIntfImgListIntf::TImgInfo* pimgiFill = pimgiForKeyEO
    (
        CQCIntfEng_LinMPatchImg::pszKey_Fill
    );

    MCQCIntfImgListIntf::TImgInfo* pimgiLT = pimgiForKeyEO
    (
        CQCIntfEng_LinMPatchImg::pszKey_LT
    );

    MCQCIntfImgListIntf::TImgInfo* pimgiRB = pimgiForKeyEO
    (
        CQCIntfEng_LinMPatchImg::pszKey_RB
    );

    if (!pimgiFill || !pimgiLT || !pimgiRB)
        return;

    // Set up initial areas that we can move into place
    const TArea& areaUs = areaActual();
    TArea areaLT(TPoint::pntOrigin, pimgiLT->m_cptrImg->szImage());
    TArea areaRB(TPoint::pntOrigin, pimgiRB->m_cptrImg->szImage());
    TArea areaCenter;

    // The center image is optional
    MCQCIntfImgListIntf::TImgInfo* pimgiC = pimgiForKeyEO
    (
        CQCIntfEng_LinMPatchImg::pszKey_Center
    );
    if (pimgiC)
    {
        areaCenter.Set(TPoint::pntOrigin, pimgiC->m_cptrImg->szImage());
        areaCenter.CenterIn(areaUs);
    }

    //
    //  If we have no center, then the fill goes between the two end
    //  caps. Else, we have two fil areas, between each end cap and the
    //  neds of the center image.
    //
    //  WE have to be careful of the end caps overlapping the center
    //  image, which could cause us to do some bad size calcs. If that
    //  happens, zero the fill areas and we won't do any fills.
    //
    TArea areaFill1(areaUs);
    TArea areaFill2(areaUs);
    if (m_bHorizontal)
    {
        areaLT.LeftJustifyIn(areaUs, kCIDLib::True);
        areaRB.RightJustifyIn(areaUs, kCIDLib::True);

        if (pimgiC)
        {
            if (areaLT.bIntersects(areaCenter)
            ||  areaRB.bIntersects(areaCenter))
            {
                areaFill1.ZeroAll();
                areaFill2.ZeroAll();
            }
             else
            {
                areaFill1.i4X(areaLT.i4Right());
                areaFill1.i4Right(areaCenter.i4X() - 1);

                areaFill2.i4X(areaCenter.i4Right());
                areaFill2.i4Right(areaRB.i4X() - 1);
            }
        }
         else
        {
            areaFill1.i4X(areaLT.i4Right());
            areaFill1.i4Right(areaRB.i4X() - 1);
        }
    }
     else
    {
        areaLT.TopJustifyIn(areaUs, kCIDLib::True);
        areaRB.BottomJustifyIn(areaUs, kCIDLib::True);

        if (pimgiC)
        {
            if (areaLT.bIntersects(areaCenter)
            ||  areaRB.bIntersects(areaCenter))
            {
                areaFill1.ZeroAll();
                areaFill2.ZeroAll();
            }
             else
            {
                areaFill1.i4Y(areaLT.i4Bottom());
                areaFill1.i4Bottom(areaCenter.i4Y() - 1);

                areaFill2.i4Y(areaCenter.i4Bottom());
                areaFill2.i4Bottom(areaRB.i4Y() - 1);
            }
        }
         else
        {
            areaFill1.i4Y(areaLT.i4Bottom());
            areaFill1.i4Right(areaRB.i4Y() - 1);
        }
    }

    // Draw these centered in their own areas
    if (areaLT.bIntersects(areaInvalid))
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiLT->m_cptrImg
            , areaLT
            , areaInvalid
            , tCIDGraphDev::EPlacement::Center
            , pimgiLT->m_i4HOffset
            , pimgiLT->m_i4VOffset
            , pimgiLT->m_c1Opacity
        );
    }

    if (areaRB.bIntersects(areaInvalid))
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiRB->m_cptrImg
            , areaRB
            , areaInvalid
            , tCIDGraphDev::EPlacement::Center
            , pimgiRB->m_i4HOffset
            , pimgiRB->m_i4VOffset
            , pimgiRB->m_c1Opacity
        );
    }

    // If we have a center image, do that
    if (pimgiC && areaCenter.bIntersects(areaInvalid))
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiC->m_cptrImg
            , areaCenter
            , areaInvalid
            , tCIDGraphDev::EPlacement::Center
            , pimgiC->m_i4HOffset
            , pimgiC->m_i4VOffset
            , pimgiC->m_c1Opacity
        );
    }

    // And tile the fill areas if we need to do them
    if (!areaFill1.bIsEmpty() && areaFill1.bIntersects(areaInvalid))
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiFill->m_cptrImg
            , areaFill1
            , areaInvalid
            , tCIDGraphDev::EPlacement::Tile
            , pimgiFill->m_i4HOffset
            , pimgiFill->m_i4VOffset
            , pimgiFill->m_c1Opacity
        );
    }

    if (pimgiC
    &&  !areaFill2.bIsEmpty()
    &&  areaFill2.bIntersects(areaInvalid))
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiFill->m_cptrImg
            , areaFill2
            , areaInvalid
            , tCIDGraphDev::EPlacement::Tile
            , pimgiFill->m_i4HOffset
            , pimgiFill->m_i4VOffset
            , pimgiFill->m_c1Opacity
        );
    }
}


