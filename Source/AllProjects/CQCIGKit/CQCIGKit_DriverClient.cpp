//
// FILE NAME: CQCIGKit_DriverClient.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/16/2001
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
//  Implements the standard CQC client side driver window, from which all
//  client drivers derive.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIGKit_.hpp"



// ----------------------------------------------------------------------------
//  Do our RTTI macros
// ----------------------------------------------------------------------------
RTTIDecls(TCQCDriverClient,TGenericWnd)


// ---------------------------------------------------------------------------
//   CLASS: TCQCDriverClient
//  PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDriverClient: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDriverClient::TCQCDriverClient( const   TCQCDriverObjCfg&   cqcdcThis
                                    , const TString&            strDriverClass
                                    , const tCQCKit::EActLevels eActivityLevel
                                    , const TCQCUserCtx&        cuctxToUse) :
    TGenericWnd()
    , m_bCleanupDone(kCIDLib::False)
    , m_bFirstTimer(kCIDLib::True)
    , m_c4PollChanges(0)
    , m_cuctxToUse(cuctxToUse)
    , m_eActivityLevel(eActivityLevel)
    , m_eConnState(tCQCGKit::EConnStates::SrvOffline)
    , m_ePrevState(tCQCGKit::EConnStates::SrvOffline)
    , m_cqcdcThis(cqcdcThis)
    , m_thrPoll
      (
        facCIDLib().strNextThreadName(strDriverClass + TString(L"Poll"))
        , TMemberFunc<TCQCDriverClient>(this, &TCQCDriverClient::ePollThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TCQCDriverClient::~TCQCDriverClient()
{
}


// ---------------------------------------------------------------------------
//  TCQCDriverClient: Public, non-virtual methods
// ---------------------------------------------------------------------------

const TCQCDriverObjCfg& TCQCDriverClient::cqcdcThis() const
{
    return m_cqcdcThis;
}


const TCQCUserCtx& TCQCDriverClient::cuctxToUse() const
{
    return m_cuctxToUse;
}


tCIDLib::TVoid
TCQCDriverClient::CreateClDrvWnd(const  TWindow&            wndParent
                                , const TArea&              areaInit
                                , const tCIDCtrls::TWndId   widToUse)
{
    //
    //  And now call down to create the window. We are initially invisible and our parent
    //  will show us once he's got us sized/positioned appropriately.
    //
    TWindow::CreateWnd
    (
        wndParent.hwndSafe()
        , kCIDCtrls::pszCustClass
        , L""
        , areaInit
        , tCIDCtrls::EWndStyles::ClippingChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , widToUse
    );
}


tCQCGKit::EConnStates TCQCDriverClient::eConnState() const
{
    return m_eConnState;
}


// Just a convenience wrapper that we pass on to the user context
tCQCKit::EUserRoles TCQCDriverClient::eUserRole() const
{
    return m_cuctxToUse.eUserRole();
}


tCQCKit::TCQCSrvProxy& TCQCDriverClient::orbcServer()
{
    return m_orbcServer;
}


TMutex* TCQCDriverClient::pmtxSync() const
{
    return &m_mtxSync;
}


const TCQCSecToken& TCQCDriverClient::sectUser() const
{
    return m_cuctxToUse.sectUser();
}


const TString& TCQCDriverClient::strMoniker() const
{
    return m_cqcdcThis.strMoniker();
}


// The client program will call this after the windows are all created and ready
tCIDLib::TVoid TCQCDriverClient::StartDriver()
{
    // If we have already been run and stopped, then that's an error
    if (m_bCleanupDone)
    {
        facCQCGKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kIGKitErrs::errcDrv_AlreadyStopped
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Already
            , m_cqcdcThis.strMoniker()
        );
    }

    // Start the poll thread
    m_thrPoll.Start();

    //
    //  Start the update timer. Initially we set a kind of long time, to
    //  give the client time to come up and get displayed. The first time
    //  the timer gets called, he will reset the period to the appropriate
    //  period.
    //
    //  <TBD> Later on, we can use the m_eActivityLevel setting to adjust
    //  these to optimize overhead to match the rate that the device's data
    //  is likely to change.
    //
    m_tmidUpdate = tmidStartTimer(3000);
}


//
//  The client program will call this top stop the driver (before it destroys our
//  parent window of course!)
//
tCIDLib::TVoid TCQCDriverClient::StopDriver()
{
    // Stop our update time if running
    try
    {
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCGKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_StopTimer
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }

    catch(...)
    {
        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_StopTimer
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }

    // Stop the poll thread
    try
    {
        if (m_thrPoll.bIsRunning())
        {
            m_thrPoll.ReqShutdownSync(10000);
            m_thrPoll.eWaitForDeath(5000);
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCGKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_StopClientDrvPollThr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }

    catch(...)
    {
        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_StopClientDrvPollThr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }

    // Let the derived class do its cleanup
    try
    {
        if (!m_bCleanupDone)
        {
            m_bCleanupDone = kCIDLib::True;
            DoCleanup();
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // Drop the server admin client proxy, if we have one
    try
    {
        m_orbcServer.DropRef();
    }

    catch(TError& errToCatch)
    {
        if (facCQCGKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_CleanupProxy
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }

    catch(...)
    {
        if (facCQCGKit().bLogFailures())
        {
            facCQCGKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kGKitMsgs::midStatus_CleanupProxy
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Internal
                , m_cqcdcThis.strMoniker()
            );
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCDriverClient: Protected, inherited methods
// ---------------------------------------------------------------------------

// Do it all in the paint, where we are clipped to our children
tCIDLib::TBoolean TCQCDriverClient::bEraseBgn(TGraphDrawDev&)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCDriverClient::bPaint(TGraphDrawDev& gdevToUse, const TArea& areaUpdate)
{
    gdevToUse.Fill(areaUpdate, rgbBgnFill());
    return kCIDLib::True;
}


tCIDLib::TVoid TCQCDriverClient::Destroyed()
{
    // Just in case...
    StopDriver();

    TParent::Destroyed();
}


tCIDLib::TVoid TCQCDriverClient::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // If the first timer, reset the period now
    if (m_bFirstTimer)
    {
        m_bFirstTimer = kCIDLib::False;
        ResetTimer(m_tmidUpdate, 1000);
    }

    //
    //  We have to lock out the poll thread during this. Use the form that let's us
    //  conditionally lock. If the poll thread should hang up for some reason, we
    //  don't want to lock up the interface. We use a short timeout so that, if the
    //  poll thread is straining because the remote servers are down, we won't make
    //  the interface overly spongy.
    //
    TLocker lockrSync(&m_mtxSync, kCIDLib::False);
    if (!lockrSync.bLock(1))
        return;

    try
    {
        //
        //  If we have no changes, then just return, cause there is nothing to do.
        //  Else, reset the counter and let the derived class update itself with this
        //  new data.
        //
        if (!m_c4PollChanges)
            return;
        m_c4PollChanges = 0;

        //
        //  See if the connection state has changed from the last one the bgn thread
        //  left us.
        //
        if (m_eConnState != m_ePrevState)
        {
            //
            //  Remember the previous state, then get it into sync with the current state.
            //  We want to do this before we make any call, in case it throws.
            //
            const tCQCGKit::EConnStates eOldState = m_ePrevState;
            m_ePrevState = m_eConnState;

            //
            //  If we have connected or disconnected, then let the derived class know so
            //  that he can update current status display stuff.
            //
            if ((eOldState != tCQCGKit::EConnStates::Connected)
            &&  (m_eConnState == tCQCGKit::EConnStates::Connected))
            {
                // Tell the parent tab window that we are not in error state
                TTabWindow* pwndPar = dynamic_cast<TTabWindow*>(pwndParent());
                if (pwndPar)
                    pwndPar->SetState(tCIDCtrls::ETabStates::Normal, kCIDLib::True);

                Connected();
            }
            else if ((eOldState == tCQCGKit::EConnStates::Connected)
                 &&  (m_eConnState != tCQCGKit::EConnStates::Connected))
            {
                // Tell the parent tab window that we are in error state
                TTabWindow* pwndPar = dynamic_cast<TTabWindow*>(pwndParent());
                if (pwndPar)
                    pwndPar->SetState(tCIDCtrls::ETabStates::Errors, kCIDLib::True);

                LostConnection();
            }
        }
         else if (m_eConnState == tCQCGKit::EConnStates::Connected)
        {
            //
            //  Its just a normal scenario. We are connected so let the driver display
            //  new data if it has any, left for it by the polling thread.
            //
            UpdateDisplayData();
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  Eat it, since there isn't much we can do, but log a message if
        //  debugging is on.
        //
        #if CID_DEBUG_ON
        if (facCQCGKit().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        #endif
    }

    catch(...)
    {
        //
        //  Eat it, since there isn't much we can do, but log a message if
        //  debugging is on.
        //
        #if CID_DEBUG_ON
        #endif
    }
}


// ---------------------------------------------------------------------------
//  Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method is called from the poll thread when its time to do another poll cycle.
//  We just break it out in order to keep that method from getting messy and bloated.
//
tCIDLib::TVoid TCQCDriverClient::DoPollCycle()
{
    // Lock during this
    TLocker lockrSync(&m_mtxSync);

    //
    //  Ok, we get called here from the poll thread (and once from the main thread on
    //  startup to kickstart things.) We are responsible for either getting ourself up
    //  to the 'connected' state, or if already there for polling data and backing our
    //  state off if something goes awry.
    //
    try
    {
        //
        //  We'll start with being completely disconnected from the server, in which case
        //  we need to try to get a client proxy. If this fails, it'll throw and we'll
        //  catch below and stay off line. If it works ok, then update to assuming the
        //  device is offline, and indicate that state changes have happened.
        //
        if (m_eConnState == tCQCGKit::EConnStates::SrvOffline)
        {
            // Drop any old reference if we have one
            if (m_orbcServer.pobjData())
                m_orbcServer.DropRef();

            //
            //  Try to get a new client proxy. If it works, then move our state up.
            //  Catch exceptions since we are going to get them if the server isn't up,
            //  and we just want to eat those and stay offline.
            //
            try
            {
                m_orbcServer = facCQCKit().orbcCQCSrvAdminProxy(m_cqcdcThis.strMoniker());
                m_eConnState = tCQCGKit::EConnStates::DevOffline;
                m_c4PollChanges++;
            }

            catch(TError& errToCatch)
            {
                // If it anything besides the driver not being loaded, rethrow
                if (!errToCatch.bCheckEvent(facCQCKit().strName()
                                            , kKitErrs::errcRem_DrvNotFound))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }
            }
        }

        //
        //  Ok, we are now connected to the server, but it might not be connected to
        //  its device. So if we last saw it offline, let's ask it for status. If it
        //  now shows connected, we get tell the driver to do one time on connect
        //  stuff, and move to connected state. We bump our changes counter to make
        //  sure the GUI thread updates in responses.
        //
        if (m_eConnState == tCQCGKit::EConnStates::DevOffline)
        {
            tCIDLib::TCard4         c4ThreadId;
            tCQCKit::EDrvStates     eState;
            tCQCKit::EVerboseLvls   eVerbose;
            m_orbcServer->QueryDriverState
            (
                m_cqcdcThis.strMoniker(), eState, eVerbose, c4ThreadId
            );
            if (eState == tCQCKit::EDrvStates::Connected)
            {
                //
                //  Ask the derived class to get any 'one time' info that it gets from
                //  it's server driver upon connect. If the driver class doesn't throw
                //  an error, we'll update our status.
                //
                if (bGetConnectData(m_orbcServer))
                {
                    m_eConnState = tCQCGKit::EConnStates::Connected;
                    m_c4PollChanges++;
                }
            }
        }

        //
        //  And if we are current fully connected, we just want to ask the derived class
        //  to poll for data. Its return value tells us if new data has showed up, or if
        //  he cannot talk to his device or server, in which case we update the status
        //  accordingly.
        //
        if (m_eConnState == tCQCGKit::EConnStates::Connected)
        {
            if (bDoPoll(m_orbcServer))
                m_c4PollChanges++;
        }
    }

    catch(const TError& errToCatch)
    {
        if (!m_orbcServer.pobjData() || m_orbcServer->bCheckForLostConnection())
        {
            // We lost connection to the server
            m_eConnState = tCQCGKit::EConnStates::SrvOffline;
            m_c4PollChanges++;
            m_orbcServer.DropRef();
        }
         else if (errToCatch.bCheckEvent(facCQCKit().strName(), kKitErrs::errcDrv_NotOnline)
              ||  errToCatch.bCheckEvent(facCQCKit().strName(), kKitErrs::errcDrv_NotFound))
        {
            if (m_eConnState != tCQCGKit::EConnStates::DevOffline)
            {
                // The device isn't there for some reason
                m_eConnState = tCQCGKit::EConnStates::DevOffline;
                m_c4PollChanges++;
            }
        }
         else if (errToCatch.bCheckEvent(facCIDOrbUC().strName()
                                        , kOrbUCErrs::errcSrv_NSNotFound))
        {
            // We couldn't find the server in the name server
            if (m_eConnState != tCQCGKit::EConnStates::SrvOffline)
            {
                m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                m_c4PollChanges++;
            }
        }
         else
        {
            // Not something obvious, so log it if logging warnings
            if (facCQCGKit().bLogWarnings() && !errToCatch.bLogged())
                TModule::LogEventObj(errToCatch);

            if (m_eConnState == tCQCGKit::EConnStates::Connected)
            {
                // We think we are connected, so try to make a call to the driver
                try
                {
                    tCIDLib::TCard4 c4Tmp;
                    m_orbcServer->c4QueryDriverId(m_cqcdcThis.strMoniker(), c4Tmp);

                    //
                    //  It worked, so just assume it was some dumb error in the
                    //  driver's GUI code.
                    //
                }

                catch(const TError& errToCatch)
                {
                    //
                    //  If it's not found, then we just lost the device,
                    //  else assume we lost the server.
                    //
                    if (errToCatch.eClass() == tCIDLib::EErrClasses::NotFound)
                        m_eConnState = tCQCGKit::EConnStates::DevOffline;
                    else
                        m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                    m_c4PollChanges++;
                }

                catch(...)
                {
                    // Assume we lost the connection
                    m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                    m_c4PollChanges++;
                }
            }
            else if (m_eConnState != tCQCGKit::EConnStates::SrvOffline)
            {
                m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                m_c4PollChanges++;
            }
        }
    }

    catch(...)
    {
        //
        //  We don't have an exception to go by, so we have to just manually
        //  try the link and see if this happened becasue we lost the
        //  connection or not. We'll just try to get the driver id, which
        //  is a simple call that will check for out driver being there.
        //
        //  If we aren't connected, just assume the worst case and say the
        //  server is offline and force us to completely reconnect.
        //
        if (m_eConnState == tCQCGKit::EConnStates::Connected)
        {
            try
            {
                tCIDLib::TCard4 c4Tmp;
                m_orbcServer->c4QueryDriverId(m_cqcdcThis.strMoniker(), c4Tmp);

                //
                //  It worked, so just assume it was some dumb error in the
                //  driver's GUI code.
                //
            }

            catch(const TError& errToCatch)
            {
                //
                //  If it's not found, then we just lost the device,
                //  else assume we lost the server.
                //
                if (errToCatch.eClass() == tCIDLib::EErrClasses::NotFound)
                    m_eConnState = tCQCGKit::EConnStates::DevOffline;
                else
                    m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                m_c4PollChanges++;
            }

            catch(...)
            {
                // Assume we lost the connection
                m_eConnState = tCQCGKit::EConnStates::SrvOffline;
                m_c4PollChanges++;
            }
        }
         else if (m_eConnState != tCQCGKit::EConnStates::SrvOffline)
        {
            m_eConnState = tCQCGKit::EConnStates::SrvOffline;
            m_c4PollChanges++;
        }
    }
}


//
//  This is the polling thread. This is started up when the window is created,
//  and runs until the window is closed. It keeps up with our current connect
//  state and does what is necessary to run the show wrt to staying connected
//  to the server and polling it for new data.
//
tCIDLib::EExitCodes
TCQCDriverClient::ePollThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the thread that started us go now
    thrThis.Sync();

    //
    //  To avoid setting a try block on every round, we use a double loop.
    //  That lets us catch exceptions and restart.
    //
    tCIDLib::TBoolean   bExit = kCIDLib::False;
    while (!bExit)
    {
        try
        {
            while (!bExit)
            {
                if (m_eConnState == tCQCGKit::EConnStates::Connected)
                {
                    if (!thrThis.bSleep(2000))
                    {
                        bExit = kCIDLib::True;
                        continue;
                    }
                }
                 else
                {
                    if (!thrThis.bSleep(4000))
                    {
                        bExit = kCIDLib::True;
                        continue;
                    }
                }

                //
                //  Call our method that does a standard poll cycle. According
                //  to our current connect status, this guy does what is
                //  necessary to keep things moving along and will update our
                //  state accordingly.
                //
                DoPollCycle();
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCGKit().bLogWarnings())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
            if (facCQCGKit().bLogFailures())
            {
                facCQCGKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kGKitMsgs::midStatus_PollThreadExcept
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                    , m_cqcdcThis.strMoniker()
                );
            }
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


