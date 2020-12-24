//
// FILE NAME: CQCWCHelper_ThisFacility.hpp
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
//  This is the header for the facility class for the web camera helper program.
//
//  The client program invokes us like this:
//
//      CQCWCHelper uniqueid
//
//      uniqueid  - We use this to create a shared memory buffer and shared event. These
//                  are used to let the program pass us in information, and we use it to
//                  return to him our ORB object id, and signal him we are ready so that he
//                  can safely grab the object id back out of the buffer.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  CLASS: TPlMsg
// PREFIX: qmsg
// ---------------------------------------------------------------------------
class TPlMsg
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TPlMsg();

        TPlMsg
        (
            const   tCQCWCHelper::EPlMsgTypes   eType
            , const tCIDLib::TBoolean           bVal
        );

        TPlMsg
        (
            const   tCQCWCHelper::EPlMsgTypes   eType
            , const tCIDLib::TCard4             c4Val
        );

        TPlMsg(const TPlMsg&);

        ~TPlMsg();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TPlMsg& operator=(const TPlMsg&);


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        tCIDLib::TBoolean           m_bVal;
        tCIDLib::TCard4             m_c4Val;
        tCQCWCHelper::EPlMsgTypes   m_eType;
};


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCWCHelper
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCWCHelper : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCWCHelper();

        TFacCQCWCHelper(const TFacCQCWCHelper&) = delete;

        ~TFacCQCWCHelper();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCWCHelper& operator=(const TFacCQCWCHelper&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetNextGUIMsg
        (
                    TGUIMsg&                  qmsgToFill
        );

        tCIDLib::TBoolean bTimedOut();

        tCIDLib::TVoid Close();

        tCQCWCHelper::EStates eCurState() const;

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TBoolean bVerboseLogging() const;

        tCIDLib::TVoid Initialize
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
            , const tCIDLib::TBoolean       bVisibility
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
        );

        tCIDLib::TVoid SetPlaybackState
        (
            const   tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetMute
        (
            const   tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetVisibility
        (
            const   tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SetVolume
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ApplyParams();

        tCIDLib::EExitCodes eMgrThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid QueuePlMsg
        (
            const   TPlMsg&                 qmsgToDo
        );

        tCIDLib::TVoid QueueGUIMsg
        (
            const   TGUIMsg&                qmsgToDo
        );

        tCIDLib::TVoid RegisterVLCEvents();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bReqMuteState
        //      This is the last mute state that was requested of us. Since we can only apply
        //      mute state while playback is actually happening, we have to save the last
        //      requested value so that we can apply on next startup. If we are currently
        //      playing when a request comes in, we apply it immediately as well. Defaults to
        //      true, and then is initialized to whatever is sent over in the initial
        //      block of info we get on startup.
        //
        //  m_bVerboseLogging
        //      The calling application can ask us to do verbose logging for better
        //      debugging in the field.
        //
        //  m_c4ReqVolume
        //      This is the last volume that was requested of us. Since we can only set the
        //      volume while playback is actually ahppen, we have to save the last requested
        //      volume so that we can apply ot the next startup. If we are current playing
        //      when a request comes it, we apply it immediately as well. Defaults to 100%,
        //      and then is initialized to whatever is sent over in the initial block of
        //      info we get on startup.
        //
        //  m_colGUIMsgQ
        //      This is where incoming GUI oriented ORB commands get queued up for the GUI
        //      thread to process.
        //
        //  m_colPlMsgQ
        //      This is where incoming player oriented ORB commands get queued up for the
        //      player thread to process.
        //
        //  m_enctLastMsg
        //      The last time we got a call from the client that started us. If we don't
        //      see something within a reasonable time, we close ourself.
        //
        //  m_eCurState
        //      Our current state, which tells us what we need to do next.
        //
        //  m_ooidServer
        //      When the client runs us, he gives us some info (an instance id and a port
        //      number) and we create an object id manually and use it to create our
        //      server object. This allows the client to connect to us via the ORB without
        //      us having to register in the name server. This is much preferrd for such
        //      a private connection, and just simplifies things a lot.
        //
        //  m_porbsCtrl
        //      Our control interface server side implementation. It's created on startup
        //      and registered with the ORB.
        //
        //  m_colParams
        //      We get quoted comma list of key=value values, that we use to allow for open
        //      ended parameters to be provided for moving forward, without having to have
        //      separate configurable parameters for them all, since they may change by
        //      model quite significantly. We parse them out and store them here for later
        //      use during VLC setup.
        //
        //  m_strURL
        //      The URL we were told to use (by the calling application.)
        //
        //  m_thrManager
        //      We spin off a background thread that manages the VLC stuff, and tries to
        //      keep us connected. The main thread just runs the main window loop and
        //      waits for it to end.
        //
        //  m_wndMainFrame
        //      Our main window. It in turn contains a browser window.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bReqMuteState;
        tCIDLib::TBoolean       m_bVerboseLogging;
        tCIDLib::TCard4         m_c4ReqVolume;
        TQueue<TGUIMsg>         m_colGUIMsgQ;
        TQueue<TPlMsg>          m_colPlMsgQ;
        tCIDLib::TKVPList       m_colParams;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        tCQCWCHelper::EStates   m_eCurState;
        TOrbObjId               m_ooidServer;
        TWCHelperCtrlImpl*      m_porbsCtrl;
        libvlc_instance_t*      m_pVLCInst;
        libvlc_media_player_t*  m_pVLCPlayer;
        TString                 m_strURL;
        TThread                 m_thrManager;
        TMainFrameWnd           m_wndMainFrame;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCWCHelper,TGUIFacility)
};



