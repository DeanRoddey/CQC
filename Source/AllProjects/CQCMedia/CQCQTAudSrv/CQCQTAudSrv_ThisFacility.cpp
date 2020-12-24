//
// FILE NAME: CQCQTAudSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/11/2007
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
//  This file implements the facility class for the Quick Time audio server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCQTAudSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCQTAudSrv,TFacility)

// Allows us to run the program from the command line for debugging
#if CID_DEBUG_ON
// #define STANDALONETEST
#endif


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCQTAudSrv_ThisFacility
{
    // -----------------------------------------------------------------------
    //  The time we'll wait for the main thread to respond to a queued cmd
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4 c4WaitCmd = 10000;
}


// ---------------------------------------------------------------------------
//  A little helper class. It's a CFStringRef janitor to make sure that the
//  CF strings we have to allocate get cleaned up.
// ---------------------------------------------------------------------------
class TCFStringJan
{
    public :
        TCFStringJan(CFStringRef pRef) :
            m_pRef(pRef)
        {
        }

        ~TCFStringJan()
        {
            if (m_pRef)
                ::CFRelease(m_pRef);
        }

        tCIDLib::TVoid Orphan()
        {
            m_pRef = 0;
        }

        CFStringRef m_pRef;
};




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCQTAudSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCQTAudSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCQTAudSrv::TFacCQCQTAudSrv() :

    TFacility
    (
        L"CQCQTAudSrv"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_colCmdQ()
    , m_i4TimeScale(0)
    , m_Movie(0)
    , m_pevWait(0)
    , m_plgrLogSrv(0)
    , m_pmtxSync(0)
{
}

TFacCQCQTAudSrv::~TFacCQCQTAudSrv()
{
    delete m_pevWait;
}


// ---------------------------------------------------------------------------
//  TFacCQCQTAudSrv: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFacCQCQTAudSrv::bHandleSignal(const tCIDLib::ESignals eSig)
{
    //
    //  If it's a Ctrl-C or Ctrl-Break, just trigger the wait event. Ignore
    //  the others. This way, we can be stopped manually when running in
    //  debug mode, but don't get blown away by logoff signals when running
    //  under a service based app shell.
    //
    if ((eSig == tCIDLib::ESignals::CtrlC) || (eSig == tCIDLib::ESignals::Break))
        m_pevWait->Trigger();

    // And return that we handled it
    return kCIDLib::True;
}


// The main thread entry point of CQCQTAudSrv
tCIDLib::EExitCodes
TFacCQCQTAudSrv::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        return tCIDLib::EExitCodes::BadEnvironment;

    tCIDLib::EExitCodes eReturn = tCIDLib::EExitCodes::Normal;
    tCIDLib::TErrCode   errcToLog = 0;
    try
    {
        // Register ourself as a signal handler
        TSignals::c4AddHandler(this);

        //
        //  The very first thing we want to do is the most fundamental boot-
        //  strapping. We need to crank up the Orb client support (so that
        //  we can find the name server.) And then install a log server
        //  logger, which will allow anything that goes wrong after that to
        //  go to the log server.
        //
        errcToLog = kKitErrs::errcApp_InitCORB;
        facCIDOrb().InitClient();

        //
        //  The next thing we want to do is to install a log server logger. We
        //  just use the standard one that's provided by CIDLib. It logs to
        //  the standard CIDLib log server, and uses a local file for fallback
        //  if it cannot connect.
        //
        m_plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(m_plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        // Log that we are staring up
        if (bLogInfo())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midApp_Startup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , TString(L"CQCQTAudSrv, ID=") + m_strUniqueID
                , strVersion()
            );
        }

        // We have to get our unique id from the calling process
        if (TSysInfo::c4CmdLineParmCount() != 1)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kQTASErrs::errcGen_NoUniqueId
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                );
            }
            return tCIDLib::EExitCodes::BadParameters;
        }
        m_strUniqueID = TSysInfo::strCmdLineParmAt(0);

        //
        //  Now we need to open up some shared resources that the calling
        //  process will have set up for us. We have an event that he
        //  uses to signal us to shut down, a mutex we both use to control
        //  access to the memory we share, and the shared memory buffer.
        //
        //  These all use the provided unique id as part of the resource
        //  name, so that we won't clash with any other instance of this
        //  program.
        //
        try
        {
            if (bLogInfo())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Creating QT audio server shared resources. ID=%(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strUniqueID
                );
            }

            //
            //  If doing a standalone test we we more forgiving on the
            //  create action.
            //
            tCIDLib::ECreateActs eCreateAct = tCIDLib::ECreateActs::OpenIfExists;
            #if defined(STANDALONETEST)
            eCreateAct = tCIDLib::ECreateActs::OpenOrCreate;
            #endif

            m_pevWait = new TEvent
            (
                TResourceName
                (
                    kCIDLib::pszResCompany
                    , m_strUniqueID
                    , TQTIntfServerBase::strEvResName
                )
                , tCIDLib::EEventStates::Reset
                , eCreateAct
            );

            // That worked, so create the mutex
            m_pmtxSync = new TMutex
            (
                TResourceName
                (
                    kCIDLib::pszResCompany
                    , m_strUniqueID
                    , TQTIntfServerBase::strMtxResName
                )
                , tCIDLib::ELockStates::Unlocked
                , eCreateAct
            );

            // And finally create the shared buffer
            m_mbufShared.bMakeBuffer
            (
                TResourceName
                (
                    kCIDLib::pszResCompany
                    , m_strUniqueID
                    , TQTIntfServerBase::strMBufResName
                )
                , tCIDLib::EMemAccFlags::ReadWrite
                , eCreateAct
            );

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"QT audio server shared resources created. ID=%(1)"
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strUniqueID
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            if(bShouldLog(errToCatch))
                LogEventObj(errToCatch);

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kQTASErrs::errcGen_CreateSharedRes
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
            throw;
        }

        // If in standalone test mode, fake the incoming buffer contents
        #if defined(STANDALONETEST)
        tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();;
        Shared.bMute = kCIDLib::False;
        Shared.bQTAvail = kCIDLib::False;
        Shared.enctDeadMan = TTime::enctNow();
        Shared.c4InitVolume = 50;
        Shared.c4Volume = 50;
        Shared.eEndState = tCQCMedia::EEndStates::Ended;
        Shared.eState = tCQCMedia::EMediaStates::Undefined;
        Shared.enctCurPos = 0;
        Shared.i4QTVersion = 0;
        Shared.ippnServer = 9999;
        Shared.c4OIDBytes = 0;
        #endif

        //
        //  Init the server side of the ORB now. We only take one connection
        //  since only the program that started us ever talks to us.
        //
        errcToLog = kKitErrs::errcApp_InitSORB;
        facCIDOrb().InitServer(m_mbufShared->ippnServer, 1);

        //
        //  Next, check to see if Quick Time is available, and store that
        //  status info (and the QT version) before we start up the auto
        //  binding and become available to clients.
        //
        CheckQTAvail();

        // Register an instance of our interface implementation with the ORB
        TQTIntfServerImpl* porbsIntf = new TQTIntfServerImpl;
        facCIDOrb().RegisterObject(porbsIntf, tCIDLib::EAdoptOpts::Adopt);

        //
        //  We need to format out our object id to the shared buffer, which
        //  the calling program will then stream out and use to connect to
        //  us via the ORB.
        //
        StoreOID(porbsIntf->ooidThis());

        //
        //  We can now trigger the other event that we use to indicate that
        //  we are ready. We can't use the wait event since we have to block
        //  on it below. The caller created it, so we just need to open
        //  it.
        //
        {
            // If doing a standane test, we may need to create it
            tCIDLib::ECreateActs eCreateAct = tCIDLib::ECreateActs::OpenIfExists;
            #if defined(STANDALONETEST)
            eCreateAct = tCIDLib::ECreateActs::OpenOrCreate;
            #endif

            TEvent evReady
            (
                TResourceName
                (
                    kCIDLib::pszResCompany
                    , m_strUniqueID
                    , TQTIntfServerBase::strReadyEvResName
                )
                , tCIDLib::EEventStates::Reset
                , eCreateAct
            );
            evReady.Trigger();
        }

        //
        //  If QT is available, process requests. Else, just block on the
        //  shutdown event until asked to exit. We've set the QT availability
        //  status in the shared memory buffer, so he knows that there's
        //  use calling us if it's not available.
        //
        if (m_mbufShared->bQTAvail)
        {
            if (bLogInfo())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kQTASMsgs::midStatus_ProcessingReqs
                    , tCIDLib::ESeverities::Info
                    , tCIDLib::EErrClasses::AppStatus
                    , m_strUniqueID
                );
            }
            ProcessRequests();
        }
         else
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kQTASMsgs::midStatus_NoQTAvail
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::OutResource
                , m_strUniqueID
            );
            m_pevWait->WaitFor();
        }

        //
        //  Indicate that we are going bye-bye in a normal way if we got the
        //  logger installed.
        //
        if (bLogInfo())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitMsgs::midApp_NormalExit
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(const TError& errToCatch)
    {
        // If the error hasn't been logged, then log it
        if (bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        if (errcToLog && bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , errcToLog
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        eReturn = tCIDLib::EExitCodes::FatalError;
    }

    catch(...)
    {
        if (errcToLog && bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , errcToLog
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        eReturn = tCIDLib::EExitCodes::FatalError;
    }

    // Do the cleanup work
    DoCleanup();

    return eReturn;
}


// ---------------------------------------------------------------------------
//  TFacCQCQTAudSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------


//
//  Set the mute state. We just store the last value set as the current
//  mute state.
//
tCIDLib::TBoolean TFacCQCQTAudSrv::bMute(const tCIDLib::TBoolean bToSet)
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Mute)
    );
    cptrEv->m_bVal = bToSet;
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;

    //
    //  Store the newly set state. No need to lock since the other process
    //  is now blocked on this call to us.
    //
    m_mbufShared->bMute = bToSet;

    return bToSet;
}


