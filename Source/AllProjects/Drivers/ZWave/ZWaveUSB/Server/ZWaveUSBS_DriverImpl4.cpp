//
// FILE NAME: ZWaveUSBS_DriverImpl4.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2007
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
//  This file contains some overflow grunt work methods from the driver
//  implementation class. This one implements various methods that are
//  related to talking specific to modules, as apposed to talking to the
//  controller itself. These mostly all involve the SendData() method to
//  single or multiple units at at time. It also implements the polling
//  code.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
//
//  We have to bring in Z-Wave specific headers. The project setup for this
//  driver will set up the include path needed.
// ---------------------------------------------------------------------------
#include    "ZWaveUSBS_.hpp"


// ---------------------------------------------------------------------------
//  TZWaveUSBSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The classes that represent units have methods to build getter or setter
//  messages for sending to units. These methods are called to invoke those
//  methods and check the return value, and throw if the unit doesn't support
//  the requested message.
//
tCIDLib::TVoid
TZWaveUSBSDriver::BuildUnitGetMsg(          TMemBuf&                mbufToFill
                                    , const TZWaveUnit&             unitTar
                                    ,       tCIDLib::TCard1&        c1RetClass
                                    ,       tCIDLib::TCard1&        c1RetCmd
                                    ,       tCIDLib::TCard1&        c1RetLen
                                    , const tCIDLib::TCard1         c1Val1
                                    , const tCIDLib::TCard1         c1Val2
                                    , const tCIDLib::TCard1         c1CBId
                                    , const tZWaveUSBSh::EUGetCmds  eCmd) const
{
    const tCIDLib::TBoolean bRes = unitTar.bBuildGetMsg
    (
        mbufToFill, c1RetClass, c1RetCmd, c1RetLen, c1Val1, c1Val2, c1CBId, eCmd
    );

    if (!bRes)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcGen_UnhandledUGetMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , unitTar.strName()
            , tZWaveUSBSh::strLoadEUGetCmds(eCmd)
        );
    }
}

tCIDLib::TVoid
TZWaveUSBSDriver::BuildUnitSetMsg(          TMemBuf&                mbufToFill
                                    , const TZWaveUnit&             unitTar
                                    , const tCIDLib::TCard1         c1Val1
                                    , const tCIDLib::TCard1         c1Val2
                                    , const tCIDLib::TCard1         c1CBId
                                    , const tZWaveUSBSh::EUSetCmds  eCmd) const
{
    const tCIDLib::TBoolean bRes = unitTar.bBuildSetMsg
    (
        mbufToFill, c1Val1, c1Val2, c1CBId, eCmd
    );

    if (!bRes)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcGen_UnhandledUSetMsg
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotSupported
            , unitTar.strName()
            , tZWaveUSBSh::strLoadEUSetCmds(eCmd)
        );
    }
}


