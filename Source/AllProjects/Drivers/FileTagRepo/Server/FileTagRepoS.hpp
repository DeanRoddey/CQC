//
// FILE NAME: FileTagRepoS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2007
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
//  This is the main header file for a tag-based media repository. The driver
//  scans a directory tree looking for new media files and adds them to the
//  database.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDCrypto.hpp"

#include    "CIDMetaExtr.hpp"

#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"

#include    "CQCMedia.hpp"

#include    "CIDJPEG.hpp"

#include    "CIDEncode.hpp"

#include    "CIDImgFact.hpp"



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "FileTagRepoS_ErrorIds.hpp"
#include    "FileTagRepoS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tFileTagRepoS
{
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "FileTagRepoS_DBLoader.hpp"
#include    "FileTagRepoS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kFileTagRepoS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"FileTagRepoS";


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
extern TFacility& facFileTagRepoS();

