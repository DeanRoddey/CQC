//
// FILE NAME: CQCIntfEng_VolKnob.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2003
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
//  display a value in the form of an LED on a rotary knob.
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
#include    "CQCIntfEng_VolKnob.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfVolKnobBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFldVolKnob,TCQCIntfVolKnobBase)
AdvRTTIDecls(TCQCIntfStaticVolKnob,TCQCIntfVolKnobBase)
AdvRTTIDecls(TCQCIntfVarVolKnob,TCQCIntfVolKnobBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_VolKnob
    {
        // -----------------------------------------------------------------------
        //  The persistent format version for the base class
        //
        //  Version 2 -
        //      The action context was moved into the events themselves, so we
        //      need to force some contexts after loading older ones.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion    = 2;

        // -----------------------------------------------------------------------
        //  The persistent format version for the field oriented one
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FldFmtVersion     = 1;

        // -----------------------------------------------------------------------
        //  The persistent format version for the static one
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion  = 1;

        // -----------------------------------------------------------------------
        //  The persistent format version for the variable based one
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  We have a fixed LED image, and know it's size, so save effort by just
        //  coding it here. And pre-do a half size that we can just for adjusting
        //  to put the center of the LED on a point.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4LEDSz     = 16;
        constexpr tCIDLib::TInt4    i4HalfLEDSz = 8;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TrackBeyond
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TrackBeyond
        );

        // Some limits and ranges we us internally
        constexpr tCIDLib::TInt4    i4MinRadius         = 16;
        constexpr tCIDLib::TInt4    i4MaxRadius         = 128;
        constexpr tCIDLib::TInt4    i4MinDegreeRange    = 90;
        constexpr tCIDLib::TInt4    i4MaxDegreeRange    = 280;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVKnobRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class.
// ---------------------------------------------------------------------------
class TCQCIntfVKnobRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfVKnobRTV(const  TString&        strValue
                        , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_strValue(strValue)
        {
        }

        ~TCQCIntfVKnobRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue( const   TString&    strId
                                    ,       TString&    strToFill) const final
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            // If it's one of ours, then handle it
            if (strId == kCQCKit::strRTVId_CurValue)
                strToFill = m_strValue;
            else
                return kCIDLib::False;
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
//   CLASS: TCQCIntfVolKnobBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfVolKnobBase::~TCQCIntfVolKnobBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVolKnobBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfVolKnobBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVolKnobBase& iwdgOther(static_cast<const TCQCIntfVolKnobBase&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgIntf::bSameImg(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    // And do our stuff
    return
    (
        (m_bUseLimits == iwdgOther.m_bUseLimits)
        && (m_c4Radius == iwdgOther.m_c4Radius)
        && (m_c4Range == iwdgOther.m_c4Range)
        && (m_fcolLimits == iwdgOther.m_fcolLimits)
    );
}


//
//  We do single finger, non-inertial pan in either axis. Else, we allow it
//  to propagate.
//
tCIDLib::TBoolean
TCQCIntfVolKnobBase::bPerGestOpts(  const   TPoint&             pntAt
                                    , const tCIDLib::EDirs      eDir
                                    , const tCIDLib::TBoolean   bTwoFingers
                                    ,       tCIDCtrls::EGestOpts& eToSet
                                    ,       tCIDLib::TFloat4&   f4VScale)
{
    // If two fingers, let it propogate
    if (bTwoFingers)
        return kCIDLib::True;

    // If in error state do nothing, but no propogation
    if (m_bValError)
        return kCIDLib::False;

    //
    //  We'll allow non-inertial pan in any direction. We remember which axis
    //  for later use during the gesture processing.
    //
    eToSet = tCIDCtrls::EGestOpts::Pan;
    m_bHorizontal = ((eDir == tCIDLib::EDirs::Left) || (eDir == tCIDLib::EDirs::Right));
    f4VScale = 1.0F;

    // No propogation
    return kCIDLib::False;
}



tCIDLib::TBoolean
TCQCIntfVolKnobBase::bProcessGestEv(const   tCIDCtrls::EGestEvs   eEv
                                    , const TPoint&             pntStart
                                    , const TPoint&             pntAt
                                    , const TPoint&             pntDelta
                                    , const tCIDLib::TBoolean   bTwoFinger)
{
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        // Get the new tracking value
        m_f8SaveValue = m_f8Value;
        m_f8Value = f8TrackDrag(pntStart, pntAt);
    }
     else if (eEv == tCIDCtrls::EGestEvs::Drag)
    {
        // Update our value based on the drag position and send on drag
        m_f8Value = f8TrackDrag(pntStart, pntAt);

        PostOps(*this, kCQCKit::strEvId_OnDrag, tCQCKit::EActOrders::Nested);
    }
     else if (eEv == tCIDCtrls::EGestEvs::End)
    {
        try
        {
            // Update the derived class with thetrcking value
            UserSetValue(m_f8Value);

            // Post any 'on set' events
            PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
        }

        catch(TError& errToCatch)
        {
            // Move the LED back to the original position
            MoveLED(m_f8SaveValue, m_f8Value);

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Load up our (currently) fixed position indicator image
    m_cptrPosImg = facCQCIntfEng().cptrGetImage
    (
        m_strPosImage, civOwner(), dsclInit
    );

    // Tell the image list mixin to load up it's images
    TCQCIntfView& civPar = civOwner();
    MCQCIntfImgIntf::InitImage(civPar, dsclInit, colErrs, *this);

    //
    //  Store the user role of the current user that we are running under.
    //  We need this to do user role specific limits.
    //
    m_eCurRole = civPar.cuctxToUse().eUserRole();

    // If in designer mode, force off the default initial error state
    if (TFacCQCIntfEng::bDesMode())
        m_bValError = kCIDLib::False;
}


// Gen up one of our runtime value objects
TCQCCmdRTVSrc*
TCQCIntfVolKnobBase::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    TString strFmt;
    FormatValue(m_f8Value, strFmt);
    return new TCQCIntfVKnobRTV(strFmt, cuctxToUse);
}


tCIDLib::TVoid TCQCIntfVolKnobBase::QueryContentSize(TSize& szToFill)
{
    // Get the size of the bitmap
    szToFill = MCQCIntfImgIntf::szImgDefault();

    // If we have a border, then increase by two
    if (bHasBorder())
        szToFill.Adjust(2);
}


// Return the paths to any images we reference
tCIDLib::TVoid TCQCIntfVolKnobBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image interface mixin
    MCQCIntfImgIntf::QueryImgPath(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
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

    // Set any kept color names that aren't already defaulted correctly
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Do our mixins. Our actions are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Knob Actions"
    );
    MCQCIntfImgIntf::QueryImgAttrs(colAttrs, adatTmp, kCIDLib::True);

    // And do our stuff
    colAttrs.objPlace
    (
        L"Knob Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    TString strLims = kCIDMData::strAttrLim_RangePref;
    strLims.AppendFormatted(CQCIntfEng_VolKnob::i4MinRadius);
    strLims.Append(kCIDLib::chComma);
    strLims.AppendFormatted(CQCIntfEng_VolKnob::i4MaxRadius);
    colAttrs.objPlace
    (
        L"Radius"
        , kCQCIntfEng::strAttr_VKnob_Radius
        , strLims
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4Radius);

    strLims = kCIDMData::strAttrLim_RangePref;
    strLims.AppendFormatted(CQCIntfEng_VolKnob::i4MinDegreeRange);
    strLims.Append(kCIDLib::chComma);
    strLims.AppendFormatted(CQCIntfEng_VolKnob::i4MaxDegreeRange);
    colAttrs.objPlace
    (
        L"Range"
        , kCQCIntfEng::strAttr_VKnob_Range
        , strLims
        , tCIDMData::EAttrTypes::Card
        , tCIDMData::EAttrEdTypes::InPlace
    ).SetCard(m_c4Range);

    //
    //  Only do these for field based widgets since we have no way of knowing what the
    //  limits will be for variable based ones (vars don't exist at design time) and static
    //  ones have built in values so there's no point in setting limits.
    //
    if (clsIsA() == TCQCIntfFldVolKnob::clsThis())
    {
        colAttrs.objPlace
        (
            L"Per User Limits:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
        );

        colAttrs.objPlace
        (
            L"Use Limits"
            , kCQCIntfEng::strAttr_VKnob_UseLimits
            , tCIDMData::EAttrTypes::Bool
        ).SetBool(m_bUseLimits);

        //
        //  Based on the use limits flag, we will enable or disable the limits, which
        //  are done as a binary blob.
        //
        {
            TBinMBufOutStream strmOut(1024UL);
            strmOut << m_fcolLimits << kCIDLib::FlushIt;
            colAttrs.objPlace
            (
                L"Limits"
                , kCQCIntfEng::strAttr_VKnob_Limits
                , strmOut.mbufData()
                , strmOut.c4CurPos()
                , TString::strEmpty()
                , m_bUseLimits ? tCIDMData::EAttrEdTypes::Visual
                               : tCIDMData::EAttrEdTypes::None
            );
        }
    }
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfVolKnobBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image interface mixin
    MCQCIntfImgIntf::UpdateImage(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::Replace(const  tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


TSize TCQCIntfVolKnobBase::szDefaultSize() const
{
    return MCQCIntfImgIntf::szImgDefault();
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVolKnobBase::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Do our mixins
    MCQCIntfImgIntf::SetImgAttr(wndAttrEd, adatNew, adatOld);
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_VKnob_Limits)
    {
        // These are binary, so just stream them in
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_fcolLimits;
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_VKnob_Radius)
    {
        m_c4Radius = adatNew.c4Val();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_VKnob_Range)
    {
        m_c4Range = adatNew.c4Val();
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_VKnob_UseLimits)
    {
        //
        //  Call a helper to do this. The editor will watch for this and enable/disable
        //  the limits attribute as well.
        //
        bUseLimits(adatNew.bVal());
    }
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImage(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVolKnobBase::bUseLimits() const
{
    return m_bUseLimits;
}

tCIDLib::TBoolean TCQCIntfVolKnobBase::bUseLimits(const tCIDLib::TBoolean bToSet)
{
    m_bUseLimits = bToSet;

    // If we disabled them, then reset all user limits
    if (!m_bUseLimits)
    {
        const tCIDLib::TCard4 c4Lims = TRawBits::c4From16
        (
            tCIDLib::TInt2(m_f8Min), tCIDLib::TInt2(m_f8Max)
        );
        tCQCKit::EUserRoles eInd = tCQCKit::EUserRoles::Min;
        for (; eInd < tCQCKit::EUserRoles::Count; eInd++)
            m_fcolLimits[eInd] = c4Lims;
    }
    return m_bUseLimits;
}


tCIDLib::TCard4 TCQCIntfVolKnobBase::c4Radius() const
{
    return m_c4Radius;
}

tCIDLib::TCard4 TCQCIntfVolKnobBase::c4Radius(const tCIDLib::TCard4 c4ToSet)
{
    m_c4Radius = c4ToSet;
    return m_c4Radius;
}


tCIDLib::TCard4 TCQCIntfVolKnobBase::c4Range() const
{
    return m_c4Range;
}

tCIDLib::TCard4 TCQCIntfVolKnobBase::c4Range(const tCIDLib::TCard4 c4ToSet)
{
    // Store the new valuea nd update the per unit coverage
    m_c4Range = c4ToSet;
    m_f8PerUnit = tCIDLib::TFloat8(m_c4Range) / (m_f8Max - m_f8Min);
    return m_c4Range;
}


const tCQCIntfEng::TUserLims& TCQCIntfVolKnobBase::fcolLimits() const
{
    return m_fcolLimits;
}


//
//  Provide access to our min/max values, which are driven by the field,
//  variable, or user settings of the derived class.
//
tCIDLib::TFloat8 TCQCIntfVolKnobBase::f8MaxVal() const
{
    return m_f8Max;
}

tCIDLib::TFloat8 TCQCIntfVolKnobBase::f8MinVal() const
{
    return m_f8Min;
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::QueryLimitsFor(const   tCQCKit::EUserRoles eRole
                                    ,       tCIDLib::TInt4&     i4Min
                                    ,       tCIDLib::TInt4&     i4Max)
{
    CIDAssert
    (
        (eRole >= tCQCKit::EUserRoles::Min) && (eRole <= tCQCKit::EUserRoles::Max)
        , L"Invalid user role value passed"
    );

    const tCIDLib::TCard4 c4Lims = m_fcolLimits[eRole];
    i4Min = TRawBits::i2Low16From32(c4Lims);
    i4Max = TRawBits::i2High16From32(c4Lims);
}


// Allow our config tab to set us up
tCIDLib::TVoid
TCQCIntfVolKnobBase::Set(const  tCIDLib::TBoolean       bUseLimits
                        , const tCIDLib::TCard4         c4Radius
                        , const tCIDLib::TCard4         c4Range
                        , const tCQCIntfEng::TUserLims& fcolLimits)
{
    m_bUseLimits = bUseLimits;
    m_c4Radius = c4Radius;
    m_c4Range = c4Range;
    m_fcolLimits = fcolLimits;
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::SetLimitsFor(  const   tCQCKit::EUserRoles eRole
                                    , const tCIDLib::TInt4      i4NewMin
                                    , const tCIDLib::TInt4      i4NewMax)
{
    CIDAssert
    (
        (eRole >= tCQCKit::EUserRoles::Min) && (eRole <= tCQCKit::EUserRoles::Max)
        , L"Invalid user role value passed"
    );

    // For convenience get our high/low limits as ints
    const tCIDLib::TInt4 i4Min = tCIDLib::TInt4(m_f8Min);
    const tCIDLib::TInt4 i4Max = tCIDLib::TInt4(m_f8Max);

    //
    //  Clip back to our allowable range, and keep a unit between the min
    //  and max.
    //
    tCIDLib::TInt4 i4Low = i4NewMin;
    tCIDLib::TInt4 i4High = i4NewMax;

    // Adjust the low or high if the new one has passed by it
    if (i4High <= i4Low)
    {
        if (i4Low + 1 <= i4Max)
            i4High = i4Low + 1;
        else if (i4High - 1 >= i4Min)
            i4Low = i4High - 1;
    }

    if (i4High > i4Max)
        i4High = i4Max;
    if (i4Low < i4Min)
        i4Low = i4Min;

    tCIDLib::TCard4 c4New = tCIDLib::TCard4(i4High);
    c4New <<= 16;
    c4New |= tCIDLib::TCard2(i4Low);
    m_fcolLimits[eRole] = c4New;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfVolKnobBase::
TCQCIntfVolKnobBase(const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName)
    , MCQCIntfImgIntf(kCIDLib::False)
    , m_bUseLimits(kCIDLib::False)
    , m_bValError(kCIDLib::True)
    , m_c4Radius(32)
    , m_c4Range(tCIDLib::TCard4(CQCIntfEng_VolKnob::i4MaxDegreeRange))
    , m_eCurRole(tCQCKit::EUserRoles::LimitedUser)
    , m_f8PerUnit(CQCIntfEng_VolKnob::i4MaxDegreeRange)
    , m_fcolLimits(tCQCKit::EUserRoles::Count)
    , m_f8Max(kCQCIntfEng::f8MaxRange)
    , m_f8Min(kCQCIntfEng::f8MinRange)
    , m_f8Value(kCQCIntfEng::f8MinRange)
    , m_strPosImage(L"/System/LEDS/Glass/Small/Red")
{
    //
    //  Initialize all the limits values to min/max values we defaulted to.
    //  The max is in the high word and the min in the low.
    //
    const tCIDLib::TCard4 c4InitLim = TRawBits::c4From16
    (
        tCIDLib::TCard2(0), tCIDLib::TCard2(kCQCIntfEng::f8MaxRange)
    );
    m_fcolLimits.AddElements(c4InitLim, tCIDLib::c4EnumOrd(tCQCKit::EUserRoles::Count));

    //
    //  Set our initial background colors, since it's not going to have
    //  any image until a field is selected. This way, they'll be able to
    //  see where it is.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // We provide one widget command value, the current volume value
    AddRTValue
    (
        facCQCKit().strMsg(kKitMsgs::midRT_CurrentValue)
        , kCQCKit::strRTVId_CurValue
        , tCQCKit::ECmdPTypes::Signed
    );

    // Add the events that we support
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
    strCmdHelpId(L"/Reference/IntfWidgets/VolumeKnob");
}

TCQCIntfVolKnobBase::TCQCIntfVolKnobBase(const TCQCIntfVolKnobBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_bValError(iwdgSrc.m_bValError)
    , m_bUseLimits(iwdgSrc.m_bUseLimits)
    , m_c4Radius(iwdgSrc.m_c4Radius)
    , m_c4Range(iwdgSrc.m_c4Range)
    , m_cptrPosImg(iwdgSrc.m_cptrPosImg)
    , m_eCurRole(iwdgSrc.m_eCurRole)
    , m_fcolLimits(iwdgSrc.m_fcolLimits)
    , m_f8PerUnit(iwdgSrc.m_f8PerUnit)
    , m_f8Max(iwdgSrc.m_f8Max)
    , m_f8Min(iwdgSrc.m_f8Min)
    , m_f8Value(iwdgSrc.m_f8Value)
    , m_strPosImage(iwdgSrc.m_strPosImage)
{
}

TCQCIntfVolKnobBase&
TCQCIntfVolKnobBase::operator=(const TCQCIntfVolKnobBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bValError     = iwdgSrc.m_bValError;
        m_bUseLimits    = iwdgSrc.m_bUseLimits;
        m_c4Radius      = iwdgSrc.m_c4Radius;
        m_c4Range       = iwdgSrc.m_c4Range;
        m_cptrPosImg    = iwdgSrc.m_cptrPosImg;
        m_eCurRole      = iwdgSrc.m_eCurRole;
        m_f8PerUnit     = iwdgSrc.m_f8PerUnit;
        m_fcolLimits    = iwdgSrc.m_fcolLimits;
        m_f8Max         = iwdgSrc.m_f8Max;
        m_f8Min         = iwdgSrc.m_f8Min;
        m_f8Value       = iwdgSrc.m_f8Value;
        m_strPosImage   = iwdgSrc.m_strPosImage;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this so that we can indicate a hit if the pointer is within
//  our radius. If it's not within our radius but it's within our overall
//  area, we return transparent, else we fail.
//
tCIDCtrls::EWndAreas TCQCIntfVolKnobBase::eHitTest(const TPoint& pntTest) const
{
    // If in designer mode, call the standard tester on our parent class
    if (TFacCQCIntfEng::bDesMode())
        return TParent::eHitTest(pntTest);

    const TArea& areaUs = areaActual();

    // Translate click point relative to origin
    TPoint pntCenter(areaUs.pntCenter());
    TPoint pntAdjusted(pntTest - pntCenter);

    //
    //  Calculate the polar coordinates of the translated click point
    //  and see if its within a tick length of the circumference of
    //  the knob. If so, then calculate the value that would represent
    //  and update the value.
    //
    tCIDLib::TCard4 c4Degrees;
    tCIDLib::TCard4 c4Radius;
    pntAdjusted.ToPolarDegrees(c4Degrees, c4Radius);

    // If within the radius, then say yes, else no
    if (c4Radius <= m_c4Radius + 10)
        return tCIDCtrls::EWndAreas::Client;

    //
    //  It's not, but if within our area, return transparent if we aren't
    //  in designer mode, else fall through to the regular hit test
    //  provided by our parent.
    //
    if (areaUs.bContainsPoint(pntTest))
        return tCIDCtrls::EWndAreas::Transparent;

    // We failed
    return tCIDCtrls::EWndAreas::Nowhere;
}



// Stream ourself in and out
tCIDLib::TVoid TCQCIntfVolKnobBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VolKnob::c2BaseFmtVersion))
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
    MCQCIntfImgIntf::ReadInImage(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // Pull in our data fields
    strmToReadFrom  >>  m_bUseLimits
                    >>  m_c4Radius
                    >>  m_c4Range
                    >>  m_fcolLimits;

    // We had an issue where the limits list wasn't being filled in, so do it now if so
    if (m_fcolLimits.bIsEmpty())
    {
        const tCIDLib::TCard4 c4InitLim = TRawBits::c4From16
        (
            tCIDLib::TCard2(0), tCIDLib::TCard2(kCQCIntfEng::f8MaxRange)
        );
        m_fcolLimits.AddElements(c4InitLim, tCIDLib::c4EnumOrd(tCQCKit::EUserRoles::Count));
    }

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // If V1, then force the action context on some events
    if (c2FmtVersion == 1)
    {
        SetEventContext
        (
            kCQCKit::strEvId_OnPress, tCQCKit::EActCmdCtx::NoContChange
        );
        SetEventContext
        (
            kCQCKit::strEvId_OnRelease, tCQCKit::EActCmdCtx::NoContChange
        );
        SetEventContext
        (
            kCQCKit::strEvId_OnDrag, tCQCKit::EActCmdCtx::NoContChange
        );
        SetEventContext
        (
            kCQCKit::strEvId_OnSet, tCQCKit::EActCmdCtx::NoContChange
        );
    }

    // Default the current user role
    m_eCurRole = tCQCKit::EUserRoles::LimitedUser;
}

tCIDLib::TVoid TCQCIntfVolKnobBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VolKnob::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfImgIntf::WriteOutImage(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << m_bUseLimits
                    << m_c4Radius
                    << m_c4Range
                    << m_fcolLimits
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfVolKnobBase::Update(        TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    // Get the remaining area to draw in
    TArea areaDraw = grgnClip.areaBounds();

    // Draw the background image if we have one
    if (bUseImage())
    {
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

    //
    //  Draw the LED if we have a good value. If tracking using the tracking
    //  value, else use the current value, and calculate the bounding box
    //  for the LED.
    //
    if (!m_bValError)
    {
        TArea areaLED
        (
            areaCalcLEDPos(m_f8Value, m_f8Min, m_f8Max)
        );

        // And draw it there if it intersects the the update area
        if (areaLED.bIntersects(areaInvalid))
        {
            TArea areaSrc(TPoint::pntOrigin, areaLED.szArea());
            m_cptrPosImg->bmpImage().AdvDrawOn
            (
                gdevTarget
                , areaSrc
                , areaLED
                , kCIDLib::True
                , m_cptrPosImg->c4TransClr()
                , m_cptrPosImg->bmpMask()
                , tCIDGraphDev::EBltTypes::None
                , 0xAA
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by derived classes to tell us what volume level to display. The
//  return is true if we have a good value and the new value is different.
//
tCIDLib::TBoolean
TCQCIntfVolKnobBase::bSetValue( const   tCIDLib::TFloat8    f8ToSet
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
    //  Now, if its changed from the current value, move the LED appropriately.
    //  If we are tracking currently, then just store the value but don't
    //  update the LED because the user is dragging it.
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


// Let's the derived class know if a value is out of the set range limits
tCIDLib::TBoolean
TCQCIntfVolKnobBase::bValueOutOfRange(const tCIDLib::TFloat8 f8Check) const
{
    if ((f8Check < m_f8Min) || (f8Check > m_f8Max))
        return kCIDLib::True;
    return kCIDLib::False;
}


//
//  This is provided to allow the field based derivative to let us know when
//  it's been assigned a new field, so that we can update ourself to match
//  the limits of that field. Only used during design time.
//
tCIDLib::TVoid
TCQCIntfVolKnobBase::SetValueAttrs( const   tCIDLib::TBoolean   bWriteable
                                    , const tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCIDLib::TBoolean   bInitToMin)
{
    //
    //  If we are using limits go through and check them. If any are out of
    //  range, revert them all back to the full min/max. If not using limits
    //  also make sure they are set to the full range.
    //
    tCIDLib::TBoolean bResetLimits(!m_bUseLimits);

    // If we could possibly still be ok, check them
    if (!bResetLimits)
    {
        const tCIDLib::TInt2 i2NewMin = tCIDLib::TInt2(f8Min);
        const tCIDLib::TInt2 i2NewMax = tCIDLib::TInt2(f8Max);

        tCQCKit::EUserRoles eInd = tCQCKit::EUserRoles::Min;
        for (; eInd < tCQCKit::EUserRoles::Count; eInd++)
        {
            tCIDLib::TCard4 c4Lims = m_fcolLimits[eInd];

            tCIDLib::TInt2 i2UserMin = TRawBits::i2Low16From32(c4Lims);
            tCIDLib::TInt2 i2UserMax = TRawBits::i2High16From32(c4Lims);

            if ((i2UserMin < i2NewMin) || (i2UserMin > i2NewMax)
            ||  (i2UserMax > i2NewMax) || (i2UserMax < i2NewMin))
            {
                bResetLimits = kCIDLib::True;
                break;
            }
        }
    }

    if (bResetLimits)
    {
        const tCIDLib::TCard4 c4Lims = TRawBits::c4From16
        (
            tCIDLib::TInt2(f8Min), tCIDLib::TInt2(f8Max)
        );
        tCQCKit::EUserRoles eInd = tCQCKit::EUserRoles::Min;
        for (; eInd < tCQCKit::EUserRoles::Count; eInd++)
            m_fcolLimits[eInd] = c4Lims;
    }

    // If the min or max has changed, then reset our stuff
    if ((f8Max != m_f8Max) || (f8Min != m_f8Min))
    {
        if (m_f8Value > f8Max)
            m_f8Value = f8Max;
        else if (m_f8Value < f8Min)
            m_f8Value = f8Min;

        // Store our min/max and calc the new per-unit degrees
        m_f8Max = f8Max;
        m_f8Min = f8Min;
        m_f8PerUnit = tCIDLib::TFloat8(m_c4Range) / (m_f8Max - m_f8Min);
    }

    //
    //  If asked or in designer mode, set the value to the min value, and
    //  take us out of error state if so since obviously we have a good
    //  value.
    //
    if (bInitToMin || TFacCQCIntfEng::bDesMode())
    {
        m_f8Value = m_f8Min;
        m_bValError = kCIDLib::False;
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfVolKnobBase: Private, non-virtual methods
// ---------------------------------------------------------------------------
TArea
TCQCIntfVolKnobBase::areaCalcLEDPos(const   tCIDLib::TFloat8    f8Value
                                    , const tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max)
{
    // Get the center point of our current area
    TArea areaRet(areaActual());
    TPoint pntCenter(areaRet.pntCenter());

    // Now set the return area size to the size of the LED
    areaRet.SetSize
    (
        CQCIntfEng_VolKnob::c4LEDSz, CQCIntfEng_VolKnob::c4LEDSz
    );

    //
    //  Now use our current radius to calculate the center point of the LED
    //  position. If the radius is zero, then we just have to put the LED
    //  on the center point.
    //
    if (!m_c4Radius)
    {
        areaRet.SetOrg(pntCenter);
        areaRet.AdjustOrg
        (
            -CQCIntfEng_VolKnob::i4HalfLEDSz, -CQCIntfEng_VolKnob::i4HalfLEDSz
        );
        return areaRet;
    }

    //
    //  Calc the number of degrees our current value represents. Clip the
    //  value to the range, just in case...
    //
    tCIDLib::TFloat8 f8Actual = f8Value;
    if (f8Value < f8Min)
        f8Actual = f8Min;
    else if (f8Value > f8Max)
        f8Actual = f8Max;
    const tCIDLib::TCard4 c4ValDegrees = tCIDLib::TCard4
    (
        (f8Actual - f8Min) * m_f8PerUnit
    );

    // Calculate the starting degree
    const tCIDLib::TCard4 c4MinDegree = 270 - (m_c4Range / 2);

    // And calulate the outer point of the value tick
    TPoint pntLEDOrg;
    pntLEDOrg.FromPolarDegrees((c4MinDegree + c4ValDegrees) % 360, m_c4Radius - 6);

    //
    //  Now adjust by the center point and back by half the LED size, to
    //  keep the LED center point on the radius.
    //
    pntLEDOrg += pntCenter;
    pntLEDOrg.Adjust
    (
        -CQCIntfEng_VolKnob::i4HalfLEDSz
        , -CQCIntfEng_VolKnob::i4HalfLEDSz
    );

    // And make that the center of the LED, and we have the answer
    areaRet.SetOrg(pntLEDOrg);
    return areaRet;
}


//
//  This is called as the user does a drag gesture on us. We adjust the
//  value up or down as the user moves left/right or up/down. We calculate
//  how much to adjust based on how far we've moved from the starting point.
//
//  We divide the whole range by 300 pixels, and use that as the adjustment
//  value.
//
tCIDLib::TFloat8
TCQCIntfVolKnobBase::f8TrackDrag(const TPoint& pntStart, const TPoint& pntNew)
{
    const tCIDLib::TFloat8 f8Per((m_f8Max - m_f8Min) / 300);

    //
    //  Calculate the new degree value for this position.
    //
    tCIDLib::TFloat8 f8Change;
    if (m_bHorizontal)
        f8Change = tCIDLib::TFloat8(pntNew.i4X() - pntStart.i4X()) * f8Per;
    else
        f8Change = tCIDLib::TFloat8(pntNew.i4Y() - pntStart.i4Y()) * f8Per;

    // Calculate the min and max degree positions
    const tCIDLib::TCard4 c4MinDegree = 270 - (m_c4Range / 2);
    const tCIDLib::TCard4 c4MaxDegree = (270 + (m_c4Range / 2)) % 360;

    // Create the new value
    tCIDLib::TFloat8 f8New = m_f8SaveValue + f8Change;
    if (f8New < m_f8Min)
        f8New = m_f8Min;
    else if (f8New > m_f8Max)
        f8New = m_f8Max;

    //
    //  If we are doing per-user role limits, then clip this value to the
    //  per-user role, else take it as is.
    //
    if (m_bUseLimits)
    {
        const tCIDLib::TCard4 c4Lims = m_fcolLimits[m_eCurRole];
        const tCIDLib::TFloat8 f8Low = tCIDLib::TFloat8(TRawBits::i2Low16From32(c4Lims));
        const tCIDLib::TFloat8 f8High = tCIDLib::TFloat8(TRawBits::i2High16From32(c4Lims));

        if (f8New < f8Low)
            f8New = f8Low;
        else if (f8New > f8High)
            f8New = f8High;
    }

    //
    //  If this is different from the last tracking value, then remove the
    //  LED at the current tracking point and draw it at the new one.
    //
    if (f8New != m_f8Value)
    {
        tCIDLib::TFloat8 f8Old = m_f8Value;
        m_f8Value = f8New;
        MoveLED(f8Old, f8New);
    }
    return f8New;
}


//
//  Moves the LED from an old position to a new position. Either could
//  potentially be outside our widget area.
//
tCIDLib::TVoid
TCQCIntfVolKnobBase::MoveLED(const  tCIDLib::TFloat8    f8OldVal
                            , const tCIDLib::TFloat8    f8NewVal)
{
    // If hidden, then do nothing
    if (eCurDisplayState() == tCQCIntfEng::EDispStates::Hidden)
        return;

    // If the values are the same, then nothing to do
    if (f8OldVal == f8NewVal)
        return;

    // Calculate the bounding box for the LED at the two locations
    TArea areaOld(areaCalcLEDPos(f8OldVal, m_f8Min, m_f8Max));
    TArea areaNew(areaCalcLEDPos(f8NewVal, m_f8Min, m_f8Max));

    if (areaOld.bIntersects(areaNew))
    {
        //
        //  The areas overlap, so ask it to just redraw everthing under
        //  the combined area.
        //
        areaNew |= areaOld;
        Redraw(areaNew);
    }
     else
    {
        //
        //  They don't overlap, so just do a double buffered redraw of the
        //  whole volume knob.
        //
        Redraw();
    }
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFldVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFldVolKnob: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFldVolKnob::TCQCIntfFldVolKnob() :

    TCQCIntfVolKnobBase
    (
        CQCIntfEng_VolKnob::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_VolKnob)
    )
    , MCQCIntfSingleFldIntf
      (
          new TCQCFldFiltNumRange
          (
              tCQCKit::EReqAccess::MReadCWrite
              , kCQCIntfEng::f8MinRange
              , kCQCIntfEng::f8MaxRange
              , kCIDLib::True
          )
      )
{
}

TCQCIntfFldVolKnob::TCQCIntfFldVolKnob(const TCQCIntfFldVolKnob& iwdgToCopy) :

    TCQCIntfVolKnobBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFldVolKnob::~TCQCIntfFldVolKnob()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVolKnob: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFldVolKnob&
TCQCIntfFldVolKnob::operator=(const TCQCIntfFldVolKnob& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVolKnob: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to let us verify that we can be associated with the passed
//  field. It can optionally tell us to set ourselves up for this new field
//  if it is acceptable, in our case storing the range info on our parent.
//
tCIDLib::TBoolean
TCQCIntfFldVolKnob::bCanAcceptField(const   TCQCFldDef&         flddTest
                                    , const TString&
                                    , const TString&
                                    , const tCIDLib::TBoolean   bStore)
{
    tCIDLib::TFloat8 f8Min, f8Max;
    const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
    (
        flddTest
        , kCQCIntfEng::f8MinRange
        , kCQCIntfEng::f8MaxRange
        , kCIDLib::True
        , f8Min
        , f8Max
    );

    // If it's good and we are storing, set up our parent class with this range info
    if (bRes && bStore)
        SetValueAttrs(flddTest.bIsWriteable(), f8Min, f8Max, kCIDLib::False);
    return bRes;
}


tCIDLib::TBoolean TCQCIntfFldVolKnob::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFldVolKnob& iwdgOther(static_cast<const TCQCIntfFldVolKnob&>(iwdgSrc));

    // We just add the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfFldVolKnob::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFldVolKnob&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFldVolKnob::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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


// Initialize ourself
tCIDLib::TVoid
TCQCIntfFldVolKnob::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfFldVolKnob::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Volume Field");
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFldVolKnob::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFldVolKnob::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFldVolKnob::RegisterFields(         TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFldVolKnob::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFldVolKnob::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // All we add is the field mixin
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    //
    //  If the field changes, we have to update the limits attribute. We include the min
    //  max values as well, for the visual editor.
    //
    if (adatNew.strId() == kCQCIntfEng::strAttr_Field)
    {
        TBinMBufOutStream strmOut(512UL);
        strmOut << fcolLimits() << kCIDLib::FlushIt;
        wndAttrEd.UpdateMemBuf
        (
            kCQCIntfEng::strAttr_VKnob_Limits, strmOut.c4CurPos(), strmOut.mbufData()
        );
    }
}


tCIDLib::TVoid
TCQCIntfFldVolKnob::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our fields mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfFldVolKnob: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFldVolKnob::FieldChanged(       TCQCFldPollInfo&    cfpiAssoc
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


tCIDLib::TVoid
TCQCIntfFldVolKnob::FormatValue(const   tCIDLib::TFloat8    f8ToFmt
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


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfFldVolKnob::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VolKnob::c2FldFmtVersion))
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

tCIDLib::TVoid TCQCIntfFldVolKnob::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VolKnob::c2FldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The base class calls this when the user drops the knowb inside the
//  radius and so a new value needs to be set. We take the value and
//  write it to our associated field. We let any exceptions propogate back
//  to the base class.
//
//  Note that if the no auto-write flag is set in the field interface,
//  we don't do anything.
//
tCIDLib::TVoid
TCQCIntfFldVolKnob::UserSetValue(const tCIDLib::TFloat8 f8NewValue)
{
    if (!bNoAutoWrite())
    {
        const tCQCKit::EFldTypes eType = eFieldType();

        TString strValue;
        if (eType == tCQCKit::EFldTypes::Card)
            strValue.AppendFormatted(tCIDLib::TCard4(f8NewValue));
        else if (eType == tCQCKit::EFldTypes::Float)
            strValue.AppendFormatted(f8NewValue);
        else if (eType == tCQCKit::EFldTypes::Int)
            strValue.AppendFormatted(tCIDLib::TInt4(f8NewValue));

        WriteValue(strMoniker(), strFieldName(), strValue);
    }
}


//
//  This is called periodically to give us a chance to update. We just pass
//  it to our field mixin. If the value has changed, he will call us back on
//  our FieldChange() method.
//
tCIDLib::TVoid
TCQCIntfFldVolKnob::ValueUpdate(        TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticVolKnob: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticVolKnob::TCQCIntfStaticVolKnob() :

    TCQCIntfVolKnobBase
    (
        CQCIntfEng_VolKnob::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_VolKnob)
    )
    , MCQCIntfStaticIntf(tCQCKit::EFldTypes::Int)
{
}

TCQCIntfStaticVolKnob::TCQCIntfStaticVolKnob(const TCQCIntfStaticVolKnob& iwdgToCopy) :

    TCQCIntfVolKnobBase(iwdgToCopy)
    , MCQCIntfStaticIntf(iwdgToCopy)
{
}

TCQCIntfStaticVolKnob::~TCQCIntfStaticVolKnob()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticVolKnob: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticVolKnob&
TCQCIntfStaticVolKnob::operator=(const TCQCIntfStaticVolKnob& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfStaticIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticVolKnob: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticVolKnob::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticVolKnob& iwdgOther(static_cast<const TCQCIntfStaticVolKnob&>(iwdgSrc));

    // We just add the static mixin
    return MCQCIntfStaticIntf::bSameStatic(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfStaticVolKnob::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticVolKnob&>(iwdgSrc);
}



// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfStaticVolKnob::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
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

        if (bSetValue(f8Val, !bCanRedraw(strEventId), kCIDLib::False))
        {
            //
            //  A new value was stored, so generate an OnSet command if not.
            //  This is done non-nested, so it won't run until we get out of
            //  whatever we are doing now. So it's safe to let it happen in
            //  the preload. The OnSet won't run till we are done.
            //
            PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
        }
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


// Called to initialize us after loading
tCIDLib::TVoid
TCQCIntfStaticVolKnob::Initialize(  TCQCIntfContainer* const    piwdgParent
                                    , TDataSrvClient&           dsclInit
                                    , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Initialize our parent to our data type and range settings, which
    //  we get from the static mixin. Tell it to set the initial value to
    //  the range minimum, which will get it out of the initial assumed value
    //  error state, and it'll stay that way from now on since we never update
    //  him again.
    //
    SetValueAttrs(kCIDLib::True, f8RangeMin(), f8RangeMax(), kCIDLib::True);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfStaticVolKnob::QueryCmds(       TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // And add our commands
    if (eContext != tCQCKit::EActCmdCtx::IVEvent)
    {
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
}


tCIDLib::TVoid
TCQCIntfStaticVolKnob::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    MCQCIntfStaticIntf::QueryStaticAttrs(colAttrs, adatTmp);
}


//
//  We override this so that, at design time, we get ourself updated as soon
//  as the designer stores new user entered range and type info. This is needed
//  so that the data is available to other tabs, the user limits stuff in
//  particular. We call the mixin first to store the data, then update our
//  parent class.
//
tCIDLib::TVoid
TCQCIntfStaticVolKnob::SetStaticInfo(const  tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCQCKit::EFldTypes  eType)
{
    MCQCIntfStaticIntf::SetStaticInfo(f8Min, f8Max, eType);

    //
    //  Use the values as stored, since it's possible he tweaked them during
    //  storage.
    //
    SetValueAttrs(kCIDLib::True, f8RangeMin(), f8RangeMax(), kCIDLib::False);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticVolKnob::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // All we add is the static mixin
    MCQCIntfStaticIntf::SetStaticAttr(wndAttrEd, adatNew, adatOld);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticVolKnob: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfStaticVolKnob::FormatValue( const   tCIDLib::TFloat8    f8ToFmt
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


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfStaticVolKnob::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VolKnob::c2StaticFmtVersion))
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
    MCQCIntfStaticIntf::ReadInStatic(strmToReadFrom);

    // We currently don't have any

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfStaticVolKnob::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VolKnob::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfStaticIntf::WriteOutStatic(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The user drug to a new value in our parent class. This is a no-op for
//  us since the actual value inside the parent class is the real value
//  in our case.
//
tCIDLib::TVoid TCQCIntfStaticVolKnob::UserSetValue(const tCIDLib::TFloat8)
{
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarVolKnob
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarVolKnob: Constructors and Destructor
// ---------------------------------------------------------------------------

// We use the same cap flags as the field based version
TCQCIntfVarVolKnob::TCQCIntfVarVolKnob() :

    TCQCIntfVolKnobBase
    (
        CQCIntfEng_VolKnob::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_VolKnob)
    )
{
}

TCQCIntfVarVolKnob::TCQCIntfVarVolKnob(const TCQCIntfVarVolKnob& iwdgToCopy) :

    TCQCIntfVolKnobBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarVolKnob::~TCQCIntfVarVolKnob()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarVolKnob: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarVolKnob&
TCQCIntfVarVolKnob::operator=(const TCQCIntfVarVolKnob& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarVolKnob: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarVolKnob::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarVolKnob& iwdgOther(static_cast<const TCQCIntfVarVolKnob&>(iwdgSrc));

    // We just add the variable mixin
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


tCIDLib::TVoid
TCQCIntfVarVolKnob::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarVolKnob&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarVolKnob::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfVarVolKnob::QueryCmds(          TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarVolKnob::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarVolKnob::Replace(const   tCQCIntfEng::ERepFlags  eToRep
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


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarVolKnob::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarVolKnob::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfVarVolKnob: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Our variable mixin calls us here when we acquire the associated variable,
//  to make sure it has the correct attributes for our use. We can only use
//  variables of numeric type with range limits that are within a specific
//  min/max range. There's a helper method in the kit facility that will
//  do this check for us. If it's good, we need to set up our parent class
//  with this range info.
//
tCIDLib::TBoolean TCQCIntfVarVolKnob::bCanAcceptVar(const TCQCActVar& varToTest)
{
    tCIDLib::TFloat8 f8RealMin;
    tCIDLib::TFloat8 f8RealMax;

    const tCIDLib::TBoolean bRes = facCQCKit().bCheckNumRangeFld
    (
        varToTest.strName()
        , varToTest.eType()
        , varToTest.strLimits()
        , kCQCIntfEng::f8MinRange
        , kCQCIntfEng::f8MaxRange
        , kCIDLib::True
        , f8RealMin
        , f8RealMax
    );

    if (bRes)
        SetValueAttrs(kCIDLib::True, f8RealMin, f8RealMax, kCIDLib::False);
    return bRes;
}


tCIDLib::TVoid
TCQCIntfVarVolKnob::FormatValue(const   tCIDLib::TFloat8    f8ToFmt
                                ,       TString&            strToFill) const
{
    const tCQCKit::EFldTypes eType = eVarType();
    if (eType == tCQCKit::EFldTypes::Card)
        strToFill.AppendFormatted(tCIDLib::TCard4(f8ToFmt));
    else if (eType == tCQCKit::EFldTypes::Float)
        strToFill.AppendFormatted(f8ToFmt);
    else if (eType == tCQCKit::EFldTypes::Int)
        strToFill.AppendFormatted(tCIDLib::TInt4(f8ToFmt));
}


// Stream ourself in and out
tCIDLib::TVoid TCQCIntfVarVolKnob::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_VolKnob::c2VarFmtVersion))
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

    // We currently don't have any

    // And it should all end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}

tCIDLib::TVoid TCQCIntfVarVolKnob::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream out our marker and format version
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_VolKnob::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // Stream our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The user drug to a new value in our parent class. We need to update the
//  variable to this new value.
//
tCIDLib::TVoid
TCQCIntfVarVolKnob::UserSetValue(const tCIDLib::TFloat8 f8NewVal)
{
    const tCQCKit::EFldTypes eType = eVarType();

    TString strValue;
    if (eType == tCQCKit::EFldTypes::Card)
        strValue.AppendFormatted(tCIDLib::TCard4(f8NewVal));
    else if (eType == tCQCKit::EFldTypes::Float)
        strValue.AppendFormatted(f8NewVal);
    else if (eType == tCQCKit::EFldTypes::Int)
        strValue.AppendFormatted(tCIDLib::TInt4(f8NewVal));

    civOwner().SetVariable(strVarName(), strValue);
}


// We just pass this on to our variable mixin
tCIDLib::TVoid
TCQCIntfVarVolKnob::ValueUpdate(        TCQCPollEngine&
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
TCQCIntfVarVolKnob::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    bSetValue(0, bNoRedraw, kCIDLib::True);
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarVolKnob::VarValChanged(  const   TCQCActVar&         varNew
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


