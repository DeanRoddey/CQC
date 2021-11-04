//
// FILE NAME: PlexRepoS_DBLoader.cpp
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
RTTIDecls(TPlexRepoLoader,TThread)
StdEnumTricks(tPlexRepoS::EAttrs)



// ---------------------------------------------------------------------------
//   CLASS: TPlexRepoLoader
//  PREFIX: dbl
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPlexRepoLoader: Constructors and Destructor
// ---------------------------------------------------------------------------
TPlexRepoLoader::TPlexRepoLoader(const  TString&        strMoniker
                                , const TIPEndPoint&    ipepSrv) :


    TThread(strMoniker + TString(L"_DBLoadThread"))
    , m_c4TitlesDone(0)
    , m_colAttrVals(tPlexRepoS::EAttrs::Count)
    , m_eMediaType(tCQCMedia::EMediaTypes::Movie)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_eVerbose(tCQCKit::EVerboseLvls::Off)
    , m_fcolValStack(16UL)
    , m_ipepServer(ipepSrv)
    , m_mbufTmp(128 * 1024, 16 * (1024 * 1024), 1024 * 1024)
    , m_pcdsSrv(nullptr)
    , m_pmdbLoading(0)
    , m_strMoniker(strMoniker)
{
    //
    //  Create a validator, telling it about the parser. Tell the parser about
    //  the validator, which he adopts. We won't ever use the validator here but
    //  it has to be set.
    //
    TDTDValidator* pvalNew = new TDTDValidator(&m_xprsDB);
    m_xprsDB.pxvalValidator(pvalNew);

    //
    //  Set ourself as a handler on the core parser, so that we will get callbacks
    //  from it.
    //
    m_xprsDB.pmxevDocEvents(this);
    m_xprsDB.pmxevErrorEvents(this);
}

TPlexRepoLoader::~TPlexRepoLoader()
{
    // We use a janitor on the data source, so it's already cleaned up
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TPlexRepoLoader::CompleteLoad()
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
TPlexRepoLoader::eStatus(tCIDLib::TCard4& c4TitlesDone) const
{
    // Return the status and the count of titles processed
    c4TitlesDone = m_c4TitlesDone;
    return m_eStatus;
}


tCQCKit::EVerboseLvls
TPlexRepoLoader::eVerboseLevel(const tCQCKit::EVerboseLvls eToSet)
{
    m_eVerbose = eToSet;
    return m_eVerbose;
}


const TString& TPlexRepoLoader::strDBSerialNum() const
{
    return m_strDBSerialNum;
}


//
//  Starts the database load process by storing the passed database to laod
//  and the file name to load from, and then starting ourself up as a separate
//  thread that does the loading.
//
tCIDLib::TVoid TPlexRepoLoader::StartLoad(TMediaDB* const pmdbToLoad)
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
tCIDLib::TVoid TPlexRepoLoader::Shutdown()
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
//  TPlexRepoLoader: Protected, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TPlexRepoLoader::DocCharacters( const   TString&            strChars
                                , const tCIDLib::TBoolean   bIsCDATA
                                , const tCIDLib::TBoolean   bIsIgnorable
                                , const tCIDXML::ELocations eLocation
                                , const tCIDLib::TBoolean)
{
    //
    //  If the m_eField value is set, then we are expecting a value for
    //  one of the fields.
    //
    //  Note that it is theoretically possible to get chars in more than
    //  one chunk, so we append here. The target strings were all cleared
    //  at the start element, so it's safe to do this.
    //
}


