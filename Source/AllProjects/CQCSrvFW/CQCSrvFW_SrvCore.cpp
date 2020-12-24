//
// FILE NAME: CQCSrvFW_SrvCore.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2019
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
//  This file implements the core CQC server framework.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSrvFW_.hpp"
#include    "CIDKernel_RegistryWin32.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSrvCore,TObject)


// ---------------------------------------------------------------------------
//   CLASS: TCQCSrvCore
//  PREFIX: cqcsrv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSrvCore: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSrvCore::~TCQCSrvCore()
{
    // Just in case it didn't get cleaned up
    try
    {
        delete m_pevWait;
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TFacCQCServer: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSrvCore::bHandleSignal(const tCIDLib::ESignals eSig)
{
    //
    //  If it's a Ctrl-C or Ctrl-Break, call our AdminStop method. Ignore
    //  the others. This way, we can be stopped manually when running in
    //  debug mode, but don't get blown away by logoff signals when running
    //  under a service based app shell.
    //
    if ((eSig == tCIDLib::ESignals::CtrlC) || (eSig == tCIDLib::ESignals::Break))
        Shutdown(tCIDLib::EExitCodes::AdminStop);

    // And return that we handled it
    return kCIDLib::True;
}



// ---------------------------------------------------------------------------
//  TCQCSrvCore: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The server program starts us up here as the main thread. We drive the show
//  here, calling him on the various callbacks.
//
tCIDLib::EExitCodes TCQCSrvCore::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Store a pointer to our thread, so that calls to Shutdown() will have
    //  access to it.
    //
    m_pthrSrvFW = &thrThis;

    //
    //  Do the initialization states, trying to get us to ready.
    //
    //  We do the steps here in a careful order. We don't check the server state to
    //  see if we need to wait on the event. That would be hard to really coordinate
    //  without race issues. But, we created it reset. If Shutdown() was called while
    //  we were in the init, that would have just triggered it anyway, so we'd just
    //  fall through.
    //
    //  We only use the state to see if we need to set a failure exit code. Until we
    //  call DoCleanup, it will be set to whatever the high water mark was. We don't
    //  need to sync the state for this simple check. Either we got to ready or we
    //  did not, and that won't change between DoInit() and DoCleanup().
    //
    //  We call WaitForTerm() to wait for the event to be posted. This lets derived
    //  classes handle this and do other things while waiting, but it still has to
    //  watch the event.
    //
    DoInit(thrThis);
    if (m_pevWait)
        WaitForTerm(*m_pevWait);
    if (m_eState != tCQCSrvFW::ESrvStates::Ready)
        m_eReturn = tCIDLib::EExitCodes::InitFailed;
    DoCleanup(thrThis);

    // Return the last exit code that was stored
    return m_eReturn;
}



// ---------------------------------------------------------------------------
//  TCQCSrvCore: Hidden constructors
// ---------------------------------------------------------------------------
TCQCSrvCore::TCQCSrvCore(const  TString&                strName
                        , const TString&                strDescr
                        , const tCIDLib::TIPPortNum     ippnDefSrvPort
                        , const TString&                strEvName
                        , const tCIDLib::EModFlags      eModFlags
                        , const tCQCSrvFW::ESrvOpts     eOptions) :


    TFacility
    (
        strName
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , eModFlags
    )
    , m_c4MaxClients(0)
    , m_c4SigHandlerId(kCIDLib::c4MaxCard)
    , m_eOptions(eOptions)
    , m_eReturn(tCIDLib::EExitCodes::Normal)
    , m_eState(tCQCSrvFW::ESrvStates::Start)
    , m_ippnListen(ippnDefSrvPort)
    , m_pevWait(nullptr)
    , m_plgrLogSrv(nullptr)
    , m_porbsCoreAdmin(nullptr)
    , m_pthrSrvFW(nullptr)
    , m_strDescr(strDescr)
    , m_strEvName(strEvName)
    , m_tmStart(tCIDLib::ESpecialTimes::CurrentTime)
{
}


// ---------------------------------------------------------------------------
//  TCQCSrvCore: Protected, virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCSrvCore::DeregisterSrvObjs()
{
}

tCQCSrvFW::EStateRes TCQCSrvCore::eLoadConfig(tCIDLib::TCard4&, const tCIDLib::TCard4)
{
    return tCQCSrvFW::EStateRes::Success;
}


//
//  If we get called here, we just build up a message indicating that we got
//  unhandled parameters, if any were gotten. This will run the cursor to the
//  end which will prevent the init logic from doing an unhandled parameter
//  error.
//
tCQCSrvFW::EStateRes
TCQCSrvCore::eProcessParms(tCIDLib::TKVPList::TCursor& cursParms)
{
    if (cursParms.bIsValid())
    {
        TString strParms;
        for (; cursParms; ++cursParms)
        {
            strParms += cursParms->strKey();
            strParms += kCIDLib::chSpace;
        }

        facCQCSrvFW().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSrvFWErrs::errcSrvFW_BadAppParms
            , strParms
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::BadParms
        );
    }
    return tCQCSrvFW::EStateRes::Success;
}


