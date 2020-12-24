//
// FILE NAME: CQCWebSrvC_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This is the header for the facility class for the Web Server client library.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWebSrvC
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCWEBSRVCEXP TFacCQCWebSrvC : public TFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCWebSrvC();

        TFacCQCWebSrvC(const TFacCQCWebSrvC&) = delete;
        TFacCQCWebSrvC(TFacCQCWebSrvC&&) = delete;

        ~TFacCQCWebSrvC();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCWebSrvC& operator=(const TFacCQCWebSrvC&) = delete;
        TFacCQCWebSrvC& operator=(TFacCQCWebSrvC&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFindHdrLine
        (
            const   TString&                strKey
            ,       TString&                strValue
            , const tCIDLib::TKVPList&      colHdrLines
        );

        tCIDLib::TBoolean bWaitConn
        (
                    TServerStreamSocket*&   psockClient
            ,       tCIDLib::TBoolean&      bSecure
            ,       TIPEndPoint&            ipepClient
        );

        tCIDLib::TVoid DecWSOBufCount();

        tCIDLib::TIPPortNum ippnWSListen() const;

        tCIDLib::TIPPortNum ippnWSListenSec() const;

        tCIDLib::TVoid IncWSOBufCount();

        tCIDLib::TVoid InitCMLRuntime();

        tCIDLib::TVoid InitSimMode();

        tCIDLib::TVoid InitWebEngine
        (
            const   tCIDLib::TIPPortNum     ippnHTTP
            , const TString&                strHTMLDir
            , const tCIDLib::TIPPortNum     ippnHTTPS
            , const TString&                strCertInfo
            , const TCQCUserCtx             cuctxToUse
            , const TMD5Hash&               mhashSrvPW
        );

        const TCQCUserCtx& cuctxToUse() const;

        const TMD5Hash& mhashSrvPW() const
        {
            return m_mhashSrvPW;
        }

        TCQCPollEngine& polleThis();

        const TCQCSecToken& sectUser() const;

        const TString& strCertInfo() const;

        const TString& strRootHTMLPath() const;

        tCIDLib::TVoid StartWebsockSession
        (
                    TCIDDataSrcJan&         janSrc
            , const TURL&                   urlReq
            , const TString&                strSecKey
            , const TString&                strURLResource
        );

        tCIDLib::TVoid Terminate();

        tCIDLib::TVoid WriteField
        (
            const   TString&                strField
            , const TString&                strValue
        );

        tCIDLib::TVoid WriteField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TWorkerThreadList = TRefVector<TWorkerThread>;
        using TWebsockThreadList = TRefVector<TWebsockThread>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eConnThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDoWRV
        //      Indicates whether the Web RIVA client is enabled for this system. THe name
        //      is purposefully obscure.
        //
        //  m_bInitCML
        //  m_bInitEngine
        //      Used to remember if we've already initialized the CML runtime or the
        //      the web engine. To avoid doing it multiple times.
        //
        //  m_colConnQ
        //      Our connection thread monitors the listener engine and drops the
        //      connections into this queue. The worker threads call bWaitConn
        //      which blocks them on this queue. It's thread safe.
        //
        //  m_colWebsockThreads
        //      If a worker thread (see m_colWorkerThreads) sees that a client is
        //      asking to upgrade to websockets. It will do some basic checking and
        //      if it looks semi-reasonable, it will call us to create a web socket
        //      thread object, and hand off the client to that new object. We look
        //      through the list for one that is not currently in use, or create a new
        //      one if necessary) and start it up with the new client info. THe
        //      original worker thread returns immediately to process more HTTP
        //      requests.
        //
        //  m_colWorkerThreads
        //      A by reference collection of worker threads. They all block on the
        //      connection queue waiting for clients. For this simple server we use
        //      a 'thread per server' paradigm, so each thread serves a client
        //      exclusively for an HTTP session (either one shot or persistent.)
        //      This guy does not need to be thread safe. Some threads are started
        //      initially before we crank up the connection thread. Then the conn
        //      thread manages it, then the threads are stopped after the conn
        //      thread is stopped.
        //
        //  m_cuctxToUse
        //      The containing application must give us a valid user context, of at
        //      least normal user rights, so that we can access resources we need.
        //
        //  m_ippnWSListen
        //  m_ippnWSListenSec
        //      The ports we listen on for incoming HTTP requests. These are defaulted to
        //      zero and we only use the ones that get set to non-zero by client code during
        //      init call.
        //
        //  m_mhashSrvPW
        //      The web server facility passes this along to us. We need it to decrypt
        //      the web passwords we get from the security server.
        //
        //  m_polleThis
        //      The polling engine that we make use of to manage the field polling
        //      for the web sockets clients, though we could also make it available
        //      to serve up field data for HTTP clients as well.
        //
        //      We use a single, shared one, since that's far more efficient.
        //
        //  m_sleServer
        //      Our listener engine which handles listening for client connections for
        //      us. Our worker threads just block on it, waiting for new connections
        //      to become available.
        //
        //  m_strCertInfo
        //      The certifiate info string we get during init. It's needed to handle
        //      secure connections.
        //
        //  m_strRootHTMLPath
        //      The base path that any on-disk HTML files are relative to.
        //
        //  m_thrConn
        //      Though the listener engine handles listening and queuing up
        //      connections, in our case we want to be able to grow our list of
        //      worker threads if the load increases so we need some form of
        //      oversight. So we use this single thread to monitor the listener
        //      engine and then move those connection objects to our own queue,
        //      which the worker threads monitor.
        //
        //  m_unamThread
        //      A unique namer to create unique names for our worker threads as we
        //      spin them up.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bInitCML;
        tCIDLib::TBoolean               m_bInitEngine;
        TSockListenerEng::TConnQueue    m_colConnQ;
        TWebsockThreadList              m_colWebsockThreads;
        TWorkerThreadList               m_colWorkerThreads;
        TCQCUserCtx                     m_cuctxToUse;
        tCIDLib::TIPPortNum             m_ippnWSListen;
        tCIDLib::TIPPortNum             m_ippnWSListenSec;
        TMD5Hash                        m_mhashSrvPW;
        TCQCPollEngine                  m_polleThis;
        TSockListenerEng                m_sleServer;
        TString                         m_strCertInfo;
        TString                         m_strRootHTMLPath;
        TThread                         m_thrConn;
        TUniqueName                     m_unamThreads;


        // -------------------------------------------------------------------
        //  Private, static methods
        //
        //  s_sciUsedWSBufs
        //      Tracks the number of web socket output buffers currently in use
        // -------------------------------------------------------------------
        static TStatsCacheItem          s_sciUsedWSOBufs;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCWebSrvC,TFacility)
};

#pragma CIDLIB_POPPACK


