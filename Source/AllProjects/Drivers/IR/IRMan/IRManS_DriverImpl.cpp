//
// FILE NAME: IRManS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2002
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
//  This file implements the bulk of the driver implementation. Some of the
//  functionality provided here is accessed via server side ORB objects that
//  we register, and which the generic IR client side driver uses to talk
//  to us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "IRManS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIRManSDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace IRManS_DriverImpl
{
    // The number of bytes in each incoming record
    const tCIDLib::TCard4   c4KeyBytes = 6;

    //
    //  The minimum inter-event time. The IRMan will send more than one
    //  event per button, so we have to eat all but the first.
    //
    const tCIDLib::TCard4   c4MinInterval = 250;
}



// ---------------------------------------------------------------------------
//   CLASS: THostMonSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TIRManSDriver::TIRManSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bTrainingMode(kCIDLib::False)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_c4NextEventTime(0)
    , m_c4ResetCount(0)
    , m_pcommIRMan(0)
    , m_strPort()
{
}


TIRManSDriver::~TIRManSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommIRMan)
    {
        try
        {
            if (m_pcommIRMan->bIsOpen())
                m_pcommIRMan->Close();

            delete m_pcommIRMan;
            m_pcommIRMan= 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TIRManSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIRManSDriver::bCheckRecTrainingData(TString& strKeyToFill)
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Store the current value, then clear the member. Return whether it
    //  had a value or not.
    //
    const tCIDLib::TBoolean bRet = !m_strTrainVal.bIsEmpty();

    if (bRet)
    {
        strKeyToFill = m_strTrainVal;
        m_strTrainVal.Clear();
    }
    return bRet;
}


tCIDLib::TBoolean
TIRManSDriver::bGetCommResource(TThread& thrThis)
{
    // Open the port now, and set the configuration
    try
    {
        // Create the port object if not yet, clsoe if open just in case
        if (!m_pcommIRMan)
            m_pcommIRMan = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommIRMan->bIsOpen())
            m_pcommIRMan->Close();

        m_pcommIRMan->Open();
        m_pcommIRMan->SetCfg(m_cpcfgSerial);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  We don't check online here, since it's intended to try to get us back
//  online if possible.
//
tCIDLib::TBoolean TIRManSDriver::bResetConnection()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    // Force it out of training mode
    m_bTrainingMode = kCIDLib::False;
    ClearEventQ();

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);

    // Call a helper method that does this
    return bCycleConn();
}


tCIDLib::TVoid TIRManSDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);
    m_strTrainVal.Clear();
}


