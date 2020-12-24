//
// FILE NAME: R2DIS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/08/2005
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
//  This file implements the bulk of the driver implementation.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "R2DIS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TR2DIDriver,TBaseIRSrvDriver)


// ---------------------------------------------------------------------------
//   CLASS: TR2DIDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TR2DIDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TR2DIDriver::TR2DIDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TBaseIRSrvDriver(cqcdcToLoad)
    , m_bGotRecTrainingData(kCIDLib::False)
    , m_bRecTrainingMode(kCIDLib::False)
    , m_c4BlasterZoneCnt(kR2DIS::c4ZoneCount)
    , m_c4FldIdFirmwareVer(kCIDLib::c4MaxCard)
    , m_c4FldIdInvoke(kCIDLib::c4MaxCard)
    , m_c4FldIdTrainingMode(kCIDLib::c4MaxCard)
    , m_c4SensorZoneCnt(0)
    , m_enctNextActivePoll(0)
    , m_pcommR2DI(0)
    , m_strMsgAckReply(L"ACK")
    , m_strMsgBlastPref(L"AIRX")
    , m_strMsgErrPref(L"Error")
    , m_strMsgPref(L"AIR")
    , m_strMsgSensorStatePref(L"AIRP")
    , m_strMsgTrainDataPref(L"AIRD")
{
}

