//
// FILE NAME: CQCVoice_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2017
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
//  This file implements the main background processing thread of the program. This
//  guy is basically running the behavior tree. It grabs recognition events that the
//  speech reco engine queues up, and finds the tree node to handle it, if any is
//  available.
//
//  It also though handles setup and tear down, getting logged in, downloading the
//  required stuff, etc... It is always trying to get us up to ready to go, implementing
//  a small state machine to drive the process.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"



// ---------------------------------------------------------------------------
//  TFacCQCVoice: Private, non-virtual methods
// ---------------------------------------------------------------------------



//
//  We get login credentials on the command line, and need to login and set up a
//  user context that will be available to our code that needs basic rights to
//  do some things.
//
tCIDLib::TBoolean
TFacCQCVoice::bDoLogin(const   tCIDLib::TCard4 c4RoundCnt
                        ,       TError&         errLast)
{
    tCIDLib::TBoolean bRet = kCIDLib::False;

    // If the first time for this state, reset the last error and show msg
    if (!c4RoundCnt)
    {
        errLast.Reset();
        facCQCVoice.ShowMsg(L"Attempting to log in");
    }

    try
    {
        // Get a reference to the security server
        tCQCKit::TSecuritySrvProxy orbcSS = facCQCKit().orbcSecuritySrvProxy();

        // And ask it to give us a challenge
        TCQCSecChallenge seccLogon;
        if (orbcSS->bLoginReq(m_strUserName, seccLogon))
        {
            // Lets do a hash of the user's password
            TMD5Hash mhashPW;
            TMessageDigest5 mdigTmp;
            mdigTmp.StartNew();
            mdigTmp.DigestStr(m_strPassword);
            mdigTmp.Complete(mhashPW);

            // And use that to validate the challenge
            seccLogon.Validate(mhashPW);

            // And send that back to get a security token, assuming its legal
            tCQCKit::ELoginRes  eResult;
            TCQCSecToken        sectLogin;
            TCQCUserAccount     uaccLogin;
            if (orbcSS->bGetSToken(seccLogon, sectLogin, uaccLogin, eResult))
            {
                bRet = kCIDLib::True;
                m_cuctxToUse.Set(uaccLogin, sectLogin);
                m_cuctxToUse.LoadEnvRTVsFromHost();
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && !errToCatch.bSameEvent(errLast))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            errLast = errToCatch;

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcGen_LoginFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }
    }

    if (bRet)
        facCQCVoice.ShowMsg(L"CQC login succeeded");

    return bRet;
}


tCIDLib::TBoolean
TFacCQCVoice::bLoadGrammar(const tCIDLib::TCard4 c4RoundCnt, TError& errLast)
{
    // If the first time for this state, reset the last error and show msg
    if (!c4RoundCnt)
    {
        errLast.Reset();
        facCQCVoice.ShowMsg(L"Attempting to load recognition grammar");
    }

    try
    {
        // The grammar data has been downloaded in a buffer, so laod from there
        m_sprecoLocal.LoadGrammarFromBuf(m_mbufGrammar, m_c4GrammarSz, kCIDLib::True);

        //
        //  Now call a helper that will use the room config data to make dynamic changes
        //  to the grammar.
        //
        LoadDynRules();

        //
        //  Now tell the recognizer that loading is complete. It will commit the grammar
        //  and enable it.
        //
        m_sprecoLocal.GrammarLoadingDone();

        // Disable some rules initially that are only needed on a scoped basis
        #if 0
        const tCIDLib::TStrList& colRules = m_sprecoLocal.colRuleNames();
        const tCIDLib::TCard4 c4Count = colRules.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strCur = colRules[c4Index];

            if (strCur.bStartsWithI(kCQCVoice::strVal_OnDemPrefix))
            {
                m_sprecoLocal.SetRuleState(strCur, kCIDLib::False);
            }
        }
        #endif
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged() && !errToCatch.bSameEvent(errLast))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            errLast = errToCatch;

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcGen_LoginFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Authority
            );
        }

        return kCIDLib::False;
    }

    facCQCVoice.ShowMsg(L"Grammar load succeeded");
    return kCIDLib::True;
}



//
//  This will check for the installation server to be available, and if so will try
//  to download the CQC Voice configuration.
//
tCIDLib::TBoolean
TFacCQCVoice::bLoadVoiceConfig(const tCIDLib::TCard4 c4RoundCnt, TError& errLast)
{
    // Set all cap flags false until proven otherwise
    m_fcolCapFlags.SetAll(kCIDLib::False);

    // If the first time for this state, reset the last error and show msg
    if (!c4RoundCnt)
    {
        errLast.Reset();
        facCQCVoice.ShowMsg(L"Loading CQC Voice configuration");
    }

    try
    {
        tCQCKit::TInstSrvProxy orbcIS = facCQCKit().orbcInstSrvProxy();

        tCIDLib::TCard4 c4BTreeSz;
        THeapBuf mbufBTree;
        orbcIS->QueryVoiceFiles
        (
            c4BTreeSz, mbufBTree, m_c4GrammarSz, m_mbufGrammar
        );

        //
        //  Make sure we can parse the behavior tree data. We have to reset the
        //  tree since we are replacing content, not adding new stuff.
        //
        m_btreeLogic.Reset();
        m_btreeLogic.LoadXMLTree(mbufBTree, c4BTreeSz);
    }

    catch(TError& errToCatch)
    {
        // If not the same as the last error we logged, then log it
        if (!errToCatch.bLogged() && !errToCatch.bSameEvent(errLast))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            errLast = errToCatch;

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcCfg_LoadCQCVoiceCfg
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    facCQCVoice.ShowMsg(L"CQCVoice resources downloaded successfully");
    return kCIDLib::True;
}


//
//  This will check for the system config server to be available, and if so will try
//  to download the room configuration data.
//
tCIDLib::TBoolean
TFacCQCVoice::bLoadRoomConfig(  const   tCIDLib::TCard4     c4RoundCnt
                                ,       TError&             errLast)
{
    // If our first time, reset the last error and show a message
    if (c4RoundCnt == 0)
    {
        errLast.Reset();
        facCQCVoice.ShowMsg(L"Downloading room configuration");
    }

    try
    {
        TCQCSysCfgClientProxy* porbcProxy = facCIDOrbUC().porbcMakeClient<TCQCSysCfgClientProxy>
        (
            TCQCSysCfgClientProxy::strBinding, 4000
        );
        TCntPtr<TCQCSysCfgClientProxy> orbcProxy(porbcProxy);
        orbcProxy->bQueryRoomCfg(m_c4SysCfgSerNum, m_scfgCurrent);

        // Clear all the cap flags and set the system config one
        m_fcolCapFlags.SetAll(kCIDLib::False);
        SetCapFlag(tCQCVoice::ECapFlags::SystemCfg, kCIDLib::True);

        //
        //  We got it so break out. We need to look through the rooms for one that
        //  has voice control set up for our local host name. That will be our
        //  room (it has to be enabled as well of course.)
        //
        FindOurRoom();

        // If we found our room, then set up data
        if (m_pscrmiOurs)
        {
            // Store the room name
            m_strRoom = m_pscrmiOurs->strName();

            // Set the room data flag to indicate whether we have room data or not
            SetCapFlag(tCQCVoice::ECapFlags::RoomData, m_pscrmiOurs != nullptr);

            //
            //  Set other individual flags based on the info we find. We won't
            //  do all of them here. So are related to dynamic rules and we'll
            //  set them later when we update the grammar. We just set the basic
            //  flags here. Some we may even clear back out again based on later
            //  more detailed exploration.
            //
            if (!m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::HVAC).bIsEmpty())
                SetCapFlag(tCQCVoice::ECapFlags::HVACData, kCIDLib::True);

            if (!m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::Music).bIsEmpty())
                SetCapFlag(tCQCVoice::ECapFlags::MusicData, kCIDLib::True);

            if (!m_pscrmiOurs->strFuncId(tCQCSysCfg::ERmFuncs::Security).bIsEmpty())
            {
                SetCapFlag(tCQCVoice::ECapFlags::SecData, kCIDLib::True);

                // Set some sub-flags
                const TSCSecInfo& scliOurs = scliSecurity();
                if (!scliOurs.m_strArmingCode.bIsEmpty())
                    SetCapFlag(tCQCVoice::ECapFlags::SecArmingCode, kCIDLib::True);
            }

            if (m_pscrmiOurs->c4RoomModeCount())
                SetCapFlag(tCQCVoice::ECapFlags::RoomModes, kCIDLib::True);
        }
         else
        {
            facCQCVoice.ShowMsg(L"No room's voice host matched our host name");
            return kCIDLib::False;
        }
    }

    catch(TError& errToCatch)
    {
        // If not the same as the last error we logged, then log it
        if (!errToCatch.bLogged() && !errToCatch.bSameEvent(errLast))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            errLast = errToCatch;

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcCfg_LoadSysCfg
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    facCQCVoice.ShowMsg(L"Room configuration downloaded successfully");
    return kCIDLib::True;
}


