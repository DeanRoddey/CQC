//
// FILE NAME: ZWaveLeviS_DriverImpl3.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2014
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
#include    "ZWaveLevi2S_.hpp"





// ---------------------------------------------------------------------------
//  TZWaveLevi2SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to check a message to see if it's at least a minimum legal
//  length. If not it logs if in verbose mode and return false.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bCheckMsgLen(const  tCIDLib::TCh* const pszType
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
//  This is called to parse a general report, either regular or secure. If it's
//  a secure one, then we ignore the first value, byte and take the rest.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bParseGenReport(const   tCIDLib::TBoolean   bSecure
                                    , const TString&            strMsg
                                    ,       tCIDLib::TCard4&    c4NodeId
                                    ,       tCIDLib::TCard4&    c4InstId
                                    ,       tCIDLib::TCardList& fcolVals)
{
    // If we bail, make sure the outputs are reasonable
    c4NodeId = 0;
    c4InstId = 0;
    fcolVals.RemoveAll();

    // Break out the node id and the list of values
    m_strTmpRep1 = strMsg;
    m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chColon);


    if (!m_strTmpRep1.bToCard4(c4NodeId, tCIDLib::ERadices::Dec)
    ||  (c4NodeId == 0)
    ||  (c4NodeId > kZWaveLevi2Sh::c4MaxUnitId))
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

    // If we are dealing with a secure message, drop the first token
    TStringTokenizer stokRep(&m_strTmpRep2, L", ");
    if (bSecure)
        stokRep.bGetNextToken(m_strTmpRep1);

    // Now break out the rest of the values
    tCIDLib::TCard4 c4Val;
    while (stokRep.bGetNextToken(m_strTmpRep1))
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
                , TCardinal(fcolVals.c4ElemCount() + 1)
                , m_strTmpRep1
            );
            return kCIDLib::False;
        }

        fcolVals.c4AddElement(c4Val);
    }

    // There can't be less than two values (cmd class and command)
    if (fcolVals.c4ElemCount() < 2)
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

    // See if it's an encapsulated msg, return zero if not multi
    c4InstId = 0;
    if (fcolVals[0] == COMMAND_CLASS_MULTI_INSTANCE)
    {
        if (fcolVals[1] == MULTI_CHANNEL_CMD_ENCAP_V2)
        {
            // Get the instance id out and move the rest down
            c4InstId = fcolVals[2];
            fcolVals.Delete(0, 4);
        }
         else if (fcolVals[1] == MULTI_INSTANCE_CMD_ENCAP_V2)
        {
            // Get the instance id out and move the rest down
            c4InstId = fcolVals[2];
            fcolVals.Delete(0, 3);
        }
    }
    return kCIDLib::True;
}


//
//  A simple send and wait reply for generic msgs that aren't being done via the unit
//  and command class objects.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bSendWaitRep(const  tCIDLib::TCard4             c4ForUnitId
                                , const tCIDLib::TCard4             c4ForInstId
                                , const tCIDLib::TCard4             c4ForClass
                                , const tCIDLib::TCard4             c4ForCmd
                                ,       tCIDLib::TCardList&         fcolVals
                                , const tZWaveLevi2Sh::EMultiTypes  eMultiType
                                , const tCIDLib::TBoolean           bSecure
                                , const tCIDLib::TBoolean           bThrowErrs)
{
    // Send it and wait for the ack
    if (!bSendWaitAck(c4ForUnitId, c4ForInstId, fcolVals, eMultiType, bSecure, bThrowErrs))
        return kCIDLib::False;

    // Wait for the trans ack
    if (!bWaitTransAck(bThrowErrs))
        return kCIDLib::False;

    // OK, wait for our expected response
    tZWaveLevi2S::ERepTypes eGot;
    return bWaitMsgMS
    (
        tZWaveLevi2S::ERepType_GenReport
        , kZWaveLevi2S::c4WaitTimeout
        , c4ForUnitId
        , c4ForInstId
        , c4ForClass
        , c4ForCmd
        , eGot
        , fcolVals
        , bThrowErrs
    );
}



