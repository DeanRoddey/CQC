//
// FILE NAME: iTunesRepoTMS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2012
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
//  This is the main implementation file of the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "iTunesRepoTMS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRepoTMSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: iTunesRepoTMSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  iTunesRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesRepoTMSDriver::TiTunesRepoTMSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldId_DBSerialNum(kCIDLib::c4MaxCard)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_LoadStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_porbcTray(nullptr)
{
}

TiTunesRepoTMSDriver::~TiTunesRepoTMSDriver()
{
}


// ---------------------------------------------------------------------------
//  TiTunesRepoTMSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  These are all just passthroughs to the tray app, which just returns the
//  info in the same form that we would have.
//
tCIDLib::TBoolean
TiTunesRepoTMSDriver::bQueryData(const  TString&            strQType
                                , const TString&            strDName
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       THeapBuf&           mbufToFill)
{
    CheckConnected();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        bRet = m_porbcTray->bQueryData(strQType, strDName, c4Bytes, mbufToFill);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return bRet;
}


tCIDLib::TBoolean
TiTunesRepoTMSDriver::bQueryData2(  const   TString&            strQType
                                    ,       tCIDLib::TCard4&    c4IOBytes
                                    ,       THeapBuf&           mbufIO)
{
    CheckConnected();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        bRet = m_porbcTray->bQueryData2(strQType, c4IOBytes, mbufIO);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return bRet;
}


tCIDLib::TBoolean
TiTunesRepoTMSDriver::bQueryTextVal(const   TString&    strQType
                                    , const TString&    strDName
                                    ,       TString&    strToFill)
{
    CheckConnected();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        bRet = m_porbcTray->bQueryTextVal(strQType, strDName, strToFill);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return bRet;
}


tCIDLib::TCard4 TiTunesRepoTMSDriver::c4QueryVal(const TString& strValId)
{
    CheckConnected();

    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        c4Ret =  m_porbcTray->c4QueryVal(strValId);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return c4Ret;
}


tCIDLib::TBoolean
TiTunesRepoTMSDriver::bSendData(const   TString&            strSType
                                ,       TString&            strDName
                                ,       tCIDLib::TCard4&    c4Bytes
                                ,       THeapBuf&           mbufToSend)
{
    CheckConnected();

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        bRet = m_porbcTray->bQueryData(strSType, strDName, c4Bytes, mbufToSend);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return bRet;
}


tCIDLib::TCard4
TiTunesRepoTMSDriver::c4SendCmd(const TString& strCmd, const TString& strParms)
{
    CheckConnected();

    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        c4Ret =  m_porbcTray->c4SendCmd(strCmd, strParms);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return c4Ret;
}



// ---------------------------------------------------------------------------
//  TiTunesRepoTMSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

