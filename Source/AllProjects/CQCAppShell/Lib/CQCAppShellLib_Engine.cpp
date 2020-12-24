//
// FILE NAME: CQCAppShellLib_Engine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This file implements our app shell engine derivative
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShellLib_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCAppShEngine,TAppShEngine)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCAppShellLib_Engine
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  The time that has to pass before we will log the same error again for
        //  the same application, to avoid logging lots of junk due to the shell's
        //  periodic nature.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard4 c4LogThrottlePeriod = kCIDLib::c4OneMinute;
    }
}


// ---------------------------------------------------------------------------
//  To reduce clutter we, provide a short named inline to get the output
//  stream to write output to. The facility class provides it.
// ---------------------------------------------------------------------------
static inline TTextOutStream& strmOut()
{
    return facCQCAppShellLib().strmOut();
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShEngine
//  PREFIX: aseng
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppShEngine: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppShEngine::TCQCAppShEngine() :

    m_bDoWaitCalls(kCIDLib::False)
    , m_c4ErrCount(0)
{
}

TCQCAppShEngine::~TCQCAppShEngine()
{
}


// ---------------------------------------------------------------------------
// TCQCAppShEngine: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCAppShEngine::bDoWaitCalls() const
{
    return m_bDoWaitCalls;
}

tCIDLib::TBoolean
TCQCAppShEngine::bDoWaitCalls(const tCIDLib::TBoolean bToSet)
{
    m_bDoWaitCalls = bToSet;
    return m_bDoWaitCalls;
}


tCIDLib::TCard4 TCQCAppShEngine::c4ErrCount() const
{
    return m_c4ErrCount;
}


tCIDLib::TVoid TCQCAppShEngine::ResetErrCount()
{
    m_c4ErrCount = 0;
}


// ---------------------------------------------------------------------------
//  TCQCAppShEngine: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCAppShEngine::bCheckShutdownComplete(TAppShRecord&           asrTarget
                                        , tCIDLib::EExitCodes&  eToFill)
{
    // Get it as it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrTarget);

    // Do a quick test to see if it's died already. If so, we are done
    if (asrActual.bWaitForDeath(eToFill, 250))
        return kCIDLib::True;

    //
    //  If we never got the event opened, then we should try it one more
    //  time, then give up on that.
    //
    if (asrActual.eStopStatus() == TCQCAppShRecord::EStopStatus::None)
    {
        //
        //  Try to create the named event that all our processes will have, and
        //  will block the main thread on.
        //
        try
        {
            TEvent evKill
            (
                asrActual.rsnWait()
                , tCIDLib::EEventStates::Reset
                , tCIDLib::ECreateActs::OpenIfExists
            );
            evKill.Trigger();

            // Indicate we've posted
            asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::PostedEvent);
        }

        catch(TError& errToCatch)
        {
            // If not just an open failure, report it
            if (!errToCatch.bCheckEvent(facCIDLib().strName()
                                        , kCIDErrs::errcEv_Open))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                ShellException(errToCatch, asrTarget);
            }

            //
            //  Give up on the event if we've been waiting more than a
            //  few seconds.
            //
            if (asrActual.c4ShutdownTime() > 2000)
                asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::NoEvent);
        }
    }
     else if (asrActual.eStopStatus() == TCQCAppShRecord::EStopStatus::PostedEvent)
    {
        //
        //  If we posted the event, and have been waiting for more than 10
        //  seconds, give up and move to the no-event state, to make us
        //  try to kill it hard.
        //
        if (asrActual.c4ShutdownTime() > 10000)
        {
            strmOut()   <<  facCQCAppShellLib().strMsg
                            (
                                kCQCShErrs::errcSh_NoEvResponse
                                , asrActual.strTitle()
                            )
                        << kCIDLib::EndLn;
            asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::NoEvent);
        }
    }
     else if (asrActual.eStopStatus() == TCQCAppShRecord::EStopStatus::NoEvent)
    {
        //
        //  Let's try to hard kill it, just to make sure that it's
        //  gone. This is non-optimal, but since it's not responding,
        //  it's probably dead anyway.
        //
        try
        {
            strmOut()   <<  facCQCAppShellLib().strMsg
                            (
                                kCQCShErrs::errcSh_TryHardKill
                                , asrActual.strTitle()
                            )
                        << kCIDLib::EndLn;

            asrTarget.extpThis().Kill();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ShellException(errToCatch, asrTarget);
            strmOut()   <<  facCQCAppShellLib().strMsg
                            (
                                kCQCShErrs::errcSh_HardKillFailed
                                , asrActual.strTitle()
                            )
                        << kCIDLib::EndLn;
        }

        catch(...)
        {
            ShellUnknownException(asrTarget);
            strmOut()   <<  facCQCAppShellLib().strMsg
                            (
                                kCQCShErrs::errcSh_HardKillFailed
                                , asrActual.strTitle()
                            )
                        << kCIDLib::EndLn;
        }

        // Indicate that we've tried a kill
        asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::Killed);
    }

    // Indicate not dead yet
    return kCIDLib::False;
}


