//
// FILE NAME: CQCAppShell_ServiceHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/09/2002
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
#include    "CQCAppShell.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TCQCAppShellService
//  PREFIX: srvh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCAppShellService: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCAppShellService::TCQCAppShellService() :

    m_pthrService(0)
{
}

TCQCAppShellService::~TCQCAppShellService()
{
    //
    //  Don't bother trying to delete the thread, and risk an exception
    //  if we didn't get it stopped.
    //
}


// ---------------------------------------------------------------------------
//  TCQCAppShellService: Public, inherited methods
// ---------------------------------------------------------------------------

// Our state is 'start pending' at this point
tCIDLib::TBoolean TCQCAppShellService::bStartServiceThread()
{
    try
    {

//
//  In case we need to debug startup, this will make it wait until we can
//  attach to it with the debugger after it starts up. Use the debugger to
//  force bWait to false and it'll fall out of this loop.
//
#if 0
        tCIDLib::TBoolean bWait = kCIDLib::True;
        while (bWait)
        {
            TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Start_Pending);
            TThread::Sleep(500);
        }
#endif

        //
        //  If it's already running, we shouldn't have got this, but just
        //  skip the startup and set it to running state in case.
        //
        if (!m_pthrService)
        {
            m_pthrService = new TThread
            (
                L"ServiceThread"
                , TMemberFunc<TFacCQCAppShell>(&facCQCAppShell(), &TFacCQCAppShell::eMainThread)
            );

            if (!m_pthrService->bIsRunning())
                m_pthrService->Start();
        }

        if (!TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Running, 0, 0))
            return kCIDLib::False;
    }

    catch(const TError& errToCatch)
    {
        if (facCQCAppShell().bShouldLog(errToCatch))
            TModule::LogEventObj(errToCatch);

        // Give up
        TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Stopped, 0, 1);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCAppShellService::bStopServiceThread()
{
    if (!TKrnlWin32Service::bPleaseWait(tCIDKernel::EWSrvStates::Stop_Pending))
        return kCIDLib::False;

    //
    //  Ask it to shut down and then wait for the thread to die. Because
    //  we have to send "we're working on it" notifications, we don't just
    //  using the standard blocking shutdown request mechanism. Instead,
    //  we tell the facility class to post an event that starts the shutdown,
    //  and we just loop, waiting for it to end.
    //
    facCQCAppShell().StartShutdown();
    tCIDLib::TCard4 c4Counter = 0;
    while (c4Counter < 120)
    {
        c4Counter++;
        try
        {
            if (!m_pthrService->bIsRunning())
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
        m_pthrService->eWaitForDeath(5000);
    }

    catch(...)
    {
    }

    // Whether it worked or not, set our state as stopped
    TKrnlWin32Service::bSetState(tCIDKernel::EWSrvStates::Stopped, 0, 0);

    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCAppShellService::bSuspendRequest()
{
    // Don't allow suspends
    return kCIDLib::False;
}


//
//  If a system shutdown is being done, then we handle the service cleanup here,
//  so that we can insure it gets time to finish, since otherwise the system
//  might shut down too fast.
//
//  If it's just a stop, we just let it be handled normally via the service
//  handling calling our bStopServiceThread() method, since we have as much time
//  as we need in that case.
//
//  In the case of a shutdown, we'll have already killed the service thread by
//  the time bStopServiceThread() is called, so it won't have to do anything.
//
tCIDLib::TVoid
TCQCAppShellService::ServiceOp(const tCIDKernel::EWSrvOps eOperation)
{
    if (eOperation == tCIDKernel::EWSrvOps::Shutdown)
    {
        if (m_pthrService->bIsRunning())
        {
            if (!bStopServiceThread())
            {
            }
        }
    }
}


//
//  We don't do anything for this. The service thread peridoically calls to
//  reset the activity timer and prevent the system from sleeping due to
//  inactivity.
//
tCIDLib::TVoid
TCQCAppShellService::ServiceOp(const tCIDKernel::EWSrvPwrOps ePowerOp)
{
}