tCQCKit::ECommResults
TIRManSDriver::eConnectToDevice(TThread& thrThis)
{
    //
    //  If we cannot cycle the connection, then it's not there so fail
    //  the attempt.
    //
    if (!bCycleConn())
        return tCQCKit::ECommResults::LostConnection;

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TIRManSDriver::eInitializeImpl()
{
    // Call our parent IR base drivers
    TParent::eInitializeImpl();

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
    m_strPort = conncfgSer.strPortPath();
    m_cpcfgSerial = conncfgSer.cpcfgSerial();

    // Clean up any existing port so it'll get recreated with new config
    delete m_pcommIRMan;
    m_pcommIRMan = 0;

    //
    //  Register our couple of fields. We just provide one that indicates
    //  the training mode state, so that the client can display an
    //  indicator that shows whether the driver is in training mode or not.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddCmd;

    //
    //  Create the fields that we need for a receiver only type of
    //  IR driver.
    //
    flddCmd.Set
    (
        TFacCQCIR::strFldName_FirmwareVer
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_TrainingState
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the ids of our fields, for efficiency
    m_c4FldIdFirmwareVer = pflddFind
    (
        TFacCQCIR::strFldName_FirmwareVer
        , kCIDLib::True
    )->c4Id();

    m_c4FldIdTrainingMode = pflddFind
    (
        TFacCQCIR::strFldName_TrainingState
        , kCIDLib::True
    )->c4Id();

    //
    //  Set the poll time a little faster than normal, since all we are doing
    //  is listening for data coming in. We'll set 100ms for polls, and 5
    //  seconds for reconnection.
    //
    SetPollTimes(100, 5000);

    //
    //  Crank up the actions processing thread if it's not already. It runs
    //  until we are unloaded pulling events out of the queue and processing
    //  them.
    //
    StartActionsThread();

    //
    //  In our case we want to go to 'wait for config' mode, not wait for
    //  comm res, since we need to get configuration before we can go online.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TIRManSDriver::ePollDevice(TThread&)
{
    //
    //  See if we get some data. We first just check for a byte. If we get
    //  one, then we try to read the rest of the packet. We use a fast timeout
    //  since its just a check to see if something is already there to read.
    //
    tCQCKit::ECommResults   eRes = tCQCKit::ECommResults::Success;
    TString                 strKey;
    if (bGetMsg(strKey, 10))
    {
        // Since we got something, reset the reset counter
        m_c4ResetCount = 0;

        //
        //  See if we have reached the next event time, and if not, we just
        //  eat it. If we have, reset the next event time for the next round
        //  and process it.
        //
        const tCIDLib::TCard4 c4CurTime = TTime::c4Millis();
        if (c4CurTime >= m_c4NextEventTime)
        {
            m_c4NextEventTime = c4CurTime + IRManS_DriverImpl::c4MinInterval;

            //
            //  Ok, we have a key. If we are in training mode, then just
            //  store it in the m_strTrainVal member. Else, we need to queue
            //  it on the action queue.
            //
            if (m_bTrainingMode)
                m_strTrainVal = strKey;
            else
                QueueRecEvent(strKey);
        }

        //
        //  While there are events still waiting, eat them, since they
        //  will be the subsequent events from the same button press.
        //
        while (bGetMsg(strKey, 100));
    }
     else
    {
        //
        //  We are on a quarter second poll period, so every 60 seconds,
        //  240 times through, without any input from the user, reset the
        //  connection to make sure it's still there.
        //
        m_c4ResetCount++;
        if (m_c4ResetCount > 240)
        {
            m_c4ResetCount = 0;
            if (!bCycleConn())
                eRes = tCQCKit::ECommResults::LostConnection;
        }
    }
    return eRes;
}


tCIDLib::TVoid TIRManSDriver::EnterRecTrainingMode()
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // We can't already be in training mode
    if (m_bTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker()
        );
    }

    //
    //  We make them wait until all the queued events are processed before
    //  they can do any training. It will only wait for a while and then
    //  throw if the queue doesn't empty.
    //
    WaitForActions();

    // Ok, go and and set the training mode flag and the field
    m_bTrainingMode = kCIDLib::True;

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TIRManSDriver::ExitRecTrainingMode()
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    if (!m_bTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_NotTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , strMoniker()
        );
    }
    m_bTrainingMode = kCIDLib::False;

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TIRManSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommIRMan->bIsOpen())
            m_pcommIRMan->Close();

        delete m_pcommIRMan;
        m_pcommIRMan= 0;
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid TIRManSDriver::TerminateImpl()
{

    // Call our parent class next
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TIRManSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TIRManSDriver::bCycleConn()
{
    //
    //  Cycle the control lines, with a small pause in between, so that the
    //  IRMan will reset and send us an ack.
    //
    m_pcommIRMan->SetDTR(tCIDComm::EPortDTR::Disable);
    m_pcommIRMan->SetRTS(tCIDComm::EPortRTS::Disable);
    TThread::Sleep(50);
    m_pcommIRMan->SetDTR(tCIDComm::EPortDTR::Enable);
    m_pcommIRMan->SetRTS(tCIDComm::EPortRTS::Enable);

    // Give the line some time to settle, then flush the garbage off the line
    TThread::Sleep(50);
    m_pcommIRMan->PurgeReadData();

    // And now send it "IR" with a pause in between
    static const tCIDLib::TSCh* const pchInit = "IR";
    m_pcommIRMan->c4WriteRawBufMS(&pchInit[0], 1, 50);
    TThread::Sleep(20);
    m_pcommIRMan->c4WriteRawBufMS(&pchInit[1], 1, 50);

    // And now wait for the ack, which is 2 bytes "OK"
    tCIDLib::TCard1 ac1Ack[2];
    if (m_pcommIRMan->c4ReadRawBufMS(ac1Ack, 2, 100) != 2)
        return kCIDLib::False;

    // Give it a bit to recover, then return status
    TThread::Sleep(100);
    return ((ac1Ack[0] == 0x4F) && (ac1Ack[1] == 0x4B));
}


tCIDLib::TBoolean
TIRManSDriver::bGetMsg(         TString&        strToFill
                        , const tCIDLib::TCard4 c4ToWait)
{
    //
    //  See if we get some data. We first just check for a byte. If we get
    //  one, then we try to read the rest of the packet. We use a fast
    //  timeout since its just a check to see if something is already there
    //  to read.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    tCIDLib::TCard1 ac1Buf[IRManS_DriverImpl::c4KeyBytes];
    if (m_pcommIRMan->c4ReadRawBufMS(ac1Buf, 1, c4ToWait))
    {
        // Reset the reset count, since we know it's alive now
        m_c4ResetCount = 0;

        // Try to read the rest of it
        const tCIDLib::TCard4 c4Read = m_pcommIRMan->c4ReadRawBufMS
        (
            &ac1Buf[1], IRManS_DriverImpl::c4KeyBytes - 1, 250
        );

        if (c4Read == IRManS_DriverImpl::c4KeyBytes - 1)
        {
            // Format it the caller's string buffer
            FormatKey(ac1Buf, strToFill);
            bRet = kCIDLib::True;
        }
         else
        {
            if (c4Read)
                IncBadMsgCounter();
            else
                IncTimeoutCounter();

            //
            //  This isn't good. We may have gotten out of sync, so pause
            //  for a bit a then purge the input buffer.
            //
            TThread::Sleep(100);
            m_pcommIRMan->PurgeReadData();
        }
    }
    return bRet;
}


tCIDLib::TVoid
TIRManSDriver::FormatKey(const  tCIDLib::TCard1* const  pac1Buf
                        ,       TString&                strToFill) const
{
    strToFill.Clear();

    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < IRManS_DriverImpl::c4KeyBytes; c4Index++)
    {
        const tCIDLib::TCard4 c4Cur = pac1Buf[c4Index];

        if (c4Cur < 0x10)
            strToFill.Append(L'0');
        strToFill.AppendFormatted(c4Cur, tCIDLib::ERadices::Hex);
    }
}


