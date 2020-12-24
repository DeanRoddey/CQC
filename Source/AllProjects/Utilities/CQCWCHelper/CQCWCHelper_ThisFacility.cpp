//
// FILE NAME: CQCWCHelper_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This file implements the facilty class for the web camera helper facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWCHelper.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCWCHelper,TGUIFacility)


static tCIDLib::TVoid
PlayerErrCB(const struct libvlc_event_t* pEvent, tCIDLib::TVoid* pFacPtr)
{
    TFacCQCWCHelper* pfacCB = static_cast<TFacCQCWCHelper*>(pFacPtr);

    if (pEvent->type == libvlc_MediaPlayerEndReached)
    {
        pfacCB->SetPlaybackState(kCIDLib::False);
    }
     else if (pEvent->type == libvlc_MediaPlayerEncounteredError)
    {
        pfacCB->SetPlaybackState(kCIDLib::False);
    }
     else if (pEvent->type == libvlc_MediaPlayerPlaying)
    {
        pfacCB->SetPlaybackState(kCIDLib::True);
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TPlMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TPlMsg: Constructors and Destructor
// ---------------------------------------------------------------------------

TPlMsg::TPlMsg() :

    m_bVal(kCIDLib::False)
    , m_c4Val(0)
    , m_eType(tCQCWCHelper::EPlMsgTypes::None)
{
}

TPlMsg::TPlMsg(const    tCQCWCHelper::EPlMsgTypes   eType
                , const tCIDLib::TBoolean           bVal) :

    m_bVal(bVal)
    , m_c4Val(0)
    , m_eType(eType)
{
}

TPlMsg::TPlMsg(const    tCQCWCHelper::EPlMsgTypes   eType
                , const tCIDLib::TCard4             c4Val) :

    m_bVal(kCIDLib::False)
    , m_c4Val(c4Val)
    , m_eType(eType)
{
}

TPlMsg::TPlMsg(const TPlMsg& qmsgSrc) :

    m_bVal(qmsgSrc.m_bVal)
    , m_c4Val(qmsgSrc.m_c4Val)
    , m_eType(qmsgSrc.m_eType)
{
}

TPlMsg::~TPlMsg()
{
}


// ---------------------------------------------------------------------------
//  TPlMsg: Public operators
// ---------------------------------------------------------------------------
TPlMsg& TPlMsg::operator=(const TPlMsg& qmsgSrc)
{
    if (this != &qmsgSrc)
    {
        m_bVal  = qmsgSrc.m_bVal;
        m_c4Val = qmsgSrc.m_c4Val;
        m_eType = qmsgSrc.m_eType;
    }
    return *this;
}



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWCHelper
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCWCHelper: Constructors and Destructor
// ---------------------------------------------------------------------------

TFacCQCWCHelper::TFacCQCWCHelper() :

    TGUIFacility
    (
        L"CQCWCHelper"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bReqMuteState(kCIDLib::False)
    , m_bVerboseLogging(kCIDLib::False)
    , m_c4ReqVolume(100)
    , m_colGUIMsgQ(tCIDLib::EMTStates::Safe)
    , m_colPlMsgQ(tCIDLib::EMTStates::Safe)
    , m_colParams()
    , m_enctLastMsg(TTime::enctNow())
    , m_eCurState(tCQCWCHelper::EStates::InitVLC)
    , m_porbsCtrl(nullptr)
    , m_pVLCInst(nullptr)
    , m_pVLCPlayer(nullptr)
    , m_thrManager
      (
        TString(L"CQCWCHelper")
        , TMemberFunc<TFacCQCWCHelper>(this, &TFacCQCWCHelper::eMgrThread)
      )
{
}

TFacCQCWCHelper::~TFacCQCWCHelper()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCWCHelper: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Return the next available message, without waiting. If nothing, returns false.
tCIDLib::TBoolean TFacCQCWCHelper::bGetNextGUIMsg(TGUIMsg& qmsgToFill)
{
    return m_colGUIMsgQ.bGetNext(qmsgToFill, 0);

}


// Called by the main window to see if we have timed out
tCIDLib::TBoolean TFacCQCWCHelper::bTimedOut()
{
    return (m_enctLastMsg + kCQCWCHelper::enctTimeout) < TTime::enctNow();
}


tCIDLib::TVoid TFacCQCWCHelper::Close()
{
    // And now queue up the message
    QueueGUIMsg(TGUIMsg(tCQCWCHelper::EGUIMsgTypes::Close));
}


// Let the main window know what our current state is
tCQCWCHelper::EStates TFacCQCWCHelper::eCurState() const
{
    return m_eCurState;
}


tCIDLib::EExitCodes TFacCQCWCHelper::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    try
    {
        // Get the standard environmental info
        TString strFailReason;
        if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        {
            TAudio::Beep(880, 100);
            TAudio::Beep(660, 100);
            return tCIDLib::EExitCodes::BadEnvironment;
        }

        facCQCWCHelper().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWCHMsgs::midStatus_Start
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );

        // Initialize the client side ORB
        facCIDOrb().InitClient();

        // Install a logger
        TModule::InstallLogger
        (
            new TLogSrvLogger(facCQCKit().strLocalLogDir())
            , tCIDLib::EAdoptOpts::Adopt
        );

        //
        //  We get a single command line parameter, which is the unique id the invoking
        //  client assigned us.
        //
        tCIDLib::TCard4 c4Count = TSysInfo::c4CmdLineParmCount();
        if (c4Count != 1)
        {
            return tCIDLib::EExitCodes::BadParameters;
        }

        // Get the first one, which should be the unique id
        TString strUniqueId = TSysInfo::strCmdLineParmAt(0);

        //
        //  To allow for standalone testing, they can pass DoDummyTesting as the unique
        //  id. We'll push fake info into members that would otherwise come from the
        //  controlling application.
        //
        const tCIDLib::TBoolean bDummyTest = strUniqueId.bCompareI(L"DoDummyTesting");

        // Open the memory buffer he created for us
        TResourceName rsnBuf(kCIDLib::pszResCompany, L"CQCWCHelper", strUniqueId);
        tCIDLib::TBoolean bCreated;
        TSharedMemBuf mbufInfo
        (
            4096
            , 8192
            , rsnBuf
            , bCreated
            , tCIDLib::EMemAccFlags::ReadWrite
            , bDummyTest ? tCIDLib::ECreateActs::CreateAlways : tCIDLib::ECreateActs::OpenIfExists
        );

        // And the ready event
        TEvent evDataReady
        (
            rsnBuf
            , bCreated
            , tCIDLib::EEventStates::Reset
            , bDummyTest ? tCIDLib::ECreateActs::CreateIfNew : tCIDLib::ECreateActs::OpenIfExists
            , kCIDLib::True
        );


        //
        //  To allow for standalone testing, they can pass DoDummyTesting as the unique
        //  id. We'll push fake info into members that would otherwise come from the
        //  controlling application. Change the address and port as required.
        //
        tCIDCtrls::TWndHandle hwndOwner;
        TString strParams;
        if (bDummyTest)
        {
            hwndOwner = TWindow::wndDesktop().hwndThis();
            m_strURL = L"rtsp://admin@192.168.0.14:554/11";
        }
         else
        {
            //
            //  Stream in the data he passes us. The first Card4 is the overall size, which we
            //  need to set up the input stream.
            //
            TBinMBufInStream strmSrc(&mbufInfo, mbufInfo.c4At(0));

            // Skip the size
            tCIDLib::TCard4 c4Size;
            strmSrc >> c4Size;

            // Next should be a start object mark
            strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

            // And now the data
            tCIDLib::TCard4 c4Handle;
            strmSrc >> m_strURL
                    >> c4Handle
                    >> strParams
                    >> m_bVerboseLogging
                    >> m_bReqMuteState
                    >> m_c4ReqVolume;

            hwndOwner = tCIDCtrls::TWndHandle(c4Handle);

            // And it should end with an end object marker
            strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);

            // If verbose logging, log what we got
            if (bVerboseLogging())
            {
                TString strMsg(L"Incoming info... URL=", 1024UL);
                strMsg.Append(m_strURL);
                strMsg.Append(L"  Params=");
                strMsg.Append(strParams);
                facCQCWCHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , strMsg
                    , tCIDLib::ESeverities::Status
                );
            }
        }

        #if CID_DEBUG_ON
        // To aid debugging sometimes...
        // thrThis.Sleep(25000);
        #endif

        // Parse our the parameters if we got any
        if (!strParams.bIsEmpty())
        {
            tCIDLib::TCard4 c4ErrInd;
            tCIDLib::TStrList colParams;
            if (!TStringTokenizer::bParseQuotedCommaList(strParams, colParams, c4ErrInd))
            {
                facCQCWCHelper().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWCHErrs::errcCam_BadParams
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
                return tCIDLib::EExitCodes::Normal;
            }

            TString strKey;
            TString strVal;
            const tCIDLib::TCard4 c4Count = colParams.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                strKey = colParams[c4Index];
                if (!strKey.bSplit(strVal, kCIDLib::chEquals))
                {
                    facCQCWCHelper().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kWCHErrs::errcCam_BadParams
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                    );
                    return tCIDLib::EExitCodes::Normal;
                }

                m_colParams.objAdd(TKeyValuePair(strKey, strVal));
            }
        }

        //
        //  Initialize the server side ORB. We only accept one connection, and let the
        //  system select the port.
        //
        facCIDOrb().InitServer(0, 1);

        //
        //  We have to give it time to get it's listener going, which it does async,
        //  because we have to report the selected port.
        //
        c4Count = 0;
        while (!facCIDOrb().ippnORB() && (c4Count < 25))
        {
            if (!thrThis.bSleep(100))
                break;
        }

        // If we never got it, give up
        if (!facCIDOrb().ippnORB())
            return tCIDLib::EExitCodes::Normal;

        // Log what we ended up selecting
        if (bVerboseLogging())
        {
            facCQCWCHelper().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWCHMsgs::midStatus_ListeningOn
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(TProcess::pidThis())
                , TCardinal(facCIDOrb().ippnORB())
            );
        }

        //
        //  Let's construct our faux object id
        //
        //  NOTE: For some reason the compiler insists this must be done via
        //  move assignment, which this class doesn't support. So I have to
        //  create a temporary and then assign that.
        //
        TOrbObjId ooidTmp
        (
            TCQCWCHelperServerBase::strInterfaceId
            , TUniqueId::strMakeId()
            , L"localhost"
            , facCIDOrb().ippnORB()
            , L"TCQCWCHelperClientProxy"
        );
        m_ooidServer = ooidTmp;

        // Create our control interface with this object id
        m_porbsCtrl = new TWCHelperCtrlImpl(m_ooidServer);

        //
        //  Register it with the ORB. No need for the rebinder here because we don't use
        //  the name server.
        //
        facCIDOrb().RegisterObject(m_porbsCtrl, tCIDLib::EAdoptOpts::Adopt);

        // Create the main frame window
        m_wndMainFrame.Create(hwndOwner, bDummyTest);

        // Create an output stream over the shared memory and put out the return info
        if (bDummyTest)
        {
            facCIDCtrls().ShowGUI(m_wndMainFrame);
        }
         else
        {
            tCIDLib::TCard4 c4Size = 0;
            {
                TBinMBufOutStream strmTar(&mbufInfo);

                // Skip the initial Card4 which has the final size
                strmTar << tCIDLib::TCard4(0)
                        << tCIDLib::EStreamMarkers::StartObject
                        << m_ooidServer
                        << tCIDLib::TCard4(m_wndMainFrame.hwndThis())
                        << tCIDLib::EStreamMarkers::EndObject
                        << kCIDLib::FlushIt;

                c4Size = strmTar.c4CurSize();
            }

            // Put the size into the first card4 now.
            mbufInfo.PutCard4(c4Size, 0);

            // And trigger the ready event to let him know the return data is ready
            evDataReady.Trigger();
        }

        // Let start up our manager thread
        m_thrManager.Start();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        return tCIDLib::EExitCodes::InitFailed;
    }

    // Process messages until we are told to exit
    facCIDCtrls().uMainMsgLoop(m_wndMainFrame);

    try
    {
        // Stop the manager thread
        m_thrManager.ReqShutdownSync(5000);
        m_thrManager.eWaitForDeath(5000);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    try
    {
        // Now we can destroy the frame window
        m_wndMainFrame.Destroy();

        // And now we can terminate the ORB
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        #if CID_DEBUG_ON
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop(), L"Web Camera Helper", L"Termination failed", errToCatch
        );
        #endif
    }

    catch(...)
    {
        #if CID_DEBUG_ON
        TErrBox msgbErr(L"Web Camera Helper", L"Termination failed");
        msgbErr.ShowIt(TWindow::wndDesktop());
        #endif
    }
    return tCIDLib::EExitCodes::Normal;
}


