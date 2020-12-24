//
// FILE NAME: ZWaveLeviC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/12/2012
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
//  This is the main header file for the Leviton ZWave client driver facility.
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
#include    "ZWaveLeviSh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TZWaveLeviCWnd;


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "ZWaveLeviC_MessageIds.hpp"
#include    "ZWaveLeviC_ErrorIds.hpp"
#include    "ZWaveLeviC_ResourceIds.hpp"
#include    "ZWaveLeviC_Type.hpp"

#include    "ZWaveLeviC_AddAssocDlg.hpp"
#include    "ZWaveLeviC_SetCfgParmDlg.hpp"
#include    "ZWaveLeviC_EdGroupsDlg.hpp"
#include    "ZWaveLeviC_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facZWaveLeviC();


