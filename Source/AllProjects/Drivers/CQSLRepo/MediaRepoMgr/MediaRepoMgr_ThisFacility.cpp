//
// FILE NAME: MediaRepoMgr_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/17/2006
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MediaRepoMgr.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacMediaRepoMgr,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacMediaRepoMgr
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacMediaRepoMgr: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacMediaRepoMgr::TFacMediaRepoMgr() :

    TGUIFacility
    (
        L"MediaRepoMgr"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_pwndMainFrame(0)
{
}

TFacMediaRepoMgr::~TFacMediaRepoMgr()
{
}


// ---------------------------------------------------------------------------
//  TFacMediaRepoMgr: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Given a client proxy for a repo manager interface, we will download the meda
//  DB from the repo and load it up into the caller's DB object, along with the
//  other info returned from the download. The return indicates whehter we got
//  new data or not. The caller can pass in a current serial number if desired, or
//  an empty string to insure he gets new data. The new serial number is returned.
//
//  It gives back the client proxy we got, in case the caller needs to do other
//  stuff.
//
tCIDLib::TBoolean
TFacMediaRepoMgr::bDownloadDB(  tMediaRepoMgr::TRepoMgrProxyPtr&    orbcRepo
                                , TMediaDB&                         mdbToFill
                                , tCQCMedia::EMTFlags&              eMTFlags
                                , TString&                          strDBSerNum
                                , TString&                          strRepoPath)
{
    tCIDLib::TCard4 c4RawBytes = 0;
    THeapBuf        mbufRaw(0x100000, 0x320000, 0x10000);

    tCIDLib::TBoolean bNewData = orbcRepo->bQueryDB
    (
        c4RawBytes, mbufRaw, strDBSerNum, strRepoPath
    );

    if (bNewData)
    {
        mdbToFill.LoadFromBinDump
        (
            mbufRaw, c4RawBytes, eMTFlags, strDBSerNum, kCIDLib::True
        );
    }
    return bNewData;
}


// Given a URL, this guy will try to extract it as a JPEG image
tCIDLib::TBoolean
TFacMediaRepoMgr::bExtractImg(          THTTPClient&    httpcGet
                                , const TString&        strURL
                                ,       TJPEGImage&     imgToFill
                                ,       TMemBuf&        mbufTmp)
{
    tCIDLib::TCard4         c4ContLen;
    tCIDLib::TKVPList       colInLines;
    tCIDLib::TKVPList       colOutLines;
    tCIDNet::EHTTPCodes     eCodeType;
    TString                 strContType;
    TString                 strRepLine;
    TString                 strUltimateURL;

    tCIDLib::TBoolean bRes = kCIDLib::False;
    try
    {
        TURL urlImg(strURL, tCIDSock::EQualified::Full);

        //
        //  And now let's do a query on this guy. Let it create a temporary data source
        //  for us, based on the URL.
        //
        TCIDDataSrcJan janEmpty;
        const tCIDLib::TCard4 c4Res = httpcGet.c4SendGetRedir
        (
            janEmpty
            , urlImg
            , TTime::enctNowPlusSecs(10)
            , L"Media Query/1.0"
            , L"image/jpeg"
            , eCodeType
            , strRepLine
            , colOutLines
            , strContType
            , mbufTmp
            , c4ContLen
            , strUltimateURL
            , colInLines
        );

        if ((c4Res == kCIDNet::c4HTTPStatus_OK) && c4ContLen)
        {
            // Put a stream over the buffer and parse into the image parm
            TBinMBufInStream strmImg(&mbufTmp, c4ContLen);
            strmImg >> imgToFill;
            bRes = kCIDLib::True;
        }
    }

    catch(...)
    {
    }
    return bRes;
}


//
//  A helper to just query a page of HTML, handling redirections. We return
//  the ultimate URL that was gotten to (if redirected it'll be differnet from
//  the incoming URL.)
//
tCIDLib::TBoolean
TFacMediaRepoMgr::bGetHTMLText(         THTTPClient&        httpcGet
                                , const TURL&               urlSrc
                                ,       TString&            strToFill
                                ,       tCIDLib::TCard4&    c4TextLen
                                ,       TMemBuf&            mbufTmp)
{
    try
    {
        tCIDLib::TCard4         c4ContLen;
        tCIDLib::TKVPList       colInLines;
        tCIDLib::TKVPList       colOutLines;
        tCIDNet::EHTTPCodes     eCodeType;
        TString                 strContType;
        TString                 strRepLine;
        TString                 strFinalURL;

        // Let it create a temporary data source for us, based on the URL.
        TCIDDataSrcJan janEmpty;
        tCIDLib::TCard4 c4Res = httpcGet.c4SendGetRedir
        (
            janEmpty
            , urlSrc
            , TTime::enctNowPlusSecs(10)
            , L"Media Query/1.0"
            , L"text/html"
            , eCodeType
            , strRepLine
            , colOutLines
            , strContType
            , mbufTmp
            , c4ContLen
            , strFinalURL
            , colInLines
        );

        // If not a good result or not content length, give up
        if ((c4Res != kCIDNet::c4HTTPStatus_OK) || !c4ContLen)
            return kCIDLib::False;

        // Get the text encoding indicated. If we don't grok, give up
        TString strEncoding;
        if (!THTTPClient::bParseTextEncoding(strContType, strEncoding)
        ||  !facCIDEncode().bSupportsEncoding(strEncoding))
        {
            return kCIDLib::False;
        }

        //
        //  We can convert it, so convert to a string. Tell it to use a replacement
        //  char on unconvertables since they might since bad data.
        //
        TJanitor<TTextConverter> janBody
        (
            facCIDEncode().ptcvtMake(strEncoding ,kCIDLib::chSpace)
        );
        c4TextLen = janBody->c4ConvertFrom(mbufTmp.pc1Data(), c4ContLen, strToFill);
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return (c4TextLen != 0);
}


//
//  Called to make a unique id for a piece of media in a drive. This is a generic
//  one which looks at the type reported by the removeable media probe, and
//  generates the right type. If a CD type, it'll return the table of contents
//  info as well.
//
//  Note that these are the ones we use for our own repository unique IDs. There
//  are others implemented for looking up metadata, each implemented in their
//  respective metadata source classes.
//
tCIDLib::TBoolean
TFacMediaRepoMgr::bMakeUniqueId(const   TString&                    strPath
                                , const TKrnlRemMedia::EMediaTypes  eMType
                                ,       TKrnlRemMedia::TCDTOCInfo&  TOCData
                                ,       TString&                    strToFill)
{
    tCIDLib::TBoolean bRet;
    if (TKrnlRemMedia::bIsCDType(eMType))
    {
        bRet = facCIDMetaExtr().bGenUniqueCDId(strPath, TOCData, strToFill);
    }
     else if (TKrnlRemMedia::bIsDVDType(eMType))
    {
        bRet = facCIDMetaExtr().bGenUniqueDVDId(strPath, strToFill);
    }
     else if (TKrnlRemMedia::bIsBDType(eMType))
    {
        bRet = facCIDMetaExtr().bGenUniqueBDId(strPath, strToFill);
    }
     else
    {
        // We don't understand this type
        bRet = kCIDLib::False;
    }
    return bRet;
}


// Scrape the albumart.org web site
tCIDLib::TBoolean
TFacMediaRepoMgr::bScrapeAAOData(const  TString&    strAlbum
                                ,       TJPEGImage& imgiToFill)
{
    TString strURL;

    //
    //  This is the search URL. We'll escape the title and replace the
    //  %(1) token with that text. Then we can do the search.
    //
    strURL = L"http://www.albumart.org/index.php?skey=%(1)&itempage=1&newsearch=1&searchindex=Music";
    TString strEncName;
    TURL::EncodeTo
    (
        strAlbum, strEncName, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append
    );
    strURL.eReplaceToken(strEncName, L'1');

    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        // Get this page
        tCIDLib::TCard4 c4TextLen;
        THeapBuf        mbufData(32 * 1024, 0x100000,  0x10000);
        TURL            urlInfo(strURL, tCIDSock::EQualified::Full);
        THTTPClient     httpcGet;
        TString         strBody;

        if (!bGetHTMLText(httpcGet, urlInfo, strBody, c4TextLen, mbufData))
            return kCIDLib::False;

        //
        //  Find the first image in the content that we might want. If we find one, keep
        //  looking until we find a good one or go far enough that it's not likely to be
        //  worth going on.
        //
        const TString   strImgPref(L"http://ecx.images-amazon.com");
        tCIDLib::TCard4 c4Ofs;
        tCIDLib::TCard4 c4Count = 0;
        if (strBody.bFirstOccurrence(strImgPref, c4Ofs, kCIDLib::False, kCIDLib::False))
        {
            do
            {
                //  We found one. Grab the chars up to the closing quote
                tCIDLib::TCard4 c4EndOfs = c4Ofs;
                while (c4EndOfs < c4TextLen)
                {
                    const tCIDLib::TCh chCur = strBody[c4EndOfs];
                    if ((chCur == kCIDLib::chQuotation)
                    ||  (chCur == kCIDLib::chGreaterThan))
                    {
                        break;
                    }
                    c4EndOfs++;
                }

                // Copy thes into a separate string
                strURL.CopyInSubStr(strBody, c4Ofs, c4EndOfs - c4Ofs);

                // Make sure it's not the small one
                if (!strURL.bEndsWithI(L"_.jpg"))
                {
                    TURL urlImg(strURL, tCIDSock::EQualified::Full);
                    bRet = facMediaRepoMgr.bExtractImg
                    (
                        httpcGet, urlImg, imgiToFill, mbufData
                    );

                    if (bRet)
                        break;

                    // Only check a few since it becomes less likely it's the one we want
                    c4Count++;
                    if (c4Count > 2)
                        break;
                }

                // Move to the end of what we just tried and try again
                c4Ofs += c4EndOfs - c4Ofs;
            }   while (strBody.bNextOccurrence(strImgPref, c4Ofs, kCIDLib::False, kCIDLib::False));
        }
    }

    catch(...)
    {
        bRet = kCIDLib::False;
    }
    return bRet;
}



// Scrape a cover art download site
tCIDLib::TBoolean
TFacMediaRepoMgr::bScrapeDCSData(const  tCIDLib::TBoolean   bIsMusic
                                , const TString&            strArtist
                                , const TString&            strAlbum
                                ,       TJPEGImage&         imgiToFill)
{
    // Build up the appropriate URL
    TString strURL;
    if (bIsMusic)
    {
        strURL = L"http://www.slothradio.com/covers/?adv=&artist=";
        TURL::EncodeTo
        (
            strArtist, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append
        );
        strURL.Append(L"&album=");
        TURL::EncodeTo
        (
            strAlbum, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append
        );

        //
        //  If either of them have an ampersand, replace that with the word 'and'
        //  since that always works better. It's easier to wait and do it, since it
        //  will be very unambigous at this point because it's already escaped
        //
        tCIDLib::TCard4 c4Index = 0;
        strURL.bReplaceSubStr(L"%26", L"and", c4Index, kCIDLib::True, kCIDLib::False);
    }
     else
    {
        strURL = L"http://www.slothradio.com/covers/dvd.php?adv=&title";
        TURL::EncodeTo
        (
            strAlbum, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append
        );
        strURL.Append(L"&actor=&director=");

        if (!strArtist.bIsEmpty())
        {
            TURL::EncodeTo
            (
                strArtist, strURL, TURL::EExpTypes::Query, tCIDLib::EAppendOver::Append
            );
        }
    }

    // And do the query to try to find the cover art
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4TextLen;
        THTTPClient     httpcGet;
        TString         strBody;
        THeapBuf        mbufData(32 * 1024, 0x100000,  0x10000);
        TURL            urlInfo;

        // Get the text of this page. If we cannot, then give up
        urlInfo.Reset(strURL, tCIDSock::EQualified::Full);
        if (!bGetHTMLText(httpcGet, urlInfo, strBody, c4TextLen, mbufData))
            return kCIDLib::False;

        // Search for the line that indicates the search results
        tCIDLib::TCard4 c4Ofs;
        const TString strArtKey(L"<!-- RESULT ITEM START -->");
        if (!strBody.bFirstOccurrence(strArtKey, c4Ofs, kCIDLib::False, kCIDLib::False))
            return kCIDLib::False;

        // Search forward from here for the next image ref
        if (!strBody.bNextOccurrence(L"img src=\"", c4Ofs, kCIDLib::False, kCIDLib::False))
            return kCIDLib::False;

        // Find the end (the next quote)
        c4Ofs += 9;
        tCIDLib::TCard4 c4EndOfs = c4Ofs;
        while (c4EndOfs < c4TextLen)
        {
            if (strBody[c4EndOfs] == kCIDLib::chQuotation)
                break;
            c4EndOfs++;
        }

        // Extract the URL out and try to get it as an image
        if (c4EndOfs < c4TextLen)
        {
            strURL.CopyInSubStr(strBody, c4Ofs, c4EndOfs - c4Ofs);
            strURL.StripWhitespace();
            urlInfo.Reset(strURL, tCIDSock::EQualified::Full);
            bRet = bExtractImg(httpcGet, urlInfo, imgiToFill, mbufData);
        }
    }

    catch(...)
    {
        bRet = kCIDLib::False;
    }
    return bRet;
}


