//
// FILE NAME: CQCEvMonEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This is the main header of the CQC system event monitor engine facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCEVMONENG)
#define CQCEVMONENGEXP CID_DLLEXPORT
#else
#define CQCEVMONENGEXP CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDMacroEng.hpp"
#include    "CQCKit.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCEvCl.hpp"



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCEvMonEng
{
    using TPollList = TVector<TCQCFldPollInfo>;
}


// ---------------------------------------------------------------------------
//  Forward refs
// ---------------------------------------------------------------------------
class TCQCEvMonEng;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCEvMonEng_ErrorIds.hpp"
#include    "CQCEvMonEng_MessageIds.hpp"
#include    "CQCEvMonEng_MonBaseClass_.hpp"
#include    "CQCEvMonEng_Monitor.hpp"
#include    "CQCEvMonEng_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCEVMONENGEXP TFacCQCEvMonEng& facCQCEvMonEng();


