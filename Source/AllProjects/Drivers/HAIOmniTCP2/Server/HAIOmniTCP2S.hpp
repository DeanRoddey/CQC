//
// FILE NAME: HAIOmniTCP2S.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  Version 1.1 - 4.4.919
//      Updated to deal with the late breaking V2 architecture change related to
//      multi-unit field name prefixes using an tilde.
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
#include    "HAIOmniTCP2Sh.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kHAIOmniTCP2S
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
    //  or receive.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard1   c1MsgType_Ack(0x01);
    const tCIDLib::TCard1   c1MsgType_Nak(0x02);
    const tCIDLib::TCard1   c1MsgType_EndOfData(0x03);
    const tCIDLib::TCard1   c1MsgType_ReadName(0x0D);
    const tCIDLib::TCard1   c1MsgType_NameData(0x0E);
    const tCIDLib::TCard1   c1MsgType_SetTime(0x13);
    const tCIDLib::TCard1   c1MsgType_SendCmd(0x14);
    const tCIDLib::TCard1   c1MsgType_EnableNotify(0x15);
    const tCIDLib::TCard1   c1MsgType_SysInfoReq(0x16);
    const tCIDLib::TCard1   c1MsgType_SysInfoReply(0x17);
    const tCIDLib::TCard1   c1MsgType_SysStatusReq(0x18);
    const tCIDLib::TCard1   c1MsgType_SysStatusReply(0x19);
    const tCIDLib::TCard1   c1MsgType_ObjCapReq(0x1E);
    const tCIDLib::TCard1   c1MsgType_ObjCapRep(0x1F);
    const tCIDLib::TCard1   c1MsgType_ObjPropReq(0x20);
    const tCIDLib::TCard1   c1MsgType_ObjPropRep(0x21);
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
}


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2S_ErrorIds.hpp"
#include    "HAIOmniTCP2S_MessageIds.hpp"
#include    "HAIOmniTCP2S_DriverImpl.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method.
// ---------------------------------------------------------------------------
extern TFacility& facHAIOmniTCP2S();


