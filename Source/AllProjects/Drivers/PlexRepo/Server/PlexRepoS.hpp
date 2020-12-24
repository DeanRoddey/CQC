//
// FILE NAME: PlexRepoS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2015
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
//  This is the main header file for a the Plex Media Center media repository
//  driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDNet.hpp"
#include    "CIDImgFact.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "PlexRepoS_ErrorIds.hpp"
#include    "PlexRepoS_MessageIds.hpp"
#include    "PlexRepoS_Types.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tPlexRepoS
{
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "PlexRepoS_DBLoader.hpp"
#include    "PlexRepoS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kPlexRepoS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"PlexRepoS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4       c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The max number of bytes of an image we'll read to generate the persistent
    //  id. It's not likely any images, that aren't actually the same, will be
    //  indentical for this number of bytes. If we end up reading the whole thing,
    //  we'll just pre-cache it.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPerIdBytes = 48 * 1024;
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facPlexRepoS();