//
//  This method does the commonly required exchange of sending out a msg and and
//  waiting for an ackknowledgement (or error) to come back. This is a special
//  case since we want to allow for a few failures and retries. It is also non-node
//  and instance specific, since it's just between us and the VRCOP, so we don't
//  check for a specific node in the response. We provide a couple of simpler versions
//  that can be commonly used to reduce verbosity.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bSendWaitAck(const  TZWaveUnit&             unitTar
                                , const TZWCmdClass&            zwccSrc
                                , const tCIDLib::TCardList&     fcolVals
                                , const tCIDLib::TBoolean       bThrowErrs)
{
    // Just call the other one
    return bSendWaitAck
    (
        unitTar.c4Id()
        , zwccSrc.c4InstId()
        , fcolVals
        , zwccSrc.eMultiType()
        , zwccSrc.bSecure()
        , bThrowErrs
    );
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bSendWaitAck(const  tCIDLib::TCard4             c4UnitId
                                , const tCIDLib::TCard4             c4InstId
                                , const tCIDLib::TCardList&         fcolVals
                                , const tCIDLib::TBoolean           bThrowErrs)
{
    return bSendWaitAck
    (
        c4UnitId
        , c4InstId
        , fcolVals
        , tZWaveLevi2Sh::EMultiTypes::Single
        , kCIDLib::False
        , bThrowErrs
    );
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bSendWaitAck(const  tCIDLib::TCard4             c4UnitId
                                , const tCIDLib::TCard4             c4InstId
                                , const tCIDLib::TCardList&         fcolVals
                                , const tZWaveLevi2Sh::EMultiTypes  eMultiType
                                , const tCIDLib::TBoolean           bSecure
                                , const tCIDLib::TBoolean           bThrowErrs)
{
    //
    //  Send the passed message, and then wait for the initial message received
    //  acknowledgement.
    //
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        tZWaveLevi2S::ERepTypes eGot;
        tCIDLib::TCardList      fcolReply(12);

        for (tCIDLib::TCard4 c4Retry = 0; c4Retry < 4; c4Retry++)
        {
            SendMsg(c4UnitId, c4InstId, bSecure, eMultiType, fcolVals);

            // These are not node oriented, so we don't indicate a node/instance
            bRes = bWaitMsgMS
            (
                tZWaveLevi2S::ERepType_MsgAck
                , 1500
                , 0
                , 256
                , 0
                , 0
                , eGot
                , fcolReply
                , bThrowErrs && (c4Retry == 3)
            );

            if (bRes)
                break;

            // If we got an error, see if it's one we can be tolerant of
            if (eGot == tZWaveLevi2S::ERepType_Error)
            {
                if ((fcolReply[0] != 3)
                &&  (fcolReply[0] != 4)
                &&  (fcolReply[0] != 0)
                &&  (fcolReply[0] != 6)
                ||  (c4Retry == 3))
                {
                    if (bThrowErrs)
                        ThrowLeviErr(fcolReply[0]);

                    LogMsg
                    (
                        L"Got Levi error waiting for ack. Err=%(1)"
                        , tCQCKit::EVerboseLvls::Medium
                        , CID_FILE
                        , CID_LINE
                        , TCardinal(fcolReply[0])
                    );
                    break;
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        if (bThrowErrs)
            throw;
        return kCIDLib::False;
    }

    return bRes;
}


//
//  A convenience method that just turns around and calls the more generic
//  bWaitMsg() method below, passing in the correct expected reply and a zero
//  for the expected node, since this is not node specific.
//
tCIDLib::TBoolean TZWaveLevi2SDriver::bWaitTransAck(const tCIDLib::TBoolean bThrowErrs)
{
    return bWaitMsg
    (
        tZWaveLevi2S::ERepType_TransAck, 0, 256, bThrowErrs
    );
}


//
//  Wait for a response for a specific node/inst id, while processing other messages,
//  unless that is disabled. We have one that just eats the returned message, for
//  when it isn't important.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bWaitMsg(const  tZWaveLevi2S::ERepTypes eWaitFor
                            , const tCIDLib::TCard4         c4ForNode
                            , const tCIDLib::TCard4         c4ForInstId
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(kZWaveLevi2S::c4WaitTimeout);

    tCIDLib::TCardList fcolVals(16);
    tZWaveLevi2S::ERepTypes eGot;
    return bWaitMsg
    (
        eWaitFor, enctEnd, c4ForNode, c4ForInstId, 0, 0, eGot, fcolVals, bThrowErrs
    );
}


tCIDLib::TBoolean
TZWaveLevi2SDriver::bWaitMsg(const  tZWaveLevi2S::ERepTypes eWaitFor
                            , const tCIDLib::TCard4         c4ForNode
                            , const tCIDLib::TCard4         c4ForInstId
                            ,       tCIDLib::TCardList&     fcolVals
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(kZWaveLevi2S::c4WaitTimeout);

    tZWaveLevi2S::ERepTypes eGot;
    return bWaitMsg
    (
        eWaitFor, enctEnd, c4ForNode, c4ForInstId, 0, 0, eGot, fcolVals, bThrowErrs
    );
}


//
//  Then fundamental msg waiting methods. There's one that takes millis and just
//  calls the other with an end time. This guy can wait for very specific msgs,
//  while processing any that don't match the criteria. If any of the For parameters
//  are zero, then we take anything for that value, else it has to match.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bWaitMsg(const  tZWaveLevi2S::ERepTypes     eWaitFor
                            , const tCIDLib::TEncodedTime       enctEnd
                            , const tCIDLib::TCard4             c4ForNode
                            , const tCIDLib::TCard4             c4ForInstId
                            , const tCIDLib::TCard4             c4ForClass
                            , const tCIDLib::TCard4             c4ForCmd
                            ,       tZWaveLevi2S::ERepTypes&    eGot
                            ,       tCIDLib::TCardList&         fcolVals
                            , const tCIDLib::TBoolean           bThrowErrs)
{
    //
    //  If there is a class or command class to listen for, then we have to be
    //  waiting for a general report or basic report.
    //
    if ((c4ForClass || c4ForCmd)
    &&  (eWaitFor != tZWaveLevi2S::ERepType_BasicReport)
    &&  (eWaitFor != tZWaveLevi2S::ERepType_GenReport))
    {
        facZWaveLevi2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcComm_BadWaitCrit
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }

    tCIDLib::TBoolean   bRes = kCIDLib::False;
    tCIDLib::TCard4     c4InstId;
    tCIDLib::TCard4     c4NodeId;

    eGot = tZWaveLevi2S::ERepType_None;
    while(kCIDLib::True)
    {
        // Tell it to throw errors
        eGot = eGetMsg(enctEnd, c4NodeId, c4InstId, fcolVals, bThrowErrs);

        // It waits for up to the end time, so if none, we have to have timed out
        if (eGot == tZWaveLevi2S::ERepType_None)
            break;

        // If an error, deal with that
        if ((eGot == tZWaveLevi2S::ERepType_Error)
        ||  (eGot == tZWaveLevi2S::ERepType_TransNak))
        {
            if (bThrowErrs)
                ThrowLeviErr(fcolVals[0]);

            LogMsg
            (
                L"Got Levi error waiting for msg. Node=%(1), Err=%(2)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , TCardinal(c4ForNode)
                , TCardinal(fcolVals[0])
            );

            return kCIDLib::False;
        }

        //
        //  Not an error, so see if it's what we are waiting for. If the wait
        //  for node is zero, take any value. If the instance is 256 take any
        //  value (0 represents a single class instance.)
        //
        if ((eGot == eWaitFor)
        &&  ((c4NodeId == c4ForNode) || !c4ForNode)
        &&  ((c4InstId == c4ForInstId) || (c4ForInstId == 256)))
        {
            if (c4ForClass || c4ForCmd)
            {
                // We have to have at least 2 bytes
                if ((fcolVals.c4ElemCount() > 1)
                &&  (!c4ForClass || (fcolVals[0] == c4ForClass))
                &&  (!c4ForCmd || (fcolVals[1] == c4ForCmd)))
                {
                    bRes = kCIDLib::True;
                    break;
                }
            }
             else
            {
                bRes = kCIDLib::True;
                break;
            }
        }

        //
        //  Not something we are waiting for, so process if async processing is not
        //  disabled.
        //
        if (!m_bNoProcess)
            ProcessMsg(eGot, c4NodeId, c4InstId, fcolVals);
    }

    return bRes;
}


tCIDLib::TBoolean
TZWaveLevi2SDriver::bWaitMsgMS( const   tZWaveLevi2S::ERepTypes     eWaitFor
                                , const tCIDLib::TCard4             c4WaitMS
                                , const tCIDLib::TCard4             c4ForNode
                                , const tCIDLib::TCard4             c4ForInstId
                                , const tCIDLib::TCard4             c4ForClass
                                , const tCIDLib::TCard4             c4ForCmd
                                ,       tZWaveLevi2S::ERepTypes&    eGot
                                ,       tCIDLib::TCardList&         fcolVals
                                , const tCIDLib::TBoolean           bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4WaitMS);
    return bWaitMsg
    (
        eWaitFor
        , enctEnd
        , c4ForNode
        , c4ForInstId
        , c4ForClass
        , c4ForCmd
        , eGot
        , fcolVals
        , bThrowErrs
    );
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bWaitMsgMS( const  tZWaveLevi2S::ERepTypes  eWaitFor
                                , const tCIDLib::TCard4         c4Millis
                                , const tCIDLib::TCard4         c4ForNode
                                , const tCIDLib::TCard4         c4ForInstId
                                , const tCIDLib::TBoolean       bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4Millis);

    tCIDLib::TCardList fcolVals(16);
    tZWaveLevi2S::ERepTypes eGot;
    return bWaitMsg
    (
        eWaitFor, enctEnd, c4ForNode, c4ForInstId, 0, 0, eGot, fcolVals, bThrowErrs
    );
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
//  We can optionally throw timeout or device errors, or just return them as msg
//  types.
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
//  We convert basic reports into general reports, so that, beyond here, we don't
//  have to make any distinction. And we will convert secure general reports to
//  regular ones, and just return a secure indicator. So, beyond this method, everyone
//  just deals with regular general reports.
//
tZWaveLevi2S::ERepTypes
TZWaveLevi2SDriver::eGetLine(       TString&                strToFill
                            , const tCIDLib::TEncodedTime   enctEnd
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    tZWaveLevi2S::ERepTypes eRes = tZWaveLevi2S::ERepType_None;

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
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
        return eRes;
    }

    // Strip any leading/trailing whitespace from it
    m_strTmpIn.StripWhitespace();

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
        return tZWaveLevi2S::ERepType_None;
    }

    // Get the command char out and copy the rest to the caller's buffer
    const tCIDLib::TCh chCmd = m_strTmpIn[1];
    m_strTmpIn.CopyOutSubStr(strToFill, 2);
    c4Len -= 2;

    // The first character tells us what it basically is
    switch(chCmd)
    {
        case kCIDLib::chExclamation :
            eRes = tZWaveLevi2S::ERepType_LearnMode;
            break;

        case kCIDLib::chLatin_E :
            eRes = tZWaveLevi2S::ERepType_MsgAck;
            break;

        case kCIDLib::chLatin_F :
            eRes = tZWaveLevi2S::ERepType_UnitID;
            break;

        case kCIDLib::chLatin_N :
        case kCIDLib::chLatin_n :
            // It has to be at least 7 characters left
            if (bCheckMsgLen(L"Node", c4Len, 7))
            {
                switch(strToFill[3])
                {
                    case kCIDLib::chLatin_L :
                        eRes = tZWaveLevi2S::ERepType_BasicReport;
                        break;

                    case kCIDLib::chLatin_S :
                        eRes = tZWaveLevi2S::ERepType_SceneReport;
                        break;

                    case kCIDLib::chQuotation :
                        eRes = tZWaveLevi2S::ERepType_UnitName;
                        break;

                    case kCIDLib::chColon :
                        // Either a secure status report or a regular one
                        if (chCmd == kCIDLib::chLatin_n)
                            eRes = tZWaveLevi2S::ERepType_SecStatus;
                        else
                            eRes = tZWaveLevi2S::ERepType_GenReport;
                        break;

                    default :
                        // Don't support this one
                        break;
                };
            }
            break;

        case kCIDLib::chLatin_X :
            // Let eGetMsg handle processing the code, say ack for now
            eRes = tZWaveLevi2S::ERepType_TransAck;
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
    if (eRes == tZWaveLevi2S::ERepType_None)
    {
        if (bThrowErrs)
        {
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
    }
    return eRes;
}


tZWaveLevi2S::ERepTypes
TZWaveLevi2SDriver::eGetMsg(const   tCIDLib::TEncodedTime   enctEnd
                            ,       tCIDLib::TCard4&        c4NodeId
                            ,       tCIDLib::TCard4&        c4InstId
                            ,       tCIDLib::TCardList&     fcolVals
                            , const tCIDLib::TBoolean       bThrowErrs)
{
    // Do an initial call to get the first (and probably only) line
    m_strTmpRep1.Clear();
    tZWaveLevi2S::ERepTypes eRes = eGetLine(m_strTmpRep1, enctEnd, bThrowErrs);

    //
    //  If not a unit oriented msg, then we want them to come back zero. If we don't
    //  get an explicit inst id, we return 0, to indicate it's just the base instance.
    //
    c4NodeId = 0;
    c4InstId = 0;

    // Empty the output values list so we can just add to it
    fcolVals.RemoveAll();

    //
    //  If successful, check to see if there is a continuation indicator. If so,
    //  then let's see if we can get it. We look until no more continuation or we
    //  run out of time, which will get us a timeout.
    //
    tCIDLib::TEncodedTime enctCont = enctEnd + (kCIDLib::enctOneMilliSec * 500);
    while ((eRes != tZWaveLevi2S::ERepType_None)
    &&     (m_strTmpRep1.chLast() == kCIDLib::chBackSlash))
    {
        // Delete the continuation char and try to get another line
        m_strTmpRep1.DeleteLast();
        const tZWaveLevi2S::ERepTypes eRes2 = eGetLine
        (
            m_strTmpRep2, enctCont, bThrowErrs
        );

        //
        //  We have to have gotten some response, it has to have a colon, and it
        //  has to match the previous message up to the colon, and it has to be the
        //  same type of line as the last. If not, then something is wrong.
        //
        tCIDLib::TCard4 c4ColonOfs = 0;
        if ((eRes2 != eRes)
        ||  !m_strTmpRep2.bFirstOccurrence(kCIDLib::chColon, c4ColonOfs, kCIDLib::False)
        ||  !m_strTmpRep2.bCompareNI(m_strTmpRep1, c4ColonOfs))
        {
            // Something isn't right
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcComm_BadMsgCont
                , m_strTmpRep3
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Strip out the prefix and add this to the previous contents, adding a comma
        m_strTmpRep2.Cut(0, c4ColonOfs + 1);
        m_strTmpRep1.Append(kCIDLib::chComma);
        m_strTmpRep1.Append(m_strTmpRep2);
    }

    // If tracking is enabled, then write out the raw msg
    if (m_bEnableTrace && !m_strTmpRep1.bIsEmpty())
    {
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        m_strmTrace << L"New ZWMsg  " << tmNow << L"\n"
                    << L"  Raw Text=" << m_strTmpRep1 << kCIDLib::EndLn;
    }

    //
    //  Based on the type, break out the information. We get out the target node if
    //  that is relevant, and break out any values if that is.
    //
    tCIDLib::TBoolean bRes = kCIDLib::True;
    switch(eRes)
    {
        case tZWaveLevi2S::ERepType_BasicReport :
        {
            // We should get data in the form iLv, where i is the id and v is the value
            tCIDLib::TCard4 c4Val;
            if ((m_strTmpRep1.c4Length() > 7)
            ||  !m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chLatin_L)
            ||  !m_strTmpRep1.bToCard4(c4NodeId, tCIDLib::ERadices::Dec)
            ||  !m_strTmpRep2.bToCard4(c4Val, tCIDLib::ERadices::Dec))
            {
                if (m_bEnableTrace)
                    m_strmTrace << L"  Badly formed basic report received" << kCIDLib::EndLn;
                bRes = kCIDLib::False;
            }

            if (m_bEnableTrace)
            {
                m_strmTrace << L"  Type=Basic Report,  Node=" << c4NodeId
                            << L",  Value=" << c4Val << kCIDLib::EndLn;
            }

            //
            //  Convert it to a general report, so we don't have to deal with the
            //  differences.
            //
            fcolVals.c4AddElement(COMMAND_CLASS_BASIC);
            fcolVals.c4AddElement(BASIC_REPORT);
            fcolVals.c4AddElement(c4Val);
            eRes = tZWaveLevi2S::ERepType_GenReport;
            break;
        }

        //
        //  Process these the same. The secure message will be a general report that
        //  is sent securely. It requires some special adjustment, which this method
        //  will handle. It will also handle de-cap'ing multi-channel msgs. If not
        //  a multi, then the inst id will be set to 0.
        //
        case tZWaveLevi2S::ERepType_GenReport :
        case tZWaveLevi2S::ERepType_SecStatus :
        {
            const tCIDLib::TBoolean bSecure(eRes == tZWaveLevi2S::ERepType_SecStatus);
            bRes = bParseGenReport
            (
                bSecure, m_strTmpRep1, c4NodeId, c4InstId, fcolVals
            );

            if (m_bEnableTrace)
            {
                const tCIDLib::TCard4 c4Cnt = fcolVals.c4ElemCount();

                m_strmTrace << L"  Type=Gen Report,  Node=" << c4NodeId
                            << L",  InstId=" << c4InstId
                            << L",  Secure=" << facCQCKit().strBoolYesNo(bSecure)
                            << L",  Bytes=" << c4Cnt
                            << L",  CmdClass=" << fcolVals[0] << L"/0x"
                            << TCardinal(fcolVals[0], tCIDLib::ERadices::Hex);
                if (c4Cnt)
                {
                    m_strmTrace << L",  Cmd=" << fcolVals[1] << L"/0x"
                                << TCardinal(fcolVals[1], tCIDLib::ERadices::Hex);
                }
                m_strmTrace << kCIDLib::EndLn;
            }

            //
            //  NOTE: We always indicate a general report. Secure only means it was
            //  a general report sent securely. Now that it's parsed, the rest of the
            //  code doesn't care how it was sent, and this avoids them having to worry
            //  about which msg type to wait for.
            //
            eRes = tZWaveLevi2S::ERepType_GenReport;
            break;
        }

        case tZWaveLevi2S::ERepType_MsgAck :
        {
            // Get the code. If zero, return an ack, else throw an error
            tCIDLib::TCard4 c4Code;
            if (!m_strTmpRep1.bToCard4(c4Code, tCIDLib::ERadices::Dec))
                c4Code = 999;

            // If an error code, change it to an error return
            if (c4Code)
            {
                eRes = tZWaveLevi2S::ERepType_Error;
                if (m_bEnableTrace)
                    m_strmTrace << L"  Type=MsgNak,  Error=" << c4Code << kCIDLib::EndLn;

                if (bThrowErrs)
                {
                    facZWaveLevi2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW2Errs::errcComm_MsgNak
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::XmitFailed
                        , TCardinal(c4Code)
                    );
                }
            }
             else
            {
                if (m_bEnableTrace)
                    m_strmTrace << L"  Type=MsgAck" << kCIDLib::EndLn;
            }

            fcolVals.c4AddElement(c4Code);
            break;
        }

        case tZWaveLevi2S::ERepType_SceneReport :
        {
            if (!m_strTmpRep1.bSplit(m_strTmpRep2, kCIDLib::chLatin_S)
            ||  !m_strTmpRep1.bToCard4(c4NodeId, tCIDLib::ERadices::Dec))
            {
                if (m_bEnableTrace)
                    m_strmTrace << L"  Badly formed scene report received" << kCIDLib::EndLn;
                bRes = kCIDLib::False;
            }
             else
            {
                // The rest should break into three integer values
                tCIDLib::TCard4 c4SceneNum;
                tCIDLib::TCard4 c4Level;
                tCIDLib::TCard4 c4Fade;

                if (!m_strTmpRep2.bSplit(m_strTmpRep3, kCIDLib::chComma)
                ||  !m_strTmpRep3.bSplit(m_strTmpRep1, kCIDLib::chComma)
                ||  !m_strTmpRep2.bToCard4(c4SceneNum, tCIDLib::ERadices::Dec)
                ||  !m_strTmpRep3.bToCard4(c4Level, tCIDLib::ERadices::Dec)
                ||  !m_strTmpRep1.bToCard4(c4Fade, tCIDLib::ERadices::Dec))
                {
                    if (m_bEnableTrace)
                        m_strmTrace << L"  Invalid scene report values" << kCIDLib::EndLn;
                    bRes = kCIDLib::False;
                }
                 else
                {
                    if (m_bEnableTrace)
                    {
                        m_strmTrace << L"  Type=Scene,  Scene#" << c4SceneNum
                                    << L",  Level=" << c4Level
                                    << L",  Fade=" << c4Fade
                                    << kCIDLib::EndLn;
                    }
                }

                if (bRes)
                {
                    //
                    //  Convert it into a general report so we don't have to special
                    //  case these things.
                    //
                    fcolVals.c4AddElement(COMMAND_CLASS_SCENE_ACTUATOR_CONF);
                    fcolVals.c4AddElement(SCENE_ACTUATOR_CONF_REPORT);
                    fcolVals.c4AddElement(c4SceneNum);
                    fcolVals.c4AddElement(c4Level);
                    fcolVals.c4AddElement(c4Fade);
                    eRes = tZWaveLevi2S::ERepType_GenReport;
                }
            }
            break;
        }

        case tZWaveLevi2S::ERepType_TransAck :
        {
            //
            //  We should have gotten a code. If zero, it's an ack. If not,
            //  then it's a transmission error.
            //
            tCIDLib::TCard4 c4Code;
            if (!m_strTmpRep1.bToCard4(c4Code, tCIDLib::ERadices::Dec) || c4Code)
            {
                if (m_bEnableTrace)
                    m_strmTrace << L"  Type=TransNak,  Error=" << c4Code << kCIDLib::EndLn;

                if (bThrowErrs)
                {
                    facZWaveLevi2S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW2Errs::errcComm_TransNak
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::XmitFailed
                        , TCardinal(c4Code)
                    );
                }
                eRes = tZWaveLevi2S::ERepType_TransNak;
            }
             else
            {
                if (m_bEnableTrace)
                    m_strmTrace << L"  Type=TransAck" << kCIDLib::EndLn;
                eRes = tZWaveLevi2S::ERepType_TransAck;
            }

            fcolVals.c4AddElement(c4Code);
            break;
        }

        case tZWaveLevi2S::ERepType_UnitID :
        {
            //
            //  It's just the unit id. Return it as the node id and as a single
            //  msg value as well.
            //
            bRes = m_strTmpRep1.bToCard4(c4NodeId, tCIDLib::ERadices::Dec);
            if (!bRes)
                c4NodeId = 999;
            fcolVals.c4AddElement(c4NodeId);

            if (m_bEnableTrace)
                m_strmTrace << L"  Type=NodeId  Id=" << c4NodeId << kCIDLib::EndLn;
            break;
        }

        case tZWaveLevi2S::ERepType_UnitName :
        {
            // The string itself is the value in this case
            break;
        }

        case tZWaveLevi2S::ERepType_LearnMode :
        default :
            // Nothing else involved in these
            break;
    };

    return eRes;
}


