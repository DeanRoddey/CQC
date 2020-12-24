//
// FILE NAME: ZWaveLeviS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
#if     defined(PROJ_ZWAVELEVIS)
#define ZWAVELEVISEXPORT    CID_DLLEXPORT
#else
#define ZWAVELEVISEXPORT    CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include underlying needed headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "ZWaveLeviSh.hpp"



// ---------------------------------------------------------------------------
//  Our constants namespace
// ---------------------------------------------------------------------------
namespace kZWaveLeviS
{
    // -----------------------------------------------------------------------
    //  The config server scope under which we store our module config data.
    //  We get it by querying the available modules and their type info. The
    //  m token is replaced with our moniker.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszCfgScope     = L"/ZWaveLevi/%(m)/ConfigInfo";


    // -----------------------------------------------------------------------
    //  The maximum number of commands we'll allow in the queue.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MaxQSize = 32;


    // -----------------------------------------------------------------------
    //  Some device result codes that we care about
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4ResCode_NoDevFound = 10;


    // -----------------------------------------------------------------------
    //  The minimum millseconds between poll rounds
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MinPollRndMSs = 2000;


    // -----------------------------------------------------------------------
    //  The minimum time we will enforce between outgoing messages.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MinSendInterval = 200;
}


// ---------------------------------------------------------------------------
//  Our types namespace
// ---------------------------------------------------------------------------
namespace tZWaveLeviS
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
    //  The type of messages we get from the controller
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
#include    "ZWaveLeviS_ErrorIds.hpp"
#include    "ZWaveLeviS_MessageIds.hpp"
#include    "ZWaveLeviS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method
// ---------------------------------------------------------------------------
extern TFacility& facZWaveLeviS();