tCQCSrvFW::EStateRes TCQCSrvCore::eWaitPrereqs(tCIDLib::TCard4&)
{
    return tCQCSrvFW::EStateRes::Success;
}


tCIDLib::TVoid TCQCSrvCore::MakeDirs()
{
}


tCIDLib::TVoid TCQCSrvCore::PostDeregTerm()
{
}


tCIDLib::TVoid TCQCSrvCore::PostUnbindTerm()
{
}


tCIDLib::TVoid TCQCSrvCore::PreBindInit()
{
}


tCIDLib::TVoid TCQCSrvCore::PreRegInit()
{
}


tCIDLib::TVoid TCQCSrvCore::RegisterSrvObjs()
{
}


tCIDLib::TVoid TCQCSrvCore::StartWorkerThreads()
{
}

tCIDLib::TVoid TCQCSrvCore::StopWorkerThreads()
{
}

tCIDLib::TVoid TCQCSrvCore::StoreConfig()
{
}


tCIDLib::TVoid TCQCSrvCore::UnbindSrvObjs(tCIDOrbUC::TNSrvProxy&)
{
}


//
//  Our default implementation just waits on the passed event, which will be the
//  m_pevWait event.
//
tCIDLib::TVoid TCQCSrvCore::WaitForTerm(TEvent& evWait)
{
    evWait.WaitFor();
}


// ---------------------------------------------------------------------------
//  TCQCSrvCore: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Sometimes the derived class needs to do some pauses during startup. We want these
//  to be done by blocking on the shutdown event, not by sleeping, so that we can watch
//  for a shutdown request via the event. So we provide this helper.
//
tCIDLib::TBoolean TCQCSrvCore::bWait(const tCIDLib::TCard4 c4Millis) const
{
    return m_pevWait->bWaitFor(c4Millis);
}



//
//  For the core admin interface to call in response to a remote shutdown request. We
//  also call it from our signal handler and the derived class can as well if he needs
//  to force a shutdown.
//
tCIDLib::TVoid TCQCSrvCore::Shutdown(const tCIDLib::EExitCodes eReturn)
{
    // Store the return code the caller wants us to treturn
    m_eReturn = eReturn;

    //
    //  If we not up to ready state yet, also do a thread shutdown request on the main
    //  thread, since it won't start waiting on the event until it's gotten to ready
    //  state.
    //
    if ((m_eState < tCQCSrvFW::ESrvStates::Ready) && m_pthrSrvFW)
        m_pthrSrvFW->ReqShutdownNoSync();

    // It's possible this could get called before it's created though highly unlikely
    if (m_pevWait)
        m_pevWait->Trigger();
}


