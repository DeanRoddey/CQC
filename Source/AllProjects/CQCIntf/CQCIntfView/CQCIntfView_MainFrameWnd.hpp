//
// FILE NAME: CQCIntfView_MainFrameWnd.hpp
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
//  This is the header for the CQCIntfView_MainFrameWnd.cpp file, which implements the
//  main frame of the standalone interface viewer.
//
// CAVEATS/GOTCHAS:
//
//  1)  We don't use an accelerator table because we want to support user defined hot
//      keys to drive actions. An accelerator would be a big pain to deal with given
//      that it would want to eat keys that the user may want to use for something else.
//      So we handle extended and media keys ourself. If there's a user defined action,
//      we do that. Else, we let it pass on to the default handler which will handle any
//      of the common hot keys.
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TMainFrameWnd : public TFrameWnd, public MCQCIntfAppHandler
{
    public  :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TEventQ = TRefQueue<TIntfCtrlEv>;


        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TMainFrameWnd();

        ~TMainFrameWnd();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AsyncCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
            ,       TCQCIntfView&           civSender
        )   override;

        tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbInfo
        )   override;

        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const override;

        tCIDLib::TVoid DismissBlanker
        (
                    TCQCIntfView&           civSender
        )   override;

        tCIDLib::TVoid DoSpecialAction
        (
            const   tCQCIntfEng::ESpecActs  eAct
            , const tCIDLib::TBoolean       bAsync
            ,       TCQCIntfView&           civSender
        )   override;

        tCQCIntfEng::EAppFlags eAppFlags() const override;

        tCIDLib::TVoid NewTemplate
        (
            const   TCQCIntfTemplate&       iwdgNew
            ,       TCQCIntfView&           civSender
            , const tCIDLib::TCard4         c4StackPos
        )   override;

        MCQCCmdTracer* pmcmdtTrace() override;


        // -------------------------------------------------------------------
        // Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddCtrlEvent
        (
                    TIntfCtrlEv* const      piceToAdopt
        );

        tCIDLib::TBoolean bLoadTemplate
        (
            const   TString&                strDefault
            , const tCIDLib::TBoolean       bSupressWarn
        );

        tCIDLib::TCard4 c4GetStatus
        (
                    TString&                strBaseTmpl
            ,       TString&                strTopTmpl
        );

        tCIDLib::TVoid Create
        (
            const   TString&                strText
            , const TCQCIntfViewState&      fstInit
        );

        tCIDLib::TVoid DoRemLoadOp
        (
            const   tCIDLib::TBoolean       bOverlay
            , const TString&                strOvrName
            , const TString&                strToLoad
        );

        tCIDLib::TVoid DoRemMiscOp
        (
            const   tCQCKit::EIVMiscOps     eOp
        );

        tCIDLib::TVoid EndFSMode();

        tCIDLib::TVoid InvokeBlanker
        (
            const   tCIDLib::TBoolean       bPowerOff
            , const tCQCIGKit::EBlankModes  eMode
        );

        TCQCIntfViewWnd* pwndClient() const
        {
            return  m_pwndClient;
        }

        tCIDLib::TVoid SetVariable
        (
            const   TString&                strVarName
            , const TString&                strValue
        );

        tCIDLib::TVoid StartFSMode();



    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bNewState
            , const tCIDCtrls::TWndHandle   hwndOther
        )   override;

        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bClick
        (
            const   tCIDCtrls::EMouseButts  eButton
            , const tCIDCtrls::EMouseClicks eClickType
            , const TPoint&                 pntAt
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TBoolean bExtKey
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TBoolean bMediaKey
        (
            const   tCIDCtrls::EExtKeys     eExtKey
            , const tCIDLib::TBoolean       bAltShift
            , const tCIDLib::TBoolean       bCtrlShift
            , const tCIDLib::TBoolean       bShift
            , const TWindow* const          pwndChild = nullptr
        )   override;

        tCIDLib::TBoolean bMinimizing() override;

        tCIDLib::TVoid CodeReceived
        (
            const   tCIDLib::TInt4          i4Code
            , const tCIDLib::TCard4         c4Data
        )   override;

        tCIDLib::TVoid DataReceived
        (
            const   tCIDLib::TCard4         c4DataId
            ,       tCIDLib::TVoid* const   pRawData
        )   override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid DisplayChanged
        (
            const   tCIDLib::TCard4         c4NewBitsPerPel
            , const TSize&                  szNew
        )   override;

        tCIDLib::TVoid DragEnd() override;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CloseBlanker();


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCheckShutdown();

        tCIDLib::TVoid DoScreenCapture
        (
            const   tCIDLib::TBoolean       bDoPNG
        );

        tCIDLib::TVoid LoadMenu();

        tCIDLib::TVoid ProcessCmds();

        tCIDLib::TVoid ResizeView();

        tCIDLib::TVoid SelectTemplate();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bCachingEnabled
        //      We remember the last cache state selected by the user, which is used when
        //      loading templates.
        //
        //  m_bInitShow
        //      To avoid various issues, we don't display our window immediately before we
        //      enter the message loop as usual. Instead we use one of our timers and this
        //      flag (as a one-shot flag) to display the GUI after we've done some message
        //      pumping, which should allow most of the msgs queued up during our startup
        //      activity to get dealt with before we become visible.
        //
        //  m_bStateChanged
        //      When any of our persistent data changes, this gets set and our timer will
        //      pick it up and flush it back out to the object store.
        //
        //  m_c4StateId
        //      The id for our state in the object store, so that we can quickly update it.
        //
        //  m_colEventQ
        //      An event queue into which our control protocol server object drops events
        //      for us to handle in our timer. He cannot handle them because the involve GUI
        //      things and activation and so on.
        //
        //  m_fstCur
        //      Our current state data, which is what we read in from the object store when
        //      we start and flush out again when it changes.
        //
        //  m_piwdgCmdParent
        //  m_pwndOwner
        //      When we get the ProcessCmds() call from the interface engine, we store the
        //      passed parent container and window so that we will have access to them when
        //      we get the async command that we post to ourself.
        //
        //  m_pwndClient
        //      Our client window, which is a standard interface viewer window from
        //      CQCIntfWEng. We don't need anything special other than that. He does all the
        //      actual interface related work mostly.
        //
        //  m_rgbFrame
        //      When we got full screen, we set the frame's bgn color to match our client
        //      window color, so that it we don't have the frame's client margin showing up.
        //      We save them so we can put them back when restored.
        //
        //  m_strStateKey
        //      We pre-build the state repository key to avoid rebuilding it all the time.
        //
        //  m_tmidFlush
        //  m_tmidCtrl
        //      We need to start up one timer for flushing out current state info and another
        //      for responding to incoming remote control commands.
        //
        //  m_widNextClientId
        //      If we create new client windows upon loading, which we do in signage mode so
        //      that we can make sure the new one works before the old one is destroyed, we
        //      need to give each new one a new id since the old one has the previous id
        //      until it is destroyed.
        //
        //  m_wndActTrace
        //      An action trace window. It's created/destroyed as required. It cannot be
        //      closed while an action is running since it is plugged into the engine. It
        //      has a flag that is set on it around the invocation of the action to prevent
        //      this.
        //
        //  m_wndBlanker
        //      The blanker window, which we create/destroy as requested by the user.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bCachingEnabled;
        tCIDLib::TBoolean       m_bInitShow;
        tCIDLib::TBoolean       m_bStateChanged;
        tCIDLib::TCard4         m_c4StateId;
        TEventQ                 m_colEventQ;
        tCIDCtrls::EPosStates   m_eLastState;
        TCQCIntfViewState       m_fstCur;
        TCQCIntfContainer*      m_piwdgCmdParent;
        TCQCIntfViewWnd*        m_pwndClient;
        TRGBClr                 m_rgbFrame1;
        TRGBClr                 m_rgbFrame2;
        TString                 m_strStateKey;
        tCIDCtrls::TTimerId     m_tmidFlush;
        tCIDCtrls::TTimerId     m_tmidCtrl;
        tCIDCtrls::TWndId       m_widNextClientId;
        TActTraceWnd            m_wndActTrace;
        TCQCBlankerWnd          m_wndBlanker;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TMainFrameWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK


