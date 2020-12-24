//
// FILE NAME: CQCIR_DriverBase.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/16/2003
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
//  This file implements common functionality for all server side IR drivers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIR_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TBaseIRSrvDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TBaseIRSrvDriver
//  PREFIX: sdrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Destructor
// ---------------------------------------------------------------------------
TBaseIRSrvDriver::~TBaseIRSrvDriver()
{
}


// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TBaseIRSrvDriver::bQueryData(const  TString&            strQType
                            , const TString&            strData
                            ,       tCIDLib::TCard4&    c4OutBytes
                            ,       THeapBuf&           mbufToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (strQType == kCQCIR::strCvtManualBlastData)
        {
            TString strError;
            bRet = bCvtManualBlastData(strData, c4OutBytes, mbufToFill, strError);
            if (!bRet)
            {
                TTextMBufOutStream strmErr(&mbufToFill);
                strmErr << strError << kCIDLib::FlushIt;
                c4OutBytes = strmErr.c4CurSize();
            }
        }
         else if (strQType == kCQCIR::strQueryBTrainingData)
        {
            bRet = bCheckBlastTrainingData(c4OutBytes, mbufToFill);
        }
         else if (strQType == kCQCIR::strQueryMappedEvents)
        {
            TVector<TCQCStdKeyedCmdSrc> colToFill;
            QueryMappedRecEvents(colToFill);

            TBinMBufOutStream strmOut(&mbufToFill);
            strmOut << colToFill << kCIDLib::FlushIt;
            c4OutBytes = strmOut.c4CurSize();
        }
         else if (strQType == kCQCIR::strQueryBlasterCfg)
        {
            tCIDLib::TCard4             c4ZoneCount;
            TFacCQCIR::TModelInfoList   colList;
            QueryBlasterConfig(colList, c4ZoneCount);

            TBinMBufOutStream strmOut(&mbufToFill);
            strmOut << colList << c4ZoneCount << kCIDLib::FlushIt;
            c4OutBytes = strmOut.c4CurSize();
        }
         else
        {
            ThrowUnknownId(strQType, L"bQueryData");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


tCIDLib::TBoolean
TBaseIRSrvDriver::bQueryData2(  const   TString&            strQType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    // Make sure we don't send back bytes based on an unitialized size value
    const tCIDLib::TCard4 c4InBytes = c4IOBytes;
    c4IOBytes = 0;

    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (strQType == kCQCIR::strFmtBlastData)
        {
            //
            //  They pass a blaster command object. We send them back the blaster
            //  data from that commands formatted to text in the driver speciifc
            //  format.
            //
            TIRBlasterCmd irbcData;
            {
                TBinMBufInStream strmSrc(&mbufIO, c4InBytes);
                strmSrc >> irbcData;
            }

            TString strFmt;
            FormatBlastData(irbcData, strFmt);

            {
                TBinMBufOutStream strmTar(&mbufIO);
                strmTar << strFmt;
                strmTar.Flush();
                c4IOBytes = strmTar.c4CurSize();
            }
        }
         else
        {
            ThrowUnknownId(strQType, L"bQueryData2");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    return bRet;
}


tCIDLib::TBoolean
TBaseIRSrvDriver::bQueryTextVal(const   TString&    strValId
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (strValId == kCQCIR::strCheckKeyUsed)
        {
            bRet = bIsRecKeyUsed(strDataName, strToFill);
        }
         else if (strValId == kCQCIR::strQueryRTrainingData)
        {
            bRet = bCheckRecTrainingData(strToFill);
        }
         else if (strValId == kCQCIR::strQueryDevCmdList)
        {
            //
            //  We just format out a list of device models that we have loaded
            //  and their commands.
            //
            bRet = bFormatCmds(strToFill);
        }
         else if (strValId == kCQCIR::strQueryFmtBlastData)
        {
            // Break the string data name out into model/cmd
            TString strModel(64UL);
            TString strCmd(64UL);
            facCQCIR().c4ParseInvokeCmd
            (
                strDataName, strModel, strCmd, c4ZoneCount(), strMoniker()
            );

            //
            //  Find this model. Don't care about the repeat count. Lock the
            //  driver while we are accessing the config.
            //
            tCIDLib::TCard4 c4RepeatCount;
            const TIRBlasterCmd& irbcFmt = m_irbcData.irbcFromName
            (
                strModel, strCmd, c4RepeatCount
            );

            // The derived class has to provide this
            FormatBlastData(irbcFmt, strToFill);
        }
         else
        {
            ThrowUnknownId(strValId, L"bQueryTextVal");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if ((eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        &&  !errToCatch.bLogged())
        {
            TModule::LogEventObj(errToCatch);
        }
        throw;
    }
    return bRet;
}


tCIDLib::TBoolean TBaseIRSrvDriver::bQueryVal(const TString& strValId)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        if (strValId == kCQCIR::strQueryBTrainMode)
        {
            bRet = bBlastTrainingMode();
        }
         else if (strValId == kCQCIR::strQueryRTrainMode)
        {
            bRet = bRecTrainingMode();
        }
         else
        {
            ThrowUnknownId(strValId, L"bQueryVal");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return bRet;
}


tCIDLib::TBoolean
TBaseIRSrvDriver::bSendData(const   TString&            strSendType
                            ,       TString&
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufData)
{
    // These will all need an input stream over the buffer
    TBinMBufInStream strmSrc(&mbufData, c4Bytes);

    tCIDLib::TBoolean bRet = kCIDLib::True;
    try
    {
        if (strSendType == kCQCIR::strInvokeBlastCmd)
        {
            tCIDLib::TCard4 c4Zone;
            TString strCmd;
            TString strDevice;
            strmSrc >> strDevice >> strCmd >> c4Zone;
            InvokeBlastCmd(strDevice, strCmd, c4Zone);
        }
         else if (strSendType == kCQCIR::strSendData)
        {
            tCIDLib::TCard4 c4Repeat;
            tCIDLib::TCard4 c4Zone;
            tCIDLib::TCard4 c4DataBytes;
            THeapBuf mbufData;
            strmSrc >> c4DataBytes >> c4Zone >> c4Repeat;
            strmSrc.c4ReadBuffer(mbufData, c4DataBytes);
            SendBlasterData(c4DataBytes, mbufData, c4Zone, c4Repeat);
        }
         else if (strSendType == kCQCIR::strStoreBlasterCfg)
        {
            TIRBlasterPerData perdToStore;
            strmSrc >> perdToStore;
            StoreBlastConfig(perdToStore);
        }
         else if (strSendType == kCQCIR::strStoreRecCfg)
        {
            TIRReceiverCfg irrcToStore;
            strmSrc >> irrcToStore;
            StoreRecCfg(irrcToStore);
        }
         else if (strSendType == kCQCIR::strStoreTrainData)
        {
            TCQCStdKeyedCmdSrc csrcToStore;
            strmSrc >> csrcToStore;
            bRet = bStoreRecTrainingData(csrcToStore);
        }
         else
        {
            ThrowUnknownId(strSendType, L"bSendData");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    //
    //  We don't return any info. SO BE SURE to zero size value, because
    //  the ORB will stream back however many bytes it indicates otherwise.
    //
    c4Bytes = 0;
    return kCIDLib::True;
}


tCIDLib::TCard4
TBaseIRSrvDriver::c4SendCmd(const   TString&    strCmd
                            , const TString&    strParms)
{
    tCIDLib::TCard4 c4Ret = 0;

    try
    {
        if (strCmd == kCQCIR::strClearBTraining)
        {
            ClearBlastTrainingData();
        }
         else if (strCmd == kCQCIR::strClearRTraining)
        {
            ClearRecTrainingData();
        }
         else if (strCmd == kCQCIR::strDelAllEvents)
        {
            DeleteAllEvents();
        }
         else if (strCmd == kCQCIR::strDeleteEvent)
        {
            DeleteRecEvent(strParms);
        }
         else if (strCmd == kCQCIR::strEnterBTraining)
        {
            EnterBlastTrainingMode();
        }
         else if (strCmd == kCQCIR::strEnterRTraining)
        {
            EnterRecTrainingMode();
        }
         else if (strCmd == kCQCIR::strExitBTraining)
        {
            ExitBlastTrainingMode();
        }
         else if (strCmd == kCQCIR::strExitRTraining)
        {
            ExitRecTrainingMode();
        }
         else if (strCmd == kCQCIR::strLoadModel)
        {
            LoadBlasterDevModel(strParms);
        }
         else if (strCmd == kCQCIR::strResetConn)
        {
            if (bResetConnection())
                c4Ret = 1;
        }
         else if (strCmd == kCQCIR::strUnloadModel)
        {
            UnloadBlasterDevModel(strParms);
        }
         else
        {
            ThrowUnknownId(strCmd, L"c4SendCmd");
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}


// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Let the derived class see the set separator character
tCIDLib::TCh TBaseIRSrvDriver::chSepChar() const
{
    return m_chSepChar;
}


// Provide const access to our blaster and receiver config
const TIRBlasterCfg& TBaseIRSrvDriver::irbcData() const
{
    return m_irbcData;
}


const TIRReceiverCfg& TBaseIRSrvDriver::irrcData() const
{
    return m_irrcData;
}



// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Hidden constructors
// ---------------------------------------------------------------------------
TBaseIRSrvDriver::TBaseIRSrvDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4BlastCfgVerId(0)
    , m_c4RecCfgVerId(0)
    , m_chSepChar(kCIDLib::chNull)
    , m_enctNextRebindTime(0)
    , m_thrActions(cqcdcToLoad.strMoniker(), cuctxDrv())
{
    // Pull the device info out of the manifest
    m_eDevCaps = facCQCIR().eExtractDevInfo
    (
        cqcdcToLoad, m_strRepoName, m_strRepoDescr
    );
}


// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  All the IR blasters store the same configuration data, so we can handle
//  the config wait method for them all.
//
//  What is stored is just the names of the device models, so we have to
//  use those names to load up the actual device model data from the data
//  server.
//
tCIDLib::TBoolean TBaseIRSrvDriver::bWaitConfig(TThread&)
{
    // Flush the current configuration
    m_irbcData.Reset();
    m_irrcData.Reset();

    //
    //  Reset our repo version ids so that we'll be forced to reload the
    //  config. Otherwise, if we were reconfigured, we'd never think we
    //  had any new data to load.
    //
    m_c4BlastCfgVerId = 0;
    m_c4RecCfgVerId = 0;

    //
    //  If this is a device that has blaster data, then load the blaster
    //  config.
    //
    TString strKey;
    if (tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::Blast)
    ||  tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::BlastTrain))
    {
        //
        //  Load up the config from the local configuration repository. If
        //  there was no file to load, then we get default data.
        //
        TIRBlasterPerData perdTmp;

        //
        //  Create the configuration repository key for our configuration
        //  info.
        //
        strKey = kCQCKit::pszOSKey_IRBlasterCfg;
        strKey.Append(kCIDLib::chForwardSlash);
        strKey.Append(strMoniker());

        tCIDLib::ELoadRes eRes = tCIDLib::ELoadRes::NotFound;
        try
        {
            // Get a config repo client proxy for the local machine
            TCfgServerClient cfgscSrc;
            eRes = cfgscSrc.eReadObject(strKey, perdTmp, m_c4BlastCfgVerId);
        }

        catch(...)
        {
            //
            //  We couldn't open the config repo, so say we failed. No
            //  need to fall through to the reciever config below because
            //  it would fail for the same reason.
            //
            return kCIDLib::False;
        }

        //
        //  If new data, we need to read it in, else we are good with our
        //  default empty config.
        //
        if (eRes == tCIDLib::ELoadRes::NewData)
        {
            // Double check that its our configuration
            if (perdTmp.strMoniker() != strMoniker())
            {
                facCQCIR().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kIRErrs::errcCfg_NotMyConfig
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , perdTmp.strMoniker()
                );

                //
                //  Return true to keep it from continuing to call back into
                //  here. It'll just have an empty config.
                //
                return kCIDLib::True;
            }

            //
            //  We need to iterate the device model names in the loaded data
            //  and load up the device model from the data server for each
            //  one.
            //
            try
            {
                // We need an IR repository client proxy
                tCQCIR::TRepoSrvProxy orbcIRRepo = facCQCIR().orbcRepoSrvProxy();

                //
                //  If we have been configured for any devices, lets get a
                //  client proxy for the IR repository server and load up
                //  the files for each one.
                //
                TIRBlasterPerData::TModelCursor cursModels(perdTmp.cursModels());
                if (cursModels.bIsValid())
                {
                    tCIDLib::TBoolean   bAdded;
                    TIRBlasterDevModel  irbdmNew;
                    for (; cursModels; ++cursModels)
                    {
                        try
                        {
                            const TString& strModelName = *cursModels;

                            orbcIRRepo->QueryDevModel
                            (
                                strModelName, m_strRepoName, irbdmNew
                            );
                            m_irbcData.irbdmAddOrUpdate(irbdmNew, bAdded);

                            //
                            //  All of these should definitely be newly added!.
                            //  If not, log a status message, but keep on
                            //  going.
                            //
                            if (!bAdded)
                            {
                                facCQCIR().LogMsg
                                (
                                    CID_FILE
                                    , CID_LINE
                                    , kIRErrs::errcCfg_DupKeyInDev
                                    , tCIDLib::ESeverities::Status
                                    , tCIDLib::EErrClasses::AppStatus
                                    , strModelName
                                );
                            }
                        }

                        catch(TError& errToCatch)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                            //
                            //  If it's a lost connection error, then just
                            //  rethrow else log and keep going.
                            //
                            if (orbcIRRepo->bCheckForLostConnection(errToCatch))
                                throw;

                            if (!errToCatch.bLogged())
                                TModule::LogEventObj(errToCatch);
                        }
                    }
                }
            }

            catch(...)
            {
                // Flush it again and say we failed
                m_irbcData.Reset();
                return kCIDLib::False;
            }
        }
    }


    //
    //  If this is a device that has reciever data, then load the receiver
    //  config.
    //
    if (tCIDLib::bAllBitsOn(m_eDevCaps, tCQCIR::EIRDevCaps::Receive))
    {
        //
        //  Create the configuration repository key for our configuration
        //  info.
        //
        strKey = kCQCKit::pszOSKey_IRReceiverCfg;
        strKey.Append(kCIDLib::chForwardSlash);
        strKey.Append(strMoniker());

        try
        {
            // Get a config repo client proxy for the local machine
            TCfgServerClient cfgscSrc;
            cfgscSrc.eReadObject(strKey, m_irrcData, m_c4RecCfgVerId);
        }

        catch(...)
        {
            // Flush both and return failure
            m_irbcData.Reset();
            m_irrcData.Reset();
            return kCIDLib::False;
        }
    }

    // It worked man, so say all is well
    return kCIDLib::True;
}


//
//  Handle driver initialization. It can be called more than once so we
//  have to deal with that.
//
tCQCKit::EDrvInitRes TBaseIRSrvDriver::eInitializeImpl()
{
    //
    //  Reset some stuff at our level to defaults that we want if the derived
    //  class doesn't set them explicitly during init.
    //
    m_chSepChar = kCIDLib::chNull;

    // And indicate we want to load configuration data
    return tCQCKit::EDrvInitRes::WaitConfig;
}


// Handle string field changes
tCQCKit::ECommResults
TBaseIRSrvDriver::eStringFldValChanged( const   TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    if (c4FldId == c4InvokeFldId())
    {
        //
        //  The CQCIR facility provides a method to parse this string, which
        //  is the same for all IR blasters. It gives us back a zone and
        //  device and command names.
        //
        //  It will verify that the zone is legal for the zone count we
        //  pass.
        //
        TString strModel(64UL);
        TString strCmd(64UL);
        const tCIDLib::TCard4 c4ZoneNum = facCQCIR().c4ParseInvokeCmd
        (
            strNewValue
            , strModel
            , strCmd
            , c4ZoneCount()
            , strMoniker()
        );

        // Look up a cmd with this dev/cmd name
        tCIDLib::TCard4 c4RepeatCount;
        const TIRBlasterCmd& irbcInvoke = m_irbcData.irbcFromName
        (
            strModel, strCmd, c4RepeatCount
        );

        //
        //  And call the derived class to get him to actually send the
        //  data to the device.
        //
        SendBlasterData
        (
            irbcInvoke.c4DataLen()
            , irbcInvoke.mbufData()
            , c4ZoneNum
            , c4RepeatCount
        );
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TBaseIRSrvDriver::TerminateImpl()
{
    //
    //  Stop the actions thread, which means we'll probably have to wait
    //  for any current action to complete.
    //
    try
    {
        if (m_thrActions.bIsRunning())
        {
            m_thrActions.ReqShutdownSync(15000);
            m_thrActions.eWaitForDeath(5000);
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        facCQCIR().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcEv_StopActsThread
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
    }
}


//
//  We handle this in order to keep our action thread's verbose setting
//  in sync with the driver verbose level.
//
tCIDLib::TVoid
TBaseIRSrvDriver::VerboseModeChanged(const tCQCKit::EVerboseLvls eNewState)
{
    TParent::VerboseModeChanged(eNewState);
    m_thrActions.bVerboseMode(eNewState != tCQCKit::EVerboseLvls::Off);
}


// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Protected, virtual methods
// ---------------------------------------------------------------------------

// Indicates if we are in blaster training mode or not
tCIDLib::TBoolean TBaseIRSrvDriver::bBlastTrainingMode() const
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // Default is to say not in training mode, if not overloaded
    return kCIDLib::False;
}


// Indicates if we have some training data and returns it if so
tCIDLib::TBoolean
TBaseIRSrvDriver::bCheckBlastTrainingData(tCIDLib::TCard4&, THeapBuf&)
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"bCheckBlasterTrainingData")
    );

    // Make the compiler happy
    return kCIDLib::False;
}


// Indicates if we have any receiver training data and returns it if so
tCIDLib::TBoolean TBaseIRSrvDriver::bCheckRecTrainingData(TString&)
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"bCheckrecTrainingData")
    );

    // Make the compiler happy
    return kCIDLib::False;
}


