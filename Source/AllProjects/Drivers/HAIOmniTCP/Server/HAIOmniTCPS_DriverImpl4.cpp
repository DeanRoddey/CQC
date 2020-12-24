//
// FILE NAME: HAIOmniTCPS_DriverImpl4.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/23/2008
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
#include    "HAIOmniTCPS.hpp"



//
//  This is called to handle a write to the InvokeCmd field. We parse the
//  passed information and take the requested action.
//
//  AreaAllOff=aaa          - Turn off the units in the area. If aaa=$All$,
//                            then all units in all areas.
//  AreaAllOn=aaa           - Turn on all units in the numbered area. If
//                            aaa=$All$, then all units in all areas.
//  ArmArea=aaa,mmm,ccc     - Arm the named area with the named mode,
//                            using code ccc. If aaa=$All$, then all areas.
//  BypassZone=aaa,zzz,ccc  - Bypass zone named zzz, using code ccc from area
//                            aaa.
//  ClearAllMsgs=aaa        - Clears all messages in the indicated area. If
//                            aaa=$All$, then for all areas.
//  ClearMsg=mmm            - Clears the indicated message number mmm in all areas.
//  ExecButton=bbb          - Invoke the button number bbb
//  PhoneMsg=ddd,mmm        - Phones the indicated message number (the index of the
//                            configured phone number, not the literal one)
//                            and says the indicated message.
//  RawCmd=ccc,111,222      - Allows you to send a raw command. You provide the
//                            cmd number and the two parameter values. You are
//                            responsible for making sure the values are valid
//  RestoreZone=aaa,zzz,ccc - Restore zone named zzz, using code ccc from area
//                            aaa.
//  SayMsg=mmm              - Say the message number mmm
//  SendAudioKC=zzz,ccc     - Sends Audio Key Code ccc to zone zzz
//  SetRoom=rrr,sss         - Sets the state for a room. rr is the room number
//                            which will be converted to a unit number by
//                            multiplying by 8. ls is the state, so either Off
//                            or On.
//  SyncTime                 - Sets the Omni time to the server's current time
//  SetZoneSrc=zzz,sss      - Sets zone zzz to source number sss
//  SetZoneVol=zzz,ppp      - Sets zone zzz to percent volume ppp
//  ShowMsg=mmm             - Show the message number mmm
//  UnitPercent=uuu,ppp     - Sets unit uuu's level to ppp percent (0 to 100.)
//  UnitOff=uuu,ttt
//  UnitOn=uuu,ttt          - Turn off/on the unit named uuu for time ttt.
//                            ttt is interpreted as:
//                              0        : Off or on unconditionally
//                              1..99    : For 1 to 99 seconds
//                              101..199 : For 1 to 99 minutes
//                              201..218 : For 1 to 18 hours
//  UnitDim=uuu,iii,sss     - Increment or decrement the brightness of unit
//  UnitBri=uuu,iii,sss       uuu. The inc/dec value is 1 to 9, and represents
//                            a 10% adjustment up or down. The change occurs
//                            over sss seconds, pass zero for immediate change.
//                            sss is clipped to 99 seconds;
//  UPBLink=lll,sss         - Activates or deactivates UPB link lll. sss is
//                            the state, 1 for activate and 0 for deactivate.
//  UPBLEDOn=uuu,lll        - Sets the state of a UPB LED. LED lll (1 to 8) is
//  UPBLEDOff=uuu,lll         turned on or off for unit uuu.
//
//
tCIDLib::TVoid THAIOmniTCPSDriver::ProcessUserCmd(const TString& strCmdStr)
{
    tCIDLib::TCard4 c4Count(0);
    TString         strCmd;
    TString         strP1;
    TString         strP2;
    TString         strP3;
    TString         strP4;

    TStringTokenizer stokCmd(&strCmdStr, L",=");

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
        facHAIOmniTCPS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPSErrs::errcFld_BadCmdFmt
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
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
    if (strCmd == L"SyncTime")
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
        mbufWrite.PutCard1(kHAIOmniTCPS::c1STX, c4Index++);
        mbufWrite.PutCard1(8, c4Index++);
        mbufWrite.PutCard1(kHAIOmniTCPS::c1MsgType_SetTime, c4Index++);
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
        SendPacket(mbufWrite, kHAIOmniTCPS::c1PackType_OmniMsg, c4Index);
        bWaitAck(kCIDLib::True);

        return;
    }

    //
    //  For the rest, based on the command we interpret the paramaters and
    //  fill in the command and p1, p2 values.
    //
    tCIDLib::TCard1 c1Cmd;
    tCIDLib::TCard1 c1P1;
    tCIDLib::TCard2 c2P2;

    if ((strCmd == L"AreaAllOff") || (strCmd == L"AreaAllOn"))
    {
        if (strCmd == L"AreaAllOff")
            c1Cmd = kHAIOmniTCPS::c1Cmd_AllOff;
        else
            c1Cmd = kHAIOmniTCPS::c1Cmd_AllOn;
        c1P1 = 0;

        // Either all areas or a particular named area
        if (strP1 == "$All$")
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strP1);
            c2P2 = tCIDLib::TCard2(pitemArea->c4ItemNum());
        }
    }
     else if ((strCmd == L"ArmArea")
          ||  (strCmd == L"BypassZone")
          ||  (strCmd == L"RestoreZone"))
    {
        // Either all areas or a particular named area
        if (strP1 == "$All$")
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strP1);
            c2P2 = tCIDLib::TCard2(pitemArea->c4ItemNum());
        }

        // Validate the code
        if (!bValidateCode(strP3, c2P2, c1P1))
        {
            facHAIOmniTCPS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCPSErrs::errcFld_BadCode
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }

        if (strCmd == L"ArmArea")
        {
            //
            //  Set the security mode enum from the text value, and use the
            //  ordinal to offset the first command of the group of arm areas.
            //
            tCIDLib::TCard1 c1Mode = 0;
            while (c1Mode < kHAIOmniTCPS::c4SecModeCnt - 1)
            {
                if (strP2 == kHAIOmniTCPS::apszSecModes[c1Mode])
                    break;
                c1Mode++;
            }

            if (c1Mode == kHAIOmniTCPS::c4SecModeCnt - 1)
            {
                // It cannot be 'unknown'
                facHAIOmniTCPS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCPSErrs::errcFld_BadCmdParm
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , TCardinal(1)
                );
            }

            // The command is the base disarm code plus the mode index
            c1Cmd = kHAIOmniTCPS::c1Cmd_DisarmArea + c1Mode;
        }
         else
        {
            //
            //  Look up the indicated zone. In this case, we go back and
            //  overwrite P2 with the zone, since it's not needed anymore
            //  now that the code is validated.
            //
            THAIOmniZone* pitemZone = m_dcfgCurrent.pitemFindZone(strP2);
            c2P2 = tCIDLib::TCard2(pitemZone->c4ItemNum());

            if (strCmd == L"BypassZone")
                c1Cmd = kHAIOmniTCPS::c1Cmd_BypassZone;
            else
                c1Cmd = kHAIOmniTCPS::c1Cmd_RestoreZone;
        }
    }
     else if (strCmd == L"ClearAllMsgs")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_ClearMessage;
        c1P1 = 0;
        if (strP1 == L"$All$")
        {
            c2P2 = 0;
        }
         else
        {
            THAIOmniZone* pitemZone = m_dcfgCurrent.pitemFindZone(strP1);
            c2P2 = tCIDLib::TCard2(pitemZone->c4ItemNum());
        }
    }
     else if (strCmd == L"ClearMsg")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_ClearMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"ExecButton")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_ExecButton;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"PhoneMsg")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_PhoneMessage;
        c1P1 = tCIDLib::TCard1(strP1.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP2.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"RawCmd")
    {
        c1Cmd = tCIDLib::TCard1(strP1.c4Val(tCIDLib::ERadices::Dec));
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP3.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"SayMsg")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_SayMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"SendAudioKC")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_SendAudioKeyCode;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"ShowMsg")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_ShowMessage;
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"SetRoom")
    {
        if (strP2.bCompareI(L"Off"))
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitOff;
        else if (strP1.bCompareI(L"On"))
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitOn;
        else
        {
            facHAIOmniTCPS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCPSErrs::errcFld_BadCmdParm
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
            facHAIOmniTCPS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCPSErrs::errcFld_BadCmdParm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
                , TCardinal(1)
            );
        }

        // Convert the room to a unit number and we are done
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(((c4Room - 1) * 8) + 1);
    }
     else if (strCmd == L"SetZoneSrc")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_SetZoneSource;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if (strCmd == L"SetZoneVol")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_SetZoneVolume;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 100)
            c1P1 = 100;
    }
     else if (strCmd == L"UnitPercent")
    {
        c1Cmd = kHAIOmniTCPS::c1Cmd_UnitLightLev;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 100)
            c1P1 = 100;

        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = tCIDLib::TCard2(pitemUnit->c4ItemNum());
    }
     else if ((strCmd == L"UnitOff") || (strCmd == L"UnitOn"))
    {
        if (strCmd == L"UnitOff")
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitOff;
        else
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitOn;
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));

        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = tCIDLib::TCard2(pitemUnit->c4ItemNum());
    }
     else if ((strCmd == L"UnitDim") || (strCmd == L"UnitBri"))
    {
        if (strCmd == L"UnitBri")
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitBri;
        else
            c1Cmd = kHAIOmniTCPS::c1Cmd_UnitDim;

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
        c2P2 = tCIDLib::TCard2(pitemUnit->c4ItemNum());
    }
     else if ((strCmd == L"UPBLinkOff") || (strCmd == L"UPBLinkOn"))
    {
        if (strCmd == L"UPBLinkOff")
            c1Cmd = kHAIOmniTCPS::c1Cmd_UPBLinkOff;
        else
            c1Cmd = kHAIOmniTCPS::c1Cmd_UPBLinkOn;

        // Get the link number out. P1 is not used
        c1P1 = 0;
        c2P2 = tCIDLib::TCard2(strP1.c4Val(tCIDLib::ERadices::Dec));
    }
     else if ((strCmd == L"UPBLEDOn") || (strCmd == L"UPBLEDOff"))
    {
        if (strCmd == L"UPBLEDOn")
            c1Cmd = kHAIOmniTCPS::c1Cmd_UPBLEDOn;
        else
            c1Cmd = kHAIOmniTCPS::c1Cmd_UPBLEDOff;

        // Get the LED number out. If not 1 to 8, reject it
        c1P1 = tCIDLib::TCard1(strP2.c4Val(tCIDLib::ERadices::Dec));
        if (c1P1 > 8)
        {
            facHAIOmniTCPS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kOmniTCPSErrs::errcFld_BadLEDNum
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        // And get the unit number out
        THAIOmniUnit* pitemUnit = m_dcfgCurrent.pitemFindUnit(strP1);
        c2P2 = tCIDLib::TCard2(pitemUnit->c4ItemNum());
    }
     else
    {
        facHAIOmniTCPS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kOmniTCPSErrs::errcFld_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmd
        );
    }

    // Looks ok, so send the info we got
    SendOmniCmd(c1Cmd, c1P1, c2P2);
    bWaitAck(kCIDLib::True);
}


