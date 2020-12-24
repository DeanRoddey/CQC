//
// FILE NAME: CQCTrayMon_AppCtrlTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
//  This is the header for the CQCTrayMon_AppCtrlTab.cpp file, which implements
//  the TCQCTrayAppCtrlTab class. This class provides the interface for the
//  app control related functioality.
//
//  It also will queue up events from the ORB server threads receiving
//  requests from remote clients (those that have to be processed within the
//  GUI thread) and processes them within a timer.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TACActivatorWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TACActivatorWnd : public TWindow
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TACActivatorWnd();

        TACActivatorWnd(const TACActivatorWnd&) = delete;

        ~TACActivatorWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TACActivatorWnd& operator=(const TACActivatorWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create();


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TACActivatorWnd, TWindow)
};



// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayAppCtrlTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCTrayAppCtrlTab : public TCQCTrayMonTab
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEventQ = TRefQueue<TAppUtilEv>;
        using TStatusQ = TQueue<TAppListChangeEv>;


        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCTrayAppCtrlTab();

        TCQCTrayAppCtrlTab(const TCQCTrayAppCtrlTab&) = delete;

        ~TCQCTrayAppCtrlTab();


        // -------------------------------------------------------------------
        //  Public opeators
        // -------------------------------------------------------------------
        TCQCTrayAppCtrlTab& operator=(const TCQCTrayAppCtrlTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Cleanup() override;

        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
        )   override;

        tCIDLib::TVoid RemoveBindings
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddACEvent
        (
                    TAppUtilEv* const       paueToAdopt
        );

        tCIDLib::TVoid AddStatusEvent
        (
            const   TString&                strMoniker
            , const TString&                strTitle
            , const tCQCTrayMon::EListEvs   eEvent
        );

        tCIDLib::TVoid AddStatusEvent
        (
            const   TString&                strMoniker
            , const tCQCTrayMon::EListEvs   eEvent
        );

        tCIDLib::TVoid AddStatusEvent
        (
            const   TString&                strStatusMsg
        );

        tCIDLib::TVoid ForceToFront();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid UpdateSelItemInfo();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLastStatus
        //      The last running/stopped status that we saw for the currently
        //      selected app in the list. This let's us know during the timer
        //      event if we need to update it or not.
        //
        //  m_colEventQ
        //      The AddEvent method adds an event object to this queue,
        //      which our timer thread processes, pulling them out in order
        //      and processing them.
        //
        //  m_colStatusQ
        //      The AddChangeEvent methods add status events to this queue,
        //      which our timer thread processes in order to keep our display
        //      up to date. It is made a thread safe collection since the
        //      calls to add them come from incoming client events on other
        //      ORB service threads.
        //
        //  m_errACLoopExcept
        //      An error we log if we get an exception in our app control event
        //      handler loop.
        //
        //  m_loglimLoop
        //      Our event processing loop could get a lot of errors if something
        //      goes wrong. So we use a log limiter to avoid logging redundant
        //      errors quickly. We take the default interval.
        //
        //  m_porbcAppCtrl
        //      Our server side app control object. We keep it here since
        //      this class is the one that interacts with it. The facility
        //      class
        //
        //  m_pwndXXX
        //      Pointers to our child widgets. We load them from a dialog
        //      resource, and just want to get typed pointers to them to
        //      make it easier to interact with them.
        //
        //  m_strEvTmp
        //      A string to use for loading messages during the timer method
        //      where we pull event objects off the queue and report them to
        //      the user.
        //
        //  m_tmidUpdate
        //      To store the timer id for the timer we start to update our
        //      display.
        //
        //  m_wndActivator
        //      In order to activate programs, we have to push ourself forward
        //      to become the active window. We don't want to do that with
        //      the actual frame window that we are embedded in. So we use
        //      this simple, top level, invisible window for that. Sine it is
        //      top level, and not owned, it won't make us come forward.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bLastStatus;
        TEventQ             m_colEventQ;
        TStatusQ            m_colStatusQ;
        TError              m_errACLoopExcept;
        TLogLimiter         m_loglimLoop;
        TAppCtrlServerImpl* m_porbsAppCtrl;
        TStaticText*        m_pwndAppName;
        TStaticMultiText*   m_pwndAppPath;
        TStaticText*        m_pwndDir;
        TTextOutWnd*        m_pwndEvLog;
        TStaticText*        m_pwndMoniker;
        TStaticMultiText*   m_pwndParams;
        TStaticText*        m_pwndStatus;
        TMultiColListBox*   m_pwndList;
        TString             m_strEvTmp;
        tCIDCtrls::TTimerId m_tmidUpdate;
        TACActivatorWnd     m_wndActivator;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrayAppCtrlTab, TCQCTrayMonTab)
};


