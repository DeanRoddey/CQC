//
// FILE NAME: CQCVoice_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2016
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
//  This file implements the facility class for the CQC Voice server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"
#include    "CQCVoice_BTNodeFactory.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCVoice,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TCQCVoiceRuleJan
//  PREFIX: jan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVoiceRuleJan: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  In this one, there is no rule name prefix, so we assume it's an on demand rule
//  which is almost always what this class is used to enable/disable on a scoped
//  basis. This greatly reduces verbiage.
//
TCQCVoiceRuleJan::TCQCVoiceRuleJan( const   TString&            strRuleName
                                    , const tCIDLib::TBoolean   bNewState) :

    m_bNewState(bNewState)
    , m_strRuleName(TString::strConcat(kCQCVoice::strVal_OnDemPrefix, strRuleName))
{
    // Set the rule to the new state (use the MEMBER here, not the parameter)
//    facCQCVoice.SetRuleState(m_strRuleName, bNewState);
}

//
//  If not an on-demand rule, they can provide an explicit prefix. We take the rule
//  name separately because it's always actually the action semantic value (which has
//  the value of the root part of the rule name (without any prefixes.) Otherwise we
//  would have to keep around unprefixed, and multiple prefixed versions of most of
//  the rule names.
//
TCQCVoiceRuleJan::TCQCVoiceRuleJan( const   TString&            strRulePref
                                    , const TString&            strRuleName
                                    , const tCIDLib::TBoolean   bNewState) :

    m_bNewState(bNewState)
    , m_strRuleName(TString::strConcat(strRulePref, strRuleName))
{
    // Set the rule to the new state (use the MEMBER here, not the parameter)
//    facCQCVoice.SetRuleState(m_strRuleName, bNewState);
}

TCQCVoiceRuleJan::~TCQCVoiceRuleJan()
{
    // Put the old state back
    try
    {
//        facCQCVoice.SetRuleState(m_strRuleName, !m_bNewState);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCVoice
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCVoice: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCVoice::TFacCQCVoice() :

    TGUIFacility
    (
        L"CQCVoice"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_bDebugOutput(kCIDLib::False)
    , m_bDebugVerbose(kCIDLib::False)
    , m_bReloadReq(kCIDLib::False)
    , m_bSpeakReplies(kCIDLib::False)
    , m_bTextReplies(kCIDLib::False)
    , m_btreeLogic(L"CQCVoice")
    , m_c4GrammarSz(0)
    , m_c4LastRemId(0)
    , m_c4MaxClients(0)
    , m_c4NextRemId(1)
    , m_c4SysCfgSerNum(0)
    , m_colReminders(128)
    , m_ctarGlobalVars(tCIDLib::EMTStates::Unsafe, kCIDLib::False)
    , m_eState(tCQCVoice::EStates::WaitLogin)
    , m_fcolCapFlags(tCQCVoice::ECapFlags::Count)
    , m_ippnListen(kCQCKit::ippnCQCVoiceSrvDefPort)
    , m_pscrmiOurs(nullptr)
    , m_pwndVisText(nullptr)
    , m_sprecoLocal(L"CQCVoiceSpReco")
    , m_thrBTree
      (
          facCIDLib().strNextThreadName(L"CQCVoice_BTree")
          , TMemberFunc<TFacCQCVoice>(this, &TFacCQCVoice::eBTThread)
      )
{
    // Seed our random number generator
    m_randSel.Seed(TTime::c4Millis());

    // If debug mode, then default debug output to true
    #if CID_DEBUG_ON
    m_bDebugOutput = kCIDLib::True;
    #endif
}

TFacCQCVoice::~TFacCQCVoice()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCVoice: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is called by our ORB based admin interface handler. We just exit the msg
//  loop with the code he provides.
//
tCIDLib::TVoid TFacCQCVoice::Shutdown(const tCIDLib::EExitCodes eExit)
{
    facCIDCtrls().ExitLoop(eExit);
}


// ---------------------------------------------------------------------------
//  TFacCQCVoice: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Just adds the passed reminder to our list, if it's not full
tCIDLib::TVoid
TFacCQCVoice::AddReminder( const   TString&        strText
                            , const tCIDLib::TCard4 c4Minutes)
{
    if (m_colReminders.bIsFull(1024))
    {
        SpeakReply(kCQCVoiceMsgs::midSpeak_ReminderQFull);
        return;
    }

    // Create a new reminder and assign it the next available id
    TCQCVoiceReminder cvremNew(strText, c4Minutes, m_c4NextRemId++);

    // It'll never happen, but be technically correct. Zero isn't a valid id
    if (!m_c4NextRemId)
        m_c4NextRemId++;

    // Save the the id as the last added reminder
    m_c4LastRemId = cvremNew.c4UniqueId();

    // And now insert it sorted by its time
    tCIDLib::TCard4 c4InsertedAt;
    m_colReminders.InsertSorted
    (
        cvremNew, TCQCVoiceReminder::eCompByTime, c4InsertedAt
    );

    //
    //  Announce that we've added it and the time we'll remind. So we need to
    //  format it out. The text has a say-as markup, which expects it in 24
    //  hour format.
    //
    TString strTime;
    cvremNew.FormatTime(strTime);

    if (TLocale::b24Hour())
        SpeakReply(kCQCVoiceMsgs::midSpeakP_WillRemind24, strTime, kCIDLib::True);
    else
        SpeakReply(kCQCVoiceMsgs::midSpeakP_WillRemind12, strTime, kCIDLib::True);
}


//
//  Checks that all the values in the passed semanic value list are at the
//  indicated confidence or more
//
tCIDLib::TBoolean
TFacCQCVoice::bAllAtConfidence(const   tCIDSpReco::TSemList&   colSemVals
                                , const tCQCVoice::EConfLevels eLevel) const
{
    const tCIDLib::TCard4 c4Count = colSemVals.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (!bIsAtConfidence(colSemVals[c4Index], eLevel))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Return the value of the indicated capabilities flag
tCIDLib::TBoolean TFacCQCVoice::bCapFlag(const tCQCVoice::ECapFlags eFlag) const
{
    return m_fcolCapFlags[eFlag];
}


//
//  Checks to see if we have room information. If not, we tell the user and returns
//  false. Then checks to see if a thermostat is configured for our room. If not
//  tells the user and returns false. Else, gives the user the configured thermo
//  moniker/sub-unit info.
//
tCIDLib::TBoolean
TFacCQCVoice::bCheckRoomThermo(TString& strMonToFill, TString& strSubToFill) const
{
    // If we have no room, then obviously not
    if (!m_pscrmiOurs)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoRoomData, m_strRoom);
        return kCIDLib::False;
    }

    // Get the function id for our room for HVAC
    const TString& strHVACId = m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::HVAC);
    if (strHVACId.bIsEmpty())
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_NoRoomData2, kCQCVoice::strVal_Thermostat, m_strRoom
        );
        return kCIDLib::False;
    }

    tCIDLib::TCard4 c4At;
    const TSCHVACInfo& scliOurs = m_scfgCurrent.scliHVACByUID(strHVACId, c4At);

    strMonToFill = scliOurs.m_strMoniker;
    strSubToFill = scliOurs.m_strThermoSub;
    return kCIDLib::True;
}


