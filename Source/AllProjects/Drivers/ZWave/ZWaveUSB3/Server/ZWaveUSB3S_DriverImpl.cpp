//
// FILE NAME: ZWaveUSB3S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2017
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
//  This file provides the primary implementation code of the driver. It's a lot
//  of code, so it's broken into a few different files. This one provides all of
//  the actual driver stuff plus some basic utility stuff.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveUSB3SDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveUSB3SDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveUSB3SDriver::TZWaveUSB3SDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c1GotSecSchemeGet(0)
    , m_c1NextSeq(tCIDLib::TCard1(TTime::enctNow() >> 16))
    , m_c4DrvVerMaj(0)
    , m_c4DrvVerMin(0)
    , m_c4FldId_InvokeCmd(kCIDLib::c4MaxCard)
    , m_c4FldId_NewConfig(kCIDLib::c4MaxCard)
    , m_c4FldId_OutQPercent(kCIDLib::c4MaxCard)
    , m_c4FldId_PendingQPercent(kCIDLib::c4MaxCard)
    , m_c4FldId_State(kCIDLib::c4MaxCard)
    , m_c4NextProcUnit(0)
    , m_eRepState(tZWaveUSB3Sh::EDrvStates::RepSucceeded)
    , m_eZWDrvState(tZWaveUSB3Sh::EDrvStates::Startup)
    , m_thrRepScan
      (
          TString(L"ZWRepThread") + cqcdcToLoad.strMoniker()
          , TMemberFunc<TZWaveUSB3SDriver>(this, &TZWaveUSB3SDriver::eRepScanThread)
      )
    , m_zstCtrl(cqcdcToLoad.strMoniker())
{
}

