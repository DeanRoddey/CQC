//
// FILE NAME: ZWaveLeviS_DriverImpl.cpp
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
//  This file provides the implementation of the driver, with some grunt
//  work methods distribuetd to secondary files.
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
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveLeviSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveLeviSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveLeviSDriver::TZWaveLeviSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bLogZWMsgs(kCIDLib::False)
    , m_bNewConfig(kCIDLib::False)
    , m_bNoProcess(kCIDLib::True)
    , m_bsNodeMap(kZWaveLeviSh::c4MaxUnitId)
    , m_c4FldId_Command(kCIDLib::c4MaxCard)
    , m_c4FldId_PollsDue(kCIDLib::c4MaxCard)
    , m_c4PollableCnt(0)
    , m_c4TOCount(0)
    , m_colPollItems(tCIDLib::EAdoptOpts::NoAdopt)
    , m_enctNextTrans(0)
    , m_enctNextPoll(0)
    , m_mbufOut(512, 512)
    , m_pcommZWave(0)
{
}

TZWaveLeviSDriver::~TZWaveLeviSDriver()
{
    // Clean up the comm port if not already
    if (m_pcommZWave)
    {
        try
        {
            if (m_pcommZWave->bIsOpen())
                m_pcommZWave->Close();

            delete m_pcommZWave;
            m_pcommZWave = 0;
        }

        catch(...)
        {
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We support some of these types of backdoor commands.
//
//  A call lock is done on this driver before this is called, so we don't
//  need to lock.
//
tCIDLib::TCard4 TZWaveLeviSDriver::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4Ret = kCIDLib::c4MaxCard;
    if (strValId == kZWaveLeviSh::strDrvQ_ConfigSerialNum)
    {
        c4Ret = m_c4CfgSerialNum;
    }
     else
    {
        facZWaveLeviS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcCfg_UnknownQuery
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strValId
        );
    }
    return c4Ret;
}


//
//  We support a few backdoor commands that aren't done via the field
//  interface.
//
//  A call lock is done on this driver before this is called, so we don't
//  need to lock.
//
tCIDLib::TCard4
TZWaveLeviSDriver::c4SendCmd(const TString& strCmd, const TString& strParms)
{
    tCIDLib::TCard4 c4Ret = 0;
    if (strCmd == kZWaveLeviSh::strDrvCmd_StartScan)
    {
        //
        //  We don't care what the parameter value is. We can't do this
        //  here, so we have to just set a flag. What we do is act like
        //  new config was uploaded. But really we just copy the current
        //  config over and then set the new config flag.
        //
        m_dcfgUpload = m_dcfgCurrent;
        m_bNewConfig = kCIDLib::True;

        c4Ret = 1;
    }
     else
    {
        facZWaveLeviS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcCfg_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strCmd
        );
    }
    return c4Ret;
}


//
//  Standard driver backdoor method to return the driver configuration info.
//  The format is purely between the client and server driver sides.
//
//  A call lock is done on this driver before this is called, so we don't
//  need to lock.
//
tCIDLib::TCard4 TZWaveLeviSDriver::c4QueryCfg(THeapBuf& mbufToFill)
{
    // Create a bin output stream over the passed buffer
    TBinMBufOutStream strmOut(&mbufToFill);

    strmOut << tCIDLib::EStreamMarkers::StartObject
            << m_dcfgCurrent
            << m_c4CfgSerialNum
            << tCIDLib::EStreamMarkers::EndObject
            << kCIDLib::FlushIt;

    return strmOut.c4CurSize();
}


