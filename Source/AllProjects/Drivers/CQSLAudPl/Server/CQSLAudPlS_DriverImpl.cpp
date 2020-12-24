//
// FILE NAME: CQSLAudPlS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2007
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
#include    "CQSLAudPlS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLAudPlSDriver,TCQCStdMediaRendDrv)


// ---------------------------------------------------------------------------
//   CLASS: CQSLAudPlSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  CQSLAudPlSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLAudPlSDriver::TCQSLAudPlSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRendDrv
    (
        cqcdcToLoad
        , tCQCMedia::EPLModes::Jukebox
        , kCQCMedia::c4RendFlag_None
        , tCQCMedia::EMTFlags::Music
    )
    , m_ippnQTServer(0)
    , m_pauplTarget(0)
{
}

TCQSLAudPlSDriver::~TCQSLAudPlSDriver()
{
}


// ---------------------------------------------------------------------------
//  TCQSLAudPlSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQSLAudPlSDriver::bGetCommResource(TThread&)
{
    //
    //  If we've not created our audio engine object yet, let's try
    //  to do that.
    //
    if (!m_pauplTarget)
    {
        TCQCAudioPlayer* pauplNew = 0;
        if (m_strAudioEngine == L"DirectShow")
        {
            pauplNew = new TCQCWMPAudioPlayer(c4DefVolume());
        }
         else if (m_strAudioEngine == L"QuickTime")
        {
            pauplNew = new TCQCQTAudioPlayer
            (
                m_ippnQTServer, strMoniker(), c4DefVolume()
            );
        }
         else
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLAudPlS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"%(1) is not a known audio engine type for driver %(2)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strAudioEngine
                    , strMoniker()
                );
            }
            return kCIDLib::False;
        }

        //
        //  Put a janitor on it and initialize it. If anything goes wrong
        //  we'll delete it and so it won't be dangerous if we come back in
        //  here again to retry the init.
        //
        TJanitor<TCQCAudioPlayer> janPlayer(pauplNew);
        try
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLAudPlS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Initializing audio player object"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            pauplNew->Initialize();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLAudPlS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The %(1) audio engine failed to initialize for driver %(2)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strAudioEngine
                    , strMoniker()
                );
            }
            return kCIDLib::False;
        }

        // It worked, so orphan it out into our member
        m_pauplTarget = janPlayer.pobjOrphan();
    }
    return kCIDLib::True;
}



