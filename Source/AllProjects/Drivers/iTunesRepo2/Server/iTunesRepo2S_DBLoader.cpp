//
// FILE NAME: iTunesRepoS_DBLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/20/2009
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
#include    "iTunesRepo2S.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRepo2Loader,TThread)



// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace iTunesRepo2S_DBLoader
{
    // -----------------------------------------------------------------------
    //  Some constant strings, to avoid the overhead during the many thousands
    //  of comparisons that will occur.
    // -----------------------------------------------------------------------
    const TString   strElem_AlbumGUID(L"AlbumGUID");
    const TString   strElem_AlbumID(L"AlbumStringGUID");
    const TString   strElem_AlbumName(L"Album");
    const TString   strElem_Artist(L"Artist");
    const TString   strElem_ArtPath(L"Artwork");
    const TString   strElem_Duration(L"Duration");
    const TString   strElem_Genre(L"Genre");
    const TString   strElem_Library(L"Library");
    const TString   strElem_LocInfo(L"Location");
    const TString   strElem_Name(L"Name");
    const TString   strElem_Number(L"Number");
    const TString   strElem_PlayList(L"Playlist");
    const TString   strElem_PlayLists(L"iTunesPlaylists");
    const TString   strElem_Repo(L"iTunesRepository");
    const TString   strElem_Track(L"Track");
    const TString   strElem_TrackID(L"TrackID");

    const TString   strText_AudioBooks(L"AudioBooks");
    const TString   strText_Library(L"Library");
    const TString   strText_Movies(L"Movies");
    const TString   strText_Music(L"Music");
    const TString   strText_MusicVideos(L"Music Videos");


    // -----------------------------------------------------------------------
    //  The size of the temporary IO buffers we create on the stack for the
    //  simple msg IO we do with the tray app to get the XML created.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4IOBufSz = 1024;

}


// ---------------------------------------------------------------------------
//   CLASS: TiTunesRepo2Loader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TiTunesRepo2Loader: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  NOTE:   We set the initial status to failed. If the caller has an error
//          after creating us, but before the thread starts, his polling
//          for the end of load will see the error status and clean up.
//          Otherwise, in that error scenario, he'd never see the load end
//          since it never actually started. We'll reset the status once we
//          actually are started up.
//
TiTunesRepo2Loader::
TiTunesRepo2Loader( const   TString&                    strMoniker
                    , const TString&                    strXMLFile
                    ,       TiTunesRepo2SDriver* const  psdrvOwner) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_c2MissingArtId(0)
    , m_c2PListArtId(0)
    , m_c2NextCatId(kCQCMedia::c2CatId_UserStart)
    , m_c2NextColId(1)
    , m_c2NextImgId(1)
    , m_c2NextItemId(1)
    , m_c4TitlesDone(0)
    , m_c4TracksIgnored(0)
    , m_colCatNames(128)
    , m_colItemIDList(tCIDLib::EAdoptOpts::NoAdopt, 1024UL)
    , m_eStatus(tCQCMedia::ELoadStatus::Failed)
    , m_eFilePart(EFilePart_None)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_mbufArt(0x10000, 0x400000)
    , m_mbufTmp(2048, 0x10000)
    , m_pmdbLoading(0)
    , m_psdrvOwner(psdrvOwner)
    , m_strAllMusicCat(kMedMsgs::midCat_AllMusic, facCQCMedia())
    , m_strMoniker(strMoniker)
    , m_pathXMLFile(strXMLFile)
{
    // Tell the item id list it can hold nulls
    m_colItemIDList.bAllowNulls(kCIDLib::True);
}

