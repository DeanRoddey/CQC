//
// FILE NAME: CQCIntfEng_ProgressBar.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2008
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
//  This file implements a progress bar widget, base class and derivatives.
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
#include    "CQCIntfEng_ProgressBar.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfProgBarBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFieldProgBar,TCQCIntfProgBarBase)
AdvRTTIDecls(TCQCIntfStaticProgBar,TCQCIntfProgBarBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ProgressBar
    {
        // -----------------------------------------------------------------------
        //  Our base persistent format versions
        //
        //  Version 2 -
        //      Added support for using these as input devices, so we added a flag
        //      to let the user configure whether that's allowed or not.
        //
        //  Version 3 -
        //      Added support to make the filling of the unused part optional. In
        //      4.4.918.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  Our field based persistent format versions
        //
        //  Version 2 -
        //      Support for field association was moved from the base widget class
        //      to a mixin, so we have to know if we've already converted.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FieldFmtVersion = 2;


        // -----------------------------------------------------------------------
        //  Our static persistent format versions
        //
        //  Version 2 -
        //      Support for field association was moved from the base widget class
        //      to a mixin, so we have to know if we've already converted.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion = 2;


        // -----------------------------------------------------------------------
        //  The caps flags that we use. For the field one, we initially assume
        //  not gestures, and force the cap flag on/off as our field association
        //  changes.
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
        );


        // -----------------------------------------------------------------------
        //  The minimum and maximum range values we require a field to have before
        //  we allow one to be associated with this type of widget. Note that
        //  the low is ignore for floating point fields.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TFloat8  f8MinRange  = 1;
        constexpr tCIDLib::TFloat8  f8MaxRange  = 65535;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfPBarRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class to provide some extra
//  pbar specific RTVs.
// ---------------------------------------------------------------------------
class TCQCIntfPBarRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfPBarRTV(const   TString&        strValue
                        , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_strValue(strValue)
        {
        }

        ~TCQCIntfPBarRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue( const   TString&    strId
                                    ,       TString&    strToFill)  const
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            // If it's one of ours, then handle it
            if (strId == kCQCKit::strRTVId_CurValue)
            {
                strToFill = m_strValue;
            }
             else
            {
                return kCIDLib::False;
            }
            return kCIDLib::True;
        }


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_strValue
        //      The current value, either from tracking or the current actual
        //      value, according to whether we were tracking or not.
        // -------------------------------------------------------------------
        TString m_strValue;
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfProgBarBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Public, static data
// ---------------------------------------------------------------------------
const TString  TCQCIntfProgBarBase::strBgnKey(L"Background");
const TString  TCQCIntfProgBarBase::strFillKey(L"FillBar");
const TString  TCQCIntfProgBarBase::strOverlayKey(L"Overlay");


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfProgBarBase::~TCQCIntfProgBarBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Public, inherited methods
// ---------------------------------------------------------------------------

// We can always size to our (image based) contents
tCIDLib::TBoolean TCQCIntfProgBarBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfProgBarBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfProgBarBase& iwdgOther(static_cast<const TCQCIntfProgBarBase&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    // And do our stuff
    return
    (
        (m_bFillUnused == iwdgOther.m_bFillUnused)
        && (m_eDir == iwdgOther.m_eDir)
        && (m_bWriteable == iwdgOther.m_bWriteable)
    );
}


//
//  We allow non-inertial pans in the direction of travel. If two fingers, or
//  we are not a writeable progress bar, or in the opposite direction of our
//  configuration, then we allow it to propogate.
//
tCIDLib::TBoolean
TCQCIntfProgBarBase::bPerGestOpts(  const   TPoint&             pntAt
                                    , const tCIDLib::EDirs      eDir
                                    , const tCIDLib::TBoolean   bTwoFingers
                                    ,       tCIDCtrls::EGestOpts& eToSet
                                    ,       tCIDLib::TFloat4&   f4VScale)
{
    // Allow it to propogate
    if (!m_bWriteable || bTwoFingers)
        return kCIDLib::True;

    // If in error state, we don't do anything, but we don't propogate.
    if (m_bValError)
        return kCIDLib::False;

    //
    //  If the incoming direction and our direction are on the same
    //  axis...
    //
    tCIDLib::TBoolean bProp = kCIDLib::True;

    if ((((m_eDir == tCIDLib::EDirs::Left) || (m_eDir == tCIDLib::EDirs::Right))
    &&   ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right)))
    ||  (((m_eDir == tCIDLib::EDirs::Up) || (m_eDir == tCIDLib::EDirs::Down))
    &&   ((eDir == tCIDLib::EDirs::Up) || (eDir == tCIDLib::EDirs::Down))))
    {
        eToSet = tCIDCtrls::EGestOpts::Pan;
        f4VScale = 1.0F;

        bProp = kCIDLib::False;
    }
    return bProp;
}



tCIDLib::TBoolean
TCQCIntfProgBarBase::bProcessGestEv(const   tCIDCtrls::EGestEvs   eEv
                                    , const TPoint&             pntStart
                                    , const TPoint&             pntAt
                                    , const TPoint&             pntDelta
                                    , const tCIDLib::TBoolean   bTwoFinger)
{

    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        // Save the current value
        m_f8SaveValue = f8ValFromPos(pntAt);
        m_c4LastPercent = c4CalcPercent(m_f8Value);

        // Do an initial update to this value
        SetFillMask(m_f8Value);
        Redraw();

        // Post any 'on press' events
        PostOps(*this, kCQCKit::strEvId_OnPress, tCQCKit::EActOrders::NonNested);
    }
     else if (eEv == tCIDCtrls::EGestEvs::Drag)
    {
        // Get the new tracking value and the percent that represents
        m_f8Value = f8ValFromPos(pntAt);
        const tCIDLib::TCard4 c4NewPer = c4CalcPercent(m_f8Value);

        // If the percentage changed since last time, redraw
        if (c4NewPer != m_c4LastPercent)
        {
            // Store the new last percent and update the mask
            m_c4LastPercent = c4NewPer;
            SetFillMask(m_f8Value);
            Redraw();

            // Send any on drag command we might have
            PostOps(*this, kCQCKit::strEvId_OnDrag, tCQCKit::EActOrders::Nested);
        }
    }
     else if (eEv == tCIDCtrls::EGestEvs::End)
    {
        // Nost any on release commands
        PostOps(*this, kCQCKit::strEvId_OnRelease, tCQCKit::EActOrders::NonNested);

        // Let the derived class react to the new value
        m_f8Value = f8ValFromPos(pntAt);
        UserSetValue(m_f8Value);

        //
        //  It worked so save this value and update. Else we could
        //  jerk quickly back to the previous value and then to the
        //  new value after it comes back to us.
        //
        SetValue(m_f8Value, kCIDLib::False, kCIDLib::False);
        m_c4LastPercent = c4CalcPercent(m_f8Value);
        SetFillMask(m_f8Value);

        Redraw();

        // Post any on set commands
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
    }

    // Doesn't matter what we return, since we are non-inertial
    return kCIDLib::True;
}


