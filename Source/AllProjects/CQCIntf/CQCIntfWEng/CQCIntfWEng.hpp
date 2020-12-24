//
// FILE NAME: CQCIntfWEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This is the main header of the standard windowed implementation of the
//  user interface engine. We just provide a concrete implementation of the
//  interface engine's view interface, which provides for display on a
//  standard window. So there's no a lot here. Mostly it's all done in the
//  base interface engine facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCINTFWENG)
#define CQCINTFWENGEXPORT   CID_DLLEXPORT
#else
#define CQCINTFWENGEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDCtrls.hpp"
#include    "CQCKit.hpp"
#include    "CQCIntfEng.hpp"


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCIntfWEng_ErrorIds.hpp"
#include    "CQCIntfWEng_MessageIds.hpp"
#include    "CQCIntfWEng_ValErrDlg.hpp"
#include    "CQCIntfWEng_View.hpp"
#include    "CQCIntfWEng_Window.hpp"
#include    "CQCIntfWEng_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCINTFWENGEXPORT TFacCQCIntfWEng& facCQCIntfWEng();

