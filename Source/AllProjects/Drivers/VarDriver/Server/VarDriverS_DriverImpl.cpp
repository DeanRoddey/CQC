//
// FILE NAME: VarDriverS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2003
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
//  This is a test driver which implements driver interface for the variable
//  driver..
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VarDriverS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TVarDriverSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace VarDriverS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  The configuration repository key for our config data
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszCfgKey = L"/Drivers/VarDriver/%(m)/CfgData";
}



// ---------------------------------------------------------------------------
//   CLASS: TVarDriverSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TVarDriverSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TVarDriverSDriver::TVarDriverSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4ChangeCnt(0)
{
}

TVarDriverSDriver::~TVarDriverSDriver()
{
}


// ---------------------------------------------------------------------------
//  TVarDriverSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  These are not actually used. They return canned data for testing
//  purposes, so that we have a simple driver that will always be able to
//  be online.
//

tCIDLib::TBoolean
TVarDriverSDriver::bQueryData(  const   TString&            strQueryType
                                , const TString&            strDataName
                                ,       tCIDLib::TCard4&    c4OutBytes
                                ,       THeapBuf&           mbufToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;
    c4OutBytes = 0;

    if ((strQueryType == L"bQueryData") && (strDataName == L"TestQD"))
    {
        c4OutBytes = 3;
        mbufToFill.PutCard1(0, 0);
        mbufToFill.PutCard1(0x74, 1);
        mbufToFill.PutCard1(0xF2, 2);
    }
     else if (strQueryType == kVarDriverC::pszCfg_CurFldVals)
    {
        //
        //  This is used by the client to get info on the field that the
        //  user has selected. It can't just do a field value read, since
        //  it also needs to get the default value for display.
        //
        //  The data name is the name of the field he wants the info
        //  for.
        //
        TCQCFldDef* pflddUpdate = m_cfgData.pflddFind(strDataName);
        if (!pflddUpdate)
        {
            c4OutBytes = 0;
            return kCIDLib::False;
        }

        // We format out the current value, a separator, then the default
        tCIDLib::TCard4     c4SerialNum = 0;
        tCQCKit::EFldTypes  eType;
        TString             strCurVal;
        bReadFieldByName(strDataName, c4SerialNum, strCurVal, eType);

        {
            TBinMBufOutStream strmOut(&mbufToFill);
            strmOut << strCurVal
                    << tCIDLib::EStreamMarkers::Frame
                    << pflddUpdate->strExtraCfg()
                    << tCIDLib::EStreamMarkers::EndObject
                    << kCIDLib::FlushIt;

            c4OutBytes = strmOut.c4CurSize();
        }
    }
     else if ((strQueryType == L"CQCQueryLrgDrvImg") && (strDataName == L"TestImg"))
    {
        // Read a test image into the caller's buffer
        TPathStr pathImg(facCQCKit().strMacroRootPath());
        pathImg.AddLevel(L"TestData");
        pathImg.AddLevel(L"TestImg1.png");
        TBinaryFile flImg(pathImg);

        try
        {
            flImg.Open
            (
                tCIDLib::EAccessModes::Read
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::None
            );

            c4OutBytes = flImg.c4ReadBuffer
            (
                mbufToFill
                , tCIDLib::TCard4(flImg.c8CurSize())
                , tCIDLib::EAllData::FailIfNotAll
            );

            bRet = c4OutBytes != 0;
        }

        catch(...)
        {
            bRet = kCIDLib::False;
        }
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strQueryType
            , strDataName
        );
    }
    return bRet;
}


tCIDLib::TBoolean
TVarDriverSDriver::bQueryData2( const   TString&            strQueryType
                                ,       tCIDLib::TCard4&    c4IOBytes
                                ,       THeapBuf&           mbufIO)
{
    if (strQueryType == L"bQueryData2")
    {
        // Make sure we got the expected values
        if (c4IOBytes != 2)
        {
            facVarDriverS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kVarSErrs::errcCmd_BadByteCnt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
                , TCardinal(c4IOBytes)
            );
        }

        // And check the values
        if ((mbufIO[0] != 44) || (mbufIO[1] != 77))
        {
            facVarDriverS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kVarSErrs::errcCmd_BadBytes
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        c4IOBytes = 4;
        mbufIO.PutCard1(1, 0);
        mbufIO.PutCard1(0x64, 1);
        mbufIO.PutCard1(0x9A, 2);
        mbufIO.PutCard1(0xF4, 3);
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strQueryType
        );
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean
TVarDriverSDriver::bQueryTextVal(const  TString&    strQueryType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    if ((strQueryType == L"bQueryTextVal") && (strDataName == L"TestTV"))
    {
        strToFill = L"Query Text Val Response";
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strQueryType
            , strDataName
        );
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TVarDriverSDriver::bQueryVal(const TString& strValId)
{
    tCIDLib::TBoolean bRet;
    if (strValId == L"bQueryTrueVal")
    {
        bRet = kCIDLib::True;
    }
     else if (strValId == L"bQueryFalseVal")
    {
        bRet = kCIDLib::False;
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
        );
    }
    return bRet;
}

tCIDLib::TBoolean
TVarDriverSDriver::bQueryVal2(const TString& strValId, const TString& strValName)
{
    tCIDLib::TBoolean bRet;
    if ((strValId == L"bQueryVal2") && (strValName == L"TrueVal"))
    {
        bRet = kCIDLib::True;
    }
     else if ((strValId == L"bQueryVal2") && (strValName == L"FalseVal"))
    {
        bRet = kCIDLib::False;
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
            , strValName
        );
    }
    return bRet;
}

