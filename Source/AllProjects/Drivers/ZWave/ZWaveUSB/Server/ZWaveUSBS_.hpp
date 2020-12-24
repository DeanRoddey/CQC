//
// FILE NAME: ZWaveUSBS_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2005
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
//  This is the internal header for this facility. Though technically we don't
//  need one for a driver, since it is never imported directly, and we could
//  just have a public main header, we generate IDL code and it assumes the
//  standard internal header.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// Just include our main header
#include "ZWaveUSBS.hpp"



// ---------------------------------------------------------------------------
//  Bring in the Z-Wave headers we need
// ---------------------------------------------------------------------------
#include    <ZW_typedefs.h>
#include    <ZW_SerialAPI.h>
#include    <ZW_ClassCmd.h>
#include    <ZW_controller_api.h>
#include    <ZW_transport_api.h>
