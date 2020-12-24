//
// FILE NAME: CQCGKit.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/12/2001
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
//  This is the main header of the CQC GUI toolkit. Clients wishing to develop
//  drivers or clients only have to include this one file. It will define and
//  sub-include anything else needed, in the correct order.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCGKIT)
#define CQCGKITEXPORT   CID_DLLEXPORT
#else
#define CQCGKITEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDCtrls.hpp"
#include    "CIDObjStore.hpp"
#include    "CQCKit.hpp"



// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCGKit_ErrorIds.hpp"
#include    "CQCGKit_MessageIds.hpp"
#include    "CQCGKit_Type.hpp"
#include    "CQCGKit_Constant.hpp"
#include    "CQCGKit_FrameState.hpp"
#include    "CQCGKit_DrvPoller.hpp"
#include    "CQCGKit_DrvInfoTab.hpp"
#include    "CQCGKit_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCGKITEXPORT TFacCQCGKit& facCQCGKit();


