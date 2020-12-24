//
// FILE NAME: ZWaveUSB3S_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/06/2017
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
//  This file implements all of the driver backdoor calls that we use to communicate
//  with the client side driver. There's quite a bit of back and forth with this driver
//  due to the complex nature of it, so we have a data exchange class that we use to
//  send data back and forth. This greatly simplifies things. We just use a single
//  backdoor method since the exchange object lets us deal with various parameters
//  and such.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3S_.hpp"



// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the user requests info on a given unit. This is something
//  that is going to take a fair amount of code. We use a fairly high msg priority
//  for these so that they get done quickly.
//
//  WE DO NOT use any of the msg I/O methods that take the unit info object, because
//  it may not be at all set up (probably not, hence why this operation was requested
//  in order to get the info required to add support.)
//
//  We can depend on the listening/freq listening info, since that is gotten for
//  all units and kept around, and of course the unit id.
//
tCIDLib::TBoolean
TZWaveUSB3SDriver::bQueryUnitInfo(  const   TZWUnitInfo&                unitiSrc
                                    ,       TString&                    strErr
                                    ,       tCIDLib::TCard8&            c8ManIds
                                    ,       tZWaveUSB3Sh::TCInfoList&   colCCs
                                    ,       tZWaveUSB3Sh::TEPInfoList&  colEPs
                                    ,       tCIDLib::TCard4&            c4FanModes
                                    ,       tCIDLib::TCard4&            c4ThermoModes
                                    ,       tCIDLib::TCard4&            c4SetPoints)
{
    c4FanModes = 0;
    c4SetPoints = 0;
    c4ThermoModes = 0;
    colCCs.RemoveAll();
    colEPs.RemoveAll();
    strErr.Clear();

    try
    {
        tCIDLib::TBoolean           bRes;
        tCIDLib::TBoolean           bGotFanMode = kCIDLib::False;
        tCIDLib::TBoolean           bGotManSpec = kCIDLib::False;
        tCIDLib::TBoolean           bGotMultiCh = kCIDLib::False;
        tCIDLib::TBoolean           bGotSetPoints = kCIDLib::False;
        tCIDLib::TBoolean           bGotThermoMode = kCIDLib::False;
        tCIDLib::TBoolean           bGotVersion = kCIDLib::False;
        tCIDLib::TBoolean           bFanModeSecure = kCIDLib::False;
        tCIDLib::TBoolean           bManSpecSecure = kCIDLib::False;
        tCIDLib::TBoolean           bMultiChSecure = kCIDLib::False;
        tCIDLib::TBoolean           bSetPointsSecure = kCIDLib::False;
        tCIDLib::TBoolean           bThermoModeSecure = kCIDLib::False;
        tCIDLib::TBoolean           bVersionSecure = kCIDLib::False;
        tCIDLib::TBoolean           bGotSecure = kCIDLib::False;
        tZWaveUSB3Sh::TXInfoList    colEmptyXI;
        tZWaveUSB3Sh::ECClasses     eClass;
        TZWInMsg                    zwimIn;
        TZWOutMsg                   zwomOut;


        //
        //  So the first thing we have to do is to query the node info frame to get
        //  the non-secure classes.
        //
        zwomOut.Reset
        (
            unitiSrc.c1Id()
            , FUNC_ID_ZW_REQUEST_NODE_INFO
            , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
        );
        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
        USB3Send(zwomOut);

        //
        //  We have to wait specially for this one since it's not a CC msg and comes
        //  back as something other than what we send.
        //
        bRes = bUSB3WaitForMsg
        (
            tZWaveUSB3Sh::EMsgTypes::Request
            , unitiSrc.c1Id()
            , FUNC_ID_ZW_APPLICATION_UPDATE
            , 0
            , 0
            , kCIDLib::enctThreeSeconds
            , zwimIn
            , kCIDLib::False
        );

        if (!bRes)
        {
            strErr = L"The non-secure classes could not be gotten";
            return kCIDLib::False;
        }

        //  We got it so let's pull out the classes and add them.
        colCCs.RemoveAll();
        {
            tCIDLib::TCard4 c4Len = zwimIn.m_c1PLLength;
            for (tCIDLib::TCard4 c4Index = 6; c4Index < c4Len; c4Index++)
            {
                const tCIDLib::TCard1 c1Cur = zwimIn[c4Index];
                eClass = tZWaveUSB3Sh::eAltNumECClasses(c1Cur);
                if (eClass != tZWaveUSB3Sh::ECClasses::Count)
                    colCCs.objAdd(TZWClassInfo(eClass));

                switch(c1Cur)
                {
                    case COMMAND_CLASS_MANUFACTURER_SPECIFIC :
                        bGotManSpec = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_MULTI_CHANNEL_V4 :
                        bGotMultiCh = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_SECURITY :
                        bGotSecure = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_THERMOSTAT_FAN_MODE :
                        bGotFanMode = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_THERMOSTAT_MODE :
                        bGotThermoMode = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_THERMOSTAT_SETPOINT :
                        bGotSetPoints = kCIDLib::True;
                        break;

                    case COMMAND_CLASS_VERSION :
                        bGotVersion = kCIDLib::True;
                        break;

                    default :
                        break;
                };
            }
        }

        //
        //  If we need to, let's get secure classes. This one is more complex and
        //  may return multiple responses. Here again, we have to explicitly wait
        //  since it's not a normal, single CC msg reply.
        //
        if (bGotSecure)
        {
            zwomOut.Reset
            (
                unitiSrc.c1Id()
                , COMMAND_CLASS_SECURITY
                , SECURITY_COMMANDS_SUPPORTED_GET
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
            );
            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();

            // Send this one securely always
            zwomOut.Finalize(unitiSrc.bFreqListener(), kCIDLib::True);
            USB3Send(zwomOut);

            while (kCIDLib::True)
            {
                bRes = bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , unitiSrc.c1Id()
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_SECURITY
                    , SECURITY_COMMANDS_SUPPORTED_REPORT
                    , kCIDLib::enctFourSeconds
                    , zwimIn
                    , kCIDLib::False
                );

                if (!bRes)
                {
                    strErr = L"The secure classes could not be gotten";
                    return kCIDLib::False;
                }

                // Pull out the classes and update our list
                tCIDLib::TCard4 c4ByteInd = 3;
                const tCIDLib::TCard4 c4ClassBytes = zwimIn.c4CCBytes();
                while (c4ByteInd < c4ClassBytes)
                {
                    const tCIDLib::TCard1 c1Cur = zwimIn.c1CCByteAt(c4ByteInd++);

                    if (c1Cur == COMMAND_CLASS_SECURITY)
                    {
                        //
                        //  This is a special case, so don't treat it as secure even if
                        //  it is in this list.
                        //
                    }
                     else if (c1Cur == COMMAND_CLASS_MARK)
                    {
                        //
                        //  Don't care about controlled classes so break out on the
                        //  divider mark.
                        //
                        break;
                    }
                     else if (c1Cur >= 0xF1)
                    {
                        // It's an extended class tht we don't understand, so skip it
                        c4ByteInd++;
                    }
                     else
                    {
                        //
                        //  If it's one we understand, then see if we already have it. If
                        //  so update it to indicate secure, else add it. Any new ones we
                        //  set the version to zero to indicate we've not gotten a version
                        //  for it yet.
                        //
                        eClass = tZWaveUSB3Sh::eAltNumECClasses(c1Cur);
                        if (eClass != tZWaveUSB3Sh::ECClasses::Count)
                        {
                            // See if we already have this one in our list
                            const tCIDLib::TCard4 c4CurCount = colCCs.c4ElemCount();
                            tCIDLib::TCard4 c4CurInd = 0;
                            while (c4CurInd < c4CurCount)
                            {
                                if (colCCs[c4CurInd].m_eClass == eClass)
                                    break;
                                c4CurInd++;
                            }

                            if (c4CurInd == c4CurCount)
                            {
                                // Don't have it yet so add it
                                colCCs.objAdd
                                (
                                    TZWClassInfo(eClass, 0, kCIDLib::True, colEmptyXI)
                                );
                            }
                             else
                            {
                                // We already have it, but it's secure now
                                colCCs[c4CurInd].m_bSecure = kCIDLib::True;
                            }
                        }

                        switch(c1Cur)
                        {
                            case COMMAND_CLASS_MANUFACTURER_SPECIFIC :
                                bGotManSpec = kCIDLib::True;
                                bManSpecSecure = kCIDLib::True;
                                break;

                            case COMMAND_CLASS_MULTI_CHANNEL_V4 :
                                bGotMultiCh = kCIDLib::True;
                                bMultiChSecure = kCIDLib::True;
                                break;

                            case COMMAND_CLASS_THERMOSTAT_FAN_MODE :
                                bGotFanMode = kCIDLib::True;
                                bFanModeSecure = kCIDLib::True;
                                break;

                            case COMMAND_CLASS_THERMOSTAT_MODE :
                                bGotThermoMode = kCIDLib::True;
                                bThermoModeSecure = kCIDLib::True;
                                break;

                            case COMMAND_CLASS_THERMOSTAT_SETPOINT :
                                bGotSetPoints = kCIDLib::True;
                                bSetPointsSecure = kCIDLib::True;
                                break;

                            case COMMAND_CLASS_VERSION :
                                bGotVersion = kCIDLib::True;
                                bVersionSecure = kCIDLib::True;
                                break;

                            default :
                                break;
                        };
                    }
                }

                // If no more reports expected, we are done
                if (!zwimIn.c1CCByteAt(2))
                    break;
            }
        }

        //
        //  At this point, if we don't have manufacturer specific ids, then this
        //  is a waste of time.
        //
        if (!bGotManSpec)
        {
            strErr = L"The unit does not support the manufacturer specific command class";
            return kCIDLib::False;
        }

        //
        //  Query the man spec info, since if this fails no point in getting more.
        //  We could do a send and wait call here, but we want to return an error
        //  specific to this step, so we explicitly wait again.
        //
        zwomOut.Reset
        (
            unitiSrc.c1Id()
            , COMMAND_CLASS_MANUFACTURER_SPECIFIC
            , MANUFACTURER_SPECIFIC_GET
            , 0
            , 2
            , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
        );
        zwomOut.AppendTransOpts();
        zwomOut.AppendCallback();
        zwomOut.Finalize(unitiSrc.bFreqListener(), bManSpecSecure);
        USB3Send(zwomOut);
        bRes = bUSB3WaitForMsg
        (
            tZWaveUSB3Sh::EMsgTypes::Request
            , unitiSrc.c1Id()
            , FUNC_ID_APPLICATION_COMMAND_HANDLER
            , COMMAND_CLASS_MANUFACTURER_SPECIFIC
            , MANUFACTURER_SPECIFIC_REPORT
            , kCIDLib::enctFourSeconds
            , zwimIn
            , kCIDLib::False
        );

        if (!bRes)
        {
            strErr = L"Could not get the manufacturer specific ids for the unit";
            return kCIDLib::False;
        }

        // Build up the id in the encoded format
        c8ManIds = zwimIn.c1CCByteAt(2);
        c8ManIds <<= 8;
        c8ManIds |= zwimIn.c1CCByteAt(3);
        c8ManIds <<= 8;
        c8ManIds |= zwimIn.c1CCByteAt(4);
        c8ManIds <<= 8;
        c8ManIds |= zwimIn.c1CCByteAt(5);
        c8ManIds <<= 8;
        c8ManIds |= zwimIn.c1CCByteAt(6);
        c8ManIds <<= 8;
        c8ManIds |= zwimIn.c1CCByteAt(7);

        //
        //  If we have version support, let's try to get the versions of all of
        //  the classes we got.
        //
        if (bGotVersion)
        {
            const tCIDLib::TCard4 c4Count = colCCs.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                zwomOut.Reset
                (
                    unitiSrc.c1Id()
                    , COMMAND_CLASS_VERSION
                    , VERSION_COMMAND_CLASS_GET
                    , 0
                    , 3
                    , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
                );

                // We have to translate to the actual ZW class id
                zwomOut += tCIDLib::TCard1
                (
                    tZWaveUSB3Sh::i4AltNumECClasses(colCCs[c4Index].m_eClass)
                );
                zwomOut.AppendTransOpts();
                zwomOut.AppendCallback();
                zwomOut.Finalize(unitiSrc.bFreqListener(), bVersionSecure);
                USB3Send(zwomOut);

                bRes = bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , unitiSrc.c1Id()
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_VERSION
                    , VERSION_COMMAND_CLASS_REPORT
                    , kCIDLib::enctFourSeconds
                    , zwimIn
                    , kCIDLib::False
                );

                if (!bRes)
                {
                    strErr = L"Could not get the version for CC ";
                    strErr.Append(tZWaveUSB3Sh::strXlatECClasses(colCCs[c4Index].m_eClass));
                    return kCIDLib::False;
                }

                colCCs[c4Index].m_c4Version = zwimIn.c1CCByteAt(3);
            }
        }

        // If it supports multi-channel, get info on the
        if (bGotMultiCh)
        {
            // Get the number of end points
            zwomOut.Reset
            (
                unitiSrc.c1Id()
                , COMMAND_CLASS_MULTI_CHANNEL_V4
                , MULTI_CHANNEL_END_POINT_GET_V4
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
            );

            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiSrc.bFreqListener(), bMultiChSecure);
            USB3Send(zwomOut);

            bRes = bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , unitiSrc.c1Id()
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_MULTI_CHANNEL_V4
                , MULTI_CHANNEL_END_POINT_REPORT_V4
                , kCIDLib::enctFourSeconds
                , zwimIn
                , kCIDLib::False
            );

            if (!bRes)
            {
                strErr = L"Could not get the end point count";
                return kCIDLib::False;
            }

            // For each end point, query the info for it
            const tCIDLib::TCard4 c4EPCount = zwimIn.c1CCByteAt(3) & 0x7F;
            TZWUSB3EndPntInfo zwepiCur;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4EPCount; c4Index++)
            {
                zwomOut.Reset
                (
                    unitiSrc.c1Id()
                    , COMMAND_CLASS_MULTI_CHANNEL_V4
                    , MULTI_CHANNEL_CAPABILITY_GET_V4
                    , 0
                    , 3
                    , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
                );

                // 1 based
                zwomOut += tCIDLib::TCard1(c4Index + 1);
                zwomOut.AppendTransOpts();
                zwomOut.AppendCallback();
                zwomOut.Finalize(unitiSrc.bFreqListener(), bMultiChSecure);
                USB3Send(zwomOut);

                bRes = bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , unitiSrc.c1Id()
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_MULTI_CHANNEL_V4
                    , MULTI_CHANNEL_CAPABILITY_REPORT_V4
                    , kCIDLib::enctFourSeconds
                    , zwimIn
                    , kCIDLib::False
                );

                if (bRes)
                {
                    zwepiCur.Reset();

                    const tCIDLib::TCard4 c4CCBytes = zwimIn.c4CCBytes();
                    zwepiCur.m_c4EndPntId = c4Index + 1;
                    zwepiCur.m_c1Generic = zwimIn.c1CCByteAt(3);
                    zwepiCur.m_c1Specific = zwimIn.c1CCByteAt(4);
                    for (tCIDLib::TCard4 c4Index = 5; c4Index < c4CCBytes; c4Index++)
                    {
                        const tZWaveUSB3Sh::ECClasses eClass
                        (
                            tZWaveUSB3Sh::eAltNumECClasses(zwimIn.c1CCByteAt(c4Index))
                        );

                        if (eClass != tZWaveUSB3Sh::ECClasses::None)
                            zwepiCur.m_fcolClasses.c4AddElement(eClass);
                    }

                    zwepiCur.m_bDynamic = (zwimIn.c1CCByteAt(2) & 0x80) != 0;
                    colEPs.objAdd(zwepiCur);
                }
            }
        }

        // If it supports thermostat/fan modes, get the supported modes
        if (bGotFanMode)
        {
            zwomOut.Reset
            (
                unitiSrc.c1Id()
                , COMMAND_CLASS_THERMOSTAT_FAN_MODE
                , THERMOSTAT_FAN_MODE_SUPPORTED_GET
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
            );

            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiSrc.bFreqListener(), bFanModeSecure);
            USB3Send(zwomOut);

            bRes = bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , unitiSrc.c1Id()
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_THERMOSTAT_FAN_MODE
                , THERMOSTAT_FAN_MODE_SUPPORTED_REPORT
                , kCIDLib::enctFourSeconds
                , zwimIn
                , kCIDLib::False
            );

            if (!bRes)
            {
                strErr = L"Could not get the fan modes";
                return kCIDLib::False;
            }

            if (zwimIn.c4CCBytes() > 3)
            {
                c4FanModes = zwimIn.c1CCByteAt(3);
                c4FanModes <<= 8;
            }
            c4FanModes |= zwimIn.c1CCByteAt(2);
        }

        if (bGotThermoMode)
        {
            zwomOut.Reset
            (
                unitiSrc.c1Id()
                , COMMAND_CLASS_THERMOSTAT_MODE
                , THERMOSTAT_MODE_SUPPORTED_GET
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
            );

            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiSrc.bFreqListener(), bThermoModeSecure);
            USB3Send(zwomOut);

            bRes = bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , unitiSrc.c1Id()
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_THERMOSTAT_MODE
                , THERMOSTAT_MODE_SUPPORTED_REPORT
                , kCIDLib::enctFourSeconds
                , zwimIn
                , kCIDLib::False
            );

            if (!bRes)
            {
                strErr = L"Could not get the thermostat modes";
                return kCIDLib::False;
            }

            if (zwimIn.c4CCBytes() > 3)
            {
                c4ThermoModes = zwimIn.c1CCByteAt(3);
                c4ThermoModes <<= 8;
            }
            c4ThermoModes |= zwimIn.c1CCByteAt(2);
        }

        if (bGotSetPoints)
        {
            zwomOut.Reset
            (
                unitiSrc.c1Id()
                , COMMAND_CLASS_THERMOSTAT_SETPOINT
                , THERMOSTAT_SETPOINT_SUPPORTED_GET
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::SpecialCmd
            );

            zwomOut.AppendTransOpts();
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiSrc.bFreqListener(), bSetPointsSecure);
            USB3Send(zwomOut);

            bRes = bUSB3WaitForMsg
            (
                tZWaveUSB3Sh::EMsgTypes::Request
                , unitiSrc.c1Id()
                , FUNC_ID_APPLICATION_COMMAND_HANDLER
                , COMMAND_CLASS_THERMOSTAT_SETPOINT
                , THERMOSTAT_SETPOINT_SUPPORTED_REPORT
                , kCIDLib::enctFourSeconds
                , zwimIn
                , kCIDLib::False
            );

            if (!bRes)
            {
                strErr = L"Could not get the thermostat setpoints";
                return kCIDLib::False;
            }
            if (zwimIn.c4CCBytes() > 3)
            {
                c4SetPoints = zwimIn.c1CCByteAt(3);
                c4SetPoints <<= 8;
            }
            c4SetPoints |= zwimIn.c1CCByteAt(2);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErr = L"An exception occurred while querying unit info. See the logs";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