TZWaveUSB3SDriver::~TZWaveUSB3SDriver()
{
    // Clean up the z-stick if not already
    m_zstCtrl.Close();
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Lets the unit classes check that any unit name they might receive via the Z-Wave
//  node naming command class is unique. Else he has to ignore it. We check case
//  insensitively, since we don't want any dups even if different case.
//
tCIDLib::TBoolean TZWaveUSB3SDriver::bUSB3IsUniqueUnitName(const TString& strToCheck) const
{
    return !m_dcfgCurrent.bUnitExists(strToCheck);
}


// Lets the unit classes get the names of their fields if neede
tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3QueryFldName(const tCIDLib::TCard4 c4FldId, TString& strToFill)
{
    return bQueryFldName(c4FldId, strToFill);
}



//
//  Waits for a specific msg to come in. This should always be used to wait for a reply,
//  since it will do the appropriate dispatching of async msgs while it waits. It can wait
//  for a particular msg id from a particular node id. If the msg  id is send data or
//  sendmultidata it will also check the command class and cmd ids.
//
//  Note that callback id is not used here. It's not really possible since the callback
//  id could change if the msg has to be retried under some circumstances. The 'send and
//  wait' type msgs use an 'ack id' as well, but it's not used here either. We are just
//  waiting for specific msgs.
//
//  Any of the values can be zero in which case they will always match. However if they
//  are all zero, we take that to mean we should just pump messages until the indicated
//  time expires. eType being the _Unknown value counts as a zero value.
//
//  We can wait for request or response msgs.
//
//  We have another version that takes a unit object instead of a raw unit id. It can do
//  a bit of useful work on behalf of the caller, then just calls the raw id version.
//
tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WaitForMsg( const   TZWaveUnit&             unitTar
                                    , const tZWaveUSB3Sh::EMsgTypes eType
                                    , const tCIDLib::TCard1         c1MsgId
                                    , const tCIDLib::TCard1         c1ClassId
                                    , const tCIDLib::TCard1         c1CmdId
                                    , const tCIDLib::TEncodedTime   enctWaitFor
                                    ,       TZWInMsg&               zwimReply
                                    , const tCIDLib::TBoolean       bThrowIfNot)
{
    //
    //  Call the other version, passing on the unit id, and adding another couple seconds
    //  to the wait time if the target unit is a frequent listener type.
    //
    return bUSB3WaitForMsg
    (
        eType
        , unitTar.c1Id()
        , c1MsgId
        , c1ClassId
        , c1CmdId
        , enctWaitFor + (unitTar.bFreqListener() ? kCIDLib::enctTwoSeconds : 0)
        , zwimReply
        , bThrowIfNot
    );
}

tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WaitForMsg( const   tZWaveUSB3Sh::EMsgTypes eType
                                    , const tCIDLib::TCard1         c1UnitId
                                    , const tCIDLib::TCard1         c1MsgId
                                    , const tCIDLib::TCard1         c1ClassId
                                    , const tCIDLib::TCard1         c1CmdId
                                    , const tCIDLib::TEncodedTime   enctWaitFor
                                    ,       TZWInMsg&               zwimReply
                                    , const tCIDLib::TBoolean       bThrowIfNot)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + enctWaitFor;

    // See if this is a just pump msgs wait
    const tCIDLib::TBoolean bNeverMatch
    (
        !c1UnitId
        && !c1MsgId
        && !c1ClassId
        && !c1CmdId
        && (eType == tZWaveUSB3Sh::EMsgTypes::Unknown)
    );

    if (!bNeverMatch && facZWaveUSB3Sh().bMediumTrace())
    {
        TString strWaitMsg(L"Waiting for unit=", 64UL);
        strWaitMsg.AppendFormatted(TCardinal(c1UnitId, tCIDLib::ERadices::Hex));
        strWaitMsg.Append(L", msg=");
        strWaitMsg.AppendFormatted(TCardinal(c1MsgId, tCIDLib::ERadices::Hex));

        tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(c1ClassId);
        if (eClass == tZWaveUSB3Sh::ECClasses::Count)
        {
            strWaitMsg.Append(L", class=");
            strWaitMsg.AppendFormatted(TCardinal(c1ClassId, tCIDLib::ERadices::Hex));
            strWaitMsg.Append(L", ccmd=");
            strWaitMsg.AppendFormatted(TCardinal(c1CmdId, tCIDLib::ERadices::Hex));
        }
         else
        {
            strWaitMsg.Append(L", class=");
            strWaitMsg.Append(tZWaveUSB3Sh::strXlatECClasses(eClass));
            strWaitMsg.Append(L", ccmd=");            ;
            strWaitMsg.Append(facZWaveUSB3Sh().pszXlatCCCmdId(c1ClassId, c1CmdId));
        }
        facZWaveUSB3Sh().LogTraceMsg(strWaitMsg);
    }

    // Wait for a report for the indicated unit to come in
    TThread* pthrCaller = TThread::pthrCaller();
    while (enctCur < enctEnd)
    {
        // Check for a shutdown request. If so, force a timeout and break out
        if (pthrCaller->bCheckShutdownRequest())
        {
            enctCur = enctEnd;
            break;
        }

        // Wait in up to two second chunks, check for shutdown requests
        tCIDLib::TEncodedTime enctCurWait;
        if (enctCur + kCIDLib::enctTwoSeconds > enctEnd)
            enctCurWait = enctEnd - enctCur;
        else
            enctCurWait = kCIDLib::enctTwoSeconds;

        tZWaveUSB3Sh::EMsgTypes eRes = eGetReply(zwimReply, enctCurWait, kCIDLib::False);

        //
        //  Simplify things below, we only process requests or responses. Acks and trans
        //  acks are handled up-stream in the higher level 'send and wait' stuff.
        //
        if ((eRes != tZWaveUSB3Sh::EMsgTypes::Request)
        &&  (eRes != tZWaveUSB3Sh::EMsgTypes::Response))
        {
            enctCur = TTime::enctNow();
            continue;
        }

        // Check the simple stuff for a failure to match
        tCIDLib::TBoolean bMatch = kCIDLib::True;
        if (bNeverMatch
        ||  ((eType != tZWaveUSB3Sh::EMsgTypes::Unknown) && (eRes != eType))
        ||  (c1UnitId && (c1UnitId != zwimReply.c1SrcId())))
        {
            bMatch = kCIDLib::False;
        }

        // If not failed yet, check the msg id
        tCIDLib::TCard1 c1GotMsgId = zwimReply.c1MsgId();
        if (bMatch && c1MsgId)
            bMatch = c1MsgId == c1GotMsgId;

        // Check the command class stuff if not already failed
        if (bMatch && (c1ClassId || c1CmdId))
        {
            if (c1GotMsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
            {
                const tCIDLib::TCard1 c1GotClassId = zwimReply.c1ClassId();
                const tCIDLib::TCard1 c1GotCmdId = zwimReply.c1CmdId();

                if ((c1ClassId && (c1ClassId != c1GotClassId))
                ||  (c1CmdId && (c1CmdId != c1GotCmdId)))
                {
                    bMatch = kCIDLib::False;
                }
            }
             else
            {
                // Can't match since it's not a cmd class msg
                bMatch = kCIDLib::False;
            }
        }

        // Dispatch it if its a CC msg or application update
        if (c1GotMsgId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
            AppCmdHandler(zwimReply);
        else if (c1GotMsgId == FUNC_ID_ZW_APPLICATION_UPDATE)
            AppUpdateHandler(zwimReply);

        // If it's our guy, then break out with success
        if (bMatch)
        {
            if (!bNeverMatch && facZWaveUSB3Sh().bMediumTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Got expected reply");
            break;
        }

        enctCur = TTime::enctNow();
    }

    // If we timed out, then give up
    if (enctCur >= enctEnd)
    {
        if (!bNeverMatch && (facZWaveUSB3Sh().bMediumTrace() || bThrowIfNot))
        {
            TString strWaitMsg(L"Unit=", 64UL);
            strWaitMsg.AppendFormatted(TCardinal(c1UnitId, tCIDLib::ERadices::Hex));
            strWaitMsg.Append(L", msg=");
            strWaitMsg.AppendFormatted(TCardinal(c1MsgId, tCIDLib::ERadices::Hex));

            tZWaveUSB3Sh::ECClasses eClass = tZWaveUSB3Sh::eAltNumECClasses(c1ClassId);
            if (eClass == tZWaveUSB3Sh::ECClasses::Count)
            {
                strWaitMsg.Append(L", class=");
                strWaitMsg.AppendFormatted(TCardinal(c1ClassId, tCIDLib::ERadices::Hex));
                strWaitMsg.Append(L", ccmd=");
                strWaitMsg.AppendFormatted(TCardinal(c1CmdId, tCIDLib::ERadices::Hex));
            }
             else
            {
                strWaitMsg.Append(L", class=");
                strWaitMsg.Append(tZWaveUSB3Sh::strXlatECClasses(eClass));
                strWaitMsg.Append(L", ccmd=");            ;
                strWaitMsg.Append(facZWaveUSB3Sh().pszXlatCCCmdId(c1ClassId, c1CmdId));
            }

            if (bThrowIfNot)
            {
                facZWaveUSB3Sh().LogTraceErr(L"Never got response. %(1)", strWaitMsg);
                facZWaveUSB3S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3Errs::errcProto_Timeout2
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Timeout
                    , strWaitMsg
                );
            }
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  These give the unit classes down in the shared utility access to write to fields
//  in response to received message.
//
tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WriteFld(const  tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bToStore)
{
    return bStoreBoolFld(c4FldId, bToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WriteFld(const  tCIDLib::TCard4 c4FldId
                                , const tCIDLib::TCard4 c4ToStore)
{
    return bStoreCardFld(c4FldId, c4ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WriteFld(const  tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TFloat8    f8ToStore)
{
    return bStoreFloatFld(c4FldId, f8ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WriteFld(const  tCIDLib::TCard4 c4FldId
                                , const tCIDLib::TInt4  i4ToStore)
{
    return bStoreIntFld(c4FldId, i4ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveUSB3SDriver::bUSB3WriteFld(const  tCIDLib::TCard4 c4FldId
                                , const TString&        strToStore)
{
    return bStoreStringFld(c4FldId, strToStore, kCIDLib::True);
}


// A controller interface method to let the units look up their fields
tCIDLib::TCard4 TZWaveUSB3SDriver::c4USB3LookupFldId(const TString& strName) const
{
    return pflddFind(strName, kCIDLib::True)->c4Id();
}


// Make our moniker available to the unit classes
const TString& TZWaveUSB3SDriver::strUSB3Moniker() const
{
    return strMoniker();
}


// A controller interface to let uses send out event triggers
tCIDLib::TVoid
TZWaveUSB3SDriver::USB3QEventTrig(  const   tCQCKit::EStdDrvEvs eEvent
                                    , const TString&            strFld
                                    , const TString&            strVal1
                                    , const TString&            strVal2
                                    , const TString&            strVal3
                                    , const TString&            strVal4)
{
    QueueEventTrig(eEvent, strFld, strVal1, strVal2, strVal3, strVal4);
}


//
//  Just sends the message. Don't call this directly unless you know for sure that
//  you don't need to get anything back. If you do, call one of the ones below or
//  in some special cases you can call this and use bUSB3WaitMsg. Or you can call
//  this if you intend to just want asynchronously for the response and don't want to
//  stop and wait for the acks, i.e. just let the Z-Stick handle that stuff and just
//  wait for the eventually response to be routed back to the unit object.
//
//  This is also called by all of the msg sending helpers, so that we have a single
//  point of transmission and can do any checking or housekeeping needed.
//
tCIDLib::TVoid TZWaveUSB3SDriver::USB3Send(const TZWOutMsg& zwomToSend)
{
    // Make sure it's in a state to be queued
    if (!zwomToSend.bCanQueue())
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcComm_NotReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
    m_zstCtrl.QueueOutMsg(zwomToSend);
}


//
//  Sends a msg that must be a CC msg and get the required reply, if one is expected.
//  Typically for the first version there is one since they provided us with a reply
//  msg.
//
//  We have another version for things like set commands, that don't get back a CC msg
//  in response, just an acknowledgement.
//
tCIDLib::TVoid
TZWaveUSB3SDriver::USB3SendCCMsg(const  TZWOutMsg&      zwomToSend
                                ,       TZWInMsg&       zwimRep
                                , const tCIDLib::TCard4 c4WaitFor)
{
    CIDAssert
    (
        zwomToSend.bIsCCMsg(), L"SendCCMsg called for non-CC msg"
    );

    // Call the other version first to wait for the acks (or failure)
    USB3SendCCMsg(zwomToSend);

    //
    //  And now wait for the CC response from the other side if there is one expected.
    //  Normally they'd call the version below if not, but just in case.
    //
    //  Note also that the reply will be an app command, not the send data we sent. That
    //  should be set on the msg, but we don't even bother to trust that. We just look
    //  for the app command handler reply.
    //
    if (zwomToSend.c1CCReplyClass() && zwomToSend.c1CCReplyCmd())
    {
        tCIDLib::TCard4 c4ExtraMS = 0;
        if (zwomToSend.bFreqListener())
            c4ExtraMS = 1500;
        bUSB3WaitForMsg
        (
            tZWaveUSB3Sh::EMsgTypes::Request
            , zwomToSend.c1TarId(kCIDLib::True)
            , FUNC_ID_APPLICATION_COMMAND_HANDLER
            , zwomToSend.c1CCReplyClass()
            , zwomToSend.c1CCReplyCmd()
            , kCIDLib::enctOneMilliSec * (c4WaitFor + c4ExtraMS)
            , zwimRep
            , kCIDLib::True
        );
    }
}

tCIDLib::TVoid TZWaveUSB3SDriver::USB3SendCCMsg(const TZWOutMsg& zwomToSend)
{
    CIDAssert(zwomToSend.bIsCCMsg(), L"SendCCMsg called for non-CC msg");

    // Do the raw send of the message
    USB3Send(zwomToSend);

    // Wait for our msgs to be sent and acked (or fail)
    if (!m_zstCtrl.bWaitTrans(zwomToSend.c4AckId(), 15000))
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcProto_NoTrans
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(zwomToSend.c4AckId())
        );
    }
}


//
//  Sends a message and waits for a RESPONSE type msg back with the same msg id, but not a
//  node id. This is is assumed to be talking to the controller, not to another node.
//
tCIDLib::TVoid
TZWaveUSB3SDriver::USB3SendCtrlRes( const   TZWOutMsg&      zwomToSend
                                    ,       TZWInMsg&       zwimRep
                                    , const tCIDLib::TCard4 c4WaitFor)
{
    USB3Send(zwomToSend);

    // Wait for our msgs to be sent and acked (or fail)
    if (!m_zstCtrl.bWaitTrans(zwomToSend.c4AckId(), 4000))
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcProto_NoTrans
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(zwomToSend.c4AckId())
        );
    }

    // And now wait for the actual response
    bUSB3WaitForMsg
    (
        tZWaveUSB3Sh::EMsgTypes::Response
        , 0
        , zwomToSend.c1MsgId()
        , 0
        , 0
        , c4WaitFor * kCIDLib::enctOneMilliSec
        , zwimRep
        , kCIDLib::True
    );
}


//
//  Same as above but it is to talk to another unit, not to our controller, so we expect
//  a unit id in the response that matches the one we sent to.
//
tCIDLib::TVoid
TZWaveUSB3SDriver::USB3SendUnitRes( const   TZWOutMsg&      zwomToSend
                                    ,       TZWInMsg&       zwimRep
                                    , const tCIDLib::TCard4 c4WaitFor                                    )
{
    USB3Send(zwomToSend);

    // Wait for our msgs to be sent and acked (or fail)
    if (!m_zstCtrl.bWaitTrans(zwomToSend.c4AckId(), 8000))
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcProto_NoTrans
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(zwomToSend.c4AckId())
        );
    }

    // And wait for the response
    bUSB3WaitForMsg
    (
        tZWaveUSB3Sh::EMsgTypes::Response
        , zwomToSend.c1TarId(kCIDLib::True)
        , zwomToSend.c1MsgId()
        , 0
        , 0
        , kCIDLib::enctOneMilliSec * c4WaitFor
        , zwimRep
        , kCIDLib::True
    );
}


//
//  Send a msg to another node, and wait for a REQUEST type reply, including a
//  potential callback. So this is for something that returns a request but isn't a
//  CC msg.
//
tCIDLib::TVoid
TZWaveUSB3SDriver::USB3SendReq( const   TZWOutMsg&      zwomToSend
                                ,       TZWInMsg&       zwimRep
                                , const tCIDLib::TCard4 c4WaitFor                                )
{
    USB3Send(zwomToSend);

    // Wait for our msgs to be sent and acked (or fail)
    if (!m_zstCtrl.bWaitTrans(zwomToSend.c4AckId(), 8000))
    {
        facZWaveUSB3S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcProto_NoTrans
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(zwomToSend.c4AckId())
        );
    }

    bUSB3WaitForMsg
    (
        tZWaveUSB3Sh::EMsgTypes::Request
        , zwomToSend.c1TarId(kCIDLib::True)
        , zwomToSend.c1MsgId()
        , 0
        , 0
        , kCIDLib::enctOneMilliSec * c4WaitFor
        , zwimRep
        , kCIDLib::True
    );
}


// To let the units set their fields to error state when needed
tCIDLib::TVoid TZWaveUSB3SDriver::USB3SetFieldErr(const tCIDLib::TCard4 c4FldId)
{
    SetFieldErr(c4FldId, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't need to lock here, because no incoming client calls will be
//  seen until we are online, and we aren't yet.
//
tCIDLib::TBoolean TZWaveUSB3SDriver::bGetCommResource(TThread& thrThis)
{
    return m_zstCtrl.bOpenCtrl();
}


//
//
//  See if we can load any config. If there isn't any, that's fine, we just
//  come up with default settings.
//
tCIDLib::TBoolean TZWaveUSB3SDriver::bWaitConfig(TThread&)
{
    // If no file, then just return true now after resetting the config object
    if (!TFileSys::bExists(m_strCfgFile, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        facZWaveUSB3Sh().LogTraceMsg(L"No configuration file found, starting with defaults");
        m_dcfgCurrent.Reset();
        return kCIDLib::True;
    }

    try
    {
        TBinFileInStream strmSrc
        (
            m_strCfgFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        strmSrc >> m_dcfgCurrent;
        facZWaveUSB3Sh().LogTraceMsg(L"Previous configuration found and loaded");
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // It was there but something is awry, return false. Make sure config is defaulted
        facZWaveUSB3Sh().LogTraceMsg(L"Error loading config, starting from scratch");
        m_dcfgCurrent.Reset();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Handle boolean field writes
//
tCQCKit::ECommResults
TZWaveUSB3SDriver::eBoolFldValChanged(  const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    // Reject if we are currently replicating
    CheckReplicating();
    facZWaveUSB3Sh().LogTraceMsg
    (
        L"Got bool field write. Name=%(1) Value=%(2)", strName, TCardinal(bNewValue)
    );

    //
    //  Find the unit that owns this field. If it returns one, then we know that the
    //  unit is in appropriate state else it couldn't have the field ids.
    //
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Unhandled;
    tCIDLib::TCard4 c4CfgInd;
    TZWaveUnit* punitTar = punitFindFldOwner(c4FldId, c4CfgInd);
    if (punitTar)
    {
        try
        {
            punitTar->bBoolFldChanged(strName, c4FldId, bNewValue, eRes);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            IncFailedWriteCounter();
            eRes = tCQCKit::ECommResults::Exception;
        }
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"No unit reported owning this field's id (%(1))", TCardinal(c4FldId)
        );
    }

    if (eRes == tCQCKit::ECommResults::Unhandled)
        IncUnknownWriteCounter();

    return eRes;
}


//
//  Handle card field writes
//
tCQCKit::ECommResults
TZWaveUSB3SDriver::eCardFldValChanged(  const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    // Reject if we are currently replicating
    CheckReplicating();
    facZWaveUSB3Sh().LogTraceMsg
    (
        L"Got card field write. Name=%(1) Value=%(2)", strName, TCardinal(c4NewValue)
    );

    // Find the unit that owns this field
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Unhandled;
    tCIDLib::TCard4 c4CfgInd;
    TZWaveUnit* punitTar = punitFindFldOwner(c4FldId, c4CfgInd);
    if (punitTar)
    {
        try
        {
            punitTar->bCardFldChanged(strName, c4FldId, c4NewValue, eRes);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            IncFailedWriteCounter();
            eRes = tCQCKit::ECommResults::Exception;
        }
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"No unit reported owning this field's id (%(1))", TCardinal(c4FldId)
        );
    }

    if (eRes == tCQCKit::ECommResults::FieldNotFound)
        IncUnknownWriteCounter();

    return eRes;
}



//
//  This guy enumerates the available units, validates this against our
//  unit configuration, registers all the required fields for the configured
//  (and newly discovered) units, and gets initial values for the ones that
//  are currently available.
//
tCQCKit::ECommResults TZWaveUSB3SDriver::eConnectToDevice(TThread& thrThis)
{
    facZWaveUSB3Sh().LogTraceMsg(L"Trying to connect to Z-Wave controller");
    try
    {
        TZWInMsg zwimIn;
        TZWOutMsg zwomOut;

        // Set the timeouts
        zwomOut.Reset(FUNC_ID_SERIAL_API_SET_TIMEOUTS, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomOut += tCIDLib::TCard1(350);
        zwomOut += tCIDLib::TCard1(50);
        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
        USB3SendCtrlRes(zwomOut, zwimIn, 3000);

        //
        //  Try to get basic info from the controller, just so that we know where
        //  we stand and if the controller is there. This will tell us if we are
        //  in the network or not as well.
        //
        m_dcfgCurrent.LoadControllerInfo(*this);

        //
        //  If we are in the network, we have to have a stored security key, so try to
        //  read it in. If that fails, we set it to the default and force ourself out
        //  of the network. If not in the network, make sure it's set to the default.
        //
        THeapBuf mbufKey(16, 16);
        mbufKey.Set(0);
        if (m_dcfgCurrent.bInNetwork())
        {
            //
            //  Load the network key we last stored, if any. If this is the first run
            //  of the program on this machine, it may not be set. If it is set, make
            //  sure it's valid.
            //
            if (!bQueryCfgData(mbufKey))
            {
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcCfg_LostNetworkKey
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::Config
                );
                m_dcfgCurrent.bInNetwork(kCIDLib::False);
            }
             else if (!facZWaveUSB3Sh().bLegalNetworkKey(mbufKey))
            {
                //
                //  Just in case, check that the key is not the default. If it is then
                //  we shouldn't be in the network, so force us to think we are out.
                //
                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3ShErrs::errcCfg_BadNetworkKey
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::NotReady
                );

                m_dcfgCurrent.bInNetwork(kCIDLib::False);
            }
        }
         else
        {
            // We aren't in the network, so store the default zeroed out key
            bSetCfgData(mbufKey);
        }

        //
        //  Tell the shared facility about our controller id and the network key
        //  we last stored, and our man ids.
        //
        facZWaveUSB3Sh().SetNetworkKey(mbufKey);
        facZWaveUSB3Sh().SetCtrlId(m_dcfgCurrent.c1CtrlId());
        m_zstCtrl.SetManIds(m_dcfgCurrent.c8ManIds());

        //
        //  Based on whether we are in the network or not, set up our node info. This
        //  will also broadcast our node info once set.
        //
        SetNodeInfo(m_dcfgCurrent.bInNetwork());

        // Do a few more things if we still seem to be in the network
        if (m_dcfgCurrent.bInNetwork())
        {
            //
            //  We send the master our man ids. If we don't do this, for whatever reason,
            //  none of our security msgs will not work. The controller somehow senses we
            //  went away and we cannot get any secure msgs to work until we do this.
            //
            //  <TBD> We changed this temporarily back to broadcast, to see if it helps
            //  with some locks not responding to secure messages. If so, then keep it
            //  that way.
            //
            //  We can this one unsecure, since the master isn't expecting secure msgs
            //  other than the security class.
            //
            //  It will be sent high priority to make sure it gets out before the
            //  reconciliation stuff happens.
            //
            //  If we lost our config, then we won't have the master id, which sucks
            //  and there's no way to get it in that case. We have to wait until we
            //  do another replication. In that case, broadcast it.
            //
            try
            {
                tCIDLib::TCard1 c1TarId = m_dcfgCurrent.c1MasterId();
                if (!c1TarId)
                    c1TarId = NODE_BROADCAST;

                facZWaveUSB3Sh().SendManIds
                (
                    *this
                    , NODE_BROADCAST // c1TarId
                    , m_dcfgCurrent.c8ManIds()
                    , kCIDLib::False // (c1TarId == NODE_BROADCAST) ? kCIDLib::False : kCIDLib::True
                );
            }

            catch(...)
            {
            }

            //
            //  It's possible, if the config file got deleted, that we saw all new
            //  units AFAWK. In that case, we have to make sure that they get prepped
            //  (or are marked failed if they can't be.) So let's do that now. Only
            //  needed (and possible) if we are in the network.
            //
            m_dcfgCurrent.ProbeNewUnits(this);
        }

        //
        //  One way or another, we now register fields based on whatever configuration
        //  we have.
        //
        RegisterFields();


        //
        //  If we are in the network, let's give our units a reasonable chance to get
        //  their values before we report ourselves connected. In theory drivers have
        //  to do this, but it's very difficult for us due to the async nature of
        //  our comms with the units.
        //
        //  We'll loop for a little bit, processing msgs. They've already sent out
        //  their initial value queries, which was done above after field reg.
        //
        if (m_dcfgCurrent.bInNetwork())
        {
            tCIDLib::TEncodedTime enctCur = TTime::enctNow();
            const tCIDLib::TEncodedTime enctEnd = enctCur + kCIDLib::enctTenSeconds;
            while ((enctCur < enctEnd) && !thrThis.bCheckShutdownRequest())
            {
                // Use a short wait to provide throttling
                const tZWaveUSB3Sh::EMsgTypes eRet = eGetReply
                (
                    zwimIn, kCIDLib::enctOneMilliSec * 250, kCIDLib::False
                );

                if (eRet != tZWaveUSB3Sh::EMsgTypes::Timeout)
                {
                    if (zwimIn.bIsAppCmd())
                        AppCmdHandler(zwimIn);
                    else if (zwimIn.bIsAppUpdate())
                        AppUpdateHandler(zwimIn);
                }

                ProcessNextUnit();
                enctCur = TTime::enctNow();
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // If we lost the Z-Stick connection, then cycle the connection
        if (!m_zstCtrl.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;

        // Else just say we haven't connected yet
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Handle driver init, the usual stuff.
//
tCQCKit::EDrvInitRes TZWaveUSB3SDriver::eInitializeImpl()
{
    // Setup our cfg file path
    TPathStr pathCfg = facCQCKit().strServerDataDir();
    pathCfg.AddLevel(L"ZWaveUSB3S");
    m_strCfgPath = pathCfg;

    // If this doesn't exist, create it
    TFileSys::MakePath(m_strCfgPath);

    //
    //  Make sure that we were configured for a serial connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCSerialConnCfg::clsThis())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCSerialConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    const TCQCSerialConnCfg& conncfgSer
        = static_cast<const TCQCSerialConnCfg&>(cqcdcOurs.conncfgReal());

    //
    //  And now set up our z-stick's configuration. We can't tell it our controller
    //  id until later.
    //
    m_zstCtrl.SetConfig(conncfgSer.strPortPath(), conncfgSer.cpcfgSerial());

    // If in debug mode, set tracing to high
    #if CID_DEBUG_ON
    facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::High);
    #endif

    // Now build up the actual file name for our config file
    pathCfg.AddLevel(strMoniker());
    pathCfg.AppendExt(L"Cfg");
    m_strCfgFile = pathCfg;

    //
    //  Set our poll and reconnect times to something desirable for us. We set the
    //  poll time to a low value, so that we can grab incoming async msgs and process
    //  them quickly.
    //
    SetPollTimes(50, 10000);

    // Store our version info later to put into the trace file if opened
    m_c4DrvVerMaj = cqcdcOurs.c4MajVersion();
    m_c4DrvVerMin = cqcdcOurs.c4MinVersion();

    // Force our shared facility to load the device info map, or to try to anyway.
    facZWaveUSB3Sh().LoadDevInfoMap();

    // Indicate that we need to load configuration
    return tCQCKit::EDrvInitRes::WaitConfig;
}



//
//  Handle int field writes
//
tCQCKit::ECommResults
TZWaveUSB3SDriver::eIntFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    // Reject if we are currently replicating
    CheckReplicating();
    facZWaveUSB3Sh().LogTraceMsg
    (
        L"Got int field write. Name=%(1) Value=%(2)", strName
        , TInteger(i4NewValue)
    );

    // Find the unit that owns this field
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Unhandled;
    tCIDLib::TCard4 c4CfgInd;
    TZWaveUnit* punitTar = punitFindFldOwner(c4FldId, c4CfgInd);
    if (punitTar)
    {
        try
        {
            punitTar->bIntFldChanged(strName, c4FldId, i4NewValue, eRes);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            IncFailedWriteCounter();
            eRes = tCQCKit::ECommResults::Exception;
        }
    }
     else
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"No unit reported owning this field's id (%(1))", TCardinal(c4FldId)
        );
    }
    if (eRes == tCQCKit::ECommResults::FieldNotFound)
        IncUnknownWriteCounter();

    return eRes;
}



//
//  Watch for aync notifications from units, do any periodic processing that we don the
//  units, and watch for an active replication to have succeeded or failed.
//
tCQCKit::ECommResults TZWaveUSB3SDriver::ePollDevice(TThread& thrCaller)
{
    // If the Z-Stick is not open at this point, we have to cycle
    if (!m_zstCtrl.bIsConnected())
        return tCQCKit::ECommResults::LostCommRes;

    try
    {
        if ((m_eZWDrvState >= tZWaveUSB3Sh::EDrvStates::FirstRepState)
        &&  (m_eZWDrvState <= tZWaveUSB3Sh::EDrvStates::LastRepState))
        {
            //
            //  We are in a replication or it has finished and we have yet to process
            //  that.
            //
            // If it has succeed, or failed, then deal with it
            if (m_eZWDrvState == tZWaveUSB3Sh::EDrvStates::RepSucceeded)
            {
                if (facZWaveUSB3Sh().bLowTrace())
                    facZWaveUSB3Sh().LogTraceMsg(L"Replication succeeded, updating configuration");

                try
                {
                    // Store it away to disc so we don't lose it
                    if (!bSaveConfig(m_dcfgReplicate))
                    {
                        facZWaveUSB3S().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kZW3Errs::errcCfg_SaveFile
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::CantDo
                        );
                    }
                     else
                    {
                        // We saved it ok, so let's store it as the new config
                        m_dcfgCurrent = m_dcfgReplicate;

                        // And now register fields based on what we have
                        RegisterFields();
                    }
                }

                catch(TError& errToCatch)
                {
                    // Just go back to ready state
                    SetDrvState(tZWaveUSB3Sh::EDrvStates::Ready);
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facZWaveUSB3S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW3Errs::errcRep_StoreFailed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }

                catch(...)
                {
                    // Just go back to ready state
                    SetDrvState(tZWaveUSB3Sh::EDrvStates::Ready);
                    facZWaveUSB3S().ThrowErr
                    (
                        CID_FILE
                        , CID_LINE
                        , kZW3Errs::errcRep_StoreFailed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }
            }
             else if (m_eZWDrvState == tZWaveUSB3Sh::EDrvStates::RepFailed)
            {
                //
                //  Go back ready state, we never changed our config so nothing really
                //  has happened.
                //
                SetDrvState(tZWaveUSB3Sh::EDrvStates::Ready);

                if (facZWaveUSB3Sh().bLowTrace())
                    facZWaveUSB3Sh().LogTraceErr(L"Replication failed");

                LogMsg
                (
                    L"Replication failed, taking no action"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                );
            }
        }
         else  if (m_eZWDrvState >= tZWaveUSB3Sh::EDrvStates::Ready)
        {
            // Keep our queue percentage fields updated
            bStoreCardFld
            (
                m_c4FldId_OutQPercent
                , m_zstCtrl.c4OutQUsedPercent()
                , kCIDLib::True
            );

            bStoreCardFld
            (
                m_c4FldId_PendingQPercent
                , m_zstCtrl.c4CANQUsedPercent()
                , kCIDLib::True
            );

            //
            //  Handle up to so many asyncs this time around. We only watch for ones
            //  that have already arrived. We will only do so many per time through
            //  so we can't get hung up. We do a little wait in order to provide some
            //  throttling.
            //
            for (tCIDLib::TCard4 c4Index = 0; c4Index < 32; c4Index++)
            {
                const tZWaveUSB3Sh::EMsgTypes eRet = eGetReply
                (
                    m_zwimPoll, kCIDLib::enctOneMilliSec * 20, kCIDLib::False
                );
                if (eRet == tZWaveUSB3Sh::EMsgTypes::Timeout)
                    break;

                //
                //  The only things we really expect here are app cmds which are msgs
                //  from ther nodes. We pass these to the app cmd handler and he does
                //  the appropriate thing. Some we handle and those targeting specifc
                //  units we pass to those units if they are in an appropriate state.
                //
                //  But we can also see app update messages which can give us more info
                //  on modules we have found but haven't figured out what they are yet.
                //
                //  We can't do anything with them if not in the network.
                //
                if (m_dcfgCurrent.bInNetwork())
                {
                    if (m_zwimPoll.bIsAppCmd())
                        AppCmdHandler(m_zwimPoll);
                    else if (m_zwimPoll.bIsAppUpdate())
                        AppUpdateHandler(m_zwimPoll);
                }
            }

            //
            //  And do any needed unit processing. This gives them a chance to do
            //  whatever they need, but mostly it's just after a replication to get
            //  info on themselves.
            //
            if (m_dcfgCurrent.bInNetwork())
                ProcessNextUnit();
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

    }
    return tCQCKit::ECommResults::Success;
}


//
//  Handle a write to a string field
//
tCQCKit::ECommResults
TZWaveUSB3SDriver::eStringFldValChanged(const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    // Reject if we are currently replicating
    CheckReplicating();
    facZWaveUSB3Sh().LogTraceMsg
    (
        L"Got string field write. Name=%(1) Value=%(2)", strName, strNewValue
        );

    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Unhandled;
    try
    {
        if (c4FldId == m_c4FldId_InvokeCmd)
        {
            // We have a separate method to process this since there's a good bit of code
            eRes = eInvokeCmd(strNewValue);
        }
         else
        {
            tCIDLib::TCard4 c4CfgInd;
            TZWaveUnit* punitTar = punitFindFldOwner(c4FldId, c4CfgInd);
            if (punitTar)
            {
                try
                {
                    punitTar->bStringFldChanged(strName, c4FldId, strNewValue, eRes);
                }

                catch(TError& errToCatch)
                {
                    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::Exception;
               }
            }
             else
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"No unit reported owning this field's id (%(1))", TCardinal(c4FldId)
                );
            }
        }

        if (eRes == tCQCKit::ECommResults::FieldNotFound)
            IncUnknownWriteCounter();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCIDLib::TVoid TZWaveUSB3SDriver::ReleaseCommResource()
{
    m_zstCtrl.Close();
}


tCIDLib::TVoid TZWaveUSB3SDriver::TerminateImpl()
{
    // If the replication/scanner thread is running, stop it
    try
    {
        if (m_thrRepScan.bIsRunning())
        {
            m_thrRepScan.ReqShutdownSync(10000);
            m_thrRepScan.eWaitForDeath(5000);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }

    #if CID_DEBUG_ON
    if (facZWaveUSB3Sh().bLowTrace())
        m_zstCtrl.LogTermTrace();
    #endif

    //
    //  In case we got any good new info on so far not in action units, save our config
    //  so that we'll already have that info next time we load.
    //
    if (!bSaveConfig(m_dcfgCurrent))
    {
        facZWaveUSB3S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW3Errs::errcCfg_SaveFile
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
    }

    // Disable tracing to flush any remaining content to the file
    facZWaveUSB3Sh().SetTraceLevel(tCQCKit::EVerboseLvls::Off);
}


// Pass on verbosity level changes to our shared facility
tCIDLib::TVoid TZWaveUSB3SDriver::VerboseModeChanged(const tCQCKit::EVerboseLvls eNewState)
{
    facZWaveUSB3Sh().SetVerboseLevel(eNewState);
}


// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called to get a msg. It should not be called directly, always use
//  bWaitForMsg or one of the methods that sends and waits for replies. That insures
//  that we dispatch any asyncs appropriately.
//
//  Only the replication loop directly calls this, since it doesn't want asyncs to be
//  dispatched while it's working.
//
tZWaveUSB3Sh::EMsgTypes
TZWaveUSB3SDriver::eGetReply(       TZWInMsg&               zwimToFill
                            , const tCIDLib::TEncodedTime   enctWait
                            , const tCIDLib::TBoolean       bThrowIfNot)
{
    tCIDLib::TCard4 c4Millis(tCIDLib::TCard4(enctWait / kCIDLib::enctOneMilliSec));
    return m_zstCtrl.eGetMsg(zwimToFill, c4Millis, bThrowIfNot);
}



// ---------------------------------------------------------------------------
//  TZWaveUSB3SDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when we receive a command class msg (an application command in Z-Wave speak.)
//  So these are all of the defined command class payload format.
//
//  We handle some special command class msgs here, some that are part of core housekeeping.
//  The others we pass to the target unit if it's one we know about and it is in a state
//  that it can handle it.
//
//  NOTE that we never get multi-channel encapsulated msgs or encrypted msgs. The I/O
//  thread expands those out before queuing up the messages for us to process.
//
tCIDLib::TVoid TZWaveUSB3SDriver::AppCmdHandler(const TZWInMsg& zwimIn)
{
    // Get the node that sent this, which is the target for any reply, and the CC stuff
    const tCIDLib::TCard1 c1TarId = zwimIn.c1SrcId();
    tCIDLib::TCard1 c1ClassId = zwimIn.c1ClassId();
    tCIDLib::TCard1 c1CmdId = zwimIn.c1CmdId();


    // Check the target id
    if (!c1TarId || ((c1TarId > kZWaveUSB3Sh::c4MaxUnits) && c1TarId != NODE_BROADCAST))
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Bad target unit id in app cmd msg. Id=%(1)"
                , TCardinal(c1TarId, tCIDLib::ERadices::Hex)
            );
        }
        return;
    }

    //
    //  Some things we can handle during a replication because they are part of that
    //  process or the master may send them during it. Other things we ignore because
    //  they are things like async notifications from devices which we can't deal
    //  with because we are updating the driver config at the time.
    //
    if (c1ClassId == COMMAND_CLASS_NODE_NAMING)
    {
        // We if a get and it's single cast, respond with our configured name
        if (c1CmdId == NODE_NAMING_NODE_NAME_GET)
        {
            if (zwimIn.bIsSingleCast())
            {
                TZWOutMsg zwomRep;
                zwomRep.Reset
                (
                    c1TarId
                    , COMMAND_CLASS_NODE_NAMING
                    , NODE_NAMING_NODE_NAME_REPORT
                    , 0
                    , 0
                    , tZWaveUSB3Sh::EMsgPrios::Reply
                );
                zwomRep += 0; //ASCII format

                // Add the bytes for our name, the default or what we were last assigned
                const TString& strName = m_dcfgCurrent.strCtrlName();
                tCIDLib::TCard4 c4Cnt = strName.c4Length();

                // Shouldn't happen but clip to max legal length just in case
                if (c4Cnt > 16)
                    c4Cnt = 16;
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
                    zwomRep += tCIDLib::TCard1(strName[c4Index]);
                zwomRep.EndCounter();
                zwomRep.AppendTransOpts();
                zwomRep.AppendCallback();

                // Send back secure if we got the request secure
                zwomRep.Finalize(kCIDLib::False, zwimIn.m_bSecure);
                USB3SendCCMsg(zwomRep);
            }
            return;
        }
    }
     else if (c1ClassId == COMMAND_CLASS_SECURITY)
    {
        if (c1CmdId == SECURITY_SCHEME_GET)
        {
            //
            //  For this one, we just set a flag. We have to control when we respond
            //  to this because we have to get some other things done after a replication
            //  loop, before we start the inclusion process. The replication thread will
            //  check this flag after it gets done with those chores. If not set, it will
            //  wait for this msg. Then, either way, it will respond which will let the
            //  reset of the process continue. He will zero this before he starts
            //  the replication loop. We store the master controller id, since this is
            //  the first msg we can get it from.
            //
            m_c1GotSecSchemeGet = zwimIn.c1SrcId();
            if (facZWaveUSB3Sh().bLowTrace())
                facZWaveUSB3Sh().LogTraceMsg(L"Got security scheme get");
        }
         else if (c1CmdId == SECURITY_SCHEME_INHERIT)
        {
            // We only support the 0th (default) security scheme
            TZWOutMsg zwomRep;
            zwomRep.Reset
            (
                c1TarId
                , COMMAND_CLASS_SECURITY
                , SECURITY_SCHEME_REPORT
                , 0
                , 0
                , tZWaveUSB3Sh::EMsgPrios::Security
            );
            zwomRep += 0;
            zwomRep.EndCounter();
            zwomRep.AppendTransOpts();
            zwomRep.AppendCallback();

            // The inherit has to be sent securely
            zwomRep.Finalize(kCIDLib::False, kCIDLib::True);
            USB3SendCCMsg(zwomRep);
        }
         else if (c1CmdId == SECURITY_COMMANDS_SUPPORTED_GET)
        {
            // Tell the caller what secure classes we support.
            TZWOutMsg zwomRep;
            zwomRep.Reset
            (
                c1TarId
                , COMMAND_CLASS_SECURITY
                , SECURITY_COMMANDS_SUPPORTED_REPORT
                , 0
                , 0
                , tZWaveUSB3Sh::EMsgPrios::Security
            );
            zwomRep += 0;
            zwomRep += COMMAND_CLASS_MANUFACTURER_SPECIFIC;
            zwomRep += COMMAND_CLASS_SECURITY;
            zwomRep.EndCounter();
            zwomRep.AppendTransOpts();
            zwomRep.AppendCallback();

            // Send securely always
            zwomRep.Finalize(kCIDLib::False, kCIDLib::True);
            USB3SendCCMsg(zwomRep);
        }
         else if (c1CmdId == NETWORK_KEY_SET)
        {
            //
            //  Send back a key verify, secure using the new key. This is our first
            //  secure msg we send when being included. Send it security priority
            //
            TZWOutMsg zwomRep;
            zwomRep.Reset
            (
                c1TarId
                , COMMAND_CLASS_SECURITY
                , NETWORK_KEY_VERIFY
                , 0
                , 2
                , tZWaveUSB3Sh::EMsgPrios::Security
            );
            zwomRep.AppendTransOpts();
            zwomRep.AppendCallback();

            // Send securely
            zwomRep.Finalize(kCIDLib::False, kCIDLib::True);
            USB3SendCCMsg(zwomRep);
        }
    }
     else if (c1ClassId == COMMAND_CLASS_VERSION)
    {
        if (c1CmdId == VERSION_GET)
        {
            //
            //  It's generally the master, asking us for our Z-Wave version info. We
            //  stored some of this away during startup, and some we just know and can
            //  do here.
            //
            TZWOutMsg zwomRep;
            zwomRep.Reset
            (
                c1TarId
                , COMMAND_CLASS_VERSION
                , VERSION_REPORT
                , 0
                , 7
                , tZWaveUSB3Sh::EMsgPrios::Reply
            );

            // We are a static controller
            zwomRep += 0x01;

            // Z-Wave protocol version/sub-ver
            tCIDLib::TCard4 c4Maj, c4Min;
            m_dcfgCurrent.QueryZWLibVer(c4Maj, c4Min);
            zwomRep += tCIDLib::TCard1(c4Maj);
            zwomRep += tCIDLib::TCard1(c4Min);

            // And our application version, which we just arbitrarily set to 1.0
            zwomRep += 1;
            zwomRep += 0;

            zwomRep.AppendTransOpts();
            zwomRep.AppendCallback();

            zwomRep.Finalize(kCIDLib::False, kCIDLib::False);
            USB3SendCCMsg(zwomRep);
            return;
        }
    }

    //
    //  It wasn't one of the special ones, then we only want to process if we are in
    //  one of the states that we need to process them.
    //
    if (m_eZWDrvState < tZWaveUSB3Sh::EDrvStates::Ready)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Not ready to process app cmd. Src=%(1), Class=%(2), Cmd=%(3)"
                , TCardinal(c1TarId, tCIDLib::ERadices::Hex)
                , TCardinal(c1ClassId, tCIDLib::ERadices::Hex)
                , TCardinal(c1CmdId, tCIDLib::ERadices::Hex)
            );
        }
        return;
    }

    if (facZWaveUSB3Sh().bMediumTrace())
    {
        facZWaveUSB3Sh().LogTraceMsg
        (
            L"Processing app cmd. Src=%(1), Class=%(2), Cmd=%(3)"
            , TCardinal(c1TarId, tCIDLib::ERadices::Hex)
            , TCardinal(c1ClassId, tCIDLib::ERadices::Hex)
            , TCardinal(c1CmdId, tCIDLib::ERadices::Hex)
        );
    }

    // The number of class payload butes cannot be less than 2 (the cmd class and the cmd)
    const tCIDLib::TCard4 c4CmdBytes = zwimIn.c4CCBytes();
    if (c4CmdBytes < 2)
    {
        if (facZWaveUSB3Sh().bMediumTrace())
            facZWaveUSB3Sh().LogTraceErr(L"Got an app cmd response shorter than 2 bytes, ignoring it");
        return;
    }

    //
    //  Some more stuff that we won't handle during replicate, but it is targeting our
    //  controller.
    //
    if (c1ClassId == COMMAND_CLASS_NODE_NAMING)
    {
        if (c1CmdId == NODE_NAMING_NODE_NAME_SET)
        {
            //
            //  NOTE: We will get one of these during replication, but we don't want it
            //  to be handled here, since this updates the current config, not the
            //  discovery info that is being filled in during replication. So the
            //  replication thread handles this directly in that case.
            //
            TString strName;
            if (TZWUSB3CCNaming::bExtractNodeName(zwimIn, strName))
            {
                //
                //  If if's different from what we have, store it and set the new config
                //  flag.
                //
                if (!strName.bCompareI(m_dcfgCurrent.strCtrlName()))
                {
                    m_dcfgCurrent.strCtrlName(strName);
                    if (bSaveConfig(m_dcfgCurrent))
                        bStoreBoolFld(m_c4FldId_NewConfig, kCIDLib::True, kCIDLib::True);
                }
            }
        }
    }


    //
    //  If we 't have this unit in our config, then pass the msg on to him to process
    //  if he is ready for it. WaitDevInfo is a special case in that it's below the
    //  normally first viable state, but we do want units in that state to be able
    //  to see at least man id reports so that they can get their device info.
    //
    TZWUnitInfo* punitiTar = m_dcfgCurrent.punitiFindById(c1TarId);
    if (punitiTar
    &&  ((punitiTar->eState() >= tZWaveUSB3Sh::EUnitStates::FirstViableState)
    ||   (punitiTar->eState() == tZWaveUSB3Sh::EUnitStates::WaitDevInfo)))
    {
        try
        {
            punitiTar->HandleCCMsg(c1ClassId, c1CmdId, zwimIn);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3Errs::errcComm_AppCmdExcept
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Unknown
                    , TCardinal(c1TarId)
                    , TCardinal(c1ClassId)
                    , TCardinal(c1CmdId)
                );
            }
        }
    }
}


tCIDLib::TVoid TZWaveUSB3SDriver::AppUpdateHandler(const TZWInMsg& zwimIn)
{
    //
    //  We don't really deal with these right now, but we need to filter them out
    //  so we don't react incorrectly below or think it's an error when it's not.
    //
    const tCIDLib::TCard1 c1UpdateStatus = zwimIn[0];
    if (c1UpdateStatus == UPDATE_STATE_DELETE_DONE)
    {
        facZWaveUSB3Sh().LogTraceErr
        (
            L"Node %(1) was reported as being removed", TCardinal(zwimIn[1])
        );
       return;
    }
     else if (c1UpdateStatus == UPDATE_STATE_SUC_ID)
    {
        return;
    }


    //
    //  Check the status. If not good, then give up. The node id won't be set
    //  here generally but we try to report it.
    //
    const tCIDLib::TCard1 c1SrcId = zwimIn[1];
    if (c1UpdateStatus != UPDATE_STATE_NODE_INFO_RECEIVED)
    {
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Got node info query failure status=%(1). NodeId=%(2)"
                , TCardinal(c1UpdateStatus)
                , TCardinal(c1SrcId)
            );
        }
        return;
    }

    // Check the source id
    if (!c1SrcId || ((c1SrcId > kZWaveUSB3Sh::c4MaxUnits) && (c1SrcId != NODE_BROADCAST)))
    {
        if (facZWaveUSB3Sh().bMediumTrace())
        {
            facZWaveUSB3Sh().LogTraceErr
            (
                L"Bad target unit id in app update msg. Id=%(1)"
                , TCardinal(c1SrcId, tCIDLib::ERadices::Hex)
            );
        }
        return;
    }

    // If it's the broadcast address, that would be weird but not something we want
    if (c1SrcId == NODE_BROADCAST)
    {
        facZWaveUSB3Sh().LogTraceErr(L"Ignored app update command for broadcast id");
        return;
    }

    //
    //  See if we have a unit for this guy.
    //
    TZWUnitInfo* punitiTar = m_dcfgCurrent.punitiFindById(c1SrcId);
    if (punitiTar)
    {
        try
        {
            punitiTar->HandleAppUpdate(zwimIn);
        }

        catch(TError& errToCatch)
        {
            if (facZWaveUSB3Sh().bLowTrace())
            {
                facZWaveUSB3Sh().LogTraceErr
                (
                    L"Exception handling app update for node %(1)", TCardinal(c1SrcId)
                );
            }

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facZWaveUSB3Sh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kZW3Errs::errcComm_AppUpdateExcept
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Unknown
                    , TCardinal(c1SrcId)
                );
            }
        }
    }
}


