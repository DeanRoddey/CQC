//
// FILE NAME: ZoomPlayerS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/165/2005
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
//  This handles some overflow grunt work methods from the main file to
//  keep the logic there cleaner.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZoomPlayerS.hpp"



// ---------------------------------------------------------------------------
//  TZoomPlayerSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The core message reading method. It will wait for up to the indicate
//  millis for a message to come in. It pulls out the command id and the
//  parameters into separate out parms.
//
tCIDLib::TBoolean
TZoomPlayerSDriver::bGetMsg(        tCIDLib::TCard4&    c4CmdId
                            ,       TString&            strParms
                            , const tCIDLib::TCard4     c4WaitFor)
{
    //
    //  Use a helper method to get a terminated message. we get it into a
    //  memory buffer and then transcode out of that.
    //
    const tCIDLib::TCard4 c4Bytes = c4GetTermedMsg
    (
        m_sockConn
        , c4WaitFor
        , 0xD
        , 0xA
        , kCIDLib::True
        , m_mbufIn
    );

    if (!c4Bytes)
        return kCIDLib::False;

    // Transcode it to text and do some validation
    m_tcvtIO.c4ConvertFrom(m_mbufIn.pc1Data(), c4Bytes, m_strIn);

    const tCIDLib::TCard4 c4Len = m_strIn.c4Length();
    if ((c4Len < 4) || (c4Len == 5))
        return kCIDLib::False;

    // If > 5 chars, then the 5th has to be a space, between cmd and parms
    if ((c4Len > 4) && (m_strIn[4] != kCIDLib::chSpace))
        return kCIDLib::False;

    //
    //  Move the parms, if any, out to the caller's parm and cap off our
    //  in stream to leave just the command value. If no parms, just clear
    //  the caller's parm.
    //
    if (c4Len > 4)
    {
        strParms.CopyInSubStr(m_strIn, 5);
        m_strIn.CapAt(4);
    }
     else
    {
        strParms.Clear();
    }

    // Try to convert the command id
    c4CmdId = m_strIn.c4Val(tCIDLib::ERadices::Dec);

    return kCIDLib::True;
}


//
//  These methods send a command. There's a variant for just a command value,
//  and others that take a value and parameters to pass.
//
tCIDLib::TVoid TZoomPlayerSDriver::SendCmd(const tCIDLib::TCard4 c4Cmd)
{
    // Format it into a 4 digit field, right justified
    m_strTmpOut.SetFormatted(c4Cmd);
    m_strOut.FormatToFld(m_strTmpOut, 4, tCIDLib::EHJustify::Right, L'0');
    m_strOut.Append(L"\r\n");

    // Transcode it and send it
    const tCIDLib::TCard4 c4BufSz = 32;
    tCIDLib::TCard1 ac1Buf[c4BufSz];
    const tCIDLib::TCard4 c4Len = m_strOut.c4Length();
    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo(m_strOut.pszBuffer(), c4Len, ac1Buf, c4BufSz, c4OutBytes);
    m_sockConn.Send(ac1Buf, c4OutBytes);
}


tCIDLib::TVoid
TZoomPlayerSDriver::SendCmd(const tCIDLib::TCard4 c4Cmd, const TString& strParms)
{
    // Format it into a 4 digit field, right justified
    m_strTmpOut.SetFormatted(c4Cmd);
    m_strOut.FormatToFld(m_strTmpOut, 4, tCIDLib::EHJustify::Right, L'0');
    m_strOut.Append(kCIDLib::chSpace);
    m_strOut.Append(strParms);
    m_strOut.Append(L"\r\n");

    // Transcode it and send it
    const tCIDLib::TCard4 c4BufSz = 4096;
    tCIDLib::TCard1 ac1Buf[c4BufSz];
    const tCIDLib::TCard4 c4Len = m_strOut.c4Length();
    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo(m_strOut.pszBuffer(), c4Len, ac1Buf, c4BufSz, c4OutBytes);
    m_sockConn.Send(ac1Buf, c4OutBytes);
}

tCIDLib::TVoid
TZoomPlayerSDriver::SendCmd(const   tCIDLib::TCard4     c4Cmd
                            , const tCIDLib::TCh* const pszParms)
{
    // Format cmd into a 4 digit field, right justified
    m_strTmpOut.SetFormatted(c4Cmd);
    m_strOut.FormatToFld(m_strTmpOut, 4, tCIDLib::EHJustify::Right, L'0');
    m_strOut.Append(kCIDLib::chSpace);
    m_strOut.Append(pszParms);
    m_strOut.Append(L"\r\n");

    // Transcode it and send it
    const tCIDLib::TCard4 c4BufSz = 4096;
    tCIDLib::TCard1 ac1Buf[c4BufSz];
    const tCIDLib::TCard4 c4Len = m_strOut.c4Length();
    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo(m_strOut.pszBuffer(), c4Len, ac1Buf, c4BufSz, c4OutBytes);
    m_sockConn.Send(ac1Buf, c4OutBytes);
}


