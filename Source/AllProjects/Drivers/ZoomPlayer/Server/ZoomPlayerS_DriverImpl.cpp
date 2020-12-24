//
// FILE NAME: ZoomPlayerS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2005
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
#include    "ZoomPlayerS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZoomPlayerSDriver,TCQCStdMediaRendDrv)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZoomPlayer_DriverImpl
{
    const tCIDLib::TCh* const pszFldAudioTrack(L"AudioTrack");
    const tCIDLib::TCh* const pszFldNavigation(L"Navigation");
    const tCIDLib::TCh* const pszFldPlayerVer(L"PlayerVer");
    const tCIDLib::TCh* const pszFldPlayURL(L"PlayURL");
    const tCIDLib::TCh* const pszFldScreenMode(L"ScreenMode");
    const tCIDLib::TCh* const pszFldSetAR(L"SetAR");
    const tCIDLib::TCh* const pszFldSetLocInfoPat(L"SetLocInfoPattern");
    const tCIDLib::TCh* const pszFldToggleNext(L"ToggleNext");
}


// ---------------------------------------------------------------------------
//   CLASS: ZoomPlayerSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  ZoomPlayerSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TZoomPlayerSDriver::TZoomPlayerSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRendDrv
    (
        cqcdcToLoad
        , tCQCMedia::EPLModes::Jukebox
        , kCQCMedia::c4RendFlag_None
        , tCQCMedia::EMTFlags::MultiMedia
    )
    , m_c4FldId_AudioTrack(kCIDLib::c4MaxCard)
    , m_c4FldId_Navigation(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayURL(kCIDLib::c4MaxCard)
    , m_c4FldId_PlayerVer(kCIDLib::c4MaxCard)
    , m_c4FldId_SetAR(kCIDLib::c4MaxCard)
    , m_c4FldId_SetLocInfoPat(kCIDLib::c4MaxCard)
    , m_c4FldId_ScreenMode(kCIDLib::c4MaxCard)
    , m_c4FldId_ToggleNext(kCIDLib::c4MaxCard)
    , m_enctCurPos(0)
    , m_enctCurTotal(0)
    , m_eEndState(tCQCMedia::EEndStates::Ended)
    , m_eState(tCQCMedia::EMediaStates::Stopped)
    , m_eMediaType(tCQCMedia::EMediaTypes::Count)
    , m_mbufIn(8192UL)
    , m_sockConn()
    , m_strIn(8192UL)
{
}

TZoomPlayerSDriver::~TZoomPlayerSDriver()
{
}


// ---------------------------------------------------------------------------
//  TZoomPlayerSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TZoomPlayerSDriver::bGetCommResource(TThread&)
{
    try
    {
        // If open, just in case, close
        if (m_sockConn.bIsOpen())
            m_sockConn.Close();

        // Just open, we don't connect yet
        m_sockConn.Open
        (
            tCIDSock::ESockProtos::TCP, m_conncfgSock.ipepConn().eAddrType()
        );
    }

    catch(...)
    {
        // Not much we can do, so eat it, but do a close just in case
        m_sockConn.Close();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  This is called when we need to start playing a new item. We get the
//  current item from the playlist manager and start it playing.
//
tCIDLib::TBoolean
TZoomPlayerSDriver::bPlayNewItem(const TCQCMediaPLItem& mpliNew)
{
    try
    {
        // Remove any current playlist from Zoom
        SendCmd(kZoomPlayerS::c4Cmd_ClearPlayList);
        ProcessMsgs();

        //
        //  Add this file to Zoom's list, and tell it to start playing. We
        //  have to give it a bit of time after the clear playlist above.
        //  If there's a location info pattern we have to use that.
        //
        TThread::Sleep(500);
        TString strLocInfo;
        if (m_strLocInfoPattern.bIsEmpty())
        {
            strLocInfo = mpliNew.strLocInfo();
        }
         else
        {
            strLocInfo = m_strLocInfoPattern;
            strLocInfo.eReplaceToken(mpliNew.strLocInfo(), kCIDLib::chLatin_L);
        }

        SendCmd(kZoomPlayerS::c4Cmd_AddToPlayList, strLocInfo);
        ProcessMsgs();
        SendCmd(kZoomPlayerS::c4Cmd_SetPlayListInd, L"0");
        ProcessMsgs();
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;

    }

    // It worked so make sure that we reset our end state
    m_eEndState = tCQCMedia::EEndStates::Running;
    return kCIDLib::True;
}


//
//  This is called when we don't have a new item to play so we need to clean
//  out the metadata.
//
tCIDLib::TVoid TZoomPlayerSDriver::ClearMetaFields()
{
    // Clear some fields of our own
    bStoreStringFld(m_c4FldId_AudioTrack, TString::strEmpty(), kCIDLib::True);

    // Reset the media type
    m_eMediaType = tCQCMedia::EMediaTypes::Count;

    // And then pass to our parent for the rest of the work
    TParent::ClearMetaFields();
}


tCQCKit::ECommResults
TZoomPlayerSDriver::eAdjustVolume(const tCIDLib::TBoolean bUp)
{
    return tCQCKit::ECommResults::Success;
}



//
//  This is called when the parent class wants us to get the player status.
//  We do periodic queries of various bits and store them away.
//
tCQCKit::ECommResults
TZoomPlayerSDriver::eCheckPlayerStatus( tCIDLib::TEncodedTime&      enctCurPos
                                        , tCIDLib::TEncodedTime&    enctTotal
                                        , tCQCMedia::EMediaStates&  eState
                                        , tCQCMedia::EEndStates&    eEndState
                                        , tCIDLib::TBoolean&        bMute
                                        , tCIDLib::TCard4&          c4Volume)
{
    try
    {
        //
        //  Process any incoming messages. We just loop with a very short
        //  timeout, so we just get what's waiting. This will also catch the
        //  player going away since we'll get a dropped connection error when
        //  we do a read.
        //
        ProcessMsgs();

        // Give back the latest info we've stored
        bMute = m_bMute;
        c4Volume = m_c4Volume;
        enctCurPos = m_enctCurPos;
        enctTotal = m_enctCurTotal;
        eEndState = m_eEndState;
        eState = m_eState;
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High, CID_FILE, CID_LINE);

        // If we lost connection, then assume the worst
        if (!m_sockConn.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;

        return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}



// The base class calls this when a transport command is received
tCQCKit::ECommResults
TZoomPlayerSDriver::eDoTransportCmd(const tCQCMedia::ETransCmds eToSet)
{
    //
    //  The next/prev ones we pass to our parent who controls the play
    //  list. The others we pass on to Zoom.
    //
    if ((eToSet == tCQCMedia::ETransCmds::Next)
    ||  (eToSet == tCQCMedia::ETransCmds::Previous))
    {
        SelectNextPrev(eToSet == tCQCMedia::ETransCmds::Next);
    }
     else
    {
        const TString* pstrFunc = 0;

        if (eToSet == tCQCMedia::ETransCmds::FF)
            pstrFunc = &kZoomPlayerS::strFunc_FF;
        else if (eToSet == tCQCMedia::ETransCmds::Next)
            pstrFunc = &kZoomPlayerS::strFunc_Next;
        else if (eToSet == tCQCMedia::ETransCmds::Pause)
            pstrFunc = &kZoomPlayerS::strFunc_Pause;
        else if (eToSet == tCQCMedia::ETransCmds::Play)
            pstrFunc = &kZoomPlayerS::strFunc_Play;
        else if (eToSet == tCQCMedia::ETransCmds::Previous)
            pstrFunc = &kZoomPlayerS::strFunc_Prev;
        else if (eToSet == tCQCMedia::ETransCmds::Rewind)
            pstrFunc = &kZoomPlayerS::strFunc_Rewind;
        else if (eToSet == tCQCMedia::ETransCmds::Stop)
            pstrFunc = &kZoomPlayerS::strFunc_Stop;

        if (pstrFunc)
            SendFunc(*pstrFunc);
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Try to connect to the player
//
tCQCKit::ECommResults
TZoomPlayerSDriver::eConnectToDevice(TThread& thrCaller)
{
    // If connected, then disconnect first, then try to connect
    try
    {
        if (m_sockConn.bIsConnected())
            m_sockConn.Close();

        // Try to connect to the remote end point
        m_sockConn.Connect(m_conncfgSock.ipepConn());
    }

    catch(...)
    {
        return tCQCKit::ECommResults::LostConnection;
    }

    // Send a couple queries to get fields updated
    SendCmd(kZoomPlayerS::c4Query_AudioTrackNames);
    SendCmd(kZoomPlayerS::c4Query_PlayState);
    SendCmd(kZoomPlayerS::c4Query_Volume);
    ProcessMsgs();

    // Make sure the play list is empty
    SendCmd(kZoomPlayerS::c4Cmd_ClearPlayList);
    ProcessMsgs();

    // Reset any current player stuff stuff we keep around
    m_bMute = kCIDLib::False;
    m_c4Volume = 0;
    m_enctCurPos = 0;
    m_enctCurTotal = 0;
    m_eEndState = tCQCMedia::EEndStates::Ended;
    m_eState = tCQCMedia::EMediaStates::Stopped;

    //
    //  Since the base class is just doing faux power management for us, go ahead
    //  and start us in ready mode, for convenience. We'll stay that way unless
    //  a clien explicitly powers us 'off'.
    //
    StorePowerStatus(tCQCKit::EPowerStatus::Ready);

    // And now pass it on to our parent
    return TParent::eConnectToDevice(thrCaller);
}


// A floating point field changed
tCQCKit::ECommResults
TZoomPlayerSDriver::eFloatFldValChanged(const  TString&             strFldName
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TFloat8    f8NewValue)
{
    if (c4FldId == m_c4FldId_SetAR)
    {
        //
        //  Conver the ratio to the format that ZP wants. So we need to
        //  break out the two parts of the floating point field.
        //
        tCIDLib::TCard4 c4AR = 100;
        c4AR <<= 16;
        c4AR |= tCIDLib::TCard4(f8NewValue * 100.0);

        TString strParm;
        strParm.SetFormatted(c4AR);
        SendCmd(kZoomPlayerS::c4Cmd_SetAR, strParm);
    }
     else
    {
        return eFloatFldValChanged(strFldName, c4FldId, f8NewValue);
    }
    return tCQCKit::ECommResults::Success;
}



// Initialize ourself and register fields
tCQCKit::EDrvInitRes TZoomPlayerSDriver::eInitializeImpl()
{
    //
    //  Make sure that we were configured for a socket connection. Otherwise,
    //  its a bad configuration.
    //
    const TCQCDriverObjCfg& cqcdcOurs = cqcdcThis();
    if (cqcdcOurs.conncfgReal().clsIsA() != TCQCIPConnCfg::clsThis())
    {
        facCQCKit().LogMsg
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
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  Its the right type so do the down cast and get the data out of it
    //  that we need.
    //
    m_conncfgSock = static_cast<const TCQCIPConnCfg&>(cqcdcOurs.conncfgReal());

    //
    //  Set appropriate poll/reconnect times. We want a fast poll time, though
    //  we don't really poll. We just want to react quickly to async messages
    //  from the player.
    //
    SetPollTimes(100, 3000);

    // Pass it on to our parent now
    return TParent::eInitializeImpl();
}


// The base class calls this when a client sets the mute state
tCQCKit::ECommResults
TZoomPlayerSDriver::eSetMute(const tCIDLib::TBoolean bToSet)
{
    // Not supported yet
    return tCQCKit::ECommResults::Success;
}


// The base class calls this when a client sets the volume
tCQCKit::ECommResults
TZoomPlayerSDriver::eSetVolume(const tCIDLib::TCard4 c4ToSet)
{
    SendExFunc(kZoomPlayerS::strExFunc_Volume, c4ToSet);
    return tCQCKit::ECommResults::Success;
}


//
//  A string field changed
//
tCQCKit::ECommResults
TZoomPlayerSDriver::eStringFldValChanged(const  TString&        strFldName
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldId_PlayURL)
    {
        DoPlayURL(strNewValue);
    }
     else if (c4FldId == m_c4FldId_ScreenMode)
    {
        // Special case the full screen one, which is an extended function
        if (strNewValue == L"Full Screen")
        {
            SendExFunc(kZoomPlayerS::strExFunc_Interface, 8);
        }
         else
        {
            const TString* pstrFunc = 0;
            if (strNewValue == L"Maximize")
                pstrFunc = &kZoomPlayerS::strFunc_Maximize;
            else if (strNewValue == L"Minimize")
                pstrFunc = &kZoomPlayerS::strFunc_Minimize;
            else if (strNewValue == L"To Front")
                pstrFunc = &kZoomPlayerS::strFunc_ToFront;
            else if (strNewValue == L"Toggle FS")
                pstrFunc = &kZoomPlayerS::strFunc_FullScreen;
            else
            {
                IncUnknownWriteCounter();
                return tCQCKit::ECommResults::BadValue;
            }
            SendFunc(*pstrFunc);
        }
    }
     else if (c4FldId == m_c4FldId_SetLocInfoPat)
    {
        // Make sure it has the require token if not empty
        if (!strNewValue.bIsEmpty() && !strNewValue.bTokenExists(kCIDLib::chLatin_L))
            return tCQCKit::ECommResults::BadValue;

        // Looks ok so store it for later use
        m_strLocInfoPattern = strNewValue;
    }
     else if (c4FldId == m_c4FldId_ToggleNext)
    {
        const TString* pstrFunc = 0;
        if (strNewValue == L"Angle")
            pstrFunc = &kZoomPlayerS::strFunc_NextAngle;
        else if (strNewValue == L"Audio")
            pstrFunc = &kZoomPlayerS::strFunc_NextAudio;
        else if (strNewValue == L"Subtitle")
            pstrFunc = &kZoomPlayerS::strFunc_NextSubtitle;
        else
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::BadValue;
        }
        SendFunc(*pstrFunc);
    }
     else if (c4FldId == m_c4FldId_Navigation)
    {
        const TString* pstrFunc = 0;
        if (strNewValue == L"Down")
            pstrFunc = &kZoomPlayerS::strFunc_Down;
        else if (strNewValue == L"Enter")
            pstrFunc = &kZoomPlayerS::strFunc_Enter;
        else if (strNewValue == L"Eject")
            pstrFunc = &kZoomPlayerS::strFunc_Eject;
        else if (strNewValue == L"Exit")
            pstrFunc = &kZoomPlayerS::strFunc_Exit;
        else if (strNewValue == L"Left")
            pstrFunc = &kZoomPlayerS::strFunc_Left;
        else if (strNewValue == L"MediaNext")
            pstrFunc = &kZoomPlayerS::strFunc_MediaNext;
        else if (strNewValue == L"MediaPrev")
            pstrFunc = &kZoomPlayerS::strFunc_MediaPrev;
        else if (strNewValue == L"Right")
            pstrFunc = &kZoomPlayerS::strFunc_Right;
        else if (strNewValue == L"RootMenu")
            pstrFunc = &kZoomPlayerS::strFunc_RootMenu;
        else if (strNewValue == L"TitleMenu")
            pstrFunc = &kZoomPlayerS::strFunc_TitleMenu;
        else if (strNewValue == L"Up")
            pstrFunc = &kZoomPlayerS::strFunc_Up;

        if (!pstrFunc)
        {
            IncUnknownWriteCounter();
            return tCQCKit::ECommResults::BadValue;
        }
        SendFunc(*pstrFunc);
    }
     else
    {
        // Pass it on to our parent
        return TParent::eStringFldValChanged(strFldName, c4FldId, strNewValue);
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Our parent calls this to give us a chance to load up any fields of
//  our own into the list.
//
tCIDLib::TVoid
TZoomPlayerSDriver::LoadDrvSpecificFlds(TVector<TCQCFldDef>& colFlds)
{
    TCQCFldDef flddNew;
    TString strLims;

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldAudioTrack
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldPlayerVer
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldToggleNext
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , L"Enum: Angle, Audio, Subtitle"
    );
    colFlds.objAdd(flddNew);


    // Do the screen mode field
    strLims = L"Enum: ";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZoomPlayerS::c4MaxScrMode; c4Index++)
    {
        strLims.Append(kZoomPlayerS::apszScrModeMap[c4Index]);
        strLims.Append(L", ");
    }
    strLims.CapAt(strLims.c4Length() - 2);

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldScreenMode
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , strLims
    );
    colFlds.objAdd(flddNew);


    // Do the navigation field
    strLims = L"Enum: ";
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kZoomPlayerS::c4MaxNav; c4Index++)
    {
        strLims.Append(kZoomPlayerS::apszNavMap[c4Index]);
        strLims.Append(L", ");
    }
    strLims.CapAt(strLims.c4Length() - 2);
    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldNavigation
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , strLims
    );
    colFlds.objAdd(flddNew);


    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldPlayURL
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldSetAR
        , tCQCKit::EFldTypes::Float
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        ZoomPlayer_DriverImpl::pszFldSetLocInfoPat
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);
}


// Called by our parent after fields are registered, so we can look up ours
tCIDLib::TVoid
TZoomPlayerSDriver::LookupDrvSpecificFlds(const tCIDLib::TCard4 c4ArchVersion)
{
    m_c4FldId_AudioTrack = pflddFind(ZoomPlayer_DriverImpl::pszFldAudioTrack, kCIDLib::True)->c4Id();
    m_c4FldId_Navigation = pflddFind(ZoomPlayer_DriverImpl::pszFldNavigation, kCIDLib::True)->c4Id();
    m_c4FldId_PlayURL = pflddFind(ZoomPlayer_DriverImpl::pszFldPlayURL, kCIDLib::True)->c4Id();
    m_c4FldId_PlayerVer = pflddFind(ZoomPlayer_DriverImpl::pszFldPlayerVer, kCIDLib::True)->c4Id();
    m_c4FldId_SetAR = pflddFind(ZoomPlayer_DriverImpl::pszFldSetAR, kCIDLib::True)->c4Id();
    m_c4FldId_SetLocInfoPat = pflddFind(ZoomPlayer_DriverImpl::pszFldSetLocInfoPat, kCIDLib::True)->c4Id();
    m_c4FldId_ScreenMode = pflddFind(ZoomPlayer_DriverImpl::pszFldScreenMode, kCIDLib::True)->c4Id();
    m_c4FldId_ToggleNext = pflddFind(ZoomPlayer_DriverImpl::pszFldToggleNext, kCIDLib::True)->c4Id();
}


tCIDLib::TVoid TZoomPlayerSDriver::ReleaseCommResource()
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


//
//  Our parent class will call this any time it wants us to stop playing
//  and unload any current media.
//
tCIDLib::TVoid TZoomPlayerSDriver::ResetPlayer()
{
    // Tell Zoom to clear its list
    SendCmd(kZoomPlayerS::c4Cmd_ClearPlayList);
    ProcessMsgs();
}


tCIDLib::TVoid TZoomPlayerSDriver::TerminateImpl()
{
}


//
//  We update all the metadata fields based on a provided play list index,
//  or if it's c4MaxCard, we clear them.
//
tCIDLib::TVoid
TZoomPlayerSDriver::UpdateMetaFields(const TCQCMediaPLItem& mpliNew)
{
    // Store the media type of this one
    m_eMediaType = mpliNew.eMediaType();

    // And pass on to our parent to do the other stuff
    TParent::UpdateMetaFields(mpliNew);
}



// ---------------------------------------------------------------------------
//  TZoomPlayerSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Pulls out one field from a formatted time value from Zoom. We pull in
//  any digits and then move past the colon (if not at the end) before
//  returning. So it should be one or two numbers, followed by either the
//  end of the string or a colon.
//
tCIDLib::TCard4
TZoomPlayerSDriver::c4ExtractTimeFld(const  TString&            strTime
                                    ,       tCIDLib::TCard4&    c4Ofs) const
{
    const tCIDLib::TCard4 c4Len = strTime.c4Length();
    tCIDLib::TCard4 c4Val = 0;
    tCIDLib::TCh chCur;

    // If not already at the end
    if (c4Ofs < c4Len)
    {
        // Check the first one. Can't be a color and must be a digit
        chCur = strTime[c4Ofs++];
        if ((chCur == kCIDLib::chColon) || !TRawStr::bIsDigit(chCur))
            return 0;

        // It's ok, put into the accum
        c4Val = tCIDLib::TCard4(chCur - 0x30);

        //
        //  Do the second digit, if any. If a colon, we are done. If not a
        //  digit otherwise, then an error, return zero.
        //
        if (c4Ofs < c4Len)
        {
            chCur = strTime[c4Ofs++];
            if (chCur == kCIDLib::chColon)
                return c4Val;
            if (!TRawStr::bIsDigit(chCur))
                return 0;

            // Looks ok, add this guy to the accumulator
            c4Val *= 10;
            c4Val += tCIDLib::TCard4(chCur - 0x30);
        }
    }

    //
    //  If not at the end, and the next char isn't a colon, it's an error.
    //  If it is a colon, skip it.
    //
    if (c4Ofs < c4Len)
    {
        if (strTime[c4Ofs] != kCIDLib::chColon)
            return 0;
        c4Ofs++;
    }
    return c4Val;
}


//
//  This is called when the user writes to the PlayURL field. We clean out
//  the metadata so that we don't look like we are playing a movie or CD,
//  and start up the player on the URL.
//
tCIDLib::TVoid TZoomPlayerSDriver::DoPlayURL(const TString& strURL)
{
    ClearMetaFields();

    // Make sure any existing playback is ended
    SendCmd(kZoomPlayerS::c4Cmd_ClearPlayList);
    ProcessMsgs();

    // And tell ZP to start playing the URL
    SendCmd(kZoomPlayerS::c4Cmd_PlayFile, strURL);
}


//
//  This takes a time of the sort we get from Zoom (in the form HH:MM:SS) and
//  converts it to a time stamp. We can get more than one in a string, so
//  this guy lets them indicate where in the string it is. We may not gat the
//  HH or MM if the media is to short to require them.
//
tCIDLib::TEncodedTime
TZoomPlayerSDriver::enctConvertTime(const TString& strTime) const
{
    const tCIDLib::TCard4 c4Len = strTime.c4Length();
    if (!c4Len)
        return 0;

    tCIDLib::TCard4 c4Hours = 0;
    tCIDLib::TCard4 c4Mins = 0;
    tCIDLib::TCard4 c4Secs = 0;
    tCIDLib::TCard4 c4Ofs = 0;

    // If longer than 6, then we have hours
    if (c4Len > 6)
        c4Hours = c4ExtractTimeFld(strTime, c4Ofs);

    // If longer than 3, we have minutes
    if (c4Len > 3)
        c4Mins = c4ExtractTimeFld(strTime, c4Ofs);

    // And get the seconds
    c4Secs = c4ExtractTimeFld(strTime, c4Ofs);

    return tCIDLib::TEncodedTime
    (
       (c4Hours * kCIDLib::enctOneHour)
       + (c4Mins * kCIDLib::enctOneMinute)
       + (c4Secs * kCIDLib::enctOneSecond)
    );
}


// Processes incoming messages and stores away field data as required
tCIDLib::TVoid
TZoomPlayerSDriver::ProcessMsg( const   tCIDLib::TCard4 c4CmdId
                                , const TString&        strParms)
{
    tCIDLib::TBoolean   bOk;
    tCIDLib::TCard4     c4TmpVal;

    switch(c4CmdId)
    {
        case kZoomPlayerS::c4Cmd_AppName :
        {
            // We don't use this currently
            break;
        }

        case kZoomPlayerS::c4Cmd_AppVersion :
        {
            // App version
            bStoreStringFld(m_c4FldId_PlayerVer, strParms, kCIDLib::True);
            break;
        }

        case kZoomPlayerS::c4Cmd_ARChanged :
        {
            //
            //  AR change when playing a DVD
            //
            //  0  - Unknown
            //  1  - Fullscreen
            //  2  - Letterbox
            //  3  - Anamorphic
            //
            break;
        }

        case kZoomPlayerS::c4Cmd_AudioTrackName :
        {
            // Cut out the first 4 chars, which are a padded number
            strParms.CopyOutSubStr(m_strTmp, 4);
            bStoreStringFld(m_c4FldId_AudioTrack, m_strTmp, kCIDLib::True);
            break;
        }

        case kZoomPlayerS::c4Cmd_EndOfTrack :
        {
            // Set the end state to ended
            m_eEndState = tCQCMedia::EEndStates::Ended;
            break;
        }

        case kZoomPlayerS::c4Cmd_PlayListIndChange :
        {
            // We aren't currently using this
            break;
        }

        case kZoomPlayerS::c4Cmd_PlayModeChanged :
        {
            //
            //  Current play mode
            //  0  - DVD Mode
            //  1  - Media Mode
            //  2  - Audio Mode
            //
            m_ePlayMode = tZoomPlayerS::EPlayModes(strParms.c4Val(tCIDLib::ERadices::Dec));
            break;
        }

        case kZoomPlayerS::c4Cmd_StateChange :
        {
            //
            //  State change
            //  0  - Closed
            //  1  - Stopped (doesn't apply to DVD, DVD Stop = Closed)
            //  2  - Paused
            //  3  - Playing
            //
            tCIDLib::TBoolean   bOk;
            c4TmpVal = TRawStr::c4AsBinary(strParms.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
            if (bOk)
            {
                if ((c4TmpVal == 0) || (c4TmpVal == 1))
                    m_eState = tCQCMedia::EMediaStates::Stopped;
                else if (c4TmpVal == 2)
                    m_eState = tCQCMedia::EMediaStates::Paused;
                else if (c4TmpVal == 3)
                    m_eState = tCQCMedia::EMediaStates::Playing;
            }
            break;
        }

        case kZoomPlayerS::c4Cmd_TimeLine :
        {
            StoreTimeline(strParms, ETime_Both);
            break;
        }

        case kZoomPlayerS::c4Cmd_TrackDuration :
        {
            StoreTimeline(strParms, ETime_Total);
            break;
        }

        case kZoomPlayerS::c4Cmd_TrackPosition :
        {
            StoreTimeline(strParms, ETime_Current);
            break;
        }

        case kZoomPlayerS::c4Cmd_Volume :
        {
            c4TmpVal = TRawStr::c4AsBinary(strParms.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
            if (bOk)
                m_c4Volume = c4TmpVal;
            else
                m_c4Volume = kCIDLib::c4MaxCard;
            break;
        }

        default :
            // Don't care about this one
            break;
    };
}


// Just processes any messages that happen to be pending
tCIDLib::TVoid TZoomPlayerSDriver::ProcessMsgs()
{
    tCIDLib::TCard4 c4Cmd;
    while (bGetMsg(c4Cmd, m_strPoll, 10))
        ProcessMsg(c4Cmd, m_strPoll);
}


