//
// FILE NAME: AutonomicS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/07/2007
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
#include    "AutonomicS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TAutonomicSDriver,TCQCStdMediaRepoDrv)


// ---------------------------------------------------------------------------
//  CLASS: TAutonomicMediaRepoEng
// PREFIX: srdb
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TAutonomicMediaRepoEng: Constructors and Destructor
// ---------------------------------------------------------------------------
TAutonomicMediaRepoEng::
TAutonomicMediaRepoEng( const   TString&                strMoniker
                        , const tCQCMedia::EMTFlags     eSupportedTypes
                        , const TIPEndPoint&            ipepSrv) :

    TCQCStdMediaRepoEng(strMoniker, eSupportedTypes)
{
}

TAutonomicMediaRepoEng::~TAutonomicMediaRepoEng()
{
}


// ---------------------------------------------------------------------------
//  TAutonomicMediaRepoEng: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  We have to provide an override of this method since we get our art via
//  HTTP.
//
tCIDLib::TCard4
TAutonomicMediaRepoEng::c4LoadCoverArt( const   TString&    strArtPath
                                        ,       TMemBuf&    mbufToFill)
{
    tCIDLib::TCard4 c4RetBytes = 0;
    try
    {
        //
        //  The art path for us is the GUI id of the title. We can use
        //  that to build the URL for the image.
        //
        TString strURL(L"http://");
        strURL.Append(m_ipepSrv.strAsText());
        strURL.Append(L':');
        strURL.AppendFormatted(m_ipepSrv.ippnThis() + 1);
        strURL.Append(L"/albumart?album=");
        strURL.Append(strArtPath);
        strURL.Append(L".jpg");

        tCIDLib::TCard4         c4Status;
        tCIDLib::TKVPList       colHeaderLines;
        tCIDNet::EHTTPCodes     eCodeType;
        TString                 strContType;
        TString                 strRepLine;
        TURL                    urlImg(strURL, tCIDSock::EQualified::Full);
        THTTPClient             httpcImg;
        c4Status = httpcImg.c4SendGet
        (
            urlImg
            , TTime::enctNowPlusSecs(5)
            , L"CQC MCE Driver/1.0"
            , L"image/jpg"
            , eCodeType
            , strRepLine
            , colHeaderLines
            , strContType
            , mbufToFill
            , c4RetBytes
        );

        if (c4Status != kCIDNet::c4HTTPStatus_OK)
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQCMedia().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Got HTTP status %(1) when querying image %(2)"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::CantDo
                    , TCardinal(c4Status)
                    , strArtPath
                );
            }
            c4RetBytes = 0;
        }
    }

    catch(const TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bNogged())
                TModule::LogEventObj(errToCatch);

            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedErrs::errcDB_CantLoadArt
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , strArtPath
            );
        }
        c4RetBytes = 0;
    }
    return c4RetBytes;
}