//
//  Set the volume. We just store the last value set as the current
//  volume.
//
tCIDLib::TCard4 TFacCQCQTAudSrv::c4Volume(const tCIDLib::TCard4 c4Percent)
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Volume)
    );
    cptrEv->m_c4Val = c4Percent;
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;

    //
    //  Store this as the last volume set and return the new value. WE
    //  don't have to lock since the other process is now blocked on this
    //  call to us.
    //
    m_mbufShared->c4Volume = c4Percent;

    return c4Percent;
}


// We just queue up a pause command on the player's queue
tCIDLib::TVoid TFacCQCQTAudSrv::Pause()
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Pause)
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}


// We just queue up a play command on the player's queue
tCIDLib::TVoid TFacCQCQTAudSrv::Play()
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Play)
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}


//
//  Loads a new file to play.
//
tCIDLib::TVoid
TFacCQCQTAudSrv::LoadFile(const TString& strToPlay, const TString& strAudioDev)
{
    ThrowIfNotReady();

    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"QT server '%(1)' got request to load new file on device '%(2)'"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::CantDo
            , m_strUniqueID
            , strAudioDev
        );
    }

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData
        (
            TCQCAudioPlayer::EEvTypes::Load, strToPlay, strAudioDev
        )
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}


//
//  This is called to clear out any current loaded media, and it clears
//  the end of media flag. We just queue up a reset command on the player's
//  queue.
//
tCIDLib::TVoid TFacCQCQTAudSrv::Reset()
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Reset)
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}


