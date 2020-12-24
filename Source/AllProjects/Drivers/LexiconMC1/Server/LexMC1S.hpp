//
// FILE NAME: LexMC1CS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2000
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
//  This is the main file for the Lexicon MC-1/DC-2 driver.
//
// CAVEATS/GOTCHAS:
//
//  1)  This driver assumes a particular level of Lexicon software, which
//      is provided in some constants below. We'll check when we connect
//      that the software is at that level or greater.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDComm.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Our facility constants namespace
// ---------------------------------------------------------------------------
namespace kLexMC1S
{
    // -----------------------------------------------------------------------
    //  These are the base names of our client and server side driver
    //  facilities.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName    = L"LexMC1C";
    const tCIDLib::TCh* const   pszServerFacName    = L"LexMC1S";


    // -----------------------------------------------------------------------
    //  Version constants for the minimum Lexicon software version we support
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4LexMajVersion     = 4;
    const tCIDLib::TCard4       c4LexMinVersion     = 0;


    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion        = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion        = CID_MINVER;
    const tCIDLib::TCard4       c4Revision          = CID_REVISION;
}



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "LexMC1S_ErrorIds.hpp"
#include    "LexMC1S_MessageIds.hpp"
#include    "LexMC1S_Types.hpp"
#include    "LexMC1S_Constants.hpp"
#include    "LexMC1S_Msg.hpp"
#include    "LexMC1S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facLexMC1S();


