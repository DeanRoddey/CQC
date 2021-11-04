//
// FILE NAME: JRDiskRepoS_DBLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/09/2005
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
#include    "JRDiskRepoS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TJRDiskRepoLoader,TThread)



// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace JRDiskRepoS_DBLoader
{
    // -----------------------------------------------------------------------
    //  The names of some things we look for in the XML
    // -----------------------------------------------------------------------
    const TString   strAttr_Name(L"Name");
    const TString   strElem_Field(L"Field");
    const TString   strElem_Item(L"Item");
    const TString   strElem_Database(L"MPL");

    const TString   strFld_Album(L"Album");
    const TString   strFld_Artist(L"Artist");
    const TString   strFld_ArtPath(L"Image File");
    const TString   strFld_BitDepth(L"Bit Depth");
    const TString   strFld_BitRate(L"Bitrate");
    const TString   strFld_Channels(L"Channels");
    const TString   strFld_Comment(L"Comment");
    const TString   strFld_DateCreated(L"Date Created");
    const TString   strFld_Duration(L"Duration");
    const TString   strFld_Filename(L"Filename");
    const TString   strFld_Genre(L"Genre");
    const TString   strFld_MediaType(L"Media Type");
    const TString   strFld_SampleRate(L"Sample Rate");
    const TString   strFld_TrackName(L"Name");
    const TString   strFld_TrackNum(L"Track #");
    const TString   strFld_UserRating(L"Rating");
    const TString   strFld_Year(L"Date");
    const TString   strVal_Audio(L"Audio");


    // -----------------------------------------------------------------------
    //  The max number of bytes of an image we'll read to generate the persistent
    //  id. It's not likely any images, that aren't actually the same, will be
    //  indentical for this number of bytes.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPerIdBytes = 32 * 1024;
}



// ---------------------------------------------------------------------------
//   CLASS: TJRDiskRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TJRDiskRepoLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TJRDiskRepoLoader::TJRDiskRepoLoader(const TString& strMoniker) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_bInItem(kCIDLib::False)
    , m_c4TitlesDone(0)
    , m_eField(EFields::None)
    , m_eMediaType(tCQCMedia::EMediaTypes::Music)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_mbufPerId(JRDiskRepoS_DBLoader::c4MaxPerIdBytes, JRDiskRepoS_DBLoader::c4MaxPerIdBytes)
    , m_pmdbLoading(0)
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

TJRDiskRepoLoader::~TJRDiskRepoLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TJRDiskRepoLoader::CompleteLoad()
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
TJRDiskRepoLoader::eStatus(tCIDLib::TCard4& c4TitlesDone) const
{
    // Return the status and the count of titles processed
    c4TitlesDone = m_c4TitlesDone;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TJRDiskRepoLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


tCIDLib::TVoid TJRDiskRepoLoader::ResetDocument()
{
    // We don't need to do anything here
}


// Let the driver get to the DB serial number we created
const TString& TJRDiskRepoLoader::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid
TJRDiskRepoLoader::StartLoad(TMediaDB* const pmdbToLoad, TString& strFilename)
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
            , tCIDLib::EErrClasses::AppStatus
            , m_strMoniker
        );
    }

    // Store the new database to load and the file name
    m_pmdbLoading = pmdbToLoad;
    m_strXMLFile = strFilename;

    // Get out the path part of the file name and the base name part
    {
        TPathStr pathTmp(strFilename);

        if (!pathTmp.bQueryName(m_strXMLBaseName)
        ||  !pathTmp.bRemoveNameExt())
        {
            facJRDiskRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kJRDRErrs::errcDB_FileNotFound
                , m_strXMLFile
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strMoniker
            );
        }

        m_strXMLPath = pathTmp;
    }

    // If the file doesn't exist, then obviously this isn't going to work
    if (!TFileSys::bExists(m_strXMLFile))
    {
        m_eStatus = tCQCMedia::ELoadStatus::Failed;

        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kJRDRErrs::errcDB_FileNotFound
                , m_strXMLFile
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strMoniker
            );
        }
        return;
    }

    // Reset any of our per-load data
    m_bInItem = kCIDLib::False;
    m_c4TitlesDone = 0;
    m_eField = EFields::None;

    // Flush the media database that we are going to load
    m_pmdbLoading->Reset();

    //
    //  Add an 'All Music' category that all stuff can be put into. We have to
    //  tell the DB to take the fixed cat ids for these. We'll just use the upper
    //  cased name as the unique id.
    //
    m_strAllMusicCatName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMusic);
    m_strTmp1 = m_strAllMusicCatName;
    m_strTmp1.ToUpper();
    TMediaCat* pmcatNew = new TMediaCat(m_eMediaType, m_strAllMusicCatName, m_strTmp1);
    pmcatNew->c2Id(kCQCMedia::c2CatId_AllMusic);
    m_pmdbLoading->c2AddCategory(pmcatNew, kCIDLib::True);

    // A a playlists category
    m_strPlayListsCatName = facCQCMedia().strMsg(kMedMsgs::midCat_PlayLists);
    m_strTmp1 = m_strPlayListsCatName;
    m_strTmp1.ToUpper();
    pmcatNew = new TMediaCat(m_eMediaType, m_strPlayListsCatName, m_strTmp1);
    pmcatNew->c2Id(kCQCMedia::c2CatId_PlayLists);
    m_pmdbLoading->c2AddCategory(pmcatNew, kCIDLib::True);


    // Set the status to indicate we are loading
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Start ourself running in another thread
    Start();
}


