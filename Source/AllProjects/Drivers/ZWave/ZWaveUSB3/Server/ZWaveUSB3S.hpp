//
// FILE NAME: ZWaveUSB3S.hpp
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
//  This is the main file for the Z-Wave USB3S controller driver.
//
// CAVEATS/GOTCHAS:
//
//  1)  We have our configuration, which includes info we got about our controller
//      and the user configuration. But we also have the network key we get if we are
//      included successfully. We keep the network key in the config server using the
//      standard per-driver storage mechanism. It's too complicated if it is stored
//      in the other configuration. They may want to delete the config at some point
//      and start again, or the file might get lost. If the key was there they would
//      have to remove and re-add our controller to get back in securely, and that
//      would change our controller's id, which would invalidate all associations
//      and on and on.
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_ZWAVEUSB3S)
#define ZWAVEUSB3SEXPORT   CID_DLLEXPORT
#else
#define ZWAVEUSB3SEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDZLib.hpp"
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "ZWaveUSB3Sh.hpp"



// ---------------------------------------------------------------------------
//  Our constants namespace
// ---------------------------------------------------------------------------
namespace kZWaveUSB3S
{
    // -----------------------------------------------------------------------
    //  The minimium inter-message send interval
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctSendInt = kCIDLib::enctOneMilliSec * 150;


    // -----------------------------------------------------------------------
    //  The minimium inter-msg interval for data type messages
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctDataInt = kCIDLib::enctOneMilliSec * 150;


    // -----------------------------------------------------------------------
    //  How many millis we wait for responses before giving up
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4WaitTimeout = 1500;
}


// ---------------------------------------------------------------------------
//  Our types namespace
// ---------------------------------------------------------------------------
namespace tZWaveUSB3S
{
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3S_ErrorIds.hpp"
#include    "ZWaveUSB3S_MessageIds.hpp"
#include    "ZWaveUSB3S_Internal.hpp"
#include    "ZWaveUSB3S_ZStick.hpp"
#include    "ZWaveUSB3S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method
// ---------------------------------------------------------------------------
extern TFacility& facZWaveUSB3S();