tZWaveLevi2S::ERepTypes
TZWaveLevi2SDriver::eGetMsgMSs( const   tCIDLib::TCard4     c4WaitMS
                                ,       tCIDLib::TCard4&    c4NodeId
                                ,       tCIDLib::TCard4&    c4InstId
                                ,       tCIDLib::TCardList& fcolVals
                                , const tCIDLib::TBoolean   bThrowErrs)
{
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4WaitMS);
    return eGetMsg(enctEnd, c4NodeId, c4InstId, fcolVals, bThrowErrs);
}



//
//  Just a wrapper around the msg waiter for when we just want to process any msgs
//  that might come in, for some period of time.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::EatMsgs(const   tCIDLib::TCard4     c4WaitMS
                            , const tCIDLib::TBoolean   bThrowErrs)
{
    while (bWaitMsgMS(tZWaveLevi2S::ERepTypes_Count, 10, 0, 256, bThrowErrs))
    {
    }
}


//
//  If we get a device error status, this can be called to log it if we are
//  above a minimum verbosity.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::LogLeviErr(  const  tCIDLib::TCard4         c4ErrCode
                                , const tCQCKit::EVerboseLvls   eMinLevel)
{
    const tCIDLib::TCh* pszErr = pszXlatLeviErr(c4ErrCode);
    if (eVerboseLevel() > eMinLevel)
    {
        facZWaveLevi2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcComm_DevError
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Nak
            , TCardinal(c4ErrCode)
            , TString(pszErr)
        );
    }
}


