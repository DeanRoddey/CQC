//
// FILE NAME: AutonomicS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2007
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
//  This is the main header file for a the Autonomic Media Center server
//  product, which provides access to and control of MCE. This is a
//  standard media repository driver, a standard renderer driver, and provides
//  some miscellaneous control features.
//
//  We load the meta data into our own in-memory database, so that we can
//  operate like a normal CQC media repository driver after that. We have to
//  do it a little differently than usual because we have to connect to the
//  device first, and only then can we download the data, whereas normally
//  this would be done in the 'wait config' phase of the driver loading.
//
//  Since we don't want to come online, what we do is to start up a loader
//  thread when we first come into Connect, and then just keep checking it
//  until it's completed or failed. If it completes, then we return success
//  from Connect.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"

#include    "CQCMedia.hpp"

#include    "CIDJPEG.hpp"

#include    "CIDNet.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "AutonomicS_ErrorIds.hpp"
#include    "AutonomicS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tAutonomicS
{
    // -----------------------------------------------------------------------
    //  An internal connection status enum value that we use to keep up with
    //  our own state, which is different from the overall driver state.
    //  These only apply while we are in the connect driver state.
    //
    //  WaitPower - We got a response from the server, but MCE is now powered
    //              up so we powered it up and now we are waiting for it to
    //              be ready. We don't wait for the response, we just check
    //              when we get back next time.
    //
    //  DB_Loading- MCE is powered up and we are connected to it. We are now
    //              wait for the the loader thread to load the media database.
    //
    //  DB_Loaded - The loader thread sets this when has successfully loaded
    //              the database, and our connect callback will return success
    //              from the connect method and we will move forward then.
    //
    //  DB_Failed - The loader thread sets this if it fails to load the media
    //              data. It will back off and try again in a while.
    // -----------------------------------------------------------------------
    enum EConnStatus
    {
        EConnStatus_StartLoad
        , EConnStatus_Loading
        , EConnStatus_Loaded
        , EConnStatus_Failed
    };
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "AutonomicS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kAutonomicS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"AutonomicS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4       c4Revision      = CID_REVISION;
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facAutonomicS();

