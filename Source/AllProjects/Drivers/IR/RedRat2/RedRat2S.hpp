//
// FILE NAME: RedRat2S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This is the main file for the RedRat2 server side driver.
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
#include    "CQCIR.hpp"



// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tRedRat2S
{
    // -----------------------------------------------------------------------
    //  This enum is returned from eWaitForMsg() and indicates what we got
    //  back from the device, if anything.
    // -----------------------------------------------------------------------
    enum class EResults
    {
        Unknown
        , BadFmt
        , NoReply
        , ErrorCode
        , Firmware
        , Ack
        , SignalData
        , EPROMValue

        , Count
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kRedRat2S
{
    // -----------------------------------------------------------------------
    // Constants for the command bytes that the RedRat uses
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1Cmd_Firmware  = 0x49;
    const tCIDLib::TCard1   c1Cmd_Blink     = 0x42;
    const tCIDLib::TCard1   c1Cmd_ErrorCode = 0x21;
    const tCIDLib::TCard1   c1Cmd_EPROM     = 0x45;
    const tCIDLib::TCard1   c1Cmd_SignalCap = 0x53;
    const tCIDLib::TCard1   c1Cmd_TermCap   = 0x54;
    const tCIDLib::TCard1   c1Cmd_Transmit  = 0x50;


    // -----------------------------------------------------------------------
    // Constants for some special bytes in the protocol
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1Proto_SOM     =  0x5B;
    const tCIDLib::TCard1   c1Proto_EOM     =  0x5D;
    const tCIDLib::TCard1   c1Proto_CR      =  0x0D;
    const tCIDLib::TCard1   c1Proto_LF      =  0x0A;
    const tCIDLib::TCard1   c1Proto_Prompt  =  0x3E;
    const tCIDLib::TCard1   c1Proto_Divider =  0x23;


    // -----------------------------------------------------------------------
    //  The base name of our server side driver
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"RedRat2S";


    // -----------------------------------------------------------------------
    //  The version info for this facility. These values come in from the build
    //  environment because all of our CQC drivers are given a version number
    //  equal to the CQC release they are in.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion            = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion            = CID_MINVER;
    const tCIDLib::TCard4   c4Revision              = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "RedRat2S_ErrorIds.hpp"
#include    "RedRat2S_MessageIds.hpp"
#include    "RedRat2S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facRedRat2S();


