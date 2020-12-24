//
// FILE NAME: PlexRepoS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/24/2008
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
#include    "PlexRepoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TPlexMediaRepoEng,TCQCStdMediaRepoEng)
RTTIDecls(TPlexRepoSDriver,TCQCStdMediaRepoDrv)



// ---------------------------------------------------------------------------
//  CLASS: TPlexMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPlexMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------

// Pass on the moniker and the media types we support to our bass class
TPlexMediaRepoEng::TPlexMediaRepoEng(const TString& strMoniker) :

    TCQCStdMediaRepoEng(strMoniker, tCQCMedia::EMTFlags::Movie)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
{
}

TPlexMediaRepoEng::~TPlexMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TPlexMediaRepoEng: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called if an image needs to be loaded.
//
//  We let exceptions propogate back.
//
tCIDLib::TCard4
TPlexMediaRepoEng::c4LoadRawCoverArt(const  TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType)
{

    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        //
        //  If we have already loaded what the caller wants, then we shouldn't get
        //  called, but return it anyway.
        //
        tCIDLib::TBoolean bScaleDown = kCIDLib::False;
        c4Ret = mimgToLoad.c4QueryArt(mbufToFill, eType, 0);

        if (!c4Ret)
        {
            //
            //  Not loaded yet, so let's try to load it. If they asked for the small
            //  art, see if we have a separate small art image. If so, we can get
            //  that directly. Else, we have to load the large art and scale it.
            //
            TString strPath = mimgToLoad.strArtPath(eType);
            if (strPath.bIsEmpty() && (eType == tCQCMedia::ERArtTypes::SmlCover))
            {
                strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::LrgCover);
                bScaleDown = kCIDLib::True;
            }

            // If we got something, then let's try to query the image from Plex
            if (!strPath.bIsEmpty())
                c4Ret = c4DownloadArt(strPath, mbufToFill);
        }

        if (eType == tCQCMedia::ERArtTypes::SmlCover)
        {
            //
            //  If getting small art and we are scalling down the large,
            //  then do that.
            //
            if (c4Ret && bScaleDown)
            {
                c4Ret = facCIDImgFact().c4CompactToJPEG
                (
                    mbufToFill, c4Ret, TSize(kCQCMedia::c4DefThumbSz)
                );
            }
        }
         else if (eType == tCQCMedia::ERArtTypes::LrgCover)
        {
            // Make sure it's not stupidly large
            c4Ret = facCIDImgFact().c4CompactToJPEG
            (
                mbufToFill, c4Ret, TSize(720, 640)
            );
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
//  TPlexMediaRepoEng: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TPlexMediaRepoEng::SetServerEP(const TIPEndPoint& ipepPlexSrv)
{
    m_ipepSrv = ipepPlexSrv;
}


tCIDLib::TVoid
TPlexMediaRepoEng::SetVerbosity(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
}


// ---------------------------------------------------------------------------
//  TPlexMediaRepoEng: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Downloads art from the Plex server. This is called if we need to fault in
//  some art. The loader loads thumbs if possible and leaves the large art to
//  be faulted in upon use.
//
tCIDLib::TCard4
TPlexMediaRepoEng::c4DownloadArt(const TString& strPath, TMemBuf& mbufToFill)
{
    TString strURL;
    TURL urlImg;
    tCIDLib::TKVPList colHTTPQVals;
    urlImg.Set
    (
        tCIDSock::EProtos::HTTP
        , TString::strEmpty()
        , TString::strEmpty()
        , m_ipepSrv.strHostName()
        , m_ipepSrv.ippnThis()
        , strPath
        , TString::strEmpty()
        , TString::strEmpty()
        , colHTTPQVals
        , kCIDLib::False
    );

    tCIDLib::TCard4 c4RetLen = 0;
    try
    {
        tCIDNet::EHTTPCodes     eCodeType;
        TString                 strRepText;
        TString                 strContType;
        tCIDLib::TKVPList       colInHdrLines;
        tCIDLib::TKVPList       colOutHdrLines;

        THTTPClient httpcImgs;
        const tCIDLib::TCard4 c4Res = httpcImgs.c4SendGet
        (
            nullptr
            , urlImg
            , TTime::enctNowPlusSecs(5)
            , L"CQC Plex Repo"
            , L"image/*"
            , eCodeType
            , strRepText
            , colOutHdrLines
            , strContType
            , mbufToFill
            , c4RetLen
            , kCIDLib::False
            , colInHdrLines
        );

        // If an HTTP error, make sure the length is zero
        if (c4Res != kCIDNet::c4HTTPStatus_OK)
        {
            if (m_eVerbose > tCQCKit::EVerboseLvls::Low)
            {
                TString strReason(L"HTTP code ");
                strReason.AppendFormatted(c4Res);

                facPlexRepoS().LogMsg
                (
                    CID_FILE
                    ,CID_LINE
                    , kPlexMsgs::midStatus_ImgFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , strPath
                    , strReason
                );
            }
            c4RetLen = 0;
        }
    }

    catch(TError& errToCatch)
    {
        if (m_eVerbose> tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facPlexRepoS().LogMsg
            (
                CID_FILE
                ,CID_LINE
                , kPlexErrs::errcPlex_ImgDownload
                , strPath
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );

            c4RetLen = 0;
        }
    }

    return c4RetLen;
}





// ---------------------------------------------------------------------------
//   CLASS: PlexRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  PlexRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TPlexRepoSDriver::TPlexRepoSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, &m_srdbEngine)
    , m_c4FldId_DBSerialNum(kCIDLib::c4MaxCard)
    , m_c4FldId_LoadStatus(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_pdblRepo(0)
    , m_srdbEngine(cqcdcToLoad.strMoniker())
{
    // Preload the 'loading' text for the status field
    m_strLoadMsg.LoadFromMsg(kPlexMsgs::midStatus_Loading, facPlexRepoS());
}

TPlexRepoSDriver::~TPlexRepoSDriver()
{
}


// ---------------------------------------------------------------------------
//  TPlexRepoSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't keep a persistent connection around. We only need it during the
//  DB loading process. So just say we are happy.
//
tCIDLib::TBoolean TPlexRepoSDriver::bGetCommResource(TThread&)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TPlexRepoSDriver::bWaitConfig(TThread&)
{
    //
    //  We asynchronously load the database. So we will make multiple
    //  passes into here. The first time, we try to star the loading
    //  process, and on subsequent entries, we check the status of the
    //  load.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_pdblRepo)
    {
        bRet = bMonitorLoad();
    }
     else
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_StartingLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_ipepServer
            );
        }

        //
        //  Create a loader and start it. Set our verbosity level on it
        //  so that it will do the appropriate logging.
        //
        m_pdblRepo = new TPlexRepoLoader(strMoniker(), m_ipepServer);
        m_pdblRepo->eVerboseLevel(eVerboseLevel());
        m_pdblRepo->StartLoad(&m_mdbLoad);
    }
    return bRet;
}


