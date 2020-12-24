//
// FILE NAME: CQCMEng.hpp
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
//  This facility exposes some CQC functionality via CML classes. These are used by
//  CQC actions, macros, CML based drivers, etc...
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
#include    "CIDMacroEng.hpp"
#include    "CQCKit.hpp"



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCMENG)
#define CQCMENGEXPORT CID_DLLEXPORT
#else
#define CQCMENGEXPORT CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCMEng_ErrorIds.hpp"
#include    "CQCMEng_MessageIds.hpp"
#include    "CQCMEng_MEngErrHandler.hpp"
#include    "CQCMEng_ClassManager.hpp"

#include    "CQCMEng_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method. We just need something for
//  logging msgs and errors, since we have no real functionality of our own, we
//  just exist to enable servers. So we use the base facility class.
// ---------------------------------------------------------------------------
extern CQCMENGEXPORT TFacCQCMEng& facCQCMEng();

