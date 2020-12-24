//
// FILE NAME: CQCEvMonEng.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  method for our facility object, and any other equired odds and ends.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCEvMonEng_.hpp"



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCEvMonEng& facCQCEvMonEng()
{
    static TFacCQCEvMonEng* pfacThis = new TFacCQCEvMonEng();
    return *pfacThis;
}

