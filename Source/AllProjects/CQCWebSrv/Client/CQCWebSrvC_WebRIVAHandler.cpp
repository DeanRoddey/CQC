//
// FILE NAME: CQCWebSrvC_WebRIVAHandler.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/27/2017
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
//  This file implements the handler for the WebBrowser based RIVA client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_.hpp"
#include    "CIDJPEG.hpp"
#include    "CIDPNG.hpp"


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCWebSrvC_WebRIVAHandler
{
    namespace
    {
        // Some bespoke Websocket error codes, starting at the per-application base code
        constexpr tCIDLib::TCard2   c2WSockErr_LoginFailed      = kCQCWebSrvC::c2WSockErr_AppBase;

        //
        //  Some time periods that we use in the Idle callback to invoke various update cycles
        //  on the view.
        //
        constexpr tCIDLib::TEncodedTime enctActive(100 * kCIDLib::enctOneMilliSec);
        constexpr tCIDLib::TEncodedTime enctValue(250 * kCIDLib::enctOneMilliSec);
        constexpr tCIDLib::TEncodedTime enctEvent(2 * kCIDLib::enctOneSecond);
        constexpr tCIDLib::TEncodedTime enctTOCheck(kCIDLib::enctOneSecond);
    }
};



// ---------------------------------------------------------------------------
//   CLASS: TWebSockRIVAThread::TGUIEvent
//  PREFIX: ev
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TWebSockRIVAThread::TGUIEvent: Public, static methods
// ---------------------------------------------------------------------------

//
//  Compares two events for the same type. This is used as a comparator function in evnt
//  queue operations that require one. We use it to only post some event types if there's
//  not already one in the queue, to avoid getting it backed up.
//
tCIDLib::TBoolean TWebSockRIVAThread::TGUIEvent::
bCompEvent( const   TWebSockRIVAThread::TGUIEvent& ev1
            , const TWebSockRIVAThread::TGUIEvent& ev2)
{
    return ev1.m_eType == ev2.m_eType;
}


// ---------------------------------------------------------------------------
// TWebSockRIVAThread::TGUIEvent: Constructors and Destructors
// ---------------------------------------------------------------------------
TWebSockRIVAThread::TGUIEvent::TGUIEvent() :

    m_c4Val1(0)
    , m_c4Val2(0)
    , m_eType(TWebSockRIVAThread::EGUIEvTypes::None)
    , m_f8Val(0.0)
    , m_i4Val1(0)
    , m_i4Val2(0)
    , m_i4Val3(0)
    , m_i4Val4(0)
    , m_pevNotify(nullptr)
    , m_piceInfo(nullptr)
    , m_padcbInfo(nullptr)
{
}

// A specialized one for async data callbacks
TWebSockRIVAThread::TGUIEvent::TGUIEvent(TCQCIntfADCB* const padcbInfo) :

    m_c4Val1(0)
    , m_c4Val2(0)
    , m_eType(TWebSockRIVAThread::EGUIEvTypes::AsyncDCB)
    , m_f8Val(0.0)
    , m_i4Val1(0)
    , m_i4Val2(0)
    , m_i4Val3(0)
    , m_i4Val4(0)
    , m_pevNotify(nullptr)
    , m_piceInfo(nullptr)
    , m_padcbInfo(padcbInfo)
{
}

TWebSockRIVAThread::TGUIEvent::TGUIEvent(const EGUIEvTypes eType) :

    m_c4Val1(0)
    , m_c4Val2(0)
    , m_eType(eType)
    , m_f8Val(0.0)
    , m_i4Val1(0)
    , m_i4Val2(0)
    , m_i4Val3(0)
    , m_i4Val4(0)
    , m_pevNotify(nullptr)
    , m_piceInfo(nullptr)
    , m_padcbInfo(nullptr)
{
}

