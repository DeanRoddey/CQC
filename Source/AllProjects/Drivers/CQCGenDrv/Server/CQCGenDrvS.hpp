//
// FILE NAME: CQCGenDrvS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
//  This is the main header of the CQCGenDrv facility. This facility defines
//  the macro language level classes that are the basis of device drivers
//  written in the macro language. There is one major one, which is the
//  base class for the driver itself, and some helper classes. It also
//  provides a derivative of the standard CQC C++ server side driver class
//  that understands how to glue the C++ level driver interfaces to the
//  macro level interfaces.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCGENDRVS)
#define CQCGENDRVSEXPORT CID_DLLEXPORT
#else
#define CQCGENDRVSEXPORT CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDMacroEng.hpp"
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMEng.hpp"


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_ErrorIds.hpp"
#include    "CQCGenDrvS_Type.hpp"
#include    "CQCGenDrvS_DriverGlue.hpp"
#include    "CQCGenDrvS_DriverClass.hpp"
#include    "CQCGenDrvS_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCGENDRVSEXPORT TFacCQCGenDrvS& facCQCGenDrvS();


