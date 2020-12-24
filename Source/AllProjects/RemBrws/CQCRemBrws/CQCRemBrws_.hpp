//
// FILE NAME: CQCRemBrws_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "CQCRemBrws.hpp"


// ---------------------------------------------------------------------------
//  Some underlying stuff that's only needed internally
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDPNG.hpp"
#include    "CIDZLib.hpp"
#include    "CIDXML.hpp"



// ---------------------------------------------------------------------------
//  Bring in any of our own stuff we only need internally
// ---------------------------------------------------------------------------

//
//  These were public but now are just for use internally in converting to the new
//  chunked file format.
//
#include    "CQCRemBrws_ImageStoreItem.hpp"
#include    "CQCRemBrws_TemplateInfo.hpp"

