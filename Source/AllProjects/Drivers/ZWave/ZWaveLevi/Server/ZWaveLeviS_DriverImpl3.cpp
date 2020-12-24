//
// FILE NAME: ZWaveLeviS_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  This file handles some grunt work overflow from the main driver impl
//  file. This file handles the lower level message sending and receiving
//  functionality, handling asyncs, waiting for replies and so forth.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviS_.hpp"





// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to check a message to see if it's at least a minimum legal
//  length. If not it logs if in verbose mode and return false.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bCheckMsgLen(const   tCIDLib::TCh* const pszType
                                , const tCIDLib::TCard4     c4MsgLen
                                , const tCIDLib::TCard4     c4MinLegal)
{
    if (c4MsgLen < c4MinLegal)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            LogMsg
            (
                L"Msg is too short for a %(1) reply"
                , tCQCKit::EVerboseLvls::High
                , CID_FILE
                , CID_LINE
                , TString(pszType)
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Because of the need to deal with security messages, which require that
//  we get another message while processing the first one, we break out this
//  code to pull apart the info and return it.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bParseGenRepport(const   TString&                strMsg
                                    ,       tCIDLib::TCard4* const  pc4ToFill
                                    ,       tCIDLib::TCard4&        c4Count
                                    , const tCIDLib::TCard4         c4MaxBytes
                                    ,       tCIDLib::TCard4&        c4NodeId)
{
    // Break out the node id and the list of values
    m_strTmpRep1 = strMsg;
    m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chColon);

    if (!m_strTmpRep1.bToCard4(c4NodeId, tCIDLib::ERadices::Dec)
    ||  (c4NodeId == 0)
    ||  (c4NodeId > kZWaveLeviSh::c4MaxUnitId))
    {
        m_strTmpRep1 = strMsg;
        if (m_strTmpRep1.c4Length() > 32)
        {
            m_strTmpRep1.CapAt(32);
            m_strTmpRep1.Append(L"...");
        }

        LogMsg
        (
            L"No/bad id found in incoming unit report. Msg=%(1)"
            , tCQCKit::EVerboseLvls::High
            , CID_FILE
            , CID_LINE
            , m_strTmpRep1
        );
        return kCIDLib::False;
    }


    // Now break out the rest of the values
    c4Count = 0;
    tCIDLib::TCard4 c4Val;
    TStringTokenizer stokRep(&m_strTmpRep2, L", ");
    while ((c4Count < c4MaxBytes) && stokRep.bGetNextToken(m_strTmpRep1))
    {
        if (!m_strTmpRep1.bToCard4(c4Val, tCIDLib::ERadices::Dec)
        ||  (c4Val > 255))
        {
            m_strTmpRep1 = strMsg;
            if (m_strTmpRep1.c4Length() > 64)
            {
                m_strTmpRep1.CapAt(64);
                m_strTmpRep1.Append(L"...");
            }
            LogMsg
            (
                L"Unit report byte #%(1) was invalid. Msg=%(2)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , TCardinal(c4Count + 1)
                , m_strTmpRep1
            );
            return kCIDLib::False;
        }

        pc4ToFill[c4Count++] = c4Val;
    }

    // There can't be less than two values (cmd class and command)
    if (c4Count < 2)
    {
        m_strTmpRep1 = strMsg;
        if (m_strTmpRep1.c4Length() > 32)
        {
            m_strTmpRep1.CapAt(32);
            m_strTmpRep1.Append(L"...");
        }
        LogMsg
        (
            L"Unit report had less than 2 bytes. Msg=%(1)"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
            , m_strTmpRep1
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  This method does the commonly required exchange of sending out a message
//  and waiting for an ackknowledgement (or error) to come back.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bSendAndWaitAck( const   TString&            strToSend
                                    , const tCIDLib::TCard4     c4WaitMS
                                    ,       tCIDLib::TCard4&    c4ResCode
                                    , const tCIDLib::TBoolean   bThrowErrs)
{
    //
    //  Send the passed message, and then wait for the initial message received
    //  acknowledgement.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        for (tCIDLib::TCard4 c4Retry = 0; c4Retry < 3; c4Retry++)
        {
            SendMsg(strToSend);

            bRes = bWaitMsg
            (
                tZWaveLeviS::ERepType_MsgAck
                , c4WaitMS / 3
                , c4ResCode
                , bThrowErrs && (c4Retry == 2)
            );

            if (bRes)
                break;

            if ((c4ResCode != 3)
            &&   (c4ResCode != 4)
            &&   (c4ResCode != 0)
            &&   (c4ResCode != 6)
            ||  (c4Retry == 3))
            {
                if (bThrowErrs)
                    ThrowLeviErr(c4ResCode);
                else
                    break;
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRes;
}


//
//  A convenience method that just turns around and calls the more generic
//  bWaitMsg() method below.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bWaitTransAck(const  tCIDLib::TCard4     c4WaitMS
                                ,       tCIDLib::TCard4&    c4ResCode
                                , const tCIDLib::TBoolean   bThrowErrs)
{
    return bWaitMsg
    (
        tZWaveLeviS::ERepType_TransAck, c4WaitMS, m_strTmpWait, c4ResCode, bThrowErrs
    );
}


//
//  Wait for a specific response, while processing other messages, unless
//  that is disabled. We have one that just eats the returned message, for
//  when it isn't important.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bWaitMsg(const   tZWaveLeviS::ERepTypes  eWaitFor
                            , const tCIDLib::TCard4         c4WaitMS
                            ,       tCIDLib::TCard4&        c4ResCode
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    return bWaitMsg(eWaitFor, c4WaitMS, m_strTmpWait, c4ResCode, bThrowErrs);
}

tCIDLib::TBoolean
TZWaveLeviSDriver::bWaitMsg(const   tZWaveLeviS::ERepTypes  eWaitFor
                            , const tCIDLib::TCard4         c4WaitMS
                            ,       TString&                strToFill
                            ,       tCIDLib::TCard4&        c4ResCode
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd = enctCur + (kCIDLib::enctOneMilliSec * c4WaitMS);

    while(enctCur <= enctEnd)
    {
        // Tell it to throw errors
        const tZWaveLeviS::ERepTypes eType = eGetMsg
        (
            strToFill, enctEnd, c4ResCode, bThrowErrs
        );

        // We got something so if it's our guy, we are done
        if (eType == eWaitFor)
            break;

        // Not something we are waiting for, so process
        ProcessMsg(eType, strToFill);

        enctCur = TTime::enctNow();
    }

    // If we got here before we timed out, then it worked
    return (enctCur <= enctEnd);
}


//
//  Waits up to the indicated time for a message to come in. If it gets one,
//  it figures out the type and returns the type and the message content, after
//  stripping off the < and CR.
//
//  Note that sometimes it sends CR\LF, but that's fine. Since we are doing a
//  start/stop type of message read, any trailng LF will be ignored as it searches
//  for the next start byte of the next message read.
//
//  Sometimes it's more convenient to use a millisecond wait, so we ahve another
//  one that just converts to an end time and calls the main one.
//
//  We can optionally throw timeout or device errors, or just return them
//  as message types.
//
//  Because of the fact that the VRCOP can do a 'line continuation' on lines that
//  more than a fairly short length, we have two methods here. One gets individual
//  lines. The other, higher level one, gets full messages. eGetMsg() will call
//  eGetLine(). If it sees a line continuation, it will call it again, to get the
//  remainder, until it doesn't get a continuation. It checks that the prefix is
//  the same, strips it off, and continues adding that input to the caller's
//  buffer. It would be a LOT easier if they didn't include the redundant prefix,
//  but oh well.
//
//  The bulk of the time we only get one, but we have to be prepared.
//
tZWaveLeviS::ERepTypes
TZWaveLeviSDriver::eGetLine(        TString&                strToFill
                            , const tCIDLib::TEncodedTime   enctEnd
                            ,       tCIDLib::TCard4&        c4ResCode
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    c4ResCode = 0;
    tZWaveLeviS::ERepTypes eRes = tZWaveLeviS::ERepType_None;

    // Wait for a message up to the max time
    const tCIDLib::TBoolean bRes = bGetASCIIStartStopMsg2
    (
        *m_pcommZWave, enctEnd, 0x3C, 0xD, m_strTmpIn
    );

    // If none available, return None or throw
    if (!bRes)
    {
        if (bThrowErrs)
        {
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
        return eRes;
    }

    // Strip any leading/trailing whitespace from it
    m_strTmpIn.StripWhitespace();

    // If message logging is enabled, then log the message
    if (m_bLogZWMsgs)
    {
        facZWaveLeviS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , m_strTmpIn
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    tCIDLib::TCard4 c4Len = m_strTmpIn.c4Length();

    // Can't be a valid message
    if (c4Len < 4)
    {
        LogMsg
        (
            L"Received msg was too short. Msg=%(1)"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
            , m_strTmpIn
        );
        return tZWaveLeviS::ERepType_None;
    }

    // Get the command char out and copy the rest to the caller's buffer
    const tCIDLib::TCh chCmd = m_strTmpIn[1];
    m_strTmpIn.CopyOutSubStr(strToFill, 2);
    c4Len -= 2;

    // The first character tells us what it basically is
    switch(chCmd)
    {
        case kCIDLib::chExclamation :
            eRes = tZWaveLeviS::ERepType_LearnMode;
            break;

        case kCIDLib::chLatin_E :
            // Get the code. If zero, return an ack, else throw an error
            if (!strToFill.bToCard4(c4ResCode, tCIDLib::ERadices::Dec))
                c4ResCode = 999;

            if (c4ResCode)
                eRes = tZWaveLeviS::ERepType_Error;
            else
                eRes = tZWaveLeviS::ERepType_MsgAck;
            break;

        case kCIDLib::chLatin_F :
            eRes = tZWaveLeviS::ERepType_UnitID;
            break;

        case kCIDLib::chLatin_N :
        case kCIDLib::chLatin_n :
            // It has to be at least 7 characters left
            if (bCheckMsgLen(L"Node", c4Len, 7))
            {
                switch(strToFill[3])
                {
                    case kCIDLib::chLatin_L :
                        eRes = tZWaveLeviS::ERepType_BasicReport;
                        break;

                    case kCIDLib::chLatin_S :
                        eRes = tZWaveLeviS::ERepType_SceneReport;
                        break;

                    case kCIDLib::chQuotation :
                        eRes = tZWaveLeviS::ERepType_UnitName;
                        break;

                    case kCIDLib::chColon :
                        // Either a secure status report or a regular one
                        if (chCmd == kCIDLib::chLatin_n)
                            eRes = tZWaveLeviS::ERepType_SecStatus;
                        else
                            eRes = tZWaveLeviS::ERepType_GenReport;
                        break;

                    default :
                        // Don't support this one
                        break;
                };
            }
            break;

        case kCIDLib::chLatin_X :
            //
            //  We should have gotten a code. If zero, it's an ack. If not,
            //  then it's a transmission error.
            //
            if (!strToFill.bToCard4(c4ResCode, tCIDLib::ERadices::Dec) || c4ResCode)
            {
                if (bThrowErrs)
                {
                    facZWaveLeviS().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZWErrs::errcComm_TransNak
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::XmitFailed
                    );
                }
                eRes = tZWaveLeviS::ERepType_TransNak;
            }
             else
            {
                eRes = tZWaveLeviS::ERepType_TransAck;
            }
            break;

        default :
            // Dunno what it is, so return with None
            LogMsg
            (
                L"Got an unknown message type. Prefix=%(1)"
                , tCQCKit::EVerboseLvls::High
                , CID_FILE
                , CID_LINE
                , TString(chCmd)
            );
            break;
    };

    // If we never got anything, return None or throw
    if (eRes == tZWaveLeviS::ERepType_None)
    {
        if (bThrowErrs)
        {
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }

        //
        //  Set the result code to a special value to indicate a timeout.
        //  This way, they can distinguish between an error from the device,
        //  which means it's still there and responded, vs. failure to get
        //  a message.
        //
        c4ResCode = kCIDLib::c4MaxCard;
    }
    return eRes;
}


tZWaveLeviS::ERepTypes
TZWaveLeviSDriver::eGetMsg(         TString&                strToFill
                            , const tCIDLib::TEncodedTime   enctEnd
                            ,       tCIDLib::TCard4&        c4ResCode
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    // Do an initial call to get the first (and probably only) line
    strToFill.Clear();
    tZWaveLeviS::ERepTypes eRes = eGetLine(strToFill, enctEnd, c4ResCode, bThrowErrs);

    //
    //  If successful, check to see if there is a continuation indicator. If so,
    //  then let's see if we can get it. We look until no more continuation or we
    //  run out of time, which will get us a timeout.
    //
    tCIDLib::TEncodedTime enctCont = enctEnd + (kCIDLib::enctOneMilliSec * 500);
    while ((eRes != tZWaveLeviS::ERepType_None)
    &&     (strToFill.chLast() == kCIDLib::chBackSlash))
    {
        // Delete the continuation char and try to get another line
        strToFill.DeleteLast();
        const tZWaveLeviS::ERepTypes eRes2 = eGetLine
        (
            m_strTmpRep3, enctCont, c4ResCode, bThrowErrs
        );

        //
        //  We have to have gotten some response, it has to have a colon, and it
        //  has to match the previous message up to the colon, and it has to be the
        //  same type of line as the last. If not, then something is wrong.
        //
        tCIDLib::TCard4 c4ColonOfs = 0;
        if ((eRes2 != eRes)
        ||  !m_strTmpRep3.bFirstOccurrence(kCIDLib::chColon, c4ColonOfs, kCIDLib::False)
        ||  m_strTmpRep3.eCompareNI(strToFill, c4ColonOfs))
        {
            // Something isn't right
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcComm_BadMsgCont
                , m_strTmpRep3
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Strip out the prefix and add this to the previous contents, adding a comma
        m_strTmpRep3.Cut(0, c4ColonOfs + 1);
        strToFill.Append(kCIDLib::chComma);
        strToFill.Append(m_strTmpRep3);
    }
    return eRes;
}


tZWaveLeviS::ERepTypes
TZWaveLeviSDriver::eGetMsgMSs(          TString&            strToFill
                                , const tCIDLib::TCard4     c4WaitMS
                                ,       tCIDLib::TCard4&    c4ResCode
                                , const tCIDLib::TBoolean   bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4WaitMS);
    return eGetMsg(strToFill, enctEnd, c4ResCode, bThrowErrs);
}


//
//  If we get a device error status, this can be called to log it if we are
//  above a minimum verbosity.
//
tCIDLib::TVoid
TZWaveLeviSDriver::LogLeviErr(  const   tCIDLib::TCard4         c4ErrCode
                                , const tCQCKit::EVerboseLvls   eMinLevel)
{
    const tCIDLib::TCh* pszErr = pszXlatLeviErr(c4ErrCode);
    if (eVerboseLevel() > eMinLevel)
    {
        facZWaveLeviS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcComm_DevError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Nak
            , TCardinal(c4ErrCode)
            , TString(pszErr)
        );
    }
}


// Translate a Leviton error to a string
const tCIDLib::TCh*
TZWaveLeviSDriver::pszXlatLeviErr(const tCIDLib::TCard4 c4ResCode) const
{
    // Get an error message for this guy
    const tCIDLib::TCh* pszErr = L"Unknown error type";
    switch(c4ResCode)
    {
        case 1 :
            pszErr = L"Wrong start character";
            break;

        case 2 :
            pszErr = L"Input buffer overflow";
            break;

        case 3 :
            pszErr = L"Can't send, all buffers taken";
            break;

        case 4 :
            pszErr = L"Previous transmission not done yet";
            break;

        case 5 :
            pszErr = L"Unknown command";
            break;

        case 6 :
            pszErr = L"Previous cmd not processed";
            break;

        case 7 :
            pszErr = L"Missing data fields";
            break;

        case 8 :
            pszErr = L"Node is SUC";
            break;

        case 9 :
            pszErr = L"EEPROM is busy";
            break;

        case 10 :
            pszErr = L"No such device found";
            break;


        // A special one that we use, not from the device
        case kCIDLib::c4MaxCard :
            pszErr = L"Timed out waiting for Leviton response";
            break;

        default :
            // Return unknown
            break;
    };
    return pszErr;
}



//
//  If we see a message that we know we need to process or see other messages
//  while we are waiting on other things, then this is called to process
//  the message and store data or state info if needed.
//
tCIDLib::TVoid
TZWaveLeviSDriver::ProcessMsg(  const   tZWaveLeviS::ERepTypes  eType
                                , const TString&                strMsg)
{
    switch (eType)
    {
        case tZWaveLeviS::ERepType_BasicReport :
            //
            //  It's some sort of basic status report from a unit, so we need
            //  to pull out the unit id and pass along the data to it to be
            //  processed.
            //
            ProcessBasicReport(strMsg);
            break;

        case tZWaveLeviS::ERepType_GenReport :
        case tZWaveLeviS::ERepType_SecStatus :
            //
            //  It's some sort of general report from a unit, so we need
            //  to pull out the unit id and pass along the data to it to be
            //  processed. Could be a secure one, so we pass along the type.
            //
            ProcessGenReport(eType, strMsg);
            break;

        case tZWaveLeviS::ERepType_SceneReport :
            ProcessSceneReport(strMsg);
            break;

        default :
            // Nothing we need or want to process
            break;
    };
}


//
//  If we get a basic unit reort, this is called to parse out the info and
//  look up the target unit in our unit configuration and pass the info on
//  to him in order to be stored if needed.
//
tCIDLib::TVoid TZWaveLeviSDriver::ProcessBasicReport(const TString& strMsg)
{
    // We should get data in the form iLv, where i is the id and v is the value
    m_strTmpRep1 = strMsg;
    tCIDLib::TCard4 c4Id;
    tCIDLib::TCard4 c4Val;
    if ((m_strTmpRep1.c4Length() > 7)
    ||  !m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chLatin_L)
    ||  !m_strTmpRep1.bToCard4(c4Id, tCIDLib::ERadices::Dec)
    ||  !m_strTmpRep2.bToCard4(c4Val, tCIDLib::ERadices::Dec))
    {
        LogMsg
        (
            L"Badly formed basic report received"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
        );
        return;
    }

    // Look up this unit by id and see if we need to do anything with it
    TZWaveUnit* punitTar = m_dcfgCurrent.punitFindById(c4Id);
    if (punitTar)
    {
        TString strMsg(L"Unit %(1) (%(2)) ");
        if (punitTar->bStoreBasicReport(c4Val, *this))
            strMsg.Append(L"accepted");
        else
            strMsg.Append(L"rejected");
        strMsg.Append(L" incoming basic report. Value=%(3)");

        LogMsg
        (
            strMsg
            , tCQCKit::EVerboseLvls::High
            , CID_FILE
            , CID_LINE
            , punitTar->strName()
            , TCardinal(punitTar->c4Id())
            , TCardinal(c4Val)
        );
    }
}


//
//  If we get a general unit reort, this is called to parse out the info and
//  look up the target unit in our unit configuration and pass the info on
//  to him in order to be stored if needed. Only it will know how to deal
//  with the data.
//
//  The form is nodeid:cmdclass,cmd[,val1, val2,...]
//
//  We break these values out and call the unit to deal with the results.
//
//  WE CAN get a secure status report, which is the above, but there's an
//  extra leading byte before the command class, which we throw away so
//  the unit objects can treat them the same.
//
tCIDLib::TVoid
TZWaveLeviSDriver::ProcessGenReport(const   tZWaveLeviS::ERepTypes  eType
                                    , const TString&                strMsg)
{
    const tCIDLib::TCard4 c4MaxBytes = 230;
    tCIDLib::TCard4 ac4Bytes[c4MaxBytes];

    // Break out the incoming info
    tCIDLib::TCard4 c4Id;
    tCIDLib::TCard4 c4Count;
    tCIDLib::TBoolean bRes = bParseGenRepport
    (
        strMsg, ac4Bytes, c4Count, c4MaxBytes, c4Id
    );

    // If it was bad, give up now
    if (!bRes)
        return;

    //
    //  It looks ok, so see if we have such a unit in our list. If not, then
    //  just skip it.
    //
    TZWaveUnit* punitTar = m_dcfgCurrent.punitFindById(c4Id);
    if (!punitTar)
        return;

    //
    //  If it's a secure report, we want to move all the values down by
    //  1 and reduce the count.
    //
    if (eType == tZWaveLeviS::ERepType_SecStatus)
    {
        for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
            ac4Bytes[c4Index - 1] = ac4Bytes[c4Index];
        c4Count--;
    }

    // We survived, so pass it on
    bRes = punitTar->bStoreGenReport(ac4Bytes, c4Count, *this);

    // If verbose, log acceptance or rejection
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        TString strMsg(L"Unit %(1) (%(2)) ");

        if (bRes)
            strMsg.Append(L"accepted");
        else
            strMsg.Append(L"rejected");

        strMsg.Append(L" incoming general report. Class/Cmd=");
        strMsg.AppendFormatted(ac4Bytes[0], tCIDLib::ERadices::Hex);
        strMsg.Append(kCIDLib::chForwardSlash);
        strMsg.AppendFormatted(ac4Bytes[1], tCIDLib::ERadices::Hex);

        LogMsg
        (
            strMsg
            , tCQCKit::EVerboseLvls::High
            , CID_FILE
            , CID_LINE
            , punitTar->strName()
            , TCardinal(punitTar->c4Id())
        );
    }
}


