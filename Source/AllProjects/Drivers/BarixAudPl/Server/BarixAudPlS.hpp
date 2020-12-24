//
// FILE NAME: BarixAudPlS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/07/2007
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
//  streaming audio player that streams out data to a Barix ExStreamer device
//  on the network. We work in terms of the usual media repository and when
//  asked to play a file, we read it in, decode it to raw PCM data and send
//  it out to the Barix.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDDAE.hpp"

#include    "CQCKit.hpp"

#include    "CQCDriver.hpp"

#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "BarixAudPlS_ErrorIds.hpp"
#include    "BarixAudPlS_MessageIds.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kBarixAudPlS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"BarixAudPlS";


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    // RTP related constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1       c1PLType        = 10;
    const tCIDLib::TCard4       c4PayloadSz     = 880;
    const tCIDLib::TCard4       c4PayloadTime   = 220;
    const tCIDLib::TCard4       c4RoundDelay    = 4989;
    const tCIDLib::TEncodedTime enctRoundDelay  = 4989 * 10;


    // -----------------------------------------------------------------------
    //  The amount of data we'll load into each buffer. We might go a little
    //  over so they are allocated large enough to handle that. And the number
    //  of buffer's we'll round robin through. This gives us 2MB of data
    //  cached up, which is about 10 seconds of data, in case of a disruption
    //  in I/O we won't run dry.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4SpoolBufSz    = 0xFFFF;
    const tCIDLib::TCard4   c4SpoolBufCnt   = 32;


    // -----------------------------------------------------------------------
    //  Now long the driver thread will wait for the spooler thread to respond
    //  to a queued command.
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    const tCIDLib::TCard4   c4WaitCmd = 60000;
    #else
    const tCIDLib::TCard4   c4WaitCmd = 5000;
    #endif
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "BarixAudPlS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facBarixAudPlS();

