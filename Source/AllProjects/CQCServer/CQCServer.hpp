//
// FILE NAME: CQCServer.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/04/2000
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
//  This is the main header of the CQC server product. It does the usual
//  things that a CIDLib based facility header does.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDCrypto.hpp"
#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCGenDrvS.hpp"
#include    "CQCRPortClient.hpp"
#include    "GC100Ser.hpp"
#include    "JAPwrSer.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCSrvFW.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCServer_ErrorIds.hpp"
#include    "CQCServer_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCServer
{
    // -----------------------------------------------------------------------
    //  Persistent state constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszConfigFile    = L"CQCServer.Config";


    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvCQCServer  = L"CQCServer";
}


// ---------------------------------------------------------------------------
//  And now sub-include most of our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQCServer_SrvAdminServerBase.hpp"
#include    "CQCServer_DriverInfo.hpp"
#include    "CQCServer_SrvAdminImpl.hpp"


// ---------------------------------------------------------------------------
//  We have to slip this in before some of the headers can be done
// ---------------------------------------------------------------------------
namespace tCQCServer
{
    // -----------------------------------------------------------------------
    //  Typedef a vector of server side driver objects. This is where we keep
    //  up with all our loaded drivers. We use a little class to hold the info
    //  we need. We use a vector so that we can give back to clients a set of
    //  indexes to represent a driver and a field within that driver. This
    //  allows for fast access to those fields later.
    //
    //  We have a 'vector list id' that gets bumped whenever any driver is
    //  removed. This invalidates any client cached ids, forcing them to
    //  update their information to get back into sync.
    // -----------------------------------------------------------------------
    typedef TRefVector<TServerDriverInfo>   TDrvList;
    typedef TDrvList::TCursor               TDrvCursor;
    typedef TDrvList::TNCCursor             TNCDrvCursor;
}


// ---------------------------------------------------------------------------
//  And now sub-include the rest of our headers
// ---------------------------------------------------------------------------
#include    "CQCServer_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCServer    facCQCServer;

