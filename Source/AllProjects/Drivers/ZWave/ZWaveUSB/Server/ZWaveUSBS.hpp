//
// FILE NAME: ZWaveUSBS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/02/2005
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
//  This is the main file for the Z-Wave USB controller driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_ZWAVEUSBS)
#define ZWAVEUSBSEXPORT    CID_DLLEXPORT
#else
#define ZWAVEUSBSEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "ZWaveUSBSh.hpp"


// ---------------------------------------------------------------------------
//  Our constants namespace
// ---------------------------------------------------------------------------
namespace kZWaveUSBS
{
    // -----------------------------------------------------------------------
    //  The config server scope under which we store our module config data.
    //  We get it via remote control replication, but need to store it for
    //  subsequent restarts. The %(m) is replaced with the moniker, so that
    //  there can be multiple Z-Wave drivers on the same host.
    //
    //  As of V2.1, we updated the Z-Wave driver and we dump the old config
    //  during upgrade, and so we had to create a new scope name so as to
    //  recognize which is which.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszOldCfgScope  = L"/ZWave/Config/%(m)/";
    const tCIDLib::TCh* const   pszCfgScope     = L"/ZWave2/%(m)/ConfigInfo";


    // -----------------------------------------------------------------------
    //  The time we'll wait between the last poll reply and the next poll
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctPollInt = kCIDLib::enctOneMilliSec * 500;


    // -----------------------------------------------------------------------
    //  The minimium inter-message send interval
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctSendInt = kCIDLib::enctOneMilliSec * 50;
}


// ---------------------------------------------------------------------------
//  Our types namespace
// ---------------------------------------------------------------------------
namespace tZWaveUSBS
{
    // -----------------------------------------------------------------------
    //  The values returned from the eGetMsg() method of the driver, which
    //  indicates what we got.
    // -----------------------------------------------------------------------
    enum EMsgTypes
    {
        EMsgType_Timeout
        , EMsgType_Nak
        , EMsgType_Ack
        , EMsgType_CallBack
        , EMsgType_Response
    };
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "ZWaveUSBS_ErrorIds.hpp"
#include    "ZWaveUSBS_MessageIds.hpp"
#include    "ZWaveUSBS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method
// ---------------------------------------------------------------------------
extern TFacility& facZWaveUSBS();



// ---------------------------------------------------------------------------
//  Some verbose logging heleprs
// ---------------------------------------------------------------------------
#define ZWLogDbgMsg(msg) ZWLogDbg \
( \
    CID_FILE \
    , CID_LINE \
    , msg \
    , MFormattable::Nul_MFormattable() \
    , MFormattable::Nul_MFormattable() \
    , MFormattable::Nul_MFormattable() \
)

#define ZWLogDbgMsg1(msg,f1) ZWLogDbg \
( \
    CID_FILE \
    , CID_LINE \
    , msg \
    , f1 \
    , MFormattable::Nul_MFormattable() \
    , MFormattable::Nul_MFormattable() \
)

#define ZWLogDbgMsg2(msg,f1,f2) ZWLogDbg \
( \
    CID_FILE \
    , CID_LINE \
    , msg \
    , f1 \
    , f2 \
    , MFormattable::Nul_MFormattable() \
)

#define ZWLogDbgMsg3(msg,f1,f2,f3) ZWLogDbg(CID_FILE, CID_LINE, msg, f1, f2, f3)

