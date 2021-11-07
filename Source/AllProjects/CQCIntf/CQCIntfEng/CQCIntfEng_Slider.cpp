//
// FILE NAME: CQCIntfEng_Slider.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2006
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
//  This file implements the slider base class and some derivatives of it.
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
#include    "CQCIntfEng_Slider.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfSliderBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfFieldSlider,TCQCIntfSliderBase)
AdvRTTIDecls(TCQCIntfStaticSlider,TCQCIntfSliderBase)
AdvRTTIDecls(TCQCIntfVarSlider,TCQCIntfSliderBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_Slider
    {
        // -----------------------------------------------------------------------
        //  The base class persistent format version
        //
        //  Version 2 -
        //      Action contexts were moved to the events themselves. So for older
        //      versions we have to force the context on a couple of them after
        //      we load.
        //
        //  Version 3 -
        //      Added the same sort of support for per-user role limits on
        //      accessible range that the volume knob has always supported.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 3;


        // -----------------------------------------------------------------------
        //  The field slider class persistent format version
        //
        //  Version 2 -
        //      The field info was moved to the new single field mixin interface,
        //      so we have to stream that in now.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FieldFmtVersion   = 2;


        // -----------------------------------------------------------------------
        //  The static class persistent format version
        //
        /// Version 2 -
        //      Version info was moved into a new static mixin intrface, so we
        //      have to stream differently now.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2StaticFmtVersion  = 2;


        // -----------------------------------------------------------------------
        //  The variable class persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2VarFmtVersion     = 1;


        // -----------------------------------------------------------------------
        //  Our capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eFldCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TakesFocus
            , tCQCIntfEng::ECapFlags::ValueUpdate
        );

        constexpr tCQCIntfEng::ECapFlags eStaticCapFlags = tCIDLib::eOREnumBits
        (
            tCQCIntfEng::ECapFlags::DoesGestures
            , tCQCIntfEng::ECapFlags::OptionalAct
            , tCQCIntfEng::ECapFlags::TakesFocus
        );


        // -------------------------------------------------------------------
        //  These are the keys we use on the image list interface for our
        //  two images.
        // -------------------------------------------------------------------
        const TString  strThumbKey(L"Thumb");
        const TString  strPressedKey(L"PressThumb");
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfSliderRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class to provide some extra
//  slider specific RTVs.
// ---------------------------------------------------------------------------
class TCQCIntfSliderRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfSliderRTV(  const   TString&        strValue
                            , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_strValue(strValue)
        {
        }

        ~TCQCIntfSliderRTV() {}


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
//   CLASS: TCQCIntfSliderBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfSliderBase::~TCQCIntfSliderBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfSliderBase::bCanSizeTo() const
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfSliderBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfSliderBase& iwdgOther(static_cast<const TCQCIntfSliderBase&>(iwdgSrc));

    // Do our mixins
    if (!MCQCIntfImgListIntf::bSameImgList(iwdgOther)
    ||  !MCQCCmdSrcIntf::bSameCmds(iwdgOther))
    {
        return kCIDLib::False;
    }

    return
    (
        (m_bUseLimits == iwdgOther.m_bUseLimits)
        && (m_fcolLimits == iwdgOther.m_fcolLimits)
    );
}


// We allow non-inertial pans in our configured orientation direction
tCIDLib::TBoolean
TCQCIntfSliderBase::bPerGestOpts(const  TPoint&             pntAt
                                , const tCIDLib::EDirs      eDir
                                , const tCIDLib::TBoolean   bTwoFingers
                                ,       tCIDCtrls::EGestOpts& eToSet
                                ,       tCIDLib::TFloat4&   f4VScale)
{
    // If not writeable or two fingers, let it propogate
    if (!m_bWriteable || bTwoFingers)
        return kCIDLib::True;

    // If in error state do nothing, but no propogation
    if (m_bValError)
        return kCIDLib::False;

    const TArea& areaUs = areaActual();

    // Let it propogate if in the other axis
    if (areaUs.c4Width() >= areaUs.c4Height())
    {
        if ((eDir != tCIDLib::EDirs::Left) && (eDir != tCIDLib::EDirs::Right))
            return kCIDLib::True;
    }
     else
    {
        if ((eDir != tCIDLib::EDirs::Up) && (eDir != tCIDLib::EDirs::Down))
            return kCIDLib::True;
    }

    // Looks ok, so allow non-inertial pan
    eToSet = tCIDCtrls::EGestOpts::Pan;
    f4VScale = 1.0F;

    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfSliderBase::bProcessGestEv( const   tCIDCtrls::EGestEvs   eEv
                                    , const TPoint&             pntStart
                                    , const TPoint&             pntAt
                                    , const TPoint&             pntDelta
                                    , const tCIDLib::TBoolean   bTwoFinger)
{
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        // Get an initial tracking value
        m_f8Value = f8ValFromPos(pntAt, kCIDLib::True);

        // Post any 'on press' events
        PostOps(*this, kCQCKit::strEvId_OnPress, tCQCKit::EActOrders::NonNested);
    }
     else if (eEv == tCIDCtrls::EGestEvs::Drag)
    {
        //
        //  Calculate the value that this point would represent within our
        //  field's range.
        //
        const tCIDLib::TFloat8 f8OldVal = m_f8Value;
        m_f8Value = f8ValFromPos(pntAt, kCIDLib::True);
        if (f8OldVal != m_f8Value)
        {
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
        const tCIDLib::TFloat8 f8OldVal = m_f8Value;
        m_f8Value = f8ValFromPos(pntAt, kCIDLib::True);

        UserSetValue(m_f8Value);
        Redraw();

        // Post any on set commands
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
    }

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfSliderBase::Clicked(const TPoint& pntAt)
{
    m_f8Value = f8ValFromPos(pntAt, kCIDLib::False);
    UserSetValue(m_f8Value);
    Redraw();

    // Post any on set commands
    PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfSliderBase::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetValue)
    {
        // We just return the currently set value
        TString strVal;
        FormatValue(m_f8Value, strVal);

        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varText = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        if (varText.bSetValue(strVal) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varText.strName(), varText.strValue());
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
TCQCIntfSliderBase::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Get our owning interface view
    TCQCIntfView& civPar = civOwner();

    // Tell the image list mixin to load up its images
    MCQCIntfImgListIntf::InitImgList(civPar, dsclInit, colErrs, *this);

    //
    //  Store the user role of the current user that we are running under.
    //  We need this to do user role specific limits.
    //
    m_eCurRole = civPar.cuctxToUse().eUserRole();

    // If in designer mode, force off the default initial error state
    if (TFacCQCIntfEng::bDesMode())
        m_bValError = kCIDLib::False;
}


//
//  Gen up one of our runtime value objects. The current value we provide
//  is different according to whether we are tracking or not.
//
TCQCCmdRTVSrc*
TCQCIntfSliderBase::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    // Ask the derived class to format it out for us
    TString strTmp;
    FormatValue(m_f8Value, strTmp);
    return new TCQCIntfSliderRTV(strTmp, cuctxToUse);
}


tCIDLib::TVoid
TCQCIntfSliderBase::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);

    // We just support the changing of the field
    {
        colCmds.objAdd
        (
            TCQCCmd
            (
                kCQCKit::strCmdId_GetValue
                , facCQCKit().strMsg(kKitMsgs::midCmd_GetValue)
                , tCQCKit::ECmdPTypes::VarName
                , facCQCKit().strMsg(kKitMsgs::midCmdP_Value)
            )
        );
    }
}


