//
// FILE NAME: CQLogicSh_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/07/2011
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
//  This is the implementation file for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQLogicSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQLogicSh,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQLogicSh::TCacheItem
//  PREFIX: item
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQLogicSh::TCacheItem: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQLogicSh::TCacheItem::TCacheItem(const TString& strName) :

    m_c4GraphCfgSerNum(0)
    , m_enctLastAccess(TTime::enctNow())
    , m_enctNextPoll(0)
    , m_grdatSamples(kCQLogicSh::c4GraphSampleCnt)
    , m_strName(strName)
{
}

TFacCQLogicSh::TCacheItem::~TCacheItem()
{
}


// ---------------------------------------------------------------------------
//  TFacCQLogicSh::TCacheItem: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Give back any new samples since the caller last checked
tCIDLib::TBoolean TFacCQLogicSh::
TCacheItem::bQueryNewSamples(tCIDLib::TCard4&           c4SerialNum
                            , tCQLogicSh::TSampleList&  fcolToFill
                            , tCIDLib::TCard4&          c4NewSamples)
{
    // Update the last accessed time stamp, then get any new samples
    m_enctLastAccess = TTime::enctNow();
    return m_grdatSamples.bQuerySamples(c4SerialNum, fcolToFill, c4NewSamples);
}


//
//  If the poller thread loses connection to the logic server, it'll call this
//  to make sure that we clear our graph out and bump our serial number. This
//  will cause clients to clear out their displays until we get back connected.
//
tCIDLib::TVoid TFacCQLogicSh::TCacheItem::Reset()
{
    m_grdatSamples.Reset(0UL);
    m_c4GraphCfgSerNum++;
}


// Store new samples from the poller thread
tCIDLib::TVoid TFacCQLogicSh::
TCacheItem::StoreNewSamples(const   tCQLogicSh::TSampleList&    fcolNew
                            , const tCIDLib::TCard4             c4NewCnt
                            , const tCIDLib::TCard4             c4SerialNum)
{
    // Push the incoming samples into our sample list
    m_grdatSamples.PushNewSamples(fcolNew, c4NewCnt, c4SerialNum);
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQLogicSh
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQLogicSh: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQLogicSh::TFacCQLogicSh() :

    TFacility
    (
        L"CQLogicSh"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bStatus(kCIDLib::False)
    , m_colCache(8)
    , m_pthrPoller(nullptr)
{
}

TFacCQLogicSh::~TFacCQLogicSh()
{
    if (m_pthrPoller)
        StopPoller();
}


// ---------------------------------------------------------------------------
//  TFacCQLogicSh: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We provide a helper for clients to query graph data from the logic server.
//  Because this data is not field data, the huge benefits of the generic
//  polling engine are not available for this stuff. So we do a little version
//  of a polling engine here, providing caching of the gotten data so that
//  often there will be no need to go back to the logic server.
//
tCQLogicSh::EGraphQRes
TFacCQLogicSh::eQueryGraphSamples(  const   TString&                strName
                                    ,       tCIDLib::TCard4&        c4CfgSerialNum
                                    ,       tCIDLib::TCard4&        c4SerialNum
                                    ,       tCQLogicSh::TSampleList& fcolToFill
                                    ,       tCIDLib::TCard4&        c4NewSamples)
{
    // If we've not created the thread yet, then do that
    if (!m_pthrPoller)
    {
        TLocker lockrSync(&m_mtxSync);
        if (!m_pthrPoller)
        {
            // Clear the cache and status just in case
            m_bStatus = kCIDLib::False;
            m_colCache.RemoveAll();

            // Then create the poller thread and start it up
            m_pthrPoller = new TThread
            (
                L"LogicSrvGraphPoller"
                , TMemberFunc<TFacCQLogicSh>(this, &TFacCQLogicSh::ePoller)
            );
            m_pthrPoller->Start();
        }
    }

    // Lock the cache and see if this one is available
    TLocker lockrSync(&m_mtxSync);

    tCQLogicSh::EGraphQRes eRes = tCQLogicSh::EGraphQRes::NoNewSamples;
    const tCIDLib::TCard4 c4Count = m_colCache.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCacheItem& itemCur = m_colCache[c4Index];

        if (itemCur.m_strName == strName)
        {
            //
            //  If our poller thread isn't happy, then just reset the caller, to
            //  make him clear his graph and keep his serial numbers zeroed so
            //  that when the server comes back he'll get new stuff.
            //
            //  Else, do a query for new stuff.
            //
            tCIDLib::TBoolean bNewSamples = kCIDLib::False;
            if (m_bStatus)
            {
                //
                //  If he is out of sync with the config, force a reset on him,
                //  so that he'll get a full whack of new samples.
                //
                if (c4CfgSerialNum != itemCur.m_c4GraphCfgSerNum)
                    c4SerialNum = 0;

                if (itemCur.m_grdatSamples.bQuerySamples(c4SerialNum, fcolToFill, c4NewSamples))
                    eRes = tCQLogicSh::EGraphQRes::NewSamples;

                //
                //  Give him back the latest config serial num. The regular serial
                //  number was already updated above.
                //
                c4CfgSerialNum = itemCur.m_c4GraphCfgSerNum;
            }
             else
            {
                fcolToFill.RemoveAll();
                c4NewSamples = 0;
                c4CfgSerialNum = 0;
                c4SerialNum = 0;
                eRes = tCQLogicSh::EGraphQRes::Error;
            }

            // Update this guy's last access stamp
            itemCur.m_enctLastAccess = TTime::enctNow();

            return eRes;
        }
    }

    //
    //  If we got to here, then we never found it, so we need to add it to the
    //  cache so that it'll be polled. We return false to tell the caller that
    //  there's no data available. He'll come back again next time and it should
    //  be ready by then.
    //
    m_colCache.objPlace(strName);
    c4CfgSerialNum = 0;
    c4NewSamples = 0;

    return eRes;
}


//
//  A helper to get a logic server client proxy. We return it via a counted
//  pointer in the usual way, so that they can be dealt with via value
//  semantics.
//
tCQLogicSh::TLogicSrvProxy
TFacCQLogicSh::orbcLogicSrvProxy(const tCIDLib::TCard4 c4WaitFor) const
{
    TLogicSrvClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TLogicSrvClientProxy>
    (
        TLogicSrvClientProxy::strBinding, c4WaitFor
    );

    if (!porbcProxy)
    {
        facCQLogicSh().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQLShErrs::errcRem_LogSrvNotAvail
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
        );
    }
    return tCQLogicSh::TLogicSrvProxy(porbcProxy);
}