//
//  When the user enters data manually, we get the text entered, and have
//  to convert it to native format (if not already, it could be Pronto
//  format) and convert to whatever format we expect to get it back as
//  later for blasting and give that back.
//
tCIDLib::TBoolean
TBaseIRSrvDriver::bCvtManualBlastData(  const   TString&
                                        ,       tCIDLib::TCard4&
                                        ,       THeapBuf&
                                        ,       TString&)
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"bCvtManualBlastData")
    );

    // Make the compiler happy
    return kCIDLib::False;
}


// Indicates if we are in receiver training mode
tCIDLib::TBoolean TBaseIRSrvDriver::bRecTrainingMode() const
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    // Default is to say not in training mode, if not overloaded
    return kCIDLib::False;
}


//
tCIDLib::TCard4 TBaseIRSrvDriver::c4ZoneCount() const
{
    // Default is an open air blaster, which can only have one zone
    return 1;
}


tCIDLib::TCard4 TBaseIRSrvDriver::c4InvokeFldId() const
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"c4InvokeFldId")
    );

    // Make compiler happy
    return 0;
}


tCIDLib::TVoid TBaseIRSrvDriver::ClearBlastTrainingData()
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"CleanBlastTrainingData")
    );
}


tCIDLib::TVoid TBaseIRSrvDriver::ClearRecTrainingData()
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"CleanRecTrainingData")
    );
}