TR2DIDriver::~TR2DIDriver()
{
    // Clean up the comm port if not already
    if (m_pcommR2DI)
    {
        try
        {
            if (m_pcommR2DI->bIsOpen())
                m_pcommR2DI->Close();

            delete m_pcommR2DI;
            m_pcommR2DI = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TR2DIDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TR2DIDriver::bCheckRecTrainingData(TString& strKeyToFill)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // Lock and see if the training data has arrived yet
    if (m_bGotRecTrainingData)
    {
        // We got something, so copy it over and clear the flags
        strKeyToFill = m_strRecTrainData;
        m_bRecTrainingMode = kCIDLib::False;
        m_bGotRecTrainingData = kCIDLib::False;

        bStoreBoolFld(m_c4FldIdTrainingMode, kCIDLib::False, kCIDLib::True);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TR2DIDriver::bCvtManualBlastData(const  TString&            strText
                                ,       tCIDLib::TCard4&    c4DataBytes
                                ,       THeapBuf&           mbufToFill
                                ,       TString&            strError)
{
    //
    //  Use a regular expression to validate the data. It might be a Pronto
    //  code, or it it might be our own format. So ask the CQCIR facility
    //  to check it for us.
    //
    TString strActual;
    if (facCQCIR().bIsValidUIRTBlastData(strText))
    {
        // Convert to our format
        TTextStringOutStream strmCvt(&strActual);
        facCQCIR().ProntoToR2DI(strText, strmCvt);
    }
     else
    {
        if (!facCQCIR().bIsValidR2DIBlastData(strText))
        {
            strError.LoadFromMsg
            (
                kIRErrs::errcFmt_BadIRDataFormat, facCQCIR(), TString(L"R2DI")
            );
            return kCIDLib::False;
        }
        strActual = strText;
    }

    //
    //  The flattened format is just the single byte version of the text,
    //  so we just do a cheap transcode fo the characters to short chars via
    //  truncation.
    //
    const tCIDLib::TCard4 c4Len = strActual.c4Length();
    CIDAssert(c4Len != 0, L"The incoming IR data length was zero");
    for (c4DataBytes = 0; c4DataBytes< c4Len; c4DataBytes++)
        mbufToFill.PutCard1(tCIDLib::TCard1(strActual[c4DataBytes]), c4DataBytes);

    return kCIDLib::True;
}


tCIDLib::TBoolean TR2DIDriver::bGetCommResource(TThread& thrThis)
{
    // Open the port now, and set the configuration
    try
    {
        // Create the port object if not yet
        if (!m_pcommR2DI)
            m_pcommR2DI = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommR2DI->bIsOpen())
            m_pcommR2DI->Close();

        m_pcommR2DI->Open();
        m_pcommR2DI->SetCfg(m_cpcfgSerial);
    }

    catch(const TError&)
    {
        // <TBD> Should we log something here?
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TR2DIDriver::bRecTrainingMode() const
{
    // No need to lock for this simple check
    return m_bRecTrainingMode;
}


tCIDLib::TBoolean TR2DIDriver::bResetConnection()
{
    // A no-op for us
    return kCIDLib::True;
}


tCIDLib::TCard4 TR2DIDriver::c4ZoneCount() const
{
    return m_c4BlasterZoneCnt;
}


tCIDLib::TCard4 TR2DIDriver::c4InvokeFldId() const
{
    return m_c4FldIdInvoke;
}


tCIDLib::TVoid TR2DIDriver::ClearRecTrainingData()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);
    m_bGotRecTrainingData = kCIDLib::False;
    m_strRecTrainData.Clear();
}


tCQCKit::ECommResults
TR2DIDriver::eBoolFldValChanged(const   TString&            strName
                                , const tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TBoolean   bNewValue)
{
    // Don't know what it is
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


//
//  To connect we need to be able to set up the blaster/sensor zone configuration
//  that we got during init, and get any sensor values stored away.
//
tCQCKit::ECommResults TR2DIDriver::eConnectToDevice(TThread& thrThis)
{
    try
    {
        TString strMsg;

        // Set the termination style to CR only
        const TString& strSetTermCh(L"AIRL01");
        SendMsg(strSetTermCh);
        WaitForAckOrError(strSetTermCh, strMsg, 2500);

        //
        //  First thing we try is to just get the device version info and
        //  store it away. That's a simple query to use as a connection
        //  attempt and we have a field for that we need to get set up
        //  anyway.
        //
        SendMsg(L"AIRV?");
        bGetMessage(strMsg, 2500, kCIDLib::True);
        bStoreStringFld(m_c4FldIdFirmwareVer, strMsg, kCIDLib::True);

        //
        //  Set up the blaster/sensor zone config in the device. We send
        //  a 1 for each sensor and a 0 for each blaster.
        //
        strMsg = L"AIRP000";
        tCIDLib::TCard4 c4Mask = 0;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kR2DIS::c4ZoneCount; c4Index++)
        {
            if (m_abZoneTypes[c4Index])
                c4Mask |= (0x1 << c4Index);
        }
        strMsg.AppendFormatted(c4Mask, tCIDLib::ERadices::Hex);
        SendAndWaitForAck(strMsg, 2000);

        // And poll any sensors to get initial values
        PollSensors(strMsg);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostCommRes;
    }

    return tCQCKit::ECommResults::Success;
}


//
//  We get the zone configuration info from the prompt values in the driver
//  config and create the appropriate fields and register them.
//
tCQCKit::EDrvInitRes TR2DIDriver::eInitializeImpl()
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

    // Delete any existing port so it'll get recreated
    delete m_pcommR2DI;
    m_pcommR2DI = 0;

    // Initialize the zone info to default to all blasters
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kR2DIS::c4ZoneCount; c4Index++)
    {
        m_abZoneTypes[c4Index] = kCIDLib::False;
        m_ac4FldIdSensors[c4Index] = kCIDLib::c4MaxCard;
    }

    tCIDLib::TCard4 c4PCount = 0;
    TString strTmp;
    m_c4BlasterZoneCnt = kR2DIS::c4ZoneCount;
    m_c4SensorZoneCnt = 0;
    if (cqcdcOurs.bFindPromptValue(L"Z1Type", L"Selected", strTmp))
    {
        c4PCount++;
        if (strTmp == kR2DIS::pszZType_Sensor)
        {
            m_c4BlasterZoneCnt--;
            m_c4SensorZoneCnt++;
            m_abZoneTypes[0] = kCIDLib::True;
        }
    }

    if (cqcdcOurs.bFindPromptValue(L"Z2Type", L"Selected", strTmp))
    {
        c4PCount++;
        if (strTmp == kR2DIS::pszZType_Sensor)
        {
            m_c4BlasterZoneCnt--;
            m_c4SensorZoneCnt++;
            m_abZoneTypes[1] = kCIDLib::True;
        }
    }

    if (cqcdcOurs.bFindPromptValue(L"Z3Type", L"Selected", strTmp))
    {
        c4PCount++;
        if (strTmp == kR2DIS::pszZType_Sensor)
        {
            m_c4BlasterZoneCnt--;
            m_c4SensorZoneCnt++;
            m_abZoneTypes[2] = kCIDLib::True;
        }
    }

    if (cqcdcOurs.bFindPromptValue(L"Z4Type", L"Selected", strTmp))
    {
        c4PCount++;
        if (strTmp == kR2DIS::pszZType_Sensor)
        {
            m_c4BlasterZoneCnt--;
            m_c4SensorZoneCnt++;
            m_abZoneTypes[3] = kCIDLib::True;
        }
    }

    // If we didn't get all zone prompts, then log but keep going
    if (c4PCount < kR2DIS::c4ZoneCount)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facR2DIS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kR2DISMsgs::midCfg_NotAllZonePrompts
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    // Register the standard fields for an non-trainable IR blaster driver
    TVector<TCQCFldDef> colFlds(16 + kR2DIS::c4ZoneCount);
    TCQCFldDef flddCmd;
    flddCmd.Set
    (
        TFacCQCIR::strFldName_FirmwareVer
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_Invoke
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddCmd.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddCmd);

    flddCmd.Set
    (
        TFacCQCIR::strFldName_TrainingState
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddCmd);

    // Do the sensor zones, which have their own fields
    tCIDLib::TCard4 c4CurZone = 1;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kR2DIS::c4ZoneCount; c4Index++)
    {
        if (m_abZoneTypes[c4Index])
        {
            strTmp = L"Sensor";
            strTmp.AppendFormatted(c4CurZone++);
            flddCmd.Set
            (
                strTmp
                , tCQCKit::EFldTypes::Boolean
                , tCQCKit::EFldAccess::Read
            );
            colFlds.objAdd(flddCmd);
        }
    }

    // Tell our base class about our fields
    SetFields(colFlds);

    //
    //  Look up the ids of our fields, for efficiency. First we look up
    //  the fixed ones, then we go through the dynanic items and look
    //  up those.
    //
    m_c4FldIdFirmwareVer = pflddFind
    (
        TFacCQCIR::strFldName_FirmwareVer, kCIDLib::True
    )->c4Id();

    m_c4FldIdInvoke = pflddFind
    (
        TFacCQCIR::strFldName_Invoke, kCIDLib::True
    )->c4Id();

    m_c4FldIdTrainingMode = pflddFind
    (
        TFacCQCIR::strFldName_TrainingState
        , kCIDLib::True
    )->c4Id();

    c4CurZone = 1;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kR2DIS::c4ZoneCount; c4Index++)
    {
        if (m_abZoneTypes[c4Index])
        {
            strTmp = L"Sensor";
            strTmp.AppendFormatted(c4CurZone++);
            m_ac4FldIdSensors[c4Index] = pflddFind(strTmp, kCIDLib::True)->c4Id();
        }
    }

    //
    //  We only need to poll in order to see if the device goes away. The
    //  sensor configured zones send async messages and blaster zones are
    //  write only. But we need to set the poll time fairly fast so that we
    //  can see sensor change events and respond to them quickly.
    //
    SetPollTimes(500, 5000);

    //
    //  In our case we want to go to 'wait for config' mode, not wait for
    //  comm res, since we need to get configuration before we can go online.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TR2DIDriver::ePollDevice(TThread& thrThis)
{
    try
    {
        TString strMsg;

        // Watch for any async messages
        while (bGetMessage(strMsg, 50, kCIDLib::False))
            ProcessMsg(strMsg);

        //
        //  If we've not seen any async messages or some sort of reply in the
        //  last 5 seconds, then do an active poll of the sensors.
        //
        if (TTime::enctNow() > m_enctNextActivePoll)
            PollSensors(strMsg);
    }

    catch(...)
    {
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TR2DIDriver::EnterRecTrainingMode()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // If we are already in training mode, then that's an error
    if (m_bRecTrainingMode)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_AlreadyTraining
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
        );
    }

    //
    //  Go into training mode. We'll send the message to put it into
    //  training mode, and the polling loop will watch for the data to
    //  arrive.
    //
    m_bGotRecTrainingData = kCIDLib::False;
    m_bRecTrainingMode = kCIDLib::True;

    // Look for field access and update the training mode field
    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid TR2DIDriver::ExitRecTrainingMode()
{
    // Clear the flags first, then send our hard coded message
    m_bRecTrainingMode = kCIDLib::False;
    m_bGotRecTrainingData = kCIDLib::False;

    bStoreBoolFld(m_c4FldIdTrainingMode, m_bRecTrainingMode, kCIDLib::True);
}


