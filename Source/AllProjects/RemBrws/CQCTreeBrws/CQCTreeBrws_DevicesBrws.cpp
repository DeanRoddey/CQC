//
// FILE NAME: CQCTreeBrws_DevicesBrws.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/27/2015
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
//  This file implements the browser for the /Devices part of the browser hierarchy.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_ChangeDrvSrvDlg_.hpp"
#include    "CQCTreeBrws_DevicesBrws_.hpp"
#include    "CQCTreeBrws_SelSrvDrvDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCDevicesBrws,TCQCTreeBrwsIntf)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCTreeBrws_DevicesBrws
    {
        //
        //  If the user right clicks on our root scope, then we provide him with a
        //  New-> submenu that has the available hosts to load a driver on. We number
        //  those sub-menu items starting at this value.
        //
        //  BE SURE these don't match any of the values in our Devices menus in the
        //  resource file.
        //
        constexpr tCIDLib::TCard4 c4FirstHostId = 9999;

        //
        //  These are the strings we post in the change notifications to the browser
        //  window, which will send them back to us in the GUI thread for us to handle.
        //
        const TString   strChange_ResetServer(L"ResetServer");
        const TString   strChange_State(L"StateChange");
    }
};



// ---------------------------------------------------------------------------
//   CLASS: TDevBrwsDrvInfo
//  PREFIX: dbdi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDevBrwsDrvInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TDevBrwsDrvInfo::strMonKey(const TDevBrwsDrvInfo& dbdiSrc)
{
    return dbdiSrc.m_strMoniker;
}

const TString& TDevBrwsDrvInfo::strPathKey(const TDevBrwsDrvInfo& dbdiSrc)
{
    return dbdiSrc.m_strBrwsPath;
}


// ---------------------------------------------------------------------------
//  TDevBrwsDrvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TDevBrwsDrvInfo::TDevBrwsDrvInfo(const  TString&            strHost
                                , const TString&            strMoniker
                                , const tCIDLib::TBoolean   bClientDriver) :

    m_bClientDriver(bClientDriver)
    , m_c4DriverId(kCIDLib::c4MaxCard)
    , m_c4Version(0)
    , m_eState(tCQCKit::EDrvStates::NotLoaded)
    , m_strMoniker(strMoniker)
{
    m_strBrwsPath = kCQCRemBrws::strPath_Devices;
    m_strBrwsPath.Append(kCIDLib::chForwardSlash);
    m_strBrwsPath.Append(strHost);
    m_strBrwsPath.Append(kCIDLib::chForwardSlash);
    m_strBrwsPath.Append(strMoniker);
}

TDevBrwsDrvInfo::~TDevBrwsDrvInfo()
{
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TDevBrwsDrvInfo::bClientDriver() const
{
    return m_bClientDriver;
}


//
//  The bgn thread calls this to update us with the latest driver state he saw
//  during polling. He will lock before calling this. We return true if the new
//  state is different from the previous.
//
tCIDLib::TBoolean TDevBrwsDrvInfo::bSetState(const tCQCKit::EDrvStates eToSet)
{
    const tCIDLib::TBoolean bRet(m_eState != eToSet);
    m_eState = eToSet;
    return bRet;
}


// Get or set the driver id for this driver
tCIDLib::TCard4 TDevBrwsDrvInfo::c4DriverId() const
{
    return m_c4DriverId;
}

tCIDLib::TCard4 TDevBrwsDrvInfo::c4DriverId(const tCIDLib::TCard4 c4ToSet)
{
    m_c4DriverId = c4ToSet;
    return m_c4DriverId;
}


// It's assumed that the caller has locked the data before calling this.
tCQCKit::EDrvStates TDevBrwsDrvInfo::eState() const
{
    return m_eState;
}


// Never changes so no need to lock
const TString& TDevBrwsDrvInfo::strBrwsPath() const
{
    return m_strBrwsPath;
}


// Never changes so no need to lock
const TString& TDevBrwsDrvInfo::strMoniker() const
{
    return m_strMoniker;
}




// ---------------------------------------------------------------------------
//   CLASS: TDevBrwsSrvInfo
//  PREFIX: dbsi
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TDevBrwsSrvInfo: Public, static methods
// ---------------------------------------------------------------------------
const TString& TDevBrwsSrvInfo::strKey(const TDevBrwsSrvInfo& dbsiSrc)
{
    return dbsiSrc.m_strBrwsPath;
}


// ---------------------------------------------------------------------------
//  TDevBrwsSrvInfo: Constructors and Destructor
// ---------------------------------------------------------------------------
TDevBrwsSrvInfo::TDevBrwsSrvInfo(const  TString&                strHost
                                ,       TCQCTreeBrowser* const  pwndTree
                                , const TCQCUserCtx&            cuctxUser) :

    m_bForceRefresh(kCIDLib::False)
    , m_c4DriverListId(0)
    , m_c4Version(0)
    , m_colDrvList
      (
        tCIDLib::EAdoptOpts::Adopt
        , 109
        , TStringKeyOps()
        , &TDevBrwsDrvInfo::strPathKey
      )
    , m_colDrvListMon
      (
        tCIDLib::EAdoptOpts::NoAdopt
        , 109
        , TStringKeyOps()
        , &TDevBrwsDrvInfo::strMonKey
      )
    , m_ePollState(EPollStates::SrvConn)
    , m_porbcSrv(nullptr)
    , m_pwndTree(pwndTree)
    , m_cuctxUser(cuctxUser)
    , m_strHost(strHost)
    , m_thrMon
      (
        TString(L"CQCDevBrwsMonThread_") + strHost
        , TMemberFunc<TDevBrwsSrvInfo>(this, &TDevBrwsSrvInfo::eMonThread)
      )
{
    m_strBrwsPath = kCQCRemBrws::strPath_Devices;
    m_strBrwsPath.Append(kCIDLib::chForwardSlash);
    m_strBrwsPath.Append(strHost);
}

TDevBrwsSrvInfo::~TDevBrwsSrvInfo()
{
}


// ---------------------------------------------------------------------------
//  TDevBrwsSrvInfo: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The GUI thread calls this to make us refresh when it knows the config has
//  changed. We just set a flag letting the bgn thread know we want to force this
//  to happen.
//
tCIDLib::TVoid TDevBrwsSrvInfo::ForceRefresh()
{
    m_bForceRefresh = kCIDLib::True;
}


// Find the driver info object for the passed driver id
TDevBrwsDrvInfo* TDevBrwsSrvInfo::pdbdiFindById(const tCIDLib::TCard4 c4ToFind)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxLock);

    TDrvList::TNCCursor cursMons(&m_colDrvList);
    if (cursMons.bReset())
    {
        do
        {
            if (cursMons.objRCur().c4DriverId() == c4ToFind)
                return &cursMons.objWCur();
        }   while(cursMons.bNext());
    }
    return nullptr;
}


// Find the driver info object for the passed moniker
TDevBrwsDrvInfo* TDevBrwsSrvInfo::pdbdiFindByMoniker(const  TString& strMoniker)
{
    // Lock while we do this
    TLocker lockrSync(&m_mtxLock);
    return m_colDrvListMon.pobjFindByKey(strMoniker, kCIDLib::False);
}


//
//  We provide indirect access to our list of monikers, which is needed in order to
//  make sure there are no dups during new driver creation.
//
tCIDLib::TVoid
TDevBrwsSrvInfo::QueryMonikers(         tCIDLib::TStrList&  colToFill
                                , const tCIDLib::TBoolean   bAppend) const
{
    if (!bAppend)
        colToFill.RemoveAll();

    // Lock while we do this
    TLocker lockrSync(&m_mtxLock);

    TDrvList::TCursor cursMons(&m_colDrvList);
    if (cursMons.bReset())
    {
        do
        {
            colToFill.objAdd(cursMons.objRCur().strMoniker());
        }   while(cursMons.bNext());
    }
}


// Never changes, so no locking required
const TString& TDevBrwsSrvInfo::strBrwsPath() const
{
    return m_strBrwsPath;
}


// Never changes so no locking required
const TString& TDevBrwsSrvInfo::strHost() const
{
    return m_strHost;
}


// The browser object calls this to start our monitor thread after we are set up
tCIDLib::TVoid TDevBrwsSrvInfo::Start()
{
    CIDAssert(!m_thrMon.bIsRunning(), L"The browser server thread is already started");
    m_thrMon.Start();
}


tCIDLib::TVoid TDevBrwsSrvInfo::StartShutdown()
{
    // If the thread is running, do a non-blocking shutdown request
    if (m_thrMon.bIsRunning())
    {
        try
        {
            m_thrMon.ReqShutdownNoSync();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


tCIDLib::TVoid TDevBrwsSrvInfo::WaitForDeath()
{
    // If the thread is still running, wait for it to die
    if (m_thrMon.bIsRunning())
    {
        try
        {
            m_thrMon.eWaitForDeath(8000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TDevBrwsSrvInfo: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TDevBrwsSrvInfo::bCheckConnection()
{
    // If we have no client proxy, then clearly nothing to do. Say not connected
    if (!m_porbcSrv)
        return kCIDLib::False;

    // See if we are disconnectd. If so, clean it up
    if (m_porbcSrv->bCheckForLostConnection())
    {
        try
        {
            delete m_porbcSrv;
        }

        catch(TError& errToCatch)
        {
            if (facCQCTreeBrws().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }

        m_porbcSrv = nullptr;

        //
        //  Set all of our drivers to offline and post a change notification if
        //  any of them changed state because of this.
        //
        SetAllStates(tCQCKit::EDrvStates::NotLoaded);

        // Say not connected
        return kCIDLib::False;
    }

    // We are connected
    return kCIDLib::True;
}


//
//  This is called if we see that the configuration has been changed behind our
//  back. We ask the data server for the current list of drivers for our host. If
//  it works we clear the refresh flag and post to the GUI thread that it needs to
//  update. We clear our list and load up new driver objects for the new monikers.
//
//  Until we return true, we will stay in the config sync state and the poll thread
//  will continue to come in here periodically to try to get the config.
//
tCIDLib::TBoolean TDevBrwsSrvInfo::bRefreshCfg()
{
    // Query the monikers for all of the drivers configured for our host
    try
    {
        // Get an installation server proxy
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Version;
        tCIDLib::TCardList fcolVers;
        tCIDLib::TBoolList fcolClDrvs;
        tCIDLib::TStrList colMons;
        if (orbcIS->bDrvCfgQueryMonList(c4Version, m_strHost, colMons, fcolVers, fcolClDrvs, m_cuctxUser.sectUser()))
        {
            // Lock while we do this
            TLocker lockrSync(&m_mtxLock);

            // Store the version we got above
            m_c4Version = c4Version;

            // Flush our driver lists
            m_colDrvList.RemoveAll();
            m_colDrvListMon.RemoveAll();

            // Post a change to the GUI thread to clear the list
            TString strPath = kCQCRemBrws::strPath_Devices;
            strPath.Append(kCIDLib::chForwardSlash);
            strPath.Append(m_strHost);
            m_pwndTree->QueueChange
            (
                strPath, CQCTreeBrws_DevicesBrws::strChange_ResetServer, TString::strEmpty()
            );

            //
            //  And now queue up an initial item state change for each driver, which
            //  will get them into the list, initially marked offline.
            //
            strPath.Append(kCIDLib::chForwardSlash);
            const tCIDLib::TCard4 c4BaseLen = strPath.c4Length();
            const TString strData(tCQCKit::strAltXlatEDrvStates(tCQCKit::EDrvStates::NotLoaded));

            const tCIDLib::TCard4 c4Count = colMons.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TString& strCurMon = colMons[c4Index];

                strPath.CapAt(c4BaseLen);
                strPath.Append(strCurMon);

                // Add an item to our driver list
                TDevBrwsDrvInfo* pdbdiNew = new TDevBrwsDrvInfo
                (
                    m_strHost, strCurMon, fcolClDrvs[c4Index]
                );

                m_colDrvList.Add(pdbdiNew);
                m_colDrvListMon.Add(pdbdiNew);

                // And queue up a state change for it
                m_pwndTree->QueueChange
                (
                    strPath, CQCTreeBrws_DevicesBrws::strChange_State, strData
                );
            }

            //
            //  Now tell the browser window to handle the changes, which it does
            //  async of course.
            //
            m_pwndTree->ProcessChanges();
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCTreeBrws().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  This is called regularly from our bgn thread to. We poll our CQCServer to
//  watch for changes in driver states.
//
//  We don't need to sync here since we aren't going to modify our driver list
//  here.
//
tCIDLib::TVoid TDevBrwsSrvInfo::DoPoll(const tCIDLib::TBoolean bFirstTime)
{
    //
    //  If the first time, we need to query our driver id list and the monikers/ids
    //  of the actual drivers on our host. We then go through our driver objects and
    //  store the ids on those that we got, the ids were defaulted to maxcard when
    //  the driver objects were created.
    //
    if (bFirstTime)
    {
        m_fcolIdPoll.RemoveAll();
        tCIDLib::TStrList colMons;
        m_c4DriverListId = m_porbcSrv->c4QueryDriverIdList2(colMons, m_fcolIdPoll, m_cuctxUser.sectUser());

        const tCIDLib::TCard4 c4QCount = colMons.c4ElemCount();
        for (tCIDLib::TCard4 c4QIndex = 0; c4QIndex < c4QCount; c4QIndex++)
        {
            TDevBrwsDrvInfo* pdbdiCur = pdbdiFindByMoniker(colMons[c4QIndex]);
            if (pdbdiCur)
                pdbdiCur->c4DriverId(m_fcolIdPoll[c4QIndex]);
        }
    }

    // Now we can do a poll
    m_fcolIdPoll.RemoveAll();
    m_fcolStatePoll.RemoveAll();
    const tCIDLib::TBoolean bValid = m_porbcSrv->bQueryDriverStates
    (
        m_c4DriverListId, m_fcolIdPoll, m_fcolStatePoll
    );

    //
    //  If the list changes and this is not the first poll, then that means that
    //  this server has been modified by some other client and we need to sync
    //  ourself back up again. We just set our state back to cfg sync state.
    //
    if (!bValid && !bFirstTime)
    {
        m_ePollState = EPollStates::SyncCfg;
        return;
    }

    //
    //  Get a copy of all of our driver monikers. As we find each one in the state
    //  list, we remove it. The rest are those that are still connected. We only queue
    //  up those that have a driver id set, the others being ones that are configured
    //  but not actually running on the host. Those we never want to update.
    //
    tCIDLib::TStrHashSet colGood(109, TStringKeyOps());
    TDrvList::TCursor cursDrvs(&m_colDrvList);
    if (cursDrvs.bReset())
    {
        do
        {
            const TDevBrwsDrvInfo& dbdiCur = cursDrvs.objRCur();
            if (dbdiCur.c4DriverId() != kCIDLib::c4MaxCard)
                colGood.objAdd(dbdiCur.strMoniker());
        }   while (cursDrvs.bNext());
    }

    // Let's process the values
    tCIDLib::TCard4 c4Posted = 0;
    const tCIDLib::TCard4 c4ValCnt = m_fcolIdPoll.c4ElemCount();
    if (c4ValCnt)
    {
        // Now go through the returned values
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ValCnt; c4Index++)
        {
            TDevBrwsDrvInfo* pdbdiCur = pdbdiFindById(m_fcolIdPoll[c4Index]);

            // If we found it, take its reported state if it's different
            if (pdbdiCur)
            {
                if (pdbdiCur->bSetState(m_fcolStatePoll[c4Index]))
                {
                    QueueDrvStateChange(pdbdiCur->strBrwsPath(), m_fcolStatePoll[c4Index]);
                    c4Posted++;
                }

                // Remove this one from the good list we created above
                colGood.bRemove(pdbdiCur->strMoniker());
            }
        }
    }

    // Now go through the remaining good ones and queue up connected states for those
    tCIDLib::TStrHashSet::TCursor cursGood(&colGood);
    if (cursGood.bReset())
    {
        do
        {
            TDevBrwsDrvInfo* pdbdiCur = pdbdiFindByMoniker(cursGood.objRCur());
            if (pdbdiCur->bSetState(tCQCKit::EDrvStates::Connected))
            {
                QueueDrvStateChange(pdbdiCur->strBrwsPath(), tCQCKit::EDrvStates::Connected);
                c4Posted++;
            }
        }   while (cursGood.bNext());
    }

    // Tell the browser to process any queued changes
    if (c4Posted)
        m_pwndTree->ProcessChanges();
}


//
//  This is our monitoring thread. It's only started if the user expands out the
//  devices part of the tree.
//
tCIDLib::EExitCodes TDevBrwsSrvInfo::eMonThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    tCIDLib::TCard4 c4SleepTime = 0;
    m_ePollState = EPollStates::SyncCfg;
    while (kCIDLib::True)
    {
        //
        //  If the GUI thread has set our forced refresh flag, then clear that flag
        //  and force us back to sync config state.
        //
        if (m_bForceRefresh)
        {
            m_bForceRefresh = kCIDLib::False;
            m_ePollState = EPollStates::SyncCfg;
        }


        // According to the poll state do what is appropriate
        if (m_ePollState == EPollStates::SyncCfg)
        {
            //
            //  We need to get the configured drivers for our server and create our
            //  driver info objects.
            //
            if (bRefreshCfg())
            {
                //
                //  We are good, so let's more forward. We don't sleep here, we want
                //  to move to the next phase immediately. If we already have our
                //  CQCServer client proxy, we move to InitData. Else we move to
                //  SrvConn to get connected. We may already be if we are here
                //  because we needed to resync.
                //
                if (m_porbcSrv)
                    m_ePollState = EPollStates::InitData;
                else
                    m_ePollState = EPollStates::SrvConn;

                //
                //  Even if we aren't already connected, reset these so that we
                //  get new data from CQCServer.
                //
                m_c4DriverListId = 0;

                // No sleep here, go straight to the new state
                c4SleepTime = 0;
            }
             else
            {
                // Sleep a while and we'll try again
                c4SleepTime = 5000;
            }
        }
         else if (m_ePollState == EPollStates::SrvConn)
        {
            // We need to get connected to our associated CQCServer.
            try
            {
                m_porbcSrv = facCQCKit().porbcCQCSrvAdminProxy(m_strHost, 2000);

                // Reset stuff related to CQCServer polling
                m_c4DriverListId = 0;
                m_ePollState = EPollStates::InitData;

                // Don't sleep in this case, move straight forward
                c4SleepTime = 0;
            }

            catch(TError& errToCatch)
            {
                if (facCQCTreeBrws().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                // Back off for a while before we try again
                c4SleepTime = 5000;
            }

            catch(...)
            {
                // Back off for a while before we try again
                c4SleepTime = 5000;
            }

        }
         else if ((m_ePollState == EPollStates::InitData)
              ||  (m_ePollState == EPollStates::Poll))
        {
            // We have what we need so we need to poll for content
            try
            {
                DoPoll(m_ePollState == EPollStates::InitData);

                //
                //  If in initialize data state, then move to poll. Don't just update
                //  the state to Poll since DoPoll might have downgraded us if it saw
                //  a change in the driver list.
                //
                if (m_ePollState == EPollStates::InitData)
                    m_ePollState = EPollStates::Poll;

                // Sleep a bit till the next poll
                c4SleepTime = 5000;
            }

            catch(TError& errToCatch)
            {
                if (facCQCTreeBrws().bLogWarnings())
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TModule::LogEventObj(errToCatch);
                }

                if (!bCheckConnection())
                {
                    m_ePollState = EPollStates::SrvConn;
                    c4SleepTime = 10000;
                }
                 else
                {
                    c4SleepTime = 6000;
                }
            }

            catch(...)
            {
                if (!bCheckConnection())
                {
                    m_ePollState = EPollStates::SrvConn;
                    c4SleepTime = 10000;
                }
                 else
                {
                    c4SleepTime = 6000;
                }
            }
        }

        // Sleep for the indicated time, breaking out if asked to shut down
        if (c4SleepTime)
        {
            if (!thrThis.bSleep(c4SleepTime))
                break;
        }
    }

    // Clean up the client proxy if created
    if (m_porbcSrv)
    {
        try
        {
            delete m_porbcSrv;
        }

        catch(...)
        {
        }

        m_porbcSrv = nullptr;
    }

    return tCIDLib::EExitCodes::Normal;
}


//
//  A helper to queue up an item state change notification to the browser window
//  This is public so that our server objects can call it.
//
tCIDLib::TVoid
TDevBrwsSrvInfo::QueueDrvStateChange(const  TString&            strPath
                                    , const tCQCKit::EDrvStates eState)
{
    // Based on the driver state, send an item state that the browser understands
    m_pwndTree->QueueChange
    (
        strPath
        , CQCTreeBrws_DevicesBrws::strChange_State
        , tCQCKit::strAltXlatEDrvStates(eState)
    );
}


//
//  Sets all of our drivers to a specific driver state
//
tCIDLib::TVoid TDevBrwsSrvInfo::SetAllStates(const tCQCKit::EDrvStates eNew)
{
    TDrvList::TNCCursor cursDrvs(&m_colDrvList);
    if (cursDrvs.bReset())
    {
        // Get the data that we'll pass as the state for this state change notification.
        const TString& strData = tCQCKit::strAltXlatEDrvStates(eNew);

        do
        {
            if (cursDrvs.objWCur().bSetState(eNew))
            {
                m_pwndTree->QueueChange
                (
                    cursDrvs.objRCur().strBrwsPath()
                    , CQCTreeBrws_DevicesBrws::strChange_State
                    , strData
                );
            }
        }   while(cursDrvs.bNext());

        // Tell the browser to process any queued changes
        m_pwndTree->ProcessChanges();
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCDevicesBrws
//  PREFIX: rbrws
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDevicesBrws: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCDevicesBrws::TCQCDevicesBrws() :

    TCQCTreeBrwsIntf
    (
        kCQCRemBrws::strPath_Devices
        , kCQCRemBrws::strItem_Devices
        , facCQCTreeBrws().strMsg(kTBrwsMsgs::midTitle_DevBrowser)
    )
    , m_c4NSVersionNum(0)
    , m_colSrvList
      (
        tCIDLib::EAdoptOpts::Adopt
        , 109
        , TStringKeyOps()
        , &TDevBrwsSrvInfo::strKey
      )
{
}

TCQCDevicesBrws::~TCQCDevicesBrws()
{
}


// ---------------------------------------------------------------------------
//  TCQCDevicesBrws: Public, inherited methods
// ---------------------------------------------------------------------------

// Our browser object never accepts dropped files, so this won't get called
tCIDLib::TVoid
TCQCDevicesBrws::AcceptFiles(const  TString&
                            , const tCIDLib::TStrList&
                            , const tCIDLib::TStrList&)
{
    CIDAssert2(L"The devices browser should not be accepting files");
}


// We never accept dropped files in this part of the tree
tCIDLib::TBoolean
TCQCDevicesBrws::bAcceptsNew(const TString&, const tCIDLib::TStrHashSet&) const
{
    return kCIDLib::False;
}


//
//  The browser window calls us here if the user invokes a menu operation on the
//  tree window.
//
tCIDLib::TBoolean
TCQCDevicesBrws::bDoMenuAction(const TString& strPath, TTreeBrowseInfo& wnotToSend)
{
    // Get the area of this item, tell it to use just the text width
    TArea areaItem;
    wndBrowser().bQueryItemArea(strPath, areaItem, kCIDLib::False, kCIDLib::True);

    // Expand it a bit relative to the tree cell
    areaItem.Inflate(1, 1);

    // Get the center point of it and convert to screen coordinates
    TPoint pntAt;
    wndBrowser().ToScreenCoordinates(areaItem.pntCenter(), pntAt);


    // Create the menu and load it up from the resource
    TPopupMenu menuAction(L"/Devices Action");
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_Devices);

    //
    //  If they clicked on our root scope, then we remove the New action item, since
    //  we need to present a list of available hosts. Else, we remove the New2 sub
    //  menu, since the host is implied by the item clicked on. We also disable some
    //  stuff.
    //
    if (strPath == kCQCRemBrws::strPath_Devices)
    {
        //
        //  They didn't pick a specific host in our list, so we need to present them
        //  with a New-> sub-menu that has all of the available hosts in it.
        //
        menuAction.DeleteItem(kCQCTreeBrws::ridMenu_Devices_New);
        try
        {
            tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();

            //
            //  If the scope just doesn't exist, then probably no CQCServers, so don't call
            //  the query or it will throw. We just fall through with an empty list for the
            //  general msg below that no servers were found.
            //
            if (facCIDOrbUC().bNSScopeExists(orbcNS, TCQCSrvAdminClientProxy::strAdminScope))
            {
                orbcNS->bQueryScopeKeys
                (
                    m_c4NSVersionNum
                    , TCQCSrvAdminClientProxy::strAdminScope
                    , m_colLastNSList
                    , kCIDLib::True
                    , kCIDLib::True
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCGKit().ShowError
            (
                wndBrowser(), L"Could not get available CQC Driver Hosts", errToCatch
            );
            return kCIDLib::False;
        }

        // If there aren't any, then tell the user so
        if (m_colLastNSList.bIsEmpty())
        {
            TOkBox msgbEmpty(L"Could find any currently available CQC driver hosts");
            msgbEmpty.ShowIt(wndBrowser());
            return kCIDLib::False;
        }

        // And let's fill in the list
        TSubMenu menuNew(menuAction, kCQCTreeBrws::ridMenu_Devices_New2);
        const tCIDLib::TCard4 c4Count = m_colLastNSList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            menuNew.AddActionItem
            (
                kCIDLib::c4MaxCard
                , m_colLastNSList[c4Index]
                , m_colLastNSList[c4Index]
                , CQCTreeBrws_DevicesBrws::c4FirstHostId + c4Index
            );
        }

        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ChangeSrv, kCIDLib::False);
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ExportFlds, kCIDLib::False);
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ExportFldsVals, kCIDLib::False);
    }
     else
    {
        // Remove the New submenu, and we'll just use the New item
        menuAction.DeleteItem(kCQCTreeBrws::ridMenu_Devices_New2);
    }

    //
    //  May have been disabled above, but we also need to make sure it's disabled if the
    //  current user isn't and admin.
    //
    if (cuctxUser().eUserRole() != tCQCKit::EUserRoles::SystemAdmin)
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ChangeSrv, kCIDLib::False);

    // Set up the menu based on selection
    SetupMenu(strPath, menuAction);

    //
    //  If it's a driver, then let's set the pause/resume and verbosity items correctly and
    //  disable the change server option. If not a driver they will have been disabled above.
    //
    if (!wndBrowser().bIsScope(strPath))
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        // Get the host and moniker out of the path
        TString strHost;
        TString strMoniker;
        ParsePath(strPath, strHost, strMoniker);
        TDevBrwsDrvInfo* pdbdiTar =  pdbdiFindDriver(strHost, strMoniker);

        // Read the driver's config and get out the current pause state
        TCQCDriverObjCfg cqcdcSel;
        if (bReadDrvConfig(strHost, strMoniker, cqcdcSel))
        {
            if (cqcdcSel.bPaused())
                menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_Pause, kCIDLib::False);
            else
                menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_Resume, kCIDLib::False);
        }
         else
        {
            // Couldn't get the config, so be safe and disable both
            menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_Pause, kCIDLib::False);
            menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_Resume, kCIDLib::False);
        }

        if (!pdbdiTar->bClientDriver())
            menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ClientDrv, kCIDLib::False);

        // This is only legal for servers
        menuAction.SetItemEnable(kCQCTreeBrws::ridMenu_Devices_ChangeSrv, kCIDLib::False);

        // Get the verbosity of the driver
		// <TBD>
    }

    // Get a selection from the user
    const tCIDLib::TCard4 c4MenuCmd = menuAction.c4Process
    (
        wndBrowser(), pntAt, tCIDLib::EVJustify::Bottom
    );

    //
    //  If one of the dynamically loaded host names, then get the name of the
    //  selected item, which is the host name, then call the add driver helper.
    //
    //  Else just call the normal menu processing handler.
    //
    if ((c4MenuCmd >= CQCTreeBrws_DevicesBrws::c4FirstHostId)
    &&  (c4MenuCmd < CQCTreeBrws_DevicesBrws::c4FirstHostId + 100))
    {
        TString strHost;
        menuAction.QueryItemText(c4MenuCmd, strHost);
        AddDriver(strHost);
        return kCIDLib::True;
    }

    //
    //  If they made a choice, then we have to translate it to the standard action
    //  enum that the browser window will understand.
    //
    return bProcessMenuSelection(c4MenuCmd, strPath, wnotToSend);
}


// We don't use a persistent connection, so we just say yes
tCIDLib::TBoolean TCQCDevicesBrws::bIsConnected() const
{
    return kCIDLib::True;
}



//
//  Our own bDoMenuAction calls this if a selection is made. It is also called by the
//  browser window if an accelerator driven command is seen. That's why it's split out
//  so that we can avoid duplicating this code.
//
tCIDLib::TBoolean
TCQCDevicesBrws::bProcessMenuSelection( const   tCIDLib::TCard4     c4CmdId
                                        , const TString&            strPath
                                        ,       TTreeBrowseInfo&    wnotToSend)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    switch(c4CmdId)
    {
        case kCQCTreeBrws::ridMenu_Devices_ChangeSrv :
        {
            //
            //  This is only available on servers. We allow the user to change the name
            //  of the server that all of the contained drivers are associated with. So
            //  make sure it's a scope, else ignore it.
            //
            if (wndBrowser().bIsScope(strPath))
                ChangeServer(strPath);
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_ClientDrv :
        {
            // Make sure it's a driver, not a host (scope)
            if (!wndBrowser().bIsScope(strPath))
            {
                // Just post a notification
                wnotToSend = TTreeBrowseInfo
                (
                    tCQCTreeBrws::EEvents::ClientDrv
                    , strPath
                    , tCQCRemBrws::EDTypes::Driver
                    , wndBrowser()
                );
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_Delete :
        {
            // Make sure it's a driver, not a host (scope)
            if (!wndBrowser().bIsScope(strPath))
            {
                // Check with the containing application if it's ok to delete this
                const tCIDLib::TBoolean bIsOpen = bSendQuestionNot
                (
                    tCQCTreeBrws::EEvents::IsOpen
                    , strPath
                    , tCQCRemBrws::EDTypes::Driver
                    , kCIDLib::False
                );

                // If open, refuse, else unload it
                if (bIsOpen)
                {
                    TErrBox msgbOpen(facCQCTreeBrws().strMsg(kTBrwsErrs::errcBrws_ItemIsOpen));
                    msgbOpen.ShowIt(wndBrowser());
                }
                 else
                {
                    UnloadDriver(strPath);
                }
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_ExportFlds :
        case kCQCTreeBrws::ridMenu_Devices_ExportFldsVals :
        {
            ExportFlds(strPath, c4CmdId == kCQCTreeBrws::ridMenu_Devices_ExportFldsVals);
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_Info :
        {
            if (!wndBrowser().bIsScope(strPath))
            {
                // Invoke a standard driver info dialog box
                TString strMon;
                facCQCRemBrws().QueryNamePart(strPath, strMon);
                facCQCGKit().ShowDriverInfo(wndBrowser(), strMon, sectUser());
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_New :
        {
            //
            //  Has to be a scope, and has to be a host, not our root. If they add
            //  from the root that has to go through the menu and get a target host
            //  to install on. This only works when they are on a host, so that we
            //  know which one to target.
            //
            if (wndBrowser().bIsScope(strPath) && !strPath.bCompareI(strRootPath()))
            {
                //
                //  Break out the name part of the path, which is the target host, and
                //  call the add driver helper.
                //
                TString strHost;
                facCQCRemBrws().QueryNamePart(strPath, strHost);
                AddDriver(strHost);
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_Pause :
        case kCQCTreeBrws::ridMenu_Devices_Resume :
        {
            if (!wndBrowser().bIsScope(strPath))
                PauseResumeDriver(strPath, c4CmdId == kCQCTreeBrws::ridMenu_Devices_Pause);
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_Reconfig :
        {
            if (!wndBrowser().bIsScope(strPath))
                ReconfigureDriver(strPath);
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_Refresh :
        {
            //
            //  If it's our top level scope, then we first check to see if there are any
            //  new servers available that we've not got loaded. For any new ones, we
            //  load them. For any existing ones we tell them to refresh.
            //
            //  If they select a specific host, then we tell that one to refresh.
            //
            if (wndBrowser().bIsScope(strPath))
            {
                if (strPath.bCompareI(kCQCRemBrws::strPath_Devices))
                {
                    // Call a helper to do this
                    RefreshAll();
                }
                 else
                {
                    TString strHost;
                    facCQCRemBrws().QueryNamePart(strPath, strHost);
                    pdbsiFindSrv(strHost)->ForceRefresh();
                }
            }
            break;
        }

        case kCQCTreeBrws::ridMenu_Devices_View :
        {
            // Has to be a driver, not a host
            if (!wndBrowser().bIsScope(strPath))
            {
                //
                //  Post a notification indicating that the selected driver is to be
                //  'viewed'.
                //
                wnotToSend = TTreeBrowseInfo
                (
                    tCQCTreeBrws::EEvents::View, strPath, tCQCRemBrws::EDTypes::Driver, wndBrowser()
                );
            }
            break;
        }

        default :
            bRet = kCIDLib::False;
            break;
    };
    return bRet;
}


//
//  For our stuff, as long as it's not our top level path or a scope, it's fine to
//  report it all when double clicked. The client code will decide if it really
//  wants to deal with it and how, likely showing details on the driver. We report these
//  all as view type operations.
//
tCIDLib::TBoolean
TCQCDevicesBrws::bReportInvocation(const TString& strPath, tCIDLib::TBoolean& bAsEdit) const
{
    bAsEdit = kCIDLib::False;
    return !wndBrowser().bIsScope(strPath);
}


//
//  We just add our top level scope. Then we start up our bgn thread which will start
//  updating our content.
//
tCIDLib::TVoid TCQCDevicesBrws::Initialize(const TCQCUserCtx& cuctxToUse)
{
    TParent::Initialize(cuctxToUse);

    TTreeView& wndTar = wndBrowser();

    //
    //  Add our top level scope. We mark it virtual so that we can trigger the
    //  loading of the info upon actual use. Otherwise, we never bother.
    //
    wndTar.AddScope
    (
        kCQCRemBrws::strPath_Root, kCQCRemBrws::strItem_Devices, kCIDLib::True
    );
}


//
//  This will happen when they expand our root. That's the only one we mark as
//  virtual. We'll fault in the info at that point and start up our bgn thread.
//  This way also, if we can't get the info, we just leave the scope marked as
//  virtaul, so we can try again later if they expand again.
//
tCIDLib::TVoid TCQCDevicesBrws::LoadScope(const TString& strPath)
{
    // Shouldn't happen, but just in case
    if (strPath != kCQCRemBrws::strPath_Devices)
    {
        CIDAssert2(L"LoadScope should only happen for the root devices scope");
        return;
    }

    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcInstSrv = facCQCKit().orbcInstSrvProxy();
        TCQCTreeBrowser& wndTar = wndBrowser();

        //
        //  Get a list of servers that have configured drivers. We'll create the
        //  server objects and add them to the tree browser. They will then load
        //  up their own contents as they get it. We don't bother trying to load
        //  up their drivers since it would be redundant. They would just turn
        //  around and have to make sure they are in sync and store version info
        //  and such.
        //
        tCIDLib::TStrList colSrvs;
        orbcInstSrv->bDrvCfgQueryHosts(colSrvs, sectUser());

        // Set up server objects for each one
        const tCIDLib::TCard4 c4Count = colSrvs.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strSrv = colSrvs[c4Index];

            //
            //  Find the server for this one, adding it if not found. The server is
            //  in the value member.
            //
            TDevBrwsSrvInfo* pdbsiCur = pdbsiFindSrv(strSrv);
            if (!pdbsiCur)
            {
                pdbsiCur = new TDevBrwsSrvInfo(strSrv, &wndBrowser(), cuctxUser());
                m_colSrvList.Add(pdbsiCur);

                wndTar.AddScope
                (
                    kCQCRemBrws::strPath_Devices, pdbsiCur->strHost(), kCIDLib::False
                );
            }
        }

        //
        //  Probably this happened, if we added any above. But, if the scope was
        //  empty, or they all got filtered, then we need to do this just in case,
        //  to clear set the child count info, which also turns off the virtual
        //  scope flag, so we won't try to fault this one back in.
        //
        wndTar.UpdateChildCnt(kCQCRemBrws::strPath_Devices);

        //
        //  At the end of an expansion, force the expanded once state on. It
        //  won't get done if this is a lazily faulted in tree and the expanded
        //  scope ended up being empty. That will cause lots of problems later.
        //
        wndTar.ForceExpandedOnce(kCQCRemBrws::strPath_Devices);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        try
        {
            m_colSrvList.RemoveAll();
        }

        catch(...)
        {
        }
    }

    catch(...)
    {
        try
        {
            m_colSrvList.RemoveAll();
        }

        catch(...)
        {
        }
    }

    // Start up the server objects' monitor threads
    TSrvList::TNCCursor cursStart(&m_colSrvList);
    if (cursStart.bReset())
    {
        do
        {
            cursStart.objWCur().Start();
        }   while(cursStart.bNext());
    }
}


//
//  Not really used here. We get configuration from the user when adding a new
//  driver, and it has all of the info needed.
//
tCIDLib::TVoid
TCQCDevicesBrws::MakeDefName(const  TString&            strParScope
                            ,       TString&            strToFill
                            , const tCIDLib::TBoolean   bIsFile) const
{
}


//
//  We get called back here when the browser window sees a change notification
//  that we posted. This lets our bgn threads post stuff that will come back to
//  us in the GUI thread context.
//
//  In our case we post changes related to changes in the state of driver items,
//  when the thread has done a resync and we need to update the displayed drivers
//  for a given servers.
//
tCIDLib::TVoid
TCQCDevicesBrws::ProcessChange( const   TString&    strPath
                                , const TString&    strOpName
                                , const TString&    strData)
{
    if (strOpName == CQCTreeBrws_DevicesBrws::strChange_ResetServer)
    {
        //
        //  We need to clear the drivers under the server represented by this
        //  path. It will get reloaded as the bgn thread posts initial state
        //  changes for the new list of drivers.
        //
        if (wndBrowser().bPathExists(strPath))
            wndBrowser().RemoveChildrenOf(strPath);
    }
     else if (strOpName == CQCTreeBrws_DevicesBrws::strChange_State)
    {
        //
        //  Break out the host and driver moniker. If we don't have the server or
        //  moniker, we add them first.
        //
        TString strHost, strMon;
        ParsePath(strPath, strHost, strMon);

        TString strHostPath(kCQCRemBrws::strPath_Devices);
        strHostPath.Append(kCIDLib::chForwardSlash);
        strHostPath.Append(strHost);

        TCQCTreeBrowser& wndTar = wndBrowser();
        if (!wndTar.bPathExists(strHostPath))
            wndTar.AddScope(kCQCRemBrws::strPath_Devices, strHost, kCIDLib::False);

        if (!wndTar.bPathExists(strPath))
            wndTar.AddItem(strHostPath, strMon);

        //
        //  An item state changed, so based on the path and the data we update the
        //  status of the item. The data is the formatted out driver state.
        //
        const tCQCKit::EDrvStates eDrvState = tCQCKit::eAltXlatEDrvStates(strData);
        tCQCTreeBrws::EItemStates eItemState(tCQCTreeBrws::EItemStates::Normal);
        if (eDrvState == tCQCKit::EDrvStates::Paused)
            eItemState = tCQCTreeBrws::EItemStates::Paused;
        else if (eDrvState != tCQCKit::EDrvStates::Connected)
            eItemState = tCQCTreeBrws::EItemStates::Bad;
        wndTar.SetItemStatus(strPath, eItemState);
    }
}


//
//  If the browser window gets an accelerator key translation call, he will call us
//  here to load up an accelerator table for him which he will process. If it causes
//  him to get a menu call, he will pass it on to us.
//
tCIDLib::TVoid
TCQCDevicesBrws::QueryAccelTable(const TString& strPath, TAccelTable& accelToFill) const
{
    //
    //  Just load it up from our menu. So we just create an instance of our menu but
    //  never show it.
    //
    TPopupMenu menuAction(L"/Devices Action");
    menuAction.Create(facCQCTreeBrws(), kCQCTreeBrws::ridMenu_Devices);

    // And now set up the accel table from the menu
    accelToFill.Create(menuAction);
}


// We need to stop our monitor thread if it got started
tCIDLib::TVoid TCQCDevicesBrws::Terminate()
{
    //
    //  Shut down any server objects we have. First we ask them to start shutting
    //  down, then go back and wait, so that they all shut down in parallel.
    //
    TSrvList::TNCCursor cursSrvs(&m_colSrvList);
    if (cursSrvs.bReset())
    {
        do
        {
            cursSrvs.objWCur().StartShutdown();
        }   while (cursSrvs.bNext());

        cursSrvs.bReset();
        do
        {
            cursSrvs.objWCur().WaitForDeath();
        }   while (cursSrvs.bNext());
    }
}


// ---------------------------------------------------------------------------
//  TCQCDevicesBrws: Protected, inherited methods
// ---------------------------------------------------------------------------

// Nothing of ours can be renamed
tCIDLib::TBoolean TCQCDevicesBrws::bCanRename(const TString& strPath) const
{
    return kCIDLib::False;
}


// Since we down't allow renaming of items, this should never get called
tCIDLib::TBoolean
TCQCDevicesBrws::bRenameItem(const  TString&
                            , const TString&
                            , const TString&
                            , const tCIDLib::TBoolean)
{
    CIDAssert2(L"bRenameItem shouldn't be called on the devices browser");
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCDevicesBrws: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called when the user asks to add a driver. We get the target host to
//  install it on.
//
tCIDLib::TVoid TCQCDevicesBrws::AddDriver(const TString& strHost)
{
    TCQCTreeBrowser& wndTree = wndBrowser();

    // Let the user select a driver to load
    TCQCDriverCfg   cqcdcSel;
    TSelSrvDrvDlg   dlgSelDrv;
    if (!dlgSelDrv.bRunDlg(wndTree, cqcdcSel, strHost, sectUser()))
        return;

    //
    //  We need to find the available serial ports on the target host, which we do
    //  via the CQCServer on that machine, since it knows which ones are in use by
    //  the other drivers there. We only need to this if it's a serial connected
    //  driver, which is a fixed attribute of the manifest info selected above.
    //
    tCIDLib::TStrList colPorts;
    if (cqcdcSel.conncfgDef().bIsA(TCQCSerialConnCfg::clsThis()))
    {
        try
        {
            TCQCSrvAdminClientProxy* porbcSrc = facCQCKit().porbcCQCSrvAdminProxy(strHost);
            tCQCKit::TCQCSrvProxy orbcSrv(porbcSrc);
            orbcSrv->bQueryCommPorts(colPorts, kCIDLib::False, kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCGKit().ShowError
            (
                wndTree
                , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_GetPorts, strHost)
                , errToCatch
            );
            return;
        }
    }

    // Go through our configured drivers and get all of the monikers
    tCIDLib::TStrList colMonikers;
    TSrvList::TCursor cursSrvs(&m_colSrvList);
    if (cursSrvs.bReset())
    {
        do
        {
            cursSrvs.objRCur().QueryMonikers(colMonikers, kCIDLib::True);
        }   while (cursSrvs.bNext());
    }

    //
    //  These selected a drivers so let's invoke the driver wizard with this manifest
    //  info. Create a driver object config from the manifest driver info we got
    //  above. This is what we'll edit.
    //
    TCQCDriverObjCfg cqcdcNew(cqcdcSel);
    const tCIDLib::TBoolean bLoad = facCQCTreeBrws().bDriverWizard
    (
        wndTree, strHost, cqcdcNew, kCIDLib::False, colMonikers, &colPorts, sectUser()
    );

    if (bLoad)
    {
        // It worked or they wanted to continue anyway
        if (bWriteDrvConfig(strHost, cqcdcNew.strMoniker(), cqcdcNew, kCIDLib::True))
        {
            try
            {
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

                TCQCSrvAdminClientProxy* porbcSrc = facCQCKit().porbcCQCSrvAdminProxy(strHost);
                tCQCKit::TCQCSrvProxy orbcSrv(porbcSrc);

                tCIDLib::TStrList colMonList(1);
                colMonList.objAdd(cqcdcNew.strMoniker());

                if (!porbcSrc->bLoadDrivers(colMonList, sectUser()))
                {
                    TErrBox msgbFailed
                    (
                        facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvLoadFailed)
                    );
                    msgbFailed.ShowIt(wndTree);
                    return;
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TYesNoBox msgbFailed
                (
                    facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvLoadFailed)
                );
                if (!msgbFailed.bShowIt(wndTree))
                    return;
            }

            try
            {
                // See if we have this server in our list
                TDevBrwsSrvInfo* pdbsiTar = pdbsiFindSrv(strHost);
                if (!pdbsiTar)
                {
                    // Don't get it so create it and add it to our server list
                    pdbsiTar = new TDevBrwsSrvInfo(strHost, &wndTree, cuctxUser());
                    m_colSrvList.Add(pdbsiTar);

                    // And start it processing
                    pdbsiTar->Start();
                }
                 else
                {
                    // We already have it, so just force it to refresh
                    pdbsiTar->ForceRefresh();
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                facCQCGKit().ShowError
                (
                    wndBrowser()
                    , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_TreeUpdate)
                    , errToCatch
                );
            }

            catch(...)
            {
                facCQCGKit().ShowError
                (
                    wndBrowser(), facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_TreeUpdate)
                );
            }
        }
         else
        {
            //
        }
    }
}


//
//  We often need to get the configuration for a driver, so it's broken out here
//  in this helper. We return true if we were able to successfully get it.
//
tCIDLib::TBoolean
TCQCDevicesBrws::bReadDrvConfig(const   TString&            strHost
                                , const TString             strMoniker
                                ,       TCQCDriverObjCfg&   cqcdcToFill)
{
    try
    {
        // Copy the host since this call returns the host it found it on
        TString strTmpHost(strHost);

        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4Ver = 0;
        orbcIS->bDrvCfgQueryConfig(c4Ver, strMoniker, strTmpHost, cqcdcToFill, sectUser());
    }

    catch(TError& errToCatch)
    {
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvCfgQFailed, strMoniker, strHost)
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvCfgQFailed, strMoniker, strHost)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Writes out the driver configuration for the indicated driver (to be loaded into
//  the indicated host.) They tell us if this should be a new one, or if they are
//  updating. We call the appropriate method, so that we catch there being existing
//  content when there shouldn't be, or vice versa.
//
tCIDLib::TBoolean
TCQCDevicesBrws::bWriteDrvConfig(const  TString&            strHost
                                , const TString             strMoniker
                                , const TCQCDriverObjCfg&   cqcdcToWrite
                                , const tCIDLib::TBoolean   bNew)
{
    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4NewVersion;
        orbcIS->DrvCfgSetConfig(strMoniker, strHost, cqcdcToWrite, bNew, c4NewVersion, sectUser());
    }

    catch(TError& errToCatch)
    {
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_CfgStoreFailed, strMoniker, strHost)
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_CfgStoreFailed, strMoniker, strHost)
        );
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Let the user select a new host to replace the passed one
tCIDLib::TVoid TCQCDevicesBrws::ChangeServer(const TString& strPath)
{
    // Get the host name out, which is the last part of the path
    TString strOldHost;
    facCQCRemBrws().QueryNamePart(strPath, strOldHost);

    try
    {
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy();
        orbcNS->bQueryScopeKeys
        (
            m_c4NSVersionNum
            , TCQCSrvAdminClientProxy::strAdminScope
            , m_colLastNSList
            , kCIDLib::True
            , kCIDLib::True
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCGKit().ShowError
        (
            wndBrowser(), L"Could not get available CQC Driver Hosts", errToCatch
        );
        return;
    }

    // Go through the list and remove the current one, since it's not an option
    tCIDLib::TCard4 c4Count = m_colLastNSList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colLastNSList[c4Index].bCompareI(strOldHost))
        {
            m_colLastNSList.RemoveAt(c4Index);
            c4Count--;
            break;
        }
    }

    // If there aren't any, then tell the user so
    if (m_colLastNSList.bIsEmpty())
    {
        TOkBox msgbEmpty(L"Could find any other available CQC driver hosts");
        msgbEmpty.ShowIt(wndBrowser());
        return;
    }

    // We have some possibilities, so let's present the list for selection
    TString strNewHost;
    TChangeDrvSrvDlg dlgSel;
    if (dlgSel.bRun(wndBrowser(), m_colLastNSList, strOldHost, strNewHost))
    {
        // They made a selection, so let's do it
        try
        {
            tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
            tCIDLib::TStrList colMsgs;
            if (orbcIS->bDrvCfgMoveDrivers(strOldHost, strNewHost, colMsgs, sectUser()))
            {
                // Remove the old host from our tree and list
                // <TBD>


                // Let's force a reload of our list
                RefreshAll();
            }
             else
            {
                // We need to display a dialog here that shows the messages
                facCIDWUtils().ShowList
                (
                    wndBrowser(), L"Some errors occurred", colMsgs
                );
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser(), L"The operation could not be completed", errToCatch
            );
        }
    }
}


//
//  We create a standard XML formatted output of the fields of the selected driver(s). If it's
//  a single driver we just do that. If it's a server, we do all of the drivers on that
//  server. Each one is written to a file based on the driver moniker, to a user selected
//  directory.
//
tCIDLib::TVoid
TCQCDevicesBrws::ExportDrv(         tCQCKit::TCQCSrvProxy&  orbcSrv
                            , const TPathStr&               pathTar
                            , const tCIDLib::TBoolean       bIncludeVals
                            , const TString&                strMoniker)
{
    tCIDLib::TCard4         c4FldListId;
    tCIDLib::TCard4         c4DriverId;
    tCIDLib::TCard4         c4DriverListId;
    TVector<TCQCFldDef>     colDrvCfgs;
    tCQCKit::EDrvStates     eState;
    tCIDLib::TCard4 c4FldCnt = orbcSrv->c4QueryFields
    (
        strMoniker, eState, colDrvCfgs, c4FldListId, c4DriverId, c4DriverListId
    );

    //
    //  Throw out any that start with a dollar sign. Those are magic ones
    //  and they will be created automatically by all drivers.
    //
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4FldCnt)
    {
        const TCQCFldDef& flddCur = colDrvCfgs[c4Index];
        if (flddCur.strName().chFirst() == kCIDLib::chDollarSign)
        {
            colDrvCfgs.RemoveAt(c4Index);
            c4FldCnt--;
        }
         else
        {
            c4Index++;
        }
    }

    // And get the driver info so we can dump the make/model
    TCQCDriverObjCfg cqcdcDump;
    if (!orbcSrv->bQueryDriverInfo(strMoniker, cqcdcDump))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcRem_QueryDriverInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , strMoniker
        );
    }

    // That worked, so let's dump the info
    TPathStr pathOutFile(pathTar);
    pathOutFile.AddLevel(strMoniker);
    pathOutFile.AppendExt(L".CQCFldDump");

    TTextFileOutStream strmOutput
    (
        pathOutFile
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Write
        , new TUTF8Converter
    );

    // Do the preamble and the start of the main element
    strmOutput  << L"<?xml version='1.0' encoding='UTF-8' ?>\n"
                << L"<!DOCTYPE CQCFldDump PUBLIC 'urn:charmedquark.com:CQC-CQCFldDumpV1.DTD' 'CQCFldDump.DTD'>\n"
                << L"<CQCFldDump Moniker='" << strMoniker << L"' FldCnt='"
                << c4FldCnt << L"' Make='" << cqcdcDump.strMake()
                << L"' Model='" << cqcdcDump.strModel()
                << L"'>\n";

    TString strValue;
    for (c4Index = 0; c4Index < c4FldCnt; c4Index++)
    {
        const TCQCFldDef& flddCur = colDrvCfgs[c4Index];
        const TString& strFldName = flddCur.strName();

        strmOutput  << L"    <CQCFldDef Name='" << strFldName
                    << L"' Type='" << tCQCKit::strXlatEFldTypes(flddCur.eType())
                    << L"' SType='" << tCQCKit::strXlatEFldSTypes(flddCur.eSemType())
                    << L"' Access='" << tCQCKit::strXlatEFldAccess(flddCur.eAccess())
                    << L"'\n            Private='"
                    << (flddCur.bPrivate() ? L"Yes" : L"No")
                    << L"' AlwaysWrite='" << (flddCur.bAlwaysWrite() ? L"Yes" : L"No")
                    << L"' QueuedWrite='" << (flddCur.bQueuedWrite() ? L"Yes" : L"No")
                    << L"'";

        // If they asked for the field values and this one is readable, get that
        if (bIncludeVals
        &&  tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
        {
            tCIDLib::TCard4 c4SerialNum = 0;
            tCQCKit::EFldTypes eType;
            orbcSrv->bReadFieldByName(c4SerialNum, strMoniker, strFldName, strValue, eType);

            strmOutput << L" FldValue='";
            facCIDXML().EscapeFor(strValue, strmOutput, tCIDXML::EEscTypes::Attribute);
            strmOutput << L"'";
        }
        strmOutput  << L">\n";

        //
        //  We put the limits info into the element body. It has to be
        //  escaped since it's free form info.
        //
        const TString& strLimits = flddCur.strLimits();
        if (!strLimits.bIsEmpty())
        {
            strmOutput << L"        ";
            facCIDXML().EscapeFor(flddCur.strLimits(), strmOutput, tCIDXML::EEscTypes::ElemText);
            strmOutput << kCIDLib::NewLn;
        }

        // And close this one off
        strmOutput  << L"    </CQCFldDef>\n";
    }

    // Wrap up the main element and flush and we are done
    strmOutput  << L"</CQCFldDump>\n\n" << kCIDLib::FlushIt;
}

tCIDLib::TVoid
TCQCDevicesBrws::ExportFlds(const   TString&            strPath
                            , const tCIDLib::TBoolean   bIncludeVals)
{
    // Ask the user where to put the file(s)
    TPathStr pathOutScope;
    {
        // Not used in this case, but have to pass one
        tCIDLib::TKVPList colTypes(1);

        tCIDLib::TStrList colSelList;
        const tCIDLib::TBoolean bGotSel = facCIDCtrls().bOpenFileDlg
        (
            wndBrowser()
            , L"Select Target Folder"
            , TString::strEmpty()
            , colSelList
            , colTypes
            , tCIDCtrls::EFOpenOpts
              (
                tCIDCtrls::EFOpenOpts::SelectFolders
              )
        );

        if (!bGotSel)
            return;

        // Store the output path and let's continue
        pathOutScope = colSelList[0];
    }

    //
    //  One way or another, we will fill this list with the monikers of the drivers to unload
    //  and remember the server name.
    //
    TString strHost;
    tCIDLib::TStrList colMonikers;

    if (wndBrowser().bIsScope(strPath))
    {
        // Get the host name out, which is the last part of the path
        facCQCRemBrws().QueryNamePart(strPath, strHost);

        // It's a server, so look up this server in our server list
        TDevBrwsSrvInfo* pdbsiTar = pdbsiFindSrv(strHost);
        if (!pdbsiTar)
        {
            // Shouldn't happen, but just in case
            facCQCGKit().ShowError(wndBrowser(), L"The server could not be found the tree");
            return;
        }

        // Get it's list of monikers into our list
        pdbsiTar->QueryMonikers(colMonikers, kCIDLib::False);
    }
     else
    {
        //
        //  It's a driver, so get the name part which is the driver. Then cut it off and get
        //  the next name, which is the server.
        //
        TString strDriver;
        facCQCRemBrws().QueryNamePart(strPath, strDriver);

        TString strTmpPath(strPath);
        if (!facCQCRemBrws().bRemoveLastPathItem(strTmpPath))
        {
            // Shouldn't happen, but just in case
            facCQCGKit().ShowError(wndBrowser(), L"The server could not be gotten from the path");
            return;
        }
        facCQCRemBrws().QueryNamePart(strTmpPath, strHost);

        // Put the driver into the list to export
        colMonikers.objAdd(strDriver);
    }

    //
    //  OK, let's get a client proxy for the server. There's on in the server object, but just
    //  to be sure, we get our own.
    //
    tCQCKit::TCQCSrvProxy orbcSrv;
    try
    {
        orbcSrv.SetPointer(facCQCKit().porbcCQCSrvAdminProxy(strHost));
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_GetCQCSProxy, strHost)
            , errToCatch
        );
        return;
    }

    // Loop through the list and export each one
    tCIDLib::TStrList colFailed;
    const tCIDLib::TCard4 c4Count = colMonikers.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        try
        {
            ExportDrv(orbcSrv, pathOutScope, bIncludeVals, colMonikers[c4Index]);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            colFailed.objAdd(colMonikers[c4Index]);
        }
    }

    const tCIDLib::TCard4 c4FailCnt = colFailed.c4ElemCount();
    if (c4FailCnt)
    {
        TTextStringOutStream strmTar(1024UL);
        strmTar << facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_ExportFlds);
        strmTar << kCIDLib::NewLn;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FailCnt; c4Index++)
            strmTar << colFailed[c4Index] << kCIDLib::NewLn;

        strmTar.Flush();

        TErrBox msgbFailed(strmTar.strData());
        msgbFailed.ShowIt(wndBrowser());
    }
     else
    {
        TOkBox msgbDone(L"All selected drivers were exported successfully");
        msgbDone.ShowIt(wndBrowser());
    }
}


//
//  Find the driver info object for the indicating host/moniker.
//
TDevBrwsDrvInfo*
TCQCDevicesBrws::pdbdiFindDriver(const TString& strHost, const TString& strMoniker)
{
    TDevBrwsSrvInfo* pdbsiTar = pdbsiFindSrv(strHost);
    if (!pdbsiTar)
        return nullptr;
    return pdbsiTar->pdbdiFindByMoniker(strMoniker);
}


//
//  Find the server info object for the indicated host.
//
TDevBrwsSrvInfo* TCQCDevicesBrws::pdbsiFindSrv(const TString& strHost)
{
    TSrvList::TNCCursor cursSrvs(&m_colSrvList);
    if (cursSrvs.bReset())
    {
        do
        {
            TDevBrwsSrvInfo& dbsiCur = cursSrvs.objWCur();
            if (dbsiCur.strHost().bCompareI(strHost))
                return &dbsiCur;
        }   while (cursSrvs.bNext());
    }

    return nullptr;
}


//
//  This is a helper that will parse the host and moniker from a browser path of
//  a driver.
//
tCIDLib::TVoid
TCQCDevicesBrws::ParsePath(const TString& strPath, TString& strHost, TString& strMon)
{
    // Copy in the part of the path past our root path, so the start of the host
    strHost.CopyInSubStr(strPath, kCQCRemBrws::strPath_Devices.c4Length() + 1);

    // Now split that on the forward slash to leave the two bits we want
    strHost.bSplit(strMon, kCIDLib::chForwardSlash);
}


//
//  This is called when the user asks to pause or resume a driver. We read the driver
//  config in, update it, and write it back out again. Our bgn thread will see the
//  change in status and update the display status of the item.
//
tCIDLib::TVoid
TCQCDevicesBrws::PauseResumeDriver( const   TString&            strPath
                                    , const tCIDLib::TBoolean   bPause)
{
    // Pull the host and moniker out of the path
    TString strHost;
    TString strMoniker;
    ParsePath(strPath, strHost, strMoniker);

    // Maie sure they really want to do it
    TYesNoBox msgbConfirm
    (
        facCQCTreeBrws().strMsg
        (
            bPause ? kTBrwsMsgs::midQ_PauseDrv : kTBrwsMsgs::midQ_ResumeDrv
            , strMoniker
            , strHost
        )
    );
    if (!msgbConfirm.bShowIt(wndBrowser()))
        return;

    //
    //  We query the driver config for this driver, update the paused/resumed value
    //  and write it back.
    //
    TCQCDriverObjCfg cqcdcEdit;
    if (bReadDrvConfig(strHost, strMoniker, cqcdcEdit))
    {
        // Update it and write it back
        cqcdcEdit.bPaused(bPause);
        if (bWriteDrvConfig(strHost, strMoniker, cqcdcEdit, kCIDLib::False))
        {
            // That worked, so let's tell the host to pause/resume this driver
            try
            {
                TCQCSrvAdminClientProxy* porbcSrc
                (
                    facCQCKit().porbcCQCSrvAdminProxy(strHost)
                );
                tCQCKit::TCQCSrvProxy orbcSrv(porbcSrc);
                orbcSrv->PauseResumeDrv
                (
                    strMoniker, bPause, sectUser(), tCQCKit::EDrvCmdWaits::NoWait
                );

                TOkBox msgbDone(facCQCTreeBrws().strMsg(kTBrwsMsgs::midPrompt_DrvPauseResDone));
                msgbDone.ShowIt(wndBrowser());
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                facCQCGKit().ShowError
                (
                    wndBrowser()
                    , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_PauseRes, strMoniker, strHost)
                    , errToCatch
                );
            }
        }
    }
}


//
//  This is called when the user asks to reconfigure a driver. It's similar to
//  AddDriver above, but we don't need to do a good bit of the setup, and we
//  invoke the driver wizard in reconfigure mode, which doesn't allow the moniker
//  to be changed.
//
//  We do have to query the current configuration for the driver. And we have to
//  get the available ports. We don't need the current monikers because they can't
//  change the moniker, so no need to enforce uniqueness.
//
tCIDLib::TVoid TCQCDevicesBrws::ReconfigureDriver(const TString& strPath)
{
    // Pull the host and moniker out of the path
    TString strHost;
    TString strMoniker;
    ParsePath(strPath, strHost, strMoniker);

    //
    //  And let's query the current configuration for this driver from the cfg server.
    //  If that fails, then we give up.
    //
    TCQCDriverObjCfg cqcdcEdit;
    if (!bReadDrvConfig(strHost, strMoniker, cqcdcEdit))
        return;

    //
    //  Now we have to query the most recent manifest info for the make/model of this
    //  driver.
    //
    TCQCDriverCfg cqcdcLatest;
    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();
        if (!orbcIS->bQueryDrvManifest(cqcdcEdit.strMake(), cqcdcEdit.strModel(), cqcdcLatest))
        {
            TErrBox msgbDone
            (
                facCQCTreeBrws().strMsg
                (
                    kTBrwsErrs::errcDev_NoManifest
                    , cqcdcEdit.strMake()
                    , cqcdcEdit.strModel()
                )
            );
            msgbDone.ShowIt(wndBrowser());
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg
              (
                kTBrwsErrs::errcDev_GetManifest
                , cqcdcEdit.strMake()
                , cqcdcEdit.strModel()
              )
            , errToCatch
        );
        return;
    }

    //
    //  Now we need to mix them together, taking the new and changed stuff from the
    //  manifest to update the driver configuration.
    //
    cqcdcEdit.UpdateBase(cqcdcLatest);

    // For any defaulting of prompts that were added after this driver was installed
    cqcdcEdit.FaultInPromptVals(kCIDLib::True);

    //
    //  We need to find the available serial ports on the target host, which we do
    //  via the CQCServer on that machine, since it knows which ones are in use by
    //  the other drivers there. We only need to this if it's a serial connected
    //  driver, which is a fixed attribute of the manifest info selected above.
    //
    tCIDLib::TStrList colPorts;
    if (cqcdcEdit.conncfgDef().bIsA(TCQCSerialConnCfg::clsThis()))
    {
        try
        {
            tCQCKit::TCQCSrvProxy orbcSrv = facCQCKit().orbcCQCSrvAdminProxy(strMoniker);
            orbcSrv->bQueryCommPorts(colPorts, kCIDLib::False, kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCGKit().ShowError
            (
                wndBrowser()
                , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_GetPorts, strHost)
                , errToCatch
            );
            return;
        }
    }

    //
    //  We always force paused state off if they reconfigure, because the driver will start
    //  up when it gets the reconfigure command. If we don't force it off in the new config
    //  we write, then it will be running but marked paused.
    //
    cqcdcEdit.bPaused(kCIDLib::False);

    //
    //  Now invoke the driver wizard in reconfigure mode with this copy of the config.
    //  We can pass an empty moniker list.
    //
    tCIDLib::TStrList colMonikers;
    const tCIDLib::TBoolean bLoad = facCQCTreeBrws().bDriverWizard
    (
        wndBrowser(), strHost, cqcdcEdit, kCIDLib::True, colMonikers, &colPorts, sectUser()
    );

    //
    //  If they didn't commit, or we cannot write out the updated driver configuration
    //  data, then give up.
    //
    if (!bLoad || !bWriteDrvConfig(strHost, strMoniker, cqcdcEdit, kCIDLib::False))
        return;

    // Else let's try to tell the server to do the reconfiguration
    try
    {
        // It worked, so ask the server to reconfigure it
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

        TCQCSrvAdminClientProxy* porbcSrc = facCQCKit().porbcCQCSrvAdminProxy(strHost);
        tCQCKit::TCQCSrvProxy orbcSrv(porbcSrc);
        porbcSrc->ReconfigDriver(strMoniker, sectUser());

        TOkBox msgbDone(facCQCTreeBrws().strMsg(kTBrwsMsgs::midPrompt_DrvReconfDone));
        msgbDone.ShowIt(wndBrowser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvLoadFailed)
            , errToCatch
        );
    }

    catch(...)
    {
        facCQCGKit().ShowError
        (
            wndBrowser(), facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_SrvDrvLoadFailed)
        );
    }
}


//
//  This is called when the user selects a refresh on our main scope. We have to first
//  see if there are any new servers that have configured drivers. If so, we need to add
//  them. If any have gone away, we have to remove them.
//
//  For those that are already loaded, we just tell them to start a refresh cycle.
//
tCIDLib::TVoid TCQCDevicesBrws::RefreshAll()
{
    TCQCTreeBrowser& wndTar = wndBrowser();
    tCIDLib::TStrList colCfgSrvs;
    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        tCQCKit::TInstSrvProxy orbcInstSrv = facCQCKit().orbcInstSrvProxy();

        //
        //  Get a list of servers that have configured drivers. This is from the configured
        //  drivers, NOT the live ones currently registered.
        //
        orbcInstSrv->bDrvCfgQueryHosts(colCfgSrvs, sectUser());
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbErr(L"The installation server could not be contacted.");
        return;
    }

    //
    //  Let's see what's new, what's old, and what's gone. So query all of the names
    //  under our main scope to get the hosts we currently know about.
    //
    tCIDLib::TStrList colCurSrvs;
    wndTar.QueryChildren(kCQCRemBrws::strPath_Devices, colCurSrvs, kCIDLib::False);

    // Now build up lists of the various types
    tCIDLib::TStrList colAddedSrvs;
    tCIDLib::TStrList colKeptSrvs;
    tCIDLib::TStrList colMissingSrvs;

    //
    //  Go through the configured list and for any that are not in the current list,
    //  move them to the added servers list. For any that are, add them to the
    //  kept list.
    //
    tCIDLib::TCard4 c4Count = colCfgSrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCur = colCfgSrvs[c4Index];
        const tCIDLib::TCard4 c4Cnt = colCurSrvs.c4ElemCount();
        tCIDLib::TCard4 c4Ind = 0;
        while (c4Ind < c4Cnt)
        {
            if (colCurSrvs[c4Ind].bCompareI(strCur))
                break;
            c4Ind++;
        }

        if (c4Ind == c4Cnt)
            colAddedSrvs.objAdd(colCfgSrvs[c4Index]);
        else
            colKeptSrvs.objAdd(colCfgSrvs[c4Index]);
    }

    //
    //  Go through the current servers list and for any not in the configured servers
    //  list, move them to the missing servers list.
    //
    c4Count = colCurSrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCur = colCurSrvs[c4Index];
        const tCIDLib::TCard4 c4Cnt = colCfgSrvs.c4ElemCount();
        tCIDLib::TCard4 c4Ind = 0;
        while (c4Ind < c4Cnt)
        {
            if (colCfgSrvs[c4Ind].bCompareI(strCur))
                break;
            c4Ind++;
        }

        if (c4Ind == c4Cnt)
            colMissingSrvs.objAdd(colCurSrvs[c4Index]);
    }

    //
    //  OK, for any missing ones, let's remove their scopes and remove them from our
    //  list. We may get a change notification for it after this, but it's scope won't
    //  be found so nothing will happen.
    //
    c4Count = colMissingSrvs.c4ElemCount();
    TString strPath;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        strPath = kCQCRemBrws::strPath_Devices;
        strPath.Append(kCIDLib::chForwardSlash);
        strPath.Append(colMissingSrvs[c4Index]);
        wndTar.RemoveItem(strPath);
    }

    // For any that are new, we need to add them
    c4Count = colAddedSrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TString& strCur = colAddedSrvs[c4Index];
        TDevBrwsSrvInfo* pdbsiCur = new TDevBrwsSrvInfo(strCur, &wndTar, cuctxUser());
        m_colSrvList.Add(pdbsiCur);
        wndTar.AddScope(kCQCRemBrws::strPath_Devices, pdbsiCur->strHost(), kCIDLib::False);
        pdbsiCur->Start();
    }

    // For any that we kepts, just ask them to refresh
    c4Count = colKeptSrvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TDevBrwsSrvInfo* pdbsiCur = pdbsiFindSrv(colKeptSrvs[c4Index]);
        if (pdbsiCur)
        {
            pdbsiCur->ForceRefresh();
        }
         else
        {
            CIDAssert2(L"One of the 'kept' servers was not foudn in our server list");
        }
    }
}


