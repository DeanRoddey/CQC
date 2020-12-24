//
// FILE NAME: CQCNSShell.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This is the main implementation file of the CQCNSShell facility. It does
//  the usual things of declaring the program entry point and the facility
//  object.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCNSShell.hpp"


// ---------------------------------------------------------------------------
//  Magic startup macro
// ---------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"CQCNSShellMainThread"
        , TMemberFunc<TFacCQCNSShell>(&facCQCNSShell, &TFacCQCNSShell::eMainThread)
    )
)



// ---------------------------------------------------------------------------
//  Declare our one global object
// ---------------------------------------------------------------------------
TFacCQCNSShell facCQCNSShell;


