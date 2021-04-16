//
// FILE NAME: ZWaveLevi2S_DriverImpl.cpp
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
#include    "ZWaveLevi2S_.hpp"



// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveLevi2SDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveLevi2SDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveLevi2SDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveLevi2SDriver::TZWaveLevi2SDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bEnableTrace(kCIDLib::False)
    , m_bNewConfig(kCIDLib::False)
    , m_bNoProcess(kCIDLib::False)
    , m_bsNodeMap(kZWaveLevi2Sh::c4MaxUnitId)
    , m_c4CfgSerialNum(1)
    , m_c4FldId_InvokeCmd(kCIDLib::c4MaxCard)
    , m_c4NextPollInd(0)
    , m_c4TOCount(0)
    , m_c4VerMaj(0)
    , m_c4VerMin(0)
    , m_enctNextPoll(0)
    , m_enctNextTrans(0)
    , m_fcolGTypeMap(kCIDLib::c4MaxCard)
    , m_mbufOut(512, 512)
    , m_pcommZWave(0)
    , m_strmTrace(new TUSASCIIConverter())
{
    // Set up our generic type map
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::None]        = kCIDLib::c4MaxCard;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::GenCtrl]     = GENERIC_TYPE_GENERIC_CONTROLLER;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::BinSensor]   = GENERIC_TYPE_SENSOR_BINARY;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::BinSwitch]   = GENERIC_TYPE_SWITCH_BINARY;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::EntryCtrl]   = GENERIC_TYPE_ENTRY_CONTROL;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::MLSensor]    = GENERIC_TYPE_SENSOR_MULTILEVEL;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::MLSwitch]    = GENERIC_TYPE_SWITCH_MULTILEVEL;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::Thermo]      = GENERIC_TYPE_THERMOSTAT;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::StaticCtrl]  = GENERIC_TYPE_STATIC_CONTROLLER;
    m_fcolGTypeMap[tZWaveLevi2Sh::EGenTypes::NotSensor]   = GENERIC_TYPE_SENSOR_NOTIFICATION;
}

