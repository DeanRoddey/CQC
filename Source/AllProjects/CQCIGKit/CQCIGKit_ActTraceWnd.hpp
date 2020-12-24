//
// FILE NAME: CQCIGKit_ActTraceWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/21/2006
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
//  This is the header for the CQCIGKit_ActTraceWnd.cpp file, which implements
//  a simple window that mixes in the CQCKit action system's action trace
//  mixin interface. This allows it to be plugged into the standard fgn or
//  bgn action engines, and they will post trace information back to us about
//  what's being done. We just dump it out to a text output windows as it
//  happens.
//
//  Since the action can be running in a background thread, we actually just
//  queue the incoming info and have a timer dump it out.
//
//  This guy can be run as either a separate top level frame window, or as
//  a modal window. The interface viewer uses it in the former way, other
//  scenarios such as testing IR receiver actions or scheduled events will
//  use it as a modal dialog to let them test that one action and then
//  close it.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCStdGUIActEngine;


// ---------------------------------------------------------------------------
//   CLASS: TActTraceWnd
//  PREFIX: dlg
// ---------------------------------------------------------------------------
class CQCIGKITEXPORT TActTraceWnd : public TFrameWnd, public MCQCCmdTracer
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TActTraceWnd();

        TActTraceWnd(const TActTraceWnd&) = delete;

        ~TActTraceWnd();


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TActTraceWnd& operator=(const TActTraceWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActCmd
        (
            const   TActOpcode&             aocOriginal
            , const TCQCCmdCfg&             ccfgExpanded
            , const tCIDLib::TCard4         c4Step
        );

        tCIDLib::TVoid ActDebug
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid ActEnd
        (
            const   tCIDLib::TBoolean       bStatus
        );

        tCIDLib::TVoid ActStartNew
        (
            const   TStdVarsTar&            ctarGlobals
        );

        tCIDLib::TVoid ActVarRemoved
        (
            const   TString&                strKey
        );

        tCIDLib::TVoid ActVarSet
        (
            const   TString&                strKey
            , const TString&                strValue
        );

        tCIDLib::TBoolean bLocked() const;

        tCIDLib::TBoolean bLocked
        (
            const   tCIDLib::TBoolean       bToSet
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInAction() const;

        tCIDLib::TVoid ClearOutput();

        tCIDLib::TVoid CreateWindow
        (
            const   TString&                strIconName
        );

        tCIDLib::TVoid RunModally
        (
            const   TWindow&                wndOwner
            , const MCQCCmdSrcIntf&         csrcToRun
            , const TCQCUserCtx&            cuctxToUse
            , const TString&                strActionName
            , const TString&                strEventId
            , const tCIDLib::TBoolean       bGetParms
            , const TString&                strIconName
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ActivationChange
        (
            const   tCIDLib::TBoolean       bState
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

        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

        tCIDLib::TVoid Timer
        (
            const   tCIDCtrls::TTimerId     tmidToDo
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDLib::TVoid QueueLine
        (
            const   TString&                    strToQ
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bBreakFlag
        //      When run modally, the modal loop takes a break flag to get it out of the
        //      loop. When in this modal mode, the Close button just sets this flag to make
        //      the window exit.
        //
        //  m_bGetParms
        //      The caller tells us if the action is of the type that the caller can provide
        //      parameters to. If so, we'll ask for them, else we pass in an empty string.
        //
        //  m_bModal
        //      This is set if we are invoked modally. We use it to control whether we do
        //      some things.
        //
        //  m_c4InAction
        //      This is bumped up on each action entry and down on each exit. If non-zero,
        //      we are in an action.
        //
        //  m_c4Locked
        //      The action engine will call bLocked() on any action tracer it is given, to
        //      insure it doesn't go away while the action is running, then again to release
        //      it at the end. However those can be nested, so we maintain a counter. For
        //      locks we bump for unlocks we decrement. When it's zero we can go away.
        //
        //  m_colQueue
        //      We queue up incoming callback info from the trace interface and our timer
        //      handler grabs stuff out of the queue and dumps it to the text output window.
        //      It's thread safe so both the timer and callbacks can access it for atomic
        //      ops without locking explicitly.
        //
        //  m_enctLast
        //      The time of the last command. Each time we reset it so that we can time
        //      the length of each action step.
        //
        //  m_pacteBgn
        //      If we are being invoked as a modal popup, then we have to invoke the action
        //      ourself when the users pressed Run. So we need to create a standard GUI based
        //      action engine to run the action. We give ourself as the debugger interface
        //      so he'll call us back and we can dump the output. It's only created when needed.
        //
        //  m_pcsrcToRun
        //      If we are being invoked as a modal popup, then we get a cmd source that has
        //      the action we are to run when they press the Run button. If not, this is null.
        //
        //  m_pctarGlobals
        //      If we are being invoked as a modal popup, then we need to maintain a global
        //      variables namespace while the window is up. We pass it into the action engine
        //      when the user invokes the action. Only created if needed.
        //
        //  m_pcuctxToUse
        //      When run as a modal window we have to be able to invoke the provided action,
        //      so the caller also has to provide us with a user context to provide in the
        //      action invocation.
        //
        //  m_pwndOwner
        //      When run as a modal window we have an owner window to be modal to. When
        //      run as a separate top level window this is null.
        //
        //  m_strActName
        //      If invoked modally, we are testing a known action, and the caller provides
        //      a title to put in the title bar.
        //
        //  m_strDivider
        //  m_strDivider2
        //      Some prefab strings that we put out regularly
        //
        //  m_strEventId
        //      If being invoked modally, the caller has to provide the id of some event to
        //      invoke on the command source.
        //
        //  m_strLine
        //      A string to use for reading in lines from the queue.
        //
        //  m_strPrevTestParams
        //      To remember previous test parameters so that we can make them the default
        //      the next time the user hits Run.
        //
        //  m_tmidUpdate
        //      We start a timer to grab msgs posted by hte bgn thread and load them out to
        //      the text output window.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bBreakFlag;
        tCIDLib::TBoolean       m_bGetParms;
        tCIDLib::TBoolean       m_bModal;
        tCIDLib::TCard4         m_c4InAction;
        tCIDLib::TCard4         m_c4Locked;
        TQueue<TString>         m_colQueue;
        tCIDLib::TEncodedTime   m_enctLast;
        TCQCStdGUIActEngine*    m_pacteBgn;
        const MCQCCmdSrcIntf*   m_pcsrcToRun;
        TStdVarsTar*            m_pctarGlobals;
        const TCQCUserCtx*      m_pcuctxToUse;
        TPushButton*            m_pwndCloseButton;
        const TWindow*          m_pwndOwner;
        TPushButton*            m_pwndRunButton;
        TTextOutWnd*            m_pwndOutput;
        TString                 m_strActName;
        TString                 m_strDivider;
        TString                 m_strDivider2;
        TString                 m_strEventId;
        TString                 m_strLine;
        TString                 m_strPrevTestParams;
        tCIDCtrls::TTimerId     m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TActTraceWnd,TFrameWnd)
};

#pragma CIDLIB_POPPACK

