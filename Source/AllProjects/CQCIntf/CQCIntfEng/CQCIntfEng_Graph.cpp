//
// FILE NAME: CQCIntfEng_Graph.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2011
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
//  This file implements our graphing widgets.
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
#include    "CQLogicSh.hpp"
#include    "CQCIntfEng_Graph.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCIntfGraphBase,TCQCIntfWidget)
AdvRTTIDecls(TCQCIntfGraphLS, TCQCIntfGraphBase)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_Graph
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2BaseFmtVersion = 1;
        constexpr tCIDLib::TCard2   c2LSFmtVersion = 1;


        // -----------------------------------------------------------------------
        //  The caps flags that we use
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::ActiveUpdate
        );


        // -----------------------------------------------------------------------
        //  The logic server graph widget's update period
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TEncodedTime enctLSSamplePer = kCIDLib::enctOneSecond * 15;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfGraphBase
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Destructor
// ---------------------------------------------------------------------------
TCQCIntfGraphBase::~TCQCIntfGraphBase()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Public, inherited methods
// ---------------------------------------------------------------------------

// We cannot size to contents since our contents is runtime only
tCIDLib::TBoolean TCQCIntfGraphBase::bCanSizeTo() const
{
    return kCIDLib::False;
}


tCIDLib::TBoolean
TCQCIntfGraphBase::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfGraphBase& iwdgOther
    (
        static_cast<const TCQCIntfGraphBase&>(iwdgSrc)
    );

    return
    (
        (m_eScale == iwdgOther.m_eScale)
        && (m_f4MaxVal == iwdgOther.m_f4MaxVal)
        && (m_f4MinVal == iwdgOther.m_f4MinVal)
    );
}


tCIDLib::TVoid TCQCIntfGraphBase::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfGraphBase&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfGraphBase::Initialize(  TCQCIntfContainer* const    piwdgParent
                                , TDataSrvClient&           dsclInit
                                , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Reset any runtime info
    m_grdatSamples.Reset(0UL);

    //
    //  Update our area and point arrays for our initial size. Pass a bogus area
    //  for the old area, so it'll update.
    //
    AdjustGraphSizePos(areaActual(), TArea::areaEmpty);

    //
    //  If we are in designer mode, then we just fill the graph with a test
    //  sine wave for display purposes.
    //
    if (TFacCQCIntfEng::bDesMode())
        GenTestWave();
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfGraphBase::QueryWdgAttrs(tCIDMData::TAttrList&  colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Delete any attributes we don't use
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Shadow);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);

    // Rename colors that we kept and aren't defaulted to what we want
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Graph Lines");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3, L"Border");

    // Add our stuff
    adatTmp.Set(L"Graph Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Min Value"
        , kCQCIntfEng::strAttr_Graph_Min
        , tCIDMData::EAttrTypes::Float
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetFloat(tCIDLib::TFloat8(m_f4MinVal));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Max Value"
        , kCQCIntfEng::strAttr_Graph_Max
        , tCIDMData::EAttrTypes::Float
        , tCIDMData::EAttrEdTypes::InPlace
    );
    adatTmp.SetFloat(tCIDLib::TFloat8(m_f4MaxVal));
    colAttrs.objAdd(adatTmp);

    TString strLim;
    tCIDMath::FormatEGraphScales
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"Scale"
        , kCQCIntfEng::strAttr_Graph_Scale
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCIDMath::strXlatEGraphScales(m_eScale));
    colAttrs.objAdd(adatTmp);
}


TSize TCQCIntfGraphBase::szDefaultSize() const
{
    if (bHasBorder())
        return TSize(248, 164);
    return TSize(244, 160);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfGraphBase::SetWdgAttr(          TAttrEditWnd&   wndAttrEd
                                , const TAttrData&      adatNew
                                , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Graph_Min)
        m_f4MinVal = tCIDLib::TFloat4(adatNew.f8Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Graph_Max)
        m_f4MaxVal = tCIDLib::TFloat4(adatNew.f8Val());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Graph_Scale)
        m_eScale = tCIDMath::eXlatEGraphScales(adatNew.strValue());
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the max and min graph range values
tCIDLib::TFloat4 TCQCIntfGraphBase::f4MaxVal() const
{
    return m_f4MaxVal;
}

