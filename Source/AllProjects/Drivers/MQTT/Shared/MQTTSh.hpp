//
// FILE NAME: MQTTSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/21/2018
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_MQTTSH)
#define MQTTSHEXPORT    CID_DLLEXPORT
#else
#define MQTTSHEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the underlying stuff we we need
// ---------------------------------------------------------------------------
#include    "CIDRegX.hpp"
#include    "CIDEncode.hpp"
#include    "CIDNet.hpp"
#include    "CQCKit.hpp"
#include    "CQCMQTT.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tMQTTSh
{
    using TTextMap = TVector<TKeyValFPair>;
}

// ---------------------------------------------------------------------------
//  Bring in our public headers
// ---------------------------------------------------------------------------
#include    "MQTTSh_ErrorIds.hpp"
#include    "MQTTSh_Shared.hpp"
#include    "MQTTSh_PLFormat.hpp"
#include    "MQTTSh_ValMaps.hpp"
#include    "MQTTSh_Config.hpp"
#include    "MQTTSh_ThisFacility.hpp"



// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern MQTTSHEXPORT TFacMQTTSh& facMQTTSh();