//
//  Handle send data backdoor calls from the client side driver. This one is
//  used for setting configuration.
//
//  A call lock is done on this driver before this is called, so we don't
//  need to lock.
//
tCIDLib::TVoid
TZWaveLeviSDriver::SetConfig(const  tCIDLib::TCard4 c4Bytes
                            , const THeapBuf&       mbufNewCfg)
{
    // Try to stream the incoming info into a local configuration object
    TZWaveDrvConfig dcfgNew;
    {
        TBinMBufInStream strmOut(&mbufNewCfg, c4Bytes);
        strmOut >> dcfgNew;
    }

    //
    //  First see if we can stream it out to the configuration server. If
    //  not, let it throw back to the client, since we don't want to accept
    //  it and then not be able to store it.
    //
    {
        TCfgServerClient cfgscLoad(5000);
        tCIDLib::TCard4 c4Ver;
        cfgscLoad.bAddOrUpdate(m_strCfgScope, c4Ver, dcfgNew, 2048);
    }

    // It worked, so copy the new configuration and set the update flag
    m_dcfgUpload = dcfgNew;
    m_bNewConfig = kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't need to lock here, because no incoming client calls will be
//  seen until we are online, and we aren't yet.
//
tCIDLib::TBoolean TZWaveLeviSDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        // Create the port object if needed, else close it just in case
        if (!m_pcommZWave)
            m_pcommZWave = facCIDComm().pcommMakeNew(m_strPort);
        else if (m_pcommZWave->bIsOpen())
            m_pcommZWave->Close();

        // Open the port now, and set the configuration
        m_pcommZWave->Open(tCIDComm::EOpenFlags::WriteThrough, 1024, 1024);
        m_pcommZWave->SetCfg(m_cpcfgSerial);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//
//  See if we can load any config. If there isn't any, that's fine, we just
//  come up with not groups and units. If we cannot get to the config server,
//  we indicate that we are still waiting.
//
tCIDLib::TBoolean TZWaveLeviSDriver::bWaitConfig(TThread&)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        //
        //  Get a config server client proxy. Use a fairly short timeout.
        //  If it's not there, we don't want to hang around and wait.
        //
        TCfgServerClient cfgscLoad(2500);

        // See if our config key exists in the repository
        if (cfgscLoad.bKeyExists(m_strCfgScope))
        {
            LogMsg
            (
                L"Trying to load existing configuration"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
            );

            //
            //  It does, so read it in. We don't need to keep the version
            //  around, since we only read it at startup and after that we
            //  only will write to it if we get a new replication.
            //
            tCIDLib::TCard4 c4Version = 0;
            tCIDLib::ELoadRes eRes = cfgscLoad.eReadObject
            (
                m_strCfgScope, m_dcfgCurrent, c4Version
            );

            //
            //  If we failed, then keep waiting. If we got new data or
            //  not data, either way we are now in sync and are good to
            //  go.
            //
            if (eRes != tCIDLib::ELoadRes::NotFound)
                bRes = kCIDLib::True;
        }
         else
        {
            LogMsg
            (
                L"No existing configuration"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
            );

            // None to load, so we are good
            bRes = kCIDLib::True;
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }

    if (bRes)
    {
        LogMsg
        (
            L"Got existing configuration, updating field list"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
        );
    }
    return bRes;
}


//
//  Handle boolean field writes
//
tCQCKit::ECommResults
TZWaveLeviSDriver::eBoolFldValChanged(const TString&            strName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const tCIDLib::TBoolean   bNewValue)
{
    LogMsg
    (
        L"Got bool field write. Name=%(1) Value=%(2)"
        , tCQCKit::EVerboseLvls::Low
        , CID_FILE
        , CID_LINE
        , strName
        , TCardinal(bNewValue)
    );

    //
    //  This will be a unit field, so find the unit that owns it.
    //
    const tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

        // Look for the unit that owns this field
        if (unitCur.bOwnsFld(c4FldId))
        {
            TString strValue;
            if (bNewValue)
                strValue = kZWaveLeviSh::strGetMsg_On;
            else
                strValue = kZWaveLeviSh::strGetMsg_Off;

            // Ask it to build the message to send
            TString strToSend;
            tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
            if (unitCur.bBuildSetMsg(c4FldId
                                    , strValue
                                    , strToSend
                                    , *this))
            {
                eRes = eSendFldWrite(strToSend, L"boolean", unitCur);
            }
             else
            {
                LogMsg
                (
                    L"Unit %(1) rejected write command"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , unitCur.strName()
                );
                IncFailedWriteCounter();
            }
            return eRes;
        }
    }

    // If we didn't find anybody who claims this field, then reject
    if (c4Index == c4UnitCount)
    {
        LogMsg
        (
            L"No unit claimed to own field %(1)"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strName
        );
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }

    LogMsg
    (
        L"Field write succeeded for field %(1)"
        , tCQCKit::EVerboseLvls::Medium
        , CID_FILE
        , CID_LINE
        , strName
    );
    return tCQCKit::ECommResults::Success;
}


//
//  Handle card field writes
//
tCQCKit::ECommResults
TZWaveLeviSDriver::eCardFldValChanged(const TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)
{
    LogMsg
    (
        L"Got card field write. Name=%(1) Value=%(2)"
        , tCQCKit::EVerboseLvls::Low
        , CID_FILE
        , CID_LINE
        , strName
        , TCardinal(c4NewValue)
    );

    // See if any unit owns this field
    tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

        // Look for the unit that owns this field
        if (unitCur.bOwnsFld(c4FldId))
        {
            TString strValue;
            strValue.SetFormatted(c4NewValue);

            // Ask it to build the message to send
            TString strToSend;
            tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
            if (unitCur.bBuildSetMsg(c4FldId
                                    , strValue
                                    , strToSend
                                    , *this))
            {
                eRes = eSendFldWrite(strToSend, L"cardinal", unitCur);
            }
             else
            {
                LogMsg
                (
                    L"Unit %(1) rejected write command"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , unitCur.strName()
                );
                IncFailedWriteCounter();
            }
            return eRes;
            break;
        }
    }

    // If we didn't find anybody who claims this field, then reject
    if (c4Index == c4UnitCount)
    {
        LogMsg
        (
            L"No unit claimed to own field %(1)"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strName
        );
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }

    LogMsg
    (
        L"Field write succeeded for field %(1)"
        , tCQCKit::EVerboseLvls::Medium
        , CID_FILE
        , CID_LINE
        , strName
    );
    return tCQCKit::ECommResults::Success;
}