TiTunesRepo2Loader::~TiTunesRepo2Loader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TiTunesRepo2Loader::CompleteLoad()
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
//  We give a status string back to the driver for display, and a load status
//  enumerated value.
//
tCQCMedia::ELoadStatus
TiTunesRepo2Loader::eStatus(TString& strStatus) const
{
    TLocker lockrSync(&m_mtxSync);
    strStatus = m_strStatus;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TiTunesRepo2Loader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


tCIDLib::TVoid TiTunesRepo2Loader::ResetDocument()
{
    // We don't need to do anything here
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid
TiTunesRepo2Loader::StartLoad(          TMediaDB* const pmdbToLoad
                                , const TIPEndPoint&    ipepTrayApp)
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
        faciTunesRepo2S().LogMsg
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
        // Store the incoming info for later use
        m_ipepTrayApp = ipepTrayApp;
        m_pmdbLoading = pmdbToLoad;

        // Reset any of our per-load data
        m_c4TitlesDone = 0;
        m_c4TracksIgnored = 0;
        m_colCatNames.RemoveAll();
        m_c2NextCatId = kCQCMedia::c2CatId_UserStart;
        m_c2NextColId = 1;
        m_c2NextItemId = 1;

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
            faciTunesRepo2S().LogMsg
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
            faciTunesRepo2S().LogMsg
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
tCIDLib::TVoid TiTunesRepo2Loader::Shutdown()
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
//  TiTunesRepo2Loader: Protected, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::EExitCodes TiTunesRepo2Loader::eProcess()
{
    // We can now set our status to loading before we let the caller go
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Let the calling thread go
    Sync();

    try
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_ContactingTray
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_ipepTrayApp
            );
        }

        //
        //  Try to connect to the tray app and send the message to get him
        //  to generate the XML. We tell him where to put it.
        //
        TClientStreamSocket sockTray(tCIDSock::ESockProtos::TCP, m_ipepTrayApp);
        SendMsg(sockTray, L"GenerateXML", L"Path", m_pathXMLFile);

        // If verbose enough, lot that we are seiting for the reply
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_WaitStartReply
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_psdrvOwner->strMoniker()
            );
        }

        //
        //  And now wait for the response.
        //
        TString strMsg;
        TString strValue;
        if (!bGetMsg(sockTray, strMsg, strValue))
        {
            m_eStatus = tCQCMedia::ELoadStatus::Failed;
            sockTray.c4Shutdown();
            sockTray.Close();

            if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
            {
                faciTunesRepo2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kiTunes2Msgs::midStatus_NoStartMsg
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_psdrvOwner->strMoniker()
                );
            }
            return tCIDLib::EExitCodes::Normal;
        }

        // Make sure it's an ack. If a nack, then we failed
        if (strMsg == L"NAK")
        {
            m_eStatus = tCQCMedia::ELoadStatus::Failed;
            sockTray.c4Shutdown();
            sockTray.Close();

            if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
            {
                strMsg.Cut(0, 6);
                faciTunesRepo2S().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kiTunes2Msgs::midStatus_StartNAK
                    , strMsg
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_psdrvOwner->strMoniker()
                );
            }
            return tCIDLib::EExitCodes::Normal;
        }

        // If verbose enough, lot that we are seiting for the reply
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_WaitEndReply
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_psdrvOwner->strMoniker()
            );
        }

        //
        //  Now we have to wait for the completion message. This could
        //  take some time. As we wait, we will get progress messages. If
        //  one does arrive within 10 seconds, we give up.
        //
        while (kCIDLib::True)
        {
            if (!bGetMsg(sockTray, strMsg, strValue))
            {
                m_eStatus = tCQCMedia::ELoadStatus::Failed;
                sockTray.c4Shutdown();
                sockTray.Close();

                if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
                {
                    faciTunesRepo2S().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kiTunes2Msgs::midStatus_NoResponse
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_psdrvOwner->strMoniker()
                    );
                }
                return tCIDLib::EExitCodes::Normal;
            }

            // Make sure it's an ack. If a nack, then we failed
            if (strMsg == L"NAK")
            {
                // If a NAK then we failed
                m_eStatus = tCQCMedia::ELoadStatus::Failed;
                sockTray.c4Shutdown();
                sockTray.Close();

                if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
                {
                    strMsg.Cut(0, 6);
                    faciTunesRepo2S().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kiTunes2Msgs::midStatus_EndNAK
                        , strMsg
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_psdrvOwner->strMoniker()
                    );
                }
                return tCIDLib::EExitCodes::Normal;
            }
             else if (strMsg == L"ACK")
            {
                // If an ACK, we are done
                break;
            }
             else if (strMsg == L"Status")
            {
                // Give the driver the status. We have to sync for this
                TLocker lockrSync(&m_mtxSync);
                m_strStatus = strValue;
            }
             else
            {
                // It's some unknown message
                m_eStatus = tCQCMedia::ELoadStatus::Failed;
                sockTray.c4Shutdown();
                sockTray.Close();

                if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
                {
                    if (strMsg.c4Length() > 20)
                        strMsg.CapAt(20);
                    faciTunesRepo2S().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kiTunes2Msgs::midStatus_UnknownReply
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_psdrvOwner->strMoniker()
                        , strMsg
                    );
                }
                return tCIDLib::EExitCodes::Normal;
            }
        }

        // We can close the socket now
        sockTray.c4Shutdown();
        sockTray.Close();

        // Build up the full URL to the file
        TPathStr pathFile(L"file://localhost/");
        pathFile.Append(m_pathXMLFile);
        pathFile.AddTrailingSeparator();
        pathFile.bReplaceChar(L'\\', L'/');
        pathFile.Append(L"iTunesRepository.xml");

        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_FileGenerated
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , pathFile
            );
        }

        const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::IgnoreBadChars;
        const tCIDXML::EParseFlags  eFlags = tCIDLib::eOREnumBits
        (
            tCIDXML::EParseFlags::Tags, tCIDXML::EParseFlags::Topology
        );

        //
        //  Kick off the parsing. We create a DTD validator and plug it into
        //  our parser. We have to tell the validator about the parser as well.
        //
        TXMLParserCore xprsFile;
        TDTDValidator* pvalNew = new TDTDValidator(&xprsFile);
        xprsFile.pxvalValidator(pvalNew);

        // Set ourself as handlers on the parser
        xprsFile.pmxevDocEvents(this);
        xprsFile.pmxevErrorEvents(this);
        xprsFile.ParseRootEntity(pathFile, eOpts, eFlags);

        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_PostProcessing
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        CIDAssert
        (
            m_colItemIDList.c4ElemCount()
              == m_pmdbLoading->c4ItemCnt(tCQCMedia::EMediaTypes::Music)
            , L"Item id list size is not equal to database item count"
        );

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
        faciTunesRepo2S().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kiTunes2Errs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TiTunesRepo2Loader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        faciTunesRepo2S().LogMsg
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
tCIDLib::TVoid TiTunesRepo2Loader::EndTag(const TXMLElemDecl& xdeclElem)
{
    const TString& strQName = xdeclElem.strFullName();

}


