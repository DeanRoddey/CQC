//
// FILE NAME: CQCIntfEng_ActEngine_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/16/2005
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
//  This is the header for the CQCIntfEng_ActEngine.cpp file, which implements
//  an action processing engine that handles the special needs of the user
//  interface system. We have various special considerations.
//
//  1.  We are generally running in a GUI type of situation so we can't just
//      run the action synchronously. It has to be run in a separate thread.
//
//  2.  That means that when widgets are targets, they cannot be directly
//      called from that background thread because they could be getting
//      accessed from the GUI thread at the same time.
//
//  3.  We can have nested actions, because the OnLoad of a template can
//      invoke a command on a widget, and that can cause that widget to
//      invoke some action that it's configured to do when it's changed in
//      some way, for instance.
//
//  4.  The widgets that are targets of an action could possibly go away
//      in the middle of an action, which can't happen in a standard action
//      engine where the targets are known and fixed. If a command reloads
//      the current template or overlay, for instance, any subsquent commands
//      in that action could reference the old stuff.
//
//  5.  Each popup layer that comes up is in a separate modal loop, so any
//      action that invoked one is now blocked until that popup exits, then
//      it picks back up.
//
//  So it's a much more complex scenario than the usual action engine needs.
//
//  So we need to define some stuff that lets us keep track of all these
//  special concerns. We define an 'action context' that we can associate
//  with each layer in the sequence of popups. This let's each layer have
//  its own separate action context that won't interfere with the underlying
//  ones. In order for a popup to even be present some action had to have
//  invoked it, so there are always nested actions where popups are concerned.
//
//  In the standard view class, it uses a template manager that maintains
//  a stack of various types of info, one slot per possible popup. One of
//  those sets of slots contains the action contexts for that level.
//
//  Also, since the action runs in the background, but widgets can be targets
//  of commands, the background thread, when it sees that the target is not
//  one of the standard targets, will do a synchronous post to the view to
//  let it handle finding the widget and passing the command on to it. The
//  bgn thread is blocked until that returns. So we define a simple structure
//  to pass that info from the bgn thread to the GUI thread.
//
//  Also, even within the context of a single template/popup context there
//  can be nested actions. If the user presses a button to scroll a list, that
//  may cause that list widget to invoke an action that happens when it is
//  scrolled. So we definte an interface action class that is used to store
//  info about each individual action. The context object maintains a stack
//  of these. When a new action is put onto the stack, it may be pushed on
//  the bottom (so that it runs after any current ones are done) or on the
//  top, which means it runs now and the current action is paused until the
//  nested one is done.
//
//  We also have to deal with event based actions, which any template can
//  be configured to use. So it responds to some incoming event and runs
//  an action. These are only allowed to happen one at a time and only when
//  no other action is active. So we have a flag in the action to remember
//  if it's an event based action.
//
//  Since we have possibly nested actions, and the engine runs on a bgn
//  thread, when anyone calls the view's PostOp method, it will see if
//  the engine is already running. If so, it will just queue up the new
//  one by pushing it on the stack. Otherwise, it will queue up the first
//  one, start up the engine, and block until the engine returns from the
//  action. This will often involve recursion of course because the GUI
//  thread blocks, the action runs, some action command calls a command
//  on a widget, so the engine posts to the GUI thread, which then causes
//  an action such as a popup, which pushes a new popup onto the view's
//  stack and enters a modal loop, and so on. So it can get quite nested
//  in the end. But it all unwinds eventually.
//
// CAVEATS/GOTCHAS:
//
//  1)  There are special considerations when in remote viewer mode, i.e.
//      when running in the remote viewer server. And there may be in some
//      other scenario in the future. So the view provides a bInterceptCmd()
//      method. Before we dispatch a non-GUI oriented command, we pass the
//      command to that call on the view (which must be callable from the
//      background thread) and if it returns true, we dispatch that command
//      in the GUI sort of way so that the view can handle it.
//
// LOG:
//
//  $Log$
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)

class TEvSrvCmdTar;

// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfAction
//  PREFIX: cia
// ---------------------------------------------------------------------------
class TCQCIntfAction : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfAction() = delete;

        TCQCIntfAction
        (
            const   TString&                strUserId
            , const TString&                strEvent
            , const MCQCCmdSrcIntf&         mcsrcToDo
            ,       tCQCKit::TCmdTarList&   colStdTars
            ,       tCQCKit::TCmdTarList&   colGUITars
            , const tCIDLib::TCard4         c4HandlerId
            , const tCIDLib::TBoolean       bEventBased
            , const TCQCUserCtx&            cuctxToUse
            , const TCQCActEngine&          acteTar
            , const TArea&                  areaSrcWidget
        );

        TCQCIntfAction(const TCQCIntfAction&) = delete;

        ~TCQCIntfAction();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfAction& operator=(const TCQCIntfAction&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        const TActOpcode& aocCurrent() const;

        tCIDLib::TBoolean bAtEnd() const;

        tCIDLib::TBoolean bEventBased() const;

        tCIDLib::TCard4 c4HandlerId() const;

        tCIDLib::TCard4 c4Index() const;

        tCIDLib::TCard4 c4Index
        (
            const   tCIDLib::TCard4         c4ToSet
        );

        tCIDLib::TCard4 c4OpCount() const;

        const TStdVarsTar& ctarLocalVars() const;

        TStdVarsTar& ctarLocalVars();

        TCQCCmdRTVSrc& crtsToUse() const;

        tCIDLib::TVoid FormatOpAt
        (
            const   tCIDLib::TCard4         c4At
            ,       TString&                strToFill
        )   const;

        tCIDLib::TVoid FormatCurOp
        (
                    TString&                strToFill
        )   const;

        tCIDLib::TVoid OffsetIndex
        (
            const   tCIDLib::TInt4          i4OfsBy
        );

        const TString& strEventId() const;

        const TString& strUserId() const;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEventBased
        //      We only want to have one event driven actino outstanding at
        //      any time, so the caller tells us if he is queuing up this
        //      action beause of an incoming event.
        //
        //  m_c4HandlerId
        //      The invoker of the action can provide a handler id that gets
        //      sent back in cases where it is needed, i.e. where there can
        //      be multiple possible handlers of a command call (mostly in
        //      calls back to interfaces which can have a base template plus
        //      popups.) This allows the template window to know which of
        //      the templates is being targeted so he can look up the target
        //      in the correct one.
        //
        //  m_c4Index
        //      To assist the host app's processing, we provide an index that
        //      they can use to keep up with where they are in our list. They
        //      have to handle nested chunks of commands and be able to get
        //      back to where they were when they pop off a nested set of
        //      commands.
        //
        //  m_colOps
        //      The opcodes (from the original source object) for the event
        //      to be invoked. We have to make a copy of them since the source
        //      could go away during processing.
        //
        //  m_pcrtsToUse
        //      We ask the command source object for a runtime value object
        //      that we can use to provide access to his RTVs.
        //
        //  m_strEventId
        //      The event id whose commands are queued up on this action.
        //
        //  m_strUserId
        //      An extra id that the caller can send along with the action
        //      for the target app to use for whatever purpose.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bEventBased;
        tCIDLib::TCard4                 m_c4HandlerId;
        tCIDLib::TCard4                 m_c4Index;
        MCQCCmdSrcIntf::TOpcodeBlock    m_colOps;
        TStdVarsTar                     m_ctarLocalVars;
        TCQCCmdRTVSrc*                  m_pcrtsToUse;
        TString                         m_strEventId;
        TString                         m_strUserId;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfAction,TObject)
};


namespace tCQCIntfEng
{
    // -----------------------------------------------------------------------
    //  When we invoke an action on a widget, we do it by a call out to the
    //  controlling viewer (who may in turn post it to itself via a window
    //  message if it's a window based viewer.) This structure is used to
    //  pass along all the info required to handle that one command.
    //
    //  The target for the command is looked up by the receiver using the
    //  target id in the command configuration. Mostly this is all just
    //  poitners because we don't want to replicate all the data just to let
    //  the viewer see it.
    //
    //  bInterceptedCmd is set if the view indicated it wanted to intercept
    //  a non-GUI command. If so, we send it to the view instead of directly
    //  calling the target. If it is set, then pctarIntercept is set so that
    //  the view doesn't have to go find it.
    // -----------------------------------------------------------------------
    struct TIntfCmdEv
    {
        // Returned info
        tCIDLib::TBoolean   bResult;
        tCIDLib::TBoolean   bTarNotFound;
        tCQCKit::ECmdRes    eRes;
        TError              errFailure;

