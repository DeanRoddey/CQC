//
// FILE NAME: VantageQS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/02/2006
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
//  This is the main file for the HAI Omni ProII server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDSock.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tVantageQS
{
    //
    //  Everything is either in an enclosure or a station, and the type of
    //  address it has depends on which. Stuff in enclosures have 4 address
    //  values and stuff in stations have 3.
    //
    enum class EAddrTypes
    {
        Enclosure
        , Station
    };

    //
    //  Indicates the type of a load. We just have one class for load type
    //  items, and use a value of this type know what type of laod it is.
    //
    enum class ELoadTypes
    {
        Dimmer
        , Binary
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kVantageQS
{
    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  Our facility name
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszFacName = L"VantageQS";
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "VantageQS_ErrorIds.hpp"
#include    "VantageQS_MessageIds.hpp"
#include    "VantageQS_Item.hpp"
#include    "VantageQS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facVantageQS();


