//
// FILE NAME: CQCIGKit.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/18/2004
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
//  This is the main header of the CQC internal GUI toolkit. We have a need
//  for some common GUI related stuff, that we don't want to put into CQCGKit
//  because that is used by drivers and any graphical utilities. This both
//  serves to make things more secure but also to drastically reduce the
//  amount of rebuilding in many cases.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCIGKIT)
#define CQCIGKITEXPORT  CID_DLLEXPORT
#else
#define CQCIGKITEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDCtrls.hpp"

#include    "CQCKit.hpp"
#include    "CQCAct.hpp"
#include    "CQCMEng.hpp"
#include    "CQCGKit.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCTreeBrws.hpp"
#include    "CQCRPortClient.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TMEngExceptVal;
class TCQCDriverClient;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCIGKit_Type.hpp"
#include    "CQCIGKit_ErrorIds.hpp"
#include    "CQCIGKit_MessageIds.hpp"
#include    "CQCIGKit_Constant.hpp"

#include    "CQCIGKit_ActTraceWnd.hpp"
#include    "CQCIGKit_InterClassMgr.hpp"
#include    "CQCIGKit_Blanker.hpp"
#include    "CQCIGKit_DriverClient.hpp"
#include    "CQCIGKit_EditTrigEv.hpp"
#include    "CQCIGKit_ChatWnd.hpp"

#include    "CQCIGKit_StdActEngine.hpp"
#include    "CQCIGKit_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCIGKITEXPORT TFacCQCIGKit& facCQCIGKit();