// Translate a Leviton error to a string
const tCIDLib::TCh*
TZWaveLevi2SDriver::pszXlatLeviErr(const tCIDLib::TCard4 c4ResCode) const
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
TZWaveLevi2SDriver::ProcessMsg( const   tZWaveLevi2S::ERepTypes eType
                                , const tCIDLib::TCard4         c4NodeId
                                , const tCIDLib::TCard4         c4InstId
                                ,       tCIDLib::TCardList&     fcolVals)
{
    switch (eType)
    {
        //
        //  NOTE: By now all secure reports and all basic and scene reports
        //  re converted to general reports!
        //
        case tZWaveLevi2S::ERepType_GenReport :
            //
            //  It's some sort of general report from a unit, so we need to pull out
            //  the unit id and pass along the data to it to be processed.
            //
            ProcessGenReport(eType, c4NodeId, c4InstId, fcolVals);
            break;

        default :
            // Nothing we need or want to process
            break;
    };
}


//
//  If we get a general unit reort, this is called to parse out the info and look
//  up the target unit in our unit configuration and pass the info on to him in order
//  to be stored if needed. Only it will know how to deal with the data.
//
//  The form is nodeid:cmdclass,cmd[,val1, val2,...]
//
//  The basic msg getter has broken those values out. If this was a secure message,
//  then it also has removed the first byte from the values, since we don't need it.
//  And the type was changed to general report, since it was just a security sent
//  general report and the rest of the code doesn't care it was securely sent.
//
//  If it was an encapsulated msg, it broke that out and passse us the instance id.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::ProcessGenReport(const  tZWaveLevi2S::ERepTypes eType
                                    , const tCIDLib::TCard4         c4NodeId
                                    , const tCIDLib::TCard4         c4InstId
                                    ,       tCIDLib::TCardList&     fcolVals)
{
    //
    //  See if we have such a unit in our list. If not, then just skip it. If so,
    //  then passt he command on to the target unit.
    //
    TZWaveUnit* punitTar = m_dcfgCurrent.punitFindById(c4NodeId);
    if (punitTar && punitTar->bIsViable())
    {
        const tCIDLib::TBoolean bRes = punitTar->bHandleGenReport(c4InstId, fcolVals, *this);
    }
}


