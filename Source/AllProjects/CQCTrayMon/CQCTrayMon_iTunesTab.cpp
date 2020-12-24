//
// FILE NAME: CQCTrayMon_iTunesTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/12/2012
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTMediaRepoEng,TCQCStdMediaRepoEng)
RTTIDecls(TDBQChangeInfo, TObject);
RTTIDecls(TCQCTrayiTunesTab, TCQCTrayMonTab)



// ---------------------------------------------------------------------------
//  CLASS: TDBQChangeInfo
// PREFIX: dbci
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDBQChangeInfo: Constructors and Destructor
// ---------------------------------------------------------------------------

TDBQChangeInfo::TDBQChangeInfo() :

    m_bDelete(kCIDLib::False)
    , m_i4Database(0)
    , m_i4PlayList(0)
    , m_i4Src(0)
    , m_i4Track(0)
{
}

TDBQChangeInfo::TDBQChangeInfo( const   tCIDLib::TInt4      i4Src
                                , const tCIDLib::TInt4      i4PlayList
                                , const tCIDLib::TInt4      i4Track
                                , const tCIDLib::TInt4      i4Database
                                , const tCIDLib::TBoolean   bDelete) :

    m_bDelete(bDelete)
    , m_i4Database(i4Database)
    , m_i4PlayList(i4PlayList)
    , m_i4Src(i4Src)
    , m_i4Track(i4Track)
{
}

TDBQChangeInfo::TDBQChangeInfo(const TDBQChangeInfo& dbciSrc) :


    m_bDelete(dbciSrc.m_bDelete)
    , m_i4Database(dbciSrc.m_i4Database)
    , m_i4PlayList(dbciSrc.m_i4PlayList)
    , m_i4Src(dbciSrc.m_i4Src)
    , m_i4Track(dbciSrc.m_i4Track)
{
}

TDBQChangeInfo::~TDBQChangeInfo()
{
}


