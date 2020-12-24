//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Config.cpp
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
//  This is a CC helper namespace to provide some configuration class grunt work
//  helpers.
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
//   NAMESPACE: TZWUSB3CCConfig
// ---------------------------------------------------------------------------

//
//  Query the value of a config parameter. If not async, we will wait for the reply
//  and c4Value will be filled in, else it will be queued up and we return
//  immediately. If bKnownAwake it will be queued for send, even if the unit
//  is not a live listener, else it will be queued on the unit for later sending
//  when it is available.
//
//  If it's not a live listener, and not known awake, and not async mode, this will
//  cause an exception since it cannot succeed.
//
tCIDLib::TBoolean
TZWUSB3CCConfig::bQueryParam(       TZWUnitInfo&        unitiSrc
                            , const tCIDLib::TCard1     c1ParamNum
                            ,       tCIDLib::TInt4&     i4Value
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg)
{
    i4Value = 0;

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiSrc
            , COMMAND_CLASS_CONFIGURATION
            , CONFIGURATION_GET
            , CONFIGURATION_REPORT
            , c1ParamNum
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

        // Make sure we got the right reply
        if (zwimIn.c1CCByteAt(2) != c1ParamNum)
        {
            strErrMsg = L"Asked for cfg param ";
            strErrMsg.AppendFormatted(c1ParamNum);
            strErrMsg.Append(L" but got ");
            strErrMsg.AppendFormatted(zwimIn.c1CCByteAt(2));
            strErrMsg.Append(L". Unit=");
            strErrMsg.Append(unitiSrc.strName());
            return kCIDLib::False;
        }

        // Get the byte size out
        const tCIDLib::TCard4 c4Bytes = tCIDLib::TCard4(zwimIn.c1CCByteAt(3) & 0x7);
        if ((c4Bytes != 1) && (c4Bytes != 2) && (c4Bytes != 4))
        {
            strErrMsg = L"Got invalid cfg param size in report. Unit=";
            strErrMsg.Append(unitiSrc.strName());
            strErrMsg.Append(L", Parm=");
            strErrMsg.AppendFormatted(c1ParamNum);
            strErrMsg.Append(L", Size=");
            strErrMsg.AppendFormatted(c4Bytes);
            return kCIDLib::False;
        }

        // Get the bytes out. They are big endian
        tCIDLib::TCard4 c4Value = 0;
        if (c4Bytes == 4)
        {
            c4Value = zwimIn.c1CCByteAt(4);
            c4Value <<= 8;
            c4Value |= zwimIn.c1CCByteAt(5);
            c4Value <<= 8;
            c4Value |= zwimIn.c1CCByteAt(6);
            c4Value <<= 8;
            c4Value |= zwimIn.c1CCByteAt(7);
        }
         else if (c4Bytes == 2)
        {
            c4Value = zwimIn.c1CCByteAt(4);
            c4Value <<= 8;
            c4Value |= zwimIn.c1CCByteAt(5);
        }
         else
        {
            c4Value = zwimIn.c1CCByteAt(4);
        }


        //
        //  Now cast this to the signed value it really is. Getting the bits out into
        //  an unsigned will correctly sign extend the value to create a signed value
        //  as required.
        //
        i4Value = tCIDLib::TInt4(c4Value);
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
TZWUSB3CCConfig::bSetParam(         TZWUnitInfo&        unitiTar
                            , const tCIDLib::TCard1     c1ParamNum
                            , const tCIDLib::TCard4     c4ByteCnt
                            , const tCIDLib::TInt4      i4Value
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg)
{
    if ((c4ByteCnt != 1) && (c4ByteCnt != 2) && (c4ByteCnt != 4))
    {
        strErrMsg = L"Invalid config parameter byte count (";
        strErrMsg.AppendFormatted(c4ByteCnt);
        strErrMsg.Append(L")");
        return kCIDLib::False;
    }

    try
    {
        //
        //  This msg is variable size so we have to build it manually. It will be
        //  four bytes plus the number of param value bytes.
        //
        TZWOutMsg zwomOut;
        zwomOut.Reset
        (
            unitiTar.c1Id()
            , COMMAND_CLASS_CONFIGURATION
            , CONFIGURATION_SET
            , 0
            , tCIDLib::TCard1(4 + c4ByteCnt)
            , bAsync ? tZWaveUSB3Sh::EMsgPrios::Async : tZWaveUSB3Sh::EMsgPrios::Command
        );
        zwomOut += c1ParamNum;
        zwomOut += tCIDLib::TCard1(c4ByteCnt);

        //
        //  Cast to an unsigned, which will make bits work out correctly for what
        //  Z-Wave wants. The value is big endian, so high bytes first
        //
        tCIDLib::TCard4 c4Val = tCIDLib::TCard4(i4Value);
        if (c4ByteCnt > 2)
        {
            zwomOut += tCIDLib::TCard1(c4Val >> 24);
            zwomOut += tCIDLib::TCard1(c4Val >> 16);
        }
        if (c4ByteCnt > 1)
            zwomOut += tCIDLib::TCard1(c4Val >> 8);
        zwomOut += tCIDLib::TCard1(c4Val);

        // It's an outgoing command, so ask for a trans ack
        zwomOut.AppendTransOpts(kCIDLib::True);
        zwomOut.AppendCallback();
        zwomOut.Finalize(unitiTar);

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
