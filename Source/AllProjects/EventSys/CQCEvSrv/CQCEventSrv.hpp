//
// FILE NAME: CQCEventSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/18/2004
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
//  This is the main header of the CQC scheduled events server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDMacroEng.hpp"

#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCGWCl.hpp"
#include    "CQCEvCl.hpp"
#include    "CQCMEng.hpp"
#include    "CQCEvMonEng.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCAct.hpp"
#include    "CQCSrvFW.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our core headers in the needed order
// ---------------------------------------------------------------------------
#include    "CQCEventSrv_ErrorIds.hpp"
#include    "CQCEventSrv_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCEventSrv
{
}


// ---------------------------------------------------------------------------
//  Constants namespace
// ---------------------------------------------------------------------------
namespace kCQCEventSrv
{
    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvEventSrv   = L"CQCEventSrv";


    // -----------------------------------------------------------------------
    //  Related to the worker threads we start up to run our events
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4EvWorkerThreads     = 16;
}


// ---------------------------------------------------------------------------
//  And sub-include our higher level headers in the needed order
// ---------------------------------------------------------------------------
#include    "CQCEventSrv_ProtoServerBase.hpp"
#include    "CQCEventSrv_ProtoServerImpl.hpp"
#include    "CQCEventSrv_Shared.hpp"

#include    "CQCEventSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCEventSrv  facCQCEventSrv;