TZWaveLevi2SDriver::~TZWaveLevi2SDriver()
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
//  TZWaveLevi2SDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  We support some of these types of backdoor commands.
//
tCIDLib::TBoolean
TZWaveLevi2SDriver::bQueryTextVal(  const   TString&    strQueryType
                                    , const TString&    strDataName
                                    ,       TString&    strToFill)
{
    // Make sure no matter what that we don't stream back bogus data
    strToFill.Clear();

    tCIDLib::TBoolean bRet = kCIDLib::True;
    if (strQueryType == kZWaveLevi2Sh::strQTextV_ProbeUnitReport)
    {
        //
        //  Note that this is going to lock out the regular functioning of the driver
        //  for a while, long enough probably to cause other clients to time out if
        //  they send any commands.
        //
        TTextStringOutStream strmTar(&strToFill);
        ProbeUnit(strDataName, strmTar);
        strmTar.Flush();
    }
     else if (strQueryType == kZWaveLevi2Sh::strQTextV_QueryAssoc)
    {
        //
        //  The dataname is in the form 'nodename/group', so break thouse out into
        //  separate values.
        //
        TString strName = strDataName;
        TString strGrp;
        if (!strName.bSplit(strGrp, L'/'))
        {
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcQuery_Assoc
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(strName);
        tCIDLib::TCard4 c4GrpId = strGrp.c4Val();

        // See if it implements the association class
        TZWCmdClass* pzwccToUse = unitSrc.pzwccFind(tZWaveLevi2Sh::ECClasses::Association, 0);
        if (!pzwccToUse)
        {
            strToFill = L"This unit does not support associations";
            return kCIDLib::False;
        }

        // Let the association class process it, casting it to its actual type
        TZWCCAssociation* pzwccAssoc = static_cast<TZWCCAssociation*>(pzwccToUse);
        tCIDLib::TCardList fcolVals(16);
        bRet = pzwccAssoc->bQueryAssociation(unitSrc, c4GrpId, fcolVals, *this);

        if (bRet)
        {
            const tCIDLib::TCard4 c4ByteCnt = fcolVals.c4ElemCount();
            for (tCIDLib::TCard4 c4BInd = 0; c4BInd < c4ByteCnt; c4BInd++)
            {
                if (c4BInd)
                    strToFill.Append(L",");
                strToFill.AppendFormatted(fcolVals[c4BInd]);
            }
        }
    }
     else
    {
        bRet = kCIDLib::False;
    }
    return bRet;
}


//
//  Standard driver backdoor method to return the driver configuration info.
//  The format is purely between the client and server driver sides.
//
tCIDLib::TCard4 TZWaveLevi2SDriver::c4QueryCfg(THeapBuf& mbufToFill)
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
//  We provide a simple status query for the client driver. He calls this with the
//  'no queue' option, so it's not queued up, so he can get to this info even if we are
//  off doing a scan and such.
//
//  SO WE HAVE TO BE CAREFUL. We are only returning basic values here, so it should be
//  safe.
//
tCIDLib::TCard4 TZWaveLevi2SDriver::c4QueryVal(const TString& strValId)
{
    tCIDLib::TCard4 c4Ret = 0;
    if (strValId == kZWaveLevi2Sh::strQCardV_Status)
    {
        // First get the config serial number
        c4Ret = m_c4CfgSerialNum;

        // And then if we are in rescan mode, set the high bit
        if (m_bNoProcess)
            c4Ret |= 0x80000000;
    }
    return c4Ret;
}


//
//  This is used to support the querying of configuration parameters from units.
//
tCIDLib::TCard4
TZWaveLevi2SDriver::c4QueryVal2(const TString& strValId, const TString& strValName)
{
    tCIDLib::TCard4 c4Ret = 0;
    if (strValId == kZWaveLevi2Sh::strQCardV_CfgParam)
    {
        // The name is in the form 'nodename/parm#'
        TString strName = strValName;
        TString strParm;
        if (!strName.bSplit(strParm, L'/'))
        {
            facZWaveLevi2S().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcQuery_CfgParm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(strName);
        tCIDLib::TCard4 c4CfgParm = strParm.c4Val();

        // See if it implements the association class
        TZWCmdClass* pzwccToUse = unitSrc.pzwccFind(tZWaveLevi2Sh::ECClasses::Configuration, 0);
        if (!pzwccToUse)
            return kCIDLib::c4MaxCard;

        // Let the config class process it, casting it to its actual type
        TZWCCConfig* pzwccConfig = static_cast<TZWCCConfig*>(pzwccToUse);
        if (!pzwccConfig->bQueryConfigParm(unitSrc, c4CfgParm, c4Ret, *this))
            c4Ret = kCIDLib::c4MaxCard;
    }
     else
    {
        facZWaveLevi2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcCfg_UnknownQuery2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strValId
            , strValName
        );
    }
    return c4Ret;
}


//
//  We support a few backdoor commands that aren't done via the field interface.
//
tCIDLib::TCard4
TZWaveLevi2SDriver::c4SendCmd(const TString& strCmd, const TString& strParms)
{
    tCIDLib::TCard4 c4Ret = 0;
    if (strCmd == kZWaveLevi2Sh::strSendCmd_StartScan)
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
     else if (strCmd == kZWaveLevi2Sh::strSendCmd_AutoCfg)
    {
        TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(strParms);
        DoAutoCfg(unitTar);
    }
     else
    {
        facZWaveLevi2S().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcCfg_UnknownCmd
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strCmd
        );
    }
    return c4Ret;
}


//
//  Handle send data backdoor calls from the client side driver. This one is
//  used for setting configuration.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::SetConfig(const tCIDLib::TCard4 c4Bytes
                            , const THeapBuf&       mbufNewCfg)
{
    //
    //  Create an input stream and stream in our configuration data. Allow
    //  any exceptions to propogate back to the caller.
    //
    TBinMBufInStream strmSrc(&mbufNewCfg, c4Bytes);

    // Stream in the full config that should follow this. Use a temp initially
    TZWaveDrvConfig dcfgNew;
    strmSrc >> dcfgNew;

    // It worked, so write it out
    WriteCfgFile();

    //
    //  Store it as new config and set the flag. The poll thread will pick it up
    //  and process it.
    //
    m_dcfgUpload = dcfgNew;
    m_bNewConfig = kCIDLib::True;
}



