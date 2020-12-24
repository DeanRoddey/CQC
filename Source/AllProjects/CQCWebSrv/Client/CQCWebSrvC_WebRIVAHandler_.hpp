//
// FILE NAME: CQCWebSrvC_WebRIVAHandler.hpp
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
//  This is the header for the class that implements a derivative of the base Websockets
//  handler class. This one is dedicated to a browser based RIVA client.
//
//  We basically just create a CQCWebRIVAEng object (provided by the CQCWebRIVA facility)
//  and pass on callbacks to it. We give it a pointer to ourself (via the base web sock
//  thread class) so that it can send messages.
//
//  We also give it a pointer to the field polling engine that our facility class maintains
//  for general use. The IV engine uses it to do it's field polling directly, so we don't
//  register any fields.
//
//  We implement a mixin provided by the WebRIVA facility, so that it has an interface by
//  which it can access this worker thread. Mostly it's just to send messages.
//
//  And we implement the CIDCtrls gesture handler mixin, so that we can be passed to a
//  our RIVA'ized gesture handler and receive gesture callbacks.
//
//  And we implement the IV engine's application callback handler, since we represent a
//  virtual IV and need to handle these callbacks on the behalf of the client.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

class TWebRIVATools;

// ---------------------------------------------------------------------------
//   CLASS: TWebSockRIVAThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWebSockRIVAThread :  public TWebsockThread
                            , public TWebRIVAWIntf
                            , public MCIDGestTarget
                            , public MCQCIntfAppHandler
{
    public :
        // --------------------------------------------------------------------
        // Constructors and Destructors
        // --------------------------------------------------------------------
        TWebSockRIVAThread
        (
            const   tCIDLib::TCard1         c1ThreadId
        );

        TWebSockRIVAThread(const TWebSockRIVAThread&) = delete;
        TWebSockRIVAThread(TWebSockRIVAThread&&) = delete;

        ~TWebSockRIVAThread();


        // --------------------------------------------------------------------
        //  Public operators
        // --------------------------------------------------------------------
        TWebSockRIVAThread& operator=(const TWebSockRIVAThread&) = delete;
        TWebSockRIVAThread& operator=(TWebSockRIVAThread&&) = delete;


        // --------------------------------------------------------------------
        //  Public, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid AsyncCallback
        (
            const   tCQCIntfEng::EAsyncCmds eCmd
            ,       TCQCIntfView&           civSender
        )   override;

        tCIDLib::TVoid AsyncDataCallback
        (
                    TCQCIntfADCB* const     padcbInfo
        )   override;

        tCIDLib::TBoolean bProcessGestEv
        (
            const   tCIDCtrls::EGestEvs     eEv
            , const TPoint&                 pntStart
            , const TPoint&                 pntAt
            , const TPoint&                 pntDelta
            , const tCIDLib::TBoolean       bTwoFingers
        )   override;

        tCIDLib::TVoid CreateRemWidget
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const TArea&                  areaAt
            , const tCIDLib::TKVPList&      colParams
            , const tCIDLib::TBoolean       bInitVis
        )   override;

        tCIDLib::TVoid DestroyRemWidget
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
        )   override;

        tCIDLib::TVoid GestClick
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TVoid DismissBlanker
        (
                    TCQCIntfView&           civSender
        )   override;

        tCIDLib::TVoid DispatchActEvent
        (
                    tCQCIntfEng::TIntfCmdEv& iceToDo
        )   override;

        tCIDLib::TVoid DoSpecialAction
        (
            const   tCQCIntfEng::ESpecActs  eAct
            , const tCIDLib::TBoolean       bAsync
            ,       TCQCIntfView&           civSender
        )   override;

        tCQCIntfEng::EAppFlags eAppFlags() const override;

        tCIDLib::TVoid FauxGUILoop
        (
                    tCIDLib::TBoolean&      bBreakFlag
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

        tCIDLib::TVoid NewTemplate
        (
            const   TCQCIntfTemplate&       iwdgNew
            ,       TCQCIntfView&           civSender
            , const tCIDLib::TCard4         c4StackPos
        )   override;

        MCQCCmdTracer* pmcmdtTrace() override;

        tCIDLib::TVoid PauseTimers
        (
            const   tCIDLib::TBoolean       bPauseState
        )   override;

        tCIDLib::TVoid PerGestOpts
        (
            const   TPoint&                 pntAt
            , const tCIDLib::EDirs          eDir
            , const tCIDLib::TBoolean       bTwoFingers
            ,       tCIDCtrls::EGestOpts&   eOpts
            ,       tCIDLib::TFloat4&       f4VScale
        )   override;

        tCIDLib::TVoid SendGraphicsMsg
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   override;

        tCIDLib::TVoid SendImgMsg
        (
            const   TString&                strFullPath
            , const TBitmap&                bmpToSend
            ,       TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   override;

        tCIDLib::TVoid SendMsg
        (
                    TMemBuf* const          pmbufToAdopt
            , const tCIDLib::TCard4         c4Size
        )   override;

        tCIDLib::TVoid SetRemWidgetVis
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const tCIDLib::TBoolean       bState
        )   override;

        tCIDLib::TVoid SetRemWidgetURL
        (
            const   TString&                strUID
            , const tCQCIntfEng::ERIVAWTypes eType
            , const TString&                strURL
            , const tCIDLib::TKVPList&      colParams
        )   override;


    protected :
        // --------------------------------------------------------------------
        //  Protected, inherited methods
        // --------------------------------------------------------------------
        tCIDLib::TCard4 c4WSInitialize
        (
            const   TURL&                   urlReq
            ,       TString&                strRepText
            ,       TString&                strErrText
        )   final;

        tCIDLib::TVoid CheckShutdownReq() const final;

        tCIDLib::TVoid Connected() final;

        tCIDLib::TVoid Disconnected() final;

        tCIDLib::TVoid FieldChanged
        (
            const   TString&                strMon
            , const TString&                strField
            , const tCIDLib::TBoolean       bGoodValue
            , const TCQCFldValue&           fvValue
        )   final;

        tCIDLib::TVoid Idle() final;

        tCIDLib::TVoid ProcessMsg
        (
            const   TString&                strMsg
        )   final;

        tCIDLib::TVoid WSTerminate() final;


    private :
        // --------------------------------------------------------------------
        //  Private class types
        // --------------------------------------------------------------------
        enum class EGUIEvTypes
        {
            None
            , ActiveUpdate
            , AsyncDCB
            , CancelInput
            , CheckTimeout
            , DispatchActEv
            , ExitLoop
            , EventUpdate
            , Move
            , HotKey
            , Press
            , Redraw
            , Release
            , SizeChange
            , SetVisState
            , ValueUpdate
        };

        class TGUIEvent
        {
            public :
                static tCIDLib::TBoolean bCompEvent
                (
                    const   TGUIEvent&      ev1
                    , const TGUIEvent&      ev2
                );

                TGUIEvent();

                TGUIEvent
                (
                    TCQCIntfADCB* const     padcbInfo
                );

                TGUIEvent
                (
                    const   EGUIEvTypes     eType
                );

                TGUIEvent(const TGUIEvent&) = delete;
                TGUIEvent(TGUIEvent&&) = delete;

                ~TGUIEvent();

                TGUIEvent& operator=(const TGUIEvent&) = delete;
                TGUIEvent& operator=(TGUIEvent&&) = delete;

                tCIDLib::TCard4             m_c4Val1;
                tCIDLib::TCard4             m_c4Val2;
                EGUIEvTypes                 m_eType;
                tCIDLib::TFloat8            m_f8Val;
                tCIDLib::TInt4              m_i4Val1;
                tCIDLib::TInt4              m_i4Val2;
                tCIDLib::TInt4              m_i4Val3;
                tCIDLib::TInt4              m_i4Val4;
                tCQCIntfEng::TIntfCmdEv*    m_piceInfo;
                TEvent*                     m_pevNotify;
                TCQCIntfADCB*               m_padcbInfo;
        };

        using TEvQueue = TRefQueue<TGUIEvent>;

        //
        //  We have some basic statefulness upon connection. There's no failed state
        //  because any failure causes us to terminate the connection.
        //
        enum class EStates
        {
            WaitSessState
            , Ready
        };


        // --------------------------------------------------------------------
        //  Private, non-virtual methods
        // --------------------------------------------------------------------
        tCIDLib::TVoid ClearGUIEventQ();

        tCIDLib::TVoid CompleteConnection
        (
                    TWebRIVATools&          wrtToUse
            , const TJSONObject&            jprsnMsg
        );

        tCIDLib::EExitCodes eFauxGUIThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid HandleGUIActDispatch
        (
            const   TGUIEvent&              gevCur
        );

        tCIDLib::TVoid LogStatusMsg
        (
            const   tCIDLib::TCh* const     pszMsg
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid LogStatusMsg
        (
            const   TString&                strMsg
            , const tCIDLib::TCard4         c4Line
        );

        tCIDLib::TVoid MakeRawObject
        (
            const   tCIDLib::TKVPList&      colValues
            ,       TString&                strToFill
        );

        tCIDLib::TVoid SendLoginRes
        (
            const   tCIDLib::TBoolean       bRes
            ,       tCIDLib::TMsgId         midText
        );

        tCIDLib::TVoid StoreServerFlags
        (
            const   tCIDLib::TCard4         c4Flags
            , const tCIDLib::TCard4         c4Mask
        );


        // --------------------------------------------------------------------
        //  Private data members
        //
        //  m_bClientVisState
        //      The client tells us when he is visible (fgn tab) or not (bgn tab) and
        //      we stop dispatching graphics and img commands while he is not visible.
        //
        //  m_bEnableCaching
        //      The client can tell us whether we should use the template cache or not. For
        //      debugging they often don't want to. Defaults to true if not explicitly set.
        //
        //  m_bGUIBailOut
        //      See the comments on the FauxGUILoop method. This is a means to force
        //      unwinding of the multiple nested calls back into the GUI loop.
        //
        //  m_bLogInMsgs
        //      The client can set this via a logsrvmsgs=1 query parameter. We log incoming
        //      messages at the point where the base class calls us with them.
        //
        //  m_bLogGUIEvents
        //      The client can set this via a logguievs=1 query parameter. We log some events
        //      in the faux GUI thread when we process them.
        //
        //  m_bShutdownReq
        //      This is shut if we see a logoff message from the client. It tells us to
        //      bail out and shut down the session since the client is gracefully exiting.
        //
        //  m_c1ThreadId
        //      Each worker thread gets a unique id. That will become part of the session
        //      cookie that's given back to the client. It goes into the last byte of the
        //      cookie. This can be used later in calls to the image download thread
        //      (which has to get a cookie) to figure out which thread was being targeted.
        //
        //  m_c4TimerSuspend
        //      The interface engine (via the graphics device) can ask that the callbacks
        //      be stopped in certain cases. We provide a method to do this, and bump this
        //      counter up and down to provide nested support for this.
        //
        //  m_colGUIEvQ
        //      This is the queue we use to simulate a GUI, so that it's just a lot
        //      easier because things can work as they do in the the more usual window
        //      based scenario. The processing thread blocks on this queue waiting for
        //      stuff to show up to do.
        //
        //  m_cptrRemDev
        //      This is our remote viewer graphic device that we use to generate the
        //      outgoing graphical operations to the client. We can't create it until
        //      after the client logs in and we get resolution and bit depth info. We
        //      store it via a counted pointer so that we can hand it to the view that
        //      way. This way, he can store it that way, and that's the form that views
        //      hand out graphics devices anyway. So this way we insure it doesn't cleaned
        //      up behind our back, because our copy here keeps it alive until after we
        //      clean up the graphics device.
        //
        //  m_cuctxClient
        //      If we log on successfully, we store the user context here to be used in any
        //      operations that require one. We pass it on to our interface view which will
        //      in turn make it available to any IV operations that need one.
        //
        //  m_enctLastMsg
        //      We have to watch for connections that just disappear and time them out. So
        //      this is set each time we get a message from a client. Each time we don't
        //      get a msg, we check to see how long its been. If it's been beyond a period
        //      of time, we drop the connection.
        //
        //  m_enctNextXXX
        //      We have to provide the driving force for some periodic operations that, in
        //      the regular IV would be done with Windows timers. So we use these to know
        //      when it's time to make them happen, which we do in the idle callback.
        //
        //  m_eCurState
        //      We don't have much in the way of statefulness, but we do need to insure some
        //      steps happen upon connection, and some of that requires waiting for info
        //      from the client, so it has to be done statefully, we can't just block and
        //      wait.
        //
        //  m_f8ClientLat
        //  m_f8ClientLong
        //      The client can give us his lat/long, so that any geo calculations can work
        //      in terms of his location, not that of the server.
        //
        //  m_gesthInp
        //      We need a gesture handler for each client session. We implement the gesture
        //      target mixin, and pass ourself to this guy. He calls us with processed
        //      gestures.
        //
        //  m_jprsIn
        //      A JSON parser to handle incoming messages from the client.
        //
        //  m_pcivTarget
        //      This is our viewer target, which we animate via calls, to drive value updates,
        //      active updates, user input, etc... It will be created after connection from
        //      a client, and after we create the remoted graphical device (which we give to
        //      the view to use for any graphical output.)
        //
        //      Other than creation and cleanup, only the faux GUI faux thread ever accesses
        //      this object. So no sync is required here.
        //
        //  m_rimapClient
        //      The image map for our client. The client helps us keep this up to date with
        //      all of the repo images it has and their versions, so we know whether we need
        //      to send them referenced images or not.
        //
        //  m_strSessionName
        //      The client can send a session name in the URL, which we can use in RIVA level
        //      messages we log, to help with debugging. Typically it would be the host name
        //      or IP or some other unique id for that client.
        //
        //  m_szDevRes
        //      The device resolution we got from the client upon connection.
        //
        //  m_thrFauxGUIThread
        //      This is another thread, owned by this thread. This thread blocks on the
        //      m_colEvQ queue, waiting for events to process. Among them are simulated
        //      GUI timer events which it uses to drive the updates of the template. This
        //      guy does a little init, then calls FauxGUILoop() to emulate a the GUI msg
        //      loop.
        //
        //      This thread (the worker thread) processes the socket comm and just posts
        //      events to the event queue when something is of interest to this thread,
        //      and it posts the timer messages.
        //
        //  m_uaccClient
        //      We store the user account of the client upon successful login.
        // --------------------------------------------------------------------
        tCIDLib::TBoolean           m_bClientVisState;
        tCIDLib::TBoolean           m_bEnableCaching;
        tCIDLib::TBoolean           m_bGUIBailOut;
        tCIDLib::TBoolean           m_bLogGUIEvents;
        tCIDLib::TBoolean           m_bLogInMsgs;
        tCIDLib::TBoolean           m_bShutdownReq;
        tCIDLib::TCard4             m_c4TimerSuspend;
        tCIDLib::TCard1             m_c1ThreadId;
        TEvQueue                    m_colGUIEvQ;
        tCQCIntfEng::TGraphIntfDev  m_cptrRemDev;
        TCQCUserCtx                 m_cuctxClient;
        tCIDLib::TEncodedTime       m_enctLastMsg;
        tCIDLib::TEncodedTime       m_enctNextActive;
        tCIDLib::TEncodedTime       m_enctNextEvent;
        tCIDLib::TEncodedTime       m_enctNextVal;
        tCIDLib::TEncodedTime       m_enctNextTOCheck;
        EStates                     m_eCurState;
        tCIDLib::TFloat8            m_f8ClientLat;
        tCIDLib::TFloat8            m_f8ClientLong;
        TRIVAGestHandler            m_gesthInp;
        TJSONParser                 m_jprsIn;
        TCQCWebRIVAView*            m_pcivTarget;
        TRIVAImgMap                 m_rimapClient;
        TString                     m_strSessionName;
        TSize                       m_szDevRes;
        TThread                     m_thrFauxGUIThread;
        TCQCUserAccount             m_uaccClient;


        // Temp code to debug a problem. Remmeber last event being processed
        EGUIEvTypes                 m_eLastGUIEv;
};

#pragma CIDLIB_POPPACK
