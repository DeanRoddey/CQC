//
// FILE NAME: FileTagRepoS_DBLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2007
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
#include    "FileTagRepoS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(FileTagRepoLoader,TThread)



// ---------------------------------------------------------------------------
//   CLASS: FileTagRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  FileTagRepoLoader: Public, static methods
// ---------------------------------------------------------------------------

//
//  A key extractor function so that we can use a keyed hash set for our
//  playlist list. The name of the playlist is stored in the 0th slot of
//  vector that makes up each playlist.
//
const TString& FileTagRepoLoader::strPLKeyExtract(const TPathList& colPL)
{
    return colPL[0];
}


// ---------------------------------------------------------------------------
//  FileTagRepoLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
FileTagRepoLoader::FileTagRepoLoader(const  TString&            strMoniker
                                    , const tCIDLib::TBoolean   bLoadPlaylists
                                    , const tCIDLib::TBoolean   bUseEmbeddedArt) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_bLoadPlaylists(bLoadPlaylists)
    , m_bUseEmbeddedArt(bUseEmbeddedArt)
    , m_c4ArtSize(0)
    , m_c4TitlesDone(0)
    , m_colArtists(256)
    , m_colFiles(256)
    , m_colGenres(256)
    , m_colPlayLists(109, TStringKeyOps(), &strPLKeyExtract)
    , m_colTracks(256)
    , m_eMediaType(tCQCMedia::EMediaTypes::Music)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_fcolBitRates(32)
    , m_fcolTimes(32)
    , m_fcolTrackNums(32)
    , m_fcolYears(32)
    , m_mbufArt(100 * 1024)
    , m_mbufTmp(64 * 1024, 0x1000000)
    , m_pmdbLoading(nullptr)
    , m_strMoniker(strMoniker)
{
}

