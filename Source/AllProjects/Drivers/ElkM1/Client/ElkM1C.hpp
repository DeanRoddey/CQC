//
// FILE NAME: ElkM1C.hpp
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




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CIDWUtils.hpp"

#include    "CQCIGKit.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TElkM1CWnd;


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kElkM1C
{
    // The maximum numbers of things that the Elk can provide
    const   tCIDLib::TCard4 c4MaxAreas(8);
    const   tCIDLib::TCard4 c4MaxOutputs(208);
    const   tCIDLib::TCard4 c4MaxPLCUnits(256);
    const   tCIDLib::TCard4 c4MaxTasks(32);
    const   tCIDLib::TCard4 c4MaxThermostats(16);
    const   tCIDLib::TCard4 c4MaxThermoCpls(48);
    const   tCIDLib::TCard4 c4MaxVoltages(208);
    const   tCIDLib::TCard4 c4MaxZones(208);

    // The version of the config data we write out
    const   tCIDLib::TCard4 c4ConfigVer = 3;
}


// ---------------------------------------------------------------------------
//  Include the core public headers that have stuff that might need to be
//  seen by the types namespace below.
// ---------------------------------------------------------------------------
#include    "ElkM1C_MessageIds.hpp"
#include    "ElkM1C_ErrorIds.hpp"
#include    "ElkM1C_ResourceIds.hpp"
#include    "ElkM1C_Type.hpp"
#include    "ElkM1C_VoltInfo.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tElkM1C
{
    // For tracking lists of things that are in or out of our configured stuff
    typedef TFundArray<tCIDLib::TBoolean>   TFlagArray;


    // For tracking the PLC module config
    enum class EPLCTypes
    {
        Unused
        , Binary
        , Dimmer
    };
    typedef TFundArray<EPLCTypes>       TPLCArray;


    // For tracking voltage configuration
    typedef TObjArray<TVoltInfo>   TVoltArray;
}


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "ElkM1C_VoltDlg.hpp"
#include    "ElkM1C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facElkM1C();