tCIDLib::TBoolean
TVarDriverSDriver::bSendData(const  TString&            strSendType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       THeapBuf&           mbufToSend)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if ((strSendType == L"bSendData") && (strDataName == L"TestSD"))
    {
        // Check the incoming data
        if ((c4Bytes == 4)
        &&  (mbufToSend[0] == 4)
        &&  (mbufToSend[1] == 5)
        &&  (mbufToSend[2] == 6)
        &&  (mbufToSend[3] == 7))
        {
            bRet = kCIDLib::True;
        }

        // Load up return info
        c4Bytes = 2;
        mbufToSend.PutCard1(55, 0);
        mbufToSend.PutCard1(73, 1);
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strSendType
            , strDataName
        );
    }
    return bRet;
}

tCIDLib::TCard4 TVarDriverSDriver::c4QueryVal(const TString& strValId)
{
    if (strValId != L"c4QueryVal")
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
        );
    }
    return 83;
}

tCIDLib::TCard4
TVarDriverSDriver::c4QueryVal2( const   TString&    strValId
                                , const TString&    strValName)
{
    if ((strValId != L"c4QueryVal2") || (strValName != L"TestQV2"))
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
            , strValName
        );
    }
    return 103;
}

tCIDLib::TInt4 TVarDriverSDriver::i4QueryVal(const TString& strValId)
{
    if (strValId != L"i4QueryVal")
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
        );
    }
    return -17;
}

tCIDLib::TInt4
TVarDriverSDriver::i4QueryVal2(const TString& strValId, const TString& strValName)
{
    if ((strValId != L"i4QueryVal") || (strValName != L"TestQV2"))
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownQIds
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strValId
            , strValName
        );
    }
    return -77;
}




tCIDLib::TCard4 TVarDriverSDriver::c4QueryCfg(THeapBuf& mbufToFill)
{
    // Create an output stream over the buffer and stream out config to it
    TBinMBufOutStream strmTar(&mbufToFill);
    strmTar << m_cfgData << kCIDLib::FlushIt;

    return strmTar.c4CurSize();
}


// We override this to let the client send us some commands
tCIDLib::TCard4
TVarDriverSDriver::c4SendCmd(const  TString&    strCmdId
                            , const TString&    strParms)
{
    if (strCmdId == kVarDriverC::pszCfg_SetDefault)
    {
        // Pass it to a helper method that does the work
        SetFieldDefault(strParms);
    }
     else
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcCmd_UnknownCmdId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmdId
        );
    }
    return 0;
}


//
//  Called by the client side interface to store new config data.
//
tCIDLib::TVoid
TVarDriverSDriver::SetConfig(const  tCIDLib::TCard4 c4Bytes
                            , const THeapBuf&       mbufNewCfg)
{
    //
    //  Create an input stream and stream in our configuration data. Allow
    //  any exceptions to propogate back to the caller. There is a leading
    //  string that tells us what he's sending us, since we can get a full
    //  config upload or some partial bits.
    //
    TBinMBufInStream strmSrc(&mbufNewCfg, c4Bytes);

    TString strCmd;
    strmSrc >> strCmd;

    if (strCmd == kVarDriverC::pszCfg_Full)
    {
        // Stream in the full config that should follow this
        strmSrc >> m_cfgData;

        // Build the configuration key we'll need
        TString strCfgKey(VarDriverS_DriverImpl::pszCfgKey);
        strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);

        //
        //  See if we can get it stored in the local config repository. If not,
        //  then let the exception propogate back to the user. We only want to
        //  set the fields if the new config is stored away safely. If we create
        //  it, leave a good bit of expansion room.
        //
        //  Get the repository path for it, which is based on moniker because
        //  we could have multiple instances of the driver loaded in the same
        //  CQCServer. It's not likely but possible.
        //
        TCfgServerClient cfgcRepo(5000);
        tCIDLib::TCard4 c4Ver = 0;
        cfgcRepo.bAddOrUpdate(strCfgKey, c4Ver, m_cfgData, 4096);

        // Register new fields now based on this new configuration
        RegisterFields();

        // Bump the reconfigured instrumentation field
        IncReconfigCounter();

        // Reset our change count
        m_c4ChangeCnt = 0;
    }
}


