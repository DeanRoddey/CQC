//
// FILE NAME: CQCGWSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2002
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
//  This is the main header of the CQC gateway server product. It provides an
//  'XML over raw sockets' interface to CQC, so that third party clients can
//  access CQC in a safe way.
//
//  There is a client side facility that provides C++ and CML wrappers around
//  the otherwise fairly tedious raw interface. We don't use it here but we
//  bring it in to get the DTD and other types and constants we share with
//  the clients.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDPNG.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDSock.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDXML.hpp"
#include    "CIDMacroEng.hpp"
#include    "CIDZLib.hpp"
#include    "CIDSChan.hpp"
#include    "CQCKit.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCMedia.hpp"
#include    "CQCGWCl.hpp"
#include    "CQCSysCfg.hpp"
#include    "CQCMEng.hpp"
#include    "CQCEvCl.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCAct.hpp"
#include    "CQCSrvFW.hpp"



// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCGWSrv_ErrorIds.hpp"
#include    "CQCGWSrv_MessageIds.hpp"
#include    "CQCGWSrv_Constants.hpp"


// ---------------------------------------------------------------------------
//  Facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCGWSrv
{
    // -----------------------------------------------------------------------
    //  If we don't hear from a client within this time, we assume he's died
    //  and drop the connection.
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    const tCIDLib::TEncodedTime enctIdleTime = kCIDLib::enctOneMinute * 10;
    #else
    const tCIDLib::TEncodedTime enctIdleTime = kCIDLib::enctOneMinute;
    #endif


    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvGWSrv      = L"CQCGWSrv";
}



// ---------------------------------------------------------------------------
//  Facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCGWSrv
{
    // -----------------------------------------------------------------------
    //  An enum that represents the 'option flags' that the clients can set
    //  on their session, to get us to do (or not do) certain things.
    // -----------------------------------------------------------------------
    enum class EOptFlags : tCIDLib::TCard4
    {
        None           = 0x00000000
        , NoAlpha      = 0x00000001
        , SendImgInfo  = 0x00000002
    };


    // -----------------------------------------------------------------------
    //  This is how the listener queues up incoming connections to be grabbed
    //  by one of the thread pool threads.
    // -----------------------------------------------------------------------
    using TConnQueue = TRefQueue<TServerStreamSocket>;
}


// ---------------------------------------------------------------------------
//  And sub-include the rest of our headers
// ---------------------------------------------------------------------------
#include    "CQCGWSrv_SysCfgIntfClientProxy.hpp"
#include    "CQCGWSrv_WorkerThread.hpp"
#include    "CQCGWSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCGWSrv facCQCGWSrv;