// The thread entry point for the loader
tCIDLib::EExitCodes TPlexRepoLoader::eProcess()
{
    // Let the calling thread go
    Sync();

    try
    {
        //
        //  Try to create a socket based data source. This will be used for any
        //  HTTP queries against the server. We put a janitor on it, which will
        //  initialize it and terminate it. This lets us use a single connection
        //  to the server for all of the transactions.
        //
        m_pcdsSrv = new TCIDSockStreamDataSrc(m_ipepServer);
        TCIDDataSrcJan janDSrv(m_pcdsSrv, tCIDLib::EAdoptOpts::Adopt, kCIDLib::True);

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
            TChunkedBinOutStream strmTar(kCIDLib::c4Sz_32M);
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
                , m_ipepServer
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
                , m_ipepServer
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
        facPlexRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The load operation failed"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Internal
        );
        m_eStatus = tCQCMedia::ELoadStatus::Failed;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TPlexRepoLoader::EndDocument(const TXMLEntitySrc& xsrcOfRoot)
{
    if (m_eVerbose >= tCQCKit::EVerboseLvls::Medium)
    {
        facPlexRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Entered EndDocument"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


tCIDLib::TVoid TPlexRepoLoader::EndTag(const TXMLElemDecl& xdeclElem)
{
    const TString& strQName = xdeclElem.strFullName();

    // The top of the value stack should be of this type
    const tPlexRepoS::EElems eType = ePopValStack(strQName);

    if (eType == tPlexRepoS::EElems::Video)
        StoreTitleSet();
}


tCIDLib::TVoid
TPlexRepoLoader::HandleXMLError(const   tCIDLib::TErrCode   errcToPost
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
        facPlexRepoS().LogMsg
        (
            strSystemId
            , c4CurLine
            , strText
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


//
//  Nothing to do for us. All our resetting is done in StartTag when we see a
//  new Video tab, which is starting a new title set.
//
tCIDLib::TVoid TPlexRepoLoader::ResetDocument()
{
}


tCIDLib::TVoid TPlexRepoLoader::StartDocument(const TXMLEntitySrc&)
{
    //
    //  Don't really need to do anything. We are initialized before each
    //  call to do a parse, so our data is already ready. If we are in
    //  high verbosity level, log a trace message.
    //
    if (m_eVerbose >= tCQCKit::EVerboseLvls::High)
    {
        facPlexRepoS().LogMsg
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
TPlexRepoLoader::StartTag(          TXMLParserCore&     xprsSrc
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
    //  Figure out the new element type and push it, doing whatever is
    //  appropriate at the start of that element type, if anything.
    //
    const tPlexRepoS::EElems eElem = tPlexRepoS::eAltXlatEElems(xdeclElem.strFullName());

    switch(eElem)
    {
        case tPlexRepoS::EElems::Director :
            // If we haven't stored a director value yet, then do it
            if (m_colAttrVals[tPlexRepoS::EAttrs::Director].bIsEmpty())
            {
                m_colAttrVals[tPlexRepoS::EAttrs::Director]
                                        = strGetAttr(colAttrList, L"tag");
            }
            break;

        case tPlexRepoS::EElems::Genre :
        {
            // Add the tag attribute value to our category list
            m_colCatList.objAdd(strGetAttr(colAttrList, L"tag"));
            break;
        }

        case tPlexRepoS::EElems::Media :
        {
            // Just store away any more attributes we care abuot
            StoreAttrs(colAttrList);

            // This is a special case one
            m_colAttrVals[tPlexRepoS::EAttrs::Id] = strGetAttr(colAttrList, L"id");
            break;
        }

        case tPlexRepoS::EElems::Role :
        {
            // Add the tag attribute value to our cast list
            m_colCastList.objAdd(strGetAttr(colAttrList, L"tag"));
            break;
        }

        case tPlexRepoS::EElems::Video :
        {
            //
            //  We need to reset all of the per video stuff to start accumulating
            //  again.
            //
            m_colCastList.RemoveAll();
            m_colCatList.RemoveAll();

            //
            //  Store any stuff that comes in the main video element. Clear all
            //  of the attribute strings first so that, at the end, if any are
            //  empty we know we never saw them.
            //
            tPlexRepoS::EAttrs eAttr = tPlexRepoS::EAttrs::Min;
            while (eAttr < tPlexRepoS::EAttrs::Count)
            {
                m_colAttrVals[eAttr].Clear();
                eAttr++;
            }

            StoreAttrs(colAttrList);
            break;
        }

        default :
            break;
    };

    // Push the new element into the stack
    m_fcolValStack.Push(eElem);
}


// ---------------------------------------------------------------------------
//  TPlexRepoLoader: Private, non=virtual methods
// ---------------------------------------------------------------------------

//
//  We get pop the current top of stack and make sure that element type has the
//  indicated Qname. We then return the type we poped off.
//
tPlexRepoS::EElems TPlexRepoLoader::ePopValStack(const TString& strQName)
{
    if (m_fcolValStack.bIsEmpty())
    {
        // There's been an underflow somehow
        facPlexRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kPlexErrs::errcXML_NestingUnderflow
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Underflow
            , strQName
        );
    }

    // Check the top of stack
    const tPlexRepoS::EElems ePopped = m_fcolValStack.tPop();
    const tPlexRepoS::EElems eExpected = tPlexRepoS::eAltXlatEElems(strQName);

    if (ePopped != eExpected)
    {
        facPlexRepoS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kPlexErrs::errcXML_NestingMatch
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , strQName
            , tPlexRepoS::strXlatEElems(ePopped)
        );
    }

    return ePopped;
}


//
//  This is called to parse the /library/sections document. We find those that
//  are marked as movies and we give back the section ids for those. The caller
//  will then process those sections.
//
tCIDLib::TVoid TPlexRepoLoader::FindSections(tCIDLib::TCardList& fcolSections)
{
    const tCIDXML::EParseFlags eFlags = tCIDXML::EParseFlags::TagsNText;

    // Build up the URL
    TString strURL;
    TURL::BuildURL
    (
        tCIDSock::strAltXlatEProtos(tCIDSock::EProtos::HTTP)
        , TString::strEmpty()
        , TString::strEmpty()
        , m_ipepServer.strAsText()
        , m_ipepServer.ippnThis()
        , L"/library/sections"
        , TString::strEmpty()
        , TString::strEmpty()
        , nullptr
        , strURL
        , tCIDSock::EExpOpts::Encode
    );

    tCIDXML::TEntitySrcRef esrSects(new TURLEntitySrc(strURL));
    if (!m_xptrsTree.bParseRootEntity(esrSects, tCIDXML::EParseOpts::None, eFlags))
    {
        const TXMLTreeParser::TErrInfo& erriFirst= m_xptrsTree.erriFirst();
        facPlexRepoS().ThrowErr
        (
            erriFirst.strSystemId()
            , erriFirst.c4Line()
            , kPlexErrs::errcXML_ParseErr
            , erriFirst.strText()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
    }

    // Get the document element out so we can start navigating.
    const TXMLTreeElement& xtnodeRoot = m_xptrsTree.xtdocThis().xtnodeRoot();

    // Let's iterate the child elements, each of which is a Directory element
    TString strVal;
    const tCIDLib::TCard4 c4ChildCount = xtnodeRoot.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        // Skip non-elements
        const TXMLTreeNode& xtnodeCur = xtnodeRoot.xtnodeChildAt(c4Index);
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeDir = static_cast<const TXMLTreeElement&>(xtnodeCur);

        //
        //  If a Directory and it's of type movie, then grab it's id and store
        //  it away.
        //
        if (xtnodeDir.strQName() == L"Directory")
        {
            tCIDLib::TCard4 c4TypeAt, c4IdAt;
            if (xtnodeDir.bAttrExists(L"type", c4TypeAt)
            &&  xtnodeDir.bAttrExists(L"key", c4IdAt))
            {
                fcolSections.c4AddElement(xtnodeDir.xtattrAt(c4IdAt).c4ValueAs());
            }
        }
    }
}


//
//  This is called to kick start the loading process. It's called from our bgn
//  thread after a little initial work is done.
//
tCIDLib::TVoid TPlexRepoLoader::ImportDatabase()
{
    try
    {
        //
        //  First we need to parse the library/sections list to find the sections
        //  that we want to load. There might be more than one. These are essentially
        //  separate directories that contain media.
        //
        tCIDLib::TCardList fcolSections;
        FindSections(fcolSections);

        // For each section, process it
        const tCIDLib::TCard4 c4Count = fcolSections.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            ProcessSection(fcolSections[c4Index]);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  We have to build up the URL for the passed path, of which we get the path part
//  of the URL. He passes us a data source to use, so that we can use a persistent
//  connection for all of the images to be more efficient.
//
//  We pre-load the thumbs, but let the driver later fault in the large art upon
//  request. But, if we don't have a thumb path, then we get the large art and
//  let the driver fault in the thumb by scaling it down
//
//  If we have a thumb path, but it fails, we leave both for later.
//
TMediaImg*
TPlexRepoLoader::pmimgDownloadArt(  const   TString&    strImgPath
                                    , const TString&    strThumbPath)
{
    tCIDLib::TBoolean   bLargeArt = kCIDLib::False;
    tCIDLib::TCard4     c4ContLen = 0;

    TString strLoadPath;
    if (strThumbPath.bIsEmpty())
    {
        bLargeArt = kCIDLib::True;
        strLoadPath = strImgPath;
    }
     else
    {
        strLoadPath = strThumbPath;
    }

    TURL urlImg;
    urlImg.Set
    (
        tCIDSock::EProtos::HTTP
        , TString::strEmpty()
        , TString::strEmpty()
        , m_ipepServer.strAsText()
        , m_ipepServer.ippnThis()
        , strLoadPath
        , TString::strEmpty()
        , TString::strEmpty()
        , m_colHTTPQVals
        , kCIDLib::False
    );

    try
    {
        tCIDNet::EHTTPCodes     eCodeType;
        TString                 strRepText;
        TString                 strContType;

        const tCIDLib::TCard4 c4Res = m_httpcImgs.c4SendGet
        (
            m_pcdsSrv
            , urlImg
            , TTime::enctNowPlusSecs(5)
            , L"CQC Plex Repo"
            , L"image/*"
            , eCodeType
            , strRepText
            , m_colOutHdrLines
            , strContType
            , m_mbufTmp
            , c4ContLen
            , kCIDLib::False
            , m_colInHdrLines
        );

        if (c4Res != kCIDNet::c4HTTPStatus_OK)
        {
            if (m_eVerbose > tCQCKit::EVerboseLvls::Low)
            {
                facPlexRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kPlexMsgs::midStatus_ImgFailed
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_colAttrVals[tPlexRepoS::EAttrs::Title]
                    , TString(L"exception")
                );
            }

            // Make sure the content is zero for below
            c4ContLen = 0;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facPlexRepoS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kPlexMsgs::midStatus_ImgFailed
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_colAttrVals[tPlexRepoS::EAttrs::Title]
            , TString(L"exception")
        );

        // Make sure the content is zero for below
        c4ContLen = 0;
    }

    // Create a new media image for this guy
    TMediaImg* pmimgRet = new TMediaImg(m_eMediaType, strImgPath, strThumbPath);

    //
    //  Set the main art path as the unique id. The persistent ids have to be
    //  something that can be used as a file name, so we need to tweak the
    //  Plex image path to create a viable name.
    //
    //  If the thumb path is empty, then use the name image one for that as
    //  wwell. We remove the /library/metadata/ prefix. Then we keep the rest
    //  but with the forward slashes replaced with underscores.
    //
    pmimgRet->strUniqueId(strImgPath);

    m_strTmp1 = strImgPath;
    m_strTmp1.Cut(0, 18);
    m_strTmp1.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chUnderscore);
    pmimgRet->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::LrgCover);

    // If we have a thumb path, process it, else just keep the main image one
    if (!strThumbPath.bIsEmpty())
    {
        m_strTmp1 = strThumbPath;
        m_strTmp1.Cut(0, 18);
        m_strTmp1.bReplaceChar(kCIDLib::chForwardSlash, kCIDLib::chUnderscore);
    }
    pmimgRet->SetPersistentId(m_strTmp1, tCQCMedia::ERArtTypes::SmlCover);

    // If we got some content, the store it
    if (c4ContLen)
    {
        //
        //  If doing small art, scale it down because this guy returns way
        //  to large images for thumbs.
        //
        if (!bLargeArt)
        {
            try
            {
                c4ContLen = facCIDImgFact().c4CompactToJPEG
                (
                    m_mbufTmp, c4ContLen, TSize(kCQCMedia::c4DefThumbSz)
                );
            }

            catch(TError& errToCatch)
            {
                if (m_eVerbose > tCQCKit::EVerboseLvls::Low)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);

                    facPlexRepoS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kPlexErrs::errcData_BadArt
                        , strImgPath
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }

                // Zero the length and we won't store anything below
                c4ContLen = 0;
            }
        }

        // If we still have something, store it
        if (c4ContLen)
        {
            pmimgRet->SetArt
            (
                c4ContLen
                , m_mbufTmp
                , bLargeArt ? tCQCMedia::ERArtTypes::LrgCover
                            : tCQCMedia::ERArtTypes::SmlCover
            );
        }
    }

    // Add it to the database
    m_pmdbLoading->c2AddImage(pmimgRet);
    return pmimgRet;
}


//
//  For each movie section found, this is called to process the movies in that
//  section. We build up the 'all movies' URL for this section and iterate all
//  of the movies in it.
//
tCIDLib::TVoid TPlexRepoLoader::ProcessSection(const tCIDLib::TCard4 c4Index)
{
    // Build up the path part of the URL for this section
    TString strSecPath(L"/library/sections/");
    strSecPath.AppendFormatted(c4Index);
    strSecPath.Append(L"/all");

    // Now build up the full URL
    TString strURL;
    TURL::BuildURL
    (
        tCIDSock::strAltXlatEProtos(tCIDSock::EProtos::HTTP)
        , TString::strEmpty()
        , TString::strEmpty()
        , m_ipepServer.strAsText()
        , m_ipepServer.ippnThis()
        , strSecPath
        , TString::strEmpty()
        , TString::strEmpty()
        , nullptr
        , strURL
        , tCIDSock::EExpOpts::Encode
    );


    // And kick off the parser
    const tCIDXML::EParseOpts   eOpts  = tCIDXML::EParseOpts::IgnoreBadChars;
    const tCIDXML::EParseFlags  eFlags = tCIDXML::EParseFlags::TagsNText;

    // We can use a URL entity ref and let the XML parser do the reading
    tCIDXML::TEntitySrcRef esrSect(new TURLEntitySrc(strURL));

    //
    //  Kick off the parsing. When we get back here, all of the data will have
    //  been processed.
    //
    m_xprsDB.ParseRootEntity(esrSect, eOpts, eFlags);
}


//
//  In some cases, we don't get the attributes by a specific name, but it's a
//  generic attribute name and the element indicates what it is. This is called
//  to get that type of attribute. The caller knows where to put it.
//
const TString&
TPlexRepoLoader::strGetAttr(const   TVector<TXMLAttr>&  colAttrList
                            , const TString&            strToFind)
{
    const tCIDLib::TCard4 c4Count = colAttrList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLAttr& xattrCur = colAttrList[c4Index];
        if (xattrCur.strQName() == strToFind)
            return xattrCur.strValue();
    }
    return TString::strEmpty();
}


//
//  This is called to store away any attributes from a start element. We only really
//  deal with 2, the Video and Media elements.
//
tCIDLib::TVoid TPlexRepoLoader::StoreAttrs(const TVector<TXMLAttr>& colAttrList)
{
    const tCIDLib::TCard4 c4Count = colAttrList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TXMLAttr& xattrCur = colAttrList[c4Index];
        const tPlexRepoS::EAttrs eAttr = tPlexRepoS::eAltXlatEAttrs(xattrCur.strQName());

        // If it's one we care about, process it
        if (eAttr < tPlexRepoS::EAttrs::Count)
            m_colAttrVals[eAttr] = xattrCur.strValue();
    }
}


