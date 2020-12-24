//
// FILE NAME: ZWaveLeviSh.hpp
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
#if     defined(PROJ_ZWAVELEVISH)
#define ZWLEVISHEXPORT  CID_DLLEXPORT
#else
#define ZWLEVISHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CQCKit.hpp"
#include "CQCDriver.hpp"



// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kZWaveLeviSh
{
    // -----------------------------------------------------------------------
    //  The maximum number of units that the Leviton unit can support.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxUnits  = 128;


    // -----------------------------------------------------------------------
    //  The maximum id for a unit.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxUnitId = 232;


    // -----------------------------------------------------------------------
    //  The maximum id for a group.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxGroupId = 255;


    // -----------------------------------------------------------------------
    //  The maximum number of seconds in a unit's poll interval
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPollSecs = 120;


    // -----------------------------------------------------------------------
    //  The names of some of the fixed fields that the client driver will
    //  refer to directly by name.
    // -----------------------------------------------------------------------
    const TString   strFld_Command(L"Command");
    const TString   strFld_PollsDue(L"UnitPollsDue");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the Command field, used
    //  to send some simple commands to our driver. They are here because the
    //  client side driver may invoke some of these.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_AddAssoc(L"ADDASSOC");
    const TString   strDrvCmd_AllOff(L"ALLOFF");
    const TString   strDrvCmd_AllOn(L"ALLON");
    const TString   strDrvCmd_AssocToCtl(L"ASSOCTOCTL");
    const TString   strDrvCmd_DelAssoc(L"DELASSOC");
    const TString   strDrvCmd_DelGroup(L"DELGROUP");
    const TString   strDrvCmd_Group(L"GROUP");
    const TString   strDrvCmd_GroupLev(L"GROUPLEV");
    const TString   strDrvCmd_LogMsgs(L"LOGMSGS");
    const TString   strDrvCmd_ResetCtrl(L"RESETCTRL");
    const TString   strDrvCmd_SetCfgParam(L"SETCFGPARAM");
    const TString   strDrvCmd_SetGroupList(L"SETGROUPLIST");
    const TString   strDrvCmd_SetRepMode(L"SETREPMODE");
    const TString   strDrvCmd_SetSetPnt(L"SETSETPNT");
    const TString   strDrvCmd_Unit(L"UNIT");
    const TString   strDrvCmd_UnlinkFromCtl(L"UNLINKFROMCTL");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the bSendCmd() backdoor
    //  driver method to send some basic commands to the driver that aren't
    //  done through the field interface.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_StartScan(L"STARTSCAN");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the bSendData() backdoor
    //  driver method to send some more complex commands to our driver.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_SetGrpConfig(L"SETGRPCFG");
    const TString   strDrvCmd_SetUnitConfig(L"SETUNITCFG");


    // -----------------------------------------------------------------------
    //  These are command ids that are used with the c4QueryVal() backdoor
    //  method.
    // -----------------------------------------------------------------------
    const TString   strDrvQ_ConfigSerialNum(L"QUERYCFGSERNUM");


    // -----------------------------------------------------------------------
    //  Some strings used by the 'build set msg' method of the unit classes.
    //  Various types of units respond to various of these commands.
    // -----------------------------------------------------------------------
    const TString   strGetMsg_Off(L"Off");
    const TString   strGetMsg_On(L"On");

}


// ---------------------------------------------------------------------------
//  Bring in our own public headers. Some others have to come in below after
//  the types namespace.
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_ErrorIds.hpp"
#include    "ZWaveLeviSh_MessageIds.hpp"
#include    "ZWaveLeviSh_Shared.hpp"
#include    "ZWaveLeviSh_Ctrl.hpp"


// ---------------------------------------------------------------------------
//  Our public types namespace
// ---------------------------------------------------------------------------
namespace tZWaveLeviSh
{
    // -----------------------------------------------------------------------
    //  Unit capabilities flags. The derived unit classes wil set these on the
    //  base unit class to let drivers (client and server) know what this
    //  unit can do.
    //
    //  Readable    - The unit can be asked for a report.
    //  AsyncNotify - Indicates that this unit can send reports asynchronously
    //                associations.
    //  BasicClass  - The unit supports basic GET/SET/REPORT functionality via
    //                the basic class.
    //  Secure      - Supports secure functionality
    //  Writeable   - It can be written to to change its state. Else it is
    //                read only.
    //  Battery     - Battery powered, so it's generally not available to
    //                get the value of.
    // -----------------------------------------------------------------------
    enum EUnitCaps
    {
        EUnitCap_None               = 0x0000
        , EUnitCap_Readable         = 0x0001
        , EUnitCap_AsyncNotify      = 0x0002
        , EUnitCap_BasicClass       = 0x0004
        , EUnitCap_Secure           = 0x0008
        , EUnitCap_Writeable        = 0x0010
        , EUnitCap_Battery          = 0x0020
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
    //              writes to it are rejected. For instance, it fails to
    //              accept its field ids, so we can't store data.
    //  Missing   - The last time we loaded the configuration (and checked for
    //              available modules) we didn't find this guy. So it is marked
    //              as missing and no fields are created for it.
    //  Error     - It is readable but has not responded to a number of queries
    //              for status, so it's been marked in error. The fields will
    //              have been put into error state.
    //  Reeady    - We found the unit during configuration loading/reloading,
    //              and it has not yet given us any reason to believe that
    //              it is in any trouble.
    // -----------------------------------------------------------------------
    enum EUnitStatus
    {
        EUnitStatus_Failed
        , EUnitStatus_Missing
        , EUnitStatus_Error
        , EUnitStatus_Ready
    };


    // -----------------------------------------------------------------------
    //  A list of unit/group ids or other values
    // -----------------------------------------------------------------------
    typedef TFundArray<tCIDLib::TCard4>     TValArray;
    typedef TFundVector<tCIDLib::TCard4>    TValList;
}


// ---------------------------------------------------------------------------
//  Bring in some more headers that need the above types
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_UnitInfo.hpp"
#include    "ZWaveLeviSh_Controller.hpp"
#include    "ZWaveLeviSh_EntryCtrl.hpp"
#include    "ZWaveLeviSh_Sensor.hpp"
#include    "ZWaveLeviSh_SwitchDim.hpp"
#include    "ZWaveLeviSh_Thermo.hpp"


// ---------------------------------------------------------------------------
//  And some more types that have to come after the above headers
// ---------------------------------------------------------------------------
namespace tZWaveLeviSh
{
    // -----------------------------------------------------------------------
    //  Collections of units. We look at them polymorphically via the base
    //  unit class.
    // -----------------------------------------------------------------------
    typedef TRefVector<TZWaveUnit>      TUnitList;
}


// ---------------------------------------------------------------------------
//  And some more headers that need to see the above types
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  Provide streaming support for some enums
// ---------------------------------------------------------------------------
EnumBinStreamMacros(tZWaveLeviSh::EUnitCaps)
EnumBinStreamMacros(tZWaveLeviSh::EUnitStatus)


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern ZWLEVISHEXPORT TFacility& facZWaveLeviSh();


