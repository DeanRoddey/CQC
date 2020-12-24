//
// FILE NAME: CQCExtractDrvFlds.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2007
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
//  This is the main header for the program. It includes anything that is
//  needed by our program. This program is a simple utility that will
//  enumerated the fields provided by a driver and spit out an XML file
//  that can be used by the generic device simulator driver to load up
//  those fields for simulation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once

// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"