// Tells whether to do debug window output or not
tCIDLib::TBoolean TFacCQCVoice::bDebugOutput() const
{
    return m_bDebugOutput;
}

tCIDLib::TBoolean TFacCQCVoice::bDebugOutput(const tCIDLib::TBoolean bToSet)
{
    m_bDebugOutput = bToSet;
    return m_bDebugOutput;
}


// Control whether debug output only shows accepted events or all events
tCIDLib::TBoolean TFacCQCVoice::bDebugVerbose() const
{
    return m_bDebugVerbose;
}

tCIDLib::TBoolean TFacCQCVoice::bDebugVerbose(const tCIDLib::TBoolean bToSet)
{
    m_bDebugVerbose = bToSet;
    return m_bDebugVerbose;
}


//
//  Checks to see if we have room information. If not, we tell the user and returns
//  false. Then checks to see if a weather source is configured for our room. If not
//  tells the user and returns false. Else, gives the user the configured weather
//  moniker.
//
tCIDLib::TBoolean TFacCQCVoice::bCheckRoomWeather(TString& strToFill) const
{
    // If we have no room, then obviously not
    if (!m_pscrmiOurs)
    {
        facCQCVoice.SpeakReply(kCQCVoiceMsgs::midErr_NoRoomData, m_strRoom);
        return kCIDLib::False;
    }

    // Get the function id for our room for weather
    const TString& strWeathId = m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::Weather);
    if (strWeathId.bIsEmpty())
    {
        facCQCVoice.SpeakReply
        (
            kCQCVoiceMsgs::midErr_NoRoomData2, kCQCVoice::strVal_Weather, m_strRoom
        );
        return kCIDLib::False;
    }

    tCIDLib::TCard4 c4At;
    const TSCWeatherInfo& scliOurs = m_scfgCurrent.scliWeatherByUID(strWeathId, c4At);

    strToFill = scliOurs.m_strMoniker;
    return kCIDLib::True;
}



//
//  This allows behavior nodes to get speech input. It will wait up to the indicated time
//  for a new speech event to become available.
//
//  If we get one, we will check it for obvious issues and handle those here so that other
//  code that needs to handle input doesn't have to deal with it. We return false if we get
//  any of those problems.
//
//  They can tell us to ignore low confidence events, or to return them for the caller
//  to respond to.
//
tCIDLib::TBoolean
TFacCQCVoice::bGetNextEvent(       TCIDSpeechRecoEv&   sprecevToFill
                            ,       TString&            strAction
                            , const tCIDLib::TCard4     c4WaitMillis
                            , const tCIDLib::TBoolean   bWantLowConf)
{
    //
    //  We don't want to do one long wait, since they can be sort of long and we need
    //  to be reponsive to shutdowns. So we loop and wait a bit at a time until we run
    //  out or get something.
    //
    tCIDLib::TBoolean bGotOne = kCIDLib::False;
    tCIDLib::TCard4 c4CurTime = 0;
    TThread* pthrUs = TThread::pthrCaller();
    while ((c4CurTime < c4WaitMillis) && !pthrUs->bCheckShutdownRequest())
    {
        tCIDAudStream::EStrmStates eState;
        bGotOne = m_sprecoLocal.bGetNextRecEvent(sprecevToFill, 250, eState);
        if (bGotOne)
        {
            //
            //  We need to do some pre-processing so that we can update some of the semantic
            //  values with the confidence values of the rules that drove them, and force
            //  the others to 100% because those are ones that were just set as literals.
            //


            //
            //  If debugging and we are showing verbose output, dump it out. We have to
            //  format it out to text and then we can post it to the main window.
            //
            if (m_bDebugOutput && m_bDebugVerbose)
            {
                TTextStringOutStream strmOut(256UL);
                strmOut << sprecevToFill << kCIDLib::EndLn;
                m_wndMain.QueueMsg(strmOut.strData());
            }

            // It has to have an action semantic value
            const TKeyNumPair* pknumAction = sprecevToFill.pknumFindSemVal
            (
                kCQCVoice::strSemVal_Action
            );

            if (pknumAction)
            {
                // If they want low confidence events or it's high confidence take it
                if (bWantLowConf
                || (sprecevToFill.f4RuleConfidence() >= kCQCVoice::f4HighConfidence))
                {
                    strAction = pknumAction->strValue();
                    break;
                }

                // Reset the flag since we aren't going to react to this one
                bGotOne = kCIDLib::False;
            }
             else
            {
                // Reset the flag since we aren't going to react to this one
                bGotOne = kCIDLib::False;
                if (m_bDebugOutput)
                {
                    m_wndMain.QueueMsg
                    (
                        L"There is no semantic information, please contact Charmed Quark"
                    );
                }
            }
        }

        c4CurTime += 250;
    }

    // If we got something, then let's process it
    if (bGotOne)
    {
        // If debugging and we didn't dump it in verbose mode above, then dump it now
        if (m_bDebugOutput && !m_bDebugVerbose)
        {
            TTextStringOutStream strmOut(256UL);
            strmOut << sprecevToFill << kCIDLib::EndLn;
            m_wndMain.QueueMsg(strmOut.strData());
        }

        // It has to have an action semantic value
        const TKeyNumPair* pknumAction = sprecevToFill.pknumFindSemVal(kCQCVoice::strSemVal_Action);
        if (!pknumAction)
        {
            facCQCVoice.SpeakReply
            (
                L"There is no semantic information, please contact Charmed Quark"
            );
            return kCIDLib::False;
        }

        // We got it
        return kCIDLib::True;
    }

    return kCIDLib::False;
}


//
//  Tests the passed rule info for a given confidence. One takes the information
//  directly, and another takes the behavior tree and the path of the rule and looks it
//  up. Failure to find the rule causes an exception.
//
//  We have some convenience ones that check for a specific level, to keep the verbiage
//  down, since these are used a lot.
//
tCIDLib::TBoolean
TFacCQCVoice::bIsAtConfidence( const   TKeyNumPair&            knumToTest
                                , const tCQCVoice::EConfLevels eLevel) const
{
    const tCIDLib::TFloat4 f4Val = knumToTest.f4Value();

    // Else we have the more useful numeric one to use
    if (eLevel >= tCQCVoice::EConfLevels::Full)
        return (f4Val >= kCQCVoice::f4FullConfidence);
    else if (eLevel >= tCQCVoice::EConfLevels::High)
        return (f4Val >= kCQCVoice::f4HighConfidence);
    else if (eLevel >= tCQCVoice::EConfLevels::Medium)
        return (f4Val >= kCQCVoice::f4MedConfidence);

    // Anything else would have to be OK
    return kCIDLib::True;
}

tCIDLib::TBoolean
TFacCQCVoice::bIsAtConfidence( const   TAIBehaviorTree&        btreeOwner
                                , const TString&                strRulePath
                                , const tCQCVoice::EConfLevels eLevel) const
{
    const tCIDLib::TKNumList& colRules = btreeOwner.ctxToUse().colKNumList();
    const tCIDLib::TCard4 c4Count = colRules.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyNumPair& knumCur = colRules[c4Index];
        if (knumCur.strKey().bCompare(strRulePath))
            return bIsAtConfidence(knumCur, eLevel);
    }

    facCQCVoice.ThrowErr
    (
        CID_FILE
        , CID_LINE
        , kCQCVoiceErrs::errcInp_RuleNotFound
        , tCIDLib::ESeverities::Failed
        , tCIDLib::EErrClasses::NotFound
        , strRulePath
    );

    // Make the compiler happy
    return kCIDLib::False;
}


tCIDLib::TBoolean
TFacCQCVoice::bIsFullConfidence(const TKeyNumPair& knumToTest) const
{
    return bIsAtConfidence(knumToTest, tCQCVoice::EConfLevels::Full);
}

tCIDLib::TBoolean
TFacCQCVoice::bIsFullConfidence(const  TAIBehaviorTree&    btreeOwner
                                , const TString&            strRulePath) const
{
    return bIsAtConfidence(btreeOwner, strRulePath, tCQCVoice::EConfLevels::Full);
}


tCIDLib::TBoolean
TFacCQCVoice::bIsHighConfidence(const TKeyNumPair& knumToTest) const
{
    return bIsAtConfidence(knumToTest, tCQCVoice::EConfLevels::High);
}

