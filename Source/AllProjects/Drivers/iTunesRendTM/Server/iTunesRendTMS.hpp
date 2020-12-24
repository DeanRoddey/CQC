//
// FILE NAME: iTunesRendTMS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2015
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
//  This is the main header file for a the iTunes media renderer driver. It
//  implements a standard CQC media renderer driver via the tray monitor. It
//  only works with the iTunes repo. That repo only does music, so that's all
//  we handle.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "iTunesRendTMS_ErrorIds.hpp"
#include    "iTunesRendTMS_MessageIds.hpp"
#include    "iTunesRendTMS_iTrayMonPlIntfClientProxy.hpp"
#include    "iTunesRendTMS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kiTunesRendTMS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"iTunesRendTMS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& faciTunesRendTMS();

