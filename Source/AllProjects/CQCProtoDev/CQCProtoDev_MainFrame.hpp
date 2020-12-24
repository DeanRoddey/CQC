//
// FILE NAME: CQCProtoDev_MainFrame.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/27/2003
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
//  This is the header for the CQCProtoDev_MainFrame.cpp file, which implements
//  the TCQCProtoDevFrameWnd. This is the main frame window of the protocol driver
//  development test harness. It manages a TGenProtoS driver object as it would be
//  managed by CQCServer.
//
//  We also implement the protocol driver's debug output mixin, so that it will send
//  us debug output that we put into a text output window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TCQCProtoDevFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCProtoDevFrameWnd : public TFrameWnd, public MGenProtoDebug
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCProtoDevFrameWnd();

        TCQCProtoDevFrameWnd(const TCQCProtoDevFrameWnd&) = delete;

        ~TCQCProtoDevFrameWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCProtoDevFrameWnd& operator=(const TCQCProtoDevFrameWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Event
        (
            const   TString&                strTitle
            , const TMemBuf&                mbufData
            , const tCIDLib::TCard4         c4Bytes
        );

        tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
            , const TString&                strMsg2
        );

        tCIDLib::TVoid Msg
        (
            const   TString&                strMsg
            , const TString&                strMsg2
            , const TString&                strMsg3
        );

        tCIDLib::TVoid StateTransition
        (
            const   tCIDLib::TCard1         c1ByteReceived
            , const TString&                strAction
            , const TString&                strNewState
        );


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Create();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;

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


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCloseCurrent();

        tCIDLib::TVoid ChangeField();

        tCIDLib::TVoid CheckShutdown();

        tCIDLib::TVoid DoNewSession();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tGenProtoS::EDbgMsgs eCurMsgChecks() const;

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid PackageDriver();

        tCIDLib::TVoid RemotePort();

        tCIDLib::TVoid SaveState();

        tCIDLib::TVoid Start();

        tCIDLib::TVoid Stop();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bLastTime
        //      Our timer, which among other things keeps the current field value
        //      updated, has to remember whether the driver was there last time he
        //      checked or not.
        //
        //  m_c4FldListId
        //  m_c4FldId
        //  m_c4FldSerialNum
        //      In order to keep the current field value updated, we have to remember
        //      which field we are displaying. These members allow us to call a polling
        //      method on the driver to get the most recent value and know if it's
        //      changed or not.
        //
        //  m_colFields
        //      The list of fields we query from the driver after it's ready and
        //      has registered the fields.
        //
        //  m_colOutputQ
        //      The debug output handler we install on the driver will dump driver
        //      debug output into this queue. Our timer will pull the items out and
        //      dump them to the text output window. It is thread safe so it doesn't
        //      require any other sync for this.
        //
        //  m_cv2vValidator
        //      A V2 validator object to support validation of driver fields.
        //
        //  m_eCurState
        //      We need to know when the driver state changes so that we can update the
        //      display.
        //
        //  m_eDefVerbosity
        //      The driver object isn't kept around when we don't have a session open,
        //      so we have to have a place to store the user's (menu) selected verbosity
        //      level until we crank up the driver.
        //
        //  m_cqcdcTest
        //      We parse out a driver config object from the XML manifest file. We keep
        //      it around since it has information of interest to us. We'll update it if
        //      they start a new session with a different manifest file.
        //
        //  m_pathProtoFile
        //      The path to the protocol file. In order to pick up changes, we reload
        //      it each time they tell us to do a syntax check or to run.
        //
        //  m_pfactRemote
        //      If the user has set us up for a remote port, then we register a remote
        //      port factory with CIDComm. We keep a pointer around for our own use and
        //      store it here, but we don't own it. If we aren't configured for a remote
        //      port, then this is null.
        //
        //  m_psdrvTest
        //      The GenProtoS driver object that we use for testing. We just run the
        //      driver as it would be run by CQCServer. It's fairly self contained
        //      anyway, so we just get it started and then leave it alone until they
        //      stop us.
        //
        //  m_pwndClient
        //      We create a generic window and set it as our client, and create all of
        //      the controls within it.
        //
        //  m_pwndXXX
        //      Typed pointers to some of our controls. We don't own them, we want
        //      typed pointers to avoid lots of downcasting.
        //
        //  m_strManifestFile
        //      The name of the manifest file that was opened to starte the current
        //      session. If no session, this is empty.
        //
        //  m_strmFmt
        //      We need a stream for formatting some of the callbacks from the driver
        //      into a stream so that we can put the result into the output queue.
        //
        //  m_tmidUpdate
        //      We create a timer that watches the output of the background thread
        //      that runs the driver.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean               m_bLastTime;
        tCIDLib::TCard4                 m_c4FldListId;
        tCIDLib::TCard4                 m_c4FldId;
        tCIDLib::TCard4                 m_c4FldSerialNum;
        TCQCServerBase::TQueryFieldList m_colFields;
        tCIDLib::TStringQ               m_colOutputQ;
        TCQCV2Validator                 m_cv2vValidator;
        TCQCDriverObjCfg                m_cqcdcTest;
        tCQCKit::EDrvStates             m_eCurState;
        tCQCKit::EVerboseLvls           m_eDefVerbosity;
        TPathStr                        m_pathProtoFile;
        TCQCRemSrvPortFactory*          m_pfactRemote;
        TGenProtoSDriver*               m_psdrvTest;
        TPushButton*                    m_pwndChangeFld;
        TGenericWnd*                    m_pwndClient;
        TCheckBox*                      m_pwndCommRes;
        TCheckBox*                      m_pwndConnection;
        TTextOutWnd*                    m_pwndDbgOut;
        TStaticText*                    m_pwndDrvState;
        TListBox*                       m_pwndFldList;
        TStaticText*                    m_pwndFldValue;
        TCheckBox*                      m_pwndFldWrite;
        TCheckBox*                      m_pwndMsgMapping;
        TCheckBox*                      m_pwndPollEvents;
        TCheckBox*                      m_pwndRecBytes;
        TPushButton*                    m_pwndStartStop;
        TCheckBox*                      m_pwndStateMachine;
        TCheckBox*                      m_pwndSendBytes;
        TString                         m_strManifestFile;
        TTextStringOutStream            m_strmFmt;
        tCIDCtrls::TTimerId             m_tmidUpdate;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCProtoDevFrameWnd,TFrameWnd)
};


