//
// FILE NAME: CQCIntfEng_StaticIntf.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/03/2008
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
//  This file implements a mixin interface that is commonly used by static
//  widgets to support having a user selected data type and optionally ranges
//  of possible values.
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


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCIntfEng_StaticIntf
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: MCQCIntfStaticIntf
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MCQCIntfStaticIntf: Destructor
// ---------------------------------------------------------------------------
MCQCIntfStaticIntf::~MCQCIntfStaticIntf()
{
}

// ---------------------------------------------------------------------------
//  MCQCIntfStaticIntf: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  Let's the config tab set us up with user config. The mixing in class
//  may override to react to this, but must call us as well.
//
tCIDLib::TVoid
MCQCIntfStaticIntf::SetStaticInfo(  const   tCIDLib::TFloat8    f8Min
                                    , const tCIDLib::TFloat8    f8Max
                                    , const tCQCKit::EFldTypes  eType)
{
    m_eType      = eType;
    m_f8RangeMax = f8Max;
    m_f8RangeMin = f8Min;
}



// ---------------------------------------------------------------------------
//  MCQCIntfStaticIntf: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
MCQCIntfStaticIntf::bSameStatic(const MCQCIntfStaticIntf& miwdgSrc) const
{
    return ((m_eType == miwdgSrc.m_eType)
            && (m_f8RangeMin == miwdgSrc.m_f8RangeMin)
            && (m_f8RangeMax == miwdgSrc.m_f8RangeMax));
}


// Provide access to the set data type
tCQCKit::EFldTypes MCQCIntfStaticIntf::eDataType() const
{
    return m_eType;
}


// Provide access to the min and max range values
tCIDLib::TFloat8 MCQCIntfStaticIntf::f8RangeMax() const
{
    return m_f8RangeMax;
}

tCIDLib::TFloat8 MCQCIntfStaticIntf::f8RangeMin() const
{
    return m_f8RangeMin;
}


tCIDLib::TVoid
MCQCIntfStaticIntf::QueryStaticAttrs(tCIDMData::TAttrList&  colAttrs
                                    , TAttrData&            adatTmp) const
{
    colAttrs.objPlace
    (
        L"Static Settings:", TString::strEmpty(), tCIDMData::EAttrTypes::Separator
    );

    // For now we only suport card, float or int
    TString strLims(kCIDMData::strAttrLim_EnumPref);
    strLims.Append(L"Card, Float, Int");
    adatTmp.Set
    (
        L"Value Type"
        , kCQCIntfEng::strAttr_Static_Type
        , strLims
        , tCIDMData::EAttrTypes::String
    );
    if (m_eType == tCQCKit::EFldTypes::Card)
        adatTmp.SetString(L"Card");
    else if (m_eType == tCQCKit::EFldTypes::Float)
        adatTmp.SetString(L"Float");
    else if (m_eType == tCQCKit::EFldTypes::Int)
        adatTmp.SetString(L"Int");
    else
    {
        CIDAssert2(L"Unknown static interface data type")
    }
    colAttrs.objAdd(adatTmp);

    //
    //  Figure out our range's attribute type. While we are at it, see if the values
    //  are equal to the natural range of the type.
    //
    tCIDLib::TBoolean bUseRange = kCIDLib::False;
    tCIDMData::EAttrTypes eRType;
    if (m_eType == tCQCKit::EFldTypes::Card)
    {
        eRType = tCIDMData::EAttrTypes::Card;
        bUseRange = (m_f8RangeMin != 0) || (m_f8RangeMax != kCIDLib::c4MaxCard);
    }
     else if (m_eType == tCQCKit::EFldTypes::Float)
    {
        eRType = tCIDMData::EAttrTypes::Float;
        bUseRange = (m_f8RangeMin != kCIDLib::f8MinFloat)
                    || (m_f8RangeMax != kCIDLib::f8MaxFloat);
    }
     else if (m_eType == tCQCKit::EFldTypes::Int)
    {
        eRType = tCIDMData::EAttrTypes::Int;
        bUseRange = (m_f8RangeMin != kCIDLib::i4MinInt)
                    || (m_f8RangeMax != kCIDLib::i4MaxInt);
    }

    //
    //  We add a faux attribute to make it easy for the user to enable/disable range
    //  control. If they disable, we just set the values back to their max values and
    //  disable those controls.
    //
    colAttrs.objPlace
    (
        L"Use Range"
        , kCQCIntfEng::strAttr_Static_UseRange
        , tCIDMData::EAttrTypes::Bool
    ).SetBool(bUseRange);


    // The type of the min/max values depend on the currently selected type
    TAttrData adatMin;
    TAttrData adatMax;

    adatMin.Set
    (
        L"Min Value"
        , kCQCIntfEng::strAttr_Static_Min
        , eRType
        , bUseRange ? tCIDMData::EAttrEdTypes::InPlace : tCIDMData::EAttrEdTypes::None
    );

    adatMax.Set
    (
        L"Max Value"
        , kCQCIntfEng::strAttr_Static_Max
        , eRType
        , bUseRange ? tCIDMData::EAttrEdTypes::InPlace : tCIDMData::EAttrEdTypes::None
    );

    if (m_eType == tCQCKit::EFldTypes::Card)
    {
        adatMin.SetCard(tCIDLib::TCard4(m_f8RangeMin));
        adatMax.SetCard(tCIDLib::TCard4(m_f8RangeMax));
    }
     else if (m_eType == tCQCKit::EFldTypes::Float)
    {
        adatMin.SetFloat(m_f8RangeMin);
        adatMax.SetFloat(m_f8RangeMax);
    }
     else if (m_eType == tCQCKit::EFldTypes::Int)
    {
        adatMin.SetInt(tCIDLib::TInt4(m_f8RangeMin));
        adatMax.SetInt(tCIDLib::TInt4(m_f8RangeMax));
    }

    colAttrs.objAdd(adatMin);
    colAttrs.objAdd(adatMax);
}


