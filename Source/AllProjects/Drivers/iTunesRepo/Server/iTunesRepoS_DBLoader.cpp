//
// FILE NAME: iTunesRepoS_DBLoader.cpp
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
RTTIDecls(TiTunesRepoLoader,TThread)



// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace iTunesRepoS_DBLoader
{
    //  The names of some things we look for in the XML
    // -----------------------------------------------------------------------
    const TString   strElem_Key(L"key");
    const TString   strElem_Int(L"integer");
    const TString   strElem_String(L"string");

    const TString   strElem_Library(L"/plist/dict");
    const TString   strElem_LibValKey(L"/plist/dict/key");
    const TString   strElem_LibValInt(L"/plist/dict/integer");
    const TString   strElem_LibValStr(L"/plist/dict/string");

    const TString   strElem_PList(L"/plist/dict/array/dict");
    const TString   strElem_PListKey(L"/plist/dict/array/dict/key");
    const TString   strElem_PListInt(L"/plist/dict/array/dict/integer");
    const TString   strElem_PListStr(L"/plist/dict/array/dict/string");
    const TString   strElem_PListValKey(L"/plist/dict/array/dict/array/dict/key");
    const TString   strElem_PListValInt(L"/plist/dict/array/dict/array/dict/integer");
    const TString   strElem_PListValStr(L"/plist/dict/array/dict/array/dict/string");

    const TString   strElem_Tracks(L"/plist/dict/dict");
    const TString   strElem_Track(L"/plist/dict/dict/dict");
    const TString   strElem_TrackKey(L"/plist/dict/dict/key");
    const TString   strElem_TrackValKey(L"/plist/dict/dict/dict/key");
    const TString   strElem_TrackValInt(L"/plist/dict/dict/dict/integer");
    const TString   strElem_TrackValStr(L"/plist/dict/dict/dict/string");

    const TString   strText_M4P(L"M4P");
    const TString   strText_MP3(L"MP3");
    const TString   strText_MP4(L"M4A");
    const TString   strText_WMA(L"WMA");

    const TString   strText_AudioBooks(L"AudioBooks");
    const TString   strText_Library(L"Library");
    const TString   strText_Movies(L"Movies");
    const TString   strText_Music(L"Music");
    const TString   strText_MusicVideos(L"Music Videos");

    const TString   strText_Album(L"Album");
    const TString   strText_AlbumArtist(L"Album Artist");
    const TString   strText_Artist(L"Artist");
    const TString   strText_DiscCnt(L"Disc Count");
    const TString   strText_DiscNum(L"Disc Number");
    const TString   strText_Genre(L"Genre");
    const TString   strText_Integer(L"integer");
    const TString   strText_LibraryUniqueId(L"Library Persistent ID");
    const TString   strText_LocInfo(L"Location");
    const TString   strText_PlayLists(L"Playlists");
    const TString   strText_Name(L"Name");
    const TString   strText_String(L"string");
    const TString   strText_TotalTime(L"Total Time");
    const TString   strText_Tracks(L"Tracks");
    const TString   strText_TrackID(L"Track ID");
    const TString   strText_TrackCnt(L"Track Count");
    const TString   strText_TrackNum(L"Track Number");
    const TString   strText_TrackUniqueId(L"Persistent ID");
    const TString   strText_Year(L"Year");
}


// ---------------------------------------------------------------------------
//  Local helper functions
// ---------------------------------------------------------------------------

//
//  A comparator to let us do binary searches and sorts of our alt id sorted
//  list of items.
//
static tCIDLib::ESortComps
eCompItemsByAltId(const tCIDLib::TCard4& c41, const TMediaItem& mitem2)
{
    const tCIDLib::TCard4 c42 = mitem2.c4AltId();
    if (c41 < c42)
        return tCIDLib::ESortComps::FirstLess;
    if (c41 > c42)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}

