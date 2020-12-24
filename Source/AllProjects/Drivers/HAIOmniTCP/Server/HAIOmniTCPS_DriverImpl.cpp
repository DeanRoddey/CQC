//
// FILE NAME: HAIOmniTCPS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
//  This is a driver which implements the HAI Omni TCP interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCPS.hpp"
#include    "DevClsHdr_Security.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniTCPSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//   CLASS: THAIOmniTCPSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniTCPSDriver::THAIOmniTCPSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_sockOmni()
    , m_splBufs(L"Omni Buf Pool", 64, 32)
    , m_strEvButton(L"Button")
    , m_strEvUPBLink(L"UPBLink")
    , m_strEvX10Code(L"X10Code")
{
    // Do some init that we want to do here and upon termination
    DoCommonInit();
}

THAIOmniTCPSDriver::~THAIOmniTCPSDriver()
{
}



// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Handle some queries that can be made against security related drivers
//  such as this.
//
tCIDLib::TBoolean
THAIOmniTCPSDriver::bQueryTextVal(  const   TString&    strQueryType
                                    , const TString&    strDataName
                                    ,       TString&    strToFill)
{
    strToFill.Clear();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (strQueryType == kDevCls_Security::pszSecQN_SecPanelInfo)
    {
        if (strDataName == kDevCls_Security::pszSecQT_AreaList)
        {
            //
            //  Give him a list of area names and related alarm and arming
            //  status fields.
            //
            const tCIDLib::TCard4 c4Count = kHAIOmniTCPSh::c4MaxAreas;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // See if we have an item at this slot
                THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaAt(c4Index);
                if (!pitemCur)
                    continue;

                // If not the first one, adda new line
                if (!strToFill.bIsEmpty())
                    strToFill.Append(L"\n");

                // Do the name and a space
                strToFill.Append(pitemCur->strName());
                strToFill.Append(kCIDLib::chSpace);

                // The alarmed field and a space
                strToFill.Append(pitemCur->strName());
                strToFill.Append(L"_Alarmed");
                strToFill.Append(kCIDLib::chSpace);

                // And the arm status
                strToFill.Append(pitemCur->strName());
                strToFill.Append(L"_Status");
            };
            bRet = kCIDLib::True;
        }
         else if (strDataName == kDevCls_Security::pszSecQT_SecArmInfo)
        {
            //
            //  First a comma separated list of arming modes, but skipping
            //  the 0th one, which is disarm, and we use a counter that only
            //  includes the ones we take for the arming command. The
            //  enum includes others that are 'in process' type of states,
            //  which we don't want.
            //
            const tCIDLib::TCard4 c4Count = kHAIOmniTCPS::c4ArmSecModCnt;
            for (tCIDLib::TCard4 c4Index = 1; c4Index < c4Count; c4Index++)
            {
                if (c4Index > 1)
                    strToFill.Append(L',');
                strToFill.Append(kHAIOmniTCPS::apszSecModes[c4Index]);
            }
            strToFill.Append(L"\n");

            // Next our arming command format
            strToFill.Append(L"ArmArea=%(a),%(m),%(c)\n");

            //
            //  And our disarm cmd, whcih is the same but with a hard coded
            //  mode in this case.
            //
            strToFill.Append(L"ArmArea=%(a),Off,%(c)\n");
            bRet = kCIDLib::True;
        }
    }
    return bRet;
}


//
//  Note that CQCServer, who calls these, does a call/field lock for the set
//  config, and a call lock for the query, so we are good on sync here.
//
tCIDLib::TCard4 THAIOmniTCPSDriver::c4QueryCfg(THeapBuf& mbufToFill)
{
    // Create an output stream over the buffer and stream out config to it
    TBinMBufOutStream strmTar(&mbufToFill);
    strmTar << m_dcfgCurrent << kCIDLib::FlushIt;
    return strmTar.c4CurSize();
}


