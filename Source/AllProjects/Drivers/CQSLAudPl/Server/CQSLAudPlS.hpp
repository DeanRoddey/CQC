//
// FILE NAME: CQSLAudPlS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2007
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
//  This is the main header file for the driver. This driver implements a
//  headless audio player that can run in the background. It is based on
//  the WMP COM interfaces (via a wrapper class in CQCMedia), and also
//  optionally supports the Quick Time interface as well, which is also
//  available via a wrapper (derived from the same base class so we can
//  treat them the same.)
//
//  It checks the extension of a file and selects the correct wrapped
//  engine. It only uses the Quick Time engine if the file is an M4P or
//  M4A or AAC file. Otherwise it uses the WMP engine.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "CQSLAudPlS_ErrorIds.hpp"
#include    "CQSLAudPlS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "CQSLAudPlS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQSLAudPlS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"CQSLAudPlS";


    // -----------------------------------------------------------------------
    //  The text for the transport field's values
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszTransportMap[] =
    {
        L"Next"
        , L"Pause"
        , L"Play"
        , L"Prev"
        , L"Stop"
    };
    const tCIDLib::TCard4    c4MaxTransport = tCIDLib::c4ArrayElems(apszTransportMap);


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
extern TFacility& facCQSLAudPlS();

