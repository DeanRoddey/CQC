//
// FILE NAME: DVDProfilerS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/15/2005
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
#include    "DVDProfilerS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TDVDPMediaRepoEng,TCQCStdMediaRepoEng)
RTTIDecls(TDVDProfilerSDriver,TCQCStdMediaRepoDrv)



// ---------------------------------------------------------------------------
//  CLASS: TDVDPMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDVDPMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TDVDPMediaRepoEng::TDVDPMediaRepoEng(const TString& strMoniker) :

    TCQCStdMediaRepoEng(strMoniker, tCQCMedia::EMTFlags::Movie)
{
}

TDVDPMediaRepoEng::~TDVDPMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TDVDPMediaRepoEng: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called if an image needs to be loaded. DVD Profiler provides large and
//  thumb art, so we can return them separately.
//
//  We let exceptions propogate back.
//
tCIDLib::TCard4
TDVDPMediaRepoEng::c4LoadRawCoverArt(const  TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        const TString& strPath = mimgToLoad.strArtPath(eType);

        // See if it exists
        if (!strPath.bIsEmpty() && TFileSys::bExists(strPath))
        {
            //
            //  Apparently not so load the file. Keep in mind that we have to
            //  use the large art path here no matter what, since we don't
            //  support the small directly.
            //
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

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
    return c4Ret;
}





// ---------------------------------------------------------------------------
//   CLASS: DVDProfilerSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  DVDProfilerSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TDVDProfilerSDriver::TDVDProfilerSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, &m_srdbEngine)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_pdblRepo(0)
    , m_srdbEngine(cqcdcToLoad.strMoniker())
{
    // Preload the 'loading' text for the status field
    m_strLoadMsg.LoadFromMsg(kDVDPrMsgs::midStatus_Loading, facDVDProfilerS());
}

TDVDProfilerSDriver::~TDVDProfilerSDriver()
{
}


// ---------------------------------------------------------------------------
//  TDVDProfilerSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TDVDProfilerSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource
    return kCIDLib::True;
}


tCIDLib::TBoolean TDVDProfilerSDriver::bWaitConfig(TThread&)
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
                , m_strXMLFile
            );
        }

        //
        //  We've not got an outstanding load, so see if the repository dir
        //  is present. If not, we just keep saying we haven't gotten config
        //  data.
        //
        if (TFileSys::bExists(m_strXMLFile, tCIDLib::EDirSearchFlags::AllFiles))
        {
            //
            //
            //  Create a loader and start it. Set our verbosity level on it
            //  so that it will do the appropriate logging.
            //
            m_pdblRepo = new TDVDProfilerLoader(strMoniker());
            m_pdblRepo->eVerboseLevel(eVerboseLevel());
            m_pdblRepo->StartLoad(&m_mdbLoad, m_strXMLFile, m_strImagePath);
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
                , m_strXMLFile
            );
        }
    }
    return bRet;
}


// Handles a write to one of our boolean fields
tCQCKit::ECommResults
TDVDProfilerSDriver::eBoolFldValChanged(const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ReloadDB)
    {
        // If not already loading, then do a load
        if (!m_pdblRepo)
        {
            // If the file doesn't exist, then reject the operation
            if (!TFileSys::bExists(m_strXMLFile, tCIDLib::EDirSearchFlags::AllFiles))
                return tCQCKit::ECommResults::ResNotFound;

            // Set the status field to show not loaded
            bStoreStringFld
            (
                m_c4FldId_Status
                , facDVDProfilerS().strMsg(kDVDPrMsgs::midStatus_NotLoaded)
                , kCIDLib::True
            );

            // Create a loader and start it
            m_pdblRepo = new TDVDProfilerLoader(strMoniker());
            m_pdblRepo->eVerboseLevel(eVerboseLevel());
            m_pdblRepo->StartLoad(&m_mdbLoad, m_strXMLFile, m_strImagePath);
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


// We don't really have any device toconnect to, so it's a no-op
tCQCKit::ECommResults TDVDProfilerSDriver::eConnectToDevice(TThread& thrCaller)
{
    return tCQCKit::ECommResults::Success;
}


// Handle our initialization callback
tCQCKit::EDrvInitRes TDVDProfilerSDriver::eInitializeImpl()
{
    //
    //  Set the reconnect and poll times to 1 second. We don't poll any
    //  device or have any device to connect to, but we do watch for the
    //  DB loader to finish and want to respond to that quickly.
    //
    SetPollTimes(1000, 1000);

    // We should have gotten a prompt value with the file to load
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    if (!cqcdcTmp.bFindPromptValue(L"LibFile", L"File", m_strXMLFile))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facDVDProfilerS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The exported library file name prompt was not set"
                , tCIDLib::ESeverities::Status
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    if (!cqcdcTmp.bFindPromptValue(L"ImagePath", L"Path", m_strImagePath))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facDVDProfilerS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The image path prompt was not set"
                , tCIDLib::ESeverities::Status
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  They only give us one image path, for historical reasons, because
    //  we originally only supported one type of image. So look at the
    //  path and, if they pointed us at the thumbnails directory, we'll
    //  move up to the main images path.
    //
    TPathStr pathImgs(m_strImagePath);
    pathImgs.bRemoveTrailingSeparator();
    if (pathImgs.bEndsWith(L"THUMBNAILS"))
    {
        pathImgs.bRemoveLevel();
        m_strImagePath = pathImgs;
    }

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    //  We do different stuff depending on if we are in V1 or V2 mode.
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
        , facDVDProfilerS().strMsg(kDVDPrMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    bStoreStringFld
    (
        m_c4FldId_LoadStatus, L"Loading", kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


// We don't really poll, but we do have to watch for loads to complete
tCQCKit::ECommResults TDVDProfilerSDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  If a database loader is active, then we are loading right now, so
    //  call the load monitoring method.
    //
    if (m_pdblRepo)
        bMonitorLoad();
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TDVDProfilerSDriver::ReleaseCommResource()
{
    // Nothing to release
}


tCIDLib::TVoid TDVDProfilerSDriver::TerminateImpl()
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

    // Tell the repo engine to flush the media database
    m_srdbEngine.ResetMediaDB();
}


// ---------------------------------------------------------------------------
//  TDVDProfilerSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TDVDProfilerSDriver::bMonitorLoad()
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
                // Set the status field to not loaded and leave loaded field false
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , facDVDProfilerS().strMsg(kDVDPrMsgs::midStatus_NotLoaded)
                    , kCIDLib::True
                );

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
                //  Set the status field to loaded and the loaded field to
                //  true, and set the title count field to the count of
                //  titles we loaded.
                //
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , facDVDProfilerS().strMsg(kDVDPrMsgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreStringFld(m_c4FldId_LoadStatus, L"Ready", kCIDLib::True);

                if (c4ArchVersion() == 1)
                    bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);

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

