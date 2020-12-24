//
// FILE NAME: CQCGWCl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2013
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
//  This is the main header of the XML GW client facility.
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
#include    "CIDSock.hpp"
#include    "CIDSChan.hpp"
#include    "CIDZLib.hpp"
#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCGWCL)
#define CQCGWCLEXPORT    CID_DLLEXPORT
#else
#define CQCGWCLEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Facility types and constants
// ---------------------------------------------------------------------------
namespace tCQCGWCl
{
};



// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCGWCl_ErrorIds.hpp"
#include    "CQCGWCl_MessageIds.hpp"

#include    "CQCGWCl_GWStructs.hpp"
#include    "CQCGWCl_CppClient.hpp"
#include    "CQCGWCl_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCGWCLEXPORT TFacCQCGWCl& facCQCGWCl();

