//
// FILE NAME: CQLogicSh.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/08/2009
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
//  This is the main implementation file for this facility, and just defines
//  a little facility global stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSh_.hpp"



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------

// The lazy eval function that faults in the facility object
TFacCQLogicSh& facCQLogicSh()
{
    static TFacCQLogicSh* pfacThis = new TFacCQLogicSh();
    return *pfacThis;
}