static tCIDLib::ESortComps
eCompItems(const TMediaItem& mitem1, const TMediaItem& mitem2)
{
    const tCIDLib::TCard4 c41 = mitem1.c4AltId();
    const tCIDLib::TCard4 c42 = mitem2.c4AltId();
    if (c41 < c42)
        return tCIDLib::ESortComps::FirstLess;
    if (c41 > c42)
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//   CLASS: TiTunesRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TiTunesRepoLoader: Public, static methods
// ---------------------------------------------------------------------------

//
//  This will build up a path name based on the passed library and track
//  unique ids.
//
tCIDLib::TVoid
TiTunesRepoLoader::BuildImgPath(const   TString&    strLibraryUniqueID
                                , const TString&    strTrackUniqueId
                                ,       TPathStr&   pathToFill)
{
    pathToFill = m_strDataPath;
    pathToFill.AddLevel(L"Album Artwork");
    pathToFill.AddLevel(L"Local");
    pathToFill.AddLevel(strLibraryUniqueID);

    //
    //  We now have to take the last three digits of the track id to their
    //  decimal equiv and each one becomes a directory layer.
    //
    tCIDLib::TCard4 c4Index = strTrackUniqueId.c4Length() - 1;

    tCIDLib::TCard1 c1Val = TString::c1ToHex(strTrackUniqueId[c4Index--]);
    m_strTmp.Clear();
    if (c1Val < 10)
        m_strTmp.Append(kCIDLib::chDigit0);
    m_strTmp.AppendFormatted(c1Val);
    pathToFill.AddLevel(m_strTmp);

    c1Val = TString::c1ToHex(strTrackUniqueId[c4Index--]);
    m_strTmp.Clear();
    if (c1Val < 10)
        m_strTmp.Append(kCIDLib::chDigit0);
    m_strTmp.AppendFormatted(c1Val);
    pathToFill.AddLevel(m_strTmp);

    c1Val = TString::c1ToHex(strTrackUniqueId[c4Index--]);
    m_strTmp.Clear();
    if (c1Val < 10)
        m_strTmp.Append(kCIDLib::chDigit0);
    m_strTmp.AppendFormatted(c1Val);
    pathToFill.AddLevel(m_strTmp);

    // And finally library unique id, a dash, and the track id
    pathToFill.AddLevel(m_strFld_LibraryUniqueID);
    pathToFill.Append(kCIDLib::chHyphenMinus);
    pathToFill.Append(strTrackUniqueId);
    pathToFill.AppendExt(L"itc");
}


// ---------------------------------------------------------------------------
//  TiTunesRepoLoader: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  NOTE:   We set the initial status to failed. If the caller has an error
//          after creating us, but before the thread starts, his polling
//          for the end of load will see the error status and clean up.
//          Otherwise, in that error scenario, he'd never see the load end
//          since it never actually started. We'll reset the status once we
//          actually are started up.
//
TiTunesRepoLoader::TiTunesRepoLoader(const  TString&            strMoniker
                                    , const TString&            strDataPath
                                    , const tCIDLib::TBoolean   bUseAlbumArtist) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_bUseAlbumArtist(bUseAlbumArtist)
    , m_c2MissingArtId(0)
    , m_c2PListArtId(0)
    , m_c2NextCatId(kCQCMedia::c2CatId_UserStart)
    , m_c2NextColId(1)
    , m_c2NextImgId(1)
    , m_c2NextItemId(1)
    , m_c2NextTitleId(1)
    , m_c4TitlesDone(0)
    , m_c4TracksIgnored(0)
    , m_colCatNames(128)
    , m_colImageMap(109, TStringKeyOps, &TKeyValuePair::strExtractKey)
    , m_eFilePos(EFilePos_None)
    , m_eStatus(tCQCMedia::ELoadStatus::Failed)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_mbufArt(0x10000, 0x400000)
    , m_mbufTmp(2048, 0x10000)
    , m_pmdbLoading(0)
    , m_strAllMusicCat(kMedMsgs::midCat_AllMusic, facCQCMedia())
    , m_strDataPath(strDataPath)
    , m_strMoniker(strMoniker)
{
    //
    //  Create a DTD validator and plug it into our parser. We have to tell
    //  the validator about the parser as well.
    //
    TDTDValidator* pvalNew = new TDTDValidator(&m_xprsThis);
    m_xprsThis.pxvalValidator(pvalNew);

    // Set ourself as handlers on the parser
    m_xprsThis.pmxevDocEvents(this);
    m_xprsThis.pmxevErrorEvents(this);
}

TiTunesRepoLoader::~TiTunesRepoLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TiTunesRepoLoader::CompleteLoad()
{
    try
    {
        // Make sure we are in the loaded or failed states
        if ((m_eStatus != tCQCMedia::ELoadStatus::Loaded)
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
TiTunesRepoLoader::eStatus( tCIDLib::TCard4&    c4TitlesDone
                            , tCIDLib::TCard4&  c4TitlesIgnored) const
{
    // Return the status and the count of titles
    c4TitlesDone = m_c4TitlesDone;
    c4TitlesIgnored = m_c4TracksIgnored;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TiTunesRepoLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


tCIDLib::TVoid TiTunesRepoLoader::ResetDocument()
{
    // We don't need to do anything here
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid
TiTunesRepoLoader::StartLoad(       TMediaDB* const pmdbToLoad
                            , const TPathStr&       pathXMLFile)
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

    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        faciTunesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Starting database load"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    try
    {
        // Store the incoming info for later use during callbacks
        m_pmdbLoading = pmdbToLoad;
        m_pathXMLFile = pathXMLFile;

        // Reset any of our per-load data
        m_eFilePos = EFilePos_None;
        m_c4TitlesDone = 0;
        m_c4TracksIgnored = 0;
        m_colCatNames.RemoveAll();
        m_c2NextCatId = kCQCMedia::c2CatId_UserStart;
        m_c2NextColId = 1;
        m_c2NextItemId = 1;
        m_c2NextTitleId = 1;
        m_strFld_LibraryUniqueID.Clear();

        // Flush the media database that we are going to load
        m_pmdbLoading->Reset();

        // Add an 'All Music' category that all stuff can be put into
        m_pmdbLoading->AddCategory
        (
            new TMediaCat
            (
                tCQCMedia::EMediaTypes::Music
                , m_strAllMusicCat
                , kCQCMedia::c2CatId_AllMusic
            )
        );


        //
        //  See if there is any artwok in the image repo for missing or playst
        //  collections. If we get them, then add them as images and remember
        //  their ids. We'll assign this same image to any playlists or collects
        //  that we don't find any art for. That is much more efficient. We
        //  pass the image repository path as the art path, but that's ok since
        //  we are pre-loading them, so we'll never actually use the path for
        //  these other than in error messages.
        //
        tCIDLib::TCard4 c4Sz;
        c4Sz = facCQCMedia().c4QuerySpecialImg(m_mbufArt, tCQCMedia::ESpcImages::Missing);
        if (c4Sz)
        {
            m_c2MissingArtId = m_c2NextImgId;
            TMediaImg* pmimgMissing = new TMediaImg
            (
                tCQCMedia::EMediaTypes::Music
                , m_c2NextImgId++
                , L"\\User\\CQC\\Media\\MissingArt"
                , TString::strEmpty()
            );
            pmimgMissing->SetArt(c4Sz, m_mbufArt);
            m_pmdbLoading->AddImage(pmimgMissing);
        }

        c4Sz = facCQCMedia().c4QuerySpecialImg(m_mbufArt, tCQCMedia::ESpcImages::PlayList);
        if (c4Sz)
        {
            m_c2PListArtId = m_c2NextImgId;
            TMediaImg* pmimgPL = new TMediaImg
            (
                tCQCMedia::EMediaTypes::Music
                , m_c2NextImgId++
                , L"\\User\\CQC\\Media\\PListArt"
                , TString::strEmpty()
            );
            pmimgPL->SetArt(c4Sz, m_mbufArt);
            m_pmdbLoading->AddImage(pmimgPL);
        }

        // Start ourself running in another thread
        Start();
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
            faciTunesRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"iTunes repository loader got an exception during init"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(...)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"iTunes repository loader got an unknown exception during init"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


//
//  Called by the main thread to make sure we are cleaned up when it needs
//  to shutdown. We set the status to failed so that if he hasn't already
//  checked the status and is asking us to show down early, we'll show up
//  as failed.
//
tCIDLib::TVoid TiTunesRepoLoader::Shutdown()
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
//  TiTunesRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TiTunesRepoLoader::DocCharacters(const  TString&            strChars
                                , const tCIDLib::TBoolean   bIsCDATA
                                , const tCIDLib::TBoolean   bIsIgnorable
                                , const tCIDXML::ELocations eLocation)
{
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the start element, so it's safe to do this.
    //
    if ((m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_LibValInt)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_LibValStr)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListInt)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListStr)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListValInt)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListValStr)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_TrackValInt)
    ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_TrackValStr))
    {
        m_strLastValue.Append(strChars);
    }
     else if ((m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_LibValKey)
          ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_TrackValKey)
          ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListKey)
          ||  (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PListValKey))
    {
        m_strLastKey.Append(strChars);
    }
}


