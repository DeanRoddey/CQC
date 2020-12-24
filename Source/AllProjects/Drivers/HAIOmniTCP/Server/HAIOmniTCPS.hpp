//
// FILE NAME: HAIOmniTCPS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This is the main file for the HAI Omni TCP server driver. This driver
//  supports the version 3.0 Omni firmware only, no earlier versions.
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
#include    "HAIOmniTCPSh.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kHAIOmniTCPS
{
    // -----------------------------------------------------------------------
    // Some constant values in messages
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1STX(0x21);
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
    const tCIDLib::TCard1   c1Cmd_SetZoneVolume(113);
    const tCIDLib::TCard1   c1Cmd_SetZoneSource(114);
    const tCIDLib::TCard1   c1Cmd_SendAudioKeyCode(115);


    // -----------------------------------------------------------------------
    //  These are the actual values for the Omni level messages that we send
    //  orreceive.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1MsgType_Ack(0x01);
    const tCIDLib::TCard1   c1MsgType_Nak(0x02);
    const tCIDLib::TCard1   c1MsgType_EndOfData(0x03);
    const tCIDLib::TCard1   c1MsgType_SetTime(0x13);
    const tCIDLib::TCard1   c1MsgType_SendCmd(0x14);
    const tCIDLib::TCard1   c1MsgType_EnableNotify(0x15);
    const tCIDLib::TCard1   c1MsgType_SysInfoReq(0x16);
    const tCIDLib::TCard1   c1MsgType_SysInfoReply(0x17);
    const tCIDLib::TCard1   c1MsgType_SysStatusReq(0x18);
    const tCIDLib::TCard1   c1MsgType_SysStatusReply(0x19);
    const tCIDLib::TCard1   c1MsgType_ObjCapReq(0x1E);
    const tCIDLib::TCard1   c1MsgType_ObjCapRep(0x1F);
//    const tCIDLib::TCard1   c1MsgType_ObjectStatusReq(0x22);
//    const tCIDLib::TCard1   c1MsgType_ObjectStatusRep(0x23);
    const tCIDLib::TCard1   c1MsgType_ValidateCodeReq(0x26);
    const tCIDLib::TCard1   c1MsgType_ValidateCodeReply(0x27);
    const tCIDLib::TCard1   c1MsgType_OtherNotifications(0x37);
    const tCIDLib::TCard1   c1MsgType_ExtObjStatusReq(0x3A);
    const tCIDLib::TCard1   c1MsgType_ExtObjStatusRep(0x3B);


    // -----------------------------------------------------------------------
    //  The system event codes we deal with
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1SysEv_UserButton(0);


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
    const tCIDLib::TCard1   c1PackType_OmniMsg(0x20);


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
    const tCIDLib::TCard1   c1ItemType_AuxSensor(8);
    const tCIDLib::TCard1   c1ItemType_AudioSource(9);
    const tCIDLib::TCard1   c1ItemType_AudioZone(10);
    const tCIDLib::TCard1   c1ItemType_ExpEnclosure(11);
    const tCIDLib::TCard1   c1ItemType_Console(12);


    // -----------------------------------------------------------------------
    //  The smallest a legal message can be
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxMsgSize(211);
    const tCIDLib::TCard4   c4MaxPacketSize(215);
    const tCIDLib::TCard4   c4MinPacketSize(5);


    // -----------------------------------------------------------------------
    // Some offsets of things in Omni messages
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MsgLenOfs(1);
    const tCIDLib::TCard4   c4MsgTypeOfs(2);
    const tCIDLib::TCard4   c4FirstDBOfs(3);
    const tCIDLib::TCard4   c4NameOfs(5);


    // -----------------------------------------------------------------------
    //  Some times that we wait for replies
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4WaitAck = 3000;
    const tCIDLib::TCard4   c4WaitReply = 2500;


    // -----------------------------------------------------------------------
    //  If we haven't seen anything from the Omni in this long, we'll actively
    //  poll it.
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctActivePoll = kCIDLib::enctOneSecond * 30;


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
    //  A value to text map for area security modes. We also have an count
    //  for the last one that's actually used in the arming command. The
    //  'ing' versions are only for current status.
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
    const tCIDLib::TCard4 c4ArmSecModCnt(7);


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
#include    "HAIOmniTCPS_ErrorIds.hpp"
#include    "HAIOmniTCPS_MessageIds.hpp"
#include    "HAIOmniTCPS_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facHAIOmniTCPS();


