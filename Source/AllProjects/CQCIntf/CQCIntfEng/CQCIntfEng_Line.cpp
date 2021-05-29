//
// FILE NAME: CQCIntfEng_Line.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2013
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
//  This file implements the class that handles line drawing.
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
#include    "CQCIntfEng_Line.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfLine,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_Line
    {
        // -----------------------------------------------------------------------
        //  Format versions
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;

        // -----------------------------------------------------------------------
        //  The fixed size we maintain in transverse direction
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4   c4TransSize = 14;


        // -----------------------------------------------------------------------
        //  Our capabilities flags
        // -----------------------------------------------------------------------
        constexpr tCQCIntfEng::ECapFlags eCapFlags = tCQCIntfEng::ECapFlags
        (
            tCQCIntfEng::ECapFlags::None
        );
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfLine
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfLine: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfLine::TCQCIntfLine() :

    TCQCIntfWidget
    (
        CQCIntfEng_Line::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_Line)
    )
    , m_bHorizontal(kCIDLib::True)
    , m_bThick(kCIDLib::False)
    , m_eEndType(tCQCIntfEng::ELineEnds::None)
{
    // Set out default look
    bShadow(kCIDLib::True);
    rgbFgn(kCQCIntfEng_::rgbDef_Text);
    rgbFgn2(kCQCIntfEng_::rgbDef_TextShadow);

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/Line");
}

TCQCIntfLine::TCQCIntfLine(const TCQCIntfLine& iwdgSrc) :

    TCQCIntfWidget(iwdgSrc)
    , m_bHorizontal(iwdgSrc.m_bHorizontal)
    , m_bThick(iwdgSrc.m_bThick)
    , m_eEndType(iwdgSrc.m_eEndType)
{
}

TCQCIntfLine::~TCQCIntfLine()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfLine: Public operators
// ---------------------------------------------------------------------------
TCQCIntfLine&
TCQCIntfLine::operator=(const TCQCIntfLine& iwdgSrc)
{
    if (this != &iwdgSrc)
    {
        TParent::operator=(iwdgSrc);

        m_bHorizontal   = iwdgSrc.m_bHorizontal;
        m_bThick        = iwdgSrc.m_bThick;
        m_eEndType      = iwdgSrc.m_eEndType;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLine: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCIntfLine::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Check our parent first
    if (!TParent::bIsSame(iwdgSrc))
        return kCIDLib::False;

    const TCQCIntfLine& iwdgOther(static_cast<const TCQCIntfLine&>(iwdgSrc));

    return
    (
        (m_bHorizontal == iwdgOther.m_bHorizontal)
        && (m_bThick == iwdgOther.m_bThick)
        && (m_eEndType == iwdgOther.m_eEndType)
    );
}


tCIDLib::TVoid
TCQCIntfLine::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfLine&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfLine::Initialize(TCQCIntfContainer* const   piwdgParent
                        , TDataSrvClient&           dsclInit
                        , tCQCIntfEng::TErrList&    colErrs)
{
    // For now, we just pass it on to our parent
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfLine::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                            , TAttrData&            adatTmp) const
{
    // Get our parent's attributes
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove those common ones that are not applicable to us
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_BgnFill);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Border);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_NoInertia);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Transparent);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Base_Underline);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Bgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn3);

    // The only two colors we keep, set their names
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1, L"Line");
    facCIDMData().SetAttrName(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2, L"Shadow");

    // Add ours
    adatTmp.Set(L"Line Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator);
    colAttrs.objAdd(adatTmp);

    TString strLim;
    tCQCIntfEng::FormatELineEnds
    (
        strLim, kCIDMData::strAttrLim_EnumPref, kCIDLib::chComma
    );
    adatTmp.Set
    (
        L"End Type"
        , kCQCIntfEng::strAttr_Line_EndType
        , strLim
        , tCIDMData::EAttrTypes::String
    );
    adatTmp.SetString(tCQCIntfEng::strXlatELineEnds(m_eEndType));
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Horziontal"
        , kCQCIntfEng::strAttr_Line_HVDir
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bHorizontal);
    colAttrs.objAdd(adatTmp);

    adatTmp.Set
    (
        L"Thick"
        , kCQCIntfEng::strAttr_Line_Thick
        , tCIDMData::EAttrTypes::Bool
    );
    adatTmp.SetBool(m_bThick);
    colAttrs.objAdd(adatTmp);
}