// ---------------------------------------------------------------------------
//  TCQCSrvCore: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If the derived class indicates it needs login, we will do a login on behalf
//  of the server using a special account that the installer left in the Windows
//  registry. The admin is responsible that this is accessible by us and not by
//  anyone it shouldn't be.
//
tCQCSrvFW::EStateRes
TCQCSrvCore::eLogin(tCIDLib::TCard4& c4WaitNext, const tCIDLib::TCard4 c4Count)
{
    // If they didn't ask for this, just say it worked
    if (!tCIDLib::bAllBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::LogIn))
        return tCQCSrvFW::EStateRes::Success;

    //
    //  Let's try to read the registry entries. This is something we have to go to the
    //  Win32 specific kernel layer for at least so far. For local development we also
    //  will look for environment variables. This is safe since it has to be a valid
    //  CQC user name and password, so someone can't just set this to something random
    //  and get any benefit from that.
    //
    TString strUserName;
    TString strPassword;

    TProcEnvironment::bFind(L"CQC_SRVNAME", strUserName);
    TProcEnvironment::bFind(L"CQC_SRVPASS", strPassword);

    // If not set via environment, check the registry
    if (strUserName.bIsEmpty() || strPassword.bIsEmpty())
    {
        //
        //  NOTE that on most machines this will actually be in SOFTWARE/Wow6432Node, because
        //  we are 32 bit and all 32 bit programs running on 64 bit Windows get redirected
        //  to this magic node for certain keys, SOFTWARE being one.
        //
        tCIDLib::TBoolean bCreated = kCIDLib::True;
        tCIDKernel::TWRegHandle hkeyCQS = TKrnlWin32Registry::hkeyOpenSubKey
        (
            tCIDKernel::ERootKeys::LocalMachine
            , L"SOFTWARE"
            , kCQCKit::pszReg_Key
            , tCIDKernel::ERegAccFlags::StdOwned
        );

        if (hkeyCQS != 0)
        {
            const tCIDLib::TCard4 c4BufLen = 2048;
            tCIDLib::TCh achBuf[c4BufLen + 1];

            if (TKrnlWin32Registry::bQueryStrValue(hkeyCQS, kCQCKit::pszReg_SrvUser, achBuf, c4BufLen))
                strUserName.FromZStr(achBuf);

            if (TKrnlWin32Registry::bQueryStrValue(hkeyCQS, kCQCKit::pszReg_SrvPass, achBuf, c4BufLen))
                strPassword.FromZStr(achBuf);

            TKrnlWin32Registry::bCloseKey(hkeyCQS);
        }
    }

    // If still nothing, then retry in 15 seconds
    if (strUserName.bIsEmpty() || strPassword.bIsEmpty())
    {
        c4WaitNext = 15000;
        return tCQCSrvFW::EStateRes::Retry;
    }

    //
    //  Get a reference to the security server. Since this may fail a couple times
    //  in normal scenarios (everything is starting up and the data server may not
    //  be ready yet, we don't call the helper in CQCKit, since it would log an
    //  error.
    //
    try
    {
        tCQCKit::TSecuritySrvProxy orbcSS
        (
            facCIDOrbUC().porbcMakeClient<TCQCSecureClientProxy>
            (
                TCQCSecureClientProxy::strBinding, 5000, kCIDLib::False
            )
        );

        if (orbcSS)
        {
            // And ask it to give us a challenge
            TCQCSecChallenge seccLogon;
            if (orbcSS->bLoginReq(strUserName, seccLogon))
            {
                // Lets do a hash of the user's password
                TMD5Hash mhashPW;
                TMessageDigest5 mdigTmp;
                mdigTmp.StartNew();
                mdigTmp.DigestText(strPassword.pszBuffer(), strPassword.c4Length());
                mdigTmp.Complete(mhashPW);

                // And use that to validate the challenge
                seccLogon.Validate(mhashPW);

                // And send that back to get a security token, assuming its legal
                TCQCSecToken sectInfo;
                TCQCUserAccount uaccInfo;
                tCQCKit::ELoginRes eRes = tCQCKit::ELoginRes::Count;
                if (orbcSS->bGetSToken(seccLogon, sectInfo, uaccInfo, eRes))
                {
                    // Save the password hash for later use by derived classes
                    m_mhashSrvPW = mhashPW;

                    m_cuctxToUse.Set(uaccInfo, sectInfo);
                    return tCQCSrvFW::EStateRes::Success;
                }

                if (facCQCSrvFW().bLogWarnings())
                {
                    LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCSrvFWErrs::errcSrvFW_Login
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::Authority
                        , tCQCKit::strXlatELoginRes(eRes)
                    );
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(tCIDLib::ForceMove(errToCatch));
    }

    // Ask to retry it again in 5 seconds
    c4WaitNext = 5000;
    return tCQCSrvFW::EStateRes::Retry;
}


//
//  This is our internal processing of parameters.
//
//  We go through the parameters and handle any that we can do ourselves. Any
//  others we collection into a key/value pair list and pass to the derived class
//  to process. If he is happy, we are good, else we return a failure.
//
tCQCSrvFW::EStateRes TCQCSrvCore::eIntProcessParms()
{
    tCIDLib::TKVPList colOthers;

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

    TString strCurParm;
    TString strVal;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        // We know what is by what it starts with
        if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);
            if (!strCurParm.bToCard4(m_ippnListen, tCIDLib::ERadices::Dec))
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_BadParmVal
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , *cursParms
                    , TString(L"ORB port")
                );
                return tCQCSrvFW::EStateRes::Failed;
            }
        }
         else if (strCurParm.bStartsWithI(L"/Max="))
        {
            strCurParm.Cut(0, 5);
            if (!strCurParm.bToCard4(m_c4MaxClients, tCIDLib::ERadices::Dec))
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_BadParmVal
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , *cursParms
                    , TString(L"maximum clients")
                );
                return tCQCSrvFW::EStateRes::Failed;
            }
        }
         else if (!strCurParm.bStartsWith(L"/"))
        {
            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcApp_BadParm
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strCurParm
            );
            return tCQCSrvFW::EStateRes::Failed;
        }
         else
        {
            // Collect it as one to pass to the derived class
            if (!strCurParm.bSplit(strVal, kCIDLib::chEquals))
                strVal.Clear();

            strCurParm.Cut(0, 1);
            strCurParm.StripWhitespace();
            strVal.StripWhitespace();
            colOthers.objAdd(TKeyValuePair(strCurParm, strVal));
        }
    }

    //
    //  If we got here none of the ones we do were bad. If we got any others,
    //  pass them to the derived class. If he complains, we throw.
    //
    if (!colOthers.bIsEmpty())
    {
        tCIDLib::TKVPList::TCursor cursOtherParms = tCIDLib::TKVPList::TCursor(&colOthers);
        const tCQCSrvFW::EStateRes eRes = eProcessParms(cursOtherParms);

        // If errors, then log and return a failed status
        if (eRes != tCQCSrvFW::EStateRes::Success)
        {
            if (cursOtherParms)
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_BadParm
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                    , *cursOtherParms
                );
            }
             else
            {
                facCQCKit().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kKitErrs::errcApp_ParmProcErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                );
            }
            return tCQCSrvFW::EStateRes::Failed;
        }
    }

    // Make sure the port got set
    if (!m_ippnListen)
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSrvFWErrs::errcSrvFW_NoSrvPort
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::BadParms
        );
        return tCQCSrvFW::EStateRes::Failed;
    }

    // If more clients than can be waited on, then clip it
    if (m_c4MaxClients > kCIDSock::c4MaxSelect)
    {
        m_c4MaxClients = kCIDSock::c4MaxSelect;
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcApp_ClippedMaxClients
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(m_c4MaxClients)
        );
    }


    // If verbose logging, then log that we completed the parms
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
    return tCQCSrvFW::EStateRes::Success;
}


