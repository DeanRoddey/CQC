//
// FILE NAME: WeatherChS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2004
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
//  This is the main file for the Weather Channel data feed driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDNet.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "WeatherChS_ErrorIds.hpp"
#include    "WeatherChS_MessageIds.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kWeatherChS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"WeatherChS";


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4       c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The number of days of forecast data we get
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4ForecastDays  = 5;
}



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tWeatherChS
{
    //
    //  We keep an array of fields for the forecast day fields, and for each
    //  day we store the field ids in this order.
    //
    enum class EFDayFlds
    {
        CondTextD
        , CondTextN
        , High
        , IconD
        , IconN
        , Low
        , Stamp
        , PerPercipD
        , PerPercipN

        , Count
        , Min = 0
        , Max = PerPercipN
    };

    // Set up a nested set of enum friend arrays
    using TIdList = TFundArray<tCIDLib::TCard4, EFDayFlds>;
    using TDayList = TFundArray<TIdList, tCIDLib::TCard4>;
}



// ---------------------------------------------------------------------------
//  These need to see the consts/types above
// ---------------------------------------------------------------------------
#include    "WeatherChS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facWeatherChS();

