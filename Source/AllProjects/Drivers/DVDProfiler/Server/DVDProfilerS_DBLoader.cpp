//
// FILE NAME: DVDProfilerS_DBLoader.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/16/2005
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
RTTIDecls(TDVDProfilerLoader,TThread)



// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace DVDProfilerS_DBLoader
{
    //  The names of some things we look for in the XML
    // -----------------------------------------------------------------------
    const TString   strAttr_CreditSubtype(L"CreditSubtype");
    const TString   strAttr_Director(L"Director");
    const TString   strAttr_Film(L"Film");
    const TString   strAttr_FirstName(L"FirstName");
    const TString   strAttr_LastName(L"LastName");
    const TString   strAttr_Name(L"Title");

    const TString   strElem_Actor(L"/Collection/DVD/Actors/Actor");
    const TString   strElem_Actors(L"/Collection/DVD/Actors");
    const TString   strElem_AspectRatio(L"/Collection/DVD/Format/FormatAspectRatio");
    const TString   strElem_ColNumber(L"/Collection/DVD/CollectionNumber");
    const TString   strElem_Credit(L"/Collection/DVD/Credits/Credit");
    const TString   strElem_Credits(L"/Collection/DVD/Credits");
    const TString   strElem_Descr(L"/Collection/DVD/Discs/Disc/DescriptionSideA");
    const TString   strElem_Disc(L"/Collection/DVD/Discs/Disc");
    const TString   strElem_Discs(L"/Collection/DVD/Discs");
    const TString   strElem_DVD(L"/Collection/DVD");
    const TString   strElem_Genre(L"/Collection/DVD/Genres/Genre");
    const TString   strElem_Genres(L"/Collection/DVD/Genres");
    const TString   strElem_Database(L"/Collection");
    const TString   strElem_Format(L"/Collection/DVD/Formats/Format");
    const TString   strElem_Formats(L"/Collection/DVD/Formats");
    const TString   strElem_Location(L"/Collection/DVD/Discs/Disc/Location");
    const TString   strElem_MFmt_BluRay(L"/Collection/DVD/MediaTypes/BluRay");
    const TString   strElem_MFmt_DVD(L"/Collection/DVD/MediaTypes/DVD");
    const TString   strElem_MFmt_HDDVD(L"/Collection/DVD/MediaTypes/HDDVD");
    const TString   strElem_Notes(L"/Collection/DVD/Notes");
    const TString   strElem_Overview(L"/Collection/DVD/Overview");
    const TString   strElem_Rating(L"/Collection/DVD/Rating");
    const TString   strElem_Review(L"/Collection/DVD/Review");
    const TString   strElem_RunningTime(L"/Collection/DVD/RunningTime");
    const TString   strElem_Slot(L"/Collection/DVD/Discs/Disc/Slot");
    const TString   strElem_SortTitle(L"/Collection/DVD/SortTitle");
    const TString   strElem_Timestamp(L"/Collection/DVD/ProfileTimestamp");
    const TString   strElem_Title(L"/Collection/DVD/Title");
    const TString   strElem_Studio(L"/Collection/DVD/Studio");
    const TString   strElem_Studios(L"/Collection/DVD/Studios");
    const TString   strElem_UniqueID(L"/Collection/DVD/ID");
    const TString   strElem_UPC(L"/Collection/DVD/UPC");
    const TString   strElem_Year(L"/Collection/DVD/ProductionYear");


    // -----------------------------------------------------------------------
    //  The max number of bytes of an image we'll read to generate the persistent
    //  id. It's not likely any images, that aren't actually the same, will be
    //  indentical for this number of bytes.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MaxPerIdBytes = 32 * 1024;
}