//
//  This is called to try to initialize the speech recognition engine.
//
tCIDLib::TBoolean
TFacCQCVoice::bSetupRecoEngine(const tCIDLib::TCard4 c4RoundCnt, TError& errLast)
{
    // If our first time, reset the last error and show a message
    if (c4RoundCnt == 0)
    {
        errLast.Reset();
        facCQCVoice.ShowMsg(L"Initializing speech recognition");
    }

    try
    {
        m_sprecoLocal.Initialize(nullptr);
    }

    catch(TError& errToCatch)
    {
        if (c4RoundCnt == 0)
            facCQCVoice.ShowMsg(L"Failed to initialize the speech engine. See the logs");

        // If not the same as the last error we logged, then log it
        if (!errToCatch.bLogged() && !errToCatch.bSameEvent(errLast))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            errLast = errToCatch;

            LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCVoiceErrs::errcReco_InitFailed
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    catch(...)
    {
        return kCIDLib::False;
    }

    facCQCVoice.ShowMsg(L"Speec recognition engine successfully initialized");
    return kCIDLib::True;
}


//
//  A thread is started on this method which runs our behavior tree and manages all
//  of the speech and speech recognition functionality.
//
tCIDLib::EExitCodes
TFacCQCVoice::eBTThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Now we can let the calling thread go
    thrThis.Sync();

    //
    //  We just go into a loop until we are asked to stop, trying to get up to
    // the point that we are waiting for speech events to process.
    //
    tCIDLib::TCard4     c4RoundCnt = 0;
    TError              errLast;
    TCIDSpeechRecoEv    sprecevCur;
    TString             strAction;
    TString             strSemVal;
    tCIDLib::TKValsList colVars;

    // Reset the tree in case this isn't the first time
    m_btreeLogic.Reset();

    TThread* pthrCaller = TThread::pthrCaller();
    m_eState = tCQCVoice::EStates::WaitLogin;
    while (!pthrCaller->bCheckShutdownRequest())
    {
        const tCQCVoice::EStates eOrgState = m_eState;
        switch(m_eState)
        {
            case tCQCVoice::EStates::WaitLogin :
                if (bDoLogin(c4RoundCnt, errLast))
                    m_eState = tCQCVoice::EStates::WaitVoiceCfg;
                break;

            case tCQCVoice::EStates::WaitVoiceCfg :
                if (bLoadVoiceConfig(c4RoundCnt, errLast))
                    m_eState = tCQCVoice::EStates::WaitRoomCfg;
                break;

            case tCQCVoice::EStates::WaitRoomCfg :
            {
                if (bLoadRoomConfig(c4RoundCnt, errLast))
                    m_eState = tCQCVoice::EStates::WaitRecoInit;
                break;
            }

            case tCQCVoice::EStates::WaitRecoInit :
                if (bSetupRecoEngine(c4RoundCnt, errLast))
                    m_eState = tCQCVoice::EStates::WaitLoadGrammar;
                break;

            case tCQCVoice::EStates::WaitLoadGrammar :
                if (bLoadGrammar(c4RoundCnt, errLast))
                {
                    m_eState = tCQCVoice::EStates::Ready;
                    facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_Ready);
                }
                break;

            case tCQCVoice::EStates::Ready :
                ProcessEvents(sprecevCur, colVars, strAction, strSemVal);
                break;
        };

        // If the state didn't change, bump the round count, else reset it
        if (m_eState == eOrgState)
            c4RoundCnt++;
        else
            c4RoundCnt = 0;

        //
        //  If not in ready state and the state didn't change, sleep a bit while watching
        //  for a shutdown request. If in ready state we are throttled by waiting for reco
        //  events, and the shudown request check at the top of the loop will
        //  catch shutdown requests.
        //
        if ((m_eState == eOrgState) && (m_eState != tCQCVoice::EStates::Ready))
        {
            if (!pthrCaller->bSleep(5000))
                break;
        }

        //
        //  Check the reload request flag. If it is set, then push us back to the
        //  WaitVoiceCfg state. This will make us reload all of our configuration.
        //
        if (m_bReloadReq)
        {
            // DO THIS BEFORE we terminate and start reloading!
            facCQCVoice.SpeakReply(kCQCVoiceMsgs::midSpeak_CfgReloading);

            // Uninitialize the reco engine and fall back to waiting for voice config
            m_sprecoLocal.Terminate();

            m_bReloadReq = kCIDLib::False;
            m_eState = tCQCVoice::EStates::WaitVoiceCfg;
        }

        // If in ready state, check for any reminders
        if (m_eState == tCQCVoice::EStates::Ready)
            CheckReminders();
    }

    // Do any cleaup necessary
    if (m_eState >= tCQCVoice::EStates::WaitRecoInit)
    {
        try
        {
            m_sprecoLocal.Terminate();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    //
    //  Reset the state. We set it at the top of our loop above, but this is also
    //  telling the GUI thread that we are not ready.
    //
    m_eState = tCQCVoice::EStates::WaitLogin;

    return tCIDLib::EExitCodes::Normal;
}


//
//  This is called to figure out which room we are configured for. We store a pointer
//  to our room data if we find it. Each room (that has voice control enabled) configures
//  the host name of the system that it wants to handle voice control for it.
//
tCIDLib::TVoid TFacCQCVoice::FindOurRoom()
{
    // Assume the worst
    m_pscrmiOurs = nullptr;

    const TString& strOurHost = TSysInfo::strIPHostName();
    const tCIDLib::TCard4 c4Count = m_scfgCurrent.c4RoomCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCSysCfgRmInfo& scriCur = m_scfgCurrent.scriRoomAt(c4Index);
        const TCQCSysCfgVoice& scvcCur = scriCur.scvcVoice();

        if (scvcCur.bEnable() && scvcCur.strHost().bCompareI(strOurHost))
        {
            m_pscrmiOurs = &scriCur;
            break;
        }
    }
}


