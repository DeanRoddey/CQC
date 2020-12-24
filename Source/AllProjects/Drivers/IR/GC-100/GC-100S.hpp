//
// FILE NAME: GC-100S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/04/2003
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
//  This is the main file for the RedRat2 server side driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCIR.hpp"



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tGC100
{
    // -----------------------------------------------------------------------
    //  The type of GC-100 modules we support
    // -----------------------------------------------------------------------
    enum class EModTypes
    {
        Unknown
        , Relay
        , IRBlaster
        , IRSensor
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kGC100
{
    // -----------------------------------------------------------------------
    //  The base name of our server side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"GC-100S";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "GC-100S_ErrorIds.hpp"
#include    "GC-100S_MessageIds.hpp"
#include    "GC-100S_AddrMapItem.hpp"
#include    "GC-100S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facGC100();


