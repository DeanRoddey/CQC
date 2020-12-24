//
// FILE NAME: CQSLSpeechS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/19/2006
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
//  This is a test driver which implements driver interface for the driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQSLSpeechS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQSLSpeechSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQSLSpeechS_DriverImpl
{
}



// ---------------------------------------------------------------------------
//   CLASS: TCQSLSpeechSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQSLSpeechSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQSLSpeechSDriver::TCQSLSpeechSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_auplOutput(100)
    , m_bGlobActEnd(kCIDLib::False)
    , m_bGlobActNext(kCIDLib::False)
    , m_bGlobActStart(kCIDLib::False)
    , m_bRejectWhenFull(kCIDLib::False)
    , m_c4FldId_ClearQ(kCIDLib::c4MaxCard)
    , m_c4FldId_Mute(kCIDLib::c4MaxCard)
    , m_c4FldId_QPaused(kCIDLib::c4MaxCard)
    , m_c4FldId_ReloadGlobActs(kCIDLib::c4MaxCard)
    , m_c4FldId_SpeechActive(kCIDLib::c4MaxCard)
    , m_c4FldId_SayText(kCIDLib::c4MaxCard)
    , m_c4FldId_Volume(kCIDLib::c4MaxCard)
    , m_c4FileNameId(1)
    , m_c4PollCnt(0)
    , m_c4SerNum_End(0)
    , m_c4SerNum_Next(0)
    , m_c4SerNum_Start(0)
    , m_colWorkQ()
    , m_ctarGlobalVars(tCIDLib::EMTStates::Safe, kCIDLib::False)
    , m_eCurState(EStates::Idle)
    , m_enctNextLoad(0)
    , m_strCmd_QueueText(L"QText")
{
}

TCQSLSpeechSDriver::~TCQSLSpeechSDriver()
{
}


// ---------------------------------------------------------------------------
//  TCQSLSpeechSDriver: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  Handles a backdoor command to queue up text with an extra bit of info that
//  will be passed to the 'next' global action.
//
tCIDLib::TCard4
TCQSLSpeechSDriver::c4SendCmd(  const   TString&    strCmd
                                , const TString&    strParms)
{
    if (strCmd == m_strCmd_QueueText)
    {
        //
        //  There are two parameter values, in a quoted comma list format.
        //  So break them out into two separate values.
        //
        tCIDLib::TStrList   colParms;
        tCIDLib::TCard4     c4ErrIndex;

        if (!TStringTokenizer::bParseQuotedCommaList(strParms, colParms, c4ErrIndex)
        ||  (colParms.c4ElemCount() != 2))
        {
            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLSpErrs::errcCmd_BadQParms
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
            return 1;
        }

        // Do it if the work queue is not full
        if (!m_colWorkQ.bIsFull(kCQSLSpeechS::c4MaxQItems))
            QueueText(colParms[0], colParms[1]);
    }
    return 0;
}


// ---------------------------------------------------------------------------
//  TCQSLSpeechSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We just use this to allocate our WMA player object
//
tCIDLib::TBoolean TCQSLSpeechSDriver::bGetCommResource(TThread&)
{
    return kCIDLib::True;
}


tCIDLib::TBoolean TCQSLSpeechSDriver::bWaitConfig(TThread&)
{
    // We don't have any config other than the driver prompts
    return kCIDLib::True;
}