TSize TCQCIntfLine::szDefaultSize() const
{
    return TSize(64, CQCIntfEng_Line::c4TransSize);
}


// Update ourself with the new attribute
tCIDLib::TVoid
TCQCIntfLine::SetWdgAttr(       TAttrEditWnd&   wndAttrEd
                        , const TAttrData&      adatNew
                        , const TAttrData&      adatOld)
{
    // Do the base attrs
    TParent::SetWdgAttr(wndAttrEd, adatNew, adatOld);

    // Handle our own attributes
    if (adatNew.strId() == kCQCIntfEng::strAttr_Line_EndType)
        m_eEndType = tCQCIntfEng::eXlatELineEnds(adatNew.strValue());
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Line_HVDir)
        m_bHorizontal = adatNew.bVal();
    else if (adatNew.strId() == kCQCIntfEng::strAttr_Line_Thick)
        m_bThick = adatNew.bVal();
}


tCIDLib::TVoid
TCQCIntfLine::Validate( const   TCQCFldCache&           cfcData
                        ,       tCQCIntfEng::TErrList&  colErrs
                        ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfLine: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCIntfLine::bHorizontal() const
{
    return m_bHorizontal;
}

tCIDLib::TBoolean
TCQCIntfLine::bHorizontal(const tCIDLib::TBoolean bToSet)
{
    //
    //  If changing the orientation, then we have to flip our size, not just
    //  change the orientation.
    //
    if (m_bHorizontal != bToSet)
    {
        if (bToSet)
            SetSize(64, CQCIntfEng_Line::c4TransSize);
        else
            SetSize(CQCIntfEng_Line::c4TransSize, 64);
        m_bHorizontal = bToSet;
    }
    return m_bHorizontal;
}


tCIDLib::TBoolean TCQCIntfLine::bThick() const
{
    return m_bThick;
}

tCIDLib::TBoolean
TCQCIntfLine::bThick(const tCIDLib::TBoolean bToSet)
{
    m_bThick  = bToSet;
    return m_bThick;
}


tCQCIntfEng::ELineEnds TCQCIntfLine::eEndType() const
{
    return m_eEndType;
}

tCQCIntfEng::ELineEnds
TCQCIntfLine::eEndType(const tCQCIntfEng::ELineEnds eToSet)
{
    m_eEndType = eToSet;
    return m_eEndType;
}


// ---------------------------------------------------------------------------
//  TCQCIntfLine: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We override this so that we can prevent resize in the transverse direction.
//  We stay 5 pixels in size in that direction. That allows enough size to
//  get ahold of the ends and to hit the center for moving.
//
//  If not in designer mode we just call the standard tester in our parent
//  class.
//
tCIDCtrls::EWndAreas TCQCIntfLine::eHitTest(const TPoint& pntTest) const
{
    //
    //  First call the standard hit tester in the base widget class to see
    //  where we were hit.
    //
    tCIDCtrls::EWndAreas eRet = TParent::eHitTest(pntTest);

    // If not in designer mode, return the result from above
    if (!TFacCQCIntfEng::bDesMode())
        return eRet;

    //
    //  Depending on our orientation, disallow some edge types, and
    //  all corner types in all case, so they can't be used. We just
    //  return nowhere for those.
    //
    if ((eRet == tCIDCtrls::EWndAreas::TopLeft)
    ||  (eRet == tCIDCtrls::EWndAreas::TopRight)
    ||  (eRet == tCIDCtrls::EWndAreas::BottomLeft)
    ||  (eRet == tCIDCtrls::EWndAreas::BottomRight))
    {
        eRet = tCIDCtrls::EWndAreas::Nowhere;
    }
     else if (m_bHorizontal)
    {
        // We don't allow top or bottom, only the ends
        if ((eRet == tCIDCtrls::EWndAreas::Top)
        ||  (eRet == tCIDCtrls::EWndAreas::Bottom))
        {
            eRet = tCIDCtrls::EWndAreas::Nowhere;
        }
    }
     else
    {
        // We don't allow left or right, only the top/bottom ends
        if ((eRet == tCIDCtrls::EWndAreas::Left)
        ||  (eRet == tCIDCtrls::EWndAreas::Right))
        {
            eRet = tCIDCtrls::EWndAreas::Nowhere;
        }
    }
    return eRet;
}


tCIDLib::TVoid TCQCIntfLine::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_Line::c2FmtVersion))
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

    // Stream in our stuff
    strmToReadFrom  >> m_bHorizontal
                    >> m_bThick
                    >> m_eEndType;

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfLine::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_Line::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // And our stuff, followed by the end marker
    strmToWriteTo   << m_bHorizontal
                    << m_bThick
                    << m_eEndType
                    << tCIDLib::EStreamMarkers::EndObject;
}


