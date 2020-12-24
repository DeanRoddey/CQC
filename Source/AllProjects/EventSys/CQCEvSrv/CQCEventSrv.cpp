//
// FILE NAME: CQCEventSrv.cpp
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
//  This is the main implementation file for this server. It just indicates
//  the magic entry point for the main thread. We start it on one of the
//  methods of the facility object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEventSrv.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCEventSrvMainThread"
        , TMemberFunc<TFacCQCEventSrv>
          (
            &facCQCEventSrv, &TFacCQCEventSrv::eMainThread
          )
    )
)


// ----------------------------------------------------------------------------
//  Declare the one global object, our facility object
// ----------------------------------------------------------------------------
TFacCQCEventSrv facCQCEventSrv;

