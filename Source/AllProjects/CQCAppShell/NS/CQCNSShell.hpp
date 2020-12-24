//
// FILE NAME: CQCNSShell.hpp
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
//  This is the main header of the CQC application shell. This is just a
//  very thin wrapper around the library that implements the bulk of the
//  functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CQCAppShellLib.hpp"


// ---------------------------------------------------------------------------
//  And now sub-include our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQCNSShell_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Our one global object
// ---------------------------------------------------------------------------
extern TFacCQCNSShell facCQCNSShell;