//
//  This is called to register our fields. This is mostly driven by the
//  configuration data, but we have some global stuff as well.
//
tCIDLib::TVoid THAIOmniTCPSDriver::RegisterFields()
{
    tCIDLib::TCard4     c4Index;
    TVector<TCQCFldDef> colFlds(64);
    TCQCFldDef          flddNew;
    TString             strLim1;
    TString             strLim2;
    TString             strLim3;
    TString             strLim4;
    TString             strLim5;
    TString             strFmt;
    TString             strFmt2;
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
    //  We need current temp and h/c setpoint limits a number of times below,
    //  and they change depending on the temp scale chosen.
    //
    TString strTempLim;
    TString strSPLim;
    if (m_bDoCelsius)
    {
        strSPLim = L"Range: -18, 50";
        strTempLim = L"Range: -40, 87";
    }
     else
    {
        strSPLim = L"Range: 0, 122";
        strTempLim = L"Range: -40, 189";
    }

    // Set up some other common limits used below
    const TString strPerLim(L"Range:0, 100");
    const TString strFlagLim(L"Range:0, 255");

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
        L"SoftwareVersion"
        , tCQCKit::EFldTypes::String
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


    // Do the expansion enclosure fields
    strName = L"Encl";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
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
    {
        strLim1 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4SecModeCnt; c4Index++)
        {
            if (c4Index)
                strLim1.Append(L",");
            strLim1.Append(kHAIOmniTCPS::apszSecModes[c4Index]);
        }

        m_c4CntAreas++;

        const tCIDLib::TCard4 c4Count = kHAIOmniTCPSh::c4MaxAreas;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // See if we have an item at this slot
            THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt(c4Index);
            if (!pitemCur)
                continue;

            // And create the fields for this area
            strName = pitemCur->strName();
            strName.Append(L"_Alarmed");
            flddNew.Set
            (
                strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read
            );
            colFlds.objAdd(flddNew);

            strName = pitemCur->strName();
            strName.Append(L"_AlarmBits");
            flddNew.Set
            (
                strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read
            );
            colFlds.objAdd(flddNew);

            strName = pitemCur->strName();
            strName.Append(L"_AlarmList");
            flddNew.Set
            (
                strName, tCQCKit::EFldTypes::StringList, tCQCKit::EFldAccess::Read
            );
            colFlds.objAdd(flddNew);

            strName = pitemCur->strName();
            strName.Append(L"_Status");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::Read
                , strLim1
            );
            colFlds.objAdd(flddNew);
        }
    }

    //
    //  Do the thermostat related fields for the thermostats. By doing all
    //  the items for a given thermo at a time, we insure that all the field
    //  ids for a given thermo are contiguous. THE ORDER IS IMPORTANT since
    //  we store first/last ids for a given thermo below when we look up the
    //  ids.
    //
    {
        strLim1 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4ThermoFanModeCnt; c4Index++)
        {
            if (c4Index)
                strLim1.Append(L",");
            strLim1.Append(kHAIOmniTCPS::apszThermoFanModes[c4Index]);
        }

        strLim2 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4ThermoModeCnt; c4Index++)
        {
            if (c4Index)
                strLim2.Append(L",");
            strLim2.Append(kHAIOmniTCPS::apszThermoModes[c4Index]);
        }

        const tCIDLib::TCard4 c4Count = kHAIOmniTCPSh::c4MaxThermos;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // See if we have a thermo at this slot
            THAIOmniThermo* pitemCur = m_dcfgCurrent.pitemThermoAt(c4Index);
            if (!pitemCur)
                continue;

            m_c4CntThermos++;

            // Do the hold status temp
            strName = pitemCur->strName();
            strName.Append(L"_Hold");
            flddNew.Set(strName, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::ReadWrite);
            colFlds.objAdd(flddNew);

            // Do the current temp
            strName = pitemCur->strName();
            strName.Append(L"_CurTemp");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::Read
                , tCQCKit::EFldSTypes::CurTemp
                , strTempLim
            );
            colFlds.objAdd(flddNew);

            // Do the cool set point
            strName = pitemCur->strName();
            strName.Append(L"_CoolSP");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::ReadWrite
                , tCQCKit::EFldSTypes::LowSetPnt
                , strSPLim
            );
            colFlds.objAdd(flddNew);

            // Do the heat set point
            strName = pitemCur->strName();
            strName.Append(L"_HeatSP");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::Int
                , tCQCKit::EFldAccess::ReadWrite
                , tCQCKit::EFldSTypes::HighSetPnt
                , strSPLim
            );
            colFlds.objAdd(flddNew);

            // Do the fan mode
            strName = pitemCur->strName();
            strName.Append(L"_FanMode");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::ReadWrite
                , strLim1
            );
            colFlds.objAdd(flddNew);

            // Do the thermo mode
            strName = pitemCur->strName();
            strName.Append(L"_Mode");
            flddNew.Set
            (
                strName
                , tCQCKit::EFldTypes::String
                , tCQCKit::EFldAccess::ReadWrite
                , strLim2
            );
            colFlds.objAdd(flddNew);
        }
    }

    // Do the unit fields
    {
        const tCIDLib::TCard4 c4Count = kHAIOmniTCPSh::c4MaxUnits;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // See if we have a unit at this slot
            THAIOmniUnit* pitemCur = m_dcfgCurrent.pitemUnitAt(c4Index);
            if (!pitemCur)
                continue;

            m_c4CntUnits++;

            // Do the value field
            strName = pitemCur->strName();
            if (pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Binary)
            {
                strName.Append(L"_State");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Boolean
                    , tCQCKit::EFldAccess::ReadWrite
                    , tCQCKit::EFldSTypes::LightSwitch
                );
            }
             else
            {
                // It's either a dimer or flag
                const TString* pstrLim;
                if (pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Dimmer)
                    pstrLim = &strPerLim;
                else
                    pstrLim = &strFlagLim;

                strName.Append(L"_Value");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Card
                    , tCQCKit::EFldAccess::ReadWrite
                    , tCQCKit::EFldSTypes::Dimmer
                    , *pstrLim
                );
            }
            colFlds.objAdd(flddNew);
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
    {
        strLim1 = L"Range: 0, 255";
        strLim2 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4ZoneArmingCnt; c4Index++)
        {
            if (c4Index)
                strLim2.Append(L",");
            strLim2.Append(kHAIOmniTCPS::apszZoneArming[c4Index]);
        }
        strLim3 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4ZoneLatchedCnt; c4Index++)
        {
            if (c4Index)
                strLim3.Append(L",");
            strLim3.Append(kHAIOmniTCPS::apszZoneLatched[c4Index]);
        }

        strLim4 = L"Enum: ";
        for (c4Index = 0; c4Index < kHAIOmniTCPS::c4ZoneStatusCnt; c4Index++)
        {
            if (c4Index)
                strLim4.Append(L",");
            strLim4.Append(kHAIOmniTCPS::apszZoneStatus[c4Index]);
        }

        const tCIDLib::TCard4 c4Count = kHAIOmniTCPSh::c4MaxZones;
        for (c4Index = 0; c4Index < c4Count; c4Index++)
        {
            // See if we have a zone at this slot
            THAIOmniZone* pitemCur = m_dcfgCurrent.pitemZoneAt(c4Index);
            if (!pitemCur)
                continue;

            m_c4CntZones++;

            //
            //  The zone status field is needed regardless of zone type.
            //  Figure out what semantic type we want to set.
            //
            tCQCKit::EFldSTypes eSemType = tCQCKit::EFldSTypes::Generic;
            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Alarm)
                eSemType = tCQCKit::EFldSTypes::SecZoneStat;
            else if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Motion)
                eSemType = tCQCKit::EFldSTypes::MotionSensor;

            strName = pitemCur->strName();
            strName.Append(L"_Status");

            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Motion)
            {
                // For motion sensors it's a boolean
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Boolean
                    , tCQCKit::EFldAccess::Read
                    , eSemType
                );
            }
             else
            {
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::String
                    , tCQCKit::EFldAccess::Read
                    , eSemType
                    , strLim4
                );
            }
            colFlds.objAdd(flddNew);


            // If an alarm zone, then do the standard zone fields
            if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Alarm)
            {
                // Do the analog loop value
                strName = pitemCur->strName();
                strName.Append(L"_Analog");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Card
                    , tCQCKit::EFldAccess::Read
                    , strLim1
                );
                colFlds.objAdd(flddNew);

                // Do the zone arming status
                strName = pitemCur->strName();
                strName.Append(L"_Arming");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::String
                    , tCQCKit::EFldAccess::Read
                    , strLim2
                );
                colFlds.objAdd(flddNew);

                // Do the latched zone status
                strName = pitemCur->strName();
                strName.Append(L"_Latched");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::String
                    , tCQCKit::EFldAccess::Read
                    , strLim3
                );
                colFlds.objAdd(flddNew);
            }
             else if ((pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Temp)
                  ||  (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Humidity))
            {
                // It's a temp/humidity zone
                tCQCKit::EFldSTypes eSType;
                if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Humidity)
                {
                    strLim5 = strPerLim;
                    eSType = tCQCKit::EFldSTypes::HumSensor;
                }
                 else
                {
                    strLim5 = strTempLim;
                    eSType = tCQCKit::EFldSTypes::TempSensor;
                }

                strName = pitemCur->strName();
                strName.Append(L"_CurVal");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::Read
                    , eSType
                    , strLim5
                );
                colFlds.objAdd(flddNew);

                if (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Humidity)
                    strLim5 = strPerLim;
                else
                    strLim5 = strSPLim;

                strName = pitemCur->strName();
                strName.Append(L"_LowSP");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::ReadWrite
                    , tCQCKit::EFldSTypes::LowSetPnt
                    , strLim5
                );
                colFlds.objAdd(flddNew);

                strName = pitemCur->strName();
                strName.Append(L"_HighSP");
                flddNew.Set
                (
                    strName
                    , tCQCKit::EFldTypes::Int
                    , tCQCKit::EFldAccess::ReadWrite
                    , tCQCKit::EFldSTypes::HighSetPnt
                    , strLim5
                );
                colFlds.objAdd(flddNew);
            }
        }
    }

    //
    //  And now register our fields with the base class and go back andlook up
    //  their ids.
    //
    SetFields(colFlds);

    tCIDLib::TCard4 c4FldId1;
    tCIDLib::TCard4 c4FldId2;
    tCIDLib::TCard4 c4FldId3;
    tCIDLib::TCard4 c4FldId4;
    tCIDLib::TCard4 c4FldId5;
    tCIDLib::TCard4 c4FldId6;
    tCIDLib::TCard4 c4FldId7;

    m_c4FldId_InvokeCmd = pflddFind(L"InvokeCmd", kCIDLib::True)->c4Id();
    m_c4FldId_MainBattery = pflddFind(L"MainBattery", kCIDLib::True)->c4Id();
    m_c4FldId_TimeValid = pflddFind(L"TimeValid", kCIDLib::True)->c4Id();

    // And now all of the item bsaed stuff
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxAreas; c4Index++)
    {
        // See if we have an item at this slot
        THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt(c4Index);
        if (!pitemCur)
            continue;

        strName = pitemCur->strName();
        strName.Append(L"_Alarmed");
        pitemCur->c4FldIdAlarmed(pflddFind(strName, kCIDLib::True)->c4Id());

        strName = pitemCur->strName();
        strName.Append(L"_AlarmBits");
        pitemCur->c4FldIdAlarmBits(pflddFind(strName, kCIDLib::True)->c4Id());

        strName = pitemCur->strName();
        strName.Append(L"_AlarmList");
        pitemCur->c4FldIdAlarmList(pflddFind(strName, kCIDLib::True)->c4Id());

        strName = pitemCur->strName();
        strName.Append(L"_Status");
        pitemCur->c4FldIdArmStatus(pflddFind(strName, kCIDLib::True)->c4Id());
    }

    strName = L"Encl";
    for (c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxEnclosures; c4Index++)
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

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxThermos; c4Index++)
    {
        // See if we have a thermo at this slot
        THAIOmniThermo* pitemCur = m_dcfgCurrent.pitemThermoAt(c4Index);
        if (!pitemCur)
            continue;

        // Do the hold status temp
        strName = pitemCur->strName();
        strName.Append(L"_Hold");
        c4FldId1 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the current temp
        strName = pitemCur->strName();
        strName.Append(L"_CurTemp");
        c4FldId2 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the cool set point
        strName = pitemCur->strName();
        strName.Append(L"_CoolSP");
        c4FldId3 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the heat set point
        strName = pitemCur->strName();
        strName.Append(L"_HeatSP");
        c4FldId4 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the fan mode
        strName = pitemCur->strName();
        strName.Append(L"_FanMode");
        c4FldId5 = pflddFind(strName, kCIDLib::True)->c4Id();

        // Do the thermo mode
        strName = pitemCur->strName();
        strName.Append(L"_Mode");
        c4FldId6 = pflddFind(strName, kCIDLib::True)->c4Id();

        if (m_c4FirstThermoFldId == kCIDLib::c4MaxCard)
            m_c4FirstThermoFldId = c4FldId1;
        m_c4LastThermoFldId = c4FldId6;

        pitemCur->SetFieldIds
        (
            c4FldId3, c4FldId2, c4FldId5, c4FldId4, c4FldId1, c4FldId6
        );
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxUnits; c4Index++)
    {
        // See if we have a unit at this slot
        THAIOmniUnit* pitemCur = m_dcfgCurrent.pitemUnitAt(c4Index);
        if (!pitemCur)
            continue;

        strName = pitemCur->strName();
        if ((pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Dimmer)
        ||  (pitemCur->eUnitType() == tHAIOmniTCPSh::EUnit_Flag))
        {
            strName.Append(L"_Value");
        }
         else
         {
            strName.Append(L"_State");
        }
        pitemCur->c4FldIdValue(pflddFind(strName, kCIDLib::True)->c4Id());

        if (m_c4FirstUnitFldId == kCIDLib::c4MaxCard)
            m_c4FirstUnitFldId = pitemCur->c4FldIdValue();
        m_c4LastUnitFldId = pitemCur->c4FldIdValue();
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kHAIOmniTCPSh::c4MaxZones; c4Index++)
    {
        // See if we have a zone at this slot
        THAIOmniZone* pitemCur = m_dcfgCurrent.pitemZoneAt(c4Index);
        if (!pitemCur)
            continue;

        const tHAIOmniTCPSh::EZoneTypes eZType = pitemCur->eZoneType();

        // The zone status is used in all cases
        strName = pitemCur->strName();
        strName.Append(L"_Status");
        c4FldId1 = pflddFind(strName, kCIDLib::True)->c4Id();

        c4FldId2 = kCIDLib::c4MaxCard;
        c4FldId3 = kCIDLib::c4MaxCard;
        c4FldId4 = kCIDLib::c4MaxCard;
        c4FldId5 = kCIDLib::c4MaxCard;
        c4FldId6 = kCIDLib::c4MaxCard;
        c4FldId7 = kCIDLib::c4MaxCard;

        tCIDLib::TCard4 c4Last = c4FldId1;

        // If an alarm zone, do the alarm orinete dfileds, else do the others
        if (eZType == tHAIOmniTCPSh::EZone_Alarm)
        {

            strName = pitemCur->strName();
            strName.Append(L"_Analog");
            c4FldId2 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName = pitemCur->strName();
            strName.Append(L"_Arming");
            c4FldId3 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName = pitemCur->strName();
            strName.Append(L"_Latched");
            c4FldId4 = pflddFind(strName, kCIDLib::True)->c4Id();

            c4Last = c4FldId4;
        }
         else if ((pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Temp)
              ||  (pitemCur->eZoneType() == tHAIOmniTCPSh::EZone_Humidity))
        {
            strName = pitemCur->strName();
            strName.Append(L"_CurVal");
            c4FldId5 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName = pitemCur->strName();
            strName.Append(L"_LowSP");
            c4FldId6 = pflddFind(strName, kCIDLib::True)->c4Id();

            strName = pitemCur->strName();
            strName.Append(L"_HighSP");
            c4FldId7 = pflddFind(strName, kCIDLib::True)->c4Id();

            c4Last = c4FldId7;
        }

        // If we've not stored the first zone field id yet, store it
        if (m_c4FirstZoneFldId == kCIDLib::c4MaxCard)
            m_c4FirstZoneFldId = c4FldId1;

        // Store the last zone field id so far
        m_c4LastZoneFldId = c4Last;

        pitemCur->SetFieldIds
        (
            c4FldId2, c4FldId3, c4FldId5, c4FldId7, c4FldId4, c4FldId6, c4FldId1
        );
    }
}


