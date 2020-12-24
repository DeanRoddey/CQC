//
// FILE NAME: HAIOmniS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/11/2006
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
//  This is a driver which implements the HAI Omni Pro II interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "HAIOmniS.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(THAIOmniSDriver,TCQCServerBase)


namespace HAIOmniS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  The config server key that we store or config in. The %(m) is replaced
    //  by our moniker.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszCfgKey = L"/Drivers/HAIOmniS/%(m)/CfgData";
};



// ---------------------------------------------------------------------------
//   CLASS: THAIOmniSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  THostMonSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
THAIOmniSDriver::THAIOmniSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c2LastSeq(0)
    , m_c4CntAreas(0)
    , m_c4CntThermos(0)
    , m_c4CntUnits(0)
    , m_c4CntZones(0)
    , m_c4FirstAreaFldId(kCIDLib::c4MaxCard)
    , m_c4FirstThermoFldId(kCIDLib::c4MaxCard)
    , m_c4FirstZoneFldId(kCIDLib::c4MaxCard)
    , m_c4FldId_InvokeCmd(kCIDLib::c4MaxCard)
    , m_c4FldId_MainBattery(kCIDLib::c4MaxCard)
    , m_c4LastAreaFldId(kCIDLib::c4MaxCard)
    , m_c4LastThermoFldId(kCIDLib::c4MaxCard)
    , m_c4LastZoneFldId(kCIDLib::c4MaxCard)
    , m_c4NextThermoBlk(0)
    , m_c4NextUnitBlk(0)
    , m_c4NextZoneBlk(0)
    , m_c4ThermoHigh(kCIDLib::c4MaxCard)
    , m_c4ThermoLow(kCIDLib::c4MaxCard)
    , m_c4TimeoutCnt(0)
    , m_c4ZoneHigh(kCIDLib::c4MaxCard)
    , m_c4ZoneLow(kCIDLib::c4MaxCard)
    , m_enctNextEvent(0)
    , m_enctNextSys(0)
    , m_enctNextThermo(0)
    , m_enctNextUnit(0)
    , m_enctNextZone(0)
    , m_fcolAreas(kHAIOmniSh::c4MaxThermos, 0)
    , m_fcolThermos(kHAIOmniSh::c4MaxThermos, 0)
    , m_fcolUnits(kHAIOmniSh::c4MaxUnits, 0)
    , m_fcolZones(kHAIOmniSh::c4MaxZones, 0)
    , m_mbufAck(kHAIOmniS::c4MaxMsgSize, kHAIOmniS::c4MaxMsgSize)
    , m_mbufPacket(kHAIOmniS::c4MaxPacketSize, kHAIOmniS::c4MaxPacketSize)
    , m_mbufRead(kHAIOmniS::c4MaxMsgSize, kHAIOmniS::c4MaxMsgSize)
    , m_mbufWrite(kHAIOmniS::c4MaxMsgSize, kHAIOmniS::c4MaxMsgSize)
    , m_sockOmni()
    , m_strEvButton(L"Button")
    , m_strEvUPBLink(L"UPBLink")
    , m_strEvX10Code(L"X10Code")
{
}

THAIOmniSDriver::~THAIOmniSDriver()
{
}