// Handle a write to one of our boolean fields
tCQCKit::ECommResults
TPlexRepoSDriver::eBoolFldValChanged(const  TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ReloadDB)
    {
        //
        //  Just call bWaitConfig() which already does what we want. But see if
        //  we alreayd have a load going on and don't bother in that case.
        //
        if (!m_pdblRepo)
        {
            bWaitConfig(*TThread::pthrCaller());
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  We don't have a device to connect to, but we can do a simple ping to make
//  sure we can connect, in order to make the driver states work more correctly.
//
//  We'll just hit the root URL.
//
tCQCKit::ECommResults TPlexRepoSDriver::eConnectToDevice(TThread& thrCaller)
{


    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TPlexRepoSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
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
            , TCQCIPConnCfg::clsThis()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Store the IP end point for later use
    m_ipepServer = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal()).ipepConn();

    //
    //  Store it on our database engine, since it's needed in our extended version
    //  of it, in order to download images.
    //
    m_srdbEngine.SetServerEP(m_ipepServer);

    // And give it our initial verbosity level
    m_srdbEngine.SetVerbosity(eVerboseLevel());

    //
    //  Set the reconnect and poll times to 1 second. We use this to both
    //  watch the DB loader during loads, and to periodically ping the server
    //  to make sure it's there.
    //
    SetPollTimes(1000, 15000);

    // And let's set up the fields
    TVector<TCQCFldDef> colFlds(16);
    TCQCFldDef flddNew;

    TString strFldDBSerialNum(L"MREPO#DBSerialNum");
    TString strFldReloadDB(L"MREPO#ReloadDB");
    TString strFldLoadStatus(L"MREPO#LoadStatus");
    TString strFldStatus(L"MREPO#Status");
    TString strFldTitleCnt(L"MREPO#TitleCount");

    flddNew.Set
    (
        strFldDBSerialNum, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
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
        strFldReloadDB, tCQCKit::EFldTypes::Boolean, tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldStatus, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        strFldTitleCnt, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read
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

    //
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , facPlexRepoS().strMsg(kPlexMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Initializing", kCIDLib::True
    );

    // Update the status to indicate we are starting to load
    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Loading", kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TPlexRepoSDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  If a database loader is active, then we are loading right now, so
    //  call the load monitoring method.
    //
    if (m_pdblRepo)
        bMonitorLoad();
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TPlexRepoSDriver::ReleaseCommResource()
{
    // Nothing to do
}


tCIDLib::TVoid TPlexRepoSDriver::TerminateImpl()
{
    // If a load is going on, then stop it
    if (m_pdblRepo)
    {
        m_pdblRepo->Shutdown();

        try
        {
            delete m_pdblRepo;
            m_pdblRepo = 0;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }

    // Tell our engine to flush the media database
    m_srdbEngine.ResetMediaDB();
}


tCIDLib::TVoid
TPlexRepoSDriver::VerboseModeChanged(const tCQCKit::EVerboseLvls eNewState)
{
    // Just keep our database engine updated so that he can log
    m_srdbEngine.SetVerbosity(eNewState);
}


// ---------------------------------------------------------------------------
//  TPlexRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TPlexRepoSDriver::bMonitorLoad()
{
    // We have a load going, so let's check the status
    tCIDLib::TCard4 c4SoFar;
    const tCQCMedia::ELoadStatus eStatus = m_pdblRepo->eStatus(c4SoFar);

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (eStatus > tCQCMedia::ELoadStatus::Loading)
    {
        try
        {
            // Make sure the repo thread is down and out
            m_pdblRepo->Shutdown();

            // If it failed, log that
            if (eStatus == tCQCMedia::ELoadStatus::Failed)
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcDB_LoadFailed
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , strMoniker()
                    );
                }
                bRet = kCIDLib::False;
            }
             else
            {
                // Give the loader a chance to wrap up
                m_pdblRepo->CompleteLoad();

                //
                //  Let our engine take all of the temp content. This will literally
                //  take all of the content, so it's pretty efficient and it will
                //  leave our temp one empty, so it's not wasting memory when not
                //  in use.
                //
                m_srdbEngine.ResetMediaDB(m_mdbLoad);

                //
                //  Set the status field to loaded and the loaded field to true, and
                //  set the title count field to the count of titles we loaded.
                //
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , facPlexRepoS().strMsg(kPlexMsgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreStringFld(m_c4FldId_LoadStatus, L"Ready", kCIDLib::True);

                bStoreCardFld
                (
                    m_c4FldId_TitleCnt
                    , m_srdbEngine.mdbInfo().c4TitleSetCnt(tCQCMedia::EMediaTypes::Movie)
                    , kCIDLib::True
                );

                // Store the DB serial number on our parent class
                const TString& strDBSerNum = m_pdblRepo->strDBSerialNum();
                SetDBSerialNum(strDBSerNum);

                // And set the serial number field
                bStoreStringFld(m_c4FldId_DBSerialNum, strDBSerNum, kCIDLib::True);

                bRet = kCIDLib::True;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            delete m_pdblRepo;
            m_pdblRepo = 0;
            throw;
        }

        delete m_pdblRepo;
        m_pdblRepo = 0;
    }
     else
    {
        // Not done yet, so keep waiting, but update the status field
        TString strSet(m_strLoadMsg);
        strSet.eReplaceToken(c4SoFar, kCIDLib::chDigit1);

        bStoreStringFld(m_c4FldId_Status, strSet, kCIDLib::True);
    }
    return bRet;
}


