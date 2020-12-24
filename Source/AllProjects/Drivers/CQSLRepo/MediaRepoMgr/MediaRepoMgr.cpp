//
// FILE NAME: MediaRepoMgr.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/01/2006
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
//  This is the main header of the program. It brings in other stuff we need
//  and declares global stuff for the program, and contains the entry point.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr.hpp"


// ---------------------------------------------------------------------------
//  Do the magic main module code
// ---------------------------------------------------------------------------
CIDLib_CtrlsModule
(
    TThread
    (
        L"MainThread"
        , TMemberFunc<TFacMediaRepoMgr>(&facMediaRepoMgr, &TFacMediaRepoMgr::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare our global data, which is exported via the main header
// ----------------------------------------------------------------------------
TFacMediaRepoMgr facMediaRepoMgr;


