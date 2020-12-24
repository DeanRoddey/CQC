//
// FILE NAME: HAIOmniTCP2S_DriverImpl4.cpp
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
//  This file contains grunt work methods from the main implementation file,
//  because this driver is far too large for one file. This one contains the
//  code for responding to writes to our command field. This driver has a
//  lot of possible commands, so it's a good bit of code.
//
//  It also contains the field registration method, which is large because
//  of the number of fields.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2S.hpp"


// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to handle a write to the InvokeCmd field. We parse the
//  passed information and take the requested action.
//
//  These are standard Security device class commands:
//
//  ArmArea : aaa,mmm,ccc   - Arm the named area with the named mode, using code ccc.
//                              If aaa=$All$, then all areas.
//  DisarmArea : aaa, ccc   - Disarm the named area using code ccc
//
//
//  These are driver specific commands:
//
//  AreaAllOff : aaa            - Turn off the units in the area. If aaa=$All$,
//                                then all units in all areas.
//  AreaAllOn : aaa             - Turn on all units in the numbered area. If
//                                aaa : $All$, then all units in all areas.
//  BypassZone : aaa,zzz,ccc    - Bypass zone named zzz, using code ccc from area
//                                aaa.
//  ClearAllMsgs : aaa          - Clears all messages in the indicated area. If
//                                aaa : $All$, then for all areas.
//  ClearMsg : mmm              - Clears the indicated message number mmm in all areas.
//  ExecButton : bbb            - Invoke the button number bbb
//  PhoneMsg : ddd,mmm          - Phones the indicated message number (the index of the
//                                configured phone number, not the literal one)
//                                and says the indicated message.
//  RawCmd : ccc,111,222        - Allows you to send a raw command. You provide the
//                                cmd number and the two parameter values. You are
//                                responsible for making sure the values are valid
//  RestoreZone : aaa,zzz,ccc   - Restore zone named zzz, using code ccc from area
//                                aaa.
//  SayMsg : mmm                - Say the message number mmm
//  SendAudioKC : zzz,ccc       - Sends Audio Key Code ccc to zone zzz
//  SetRoom : rrr,sss           - Sets the state for a room. rr is the room number
//                                which will be converted to a unit number by
//                                multiplying by 8. ls is the state, so either Off
//                                or On.
//  SyncTime                    - Sets the Omni time to the server's current time
//  SetZoneSrc : zzz,sss        - Sets zone zzz to source number sss
//  SetZoneVol : zzz,ppp        - Sets zone zzz to percent volume ppp
//  ShowMsg : mmm               - Show the message number mmm
//  UnitPercent : uuu,ppp       - Sets unit uuu's level to ppp percent (0 to 100.)
//  UnitOff : uuu,ttt
//  UnitOn : uuu,ttt            - Turn off/on the unit named uuu for time ttt.
//                                ttt is interpreted as:
//                                  0        : Off or on unconditionally
//                                  1..99    : For 1 to 99 seconds
//                                  101..199 : For 1 to 99 minutes
//                                  201..218 : For 1 to 18 hours
//  UnitDim : uuu,iii,sss       - Increment or decrement the brightness of unit
//  UnitBri : uuu,iii,sss           uuu. The inc/dec value is 1 to 9, and represents
//                                a 10% adjustment up or down. The change occurs
//                                over sss seconds, pass zero for immediate change.
//                                sss is clipped to 99 seconds;
//  UPBLink : lll,sss           - Activates or deactivates UPB link lll. sss is
//                                the state, 1 for activate and 0 for deactivate.
//  UPBLEDOn : uuu,lll          - Sets the state of a UPB LED. LED lll (1 to 8) is
//  UPBLEDOff : uuu,lll             turned on or off for unit uuu.
//
//
tCQCKit::ECommResults THAIOmniTCP2S::eProcessUserCmd(const TString& strCmdStr)
{
    tCIDLib::TCard4 c4Count(0);
    TString         strCmd;
    TString         strP1;
    TString         strP2;
    TString         strP3;
    TString         strP4;

    TStringTokenizer stokCmd(&strCmdStr, L",:");

    // Get the command and the up to four parms
    if (stokCmd.bGetNextToken(strCmd))
    {
        c4Count++;
        if (stokCmd.bGetNextToken(strP1))
        {
            c4Count++;
            if (stokCmd.bGetNextToken(strP2))
            {
                c4Count++;
                if (stokCmd.bGetNextToken(strP3))
                {
                    c4Count++;
                    if (stokCmd.bGetNextToken(strP4))
                        c4Count++;
                }
            }
        }
    }

    // We had to at least get the command
    if (!c4Count)
    {
        facHAIOmniTCP2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcFld_BadCmdFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
        return tCQCKit::ECommResults::UnknownCmd;
    }

    // Strip the whitespace off the parms
    strCmd.StripWhitespace();
    strP1.StripWhitespace();
    strP2.StripWhitespace();
    strP3.StripWhitespace();
    strP4.StripWhitespace();

    //
    //  Some are special cases and don't use the simple two value command
    //  sequence that all the others below use. So we handle these separately.
    //
    if (strCmd.bCompareI(L"SyncTime"))
    {
        // Get the current time
        TTime tmTmp(tCIDLib::ESpecialTimes::CurrentTime);

        // Break out it out into it's separate components
        tCIDLib::TCard4         c4Year;
        tCIDLib::EMonths        eMonth;
        tCIDLib::TCard4         c4Day;
        tCIDLib::TCard4         c4Hour;
        tCIDLib::TCard4         c4Minute;
        tCIDLib::TCard4         c4Second;
        tCIDLib::TCard4         c4Millis;
        tCIDLib::TEncodedTime   enctStamp;

        tCIDLib::EWeekDays eWeekDay = tmTmp.eExpandDetails
        (
            c4Year
            , eMonth
            , c4Day
            , c4Hour
            , c4Minute
            , c4Second
            , c4Millis
            , enctStamp
        );

        // Build up the message to send
        THeapBufPool::TElemJan janWrite(&m_splBufs, 64);
        TMemBuf& mbufWrite = *janWrite;

        tCIDLib::TCard4 c4Index = 0;
        mbufWrite.PutCard1(kHAIOmniTCP2S::c1STX, c4Index++);
        mbufWrite.PutCard1(8, c4Index++);
        mbufWrite.PutCard1(kHAIOmniTCP2S::c1MsgType_SetTime, c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(c4Year - 2000), c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(eMonth) + 1, c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(c4Day), c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(eWeekDay) + 1, c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(c4Hour), c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(c4Minute), c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(TTime::bDST() ? 1 : 0), c4Index++);

        const tCIDLib::TCard2 c2CRC = c2CalcCRC(mbufWrite);
        mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC & 0xFF), c4Index++);
        mbufWrite.PutCard1(tCIDLib::TCard1(c2CRC >> 8), c4Index++);

        // And send the message and wait for an ack
        SendPacket(mbufWrite, kHAIOmniTCP2S::c1PackType_OmniMsg, c4Index);
        bWaitAck(kCIDLib::True);

        return tCQCKit::ECommResults::Success;
    }

    //
    //  Convert the standard Security class' DisarmArea to our ArmArea command,
    //  with a mode of Off. Move the code to parameter 3.
    //
    if (strCmd.bCompareI(L"DisarmArea"))
    {
        strCmd = L"ArmArea";
        strP3 = strP2;
        strP2 = L"Off";
    }

    //
    //  For the rest, based on the command we interpret the parameters and
    //  fill in the command and p1, p2 values.
    //
    tCIDLib::TCard1 c1Cmd;
    tCIDLib::TCard1 c1P1;
    tCIDLib::TCard2 c2P2;

    if (strCmd.bCompareI(L"AreaAllOff") || strCmd.bCompareI(L"AreaAllOn"))
    {
        if (strCmd.bCompareI(L"AreaAllOff"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_AllOff;
        else
            c1Cmd = kHAIOmniTCP2S::c1Cmd_AllOn;
        c1P1 = 0;

        // Either all areas or a particular named area
        if (strP1.bCompareI(L"$All$"))
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strP1);
            c2P2 = pitemArea->c2ItemNum();
        }
    }
     else if (strCmd.bCompareI(L"ArmArea")
          ||  strCmd.bCompareI(L"BypassZone")
          ||  strCmd.bCompareI(L"RestoreZone"))
    {
        // Either all areas or a particular named area
        if (strP1.bCompareI(L"$All$"))
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strP1);
            c2P2 = pitemArea->c2ItemNum();
        }

        // Validate the code
        if (!bValidateCode(strP3, c2P2, c1P1))
        {
            facHAIOmniTCP2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcFld_BadCode
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
            return tCQCKit::ECommResults::CmdParms;
        }

        if (strCmd.bCompareI(L"ArmArea"))
        {
            //
            //  Set the security mode enum from the text value, and use the
            //  ordinal to offset the first command of the group of arm areas.
            //
            tHAIOmniTCP2Sh::EArmModes eMode = tHAIOmniTCP2Sh::eXlatEArmModes(strP2);

            if (eMode >= tHAIOmniTCP2Sh::EArmModes::Count)
            {
                facHAIOmniTCP2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCP2SErrs::errcFld_BadCmdParm
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(1)
                );
                return tCQCKit::ECommResults::CmdParms;
            }

            // The command is the base disarm code plus the mode index
            c1Cmd = kHAIOmniTCP2S::c1Cmd_DisarmArea + tCIDLib::TCard1(eMode);
        }
         else
        {
            //
            //  Look up the indicated zone. In this case, we go back and
            //  overwrite P2 with the zone, since it's not needed anymore
            //  now that the code is validated.
            //
            THAIOmniZone* pitemZone = m_dcfgCurrent.pitemFindZone(strP2);
            c2P2 = pitemZone->c2ItemNum();

            if (strCmd.bCompareI(L"BypassZone"))
                c1Cmd = kHAIOmniTCP2S::c1Cmd_BypassZone;
            else
                c1Cmd = kHAIOmniTCP2S::c1Cmd_RestoreZone;
        }
    }
     else if (strCmd.bCompareI(L"ClearAllMsgs"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_ClearMessage;
        c1P1 = 0;
        if (strP1.bCompareI(L"$All$"))
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniZone* pitemZone = m_dcfgCurrent.pitemFindZone(strP1);
            c2P2 = pitemZone->c2ItemNum();
        }
    }
     else if (strCmd.bCompareI(L"ClearMsg"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_ClearMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"ExecButton"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_ExecButton;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"PhoneMsg"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_PhoneMessage;
        c1P1 = tCIDLib::TCard1(strP1.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP2.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"RawCmd"))
    {
        c1Cmd = tCIDLib::TCard1(strP1.c4Val(tCIDLib::ERadices::Dec));
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP3.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"SayMsg"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_SayMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"SendAudioKC"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_SendAudioKeyCode;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"ShowMsg"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_ShowMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"SetRoom"))
    {
        if (strP2.bCompareI(L"Off"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitOff;
        else if (strP1.bCompareI(L"On"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitOn;
        else
        {
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcFld_BadCmdParm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(2)
            );
        }

        // Make sure the room is a legal number
        tCIDLib::TCard4 c4Room;
        if (!strP1.bToCard4(c4Room)
        ||  !c4Room
        ||  ((((c4Room - 1) * 8) + 1) > m_c4MaxUnits))
        {
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcFld_BadCmdParm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(1)
            );
        }

        // Convert the room to a unit number and we are done
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(((c4Room - 1) * 8) + 1);
    }
     else if (strCmd.bCompareI(L"SetZoneSrc"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_SetZoneSource;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"SetZoneVol"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_SetZoneVolume;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 100)
            c1P1 = 100;
    }
     else if (strCmd.bCompareI(L"UnitPercent"))
    {
        c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitLightLev;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 100)
            c1P1 = 100;

        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = pitemUnit->c2ItemNum();
    }
     else if (strCmd.bCompareI(L"UnitOff") || strCmd.bCompareI(L"UnitOn"))
    {
        if (strCmd.bCompareI(L"UnitOff"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitOff;
        else
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitOn;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));

        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = pitemUnit->c2ItemNum();
    }
     else if (strCmd.bCompareI(L"UnitDim") || strCmd.bCompareI(L"UnitBri"))
    {
        if (strCmd.bCompareI(L"UnitBri"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitBri;
        else
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UnitDim;

        //
        //  Get the inc/dec amount and clip back if invalid, then add to the
        //  command.
        //
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 9)
            c1P1 = 9;
        c1Cmd += c1P1;

        // Get the seconds and clip if needed
        c1P1 = tCIDLib::TCard1(strP3.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 99)
            c1P1 = 99;

        // And get the unit number
        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = pitemUnit->c2ItemNum();
    }
     else if (strCmd.bCompareI(L"UPBLinkOff") || strCmd.bCompareI(L"UPBLinkOn"))
    {
        if (strCmd.bCompareI(L"UPBLinkOff"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UPBLinkOff;
        else
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UPBLinkOn;

        // Get the link number out. P1 is not used
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd.bCompareI(L"UPBLEDOn") || strCmd.bCompareI(L"UPBLEDOff"))
    {
        if (strCmd.bCompareI(L"UPBLEDOn"))
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UPBLEDOn;
        else
            c1Cmd = kHAIOmniTCP2S::c1Cmd_UPBLEDOff;

        // Get the LED number out. If not 1 to 8, reject it
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 8)
        {
            facHAIOmniTCP2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCP2SErrs::errcFld_BadLEDNum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        // And get the unit number out
        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = pitemUnit->c2ItemNum();
    }
     else
    {
        facHAIOmniTCP2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kOmniTCP2SErrs::errcFld_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmd
        );
        return tCQCKit::ECommResults::UnknownCmd;
    }

    // Looks ok, so send the info we got
    SendOmniCmd(c1Cmd, c1P1, c2P2);
    bWaitAck(kCIDLib::True);

    return tCQCKit::ECommResults::Success;
}


//
//  This is called to register our fields. This is mostly driven by the
//  configuration data, but we have some global stuff as well.
//
tCIDLib::TVoid THAIOmniTCP2S::RegisterFields()
{
    tCIDLib::TCard4     c4Count;
    tCIDLib::TCard4     c4Index;
    TVector<TCQCFldDef> colFlds(128);
    TCQCFldDef          flddNew;
    TString             strName;

    // Reset the first/last field id values
    m_c4FirstAreaFldId = kCIDLib::c4MaxCard;
    m_c4FirstThermoFldId = kCIDLib::c4MaxCard;
    m_c4FirstUnitFldId = kCIDLib::c4MaxCard;
    m_c4FirstZoneFldId = kCIDLib::c4MaxCard;
    m_c4LastAreaFldId = kCIDLib::c4MaxCard;
    m_c4LastThermoFldId = kCIDLib::c4MaxCard;
    m_c4LastUnitFldId = kCIDLib::c4MaxCard;
    m_c4LastZoneFldId = kCIDLib::c4MaxCard;

    //
    //  First do the ones that we know are always going to be there all
    //  the time.
    //
    flddNew.Set
    (
        L"InvokeCmd"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"MainBattery"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"TimeValid"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    // Do the device info fields
    TCQCDevClass::c4LoadDevInfoFlds(colFlds, TString::strEmpty());

    // Do the expansion enclosure fields
    strName = L"Encl";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxAreas; c4Index++)
    {
        strName.CapAt(4);
        strName.AppendFormatted(c4Index + 1);

        strName.Append(L"ACPowerOff");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"BatteryLow");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"CommFailure");
        flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);

        strName.CapAt(5);
        strName.Append(L"BatteryLevel");
        flddNew.Set(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
        colFlds.objAdd(flddNew);
    }

    // Do the area status fields
    c4Count = m_dcfgCurrent.c4AreaCnt();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            THAIOmniArea& itemCur = m_dcfgCurrent.itemAreaAt(c4Index);
            itemCur.c4QueryFldDefs(colFlds, flddNew, m_bDoCelsius);
        }
    }

    //
    //  Do the thermostat related fields for the thermostats. By doing all
    //  the items for a given thermo at a time, we insure that all the field
    //  ids for a given thermo are contiguous.
    //
    c4Count = m_dcfgCurrent.c4ThermoCnt();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const THAIOmniThermo& itemCur = m_dcfgCurrent.itemThermoAt(c4Index);
            itemCur.c4QueryFldDefs(colFlds, flddNew, m_bDoCelsius);
        }
    }

    // Do the unit fields
    c4Count = m_dcfgCurrent.c4UnitCnt();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const THAIOmniUnit& itemCur = m_dcfgCurrent.itemUnitAt(c4Index);
            itemCur.c4QueryFldDefs(colFlds, flddNew, m_bDoCelsius);
        }
    }


    //
    //  Do the zone status fields, for defined zones. By doing all the items
    //  for a given zone at a time, we insure that all the field ids for a
    //  given zone are contiguous. THE ORDER IS IMPORTANT since we store
    //  first/last ids for a given zone below when we look up the ids.
    //
    //  There can be 175 of these, though it's not actually likely to be
    //  anywhere near that. But we put the zone number into a 3 digit field.
    //
    c4Count = m_dcfgCurrent.c4ZoneCnt();
    if (c4Count)
    {
        for (c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const THAIOmniZone& itemCur = m_dcfgCurrent.itemZoneAt(c4Index);
            itemCur.c4QueryFldDefs(colFlds, flddNew, m_bDoCelsius);
        }
    }

    //
    //  And now register our fields with the base class and go back andlook up
    //  their ids.
    //
    SetFields(colFlds);

    m_c4FldId_InvokeCmd = pflddFind(L"InvokeCmd", kCIDLib::True)->c4Id();
    m_c4FldId_MainBattery = pflddFind(L"MainBattery", kCIDLib::True)->c4Id();
    m_c4FldId_TimeValid = pflddFind(L"TimeValid", kCIDLib::True)->c4Id();

    m_c4FldId_Firmware = flddFind(tCQCKit::EDevClasses::DeviceInfo, L"Firmware").c4Id();
    m_c4FldId_Model = flddFind(tCQCKit::EDevClasses::DeviceInfo, L"Model").c4Id();

    tCIDLib::TCard4 c4FirstId, c4LastId;

    // And now all of the item bsaed stuff
    c4Count = m_dcfgCurrent.c4AreaCnt();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        THAIOmniArea& itemCur = m_dcfgCurrent.itemAreaAt(c4Index);
        itemCur.StoreFldIds(*this);
    }

    strName = L"Encl";
    for (c4Index = 0; c4Index < kHAIOmniTCP2Sh::c4MaxEnclosures; c4Index++)
    {
        strName.CapAt(4);
        strName.AppendFormatted(c4Index + 1);

        strName.Append(L"ACPowerOff");
        m_ac4FldId_EnclACOff[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"BatteryLow");
        m_ac4FldId_EnclBattLow[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"CommFailure");
        m_ac4FldId_EnclCommFail[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName.CapAt(5);
        strName.Append(L"BatteryLevel");
        m_ac4FldId_EnclBattery[c4Index] = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    c4Count = m_dcfgCurrent.c4ThermoCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        THAIOmniThermo& itemCur = m_dcfgCurrent.itemThermoAt(c4Index);
        itemCur.StoreFldIds(*this);

        itemCur.QueryFirstLastId(c4FirstId, c4LastId);
        if (m_c4FirstThermoFldId == kCIDLib::c4MaxCard)
            m_c4FirstThermoFldId = c4FirstId;
        m_c4LastThermoFldId = c4LastId;
    }

    c4Count = m_dcfgCurrent.c4UnitCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        THAIOmniUnit& itemCur = m_dcfgCurrent.itemUnitAt(c4Index);
        itemCur.StoreFldIds(*this);

        itemCur.QueryFirstLastId(c4FirstId, c4LastId);
        if (m_c4FirstUnitFldId == kCIDLib::c4MaxCard)
            m_c4FirstUnitFldId = c4FirstId;
        m_c4LastUnitFldId = c4LastId;
    }

    c4Count = m_dcfgCurrent.c4ZoneCnt();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        THAIOmniZone& itemCur = m_dcfgCurrent.itemZoneAt(c4Index);
        itemCur.StoreFldIds(*this);
        itemCur.QueryFirstLastId(c4FirstId, c4LastId);

        // If we've not stored the first zone field id yet, store it
        if (m_c4FirstZoneFldId == kCIDLib::c4MaxCard)
            m_c4FirstZoneFldId = c4FirstId;

        // Store the last zone field id so far
        m_c4LastZoneFldId = c4LastId;
    }
}