tCIDLib::TVoid
TR2DIDriver::InvokeBlastCmd(const   TString&        strDevice
                            , const TString&        strCmd
                            , const tCIDLib::TCard4 c4ZoneNum)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // Make sure the zone number is legal for our zone count
    if (c4ZoneNum >= m_c4BlasterZoneCnt)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum)
            , strMoniker()
        );
    }

    // Look up the command
    tCIDLib::TCard4 c4Repeat;
    const TIRBlasterCmd& irbcInvoke = irbcFromName(strDevice, strCmd, c4Repeat);

    // And call the helper method that does the actual work
    SendIRCmd
    (
        irbcInvoke.mbufData().pc1Data()
        , irbcInvoke.c4DataLen()
        , c4ZoneNum
        , c4Repeat
    );
}


tCIDLib::TVoid TR2DIDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommR2DI->bIsOpen())
            m_pcommR2DI->Close();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid
TR2DIDriver::SendBlasterData(const  tCIDLib::TCard4 c4DataBytes
                            , const TMemBuf&        mbufToSend
                            , const tCIDLib::TCard4 c4ZoneNum
                            , const tCIDLib::TCard4 c4RepeatCount)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // Make sure the zone number is legal for our zone count
    if (c4ZoneNum >= m_c4BlasterZoneCnt)
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcBlast_BadZone
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TCardinal(c4ZoneNum)
            , strMoniker()
        );
    }

    //
    //  Call a helper that does the actual send, since we have to also
    //  support sending via command name, and we don't want to replicate
    //  this code.
    //
    SendIRCmd(mbufToSend.pc1Data(), c4DataBytes, c4ZoneNum, c4RepeatCount);
}


