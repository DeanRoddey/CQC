//
// FILE NAME: MQTTS_DrvEvent.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/04/2019
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
//  This file implements the simple driver event class used to report info from the
//  driver thread to the I/O thread.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Bring in our own public header
// ---------------------------------------------------------------------------
#include    "MQTTS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTDrvEvent, TObject)



// ---------------------------------------------------------------------------
//   CLASS: TMQTTDrvEvent
//  PREFIX: ioev
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTDrvEvent: Constructor and Destructor
// ---------------------------------------------------------------------------
TMQTTDrvEvent::TMQTTDrvEvent()
{
    Reset();
}

TMQTTDrvEvent::~TMQTTDrvEvent()
{
}


// ---------------------------------------------------------------------------
//  TMQTTDrvEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMQTTDrvEvent::Reset()
{
    m_eEvent = tMQTTS::EDrvEvents::None;

    m_strValue.Clear();
    m_strFldName.Clear();
}