        // Input info
        tCIDLib::TCard4     c4HandlerId;
        tCIDLib::TCard4     c4StepInd;
        TCQCCmdCfg*         pccfgToDo;
        const TString*      pstrEventId;
        const TString*      pstrUserId;
        TStdVarsTar*        pctarGlobals;
        TStdVarsTar*        pctarLocals;
        MCQCCmdTracer*      pcmdtDebug;

        // The action engine instance invoking it
        TCQCActEngine*      pacteOwner;

        tCIDLib::TBoolean   bInterceptedCmd;
        MCQCCmdTarIntf*     pmctarIntercept;
    };
}


// ---------------------------------------------------------------------------
//   CLASS: TCQCIntfActCtx
//  PREFIX: iac
// ---------------------------------------------------------------------------
class TCQCIntfActCtx : public TCQCActEngine
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        enum class EStates
        {
            Idle
            , Running
            , DoneOk
            , DoneCancel
            , DoneExit
            , DoneErr
            , DoneExcept
            , DoneUnknownExcept
        };
        using TActStack = TRefDeque<TCQCIntfAction>;


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCIntfActCtx() = delete;

        TCQCIntfActCtx
        (
            const   TCQCUserCtx&            cuctxToUse
        );

        TCQCIntfActCtx(const TCQCIntfActCtx&) = delete;

        ~TCQCIntfActCtx();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCIntfActCtx& operator=(const TCQCIntfActCtx&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCQCKit::TCmdTarList& colExtra() override;

        const TStdVarsTar& ctarLocals() const  override;

        TStdVarsTar& ctarLocals()  override;

        tCIDLib::TVoid Init
        (
            const   TString&                strActParms
        )   override;

        MCQCCmdTracer* const pcmdtDebug() override;

        tCIDLib::TVoid Reset() override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGotTrailingAction() const;

        tCIDLib::TBoolean bGotTrailingAction
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bPendingEventAct() const;

        tCIDLib::TBoolean bProcessing() const;

        tCIDLib::TBoolean bPopupRes() const;

        tCIDLib::TBoolean bPopupRes
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bQueryRetValue
        (
            const   TString&                strName
            ,       TString&                strToFill
        )   const;

        tCIDLib::TBoolean bQueryTmplValue
        (
            const   TString&                strName
            ,       TString&                strToFill
        )   const;

        tCQCKit::ECmdRes eInvokeCmds();

        tCQCKit::EActPostRes ePostOps
        (
                    TCQCIntfView* const     pcivOwner
            ,       TStdVarsTar* const      pctarGlobals
            ,       tCQCKit::TCmdTarList&   colExtraStdTars
            ,       MCQCCmdTracer* const    pcmdtDebug
            , const MCQCCmdSrcIntf&         mcsrcOps
            ,       tCQCKit::TCmdTarList&   colWdgTars
            , const TString&                strEvent
            , const TString&                strUserId
            , const tCIDLib::TCard4         c4HandlerId
            , const tCIDLib::TBoolean       bEventBased
            , const tCQCKit::EActOrders     eOrder
            , const TArea&                  areaSrcWidget
        );

        TString strCurUserId();

        const TString& strRetValue
        (
            const   TString&                strName
        )   const;

        const TString& strTmplValue
        (
            const   TString&                strName
        )   const;

        tCIDLib::TVoid SetRetValue
        (
            const   TString&                strName
            , const TString&                strToSet
        );

        tCIDLib::TVoid SetTmplValue
        (
            const   TString&                strName
            , const TString&                strToSet
        );

        tCIDLib::TVoid TakeRetInfoFrom
        (
            const   TCQCIntfActCtx&         ciacSrc
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eRunThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        MCQCCmdTarIntf* pmctarFind
        (
            const   tCIDLib::TCard4         c4Id
            ,       TCQCIntfAction&         ciaCur
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotTrailingAction
        //      There is a special case where a mouse release comes in after
        //      the thread is down, but before that action has unwound back
        //      out of the modal loop and cleaned up. This is very tricky
        //      since the thread isn't running but we can't start it up again.
        //      So we will set this flag when that happens. The view, when
        //      the action unwinds, will see the flag and know that there
        //      are more commands to process and will call back into the
        //      invoke method again without resetting us.
        //
        //  m_bPopupRes
        //      As with the return values below, the action can set a return
        //      value as a status if it wants for some invoker to see.
        //
        //  m_c4NestingLevel
        //      We get a nesting level assigned each time we crank up our
        //      thread (for a new action), and that is used by the thread to
        //      indicate when it is done.
        //
        //  m_colActStack
        //      This is the stack that we push actions onto to be processed.
        //      We can only process really one action at a time, but some
        //      actions cause other, nested, actions to be invoked (such as
        //      OnLoad of a loaded overlay), so we have to be able to push
        //      a new action onto the stack, process it, and then go back to
        //      where we were. Actually it's a deque because we also have to
        //      deal with the press/release/click set of actions that a user
        //      can cause. In that case, they can all happen before the press
        //      action is done, but they are not nested, so we have to push
        //      them on the bottom, to be processed serially.
        //
        //  m_colExtra
        //      Any of the non-GUI but non-standard (not implemented down in
        //      CQCKit with the base action interface stuff) targets need
        //      to be made accessiable via a virtual we inherit from the
        //      base engine class. We add the event server to this list, plus
        //      any that come in via the colExtraStdTars parameter. This guy
        //      is non-adopting, so it's fine to just point it at those.
        //
        //  m_colRetVals
        //      An invoked action can add return values that the invoking
        //      action can see. They are key/value pairs.
        //
        //  m_colTargets
        //      We keep a list of the standard targets here, for quick lookup.
        //      These cannot go away while the action is running. Anything
        //      not in this list, we assume is one of the widget targets
        //      and pass it off to the owning viewer.
        //
        //  m_colTmplVals
        //      An invoked action can store named values for its own internal
        //      use.
        //
        //  m_ctarXXX
        //      The standard targets that we always want to have. The caller
        //      can provide others. We put pointers to all of them into the
        //      m_colTargets collection. The locals are per-action, so they
        //      are in the GUI action class above. The globals are provided
        //      by the calling application.
        //
        //  m_eCurState
        //      This is set by the background thread to indicate what it's
        //      current state is. When our timer sees that the action has
        //      completed, it will close us (if we ever got created.)
        //
        //  m_errThrown
        //      If an exception occurs in the processing thread, it's stored
        //      here for the main thread to see it when it wakes up again.
        //
        //  m_mtxtSync
        //      See the file header comments about synchronization. This guy
        //      is mutable so we can lock it in const methods.
        //
        //  m_pcmdtDebug
        //      They can pass us a tracer object and we'll call back on it
        //      to provide then debugging info.
        //
        //  m_pctarGlobals
        //      We have to deal with the global variables target specialy
        //      in a number of places, so we keep a separate pointer to it.
        //
        //  m_pcivOwner
        //      The viewer that is running this action.
        //
        //  m_strStepInErr
        //      In order to display a human readable diagnostic of which
        //      step failed, the bgn thread pulls out the formatted command
        //      step of the failed one and put it here for use by the GUI
        //      thread.
        //
        //  m_thrRunner
        //      The thread object that we start up to run the action.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bGotTrailingAction;
        tCIDLib::TBoolean       m_bPopupRes;
        tCIDLib::TCard4         m_c4NestingLevel;
        TActStack               m_colActStack;
        tCQCKit::TCmdTarList    m_colExtra;
        tCIDLib::TKVPList       m_colRetVals;
        tCQCKit::TCmdTarList    m_colTargets;
        tCIDLib::TKVPList       m_colTmplVals;
        TEvSrvCmdTar            m_ctarEventSrv;
        TStdFieldCmdTar         m_ctarFlds;
        TStdMacroCmdTar         m_ctarMacros;
        TStdSystemCmdTar        m_ctarSystem;
        EStates                 m_eCurState;
        TError                  m_errThrown;
        mutable TMutex          m_mtxSync;
        MCQCCmdTracer*          m_pcmdtDebug;
        TStdVarsTar*            m_pctarGlobals;
        TCQCIntfView*           m_pcivOwner;
        TString                 m_strStepInErr;
        TThread                 m_thrRunner;


        // -------------------------------------------------------------------
        //  Magic Macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCIntfActCtx,TCQCActEngine)
};

#pragma CIDLIB_POPPACK

