//
// FILE NAME: MQTTS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2019
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
//  This is the main file for the MQTT driver. This driver works in terms of a
//  separate 'MQTT Gateway Server' service.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDSock.hpp"
#include    "CIDNet.hpp"
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMQTT.hpp"
#include    "MQTTSh.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kMQTTS
{
    // The name of our server facility
    const tCIDLib::TCh* const   pszServerFacName = L"MQTTS";

    //
    //  Some protocol related stuff that is only relevant here to this program. We
    //  we add the moniker to the client id (in case there are multiples.)
    //
    const tCIDLib::TCard2 c2KeepAliveSecs = 60;
    const tCIDLib::TCard2 c2KeepAliveThresh = 45;
    const TString strClientId = L"CQCMQTTS";
}


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tMQTTS
{
    // The events that the I/O thread queues up on the main driver thread
    enum class EIOEvents
    {
        None
        , FldError
        , FldValue
        , StateChange
        , FieldWrite
    };

    // The events that the main driver thread queues up on the I/O thread
    enum class EDrvEvents
    {
        None
        , FieldWrite
        , ReloadCfg
    };
}


// ---------------------------------------------------------------------------
//  Forward ref some stuff for the headers below
// ---------------------------------------------------------------------------
class TMQTTS;


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "MQTTS_ErrorIds.hpp"
#include    "MQTTS_MessageIds.hpp"
#include    "MQTTS_IOEvent.hpp"
#include    "MQTTS_DrvEvent.hpp"
#include    "MQTTS_MsgPools.hpp"

using TMQTTIOEvPtr = TFixedPoolPtr<TMQTTIOEvent>;
using TMQTTDrvEvPtr = TFixedPoolPtr<TMQTTDrvEvent>;
using TMQTTInMsgPtr = TSimplePoolPtr<TMQTTInMsg>;
using TMQTTOutMsgPtr = TSimplePoolPtr<TMQTTOutMsg>;

#include    "MQTTS_IOThread.hpp"
#include    "MQTTS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facMQTTS();

