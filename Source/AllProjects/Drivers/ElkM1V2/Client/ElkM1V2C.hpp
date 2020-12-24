//
// FILE NAME: ElkM1V2C.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2005
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
//  This is the main header file for the Elk M1 client driver facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"
#include    "CQCIGKit.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TElkM1V2CWnd;


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kElkM1V2C
{
    // The maximum numbers of things that the Elk can provide
    const   tCIDLib::TCard4 c4MaxAreas(8);
    const   tCIDLib::TCard4 c4MaxCounters(16);
    const   tCIDLib::TCard4 c4MaxLoads(256);
    const   tCIDLib::TCard4 c4MaxOutputs(208);
    const   tCIDLib::TCard4 c4MaxThermos(16);
    const   tCIDLib::TCard4 c4MaxThermoCpls(48);
    const   tCIDLib::TCard4 c4MaxVolts(208);
    const   tCIDLib::TCard4 c4MaxZones(208);

    // The version of the config data we write out
    const   tCIDLib::TCard4 c4ConfigVer = 1;
}



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tElkM1V2C
{
    enum class EBaseValRes
    {
        OK
        , Name
        , Id
    };


    // The types of items, index into the types list box
    enum class EItemTypes
    {
        Area
        , Counter
        , Load
        , Output
        , Thermo
        , ThermoCpl
        , Volt
        , Zone

        , Count
        , Min       = Area
        , Max       = Zone
    };


    // Zone types
    enum class EZoneTypes
    {
        Motion
        , Security
    };
}



// ---------------------------------------------------------------------------
//  Include the core public headers that have stuff that might need to be
//  seen by the types namespace below.
// ---------------------------------------------------------------------------
#include    "ElkM1V2C_MessageIds.hpp"
#include    "ElkM1V2C_ErrorIds.hpp"
#include    "ElkM1V2C_ResourceIds.hpp"
#include    "ElkM1V2C_Type.hpp"

#include    "ElkM1V2C_ItemInfo.hpp"
#include    "ElkM1V2C_LimInfo.hpp"

#include    "ElkM1V2C_AreaInfo.hpp"
#include    "ElkM1V2C_CounterInfo.hpp"
#include    "ElkM1V2C_LoadInfo.hpp"
#include    "ElkM1V2C_OutputInfo.hpp"
#include    "ElkM1V2C_ThermoInfo.hpp"
#include    "ElkM1V2C_ThermoCplInfo.hpp"
#include    "ElkM1V2C_VoltInfo.hpp"
#include    "ElkM1V2C_ZoneInfo.hpp"


// ---------------------------------------------------------------------------
//  Slip in some more types
// ---------------------------------------------------------------------------
namespace tElkM1V2C
{
    //
    //  Though we give up type safety, we look at all of the lists of configurable
    //  items as by ref lists of the base item info class. This vastly reduces
    //  the amount of code, since so many things can then be done generically for
    //  all types.
    //
    typedef TRefVector<TItemInfo>   TItemList;

    //
    //  And then we have another vector of these item lists. We add one item list
    //  per item type. Can't use an array or even a by value vector here here, even
    //  though the size of this one will be fixed, since we need to control the
    //  construction of the individual lists.
    //
    typedef TRefVector<TItemList, EItemTypes>   TListList;
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kElkM1V2C
{
    // A lookup table for max ids for a given type
    const tCIDLib::TCard4 ac4MaxIdVals[] =
    {
        kElkM1V2C::c4MaxAreas
        , kElkM1V2C::c4MaxCounters
        , kElkM1V2C::c4MaxLoads
        , kElkM1V2C::c4MaxOutputs
        , kElkM1V2C::c4MaxThermos
        , kElkM1V2C::c4MaxThermoCpls
        , kElkM1V2C::c4MaxVolts
        , kElkM1V2C::c4MaxZones
    };
    static const TEArray<tCIDLib::TCard4, tElkM1V2C::EItemTypes, tElkM1V2C::EItemTypes::Count>
    ac4MaxIds(ac4MaxIdVals);


    //
    //  The names we show above the item list box as each type is selected.
    //
    //  Keep in sync with the item types enum above
    //
    const tCIDLib::TCh* const apszTypeVals[] =
    {
        L"Areas"
        , L"Counters"
        , L"Lights"
        , L"Outputs"
        , L"Thermos"
        , L"Thermocouples"
        , L"Voltages"
        , L"Zones"
    };
    static const TEArray<const tCIDLib::TCh*, tElkM1V2C::EItemTypes, tElkM1V2C::EItemTypes::Count>
    apszTypes(apszTypeVals);

    //
    //  Similar to above but used to output the start/end block lines when we format
    //  out config to send to the driver.
    //
    const tCIDLib::TCh* const apszBlockVals[] =
    {
        L"Areas"
        , L"Counters"
        , L"Loads"
        , L"Outputs"
        , L"Thermos"
        , L"ThermoCpls"
        , L"Volts"
        , L"Zones"
    };
    static const TEArray<const tCIDLib::TCh*, tElkM1V2C::EItemTypes, tElkM1V2C::EItemTypes::Count>
    apszBlocks(apszBlockVals);
};


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "ElkM1V2C_EdGenDlg.hpp"
#include    "ElkM1V2C_EdGenLimDlg.hpp"
#include    "ElkM1V2C_EdLoadDlg.hpp"
#include    "ElkM1V2C_EdVoltDlg.hpp"
#include    "ElkM1V2C_EdZoneDlg.hpp"
#include    "ElkM1V2C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facElkM1V2C();



// ---------------------------------------------------------------------------
//  Enum tricks
// ---------------------------------------------------------------------------
StdEnumTricks(tElkM1V2C::EItemTypes)