tCIDLib::TBoolean
TFacCQCVoice::bIsHighConfidence(const  TAIBehaviorTree&        btreeOwner
                                , const TString&                strRulePath) const
{
    return bIsAtConfidence(btreeOwner, strRulePath, tCQCVoice::EConfLevels::High);
}


tCIDLib::TBoolean
TFacCQCVoice::bIsMedConfidence(const TKeyNumPair& knumToTest) const
{
    return bIsAtConfidence(knumToTest, tCQCVoice::EConfLevels::Medium);
}

tCIDLib::TBoolean
TFacCQCVoice::bIsMedConfidence(const   TAIBehaviorTree&    btreeOwner
                                , const TString&            strRulePath) const
{
    return bIsAtConfidence(btreeOwner, strRulePath, tCQCVoice::EConfLevels::Medium);
}


//
//  Helpers to do range checking on various types of fields. This will be pretty
//  commomly needed, so best to not replicate it all over. If it's not valid we tell
//  the user the valid range and return false.
//
tCIDLib::TBoolean
TFacCQCVoice::bRangeCheckIntFld(       tCQCKit::TCQCSrvProxy&  orbcDrv
                                , const TString&                strMoniker
                                , const TString&                strField
                                , const tCIDLib::TInt4          i4TestVal)
{
    try
    {
        TCQCFldDef flddTest;
        orbcDrv->QueryFieldDef(strMoniker, strField, flddTest);

        // It should be a numeric type
        CIDAssert(flddTest.bIsNumericType(), L"The field to range check is not numeric");

        // See if it has a range limit
        TCQCFldLimit* pfldlTest = TCQCFldLimit::pfldlMakeNew(flddTest);

        // If not limit, then clearly it's valid, though wierd that we'd be called
        if (!pfldlTest)
            return kCIDLib::True;

        // Put a janitor on it so it gets cleaned up
        TJanitor<TCQCFldLimit> janLimit(pfldlTest);

        // It should have an integer limit of some sort
        CIDAssert
        (
            pfldlTest->bIsDescendantOf(TCQCFldIntLimit::clsThis())
            , L"The field does not have an integer range limit"
        );

        // Cast it to its actual type
        TCQCFldIntLimit* pfldlInt = static_cast<TCQCFldIntLimit*>(pfldlTest);

        // And check it. If not good, then tell the user and return failure
        if (pfldlInt->eValidate(i4TestVal) != tCQCKit::EValResults::OK)
        {
            SpeakReply
            (
                kCQCVoiceMsgs::midErr_RangeLimit
                , TInteger(pfldlInt->i4Min())
                , TInteger(pfldlInt->i4Max())
            );
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        SpeakReply(kCQCVoiceMsgs::midErr_GetFldDef);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


//
//  Search the current list of active reminders to see if this one exists. If so,
//  return true and how many instances exist.
//
tCIDLib::TBoolean
TFacCQCVoice::bReminderExists(const TString& strToCheck, tCIDLib::TCard4& c4FndCount) const
{
    c4FndCount = 0;
    const tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colReminders[c4Index].strText().bCompareI(strToCheck))
            c4FndCount++;
    }
    return (c4FndCount != 0);
}


// For any stuff that needs to run actions, we provide a common global vars target
TStdVarsTar& TFacCQCVoice::ctarGlobalVars()
{
    return m_ctarGlobalVars;
}


// Provide access to our user context to those bits that need it.
const TCQCUserCtx& TFacCQCVoice::cuctxToUse() const
{
    return m_cuctxToUse;
}


// If there are any outstanding reminders, remove them and say we did
tCIDLib::TVoid TFacCQCVoice::CancelAllReminders()
{
    const tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    if (c4Count)
    {
        m_colReminders.RemoveAll();

        if (c4Count > 1)
            SpeakReply(kCQCVoiceMsgs::midSpeak_RemindersCancelledP, TCardinal(c4Count));
        else
            SpeakReply(kCQCVoiceMsgs::midSpeak_RemindersCancelledS, TCardinal(c4Count));
    }
     else
    {
        // None to cancel so say that
        SpeakReply(kCQCVoiceMsgs::midSpeak_NoReminders);
    }
}


// Cancel the last reminder if it has not already happened
tCIDLib::TVoid TFacCQCVoice::CancelLastReminder()
{
    const tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCVoiceReminder& cvremCur = m_colReminders[c4Index];

        if (cvremCur.c4UniqueId() == m_c4LastRemId)
        {
            m_c4LastRemId = 0;
            m_colReminders.RemoveAt(c4Index);
            SpeakReply(kCQCVoiceMsgs::midSpeak_LastRemCancelled);
            return;
        }
    }

    // It is too late
    SpeakReply(kCQCVoiceMsgs::midSpeak_NoLastReminder);
}



