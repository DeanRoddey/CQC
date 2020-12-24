//
// FILE NAME: CQCEventSrv_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/18/2004
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
//  This is the header for the facility class for the events server. We field the
//  incoming requests from the remote clients to manage the event lists (triggered
//  or scheduled), and we watch for scheduled events to become ready to run and
//  for incoming triggers to trigger a triggered event. We queue these up and let a
//  thread pool process them.
//
//  For triggered events we queue the incoming trigger object in addition to the
//  configured event to perform. To avoid replicating it for every triggered event
//  that it causes to run, we use a counted pointer on it. It'll get dropped when
//  the last thread that uses it lets it go.
//
//  We also support 'monitors', in which each one is given its own permanent thread
//  and they just get set up upon start up (or configuraton of a new one by a client)
//  and they run until we shut down or a client deletes one of them. These are used
//  to monitor changes in the system and react to them in a more complex way than
//  triggered events, since they can maintain context over time, and will never run
//  more than one instance at a time. These all derive from a common CML class that
//  proivdes the interface that we work through.
//
//  The triggered/scheduled events are defined in CQCKit. It also defines a simple
//  derivative of each which includes the path that they were loaded from. That's not
//  stored in the base class since those are written to disc and we'd constantly have
//  to be dealing with keeping the stored path in sync with the actual storage
//  location. It's really only needed by folks like us who need to have a list keyed
//  on the path.
//
//  The path is the type relative path, the same as used by the data server to read
//  and write them, so /User/xyz. Since we always know the type we are dealing with,
//  there is no ambiguity issue with using this relative path type.
//
//  Synchronization
//
//  This server is fairly complicated in terms of threading and synchronization. Leaving
//  aside monitors and looking at scheduled/triggered events, it is like this:
//
//      TriggerThread --
//                      |-> ActionQ --> Dispatcher --> [Worker thread pool]
//      Schedulehread --
//
//  So we have one thread that is always blocked on the pub/sub topic for incoming
//  event triggers. And we have one that maintains a time sorted list of scheduled
//  events and watches for time for the next one to run. These both drop events that
//  are ready to run into an action queue (made up of TEventQItem objects.)
//
//  The dispatcher thread is mostly always blocked on the action queue, waiting for
//  events to dispatch. When it gets one, it finds an appropriate thread to process
//  that event action and gives the action to it. The thread gets a copy of the
//  event action that it can modify as required.
//
//  Each worker thread has a queue of its own. When the dispatcher puts an event into
//  a thread's queue, if that thread is not already processing (see serialization below)
//  it will post an event that each worker thread blocks on, telling it that it has work
//  to do.
//
//  The worker thread will process that event, and check his queue again. If nothing
//  to process he will reset his event and state and go back to blocking on the event
//  until new work is available.
//
//  The dispatcher has an event also. If it cannot find a free thread, it will reset
//  this event and block on it. Every time a worker finishes and goes back to idle mode,
//  he will trigger this event. That will wake up the dispatcher and let him dispatch
//  his current event.
//
//  The two feeding threads and the dispatcher share the action queue. It is thread safe
//  and they both do simple add/get operations on it, which are atomic, so everything is
//  very simple there.
//
//  The dispatcher and the worker threads share a single mutex that they use to coordinate
//  between themselves and the dispatcher thread. This is to protect the worker threads'
//  work queue, their current active state, and the events used by the workers and the one
//  used by the dispatcher. Manipulation of those events must be coordinated to avoid
//  race conditions.
//
//  This is all fairly straightforward. It could be much simpler and used to be, but as of
//  5.3.928, support for serialization of triggered events was added, and that threw a
//  monkey wrench into the works.
//
//
//  Serialization
//
//  Triggered events can be marked as 'serialized'. In that case, we only run one of
//  that type at a time. This introduces complications. Free thread pool threads cannot
//  process any that are of a type already being processed. But we can't leave in them
//  in an input queue that the worker threads are watching, since they would just spin
//  continuously.
//
//  This required the introduction of the intermediate dispatcher thread discussed above,
//  and a separate per-worker queue for worker threads.
//
//  When the dispatcher pulls a triggered event off the action queue, he sees if it is
//  marked as serialized. If so, he searchs the threads for an active one that is
//  processing that event path. If he finds one, he just adds the event action to that
//  thread's queue. The thread will pull it out next and process it it.
//
//  If there is no previous instance being processed, then it will be treated normally and
//  given to any inactive thread.
//
//  This scheme allows us to both serialize triggered events, and still process triggers
//  basically in the order received. I.e. we don't push serialized ones out to a separate
//  queue that is completely separately processed. The only deviation from order of receipt
//  is that serialized ones that get queued up to wait for the previous instance can happen
//  after non-serialized ones that come in later, because the latter don't need to wait and
//  just be handed off to any available therad.
//
//
//  The action queue is of limited size. If we are getting stuff so fast that we cannot
//  get them run through the thread pool fast enough, then the two input threads will
//  start rejecting them because the action queue is full.
//
//
//  We derive from the standard CQC core server framework class. So mostly we are just
//  responding to callbacks from him. We set up our server side objects and get them
//  registered with the ORB and start up our helper threads, and they mind their own
//  business.
//
// CAVEATS/GOTCHAS:
//
//  1)  Note that, when it is time to run an event of either type, we give it to a
//      thread from a thread pool. It gets a copy of that event, so we don't have
//      to worry about any incoming requests to change the list of actions that might
//      change or remove any actions currently in progress. They will run till they
//      finish and then the copy will be thrown away.
//
// LOG:
//
//  $Log$
//

