//
// FILE NAME: TandbergMXPS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/31/2008
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
//  This is the main file for the Tandberg MXP driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDEncode.hpp"
#include    "CIDXML.hpp"
#include    "CIDNet.hpp"
#include    "CIDComm.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "TandbergMXPS_ErrorIds.hpp"
#include    "TandbergMXPS_MessageIds.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kTandbergMXPS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"TandbergMXPS";


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The count of some elements in the XML we care about
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MaxCallItems      = 7;
    const tCIDLib::TCard4       c4MaxLines          = 3;
    const tCIDLib::TCard4       c4MaxMics           = 3;


    // -----------------------------------------------------------------------
    //  Some poll intervals
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctStatusPoll  = kCIDLib::enctOneSecond * 10;
}



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tTandbergMXPS
{
    // -----------------------------------------------------------------------
    //  Standard XML flags that we use
    // -----------------------------------------------------------------------
    const tCIDXML::EParseFlags eXMLFlags = tCIDXML::EParseFlags
    (
        tCIDXML::EParseFlags::Tags
        | tCIDXML::EParseFlags::Chars
    );
}



// ---------------------------------------------------------------------------
//  These need to see the consts/types above
// ---------------------------------------------------------------------------
#include    "TandbergMXPS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facTandbergMXPS();