tCIDLib::TVoid
TCQCAppShEngine::Exited(        TAppShRecord&       asrExited
                        , const tCIDLib::EExitCodes eCode)
{
    // Get it as it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrExited);

    //
    //  See if we've logged one of these within the minimum time period. If
    //  so, then don't log it.
    //
    const tCIDLib::TCard4 c4Cur = TTime::c4Millis();
    const tCIDLib::TCard4 c4TargetTime
    (
        asrActual.c4LastCycleTime()
        + (CQCAppShellLib_Engine::c4LogThrottlePeriod * 2)
    );

    // If the target time is still after the current time, then not yet
    if (c4TargetTime > c4Cur)
        return;

    // Bump the time stamp on this one to now
    asrActual.c4LastCycleTime(c4Cur);

    // Log this information
    if (facCQCAppShellLib().bLogInfo())
    {
        facCQCAppShellLib().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCShMsgs::midStatus_Exited
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , asrExited.strTitle()
            , tCIDLib::strLoadEExitCodes(eCode)
        );
    }

    // And display it to the user
    strmOut()   <<  facCQCAppShellLib().strMsg
                    (
                        kCQCShMsgs::midStatus_Exited
                        , asrExited.strTitle()
                        , tCIDLib::strLoadEExitCodes(eCode)
                    )
                <<  kCIDLib::NewLn << kCIDLib::FlushIt;
}


tCIDLib::TVoid TCQCAppShEngine::IdleCallback()
{
    // Nothing to do
}


tCIDLib::TVoid TCQCAppShEngine::Restarted(TAppShRecord& asrStarted)
{
    // Get it as it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrStarted);

    //
    //  See if we've logged one of these within the minimum time period. If
    //  so, then don't log it.
    //
    const tCIDLib::TCard4 c4Cur = TTime::c4Millis();
    const tCIDLib::TCard4 c4TargetTime
    (
        asrActual.c4LastCycleTime()
        + (CQCAppShellLib_Engine::c4LogThrottlePeriod * 2)
    );

    // If the target time is still after the current time, then not yet
    if (c4TargetTime > c4Cur)
        return;

    // Bump the time stamp on this one to now
    asrActual.c4LastCycleTime(c4Cur);

    // Log this information
    if (facCQCAppShellLib().bLogWarnings())
    {
        facCQCAppShellLib().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCShMsgs::midStatus_Restarting
            , tCIDLib::ESeverities::Warn
            , tCIDLib::EErrClasses::AppStatus
            , asrStarted.strTitle()
        );
    }

    // And display it to the user
    strmOut()   << facCQCAppShellLib().strMsg(kCQCShMsgs::midStatus_Restarting, asrStarted.strTitle())
                << kCIDLib::NewLn << kCIDLib::FlushIt;
}