//
//  Checks to see if we are in replication mode and throws an error if so. The error is
//  from the shared faciity so that the client can know what is is and check for this one
//  specifically.
//
tCIDLib::TVoid TZWaveUSB3SDriver::CheckReplicating() const
{
    if ((m_eZWDrvState >= tZWaveUSB3Sh::EDrvStates::FirstRepState)
    &&  (m_eZWDrvState <= tZWaveUSB3Sh::EDrvStates::LastRepState))
    {
        facZWaveUSB3Sh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW3ShErrs::errcCIntf_Replicating
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Busy
        );
    }
}


//
//  We parse out the command and invoke it, returning the appropriate value for
//  our caller to return to the client. There are only a small number of them.
//  Most commands are only for the client side driver to use, so they are implemented
//  via a backdoor command.
//
//  The commands supported here are:
//
//      EnableTrace [Off|Low|Medium|High]
//          Set the trace log verbosity (which will also enable/disable it.)
//
//      FlushTrace
//          Lets us force the trace to flush so we can read it on the fly.
//
//      RecycleTrace
//          Clears the trace file out so that we start collecting new trace info.
//
//      SetCfgParam unitname parm# val bytes [awake]
//          Set the indicated configuration parameter on the named unit. They can
//          optionally tell us that the unit is now awake if it is battery powered
//
//