// We just move the value to the cick point, if this one is configured as writeable
tCIDLib::TVoid TCQCIntfProgBarBase::Clicked(const TPoint& pntAt)
{
    if (m_bWriteable)
    {
        m_f8Value = f8ValFromPos(pntAt);
        UserSetValue(m_f8Value);

        //
        //  It worked so save this value and update. Else we could
        //  jerk quickly back to the previous value and then to the
        //  new value after it comes back to us.
        //
        SetValue(m_f8Value, kCIDLib::False, kCIDLib::False);
        m_c4LastPercent = c4CalcPercent(m_f8Value);
        SetFillMask(m_f8Value);

        Redraw();

        // Post any on set commands
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
    }
}


//
//  Initialize ourself. We pass this on to our parent and any mixins.
//  If in designer mode, we put some bogus range/value values into our
//  members so that the bar will be visible for design purposes.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Initialize any mixins
    MCQCIntfImgListIntf::InitImgList(civOwner(), dsclInit, colErrs, *this);

    // Do an intial set up of our mask image
    CreateFillMask();

    //
    //  If in designer mode, set some useful values on us, so that they
    //  can see some of the bar extended, even if they've not actually
    //  configured us yet.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        m_bValError = kCIDLib::False;
        m_c4LastPercent = 50;
        m_f8Min = 0;
        m_f8Max = 16;
        m_f8SaveValue = 8;
        m_f8Value = 8;
        SetFillMask(m_f8Value);
    }
}


//
//  Gen up one of our runtime value objects. The current value we provide
//  is different according to whether we are tracking or not.
//
TCQCCmdRTVSrc*
TCQCIntfProgBarBase::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    // Ask the derived class to format it out for us
    TString strTmp;
    FormatValue(m_f8Value, strTmp);
    return new TCQCIntfPBarRTV(strTmp, cuctxToUse);
}


// We can do this on behalf of all our derivatives
tCIDLib::TVoid TCQCIntfProgBarBase::QueryContentSize(TSize& szToFill)
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
    szToFill = areaBounds.szArea();
}


// We can do this on behalf of all our derivatives
tCIDLib::TVoid TCQCIntfProgBarBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just pass on the image interface mixin
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfProgBarBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);

    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Gradient 1");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Gradient 2");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Bar Bgn");

    // Do our mixins. Our actions are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, TString::strEmpty()
    );
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);

    // Do our stuff
    adatTmp.Set(L"Progress Bar Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Fill Unused"
        , kCQCIntfEng::strAttr_PBar_FillUnused
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bWriteable);
    colAttrs.objAdd(adatTmp);


    TString strLims(kCIDMData::strAttrLim_EnumPref);
    tCIDLib::EDirs eDir = tCIDLib::EDirs::Min;
    while (eDir <= tCIDLib::EDirs::Max)
    {
        if (eDir != tCIDLib::EDirs::Min)
            strLims.Append(kCIDLib::chComma);
        strLims.Append(tCIDLib::pszXlatEDirs(eDir));
        eDir++;
    }

    adatTmp.Set
    (
        L"Direction"
        , kCQCIntfEng::strAttr_PBar_Dir
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCIDLib::pszXlatEDirs(m_eDir));
    colAttrs.objAdd(adatTmp);


    adatTmp.Set
    (
        L"Writeable"
        , kCQCIntfEng::strAttr_PBar_Writeable
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bWriteable);
    colAttrs.objAdd(adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache. We can do this on behalf of
//  all our derivatives
//
tCIDLib::TVoid
TCQCIntfProgBarBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfProgBarBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
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


// We can do this on behalf of all our derivatives
TSize TCQCIntfProgBarBase::szDefaultSize() const
{
    return MCQCIntfImgListIntf::szImgDefault();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfProgBarBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_PBar_FillUnused)
        m_bFillUnused = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_PBar_Dir)
        m_eDir = tCIDLib::eXlatEDirs(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_PBar_Writeable)
        m_bWriteable = adatNew.bVal();
}