// Try to create a client proxy for the tray monitor
tCIDLib::TBoolean TiTunesRepoTMSDriver::bGetCommResource(TThread&)
{
    try
    {
        // See if we can find the object id, with a pretty short timeout
        const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusSecs(2);
        TOrbObjId ooidTray;
        if (facCIDOrbUC().bGetNSObject(m_strBinding, ooidTray, enctEnd))
            m_porbcTray = new TiTRepoIntfClientProxy(ooidTray, m_strBinding);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // If we got the client proxy allocated, we succeeded
    return (m_porbcTray != 0);
}


// Nothing to do. The tray loads the database
tCIDLib::TBoolean TiTunesRepoTMSDriver::bWaitConfig(TThread&)
{
    return kCIDLib::True;
}


tCQCKit::ECommResults
TiTunesRepoTMSDriver::eBoolFldValChanged(const  TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    // If not connected to the tray monitor, then reject
    CheckConnected();

    if (c4FldId == m_c4FldId_ReloadDB)
    {
        if (m_porbcTray->bReloadDB())
        {
            LogMsg
            (
                L"The Tray Monitor rejected the reload command"
                , tCQCKit::EVerboseLvls::Low
                , CID_FILE
                , CID_LINE
            );
            IncFailedWriteCounter();
            return tCQCKit::ECommResults::ValueRejected;
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TiTunesRepoTMSDriver::eConnectToDevice(TThread& thrCaller)
{
    //
    //  We have already really connected, in the getting of the comm resource,
    //  but we will do an initial query of the status of the tray monitor's
    //  iTunes interface and get that stored.
    //


    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TiTunesRepoTMSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCOtherConnCfg::clsThis())
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
            , TCQCOtherConnCfg::clsThis()
        );
    }

    SetPollTimes(2000, 5000);

    // Set up the binding we expect to find the tray monitor at
    m_strBinding = TiTRepoIntfClientProxy::strImplBinding;
    m_strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddNew;

    TString strFldDBSerialNum(L"DBSerialNum");
    TString strFldLoaded(L"Loaded");
    TString strFldReloadDB(L"ReloadDB");
    TString strFldLoadStatus(L"LoadStatus");
    TString strFldStatus(L"Status");
    TString strFldTitleCnt(L"TitleCount");

    const tCIDLib::TBoolean bV2(c4ArchVersion() > 1);

    //
    //  If V2, prepend the device class prefix to the fields used in V2. Else,
    //  set up any V1 only fields.
    //
    if (bV2)
    {
        const TString& strPref(L"MREPO#");
        strFldDBSerialNum.Prepend(strPref);
        strFldLoadStatus.Prepend(strPref);
        strFldReloadDB.Prepend(strPref);
        strFldStatus.Prepend(strPref);
        strFldTitleCnt.Prepend(strPref);
    }
     else
    {
        flddNew.Set
        (
            strFldLoaded
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::Read
        );
        colFlds.objAdd(flddNew);
    }

    //
    //  These are the same, the V2 ones just have the device class prefix.
    //  The DBSerialNum and LoadStatus fields weren't there in V1 but it
    //  doesn't hurt anything to include them V1 drivers, and it makes things
    //  simpler since we don't have to special case them.
    //
    flddNew.Set
    (
        strFldDBSerialNum
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldLoadStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
        , L"Enum: Initializing, Loading, Ready, Error"
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldReloadDB
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldTitleCnt
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);


    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);
    m_c4FldId_DBSerialNum = pflddFind(strFldDBSerialNum, kCIDLib::True)->c4Id();
    m_c4FldId_LoadStatus = pflddFind(strFldLoadStatus, kCIDLib::True)->c4Id();
    m_c4FldId_ReloadDB = pflddFind(strFldReloadDB, kCIDLib::True)->c4Id();
    m_c4FldId_Status = pflddFind(strFldStatus, kCIDLib::True)->c4Id();
    m_c4FldId_TitleCnt = pflddFind(strFldTitleCnt, kCIDLib::True)->c4Id();

    if (!bV2)
        m_c4FldId_Loaded = pflddFind(strFldLoaded, kCIDLib::True)->c4Id();

    //
    //  Init the status field to indicate not loaded. The loaded field will have
    //  a default initial value of false, which is what we want. And set the load
    //  status field to loading.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , faciTunesRepoTMS().strMsg(kiTunesTMMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Initializing", kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TiTunesRepoTMSDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  Just ping it for the status, and update the status field with the
    //  new status. If we lose connection, restart the connection.
    //
    try
    {
        tCIDLib::TCard4 c4TitleCnt;
        tCQCMedia::ELoadStatus eCurStatus;
        tCQCMedia::ELoadStatus eLoadStatus;
        m_porbcTray->QueryStatus(eCurStatus, eLoadStatus, m_strPollTmp, c4TitleCnt);

        bStoreStringFld
        (
            m_c4FldId_Status
            , tCQCMedia::strXlatELoadStatus(eCurStatus)
            , kCIDLib::True
        );
        bStoreStringFld
        (
            m_c4FldId_LoadStatus
            , tCQCMedia::strXlatELoadStatus(eLoadStatus)
            , kCIDLib::True
        );

        bStoreStringFld(m_c4FldId_DBSerialNum, m_strPollTmp, kCIDLib::True);
        bStoreCardFld(m_c4FldId_TitleCnt, c4TitleCnt, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        //
        //  Assume the worst and recylce our client proxy. Note in this
        //  case we can't test if there is a loss of connection, since
        //  the server may still be there, but not the server side
        //  interface for this client proxy. The iTunes interface is
        //  configurable and it could get removed.
        //
        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TiTunesRepoTMSDriver::ReleaseCommResource()
{
    // Free the client proxy if we have one
    if (m_porbcTray)
    {
        try
        {
            delete m_porbcTray;
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        m_porbcTray = 0;
    }
}


tCIDLib::TVoid TiTunesRepoTMSDriver::TerminateImpl()
{
}



// ---------------------------------------------------------------------------
//  TiTunesRepoTMSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called by the backdoor handler methods, to see if we are connected and
//  throws if not. Generally shouldn't happen, since the client should check
//  if we are ready. But there's a window of opportunity for it to happen if
//  we go offline after they check and dispatch a command.
//
tCIDLib::TVoid TiTunesRepoTMSDriver::CheckConnected()
{
    if (!m_porbcTray || m_porbcTray->bCheckForLostConnection())
    {
        faciTunesRepoTMS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kiTunesTMErrs::errcProto_NotReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}