// This is an internal class but we have a pointer to it in the worker thread class
class TEventMacroEngine;


// ---------------------------------------------------------------------------
//   CLASS: TEventQItem
//  PREFIX: qitem
// ---------------------------------------------------------------------------
class TEventQItem
{
    public :
        // -------------------------------------------------------------------
        //  Class types
        // -------------------------------------------------------------------
        using TSrcPtr = TCntPtr<TCQCStdCmdSrc>;


        // -------------------------------------------------------------------
        //  Constructors and Destructore
        // -------------------------------------------------------------------
        TEventQItem();

        TEventQItem
        (
            const   TString&                strPath
            ,       TCQCTrgEventEx* const   pcsrcToAdopt
        );

        TEventQItem
        (
            const   TString&                strPath
            ,       TCQCSchEvent* const     pcsrcToAdopt
        );

        TEventQItem(const TEventQItem&) = default;
        TEventQItem(TEventQItem&&) = delete;

        ~TEventQItem() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TEventQItem& operator=(const TEventQItem&) = default;
        TEventQItem& operator=(TEventQItem&&) = default;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Reset();


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_bLoggable
        //  m_bScheduled
        //      During the ctor, we store some info while we still know what type of
        //      event we are dealing with, just to avoid some RTTI later. We store
        //      which type of event it is and whether it's been marked loggable.
        //
        //  m_bSerialized
        //      If a triggered event, we remember if it is marked as serialized. This
        //      is important to the processing, and it gets lost since we store the
        //      events via a common base class that doesn't know this info.
        //
        //  m_cptrTrigger
        //      The trigger data, if any. It's only used in triggered events. It's a
        //      counted pointer to the trigger info so that we don't have to replicate it
        //      for every worker thread it gets handed off to. When the last thread
        //      drops it, it'll be released.
        //
        //  m_cptrSrc
        //      A pointer to the command src to process. The m_bScheduled flag tells us
        //      if it's a scheduled or triggered event. It has to be a pointer since we
        //      have to hold either type via their shared base class. Since the queue
        //      that holds these objects is by value, we need to by copyable, and a counted
        //      pointer deals with that.
        //
        //  m_strEvPath
        //      We are using the base command source class to store a counted pointer to
        //      the incoming event. That doesn't provide us access to the original path
        //      of the event. So we get it on during the ctor and store it separately.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bLoggable;
        tCIDLib::TBoolean   m_bScheduled;
        tCIDLib::TBoolean   m_bSerialized;
        tCQCKit::TCQCEvPtr  m_cptrTrigger;
        TSrcPtr             m_cptrSrc;
        TString             m_strEvPath;
};