//
//  This is called to update the popup menu based on the driver or host that was
//  clicked on.
//
tCIDLib::TVoid TCQCDevicesBrws::SetupMenu(const TString& strPath, TMenu& menuTar) const
{
    //
    //  If this is a power user, we only allow info type operations. Else it has to be
    //  an admin.
    //
    tCIDLib::TCardList fcolList;
    if (strPath == kCQCRemBrws::strPath_Devices)
    {
        // If it's our root, then all they can do is add a driver
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_ClientDrv);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Delete);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Info);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Pause);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Reconfig);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Resume);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_View);

        // If a power user, not even that
        if (cuctxUser().eUserRole() == tCQCKit::EUserRoles::PowerUser)
            fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_New);
    }
     else if (wndBrowser().bIsScope(strPath))
    {
        // It's a scope, then it's got to be a host, so disable driver ones
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_ClientDrv);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Delete);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Info);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Pause);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Reconfig);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Resume);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_View);

        // If a power user, no add driver
        if (cuctxUser().eUserRole() == tCQCKit::EUserRoles::PowerUser)
            fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_New);
    }
     else
    {
        // Gotta be a driver, so disable host related ones
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_New);
        fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Refresh);

        // If a power user, disable delete
        if (cuctxUser().eUserRole() == tCQCKit::EUserRoles::PowerUser)
            fcolList.c4AddElement(kCQCTreeBrws::ridMenu_Devices_Delete);
    }

    const tCIDLib::TCard4 c4Count = fcolList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        menuTar.SetItemEnable(fcolList[c4Index], kCIDLib::False);
}



//
//  Called when the user asks to delete a driver, either via the menu, or by hitting
//  Delete with a driver selected. However, if they hit delete, they could be on a
//  host, so we have to check for that.
//
tCIDLib::TVoid TCQCDevicesBrws::UnloadDriver(const TString& strPath)
{
    // If a scope, then it's a host, not a driver, so ignore
    if (wndBrowser().bIsScope(strPath))
        return;

    // Parse out the host and driver moniker from the path
    TString strHost;
    TString strMoniker;
    ParsePath(strPath, strHost, strMoniker);

    // Make sure they want to do it
    TYesNoBox msgbConfirm
    (
        facCQCTreeBrws().strMsg(kTBrwsMsgs::midQ_UnloadDrv, strMoniker, strHost)
    );
    if (!msgbConfirm.bShowIt(wndBrowser()))
        return;

    //
    //  Delete this item from the configured drivers. But first look up the driver and
    //  see if it has a driver id. If not, then it is not actually loaded on the target
    //  host and so we don't want to try to unload it.
    //
    TDevBrwsSrvInfo* pdbsiHost = pdbsiFindSrv(strHost);
    TDevBrwsDrvInfo* pdbdiDrv = pdbsiHost->pdbdiFindByMoniker(strMoniker);
    const tCIDLib::TBoolean bUnload
    (
        pdbdiDrv && (pdbdiDrv->c4DriverId() != kCIDLib::c4MaxCard)
    );
    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4NewVer = 0;
        orbcIS->DrvCfgRemoveDrv(strMoniker, strHost, c4NewVer, sectUser());

        //
        //  If the new version is max card, then the host was removed becasue that as the last
        //  driver in it, so we will force a full refresh. Else, we just refresh the target
        //  host object.
        //
        if (c4NewVer == kCIDLib::c4MaxCard)
        {
            // This host is now empty of drivers, so remove it
            try
            {
                TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

                // First try to stop the host object
                pdbsiHost->StartShutdown();
                pdbsiHost->WaitForDeath();

                // It's down so remove it. Build a path for just the host part
                TString strHostPath(strPath);
                if (facCQCRemBrws().bRemoveLastPathItem(strHostPath))
                {
                    // It should be in our server list since we just found it above
                    if (m_colSrvList.bRemoveKeyIfExists(strHostPath))
                    {
                        // And then remove it from the tree as well
                        wndBrowser().RemoveItem(strHostPath);
                    }
                    else
                    {
                        TString strErr(L"The path was not in the server list. Path=", 128UL);
                        strErr += strHostPath;
                        CIDAssert2(strErr);
                    }
                }
                 else
                {
                    CIDAssert2(L"Could not create the host path to remove");
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                facCQCGKit().ShowError
                (
                    wndBrowser()
                    , L"/Devices browser"
                    , L"Could not stop the host object's monitor thread"
                    , errToCatch
                );
            }
        }
         else
        {
            pdbsiHost->ForceRefresh();
        }
    }

    catch(TError& errToCatch)
    {
        facCQCGKit().ShowError
        (
            wndBrowser()
            , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_CfgDelFailed, strMoniker, strHost)
            , errToCatch
        );
    }

    //
    //  Whether the config stuff worked or not, if it is running on the server, then to
    //  remove the driver from the CQCServer host.
    //
    if (bUnload)
    {
        try
        {
            TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);

            TCQCSrvAdminClientProxy* porbcSrc = facCQCKit().porbcCQCSrvAdminProxy(strHost);
            tCQCKit::TCQCSrvProxy orbcSrv(porbcSrc);
            porbcSrc->UnloadDriver(strMoniker, sectUser());
        }

        catch(TError& errToCatch)
        {
            facCQCGKit().ShowError
            (
                wndBrowser()
                , facCQCTreeBrws().strMsg(kTBrwsErrs::errcDev_DrvUnloadFailed, strHost)
                , errToCatch
            );

            // Don't fall through
            return;
        }
    }

    // OK, we are done so tell the user
    TOkBox msgbDone
    (
        facCQCTreeBrws().strMsg(kTBrwsMsgs::midPrompt_DrvUnloadDone)
    );
    msgbDone.ShowIt(wndBrowser());
}