//
//  Called by the main thread to make sure we are cleaned up when it needs
//  to shutdown.
//
tCIDLib::TVoid TJRDiskRepoLoader::Shutdown()
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

        // Be cautious and say it failed
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }
}


// ---------------------------------------------------------------------------
//  TJRDiskRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TJRDiskRepoLoader::DocCharacters(const  TString&            strChars
                                , const tCIDLib::TBoolean   bIsCDATA
                                , const tCIDLib::TBoolean   bIsIgnorable
                                , const tCIDXML::ELocations eLocation
                                , const tCIDLib::TBoolean   )
{
    //
    //  If the m_eField value is set, then we are expecting a value for
    //  one of the fields.
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the start element, so it's safe to do this.
    //
    switch(m_eField)
    {
        case EFields::Album :
            m_strFld_Album.Append(strChars);
            break;

        case EFields::Artist :
            m_strFld_Artist.Append(strChars);
            break;

        case EFields::ArtPath :
            m_pathFld_ArtPath.Append(strChars);
            m_pathFld_ArtPath.Normalize();
            break;

        case EFields::BitDepth :
            m_strFld_BitDepth.Append(strChars);
            break;

        case EFields::BitRate :
            m_strFld_BitRate.Append(strChars);
            break;

        case EFields::Channels :
            m_strFld_Channels.Append(strChars);
            break;

        case EFields::DateCreated :
            m_strFld_DateCreated.Append(strChars);
            break;

        case EFields::Description :
            m_strFld_Description.Append(strChars);
            break;

        case EFields::Duration :
            m_strFld_Duration.Append(strChars);
            break;

        case EFields::Filename :
            m_pathFld_FileName.Append(strChars);
            m_pathFld_FileName.Normalize();
            break;

        case EFields::Genre :
            m_strFld_CatNames.Append(strChars);
            break;

        case EFields::MediaType :
            m_strFld_MediaType.Append(strChars);
            break;

        case EFields::SampleRate :
            m_strFld_SampleRate.Append(strChars);
            break;

        case EFields::TrackName :
            m_strFld_TrackName.Append(strChars);
            break;

        case EFields::TrackNum :
            m_strFld_TrackNum.Append(strChars);
            break;

        case EFields::UserRating :
            m_strFld_UserRating.Append(strChars);
            break;

        case EFields::Year :
            m_strFld_Year.Append(strChars);
            break;
    };
}