// A helper to check that the expected number of parameters were received
static tCIDLib::TBoolean
bCheckCmdParams(const   tCIDLib::TCard4 c4MinCnt
                , const tCIDLib::TCard4 c4MaxCnt
                , const tCIDLib::TCard4 c4GotCnt)
{
    return ((c4GotCnt >= c4MinCnt) && (c4GotCnt <= c4MaxCnt));
}

tCQCKit::ECommResults TZWaveUSB3SDriver::eInvokeCmd(const TString& strCmdText)
{
    //
    //  The caller (string field changed callback) only allows writes if the driver is
    //  in ready state, but just in case...
    //
    if (m_eZWDrvState != tZWaveUSB3Sh::EDrvStates::Ready)
    {
        LogMsg
        (
            L"The driver is not currently ready to accept commands"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strCmdText
        );
        return tCQCKit::ECommResults::Busy;
    }

    //
    //  Break out the command parameters. They are in the standard command
    //  line format, so space separated tokens, quoted where necessary to
    //  include spaces in tokens.
    //
    tCIDLib::TStrList colTokens;
    if (!TExternalProcess::c4BreakOutParms(strCmdText, colTokens) || colTokens.bIsEmpty())
    {
        LogMsg
        (
            L"'%(1)' is not a valid Command field string"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strCmdText
        );
        return tCQCKit::ECommResults::BadValue;
    }

    // Strip leading and trailing white space from the tokens
    const tCIDLib::TCard4 c4TokenCnt = colTokens.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TokenCnt; c4Index++)
        colTokens[c4Index].StripWhitespace();

    // The first one is the command, and may be the only one
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    const TString strCmd = colTokens[0];

    // Remove it now from the list to make the parameter processing below more logical
    colTokens.RemoveAt(0);
    try
    {
       if (strCmd.bCompareI(kZWaveUSB3Sh::strInvokeCmd_EnableTrace))
        {
            if (!bCheckCmdParams(1, 1, colTokens.c4ElemCount()))
                return tCQCKit::ECommResults::CmdParms;

            tCQCKit::EVerboseLvls eLevel = tCQCKit::EVerboseLvls::Count;
            if (colTokens[0].bCompareI(L"Off"))
                eLevel = tCQCKit::EVerboseLvls::Off;
            else if (colTokens[0].bCompareI(L"Low"))
                eLevel = tCQCKit::EVerboseLvls::Off;
            else if (colTokens[0].bCompareI(L"Medium"))
                eLevel = tCQCKit::EVerboseLvls::Medium;
            else if (colTokens[0].bCompareI(L"High"))
                eLevel = tCQCKit::EVerboseLvls::High;
            else
                eRes = tCQCKit::ECommResults::CmdParms;

            if (eLevel != tCQCKit::EVerboseLvls::Count)
                facZWaveUSB3Sh().SetTraceLevel(eLevel);
        }
         else if (strCmd.bCompareI(kZWaveUSB3Sh::strInvokeCmd_FlushTrace))
        {
            facZWaveUSB3Sh().FlushMsgTrace();
        }
         else if (strCmd.bCompareI(kZWaveUSB3Sh::strInvokeCmd_RecycleTrace))
        {
            facZWaveUSB3Sh().ResetMsgTrace();
        }
         else if (strCmd.bCompareI(kZWaveUSB3Sh::strInvokeCmd_SetCfgParam))
        {
            if (!bCheckCmdParams(4, 5, colTokens.c4ElemCount()))
                return tCQCKit::ECommResults::CmdParms;

            // Get the unit name and see if we can find it
            TZWUnitInfo* punitiTar = m_dcfgCurrent.punitiFindByName(colTokens[0]);
            if (!punitiTar)
                return tCQCKit::ECommResults::ResNotFound;

            // Next we get three numbers
            tCIDLib::TCard1 c1PNum;
            tCIDLib::TCard4 c4Val, c4Bytes;
            if (!colTokens[1].bToCard1(c1PNum, tCIDLib::ERadices::Auto)
            ||  !colTokens[2].bToCard4(c4Val, tCIDLib::ERadices::Auto)
            ||  !colTokens[3].bToCard4(c4Bytes, tCIDLib::ERadices::Auto))
            {
                return tCQCKit::ECommResults::CmdParms;
            }

            // If there's an awake get that
            tCIDLib::TBoolean bAwake = kCIDLib::False;
            if (colTokens.c4ElemCount() == 5)
                bAwake = colTokens[4].bCompareI(L"Awake");

            // Let's do the command, synchronously
            TString strErr;
            const tCIDLib::TBoolean bRes = TZWUSB3CCConfig::bSetParam
            (
                *punitiTar
                , c1PNum
                , c4Bytes
                , tCIDLib::TInt4(c4Val)
                , kCIDLib::True
                , bAwake
                , strErr
            );

            if (!bRes)
            {
                LogMsg(strErr, tCQCKit::EVerboseLvls::Low, CID_FILE, CID_LINE);
                eRes = tCQCKit::ECommResults::DeviceResponse;
            }
        }
         else
        {
            LogMsg
            (
                L"'%(1)' is not a valid InvokeCmd command value"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
                , strCmd
            );
            eRes = tCQCKit::ECommResults::BadValue;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        eRes = tCQCKit::ECommResults::Exception;
    }

    return eRes;
}





