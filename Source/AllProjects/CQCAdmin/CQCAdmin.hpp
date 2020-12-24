//
// FILE NAME: CQCAdmin.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/18/2015
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
//  This is the main header of the CQC admin client. It brings in all of the headers
//  we require, defines global types and such.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDComm.hpp"
#include    "CIDPNG.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCMEng.hpp"
#include    "CQCMedia.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCTreeBrws.hpp"
#include    "CQCIntfEd.hpp"
#include    "CQCGWCl.hpp"
#include    "CQCWebSrvC.hpp"
#include    "CQCEvMonEng.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"
#include    "CQLogicSh.hpp"
#include    "CQCSysCfg.hpp"
#include    "CQCGenDrvS.hpp"


// ---------------------------------------------------------------------------
//  Facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCAdmin
{
    //
    //  We set a value on the hidden window name of each tab. It's one of these
    //  prefixes followed by the hierarchical path of the things being viewed or
    //  edited. This makes it possile for anything that can get the window name
    //  string to figure out what it is for.
    //
    const   TString         strTabPref_Edit(L"[EDIT]-");
    const   TString         strTabPref_View(L"[VIEW]-");

    // -----------------------------------------------------------------------
    //  Some ids for things that we store in a per-user, local object store. This is
    //  for user/app specific stuff like previous window area and stuff like that.
    //  These are done via a generic mechanism provided by CQCGKit, which allows us
    //  to easily store/recall data from such an object store. The pszAppStoreKey
    //  value is the program specific name we use to initialize it, which is used in
    //  the object store file name. The other values are keys for values we store.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszAppStoreKey      = L"AdminIntf";
    const tCIDLib::TCh* const   pszWndState         = L"LastWndInfo";

    // -----------------------------------------------------------------------
    //  These are also local object store keys, but in this case they are full paths
    //  for some that we deal with more manually, not using the generic mechanism
    //  that the above values are for.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszCfgKey_MainFrame      = L"/Wnd/MainFrameState";
    const tCIDLib::TCh* const   pszCfgKey_AutogenLast    = L"/Autogen/LastSettings";
}


// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCAdmin
{
    // Used in the content tabs when saving, to indicate what save mode
    enum class ESaveModes
    {
        Test
        , Save
    };
}



// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCAdmin_ErrorIds.hpp"
#include    "CQCAdmin_MessageIds.hpp"
#include    "CQCAdmin_ResourceIds.hpp"

#include    "CQCAdmin_ContentTab.hpp"
#include    "CQCAdmin_BrowserTab.hpp"
#include    "CQCAdmin_TextViewTab.hpp"
#include    "CQCAdmin_StateInfo.hpp"
#include    "CQCAdmin_SysCfgIntfClientProxy.hpp"

#include    "CQCAdmin_MainFrameWnd.hpp"
#include    "CQCAdmin_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCAdmin    facCQCAdmin;


