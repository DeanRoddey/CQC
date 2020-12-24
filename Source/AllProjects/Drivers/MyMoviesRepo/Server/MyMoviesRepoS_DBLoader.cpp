//
// FILE NAME: MyMoviesRepoS_DBLoader.cpp
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
RTTIDecls(TMyMoviesRepoLoader,TThread)



// ---------------------------------------------------------------------------
//   CLASS: TMyMoviesRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMyMoviesRepoLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TMyMoviesRepoLoader::TMyMoviesRepoLoader(const  TString&    strMoniker
                                        , const TString&    strODBCSrc
                                        , const TString&    strUserName
                                        , const TString&    strPassword
                                        , const TString&    strImagePath) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_c4TitlesDone(0)
    , m_eMediaType(tCQCMedia::EMediaTypes::Movie)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_pmdbLoading(0)
    , m_strImagePath(strImagePath)
    , m_strODBCSrc(strODBCSrc)
    , m_strPassword(strPassword)
    , m_strUserName(strUserName)
    , m_strMoniker(strMoniker)
{
}

TMyMoviesRepoLoader::~TMyMoviesRepoLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TMyMoviesRepoLoader::CompleteLoad()
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
TMyMoviesRepoLoader::eStatus(tCIDLib::TCard4& c4TitlesDone) const
{
    // Return the status and the count of titles processed
    c4TitlesDone = m_c4TitlesDone;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TMyMoviesRepoLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


const TString& TMyMoviesRepoLoader::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid TMyMoviesRepoLoader::StartLoad(TMediaDB* const pmdbToLoad)
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
    }

    // Store the new database to load and reset it
    m_pmdbLoading = pmdbToLoad;
    m_pmdbLoading->Reset();

    // Reset any of our per-load data
    m_c4TitlesDone = 0;

    // Add an 'All Movies' category that all stuff can be put into
    m_strAllMoviesCatName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMovies);
    m_strTmp1 = m_strAllMoviesCatName;
    m_strTmp1.ToUpper();
    TMediaCat* pmcatAll = new TMediaCat(m_eMediaType, m_strAllMoviesCatName, m_strTmp1);
    pmcatAll->c2Id(kCQCMedia::c2CatId_AllMovies);
    m_pmdbLoading->c2AddCategory(pmcatAll, kCIDLib::True);

    // Set the status to indicate we are loading
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Start ourself running in another thread
    Start();
}


//
//  Called by the main thread to make sure we are cleaned up when it needs
//  to shutdown.
//
tCIDLib::TVoid TMyMoviesRepoLoader::Shutdown()
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
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }
}


// ---------------------------------------------------------------------------
//  TMyMoviesRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------

// The thread entry point for the loader
tCIDLib::EExitCodes TMyMoviesRepoLoader::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        //
        //  Call a helper that does the actual database iteration and loading
        //  up the content.
        //
        ImportDatabase();

        //
        //  Ok, it's done successfully, so call the load completion method
        //  on the media database, so that it can do any stuff it has to wait
        //  until all the data is available to do.
        //
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

        // Mark us now as done
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
                , m_strODBCSrc
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
                , m_strODBCSrc
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
        facMyMoviesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMyMvErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TMyMoviesRepoLoader: Private, non=virtual methods
// ---------------------------------------------------------------------------

