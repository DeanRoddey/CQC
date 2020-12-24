//
// FILE NAME: CQCClService_ThisFacility.hpp
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
//  This is the header for the facility class for the service. The main thread
//  as usually is pointed to here, and we manage the threads that do the actual
//  media data downloading work.
//
//  Note that, in real service mode (production build) the main thread is
//  started and managed by the service handler. In debug mode, unless the code
//  in the main cpp file is changed to do otherwise) we run as a standard
//  program. We handle signals for the debug scenario so that we can be stopped
//  by a Ctrl-C.
//
//  For each repo we find, we keep an object in a list. Each of them has their
//  own thread that does the downloading of data.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TZLibCompressor;


// ---------------------------------------------------------------------------
//   CLASS: TCacheThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TCacheThread : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Public, static methods
        // -------------------------------------------------------------------
        static const TString& strKey
        (
            const   TCacheThread&           thrSrc
        );


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCacheThread() = delete;

        TCacheThread
        (
            const   TString&                strRepo
        );

        TCacheThread(const TCacheThread&) = delete;
        TCacheThread(TCacheThread&&) = delete;

        ~TCacheThread();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCacheThread& operator=(const TCacheThread&) = delete;
        TCacheThread& operator=(TCacheThread&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4QueryCurData
        (
                    TMemBuf&                mbufToFill
        )   const;

        const TString& strRepoMoniker() const;

        const TString& strCurSerialNum() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Init() final;

        tCIDLib::TVoid Terminate() final;

        tCIDLib::EExitCodes eProcess() final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DownloadImgs
        (
                    tCQCKit::TCQCSrvProxy&  orbcSrv
            ,       TMediaDB&               mdbTest
            ,       THeapBuf&               mbufTmp
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4CurDataSz
        //      The latest database info we've downloaded is stored in the
        //      m_mbufCurData member, and this is the size of that data. This
        //      is kept around to provide to clients upon request. If we have
        //      not gotten data yet, it's zero.
        //
        //  m_c4FailCnt
        //      If we fail to contact our repo driver for more than a number
        //      of times in a raw, we stop our thread. The master thread will
        //      see that we aren't running and remove us from the list.
        //
        //  m_mbufCurData
        //      The latest downloaded data is stored here. This is the zlib
        //      compressed version to speed downloads by the clients who are
        //      coming to get the data, and to reduce memory usage.
        //
        //  m_mtxSync
        //      This is used to sync updates to the current data members,
        //      since both this thread and the incoming remote client ORB
        //      threads need to access it. So we need to make sure that this
        //      thead doesn't change it while it's being copied out to a
        //      client. It's mutable since it has to lock during const method
        //      calls.
        //
        //  m_strCurSerialNum
        //      The last serial number we got. We have to pass it back in to
        //      the repo driver to see if new data is available. We start it
        //      at zero, which insures we get initial data. We also return
        //      to clients on our end when they ask for the current data.
        //
        //  m_strOutPath
        //      The path that we output our information to. The clients will
        //      look here to find the info.
        //
        //  m_strRepoMoniker
        //      The repo for which we are set up to get data from.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4CurDataSz;
        tCIDLib::TCard4     m_c4FailCnt;
        THeapBuf            m_mbufCurData;
        mutable TMutex      m_mtxSync;
        TString             m_strCurSerialNum;
        TString             m_strOutPath;
        TString             m_strRepoMoniker;
};



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCClService
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCClService : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCClService();

        TFacCQCClService(const TFacCQCClService&) = delete;
        TFacCQCClService(TFacCQCClService&&) = delete;

        ~TFacCQCClService();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCClService& operator=(const TFacCQCClService&) = delete;
        TFacCQCClService& operator=(TFacCQCClService&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        )   final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bQueryEventKey
        (
                    TMD5Hash&               mhashToFill
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TBoolean bQueryRepoDB
        (
            const   TString&                strRepoMon
            , const TString&                strCurSerialNum
            ,       tCIDLib::TCard4&        c4BufSz
            ,       THeapBuf&               mbufData
        );

        tCIDLib::TBoolean bSrvRunning() const;

        tCIDLib::TCard4 c4QueryRepoList
        (
                    TVector<TString>&       colToFill
        );

        tCIDLib::EExitCodes eServiceThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid StartProcessing();

        tCIDLib::TVoid StartService();

        tCIDLib::TVoid StopProcessing();

        tCIDLib::TVoid StartShutdown();

        tCIDLib::TVoid WaitComplete();


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        using TRepoCache = TRefKeyedHashSet<TCacheThread, TString, TStringKeyOps>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoInit();

        tCIDLib::EExitCodes eMasterThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid ORBCleanup();

        tCIDLib::TVoid ParseParms();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxClients
        //      The max simultaneous clients to support. If its not provided
        //      on the command line, we allow the default Orb max to be used.
        //
        //  m_colRepoThreads
        //      A list of repos currently on our list. Each one is a thread that
        //      is responsible for downloading data from its repo and storing
        //      it locally. The master thread adds or removes items as
        //      required. This guy is thread safe because we have to sync
        //      with incoming client ORB requests.
        //
        //  m_evShutdown
        //      Because we have to keep sending "we're working on it" messages
        //      to the service handler, we cannot use the usual blocking
        //      shutdown request stuff built into TThread. So the service
        //      thread triggers this event to kick us into shutting down, and
        //      then it just waits for us to die by looping and tell the
        //      service manager to wait.
        //
        //  m_ippnListen
        //      The port to register our ORB listener thread on. It has a
        //      default but can be overridden on the command line.
        //
        //  m_plgrInstalled
        //      This is a pointer to log server logger that we installed. We
        //      don't own this, the CIDLib logging system does. But we keep
        //      a pointer so that we can put it into 'forced local' mode as
        //      we are shutting down.
        //
        //  m_porbsImpl
        //      A pointer to our ORB interface implementation object. We don't
        //      own this, the ORB does. We just keep a typed pointer around
        //      for convenience.
        //
        //  m_strORBBinding
        //      We build up the binding for our ORB server interface that
        //      we register in the name server.
        //
        //  m_thrMaster
        //      The master thread that watches for new repos to show up and
        //      deploys other threads to manage the various repos.
        //
        //  m_thrService
        //      Our service thread. It's either started up by the service
        //      handler or by the main startup macro in debug mode.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4MaxClients;
        TRepoCache          m_colRepoThreads;
        TEvent              m_evShutdown;
        tCIDLib::TIPPortNum m_ippnListen;
        TLogSrvLogger*      m_plgrInstalled;
        TCQCClSrvImpl*      m_porbsImpl;
        TString             m_strORBBinding;
        TThread             m_thrMaster;
        TThread             m_thrService;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCClService,TFacility)
};


