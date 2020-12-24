//
// FILE NAME: CQCTreeBrws_.hpp
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
#include "CQCTreeBrws.hpp"


// ---------------------------------------------------------------------------
//  Some underlying stuff that's only needed internally
// ---------------------------------------------------------------------------
#include    "CIDDBase.hpp"
#include    "CIDRegX.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDPNG.hpp"
#include    "CIDUPnP.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDMacroEng.hpp"
#include    "CQCGKit.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"
#include    "CQCIntfEng.hpp"



// ---------------------------------------------------------------------------
//  Constants we only need internally
// ---------------------------------------------------------------------------
namespace kCQCTreeBrws_
{
    //
    //  Ids used to post codes to the browser window, the first set it posts to
    //  itself and the othres are posted from the browser objects.
    //
    const tCIDLib::TInt4    i4FileDrop      = 1;

    const tCIDLib::TInt4    i4ChangeQ       = 100;
};


// ---------------------------------------------------------------------------
//  Bring in any of our own stuff we only need internally
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_ResourceIds.hpp"