tCIDLib::TVoid TBaseIRSrvDriver::EnterBlastTrainingMode()
{
    // If they don't override this, that's bad so we throw
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"EnterBlastTrainingMode")
    );
}


tCIDLib::TVoid TBaseIRSrvDriver::EnterRecTrainingMode()
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"EnterRecTrainingMode")
    );
}


tCIDLib::TVoid TBaseIRSrvDriver::ExitBlastTrainingMode()
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"EnterBlastTrainingMode")
    );
}


tCIDLib::TVoid TBaseIRSrvDriver::ExitRecTrainingMode()
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"ExitRecTrainingMode")
    );
}


tCIDLib::TVoid
TBaseIRSrvDriver::FormatBlastData(const TIRBlasterCmd&, TString&)
{
}


tCIDLib::TVoid
TBaseIRSrvDriver::InvokeBlastCmd(const  TString&
                                , const TString&
                                , const tCIDLib::TCard4)
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"InvokeBlastCmd")
    );
}


tCIDLib::TVoid
TBaseIRSrvDriver::SendBlasterData(  const   tCIDLib::TCard4
                                    , const TMemBuf&
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TCard4)
{
    // If they don't override this, that's bad so we throw
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_NoMethodOverride
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strMoniker()
        , TString(L"SendBlasterData")
    );
}



// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Formats out a list of the device model names that we have loaded, and
//  for each one, it formats out the names of the commands that make it up.
//  It's a set of blocks like:
//
//  Model=
//  cmd1
//  cmd2
//  cmdx
//
//  Model=
//  cmd1
//  cmd2
//  cmdx
//
tCIDLib::TBoolean TBaseIRSrvDriver::bFormatCmds(TString& strToFill)
{
    TTextStringOutStream strmTar(&strToFill);
    TIRBlasterCfg::TModelCursor cursModels(m_irbcData.cursModels());
    if (!cursModels.bIsValid())
        return kCIDLib::False;

    for (; cursModels; ++cursModels)
    {
        const TIRBlasterDevModel& irbdmCur = *cursModels;
        strmTar << L"Model=" << irbdmCur.strModel() << kCIDLib::NewLn;

        TIRBlasterDevModel::TCmdCursor cursCmds = irbdmCur.cursCmds();
        for (; cursCmds; ++cursCmds)
            strmTar << cursCmds->strName() << kCIDLib::NewLn;

        strmTar << kCIDLib::NewLn;
    }

    strmTar.Flush();
    return kCIDLib::True;
}


tCIDLib::TVoid TBaseIRSrvDriver::ClearEventQ()
{
    m_thrActions.FlushQueue();
}