//
//  This guy enumerates the available units, validates this against our
//  unit configuration, registers all the required fields for the configured
//  (and newly discovered) units, and gets initial values for the ones that
//  are currently available.
//
tCQCKit::ECommResults TZWaveLeviSDriver::eConnectToDevice(TThread& thrThis)
{
    LogMsg
    (
        L"Trying to connect to Z-Wave controller"
        , tCQCKit::EVerboseLvls::Medium
        , CID_FILE
        , CID_LINE
    );

    //
    //  Load the configuration. This call assumes it's getting a new config
    //  since it's also called when the client pushes up new config. So we
    //  have to copy the current config and pass that in. If all goes well,
    //  it'll update the current config with the (possibly updated) new config.
    //
    //  If it fails, then indicate we haven't gotten the connection yet.
    //
    TZWaveDrvConfig dcfgNew(m_dcfgCurrent);
    if (!bLoadConfig(dcfgNew, kCIDLib::True))
        return tCQCKit::ECommResults::LostConnection;

    // Reset per-connection stuff
    m_bsNodeMap.Clear();
    m_enctNextTrans = 0;
    m_enctNextPoll = 0;
    m_colPollItems.RemoveAll();
    m_c4TOCount = 0;

    return tCQCKit::ECommResults::Success;
}


//
//  Handle driver init, the usual stuff.
//
tCQCKit::EDrvInitRes TZWaveLeviSDriver::eInitializeImpl()
{
    LogMsg
    (
        L"Initializing Z-Wave driver"
        , tCQCKit::EVerboseLvls::Medium
        , CID_FILE
        , CID_LINE
    );

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

    // Clean up any existing port so it'll get redone with new config
    delete m_pcommZWave;
    m_pcommZWave = 0;

    // Setup our cfg scope by putting in our moniker
    m_strCfgScope = kZWaveLeviS::pszCfgScope;
    m_strCfgScope.eReplaceToken(cqcdcOurs.strMoniker(), kCIDLib::chLatin_m);

    //
    //  Set our poll and reconnect times to something desirable for us. We
    //  set the poll time to the lowest possible, so that we can get incoming
    //  messages out of the controller ASAP. It has a limited buffer size
    //  and we don't want it to overflow.
    //
    SetPollTimes(10, 15000);

    // Crank up a bit the extension time on the message reading helpers
    enctExtend(kCIDLib::enctOneMilliSec * 500);

    // Indicate that we need to load configuration
    return tCQCKit::EDrvInitRes::WaitConfig;
}



//
//  Handle int field writes
//
tCQCKit::ECommResults
TZWaveLeviSDriver::eIntFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4NewValue)
{
    LogMsg
    (
        L"Got int field write. Name=%(1) Value=%(2)"
        , tCQCKit::EVerboseLvls::Low
        , CID_FILE
        , CID_LINE
        , strName
        , TCardinal(i4NewValue)
    );

    // See if any unit owns this field
    tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

        // Look for the unit that owns this field
        if (unitCur.bOwnsFld(c4FldId))
        {
            TString strValue;
            strValue.SetFormatted(i4NewValue);

            // Ask it to build the message to send
            TString strToSend;
            tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
            if (unitCur.bBuildSetMsg(c4FldId
                                    , strValue
                                    , strToSend
                                    , *this))
            {
                eRes = eSendFldWrite(strToSend, L"integer", unitCur);
            }
             else
            {
                LogMsg
                (
                    L"Unit %(1) rejected write command"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , unitCur.strName()
                );
                IncFailedWriteCounter();
            }
            return eRes;
        }
    }

    // If we didn't find anybody who claims this field, then reject
    if (c4Index == c4UnitCount)
    {
        LogMsg
        (
            L"No unit claimed to own field %(1)"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strName
        );
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }

    LogMsg
    (
        L"Field write succeeded for field %(1)"
        , tCQCKit::EVerboseLvls::Medium
        , CID_FILE
        , CID_LINE
        , strName
    );
    return tCQCKit::ECommResults::Success;
}