//
//  This is the main method for the import of the MyMovies database. We
//  open it up as an ODBC source and run through the various tables and
//  load up our database. The caller will catch any exceptions
//
tCIDLib::TVoid TMyMoviesRepoLoader::ImportDatabase()
{
    //
    //  Open up the data source. The main driver will check that it's
    //  there before starting a load, so this should always work.
    //
    TDBConnection dbconMM;
    dbconMM.Connect(m_strODBCSrc, m_strUserName, m_strPassword, kCIDLib::False);

    //
    //  First of all, load the database table, which gets us a version number
    //  so that we can know what version of the database this is. We have to
    //  deal with some differences.
    //
    tCIDLib::TInt4 i4DBVersion;
    {
        TDBDirStatement dbstmtDB(dbconMM, L"Query Database Table");
        dbstmtDB.Reset();

        dbstmtDB.Reset();
        dbstmtDB.AddColBinding(L"Version", tCIDDBase::ECppTypes::Int4);
        dbstmtDB.Execute(L"SELECT intVersion FROM tblDatabase");
        dbstmtDB.bFetch();
        i4DBVersion = dbstmtDB.i4ColVal(0);
    }

    //
    //  We need to load a bunch of lookup tables. A lot of the data is
    //  stored in tables where each entry has a title id, and those are
    //  used as keys in the main title table. So we load up the fundamental
    //  stuff into tables (sorted by id), so that we can later quickly
    //  go back and resolve the id keys without having to do SQL queries.
    //

    // Load the categories
    TItemTable colCats;
    {
        TDBDirStatement dbstmtCats(dbconMM, L"Query Categories");
        dbstmtCats.Reset();

        //
        //  Add column bindings that we'll need, and execute a select
        //  statement.
        //
        dbstmtCats.Reset();
        dbstmtCats.AddColStrBinding(L"Name", 256);
        dbstmtCats.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
        dbstmtCats.Execute(L"SELECT nvcName, intId FROM tblGenres ORDER BY intId");
        while (dbstmtCats.bFetch())
        {
            if (dbstmtCats.strColVal(0).bIsEmpty())
                continue;
            colCats.objAdd
            (
                TTblItem(dbstmtCats.strColVal(0), dbstmtCats.i4ColVal(1))
            );
        }
    }

    //
    //  For pre V19 versions, we load up director an actor tables and do
    //  simple lookups for them once. Otherwise, there's a single Persons
    //  table which we load here and then reference below via intermediate
    //  table lookup per-title, which we'll then use to load the actors
    //  directors list each time around.
    //
    TItemTable colActors;
    TItemTable colDirectors;
    TItemTable colPersons;
    if (i4DBVersion < 19)
    {
        {
            TDBDirStatement dbstmtDirectors(dbconMM, L"Query Directors");
            dbstmtDirectors.Reset();

            dbstmtDirectors.Reset();
            dbstmtDirectors.AddColStrBinding(L"Name", 255);
            dbstmtDirectors.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
            dbstmtDirectors.Execute
            (
                L"SELECT nvcName, intId FROM tblDirectors ORDER BY intId"
            );

            while (dbstmtDirectors.bFetch())
            {
                if (dbstmtDirectors.strColVal(0).bIsEmpty())
                    continue;
                colDirectors.objAdd
                (
                    TTblItem(dbstmtDirectors.strColVal(0), dbstmtDirectors.i4ColVal(1))
                );
            }
        }

        {
            TDBDirStatement dbstmtActors(dbconMM, L"Query Actors");
            dbstmtActors.Reset();

            dbstmtActors.Reset();
            dbstmtActors.AddColStrBinding(L"Name", 255);
            dbstmtActors.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
            dbstmtActors.Execute
            (
                L"SELECT nvcName, intId FROM tblActors ORDER BY intId"
            );

            while (dbstmtActors.bFetch())
            {
                if (dbstmtActors.strColVal(0).bIsEmpty())
                    continue;
                colActors.objAdd
                (
                    TTblItem(dbstmtActors.strColVal(0), dbstmtActors.i4ColVal(1))
                );
            }
        }
    }
     else
    {
        TDBDirStatement dbstmtPersons(dbconMM, L"Query Persons");
        dbstmtPersons.Reset();

        dbstmtPersons.Reset();
        dbstmtPersons.AddColStrBinding(L"Name", 255);
        dbstmtPersons.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
        dbstmtPersons.Execute
        (
            L"SELECT nvcName, intId FROM tblPersons ORDER BY intId"
        );

        while (dbstmtPersons.bFetch())
        {
            if (dbstmtPersons.strColVal(0).bIsEmpty())
                continue;
            colPersons.objAdd
            (
                TTblItem(dbstmtPersons.strColVal(0), dbstmtPersons.i4ColVal(1))
            );
        }

    }

    // Get the studios
    TItemTable colStudioList;
    {
        TDBDirStatement dbstmtStudios(dbconMM, L"Query Studios");
        dbstmtStudios.Reset();

        dbstmtStudios.Reset();
        dbstmtStudios.AddColStrBinding(L"Name", 255);
        dbstmtStudios.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
        dbstmtStudios.Execute
        (
            L"SELECT nvcName, intId FROM tblStudios ORDER BY intId"
        );

        while (dbstmtStudios.bFetch())
        {
            if (dbstmtStudios.strColVal(0).bIsEmpty())
                continue;
            colStudioList.objAdd
            (
                TTblItem(dbstmtStudios.strColVal(0), dbstmtStudios.i4ColVal(1))
            );
        }
    }

    // Load the format types
    TItemTable colFmtTypes;
    {
        TDBDirStatement dbstmtFmts(dbconMM, L"Query Formats");
        dbstmtFmts.Reset();

        //
        //  Add column bindings that we'll need, and execute a select
        //  statement.
        //
        dbstmtFmts.Reset();
        dbstmtFmts.AddColStrBinding(L"Name", 256);
        dbstmtFmts.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
        dbstmtFmts.Execute(L"SELECT nvcName, intId FROM tblTypes ORDER BY intId");
        while (dbstmtFmts.bFetch())
        {
            if (dbstmtFmts.strColVal(0).bIsEmpty())
                continue;
            colFmtTypes.objAdd
            (
                TTblItem(dbstmtFmts.strColVal(0), dbstmtFmts.i4ColVal(1))
            );
        }
    }


    //
    //  Now let's interate the list of titles. To avoid having multiple
    //  quries going on, we just load up the titles here, and save the
    //  discs for the next loop. We'll also build some tables to store
    //  stuff that we get on the title, but for us we store on the
    //  collections, so we have to save them for later.
    //
    TTblItem* pitemTmp;
    TItemTable colArt;
    TItemTable colAspect;
    TItemTable colDescr;
    TItemTable colDuration;
    TItemTable colFormat;
    TItemTable colRating;
    TItemTable colUPC;
    TItemTable colYears;
    {
        TDBDirStatement dbstmtTitles(dbconMM, L"Query Titles");
        dbstmtTitles.Reset();

        //
        //  Add column bindings that we'll need, and execute a select
        //  statement.
        //
        dbstmtTitles.Reset();
        dbstmtTitles.AddColStrBinding(L"Title", 256);
        dbstmtTitles.AddColStrBinding(L"Descr", 8192);
        dbstmtTitles.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColBinding(L"Director", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColBinding(L"Rating", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColBinding(L"Runtime", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColStrBinding(L"UPC", 256);
        dbstmtTitles.AddColStrBinding(L"CoverArt", 256);
        dbstmtTitles.AddColBinding(L"Year", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColStrBinding(L"SortTitle", 256);
        dbstmtTitles.AddColBinding(L"Format", tCIDDBase::ECppTypes::Int4);
        dbstmtTitles.AddColStrBinding(L"Aspect", 256);


        //
        //  Theres a difference in some fields if we are pre version 19
        //  so we have to do a different query.
        //
        if (i4DBVersion < 19)
        {
            dbstmtTitles.Execute
            (
                L"SELECT nvcOriginalTitle, ntxDescription, intId, intDirector,"
                L" intRating, intRuntime, nvcUPC, nvcCover, intProductionYear,"
                L" nvcSortTitle, intType, nvcAspectRatio"
                L" FROM tblTitles"
            );
        }
         else
        {
            //
            //  Note that intDirector really isn't used now. It is still present
            //  but the way of finding people now is to search the title persons
            //  table for any persons from that title, to get their id, then
            //  search the persons table for that id, so much more convoluted.
            //
            //  But we don't want to remove it since that would require adjusting
            //  down all the binding indices below.
            //
            dbstmtTitles.Execute
            (
                L"SELECT nvcOriginalTitle, ntxDescription, intId, intDirector,"
                L" intCertification, intRuntime, nvcUPC, nvcCover, intProductionYear,"
                L" nvcSortTitle, intType, nvcAspectRatio"
                L" FROM tblTitles"
            );
        }

        while (dbstmtTitles.bFetch())
        {
            // If the title is empty, we have to ignore this one
            if (dbstmtTitles.strColVal(0).bIsEmpty())
                continue;

            //
            //  Get the title id out, which we'll need later to look it up
            //  and to later go back and look them up. We store it in each
            //  of the list items above so that we can find the ones for
            //  a given title id.
            //
            const tCIDLib::TCard4 c4TitleID = dbstmtTitles.i4ColVal(2);

            // Store some stuff in tables for later
            if (!dbstmtTitles.strColVal(11).bIsEmpty())
                colAspect.objAdd(TTblItem(dbstmtTitles.strColVal(11), c4TitleID));
            if (!dbstmtTitles.strColVal(6).bIsEmpty())
                colUPC.objAdd(TTblItem(dbstmtTitles.strColVal(6), c4TitleID));

            if (!dbstmtTitles.strColVal(1).bIsEmpty())
            {
                //
                //  The description has CR/LF separators, but we always assume
                //  just LF once internalized. So we have to go through and
                //  read in each line and adjust. The default terminiator
                //  for text streams is CR/LF on input so we just stream in
                //  each line and write them back out with the appropriate
                //  terminator, also getting rid of any extra empty lines.
                //
                m_strTmp1 = dbstmtTitles.strColVal(1);
                TTextStringInStream strmSrc(&m_strTmp1);
                m_strTmp2.Clear();
                tCIDLib::TCard4 c4EmptyCnt = 0;
                while (!strmSrc.bEndOfStream())
                {
                    strmSrc >> m_strTmp3;
                    if (m_strTmp3.bIsEmpty())
                    {
                        //
                        //  We let the first empty line through, then eat
                        //  empty lines until we see another non-empty.
                        //
                        if (!c4EmptyCnt)
                            m_strTmp2.Append(L"\n\n");
                        c4EmptyCnt++;
                    }
                     else
                    {
                        //
                        //  Clear the empty count now that we've gotten
                        //  a non-empty line, and copy this one over as
                        //  is.
                        //
                        c4EmptyCnt = 0;
                        m_strTmp2.Append(m_strTmp3);
                    }
                }
                colDescr.objAdd(TTblItem(m_strTmp2, c4TitleID));
            }

            // Translate the rating value and store that
            switch(dbstmtTitles.i4ColVal(4))
            {
                case -1 :
                case 0 :
                    m_strTmp1 = L"Unrated";
                    break;

                case 1 :
                    m_strTmp1 = L"G";
                    break;

                case 3 :
                    m_strTmp1 = L"PG";
                    break;

                case 4 :
                    m_strTmp1 = L"PG-13";
                    break;

                case 6 :
                    m_strTmp1 = L"R";
                    break;

                case 7 :
                    m_strTmp1 = L"NC-17";
                    break;

                default :
                    m_strTmp1.Clear();
                    break;
            }
            colRating.objAdd(TTblItem(m_strTmp1, c4TitleID));

            // Format the duration out so we can use a table to to store that
            m_strTmp1.SetFormatted(dbstmtTitles.i4ColVal(5) * 60);
            colDuration.objAdd(TTblItem(m_strTmp1, c4TitleID));

            // Translate the format, if any, first and then store that
            TTblItem* pitemFormat = pitemFind(colFmtTypes, dbstmtTitles.i4ColVal(10));
            if (pitemFormat)
                colFormat.objAdd(TTblItem(pitemFormat->m_strValue, c4TitleID));

            //
            //  The sort title needs to be upper case for our system. If it
            //  is empty, then set it to the title, upper cased.
            //
            if (dbstmtTitles.strColVal(9).bIsEmpty())
                m_strTmp1 = dbstmtTitles.strColVal(0);
            else
                m_strTmp1 = dbstmtTitles.strColVal(9);
            m_strTmp1.ToUpper();

            // Create a unique id based on the MM id
            m_strTmp2 = L"MyMvTitle-";
            m_strTmp2.AppendFormatted(c4TitleID);

            //
            //  Be careful and watch for a dup, which will fail. If so,
            //  try to create a new unique id. If not unique after a
            //  few tries, skip it.
            //
            if (m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strTmp2, kCIDLib::False))
            {
                m_strTmp2.Append(L'_');
                if (m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strTmp2, kCIDLib::False))
                {
                    m_strTmp2.Append(L'_');
                    if (m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strTmp2, kCIDLib::False))
                    {
                        // If still not unique, skip this one
                        continue;
                    }
                }
            }

            // Create a new title set
            TMediaTitleSet* pmtsCur = new TMediaTitleSet
            (
                dbstmtTitles.strColVal(0)
                , m_strTmp2
                , m_strTmp1
                , m_eMediaType
            );
            TJanitor<TMediaTitleSet> janSet(pmtsCur);

            //
            //  In pre V19, we go to the directors list we pre-loaded. Else,
            //  we have to wait till the next loop below to do it, since we
            //  have to do an indirect lookup, as we do with the actors.
            //
            if (i4DBVersion < 19)
            {
                pitemTmp = pitemFind(colDirectors, dbstmtTitles.i4ColVal(3));
                if (pitemTmp)
                    pmtsCur->strArtist(pitemTmp->m_strValue);
            }

            // Store the year away for the later disc processing
            colYears.objAdd(TTblItem(dbstmtTitles.i4ColVal(8), c4TitleID));

            //
            //  Store the MyMovies id in the AltId so that we can back and find the
            //  discs for this title set. We need to be back to get back to them by
            //  the alt id.
            //
            pmtsCur->c4AltId(c4TitleID);

            // Set up a table item for the art
            colArt.objAdd
            (
                colArt.objAdd(TTblItem(dbstmtTitles.strColVal(7), c4TitleID))
            );

            // And now we can add this title
            m_pmdbLoading->c2AddTitle(janSet.pobjOrphan());

            // Bump the count of titles done
            m_c4TitlesDone++;
        }
    }


    //
    //  Now we go back through the titles and use the store info to set up
    //  title sets for each of them. We have to do various auxillary searches
    //  for the required info.
    //
    const TString strTitleQueryBase
    (
        L"SELECT nvcLabel,intId,intTitle,intLocation,nvcLocation,nvcSlot"
        L" FROM tblDiscs WHERE intTitle='"
    );

    const TString strGenreQueryBase
    (
        L"SELECT intGenre FROM tblTitleGenre WHERE intTitle='"
    );

    const TString strPersonalQueryBase
    (
        L"SELECT datAdded, intRating FROM tblTitlePersonal WHERE intTitle='"
    );

    const TString strStudioQueryBase
    (
        L"SELECT intStudio from tblTitleStudios where intTitle='"
    );


    TFundVector<tCIDLib::TCard2> fcolCatIds;
    TFundVector<tCIDLib::TCard2> fcolColds;

    TString strStudio;
    const tCIDLib::TCard4 c4TitleCnt = m_pmdbLoading->c4TitleSetCnt(m_eMediaType);
    for (tCIDLib::TCard4 c4TitleInd = 0; c4TitleInd < c4TitleCnt; c4TitleInd++)
    {
        // Get the next title to work on, and pull the data base unique id out
        TMediaTitleSet mtsCur = m_pmdbLoading->mtsAt(m_eMediaType, c4TitleInd);
        const tCIDLib::TCard4 c4TitleId = mtsCur.c4AltId();

        // Look up the previously stored info for this title
        TTblItem* pitemAspect = pitemFind(colAspect, c4TitleId);
        TTblItem* pitemDescr = pitemFind(colDescr, c4TitleId);
        TTblItem* pitemDuration = pitemFind(colDuration, c4TitleId);
        TTblItem* pitemMediaFmt = pitemFind(colFormat, c4TitleId);
        TTblItem* pitemRating = pitemFind(colRating, c4TitleId);
        TTblItem* pitemUPC = pitemFind(colUPC, c4TitleId);

        //
        //  Some of them we have to convert to numbers and it's better to it
        //  here once instead of for every disc.
        //
        tCIDLib::TCard4 c4CurYear = 0;
        TTblItem* pitemYear = pitemFind(colYears, c4TitleId);
        if (pitemYear)
            pitemYear->m_strValue.bToCard4(c4CurYear, tCIDLib::ERadices::Dec);

        tCIDLib::TCard4 c4CurDuration = 0;
        if (pitemDuration)
            pitemDuration->m_strValue.bToCard4(c4CurDuration, tCIDLib::ERadices::Dec);

        //
        //  If no art for this guy yet, then try to load it from the common cover
        //  art directory. If it's not there, we'll try in the per-disc directories
        //  below.
        //
        pitemTmp = pitemFind(colArt, c4TitleId);
        const TMediaImg* pmimgArt = 0;
        if (pitemTmp)
        {
            // If this art isn't already there, then add it
            pmimgArt = m_pmdbLoading->pmimgByUniqueId
            (
                tCQCMedia::EMediaTypes::Movie, pitemTmp->m_strValue, kCIDLib::False
            );

            if (!pmimgArt)
                pmimgArt = pmimgLoadArt(*pitemTmp);
        }

        //
        //  Look up all of the categories that this title is in. We'll add
        //  each of the dics to these categories.
        //
        fcolCatIds.RemoveAll();
        {
            TDBDirStatement dbstmtCats(dbconMM, L"Query Title Categories");
            dbstmtCats.Reset();

            dbstmtCats.Reset();
            dbstmtCats.AddColBinding(L"GenreId", tCIDDBase::ECppTypes::Int4);

            m_strTmp1 = strGenreQueryBase;
            m_strTmp1.AppendFormatted(c4TitleId);
            m_strTmp1.Append(L"'");
            dbstmtCats.Execute(m_strTmp1);

            while (dbstmtCats.bFetch())
            {
                const tCIDLib::TCard4 c4GenreId = dbstmtCats.i4ColVal(0);

                //
                //  See if we found this one in the genres table. If so,
                //  then process it.
                //
                pitemTmp = pitemFind(colCats, c4GenreId);
                if (pitemTmp)
                {
                    m_strTmp1 = pitemTmp->m_strValue;
                    m_strTmp1.ToUpper();
                    const TMediaCat* pmcatCur = m_pmdbLoading->pmcatByUniqueId
                    (
                        tCQCMedia::EMediaTypes::Movie, m_strTmp1, kCIDLib::False
                    );

                    if (!pmcatCur)
                    {
                        TMediaCat* pmcatNew = new TMediaCat
                        (
                            tCQCMedia::EMediaTypes::Movie
                            , pitemTmp->m_strValue
                            , m_strTmp1
                        );
                        m_pmdbLoading->c2AddCategory(pmcatNew);
                        pmcatCur = pmcatNew;
                    }
                    fcolCatIds.c4AddElement(pmcatCur->c2Id());
                }
            }
        }

        //
        //  Look up the studio for this title and store away the value
        //  for use in the collections below. It supports more than one
        //  but we just take the first one.
        //
        strStudio.Clear();
        {
            TDBDirStatement dbstmtStudios(dbconMM, L"Query Title Studios");
            dbstmtStudios.Reset();

            dbstmtStudios.Reset();
            dbstmtStudios.AddColBinding(L"StudioId", tCIDDBase::ECppTypes::Int4);

            m_strTmp1 = strStudioQueryBase;
            m_strTmp1.AppendFormatted(c4TitleId);
            m_strTmp1.Append(L"'");
            dbstmtStudios.Execute(m_strTmp1);

            while (dbstmtStudios.bFetch())
            {
                const tCIDLib::TCard4 c4StudioId = dbstmtStudios.i4ColVal(0);

                //
                //  Grab the studio from our previous lookup. If we find it,
                //  take this one and break out.
                //
                pitemTmp = pitemFind(colStudioList, c4StudioId);
                if (pitemTmp)
                {
                    strStudio = pitemTmp->m_strValue;
                    break;
                }
            }
        }


        //
        //  If this is vV19 or greater, then we need to load the actors and
        //  directors lists separately for each title because it involves a
        //  multi-level indirection. We look up the people for this title,
        //  and use that to reference the persons list.
        //
        //  We set up a cast string for the collections below and set up
        //  the atcors list (which otherwise was set up above in a more
        //  straightforward way for older database versions.) And we'll
        //  set store the director.
        //
        TString strCast;
        if (i4DBVersion >= 19)
        {
            colActors.RemoveAll();
            colDirectors.RemoveAll();

            {
                TDBDirStatement dbstmtPersons(dbconMM, L"Query Title Persons");
                dbstmtPersons.Reset();
                dbstmtPersons.AddColBinding(L"PersonId", tCIDDBase::ECppTypes::Int4);
                dbstmtPersons.AddColBinding(L"Type", tCIDDBase::ECppTypes::Int4);

                m_strTmp1 = L"SELECT intPerson, intType from tblTitlePerson WHERE "
                            L"intTitle=";
                m_strTmp1.AppendFormatted(c4TitleId);
                m_strTmp1.Append(L" ORDER BY intSort");
                dbstmtPersons.Execute(m_strTmp1);

                while (dbstmtPersons.bFetch())
                {
                    pitemTmp = pitemFind(colPersons, dbstmtPersons.i4ColVal(0));
                    if (pitemTmp)
                    {
                        if (dbstmtPersons.i4ColVal(1) == 1)
                        {
                            // It's an actor
                            colActors.objAdd(*pitemTmp);

                            if (!strCast.bIsEmpty())
                                strCast.Append(L", ");
                            strCast.Append(pitemTmp->m_strValue);
                        }
                         else if (dbstmtPersons.i4ColVal(1) == 2)
                        {
                            // It's a director. If we don't have one already, take it
                            if (mtsCur.strArtist().bIsEmpty())
                                mtsCur.strArtist(pitemTmp->m_strValue);
                        }
                    }
                }
            }
        }

        //
        //  Now set up a query on the discs table to find all of the
        //  discs that match this title.
        //
        tCIDLib::TCard4 c4DiscNum = 1;
        {
            TDBDirStatement dbstmtDiscs(dbconMM, L"Query Discs");
            dbstmtDiscs.Reset();

            //
            //  Add column bindings that we'll need, and execute a select
            //  statement.
            //
            dbstmtDiscs.Reset();
            dbstmtDiscs.AddColStrBinding(L"Label", 255);
            dbstmtDiscs.AddColBinding(L"Number", tCIDDBase::ECppTypes::Int4);
            dbstmtDiscs.AddColBinding(L"TitleId", tCIDDBase::ECppTypes::Int4);
            dbstmtDiscs.AddColBinding(L"LocType", tCIDDBase::ECppTypes::Int4);
            dbstmtDiscs.AddColStrBinding(L"Location", 255);
            dbstmtDiscs.AddColStrBinding(L"Slot", 255);

            m_strTmp1 = strTitleQueryBase;
            m_strTmp1.AppendFormatted(c4TitleId);
            m_strTmp1.Append(L"'");
            dbstmtDiscs.Execute(m_strTmp1);

            //
            //  We wil remember the collection ids we add. At the end, we'll
            //  go back, if we found art, and set it on the collections. Since
            //  we may not find art until we search the disc paths below, we
            //  have to just wait till the end.
            //
            fcolColds.RemoveAll();

            while (dbstmtDiscs.bFetch())
            {
                // Create a per-disc unique id based on the title set's
                m_strTmp1 = mtsCur.strUniqueId();
                m_strTmp1.AppendFormatted(c4DiscNum++);

                //
                //  Set up the location info. The location types in MyMovies
                //  are:
                //
                //  1. A directory
                //  2. A file path
                //  3. Offline DVD disc
                //  4. A changer/slot
                //  5. In a media changer
                //
                //  We can only handle 1, 2, and 4 currently.
                //
                const tCIDLib::TInt4 i4MMLocType = dbstmtDiscs.i4ColVal(3);
                tCQCMedia::ELocTypes eLocType;
                switch(i4MMLocType)
                {
                    case 1 :
                    case 2 :
                        eLocType = tCQCMedia::ELocTypes::FileCol;
                        break;

                    case 4 :
                        eLocType = tCQCMedia::ELocTypes::Changer;
                        break;

                    default :
                        eLocType = tCQCMedia::ELocTypes::Count;
                        break;
                };

                // If we can't handle this location type, then skip it
                if (eLocType == tCQCMedia::ELocTypes::Count)
                    continue;

                // Add a new collection to this title set and the database
                TMediaCollect* pmcolCur = new TMediaCollect
                (
                    dbstmtDiscs.strColVal(0)
                    , m_strTmp1
                    , c4CurYear
                    , eLocType
                    , m_eMediaType
                );
                TJanitor<TMediaCollect> janCol(pmcolCur);

                // Store the location info
                m_strTmp1 = dbstmtDiscs.strColVal(4);
                if (eLocType == tCQCMedia::ELocTypes::Changer)
                {
                    m_strTmp1.Append(kCIDLib::chPeriod);
                    m_strTmp1.Append(dbstmtDiscs.strColVal(5));
                }
                pmcolCur->strLocInfo(m_strTmp1);

                // The artist is the same as at the title level
                pmcolCur->strArtist(mtsCur.strArtist());

                // Store the cast and studio strings we created above
                pmcolCur->strCast(strCast);
                pmcolCur->strLabel(strStudio);

                //
                //  If V19 or greater, then just take the 0th element in
                //  the actors list, since it's onyl for this title. Else,
                //  find the first one from this title.
                //
                if (i4DBVersion >= 19)
                {
                    if (!colActors.bIsEmpty())
                        pmcolCur->strLeadActor(colActors[0].m_strValue);
                }
                 else
                {
                    pitemTmp = pitemFind(colActors, c4TitleId);
                    if (pitemTmp)
                        pmcolCur->strLeadActor(pitemTmp->m_strValue);
                }

                // Store the duration we set up above
                pmcolCur->c4Duration(c4CurDuration);

                // Store the other stuff from the preloaded tables
                if (pitemAspect)
                    pmcolCur->strAspectRatio(pitemAspect->m_strValue);
                if (pitemDescr)
                    pmcolCur->strDescr(pitemDescr->m_strValue);
                if (pitemMediaFmt)
                    pmcolCur->strMediaFormat(pitemMediaFmt->m_strValue);
                if (pitemRating)
                    pmcolCur->strRating(pitemRating->m_strValue);
                if (pitemUPC)
                    pmcolCur->strUPC(pitemUPC->m_strValue);

                // Add this one to the all movies category
                pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMovies);

                // And to any other categories we found
                const tCIDLib::TCard4 c4CatCnt = fcolCatIds.c4ElemCount();
                for (tCIDLib::TCard4 c4CatInd = 0; c4CatInd < c4CatCnt; c4CatInd++)
                    pmcolCur->bAddCategory(fcolCatIds[c4CatInd]);

                // Make a unique id that's the collection id plus a suffix
                m_strTmp1 = pmcolCur->strUniqueId();
                m_strTmp1.Append(L"-1");

                // Add a single chapter item that represents the whole movie contents
                TMediaItem* pmitemChapter = new TMediaItem
                (
                    L"Movie"
                    , m_strTmp1
                    , TString::strEmpty()
                    , m_eMediaType
                );
                pmitemChapter->strArtist(pmcolCur->strArtist());

                // Add the item, and then add it to the collection
                m_pmdbLoading->c2AddItem(pmitemChapter);
                pmcolCur->bAddItem(*pmitemChapter);

                //
                //  Now we can add the collection since it's updated, and we can
                //  then add it to the title now that its id is set.
                //
                m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());
                mtsCur.bAddCollect(*pmcolCur);

                // Remember this one
                fcolColds.c4AddElement(pmcolCur->c2Id());

                //
                //  If we haven't found art yet, see if there's any in this guy's
                //  path, if it's a file or directory.
                //
                if (!pmimgArt && (eLocType == tCQCMedia::ELocTypes::FileCol))
                {
                    pmimgArt = pmimgLoadDiscArt
                    (
                        mtsCur.strUniqueId()
                        , pmcolCur->strLocInfo()
                        , i4MMLocType == 2
                    );
                }
            }

            // If we found any art, add it to all of the collections
            if (pmimgArt && !fcolColds.bIsEmpty())
            {
                const tCIDLib::TCard4 c4ColCnt = fcolColds.c4ElemCount();
                for (tCIDLib::TCard4 c4ColInd = 0; c4ColInd < c4ColCnt; c4ColInd++)
                {
                    // Update the art id on this collection
                    m_pmdbLoading->SetArtId
                    (
                        fcolColds[c4ColInd]
                        , tCQCMedia::EMediaTypes::Movie
                        , kCIDLib::False
                        , pmimgArt->c2Id()
                    );
                }
            }
        }

        //
        //  And we have to look up some stuff in the TitlePersonal table,
        //  which stores the user rating and date added info we need. The
        //  key is the unique title key, which we stored as the sequence
        //  number.
        //
        {
            TDBDirStatement dbstmtPersonal(dbconMM, L"Query Personal");
            dbstmtPersonal.Reset();

            //
            //  Add column bindings that we'll need, and execute a select
            //  statement.
            //
            dbstmtPersonal.Reset();
            dbstmtPersonal.AddColDateBinding(L"DateAdded", L"%(Y)-%(M)-%(D)");
            dbstmtPersonal.AddColBinding(L"Rating", tCIDDBase::ECppTypes::Int4);

            m_strTmp1 = strPersonalQueryBase;
            m_strTmp1.AppendFormatted(c4TitleId);
            m_strTmp1.Append(L"'");
            dbstmtPersonal.Execute(m_strTmp1);

            // We should only get one hit
            if (dbstmtPersonal.bFetch())
            {
                tCIDLib::TInt4 i4URating = dbstmtPersonal.i4ColVal(1);
                if (i4URating < tCIDLib::TInt4(kCQCMedia::c4MinUserRating))
                    i4URating = tCIDLib::TInt4(kCQCMedia::c4MinUserRating);
                else if (i4URating > tCIDLib::TInt4(kCQCMedia::c4MaxUserRating))
                    i4URating = tCIDLib::TInt4(kCQCMedia::c4MaxUserRating);

                mtsCur.c4UserRating(tCIDLib::TCard4(i4URating));
                mtsCur.enctAdded(dbstmtPersonal.tmDateColVal(0).enctTime());
            }
        }

        // Put the new title set info back now
        m_pmdbLoading->UpdateTitle(mtsCur);
    }

    // And we are done, so close the connection
    dbconMM.Disconnect();

    //
    //  Make a pass back through and, for any title sets that we couldn't
    //  load any collections for, remove them. We don't have to remove any
    //  referenced stuff, because by definition the ones we are removing
    //  can't have any.
    //
    //  We'll set the sequence number at this point as well by just assigning
    //  a running counter to the titles we keept.
    //
    m_c4TitlesDone = m_pmdbLoading->c4TitleSetCnt(m_eMediaType);
    tCIDLib::TCard4 c4Index = 0;
    tCIDLib::TCard4 c4SeqNum = 1;
    while (c4Index < m_c4TitlesDone)
    {
        const TMediaTitleSet& mtsCur = m_pmdbLoading->mtsAt(m_eMediaType, c4Index);
        if (!mtsCur.c4ColCount())
        {
            m_pmdbLoading->bRemoveObject
            (
                m_eMediaType, tCQCMedia::EDataTypes::TitleSet, mtsCur.c2Id()
            );

            // And reduce the titles done count
            m_c4TitlesDone--;
        }
         else
        {
            c4Index++;
            m_pmdbLoading->SetSeqNum(mtsCur.c2Id(), m_eMediaType, c4SeqNum++);
        }
    }
}