// We just queue up a stop command on the player's queue
tCIDLib::TVoid TFacCQCQTAudSrv::Stop()
{
    ThrowIfNotReady();

    TCQCAudioPlayer::TQElem cptrEv
    (
        new TCQCAudioPlayer::TEvData(TCQCAudioPlayer::EEvTypes::Stop)
    );
    m_colCmdQ.objAdd(cptrEv);

    // Wait a while for it to complete
    cptrEv->m_evWait.WaitFor(CQCQTAudSrv_ThisFacility::c4WaitCmd);

    if (cptrEv->m_perrFailure)
        throw *cptrEv->m_perrFailure;
}



// ---------------------------------------------------------------------------
//  TFacCQCQTAudSrv: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to apply a new percent level volume to the movie object.
//
//  NOTE: We cannot optimize and not do anything if the new volume is the
//  same as the current one, because it also is called on a change to the
//  mute state because we have to set a negative volume.
//
tCIDLib::TVoid
TFacCQCQTAudSrv::ApplyMovieVolume(const tCIDLib::TCard4 c4Volume)
{
    if (m_Movie)
    {
        //
        //  The volume is 0.0 to 1.0, with the integral part in the top
        //  byte of a word and the fractional part in the low byte. So
        //  basically except for full volume (1.0) the top word is always
        //  empty and the bottom holds the actual percentage.
        //
        tCIDLib::TFloat8 f8Percent(c4Volume);
        f8Percent /= 100.0;
        f8Percent *= 256;
        ::SetMovieVolume(m_Movie, tCIDLib::TInt2(f8Percent));
    }
}


