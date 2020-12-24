//
// FILE NAME: CQCSysCfg.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/06/2011
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
//  This is the main header of the CQC system configuration library facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCSYSCFG)
#define CQCSYSCFGEXP CID_DLLEXPORT
#else
#define CQCSYSCFGEXP CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCSysCfg
{
    // -----------------------------------------------------------------------
    //  Max parms to a global action or room mode action
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4 c4MaxGActParms = 4;
    constexpr tCIDLib::TCard4 c4MaxRmModeParms = 4;


    // -----------------------------------------------------------------------
    //  Number of standard per-room variables we support
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4 c4MaxRoomVars = 4;


    // -----------------------------------------------------------------------
    //  The maximum number of rooms we can have
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4 c4MaxRooms    = 128;
}


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCSysCfg
{
}


// ---------------------------------------------------------------------------
//  Forward refs
// ---------------------------------------------------------------------------
class TCQCSysCfg;


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCSysCfg_ErrorIds.hpp"
#include    "CQCSysCfg_MessageIds.hpp"
#include    "CQCSysCfg_Shared.hpp"

#include    "CQCSysCfg_Parts.hpp"
#include    "CQCSysCfg_VoiceCfg.hpp"

// Slip in some more types here
namespace tCQCSysCfg
{
    using TGlobActList = TObjArray<TSCGlobActInfo, tCQCSysCfg::EGlobActs>;
    using TItemList = TRefVector<TSysCfgListItem>;
    using TRoomModeList = TVector<TSCGlobActInfo>;
}

// And some more types
#include    "CQCSysCfg_Room.hpp"
namespace tCQCSysCfg
{
    using TResList = TVector<TSize>;
    using TRmInfoList = TVector<TCQCSysCfgRmInfo>;
}


#include    "CQCSysCfg_Layout.hpp"
#include    "CQCSysCfg_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCSYSCFGEXP TFacCQCSysCfg& facCQCSysCfg();

