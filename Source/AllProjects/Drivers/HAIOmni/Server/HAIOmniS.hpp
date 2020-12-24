//
// FILE NAME: HAIOmniS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2005
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
//  This is the main file for the HAI Omni ProII server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDSock.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "HAIOmniSh.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kHAIOmniS
{
    // -----------------------------------------------------------------------
    // Some constant values in messages
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1STX(0x5A);
    const tCIDLib::TCard2   c2CRCPoly(0xA001);


    // -----------------------------------------------------------------------
    //  The message that sends a command to the Omni takes these values to
    //  indicate what command is being sent.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1Cmd_UnitOff(0);
    const tCIDLib::TCard1   c1Cmd_UnitOn(1);
    const tCIDLib::TCard1   c1Cmd_AllOff(2);
    const tCIDLib::TCard1   c1Cmd_AllOn(3);
    const tCIDLib::TCard1   c1Cmd_BypassZone(4);
    const tCIDLib::TCard1   c1Cmd_RestoreZone(5);
    const tCIDLib::TCard1   c1Cmd_ExecButton(7);
    const tCIDLib::TCard1   c1Cmd_SetEnergyCost(8);
    const tCIDLib::TCard1   c1Cmd_UnitLightLev(9);
    const tCIDLib::TCard1   c1Cmd_SetCounter(12);
    const tCIDLib::TCard1   c1Cmd_UnitDim(16);
    const tCIDLib::TCard1   c1Cmd_UPBLinkOff(28);
    const tCIDLib::TCard1   c1Cmd_UPBLinkOn(29);
    const tCIDLib::TCard1   c1Cmd_UnitBri(32);
    const tCIDLib::TCard1   c1Cmd_UPBLEDOff(44);
    const tCIDLib::TCard1   c1Cmd_UPBLEDOn(45);
    const tCIDLib::TCard1   c1Cmd_DisarmArea(48);
    const tCIDLib::TCard1   c1Cmd_ArmAreaDay(49);
    const tCIDLib::TCard1   c1Cmd_ArmAreaNight(50);
    const tCIDLib::TCard1   c1Cmd_ArmAreaAway(51);
    const tCIDLib::TCard1   c1Cmd_ArmAreaVacation(52);
    const tCIDLib::TCard1   c1Cmd_ArmAreaDayInstant(53);
    const tCIDLib::TCard1   c1Cmd_ArmAreaNightDelay(54);
    const tCIDLib::TCard1   c1Cmd_SetHeatSP(66);
    const tCIDLib::TCard1   c1Cmd_SetCoolSP(67);
    const tCIDLib::TCard1   c1Cmd_SetThermoMode(68);
    const tCIDLib::TCard1   c1Cmd_SetThermoFanMode(69);
    const tCIDLib::TCard1   c1Cmd_SetThermoHoldMode(70);
    const tCIDLib::TCard1   c1Cmd_ShowMessage(80);
    const tCIDLib::TCard1   c1Cmd_ClearMessage(82);
    const tCIDLib::TCard1   c1Cmd_SayMessage(83);
    const tCIDLib::TCard1   c1Cmd_PhoneMessage(84);


    // -----------------------------------------------------------------------
    //  These are the actual values for the Omni level messages that we send
    //  orreceive.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1MsgType_EndOfData(0x03);
    const tCIDLib::TCard1   c1MsgType_Ack(0x05);
    const tCIDLib::TCard1   c1MsgType_Nak(0x06);
    const tCIDLib::TCard1   c1MsgType_NameInfo(0x0B);
    const tCIDLib::TCard1   c1MsgType_UploadNames(0x0C);
    const tCIDLib::TCard1   c1MsgType_SendCmd(0x0F);
    const tCIDLib::TCard1   c1MsgType_SysInfo(0x11);
    const tCIDLib::TCard1   c1MsgType_SysInfoReply(0x12);
    const tCIDLib::TCard1   c1MsgType_SysStatus(0x13);
    const tCIDLib::TCard1   c1MsgType_SysStatusReply(0x14);
    const tCIDLib::TCard1   c1MsgType_ZoneStatus(0x15);
    const tCIDLib::TCard1   c1MsgType_ZoneStatusReply(0x16);
    const tCIDLib::TCard1   c1MsgType_ReqUnitStatus(0x17);
    const tCIDLib::TCard1   c1MsgType_UnitStatusReply(0x18);
    const tCIDLib::TCard1   c1MsgType_ReqAuxStatus(0x19);
    const tCIDLib::TCard1   c1MsgType_AuxStatusReply(0x1A);
    const tCIDLib::TCard1   c1MsgType_ReqThermoStatus(0x1E);
    const tCIDLib::TCard1   c1MsgType_ThermoReply(0x1F);
    const tCIDLib::TCard1   c1MsgType_Login(0x20);
    const tCIDLib::TCard1   c1MsgType_Logout(0x21);
    const tCIDLib::TCard1   c1MsgType_ReqSysEvents(0x22);
    const tCIDLib::TCard1   c1MsgType_SysEventsReply(0x23);
    const tCIDLib::TCard1   c1MsgType_CodeValid(0x26);
    const tCIDLib::TCard1   c1MsgType_CodeValidReply(0x27);


    // -----------------------------------------------------------------------
    //  These are the packet types (which wrap the actual Omni data.)
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1PackType_None(0);
    const tCIDLib::TCard1   c1PackType_ReqNewSess(1);
    const tCIDLib::TCard1   c1PackType_AckNewSess(2);
    const tCIDLib::TCard1   c1PackType_ReqSecSess(3);
    const tCIDLib::TCard1   c1PackType_AckSecSess(4);
    const tCIDLib::TCard1   c1PackType_TermSess(5);
    const tCIDLib::TCard1   c1PackType_AckTermSess(6);
    const tCIDLib::TCard1   c1PackType_NakNewSess(7);
    const tCIDLib::TCard1   c1PackType_OmniMsg(16);


    // -----------------------------------------------------------------------
    // The values for the item types that the Omni supports
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1ItemType_Zone(1);
    const tCIDLib::TCard1   c1ItemType_Unit(2);
    const tCIDLib::TCard1   c1ItemType_Button(3);
    const tCIDLib::TCard1   c1ItemType_Code(4);
    const tCIDLib::TCard1   c1ItemType_Area(5);
    const tCIDLib::TCard1   c1ItemType_Thermostat(6);
    const tCIDLib::TCard1   c1ItemType_Message(7);


    // -----------------------------------------------------------------------
    // The smallest a legal message can be
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxMsgSize(211);
    const tCIDLib::TCard4   c4MinMsgSize(5);
    const tCIDLib::TCard4   c4MaxPacketSize(215);
    const tCIDLib::TCard4   c4MinPacketSize(4);


    // -----------------------------------------------------------------------
    // Some offsets of things in Omni messages
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MsgLenOfs(1);
    const tCIDLib::TCard4   c4MsgTypeOfs(2);
    const tCIDLib::TCard4   c4FirstDBOfs(3);
    const tCIDLib::TCard4   c4NameOfs(5);



    // -----------------------------------------------------------------------
    //  A text map for the alarm types
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszAlarmTypes[] =
    {
        L"Burglary"
        , L"Fire"
        , L"Gas"
        , L"Auxillary"
        , L"Freeze"
        , L"Water"
        , L"Duress"
        , L"Temperatur"
        , L"Unknown"
    };
    const tCIDLib::TCard4 c4AlarmTypeCnt = tCIDLib::c4ArrayElems(apszAlarmTypes);


    // -----------------------------------------------------------------------
    //  A value to text map for area security modes
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszSecModes[] =
    {
        L"Off"
        , L"Day"
        , L"Night"
        , L"Away"
        , L"Vacation"
        , L"DayInstant"
        , L"NightDelay"
        , L"ArmingDay"
        , L"ArmingNight"
        , L"ArmingAway"
        , L"ArmingVacation"
        , L"ArmingDayInstant"
        , L"ArmingNightDelay"
        , L"Unknown"
    };
    const tCIDLib::TCard4 c4SecModeCnt = tCIDLib::c4ArrayElems(apszSecModes);


    // -----------------------------------------------------------------------
    //  A value to text map for thermo modes
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszThermoModes[] =
    {
        L"Off"
        , L"Heat"
        , L"Cool"
        , L"Auto"
        , L"EmergencyHeat"
        , L"Unknown"
    };
    const tCIDLib::TCard4 c4ThermoModeCnt = tCIDLib::c4ArrayElems(apszThermoModes);


    // -----------------------------------------------------------------------
    //  A value to text map for thermo fan modes
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszThermoFanModes[] =
    {
        L"Auto"
        , L"On"
        , L"Unknown"
    };
    const tCIDLib::TCard4 c4ThermoFanModeCnt = tCIDLib::c4ArrayElems(apszThermoFanModes);


    // -----------------------------------------------------------------------
    //  A value to text map for zone arming status
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszZoneArming[] =
    {
        L"Disarmed"
        , L"Armed"
        , L"Bypassed by User"
        , L"Bypassed by System"
    };
    const tCIDLib::TCard4 c4ZoneArmingCnt = tCIDLib::c4ArrayElems(apszZoneArming);


    // -----------------------------------------------------------------------
    //  A value to text map for zone status
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszZoneStatus[] =
    {
        L"Secure"
        , L"Not Ready"
        , L"Trouble"
    };
    const tCIDLib::TCard4 c4ZoneStatusCnt = tCIDLib::c4ArrayElems(apszZoneStatus);


    // -----------------------------------------------------------------------
    //  A value to text map for latched zone status
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszZoneLatched[] =
    {
        L"Secure"
        , L"Tripped"
        , L"Reset"
    };
    const tCIDLib::TCard4 c4ZoneLatchedCnt = tCIDLib::c4ArrayElems(apszZoneLatched);
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "HAIOmniS_ErrorIds.hpp"
#include    "HAIOmniS_MessageIds.hpp"
#include    "HAIOmniS_Item.hpp"
#include    "HAIOmniS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facHAIOmniS();