tCIDLib::TVoid
TCQCAppShEngine::ShellException(TError&         errToCatch
                                , TAppShRecord& asrInvolved)
{
    // Get it as it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrInvolved);

    //
    //  If this is the same error we logged last time, see if the minimum
    //  period of time has expired. If not, don't log it or report it.
    //
    const tCIDLib::TCard4 c4Cur = TTime::c4Millis();
    if (asrActual.errLast().bSameEvent(errToCatch))
    {
        const tCIDLib::TCard4 c4TargetTime
        (
            asrActual.c4LastLogTime()
            + CQCAppShellLib_Engine::c4LogThrottlePeriod
        );

        // If the target time is still after the current time, then not yet
        if (c4TargetTime > c4Cur)
            return;
    }
     else
    {
        // Set this as the new last error for this app
        asrActual.errLast(errToCatch);
    }

    // Bump the time stamp on this one to now
    asrActual.c4LastLogTime(c4Cur);

    // If not logged to the log server yet, then log it
    if (!errToCatch.bLogged())
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // And display to the user in the shell log
    strmOut()   << facCQCAppShellLib().strMsg(kCQCShErrs::errcSh_Exception)
                << errToCatch.strErrText() << kCIDLib::NewLn << kCIDLib::FlushIt;

    // Bump the count of exceptions
    m_c4ErrCount++;
}


tCIDLib::TVoid TCQCAppShEngine::ShellMessage(const TString& strMsg)
{
    strmOut() << strMsg << kCIDLib::NewLn << kCIDLib::FlushIt;
}


tCIDLib::TVoid
TCQCAppShEngine::ShellUnknownException(TAppShRecord& asrInvolved)
{
    // Get it as it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrInvolved);

    //
    //  See if we've logged one of these within the minimum time period. If
    //  so, then don't log it.
    //
    const tCIDLib::TCard4 c4Cur = TTime::c4Millis();
    const tCIDLib::TCard4 c4TargetTime
    (
        asrActual.c4LastULogTime()
        + CQCAppShellLib_Engine::c4LogThrottlePeriod
    );

    // If the target time is still after the current time, then not yet
    if (c4TargetTime > c4Cur)
        return;

    // Bump the time stamp on this one to now
    asrActual.c4LastULogTime(c4Cur);

    // And display to the user
    strmOut()   << facCQCAppShellLib().strMsg(kCQCShErrs::errcSh_UnknownException)
                << kCIDLib::NewLn << kCIDLib::FlushIt;

    // Bump the count of exceptions
    m_c4ErrCount++;
}


tCIDLib::TVoid TCQCAppShEngine::Started(TAppShRecord& asrStarted)
{
    // Log this information
    if (facCQCAppShellLib().bLogInfo())
    {
        facCQCAppShellLib().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCShMsgs::midStatus_Starting
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
            , asrStarted.strTitle()
        );
    }

    // And display it to the user
    strmOut()   << facCQCAppShellLib().strMsg(kCQCShMsgs::midStatus_Starting, asrStarted.strTitle())
                << kCIDLib::NewLn << kCIDLib::FlushIt;
}


tCIDLib::TVoid TCQCAppShEngine::StartShutdown(TAppShRecord& asrTarget)
{
    // Cast it to it's actual type
    TCQCAppShRecord& asrActual = static_cast<TCQCAppShRecord&>(asrTarget);

    //
    //  Do a quick check to see if it is running. We cannot get out of sync
    //  on this, because the monitor thread is locked out during this call,
    //  and if it started any processes, they will be running already no
    //  matter how quickly after that we are called here. So if it is not
    //  running now, then we are done.
    //
    if (!asrTarget.bIsRunning())
        return;

    // Reset the stop status
    asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::None);

    //
    //  Try to create the named event that all our processes will have, and
    //  will block the main thread on.
    //
    try
    {
        TEvent evKill
        (
            asrActual.rsnWait()
            , tCIDLib::EEventStates::Reset
            , tCIDLib::ECreateActs::OpenIfExists
        );
        evKill.Trigger();
        asrActual.eStopStatus(TCQCAppShRecord::EStopStatus::PostedEvent);
    }

    catch(TError& errToCatch)
    {
        if (facCQCAppShellLib().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // If not just an open failure, report it
        if (!errToCatch.bCheckEvent(facCIDLib().strName()
                                    , kCIDErrs::errcEv_Open))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            ShellException(errToCatch, asrTarget);
        }
    }
}

