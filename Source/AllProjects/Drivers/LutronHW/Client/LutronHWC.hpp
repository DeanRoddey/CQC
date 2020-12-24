//
// FILE NAME: LutronHWC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This is the main header file for the Lutron Homeworks client driver facility. This
//  is the original HW version, not the QS versions which is handled by the RA2 driver.
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
#include    "CQCIGKit.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TLutronHWCWnd;


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kLutronHWC
{
    // The version of the config data we are compiled for
    const   tCIDLib::TCard4 c4ConfigVer = 1;

    // Max button and LED numbers we let them enter
    const   tCIDLib::TCard4 c4MaxNumber = 24;
}


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "LutronHWC_MessageIds.hpp"
#include    "LutronHWC_ErrorIds.hpp"
#include    "LutronHWC_ResourceIds.hpp"
#include    "LutronHWC_Data.hpp"
#include    "LutronHWC_Item.hpp"
#include    "LutronHWC_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facLutronHWC();