// ---------------------------------------------------------------------------
//   CLASS: TWorkerThread
//  PREFIX: thr
// ---------------------------------------------------------------------------
class TWorkerThread : public TThread
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TWorkerThread
        (
            const   tCIDLib::TCard4         c4Index
            ,       TMutex* const           pmtxSync
            ,       TEvent* const           pevWaitWorkers
            ,       TStdVarsTar* const      pctarGVars
            , const TCQCUserCtx&            cuctxToUse
        );

        TWorkerThread(const TWorkerThread&) = delete;
        TWorkerThread(TWorkerThread&&) = delete;

        ~TWorkerThread() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TWorkerThread& operator=(const TWorkerThread&) = delete;
        TWorkerThread& operator=(TWorkerThread&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddEvent
        (
            const   TEventQItem&            qitemNew
        );

        tCIDLib::TBoolean bIsActive()
        {
            return m_bActive;
        }

        tCIDLib::TBoolean bIsHandlingEvType
        (
            const   TString&                strPath
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::EExitCodes eProcess() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types and constants
        // -------------------------------------------------------------------
        const tCIDLib::TCard4 c4MaxEventblock = 64;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ProcessSchedEv();

        tCIDLib::TVoid ProcessTrigEv();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bActive
        //      Set true when we are actively processing an event, else false. Protected
        //      by the synx mutex.
        //
        //  m_colList
        //      The dispatching thread can give us more than one event at once, in the
        //      case of serialized triggered events. So we have to be prepared for that.
        //      The 0th one is the one we are working on at any given time. If this list
        //      is empty, then we stop after the current one is processed.
        //
        //      Protected by the sync mutex.
        //
        //  m_ctarEvent
        //      The triggered event target, which is set up with trigger info if we are
        //      doing a triggered event.
        //
        //  m_evControl
        //      An event that we are blocked on until the dispatch thread gives us
        //      an event to process.
        //
        //  m_pctarGVars
        //      We get a pointer to the shared global variables target from the facility
        //      object, which we pass to actions. It is thread safe.
        //
        //  m_pmeEvent
        //      We create a derivative of the macro engine that we use to process any
        //      CML macros invoked from actions. For triggered events we also set the
        //      trigger info on it, since that is accessible from CML.
        //
        //  m_pevWaitWorkers
        //      A pointer to the shared 'wait workers' event. We post this when we are
        //      done processing and going back to idle state again to wait for another
        //      event to process.
        //
        //  m_pmtxSync
        //      A mutex we share with the dispatch thread, and use to synchronized some
        //      activities between us.
        //
        //  m_qitemCur
        //      We pull each item out our list and put it here for processing. Protected
        //      by the sync mutex.
        // -------------------------------------------------------------------
        TCQCStdActEngine    m_acteToUse;
        tCIDLib::TBoolean   m_bActive;
        TQueue<TEventQItem> m_colList;
        TTrigEvCmdTarget    m_ctarEvent;
        TEvent              m_evControl;
        TStdVarsTar*        m_pctarGVars;
        TEvent*             m_pevWaitWorkers;
        TEventMacroEngine*  m_pmeEvent;
        TMutex*             m_pmtxSync;
        TEventQItem         m_qitemCur;
};




// ---------------------------------------------------------------------------
//   CLASS: TFacCQCEventSrv
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCEventSrv : public TCQCSrvCore
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCEventSrv();

        TFacCQCEventSrv(const TFacCQCEventSrv&) = delete;
        TFacCQCEventSrv(TFacCQCEventSrv&&) = delete;

        ~TFacCQCEventSrv();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCEventSrv& operator=(const TFacCQCEventSrv&) = delete;
        TFacCQCEventSrv& operator=(TFacCQCEventSrv&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DumpEvents
        (
            const   TString&                strTarFile
        );

        tCIDLib::TVoid AddEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid DeleteEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,        tCIDLib::TCard4&       c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );

        TMD5Hash mhashTrigKey() const
        {
            return m_mhashTrigKey;
        }

        tCIDLib::TVoid PauseEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const tCIDLib::TBoolean       bPaused
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid RenameEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelParPath
            , const TString&                strOldName
            , const TString&                strNewName
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid RenameScope
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelParPath
            , const TString&                strOldName
            , const TString&                strNewName
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid UpdateEvent
        (
            const   tCQCKit::EEvSrvTypes    eEvType
            , const TString&                strRelPath
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid QueryPerEvInfo
        (
            const   TString&                strPath
            ,       tCQCKit::ESchTypes&     eType
            ,       tCIDLib::TEncodedTime&  enctStart
            ,       tCIDLib::TCard4&        c4Period
            ,       tCIDLib::TBoolean&      bPaused
        );

        tCIDLib::TVoid QuerySchEvInfo
        (
            const   TString&                strPath
            ,       tCQCKit::ESchTypes&     eType
            ,       tCIDLib::TCard4&        c4Day
            ,       tCIDLib::TCard4&        c4Hour
            ,       tCIDLib::TCard4&        c4Minute
            ,       tCIDLib::TCard4&        c4Mask
            ,       tCIDLib::TBoolean&      bPaused
        );

        tCIDLib::TVoid QuerySunBasedEvInfo
        (
            const   TString&                strPath
            ,       tCIDLib::TInt4&         i4Offset
            ,       tCIDLib::TBoolean&      bPaused
        );

        tCIDLib::TVoid ReloadList
        (
            const   tCQCKit::EEvSrvTypes    eEvType
        );

        tCIDLib::TVoid SetLocInfo
        (
            const   tCIDLib::TFloat8        f8Lat
            , const tCIDLib::TFloat8        f8Long
            , const TCQCSecToken&           sectUser
        );

        tCIDLib::TVoid UpdateEvent
        (
            const   TString&                strPath
            ,       tCIDLib::TCard4&        c4ListSerNum
            ,       tCIDLib::TCard4&        c4ChangeSerNum
            , const TCQCSecToken&           sectUser
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DeregisterSrvObjs() final;

        tCQCSrvFW::EStateRes eLoadConfig
        (
                    tCIDLib::TCard4&        c4WaitNext
            , const tCIDLib::TCard4         c4Count
        )   final;

        tCIDLib::TVoid PostDeregTerm() final;

        tCIDLib::TVoid PreRegInit() final;

        tCIDLib::TVoid QueryCoreAdminInfo
        (
                    TString&                strCoreAdminBinding
            ,       TString&                strCoreAdminDesc
            ,       TString&                strExtra1
            ,       TString&                strExtra2
            ,       TString&                strExtra3
            ,       TString&                strExtra4
        )   final;

        tCIDLib::TVoid RegisterSrvObjs() final;

        tCIDLib::TVoid StartWorkerThreads() final;

        tCIDLib::TVoid StopWorkerThreads() final;

        tCIDLib::TVoid UnbindSrvObjs
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data types
        // -------------------------------------------------------------------
        using TSortedEvList = TRefVector<TKeyedCQCSchEvent>;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCQCKit::EEvSrvTypes eCheckListChanges
        (
            const   tCIDLib::TCard4         c4SerNumToCheck
        )   const;

        tCIDLib::TBoolean bCheckDS();

        tCIDLib::TCard4 c4FindEvMon
        (
            const   TString&                strRelPath
            , const tCIDLib::EFindFlags     eFlag
        );

        tCIDLib::TCard4 c4FindSchEv
        (
            const   TString&                strRelPath
            , const tCIDLib::EFindFlags     eFlag
        );

        tCIDLib::TCard4 c4FindTrgEv
        (
            const   TString&                strRelPath
            , const tCIDLib::EFindFlags     eFlag
        );

        tCIDLib::EExitCodes eDispatchThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::EExitCodes eEvThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::EExitCodes eQRThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid LoadEvMonitors
        (
                    TDataSrvClient&         dsclToUse
        );

        tCIDLib::TVoid LoadSchedEvs
        (
                    TDataSrvClient&         dsclToUse
        );

        tCIDLib::TVoid LoadTrigEvs
        (
                    TDataSrvClient&         dsclToUse
        );

        tCIDLib::TVoid MakeSortedSchedule();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bGotLocInfo
        //      We have to get location info before we can start processing events,
        //      so this is used to keep up with whether we have gotten that yet. It
        //      is needed for sunrise/sunset event processing.
        //
        //  m_c4SerChanges
        //      Every time an existing event configuration changed, this number is set
        //      on it, and then this value is bumped. This allows clients to ask for
        //      anything that changed since the last time it checked. It's also used to
        //      set the list serial numbers below.
        //
        //  m_c4SerEvMonList
        //  m_c4SerSchEvList
        //  m_c4SerTrgEvList
        //      Any time we make a change to one of the event type lists, we bump
        //      the change serial number (above) and store it into the apprpriate
        //      one of these values. If a client's last seen change serial number is
        //      less than one of these, then that list has changed since he last
        //      checked and he needs to re-sync.
        //
        //  m_colActQ
        //      When an event's time has come, a copy of the action is placed on
        //      this queue. This guy contains a copy of the event (scheduled or triggered)
        //      plus any other data required. This way the outside world can send us
        //      modifications to the configured events without conflicts. Those changes
        //      will be used the next time that event is ready.
        //
        //      This guy is thread safe. It is shared by the two queuing threads, and
        //      the dispatch thread. They put them in, and he pulls them out. Both are
        //      just single, atomic operations, so simple to deal with.
        //
        //  m_colEvMonitors
        //      We need one thread per configured event monitor. Each one gets its
        //      own thread. We also store the configuration info in these as well.
        //      We keep one of these objects around for each configured monitor and
        //      start/stop it as required. These are keyed by the type relative path,
        //      so starting at /User or /System.
        //
        //  m_colSchEvents
        //  m_colTrgEvents
        //      This is where we keep the raw list of events at runtime. For scheduled
        //      events also use a by ref collection pointing into the list so that we
        //      can have a separate view, sorted by next invocation time, which is
        //      m_colTimeView below. These are keyed by the type relative path, so
        //      starting with /User or /System.
        //
        //  m_colTimeView
        //      We load this with pointers to the items in the m_colSchEvents list, non-
        //      adopting, and sort it by time. This lets us always have a list of the
        //      things to do in order of time. Any time one comes up time, we calculate
        //      it's next time and re-sort. So this always has the next available
        //      scheduled event in the 0th element.
        //
        //  m_colWorkerThreads
        //      We have a small pool of threads to process scheduled and triggered events.
        //      Event monitors have their own list since they are fixed threads. These
        //      guys interact with the dispatcher thread. The use the m_mtxInner mutex
        //      to sync with him.
        //
        //  m_ctarGVars
        //      The background threads that process event actions will use this single
        //      object for their global variables. This will allow all of the actions to
        //      share a set of global variables. It's set up to be thread safe.
        //
        //  m_eCfgState
        //      This is used during the config loading process.
        //
        //  m_f8Lat
        //  m_f8Long
        //      The lat/long info that we read from the master server config server or
        //      that is set on us initially by a client. This is used to deal with
        //      scheduling sunrise/sunset scheduled events.
        //
        //  m_mhashTrigKey
        //      We generate this on startup and clients that need to send event triggers
        //      query it via our ORB interface.
        //
        //  m_mtxInner
        //      See the header comments. This is used to sync beteen the dispatching
        //      threads and the worker threads. They share some state info about the
        //      worker threads.
        //
        //  m_mtxOuter
        //      See the header comments. This is used to sync between the two queuing
        //      threads and the outside world, to control access to the lists of
        //      configured schedled and triggered events, which they share.
        //
        //  m_porbsProtoImpl
        //      We keep a pointer to our server objects, which we register with the ORB
        //      and advertise in the name server. We don't own it, the ORB does. This one
        //      is for the main scheduled events server.
        //
        //  m_ppolleToUse
        //      We have to provide a polling engine to some facilities we make use of. We
        //      want them all to share one for efficiency.
        //
        //  m_thrEv
        //      The triggered event monitor thread. This guy stays blocked on CQCKit's
        //      event trigger read method. When a new trigger arrives, it will run all
        //      the configured triggered event's filters against the event and for
        //      those that match, it'll put them in the m_colActQ.
        //
        //  m_thrQR
        //      The 'queue'er' thread. It just keeps an eye on the sorted time
        //      view collection (m_colTimeView) and when an action is ready
        //      to go, it puts a copy of it on the action queue (m_colActQ)
        //      so that it will be processed, updates the time for the next
        //      schedulable time, and then waits for the next one. We just
        //      start it up on the eQRThread() method.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bGotLocInfo;
        tCIDLib::TCard4             m_c4ActiveWorkers;
        tCIDLib::TCard4             m_c4SerChanges;
        tCIDLib::TCard4             m_c4SerEvMonList;
        tCIDLib::TCard4             m_c4SerSchEvList;
        tCIDLib::TCard4             m_c4SerTrgEvList;
        TQueue<TEventQItem>         m_colActQ;
        TRefVector<TCQCEvMonitor>   m_colEvMonitors;
        tCQCEvCl::TKeyedSchEvList   m_colSchEvents;
        tCQCEvCl::TKeyedTrgEvList   m_colTrgEvents;
        TSortedEvList               m_colTimeView;
        TRefVector<TWorkerThread>   m_colWorkerThreads;
        TStdVarsTar                 m_ctarGVars;
        tCQCEventSrv::ECfgStates    m_eCfgState;
        TEvent                      m_evWaitWorkers;
        tCIDLib::TFloat8            m_f8Lat;
        tCIDLib::TFloat8            m_f8Long;
        TMD5Hash                    m_mhashTrigKey;
        TMutex                      m_mtxInner;
        TMutex                      m_mtxOuter;
        TCQCPollEngine              m_polleToUse;
        TEventServerImpl*           m_porbsProtoImpl;
        TThread                     m_thrDispatch;
        TThread                     m_thrEv;
        TThread                     m_thrQR;


        // -------------------------------------------------------------------
        //  Private, static members
        //
        //  s_sciCurSrvState
        //      We keep the current state of the event server available as a stat, so
        //      that we can monitor whether its health.
        //
        //  s_sciTrigsRecieved
        //      The number of event triggers we've received from CQCKit's published
        //      event trigger topic. CQCKit maintains a triggers received, so any
        //      discrepancy means something is going awry in between.
        //
        //  s_sciWorkersBusy
        //      The number of worker threads currently busy
        // -------------------------------------------------------------------
        static TStatsCacheItem      s_sciCurSrvState;
        static TStatsCacheItem      s_sciTrigsReceived;
        static TStatsCacheItem      s_sciWorkersBusy;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCEventSrv,TFacility)
};


