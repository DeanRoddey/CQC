//
// FILE NAME: IRManS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2002
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
//  This is the main file for the IRMan server side driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCIR.hpp"


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kIRManS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"IRManS";


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.) For all Charmed Quark drivers, their version
    //  is set to the product release version, for simplicity. So we set them
    //  to magic values provided by the CIDBuild build utility.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "IRManS_ErrorIds.hpp"
#include    "IRManS_MessageIds.hpp"
#include    "IRManS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facIRManS();