FileTagRepoLoader::~FileTagRepoLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid FileTagRepoLoader::CompleteLoad()
{
    try
    {
        // Make sure we are in the loaded or failed states
        if ((m_eStatus != tCQCMedia::ELoadStatus::Ready)
        &&  (m_eStatus != tCQCMedia::ELoadStatus::Failed))
        {
            // Log a message that we are already loading and return
            facCQCMedia().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_LoadNotDone
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::NotReady
                , m_strMoniker
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  Note that both the values we return here are fundamental values and we
//  don't need to synchronize just to return these.
//
tCQCMedia::ELoadStatus
FileTagRepoLoader::eStatus(tCIDLib::TCard4& c4TitlesDone) const
{
    // Return the status and the count of titles processed
    c4TitlesDone = m_c4TitlesDone;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
FileTagRepoLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


const TString& FileTagRepoLoader::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


//
//  Starts the database load process by storing the passed database to laod
//  and the path to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid
FileTagRepoLoader::StartLoad(TMediaDB* const pmdbToLoad, TString& strRepoPath)
{
    // Make sure we are not already loading
    if (m_eStatus == tCQCMedia::ELoadStatus::Loading)
    {
        // Log a message that we are already loading and return
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcDB_AlreadyLoading
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
            , m_strMoniker
        );
        return;
    }

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedMsgs::midStatus_StartingLoad
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strRepoPath
        );
    }

    // Store the new database to load and the file name
    m_pmdbLoading = pmdbToLoad;
    m_strRepoPath = strRepoPath;

    // Reset any of our per-load data
    m_c4TitlesDone = 0;

    // Flush the media database that we are going to load
    m_pmdbLoading->Reset();

    // Add an 'All Music' category that all stuff can be put into
    m_strTmp1 = facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic);
    m_strTmp2 = m_strTmp1;
    m_strTmp2.ToUpper();
    TMediaCat* pmcatAll = new TMediaCat(m_eMediaType, m_strTmp1, m_strTmp2);
    pmcatAll->c2Id(kCQCMedia::c2CatId_AllMusic);
    m_pmdbLoading->c2AddCategory(pmcatAll, kCIDLib::True);

    // And a playlists category as well
    m_strTmp1 = facCQCMedia().strMsg(kMedMsgs::midCat_PlayLists);
    m_strTmp2 = m_strTmp1;
    m_strTmp2.ToUpper();
    TMediaCat* pmcatPLs = new TMediaCat(m_eMediaType, m_strTmp1, m_strTmp2);
    pmcatPLs->c2Id(kCQCMedia::c2CatId_PlayLists);
    m_pmdbLoading->c2AddCategory(pmcatPLs, kCIDLib::True);

    // Set the status to indicate we are loading
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Start ourself running in another thread
    Start();

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_ThreadStarted
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  Called by the main thread to make sure we are cleaned up when it needs
//  to shutdown.
//
tCIDLib::TVoid FileTagRepoLoader::Shutdown()
{
    // If the thread is running, let's ask it to shut down
    try
    {
        ReqShutdownSync(8000);
        eWaitForDeath(3000);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    m_eStatus = tCQCMedia::ELoadStatus::Failed;
}


// ---------------------------------------------------------------------------
//  FileTagRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------


tCIDLib::EExitCodes FileTagRepoLoader::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        // Kick off the recursive scan of the directory
        ProcessDir(m_strRepoPath);

        // Do any post-processing we need, mostly finishing up playlist stuff
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_PostProcessing
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        PostProcessing();
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_PostComplete
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        //
        //  Ok, it's done successfully, so call the load completion method
        //  on the media database, so that it can do any stuff it has to wait
        //  until all the data is available to do.
        //
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_LoadingThreadDone
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        m_pmdbLoading->LoadComplete();

        //
        //  Generate a database serial number. The only way we can reasonably create
        //  one is to flatten the database and hash the resulting buffer.
        //
        {
            TChunkedBinOutStream strmTar(32 * (1024 * 1024));
            strmTar << *m_pmdbLoading << kCIDLib::FlushIt;

            TChunkedBinInStream strmSrc(strmTar);
            facCQCMedia().CreatePersistentId
            (
                strmSrc, strmTar.c4CurSize(), m_strDBSerialNum
            );
        }

        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_DBComplete
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        // And mark us as loaded
        m_eStatus = tCQCMedia::ELoadStatus::Ready;
    }

    catch(const tCQCMedia::ELoadStatus)
    {
        // We were asked to shutdown, so just fail it and return
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    catch(TError& errToCatch)
    {
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_ExceptInLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strRepoPath
            );
        }
    }

    catch(...)
    {
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_ExceptInLoad
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strRepoPath
            );
        }
    }

    //
    //  Make sure that the status didn't end up stuck on loading somehow,
    //  since that would make the driver wait forever.
    //
    if ((m_eStatus != tCQCMedia::ELoadStatus::Failed)
    &&  (m_eStatus != tCQCMedia::ELoadStatus::Ready))
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  FileTagRepoLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called at the end, to finish up anything that we couldn't do
//  until everything was completed.
//
tCIDLib::TVoid FileTagRepoLoader::PostProcessing()
{
    // If we have no playlists, then nothing to do
    if (m_colPlayLists.bIsEmpty())
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_NoPlaylists
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return;
    }

    TPlayLists::TCursor cursPLs(&m_colPlayLists);
    do
    {
        // Get the next playlist out
        const TPathList& colCur = cursPLs.objRCur();

        //
        //  Get the 0th element, which is the name. The hash set already
        //  insured that we have no dups. Create a title set for this
        //  one.
        //
        const TString& strPLName = colCur[0];

        // If it's really big or empty, skip it
        const tCIDLib::TCard4 c4ItemCnt = colCur.c4ElemCount();
        if ((c4ItemCnt> 8000) || (c4ItemCnt < 3))
        {
            if (c4ItemCnt && (m_eVerbose >= tCQCKit::EVerboseLvls::Medium))
            {
                facFileTagRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Playlist '%(1)' is too large or empty, skipped it"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strPLName
                );
            }
            continue;
        }

        // Looks reasonable, so let's do it. Create an id
        m_strTmp1 = L"PlayList_";
        m_strTmp1.Append(strPLName);

        //
        //  Make the name isn't a dup. If so, skip it. It shouldn't be but
        //  someone could name a real title what this one came out as.
        //
        if (m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strTmp1, kCIDLib::False))
            continue;

        // Create a new title set and add it to the database
        m_c4TitlesDone++;
        facCQCMedia().bMakeSortTitle(m_strTmp1, m_strTmp2);
        TMediaTitleSet* pmtsPL = new TMediaTitleSet
        (
            strPLName, m_strTmp1, m_strTmp2, m_eMediaType
        );
        TJanitor<TMediaTitleSet> janSet(pmtsPL);

        pmtsPL->strArtist(L"Various Artists");

        //
        //  Create a collection and add it to the title set and database.
        //  Set the artist to various artists, which it always will be
        //  for a playlist. Give it the same name, but create a new id
        //  for it by appending _1 to the title id. Give it the same numerical
        //  id since there is a one to one relationship between title sets
        //  and collections in this repository.
        //
        m_strTmp1.Append(L"_1");
        TMediaCollect* pmcolPL = new TMediaCollect
        (
            strPLName
            , m_strTmp1
            , 0
            , tCQCMedia::ELocTypes::FileItem
            , m_eMediaType
        );
        TJanitor<TMediaCollect> janCol(pmcolPL);

        pmcolPL->strArtist(L"Various Artists");
        pmcolPL->bIsPlayList(kCIDLib::True);

        //
        //  If it has a cover art file, then set that, for later faulting in.
        //
        const TString& strPLArt = colCur[1];
        if (!strPLArt.bIsEmpty())
        {
            TMediaImg* pmimgArt = new TMediaImg
            (
                m_eMediaType, strPLArt, TString::strEmpty()
            );
            m_pmdbLoading->c2AddImage(pmimgArt);
            pmcolPL->c2ArtId(pmimgArt->c2Id());
        }

        // Add this guy to the all music category and the playlists category
        pmcolPL->bAddCategory(kCQCMedia::c2CatId_AllMusic);
        pmcolPL->bAddCategory(kCQCMedia::c2CatId_PlayLists);

        //
        //  Now loop through all of the ids we stored for this playlist, look up
        //  the items and add their ids to the playlist collection. Since it's marked
        //  as a playlist, it won't own these items.
        //
        //  Start on 2 since 0 is the playlist name and 1 the art path.
        //
        const tCIDLib::TCard4 c4BaseIDLen = m_strTmp1.c4Length();
        tCIDLib::TCard4 c4ColDur = 0;
        for (tCIDLib::TCard4 c4PLIndex = 2; c4PLIndex < c4ItemCnt; c4PLIndex++)
        {
            const TString& strLoc = colCur[c4PLIndex];
            const TMediaItem* pmitemCur = m_pmdbLoading->pmitemByLocInfo
            (
                m_eMediaType, strLoc, kCIDLib::False
            );

            if (pmitemCur)
                pmcolPL->bAddItem(*pmitemCur);
        }

        // Store the stuff we acculuated from the items above
        pmcolPL->c4Duration(c4ColDur);

        //
        //  Add the collection now that it's set up, and then we can add it to the
        //  title set.
        //
        m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());
        pmtsPL->bAddCollect(*pmcolPL);

        // Add the title set now
        m_pmdbLoading->c2AddTitle(janSet.pobjOrphan());

    }   while(cursPLs.bNext());
}


