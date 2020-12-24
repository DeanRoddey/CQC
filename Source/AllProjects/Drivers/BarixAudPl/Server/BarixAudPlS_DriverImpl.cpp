//
// FILE NAME: BarixAudPlS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/07/2007
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
#include    "BarixAudPlS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TBarixAudPlSDriver,TCQCStdMediaRendDrv)



// ---------------------------------------------------------------------------
//   CLASS: BarixAudPlSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  BarixAudPlSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TBarixAudPlSDriver::TBarixAudPlSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCStdMediaRendDrv
    (
        cqcdcToLoad
        , tCQCMedia::EPLModes::Jukebox
        , kCQCMedia::c4RendFlag_None
        , tCQCMedia::EMTFlags::Music
    )
    , m_bMute(kCIDLib::False)
    , m_c4PacketsSent(0)
    , m_c4Volume(0)
    , m_colBufQ(tCIDLib::EAdoptOpts::NoAdopt, tCIDLib::EMTStates::Safe)
    , m_colCmdQ(tCIDLib::EMTStates::Safe)
    , m_sockCtrl()
    , m_strRepl_Ack(L"OK")
    , m_thrLoader
      (
        L"BarixLoaderThread_" + cqcdcToLoad.strMoniker()
        , TMemberFunc<TBarixAudPlSDriver>(this, &TBarixAudPlSDriver::eLoaderThread)
      )
    , m_thrSpool
      (
        L"BarixSpoolerThread_" + cqcdcToLoad.strMoniker()
        , TMemberFunc<TBarixAudPlSDriver>(this, &TBarixAudPlSDriver::eSpoolThread)
      )
{
}

TBarixAudPlSDriver::~TBarixAudPlSDriver()
{
}


