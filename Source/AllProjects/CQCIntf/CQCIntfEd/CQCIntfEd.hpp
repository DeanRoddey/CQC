//
// FILE NAME: CQCIntfEd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This is the main header for the interface editor engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCINTFED)
#define CQCINTFEDEXPORT CID_DLLEXPORT
#else
#define CQCINTFEDEXPORT CID_DLLIMPORT
#endif



// ----------------------------------------------------------------------------
//  Include any needed underlying facility headers
// ----------------------------------------------------------------------------
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"
#include    "CQCIntfEng.hpp"
#include    "CQCTreeBrws.hpp"



// ----------------------------------------------------------------------------
//  And include our own headers
// ----------------------------------------------------------------------------
#include    "CQCIntfEd_MessageIds.hpp"
#include    "CQCIntfEd_ErrorIds.hpp"
#include    "CQCIntfEd_MainWnd.hpp"
#include    "CQCIntfEd_ThisFacility.hpp"


// ----------------------------------------------------------------------------
//  Globals exported from CQCIntfEd.cpp
// ----------------------------------------------------------------------------
extern TFacCQCIntfEd& facCQCIntfEd();