//
//  Processes a single file. We add new per-item stuff to the per-item
//  lists, and update any title/collection level stuff we've not gotten
//  yet. We store per-track info into vectors that we expand as required
//  to hold the number of tracks, but we don't flush them over and over.
//  We use the passed track count to expand if needed, else we reuse.
//
tCIDLib::TVoid
FileTagRepoLoader::ProcessFile( const   TFindBuf&           fndbSearch
                                ,       tCIDLib::TCard4&    c4TrackCnt
                                ,       TCIDMetaExtractor&  metexToUse)
{
    //
    //  Get title/col level stuff into temps, and only get embedded art if we
    //  haven't already gotten some via another means and the user has configured
    //  this driver to use embedded art. If we don't get the embedded, later we will
    //  check for external cover art.
    //
    tCIDLib::TCard4 c4ArtSz;
    tCIDLib::TCard4 c4BitRate = 0;
    tCIDLib::TCard4 c4Duration = 0;
    tCIDLib::TCard4 c4Rating = 1;
    tCIDLib::TCard4 c4TrackNum;
    tCIDLib::TCard4 c4Year;
    const tCIDLib::TBoolean bRes = metexToUse.bExtract
    (
        fndbSearch.pathFileName()
        , m_strTmp1                     // Temp category
        , m_strTmp2                     // Temp Descr
        , m_strTmp3                     // Temp album
        , m_strTmp4                     // Temp album artist
        , m_strTmp5                     // Temp artist
        , m_strTmp6                     // Temp track name
        , c4TrackNum
        , c4Duration
        , c4Year
        , c4BitRate
        , c4Rating
        , (m_c4ArtSize == 0) && m_bUseEmbeddedArt
        , m_mbufTmp
        , c4ArtSz
    );

    if (bRes)
    {
        // Store any title/col level stuff we haven't gotten already
        if (m_strDescr.bIsEmpty() && !m_strTmp1.bIsEmpty())
            m_strDescr = m_strTmp2;
        if (m_strAlbum.bIsEmpty() && !m_strTmp3.bIsEmpty())
            m_strAlbum = m_strTmp3;
        if (m_strAlbumArtist.bIsEmpty() && !m_strTmp4.bIsEmpty())
            m_strAlbumArtist = m_strTmp4;

        // Did we get any art this time? If so, then copy it over
        if (!m_c4ArtSize && c4ArtSz)
        {
            m_c4ArtSize = c4ArtSz;
            m_mbufArt.CopyIn(m_mbufTmp, c4ArtSz);
        }

        // Store the track level stuff
        if (m_colGenres.c4ElemCount() < c4TrackCnt + 1)
            m_colGenres.objAdd(m_strTmp1);
        else
            m_colGenres[c4TrackCnt] = m_strTmp1;

        if (m_colFiles.c4ElemCount() < c4TrackCnt + 1)
            m_colFiles.objAdd(fndbSearch.pathFileName());
        else
            m_colFiles[c4TrackCnt] = fndbSearch.pathFileName();

        if (m_colArtists.c4ElemCount() < c4TrackCnt + 1)
            m_colArtists.objAdd(m_strTmp5);
        else
            m_colArtists[c4TrackCnt] = m_strTmp5;

        if (m_colTracks.c4ElemCount() < c4TrackCnt + 1)
            m_colTracks.objAdd(m_strTmp6);
        else
            m_colTracks[c4TrackCnt] = m_strTmp6;

        if (m_fcolBitRates.c4ElemCount() < c4TrackCnt + 1)
            m_fcolBitRates.c4AddElement(c4BitRate);
        else
            m_fcolBitRates[c4TrackCnt] = c4BitRate;

        if (m_fcolTrackNums.c4ElemCount() < c4TrackCnt + 1)
            m_fcolTrackNums.c4AddElement(c4TrackNum);
        else
            m_fcolTrackNums[c4TrackCnt] = c4TrackNum;

        if (m_fcolTimes.c4ElemCount() < c4TrackCnt + 1)
            m_fcolTimes.c4AddElement(c4Duration);
        else
            m_fcolTimes[c4TrackCnt] = c4Duration;

        if (m_fcolYears.c4ElemCount() < c4TrackCnt + 1)
            m_fcolYears.c4AddElement(c4Year);
        else
            m_fcolYears[c4TrackCnt] = c4Year;

        // And bump the count of valid tracks
        c4TrackCnt++;
    }
}


