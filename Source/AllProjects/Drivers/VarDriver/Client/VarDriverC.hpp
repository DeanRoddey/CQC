//
// FILE NAME: CQCHostMonC.hpp
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
//  This is the main header file for the variable driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDCtrls.hpp"
//#include    "CIDWUtils.hpp"

#include    "CQCIGKit.hpp"



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tVarDriverC
{
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order.
// ---------------------------------------------------------------------------
#include    "VarDriverC_Constant.hpp"
#include    "VarDriverC_ErrorIds.hpp"
#include    "VarDriverC_MessageIds.hpp"
#include    "VarDriverC_ResourceIds.hpp"
#include    "VarDriverC_EdFldDlg.hpp"
#include    "VarDriverC_Window.hpp"


// ---------------------------------------------------------------------------
//  Export our facility object getter
// ---------------------------------------------------------------------------
extern TGUIFacility& facVarDriverC();

