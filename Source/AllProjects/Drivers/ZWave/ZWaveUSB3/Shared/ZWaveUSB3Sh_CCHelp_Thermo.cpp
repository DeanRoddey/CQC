//
// FILE NAME: ZWaveUSB3Sh_CCHelp_Thermo.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/26/2018
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
//  This is a CC helper namespace to provide some thermostat oriented helper
//  functions.
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
//   NAMESPACE: TZWUSB3CCThermo
// ---------------------------------------------------------------------------

// Query supported modes and fan modes
tCIDLib::TBoolean
TZWUSB3CCThermo::bQueryModes(       TZWUnitInfo&        unitiSrc
                            ,       tCIDLib::TCard4&    c4FanModes
                            ,       tCIDLib::TCard4&    c4Modes
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg)
{
    c4FanModes = 0;
    c4Modes = 0;

    try
    {
        // If the unit supports the fan mode class, then get the modes
        if (unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::ThermoFanMode))
        {
            TZWOutMsg zwomOut;
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiSrc
                , COMMAND_CLASS_THERMOSTAT_FAN_MODE
                , THERMOSTAT_FAN_MODE_SUPPORTED_GET
                , THERMOSTAT_FAN_MODE_SUPPORTED_REPORT
                , tZWaveUSB3Sh::EMsgPrios::Query
            );

            if (bAsync)
            {
                unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
            }
             else
            {
                TZWInMsg zwimReply;
                unitiSrc.SendWaitReply(zwomOut, zwimReply, bKnownAwake);

                //
                //  There may be more but any we understand are covered in two mask bytes,
                //  putting them in backwards so that the low byte ends up in our low
                //  byte.
                //
                c4FanModes = zwimReply.c1CCByteAt(3);
                c4FanModes <<= 8;
                c4FanModes |= zwimReply.c1CCByteAt(2);
            }
        }

        // And do thermo mode
        if (unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::ThermoMode))
        {
            TZWOutMsg zwomOut;
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiSrc
                , COMMAND_CLASS_THERMOSTAT_MODE
                , THERMOSTAT_MODE_SUPPORTED_GET
                , THERMOSTAT_MODE_SUPPORTED_REPORT
                , tZWaveUSB3Sh::EMsgPrios::Query
            );

            if (bAsync)
            {
                unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
            }
             else
            {
                TZWInMsg zwimReply;
                unitiSrc.SendWaitReply(zwomOut, zwimReply, bKnownAwake);

                // As above, two bytes covers the ones we understand, and in reverse order
                c4Modes = zwimReply.c1CCByteAt(3);
                c4Modes <<= 8;
                c4Modes |= zwimReply.c1CCByteAt(2);
            }
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


