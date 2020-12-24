//
// FILE NAME: CQCPollEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/25/2008
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
//  This is the main header of the CQC polling engine facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCPOLLENG)
#define CQCPOLLENGEXPORT    CID_DLLEXPORT
#else
#define CQCPOLLENGEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCPollEng
{
    // -----------------------------------------------------------------------
    //  Used as special stream markers
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1Present       = 0xDE;
    const tCIDLib::TCard1   c1NotPresent    = 0xCA;


    // -----------------------------------------------------------------------
    //  Used to hash the ids in a field link object
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4LinkHashMod   = 109;
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCPollEng_ErrorIds.hpp"
#include    "CQCPollEng_Shared.hpp"
#include    "CQCPollEng_ThisFacility.hpp"
#include    "CQCPollEng_Engine.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCPOLLENGEXPORT TFacCQCPollEng& facCQCPollEng();


