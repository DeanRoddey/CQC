//
// FILE NAME: ZWaveLevi2Sh.hpp
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
#if     defined(PROJ_ZWAVELEVI2SH)
#define ZWLEVI2SHEXPORT  CID_DLLEXPORT
#else
#define ZWLEVI2SHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CIDXML.hpp"

#include "CQCKit.hpp"
#include "CQCDriver.hpp"



// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kZWaveLevi2Sh
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
    //  The names of some of the fixed fields that the client driver will
    //  refer to directly by name.
    // -----------------------------------------------------------------------
    const TString   strFld_InvokeCmd(L"InvokeCmd");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the InvokeCmd field, used
    //  to send some simple commands to our driver. They are here because the
    //  client side driver may invoke some of these.
    // -----------------------------------------------------------------------
    const TString   strDrvCmd_AddAssoc(L"ADDASSOC");
    const TString   strDrvCmd_AllOff(L"ALLOFF");
    const TString   strDrvCmd_AllOn(L"ALLON");
    const TString   strDrvCmd_AssocToVRCOP(L"ASSOCTOVRCOP");
    const TString   strDrvCmd_DelAllAssoc(L"DELALLASSOC");
    const TString   strDrvCmd_DelAssoc(L"DELASSOC");
    const TString   strDrvCmd_EnableTrace(L"ENABLETRACE");
    const TString   strDrvCmd_SetCfgParam(L"SETCFGPARAM");
    const TString   strDrvCmd_SetWakeupSecs(L"SETWAKEUPSECS");

#if 0
    const TString   strDrvCmd_DelGroup(L"DELGROUP");
    const TString   strDrvCmd_Group(L"GROUP");
    const TString   strDrvCmd_GroupLev(L"GROUPLEV");
    const TString   strDrvCmd_ResetCtrl(L"RESETCTRL");
    const TString   strDrvCmd_SetGroupList(L"SETGROUPLIST");
    const TString   strDrvCmd_Unit(L"UNIT");
    const TString   strDrvCmd_UnlinkFromCtl(L"UNLINKFROMCTL");
#endif


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the bSendCmd() backdoor
    //  driver method to send some basic commands to the driver that aren't
    //  done through the field interface.
    // -----------------------------------------------------------------------
    const TString   strSendCmd_AutoCfg(L"AUTOCFG");
    const TString   strSendCmd_StartScan(L"STARTSCAN");


    // -----------------------------------------------------------------------
    //  These are the command ids that we use with the bSendData() backdoor
    //  driver method to send some more complex commands to our driver.
    // -----------------------------------------------------------------------
    const TString   strSendData_SetGrpConfig(L"SETGRPCFG");
    const TString   strSendData_SetUnitConfig(L"SETUNITCFG");


    // -----------------------------------------------------------------------
    //  These are command ids that are used with the c4QueryVal() backdoor
    //  method.
    // -----------------------------------------------------------------------
    const TString   strQCardV_Status(L"QUERYSTATUS");


    // -----------------------------------------------------------------------
    //  These are command ids that are used with the c4QueryVal2() backdoor
    //  method.
    // -----------------------------------------------------------------------
    const TString   strQCardV_CfgParam(L"QUERYCFGPARAM");


    // -----------------------------------------------------------------------
    //  These are command ids that are used with the bQueryTextVal() backdoor
    //  method.
    // -----------------------------------------------------------------------
    const TString   strQTextV_ProbeUnitReport(L"PROBEUNITREPORT");
    const TString   strQTextV_QueryAssoc(L"QUERYASSOC");
}


// ---------------------------------------------------------------------------
//  Bring in our own public headers. Some others have to come in below after
//  the types namespace.
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_ErrorIds.hpp"
#include    "ZWaveLevi2Sh_MessageIds.hpp"
#include    "ZWaveLevi2Sh_Shared.hpp"
#include    "ZWaveLevi2Sh_Option.hpp"
#include    "ZWaveLevi2Sh_CmdQ.hpp"


namespace tZWaveLevi2Sh
{
    // -----------------------------------------------------------------------
    //  A list of option objects, which the command class header needs to see, since
    //  we pass this to them to load up.
    // -----------------------------------------------------------------------
    typedef TVector<TZWOption>              TOptList;


    // -----------------------------------------------------------------------
    //  A queue of commands for storing commands on wakeup type units until they can
    //  be sent. We don't really need a queue, we just use a vector. Generally we will
    //  just send them all and flush it every time we get a wakeup.
    // -----------------------------------------------------------------------
    typedef TVector<TZWQCmd>                TZWCmdQ;


    // -----------------------------------------------------------------------
    //  A hash set of option value objects, which the unit class uses to track all
    //  of the options needed by its command class objects.
    // -----------------------------------------------------------------------
    typedef TKeyedHashSet<TZWOptionVal, TString, TStringKeyOps>  TOptValList;


    // -----------------------------------------------------------------------
    //  A list of command class enums
    // -----------------------------------------------------------------------
    typedef TFundVector<ECClasses>          TClsList;


    // -----------------------------------------------------------------------
    //  For auto-config info in the device info objects
    // -----------------------------------------------------------------------
    typedef TVector<TKeyValues>             TAutoCfgList;
}

#include    "ZWaveLevi2Sh_DevInfo.hpp"
#include    "ZWaveLevi2Sh_CmdClass.hpp"


namespace tZWaveLevi2Sh
{
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
    //  Some commonly used collections
    // -----------------------------------------------------------------------
    typedef TRefVector<TZWCmdClass>         TCmdIntfList;
    typedef TFundArray<EGenTypes>           TGTypeArray;
    typedef TFundVector<tCIDLib::TCard4>    TValList;
}


// ---------------------------------------------------------------------------
//  Bring in some more headers that need the above types
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_Association.hpp"
#include    "ZWaveLevi2Sh_Battery.hpp"
#include    "ZWaveLevi2Sh_ColorSwitch.hpp"
#include    "ZWaveLevi2Sh_Config.hpp"
#include    "ZWaveLevi2Sh_Scene.hpp"
#include    "ZWaveLevi2Sh_EntryCtrl.hpp"
#include    "ZWaveLevi2Sh_Sensors.hpp"
#include    "ZWaveLevi2Sh_SwitchDim.hpp"
#include    "ZWaveLevi2Sh_Thermo.hpp"
#include    "ZWaveLevi2Sh_UnitInfo.hpp"


// ---------------------------------------------------------------------------
//  And some more types that have to come after the above headers
// ---------------------------------------------------------------------------
namespace tZWaveLevi2Sh
{
    // -----------------------------------------------------------------------
    //  Collections of units.
    // -----------------------------------------------------------------------
    typedef TVector<TZWaveUnit>             TUnitList;
}


// ---------------------------------------------------------------------------
//  And some more headers that need to see the above types
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern ZWLEVI2SHEXPORT TFacility& facZWaveLevi2Sh();

