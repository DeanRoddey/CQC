//
// FILE NAME: CQCWBHelper_ThisFacility.hpp
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
//  This is the header for the facility class for the remote port server.
//
//  The client program invokes us like this:
//
//      CQCWBHelper uniqueid
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
//   CLASS: TFacCQCWBHelper
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCWBHelper : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCWBHelper();

        TFacCQCWBHelper(const TFacCQCWBHelper&) = delete;

        ~TFacCQCWBHelper();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TFacCQCWBHelper& operator=(const TFacCQCWBHelper&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetNextMsg
        (
                    TQMsg&                  qmsgToFill
        );

        tCIDLib::TBoolean bTimedOut();

        tCIDLib::TVoid Close();

        tCIDLib::TVoid DoOp
        (
            const   tWebBrowser::ENavOps    eToDo
        );

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid Initialize
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
            , const tCIDLib::TBoolean       bVisibility
            , const TString&                strInitURL
        );

        tCIDLib::TVoid Ping();

        tCIDLib::TVoid SetArea
        (
            const   TArea&                  areaPar
            , const TArea&                  areaWidget
        );

        tCIDLib::TVoid SetURL
        (
            const   TString&                strToSet
        );

        tCIDLib::TVoid SetVisibility
        (
            const   tCIDLib::TBoolean       bNewState
        );


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid QueueMsg
        (
            const   TQMsg&                  qmsgToDo
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSilentMode
        //      Indicates whether we put the browser into silent mode or not, passed to us
        //      by the invoking client.
        //
        //  m_c4RefreshMins
        //      The auto-refresh minutes, zero if not enabled. Passed to us by the invoking
        //      client.
        //
        //  m_colMsgQ
        //      This is where incoming command messages get queued up for us to process,
        //      via incoming ORB calls.
        //
        //  m_enctLastMsg
        //      The last time we got a call from the client that started us. If we don't
        //      see something within a reasonable time, we close ourself.
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
        //  m_wndMainFrame
        //      Our main window. It in turn contains a browser window.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSilentMode;
        tCIDLib::TCard4         m_c4RefreshMins;
        TQueue<TQMsg>           m_colMsgQ;
        tCIDLib::TEncodedTime   m_enctLastMsg;
        TOrbObjId               m_ooidServer;
        TWBHelperCtrlImpl*      m_porbsCtrl;
        TMainFrameWnd           m_wndMainFrame;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCWBHelper,TGUIFacility)
};