// Scrape some info from the MS site if the we got a 'more info' URL from WMP
tCIDLib::TBoolean
TFacMediaRepoMgr::bScrapeMSData(const TString& strURL, TJPEGImage& imgiToFill)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    try
    {
        tCIDLib::TCard4 c4EndOfs;
        tCIDLib::TCard4 c4Ofs;
        tCIDLib::TCard4 c4TextLen;
        THTTPClient     httpcGet;
        THeapBuf        mbufData(32 * 1024, 0x100000,  0x10000);
        TString         strBody;
        TString         strImgURL;
        TURL            urlInfo;

        // Try to get the passed URL. If not, then give up
        urlInfo.Reset(strURL, tCIDSock::EQualified::Full);
        if (!bGetHTMLText(httpcGet, urlInfo, strBody, c4TextLen, mbufData))
            return kCIDLib::False;

        //
        //  Search for the line with the album art. If we find it, we'll try
        //  to do a GET on the image indicated.
        //
        const TString strArtKey(L"windowsmedia.com/cover");
        if (strBody.bFirstOccurrence(strArtKey, c4Ofs, kCIDLib::False, kCIDLib::False))
        {
            // Work backwards and forwards to the quotes and we have the URL
            c4EndOfs = c4Ofs;

            while (c4Ofs && (strBody[c4Ofs] != kCIDLib::chQuotation))
                c4Ofs--;
            c4Ofs++;

            while (c4EndOfs < c4TextLen)
            {
                if (strBody[c4EndOfs] == kCIDLib::chQuotation)
                    break;
                c4EndOfs++;
            }

            // And extract the URL out
            strImgURL.CopyInSubStr(strBody, c4Ofs, c4EndOfs - c4Ofs);
            strImgURL.StripWhitespace();

            // And now let's do an image GET on this guy
            urlInfo.Reset(strImgURL, tCIDSock::EQualified::Full);
            bRet = bExtractImg(httpcGet, urlInfo, imgiToFill, mbufData);
        }
    }

    catch(...)
    {
        bRet = kCIDLib::False;
    }
    return bRet;
}



//
//  Builds the paths we are used to store ripped media data files until they are
//  uploaded. We encode various bits of info into the file names so that what they
//  are is easily identified.
//
//  The format is:
//
//  mediatype_datatype_id_1basedtracknum.fileext
//
tCIDLib::TVoid
TFacMediaRepoMgr::BuildUploadMediaPath( const   tCQCMedia::EMediaTypes  eMType
                                        , const tCQCMedia::EDataTypes   eDType
                                        , const tCIDLib::TCard2         c2Id
                                        , const tCIDLib::TCard4         c4TrackNum
                                        , const tCIDLib::TCh* const     pszFileExt
                                        ,       TPathStr&               pathToFill)
{
    pathToFill = tCQCMedia::strXlatEMediaTypes(eMType);
    pathToFill.Append(L"_");
    pathToFill.Append(tCQCMedia::strXlatEDataTypes(eDType));
    pathToFill.Append(L"_");
    pathToFill.AppendFormatted(c2Id);
    pathToFill.Append(L"_");
    pathToFill.AppendFormatted(c4TrackNum);
    pathToFill.AppendExt(pszFileExt);
}


//
//  The program entry point. We get everything initialized and start the main
//  menu, wait for the end and clean up.
//
tCIDLib::EExitCodes
TFacMediaRepoMgr::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        return tCIDLib::EExitCodes::BadEnvironment;

    // Create a main frame window object
    TLogSrvLogger* plgrLogSrv = nullptr;
    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit.
        //
        TResourceName rsnInstance(L"CQSL", L"MediaRepoMgr", L"SingleInstanceInfo");
        if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::True))
            return tCIDLib::EExitCodes::Normal;

        //
        //  Initialize the client side orb interface. We need this to do any
        //  distributed stuff.
        //
        facCIDOrb().InitClient();

        // Install a standard log server logger
        plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        //
        //  Build up our local temporary path that we store ripped data in
        //  until we can upload it.
        //
        m_pathRipDir = facCQCKit().strClientDataDir();
        m_pathRipDir.AddLevel(L"RipDir");

        // Make sure it exists, and clean it out
        try
        {
            if (!TFileSys::bExists(m_pathRipDir))
                TFileSys::MakePath(m_pathRipDir);
            else
                TFileSys::CleanPath(m_pathRipDir);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            errToCatch.strAuxText(m_pathRipDir);

            TExceptDlg dlgErr
            (
                TWindow::wndDesktop()
                , facMediaRepoMgr.strMsg(kMRMgrMsgs::midMain_FrameTitle)
                , facMediaRepoMgr.strMsg(kMRMgrErrs::errcFile_CreateRipDir)
                , errToCatch
            );
            return tCIDLib::EExitCodes::InitFailed;
        }

        //
        //  Wait for the back end to come up, in case it hasn't yet. Just
        //  make it wait for the data server's security interface.
        //
        if (!facCQCIGKit().bWaitForCQC( TWindow::wndDesktop()
                                        , strMsg(kMRMgrMsgs::midMain_FrameTitle)
                                        , TCQCSecureClientProxy::strBinding
                                        , 60000))
        {
            return tCIDLib::EExitCodes::InitFailed;
        }

        //
        //  Before we create any window, lets ask the user to log on. We put
        //  up a modal dialog for this. If they can't log on ok, then we
        //  get back out now. If they do, this method will store the security
        //  info with the CQCKit facility, where everyone can get to it.
        //
        if (!bDoLogon())
            return tCIDLib::EExitCodes::PermissionLevel;

        // Initialize the local config store
        facCQCGKit().InitObjectStore
        (
            kMediaRepoMgr::pszStoreKey, m_cuctxToUse.strUserName(), kCIDLib::False
        );

        // Create our main frame window now that everything is ready
        m_pwndMainFrame = new TMainFrameWnd();
        if (!m_pwndMainFrame->bCreate())
            return tCIDLib::EExitCodes::InitFailed;
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , L"CQSL Media Repository Manager"
            , L"Initialization failed"
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    // Enter message loop till we are done
    facCIDCtrls().uMainMsgLoop(*m_pwndMainFrame);

    // Do our cleanup
    try
    {
        // Now we can clean up the main window
        if (m_pwndMainFrame)
            m_pwndMainFrame->Destroy();

        // Terminate the local object store
        facCQCGKit().TermObjectStore();

        // Force the log server logger to local logging
        if (plgrLogSrv)
            plgrLogSrv->bForceLocal(kCIDLib::True);

        // Terminate the Orb support
        facCIDOrb().Terminate();

        // Clean up the window object if we created it
        if (m_pwndMainFrame)
            delete m_pwndMainFrame;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , strMsg(kMRMgrMsgs::midMain_FrameTitle)
            , L"Media Repository Manager cleanup failed"
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This method will find and return a client proxy for the repository manager
//  interface on a CQSL repository driver.
//
tMediaRepoMgr::TRepoMgrProxyPtr
TFacMediaRepoMgr::orbcMakeProxy(const TString& strRepoMoniker)
{
    //
    //  The binding is provided by the proxy client, but we have to do a
    //  token replacement of the moniker in it.
    //
    TString strMgrBinding = TCQSLRepoMgrClientProxy::strBindingScope;
    strMgrBinding.eReplaceToken(strRepoMoniker, L'm');
    strMgrBinding.Append(L"/");
    strMgrBinding.Append(TCQSLRepoMgrClientProxy::strBindingName);

    tMediaRepoMgr::TRepoMgrProxyPtr orbcRepo
    (
        facCIDOrbUC().porbcMakeClient<TCQSLRepoMgrClientProxy>(strMgrBinding)
    );

    if (!orbcRepo.pobjData())
    {
        facMediaRepoMgr.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMRMgrErrs::errcComm_NoManagementIntf
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
            , strRepoMoniker
        );
    }
    return orbcRepo;
}


// Returns the pre-built temporary rip storage directory
const TString& TFacMediaRepoMgr::strRipDir() const
{
    return m_pathRipDir;
}


// A conveniene wrapper that we just pass on to our user context
const TString& TFacMediaRepoMgr::strUserName() const
{
    return m_cuctxToUse.strUserName();
}


// ---------------------------------------------------------------------------
//  TFacMediaRepoMgr: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFacMediaRepoMgr::bDoLogon()
{
    TCQCSecToken sectTmp;
    if (facCQCGKit().bLogon(TWindow::wndDesktop()
                            , sectTmp
                            , m_uaccLogin
                            , tCQCKit::EUserRoles::PowerUser
                            , strMsg(kMRMgrMsgs::midMain_FrameTitle)
                            , kCIDLib::False
                            , L"RepoMgr"))
    {
        // It worked so set up our user context
        m_cuctxToUse.Set(m_uaccLogin, sectTmp);
        m_cuctxToUse.LoadEnvRTVsFromHost();

        return kCIDLib::True;
    }
    return kCIDLib::False;
}


