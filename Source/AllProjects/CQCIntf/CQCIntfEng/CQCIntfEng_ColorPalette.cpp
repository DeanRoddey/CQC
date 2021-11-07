//
// FILE NAME: CQCIntfEng_ClrPalette.cpp
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
#include    "CQCIntfEng_ColorPalette.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfClrPalette,TCQCIntfWidget)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_ClrPalette
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
            tCQCIntfEng::ECapFlags::DoesGestures
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfPalRTV
//  PREFIX: crtvs
//
//  We need a simple derivative of the RTV source class to provide some extra
//  slider specific RTVs.
// ---------------------------------------------------------------------------
class TCQCIntfPalRTV : public TCQCCmdRTVSrc
{
    public :
        // -------------------------------------------------------------------
        //  Public Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCIntfPalRTV( const   THSVClr&        hsvCurrent
                        , const TCQCUserCtx&    cuctxToUse) :

            TCQCCmdRTVSrc(cuctxToUse)
            , m_hsvCurrent(hsvCurrent)
        {
        }

        ~TCQCIntfPalRTV() {}


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bRTValue( const   TString&    strId
                                    ,       TString&    strToFill)  const final
        {
            // Call our parent first
            if (TCQCCmdRTVSrc::bRTValue(strId, strToFill))
                return kCIDLib::True;

            // If it's one of ours, then handle it
            if (strId == kCQCIntfEng::strRTVId_ColorRGB)
            {
                TRGBClr rgbTmp;
                m_hsvCurrent.ToRGB(rgbTmp);

                rgbTmp.FormatToText(strToFill, kCIDLib::chSpace);
            }
             else if ((strId == kCQCIntfEng::strRTVId_ColorHSVInt)
                  ||  (strId == kCQCIntfEng::strRTVId_ColorHSVFloat)
                  ||  (strId == kCQCIntfEng::strRTVId_ColorHSInt))
            {
                // We can use standard formatting support for this
                m_hsvCurrent.FormatToText
                (
                    strToFill
                    , kCIDLib::chSpace
                    , strId != kCQCIntfEng::strRTVId_ColorHSVFloat
                );

                // But in this case, remove the last value
                if (strId == kCQCIntfEng::strRTVId_ColorHSInt)
                {
                    tCIDLib::TCard4 c4At;
                    if (strToFill.bLastOccurrence(kCIDLib::chSpace, c4At))
                        strToFill.CapAt(c4At);
                }
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
        //  m_hsvCurrent
        //      The current color. If tracking then this is the tracking color.
        // -------------------------------------------------------------------
        THSVClr m_hsvCurrent;
};




// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfClrPalette
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfClrPalette: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfClrPalette::TCQCIntfClrPalette() :

    TCQCIntfWidget
    (
        CQCIntfEng_ClrPalette::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_ClrPalette)
    )
    , m_szPal(360, 256)
    , m_hsvCur(0, 1.0, 0.8F)
{
    //
    //  Set our initial background colors, so that they can see any filled background
    //  if they are sized up larger than the actual palette bitmap.
    //
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

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

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_ColorRGB)
        , kCQCIntfEng::strRTVId_ColorRGB
        , tCQCKit::ECmdPTypes::Color
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_ColorHSVInt)
        , kCQCIntfEng::strRTVId_ColorHSVInt
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_ColorHSInt)
        , kCQCIntfEng::strRTVId_ColorHSInt
        , tCQCKit::ECmdPTypes::Text
    );

    AddRTValue
    (
        facCQCIntfEng().strMsg(kIEngMsgs::midRTVal_ColorHSVFloat)
        , kCQCIntfEng::strRTVId_ColorHSVFloat
        , tCQCKit::ECmdPTypes::Text
    );

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/ColorPalette");
}