tCIDLib::TVoid TR2DIDriver::TerminateImpl()
{

    // Call our parent last
    TParent::TerminateImpl();
}


// ---------------------------------------------------------------------------
//  TR2DIDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TR2DIDriver::bGetMessage(       TString&            strToFill
                        , const tCIDLib::TCard4     c4WaitFor
                        , const tCIDLib::TBoolean   bThrowIfNot)
{
    if (!bGetASCIITermMsg(*m_pcommR2DI, c4WaitFor, 0xD, 0, strToFill))
    {
        if (bThrowIfNot)
        {
            facR2DIS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kR2DISErrs::errcComm_Timeout
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Timeout
            );
        }
        return kCIDLib::False;
    }

    // Set the next active poll stamp to 5 seconds from now
    m_enctNextActivePoll = TTime::enctNowPlusSecs(5);
    return kCIDLib::True;
}


//
//  Called to poll the configured sensors. This is also used as an active
//  ping of the device, so we don't check to see if we actually have any
//  sensors, we just do it. The caller provides us with a temp string to
//  use.
//
tCIDLib::TVoid TR2DIDriver::PollSensors(TString& strTmp)
{
    strTmp = L"AIRP?";
    SendMsg(strTmp);
    bGetMessage(strTmp, 2500, kCIDLib::True);

    //
    //  We could get an async here, so if it starts with AIR, then
    //  call process message, and try again.
    //
    while (kCIDLib::True)
    {
        if (strTmp.bStartsWith(m_strMsgPref))
        {
            ProcessMsg(strTmp);
            bGetMessage(strTmp, 2500, kCIDLib::True);
        }
         else
        {
            break;
        }
    }

    // It should be 4 one or zero digits
    if (strTmp.c4Length() != 4)
    {
        facR2DIS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kR2DISErrs::errcComm_BadSensorReply
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Protocol
        );
    }

    //
    //  Loop through the zones. For the ones that are configured as sensors,
    //  store the associated boolean value from the reply.
    //
    tCIDLib::TBoolean bOk;
    const tCIDLib::TCard4 c4Mask = TRawStr::c4AsBinary
    (
        strTmp.pszBuffer(), bOk, tCIDLib::ERadices::Hex
    );

    if (!bOk || (c4Mask > 0xF))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facR2DIS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Sensor state reply had bad mask"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return;
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < kR2DIS::c4ZoneCount; c4Index++)
    {
        // If not a sensor zone, skip it
        if (!m_abZoneTypes[c4Index])
            continue;

        bStoreBoolFld
        (
            m_ac4FldIdSensors[c4Index]
            , (c4Mask & (0x1 << c4Index)) != 0
            , kCIDLib::True
        );
    }
}


