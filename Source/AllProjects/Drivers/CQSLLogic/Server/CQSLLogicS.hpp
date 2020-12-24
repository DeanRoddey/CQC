//
// FILE NAME: CQSLLogicS.hpp
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
//  This is the main file for the CQSL Logic Server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"

#include    "CQLogicSh.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQSLLogicS
{
    // -----------------------------------------------------------------------
    //  The base name of our facilities
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName = L"CQSLLogicS";
}


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQSLLogicS
{
    // -----------------------------------------------------------------------
    //  Returned from the driver method that does the field updates, to
    //  indicate the status of the operation.
    // -----------------------------------------------------------------------
    enum class EUpdateRes
    {
        Success
        , Resync
        , Error
    };
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "CQSLLogicS_ErrorIds.hpp"
#include    "CQSLLogicS_MessageIds.hpp"
#include    "CQSLLogicS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facCQSLLogicS();


