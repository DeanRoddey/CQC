//
// FILE NAME: CQLogicSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2009
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
//  This is the main header of the logic server product. It does the usual
//  things that a CIDLib based facility header does.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"
#include    "CQCPollEng.hpp"
#include    "CQLogicSh.hpp"
#include    "CQCSrvFW.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQLogicSrv_ErrorIds.hpp"
#include    "CQLogicSrv_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQLogicSrv
{
    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvLogicSrv   = L"CQLogicSrv";
}


// ---------------------------------------------------------------------------
//  And now sub-include most of our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQLogicSrv_LSIntfServerBase.hpp"
#include    "CQLogicSrv_LSIntfImpl.hpp"


// ---------------------------------------------------------------------------
//  And now sub-include the rest of our headers
// ---------------------------------------------------------------------------
#include    "CQLogicSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQLogicSrv    facCQLogicSrv;

