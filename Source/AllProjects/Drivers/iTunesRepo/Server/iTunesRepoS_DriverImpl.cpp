//
// FILE NAME: iTunesRepoS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/06/2007
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
#include    "iTunesRepoS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRepoSDriver,TCQCStdMediaRepoDrv)



// ---------------------------------------------------------------------------
//   CLASS: iTunesRepoSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  iTunesRepoSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesRepoSDriver::TiTunesRepoSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, tCQCMedia::EMTFlags::Music)
    , m_bUseAlbumArtist(kCIDLib::False)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_TitlesDone(kCIDLib::c4MaxCard)
    , m_c4FldId_TracksIgnored(kCIDLib::c4MaxCard)
    , m_pdblRepo(0)
{
    // Preload the 'loading' text for the status field
    m_strLoadMsg.LoadFromMsg(kiTunesMsgs::midStatus_Loading, faciTunesRepoS());
}

TiTunesRepoSDriver::~TiTunesRepoSDriver()
{
}


// ---------------------------------------------------------------------------
//  TiTunesRepoSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TiTunesRepoSDriver::bGetCommResource(TThread&)
{
    // We have no comm resource
    return kCIDLib::True;
}


tCIDLib::TBoolean TiTunesRepoSDriver::bWaitConfig(TThread&)
{
    //
    //  We asynchronously load the database. So we will make multiple
    //  passes into here. The first time, we try to start the loading
    //  process, and on subsequent entries, we check the status of the
    //  load.
    //
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_pdblRepo)
    {
        try
        {
            bRet = bMonitorLoad();
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
                    , L"An exception proporated out of the load monitor"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
        }

        catch(...)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"An unknown exception proporated out of the load monitor"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
        }
    }
     else
    {
        // No outstanding load operation, so try one
        StartLoad();
    }
    return bRet;
}


//
//  Our base class calls us back on this if it needs to load up cover art
//  into the in-memory cache. In our case, we pre-load all the data from
//  embedded artwork during the database load. So we should never get
//  called here for those. We'll only get called for external .itc files
//  that have been loaded yet.
//
tCIDLib::TCard4
TiTunesRepoSDriver::c4LoadCoverArt( const   TMediaImg&          mimgToLoad
                                    ,       TMemBuf&            mbufToFill
                                    , const tCIDLib::TBoolean   bLarge)
{
    //
    //  If the file doesn't exist, then see if were able to cach up an
    //  embedded image during loading. If so, return that, else zero.
    //
    tCIDLib::TCard4 c4RetBytes = 0;
    if (!TFileSys::bExists(mimgToLoad.strArtPath()))
    {
        if (mimgToLoad.c4Size())
        {
            c4RetBytes = mimgToLoad.c4Size();
            mbufToFill.CopyIn(mimgToLoad.mbufArt(), c4RetBytes);
        }
        return c4RetBytes;
    }

    try
    {
        //
        //  Ok, we need to load it. Read it first into a buffer via a
        //  binary file
        //
        TBinaryFile flArt(mimgToLoad.strArtPath());
        flArt.Open
        (
            tCIDLib::EAccessModes::Multi_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        c4RetBytes = tCIDLib::TCard4(flArt.c8CurSize());

        //
        //  Search for the start of the actual image data. We get the
        //  first 32 bit value, which indicates the length of section
        //  1.
        //
        tCIDLib::TCard4 c4Val;
        flArt.c4ReadBuffer(&c4Val, 4);
        #if defined(CIDLIB_LITTLEENDIAN)
        c4Val = TRawBits::c4SwapBytes(c4Val);
        #endif

        // Seek forward that much plus another 4
        flArt.c8OffsetFilePos(c4Val + 4);

        // Get the size of part two and skip it, minus 12
        flArt.c4ReadBuffer(&c4Val, 4);
        #if defined(CIDLIB_LITTLEENDIAN)
        c4Val = TRawBits::c4SwapBytes(c4Val);
        #endif
        flArt.c8OffsetFilePos(c4Val - 12);

        // This should be the image data
        c4RetBytes -= tCIDLib::TCard4(flArt.c8CurPos());
        flArt.c4ReadBuffer(mbufToFill, c4RetBytes);

        //
        //  Convert it to the appropriate type of image.
        //
        const tCIDImage::EImgTypes eType = facCIDImgFact().eProbeImg(mbufToFill, c4RetBytes);

        // If it's not a legal image type, then return zero bytes
        if (eType == tCIDImage::EImgTypes::Unknown)
            c4RetBytes = 0;

        //
        //  If they asked for the small image, then we need to scale this
        //  one. Our parent class provides a helper to do this for us.
        //
        if (!bLarge)
            c4RetBytes = c4DefScaleThumb(mbufToFill, c4RetBytes);
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
                , kMedErrs::errcDB_CantLoadArt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , mimgToLoad.strArtPath()
            );
        }
        return 0;
    }
    return c4RetBytes;
}