//
//  We are always transparent background. We draw our line down the center
//  of the area, in the direction we are set up for. We down't allow ourself
//  to be resized in the transverse direction.
//
//  If a 'thick' line we either do a geometric line, or just draw it twice
//  if in remote viewer mode, since it doesn't understand geomstric lines.
//
tCIDLib::TVoid
TCQCIntfLine::Update(       TGraphDrawDev&  gdevTarget
                    , const TArea&          areaInvalid
                    ,       TGUIRegion&     grgnClip)
{
    const TArea& areaUs = areaActual();

    TPoint pntStart = areaUs.pntCenter();
    TPoint pntEnd = pntStart;

    //
    //  Set the values to their correct respective ends of our area. Also
    //  if we are both thick and shadowed, moved back by one. to keep us
    //  closer to the center.
    //
    TPoint pntSS;
    TPoint pntSE;
    if (m_bHorizontal)
    {
        pntStart.i4X(areaUs.i4X());
        pntEnd.i4X(areaUs.i4Right());

        if (m_bThick && bShadow())
        {
            pntStart.Adjust(0, -1);
            pntEnd.Adjust(0, -1);
        }

        pntSS = pntStart;
        pntSE = pntEnd;
        if (m_bThick)
        {
            pntSS.Adjust(0, 1);
            pntSE.Adjust(0, 1);
        }
    }
     else
    {
        pntStart.i4Y(areaUs.i4Y());
        pntEnd.i4Y(areaUs.i4Bottom());

        if (m_bThick && bShadow())
        {
            pntStart.Adjust(-1, 0);
            pntEnd.Adjust(-1, 0);
        }

        pntSS = pntStart;
        pntSE = pntEnd;
        if (m_bThick)
        {
            pntSS.Adjust(1, 0);
            pntSE.Adjust(1, 0);
        }
    }

    if (bShadow())
    {
        if (m_bHorizontal)
        {
            pntSS.Adjust(0, 1);
            pntSE.Adjust(0, 1);
        }
         else
        {
            pntSS.Adjust(1, 0);
            pntSE.Adjust(1, 0);
        }
    }

    //
    //  Draw the shadow version first. Move down and right either one or
    //  two pixels, depending on if we are thick or not. The shadow is always
    //  one pixel.
    //
    Draw(gdevTarget, pntSS, pntSE, rgbFgn2(), kCIDLib::False, kCIDLib::False);

    // Now draw the main line
    Draw(gdevTarget, pntStart, pntEnd, rgbFgn(), m_bThick, kCIDLib::True);

    //
    //  If we have an image end type, then we can do that now. They just draw
    //  over the line content at the ends.
    //
    if (m_eEndType == tCQCIntfEng::ELineEnds::Circle)
    {
        // No outline on these
        TCosmeticPen gpenInvis(facCIDGraphDev().rgbWhite, tCIDGraphDev::ELineStyles::Invisible);
        TPenJanitor janPen(&gdevTarget, &gpenInvis);

        //
        //  If thick and shadow, move the shadow points back one since we pushed
        //  them an extra pixel above to avoid hvaing to draw an extra shadow
        //  line.
        //
        if (m_bThick && bShadow())
        {
            if (m_bHorizontal)
            {
                pntSS.Adjust(0, -1);
                pntSE.Adjust(0, -1);
            }
             else
            {
                pntSS.Adjust(-1, 0);
                pntSE.Adjust(-1, 0);
            }
        }

        // Draw the shadow first
        if (bShadow())
            DrawCircleEnds(gdevTarget, pntSS, pntSE, rgbFgn3(), kCIDLib::True);

        // And then the actual ends
        DrawCircleEnds(gdevTarget, pntStart, pntEnd, rgbFgn(), kCIDLib::False);
    }
}


