//
// FILE NAME: CQCMQTT.cpp
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
//  We just do the usual library thing and provide our facility object's lazy
//  eval method.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCMQTT_.hpp"


// The lazy eval function that faults in the facility object
TFacCQCMQTT& facCQCMQTT()
{
    static TFacCQCMQTT* pfacThis = new TFacCQCMQTT();
    return *pfacThis;
}