// Stop the poller thread if it's running
tCIDLib::TVoid TFacCQLogicSh::StopPoller()
{
    if (m_pthrPoller)
    {
        try
        {
            m_pthrPoller->ReqShutdownSync(8000);
            m_pthrPoller->eWaitForDeath(2000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }
}


// ---------------------------------------------------------------------------
//  TFacCQLogicSh: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The poller thread is pointed here. We just wake up periodically and go
//  through the cache. Any that are out of date, we drop. Any that are ready
//  to poll, we poll.
//
tCIDLib::EExitCodes
TFacCQLogicSh::ePoller(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the caller go
    thrThis.Sync();

    // We need a local sample list to query the server with
    tCQLogicSh::TSampleList fcolSamples(kCQLogicSh::c4GraphSampleCnt);

    // We need a client proxy to talk to the server with
    tCQLogicSh::TLogicSrvProxy orbcLS;
    tCIDLib::TEncodedTime      enctNextConn = 0;

    m_bStatus = kCIDLib::False;
    while(kCIDLib::True)
    {
        //
        //  Use the breakable block to wait for a while. If asked to shutdown
        //  while we were sleeping, then break out.
        //
        if (!thrThis.bSleep(250))
            break;

        try
        {
            // Lock while we check for things to do
            TLocker lockrSync(&m_mtxSync);

            //
            //  First see if we can drop any cache items. Doing this first means
            //  that we can't get stuck always trying to reconnect because the
            //  cache isn't empty, but unable to drop the now old cache items
            //  because we never get connected and get to the drop check code.
            //
            //  We calc a drop time some minutes into the past. Any item which
            //  was last accessed by client code prior to that is dropped.
            //
            tCIDLib::TEncodedTime       enctCur = TTime::enctNow();
            const tCIDLib::TEncodedTime enctDropPoint
            (
                enctCur - (kCIDLib::enctOneMinute * 2)
            );
            tCIDLib::TCard4 c4Count = m_colCache.c4ElemCount();
            tCIDLib::TCard4 c4Index = 0;
            while(c4Index < c4Count)
            {
                TCacheItem& itemCur = m_colCache[c4Index];
                if (itemCur.m_enctLastAccess < enctDropPoint)
                {
                    // It's dead, so remove it and reduce the count
                    c4Count--;
                    m_colCache.RemoveAt(c4Index);
                }
                 else
                {
                    //
                    //  Still ok, so move forward. But if we are not connected
                    //  to the logic server right now, make sure that it has no
                    //  samples, so that the clients will clear their graphs until
                    //  we get in sync again.
                    //
                    c4Index++;
                }
            }

            //
            //  If nothing in the cache, then nothing to do. If we have a conn
            //  then drop it. Release the lock while we do that, in case it
            //  should take a little time.
            //
            // !!!!!!!!!
            //
            //  We are releasing the mutex here to drop the connection, so don't
            //  access the cache after it's released.
            //
            if (m_colCache.bIsEmpty())
            {
                //
                //  Release the lock before we do this, in case it should block
                //  for a little bit. We are going back to the top to sleep so
                //  it doesn't matter now anyway. Before we release, reset all
                //  of the items.
                //
                if (orbcLS.pobjData() != nullptr)
                {
                    ResetCacheItems();

                    lockrSync.Release();
                    orbcLS.DropRef();
                    m_bStatus = kCIDLib::False;
                }

                // And to back and wait again
                continue;
            }

            //
            //  If we have stuff in the cache but we are not connected, then
            //  try to connect. We know the stuff in the cache is still good
            //  since we dropped any stale ones above. So we do need to try
            //  to get connected.
            //
            if (!m_colCache.bIsEmpty() && !orbcLS.pobjData())
            {
                try
                {
                    // See if it's been long enough to try the server again
                    if (enctNextConn < TTime::enctNow())
                    {
                        orbcLS = orbcLogicSrvProxy();

                        //
                        //  We connected so zero out the next connect time for
                        //  the next time we need to reconnect.
                        //
                        enctNextConn = 0;
                        m_bStatus = kCIDLib::True;
                    }
                }

                catch(...)
                {
                    // It failed so set the next time we can check
                    m_bStatus = kCIDLib::False;
                    enctNextConn = TTime::enctNowPlusSecs(5);
                }
            }

            // If we didn't get connected, go back and wait again
            if (!orbcLS.pobjData())
                continue;

            //
            //  Ok, we potentially have work to do and seem to be connected.
            //  So go through them and see if any are ready to poll.
            //
            enctCur = TTime::enctNow();
            c4Count = m_colCache.c4ElemCount();
            for (c4Index = 0; c4Index < c4Count; c4Index++)
            {
                //
                //  Do shutdown request checks since this could take a bit if
                //  they have multiple graphs.
                //
                if (thrThis.bCheckShutdownRequest())
                    break;

                // Get the current item and see if it's time to poll it
                TCacheItem& itemCur = m_colCache[c4Index];
                if (itemCur.m_enctNextPoll < enctCur)
                {
                    //
                    //  OK, let's poll this guy. We have to temporarily
                    //  release the mutex lock to do this. We pass in the
                    //  current serial numbers that we have, and should get
                    //  back the new ones if any new data.
                    //
                    lockrSync.Release();

                    tCIDLib::TCard4 c4NewCnt;
                    tCIDLib::TCard4 c4SerNum = itemCur.m_grdatSamples.c4SerialNum();
                    const tCIDLib::TBoolean bNew = orbcLS->bQueryGraphSamples
                    (
                        itemCur.m_strName
                        , itemCur.m_c4GraphCfgSerNum
                        , c4SerNum
                        , c4NewCnt
                        , fcolSamples
                    );

                    //
                    //  !!!!!!!
                    //
                    //  Get the lock back now, before we continue, or very
                    //  bad things could happen.
                    //
                    lockrSync.Lock();

                    // If we got any new samples, then update this item
                    if (bNew)
                        itemCur.StoreNewSamples(fcolSamples, c4NewCnt, c4SerNum);

                    // Update the current time in case this took a bit
                    enctCur = TTime::enctNow();

                    //
                    //  Update this guy's poll check time, whether we got any
                    //  samples or not this time.
                    //
                    itemCur.m_enctNextPoll = enctCur + (kCIDLib::enctOneSecond * 10);
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQLogicSh().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            //
            //  If we lost the logic server, then reset the proxy so that we
            //  will start trying to reconnect.
            //
            if ((orbcLS.pobjData() != 0)
            &&  orbcLS->bCheckForLostConnection(errToCatch))
            {
                try
                {
                    // Lock and reset all of the cache items
                    {
                        TLocker lockrSync(&m_mtxSync);
                        ResetCacheItems();
                    }

                    // And drop our connection
                    orbcLS.DropRef();
                    m_bStatus = kCIDLib::False;
                }

                catch(TError& errToCatch)
                {
                    if (facCQLogicSh().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        LogEventObj(errToCatch);
                    }
                }
            }
        }

        catch(...)
        {
            if (facCQLogicSh().bLogFailures())
            {
                facCQLogicSh().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQLShErrs::errcRem_PollErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Unknown
                );
            }

            if ((orbcLS.pobjData() != 0) && orbcLS->bCheckForLostConnection())
            {
                try
                {
                    // Lock and reset all of the cache items
                    {
                        TLocker lockrSync(&m_mtxSync);
                        ResetCacheItems();
                    }

                    orbcLS.DropRef();
                    m_bStatus = kCIDLib::False;
                }

                catch(TError& errToCatch)
                {
                    if (facCQLogicSh().bShouldLog(errToCatch))
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        LogEventObj(errToCatch);
                    }
                }
            }
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  If we lose connection to the logic server any existing cache items are reset
//  so that clients will clear their content and so that the poller will start
//  getting new data again.
//
//  The caller must have already locked
//
tCIDLib::TVoid TFacCQLogicSh::ResetCacheItems()
{
    const tCIDLib::TCard4 c4Count = m_colCache.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        m_colCache[c4Index].Reset();
}

