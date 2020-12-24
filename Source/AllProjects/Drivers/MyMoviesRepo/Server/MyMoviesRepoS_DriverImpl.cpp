//
// FILE NAME: MyMoviesRepoS_DriverImpl.cpp
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
#include    "MyMoviesRepoS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TMMMediaRepoEng,TCQCStdMediaRepoEng)
RTTIDecls(TMyMoviesRepoSDriver,TCQCStdMediaRepoDrv)



// ---------------------------------------------------------------------------
//  CLASS: TMMMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMMMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------

// Pass on the moniker and the media types we support to our bass class
TMMMediaRepoEng::TMMMediaRepoEng(const TString& strMoniker) :

    TCQCStdMediaRepoEng(strMoniker, tCQCMedia::EMTFlags::Movie)
{
}

TMMMediaRepoEng::~TMMMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TMMMediaRepoEng: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called if an image needs to be loaded. In pre-5.0 versions of MM, the
//  images were separate and there were thumbs and large art, which we loaded
//  separately. If this is an older version, we'll have both loaded in almost all
//  cases. If 5.0 or later we just get the large art, and will have to gen up a
//  thumb.
//
//  We let exceptions propogate back.
//
tCIDLib::TCard4
TMMMediaRepoEng::c4LoadRawCoverArt( const   TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType)
{

    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        //
        //  If we have already loaded what teh caller wants, then we shouldn't get
        //  called, but return it anyway.
        //
        tCIDLib::TBoolean bScaleDown = kCIDLib::False;
        c4Ret = mimgToLoad.c4QueryArt(mbufToFill, eType, 0);

        if (!c4Ret)
        {
            //
            //  Not loaded yet, so let's try to load it. If they asked for the small
            //  art, see if we have a separate small art image. If so, we can get that
            //  directly. Else, we have to load the large art and scale it.
            //
            TString strPath = mimgToLoad.strArtPath(eType);
            if (strPath.bIsEmpty() && (eType == tCQCMedia::ERArtTypes::SmlCover))
            {
                strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::LrgCover);
                bScaleDown = kCIDLib::True;
            }

            // If we got something and the file exists, let's load it
            if (!strPath.bIsEmpty() && TFileSys::bExists(strPath))
            {
                TBinaryFile flArt(strPath);
                flArt.Open
                (
                    tCIDLib::EAccessModes::Read
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                c4Ret = tCIDLib::TCard4(flArt.c8CurSize());
                flArt.c4ReadBuffer(mbufToFill, c4Ret, tCIDLib::EAllData::FailIfNotAll);
            }
        }

        if (c4Ret && bScaleDown)
        {
            //
            //  If we are returning anything, see if we are doing the scaling down of
            //  large art.
            //
            c4Ret = facCIDImgFact().c4CompactToJPEG
            (
                mbufToFill, c4Ret, TSize(kCQCMedia::c4DefThumbSz)
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
//   CLASS: MyMoviesRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  MyMoviesRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TMyMoviesRepoSDriver::TMyMoviesRepoSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, &m_srdbEngine)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_pdblRepo(0)
    , m_srdbEngine(cqcdcToLoad.strMoniker())
    , m_strPassword(L"7eBrABud")
    , m_strUserName(L"mymovies")
{
    // Preload the 'loading' text for the status field
    m_strLoadMsg.LoadFromMsg(kMyMvMsgs::midStatus_Loading, facMyMoviesRepoS());
}

TMyMoviesRepoSDriver::~TMyMoviesRepoSDriver()
{
}


// ---------------------------------------------------------------------------
//  TMyMoviesRepoSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMyMoviesRepoSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource
    return kCIDLib::True;
}


tCIDLib::TBoolean TMyMoviesRepoSDriver::bWaitConfig(TThread&)
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
                , m_strODBCSrc
            );
        }

        //
        //  We've not got an outstanding load, so see if we can open the
        //  database before we bother kicking off the load.
        //
        if (bSrcExists())
        {
            //
            //  Create a loader and start it. Set our verbosity level on it
            //  so that it will do the appropriate logging.
            //
            m_pdblRepo = new TMyMoviesRepoLoader
            (
                strMoniker(), m_strODBCSrc, m_strUserName, m_strPassword, m_strImagePath
            );

            m_pdblRepo->eVerboseLevel(eVerboseLevel());
            m_pdblRepo->StartLoad(&m_mdbLoad);
        }
         else if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_RepoNotFound
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strODBCSrc
            );
        }
    }
    return bRet;
}


