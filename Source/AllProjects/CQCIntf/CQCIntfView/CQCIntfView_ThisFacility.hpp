//
// FILE NAME: CQCIntfView_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/15/2002
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
//  This is the header for the facility class for the standalone interface
//  viewer.
//
// CAVEATS/GOTCHAS:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCIntfView
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCIntfView : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCIntfView();

        ~TFacCQCIntfView();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bFullScreen() const;

        tCIDLib::TBoolean bKioskMode() const;

        tCIDLib::TBoolean bQueueTTS
        (
            const   TString&                strToSpeak
        );

        tCIDLib::TBoolean bNoState() const;

        tCIDLib::TBoolean bSignageMode() const;

        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCKit::EUserRoles eUserRole() const;

        tCIDLib::TVoid FlushTTSQ();

        tCIDLib::TIPPortNum ippnListen() const;

        const TCQCSecToken& sectUser() const;

        const TString& strCfgBinding() const;

        const TString& strUserName() const;

        tCIDLib::TVoid StoreStatus
        (
            const   TString&                strBaseTmpl
            , const TString&                strTopTmpl
            , const tCIDLib::TCard4         c4LayerCnt
        );

        const TMainFrameWnd& wndMain() const;

        TMainFrameWnd& wndMain();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoLogon();

        tCIDLib::TBoolean bParseParams
        (
                    TString&                strInitTmpl
            ,       tCIDLib::TCard4&        c4StartDelay
            ,       tCIDLib::TBoolean&      bNoState
            ,       tCIDLib::TBoolean&      bFSMode
        );

        tCIDLib::TVoid CheckSignage();

        tCIDLib::EExitCodes eCleanUp();

        tCIDLib::EExitCodes eSpeechThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bFullScreen
        //      Set via the /FullScreen parameter, it forces us to go straight
        //      to full screen mode.Also implies the /NoState flag.
        //
        //  m_bKioskMode
        //      Set via the /Kiosk parameter, this puts us into Kiosk mode,
        //      where we go full screen and won't exit it without an admin
        //      logon.
        //
        //  m_bSignageMode
        //      If the /Signage=mm parameter is provided, this is set and it puts
        //      us into digital signage mode. We force kiosk and full screen
        //      on and don't do a single isntance check. And we take a monitor
        //      number to go full screen on.
        //
        //  m_colSpeech
        //      Remote requests to do TTS in the background are queued up
        //      here for the m_thrSpeech thread to process. Sync is provided
        //      through this collection itself.
        //
        //  m_cuctxToUse
        //      The user context that we set up when the user logs in. We
        //      have to pass this to various parties along the way.
        //
        //  m_ippnListen
        //      Set via the /Port= parameter, else it is defaulted. This is
        //      the port that we set up the server side ORB on to access
        //      connections from our external control driver.
        //
        //  m_mtxSync
        //      We need to be able to sync with the sync thread and the
        //      logon process, which will store the m_strId field when the
        //      user logs on, and with calls from the engine to get the
        //      id.
        //
        //  m_plgrLogSrv
        //      Our logger that we install.
        //
        //  m_polleToUse
        //      We have to provide a polling engine to various subsystems.
        //
        //  m_porbsCtrlImpl
        //      A pointer to the object we register for external control.
        //
        //  m_strAppAttachKey
        //      We store and reload attached application information so that
        //      we can re-attach to them on startup. We build up the key
        //      on init so we don't have to do it more than once.
        //
        //  m_strCtrlBinding
        //      The name we used in the name server binding if our control
        //      port was enabled. Set either by default to the local host
        //      name or to the value provided on the command line via the
        //      /Binding= parameter.
        //
        //  m_strDisplayName
        //      If in signage mode, we get a display name, and store it away
        //      here. We use this later to get the area of the target display
        //      so that we can go full screen there.
        //
        //  m_thrSpeech
        //      We spin up a background thread that processes any text to
        //      speech we are asked remotely to do. This guy just blocks
        //      on the m_colSpeech queue waiting for text to speak. We start
        //      it up on eSpeechThread.
        //
        //  m_uaccLogin
        //      The user account info for the user who logged in. There is
        //      nothing unsafe in here. Those fields are not returned from
        //      the security server. We want it for things like logon time
        //      limits and the default template.
        //
        //  m_wndMain
        //      The main frame window of the client.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bFullScreen;
        tCIDLib::TBoolean       m_bKioskMode;
        tCIDLib::TBoolean       m_bSignageMode;
        tCIDLib::TStringQ       m_colSpeech;
        TCQCUserCtx             m_cuctxToUse;
        tCIDLib::TIPPortNum     m_ippnListen;
        TMutex                  m_mtxSync;
        TLogSrvLogger*          m_plgrLogSrv;
        TCQCPollEngine          m_polleToUse;
        TIntfCtrlServer*        m_porbsCtrlImpl;
        TString                 m_strAppAttachKey;
        TString                 m_strCtrlBinding;
        TString                 m_strDisplayName;
        TThread                 m_thrSpeech;
        TCQCUserAccount         m_uaccLogin;
        TMainFrameWnd           m_wndMain;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCIntfView,TGUIFacility)
};

#pragma CIDLIB_POPPACK