tCIDLib::EExitCodes TiTunesRepoLoader::eProcess()
{
    // We can now set our status to loading before we let the caller go
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Let the calling thread go
    Sync();

    try
    {
        //
        //  Ignore the DTD, which is not local so we don't want to bother
        //  with it.
        //
        const tCIDXML::EParseOpts   eOpts = tCIDLib::eOREnumBits
        (
            tCIDXML::EParseOpts::IgnoreDTD, tCIDXML::EParseOpts::IgnoreBadChars
        );

        const tCIDXML::EParseFlags  eFlags = tCIDLib::eOREnumBits
        (
            tCIDXML::EParseFlags::Chars
            , tCIDXML::EParseFlags::Tags
            , tCIDXML::EParseFlags::Topology
        );

        // Kick off the parsing
        m_xprsThis.ParseRootEntity(m_pathXMLFile, eOpts, eFlags);

        // Handle any post-processing
        PostProcessing();

        //
        //  Ok, it's done successfully, so call the load completion method
        //  on the media database, so that it can do any stuff it has to wait
        //  until all the data is available to do.
        //
        m_pmdbLoading->LoadComplete();

        m_eStatus = tCQCMedia::ELoadStatus::Loaded;
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
                , m_pathXMLFile
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
                , m_pathXMLFile
            );
        }
    }

    //
    //  Make sure that the status didn't end up stuck on loading somehow,
    //  since that would make the driver wait forever.
    //
    if ((m_eStatus != tCQCMedia::ELoadStatus::Failed)
    &&  (m_eStatus != tCQCMedia::ELoadStatus::Loaded))
    {
        faciTunesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kiTunesErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TiTunesRepoLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        faciTunesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Entered EndDocument"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  This is called when the XML parser sees an end tag. Because of the way
//  this XML file is structured, we have to do most of the triggering of
//  things off of the end of tags (no attributes are used, everything is
//  in element text.)
//
tCIDLib::TVoid TiTunesRepoLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    const TString& strQName = xdeclElem.strFullName();

    //
    //  If we are at the end of one of the tag/value pairs, then we need
    //  to store away the data we collected.
    //
    if ((strQName == iTunesRepoS_DBLoader::strText_Integer)
    ||   (strQName == iTunesRepoS_DBLoader::strText_String))
    {
        ProcessKey(xdeclElem);
    }
     else if (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_LibValKey)
    {
        // Watch for the start of the playlists and tracks sections
        if (m_strLastKey == iTunesRepoS_DBLoader::strText_PlayLists)
        {
            m_eFilePos = EFilePos_PlayLists;
            m_strLastKey.Clear();
            m_strLastValue.Clear();
            m_colPLNames.RemoveAll();
            m_colPListList.RemoveAll();
        }
         else if (m_strLastKey == iTunesRepoS_DBLoader::strText_Tracks)
        {
            //
            //  We have to have gotten a library id by now. If not, then
            //  the file is invalid.
            //
            if (m_strFld_LibraryUniqueID.bIsEmpty())
            {
                if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
                {
                    faciTunesRepoS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"No unique library id element found. Can't load."
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
            }

            m_eFilePos = EFilePos_Tracks;
            m_strLastKey.Clear();
            m_strLastValue.Clear();
        }
    }
     else if (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_Track)
    {
        const tCIDLib::TCard4 c4Line = m_xprsThis.c4CurLine();
        try
        {
            ProcessTrack();
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
                faciTunesRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Got an exception while processing track ending on line %(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c4Line)
                );
            }
        }

        catch(...)
        {
            if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
            {
                faciTunesRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Got an exception while processing track ending on line %(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(c4Line)
                );
            }
        }
    }

    // Pop our XML path (which verifies the nesting for us)
    PopXMLPath(xdeclElem);
}


tCIDLib::TVoid
TiTunesRepoLoader::HandleXMLError( const    tCIDLib::TErrCode   errcToPost
                                    , const tCIDXML::EErrTypes  eType
                                    , const TString&            strText
                                    , const tCIDLib::TCard4     c4CurColumn
                                    , const tCIDLib::TCard4     c4CurLine
                                    , const TString&            strSystemId)
{
    // Log the message and set the failure flag
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        faciTunesRepoS().LogMsg
        (
            strSystemId
            , c4CurLine
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
    }
    m_eStatus = tCQCMedia::ELoadStatus::Failed;
}


