//
// FILE NAME: CQCMedia.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2005
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
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Force some platform libraries to be included
// ---------------------------------------------------------------------------
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "Strmiids.lib")
#pragma comment(lib, "wmvcore.lib")



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCMedia& facCQCMedia()
{
    static TFacCQCMedia* pfacThis = new TFacCQCMedia();
    return *pfacThis;
}
