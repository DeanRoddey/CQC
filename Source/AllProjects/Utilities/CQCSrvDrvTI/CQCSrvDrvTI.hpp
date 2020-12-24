//
// FILE NAME: CQCSrvDrvTI.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/01/20`8
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
//  This is the main header of the CQC server driver test interface facility. This
//  guy creates a specialized derivative of the standard ORB interface that is used
//  by CQCServer to expose drivers and driver maintenance APIs. Our C++ driver test
//  harness and CML driver IDE both need to be able to test a driver, and possibly
//  a client side interface locally within themselves. That means they need to be
//  able to wire them together.
//
//  This facility provides a faux implementation of that server side ORB interface
//  that just forwards calls to a server side driver it has been given a pointer
//  to. Some stuff it doesn't have to implementat because they are related to driver
//  management, which isn't necessary for these testing/debugging programs.
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
#if     defined(PROJ_CQCSRVDRVTI)
#define CQCSRVDRVTIEXPORT    CID_DLLEXPORT
#else
#define CQCSRVDRVTIEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TCQCServerBase;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCSrvDrvTI_ErrorIds.hpp"
#include    "CQCSrvDrvTI_MessageIds.hpp"

#include    "CQCSrvDrvTI_TIServerBase.hpp"
#include    "CQCSrvDrvTI_TIImpl.hpp"

#include    "CQCSrvDrvTI_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCSRVDRVTIEXPORT TFacCQCSrvDrvTI& facCQCSrvDrvTI();