TWebSockRIVAThread::TGUIEvent::~TGUIEvent()
{
    // If it didn't get cleaned up, then destroy it
    if (m_padcbInfo)
    {
        try
        {
            delete m_padcbInfo;
        }

        catch(...)
        {
        }
        m_padcbInfo = nullptr;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TWebSockRIVAThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWebSockRIVAThread: Constructors and destructor
// ---------------------------------------------------------------------------
TWebSockRIVAThread::TWebSockRIVAThread(const tCIDLib::TCard1 c1ThreadId) :

    TWebsockThread(tCQCWebSrvC::EWSockTypes::RIVA, kCIDLib::False)
    , m_bClientVisState(kCIDLib::True)
    , m_bEnableCaching(kCIDLib::True)
    , m_bGUIBailOut(kCIDLib::False)
    , m_bLogGUIEvents(kCIDLib::False)
    , m_bLogInMsgs(kCIDLib::False)
    , m_bShutdownReq(kCIDLib::False)
    , m_c4TimerSuspend(0)
    , m_colGUIEvQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_enctLastMsg(0)
    , m_enctNextActive(0)
    , m_enctNextEvent(0)
    , m_enctNextVal(0)
    , m_enctNextTOCheck(0)
    , m_eCurState(EStates::WaitSessState)
    , m_f8ClientLat(kCQCWebRIVA::f8LocNotSet)
    , m_f8ClientLong(kCQCWebRIVA::f8LocNotSet)
    , m_gesthInp(this)
    , m_pcivTarget(nullptr)
    , m_szDevRes(800, 600)
    , m_thrFauxGUIThread
      (
        TString(L"CQCWebRIVAGUIThread_") + TString(TCardinal(c1ThreadId))
        , TMemberFunc<TWebSockRIVAThread>(this, &TWebSockRIVAThread::eFauxGUIThread)
      )
{
}

TWebSockRIVAThread::~TWebSockRIVAThread()
{
}


// ---------------------------------------------------------------------------
//  TWebSockRIVAThread: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TWebSockRIVAThread::AsyncCallback(const tCQCIntfEng::EAsyncCmds, TCQCIntfView&)
{
    // A no-op for now. We might implement some commands later
}


//
//  Queue this up. The faux GUI thread will grab it and make the call back to the
//  view.
//
tCIDLib::TVoid
TWebSockRIVAThread::AsyncDataCallback(TCQCIntfADCB* const padcbInfo)
{
    // Queue up a GUI event for this guy
    m_colGUIEvQ.Add(new TGUIEvent(padcbInfo));
}


// If our view is set, then pass it on
tCIDLib::TBoolean
TWebSockRIVAThread::bProcessGestEv( const   tCIDCtrls::EGestEvs eEv
                                    , const TPoint&             pntStart
                                    , const TPoint&             pntAt
                                    , const TPoint&             pntDelta
                                    , const tCIDLib::TBoolean   bTwoFingers)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;
    if (m_pcivTarget)
    {
        bRet = m_pcivTarget->bProcessGestEv
        (
            eEv, pntStart, pntAt, pntDelta, bTwoFingers
        );
    }
    return bRet;
}


// We just need to send this on to the client
tCIDLib::TVoid
TWebSockRIVAThread::CreateRemWidget(const   TString&                    strUID
                                    , const tCQCIntfEng::ERIVAWTypes    eType
                                    , const TArea&                      areaAt
                                    , const tCIDLib::TKVPList&          colParams
                                    , const tCIDLib::TBoolean           bInitVis)
{
    // Turn the parameters into a raw JSON array
    TString strParams;
    MakeRawObject(colParams, strParams);

    TWebRIVATools wrtToUse;
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = wrtToUse.pmbufFormatCreateRemWidget
    (
        c4Bytes, strUID, tWebRIVA::EWdgTypes(eType), areaAt, strParams, bInitVis
    );
    SendMsg(pmbufNew, c4Bytes);
}


// Send this on to the client
tCIDLib::TVoid
TWebSockRIVAThread::DestroyRemWidget(const  TString&                    strUID
                                    , const tCQCIntfEng::ERIVAWTypes    eType)
{
    TWebRIVATools wrtToUse;
    tCIDLib::TCard4 c4Bytes;
    TMemBuf* pmbufNew = wrtToUse.pmbufFormatDestroyRemWidget
    (
        c4Bytes, strUID, tWebRIVA::EWdgTypes(eType)
    );
    SendMsg(pmbufNew, c4Bytes);
}


//
//  This is called by the int engine becase we register ourself as a handler
//  for special actions. This one for now is just ignored in remote mode.
//
tCIDLib::TVoid TWebSockRIVAThread::DismissBlanker(TCQCIntfView&)
{
}


//
//  We have to emulate the way action command processing works in the GUI scenario, so
//  our view derivative calls this method any time an action command targets one of the
//  widgets. We queue it up on the faux GUI thread's queue and then block till he
//  finishes processesing it.
//
//  This correctly serializes access to the template and allows the action engine to
//  call back into the GUI message loop if a popup is done (which otherwise would happen
//  from the background thread that the action runs in.
//
//  WE CANNOT allow any exception to propogate out of here, since it will propogate in
//  the Faux GUI thread, not back to the action thread. So we catch them and return the
//  info in the passed command event object.
//
tCIDLib::TVoid
TWebSockRIVAThread::DispatchActEvent(tCQCIntfEng::TIntfCmdEv& iceToDo)
{
    // Queue up a GUI event and point it at an event that we'll wait on
    TEvent evWait(tCIDLib::EEventStates::Reset);

    TGUIEvent* pgevDispatch = new TGUIEvent(EGUIEvTypes::DispatchActEv);
    pgevDispatch->m_pevNotify = &evWait;
    pgevDispatch->m_piceInfo = &iceToDo;
    m_colGUIEvQ.Add(pgevDispatch);

    // Now wait for the event to trigger
    evWait.WaitFor();
}


//
//  This is called by the int engine becase we register ourself as a handler
//  for special actions. We handle the exit one ourself, and pass the others
//  to the client.
//
tCIDLib::TVoid
TWebSockRIVAThread::DoSpecialAction(const   tCQCIntfEng::ESpecActs eSpecAct
                                    , const tCIDLib::TBoolean
                                    ,       TCQCIntfView&)
{
    //
    //  We handle the exit action if a popup is up, else we pass everything
    //  to the client to handle.
    //
    if ((eSpecAct == tCQCIntfEng::ESpecActs::Exit) && m_pcivTarget->bHasPopups())
    {
        // We need to queue an exit loop command to the GUI thread
        m_colGUIEvQ.Add(new TGUIEvent(EGUIEvTypes::ExitLoop));
    }
     else
    {
        TWebRIVATools wrtToUse;
        tCIDLib::TCard4 c4Bytes;
        TMemBuf* pmbufNew = wrtToUse.pmbufFormatSpecialAction
        (
            c4Bytes, tWebRIVA::ESpecialActs(eSpecAct)
        );
        SendMsg(pmbufNew, c4Bytes);
    }
}


// Always just report that we are in remote mode
tCQCIntfEng::EAppFlags TWebSockRIVAThread::eAppFlags() const
{
    return tCQCIntfEng::EAppFlags::RemoteMode;
}


//
//  We start up a secondary processing thread, which just calls this method. This loop
//  simulates a GUI type event loop thread, so that things can work more like in the
//  standard Window based template engine. The worker thread posts us events to service.
//  This also must makes it esier for us as well, since it allows this worker thread
//  to concentrate purely on servicing the client and for fully async communications
//  between the two threads via queues.
//
//  If one of our press/release events (which are passed to the interface engine) causes
//  an action to occur, then the interface engine will start another thread to process
//  the action. If any command target is a widget, then that thread will call back into
//  our view, asking for the cmd to be dispatched. The view will call DispatchActEvent()
//  above. The thread will queue up an item in our queue. We'll look up the target
//  widget and perform the command, then trigger the event in the queued item to let the
//  calling action thread go.
//
//  If the command is to do a popup/popout, then the command will not return until the
//  popup is closed. The engine will call back into here (on this same thread) to do what
//  would be a modal loop in a real GUI. So the action thread will remain blocked until
//  that popup is exited, in which case we'll get an ExitLoop command, which will unwind
//  back to the previous recursion level, fall out of the action dispatch command below,
//  and that will release the waiting action thread. If the action is not done, then
//  it will happen more times. If it is, then the action thread will end, and we will come
//  back out of the Press/Release block below that started the whole thing, and we are now
//  unrecursed for that action.
//
//  So, recursion happens, but it's always on this thread. The action threads don't actually
//  call this method. They block above and wait for the posted action command to complete.
//
//  To support unwinding the stack completely, we have the m_bGUIBailout thread. If it's
//  set, that will cause all the nested loops to exit. It will also cause all waiting action
//  threads to be released (and we'll pass back it back a status that tells it to stop
//  processing the action. Since we could only be in a nested scenario if we were recursed
//  back in from a dispatch action event command, each exit from the subsequently invoked
//  modal loop will get us back out of the dispatch below which will wake up the action
//  thread and tell it to stop.
//
//  So the GUI bailout flag cleanly gets the whole GUI loop unwound and all nested
//  actions stopped.
//
tCIDLib::TVoid TWebSockRIVAThread::FauxGUILoop(tCIDLib::TBoolean& bBreakFlag)
{
    while (!bBreakFlag && !m_bGUIBailOut)
    {
        TGUIEvent* pgevCur = nullptr;
        try
        {
            // TEmp debug code
            m_eLastGUIEv = EGUIEvTypes::None;

            // Wait for an event to process
            pgevCur = m_colGUIEvQ.pobjGetNext(100, kCIDLib::False);
            if (pgevCur)
            {
                // Temp debug code
                m_eLastGUIEv = pgevCur->m_eType;

                TJanitor<TGUIEvent> janEvent(pgevCur);
                switch(pgevCur->m_eType)
                {
                    case EGUIEvTypes::ActiveUpdate :
                        //
                        //  Ask the view to do a active update pass. This one
                        //  we post to ourself, it doesn't come from the client.
                        //
                        m_pcivTarget->DoActiveUpdatePass();
                        break;

                    case EGUIEvTypes::AsyncDCB :
                    {
                        //
                        //  Get the callback object out and orphan from the event
                        //  into a janitor to insure it gets cleaned up but the event
                        //  doesn't try to do it again.
                        //
                        TCQCIntfADCB* padcbInfo = pgevCur->m_padcbInfo;
                        pgevCur->m_padcbInfo = nullptr;
                        if (padcbInfo)
                        {
                            TJanitor<TCQCIntfADCB> janData(padcbInfo);

                            if (m_bLogGUIEvents)
                            {
                                TString strMsg(L"Handling asyncCB. CBId=");
                                strMsg += pgevCur->m_padcbInfo->m_strCBId;
                                LogStatusMsg(strMsg, CID_LINE);
                            }

                            // And now do the callback
                            padcbInfo->m_pcivCaller->AsyncDataCallback(padcbInfo);
                        }
                        break;
                    }

                    case EGUIEvTypes::CancelInput :
                    {
                        if (m_bLogGUIEvents)
                            LogStatusMsg(L"Got gesture cancel", CID_LINE);
                        m_gesthInp.CancelGesture();
                        break;
                    }

                    case EGUIEvTypes::CheckTimeout :
                    {
                        // Let the view see if it's time for a timeout event
                        m_pcivTarget->bCheckTimeout();
                        break;
                    }

                    case EGUIEvTypes::DispatchActEv :
                    {
                        // Handle an action command from a background action thread
                        if (m_bLogGUIEvents)
                        {
                            TString strMsg(L"Handling dispatched action command. Target=");
                            strMsg += pgevCur->m_piceInfo->pccfgToDo->strTargetName();
                            strMsg += L", Cmd=";
                            strMsg += pgevCur->m_piceInfo->pccfgToDo->strCmdId();
                            LogStatusMsg(strMsg, CID_LINE);
                        }
                        HandleGUIActDispatch(*pgevCur);
                        break;
                    }

                    case EGUIEvTypes::ExitLoop :
                        if (m_bLogGUIEvents)
                            LogStatusMsg(L"Processing exit event", CID_LINE);
                        bBreakFlag = kCIDLib::True;
                        break;

                    case EGUIEvTypes::EventUpdate :
                        // We don't currently handle async events in RIVA
                        break;

                    case EGUIEvTypes::HotKey :
                    {
                        // Pass the hot key on to the viewer
                        CIDAssert(m_pcivTarget != nullptr, L"Got mouse  event before view was set")
                        m_pcivTarget->HotKey(tCQCIntfEng::EHotKeys(pgevCur->m_c4Val1));
                        break;
                    }

                    case EGUIEvTypes::Move :
                    {
                        // Pass the mouse movement to the viewer
                        CIDAssert(m_pcivTarget != nullptr, L"Got mouse event before view was set")

                        TPoint pntAt(pgevCur->m_i4Val1, pgevCur->m_i4Val2);
                        m_gesthInp.HandleMsg(tCQCWebRIVA::EGestEvs::Move, pntAt);
                        break;
                    }

                    case EGUIEvTypes::Press :
                    case EGUIEvTypes::Release :
                    {
                        //
                        //  Pass the mouse press/release to the viewer.
                        //
                        CIDAssert(m_pcivTarget != nullptr, L"Got mouse event before view was set")

                        if (m_bLogGUIEvents)
                        {
                            LogStatusMsg
                            (
                                (pgevCur->m_eType == EGUIEvTypes::Press) ? L"Processing press event"
                                                                         : L"Processing release event"
                                , CID_LINE
                            );
                        }

                        TPoint pntAt(pgevCur->m_i4Val1, pgevCur->m_i4Val2);
                        m_gesthInp.HandleMsg
                        (
                            (pgevCur->m_eType == EGUIEvTypes::Press)
                                    ? tCQCWebRIVA::EGestEvs::Press
                                    : tCQCWebRIVA::EGestEvs::Release
                            , pntAt
                        );
                        break;
                    }

                    case EGUIEvTypes::Redraw :
                    {
                        // Force a redraw of the whole view
                        m_pcivTarget->Redraw();
                        break;
                    }

                    case EGUIEvTypes::SetVisState :
                    {
                        //
                        //  The client has changed bgn/fgn state. Set a flag to remember
                        //  this state. If it's not visible, we'll stop dispatching graphics
                        //  commands. If he came back, then force a redraw.
                        //
                        const tCIDLib::TBoolean bNewState(pgevCur->m_c4Val1 != 0);
                        if (m_bClientVisState != bNewState)

                            if (m_bLogGUIEvents)
                            {
                                if (bNewState)
                                    LogStatusMsg(L"Client has moved to fgn state", CID_LINE);
                                else
                                    LogStatusMsg(L"Client has moved to bgn state", CID_LINE);
                            }
                                                                    {
                            m_bClientVisState = bNewState;
                            if (m_bClientVisState)
                                m_pcivTarget->Redraw();
                        }
                        break;
                    }

                    case EGUIEvTypes::SizeChange :
                    {
                        if (m_bLogGUIEvents)
                            LogStatusMsg(L"Procesing display size change event", CID_LINE);

                        // Tell the view that the view area changed
                        TSize szNew(pgevCur->m_c4Val1, pgevCur->m_c4Val2);
                        m_pcivTarget->NewSize(szNew);
                        break;
                    }

                    case EGUIEvTypes::ValueUpdate :
                        //
                        //  Ask the view to do a value based update pass. This
                        //  one we post to ourself, it doesn't come from the
                        //  client.
                        //
                        m_pcivTarget->DoUpdatePass();
                        break;

                    default :
                        break;
                };
            }
        }

        catch(TError& errToCatch)
        {
            // Just in case, so we don't go crazy in a worst case scenario
            TThread::Sleep(100);

            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadError
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_strSessionName
            );
        }

        catch(...)
        {
            // Just in case, so we don't go crazy in a worst case scenario
            TThread::Sleep(100);

            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadError
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_strSessionName
            );
        }
    }
}



