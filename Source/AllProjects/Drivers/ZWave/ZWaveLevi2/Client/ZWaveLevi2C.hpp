//
// FILE NAME: ZWaveLevi2C.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/21/2014
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
#include    "ZWaveLevi2Sh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TZWaveLevi2CWnd;


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2C_MessageIds.hpp"
#include    "ZWaveLevi2C_ErrorIds.hpp"
#include    "ZWaveLevi2C_ResourceIds.hpp"
#include    "ZWaveLevi2C_Type.hpp"

#include    "ZWaveLevi2C_DummyLevi.hpp"
#include    "ZWaveLevi2C_AddAssocDlg.hpp"
#include    "ZWaveLevi2C_QAssocDlg.hpp"
#include    "ZWaveLevi2C_SetCfgParmDlg.hpp"
#include    "ZWaveLevi2C_EdGroupsDlg.hpp"
#include    "ZWaveLevi2C_ZWIndex.hpp"
#include    "ZWaveLevi2C_SelTypeDlg.hpp"
#include    "ZWaveLevi2C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facZWaveLevi2C();


