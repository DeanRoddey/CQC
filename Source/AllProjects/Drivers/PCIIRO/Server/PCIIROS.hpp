//
// FILE NAME: PCIIROS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2006
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
//  This is the main file for the driver
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



// ---------------------------------------------------------------------------
//  We need to bring in a Win32 specific header here, kernel level, to do
//  registry access.
// ---------------------------------------------------------------------------
#include    "CIDKernel_RegistryWin32.hpp"


// ---------------------------------------------------------------------------
//  Bring in the I/O access header. Byte pack it.
// ---------------------------------------------------------------------------
#pragma     CIDLIB_PACK(1)
#include    <ACCES32.h>
#pragma     CIDLIB_POPPACK


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kPCIIROS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"PCIIROS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  Some board related constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPoints = 16;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "PCIIROS_ErrorIds.hpp"
#include    "PCIIROS_MessageIds.hpp"
#include    "PCIIROS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facPCIIROS();

