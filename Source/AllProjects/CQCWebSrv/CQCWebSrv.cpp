//
// FILE NAME: CQCWebSrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This is the main implementation file of the CQCWebSrv facility. It does
//  the usual things of declaring the program entry point and the facility
//  object.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrv.hpp"


// ----------------------------------------------------------------------------
//  Main module magic macro
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"MainCQCWebSrvThread"
        , TMemberFunc<TFacCQCWebSrv>(&facCQCWebSrv, &TFacCQCWebSrv::eMainThread)
    )
)


// ----------------------------------------------------------------------------
//  Declare the one global object, our facility object
// ----------------------------------------------------------------------------
TFacCQCWebSrv facCQCWebSrv;