// Read in our persistent data
tCIDLib::TVoid
MCQCIntfStaticIntf::ReadInStatic(TBinInStream& strmToReadFrom)
{
    // Our stuff shuold start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_StaticIntf::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString(L"MCQCIntfStaticIntf")
        );
    }

    strmToReadFrom  >> m_f8RangeMax
                    >> m_f8RangeMin
                    >> m_eType;

    // And it should all end with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Sanity check the values
    if (m_f8RangeMax < m_f8RangeMin)
    {
        m_f8RangeMax = 0;
        m_f8RangeMin = 100;
    }
}


tCIDLib::TVoid
MCQCIntfStaticIntf::SetStaticAttr(          TAttrEditWnd&   wndAttrEd
                                    , const TAttrData&      adatNew
                                    , const TAttrData&      adatOld)
{
    if ((adatNew.strId() == kCQCIntfEng::strAttr_Static_UseRange)
    ||  (adatNew.strId() == kCQCIntfEng::strAttr_Static_Type))
    {
        tCIDLib::TBoolean bUseLimits = kCIDLib::True;
        if (adatNew.strId() == kCQCIntfEng::strAttr_Static_Type)
        {
            // Force the limits off so that we get new limits set below
            bUseLimits = kCIDLib::False;

            if (adatNew.strValue() == L"Card")
                m_eType = tCQCKit::EFldTypes::Card;
            else if (adatNew.strValue() == L"Float")
                m_eType = tCQCKit::EFldTypes::Float;
            else if (adatNew.strValue() == L"Int")
                m_eType = tCQCKit::EFldTypes::Int;
            else
            {
                #if CID_DEBUG_ON
                CIDAssert2(TString::strConcat(adatNew.strValue(), L" is not a valid static data type"))
                #endif
            }

            // The editor window will update the type of the min/max attributes to match
        }
         else
        {
            // We are changing the actual use range attribute, so remember that value
            bUseLimits = adatNew.bVal();
        }

        //
        //  If they disable, then we push the range back to its full range for
        //  the current type.
        //
        if (!bUseLimits)
        {
            if (m_eType == tCQCKit::EFldTypes::Card)
            {
                m_f8RangeMin = tCIDLib::TFloat8(0);
                m_f8RangeMax = tCIDLib::TFloat8(kCIDLib::c4MaxCard);
            }
             else if (m_eType == tCQCKit::EFldTypes::Float)
            {
                m_f8RangeMin = kCIDLib::f8MinFloat;
                m_f8RangeMax = kCIDLib::f8MaxFloat;
            }
             else if (m_eType == tCQCKit::EFldTypes::Int)
            {
                m_f8RangeMin = tCIDLib::TFloat8(kCIDLib::i4MinInt);
                m_f8RangeMax = tCIDLib::TFloat8(kCIDLib::i4MaxInt);
            }
        }
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Static_Min)
    {
        if (m_eType == tCQCKit::EFldTypes::Card)
            m_f8RangeMin = tCIDLib::TFloat8(adatNew.c4Val());
        else if (m_eType == tCQCKit::EFldTypes::Float)
            m_f8RangeMin = adatNew.f8Val();
        else if (m_eType == tCQCKit::EFldTypes::Int)
            m_f8RangeMin = tCIDLib::TFloat8(adatNew.i4Val());
    }
     else if (adatNew.strId() == kCQCIntfEng::strAttr_Static_Max)
    {
        if (m_eType == tCQCKit::EFldTypes::Card)
            m_f8RangeMax = tCIDLib::TFloat8(adatNew.c4Val());
        else if (m_eType == tCQCKit::EFldTypes::Float)
            m_f8RangeMax = adatNew.f8Val();
        else if (m_eType == tCQCKit::EFldTypes::Int)
            m_f8RangeMax = tCIDLib::TFloat8(adatNew.i4Val());
    }
}


// Write out our persistent info
tCIDLib::TVoid
MCQCIntfStaticIntf::WriteOutStatic(TBinOutStream& strmToWriteTo) const
{
    // Stream our start marker and foramt version
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCIntfEng_StaticIntf::c2FmtVersion
                    << m_f8RangeMax
                    << m_f8RangeMin
                    << m_eType
                    << tCIDLib::EStreamMarkers::Frame;

}


// ---------------------------------------------------------------------------
//  MCQCIntfStaticIntf: Hidden constructors operators
// ---------------------------------------------------------------------------
MCQCIntfStaticIntf::MCQCIntfStaticIntf(const tCQCKit::EFldTypes eType) :

    m_eType(eType)
    , m_f8RangeMax(100)
    , m_f8RangeMin(0)
{
}

// We don't take the source values
MCQCIntfStaticIntf::MCQCIntfStaticIntf(const MCQCIntfStaticIntf& miwdgSrc) :

    m_eType(miwdgSrc.m_eType)
    , m_f8RangeMax(miwdgSrc.m_f8RangeMax)
    , m_f8RangeMin(miwdgSrc.m_f8RangeMin)
{
}

MCQCIntfStaticIntf&
MCQCIntfStaticIntf::operator=(const MCQCIntfStaticIntf& miwdgSrc)
{
    if (this != &miwdgSrc)
    {
        m_eType      = miwdgSrc.m_eType;
        m_f8RangeMax = miwdgSrc.m_f8RangeMax;
        m_f8RangeMin = miwdgSrc.m_f8RangeMin;
    }
    return *this;
}

