//
// FILE NAME: CQCVoice_ThisFacility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/03/2016
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
//  This is the header for the facility class for this facility. This guy is mainly
//  just a passive storer and coordinator of access to various information we need
//  to track. The actual work is done by the behavior three thread, which is started
//  on the eBTThread method. It and the BT tree nodes it invokes will call us here to
//  change our state or access data as required.
//
//  Some stuff is local in the BT thread method if no one else needs to see it, which
//  just makes it harder to do anything wrong.
//
//  Somce the BT thread does everything except a little start/shutdown stuff, at which
//  time the BT thread is no running, we really don't have any sync concerns in this
//  program at this time.
//
//  In addition to the normal spoken replies, we can also do a visual reply, to help
//  hearing impaired folks use voice control. We can do either or both. If visual is
//  enabled we create a separate window and send the text to it. If only doing visual
//  we just display the text and immediately return to start listning. Else we will
//  display it with a blinking symbol that blinks until the spoken text is completed
//  and we are ready to return and listen again.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)

// ---------------------------------------------------------------------------
//   CLASS: TFacCQCVoice
//  PREFIX: fac
// ---------------------------------------------------------------------------
class TFacCQCVoice : public TGUIFacility
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TFacCQCVoice();

        TFacCQCVoice(const TFacCQCVoice&) = delete;
        TFacCQCVoice(TFacCQCVoice&&) = delete;

        ~TFacCQCVoice();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AddReminder
        (
            const   TString&                strText
            , const tCIDLib::TCard4         c4Minutes
        );

        tCIDLib::TBoolean bAllAtConfidence
        (
            const   tCIDSpReco::TSemList&   colSemVals
            , const tCQCVoice::EConfLevels eLevel
        )   const;

        tCIDLib::TBoolean bCapFlag
        (
            const   tCQCVoice::ECapFlags   eFlag
        )   const;

        tCIDLib::TBoolean bCheckAction
        (
            const   tCIDSpReco::TSemList&   colSemValues
            , const TString&                strAction
        )   const;

        tCIDLib::TBoolean bCheckRoomThermo
        (
                    TString&                strMonToFill
            ,       TString&                strSubToFill
        )   const;

        tCIDLib::TBoolean bCheckRoomWeather
        (
                    TString&                strMonToFill
        )   const;

        tCIDLib::TBoolean bDebugOutput() const;

        tCIDLib::TBoolean bDebugOutput
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bDebugVerbose() const;

        tCIDLib::TBoolean bDebugVerbose
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TBoolean bGetNextEvent
        (
                    TCIDSpeechRecoEv&       sprecevToFill
            ,       TString&                strAction
            , const tCIDLib::TCard4         c4WaitMillis
            , const tCIDLib::TBoolean       bWantLowConf = kCIDLib::False
        );

        tCIDLib::TBoolean bIsAtConfidence
        (
            const   TKeyNumPair&            knumToTest
            , const tCQCVoice::EConfLevels eLevel
        )   const;

        tCIDLib::TBoolean bIsAtConfidence
        (
            const   TAIBehaviorTree&        btreeOwner
            , const TString&                strSemName
            , const tCQCVoice::EConfLevels eLevel
        )   const;

        tCIDLib::TBoolean bIsFullConfidence
        (
            const   TKeyNumPair&            knumToTest
        )   const;

        tCIDLib::TBoolean bIsFullConfidence
        (
            const   TAIBehaviorTree&        btreeOwner
            , const TString&                strSemName
        )   const;

        tCIDLib::TBoolean bIsHighConfidence
        (
            const   TKeyNumPair&            knumToTest
        )   const;

        tCIDLib::TBoolean bIsHighConfidence
        (
            const   TAIBehaviorTree&        btreeOwner
            , const TString&                strSemName
        )   const;

        tCIDLib::TBoolean bIsMedConfidence
        (
            const   TKeyNumPair&            knumToTest
        )   const;

        tCIDLib::TBoolean bIsMedConfidence
        (
            const   TAIBehaviorTree&        btreeOwner
            , const TString&                strSemName
        )   const;

        tCIDLib::TBoolean bRangeCheckIntFld
        (
                    tCQCKit::TCQCSrvProxy&  orbcDrv
            , const TString&                strMoniker
            , const TString&                strField
            , const tCIDLib::TInt4          i4TestVal
        );

        tCIDLib::TBoolean bRangeCheckValue
        (
            const   tCIDLib::TInt4          i4ToCheck
            , const tCIDLib::TInt4          i4Min
            , const tCIDLib::TInt4          i4Max
        );

        tCIDLib::TBoolean bReminderExists
        (
            const   TString&                strToCheck
            ,       tCIDLib::TCard4&        c4Count
        )   const;

        tCIDLib::TBoolean bRunGlobalAct
        (
            const   TSCGlobActInfo&         scliToRun
        );

        tCIDLib::TBoolean bSetLightLevel
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4Percent
        );

        tCIDLib::TBoolean bSetLightState
        (
            const   TString&                strName
            , const tCIDLib::TBoolean       bState
        );

        tCIDLib::TBoolean bSetThermoSP
        (
            const   TString&                strSPName
            , const TString&                strVal
            , const TString&                strSpokenSPName
        );

        tCIDLib::TBoolean bWriteField
        (
            const   TString&                strMoniker
            , const TString&                strField
            , const TString&                strValue
            , const tCIDLib::TBoolean       bThrowIfErr = kCIDLib::False
        );

        TStdVarsTar& ctarGlobalVars();

        const TCQCUserCtx& cuctxToUse() const;

        tCIDLib::TVoid CancelAllReminders();

        tCIDLib::TVoid CancelLastReminder();

        tCIDLib::TVoid CheckReminders();

        tCQCVoice::EWaitRepModes eClarifyOffOn
        (
                    tCIDLib::TBoolean&      bState
        );

        tCQCVoice::EWaitRepModes eClarifyPercentage
        (
                    TString&                strValue
        );

        tCQCVoice::EWaitRepModes eClarifySignedNum
        (
                    TString&                strValue
        );

        tCIDLib::EExitCodes eMainThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCQCVoice::ELightReads eQueryLightState
        (
            const   TString&                strName
        );

        tCQCVoice::EWaitRepModes eShouldITurnItX
        (
            const   tCIDLib::TBoolean       bTarState
        );

        tCQCVoice::EWaitRepModes eWaitForAction
        (
            const   tCIDLib::TStrList&      colWaitForActs
            , const TString&                strWaitForDescr
            ,       TCIDSpeechRecoEv&       sprecevToFill
            ,       tCIDLib::TCard4&        c4MatchedAt
        );

        tCQCVoice::EWaitRepModes eWaitForAction
        (
            const   TString&                strWaitFor
            , const TString&                strWaitForDescr
            ,       TCIDSpeechRecoEv&       sprecevToFill
        );

        tCQCVoice::EWaitRepModes eWaitForActionOrNo
        (
            const   TString&                strWaitFor
            , const TString&                strWaitForDescr
            ,       TCIDSpeechRecoEv&       sprecevToFill
        );

        tCQCVoice::EWaitRepModes eWaitForActionOrYes
        (
            const   TString&                strWaitFor
            , const TString&                strWaitForDescr
            ,       TCIDSpeechRecoEv&       sprecevToFill
        );

        tCQCVoice::EWaitRepModes eWaitForGoodEvent
        (
                    TCIDSpeechRecoEv&       sprecevToFill
            ,       TString&                strAction
            , const tCIDLib::TCard4         c4WaitMSs
        );

        tCIDLib::TVoid EatPendingEvents();

        const TMediaDB& mdbMusic() const;

        const TSCLoadInfo* pscliLoadByName
        (
            const   TString&                strName
        )   const;

        const TCQCSysCfg& scfgCurrent() const;

        const TSCHVACInfo& scliHVAC() const;

        const TSCMediaInfo& scliMusic() const;

        const TSCSecInfo& scliSecurity() const;

        const TCQCSysCfgRmInfo& scrmiOurs() const;

        const TString& strRoom() const;

        tCIDLib::TVoid SetCapFlag
        (
            const   tCQCVoice::ECapFlags   eFlag
            , const tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetRecoPause
        (
            const   tCIDLib::TBoolean       bToSet
        );

        tCIDLib::TVoid SetReloadReqFlag();

        tCIDLib::TVoid SetRuleState
        (
            const   TString&                strRuleName
            , const tCIDLib::TBoolean       bNewState
        );

        tCIDLib::TVoid SleepWake
        (
            const   tCIDLib::TBoolean       bWakingUp
        );

        tCIDLib::TVoid ShowMsg
        (
            const   TString&                strMsg
        );

        tCIDLib::TVoid SpeakCallMe();

        tCIDLib::TVoid SpeakCantUnderstand
        (
            const   TString&                strWhat
        );

        tCIDLib::TVoid SpeakDone();

        tCIDLib::TVoid SpeakFailed();

        tCIDLib::TVoid SpeakPleaseWait();

        tCIDLib::TVoid SpeakReply
        (
            const   TString&                strText
            , const tCIDLib::TBoolean       bMarkup = kCIDLib::False
        );

        tCIDLib::TVoid SpeakReply
        (
            const   tCIDLib::TMsgId         midText
            , const tCIDLib::TBoolean       bMarkup = kCIDLib::False
        );

        tCIDLib::TVoid SpeakReply
        (
            const   tCIDLib::TMsgId         midText
            , const MFormattable&           fmtblToken1
            , const tCIDLib::TBoolean       bMarkup = kCIDLib::False
        );

        tCIDLib::TVoid SpeakReply
        (
            const   tCIDLib::TMsgId         midText
            , const MFormattable&           fmtblToken1
            , const MFormattable&           fmtblToken2
            , const tCIDLib::TBoolean       bMarkup = kCIDLib::False
        );

        tCIDLib::TVoid SpeakReply
        (
            const   tCIDLib::TMsgId         midText
            , const MFormattable&           fmtblToken1
            , const MFormattable&           fmtblToken2
            , const MFormattable&           fmtblToken3
            , const tCIDLib::TBoolean       bMarkup = kCIDLib::False
        );

        tCIDLib::TVoid SpeakTryAgain();

        tCIDLib::TVoid SpeakWhatCanIDo();

        tCIDLib::TVoid Shutdown
        (
            const   tCIDLib::EExitCodes     eReturn
        );

        tCIDLib::TVoid ToggleFrameVis();

        tCIDLib::TVoid UpdateReminderTime
        (
            const   TString&                strToFind
            , const tCIDLib::TCard4         c4Minutes
        );


    private :
        // -------------------------------------------------------------------
        //  Private types
        // -------------------------------------------------------------------
        typedef TQueue<TCIDSpeechRecoEv>                            TSpeechQ;
        typedef TVector<TCQCVoiceReminder>                          TReminderList;
        typedef TFundArray<tCIDLib::TBoolean, tCQCVoice::ECapFlags> TCapFlagsList;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AdjustGrammar();

        tCIDLib::TBoolean bDoLogin
        (
            const   tCIDLib::TCard4         c4RoundCnt
            ,       TError&                 errLast
        );

        tCIDLib::TBoolean bLoadGrammar
        (
            const   tCIDLib::TCard4         c4RoundCnt
            ,       TError&                 errLast
        );

        tCIDLib::TBoolean bLoadVoiceConfig
        (
            const   tCIDLib::TCard4         c4RoundCnt
            ,       TError&                 errLast
        );

        tCIDLib::TBoolean bLoadRoomConfig
        (
            const   tCIDLib::TCard4         c4RoundCnt
            ,       TError&                 errLast
        );

        tCIDLib::TBoolean bParseParms
        (
                    tCIDLib::TCard4&        c4DelaySecs
        );

        tCIDLib::TBoolean bSetupRecoEngine
        (
            const   tCIDLib::TCard4         c4RoundCnt
            ,       TError&                 errLast
        );

        tCIDLib::TVoid DoCleanup();

        tCIDLib::EExitCodes eBTThread
        (
                    TThread&                thrThis
            ,       tCIDLib::TVoid*         pData
        );

        tCIDLib::TVoid FindOurRoom();

        tCIDLib::TVoid GetMarkedupText
        (
            const   TString&                strXML
            ,       TString&                strToFill
        );

        TCIDSpReco& sprecoLocal();

        tCIDLib::TVoid LoadCustReminders();

        tCIDLib::TVoid LoadDynRules();

        tCIDLib::TVoid LoadMusicCats();

        tCIDLib::TVoid LoadPlaylists();

        tCIDLib::TVoid LoadRoomModes();

        tCIDLib::TVoid LoadSecArmModes
        (
            const   TSCSecInfo&             scliSec
        );

        tCIDLib::TVoid LoadSecZones();

        TCQCVoiceReminder* pcvremFindByUniqueId
        (
            const   tCIDLib::TCard4         c4UniqueId
        );

        tCIDLib::TVoid ProcessEvents
        (
                    TCIDSpeechRecoEv&       sprecevCurm
            , const tCIDLib::TKValsList     colVars
            ,       TString&                strAction
            ,       TString&                strSemVal
        );

        tCIDLib::TVoid RemoveBindings();

        tCIDLib::TVoid SetConversationMode
        (
            const   tCIDLib::TBoolean       bToSet
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bDebugOutput
        //      Controls whether we show debug output in the output window. This is
        //      set or cleared via a checkbox on the main window. The other code checks
        //      this flag to control whether it does debug output or not.
        //
        //  m_bDebugVerbose
        //      If m_bDebugOutput is true, this can be set to display all events, and some
        //      other info, whereas normally it will only show accepted events.
        //
        //  m_bReloadReq
        //      The user can request a system config reload by voice command, but we
        //      provide a button on our main window to do it as well. It cannot do it
        //      directly, bit it just calls us to set this flag, which the BT thread
        //      will see the next time it is in a position to do it.
        //
        //  m_btreeLogic
        //      This is the behavior tree that we load and use to drive our interaction
        //      with the user. It is only used by the behavior tree thread.
        //
        //  m_bSpeakReplies
        //  m_bTextReplies
        //      We can be configured for spoken replies, displayed text replies, or
        //      both. The configuration will be passed to us as parameters which will be
        //      used to set these. If neither is indicated, we default to spoken.
        //
        //  m_c4GrammarSz
        //      This is the size of the last grammar buffer we downloaded from the
        //      MS. It relates to m_mbufGrammar.
        //
        //  m_c4MaxClients
        //      The max simultaneous clients to support. If its not provided on the
        //      command line, we allow the default Orb max to be used.
        //
        //  m_c4LastRemId
        //      The unique id of the last reminder that was added, zero if none or it
        //      has already been done. The last one added is not necessarily at the
        //      start of the list, since they are sorted by time.
        //
        //  m_c4NextRemId
        //      We assign a unique id to each reminder, and store the id of the last
        //      added one, so that we can support a 'cancel last reminder' command. Zero
        //      is never a valid id.
        //
        //  m_c4SysCfgSerNum
        //      The last system configuration serial number we got. Initialized to zero
        //      to insure we get initial data.
        //
        //  m_colReminders
        //      This is where we store any reminders that the user gives us. It is sorted
        //      ascending by the 'at time' of the reminders, so the ones at the start of
        //      the list have the earliest times and so the next ones that will be done.
        //      The background thread calls us periodically to let us check for any
        //      reminders. He calls us when idle, i.e. not in any conversation.
        //
        //  m_cptrMusic
        //      If music is configured for our room, then we get the music repo's media
        //      database and store it here for later use. We also store the serial number
        //      for this database (m_strMusicDBSerNum) so that we know when it changes and
        //      only update when there's new data.
        //
        //  m_ctarGlobalVars
        //      Global variables for any actions invoked by use on behalf of the user.
        //      They don't need sync since we are single threaded wrt to invoking user
        //      actions.
        //
        //  m_cuctxToUse
        //      We get a user name and password on the command line, a Limited type account,
        //      which gives us just enough rights to do the things we are configured to do.
        //
        //  m_fcolCapFlags
        //      Some flags we set while setting up, which can be used by nodes later to
        //      know if they can actually do anything or need to just tell the user that
        //      that isn't supported with their current configuration.
        //
        //  m_ippnListen
        //      The IP port on which we tell the Orb server to listen. If its not given
        //      on the command line, we use the default from the kCQCKit namespace.
        //
        //  m_mbufGrammar
        //      We download the grammar data from the MS and store it here for the Kinect
        //      callback thread to access. The grammar buffer needs sync between the BT
        //      thread that sets it and Kinect callback thread that reads it. We just use
        //      the event queue's mutex for this. We only lock it long enough to copy in or
        //      out the buffer data. The behavior tree stuff is just directly used upon
        //      download to set up the behavior tree, so it doesn't need to be kept around.
        //
        //  m_plgrLogSrv
        //      We install a log server logger. We don't own it after we've installed it,
        //      but we need to keep the pointer around so that we can force it back to
        //      local logging before we toast the client ORB support.
        //
        //  m_pscrmiOurs
        //      When we get the room config, we see if our configured room is there and
        //      store the pointer if so, else a null pointer.
        //
        //  m_pwndVisText
        //      If we are configured for visual text, we will create this window, else
        //      this is null.
        //
        //  m_randSel
        //      We need to do some random number selection. It's mutable because it's an
        //      internal detail and otherwise none of the reply speaking methods that use
        //      it could be const.
        //
        //  m_scfgCurrent
        //      We download the room configuration so that we can plug in the per-room
        //      stuff.
        //
        //  m_speechOut
        //      We provide a method for the behavior tree nodes to speak text back to
        //      the user. We create a speech object for that. This is only ever accessed
        //      by the behavior tree thread.
        //
        //  m_sprecoLocal
        //      This is the local speech recognition engine we are managing.
        //
        //  m_strMusicDBSerNum
        //      If music configured for our room, then this is the media database
        //      serial number, empty if we've not gotten the data yet. We use this to
        //      know when the data has changed.
        //
        //  m_strPassword
        //      The password we got on the command line
        //
        //  m_strRoom
        //      The name of the room this voice instance is configured for.
        //
        //  m_strUserName
        //      The user name we got on the command line
        //
        //  m_thrBTree
        //      We start a thread on one of our private methods to run the behavior tree.
        //      It is started up just before the main thread blocks on the waiting event.
        //      The root node will run until asked to stop. The handler method for this
        //      thread is in ThisFacility4.cpp.
        //
        //  m_wndMain
        //      Our main window, though it's only shown when the user toggles it visible
        //      by clicking on the tray monitor icon.
        //
        //  m_wndTray
        //      The tray app interface window. This one is never visible. It's just there
        //      to provide the interface to the system tray. The m_wndMain is the actual
        //      displayed window. But this is the main window.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bDebugOutput;
        tCIDLib::TBoolean       m_bDebugVerbose;
        tCIDLib::TBoolean       m_bReloadReq;
        tCIDLib::TBoolean       m_bSpeakReplies;
        tCIDLib::TBoolean       m_bTextReplies;
        TAIBehaviorTree         m_btreeLogic;
        tCIDLib::TCard4         m_c4GrammarSz;
        tCIDLib::TCard4         m_c4MaxClients;
        tCIDLib::TCard4         m_c4LastRemId;
        tCIDLib::TCard4         m_c4NextRemId;
        tCIDLib::TCard4         m_c4SysCfgSerNum;
        TReminderList           m_colReminders;
        TFacCQCMedia::TMDBPtr   m_cptrMusic;
        TStdVarsTar             m_ctarGlobalVars;
        TCQCUserCtx             m_cuctxToUse;
        tCQCVoice::EStates      m_eState;
        TCapFlagsList           m_fcolCapFlags;
        tCIDLib::TIPPortNum     m_ippnListen;
        THeapBuf                m_mbufGrammar;
        TLogSrvLogger*          m_plgrLogSrv;
        mutable TRandomNum      m_randSel;
        TCQCSysCfg              m_scfgCurrent;
        TSpeech                 m_speechOut;
        TCIDSpReco              m_sprecoLocal;
        const TCQCSysCfgRmInfo* m_pscrmiOurs;
        TCQCVoiceVisTextFrame*  m_pwndVisText;
        TString                 m_strMusicDBSerNum;
        TString                 m_strPassword;
        TString                 m_strRoom;
        TString                 m_strUserName;
        TThread                 m_thrBTree;
        TCQCVoiceFrameWnd       m_wndMain;
        TCQCVoiceTrayWnd        m_wndTray;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TFacCQCVoice,TGUIFacility)
};


// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceRuleJan
//  PREFIX: jan
// ---------------------------------------------------------------------------
class TCQCVoiceRuleJan
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCVoiceRuleJan() = delete;

        TCQCVoiceRuleJan
        (
            const   TString&                strRuleName
            , const tCIDLib::TBoolean       bNewState
        );

        TCQCVoiceRuleJan
        (
            const   TString&                strRulePref
            , const TString&                strRuleName
            , const tCIDLib::TBoolean       bNewState
        );

        TCQCVoiceRuleJan(const TCQCVoiceRuleJan&) = delete;
        TCQCVoiceRuleJan(TCQCVoiceRuleJan&&) = delete;

        ~TCQCVoiceRuleJan();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCVoiceRuleJan& operator=(const TCQCVoiceRuleJan&) = delete;
        TCQCVoiceRuleJan& operator=(TCQCVoiceRuleJan&&) = delete;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bNewState
        //      The new requested state. Upon dtor, we will set the state to the
        //      opposite of this. There's no way to get the current state of the
        //      rule, so we can't just insure that it gets back to the previous
        //      state no matter what.
        //
        //  m_strRuleName
        //      The name of the rule we are operating on. This is the prefix plus
        //      the rule name, which we set up in the ctor. If the ctor that doesn't
        //      take a prefix is called, we assume the on demand prefix.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bNewState;
        TString             m_strRuleName;
};

#pragma CIDLIB_POPPACK