const TIRBlasterCmd&
TBaseIRSrvDriver::irbcFromName( const   TString&            strDevice
                                , const TString&            strCmd
                                ,       tCIDLib::TCard4&    c4RepeatCount)
{
    return m_irbcData.irbcFromName(strDevice, strCmd, c4RepeatCount);
}



//
//  This is called by derived classes to queue up an IR receiving event
//  which we queue on the action thread's queue to be processed, if we
//  find the passed event key.
//
tCIDLib::TVoid TBaseIRSrvDriver::QueueRecEvent(const TString& strEventStr)
{
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        TString strEvLog(strEventStr);
        if (strEvLog.c4Length() > 64)
            strEvLog.CapAt(64);

        facCQCIR().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIRMsgs::midStatus_GotEvent
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::NotFound
            , strEvLog
        );
    }

    //
    //  See if the event is found. If so, then queue it up. If we have a
    //  separator character queued up, then we have to split it apart and
    //  use the first part as the key and the second part becomes action
    //  parameter data to be passed to the action.
    //
    TString strKey = strEventStr;
    TString strData;
    if (m_chSepChar != kCIDLib::chNull)
        strKey.bSplit(strData, m_chSepChar);

    const TCQCStdKeyedCmdSrc* pcsrcTmp = m_irrcData.pcsrcFindByKey(strKey);
    if (pcsrcTmp)
    {
        m_thrActions.QueueEvent(*pcsrcTmp, strKey, strData);
    }
     else
    {
        // If at verbose or greater level, log this failure to match
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            TString strEvLog(strEventStr);
            if (strEvLog.c4Length() > 64)
                strEvLog.CapAt(64);

            facCQCIR().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIRMsgs::midStatus_NoRecMatch
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::NotFound
                , strEvLog
                , strMoniker()
            );
        }
    }
}