//
//  Set the level for the units in a group. Only those units of the group that
//  support multi-level operations are targeted.
//
tCIDLib::TVoid
TZWaveUSBSDriver::DoGroupLevel( const   TZWaveGroupInfo&    grpiTar
                                , const tCIDLib::TCard1     c1Lev)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        ZWLogDbgMsg2
        (
            L"Setting group level. Grp=%(1), Lev=%(2)"
            , grpiTar.strName()
            , TCardinal(c1Lev)
        );
    }

    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    //
    //  Ok, get the unit list for this group. It shouldn't be empty, but
    //  if in debug mode, check it.
    //
    const tZWaveUSBSh::TIdList& fcolToAdd = grpiTar.fcolUnits();
    const tCIDLib::TCard4 c4Count = fcolToAdd.c4ElemCount();

    mbufOut.PutCard1(0, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(FUNC_ID_ZW_SEND_DATA_MULTI, 2);

    // This is the count of targets, which we don't know yet
    mbufOut.PutCard1(0, 3);

    //
    //  Put in the target units. But only take the ones that support
    //  multi-level operations.
    //
    tCIDLib::TCard1 c1Cnt = 4;
    tCIDLib::TCard4 c4UnitsAdded = 0;
    for (tCIDLib::TCard4 c4UInd = 0; c4UInd < c4Count; c4UInd++)
    {
        const tCIDLib::TCard1 c1CurUnitId = fcolToAdd[c4UInd];
        const TZWaveUnit* const punitCur = m_dcfgCurrent.punitFindById(c1CurUnitId);
        if (!punitCur)
            continue;

        // If multi-level capable, take it and bump the count of units added
        if (punitCur->bHasCap(tZWaveUSBSh::EUnitCap_Level))
        {
            mbufOut.PutCard1(c1CurUnitId, c1Cnt++);
            c4UnitsAdded++;
        }
    }

    //
    //  We may not have had any multi-levels in this group. If not, then
    //  there is nothing to do.
    //
    if (c4UnitsAdded)
    {
        // Go back and update the count
        mbufOut.PutCard1(tCIDLib::TCard1(c4UnitsAdded), 3);

        mbufOut.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_SET_FRAME), c1Cnt++);
        mbufOut.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL, c1Cnt++);
        mbufOut.PutCard1(SWITCH_MULTILEVEL_SET, c1Cnt++);
        mbufOut.PutCard1(c1Lev, c1Cnt++);
        mbufOut.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
        mbufOut.PutCard1(c1NextCallbackId(), c1Cnt++);

        // Put the full length in now and send
        mbufOut.PutCard1(c1Cnt, 0);
        SendData(mbufOut);
    }
}


//
//  Set the on/off state for the units in a group. Only those units of the
//  group that support on/off operations are targeted.
//
tCIDLib::TVoid
TZWaveUSBSDriver::DoGroupOnOff( const   TZWaveGroupInfo&    grpiTar
                                , const tCIDLib::TBoolean   bToSet)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        ZWLogDbgMsg2
        (
            L"Setting group on/off. Grp=%(1), State=%(2)"
            , grpiTar.strName()
            , TCardinal(bToSet)
        );
    }

    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    //
    //  Ok, get the unit list for this group. It shouldn't be empty, but
    //  if in debug mode, check it.
    //
    const tZWaveUSBSh::TIdList& fcolToAdd = grpiTar.fcolUnits();
    const tCIDLib::TCard4 c4Count = fcolToAdd.c4ElemCount();

    mbufOut.PutCard1(0, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(FUNC_ID_ZW_SEND_DATA_MULTI, 2);

    // This is the count of targets, which we don't know yet
    mbufOut.PutCard1(0, 3);

    // Put in the target units
    tCIDLib::TCard4 c4UnitsAdded = 0;
    tCIDLib::TCard1 c1Cnt = 4;
    for (tCIDLib::TCard4 c4UInd = 0; c4UInd < c4Count; c4UInd++)
    {
        const tCIDLib::TCard1 c1CurUnitId = fcolToAdd[c4UInd];
        const TZWaveUnit* const punitCur = m_dcfgCurrent.punitFindById(c1CurUnitId);
        if (!punitCur)
            continue;

        // If on/off capable, take it and bump the count of units added
        if (punitCur->bHasCap(tZWaveUSBSh::EUnitCap_OnOff))
        {
            c4UnitsAdded++;
            mbufOut.PutCard1(fcolToAdd[c4UInd], c1Cnt++);
        }
    }
    //
    //  We may not have had any multi-levels in this group. If not, then
    //  there is nothing to do.
    //
    if (c4UnitsAdded)
    {
        // Go back and update the count
        mbufOut.PutCard1(tCIDLib::TCard1(c4UnitsAdded), 3);

        mbufOut.PutCard1(sizeof(_ZW_BASIC_SET_FRAME_), c1Cnt++);
        mbufOut.PutCard1(COMMAND_CLASS_BASIC, c1Cnt++);
        mbufOut.PutCard1(BASIC_SET, c1Cnt++);
        mbufOut.PutCard1(bToSet ? 0 : 0xFF, c1Cnt++);
        mbufOut.PutCard1(TRANSMIT_OPTION_ACK | TRANSMIT_OPTION_AUTO_ROUTE, c1Cnt++);
        mbufOut.PutCard1(c1NextCallbackId(), c1Cnt++);

        // Put the full length in now and send
        mbufOut.PutCard1(c1Cnt, 0);
        SendData(mbufOut);
    }
}


