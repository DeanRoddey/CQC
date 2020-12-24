//
// FILE NAME: CQCTrayMon.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2004
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
//  This is the main header of the CQC Tray Monitor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDPNG.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDImgFact.hpp"

#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCIGKit.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCTrayMon
{
    // -----------------------------------------------------------------------
    //  The ids we give the main frame window tabs
    // -----------------------------------------------------------------------
    constexpr   tCIDLib::TCard4 c4TabId_AppCtrl = 1;
    constexpr   tCIDLib::TCard4 c4TabId_iTunes  = 2;


    // -----------------------------------------------------------------------
    //  The max bytes we'll read to generate image persistent ids
    // -----------------------------------------------------------------------
    constexpr   tCIDLib::TCard4 c4MaxImgPerIdBytes = 32 * 1024;
}



// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCTrayMon_ErrorIds.hpp"
#include    "CQCTrayMon_MessageIds.hpp"
#include    "CQCTrayMon_ResourceIds.hpp"
#include    "CQCTrayMon_Type.hpp"

#include    "CQCTrayMon_WndMapItem.hpp"
#include    "CQCTrayMon_AppRecord.hpp"
#include    "CQCTrayMon_ListChangeEvent.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCTrayMon
{
    // -----------------------------------------------------------------------
    //  We need a ref hash set for storing app control records. It's used by
    //  the protocol server impl class, but also exposed to the main window
    //  which needs to display information about the active apps.
    // -----------------------------------------------------------------------
    using TAppList = TRefKeyedHashSet<TAppCtrlRec,TString,TStringKeyOps>;
}

#include    "CQCTrayMon_CoreAdminImpl.hpp"
#include    "CQCTrayMon_TabWnd.hpp"
#include    "CQCTrayMon_AppCtrlServerBase.hpp"
#include    "CQCTrayMon_AppCtrlSrvImpl.hpp"
#include    "CQCTrayMon_AppCtrlEvents.hpp"
#include    "CQCTrayMon_AppCtrlTab.hpp"
#include    "CQCTrayMon_iTPlayerIntfServerBase.hpp"
#include    "CQCTrayMon_iTRepoIntfServerBase.hpp"
#include    "CQCTrayMon_iTPlayerIntfImpl.hpp"
#include    "CQCTrayMon_iTRepoIntfImpl.hpp"
#include    "CQCTrayMon_iTunesTab.hpp"
#include    "CQCTrayMon_TrayWnd.hpp"
#include    "CQCTrayMon_MainFrame.hpp"
#include    "CQCTrayMon_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCTrayMon   facCQCTrayMon;


