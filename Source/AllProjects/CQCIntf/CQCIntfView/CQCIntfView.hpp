//
// FILE NAME: CQCIntfView.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2002
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
//  This is the main header of the CQC standalone interface viewer.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDObjStore.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDPNG.hpp"
#include    "CIDCtrls.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCMEng.hpp"
#include    "CQCIntfEng.hpp"
#include    "CQCIntfWEng.hpp"
#include    "CQLogicSh.hpp"
#include    "CQCGWCl.hpp"


// ---------------------------------------------------------------------------
//  Some forward refs
// ---------------------------------------------------------------------------
class   TMainFrameWnd;
class   TCQCIntfViewWnd;


// ---------------------------------------------------------------------------
//  Local program constants
// ---------------------------------------------------------------------------
namespace kCQCIntfView
{
    // -----------------------------------------------------------------------
    //  Some keys we use in our local config store
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszStoreAppKey        = L"IntfViewer";
    const tCIDLib::TCh* const pszCfgKey_MainFrame   = L"/Wnd/MainFrameState";
}


// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCIntfView_ErrorIds.hpp"
#include    "CQCIntfView_MessageIds.hpp"
#include    "CQCIntfView_ResourceIds.hpp"

#include    "CQCIntfView_ShowFlds.hpp"
#include    "CQCIntfView_StateData.hpp"
#include    "CQCIntfView_ExtCtrlServerBase.hpp"
#include    "CQCIntfView_ExtCtrlServerImpl.hpp"
#include    "CQCIntfView_MainFrameWnd.hpp"
#include    "CQCIntfView_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCIntfView    facCQCIntfView;


