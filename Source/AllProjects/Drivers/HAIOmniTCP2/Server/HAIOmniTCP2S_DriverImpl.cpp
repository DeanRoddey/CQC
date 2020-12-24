//
// FILE NAME: HAIOmniTCP2S_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/06/2014
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
//  This is a driver which implements the V2 HAI Omni TCP interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2S.hpp"
#include    "DevClsHdr_Security.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniTCP2S,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniTCP2S
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THAIOmniTCP2S: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniTCP2S::THAIOmniTCP2S(const TCQCDriverObjCfg& cqcdcToLoad) :

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

THAIOmniTCP2S::~THAIOmniTCP2S()
{
}



// ---------------------------------------------------------------------------
//  THAIOmniTCP2S: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Handle some queries that can be made against security related drivers
//  such as this.
//
tCIDLib::TBoolean
THAIOmniTCP2S::bQueryTextVal(const  TString&    strQueryType
                            , const TString&    strDataName
                            ,       TString&    strToFill)
{
    strToFill.Clear();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (strQueryType == kDevCls_Security::pszSecQN_SecPanelInfo)
    {
        if (strDataName == kDevCls_Security::pszSecQT_AreaList)
        {
            // Give him a list of area names as a quoted comma list
            const tCIDLib::TCard4 c4Count = kHAIOmniTCP2Sh::c4MaxAreas;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                // See if we have an item at this slot
                tCIDLib::TCard4 c4At;
                THAIOmniArea* pitemCur = m_dcfgCurrent.pitemAreaById
                (
                    tCIDLib::TCard2(c4Index), c4At
                );

                if (!pitemCur)
                    continue;

                // If not the first one, add a comma
                if (!strToFill.bIsEmpty())
                    strToFill.Append(L", ");

                // Do the name, quoted
                strToFill.Append(kCIDLib::chQuotation);
                strToFill.Append(pitemCur->strName());
                strToFill.Append(kCIDLib::chQuotation);
            };
            bRet = kCIDLib::True;
        }
         else if (strDataName == kDevCls_Security::pszSecQT_SecArmInfo)
        {
            //
            //  First a comma separated list of arming modes, but skipping
            //  the 0th one, which is disarm.
            //
            tHAIOmniTCP2Sh::EArmModes eMode = tHAIOmniTCP2Sh::EArmModes::Min;
            eMode++;
            const tHAIOmniTCP2Sh::EArmModes eStart = eMode;
            while (eMode <= tHAIOmniTCP2Sh::EArmModes::Max)
            {
                if (eMode > eStart)
                    strToFill.Append(L',');
                strToFill.Append(tHAIOmniTCP2Sh::strXlatEArmModes(eMode));
                eMode++;
            }
            bRet = kCIDLib::True;
        }
    }
     else if (strQueryType == kDevCls_Security::pszSecQN_AreaAlarms)
    {
        THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strDataName, kCIDLib::False);
        if (pitemArea)
        {
            //
            //  Just read the current value of the area's alarm list field. We'll
            //  just rebuild it up into a quoted comma list.
            //
            tCIDLib::TCard4 c4SerialNum = 0;
            bReadSListFld
            (
                pitemArea->c4FldIdAlarms(), c4SerialNum, m_colTmpList, kCIDLib::True
            );

            const tCIDLib::TCard4 c4Cnt = m_colTmpList.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Cnt; c4Index++)
                TStringTokenizer::BuildQuotedCommaList(m_colTmpList[c4Index], strToFill);

            bRet = kCIDLib::True;
        }
    }
     else if (strQueryType == kDevCls_Security::pszSecQN_AreaZones)
    {
        //
        //  Returns a quoted comma list of the zones associated with an indicated
        //  area. So loop through all the zones and format out the ones that report
        //  belonging to the area. First look up the area and get its area number.
        //
        THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strDataName, kCIDLib::False);
        if (pitemArea)
        {
            const tCIDLib::TCard4 c4ZCnt = m_dcfgCurrent.c4ZoneCnt();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ZCnt; c4Index++)
            {
                const THAIOmniZone& itemCur = m_dcfgCurrent.itemZoneAt(c4Index);

                if (itemCur.c4AreaNum() == pitemArea->c2ItemNum())
                    TStringTokenizer::BuildQuotedCommaList(itemCur.strName(), strToFill);
            }
            bRet = kCIDLib::True;
        }
    }
     else if (strQueryType == kDevCls_Security::pszSecQN_AreaZStats)
    {
        //
        //  Return a quoted comma list of all of the zone status fields for the
        //  indicated area.
        //
        THAIOmniArea* pitemArea = m_dcfgCurrent.pitemFindArea(strDataName, kCIDLib::False);
        if (pitemArea)
        {
            TString strName;
            const tCIDLib::TCard4 c4ZCnt = m_dcfgCurrent.c4ZoneCnt();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ZCnt; c4Index++)
            {
                const THAIOmniZone& itemCur = m_dcfgCurrent.itemZoneAt(c4Index);

                // If it's in this area and is an alarm type
                if ((itemCur.c4AreaNum() == pitemArea->c2ItemNum())
                &&  itemCur.bAlarmType())
                {
                    strName = L"SEC#Zone_";
                    strName.Append(itemCur.strName());
                    strName.Append(L"_Status");
                    TStringTokenizer::BuildQuotedCommaList(strName, strToFill);
                }
            }
            bRet = kCIDLib::True;
        }
    }
    return bRet;
}