TCQCIntfClrPalette::TCQCIntfClrPalette(const TCQCIntfClrPalette& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , MCQCCmdSrcIntf(iwdgSrc)
    , m_hsvCur(iwdgSrc.m_hsvCur)
    , m_szPal(iwdgSrc.m_szPal)
{
}

TCQCIntfClrPalette::~TCQCIntfClrPalette()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfClrPalette: Public operators
// ---------------------------------------------------------------------------
TCQCIntfClrPalette&
TCQCIntfClrPalette::operator=(const TCQCIntfClrPalette& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        MCQCCmdSrcIntf::operator=(iwdgSrc);
        m_hsvCur = iwdgSrc.m_hsvCur;
        m_szPal = iwdgSrc.m_szPal;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfClrPalette: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfClrPalette::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    // For now that's all
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCIntfClrPalette::bCanSizeTo() const
{
    return kCIDLib::True;
}


//
//  We allow non-inertial pans. The scale doesn't really matter in non-inertials but
//  we set it to unity anyway. Two finger ones we allow to propagate. Otherwise we
//  are going to process it.
//
tCIDLib::TBoolean
TCQCIntfClrPalette::bPerGestOpts(const  TPoint&                 pntAt
                                , const tCIDLib::EDirs          eDir
                                , const tCIDLib::TBoolean       bTwoFingers
                                ,       tCIDCtrls::EGestOpts&   eToSet
                                ,       tCIDLib::TFloat4&       f4VScale)
{
    // We don't do two finger gestures
    if (bTwoFingers)
        return kCIDLib::True;

    eToSet = tCIDCtrls::EGestOpts::Pan;
    f4VScale = 1.0F;
    return kCIDLib::False;
}


tCIDLib::TVoid TCQCIntfClrPalette::QueryContentSize(TSize& szToFill)
{
    szToFill = m_szPal;

    if (bHasBorder())
        szToFill.Adjust(1, 1);
}



tCIDLib::TBoolean
TCQCIntfClrPalette::bProcessGestEv( const   tCIDCtrls::EGestEvs eEv
                                    , const TPoint&             pntStart
                                    , const TPoint&             pntAt
                                    , const TPoint&             pntDelta
                                    , const tCIDLib::TBoolean   bTwoFinger)
{
    if (eEv == tCIDCtrls::EGestEvs::Start)
    {
        // Store new color if we hit one, and do any OnPress actions
        bPointToColor(pntAt);

        // Post any 'on press' events
        PostOps(*this, kCQCKit::strEvId_OnPress, tCQCKit::EActOrders::NonNested);
    }
     else if (eEv == tCIDCtrls::EGestEvs::Drag)
    {
        // Store new color if we hit one,and do any OnDrag actions
        bPointToColor(pntAt);
        PostOps(*this, kCQCKit::strEvId_OnDrag, tCQCKit::EActOrders::NonNested);
    }
     else if (eEv == tCIDCtrls::EGestEvs::End)
    {
        // Store new color if we hit one,and do any OnRelease actions
        bPointToColor(pntAt);
        PostOps(*this, kCQCKit::strEvId_OnRelease, tCQCKit::EActOrders::NonNested);

        // And do an OnSet for any final update
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
    }
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCIntfClrPalette::Clicked(const TPoint& pntAt)
{
    //
    //  Convert the point to a color. If it hit a valid color, then we want to send
    //  an OnSet command.
    //
    if (bPointToColor(pntAt))
        PostOps(*this, kCQCKit::strEvId_OnSet, tCQCKit::EActOrders::NonNested);
}


tCIDLib::TVoid TCQCIntfClrPalette::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfClrPalette&>(iwdgSrc);
}


tCQCKit::ECmdRes
TCQCIntfClrPalette::eDoCmd( const   TCQCCmdCfg&         ccfgToDo
                            , const tCIDLib::TCard4     c4Index
                            , const TString&            strUserId
                            , const TString&            strEventId
                            ,       TStdVarsTar&        ctarGlobals
                            ,       tCIDLib::TBoolean&  bResult
                            ,       TCQCActEngine&      acteTar)
{
    if (ccfgToDo.strCmdId() == kCQCKit::strCmdId_GetColor)
    {
        const TString& strKey = ccfgToDo.piAt(0).m_strValue;
        TCQCActVar& varTar = TStdVarsTar::varFind
        (
            strKey, acteTar.ctarLocals(), ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );

        TString strToSet;
        if (ccfgToDo.piAt(1).m_strValue == L"RGB")
        {
            TRGBClr rgbTmp;
            m_hsvCur.ToRGB(rgbTmp);
            rgbTmp.FormatToText(strToSet, kCIDLib::chSpace);
        }
         else if ((ccfgToDo.piAt(1).m_strValue == L"HSV Float")
              ||  (ccfgToDo.piAt(1).m_strValue == L"HSV Integer"))
        {
            m_hsvCur.FormatToText
            (
                strToSet
                , kCIDLib::chSpace
                , (ccfgToDo.piAt(1).m_strValue == L"HSV Integer")
            );
        }

        if (varTar.bSetValue(strToSet) && acteTar.pcmdtDebug())
            acteTar.pcmdtDebug()->ActVarSet(varTar.strName(), varTar.strValue());
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
TCQCIntfClrPalette::Initialize( TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid TCQCIntfClrPalette::PostInit()
{
    // Do an initial palette generation, pass zero area for the old so it'll update
    DoSizeCalcs(areaActual(), TArea::areaEmpty);
}


//
//  Gen up one of our runtime value objects. The current value we provide
//  is different according to whether we are tracking or not.
//
TCQCCmdRTVSrc*
TCQCIntfClrPalette::pcrtsMakeNew(const TCQCUserCtx& cuctxToUse) const
{
    // We provide the current color
    return new TCQCIntfPalRTV(m_hsvCur, cuctxToUse);
}


// Give back our list of support action commands
tCIDLib::TVoid
TCQCIntfClrPalette::QueryCmds(          TCollection<TCQCCmd>&   colCmds
                                , const tCQCKit::EActCmdCtx     eContext) const
{
    // Call our parent first, to build them up in order of class layers
    TParent::QueryCmds(colCmds, eContext);


    //
    //  Get the last color set on the palette. This will include any previously set
    //  V part that has been set on us, though we don't use it.
    //
    {
        TCQCCmd cmdGetClr
        (
            kCQCKit::strCmdId_GetColor
            , facCQCKit().strMsg(kKitMsgs::midCmd_GetColor)
            , 2
        );

        cmdGetClr.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_VarKey), tCQCKit::ECmdPTypes::VarName
            )
        );

        cmdGetClr.cmdpAddParm
        (
            TCQCCmdParm
            (
                facCQCKit().strMsg(kKitMsgs::midCmdP_Format), L"RGB, HSV Float, HSV Integer"
            )
        );

        colCmds.objAdd(cmdGetClr);
    }
}


tCIDLib::TVoid
TCQCIntfClrPalette::QueryWdgAttrs(  tCIDMData::TAttrList&   colAttrs
                                    , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // objaToFill[tCQCIntfEng::EWdgColor_Fgn3] = L"Border";
}


TSize TCQCIntfClrPalette::szDefaultSize() const
{
    return TSize(360, 256);
}


// Set up default parameter values for some commands
tCIDLib::TVoid TCQCIntfClrPalette::SetDefParms(TCQCCmdCfg& ccfgToSet) const
{
    TParent::SetDefParms(ccfgToSet);

    // None of our own yet
}


tCIDLib::TVoid
TCQCIntfClrPalette::Validate(const  TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfClrPalette: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We need to recalculate our palette, and regenerate the image if the size
//  changed enough to affect it.
//
tCIDLib::TVoid
TCQCIntfClrPalette::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    TParent::AreaChanged(areaNew, areaOld);

    // Do our size cacls
    DoSizeCalcs(areaNew, areaOld);
}


tCIDLib::TVoid
TCQCIntfClrPalette::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    TParent::SizeChanged(szNew, szOld);

    // Do our size calcs
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);

    DoSizeCalcs(areaNew, areaOld);
}