//
//  When we are in ready state, this is called to process any events. The main
//  thread loop passes us some objects to use, so that we don't have to constantly
//  create and destroy them.
//
tCIDLib::TVoid
TFacCQCVoice::ProcessEvents(       TCIDSpeechRecoEv&   sprecevCur
                            , const tCIDLib::TKValsList colVars
                            ,       TString&            strAction
                            ,       TString&            strSemVal)
{

    //
    //  Wait for an event. This is the top level loop, so we can wait for a bit.
    //  But not too long so that we are checking for shutdown requests. And we
    //  can ignore low confidence events there.
    //
    if (facCQCVoice.bGetNextEvent(sprecevCur, strAction, 1000, kCIDLib::False))
    {
        //
        //  We decide here if we have seen something that will put us into conversation
        //  mode. That would be the following:
        //
        //  1. We get the wakeup action
        //  2. We get another rule that has the Prefixed=Prefixed semantic value,
        //      which means they said, [keyword], blah blah blah
        //
        //  If so, we run it
        //
        const tCIDLib::TBoolean bIsWakeup(strAction.bCompareI(kCQCVoice::strAction_WakeupKeyWord));
        if (bIsWakeup
        ||  (sprecevCur.bFindSemVal(kCQCVoice::strSemVal_Prefixed, strSemVal)
        &&   strSemVal.bCompareI(kCQCVoice::strVal_Prefixed)))
        {
            //
            //  If it's a prefixed command, make sure that the prefix value has high
            //  confidence, so that we don't bogus trigger.
            //
            if (!bIsWakeup)
            {
                const TKeyNumPair* pknumKeyword = sprecevCur.pknumFindSemVal
                (
                    kCQCVoice::strSemVal_Prefixed
                );

                if (!pknumKeyword || (pknumKeyword->f4Value() < kCQCVoice::f4HighConfidence))
                    return;
            }

            try
            {
                // Put us into conversation mode
                SetConversationMode(kCIDLib::True);

                m_btreeLogic.eRun
                (
                    kCQCVoice::strVal_CQCVoiceTree, colVars, sprecevCur.colSemList()
                );

                // Take us out of conversation mode
                SetConversationMode(kCIDLib::False);
            }

            catch(TError& errToCatch)
            {
                // Make sure we get back out of conversation mode
                SetConversationMode(kCIDLib::False);

                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            catch(...)
            {
                // Make sure we get back out of conversation mode
                SetConversationMode(kCIDLib::False);
            }
        }
    }
}