//
//  Given the data that makes upa message, we format it out and send it.
//
//  Before we send, we process any waiting messages. This deals with the issue where
//  some acks from previous things timed out and are still there. This way we'll chew
//  them up before we send another message and hopefully we'll only see acks
//  legitimately for this message, though there's always a chance that a bogus one is
//  on the way now.
//
//  We also deal with secure vs. non-secure messages and with multi-channel
//  encapsulation for instance ids above 1.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::SendMsg(const   TZWaveUnit&         unitTar
                            , const TZWCmdClass&        zwccSrc
                            , const tCIDLib::TCardList& fcolVals)
{
    // Call the other version and pass along the unit/class info
    SendMsg
    (
        unitTar.c4Id()
        , zwccSrc.c4InstId()
        , zwccSrc.bSecure()
        , zwccSrc.eMultiType()
        , fcolVals
    );
}

tCIDLib::TVoid
TZWaveLevi2SDriver::SendMsg(const   tCIDLib::TCard4             c4UnitId
                            , const tCIDLib::TCard4             c4InstId
                            , const tCIDLib::TBoolean           bSecure
                            , const tZWaveLevi2Sh::EMultiTypes  eMultiType
                            , const tCIDLib::TCardList&         fcolVals)
{
    // Just process any msgs that might be waiting
    EatMsgs(10, kCIDLib::False);

    // Build up the actual message, starting with the opening bits and node id
    m_strTmpOut = L">N";
    m_strTmpOut.AppendFormatted(c4UnitId);

    if (bSecure)
        m_strTmpOut.Append(L"SS");
    else
        m_strTmpOut.Append(L"SE");

    // If the instance id isn't 0, then we have to encapsulate it
    if (c4InstId > 0)
    {
        if (eMultiType == tZWaveLevi2Sh::EMultiTypes::Instance)
        {
            m_strTmpOut.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_MULTI_INSTANCE));
            m_strTmpOut.Append(kCIDLib::chComma);
            m_strTmpOut.AppendFormatted(tCIDLib::TCard4(MULTI_INSTANCE_CMD_ENCAP_V2));
            m_strTmpOut.Append(kCIDLib::chComma);
            m_strTmpOut.AppendFormatted(c4InstId);
            m_strTmpOut.Append(kCIDLib::chComma);
        }
         else if (eMultiType == tZWaveLevi2Sh::EMultiTypes::Channel)
        {
            m_strTmpOut.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_MULTI_INSTANCE));
            m_strTmpOut.Append(kCIDLib::chComma);
            m_strTmpOut.AppendFormatted(tCIDLib::TCard4(MULTI_CHANNEL_CMD_ENCAP_V2));
            m_strTmpOut.Append(kCIDLib::chComma);
            m_strTmpOut.AppendFormatted(1UL);
            m_strTmpOut.Append(kCIDLib::chComma);
            m_strTmpOut.AppendFormatted(c4InstId);
            m_strTmpOut.Append(kCIDLib::chComma);
        }
         else
        {
            CIDAssert2(L"Unknown multi-instance type");
        }
    }

    // And do the rest of the values
    const tCIDLib::TCard4 c4Count = fcolVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            m_strTmpOut.Append(kCIDLib::chComma);
        m_strTmpOut.AppendFormatted(fcolVals[c4Index]);
    }

    // If message tracing is enabled
    if (m_bEnableTrace)
    {
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        m_strmTrace << L"\nSending Msg --------- " << tmNow << L"\n"
                    << L"  Raw Msg=" << m_strTmpOut
                    << L",  TargetId=" << c4UnitId
                    << L",  CmdClass=" << fcolVals[0] << L"/0x"
                    << TCardinal(fcolVals[0], tCIDLib::ERadices::Hex)
                    << L",  Cmd=" << fcolVals[1] << L"/0x"
                    << TCardinal(fcolVals[1], tCIDLib::ERadices::Hex);
        if (c4InstId)
        {
            m_strmTrace << L",  Multi=" << facCQCKit().strBoolYesNo(c4InstId > 0)
                        << L",  InstId=" << c4InstId;
        }
        m_strmTrace << kCIDLib::EndLn;
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
        if (m_pcommZWave->c4WriteMBufMS(m_mbufOut, c4Len, 1000) != c4Len)
        {
            // We didn't send all the bytes so throw
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcComm_UnitSendFailed
                , m_strTmpOut
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::XmitFailed
                , TCardinal(c4UnitId)
            );
        }
        m_enctNextTrans = TTime::enctNowPlusMSs(kZWaveLevi2S::c4MinSendInterval);
    }

    catch(TError& errToCatch)
    {
        m_enctNextTrans = TTime::enctNowPlusMSs(kZWaveLevi2S::c4MinSendInterval);

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This is a convenience for code that wants to send standard command class msgs
//  to units. We call SendMsg() to send the incoming data, then we wait for the ack
//  and the transmission ack.
//
//  We have another for those cases where just teh command class and command are
//  required, which is not uncommon. We just set up the values for the caller.
//
//  We cheat a little, if we get the ack, but we are out of time after that. We will
//  extend the end time more to allow for the trans ack.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::SendUnitCmd(const   TZWaveUnit&             unitTar
                                , const TZWCmdClass&            zwccSrc
                                , const tCIDLib::TCardList&     fcolVals)
{
    bSendWaitAck(unitTar, zwccSrc, fcolVals, kCIDLib::True);
    bWaitTransAck();
}

