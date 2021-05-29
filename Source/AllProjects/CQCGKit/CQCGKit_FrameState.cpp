//
// FILE NAME: CQCGKit_FrameState.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2002
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
//  This file implements the the TCQCFrameState class, which is the base
//  class for a number of persistent state classes that are used to store
//  info about frame window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCFrameState,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCGKit_FrameState
    {
        constexpr tCIDLib::TCard2 c2OldFmtVersion   = 2;
        constexpr tCIDLib::TCard2 c2FmtVersion      = 3;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCFrameState
//  PREFIX: fst
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCFrameState: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCFrameState::TCQCFrameState() :

    m_areaFrame(facCIDCtrls().areaDefWnd())
    , m_ePosState(tCIDCtrls::EPosStates::Restored)
{
}

TCQCFrameState::TCQCFrameState(const TArea& areaFrame) :

    m_areaFrame(areaFrame)
    , m_ePosState(tCIDCtrls::EPosStates::Restored)
{
}

TCQCFrameState::TCQCFrameState( const   TArea&                  areaFrame
                                , const tCIDCtrls::EPosStates   ePosState) :
    m_areaFrame(areaFrame)
    , m_ePosState(ePosState)
{
}

TCQCFrameState::~TCQCFrameState()
{
}


// ---------------------------------------------------------------------------
//  TCQCFrameState: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TArea& TCQCFrameState::areaFrame() const
{
    return m_areaFrame;
}

const TArea& TCQCFrameState::areaFrame(const TArea& areaToSet)
{
    m_areaFrame = areaToSet;
    return m_areaFrame;
}


tCIDCtrls::EPosStates TCQCFrameState::ePosState() const
{
    return m_ePosState;
}

tCIDCtrls::EPosStates TCQCFrameState::ePosState(const tCIDCtrls::EPosStates eToSet)
{
    m_ePosState = eToSet;
    return m_ePosState;
}


tCIDLib::TVoid
TCQCFrameState::SetArea(const   TArea&                  areaToSet
                        , const tCIDCtrls::EPosStates   eToSet)
{
    m_ePosState = eToSet;
    m_areaFrame = areaToSet;
}


// We just get the info ourself from the passed window
tCIDLib::TVoid TCQCFrameState::SetFrom(const TFrameWnd& wndSrc)
{
    m_ePosState = wndSrc.ePosState(m_areaFrame);
}


tCIDLib::TVoid
TCQCFrameState::SetOrg( const   TPoint&                 pntToSet
                        , const tCIDCtrls::EPosStates   eToSet)
{
    m_ePosState = eToSet;
    m_areaFrame.SetOrg(pntToSet);
}


tCIDLib::TVoid
TCQCFrameState::SetSize(const   TSize&                  szToSet
                        , const tCIDCtrls::EPosStates   eToSet)
{
    m_ePosState = eToSet;
    m_areaFrame.SetSize(szToSet);
}


// ---------------------------------------------------------------------------
//  TCQCFrameState: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCFrameState::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Our stuff should start with a start object
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    //
    //  And next should be the format version. We have two legal versions
    //  right now. The old one had a minimized area, which we don't use any
    //  more.
    //
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if ((c2FmtVersion != CQCGKit_FrameState::c2FmtVersion)
    &&  (c2FmtVersion != CQCGKit_FrameState::c2OldFmtVersion))
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

    // And get our stuff out
    strmToReadFrom >> m_areaFrame;

    // If the old version, then eat the mininized area
    if (c2FmtVersion == CQCGKit_FrameState::c2OldFmtVersion)
    {
        TArea areaDummy;
        strmToReadFrom  >>  areaDummy;
    }
    strmToReadFrom >> m_ePosState;

    // Make sure we get the frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Adjust the area if needed, to keep it valid
    facCIDCtrls().ValidatePrevWindowArea(m_areaFrame);
}


tCIDLib::TVoid TCQCFrameState::StreamTo(TBinOutStream& strmToWriteTo) const
{
    //
    //  And dump our our stuff, with a frame and trailing end object marker.
    //  We also put out a format version, to support later automatic
    //  upgrading of the format.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCGKit_FrameState::c2FmtVersion
                    << m_areaFrame
                    << m_ePosState
                    << tCIDLib::EStreamMarkers::Frame;
}


