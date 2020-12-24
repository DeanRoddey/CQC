//
// FILE NAME: CQCTrayMon_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/14/2004
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
//  This is the header for the facility class for the try monitor program.
//
// CAVEATS/GOTCHAS:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCTrayMon
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCTrayMon : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCTrayMon();

        ~TFacCQCTrayMon();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::EVerboseLvls eVerboseLvl() const;

        tCQCKit::EVerboseLvls eVerboseLvl
        (
            const   tCQCKit::EVerboseLvls   eToSet
        );

        const TString& strBinding() const;

        tCIDLib::TVoid ShowNotification
        (
            const   TString&                strTitle
            , const TString&                strText
            , const tCIDLib::TBoolean       bError
        );

        tCIDLib::TVoid SleepWake
        (
            const   tCIDLib::TBoolean       bWakingUp
        );

        tCIDLib::TVoid Shutdown
        (
            const   tCIDLib::EExitCodes     eExit
        );

        tCIDLib::TVoid ToggleFrameVis();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckAService
        (
            const   tCIDLib::TCh* const     pszName
            , const tCQCTrayMon::ESrvStates ePrev
            ,       tCQCTrayMon::ESrvStates& eToSet
        );

        tCIDLib::TVoid Cleanup();

        tCIDLib::TVoid CreateServerObjects();

        tCIDLib::EExitCodes eMonThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid ParseParams
        (
                    tCIDLib::TCard4&        c4StartDelay
            ,       tCIDLib::TCard4&        bAppCtrl
            ,       tCIDLib::TCard4&        biTunes
        );

        tCIDLib::TVoid StoreSrvState
        (
            const   tCIDLib::TBoolean       bShow
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxClients
        //      The maximum number of simultaneous clients we will accept.
        //      This can be provided via the /Max= parameter, else it will
        //      be defaulted by the ORB.
        //
        //  m_eLastAppSrvState
        //  m_eLastClSrvState
        //      The monitor thread updates m_eLastState with each new service
        //      state it sees. If it gets a new state, it posts something to
        //      the tray window so it can show a balloon popup. We need one for
        //      each of our two possible services.
        //
        //  m_eVerboseLvl
        //      The logging verbosity level. This can be set via a menu
        //      option.
        //
        //  m_ippnListen
        //      The IP port on which we tell the Orb server to listen. If its
        //      not given on the command line, we use the default from the
        //      kCQCKit namespace.
        //
        //  m_plgrLogSrv
        //      Our ORB based logger client. We keep it around so that we
        //      can clean it up during shutdown.
        //
        //  m_porbsXXX
        //      Some application level server side objects we register with
        //      the ORB. We keep pointers around so that we can deregister
        //      them during shutdown.
        //
        //  m_strBinding
        //      The binding name we use for any name server bindings. The
        //      installer just defaults it to the local host name, but the
        //      user can change it.
        //
        //  m_thrMonitor
        //      We spin off a thread to do basic system status monitoring,
        //      basically watching the service to see whether it is running
        //      or not (we show a balloon popup when we see it start or stop.)
        //      And we ping some of the servers periodically as well and
        //      report if that fails (but the service appears to be running.)
        //
        //      If there are no bgn servers installed, then we don't even
        //      bother to start this thread up.
        //
        //  m_wndMain
        //      The main frame window. It is only visible when the user clicks
        //      on the tray icon. Otherwise it's hidden.
        //
        //  m_wndTray
        //      The tray window. This one is never visible. It's just there
        //      to provide the interface to the system tray. The m_wndMain
        //      is the main frame window.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4MaxClients;
        tCQCTrayMon::ESrvStates m_eLastAppSrvState;
        tCQCTrayMon::ESrvStates m_eLastClSrvState;
        tCQCKit::EVerboseLvls   m_eVerboseLvl;
        tCIDLib::TIPPortNum     m_ippnListen;
        TLogSrvLogger*          m_plgrLogSrv;
        TCIDCoreAdminImpl*      m_porbsCoreAdmin;
        TRemDiskBrowserImpl*    m_porbsBrowser;
        TString                 m_strBinding;
        TThread                 m_thrMonitor;
        TCQCTrayMonFrameWnd     m_wndMain;
        TTrayMonWnd             m_wndTray;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCTrayMon,TGUIFacility)
};

#pragma CIDLIB_POPPACK