tCIDLib::TFloat4 TCQCIntfGraphBase::f4MaxVal(const tCIDLib::TFloat4 f4ToSet)
{
    m_f4MaxVal = f4ToSet;
    return m_f4MaxVal;
}


tCIDLib::TFloat4 TCQCIntfGraphBase::f4MinVal() const
{
    return m_f4MinVal;
}

tCIDLib::TFloat4 TCQCIntfGraphBase::f4MinVal(const tCIDLib::TFloat4 f4ToSet)
{
    m_f4MinVal = f4ToSet;
    return m_f4MinVal;
}


// Get/set the graph scale
tCIDMath::EGraphScales TCQCIntfGraphBase::eScale() const
{
    return m_eScale;
}

tCIDMath::EGraphScales
TCQCIntfGraphBase::eScale(const tCIDMath::EGraphScales eToSet)
{
    m_eScale = eToSet;
    return m_eScale;
}


//
//  This is for the designer, to force it to update the test data after the
//  user changes the configuration. Pass a bogus empty old area so it'll update.
//
tCIDLib::TVoid TCQCIntfGraphBase::ForceUpdate()
{
    AdjustGraphSizePos(areaActual(), TArea::areaEmpty);
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Hidden Constructors and operators
// ---------------------------------------------------------------------------
TCQCIntfGraphBase::TCQCIntfGraphBase(const  TString&        strTypeName
                                    , const tCIDLib::TCard4 c4MaxSamples) :

    TCQCIntfWidget(CQCIntfEng_Graph::eCapFlags, strTypeName)
    , m_c4AvailPnts(0)
    , m_c4MaxSamples(c4MaxSamples)
    , m_eScale(tCIDMath::EGraphScales::Linear)
    , m_f4MaxVal(256)
    , m_f4MinVal(0)
    , m_grdatSamples(c4MaxSamples)
    , m_pntaDrawPnts(2)
    , m_pntaGraphPnts(2)
{
}

// Sample data is viewing time only and isn't copied
TCQCIntfGraphBase::TCQCIntfGraphBase(const TCQCIntfGraphBase& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_c4AvailPnts(0)
    , m_c4MaxSamples(iwdgSrc.m_c4MaxSamples)
    , m_eScale(iwdgSrc.m_eScale)
    , m_f4MaxVal(iwdgSrc.m_f4MaxVal)
    , m_f4MinVal(iwdgSrc.m_f4MinVal)
    , m_grdatSamples(iwdgSrc.m_c4MaxSamples)
    , m_pntaDrawPnts(iwdgSrc.m_pntaDrawPnts.c4Count())
    , m_pntaGraphPnts(iwdgSrc.m_pntaGraphPnts.c4Count())
{
}

TCQCIntfGraphBase&
TCQCIntfGraphBase::operator=(const TCQCIntfGraphBase& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_c4MaxSamples = iwdgSrc.m_c4MaxSamples;
        m_eScale       = iwdgSrc.m_eScale;
        m_f4MaxVal     = iwdgSrc.m_f4MaxVal;
        m_f4MinVal     = iwdgSrc.m_f4MinVal;

        // Sample data is viewing time only and is not copied
        m_c4AvailPnts = 0;
        m_grdatSamples.Reset(0UL);
        m_pntaDrawPnts.Reallocate(iwdgSrc.m_pntaDrawPnts.c4Count());
        m_pntaGraphPnts.Reallocate(iwdgSrc.m_pntaGraphPnts.c4Count());
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have to update our graph area, which is the part of our area that we
//  draw the graph data in.
//
tCIDLib::TVoid
TCQCIntfGraphBase::AreaChanged(const TArea& areaNew, const TArea& areaOld)
{
    // Call our parent first
    TParent::AreaChanged(areaNew, areaOld);

    // Adjust our graph size/pos info as required
    AdjustGraphSizePos(areaNew, areaOld);

    if (TFacCQCIntfEng::bDesMode())
        GenTestWave();
}


// Stream our configuration in from the passed stream
tCIDLib::TVoid TCQCIntfGraphBase::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Graph::c2BaseFmtVersion))
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

    // Stream our stuff
    strmToReadFrom  >> m_f4MaxVal
                    >> m_f4MinVal
                    >> m_eScale;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any images and make sure we are in ready state
    m_grdatSamples.Reset(0UL);
    m_c4AvailPnts = 0;
}