tCIDLib::TVoid TiTunesRepoLoader::StartDocument(const TXMLEntitySrc&)
{
    //
    //  Don't really need to do anything. We are initialized before each
    //  call to do a parse, so our data is already ready. If we are in
    //  high verbosity level, log a trace message.
    //
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        faciTunesRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Entered StartDocument"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid
TiTunesRepoLoader::StartTag(        TXMLParserCore&     xprsSrc
                            , const TXMLElemDecl&       xdeclElem
                            , const tCIDLib::TBoolean   bEmpty
                            , const TVector<TXMLAttr>&  colAttrList
                            , const tCIDLib::TCard4     c4AttrCnt)
{
    //
    //  Check for a shutdown request. This load can take a while and we
    //  want to shutdown quickly if asked. We just throw the failed enum
    //  value, so that it will slip through all the way back without
    //  getting caught and we know why we got there.
    //
    if (bCheckShutdownRequest())
        throw tCQCMedia::ELoadStatus::Failed;

    // Get the raw element name first
    const TString& strQName = xdeclElem.strFullName();

    // Clear the last key/values if starting a new key
    if (strQName == iTunesRepoS_DBLoader::strElem_Key)
    {
        m_strLastKey.Clear();
        m_strLastValue.Clear();
    }

    // Push the new element onto the current full path 'stack'
    m_strCurXMLPath.Append(L'/');
    m_strCurXMLPath.Append(strQName);

    if (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_Library)
    {
        m_eFilePos = EFilePos_Library;
    }
     else if (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_Track)
    {
        //
        //  If it's the start of a new track, then clear the track related
        //  temp field storage members in preparation for another track.
        //
        m_strFld_Album.Clear();
        m_strFld_AlbumArtist.Clear();
        m_strFld_Artist.Clear();
        m_strFld_DiscCnt.Clear();
        m_strFld_DiscNum.Clear();
        m_strFld_Genre.Clear();
        m_strFld_LocInfo.Clear();
        m_strFld_Name.Clear();
        m_strFld_TotalTime.Clear();
        m_strFld_TrackID.Clear();
        m_strFld_TrackCnt.Clear();
        m_strFld_TrackNum.Clear();
        m_strFld_TrackUniqueID.Clear();
        m_strFld_Year.Clear();
    }
     else if (m_strCurXMLPath == iTunesRepoS_DBLoader::strElem_PList)
    {
        //
        //  It's the start of a new playlist, so add a new item to the
        //  the name and and id list vectors. These are empty and will
        //  be filled in as we get the info.
        //
        m_colPLNames.objAdd(TString::strEmpty());
        m_colPListList.objAdd(TPLIds());
    }
}



// ---------------------------------------------------------------------------
//  TiTunesRepoLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Pops the last element off of the current XML path and verifies that
//  that element is the one that we got that's ending. We consider any
//  errors here to be fatal and exit.
//
tCIDLib::TVoid TiTunesRepoLoader::PopXMLPath(const TXMLElemDecl& xdeclEnding)
{
    // If it's empty, then we've underflowed
    if (m_strCurXMLPath.bIsEmpty())
    {
        faciTunesRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kiTunesErrs::errcDB_BadNesting
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Find the last separator
    tCIDLib::TCard4 c4Ofs;
    if (!m_strCurXMLPath.bLastOccurrence(L'/', c4Ofs))
    {
        faciTunesRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kiTunesErrs::errcDB_BadNesting
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Get the bit after the separate to a tmp and compare to the passed element
    m_strCurXMLPath.CopyOutSubStr(m_strTmp, c4Ofs + 1);
    if (m_strTmp != xdeclEnding.strFullName())
    {
        faciTunesRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kiTunesErrs::errcDB_BadNesting2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , xdeclEnding.strFullName()
            , m_strTmp
        );
    }

    // Looks ok, cap the path at the divider
    m_strCurXMLPath.CapAt(c4Ofs);
}


//
//  This is called at the end to do some post-processing work. We do a
//  few things. We handle setting up some collection and title set info
//  that we don't get from iTunes, using the info we did get on the
//  individual tracks. And we create all of the play lists based on the
//  playlist indices we stored during parsing.
//
//  We do the playlists first, so that they also get subjected to the
//  other processing.
//
tCIDLib::TVoid TiTunesRepoLoader::PostProcessing()
{
    const tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Music;

    //
    //  Deal with the playlists. First we go through a build a map to the
    //  items and sort by the Track ID, which we stored in the Alt Id field.
    //
    const tCIDLib::TCard4 c4ItemCnt = m_pmdbLoading->c4ItemCnt(eType);
    tCQCMedia::TItemIdList colByTID(tCIDLib::EAdoptOpts::NoAdopt, c4ItemCnt);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ItemCnt; c4Index++)
        colByTID.Add(&m_pmdbLoading->mitemAt(eType, c4Index));
    colByTID.Sort(&eCompItems);

    //
    //  Let's create a playlists category if it doesn't already exist. We'll
    //  put all of the playlists into that category.
    //
    TString strPLCat(iTunesRepoS_DBLoader::strText_PlayLists);
    strPLCat.ToUpper();
    TMediaCat* pmcatPLs = m_pmdbLoading->pmcatByUniqueId(eType, strPLCat, kCIDLib::False);
    if (!pmcatPLs)
    {
        pmcatPLs = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Music
            , iTunesRepoS_DBLoader::strText_PlayLists
            , m_c2NextCatId++
        );
        m_pmdbLoading->AddCategory(pmcatPLs);
    }
    const tCIDLib::TCard2 c2PLCatId = pmcatPLs->c2Id();

    //
    //  Now we can run through all of the playlists we stored and for each
    //  one we can look up the referenced items and create new collections
    //  for them.
    //
    const tCIDLib::TCard4 c4PLCount = m_colPLNames.c4ElemCount();
    TString strPLId;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PLCount; c4Index++)
    {
        // If we've maxed on item ids, then we have to stop
        if (m_c2NextItemId == kCIDLib::c2MaxCard)
            break;

        const TString& strPLName = m_colPLNames[c4Index];
        if (strPLName.bIsEmpty())
            continue;

        // Watch for some that we know we don't want
        if ((strPLName == iTunesRepoS_DBLoader::strText_AudioBooks)
        ||  (strPLName == iTunesRepoS_DBLoader::strText_Library)
        ||  (strPLName == iTunesRepoS_DBLoader::strText_Movies)
        ||  (strPLName == iTunesRepoS_DBLoader::strText_Music)
        ||  (strPLName == iTunesRepoS_DBLoader::strText_MusicVideos))
        {
            continue;
        }

        //
        //  See if this one has any entries. If not, skip it. Or, if it
        //  has over 5K entries, skip it also for practical purposes.
        //
        TPLIds& mcolPLIds = m_colPListList[c4Index];
        const tCIDLib::TCard4 c4TIDCnt = mcolPLIds.c4ElemCount();
        if (!c4TIDCnt || (c4TIDCnt > 5000))
        {
            if (c4TIDCnt && (m_eVerbose >= tCQCKit::EVerboseLvls::Medium))
            {
                faciTunesRepoS().LogMsg
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

        // Build up the unique Id
        strPLId = L"PlayList_";
        strPLId.Append(strPLName);

        // Make the name isn't a dup. If so, skip it
        if (m_pmdbLoading->pmtsByUniqueId(eType, strPLId, kCIDLib::False))
            continue;

        // Create a new title set and add it to the database
        m_c4TitlesDone++;
        facCQCMedia().bMakeSortTitle(strPLName, m_strTmp);
        TMediaTitleSet* pmtsPL = new TMediaTitleSet
        (
            strPLName
            , strPLId
            , m_strTmp
            , m_c2NextTitleId++
            , eType
        );
        m_pmdbLoading->AddTitle(pmtsPL);

        //
        //  Create a collection and add it to the title set and database.
        //  Set the artist to various artists, which it always will be
        //  for a playlist.
        //
        strPLId.Append(L"_1");
        TMediaCollect* pmcolPL = new TMediaCollect
        (
            strPLName
            , strPLId
            , m_c2NextColId++
            , 0
            , tCQCMedia::ELocTypes::FileItem
            , eType
        );
        pmcolPL->strArtist(L"Various Artists");
        pmcolPL->c2ArtId(m_c2PListArtId);
        pmcolPL->bIsPlayList(kCIDLib::True);
        m_pmdbLoading->AddCollect(pmcolPL);
        pmtsPL->bAddCollect(*pmcolPL);

        // Add this guy to the all music category and the playlists category
        pmcolPL->bAddCategory(kCQCMedia::c2CatId_AllMusic);
        pmcolPL->bAddCategory(c2PLCatId);

        //
        //  Now loop through all of the ids we stored for this playlist,
        //  look up the original items and create new items to go into the
        //  playlist collection. We remember the latest year and use that
        //  as the playlist year, and we accumulate the durations to set
        //  on the collection.
        //
        const tCIDLib::TCard4 c4BaseIDLen = strPLId.c4Length();
        tCIDLib::TCard4 c4Year = 0;
        tCIDLib::TCard4 c4ColDur = 0;
        for (tCIDLib::TCard4 c4TIDInd = 0; c4TIDInd < c4TIDCnt; c4TIDInd++)
        {
            // If we've maxed on item ids, then we have to stop
            if (m_c2NextItemId == kCIDLib::c2MaxCard)
                break;

            const tCIDLib::TCard4 c4CurTID = mcolPLIds[c4TIDInd];
            tCIDLib::TCard4 c4AtInd;
            TMediaItem* pmitemCur = colByTID.pobjKeyedBinarySearch<tCIDLib::TCard4>
            (
                c4CurTID, &eCompItemsByAltId, c4AtInd
            );

            // If not found, then skip it
            if (!pmitemCur)
                continue;

            //
            //  We got one, so fill in a new item based on that, but using
            //  a new id.
            //
            strPLId.CapAt(c4BaseIDLen);
            strPLId.Append(kCIDLib::chUnderscore);
            strPLId.AppendFormatted(c4TIDInd + 1);

            TMediaItem* pmitemNew = new TMediaItem
            (
                pmitemCur->strName()
                , strPLId
                , m_c2NextItemId++
                , pmitemCur->strLocInfo()
                , eType
            );
            pmitemNew->strArtist(pmitemCur->strArtist());
            pmitemNew->c4Duration(pmitemCur->c4Duration());
            c4ColDur += pmitemCur->c4Duration();

            // Add it to the database and the collection
            m_pmdbLoading->AddItem(pmitemNew);
            pmcolPL->bAddItem(*pmitemNew);
        }

        // Store the stuff we acculuated from the items above
        pmcolPL->c4Duration(c4ColDur);
    }

    //
    //  Deal with the collection level artist name if we are not in
    //  album artist mode. If all of the item artist names are the same,
    //  we set the collection level artist to that. Else we set it to
    //  Various Artists.
    //
    if (!m_bUseAlbumArtist)
    {
        const tCIDLib::TCard4 c4ColCnt = m_pmdbLoading->c4CollectCnt(eType);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
        {
            //
            //  If all of the artist names are the same, then we just use
            //  that. Else, set it to "Various Artists"
            //
            TMediaCollect& mcolCur = m_pmdbLoading->mcolAt(eType, c4Index);
            const tCIDLib::TCard4 c4ItemCnt = mcolCur.c4ItemCount();
            if (c4ItemCnt)
            {
                TMediaItem* pmitemPrev = &mcolCur.mitemAt(*m_pmdbLoading, 0);
                TMediaItem* pmitemCur = 0;
                tCIDLib::TCard4 c4IIndex = 1;
                for (; c4IIndex < c4ItemCnt; c4IIndex++)
                {
                    pmitemCur = &mcolCur.mitemAt(*m_pmdbLoading, c4IIndex);
                    if (pmitemCur->strArtist() != pmitemPrev->strArtist())
                        break;
                    pmitemPrev = pmitemCur;
                }

                if (c4IIndex == c4ItemCnt)
                {
                    // They are all the same
                    pmitemCur = &mcolCur.mitemAt(*m_pmdbLoading, 0);
                    mcolCur.strArtist(pmitemCur->strArtist());
                }
                 else
                {
                    mcolCur.strArtist(L"Various Artists");
                }
            }
             else
            {
                mcolCur.strArtist(L"Unknown");
            }

            //
            //  Make another pass and add up the durations of all the
            //  items and set that as the collection level duration.
            //
            tCIDLib::TCard4 c4ColDuration = 0;
            for (tCIDLib::TCard4 c4IIndex = 0; c4IIndex < c4ItemCnt; c4IIndex++)
                c4ColDuration += mcolCur.mitemAt(*m_pmdbLoading, c4IIndex).c4Duration();
            mcolCur.c4Duration(c4ColDuration);
        }
    }

    //
    //  And do the same for title sets wrt to their collections. In this
    //  case we always done it, since we don't get any title set level
    //  artist info in either case.
    //
    const tCIDLib::TCard4 c4SetCnt = m_pmdbLoading->c4TitleSetCnt(eType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4SetCnt; c4Index++)
    {
        TMediaTitleSet& mtsCur = m_pmdbLoading->mtsAt(eType, c4Index);
        const tCIDLib::TCard4 c4ColCnt = mtsCur.c4ColCount();
        TMediaCollect* pmcolPrev = &mtsCur.mcolAt(*m_pmdbLoading, 0);
        TMediaCollect* pmcolCur = 0;
        tCIDLib::TCard4 c4IIndex = 1;
        for (; c4IIndex < c4ColCnt; c4IIndex++)
        {
            pmcolCur = &mtsCur.mcolAt(*m_pmdbLoading, c4IIndex);
            if (pmcolCur->strArtist() != pmcolPrev->strArtist())
                break;
            pmcolPrev = pmcolCur;
        }

        if (c4IIndex == c4ColCnt)
        {
            // They are all the same
            pmcolCur = &mtsCur.mcolAt(*m_pmdbLoading, 0);
            mtsCur.strArtist(pmcolCur->strArtist());
        }
         else
        {
            mtsCur.strArtist(L"Various Artists");
        }
    }

    //
    //  If we have a missing art id, then loop through all of the collections
    //  and set that as the art id for any that we didn't find art for.
    //
    if (m_c2MissingArtId)
    {
        const tCIDLib::TCard4 c4ColCnt = m_pmdbLoading->c4CollectCnt(eType);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
        {
            TMediaCollect& mcolCur = m_pmdbLoading->mcolAt(eType, c4Index);
            if (!mcolCur.c2ArtId())
                mcolCur.c2ArtId(m_c2MissingArtId);
        }
    }
}


//
//  When we see the end of one of the value elements (integer, string, etc...)
//  this is called. We look at the last key value stored and the current
//  section of the file we are in and interpret the information accordingly.
//
tCIDLib::TVoid TiTunesRepoLoader::ProcessKey(const TXMLElemDecl& xdeclElem)
{
    if (m_strLastKey.bIsEmpty())
    {
        //
        //  If there was no key we cared about before the value element,
        //  then just clear the key and value accumulators and return.
        //
        m_strLastKey.Clear();
        m_strLastValue.Clear();
        return;
    }

    //
    //  Based on where we are in the file, we look for different stuff
    //
    if (m_eFilePos == EFilePos_Library)
    {
        if (m_strLastKey == iTunesRepoS_DBLoader::strText_LibraryUniqueId)
            m_strFld_LibraryUniqueID = m_strLastValue;
    }
     else if (m_eFilePos == EFilePos_PlayLists)
    {
        const tCIDLib::TCard4 c4PLIndex = m_colPLNames.c4ElemCount() - 1;
        if (m_strLastKey == iTunesRepoS_DBLoader::strText_Name)
        {
            m_colPLNames[c4PLIndex] = m_strLastValue;
        }
         else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TrackID)
        {
            tCIDLib::TCard4 c4TrackId = 0;
            m_strLastValue.bToCard4(c4TrackId, tCIDLib::ERadices::Dec);
            m_colPListList[c4PLIndex].c4AddElement(c4TrackId);
        }
    }
     else if (m_eFilePos == EFilePos_Tracks)
    {
        if (m_strLastKey == iTunesRepoS_DBLoader::strText_Album)
            m_strFld_Album = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_AlbumArtist)
            m_strFld_AlbumArtist = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_Artist)
            m_strFld_Artist = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_DiscCnt)
            m_strFld_DiscCnt = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_DiscNum)
            m_strFld_DiscNum = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_Genre)
            m_strFld_Genre = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_LocInfo)
            m_strFld_LocInfo = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_Name)
            m_strFld_Name = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TotalTime)
            m_strFld_TotalTime = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TrackCnt)
            m_strFld_TrackCnt = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TrackID)
            m_strFld_TrackID = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TrackNum)
            m_strFld_TrackNum = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_TrackUniqueId)
            m_strFld_TrackUniqueID = m_strLastValue;
        else if (m_strLastKey == iTunesRepoS_DBLoader::strText_Year)
            m_strFld_Year = m_strLastValue;
    }
}