//
//  These send an extended function, which takes a function name and some
//  parameter.
//
tCIDLib::TVoid
TZoomPlayerSDriver::SendExFunc( const   TString&        strFunc
                                , const tCIDLib::TCard4 c4Value)
{
    // Format the 'send function' command id into a 4 digit field, right just
    m_strTmpOut.SetFormatted(kZoomPlayerS::c4Cmd_ExFunction);
    m_strOut.FormatToFld(m_strTmpOut, 4, tCIDLib::EHJustify::Right, L'0');
    m_strOut.Append(kCIDLib::chSpace);
    m_strOut.Append(strFunc);
    m_strOut.Append(kCIDLib::chComma);
    m_strOut.AppendFormatted(c4Value);
    m_strOut.Append(L"\r\n");

    // Transcode it and send it
    const tCIDLib::TCard4 c4BufSz = 4096;
    tCIDLib::TCard1 ac1Buf[c4BufSz];
    const tCIDLib::TCard4 c4Len = m_strOut.c4Length();
    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo(m_strOut.pszBuffer(), c4Len, ac1Buf, c4BufSz, c4OutBytes);
    m_sockConn.Send(ac1Buf, c4OutBytes);

    //
    //  Wait for the ack. It actually returns the function and the value,
    //  without the newline. So we have to create another string to use as
    //  the function ack.
    //
    TString strAck;
    strAck.CopyInSubStr(m_strOut, 5, m_strOut.c4Length() - 7);
    WaitForFuncAck(kZoomPlayerS::c4Cmd_ExFunction, strAck, 5000);
}


//
//  This one sends a standard function, which just takes a function name and
//  nothing else.
//
tCIDLib::TVoid
TZoomPlayerSDriver::SendFunc(const TString& strFunc)
{
    // Format the 'send function' command id into a 4 digit field, right just
    m_strTmpOut.SetFormatted(kZoomPlayerS::c4Cmd_Function);
    m_strOut.FormatToFld(m_strTmpOut, 4, tCIDLib::EHJustify::Right, L'0');
    m_strOut.Append(kCIDLib::chSpace);
    m_strOut.Append(strFunc);
    m_strOut.Append(L"\r\n");

    // Transcode it and send it
    const tCIDLib::TCard4 c4BufSz = 4096;
    tCIDLib::TCard1 ac1Buf[c4BufSz];
    const tCIDLib::TCard4 c4Len = m_strOut.c4Length();
    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo(m_strOut.pszBuffer(), c4Len, ac1Buf, c4BufSz, c4OutBytes);
    m_sockConn.Send(ac1Buf, c4OutBytes);

    //
    //  If it wasn't an exit function, then wait for an ack. If it was an
    //  exit, then the player is going to drop the socket.
    //
    if (strFunc != kZoomPlayerS::strFunc_Exit)
        WaitForFuncAck(kZoomPlayerS::c4Cmd_Function, strFunc, 5000);
}


//
//  This is called any time we get a timeline message or if we get a
//  total time or current time message. We store away the time we got
//  for later giving back to our parent class.
//
tCIDLib::TVoid
TZoomPlayerSDriver::StoreTimeline(  const   TString&    strValue
                                    , const ETimes      eToStore)
{
    tCIDLib::TEncodedTime enctCur = kCIDLib::c8MaxCard;
    tCIDLib::TEncodedTime enctTot = kCIDLib::c8MaxCard;

    // Break the value into two parts on the slash
    m_strTmp = strValue;
    if (m_strTmp.bSplit(m_strTmp2, kCIDLib::chForwardSlash))
    {
        m_strTmp.StripWhitespace();
        m_strTmp2.StripWhitespace();

        if (eToStore == ETime_Both)
        {
            // We got both parts, so parse them both out
            enctCur = enctConvertTime(m_strTmp);
            enctTot = enctConvertTime(m_strTmp2);
        }
         else if (eToStore == ETime_Current)
        {
            enctCur = enctConvertTime(m_strTmp);
        }
         else if (eToStore == ETime_Total)
        {
            enctTot = enctConvertTime(m_strTmp2);
        }
    }

    // Store the item total if we got it
    if (enctTot != kCIDLib::c8MaxCard)
        m_enctCurTotal = enctTot;

    // Store the current item time if we got it
    if (enctCur != kCIDLib::c8MaxCard)
        m_enctCurPos = enctCur;
}



//
//  Waits for a reply message. Queries obviously return something data, but
//  commands that invoke functions return an ACK (which is just an echo back
//  of the command and function name) to indicate the function has completed.
//  This guy waits for those, processing other commands in the interrim.
//
tCIDLib::TVoid
TZoomPlayerSDriver::WaitForFuncAck( const   tCIDLib::TCard4 c4CmdId
                                    , const TString&        strFunc
                                    , const tCIDLib::TCard4 c4WaitFor)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec);

    tCIDLib::TCard4 c4CmdGot;
    TString         strParms;
    while (enctCur < enctEnd)
    {
        if (bGetMsg(c4CmdGot, strParms, 100))
        {
            // Process it first
            ProcessMsg(c4CmdGot, strParms);

            //
            //  If it's the one we are waiting for, we are done
            if ((c4CmdGot == c4CmdId) && (strFunc == strParms))
                break;
        }

        // Move the current time forward for the next round
        enctCur = TTime::enctNow();
    }

    // If we never got the ack, throw
    if (enctCur >= enctEnd)
    {
        facZoomPlayerS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZmPErrs::errcProto_Timeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
        );
    }
}