// ---------------------------------------------------------------------------
//  TVarDriverSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TVarDriverSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource, so just say we are good
    return kCIDLib::True;
}


tCIDLib::TBoolean TVarDriverSDriver::bWaitConfig(TThread& thrThis)
{
    //
    //  Try to get a configuration repository client proxy and load up our
    //  config. If no config has been stored yet, then we do nothing and say
    //  we are good. If we have some, then we register the fields. If we
    //  get any error while trying to load the config, we just keep waiting.
    //
    try
    {
        TCfgServerClient cfgcRepo(2000);
        TString strCfgKey(VarDriverS_DriverImpl::pszCfgKey);
        strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);
        tCIDLib::TCard4 c4Ver = 0;

        const tCIDLib::ELoadRes eRes = cfgcRepo.eReadObject
        (
            strCfgKey, m_cfgData, c4Ver
        );

        //
        //  If we got new data, the load up the fields, else we start with
        //  no fields.
        //
        if (eRes == tCIDLib::ELoadRes::NewData)
        {
            RegisterFields();

            // Reset our change count
            m_c4ChangeCnt = 0;
        }
    }

    catch(...)
    {
        // Keep waiting for a good config read
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCQCKit::ECommResults
TVarDriverSDriver::eBoolFldValChanged(  const   TString&            strName
                                        , const tCIDLib::TCard4
                                        , const tCIDLib::TBoolean   bVal)
{
    PersistValue(strName, bVal ? kCQCKit::pszFld_True : kCQCKit::pszFld_False);
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVarDriverSDriver::eCardFldValChanged(  const   TString&        strName
                                        , const tCIDLib::TCard4
                                        , const tCIDLib::TCard4 c4Val)
{
    TString strVal;
    strVal.AppendFormatted(c4Val);
    PersistValue(strName, strVal);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TVarDriverSDriver::eConnectToDevice(TThread&)
{
    // Reset some info
    m_c4ChangeCnt = 0;

    //
    //  We are always 'connected', because we don't have a real device. Since
    //  we set the online state during our init, and never go offline, this
    //  method will never be called, but we have to make the compiler happy,
    //  so just return success.
    //
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVarDriverSDriver::eFloatFldValChanged( const   TString&            strName
                                        , const tCIDLib::TCard4
                                        , const tCIDLib::TFloat8    f8Val)
{
    TString strVal;
    strVal.AppendFormatted(f8Val, 4);
    PersistValue(strName, strVal);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TVarDriverSDriver::eInitializeImpl()
{
    //
    //  Set our poll time to 1 minute, which we use to flush changes to disk.
    //  This insures we get them flushed before too long, but also insures
    //  we don't beat the config server to death.
    //
    SetPollTimes(60000, 5000);

    //
    //  We have no real init to do. We have to load our configuraiton before
    //  we can set up our fields, so just return that we are waiting for
    //  config.
    //
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults
TVarDriverSDriver::eIntFldValChanged(const  TString&        strName
                                    , const tCIDLib::TCard4
                                    , const tCIDLib::TInt4  i4Val)
{
    TString strVal;
    strVal.AppendFormatted(i4Val);
    PersistValue(strName, strVal);

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TVarDriverSDriver::ePollDevice(TThread&)
{
    //
    //  If there's been new default values set since we last checked, store
    //  the changes. Our poll time is set to 30 seconds, so we won't store
    //  any quicker than once per that often.
    //
    if (m_c4ChangeCnt)
        StoreConfig();

    //
    //  We will always be successful, since we have no real device that can
    //  go offline.
    //
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVarDriverSDriver::eStringFldValChanged(const   TString&        strFldName
                                        , const tCIDLib::TCard4
                                        , const TString&        strValue)
{
    PersistValue(strFldName, strValue);
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVarDriverSDriver::eSListFldValChanged( const   TString&            strFldName
                                        , const tCIDLib::TCard4
                                        , const TVector<TString>&   colVal)
{
    //
    //  In this case, we will do it ourselves instead of using the PersistValue
    //  method, since we can avoid formatting the string out if we don't need
    //  to persist it.
    //
    TCQCFldDef* pflddUpdate = m_cfgData.pflddFind(strFldName);
    if (!pflddUpdate)
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcFld_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strFldName
        );
    }

    // We use the private field as a persistence flag
    if (pflddUpdate->bPrivate())
    {
        TString strValue;
        TStringTokenizer::BuildQuotedCommaList(colVal, strValue);
        pflddUpdate->strExtraCfg(strValue);

        // Bump the change counter
        m_c4ChangeCnt++;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TVarDriverSDriver::eTimeFldValChanged(  const   TString&            strName
                                        , const tCIDLib::TCard4
                                        , const tCIDLib::TCard8&    c8Time)
{
    // Format it out in full date/time format
    TString strVal(L"0x");
    strVal.AppendFormatted(c8Time, tCIDLib::ERadices::Hex);
    PersistValue(strName, strVal);
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TVarDriverSDriver::ReleaseCommResource()
{
    // If any outstanding changes, do a final flush of config
    if (m_c4ChangeCnt)
        StoreConfig();
}


tCIDLib::TVoid TVarDriverSDriver::TerminateImpl()
{
}


// ---------------------------------------------------------------------------
//  TVarDriverSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This will take the current value of the passed field and make it the
//  persistently configured default value for that field, if this field is
//  marked as such.
//
tCIDLib::TVoid
TVarDriverSDriver::PersistValue(const   TString&    strField
                                , const TString&    strValue)
{
    //
    //  Find this guy in the driver config and then get it's current
    //  value and put it in the extra config value. If not found, then
    //  throw back to the caller.
    //
    TCQCFldDef* pflddUpdate = m_cfgData.pflddFind(strField);
    if (!pflddUpdate)
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcFld_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strField
        );
    }

    // We use the private field as a persistence flag
    if (pflddUpdate->bPrivate())
    {
        pflddUpdate->strExtraCfg(strValue);

        // Bump the change counter
        m_c4ChangeCnt++;
    }
}


tCIDLib::TVoid TVarDriverSDriver::RegisterFields()
{
    //
    //  Tell our base class about our fields. We can pass the collection inside
    //  the config object directly in, since it's in the form required already.
    //
    //  Int his case, we don't have to go back and look up the field ids, since
    //  we are keeping the field list around and the SetFields method will
    //  set the ids into the field def objects.
    //
    TVarDrvCfg::TFldInfoList& colFlds = m_cfgData.colFields();
    SetFields(colFlds);

    //
    //  Go back and set each field to the default value indicated in the
    //  field def object's extra config field. This insures that they have
    //  an initial value that is both valid for their limits and the value
    //  that the user wants.
    //
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    const tCIDLib::TCard4 c4FLId = c4FieldListId();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = colFlds[c4Index];

        //
        //  Be extra careful and catch a rejected value if it should happen,
        //  though it shouldn't be possible. If so, then skip it. The last
        //  value indicates it's a write from the driver, not from a client,
        //  so we can write to read only fields from our end.
        //
        try
        {
            WriteField
            (
                c4FLId, flddCur.c4Id(), flddCur.strExtraCfg(), kCIDLib::True
            );
        }

        catch(...)
        {

        }
    }
}



//
//  Though we support automatic persistence, in some cases they want to
//  have a default value that is separate from the current value, so the
//  field will come back to that value on startup, despite what it was
//  set to in the meantime.
//
tCIDLib::TVoid TVarDriverSDriver::SetFieldDefault(const TString& strField)
{
    //
    //  Find this guy in the driver config and then get it's current
    //  value and put it in the extra config value. If not found, then
    //  throw back to the caller.
    //
    TCQCFldDef* pflddUpdate = m_cfgData.pflddFind(strField);
    if (!pflddUpdate)
    {
        facVarDriverS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kVarSErrs::errcFld_NotFound
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strField
        );
    }

    // Get the current value of the field
    TString strValue;
    tCIDLib::TCard4 c4Serial = 0;
    tCQCKit::EFldTypes eType;
    if (bReadFieldByName(strField, c4Serial, strValue, eType))
    {
        pflddUpdate->strExtraCfg(strValue);

        // Bump the change counter
        m_c4ChangeCnt++;
    }
}


// Store our config data out to the config server
tCIDLib::TVoid TVarDriverSDriver::StoreConfig()
{
    // Store the configuration back out
    TString strCfgKey(VarDriverS_DriverImpl::pszCfgKey);
    strCfgKey.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);

    // And update it in the repository
    try
    {
        TCfgServerClient cfgcRepo(5000);
        cfgcRepo.c4UpdateObject(strCfgKey, m_cfgData);

        // We were successful, so clear the changes flag
        m_c4ChangeCnt = 0;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