//
//  Find our unit in the list by its id or name of if it owns a speciifc field id.
//
//  Even if we have one with that id or name, it doesn't mean that it has a unit object.
//  It either may never, or at least not until the user provides us with more info, or it
//  just may not be ready yet.
//
//  THESE ONLY work on the current config.
//
TZWaveUnit* TZWaveUSB3SDriver::punitFindById(const tCIDLib::TCard1 c1ToFind)
{
    TZWaveUnit* punitRet = nullptr;

    tZWaveUSB3Sh::TUnitInfoList& colUnits = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4Count = colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = colUnits[c4Index];
        if (unitiCur.c1Id() == c1ToFind)
        {
            // It's our guy by id. See if he has a unit object
            TZWaveUnit* punitCur = unitiCur.punitThis();
            if (punitCur)
            {
                punitRet = punitCur;
                break;
            }
        }
    }
    return punitRet;
}

TZWaveUnit* TZWaveUSB3SDriver::punitFindByName(const TString& strToFind)
{
    TZWaveUnit* punitRet = nullptr;

    tZWaveUSB3Sh::TUnitInfoList& colUnits = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4Count = colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = colUnits[c4Index];
        if (unitiCur.strName().bCompareI(strToFind))
        {
            // It's our guy by name. See if he has a unit object
            TZWaveUnit* punitCur = unitiCur.punitThis();
            if (punitCur)
            {
                punitRet = punitCur;
                break;
            }
        }
    }
    return punitRet;
}