// ---------------------------------------------------------------------------
//   CLASS: TDVDProfilerLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDVDProfilerLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TDVDProfilerLoader::TDVDProfilerLoader(const TString& strMoniker) :

    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_c4TitlesDone(0)
    , m_colDiscs(8)
    , m_colCatNames(128)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_mbufPerId
      (
        DVDProfilerS_DBLoader::c4MaxPerIdBytes
        , DVDProfilerS_DBLoader::c4MaxPerIdBytes
      )
    , m_pmdbLoading(nullptr)
    , m_strDoubleNL(L"\r\n\r\n")
    , m_strFileLocPref(L"[ONLINELOCATION=")
    , m_strFileLocPrefDx(L"[ONLINELOCATIOND")
    , m_strMoniker(strMoniker)
    , m_strTrue(L"True")
{
    // Set up our time parser for the date part of the time stamp field
    m_fcolStamp.c4AddElement(tCIDLib::ETimeComps::Year4);
    m_fcolStamp.c4AddElement(tCIDLib::ETimeComps::Punct);
    m_fcolStamp.c4AddElement(tCIDLib::ETimeComps::MonthNum);
    m_fcolStamp.c4AddElement(tCIDLib::ETimeComps::Punct);
    m_fcolStamp.c4AddElement(tCIDLib::ETimeComps::DayNum);

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

TDVDProfilerLoader::~TDVDProfilerLoader()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TDVDProfilerLoader::CompleteLoad()
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
TDVDProfilerLoader::eStatus(tCIDLib::TCard4& c4TitlesDone) const
{
    // Return the status and the count of titles processed
    c4TitlesDone = m_c4TitlesDone;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TDVDProfilerLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


tCIDLib::TVoid TDVDProfilerLoader::ResetDocument()
{
    // We don't need to do anything here
}


const TString& TDVDProfilerLoader::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid
TDVDProfilerLoader::StartLoad(          TMediaDB* const pmdbToLoad
                                , const TString&        strFilename
                                , const TString&        strImagePath)
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

    // Store the incoming info for later use during callbacks
    m_pmdbLoading = pmdbToLoad;
    m_strFilename = strFilename;
    m_strImagePath = strImagePath;

    // Cerate the thumbnails directory from the images directory
    TPathStr pathTmp(strImagePath);
    pathTmp.AddLevel(L"Thumbnails");
    m_strThumbsPath = pathTmp;

    // Reset any of our per-load data
    m_c4TitlesDone = 0;
    m_colDiscs.RemoveAll();
    m_colCatNames.RemoveAll();

    // Flush the media database that we are going to load
    m_pmdbLoading->Reset();

    // Add an 'All Movies' category that all stuff can be put into
    {
        TString strName = facCQCMedia().strMsg(kMedMsgs::midCat_AllMovies);
        TString strUID = strName;
        strUID.ToUpper();

        TMediaCat* pmcatAll = new TMediaCat
        (
            tCQCMedia::EMediaTypes::Movie, strName, strUID
        );
        pmcatAll->c2Id(kCQCMedia::c2CatId_AllMovies);
        m_pmdbLoading->c2AddCategory(pmcatAll, kCIDLib::True);
    }

    // Set the status to indicate we are loading
    m_eStatus = tCQCMedia::ELoadStatus::Loading;

    // Start ourself running in another thread
    Start();
}


//
//  Called by the main thread to make sure we are cleaned up when it needs
//  to shutdown.
//
tCIDLib::TVoid TDVDProfilerLoader::Shutdown()
{
    // If the thread is running, let's ask it to shut down
    try
    {
        ReqShutdownSync(8000);
        eWaitForDeath(3000);
    }

    catch(const TError& errToCatch)
    {
        if (facDVDProfilerS().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);
    }
    m_eStatus = tCQCMedia::ELoadStatus::Failed;
}


// ---------------------------------------------------------------------------
//  TDVDProfilerLoader: Protected, virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TDVDProfilerLoader::DocCharacters(  const   TString&            strChars
                                    , const tCIDLib::TBoolean   bIsCDATA
                                    , const tCIDLib::TBoolean   bIsIgnorable
                                    , const tCIDXML::ELocations eLocation)
{
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the start element, so it's safe to do this.
    //
    if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_AspectRatio)
    {
        m_strFld_AspectRatio.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_ColNumber)
    {
        m_strFld_ColNumber.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Descr)
    {
        TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
        diCur.m_strDescr.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Genre)
    {
        m_strFld_Genre.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Location)
    {
        TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
        diCur.m_strLocInfo.Append(strChars);
        diCur.m_bChanger = kCIDLib::True;
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_MFmt_BluRay)
    {
        m_strFld_MFmt_BluRay.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_MFmt_DVD)
    {
        m_strFld_MFmt_DVD.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_MFmt_HDDVD)
    {
        m_strFld_MFmt_HDDVD.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Notes)
    {
        m_strFld_Notes.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Overview)
    {
        m_strFld_Overview.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Rating)
    {
        m_strFld_Rating.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_RunningTime)
    {
        m_strFld_RunningTime.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Slot)
    {
        TDiscInfo& diCur = m_colDiscs[m_colDiscs.c4ElemCount() - 1];
        if (!diCur.m_strLocInfo.bIsEmpty())
        {
            diCur.m_strLocInfo.Append(L'.');
            diCur.m_strLocInfo.Append(strChars);
            diCur.m_strLocInfo.Append(L"\n");
        }
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_SortTitle)
    {
        m_strFld_SortTitle.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Studio)
    {
        //
        //  There can be multiple studio entries. We store up chars for
        //  the current one and append it to the actual value when we
        //  see the end of the containing element and then flush this
        //  string to start accumulating the next one.
        //
        m_strFld_Studio.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Timestamp)
    {
        m_strFld_Timestamp.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Title)
    {
        m_strFld_Title.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_UniqueID)
    {
        m_strFld_UniqueID.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_UPC)
    {
        m_strFld_UPC.Append(strChars);
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Year)
    {
        m_strFld_Year.Append(strChars);
    }
}