//
//  For us, this can be a bit complex. We want to basically calculate thje
//  bounds of any images (which can be offset).
//
//  So ask the image list mixin to do give us the bounds of any images,
//  including offsets. It returns an origin centered area, which is what
//  we want.
//
tCIDLib::TVoid TCQCIntfSliderBase::QueryContentSize(TSize& szToFill)
{
    TArea areaBounds = areaImgBounds();

    // If none, then provide a little default size, if so, expand it out a bit
    if (areaBounds.bIsEmpty() || (areaBounds.szArea() == TSize(1, 1)))
        areaBounds.Set(-8, -8, 16, 16);
    else
        areaBounds.Inflate(8, 8);

    szToFill = areaBounds.szArea();
}


tCIDLib::TVoid TCQCIntfSliderBase::
QueryReferencedImgs(        tCIDLib::TStrHashSet&   colToFill
                    ,       tCIDLib::TStrHashSet&
                    , const tCIDLib::TBoolean       bIncludeSysImgs
                    , const tCIDLib::TBoolean       ) const
{
    // Just delegate to our image list interface
    MCQCIntfImgListIntf::QueryImgPaths(colToFill, bIncludeSysImgs);
}


tCIDLib::TVoid
TCQCIntfSliderBase::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
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

    // Do our mixins. Our commands are not required
    MCQCCmdSrcIntf::QueryCmdAttrs
    (
        colAttrs, adatTmp, kCIDLib::True, kCIDLib::False, L"Actions"
    );
    MCQCIntfImgListIntf::QueryImgListAttrs(colAttrs, adatTmp);
}


//
//  Called by the designer when the user modifies the image repository, to
//  get us to update our images from the cache.
//
tCIDLib::TVoid
TCQCIntfSliderBase::RefreshImages(TDataSrvClient& dsclToUse)
{
    // Just delegate to our image list interface mixin
    MCQCIntfImgListIntf::UpdateAllImgs(civOwner(), dsclToUse);
}


tCIDLib::TVoid
TCQCIntfSliderBase::Replace(const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::Image))
        MCQCIntfImgListIntf::ReplaceImage(strSrc, strTar, bRegEx, bFull, regxFind);

    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::ActionParms))
        CmdSearchNReplace(strSrc, strTar, bRegEx, bFull, regxFind);
}