// Mostly we use this ourself, but the window and other stuff might need it
tCIDLib::TBoolean TFacCQCWCHelper::bVerboseLogging() const
{
    return m_bVerboseLogging;
}


tCIDLib::TVoid
TFacCQCWCHelper::Initialize(const   TArea&              areaPar
                            , const TArea&              areaWidget
                            , const tCIDLib::TBoolean   bVisibility)
{
    // Just queue it up, also updates the last msg time stamp
    QueueGUIMsg(TGUIMsg(areaPar, areaWidget, bVisibility));
}



tCIDLib::TVoid TFacCQCWCHelper::Ping()
{
    // Just update our last msg time stamp
    m_enctLastMsg = TTime::enctNow();
}


tCIDLib::TVoid TFacCQCWCHelper::SetArea(const TArea& areaPar, const TArea& areaWidget)
{
    // Just queue it up, also updates the last msg time stamp
    QueueGUIMsg(TGUIMsg(areaPar, areaWidget));
}


//
//  Our callback handler will call this to let us know that our playback state has changed.
//  Either we have started playing, or playback ended.
//
tCIDLib::TVoid TFacCQCWCHelper::SetPlaybackState(const tCIDLib::TBoolean bNewState)
{
    if (bNewState)
    {
        //
        //  Set the audio mute that we were last asked to set. We can't do this until
        //  playback actually starts, since until then it won't necessarily have any
        //  active audio tracks to mute.
        //
        if (m_pVLCPlayer)
        {
            libvlc_audio_set_mute(m_pVLCPlayer, m_bReqMuteState ? 1 : 0);
            libvlc_audio_set_volume(m_pVLCPlayer, m_c4ReqVolume);
        }

        //
        //  If we are currently in waiting for playback state, then the background thread
        //  is waiting for us to see this, so just update his state to make him move forward.
        //
        if (m_eCurState == tCQCWCHelper::EStates::WaitPlayback)
            m_eCurState = tCQCWCHelper::EStates::Playing;
    }
     else
    {
        //
        //  If we are in playing state, then this means we've lost playback, so if
        //  we need to start it to restarting state.
        //
        if (m_eCurState == tCQCWCHelper::EStates::Playing)
            m_eCurState = tCQCWCHelper::EStates::Restarting;
    }
}


//
//  We store the new mute state as the last requested one. And we queue up a mute
//  command to the player msg queue.
//
tCIDLib::TVoid TFacCQCWCHelper::SetMute(const tCIDLib::TBoolean bNewState)
{
    if (bNewState != m_bReqMuteState)
    {
        m_bReqMuteState = bNewState;
        QueuePlMsg(TPlMsg(tCQCWCHelper::EPlMsgTypes::SetMute, bNewState));
    }
}


tCIDLib::TVoid TFacCQCWCHelper::SetVisibility(const tCIDLib::TBoolean bNewState)
{
    // Just queue it up
    QueueGUIMsg(TGUIMsg(tCQCWCHelper::EGUIMsgTypes::SetVisibility, bNewState));
}


//
//  We store the new volume as the last requested one. And we queue up a volume
//  command to the player msg queue.
//
tCIDLib::TVoid TFacCQCWCHelper::SetVolume(const tCIDLib::TCard4 c4ToSet)
{
    if (c4ToSet != m_c4ReqVolume)
    {
        m_c4ReqVolume = c4ToSet;
        QueuePlMsg(TPlMsg(tCQCWCHelper::EPlMsgTypes::SetVolume, c4ToSet));
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCWCHelper: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Once the player is created, this is called to allow us to apply any parameters
//  that we got.
//
tCIDLib::TVoid TFacCQCWCHelper::ApplyParams()
{
    const tCIDLib::TCard4 c4Count = m_colParams.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = m_colParams[c4Index];
        if (kvalCur.strKey().bCompareI(TCQCWCHelperServerBase::strParam_CustAR))
        {
            tCIDLib::TSCh* pszVal = TRawStr::pszConvert(kvalCur.strValue().pszBuffer());
            TArrayJanitor<tCIDLib::TSCh> janURL(pszVal);
            ::libvlc_video_set_aspect_ratio(m_pVLCPlayer, pszVal);
        }
    }
}



tCIDLib::EExitCodes
TFacCQCWCHelper::eMgrThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    //
    //  We will track how long we have been in playing state. If it's been more than 1
    //  hour straight, we will force a reset, because sometimes things get weird if
    //  we don't.
    //
    tCIDLib::TEncodedTime enctLastPlay = 0;
    #if CID_DEBUG_ON
    const tCIDLib::TEncodedTime enctRestartInterval = kCIDLib::enctOneMinute * 2;
    #else
    const tCIDLib::TEncodedTime enctRestartInterval = kCIDLib::enctOneHour;
    #endif

    // Used to time out waiting for states to change or for recovery
    tCIDLib::TEncodedTime enctWaitState = 0;

    TPlMsg qmsgCur;
    while (!thrThis.bCheckShutdownRequest())
    {
        try
        {
            switch(m_eCurState)
            {
                case tCQCWCHelper::EStates::InitVLC :
                {
                    // Make sure our time stamps are reset to be safe
                    enctLastPlay = 0;
                    enctWaitState = 0;

                    m_pVLCInst = libvlc_new(0, NULL);
                    if (m_pVLCInst)
                        m_eCurState = tCQCWCHelper::EStates::InitPlayer;
                    break;
                }

                case tCQCWCHelper::EStates::InitPlayer :
                {
                    // Convert the URL to short chars
                    tCIDLib::TSCh* pszURL = TRawStr::pszConvert(m_strURL.pszBuffer());
                    TArrayJanitor<tCIDLib::TSCh> janURL(pszURL);

                    // Try to create the player. Give up if not
                    m_pVLCPlayer = ::libvlc_media_player_new(m_pVLCInst);
                    if (!m_pVLCPlayer)
                    {
                        m_eCurState = tCQCWCHelper::EStates::Failed;
                        enctWaitState = TTime::enctNowPlusSecs(20);
                    }
                     else
                    {
                        // We got the player so associate it with our window
                        ::libvlc_media_player_set_hwnd
                        (
                            m_pVLCPlayer, m_wndMainFrame.wndVideo().hwndThis()
                        );

                        libvlc_media_t *pVLCMedia = nullptr;
                        pVLCMedia = ::libvlc_media_new_location(m_pVLCInst, pszURL);
                        if (!pVLCMedia)
                        {
                            m_eCurState = tCQCWCHelper::EStates::Failed;
                            enctWaitState = TTime::enctNowPlusSecs(20);
                        }
                         else
                        {
                            // Create a new player for the media we created
                            ::libvlc_media_player_set_media(m_pVLCPlayer, pVLCMedia);

                            // And we can let the media go now that the player has it
                            ::libvlc_media_release(pVLCMedia);
                        }
                    }

                    // If still in initial player state, do the rest of the setup
                    if (m_eCurState == tCQCWCHelper::EStates::InitPlayer)
                    {
                        //
                        //  Get the player's event manager and register for the events
                        //  we want to handle.
                        //
                        RegisterVLCEvents();

                        // Apply any parameters we got
                        ApplyParams();

                        // Point the player at our window and start it playing
                        int iRes = ::libvlc_media_player_play(m_pVLCPlayer);
                        if (iRes == -1)
                        {
                            m_eCurState = tCQCWCHelper::EStates::Failed;
                            enctWaitState = TTime::enctNowPlusSecs(20);
                        }
                         else
                        {
                            //
                            //  And now we can move to waiting for playback to start, and
                            //  store a time. If we don't see playback start by that time,
                            //  we'll recycle again.
                            //
                            enctWaitState = TTime::enctNowPlusSecs(10);
                            m_eCurState = tCQCWCHelper::EStates::WaitPlayback ;
                        }
                    }
                    break;
                }

                case tCQCWCHelper::EStates::WaitPlayback :
                {
                    //
                    //  If our event handler sees a 'playing' event , it will calls us
                    //  to let us know. If our state is wait playback, it will move it
                    //  forward to playing.
                    //
                    //  We also handle timing out.
                    //
                    if (TTime::enctNow() > enctWaitState)
                        m_eCurState = tCQCWCHelper::EStates::Restarting;
                    break;
                }

                case tCQCWCHelper::EStates::Playing :
                {
                    //
                    //  Our event handler will call us if it sees the media stop playing.
                    //  If our state is playing, we will go into restarting state.
                    //
                    //  If we haven't started tracking playback time, then do that, else
                    //  see how long we've been playing. If more than our reset time,
                    //  then restart us.
                    //
                    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
                    if (!enctLastPlay)
                    {
                        enctLastPlay = enctNow;
                    }
                     else if ((enctNow - enctLastPlay) > enctRestartInterval)
                    {
                        enctLastPlay = 0;
                        m_eCurState = tCQCWCHelper::EStates::Restarting;
                    }
                    break;
                }

                case tCQCWCHelper::EStates::Restarting :
                {
                    // Just to be extra safe
                    enctLastPlay = 0;
                    enctWaitState = 0;

                    // We release all of the resource, and go back to InitVLC mode.
                    if (m_pVLCPlayer)
                    {
                        ::libvlc_media_player_release(m_pVLCPlayer);
                        m_pVLCPlayer = nullptr;
                    }

                    if (m_pVLCInst)
                    {
                        ::libvlc_release(m_pVLCInst);
                        m_pVLCInst = nullptr;
                    }

                    m_eCurState = tCQCWCHelper::EStates::InitVLC;
                    break;
                }

                case tCQCWCHelper::EStates::Failed :
                {
                    //
                    //  Upon failure we come here and will wait for a while, then
                    //  we move to restart mode.
                    //
                    if (TTime::enctNow() > enctWaitState)
                        m_eCurState = tCQCWCHelper::EStates::Restarting;
                    break;
                }

                default :
                    break;
            };
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }

        //
        //  Watch for msgs to process, and this also serves to throttle us when there
        //  aren't any, which is almost all the time.
        //
        try
        {
            if (m_colPlMsgQ.bGetNext(qmsgCur, 100))
            {
                switch(qmsgCur.m_eType)
                {
                    case tCQCWCHelper::EPlMsgTypes::SetMute :
                    {
                        // If we have a player object set the mute
                        if (m_pVLCPlayer)
                            libvlc_audio_set_mute(m_pVLCPlayer, qmsgCur.m_bVal ? 1 : 0);
                        break;
                    }

                    case tCQCWCHelper::EPlMsgTypes::SetVolume :
                    {
                        // If we have a player object set the volume
                        if (m_pVLCPlayer)
                            libvlc_audio_set_volume(m_pVLCPlayer, qmsgCur.m_c4Val);
                        break;
                    }

                    default :
                        break;
                };
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }

    // Shut down our VLC resources
    if (m_pVLCPlayer)
    {
        libvlc_media_player_release(m_pVLCPlayer);
        m_pVLCPlayer = nullptr;
    }

    if (m_pVLCInst)
    {
        libvlc_release(m_pVLCInst);
        m_pVLCInst = nullptr;
    }

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TFacCQCWCHelper::QueueGUIMsg(const TGUIMsg& qmsgToDo)
{
    m_enctLastMsg = TTime::enctNow();

    // Should not be remotely possible, but just in case
    if (m_colGUIMsgQ.bIsFull(16))
    {
        facCQCWCHelper().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWCHMsgs::midStatus_QFull
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"GUI")
        );
        m_colGUIMsgQ.RemoveAll();
    }
     else if (qmsgToDo.m_eType == tCQCWCHelper::EGUIMsgTypes::SetArea)
     {
        if (facCQCWCHelper().bVerboseLogging())
        {
            facCQCWCHelper().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Queuing incoming set area message."
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            m_colGUIMsgQ.RemoveAll();
        }
    }

    //
    //  To avoid extra work, we will update an existing command of the same type in the queue
    //  if possible, and only add one if needed. This way, we don't do unnecessary operations
    //  and avoid overwhelming the queue if we get behind somehow.
    //
    const tCIDLib::TBoolean bSend = m_colGUIMsgQ.bAddOrUpdate
    (
        qmsgToDo
        , [](const TGUIMsg& qmsg1, const TGUIMsg& qmsg2)
          {
            if (qmsg1.m_eType > qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstGreater;
            if (qmsg1.m_eType < qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstLess;
            return tCIDLib::ESortComps::Equal;
          }
    );
    if (bSend)
        m_wndMainFrame.SendCode(kCQCWCHelper::i4NewMsg, 0);
}


tCIDLib::TVoid TFacCQCWCHelper::QueuePlMsg(const TPlMsg& qmsgToDo)
{
    m_enctLastMsg = TTime::enctNow();

    // Should not be remotely possible, but just in case
    if (m_colPlMsgQ.bIsFull(16))
    {
        facCQCWCHelper().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWCHMsgs::midStatus_QFull
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"Player")
        );
        m_colPlMsgQ.RemoveAll();
    }

    //
    //  To avoid extra work, we will update an existing command of the same type in the queue
    //  if possible, and only add one if needed. This way, we don't do unnecessary operations
    //  and avoid overwhelming the queue if we get behind somehow.
    //
    m_colPlMsgQ.bAddOrUpdate
    (
        qmsgToDo
        , [](const TPlMsg& qmsg1, const TPlMsg& qmsg2)
          {
            if (qmsg1.m_eType > qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstGreater;
            if (qmsg1.m_eType < qmsg2.m_eType)
                return tCIDLib::ESortComps::FirstLess;
            return tCIDLib::ESortComps::Equal;
          }
    );
}



//
//  If the player is successfully created, this is called to let us register for any events
//  we want to handle.
//
tCIDLib::TVoid TFacCQCWCHelper::RegisterVLCEvents()
{
    libvlc_event_manager_t* pEvMgr = libvlc_media_player_event_manager(m_pVLCPlayer);

    // Error events
    libvlc_event_attach(pEvMgr, libvlc_MediaPlayerEndReached, &PlayerErrCB, this);
    libvlc_event_attach(pEvMgr, libvlc_MediaPlayerEncounteredError, &PlayerErrCB, this);

    // Status events
    libvlc_event_attach(pEvMgr, libvlc_MediaPlayerPlaying, &PlayerErrCB, this);
}