// Query supported set points
tCIDLib::TBoolean
TZWUSB3CCThermo::bQuerySPs(         TZWUnitInfo&        unitiSrc
                            ,       tCIDLib::TCard4&    c4SPMask
                            , const tCIDLib::TBoolean   bAsync
                            , const tCIDLib::TBoolean   bKnownAwake
                            ,       TString&            strErrMsg)
{
    c4SPMask = 0;
    try
    {
        // If the unit supports the set point class, then get the modes
        if (unitiSrc.bSupportsClass(tZWaveUSB3Sh::ECClasses::ThermoSetPnt))
        {
            TZWOutMsg zwomOut;
            facZWaveUSB3Sh().BuildCCMsg
            (
                zwomOut
                , unitiSrc
                , COMMAND_CLASS_THERMOSTAT_SETPOINT
                , THERMOSTAT_SETPOINT_SUPPORTED_GET
                , THERMOSTAT_SETPOINT_SUPPORTED_REPORT
                , tZWaveUSB3Sh::EMsgPrios::Query
            );

            if (bAsync)
            {
                unitiSrc.SendMsgAsync(zwomOut, bKnownAwake);
            }
             else
            {
                TZWInMsg zwimReply;
                unitiSrc.SendWaitReply(zwomOut, zwimReply, bKnownAwake);

                //
                //  There may be more but any we understand are covered in two mask bytes,
                //  putting them in backwards so that the low byte ends up in our low
                //  byte. It's possible that they may not support any in the high byte
                //  so we have to be careful to check that.
                //
                if (zwimReply.c4CCBytes() > 3)
                {
                    c4SPMask = zwimReply.c1CCByteAt(3);
                    c4SPMask <<= 8;
                }
                c4SPMask |= zwimReply.c1CCByteAt(2);
            }
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
//  A helper to set up standard attributes for a thermostat's current temp, which
//  would be the same for any temp sensor as well.
//
tCIDLib::TVoid
TZWUSB3CCThermo::QueryTempAttrs(const   TZWUnitInfo&            unitiSrc
                                ,       tCIDMData::TAttrList&   colToFill
                                ,       TAttrData&              adatTmp)
{
    //
    //  The current temp is required, so we always do the temp scale and temp range
    //  options. If not set go with F. If any other attr is not already set, then
    //  default them based on scale.
    //
    tCIDLib::TBoolean bFScale = kCIDLib::True;
    tCIDLib::TInt4 i4Val;
    TString strRange;
    TString strTmp;
    tCIDLib::TInt4 i4Min, i4Max;
    {
        const TString strScale
        (
            unitiSrc.strQueryOptVal(kZWaveUSB3Sh::strUOpt_TempScale, L"F")
        );
        adatTmp.Set
        (
            L"Temp Scale"
            , kZWaveUSB3Sh::strUOpt_TempScale
            , L"Enum: F, C"
            , tCIDMData::EAttrTypes::String
        );
        adatTmp.SetString(strScale);
        colToFill.objAdd(adatTmp);

        // Remember the scale
        bFScale = (strScale == L"F");
    }

    // Set up a possible range for current temp
    {
        if (bFScale)
        {
            i4Min = -20;
            i4Max = 120;
        }
         else
        {
            i4Min = -30;
            i4Max = 55;
        }

        //
        //  Set the range for min to be the lower half of the range minus a little
        //  to avoid overlap.
        //
        const tCIDLib::TInt4 i4RangeTop((i4Min + ((i4Max - i4Min) / 2)) - 2);
        strRange = L"Range: ";
        strRange.AppendFormatted(i4Min);
        strRange.Append(kCIDLib::chComma);
        strRange.AppendFormatted(i4RangeTop);
        adatTmp.Set
        (
            L"Min Temp"
            , kZWaveUSB3Sh::strUOpt_CurTempMin
            , strRange
            , tCIDMData::EAttrTypes::Int
        );

        // Just in case make sure its still valid before we set it
        i4Val = unitiSrc.i4QueryOptVal(kZWaveUSB3Sh::strUOpt_CurTempMin, i4Min);
        if ((i4Val < i4Min) || (i4Val > i4RangeTop))
            i4Val = i4Min;
        adatTmp.SetInt(i4Val);
        colToFill.objAdd(adatTmp);

        //
        //  Set the range for the max to be the upper half, plus a little to avoid
        //  overlap.
        //
        const tCIDLib::TInt4 i4RangeLow(i4Min + ((i4Max - i4Min) / 2) + 2);
        strRange = L"Range: ";
        strRange.AppendFormatted(i4RangeLow);
        strRange.Append(kCIDLib::chComma);
        strRange.AppendFormatted(i4Max);
        adatTmp.Set
        (
            L"Max Temp"
            , kZWaveUSB3Sh::strUOpt_CurTempMax
            , strRange
            , tCIDMData::EAttrTypes::Int
        );
        i4Val = unitiSrc.i4QueryOptVal(kZWaveUSB3Sh::strUOpt_CurTempMax, i4Max);
        if ((i4Val < i4RangeLow) || (i4Val > i4Max))
            i4Val = i4Max;
        adatTmp.SetInt(i4Val);
        colToFill.objAdd(adatTmp);
    }
}