// We can do this on behalf of all our derivatives
TSize TCQCIntfSliderBase::szDefaultSize() const
{
    return TSize(32, 64);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfSliderBase::SetWdgAttr(         TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    MCQCIntfImgListIntf::SetImgListAttr(civOwner(), wndAttrEd, adatNew, adatOld);
    MCQCCmdSrcIntf::SetCmdAttr(adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Slider_UseLimits)
    {
        //
        //  Call a helper to do this. The editor will watch for this and enable/disable
        //  the limits attribute.
        //
        bUseLimits(adatNew.bVal());
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Slider_Limits)
    {
        // These are binary, so just stream them in
        TBinMBufInStream strmSrc(&adatNew.mbufVal(), adatNew.c4Bytes());
        strmSrc >> m_fcolLimits;
    }
}


tCIDLib::TVoid
TCQCIntfSliderBase::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our image mixin
    ValidateImgList(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfSliderBase::bUseLimits() const
{
    return m_bUseLimits;
}

tCIDLib::TBoolean TCQCIntfSliderBase::bUseLimits(const tCIDLib::TBoolean bToSet)
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



//
//  Provide access to our min/max values, which are driven by the field,
//  variable, or user settings of the derived class.
//
tCIDLib::TFloat8 TCQCIntfSliderBase::f8MaxVal() const
{
    return m_f8Max;
}

tCIDLib::TFloat8 TCQCIntfSliderBase::f8MinVal() const
{
    return m_f8Min;
}


const tCQCIntfEng::TUserLims& TCQCIntfSliderBase::fcolLimits() const
{
    return m_fcolLimits;
}


tCIDLib::TVoid
TCQCIntfSliderBase::QueryLimitsFor( const   tCQCKit::EUserRoles eRole
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


tCIDLib::TVoid
TCQCIntfSliderBase::Set(const   tCIDLib::TBoolean       bUseLimits
                        , const tCQCIntfEng::TUserLims& fcolLimits)
{
    m_bUseLimits = bUseLimits;
    m_fcolLimits = fcolLimits;
}


tCIDLib::TVoid
TCQCIntfSliderBase::SetLimitsFor(const  tCQCKit::EUserRoles eRole
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
//  TCQCIntfSliderBase: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfSliderBase::
TCQCIntfSliderBase( const   tCQCIntfEng::ECapFlags  eCapFlags
                    , const TString&                strTypeName) :

    TCQCIntfWidget(eCapFlags, strTypeName)
    , m_bUseLimits(kCIDLib::False)
    , m_bValError(kCIDLib::True)
    , m_bWriteable(kCIDLib::False)
    , m_eCurRole(tCQCKit::EUserRoles::LimitedUser)
    , m_f8Max(kCQCIntfEng::f8MaxRange)
    , m_f8Min(kCQCIntfEng::f8MinRange)
    , m_f8Value(kCQCIntfEng::f8MinRange)
    , m_fcolLimits(tCQCKit::EUserRoles::Count)
    , m_i4Offset(0)
{
    //
    //  Initialize all the limits values to min/max values we defaulted to.
    //  The max is in the high word and the min in the low.
    //
    const tCIDLib::TCard4 c4InitLim = TRawBits::c4From16
    (
        tCIDLib::TCard2(kCQCIntfEng::f8MinRange)
        , tCIDLib::TCard2(kCQCIntfEng::f8MaxRange)
    );
    m_fcolLimits.AddElements(c4InitLim, tCIDLib::c4EnumOrd(tCQCKit::EUserRoles::Count));

    //
    //  Load up the keys into the image list mixin. These default settings will
    //  be overridden if we are being created to stream in a widget.
    //
    AddNewImgKey
    (
        CQCIntfEng_Slider::strThumbKey, tCQCIntfEng::EImgReqFlags::Required
    );
    AddNewImgKey
    (
        CQCIntfEng_Slider::strPressedKey, tCQCIntfEng::EImgReqFlags::Optional
    );

    // We provide one widget command value, the current value
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

    // Set our default appearance
    bIsTransparent(kCIDLib::True);
    SetSize(TSize(48, 148), kCIDLib::False);
    SetImgKey
    (
        CQCIntfEng_Slider::strThumbKey
        , L"/System/ImgSets/Antique/Slider Thumb"
        , tCQCIntfEng::EImgReqFlags::Required
        , 0xFF
        , kCIDLib::True
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Slider");
}

TCQCIntfSliderBase::TCQCIntfSliderBase(const TCQCIntfSliderBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCIntfImgListIntf(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_bUseLimits(iwdgSrc.m_bUseLimits)
    , m_bValError(iwdgSrc.m_bValError)
    , m_bWriteable(iwdgSrc.m_bWriteable)
    , m_eCurRole(iwdgSrc.m_eCurRole)
    , m_fcolLimits(iwdgSrc.m_fcolLimits)
    , m_f8Max(iwdgSrc.m_f8Max)
    , m_f8Min(iwdgSrc.m_f8Min)
    , m_f8Value(iwdgSrc.m_f8Value)
    , m_i4Offset(iwdgSrc.m_i4Offset)
{
}

TCQCIntfSliderBase&
TCQCIntfSliderBase::operator=(const TCQCIntfSliderBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCIntfImgListIntf::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);

        m_bUseLimits    = iwdgSrc.m_bUseLimits;
        m_bValError     = iwdgSrc.m_bValError;
        m_bWriteable    = iwdgSrc.m_bWriteable;
        m_eCurRole      = iwdgSrc.m_eCurRole;
        m_f8Max         = iwdgSrc.m_f8Max;
        m_f8Min         = iwdgSrc.m_f8Min;
        m_f8Value       = iwdgSrc.m_f8Value;
        m_fcolLimits    = iwdgSrc.m_fcolLimits;
        m_i4Offset      = iwdgSrc.m_i4Offset;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Handle streaming us in and out. Note that V2 format didn't change any
//  actual streaming. It just let us change some setup.
//
tCIDLib::TVoid TCQCIntfSliderBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Slider::c2BaseFmtVersion))
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
    MCQCIntfImgListIntf::ReadInImgList(strmToReadFrom);
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // If V3 or beyond, read in the per-user limits, else default it
    if (c2FmtVersion > 2)
    {
        strmToReadFrom  >>  m_bUseLimits
                        >>  m_fcolLimits;
    }
     else
    {
        // Set up the default limits and disable them
        const tCIDLib::TCard4 c4InitLim = TRawBits::c4From16
        (
            tCIDLib::TCard2(0), tCIDLib::TCard2(kCQCIntfEng::f8MaxRange)
        );
        m_fcolLimits.SetAll(c4InitLim);
        m_bUseLimits = kCIDLib::False;
    }

    // And it should all end with an end marker
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


tCIDLib::TVoid TCQCIntfSliderBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Slider::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Handle any mixins
    MCQCIntfImgListIntf::WriteOutImgList(strmToWriteTo);
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // Write out our stuff and end object marker
    strmToWriteTo   << m_bUseLimits
                    << m_fcolLimits
                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  This is called when we need to redraw. We get the invalid area and
//  try to do the minimal update necessary to fill that space.
//
tCIDLib::TVoid
TCQCIntfSliderBase::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Move in a bit more, clip and draw
    grgnClip.Deflate(1, 1);
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    TArea areaDraw = grgnClip.areaBounds();

    //
    //  Calc the thumb area for our current value. If tracking, we use
    //  the tracking area.
    //
    TArea areaImgSrc;
    TArea areaTar;
    MCQCIntfImgListIntf::TImgInfo* pimgiDraw = pimgiCalcThumbArea
    (
        m_f8Value, areaImgSrc, areaTar, kCIDLib::False
    );

    // And now we can draw the thumb if it intersects the invalid area
    if (pimgiDraw && areaTar.bIntersects(areaInvalid))
    {
        pimgiDraw->m_cptrImg->bmpImage().AdvDrawOn
        (
            gdevTarget
            , areaImgSrc
            , areaTar
            , pimgiDraw->m_cptrImg->bTransparent()
            , pimgiDraw->m_cptrImg->c4TransClr()
            , pimgiDraw->m_cptrImg->bmpMask()
            , tCIDGraphDev::EBltTypes::None
            , pimgiDraw->m_c1Opacity
        );
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by derived classes to tell us what volume level to display. The
//  return is true if we have a good value and the new value is different.
//
tCIDLib::TBoolean
TCQCIntfSliderBase::bSetValue(  const   tCIDLib::TFloat8    f8ToSet
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
TCQCIntfSliderBase::bValueOutOfRange(const tCIDLib::TFloat8 f8Check) const
{
    if ((f8Check < m_f8Min) || (f8Check > m_f8Max))
        return kCIDLib::True;
    return kCIDLib::False;
}


//
//  Let's the derived class tell us about the value, it's type and limits
//  and whether it's writeable.
//
tCIDLib::TVoid
TCQCIntfSliderBase::SetValueAttrs(  const   tCIDLib::TBoolean   bWriteable
                                    , const tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCIDLib::TBoolean   bInitToMin)
{
    //
    //  If we are using limits go through and check them. If any are out of range,
    //  revert them all back to the full min/max. If not using limits also make
    //  sure they are set to the full range.
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

        m_f8Max = f8Max;
        m_f8Min = f8Min;
    }

    //
    //  If in designer mode or asked, put the current value at the min, and
    //  take us out of the initial error mode if we are still in it. The static
    //  derivative does this since it never gets any external value to set.
    //
    if (TFacCQCIntfEng::bDesMode() || bInitToMin)
    {
        m_f8Value = m_f8Min;
        m_bValError = kCIDLib::False;
    }

    m_bWriteable = bWriteable;
}


// ---------------------------------------------------------------------------
//  TCQCIntfSliderBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Calculate a new value within the range, based on the passed point. We
//  assume the caller checked that we within the axis of travel, so we
//  check that.
//
tCIDLib::TFloat8
TCQCIntfSliderBase::f8ValFromPos(const  TPoint&             pntAt
                                , const tCIDLib::TBoolean   bDrag)
{
    MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForState(bDrag);
    TArea areaSrc;
    if (pimgiCur)
        areaSrc.Set(TPoint::pntOrigin, pimgiCur->m_cptrImg->szImage());
    else
        areaSrc.Set(TPoint::pntOrigin, TSize(0, 0));

    const TArea& areaUs = areaActual();
    tCIDLib::TFloat8 f8Ret;
    if (areaUs.c4Width() >= areaUs.c4Height())
    {
        const tCIDLib::TInt4 i4HalfWidth(tCIDLib::TInt4(areaSrc.c4Width() / 2));
        const tCIDLib::TInt4 i4Min = areaUs.i4X() + i4HalfWidth;
        const tCIDLib::TInt4 i4Max = areaUs.i4Right() - i4HalfWidth;

        if (pntAt.i4X() <= i4Min)
        {
            f8Ret = m_f8Min;
        }
         else if (pntAt.i4X() >= i4Max)
        {
            f8Ret = m_f8Max;
        }
         else
        {
            tCIDLib::TFloat8 f8Per = tCIDLib::TFloat8(pntAt.i4X() - i4Min)
                                     / tCIDLib::TFloat8(i4Max - i4Min);
            f8Ret = m_f8Min + ((m_f8Max - m_f8Min) * f8Per);
        }
    }
     else
    {
        const tCIDLib::TInt4 i4HalfHeight(tCIDLib::TInt4(areaSrc.c4Height() / 2));
        const tCIDLib::TInt4 i4Max = areaUs.i4Y() + i4HalfHeight;
        const tCIDLib::TInt4 i4Min = areaUs.i4Bottom() - i4HalfHeight;

        if (pntAt.i4Y() >= i4Min)
        {
            f8Ret = m_f8Min;
        }
         else if (pntAt.i4Y() <= i4Max)
        {
            f8Ret = m_f8Max;
        }
         else
        {
            tCIDLib::TFloat8 f8Per
            (
                tCIDLib::TFloat8(i4Min - pntAt.i4Y())
                / tCIDLib::TFloat8(i4Min - i4Max)
            );
            f8Ret = m_f8Min + ((m_f8Max - m_f8Min) * f8Per);
        }
    }

    //
    //  If we are doing per-user role limits, then clip this value to the
    //  per-user role, else take it as is.
    //
    if (m_bUseLimits)
    {
        const tCIDLib::TCard4 c4Lims = m_fcolLimits[m_eCurRole];
        const tCIDLib::TFloat8 f8Low = tCIDLib::TFloat8(TRawBits::i2Low16From32(c4Lims));
        const tCIDLib::TFloat8 f8High = tCIDLib::TFloat8(TRawBits::i2High16From32(c4Lims));

        if (f8Ret < f8Low)
            f8Ret = f8Low;
        else if (f8Ret > f8High)
            f8Ret = f8High;
    }

    return f8Ret;
}


//
//  Calculates the thumb area for the passed value, based on the current
//  tracking state. It passes back the correct image for the state, and
//  the area of the image for the state.
//
MCQCIntfImgListIntf::TImgInfo*
TCQCIntfSliderBase::pimgiCalcThumbArea( const   tCIDLib::TFloat8    f8Val
                                        ,       TArea&              areaImgSrc
                                        ,       TArea&              areaToFill
                                        , const tCIDLib::TBoolean   bDrag)
{
    // Get the appropriate thumb from the image list mixin
    MCQCIntfImgListIntf::TImgInfo* pimgiCur = pimgiForState(bDrag);

    // Get the thumb image size
    if (pimgiCur)
        areaImgSrc.Set(TPoint::pntOrigin, pimgiCur->m_cptrImg->szImage());
    else
        areaImgSrc.Set(TPoint::pntOrigin, TSize(0, 0));

    //
    //  Decide if we are doing horz or vertical movement. It's based on whether
    //  we are wider than tall. Then the available movement area is the len
    //  of the axis of movement minus the size of the thumb in that axis (the
    //  center point of the thumb indicates the position so when the thumb is
    //  at the ends of widget area, the center point is half the thumb size
    //  out from the end.)
    //
    //  If the thumb is bigger than or as big as the movement axis, we don't
    //  display anything.
    //
    const TArea& areaUs = areaActual();
    areaToFill = areaImgSrc;
    if (areaUs.c4Width() >= areaUs.c4Height())
    {
        const tCIDLib::TInt4 i4HalfWidth(tCIDLib::TInt4(areaImgSrc.c4Width() / 2));
        tCIDLib::TInt4 i4Width  = tCIDLib::TInt4(areaUs.c4Width())
                                - tCIDLib::TInt4(areaImgSrc.c4Width());
        if (i4Width <= 0)
        {
            areaToFill.ZeroAll();
            return pimgiCur;
        }

        // We can move it so move it
        tCIDLib::TFloat8 f8Per = (f8Val - m_f8Min)
                                 / (m_f8Max - m_f8Min);

        //
        //  The X offset is our start pos, plus the available width times
        //  the percent of the value within the range. Since the image
        //  origin is to the left we just use our left side as the base
        //  position which puts the mid-point at the right place.
        //
        const tCIDLib::TInt4 i4XPos = areaUs.i4X()
                                      + tCIDLib::TInt4(i4Width * f8Per);

        // Create a target area that's centered vertically and at the x pos
        areaToFill.CenterIn(areaUs);
        areaToFill.i4X(i4XPos);
    }
     else
    {
        const tCIDLib::TInt4 i4HalfHeight(tCIDLib::TInt4(areaImgSrc.c4Height() / 2));
        tCIDLib::TInt4 i4Height = tCIDLib::TInt4(areaUs.c4Height())
                                - tCIDLib::TInt4(areaImgSrc.c4Height());
        if (i4Height <= 0)
        {
            areaToFill.ZeroAll();
            return pimgiCur;
        }

        // We can move it so move it
        tCIDLib::TFloat8 f8Per = (f8Val - m_f8Min)
                                 / (m_f8Max - m_f8Min);

        //
        //  The X offset is our start pos, plus the available height times
        //  the percent of the value within the range.
        //
        const tCIDLib::TInt4 i4YPos = (areaUs.i4Bottom()
                                       - tCIDLib::TInt4(i4Height * f8Per))
                                       - tCIDLib::TInt4(areaImgSrc.c4Height());

        // Create a target area that's centered horizontal and at the y pos
        areaToFill.CenterIn(areaUs);
        areaToFill.i4Y(i4YPos);
    }
    return pimgiCur;
}


//
//  Returns the image info object (from our image list mixin) for the current
//  state of this widget (pressed or umpressed.)
//
MCQCIntfImgListIntf::TImgInfo*
TCQCIntfSliderBase::pimgiForState(const tCIDLib::TBoolean bDragging)
{
    // If in designer mode, just return the unpressed image
    if (TFacCQCIntfEng::bDesMode())
        return pimgiForKey(CQCIntfEng_Slider::strThumbKey);

    //
    //  If then show the pressed image if we have one, else fall through
    //  to the regular thumb.
    //
    if (bDragging)
    {
        MCQCIntfImgListIntf::TImgInfo* pimgiRet
                        = pimgiForKey(CQCIntfEng_Slider::strPressedKey);
        if (pimgiRet->m_bEnabled)
            return pimgiRet;
    }
    return pimgiForKey(CQCIntfEng_Slider::strThumbKey);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfFieldSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfFieldSlider: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfFieldSlider::TCQCIntfFieldSlider() :

    TCQCIntfSliderBase
    (
        CQCIntfEng_Slider::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefField)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Slider)
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

TCQCIntfFieldSlider::TCQCIntfFieldSlider(const TCQCIntfFieldSlider& iwdgToCopy) :

    TCQCIntfSliderBase(iwdgToCopy)
    , MCQCIntfSingleFldIntf(iwdgToCopy)
{
}

TCQCIntfFieldSlider::~TCQCIntfFieldSlider()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldSlider: Public operators
// ---------------------------------------------------------------------------
TCQCIntfFieldSlider&
TCQCIntfFieldSlider::operator=(const TCQCIntfFieldSlider& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfSingleFldIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfFieldSlider: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called to see if we can accept, and optionally update ourselves
//  to conform to, a new field. In our case we check that it's a numeric
//  field and if so (and we are storing) we store the range info on our parent
//  class.
//
tCIDLib::TBoolean
TCQCIntfFieldSlider::bCanAcceptField(const  TCQCFldDef&         flddTest
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

    // If it's good, set up our parent class with this range info
    if (bRes && bStore)
        SetValueAttrs(flddTest.bIsWriteable(), f8Min, f8Max, kCIDLib::False);
    return bRes;
}


tCIDLib::TBoolean TCQCIntfFieldSlider::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfFieldSlider& iwdgOther(static_cast<const TCQCIntfFieldSlider&>(iwdgSrc));

    // We just add the field mixin
    return MCQCIntfSingleFldIntf::bSameField(iwdgOther);
}


// Polymorphically copy us
tCIDLib::TVoid
TCQCIntfFieldSlider::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfFieldSlider&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfFieldSlider::eDoCmd(const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfFieldSlider::Initialize(TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfFieldSlider::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Do our mixins
    MCQCIntfSingleFldIntf::QueryFieldAttrs(colAttrs, adatTmp, L"Value Field");

    //
    //  Only do these for field based widgets since we have no way of knowing what the
    //  limits will be for variable based ones (vars don't exist at design time) and static
    //  ones have built in values so there's no point in setting limits.
    //
    colAttrs.objPlace
    (
        L"Per User Limits:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // And do our stuff
    colAttrs.objPlace
    (
        L"Use Limits"
        , kCQCIntfEng::strAttr_Slider_UseLimits
        , tCIDMData::EAttrTypes::Bool
        , tCIDMData::EAttrEdTypes::Both
    ).SetBool(bUseLimits());

    //
    //  Based on the use limits flag, we will enable or disable the limits, which are
    //  done as a binary blob.
    //
    TBinMBufOutStream strmOut(512UL);
    strmOut << fcolLimits() << kCIDLib::FlushIt;
    colAttrs.objPlace
    (
        L"Limits"
        , kCQCIntfEng::strAttr_Slider_Limits
        , strmOut.mbufData()
        , strmOut.c4CurPos()
        , TString::strEmpty()
        , bUseLimits() ? tCIDMData::EAttrEdTypes::Visual : tCIDMData::EAttrEdTypes::None
    );
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfFieldSlider::QueryCmds(         TCollection<TCQCCmd>&   colCmds
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
TCQCIntfFieldSlider::QueryMonikers(tCIDLib::TStrHashSet& colToFill) const
{
    TParent::QueryMonikers(colToFill);
    MCQCIntfSingleFldIntf::QueryFieldMonikers(colToFill);
}


// We just pass this on to our single field mixin
tCIDLib::TVoid
TCQCIntfFieldSlider::RegisterFields(        TCQCPollEngine& polleToUse
                                    , const TCQCFldCache&   cfcData)
{
    MCQCIntfSingleFldIntf::RegisterSingleField(polleToUse, cfcData);
}


tCIDLib::TVoid
TCQCIntfFieldSlider::Replace(const  tCQCIntfEng::ERepFlags  eToRep
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
TCQCIntfFieldSlider::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);
    MCQCIntfSingleFldIntf::SetFieldAttr(wndAttrEd, adatNew, adatOld);

    // If the field changes, we have to update the limits attribute
    if (adatNew.strId() == kCQCIntfEng::strAttr_Field)
    {
        TBinMBufOutStream strmOut(512UL);
        strmOut << fcolLimits() << kCIDLib::FlushIt;
        wndAttrEd.UpdateMemBuf
        (
            kCQCIntfEng::strAttr_Slider_Limits, strmOut.c4CurPos(), strmOut.mbufData()
        );
    }
}


// At this level, we vall our parent, then validate the field mixin
tCIDLib::TVoid
TCQCIntfFieldSlider::Validate(  const   TCQCFldCache&           cfcData
                                ,       tCQCIntfEng::TErrList&  colErrs
                                ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our field mixin
    bValidateField(cfcData, colErrs, dsclVal, *this);
}



// ---------------------------------------------------------------------------
//  TCQCIntfFieldSlider: Protected, inherited methods
// ---------------------------------------------------------------------------

// Our field mixin saw a change in our field, so react to that
tCIDLib::TVoid
TCQCIntfFieldSlider::FieldChanged(          TCQCFldPollInfo&    cfpiAssoc
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
TCQCIntfFieldSlider::FormatValue(const  tCIDLib::TFloat8    f8ToFmt
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


// Handle streaming us in and out
tCIDLib::TVoid TCQCIntfFieldSlider::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Slider::c2FieldFmtVersion))
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

    // We don't have any persistent stuff ourself yet

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfFieldSlider::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Slider::c2FieldFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream any mixins
    MCQCIntfSingleFldIntf::WriteOutField(strmToWriteTo);

    // We don't have any of our own yet, so just stream the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The base class calls this when the user drops the slider inside the
//  slider track and so a new value needs to be set. We take the value and
//  write it to our associated field. We let any exceptions propogate back
//  to the base class.
//
//  Note that if the no auto-write flag is set in the field interface,
//  we don't do anything.
//
tCIDLib::TVoid
TCQCIntfFieldSlider::UserSetValue(const tCIDLib::TFloat8 f8NewValue)
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
TCQCIntfFieldSlider::ValueUpdate(       TCQCPollEngine&     polleToUse
                                , const tCIDLib::TBoolean   bNoRedraw
                                , const tCIDLib::TBoolean
                                , const TStdVarsTar&        ctarGlobalVars
                                , const TGUIRegion&)
{
    FieldUpdate(polleToUse, bNoRedraw, ctarGlobalVars);
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfStaticSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfStaticSlider: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfStaticSlider::TCQCIntfStaticSlider() :

    TCQCIntfSliderBase
    (
        CQCIntfEng_Slider::eStaticCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefStatic)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Slider)
    )
    , MCQCIntfStaticIntf(tCQCKit::EFldTypes::Int)
{
}

TCQCIntfStaticSlider::TCQCIntfStaticSlider(const TCQCIntfStaticSlider& iwdgToCopy) :

    TCQCIntfSliderBase(iwdgToCopy)
    , MCQCIntfStaticIntf(iwdgToCopy)
{
}

TCQCIntfStaticSlider::~TCQCIntfStaticSlider()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticSlider: Public operators
// ---------------------------------------------------------------------------
TCQCIntfStaticSlider&
TCQCIntfStaticSlider::operator=(const TCQCIntfStaticSlider& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfStaticIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticSlider: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfStaticSlider::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfStaticSlider& iwdgOther(static_cast<const TCQCIntfStaticSlider&>(iwdgSrc));

    // We just add the static mixin
    return MCQCIntfStaticIntf::bSameStatic(iwdgOther);
}


// Polymorphically copy us
tCIDLib::TVoid
TCQCIntfStaticSlider::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfStaticSlider&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfStaticSlider::eDoCmd(const  TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCIntfEng::strCmdId_SetRange)
    {
        //
        //  Make sure the values are ok. If so, it updates our static interface
        //  and our parent class with the new values.
        //
        CheckRangeVals(ccfgToDo.piAt(0).m_strValue, ccfgToDo.piAt(1).m_strValue);

        //
        //  If we can redraw, then do that, and send out an OnSet so that any change
        //  in value will be picked up.
        //
        if (bCanRedraw(strEventId))
        {
            Redraw();
            PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
        }
    }
     else if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_SetValue)
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
            // A new value was stored, so generate an OnSet command
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
TCQCIntfStaticSlider::Initialize(TCQCIntfContainer* const   piwdgParent
                                , TDataSrvClient&           dsclInit
                                ,  tCQCIntfEng::TErrList&   colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    //
    //  Initialize our parent to our data type and range settings, which
    //  we get from the static mixin. Tell it to set the initial value to
    //  the range minimum, which will get it out of the initial error state.
    //  Since we never actually set a value otherwise, this is necessary.
    //
    SetValueAttrs(kCIDLib::True, f8RangeMin(), f8RangeMax(), kCIDLib::True);
}


// Return any action commands we support
tCIDLib::TVoid
TCQCIntfStaticSlider::QueryCmds(        TCollection<TCQCCmd>&   colCmds
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
                kCQCIntfEng::strCmdId_SetRange
                , facCQCIntfEng().strMsg(kIEngMsgs::midCmd_SetRange)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MinVal)
                , tCQCKit::ECmdPTypes::Signed
                , facCQCKit().strMsg(kKitMsgs::midCmdP_MaxVal)
            )
        );
    }

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
TCQCIntfStaticSlider::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // We just add the statoc mixin
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
TCQCIntfStaticSlider::SetStaticInfo(const   tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCQCKit::EFldTypes  eType)
{
    MCQCIntfStaticIntf::SetStaticInfo(f8Min, f8Max, eType);

    //
    //  Use the values as stored, since it's possible he tweaked them during
    //  storage.
    //
    SetValueAttrs(kCIDLib::True, f8RangeMin(), f8RangeMax(), kCIDLib::True);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfStaticSlider::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // We just add the static mixin
    MCQCIntfStaticIntf::SetStaticAttr(wndAttrEd, adatNew, adatOld);
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticSlider: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfStaticSlider::FormatValue(  const   tCIDLib::TFloat8    f8ToFmt
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


// Handle streaming us in and out
tCIDLib::TVoid TCQCIntfStaticSlider::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Slider::c2StaticFmtVersion))
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

    // And pull in our stuff or the static interface, according to version
    if (c2FmtVersion == 1)
    {
        //
        //  Pull the range values into temps and set on the new static mixn.
        //  The old data type would have been assumed to be int, so set that
        //  as well.
        //
        tCIDLib::TInt4 i4Max, i4Min;
        strmToReadFrom >> i4Max >> i4Min;
        SetStaticInfo(i4Min, i4Max, tCQCKit::EFldTypes::Int);
    }
     else
    {
        MCQCIntfStaticIntf::ReadInStatic(strmToReadFrom);
    }

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfStaticSlider::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Slider::c2StaticFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfStaticIntf::WriteOutStatic(strmToWriteTo);

    // And do our stuff and the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The base class calls this when the user drops the slider inside the
//  slider track and so a new value needs to be set. For us, this is a no
//  op, since whatever value the user slides it to is fine with us.
//
tCIDLib::TVoid TCQCIntfStaticSlider::UserSetValue(const tCIDLib::TFloat8)
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfStaticSlider: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Used by our SetRange command, to avoid a lot of redundant code.
//
tCIDLib::TVoid
TCQCIntfStaticSlider::CheckRangeVals(const TString& strMin, const TString& strMax)
{
    // Validate the values
    tCIDLib::TFloat8 f8Min = strMin.f8Val();
    tCIDLib::TFloat8 f8Max = strMax.f8Val();

    tCIDLib::TBoolean bValid = kCIDLib::False;
    if (f8Max > f8Min)
    {
        // Validate against the type
        switch(eDataType())
        {
            case tCQCKit::EFldTypes::Card :
            {
                bValid = ((f8Min > 0) && (f8Max < tCIDLib::TFloat8(kCIDLib::c4MaxCard)));
                break;
            }

            case tCQCKit::EFldTypes::Float :
            {
                bValid = ((f8Min > kCIDLib::f8MinFloat) && (f8Max < kCIDLib::f8MaxFloat));
                break;
            }

            case tCQCKit::EFldTypes::Int :
            {
                bValid = ((f8Min > tCIDLib::TFloat8(kCIDLib::i4MinInt))
                            && (f8Max < tCIDLib::TFloat8(kCIDLib::i4MaxInt)));
                break;
            }

            default :
                CIDAssert2(L"Unknown static interface data type")
                break;
        };
    }

    if (!bValid)
    {
        facCQCIntfEng().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIEngErrs::errcCfg_BadRange
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Range
        );
    }

    //
    //  Looks ok, so set it on our static mixin. We just set the data type back
    //  on itself since that hasn't changed.
    //
    SetStaticInfo(f8Min, f8Max, eDataType());
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfVarSlider
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfVarSlider: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfVarSlider::TCQCIntfVarSlider() :

    TCQCIntfSliderBase
    (
        CQCIntfEng_Slider::eFldCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_PrefVariable)
          + facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Slider)
    )
    , MCQCIntfVarIntf()
{
}

TCQCIntfVarSlider::TCQCIntfVarSlider(const TCQCIntfVarSlider& iwdgToCopy) :

    TCQCIntfSliderBase(iwdgToCopy)
    , MCQCIntfVarIntf(iwdgToCopy)
{
}

TCQCIntfVarSlider::~TCQCIntfVarSlider()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarSlider: Public operators
// ---------------------------------------------------------------------------
TCQCIntfVarSlider&
TCQCIntfVarSlider::operator=(const TCQCIntfVarSlider& iwdgToAssign)
{
    if (this != &iwdgToAssign)
    {
        TParent::operator=(iwdgToAssign);
        MCQCIntfVarIntf::operator=(iwdgToAssign);
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarSlider: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfVarSlider::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfVarSlider& iwdgOther(static_cast<const TCQCIntfVarSlider&>(iwdgSrc));

    // We just add the variable mixin
    return MCQCIntfVarIntf::bSameVariable(iwdgOther);
}


// Polymorphically copy us from another widget of the same type
tCIDLib::TVoid TCQCIntfVarSlider::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfVarSlider&>(iwdgSrc);
}


// Called when an action command is invoked on us
tCQCKit::ECmdRes
TCQCIntfVarSlider::eDoCmd(  const   TCQCCmdCfg&         ccfgToDo
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
TCQCIntfVarSlider::QueryCmds(       TCollection<TCQCCmd>&   colCmds
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
TCQCIntfVarSlider::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // We just add the variable mixin
    MCQCIntfVarIntf::QueryVarAttrs(colAttrs, adatTmp);
}


tCIDLib::TVoid
TCQCIntfVarSlider::Replace( const   tCQCIntfEng::ERepFlags  eToRep
                            , const TString&                strSrc
                            , const TString&                strTar
                            , const tCIDLib::TBoolean       bRegEx
                            , const tCIDLib::TBoolean       bFull
                            ,       TRegEx&                 regxFind)
{
    TParent::Replace(eToRep, strSrc, strTar, bRegEx, bFull, regxFind);
    if (tCIDLib::bAllBitsOn(eToRep, tCQCIntfEng::ERepFlags::AssocVar))
        MCQCIntfVarIntf::ReplaceVar(strSrc, strTar, bRegEx, bFull, regxFind);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfVarSlider::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // We just add the variable mixin
    MCQCIntfVarIntf::SetVarAttr(wndAttrEd, adatNew, adatOld);
}


tCIDLib::TVoid
TCQCIntfVarSlider::Validate(const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);

    // And our variable interface
    ValidateVar(colErrs, dsclVal, *this);
}


// ---------------------------------------------------------------------------
//  TCQCIntfVarSlider: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Our variable mixin calls us here when we acquire the associated variable,
//  to make sure it has the correct attributes for our use. We can only use
//  variables of numeric type with range limits that are within a specific
//  min/max range. There's a helper method in the kit facility that will
//  do this check for us. If it's good, we need to set up our parent class
//  with this range info.
//
tCIDLib::TBoolean TCQCIntfVarSlider::bCanAcceptVar(const TCQCActVar& varToTest)
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

    // If it's good, store it on our parent class
    if (bRes)
        SetValueAttrs(kCIDLib::True, f8RealMin, f8RealMax, kCIDLib::True);
    return bRes;
}


tCIDLib::TVoid
TCQCIntfVarSlider::FormatValue( const   tCIDLib::TFloat8    f8ToFmt
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


// Stream in/out our contents
tCIDLib::TVoid TCQCIntfVarSlider::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Slider::c2VarFmtVersion))
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

tCIDLib::TVoid TCQCIntfVarSlider::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format version
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Slider::c2VarFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCIntfVarIntf::WriteOutVar(strmToWriteTo);

    // We don't have any data now so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


//
//  The base class calls this when the user drops the slider inside the
//  slider track and so a new value needs to be set. For us, we need to
//  update the variable to reflect this value.
//
tCIDLib::TVoid
TCQCIntfVarSlider::UserSetValue(const tCIDLib::TFloat8 f8NewVal)
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
TCQCIntfVarSlider::ValueUpdate(         TCQCPollEngine&
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
tCIDLib::TVoid TCQCIntfVarSlider::VarInError(const tCIDLib::TBoolean bNoRedraw)
{
    bSetValue(0, bNoRedraw, kCIDLib::True);
}



//
//  Our variable interface mixin calls us back here if the value changes.
//  So, when we get called here, we know that we are in ready state, either
//  already were or have just transitioned into it.
//
tCIDLib::TVoid
TCQCIntfVarSlider::VarValChanged(const  TCQCActVar&         varNew
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