//
//  We can do this on behalf of all our derivatives generally, but they can
//  override and do extra validation if they want, after calling us.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent first
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image list mixin
    MCQCIntfImgListIntf::ValidateImgList(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Get or set the fill used flag.
//
tCIDLib::TBoolean TCQCIntfProgBarBase::bFillUnused() const
{
    return m_bFillUnused;
}

tCIDLib::TBoolean
TCQCIntfProgBarBase::bFillUnused(const tCIDLib::TBoolean bToSet)
{
    if (m_bFillUnused != bToSet)
    {
        m_bFillUnused = bToSet;

        //
        //  If in designer mode update the fill mask so that they will
        //  see the change. We have to do the whole Create, not just the
        //  Set, since the image might have changed and it might be after the
        //  initial setting of the fill image in which case we won't have
        //  event done a create yet.
        //
        if (TFacCQCIntfEng::bDesMode())
        {
            CreateFillMask();
            SetFillMask(m_f8Value);
        }
    }

    return m_bFillUnused;
}



//
//  Get or set the writeable flag. This is both for the designer to store
//  the configured value, and for the derived class to potentially override
//  the value at viewing time.
//
tCIDLib::TBoolean TCQCIntfProgBarBase::bIsWriteable() const
{
    return m_bWriteable;
}

tCIDLib::TBoolean
TCQCIntfProgBarBase::bIsWriteable(const tCIDLib::TBoolean bToSet)
{
    m_bWriteable = bToSet;
    return m_bWriteable;
}


//
//  Get/set the direction of the bar. If it changes, we have to recreate the
//  fill mask if we are in designer mode. It can't ever change in viewer
//  mode.
//
tCIDLib::EDirs TCQCIntfProgBarBase::eDir() const
{
    return m_eDir;
}

tCIDLib::EDirs TCQCIntfProgBarBase::eDir(const tCIDLib::EDirs eToSet)
{
    if (eToSet != m_eDir)
    {
        m_eDir = eToSet;

        //
        //  If in designer mode update the fill mask so that they will
        //  see the change. We have to do the whole Create, not just the
        //  Set, since the image might have changed and it might be after the
        //  initial setting of the fill image in which case we won't have
        //  event done a create yet.
        //
        if (TFacCQCIntfEng::bDesMode())
        {
            CreateFillMask();
            SetFillMask(m_f8Value);
        }
    }
    return m_eDir;
}


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Hidden constructors and operators
// ---------------------------------------------------------------------------

//
//  Be sure to get the fill bitmap and mask pixel array set up as an in memory
//  bitmap since it has to be or we'll get errors when it's accessed.
//
TCQCIntfProgBarBase::
TCQCIntfProgBarBase(const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName)
    , MCQCIntfImgListIntf()
    , MCQCCmdSrcIntf()
    , m_bFillUnused(kCIDLib::True)
    , m_bValError(kCIDLib::True)
    , m_bWriteable(kCIDLib::False)
    , m_bmpFill
      (
        TSize(1, 1)
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
      )
    , m_c4LastPercent(0)
    , m_eDir(tCIDLib::EDirs::Left)
    , m_f8Max(0)
    , m_f8Min(0)
    , m_f8SaveValue(0)
    , m_f8Value(0)
    , m_pixaMask
      (
        tCIDImage::EPixFmts::GrayScale
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::ERowOrders::TopDown
        , TSize(1, 1)
      )
{
    //
    //  Set our initial background colors, since it's not going to have
    //  any image until one is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    //
    //  Load up the keys into the image list mixin, initially all disabled
    //  since by default we don't use them. These default settings will be
    //  overridden if we are being created to stream in a widget.
    //
    AddNewImgKey
    (
        strBgnKey, tCQCIntfEng::EImgReqFlags::Optional
    );
    AddNewImgKey
    (
        strFillKey, tCQCIntfEng::EImgReqFlags::Required
    );
    AddNewImgKey
    (
        strOverlayKey, tCQCIntfEng::EImgReqFlags::Optional
    );

    // Add a current value runtime value
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_CurrentValue)
        , kCQCKit::strRTVId_CurValue
        , tCQCKit::ECmdPTypes::Signed
    );

    // Add all of the events that we support
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnPress)
        , kCQCKit::strEvId_OnPress
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnRelease)
        , kCQCKit::strEvId_OnRelease
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnDrag)
        , kCQCKit::strEvId_OnDrag
        , tCQCKit::EActCmdCtx::NoContChange
    );
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSet)
        , kCQCKit::strEvId_OnSet
        , tCQCKit::EActCmdCtx::NoContChange
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/ProgressBar");
}

// Don't bother copying any tracking info.
TCQCIntfProgBarBase::TCQCIntfProgBarBase(const TCQCIntfProgBarBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_bFillUnused(iwdgSrc.m_bFillUnused)
    , m_bValError(iwdgSrc.m_bValError)
    , m_bWriteable(iwdgSrc.m_bWriteable)
    , m_c4LastPercent(iwdgSrc.m_c4LastPercent)
    , m_eDir(iwdgSrc.m_eDir)
    , m_f8Max(iwdgSrc.m_f8Max)
    , m_f8Min(iwdgSrc.m_f8Min)
    , m_f8SaveValue(0)
    , m_f8Value(iwdgSrc.m_f8Value)
{
    //
    //  If in designer mode, deep copy the mask bitmap, and copy the pixel
    //  array. Otherwise, we'll loose the faux display info we set up during
    //  init of the original widget. Otherwise, no need since it'll be set
    //  up when the initial value gets set.
    //
    if (TFacCQCIntfEng::bDesMode())
    {
        m_pixaMask = iwdgSrc.m_pixaMask;
        m_bmpFill = iwdgSrc.m_bmpFill;
        m_bmpFill.DeepCopy();
    }
}

TCQCIntfProgBarBase&
TCQCIntfProgBarBase::operator=(const TCQCIntfProgBarBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bFillUnused   = iwdgSrc.m_bFillUnused;
        m_bValError     = iwdgSrc.m_bValError;
        m_bWriteable    = iwdgSrc.m_bWriteable;
        m_c4LastPercent = iwdgSrc.m_c4LastPercent;
        m_eDir          = iwdgSrc.m_eDir;
        m_f8Max         = iwdgSrc.m_f8Max;
        m_f8Min         = iwdgSrc.m_f8Min;
        m_f8Value       = iwdgSrc.m_f8Value;

        //
        //  Don't copy over any tracking stuff. No object would ever get
        //  copied while it's tracking.
        //
        m_f8SaveValue = 0.0;

        //
        //  If in designer mode, deep copy the mask bitmap, and copy the pixel
        //  array. Otherwise, we'll loose the faux display info we set up during
        //  init of the original widget. Otherwise, no need since it'll be set
        //  up when the initial value gets set.
        //
        if (TFacCQCIntfEng::bDesMode())
        {
            m_pixaMask = iwdgSrc.m_pixaMask;
            m_bmpFill = iwdgSrc.m_bmpFill;
            m_bmpFill.DeepCopy();
        }
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Protected, inherited methods
// ---------------------------------------------------------------------------

// Handle streaming in and out our type of widget
tCIDLib::TVoid TCQCIntfProgBarBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ProgressBar::c2BaseFmtVersion))
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
    //  Handle any mixins. We have a special case with the command
    //  source since this was added later. Any events we need were
    //  added in the ctor, and since in V1 we won't read in anything,
    //  they will still be there. We just need to remove any existing
    //  commands.
    //
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);

    if (c2FmtVersion > 1)
        MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);
    else
        MCQCCmdSrcIntf::ResetAllOps(kCIDLib::False);

    // Now handle any of our stuff
    strmToReadFrom  >> m_eDir;

    // If V2 or beyond, read in the writeable flag, else default it
    if (c2FmtVersion > 1)
        strmToReadFrom >> m_bWriteable;
    else
        m_bWriteable = kCIDLib::False;

    // If V3 or beyond, read in the fill unused flag, else default it
    if (c2FmtVersion > 2)
        strmToReadFrom >> m_bFillUnused;
    else
        m_bFillUnused = kCIDLib::True;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any runtime stuff
    m_bValError = kCIDLib::True;
    m_c4LastPercent = 0;
    m_f8Min = 0;
    m_f8Max = 1;
    m_f8SaveValue = 0;
    m_f8Value = 0;
}