tCIDLib::TVoid
THAIOmniTCPSDriver::SetConfig(  const   tCIDLib::TCard4 c4Bytes
                                , const THeapBuf&       mbufNewCfg)
{
    //
    //  Create an input stream and stream in our configuration data. Allow
    //  any exceptions to propogate back to the caller.
    //
    TBinMBufInStream strmSrc(&mbufNewCfg, c4Bytes);

    // Stream in the full config that should follow this
    strmSrc >> m_dcfgCurrent;

    // Write it out to our file
    WriteCfgFile();

    // Register new fields now based on this new configuration
    RegisterFields();

    // Bump the reconfigured instrumentation field
    IncReconfigCounter();

    // Now let's get all our fields initialized
    bInitFields();
}


// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniTCPSDriver::bGetCommResource(TThread& thrThis)
{
    // Just in case, close it down if not already
    try
    {
        if (m_sockOmni.bIsConnected())
            m_sockOmni.c4Shutdown();
    }

    catch(...) {}

    try
    {
        m_sockOmni.Close();
    }

    catch(...) {}

    //
    //  And now open it. We just open, we don't connect yet. We have to use
    //  the same address type as what we are going to use on connect, which
    //  is in the resolved IP endpoint of the connection info.
    //
    try
    {
        m_sockOmni.Open
        (
            tCIDSock::ESockProtos::TCP, m_conncfgSock.ipepConn().eAddrType()
        );
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        // Not much we can do, so eat it, but do a close just in case
        m_sockOmni.Close();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniTCPSDriver::bWaitConfig(TThread& thrThis)
{
    //
    //  As of version 4.2.917 (driver version 1.2), we moved the config out
    //  to an external file and radically changed it. We store the config
    //  object now, and it manages the item object for us, and the items are
    //  used both for config and runtime.
    //
    //  If we don't find the file, we see if we are running the first time
    //  after the upgrade and look for any old config first from the cfg
    //  server, and read it in, and write it out to the external file, then
    //  delete it from the config server.
    //
    //  Try to get a configuration repository client proxy and load up our
    //  config. If no config has been stored yet, then we do nothing and say
    //  we are good. If we have some, then we register the fields. If we
    //  get any error while trying to load the config, we just keep waiting.
    //
    if (!TFileSys::bExists(m_strCfgFile, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        try
        {
            TCfgServerClient cfgcRepo(kHAIOmniTCPS::c4WaitReply);
            TString strCfgKey(L"/Drivers/HAIOmniTCPS/%(m)/CfgData");
            strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);
            tCIDLib::TCard4 c4Ver = 0;
            const tCIDLib::ELoadRes eRes = cfgcRepo.eReadObject
            (
                strCfgKey, m_dcfgCurrent, c4Ver
            );

            // If we didn't get any data, then reset our current config
            if (eRes != tCIDLib::ELoadRes::NewData)
            {
                m_dcfgCurrent.Reset();

                // Put in some testing stuff
                #if CID_DEBUG_ON
                // #define USEFAKECFG 1
                #if USEFAKECFG
                m_dcfgCurrent.SetAreaAt(THAIOmniArea(L"BottomFloor", 1), 0);
                m_dcfgCurrent.SetAreaAt(THAIOmniArea(L"", 2), 1);
                m_dcfgCurrent.SetZoneAt(THAIOmniZone(L"PatioBreak", 1, tHAIOmniTCPSh::EZone_Alarm), 0);
                #endif
                #endif
            }

            //
            //  Let's write the data out to the file. Even if we got no
            //  data, we'll get the default config to the file.
            //
            WriteCfgFile();

            //
            //  That worked, so let's delete the cfg server based config,
            //  if it exists.
            //
            cfgcRepo.bDeleteObjectIfExists(strCfgKey);
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged()
            &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            //
            //  It wasn't that it didn't exist, but an error occurred, so
            //  we keep trying.
            //
            return kCIDLib::False;
        }
    }

    // OK, now if we have a file, let's load it
    return bReadCfgFile();
}


tCQCKit::ECommResults
THAIOmniTCPSDriver::eBoolFldValChanged( const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    try
    {
        if ((c4FldId >= m_c4FirstThermoFldId) && (c4FldId <= m_c4LastThermoFldId))
        {
            //
            //  It has to be the hold field since that's the only boolean
            //  writeable we have for thermos.
            //
            THAIOmniThermo* pitemTar = m_dcfgCurrent.pitemOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any thermostat"
                    , tCIDLib::ESeverities::Status
                );
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::Internal;
            }

            SendOmniCmd
            (
                kHAIOmniTCPS::c1Cmd_SetThermoHoldMode
                , bNewValue ? 0xFF : 0
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(kCIDLib::True);
        }
         else if ((c4FldId >= m_c4FirstUnitFldId) && (c4FldId <= m_c4LastUnitFldId))
        {
            THAIOmniUnit* pitemTar = m_dcfgCurrent.pitemOwningUnit(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any unit"
                    , tCIDLib::ESeverities::Status
                );
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::Internal;
            }

            // It's got to be a binary unit
            if (pitemTar->eUnitType() != tHAIOmniTCPSh::EUnit_Binary)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The target unit is not a binary unit"
                    , tCIDLib::ESeverities::Status
                );
                return tCQCKit::ECommResults::Internal;
            }

            SendOmniCmd
            (
                bNewValue ? kHAIOmniTCPS::c1Cmd_UnitOn : kHAIOmniTCPS::c1Cmd_UnitOff
                , 0
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(kCIDLib::True);

            //
            //  We have to do this here, even though it's done when we get reports from
            //  the Omni as well. Once we return from here, the new value is stored to
            //  the field. If we depend on the reflected back value to send the trigger,
            //  it won't work since it won't look like the value has changed at that
            //  point. And we hvae to do it there to catch changes outside of CQC being
            //  reported.
            //
            //  We won't get called here unless the new value is different from what
            //  is currently stored, so this shouldn't result in any bogus reports.
            //
            if (pitemTar->bSendTrig())
            {
                QueueEventTrig
                (
                    tCQCKit::EStdDrvEvs::LoadChange
                    , strName
                    , facCQCKit().strBoolOffOn(bNewValue)
                    , TString(TCardinal(pitemTar->c4ItemNum()))
                    , pitemTar->strName()
                    , TString::strEmpty()
                );
            }
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
THAIOmniTCPSDriver::eCardFldValChanged( const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    try
    {
        if ((c4FldId >= m_c4FirstUnitFldId) && (c4FldId <= m_c4LastUnitFldId))
        {
            THAIOmniUnit* pitemTar = m_dcfgCurrent.pitemOwningUnit(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any unit"
                    , tCIDLib::ESeverities::Status
                );
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::Internal;
            }

            // It's got to be a dimmer or flag
            if (pitemTar->eUnitType() == tHAIOmniTCPSh::EUnit_Dimmer)
            {
                SendOmniCmd
                (
                    kHAIOmniTCPS::c1Cmd_UnitLightLev
                    , tCIDLib::TCard1((c4NewValue > 100) ? 100 : c4NewValue)
                    , tCIDLib::TCard2(pitemTar->c4ItemNum())
                );
            }
             else if (pitemTar->eUnitType() == tHAIOmniTCPSh::EUnit_Flag)
            {
                SendOmniCmd
                (
                    kHAIOmniTCPS::c1Cmd_SetCounter
                    , tCIDLib::TCard1(c4NewValue & 0xFF)
                    , tCIDLib::TCard2(pitemTar->c4ItemNum())
                );
            }
             else
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The target unit is not a dimmer or flag unit"
                    , tCIDLib::ESeverities::Status
                );
                return tCQCKit::ECommResults::Internal;
            }
            bWaitAck(kCIDLib::True);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  We do the negotiation for a session with the Omni to get a session key
//  and get logged in, then we query all the named items from the Omni,
//  and finally register and look up all our fields, some of which are
//  always present and some of which are driven by the named items we find.
//
tCQCKit::ECommResults THAIOmniTCPSDriver::eConnectToDevice(TThread& thrThis)
{
    // If not connected, then try to connect
    if (!m_sockOmni.bIsConnected())
    {
        try
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCPSMsgs::midStatus_ConnEndPoint
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_conncfgSock.ipepConn()
                );
            }

            // Try to connect to the remote end point
            m_sockOmni.Connect(m_conncfgSock.ipepConn());
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            return tCQCKit::ECommResults::LostCommRes;
        }
    }

    // Do the encrypted session establishment exchange
    if (!bCreateSession())
        return tCQCKit::ECommResults::LostCommRes;

    //
    //  Query some counts of things supported by this particular model
    //  we are talking about.
    //
    m_c4MaxAreas = c4QueryObjTypeMax(kHAIOmniTCPS::c1ItemType_Area);
    m_c4MaxEnclosures = c4QueryObjTypeMax(kHAIOmniTCPS::c1ItemType_ExpEnclosure);
    m_c4MaxThermos = c4QueryObjTypeMax(kHAIOmniTCPS::c1ItemType_Thermostat);
    m_c4MaxUnits = c4QueryObjTypeMax(kHAIOmniTCPS::c1ItemType_Unit);
    m_c4MaxZones = c4QueryObjTypeMax(kHAIOmniTCPS::c1ItemType_Zone);

    //
    //  Now register our fields. We base this on the configuration data we
    //  loaded in bWaitConfig() above.
    //
    RegisterFields();

    // Now let's get all our fields initialized
    if (!bInitFields())
        return tCQCKit::ECommResults::LostConnection;

    // Reset the last ping time
    m_enctLastPing = TTime::enctNow();

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes THAIOmniTCPSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCIPConnCfg::clsThis()
        );
    }

    // Store our socket configuration info
    m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());

    // Initialize some field id arrays
    const tCIDLib::TCard4 c4Init = kCIDLib::c4MaxCard;
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattery, c4Init, kHAIOmniTCPSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclACOff, c4Init, kHAIOmniTCPSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattLow, c4Init, kHAIOmniTCPSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclCommFail, c4Init, kHAIOmniTCPSh::c4MaxEnclosures);

    //
    //  Get out the driver prompt values we get. We should get a key, in
    //  the form of a 32 character string of hex digits, and a temp format
    //  indicator.
    //
    TString strKey;
    TString strTempFmt;
    if (!cqcdcOurs.bFindPromptValue(L"CryptoKey", L"Text", strKey)
    ||  !cqcdcOurs.bFindPromptValue(L"TempFmt", L"Selected", strTempFmt))
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"One or more of the required driver prompt values was not present for driver %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }
    m_bDoCelsius = (strTempFmt == L"Celsius");

    // Check the key length
    if (strKey.c4Length() != 32)
    {
        facHAIOmniTCPS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The encryption key prompt value must be 32 characters for driver %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Store the key in an MD5 hash, which makes it much easier for us
    //  to deal with, since it's binary. He expects a dash in between the
    //  two halfs of the hash, so add that.
    //
    try
    {
        strKey.Insert(kCIDLib::chHyphenMinus, 16);
        m_mhashKey.ParseFormatted(strKey);
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facHAIOmniTCPS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Crypto key prompt value was badly formed for driver %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Build up our configuration file
    TPathStr pathCfg = facCQCKit().strServerDataDir();
    pathCfg.AddLevel(L"HAIOmniTCP");
    m_strCfgPath = pathCfg;
    pathCfg.AddLevel(strMoniker());
    pathCfg.AppendExt(L"Cfg");
    m_strCfgFile = pathCfg;

    // We want to poll quickly, because we depend on async notifications
    SetPollTimes(50, 15000);
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults
THAIOmniTCPSDriver::eIntFldValChanged(  const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TInt4  i4NewValue)
{
    try
    {
        // Figure out which field was written to.
        if ((c4FldId >= m_c4FirstThermoFldId) && (c4FldId <= m_c4LastThermoFldId))
        {
            //
            //  It has to be the heat/cool set points, since they are the
            //  only int fields related to thermos, so find this guy.
            //
            THAIOmniThermo* pitemTar = m_dcfgCurrent.pitemOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any thermostat"
                    , tCIDLib::ESeverities::Status
                );
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::Internal;
            }

            //
            //  Ok, we can send the message now. We need to convert the passed
            //  value to Omni temp format.
            //
            const tCIDLib::TBoolean bHeat(pitemTar->c4FldIdHeatSP() == c4FldId);
            const tCIDLib::TCard4 c4OTemp = c4ToOmniTemp(i4NewValue, m_bDoCelsius);
            SendOmniCmd
            (
                bHeat ? kHAIOmniTCPS::c1Cmd_SetHeatSP
                      : kHAIOmniTCPS::c1Cmd_SetCoolSP
                , tCIDLib::TCard1(c4OTemp)
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(kCIDLib::True);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults THAIOmniTCPSDriver::ePollDevice(TThread& thrThis)
{
    try
    {
        // Watch for any async messages
        ProcessAsyncs();

        //
        //  Since we depend on async notifications, we'll periodically
        //  ping the Omni if nothing has come in in the last so many
        //  seconds. So if it's been over the active poll time, we'll send
        //  a system status query.
        //
        const tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TEncodedTime enctCommDiff = enctCur - m_enctLastPing;
        if (enctCommDiff > kHAIOmniTCPS::enctActivePoll)
        {
            //
            //  Reset the last ping immediately just in case we get an
            //  exception below somehow. This way we insure we won't re-
            //  trigger until another period has expired.
            //
            m_enctLastPing = enctCur;

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Doing active poll"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }

            THeapBufPool::TElemJan janPool(&m_splBufs, 64);
            TMemBuf& mbufIn = *janPool;
            SendOmniMsg(kHAIOmniTCPS::c1MsgType_SysStatusReq);
            WaitReply
            (
                3000, kHAIOmniTCPS::c1MsgType_SysStatusReply, 0, mbufIn
            );
            StoreSysStatus(mbufIn);

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Back from active poll"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniTCPS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"An exception occured during poll callback"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
THAIOmniTCPSDriver::eStringFldValChanged(const  TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    try
    {
        // Figure out which field was written to.
        tCIDLib::TCard1 c1Cmd;
        tCIDLib::TCard1 c1Val;
        if (c4FldId == m_c4FldId_InvokeCmd)
        {
            ProcessUserCmd(strNewValue);
        }
         else if ((c4FldId >= m_c4FirstThermoFldId)
              &&  (c4FldId <= m_c4LastThermoFldId))
        {
            //
            //  It has to be the mode or fan mode, so find the thermostat
            //  that owns this field.
            //
            THAIOmniThermo* pitemTar = m_dcfgCurrent.pitemOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCPS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any thermostat"
                    , tCIDLib::ESeverities::Status
                );
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::Internal;
            }

            //
            //  Convert the passed fan mode to a number. Based on which
            //  field was being written, we set the command and value to
            //  write. Note that the maps have unknown as the last value,
            //  so we search for < the count-1, so if we don't find it, we
            //  end up on unknown.
            //
            if (pitemTar->c4FldIdFanMode() == c4FldId)
            {
                c1Val = 0;
                while (c1Val < kHAIOmniTCPS::c4ThermoFanModeCnt - 1)
                {
                    if (strNewValue == kHAIOmniTCPS::apszThermoFanModes[c1Val])
                        break;
                    c1Val++;
                }
                c1Cmd = kHAIOmniTCPS::c1Cmd_SetThermoFanMode;
            }
             else if (pitemTar->c4FldIdMode() == c4FldId)
            {
                c1Val = 0;
                while (c1Val < kHAIOmniTCPS::c4ThermoModeCnt - 1)
                {
                    if (strNewValue == kHAIOmniTCPS::apszThermoModes[c1Val])
                        break;
                    c1Val++;
                }
                c1Cmd = kHAIOmniTCPS::c1Cmd_SetThermoMode;
            }
            SendOmniCmd(c1Cmd, c1Val, tCIDLib::TCard2(pitemTar->c4ItemNum()));
            bWaitAck(kCIDLib::True);
        }
         else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::FieldNotFound;
        }
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid THAIOmniTCPSDriver::ReleaseCommResource()
{
    // If our socket is opened, then close the session
    if (m_sockOmni.bIsOpen())
    {
        try
        {
            SendPacket(kHAIOmniTCPS::c1PackType_TermSess);
        }

        catch(const TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        }
    }

    // Make sure the socket is shutdown
    try
    {
        if (m_sockOmni.bIsConnected())
            m_sockOmni.c4Shutdown();
    }

    catch(...) {}

    // And then close it
    try
    {
        m_sockOmni.Close();
    }

    catch(...) {}
}


tCIDLib::TVoid THAIOmniTCPSDriver::TerminateImpl()
{
    //
    //  Call the common init method to reset a lot of stuff back to their
    //  defaults.
    //
    DoCommonInit();

    // Reset the configuration object to clean up everything
    m_dcfgCurrent.Reset();
}


// ---------------------------------------------------------------------------
//  THAIOmniTCPSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Reads the config in from the config file
tCIDLib::TBoolean THAIOmniTCPSDriver::bReadCfgFile()
{
    // If no file, then just return false now
    if (!TFileSys::bExists(m_strCfgFile, tCIDLib::EDirSearchFlags::NormalFiles))
        return kCIDLib::False;

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
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // It was there but something is awry, return false
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Do some basic setup of members to their loaded defaults
tCIDLib::TVoid THAIOmniTCPSDriver::DoCommonInit()
{
    // Reset some constant values
    m_c4ThermoHigh = 0;
    m_c4ThermoLow = kHAIOmniTCPSh::c4MaxThermos;
    m_c4ZoneHigh = 0;
    m_c4ZoneLow = kHAIOmniTCPSh::c4MaxZones;

    m_c4CntAreas = 0;
    m_c4CntEnclosures = 0;
    m_c4CntThermos = 0;
    m_c4CntUnits = 0;
    m_c4CntZones = 0;

    m_c4MaxAreas = 0;
    m_c4MaxEnclosures = 0;
    m_c4MaxThermos = 0;
    m_c4MaxUnits = 0;
    m_c4MaxZones = 0;

    m_c4FirstAreaFldId   = kCIDLib::c4MaxCard;
    m_c4FirstThermoFldId = kCIDLib::c4MaxCard;
    m_c4FirstUnitFldId   = kCIDLib::c4MaxCard;
    m_c4FirstZoneFldId   = kCIDLib::c4MaxCard;
    m_c4FldId_InvokeCmd  = kCIDLib::c4MaxCard;
    m_c4FldId_MainBattery= kCIDLib::c4MaxCard;
    m_c4FldId_TimeValid  = kCIDLib::c4MaxCard;
    m_c4LastAreaFldId    = kCIDLib::c4MaxCard;
    m_c4LastThermoFldId  = kCIDLib::c4MaxCard;
    m_c4LastUnitFldId    = kCIDLib::c4MaxCard;
    m_c4LastZoneFldId    = kCIDLib::c4MaxCard;
}


// Writes out config out to the config file
tCIDLib::TVoid THAIOmniTCPSDriver::WriteCfgFile()
{
    // Make sure the path part exists
    TFileSys::MakePath(m_strCfgPath);

    TBinFileOutStream strmOut
    (
        m_strCfgFile
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );
    strmOut << m_dcfgCurrent << kCIDLib::FlushIt;
}

