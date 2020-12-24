//
// FILE NAME: CQCGWSrv_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2002
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
//  This file implements the facility class for the CQC gateway server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGWSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCGWSrv, TCQCSrvCore)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGWSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCGWSrv: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We don't ask for a login from server core. We require a login for each client
//  connection and use those credentials for any privileged operations.
//
TFacCQCGWSrv::TFacCQCGWSrv() :

    TCQCSrvCore
    (
        L"CQCGWSrv"
        , L"CQC XML Gateway Server"
        , kCQCKit::ippnCQCGWSrvDefPort
        , kCQCGWSrv::pszEvGWSrv
        , tCIDLib::EModFlags::HasMsgFile
        , tCQCSrvFW::ESrvOpts::None
    )
    , m_c4MaxGWClients(kCQCGWSrv::c4DefMaxGWClients)
    , m_colWorkerThreads
      (
        tCIDLib::EAdoptOpts::Adopt
        , kCQCGWSrv::c4InitWorkerThreads
        , tCIDLib::EMTStates::Safe
      )
    , m_ippnGWListen(0)
    , m_ippnGWListenSec(0)
    , m_sleServer()
    , m_unamThreads(L"CQCGWSrvWorkerThread%(1)")
{
}


// ---------------------------------------------------------------------------
//  TFacCQCGWSrv: Public, non-virtual methods
// ---------------------------------------------------------------------------

TCQCPollEngine& TFacCQCGWSrv::polleThis() noexcept
{
    return m_polleThis;
}


// ---------------------------------------------------------------------------
//  TFacCQCGWSrv: Protected, inherited methods
// ---------------------------------------------------------------------------

// We don't use the extra values, so just return the binding and description
tCIDLib::TVoid
TFacCQCGWSrv::QueryCoreAdminInfo(TString&   strCoreAdminBinding
                                , TString&  strCoreAdminDesc
                                , TString&
                                , TString&
                                , TString&
                                , TString&  )
{
    strCoreAdminBinding = kCQCKit::pszCQCGWSrvCoreAdmin;
    strCoreAdminDesc = L"CQC Gateway Server Core Admin Object";
}


// Process our command parameters at this level
tCQCSrvFW::EStateRes TFacCQCGWSrv::eProcessParms(tCIDLib::TKVPList::TCursor& cursParms)
{
    for (; cursParms; ++cursParms)
    {
        const TString& strKey = cursParms->strKey();
        const TString& strValue = cursParms->strValue();

        if (strKey.bCompareI(L"GWPort"))
        {
            m_ippnGWListen = strValue.c4Val();
        }
         else if (strKey.bCompareI(L"GWMax"))
        {
            m_c4MaxGWClients = strValue.c4Val();
        }
         else if (strKey.bCompareI(L"SecPort"))
        {
            m_ippnGWListenSec = strValue.c4Val();
        }
         else if (strKey.bCompareI(L"CertInfo"))
        {
            m_strCertInfo = strValue;
        }
         else
        {
            return tCQCSrvFW::EStateRes::Failed;
        }
    }

    // If we didn't get certificate info, insure the secure port is ignored, even if set
    if (m_strCertInfo.bIsEmpty())
        m_ippnGWListenSec = 0;

    // If at least one port isn't non-zero, then nothing to do
    if ((m_ippnGWListenSec == 0) && (m_ippnGWListen == 0))
    {
        facCQCGWSrv.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSErrs::errcCfg_NoPorts
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );

        // We can't continue since there's no recovering from this
        tCQCSrvFW::EStateRes::Failed;
    }

    return tCQCSrvFW::EStateRes::Success;
}


tCIDLib::TVoid TFacCQCGWSrv::PostDeregTerm()
{
    try
    {
        m_polleThis.StopEngine();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);

        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSMsgs::midStatus_EngineTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }
}


tCIDLib::TVoid TFacCQCGWSrv::PostUnbindTerm()
{
    // Stop the media database cacher engine if it got started
    try
    {
        facCQCMedia().StopMDBCacher();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);

        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCGWSMsgs::midStatus_CacherTerm
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Term
        );
    }
}


// Do stuff we need before our public interfaces are registered
tCIDLib::TVoid TFacCQCGWSrv::PreRegInit()
{
    //
    //  We have to install some class loaders on the macro engine facility,
    //  so that the standard CQC runtime and media access CML classes.
    //
    facCQCMEng().InitCMLRuntime(cuctxToUse().sectUser());
    facCQCMedia().InitCMLRuntime();
    facCQCGWCl().InitCMLRuntime();

    // Crank up the polling engine before we start advertising any services
    m_polleThis.StartEngine(cuctxToUse().sectUser());

    // Start up the media cacher
    facCQCMedia().StartMDBCacher();
}


tCIDLib::TVoid TFacCQCGWSrv::StartWorkerThreads()
{
    // Spin up some initial worker threads
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < kCQCGWSrv::c4InitWorkerThreads; c4Index++)
    {
        m_colWorkerThreads.Add(new TWorkerThread(m_unamThreads.strQueryNewName()));
    }

    for (c4Index = 0; c4Index < kCQCGWSrv::c4InitWorkerThreads; c4Index++)
        m_colWorkerThreads[c4Index]->Start();

    // And now start up our listener engine to let clients connect
    m_sleServer.Initialize(tCIDSock::ESockProtos::TCP, m_ippnGWListen, m_ippnGWListenSec);
}


tCIDLib::TVoid TFacCQCGWSrv::StopWorkerThreads()
{
    // Shut down the listener engine so we stop accepting connections
    try
    {
        m_sleServer.Cleanup();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
    }

    //
    //  Then we can do the worker threads overlapped, by asking them all to stop
    //  and then going back and waiting for them to end.
    //
    const tCIDLib::TCard4 c4WorkerCount = m_colWorkerThreads.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4WorkerCount; c4Index++)
    {
        try
        {
            m_colWorkerThreads[c4Index]->ReqShutdownSync(10000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCGWSMsgs::midStatus_WorkerTerm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
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
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCGWSMsgs::midStatus_WorkerTerm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }
}


//
//  Work threads call this to wait for a client connection. We just forward it to the
//  listener engine.
//
TUniquePtr<TSockLEngConn> TFacCQCGWSrv::uptrWait(const tCIDLib::TCard4 c4WaitMSs)
{
    return m_sleServer.uptrWait(c4WaitMSs);
}

