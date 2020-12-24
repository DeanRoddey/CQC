//
// FILE NAME: MQTTS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2019
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
//  This is a driver which implements the MQTT 3.1.1 client driver
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MQTTS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMQTTS,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: TMQTTS
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMQTTS: Constructors and Destructor
// ---------------------------------------------------------------------------
TMQTTS::TMQTTS(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_ReloadCfg(kCIDLib::c4MaxCard)
    , m_colIOEvQ(tCIDLib::EMTStates::Safe)
    , m_evptrCur(m_splIOEvents.spptrReserveElem())
    , m_thrIO(this)
{
}

TMQTTS::~TMQTTS()
{
}


// ---------------------------------------------------------------------------
//  TMQTTS: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The I/O thread calls us here to queue up I/O events. The queue is thread safe
//  so we can just drop them in.
//
tCIDLib::TVoid TMQTTS::QueueIOEvent(TMQTTIOEvPtr& evptrToQ)
{
    if (!m_colIOEvQ.bIsFull(1024))
    {
        m_colIOEvQ.objAdd(evptrToQ);
    }
     else
    {
        if (facMQTTSh().bTraceMode())
        {
            facMQTTSh().LogTraceMsg
            (
                tMQTTSh::EMsgSrcs::Driver, L"Driver event queue is full"
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TMQTTS: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Watch for the next I/O state change in the event queue. If we get one, and
//  it is a move to connecting state, then we return success. There is no further
//  state to fall backwards to for us at this point. So we just wait for the
//  connecting state or beyond.
//
tCIDLib::TBoolean TMQTTS::bGetCommResource(TThread& thrThis)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (!bRet)
    {
        const tMQTTSh::EClStates eState = eWaitForIOStateChange();
        if (eState == tMQTTSh::EClStates::Count)
            break;

        bRet = (eState >= tMQTTSh::EClStates::Connecting)
                && (eState <= tMQTTSh::EClStates::Ready);
    }

    if (bRet && facMQTTSh().bTraceMode())
    {
        facMQTTSh().LogTraceMsg
        (
            tMQTTSh::EMsgSrcs::Driver, L"I/O thread has connected to server"
        );
    }
    return bRet;
}


//
//  The msg I/O thread loads the the config. We wait for him to post us an initialize
//  state, which means he has loaded the configuration. We can't really fall backwards
//  here so we only wait for a state that lets us move forward.
//
tCIDLib::TBoolean TMQTTS::bWaitConfig(TThread& thrThis)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    while (!bRet)
    {
        const tMQTTSh::EClStates eState = eWaitForIOStateChange();
        if (eState == tMQTTSh::EClStates::Count)
            break;

        bRet =  (eState >= tMQTTSh::EClStates::Initialize)
                && (eState <= tMQTTSh::EClStates::Ready);
    }
    if (bRet && facMQTTSh().bTraceMode())
    {
        facMQTTSh().LogTraceMsg
        (
            tMQTTSh::EMsgSrcs::Driver, L"I/O thread has loaded the configuration"
        );
    }
    return bRet;
}


//  Handle any boolean field writes
tCQCKit::ECommResults
TMQTTS::eBoolFldValChanged( const   TString&            strName
                            , const tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TBoolean   bNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;
    try
    {
        //
        //  First see if it is one of our own. Else we pass it on to the I/O
        //  to let him handle it.
        //
        if (c4FldId == m_c4FldId_ReloadCfg)
        {
            TMQTTDrvEvPtr evptrCfg = m_splDrvEvents.spptrReserveElem();
            evptrCfg->m_eEvent = tMQTTS::EDrvEvents::ReloadCfg;
            m_thrIO.bQueueDrvEvent(evptrCfg);
        }
         else
        {
            eRes = eQueueFldWrite(strName, facCQCKit().strBoolVal(bNewValue));
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}



// Handle any Card field writes
tCQCKit::ECommResults
TMQTTS::eCardFldValChanged( const   TString&        strName
                            , const tCIDLib::TCard4 c4FldId
                            , const tCIDLib::TCard4 c4NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    // WE don't have driver level ones, so pass it to the I/O thread
    try
    {
        TString strNewVal;
        strNewVal.AppendFormatted(c4NewValue);
        eRes = eQueueFldWrite(strName, strNewVal);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


//
//  We watch for I/O state change events from the I/O thread. If we see it go to
//  a state below connecting, or to disconnecting or beyond, we return lost
//  comm res.
//
//  Else we are just waiting for him to get to ready state at this point.
//
tCQCKit::ECommResults TMQTTS::eConnectToDevice(TThread& thrThis)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostConnection;
    while (kCIDLib::True)
    {
        const tMQTTSh::EClStates eState = eWaitForIOStateChange();
        if (eState == tMQTTSh::EClStates::Count)
            break;

        if ((eState < tMQTTSh::EClStates::Connecting)
        ||  (eState >= tMQTTSh::EClStates::Disconnecting))
        {
            // We have lost it, so return with the default lost connection
            break;
        }
         else if (eState == tMQTTSh::EClStates::Ready)
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceMsg
                (
                    tMQTTSh::EMsgSrcs::Driver
                    , L"I/O thread is in ready state, querying fields"
                );
            }

            //
            //  Note it's possible he could have lost connection again by the time
            //  we make the call. If so, he returns false and we just fall through.
            //  Else we set the fields.
            //
            tCQCKit::TFldDefList colFlds;
            if (m_thrIO.bQueryFldDefs(colFlds))
            {
                // Add some of our own
                TCQCFldDef flddTmp;
                flddTmp.Set
                (
                    L"ReloadCfg", tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write
                );
                colFlds.objAdd(flddTmp);

                SetFields(colFlds);
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceMsg
                    (
                        tMQTTSh::EMsgSrcs::Driver, L"Fields were successfully set"
                    );
                }

                // Make sure they are all in error until proven otherwise
                SetAllErrStates();

                // Look up any of our own
                m_c4FldId_ReloadCfg = pflddFind(L"ReloadCfg")->c4Id();
            }
            eRes = tCQCKit::ECommResults::Success;
            break;
        }
    }
    return eRes;
}


// Handle any Float field writes
tCQCKit::ECommResults
TMQTTS::eFloatFldValChanged(const   TString&            strName
                            , const tCIDLib::TCard4     c4FldId
                            , const tCIDLib::TFloat8    f8NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    // WE don't have driver level ones, so pass it to the I/O thread
    try
    {
        TString strNewVal;
        strNewVal.AppendFormatted(f8NewValue, 5);
        eRes = eQueueFldWrite(strName, strNewVal);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCQCKit::EDrvInitRes TMQTTS::eInitializeImpl()
{
    //
    //  We want to poll quickly, because we get queued up reports from the I/O
    //  thread that we need to process. The reconnect time isn't really us since
    //  the I/O thread does that. But we want to report it periodically as we
    //  try to connect.
    //
    SetPollTimes(50, 5000);

    // Start up he I/O thread
    m_thrIO.Start();

    // And next we have to wait for the I/O thread to load the configuration
    return tCQCKit::EDrvInitRes::WaitConfig;
}


//  Handle any Int field writes
tCQCKit::ECommResults
TMQTTS::eIntFldValChanged(  const   TString&        strName
                            , const tCIDLib::TCard4 c4FldId
                            , const tCIDLib::TInt4  i4NewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    // WE don't have driver level ones, so pass it to the I/O thread
    try
    {
        TString strNewVal;
        strNewVal.AppendFormatted(i4NewValue);
        eRes = eQueueFldWrite(strName, strNewVal);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


//
//  We just need to grab any reports from the I/O thread out of the I/O queue,
//  and process them as needed.
//
tCQCKit::ECommResults TMQTTS::ePollDevice(TThread& thrThis)
{
    return eProcessIOEvents(10);
}


//
//  The I/O thread manages the connection, so we don't do anything here. It will
//  only get stopped when Terminate() is called.
//
tCIDLib::TVoid TMQTTS::ReleaseCommResource()
{
}


//
//  Handle any String field writes
//
tCQCKit::ECommResults
TMQTTS::eStringFldValChanged(const  TString&        strName
                            , const tCIDLib::TCard4 c4FldId
                            , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    // WE don't have writable driver level string fields, so pass it to the I/O thread
    try
    {
        //
        //  Now we'll wait for a while for it to post back either a failure
        //  or success or status change. We process other events while we
        //  wait.
        //
        eRes = eQueueFldWrite(strName, strNewValue);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


//
//  Just in case, if the I/O thread is still running, then stop it. It should
//  get stopped when the comm resource is released, but since it's a separate
//  thread we need to be extra careful.
//
tCIDLib::TVoid TMQTTS::TerminateImpl()
{
    ShutdownIO();

    // Make sure we release any final event
    m_evptrCur.Release();

    // And flush any events from the I/O event queue
    m_colIOEvQ.RemoveAll();
}


// Update the shared facility when our verbosity level changes
tCIDLib::TVoid TMQTTS::VerboseModeChanged(const tCQCKit::EVerboseLvls eNewState)
{
    facMQTTSh().eVerbosity(eNewState);
}


// ---------------------------------------------------------------------------
//  TMQTTS: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called from field writes. We post a field write to the I/O thread. Then
//  we wait for him to send us back a success, failure, or of course we might get a
//  status change.
//
//  If the return is success, then we got one of the field write replies. Else it
//  is the comm result appropriate for the I/O state change.
//
//  If we get one for a field other than the one passed, then we ignore it since it
//  will be one we already timed out on.
//
tCQCKit::ECommResults
TMQTTS::eQueueFldWrite( const   TString&        strFldName
                        , const TString&        strNewValue)
{
    // Get a driver event from the pool, set it up and queue it ojn the I/OP thread
    TMQTTDrvEvPtr evptrWrite = m_splDrvEvents.spptrReserveElem();
    evptrWrite->m_eEvent = tMQTTS::EDrvEvents::FieldWrite;
    evptrWrite->m_strValue = strNewValue;
    evptrWrite->m_strFldName = strFldName;
    m_thrIO.bQueueDrvEvent(evptrWrite);

    // And wait for the result, for up to 4 seconds
    tCQCKit::ECommResults eRet = tCQCKit::ECommResults::Success;
    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusSecs(4);
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    TThread& thrMe = *TThread::pthrCaller();
    while ((enctNow < enctEnd) && !thrMe.bCheckShutdownRequest())
    {
        // Calc the remaining MS, and clip back to a half second for each round max
        tCIDLib::TCard4 c4MSLeft
        (
            tCIDLib::TCard4((enctEnd - enctNow) / kCIDLib::enctOneMilliSec)
        );
        if (c4MSLeft > 500)
            c4MSLeft = 500;

        if (m_colIOEvQ.bGetNext(m_evptrCur, c4MSLeft, kCIDLib::False))
        {
            if ((m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FldError)
            ||  (m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FldValue))
            {
                StoreFldVal(m_evptrCur);
            }
             else if (m_evptrCur->m_eEvent == tMQTTS::EIOEvents::StateChange)
            {
                //
                //  If we are in connected state and he transitions to a non-ready state
                //  then we need to recycle.
                //
                if (m_evptrCur->m_eIOState != tMQTTSh::EClStates::Ready)
                {
                    eRet = tCQCKit::ECommResults::LostCommRes;
                    break;
                }
            }
             else if ((m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FieldWrite)
                  &&  (m_evptrCur->m_strFldName == strFldName))
            {
                // It's what we've all been waiting for, just give back his reported result
                eRet = m_evptrCur->m_eCommRes;
                break;
            }
        }
        enctNow = TTime::enctNow();
    }

    // We either got a state change or the response we wanted
    return eRet;
}


//
//  This is called from our poll callback, to grab any incoming I/O events and
//  react to them. Mostly we expect field related events, but we can also get
//  an I/O thread state change back to something prior to ready state. If that
//  happens, we will recycle the connection.
//
tCQCKit::ECommResults TMQTTS::eProcessIOEvents(const tCIDLib::TCard4 c4WaitMS)
{
    tCQCKit::ECommResults eRet = tCQCKit::ECommResults::Success;

    const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusMSs(c4WaitMS);
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();
    TThread& thrMe = *TThread::pthrCaller();
    while ((enctNow < enctEnd) && !thrMe.bCheckShutdownRequest())
    {
        // Calc the remaining MS
        const tCIDLib::TCard4 c4MSLeft
        (
            tCIDLib::TCard4((enctEnd - enctNow) / kCIDLib::enctOneMilliSec)
        );
        if (m_colIOEvQ.bGetNext(m_evptrCur, c4MSLeft, kCIDLib::False))
        {
            if ((m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FldError)
            ||  (m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FldValue))
            {
                StoreFldVal(m_evptrCur);
            }
             else if (m_evptrCur->m_eEvent == tMQTTS::EIOEvents::StateChange)
            {
                //
                //  If we are in connected state and he transitions to a non-ready state
                //  then we need to recycle.
                //
                if ((eState() == tCQCKit::EDrvStates::Connected)
                &&  (m_evptrCur->m_eIOState != tMQTTSh::EClStates::Ready))
                {
                    eRet = tCQCKit::ECommResults::LostCommRes;
                    break;
                }
            }
             else if (m_evptrCur->m_eEvent == tMQTTS::EIOEvents::FieldWrite)
            {
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceMsg
                    (
                        tMQTTSh::EMsgSrcs::Driver
                        , L"Got orphaned field write result for field '%(1)'"
                        , m_evptrCur->m_strFldName
                    );
                }
            }
        }
        enctNow = TTime::enctNow();
    }
    return eRet;
}



//
//  Once we are up and ready, eProcessIOEvents() is called. But, during the
//  startup phase we have to watch for state change events to tell us when we can
//  move forward (or have to fall back.) We do a small wait, just so we don't
//  unnecessarily go back around again and have to come back.
//
tMQTTSh::EClStates TMQTTS::eWaitForIOStateChange()
{
    if (!m_colIOEvQ.bIsEmpty())
    {
        tCIDLib::TCard4 c4Count = 0;
        TMQTTIOEvPtr evptrNew = m_splIOEvents.spptrReserveElem();
        while (m_colIOEvQ.bGetNext(evptrNew, 50, kCIDLib::False) && (c4Count < 32))
        {
            c4Count++;
            if (evptrNew->m_eEvent == tMQTTS::EIOEvents::StateChange)
            {
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceMsg
                    (
                        tMQTTSh::EMsgSrcs::Driver
                        , L"I/O thread reported new state %(1)"
                        , tMQTTSh::strLoadEClStates(evptrNew->m_eIOState)
                    );
                }
                return evptrNew->m_eIOState;
            }
        }
    }
    return tMQTTSh::EClStates::Count;
}


//
//  When we get a publish event from the I/O thread, queues up an event for us to
//  process. StoreFldVal() below gets called. If it stores the value successfully,
//  and the field is V2 compatible, it calls us here to see if we need to send out
//  a standard event trigger.
//
tCIDLib::TVoid TMQTTS::SendEvTrigger(const TMQTTIOEvent& ioevSrc)
{
    // So far we only have these, others may be supported later
    if ((ioevSrc.m_eSemType == tCQCKit::EFldSTypes::MotionSensor)
    ||  (ioevSrc.m_eSemType == tCQCKit::EFldSTypes::LightSwitch))
    {
        // We have to convert the value string to a boolean
        tCIDLib::TBoolean bVal = facCQCKit().bCheckBoolVal(ioevSrc.m_strValue);

        // Get the two bits that are different between these
        tCQCKit::EStdDrvEvs eEvent;
        TString strEvVal;
        if (ioevSrc.m_eSemType == tCQCKit::EFldSTypes::MotionSensor)
        {
            eEvent = tCQCKit::EStdDrvEvs::Motion;
            strEvVal = facCQCKit().strBoolStartEnd(bVal);
        }
         else
        {
            eEvent = tCQCKit::EStdDrvEvs::LoadChange;
            strEvVal = facCQCKit().strBoolOffOn(bVal);
        }

        QueueEventTrig
        (
            eEvent
            , ioevSrc.m_strFldName
            , strEvVal
            , TString::strEmpty()
            , ioevSrc.m_strBaseName
            , TString::strEmpty()
        );
    }
}


//
//  We need to do this from more than one place, so break it out here. We need to
//  get the I/O thread shutdown. We need to flush the I/O queue, and do any other
//  cleanup required.
//
tCIDLib::TVoid TMQTTS::ShutdownIO() noexcept
{

    if (m_thrIO.bIsRunning())
    {
        try
        {
            m_thrIO.ReqShutdownSync(5000);
            m_thrIO.eWaitForDeath(3000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  This is called when we get an I/O event with a new field value. We need to
//  store the value away, if it is valid.
//
tCIDLib::TVoid TMQTTS::StoreFldVal(const TMQTTIOEvPtr& evptrStore)
{
    try
    {
        tCIDLib::TCard4     c4FldId;
        tCQCKit::EFldTypes  eType;
        const tCIDLib::TBoolean bFound = bQueryFldId
        (
            evptrStore->m_strFldName, c4FldId, eType, kCIDLib::False
        );

        //
        //  If not found, or if the type doesn't match the type reported, something
        //  is wrong.
        //
        if (!bFound)
        {
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::Driver
                    , L"Got an I/O thread event for unknown field %(1)"
                    , evptrStore->m_strFldName
                );
            }
        }
         else if (evptrStore->m_eFldType != eType)
        {
            // Set the field to error state
            SetFieldErr(c4FldId, kCIDLib::True);
            if (facMQTTSh().bTraceMode())
            {
                facMQTTSh().LogTraceErr
                (
                    tMQTTSh::EMsgSrcs::Driver
                    , L"I/O thread reported an invalid type for field %(1)"
                    , evptrStore->m_strFldName
                );
            }
        }
         else
        {
            // If it's an error event, just set the error , else store the value
            if (evptrStore->m_eEvent == tMQTTS::EIOEvents::FldError)
            {
                SetFieldErr(c4FldId, kCIDLib::True);
                if (facMQTTSh().bTraceMode())
                {
                    facMQTTSh().LogTraceErr
                    (
                        tMQTTSh::EMsgSrcs::Driver
                        , L"I/O thread reported error state for field %(1)"
                        , evptrStore->m_strFldName
                    );
                }
            }
             else
            {
                try
                {
                    if (facMQTTSh().bTraceMode())
                    {
                        facMQTTSh().LogTraceMsg
                        (
                            tMQTTSh::EMsgSrcs::Driver
                            , L"I/O thread reported a new value for field %(1)"
                            , evptrStore->m_strFldName
                        );
                    }

                    WriteField
                    (
                        c4FieldListId(), c4FldId, evptrStore->m_strValue, kCIDLib::True
                    );

                    if (facMQTTSh().bTraceMode())
                    {
                        facMQTTSh().LogTraceMsg
                        (
                            tMQTTSh::EMsgSrcs::Driver
                            , L"Field %(1) was updated successful"
                            , evptrStore->m_strFldName
                        );
                    }

                    //
                    //  If it is a V2 compatible field, we may need to send out an
                    //  event trigger.
                    //
                    if (evptrStore->m_bV2Compat)
                        SendEvTrigger(*evptrStore);
                }

                catch(TError& errToCatch)
                {
                    if (facMQTTSh().bTraceMode())
                    {
                        facMQTTSh().LogTraceErr
                        (
                            tMQTTSh::EMsgSrcs::Driver
                            , L"An exception occurred while handling new value for field %(1)"
                            , evptrStore->m_strFldName
                        );
                    }

                    // Assume a bad value and set the field into error state
                    SetFieldErr(c4FldId, kCIDLib::True);
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}
