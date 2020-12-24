//
// FILE NAME: ZWaveUSB3S_.hpp
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
//  This is the internal header for this facility. Though technically we don't
//  need one for a driver, since it is never imported directly, and we could
//  just have a public main header, we generate IDL code and it assumes the
//  standard internal header.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
// Include our main header and any internal stuff
// ---------------------------------------------------------------------------
#include "ZWaveUSB3S.hpp"


// ---------------------------------------------------------------------------
//  We need the core Z-Wave headers
// ---------------------------------------------------------------------------
#include    <ZW_typedefs.h>
#include    <ZW_transport_api.h>
#include    <ZW_controller_static_api.h>
#include    <ZW_serialapi.h>
#include    <ZW_classCmd.h>

