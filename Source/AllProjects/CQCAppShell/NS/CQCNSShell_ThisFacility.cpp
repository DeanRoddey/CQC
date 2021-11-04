//
// FILE NAME: CQCNSShell_ThisFacility.cpp
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
//  This file implements
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCNSShell.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCNSShell,TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCNSShell
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCNSShell: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  Note that we use the same version constants as the toolkit facility. Since
//  they must be used together, this makes sense.
//
TFacCQCNSShell::TFacCQCNSShell() :

    TFacility
    (
        L"CQCNSShell"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::None
    )
    , m_evShutdown(tCIDLib::EEventStates::Reset)
{
}

TFacCQCNSShell::~TFacCQCNSShell()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCNSShell: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFacCQCNSShell::bHandleSignal(const tCIDLib::ESignals)
{
    // Trigger our shutdown event to cause the main thread to exit
    m_evShutdown.Trigger();
    return kCIDLib::True;
}


tCIDLib::TBoolean
TFacCQCNSShell::bQueryNewMsgs(         tCIDLib::TCard4&    c4MsgIndex
                                ,       TVector<TString>&   colMsgs
                                , const tCIDLib::TBoolean   bAddNewLine)
{
    // Just delegate to the engine
    return facCQCAppShellLib().bQueryNewMsgs(c4MsgIndex, colMsgs, bAddNewLine);
}


tCIDLib::TCard4 TFacCQCNSShell::c4QueryApps(tCIDLib::TKVPList& colApps)
{
    // Just delegate to the app engine
    return facCQCAppShellLib().c4QueryApps(colApps);
}


tCIDLib::TVoid TFacCQCNSShell::CycleApps()
{
    // Just delegate to the app shell engine
    facCQCAppShellLib().CycleApps();
}


tCIDLib::EExitCodes
TFacCQCNSShell::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    // Get the first parm. If not 'Booble', then just exit
    const tCIDLib::TCard4 c4ParmCnt = TSysInfo::c4CmdLineParmCount();
    if ((c4ParmCnt < 1) || (c4ParmCnt > 2))
        return tCIDLib::EExitCodes::Normal;

    TString strParm = TSysInfo::strCmdLineParmAt(0);
    if (strParm != L"Booble")
        return tCIDLib::EExitCodes::Normal;

    // Register ourself as a signal handler
    TSignals::c4AddHandler(this);

    // Show our blurb to the console
    ShowBlurb();

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

    //
    //  Initialize the engine. Generally we'd point it at the same directory
    //  as what we are running in, but for debug purposes, we allow it to be
    //  set on the command line as a second parameter. If not there, then we
    //  use our directory.
    //
    TString strFilePath;
    if (c4ParmCnt == 2)
        strFilePath = TSysInfo::strCmdLineParmAt(1);
    else
        strFilePath = strPath();

    m_conOut << L"Initilizing app shell\n  Path="
             << strFilePath
             << kCIDLib::EndLn;
    facCQCAppShellLib().Initialize(strFilePath);

    // And now block forever until asked to stop
    m_evShutdown.WaitFor();

    // Ask the engine to stop
    m_conOut << L"Stopping app shell" << kCIDLib::EndLn;
    facCQCAppShellLib().StopEngine();

    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TFacCQCNSShell::StartAllApps()
{
    // Tell the engine to activate all the apps
    m_conOut << L"Starting up all apps" << kCIDLib::EndLn;
    facCQCAppShellLib().StartAllApps();
}


tCIDLib::TVoid TFacCQCNSShell::StopAllApps()
{
    // Tell the engine to stop all the apps
    m_conOut << L"Stopping all apps" << kCIDLib::EndLn;
    facCQCAppShellLib().StopAllApps();
}



// ---------------------------------------------------------------------------
//  TFacCQCNSShell: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TFacCQCNSShell::ShowBlurb()
{
    // Show the general blurb line, and a version
    m_conOut    << facCQCAppShellLib().strMsg(kCQCShMsgs::midGen_Blurb)
                << facCQCAppShellLib().strMsg(kCQCShMsgs::midGen_Version)
                << strVersion()
                << kCIDLib::NewLn
                << facCQCAppShellLib().strMsg(kCQCShMsgs::midGen_StopInfo)
                << kCIDLib::EndLn;
}


