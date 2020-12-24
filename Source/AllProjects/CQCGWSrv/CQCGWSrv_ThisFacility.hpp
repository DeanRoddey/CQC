//
// FILE NAME: CQCGWSrv_ThisFacility.hpp
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
//  This is the header for the facility class for the gateway server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCGWSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCGWSrv : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCGWSrv();

        TFacCQCGWSrv(const TFacCQCGWSrv&) = delete;
        TFacCQCGWSrv(TFacCQCGWSrv&&) = delete;

        ~TFacCQCGWSrv() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCGWSrv& operator=(const TFacCQCGWSrv&) = delete;
        TFacCQCGWSrv& operator=(TFacCQCGWSrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TCQCPollEngine& polleThis() noexcept;

        TString strCertInfo() const noexcept
        {
            return m_strCertInfo;
        }

        tCIDLib::TVoid Shutdown
        (
            const   tCIDLib::EExitCodes     eReturn
        );

        TUniquePtr<TSockLEngConn> uptrWait
        (
            const   tCIDLib::TCard4         c4WaitMSs
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCQCSrvFW::EStateRes eProcessParms
        (
                    tCIDLib::TKVPList::TCursor& cursParms
        )   final;

        tCIDLib::TVoid PostDeregTerm() final;

        tCIDLib::TVoid PostUnbindTerm() final;

        tCIDLib::TVoid PreRegInit() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   final;

        tCIDLib::TVoid StartWorkerThreads() final;

        tCIDLib::TVoid StopWorkerThreads() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TWorkerThreadList = TRefVector<TWorkerThread>;


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxGWClients
        //      The max simultaneous gateway clients to support. The gateway
        //      clients are NOT ORB based, so we need separate settings for
        //      these connections. If not set, it will be defaulted.
        //
        //  m_colWorkerThreads
        //      A by reference collection of worker threads. They all block
        //      on the connection queue waiting for clients. For this simple
        //      server we use a 'thread per server' paradigm, so each thread
        //      serves a client exclusively until the client disconnects.
        //
        //  m_ippnGWListen
        //  m_ippnGWListenSec
        //      The ports we listen on for gateway connections, which are NOT
        //      ORB based, so we need a different port for these connections.
        //      We on two ports, one for secure and one for insecure connections.
        //      Secure connections require a certificate.
        //
        //      If we get no certificate info, the secure one is set to zero,
        //      which tells the listener engine to not use it! So don't default
        //      it.
        //
        //  m_polleThis
        //      The polling engine that we make use of to manage our polling.
        //      It will manage all of the fields we want to monitor, keep
        //      them up to data via polling, attempt to reconnect to lost
        //      servers, and so on.
        //
        //      In order to be more efficient, we use one engine instance and
        //      share it between all worker threads. This way, we only have
        //      one connection from here to each CQCServer, and don't do
        //      redundant polling of fields being viewed by multiple clients.
        //
        //  m_sleServer
        //      We use a socket listener engine, since we have to listen on two
        //      seperate ports potentially. We may only be configured for secure
        //      or non-secure connections, but it can be both as well.
        //
        //  m_strCertInfo
        //      This is the info on the certificate to use for secure connections.
        //      If not provide, then we don't listn on secure port.
        //
        //  m_unamThread
        //      A unique namer to create unique names for our worker threads
        //      as we spin them up.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4MaxGWClients;
        TWorkerThreadList       m_colWorkerThreads;
        tCIDLib::TIPPortNum     m_ippnGWListen;
        tCIDLib::TIPPortNum     m_ippnGWListenSec;
        TCQCPollEngine          m_polleThis;
        TSockListenerEng        m_sleServer;
        TString                 m_strCertInfo;
        TUniqueName             m_unamThreads;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCGWSrv,TCQCSrvCore)
};

#pragma CIDLIB_POPPACK

