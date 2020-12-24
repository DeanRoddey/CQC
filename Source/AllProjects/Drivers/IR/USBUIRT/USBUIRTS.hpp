//
// FILE NAME: USBUIRTS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/12/2003
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
//  This is the main file for the USBUIRT server side driver.
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
namespace tUSBUIRTS
{
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kUSBUIRTS
{
    // -----------------------------------------------------------------------
    //  The base name of our server side driver and the support DLL.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"USBUIRTS";
    const tCIDLib::TCh* const   pszSupportDLL       = L"uuirtdrv.dll";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;


    // -----------------------------------------------------------------------
    //  To avoid sync issues, when we get a callback from the UIRT (which can
    //  happen at any time) we just post the data up to ourself as an async
    //  backdoor command. This is the SendCmd send type, and the UIRT string
    //  is the param.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszCmd_UIRTData     = L"UIRTData";
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "USBUIRTS_ErrorIds.hpp"
#include    "USBUIRTS_MessageIds.hpp"
#include    "USBUIRTS_Interface.hpp"
#include    "USBUIRTS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facUSBUIRTS();


