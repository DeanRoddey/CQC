//
// FILE NAME: GenLTrigS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/15/2013
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
//  This is the main file for the generic listener trigger server side driver.
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
namespace tGenLTrigS
{
    // -----------------------------------------------------------------------
    //  The types of line terminations we support.
    // -----------------------------------------------------------------------
    enum class ETermTypes
    {
        CR
        , LF
        , CRLF
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kGenLTrigS
{
    // -----------------------------------------------------------------------
    //  The base name of our server side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"GenLTrigS";


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
#include    "GenLTrigS_ErrorIds.hpp"
#include    "GenLTrigS_MessageIds.hpp"
#include    "GenLTrigS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facGenLTrigS();


