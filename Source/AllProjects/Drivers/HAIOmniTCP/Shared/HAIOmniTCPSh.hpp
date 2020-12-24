//
// FILE NAME: HAIOmniTCPSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This is the public facility header. It defines and includes stuff that
//  is publically visible
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_HAIOMNITCPSH)
#define HAIOMNITCPSHEXPORT  CID_DLLEXPORT
#else
#define HAIOMNITCPSHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CIDLib.hpp"
#include "CIDOrbUC.hpp"

#include "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kHAIOmniTCPSh
{
    // -----------------------------------------------------------------------
    //  These are the numbers of all the things we deal with in the Omni.
    //  These are the worst cases, from the Omni ProII. We also support the
    //  regular IIe, can have fewer. We get the actual counts from the
    //  device. But we can use these to allocate storage areas and know it's
    //  enough to hold anything we'll be asked to handle.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxAreas(8);
    const tCIDLib::TCard4   c4MaxButtons(128);
    const tCIDLib::TCard4   c4MaxEnclosures(8);
    const tCIDLib::TCard4   c4MaxExps(8);
    const tCIDLib::TCard4   c4MaxMsgs(128);
    const tCIDLib::TCard4   c4MaxThermos(64);
    const tCIDLib::TCard4   c4MaxUnits(511);
    const tCIDLib::TCard4   c4MaxZones(176);

    //
    //  The version of the config data we write out. Version 2 is our new
    //  format as of 4.2.917.
    //
    const   tCIDLib::TCard4 c4ConfigVer = 2;


    // -----------------------------------------------------------------------
    //  The base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName = L"HAIOmniTCPC";
    const tCIDLib::TCh* const   pszServerFacName = L"HAIOmniTCPS";
}


// ---------------------------------------------------------------------------
//  Bring in our own public headers. Some others have to come in below after
//  the types namespace.
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  Public facility types
// ---------------------------------------------------------------------------
namespace tHAIOmniTCPSh
{
    //
    //  The types of named items in the Omni
    //
    //  Add new ones at the end! These are presisted.
    //
    enum EItemTypes
    {
        EItem_Unknown
        , EItem_Area
        , EItem_Button
        , EItem_Code
        , EItem_Message
        , EItem_Thermo
        , EItem_Unit
        , EItem_Zone
    };


    //
    //  Used in units to indicate what type of unit. Unused needs to be the
    //  first value, so that it comes out zero!
    //
    //  Add new ones at the end! These are presisted.
    //
    enum EUnitTypes
    {
        EUnit_Unused
        , EUnit_Binary
        , EUnit_Dimmer
        , EUnit_Flag
    };


    //
    //  Used in zones to indicate what type of zone. Unused needs to be the
    //  first value, so that it comes out zero! There are other zone types
    //  but these are the only ones we support for now.
    //
    //  Add new ones at the end! These are presisted.
    //
    enum EZoneTypes
    {
        EZone_Unused
        , EZone_Alarm
        , EZone_Humidity
        , EZone_Temp

        , EZone_Motion
    };
}


// ---------------------------------------------------------------------------
//  Provide streaming support for some enums
// ---------------------------------------------------------------------------
EnumBinStreamMacros(tHAIOmniTCPSh::EItemTypes)
EnumBinStreamMacros(tHAIOmniTCPSh::EUnitTypes)
EnumBinStreamMacros(tHAIOmniTCPSh::EZoneTypes)



// ---------------------------------------------------------------------------
//  Bring in some more headers that need the above types
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPSh_ErrorIds.hpp"
#include    "HAIOmniTCPSh_Item.hpp"
#include    "HAIOmniTCPSh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern HAIOMNITCPSHEXPORT TFacility& facHAIOmniTCPSh();