// Checks to see if QT is available and, if so, gets it initialized
tCIDLib::TVoid TFacCQCQTAudSrv::CheckQTAvail()
{
    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"Initializing QuickTime support. ID=%(1)"
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , m_strUniqueID
        );
    }

    // See if QT is available
    OSErr QTErr;
    m_mbufShared->bQTAvail = kCIDLib::False;
    m_mbufShared->i4QTVersion = 0;
    try
    {
        QTErr = ::InitializeQTML(0);
        if (QTErr)
        {
            if (bLogFailures())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kQTASErrs::errcQT_InitFailed
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::InitFailed
                    , TInteger(QTErr)
                );
            }
        }
         else
        {
            tCIDLib::TInt4 i4Ver;
            ::Gestalt(gestaltQuickTime, &i4Ver);
            m_mbufShared->bQTAvail = kCIDLib::True;
            m_mbufShared->i4QTVersion = i4Ver;

            // And we need to call this before we can do any QT calls
            QTErr = ::EnterMovies();
            if (QTErr)
            {
                if (bLogFailures())
                {
                    LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kQTASErrs::errcQT_Init2Failed
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::InitFailed
                        , TInteger(QTErr)
                    );
                }
            }
        }
    }

    catch(...)
    {
        if (bLogFailures())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kQTASErrs::errcQT_InitFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::InitFailed
                , TInteger(QTErr)
            );
        }
    }

    // If not available, then log that
    if (!m_mbufShared->bQTAvail)
    {
        if (bLogStatus())
        {
            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kQTASErrs::errcQT_QTNotAvailable
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


//
//  This is called to stop any current playback and clean up any resources
//  used during playback, both on the way out and to just clean up after
//  the end of the current media we are playing.
//
tCIDLib::TVoid TFacCQCQTAudSrv::CleanupPlayer(const tCIDLib::TBoolean bErr)
{
    if (m_Movie)
    {
        //
        //  For whatever reason we MUST do a call to MoviesTask before
        //  we stop it else it will fail.
        //
        ::MoviesTask(m_Movie, 200);
        ::StopMovie(m_Movie);

        // Give it some time to wrap up before we dispose
        TThread::Sleep(250);
        ::DisposeMovie(m_Movie);

        // Reset some stuff in the memory buffer
        TLocker lockrSync(m_pmtxSync);

        tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();
        Shared.enctCurPos = 0;
        Shared.eEndState = bErr ? tCQCMedia::EEndStates::Failed
                                : tCQCMedia::EEndStates::Ended;
        Shared.eState = tCQCMedia::EMediaStates::Stopped;
    }
}


//
//  This is called on exit, normal or early exit, to clean up anything
//  that we got going during start, in the correct order.
//
tCIDLib::TVoid TFacCQCQTAudSrv::DoCleanup()
{
    //
    //  Terminate the Orb support. Force the log server logger back to
    //  local logger before we kill the ORB client.
    //
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->bForceLocal(kCIDLib::True);
        facCIDOrb().Terminate();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_TermORB
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(...)
    {
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_TermORB
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    //
    //  And finally make the logger stop logging. We told TModule to adopt it
    //  so he will clean it up.
    //
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->Cleanup();
    }

    catch(...)
    {
    }

    // And we can clean up the shared stuff
    delete m_pevWait;
    m_pevWait = nullptr;

    delete m_pmtxSync;
    m_pmtxSync = nullptr;
}


//
//  This is called to load up a new URL and set up the selected audio device
//  that we are told to play through.
//
//  We first clean up any previous movie, then create the new audio device,
//  and then load up the new movie.
//
//  !!!!!!IMPORTANT!!!!!!
//
//  DO NOT put any calls to MovieTask in here. Just start the movie playing
//  and return. For whatever reason, it will damage the movie object and
//  you can't delete it and we get stuck and can't do anything.
//
tCIDLib::TVoid
TFacCQCQTAudSrv::LoadNewURL(const TString& strToLoad, const TString& strAudioDev)
{
    OSStatus OSStatus;

    if (bLogInfo())
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"QT server '%(1)' is loading a new file on device '%(2)'"
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , m_strUniqueID
            , strAudioDev
        );
    }

    // Clean up any existing playback
    if (m_Movie)
        CleanupPlayer(kCIDLib::False);

    //
    //  Convert the passed audio device name into a system level audio
    //  device identifier that we need.
    //
    TString strAudioID;
    if (!TCQCAudioPlayer::bFindDevGUID(strAudioDev, strAudioID))
    {
        facCQCMedia().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kMedErrs::errcMP_NoSuchAudioDev
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::CantDo
            , strAudioDev
        );
    }

    //
    //  Create the audio device object for the audio device we were told
    //  to use. This has to succeed or we bail out
    //
    QTAudioContextRef AudioContextRef = 0;
    {
        CFStringRef CFAudioDev = NULL;
        CFAudioDev = ::CFStringCreateWithCharacters
        (
            NULL, (UniChar*)strAudioID.pszBuffer(), strAudioID.c4Length()
        );
        TCFStringJan janToPlay(CFAudioDev);

        OSStatus = ::QTAudioContextCreateForAudioDevice
        (
            NULL, CFAudioDev, NULL, &AudioContextRef
        );

        if (OSStatus != noErr)
        {
            ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kQTASErrs::errcQT_CreateAudioDev
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::CantDo
                , strAudioDev
                , TInteger(OSStatus)
            );
        }
    }

    // Convert the source file to a CFString
    CFStringRef CFToPlay = NULL;
    CFToPlay = ::CFStringCreateWithCharacters
    (
        kCFAllocatorDefault
        , (UniChar*)strToLoad.pszBuffer()
        , strToLoad.c4Length()
    );
    TCFStringJan janToPlay(CFToPlay);

    //
    //  That worked so try to create a movie. Because we want to set the
    //  target audio device, we have to use the less convenient version
    //  that creates from an array of properties.
    //
    const tCIDLib::TCard4 c4PropCnt = 5;
    QTNewMoviePropertyElement aPropList[c4PropCnt] = {0};

    aPropList[0].propClass = kQTPropertyClass_DataLocation;