//
//  At the end of a Video element, we are called here and, if we have gotten enuogh
//  info, we create a new title set and a collection within it.
//
tCIDLib::TVoid TPlexRepoLoader::StoreTitleSet()
{
    const TString& strId = m_colAttrVals[tPlexRepoS::EAttrs::Id];
    const TString& strKey = m_colAttrVals[tPlexRepoS::EAttrs::Key];
    const TString& strTitle= m_colAttrVals[tPlexRepoS::EAttrs::Title];

    // Check for required info that's not available
    tCIDLib::TCard4 c4SeqId = 0;
    if (strKey.bIsEmpty()
    ||  strTitle.bIsEmpty()
    ||  strId.bIsEmpty()
    ||  !strId.bToCard4(c4SeqId, tCIDLib::ERadices::Dec))
    {
        if (m_eVerbose > tCQCKit::EVerboseLvls::Low)
        {
            facPlexRepoS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kPlexMsgs::midStatus_MissingInfo
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return;
    }

    //
    //  Just to be sure, make sure this key isn't already there. If so, something
    //  is wrong, so we need to ignore this one.
    //
    {
        tCIDLib::TCard2 c2DupId;
        const tCIDLib::TBoolean bDup = m_pmdbLoading->bCheckUIDExists
        (
            m_eMediaType, tCQCMedia::EDataTypes::TitleSet, strKey, c2DupId, m_strTmp1
        );
        if (bDup)
        {
            if (m_eVerbose > tCQCKit::EVerboseLvls::Low)
            {
                facPlexRepoS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kPlexErrs::errcXML_DupKey
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strKey
                );
            }
            return ;
        }
    }

    // If the year isn't a valid number, set it to 1900
    tCIDLib::TCard4 c4Year;
    if (!m_colAttrVals[tPlexRepoS::EAttrs::Year].bToCard4(c4Year, tCIDLib::ERadices::Dec))
        c4Year = 1900;

    //
    //  If the duration isn't a valid number, set it to 0. Else it should be in
    //  milliseconds and we want seconds.
    //
    tCIDLib::TCard4 c4Duration = 0;
    if (m_colAttrVals[tPlexRepoS::EAttrs::Duration].bToCard4(c4Duration, tCIDLib::ERadices::Dec))
    {
        c4Duration /= 1000;
    }

    //
    //  If the year isn't a valid number, set it to 1. If it is, convert to a
    //  cardinal value, since it's floating point.
    //
    tCIDLib::TCard4 c4UserRating = 1;
    {
        tCIDLib::TFloat8 f8Rating;
        if (m_colAttrVals[tPlexRepoS::EAttrs::UserRating].bToFloat8(f8Rating))
            c4UserRating = tCIDLib::TCard4(f8Rating);
    }

    // If we got art, then let's download that
    const TMediaImg* pmimgArt = 0;
    {
        const TString& strArtPath = m_colAttrVals[tPlexRepoS::EAttrs::Art];
        const TString& strThumbPath = m_colAttrVals[tPlexRepoS::EAttrs::ArtThumb];
        if (!strArtPath.bIsEmpty())
        {
            // See if we have this one already, unlikely but worth a quick check
            pmimgArt = m_pmdbLoading->pmimgByUniqueId
            (
                tCQCMedia::EMediaTypes::Movie, strArtPath, kCIDLib::False
            );

            // If not, then download it
            if (!pmimgArt)
                pmimgArt = pmimgDownloadArt(strArtPath, strThumbPath);
        }
    }

    //
    //  Make sure any categories we got this time are added, if not already
    //  present. We also gather up the ids of all of the categories this guy
    //  is in for setting on the stored data below.
    //
    m_fcolCatIds.RemoveAll();
    {

        const tCIDLib::TCard4 c4CatCnt = m_colCatList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CatCnt; c4Index++)
        {
            const TString& strCur = m_colCatList[c4Index];

            // The unique key is the upper cased name
            m_strTmp1 = strCur;
            m_strTmp1.ToUpper();
            const TMediaCat* pmcatCur = m_pmdbLoading->pmcatByUniqueId
            (
                tCQCMedia::EMediaTypes::Movie, m_strTmp1, kCIDLib::False
            );

            // If not there, add it
            if (!pmcatCur)
            {
                TMediaCat* pmcatNew = new TMediaCat
                (
                    tCQCMedia::EMediaTypes::Movie, strCur, m_strTmp1
                );
                m_pmdbLoading->c2AddCategory(pmcatNew);
                pmcatCur = pmcatNew;
            }
            m_fcolCatIds.c4AddElement(pmcatCur->c2Id());
        }
    }

    //
    //  Create a new title set. THe sort title is an upper cased version of
    //  the title.
    //
    m_strTmp1 = strTitle;
    m_strTmp1.ToUpper();
    TMediaTitleSet* pmtsCur = new TMediaTitleSet
    (
        strTitle, strKey, m_strTmp1, m_eMediaType
    );
    TJanitor<TMediaTitleSet> janTitle(pmtsCur);

    pmtsCur->c4SeqNum(c4SeqId);
    pmtsCur->c4UserRating(c4UserRating);

    // Now add the title, orphaning it out of the janitor
    m_pmdbLoading->c2AddTitle(janTitle.pobjOrphan());

    // Bump the count of titles done
    m_c4TitlesDone++;



    //
    //  Add a collection for the movie. We indicate a location type of collection
    //  based file, but it doesn't really matter since this guy is only used with
    //  the Plex renderer, which knows what the location means.
    //
    TMediaCollect* pmcolCur = new TMediaCollect
    (
        strTitle, strKey, c4Year, tCQCMedia::ELocTypes::FileCol, m_eMediaType
    );
    TJanitor<TMediaCollect> janCol(pmcolCur);

    // Set the collection level info we have
    pmcolCur->c4Duration(c4Duration);
    pmcolCur->strAspectRatio(m_colAttrVals[tPlexRepoS::EAttrs::AspectRatio]);
    pmcolCur->strArtist(m_colAttrVals[tPlexRepoS::EAttrs::Director]);
    pmcolCur->strDescr(m_colAttrVals[tPlexRepoS::EAttrs::Descr]);
    pmcolCur->strRating(m_colAttrVals[tPlexRepoS::EAttrs::Rating]);
    pmcolCur->strLabel(m_colAttrVals[tPlexRepoS::EAttrs::Studio]);
    pmcolCur->strLocInfo(strKey);

    // Add this one to the all movies category, and any explicit categories
    pmcolCur->bAddCategory(kCQCMedia::c2CatId_AllMovies);
    const tCIDLib::TCard4 c4CatCnt = m_fcolCatIds.c4ElemCount();
    for (tCIDLib::TCard4 c4CatInd = 0; c4CatInd < c4CatCnt; c4CatInd++)
        pmcolCur->bAddCategory(tCIDLib::TCard2(m_fcolCatIds[c4CatInd]));

    //
    //  Build up a command separated string of cast members. Set the first one
    //  as the lead actor.
    //
    {
        const tCIDLib::TCard4 c4CastCnt = m_colCastList.c4ElemCount();
        m_strTmp1.Clear();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4CastCnt; c4Index++)
        {
            if (c4Index)
                m_strTmp1.Append(L", ");
            else
                pmcolCur->strLeadActor(m_colCastList[c4Index]);
            m_strTmp1.Append(m_colCastList[c4Index]);
        }
        pmcolCur->strCast(m_strTmp1);
    }

    // If we got art, set that on the collection
    if (pmimgArt)
        pmcolCur->c2ArtId(pmimgArt->c2Id());


    // Create a single item for the movie and set some item level stuff on it
    TMediaItem* pmitemMovie = new TMediaItem
    (
        L"Movie", strKey, TString::strEmpty(), m_eMediaType
    );
    TJanitor<TMediaItem> janItem(pmitemMovie);

    pmitemMovie->strArtist(m_colAttrVals[tPlexRepoS::EAttrs::Director]);
    pmitemMovie->c4Duration(c4Duration);
    pmitemMovie->c4Year(c4Year);

    // If the channels value is valid, add to item and collection
    tCIDLib::TCard4 c4Channels;
    if (m_colAttrVals[tPlexRepoS::EAttrs::Channels].bToCard4(c4Channels, tCIDLib::ERadices::Dec))
        pmitemMovie->c4Channels(c4Channels);

    //
    //  Now add the item to the database and to the collection. Then add the
    //  collection to the database and to the title set.
    //
    m_pmdbLoading->c2AddItem(janItem.pobjOrphan());
    pmcolCur->bAddItem(*pmitemMovie);
    m_pmdbLoading->c2AddCollect(janCol.pobjOrphan());
    pmtsCur->bAddCollect(*pmcolCur);
}

