//
// FILE NAME: CQCRPortClient.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/05/2000
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
//  method for our facility object, and any other odds and ends that don't
//  belong to any particular class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortClient_.hpp"


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacCQCRPortClient& facCQCRPortClient()
{
    static TFacCQCRPortClient* pfacThis = new TFacCQCRPortClient();
    return *pfacThis;
}
