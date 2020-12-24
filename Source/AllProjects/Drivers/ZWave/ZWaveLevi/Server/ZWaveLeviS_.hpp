//
// FILE NAME: ZWaveLeviS_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
#include "ZWaveLeviS.hpp"



// ---------------------------------------------------------------------------
//  We need some Z-Wave headers because some stuff is still just passthrough
//  of Z-Wave API values.
// ---------------------------------------------------------------------------
#include    <ZW_typedefs.h>
#include    <ZW_SerialAPI.h>
#include    <ZW_ClassCmd.h>