// ---------------------------------------------------------------------------
//  THAIOmniSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean THAIOmniSDriver::bGetCommResource(TThread& thrThis)
{
    try
    {
        m_sockOmni.Open(tCIDSock::ESockProtos::UDP, m_ipepOmni.eAddrType());
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean THAIOmniSDriver::bWaitConfig(TThread& thrThis)
{
    try
    {
        //
        //  Try to read it, it if fails, just reset our config. This will convert old
        //  config repo based info to the text file format if found and possible.
        //
        if (!bReadCfgFile())
            m_dcfgCurrent.Reset();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCQCKit::ECommResults
THAIOmniSDriver::eBoolFldValChanged(const   TString&            strName
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
            THAIOmniThermo* pitemTar = pitemFindOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniS().LogMsg
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
                kHAIOmniS::c1Cmd_SetThermoHoldMode
                , bNewValue ? 0xFF : 0
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(3000, kCIDLib::True);
        }
         else if ((c4FldId >= m_c4FirstUnitFldId) && (c4FldId <= m_c4LastUnitFldId))
        {
            THAIOmniUnit* pitemTar = pitemFindOwningUnit(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniS().LogMsg
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
            if (pitemTar->eType() != tHAIOmniSh::EUnit_Binary)
            {
                facHAIOmniS().LogMsg
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
                bNewValue ? kHAIOmniS::c1Cmd_UnitOn : kHAIOmniS::c1Cmd_UnitOff
                , 0
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(3000, kCIDLib::True);
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
THAIOmniSDriver::eCardFldValChanged(const   TString&        strName
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4NewValue)
{
    try
    {
        if ((c4FldId >= m_c4FirstUnitFldId) && (c4FldId <= m_c4LastUnitFldId))
        {
            THAIOmniUnit* pitemTar = pitemFindOwningUnit(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniS().LogMsg
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
            if (pitemTar->eType() == tHAIOmniSh::EUnit_Dimmer)
            {
                SendOmniCmd
                (
                    kHAIOmniS::c1Cmd_UnitLightLev
                    , tCIDLib::TCard1((c4NewValue > 100) ? 100 : c4NewValue)
                    , tCIDLib::TCard2(pitemTar->c4ItemNum())
                );
            }
             else if (pitemTar->eType() == tHAIOmniSh::EUnit_Flag)
            {
                SendOmniCmd
                (
                    kHAIOmniS::c1Cmd_SetCounter
                    , tCIDLib::TCard1(c4NewValue & 0xFF)
                    , tCIDLib::TCard2(pitemTar->c4ItemNum())
                );
            }
             else
            {
                facHAIOmniS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The target unit is not a dimmer or flag unit"
                    , tCIDLib::ESeverities::Status
                );
                return tCQCKit::ECommResults::Internal;
            }
            bWaitAck(3000, kCIDLib::True);
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
tCQCKit::ECommResults THAIOmniSDriver::eConnectToDevice(TThread& thrThis)
{
    // Do the encrypted session establishment exchange
    if (!bCreateSession())
        return tCQCKit::ECommResults::LostConnection;

    // We have to log on successfully before we can do anything
    if (!bDoLogin())
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Login to Omni failed, lowering reconnect time"
                , tCIDLib::ESeverities::Status
            );
        }
        return tCQCKit::ECommResults::LostConnection;
    }

    //
    //  Now register our fields. We base this on the configuration data we
    //  loaded in bWaitConfig() above. This will reset any polling related
    //  stuff to get ready for polling.
    //
    RegisterFields();

    // Now let's get all our fields initialized
    if (!bInitFields())
        return tCQCKit::ECommResults::LostConnection;

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes THAIOmniSDriver::eInitializeImpl()
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

    // Just get the IP end point out of the config. That's all we need
    m_ipepOmni = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal()).ipepConn();

    // Initialize some field id arrays
    const tCIDLib::TCard4 c4Init = kCIDLib::c4MaxCard;
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattery, c4Init, kHAIOmniSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclACOff, c4Init, kHAIOmniSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclBattLow, c4Init, kHAIOmniSh::c4MaxEnclosures);
    TRawMem::SetMemBuf(m_ac4FldId_EnclCommFail, c4Init, kHAIOmniSh::c4MaxEnclosures);

    //
    //  Get out the driver prompt values we get. We should get a key, in
    //  the form of a 32 character string of hex digits, and a 4 character
    //  code that we use to log into the Omni.
    //
    TString strKey;
    TString strTempFmt;
    if (!cqcdcOurs.bFindPromptValue(L"CryptoKey", L"Text", strKey)
    ||  !cqcdcOurs.bFindPromptValue(L"LoginId", L"Text", m_strLogin)
    ||  !cqcdcOurs.bFindPromptValue(L"TempFmt", L"Selected", strTempFmt))
    {
        facHAIOmniS().LogMsg
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

    // Check the lengths
    if (strKey.c4Length() != 32)
    {
        facHAIOmniS().LogMsg
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

    if (m_strLogin.c4Length() != 4)
    {
        facHAIOmniS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The login prompt value must be 4 characters"
            , tCIDLib::ESeverities::Status
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
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facHAIOmniS().LogMsg
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
    pathCfg.AddLevel(L"HAIOmni");
    m_strCfgPath = pathCfg;

    pathCfg.AddLevel(strMoniker());
    pathCfg.Append(L"_Cfg");
    pathCfg.AppendExt(L"Txt");
    m_strCfgFile = pathCfg;

    SetPollTimes(100, 20000);
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults
THAIOmniSDriver::eIntFldValChanged( const   TString&        strName
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
            THAIOmniThermo* pitemTar = pitemFindOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniS().LogMsg
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
                bHeat ? kHAIOmniS::c1Cmd_SetHeatSP : kHAIOmniS::c1Cmd_SetCoolSP
                , tCIDLib::TCard1(c4OTemp)
                , tCIDLib::TCard2(pitemTar->c4ItemNum())
            );
            bWaitAck(3000, kCIDLib::True);
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


tCQCKit::ECommResults THAIOmniSDriver::ePollDevice(TThread& thrThis)
{
    try
    {
        // Decide if it's time to poll events
        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
        const tCIDLib::TBoolean bDoEvents(enctCur >= m_enctNextEvent);

        //
        //  Every other time through we'll check for new system events. If
        //  we are in arm countdown mode, poll every time since there's
        //  likely to be a lot of activity.
        //
        if (bDoEvents)
            CheckSysEvents();

        //
        //  Now do the others. An arm event could have adjusted the next poll
        //  time for these, so we have to do them after events.
        //
        enctCur = TTime::enctNow();
        const tCIDLib::TBoolean bDoSys(enctCur >= m_enctNextSys);
        const tCIDLib::TBoolean bDoThermos(enctCur >= m_enctNextThermo);
        const tCIDLib::TBoolean bDoUnits(enctCur >= m_enctNextUnit);
        const tCIDLib::TBoolean bDoZones(enctCur >= m_enctNextZone);

        //
        //  Since we use system events to track security status, the only
        //  things we care about in here during polling is stuff that won't
        //  change very often (battery and enclosure status stuff.)
        //
        if (bDoSys)
        {
            SendOmniMsg(kHAIOmniS::c1MsgType_SysStatus);
            WaitReply(4000, kHAIOmniS::c1MsgType_SysStatusReply);
            StoreSysStatus(kCIDLib::False);
        }

        // Poll other stuff as appropriate
        if (bDoZones)
            PollZones(kCIDLib::False);

        if (bDoThermos)
            PollThermos(kCIDLib::False);

        if (bDoUnits)
            PollUnits(kCIDLib::False);

        // Obviously no timeout this time so reset the counter
        m_c4TimeoutCnt = 0;
    }

    catch(const TError& errToCatch)
    {
        //
        //  We'll allow 4 consequtive timeouts before we give up. So if it's
        //  a timeout, bump the counter. If we've hit 4, then recycle the
        //  connection, else fall through. Otherwise, panic and reset the
        //  whole thing.
        //
        if (errToCatch.bCheckEvent( facHAIOmniS().strName()
                                    , kOmniSErrs::errcProto_Timeout))
        {
            m_c4TimeoutCnt++;
            if (m_c4TimeoutCnt >= 4)
            {
                LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
                return tCQCKit::ECommResults::LostConnection;
            }
        }
         else
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
            return tCQCKit::ECommResults::LostCommRes;
        }
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
THAIOmniSDriver::eStringFldValChanged(  const   TString&        strName
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
            THAIOmniThermo* pitemTar = pitemFindOwningThermo(c4FldId);
            if (!pitemTar)
            {
                facHAIOmniS().LogMsg
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
                while (c1Val < kHAIOmniS::c4ThermoFanModeCnt - 1)
                {
                    if (strNewValue == kHAIOmniS::apszThermoFanModes[c1Val])
                        break;
                    c1Val++;
                }
                c1Cmd = kHAIOmniS::c1Cmd_SetThermoFanMode;
            }
             else if (pitemTar->c4FldIdMode() == c4FldId)
            {
                c1Val = 0;
                while (c1Val < kHAIOmniS::c4ThermoModeCnt - 1)
                {
                    if (strNewValue == kHAIOmniS::apszThermoModes[c1Val])
                        break;
                    c1Val++;
                }
                c1Cmd = kHAIOmniS::c1Cmd_SetThermoMode;
            }
            SendOmniCmd(c1Cmd, c1Val, tCIDLib::TCard2(pitemTar->c4ItemNum()));
            bWaitAck(2000, kCIDLib::True);
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


tCIDLib::TVoid THAIOmniSDriver::ReleaseCommResource()
{
    // If our socket is opened, then log out and send a close session
    if (m_sockOmni.bIsOpen())
    {
        try
        {
            SendOmniMsg(kHAIOmniS::c1MsgType_Logout);
            SendPacket(kHAIOmniS::c1PackType_TermSess);
        }

        catch(const TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        }
    }

    try
    {
        m_sockOmni.Close();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid THAIOmniSDriver::TerminateImpl()
{
    // Clean up any named items we allocated
    ReleaseNamedItems();
}



// ---------------------------------------------------------------------------
//  THAIOmniSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Reads the config in from the config file
tCIDLib::TBoolean THAIOmniSDriver::bReadCfgFile()
{
    //
    //  If no file, then see if the old config exists. If so, we can read that in and
    //  spit out the text file.
    //
    if (!TFileSys::bExists(m_strCfgFile, tCIDLib::EDirSearchFlags::NormalFiles))
    {
        TCfgServerClient cfgcRepo(2000);
        TString strCfgKey(HAIOmniS_DriverImpl::pszCfgKey);
        strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);
        tCIDLib::TCard4 c4Ver = 0;
        const tCIDLib::ELoadRes eRes = cfgcRepo.eReadObject
        (
            strCfgKey, m_dcfgCurrent, c4Ver
        );

        // If no new data, then we just don't have any
        if (eRes != tCIDLib::ELoadRes::NewData)
            return kCIDLib::False;

        // Make sure the directory exists
        TFileSys::MakePath(m_strCfgPath);

        // And create the new style text file and format it out
        TTextFileOutStream strmTar
        (
            m_strCfgFile
            , tCIDLib::ECreateActs::CreateIfNew
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Write
            , new TUTF8Converter
        );

        m_dcfgCurrent.FormatTo(strmTar);
    }

    try
    {
        TTextFileInStream strmSrc
        (
            m_strCfgFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Read
            , new TUTF8Converter
        );

        if (!m_dcfgCurrent.bParseFrom(strmSrc))
        {
            facHAIOmniS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kOmniSErrs::errcCfg_ParseFile
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Init
            );
            return kCIDLib::False;
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

        // It was there but something is awry, return false
        return kCIDLib::False;
    }
    return kCIDLib::True;
}
