//
// FILE NAME: ZWaveUSBC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/26/2005
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
//  This is the main header file for the ZWave client driver facility.
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
#include    "ZWaveUSBSh.hpp"


// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TZWaveUSBCWnd;


// ---------------------------------------------------------------------------
//  Include our other public headers
// ---------------------------------------------------------------------------
#include    "ZWaveUSBC_MessageIds.hpp"
#include    "ZWaveUSBC_ErrorIds.hpp"
#include    "ZWaveUSBC_ResourceIds.hpp"
#include    "ZWaveUSBC_Type.hpp"

#include    "ZWaveUSBC_LearnDlg.hpp"
#include    "ZWaveUSBC_EdUnitDlg.hpp"
#include    "ZWaveUSBC_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facZWaveUSBC();