//
//  We have to update our graph area, which is the part of our area that we
//  draw the graph data in.
//
tCIDLib::TVoid
TCQCIntfGraphBase::SizeChanged(const TSize& szNew, const TSize& szOld)
{
    // Call our parent first
    TParent::SizeChanged(szNew, szOld);

    // Adjust our graph size/pos info as required
    TPoint pntOrg = areaActual().pntOrg();
    TArea areaNew(pntOrg, szNew);
    TArea areaOld(pntOrg, szOld);
    AdjustGraphSizePos(areaNew, areaOld);

    // If in designer mode, generate a test wave at our new size
    if (TFacCQCIntfEng::bDesMode())
        GenTestWave();
}

tCIDLib::TVoid TCQCIntfGraphBase::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Graph::c2BaseFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << m_f4MaxVal
                    << m_f4MinVal
                    << m_eScale
                    << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfGraphBase::Update(          TGraphDrawDev&  gdevTarget
                            , const TArea&          areaInvalid
                            ,       TGUIRegion&     grgnClip)
{
    // Do any bgn fill/border
    TParent::Update(gdevTarget, areaInvalid, grgnClip);

    if (m_c4AvailPnts)
    {
        TRegionJanitor janClip(&gdevTarget, grgnClip, tCIDGraphDev::EClipModes::And);
        gdevTarget.DrawPolyLine(m_pntaDrawPnts, rgbFgn(), m_c4AvailPnts);
    }
}



// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Derived classes call this to add more samples to the graph. We add them
//  to our graph data, calculate the new point array, and redraw.
//
tCIDLib::TVoid
TCQCIntfGraphBase::AddNewSamples(const  tCQLogicSh::TSampleList&    fcolNew
                                , const tCIDLib::TCard4             c4Count
                                , const tCIDLib::TBoolean           bNoRedraw)
{
    m_grdatSamples.PushNewSamples(fcolNew, c4Count, 0);

    m_c4AvailPnts = m_pntaGraphPnts.c4SetFromGraph
    (
        m_grdatSamples
        , m_eScale
        , m_areaGraph.c4Height()
        , m_f4MinVal
        , m_f4MaxVal
    );

    // Update the drawable points offset to our drawing origin
    m_pntaDrawPnts.SetFromOffset(m_pntaGraphPnts, m_areaGraph.pntOrg());

    if (!bNoRedraw)
        Redraw();
}