//
//  If we get a scene reort, this is called to parse out the info and
//  store it.
//
tCIDLib::TVoid TZWaveLeviSDriver::ProcessSceneReport(const TString& strMsg)
{
    //
    //  We should get data in the form iSs,l,t, is the id of the node, S
    //  means it's a scene command, and we get three values, the scene
    //  number, the level, and the fade time.
    //
    m_strTmpRep2 = strMsg;
    tCIDLib::TCard4 c4Id;
    if (!m_strTmpRep2.bSplit(m_strTmpRep1, kCIDLib::chLatin_S)
    ||  !m_strTmpRep2.bToCard4(c4Id, tCIDLib::ERadices::Dec))
    {
        LogMsg
        (
            L"Badly formed scene report received"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
        );
        return;
    }

    // The rest should break into three integer values
    tCIDLib::TCard4 c4SceneNum;
    tCIDLib::TCard4 c4Level;
    tCIDLib::TCard4 c4Fade;

    if (!m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chComma)
    ||  !m_strTmpRep2.bSplit(m_strTmpRep3, kCIDLib::chComma)
    ||  !m_strTmpRep1.bToCard4(c4SceneNum, tCIDLib::ERadices::Dec)
    ||  !m_strTmpRep2.bToCard4(c4Level, tCIDLib::ERadices::Dec)
    ||  !m_strTmpRep3.bToCard4(c4Fade, tCIDLib::ERadices::Dec))
    {
        LogMsg
        (
            L"Badly formed scene report received"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
        );
        return;
    }

    // Look up this unit by id and see if we need to do anything with it
    TZWaveUnit* punitTar = m_dcfgCurrent.punitFindById(c4Id);
    if (punitTar)
    {
        TString strMsg(L"Unit %(1) (%(2)) ");
        if (punitTar->bStoreSceneReport(c4SceneNum, c4Level, c4Fade, *this))
            strMsg.Append(L"accepted");
        else
            strMsg.Append(L"rejected");
        strMsg.Append(L" incoming scene report. Scene=%(3), Level=%(4)");

        LogMsg
        (
            strMsg
            , tCQCKit::EVerboseLvls::High
            , CID_FILE
            , CID_LINE
            , punitTar->strName()
            , TCardinal(punitTar->c4Id())
            , TCardinal(c4SceneNum)
            , TCardinal(c4Level)
        );
    }
}