TZWUnitInfo* TZWaveUSB3SDriver::punitiFindByName(const TString& strToFind)
{
    TZWUnitInfo* punitiRet = nullptr;

    tZWaveUSB3Sh::TUnitInfoList& colUnits = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4Count = colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TZWUnitInfo& unitiCur = colUnits[c4Index];
        if (unitiCur.strName().bCompareI(strToFind))
        {
            punitiRet = &unitiCur;
            break;
        }
    }
    return punitiRet;
}

TZWaveUnit*
TZWaveUSB3SDriver::punitFindFldOwner(const  tCIDLib::TCard4     c4FldId
                                    ,       tCIDLib::TCard4&    c4Index)
{
    TZWaveUnit* punitRet = nullptr;

    // Find the unit that owns this field
    tZWaveUSB3Sh::TUnitInfoList& colUnits = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4Count = colUnits.c4ElemCount();
    for (c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // If this guy has a unit object and it owns the field, it's our guy
        TZWaveUnit* punitCur = colUnits[c4Index].punitThis();
        if (punitCur && punitCur->bOwnsFld(c4FldId))
        {
            punitRet = punitCur;
            break;
        }
    }
    return punitRet;
}


//
//  Our poll callback calls this periodically to let us.
//
tCIDLib::TVoid TZWaveUSB3SDriver::ProcessNextUnit()
{
    tZWaveUSB3Sh::TUnitInfoList& colList = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
    if (c4Count)
    {
        if (c4Count > 1)
        {
            //
            //  Find the next unit and call its processing method. We move forward one
            //  from the current index. Then we search for a viable unit until we find
            //  one or get back to the original index.
            //
            const tCIDLib::TCard4 c4OrgInd = m_c4NextProcUnit;

            m_c4NextProcUnit++;
            if (m_c4NextProcUnit == c4Count)
                m_c4NextProcUnit = 0;
            while (m_c4NextProcUnit != c4OrgInd)
            {
                // If it's in a viable state, break out
                if (colList[m_c4NextProcUnit].bIsViable())
                    break;

                m_c4NextProcUnit++;
                if (m_c4NextProcUnit == c4Count)
                    m_c4NextProcUnit = 0;
            }
        }
         else
        {
            // If we only have one, then it has to be the 0th one
            m_c4NextProcUnit = 0;
        }

        // If the one we ended on is viable, then process it
        if (colList[m_c4NextProcUnit].bIsViable())
        {
            try
            {
                colList[m_c4NextProcUnit].Process();
            }

            catch(TError& errToCatch)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }
            }

            catch(...)
            {
            }
        }
    }
}