//
//  This is the recursive scanner method. It gets called on the main repo
//  path by the loader thread and just grinds away until it's iterated the
//  whole directory.
//
//  We do a depth first parse, looking first for sub-dirs. We assume that all
//  of the files in a given directory belong to the same collection. So we
//  use the hash of the path as a unique id for the title set (we can only
//  do a single collection per title set because we don't have the info we
//  need to put together title sets.)
//
//  We have to jump through some hoops here. Any of the files might be
//  missing important information that we need, so we can't just look at the
//  first file for the data and set it all up and then process the others.
//  OTOH, we don't want to redundanty scan a bunch of the files to find the
//  info we need to get started and then go back and scan them again to
//  actually load up the data.
//
//  So we fill in a set of vectors with the data, and just process the whole
//  directory at once until we have all the data and then we can scan the
//  vectors to aggregate the data we need to set up the database.
//
tCIDLib::TVoid FileTagRepoLoader::ProcessDir(const TString& strPath)
{
    TDirIter diterLevel;
    TFindBuf fndbSearch;

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_InDirectory
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strPath
        );
    }

    // See if there are any subdirs first and recurse on them.
    if (diterLevel.bFindFirst(  strPath
                                , kCIDLib::pszAllFilesSpec
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::NormalDirs))
    {
        do
        {
            if (fndbSearch.bIsDirectory())
                ProcessDir(fndbSearch.pathFileName());
        }   while (diterLevel.bFindNext(fndbSearch));
    }

    //
    //  Process any playlist files in this directory. For whatever reason,
    //  Windows retardly treats searches as though they really have a * at
    //  the end of the extension, so this will return both .m3u and .m3u8
    //  files. Only do this if asked to.
    //
    if (m_bLoadPlaylists)
    {
        if (diterLevel.bFindFirst(  strPath
                                    , L"*.m3u"
                                    , fndbSearch
                                    , tCIDLib::EDirSearchFlags::AllFiles))
        {
            do
            {
                ProcessPlaylist(fndbSearch.pathFileName());
            }   while (diterLevel.bFindNext(fndbSearch));
        }

        if (diterLevel.bFindFirst(  strPath
                                    , L"*.m3u8"
                                    , fndbSearch
                                    , tCIDLib::EDirSearchFlags::AllFiles))
        {
            do
            {
                ProcessPlaylist(fndbSearch.pathFileName());
            } while(diterLevel.bFindNext(fndbSearch));
        }
    }

    // Reset some stuff for this new directory and let's look for tracks
    tCIDLib::TCard4 c4TrackCnt = 0;
    m_c4ArtSize = 0;
    m_strAlbum.Clear();
    m_strAlbumArtist.Clear();
    m_strDescr.Clear();

    //
    //  And now process this directory. We look for each type of file that
    //  we support. We search for:
    //
    //  1. MPEG4 (M4P/M4A)
    //  2. MP3
    //  3. WMA
    //
    if (diterLevel.bFindFirst(  strPath
                                , L"*.m4p"
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::AllFiles))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_ProcessingFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"MP4")
            );
        }

        do
        {
            ProcessFile(fndbSearch, c4TrackCnt, m_metexAAC);
        }   while (diterLevel.bFindNext(fndbSearch));
    }

    if (diterLevel.bFindFirst(  strPath
                                , L"*.m4a"
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::AllFiles))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_ProcessingFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"M4A")
            );
        }

        do
        {
            ProcessFile(fndbSearch, c4TrackCnt, m_metexAAC);
        }   while (diterLevel.bFindNext(fndbSearch));
    }

    if (diterLevel.bFindFirst(  strPath
                                , L"*.MP3"
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::AllFiles))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_ProcessingFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"MP3")
            );
        }

        do
        {
            ProcessFile(fndbSearch, c4TrackCnt, m_metexWMA);
        }   while (diterLevel.bFindNext(fndbSearch));
    }

    if (diterLevel.bFindFirst(  strPath
                                , L"*.WMA"
                                , fndbSearch
                                , tCIDLib::EDirSearchFlags::AllFiles))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_ProcessingFiles
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"WMA")
            );
        }

        do
        {
            ProcessFile(fndbSearch, c4TrackCnt, m_metexWMA);
        }   while (diterLevel.bFindNext(fndbSearch));
    }

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_TracksFound
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(c4TrackCnt)
        );
    }

    // If we didn't find anything, we are done
    if (!c4TrackCnt)
        return;

    //
    //  Let's process what we found. First we hash the directory path and spit it
    //  back out as a text hash. This will be the unique id for the title set, and
    //  the other ids will be derived from it by just adding prefixes or suffixes.
    //
    m_mdigId.StartNew();
    m_mdigId.DigestStr(strPath);
    m_mdigId.Complete(m_mhashId);
    m_mhashId.FormatToStr(m_strUniqueId);

    //
    //  There is some content, so see if any external cover art file is present. We'll
    //  try to find one that's closest to about 32K, favoring a larger one if they are
    //  basically the same difference from the ideal.
    //
    //  We only do this if there wasn't embedded art, or we were told not to do any
    //  embedded art in which case we wouldn't have stored any. Else we try to load an
    //  outboard cover art image.
    //
    //  Note we have to set a unique id here that will not change on a reload, else
    //  the DB serial number will come out differently each time. So we generate one
    //  from the art content itself.
    //
    TMediaImg* pmimgArt = 0;
    TJanitor<TMediaImg> janImg;
    if (m_c4ArtSize)
    {
        facCQCMedia().CreatePersistentId(m_mbufArt, m_c4ArtSize, m_strTmp1);

        // Pre-cache the art since we already loaded it
        pmimgArt = new TMediaImg(m_eMediaType, TString::strEmpty(), TString::strEmpty());
        pmimgArt->SetArt(m_c4ArtSize, m_mbufArt, tCQCMedia::ERArtTypes::LrgCover);

        janImg.Set(pmimgArt);
    }
     else
    {
        const tCIDLib::TInt4    i4TarSize(32 * 1024);
        tCIDLib::TInt4          i4LastDiff;
        tCIDLib::TInt4          i4LastSz;
        TString                 strBestImg;

        if (diterLevel.bFindFirst(  strPath
                                    , L"*.jpg"
                                    , fndbSearch
                                    , tCIDLib::EDirSearchFlags::AllFiles))
        {
            if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
            {
                facFileTagRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kFlTagRepoMsgs::midStatus_SelectArt
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c4TrackCnt)
                );
            }

            //
            //  Start with both set to the first file we found, and set up
            //  the diffs and sizes
            //
            i4LastSz = tCIDLib::TInt4(fndbSearch.c8Size());
            i4LastDiff = i4LastSz - i4TarSize;
            if (i4LastDiff < 0)
                i4LastDiff *= -1;
            strBestImg = fndbSearch.pathFileName();

            //
            //  While we have more, compare them and see if any of them are
            //  closer to the optimal size.
            //
            while (diterLevel.bFindNext(fndbSearch))
            {
                tCIDLib::TInt4 i4Sz = tCIDLib::TInt4(fndbSearch.c8Size());
                tCIDLib::TInt4 i4Diff = i4Sz - i4TarSize;
                if (i4Diff < 0)
                    i4Diff *= -1;

                if (i4Diff < i4LastDiff)
                {
                    //
                    //  If the differences are small, and the new size is
                    //  smaller, keep the larger one. If the difference is
                    //  significant, then take the closer one.
                    //
                    if ((i4LastDiff - i4Diff > 4096) || (i4Sz > i4LastSz))
                    {
                        i4LastSz = i4Sz;
                        i4LastDiff = i4Diff;
                        strBestImg = fndbSearch.pathFileName();
                    }
                }
            }
        }

        //
        //  If we got any art, add it to the database. We don't load the actual
        //  file, so it will be faulted in later upon access. Since we just
        //  look for a larger image, we just set the art path, not the large
        //  art path. We use the standard image loader halper in the driver that
        //  understands this means that this means we only have one image and
        //  it will create the small one by scaling this image if required.
        //
        if (!strBestImg.bIsEmpty())
        {
            if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
            {
                facFileTagRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kFlTagRepoMsgs::midStatus_LoadingArt
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strBestImg
                );
            }

            //
            //  Load the image and cache it. Generate the persistent id, and store
            //  it on the large and small art.
            //
            try
            {
                TBinaryFile flImg(strBestImg);
                flImg.Open
                (
                    tCIDLib::EAccessModes::Excl_Read
                    , tCIDLib::ECreateActs::OpenIfExists
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                );

                // Read up to the max persistent id bytes and create it
                const tCIDLib::TCard4 c4PerIdSz = flImg.c4ReadBuffer
                (
                    m_mbufArt, 48 * 1024, tCIDLib::EAllData::OkIfNotAll
                );
                facCQCMedia().CreatePersistentId(m_mbufArt, c4PerIdSz, m_strTmp1);

                pmimgArt = new TMediaImg(m_eMediaType, strBestImg, TString::strEmpty());
                janImg.Set(pmimgArt);

                //
                //  If we read the whole file, then go ahead and cache it since
                //  we have it all already.
                //
                if (tCIDLib::TCard4(flImg.c8CurSize()) == c4PerIdSz)
                    pmimgArt->SetArt(c4PerIdSz, m_mbufArt, tCQCMedia::ERArtTypes::LrgCover);
            }

            catch(...)
            {
            }
        }
    }

    //
    //  If we got some art, set up the persistent id and add it to the database.
    //  Since we create the thumb art from the large art, set the same id on both
    //  of them, since they are from the same iamge.
    if (pmimgArt)
    {
        m_strTmp7 = L"Img_";
        m_strTmp7.Append(m_strUniqueId);
        pmimgArt->strUniqueId(m_strTmp7);

        pmimgArt->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::SmlCover);
        pmimgArt->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::LrgCover);
        m_pmdbLoading->c2AddImage(janImg.pobjOrphan());
    }

    //
    //  If we did not get an album artist, then build one from the per-track
    //  artists and set it on the collection and title. If they are all the
    //  same, we use that, else we use "Various Artists".
    //
    if (m_strAlbumArtist.bIsEmpty())
    {
        tCIDLib::TCard4 c4Index = 1;
        for (; c4Index < c4TrackCnt; c4Index++)
        {
            if (m_colArtists[c4Index] != m_colArtists[c4Index - 1])
                break;
        }

        if (c4Index == c4TrackCnt)
            m_strAlbumArtist = m_colArtists[0];
         else
            m_strAlbumArtist = L"Various Artists";
    }

    //
    //  Split the album artist out, and replace any circumflexes with spaces,
    //  because of historical reasons that no longer matter, we just don't
    //  want to break older setups.
    //
    m_strAlbumArtist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);

    //
    //  Create a new title set and add it to the database. Create a sort
    //  title automaticlaly based on the album name.
    //
    facCQCMedia().bMakeSortTitle(m_strAlbum, m_strTmp1);
    TMediaTitleSet* pmtsCur = new TMediaTitleSet
    (
        m_strAlbum, m_strUniqueId, m_strTmp1, m_eMediaType
    );
    TJanitor<TMediaTitleSet> janSet(pmtsCur);

    pmtsCur->strArtist(m_strAlbumArtist);

    // Bump the titles done
    m_c4TitlesDone++;

    //
    //  And create and add a new collection, creating a new unique id for
    //  it based on the title set. Ste the year to zero here. It'll be
    //  aggregated from contained items at the end.
    //
    m_strTmp1 = m_strUniqueId;
    m_strTmp1.Append(L"_1");
    TMediaCollect* pmcolCur = new TMediaCollect
    (
        m_strAlbum
        , m_strTmp1
        , 0
        , tCQCMedia::ELocTypes::FileItem
        , m_eMediaType
    );
    TJanitor<TMediaCollect> janCol(pmcolCur);

    pmcolCur->strArtist(m_strAlbumArtist);

    // Set the image id for the cover art if any
    if (pmimgArt)
        pmcolCur->c2ArtId(pmimgArt->c2Id());

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_AddCats
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Add it to the all music category
    pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMusic);

    //
    //  If we got any other categories, then add it to all of them. We
    //  have the same number of potential genres as tracks since we store
    //  only one per.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TrackCnt; c4Index++)
    {
        const TString& strCurCat = m_colGenres[c4Index];
        if (strCurCat.bIsEmpty())
            continue;

        // See if we already have this cat, else add it
        m_strTmp1 = strCurCat;
        m_strTmp1.ToUpper();
        const TMediaCat* pmcatCur = m_pmdbLoading->pmcatByUniqueId
        (
            m_eMediaType, m_strTmp1, kCIDLib::False
        );

        if (!pmcatCur)
        {
            TMediaCat* pmcatNew = new TMediaCat(m_eMediaType, strCurCat, m_strTmp1);
            m_pmdbLoading->c2AddCategory(pmcatNew);
            pmcatCur = pmcatNew;
        }

        // Add our collection to this category
        pmcolCur->bAddCategory(*pmcatCur);
    }

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_CreateItems
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TrackCnt; c4Index++)
    {
        const TPathStr& pathCur = m_colFiles[c4Index];
        const tCIDLib::TCard4 c4TrackNum = m_fcolTrackNums[c4Index];

        m_strTmp1 = pmcolCur->strUniqueId();
        m_strTmp1.Append(kCIDLib::chUnderscore);
        m_strTmp1.AppendFormatted(c4Index);
        TMediaItem* pmitemNew = new TMediaItem
        (
            m_colTracks[c4Index]
            , m_strTmp1
            , m_colFiles[c4Index]
            , m_eMediaType
        );
        TJanitor<TMediaItem> janItem(pmitemNew);

        pmitemNew->c4BitRate(m_fcolBitRates[c4Index]);
        pmitemNew->c4Duration(m_fcolTimes[c4Index]);
        pmitemNew->c4Year(m_fcolYears[c4Index]);

        //
        //  Since we could get them out of order, we use the optional load
        //  order field to store the track number so that we can insert new
        //  ones at the correct spot in the collection.
        //
        pmitemNew->c4LoadOrder(c4TrackNum);

        //
        //  If we got art, set it on the item as well, so that it can be
        //  displayed when this item is being accessed via a playlist.
        //
        if (pmimgArt)
            pmitemNew->c2ArtId(pmimgArt->c2Id());

        //
        //  Get rid of any circumflex which used to be used as a separator
        //  when we stored separate first/last names.
        //
        TString strCurArtist = m_colArtists[c4Index];
        if (!strCurArtist.bIsEmpty())
        {
            strCurArtist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);
            pmitemNew->strArtist(strCurArtist);
        }

        //
        //  Figure out where to add the track in the collection. They are
        //  not necessarily in order.
        //
        const tCIDLib::TCard4 c4ItemCnt = pmcolCur->c4ItemCount();
        tCIDLib::TCard4 c4IInd = 0;
        for (; c4IInd < c4ItemCnt; c4IInd++)
        {
            //
            //  Break out when we see an existing track with a larger track
            //  number. Since we are adding them in order, this will keep the
            //  newly added ones sorted. Throw if not found since we are
            //  iterating the count the collection said it has, and using
            //  the id he reports at each index.
            //
            const TMediaItem* pmitemCur = m_pmdbLoading->pmitemById
            (
                m_eMediaType, pmcolCur->c2ItemIdAt(c4IInd), kCIDLib::True
            );
            if (pmitemCur->c4LoadOrder() > c4TrackNum)
                break;
        }

        // If not found, insert at the end
        if (c4IInd == c4ItemCnt)
            c4IInd = kCIDLib::c4MaxCard;

        // Add it to the database, then to the collection now that it's id is set
        m_pmdbLoading->c2AddItem(janItem.pobjOrphan());

        pmcolCur->bAddItem(*pmitemNew, c4IInd);
    }

    // Now we can add the collection and add it to the set now that it has an id
    m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());
    pmtsCur->bAddCollect(*pmcolCur);

    // And finally add the set
    m_pmdbLoading->c2AddTitle(janSet.pobjOrphan());

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_OutDirectory
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strPath
        );
    }
}


