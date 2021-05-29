//
// FILE NAME: CQCIntfEng_AreaFill.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/07/2002
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
//  This file implements the class that understands the grunt work of
//  doing an area fill.
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
#include    "CQCIntfEng_AreaFill.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
AdvRTTIDecls(TCQCIntfAreaFill,TCQCIntfWidget)



// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCIntfEng_AreaFill
    {
        // -----------------------------------------------------------------------
        //  Format versions
        //
        //  1.  Reset the version number as of 1.4
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2   c2FmtVersion    = 1;


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
//   CLASS: TCQCIntfAreaFill
//  PREFIX: iwdg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCIntfAreaFill: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCIntfAreaFill::TCQCIntfAreaFill() :

    TCQCIntfWidget
    (
        CQCIntfEng_AreaFill::eCapFlags
        , facCQCIntfEng().strMsg(kIEngMsgs::midWidget_AreaFill)
    )
{
    // So that it's initially visible, set a slight gradient fill
    rgbBgn(TRGBClr(0xFA, 0xFA, 0xFA));
    rgbBgn2(TRGBClr(0xD6, 0xD6, 0xD6));

    // Set our command help id for our family of widgets
    strCmdHelpId(L"/Reference/IntfWidgets/AreaColorFill");
}

TCQCIntfAreaFill::TCQCIntfAreaFill(const TCQCIntfAreaFill& iwdgToCopy) :

    TCQCIntfWidget(iwdgToCopy)
{
}

TCQCIntfAreaFill::~TCQCIntfAreaFill()
{
}


// ---------------------------------------------------------------------------
//  TCQCIntfAreaFill: Public operators
// ---------------------------------------------------------------------------
TCQCIntfAreaFill&
TCQCIntfAreaFill::operator=(const TCQCIntfAreaFill& iwdgToAssign)
{
    if (this == &iwdgToAssign)
        return *this;

    TParent::operator=(iwdgToAssign);

    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCIntfAreaFill: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCIntfAreaFill::bIsSame(const TCQCIntfWidget& iwdgSrc) const
{
    // Currently nothing of our own so just return our parent
    return TParent::bIsSame(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfAreaFill::CopyFrom(const TCQCIntfWidget& iwdgSrc)
{
    ValidateTypes(iwdgSrc.clsIsA());
    *this = static_cast<const TCQCIntfAreaFill&>(iwdgSrc);
}


tCIDLib::TVoid
TCQCIntfAreaFill::Initialize(TCQCIntfContainer* const   piwdgParent
                            , TDataSrvClient&           dsclInit
                            , tCQCIntfEng::TErrList&    colErrs)
{
    // For now, we just pass it on to our parent
    TParent::Initialize(piwdgParent, dsclInit, colErrs);
}


tCIDLib::TVoid
TCQCIntfAreaFill::QueryWdgAttrs(tCIDMData::TAttrList&   colAttrs
                                , TAttrData&            adatTmp) const
{
    TParent::QueryWdgAttrs(colAttrs, adatTmp);

    // Remove the ones we don't use and set appropriate names for the others
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Fgn2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra1);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra2);
    facCIDMData().RemoveAttr(colAttrs, kCQCIntfEng::strAttr_Clr_Extra3);
}


TSize TCQCIntfAreaFill::szDefaultSize() const
{
    return TSize(32, 32);
}




tCIDLib::TVoid
TCQCIntfAreaFill::Validate( const   TCQCFldCache&           cfcData
                            ,       tCQCIntfEng::TErrList&  colErrs
                            ,       TDataSrvClient&         dsclVal) const
{
    // Just call our parent for now
    TParent::Validate(cfcData, colErrs, dsclVal);
}


// ---------------------------------------------------------------------------
//  TCQCIntfAreaFill: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCIntfAreaFill::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCIntfEng_AreaFill::c2FmtVersion))
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

    // We don't have any data of our own right now

    // And it should all end with an end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCIntfAreaFill::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Do our start marker and format verson
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCIntfEng_AreaFill::c2FmtVersion;

    // Call our parent, so he can stream his stuff out
    TParent::StreamTo(strmToWriteTo);

    // We don't have any data at this time, so just stream out the end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::EndObject;
}


tCIDLib::TVoid
TCQCIntfAreaFill::Update(       TGraphDrawDev&  gdevTarget
                        , const TArea&          areaInvalid
                        ,       TGUIRegion&     grgnClip)
{
    // All we need to do is call our parent for now
    TParent::Update(gdevTarget, areaInvalid, grgnClip);
}

