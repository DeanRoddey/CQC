//
// FILE NAME: CQCAppShellLib.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This is the main header of the CQC application shell.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCAPPSHELLLIB)
#define CQCAPPSHLIBEXPORT   CID_DLLEXPORT
#else
#define CQCAPPSHLIBEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDAppSh.hpp"
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our headers
// ---------------------------------------------------------------------------
#include    "CQCAppShellLib_ErrorIds.hpp"
#include    "CQCAppShellLib_MessageIds.hpp"
#include    "CQCAppShellLib_Record.hpp"
#include    "CQCAppShellLib_Engine.hpp"
#include    "CQCAppShellLib_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern CQCAPPSHLIBEXPORT TFacCQCAppShellLib& facCQCAppShellLib();