tCIDLib::TVoid
TiTunesRepo2Loader::HandleXMLError( const   tCIDLib::TErrCode   errcToPost
                                    , const tCIDXML::EErrTypes  eType
                                    , const TString&            strText
                                    , const tCIDLib::TCard4     c4CurColumn
                                    , const tCIDLib::TCard4     c4CurLine
                                    , const TString&            strSystemId)
{
    // Log the message and set the failure flag
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        faciTunesRepo2S().LogMsg
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


tCIDLib::TVoid TiTunesRepo2Loader::StartDocument(const TXMLEntitySrc&)
{
    //
    //  Don't really need to do anything. We are initialized before each
    //  call to do a parse, so our data is already ready. If we are in
    //  high verbosity level, log a trace message.
    //
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        faciTunesRepo2S().LogMsg
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
TiTunesRepo2Loader::StartTag(       TXMLParserCore&     xprsSrc
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

    //
    //  If it's an Item element, then we have to do a little preperatory
    //  work. Else, it's got to be a field so we set up for the field data
    //  to come.
    //
    const TString& strQName = xdeclElem.strFullName();
    if (strQName == iTunesRepo2S_DBLoader::strElem_Repo)
    {
        m_eFilePart = EFilePart_Repository;
    }
     else if (strQName == iTunesRepo2S_DBLoader::strElem_Library)
    {
        m_eFilePart = EFilePart_Library;
    }
     else if (strQName == iTunesRepo2S_DBLoader::strElem_PlayLists)
    {
        m_eFilePart = EFilePart_PlayLists;
    }
     else if (strQName == iTunesRepo2S_DBLoader::strElem_PlayList)
    {
        m_eFilePart = EFilePart_PlayList;
        AddNewPlayList(xdeclElem, colAttrList, c4AttrCnt);
    }
     else if (strQName == iTunesRepo2S_DBLoader::strElem_Track)
    {
        // We have to be in the library or playlist parts
        if (m_eFilePart == EFilePart_Library)
        {
            ProcessTrack(xprsSrc, xdeclElem, colAttrList, c4AttrCnt);
        }
         else if (m_eFilePart == EFilePart_PlayList)
        {
            ProcessPLTrack(xprsSrc, xdeclElem, colAttrList, c4AttrCnt);
        }
         else
        {
            // This isn't something legal
        }
    }
     else
    {
        // This isn't something legal

    }
}



// ---------------------------------------------------------------------------
//  TiTunesRepo2Loader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when we see the start element of a playlist. We add an entry in
//  the playlist name list, and an entry in the playlist id list, which will
//  subsequently get filled in when we see track items. These are just pust
//  into the top-most id entry.
//
//  If no name, then we leave it empty. It'll get ignored later when we
//  go back and build up the lists.
//
tCIDLib::TVoid
TiTunesRepo2Loader::AddNewPlayList( const   TXMLElemDecl&       xdeclElem
                                    , const TVector<TXMLAttr>&  colAttrList
                                    , const tCIDLib::TCard4     c4AttrCnt)
{
    // Loop through the attrs and find what we need
    TString strName;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AttrCnt; c4Index++)
    {
        const TXMLAttr& xattrCur = colAttrList[c4Index];
        if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Name)
            strName = xattrCur.strValue();
    }

    // Add the new playlist name and an empty id list
    m_colPLNames.objAdd(strName);
    m_colPListList.objAdd(TPLIds());
}


