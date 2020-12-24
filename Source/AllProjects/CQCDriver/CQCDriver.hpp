//
// FILE NAME: CQCDriver.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/04/2000
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
//  This is the main header of the CQC driver stuff facility, that just holds
//  some driver related stuff we really wanted to get out of CQCKit, because
//  of constant full rebuilds every time we change something driver related.
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
#include    "CIDComm.hpp"
#include    "CIDSock.hpp"
#include    "CIDOrb.hpp"
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCDRIVER)
#define CQCDRIVEREXPORT    CID_DLLEXPORT
#else
#define CQCDRIVEREXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCDriver_ErrorIds.hpp"
#include    "CQCDriver_MessageIds.hpp"

#include    "CQCDriver_DriverBase.hpp"
#include    "CQCDriver_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCDRIVEREXPORT TFacCQCDriver& facCQCDriver();

