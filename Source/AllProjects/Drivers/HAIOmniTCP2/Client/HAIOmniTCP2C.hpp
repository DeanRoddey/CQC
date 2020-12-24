//
// FILE NAME: HAIOmniTCP2C.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/09/2014
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
#include    "CIDCtrls.hpp"

#include    "CQCIGKit.hpp"
#include    "HAIOmniTCP2Sh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class THAIOmniTCPCWnd2;


// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tHAIOmniTCP2C
{
    // -----------------------------------------------------------------------
    //  Returned from the item edit dialog
    // -----------------------------------------------------------------------
    enum class EEditRes
    {
        Cancel
        , Save
        , Delete
    };
}


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2C_MessageIds.hpp"
#include    "HAIOmniTCP2C_ErrorIds.hpp"
#include    "HAIOmniTCP2C_ResourceIds.hpp"
#include    "HAIOmniTCP2C_AllTrigsDlg.hpp"
#include    "HAIOmniTCP2C_EdItemDlg.hpp"
#include    "HAIOmniTCP2C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facHAIOmniTCP2C();