//
//  A message getter helper method. We pull in the message and transcode it
//  into a string. We don't use the bGetASCIIStopStopMsg method because we
//  want to discard the start/end markers, and it's easier to do that if we
//  read to a buffer and transcode ourself.
//
//  The simple commands it sends us are pseudo XML. We use an XML tree
//  parser to parse it, to avoid having to do any parsing ourself. We have
//  to put in an XML Decl of our own and the bring in the transcoded text
//  after it.
//
tCIDLib::TBoolean
TiTunesRepo2Loader::bGetMsg(TClientStreamSocket&    sockTray
                            , TString&              strCmdToFill
                            , TString&              strValToFill)
{
    // Read in a message. If that fails, return false
    const tCIDLib::TCard4 c4Len = m_psdrvOwner->c4GetStartStopMsg
    (
        sockTray, 30000, 2, 3, m_mbufTmp.c4MaxSize(), m_mbufTmp
    );

    //
    //  No even remotely reasonable message could be less than this, and it
    //  insures we cannot underflow below.
    //
    if (c4Len < 4)
        return kCIDLib::False;

    // Put an XML Decl into the temp string
    m_strTmp = L"<?xml version='1.0' encoding='$NativeWideChar$'?>\r\n";

    //
    //  Transcode it into the string, skipping the start/stop characters. So we
    //  start on the second byte and transcode the count minute 2 bytes
    //
    //  Tell it to append to the string so it doesn't overwrite our XMLDecl.
    //
    m_tcvtMsg.c4ConvertFrom
    (
        m_mbufTmp.pc1DataAt(1), c4Len - 2, m_strTmp, tCIDLib::EAppendOver::Append
    );

    // And now parse it. We have to set up an entity reference for the string
    tCIDXML::TEntitySrcRef esrToParse(new TMemBufEntitySrc(L"RootEntity", m_strTmp));

    const tCIDLib::TBoolean bRes = m_xtprsMsg.bParseRootEntity
    (
        esrToParse
        , tCIDXML::EParseOpts::None
        , tCIDXML::EParseFlags::TagsNChars
    );

    if (!bRes)
    {

        const TXMLTreeParser::TErrInfo& erriFirst = m_xtprsMsg.colErrors()[0];

        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Could not parse incoming message from tray app"
                , erriFirst.strText()
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    // OK the root element name is the command
    const TXMLTreeElement& xtnodeRoot = m_xtprsMsg.xtdocThis().xtnodeRoot();
    strCmdToFill = xtnodeRoot.strQName();

    // See if it has an attribute. If so, take the value
    if (xtnodeRoot.c4AttrCount())
        strValToFill = xtnodeRoot.xtattrAt(0).strValue();
    else
        strValToFill.Clear();

    return kCIDLib::True;
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
tCIDLib::TVoid TiTunesRepo2Loader::PostProcessing()
{
    const tCQCMedia::EMediaTypes eType = tCQCMedia::EMediaTypes::Music;

    //
    //  Let's create a playlists category if it doesn't already exist. We'll
    //  put all of the playlists into that category.
    //
    TString strPLCat(L"PLAYLISTS");
    TMediaCat* pmcatPLs = m_pmdbLoading->pmcatByUniqueId(eType, strPLCat, kCIDLib::False);
    if (!pmcatPLs)
    {
        pmcatPLs = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Music
            , L"Playlists"
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
        if ((strPLName == iTunesRepo2S_DBLoader::strText_AudioBooks)
        ||  (strPLName == iTunesRepo2S_DBLoader::strText_Library)
        ||  (strPLName == iTunesRepo2S_DBLoader::strText_Movies)
        ||  (strPLName == iTunesRepo2S_DBLoader::strText_Music)
        ||  (strPLName == iTunesRepo2S_DBLoader::strText_MusicVideos))
        {
            continue;
        }

        //
        //  See if this one has any entries. If not, skip it. Or, if it
        //  has over 5K entries, skip it also for practical purposes.
        //
        TPLIds& colPLIds = m_colPListList[c4Index];
        const tCIDLib::TCard4 c4TIDCnt = colPLIds.c4ElemCount();
        if (!c4TIDCnt || (c4TIDCnt > 5000))
        {
            if (c4TIDCnt && (m_eVerbose >= tCQCKit::EVerboseLvls::Medium))
            {
                faciTunesRepo2S().LogMsg
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
            , m_c2NextColId++
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
            , pmtsPL->c2Id()
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
        tCIDLib::TCard4 c4ColDur = 0;
        for (tCIDLib::TCard4 c4TIDInd = 0; c4TIDInd < c4TIDCnt; c4TIDInd++)
        {
            // If we've maxed on item ids, then we have to stop
            if (m_c2NextItemId == kCIDLib::c2MaxCard)
                break;

            //
            //  Look up this one in the by id list. If blank, skip it. We
            //  get out the stored id for this entry in this playlists's
            //  list of tracks, subtract one, and use that to index the
            //  linear list of item pointers we stored during parsing.
            //
            //  The ids in the XML are 1 based, so we have to sub 1.
            //
            TMediaItem* pmitemCur = m_colItemIDList[colPLIds[c4TIDInd] - 1];

            //
            //  If not found, then skip it. This means we couldn't use
            //  the referenced track during parsing, so it can't be in the
            //  playlist now either.
            //
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
    //  Deal with the collection level artist name. If all of the
    //  item artist names are the same, we set the collection level
    //  artist to that. Else we set it to Various Artists.
    //
    const tCIDLib::TCard4 c4ColCnt = m_pmdbLoading->c4CollectCnt(eType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCnt; c4Index++)
    {
        //
        //  If all of the artist names are the same, then we just use
        //  that. Else, set it to "Various Artists"
        //
        TMediaCollect& mcolCur = m_pmdbLoading->mcolAt(eType, c4Index);
        const tCIDLib::TCard4 c4ItemCnt = mcolCur.c4ItemCount();
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

    // And do the same for title sets wrt to their collections
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
//  Each PL track is a single element. So when that element is seen, we get
//  called here with the info.
//
tCIDLib::TVoid
TiTunesRepo2Loader::ProcessPLTrack(         TXMLParserCore&     xtprsSrc
                                    , const TXMLElemDecl&       xdeclElem
                                    , const TVector<TXMLAttr>&  colAttrList
                                    , const tCIDLib::TCard4     c4AttrCnt)
{
    CIDAssert(c4AttrCnt == 1, L"Didn't have one attribute on PL track");

    // There's only one attribute, the track id
    const tCIDLib::TCard4 c4TrackId = colAttrList[0].strValue().c4Val();

    //
    //  Add this one to the list at the end of the playlist vector. We use
    //  the name list for the count, to help insure we stay in sync. If
    //  the name list is larger, we'll get an error.
    //
    m_colPListList.objAt(m_colPLNames.c4ElemCount() - 1).c4AddElement(c4TrackId);
}


//
//  Each track is a single element. So when that element is seen, we get
//  called here with the info. Here is an example of a track:
//
//    <Track
//        AlbumGUID="171c6561-5d7e-4711-aa35-12351b3b020e"
//        AlbumStringGUID="Chantal Kreviazuk - Under These Rocks And Stones"
//        Album="Under These Rocks And Stones"
//        Artist="Chantal Kreviazuk"
//        BitRate="256"
//        Compilation="False"
//        Composer=""
//        Duration="285"
//        EQ=""
//        Genre="Alternative &amp; Punk"
//        Grouping=""
//        Location="C:\Media\iTunes\Chantal Kreviazuk\Under These Rocks And Stones\06 Wayne.m4a"
//        Name="Wayne"
//        Number="6"
//        PlayOrderIndex="16"
//        SampleRate="44100"
//        ime="4:45"
//        TrackID="9"
//        TrackNumber="6"
//        Artwork="C:\Tmp\iTunes\171c6561-5d7e-4711-aa35-12351b3b020e.jpeg"
//    />
//
tCIDLib::TVoid
TiTunesRepo2Loader::ProcessTrack(       TXMLParserCore&     xprsSrc
                                , const TXMLElemDecl&       xdeclElem
                                , const TVector<TXMLAttr>&  colAttrList
                                , const tCIDLib::TCard4     c4AttrCnt)
{
    //
    //  If we've maxed on item ids, then we have to stop. We logged last type
    //  through (at the bottom of this method) that we maxed out.
    //
    if (m_c2NextItemId == kCIDLib::c2MaxCard)
        return;

    //
    //  Default everything and then run through the attributes and store the
    //  values. If we don't end up with something that's required, we drop
    //  this one.
    //
    tCIDLib::TCard4 c4Duration = 0;
    tCIDLib::TCard4 c4TrackID = kCIDLib::c4MaxCard;
    tCIDLib::TCard4 c4TrackNum = kCIDLib::c4MaxCard;
    tCIDLib::TCard4 c4Year = 0;
    m_strTr_AlbumID.Clear();
    m_strTr_AlbumName.Clear();
    m_strTr_Artist.Clear();
    m_strTr_ArtPath.Clear();
    m_strTr_Genre.Clear();
    m_strTr_LocInfo.Clear();
    m_strTr_Name.Clear();

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AttrCnt; c4Index++)
    {
        const TXMLAttr& xattrCur = colAttrList[c4Index];

        if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_AlbumID)
            m_strTr_AlbumID = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_AlbumName)
            m_strTr_AlbumName = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Artist)
            m_strTr_Artist = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_ArtPath)
            m_strTr_ArtPath = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Duration)
            c4Duration = xattrCur.strValue().c4Val();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Genre)
            m_strTr_Genre = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_LocInfo)
            m_strTr_LocInfo = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Name)
            m_strTr_Name = xattrCur.strValue();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_Number)
            c4TrackNum = xattrCur.strValue().c4Val();
        else if (xattrCur.strQName() == iTunesRepo2S_DBLoader::strElem_TrackID)
            c4TrackID = xattrCur.strValue().c4Val();
    }

    if (m_strTr_AlbumID.bIsEmpty()
    ||  m_strTr_AlbumName.bIsEmpty()
    ||  m_strTr_Artist.bIsEmpty()
    ||  m_strTr_LocInfo.bIsEmpty()
    ||  m_strTr_Name.bIsEmpty()
    ||  (c4TrackNum == kCIDLib::c4MaxCard)
    ||  (c4TrackID == kCIDLib::c4MaxCard))
    {
        // Can't use this one
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_IgnoredTrack
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(xprsSrc.c4CurLine())
            );
        }

        // Add a null slot in the id list
        m_colItemIDList.Add(0);
        return;
    }

    // Watch for a path that's illegally long
    if (m_strTr_LocInfo.c4Length() > kCIDLib::c4MaxPathLen)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_LocInfoLen
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(xprsSrc.c4CurLine())
            );
        }

        // Add a null slot in the id list
        m_colItemIDList.Add(0);
        return;
    }

    //
    //  If we got an art path, see if this image is already in the database,
    //  else add it.
    //
    TMediaImg* pmimgArt = 0;
    if (!m_strTr_ArtPath.bIsEmpty())
    {
        pmimgArt = m_pmdbLoading->pmimgByPath
        (
            tCQCMedia::EMediaTypes::Music, m_strTr_ArtPath, kCIDLib::False
        );

        if (!pmimgArt)
        {
            //
            //  We don't have separate large/small images, so we just pass
            //  an empty string for the thumb path. It'll get generated
            //  by scaling down the large one.
            //
            pmimgArt = new TMediaImg
            (
                tCQCMedia::EMediaTypes::Music
                , m_c2NextImgId++
                , m_strTr_ArtPath
                , TString::strEmpty()
            );
            m_pmdbLoading->AddImage(pmimgArt);
        }
    }

    //
    //  It looks ok, so let's handle it. If there's no title set with this
    //  album id, add it, else we'll update the existing one.
    //
    tCIDLib::TBoolean bNewTitle = kCIDLib::False;
    TMediaTitleSet* pmtsCur = 0;
    pmtsCur = m_pmdbLoading->pmtsByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, m_strTr_AlbumID, kCIDLib::False
    );

    TMediaCollect* pmcolCur;
    if (pmtsCur)
    {
        // It already exists, so get the collection out
        pmcolCur = &pmtsCur->mcolAt(*m_pmdbLoading, 0);

        //
        //  If this year is non-zero and before the earliest year we have stored
        //  so far for this collection, take it.
        //
        if (c4Year && (c4Year < pmcolCur->c4Year()))
            pmcolCur->c4Year(c4Year);
    }
     else
    {
        // A new one, so create a new title set and add it to the database
        facCQCMedia().bMakeSortTitle(m_strTr_AlbumName, m_strTmp);
        bNewTitle = kCIDLib::True;
        pmtsCur = new TMediaTitleSet
        (
            m_strTr_AlbumName
            , m_strTr_AlbumID
            , m_strTmp
            , m_c2NextColId++
            , tCQCMedia::EMediaTypes::Music
        );
        m_pmdbLoading->AddTitle(pmtsCur);

        //
        //  Add a single collection. Use the title set unique id for
        //  the disc id, since it doesn't really matter in this repository.
        //  Give it the same numeric id as the title set we just added,
        //  since there is a one to one relationship between them in this
        //  repository.
        //
        pmcolCur = new TMediaCollect
        (
            m_strTr_AlbumName
            , pmtsCur->strUniqueId()
            , pmtsCur->c2Id()
            , c4Year
            , tCQCMedia::ELocTypes::FileItem
            , tCQCMedia::EMediaTypes::Music
        );
        m_pmdbLoading->AddCollect(pmcolCur);
        pmtsCur->bAddCollect(*pmcolCur);

        // Add it to the all music category
        pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMusic);

        // Set the image id for the cover art if any
        if (pmimgArt)
            pmcolCur->c2ArtId(pmimgArt->c2Id());

        // Bump the title count
        m_c4TitlesDone++;
    }

    //
    //  Create the non-case sensitive version of the category name, and
    //  see if it exists. If not, add it.
    //
    m_strTmp = m_strTr_Genre;
    m_strTmp.ToUpper();
    TMediaCat* pmcatCur = m_pmdbLoading->pmcatByUniqueId
    (
        tCQCMedia::EMediaTypes::Music, m_strTmp, kCIDLib::False
    );

    if (!pmcatCur)
    {
        pmcatCur = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Music, m_strTr_Genre, m_c2NextCatId++
        );
        m_pmdbLoading->AddCategory(pmcatCur);
    }

    // Add the collection to this category if not already
    pmcolCur->bAddCategory(*pmcatCur);

    //
    //  Create a unique id for the track, based on the album id plus a
    //  track number.
    //
    //  Note that, we pass in a unique id for items, but they aren't used
    //  to store the items in the database. They are only stored by id. So
    //  there's no way to look one up by unique id to see if it's already
    //  in the database.
    //
    m_strTmp = m_strTr_AlbumID;
    m_strTmp.Append(kCIDLib::chUnderscore);
    m_strTmp.AppendFormatted(c4TrackNum);
    TMediaItem* pmitemNew = new TMediaItem
    (
        m_strTr_Name
        , m_strTmp
        , m_c2NextItemId++
        , m_strTr_LocInfo
        , tCQCMedia::EMediaTypes::Music
    );
    m_pmdbLoading->AddItem(pmitemNew);

    // Since we could get them out of order, set the track number
    pmitemNew->c4LoadOrder(c4TrackNum);

    // Store the artist name and duration info
    pmitemNew->strArtist(m_strTr_Artist);
    pmitemNew->c4Duration(c4Duration);

    //
    //  Figure out where to add the track in the collection. They are not
    //  necessarliy in order.
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

    // Add this one to the id list
    m_colItemIDList.Add(pmitemNew);

    // Add this track's time to our total
    pmcolCur->c4Duration(pmcolCur->c4Duration() + c4Duration);

    //
    //  If we hit the maximum item id, then log this. We are going to
    //  ignore everything from here on out.
    //
    if (m_c2NextItemId == kCIDLib::c2MaxCard)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            faciTunesRepo2S().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kiTunes2Msgs::midStatus_MaxItems
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