//
//  Store a key separator character. The derived class can call this to set a
//  separator character on us, which he'll either get from his manifest or just
//  use some fixed character.
//
tCIDLib::TVoid TBaseIRSrvDriver::SetKeySepChar(const tCIDLib::TCh chSepChar)
{
    m_chSepChar = chSepChar;
}


//
//  Called to start up the action processing thread. The derived class
//  does this when he's ready to start processing.
//
tCIDLib::TVoid TBaseIRSrvDriver::StartActionsThread()
{
    // If not already started, then start up the action processing thread
    if (!m_thrActions.bIsRunning())
        m_thrActions.Start();
}


// Wait for existing actions to be processed
tCIDLib::TVoid TBaseIRSrvDriver::WaitForActions()
{
    // If already empty, then just return
    if (m_thrActions.bIsEmpty())
        return;

    // Else we want to wait for it
    tCIDLib::TCard4 c4Count = 0;
    TThread*        pthrThis = TThread::pthrCaller();
    while (c4Count < 10)
    {
        if (m_thrActions.bIsEmpty())
            break;
        c4Count++;

        // Sleep for a bit, but return if we get a shutdown request
        if (!pthrThis->bSleep(200))
            return;
    }

    // If they still haven't completed, then throw
    if (!m_thrActions.bIsEmpty())
    {
        facCQCIR().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kIRErrs::errcTrain_ActiveEvents
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}



// ---------------------------------------------------------------------------
//  TBaseIRSrvDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TBaseIRSrvDriver::bIsRecKeyUsed(const   TString&    strKeyToCheck
                                ,       TString&    strTitleToFill)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    const TCQCStdKeyedCmdSrc* pcsrcInvoke = m_irrcData.pcsrcFindByKey(strKeyToCheck);
    if (pcsrcInvoke)
    {
        strTitleToFill = pcsrcInvoke->strTitle();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TBaseIRSrvDriver::bStoreRecTrainingData(const TCQCStdKeyedCmdSrc& csrcToStore)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Make a copy of the config data, add/update this key from it, and
    //  then see if we can store it. If we can, then we'll go ahead and
    //  add/update it in our live copy. If it fails, we'll throw out of
    //  the save event and won't have changed anything.
    //
    TIRReceiverCfg irrcTmp(m_irrcData);
    irrcTmp.bAddOrUpdate(csrcToStore);
    StoreRecCfg(irrcTmp);

    // Update our live config now, and return whether we added or updated
    return m_irrcData.bAddOrUpdate(csrcToStore);
}


