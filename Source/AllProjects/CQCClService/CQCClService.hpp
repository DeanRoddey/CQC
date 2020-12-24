//
// FILE NAME: CQCClService.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/20/2013
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
//  This is the main header of the CQC client serivice.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDZLib.hpp"
#include    "CIDImgFact.hpp"
#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Bring in the Windows specific service support stuff
// ---------------------------------------------------------------------------
#include    "CIDKernel_ServiceWin32.hpp"


// ---------------------------------------------------------------------------
//  And now sub-include our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQCClService_ErrorIds.hpp"
#include    "CQCClService_MessageIds.hpp"
#include    "CQCClService_ClSrvServerBase.hpp"
#include    "CQCClService_ClSrvImpl.hpp"
#include    "CQCClService_ServiceHandler.hpp"
#include    "CQCClService_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern TFacCQCClService& facCQCClService();


// ---------------------------------------------------------------------------
//  Normally, in debug mode we build this program to run as a non-service, so we can
//  just run it normally and debug and such. But it can be forced to run as a service
//  in debug mode if needed, or vice versa, to run it normally in production builds.
//  Just temporarily get rid of the debug mode conditional check.
// ---------------------------------------------------------------------------
#if CID_DEBUG_ON
#define CLSRV_NON_SERVICE_MODE 1
#endif
