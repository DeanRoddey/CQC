//
// FILE NAME: CQSLSpeechS_DriverImpl.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/09/2006
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
//  This is the header file for the advanced speech driver. We use an
//  audio player engine to output files that are created by spitting out
//  text to speech to a WAV file. We maintain a simple playlist (a queue
//  really) of these and feed them to the audio player one at a time,
//  deleting them when they've been played.
//
//  We generate the files into a directory under a path:
//
//      [cqc]\CQCData\Server\Data\CQSLSpeech\[moniker]
//
//  So each instance of this driver has its own sub-directory. When the
//  driver comes up it deletes any existing files that might have gotten
//  stranded.
//
//  Note that the media player already has an internal player thread so
//  we don't have to spin up another one ourselves. We just have to poll
//  the player object and watch for end of media events, and queue up the
//  next file if we have one.
//
//  We also provide a means to pause the queue via a boolean field. When
//  this field is set, we'll stop spooling out the files, though we'll only
//  keep up to a certain number of files. After that, we start tossing out
//  the oldest one and putting the new one in.
//
//  We maintain a 'speech active' field that is true whenever speech is
//  being done. Note that it's set at the point where they write to our
//  SayText field, so that by the time they return from the field write, it
//  is already set and there's no worries of thinking it's already done
//  before it starts.
//
//
//  In order to allow the user to handle setting up for speech and cleaning
//  up after speech is done (i.e.to interrupt something, do speech, and come
//  back to it again), we will look for three global actions:
//
//      \User\Drivers\Speech\[moniker]\StartSpeech
//      \User\Drivers\Speech\[moniker]\NextSpeech
//      \User\Drivers\Speech\[moniker]\StopSpeech
//
//  If these are present we will run them when transitioning from having
//  nothing to speech to needing to start speaking, and vice versa. We load
//  them on our init, and won't go back and reload them unless the user writes
//  to our ReloadHelperActs field.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TCQSLSpeechSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TCQSLSpeechSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQSLSpeechSDriver() = delete;

        TCQSLSpeechSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TCQSLSpeechSDriver(const TCQSLSpeechSDriver&) = delete;
        TCQSLSpeechSDriver(TCQSLSpeechSDriver&&) = delete;

        ~TCQSLSpeechSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQSLSpeechSDriver& operator=(const TCQSLSpeechSDriver&) = delete;
        TCQSLSpeechSDriver& operator=(TCQSLSpeechSDriver&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmd
            , const TString&                strParms
        )   final;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        )   final;

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::ECommResults eBoolFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TBoolean       bNewValue
        )   final;

        tCQCKit::ECommResults eCardFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4NewValue
        )   final;

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::EDrvInitRes eInitializeImpl() final;

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        )   final;

        tCQCKit::ECommResults eStringFldValChanged
        (
            const   TString&                strName
            , const tCIDLib::TCard4         c4FldId
            , const TString&                strNewValue
        )   final;

        tCIDLib::TVoid ReleaseCommResource() final;

        tCIDLib::TVoid TerminateImpl() final;


    private :
        // -------------------------------------------------------------------
        //  Private class types
        // -------------------------------------------------------------------
        enum class EStates
        {
            Idle
            , Speaking
            , Next
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid DoEndAction();

        tCIDLib::TVoid DoNextAction
        (
            const   TString&                strExtraVal
        );

        tCIDLib::TVoid DoStartAction();

        tCIDLib::TVoid LoadGlobalActs();

        tCIDLib::TVoid QueueText
        (
            const   TString&                strText
            , const TString&                strExtraVal
        );

        tCIDLib::TVoid SetSpeechActiveFld
        (
            const   tCIDLib::TBoolean       bState
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_auplOutput
        //      This is our WMP player wrapper object that's used to do the
        //      output.
        //
        //  m_bGlobActEnd
        //  m_bGlobActNext
        //  m_bGlobActStart
        //      Indicates whether we found and loaded the start/end, and
        //      next global actions that we invoke. If we didn't find one,
        //      we don't try to invoke it.
        //
        //  m_c4FldId_XXX
        //      The ids for our fields that we look up for later quick access.
        //
        //  m_c4FileNameId
        //      We have to create a new file for each block of speech we render. So this is
        //      used to generate unique names. We flush the directory on startup, and start
        //      this at one and bump it for each new file, so we know we won't create any
        //      dups.
        //
        //  m_c4PollCnt
        //      Just bumped on each poll callback and used to do some things
        //      on a slower cycle than the actual poll callback period.
        //
        //  m_colWorkQ
        //      The work queue that we queue up the files to spool out. It
        //      doesn't need to be thread safe since it only gets accessed
        //      by either our poll callback or field writes, and these are
        //      serialized. That's the default so we just take the default
        //      ctor. It's a queue of key/value pairs. The key is the path
        //      to the pre-streamed out wav file to speak, and the value is
        //      an optional piece of text that will be passed to the 'next'
        //      global action if we invoke it.
        //
        //  m_csrcEnd
        //  m_csrcNext
        //  m_csrcStart
        //      These are the global actions that we will run when speech is
        //      about to start, when it ends, and before each new queued piece
        //      of text. The m_bGlobActXXX flags indicate whether we found these
        //      actions or not, and therefore whether we'll run them or not.
        //
        //  m_ctarGlobalVars
        //      Our global variables action target that we have to provide to
        //      the action engine to run the start/end speech global actions.
        //
        //  m_eCurState
        //      A little state machine to keep up with our current state. We
        //      are either idle (no speach going on), or speaking, or we have
        //      finished a currently outstanding file and are ready for the
        //      next (if any, else we go back to idle.)
        //
        //  m_enctNextLoad
        //      We want to avoid checking for new global action content rapidly
        //      if we get a series of speech events in a row, but to check often
        //      enough that changes are picked up before too long. So we use this
        //      timer to limit how often we'll check.
        //
        //  m_kvalCurFile
        //      The current file that we are playing. When it ends, we delete
        //      this file and try to start the next one. The key is the file
        //      path and the value is any extra value to be passed to the
        //      'next' global action.
        //
        //  m_strAudioDev
        //      The audio output device we were told to output on. We get
        //      this from a driver prompt.
        //
        //  m_strCmd_XXX
        //      The names of any backdoor commands we support. We set them
        //      up during ctor to keep around for later use.
        //
        //  m_strGlobActEndPath
        //  m_strGlobActNextPath
        //  m_strGlobActStartPath
        //      The path to the global actions that we will try to load. It's
        //      based partly on the moniker, so we pre-build them during init
        //      for later use.
        //
        //  m_strOutPath
        //      We pre-set up our output path during init since it never
        //      changes.
        //
        //  m_strVoice
        //      The voice that we were told to use, via a driver install
        //      prompt.
        // -------------------------------------------------------------------
        TCQCWMPAudioPlayer      m_auplOutput;
        tCIDLib::TBoolean       m_bGlobActEnd;
        tCIDLib::TBoolean       m_bGlobActNext;
        tCIDLib::TBoolean       m_bGlobActStart;
        tCIDLib::TBoolean       m_bRejectWhenFull;
        tCIDLib::TCard4         m_c4FldId_ClearQ;
        tCIDLib::TCard4         m_c4FldId_Mute;
        tCIDLib::TCard4         m_c4FldId_QPaused;
        tCIDLib::TCard4         m_c4FldId_ReloadGlobActs;
        tCIDLib::TCard4         m_c4FldId_SpeechActive;
        tCIDLib::TCard4         m_c4FldId_SayText;
        tCIDLib::TCard4         m_c4FldId_Volume;
        tCIDLib::TCard4         m_c4FileNameId;
        tCIDLib::TCard4         m_c4PollCnt;
        tCIDLib::TCard4         m_c4SerNum_End;
        tCIDLib::TCard4         m_c4SerNum_Next;
        tCIDLib::TCard4         m_c4SerNum_Start;
        tCIDLib::TKVPairQ       m_colWorkQ;
        TCQCStdCmdSrc           m_csrcEnd;
        TCQCStdCmdSrc           m_csrcNext;
        TCQCStdCmdSrc           m_csrcStart;
        TStdVarsTar             m_ctarGlobalVars;
        EStates                 m_eCurState;
        tCIDLib::TEncodedTime   m_enctNextLoad;
        TKeyValuePair           m_kvalCurFile;
        TString                 m_strAudioDev;
        const TString           m_strCmd_QueueText;
        TString                 m_strGlobActEndPath;
        TString                 m_strGlobActNextPath;
        TString                 m_strGlobActStartPath;
        TString                 m_strOutPath;
        TString                 m_strTmp;
        TString                 m_strVoice;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQSLSpeechSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK

