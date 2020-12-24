//
// FILE NAME: iTunesRendTMS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/05/2015
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
#include    "iTunesRendTMS_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TiTunesRendTMSDriver,TCQCStdMediaRendDrv)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace iTunesRend_DriverImpl
{
}


// ---------------------------------------------------------------------------
//   CLASS: iTunesRendTMSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  iTunesRendTMSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TiTunesRendTMSDriver::TiTunesRendTMSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRendDrv
    (
        cqcdcToLoad
        , tCQCMedia::EPLModes::Jukebox
        , kCQCMedia::c4RendFlag_None
        , tCQCMedia::EMTFlags::Music
    )
    , m_bMute(kCIDLib::False)
    , m_c4MuteVolume(0)
    , m_c4SerialNum(0)
    , m_c4Volume(0)
    , m_enctCurPos(0)
    , m_enctTotal(0)
    , m_eEndState(tCQCMedia::EEndStates::Ended)
    , m_ePlState(TiTPlayerIntfClientProxy::EPlStates::Stopped)
    , m_eState(tCQCMedia::EMediaStates::Stopped)
    , m_porbcTray(nullptr)
{
}

TiTunesRendTMSDriver::~TiTunesRendTMSDriver()
{
}


// ---------------------------------------------------------------------------
//  TiTunesRendTMSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TiTunesRendTMSDriver::bGetCommResource(TThread&)
{
    try
    {
        // See if we can find the object id, with a pretty short timeout
        const tCIDLib::TEncodedTime enctEnd = TTime::enctNowPlusSecs(2);
        TOrbObjId ooidTray;
        if (facCIDOrbUC().bGetNSObject(m_strBinding, ooidTray, enctEnd))
            m_porbcTray = new TiTPlayerIntfClientProxy(ooidTray, m_strBinding);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
    return (m_porbcTray != nullptr);
}


//
//  This is called when we need to start playing a new item. We get the current
//  item from the playlist manager and start it playing. The player ORB
//  interface provides a method to start playing by cookie, so we can just pull
//  the cookie out and call that method.
//
//  The repo only does music, so we always just pass the item cookie, since only
//  items are queued up.
//
tCIDLib::TBoolean
TiTunesRendTMSDriver::bPlayNewItem(const TCQCMediaPLItem& mpliNew)
{
    // If not connected, this shouldn't happen, but just in case
    if (!m_porbcTray)
        return kCIDLib::False;

    try
    {
        //
        //  Set up our states on the assumption it'll work. If not, then we'll
        //  see later that we aren't playing and transition again.
        //
        m_eEndState = tCQCMedia::EEndStates::Running;
        m_eState = tCQCMedia::EMediaStates::Playing;

        m_porbcTray->SelTrackByCookie(mpliNew.strItemCookie());
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
    return kCIDLib::True;
}


// Nonthing for us to do right now, so just pass to the parent
tCIDLib::TVoid TiTunesRendTMSDriver::ClearMetaFields()
{
    TParent::ClearMetaFields();
}


tCQCKit::ECommResults
TiTunesRendTMSDriver::eAdjustVolume(const tCIDLib::TBoolean bUp)
{
    // Shouldn't happen, but just in case
    if (!m_porbcTray)
        return tCQCKit::ECommResults::NotReady;


    return tCQCKit::ECommResults::Success;
}



//
//  This is called when the parent class wants us to get the player status.
//  We do periodic queries of various bits and store them away.
//
tCQCKit::ECommResults
TiTunesRendTMSDriver::eCheckPlayerStatus( tCIDLib::TEncodedTime&    enctCurPos
                                        , tCIDLib::TEncodedTime&    enctTotal
                                        , tCQCMedia::EMediaStates&  eState
                                        , tCQCMedia::EEndStates&    eEndState
                                        , tCIDLib::TBoolean&        bMute
                                        , tCIDLib::TCard4&          c4Volume)
{
    // Shouldn't happen, but just in case
    if (!m_porbcTray)
        return tCQCKit::ECommResults::LostCommRes;

    try
    {
        tCIDLib::TBoolean   bConn;
        TPlBase::EPlStates  ePlState;

        // Query the player status
        const tCIDLib::TBoolean bNewData = m_porbcTray->bGetPlayerState
        (
            m_c4SerialNum
            , bConn
            , m_c4Volume
            , ePlState
            , m_bMute
            , m_enctTotal
            , m_enctCurPos
            , m_strPollTmp1
            , m_strPollTmp2
            , m_strPollTmp3
        );

        // If the connection status has gond false, recycle the connection
        if (!bConn)
        {
            ResetState();
            return tCQCKit::ECommResults::LostConnection;
        }

        // Copy over the last stuff we got, even if didn't get new stuff above
        bMute = m_bMute;
        c4Volume = m_c4Volume;
        enctCurPos = m_enctCurPos;
        enctTotal = m_enctTotal;

        // See if our player state changed. If so, we want to
        if (bNewData && (ePlState != m_ePlState))
        {
            if ((m_ePlState == TPlBase::EPlStates::Playing)
            &&  (ePlState == TPlBase::EPlStates::Stopped))
            {
                // If we were playing and now are stopped do an end media
                m_eEndState = tCQCMedia::EEndStates::Ended;
            }
             else if ((m_ePlState == TPlBase::EPlStates::Stopped)
                  &&  (ePlState == TPlBase::EPlStates::Playing))
            {
                // If we were stopped and now playing, clear the end media flag
                m_eEndState = tCQCMedia::EEndStates::Running;
            }

            // Store the new player state
            m_ePlState = ePlState;
        }

        // Set our media state based on the player state
        if (m_ePlState == TPlBase::EPlStates::Playing)
            m_eState = tCQCMedia::EMediaStates::Playing;
        else if (m_ePlState == TPlBase::EPlStates::Stopped)
            m_eState = tCQCMedia::EMediaStates::Stopped;

        // And give back what we got
        eEndState = m_eEndState;
        eState = m_eState;

        // Save the current volume in case of a mute, if not zero
        if (m_c4Volume)
            m_c4MuteVolume = m_c4Volume;
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::High, CID_FILE, CID_LINE);

        // If the proxy is not connected, force a recycle of the comm resource
        if (m_porbcTray->bCheckForLostConnection())
            return tCQCKit::ECommResults::LostCommRes;

        // Else just make us go back through the connection process
        return tCQCKit::ECommResults::LostConnection;
    }
    return tCQCKit::ECommResults::Success;
}


// The base class calls this when a transport command is received
tCQCKit::ECommResults
TiTunesRendTMSDriver::eDoTransportCmd(const tCQCMedia::ETransCmds eToSet)
{
    // Shouldn't happen, but just in case
    if (!m_porbcTray)
        return tCQCKit::ECommResults::NotReady;

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
        //
        //  Translate the standard renderer trans command to the iTunes player
        //  command.
        //
        TPlBase::EPlCmds eCmd = TPlBase::EPlCmds::Count;
        switch(eToSet)
        {
            case tCQCMedia::ETransCmds::Pause :
                eCmd = TPlBase::EPlCmds::Pause;
                break;

            case tCQCMedia::ETransCmds::Play :
                eCmd = TPlBase::EPlCmds::Play;
                break;

            case tCQCMedia::ETransCmds::Stop :
                eCmd = TPlBase::EPlCmds::Stop;
                break;

            case tCQCMedia::ETransCmds::Next :
                eCmd = TPlBase::EPlCmds::Next;
                break;

            case tCQCMedia::ETransCmds::Previous :
                eCmd = TPlBase::EPlCmds::Previous;
                break;

            case tCQCMedia::ETransCmds::FF :
                eCmd = TPlBase::EPlCmds::FF;
                break;

            case tCQCMedia::ETransCmds::Rewind :
                eCmd = TPlBase::EPlCmds::Rewind;
                break;

            default :
                faciTunesRendTMS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Unknown media transport command '%(1)'"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , tCQCMedia::strXlatETransCmds(eToSet)
                );
                return tCQCKit::ECommResults::Internal;
        };

        try
        {
            m_porbcTray->DoPlayerCmd(eCmd);
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // If the proxy is disconnected, recycle
            if (m_porbcTray->bCheckForLostConnection())
                return tCQCKit::ECommResults::LostCommRes;

            // Else just indicate we got an exception
            return tCQCKit::ECommResults::Exception;
        }
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Try to connect to the player. We are already essentially connected. When
//  we pass the call on to our parent, he'll call our player status callback
//  for initial status and get the fields set up.
//
tCQCKit::ECommResults
TiTunesRendTMSDriver::eConnectToDevice(TThread& thrCaller)
{
    //
    //  We don't really have a power state, but we have to implement the power
    //  device class, so just set us to ready state initially. Unless the user
    //  does power commands on us, we'll stay this wya.
    //
    StorePowerStatus(tCQCKit::EPowerStatus::Ready);

    // Reset any per-connection stuff
    m_c4SerialNum = 0;

    // And now pass it on to our parent
    return TParent::eConnectToDevice(thrCaller);
}


//
//  We don't really have much initialization to do at this point. We just set
//  up the name server binding that the tray monitor on this machine should
//  be at.
//
tCQCKit::EDrvInitRes TiTunesRendTMSDriver::eInitializeImpl()
{
    m_strBinding = TiTPlayerIntfClientProxy::strImplBinding;
    m_strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_b);

    // Pass it on to our parent now
    return TParent::eInitializeImpl();
}