tCIDLib::TBoolean TZWaveLevi2SDriver::bLeviFldExists(const TString& strName) const
{
    return bFieldExists(strName);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviQueryFldName(const tCIDLib::TCard4 c4FldId, TString& strToFill) const
{
    return bQueryFldName(c4FldId, strToFill);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviStoreBoolFld(  const   tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bToStore)
{
    return bStoreBoolFld(c4FldId, bToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviStoreCardFld(  const   tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4ToStore)
{
    return bStoreCardFld(c4FldId, c4ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviStoreFloatFld( const   tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TFloat8    f8ToStore)
{
    return bStoreFloatFld(c4FldId, f8ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviStoreIntFld(const  tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TInt4  i4ToStore)
{
    return bStoreIntFld(c4FldId, i4ToStore, kCIDLib::True);
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviStoreStrFld(const  tCIDLib::TCard4 c4FldId
                                    , const TString&        strToStore)
{
    return bStoreStringFld(c4FldId, strToStore, kCIDLib::True);
}

tCIDLib::TBoolean TZWaveLevi2SDriver::bLeviTraceEnabled() const
{
    return m_bEnableTrace;
}

tCIDLib::TBoolean
TZWaveLevi2SDriver::bLeviWaitUnitMsg(       TZWaveUnit&         unitSrc
                                    , const TZWCmdClass&        zwccSrc
                                    , const tCIDLib::TCard4     c4CmdClass
                                    , const tCIDLib::TCard4     c4CmdId
                                    ,       tCIDLib::TCardList& fcolVals)
{
    tZWaveLevi2S::ERepTypes         eGot;
    const tCIDLib::TEncodedTime     enctEnd = TTime::enctNowPlusMSs(kZWaveLevi2S::c4WaitTimeout);
    return bWaitMsg
    (
        tZWaveLevi2S::ERepType_GenReport
        , enctEnd
        , unitSrc.c4Id()
        , zwccSrc.c4InstId()
        , c4CmdClass
        , c4CmdId
        , eGot
        , fcolVals
        , kCIDLib::False
    );
}


tCIDLib::TCard4 TZWaveLevi2SDriver::c4LeviFldIdFromName(const TString& strName) const
{
    return pflddFind(strName, kCIDLib::True)->c4Id();
}

tCIDLib::TCard4 TZWaveLevi2SDriver::c4LeviVRCOPId() const
{
    return m_dcfgCurrent.c4FindVRCOP(kCIDLib::False);
}

tCQCKit::EVerboseLvls TZWaveLevi2SDriver::eLeviVerboseLevel() const
{
    return eVerboseLevel();
}


tCIDLib::TVoid
TZWaveLevi2SDriver::LeviQEventTrig( const   tCQCKit::EStdDrvEvs eEvent
                                    , const TString&            strFld
                                    , const TString&            strVal1
                                    , const TString&            strVal2
                                    , const TString&            strVal3
                                    , const TString&            strVal4)
{
    QueueEventTrig(eEvent, strFld, strVal1, strVal2, strVal3, strVal4);
}


tCIDLib::TVoid
TZWaveLevi2SDriver::LeviSendMsg(const   TZWaveUnit&         unitTar
                                , const tCIDLib::TCardList& fcolVals)
{
    bSendWaitAck
    (
        unitTar.c4Id()
        , 0
        , fcolVals
        , tZWaveLevi2Sh::EMultiTypes::Single
        , kCIDLib::False
        , kCIDLib::True
    );
    bWaitTransAck(kCIDLib::True);
}

tCIDLib::TVoid
TZWaveLevi2SDriver::LeviSendUnitMsg(const   TZWaveUnit&         unitTar
                                    , const TZWCmdClass&        zwccSrc
                                    , const tCIDLib::TCardList& fcolVals)
{
    bSendWaitAck(unitTar, zwccSrc, fcolVals, kCIDLib::True);
    bWaitTransAck(kCIDLib::True);
}

tCIDLib::TVoid
TZWaveLevi2SDriver::LeviSendUnitMsg(const   TZWaveUnit&     unitTar
                                    , const TZWCmdClass&    zwccSrc
                                    , const tCIDLib::TCard4 c4CmdClass
                                    , const tCIDLib::TCard4 c4Cmd)
{
    tCIDLib::TCardList fcolVals(2);
    fcolVals.c4AddElement(c4CmdClass);
    fcolVals.c4AddElement(c4Cmd);

    bSendWaitAck(unitTar, zwccSrc, fcolVals, kCIDLib::True);
    bWaitTransAck(kCIDLib::True);
}


tCIDLib::TVoid TZWaveLevi2SDriver::LeviSetFldErr(const tCIDLib::TCard4 c4FldId)
{
    SetFieldErr(c4FldId, kCIDLib::True);
}

const TCQCFldLimit*
TZWaveLevi2SDriver::pfldlLeviLimsFor(const  tCIDLib::TCard4 c4FldId
                                    , const TClass&         clsType) const
{
    return pfldlLimsFor(c4FldId, clsType);
}

const TString& TZWaveLevi2SDriver::strLeviMoniker() const
{
    return strMoniker();
}

TTextOutStream& TZWaveLevi2SDriver::strmLeviTrace()
{
    return m_strmTrace;
}



// ---------------------------------------------------------------------------
//  TZWaveLevi2SDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't need to lock here, because no incoming client calls will be
//  seen until we are online, and we aren't yet.
//
tCIDLib::TBoolean TZWaveLevi2SDriver::bGetCommResource(TThread& thrThis)
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
//  come up with no groups and units.
//
tCIDLib::TBoolean TZWaveLevi2SDriver::bWaitConfig(TThread&)
{
    //
    //  For now we just call the config file reader method. It's split out because
    //  it needs to be called from other places as well.
    //
    return bReadCfgFile();
}


//
//  Handle boolean field writes
//
tCQCKit::ECommResults
TZWaveLevi2SDriver::eBoolFldValChanged( const   TString&            strName
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

    const tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);
        tCIDLib::TCard4 c4CmdIndex;
        if (unitCur.bOwnsFld(c4FldId, c4CmdIndex))
            return unitCur.eWriteBoolFld(c4FldId, bNewValue, *this, c4CmdIndex);
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
TZWaveLevi2SDriver::eCardFldValChanged( const   TString&        strName
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
        tCIDLib::TCard4 c4CmdIndex;
        if (unitCur.bOwnsFld(c4FldId, c4CmdIndex))
            return  unitCur.eWriteCardFld(c4FldId, c4NewValue, *this, c4CmdIndex);
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
tCQCKit::ECommResults TZWaveLevi2SDriver::eConnectToDevice(TThread& thrThis)
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
    m_c4NextPollInd = 0;
    m_c4TOCount = 0;
    m_enctNextTrans = 0;
    m_enctNextPoll = TTime::enctNowPlusSecs(kZWaveLevi2S::c4PollSecs);

    return tCQCKit::ECommResults::Success;
}


//
//  Handle driver init, the usual stuff.
//
tCQCKit::EDrvInitRes TZWaveLevi2SDriver::eInitializeImpl()
{
    LogMsg
    (
        TString(L"Initializing Z-Wave driver. Mode=V") + TCardinal(c4ArchVersion())
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

    // Setup our cfg file path
    TPathStr pathCfg = facCQCKit().strServerDataDir();
    pathCfg.AddLevel(L"ZWaveLevi2");
    m_strCfgPath = pathCfg;

    // If this doesn't exist, create it
    TFileSys::MakePath(m_strCfgPath);

    // Now build up the actual file name for our config file
    pathCfg.AddLevel(strMoniker());
    pathCfg.AppendExt(L"Cfg");
    m_strCfgFile = pathCfg;

    //
    //  Set our poll and reconnect times to something desirable for us. We
    //  set the poll time to the lowest possible, so that we can get incoming
    //  messages out of the controller ASAP. It has a limited buffer size
    //  and we don't want it to overflow.
    //
    SetPollTimes(10, 10000);

    // Crank up a bit the extension time on the message reading helpers
    enctExtend(kCIDLib::enctOneMilliSec * 500);

    // Store our version info later to put into the trace file if opened
    m_c4VerMaj = cqcdcOurs.c4MajVersion();
    m_c4VerMin = cqcdcOurs.c4MinVersion();


    // In debug mode, start off with tracing enabled by default
    #if CID_DEBUG_ON
    StartTrace(kCIDLib::True);
    #endif

    // Indicate that we need to load configuration
    return tCQCKit::EDrvInitRes::WaitConfig;
}



//
//  Handle int field writes
//
tCQCKit::ECommResults
TZWaveLevi2SDriver::eIntFldValChanged(  const   TString&        strName
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
        , TInteger(i4NewValue)
    );

    // See if any unit owns this field
    tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4UnitCount; c4Index++)
    {
        TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);
        tCIDLib::TCard4 c4CmdIndex;
        if (unitCur.bOwnsFld(c4FldId, c4CmdIndex))
            return  unitCur.eWriteIntFld(c4FldId, i4NewValue, *this, c4CmdIndex);
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
//  Watch for aync notifications from units. We also watch for new configuration having
//  been uploaded.
//
tCQCKit::ECommResults TZWaveLevi2SDriver::ePollDevice(TThread& thrCaller)
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
                facZWaveLevi2S().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kZW2Errs::errcCfg_CantSet
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                );
            }
        }

        // Check for any available async messages
        EatMsgs(50, kCIDLib::False);

        //
        //  Now we round robin through the units, giving them a chance to do any
        //  active polling they need to do. We assume a slow poll rate, on the order
        //  of ten minutes or more, so we don't ahve to go too fast here.
        //
        if (TTime::enctNow() >= m_enctNextPoll)
        {
            //
            //  Move forward until we find one that does a poll. If we get back to
            //  the starting index, then nothing to do.
            //
            const tCIDLib::TCard4 c4Count  = m_dcfgCurrent.c4UnitCnt();
            if (c4Count)
            {
                const tCIDLib::TCard4 c4OrgInd = m_c4NextPollInd;
                do
                {
                    //
                    //  If it's in viable state, and it's not a wakeup type, then it
                    //  should be pollable, though it might not choose to do anything.
                    //
                    TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(m_c4NextPollInd);
                    if (unitCur.bIsViable()
                    &&  !unitCur.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
                    {
                        if (unitCur.bActivePoll(*this))
                            break;
                    }

                    m_c4NextPollInd++;
                    if (m_c4NextPollInd >= c4Count)
                        m_c4NextPollInd = 0;

                }   while(m_c4NextPollInd != c4OrgInd);
            }

            m_enctNextPoll = TTime::enctNowPlusSecs(kZWaveLevi2S::c4PollSecs);
        }
    }

    catch(TError& errToCatch)
    {
        if ((eVerboseLevel() >= tCQCKit::EVerboseLvls::High) && !errToCatch.bLogged())
            TModule::LogEventObj(errToCatch);

        //
        //  Do a test to see if we are still connected to the device. If so, the this
        //  is not a loss of connection error. We just do a simple clear of the node
        //  list, which only talks to the VRCOP and should get an ack if we are
        //  connected.
        //
        try
        {
            // Pause a bit first, just in case
            if (thrCaller.bSleep(1000))
                SendVRCOPMsg(L"N,", kCIDLib::True);
        }

        catch(...)
        {
            // If this happens more than 3 times in a row, give up
            m_c4TOCount++;
            if (m_c4TOCount > 3)
            {
                LogMsg
                (
                    L"Lost connection during poll, recycling..."
                    , tCQCKit::EVerboseLvls::High
                    , CID_FILE
                    , CID_LINE
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
TZWaveLevi2SDriver::eStringFldValChanged(const  TString&        strName
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

    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::ValueRejected;
    if (c4FldId == m_c4FldId_InvokeCmd)
    {
        eRes = eProcessCmdFld(strNewValue);
        if (eRes != tCQCKit::ECommResults::Success)
            IncFailedWriteCounter();
    }
     else
    {
        // Let's see if a unit claims this field
        const tCIDLib::TCard4 c4UnitCount = m_dcfgCurrent.c4UnitCnt();
        tCIDLib::TCard4 c4Index = 0;
        for (; c4Index < c4UnitCount; c4Index++)
        {
            TZWaveUnit& unitCur = m_dcfgCurrent.unitAt(c4Index);
            tCIDLib::TCard4 c4CmdIndex;
            if (unitCur.bOwnsFld(c4FldId, c4CmdIndex))
            {
                eRes = unitCur.eWriteStringFld(c4FldId, strNewValue, *this, c4CmdIndex);
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
            eRes = tCQCKit::ECommResults::FieldNotFound;
        }

        if (eRes != tCQCKit::ECommResults::Success)
        {
            LogMsg
            (
                L"Field write failed for field %(1)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , strName
            );
            IncFailedWriteCounter();
        }
         else
        {
            LogMsg
            (
                L"Field write succeeded for field %(1)"
                , tCQCKit::EVerboseLvls::Medium
                , CID_FILE
                , CID_LINE
                , strName
            );
        }
    }

    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TZWaveLevi2SDriver::ReleaseCommResource()
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


tCIDLib::TVoid TZWaveLevi2SDriver::TerminateImpl()
{
}



// ---------------------------------------------------------------------------
//  TZWaveLevi2SDriver: Private, non-virtual methods
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
TZWaveLevi2SDriver::bLoadConfig(        TZWaveDrvConfig&    dcfgNew
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
            EnumerateDevices(dcfgNew);

            //
            //  OK, it worked so store the new configuration. We need store the new
            //  stuff before we register fields, because we have to set up all of
            //  the device info. That stuf isn't copied so we have to do it on the
            //  actual target config object.
            //
            m_dcfgCurrent = dcfgNew;

            //
            //  Tell it to load it's device info. For any that are ready, i.e. their
            //  device type has been set by the user and it can be successfully loaded
            //  here, this will prep them for use and put them into Init status.
            //
            //  Any that were ready to init, but we failed to load the info, will go
            //  into failed state.
            //
            m_dcfgCurrent.LoadDevInfo(*this);

            // Register fields now that we know what we have
            RegisterFields(m_dcfgCurrent);

            // Give them all a chance to do any init they want
            m_dcfgCurrent.InitVals(*this);

            // Write out the the new config if not in OnConnect
            if (!bOnConnect)
                WriteCfgFile();

            // Update the serial number
            m_c4CfgSerialNum++;
        }

        //
        //  Get initial values for configured units. Async processing is on again now,
        //  so we'll pick up incoming reports while we are waiting on other responses
        //  during the initial query round.
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
//  A little helper for the command field handler method, to check that a
//  required number of parameters are available, else throw. It can be a
//  literal count check or at least that many.
//
//  The actual count is one less than the parm count since that includes the
//  command itself. If it's zero we consider that an error since the caller
//  should not have even called us in that case.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::CheckCmdFldParmCnt( const   TString&            strCmd
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
        facZWaveLevi2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kZW2Errs::errcFld_CmdParms
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
            , strCmd
            , TCardinal(c4ExpCnt)
            , TCardinal(c4ParmCnt ? c4ParmCnt - 1 : 0)
        );
        throw tCQCKit::ECommResults::CmdParms;
    }
}


//
//  Checks a provided textual representation of a value that should be a
//  a numeric value. Make sure it can be converted to a number and that it
//  is within a given range. If not throw.
//
tCIDLib::TVoid
TZWaveLevi2SDriver::CheckNumericVal(const   TString&            strVal
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
            facZWaveLevi2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcFld_NumValFmt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TString(pszName)
            );
            throw tCQCKit::ECommResults::CmdParms;
        }
         else
        {
            facZWaveLevi2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kZW2Errs::errcFld_NumValFmt2
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , TString(pszName)
                , TCardinal(c4Min)
                , TCardinal(c4Max)
            );
            throw tCQCKit::ECommResults::CmdParms;
        }
    }
}



//
//  We support a free format command field that lets them send various commands via
//  a formatted string.
//
//  The commands are:
//
//      AddAssoc [tarunit] [srcunit] srcgrpnum
//      AllOn
//      AllOff
//      AssocToVRCOP [srcunit] srcgrpnum
//      DelAllAssoc [unit] grpnum
//      DelAssoc [unit] grpnum [removeunit]
//      EnableTrace (Yes | No)
//      SetCfgParam [tarunit] parm# val bytes
//      SetWakeupSecs [tarunit] [srcunit] Secs
//
//
//      Group grpname (On|Off)|(RampUp|RampDn|RampEnd)
//      GroupLev grpname %level
//      SetGroupList grpname unit1 [unit2 unit3 ...]
//      SetSetPnt thermoname (Heat|Cool|Furnace|Dry|Moist|Auto) temp [F|C]
//      Unit unitname (On|Off|RampUp|RampDn|RampEnd)
//
tCQCKit::ECommResults TZWaveLevi2SDriver::eProcessCmdFld(const TString& strCmdText)
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
        return tCQCKit::ECommResults::BadValue;
    }

    // Strip leading and trailing white space from the tokens
    const tCIDLib::TCard4 c4TokenCnt = colTokens.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TokenCnt; c4Index++)
        colTokens[c4Index].StripWhitespace();

    // Upper case the first one, which is the command
    colTokens[0].ToUpper();
    const TString& strCmd = colTokens[0];

    tCIDLib::TCardList fcolVals(64);
    try
    {
        // And, based on the command, process the info
        TString strToSend;
        if ((strCmd == kZWaveLevi2Sh::strDrvCmd_AddAssoc)
        ||  (strCmd == kZWaveLevi2Sh::strDrvCmd_AssocToVRCOP))
        {
            //
            //  These are the same except that the target is not there on the
            //  VRCOP association, since it's the VRCOP.
            //
            tCIDLib::TCard4 c4Ind;
            if (strCmd == kZWaveLevi2Sh::strDrvCmd_AddAssoc)
            {
                CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3);
                c4Ind = 2;
            }
             else
            {
                CheckCmdFldParmCnt(strCmd, c4TokenCnt, 2);
                c4Ind = 1;
            }

            // Src unit by name
            TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(colTokens[c4Ind++]);

            // Make sure it supports associations
            TZWCmdClass* pzwccToUse = unitSrc.pzwccFind(tZWaveLevi2Sh::ECClasses::Association, 0);
            if (!pzwccToUse)
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::NotSupported;
            }

            tCIDLib::TCard4 c4GrpId;
            CheckNumericVal(colTokens[c4Ind], c4Ind, c4GrpId, L"groupnum", 1, 255);

            // Target is either VRCOP, or a unit name/id
            TZWaveUnit* punitTar = 0;
            if (strCmd == kZWaveLevi2Sh::strDrvCmd_AddAssoc)
            {
                punitTar = &m_dcfgCurrent.unitFindByName(colTokens[1]);
            }
             else
            {
                punitTar = &m_dcfgCurrent.unitFindById
                (
                    m_dcfgCurrent.c4FindVRCOP(kCIDLib::True)
                );
            }

            TZWCCAssociation* pzwccAssoc = static_cast<TZWCCAssociation*>(pzwccToUse);
            if (!pzwccAssoc->bDoAssociation(unitSrc, 0, *punitTar, c4GrpId, *this))
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_AllOn)
        {
            strToSend = L"N,ON";
            SendVRCOPMsg(strToSend, kCIDLib::True);
            TThread::Sleep(500);
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_AllOff)
        {
            strToSend = L"N,OF";
            SendVRCOPMsg(strToSend, kCIDLib::True);
            TThread::Sleep(500);
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_DelAllAssoc)
        {
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 2);

            // Src unit by name
            TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(colTokens[1]);

            // Make sure it supports associations
            TZWCmdClass* pzwccToUse = unitSrc.pzwccFind(tZWaveLevi2Sh::ECClasses::Association, 0);
            if (!pzwccToUse)
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::NotSupported;
            }

            tCIDLib::TCard4 c4GrpId;
            CheckNumericVal(colTokens[2], 2, c4GrpId, L"groupnum", 1, 255);

            TZWCCAssociation* pzwccAssoc = static_cast<TZWCCAssociation*>(pzwccToUse);
            if (!pzwccAssoc->bDelAllAssociation(unitSrc, 0, c4GrpId, *this))
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_DelAssoc)
        {
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3);

            // Src unit by name
            TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(colTokens[1]);

            // Make sure it supports associations
            TZWCmdClass* pzwccToUse = unitSrc.pzwccFind(tZWaveLevi2Sh::ECClasses::Association, 0);
            if (!pzwccToUse)
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::NotSupported;
            }

            tCIDLib::TCard4 c4GrpId;
            CheckNumericVal(colTokens[2], 2, c4GrpId, L"groupnum", 1, 255);

            // And the unit to remove
            TZWaveUnit& unitRem = m_dcfgCurrent.unitFindByName(colTokens[3]);

            TZWCCAssociation* pzwccAssoc = static_cast<TZWCCAssociation*>(pzwccToUse);
            if (!pzwccAssoc->bDelAssociation(unitSrc, 0, c4GrpId, unitRem, *this))
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_EnableTrace)
        {
            // We get a yes or no value
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 1);
            StartTrace(colTokens[1].eCompareI(L"YES") == tCIDLib::ESortComps::Equal);
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_SetCfgParam)
        {
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 4, kCIDLib::True);

            // Find the target unit
            TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);

            // Make sure it supports associations
            TZWCmdClass* pzwccToUse = unitTar.pzwccFind(tZWaveLevi2Sh::ECClasses::Configuration, 0);
            if (!pzwccToUse)
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::NotSupported;
            }

            // Check the configuration number, the value and the size
            tCIDLib::TCard4 c4Num;
            CheckNumericVal(colTokens[2], 2, c4Num, L"parm#");

            tCIDLib::TCard4 c4Val;
            if (!colTokens[3].bToCard4(c4Val, tCIDLib::ERadices::Dec))
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::CmdParms;
            }

            tCIDLib::TCard4 c4Size;
            CheckNumericVal(colTokens[4], 4, c4Size, L"valsize");

            TZWCCConfig* pzwccConfig = static_cast<TZWCCConfig*>(pzwccToUse);
            if (!pzwccConfig->bSetConfigParm(unitTar, c4Num, c4Size, c4Val, *this))
            {
                IncFailedWriteCounter();
                return tCQCKit::ECommResults::ValueRejected;
            }
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_SetWakeupSecs)
        {
            CheckCmdFldParmCnt(strCmd, c4TokenCnt, 3);

            // Units by name
            const TZWaveUnit& unitTar = m_dcfgCurrent.unitFindByName(colTokens[1]);
            TZWaveUnit& unitSrc = m_dcfgCurrent.unitFindByName(colTokens[2]);

            tCIDLib::TCard4 c4Secs;
            CheckNumericVal(colTokens[3], 3, c4Secs, L"seconds", 30, 60000);

            fcolVals.c4AddElement(COMMAND_CLASS_WAKE_UP);
            fcolVals.c4AddElement(WAKE_UP_INTERVAL_SET);
            fcolVals.c4AddElement(tCIDLib::TCard1(c4Secs >> 16));
            fcolVals.c4AddElement(tCIDLib::TCard1(c4Secs >> 8));
            fcolVals.c4AddElement(tCIDLib::TCard1(c4Secs));
            fcolVals.c4AddElement(unitTar.c4Id());

            if (unitTar.bImplementsClass(tZWaveLevi2Sh::ECClasses::WakeUp))
            {
                TZWQCmd zwqcNew(0, tZWaveLevi2Sh::ECClasses::WakeUp, fcolVals);
                unitSrc.QueueUpCmd(zwqcNew, *this);
            }
             else
            {
                const tCIDLib::TBoolean bRes = bSendWaitAck
                (
                    unitSrc.c4Id()
                    , 0
                    , fcolVals
                    , tZWaveLevi2Sh::EMultiTypes::Single
                    , unitSrc.bClassIsSecure(tZWaveLevi2Sh::ECClasses::WakeUp)
                    , kCIDLib::False
                );

                if (!bRes)
                {
                    IncFailedWriteCounter();
                    return tCQCKit::ECommResults::CommError;
                }
            }
        }

