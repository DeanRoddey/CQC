//
// FILE NAME: CQCAppShell.cpp
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
//  This is the main implementation file of the CQCApShell facility. It does
//  the usual things of declaring the program entry point and the facility
//  object.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShell.hpp"


// ---------------------------------------------------------------------------
//  Magic startup macro for a shell based app. We tell it the type of our
//  shell services handler.
// ---------------------------------------------------------------------------
CIDLib_MainService(L"CQCAppShell", TCQCAppShellService, kCIDLib::False)



// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
TFacCQCAppShell& facCQCAppShell()
{
    static TFacCQCAppShell* pfacThis = new TFacCQCAppShell();
    return *pfacThis;
}