// If our view is set, just pass it on
tCIDLib::TVoid TWebSockRIVAThread::GestClick(const TPoint& pntAt)
{
    if (m_pcivTarget)
        m_pcivTarget->Clicked(pntAt);
}


// If our view is set, just pass it on
tCIDLib::TVoid
TWebSockRIVAThread::GestFlick(const  tCIDLib::EDirs  eDirection
                        , const TPoint&         pntStartPos)
{
    if (m_pcivTarget)
        m_pcivTarget->ProcessFlick(eDirection, pntStartPos);
}


// A no-op for us
tCIDLib::TVoid
TWebSockRIVAThread::GestInertiaIdle( const   tCIDLib::TEncodedTime
                                , const tCIDLib::TCard4
                                , const tCIDLib::EDirs)
{
}


// A no-op for us
tCIDLib::TVoid TWebSockRIVAThread::GestReset(const tCIDCtrls::EGestReset)
{
}


//
//  Let the client know a new template was loaded and it's size, and we pass the border
//  color that's currently set.
//
tCIDLib::TVoid
TWebSockRIVAThread::NewTemplate(const   TCQCIntfTemplate&   iwdgNew
                                ,       TCQCIntfView&
                                , const tCIDLib::TCard4     c4StackPos)
{
    TWebRIVATools wrtToUse;

    //
    //  If it's a new base template, we need to update our virtual device res. In the
    //  RIVA scenario there's no actual screen so we keep it set to the size of the
    //  base template. Only do this if the view is already created since it will also
    //  get called on initial startup before the view exists, and all of this kind
    //  of stuff will be getting taken care of differently.
    //
    if ((c4StackPos == 0) && m_pcivTarget)
    {
        m_szDevRes = iwdgNew.areaActual().szArea();

        TWebRIVAGraphDev* pgdevNew = new TWebRIVAGraphDev(m_szDevRes, this);
        m_cptrRemDev.SetPointer(pgdevNew);

        // Let the view update itself
        m_pcivTarget->NewSize(m_szDevRes);
    }

    tCIDLib::TCard4 c4Size;
    TMemBuf* pmbufMsg = wrtToUse.pmbufFormatNewTemplate
    (
        c4Size, iwdgNew.strTemplateName(), iwdgNew.areaActual().szArea()
    );
    SendMsg(pmbufMsg, c4Size);
}


MCQCCmdTracer* TWebSockRIVAThread::pmcmdtTrace()
{
    // A No-op for us
    return nullptr;
}


tCIDLib::TVoid
TWebSockRIVAThread::PauseTimers(const tCIDLib::TBoolean bPauseState)
{
    if (bPauseState)
    {
        m_c4TimerSuspend++;
    }
     else
    {
        CIDAssert(m_c4TimerSuspend > 0, L"Timer supsension counter underflow");
        if (m_c4TimerSuspend)
            m_c4TimerSuspend--;
    }
}


// If our view is set, just pass it on
tCIDLib::TVoid
TWebSockRIVAThread::PerGestOpts(const   TPoint&                 pntAt
                                , const tCIDLib::EDirs          eDir
                                , const tCIDLib::TBoolean       bTwoFingers
                                ,       tCIDCtrls::EGestOpts&   eOpts
                                ,       tCIDLib::TFloat4&       f4VScale)
{
    if (m_pcivTarget)
        m_pcivTarget->PerGestOpts(pntAt, eDir, bTwoFingers, eOpts, f4VScale);
}


//
//  Any messages sent from the RIVA stuff will go through these methods. Partly because the
//  underlying message queuing methods in our base class are protected so that other code
//  needs us to provide access to them, and also partly so that we can intercept them and
//  do any necessary magic along the way.
//

