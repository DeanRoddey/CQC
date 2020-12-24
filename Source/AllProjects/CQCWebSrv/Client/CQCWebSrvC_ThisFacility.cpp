//
// FILE NAME: CQCWebSrvC_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2014
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
//  This file implements the facility class for the CQC Web Server Client library.
//
//  We also implement our simple CML class loader, since it's only needed inside
//  this file and doesn't need to be exported.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCWebSrvC,TFacility)




// ---------------------------------------------------------------------------
//  CLASS: TCQCWSCClassLoader
// PREFIX: mecl
// ---------------------------------------------------------------------------
class TCQCWSCClassLoader : public MMEngExtClassLoader
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCWSCClassLoader() {}

        TCQCWSCClassLoader(const TCQCWSCClassLoader&) = delete;
        TCQCWSCClassLoader(TCQCWSCClassLoader&&) = delete;

        ~TCQCWSCClassLoader() {}


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCWSCClassLoader& operator=(const TCQCWSCClassLoader&) = delete;
        TCQCWSCClassLoader& operator=(TCQCWSCClassLoader&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMEngClassInfo* pmeciLoadClass
        (
                    TCIDMacroEngine&        meTarget
            , const TString&                strClassPath
        )   final;
};


// ---------------------------------------------------------------------------
//  TCQCRTClassLoader: Public, inherited methods
// ---------------------------------------------------------------------------
TMEngClassInfo*
TCQCWSCClassLoader::pmeciLoadClass(         TCIDMacroEngine&    meTarget
                                    , const TString&            strClassPath)
{
    TMEngClassInfo* pmeciRet = nullptr;

    if (strClassPath == TCMLBinBaseInfo::strPath())
        pmeciRet = new TCMLBinBaseInfo(meTarget);
    else if (strClassPath == TCMLWSockBaseInfo::strPath())
        pmeciRet = new TCMLWSockBaseInfo(meTarget);
    return pmeciRet;
}




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWebSrvC
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCWebSrvC: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCWebSrvC::TFacCQCWebSrvC() :

    TFacility
    (
        L"CQCWebSrvC"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bInitCML(kCIDLib::False)
    , m_bInitEngine(kCIDLib::False)
    , m_colConnQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_colWebsockThreads(tCIDLib::EAdoptOpts::Adopt, 4, tCIDLib::EMTStates::Safe)
    , m_colWorkerThreads
      (
        tCIDLib::EAdoptOpts::Adopt
        , kCQCWebSrvC::c4InitWorkerThreads
        , tCIDLib::EMTStates::Unsafe
      )
    , m_ippnWSListen(0)
    , m_ippnWSListenSec(0)
    , m_thrConn
      (
        facCIDLib().strNextThreadName(TString(L"WebSrvConn"))
        , TMemberFunc<TFacCQCWebSrvC>(this, &TFacCQCWebSrvC::eConnThread)
      )
    , m_unamThreads(L"CQCWebSrvWorkerThread%(1)")
{
    // Register our statistics
    TStatsCache::RegisterItem
    (
        L"/Stats/CQCWebSrv/UsedWSOBufs", tCIDLib::EStatItemTypes::Counter, s_sciUsedWSOBufs
    );

    //
    //  And temporarily we have to also indicate WebRIVA until the old RIVA clients are
    //  retired.
    //
    TFacCQCIntfEng::bWebRIVAMode(kCIDLib::True);
}

TFacCQCWebSrvC::~TFacCQCWebSrvC()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCWebSrvC: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Used to look up a specific header line, if present, in a list of incoming
//  HTTP header lines.
//
tCIDLib::TBoolean
TFacCQCWebSrvC::bFindHdrLine(const  TString&            strKey
                            ,       TString&            strValue
                            , const tCIDLib::TKVPList&  colHdrLines)
{
    const tCIDLib::TCard4 c4Count = colHdrLines.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValuePair& kvalCur = colHdrLines[c4Index];
        if (kvalCur.strKey() == strKey)
        {
            strValue = kvalCur.strValue();
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  The worker threads call into here to wait for connections. They just block on
//  our connection queue.
//
tCIDLib::TBoolean
TFacCQCWebSrvC::bWaitConn(  TServerStreamSocket*&   psockClient
                            , tCIDLib::TBoolean&    bSecure
                            , TIPEndPoint&          ipepClient)
{
    TSockLEngConn* pslecNew = m_colConnQ.pobjGetNext(500, kCIDLib::False);
    if (!pslecNew)
    {
        // Nothing available, so just return
        psockClient = 0;
        return kCIDLib::False;
    }

    // We got one so get the info out
    psockClient = pslecNew->psockConn();
    ipepClient = pslecNew->ipepClient();
    bSecure = pslecNew->bSecure();

    // Orphan the socket and delete the connection object
    pslecNew->Orphan();
    delete pslecNew;

    return kCIDLib::True;
}

// Decrement the count of used web socket output buffers statistic
tCIDLib::TVoid TFacCQCWebSrvC::DecWSOBufCount()
{
    TStatsCache::c8DecCounter(s_sciUsedWSOBufs);
}


//
//  Provide access to our two listening ports, so that the worker threads can figure
//  out what mode they were invoked in (by comparing the server side end point of
//  the connection to these.)
//
tCIDLib::TIPPortNum TFacCQCWebSrvC::ippnWSListen() const
{
    return m_ippnWSListen;
}

tCIDLib::TIPPortNum TFacCQCWebSrvC::ippnWSListenSec() const
{
    return m_ippnWSListenSec;
}


// Increment the count of used web socket output buffers statistic
tCIDLib::TVoid TFacCQCWebSrvC::IncWSOBufCount()
{
    TStatsCache::c8IncCounter(s_sciUsedWSOBufs);
}


//
//  The web server will call this to crank up he web server engine. It must also
//  call InitializeCMLRuntime() to load our CML class loader as well. These are
//  separated out because the MacroWS just initializes the CML runtime initially. If
//  the CML class derives one of our CML handler types and invokes a simulator method
//  that will call this to crank up the web server engine.
//
tCIDLib::TVoid TFacCQCWebSrvC::InitCMLRuntime()
{
    // If already initialized then do nothing
    if (m_bInitCML)
        return;

    // Remember we've initialized
    m_bInitCML = kCIDLib::True;

    // Set an instance of our class loader
    facCIDMacroEng().AddClassLoader(new TCQCWSCClassLoader);
}


tCIDLib::TVoid TFacCQCWebSrvC::InitSimMode()
{
    // The runtime we can do as is
    InitCMLRuntime();

    // And start up the poll engine
    m_polleThis.StartEngine(m_cuctxToUse.sectUser());
}

tCIDLib::TVoid
TFacCQCWebSrvC::InitWebEngine(  const   tCIDLib::TIPPortNum ippnHTTP
                                , const TString&            strHTMLDir
                                , const tCIDLib::TIPPortNum ippnHTTPS
                                , const TString&            strCertInfo
                                , const TCQCUserCtx         cuctxToUse
                                , const TMD5Hash&           mhashSrvPW)
{
    // If already initialized then do nothing
    if (m_bInitEngine)
        return;

    // Remember we've initialized
    m_bInitEngine = kCIDLib::True;

    // Store the user context and any other flags
    m_cuctxToUse = cuctxToUse;

    //
    //  If an HTML root path wasn't set explicitly, then create one relative
    //  to the data dir.
    //
    if (strHTMLDir.bIsEmpty())
    {
        TPathStr pathTmp = facCQCKit().strDataDir();
        pathTmp.AddLevel(L"HTMLRoot");
        m_strRootHTMLPath = pathTmp;
    }
     else
    {
        m_strRootHTMLPath = strHTMLDir;
    }

    // Take whatever certificate info we were given
    m_strCertInfo = strCertInfo;

    // And we need to store away the server password has for later use
    m_mhashSrvPW = mhashSrvPW;

    //
    //  If either port is non-zero, override our defaults set in the ctor. However,
    //  if we got no certificate info, force the secure port to zero since we can't
    //  do secure connections without one.
    //
    if (ippnHTTP)
        m_ippnWSListen = ippnHTTP;

    if (m_strCertInfo.bIsEmpty())
        m_ippnWSListenSec = 0;
    else if (ippnHTTPS)
        m_ippnWSListenSec = ippnHTTPS;

    //
    //  One of them should be initialized. If not, we don't do anything, since nothing
    //  is going to work. They would have to terminate us and reinit.
    //
    if ((m_ippnWSListenSec == 0) && (m_ippnWSListen == 0))
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCErrs::errcCfg_NoPorts
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
    }
     else
    {
        // And start up the polling engine
        m_polleThis.StartEngine(m_cuctxToUse.sectUser());

        //
        //  And initialize our listener engine. If the secure port is not enabled,
        //  it'll be zero, which will tell the engine not to listen on that one.
        //  Let it pick a default for max wait count.
        //
        m_sleServer.Initialize
        (
            tCIDSock::ESockProtos::TCP
            , m_ippnWSListen
            , m_ippnWSListenSec
            , 0
        );

        //
        //  And finally Spin up some initial worker threads now that everything is
        //  in place.
        //
        //  NOTE:   This collection is not thread safe. We have not started the connect
        //          proceessing thread yet and he is the only other one that deals
        //          with this list, and he's stopped before we clean this list up,
        //          so it's never accessed by more than one thread at a time.
        //
        tCIDLib::TCard4 c4Index;
        for (c4Index = 0; c4Index < kCQCWebSrvC::c4InitWorkerThreads; c4Index++)
            m_colWorkerThreads.Add(new TWorkerThread(m_unamThreads.strQueryNewName()));
        for (c4Index = 0; c4Index < kCQCWebSrvC::c4InitWorkerThreads; c4Index++)
            m_colWorkerThreads[c4Index]->Start();


        // We have to initialize the interface engine and give him a polling engine to use
        facCQCIntfEng().StartEngine(&m_polleThis);

        //
        //  And now start the connection thread. At this point we will start processing
        //  any incoming connections.
        //
        m_thrConn.Start();
    }
}


//
//  Provide access to the user context we maintain. The worker threads will
//  want to get it.
//
const TCQCUserCtx& TFacCQCWebSrvC::cuctxToUse() const
{
    return m_cuctxToUse;
}


TCQCPollEngine& TFacCQCWebSrvC::polleThis()
{
    return m_polleThis;
}


// A convenience wrapper that we just pass on to the user context
const TCQCSecToken& TFacCQCWebSrvC::sectUser() const
{
    return m_cuctxToUse.sectUser();
}


// Provide access to the certificate info we were given
const TString& TFacCQCWebSrvC::strCertInfo() const
{
    return m_strCertInfo;
}

// Provides access to the HTML root path, mainly for the worker threads
const TString& TFacCQCWebSrvC::strRootHTMLPath() const
{
    return m_strRootHTMLPath;
}


//
//  The main HTTP handling worker thread will, upon seeing an upgrade request to
//  web sockets type HTTP transaction, call us here to hand off the socket to a
//  a websockets handler. He will then return to continue processing HTTP reqeuests.
//
//  We will orphan the socket out of the janitor, and the new websocket handler is
//  responsible for it. The caller should not try to access it again.
//
tCIDLib::TVoid
TFacCQCWebSrvC::StartWebsockSession(        TCIDDataSrcJan&         janSrc
                                    , const TURL&                   urlReq
                                    , const TString&                strSecKey
                                    , const TString&                strURLResource)
{
    //
    //  We need to figure out which type of web socket connection it is. We have to
    //  find a fallow one or create a new one of that type. It should have already been
    //  proven that the URL resource represents a valid path (though in the case of
    //  CML based handlers the class it resolves to might not exist.)
    //
    tCQCWebSrvC::EWSockTypes eType;
    if (strURLResource.bStartsWithI(L"/Websock/User"))
        eType = tCQCWebSrvC::EWSockTypes::CML;
    else if (strURLResource.bCompareI(kCQCWebRIVA::strURL_WebWSRIVA))
        eType = tCQCWebSrvC::EWSockTypes::RIVA;
    else
    {
        CIDAssert2(L"Unknown Websock handler type");
    }


    //
    //  See if we can find an idle web socket object to use. We have to lock while
    //  doing this, until we get the new object started up and in active state.
    //
    //  The collection is created thread safe so that we can just use that.
    //
    TLocker lockrSync(&m_colWebsockThreads);

    TWebsockThread* pthrNew = 0;
    const tCIDLib::TCard4 c4Count = m_colWebsockThreads.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TWebsockThread* pthrCur = m_colWebsockThreads[c4Index];

        // If this one is running, skip it
        if (pthrCur->bIsRunning())
            continue;

        // Take this guy if it's of the type we need
        if (pthrCur->eType() == eType)
            pthrNew = pthrCur;
        break;
    }

    // If we didn't get one, create a new one and add it
    if (!pthrNew)
    {
        if (eType == tCQCWebSrvC::EWSockTypes::CML)
            pthrNew = new TWebsockCMLThread(kCIDLib::False);
        else if (eType == tCQCWebSrvC::EWSockTypes::RIVA)
            pthrNew = new TWebSockRIVAThread(tCIDLib::TCard1(c4Count));
        else
        {
            CIDAssert2(L"Unknown Websock handler type");
        }

        m_colWebsockThreads.Add(pthrNew);
    }

    //
    //  Set up the new guy, If it works, he will adopt the socket and will start him
    //  self up running as a separate thread.
    //
    pthrNew->Reset(janSrc, urlReq, strSecKey);
}


//
//  Anything done in Initialize() is undone here. The client application should call this
//  before exiting if it has initialized.
//
tCIDLib::TVoid TFacCQCWebSrvC::Terminate()
{
    //
    //  Stop the connection thread before we stop the listener. This one should
    //  stop pretty fast since it's not doing much.
    //
    if (m_thrConn.bIsRunning())
    {
        try
        {

            m_thrConn.ReqShutdownSync();
            m_thrConn.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    // Clean up the listener engine
    try
    {
        m_sleServer.Cleanup();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);

        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcApp_TermListener
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    catch(...)
    {
        // Log it but not much else we can do
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcApp_TermListener
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::Term
        );
    }

    //
    //  Shut down all of the worker threads and websocket threads. We make one pass
    //  and start them all stopping, then we do another waiting for them to stop.
    //
    const tCIDLib::TCard4 c4WorkerCount = m_colWorkerThreads.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WorkerCount; c4Index++)
    {
        try
        {
            m_colWorkerThreads[c4Index]->ReqShutdownNoSync();
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    const tCIDLib::TCard4 c4WSockCount = m_colWebsockThreads.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WSockCount; c4Index++)
    {
        try
        {
            m_colWebsockThreads[c4Index]->ReqShutdownNoSync();
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WebsockTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WebsockTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WorkerCount; c4Index++)
    {
        try
        {
            m_colWorkerThreads[c4Index]->eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WorkerTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WSockCount; c4Index++)
    {
        try
        {
            m_colWebsockThreads[c4Index]->eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            if (bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                LogEventObj(errToCatch);
            }

            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WebsockTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }

        catch(...)
        {
            if (bLogStatus())
            {
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCMsgs::midStatus_WebsockTerm
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                );
            }
        }
    }

    // Stop the interface engine
    try
    {
        facCQCIntfEng().StopEngine();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_IVEngineTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }

    catch(...)
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_IVEngineTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }

    //
    //  Stop the polling engine if started. This might be started even if the
    //  engine as a whole wasn't initilized, since it's started in simulator mode
    //  within the CML IDE.
    //
    //  We don't want to do this until the worker and web socket threads are
    //  stopped since they may be accessing the polling engine.
    //
    try
    {
        m_polleThis.StopEngine();
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_PollEngineTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }

    catch(...)
    {
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCWSCMsgs::midStatus_PollEngineTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }


    // And we are no longer initialized
    m_bInitEngine = kCIDLib::False;
}


tCIDLib::TVoid TFacCQCWebSrvC::WriteField(const TString& strField, const TString& strValue)
{
    m_polleThis.WriteField(strField, strValue, sectUser());
}

tCIDLib::TVoid
TFacCQCWebSrvC::WriteField(const TString& strMoniker, const TString& strField, const TString& strValue)
{
    TString strFullFld(TStrCat(strMoniker, L".", strField));
    m_polleThis.WriteField(strFullFld, strValue, sectUser());
}


// ---------------------------------------------------------------------------
//  TFacCQCRemVComm: Private, static data members
// ---------------------------------------------------------------------------
TStatsCacheItem TFacCQCWebSrvC::s_sciUsedWSOBufs;



// ---------------------------------------------------------------------------
//  TFacCQCWebSrvC: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The m_thrConn object is started up on this method, and just watches the
//  listener engine for connections, putting them into our queue. We also handle
//  spinning up more worker threads if we get backed up.
//
tCIDLib::EExitCodes
TFacCQCWebSrvC::eConnThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the caller go
    thrThis.Sync();

    // And we just wait until asked to shut down, waiting for connections.
    while (kCIDLib::True)
    {
        if (thrThis.bCheckShutdownRequest())
            break;

        try
        {
            // Wait up to a second for a connection
            TSockLEngConn* pslecNew = m_sleServer.pslecWait(250);
            if (pslecNew)
            {
                TJanitor<TSockLEngConn> janConn(pslecNew);
                tCIDLib::TBoolean bAccept = kCIDLib::True;

                //
                //  If the already queued connections haven't already been
                //  processed, then see if we can start up another thread. These
                //  guys should be gotten to very quickly if all worker threads
                //  are not busy.
                //
                if (m_colConnQ.c4ElemCount() > 1)
                {
                    if (m_colWorkerThreads.bIsFull(kCIDSock::c4MaxSelect))
                    {
                        // We are full, can't take it
                        if (facCQCWebSrvC().bLogWarnings())
                        {
                            LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQCWSCErrs::errcGen_Full
                                , tCIDLib::ESeverities::Warn
                                , tCIDLib::EErrClasses::OutResource
                            );
                        }

                        // Indicate we cannot accept it
                        bAccept = kCIDLib::False;
                    }
                     else
                    {
                        if (facCQCWebSrvC().bLogInfo())
                        {
                            LogMsg
                            (
                                CID_FILE
                                , CID_LINE
                                , kCQCWSCMsgs::midStatus_NewWorker
                                , tCIDLib::ESeverities::Status
                                , tCIDLib::EErrClasses::AppStatus
                            );
                        }

                        // We have more capacity so spin up another thread
                        TWorkerThread* pthrNew = new TWorkerThread(m_unamThreads.strQueryNewName());
                        m_colWorkerThreads.Add(pthrNew);
                        pthrNew->Start();
                    }
                }

                //
                //  If accepting, then orphan it out into the connection queue. Else
                //  let the janitor clean it up.
                //
                if (bAccept)
                    m_colConnQ.Add(janConn.pobjOrphan());
            }
        }

        catch(TError& errToCatch)
        {
            if (facCQCWebSrvC().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        catch(...)
        {
        }
    }

    return tCIDLib::EExitCodes::Normal;
}


