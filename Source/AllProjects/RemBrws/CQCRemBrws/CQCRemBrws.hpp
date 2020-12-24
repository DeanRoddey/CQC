//
// FILE NAME: CQCRemBrws.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/24/2015
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
//  This facility defines some types and interfaces for managing remote resources, i.e.
//  getting/setting (browsing) stuff owned by the core back end servers. We provide an
//  abtract interface via which all the back end resources can be managed. Higher level
//  code provides derivatives of this interface, in CQCTreeBrws which provides a tree
//  control based browser framework that works in terms of our abstract browser interface.
//
//  We also provide the ORB interface for talking to the data server, and a simplified
//  wrapper class around the data server browser implementation, to make it easier to
//  read/write specific data types.
//
//  And we also have here some general classes that are involved in talking to the data
//  server and for which there is no better place for them to live.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CQCKit.hpp"



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCREMBRWS)
#define CQCREMBRWSEXP CID_DLLEXPORT
#else
#define CQCREMBRWSEXP CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCRemBrws
{
    // -----------------------------------------------------------------------
    //  This is the size of data blocks that are used to transfer files from the
    //  data server. We don't want to make it too big, since it will eat up a lot
    //  of bandwidth for other threads using the same ORB connection in that
    //  process. But it needs to be big enough to avoid too many transactions
    //  to get a larger file.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4DataBlockSz       = 0x10000;
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCRemBrws_ErrorIds.hpp"
#include    "CQCRemBrws_MessageIds.hpp"
#include    "CQCRemBrws_Shared.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCRemBrws
{
    using EDTypeList = TFundVector<tCQCRemBrws::EDTypes>;
}

#include    "CQCRemBrws_ImgMap.hpp"
#include    "CQCRemBrws_DataSrvBrwsItem.hpp"
#include    "CQCRemBrws_AppShellAdminClientProxy.hpp"
#include    "CQCRemBrws_DataSrvAccClientProxy.hpp"

namespace tCQCRemBrws
{
    using TDSrvClProxy = TCntPtr<TDataSrvAccClientProxy>;
}

#include    "CQCRemBrws_DataSrvClient.hpp"
#include    "CQCRemBrws_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCREMBRWSEXP TFacCQCRemBrws& facCQCRemBrws();