// ---------------------------------------------------------------------------
//  TDBQChangeInfo: Public operators
// ---------------------------------------------------------------------------
TDBQChangeInfo& TDBQChangeInfo::operator=(const TDBQChangeInfo& dbciSrc)
{
    if (&dbciSrc != this)
    {
        m_bDelete    = dbciSrc.m_bDelete;
        m_i4Database = dbciSrc.m_i4Database;
        m_i4PlayList = dbciSrc.m_i4PlayList;
        m_i4Src      = dbciSrc.m_i4Src;
        m_i4Track    = dbciSrc.m_i4Track;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TDBQChangeInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the type of object we represent
tCQCTrayMon::EiTunesObjs TDBQChangeInfo::eObjType() const
{
    if (m_i4Database || m_i4Track)
        return tCQCTrayMon::EiTunesObjs::Track;

    if (m_i4PlayList)
        return tCQCTrayMon::EiTunesObjs::PlayList;

    return tCQCTrayMon::EiTunesObjs::Source;
}




// ---------------------------------------------------------------------------
//  CLASS: TiTMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TiTMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Indicate the media types we support
//
TiTMediaRepoEng::TiTMediaRepoEng(const TString& strMoniker) :

    TCQCStdMediaRepoEng(strMoniker, tCQCMedia::EMTFlags::Music)
{
}

TiTMediaRepoEng::~TiTMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TiTMediaRepoEng: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called if an image needs to be loaded. We only support large art directly
//  so we scale down the large art if they ask for a thumb. We should never get called
//  for anything we have already returned, since it will be cached.
//
//  Unfortunately we can't cache the large art if we load it to create the thumb. If
//  the large art is subsequently asked for, we have to load it again. We give up this
//  slight efficiency in order for the standard repo DB engine code that calls this
//  to be a lot simpler.
//
//  We let exceptions propogate back.
//
tCIDLib::TCard4
TiTMediaRepoEng::c4LoadRawCoverArt( const   TMediaImg&              mimgToLoad
                                    ,       TMemBuf&                mbufToFill
                                    , const tCQCMedia::ERArtTypes   eType)
{
    tCIDLib::TCard4 c4Ret = 0;
    try
    {
        //
        //  If the large art has already been loaded and they want small, then we
        //  don't have to load the file again.
        //
        if (eType == tCQCMedia::ERArtTypes::SmlCover)
            c4Ret = mimgToLoad.c4QueryArt(mbufToFill, tCQCMedia::ERArtTypes::LrgCover, 0);

        if (!c4Ret)
        {
            //
            //  Apparently not so load the file. Keep in mind that we have to use the
            //  large art path here no matter what, since we don't support the small
            //  directly.
            //
            const TString& strPath = mimgToLoad.strArtPath(tCQCMedia::ERArtTypes::LrgCover);
            if (!strPath.bIsEmpty() && TFileSys::bExists(strPath))
            {
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

        //
        //  If they asked for small art, let's scale it down since we don't support
        //  separate thumbs. We use the same persistent id either way, since it's
        //  the same image.
        //
        if ((eType == tCQCMedia::ERArtTypes::SmlCover) && c4Ret)
        {
            c4Ret = facCIDImgFact().c4CompactToJPEG
            (
                mbufToFill, c4Ret, TSize(kCQCMedia::c4DefThumbSz)
            );
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
//  CLASS: TCQCTrayiTunesTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTrayiTunesTab::TCQCTrayiTunesTab() :

    TCQCTrayMonTab(L"iTunes Server")
    , m_bDBChanges(kCIDLib::False)
    , m_bDelImages(kCIDLib::False)
    , m_bForceReload(kCIDLib::False)
    , m_bQuitNotification(kCIDLib::False)
    , m_c4CurVolume(0)
    , m_c4EvAdviseId(0)
    , m_c4LastFailedCnt(0)
    , m_c4LastIgnoredCnt(0)
    , m_c4IgnoredCnt(0)
    , m_c4LastTitleCnt(0)
    , m_c4PlSerialNum(1)
    , m_c4TitleCnt(0)
    , m_c4TracksFailed(0)
    , m_c4TracksIgnored(0)
    , m_colDBChanges()
    , m_colLogQ()
    , m_colPlCmdQ()
    , m_eLastStatus(tCQCMedia::ELoadStatus::Init)
    , m_eLoadStatus(tCQCMedia::ELoadStatus::Init)
    , m_eMediaType(tCQCMedia::EMediaTypes::Music)
    , m_ePlState(TiTPlayerIntfServerBase::EPlStates::Stopped)
    , m_eStatus(tCQCMedia::ELoadStatus::Init)
    , m_i4MainListId(0)
    , m_i4MainSrcId(0)
    , m_mbufArtLoad(kCQCTrayMon::c4MaxImgPerIdBytes, kCQCTrayMon::c4MaxImgPerIdBytes)
    , m_piTunes(nullptr)
    , m_piTunesEv(nullptr)
    , m_porbsPlIntf(nullptr)
    , m_porbsRepoIntf(nullptr)
    , m_pwndDelImages(nullptr)
    , m_pwndFailedCnt(nullptr)
    , m_pwndForceReload(nullptr)
    , m_pwndIgnoredCnt(nullptr)
    , m_pwndLog(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndTitleCnt(nullptr)
    , m_srdbEng(TString(L"CQCTrayMon_") + TSysInfo::strIPHostName())
    , m_strAllMusicCat(kMedMsgs::midCat_AllMusic, facCQCMedia())
    , m_strPlayListCat(kMedMsgs::midCat_PlayLists, facCQCMedia())
    , m_strmfUID(8, 0, tCIDLib::EHJustify::Right, L'0')
    , m_strmFmt(64UL)
    , m_thriTunes
      (
        TString(L"CQCTrayiTunes")
        , TMemberFunc<TCQCTrayiTunesTab>(this, &TCQCTrayiTunesTab::eiTunesThread)
      )
    , m_tmidUpdate(0)
{
    // Set up the path we store the art in
    m_pathArtDir = facCQCKit().strClientDataDir();
    m_pathArtDir.AddLevel(L"CQCTrayMon");
    m_pathArtDir.AddLevel(L"iTunesArt");
}

TCQCTrayiTunesTab::~TCQCTrayiTunesTab()
{
}


// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Public, inherited methods
// ---------------------------------------------------------------------------

// This is called after RemoveBindings, to do any final cleanup
tCIDLib::TVoid TCQCTrayiTunesTab::Cleanup()
{
    // Stop our iTunes interface thread if it was started
    if (m_thriTunes.bIsRunning())
    {
        try
        {
            m_thriTunes.ReqShutdownSync(10000);
            m_thriTunes.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Deregister our remote interfaces
    if (m_porbsRepoIntf)
        facCIDOrb().DeregisterObject(m_porbsRepoIntf);

    if (m_porbsPlIntf)
        facCIDOrb().DeregisterObject(m_porbsPlIntf);
}


// Called during shutdown to get us to unbind any bindings we have
tCIDLib::TVoid TCQCTrayiTunesTab::RemoveBindings(tCIDOrbUC::TNSrvProxy& orbcNS)
{
    //
    //  Unbind the interfaces we expose for the iTunes repo and media
    //  renderer drivers.
    //
    TString strBind;
    try
    {
        strBind = TiTunesPlCtrlImpl::strImplBinding;
        strBind.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);
        orbcNS->RemoveBinding(strBind, kCIDLib::False);
    }

    catch(...)
    {
    }

    try
    {
        strBind = TiTunesRepoCtrlImpl::strImplBinding;
        strBind.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);
        orbcNS->RemoveBinding(strBind, kCIDLib::False);
    }

    catch(...)
    {
    }
}


//
//  Called during start up to create our tab and any stuff we need.
//
tCIDLib::TVoid TCQCTrayiTunesTab::CreateTab(TTabbedWnd& wndParent)
{
    wndParent.c4CreateTab(this, strName());

    //
    //  Create and register our two interfaces
    //
    //  !!NOTE!!
    //
    //  For these we use the local host name as the binding, no
    //  matter what was set on the command line. The iTunes drivers that
    //  talk to us must be on the local machine and they automatically
    //  set up the binding using the local host name.
    //
    m_porbsPlIntf = new TiTunesPlCtrlImpl(this);
    facCIDOrb().RegisterObject(m_porbsPlIntf, tCIDLib::EAdoptOpts::Adopt);
    TString strBind = TiTunesPlCtrlImpl::strImplBinding;
    strBind.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsPlIntf->ooidThis()
        , strBind
        , L"CQC Tray Monitor iTunes Player Interface"
    );

    m_porbsRepoIntf = new TiTunesRepoCtrlImpl(this);
    facCIDOrb().RegisterObject(m_porbsRepoIntf, tCIDLib::EAdoptOpts::Adopt);
    strBind = TiTunesRepoCtrlImpl::strImplBinding;
    strBind.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsRepoIntf->ooidThis()
        , strBind
        , L"CQC Tray Monitor iTunes Repo Interface"
    );

    // Start up our processing thread
    m_thriTunes.Start();
}


// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TCQCTrayiTunesTab::bQueryData(  const   TString&            strQType
                                , const TString&            strDataName
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufToFill)
{
    c4BufSz = 0;

    CheckDBReady(CID_LINE);

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);

    //
    //  We handle the query for metadata dumps here instead of passing it to
    //  the engine. We have the serial number info here.
    //
    if ((strQType == kCQCMedia::strQuery_BinMediaDump)
    ||  (strQType == kCQCMedia::strQuery_XMLMediaDump))
    {
        const TMediaDB& mdbDump = m_srdbEng.mdbInfo();

        // If the serial number is the same, we return nothing
        if (strDataName == m_strDBSerialNum)
        {
            c4BufSz = 0;
            return kCIDLib::False;
        }

        // Get a dump of the requested type
        if (strQType == kCQCMedia::strQuery_BinMediaDump)
        {
            c4BufSz = mdbDump.c4BuildBinDump
            (
                mbufToFill, L"CQCTrayMon", m_strDBSerialNum, m_srdbEng.eMediaTypes()
            );
        }
         else
        {
            c4BufSz = mdbDump.c4BuildXMLDump
            (
                mbufToFill, L"CQCTrayMon", m_strDBSerialNum, m_srdbEng.eMediaTypes()
            );
        }

        return kCIDLib::True;
    }

    return m_srdbEng.bQueryData(strQType, strDataName, c4BufSz, mbufToFill);
}


tCIDLib::TBoolean
TCQCTrayiTunesTab::bQueryData2( const   TString&            strQType
                                ,       tCIDLib::TCard4&    c4BufSz
                                ,       THeapBuf&           mbufIO)
{
    CheckDBReady(CID_LINE);

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);
    return m_srdbEng.bQueryData2(strQType, c4BufSz, mbufIO);
}


tCIDLib::TBoolean
TCQCTrayiTunesTab::bQueryTextVal(const  TString&    strQType
                                , const TString&    strDataName
                                ,       TString&    strToFill)
{
    CheckDBReady(CID_LINE);

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);
    return m_srdbEng.bQueryTextVal(strQType, strDataName, strToFill);
}


// The driver passes this along in reponse to a write to the ReloadDB field
tCIDLib::TBoolean TCQCTrayiTunesTab::bReloadDB()
{
    // Sync while we get this data
    TLocker lockrSync(&m_mtxSync);

    if (!m_bForceReload)
        m_bForceReload = kCIDLib::True;

    return kCIDLib::True;
}


tCIDLib::TBoolean
TCQCTrayiTunesTab::bSendData(const  TString&            strSType
                            ,       TString&            strDataName
                            ,       tCIDLib::TCard4&    c4BufSz
                            ,       THeapBuf&           mbufToSend)
{
    CheckDBReady(CID_LINE);

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);
    return m_srdbEng.bSendData(strSType, strDataName, c4BufSz, mbufToSend);
}


tCIDLib::TCard4
TCQCTrayiTunesTab::c4QueryVal(const TString& strValId)
{
    CheckDBReady(CID_LINE);

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);
    return m_srdbEng.c4QueryVal(strValId);
}


