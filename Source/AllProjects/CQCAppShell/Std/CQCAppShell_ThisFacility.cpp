//
// FILE NAME: CQCAppShell_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/30/2001
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
//  This file implements the more generic parts of the facility class for the
//  CQC app shell.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAppShell.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCAppShell,TFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCAppShell
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCAppShell: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCAppShell::TFacCQCAppShell() :

    TFacility
    (
        L"CQCAppShell"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::None
    )
    , m_c4MaxClients(0)
    , m_evShutdown(tCIDLib::EEventStates::Reset)
    , m_ippnListen(kCQCKit::ippnSrvShellDefPort)
    , m_porbsAdmin(0)
{
}

TFacCQCAppShell::~TFacCQCAppShell()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCAppShell: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TFacCQCAppShell::bQueryNewMsgs(         tCIDLib::TCard4&    c4MsgIndex
                                ,       TVector<TString>&   colMsgs
                                , const tCIDLib::TBoolean   bAddNewLine)
{
    // Just delegate to the engine
    return facCQCAppShellLib().bQueryNewMsgs(c4MsgIndex, colMsgs, bAddNewLine);
}


tCIDLib::TCard4 TFacCQCAppShell::c4QueryApps(tCIDLib::TKVPList& colApps)
{
    // Just delegate to the app engine
    return facCQCAppShellLib().c4QueryApps(colApps);
}


tCIDLib::TVoid TFacCQCAppShell::CycleApps()
{
    // Just delegate to the app shell engine
    facCQCAppShellLib().CycleApps();
}


tCIDLib::EExitCodes
TFacCQCAppShell::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    try
    {
        //
        //  Set the environment variable that the stack dump program
        //  uses to create dump files, so that we insure that they get
        //  into the right spot. So get our target path, remove the Bin
        //  part, then add on the logs directory.
        //
        TPathStr pathDumpDir = strPath();
        pathDumpDir.bRemoveLevel();
        pathDumpDir.AddLevel(L"\\CQCData\\Logs");
        TProcEnvironment::bAddOrUpdate(kCIDLib::pszErrDumpDir, pathDumpDir);

        // Initialize the client side of the Orb support.
        facCIDOrb().InitClient();

        // Parse our command line parms
        ParseParms();

        //
        //  Initialize the server side of the Orb support, which relies
        //  on parameter info potentially.
        //
        facCIDOrb().InitServer(m_ippnListen, m_c4MaxClients);

        // Initialize the engine. Give him our path to find the list file
        facCQCAppShellLib().Initialize(strPath());

        //
        //  Build up the binding name for our admin object. It is based
        //  on the host name.
        //
        const TString strOurHost(TSysInfo::strIPHostName());
        TString strBinding(TCQCAppShellAdminServerBase::strScope);
        strBinding.Append(L"/");
        strBinding.Append(strOurHost);

        //
        //  And now create our server object, register it with the ORB, and
        //  add it to the rebinder.
        //
        m_porbsAdmin = new TCQCAppShellImpl;
        facCIDOrb().RegisterObject(m_porbsAdmin, tCIDLib::EAdoptOpts::Adopt);
        facCIDOrbUC().RegRebindObj
        (
            m_porbsAdmin->ooidThis()
            , strBinding
            , facCQCAppShellLib().strMsg(kCQCShMsgs::midScope_AdminScopeDescr, strOurHost)
        );

        // Start up the auto-rebinder
        facCIDOrbUC().StartRebinder();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
    }

    catch(...)
    {
    }

    //
    //  And now block forever until asked to stop. We wake up periodically
    //  and refresh the stay active state, so the system can't go to sleep
    //  due to inactivity.
    //
    while (kCIDLib::True)
    {
        // Wait a bit. If triggered, break out
        if (m_evShutdown.bWaitFor(5000))
            break;

        // Refresh the active state
        TKrnlWin32Service::RefreshStayAwake();
    }

    // Ask the engine to stop
    facCQCAppShellLib().StopEngine();

    //
    //  Remove any name server bindings we have, and shut down the ORB. We
    //  do this both to prevent any client activity while we are shutting
    //  stuff down, and because the name server might actually be one of the
    //  apps we are shutting down.
    //
    try
    {
        RemoveBindings();
        facCIDOrb().Terminate();
    }

    catch(const TError& errToCatch)
    {
        if (facCQCAppShellLib().bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        if (facCQCAppShellLib().bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_StopORB
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }

    catch(...)
    {
        if (facCQCAppShellLib().bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_StopORB
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }

    // Shut down all processes
    try
    {
        facCQCAppShellLib().StopEngine();
    }

    catch(const TError& errToCatch)
    {
        if (facCQCAppShellLib().bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        if (facCQCAppShellLib().bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_ErrShutdown
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }

    catch(...)
    {
        if (facCQCAppShellLib().bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_ErrShutdown
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }
    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TFacCQCAppShell::StartAllApps()
{
    // Tell the engine to activate all the apps
    facCQCAppShellLib().StartAllApps();
}


tCIDLib::TVoid TFacCQCAppShell::StartShutdown()
{
    // Just trigger the event that the main thread is waiting on
    m_evShutdown.Trigger();
}


tCIDLib::TVoid TFacCQCAppShell::StopAllApps()
{
    // Tell the engine to stop all the apps
    facCQCAppShellLib().StopAllApps();
}



// ---------------------------------------------------------------------------
//  TFacCQCAppShell: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Called from the main thread on startup to parse command line parms
tCIDLib::TVoid TFacCQCAppShell::ParseParms()
{
    if (bLogInfo())
    {
        facCQCAppShellLib().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCShMsgs::midStatus_StartParms
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }

    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    TString strCurParm;
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        // We know what is by what it starts with
        if (strCurParm.bStartsWithI(L"/Port="))
        {
            strCurParm.Cut(0, 6);
            m_ippnListen = strCurParm.c4Val();
        }
         else if (strCurParm.bStartsWithI(L"/Max="))
        {
            strCurParm.Cut(0, 5);
            m_c4MaxClients = strCurParm.c4Val();
        }
         else
        {
            if (bLogWarnings())
            {
                facCQCAppShellLib().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCShMsgs::midStatus_UnknownParm
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::BadParms
                    , strCurParm
                );
            }
        }
    }

    if (bLogInfo())
    {
        facCQCAppShellLib().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kCQCShMsgs::midStatus_ParmsComplete
            , tCIDLib::ESeverities::Info
            , tCIDLib::EErrClasses::AppStatus
        );
    }
}


// Called at shutdown to remove our name server bindings
tCIDLib::TVoid TFacCQCAppShell::RemoveBindings()
{
    // Get a client name server proxy
    tCIDOrbUC::TNSrvProxy orbcNS;

    try
    {
        //
        //  Shut down the rebinder before we start removing bindings.
        //  Otherwise, he'll just start rebinding them again.
        //
        facCIDOrbUC().StopRebinder();

        // And get us a name server proxy to use for all of this stuff
        orbcNS = facCIDOrbUC().orbcNameSrvProxy();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        // If we can't get the NS proxy, we can't do much
        if (bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_NSCleanup
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
        return;
    }

    // First try to remove our admin object
    try
    {
        TString strBinding(TCQCAppShellAdminServerBase::strScope);
        strBinding.Append(L"/");
        strBinding.Append(TSysInfo::strIPHostName());
        orbcNS->RemoveBinding(strBinding, kCIDLib::False);
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);

        if (bLogFailures())
        {
            facCQCAppShellLib().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kCQCShMsgs::midStatus_UnbindAdmin
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Term
            );
        }
    }
}