//
//  If any of the send and wait methods below see a message other than the
//  one waited for, they will call this method, to let us process it. The
//  device does send some async messages.
//
//  Note that the parameter is non-const, and we can mangle it in the
//  processes of getting the data out.
//
tCIDLib::TVoid TR2DIDriver::ProcessMsg(TString& strMsg)
{
    if (strMsg.bStartsWith(m_strMsgSensorStatePref))
    {
        // Pull out the probe number and state
        const tCIDLib::TCard4 c4PrefLen = m_strMsgSensorStatePref.c4Length();
        tCIDLib::TCard4 c4Num = kCIDLib::c4MaxCard;
        try
        {
            strMsg.Cut(0, c4PrefLen);
            TString strVal;
            if (strMsg.bSplit(strVal, kCIDLib::chColon))
            {
                c4Num = strMsg.c4Val(tCIDLib::ERadices::Dec);
                const tCIDLib::TBoolean bVal = strVal == L"0001";

                bStoreBoolFld(m_ac4FldIdSensors[c4Num], bVal, kCIDLib::True);
            }
             else
            {
                IncBadMsgCounter();
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facR2DIS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Async sensor state message was badly formed"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
            }
        }

        catch(TError& errToCatch)
        {
            IncBadMsgCounter();
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);

            // If we got a valid sensor id, then set this field in error
            if (c4Num && (c4Num < kR2DIS::c4ZoneCount) && m_abZoneTypes[c4Num])
                SetFieldErr(m_ac4FldIdSensors[c4Num], kCIDLib::True);
        }
    }
     else if (strMsg.bStartsWith(m_strMsgTrainDataPref))
    {
        // Store it and indicate we have the training data
        m_strRecTrainData = strMsg;
        m_bGotRecTrainingData = kCIDLib::True;
    }
}


//
//  Sends the passed message and waits for an ack from the device, processing
//  any other async messages it sees along the way.
//
tCIDLib::TVoid
TR2DIDriver::SendAndWaitForAck( const   TString&        strToSend
                                , const tCIDLib::TCard4 c4WaitFor)
{
    SendMsg(strToSend);
    TString strReply;
    WaitForAckOrError(strToSend, strReply, c4WaitFor);
}