tCIDLib::TVoid TBaseIRSrvDriver::DeleteAllEvents()
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // Try it on a temp copy first
    TIRReceiverCfg irrcTmp(m_irrcData);
    irrcTmp.Reset();
    StoreRecCfg(irrcTmp);

    // It worked, so do it to our data
    m_irrcData.Reset();
}


tCIDLib::TVoid TBaseIRSrvDriver::DeleteRecEvent(const TString& strKeyToDelete)
{
    // If we aren't online, then throw, else lock and do it
    CheckOnline(CID_FILE, CID_LINE);

    // If its not a known key, then throw an exception
    {
        if (!m_irrcData.pcsrcFindByKey(strKeyToDelete))
        {
            facCQCIR().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kIRErrs::errcCfg_KeyNotFound
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotFound
                , strKeyToDelete
            );
        }
    }

    //
    //  Make a copy of the config data, remove this key from it, and
    //  then see if we can store it. If we can, then we'll go ahead and
    //  remove it from our live copy. If it fails, we'll throw out of
    //  the save event and won't have changed anything.
    //
    TIRReceiverCfg irrcTmp(m_irrcData);
    irrcTmp.RemoveKey(strKeyToDelete);
    StoreRecCfg(irrcTmp);

    // Ok, it worked so we can remove it from our live data
    m_irrcData.RemoveKey(strKeyToDelete);
}


tCIDLib::TVoid
TBaseIRSrvDriver::LoadBlasterDevModel(const TString& strModelName)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  The client wants us to load up a new device file. Until we are ready
    //  to load up the new file, we don't need to do any locking on this
    //  object, so lets do all the work to get the file and parse it first.
    //
    //  So we have to get the client proxy for the IR protocol repository
    //  server.
    //
    tCQCIR::TRepoSrvProxy orbcIRRepo = facCQCIR().orbcRepoSrvProxy();

    TIRBlasterDevModel irbdmNew;
    orbcIRRepo->QueryDevModel(strModelName, m_strRepoName, irbdmNew);

    //
    //  Set up a persistant data object with the models we current have
    //  loaded, then add the new one to it, and try to store that one. If
    //  we can't store it, we don't want to add it to our data, so by doing
    //  this first, if it throws back to the caller, we've not gotten any
    //  data out of sync.
    //
    //  If the model is already in our config, then don't bother since
    //  we already have it loaded, and they are just asking us to update
    //  the model data.
    //
    if (!m_irbcData.bModelExists(strModelName))
    {
        TIRBlasterPerData perdTmp(strMoniker());
        perdTmp.SetModels(m_irbcData.cursModels());
        perdTmp.AddModel(strModelName);
        StoreBlastConfig(perdTmp);
    }

    // And now add/update it in our config data object
    tCIDLib::TBoolean bAdded;
    m_irbcData.irbdmAddOrUpdate(irbdmNew, bAdded);
}


