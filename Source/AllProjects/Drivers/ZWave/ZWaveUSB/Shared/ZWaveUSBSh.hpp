//
// FILE NAME: ZWaveUSBSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/24/2005
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
//  This is the public facility header. It defines and includes stuff that
//  is publically visible
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_ZWAVEUSBSH)
#define ZWUSBSHEXPORT  CID_DLLEXPORT
#else
#define ZWUSBSHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CQCKit.hpp"
#include "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kZWaveUSBSh
{
    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the c4SendCmd() backdoor
    //  driver method to send some simple commands to our driver.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_AddAssoc(L"AddAssociation");
    const TString   strDrvCmd_DelAssoc(L"DelAssociation");
    const TString   strDrvCmd_DelGroup(L"DelGroup");
    const TString   strDrvCmd_ResetCtrl(L"ResetCtrl");
    const TString   strDrvCmd_SetCfgParam(L"SetCfgParam");
    const TString   strDrvCmd_SetRepMode(L"SetRepMode");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the bSendData() backdoor
    //  driver method to send some more complex commands to our driver.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_SetConfig(L"SetConfig");


    // -----------------------------------------------------------------------
    //  The ids for some values the driver makes available to the client
    //  driver via the backdoor value query methods. As above, we use some
    //  fairly random values, to make it less likely a bogus value will
    //  accidently matcha real value.
    //
    //  DrvStatus - Returns 0 or 1 in low word, indicating whether we are in
    //              replication mode or not. If 1, then high word contains
    //              the ERepRes value that indicates the status of the
    //              replication.
    // -----------------------------------------------------------------------
    const TString   strValId_DrvStatus(L"DrvStatus");
    const TString   strValId_QueryAssoc(L"QueryAssoc");


    // -----------------------------------------------------------------------
    //  The maximum number of units that can be in a Z-Wave network because
    //  this is the maximum unit id.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1MaxUnits  = 232;


    // -----------------------------------------------------------------------
    //  The names of some of the fixed fields that the client driver will
    //  refer to directly by name.
    // -----------------------------------------------------------------------
    const TString   strFld_Command(L"Command");


    // -----------------------------------------------------------------------
    //  Special group names
    // -----------------------------------------------------------------------
    const TString   strGrp_AllOnOff(L"AllOnOff");
}


// ---------------------------------------------------------------------------
//  Bring in our own public headers. Some others have to come in below after
//  the types namespace.
// ---------------------------------------------------------------------------
#include    "ZWaveUSBSh_ErrorIds.hpp"
#include    "ZWaveUSBSh_MessageIds.hpp"
#include    "ZWaveUSBSh_Shared.hpp"
#include    "ZWaveUSBSh_Ctrl.hpp"


// ---------------------------------------------------------------------------
//  Our public types namespace
// ---------------------------------------------------------------------------
namespace tZWaveUSBSh
{
    // -----------------------------------------------------------------------
    //  Unit capabilities flags. The derived unit classes wil set these on the
    //  base unit class to let drivers (client and server) know what this
    //  unit can do.
    //
    //  Pollable    - The unit can be asked for a report. This controls whether
    //                the user can enable polling on this unit.
    //  AsyncNotify - Indicates that this unit can send reports asynchronously.
    //  OnOff       - The unit supports on/off functionality
    //  Level       - The unit supports multi-level functionality
    // -----------------------------------------------------------------------
    enum EUnitCaps
    {
        EUnitCap_None           = 0x0000
        , EUnitCap_Pollable     = 0x0001
        , EUnitCap_AsyncNotify  = 0x0002
        , EUnitCap_OnOff        = 0x0004
        , EUnitCap_Level        = 0x0008

        // Some convenient combos
        , EUnitCaps_BinPoll     = 0x0005
        , EUnitCaps_MLPoll      = 0x000D
        , EUnitCaps_BinAsync    = 0x0006
    };


    // -----------------------------------------------------------------------
    //  When we do matching of class types reported by the controller for a
    //  unit, and what our available classes can handle, this is returned to
    //  indicate how good a match. They must remain in order of increasing
    //  'goodness'.
    // -----------------------------------------------------------------------
    enum EUnitMatch
    {
        EUnitMatch_None
        , EUnitMatch_Generic
        , EUnitMatch_Specific
    };


    // -----------------------------------------------------------------------
    //  An overall status for each unit. The status mean:
    //
    //  Failed    - It is known bad for some reason. It's not polled and any
    //              writes to it are rejected.
    //  Missing   - In the last node map that we got from the controller, this
    //              guy was not there. So we don't try to poll it. We will
    //              periodically get a new node map and update the units. Or
    //              the user can force one.
    //  Error     - The unit failed to respond for the last X number of tries
    //              at polling it. So we've put its fields into error state
    //              until we can get a good poll. We increase the poll time
    //              if it's not already pretty long, so that the timeouts
    //              do not cause delays polling the good units.
    //  Ready     - There is no reason to think that the unit is not ready to
    //              for access. For non-polled units this just means that the
    //              last write command (or the initial query of the value)
    //              worked.
    // -----------------------------------------------------------------------
    enum EUnitStatus
    {
        EUnitStatus_Failed
        , EUnitStatus_Missing
        , EUnitStatus_Error
        , EUnitStatus_Ready
    };


    // -----------------------------------------------------------------------
    //  A list of unit/group ids
    // -----------------------------------------------------------------------
    typedef TFundVector<tCIDLib::TCard1>    TIdList;
    typedef TFundVector<tCIDLib::TCard4>    TFldIdList;
}


// ---------------------------------------------------------------------------
//  Bring in some more headers that need the above types
// ---------------------------------------------------------------------------
#include    "ZWaveUSBSh_UnitInfo.hpp"
#include    "ZWaveUSBSh_GroupInfo.hpp"


// ---------------------------------------------------------------------------
//  And some more types that have to come after the above headers
// ---------------------------------------------------------------------------
namespace tZWaveUSBSh
{
    // -----------------------------------------------------------------------
    //  Collections of units. We look at them polymorphically via the base
    //  unit class.
    // -----------------------------------------------------------------------
    typedef TRefVector<TZWaveUnit>      TUnitList;


    // -----------------------------------------------------------------------
    //  Collections of groups. These we can do monomorphically
    // -----------------------------------------------------------------------
    typedef TVector<TZWaveGroupInfo>    TGroupList;
}


// ---------------------------------------------------------------------------
//  And some more headers that need to see the above types
// ---------------------------------------------------------------------------
#include    "ZWaveUSBSh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  Provide streaming support for some enums
// ---------------------------------------------------------------------------
EnumBinStreamMacros(tZWaveUSBSh::EUnitStatus)


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern ZWUSBSHEXPORT TFacility& facZWaveUSBSh();