//
//  Watch for ayncs and status replies (which we don't want for synchronously
//  when we poll), and periodically poll units when they are ready and need
//  to be polled.
//
tCQCKit::ECommResults TZWaveLeviSDriver::ePollDevice(TThread& thrCaller)
{
    try
    {
        //
        //  If the new configuration flag is set, then the client has uploaded
        //  new configuration. We need to reload this new configuration and
        //  then pick up again.
        //
        if (m_bNewConfig)
        {
            // Clear the flag first, to insure we don't get caught in a loop
            m_bNewConfig = kCIDLib::False;
            if (!bLoadConfig(m_dcfgUpload, kCIDLib::False))
            {
                facZWaveLeviS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZWErrs::errcCfg_CantSet
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }

        //
        //  Check for any available async messages. We just do a short wait for
        //  a message type we know we won't get. Tell it not to throw, since
        //  we know that we'll timeout once we've eaten available msgs. Any msgs
        //  that come in while we wait will be processed if appropriate.
        //
        tCIDLib::TCard4 c4ResCode;
        while (bWaitMsg(tZWaveLeviS::ERepTypes_Count, 50, c4ResCode, kCIDLib::False))
        {
        }

        //
        //  If it's been at least a 500ms since we last checked, then
        //  see if we have any units to poll.
        //
        //  BE SURE to get it updated no matter how we get out of here or
        //  we'll continually bang away. We can't set it before we do the
        //  round since then the round time will count against it.
        //
        if (m_enctNextPoll < TTime::enctNow())
        {
            try
            {
                DoPollRound();

                // Make sure the next poll time gets updated
                m_enctNextPoll = TTime::enctNowPlusMSs(kZWaveLeviS::c4MinPollRndMSs);

                // Clear the poll timeout counter
                m_c4TOCount = 0;
            }

            catch(TError& errToCatch)
            {
                // Make sure round stuff gets updated
                m_enctNextPoll = TTime::enctNowPlusMSs(kZWaveLeviS::c4MinPollRndMSs);

                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
                {
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    LogMsg
                    (
                        L"Exception occurred in poll round"
                        , tCQCKit::EVerboseLvls::High
                        , CID_FILE
                        , CID_LINE
                    );
                }
                throw;
            }

            catch(...)
            {
                // Make sure round stuff gets updated
                m_enctNextPoll = TTime::enctNowPlusMSs(kZWaveLeviS::c4MinPollRndMSs);

                LogMsg
                (
                    L"Unknown exception poll round"
                    , tCQCKit::EVerboseLvls::High
                    , CID_FILE
                    , CID_LINE
                );
                throw;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if ((eVerboseLevel() >= tCQCKit::EVerboseLvls::High) && !errToCatch.bLogged())
            TModule::LogEventObj(errToCatch);

        //
        //  Do a test to see if we are still connected to the device. If so,
        //  then this is not a loss of connection error. We just do a simple
        //  clear of the node list, which only talkes to the VRCOP and should
        //  get an ack if we are connected.
        //
        tCIDLib::TCard4 c4ResCode;
        try
        {
            // Pause a bit first, just in case
            if (thrCaller.bSleep(1000))
                bSendAndWaitAck(L"N,", 3000, c4ResCode, kCIDLib::True);
        }

        catch(...)
        {
            // If this happens more than 3 times in a row, give up
            m_c4TOCount++;
            if (m_c4TOCount > 3)
            {
                LogMsg
                (
                    L"Lost connection during poll, recycling... ResCode=%(1)"
                    , tCQCKit::EVerboseLvls::High
                    , CID_FILE
                    , CID_LINE
                    , TCardinal(c4ResCode)
                );
                return tCQCKit::ECommResults::LostConnection;
            }
        }
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Handle a write to a string field
//
tCQCKit::ECommResults
TZWaveLeviSDriver::eStringFldValChanged(const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    LogMsg
    (
        L"Got string field write. Name=%(1) Value=%(2)"
        , tCQCKit::EVerboseLvls::Low
        , CID_FILE
        , CID_LINE
        , strName
        , strNewValue
    );

    if (c4FldId == m_c4FldId_Command)
    {
        if (!bProcessCmdFld(strNewValue))
        {
            IncFailedWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }
    }
     else
    {
        // Let's see if a unit claims this field
        const tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4UnitCount; c4Index++)
        {
            TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);

            // Look for the unit that owns this field
            if (unitCur.bOwnsFld(c4FldId))
            {
                // Ask it to build the message to send
                TString strToSend;
                tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
                if (unitCur.bBuildSetMsg(c4FldId
                                        , strNewValue
                                        , strToSend
                                        , *this))
                {
                    eRes = eSendFldWrite(strToSend, L"string", unitCur);
                }
                 else
                {
                    LogMsg
                    (
                        L"Unit %(1) rejected write command"
                        , tCQCKit::EVerboseLvls::Low
                        , CID_FILE
                        , CID_LINE
                        , unitCur.strName()
                    );
                    IncFailedWriteCounter();
                }
                return eRes;
            }
        }

        // If we didn't find anybody who claims this field, then reject
        if (c4Index == c4UnitCount)
        {
            LogMsg
            (
                L"No unit claimed to own field %(1)"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
                , strName
            );
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }

        LogMsg
        (
            L"Field write succeeded for field %(1)"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
            , strName
        );
    }

    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TZWaveLeviSDriver::ReleaseCommResource()
{
    try
    {
        if (m_pcommZWave->bIsOpen())
            m_pcommZWave->Close();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
    }
}


tCIDLib::TVoid TZWaveLeviSDriver::TerminateImpl()
{
}



// ---------------------------------------------------------------------------
//  TZWaveLeviSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called on connect when we need to get ourself set up, and when
//  the client driver pushes up new configuration and we need to do the same
//  steps.
//
//  We have to probe the Z-Wave network and find the units available, validate
//  that against our config and update if needed, and then register our fields
//  and get initial info.
//
tCIDLib::TBoolean
TZWaveLeviSDriver::bLoadConfig(         TZWaveDrvConfig&    dcfgNew
                                , const tCIDLib::TBoolean   bOnConnect)
{
    try
    {
        //
        //  Iterate the available devices from the controller. We go through
        //  the existing configuration and verify it against what we are setup
        //  for, marking those not available and those that are.
        //
        //  Stop the processing of unsolicited messages until we get through
        //  this step.
        //
        //  We work off of a copy of our current configuration. If we don't
        //  survive the process, then we still have the original. We only store
        //  it away if we get all the way through the field registration
        //  process.
        //
        {
            TBoolJanitor janNoProc(&m_bNoProcess, kCIDLib::True);
            EnumeratedDevices(dcfgNew);

            // Register fields now that we know what we have
            RegisterFields(dcfgNew);

            // OK, it worked so store the new configuration
            m_dcfgCurrent = dcfgNew;

            // Update the serial number
            m_c4CfgSerialNum++;
        }

        //
        //  Get initial values for configured units. Async processing is on
        //  again now, so we'll pick up incoming reports while we are waiting
        //  on other responses during the initial query round.
        //
        GetInitVals();
    }

    catch(TError& errToCatch)
    {
        LogError
        (
            errToCatch
            , bOnConnect ? tCQCKit::EVerboseLvls::Medium : tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  We support a 'command' field that lets them send various commands via
//  a formatted string.
//
//  The commands are:
//
//      AddAssoc [tarunit] [srcunit] srcgrpnum
//      AllOn
//      AllOff
//      AssocToCtl unitname grpnum
//      DelAssoc tarunit srcunit srcgrpnum
//      Group grpname (On|Off)|(RampUp|RampDn|RampEnd)
//      GroupLev grpname %level
//      LogMsgs (Yes | No)
//      SetCfgParam unitname parm# val1 [val2 val3 ...]
//      SetGroupList grpname unit1 [unit2 unit3 ...]
//      SetSetPnt thermoname (Heat|Cool|Furnace|Dry|Moist|Auto) temp [F|C]
//      Unit unitname (On|Off|RampUp|RampDn|RampEnd)
//
tCIDLib::TBoolean TZWaveLeviSDriver::bProcessCmdFld(const TString& strCmdText)
{
    //
    //  Break out the command parameters. They are in the standard command
    //  line format, so space separated tokens, quoted where necessary to
    //  include spaces in tokens.
    //
    tCIDLib::TStrList colTokens;
    if (!TExternalProcess::c4BreakOutParms(strCmdText, colTokens)
    ||  colTokens.bIsEmpty())
    {
        LogMsg
        (
            L"'%(1)' is not a valid Command field string"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , strCmdText
        );
        return kCIDLib::False;
    }

    // Strip leading and trailing white space from the tokens
    const tCIDLib::TCard4 c4TokenCnt = colTokens.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TokenCnt; c4Index++)
        colTokens[c4Index].StripWhitespace();

    // Upper case the first one, which is the command
    colTokens[0].ToUpper();
    const TString& strCmd = colTokens[0];

    try
    {
        // And, based on the command, process the info
        tCIDLib::TCard4 c4ResCode;
        TString strToSend;
        if ((strCmd == kZWaveLeviSh::strDrvCmd_AddAssoc)
        ||  (strCmd == kZWaveLeviSh::strDrvCmd_DelAssoc))
        {
            //
            //  These are the same other than the command. We get a target unit,
            //  a source unit and a group in that source unit.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3);

            tCIDLib::TCard4 c4GrpId;
            CheckNumericVal(colTokens[3], 3, c4GrpId, L"groupnum");
            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);
            const TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(colTokens[2]);

            strToSend = L"N";
            strToSend.AppendFormatted(unitSrc.c4Id());
            strToSend.Append(L"SE");
            strToSend.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_ASSOCIATION));
            strToSend.Append(kCIDLib::chComma);
            if (strCmd ==  kZWaveLeviSh::strDrvCmd_AddAssoc)
                strToSend.AppendFormatted(tCIDLib::TCard4(ASSOCIATION_SET));
            else
                strToSend.AppendFormatted(tCIDLib::TCard4(ASSOCIATION_REMOVE));
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(c4GrpId);
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(unitTar.c4Id());

            SendUnitCmd(strToSend, 5000);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_AllOn)
        {
            strToSend = L"N,ON";
            bSendAndWaitAck(strToSend, 5000, c4ResCode);
            TThread::Sleep(500);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_AllOff)
        {
            strToSend = L"N,OF";
            bSendAndWaitAck(strToSend, 5000, c4ResCode);
            TThread::Sleep(500);
        }
         else if ((strCmd == kZWaveLeviSh::strDrvCmd_AssocToCtl)
              ||  (strCmd == kZWaveLeviSh::strDrvCmd_UnlinkFromCtl))
        {
            //
            //  These are the same other than the command. For both we have
            //  to have a controller marked as the VRCOP. Tell the lookup
            //  method to throw if not found.
            //
            const tCIDLib::TCard4 c4VRCOPId = c4FindVRCOP(m_dcfgCurrent, kCIDLib::True);

            //
            //  We have to use the passthrough command to do this one since it's
            //  not supported directly. The first parm is the name of the unit
            //  that are going to send the command to, and the second is the
            //  group number to add the controller's id to.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 2);
            tCIDLib::TCard4 c4GrpId;
            CheckNumericVal(colTokens[2], 2, c4GrpId, L"groupnum");

            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);
            strToSend = L"N";
            strToSend.AppendFormatted(unitTar.c4Id());
            strToSend.Append(L"SE");
            strToSend.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_ASSOCIATION));
            strToSend.Append(kCIDLib::chComma);
            if (strCmd ==  kZWaveLeviSh::strDrvCmd_AssocToCtl)
                strToSend.AppendFormatted(tCIDLib::TCard4(ASSOCIATION_SET));
            else
                strToSend.AppendFormatted(tCIDLib::TCard4(ASSOCIATION_REMOVE));
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(c4GrpId);
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(c4VRCOPId);

            SendUnitCmd(strToSend, 5000);
        }
         else if ((strCmd == kZWaveLeviSh::strDrvCmd_Unit)
              ||  (strCmd == kZWaveLeviSh::strDrvCmd_Group))
        {
            //
            //  These are very similar so we handle them together. We get a
            //  unit or group name, followed by a command to do, so two parms.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 2);

            //
            //  Get the id of the thing we are dealing with. And, if doing
            //  a group a group recall command. Else set the current live
            //  group to the unit.
            //
            tCIDLib::TCard4 c4TarId;
            if (strCmd == kZWaveLeviSh::strDrvCmd_Unit)
            {
                c4TarId = m_dcfgCurrent.unitFindByName(colTokens[1]).c4Id();
                strToSend = L"N";
                strToSend.AppendFormatted(c4TarId);
            }
             else
            {
                m_dcfgCurrent.FindGroupByName(colTokens[1], c4TarId);
                strToSend = L"GR";
                strToSend.AppendFormatted(c4TarId);
            }

            //
            //  Based on the type of command, build the rest of the
            //  string.
            //
            if (!colTokens[2].eCompareI(L"On"))
                strToSend.Append(L"ON");
            else if (!colTokens[2].eCompareI(L"Off"))
                strToSend.Append(L"OF");
            else if (!colTokens[2].eCompareI(L"RampDn"))
                strToSend.Append(L"DI");
            else if (!colTokens[2].eCompareI(L"RampEnd"))
                strToSend.Append(L"ST");
            else if (!colTokens[2].eCompareI(L"RampUp"))
                strToSend.Append(L"BR");

            // Send it and wait for the ack
            SendUnitCmd(strToSend, 5000);

            // These need a little time, so prevent a rushed next command
            TThread::Sleep(100);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_GroupLev)
        {
            // We get a group name and a level to set
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 2);

            tCIDLib::TCard4 c4Level;
            CheckNumericVal(colTokens[2], 2, c4Level, L"level");

            // See if we have such a group and get the group id
            tCIDLib::TCard4 c4GrpId;
            m_dcfgCurrent.FindGroupByName(colTokens[1], c4GrpId);

            // Recall the group first, then set level
            strToSend = L"GR";
            strToSend.AppendFormatted(c4GrpId);
            strToSend.Append(kCIDLib::chLatin_L);
            strToSend.AppendFormatted(c4Level);

            SendUnitCmd(strToSend, 5000);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_LogMsgs)
        {
            // We get a yes or no value
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 1);
            if (!colTokens[1].eCompareI(L"YES"))
                m_bLogZWMsgs = kCIDLib::True;
            else
                m_bLogZWMsgs = kCIDLib::False;
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_SetCfgParam)
        {
            //
            //  We get the target unit, a param number, and at least one
            //  byte value to set, so three or more values.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3, kCIDLib::True);

            // Find the target unit
            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);

            // Make sure the parm number and value are valid
            tCIDLib::TCard4 c4Num;
            tCIDLib::TCard4 c4Val;
            CheckNumericVal(colTokens[2], 2, c4Num, L"parm#");

            // This one requires a passthrough command
            strToSend = L"N";
            strToSend.AppendFormatted(unitTar.c4Id());
            strToSend.Append(L"SE");
            strToSend.AppendFormatted(tCIDLib::TCard4(COMMAND_CLASS_CONFIGURATION));
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(tCIDLib::TCard4(CONFIGURATION_SET));
            strToSend.Append(kCIDLib::chComma);
            strToSend.AppendFormatted(c4TokenCnt - 3);

            for (tCIDLib::TCard4 c4ByteInd = 3; c4ByteInd < c4TokenCnt; c4ByteInd++)
            {
                strToSend.Append(kCIDLib::chComma);
                CheckNumericVal(colTokens[c4ByteInd], c4ByteInd, c4Val, L"value");
                strToSend.AppendFormatted(c4Val);
            }

            SendUnitCmd(strToSend, 5000);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_SetGroupList)
        {
            //
            //  We should get the name of an existing group, then a list of
            //  zero or more units that should be in that group, so at least
            //  one parm, but usually more.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 1, kCIDLib::True);

            // See if we have such a group and get the group id
            tCIDLib::TCard4 c4GrpId;
            m_dcfgCurrent.FindGroupByName(colTokens[1], c4GrpId);

            //
            //  And now we go through and build up the command to set the
            //  group ids. If we hit the 80 char limit we do an append
            //  command and keep going.
            //
            strToSend = L"N";
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TokenCnt - 2; c4Index++)
            {
                // See if we have such a unit
                const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[c4Index]);

                // We do so add it to the list
                if (c4Index)
                    strToSend.Append(kCIDLib::chComma);
                strToSend.AppendFormatted(unitTar.c4Id());

                if (strToSend.c4Length() > 70)
                {
                    // Send this bunch and start an append command for more
                    bSendAndWaitAck(strToSend, 5000, c4ResCode);
                    strToSend = L"AP";
                }
            }

            //
            //  Now append the group store command onto what we have left,
            //  and send that.
            //
            strToSend.Append(L"GS");
            strToSend.AppendFormatted(c4GrpId);
            SendUnitCmd(strToSend, 5000);
        }
         else if (strCmd == kZWaveLeviSh::strDrvCmd_SetSetPnt)
        {
            //
            //  We should get the thermo, sp type, and sp temp. Optionally an
            //  F/C, so three or more.
            //
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3, kCIDLib::True);

            // Find the unit. It must be a thermostat
            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);
            if (!unitTar.bIsA(TZWThermo::clsThis()))
            {
                LogMsg
                (
                    L"'%(1)' is not a thermostat unit"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , colTokens[1]
                );
                return kCIDLib::False;
            }

            // Cast to the actual type
            const TZWThermo& unitThermo = (TZWThermo&)unitTar;

            // Make sure we can convert the temp
            tCIDLib::TInt4 i4Temp;
            if (!colTokens[3].bToInt4(i4Temp, tCIDLib::ERadices::Dec))
            {
                LogMsg
                (
                    L"'%(1)' is not a value temperature"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , colTokens[3]
                );
                return kCIDLib::False;
            }

            // Next we get the set point type
            const TString& strSPType = colTokens[2];
            tCIDLib::TCard4 c4SPId;
            if (strSPType == L"Heat")
                c4SPId = 1;
            else if (strSPType == L"Cool")
                c4SPId = 2;
            else if (strSPType == L"Furnace")
                c4SPId = 3;
            else if (strSPType == L"Dry")
                c4SPId = 4;
            else if (strSPType == L"Moist")
                c4SPId = 5;
            else if (strSPType == L"Auto")
                c4SPId = 6;
            else
            {
                LogMsg
                (
                    L"'%(1)' is not a valid thermo set point type"
                    , tCQCKit::EVerboseLvls::Low
                    , CID_FILE
                    , CID_LINE
                    , strSPType
                );
                return kCIDLib::False;
            }

            //
            //  If we got four parms, then the last one is an F/C indicator.
            //  If not there, we assume F.
            //
            tCIDLib::TBoolean bCTemp = kCIDLib::False;
            if (c4TokenCnt == 5)
                bCTemp = colTokens[4] == L"C";

            // Ask the thermo to build the msg
            unitThermo.BuildSPSetMsg
            (
                c4SPId, bCTemp, colTokens[3], strToSend
            );
            SendUnitCmd(strToSend, 5000);
        }
         else
        {
            LogMsg
            (
                L"'%(1)' is not a valid Command field command"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
                , strCmd
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  A little helper for the command field handler method, to check that a
//  required number of parameters are available, else throw. It can be a
//  literal count check or at least that many.
//
//  The actual count is one less than the parm count since that includes the
//  command itself. If it's zero we consider that an error since the caller
//  should not have even called us in that case.
//
tCIDLib::TVoid
TZWaveLeviSDriver::CheckCmdFldParmCnt(  const   TString&            strCmd
                                        , const tCIDLib::TCard4     c4ParmCnt
                                        , const tCIDLib::TCard4     c4ExpCnt
                                        , const tCIDLib::TBoolean   bOrMore)
{
    tCIDLib::TBoolean bErr;
    if (bOrMore)
        bErr = ((c4ParmCnt == 0) || (c4ParmCnt - 1 < c4ExpCnt));
    else
        bErr = ((c4ParmCnt == 0) || (c4ParmCnt - 1 != c4ExpCnt));

    if (bErr)
    {
        facZWaveLeviS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZWErrs::errcFld_CmdParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strCmd
            , TCardinal(c4ExpCnt)
            , TCardinal(c4ParmCnt ? c4ParmCnt - 1 : 0)
        );
    }
}


//
//  Checks a provided textual representation of a value that should be a
//  a numeric value. Make sure it can be converted to a number and that it
//  is within a given range. If not throw.
//
tCIDLib::TVoid
TZWaveLeviSDriver::CheckNumericVal( const   TString&            strVal
                                    , const tCIDLib::TCard4     c4ParmNum
                                    ,       tCIDLib::TCard4&    c4ToFill
                                    , const tCIDLib::TCh* const pszName
                                    , const tCIDLib::TCard4     c4Min
                                    , const tCIDLib::TCard4     c4Max)
{
    if (!strVal.bToCard4(c4ToFill, tCIDLib::ERadices::Dec)
    ||  (c4ToFill < c4Min)
    ||  (c4ToFill > c4Max))
    {
        if ((c4Min == 0) && (c4Max == kCIDLib::c4MaxCard))
        {
            // No range involved
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcFld_NumValFmt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TString(pszName)
            );
        }
         else
        {
            facZWaveLeviS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZWErrs::errcFld_NumValFmt2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TString(pszName)
                , TCardinal(c4Min)
                , TCardinal(c4Max)
            );
        }
    }
}