//
//  The BT thread calls this regularly, to let us check to see if any reminders are
//  ready to process. Speaking each reminder takes time, so we continue to check, since
//  another one may be ready by the time the current one is spoken.
//
tCIDLib::TVoid TFacCQCVoice::CheckReminders()
{
    tCIDLib::TEncodedTime enctCur(TTime::enctNow());

    tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    tCIDLib::TCard4 c4Index = 0;
    while (c4Index < c4Count)
    {
        const TCQCVoiceReminder& cvremCur = m_colReminders[c4Index];

        // If the this one isn't ready, none following it can be, so stop
        if (enctCur < cvremCur.enctAt())
            break;

        // If this was the last one queued, then clear the last reminder id
        if (cvremCur.c4UniqueId() == m_c4LastRemId)
            m_c4LastRemId = 0;

        // Let's speak this one
        try
        {
            tCIDLib::TMsgId midReply;
            if (TLocale::b24Hour())
                midReply = kCQCVoiceMsgs::midSpeakP_Reminder24;
            else
                midReply = kCQCVoiceMsgs::midSpeakP_Reminder12;

            TString strTime;
            cvremCur.FormatTime(strTime);
            SpeakReply(midReply, cvremCur.strText(), strTime, kCIDLib::True);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Remove this one and bump down the count
        m_colReminders.RemoveAt(c4Index);
        c4Count--;

        // Update the time to account for the time taken to speak the above
        enctCur = TTime::enctNow();
    }
}


//
//  We read and discard any recognition events that are already in the queue. In
//  theory a new one could show up just as we are doing this and we eat it, but it's
//  very unlikely given that this will take microseconds.
//
tCIDLib::TVoid TFacCQCVoice::EatPendingEvents()
{
    tCIDAudStream::EStrmStates eState;
    TCIDSpeechRecoEv  sprecevToEat;
    while (m_sprecoLocal.bGetNextRecEvent(sprecevToEat, 50, eState))
    {
    }
}


//
//  Make our media database pointer available. It may not be set, but we assume that
//  the caller with either check, or look at the caps flags. If they don't this is
//  going to throw.
//
const TMediaDB& TFacCQCVoice::mdbMusic() const
{
    return m_cptrMusic->mdbData();
}


// The main thread entry point of the server
tCIDLib::EExitCodes TFacCQCVoice::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::False))
        return tCIDLib::EExitCodes::BadEnvironment;

    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit. Tell it not try to bring
        //  the old one forward, since it's not a normal window and would
        //  fail anyway.
        //
        TResourceName rsnInstance(L"CQSL", L"CQCVoice", L"SingleInstanceInfo");
        if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::False))
            return tCIDLib::EExitCodes::Normal;

        //
        //  The next thing we want to do is the most fundamental boot-strapping. We
        //  need to crank up the Orb client support (so that we can find the name
        //  server.) And then install a log server logger, which will allow anything
        //  hat goes wrong after that to go to the log server.
        //
        facCIDOrb().InitClient();

        //
        //  The next thing we want to do is to install a log server logger. We
        //  just use the standard one that's provided by CIDLib. It logs to
        //  the standard CIDLib log server, and uses a local file for fallback
        //  if it cannot connect.
        //
        m_plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(m_plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        // Now parse the parameters
        tCIDLib::TCard4 c4DelaySecs = 0;
        if (!bParseParms(c4DelaySecs))
            return tCIDLib::EExitCodes::Deschedule;

        // Log that we are staring up
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_Startup
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TString(L"CQCVoice")
            , strVersion()
        );

        //
        //  If they indicated a startup delay, then let's wait for that
        //  number of seconds. The delay is in seconds.
        //
        if (c4DelaySecs)
            facCIDCtrls().MsgYield(c4DelaySecs * 1000);

        // Init the server side of the ORB now
        facCIDOrb().InitServer(m_ippnListen, m_c4MaxClients);

        //
        //  Create one of our behavior tree node factories and register it with CIDAI. Has
        //  to be done before we load the configuration.
        //
        facCIDAI().RegisterFactory(new TCQCVoiceBTNodeFact);

        // Start up the media database cacher
        facCQCMedia().StartMDBCacher();

        // Register an instance of our core admin implementation
        TCIDCoreAdminImpl* porbsCoreAdmin = new TCIDCoreAdminImpl;
        facCIDOrb().RegisterObject(porbsCoreAdmin, tCIDLib::EAdoptOpts::Adopt);

        //
        //  Register it with the rebinder. The binding has to have our host name put
        //  into it, since these can be installed on multiple machines.
        //
        TString strBinding(kCQCKit::pszCQCVoiceSrvAdmin);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);
        facCIDOrbUC().RegRebindObj
        (
            porbsCoreAdmin->ooidThis(), strBinding, L"CQC Voice Core Admin Object"
        );

        // Create or main frame window
        m_wndMain.Create();

        //
        //  Load the icon we'll use. We'll pass it to the tray window which needs to
        //  pass it in each time we update the tray icon and we'll set it on the main
        //  frame.
        //
        TIcon iconTray(facCQCVoice, kCQCVoice::ridIco_Tray);

        // If we are to do visual text replies, create that window
        if (m_bTextReplies)
        {
            m_pwndVisText = new TCQCVoiceVisTextFrame();
            m_pwndVisText->Create();
        }

        //
        //  And now create the tray app interface window. Give it a known text value
        //  so that the installer can find the (otherwise not very easily findable)
        //  main window to force a shutdown.
        //
        m_wndTray.CreateTrayApp
        (
            TArea(10, 10, 100, 100), iconTray, L"CQCVoiceWnd"
        );

        //
        //  And start up our thread that runs the behavior tree. We start it on a
        //  private method of ours. We need to be sure this is done after the windows
        //  are created.
        //
        m_thrBTree.Start();

        //
        //  And now start the rebinder since everything is in place. It will immediately
        //  bind all our server side objects, then periodically make sure that they
        //  are rebound if the name server cycles. We don't want to do it before now
        //  because we want to be ready before accepting any connections.
        //
        facCIDOrbUC().StartRebinder();

        //
        //  Call the window processing loop. We won't come back until the program exits.
        //  We make the hidden tray window the main window here, not our frame window.
        //
        facCIDCtrls().uMainMsgLoop(m_wndTray);
    }

    catch(TError& errToCatch)
    {
        // If the error hasn't been logged, then log it
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_InitFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return tCIDLib::EExitCodes::FatalError;
    }

    catch(...)
    {
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_InitFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }


    // Indicate that we are going bye-bye in a normal way
    if (bLogStatus())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_NormalExit
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Do the cleanup work
    DoCleanup();

    return tCIDLib::EExitCodes::Normal;
}