// ---------------------------------------------------------------------------
//  TCQCIntfLine: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TCQCIntfLine::Draw(         TGraphDrawDev&      gdevTarget
                    , const TPoint&             pntStart
                    , const TPoint&             pntEnd
                    , const TRGBClr&            rgbLine
                    , const tCIDLib::TBoolean   bThick
                    , const tCIDLib::TBoolean   bDoEnds)
{
    gdevTarget.DrawLine(pntStart, pntEnd, rgbLine);

    // If a thick line, draw it again
    if (bThick)
    {
        m_pntEnd = pntEnd;
        m_pntStart = pntStart;
        if (m_bHorizontal)
        {
            m_pntEnd.Adjust(0, 1);
            m_pntStart.Adjust(0, 1);
        }
         else
        {
            m_pntEnd.Adjust(1, 0);
            m_pntStart.Adjust(1, 0);
        }

        gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);
    }

    //
    //  Do the line end if it's a cross bar type, which is just a line.
    //  For the others they are done separately and will just overwrite
    //  the ends of the lines.
    //
    if (bDoEnds && (m_eEndType == tCQCIntfEng::ELineEnds::Crossbar))
    {
        if (m_bHorizontal)
        {
            // Do the left one
            m_pntStart = pntStart;
            m_pntEnd = pntStart;
            m_pntStart.Adjust(0, -5);
            m_pntEnd.Adjust(0, 5);
            if (bThick)
                m_pntEnd.Adjust(0, 1);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);

            m_pntStart.Adjust(1, 0);
            m_pntEnd.Adjust(1, 0);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);


            // And do the right side
            m_pntStart = pntEnd;
            m_pntEnd = pntEnd;
            m_pntStart.Adjust(0, -5);
            m_pntEnd.Adjust(0, 5);
            if (bThick)
                m_pntEnd.Adjust(0, 1);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);

            m_pntStart.Adjust(-1, 0);
            m_pntEnd.Adjust(-1, 0);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);
        }
         else
        {
            // Do the top one
            m_pntStart = pntStart;
            m_pntEnd = pntStart;
            m_pntStart.Adjust(-5, 0);
            m_pntEnd.Adjust(5, 0);
            if (bThick)
                m_pntEnd.Adjust(1, 0);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);

            m_pntStart.Adjust(0, 1);
            m_pntEnd.Adjust(0, 1);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);


            // And do the right side
            m_pntStart = pntEnd;
            m_pntEnd = pntEnd;
            m_pntStart.Adjust(-5, 0);
            m_pntEnd.Adjust(5, 0);
            if (bThick)
                m_pntEnd.Adjust(1, 0);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);

            m_pntStart.Adjust(0, -1);
            m_pntEnd.Adjust(0, -1);
            gdevTarget.DrawLine(m_pntStart, m_pntEnd, rgbLine);
        }
    }
}


tCIDLib::TVoid
TCQCIntfLine::DrawCircleEnds(       TGraphDrawDev&      gdevTarget
                            , const TPoint&             pntStart
                            , const TPoint&             pntEnd
                            , const TRGBClr&            rgbFill
                            , const tCIDLib::TBoolean   bShadow)
{
    TSolidBrush gbrushFill(rgbFill);
    TBrushJanitor janBrush(&gdevTarget, &gbrushFill);

    const tCIDLib::TCard4 c4Radius = m_bThick ? 5 : 4;
    m_pntStart = pntStart;
    if (m_bHorizontal)
        m_pntStart.Adjust(c4Radius, 0);
    else
        m_pntStart.Adjust(0, c4Radius);

    m_pntEnd = pntEnd;
    if (m_bHorizontal)
        m_pntEnd.Adjust(-tCIDLib::TInt4(c4Radius - 1), 0);
    else
        m_pntEnd.Adjust(0, -tCIDLib::TInt4(c4Radius - 1));

    if (m_bThick)
    {
        if (m_bHorizontal)
        {
            m_pntStart.Adjust(0, 1);
            m_pntEnd.Adjust(0, 1);
        }
         else
        {
            m_pntStart.Adjust(1, 0);
            m_pntEnd.Adjust(1, 0);
        }
    }

    gdevTarget.DrawFilledCircle(c4Radius, m_pntStart);
    gdevTarget.DrawFilledCircle(c4Radius, m_pntEnd);
}