//
//  This gets called from the directory processing method above if any
//  playlist files are found.
//
//  If anything goes wrong, we just ignore the file.
//
tCIDLib::TVoid FileTagRepoLoader::ProcessPlaylist(const TPathStr& pathPLFile)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_ProcessingPL
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(m_colPlayLists.c4ElemCount() + 1)
            , pathPLFile
        );
    }

    // If our playlist is full, then don't do anymore
    if (m_colPlayLists.bIsFull(8192))
        return;

    //
    //  Create the name of the playlist, based on the file name. If there
    //  is no name component, something is seriously wierd so just return.
    //
    TString  strName;
    if (!pathPLFile.bQueryName(strName))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_BadPLPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , pathPLFile
            );
        }
        return;
    }

    // If there is alreayd one with this name, skip it
    if (m_colPlayLists.bKeyExists(strName))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facFileTagRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kFlTagRepoMsgs::midStatus_DupPLName
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strName
            );
        }
        return;
    }

    //
    //  And create the bass path that any relative path references will be
    //  relative to.
    //
    TPathStr pathBase(pathPLFile);
    pathBase.bRemoveNameExt();

    try
    {
        //
        //  Open the file as an input text stream, with appropriate encoding,
        //  based on the extension.
        //
        TString strExt;
        TTextConverter* ptcvtPL;
        pathPLFile.bQueryExt(strExt);
        if (strExt.bCompareI(L"M3U8"))
            ptcvtPL = facCIDEncode().ptcvtMake(L"UTF-8");
        else
            ptcvtPL = facCIDEncode().ptcvtMake(L"Latin1");

        TTextFileInStream strmPL
        (
            pathPLFile
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Multi_Read
            , ptcvtPL
        );

        //
        //  Add a new temporary item to the playlists vector and get a ref to
        //  it. The first entry is the name of the playlist, so that we don't
        //  need a separate place to store that. We have to use a temp to
        //  create it because we have to get the 0th entry first, since it
        //  is the key for the keyed hash set.
        //
        TPathList colTmpNew;
        colTmpNew.objAdd(strName);
        TPathList& colNew = m_colPlayLists.objAdd(colTmpNew);

        //
        //  The second entry is for the path to any cover art. If there's a
        //  .jpg file in the same directory with the same name, we take that
        //  as the cover art for this playlist. We store the path for later
        //  loading.
        //
        m_pathTmp1 = pathPLFile;
        m_pathTmp1.bRemoveExt();
        m_pathTmp1.AppendExt(L"jpg");
        if (TFileSys::bExists(m_pathTmp1))
            colNew.objAdd(m_pathTmp1);
        else
            colNew.objAdd(TString::strEmpty());

        //
        //  And let's run through the lines in this file. We toss out
        //  directives and comments.
        //
        tCIDLib::TCard4 c4LineNum(0);
        while (!strmPL.bEndOfStream())
        {
            // Get a line with lead/trail whitespace stripped off
            c4LineNum++;
            strmPL.c4GetLine(m_pathTmp1, 0, kCIDLib::True);

            // Skip empty lines or comments/directives
            if (m_pathTmp1.bIsEmpty() || (m_pathTmp1[0] == kCIDLib::chPoundSign))
                continue;

            if (m_pathTmp1.bIsFullyQualified())
            {
                // Take it as is
                colNew.objAdd(m_pathTmp1);
            }
             else
            {
                // It's relative so expand it and normalize it
                m_pathTmp2 = pathBase;
                m_pathTmp2.AddLevel(m_pathTmp1);

                try
                {
                    m_pathTmp2.Normalize();
                    colNew.objAdd(m_pathTmp2);
                }

                catch(TError& errToCatch)
                {
                    // Just skip this one, it's bad
                    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
                    {
                        if (!errToCatch.bLogged())
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }

                        facCQCMedia().LogMsg
                        (
                            CID_FILE
                            , CID_LINE
                            , kMedMsgs::midStatus_BadPLLine
                            , tCIDLib::ESeverities::Status
                            , tCIDLib::EErrClasses::AppStatus
                            , TCardinal(c4LineNum)
                            , pathPLFile
                        );
                    }
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_PLParseFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , pathPLFile
            );
        }

        // Remove the playlist entry if we added it
        m_colPlayLists.bRemoveKeyIfExists(strName);
    }

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facFileTagRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kFlTagRepoMsgs::midStatus_PLComplete
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , pathPLFile
        );
    }
}