//
//  Called for each title. We check to see if there is an image under this id
//  in the standard location. In newer versions it might not be there, in which
//  case the pmimgLoadDiscArt() method will be called for each disc to try that.
//
//  The image id is the unique title id, and we will use the title id if we try
//  to load from the media disc directory as well, so it should remain consistent.
//
const TMediaImg* TMyMoviesRepoLoader::pmimgLoadArt(const TTblItem& itemImg)
{
    // If there is already an image with this id, return that
    {
        const TMediaImg* pmimgFind = m_pmdbLoading->pmimgByUniqueId
        (
            tCQCMedia::EMediaTypes::Movie, itemImg.m_strValue, kCIDLib::False
        );
        if (pmimgFind)
            return pmimgFind;
    }

    // Create a new image object and give it the unique id that we got from MM.
    TMediaImg* pmimgRet = new TMediaImg
    (
        tCQCMedia::EMediaTypes::Movie, TString::strEmpty(), TString::strEmpty()
    );
    TJanitor<TMediaImg> janImg(pmimgRet);

    pmimgRet->strUniqueId(itemImg.m_strValue);

    // Try the large large first
    tCIDLib::TBoolean bGotLarge = kCIDLib::False;
    tCIDLib::TBoolean bGotThumb = kCIDLib::False;
    try
    {
        // First try to get the medium sized one
        m_pathTmp1 = m_strImagePath;
        m_pathTmp1.AddLevel(L"med");
        m_pathTmp1.Append(itemImg.m_strValue);
        m_pathTmp1.AppendExt(L"jpg");

        if (!TFileSys::bExists(m_pathTmp1))
        {
            // Try another variation
            m_pathTmp1 = m_strImagePath;
            m_pathTmp1.AddLevel(L"medium");
            m_pathTmp1.Append(itemImg.m_strValue);
            m_pathTmp1.AppendExt(L"jpg");


            // Not there so try the big one
            if (!TFileSys::bExists(m_pathTmp1))
            {
                m_pathTmp1 = m_strImagePath;
                m_pathTmp1.AddLevel(itemImg.m_strValue);
                m_pathTmp1.AppendExt(L"jpg");
            }
        }

        if (TFileSys::bExists(m_pathTmp1))
        {
            pmimgRet->SetPersistentId(itemImg.m_strValue, tCQCMedia::ERArtTypes::LrgCover);
            pmimgRet->SetArtPath(m_pathTmp1, tCQCMedia::ERArtTypes::LrgCover);
            bGotLarge = kCIDLib::True;
        }
    }

    catch(...)
    {
    }

    try
    {
        // There are a couple variations, so try them all
        m_pathTmp1 = m_strImagePath;
        m_pathTmp1.AddLevel(L"thumb");
        m_pathTmp1.Append(itemImg.m_strValue);
        m_pathTmp1.AppendExt(L"jpg");

        if (!TFileSys::bExists(m_pathTmp1))
        {
            // Not that one, try this one
            m_pathTmp1 = m_strImagePath;
            m_pathTmp1.AddLevel(L"tmb");
            m_pathTmp1.Append(itemImg.m_strValue);
            m_pathTmp1.AppendExt(L"jpg");
        }

        if (TFileSys::bExists(m_pathTmp1))
        {
            pmimgRet->SetArtPath(m_pathTmp1, tCQCMedia::ERArtTypes::SmlCover);
            bGotThumb = kCIDLib::True;
        }

        //
        //  Either way, set the persistent id for the small cover to the same thing
        //  since it uses the same MM image id. And, if we didn't find the thumb
        //  images, we'll create one from the large art.
        //
        pmimgRet->SetPersistentId(itemImg.m_strValue, tCQCMedia::ERArtTypes::SmlCover);
    }

    catch(...)
    {
    }

    // If we got either image, then let's store it and return it
    if (bGotLarge || bGotThumb)
    {
        m_pmdbLoading->c2AddImage(janImg.pobjOrphan());
        return pmimgRet;
    }
    return 0;
}