//
//  If the derived class discovers that its source of graph samples is no longer
//  trustworth, it calls this to reset us.
//
tCIDLib::TVoid TCQCIntfGraphBase::ResetGraph()
{
    m_grdatSamples.Reset(0UL);
    m_c4AvailPnts = 0;
    Redraw();
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphBase: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  On init and on any area change, we call this to get ourselfs back into
//
tCIDLib::TVoid
TCQCIntfGraphBase::AdjustGraphSizePos(const TArea& areaNew, const TArea& areaOld)
{
    // Remember our current count of points
    const tCIDLib::TCard4 c4OldCnt = m_pntaDrawPnts.c4Count();

    // Calc the new area
    m_areaGraph = areaNew;
    m_areaGraph.Deflate(2);

    //
    //  If the size has changed, reallocate the graph points lists and reload them
    //  from the graph data.
    //
    const tCIDLib::TCard4 c4NewCnt = m_areaGraph.c4Width();
    if (c4NewCnt != c4OldCnt)
    {
        m_pntaDrawPnts.Reallocate(c4NewCnt);
        m_pntaGraphPnts.Reallocate(c4NewCnt);
        m_c4AvailPnts = 0;

        m_c4AvailPnts = m_pntaGraphPnts.c4SetFromGraph
        (
            m_grdatSamples
            , m_eScale
            , m_areaGraph.c4Height()
            , m_f4MinVal
            , m_f4MaxVal
        );
    }

    //
    //  Either way, update our drawing points list, in case the origin
    //  moved. We need to move our drawable list to move with us.
    //
    m_pntaDrawPnts.SetFromOffset(m_pntaGraphPnts, m_areaGraph.pntOrg());
}


//
//  When in designer mode, we fill ourself with a simple sine way for display
//  purposes so taht the user can see where the graph object is when it is
//  set to transparent, which normally it would be.
//
tCIDLib::TVoid TCQCIntfGraphBase::GenTestWave()
{
    // Put some dummy data in for testing, just a sine wave
    const tCIDLib::TCard4  c4SampleCnt = m_grdatSamples.c4MaxSamples();
    const tCIDLib::TFloat4 f4Inc = ((2 * kCIDLib::f4PI) * 2) / c4SampleCnt;
    const tCIDLib::TFloat4 f4Range(m_f4MaxVal - m_f4MinVal);
    const tCIDLib::TFloat4 f4HalfRange(f4Range / 2);

    //
    //  We need to adjust the values if they are not either zero based or
    //  zero centered, in order to keep them between the min and max. So we
    //  get the center point of the high/low values and calculate how far
    //  that is off from the half range.
    //
    tCIDLib::TFloat4 f4DCOfs = (m_f4MinVal + m_f4MaxVal) / 2;

    tCIDLib::TFloat4 f4Angle = -(2 * kCIDLib::f4PI);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SampleCnt; c4Index++)
    {
        m_grdatSamples.PushNewSample
        (
            (TMathLib::f4Sine(f4Angle) * f4HalfRange) + f4DCOfs
        );
        f4Angle += f4Inc;
    }

    //
    //  Load up our list of points from the graph data, scaled appropriately.
    //  For our purposes we always indicate it's a linear scale so that the
    //  sine wave fills the whole graph vertically.
    //
    m_c4AvailPnts = m_pntaGraphPnts.c4SetFromGraph
    (
        m_grdatSamples
        , tCIDMath::EGraphScales::Linear
        , m_areaGraph.c4Height()
        , m_f4MinVal
        , m_f4MaxVal
    );

    // Update the drawable points offset to our drawing origin
    m_pntaDrawPnts.SetFromOffset(m_pntaGraphPnts, m_areaGraph.pntOrg());
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfGraphLS
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfGraphLS: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfGraphLS::TCQCIntfGraphLS() :

    TCQCIntfGraphBase(L"Logic Srv Graph", kCQLogicSh::c4GraphSampleCnt)
    , m_c4CfgSerialNum(0)
    , m_c4SerialNum(0)
    , m_enctNextUpdate(0)
    , m_fcolNewSamples(kCQLogicSh::c4GraphSampleCnt)
{
}

// Only the name value is something that needs to be copied
TCQCIntfGraphLS::TCQCIntfGraphLS(const TCQCIntfGraphLS& iwdgSrc) :

    TCQCIntfGraphBase(iwdgSrc)
    , m_strGraphName(iwdgSrc.m_strGraphName)
{
    m_c4CfgSerialNum =  0;
    m_c4SerialNum =  0;
    m_enctNextUpdate = 0;
}

TCQCIntfGraphLS::~TCQCIntfGraphLS()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphLS: Public operators
// ---------------------------------------------------------------------------
TCQCIntfGraphLS& TCQCIntfGraphLS::operator=(const TCQCIntfGraphLS& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);
        m_strGraphName = iwdgSrc.m_strGraphName;

        // Only the name needs to be moved over, reset everything else
        m_c4CfgSerialNum =  0;
        m_c4SerialNum =  0;
        m_enctNextUpdate = 0;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphLS: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfGraphLS::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfGraphLS& iwdgOther
    (
        static_cast<const TCQCIntfGraphLS&>(iwdgSrc)
    );
    return (m_strGraphName == iwdgOther.m_strGraphName);
}


tCIDLib::TVoid TCQCIntfGraphLS::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfGraphLS&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfGraphLS::Initialize(TCQCIntfContainer* const    piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // Call our parent first
    TParent::Initialize(piwdgParent, dsclInit, colErrs);

    // Reset any runtime info
    m_c4CfgSerialNum =  0;
    m_c4SerialNum =  0;
    m_enctNextUpdate = 0;
}


//
//  We do an initial query for graph data from the logic server shared
//  facility client. If the graph data is not already cached, then this
//  will just queue it up to start getting some. If it is, we'll get what
//  is available.
//
//  If we get nothing, we set out the next update stamp so that we'll
//  update in a short period of time, but enough that the cache should
//  hae time to get updated. Else we'll set it to the normal next update
//  time.
//
//  Only do this if not in the designer, where we'll have bogus data
//  already loaded.
//
tCIDLib::TVoid TCQCIntfGraphLS::PostInit()
{
    if (!TFacCQCIntfEng::bDesMode())
    {
        tCIDLib::TCard4 c4NewSamples;
        const tCQLogicSh::EGraphQRes eRes = facCQLogicSh().eQueryGraphSamples
        (
            m_strGraphName
            , m_c4CfgSerialNum
            , m_c4SerialNum
            , m_fcolNewSamples
            , c4NewSamples
        );

        if (eRes == tCQLogicSh::EGraphQRes::NewSamples)
        {
            //
            //  Tell the bass class not to redraw, just store the samples. We
            //  will be redraw in the initial display of the template data.
            //
            AddNewSamples(m_fcolNewSamples, c4NewSamples, kCIDLib::False);
            m_enctNextUpdate = 0;
        }
         else
        {
            m_enctNextUpdate = TTime::enctNow() + kCIDLib::enctTwoSeconds;
        }
    }
}