//
//  We do the cleanup in the reverse order back down the ESrvStates enum
//
tCIDLib::TVoid TCQCSrvCore::DoCleanup(TThread& thrThis)
{
    //
    //  Start with our state enum whereever it got to, and start working backwards
    //  will we hit start. For human consumption we do the states in reverse order
    //  here, but ultimately it doesn't matter.
    //
    while (m_eState > tCQCSrvFW::ESrvStates::Start)
    {
        //  Unlike init, we don't give up if errors occur, we want to keep moving
        //  down through states and undo things.
        //
        try
        {
            switch(m_eState)
            {
                case tCQCSrvFW::ESrvStates::Ready :
                    // Log a shutting down message
                    facCQCSrvFW().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCSrvFWMsgs::midGen_Shutdown
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strDescr
                    );
                    break;

                case tCQCSrvFW::ESrvStates::EnableEvents :
                {
                    if (tCIDLib::bAnyBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::AllEvents))
                        facCQCKit().StopEventProcessing();
                    break;
                }

                case tCQCSrvFW::ESrvStates::StartRebinder :
                    // Stop the rebinder and ask the derived class to unbind objects
                    facCIDOrbUC().StopRebinder();
                    IntUnbindObjects();
                    break;

                case tCQCSrvFW::ESrvStates::PreBindInit :
                    // This is now a post un-bind term
                    PostUnbindTerm();
                    break;

                case tCQCSrvFW::ESrvStates::StartWorkers :
                    StopWorkerThreads();
                    break;

                case tCQCSrvFW::ESrvStates::RegSrvObjects :
                {
                    IntDeregObjects();
                    break;
                }

                case tCQCSrvFW::ESrvStates::PreRegInit :
                    // This is now a post deregister term
                    PostDeregTerm();
                    break;

                case tCQCSrvFW::ESrvStates::WaitPrereqs :
                    // Nothing to do on cleanup
                    break;


                case tCQCSrvFW::ESrvStates::LoadConfig :
                    // Let it save away any config changes
                    StoreConfig();
                    break;

                case tCQCSrvFW::ESrvStates::InitServerOrb :
                    // Nothing to do here, we'll undo the ORB at the client init stage
                    break;

                case tCQCSrvFW::ESrvStates::LogIn :
                    // Nothing to undo for this one
                    break;

                case tCQCSrvFW::ESrvStates::MakeDirs :
                    // Nothing to undo for this one
                    break;

                case tCQCSrvFW::ESrvStates::ParseParms :
                    // Nothing to undo for this one
                    break;

                case tCQCSrvFW::ESrvStates::InstallLogger :
                    // Force the logger back to local
                    if (m_plgrLogSrv)
                        m_plgrLogSrv->bForceLocal(kCIDLib::True);
                    break;

                case tCQCSrvFW::ESrvStates::InitClientOrb :
                    // Terminate the ORB
                    facCIDOrb().Terminate();

                    // If we installed a logger, tell it to clean up now
                    if (m_plgrLogSrv)
                        m_plgrLogSrv->Cleanup();
                    break;

                case tCQCSrvFW::ESrvStates::SetSigHandler :
                {
                    // Remove ourself as a signal handler
                    TSignals::RemoveHandler(this, m_c4SigHandlerId);
                    break;
                }

                case tCQCSrvFW::ESrvStates::CreateEvent :
                    // Clean up the event now if we created it
                    if (m_pevWait)
                    {
                        delete m_pevWait;
                        m_pevWait = nullptr;
                    }
                    break;


                case tCQCSrvFW::ESrvStates::LoadEnv :
                    // Nothing to undo here
                    break;

                default :
                    CIDAssert2X
                    (
                        L"Unknown CQC server framework state: %(2)"
                        , tCQCSrvFW::strXlatESrvStates(m_eState)
                    );
                    break;
            };

        }

        catch(TError& errToCatch)
        {
            // Log that this state failed
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);

            facCQCKit().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSrvFWErrs::errcSrvFW_TermError
                , tCQCSrvFW::strXlatESrvStates(m_eState)
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        catch(...)
        {
        }

        // Move to the previous state and go again
        m_eState--;
    }
}