//
//  The caller sends us a command, an attribute name and a value for the
//  attribute. We build up the XML message and send it. For the tray app
//  it doesn't expect an XMLDecl so we don't add one.
//
tCIDLib::TVoid
TiTunesRepo2Loader::SendMsg(        TClientStreamSocket&    sockTray
                            , const TString&                strCmd
                            , const TString&                strAttrName
                            , const TString&                strAttrVal)
{
    //
    //  Build up the message into a temp string. We have to escape the
    //  attribute just in case. We'll release the stream after we get
    //  data flushed out into the string.
    //
    {
        TTextStringOutStream strmOut(&m_strTmp);
        strmOut << L"<" << strCmd;
        facCIDXML().FormatAttr(strmOut, strAttrName, strAttrVal, kCIDLib::True);
        strmOut << L"/>\r\n" << kCIDLib::FlushIt;
    }

    //
    //  We need to get it into a buffer with STX/ETX bracket bytes around it.
    //  So we transcode into the second byte to leave space.
    //
    tCIDLib::TCard1 ac1TmpBuf[iTunesRepo2S_DBLoader::c4IOBufSz];
    tCIDLib::TCard4 c4Cnt;
    m_tcvtMsg.c4ConvertTo
    (
        m_strTmp.pszBuffer()
        , m_strTmp.c4Length()
        , &ac1TmpBuf[1]
        , iTunesRepo2S_DBLoader::c4IOBufSz - 2
        , c4Cnt
    );

    // Put in the bracket bytes and add them to the count
    ac1TmpBuf[0] = 2;
    ac1TmpBuf[c4Cnt + 1] = 3;
    c4Cnt += 2;

    // And now we can send it
    sockTray.Send(ac1TmpBuf, c4Cnt);
}


