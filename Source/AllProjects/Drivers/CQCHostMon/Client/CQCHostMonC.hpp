//
// FILE NAME: CQCHostMonC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/21/2000
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
//  This is the main header file for the host monitor driver. Since no one
//  ever includes our headers but us, we don't bother with having an internal
//  facility header.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDCtrls.hpp"

#include    "CQCIGKit.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tHostMonC
{
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCHostMonC_Constant.hpp"
#include    "CQCHostMonC_ErrorIds.hpp"
#include    "CQCHostMonC_MessageIds.hpp"
#include    "CQCHostMonC_ResourceIds.hpp"
#include    "CQCHostMonC_VolInfo.hpp"
#include    "CQCHostMonC_Window.hpp"


// ---------------------------------------------------------------------------
//  Export our facility object getter
// ---------------------------------------------------------------------------
extern TGUIFacility& facHostMonC();

