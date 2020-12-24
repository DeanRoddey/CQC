//
// FILE NAME: CQCAct.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/27/2015
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
//  This facility provides
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDEncode.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDRegX.hpp"
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCACT)
#define CQCACTEXPORT CID_DLLEXPORT
#else
#define CQCACTEXPORT CID_DLLIMPORT
#endif



// ---------------------------------------------------------------------------
//  Facility class constants
// ---------------------------------------------------------------------------
namespace kCQCAct
{
    // -----------------------------------------------------------------------
    //  The number of times that the system command target allows.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxSysTarTimers = 4;
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCAct_ErrorIds.hpp"
#include    "CQCAct_MessageIds.hpp"

#include    "CQCAct_EvSrvTarget.hpp"
#include    "CQCAct_StdCmdTargets.hpp"
#include    "CQCAct_SystemTarget.hpp"
#include    "CQCAct_StdActEngine.hpp"

#include    "CQCAct_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCACTEXPORT TFacCQCAct& facCQCAct();