//
//  Called by our parent class to ak us to start up the player on the
//  pass media item. We return true or false to indicate if we id ok or not.
//
tCIDLib::TBoolean TCQSLAudPlSDriver::bPlayNewItem(const TCQCMediaPLItem& mpliNew)
{
    // Stop any existing playback
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQSLAudPlS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Driver %(1) is stopping any current playback"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
        );
    }
    m_pauplTarget->Stop();
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQSLAudPlS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Driver %(1) has stopped any current playback"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_strAudioEngine
            , strMoniker()
        );
    }

    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facCQSLAudPlS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Driver %(1) loading new file on '%(2)' engine"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , strMoniker()
            , m_strAudioEngine
        );
    }

    try
    {
        m_pauplTarget->LoadFile(mpliNew.strLocInfo(), m_strAudioDev);

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQSLAudPlS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Driver %(1) is back from loading new file, updating metadata fields"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  DO NOT reset the underlying player object. If it fails to load,
        //  it will act like the song already played. I.e. it'll clear the
        //  graph but set the end of media flag. This will cause our poll
        //  cycle next time to think it's already time to move to the next
        //  song. If we reset the player here, it would clear the end of
        //  media flag and we'd stop moving forward.
        //
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Try to load configuration data
tCIDLib::TBoolean TCQSLAudPlSDriver::bWaitConfig(TThread&)
{
    // We have no config to get
    return kCIDLib::True;
}



tCQCKit::ECommResults
TCQSLAudPlSDriver::eAdjustVolume(const tCIDLib::TBoolean bUp)
{
    // Not implemented yet, just say it worked
    return tCQCKit::ECommResults::Success;
}



//
//  This is called when the parent class wants us to get the player status.
//  We return the state and current position, and update any of our own
//  state fields.
//
//  We don't return total time, so we just leave it at the default value set
//  by our base class. He'll provide it from the metadata instead.
//
tCQCKit::ECommResults
TCQSLAudPlSDriver::eCheckPlayerStatus(  tCIDLib::TEncodedTime&      enctCurPos
                                        , tCIDLib::TEncodedTime&    enctTotal
                                        , tCQCMedia::EMediaStates&  eState
                                        , tCQCMedia::EEndStates&    eEndState
                                        , tCIDLib::TBoolean&        bMute
                                        , tCIDLib::TCard4&          c4Volume)
{
    try
    {
        // Store the new status string
        m_pauplTarget->GetStatusInfo(eState, eEndState, enctCurPos);

        bMute = m_pauplTarget->bMute();
        c4Volume = m_pauplTarget->c4Volume();
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


// Try to connect to the player
tCQCKit::ECommResults TCQSLAudPlSDriver::eConnectToDevice(TThread& thrCaller)
{
    //
    //  We don't actually have any connection, but reset the player and
    //  the playlist, store any current status to get us up to date,
    //  clear the metadata fields and update the item count and serial
    //  number fields.
    //
    //  Since we are just letting the base class do faux power management for us,
    //  go ahead and set ready status, so that we are 'powered on' to start with
    //  and will just stay that way unless explicitly asked to 'power off'.
    //
    try
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQSLAudPlS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Trying to start up the audio player engine"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        m_pauplTarget->Reset();

        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQSLAudPlS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Successfully started the audio player engine"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        StorePowerStatus(tCQCKit::EPowerStatus::Ready);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }
    return TParent::eConnectToDevice(thrCaller);
}


// The base class calls this when a transport command is received
tCQCKit::ECommResults
TCQSLAudPlSDriver::eDoTransportCmd(const tCQCMedia::ETransCmds eToSet)
{
    //
    //  These are all done via functions, so we figure out the function
    //  to send.
    //
    if ((eToSet == tCQCMedia::ETransCmds::Next)
    ||  (eToSet == tCQCMedia::ETransCmds::Previous))
    {
        // Call our parent class
        SelectNextPrev(eToSet == tCQCMedia::ETransCmds::Next);
    }
     else if (eToSet == tCQCMedia::ETransCmds::Pause)
    {
        m_pauplTarget->Pause();
    }
     else if (eToSet == tCQCMedia::ETransCmds::Play)
    {
        m_pauplTarget->Play();
    }
     else if (eToSet == tCQCMedia::ETransCmds::Stop)
    {
        m_pauplTarget->Stop();
    }
     else if ((eToSet == tCQCMedia::ETransCmds::FF)
          ||  (eToSet == tCQCMedia::ETransCmds::Rewind))
    {
        // We don't currently support these
    }
    return tCQCKit::ECommResults::Success;
}


// Initialize ourself and register fields
tCQCKit::EDrvInitRes TCQSLAudPlSDriver::eInitializeImpl()
{
    //
    //  Set appropriate poll/reconnect times. We want a pretty fast poll time
    //  so that we can keep our current status up to date.
    //
    SetPollTimes(400, 15000);

    //
    //  Process the driver prompts that are meaningful to us at this level
    //
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();

    if (!cqcdcTmp.bFindPromptValue(L"AudioDevice", L"DevName", m_strAudioDev))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQSLAudPlS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The audio device prompt was not set for driver %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  A new prompt was added to let them set the audio engine we should
    //  use. It may not be present, in which case we default it to the
    //  DirectShow engine which is what used to be the only one.
    //
    if (!cqcdcTmp.bFindPromptValue(L"AudioEngine", L"Selected", m_strAudioEngine))
        m_strAudioEngine = L"DirectShow";

    //
    //  And a new prompt was added to let them set the IP port number for
    //  the QuickTime audio server program if we are using the QuickTime
    //  engine. We can't default this one and it should always be there since
    //  they'd only get the quick time engine if the reloaded the driver
    //  to take advantage of the new stuff.
    //
    if (m_strAudioEngine == L"QuickTime")
    {
        TString strTmp;
        if (!cqcdcTmp.bFindPromptValue(L"QTPort", L"Value", strTmp))
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facCQSLAudPlS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"The QuickTime port number prompt was not provided for driver %(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strMoniker()
                );
            }
            return tCQCKit::EDrvInitRes::Failed;
        }
        m_ippnQTServer = strTmp.c4Val(tCIDLib::ERadices::Dec);
    }
     else
    {
        // We don't need this for anything but QuickTime
        m_ippnQTServer = 0;
    }

    //
    //  And now call our parent and return his return, to completely init.
    //  He'll call us back on some virtuals to do setup.
    //
    return TParent::eInitializeImpl();
}


// The base class calls this when a client sets the mute state
tCQCKit::ECommResults
TCQSLAudPlSDriver::eSetMute(const tCIDLib::TBoolean bToSet)
{
    // Just pass it on the to player
    m_pauplTarget->bMute(bToSet);
    return tCQCKit::ECommResults::Success;
}


// The base class calls this when a client sets the volume
tCQCKit::ECommResults
TCQSLAudPlSDriver::eSetVolume(const tCIDLib::TCard4 c4ToSet)
{
    // Just pass it on the to player
    m_pauplTarget->c4Volume(c4ToSet);
    return tCQCKit::ECommResults::Success;
}


// Release our player
tCIDLib::TVoid TCQSLAudPlSDriver::ReleaseCommResource()
{
    // Clean up the player object and make sure it's stopped
    try
    {
        m_pauplTarget->Terminate();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


//
//  Our parent class will call this any time it wants us to stop playing
//  and unload any current media. Any fields related to the player state
//  we are affecting will be updated in the next poll.
//
tCIDLib::TVoid TCQSLAudPlSDriver::ResetPlayer()
{
    m_pauplTarget->Reset();
}


tCIDLib::TVoid TCQSLAudPlSDriver::TerminateImpl()
{
    // We need to clean up the audio player if it got created.
    if (m_pauplTarget)
    {
        try
        {
            delete m_pauplTarget;
        }

        catch(...)
        {
        }
        m_pauplTarget = 0;
    }
}