//
//  We set up any driver level fields, and then we go through and set up the fields for
//  any units that have been assigned their device info at this point, and set them to
//  GetInitVals state.
//
//  The others are just left alone and will start to collect data over time. Any that can
//  be identified will go to WaitApprove state, ready for the user to approve them.
//
tCIDLib::TVoid TZWaveUSB3SDriver::RegisterFields()
{
    // Build up a field list based on the new config
    TString strLims;
    TVector<TCQCFldDef> colFlds(64);
    TCQCFldDef flddNew;

    flddNew.Set(L"CtrlUnitId", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(L"HomeId", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set(L"InZWNetwork", tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"InvokeCmd", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Write, kCIDLib::True
    );
    colFlds.objAdd(flddNew);

    flddNew.Set(L"NewConfig", tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Read);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"OutQPercent"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range: 0, 100"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"PendingQPercent"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , L"Range: 0, 100"
    );
    colFlds.objAdd(flddNew);

    tZWaveUSB3Sh::FormatEDrvStates(strLims, L"Enum: ", kCIDLib::chComma);
    flddNew.Set(L"State", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read, strLims);
    colFlds.objAdd(flddNew);

    //
    //  We store the controller version and FW version as the standard V2
    //  device info fields.
    //
    TCQCDevClass::c4LoadDevInfoFlds(colFlds, TString::strEmpty());

    //
    //  If we are in the Z-Wave network, then we need to register our unit driven
    //  fields. We go through them and get basic unit info on them. Those that have
    //  device info we get ready and register their fields.
    //
    if (m_dcfgCurrent.bInNetwork())
    {
        // Prep our units for use
        m_dcfgCurrent.PrepForUse(this);

        tZWaveUSB3Sh::TUnitInfoList& colList = m_dcfgCurrent.colUnits();
        tCIDLib::TCard4 c4Count = colList.c4ElemCount();
        if (c4Count)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TZWUnitInfo& unitiCur = colList[c4Index];

                if (unitiCur.eState() == tZWaveUSB3Sh::EUnitStates::GetInitVals)
                {
                    CIDAssertX
                    (
                        unitiCur.punitThis() != nullptr
                        , L"Unit %(1) in WaitFlds state but has no handler object"
                        , unitiCur.strName()
                    );
                    unitiCur.punitThis()->QueryFldDefs(colFlds, flddNew);
                }
            }
        }
    }

    //
    //  Save our config now to pick up anything that might have changed during the
    //  unit processing.
    //
    bSaveConfig(m_dcfgCurrent);

    // Set these fields on our base class
    SetFields(colFlds);

    //
    //  Initial them all to error state. We are one of those drivers that cannot
    //  promise to get all fields updated before reporting we are connected. The
    //  realities of Z-Wave don't allow for that. This makes sure any not set are
    //  in error state, and that we don't trigger the warning message that we have
    //  fields not explicitly set to some valid state.
    //
    SetAllErrStates();

    //
    //  These we just store by name and never change unless we do a replication.
    //
    bStoreBoolFld(L"InZWNetwork", m_dcfgCurrent.bInNetwork(), kCIDLib::True);
    bStoreCardFld(L"HomeId", m_dcfgCurrent.c4HomeId(), kCIDLib::True);
    bStoreCardFld(L"CtrlUnitId", m_dcfgCurrent.c1CtrlId(), kCIDLib::True);
    bStoreStringFld(L"DEVI#Firmware", m_dcfgCurrent.strFWVersion(), kCIDLib::True);
    bStoreStringFld(L"DEVI#Model", m_dcfgCurrent.strCtrlModel(), kCIDLib::True);

    // Look up any other fields that we own at this level
    m_c4FldId_InvokeCmd = pflddFind(L"InvokeCmd", kCIDLib::True)->c4Id();
    m_c4FldId_NewConfig = pflddFind(L"NewConfig", kCIDLib::True)->c4Id();
    m_c4FldId_OutQPercent = pflddFind(L"OutQPercent", kCIDLib::True)->c4Id();
    m_c4FldId_PendingQPercent = pflddFind(L"PendingQPercent", kCIDLib::True)->c4Id();
    m_c4FldId_State = pflddFind(L"State", kCIDLib::True)->c4Id();

    // And get initial values set on the ones that need it
    UseDefFldValue(m_c4FldId_NewConfig);

    //
    //  And, if in network, let the units look up their fields. And we move them to get
    //  init values state, so that they will start trying to get all their field values.
    //  They will be moved to ready state after they issue their initial value stuff.
    //
    if (m_dcfgCurrent.bInNetwork())
    {
        tZWaveUSB3Sh::TUnitInfoList& colList = m_dcfgCurrent.colUnits();
        const tCIDLib::TCard4 c4Count = colList.c4ElemCount();
        if (c4Count)
        {
            SetDrvState(tZWaveUSB3Sh::EDrvStates::PrepUnits);
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                TZWUnitInfo& unitiCur = colList[c4Index];
                if (unitiCur.eState() >= tZWaveUSB3Sh::EUnitStates::GetInitVals)
                    unitiCur.punitThis()->StoreFldIds();
            }
        }
    }

    // One way or another we go to ready state
    SetDrvState(tZWaveUSB3Sh::EDrvStates::Ready);
}


