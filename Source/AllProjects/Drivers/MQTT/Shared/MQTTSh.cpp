//
// FILE NAME: MQTTSh.cpp
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
//  This is the main implementation file of a library facility that is shared
//  between the MQTT gateway server and the CQC driver that exposes it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MQTTSh_.hpp"


// The lazy eval function that faults in the facility object
TFacMQTTSh& facMQTTSh()
{
    static TFacMQTTSh* pfacThis = new TFacMQTTSh();
    return *pfacThis;
}