tCIDLib::TVoid TCQCIntfClrPalette::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_ClrPalette::c2FmtVersion))
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
    MCQCCmdSrcIntf::ReadInOps(strmToReadFrom);

    // We don't current have any persistent data

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfClrPalette::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_ClrPalette::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Do any mixins
    MCQCCmdSrcIntf::WriteOutOps(strmToWriteTo);

    // WE don't have any stuff of our own so just do the end marker
    strmToWriteTo << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfClrPalette::Update(         TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    // Clip and draw
    TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);

    TArea areaPal(TPoint::pntOrigin, m_szPal);
    areaPal.CenterIn(areaActual());

    // If disabled, draw it mostly transparent using constant opacity
    if (eCurDisplayState() == tCQCIntfEng::EDispStates::Disabled)
        gdevTarget.AlphaBlit(m_bmpPal, areaPal.pntOrg(), 0x10, kCIDLib::False);
    else
        gdevTarget.DrawBitmap(m_bmpPal, TPoint::pntOrigin, areaPal.pntOrg());
}



// ---------------------------------------------------------------------------
//  TCQCIntfClrPalette: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfClrPalette::bPointToColor(const TPoint& pntSrc)
{
    TArea areaPal(TPoint::pntOrigin, m_szPal);
    areaPal.CenterIn(areaActual());
    if (!areaPal.bContainsPoint(pntSrc))
        return kCIDLib::False;

    //
    //  Convert to the hue/saturation values and update our current color
    //  with the new values. We don't set the Value attribute.
    //
    tCIDLib::TFloat4 f4Hue = tCIDLib::TFloat4(pntSrc.i4X() - areaPal.i4X());
    f4Hue /= tCIDLib::TFloat4(areaPal.c4Width());
    f4Hue *= 360;

    tCIDLib::TFloat4 f4Saturation = tCIDLib::TFloat4(pntSrc.i4Y() - areaPal.i4Y());
    f4Saturation /= tCIDLib::TFloat4(areaPal.c4Height());

    // Set our current color to this, plus our current value level
    m_hsvCur.Set(f4Hue, f4Saturation, m_hsvCur.f4Value());

    return kCIDLib::True;
}


