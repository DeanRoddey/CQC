//
// FILE NAME: GC100Ser.hpp
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
//  This is the main header of the faciliyt. It exports anything we want
//  to expose and brings in any underlying headers we need.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_GC100SER)
#define GC100SEREXPORT    CID_DLLEXPORT
#else
#define GC100SEREXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our needed underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "GC100Ser_ErrorIds.hpp"
#include    "GC100Ser_MessageIds.hpp"
#include    "GC100Ser_Constant.hpp"

#include    "GC100Ser_GCCfg.hpp"
#include    "GC100Ser_PortFactory.hpp"
#include    "GC100Ser_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern GC100SEREXPORT TFacGC100Ser& facGC100Ser();