// Does a ramp up/down/stop command for a group
tCIDLib::TVoid
TZWaveUSBSDriver::DoGroupRamp(  const   TZWaveGroupInfo&    grpiTar
                                , const TString&            strOp)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        ZWLogDbgMsg2
        (
            L"Doing group ramp. Grp=%(1), Lev=%(2)", grpiTar.strName(), strOp
        );
    }

    // Get a buffer from the pool
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 64);
    TMemBuf& mbufOut = janPool.objGet();

    //
    //  Ok, get the unit list for this group. It shouldn't be empty, but
    //  if in debug mode, check it.
    //
    const tZWaveUSBSh::TIdList& fcolToAdd = grpiTar.fcolUnits();
    const tCIDLib::TCard4 c4Count = fcolToAdd.c4ElemCount();

    const tCIDLib::TBoolean bUp = (strOp == L"RAMPUP");
    const tCIDLib::TBoolean bStart = (strOp != L"RAMPEND");
    mbufOut.PutCard1(0, 0);
    mbufOut.PutCard1(REQUEST, 1);
    mbufOut.PutCard1(FUNC_ID_ZW_SEND_DATA_MULTI, 2);

    // We don't know the real count yet
    mbufOut.PutCard1(0, 3);

    // Put in the target units that are multi-level and keep a count of added
    tCIDLib::TCard1 c1Cnt = 4;
    tCIDLib::TCard4 c4Added = 0;
    for (tCIDLib::TCard4 c4UInd = 0; c4UInd < c4Count; c4UInd++)
    {
        const tCIDLib::TCard1 c1CurUnitId = fcolToAdd[c4UInd];
        const TZWaveUnit* const punitCur = m_dcfgCurrent.punitFindById(c1CurUnitId);
        if (!punitCur)
            continue;

        // If multi-level capable, take it and bump the count of units added
        if (punitCur->bHasCap(tZWaveUSBSh::EUnitCap_Level))
        {
            mbufOut.PutCard1(c1CurUnitId, c1Cnt++);
            c4Added++;
        }
    }

    // If we found any multi-levels, then lets do it
    if (c4Added)
    {
        // Update the count now that we know it
        mbufOut.PutCard1(tCIDLib::TCard1(c4Added), 3);

        if (bStart)
        {
            // No wrap, set 0x40 if down, don't if up
            mbufOut.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_START_LEVEL_CHANGE_FRAME), c1Cnt++);
            mbufOut.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL, c1Cnt++);
            mbufOut.PutCard1(SWITCH_MULTILEVEL_START_LEVEL_CHANGE, c1Cnt++);
            mbufOut.PutCard1(bUp ? 0 : 0x40, c1Cnt++);

            //
            //  It's supposed to ignore the start value based on flags above. But
            //  it doesn't. Since this is a group, we can't really get an existing
            //  value, so if ramping up, set it to 0, else set it to max.
            //
            if (bUp)
                mbufOut.PutCard1(0, c1Cnt++);
            else
                mbufOut.PutCar1(99, c1Cnt++);
        }
         else
        {
            mbufOut.PutCard1(sizeof(ZW_SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE_FRAME). c1Cnt++);
            mbufOut.PutCard1(COMMAND_CLASS_SWITCH_MULTILEVEL. c1Cnt++);
            mbufOut.PutCard1(SWITCH_MULTILEVEL_STOP_LEVEL_CHANGE. c1Cnt++);
        }
        mbufOut.PutCard1(TRANSMIT_OPTION_AUTO_ROUTE | TRANSMIT_OPTION_ACK, c1Cnt++);
        mbufOut.PutCard1(c1NextCallbackId(), c1Cnt++);

        // Put the full length in now and send
        mbufOut.PutCard1(c1Cnt, 0);
        SendData(mbufOut);
    }
}



