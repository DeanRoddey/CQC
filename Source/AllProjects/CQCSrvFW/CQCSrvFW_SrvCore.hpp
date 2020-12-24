//
// FILE NAME: CQCSrvFW_SrvCore.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2019
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
//  This is the header for the CQCSrvFW_SrvCore.cpp file, which implements a
//  framework that CQC server applications can derive from and just provide a
//  fairly small amount of extension to in order to create a standard CQC server
//  that does all the things it needs to do.
//
//  All the servers are centered around their facility class, so we create a
//  derivative of TFacility, and then they derive from us.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

#pragma CIDLIB_PACK(CIDLIBPACK)

class TCIDCoreAdminImpl;

// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvCore
//  PREFIX: fac
// ---------------------------------------------------------------------------
class CQCSRVFWEXPORT TCQCSrvCore : public TFacility, public MSignalHandler
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCSrvCore() = delete;

        TCQCSrvCore(const TCQCSrvCore&) = delete;
        TCQCSrvCore(TCQCSrvCore&&) = delete;

        ~TCQCSrvCore();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCSrvCore& operator=(const TCQCSrvCore&) = delete;
        TCQCSrvCore& operator=(TCQCSrvCore&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bHandleSignal
        (
            const   tCIDLib::ESignals       eSignal
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TCQCUserCtx& cuctxToUse() const
        {
            return m_cuctxToUse;
        }

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


    protected :
        // -------------------------------------------------------------------
        //  The ORB admin interface is a friend so he can call us without having
        //  to expose stuff only intended for him.
        // -------------------------------------------------------------------
        friend class TCIDCoreAdminImpl;


        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TCQCSrvCore
        (
            const   TString&                strFacName
            , const TString&                strDescr
            , const tCIDLib::TIPPortNum     ippnDefSrvPort
            , const TString&                strEvName
            , const tCIDLib::EModFlags      eModFlags
            , const tCQCSrvFW::ESrvOpts     eOptions
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid DeregisterSrvObjs();

        virtual tCQCSrvFW::EStateRes eLoadConfig
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        );

        virtual tCQCSrvFW::EStateRes eProcessParms
        (
                    tCIDLib::TKVPList::TCursor& cursParms
        );

        virtual tCQCSrvFW::EStateRes eWaitPrereqs
        (
                    tCIDLib::TCard4&        c4WaitNext
        );

        virtual tCIDLib::TVoid MakeDirs();

        virtual tCIDLib::TVoid PostDeregTerm();

        virtual tCIDLib::TVoid PostUnbindTerm();

        virtual tCIDLib::TVoid PreBindInit();

        virtual tCIDLib::TVoid PreRegInit();

        virtual tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        ) = 0;

        virtual tCIDLib::TVoid RegisterSrvObjs();

        virtual tCIDLib::TVoid StartWorkerThreads();

        virtual tCIDLib::TVoid StopWorkerThreads();

        virtual tCIDLib::TVoid StoreConfig();

        virtual tCIDLib::TVoid UnbindSrvObjs
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        );

        virtual tCIDLib::TVoid WaitForTerm
        (
                    TEvent&                 evWait
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bWait
        (
            const   tCIDLib::TCard4         c4Millis
        )   const;

        const TMD5Hash& mhashSrvPW() const
        {
            return m_mhashSrvPW;
        }

        tCIDLib::TVoid Shutdown
        (
            const   tCIDLib::EExitCodes     eReturn
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoCleanup
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid DoInit
        (
                    TThread&                thrThis
        );

        tCQCSrvFW::EStateRes eEnableEvents();

        tCQCSrvFW::EStateRes eLogin
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        );

        tCQCSrvFW::EStateRes eIntProcessParms();

        tCIDLib::TVoid IntDeregObjects();

        tCIDLib::TVoid IntRegObjects();

        tCIDLib::TVoid IntUnbindObjects();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxClients
        //      The max simultaneous clients to support. If its not provided on the
        //      command line, we allow the default Orb max to be used.
        //
        //  m_c4SigHandlerId
        //      When we installer ourselves a signal handler, we store the returned id
        //      and pass it back in when we remove ourself.
        //
        //  m_colBindings
        //      The derived class does server side object bindings by way of us, so
        //      that we can keep up with them and clean them up on behalf of the
        //      derived class.
        //
        //  m_cuctxToUse
        //      There are a few things that require administrative rights. Some of the
        //      actions supported by some of the standard command targets require a
        //      security context be passed. This is provided in the environment or the
        //      system registry.
        //
        //  m_eOptions
        //      We can do some stuff for the derived class by just knowing he wants
        //      it, so this is a bitmapped enum with options he can pass to our ctor.
        //
        //  m_eReturn
        //      The return value that the derived class wants us to
        //
        //  m_eState
        //      The current state of the server. We are trying to move this forward
        //      to Ready state. On shutdown (or bailout during startup) we move this
        //      backwards to clean up any states we got done.
        //
        //  m_ippnListen
        //      The port that the ORB should listen on.
        //
        //  m_pevWait
        //      The main thread just waits on this event once it has done the init.
        //      It is named so that, worst case, the app shell can use this to make
        //      it exit if it cannot get to the admin interface.
        //
        //  m_plgrLogSrv
        //      We install a log server logger. We don't own it after we've installed
        //      it, but we need to keep the pointer around so that we can force it
        //      back to local logging before we toast the client ORB support.
        //
        //  m_porbsCoreAdmin
        //      We create an register a core admin object on behalf of the derived
        //      class. We don't own it but keep pointer around so we can deregister
        //      on shutdown.
        //
        //  m_pthrSrvFW
        //      We store a pointer to our main entry point there, so that it will
        //      be available to calls to AdminStop(), which can be called from ORB
        //      threads or derived class worker threads. It will do an aysnc shutdown
        //      request on this thread.
        //
        //  m_strAdminBinding
        //      We register a core admin object on behalf of the derived class, and
        //      he gives us a binding name to use. It may have a %(h) in it which will
        //      be replaced with the host name. We also get the descriptive text, but
        //      this one we have to keep around for later unbinding.
        //
        //  m_strDescr
        //      A descriptive text string (short) that we log on startup and shutdown
        //      to help track the coming and going of servers.
        //
        //  m_strEvName
        //      The derived class gives us the name to give to the shutdown event.
        //
        //  m_strName
        //      The facility name. We pass this to the base facility class and use
        //      it in any named shared resources, and in some logged errors and
        //      such.
        //
        //  m_mhashSrvPW
        //      If the derived server asks for a login, then we use the (command line
        //      or environment provided) account to login and set m_cutxToUse. We also
        //      store hash of the password here since that is needed by the derived
        //      servers sometimes and it's not something stored in the user context.
        //
        //  m_tmStart
        //      This is set in the ctor, and provides the start up time to
        //      the admin queries.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4MaxClients;
        tCIDLib::TCard4         m_c4SigHandlerId;
        TCQCUserCtx             m_cuctxToUse;
        tCIDLib::EExitCodes     m_eReturn;
        tCQCSrvFW::ESrvOpts     m_eOptions;
        tCQCSrvFW::ESrvStates   m_eState;
        tCIDLib::TIPPortNum     m_ippnListen;
        TMD5Hash                m_mhashSrvPW;
        TEvent*                 m_pevWait;
        TLogSrvLogger*          m_plgrLogSrv;
        TCIDCoreAdminImpl*      m_porbsCoreAdmin;
        TThread*                m_pthrSrvFW;
        TString                 m_strAdminBinding;
        TString                 m_strDescr;
        TString                 m_strEvName;
        TString                 m_strFacName;
        TTime                   m_tmStart;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCSrvCore,TFacility)
};


#pragma CIDLIB_POPPACK
