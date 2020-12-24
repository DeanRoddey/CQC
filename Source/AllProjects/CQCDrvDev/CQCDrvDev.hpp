//
// FILE NAME: CQCDrvDev.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/14/2003
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
//  This is the main header for this facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our needed underlying headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDXML.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CIDMacroDbg.hpp"
#include    "CQCMedia.hpp"
#include    "CQCGenDrvS.hpp"
#include    "CQCRPortClient.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"
#include    "CQCGWCl.hpp"
#include    "CQCMEng.hpp"
#include    "CQCTreeBrws.hpp"


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "CQCDrvDev_Type.hpp"
#include    "CQCDrvDev_ErrorIds.hpp"
#include    "CQCDrvDev_MessageIds.hpp"
#include    "CQCDrvDev_ResourceIds.hpp"
#include    "CQCDrvDev_Constant.hpp"

#include    "CQCDrvDev_QDrvTextDlg.hpp"
#include    "CQCDrvDev_SendDrvCmdDlg.hpp"
#include    "CQCDrvDev_FldMonWnd.hpp"
#include    "CQCDrvDev_MainFrame.hpp"
#include    "CQCDrvDev_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object to other files of the program
// ---------------------------------------------------------------------------
extern TFacCQCDrvDev facCQCDrvDev;


