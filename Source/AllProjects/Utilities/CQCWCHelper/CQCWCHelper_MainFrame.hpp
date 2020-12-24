//
// FILE NAME: CQCWCHelper_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This is the header for the CQCWCHelper_MainFrame.cpp file, which provides the main
//  frame window for this program. This guy is set up to have no decorations at all. It
//  just contains a client window that completely covers it and at which we point the
//  video output.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TGUIMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------
class TGUIMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TGUIMsg();

        TGUIMsg
        (
            const   tCQCWCHelper::EGUIMsgTypes  eType
        );

        TGUIMsg
        (
            const   tCQCWCHelper::EGUIMsgTypes  eType
            , const tCIDLib::TBoolean           bVal
        );

        TGUIMsg
        (
            const   tCQCWCHelper::EGUIMsgTypes  eType
            , const TString&                    strVal
        );

        TGUIMsg
        (
            const   TArea&                      areaPar
            , const TArea&                      areaWidget
        );

        TGUIMsg
        (
            const   TArea&                      areaPar
            , const TArea&                      areaWidget
            , const tCIDLib::TBoolean           bVisibility
        );

        TGUIMsg
        (
            const   TGUIMsg&                    qmsgSrc
        );

        ~TGUIMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGUIMsg& operator=
        (
            const   TGUIMsg&                    qmsgSrc
        );


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TArea                       m_areaVal;
        TArea                       m_areaVal2;
        tCIDLib::TBoolean           m_bVal;
        tCQCWCHelper::EGUIMsgTypes  m_eType;
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
            , const tCIDLib::TBoolean       bDummyTest
        );

        TGenericWnd& wndVideo()
        {
            return m_wndVideo;
        }


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
        //  m_bDummyTest
        //      Indicates if we were invoked in dummy test mode. If so, we won't time
        //      out since there is no controlling application.
        //
        //  m_qmsgCur
        //      A queue message object that use to read in messages from the queue.
        //
        //  m_tmidMonitor
        //      We start a timer to watch for timeouts, where we don't get any input
        //      from the invoking client within a period of time.
        //
        //  m_wndVideo
        //      This is just a generic window that we keep sized to cover us, and that
        //      we point the VLC output to.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDummyTest;
        TGUIMsg                 m_qmsgCur;
        tCIDCtrls::TTimerId     m_tmidMonitor;
        TGenericWnd             m_wndVideo;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK

