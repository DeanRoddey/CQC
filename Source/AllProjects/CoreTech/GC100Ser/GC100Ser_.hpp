//
// FILE NAME: GC100Ser_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/12/2013
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
//  We bring in all of the controller and module header here, because these
//  are used internally only. The outside world only sees controllers and
//  modules via the abstract interface that we export. They never create these
//  specific implementations directly.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "GC100Ser.hpp"


// ---------------------------------------------------------------------------
//  Any internal only headers
// ---------------------------------------------------------------------------
#include    "CIDComm.hpp"
#include    "CIDOrbUC.hpp"
#include    "GC100Ser_Port_.hpp"