//
//  The entry point for our background loader thread.
//
tCIDLib::EExitCodes TJRDiskRepoLoader::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::IgnoreBadChars;
        const tCIDXML::EParseFlags  eFlags = tCIDXML::EParseFlags::TagsNText;

        // Kick off the parsing
        m_xprsThis.ParseRootEntity(m_strXMLFile, eOpts, eFlags);

        // Do any needed post-processing
        PostProcessing();

        //
        //  Ok, it's done successfully, so call the load completion method
        //  on the media database, so that it can do any stuff it has to
        //  wait until all the data is available to do.
        //
        m_pmdbLoading->LoadComplete();

        //
        //  We have to generate a database serial number. So we just flatten out the
        //  database and hash the buffer. A bit piggy but the only way to do it that
        //  insures it only changes if the data we actually care about changes.
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
                , m_strXMLFile
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
                , m_strXMLFile
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
        facJRDiskRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kJRDRErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TJRDiskRepoLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        facJRDiskRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Entered EndDocument"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid TJRDiskRepoLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    const TString& strQName = xdeclElem.strFullName();
    if (strQName == JRDiskRepoS_DBLoader::strElem_Item)
    {
        // If not in an item right now, then it's bad
        if (!m_bInItem)
        {
            facJRDiskRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kJRDRErrs::errcDB_BadNesting
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Clear the flag now that we've finished this guy
        m_bInItem = kCIDLib::False;

        // If it's not an audio file type, we don't want it
        if (m_strFld_MediaType == JRDiskRepoS_DBLoader::strVal_Audio)
            ProcessItem();
    }
     else if (strQName == JRDiskRepoS_DBLoader::strElem_Database)
    {
    }
     else
    {
        //
        //  Probably end of a field, in which case we don't need to take any
        //  more text for this field. If not end of a field, this doesn't
        //  hurt since it should be none in that case anyway, and it avoids
        //  doing a field name compare for this last block.
        //
        m_eField = EFields::None;
    }
}


tCIDLib::TVoid
TJRDiskRepoLoader::HandleXMLError(  const   tCIDLib::TErrCode   errcToPost
                                    , const tCIDXML::EErrTypes  eType
                                    , const TString&            strText
                                    , const tCIDLib::TCard4     c4CurColumn
                                    , const tCIDLib::TCard4     c4CurLine
                                    , const TString&            strSystemId)
{
    // Log the message and set the failure flag
    m_eStatus = tCQCMedia::ELoadStatus::Failed;
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        facJRDiskRepoS().LogMsg
        (
            strSystemId
            , c4CurLine
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid TJRDiskRepoLoader::StartDocument(const TXMLEntitySrc&)
{
    //
    //  Don't really need to do anything. We are initialized before each
    //  call to do a parse, so our data is already ready. If we are in
    //  high verbosity level, log a trace message.
    //
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facJRDiskRepoS().LogMsg
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
TJRDiskRepoLoader::StartTag(        TXMLParserCore&     xprsSrc
                            , const TXMLElemDecl&       xdeclElem
                            , const tCIDLib::TBoolean   bEmpty
                            , const TVector<TXMLAttr>&  colAttrList
                            , const tCIDLib::TCard4     c4AttrListSize)
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
    if (strQName == JRDiskRepoS_DBLoader::strElem_Item)
    {
        // If we are already in an item, then the file is bad
        if (m_bInItem)
        {
            facJRDiskRepoS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kJRDRErrs::errcDB_BadNesting
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Format
            );
        }

        // Set the flag and reset any per-item data
        m_bInItem = kCIDLib::True;

        m_pathFld_ArtPath.Clear();
        m_pathFld_FileName.Clear();

        m_strFld_Album.Clear();
        m_strFld_Artist.Clear();
        m_strFld_BitDepth.Clear();
        m_strFld_BitRate.Clear();
        m_strFld_CatNames.Clear();
        m_strFld_Channels.Clear();
        m_strFld_DateCreated.Clear();
        m_strFld_Description.Clear();
        m_strFld_Duration.Clear();
        m_strFld_MediaType.Clear();
        m_strFld_SampleRate.Clear();
        m_strFld_TrackName.Clear();
        m_strFld_TrackNum.Clear();
        m_strFld_UserRating.Clear();
        m_strFld_Year.Clear();
    }
     else if (strQName == JRDiskRepoS_DBLoader::strElem_Field)
    {
        //
        //  It's a field value, so look at the Name attribute and set
        //  the m_eField enum to indicate what field we expect to see the
        //  data for in the next characters callback.
        //
        const TXMLAttr* pxattrName = 0;
        if (c4AttrListSize > 1)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4AttrListSize; c4Index++)
            {
                const TXMLAttr& xattrCur = colAttrList[c4Index];
                if (xattrCur.strQName() == JRDiskRepoS_DBLoader::strAttr_Name)
                {
                    pxattrName = &xattrCur;
                    break;
                }
            }

            // If it doesn't have a name attr, the file is bad
            if (!pxattrName)
            {
                facJRDiskRepoS().ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kJRDRErrs::errcDB_NoFieldName
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
        }
         else
        {
            // Just one attr, assume it's the name
            pxattrName = &colAttrList[0];
        }

        //
        //  Get the value and set the field enum. There are fields we don't
        //  care about, in which we set it to unknown.
        //
        const TString& strFldName = pxattrName->strValue();

        //
        //  To avoid a lot of usless comparisons, we use pre-determined
        //  length values to only look at strings that are of the same
        //  length as ones we care about.
        //
        const tCIDLib::TCard4 c4NewLen = strFldName.c4Length();
        m_eField = EFields::None;
        switch(c4NewLen)
        {
            case 4 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_Year)
                    m_eField = EFields::Year;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_TrackName)
                    m_eField = EFields::TrackName;
                break;

            case 5 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_Album)
                    m_eField = EFields::Album;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_Genre)
                    m_eField = EFields::Genre;
                break;

            case 6 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_Artist)
                    m_eField = EFields::Artist;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_UserRating)
                    m_eField = EFields::UserRating;
                break;

            case 7 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_BitRate)
                    m_eField = EFields::BitRate;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_TrackNum)
                    m_eField = EFields::TrackNum;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_Comment)
                    m_eField = EFields::Description;
                break;

            case 8 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_Channels)
                    m_eField = EFields::Channels;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_Filename)
                    m_eField = EFields::Filename;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_Duration)
                    m_eField = EFields::Duration;
                break;

            case 9 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_BitDepth)
                    m_eField = EFields::BitDepth;
                break;

            case 10 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_ArtPath)
                    m_eField = EFields::ArtPath;
                else if (strFldName == JRDiskRepoS_DBLoader::strFld_MediaType)
                    m_eField = EFields::MediaType;
                break;

            case 11 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_SampleRate)
                    m_eField = EFields::SampleRate;
                break;

            case 12 :
                if (strFldName == JRDiskRepoS_DBLoader::strFld_DateCreated)
                    m_eField = EFields::DateCreated;
                break;

            default :
                break;
        };
    }
}

// ---------------------------------------------------------------------------
//  TJRDiskRepoLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to open and read in the contents of a playlist file.
//
tCIDLib::TBoolean
TJRDiskRepoLoader::bLoadPlaylist(const  TPathStr&           pathPLFile
                                , const TString&            strBaseName
                                ,       tCIDLib::TStrList&  colToLoad)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedMsgs::midStatus_LoadingPL
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strBaseName
        );
    }

    colToLoad.RemoveAll();
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

        // And run through the lines in this file. We toss out directives and comments.
        tCIDLib::TCard4 c4LineNum(0);
        while (!strmPL.bEndOfStream())
        {
            // Get a line with lead/trail whitespace stripped off
            c4LineNum++;
            strmPL.c4GetLine(m_pathTmp, 0, kCIDLib::True);

            // Skip empty lines or comments/directives
            if (m_pathTmp.bIsEmpty() || (m_pathTmp[0] == kCIDLib::chPoundSign))
                continue;

            // Only take it if fully qualified
            if (m_pathTmp.bIsFullyQualified())
                colToLoad.objAdd(m_pathTmp);
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
        return kCIDLib::False;
    }

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facCQCMedia().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kMedMsgs::midStatus_PLLoaded
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , pathPLFile
        );
    }

    // If not empty, or too big, we say it's good
    return !colToLoad.bIsEmpty() && (colToLoad.c4ElemCount() < 8192);
}


//
//  A helper to convert a cardinal value. It checks to see if it's empty or
//  cannot be converted and returns a default if so. Else it returns the
//  converted value.
//
//  It's possible that it might be a floating point value. Perhaps JR has
//  changed recently because this never came up before, but some people have
//  complained the time wasn't correct and it was because the value was a
//  float. So if it doesn't convert as a Card, we try a float and take the
//  integral part.
//
tCIDLib::TCard4
TJRDiskRepoLoader::c4XlatCardVal(const TString& strText, const tCIDLib::TCard4 c4Def)
{
    tCIDLib::TCard4 c4Val;
    if (strText.bIsEmpty())
    {
        c4Val = c4Def;
    }
     else
    {
        tCIDLib::TBoolean bValid;
        c4Val = TRawStr::c4AsBinary
        (
            strText.pszBuffer(), bValid, tCIDLib::ERadices::Dec
        );

        if (!bValid)
        {
            const tCIDLib::TFloat8 f8Val = TRawStr::f8AsBinary
            (
                strText.pszBuffer(), bValid
            );

            if (bValid)
                c4Val = tCIDLib::TCard4(f8Val);
            else
                c4Val = c4Def;
        }
    }
    return c4Val;
}

tCIDLib::TCard8
TJRDiskRepoLoader::c8XlatCardVal(const TString& strText, const tCIDLib::TCard8 c8Def)
{
    tCIDLib::TCard8 c8Val;
    if (strText.bIsEmpty())
    {
        c8Val = c8Def;
    }
     else
    {
        tCIDLib::TBoolean bValid;
        c8Val = TRawStr::c8AsBinary
        (
            strText.pszBuffer(), bValid, tCIDLib::ERadices::Dec
        );
        if (!bValid)
            c8Val = c8Def;
    }
    return c8Val;
}


//
//  This is called to process a cover art file, either for a real collection or a
//  playlist, it doesn't matter. We get the fully qualified path to the file. If we
//  add it, we return a pointer to the image object we created.
//
const TMediaImg* TJRDiskRepoLoader::pmimgProcessArtFile(const TString& strImgPath)
{
    // If it doesn't exist, then nothing to do
    if (!TFileSys::bExists(strImgPath))
        return nullptr;

    //
    //  Generate the unique id. we need to insure it stays the same upon reload, so we
    //  have to stick to info in the data. But there's not much to work with. So we just
    //  hash the art path.
    //
    m_mdigId.StartNew();
    m_mdigId.DigestStr(strImgPath);
    m_mdigId.Complete(m_mhashId);
    m_mhashId.FormatToStr(m_strUniqueId);

    // See if we have this one already. If so, just return that
    {
        const TMediaImg* pmimgCur = m_pmdbLoading->pmimgByUniqueId
        (
            m_eMediaType, m_strUniqueId, kCIDLib::False
        );
        if (pmimgCur)
            return pmimgCur;
    }

    // Let's read up to 32K of this image, and hash that to create the persistent id.
    TMediaImg* pmimgNew = nullptr;
    try
    {
        TBinaryFile flImg(strImgPath);
        flImg.Open
        (
            tCIDLib::EAccessModes::Excl_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );

        // Read up to the max bytes
        const tCIDLib::TCard4 c4PerIdBytes = flImg.c4ReadBuffer
        (
            m_mbufPerId
            , JRDiskRepoS_DBLoader::c4MaxPerIdBytes
            , tCIDLib::EAllData::OkIfNotAll
        );

        //
        //  We don't have separate large/small images, so we just pass an empty string
        //  for the thumb image. It'll get generated by scaling down the large one.
        //
        pmimgNew = new TMediaImg(m_eMediaType, strImgPath, TString::strEmpty());
        TJanitor<TMediaImg> janImg(pmimgNew);
        pmimgNew->strUniqueId(m_strUniqueId);

        //
        //  If we read the whole file, then go ahead and cache it since we have it all
        //  already. This will be fairly common.
        //
        if (tCIDLib::TCard4(flImg.c8CurSize()) == c4PerIdBytes)
            pmimgNew->SetArt(c4PerIdBytes, m_mbufPerId, tCQCMedia::ERArtTypes::LrgCover);

        //
        //  Generate the persistent id and store it. We store the same thing for large
        //  and small art, since we create the small from the large. If we don't do this,
        //  the client service will think there's no small art.
        //
        facCQCMedia().CreatePersistentId(m_mbufPerId, c4PerIdBytes, m_strTmp1);
        pmimgNew->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::LrgCover);
        pmimgNew->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::SmlCover);

        // OK, we survived so add it
        m_pmdbLoading->c2AddImage(janImg.pobjOrphan());
    }

    catch(...)
    {
        // Just don't load an image for this one
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Could not process art file"
                , strImgPath
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return nullptr;
    }

    return pmimgNew;
}


//
//  This is called after the XML parsing is complete, which means we have all of the
//  data gathered up.
//
tCIDLib::TVoid TJRDiskRepoLoader::PostProcessing()
{
    //
    //  Deal with artist names, checking to see if all the artists in a given
    //  collection are the same, and take that if so, else various artists. And
    //  for the titles relative to their collections. The DB provides a helper
    //  to do this.
    //
    m_pmdbLoading->AccumArtistNames(m_eMediaType);

    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facJRDiskRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Looking for playlists to load"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    //
    //  Process any playlist files in this directory. For whatever reason,
    //  Windows retardly treats searches as though they really have a * at
    //  the end of the extension, so this will return both .m3u and .m3u8
    //  files. Only do this if asked to.
    //
    tCIDLib::TStrList colTmp(256);
    TFindBuf fndbSearch;
    TDirIter diterPLs;
    if (diterPLs.bFindFirst(m_strXMLPath
                            , L"*.m3u"
                            , fndbSearch
                            , tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            ProcessPlaylist(fndbSearch.pathFileName(), colTmp);
        }   while (diterPLs.bFindNext(fndbSearch));
    }

    if (diterPLs.bFindFirst(m_strXMLPath
                            , L"*.m3u8"
                            , fndbSearch
                            , tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            ProcessPlaylist(fndbSearch.pathFileName(), colTmp);
        }   while(diterPLs.bFindNext(fndbSearch));
    }
}


//
//  Called when we hit the end of an item and need to get the data stored
//  away.
//
tCIDLib::TVoid TJRDiskRepoLoader::ProcessItem()
{
    // If the title or file path is empty, then we can't do this one
    if (m_strFld_Album.bIsEmpty() || m_pathFld_FileName.bIsEmpty())
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"CD name/loc info was empty, skipping item ended at line %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(m_xprsThis.c4CurLine())
            );
        }
        return;
    }

    //
    //  Sometimes J.River doesn't enforce path length restrictions, which
    //  will cause us to fail below. So check the length up front as well and
    //  ignore if too long.
    //
    if (m_pathFld_FileName.c4Length() > kCIDLib::c4MaxPathLen)
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"File path for item ending at line %(1) was too long"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(m_xprsThis.c4CurLine())
            );
        }
        return;
    }

    //
    //  If we got an art path, then add this one. We have to generate a persistent
    //  id for each image. Though it's possible that we might have the same image
    //  used more than once, it's not worth checking for, so we don't bother. We'll
    //  just load it more than once.
    //
    //  Also here check that the path length is not overly long. It was normalized
    //  during the storing of the XML data for this entry.
    //
    const TMediaImg* pmimgArt = nullptr;
    if (!m_pathFld_ArtPath.bIsEmpty()
    &&  (m_pathFld_ArtPath.c4Length() < kCIDLib::c4MaxPathLen))
    {
        try
        {
            // If it's not fully qualified, make it relative to the media file
            if (!m_pathFld_ArtPath.bIsFullyQualified())
            {
                m_pathTmp = m_pathFld_FileName;
                m_pathTmp.bRemoveNameExt();
                m_pathFld_ArtPath.AddToBasePath(m_pathTmp);
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
                    , L"Could not process art file path at line %(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , TCardinal(m_xprsThis.c4CurLine())
                );
            }

            // Empty the path so we do nothing elow
            m_pathFld_ArtPath.Clear();
        }

        //
        //  Process the image. We get a null back if it couldn't be found or loaded. If
        //  it has already been handled, we get the previous copy back. It will have
        //  been added to the database for us upon return if new.
        //
        pmimgArt = pmimgProcessArtFile(m_pathFld_ArtPath);
    }

    //
    //  They don't provide unique ids, so we generate one ourself by hashing
    //  the album name.
    //
    m_mdigId.StartNew();
    m_mdigId.DigestStr(m_strFld_Album);
    m_mdigId.Complete(m_mhashId);
    m_mhashId.FormatToStr(m_strUniqueId);

    //
    //  The date is a Julian date, based on year 1900, so we have to do
    //  a bit of conversion. So convert to a number and add the base Julian
    //  date value for 1/1/1900, then convert to year/month/day values.
    //
    tCIDLib::TCard4 c4Year = 0;
    if (!m_strFld_Year.bIsEmpty())
    {
        const tCIDLib::TFloat8 f8Julian
        (
            m_strFld_Year.f8Val() + kCIDLib::c4Julian1900
        );
        tCIDLib::TCard4  c4Day;
        tCIDLib::EMonths eMonth;
        TTime::CvtFromJulian(f8Julian, c4Year, eMonth, c4Day);
    }

    //
    //  See if this title is already in the database. If not, we need to
    //  create it. Unfortunately, we have nothing in the data to allow us
    //  to deal correctly with individual discs and create per-disc
    //  collections within the title set. So we just put all the tracks
    //  into a single collection.
    //
    tCIDLib::TBoolean bNewTitle = kCIDLib::False;
    const TMediaTitleSet* pmtsCur = 0;
    pmtsCur = m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strUniqueId, kCIDLib::False);

    const TMediaCollect* pmcolCur = nullptr;
    if (pmtsCur)
    {
        // It already exists, so get the collection out
        pmcolCur = &pmtsCur->mcolAt(*m_pmdbLoading, 0);

        // If we have no description yet, and this item had one, use it
        if (pmcolCur->strDescr().bIsEmpty() && !m_strFld_Description.bIsEmpty())
        {
            m_pmdbLoading->SetDescription
            (
                pmcolCur->c2Id(), m_eMediaType, m_strFld_Description
            );
        }
    }
     else
    {
        //
        //  Add a single collection. Use the same unique id as we will use for
        //  the set/
        //
        //  Just set a zero year. It will get set at the end when we tell the
        //  database to finalize the data, which will aggregate that info from
        //  contained items.
        //
        TMediaCollect* pmcolNew = new TMediaCollect
        (
            m_strFld_Album
            , m_strUniqueId
            , 0
            , tCQCMedia::ELocTypes::FileItem
            , m_eMediaType
        );
        TJanitor<TMediaCollect> janCol(pmcolNew);

        // Add it to the all music category
        pmcolNew->bAddCategory(kCQCMedia::c2CatId_AllMusic);

        // Set the image id for the cover art if any
        if (pmimgArt)
            pmcolNew->c2ArtId(pmimgArt->c2Id());

        // If we have any description store that
        pmcolNew->strDescr(m_strFld_Description);

        // And add it to the database
        m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());


        // Now create the title set and add the above collection to it
        facCQCMedia().bMakeSortTitle(m_strFld_Album, m_strTmp1);
        bNewTitle = kCIDLib::True;
        TMediaTitleSet* pmtsNew = new TMediaTitleSet
        (
            m_strFld_Album, m_strUniqueId, m_strTmp1, m_eMediaType
        );
        TJanitor<TMediaTitleSet> janSet(pmtsNew);

        //
        //  For some values we'll take the one in the first item that causes the set
        //  to be created to be taken as the value for the overall title set.
        //
        //  The date added is seconds since midnight 1/1/1970. Our regular 100-ns
        //  time stamps are from there as well, so we just multiply it up to 100-ns
        //  count.
        //
        pmtsNew->c4SeqNum(m_pmdbLoading->c4TitleSetCnt(m_eMediaType));
        pmtsNew->enctAdded
        (
            c8XlatCardVal(m_strFld_DateCreated, 0) * kCIDLib::enctOneSecond
        );

        pmtsNew->bAddCollect(*pmcolNew);
        m_pmdbLoading->c2AddTitle(janSet.pobjOrphan());

        // Bump the count of titles added
        m_c4TitlesDone++;

        //
        //  Now update the original points above to point at our new ones, for
        //  subsequent use below.
        //
        pmtsCur = pmtsNew;
        pmcolCur = pmcolNew;
    }

    //
    //  Scan through the genres in the genre list we found for this round and see
    //  if they exist yet. Create them if not, and add them to the list of cats or
    //  this collection.
    //
    if (!m_strFld_CatNames.bIsEmpty())
    {
        TStringTokenizer stokCats(&m_strFld_CatNames, L",");
        TString          strCurCat;
        while (stokCats.bGetNextToken(strCurCat))
        {
            strCurCat.StripWhitespace();

            // Use the upper cased name as a unique id
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

            // Add this one to the collection if not already
            m_pmdbLoading->AddColToCat(pmcolCur->c2Id(), pmcatCur->c2Id(), m_eMediaType);
        }
    }

    //
    //  Set the unique id to the album id, plus an underscore and then the
    //  current count of items in this collection plus one. We can't use
    //  the track number since it might be a dup, and that would cause an
    //  error in the hash set.
    //
    m_strUniqueId.Append(kCIDLib::chUnderscore);
    m_strUniqueId.AppendFormatted(pmcolCur->c4ItemCount() + 1);
    TMediaItem* pmitemNew = new TMediaItem
    (
        m_strFld_TrackName, m_strUniqueId, m_pathFld_FileName, m_eMediaType
    );
    TJanitor<TMediaItem> janItem(pmitemNew);

    //
    //  Since we could get them out of order, we use the optional load order
    //  field to store the track number so that we can insert new ones at
    //  the correct spot in the collection.
    //
    const tCIDLib::TCard4 c4TrackNum = c4XlatCardVal(m_strFld_TrackNum, 0);
    pmitemNew->c4LoadOrder(c4TrackNum);

    //
    //  If we got art, set it on the item as well, so that it can be
    //  displayed when this item is being accessed via a playlist.
    //
    if (pmimgArt)
        pmitemNew->c2ArtId(pmimgArt->c2Id());

    //
    //  Break out the artist name into first and last names. Once that's
    //  done, we then replace any ^ characters with spaces. This allows
    //  the user to force a name to be a single name.
    //
    m_strFld_Artist.bReplaceChar(kCIDLib::chCircumflex, kCIDLib::chSpace);
    pmitemNew->strArtist(m_strFld_Artist);

    // Figure out the rating
    tCIDLib::TCard4 c4Rating = c4XlatCardVal
    (
        m_strFld_UserRating, kCQCMedia::c4MinUserRating
    );
    if (c4Rating > kCQCMedia::c4MaxUserRating)
        c4Rating = kCQCMedia::c4MaxUserRating;

    //
    //  And store the other item stuff. Some of this is just for
    //  aggregation up to the col/set when we are done loading.
    //
    pmitemNew->c4BitDepth(c4XlatCardVal(m_strFld_BitDepth, 0));
    pmitemNew->c4BitRate(c4XlatCardVal(m_strFld_BitRate, 0));
    pmitemNew->c4Channels(c4XlatCardVal(m_strFld_Channels, 2));
    pmitemNew->c4SampleRate(c4XlatCardVal(m_strFld_SampleRate, 0));
    pmitemNew->c4Duration(c4XlatCardVal(m_strFld_Duration, 0));
    pmitemNew->c4Rating(c4Rating);
    pmitemNew->c4Year(c4Year);

    // OK, now that it's set up, we can add it, orphaning out of the janitor
    m_pmdbLoading->c2AddItem(janItem.pobjOrphan());

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
            m_eMediaType, pmcolCur->c2ItemIdAt(c4Index), kCIDLib::False
        );
        if (pmitemCur->c4LoadOrder() > c4TrackNum)
            break;
    }

    if (c4Index == c4ItemCnt)
        c4Index = kCIDLib::c4MaxCard;

    //
    //  Add it to the collection. We can't do this directly, since we only have
    //  const access, so use a helper on the database.
    //
    m_pmdbLoading->AddColItem(pmcolCur->c2Id(), m_eMediaType, *pmitemNew, c4Index);
}


//
//  In the post-processing steps, for each playlist file that we find, this is
//  called to process that file and add it to the database. We get a temp string
//  list to use, so we don't have to continually create and destroy one.
//
tCIDLib::TVoid
TJRDiskRepoLoader::ProcessPlaylist(const TPathStr& pathFile, tCIDLib::TStrList& colTmp)
{
    TString     strPLName;
    TPathStr    pathImg;

    // Pull out just the name part
    if (!pathFile.bQueryName(strPLName))
        return;

    // Creat a name for a potential cover art image
    pathImg = m_strXMLPath;
    pathImg.AddLevel(strPLName);
    pathImg.AppendExt(L"jpg");

    // Load the list of files from the file. If it fails, ignore this one
    if (!bLoadPlaylist(pathFile, strPLName, colTmp))
        return;

    // Looks reasonable, so let's do it. Create an id
    m_strTmp1 = L"PlayList_";
    m_strTmp1.Append(strPLName);

    //
    //  Make sure the name isn't a dup. If so, skip it. It shouldn't be but someone
    //  could name a real title the same as one of the playlist files.
    //
    if (m_pmdbLoading->pmtsByUniqueId(m_eMediaType, m_strTmp1, kCIDLib::False))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Playlist name already exists, skipping it"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strPLName
            );
        }
        return;
    }

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
        strPLName, m_strTmp1, 0, tCQCMedia::ELocTypes::FileItem, m_eMediaType
    );
    TJanitor<TMediaCollect> janCol(pmcolPL);

    pmcolPL->strArtist(L"Various Artists");
    pmcolPL->bIsPlayList(kCIDLib::True);

    // If the cover art file exists, store it for later faulting in
    if (!pathImg.bIsEmpty())
    {
        // Process the art file
        const TMediaImg* pmimgArt = pmimgProcessArtFile(pathImg);

        // If we got it, set the id on the collection
        if (pmimgArt)
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
    const tCIDLib::TCard4 c4ItemCnt = colTmp.c4ElemCount();
    const tCIDLib::TCard4 c4BaseIDLen = m_strTmp1.c4Length();
    tCIDLib::TCard4 c4ColDur = 0;
    for (tCIDLib::TCard4 c4PLIndex = 2; c4PLIndex < c4ItemCnt; c4PLIndex++)
    {
        const TString& strLoc = colTmp[c4PLIndex];
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
}
