//
// FILE NAME: CQCSrvDrvTI.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2018
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
//  This is the main cpp file of the facility. It provides the lazy eval
//  method for our facility object, and some other odds and ends.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSrvDrvTI_.hpp"



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCSrvDrvTI& facCQCSrvDrvTI()
{
    static TFacCQCSrvDrvTI* pfacThis = new TFacCQCSrvDrvTI();
    return *pfacThis;
}