tCIDLib::TVoid
TCQCIntfProgBarBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_ProgressBar::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins. Command source as of V2 and beyond
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << m_eDir

                       // V2 stuff
                    << m_bWriteable

                       // V3 stuff
                    << m_bFillUnused

                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is called when we need to redraw. We handle redrawing on behalf of
//  all our derivatives.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Move in a bit more, clip and udpate
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    // Draw the background image, if any
    TArea                           areaSrc;
    TArea                           areaTar;
    tCIDGraphDev::EBltTypes         eBlt;
    MCQCIntfImgListIntf::TImgInfo*  pimgiSrc;

    pimgiSrc = pimgiForKeyEO(strBgnKey);
    if (pimgiSrc)
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiSrc->m_cptrImg
            , areaDraw
            , areaInvalid
            , pimgiSrc->m_i4HOffset
            , pimgiSrc->m_i4VOffset
            , pimgiSrc->m_c1Opacity
        );
    }

    //
    //  And now blit our mask if we have one and if we have a good field
    //  value currently. If we are in remote mode, we call a method on the
    //  graphics device to do the work, so that it'll get sent to the
    //  RIVA client. Else we do our own more efficient version, using our
    //  pre-created mask and such.
    //
    pimgiSrc = pimgiForKeyEO(strFillKey);
    if (!m_bValError && pimgiSrc)
    {
        areaSrc.Set(TPoint::pntOrigin, m_bmpFill.szBitmap());
        eBlt = facCIDGraphDev().eCalcPlacement
        (
            areaDraw
            , areaSrc
            , areaTar
            , tCIDGraphDev::EPlacement::Center
            , kCIDLib::False
        );

        // Apply any offset
        areaTar.AdjustOrg(pimgiSrc->m_i4HOffset, pimgiSrc->m_i4VOffset);

        //
        //  If the target area intersects the invalid area, then draw it,
        //  but we actually draw our mask image.
        //
        if (areaTar.bIntersects(areaInvalid))
        {
            if (TFacCQCIntfEng::bRemoteMode())
            {
                gdevTarget.DrawPBar
                (
                    pimgiSrc->m_cptrImg->bmpImage()
                    , tCIDLib::TFloat4(c4CalcPercent(m_f8Value)) / 100.0F
                    , areaSrc
                    , areaTar
                    , eBlt
                    , m_eDir
                    , rgbFgn()
                    , rgbFgn2()
                    , rgbFgn3()
                    , pimgiSrc->m_c1Opacity
                );
            }
             else
            {
                m_bmpFill.AdvDrawOn
                (
                    gdevTarget
                    , areaSrc
                    , areaTar
                    , kCIDLib::False
                    , 0
                    , pimgiSrc->m_cptrImg->bmpMask()
                    , eBlt
                    , pimgiSrc->m_c1Opacity
                );
            }
        }
    }

    // And finally blit the overlay image if any.
    pimgiSrc = pimgiForKeyEO(strOverlayKey);
    if (pimgiSrc)
    {
        facCQCIntfEng().DrawImage
        (
            gdevTarget
            , kCIDLib::False
            , pimgiSrc->m_cptrImg
            , areaDraw
            , areaInvalid
            , pimgiSrc->m_i4HOffset
            , pimgiSrc->m_i4VOffset
            , pimgiSrc->m_c1Opacity
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Let's the derived class know if a value is out of the set range limits
tCIDLib::TBoolean
TCQCIntfProgBarBase::bValueOutOfRange(const tCIDLib::TFloat8 f8Check) const
{
    if ((f8Check < m_f8Min) || (f8Check > m_f8Max))
        return kCIDLib::True;
    return kCIDLib::False;
}


// Let derived classes see the current value if they need to
tCIDLib::TFloat8 TCQCIntfProgBarBase::f8PBValue() const
{
    return m_f8Value;
}


//
//  Force a bad value so we'll have to redraw when we get the next value
//  update. This let's the derived classes insure we'll redraw the next time
//  a new value is set.
//
tCIDLib::TVoid TCQCIntfProgBarBase::ForceNextRedraw()
{
    m_c4LastPercent = 101;
}


//
//  This is called when the value changes and we need to update the contents
//  of the fill mask to reflect the new percentage for the bar.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::SetFillMask(const tCIDLib::TFloat8 f8TarValue)
{
    //
    //  See if we have a fill bar key. If not, then we never will have set
    //  up the fill mask, so we can't really do anything.
    //
    MCQCIntfImgListIntf::TImgInfo* pimgiSrc(pimgiForKeyEO(strFillKey));
    if (!pimgiSrc)
        return;

    // Calc the current percentage
    tCIDLib::TFloat4 f4Percent = tCIDLib::TFloat4(c4CalcPercent(f8TarValue));
    f4Percent /= 100.0;

    //
    //  Based on the direction of the fill, we may need to reverse the two
    //  colors, and remember if we are doing vertical or horz. And we need
    //  to figure out the two fill areas.
    //
    tCIDLib::TBoolean   bIsVert = kCIDLib::False;
    TArea               areaBar(TPoint::pntOrigin, pimgiSrc->m_cptrImg->szImage());
    TArea               areaFill(areaBar);
    TRGBClr             rgbStartFill;
    TRGBClr             rgbEndFill;
    switch(m_eDir)
    {
        case tCIDLib::EDirs::Left :
            rgbStartFill = rgbFgn();
            rgbEndFill = rgbFgn2();
            areaBar.ScaleSize(f4Percent, 1.0);
            areaFill.ScaleSize(1.0 - f4Percent, 1.0);
            areaFill.i4X(areaBar.i4Right() - 1);
            break;

        case tCIDLib::EDirs::Right :
            rgbStartFill = rgbFgn2();
            rgbEndFill = rgbFgn();
            areaFill.ScaleSize(1.0 - f4Percent, 1.0);
            areaBar.ScaleSize(f4Percent, 1.0);
            areaBar.i4X(areaFill.i4Right() - 1);
            break;

        case tCIDLib::EDirs::Down :
            rgbStartFill = rgbFgn();
            rgbEndFill = rgbFgn2();
            areaBar.ScaleSize(1.0, f4Percent);
            areaFill.ScaleSize(1.0, 1.0 - f4Percent);
            areaFill.i4Y(areaBar.i4Bottom() - 1);
            bIsVert = kCIDLib::True;
            break;

        case tCIDLib::EDirs::Up :
            rgbStartFill = rgbFgn2();
            rgbEndFill = rgbFgn();
            bIsVert = kCIDLib::True;
            areaFill.ScaleSize(1.0, 1.0 - f4Percent);
            areaBar.ScaleSize(1.0, f4Percent);
            areaBar.i4Y(areaFill.i4Bottom() - 1);
            bIsVert = kCIDLib::True;
            break;

        default :
            CIDAssert(0, L"Unknown direction");
            break;
    };

    // If the F1 and F2 colors are not the same, then we have to interpolate
    if (rgbStartFill != rgbEndFill)
    {
        //
        //  We need to interpolate, and do the same sort of flipping as
        //  done above if needed. To interpolate we convert to HSV format
        //  since that makes it easy to maintain the same hue and just
        //  adjust the saturation.
        //
        THSVClr hsvStart(rgbStartFill);
        THSVClr hsvEnd(rgbEndFill);
        const tCIDLib::TFloat4 f4StartSat = hsvStart.f4Saturation();
        tCIDLib::TFloat4 f4EndSat = hsvEnd.f4Saturation();

        if (f4EndSat > f4StartSat)
            f4EndSat = f4StartSat + ((f4EndSat - f4StartSat) * f4Percent);
        else
            f4EndSat = f4StartSat - ((f4StartSat - f4EndSat) * f4Percent);
        hsvEnd.f4Saturation(f4EndSat);

        // Now copy the result back
        hsvEnd.ToRGB(rgbEndFill);
    }

    //
    //  Now we need to do two fills, one with the bar colors and the rest with the
    //  unused portion color, so create a device over our mask bitmap and do the
    //  fills. We need the faux scope because we have to release the memory device
    //  before we try to access the pixel data.
    //
    //  The exception is if we are configured for 'no unused' mode, in which case
    //  We are going to set that portion transparent below so no need to fill it.
    //
    {
        TGraphMemDev gdevMask(gdevCompat(), m_bmpFill);
        gdevMask.GradientFill
        (
            areaBar
            , rgbStartFill
            , rgbEndFill
            , bIsVert ? tCIDGraphDev::EGradDirs::Vertical
                      : tCIDGraphDev::EGradDirs::Horizontal
        );

        if (m_bFillUnused)
            gdevMask.Fill(areaFill, rgbFgn3());
    }

    //
    //  Now we need to reapply the alpha channel we saved earlier, because
    //  the fills above will have whacked it.
    //
    TArea areaBlit(TPoint::pntOrigin, pimgiSrc->m_cptrImg->szImage());
    TPixelArray& pixaData = m_bmpFill.pixaData();
    pixaData.CreateAlphaChannelFrom
    (
        m_pixaMask, areaBlit, TPoint::pntOrigin, kCIDLib::False
    );

    //
    //  If not filling the unused part, then set that transparent, since we pre
    //  multiplied above, we want to just set the colors all to black as well.
    //
    if (!m_bFillUnused)
        pixaData.SetAlphaAt(0, areaFill);

    // Premultiply now, since we are going to alpha-blit this image
    pixaData.Premultiply();
}


//
//  Called by the derived class when the configuration changes such that the
//  min/max values are affected.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::SetRangeLimits(const   tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max)
{
    if ((f8Min != m_f8Min) || (f8Max != m_f8Max))
    {
        // Update our current value so that it's within the new min/max
        if (m_f8Value > f8Max)
            m_f8Value = f8Max;
        else if (m_f8Value < f8Min)
            m_f8Value = f8Min;

        // Store our min/max and force a redraw
        m_f8Max = f8Max;
        m_f8Min = f8Min;

        //
        //  If in designer mode, set the value to mid-point so that they can
        //  see the bar while desiging.
        //
        if (TFacCQCIntfEng::bDesMode())
            m_f8Value = m_f8Min + ((m_f8Max - m_f8Min) / 2);

        //
        //  Update the fill mask. We always do this on the current value.
        //  This would never get called while tracking.
        //
        SetFillMask(m_f8Value);
    }
}


//
//  The derived class calls this to tell us to set ourself to a new value.
//  We don't necessarily redraw since we deal in percentages of a range, and
//  the new value may not change the percentage. But we store the new value
//  at least.
//
tCIDLib::TVoid
TCQCIntfProgBarBase::SetValue(  const   tCIDLib::TFloat8    f8New
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean   bValError)
{
    if ((f8New != m_f8Value) || (bValError != m_bValError))
    {
        m_bValError = bValError;

        //
        //  Calculate the new precentage this represents. This is what drives
        //  our display, not the actual value. If the percentage didn't change
        //  we don't need to do anything. We'll store the value regardless for
        //  outsiders to see.
        //
        if (m_bValError)
        {
            m_f8Value = m_f8Min;
            m_c4LastPercent = 101;
        }
         else
        {
            m_f8Value = f8New;
            const tCIDLib::TCard4 c4NewPer = tCIDLib::TCard4
            (
                c4CalcPercent(f8New)
            );
            if (c4NewPer != m_c4LastPercent)
            {
                // Store the new precent and update
                m_c4LastPercent = c4NewPer;

                //
                //  Store the new value now, and redraw our fill mask image
                //  based on the new value.
                //
                SetFillMask(m_f8Value);
            }
        }

        //
        //  If not hidden or supressed, then update with our new value, even
        //  if we got a bad value, since we want to display in error state.
        //
        if ((eCurDisplayState() != tCQCIntfEng::EDispStates::Hidden) && !bNoRedraw)
            Redraw();
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfProgBarBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Calculate a new value within the range, based on the passed point. We
//  assume the caller checked that we within the axis of travel, so we
//  check that.
//
tCIDLib::TFloat8 TCQCIntfProgBarBase::f8ValFromPos(const TPoint& pntAt)
{
    //
    //  Here are are wanting to work within the area of the actual fill bar, not the
    //  optional background image which could make us larger. So get the fill bar image
    //  and center it in us.
    //
    TArea areaUs(TPoint::pntOrigin, m_bmpFill.szBitmap());
    areaUs.CenterIn(areaActual());
    tCIDLib::TFloat8 f8Ret;

    if ((m_eDir == tCIDLib::EDirs::Left) || (m_eDir == tCIDLib::EDirs::Right))
    {
        const tCIDLib::TInt4 i4Min = areaUs.i4X();
        const tCIDLib::TInt4 i4Max = areaUs.i4Right() - 1;
        const tCIDLib::TFloat8 f8Range = tCIDLib::TFloat8(i4Max - i4Min);

        if (pntAt.i4X() <= i4Min)
        {
            if (m_eDir == tCIDLib::EDirs::Left)
                f8Ret = m_f8Min;
            else
                f8Ret = m_f8Max;
        }
         else if (pntAt.i4X() >= i4Max)
        {
            if (m_eDir == tCIDLib::EDirs::Left)
                f8Ret = m_f8Max;
            else
                f8Ret = m_f8Min;
        }
         else
        {
            tCIDLib::TFloat8 f8Per;

            if (m_eDir == tCIDLib::EDirs::Left)
                f8Per = tCIDLib::TFloat8(pntAt.i4X() - i4Min) / f8Range;
            else
                f8Per = tCIDLib::TFloat8(i4Max - pntAt.i4X()) / f8Range;

            f8Ret = m_f8Min + ((m_f8Max - m_f8Min) * f8Per);
        }
    }
     else
    {
        const tCIDLib::TInt4 i4Min = areaUs.i4Y();
        const tCIDLib::TInt4 i4Max = areaUs.i4Bottom() - 1;
        const tCIDLib::TFloat8 f8Range = tCIDLib::TFloat8(i4Max - i4Min);


        if (pntAt.i4Y() >= i4Max)
        {
            if (m_eDir == tCIDLib::EDirs::Up)
                f8Ret = m_f8Min;
            else
                f8Ret = m_f8Max;
        }
         else if (pntAt.i4Y() <= i4Min)
        {
            if (m_eDir == tCIDLib::EDirs::Up)
                f8Ret = m_f8Max;
            else
                f8Ret = m_f8Min;
        }
         else
        {
            tCIDLib::TFloat8 f8Per;
            if (m_eDir == tCIDLib::EDirs::Up)
                f8Per = tCIDLib::TFloat8(i4Max - pntAt.i4Y()) / f8Range;
            else
                f8Per = tCIDLib::TFloat8(pntAt.i4Y() - i4Min) / f8Range;

            f8Ret = m_f8Min + ((m_f8Max - m_f8Min) * f8Per);
        }
    }
    return f8Ret;
}



//
//  This is called during initialization (and in design mode any time that
//  the selected image changes), so that we can create a new fill image and
//  fill mask. The direction of the mask or the images cannot change during
//  viewing, so it only needs to be called during init.
//
tCIDLib::TVoid TCQCIntfProgBarBase::CreateFillMask()
{
    // Get the image for the bar. If not set, do nothing
    MCQCIntfImgListIntf::TImgInfo* pimgiSrc = pimgiForKeyEO(strFillKey);
    if (!pimgiSrc)
        return;

    // We have one so get the size out
    TSize szImg = pimgiSrc->m_cptrImg->szImage();
    const tCIDLib::TCard4 c4CX = szImg.c4Width();
    const tCIDLib::TCard4 c4CY = szImg.c4Height();

    tCIDLib::TBoolean bValid = kCIDLib::True;
    switch(m_eDir)
    {
        case tCIDLib::EDirs::Left :
        case tCIDLib::EDirs::Right :
            if (c4CX < 4)
                bValid = kCIDLib::False;
            break;

        case tCIDLib::EDirs::Up :
        case tCIDLib::EDirs::Down :
            if (c4CY < 4)
                bValid = kCIDLib::False;
            break;

        default :
            CIDAssert(0, L"Unknown direction");
            break;
    };

    if (!pimgiSrc->m_cptrImg->bHasAlpha())
        bValid = kCIDLib::False;

    //
    //  If not valid, then nothing to do, but mark the image not enabled
    //  so we won't keep trying to draw it.
    //
    if (!bValid)
    {
        pimgiSrc->m_bEnabled = kCIDLib::False;
        return;
    }

    //
    //  We have a valid image, so let's do our thing. We need to create the fill bitmap
    //  to the new size. Force it to be 32 bit and in memory so that we can directly
    //  get to the bits. We need this for efficiency.
    //
    m_bmpFill = TBitmap
    (
        szImg
        , tCIDImage::EPixFmts::TrueAlpha
        , tCIDImage::EBitDepths::Eight
        , kCIDLib::True
    );

    //
    //  Now steal the alpha channel from the original image. It is actually
    //  not guaranteed to be an in-memory, so we need to query the image
    //  data out into a pixel array, then extract the alpha channel into
    //  another.
    //
    TClrPalette palDummy;
    TPixelArray pixaImg;
    pimgiSrc->m_cptrImg->bmpImage().QueryImgData(gdevCompat(), pixaImg, palDummy);
    m_pixaMask.Reset
    (
        tCIDImage::EPixFmts::GrayScale
        , tCIDImage::EBitDepths::Eight
        , tCIDImage::ERowOrders::TopDown
        , szImg
    );
    pixaImg.ExtractAlphaChannel(m_pixaMask);
}


//
//  We have to do this from a few spots, so it's best to break it out and do
//  it in one place. We also have to deal with calculating this for the
//  current value normally and for the tracking value while tracking, so
//  we take a current value to use.
//
tCIDLib::TCard4
TCQCIntfProgBarBase::c4CalcPercent(const tCIDLib::TFloat8 f8TarVal) const
{
    // Get the zero based value and range
    const tCIDLib::TFloat8 f8Range = (m_f8Max - m_f8Min);
    const tCIDLib::TFloat8 f8Value = f8TarVal - m_f8Min;

    // Calc the percentage and clip if needed
    tCIDLib::TCard4 c4Ret = tCIDLib::TCard4((f8Value / f8Range) * 100.0);

    if (c4Ret > 100)
        c4Ret = 100;
    return c4Ret;
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldProgBar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFieldProgBar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFieldProgBar::TCQCIntfFieldProgBar() :

    TCQCIntfProgBarBase
    (
        CQCIntfEng_ProgressBar::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_ProgressBar)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltNumRange
          (
              tCQCKit::EReqAccess::MReadCWrite
              , CQCIntfEng_ProgressBar::f8MinRange
              , CQCIntfEng_ProgressBar::f8MaxRange
              , kCIDLib::True
          )
      )
{
}

TCQCIntfFieldProgBar::TCQCIntfFieldProgBar(const TCQCIntfFieldProgBar& iwdgToCopy) :

    TCQCIntfProgBarBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFieldProgBar::~TCQCIntfFieldProgBar()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldProgBar: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFieldProgBar&
TCQCIntfFieldProgBar::operator=(const TCQCIntfFieldProgBar& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldProgBar: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept, and optionally update ourselves
//  to conform to, a new field. In our case we check that it's a numeric
//  field and if so (and we are storing) we store the range on our parent
//  class.
//
//  We also update our gesture acceptance capabilities flag based on whether
//  the field is writeale or not.
//
tCIDLib::TBoolean
TCQCIntfFieldProgBar::bCanAcceptField(  const   TCQCFldDef&         flddTest
                                        , const TString&
                                        , const TString&
                                        , const tCIDLib::TBoolean   bStore)
{
    tCIDLib::TFloat8 f8Min, f8Max;
    const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
    (
        flddTest
        , CQCIntfEng_ProgressBar::f8MinRange
        , CQCIntfEng_ProgressBar::f8MaxRange
        , kCIDLib::True
        , f8Min
        , f8Max
    );

    //
    //  If it's good snd storing, set up our parent class with this range
    //  info. We also tell it whether our field is writeable, in which
    //  case he'll allow updates from the user.
    //
    if (bRes && bStore)
    {
        SetRangeLimits(f8Min, f8Max);
        bIsWriteable(flddTest.bIsWriteable());

        tCQCIntfEng::ECapFlags eSet;
        if (flddTest.bIsWriteable())
            eSet = tCQCIntfEng::ECapFlags::DoesGestures;
        else
            eSet = tCQCIntfEng::ECapFlags::None;
        SetCapFlags(eSet, tCQCIntfEng::ECapFlags::DoesGestures);
    }
    return bRes;
}


tCIDLib::TBoolean TCQCIntfFieldProgBar::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFieldProgBar& iwdgOther(static_cast<const TCQCIntfFieldProgBar&>(iwdgSrc));

    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us
tCIDLib::TVoid TCQCIntfFieldProgBar::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFieldProgBar&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFieldProgBar::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
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


//
//  Initialize ourself. We pass this on to our parent and any mixins.
//  If in designer mode, we put some bogus range/value values into our
//  members so that the bar will be visible for design purposes.
//
tCIDLib::TVoid
TCQCIntfFieldProgBar::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFieldProgBar::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFieldProgBar::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


tCIDLib::TVoid
TCQCIntfFieldProgBar::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Just add our field mixin's stuff
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Value Field");
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldProgBar::RegisterFields(       TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFieldProgBar::Replace(  const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFieldProgBar::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);
}


// At this level, we vall our parent, then validate the field mixin
tCIDLib::TVoid
TCQCIntfFieldProgBar::Validate( const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our field mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFieldProgBar: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFieldProgBar::FieldChanged(         TCQCFldPollInfo&    cfpiAssoc
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const TStdVarsTar&        )
{
    const tCQCPollEng::EFldStates eNewState = eFieldState();
    if (eNewState == tCQCPollEng::EFldStates::Ready)
    {
        //
        //  Get the new value, either a card or int value, and update our offset
        //  and store the value.
        //
        tCIDLib::TBoolean bError = kCIDLib::False;
        tCIDLib::TFloat8 f8New;
        const TCQCFldValue& fvNewValue = fvCurrent();
        switch(fvNewValue.eFldType())
        {
            case tCQCKit::EFldTypes::Card :
            {
                const TCQCCardFldValue& fvCard
                            = static_cast<const TCQCCardFldValue&>(fvNewValue);
                f8New = tCIDLib::TFloat8(fvCard.c4Value());
                break;
            }

            case tCQCKit::EFldTypes::Float :
            {
                const TCQCFloatFldValue& fvFloat
                            = static_cast<const TCQCFloatFldValue&>(fvNewValue);
                f8New = fvFloat.f8Value();
                break;
            }

            case tCQCKit::EFldTypes::Int :
            {
                const TCQCIntFldValue& fvInt
                            = static_cast<const TCQCIntFldValue&>(fvNewValue);
                f8New = tCIDLib::TFloat8(fvInt.i4Value());
                break;
            }

            default :
                bError = kCIDLib::True;
                f8New = 0;
                break;
        }

        SetValue(f8New, bNoRedraw, bError);
    }
     else
    {
        SetValue(0, bNoRedraw, kCIDLib::True);
    }
}


//
//  The base class calls us to format out a current value it has, since
//  only we know the possible data types (whereas he has to store them
//  all as float.)
//
tCIDLib::TVoid
TCQCIntfFieldProgBar::FormatValue(  const   tCIDLib::TFloat8    f8ToFmt
                                    ,       TString&            strToFill) const
{
    const tCQCKit::EFldTypes eType = eFieldType();
    if (eType == tCQCKit::EFldTypes::Card)
        strToFill.AppendFormatted(tCIDLib::TCard4(f8ToFmt));
    else if (eType == tCQCKit::EFldTypes::Float)
        strToFill.AppendFormatted(f8ToFmt);
    else if (eType == tCQCKit::EFldTypes::Int)
        strToFill.AppendFormatted(tCIDLib::TInt4(f8ToFmt));
}


// Handle streaming in and out our type of widget
tCIDLib::TVoid TCQCIntfFieldProgBar::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ProgressBar::c2FieldFmtVersion))
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

    // Nothing of our own for now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFieldProgBar::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_ProgressBar::c2FieldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do our mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo
                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  Called if user interaction (handled by the base class) changed our value.
//  Any required redrawing is already done. We use this to update the field
//  value of our associated field.
//
//  Note that if the no auto-write flag is set in the field interface, we
//  don't do anything.
//
tCIDLib::TVoid
TCQCIntfFieldProgBar::UserSetValue(const tCIDLib::TFloat8 f8NewVal)
{
    if (!bNoAutoWrite())
    {
        const tCQCKit::EFldTypes eType = eFieldType();

        TString strValue;
        if (eType == tCQCKit::EFldTypes::Card)
            strValue.AppendFormatted(tCIDLib::TCard4(f8NewVal));
        else if (eType == tCQCKit::EFldTypes::Float)
            strValue.AppendFormatted(f8NewVal);
        else if (eType == tCQCKit::EFldTypes::Int)
            strValue.AppendFormatted(tCIDLib::TInt4(f8NewVal));

        WriteValue(strMoniker(), strFieldName(), strValue);
    }
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfFieldProgBar::ValueUpdate(          TCQCPollEngine&     polleToUse
                                    , const tCIDLib::TBoolean   bNoRedraw
                                    , const tCIDLib::TBoolean
                                    , const TStdVarsTar&        ctarGlobalVars
                                    , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticProgBar
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticProgBar: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticProgBar::TCQCIntfStaticProgBar() :

    TCQCIntfProgBarBase
    (
        CQCIntfEng_ProgressBar::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_ProgressBar)
    )
    , MCQCIntfStaticIntf(tCQCKit::EFldTypes::Int)
{
}

TCQCIntfStaticProgBar::TCQCIntfStaticProgBar(const TCQCIntfStaticProgBar& iwdgToCopy) :

    TCQCIntfProgBarBase(iwdgToCopy)
    , MCQCIntfStaticIntf(iwdgToCopy)
{
}

TCQCIntfStaticProgBar::~TCQCIntfStaticProgBar()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticProgBar: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticProgBar&
TCQCIntfStaticProgBar::operator=(const TCQCIntfStaticProgBar& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfStaticIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticProgBar: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticProgBar::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticProgBar& iwdgOther(static_cast<const TCQCIntfStaticProgBar&>(iwdgSrc));

    // There's not anything else for us at this time
    return MCQCIntfStaticIntf::bSameStatic(iwdgOther);
}


// Polymorphically copy us
tCIDLib::TVoid TCQCIntfStaticProgBar::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticProgBar&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfStaticProgBar::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
                                , const tCIDLib::TCard4     c4Index
                                , const TString&            strUserId
                                , const TString&            strEventId
                                ,       TStdVarsTar&        ctarGlobals
                                ,       tCIDLib::TBoolean&  bResult
                                ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetValue)
    {
        //
        //  Just pass the value on to our parent class. Our value is actually
        //  integral, but the base class works with floats. So we just convert
        //  it to a float. Any legal integer would convert ok to to a float.
        //
        //  We insure it's within the limits.
        //
        const tCIDLib::TFloat8 f8Val = ccfgToDo.piAt(0).m_strValue.f8Val();
        if (bValueOutOfRange(f8Val))
        {
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midCmdVR_BadNumeric
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
            );
        }

        SetValue(f8Val, !bCanRedraw(strEventId), kCIDLib::False);
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


//
//  Initialize ourself. We pass this on to our parent and any mixins.
//
tCIDLib::TVoid
TCQCIntfStaticProgBar::Initialize(  TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Set our configured range limits on the parent class.
    SetRangeLimits(f8RangeMin(), f8RangeMax());
}


//
//  We need to set an initial value on the base class, since otherwise it
//  will never happen. Our only value is internal, so there's no update pass
//  that's going to happen. If we don't do this, we don't get the initial
//  creation of the fill mask and such.
//
//  However, we may have gotten a value set in onpreload or onload, so we just
//  set the current value.
//
tCIDLib::TVoid TCQCIntfStaticProgBar::PostInit()
{
    SetValue(f8PBValue(), kCIDLib::True, kCIDLib::False);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfStaticProgBar::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // And add our commands
    colCmds.objAdd
    (
        TCQCCmd
        (
            kCQCKit::strCmdId_SetValue
            , facCQCKit().strMsg(kKitMsgs::midCmd_SetValue)
            , tCQCKit::ECmdPTypes::Signed
            , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
        )
    );
}


tCIDLib::TVoid
TCQCIntfStaticProgBar::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    MCQCIntfStaticIntf::QueryStaticAttrs(colAttrs, adatTmp);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticProgBar::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    MCQCIntfStaticIntf::SetStaticAttr(wndAttrEd, adatNew, adatOld);
}



// ---------------------------------------------------------------------------
//  TCQCIntfStaticProgBar: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  The base class calls us to format out a current value it has, since
//  only we know the possible data types (whereas he has to store them
//  all as float.)
//
tCIDLib::TVoid
TCQCIntfStaticProgBar::FormatValue(  const   tCIDLib::TFloat8    f8ToFmt
                                    ,       TString&            strToFill) const
{
    const tCQCKit::EFldTypes eType = eDataType();
    if (eType == tCQCKit::EFldTypes::Card)
        strToFill.AppendFormatted(tCIDLib::TCard4(f8ToFmt));
    else if (eType == tCQCKit::EFldTypes::Float)
        strToFill.AppendFormatted(f8ToFmt);
    else if (eType == tCQCKit::EFldTypes::Int)
        strToFill.AppendFormatted(tCIDLib::TInt4(f8ToFmt));
}


// Handle streaming in and out our type of widget
tCIDLib::TVoid TCQCIntfStaticProgBar::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ProgressBar::c2StaticFmtVersion))
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
    //  If V1, then read in the old min/max and move it over to our static
    //  mixin. Else stream in the mixin.
    //
    if (c2FmtVersion == 1)
    {
        tCIDLib::TCard4 i4CfgMax, i4CfgMin;
        strmToReadFrom >> i4CfgMax >> i4CfgMin;
        SetStaticInfo(i4CfgMin, i4CfgMax, tCQCKit::EFldTypes::Int);
    }
     else
    {
        MCQCIntfStaticIntf::ReadInStatic(strmToReadFrom);
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid
TCQCIntfStaticProgBar::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_ProgressBar::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfStaticIntf::WriteOutStatic(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}



//
//  Called if user interaction (handled by the base class) changed our value.
//  Any required redrawing is already done, and since we just let the base
//  class current value be the value, we don't need to do anything.
//
tCIDLib::TVoid TCQCIntfStaticProgBar::UserSetValue(const tCIDLib::TFloat8)
{
}

