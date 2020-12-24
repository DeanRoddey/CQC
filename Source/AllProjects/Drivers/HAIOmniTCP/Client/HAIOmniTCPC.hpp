//
// FILE NAME: HAIOmniTCPC.hpp
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

#include    "CQCIGKit.hpp"
#include    "HAIOmniTCPSh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class THAIOmniTCPCWnd;


// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tHAIOmniTCPC
{
    // -----------------------------------------------------------------------
    //  Returned from the item edit dialog
    // -----------------------------------------------------------------------
    enum EEditRes
    {
        EEditRes_Cancel
        , EEditRes_Save
        , EEditRes_Delete
    };
}


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPC_MessageIds.hpp"
#include    "HAIOmniTCPC_ErrorIds.hpp"
#include    "HAIOmniTCPC_ResourceIds.hpp"
#include    "HAIOmniTCPC_AllTrigsDlg.hpp"
#include    "HAIOmniTCPC_EdItemDlg.hpp"
#include    "HAIOmniTCPC_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facHAIOmniTCPC();