tCIDLib::TCard4
TCQCTrayiTunesTab::c4SendCmd(const  TString&    strCmdId
                            , const TString&    strParms)
{
    CheckDBReady(CID_LINE);

    // Special case this oen since we have to handle it ourself
    if (strCmdId == kCQCMedia::strCmd_SetUserRating)
    {
        TString strCookie(strParms);
        TString strRating;
        strCookie.bSplit(strRating, kCIDLib::chSpace);
        strCookie.StripWhitespace();
        strRating.StripWhitespace();
        tCIDLib::TCard4 c4Rating = strRating.c4Val();

        //
        //  Parse the cookie as a title cookie to make sure it is valid
        //  and to get the title set id and media type.
        //
        tCIDLib::TCard2 c2CatId;
        tCIDLib::TCard2 c2TitleId;
        tCQCMedia::EMediaTypes eMType = facCQCMedia().eParseTitleCookie
        (
            strCookie, c2CatId, c2TitleId
        );

        // Looks reasonable so do it
        TLocker lockrSync(&m_mtxSync);

        //
        //  Update our in-memory info. He'll return a pointer to the object if it
        //  is found, and we can use that to call a helper that will apply the
        //  change to the iTunes database.
        //
        TMediaDB& mdbTar = m_srdbEng.mdbInfo();
        const TMediaTitleSet* pmtsUpdate = mdbTar.pmtsSetTitleRating
        (
            c2TitleId, eMType, c4Rating
        );

        if (pmtsUpdate)
        {
            SetUserRating(*pmtsUpdate, c4Rating);
            QLogMsg(L"User rating updated for title '%(1)'", pmtsUpdate->strName());
        }
        return 1;
    }

    // Sync with the bgn thread
    TLocker lockrSync(&m_mtxSync);
    return m_srdbEng.c4SendCmd(strCmdId, strParms);
}