//
//  Given a change in our size, we need to regenerate the bitmap. This should only happen
//  more than once at design time. Otherwise we'll get called once in post init.
//
tCIDLib::TVoid
TCQCIntfClrPalette::DoSizeCalcs(const TArea& areaNew, const TArea& areaOld)
{
    // Only bother to do anything if the size changed. Origin changes don't matter here
    if (!areaNew.bSameSize(areaOld))
    {
        // Start with our new area. Move it in if we have a border.
        TArea areaPal = areaNew;
        if (bHasBorder())
            areaPal.Deflate(1, 1);

        //
        //  Check for a patholgoically small size. If so, we just clip instead of
        //  trying to shink that small.
        //
        areaPal.TakeLarger(TSize(64, 64));

        //  Round to the nearest 3 pixel boundary in both axes.
        tCIDLib::TCard4 c4Sz = areaPal.c4Width() / 3;
        c4Sz *= 3;
        areaPal.c4Width(c4Sz);

        c4Sz = areaPal.c4Height() / 3;
        c4Sz *= 3;
        areaPal.c4Height(c4Sz);

        // If the palette area size changed, then regenerate the image
        if (m_bmpPal.szBitmap() != areaPal.szArea())
        {
            m_szPal = areaPal.szArea();
            TGraphMemDev gdevBmp(civOwner().gdevCompat(), m_szPal);

            //
            //  We split it up into 3x3 chunks. Our size is always adjusted to
            //  evenly divide by 3s.
            //
            const tCIDLib::TCard4 c4Rows = m_szPal.c4Height() / 3;
            const tCIDLib::TCard4 c4Cols = m_szPal.c4Width() / 3;

            // Decide how much to adjust for each row/col
            const tCIDLib::TFloat4  f4HInc = 360.0F / c4Cols;
            const tCIDLib::TFloat4  f4VInc = 1.0F / c4Rows;

            TArea               areaChunk(0, 0, 3, 3);
            THSVClr             hsvCur;
            TRGBClr             rgbCur;
            tCIDLib::TFloat4    f4HVal = 0;
            tCIDLib::TFloat4    f4VVal = 0;
            for (tCIDLib::TCard4 c4Row = 0; c4Row < c4Rows; c4Row++)
            {
                f4HVal = 0;
                for (tCIDLib::TCard4 c4Col = 0; c4Col < c4Cols; c4Col++)
                {
                    hsvCur.Set(f4HVal, f4VVal, 0.85F);
                    hsvCur.ToRGB(rgbCur);

                    gdevBmp.Fill(areaChunk, rgbCur);
                    areaChunk.Move(tCIDLib::EDirs::Right);

                    f4HVal += f4HInc;
                    if (f4HVal > 360)
                        f4HVal = 360;
                }

                f4VVal += f4VInc;
                if (f4VVal > 1)
                    f4VVal = 1;
                areaChunk.Move(tCIDLib::EDirs::Down);
                areaChunk.i4X(0);
            }

            m_bmpPal = TBitmap(gdevBmp.bmpCurrent());

            //
            //  Generate a unique id for this guy. We just call it color palette, since
            //  any color palette image of the same size will look the same.
            //
            m_bmpPal.strUserData(TString::strConcat(kCQCKit::strWRIVA_IVOTFPref, L"ColorPalette"));
        }
    }
}