//
//  This one is called by the redirecting graphics device. It could call SendMsg directly,
//  but we need to be able to suppress drawing commands when the client is not in the fgn,
//  so they call this. If the client is visible, we queue the msg, else we just eat it.
//
tCIDLib::TVoid
TWebSockRIVAThread::SendGraphicsMsg(TMemBuf* const pmbufToAdopt, const tCIDLib::TCard4 c4Size)
{
    if (m_bClientVisState)
        QueueTextMsg(pmbufToAdopt, c4Size);
    else
        delete pmbufToAdopt;
}


//
//  This one is used to send any image related messages. We see if the image is in the client's
//  image map. If not, then we send it. That way, he has the image before he needs to draw it.
//
//  Because we can get various types of images, but we don't want to have to worry about going
//  to find the original image data. We know we have the image, because we are drawing it
//  right now. So we just convert it to PNG or JPG and send that. This avoids all kinds of
//  complications that would otherwise occur, and also allows us to deal with images that the
//  IV builds on the fly, such as color palettes.
//
//  If the image is a CQC image repo image or an on the fly image, we send it as PNG, else we
//  send it as JPEG.
//
//  If the client is not visible, in a bgn tab, we just eat the message. When it comes forward
//  again a full redraw will be done, and this will drawn again and it still won't be in the
//  client's cache (if its not now) and so we will send it then.
//
tCIDLib::TVoid
TWebSockRIVAThread::SendImgMsg( const   TString&            strFullPath
                                , const TBitmap&            bmpToSend
                                ,       TMemBuf* const      pmbufToAdopt
                                , const tCIDLib::TCard4     c4Size)
{
    // Just in case...
    TJanitor<TMemBuf> janBuf(pmbufToAdopt);

    // If the client isn't visible we just return, and eat the msg
    if (!m_bClientVisState)
        return;

    // Get out the path and serial number
    const tCIDLib::TCard4 c4SerialNum = bmpToSend.c4SerialNum();

    const TRIVAImgItem* pimiTar = m_rimapClient.pimiFind(strFullPath);
    if (!pimiTar || (pimiTar->objValue() != c4SerialNum))
    {
        try
        {
            //
            //  Log the paths of the messages we are sending, to correlate with the browser
            //  console output
            //
            #if CID_DEBUG_ON
            //facCQCWebSrvC().LogMsg
            //(
            //    CID_FILE
            //    , CID_LINE
            //    , strFullPath
            //    , tCIDLib::ESeverities::Status
            //);
            #endif

            //
            //  Let's convert this guy to a PNG first. Though in some cases we can get direct
            //  access to the pixel data, we also need to potentially get the palette if it is
            //  palette based. So we just query the pixel and palette info.
            //
            TPixelArray pixaData;
            TClrPalette palData;
            bmpToSend.QueryImgData(m_pcivTarget->gdevCompat(), pixaData, palData);

            // Generate an image from this
            tCIDLib::TBoolean bIsPNG;
            TCIDImage* pimgToSend = nullptr;
            if (strFullPath.bStartsWith(kCQCKit::strWRIVA_RepoPref)
            ||  strFullPath.bStartsWith(kCQCKit::strWRIVA_IVOTFPref))
            {
                bIsPNG = kCIDLib::True;
                pimgToSend = new TPNGImage(pixaData, palData);
            }
             else
            {
                bIsPNG = kCIDLib::False;
                pimgToSend = new TJPEGImage(pixaData, palData);
            }
            TJanitor<TCIDImage> janImg(pimgToSend);

            // Convert it to Base64 in a string
            tCIDLib::TCard4 c4BinImgBytes = 0;
            TString strEncoded(pimgToSend->c4ImageSz() + (pimgToSend->c4ImageSz() / 4));
            {
                // Create a buffer to hold the raw flattened PNG and stream it out
                TBinMBufOutStream strmRaw(strEncoded.c4BufChars());
                strmRaw << *pimgToSend << kCIDLib::FlushIt;
                c4BinImgBytes = strmRaw.c4CurSize();

                //
                //  Create a linked input stream over the output stream so we can stream from
                //  it directly and avoid any copying.
                //
                TBinMBufInStream strmRawIn(strmRaw);

                // Now create one over the target string and encode to that
                TTextStringOutStream strmTar(&strEncoded);
                TBase64 b64Encode;

                // Disable line breaks by setting the line width to max card
                b64Encode.c4LineWidth(kCIDLib::c4MaxCard);
                b64Encode.Encode(strmRawIn, strmTar);
                strmTar.Flush();
            }

            //
            //  Now we send the image data in chunks. We have to send a first image
            //  msg first. If that covers the whole image, then fine, else we continue
            //  until the last one.
            //
            TWebRIVATools wrtToUse;
            TMemBuf* pmbufCur = nullptr;

            //
            //  The max images bytes we send per round has to leave room for the other msg
            //  overhead. Normally it would have to also include space for possible UTF-8
            //  expansion of chars to bytes, but we know this is base64 encoded.
            //
            const tCIDLib::TCard4 c4MaxChunkImgChars(kCQCWebSrvC::c4MaxWebsockMsgSz - 4192);

            //
            //  Optimize if the whole thing can be sent in one chunk since we don't have
            //  to copy the image data, we can just send from the original.
            //
            const tCIDLib::TCard4 c4ImgChars = strEncoded.c4Length();
            tCIDLib::TCard4 c4MsgBytes;
            if (c4ImgChars < c4MaxChunkImgChars)
            {
                // Indicate this is the last block
                pmbufCur = wrtToUse.pmbufFormatImgDataFirst
                (
                    c4MsgBytes
                    , strFullPath
                    , c4SerialNum
                    , c4BinImgBytes
                    , bmpToSend.szBitmap()
                    , bIsPNG
                    , kCIDLib::True
                    , strEncoded
                );
                SendMsg(pmbufCur, c4MsgBytes);
            }
             else
            {
                tCIDLib::TCard4 c4SoFar = 0;
                tCIDLib::TCard4 c4AvailChars = c4ImgChars;
                tCIDLib::TCard4 c4ChunkChars = 0;
                TString strChunkStr;

                // We have to send at least one max img bytes chunk so fully allocate
                THeapBuf mbufChunk(c4MaxChunkImgChars, c4MaxChunkImgChars);

                while (c4AvailChars > 0)
                {
                    //
                    //  Get up to a chunk's worth of bytes. We have to leave room for the other
                    //  msg data values.
                    //
                    if (c4AvailChars >= c4MaxChunkImgChars)
                        c4ChunkChars = c4MaxChunkImgChars;
                    else
                        c4ChunkChars = c4AvailChars;

                    // Reduce the available chars now, so we can know if this is the last one
                    c4AvailChars -= c4ChunkChars;

                    strChunkStr.CopyInSubStr(strEncoded, c4SoFar, c4ChunkChars);
                    if (c4SoFar)
                    {
                        //
                        //  It's not the first chunk. If available bytes is zero, then it
                        //  is the last block for this image.
                        //
                        pmbufCur = wrtToUse.pmbufFormatImgDataNext
                        (
                            c4MsgBytes, strFullPath, c4AvailChars == 0, strChunkStr
                        );
                    }
                     else
                    {
                        // It's the first chunk
                        pmbufCur = wrtToUse.pmbufFormatImgDataFirst
                        (
                            c4MsgBytes
                            , strFullPath
                            , c4SerialNum
                            , c4BinImgBytes
                            , bmpToSend.szBitmap()
                            , bIsPNG
                            , kCIDLib::False
                            , strChunkStr
                        );
                    }

                    SendMsg(pmbufCur, c4MsgBytes);

                    // Move our working index forward by this chunk size
                    c4SoFar += c4ChunkChars;
                }
            }

            //
            //  Assume at this point that he has this image in his cache. He will send
            //  us a message that he added it, but we could see this image drawn a number
            //  of times before that got to us.
            //
            m_rimapClient.bAddUpdate(strFullPath, c4SerialNum);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            //
            //  Fall through and at least still send the message. The client won't have the
            //  image.
            //
        }
    }

    // Now send the original msg that references this image
    SendMsg(janBuf.pobjOrphan(), c4Size);
}