//
//  This method is called from the poll callback. We just see if any units
//  need to be polled. We queue up any units that are ready to poll that
//  are not already on the poll queue.
//
//  We assume that the caller has confirmed that we are not in replication
//  mode and that it's safe to access the configuration data.
//
//  We let exceptions propogate back to the poll method.
//
tCIDLib::TVoid TZWaveUSBSDriver::DoPollCycle(TThread& thrCaller)
{
    // Get a buffer from the pool for reading
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 256);
    TMemBuf& mbufIn = janPool.objGet();

    //
    //  Process any waiting messages. So we just do a very short timeout,
    //  so we only get stuff that is already available.
    //
    while (kCIDLib::True)
    {
        tCIDLib::TCard4 c4Read;

        tCIDLib::TCard1 c1FuncId;
        tCIDLib::TCard1 c1CBId;
        tZWaveUSBS::EMsgTypes eRes = eGetReply
        (
            c4Read
            , mbufIn
            , kCIDLib::enctOneMilliSec * 10
            , c1FuncId
            , c1CBId
            , kCIDLib::False
        );

        if (eRes == tZWaveUSBS::EMsgType_Timeout)
            break;

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            ZWLogDbgMsg1(L"Got async msg. CmdId=%(1)", TCardinal(c1FuncId));

        if (eRes == tZWaveUSBS::EMsgType_CallBack)
            HandleCallBack(c1FuncId, mbufIn, c4Read);
        else if (eRes == tZWaveUSBS::EMsgType_Response)
            HandleResponse(c1FuncId, mbufIn, c4Read);
    }

    //
    //  Get the current time. We'll check each unit and see if it's next
    //  poll time is beyond this, which means it's ready to poll.
    //
    const tCIDLib::TEncodedTime enctCur = TTime::enctNow();

    // We need to loop through the configured units
    tZWaveUSBSh::TUnitList& colUnits = m_dcfgCurrent.colUnits();
    const tCIDLib::TCard4 c4UCount = colUnits.c4ElemCount();
    for (tCIDLib::TCard4 c4UInd = 0; c4UInd < c4UCount; c4UInd++)
    {
        TZWaveUnit* punitCur = colUnits[c4UInd];

        //
        //  If this guy is not pollable or is set up not be to polled
        //  even though it could be, then skip it.
        //
        if (!punitCur->bHasCap(tZWaveUSBSh::EUnitCap_Pollable)
        ||  !punitCur->enctPollPeriod())
        {
            continue;
        }

        if (punitCur->enctNextPoll() < enctCur)
        {
            //
            //  This one is ready, so if it's not already in the queue,
            //  add it.
            //
            m_colPollQ.bPutIfNew(punitCur);
        }
    }


    //
    //  Ok, now if the poll queue isn't empty, then we want to pull the
    //  next unit off and do a poll on it.
    //
    TZWaveUnit* punitPoll = m_colPollQ.pobjGetNext(0, kCIDLib::False);
    if (punitPoll)
        RequestReport(*punitPoll);
}


//
//  A helper for building up and sending out a unit set command, generally
//  in response to a field write from a client.
//
tCIDLib::TVoid
TZWaveUSBSDriver::DoUnitSet(const   TZWaveUnit&             unitTar
                            , const tZWaveUSBSh::EUSetCmds  eCmd
                            , const tCIDLib::TCard1         c1Value)
{
    TSimplePoolJan<THeapBuf> janPool(&m_splBufs, 128);
    TMemBuf& mbufOut = janPool.objGet();

    // Ask the unit to build us a command to send and send it out
    BuildUnitSetMsg
    (
        mbufOut, unitTar, c1Value, 0, c1NextCallbackId(), eCmd
    );
    SendData(mbufOut);
}


