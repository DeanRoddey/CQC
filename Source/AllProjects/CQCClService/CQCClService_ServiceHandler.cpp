//
// FILE NAME: CQCClService_ServiceHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/20/2013
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
//  This file implements the Win32 service handler that attachs us to the
//  service manager.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCClService.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TCQCClServiceService
//  PREFIX: srvh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCClServiceService: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCClServiceService::TCQCClServiceService()
{
}

TCQCClServiceService::~TCQCClServiceService()
{
}


// ---------------------------------------------------------------------------
//  TCQCClServiceService: Public, inherited methods
// ---------------------------------------------------------------------------

// Our state is 'start pending' at this point
tCIDLib::TBoolean TCQCClServiceService::bStartServiceThread()
{
    //
    //  In case we need to debug startup, this will make it wait until we can
    //  attach to it with the debugger after it starts up. Use the debugger to
    //  force bWait to false and it'll fall out of this loop.
    //
    #if CID_DEBUG_ON
    #if defined(DBG_SERVICE_MODE)

        tCIDLib::TBoolean bWait = kCIDLib::True;
        while (bWait)
        {
            TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Start_Pending);
            TThread::Sleep(500);
        }
    #endif
    #endif

    try
    {
        //
        //  If it's already running, we shouldn't have got this, but just
        //  skip the startup and set it to running state in case.
        //
        if (!facCQCClService().bSrvRunning())
            facCQCClService().StartService();

        if (!TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Running, 0, 0))
            return kCIDLib::False;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCClService().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Give up
        TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Stopped, 0, 1);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCClServiceService::bStopServiceThread()
{
    // If the service thread isn't running, then nothing to do
    if (!facCQCClService().bSrvRunning())
        return kCIDLib::True;

    facCQCClService().LogMsg
    (
        CID_FILE, CID_LINE, L"Stopping service thread", tCIDLib::ESeverities::Status
    );

    TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Stop_Pending);

    //
    //  Ask it to shut down and then wait for the thread to die. Because
    //  we have to send "we're working on it" notifications, we don't just
    //  using the standard blocking shutdown request mechanism. Instead,
    //  we tell the facility class to post an event that starts the shutdown,
    //  and we just loop, waiting for it to end.
    //
    facCQCClService().StartShutdown();
    tCIDLib::TCard4 c4Counter = 0;
    while (c4Counter < 120)
    {
        c4Counter++;
        try
        {
            if (!facCQCClService().bSrvRunning())
                break;

            TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Stop_Pending);
            TThread::Sleep(200);
        }

        catch(...)
        {
        }
    }

    // Wait that last bit for it to really end
    try
    {
        TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Stop_Pending);
        facCQCClService().WaitComplete();
    }

    catch(...)
    {
    }

    // Whether it worked or not, set our state as stopped
    TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Stopped, 0, 0);

    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCClServiceService::bSuspendRequest()
{
    // We allow suspends
    return kCIDLib::True;
}


//
//  We only support stop/start (plus power ops below), so we don't expect to be
//  called here other than for start/stop, and the actual real work for start and
//  stop are done via the start/stop service thread methods above. If we supported
//  pause/continue, those would be handled here.
//
tCIDLib::TVoid TCQCClServiceService::ServiceOp(const tCIDKernel::EWSrvOps)
{
}


//
//  We stop and stat our processing thread when suspended and resumed. We don't try
//  to log here since the logging support is being started/stopped during this. Our
//  base class will log to the service oriented local text file to indicate what
//  events were received, so that's enough for post-mortem debugging if needed.
//
tCIDLib::TVoid
TCQCClServiceService::ServiceOp(const tCIDKernel::EWSrvPwrOps ePowerOp)
{
    //
    //  The resume auto notification is always sent, regardless of why we were
    //  suspended. So, we just watch for it and start back up. We are non-interactive
    //  so it doesn't matter why we got suspended.
    //
    if (ePowerOp == tCIDKernel::EWSrvPwrOps::ResumeAuto)
    {
        facCQCClService().StartProcessing();
    }
     else if (ePowerOp == tCIDKernel::EWSrvPwrOps::Suspend)
    {
        facCQCClService().StopProcessing();
    }
}