//
//  This one ultimately gets called for all RIVA messages. Our other helpers here will
//  call this one. Here we can do any stuff we need to do based on number of messages sent
//  or tracking the time of the last message sent or anything like that.
//
tCIDLib::TVoid
TWebSockRIVAThread::SendMsg(TMemBuf* const pmbufToAdopt, const tCIDLib::TCard4 c4Size)
{
    // Pass it on to the web socket parent class to be queued up for transmission
    QueueTextMsg(pmbufToAdopt, c4Size);
}


// Send a message to the client
tCIDLib::TVoid
TWebSockRIVAThread::SetRemWidgetVis(const   TString&                    strUID
                                    , const tCQCIntfEng::ERIVAWTypes    eType
                                    , const tCIDLib::TBoolean           bNewState)
{
    TWebRIVATools wrtToUse;
    tCIDLib::TCard4 c4Size;
    TMemBuf* pmbufMsg = wrtToUse.pmbufFormatSetRemWidgetVis
    (
        c4Size, strUID, tWebRIVA::EWdgTypes(eType), bNewState
    );
    SendMsg(pmbufMsg, c4Size);
}

// Send a message to the client
tCIDLib::TVoid
TWebSockRIVAThread::SetRemWidgetURL(const   TString&                    strUID
                                    , const tCQCIntfEng::ERIVAWTypes    eType
                                    , const TString&                    strURL
                                    , const tCIDLib::TKVPList&          colParams)
{
    // Turn the parameters into a raw JSON object
    TString strParams;
    MakeRawObject(colParams, strParams);

    TWebRIVATools wrtToUse;
    tCIDLib::TCard4 c4Size;
    TMemBuf* pmbufMsg = wrtToUse.pmbufFormatSetRemWidgetURL
    (
        c4Size, strUID, tWebRIVA::EWdgTypes(eType), strURL, strParams
    );
    SendMsg(pmbufMsg, c4Size);
}


// ---------------------------------------------------------------------------
//  TWebSockRIVAThread: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Prep any data members for a new connection. Check any query parms that we support
//  in order to get set up any options for us. We don't yet trust the client, so we don't
//  do anything yet that requires trust.
//
//  Since these threads are reused, we must be sure that we initialize all data that
//  is per-connection.
//
//  To be extra safe, we make sure that the GUI thread got stopped, and try to stop it
//  if not.
//
tCIDLib::TCard4
TWebSockRIVAThread::c4WSInitialize( const   TURL&           urlReq
                                    ,       TString&        strRepText
                                    ,       TString&        strErrText)
{
    if (m_thrFauxGUIThread.bIsRunning())
    {
        try
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIStillRunning
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Internal
                , TInteger(tCIDLib::TInt4(m_eLastGUIEv))
            );
            m_bGUIBailOut = kCIDLib::True;
            m_thrFauxGUIThread.eWaitForDeath(5000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            return kCIDNet::c4HTTPStatus_SrvError;
        }
    }

    strRepText = L"OK";
    return kCIDNet::c4HTTPStatus_OK;
}


tCIDLib::TVoid TWebSockRIVAThread::CheckShutdownReq() const
{

}


//
//  We have a conneced client now and the client has gone through any secure socket
//  negotiations and such. So now we need to try to log in. They provide the login info
//  via URL parameters.
//
tCIDLib::TVoid TWebSockRIVAThread::Connected()
{
    // We have to have gotten name/password query parameters
    TString strUserName, strPassword;
    if (!bFindQParam(L"user", strUserName) || !bFindQParam(L"pw", strPassword))
    {
        // Send a failure back and shutdown
        SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_NoCreds);
        StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
        return;
    }

    // We can get a session name that we'll use in some log messages
    if (!bFindQParam(L"sessname", m_strSessionName))
        m_strSessionName = L"Unknown";

    //
    //  Default the optional flags that the clients can set to known defaults, so
    //  that we and the client always start in sync. He can send us messages to set
    //  these as desired after connection.
    //
    m_bEnableCaching = kCIDLib::True;
    m_bLogInMsgs = kCIDLib::False;
    m_bLogGUIEvents = kCIDLib::False;

    // They can provide the lat/long info
    m_f8ClientLat = kCQCWebRIVA::f8LocNotSet;
    m_f8ClientLong = kCQCWebRIVA::f8LocNotSet;

    // Reset stuff that is per-connection
    m_enctNextActive = 0;
    m_enctNextEvent = 0;
    m_enctNextVal = 0;

    // We got the values so try the actual login
    try
    {
        // Get a reference to the security server
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

        // And ask it to give us a challenge
        TCQCSecChallenge seccLogon;
        if (orbcSS->bLoginReq(strUserName, seccLogon))
        {
            // Lets do a hash of the user's password
            TMD5Hash mhashPW;
            TMessageDigest5 mdigTmp;
            mdigTmp.StartNew();
            mdigTmp.DigestStr(strPassword);
            mdigTmp.Complete(mhashPW);

            // And use that to validate the challenge
            seccLogon.Validate(mhashPW);

            // And send that back to get a security token, assuming its legal
            TCQCUserAccount uaccToFill;
            TCQCSecToken sectToFill;
            tCQCKit::ELoginRes eRes;
            if (orbcSS->bGetSToken(seccLogon, sectToFill, uaccToFill, eRes)
            &&  (eRes == tCQCKit::ELoginRes::Ok))
            {
                // Make sure this guy has a default template
                if (uaccToFill.strDefInterfaceName().bIsEmpty())
                {
                    SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_NoDefTmpl);
                    StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
                    return;
                }

               // It worked so store the info away
                m_cuctxClient.Set(uaccToFill, sectToFill);
                m_uaccClient = uaccToFill;

                //
                //  They can send environmental variables for this session. So search
                //  for those and set them if found.
                //
                TString strVarVal;
                TString strExpVal;
                TString strVarName(L"env");
                for (tCIDLib::TCard4 c4Index = 1; c4Index <= 9; c4Index++)
                {
                    strVarName.CapAt(3);
                    strVarName.AppendFormatted(c4Index);

                    if (bFindQParam(strVarName, strVarVal))
                    {
                        // Expand it out
                        TURL::ExpandTo
                        (
                            strVarVal
                            , strExpVal
                            , TURL::EExpTypes::Query
                            , tCIDLib::EAppendOver::Overwrite
                        );
                        m_cuctxClient.SetEnvRTVAt(c4Index - 1, strExpVal);
                    }
                }

                // Tell the client we logged in and provide user info
                SendLoginRes(kCIDLib::True, kCQCWSCMsgs::midWRIVA_LoginSucceeded);
            }
             else
            {
                SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_BadCreds);
                StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
                return;
            }
        }
         else
        {
            // It has to be an unknown name/password
            SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_BadCreds);
            StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
            return;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_LoginExcept);
        StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
        return;
    }


    catch(...)
    {
        SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_LoginExcept);
        StartShutdown(CQCWebSrvC_WebRIVAHandler::c2WSockErr_LoginFailed);
        return;
    }


    // Now we need to wait to get the session state from the client
    m_eCurState = EStates::WaitSessState;
}