//
//  Returns the room for which we are configured, if it exists. They caller should
//  check the caps flag to make sure we have it first, else we'll throw if not.
//
const TCQCSysCfgRmInfo& TFacCQCVoice::scrmiOurs() const
{
    if (!m_pscrmiOurs)
    {
        facCQCVoice.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcCfg_NoRoomData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , m_strRoom
        );
    }
    return *m_pscrmiOurs;
}


//
//  Convenience methods to get various types of config info for our room. The caller
//  should check the cap flags before calling these, since they'll throw if the reqested
//  data is not available.
//
const TSCHVACInfo& TFacCQCVoice::scliHVAC() const
{
    const TSCHVACInfo* pscliHVAC = nullptr;

    // If the HVAC data caps flag isn't set, then no need to bother
    if (bCapFlag(tCQCVoice::ECapFlags::HVACData))
    {
        // Get the id of our room's HVAC info
        const TString& strHVACId = m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::HVAC);

        // If we have one, then get it
        if (!strHVACId.bIsEmpty())
        {
            tCIDLib::TCard4 c4At;
            pscliHVAC = &m_scfgCurrent.scliHVACByUID(strHVACId, c4At);
        }
    }

    if (!pscliHVAC)
    {
        facCQCVoice.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcCfg_NoRoomData2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , TString(L"HVAC")
            , m_strRoom
        );
    }
    return *pscliHVAC;
}

const TSCMediaInfo& TFacCQCVoice::scliMusic() const
{
    const TSCMediaInfo* pscliMusic = nullptr;

    // If the music data caps flag isn't set, then no need to bother
    if (bCapFlag(tCQCVoice::ECapFlags::MusicData))
    {
        // Get the id of our room's music info
        const TString& strMusicId = m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::Music);

        // If we have one, then get it
        if (!strMusicId.bIsEmpty())
        {
            tCIDLib::TCard4 c4At;
            pscliMusic = &m_scfgCurrent.scliMediaByUID(strMusicId, c4At);
        }
    }

    if (!pscliMusic)
    {
        facCQCVoice.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcCfg_NoRoomData2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , TString(L"music")
            , m_strRoom
        );
    }
    return *pscliMusic;
}

const TSCSecInfo& TFacCQCVoice::scliSecurity() const
{
    const TSCSecInfo* pscliSec = nullptr;

    // If the security data cap flag isn't set, then no need to bother
    if (bCapFlag(tCQCVoice::ECapFlags::SecData))
    {
        // Get the id of our room's sec info
        const TString& strSecId = m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::Security);

        // If we have one, then get it
        if (!strSecId.bIsEmpty())
        {
            tCIDLib::TCard4 c4At;
            pscliSec = &m_scfgCurrent.scliSecByUID(strSecId, c4At);
        }
    }

    if (!pscliSec)
    {
        facCQCVoice.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcCfg_NoRoomData2
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotReady
            , TString(L"Security")
            , m_strRoom
        );
    }
    return *pscliSec;
}


//
//  Various things need access to the latest config data. No need to sync. It's only
//  reloaded upon request from the user (or initially during startup.)

const TCQCSysCfg& TFacCQCVoice::scfgCurrent() const
{
    return m_scfgCurrent;
}

// Provide access to the room that we are configured by
const TString& TFacCQCVoice::strRoom() const
{
    return m_strRoom;
}


// Set one of the capabilities flags
tCIDLib::TVoid
TFacCQCVoice::SetCapFlag(  const   tCQCVoice::ECapFlags   eFlag
                            , const tCIDLib::TBoolean       bToSet)
{
    m_fcolCapFlags[eFlag] = bToSet;
}


//
//  Set the recognition input pause/resume state. Setting it to true will prevent the
//  input stream from passing any data to the recognition engine. This is done to
//  prevent us hearing our own speech.
//
tCIDLib::TVoid TFacCQCVoice::SetRecoPause(const tCIDLib::TBoolean bToSet)
{
    if (m_eState >= tCQCVoice::EStates::WaitRecoInit)
    {
        if (m_bDebugOutput && bToSet)
            ShowMsg(L"Disabling speech recognition");

        m_sprecoLocal.bRecoPause(bToSet);

        if (m_bDebugOutput && !bToSet)
            ShowMsg(L"Enabling speech recognition");
    }
}



