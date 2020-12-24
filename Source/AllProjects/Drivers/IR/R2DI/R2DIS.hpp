//
// FILE NAME: R2DIS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2005
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
namespace tR2DIS
{
    // -----------------------------------------------------------------------
    //  The type of R2DI modules we support
    // -----------------------------------------------------------------------
    enum EModTypes
    {
        EModType_Unknown
        , EModType_Relay
        , EModType_IRBlaster
        , EModType_IRSensor
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kR2DIS
{
    // -----------------------------------------------------------------------
    //  The base name of our server side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"R2DIS";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The zone types driver prompt values
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszZType_Blaster    = L"Blaster";
    const tCIDLib::TCh* const   pszZType_Sensor     = L"Sensor";


    // -----------------------------------------------------------------------
    //  The count of zones this device supports.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4ZoneCount = 4;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "R2DIS_ErrorIds.hpp"
#include    "R2DIS_MessageIds.hpp"
#include    "R2DIS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facR2DIS();