// The base class calls this when a client sets the mute state
tCQCKit::ECommResults
TiTunesRendTMSDriver::eSetMute(const tCIDLib::TBoolean bToSet)
{
    // Shouldn't happen, but just in case
    if (!m_porbcTray)
        return tCQCKit::ECommResults::NotReady;

    try
    {
        m_porbcTray->SetMute(bToSet);

        // If we unmuted, then put back the last pre-mute volume we saw
        if (!bToSet)
            m_porbcTray->SetVolume(m_c4MuteVolume);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // If the proxy is disconnected, recycle, else just indicate an exception
        if (m_porbcTray->bCheckForLostConnection())
            return tCQCKit::ECommResults::LostCommRes;
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


// The base class calls this when a client sets the volume
tCQCKit::ECommResults
TiTunesRendTMSDriver::eSetVolume(const tCIDLib::TCard4 c4ToSet)
{
    // Shouldn't happen, but just in case
    if (!m_porbcTray)
        return tCQCKit::ECommResults::NotReady;

    try
    {
        m_porbcTray->SetVolume(c4ToSet);
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() > tCQCKit::EVerboseLvls::Low)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // If the proxy is disconnected, recycle, else just indicate an exception
        if (m_porbcTray->bCheckForLostConnection())
            return tCQCKit::ECommResults::LostCommRes;
        return tCQCKit::ECommResults::Exception;
    }
    return tCQCKit::ECommResults::Success;
}


//
//  Our parent calls this to give us a chance to load up any fields of our own
//  into the list. We don't currently have any.
//
tCIDLib::TVoid
TiTunesRendTMSDriver::LoadDrvSpecificFlds(TVector<TCQCFldDef>& colFlds)
{
}


//
//  Called by our parent after fields are registered, so we can look up ours.
//  Currently we don't have any.
//
tCIDLib::TVoid
TiTunesRendTMSDriver::LookupDrvSpecificFlds(const tCIDLib::TCard4 c4ArchVersion)
{
}


tCIDLib::TVoid TiTunesRendTMSDriver::ReleaseCommResource()
{
    // Free the client proxy if we have one
    if (m_porbcTray)
    {
        try
        {
            delete m_porbcTray;
        }

        catch(TError& errToCatch)
        {
            if (eVerboseLevel() > tCQCKit::EVerboseLvls::High)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
        m_porbcTray = 0;
    }
}


//
//  Our parent class will call this any time it wants us to stop playing
//  and unload any current media.
//
tCIDLib::TVoid TiTunesRendTMSDriver::ResetPlayer()
{
    //
    //  If we have a proxy, tell the player to stop. We do it by just faking
    //  a call to our own transport command callback. That will cause us to
    //  see playback stop and we'll reset our state and times.
    //
    eDoTransportCmd(tCQCMedia::ETransCmds::Stop);
}


// Nothing to do for us currently
tCIDLib::TVoid TiTunesRendTMSDriver::TerminateImpl()
{
}


//
//  We update all the metadata fields based on a provided play list index,
//  or if it's c4MaxCard, we clear them. For now we don't have any beyond what
//  the base class provides, so we just pass it on.
//
tCIDLib::TVoid
TiTunesRendTMSDriver::UpdateMetaFields(const TCQCMediaPLItem& mpliNew)
{
    // And pass on to our parent to do the other stuff
    TParent::UpdateMetaFields(mpliNew);
}



// ---------------------------------------------------------------------------
//  TiTunesRendTMSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Reset our our latest player state info
tCIDLib::TVoid TiTunesRendTMSDriver::ResetState()
{
    m_bMute = kCIDLib::False;
    m_c4MuteVolume = 0;
    m_c4Volume = 0;
    m_enctCurPos = 0;
    m_enctTotal = 0;
    m_eState = tCQCMedia::EMediaStates::Stopped;
    m_eEndState = tCQCMedia::EEndStates::Ended;
}