// Sets the reload request flag for the BT thread to see and react to
tCIDLib::TVoid TFacCQCVoice::SetReloadReqFlag()
{
    m_bReloadReq = kCIDLib::True;
}


//
//  We upset the enable/disable state of the indicated rule. We check to see if we
//  are at least up to the point of initializing the recognizer.
//
tCIDLib::TVoid
TFacCQCVoice::SetRuleState(const TString& strRuleName, const tCIDLib::TBoolean bNewState)
{
    #if 0
    if (m_eState >= tCQCVoice::EStates::WaitRecoInit)
    {
        m_sprecoLocal.SetRuleState(strRuleName, bNewState);

        if (m_bDebugOutput)
        {
            TString strMsg(L"Rule '", 64UL);
            strMsg.Append(strRuleName);
            strMsg.Append(L" was ");
            strMsg.Append(bNewState ? L"Enabled" : L"Disabled");
            ShowMsg(strMsg);
        }
    }
    #endif
}


//
//  Background threads call this to dipslay messages in our main window. We just pass
//  it on to the main frame window.
//
tCIDLib::TVoid TFacCQCVoice::ShowMsg(const TString& strMsg)
{
    m_wndMain.QueueMsg(strMsg);
}


//
//  Called from the tray interface window upon a power state change. We will stop our
//  behavior tree three and the speech reco engine when being shut down, and start
//  them back up when powered back up. All of this stuff is handled by the behavior
//  tree thread so just stopping and starting that does most of the work.
//
tCIDLib::TVoid TFacCQCVoice::SleepWake(const tCIDLib::TBoolean bWakingUp)
{
    if (bWakingUp)
    {
        facCQCVoice.ShowMsg(L"Restoring from system sleep");

        // Start the behavior tree back up
        m_thrBTree.Start();

        // And start the rebinder back up
        facCIDOrbUC().StartRebinder();
    }
     else
    {
        facCQCVoice.ShowMsg(L"System sleep state invoked....");

        //
        //  Remove any bindings, which will stop the rebinder, but it doesn't remove
        //  the bindings we've registered with him.
        //
        RemoveBindings();

        // Stop the behavior tree thread
        try
        {
            if (m_thrBTree.bIsRunning())
            {
                m_thrBTree.ReqShutdownSync(5000);
                m_thrBTree.eWaitForDeath(5000);
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}


//
//  When the (hidden) tray window is notified that the user clicked on our
//  tray icon, this is called. If the main frame is hidden, we show it, and
//  vice versa.
//
tCIDLib::TVoid TFacCQCVoice::ToggleFrameVis()
{
    m_wndMain.SetVisibility(!m_wndMain.bIsVisible());
}


//
//  If the user adds a reminder and there's already one outstanding, we ask if we should
//  update that one or add a new one. If updating, this is called to change the time.
//
tCIDLib::TVoid
TFacCQCVoice::UpdateReminderTime(const  TString&        strToFind
                                , const tCIDLib::TCard4 c4Minutes)
{
    const tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCVoiceReminder& cvremCur = m_colReminders[c4Index];
        if (cvremCur.strText().bCompareI(strToFind))
        {
            // Set this one as the most recent
            m_c4LastRemId = cvremCur.c4UniqueId();

           // Update the time on this one, then re-sort the list
            cvremCur.UpdateTime(c4Minutes);
            m_colReminders.Sort(&TCQCVoiceReminder::eCompByTime);

            TString strTime;
            cvremCur.FormatTime(strTime);
            if (TLocale::b24Hour())
                SpeakReply(kCQCVoiceMsgs::midSpeakP_WillRemind24, strTime, kCIDLib::True);
            else
                SpeakReply(kCQCVoiceMsgs::midSpeakP_WillRemind12, strTime, kCIDLib::True);
            return;
        }
    }

    // Say we never found it
    SpeakReply(kCQCVoiceMsgs::midErr_RemNotFound, strToFind);
}



// ---------------------------------------------------------------------------
//  TFacCQCVoice: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Called on startup to parse the command line parms. Some are standard ones that are
//  gotten by CQCKit for us, in the call to get environmental settings back up in the main
//  entry point method.
//
tCIDLib::TBoolean TFacCQCVoice::bParseParms(tCIDLib::TCard4& c4DelaySecs)
{
    // Log that we are starting the parms
    if (bLogInfo())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_StartCmdLineParse
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // Initialize some stuff first, so that they only change if set explicitly
    m_bSpeakReplies = kCIDLib::False;
    m_bTextReplies = kCIDLib::False;

    TString strCurParm;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        // We know what is by what it starts with
        if (strCurParm.bStartsWithI(L"/Delay="))
        {
            //
            //  Startup delay to avoid issues with the system coming up
            //  and not responding to our requests.
            //
            try
            {
                strCurParm.Cut(0, 7);
                c4DelaySecs = strCurParm.c4Val(tCIDLib::ERadices::Dec);
            }

            catch(...)
            {
            }

            // Clip it back to 120 seconds
            if (c4DelaySecs > 120)
                c4DelaySecs = 120;
        }
         else if (strCurParm.bStartsWithI(L"/Password="))
        {
            strCurParm.Cut(0, 10);
            m_strPassword = strCurParm;
        }
         else if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);
            try
            {
                m_ippnListen = strCurParm.c4Val();
            }

            catch(TError& errToCatch)
            {
                if (bShouldLog(errToCatch))
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    LogEventObj(errToCatch);
                }

                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_BadParmVal
                    , tCIDLib::ESeverities::Status
                    , tCIDLib::EErrClasses::AppStatus
                    , strCurParm
                    , TString(L"ORB port")
                );
            }
        }
         else if (strCurParm.bCompareI(L"/SpeakReplies"))
        {
            m_bSpeakReplies = kCIDLib::True;
        }
         else if (strCurParm.bCompareI(L"/TextReplies"))
        {
            m_bTextReplies = kCIDLib::True;
        }
         else if (strCurParm.bStartsWithI(L"/User="))
        {
            strCurParm.Cut(0, 6);
            m_strUserName = strCurParm;
        }
         else if (bLogWarnings())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_BadParm
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::BadParms
                , strCurParm
            );
        }
    }

    // Log that we completed the parsing
    if (bLogInfo())
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitMsgs::midApp_EndCmdLineParse
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    // If we didn't get the user name and password, we failed
    if (m_strUserName.bIsEmpty() || m_strPassword.bIsEmpty())
    {
        facCQCVoice.LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCVoiceErrs::errcGen_NoLogonCreds
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        return kCIDLib::False;
    }

    // If neither reply styles is set, default to spoken
    if (!m_bSpeakReplies && !m_bTextReplies)
        m_bSpeakReplies = kCIDLib::True;

    return kCIDLib::True;
}