//
//  One of the driver support methods. We return the status to the driver
//  who can display it. He uses this as a regular ping as well.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::QueryStatus( tCQCMedia::ELoadStatus&     eCurStatus
                                , tCQCMedia::ELoadStatus&   eLoadStatus
                                , TString&                  strDBSerialNum
                                , tCIDLib::TCard4&          c4TitleCnt)
{
    // Sync while we get this data
    TLocker lockrSync(&m_mtxSync);

    c4TitleCnt = m_c4TitleCnt;
    eCurStatus = m_eStatus;
    eLoadStatus = m_eLoadStatus;
    strDBSerialNum = m_strDBSerialNum;
}



// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCTrayiTunesTab::bCreated()
{
    TParent::bCreated();

    // Load up our dialog resource and create our contents within that
    TDlgDesc dlgdChildren;
    facCQCTrayMon.bCreateDlgDesc(kCQCTrayMon::ridDlg_iTunes, dlgdChildren);
    SetBgnColor(TDlgBox::rgbDlgBgn(dlgdChildren.eTheme()));

    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Get typed pointers to some of them
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_DelImages, m_pwndDelImages);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_FailedCnt, m_pwndFailedCnt);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_ForceReload, m_pwndForceReload);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_IgnoredCnt, m_pwndIgnoredCnt);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_Log, m_pwndLog);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_iTunes_TitleCnt, m_pwndTitleCnt);

    // Initialize some of the values in case we can't ever start setting them
    const TString strInitCnt(L"0");
    m_pwndFailedCnt->strWndText(strInitCnt);
    m_pwndIgnoredCnt->strWndText(strInitCnt);
    m_pwndStatus->strWndText(L"Unknown");
    m_pwndTitleCnt->strWndText(strInitCnt);

    // Install handlers
    m_pwndDelImages->pnothRegisterHandler(this, &TCQCTrayiTunesTab::eClickHandler);
    m_pwndForceReload->pnothRegisterHandler(this, &TCQCTrayiTunesTab::eClickHandler);

    // Start up an update timer to keep our display status current
    m_tmidUpdate = tmidStartTimer(200);

    return kCIDLib::True;
}



tCIDLib::TVoid TCQCTrayiTunesTab::Destroyed()
{
    // Kill our update timer
    if (m_tmidUpdate)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = 0;
    }

    // And pass it on
    TParent::Destroyed();
}


