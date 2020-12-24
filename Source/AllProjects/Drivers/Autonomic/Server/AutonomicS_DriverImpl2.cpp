//
// FILE NAME: AutonomicS_DriverImpl2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2007
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
//  This just contains some grunt work methods from the main implementation
//  file, to keep it from getting too big. These are related to comm to and
//  from the device.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "AutonomicS.hpp"



// ---------------------------------------------------------------------------
//  TAutonomicSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Our lowest level message getter
tCIDLib::TBoolean
TAutonomicSDriver::bGetMsg(         TString&                strToFill
                            , const tCIDLib::TEncodedTime   enctEnd)
{
    if (bGetASCIITermMsg2(m_sockConn, enctEnd, 13, 10, strToFill))
    {
        // Set the last time we got something from it
        m_enctLastMsg = TTime::enctNow();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


//
//  This is called for each message while we are waiting for another
//  response, since the server sends async messages and we have to process
//  them while we wait for other things.
//
//  We return true if we see the Running state change report, since it is
//  the last one in the list always and this lets the caller know to break
//  out of his loop.
//
tCIDLib::TBoolean TAutonomicSDriver::bProcessMsg(const TString& strMsg)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    if (strMsg.bStartsWith(m_strMsg_ReportState)
    ||  strMsg.bStartsWith(m_strMsg_StateChanged))
    {
        //
        //  It's a sync or async report of status information. We just
        //  pull out the key=value pair and if it's a key we care about,
        //  we pull out the value and store it.
        //
        static TString strKey_ArtistName(L"ArtistName");
        static TString strKey_CIDName(L"CallingPartyName");
        static TString strKey_CIDNumber(L"CallingPartyNumber");
        static TString strKey_MediaControl(L"MediaControl");
        static TString strKey_MediaName(L"MediaName");
        static TString strKey_Running(L"Running");
        static TString strKey_SessionEnd(L"SessionEnd");
        static TString strKey_SessionStart(L"SessionStart");
        static TString strKey_TotalTracks(L"TotalTracks");
        static TString strKey_TrackDuration(L"TrackDuration");
        static TString strKey_TrackName(L"TrackName");
        static TString strKey_TrackNumber(L"TrackNumber");
        static TString strKey_TrackTime(L"TrackTime");
        static TString strKey_Volume(L"Volume");

        //
        //  Strip off the prefix and the instance name. We need to skip
        //  forward to the next space after the space after the prefix.
        //
        tCIDLib::TCard4 c4Ofs;
        if (strMsg.bStartsWith(m_strMsg_ReportState))
            c4Ofs = m_strMsg_ReportState.c4Length();
        else
            c4Ofs = m_strMsg_StateChanged.c4Length();

        if (strMsg.bNextOccurrence( kCIDLib::chSpace, c4Ofs))
        {
            // Pull out the key=value pair
            TString strName;
            TString strValue;
            strMsg.CopyOutSubStr(strName, c4Ofs);
            strName.bSplit(strValue, kCIDLib::chEquals);

            if (strName == strKey_ArtistName)
            {
                bStoreStringFld(m_c4FldId_CurColArtist, strValue, kCIDLib::True);
            }
             else if (strName == strKey_CIDName)
            {
                bStoreStringFld(m_c4FldId_CIDName, strValue, kCIDLib::True);
            }
             else if (strName == strKey_CIDNumber)
            {
                bStoreStringFld(m_c4FldId_CIDNumber, strValue, kCIDLib::True);
            }
             else if (strName == strKey_MediaName)
            {
                bStoreStringFld(m_c4FldId_CurColName, strValue, kCIDLib::True);
            }
             else if (strName == strKey_MediaControl)
            {
                bStoreStringFld(m_c4FldId_MediaState, strValue, kCIDLib::True);
            }
             else if (strName == strKey_TrackDuration)
            {
                const tCIDLib::TCard4 c4Secs = strValue.c4Val(tCIDLib::ERadices::Dec);
                facCQCMedia().FormatDuration(c4Secs, strValue);
                bStoreStringFld(m_c4FldId_CurItemTotal, strValue, kCIDLib::True);
            }
             else if (strName == strKey_TrackTime)
            {
                const tCIDLib::TCard4 c4Secs = strValue.c4Val(tCIDLib::ERadices::Dec);
                facCQCMedia().FormatDuration(c4Secs, strValue);
                bStoreStringFld(m_c4FldId_CurItemTime, strValue, kCIDLib::True);
            }
             else if (strName == strKey_Running)
            {
                bStoreBoolFld(m_c4FldId_Running, strValue == m_strVal_True, kCIDLib::True);
                bRes = kCIDLib::True;
            }
             else if (strName == strKey_Volume)
            {
                bStoreCardFld(m_c4FldId_Volume, strValue.c4Val(), kCIDLib::True);
            }
        }
         else
        {
            // It's not well formed
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
            {
                facAutonomicS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Status report response was malformed, skipping it"
                    , tCIDLib::ESeverities::Status
                );
            }
        }
    }
    return bRes;
}


//
//  A combination of SendMsg() and bWaitForReply(), so it makes things
//  very convenient to send a messgae, wait for a reply and process asyncs
//  and return the eventual reply the caller cares about.
//
tCIDLib::TBoolean TAutonomicSDriver
::bSendAndWaitForRep(const  TString&            strToSend
                    , const TString&            strRepPrefix
                    , const tCIDLib::TCard4     c4WaitFor
                    ,       TString&            strToFill
                    , const tCIDLib::TBoolean   bThrowIfNot)
{
    SendMsg(strToSend);
    return bWaitForReply(strRepPrefix, c4WaitFor, strToFill, bThrowIfNot);
}


//
//  This is called in the scenarios where we are waiting for a simple reply
//  and need to process asyncs while we wait.
//
tCIDLib::TBoolean
TAutonomicSDriver::bWaitForReply(const  TString&            strReplyPref
                                , const tCIDLib::TCard4     c4WaitFor
                                ,       TString&            strToFill
                                , const tCIDLib::TBoolean   bThrowIfNot)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec);

    TThread* pthrCaller = 0;
    while (enctCur < enctEnd)
    {
        if (bGetMsg(strToFill, enctEnd))
        {
            // Process it first
            bProcessMsg(strToFill);

            // If it's the one we are waiting for, we are done
            if (strToFill.bStartsWith(strReplyPref))
                return kCIDLib::True;
        }

        // Watch for shutdown requests
        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
            return kCIDLib::False;

        // Move the current time forward for the next round
        enctCur = TTime::enctNow();
    }

    // If asked, throw because we didn't get it. Else just log if verbose
    if (bThrowIfNot)
    {
        facAutonomicS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kAnomicErrs::errcComm_RepTimeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , strReplyPref
        );
    }
     else if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facAutonomicS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kAnomicErrs::errcComm_RepTimeout
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , strReplyPref
        );
    }
    return kCIDLib::False;
}


//
//  A helper method that takes a message to send, flattens it into a buffer,
//  adds the CR/LF and sends it out.
//
tCIDLib::TVoid TAutonomicSDriver::SendMsg(const TString& strToSend)
{
    // Transcode it to ASCII into our send buffer
    tCIDLib::TCard4 c4OutBytes = m_tcvtIO.c4ConvertTo(strToSend, m_mbufSend);

    // Append the CR/LF and send it
    m_mbufSend.PutCard1(13, c4OutBytes++);
    m_mbufSend.PutCard1(10, c4OutBytes++);
    m_sockConn.Send(m_mbufSend.pc1Data(), c4OutBytes);
}


