//
// FILE NAME: iTunesRepoS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2007
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
//  This is the main header file for a the iTunes disk-based media
//  repository browser.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDXML.hpp"
#include    "CIDSock.hpp"
#include    "CIDMetaExtr.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "iTunesRepoS_ErrorIds.hpp"
#include    "iTunesRepoS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tiTunesRepo
{
    // -----------------------------------------------------------------------
    //  A status enum that the loader thread class makes available to the
    //  driver, to let it know the status of the load process.
    // -----------------------------------------------------------------------
    enum ELoadStatus
    {
        ELoadStatus_Idle
        , ELoadStatus_Loading
        , ELoadStatus_Loaded
        , ELoadStatus_Failed
    };
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "iTunesRepoS_DBLoader.hpp"
#include    "iTunesRepoS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kiTunesRepoS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName= L"iTunesRepoS";


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
extern TFacility& faciTunesRepoS();