//
//  Runs us up through the initialization steps, until we either get to Ready or
//  fail or are asked to stop. We just increment the m_eState value as we get
//  though each init step.
//
//  Some of them can ask to retry and we wait and go back around on the same state
//  again.
//
tCIDLib::TVoid TCQCSrvCore::DoInit(TThread& thrThis)
{
    //
    //  We only need one try, since we give up if an error occurs that we don't deal
    //  specifially.
    //
    try
    {
        //
        //  For use below in those cases where things can be retried. We tell the
        //  derived class how many times it's happened, since he may want to give
        //  up after a while.
        //
        tCIDLib::TBoolean bForceExit = kCIDLib::False;
        tCIDLib::TCard4 c4RoundCnt;
        while ((m_eState < tCQCSrvFW::ESrvStates::Ready) && !bForceExit)
        {
            tCIDLib::TCard4 c4WaitNextMSs = 0;
            tCQCSrvFW::EStateRes eRes = tCQCSrvFW::EStateRes::Success;

            switch(m_eState)
            {
                case tCQCSrvFW::ESrvStates::Start :
                    // This is our entry state. We don't need to do anything here currently
                    break;

                case tCQCSrvFW::ESrvStates::LoadEnv :
                {
                    TString strErr;
                    if (!facCQCKit().bLoadEnvInfo(strErr, kCIDLib::False))
                    {
                        ThrowErr
                        (
                            CID_FILE
                            , CID_LINE
                            , kCQCSrvFWErrs::errcSrvFW_LoadEnvInfo
                            , tCIDLib::ESeverities::Failed
                            , tCIDLib::EErrClasses::Init
                            , strErr
                        );
                    }
                    break;
                }

                case tCQCSrvFW::ESrvStates::CreateEvent :
                    //
                    //  This will throw if it fails. The most likely scenario is that
                    //  it already exits, which means this program is already running.
                    //
                    //  It needs to be initially reset, so we just block on it after
                    //  init, and we don't iterfere with someone else triggering it
                    //  after we get it created.
                    //
                    m_pevWait = new TEvent
                    (
                        TResourceName
                        (
                            kCIDLib::pszResCompany, m_strEvName, kCIDOrbUC::pszEvResource
                        )
                        , tCIDLib::EEventStates::Reset
                        , tCIDLib::ECreateActs::CreateIfNew
                    );
                    break;

                case tCQCSrvFW::ESrvStates::SetSigHandler :
                    // Register ourself as a signal handler and save the id
                    m_c4SigHandlerId = TSignals::c4AddHandler(this);
                    break;

                case tCQCSrvFW::ESrvStates::InitClientOrb :
                    // Let us talk to other servers at this point
                    facCIDOrb().InitClient();
                    break;

                case tCQCSrvFW::ESrvStates::InstallLogger :
                    //
                    //  Install the standard log server logger, wtih the environmentally
                    //  driven local fallback path.
                    //
                    m_plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
                    TModule::InstallLogger(m_plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

                    // Log a startup message now
                    facCQCSrvFW().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCSrvFWMsgs::midGen_Startup
                        , tCIDLib::ESeverities::Status
                        , tCIDLib::EErrClasses::AppStatus
                        , m_strDescr
                    );
                    break;

                case tCQCSrvFW::ESrvStates::ParseParms :
                    eRes = eIntProcessParms();
                    break;

                case tCQCSrvFW::ESrvStates::MakeDirs :
                    // Let the derived class create any directories he needs
                    MakeDirs();
                    break;

                case tCQCSrvFW::ESrvStates::LogIn :
                    // If the derived class indicates he needs to be logged on do that
                    if (tCIDLib::bAllBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::LogIn))
                    {
                        eRes = eLogin(c4WaitNextMSs, ++c4RoundCnt);
                    }
                    break;

                case tCQCSrvFW::ESrvStates::InitServerOrb :
                    facCIDOrb().InitServer(m_ippnListen, m_c4MaxClients);
                    break;

                case tCQCSrvFW::ESrvStates::LoadConfig :
                    eRes = eLoadConfig(c4WaitNextMSs, ++c4RoundCnt);
                    break;

                case tCQCSrvFW::ESrvStates::WaitPrereqs :
                    eRes = eWaitPrereqs(c4WaitNextMSs);
                    break;

                case tCQCSrvFW::ESrvStates::PreRegInit :
                    //
                    //  Let the derievd class do any init before we start registering
                    //  server side objects.
                    //
                    PreRegInit();
                    break;

                case tCQCSrvFW::ESrvStates::RegSrvObjects :
                    // Register our server objects and let the derived class do so
                    IntRegObjects();
                    break;

                case tCQCSrvFW::ESrvStates::StartWorkers :
                    StartWorkerThreads();
                    break;

                case tCQCSrvFW::ESrvStates::PreBindInit :
                    //
                    //  Let the derievd class do any init just before we are going
                    //  to start the rebinder and make the server visible publically.
                    //
                    PreBindInit();
                    break;

                case tCQCSrvFW::ESrvStates::StartRebinder :
                    facCIDOrbUC().StartRebinder();
                    break;

                case tCQCSrvFW::ESrvStates::EnableEvents :
                {
                    if (tCIDLib::bAnyBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::AllEvents))
                    {
                        //
                        //  We have to get the event key, which requires talking to the
                        //  the client service. This in turn requires that we have valid
                        //  user credentials.
                        //
                        eRes = eEnableEvents();
                    }
                    break;
                }

                default :
                    //
                    //  We won't see ready here since we'll fall at the top after we inc
                    //  at the bottom.
                    //
                    CIDAssert2X
                    (
                        L"Unknown CQC server framework state: %(2)"
                        , tCQCSrvFW::strXlatESrvStates(m_eState)
                    );
                    break;
            };

            // If a failure, then break out
            if (eRes == tCQCSrvFW::EStateRes::Failed)
                break;

            if (eRes == tCQCSrvFW::EStateRes::Retry)
            {
                //
                //  If it was retry, then sleep for the indicated time, watching for
                //  shutdown requests. We stay on the same state.
                //
                //  Just in case, make sure they set a timeout. If not, set one for
                //  15 seconds, which should never be abusive. If they set one less
                //  than 1 seconds, set it to one.
                //
                if (!c4WaitNextMSs)
                    c4WaitNextMSs = kCIDLib::c4OneSecond * 15;
                else if (c4WaitNextMSs < kCIDLib::c4OneSecond)
                    c4WaitNextMSs = kCIDLib::c4OneSecond;

                if (!thrThis.bSleep(c4WaitNextMSs))
                    break;
            }
             else if (eRes == tCQCSrvFW::EStateRes::Success)
            {
                //
                //  Do a shutdown request check and break out if so. We aren't blocked
                //  on the shutdown event yet, so we'd never know if we were being asked
                //  to shutdown until we return from this init stuff otherwise.
                //
                if (thrThis.bCheckShutdownRequest())
                    break;

                // Else we move to the next state and log that if verbose
                m_eState++;

                // Reset the round count in case it's used in this next state
                c4RoundCnt = 0;
            }
        }

        // If we are ready, then log that
        if (m_eState == tCQCSrvFW::ESrvStates::Ready)
        {
            facCQCSrvFW().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCSrvFWMsgs::midGen_Ready
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
                , m_strDescr
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);

        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSrvFWErrs::errcSrvFW_InitFailed
            , tCQCSrvFW::strXlatESrvStates(m_eState)
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        m_eReturn = tCIDLib::EExitCodes::FatalError;
    }

    catch(...)
    {
        facCQCKit().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCSrvFWErrs::errcSrvFW_InitFailed
            , tCQCSrvFW::strXlatESrvStates(m_eState)
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
        m_eReturn = tCIDLib::EExitCodes::FatalError;
    }

    //
    //  If we failed, then make sure the wait event gets triggered. Else we will
    //  just block upon return.
    //
    if (m_eReturn != tCIDLib::EExitCodes::Normal)
    {
        if (m_pevWait)
            m_pevWait->Trigger();
    }
}