//
//  Standard driver backdoor method that lets the client send us a command with data
//  and for us to optionally send back some kind of response. We use this one for the
//  more complex types of commands that are best done by flattening the data objects
//  and sending them as a memory buffer.
//
//  Some of the commands have an 'awake' value. This is used to indicate that the
//  client knows (the user told him) that the device is awake. If indicated, we will
//  send msgs synchronously, even if the unit is not a live listener. If the unit
//  is a live listener the value of awake doesn't matter.
//
//  The data streamed in and out for each command is arbitrary, but generally quite
//  simple. So the shared facility defines a TZWSendCmdData class that both sides can
//  see. It some standard, named members and some ad hoc ones that can be used for any
//  given command. Worst case, if an object or list of objects needs to be sent
//  it can be flatted into a memory buffer member of the data class.
//
//  NOTE that we use unit ids, not names. This greatly simplifies things because the
//  client may be making edits, which includes changing unit names, but will then
//  do some operation before saving those changes. If we used names that wouldn't work
//  since the server doesn't know those new names yet.
//
//  The commands (and parameters where the non-specific ones are needed.)
//
//      ApproveNewUnits
//          If there are any units in WaitApprove state, it will force a re-register
//          of fields, which will pick up these new guys.
//
//      AssocToDrv unitid groupnum awake
//          c1Val1 = groupnum
//          Adds our controller's id to the indicated group on the indicated unit.
//          Since we are the target, no multi-channel stuff required.
//
//      DisableUnit unitid
//          Disables a unit by setting its state to disabled. They have to do a rescan
//          to get it back.
//
//      DoAutoCfg unitid awake
//          We find the named unit and tell it to send out any configuration msgs
//          it is set up for. If a non-listener, they are just queued up for later
//          transmission. It will return a message whether error or success since it
//          may need to inform the user that the msgs were queued pending a wakeup.
//
//      ForceRescan unitid awake
//          Forces a unit back to its starting point so that it can go back through
//          its process of gathering information. If it's a non-listener it has to
//          be forced to restart, usually means batteries out then back in.
//
//          Have to be careful about this one since it may already have fields, which
//          implies it is in at least GetInitVals state. So we can leave it in an
//          inconsistent state. So, if it is GetInitVals or beyond, we force a re-
//          registration of fields.
//
//      QueryAssoc unitid groupnum awake
//          c1Val1 = grpnum
//          Query the units associated with the target named unit, in the indicated
//          group. The returned info is put into the memory buffer as bytes. THey
//          are a set of alternating bytes, the first is the id and the second is the
//          end point for the first association. Then the next two are the id and
//          end point for the second associations, etc... If not an end point based
//          association the end point byte is 0xFF.
//
//      QueryConfig
//          On output the current config will be streamed into the memory buffer.
//
//      QueryCfgParam unitid pnum awake
//          c1Val1 = pnum
//          For return:
//          c1Val1 = pnum
//          c1Val2 = bytes
//          i4Val1 = value;
//
//      QueryGrpCnt unitname awake
//          For return:
//          c4Val1 = number of groups
//
//      QueryUnitReport
//          Queries a report from the driver for a specific unit. This gathers up the
//          info about our stuff, not the actual module. It just returns the UTF-8
//          encoded text, compressed with ZLib.
//
//      QueryRepStatus
//          On return m_c4Val1 is set to the driver state enum value for the replication
//          state. The caller needs to cast it back to the enum value.
//
//      QueryStatusInfo
//          This is used to get the overall info for the driver, so that the client can
//          do a quick test of whether he needs to get our config again or update some
//          display info. On return we format into the memory buffer:
//              1. Config serial number
//              2. a boolean that indicates our in/out of Z-Wave network status
//              3. Our Z-Wave specific driver status enum
//              4. Our current trace level
//
//      QueryTraceLevel
//          On return m_c4Val1 is 0 for off, 1 for low, 2 for medium, and 3 for high.
//
//      QueryUnitInfo unitid
//          The unit info object will be streamed into the memory buffer member.
//
//      QueryWakeup unitid awake
//          On return:
//          c1Val1 = target wakeup id
//          c4Val1 = Wakeup interval seconds
//
//      ReloadDevInfoMap
//          Forces the driver to reload the device info map, so that any subsequent
//          lookups of device info will pick up new data from disk. This is to allow
//          us to update the device info files and get existing drivers to pick up the
//          new stuff without having to restart the driver. Used with ForceRescan
//          above, we can force a specific unit to then read in that new data.
//
//      RemoveAssoc unitid groupnum tarid tarendpoint awake
//          c1Val1 = groupnum
//          c1Val2 = tarid
//          tarid is (a target unit id) is removed from the indicated group in the named
//          unit. If the association being removed is not for a particular end point in
//          the target id, then c1Val1 will be 0xFF, else it will be the end point id.
//
//      Replicate
//          Puts us into replication mode. This just starts up the rep/scan thread
//          in replication mode and returns.
//
//      ResetUnitCfg
//          This is not generally used, but it will remove all of our unit information
//          from the driver config and then do the same as ScanNetwork below.
//
//      SetAssoc unitid groupnum tarid tarendpoint awake
//          c1Val1 = groupnum
//          c1Val2 = tarid
//          tarid = The id to put into the group. If the target unit needs to report to
//          an end point then the end point id should be something other than 0xFF.
//
//      SetConfig
//          m_mbufData = Flattened configuration
//          m_c4BufSz = bytes in the buffer
//
//      SetCfgParam unitid pnum pval bytes awake
//          c1Val1 = pnum
//          i4Val1 = pval
//          c1Val2 = bytes (1, 2 or 4)
//          Sets a configuration parameter on the indicated unit. The number, the value,
//          and the number of bytes in the value are provided.
//
//      SetTrace option
//          c4Val1 = x
//          Sets the trace file state. c4Val1 is one of these values:
//              0 = Disable
//              1 = Enable at low verbosity
//              2 = Enable at medium verbosity
//              3 = Enable at high verbosity
//              5 = Flush
//              6 = Reset
//
tCIDLib::TBoolean
TZWaveUSB3SDriver::bSendData(const  TString&            strDataType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufData)
{
    //
    //  Get the incoming bytes out now and zero the parameter. This way, if by some
    //  chance we fail to get it updated, we won't stream back bogus data.
    //
    const tCIDLib::TCard4 c4InBytes = c4Bytes;
    c4Bytes = 0;

    //
    //  A little extra code but it significantly simplifies the below if we do this
    //  first. If the data type prefix indicates it's a poll operation, we let it go.
    //  Else we have to be in ready state.
    //
    if ((m_eZWDrvState != tZWaveUSB3Sh::EDrvStates::Ready)
    &&  strDataType.bStartsWithI(kZWaveUSB3Sh::strSendData_PollPref))
    {
        m_zwdxdReply.StatusReset
        (
            kCIDLib::False
            , L"The driver is not currently ready to accept commands"
        );

        TBinMBufOutStream strmRep(&mbufData);
        strmRep << m_zwdxdReply << kCIDLib::FlushIt;
        c4Bytes = strmRep.c4CurSize();
        return kCIDLib::True;
    }

    // Optimistically reset the reply object with positive status
    m_zwdxdReply.StatusReset(kCIDLib::True);

    try
    {
        // Create a stream over the buffer and stream in a data object
        {
            TBinMBufInStream strmSrc(&mbufData, c4InBytes);
            strmSrc >> m_zwdxdIn;
        }

        //
        //  If there is a unit id in the incoming data, try to find it. If not
        //  found no need to do the commands. It's a win to do this front.
        //
        TZWUnitInfo* punitiCmd = nullptr;
        if (m_zwdxdIn.m_c1UnitId)
        {
            punitiCmd = m_dcfgCurrent.punitiFindById(m_zwdxdIn.m_c1UnitId);
            if (!punitiCmd)
            {
                m_zwdxdReply.StatusReset
                (
                    kCIDLib::False
                    , L"No unit found with id %(1)"
                    , TCardinal(m_zwdxdIn.m_c1UnitId)
                );
                TBinMBufOutStream strmRep(&mbufData);
                strmRep << m_zwdxdReply << kCIDLib::FlushIt;
                c4Bytes = strmRep.c4CurSize();
                return kCIDLib::True;
            }
        }

        if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_ApproveNewUnits)
        {
            //
            //  See if any units are waiting for approval. If so, push them to get
            //  init values state, remembering the ids of any that we do.
            //
            tCIDLib::TByteList fcolApprove;
            tZWaveUSB3Sh::TUnitInfoList& colUnits = m_dcfgCurrent.colUnits();
            tCIDLib::TCard4 c4Count = colUnits.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TZWUnitInfo& unitiCur = colUnits[c4Index];
                if (unitiCur.eState() == tZWaveUSB3Sh::EUnitStates::WaitApprove)
                {
                    unitiCur.ApproveForUse();
                    fcolApprove.c4AddElement(unitiCur.c1Id());
                }
            }

            if (!fcolApprove.bIsEmpty())
            {
                //
                //  We need to do it. This means we have to push any units that are
                //  at ready state back to GetInitVals state, since the field reg
                //  method assumes that is what new units that are viable are going
                //  to be at. And the re-reg will trash any existing fields, so we
                //  need to re-query them.
                //
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    TZWUnitInfo& unitiCur = colUnits[c4Index];
                    if (unitiCur.eState() == tZWaveUSB3Sh::EUnitStates::Ready)
                        unitiCur.eState(tZWaveUSB3Sh::EUnitStates::GetInitVals);
                }

                // Save this as our new current config
                bSaveConfig(m_dcfgCurrent);

                // Register our fields now
                RegisterFields();

                // And go back and send out auto-config for the ones we approved
                TString strErr;
                c4Count = fcolApprove.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    TZWUnitInfo& unitiCur = m_dcfgCurrent.unitiFindById(fcolApprove[c4Index]);
                    if (!unitiCur.bDoAutoCfg(kCIDLib::False, strErr))
                    {
                        if (facZWaveUSB3Sh().bLowTrace())
                        {
                            facZWaveUSB3Sh().LogTraceErr
                            (
                                L"Auto-config failed on newly approved unit %(1)"
                                , unitiCur.strName()
                            );
                        }
                    }
                }
            }
             else
            {
                m_zwdxdReply.StatusReset(kCIDLib::False, L"No units were waiting for approval");
            }
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_AssocToDrv)
        {
            TString strErr;
            tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bSetAssoc
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_dcfgCurrent.c1CtrlId()
                , kCIDLib::True
                , m_zwdxdIn.m_bAwake
                , strErr
            );

            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_DisableUnit)
        {
            //
            //  If it is not already disabled, then mark it. And we have to re-register
            //  fields since we need to get rid of all of his. It's possible he has
            //  no fields, but the amount of code required to figure that out is way
            //  to much to want to bother with.
            //
            if (punitiCmd->eState() != tZWaveUSB3Sh::EUnitStates::Disabled)
            {
                punitiCmd->eState(tZWaveUSB3Sh::EUnitStates::Disabled);
                RegisterFields();
            }
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_DoAutoCfg)
        {
            // Just pass on to the unit with the awake flag we got
            TString strStatus;
            const tCIDLib::TBoolean bRes = punitiCmd->bDoAutoCfg
            (
                m_zwdxdIn.m_bAwake, strStatus
            );
            m_zwdxdReply.StatusReset(bRes, strStatus);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_ForceRescan)
        {
            //
            //  Just in case the dev info has been updated (one of the main reasons
            //  this would be done), we force it out of the cache if present.
            //
            if (punitiCmd->eState() >= tZWaveUSB3Sh::EUnitStates::GetInitVals)
                facZWaveUSB3Sh().ReloadDevInfo(punitiCmd->c8ManIds());

            // And now force it to rescan, passing the awake flag
            if (punitiCmd->bForceRescan(m_zwdxdIn.m_bAwake))
                RegisterFields();
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryAssoc)
        {
            tCIDLib::TCardList fcolAssoc;
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bQueryAssociations
            (
                this, *punitiCmd, m_zwdxdIn.m_c1Val1, fcolAssoc, strErr
            );

            if (bRes)
            {
                m_zwdxdReply.m_c4BufSz = 0;
                const tCIDLib::TCard4 c4Count = fcolAssoc.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    const tCIDLib::TCard4 c4Cur = fcolAssoc[c4Index];
                    m_zwdxdReply.m_mbufData.PutCard1
                    (
                        tCIDLib::TCard1(c4Cur >> 16), m_zwdxdReply.m_c4BufSz++
                    );
                    m_zwdxdReply.m_mbufData.PutCard1
                    (
                        tCIDLib::TCard1(c4Cur), m_zwdxdReply.m_c4BufSz++
                    );
                }
            }
             else
            {
                m_zwdxdReply.StatusReset(bRes, strErr);
            }
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryConfig)
        {
            TBinMBufOutStream strmTar(&m_zwdxdReply.m_mbufData);
            strmTar << m_dcfgCurrent << kCIDLib::FlushIt;
            m_zwdxdReply.m_c4BufSz = strmTar.c4CurSize();
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryCfgParam)
        {
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCConfig:: bQueryParam
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_zwdxdReply.m_i4Val1
                , kCIDLib::False
                , m_zwdxdIn.m_bAwake
                , strErr
            );
            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryGrpCnt)
        {
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bQueryAssocGrps
            (
                *punitiCmd, m_zwdxdIn.m_c4Val1, kCIDLib::False, m_zwdxdIn.m_bAwake, strErr
            );

            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryTraceLevel)
        {
            switch(facZWaveUSB3Sh().eTraceLevel())
            {
                case tCQCKit::EVerboseLvls::Off :
                    m_zwdxdReply.m_c4Val1 = 0;
                    break;

                case tCQCKit::EVerboseLvls::Low :
                    m_zwdxdReply.m_c4Val1 = 1;
                    break;

                case tCQCKit::EVerboseLvls::Medium :
                    m_zwdxdReply.m_c4Val1 = 2;
                    break;

                case tCQCKit::EVerboseLvls::High :
                    m_zwdxdReply.m_c4Val1 = 3;
                    break;

                default :
                    m_zwdxdReply.StatusReset(kCIDLib::False, L"Known trace verbosity value");
                    break;
            };
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryUnitReport)
        {
            m_zwdxdReply.m_c4BufSz = c4QueryUnitReport(*punitiCmd, m_zwdxdReply.m_mbufData);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryRepStatus)
        {
            m_zwdxdReply.m_c4Val1 = tCIDLib::TCard4(m_eRepState);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryStatusInfo)
        {
            const tCIDLib::TCard4 c4SerNum = m_dcfgCurrent.c4CalcSNHash();

            TBinMBufOutStream strmTar(&m_zwdxdReply.m_mbufData);
            strmTar << c4SerNum
                    << m_dcfgCurrent.bInNetwork()
                    << m_eZWDrvState
                    << facZWaveUSB3Sh().eTraceLevel()
                    << kCIDLib::FlushIt;
            m_zwdxdReply.m_c4BufSz = strmTar.c4CurSize();
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryUnitInfo)
        {
            tCIDLib::TCard8             c8ManIds;
            tZWaveUSB3Sh::TCInfoList    colCCs;
            TString                     strErr;
            tZWaveUSB3Sh::TEPInfoList   colEPs;
            tCIDLib::TCard4             c4FanModes;
            tCIDLib::TCard4             c4ThermoModes;
            tCIDLib::TCard4             c4SetPoints;
            const tCIDLib::TBoolean bRes = bQueryUnitInfo
            (
                *punitiCmd
                , strErr
                , c8ManIds
                , colCCs
                , colEPs
                , c4FanModes
                , c4ThermoModes
                , c4SetPoints
            );

            if (bRes)
            {
                TBinMBufOutStream strmTar(&m_zwdxdReply.m_mbufData);
                strmTar << c8ManIds
                        << colCCs
                        << colEPs
                        << c4FanModes
                        << c4ThermoModes
                        << c4SetPoints
                        << kCIDLib::FlushIt;
                m_zwdxdReply.m_c4BufSz = strmTar.c4CurSize();
            }
             else
            {
                m_zwdxdReply.StatusReset(kCIDLib::False, strErr);
            }
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_QueryWakeup)
        {
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCWakeup::bQueryWakeup
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_zwdxdIn.m_c4Val1
                , kCIDLib::False
                , m_zwdxdIn.m_bAwake
                , strErr
            );
            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_RemoveAssoc)
        {
            TString strErr;
            tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bRemoveAssoc
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_zwdxdIn.m_c1Val2
                , kCIDLib::True
                , m_zwdxdIn.m_bAwake
                , strErr
                , m_zwdxdIn.m_c1EPId
            );

            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if ((m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_Replicate)
              ||  (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_ResetUnitCfg))
        {
            //
            //  Make a copy of the current config to use as the working config during
            //  the replication. If resetting the unit list, then do that to the
            //  copy first, then start the replication thread.
            //
            try
            {
                m_dcfgReplicate = m_dcfgCurrent;

                //
                //  If it's a unit reset, delete the unit info list first, but maintain
                //  any current node level info.
                //
                if (m_zwdxdIn.m_strCmd.bCompareI(kZWaveUSB3Sh::strSendData_ResetUnitCfg))
                    m_dcfgReplicate.ResetUnitList();

                m_thrRepScan.Start();
            }

            catch(...)
            {
                // Make sure we get back ready state
                SetDrvState(tZWaveUSB3Sh::EDrvStates::Ready);
                throw;
            }
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_SetAssoc)
        {
            TString strErr;
            tCIDLib::TBoolean bRes = TZWUSB3CCAssoc::bSetAssoc
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_zwdxdIn.m_c1Val2
                , kCIDLib::True
                , m_zwdxdIn.m_bAwake
                , strErr
                , m_zwdxdIn.m_c1EPId
            );
            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_SetConfig)
        {
            // Stream in the new config to the replication config version
            TBinMBufInStream strmSrc(&m_zwdxdIn.m_mbufData, m_zwdxdIn.m_c4BufSz);
            strmSrc >> m_dcfgReplicate;

            //
            //  Now tell the driver we just completed a replication. He will pick
            //  up this new config and update.
            //
            SetDrvState(tZWaveUSB3Sh::EDrvStates::RepSucceeded);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_SetCfgParam)
        {
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCConfig::bSetParam
            (
                *punitiCmd
                , m_zwdxdIn.m_c1Val1
                , m_zwdxdIn.m_c1Val2
                , m_zwdxdIn.m_i4Val1
                , kCIDLib::True
                , m_zwdxdIn.m_bAwake
                , strErr
            );

            if (!bRes)
                m_zwdxdReply.StatusReset(bRes, strErr);
        }
         else if (m_zwdxdIn.m_strCmd == kZWaveUSB3Sh::strSendData_SetTrace)
        {
            switch(m_zwdxdIn.m_c4Val1)
            {
                case 0 :
                case 1 :
                case 2 :
                case 3 :
                    if (m_zwdxdIn.m_c4Val1 == 0)
                        facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::Off);
                    else if (m_zwdxdIn.m_c4Val1 == 1)
                        facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::Low);
                    else if (m_zwdxdIn.m_c4Val1 == 2)
                        facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::Medium);
                    else if (m_zwdxdIn.m_c4Val1 == 3)
                        facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::High);
                    else
                    {
                        m_zwdxdReply.StatusReset(kCIDLib::False, L"Uknown trace verbosity");
                    }
                    break;

                case 5 :
                    facZWaveUSB3Sh().FlushMsgTrace();
                    break;

                case 6 :
                    facZWaveUSB3Sh().ResetMsgTrace();
                    break;

                default :
                    m_zwdxdReply.StatusReset
                    (
                        kCIDLib::False
                        , L"'%(1)' is not a valid SetTrace value"
                        , TCardinal(m_zwdxdIn.m_c1Val1)
                    );
                    break;
            };
        }
         else
        {
            m_zwdxdReply.StatusReset
            (
                kCIDLib::False
                , L"'%(1)' is not a valid bSendData() command name"
                , m_zwdxdIn.m_strCmd
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        m_zwdxdReply.StatusReset(kCIDLib::False, L"An exception occurred, see the logs");
    }

    catch(...)
    {
        m_zwdxdReply.StatusReset
        (
            kCIDLib::False
            , L"A system exception occurred while processing command %(1)"
            , m_zwdxdIn.m_strCmd
        );
    }

    // Stream the reply object back out as the return data
    {
        TBinMBufOutStream strmRep(&mbufData);
        strmRep << m_zwdxdReply << kCIDLib::FlushIt;
        c4Bytes = strmRep.c4CurSize();
    }


    // Indicates that there is return data in our case
    return kCIDLib::True;;
}


// Just a little code to kick off a report and to compress it for return
tCIDLib::TCard4
TZWaveUSB3SDriver::c4QueryUnitReport(const TZWUnitInfo& unitiSrc, TMemBuf& mbufToFill)
{
    //
    //  Set up an output text stream with a UTF-8 converter since that's the format
    //  we promise to return. We stream to a temp since we have to compress, so we
    //  can do that into the caller's buffer.
    //
    tCIDLib::TCard4 c4TextBytes = 0;
    THeapBuf mbufText(0x10000, 0x100000);
    {
        TTextMBufOutStream strmTar
        (
            &mbufText, tCIDLib::EAdoptOpts::NoAdopt, new TUTF8Converter
        );
        unitiSrc.FormatReport(strmTar);

        strmTar.Flush();
        c4TextBytes = strmTar.c4CurSize();
    }

    // Now let's compress it
    TZLibCompressor zlibReport;
    TBinMBufInStream strmSrc(&mbufText, c4TextBytes);
    TBinMBufOutStream strmTar(&mbufToFill);

    return zlibReport.c4Compress(strmSrc, strmTar, c4TextBytes);
}