// Handle a write to one of our boolean fields
tCQCKit::ECommResults
TMyMoviesRepoSDriver::eBoolFldValChanged(const  TString&            strName
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


tCQCKit::ECommResults TMyMoviesRepoSDriver::eConnectToDevice(TThread& thrCaller)
{
    // We don't have a device to connect to
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TMyMoviesRepoSDriver::eInitializeImpl()
{
    //
    //  Set the reconnect and poll times to 1 second. We don't poll any
    //  device or have any device to connect to, but we do watch for the
    //  DB loader to finish and want to respond to that quickly.
    //
    SetPollTimes(1000, 15000);

    // We should have gotten a prompt value with the DSN to use
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    if (!cqcdcTmp.bFindPromptValue(L"DBSrc", L"SrcName", m_strODBCSrc))
    {
        facMyMoviesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The ODBC source prompt was not set for driver %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // And another with the path to the images
    if (!cqcdcTmp.bFindPromptValue(L"ImagePath", L"Path", m_strImagePath))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facMyMoviesRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The image path prompt was not set"
                , tCIDLib::ESeverities::Status
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    // And let's set up the fields
    TVector<TCQCFldDef> colFlds(16);
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
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , facMyMoviesRepoS().strMsg(kMyMvMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Initializing", kCIDLib::True
    );

    // Initialize ODBC support if not already
    facCIDDBase().Initialize();

    // Update the status to indicate we are starting to load
    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Loading", kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TMyMoviesRepoSDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  If a database loader is active, then we are loading right now, so
    //  call the load monitoring method.
    //
    if (m_pdblRepo)
        bMonitorLoad();
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TMyMoviesRepoSDriver::ReleaseCommResource()
{
    // Nothing to release
}


tCIDLib::TVoid TMyMoviesRepoSDriver::TerminateImpl()
{
    // If a load is going on, then stop it
    if (m_pdblRepo)
        m_pdblRepo->Shutdown();

    try
    {
        delete m_pdblRepo;
        m_pdblRepo = 0;
    }

    catch(...)
    {
    }

    // Tell our engine to flush the media database
    m_srdbEngine.ResetMediaDB();
}


// ---------------------------------------------------------------------------
//  TMyMoviesRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TMyMoviesRepoSDriver::bMonitorLoad()
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
                    , facMyMoviesRepoS().strMsg(kMyMvMsgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreStringFld(m_c4FldId_LoadStatus, L"Ready", kCIDLib::True);

                bStoreCardFld
                (
                    m_c4FldId_TitleCnt
                    , m_srdbEngine.mdbInfo().c4TitleSetCnt(tCQCMedia::EMediaTypes::Movie)
                    , kCIDLib::True
                );

                // If V1, update the old boolean loaded field
                if (c4ArchVersion() == 1)
                    bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);

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


//
//  Checks to see if the provided ODBC source file exists, before we bother
//  kicking off the loader.
//
tCIDLib::TBoolean TMyMoviesRepoSDriver::bSrcExists()
{
    try
    {
        TDBConnection dbconTest;
        dbconTest.Connect(m_strODBCSrc, m_strUserName, m_strPassword, kCIDLib::False);
        dbconTest.Disconnect();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMyMvErrs::errcDB_DBConnFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strODBCSrc
                , strMoniker()
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

