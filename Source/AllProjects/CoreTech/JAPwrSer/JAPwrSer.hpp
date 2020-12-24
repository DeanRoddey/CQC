//
// FILE NAME: JAPwrSer.hpp
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
//  This is the main header of the facility. It exports anything we want to expose
//  and brings in any underlying headers we need. This facility enables CQC to use
//  the serial port passthrough capabilities of a Just Add Power device. So they
//  show up as serial ports anywhere in CQC that regular ports would.
//
//  JAP devices currently only support a single serial port, but we could handle
//  more if a newer model allowed for it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_JAPWRSER)
#define JAPWRSEREXPORT    CID_DLLEXPORT
#else
#define JAPWRSEREXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our needed underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "JAPwrSer_ErrorIds.hpp"
#include    "JAPwrSer_MessageIds.hpp"
#include    "JAPwrSer_Constant.hpp"

#include    "JAPwrSer_PortCfg.hpp"
#include    "JAPwrSer_PortFactory.hpp"
#include    "JAPwrSer_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern JAPWRSEREXPORT TFacJAPwrSer& facJAPwrSer();