// A Boolean field changed
tCQCKit::ECommResults
TCQSLSpeechSDriver::eBoolFldValChanged( const   TString&
                                        , const tCIDLib::TCard4     c4FldId
                                        , const tCIDLib::TBoolean   bNewValue)
{
    if (c4FldId == m_c4FldId_ClearQ)
    {
        //
        //  Clear the queue. We don't update any active speech stuff since
        //  speech could still be going on until the current one ends. It
        //  also does not unpause the queue
        //
        m_colWorkQ.RemoveAll();
    }
     else if (c4FldId == m_c4FldId_Mute)
    {
        // Just pass it on the player
        m_auplOutput.bMute(bNewValue);
    }
     else if (c4FldId == m_c4FldId_QPaused)
    {
        //
        //  The value will be stored in the field and the poll callback
        //  will see this and stop outputting once the current event is
        //  done.
        //
        //  However, if unpausing, and the queue is not full, go ahead and
        //  set the speech active field now so that the caller can block
        //  on it immediately upon return without any race condition
        //  issues.
        //
        if (!bNewValue && !m_colWorkQ.bIsEmpty())
            SetSpeechActiveFld(kCIDLib::True);
    }
     else if (c4FldId == m_c4FldId_ReloadGlobActs)
    {
        // Zero out the next load timer, to force a check next time
        m_enctNextLoad = 0;
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


// A Cardinal field changed
tCQCKit::ECommResults
TCQSLSpeechSDriver::eCardFldValChanged( const   TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const tCIDLib::TCard4 c4NewValue)
{
    if (c4FldId == m_c4FldId_Volume)
    {
        // Just pass it on the to player
        m_auplOutput.c4Volume(c4NewValue);
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }
    return tCQCKit::ECommResults::Success;
}


tCQCKit::ECommResults TCQSLSpeechSDriver::eConnectToDevice(TThread&)
{
    //
    //  We don't actually have any connection, but reset the player and
    //  the playlist, store any current status to get us up to date,
    //  clear the metadata fields and update the item count and serial
    //  number fields.
    //
    //  We also make sure our output directory exists and created it
    //  if not. If it does create, we clean out any current files from it
    //  since they would have to be abandoned files.
    //
    try
    {
        m_auplOutput.Reset();

        // See if our output path exists
        if (TFileSys::bIsDirectory(m_strOutPath))
        {
            // Clean out any files that might have gotten left
            TFileSys::CleanPath(m_strOutPath, tCIDLib::ETreeDelModes::Clean);
        }
         else
        {
            // Create the path
            TFileSys::MakePath(m_strOutPath);
        }
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        return tCQCKit::ECommResults::LostConnection;
    }

    // Set the speech active and paused fields to false
    bStoreBoolFld(m_c4FldId_QPaused, kCIDLib::False, kCIDLib::True);
    bStoreBoolFld(m_c4FldId_SpeechActive, kCIDLib::False, kCIDLib::True);

    // Make sure the mute and volume fields are in sync with the player
    bStoreBoolFld(m_c4FldId_Mute, m_auplOutput.bMute(), kCIDLib::True);
    bStoreCardFld(m_c4FldId_Volume, m_auplOutput.c4Volume(), kCIDLib::True);

    //
    //  Clear the current file member since we aren't playing anything, and
    //  set the state to idle.
    //
    m_eCurState = EStates::Idle;
    m_kvalCurFile.Clear();

    // Clear our global variables target out
    m_ctarGlobalVars.DeleteAllVars();

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TCQSLSpeechSDriver::eInitializeImpl()
{
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();

    // We have to get a driver prompt for the output device
    if (!cqcdcTmp.bFindPromptValue(L"AudioDevice", L"DevName", m_strAudioDev))
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The audio device prompt was not set for driver %(1)"
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , strMoniker()
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  We may have a prompt for whether to reject new writes when the
    //  queue is full or not. It was added later so it might not be
    //  there. If not, we default it to true.
    //
    TString strVal;
    if (cqcdcTmp.bFindPromptValue(L"RejectFull", L"State", strVal))
        m_bRejectWhenFull = facCQCKit().bCheckBoolVal(strVal);
    else
        m_bRejectWhenFull = kCIDLib::True;

    // Try to initialize our player object
    try
    {
        m_auplOutput.Initialize();
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLSpErrs::errcAudio_InitPlayer
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return tCQCKit::EDrvInitRes::Failed;
    }

    //
    //  We may get a prompt for the voice to use. It was added after the
    //  fact so it may not be present for already installed drivers.
    //
    if (!cqcdcTmp.bFindPromptValue(L"Voice", L"VoiceName", m_strVoice))
        m_strVoice.Clear();

    // We can set up all our fields now since they never change
    TVector<TCQCFldDef> colFlds(8);
    TCQCFldDef          flddNew;

    flddNew.Set
    (
        L"ClearQ"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"Mute"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"QPaused"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"ReloadGlobActs"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Write
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"SpeechActive"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::Read
    );
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"SayText"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
    );
    flddNew.bAlwaysWrite(kCIDLib::True);
    colFlds.objAdd(flddNew);

    flddNew.Set
    (
        L"Volume"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , L"Range:0, 100"
    );
    colFlds.objAdd(flddNew);

    // Set the fields and go back and look up them by id
    SetFields(colFlds);
    m_c4FldId_ClearQ = pflddFind(L"ClearQ", kCIDLib::True)->c4Id();
    m_c4FldId_Mute = pflddFind(L"Mute", kCIDLib::True)->c4Id();
    m_c4FldId_QPaused = pflddFind(L"QPaused", kCIDLib::True)->c4Id();
    m_c4FldId_ReloadGlobActs = pflddFind(L"ReloadGlobActs", kCIDLib::True)->c4Id();
    m_c4FldId_SpeechActive = pflddFind(L"SpeechActive", kCIDLib::True)->c4Id();
    m_c4FldId_SayText = pflddFind(L"SayText", kCIDLib::True)->c4Id();
    m_c4FldId_Volume = pflddFind(L"Volume", kCIDLib::True)->c4Id();

    //
    //  Set our poll time pretty fast so that we can react quickly to
    //  end of media events from the player object.
    //
    SetPollTimes(50, 5000);

    // Set up our output path
    TPathStr pathOut(facCQCKit().strServerDataDir());
    pathOut.AddLevel(L"CQSLSpeech");
    pathOut.AddLevel(strMoniker());
    m_strOutPath = pathOut;

    // Reset our poll counter to zero
    m_c4PollCnt = 0;

    // Build the two global action paths
    m_strGlobActEndPath = L"\\User\\Drivers\\Speech\\%(m)\\StopSpeech";
    m_strGlobActEndPath.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);

    m_strGlobActNextPath = L"\\User\\Drivers\\Speech\\%(m)\\NextSpeech";
    m_strGlobActNextPath.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);

    m_strGlobActStartPath = L"\\User\\Drivers\\Speech\\%(m)\\StartSpeech";
    m_strGlobActStartPath.eReplaceToken(strMoniker(), kCIDLib::chLatin_m);

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TCQSLSpeechSDriver::ePollDevice(TThread&)
{
    // Bump our poll counter
    m_c4PollCnt++;

    //
    // Based on our current state, do what we need to do
    try
    {
        //
        //  We should never see 'next' state here. That's a transitional
        //  state only used below in this method. So, if we see it, just
        //  for safety put us back into speaking state, so that we'll
        //  recover in a reasoanbly failsafe way.
        //
        if (m_eCurState == EStates::Next)
            m_eCurState = EStates::Speaking;

        //
        //  First see if we are speaking. If so, we need to check the status
        //  of that and see if we are done. We do this first so that, if we
        //  are done, then the same code below can start a new one whether
        //  we were currently active or in idle state.
        //
        if (m_eCurState == EStates::Speaking)
        {
            tCQCMedia::EEndStates   eEndState;
            tCQCMedia::EMediaStates eState;
            tCIDLib::TEncodedTime   enctPlayPos = 0;
            m_auplOutput.GetStatusInfo(eState, eEndState, enctPlayPos);

            // If no longer active, then the current file is done or failed
            if ((eEndState == tCQCMedia::EEndStates::Ended)
            ||  (eEndState == tCQCMedia::EEndStates::Failed))
            {
                // Reset our status now to Next mode
                m_eCurState = EStates::Next;

                // Reset the audio player so it'll let the file go
                m_auplOutput.Reset();

                // Delete the current file
                try
                {
                    TFileSys::DeleteFile(m_kvalCurFile.strKey());
                }

                catch(TError& errToCatch)
                {
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    facCQSLSpeechS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQSLSpErrs::errcSpeech_DelFile
                        , m_kvalCurFile.strKey()
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );
                }

                // Clear the current file indicator now
                m_kvalCurFile.Clear();
            }
        }

        // If not still in speaking mode, see if there's anything else to do
        if (m_eCurState != EStates::Speaking)
        {
            // Get the value of the queue paused field for later use
            tCIDLib::TBoolean bPaused = kCIDLib::False;
            tCIDLib::TCard4 c4SerNum = 0;
            bReadBoolFld(m_c4FldId_QPaused, c4SerNum, bPaused);

            //
            //  If not paused, do a zero timeout read on the queue to see
            //  if we have more to do. If we are paused, we just act like
            //  there isn't anything to do.
            //
            //  !!!!Check the paused FIRST, so we don't eat and lose a
            //  queue entry if we end up being paused.
            //
            if (!bPaused && m_colWorkQ.bGetNext(m_kvalCurFile, 0))
            {
                //
                //  If we are coming from idle, we have to do do a couple
                //  things.
                //
                if (m_eCurState == EStates::Idle)
                {
                    // Make sure we have the latest global actions
                    LoadGlobalActs();

                    // And we need to run the Start action if we have one
                    DoStartAction();
                }

                //
                //  If we have the 'next' global action, invoke it, passing
                //  on the extra queued value if any.
                //
                DoNextAction(m_kvalCurFile.strValue());

                //
                //  And now try to start playing the file. Set the state to
                //  speaking first.
                //
                m_eCurState = EStates::Speaking;
                try
                {
                    m_auplOutput.LoadFile(m_kvalCurFile.strKey(), m_strAudioDev);

                    //
                    //  It started, so set our speech active field if not
                    //  already.
                    //
                    SetSpeechActiveFld(kCIDLib::True);
                }

                catch(TError& errToCatch)
                {
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    facCQSLSpeechS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQSLSpErrs::errcAudio_StartPlay
                        , m_kvalCurFile.strKey()
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::CantDo
                    );

                    //
                    //  Just fall through and we'll clean this file up
                    //  naturally the next time around and try the next one.
                    //
                }
            }
             else
            {
                //
                //  Nothing to do, make sure our active field is cleared.
                //  If we are in Next state, then we need to call the end
                //  action, then reset the state to idle.
                //
                if (m_eCurState == EStates::Next)
                    DoEndAction();

                m_eCurState = EStates::Idle;
                SetSpeechActiveFld(kCIDLib::False);
            }
        }

        // Once a half second or so, update our mute and volume
        if (!(m_c4PollCnt % 10))
        {
            bStoreBoolFld(m_c4FldId_Mute, m_auplOutput.bMute(), kCIDLib::True);
            bStoreCardFld(m_c4FldId_Volume, m_auplOutput.c4Volume(), kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLSpErrs::errcSpeech_PollErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }

    }

    return tCQCKit::ECommResults::Success;
}