tCIDLib::EExitCodes TDVDProfilerLoader::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::IgnoreBadChars;
        const tCIDXML::EParseFlags  eFlags = tCIDXML::EParseFlags::TagsNText;

        // Kick off the parsing
        m_xprsThis.ParseRootEntity(m_strFilename, eOpts, eFlags);

        //
        //  Ok, it's done successfully, so call the load completion method
        //  on the media database, so that it can do any stuff it has to wait
        //  until all the data is available to do.
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

        // Mark us as done now
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
            if (facDVDProfilerS().bShouldLog(errToCatch))
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
                , m_strFilename
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
                , m_strFilename
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
        facDVDProfilerS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kDVDPrErrs::errcDB_LoadStatusFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TDVDProfilerLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        facDVDProfilerS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Entered EndDocument"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid TDVDProfilerLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_DVD)
    {
        // We have a title's worth of stuff, so process it
        ProcessTitle();
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Genre)
    {
        //
        //  If we are coming out of a Genre field, then add the current
        //  genre text to the list and clear the genre text accumulator.
        //
        //  If it's a slashed or hyphenated genre, replace the slash or
        //  hyphen with a space, so the genre name can wrap when viewed.
        //
        //  It maybe empty, in which case we ignore it.
        //
        if (!m_strFld_Genre.bIsEmpty())
        {
            m_strFld_Genre.bReplaceChar(kCIDLib::chHyphenMinus, kCIDLib::chSpace);
            m_strFld_Genre.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chSpace);
            m_colCatNames.objAdd(m_strFld_Genre);
            m_strFld_Genre.Clear();
        }
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Database)
    {
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Notes)
    {
        if (!m_colDiscs.bIsEmpty())
        {
            // Pull out newline separated lines
            TTextStringInStream strmSrc(&m_strFld_Notes);
            while (!strmSrc.bEndOfStream())
            {
                strmSrc >> m_strTmp;
                m_strTmp.StripWhitespace();

                //
                //  Make sure that it's a reasonable length. It has to be
                //  at least the file smallest file location prefix plus
                //  a quote and closing bracket plus a minimal path length
                //  which is going to be at least 8 characters to be of
                //  any pratical use since it must be fully qualified.
                //
                if (m_strTmp.c4Length() < m_strFileLocPref.c4Length() + 8)
                    continue;

                if (m_strTmp.bStartsWith(m_strFileLocPrefDx))
                {
                    //
                    //  It's for a specific disk, so find it. We have to
                    //  break out the number part of the string.
                    //
                    const tCIDLib::TCard4 c4PrefOfs = m_strFileLocPrefDx.c4Length();
                    tCIDLib::TCard4 c4EqualsOfs;
                    if (!m_strTmp.bFirstOccurrence(kCIDLib::chEquals, c4EqualsOfs)
                    ||  (c4EqualsOfs <= c4PrefOfs))
                    {
                        continue;
                    }
                    m_strTmp.CopyOutSubStr
                    (
                        m_strTmp2, c4PrefOfs, c4EqualsOfs - c4PrefOfs
                    );
                    const tCIDLib::TCard4 c4Ind = m_strTmp2.c4Val(tCIDLib::ERadices::Dec);

                    if (c4Ind && (c4Ind <= m_colDiscs.c4ElemCount()))
                    {
                        TDiscInfo& diTar = m_colDiscs[c4Ind - 1];
                        diTar.m_strLocInfo.CopyInSubStr
                        (
                            m_strTmp
                            , c4EqualsOfs + 2
                            , kCIDLib::c4MaxCard
                        );
                        diTar.m_strLocInfo.DeleteLast();
                        diTar.m_strLocInfo.DeleteLast();
                        diTar.m_bChanger = kCIDLib::False;
                    }
                     else
                    {
                        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
                        {
                            facDVDProfilerS().LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , L"Got an invalid online location disc number"
                                , m_strTmp
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }
                    }
                }
                 else if (m_strTmp.bStartsWith(m_strFileLocPref))
                {
                    TDiscInfo& diTar = m_colDiscs[0];
                    diTar.m_strLocInfo.CopyInSubStr
                    (
                        m_strTmp
                        , m_strFileLocPref.c4Length() + 1
                        , kCIDLib::c4MaxCard
                    );
                    diTar.m_strLocInfo.DeleteLast();
                    diTar.m_strLocInfo.DeleteLast();
                    diTar.m_bChanger = kCIDLib::False;

                    // And break out
                    break;
                }
                 else
                {
                    // It's not something we understand, so break out
                    break;
                }
            }
        }
        m_strFld_Notes.Clear();
    }
     else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Studio)
    {
        // Append the current studio to the overall studio field
        if (!m_strFld_Studio.bIsEmpty() && !m_strFld_Studio.bIsEmpty())
            m_strFld_Studio.Append(L", ");
        m_strFld_Studio.Append(m_strFld_Studio);
        m_strFld_Studio.Clear();
    }

    // Pop our XML path (which verifies the nesting for us
    PopXMLPath(xdeclElem);

}


tCIDLib::TVoid
TDVDProfilerLoader::HandleXMLError( const   tCIDLib::TErrCode   errcToPost
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
        facDVDProfilerS().LogMsg
        (
            strSystemId
            , c4CurLine
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid TDVDProfilerLoader::StartDocument(const TXMLEntitySrc&)
{
    //
    //  Don't really need to do anything. We are initialized before each
    //  call to do a parse, so our data is already ready. If we are in
    //  high verbosity level, log a trace message.
    //
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facDVDProfilerS().LogMsg
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
TDVDProfilerLoader::StartTag(       TXMLParserCore&     xprsSrc
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

    // Push the new element
    const TString& strQName = xdeclElem.strFullName();
    m_strCurXMLPath.Append(L'/');
    m_strCurXMLPath.Append(strQName);

    if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_DVD)
    {
        // Reset any per-item data
        m_colDiscs.RemoveAll();
        m_colCatNames.RemoveAll();

        m_strFld_AspectRatio.Clear();
        m_strFld_Cast.Clear();
        m_strFld_ColNumber.Clear();
        m_strFld_CreditSubtype.Clear();
        m_strFld_Director.Clear();
        m_strFld_FirstName.Clear();
        m_strFld_Genre.Clear();
        m_strFld_LastName.Clear();
        m_strFld_LeadActor.Clear();
        m_strFld_MFmt_BluRay.Clear();
        m_strFld_MFmt_DVD.Clear();
        m_strFld_MFmt_HDDVD.Clear();
        m_strFld_Notes.Clear();
        m_strFld_Overview.Clear();
        m_strFld_Rating.Clear();
        m_strFld_Review.Clear();
        m_strFld_RunningTime.Clear();
        m_strFld_SortTitle.Clear();
        m_strFld_Studio.Clear();
        m_strFld_Timestamp.Clear();
        m_strFld_Title.Clear();
        m_strFld_UniqueID.Clear();
        m_strFld_UPC.Clear();
        m_strFld_Year.Clear();
    }
     else
    {
        // If a new disc, then add a disc info object
        if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Disc)
            m_colDiscs.objAdd(TDiscInfo());

        //
        //  Watch for some stuff we need to pull from attributes on some
        //  elements.
        //
        if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Actor)
        {
            bFindAttr
            (
                DVDProfilerS_DBLoader::strAttr_FirstName
                , colAttrList
                , c4AttrCnt
                , m_strTmp
            );
            bFindAttr
            (
                DVDProfilerS_DBLoader::strAttr_LastName
                , colAttrList
                , c4AttrCnt
                , m_strTmp2
            );

            //
            //  If this is the first actor, then store it as the name of the
            //  lead actor. Else, we want to put out a trailing comma to the
            //  cast string in preperartion for the new item.
            //
            if (m_strFld_Cast.bIsEmpty())
            {
                m_strFld_LeadActor = m_strTmp;
                if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                    m_strFld_LeadActor.Append(L' ');
                m_strFld_LeadActor.Append(m_strTmp2);
            }
             else if (!m_strTmp.bIsEmpty() || !m_strTmp2.bIsEmpty())
            {
                //
                //  We have a name, and it's not the first one, so append a
                //  comma to the cast to make way for the next one.
                //
                m_strFld_Cast.Append(L", ");
            }

            m_strFld_Cast.Append(m_strTmp);
            if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                m_strFld_Cast.Append(kCIDLib::chSpace);
            m_strFld_Cast.Append(m_strTmp2);
        }
         else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Credit)
        {
            // If there's a credit sub-type attr, pull that out
            if (bFindAttr(DVDProfilerS_DBLoader::strAttr_CreditSubtype, colAttrList, c4AttrCnt, m_strTmp)
            &&  (m_strTmp == DVDProfilerS_DBLoader::strAttr_Director))
            {
                bFindAttr
                (
                    DVDProfilerS_DBLoader::strAttr_FirstName
                    , colAttrList
                    , c4AttrCnt
                    , m_strTmp
                );
                bFindAttr
                (
                    DVDProfilerS_DBLoader::strAttr_LastName
                    , colAttrList
                    , c4AttrCnt
                    , m_strTmp2
                );

                m_strFld_Director = m_strTmp;
                if (!m_strTmp.bIsEmpty() && !m_strTmp2.bIsEmpty())
                    m_strFld_Director.Append(L' ');
                m_strFld_Director.Append(m_strTmp2);
            }
        }
         else if (m_strCurXMLPath == DVDProfilerS_DBLoader::strElem_Review)
        {
            // Get the movie review value
            bFindAttr
            (
                DVDProfilerS_DBLoader::strAttr_Film
                , colAttrList
                , c4AttrCnt
                , m_strFld_Review
            );
        }
    }
}



// ---------------------------------------------------------------------------
//  TDVDProfilerLoader: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We need to get some info out of the attributes of some start elements,
//  so we have this helper to search for the attribute value we want from
//  the list of attributes that the start element callback got.
//
tCIDLib::TBoolean
TDVDProfilerLoader::bFindAttr(  const   TString&            strQName
                                , const TVector<TXMLAttr>&  colAttrList
                                , const tCIDLib::TCard4     c4ListSz
                                ,       TString&            strValue)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ListSz; c4Index++)
    {
        if (colAttrList[c4Index].strQName() == strQName)
        {
            strValue = colAttrList[c4Index].strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  Pops the last element off of the current XML path and verifies that
//  that element is the one that we got that's ending. We consider any
//  errors here to be fatal and exit.
//
tCIDLib::TVoid TDVDProfilerLoader::PopXMLPath(const TXMLElemDecl& xdeclEnding)
{
    // If it's empty, then we've underflowed
    if (m_strCurXMLPath.bIsEmpty())
    {
        facDVDProfilerS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDVDPrErrs::errcDB_BadNesting
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Find the last separator
    tCIDLib::TCard4 c4Ofs;
    if (!m_strCurXMLPath.bLastOccurrence(L'/', c4Ofs))
    {
        facDVDProfilerS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDVDPrErrs::errcDB_BadNesting
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Get the bit after the separate to a tmp and compare to the passed element
    m_strCurXMLPath.CopyOutSubStr(m_strTmp, c4Ofs + 1);
    if (m_strTmp != xdeclEnding.strFullName())
    {
        facDVDProfilerS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kDVDPrErrs::errcDB_BadNesting2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , xdeclEnding.strFullName()
            , m_strTmp
        );
    }

    // Looks ok, cap the path at the divider
    m_strCurXMLPath.CapAt(c4Ofs);
}


// We have a title's worth o' info, so process it
tCIDLib::TVoid TDVDProfilerLoader::ProcessTitle()
{
    // If we found no discs for this title, then nothing to do
    if (m_colDiscs.bIsEmpty())
        return;

    // If the id is empty, then we can't do this one
    if (m_strFld_UniqueID.bIsEmpty())
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"DVD unique ID was empty, skipping title ended at line %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(m_xprsThis.c4CurLine())
            );
        }
        return;
    }

    //
    //  And now we need to process the data we've collected on the current
    //  title. If it already exists, then ignore this one since it's a dup.
    //
    if (m_pmdbLoading->pmtsByUniqueId(  tCQCMedia::EMediaTypes::Movie
                                        , m_strFld_UniqueID
                                        , kCIDLib::False))
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"DVD with ID %(1) is already in the database. Skipping title ended at line %(2)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strFld_UniqueID
                , TCardinal(m_xprsThis.c4CurLine())
            );
        }
        return;
    }

    // We need to look for the art for this one, and see if it's in the database
    const TMediaImg* pmimgArt = m_pmdbLoading->pmimgByUniqueId
    (
        tCQCMedia::EMediaTypes::Movie, m_strFld_UniqueID, kCIDLib::False
    );

    if (!pmimgArt)
    {
        //
        //  Not there so far, so let's see if either the large or thumb art exists.
        //  If so, then add the image.
        //
        m_pathTmp = m_strImagePath;
        m_pathTmp.AddLevel(m_strFld_UniqueID);
        m_pathTmp.Append(L'f');
        m_pathTmp.AppendExt(L"jpg");

        m_pathTmp2 = m_strThumbsPath;
        m_pathTmp2.AddLevel(m_strFld_UniqueID);
        m_pathTmp2.Append(L'f');
        m_pathTmp2.AppendExt(L"jpg");

        if (TFileSys::bExists(m_pathTmp) || TFileSys::bExists(m_pathTmp2))
        {
            //
            //  We have to create a persistent id. This requires that we read in
            //  the files and hash them. Since there's separate art, we have to
            //  do both of them.
            //
            TString strLrgPerId;
            TString strSmlPerId;
            if (TFileSys::bExists(m_pathTmp))
            {
                try
                {
                    TBinaryFile flImg(m_pathTmp);
                    flImg.Open
                    (
                        tCIDLib::EAccessModes::Excl_Read
                        , tCIDLib::ECreateActs::OpenIfExists
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );

                    // Read up to the max bytes
                    const tCIDLib::TCard4 c4Sz = flImg.c4ReadBuffer
                    (
                        m_mbufPerId
                        , DVDProfilerS_DBLoader::c4MaxPerIdBytes
                        , tCIDLib::EAllData::OkIfNotAll
                    );
                    facCQCMedia().CreatePersistentId(m_mbufPerId, c4Sz, strLrgPerId);
                }

                catch(...)
                {
                }
            }

            if (TFileSys::bExists(m_pathTmp2))
            {
                try
                {
                    TBinaryFile flImg(m_pathTmp2);
                    flImg.Open
                    (
                        tCIDLib::EAccessModes::Excl_Read
                        , tCIDLib::ECreateActs::OpenIfExists
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );

                    // Read up to the max bytes
                    const tCIDLib::TCard4 c4Sz = flImg.c4ReadBuffer
                    (
                        m_mbufPerId
                        , DVDProfilerS_DBLoader::c4MaxPerIdBytes
                        , tCIDLib::EAllData::OkIfNotAll
                    );
                    facCQCMedia().CreatePersistentId(m_mbufPerId, c4Sz, strSmlPerId);
                }

                catch(...)
                {
                }
            }

            TMediaImg* pmimgNew = new TMediaImg
            (
                tCQCMedia::EMediaTypes::Movie, m_pathTmp, m_pathTmp2
            );
            pmimgNew->strUniqueId(m_strFld_UniqueID);
            pmimgNew->SetPersistentId(strLrgPerId, tCQCMedia::ERArtTypes::LrgCover);
            pmimgNew->SetPersistentId(strSmlPerId, tCQCMedia::ERArtTypes::SmlCover);

            m_pmdbLoading->c2AddImage(pmimgNew);

            pmimgArt = pmimgNew;
        }
    }
     else
    {
        if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The cover art file was not found for title '%(1)'"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strFld_Title
            );
        }
    }

    //
    //  We don't get separate year/duration values for each disc, so just
    //  get them out now and use them for all the discs below. Not optimal
    //  but the best we can do.
    //
    tCIDLib::TCard4     c4Duration = 0;
    tCIDLib::TCard4     c4Year = 0;
    if (!m_strFld_RunningTime.bIsEmpty())
    {
        if (!m_strFld_RunningTime.bToCard4(c4Duration, tCIDLib::ERadices::Dec))
            c4Duration = 0;

        // It's in minutes, we want seconds
        c4Duration *= 60;
    }

    if (!m_strFld_Year.bIsEmpty())
    {
        if (!m_strFld_Year.bToCard4(c4Year, tCIDLib::ERadices::Dec))
            c4Year = 0;
    }

    //
    //  Create a new media title set for this guy. Put a janitor on it until we are
    //  sure we are going to keep it and we have it all filled in.
    //
    if (m_strFld_SortTitle.bIsEmpty())
        facCQCMedia().bMakeSortTitle(m_strFld_Title, m_strFld_SortTitle);
    TMediaTitleSet* pmtsNew = new TMediaTitleSet
    (
        m_strFld_Title
        , m_strFld_UniqueID
        , m_strFld_SortTitle
        , tCQCMedia::EMediaTypes::Movie
    );
    TJanitor<TMediaTitleSet> janSet(pmtsNew);

    // Set some more title level info
    pmtsNew->strArtist(m_strFld_Director);

    //
    //  Figure out what media format. We'll take the highest quality one
    //  that is set.
    //
    if (m_strFld_MFmt_BluRay.bCompareI(m_strTrue))
        m_strMediaFmt = L"BluRay";
    else if (m_strFld_MFmt_HDDVD.bCompareI(m_strTrue))
        m_strMediaFmt = L"HDDVD";
    else if (m_strFld_MFmt_DVD.bCompareI(m_strTrue))
        m_strMediaFmt = L"DVD";
    else
        m_strMediaFmt.Clear();

    //
    //  Create collection objects for each disc we found and add them to the
    //  title set.
    //
    tCIDLib::TCard4 c4DiscCnt = m_colDiscs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DiscCnt; c4Index++)
    {
        TDiscInfo& diCur = m_colDiscs[c4Index];
        diCur.m_strLocInfo.StripWhitespace();
        if (diCur.m_strLocInfo.bIsEmpty())
            continue;

        // Just use the title set unique id plus a suffix for the collections
        m_strTmp = pmtsNew->strUniqueId();
        m_strTmp.AppendFormatted(pmtsNew->c4ColCount());
        TMediaCollect* pmcolTar = new TMediaCollect
        (
            diCur.m_strDescr
            , m_strTmp
            , c4Year
            , diCur.m_bChanger ? tCQCMedia::ELocTypes::Changer
                               : tCQCMedia::ELocTypes::FileCol
            , tCQCMedia::EMediaTypes::Movie
        );
        TJanitor<TMediaCollect> janCol(pmcolTar);

        //
        //  Scan through the genres in the genre list we found for this guy
        //  and see if they exist yet. Create them if not, and add them
        //  to this collection where not already added.
        //
        //  The unique id for categories is the name upper cased.
        //
        const tCIDLib::TCard4 c4CatCnt = m_colCatNames.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
        {
            const TString& strCat = m_colCatNames[c4Index];
            m_strTmp = strCat;
            m_strTmp.ToUpper();
            const TMediaCat* pmcatCur = m_pmdbLoading->pmcatByUniqueId
            (
                tCQCMedia::EMediaTypes::Movie, m_strTmp, kCIDLib::False
            );

            if (!pmcatCur)
            {
                TMediaCat* pmcatNew = new TMediaCat
                (
                    tCQCMedia::EMediaTypes::Movie, strCat, m_strTmp
                );
                m_pmdbLoading->c2AddCategory(pmcatNew);
                pmcatCur = pmcatNew;
            }

            if (pmcatCur)
                pmcolTar->bAddCategory(*pmcatCur);
        }

        // And put it into the all movies category
        pmcolTar->bAddCategory(kCQCMedia::c2CatId_AllMovies);

        // Store the other info on the collection
        pmcolTar->c4Duration(c4Duration);

        // Set the media format
        pmcolTar->strMediaFormat(m_strMediaFmt);

        //
        //  These guys use two new lines between paragraphs, which makes the
        //  text look bad when viewed, with big gaps. So we do a search for
        //  them and replace them with single new lines.
        //
        //  Note that they were expanded from entity references, they will
        //  remain as CR\LF pairs, and won't have been reduced to LFs by
        //  either the XML parser or our text streams.
        //
        tCIDLib::TCard4 c4Ofs;
        if (m_strFld_Overview.bFirstOccurrence(m_strDoubleNL, c4Ofs))
        {
            do
            {
                m_strFld_Overview.PutAt(c4Ofs + 0, kCIDLib::chSpace);
                m_strFld_Overview.PutAt(c4Ofs + 1, kCIDLib::chSpace);
                m_strFld_Overview.PutAt(c4Ofs + 2, kCIDLib::chLF);
                m_strFld_Overview.PutAt(c4Ofs + 3, kCIDLib::chLF);
            }
            while (m_strFld_Overview.bNextOccurrence(m_strDoubleNL, c4Ofs));
        }
        pmcolTar->strDescr(m_strFld_Overview);

        // Set all the other collection info we gathered
        pmcolTar->strArtist(m_strFld_Director);
        pmcolTar->strAspectRatio(m_strFld_AspectRatio);
        pmcolTar->strCast(m_strFld_Cast);
        pmcolTar->strLabel(m_strFld_Studio);
        pmcolTar->strRating(m_strFld_Rating);
        pmcolTar->strLeadActor(m_strFld_LeadActor);

        //
        //  Parse out the date added. It's a date (YYYY-MM-DD) then a T
        //  character and a time. We just cap it at the T and parse the
        //  timestamp.
        //
        if (m_strFld_Timestamp.bCapAtChar(kCIDLib::chLatin_T))
        {
            if (m_tmStamp.bParseFromText(m_strFld_Timestamp, m_fcolStamp))
                pmtsNew->enctAdded(m_tmStamp.enctTime());
        }

        //
        //  The review should be a number from 0 to 9, which we convert
        //  to our 1 to 10 range.
        //
        tCIDLib::TCard4 c4UserRating;
        if (m_strFld_Review.bToCard4(c4UserRating, tCIDLib::ERadices::Dec))
        {
            c4UserRating++;
            if (c4UserRating > kCQCMedia::c4MaxUserRating)
                c4UserRating = kCQCMedia::c4MaxUserRating;
            pmtsNew->c4UserRating(c4UserRating);
        }

        //
        //  The collection number we set as our sequence number. It should
        //  be a positive number.
        //
        tCIDLib::TCard4 c4ColNum;
        if (m_strFld_ColNumber.bToCard4(c4ColNum, tCIDLib::ERadices::Dec))
            pmtsNew->c4SeqNum(c4ColNum);

        // Set the art id, if any
        if (pmimgArt)
            pmcolTar->c2ArtId(pmimgArt->c2Id());

        //
        //  Set the location info on the collection. This driver either
        //  uses per-disc file or changer location type, both of which
        //  imply a single location, stored at the collection level.
        //
        pmcolTar->strLocInfo(diCur.m_strLocInfo);

        // Make a unique id that's the collection id plus a suffix
        m_strTmp = pmcolTar->strUniqueId();
        m_strTmp.Append(L"-1");

        // Add a single chapter item that represents the whole movie contents
        TMediaItem* pmitemChapter = new TMediaItem
        (
            pmcolTar->strName()
            , m_strTmp
            , TString::strEmpty()
            , tCQCMedia::EMediaTypes::Movie
        );
        pmitemChapter->strArtist(m_strFld_Director);
        m_pmdbLoading->c2AddItem(pmitemChapter);
        pmcolTar->bAddItem(*pmitemChapter);

        // Add teh collection to the database, then to the set once the id is set
        m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());
        pmtsNew->bAddCollect(*pmcolTar);
    }

    // Now we can add the title
    m_pmdbLoading->c2AddTitle(janSet.pobjOrphan());
    m_c4TitlesDone++;
}

