//
// FILE NAME: CQCIntfWEng_Window.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/06/2009
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
//  This is the header for the CQCIntfWEng_Window.cpp file, which implements
//  the actual display window. The view passes on requests to this guy and
//  this guy passes on user input, paint, and timer events to the view.
//
//  We derive from the TTouchWnd class, which provides us with generic touch
//  input, whether the local machine has multi-touch or just regular mouse
//  support.
//
//  We also provide a no-input mode, where we will ignore all touch screen
//  input, which can be used in digital signage scenarios. This is set on
//  the facility object and we look at whatever is set there.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfViewWnd
//  PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCINTFWENGEXPORT TCQCIntfViewWnd : public TTouchWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfViewWnd();

        TCQCIntfViewWnd(const TCQCIntfViewWnd&) = delete;

        ~TCQCIntfViewWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfViewWnd& operator=(const TCQCIntfViewWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFinger
        )   override;

        tCIDLib::TVoid GestClick
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TVoid GestFlick
        (
            const   tCIDLib::EDirs          eDirection
            , const TPoint&                 pntStartPos
        )   override;

        tCIDLib::TVoid GestInertiaIdle
        (
            const   tCIDLib::TEncodedTime   enctEnd
            , const tCIDLib::TCard4         c4Millis
            , const tCIDLib::EDirs          eDir
        )   override;

        tCIDLib::TVoid GestReset
        (
            const   tCIDCtrls::EGestReset   eType
        )   override;

        tCIDLib::TVoid PerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eToSet
            ,       tCIDLib::TFloat4&       f4VScale
        )   override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInitialize
        (
            const   TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TBoolean bInitialize
        (
                    TWindow&                wndParent
            ,       TDataSrvClient&         dsclToUse
            ,       tCQCIntfEng::TErrList&  colErrs
            , const TString&                strTitle
            ,       MCQCIntfAppHandler* const pmiahToUse
            , const TCQCIntfTemplate&       iwdgTemplate
            , const TCQCUserCtx&            cuctxToUse
        );

        tCIDLib::TBoolean bReload
        (
                    TWindow&                wndParent
            ,       TDataSrvClient&         dsclToUse
            ,       tCQCIntfEng::TErrList&  colErrs
            , const TCQCIntfTemplate&       iwdgTemplate
        );

        const TCQCIntfWView& civTarget() const;

        TCQCIntfWView& civTarget();

        tCIDLib::TVoid Create
        (
                    TWindow&                wndParent
            , const tCIDCtrls::TWndId       widToUse
            , const TArea&                  areaInit
        );

        tCIDLib::TVoid DisplayResChanged
        (
            const   TSize&                  szNew
        );

        tCIDLib::TVoid NewSize
        (
            const   TSize&                  szNew
        );

        tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid ProcessActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        );

        const TString& strTmplId() const;

        tCIDLib::TVoid ScrPosChanged();

        tCIDLib::TVoid UpdateLastActivityStamp();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
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
                    TGraphDrawDev&          gdevTarget
        )   override;

        tCIDLib::TBoolean bExtKey
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TBoolean bPaint
        (
                    TGraphDrawDev&          gdevToUse
            , const TArea&                  areaUpdate
        )   override;

        tCIDLib::TVoid DataReceived
        (
            const   tCIDLib::TCard4         c4DataId
            ,       tCIDLib::TVoid* const   pData
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid GettingFocus() override;

        tCIDLib::TVoid LosingFocus() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid StartTimers();

        tCIDLib::TVoid StopTimers();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bmpBusyHalo
        //      An alpha based image that's a red halo. If the user presses
        //      down and there's an active action, we blend this around the
        //      press point to indicate it's being ignored, then redraw the
        //      area to remove it.
        //
        //  m_c4PauseTimers
        //      We have to support the view's timer pause/start stuff. We use
        //      a cardinal value since we have to support nesting. So if zero
        //      timers are allowed, else suppressed.
        //
        //  m_pcivTarget
        //      We create a view object that provides the interaction with
        //      the displayed template. This is our own view derivatives, though
        //      the bulk of the work is provided by its parent classes.
        //
        //      It's a pointer because we cannot initialize it until we are
        //      created.
        //
        //  m_szLast
        //      To support minimal redraw.
        //
        //  m_strTmplId
        //      For indentification purposes in apps that support more than
        //      one template window at once, we generate a unique id for
        //      ourself and provide a method to access it. It's a formatted
        //      MD5 hash.
        //
        //  m_tmidActiveUp
        //  m_tmidEvents
        //  m_tmidValueUp
        //      Ids for the timers we start to drive the activity we are responsible
        //      for. We register them upon creation.
        // -------------------------------------------------------------------
        TBitmap             m_bmpBusyHalo;
        tCIDLib::TCard4     m_c4PauseTimers;
        TCQCIntfWView*      m_pcivTarget;
        TSize               m_szLast;
        TString             m_strTmplId;
        tCIDCtrls::TTimerId m_tmidActiveUp;
        tCIDCtrls::TTimerId m_tmidEvents;
        tCIDCtrls::TTimerId m_tmidValueUp;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfViewWnd,TTouchWnd)
};

#pragma CIDLIB_POPPACK


