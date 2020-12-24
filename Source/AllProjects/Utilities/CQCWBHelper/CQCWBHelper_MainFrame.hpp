//
// FILE NAME: CQCWBHelper_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/16/2016
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
//  This is the header for the CQCWBHelper_MainFrame.cpp file, which provides the main
//  frame window for this program. This guy is set up to have no decorations at all. It
//  just contains the web browser window, which completely fills it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TQMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------
class TQMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TQMsg();

        TQMsg
        (
            const   tCQCWBHelper::EQMsgTypes    eType
        );

        TQMsg
        (
            const   tCQCWBHelper::EQMsgTypes    eType
            , const tCIDLib::TBoolean           bVal
        );

        TQMsg
        (
            const   tCQCWBHelper::EQMsgTypes    eType
            , const TString&                    strVal
        );

        TQMsg
        (
            const   tWebBrowser::ENavOps        eToDo
        );

        TQMsg
        (
            const   TArea&                      areaPar
            , const TArea&                      areaWidget
        );

        TQMsg
        (
            const   TArea&                      areaPar
            , const TArea&                      areaWidget
            , const tCIDLib::TBoolean           bVisibility
            , const TString&                    strInitURL
        );

        TQMsg(const TQMsg&);

        ~TQMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TQMsg& operator=(const TQMsg&);


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TArea                       m_areaVal;
        TArea                       m_areaVal2;
        tCIDLib::TBoolean           m_bVal;
        tWebBrowser::ENavOps        m_eWebOp;
        tCQCWBHelper::EQMsgTypes    m_eType;
        TString                     m_strVal1;
};



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

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create
        (
            const   tCIDCtrls::TWndHandle   hwndOwner
            , const tCIDLib::TBoolean       bSilentMode
            , const tCIDLib::TCard4         c4RefreshMins
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bEraseBgn
        (
                    TGraphDrawDev&          gdevToUse
        )   override;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaInvalid
        )   override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidSrc
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid SetNewArea
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4RefreshMins
        //      If non-zero we auto-refresh every this many minutes.
        //
        //  m_enctNextRefresh
        //      If auto-refresh is enabled, this is the next refresh time.
        //
        //  m_qmsgCur
        //      A queue message object that use to read in messages from the queue.
        //
        //  m_strLastURL
        //      The last URL we were given to display, so what we are supposed to be
        //      showing now. For error logging purposes.
        //
        //  m_tmidMonitor
        //      We start a timer to watch for timeouts, where we don't get any input
        //      from the invoking client within a period of time.
        //
        //  m_wndBrowser
        //      The web browser window that we create as a child of us, and which we
        //      keep sized to completely fill us.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4RefreshMins;
        tCIDLib::TEncodedTime   m_enctNextRefresh;
        TQMsg                   m_qmsgCur;
        TString                 m_strLastURL;
        tCIDCtrls::TTimerId     m_tmidMonitor;
        TWebBrowserWnd          m_wndBrowser;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK

