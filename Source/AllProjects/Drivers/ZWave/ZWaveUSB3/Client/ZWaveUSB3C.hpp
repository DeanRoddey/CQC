//
// FILE NAME: ZWaveUSB3C.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/05/2018
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
//  This is the main header file for the ZWave USB3 client driver facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "ZWaveUSB3Sh.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class TZWaveUSB3CWnd;


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kZWaveUSB3C
{
}


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3C_MessageIds.hpp"
#include    "ZWaveUSB3C_ErrorIds.hpp"
#include    "ZWaveUSB3C_ResourceIds.hpp"
#include    "ZWaveUSB3C_TypesConsts.hpp"

#include    "ZWaveUSB3C_AssocDlg.hpp"
#include    "ZWaveUSB3C_AssocToDrvDlg.hpp"
#include    "ZWaveUSB3C_AutoCfgDlg.hpp"
#include    "ZWaveUSB3C_CfgParamDlg.hpp"
#include    "ZWaveUSB3C_GenReportDlg.hpp"
#include    "ZWaveUSB3C_InclusionDlg.hpp"
#include    "ZWaveUSB3C_ResetUnitDlg.hpp"
#include    "ZWaveUSB3C_SelUTypeDlg.hpp"
#include    "ZWaveUSB3C_UnitInfoDlg.hpp"

#include    "ZWaveUSB3C_Window.hpp"


// ---------------------------------------------------------------------------
//  Expose our facility object getter for internall use.
// ---------------------------------------------------------------------------
extern TGUIFacility& facZWaveUSB3C();
