//
// FILE NAME: LutronRA2C.hpp
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
//  This is the main header file for the Lutron RadioRA2 client driver facility.
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
class TLutronRA2CWnd;


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kLutronRA2C
{
    // The version of the config data we are compiled for
    const   tCIDLib::TCard4 c4ConfigVer = 5;

    // Max button and LED numbers we let them enter
    const   tCIDLib::TCard4 c4MaxNumber = 24;
}


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "LutronRA2C_MessageIds.hpp"
#include    "LutronRA2C_ErrorIds.hpp"
#include    "LutronRA2C_ResourceIds.hpp"
#include    "LutronRA2C_Data.hpp"


namespace tLutronRA2C
{
    using TColStrs = TVector<TString, tLutronRA2C::EListCols>;
}

#include    "LutronRA2C_Item.hpp"
#include    "LutronRA2C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facLutronRA2C();
