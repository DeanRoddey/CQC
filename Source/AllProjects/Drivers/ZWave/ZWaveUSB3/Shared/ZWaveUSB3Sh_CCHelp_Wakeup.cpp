//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Wakeup.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/20/2018
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
//  This is a CC helper namespace to provide some Wakeup class grunt work helpers.
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
//   NAMESPACE: TZWUSB3CCWakeup
// ---------------------------------------------------------------------------

//
//  Query the wakeup information for the source unit. If not async, we will wait for
//  the reply and c1Report2/c2Interval will be filled in, else it will be queued up
//  and we return immediately. If bKnownAwake it will be queued for send, even if
//  the unit is not a live listener, else it will be queued on the unit for later
//  sending when it is available.
//
//  If it's not a live listener, and not known awake, and not async mode, this will
//  cause an exception since it cannot succeed.
//
tCIDLib::TBoolean
TZWUSB3CCWakeup::bQueryWakeup(          TZWUnitInfo&        unitiSrc
                                ,       tCIDLib::TCard1&    c1ReportTo
                                ,       tCIDLib::TCard4&    c4Interval
                                , const tCIDLib::TBoolean   bAsync
                                , const tCIDLib::TBoolean   bKnownAwake
                                ,       TString&            strErrMsg)
{
    c1ReportTo = 0;
    c4Interval = 0;

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiSrc
            , COMMAND_CLASS_WAKE_UP
            , WAKE_UP_INTERVAL_GET
            , WAKE_UP_INTERVAL_REPORT
            , tZWaveUSB3Sh::EMsgPrios::Query
        );

        // If async, just queue it up and return
        if (bAsync)
        {
            unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
            return kCIDLib::True;
        }

        // Else send and wait for the response
        TZWInMsg zwimIn;
        unitiSrc.SendWaitReply(zwomOut, zwimIn, bKnownAwake);

        c4Interval = zwimIn.c1CCByteAt(2);
        c4Interval <<= 8;
        c4Interval = zwimIn.c1CCByteAt(3);
        c4Interval <<= 8;
        c4Interval = zwimIn.c1CCByteAt(4);

        c1ReportTo = zwimIn.c1CCByteAt(5);
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
//  Set a parameter to a particular value of a particular size. If not async, we
//  wait for the transmission ack. If known awake, it will get queued up for send
//  even if the unit is not a live listener.
//
tCIDLib::TBoolean
TZWUSB3CCWakeup::bSetWakeup(        TZWUnitInfo&        unitiTar
                            , const tCIDLib::TCard1     c1ReportTo
                            , const tCIDLib::TCard4     c4Interval
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg)
{
    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiTar
            , COMMAND_CLASS_WAKE_UP
            , WAKE_UP_INTERVAL_SET
            , 0
            , tCIDLib::TCard1(c4Interval >> 16)
            , tCIDLib::TCard1(c4Interval >> 8)
            , tCIDLib::TCard1(c4Interval)
            , c1ReportTo
            , tZWaveUSB3Sh::EMsgPrios::Command
        );

        // If async, just queue it up, else send and wait for ack
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
