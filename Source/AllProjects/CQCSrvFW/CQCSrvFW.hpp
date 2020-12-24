//
// FILE NAME: CQCSrvFW.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2019
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
//  This facility provides a framework for creating standard CQC servers, which
//  saves a lot of redundancy and makes it easie to apply new capabilities to
//  all servers. Of course some exist below CQC, so this is just for CQC servers.
//
//  It handles the standard admin interface type stuff, parameter parsing, init
//  and term stuff, shutdown request handling and so forth.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers that we need publically
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCSRVFW)
#define CQCSRVFWEXPORT CID_DLLEXPORT
#else
#define CQCSRVFWEXPORT CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCSrvFW_ErrorIds.hpp"
#include    "CQCSrvFW_MessageIds.hpp"
#include    "CQCSrvFW_Shared.hpp"
#include    "CQCSrvFW_SrvCore.hpp"


// ---------------------------------------------------------------------------
//  Our standard facility lazy eval method
// ---------------------------------------------------------------------------
extern TFacility& facCQCSrvFW();