//
//  We call our parent first, then add a few bits of our own
//
tCIDLib::TVoid
TCQCIntfGraphLS::QueryWdgAttrs( tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    //
    //  We set the attribute type to indicate it's a Logic Server graph, which the
    //  editor window will use to do the right visual edit.
    //
    adatTmp.Set
    (
        L"Graph Name"
        , kCQCIntfEng::strAttr_Graph_LSName
        , tCIDMData::EAttrTypes::String
        , tCIDMData::EAttrEdTypes::Both
    );
    adatTmp.SetString(m_strGraphName);
    adatTmp.SetSpecType(kCQCIntfEng::strAttrType_LSGraph);
    colAttrs.objAdd(adatTmp);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfGraphLS::SetWdgAttr(        TAttrEditWnd&   wndAttrEd
                            , const TAttrData&      adatNew
                            , const TAttrData&      adatOld)
{
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    if (adatNew.strId() == kCQCIntfEng::strAttr_Graph_LSName)
        m_strGraphName = adatNew.strValue();
}


tCIDLib::TVoid
TCQCIntfGraphLS::Validate(  const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Call our parent
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphLS: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Get/set the graph name
const TString& TCQCIntfGraphLS::strGraphName() const
{
    return m_strGraphName;
}

const TString& TCQCIntfGraphLS::strGraphName(const TString& strToSet)
{
    m_strGraphName = strToSet;
    return m_strGraphName;
}


// ---------------------------------------------------------------------------
//  TCQCIntfGraphLS: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCIntfGraphLS::ActiveUpdate(  const   tCIDLib::TBoolean bNoRedraw
                                , const TGUIRegion&
                                , const tCIDLib::TBoolean)
{
    //
    //  If our next sample time is arrived, then let's see if we have more
    //  data to display.
    //
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur >= m_enctNextUpdate)
    {
        // Update the next sample time first, in case of error
        m_enctNextUpdate = enctCur + CQCIntfEng_Graph::enctLSSamplePer;

        tCIDLib::TCard4 c4NewSamples;
        const tCQLogicSh::EGraphQRes eRes = facCQLogicSh().eQueryGraphSamples
        (
            m_strGraphName
            , m_c4CfgSerialNum
            , m_c4SerialNum
            , m_fcolNewSamples
            , c4NewSamples
        );

        //
        //  If we got new samples, give them to the base class. If the logic
        //  server polling engine is in error state, then reset our parent
        //  class, to clear the graph contents until we reconnect. The above
        //  call will have already reset our serial numbers in that case, so
        //  that we'll start from scratch again once the connection is back
        //  up.
        //
        if (eRes == tCQLogicSh::EGraphQRes::NewSamples)
            AddNewSamples(m_fcolNewSamples, c4NewSamples, kCIDLib::False);
        else if (eRes == tCQLogicSh::EGraphQRes::Error)
            ResetGraph();
    }
}


// Stream ourself in
tCIDLib::TVoid TCQCIntfGraphLS::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version first
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Graph::c2LSFmtVersion))
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

    // Stream our stuff
    strmToReadFrom  >> m_strGraphName;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);

    // Reset any images and make sure we are in ready state
    m_c4CfgSerialNum =  0;
    m_c4SerialNum =  0;
    m_enctNextUpdate = 0;
}


// Stream ourself out
tCIDLib::TVoid TCQCIntfGraphLS::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and format verson
    strmToWriteTo << tCIDLib::EStreamMarkers::StartObject
                  << CQCIntfEng_Graph::c2LSFmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // Stream our stuff
    strmToWriteTo   << m_strGraphName
                    << tCIDLib::EStreamMarkers::EndObject;
}

