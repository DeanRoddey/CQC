//
// FILE NAME: VarDriverS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This is the main file for the variable driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  We bring in a header from our client library
// ---------------------------------------------------------------------------
#include    "VarDriverC_Constant.hpp"



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "VarDriverS_ErrorIds.hpp"
#include    "VarDriverS_MessageIds.hpp"
#include    "VarDriverS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facVarDriverS();

