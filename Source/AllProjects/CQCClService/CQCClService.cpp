//
// FILE NAME: CQCClService.cpp
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
// CAVEATS/GOTCHAS:
//
//  This is the main implementation file of the service. We provide the magic
//  kickoff macro and the facility lazy eval method.
//
//  In debug mode, we use the standard program kickoff macro unless explicitly
//  modified for testing in real service mode in a debug build. This way we
//  can run the service as a standard app for most debugging.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCClService.hpp"


// ---------------------------------------------------------------------------
//  Magic startup macro for a shell based apps, though we also allow for non-shell
//  form for debugging purposes.
// ---------------------------------------------------------------------------
#if defined(CLSRV_NON_SERVICE_MODE)
CIDLib_MainModule
(
    TThread
    (
        L"CQCClSrvMainThread"
        , TMemberFunc<TFacCQCClService>
      (
            &facCQCClService(), &TFacCQCClService::eServiceThread
          )
    )
)
#else
CIDLib_MainService(L"CQCClService", TCQCClServiceService, kCIDLib::False)
#endif




// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
TFacCQCClService& facCQCClService()
{
    static TFacCQCClService* pfacThis = new TFacCQCClService();
    return *pfacThis;
}

