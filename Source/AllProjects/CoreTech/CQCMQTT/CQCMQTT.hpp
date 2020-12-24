//
// FILE NAME: CQCMQTT.hpp
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
#if     defined(PROJ_CQCMQTT)
#define CQCMQTTEXPORT    CID_DLLEXPORT
#else
#define CQCMQTTEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the underlying stuff we we need
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDOrb.hpp"
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Bring in our public headers
// ---------------------------------------------------------------------------
#include    "CQCMQTT_ErrorIds.hpp"
#include    "CQCMQTT_Shared.hpp"


// ---------------------------------------------------------------------------
//  Slip in some facility types
// ---------------------------------------------------------------------------
namespace tCQCMQTT
{
    using TQOSList  = TFundVector<tCQCMQTT::EQOSLevs>;
}


#include    "CQCMQTT_Msg.hpp"
#include    "CQCMQTT_ThisFacility.hpp"



// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern CQCMQTTEXPORT TFacCQCMQTT& facCQCMQTT();