//
//  Called from the DoPollCycle() method above, when it's time for a unit
//  to be polled. We ask the unit for some values that we need to send out
//  the right type of request for the data that the target type of unit
//  returns. We then pass the report data back to the unit for it to process
//  and store the info.
//
tCIDLib::TVoid TZWaveUSBSDriver::RequestReport(TZWaveUnit& unitSrc)
{
    // Get some buffers for our use
    TSimplePoolJan<THeapBuf> janOut(&m_splBufs, 128);
    TMemBuf& mbufOut = janOut.objGet();
    TSimplePoolJan<THeapBuf> janIn(&m_splBufs, 512);
    TMemBuf& mbufIn = janIn.objGet();

    //
    //  Ask the unit to build the desired get message and tell us the
    //  command class type/cmd value he expects in return, as well as the
    //  expected return len.
    //
    tCIDLib::TCard1 c1RetClass;
    tCIDLib::TCard1 c1RetCmd;
    tCIDLib::TCard1 c1RetLen;
    BuildUnitGetMsg
    (
        mbufOut
        , unitSrc
        , c1RetClass
        , c1RetCmd
        , c1RetLen
        , 0
        , 0
        , c1NextCallbackId()
        , tZWaveUSBSh::EUGetCmd_GetReport
    );

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
        ZWLogDbgMsg1(L"Requesting report. Id=%(1)", TCardinal(unitSrc.c1Id()));

    //
    //  Tell this guy to store the 'lag time', which is the difference between
    //  now and its actual scheduled poll time.
    //
    unitSrc.StoreLagTime();
    try
    {
        // Send and wait for the replies
        SendData(mbufOut);

        // And now wait for the value to come back
        WaitForCmdCB
        (
            unitSrc.c1Id()
            , c1RetClass
            , c1RetCmd
            , c1RetLen
            , kCIDLib::enctOneSecond * 4
            , mbufIn
        );

        //
        //  Reset this guy's retries (since we got a good reply), reset its
        //  poll time for the next available time, and set our next avaliable
        //  send data time.
        //
        unitSrc.c4Retries(0);
        unitSrc.ResetPollTime();
        m_enctNextData = TTime::enctNow() + kZWaveUSBS::enctPollInt;

        //
        //  And give this unit the reply data to store. He will do a field
        //  lock if he stores field ata.
        //
        unitSrc.HandleAppCmd
        (
            mbufIn.pc1DataAt(6), mbufIn[5], *this
        );
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            ZWLogDbgMsg1(L"Max retries on unit %(1)", TCardinal(unitSrc.c1Id()));

        // Increment the retries. If it's hit 3, then put it into error state
        if (unitSrc.c4IncRetries() >= 3)
            unitSrc.SetErrorState(*this);
    }
}


//
//  Waits for an application command handler to come in for the indicated
//  unit id, with the indicated class type and class command and class data
//  bytes. Any others are processed aysnchronously.
//
//  Note that the c1Len parm refers to the class data, not to the overall
//  message length. The caller is waiting for a class frame payload and
//  any given such class frame has a specific size.
//
//  We have one that handles application command handler callbacks, which
//  will contain some sort of class command type of frame. And we have
//  one that handles just regular callbacks for a command.
//
tCIDLib::TVoid
TZWaveUSBSDriver::WaitForCmdCB( const   tCIDLib::TCard1         c1ExpUnitId
                                , const tCIDLib::TCard1         c1ExpClass
                                , const tCIDLib::TCard1         c1ExpFuncId
                                , const tCIDLib::TCard1         c1ExpLen
                                , const tCIDLib::TEncodedTime   enctWaitFor
                                ,       TMemBuf&                mbufToUse)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + enctWaitFor;

    // Wait for a report for the indicated unit to come in
    tCIDLib::TCard4 c4Read;
    TThread*        pthrCaller = 0;
    while (enctCur < enctEnd)
    {
        // Wait in up to one second chunks, check for shutdown requests
        tCIDLib::TEncodedTime enctCurWait;
        if (enctCur + kCIDLib::enctOneSecond > enctEnd)
            enctCurWait = enctEnd - enctCur;
        else
            enctCurWait = kCIDLib::enctOneSecond;

        tCIDLib::TCard1 c1FuncId;
        tCIDLib::TCard1 c1CBId;
        tZWaveUSBS::EMsgTypes eRes = eGetReply
        (
            c4Read
            , mbufToUse
            , enctCurWait
            , c1FuncId
            , c1CBId
            , kCIDLib::False
        );

        if (eRes == tZWaveUSBS::EMsgType_CallBack)
        {
            // If it was the one we care about, then break out
            if ((c1FuncId == FUNC_ID_APPLICATION_COMMAND_HANDLER)
            &&  (c4Read >= 8))
            {
                if ((mbufToUse[4] == c1ExpUnitId)
                &&  (mbufToUse[5] == c1ExpLen)
                &&  (mbufToUse[6] == c1ExpClass)
                &&  (mbufToUse[7] == c1ExpFuncId))
                {
                    break;
                }
            }

            //
            //  Not ours, so just process it and keep waiting. Bump the end
            //  time slightly to make up for the time we took up.
            //
            HandleCallBack(c1FuncId, mbufToUse, c4Read);
            enctEnd += (50 * kCIDLib::enctOneMilliSec);
        }
         else if (eRes == tZWaveUSBS::EMsgType_Response)
        {
            HandleResponse(c1FuncId, mbufToUse, c4Read);
        }

        // Check for a shutdown request. If so, force an error to get out
        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
        {
            enctCur = enctEnd;
            break;
        }

        enctCur = TTime::enctNow();
    }

    // If we timed out, then give up
    if (enctCur >= enctEnd)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_Timeout2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(FUNC_ID_APPLICATION_COMMAND_HANDLER, tCIDLib::ERadices::Hex)
        );
    }
}

