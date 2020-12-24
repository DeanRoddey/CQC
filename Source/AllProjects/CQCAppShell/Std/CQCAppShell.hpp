//
// FILE NAME: CQCAppShell.hpp
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
#include    "CIDOrbUC.hpp"
#include    "CQCAppShellLib.hpp"


// ---------------------------------------------------------------------------
//  Bring in the Windows specific service support stuff
// ---------------------------------------------------------------------------
#include    "CIDKernel_ServiceWin32.hpp"


// ---------------------------------------------------------------------------
//  And now sub-include our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQCAppShell_ServiceHandler.hpp"
#include    "CQCAppShell_AppShellAdminServerBase.hpp"
#include    "CQCAppShell_AdminImpl.hpp"
#include    "CQCAppShell_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern TFacCQCAppShell& facCQCAppShell();

