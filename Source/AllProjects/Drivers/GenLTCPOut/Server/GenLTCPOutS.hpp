//
// FILE NAME: GenLTCPOutS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2013
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
//  This is the main file for the generic listening TCP/IP passthrough server
//  side driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDSock.hpp"

#include    "CIDOrbUC.hpp"

#include    "CIDEncode.hpp"

#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kGenLTCPOutS
{
    // -----------------------------------------------------------------------
    //  The base name of our server side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"GenLTCPOutS";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;
}


namespace tGenLTCPOutS
{
    // -----------------------------------------------------------------------
    //  The kind of text decoration that the use asked us to add to the
    //  outgoing text msgs.
    // -----------------------------------------------------------------------
    enum class EDecors
    {
        None
        , CR
        , LF
        , CRLF
        , STXETX
    };
};


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "GenLTCPOutS_ErrorIds.hpp"
#include    "GenLTCPOutS_MessageIds.hpp"
#include    "GenLTCPOutS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facGenLTCPOutS();