//
//  Note that CQCServer, who calls these, does a call/field lock for the set
//  config, and a call lock for the query, so we are good on sync here.
//
tCIDLib::TCard4 THAIOmniTCP2S::c4QueryCfg(THeapBuf& mbufToFill)
{
    // Create an output stream over the buffer and stream out config to it
    TBinMBufOutStream strmTar(&mbufToFill);
    strmTar << m_dcfgCurrent << kCIDLib::FlushIt;
    return strmTar.c4CurSize();
}


tCIDLib::TVoid
THAIOmniTCP2S::SetConfig(const  tCIDLib::TCard4 c4Bytes
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
//  THAIOmniTCP2S: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniTCP2S::bGetCommResource(TThread& thrThis)
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


tCIDLib::TBoolean THAIOmniTCP2S::bWaitConfig(TThread& thrThis)
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
            TCfgServerClient cfgcRepo(kHAIOmniTCP2S::c4WaitReply);
            TString strCfgKey(L"/Drivers/HAIOmniTCPS/%(m)/CfgData");
            strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);
            tCIDLib::TCard4 c4Ver = 0;
            const tCIDLib::ELoadRes eRes = cfgcRepo.eReadObject
            (
                strCfgKey, m_dcfgCurrent, c4Ver
            );

            // If we didn't get any data, then move forward with an empty config
            if (eRes != tCIDLib::ELoadRes::NewData)
                m_dcfgCurrent.Reset();

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
THAIOmniTCP2S::eBoolFldValChanged(  const   TString&            strName
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
                facHAIOmniTCP2S().LogMsg
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
                kHAIOmniTCP2S::c1Cmd_SetThermoHoldMode
                , bNewValue ? 0xFF : 0
                , pitemTar->c2ItemNum()
            );
            bWaitAck(kCIDLib::True);
        }
         else if ((c4FldId >= m_c4FirstUnitFldId) && (c4FldId <= m_c4LastUnitFldId))
        {
            THAIOmniUnit* pitemTar = m_dcfgCurrent.pitemOwningUnit(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniTCP2S().LogMsg
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
            if (pitemTar->eUnitType() == tHAIOmniTCP2Sh::EUnitTypes::Switch)
            {
                SendOmniCmd
                (
                    bNewValue ? kHAIOmniTCP2S::c1Cmd_UnitOn : kHAIOmniTCP2S::c1Cmd_UnitOff
                    , 0
                    , pitemTar->c2ItemNum()
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
                        , TCardinal(pitemTar->c2ItemNum())
                        , pitemTar->strName()
                        , TString::strEmpty()
                    );
                }
            }
             else
            {
                facHAIOmniTCP2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The target unit is not a switch"
                    , tCIDLib::ESeverities::Status
                );
                return tCQCKit::ECommResults::Internal;
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
THAIOmniTCP2S::eCardFldValChanged(  const   TString&        strName
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
                facHAIOmniTCP2S().LogMsg
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
            if ((pitemTar->eUnitType() == tHAIOmniTCP2Sh::EUnitTypes::Dimmer)
            ||  (pitemTar->eUnitType() == tHAIOmniTCP2Sh::EUnitTypes::Flag))
            {
                // Make sure the value is valid
                if (pitemTar->eUnitType() == tHAIOmniTCP2Sh::EUnitTypes::Dimmer)
                {
                    if (c4NewValue > 100)
                        return tCQCKit::ECommResults::BadValue;
                }
                 else
                {
                    if (c4NewValue > 255)
                        return tCQCKit::ECommResults::BadValue;
                }

                SendOmniCmd
                (
                    kHAIOmniTCP2S::c1Cmd_UnitLightLev
                    , tCIDLib::TCard1(c4NewValue)
                    , pitemTar->c2ItemNum()
                );
            }
             else
            {
                facHAIOmniTCP2S().LogMsg
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
tCQCKit::ECommResults THAIOmniTCP2S::eConnectToDevice(TThread& thrThis)
{
    // If not connected, then try to connect
    if (!m_sockOmni.bIsConnected())
    {
        try
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::High)
            {
                facHAIOmniTCP2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kOmniTCP2SMsgs::midStatus_ConnEndPoint
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
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
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
    m_c4MaxAreas = c4QueryObjTypeMax(kHAIOmniTCP2S::c1ItemType_Area);
    m_c4MaxEnclosures = c4QueryObjTypeMax(kHAIOmniTCP2S::c1ItemType_ExpEnclosure);
    m_c4MaxThermos = c4QueryObjTypeMax(kHAIOmniTCP2S::c1ItemType_Thermostat);
    m_c4MaxUnits = c4QueryObjTypeMax(kHAIOmniTCP2S::c1ItemType_Unit);
    m_c4MaxZones = c4QueryObjTypeMax(kHAIOmniTCP2S::c1ItemType_Zone);

    //
    //  If our configuration is empty, then we never loaded any config, so try to
    //  import some default config from the device.
    //
    if (m_dcfgCurrent.bIsEmpty())
    {
        try
        {
            ImportOmniCfg(m_dcfgCurrent);

            // It worked, so store the config
            WriteCfgFile();
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                LogMsg
                (
                    L"Failed to import Omni config"
                    , tCQCKit::EVerboseLvls::Medium
                    , CID_FILE
                    , CID_LINE
                );
            }

            m_dcfgCurrent.Reset();
        }
    }

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


tCQCKit::EDrvInitRes THAIOmniTCP2S::eInitializeImpl()
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
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattery, c4Init, kHAIOmniTCP2Sh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclACOff, c4Init, kHAIOmniTCP2Sh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattLow, c4Init, kHAIOmniTCP2Sh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclCommFail, c4Init, kHAIOmniTCP2Sh::c4MaxEnclosures);

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
        facHAIOmniTCP2S().LogMsg
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
        facHAIOmniTCP2S().LogMsg
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
            facHAIOmniTCP2S().LogMsg
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
    pathCfg.AddLevel(L"HAIOmniTCP2");
    m_strCfgPath = pathCfg;
    pathCfg.AddLevel(strMoniker());
    pathCfg.AppendExt(L"Cfg");
    m_strCfgFile = pathCfg;

    // We want to poll quickly, because we depend on async notifications
    SetPollTimes(50, 15000);
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults
THAIOmniTCP2S::eIntFldValChanged(const  TString&        strName
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
                facHAIOmniTCP2S().LogMsg
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
            const tCIDLib::TBoolean bHeat(pitemTar->c4FldIdLowSP() == c4FldId);
            const tCIDLib::TCard4 c4OTemp = c4ToOmniTemp(i4NewValue, m_bDoCelsius);
            SendOmniCmd
            (
                bHeat ? kHAIOmniTCP2S::c1Cmd_SetHeatSP
                      : kHAIOmniTCP2S::c1Cmd_SetCoolSP
                , tCIDLib::TCard1(c4OTemp)
                , pitemTar->c2ItemNum()
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


tCQCKit::ECommResults THAIOmniTCP2S::ePollDevice(TThread& thrThis)
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
        if (enctCommDiff > kHAIOmniTCP2S::enctActivePoll)
        {
            //
            //  Reset the last ping immediately just in case we get an
            //  exception below somehow. This way we insure we won't re-
            //  trigger until another period has expired.
            //
            m_enctLastPing = enctCur;

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCP2S().LogMsg
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
            SendOmniMsg(kHAIOmniTCP2S::c1MsgType_SysStatusReq);
            WaitReply
            (
                3000, kHAIOmniTCP2S::c1MsgType_SysStatusReply, 0, mbufIn
            );
            StoreSysStatus(mbufIn);

            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facHAIOmniTCP2S().LogMsg
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
            facHAIOmniTCP2S().LogMsg
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
THAIOmniTCP2S::eStringFldValChanged(const   TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const TString&        strNewValue)
{
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::Success;

    try
    {
        // Figure out which field was written to.
        tCIDLib::TCard1 c1Cmd;
        tCIDLib::TCard1 c1Val;
        if (c4FldId == m_c4FldId_InvokeCmd)
        {
            eRes = eProcessUserCmd(strNewValue);
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
                facHAIOmniTCP2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The passed field id was not owned by any thermostat"
                    , tCIDLib::ESeverities::Status
                );
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
                tHAIOmniTCP2Sh::EFanModes eMode = tHAIOmniTCP2Sh::eXlatEFanModes(strNewValue);
                if (eMode >= tHAIOmniTCP2Sh::EFanModes::Count)
                    return tCQCKit::ECommResults::CmdParms;

                c1Val = tCIDLib::TCard1(eMode);
                c1Cmd = kHAIOmniTCP2S::c1Cmd_SetThermoFanMode;
            }
             else if (pitemTar->c4FldIdMode() == c4FldId)
            {
                tHAIOmniTCP2Sh::EThermoModes eMode
                (
                    tHAIOmniTCP2Sh::eXlatEThermoModes(strNewValue)
                );
                if (eMode >= tHAIOmniTCP2Sh::EThermoModes::Count)
                    return tCQCKit::ECommResults::CmdParms;

                c1Val = tCIDLib::TCard1(eMode);
                c1Cmd = kHAIOmniTCP2S::c1Cmd_SetThermoMode;
            }
            SendOmniCmd(c1Cmd, c1Val, pitemTar->c2ItemNum());
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
        eRes = tCQCKit::ECommResults::Exception;
    }
    return eRes;
}


tCIDLib::TVoid THAIOmniTCP2S::ReleaseCommResource()
{
    // If our socket is opened, then close the session
    if (m_sockOmni.bIsOpen())
    {
        try
        {
            SendPacket(kHAIOmniTCP2S::c1PackType_TermSess);
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


tCIDLib::TVoid THAIOmniTCP2S::TerminateImpl()
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
//  THAIOmniTCP2S: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Reads the config in from the config file
tCIDLib::TBoolean THAIOmniTCP2S::bReadCfgFile()
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
tCIDLib::TVoid THAIOmniTCP2S::DoCommonInit()
{
    // Reset some constant values
    m_c4ThermoHigh = 0;
    m_c4ThermoLow = kHAIOmniTCP2Sh::c4MaxThermos;
    m_c4ZoneHigh = 0;
    m_c4ZoneLow = kHAIOmniTCP2Sh::c4MaxZones;

    m_c4MaxAreas = 0;
    m_c4MaxEnclosures = 0;
    m_c4MaxThermos = 0;
    m_c4MaxUnits = 0;
    m_c4MaxZones = 0;

    m_c4FirstAreaFldId      = kCIDLib::c4MaxCard;
    m_c4FirstThermoFldId    = kCIDLib::c4MaxCard;
    m_c4FirstUnitFldId      = kCIDLib::c4MaxCard;
    m_c4FirstZoneFldId      = kCIDLib::c4MaxCard;
    m_c4FldId_InvokeCmd     = kCIDLib::c4MaxCard;
    m_c4FldId_MainBattery   = kCIDLib::c4MaxCard;
    m_c4FldId_TimeValid     = kCIDLib::c4MaxCard;
    m_c4LastAreaFldId       = kCIDLib::c4MaxCard;
    m_c4LastThermoFldId     = kCIDLib::c4MaxCard;
    m_c4LastUnitFldId       = kCIDLib::c4MaxCard;
    m_c4LastZoneFldId       = kCIDLib::c4MaxCard;

    // Device info fields
    m_c4FldId_Firmware      = kCIDLib::c4MaxCard;
    m_c4FldId_Model         = kCIDLib::c4MaxCard;
}


// Writes out config out to the config file
tCIDLib::TVoid THAIOmniTCP2S::WriteCfgFile()
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

