//
// FILE NAME: CQSLSpeechS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2006
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
//  This is the main file for the driver
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
#include    "CQCAct.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"
#include    "CQCRemBrws.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQSLSpeechS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"CQSLSpeechS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The maximum number of queued speech items we'll allow before we start
    //  rejecting them.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxQItems     = 64;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "CQSLSpeechS_ErrorIds.hpp"
#include    "CQSLSpeechS_MessageIds.hpp"
#include    "CQSLSpeechS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facCQSLSpeechS();

