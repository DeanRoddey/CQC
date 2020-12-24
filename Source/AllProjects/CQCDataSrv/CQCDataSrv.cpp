//
// FILE NAME: CQCDataSrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/23/2002
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
#include    "CQCDataSrv.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCDataSrvMainThread"
        , TMemberFunc<TFacCQCDataSrv>(&facCQCDataSrv, &TFacCQCDataSrv::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare the one global object, our facility object
// ----------------------------------------------------------------------------
TFacCQCDataSrv  facCQCDataSrv;
