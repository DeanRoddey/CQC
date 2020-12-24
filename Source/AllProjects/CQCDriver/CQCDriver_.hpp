//
// FILE NAME: CQCDriver_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2000
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
//  This is the internal facility header
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
#include "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Bring in any stuff we need internally only
// ---------------------------------------------------------------------------
#include    "CIDRegX.hpp"
#include    "CIDXML.hpp"


//
//  This is the bare minimum stuff that a facilty like this, which may indirectly
//  invoke CML code, needs to bring in. We just have to handle some exeptions that
//  the engine throws that are purposefully not based on TError.
//
//  This allows us to not have to bring in all of that macro engine code. It's split
//  out down to the CIDLib facilty, which doesn't include it but just makes it
//  available, and this insures it gets copied to res when we are built since we
//  don't include a dependency on the macro engine itself.
//
#include    "CIDLib_MacroExcept.hpp"