#if 0
         else if ((strCmd == kZWaveLevi2Sh::strDrvCmd_Unit)
              ||  (strCmd == kZWaveLevi2Sh::strDrvCmd_Group))
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
            if (strCmd == kZWaveLevi2Sh::strDrvCmd_Unit)
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
            SendUnitCmd(strToSend, 5000, c4TarId);

            // These need a little time, so prevent a rushed next command
            TThread::Sleep(100);
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_GroupLev)
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

            SendUnitCmd(strToSend, 5000, 0);
        }
         else if (strCmd == kZWaveLevi2Sh::strDrvCmd_SetGroupList)
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
                    bSendAndWaitAck(strToSend, 5000);
                    strToSend = L"AP";
                }
            }

            //
            //  Now append the group store command onto what we have left,
            //  and send that.
            //
            strToSend.Append(L"GS");
            strToSend.AppendFormatted(c4GrpId);
            SendUnitCmd(strToSend, 5000, 0);
        }
#endif
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
            return tCQCKit::ECommResults::UnknownCmd;
        }
    }

    catch(const tCQCKit::ECommResults eErr)
    {
        //
        //  The helper methods that check parameters will just throw a comm result
        //  that we can just return.
        //
        return eErr;
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return tCQCKit::ECommResults::Exception;
    }

    return tCQCKit::ECommResults::Success;
}