// One of our boolean fields (a contact closure) was written to
tCQCKit::ECommResults
TCQSLSpeechSDriver::eStringFldValChanged(const  TString&
                                        , const tCIDLib::TCard4 c4FldId
                                        , const TString&        strNewValue)
{
    if (c4FldId == m_c4FldId_SayText)
    {
        //
        //  If the queue is full, we have to ignore this one since we
        //  can't queue up any more. If we are configured to do so, we
        //  can reject the field write so they'll know.
        //
        if (m_colWorkQ.bIsFull(kCQSLSpeechS::c4MaxQItems))
        {
            //
            //  If configured to do so, reject the value, otherwise just
            //  eat it and log a message that we dropped one.
            //
            if (m_bRejectWhenFull)
                return tCQCKit::ECommResults::Full;
        }
         else
        {
            // Looks ok, so queue up the new value
            QueueText(strNewValue, TString::strEmpty());
        }
    }
     else
    {
        IncUnknownWriteCounter();
        return tCQCKit::ECommResults::FieldNotFound;
    }

    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TCQSLSpeechSDriver::ReleaseCommResource()
{
    // Clean up the player object and make sure it's stopped
    try
    {
        m_auplOutput.Terminate();
    }

    catch(const TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
    }
}


tCIDLib::TVoid TCQSLSpeechSDriver::TerminateImpl()
{
    // Nothing to do
}


// ---------------------------------------------------------------------------
//  TCQSLSpeechSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called when we go back to idle state, i.e. nothing more for now to speak.
//  We invoke the end global action if we have one.
//
tCIDLib::TVoid TCQSLSpeechSDriver::DoEndAction()
{
    if (m_bGlobActEnd)
    {
        try
        {
            tCQCKit::TCmdTarList colEmpty(tCIDLib::EAdoptOpts::NoAdopt);
            TCQCStdActEngine acteGlobal(cuctxDrv());
            acteGlobal.eInvokeOps
            (
                m_csrcEnd
                , kCQCKit::strEvId_OnTrigger
                , m_ctarGlobalVars
                , colEmpty
                , 0
                , TString::strEmpty()
            );
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged()
            &&  (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  Called each time a new piece of queued text is to be spoken, if we loaded
//  a 'next' global action.
//
tCIDLib::TVoid TCQSLSpeechSDriver::DoNextAction(const TString& strExtraVal)
{
    if (m_bGlobActNext)
    {
        try
        {
            // Set up a single action parameter with the passed extra value
            TString strParms;
            TStringTokenizer::BuildQuotedCommaList(strExtraVal, strParms);

            tCQCKit::TCmdTarList colEmpty(tCIDLib::EAdoptOpts::NoAdopt);
            TCQCStdActEngine acteGlobal(cuctxDrv());
            acteGlobal.eInvokeOps
            (
                m_csrcNext
                , kCQCKit::strEvId_OnTrigger
                , m_ctarGlobalVars
                , colEmpty
                , 0
                , strParms
            );
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged()
            &&  (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  Called when we go from idle to speaking state. We invoke the start global
//  action if we have one.
//
tCIDLib::TVoid TCQSLSpeechSDriver::DoStartAction()
{
    // If we have a start action, then run it
    if (m_bGlobActStart)
    {
        try
        {
            tCQCKit::TCmdTarList colEmpty(tCIDLib::EAdoptOpts::NoAdopt);
            TCQCStdActEngine acteGlobal(cuctxDrv());
            acteGlobal.eInvokeOps
            (
                m_csrcStart
                , kCQCKit::strEvId_OnTrigger
                , m_ctarGlobalVars
                , colEmpty
                , 0
                , TString::strEmpty()
            );
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged()
            &&  (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }
}


//
//  This is called periodically to see if we need to reload our actions. We keep
//  the serial number of each global action, so the server can tell us if we need
//  to download them again or not.
//
//  Any that fail we force their serial numbers back to zero, to force a load
//  next time.
//
tCIDLib::TVoid TCQSLSpeechSDriver::LoadGlobalActs()
{
    //
    //  See if it's time to check. If we haven't hit the next load time, then just
    //  return.
    //
    if (TTime::enctNow() < m_enctNextLoad)
        return;

    // Reset the time stamp before do anything else to insure it gets set
    m_enctNextLoad = TTime::enctNowPlusSecs(60);

    // Flush any global variables just to clean up
    m_ctarGlobalVars.DeleteAllVars();

    //
    //  Clear the loaded flags so we only need to set the ones loaded (or don't
    //  need to be reloaded) below.
    //
    m_bGlobActEnd = kCIDLib::False;
    m_bGlobActNext = kCIDLib::False;
    m_bGlobActStart = kCIDLib::False;
    tCIDLib::TEncodedTime enctLastChange;
    try
    {
        TDataSrvClient      dsclAct;
        tCIDLib::TKVPFList  colMeta;
        try
        {
            if (!m_strGlobActStartPath.bIsEmpty())
            {
                dsclAct.bReadGlobalAction
                (
                    m_strGlobActStartPath
                    , m_c4SerNum_Start
                    , enctLastChange
                    , m_csrcStart
                    , colMeta
                    , cuctxDrv().sectUser()
                );
            }

            // No exception, so we got it, or already had it
            m_bGlobActStart = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            m_bGlobActStart = kCIDLib::False;
            m_c4SerNum_Start = 0;

            if (!errToCatch.bLogged() && (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

        try
        {
            if (!m_strGlobActEndPath.bIsEmpty())
            {
                 dsclAct.bReadGlobalAction
                (
                    m_strGlobActEndPath
                    , m_c4SerNum_End
                    , enctLastChange
                    , m_csrcEnd
                    , colMeta
                    , cuctxDrv().sectUser()
                );
            }

            // No exception, so we got it, or already had it
            m_bGlobActEnd = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            m_bGlobActEnd = kCIDLib::False;
            m_c4SerNum_End = 0;

            if (!errToCatch.bLogged() && (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }


        try
        {
            if (!m_strGlobActNextPath.bIsEmpty())
            {
                 dsclAct.bReadGlobalAction
                (
                    m_strGlobActNextPath
                    , m_c4SerNum_Next
                    , enctLastChange
                    , m_csrcNext
                    , colMeta
                    , cuctxDrv().sectUser()
                );
            }

            // No exception, so we got it, or already had it
            m_bGlobActNext = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            m_bGlobActNext = kCIDLib::False;
            m_c4SerNum_Next = 0;

            if (!errToCatch.bLogged() && (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }

    }

    catch(TError& errToCatch)
    {
        // Not much we can do, keep the ones we have managed to get
        if (!errToCatch.bLogged() && (eVerboseLevel() > tCQCKit::EVerboseLvls::Low))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  This is called to do the grunt work of generating a new file and queuing
//  it up to be processed. We can get an extra value that will be queued up
//  along with the text, to be passed to the 'next' global action if it is
//  available.
//
tCIDLib::TVoid
TCQSLSpeechSDriver::QueueText(const TString& strText, const TString& strExtraVal)
{
    // Create a unique file name in our output path
    TPathStr pathFlName(m_strOutPath.c4Length() + 32UL);
    pathFlName = m_strOutPath;
    pathFlName.AddLevel(L"CQSLS_TTSData");
    pathFlName.AppendFormatted(m_c4FileNameId++);
    pathFlName.AppendExt(L"wav");

    // And render the text to that
    try
    {
        TSpeech::SpeakToFile
        (
            strText
            , pathFlName
            , tCIDLib::EAudioFmts::F16Khz16BitMono
            , m_strVoice
        );
    }

    catch(TError& errToCatch)
    {
        //
        //  It failed so log it if needed and make sure that the file
        //  gets cleaned up
        //
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLSpErrs::errcSpeech_Failed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }

        try
        {
            if (TFileSys::bExists(pathFlName))
                TFileSys::DeleteFile(pathFlName);
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            facCQSLSpeechS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQSLSpErrs::errcSpeech_CleanupErr
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
            );
        }
        return;
    }

    //
    //  Looks ok, so queue up the file name for processing, and go ahead
    //  and set the speech active field now so that the caller can immediately
    //  block on the active speech field.
    //
    //  However, if the queue is paused, we don't do this, so we read the
    //  value of the paused field to see if we need to set the active field.
    //
    m_colWorkQ.objAdd(TKeyValuePair(pathFlName, strExtraVal));

    tCIDLib::TBoolean bPaused;
    {
        tCIDLib::TCard4 c4SerNum = 0;
        bReadBoolFld(m_c4FldId_QPaused, c4SerNum, bPaused);
    }

    if (!bPaused)
        SetSpeechActiveFld(kCIDLib::True);
}


//
//  This separated out just in case we eventually want to do something else
//  any time the active field is changed. But for now it just sets the field.
//
tCIDLib::TVoid
TCQSLSpeechSDriver::SetSpeechActiveFld(const tCIDLib::TBoolean bState)
{
    bStoreBoolFld(m_c4FldId_SpeechActive, bState, kCIDLib::True);
}

