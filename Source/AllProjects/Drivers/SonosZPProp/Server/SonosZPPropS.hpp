//
// FILE NAME: SonosZPPropS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/14/2012
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
//  This is the main header file for a generic UPnP media renderer driver.
//  It should work with any compliant UPnP renderer, in theory. A separate
//  manifest is created for each variation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDUPnP.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "SonosZPPropS_ErrorIds.hpp"
#include    "SonosZPPropS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tSonosZPPropS
{
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "SonosZPPropS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kSonosZPPropS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName = L"SonosZPPropS";



    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facSonosZPPropS();