//
//  This is called on exit, normal or early exit, to clean up anything
//  that we got going during start, in the correct order.
//
tCIDLib::TVoid TFacCQCVoice::DoCleanup()
{
    // Remove any name server bindings so new clients can't get to us
    RemoveBindings();

    // Stop the media data base cacher
    facCQCMedia().StopMDBCacher();

    // Stop the behavior tree thread
    try
    {
        if (m_thrBTree.bIsRunning())
        {
            m_thrBTree.ReqShutdownSync(5000);
            m_thrBTree.eWaitForDeath(5000);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // Now we can destroy the main frame
    m_wndMain.Destroy();

    // And destroy the tray window, which will remove us from the system tray.
    m_wndTray.Destroy();

    // Destroy the visual text window if it was created
    if (m_pwndVisText)
    {
        try
        {
            m_pwndVisText->Destroy();
            delete m_pwndVisText;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    //
    //  Terminate the Orb support. Force the log server logger back to local
    //  logger before we kill the ORB client.
    //
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->bForceLocal(kCIDLib::True);
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_TermORB
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    catch(...)
    {
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_TermORB
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }

    //
    //  And finally make the logger stop logging. We told TModule to adopt it so
    //  he will clean it up.
    //
    try
    {
        if (m_plgrLogSrv)
            m_plgrLogSrv->Cleanup();
    }

    catch(...)
    {
    }
}


//
//  This will look for a queued reminder by its unique id, returning either a
//  pointer or null.
//
TCQCVoiceReminder* TFacCQCVoice::pcvremFindByUniqueId(const tCIDLib::TCard4 c4UniqueId)
{
    const tCIDLib::TCard4 c4Count = m_colReminders.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colReminders[c4Index].c4UniqueId() == c4UniqueId)
            return &m_colReminders[c4Index];
    }
    return nullptr;
}


//
//  Called during cleanup to remove all of the name server bindings that
//  we have created. Our return indicates whether we found the name server
//  or not. There are various steps to go after this is done and they might
//  also need to access the NS, so we can tell them not to bother and
//  greatly speed up the shutdown.
//
tCIDLib::TVoid TFacCQCVoice::RemoveBindings()
{
    tCIDLib::TBoolean bAnyFailed = kCIDLib::False;

    // Get a client name server proxy
    tCIDOrbUC::TNSrvProxy orbcNS;
    try
    {
        //
        //  Shut down the rebinder before we start removing bindings. Otherwise,
        //  he'll just start rebinding them again.
        //
        facCIDOrbUC().StopRebinder();

        //
        //  And get us a name server proxy to use for all of this stuff
        //
        //  We explicitly set a fairly short timeout on getting the proxy,
        //  since we don't want to delay shutdown too much and if we are going
        //  down probably he is also. Worst case, the bindings don't get
        //  cleaned up and they get thrown out by the NS because they are not
        //  being renewed.
        //
        orbcNS = facCIDOrbUC().orbcNameSrvProxy(3000);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        // If we can't get the NS proxy, we can't do much
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_NSCleanup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return;
    }


    // Try to remove our core object
    try
    {
        TString strBinding(kCQCKit::pszCQCVoiceSrvAdmin);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);
        orbcNS->RemoveBinding(strBinding, kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }

        // If we can't get the NS proxy, we can't do much
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_UnbindCoreAdmin
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        bAnyFailed = kCIDLib::True;
    }

    // If we got any failures, then log it
    if (bAnyFailed && bLogFailures())
    {
        if (bLogStatus())
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_NSCleanup
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
    }
}