//    aPropList[0].propID = kQTDataLocationPropertyID_CFStringNativePath;
    aPropList[0].propID = kQTDataLocationPropertyID_CFStringWindowsPath;
    aPropList[0].propValueSize = sizeof(CFToPlay);
    aPropList[0].propValueAddress = (void*)&CFToPlay;
    aPropList[0].propStatus = 0;

    bool boolVal = true;
    aPropList[1].propClass = kQTPropertyClass_MovieInstantiation;
    aPropList[1].propID = kQTMovieInstantiationPropertyID_DontAskUnresolvedDataRefs;
    aPropList[1].propValueSize = sizeof(boolVal);
    aPropList[1].propValueAddress = &boolVal;
    aPropList[1].propStatus = 0;

    aPropList[2].propClass = kQTPropertyClass_NewMovieProperty;
    aPropList[2].propID = kQTNewMoviePropertyID_Active;
    aPropList[2].propValueSize = sizeof(boolVal);
    aPropList[2].propValueAddress = &boolVal;
    aPropList[2].propStatus = 0;

    aPropList[3].propClass = kQTPropertyClass_NewMovieProperty;
    aPropList[3].propID = kQTNewMoviePropertyID_DontInteractWithUser;
    aPropList[3].propValueSize = sizeof(boolVal);
    aPropList[3].propValueAddress = &boolVal;
    aPropList[3].propStatus = 0;

    aPropList[4].propClass = kQTPropertyClass_Context;
    aPropList[4].propID = kQTContextPropertyID_AudioContext;
    aPropList[4].propValueSize = sizeof(AudioContextRef);
    aPropList[4].propValueAddress = &AudioContextRef;
    aPropList[4].propStatus = 0;

    Movie NewMovie;
    OSStatus = ::NewMovieFromProperties
    (
        5, aPropList, 0, NULL, &NewMovie
    );

    // We can let the audio context go now
    ::QTAudioContextRelease(AudioContextRef);

    if (OSStatus != noErr)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kQTASErrs::errcQT_CreateMovie
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::CantDo
            , TInteger(OSStatus)
        );
    }

    //
    //  Pause a while to let it get going. Otherwise, it can cause an
    //  exception if we do something too quickly after creation.
    //
    TThread::Sleep(250);

    //
    //  Store the time scale for later use, and zero out the current position
    //  so it'll be zero until we start updating it again.
    //
    m_i4TimeScale = ::GetMovieTimeScale(NewMovie);
    m_mbufShared->enctCurPos = 0;

    // It worked so store the new movie and set the status to loading
    m_mbufShared->eEndState = tCQCMedia::EEndStates::Running;
    m_mbufShared->eState = tCQCMedia::EMediaStates::Loading;
    m_Movie = NewMovie;

    // Set the initial volume, based on the last set volume and mute state
    ApplyMovieVolume(m_mbufShared->c4Volume);
    if (m_mbufShared->bMute)
        ::SetMovieAudioMute(NewMovie, m_mbufShared->bMute, 0);

    // And now start it playing
    ::StartMovie(m_Movie);
}


