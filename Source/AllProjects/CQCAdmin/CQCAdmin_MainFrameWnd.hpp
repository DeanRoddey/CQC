//
// FILE NAME: CQCAdmin_MainFrameWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2000
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
//  This is the header for the CQCClient_MainFrameWnd.cpp file, which
//  implements the
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        TMainFrameWnd(const TMainFrameWnd&) = delete;
        TMainFrameWnd(TMainFrameWnd&&) = delete;

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        TMainFrameWnd& operator=(const TMainFrameWnd&) = delete;
        TMainFrameWnd& operator=(TMainFrameWnd&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const final;


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreateMain
        (
            const   TCQCCAppState&          fstInit
        );

        tCIDLib::TVoid RegisterBrowserEvs
        (
                    TCQCTreeBrowser&        wndBrowser
        );

        tCIDLib::TVoid SetupComplete();

    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bState
            , const tCIDCtrls::TWndHandle   hwndOther
        )   final;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bAllowShutdown() final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::TVoid DragEnd() final;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   final;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCloseTab
        (
                    TContentTab* const      pwndToClose
        );

        tCIDLib::TBoolean bSaveAllTabs();

        tCIDLib::TBoolean bSaveTab
        (
                    TContentTab&            wndToSave
        );

        tCIDLib::TVoid CheckTabChanges
        (
                    TContentTab&             wndTar
        );

        tCIDLib::TVoid ClearStatusBar();

        tCIDLib::TVoid CreatePackage
        (
            const   TTreeBrowseInfo&        wnotBrowser
        );

        tCIDLib::TVoid CreateTmplPackage
        (
            const   TString&                strRelPath
            ,       TCQCPackage&            packNew
            ,       TDataSrvClient&         dsclLoad
        );

        tCIDCtrls::EEvResponses eBrowseHandler
        (
                    TTreeBrowseInfo&        wnotBrowser
        );

        tCIDCtrls::EEvResponses eEditTabHandler
        (
                    TTabEventInfo&          wnotEdit
        );

        tCIDLib::TVoid EditEMailAcct
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditEventMon
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditGC100Ports();

        tCIDLib::TVoid EditGlobalAction
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditJAPwrPorts();

        tCIDLib::TVoid EditImage
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditLocation();

        tCIDLib::TVoid EditLogicSrv();

        tCIDLib::TVoid EditMacro
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditSchedEvent
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditSystemCfg
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditTemplate
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditTrigEvent
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid EditUserAccount
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid FlushLogs();

        tCIDLib::TVoid ImportPackage();

        tCIDLib::TVoid LoadFile
        (
            const   TTreeBrowseInfo&        wnotBrowser
            , const tCIDLib::TBoolean       bEditMode
        );

        tCIDLib::TVoid LoadClientDrv
        (
            const   TTreeBrowseInfo&        wnotBrowser
        );

        tCIDLib::TVoid MakeDisplayName
        (
            const   TString&                strFullPath
            ,       TString&                strToFill
        );

        TContentTab* pwndFindContTab
        (
            const   TString&                strPath
            , const tCIDLib::TBoolean       bEditMode
        );

        tCIDLib::TVoid RenameItem
        (
            const   TTreeBrowseInfo&        wnotBrowser
        );

        tCIDLib::TVoid StoreStateInfo();

        tCIDLib::TVoid UpdatePauseState
        (
            const   TTreeBrowseInfo&        wnotEvent
        );

        tCIDLib::TVoid ViewDriver
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ViewHelp
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid ViewImage
        (
            const   TString&                strPath
        );

        tCIDLib::TVoid ViewMacro
        (
            const  TString&                 strPath
        );

        tCIDLib::TVoid ViewSystemInfo
        (
            const  TString&                 strPath
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSizePosChanged
        //      This is set any time we get an area changed callback. Our state flushing
        //      timer uses this to know if it needs to store new info, else it doesn't
        //      do anything (which is much more efficient.)
        //
        //  m_colSkipWnds
        //      When we query the available client area, we have to skip our own client
        //      window, so we pre-set up the skip list that we pass into the query.
        //
        //  m_fstInit
        //      See the delayed init timer comments below. We have to save this on create
        //      to use during the delayed init.
        //
        //  m_pwndBrowserTab
        //      We need to keep a pointer to this around for doing certain things. Mostly
        //      it talks to us, but sometimes we need to talk to it.
        //
        //  m_pwndClient
        //      The main pane container window. We set it as a client of ourself and keep
        //      keep it sized to our client area.
        //
        //  m_pwndEditPane
        //  m_pwndProjPane
        //      Our two tiled panes. The project pane is on the left and holds tabs for
        //      browsing resources, and file type specific tabs that are added and removed
        //      on the fly.
        //
        //      The edit pane is a tabbed window that holds a tab for each opened
        //      file. We register ourseif for notifications from it so that we can
        //      handle tab closing and saving changes.
        //
        //  m_tmidCheckChanges
        //      This timer is used to periodically check the active tab for changes. If so
        //      we set the tab to changed state so that that will be indicated in the tab
        //      header. Other tabs are free to update themselves if that is something that
        //      they can reasonably do without even more overhead than how we do it, which
        //      is to just ask the tab if it has changes (which typically means they compare
        //      all their data to the last saved version.) This is safe but can be high
        //      overhead. So we don't go too fast.
        //
        //  m_tmidDelayInit
        //      We have some issues getting the status bar initially displayed and
        //      counted among the children of the frame (so that it gets removed when
        //      we get the available client area.) And we need to do some post-frame
        //      display checks as well. So we do a one shot timer to let messages b
        //       pumped. Then we play some tricks then show ourself.
        //
        //  m_tmidStateFlush
        //      We start this one up to periodically flush out our current application
        //      state to persistent storage.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bSizePosChanged;
        tCIDCtrls::TWndList m_colSkipWnds;
        TCQCCAppState       m_fstInit;
        TBrowserTab*        m_pwndBrowserTab;
        TPaneWndCont*       m_pwndClient;
        TTabbedWnd*         m_pwndEditPane;
        TTabbedWnd*         m_pwndProjPane;
        tCIDCtrls::TTimerId m_tmidCheckChanges;
        tCIDCtrls::TTimerId m_tmidDelayInit;
        tCIDCtrls::TTimerId m_tmidStateFlush;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};