tCIDLib::TVoid
TR2DIDriver::SendIRCmd( const   tCIDLib::TCard1* const  pc1Data
                        , const tCIDLib::TCard4         c4Bytes
                        , const tCIDLib::TCard4         c4ZoneNum
                        , const tCIDLib::TCard4         c4RepeatCount)
{
    TString strTmp;
    TString strCmd;

    //
    //  Enable the target zone. We have to run up through the zone type
    //  list till we come to the ZoneNum'th non-sensor entry, and that's
    //  the number of the actual R2DI zone to blast to.
    //
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4R2DIZone = 0;
    for (; c4Index < kR2DIS::c4ZoneCount; c4Index++)
    {
        if (!m_abZoneTypes[c4Index])
        {
            if (c4R2DIZone == c4ZoneNum)
                break;

            // Not our guy so count up one blaster zone
            c4R2DIZone++;
        }
    }

    //
    //  If we didn't find it, then that's bad. The caller should have checked
    //  that the zone number was within our count of configured blaster zones.
    //
    if (c4Index == kR2DIS::c4ZoneCount)
    {
        facR2DIS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kR2DISMsgs::midStatus_BZoneNotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , TCardinal(c4ZoneNum + 1)
            , strMoniker()
        );
        return;
    }

    //
    //  Send the emitter selection message. The data value is a 4 byte hex
    //  value in which the 1st bit is zone 1, 2nd bit is zone 2, and so forth,
    //  so we want to turn on the bit for the zone we ended up selecting
    //  above.
    //
    strCmd = L"AIRO%(1,4,0):";
    strCmd.eReplaceToken(TCardinal(0x1UL << c4Index, tCIDLib::ERadices::Hex), L'1');

    //
    //  Now let's convert the actual IR data and append that to the command.
    //  We know it's just ASCII data.
    //
    m_tcvtIO.c4ConvertFrom(pc1Data, c4Bytes, strTmp);
    strCmd.Append(strTmp);

    //
    //  And set up the repeat and add that to the command. Note taht the
    //  repeat count in the CQC world means how many times to play the
    //  command, but here it means how many times to repeat it after it's
    //  played the first time. So we need to subtract one, and if it's
    //  zero we just don't put the repeat on the command.
    //
    if (c4RepeatCount > 1)
    {
        strTmp = L":AIRR%(1,4,0):0000";
        strTmp.eReplaceToken
        (
            TCardinal(c4RepeatCount - 1, tCIDLib::ERadices::Hex), L'1'
        );
        strCmd.Append(strTmp);
    }

    tCIDLib::TCard4 c4BufSz = strCmd.c4Length();
    tCIDLib::TCard1* pc1Send = new tCIDLib::TCard1[c4BufSz + 2];
    tCIDLib::TCard4 c4OutBytes;
    TArrayJanitor<tCIDLib::TCard1> janSend(pc1Send);
    {
        m_tcvtIO.c4ConvertTo
        (
            strCmd.pszBuffer(), c4BufSz, pc1Send, c4BufSz, c4OutBytes
        );
        pc1Send[c4OutBytes++] = 0x0D;
    }

    // And send it
    if (m_pcommR2DI->c4WriteRawBufMS(pc1Send, c4OutBytes, 1000) != c4OutBytes)
    {
        facR2DIS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kR2DISErrs::errcComm_SendFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TString(L"IR command")
        );
    }

    // We should get an ack or error here also
    WaitForAckOrError(m_strMsgBlastPref, strTmp, 2000);
}


tCIDLib::TVoid TR2DIDriver::SendMsg(const TString& strToSend)
{
    // Transcode the text to ASCII
    const tCIDLib::TCard4 c4BufSz = 1024;
    tCIDLib::TCard1 ac1Buf[c4BufSz];

    tCIDLib::TCard4 c4OutBytes;
    m_tcvtIO.c4ConvertTo
    (
        strToSend.pszBuffer()
        , strToSend.c4Length()
        , ac1Buf
        , c4BufSz - 2
        , c4OutBytes
    );

    // Put the CR on it
    ac1Buf[c4OutBytes++] = 0xD;

    // And send it
    if (m_pcommR2DI->c4WriteRawBufMS(ac1Buf, c4OutBytes, 1000) != c4OutBytes)
    {
        TString strMsg(strToSend);
        if (strMsg.c4Length() > 4)
            strMsg.CapAt(4);

        facR2DIS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kR2DISErrs::errcComm_SendFailed
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strMsg
        );
    }
}


tCIDLib::TVoid
TR2DIDriver::WaitForAckOrError( const   TString&        strResponseTo
                                ,       TString&        strTmp
                                , const tCIDLib::TCard4 c4WaitFor)
{
    // It worked, so let's wait for the ack
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    const tCIDLib::TEncodedTime enctEnd
    (
        enctCur + (c4WaitFor * kCIDLib::enctOneMilliSec)
    );

    while (enctCur < enctEnd)
    {
        if (bGetMessage(strTmp, 250, kCIDLib::False))
        {
            if (strTmp == m_strMsgAckReply)
                return;

            // If an error code, then throw
            if (strTmp.bStartsWith(m_strMsgErrPref))
            {
                strTmp.Cut(0, m_strMsgErrPref.c4Length());
                facR2DIS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kR2DISErrs::errcComm_Nak
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Protocol
                    , strTmp
                );
            }

            // We got something, but not the ack, so process it
            ProcessMsg(strTmp);
        }
        enctCur = TTime::enctNow();
    }

    // We never saw the ack
    strTmp = strResponseTo;
    if (strTmp.c4Length() > 4)
        strTmp.CapAt(0);
    facR2DIS().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kR2DISErrs::errcComm_NoAck
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Timeout
        , strTmp
    );
}