tCIDLib::TVoid
TZWaveUSBSDriver::WaitForCmdCB( const   tCIDLib::TCard1         c1ExpFuncId
                                , const tCIDLib::TCard1         c1ExpCBId
                                , const tCIDLib::TEncodedTime   enctWaitFor
                                ,       TMemBuf&                mbufToUse)
{
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + enctWaitFor;

    // Wait for a report for the indicated unit to come in
    tCIDLib::TCard4 c4Read;
    TThread*        pthrCaller = 0;
    while (enctCur < enctEnd)
    {
        // Wait in up to one second chunks, check for shutdown requests
        tCIDLib::TEncodedTime enctCurWait;
        if (enctCur + kCIDLib::enctOneSecond > enctEnd)
            enctCurWait = enctEnd - enctCur;
        else
            enctCurWait = kCIDLib::enctOneSecond;

        // Tell him to throw on a timeout or failure
        tCIDLib::TCard1 c1FuncId;
        tCIDLib::TCard1 c1CBId;
        tZWaveUSBS::EMsgTypes eRes = eGetReply
        (
            c4Read
            , mbufToUse
            , enctCurWait
            , c1FuncId
            , c1CBId
            , kCIDLib::False
        );

        if (eRes == tZWaveUSBS::EMsgType_CallBack)
        {
            // If it was the one we care about, then break out
            if (c1FuncId == c1ExpFuncId)
            {
                // If the callback id isn't zero, then verify that
                if (!c1ExpCBId || (c1CBId == c1ExpCBId))
                    break;
            }

            //
            //  Not ours, so just process it and keep waiting. Bump the end
            //  time slightly to make up for the time we took up.
            //
            HandleCallBack(c1FuncId, mbufToUse, c4Read);
            enctEnd += (50 * kCIDLib::enctOneMilliSec);
        }
         else if (eRes == tZWaveUSBS::EMsgType_Response)
        {
            HandleResponse(c1FuncId, mbufToUse, c4Read);
        }

        // Check for a shutdown request. If so, force an error to get out
        if (!pthrCaller)
            pthrCaller = TThread::pthrCaller();
        if (pthrCaller->bCheckShutdownRequest())
        {
            enctCur = enctEnd;
            break;
        }

        enctCur = TTime::enctNow();
    }

    // If we timed out, then give up
    if (enctCur >= enctEnd)
    {
        facZWaveUSBS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcProto_Timeout2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Timeout
            , TCardinal(FUNC_ID_APPLICATION_COMMAND_HANDLER, tCIDLib::ERadices::Hex)
        );
    }
}


