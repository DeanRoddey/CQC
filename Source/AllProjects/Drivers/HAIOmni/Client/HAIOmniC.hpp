//
// FILE NAME: HAIOmniC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2006
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
//  This is the main header file for the HAI Omni client driver facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCGKit.hpp"
#include    "HAIOmniSh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class THAIOmniCWnd;



// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "HAIOmniC_MessageIds.hpp"
#include    "HAIOmniC_ErrorIds.hpp"
#include    "HAIOmniC_ResourceIds.hpp"
#include    "HAIOmniC_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facHAIOmniC();