// ---------------------------------------------------------------------------
//  TBarixAudPlSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Note that the socket we get here is for control only. Media data is done via
//  UDP and the spooler thread manages that bit. We assume that, if we lose this
//  socket that we have lost the connection.
//
tCIDLib::TBoolean TBarixAudPlSDriver::bGetCommResource(TThread&)
{
    try
    {
        // Close first if open, just in case, then open
        if (m_sockCtrl.bIsOpen())
            m_sockCtrl.Close();
        m_sockCtrl.Open(tCIDSock::ESockProtos::TCP, m_ipepCtrl.eAddrType());
    }

    catch(...)
    {
        // Not much we can do, so eat it, but do a close just in case
        m_sockCtrl.Close();
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Our parent class wants us to play a new media item
tCIDLib::TBoolean TBarixAudPlSDriver::bPlayNewItem(const TCQCMediaPLItem& mpliNew)
{
    // Get the current item and queue it up on the command queue
    try
    {
        QueueSpoolerCmd(TCQCAudioPlayer::EEvTypes::Load, mpliNew.strLocInfo());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Try to load configuration data
tCIDLib::TBoolean TBarixAudPlSDriver::bWaitConfig(TThread&)
{
    return kCIDLib::True;
}


tCQCKit::ECommResults
TBarixAudPlSDriver::eAdjustVolume(const tCIDLib::TBoolean bUp)
{
    // Queue up a volume adjust command to the spooler thread
    try
    {
        SendCmd(kCIDLib::chLatin_c, bUp ? 19 : 20);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);

        if (!m_sockCtrl.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TBarixAudPlSDriver::eCheckPlayerStatus( tCIDLib::TEncodedTime&      enctCurPos
                                        , tCIDLib::TEncodedTime&    enctTotal
                                        , tCQCMedia::EMediaStates&  eState
                                        , tCQCMedia::EEndStates&    eEndState
                                        , tCIDLib::TBoolean&        bMute
                                        , tCIDLib::TCard4&          c4Volume)
{
    // Lock while we get out the data
    tCIDLib::TCard4 c4PacketCnt = 0;
    {
        TCritSecLocker crslSync(&m_crsSync);

        eState = m_eMediaState;
        eEndState = m_eEndState;
        c4PacketCnt = m_c4PacketsSent;

        bMute = m_bMute;
        c4Volume = m_c4Volume;
    }

    //
    //  Calc the current time based on packets sent and the time per packet. We
    //  don't update the total time and allow the time set by the metadata to be
    //  used.
    //
    static const tCIDLib::TEncodedTime enctPerPacket(kBarixAudPlS::c4RoundDelay * 10);
    enctCurPos = c4PacketCnt * enctPerPacket;

    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TBarixAudPlSDriver::eConnectToDevice(TThread& thrThis)
{
    // If not connected, then try to connect
    if (!m_sockCtrl.bIsConnected())
    {
        try
        {
            // Try to connect to the remote end point
            m_sockCtrl.Connect(m_ipepCtrl);

            // That worked so set the volume to the default
            const tCIDLib::TCard4 c4InitVol = c4DefVolume();
            SendCmd(kCIDLib::chLatin_V, c4InitVol);

            // Remember this as our volume and assume unmuted
            m_c4Volume = c4InitVol;
            m_bMute = kCIDLib::False;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
            return tCQCKit::ECommResults::LostConnection;
        }

        catch(...)
        {
            return tCQCKit::ECommResults::LostConnection;
        }
    }

    // It worked so pass it on to our parent
    return TParent:: eConnectToDevice(thrThis);
}


tCQCKit::ECommResults
TBarixAudPlSDriver::eDoTransportCmd(const tCQCMedia::ETransCmds eToSet)
{
    if ((eToSet == tCQCMedia::ETransCmds::Next)
    ||  (eToSet == tCQCMedia::ETransCmds::Previous))
    {
        // Let our parent handle the default response
        SelectNextPrev(eToSet == tCQCMedia::ETransCmds::Next);
        return tCQCKit::ECommResults::Success;
    }

    // Otherwise we just queue up commands to the spooler thread
    try
    {
        if (eToSet == tCQCMedia::ETransCmds::Pause)
        {
            QueueSpoolerCmd(TCQCAudioPlayer::EEvTypes::Pause);
        }
         else if (eToSet == tCQCMedia::ETransCmds::Play)
        {
            QueueSpoolerCmd(TCQCAudioPlayer::EEvTypes::Play);
        }
         else if (eToSet == tCQCMedia::ETransCmds::Stop)
        {
            QueueSpoolerCmd(TCQCAudioPlayer::EEvTypes::Stop);
        }
         else if ((eToSet == tCQCMedia::ETransCmds::FF)
              ||  (eToSet == tCQCMedia::ETransCmds::Rewind))
        {
            // These we don't support
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);
    }
    return tCQCKit::ECommResults::Success;
}


// Initialize ourself and register fields
tCQCKit::EDrvInitRes TBarixAudPlSDriver::eInitializeImpl()
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

    //
    //  Get out the two end points we use. We get the streaming end point,
    //  and assume the control end point is the next sequential port up.
    //
    m_ipepStream = m_conncfgSock.ipepConn();
    m_ipepCtrl = TIPEndPoint
    (
        m_ipepStream.strAsText()
        , m_ipepStream.ippnThis() + 1
        , tCIDSock::EAddrTypes::Unspec
    );

    // Set appropriate poll/reconnect times
    SetPollTimes(1000, 15000);

    //
    //  All went well, so start up the the spooler and loader threads if
    //  they are not already running.
    //
    //  These threads don't access any fields, so it won't matter if we
    //  get called twice and reset the fields. And they can't actually be
    //  doing any media spooling since no one could queue anything up until
    //  we come up online.
    //
    //  !!!But, be careful that this driver isn't changed such that we recycle
    //  any members that these threads might look at while just idling!
    //
    if (!m_thrLoader.bIsRunning())
        m_thrLoader.Start();
    if (!m_thrSpool.bIsRunning())
        m_thrSpool.Start();

    // Call our parent class to do his init
    return TParent::eInitializeImpl();
}


//
//  There is no discrete mute command, so we fake it with the volume.
//
tCQCKit::ECommResults TBarixAudPlSDriver::eSetMute(const tCIDLib::TBoolean bToSet)
{
    // Send the device a mute command
    try
    {
        // If the requested mute state is not the same as the current, toggle it
        if (bToSet && !m_bMute)
        {
            // We need to go into mute state, so set zero volume
            SendCmd(kCIDLib::chLatin_V, 0);
            m_bMute = kCIDLib::True;
        }
         else if (!bToSet && m_bMute)
        {
            //
            //  We need to leave mute state, so set the volume back to whatever it
            //  was before.
            //
            SendCmd(kCIDLib::chLatin_V, m_c4Volume);
            m_bMute = kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Low);

        if (!m_sockCtrl.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults
TBarixAudPlSDriver::eSetVolume(const tCIDLib::TCard4 c4ToSet)
{
    // It supports percent based volume now, so we can directly send the value
    try
    {
        SendCmd(kCIDLib::chLatin_V, c4ToSet);

        // Store this as the last volume, and exist mute state if in it
        m_bMute = kCIDLib::False;
        m_c4Volume = c4ToSet;
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        if (!m_sockCtrl.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


// Handle power state change callbacks from the base renderer driver
tCQCKit::ECommResults
TBarixAudPlSDriver::eStartPowerTrans(const tCIDLib::TBoolean bNewState)
{
    try
    {
        SendCmd(kCIDLib::chLatin_c, bNewState ? 102 : 101);

        tCQCKit::EPowerStatus eStatus;
        if (bNewState)
            eStatus = tCQCKit::EPowerStatus::Ready;
        else
            eStatus = tCQCKit::EPowerStatus::Off;
        StorePowerStatus(eStatus);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);

        if (!m_sockCtrl.bIsConnected())
            return tCQCKit::ECommResults::LostCommRes;
    }
    return tCQCKit::ECommResults::Success;
}


// Just let our control socket go
tCIDLib::TVoid TBarixAudPlSDriver::ReleaseCommResource()
{
    // First do a clean shutdown
    try
    {
        m_sockCtrl.c4Shutdown();
    }

    catch(...) {}

    // And close it
    try
    {
        m_sockCtrl.Close();
    }

    catch(...) {}
}


// We just queue up a reset command on the spooler thread
tCIDLib::TVoid TBarixAudPlSDriver::ResetPlayer()
{
    QueueSpoolerCmd(TCQCAudioPlayer::EEvTypes::Reset);
}


// The control socket will be cleaned up. We need to shut down our background threads
tCIDLib::TVoid TBarixAudPlSDriver::TerminateImpl()
{
    // Stop the spooler thread
    try
    {
        m_thrSpool.ReqShutdownSync(10000);
        m_thrSpool.eWaitForDeath(4000);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Stop the loader thread
    try
    {
        m_thrLoader.ReqShutdownSync(10000);
        m_thrLoader.eWaitForDeath(4000);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}