//
//  We have a track's worth o' info, so process it. We'll create a new
//  titleset and/or collection as required.
//
tCIDLib::TVoid TiTunesRepoLoader::ProcessTrack()
{
    //
    //  OK, let's process this title. Make sure we got any required info
    //  and that it's a file type and a type of file we deal with. If not,
    //  then we have to skip this one.
    //
    if (m_strFld_Album.bIsEmpty()
    ||  m_strFld_TrackUniqueID.bIsEmpty()
    ||  m_strFld_TrackNum.bIsEmpty()
    ||  m_strFld_LocInfo.bIsEmpty())
    {
        m_c4TracksIgnored++;
        return;
    }

    // If we are in album artist mode, then that has to be set
    if (m_bUseAlbumArtist)
    {
        if (m_strFld_AlbumArtist.bIsEmpty())
        {
            m_c4TracksIgnored++;
            return;
        }

        // If there's no per-track artist, then set it to the album artist
        if (m_strFld_Artist.bIsEmpty())
            m_strFld_Artist = m_strFld_AlbumArtist;
    }
     else if (m_strFld_Artist.bIsEmpty())
    {
        // Set the per-track artist to unknown
        m_strFld_Artist = L"Unknown";
    }

    // Assume disc 1 if not provided
    if (m_strFld_DiscNum.bIsEmpty())
        m_strFld_DiscNum = L"1";

    // The track and disc number must be convertable to numbers
    tCIDLib::TCard4 c4DiscNum = 0;
    tCIDLib::TCard4 c4TrackNum = 0;
    if (!m_strFld_DiscNum.bToCard4(c4DiscNum, tCIDLib::ERadices::Dec)
    ||  !m_strFld_TrackNum.bToCard4(c4TrackNum, tCIDLib::ERadices::Dec))
    {
        m_c4TracksIgnored++;
        return;
    }

    // Parse the location URL
    try
    {
        m_urlLoc.Reset(m_strFld_LocInfo, tCIDSock::EQualified::Full);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && (m_eVerbose >= tCQCKit::EVerboseLvls::High))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        m_c4TracksIgnored++;
        return;
    }

    // It has to be file based
    if (m_urlLoc.eProto() != tCIDSock::EProtos::File)
    {
        m_c4TracksIgnored++;
        return;
    }

    //
    //  Get the actual path part out, and convert to a local file path
    //  format. We have to watch for UNC type names, which are going to look
    //  like file://localhost//SomeShare/blah. This is really not a legal
    //  URL anyway probably, but when we get this back out we lose the first
    //  slash and need to add it back.
    //
    const tCIDLib::TBoolean bUNCPath
    (
        m_strFld_LocInfo.bStartsWithI(L"file://localhost//")
    );

    m_urlLoc.strFullURL(m_tcvtExpand, m_mbufTmp).CopyOutSubStr(m_strFld_LocInfo, 17);
    if (bUNCPath)
        m_strFld_LocInfo.Prepend(L"/");
    m_strFld_LocInfo.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chBackSlash);

    //
    //  Pull out the extension and make sure it's a type that we would want
    //  to deal with.
    //
    tCIDLib::TCard4 c4Offset;
    if (!m_strFld_LocInfo.bLastOccurrence(kCIDLib::chPeriod, c4Offset)
    ||  (c4Offset >= m_strFld_LocInfo.c4Length()))
    {
        m_c4TracksIgnored++;
        return;
    }
    TString strFileExt;
    m_strFld_LocInfo.CopyOutSubStr(strFileExt, c4Offset + 1);
    strFileExt.ToUpper();
    if ((strFileExt != iTunesRepoS_DBLoader::strText_M4P)
    &&  (strFileExt != iTunesRepoS_DBLoader::strText_MP3)
    &&  (strFileExt != iTunesRepoS_DBLoader::strText_MP4)
    &&  (strFileExt != iTunesRepoS_DBLoader::strText_WMA))
    {
        m_c4TracksIgnored++;
        return;
    }

    // They are one based, so we reduce them by 1 for an index
    if (!c4DiscNum || !c4TrackNum)
    {
        m_c4TracksIgnored++;
        return;
    }
    c4DiscNum--;
    c4TrackNum--;

    // The year isn't required. If it fails to convert it'll be zero
    tCIDLib::TCard4 c4Year;
    m_strFld_Year.bToCard4(c4Year, tCIDLib::ERadices::Dec);

    //
    //  Build up the id we'll use for the title set. We don't get an
    //  album or disc level id, so we generate a hash. We use either just
    //  the album title, or the album artist plus the title.
    //
    if (m_bUseAlbumArtist)
    {
        m_strTmpID = m_strFld_AlbumArtist;
        m_strTmpID.Append(kCIDLib::chSpace);
        m_strTmpID.Append(m_strFld_Album);
    }
     else
    {
        m_strTmpID = m_strFld_Album;
    }
    m_mdigId.Digest(m_mhashId, m_strTmpID);
    m_mhashId.FormatToStr(m_strTmpID);

    //
    //  See if this guy is in the database yet. If so, then get a pointer
    //  to it so that we can update it. Else create a new one.
    //
    tCIDLib::TBoolean bNewTitle = kCIDLib::False;
    TMediaTitleSet* pmtsCur = 0;
    pmtsCur = m_pmdbLoading->pmtsByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, m_strTmpID, kCIDLib::False
    );

    if (!pmtsCur)
    {
        // Bump the title count
        m_c4TitlesDone++;

        // A new one, so create a new title set and add it to the database
        bNewTitle = kCIDLib::True;
        facCQCMedia().bMakeSortTitle(m_strFld_Album, m_strTmp);
        pmtsCur = new TMediaTitleSet
        (
            m_strFld_Album
            , m_strTmpID
            , m_strTmp
            , m_c2NextTitleId++
            , tCQCMedia::EMediaTypes::Music
        );
        m_pmdbLoading->AddTitle(pmtsCur);
    }

    //
    //  See if the indicated disc number exists yet. If not, then create
    //  it and add it to the database and the title set.
    //
    m_strTmpID.Append(kCIDLib::chHyphenMinus);
    m_strTmpID.AppendFormatted(c4DiscNum + 1);
    TMediaCollect* pmcolCur;
    pmcolCur = m_pmdbLoading->pmcolByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, m_strTmpID, kCIDLib::False
    );

    if (!pmcolCur)
    {
        pmcolCur = new TMediaCollect
        (
            m_strFld_Album
            , m_strTmpID
            , m_c2NextColId++
            , c4Year
            , tCQCMedia::ELocTypes::FileItem
            , tCQCMedia::EMediaTypes::Music
        );
        m_pmdbLoading->AddCollect(pmcolCur);
        pmtsCur->bAddCollect(*pmcolCur);

        //
        //  If we are in album artist mode, then break out the album
        //  artist and set it on the collection. Replace the circumflexes
        //  for historical reasons.
        //
        if (m_bUseAlbumArtist)
        {
            m_strFld_AlbumArtist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);
            pmcolCur->strArtist(m_strFld_AlbumArtist);
        }

        // Add it to the all music category
        pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMusic);
    }
     else
    {
        //
        //  If this new is non-zero and before the earliest year we've stored
        //  for this collection so far, take it.
        //
        if (c4Year && (c4Year < pmcolCur->c4Year()))
            pmcolCur->c4Year(c4Year);
    }

    //
    //  If we've not gotten any art yet, then let's see if we can find
    //  any.
    //
    if (!pmcolCur->c2ArtId())
    {
        // First we look for an exact match based on the ids
        BuildImgPath(m_strFld_LibraryUniqueID, m_strFld_TrackUniqueID, m_pathTmp);

        //
        //  If this file exists, we go with that. If not, let's see if we
        //  have a match in our image map.
        //
        TMediaImg* pmimgArt = 0;
        if (TFileSys::bExists(m_pathTmp, tCIDLib::EDirSearchFlags::AllFiles))
        {
            //
            //  We got a match so see if this image is already in the
            //  database, else add it.
            //
            pmimgArt = m_pmdbLoading->pmimgByPath
            (
                tCQCMedia::EMediaTypes::Music, m_pathTmp, kCIDLib::False
            );

            if (!pmimgArt)
            {
                //
                //  We just have one image, so we set the thumb path to
                //  an empty string. Our driver will scale this image if
                //  needed to create the small image.
                //
                pmimgArt = new TMediaImg
                (
                    tCQCMedia::EMediaTypes::Music
                    , m_c2NextImgId++
                    , m_pathTmp
                    , TString::strEmpty()
                );
                m_pmdbLoading->AddImage(pmimgArt);
            }
        }

        // If still nothing, it could be embedded, so check that
        if (!pmimgArt)
        {
            // See which extractor we should use
            TCIDMetaExtractor* pmetaxToUse = 0;
            if ((strFileExt == L"MP3") || (strFileExt == L"WMA"))
                pmetaxToUse = &m_metexWMA;
            else
                pmetaxToUse = &m_metexAAC;

            //
            //  Check it first. The meta extraction stuff is really piggy to
            //  crank up only to find that the file isn't there.
            //
            if (TFileSys::bExists(m_strFld_LocInfo))
            {
                try
                {
                    tCIDLib::TCard4 c4ArtSz;
                    if (pmetaxToUse->bExtractImage(m_strFld_LocInfo, m_mbufArt, c4ArtSz))
                    {
                        //
                        //  We found some, so create a new media image object
                        //  and store the art on it. So in this case, we are
                        //  pre-loading the artwork.
                        //
                        pmimgArt = new TMediaImg
                        (
                            tCQCMedia::EMediaTypes::Music
                            , m_c2NextImgId++
                            , m_strFld_LocInfo
                            , TString::strEmpty()
                        );
                        m_pmdbLoading->AddImage(pmimgArt);
                        pmimgArt->SetArt(c4ArtSz, m_mbufArt);

                        #if CID_DEBUG_ON
                        #if defined(EXTR_ART)
                        TPathStr pathTmp(m_strFld_LocInfo);
                        pathTmp.bRemoveExt();
                        pathTmp.AppendExt(L"jpg");
                        TBinaryFile flOutArt(pathTmp);
                        flOutArt.Open
                        (
                            tCIDLib::EAccessModes::Excl_Write
                            , tCIDLib::ECreateActs::CreateAlways
                            , tCIDLib::EFilePerms::Default
                            , tCIDLib::EFileFlags::SequentialScan
                        );
                        flOutArt.c4WriteBuffer(m_mbufArt, c4ArtSz);
                        #endif
                        #endif
                    }
                }

                catch(TError& errToCatch)
                {
                    if (!errToCatch.bLogged()
                    &&  (m_eVerbose >= tCQCKit::EVerboseLvls::Medium))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                }
            }
        }

        // If we found something, set the id on this collection
        if (pmimgArt)
            pmcolCur->c2ArtId(pmimgArt->c2Id());
    }

    //
    //  Scan through the genres in the genre list we found for this round
    //  and see if they exist yet. Create them if not, and add them to the
    //  list of categories or this collection.
    //
    //  The unique id for the categories just gets set to the same value
    //  as the name, so by checking for whether the UID exists already
    //  we keep only unique names in the category list.
    //
    if (!m_strFld_Genre.bIsEmpty())
    {
        TMediaCat*       pmcatCur;
        TStringTokenizer stokCats(&m_strFld_Genre, L",");
        while (stokCats.bGetNextToken(m_strTmp2))
        {
            m_strTmp2.StripWhitespace();
            m_strTmp = m_strTmp2;
            m_strTmp.ToUpper();
            pmcatCur = m_pmdbLoading->pmcatByUniqueId
            (
                tCQCMedia::EMediaTypes::Music, m_strTmp, kCIDLib::False
            );

            if (!pmcatCur)
            {
                pmcatCur = new TMediaCat
                (
                    tCQCMedia::EMediaTypes::Music, m_strTmp2, m_c2NextCatId++
                );
                m_pmdbLoading->AddCategory(pmcatCur);
            }

            // Add this one to the collection if not already
            pmcolCur->bAddCategory(*pmcatCur);
        }
    }

    //
    //  OK, and now let's add an item to the collection for this track. We
    //  use the iTunes provided unique id for the track.
    //
    TMediaItem* pmitemNew = new TMediaItem
    (
        m_strFld_Name
        , m_strFld_TrackUniqueID
        , m_c2NextItemId++
        , m_strFld_LocInfo
        , tCQCMedia::EMediaTypes::Music
    );

    //
    //  Since we could get them out of order, we use the optional load order
    //  field to store the track number so that we can insert new ones at
    //  the correct spot in the collection.
    //
    pmitemNew->c4LoadOrder(c4TrackNum);

    //
    //  Add it to the database so that someone is responsible for it while we
    //  do the remaining setup. We need to get it set up more before we add
    //  it to the collection.
    //
    m_pmdbLoading->AddItem(pmitemNew);

    //
    //  We store the track id for later use when we need to come back
    //  and look them up to create the playlists. We use the 'alt id'
    //  field to store this, which is for storing repository specific
    //  numeric ids. If it fails to convert to a number, we just don't
    //  store zero and therefor it won't get into any playlist.
    //
    tCIDLib::TCard4 c4TrackId = 0;
    m_strFld_TrackID.bToCard4(c4TrackId, tCIDLib::ERadices::Dec);
    pmitemNew->c4AltId(c4TrackId);

    //
    //  Break out the per-item artist name into first and last names. Once
    //  that's done, we then replace any ^ characters with spaces for
    //  historical reasons.
    //
    m_strFld_Artist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);
    pmitemNew->strArtist(m_strFld_Artist);

    // Convert the track duration and store it
    tCIDLib::TCard4 c4Seconds = 0;
    if (m_strFld_TotalTime.bToCard4(c4Seconds, tCIDLib::ERadices::Dec))
        c4Seconds /= kCIDLib::c4OneSecond;
    pmitemNew->c4Duration(c4Seconds);

    //
    //  Figure out where to add the track in the collection. They are not
    //  necessarily in order.
    //
    const tCIDLib::TCard4 c4ItemCnt = pmcolCur->c4ItemCount();
    tCIDLib::TCard4 c4Index = 0;
    for (; c4Index < c4ItemCnt; c4Index++)
    {
        //
        //  Break out when we see an existing track with a larger track
        //  number. Since we are adding them in order, this will keep the
        //  newly added ones sorted.
        //
        const TMediaItem* pmitemCur = m_pmdbLoading->pmitemById
        (
            tCQCMedia::EMediaTypes::Music
            , pmcolCur->c2ItemIdAt(c4Index)
            , kCIDLib::False
        );
        if (pmitemCur->c4LoadOrder() > c4TrackNum)
            break;
    }

    if (c4Index == c4ItemCnt)
        c4Index = kCIDLib::c4MaxCard;
    pmcolCur->bAddItem(*pmitemNew, c4Index);
}

