//
// FILE NAME: ZWaveLevi2S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
//  This is the main file for the Leviton Z-Wave controller driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_ZWAVELEVI2S)
#define ZWAVELEVI2SEXPORT   CID_DLLEXPORT
#else
#define ZWAVELEVI2SEXPORT   CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "ZWaveLevi2Sh.hpp"



// ---------------------------------------------------------------------------
//  Our constants namespace
// ---------------------------------------------------------------------------
namespace kZWaveLevi2S
{
    // -----------------------------------------------------------------------
    //  The maximum number of commands we'll allow in the queue.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MaxQSize = 32;


    // -----------------------------------------------------------------------
    //  Some device result codes that we care about
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4ResCode_NoDevFound = 10;


    // -----------------------------------------------------------------------
    //  The number of seconds between our poll method checking for a unit to
    //  poll.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4PollSecs   = 15;


    // -----------------------------------------------------------------------
    //  The minimum time we will enforce between outgoing messages.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MinSendInterval = 200;


    // -----------------------------------------------------------------------
    //  How many millis we wait for responses before giving up
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4WaitTimeout = 4000;
}


// ---------------------------------------------------------------------------
//  Our types namespace
// ---------------------------------------------------------------------------
namespace tZWaveLevi2S
{
    // -----------------------------------------------------------------------
    //  The types of commands that we can queue up for the bgn thread to
    //  process. This is used by the TZWQCmd class.
    // -----------------------------------------------------------------------
    enum EQCmdTypes
    {
        EQCmdType_FldWrite

        , EQCmdTypes_Count
    };


    // -----------------------------------------------------------------------
    //  The type of messages we get from the controller.
    // -----------------------------------------------------------------------
    enum ERepTypes
    {
        ERepType_None
        , ERepType_BasicReport
        , ERepType_Error
        , ERepType_GenReport
        , ERepType_LearnMode
        , ERepType_MsgAck
        , ERepType_SceneReport
        , ERepType_SecStatus
        , ERepType_TransAck
        , ERepType_TransNak
        , ERepType_UnitID
        , ERepType_UnitName

        , ERepType_MemoryActivity
        , ERepType_ReloadDevices

        , ERepTypes_Count
    };
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2S_ErrorIds.hpp"
#include    "ZWaveLevi2S_MessageIds.hpp"
#include    "ZWaveLevi2S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method
// ---------------------------------------------------------------------------
extern TFacility& facZWaveLevi2S();


