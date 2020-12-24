//
// FILE NAME: CQCMedia_QTAudioPlayer.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/10/2007
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
//  This file implements a wrapper around the Quick Time 7 SDK, for
//  doing simple headless audio playback.
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
#include    "CQCMedia_.hpp"


// ---------------------------------------------------------------------------
//  Magic RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCQTAudioPlayer,TCQCAudioPlayer)



// ---------------------------------------------------------------------------
//  Local types and data, continued
// ---------------------------------------------------------------------------
namespace CQCMedia_QTWMPAudioPlayer
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  A unique name object to give unique names to our threads that we spin
        //  up for each new instance.
        // -----------------------------------------------------------------------
        TUniqueName unamThreads(L"QTAudioPlayerThread_%(1)", 1);


        // -----------------------------------------------------------------------
        //  We will wait for up to this long for a queued command to be processed
        //  by the player thread.
        // -----------------------------------------------------------------------
        #if CID_DEBUG_ON
        constexpr tCIDLib::TCard4   c4WaitCmd = 600000;
        #else
        constexpr tCIDLib::TCard4   c4WaitCmd = 6000;
        #endif
    }
}



// ---------------------------------------------------------------------------
//  Local helper functions
// ---------------------------------------------------------------------------




// ---------------------------------------------------------------------------
//  CLASS: TCQCQTAudioPlayer
// PREFIX: cwmp
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCQTAudioPlayer: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCQTAudioPlayer::TCQCQTAudioPlayer(const  tCIDLib::TIPPortNum ippnServer
                                    , const TString&            strUniqueName
                                    , const tCIDLib::TCard4     c4DefVolume) :

    TCQCAudioPlayer(c4DefVolume)
    , m_ippnServer(ippnServer)
    , m_pevSync(nullptr)
    , m_pmtxSync(nullptr)
    , m_strUniqueName(strUniqueName)
{
}

TCQCQTAudioPlayer::~TCQCQTAudioPlayer()
{
    // Should have been done in Terminate, but just in case
    try
    {
        if (m_pevSync)
        {
            delete m_pevSync;
            m_pevSync = nullptr;
        }

        if (m_pmtxSync)
        {
            delete m_pmtxSync;
            m_pmtxSync = nullptr;
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


// ---------------------------------------------------------------------------
//  TCQCQTAudioPlayer: Public, inherited methods
// ---------------------------------------------------------------------------

// Mute or unmute our player
tCIDLib::TBoolean TCQCQTAudioPlayer::bMute() const
{
    return m_mbufShared->bMute;
}

tCIDLib::TBoolean TCQCQTAudioPlayer::bMute(const tCIDLib::TBoolean bToSet)
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    return orbcServer.bMute(bToSet);
}


// Get/set the volume
tCIDLib::TCard4 TCQCQTAudioPlayer::c4Volume() const
{
    return m_mbufShared.objData().c4Volume;
}

tCIDLib::TCard4 TCQCQTAudioPlayer::c4Volume(const tCIDLib::TCard4 c4Percent)
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    return orbcServer.c4Volume(c4Percent);
}


// Proivdes access to the current playback position
tCIDLib::TEncodedTime TCQCQTAudioPlayer::enctCurPos() const
{
    // WE have to lock since this isn't a fundamental 32 bit value
    TLocker lockrSync(m_pmtxSync);
    return m_mbufShared.objData().enctCurPos;
}


//
//  A way to get all the status of the player quickly
//
tCIDLib::TVoid
TCQCQTAudioPlayer::GetStatusInfo(tCQCMedia::EMediaStates&   eMediaState
                                , tCQCMedia::EEndStates&    eEndState
                                , tCIDLib::TEncodedTime&    enctPlayPos)
{
    // We have to lock for this one
    TLocker lockrSync(m_pmtxSync);

    tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();
    eEndState = Shared.eEndState;
    eMediaState = Shared.eState;
    enctPlayPos = Shared.enctCurPos;

    // Reset the dead man flag every time through
    Shared.enctDeadMan = TTime::enctNow();
}


//
//  The client calls this after construction. We use it to start up our
//  external process. We are tolerant here of things already having been
//  created or started.
//
tCIDLib::TVoid TCQCQTAudioPlayer::Initialize()
{
    // Create or open the sync event
    if (!m_pevSync)
    {
        m_pevSync = new TEvent
        (
            TResourceName
            (
                kCIDLib::pszResCompany
                , m_strUniqueName
                , TQTIntfClientProxy::strEvResName
            )
            , tCIDLib::EEventStates::Reset
            , tCIDLib::ECreateActs::OpenOrCreate
        );
    }

    // And the mutex we use to sync
    if (!m_pmtxSync)
    {
        m_pmtxSync = new TMutex
        (
            TResourceName
            (
                kCIDLib::pszResCompany
                , m_strUniqueName
                , TQTIntfClientProxy::strMtxResName
            )
            , tCIDLib::ELockStates::Unlocked
            , tCIDLib::ECreateActs::OpenOrCreate
        );
    }

    // The memory buffer use for comm
    if (!m_mbufShared.pobjData())
    {
        m_mbufShared.bMakeBuffer
        (
            TResourceName
            (
                kCIDLib::pszResCompany
                , m_strUniqueName
                , TQTIntfClientProxy::strMBufResName
            )
            , tCIDLib::EMemAccFlags::ReadWrite
            , tCIDLib::ECreateActs::OpenOrCreate
        );
    }

    // Start up the server program if not already running
    if (!m_extpServer.bIsRunning())
        StartServer();
}


// We just queue up a pause command on the player's queue
tCIDLib::TVoid TCQCQTAudioPlayer::Pause()
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    orbcServer.Pause();
}