tCQCKit::ECommResults
TiTunesRepoSDriver::eBoolFldValChanged( const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ReloadDB)
    {
        // If not already loading, then do a load
        if (!m_pdblRepo)
            StartLoad();
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TiTunesRepoSDriver::eConnectToDevice(TThread& thrCaller)
{
    // We don't have a device to connect to
    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TiTunesRepoSDriver::eInitializeImpl()
{
    //
    //  Set the reconnect and poll times to 1 second. We don't poll any
    //  device or have any device to connect to, but we do watch for the
    //  DB loader to finish and want to respond to that quickly.
    //
    SetPollTimes(1000, 1000);

    // We should have gotten a prompt value with the iTunes path
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    if (!cqcdcTmp.bFindPromptValue("iTunesPath", L"Path", m_strITunesPath))
    {
        faciTunesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The iTunes data path prompt was not set for driver %(1)"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  And we added one for forcing the use of the album artist. This one
    //  might not be present, so default it if needed.
    //
    TString strTmp;
    if (cqcdcTmp.bFindPromptValue("UseAlbumArtist", L"State", strTmp))
        m_bUseAlbumArtist = (strTmp == kCQCKit::pszFld_True);
    else
        m_bUseAlbumArtist = kCIDLib::False;

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef flddNew;

    const tCIDLib::TCh* const pszFldLoaded(L"Loaded");
    const tCIDLib::TCh* const pszFldReloadDB(L"ReloadDB");
    const tCIDLib::TCh* const pszFldStatus(L"Status");
    const tCIDLib::TCh* const pszFldTitlesDone(L"TitleCount");
    const tCIDLib::TCh* const pszFldTracksIgnored(L"TracksIgnored");

    flddNew.Set
    (
        pszFldLoaded
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldReloadDB
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldStatus
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldTitlesDone
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldTracksIgnored
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);
    m_c4FldId_Loaded = pflddFind(pszFldLoaded, kCIDLib::True)->c4Id();
    m_c4FldId_ReloadDB = pflddFind(pszFldReloadDB, kCIDLib::True)->c4Id();
    m_c4FldId_Status = pflddFind(pszFldStatus, kCIDLib::True)->c4Id();
    m_c4FldId_TitlesDone = pflddFind(pszFldTitlesDone, kCIDLib::True)->c4Id();
    m_c4FldId_TracksIgnored = pflddFind(pszFldTracksIgnored, kCIDLib::True)->c4Id();

    //
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , faciTunesRepoS().strMsg(kiTunesMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    // We'll try to get a first load of the database before we come online
    return tCQCKit::EDrvInitRes::WaitConfig;
}


tCQCKit::ECommResults TiTunesRepoSDriver::ePollDevice(TThread& thrCaller)
{
    //
    //  If a database loader is active, then we are loading right now, so
    //  call the load monitoring method.
    //
    if (m_pdblRepo)
        bMonitorLoad();
    return tCQCKit::ECommResults::Success;
}



tCIDLib::TVoid TiTunesRepoSDriver::ReleaseCommResource()
{
    // Nothing to release
}


tCIDLib::TVoid TiTunesRepoSDriver::TerminateImpl()
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

    // Tell the base class to flush the media database
    ResetMediaDB();
}


// ---------------------------------------------------------------------------
//  TiTunesRepoSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TiTunesRepoSDriver::bMonitorLoad()
{
    // We have a load going, so let's check the status
    tCIDLib::TCard4 c4TitlesDone;
    tCIDLib::TCard4 c4TracksIgnored;
    const tCQCMedia::ELoadStatus eStatus = m_pdblRepo->eStatus
    (
        c4TitlesDone, c4TracksIgnored
    );

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
                    , faciTunesRepoS().strMsg(kiTunesMsgs::midStatus_NotLoaded)
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
                //
                //  Set the status field to loaded and the loaded field to
                //  true, and set the count fields.
                //
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , faciTunesRepoS().strMsg(kiTunesMsgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);
                bStoreCardFld
                (
                    m_c4FldId_TitlesDone, c4TitlesDone, kCIDLib::True
                );
                bStoreCardFld
                (
                    m_c4FldId_TracksIgnored, c4TracksIgnored, kCIDLib::True
                );

                // Give the loader a chance to wrap up
                m_pdblRepo->CompleteLoad();
                bRet = kCIDLib::True;
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            // Tell the parent class we are done loading
            bLoading(kCIDLib::False);
            delete m_pdblRepo;
            m_pdblRepo = 0;
            throw;
        }

        // Tell the parent class we are done loading
        bLoading(kCIDLib::False);
        delete m_pdblRepo;
        m_pdblRepo = 0;
    }
     else
    {
        // Not done yet, so keep waiting, but update the status fields
        TString strSet(m_strLoadMsg);
        strSet.eReplaceToken(c4TitlesDone, kCIDLib::chDigit1);

        bStoreStringFld(m_c4FldId_Status, strSet, kCIDLib::True);
        bStoreCardFld
        (
            m_c4FldId_TitlesDone, c4TitlesDone, kCIDLib::True
        );
        bStoreCardFld
        (
            m_c4FldId_TracksIgnored, c4TracksIgnored, kCIDLib::True
        );
    }
    return bRet;
}


tCIDLib::TVoid TiTunesRepoSDriver::StartLoad()
{
    //
    //  The XML file comes in two different forms, so check for both. Check
    //  for the shorter version first, because it shows up on a move of the
    //  iTunes directory, but the old one stays there also. If its not moved,
    //  then there should only be the longer form.
    //
    TPathStr pathXMLFile = m_strITunesPath;
    pathXMLFile.AddLevel(L"iTunes Library.xml");

    tCIDLib::TBoolean bSrcFound;
    bSrcFound = TFileSys::bExists(pathXMLFile, tCIDLib::EDirSearchFlags::AllFiles);
    if (!bSrcFound)
    {
        // Not that one, so try the other one
        pathXMLFile = m_strITunesPath;
        pathXMLFile.AddLevel(L"iTunes Music Library.xml");
        bSrcFound = TFileSys::bExists(pathXMLFile, tCIDLib::EDirSearchFlags::AllFiles);
    }

    // If we found one, then try start a load
    if (bSrcFound)
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
                , pathXMLFile
            );
        }

        //
        //  Create a loader. Set our verbosity level on it so that it will
        //  do the appropriate logging. Once we get the new loader set,
        //  then that's the signal that we are in loading mode, so we can
        //  go ahead and set up other stuff before starting the loading
        //  process, sine we know we'll get back out of loading mode even
        //  if we were to fail before we finish this process, since the
        //  poll callback watchs this member, and the initial state is
        //  set to error status until we start it up.
        //
        m_pdblRepo = new TiTunesRepoLoader
        (
            strMoniker(), m_strITunesPath, m_bUseAlbumArtist
        );
        m_pdblRepo->eVerboseLevel(eVerboseLevel());

        //
        //  Set the loading flag on the parent class, so that he can
        //  reject incoming requests at this point, since the loading
        //  process will be updating the database.
        //
        bLoading(kCIDLib::True);

        //
        //  Make sure that our fields that represent the load state are
        //  set to their default values since if we are waiting for config,
        //  we can't have any titles loaded yet.
        //
        {
            bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);
            bStoreCardFld(m_c4FldId_TitlesDone, 0, kCIDLib::True);
            bStoreStringFld
            (
                m_c4FldId_Status
                , faciTunesRepoS().strMsg(kiTunesMsgs::midStatus_NotLoaded)
                , kCIDLib::True
            );
        }

        // And finally start the loading process
        m_pdblRepo->StartLoad(&mdbInfo(), pathXMLFile);
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
            , pathXMLFile
        );
    }
}


