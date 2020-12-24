//
// FILE NAME: WundergroundS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2011
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
//  This is the main file for the Weather Underground data feed driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDNet.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "WundergroundS_ErrorIds.hpp"
#include    "WundergroundS_MessageIds.hpp"




// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tWundergroundS
{
    //
    //  We keep an array of fields for the forecast day fields, and for each
    //  day we store the field ids in this order, so make sure that you add
    //  the fields to the list in this order!
    //
    enum EFDayFlds
    {
        EFDay_CondText
        , EFDay_High
        , EFDay_Icon
        , EFDay_Low
        , EFDay_POP
        , EFDay_Stamp

        , EFDay_FldsPer
    };

    //
    //  We map the Wunderground icon names to the equivalent icon numbers
    //  in the Weather Channel set of images. This way, folks can use the
    //  same weather icon widget with this driver, and those images (and the
    //  alternate ones that others have created) are much better and transparent
    //  PNGs.
    //
    struct TIconMap
    {
        const tCIDLib::TCh* const   pszIconName;
        tCIDLib::TCard4             c4IconNum;
    };
}



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kWundergroundS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"WundergroundS";


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The number of forecast days
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4ForecastDays = 8;


    // -----------------------------------------------------------------------
    //  If we cannot connect to the server, we schedule another try in this
    //  many minutes. We aren't eating up a query here, since we never got
    //  to the server.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4QueryFailMins = 2;


    // -----------------------------------------------------------------------
    //  If we get a reply but we cannot parse it for some reason, we schedule
    //  another try in this many minutes. We've used up a query probably, so
    //  we want to be careful about these. We go to great lengths to deal with
    //  bad data, but you never know.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4ContentFailMins = 10;


    // -----------------------------------------------------------------------
    //  The icon name to number map. The 0th entry must be for the unknown
    //  icon.
    // -----------------------------------------------------------------------
    const tWundergroundS::TIconMap aMap[] =
    {
        { L"unknown", 48 }
      , { L"chanceflurries", 15 }
      , { L"chancerain", 40 }
      , { L"chancesleet", 11 }
      , { L"chancesnow", 16 }
      , { L"chancetstorms", 35 }
      , { L"clear", 32 }
      , { L"cloudy", 26 }
      , { L"flurries", 15 }
      , { L"fog", 20 }
      , { L"hazy", 21 }
      , { L"mostlycloudy", 28 }
      , { L"mostlysunny", 34 }
      , { L"partlycloudy", 30 }
      , { L"partlysunny", 34 }
      , { L"sleet", 11 }
      , { L"rain", 12 }
      , { L"snow", 14 }
      , { L"sunny", 32 }
      , { L"tstorms", 35 }
    };
    const tCIDLib::TCard4 c4IconCnt = tCIDLib::c4ArrayElems(aMap);
}



// ---------------------------------------------------------------------------
//  These need to see the consts/types above
// ---------------------------------------------------------------------------
#include    "WundergroundS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facWundergroundS();