// We just queue up a play command on the player's queue
tCIDLib::TVoid TCQCQTAudioPlayer::Play()
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    orbcServer.Play();
}


//
//  Loads a new file to play.
//
tCIDLib::TVoid
TCQCQTAudioPlayer::LoadFile(const   TString&    strToPlay
                            , const TString&    strTargetDev)
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    orbcServer.LoadFile(strToPlay, strTargetDev);
}


//
//  This is called to clear out any current loaded media, and it clears
//  the end of media flag. We just queue up a reset command on the player's
//  queue.
//
tCIDLib::TVoid TCQCQTAudioPlayer::Reset()
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    orbcServer.Reset();
}


// We just queue up a stop command on the player's queue
tCIDLib::TVoid TCQCQTAudioPlayer::Stop()
{
    CheckServerProc();
    TQTIntfClientProxy orbcServer(m_ooidServer, TString::strEmpty());
    orbcServer.Stop();
}


//
//  The client calls this before destruction. We shut down our external
//  player program.
//
tCIDLib::TVoid TCQCQTAudioPlayer::Terminate()
{
    // If it's running, try to stop it
    if (m_extpServer.bIsRunning())
    {
        // Trigger the sync event and then wait for the guy to die
        try
        {
            CIDAssert(m_pevSync != nullptr, L"Sync event is not set");
            m_pevSync->Trigger();
            m_extpServer.eWaitForDeath(10000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Close our shared resources
    delete m_pevSync;
    m_pevSync = nullptr;
    delete m_pmtxSync;
    m_pmtxSync = nullptr;
    m_mbufShared.Close();
}


// ---------------------------------------------------------------------------
//  TCQCQTAudioPlayer: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called before issuing any commands to the server. We'll see if
//  the program is still running. If not, we'll log a message and start it
//  up again.
//
tCIDLib::TVoid TCQCQTAudioPlayer::CheckServerProc()
{
    if (!m_extpServer.bIsRunning())
    {
        if (facCQCMedia().bLogInfo())
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kMedMsgs::midStatus_RestaringQTServer
                , tCIDLib::ESeverities::Info
                , tCIDLib::EErrClasses::AppStatus
                , m_strUniqueName
            );
        }
        StartServer();
    }
}


//
//  This is called to do the actual startup of the QT server proces. We set
//  up the shared memory with default initial info and start it up and wait
//  for it to come up and store the object id in the shared memory area.
//
tCIDLib::TVoid TCQCQTAudioPlayer::StartServer()
{
    // Initialize the shared memory
    tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();;

    Shared.bMute = kCIDLib::False;
    Shared.bQTAvail = kCIDLib::False;
    Shared.enctDeadMan = TTime::enctNow();
    Shared.c4InitVolume = c4DefVolume();
    Shared.c4Volume = c4DefVolume();
    Shared.eEndState = tCQCMedia::EEndStates::Ended;
    Shared.eState = tCQCMedia::EMediaStates::Undefined;
    Shared.enctCurPos = 0;
    Shared.i4QTVersion = 0;
    Shared.ippnServer = m_ippnServer;
    Shared.c4OIDBytes = 0;

    // Make sure the wait event is reset
    m_pevSync->Reset();

    //
    //  We create another event that is just temporary to wait for him to
    //  get up and running. So we just create it as a local.
    //
    TEvent evReady
    (
        TResourceName
        (
            kCIDLib::pszResCompany
            , m_strUniqueName
            , TQTIntfClientProxy::strReadyEvResName
        )
        , tCIDLib::EEventStates::Reset
        , tCIDLib::ECreateActs::CreateIfNew
    );

    //
    //  We have to pass in the unique name. We expect the QT server
    //  program to be in the same path as our facility. And we need to
    //  pass him some CQC/CIDLib specific info. But we have check first because
    //  it might already be in our environment (in the development setup mainly.)
    //
    TPathStr pathQTServer(facCQCMedia().strPath());
    pathQTServer.AddLevel(L"CQCQTAudSrv.exe");

    TEnvironment envServer(kCIDLib::True);
    if (!envServer.bKeyExists(kCQCKit::pszDataDirVarName))
        envServer.Add(kCQCKit::pszDataDirVarName, facCQCKit().strServerDataDir());

    TString strCmdLine(L"\"");
    strCmdLine.Append(pathQTServer);
    strCmdLine.Append(L"\" \"");
    strCmdLine.Append(m_strUniqueName);
    strCmdLine.Append(L'"');

    try
    {
        m_extpServer.Start
        (
            strCmdLine
            , facCQCMedia().strPath()
            , envServer
            , tCIDLib::EExtProcFlags
              (
                tCIDLib::EExtProcFlags::Detached
              )
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        if (facCQCMedia().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        if (facCQCMedia().bLogFailures())
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Couldn't start the QuickTime Server. ID=%(1)"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , m_strUniqueName
            );
        }
        throw;
    }

    catch(...)
    {
        if (facCQCMedia().bLogFailures())
        {
            facCQCMedia().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"Couldn't start the QuickTime Server. ID=%(1)"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , m_strUniqueName
            );
        }
        throw;
    }

    //
    //  Now wait for it to start up. He'll pulse the event once he starts
    //  up far enough to get his object id stored.
    //
    if (!evReady.bWaitFor(12000))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcQT_ServerNeverReady
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }

    // Stream out the object id
    TBinMBufInStream strmSrc(Shared.ac1OID, Shared.c4OIDBytes);
    strmSrc >> m_ooidServer;
}