//
//  Called when it's time to do another poll round. We check our list for
//  units to do poll. We just wait for the transmission ack. We'll get the
//  replies later as (apparent) asyncs.
//
tCIDLib::TVoid TZWaveLeviSDriver::DoPollRound()
{
    //
    //  See how many units are at or past due. This will only return ones
    //  that are possibly pollable. So no battery powered or non-readable
    //  or non-viable or non-pollable ones.
    //
    tCIDLib::TCard4 c4PollCnt = m_dcfgCurrent.c4QueryDuePollList
    (
        m_colPollItems, *this
    );

    // If non, we are done
    if (!c4PollCnt)
    {
        // Update the field that shows the number of units due just in case
        bStoreCardFld(m_c4FldId_PollsDue, c4PollCnt, kCIDLib::True);
        return;
    }

    //
    //  The next one isn't basic, so so it separately. Get a
    //  pointer then remove it from the list. This list is non
    //  adopting, so we can just remove it from the list after
    //  we get a pointer.
    //
    TZWaveUnit* punitCur = m_colPollItems[0];
    m_colPollItems.RemoveAt(0);

    // Update the field that shows the number of units due
    bStoreCardFld(m_c4FldId_PollsDue, c4PollCnt, kCIDLib::True);

    //
    //  Ask it to build a query message for itself and send it. It
    //  can have more than one query, so we just increment the query
    //  each time (it will wrap around of course), and go through one
    //  of his queries each time he comes up.
    //
    const tCIDLib::TCard4 c4GetMsgInd = punitCur->c4IncGetMsgIndex();
    if (punitCur->bBuildGetMsg(m_strTmpPoll1, c4GetMsgInd))
    {
        try
        {
            // Let hte first one throw since it means no response from the VRCOP
            tCIDLib::TCard4 c4ResCode;
            bSendAndWaitAck(m_strTmpPoll1, 8000, c4ResCode, kCIDLib::True);

            // And this one just means no transmisstion
            if (!bWaitTransAck(8000, c4ResCode, kCIDLib::False))
            {
                LogMsg
                (
                    L"Poll failed to transmit msg for unit. Unit=%(1)"
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , punitCur->strName()
                );

                //
                //  Just process messages for a short period of time, to give
                //  it time to settle after a transmission.
                //
                while (bWaitMsg(tZWaveLeviS::ERepTypes_Count, 250, c4ResCode, kCIDLib::False))
                {}
            }

            // Update the last poll time of this unit
            punitCur->ResetPollTime(kCIDLib::False);
        }

        catch(TError& errToCatch)
        {
            // Even though it may have failed, update the last poll time
            punitCur->ResetPollTime(kCIDLib::False);

            if ((eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            &&  !errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                LogMsg
                (
                    L"Exception occured while polling. Unit=%(1)"
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                    , punitCur->strName()
                );
            }
            throw;
        }
    }
     else
    {
        // It failed to build a get message
        LogMsg
        (
            L"Unit %(1) did not build a GET message"
            , tCQCKit::EVerboseLvls::Medium
            , CID_FILE
            , CID_LINE
        );
    }
}



tCQCKit::ECommResults
TZWaveLeviSDriver::eSendFldWrite(const  TString&            strToSend
                                , const tCIDLib::TCh* const pszType
                                , const TZWaveUnit&         unitTar)
{
    tCIDLib::TCard4 c4ResCode;
    if (!bSendAndWaitAck(strToSend, 8000, c4ResCode, kCIDLib::False))
    {
        LogMsg
        (
            L"Failed to start %(1) fld change msg for %(2). ResCode=%(3)"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , TString(pszType)
            , unitTar.strName()
            , TCardinal(c4ResCode)
        );
        IncFailedWriteCounter();
        return tCQCKit::ECommResults::CommError;
    }

    if (!bWaitTransAck(8000, c4ResCode, kCIDLib::False))
    {
        LogMsg
        (
            L"No transmit ack received for %(1) fld write for %(2). ResCode=%(3)"
            , tCQCKit::EVerboseLvls::Low
            , CID_FILE
            , CID_LINE
            , TString(pszType)
            , unitTar.strName()
            , TCardinal(c4ResCode)
        );
        IncFailedWriteCounter();
        return tCQCKit::ECommResults::CommError;
    }
    return tCQCKit::ECommResults::Success;
}