//
//  If the art is not stored in the common cover art directory under the title id,
//  then we will look for it in the per-disc directories to see if we can find
//  something there. It can be a file or directory type. If file, we have to remove
//  the file name to get the path to search.
//
const TMediaImg*
TMyMoviesRepoLoader::pmimgLoadDiscArt(  const   TString&            strTitleId
                                        , const TString&            strPath
                                        , const tCIDLib::TBoolean   bFileType)
{
    // See if we can find an image in the passed path
    m_pathTmp1 = strPath;
    if (bFileType)
        m_pathTmp1.bRemoveNameExt();

    // First we'll look for a specific one we know is good
    m_pathTmp1.AddLevel(L"mymovies-front");
    m_pathTmp1.AppendExt(L"jpg");

    if (!TFileSys::bExists(m_pathTmp1))
    {
        // Not there, so try to find any jpg file
        m_pathTmp1.bRemoveNameExt();
        m_pathTmp1.AddLevel(L"*");
        m_pathTmp1.AppendExt(L"jpg");

        // If not, then give up
        TFindBuf fndbToFill;
        if (!TFileSys::bExists(m_pathTmp1, fndbToFill))
            return 0;

        m_pathTmp1 = fndbToFill.pathFileName();
    }

    //
    //  We have to pre-load this image since we have to generate a unique id for
    //  it from the image data.
    //
    tCIDLib::TBoolean bAllBytes = kCIDLib::False;
    tCIDLib::TCard4 c4IdBytes = 0;
    try
    {
        TBinaryFile flImg(m_pathTmp1);
        flImg.Open
        (
            tCIDLib::EAccessModes::Excl_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        // Read up to the max bytes
        c4IdBytes = flImg.c4ReadBuffer
        (
            m_mbufTmp
            , kMyMoviesRepoS::c4MaxPerIdBytes
            , tCIDLib::EAllData::OkIfNotAll
        );

        // Remember if we read the whole file
        bAllBytes = tCIDLib::TCard4(flImg.c8CurSize()) == c4IdBytes;
    }

    catch(TError& errToCatch)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Couldn't do it, so give up
        return 0;
    }

    // Create the persistent id
    facCQCMedia().CreatePersistentId(m_mbufTmp, c4IdBytes, m_strTmp2);

    //
    //  Create a new image object and give it the title's unique id. WE can set
    //  the large art path, but not the thumb.
    //
    TMediaImg* pmimgRet = new TMediaImg
    (
        tCQCMedia::EMediaTypes::Movie, m_pathTmp1, TString::strEmpty()
    );
    TJanitor<TMediaImg> janImg(pmimgRet);
    pmimgRet->strUniqueId(strTitleId);

    //
    //  Set the persistent id for the large and small art. They can be the same
    //  since it's the same image.
    //
    pmimgRet->SetPersistentId(m_strTmp2, tCQCMedia::ERArtTypes::LrgCover);
    pmimgRet->SetPersistentId(m_strTmp2, tCQCMedia::ERArtTypes::SmlCover);

    //
    //  If we read the whole file, then go ahead and cache it since we have it all
    //  already.
    //
    if (bAllBytes)
        pmimgRet->SetArt(c4IdBytes, m_mbufTmp, tCQCMedia::ERArtTypes::LrgCover);

    // Ok, now we can add it and return it
    m_pmdbLoading->c2AddImage(janImg.pobjOrphan());
    return pmimgRet;
}