//
//  This is called from the processing loop any time we get a posted cmd
//  from the controlling app.
//
//
//  NOTE:   We don't have to lock in order to write to the shared buffer
//          because the calling app is blocked in the ORB waiting for
//          us to return back to him.
//
tCIDLib::TVoid TFacCQCQTAudSrv::ProcessCmd(TCQCAudioPlayer::TQElem& cptrEv)
{
    try
    {
        // We got one, so process it
        switch(cptrEv->m_eType)
        {
            case TCQCAudioPlayer::EEvTypes::Load :
            {
                LoadNewURL(*cptrEv->m_pstrVal1, *cptrEv->m_pstrVal2);
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Mute :
            {
                //
                //  Store the new mute value. If we have an active movie,
                //  the update the movie object.
                //
                m_mbufShared->bMute = cptrEv->m_bVal;
                if (m_Movie)
                    ::SetMovieAudioMute(m_Movie, m_mbufShared->bMute, 0);
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Pause :
            {
                if (m_Movie)
                    ::StopMovie(m_Movie);
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Play :
            {
                if (m_Movie)
                {
                    ::StartMovie(m_Movie);
                    ::MoviesTask(m_Movie, 0);
                }

                tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();
                Shared.eState = tCQCMedia::EMediaStates::Playing;
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Reset :
            {
                CleanupPlayer(kCIDLib::False);
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Volume :
            {
                //
                //  We get a 0 to 100 value. We have to convert it to a
                //  0 to 1.0 value with the integral part in the high 8
                //  bits, and the fractional part in the low 8.
                //
                if (m_Movie)
                    ApplyMovieVolume(cptrEv->m_c4Val);
                break;
            }

            case TCQCAudioPlayer::EEvTypes::Stop :
            {
                if (m_Movie)
                {
                    ::StopMovie(m_Movie);
                    ::SetMovieTimeValue(m_Movie, 0);
                }
                break;
            }

            default :
                // <TBD> Dunno what this is, should log something
                break;
        };

        // Trigger the event, and then drop the reference.
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);

        cptrEv->m_perrFailure = new TError(errToCatch);
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
        throw;
    }

    catch(...)
    {
        cptrEv->m_perrFailure = new TError
        (
            strName()
            , CID_FILE
            , CID_LINE
            , kMedErrs::errcMP_CmdError
            , facCQCMedia().strMsg
              (
                kMedErrs::errcMP_CmdError, TCardinal(tCIDLib::c4EnumOrd(cptrEv->m_eType))
              )
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
        );
        cptrEv->m_evWait.Trigger();
        cptrEv.DropRef();
        throw;
    }
}


//
//  The body of the work done is just separated out to this method in order
//  to keep the main thread code above cleaner. We just loop until asked to
//  shutdown, processing any commands posted via the ORB interface and
//  polling the QT engine for status which we store away.
//
tCIDLib::TVoid TFacCQCQTAudSrv::ProcessRequests()
{
    tCIDLib::TBoolean       bDone = kCIDLib::False;
    TCQCAudioPlayer::TQElem cptrEv;
    tCIDLib::TCard4         c4LoopCnt = 0;

    while (!bDone)
    {
        c4LoopCnt++;

        //
        //  Block a little on each round. We do it by blocking on the
        //  event that is used to ask us to shut down, so we kill two
        //  birds with one stone. If we get back because it was posted,
        //  then we know we need to return.
        //
        if (m_pevWait->bWaitFor(100))
            break;

        try
        {
            //
            //  Wait a bit for a command to come in, using a short time
            //  out.
            //
            if (m_colCmdQ.bGetNext(cptrEv, 100))
                ProcessCmd(cptrEv);

            //
            //  Every 4 times through, get the current state and time
            //  position if the state indicates we are playing. And every
            //  time through, call the movies task.
            //
            if (m_Movie)
                ::MoviesTask(m_Movie, 0);

            if (!(c4LoopCnt % 4))
            {
                tCQCMedia::TQTSharedInfo& Shared = m_mbufShared.objData();

                // If we have a movie, then do some work
                if (m_Movie)
                {
                    // Calc current elapsed time
                    const tCIDLib::TEncodedTime enctNew
                    (
                        (::GetMovieTime(m_Movie, 0) / m_i4TimeScale)
                        * kCIDLib::enctOneSecond
                    );

                    // See if we have a new end of media
                    tCIDLib::TBoolean bAtEnd = kCIDLib::False;
                    tCIDLib::TBoolean bIsDone = (::IsMovieDone(m_Movie) != 0);

                    // We need to lock for the rest
                    TLocker lockrSync(m_pmtxSync);

                    // Store the new position
                    Shared.enctCurPos = enctNew;

                    //
                    //  If we hadn't already ended, and we have now, then we
                    //  have transitions to end of media.
                    //
                    if ((Shared.eEndState == tCQCMedia::EEndStates::Running)
                    &&  bIsDone)
                    {
                        Shared.eEndState = tCQCMedia::EEndStates::Ended;
                        bAtEnd = kCIDLib::True;
                    }

                    //
                    //  Get the rate, which tells us if we are playing
                    //  or stopped.
                    //
                    const tCQCMedia::EMediaStates eOldState = Shared.eState;
                    if (bAtEnd)
                    {
                        // We ended so go to stopped state
                        Shared.eState = tCQCMedia::EMediaStates::Stopped;
                    }
                     else
                    {
                        Fixed CurRate = ::GetMovieRate(m_Movie);
                        if (CurRate)
                            Shared.eState = tCQCMedia::EMediaStates::Playing;
                        else
                            Shared.eState = tCQCMedia::EMediaStates::Stopped;
                    }

                    // We are unlocked now. If we hit the end then clean up
                    if (bAtEnd)
                        CleanupPlayer(kCIDLib::False);
                }

                //
                //  Every so many times through, check the the deadman and
                //  see if the parent player hasn't updated it in the last
                //  minute. If so, he must be dead.
                //
                #if !defined(STANDLONETEST)
                if (!(c4LoopCnt % 5))
                {
                    TLocker lockrSync(m_pmtxSync);

                    //
                    //  Be careful just in case some slight race condition
                    //  in the way the system reports the time to different
                    //  processes that we don't get a negative number and
                    //  incorrectly bail out.
                    //
                    const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
                    if ((enctNow > Shared.enctDeadMan)
                    &&  ((enctNow - Shared.enctDeadMan) > kCIDLib::enctOneMinute))
                    {
                        if (facCQCMedia().bLogWarnings())
                        {
                            facCQCQTAudSrv.LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kQTASErrs::errcGen_DeadMan
                                , tCIDLib::ESeverities::Warn
                                , tCIDLib::EErrClasses::Timeout
                            );
                        }
                        bDone = kCIDLib::True;
                        m_pevWait->Trigger();
                        break;
                    }
                }
                #endif
            }
        }

        catch(const TError& errToCatch)
        {
            try
            {
                if (facCQCMedia().bShouldLog(errToCatch))
                    TModule::LogEventObj(errToCatch);

                if (facCQCMedia().bLogFailures())
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcMP_LoopExcept
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }

                if (cptrEv.pobjData())
                {
                    cptrEv->m_evWait.Trigger();
                    cptrEv.DropRef();
                }
                CleanupPlayer(kCIDLib::True);
            }

            catch(...)
            {
                // Do a panic recycle
                TProcess::ExitProcess(tCIDLib::EExitCodes::FatalError);
            }
            m_colCmdQ.RemoveAll();
        }

        catch(...)
        {
            try
            {
                if (facCQCMedia().bLogFailures())
                {
                    facCQCMedia().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kMedErrs::errcMP_LoopExcept
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::AppStatus
                    );
                }

                if (cptrEv.pobjData())
                {
                    cptrEv->m_evWait.Trigger();
                    cptrEv.DropRef();
                }
                CleanupPlayer(kCIDLib::True);
            }

            catch(...)
            {
                // Do a panic recycle
                TProcess::ExitProcess(tCIDLib::EExitCodes::FatalError);
            }
            m_colCmdQ.RemoveAll();
        }
    }

    // We can now clean up the QT environment
    try
    {
        ::ExitMovies();
        ::TerminateQTML();
    }

    catch(...)
    {
    }
}


//
//  We flaten our object id to the shared memory buffer and update the
//  size field so that the calling process can get to it.
//
tCIDLib::TVoid TFacCQCQTAudSrv::StoreOID(const TOrbObjId& ooidToStore)
{
    TBinMBufOutStream strmOut(2048UL, 2048UL);
    strmOut << ooidToStore << kCIDLib::FlushIt;

    m_mbufShared->c4OIDBytes = strmOut.c4CurSize();
    strmOut.mbufData().CopyOut(m_mbufShared->ac1OID, m_mbufShared->c4OIDBytes);
}


//
//  Called upon entry to any of the methods that the driver calls, to throw
//  if we couldn't get QT initialized.
//
tCIDLib::TVoid TFacCQCQTAudSrv::ThrowIfNotReady()
{
    if (!m_mbufShared->bQTAvail)
    {
        ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kQTASErrs::errcQT_QTNotAvailable
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
}