tCIDLib::TVoid TBaseIRSrvDriver::
QueryBlasterConfig( TVector<TIRBlasterDevModelInfo>&    colList
                    , tCIDLib::TCard4&                  c4ZoneCountToFill)
{
    // If we aren't online, then throw. Else do it
    CheckOnline(CID_FILE, CID_LINE);

    // Give him a copy of our zone count member
    c4ZoneCountToFill = c4ZoneCount();

    //
    //  Loop through the devices we have loaded and load the caller's list.
    //  We don't have the file name information here, so we just set it to
    //  an empty string.
    //
    TIRBlasterCfg::TModelCursor cursModels(m_irbcData.cursModels());
    for (; cursModels; ++cursModels)
    {
        const TIRBlasterDevModel& irbdmCur = *cursModels;
        colList.objAdd
        (
            TIRBlasterDevModelInfo
            (
                irbdmCur.strModel()
                , irbdmCur.strMake()
                , irbdmCur.strDescription()
                , TString::strEmpty()
                , irbdmCur.eCategory()
            )
        );
    }
}


tCIDLib::TVoid
TBaseIRSrvDriver::QueryMappedRecEvents(TVector<TCQCStdKeyedCmdSrc>& colToFill)
{
    // If we aren't online, then throw, else do it
    CheckOnline(CID_FILE, CID_LINE);

    // Iterate our collection and copy over each on to the caller's collection
    TIRReceiverCfg::TEventList::TCursor cursCopy(m_irrcData.cursEvents());
    for (; cursCopy; ++cursCopy)
        colToFill.objAdd(*cursCopy);
}


tCIDLib::TVoid
TBaseIRSrvDriver::StoreBlastConfig(const TIRBlasterPerData& perdToStore)
{
    //
    //  Create the configuration repository key for our configuration
    //  info.
    //
    TString strKey(kCQCKit::pszOSKey_IRBlasterCfg);
    strKey.Append(kCIDLib::chForwardSlash);
    strKey.Append(strMoniker());

    // Get a config repo client proxy for the local machine
    TCfgServerClient cfgscTar;
    cfgscTar.bAddOrUpdate(strKey, m_c4BlastCfgVerId, perdToStore, 2048);

    // It worked, so bump the reconfigured instrumentation field
    IncReconfigCounter();
}


tCIDLib::TVoid
TBaseIRSrvDriver::StoreRecCfg(const TIRReceiverCfg& irrcToStore)
{
    //
    //  Create the configuration repository key for our configuration
    //  info.
    //
    TString strKey(kCQCKit::pszOSKey_IRReceiverCfg);
    strKey.Append(kCIDLib::chForwardSlash);
    strKey.Append(strMoniker());

    //
    //  Get a config repo client proxy for the local machine, and
    //  add or update the key with our current config data. In case
    //  we are creating it, add some extra reserve space so that it
    //  won't have to be moved if we expand it.
    //
    TCfgServerClient cfgscTar;
    cfgscTar.bAddOrUpdate(strKey, m_c4RecCfgVerId, irrcToStore, 4096);
}


tCIDLib::TVoid
TBaseIRSrvDriver::ThrowUnknownId(const  TString&            strId
                                , const tCIDLib::TCh* const pszMethod)
{
    facCQCIR().ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kIRErrs::errcGen_UnknownId
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::Unknown
        , strId
        , TString(pszMethod)
    );
}


tCIDLib::TVoid
TBaseIRSrvDriver::UnloadBlasterDevModel(const TString& strModelName)
{
    // If we aren't online, then throw
    CheckOnline(CID_FILE, CID_LINE);

    //
    //  Set up a persistent data object with our current models, then remove
    //  the passed one, and try to store that. If it fails, we'll thrown
    //  back to the client without any changes. Else, we know it's stored
    //  and we can remove it from our list.
    //
    TIRBlasterPerData perdTmp(strMoniker());
    perdTmp.SetModels(m_irbcData.cursModels());
    perdTmp.RemoveModel(strModelName);
    StoreBlastConfig(perdTmp);

    // Looks ok, so remove it from our live list also
    m_irbcData.bRemoveModel(strModelName);
}