//
//  We start up a timer so that we can keep our display info updated.
//
tCIDLib::TVoid TCQCTrayiTunesTab::Timer(const tCIDCtrls::TTimerId tmidThis)
{
    // If not our timer, pass it to our parent and return
    if (tmidThis != m_tmidUpdate)
    {
        TParent::Timer(tmidThis);
        return;
    }

    //
    //  If the loading status has changed, then update. Don't bother with
    //  locking here, we are just looking at a basic value.
    //
    if (m_eLastStatus != m_eStatus)
    {
        // Copy the new status first, so no timing issues
        m_eLastStatus = m_eStatus;

        const tCIDLib::TCh* pszNew = pszXlatStatus(m_eLastStatus);
        m_pwndStatus->strWndText(pszNew);

        // Enable/disable stuff based on state
        m_pwndForceReload->SetEnable(m_eLastStatus == tCQCMedia::ELoadStatus::Ready);
    }

    // If the title count has changed, update that
    if (m_c4LastTitleCnt != m_c4TitleCnt)
    {
        m_c4LastTitleCnt = m_c4TitleCnt;

        m_strTimerTmp.SetFormatted(m_c4LastTitleCnt);
        m_pwndTitleCnt->strWndText(m_strTimerTmp);
    }

    // If the ignored count has changed, update that
    if (m_c4LastIgnoredCnt != m_c4TracksIgnored)
    {
        m_c4LastIgnoredCnt = m_c4TracksIgnored;

        m_strTimerTmp.SetFormatted(m_c4LastIgnoredCnt);
        m_pwndIgnoredCnt->strWndText(m_strTimerTmp);
    }

    // If the failed count has changed, update that
    if (m_c4LastFailedCnt != m_c4TracksFailed)
    {
        m_c4LastFailedCnt = m_c4TracksFailed;

        m_strTimerTmp.SetFormatted(m_c4LastFailedCnt);
        m_pwndFailedCnt->strWndText(m_strTimerTmp);
    }

    // If any log message queued up, output them
    if (!m_colLogQ.bIsEmpty())
    {
        TLocker lockrSync(&m_mtxSync);

        while (m_colLogQ.bGetNextMv(m_strTimerTmp, 0, kCIDLib::False))
        {
            // Add a new line so the output window will treat it as a line
            m_strTimerTmp.Append(L"\n");
            m_pwndLog->AddString(m_strTimerTmp);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCTrayiTunesTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once we are loaded, this is called to watch for database change events or
//  incoming driver commands to process.
//
tCIDLib::TBoolean TCQCTrayiTunesTab::bCheckEvents(TThread& thrThis)
{
    //
    //  Make sure we are still connected. If not, disconnect and return.
    //  The thread will start trying to reconnect.
    //
    if (m_bQuitNotification)
    {
        Disconnect();
        return kCIDLib::False;
    }

    //
    //  Seem to be connected still, so see if we got any database change
    //  notifications. If so, process them.
    //
    TLocker lockrSync(&m_mtxSync);
    try
    {
        //
        //  While there are events in the list, process them. We call a
        //  helper in the overflow file since there's a good bit of work.
        //
        //  If we see the force reload flag or get a shutdown request,
        //  we break out. In either case, these changes are no longer
        //  of use.
        //
        //  Just in case, don't allow us to get stuck in here, so we'll
        //  do up to 64 at at time.
        //
        tCIDLib::TCard4 c4EvCnt = 0;
        while (m_colDBChanges.bGetNext(m_dbciTmp, 5) && (c4EvCnt < 64))
        {
            if (m_bForceReload || thrThis.bCheckShutdownRequest())
                break;

            ProcessEvent(m_dbciTmp);
            c4EvCnt++;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // If any player commands, do those
    try
    {
        tCIDLib::TCard4 c4CmdCnt = 0;
        while (m_colPlCmdQ.bGetNext(m_plcmdTmp, 5) && (c4CmdCnt < 4))
        {
            if (m_bForceReload || thrThis.bCheckShutdownRequest())
                break;

            // Call an command to do the command. This is in the 3 file
            ProcessPlCmd(m_plcmdTmp);
            c4CmdCnt++;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    //
    //  If there's currently a track playing and it had duration info, then get
    //  the current position info.
    //
    if ((m_ePlState == TiTPlayerIntfServerBase::EPlStates::Playing) && m_enctCurTotal)
        StoreCurPos();

    return kCIDLib::True;
}


//
//  Given an object id, we parse out the four values and give them back.
//  For track oriented ones only the i4DB will be useful, the rest will
//  be zero.
//
tCIDLib::TBoolean
TCQCTrayiTunesTab::bParseObjUID(const   TString&        strUID
                                ,       tCIDLib::TInt4& i4Src
                                ,       tCIDLib::TInt4& i4PL
                                ,       tCIDLib::TInt4& i4Track
                                ,       tCIDLib::TInt4& i4DB)
{
    TStringTokenizer stokUID(&strUID, L"-");
    TString strSrc;
    TString strPL;
    TString strTrack;
    TString strDB;

    if (!stokUID.bGetNextToken(strSrc)
    ||  !stokUID.bGetNextToken(strPL)
    ||  !stokUID.bGetNextToken(strTrack)
    ||  !stokUID.bGetNextToken(strDB))
    {
        return kCIDLib::False;
    }

    try
    {
        i4Src = strSrc.i4Val(tCIDLib::ERadices::Hex);
        i4PL = strPL.i4Val(tCIDLib::ERadices::Hex);
        i4Track = strTrack.i4Val(tCIDLib::ERadices::Hex);
        i4DB = strDB.i4Val(tCIDLib::ERadices::Hex);
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}



//
//  Called by the methods that handle incoming calls from the driver, to
//  see if the database is ready.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::CheckDBReady(const tCIDLib::TCard4 c4Line)
{
    if (m_eStatus != tCQCMedia::ELoadStatus::Ready)
    {
        facCQCTrayMon.ThrowErr
        (
            CID_FILE
            , c4Line
            , kTrayMonErrs::errciTunes_DBNotReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}


//
//  At the end of a database load, if the user as checked the delete images on load
//  check box, we go through and find all of the image files not in use and del them.
//
tCIDLib::TVoid TCQCTrayiTunesTab::CleanupArt(TMediaDB& mdbTar)
{
    //
    //  Iterate all of the music images and create a hash set of the paths. We can
    //  use this below to know which paths are still referenced and which aren't.
    //
    tCIDLib::TStrHashSet colPaths(109, TStringKeyOps());
    const tCIDLib::TCard4 c4ImgCnt = mdbTar.c4ImageCnt(m_eMediaType);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ImgCnt; c4Index++)
    {
        colPaths.objAdd
        (
            mdbTar.mimgAt(m_eMediaType, c4Index).strArtPath(tCQCMedia::ERArtTypes::LrgCover)
        );
    }

    //
    //  Iterate all the files in the image directory and see if the path is in the
    //  list of paths we created above.
    //
    TDirIter    diterLevel;
    TFindBuf    fndbSearch;
    if (!diterLevel.bFindFirst(m_pathArtDir, L"*.*", fndbSearch, tCIDLib::EDirSearchFlags::NormalFiles))
        return;

    do
    {
        if (!colPaths.bHasElement(fndbSearch.pathFileName()))
        {
            try
            {
                TFileSys::DeleteFile(fndbSearch.pathFileName());
            }

            catch(...)
            {
            }
        }
    }   while(diterLevel.bFindNext(fndbSearch));
}



// Handle buttons
tCIDCtrls::EEvResponses
TCQCTrayiTunesTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCTrayMon::ridDlg_iTunes_ForceReload)
    {
        // Just set the force reload flag for the bgn thread to see
        m_bForceReload = kCIDLib::True;
    }
     else if (wnotEvent.widSource() == kCQCTrayMon::ridDlg_iTunes_DelImages)
    {
        // Just store the state for the bgn thread
        m_bDelImages = m_pwndDelImages->bCheckedState();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If there are servers installed, i.e. the app shell is running on this
//  machine, we start this thread to monitor the state of the service.
//
tCIDLib::EExitCodes
TCQCTrayiTunesTab::eiTunesThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go
    thrThis.Sync();

    //
    //  A time stamp for limiting how often we regenerate a DB serial number in
    //  response to event driven changes. We do 25 seconds. That allows it to
    //  not lag too much, but also not to do it multiple times for a quick sequence
    //  of changes.
    //
    const tCIDLib::TEncodedTime enctDBCInterval(25 * kCIDLib::enctOneSecond);
    tCIDLib::TEncodedTime enctNextDBC = TTime::enctNow() + enctDBCInterval;

    // Loop until asked to shut down
    while (kCIDLib::True)
    {
        try
        {

            //
            //  Based on our current start do the right thing, and decide if
            //  we want to sleep this round, and how much if so.
            //
            tCIDLib::TCard4 c4Sleep = 0;
            switch(m_eStatus)
            {
                case tCQCMedia::ELoadStatus::Init :
                {
                    //
                    //  Try to create our COM interface and connect to the
                    //  local iTunes instance.
                    //
                    try
                    {
                        ConnectToiTunes();
                        QLogMsg(L"Connected to iTunes successfully");
                    }

                    catch(TError& errToCatch)
                    {
                        if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    // Clear this since we are going to load anyway
                    m_bForceReload = kCIDLib::False;

                    // If we didn't move forward, then sleep this round
                    if (m_eStatus ==  tCQCMedia::ELoadStatus::Init)
                        c4Sleep = 25000;
                    break;
                }

                case tCQCMedia::ELoadStatus::Loading :
                {
                    // Try to load the database
                    try
                    {
                        LoadDB(thrThis);
                    }

                    catch(TError& errToCatch)
                    {
                        if (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium)
                        {
                            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                            TModule::LogEventObj(errToCatch);
                        }
                    }

                    // We either loaded or tried, so clear this if set
                    m_bForceReload = kCIDLib::False;

                    // If we didn't move forward, then sleep this round
                    if (m_eStatus == tCQCMedia::ELoadStatus::Loading)
                        c4Sleep = 3000;
                    break;
                }

                case tCQCMedia::ELoadStatus::Ready :
                {
                    //
                    //  See if we have any database change or player command
                    //  events to process. If so, let's deal with those. this
                    //  will also make sure we are still connected. If not,
                    //  it will disconnect and put us back into connecting
                    //  state.
                    //
                    if (bCheckEvents(thrThis))
                    {
                        //
                        //  If force reload is set, reset our status and don't
                        //  sleep any. Else, sleep just a bit so we recheck
                        //  quickly when in this state.
                        //
                        if (m_bForceReload)
                        {
                            m_bForceReload = kCIDLib::False;
                            StoreNewStatus(tCQCMedia::ELoadStatus::Loading);
                        }
                         else
                        {
                            c4Sleep = 200;
                        }
                    }

                    //
                    //  If database changes and it's been more than the minimum
                    //  interval, then let's generate a new DB serial number.
                    //
                    if (m_bDBChanges)
                    {
                        tCIDLib::TEncodedTime enctCur = TTime::enctNow();
                        if (enctCur >= enctNextDBC)
                        {
                            //
                            //  Clear the flag and reset the time stamp first, in
                            //  case of failure below
                            //
                            m_bDBChanges = kCIDLib::False;
                            enctNextDBC = enctCur + enctDBCInterval;

                            TString strDBSerNum;
                            CalcDBSerialNum(m_srdbEng.mdbInfo(), strDBSerNum);

                            //
                            //  Lock and update. The poll thread reads the DB
                            //  serial number member, so we can't just update it
                            //  without locking.
                            //
                            TLocker lockrSync(&m_mtxSync);
                            m_strDBSerialNum = strDBSerNum;
                        }
                    }
                    break;
                }

                default :
                    break;
            };


            if (c4Sleep)
            {
                // Sleep the indictated amount. Break out if shutdown req
                if (!thrThis.bSleep(c4Sleep))
                    break;
            }
             else
            {
                // No sleep, but do a shutdown request check
                if (thrThis.bCheckShutdownRequest())
                    break;
            }
        }

        //
        //  If we get to these, then something fundamental went wrong, since
        //  we handle exceptions individually above.
        //
        //  So we panic, and restart. Sleep a little after we disconnect,
        //  just to back off and let things settle.
        //
        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged()
            &&  (facCQCTrayMon.eVerboseLvl() >= tCQCKit::EVerboseLvls::Medium))
            {
                TModule::LogEventObj(errToCatch);
            }

            Disconnect();
            if (!thrThis.bSleep(2000))
                break;
        }

        catch(...)
        {
            Disconnect();
            if (!thrThis.bSleep(2000))
                break;
        }
    }

    // Disconnect since we are done
    Disconnect();

    return tCIDLib::EExitCodes::Normal;
}


//
//  Format out the ids of an iTunes object into a unique id we can
//  use in our media objects, and to map back and forth between
//  them.
//
//  See the class level comments for format details.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::FormatListUID(const  TDBQChangeInfo& dbciEv
                                ,       TString&        strUID)
{
    TString strTmp;
    FormatObjUID
    (
        dbciEv.m_i4Src, dbciEv.m_i4PlayList, 0, 0, strUID, strTmp
    );
}


tCIDLib::TVoid
TCQCTrayiTunesTab::FormatObjUID(const   TDBQChangeInfo& dbciEv
                                ,       TString&        strUID
                                ,       TString&        strTrackUID)
{
    FormatObjUID
    (
        dbciEv.m_i4Src
        , dbciEv.m_i4PlayList
        , dbciEv.m_i4Track
        , dbciEv.m_i4Database
        , strUID
        , strTrackUID
    );
}

tCIDLib::TVoid
TCQCTrayiTunesTab::FormatObjUID(const   tCIDLib::TInt4  i4Src
                                , const tCIDLib::TInt4  i4PL
                                , const tCIDLib::TInt4  i4Track
                                , const tCIDLib::TInt4  i4DB
                                ,       TString&        strUID
                                ,       TString&        strTrackUID)
{
    // Format the regular id first
    TStreamJanitor janFmt(&m_strmFmt);
    m_strmFmt.Reset();
    m_strmFmt   << m_strmfUID
                << TCardinal(i4Src, tCIDLib::ERadices::Hex)
                << TTextOutStream::RepChars(L'-', 1)
                << TCardinal(i4PL, tCIDLib::ERadices::Hex)
                << TTextOutStream::RepChars(L'-', 1)
                << TCardinal(i4Track, tCIDLib::ERadices::Hex)
                << TTextOutStream::RepChars(L'-', 1)
                << TCardinal(i4DB, tCIDLib::ERadices::Hex)
                << kCIDLib::FlushIt;
    strUID = m_strmFmt.strData();


    // If it's a track related one, do that version
    if (i4DB)
    {
        //
        //  Set the default format temporarily so that we can put in the
        //  prefix bit, else it'll get truncated to the formatted width.
        //  After that we set it back to the UID format.
        //
        m_strmFmt.Reset();
        m_strmFmt   << TTextOutStream::strmfDefault()
                    << L"00000000-00000000-00000000-"
                    << m_strmfUID
                    << TCardinal(i4DB, tCIDLib::ERadices::Hex)
                    << kCIDLib::FlushIt;
        strTrackUID = m_strmFmt.strData();
    }
}


// Translate the status into a string
const tCIDLib::TCh*
TCQCTrayiTunesTab::pszXlatStatus(const tCQCMedia::ELoadStatus eStatus)
{
    const tCIDLib::TCh* pszNew = L"????";;
    switch(m_eLastStatus)
    {
        case tCQCMedia::ELoadStatus::Init :
            pszNew = L"Connecting to iTunes";
            break;

        case tCQCMedia::ELoadStatus::Loading :
            pszNew = L"Loading Database";
            break;

        case tCQCMedia::ELoadStatus::Ready :
            pszNew = L"Database is Ready";
            break;

        default:
            break;
    };
    return pszNew;
}


//
//
//  Sync and queue up a log message on the log queue. Handle the queue
//  getting backed up.
//
//  We want to limit the length of the formatted values because they are
//  typically the names of albums or tracks. So we pre-format them into
//  separate strings first and cap/ellipse them if needed.
//
tCIDLib::TVoid TCQCTrayiTunesTab::QLogMsg(const TString& strMsg)
{
    TLocker lockrSync(&m_mtxSync);
    m_colLogQ.objAdd(strMsg);
}

tCIDLib::TVoid
TCQCTrayiTunesTab::QLogMsg( const   TString&        strMsg
                            , const MFormattable&   mfmtblToken1)
{
    m_strThreadLogTmp = strMsg;

    m_strThreadLogTmp2 = L"%(1)";
    m_strThreadLogTmp2.eReplaceToken(mfmtblToken1, L'1');
    if (m_strThreadLogTmp2.c4Length() > 28)
    {
        m_strThreadLogTmp2.CapAt(24);
        m_strThreadLogTmp2.Append(L"...");
    }
    m_strThreadLogTmp.eReplaceToken(m_strThreadLogTmp2, L'1');

    TLocker lockrSync(&m_mtxSync);
    m_colLogQ.objAdd(m_strThreadLogTmp);
}


tCIDLib::TVoid
TCQCTrayiTunesTab::QLogMsg( const   TString&        strMsg
                            , const MFormattable&   mfmtblToken1
                            , const MFormattable&   mfmtblToken2)
{
    m_strThreadLogTmp = strMsg;

    m_strThreadLogTmp2 = L"%(1)";
    m_strThreadLogTmp2.eReplaceToken(mfmtblToken1, L'1');
    if (m_strThreadLogTmp2.c4Length() > 28)
    {
        m_strThreadLogTmp2.CapAt(24);
        m_strThreadLogTmp2.Append(L"...");
    }
    m_strThreadLogTmp.eReplaceToken(m_strThreadLogTmp2, L'1');

    m_strThreadLogTmp2 = L"%(2)";
    m_strThreadLogTmp2.eReplaceToken(mfmtblToken2, L'2');
    if (m_strThreadLogTmp2.c4Length() > 28)
    {
        m_strThreadLogTmp2.CapAt(24);
        m_strThreadLogTmp2.Append(L"...");
    }
    m_strThreadLogTmp.eReplaceToken(m_strThreadLogTmp2, L'2');

    TLocker lockrSync(&m_mtxSync);
    m_colLogQ.objAdd(m_strThreadLogTmp);
}


//
//  Called when the bgn thread needs to store a new status. This is only called by
//  the iTunes thread. It does set info that is accessed by the repo and player
//  threads, but it only sets fundamental values, and the other threads only read
//  these values, they don't change them. So no locking is currently required.
//
tCIDLib::TVoid
TCQCTrayiTunesTab::StoreNewStatus(const tCQCMedia::ELoadStatus eToSet)
{
    if (eToSet != m_eStatus)
    {
        m_eStatus = eToSet;

        // Bump the player status serial number
        m_c4PlSerialNum++;
        if (!m_c4PlSerialNum)
            m_c4PlSerialNum++;
    }

    //
    //  And udpate the load status, which always runs up to loaded and then
    //  stays there.
    //
    if ((eToSet > m_eLoadStatus) && (eToSet < tCQCMedia::ELoadStatus::Failed))
        m_eLoadStatus = eToSet;
}