//
//  Called when the client driver asks to reset ourself. We put ourself back to ground
//  zero state, though still online since we are still talking to the ctrl. This is called
//  after an exclusion from the network typically.
//
//  We let any exceptions propagate back. The operations are done such that, if it fails,
//  it won't leave us in an inconsistent state, though they might have re-replicate to get
//  us back going again.
//
//  NOTE that we don't trash the unit config. It's left in place in case they are going
//  to re-add us.
//
tCIDLib::TVoid TZWaveUSB3SDriver::ResetCtrl()
{
    if (facZWaveUSB3Sh().bLowTrace())
        facZWaveUSB3Sh().LogTraceMsg(L"Resetting driver state)", strMoniker());


    // And now reset the controller
    try
    {
        // It will send a callback, so wait for that
        if (facZWaveUSB3Sh().bLowTrace())
            facZWaveUSB3Sh().LogTraceMsg(L"Waiting for controller to reset");

        TZWOutMsg zwomOut;
        zwomOut.Reset(FUNC_ID_ZW_SET_DEFAULT, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
        zwomOut.AppendCallback();
        zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
        TZWInMsg zwimIn;
        USB3SendReq(zwomOut, zwimIn, 4000);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // Query new basic info, since we still want to expose that stuff
    m_dcfgCurrent.LoadControllerInfo(*this);

    //
    //  It worked, so let's trash our configuraton and update our field
    //  list and write the new config out to the config server.
    //
    if (facZWaveUSB3Sh().bLowTrace())
        facZWaveUSB3Sh().LogTraceMsg(L"Clearing driver config and storing it");

    // Delete our stored key info if we set it
    bDeleteCfgData();

    // And let's register fields based on this new basic info
    RegisterFields();

    // Save our new configuration
    bSaveConfig(m_dcfgCurrent);
}


//
//  A helper to set our driver state field. This should ALWAYS be used to set the
//  state so that any magic stuff that needs to happen will get done.
//
tCIDLib::TVoid TZWaveUSB3SDriver::SetDrvState(const tZWaveUSB3Sh::EDrvStates eToSet)
{
    // For any replication states, we also update the rep state
    if ((eToSet >= tZWaveUSB3Sh::EDrvStates::FirstRepState)
    &&  (eToSet <= tZWaveUSB3Sh::EDrvStates::LastRepState))
    {
        m_eRepState = eToSet;
    }

    // If our state has changed, then store it
    if (eToSet != m_eZWDrvState)
    {
        m_eZWDrvState = eToSet;
        if (facZWaveUSB3Sh().bLowTrace())
        {
            facZWaveUSB3Sh().LogTraceMsg
            (
                L"Driver transitioned to state: %(1)"
                , tZWaveUSB3Sh::strLoadEDrvStates(m_eZWDrvState)
            );
        }

        // If our state field is ready, store the text of this state to it
        if (m_c4FldId_State != kCIDLib::c4MaxCard)
        {
            bStoreStringFld
            (
                m_c4FldId_State
                , tZWaveUSB3Sh::strLoadEDrvStates(m_eZWDrvState)
                , kCIDLib::True
            );
        }
    }
}



//
//  Sets our reported node info. We have to do this differently depending on whether we
//  are securely included or not.
//
tCIDLib::TVoid TZWaveUSB3SDriver::SetNodeInfo(const tCIDLib::TBoolean bSecureMode)
{
    //
    //  Set our node info, which the controller will send for us any time our
    //  node info frame is requested.
    //
    //  No reply expected, so just do a basic send.
    //
    TZWOutMsg zwomOut;
    zwomOut.Reset(FUNC_ID_SERIAL_API_APPL_NODE_INFORMATION, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
    zwomOut += 0x1; // Listening
    zwomOut += GENERIC_TYPE_STATIC_CONTROLLER;
    zwomOut += SPECIFIC_TYPE_PC_CONTROLLER;

    if (bSecureMode)
    {
        //
        //  Can't show manspec or version when in secure mode. We'll report manspec and
        //  naming via secure CCs report when secure, so we don't report them here.
        //
        zwomOut += 1;
        zwomOut += COMMAND_CLASS_SECURITY;
    }
     else
    {
        //
        //  Have to report zecurity when in non-secure mode or it won't add us securely.
        //  Manspec we can show until we get added securely, when it moves to the secure
        //  command supported CC class.
        //
        zwomOut += 2;
        zwomOut += COMMAND_CLASS_SECURITY;
        zwomOut += COMMAND_CLASS_MANUFACTURER_SPECIFIC;
    }
    zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
    USB3Send(zwomOut);

    // Broadcast our info out. Use a null callback since we are broadcasting
    zwomOut.Reset(FUNC_ID_ZW_SEND_NODE_INFORMATION, tZWaveUSB3Sh::EMsgPrios::SpecialCmd);
    zwomOut += NODE_BROADCAST;
    zwomOut.AppendTransOpts();
    zwomOut.AppendNullCallback();
    zwomOut.Finalize(kCIDLib::False, kCIDLib::False);
    USB3Send(zwomOut);

    //
    //  Sleep a little bit before doing anything else since we are not waiting for a callback
    //  transmission callback.
    //
    TThread::Sleep(2000);
}
