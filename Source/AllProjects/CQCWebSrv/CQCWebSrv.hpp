//
// FILE NAME: CQCWebSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/01/2005
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
//  This is the main header of the CQC web server product. It provides a
//  simple web server that allows for access to CQC functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"
#include    "CQCMEng.hpp"
#include    "CQCAct.hpp"
#include    "CQCIntfEng.hpp"
#include    "CQCWebSrvC.hpp"
#include    "CQCSrvFW.hpp"


// ---------------------------------------------------------------------------
//  Include our headers
// ---------------------------------------------------------------------------
#include    "CQCWebSrv_ErrorIds.hpp"
#include    "CQCWebSrv_MessageIds.hpp"
#include    "CQCWebSrv_Constants.hpp"

#include    "CQCWebSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCWebSrv facCQCWebSrv;