tCIDLib::TVoid
TZWaveLevi2SDriver::SendUnitCmd(const   TZWaveUnit&         unitTar
                                , const TZWCmdClass&        zwccSrc
                                , const tCIDLib::TCard4     c4CmdClass
                                , const tCIDLib::TCard4     c4Cmd)
{
    tCIDLib::TCardList fcolVals(2);
    fcolVals.c4AddElement(c4CmdClass);
    fcolVals.c4AddElement(c4Cmd);
    bSendWaitAck(unitTar, zwccSrc, fcolVals, kCIDLib::True);
    bWaitTransAck();
}



//
//  A special case helper for sending msgs to the VRCOP. These aren't the same as the
//  unit oriented msgs. We just take a string which we set up and send, then we do
//  a wait for the ack and transmit ack, where appropriate.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::SendVRCOPMsg(const  TString&            strMsg
                                , const tCIDLib::TBoolean   bTransAck)
{
    // Just process any msgs that might be waiting
    EatMsgs(10, kCIDLib::False);

    if (m_bEnableTrace)
    {
        TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
        m_strmTrace << L"\nSending Raw VRCOP Msg ----- " << tmNow << L"\n"
                    << L"  Text=" << strMsg << kCIDLib::EndLn;
    }

    // Transcode it to the output memory buffer and get the byte count
    tCIDLib::TCard4 c4Len;
    m_tcvtOut.c4ConvertTo(strMsg, m_mbufOut, c4Len);

    // Put the CR on
    m_mbufOut.PutCard1(0xD, c4Len++);

    if (m_pcommZWave->c4WriteMBufMS(m_mbufOut, c4Len, 1000) != c4Len)
    {
        // We didn't send all the bytes so throw
        facZWaveLevi2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcComm_SendFailed
            , strMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::XmitFailed
        );
    }

    bWaitMsg(tZWaveLevi2S::ERepType_MsgAck, 0, 256, kCIDLib::True);

    // If asked, wait for a transmit ack
    if (bTransAck)
        bWaitMsg(tZWaveLevi2S::ERepType_TransAck, 0, 256, kCIDLib::True);
}



//
//  If we get a device error status, this can be called to throw it as
//  an exception.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::ThrowLeviErr(const tCIDLib::TCard4 c4ErrCode)
{
    const tCIDLib::TCh* pszErr = pszXlatLeviErr(c4ErrCode);

    facZWaveLevi2S().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kZW2Errs::errcComm_DevError
        , TString(pszErr)
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Nak
        , TCardinal(c4ErrCode)
    );
}


