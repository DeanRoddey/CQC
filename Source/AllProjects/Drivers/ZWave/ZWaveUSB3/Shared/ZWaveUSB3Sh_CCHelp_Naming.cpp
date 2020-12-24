//
// FILE NAME: ZWaveUSB3Sh_CCNaming.cpp
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
//  This is a CC helper namespace to provide some node nameing class grunt work
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
//   NAMESPACE: TZWUSB3CCNaming
// ---------------------------------------------------------------------------


//
//  We assume we are getting a SET command. If so, we get the name bytes out and convert
//  from the source character set to the caller's string.
//
tCIDLib::TBoolean
TZWUSB3CCNaming::bExtractNodeName(const TZWInMsg& zwimSrc, TString& strToFill)
{
    //
    //  Get the name out and update our config info. First get the character set
    //  byte so we know how to interpret the bytes. And calculate how many bytes
    //  there are.
    //
    const tCIDLib::TCard1 c1CharSet = zwimSrc.c1CCByteAt(2) & 0x7;

    // If greater than two we don't understand it
    if (c1CharSet > 2)
    {
        // If in verbose omde, log that we couldn't do this
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcCC_BadNamingCharSet
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::Format
                , TCardinal(c1CharSet)
            );
        }
        return kCIDLib::False;
    }


    // It a character set we understand, so do it
    const tCIDLib::TCard4 c4ByteCnt = zwimSrc.c4CCBytes() - 3;

    //  Create the correct text encoder to do the conversion.
    TString strEncoder;
    if (c1CharSet == 0)
        strEncoder = L"ISO-8859-1";
    else if (c1CharSet == 1)
        strEncoder = L"ZWaveOEM";
    else
        strEncoder = L"UTF-16BE";
    TJanitor<TTextConverter> janCvt
    (
        facCIDEncode().ptcvtMake(strEncoder, kCIDLib::chUnderscore)
    );

    // And do the conversion. If we get any characters out, then store it
    if (!janCvt->c4ConvertFrom(zwimSrc.pc1CCPayload() + 3, c4ByteCnt, strToFill))
    {
        if (facZWaveUSB3Sh().eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facZWaveUSB3Sh().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW3ShErrs::errcCC_NameExtract
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::Format
                , TCardinal(c1CharSet)
            );
        }
        return kCIDLib::False;
    }

    // Normalize the name to meet CQC field name standards
    const tCIDLib::TCard4 c4Len = strToFill.c4Length();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Len; c4Index++)
    {
        const tCIDLib::TCh chCur = strToFill[c4Index];
        if (!TRawStr::bIsAlphaNum(chCur)
        &&  (chCur != kCIDLib::chHyphenMinus)
        &&  (chCur != kCIDLib::chUnderscore))
        {
            strToFill.PutAt(c4Index, kCIDLib::chUnderscore);
        }
    }
    return kCIDLib::True;
}


//
//  Send a node name query to the target node. If the
//
tCIDLib::TBoolean
TZWUSB3CCNaming::bQueryUnitName(        TZWUnitInfo&        unitiTar
                                , const tCIDLib::TBoolean   bAsync
                                , const tCIDLib::TBoolean   bKnownAwake
                                ,       TString&            strToFill
                                ,       TString&            strErrMsg)
{
    // Make sure the target unit supports naming
    if (!unitiTar.bSupportsClass(tZWaveUSB3Sh::ECClasses::Naming))
    {
        strErrMsg = L"Unit ";
        strErrMsg += unitiTar.strName();
        strErrMsg += L" does not support node naming";
        return kCIDLib::False;
    }

    try
    {
        TZWOutMsg zwomOut;
        facZWaveUSB3Sh().BuildCCMsg
        (
            zwomOut
            , unitiTar
            , COMMAND_CLASS_NODE_NAMING
            , NODE_NAMING_NODE_NAME_GET
            , NODE_NAMING_NODE_NAME_REPORT
            , tZWaveUSB3Sh::EMsgPrios::Query
        );

        // If async, just send it and we are done
        if (bAsync)
        {
            unitiTar.SendMsgAsync(zwomOut, bKnownAwake);
            return kCIDLib::True;
        }

        //
        //  Send and wait for the reply. If the unit is a non-listener and bKnownAwake
        //  is false, this will throw.
        //
        TZWInMsg zwimIn;
        unitiTar.SendWaitReply(zwomOut, zwimIn, bKnownAwake);

        // Get the character format out and translate as required
        const tCIDLib::TCard1 c1Fmt = zwimIn.c1CCByteAt(2);
        const tCIDLib::TCard4 c4Count = zwimIn.c4CCBytes() - 3;

        strToFill.Clear();
        if (c4Count)
        {
            if (c1Fmt == 0)
            {
                // Easy it's ASCII
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                    strToFill.Append(tCIDLib::TCh(zwimIn.c1CCByteAt(3 + c4Index)));
            }
             else if (c1Fmt == 2)
            {
                //
                //  It's big endian UTF-16, so flip if we are little endian. That also
                //  means that the count is half.
                //
                for (tCIDLib::TCard4 c4Index = 0; c4Index < (c4Count / 2); c4Index += 2)
                {
                    tCIDLib::TCard2 c2Cur = zwimIn.c1CCByteAt(3 + c4Index);
                    c2Cur |= zwimIn.c1CCByteAt(3 + c4Index + 1);

                    #if defined(CIDLIB_LITTLEENDIAN)
                    c2Cur = TRawBits::c2SwapBytes(c2Cur);
                    #endif
                    strToFill.Append(tCIDLib::TCh(c2Cur));
                }
            }
             else
            {
                strErrMsg.SetFormatted(c1Fmt);
                strErrMsg.Append(L" is not a known node naming format. Unit=");
                strErrMsg.Append(unitiTar.strName());
                return kCIDLib::False;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

