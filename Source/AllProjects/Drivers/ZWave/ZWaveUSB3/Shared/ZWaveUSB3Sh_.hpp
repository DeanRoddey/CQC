//
// FILE NAME: ZWaveUSB3Sh_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This is the internal facility header. It defines and includes stuff that
//  is only required internally.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "ZWaveUSB3Sh.hpp"


// ---------------------------------------------------------------------------
//  Bring in any Z-Wave includes we need
// ---------------------------------------------------------------------------
#define     ZW_CONTROLLER_STATIC 1
#include    <ZW_typedefs.h>
#include    <ZW_controller_api.h>
#include    <ZW_transport_api.h>
#include    <ZW_classcmd.h>
#include    <ZW_SerialAPI.h>


// ---------------------------------------------------------------------------
//  Bring in stuff we only need internally
// ---------------------------------------------------------------------------
#include    "CIDImage.hpp"
#include    "CIDEncode.hpp"


// ---------------------------------------------------------------------------
//  Internal constants
// ---------------------------------------------------------------------------
namespace kZWaveUSB3Sh_
{
    // -----------------------------------------------------------------------
    //  The default transmit options if not overridden
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1       c1DefTransOpts = TRANSMIT_OPTION_AUTO_ROUTE
                                                 | TRANSMIT_OPTION_EXPLORE;
}


// ---------------------------------------------------------------------------
//  Our own internal only headers
// ---------------------------------------------------------------------------

//
//  Command class implementation derivatives. These are created and managed by the
//  unit info class, so no need for them to be public.
//
#include    "ZWaveUSB3Sh_CCImpl_Basic_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_Battery_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_BinSensor_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_BinSwitch_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_CentralScene_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_CSwitch_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_DoorLock_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_MLSensor_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_MLSwitch_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_Notification_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_Scene_.hpp"
#include    "ZWaveUSB3Sh_CCImpl_Thermostat_.hpp"


//
//  Actual unit handler derivatives, generic and model specific ones. These are created
//  and managed here by the unit info class, so no need for them to be public.
//
#include    "ZWaveUSB3Sh_Unit_AeonBadMF_.hpp"
#include    "ZWaveUSB3Sh_Unit_BinNotSensor_.hpp"
#include    "ZWaveUSB3Sh_Unit_Controller_.hpp"
#include    "ZWaveUSB3Sh_Unit_Dimmer_.hpp"
#include    "ZWaveUSB3Sh_Unit_DualBinSensor_.hpp"
#include    "ZWaveUSB3Sh_Unit_GenBinSensor_.hpp"
#include    "ZWaveUSB3Sh_Unit_GenDimmer_.hpp"
#include    "ZWaveUSB3Sh_Unit_GenLightSwitch_.hpp"
#include    "ZWaveUSB3Sh_Unit_GenSceneAct_.hpp"
#include    "ZWaveUSB3Sh_Unit_GenRGBW_.hpp"
#include    "ZWaveUSB3Sh_Unit_LightSwitch_.hpp"
#include    "ZWaveUSB3Sh_Unit_Lock_.hpp"
#include    "ZWaveUSB3Sh_Unit_MChCombo_.hpp"
#include    "ZWaveUSB3Sh_Unit_NChOutlet_.hpp"
#include    "ZWaveUSB3Sh_Unit_NMCMSensor_.hpp"
#include    "ZWaveUSB3Sh_Unit_Outlet_.hpp"
#include    "ZWaveUSB3Sh_Unit_TempSP_.hpp"
#include    "ZWaveUSB3Sh_Unit_Thermostat_.hpp"