//
//  Given a basic message, we add the > to the start and ;<CR> to the end,
//  transcode to binary, and send it.
//
//  Before we send, we process any waiting messages. This deals with the
//  issue where some acks from previous things timed out and are still there.
//  This way we'll chew them up before we send another message and hopefully
//  we'll only see acks legitimately for this message, though there's always
//  a chance that a bogus one is on the way now.
//
tCIDLib::TVoid TZWaveLeviSDriver::SendMsg(const TString& strToSend)
{
    tCIDLib::TCard4 c4ResCode;
    while (bWaitMsg(tZWaveLeviS::ERepTypes_Count, 5, c4ResCode, kCIDLib::False))
    {
    }

    // Build up the actual message
    m_strTmpOut = L">";
    m_strTmpOut.Append(strToSend);

    // If message logging is enabled
    if (m_bLogZWMsgs)
    {
        facZWaveLeviS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , m_strTmpOut
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Transcode it to the output memory buffer and get the byte count
    tCIDLib::TCard4 c4Len;
    m_tcvtOut.c4ConvertTo(m_strTmpOut, m_mbufOut, c4Len);

    // Put the CR on
    m_mbufOut.PutCard1(0xD, c4Len++);

    //
    //  We enforce a minimum send interval. If we haven't passed that,
    //  then pause for the remaining time.
    //
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur < m_enctNextTrans)
    {
        const tCIDLib::TCard4 c4Millis
        (
            tCIDLib::TCard4((m_enctNextTrans - enctCur) / kCIDLib::enctOneMilliSec)
        );

        if (c4Millis)
            TThread::Sleep(c4Millis);
    }

    //
    //  And send it. Make sure we get the next possible time updated AFTER
    //  we send, so that time taken to send doesn't come out of the interval.
    //
    try
    {
        if (m_pcommZWave->c4WriteMBufMS(m_mbufOut, c4Len, 500) != c4Len)
        {
            // We didn't send all the bytes so throw
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcComm_SendFailed
                , strToSend
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::XmitFailed
            );
        }
        m_enctNextTrans = TTime::enctNowPlusMSs(kZWaveLeviS::c4MinSendInterval);
    }

    catch(TError& errToCatch)
    {
        m_enctNextTrans = TTime::enctNowPlusMSs(kZWaveLeviS::c4MinSendInterval);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Does the common function of sending a unit oriented (single-cast) command
//  out, which will result in both an ack and a transmission ack, each of which
//  could indicate an error. If it doesn't receive one or if one of them is
//  in error, it throws (logging if in verbose mode.) It processes messages
//  while waiting.
//
tCIDLib::TVoid
TZWaveLeviSDriver::SendUnitCmd( const   TString&        strToSend
                                , const tCIDLib::TCard4 c4WaitMS)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4WaitMS);

    tCIDLib::TCard4 c4ResCode;
    bSendAndWaitAck(strToSend, c4WaitMS, c4ResCode, kCIDLib::True);

    //
    //  See if we have any time left. We'll cheat a little if none, since we
    //  did get the initial ack.
    //
    tCIDLib::TCard4 c4MillisLeft = 0;
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    if (enctCur < enctEnd)
        c4MillisLeft = tCIDLib::TCard4((enctEnd - enctCur) / kCIDLib::enctOneMilliSec);
    if (!c4MillisLeft)
        c4MillisLeft = 2500;
    bWaitTransAck(c4MillisLeft, c4ResCode);
}



//
//  If we get a device error status, this can be called to throw it as
//  an exception.
//
tCIDLib::TVoid
TZWaveLeviSDriver::ThrowLeviErr(const tCIDLib::TCard4 c4ErrCode)
{
    const tCIDLib::TCh* pszErr = pszXlatLeviErr(c4ErrCode);

    facZWaveLeviS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kZWErrs::errcComm_DevError
        , TString(pszErr)
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Nak
        , TCardinal(c4ErrCode)
    );
}