//
//  Indicate we are done
//
tCIDLib::TVoid TWebSockRIVAThread::Disconnected()
{
    // Stop the 'GUI' thread
    if (m_thrFauxGUIThread.bIsRunning())
    {
        try
        {
            //
            //  We stop this guy by setting the GUI bailout flag, not via the
            //  usual means, because the it can actually have been called back
            //  into by other threads from the action engine. This will cause
            //  all nested calls to it to unwind.
            //
            m_bGUIBailOut = kCIDLib::True;
            m_thrFauxGUIThread.eWaitForDeath(10000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadNotStopped
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Term
                , TInteger(tCIDLib::TInt4(m_eLastGUIEv))
            );
        }

        catch(...)
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadNotStopped
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::Term
                , TInteger(tCIDLib::TInt4(m_eLastGUIEv))
            );
        }
    }

    // Clear the event queue which will shut down any actions
    ClearGUIEventQ();

    // Clean up the view if we created it
    if (m_pcivTarget)
    {
        try
        {
            delete m_pcivTarget;
            m_pcivTarget = nullptr;
        }

        catch(TError& errToCatch)
        {
            if (facCQCWebSrvC().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            m_pcivTarget = nullptr;
        }
    }

    // Clean up the graphics device if we created it
    if (m_cptrRemDev.pobjData())
    {
        try
        {
            m_cptrRemDev.DropRef();
        }

        catch(TError& errToCatch)
        {
            if (facCQCWebSrvC().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    // Make sure we don't leave credentials around
    m_cuctxClient.Reset();
}


//
//  We don't use this and never register any fields. We are hosting an interface engine
//  and it uses the polling engine directly.
//
tCIDLib::TVoid
TWebSockRIVAThread::FieldChanged(const  TString&
                                , const TString&
                                , const tCIDLib::TBoolean
                                , const TCQCFldValue&)
{
}


tCIDLib::TVoid TWebSockRIVAThread::Idle()
{
    // If not in ready state, do nothing
    if (!bIsReady())
        return;

    //
    //  Check the time against our next update time stamps and post GUI events for any
    //  that are ready.
    //
    try
    {
        const tCIDLib::TEncodedTime enctNow = TTime::enctNow();
        if (enctNow >= m_enctNextActive)
        {
            m_enctNextActive = enctNow + CQCWebSrvC_WebRIVAHandler::enctActive;
            if (!m_c4TimerSuspend)
            {
                m_colGUIEvQ.bPutIfNew
                (
                    new TGUIEvent(EGUIEvTypes::ActiveUpdate), TGUIEvent::bCompEvent
                );
            }
        }

        if (enctNow >= m_enctNextEvent)
        {
            m_enctNextEvent = enctNow + CQCWebSrvC_WebRIVAHandler::enctEvent;
            if (!m_c4TimerSuspend)
            {
                m_colGUIEvQ.bPutIfNew
                (
                    new TGUIEvent(EGUIEvTypes::EventUpdate), TGUIEvent::bCompEvent
                );
            }
        }

        if (enctNow >= m_enctNextVal)
        {
            m_enctNextVal = enctNow + CQCWebSrvC_WebRIVAHandler::enctValue;
            if (!m_c4TimerSuspend)
            {
                m_colGUIEvQ.bPutIfNew
                (
                    new TGUIEvent(EGUIEvTypes::ValueUpdate), TGUIEvent::bCompEvent
                );
            }
        }

        //
        //  And we want to give the view a chance to timeout events
        //  for any popups that enable timeouts.
        //
        if (enctNow >= m_enctNextTOCheck)
        {
            m_enctNextTOCheck = enctNow + CQCWebSrvC_WebRIVAHandler::enctTOCheck;
            m_colGUIEvQ.bPutIfNew
            (
                new TGUIEvent(EGUIEvTypes::CheckTimeout), TGUIEvent::bCompEvent
            );
        }
    }

    catch(TError& errToCatch)
    {
        if (facCQCWebSrvC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}



tCIDLib::TVoid TWebSockRIVAThread::ProcessMsg(const TString& strMsg)
{
    // Parse out the message as a JSON object
    try
    {
        TTextStringInStream strmSrc(&strMsg);
        TJSONValue* pjprsnRep = m_jprsIn.pjprsnParse(strmSrc);

        // The reply is actually an object so cast it
        TJSONObject* pjprsnMsg = static_cast<TJSONObject*>(pjprsnRep);

        // Get the opcode out so we know what the rest is
        const tWebRIVA::EOpCodes eOpCode = tWebRIVA::EOpCodes
        (
            pjprsnMsg->strValByName(kWebRIVA::strAttr_OpCode).i4Val()
        );

        TWebRIVATools wrtToUse;

        //
        //  If in wait image map, we have to see an image map. Anything else is
        //  an error. The client has to send this immediately upon connection. If
        //  so, we can store that info away and then load the initial template and
        //  complete the connection process.
        //
        //  If in Ready mode, we can process other types of messages.
        //
        //  We allow proxy log server requests from the client event before the
        //  image map, to help debug startup issues.
        //
        if (eOpCode == tWebRIVA::EOpCodes::LogMsg)
        {
            // The client wants us to log a message on his behalf
            TString strMsg;
            tCIDLib::TBoolean bError;
            wrtToUse.ExtractLogMsg(*pjprsnMsg, strMsg, bError);
            facCQCWebSrvC().LogMsg
            (
                L"RIVA Client"
                , 0
                , kCQCWSCMsgs::midWRIVA_ClientMsg
                , bError ? tCIDLib::ESeverities::Failed : tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strSessionName
                , strMsg
            );
        }
         else if (m_eCurState == EStates::WaitSessState)
        {
            if (eOpCode == tWebRIVA::EOpCodes::SessionState)
            {
                CompleteConnection(wrtToUse, *pjprsnMsg);

                // We can move to ready state now
                m_eCurState = EStates::Ready;
            }
             else
            {
                // Can't be right, so drop the connection
                SendLoginRes(kCIDLib::False, kCQCWSCErrs::errcWRIVA_NoSessState);
                StartShutdown(kCQCWebSrvC::c2WSockErr_Proto);
            }
        }
         else
        {
            switch(eOpCode)
            {
                case tWebRIVA::EOpCodes::CancelInput :
                {
                    TGUIEvent* pevToQ(new TGUIEvent(EGUIEvTypes::CancelInput));
                    m_colGUIEvQ.Add(pevToQ);
                    if (m_bLogInMsgs)
                        LogStatusMsg(L"Got cancel input", CID_LINE);
                    break;
                }

                case tWebRIVA::EOpCodes::Move :
                {
                    TPoint pntAt;
                    wrtToUse.ExtractMove(*pjprsnMsg, pntAt);
                    TGUIEvent* pevToQ(new TGUIEvent(EGUIEvTypes::Move));
                    pevToQ->m_i4Val1 = pntAt.i4X();
                    pevToQ->m_i4Val2 = pntAt.i4Y();
                    m_colGUIEvQ.Add(pevToQ);
                    break;
                }

                case tWebRIVA::EOpCodes::Ping :
                {
                    //
                    //  Just eat it. This will update the last msg time stamp which
                    //  is all it needs to do.
                    //
                    break;
                }

                case tWebRIVA::EOpCodes::Press :
                {
                    TPoint pntAt;
                    wrtToUse.ExtractPress(*pjprsnMsg, pntAt);

                    TGUIEvent* pevToQ(new TGUIEvent(EGUIEvTypes::Press));
                    pevToQ->m_i4Val1 = pntAt.i4X();
                    pevToQ->m_i4Val2 = pntAt.i4Y();
                    m_colGUIEvQ.Add(pevToQ);
                    if (m_bLogInMsgs)
                    {
                        TString strMsg(L"Got press msg at %(1)", pntAt);
                        LogStatusMsg(strMsg, CID_LINE);
                    }
                    break;
                }

                case tWebRIVA::EOpCodes::Release :
                {
                    TPoint pntAt;
                    wrtToUse.ExtractRelease(*pjprsnMsg, pntAt);
                    TGUIEvent* pevToQ(new TGUIEvent(EGUIEvTypes::Release));
                    pevToQ->m_i4Val1 = pntAt.i4X();
                    pevToQ->m_i4Val2 = pntAt.i4Y();
                    m_colGUIEvQ.Add(pevToQ);
                    if (m_bLogInMsgs)
                    {
                        TString strMsg(L"Got release msg at %(1)", pntAt);
                        LogStatusMsg(strMsg, CID_LINE);
                    }
                    break;
                }

                case tWebRIVA::EOpCodes::SetServerFlags :
                {
                    tCIDLib::TCard4 c4Flags, c4Mask;
                    wrtToUse.ExtractSetServerFlags(*pjprsnMsg, c4Flags, c4Mask);
                    StoreServerFlags(c4Flags, c4Mask);
                    break;
                }

                case tWebRIVA::EOpCodes::SetVisState :
                {
                    //
                    //  The client has changed fgn/bgn state. Post this to the GUI
                    //  thread to handler.
                    //
                    tCIDLib::TBoolean bState;
                    wrtToUse.ExtractSetVisState(*pjprsnMsg, bState);
                    TGUIEvent* pevToQ(new TGUIEvent(EGUIEvTypes::SetVisState));
                    pevToQ->m_c4Val1 = bState ? 1 : 0;
                    m_colGUIEvQ.Add(pevToQ);
                    break;
                }

                default :
                    if (m_bLogInMsgs)
                    {
                        TString strMsg(L"Unknown RIVA message: ", 10);
                        strMsg.AppendFormatted(TInteger(tCIDLib::i4EnumOrd(eOpCode)));
                        LogStatusMsg(strMsg, CID_LINE);
                    }
                    break;
            };
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }
}


tCIDLib::TVoid TWebSockRIVAThread::WSTerminate()
{
}


// ---------------------------------------------------------------------------
//  TWebSockRIVAThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We clean up any entries in the GUI event queue. Any action threads
//  should have already been stopped.
//
tCIDLib::TVoid TWebSockRIVAThread::ClearGUIEventQ()
{
    while (!m_colGUIEvQ.bIsEmpty())
    {
        try
        {
            TGUIEvent* pgevCur = m_colGUIEvQ.pobjGetNext(0, kCIDLib::False);
            if (pgevCur->m_pevNotify)
            {
                pgevCur->m_piceInfo->eRes = tCQCKit::ECmdRes::Stop;
                pgevCur->m_pevNotify->Trigger();
            }
            delete pgevCur;
        }

        catch(...)
        {
            if (facCQCWebSrvC().bLogFailures())
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCErrs::errcWRIVA_CleanupErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , TString(L"GUI event")
                    , m_strSessionName
                );
            }
        }
    }
}


//
//  This is called after the client connects, and we get the expected session state
//  msg. At that point we can continue with the connection process. We need to set
//  our image map to match his and store the passed server flags.
//
//  Then we can load his default template, which will start us spitting out messages to
//  him.
//
//  We wil also be told if he is in a background tab. If so, we will set the background
//  state flag, and that will suppress graphics/images msgs being sent until he comes
//  forward. This can happen if we lose connection while he is in the background and he
//  then reconnects.
//
tCIDLib::TVoid
TWebSockRIVAThread::CompleteConnection(TWebRIVATools& wrtToUse, const TJSONObject& jprsnMsg)
{
    //
    //  First process the received image map message. If this fails, we'll log something
    //  but we won't give up. We'll just reset our map to empty and resend the images that
    //  are necessary.
    //
    try
    {
        m_rimapClient.Reset();

        // There is an array named imgList. We need to get that and iterate its elements
        const TJSONArray& jprsnList = *jprsnMsg.pjprsnFindArray(kWebRIVA::strAttr_List);
        const tCIDLib::TCard4 c4Count = jprsnList.c4ValCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TJSONObject& jprsnImg = *jprsnList.pjprsnObjAt(c4Index);
            m_rimapClient.bAddUpdate
            (
                jprsnImg.strValByName(kWebRIVA::strAttr_Path)
                , jprsnImg.c4FindVal(kWebRIVA::strAttr_SerialNum)
            );
        }

        // Get the server flags and mask and set those flags appropriately
        const tCIDLib::TCard4 c4Flags = jprsnMsg.c4FindVal(kWebRIVA::strAttr_ToSet);
        const tCIDLib::TCard4 c4Mask = jprsnMsg.c4FindVal(kWebRIVA::strAttr_Mask);
        StoreServerFlags(c4Flags, c4Mask);

        //
        //  Store the visibility state (fgn/bgn tab state), which we piggyback on the
        //  flags in this msg.
        //
        m_bClientVisState = (c4Flags & kWebRIVA::c4SrvFlag_InBgnTab) == 0;
    }

    catch(TError& errToCatch)
    {
        // Oh well, log it and reset the map
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        m_rimapClient.Reset();
    }

    // OK, now we can try to load the default template for our user
    try
    {
        tCIDLib::TFloat8    f8SrvLat;
        tCIDLib::TFloat8    f8SrvLong;
        TCQCIntfTemplate    iwdgInit;
        TDataSrvClient      dsclTmpl;

        //
        //  Download the template and stream it in. For this initial query we pass
        //  in the caching enablement flag we got. We haven't created the view yet
        //  so we can't set the flag on it until we do.
        //
        facCQCIntfEng().QueryTemplate
        (
            m_uaccClient.strDefInterfaceName()
            , dsclTmpl
            , iwdgInit
            , m_cuctxClient
            , m_bEnableCaching
        );

        // Get the server side location info
        facCQCKit().bGetLocationInfo
        (
            tCIDLib::ECSSides::Server, f8SrvLat, f8SrvLong, kCIDLib::True, m_cuctxClient.sectUser()
        );


        // For us the 'screen res' is the size of the base template
        m_szDevRes = iwdgInit.areaRelative().szArea();

        //
        //  Now create our shadow device, the size of the base template in our
        //  case.
        //
        TWebRIVAGraphDev* pgdevNew = new TWebRIVAGraphDev(m_szDevRes, this);
        m_cptrRemDev.SetPointer(pgdevNew);

        //
        //  And create our view. He gets a copy of our remote graphics device, but we
        //  still own it since the ref count will stay non-zero because of our counted
        //  pointer that we store it in.
        //
        m_pcivTarget = new TCQCWebRIVAView
        (
            iwdgInit
            , L"CQC Remote Viewer"
            , f8SrvLat
            , f8SrvLong
            , m_szDevRes
            , m_cptrRemDev
            , pgdevNew
            , this
            , this
            , m_cuctxClient
            , m_f8ClientLat
            , m_f8ClientLong
        );

        // Start up the faux GUI thread, making sure the bailout flag is cleared first
        m_bGUIBailOut = kCIDLib::False;
        m_thrFauxGUIThread.Start();

        //
        //  Set the caching option on it. DO this before we initialize it below, since
        //  that will kick off any recursive loading of overlays, so we need to have
        //  this set before that happens.
        //
        m_pcivTarget->bEnableTmplCaching(m_bEnableCaching);

        // And initialize it
        tCQCIntfEng::TErrList colErrs;
        if (!m_pcivTarget->bInitialize(dsclTmpl, colErrs))
        {
            if (facCQCWebSrvC().bLogFailures())
            {
                facCQCWebSrvC().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCWSCErrs::errcWRIVA_FailedTmplInit
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::AppStatus
                    , m_uaccClient.strDefInterfaceName()
                    , m_strSessionName
                );
            }

            #if 0
            facCQCRemVSrv.SendStatusReply
            (
                *m_psockTarget
                , m_c2NextOutSeqId
                , kCIDLib::False
                , L"The template could not be initialized"
            );
            #endif

            // Reset the the template so they end up with an empty window
            m_pcivTarget->iwdgBaseTmpl().ResetWidget();
        }

        m_pcivTarget->NewSize(m_szDevRes);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }
}


//
//  This is the thread that simulates the GUI thread. It's started up by
//  this (worker) thread after initialization is done. It simulates the
//  GUI thread that would normally handle user input, and that the interface
//  engine would use to implement modals loops and whatnot.
//
//  So the worker thread is the 'background' thread, and this faux GUI
//  thread is the 'foreground' thread, though in this case the background
//  thread is really running things (acting like the OS in the real GUI
//  scenario.)
//
tCIDLib::EExitCodes
TWebSockRIVAThread::eFauxGUIThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the calling thread go
    thrThis.Sync();

    // TEmp debug code
    m_eLastGUIEv = EGUIEvTypes::None;

    try
    {
        //
        //  Now call the service loop. We'll stay there until the client
        //  drops the connection (in which case the worker thread that owns
        //  us will post us a message to that effect and we'll unwind and
        //  exit.
        //
        tCIDLib::TBoolean bBreakFlag = kCIDLib::False;
        FauxGUILoop(bBreakFlag);
    }

    catch(TError& errToCatch)
    {
        if (facCQCWebSrvC().bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        if (facCQCWebSrvC().bLogFailures())
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadError
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_strSessionName
            );
        }
    }

    catch(...)
    {
        if (facCQCWebSrvC().bLogFailures())
        {
            facCQCWebSrvC().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCWSCErrs::errcWRIVA_GUIThreadError
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , m_strSessionName
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  This is called when our faux GUI thread gets a command dispatch from the
//  action engine thread. We just moved it out here to keep that method from
//  getting bloated.
//
//  The action engine thread will dispatch us commands if they are either
//  non-standard targets (which means a widget) or standard targets that are
//  marked as GUI targets (such as the interface viewer.)
//
//
//  >>>>>>>>
//  WE CANNOT allow any exception to propogate here, since it will propogate
//  in this thread, not back to the waiting action thread.
//  <<<<<<<<
//
tCIDLib::TVoid TWebSockRIVAThread::HandleGUIActDispatch(const TGUIEvent& gevCur)
{
    tCQCIntfEng::TIntfCmdEv& iceToDo = *gevCur.m_piceInfo;
    iceToDo.eRes = tCQCKit::ECmdRes::Ok;
    try
    {
        // Get a ref ot he command we are to do and the target template
        const TCQCCmdCfg& ccfgToDo = *iceToDo.pccfgToDo;
        TCQCIntfTemplate& iwdgTmpl = m_pcivTarget->iwdgAt(iceToDo.c4HandlerId);

        // Find the target widget by the id in the command
        TCQCIntfWidget* piwdgTar(iwdgTmpl.piwdgFindByUID(ccfgToDo.c4TargetId(), kCIDLib::True));

        // If we didn't find it, it could be the view itself
        MCQCCmdTarIntf* pmctarCur = nullptr;
        if (piwdgTar)
            pmctarCur = dynamic_cast<MCQCCmdTarIntf*>(piwdgTar);
         else if (m_pcivTarget->c4UniqueId() == ccfgToDo.c4TargetId())
            pmctarCur = m_pcivTarget;

        if (pmctarCur)
        {
            iceToDo.eRes = pmctarCur->eDoCmd
            (
                ccfgToDo
                , iceToDo.c4StepInd
                , *iceToDo.pstrUserId
                , *iceToDo.pstrEventId
                , *iceToDo.pctarGlobals
                , iceToDo.bResult
                , *iceToDo.pacteOwner
            );
        }
         else
        {
            iceToDo.bTarNotFound = kCIDLib::True;
        }
    }

    catch(const TError& errToCatch)
    {
        iceToDo.eRes = tCQCKit::ECmdRes::Except;
        iceToDo.errFailure = errToCatch;
    }

    catch(...)
    {
        iceToDo.eRes = tCQCKit::ECmdRes::UnknownExcept;
    }

    //
    //  If we got out because of a bailout, tell the action
    //  thread to stop processing and exit.
    //
    if (m_bGUIBailOut)
        iceToDo.eRes = tCQCKit::ECmdRes::Stop;

    // Now wake up the background action thread what's waiting
    gevCur.m_pevNotify->Trigger();
}


//
//  A convenience method to log status messages. We automatically include the file name
//  and the session name.
//
tCIDLib::TVoid
TWebSockRIVAThread::LogStatusMsg(const TString& strMsg, const tCIDLib::TCard4 c4Line)
{
    TString strToLog(L"[", strMsg.c4Length() + m_strSessionName.c4Length() + 16);
    strToLog.Append(m_strSessionName);
    strToLog.Append(L"] ");
    strToLog.Append(strMsg);

    facCQCWebSrvC().LogMsg(CID_FILE, c4Line, strToLog, tCIDLib::ESeverities::Status);
}

tCIDLib::TVoid
TWebSockRIVAThread::LogStatusMsg(const tCIDLib::TCh* const pszMsg, const tCIDLib::TCard4 c4Line)
{
    TString strToLog(L"[", TRawStr::c4StrLen(pszMsg) + m_strSessionName.c4Length() + 16);
    strToLog.Append(m_strSessionName);
    strToLog.Append(L"] ");
    strToLog.Append(pszMsg);

    facCQCWebSrvC().LogMsg(CID_FILE, c4Line, strToLog, tCIDLib::ESeverities::Status);
}


// Formats a list of key/value string pairs into a JSON object
tCIDLib::TVoid
TWebSockRIVAThread::MakeRawObject(const tCIDLib::TKVPList& colValues, TString& strToFill)
{
    strToFill.Clear();
    strToFill += kCIDLib::chOpenBrace;
    const tCIDLib::TCard4 c4Count = colValues.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index)
            strToFill += kCIDLib::chComma;

        strToFill += kCIDLib::chQuotation;
        strToFill += colValues[c4Index].strKey();
        strToFill += kCIDLib::chQuotation;

        strToFill += kCIDLib::chColon;

        strToFill += kCIDLib::chQuotation;
        strToFill += colValues[c4Index].strValue();
        strToFill += kCIDLib::chQuotation;
    }
    strToFill += kCIDLib::chCloseBrace;
}


//
//  This is called to send a login result msg to the client. The caller provides the success
//  or failure indicator and a message to send with it. If successful, we also send the
//  login name and user role back.
//
tCIDLib::TVoid
TWebSockRIVAThread::SendLoginRes(const tCIDLib::TBoolean bRes, tCIDLib::TMsgId midText)
{
    //
    //  To avoid any sync requirements, we just create our own tools object. The overhead
    //  is not high for a very seldom sent message like this.
    //
    TWebRIVATools wrtToUse;

    TString strName;
    tWebRIVA::EUserRoles eRole = tWebRIVA::EUserRoles::Limited;

    // If it was successful, get the real name and user role
    if (bRes)
    {
        // The user role values are the same between ours and the protocol's
        eRole = tWebRIVA::EUserRoles(m_uaccClient.eRole());
        strName = m_uaccClient.strLoginName();
    }

    tCIDLib::TCard4 c4Size;
    TMemBuf* pmbufMsg = wrtToUse.pmbufFormatLoginRes
    (
        c4Size, bRes, facCQCWebSrvC().strMsg(midText), strName, eRole
    );
    SendMsg(pmbufMsg, c4Size);
}


// Store away the passed server flags
tCIDLib::TVoid
TWebSockRIVAThread::StoreServerFlags(const  tCIDLib::TCard4 c4Flags
                                    , const tCIDLib::TCard4 c4Mask)
{
    if (c4Mask & kWebRIVA::c4SrvFlag_NoCache)
    {
        // Note the reverse meanings here!
        m_bEnableCaching = (c4Flags & kWebRIVA::c4SrvFlag_NoCache) == 0;

        // If the view is already created, set it on that
        if (m_pcivTarget)
            m_pcivTarget->bEnableTmplCaching(m_bEnableCaching);
    }

    if (c4Mask & kWebRIVA::c4SrvFlag_LogGUIEvents)
        m_bLogGUIEvents = (c4Flags & kWebRIVA::c4SrvFlag_LogGUIEvents) != 0;

    if (c4Mask & kWebRIVA::c4SrvFlag_LogSrvMsgs)
        m_bLogInMsgs = (c4Flags & kWebRIVA::c4SrvFlag_LogSrvMsgs) != 0;
}
