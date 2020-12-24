//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Assoc.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2017
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
//  This is a CC helper namespace to provide some associationg oriented helper
//  namespaces.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSB3Sh_.hpp"



// ---------------------------------------------------------------------------
//   NAMESPACE: TZWUSB3CCAssoc
// ---------------------------------------------------------------------------

//
//  Query the associated ids/end points from the indicated group number within the
//  the indicated unit. See the header comments! This one is synchronous only!
//
tCIDLib::TBoolean
TZWUSB3CCAssoc::bQueryAssociations(         MZWaveCtrlIntf* const   pmzwciOwner
                                    , const TZWUnitInfo&            unitiSrc
                                    , const tCIDLib::TCard1         c1GroupNum
                                    ,       tCIDLib::TCardList&     fcolToFill
                                    ,       TString&                strErrMsg)
{
    fcolToFill.RemoveAll();

    // Remember if multi-channel support is available, for use below
    const tCIDLib::TBoolean bMultiCh
    (
        unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::MultiChAssoc)
    );
    try
    {
        // Build the appropriate type of query depending on support
        TZWOutMsg zwomOut;
        if (bMultiCh)
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiSrc
                , COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2
                , MULTI_CHANNEL_ASSOCIATION_GET_V2
                , MULTI_CHANNEL_ASSOCIATION_REPORT_V2
                , c1GroupNum
                , tZWaveUSB3Sh::EMsgPrios::Query
            );
        }
         else if (unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::Association))
        {
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiSrc
                , COMMAND_CLASS_ASSOCIATION
                , ASSOCIATION_GET
                , ASSOCIATION_REPORT
                , c1GroupNum
                , tZWaveUSB3Sh::EMsgPrios::Query
            );
        }
         else
        {
            strErrMsg = L"Unit '";
            strErrMsg.Append(unitiSrc.strName());
            strErrMsg.Append(L"' does not support associations");
            return kCIDLib::False;
        }

        //
        //  Send it and wait for the acks. If we get those we'll fall through below
        //  to wait for however many replies there.
        //
        pmzwciOwner->USB3SendCCMsg(zwomOut);

        //
        //  While more reports, process them. For safety, use a loop counter and
        //  don't do more than 16;
        //
        tCIDLib::TCard4 c4SoFar = 0;
        TZWInMsg zwimReply;
        while (c4SoFar < 16)
        {
            // Wait for a reply, throw if it doesn't arrive
            tCIDLib::TBoolean bGotReply;

            if (bMultiCh)
            {
                bGotReply = pmzwciOwner->bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , unitiSrc.c1Id()
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2
                    , MULTI_CHANNEL_ASSOCIATION_REPORT_V2
                    , kCIDLib::enctFourSeconds
                    , zwimReply
                    , kCIDLib::False
                );
            }
             else
            {
                bGotReply = pmzwciOwner->bUSB3WaitForMsg
                (
                    tZWaveUSB3Sh::EMsgTypes::Request
                    , unitiSrc.c1Id()
                    , FUNC_ID_APPLICATION_COMMAND_HANDLER
                    , COMMAND_CLASS_ASSOCIATION
                    , ASSOCIATION_REPORT
                    , kCIDLib::enctFourSeconds
                    , zwimReply
                    , kCIDLib::False
                );
            }

            // If no reply, we failed
            if (!bGotReply)
            {
                strErrMsg = L"Did not get all replies to association query for unit '";
                strErrMsg.Append(unitiSrc.strName());
                return kCIDLib::False;
            }

            //
            //  Let's pull out the values. This is a bit more complex since it is a
            //  mix of single and multi-end point values. They start at CC offset
            //  5, so we just consume until we hit the end.
            //
            tCIDLib::TCard4 c4CCByteCnt = zwimReply.c4CCBytes();
            tCIDLib::TCard4 c4CCIndex = 5;

            // First we get single point ones
            if (bMultiCh)
            {
                tCIDLib::TBoolean bSingleMode = kCIDLib::True;
                while (c4CCIndex < c4CCByteCnt)
                {
                    tCIDLib::TCard4 c4Val = zwimReply.c1CCByteAt(c4CCIndex++);

                    // If we see the marker, switch modes
                    if (c4Val == MULTI_CHANNEL_ASSOCIATION_REPORT_MARKER_V2)
                    {
                        bSingleMode = kCIDLib::False;
                        continue;
                    }

                    // If in single mode, set the low to all 1s, else to the end point
                    c4Val <<= 16;
                    if (bSingleMode)
                        c4Val |= kCIDLib::c2MaxCard;
                    else
                        c4Val |= zwimReply.c1CCByteAt(c4CCIndex++);

                    fcolToFill.c4AddElement(c4Val);
                }
            }
             else
            {
                const tCIDLib::TCard4 c4CCByteCnt = zwimReply.c4CCBytes();
                tCIDLib::TCard4 c4CCIndex = 5;
                while (c4CCIndex < c4CCByteCnt)
                {
                    tCIDLib::TCard4 c4Cur = zwimReply.c1CCByteAt(c4CCIndex++);
                    c4Cur <<= 16;
                    c4Cur |= kCIDLib::c2MaxCard;

                    fcolToFill.c4AddElement(c4Cur);
                }
            }


            // If no more replies coming, then we are done
            if (!zwimReply.c1CCByteAt(4))
                break;

            c4SoFar++;
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

//
//  Query how many association groups this unit has. If not async, we wait for the
//  reply and return the count. Else we just send it and return nothing.
//
tCIDLib::TBoolean
TZWUSB3CCAssoc::bQueryAssocGrps(        TZWUnitInfo&        unitiTar
                                ,       tCIDLib::TCard4&    c4ToFill
                                , const tCIDLib::TBoolean   bAsync
                                , const tCIDLib::TBoolean   bKnownAwake
                                ,       TString&            strErrMsg)
{
    // Make sure the target unit supports assoc
    if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Association))
    {
        strErrMsg = L"Unit ";
        strErrMsg.Append(unitiTar.strName());
        strErrMsg.Append(L" does not support association");
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiTar
            , COMMAND_CLASS_ASSOCIATION
            , ASSOCIATION_GROUPINGS_GET
            , ASSOCIATION_GROUPINGS_REPORT
            , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Query
            , kCIDLib::False
        );

        if (bAsync)
        {
            unitiTar.SendMsgAsync(zwomOut, bKnownAwake);
        }
         else
        {
            TZWInMsg zwimReply;
            unitiTar.SendWaitReply(zwomOut, zwimReply, bKnownAwake);

            c4ToFill = zwimReply.c1CCByteAt(2);
        }
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This function will remove a regular or multi-channel association, depending on
//  he value of c1EPId. It can also do sync or async. If sync, it will wait for
//  the tranmission ack, else it doesn't wait at all.
//
//  Note we don't SEND a multi-channel message, we are setting a multi-channel
//  association, which we do by talking to the root device.
//
tCIDLib::TBoolean
TZWUSB3CCAssoc::bRemoveAssoc(       TZWUnitInfo&        unitiTar
                            , const tCIDLib::TCard1     c1GroupNum
                            , const tCIDLib::TCard1     c1IdToRemove
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg
                            , const tCIDLib::TCard1     c1EPId)
{
    if (!c1IdToRemove || (c1IdToRemove > kZWaveUSB3Sh::c4MaxUnits))
    {
        strErrMsg = L"'";
        strErrMsg.AppendFormatted(c1IdToRemove);
        strErrMsg.Append(L"' is not a valid unit id");
        return kCIDLib::False;
    }

    if (!c1GroupNum)
    {
        strErrMsg = L"The group number to remove to cannot be zero";
        return kCIDLib::False;
    }

    // If the end point id is not 0xFF, make sure it's 0 to 127
    if ((c1EPId > 127) && (c1EPId != 0xFF))
    {
        strErrMsg = L"The end point id must be 0 to 127, got ";
        strErrMsg.AppendFormatted(c1EPId);
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        if (c1EPId != 0xFF)
        {
            // Make sure the target unit supports multi-channel assoc
            if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::MultiChAssoc))
            {
                strErrMsg = L"Unit ";
                strErrMsg.Append(unitiTar.strName());
                strErrMsg.Append(L" does not support multi-channel association");
                return kCIDLib::False;
            }

            //
            //  Need to build this on up by hand. Four CC bytes. Request a callback
            //  since this is a command, not a query.
            //
            zwomOut.Reset
            (
                unitiTar.c1Id()
                , COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2
                , MULTI_CHANNEL_ASSOCIATION_REMOVE_V2
                , 0
                , 4
                , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Command
            );
            zwomOut += c1GroupNum;
            zwomOut += MULTI_CHANNEL_ASSOCIATION_REMOVE_MARKER_V2;
            zwomOut += c1IdToRemove;
            zwomOut += c1EPId;
            zwomOut.AppendTransOpts(kCIDLib::True);
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiTar);
        }
         else
        {
            // Make sure the target unit supports assoc
            if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Association))
            {
                strErrMsg = L"Unit ";
                strErrMsg.Append(unitiTar.strName());
                strErrMsg.Append(L" does not support association");
                return kCIDLib::False;
            }

            // Just removing a regular target
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiTar
                , COMMAND_CLASS_ASSOCIATION
                , ASSOCIATION_REMOVE
                , 0
                , c1GroupNum
                , c1IdToRemove
                , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Command
                , kCIDLib::True
            );
        }

        //
        //  If async, we just queue it up and wait for nothing, else we wait for
        //  the ack we requested.
        //
        if (bAsync)
            unitiTar.SendMsgAsync(zwomOut, bKnownAwake);
        else
            unitiTar.SendWaitAck(zwomOut, bKnownAwake);
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  This function will set a regular or multi-channel association, depending on
//  he value of c1EPId. It can also do sync or async. If sync, it will wait for
//  the tranmission ack, else it doesn't wait at all.
//
//  Note we don't SEND a multi-channel message, we are setting a multi-channel
//  association, which we do by talking to the root device.
//
tCIDLib::TBoolean
TZWUSB3CCAssoc::bSetAssoc(          TZWUnitInfo&        unitiTar
                            , const tCIDLib::TCard1     c1GroupNum
                            , const tCIDLib::TCard1     c1IdToAdd
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg
                            , const tCIDLib::TCard1     c1EPId)
{
    if (!c1IdToAdd || (c1IdToAdd > kZWaveUSB3Sh::c4MaxUnits))
    {
        strErrMsg = L"'";
        strErrMsg.AppendFormatted(c1IdToAdd);
        strErrMsg.Append(L"' is not a valid unit id");
        return kCIDLib::False;
    }

    if (!c1GroupNum)
    {
        strErrMsg = L"The group number to associate to cannot be zero";
        return kCIDLib::False;
    }

    // If the end point id is not 0xFF, make sure it's 0 to 127
    if ((c1EPId > 127) && (c1EPId != 0xFF))
    {
        strErrMsg = L"The end point id must be 0 to 127, got ";
        strErrMsg.AppendFormatted(c1EPId);
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        if (c1EPId != 0xFF)
        {
            // Make sure the target unit supports multi-channel assoc
            if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::MultiChAssoc))
            {
                strErrMsg = L"The target unit does not support multi-channel association";
                return kCIDLib::False;
            }

            //
            //  Need to build this on up by hand. Four CC bytes. Request a callback
            //  since this is a command, not a query.
            //
            zwomOut.Reset
            (
                unitiTar.c1Id()
                , COMMAND_CLASS_MULTI_CHANNEL_ASSOCIATION_V2
                , MULTI_CHANNEL_ASSOCIATION_SET_V2
                , 0
                , 4
                , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Command
            );
            zwomOut += c1GroupNum;
            zwomOut += MULTI_CHANNEL_ASSOCIATION_SET_MARKER_V2;
            zwomOut += c1IdToAdd;
            zwomOut += c1EPId;
            zwomOut.AppendTransOpts(kCIDLib::True);
            zwomOut.AppendCallback();
            zwomOut.Finalize(unitiTar);
        }
         else
        {
            // Just setting a regular target
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiTar
                , COMMAND_CLASS_ASSOCIATION
                , ASSOCIATION_SET
                , 0
                , c1GroupNum
                , c1IdToAdd
                , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Command
                , kCIDLib::True
            );
        }

        //
        //  If async, we just queue it up and wait for nothing, else we wait for
        //  the ack we requested.
        //
        if (bAsync)
            unitiTar.SendMsgAsync(zwomOut, bKnownAwake);
        else
            unitiTar.SendWaitAck(zwomOut, bKnownAwake);
    }

    catch(TError& errToCatch)
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

