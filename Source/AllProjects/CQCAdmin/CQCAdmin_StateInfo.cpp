//
// FILE NAME: CQCAdmin_StateInfo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/17/2002
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
//  This file implements persistent state storage classes for this application.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCCAppState,TCQCFrameState)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCClient_StateInfo
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Persistent version ids
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2AppFmtVersion   = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCCAppState
//  PREFIX: fst
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCCAppState: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCCAppState::TCQCCAppState()
{
}

TCQCCAppState::TCQCCAppState(const TArea& areaFrame) :

    TCQCFrameState(areaFrame)
{
}

TCQCCAppState::~TCQCCAppState()
{
}


// ---------------------------------------------------------------------------
//  TCQCCAppState: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCCAppState::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Call our parent first
    TParent::StreamFrom(strmToReadFrom);

    // Our stuff should start with a frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    //
    //  And next should be the format version. We only have one version
    //  now, so we just check that it matches that.
    //
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (c2FmtVersion != CQCClient_StateInfo::c2AppFmtVersion)
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

    // And get our stuff. We don't have any of our own at this time

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCCAppState::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // Call our parent first
    TParent::StreamTo(strmToWriteTo);

    //
    //  We don't have any info of our own yet, but put out the bookkeeping
    //  stuff so we can add it later.
    //
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCClient_StateInfo::c2AppFmtVersion
                    << tCIDLib::EStreamMarkers::EndObject;
}

