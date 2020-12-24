//
// FILE NAME: IntfVCtrlS_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/01/2004
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
//  This is the main file for the CQC Interface Viewer control driver, which
//  allows users to move the cursor and select buttons and such on the CQC
//  interface viewer (mostly to allow them to control it via IR remote.)
//
// CAVEATS/GOTCHAS:
//
//  1)  We don't have a public header, we just have this private one. The
//      reason for this is that the IDL generator will spit out an include
//      of a private header for any non-Exe facility and we use an IDL
//      interface. And we don't need both public and private because we are
//      a dynamically loaded library, so we just have a private one.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"


namespace kIntfVCtrlS
{
    // -----------------------------------------------------------------------
    //  The version info for this driver (client and server sides, which we'll
    //  always keep in sync.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "IntfVCtrlS_ErrorIds.hpp"
#include    "IntfVCtrlS_MessageIds.hpp"
#include    "IntfVCtrlS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facIntfVCtrlS();