tCQCSrvFW::EStateRes TCQCSrvCore::eEnableEvents()
{
    tCQCKit::EEvProcTypes eEvents = tCQCKit::EEvProcTypes::None;
    if (tCIDLib::bAllBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::EventsIn))
        eEvents = tCIDLib::eOREnumBits(eEvents, tCQCKit::EEvProcTypes::Receive);
    if (tCIDLib::bAllBitsOn(m_eOptions, tCQCSrvFW::ESrvOpts::EventsOut))
        eEvents = tCIDLib::eOREnumBits(eEvents, tCQCKit::EEvProcTypes::Send);

    if ((eEvents | tCQCKit::EEvProcTypes::None) == tCQCKit::EEvProcTypes::None)
    {
        // Just say we did it, though we are not going to do anything
        return tCQCSrvFW::EStateRes::Success;
    }


    facCQCKit().StartEventProcessing(eEvents, m_cuctxToUse.sectUser());
    return tCQCSrvFW::EStateRes::Success;
}


//
//  Our cleanup calls this. We deregister our ORB objects and then let
//  the derived class do the same.
//
tCIDLib::TVoid TCQCSrvCore::IntDeregObjects()
{
    try
    {
        facCIDOrb().DeregisterObject(m_porbsCoreAdmin);
        DeregisterSrvObjs();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
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


//
//  Called by our setup method. We register bind our core admin interface, and then
//  let the derived class do his.
//
tCIDLib::TVoid TCQCSrvCore::IntRegObjects()
{
    try
    {
        // Let the derived class do his
        RegisterSrvObjs();

        // He didn't throw, so do the core admin
        m_porbsCoreAdmin = new TCIDCoreAdminImpl(this);
        facCIDOrb().RegisterObject(m_porbsCoreAdmin, tCIDLib::EAdoptOpts::Adopt);

        TString strExtra1;
        TString strExtra2;
        TString strExtra3;
        TString strExtra4;
        TString strDescr;
        QueryCoreAdminInfo
        (
            m_strAdminBinding, strDescr, strExtra1, strExtra2, strExtra3, strExtra4
        );

        // The binding might have a host name resplacement token
        m_strAdminBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);
        facCIDOrbUC().RegRebindObj
        (
            m_porbsCoreAdmin->ooidThis()
            , m_strAdminBinding
            , strDescr
            , strExtra1
            , strExtra2
            , strExtra3
            , strExtra4
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


//
//  Our cleanup calls this. We unbind our ORB objects and then let
//  the derived class do the same.
//
tCIDLib::TVoid TCQCSrvCore::IntUnbindObjects()
{
    try
    {
        //
        //  Get us a name server proxy to use for all of this stuff. We use a
        //  short timeout so we don't hold up shutdown if there's an issue.
        //
        tCIDOrbUC::TNSrvProxy orbcNS = facCIDOrbUC().orbcNameSrvProxy(2500);

        // Do our base admin
        orbcNS->RemoveBinding(m_strAdminBinding, kCIDLib::False);
        UnbindSrvObjs(orbcNS);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
        LogMsg
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcApp_NSCleanup
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}