// ---------------------------------------------------------------------------
//   CLASS: AutonomicSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  AutonomicSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TAutonomicSDriver::TAutonomicSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRepoDrv(cqcdcToLoad, tCQCMedia::EMTFlags::Music, &m_srdbEngine)
    , m_c2NextId_Cat(1)
    , m_c2NextId_Title(1)
    , m_c4FldId_AdjVolume(kCIDLib::c4MaxCard)
    , m_c4FldId_AdjChannel(kCIDLib::c4MaxCard)
    , m_c4FldId_CIDName(kCIDLib::c4MaxCard)
    , m_c4FldId_CIDNumber(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColArtist(kCIDLib::c4MaxCard)
    , m_c4FldId_CurColName(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemTime(kCIDLib::c4MaxCard)
    , m_c4FldId_CurItemTotal(kCIDLib::c4MaxCard)
    , m_c4FldId_EnqueueMedia(kCIDLib::c4MaxCard)
    , m_c4FldId_Loaded(kCIDLib::c4MaxCard)
    , m_c4FldId_MediaState(kCIDLib::c4MaxCard)
    , m_c4FldId_Mute(kCIDLib::c4MaxCard)
    , m_c4FldId_Navigation(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayMedia(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadDB(kCIDLib::c4MaxCard)
    , m_c4FldId_Running(kCIDLib::c4MaxCard)
    , m_c4FldId_ScreenNavigation(kCIDLib::c4MaxCard)
    , m_c4FldId_Status(kCIDLib::c4MaxCard)
    , m_c4FldId_TitleCnt(kCIDLib::c4MaxCard)
    , m_c4FldId_Transport(kCIDLib::c4MaxCard)
    , m_c4FldId_Volume(kCIDLib::c4MaxCard)
    , m_c4TitlesDone(0)
    , m_eConnStatus(tAutonomicS::EConnStatus_StartLoad)
    , m_enctLastMsg(0)
    , m_enctNextLoad(0)
    , m_mbufSend(4096, 8192)
    , m_sockConn()
    , m_srdbEngine
      (
        cqcdcToLoad.strMoniker()
        , tCQCMedia::EMTFlags::Music
        , static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal()).ipepConn()
      )
    , m_strMsg_Album(L"  Album")
    , m_strMsg_BannerTerm(L"Type '?' for help")
    , m_strMsg_BeginAlbums(L"BeginAlbums Total=")
    , m_strMsg_BeginGenres(L"BeginGenres Total=")
    , m_strMsg_BrowseAlbums(L"BrowseAlbums")
    , m_strMsg_BrowseGenres(L"BrowseGenres")
    , m_strMsg_ClearMusicFilter(L"ClearMusicFilter")
    , m_strMsg_EndAlbums(L"EndAlbums ")
    , m_strMsg_EndGenres(L"EndGenres ")
    , m_strMsg_Events(L"Events")
    , m_strMsg_Genre(L"  Genre")
    , m_strMsg_GetMCEStatus(L"GetMCEStatus")
    , m_strMsg_MediaFilter(L"MediaFilter")
    , m_strMsg_MediaName(L"MediaName")
    , m_strMsg_MediaState(L"MediaState")
    , m_strMsg_More(L"More")
    , m_strMsg_MusicFilter(L"MusicFilter")
    , m_strMsg_Mute(L"Mute")
    , m_strMsg_Navigate(L"Navigate")
    , m_strMsg_NoMore(L"NoMore")
    , m_strMsg_Ping(L"Ping")
    , m_strMsg_PlayAlbum(L"PlayAlbum")
    , m_strMsg_Pong(L"Pong")
    , m_strMsg_ReportState(L"ReportState")
    , m_strMsg_SendKeys(L"SendKeys")
    , m_strMsg_SendRemote(L"SendRemote")
    , m_strMsg_SetMediaFilter(L"SetMediaFilter")
    , m_strMsg_StartMCE(L"StartMCE")
    , m_strMsg_StateChanged(L"StateChanged")
    , m_strMsg_SubscribeOn(L"SubscribeEvents True")
    , m_strMsg_Trans_Next(L"SkipNext")
    , m_strMsg_Trans_Play(L"Play")
    , m_strMsg_Trans_Prev(L"SkipPrev")
    , m_strMsg_Trans_Random(L"Random")
    , m_strMsg_Trans_Repeat(L"Repeat")
    , m_strMsg_Trans_Stop(L"Stop")
    , m_strMsg_VolumeDown(L"VolumeDown")
    , m_strMsg_VolumeUp(L"VolumeUp")
    , m_strVal_False(L"False")
    , m_strVal_True(L"True")
    , m_thrLoader
      (
        L"AutonomicLoaderThread_" + cqcdcToLoad.strMoniker()
        , TMemberFunc<TAutonomicSDriver>(this, &TAutonomicSDriver::eLoaderThread)
      )
{
}

TAutonomicSDriver::~TAutonomicSDriver()
{
}


// ---------------------------------------------------------------------------
//  TAutonomicSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TAutonomicSDriver::bGetCommResource(TThread&)
{
    try
    {
        // Close if already opened, just in case, then open
        if (m_sockConn.bIsOpen())
            m_sockConn.Close();

        m_sockConn.Open(tCIDSock::ESockProtos::TCP);
    }

    catch(...)
    {
        // Not much we can do, so eat it, but do a close just in case
        m_sockConn.Close();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TAutonomicSDriver::bWaitConfig(TThread&)
{
    //
    //  We have no config to get (or we get it from the device), so just
    //  say we are happy.
    //
    return kCIDLib::True;
}


tCQCKit::ECommResults
TAutonomicSDriver::eBoolFldValChanged(  const   TString&            strName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_AdjVolume)
    {
        TString strMsg;
        bSendAndWaitForRep
        (
            bNewValue ? m_strMsg_VolumeUp : m_strMsg_VolumeDown
            , bNewValue ? m_strMsg_VolumeUp : m_strMsg_VolumeDown
            , 2500
            , strMsg
        );
    }
     else if (c4FldId == m_c4FldId_AdjChannel)
    {
        TString strMsg(m_strMsg_SendKeys);
        strMsg.Append(bNewValue ? L" Chan/PageUp" : L" Chan/PageDown");
        bSendAndWaitForRep(strMsg, m_strMsg_SendRemote, 5000, strMsg);
    }
     else if (c4FldId == m_c4FldId_Mute)
    {
        TString strMsg(m_strMsg_Mute);
        strMsg.Append(kCIDLib::chSpace);
        strMsg.Append(bNewValue ? m_strVal_True : m_strVal_False);
        bSendAndWaitForRep
        (
            strMsg
            , m_strMsg_Mute
            , 2500
            , strMsg
        );
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TAutonomicSDriver::eCardFldValChanged(  const   TString&        strName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    // We don't have any of these right now
    IncUnknownWriteCounter();
    return tCQCKit::ECommResults::FieldNotFound;
}


tCQCKit::ECommResults TAutonomicSDriver::eConnectToDevice(TThread& thrCaller)
{
    // Assume the worst until proven otherwise
    tCQCKit::ECommResults eRes = tCQCKit::ECommResults::LostConnection;

    //
    //  Try to connect the socket if not already. If that fails, we just
    //  indicate that we didn't connect and we'll try again next time.
    //
    try
    {
        if (!m_sockConn.bIsConnected())
        {
            m_sockConn.Connect(m_conncfgSock.ipepConn());

            // Send just a CR/LF to make him wake up
            SendMsg(L"");

            //
            //  It should have sent out an introductory banner of two
            //  lines, so eat them.
            //
            TString strTmp;
            if (!bWaitForReply(m_strMsg_BannerTerm, 6000, strTmp))
            {
                if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
                {
                    facAutonomicS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , L"Did not receive the connection banner"
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }
                return tCQCKit::ECommResults::LostCommRes;
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged()
        &&  (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // In this case, panic and just recycle the connect
        return tCQCKit::ECommResults::LostCommRes;
    }

    //
    //  Check the connection status. If we've not started up the loader
    //  thread yet, then do that. Else, we check the status to see if it
    //  is done yet or not.
    //
    if (m_eConnStatus == tAutonomicS::EConnStatus_StartLoad)
    {
        //
        //  If it's been long enough since the last try, we can crank up
        //  the loader thread. It will update the status before it lets
        //  us go.
        //
        if (TTime::enctNow() > m_enctNextLoad)
            m_thrLoader.Start();
    }
     else
    {
        //
        //  Check the status. If done, then we can set the response to
        //  success and we'll move forward to polling state. If the laod
        //  failed or succeeded, this call will set the last load time
        //  stamp, so that we will wait the appropriate minimum time
        //  before trying another load. It will also reset the status
        //  so that if we go offline, it will be set appropriately for
        //  when we come back in here to start trying to reconnect.
        //
        if (bMonitorLoad())
        {
            // Load all the initial status info
            eRes = eInitFields();

            //
            //  And enable async events. Use the poll method's temp string
            //  as our reply string here, since we know he doesn't need it
            //  right now.
            //
            bSendAndWaitForRep
            (
                m_strMsg_SubscribeOn, m_strMsg_Events, 2500, m_strPollTmp
            );
        }
    }

    // We don't have a device to connect to
    return eRes;
}


tCQCKit::EDrvInitRes TAutonomicSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
    {
        // Note that we are throwing a CQCKit error here!
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcDrv_BadConnCfgType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Config
            , cqcdcOurs.conncfgReal().clsIsA()
            , clsIsA()
            , TCQCIPConnCfg::clsThis()
        );
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());

    // Set our poll times
    SetPollTimes(500, 2500);

    // We should have gotten a prompt value with the MCE instance
    if (!cqcdcOurs.bFindPromptValue(L"MCEInstance", L"Text", m_strMCEInstance))
    {
        facAutonomicS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The MCE instance prompt was not set"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  And set up our device fields. The bulk of what this driver does is
    //  via the media interface, and this driver doesn't control any device
    //  or app, so there are just a few.
    //
    TVector<TCQCFldDef> colFlds(6);
    TCQCFldDef flddNew;

    const tCIDLib::TCh* const pszFldAdjChannel(L"AdjChannel");
    const tCIDLib::TCh* const pszFldAdjVolume(L"AdjVolume");
    const tCIDLib::TCh* const pszFldCIDName(L"CIDName");
    const tCIDLib::TCh* const pszFldCIDNumber(L"CIDNumber");
    const tCIDLib::TCh* const pszFldLoaded(L"Loaded");
    const tCIDLib::TCh* const pszFldMediaState(L"MediaState");
    const tCIDLib::TCh* const pszFldMute(L"Mute");
    const tCIDLib::TCh* const pszFldNavigation(L"Navigation");
    const tCIDLib::TCh* const pszFldReloadDB(L"ReloadDB");
    const tCIDLib::TCh* const pszFldRunning(L"Running");
    const tCIDLib::TCh* const pszFldScreenNav(L"ScreenNavigation");
    const tCIDLib::TCh* const pszFldStatus(L"Status");
    const tCIDLib::TCh* const pszFldTitleCnt(L"TitleCount");
    const tCIDLib::TCh* const pszFldTransport(L"Transport");
    const tCIDLib::TCh* const pszFldVolume(L"Volume");

    flddNew.Set
    (
        pszFldAdjChannel
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldAdjVolume
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldCIDName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldCIDNumber
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurColArtist
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurColName
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurItemTime
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_CurItemTotal
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_EnqueueMedia
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldLoaded
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldMediaState
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldMute
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        kCQCMedia::strMediaFld_PlayMedia
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldNavigation
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: Home, Up, Down, Left, Right, OK, Back, Details, Guide"
          L", Jump, MoreInfo, Play, Pause, Stop, Record, FastForward, Rewind"
          L", Skip, Replay, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, Clear, Enter"
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
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
        pszFldRunning
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldScreenNav
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: InternetRadio, LiveTV, MorePrograms, MusicAlbums, MusicArtists"
          L", MusicSongs, MyMusic, MyPictures, MyTV, MyVideos, RecordedTV"
          L", ScheduledTVRecordings, SlideShow, Start, TVGuide, Visualizations"
          L", PhotoDetails"
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
        pszFldTitleCnt
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldTransport
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: Play, Stop, Pause, SkipPrev, SkipNext"
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        pszFldVolume
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    //
    //  Now register our fields with the base driver class, and go back
    //  and look up the ids.
    //
    SetFields(colFlds);
    m_c4FldId_AdjVolume = pflddFind(pszFldAdjVolume, kCIDLib::True)->c4Id();
    m_c4FldId_AdjChannel = pflddFind(pszFldAdjChannel, kCIDLib::True)->c4Id();
    m_c4FldId_CIDName = pflddFind(pszFldCIDName, kCIDLib::True)->c4Id();
    m_c4FldId_CIDNumber = pflddFind(pszFldCIDNumber, kCIDLib::True)->c4Id();
    m_c4FldId_CurColArtist = pflddFind(kCQCMedia::strMediaFld_CurColArtist, kCIDLib::True)->c4Id();
    m_c4FldId_CurColName = pflddFind(kCQCMedia::strMediaFld_CurColName, kCIDLib::True)->c4Id();
    m_c4FldId_CurItemTime = pflddFind(kCQCMedia::strMediaFld_CurItemTime, kCIDLib::True)->c4Id();
    m_c4FldId_CurItemTotal = pflddFind(kCQCMedia::strMediaFld_CurItemTotal, kCIDLib::True)->c4Id();
    m_c4FldId_EnqueueMedia = pflddFind(kCQCMedia::strMediaFld_EnqueueMedia, kCIDLib::True)->c4Id();
    m_c4FldId_Loaded = pflddFind(pszFldLoaded, kCIDLib::True)->c4Id();
    m_c4FldId_MediaState = pflddFind(pszFldMediaState, kCIDLib::True)->c4Id();
    m_c4FldId_Mute = pflddFind(pszFldMute, kCIDLib::True)->c4Id();
    m_c4FldId_Navigation = pflddFind(pszFldNavigation, kCIDLib::True)->c4Id();
    m_c4FldId_PlayMedia = pflddFind(kCQCMedia::strMediaFld_PlayMedia, kCIDLib::True)->c4Id();
    m_c4FldId_ReloadDB = pflddFind(pszFldReloadDB, kCIDLib::True)->c4Id();
    m_c4FldId_Running = pflddFind(pszFldRunning, kCIDLib::True)->c4Id();
    m_c4FldId_ScreenNavigation = pflddFind(pszFldScreenNav, kCIDLib::True)->c4Id();
    m_c4FldId_Status = pflddFind(pszFldStatus, kCIDLib::True)->c4Id();
    m_c4FldId_TitleCnt = pflddFind(pszFldTitleCnt, kCIDLib::True)->c4Id();
    m_c4FldId_Transport = pflddFind(pszFldTransport, kCIDLib::True)->c4Id();
    m_c4FldId_Volume = pflddFind(pszFldVolume, kCIDLib::True)->c4Id();

    //
    //  Init the status field to indicate not loaded. The loaded field will
    //  have a default initial value of false, which is what we want.
    //
    bStoreStringFld
    (
        m_c4FldId_Status
        , facAutonomicS().strMsg(kAnomicMsgs::midStatus_NotLoaded)
        , kCIDLib::True
    );

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TAutonomicSDriver::ePollDevice(TThread& thrCaller)
{
    // Just watch for any asyncs that come in
    while (bGetMsg(m_strPollTmp, TTime::enctNowPlusMSs(5)))
        bProcessMsg(m_strPollTmp);

    //
    //  If we've not gotten a message in the last 30 seconds, then do an
    //  active ping.
    //
    if (m_enctLastMsg + (30 * kCIDLib::enctOneSecond) < TTime::enctNow())
    {
        if (!bSendAndWaitForRep(m_strMsg_Ping, m_strMsg_Pong, 2500, m_strPollTmp))
            return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


// A string field changed
tCQCKit::ECommResults
TAutonomicSDriver::eStringFldValChanged(const   TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    if ((c4FldId == m_c4FldId_EnqueueMedia)
    ||  (c4FldId == m_c4FldId_PlayMedia))
    {
        // Call a helper for this one
        if (!bDoPlayMedia(strNewValue, (c4FldId == m_c4FldId_EnqueueMedia)))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_Navigation)
    {
        // This is a send and pray type deal
        TString strTmp(m_strMsg_SendKeys);
        strTmp.Append(kCIDLib::chSpace);
        strTmp.Append(strNewValue);
        SendMsg(strTmp);
    }
     else if (c4FldId == m_c4FldId_ScreenNavigation)
    {
        TString strMsg(m_strMsg_Navigate);
        strMsg.Append(kCIDLib::chSpace);
        strMsg.Append(strNewValue);
        if (!bSendAndWaitForRep(strMsg, m_strMsg_StateChanged, 5000, strMsg))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else if (c4FldId == m_c4FldId_Transport)
    {
        TString strTmp;
        if (!bSendAndWaitForRep(strNewValue, strNewValue, 6000, strTmp))
            return tCQCKit::ECommResults::ValueRejected;
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Clean up our socket. The cover art socket is only used during the
//  actual download process, so we only keep the main connection around
//  long term.
//
tCIDLib::TVoid TAutonomicSDriver::ReleaseCommResource()
{
    // First do a clean shutdown
    try
    {
        m_sockConn.c4Shutdown();
    }

    catch(...) {}

    // And close it
    try
    {
        m_sockConn.Close();
    }

    catch(...) {}
}


// We need to make sure our loader thread is down before we are dumped
tCIDLib::TVoid TAutonomicSDriver::TerminateImpl()
{
    // Shut down the loader thread if its running
    if (m_thrLoader.bIsRunning())
    {
        try
        {
            m_thrLoader.ReqShutdownSync(8000);
            m_thrLoader.eWaitForDeath(2500);
        }

        catch(const TError& errToCatch)
        {
            if (!errToCatch.bNogged())
                TModule::LogEventObj(errToCatch);
        }
    }

    // Tell our parent class to flush the media database
    ResetMediaDB();
}


// ---------------------------------------------------------------------------
//  TAutonomicSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when a client writes to our PlayMedia or EnqueueMedia
//  fields. We get the value written, which should be a title or collection
//  cookie.
//
tCIDLib::TBoolean
TAutonomicSDriver::bDoPlayMedia(const   TString&            strToPlay
                                , const tCIDLib::TBoolean   bEnqueue)
{
    // Parse this cookie using the version that figures out the type for us
    tCIDLib::TCard2        c2CatId;
    tCIDLib::TCard2        c2ColId;
    tCIDLib::TCard2        c2ItemId;
    tCIDLib::TCard2        c2TitleId;
    tCQCMedia::EMediaTypes eType;
    const tCQCMedia::ECookieTypes eCookieType = facCQCMedia().eCheckCookie
    (
        strToPlay, eType, c2CatId, c2TitleId, c2ColId, c2ItemId
    );

    // We only support music
    if (eType != tCQCMedia::EMediaTypes::Music)
        return kCIDLib::False;

    //
    //  We need a title and collection cookie, no matter what we got, so
    //  build up the one we didn't get.
    //
    TString strColCookie;
    TString strTitleCookie;
    if (eCookieType == tCQCMedia::ECookieTypes::Title)
    {
        strColCookie = strToPlay;
        strColCookie.Append(L",1");
        strTitleCookie = strToPlay;
    }
     else if (eCookieType == tCQCMedia::ECookieTypes::Collect)
    {
        strColCookie = strToPlay;
        facCQCMedia().FormatTitleCookie(eType, c2CatId, c2TitleId, strTitleCookie);
    }
     else
    {
        // It's not a valid cookie type for this type of oepration
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcRend_BadCookie
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }

    // Look up the indicated title set
    TMediaTitleSet* pmtsPlay = mdbInfo().pmtsById
    (
        tCQCMedia::EMediaTypes::Music, c2TitleId, kCIDLib::True
    );

    // They only ever have one collection in this repository, so get that
    TMediaCollect& mcolPlay = pmtsPlay->mcolAt(mdbInfo(), 0);

    // Ok, the unique id is the GUI id we want to send, so that's all we need
    TString strMsg(m_strMsg_PlayAlbum);
    strMsg.Append(kCIDLib::chSpace);
    strMsg.Append(pmtsPlay->strUniqueId());
    strMsg.Append(kCIDLib::chSpace);
    if (bEnqueue)
        strMsg.Append(m_strVal_True);
    else
        strMsg.Append(m_strVal_False);

    //
    //  Use one of the cookie strings as a temp to just get the reply
    //  back in.
    //
    return bSendAndWaitForRep(strMsg, m_strMsg_PlayAlbum, 5000, strColCookie);
}


//
//  This is called when a load is underway. We watch the load status and
//  handle with it finishes.
//
tCIDLib::TBoolean TAutonomicSDriver::bMonitorLoad()
{
    //
    //  Get copies of the status members. These are fundamental types so we
    //  don't worry about synchronization here.
    //
    tCIDLib::TCard4 c4SoFar = m_c4TitlesDone;
    const tAutonomicS::EConnStatus eStatus = m_eConnStatus;

    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (eStatus > tAutonomicS::EConnStatus_Loading)
    {
        try
        {
            // Make sure the repo thread is down and out
            m_thrLoader.ReqShutdownSync(8000);
            m_thrLoader.eWaitForDeath(3000);

            // If it failed, log that
            if (eStatus == tAutonomicS::EConnStatus_Failed)
            {
                // Set the status field to not loaded and leave loaded field false
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , facAutonomicS().strMsg(kAnomicMsgs::midStatus_NotLoaded)
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
            }
             else
            {
                // Store the new contents from the load DB to ours
                ResetMediaDB(m_mdbLoad);

                // Update the load status fields
                bStoreStringFld
                (
                    m_c4FldId_Status
                    , facAutonomicS().strMsg(kAnomicMsgs::midStatus_Loaded)
                    , kCIDLib::True
                );
                bStoreCardFld
                (
                    m_c4FldId_TitleCnt
                    , mdbInfo().c4TitleSetCnt(tCQCMedia::EMediaTypes::Music)
                    , kCIDLib::True
                );
                bStoreBoolFld(m_c4FldId_Loaded, kCIDLib::True, kCIDLib::True);

                // We succeeded so return true
                bRet = kCIDLib::True;
            }
        }

        catch(const TError& errToCatch)
        {
            if (!errToCatch.bNogged())
                TModule::LogEventObj(errToCatch);

            // Fall through with false return value
        }

        // Set the next minimum load time to now plus 10 seconds
        m_enctNextLoad = TTime::enctNowPlusSecs(10);

        // And reset the status for next time
        m_eConnStatus = tAutonomicS::EConnStatus_StartLoad;
    }
     else
    {
        // Not done yet, so keep waiting, but update the status field
        TString strSet(m_strLoadMsg);
        strSet.ReplaceToken(c4SoFar, kCIDLib::chDigit1);
        TLocker lockrField(pmtxField());
        bStoreStringFld(m_c4FldId_Status, strSet, kCIDLib::True);
    }

    return bRet;
}


//
//  We send a get status request and store away all the returned info.
//  We have to field lock here while updating fields, since we call it
//  both from the connect and perodically just to make sure we are in
//  sync.
//
tCQCKit::ECommResults TAutonomicSDriver::eInitFields()
{
    // Ask for the status info
    SendMsg(m_strMsg_GetMCEStatus);

    //
    //  We loop until we see the Running status, which is the last one.
    //  For each one, we pull out the key=value, map the key to our field,
    //  and store the value.
    //
    tCIDLib::TEncodedTime enctCur = TTime::enctNow();
    tCIDLib::TEncodedTime enctEnd = enctCur + (kCIDLib::enctOneSecond * 5);

    tCIDLib::TCard4 c4CurIndex = 1;
    TString strMsg;
    while (enctCur < enctEnd)
    {
        if (!bWaitForReply(m_strMsg_ReportState, 2500, strMsg))
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
            {
                facAutonomicS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Failed to get status report reply"
                    , tCIDLib::ESeverities::Status
                );
            }
            return tCQCKit::ECommResults::LostConnection;
        }

        //
        //  Just call the same method that processes these when they arrive
        //  asynch. He doesn't know or care how they get here. It returns
        //  true if it sees the Running state change, which is always the
        //  last in the list, so we can just break out when we see it.
        //
        if (bProcessMsg(strMsg))
            break;
    }
    return tCQCKit::ECommResults::Success;
}


