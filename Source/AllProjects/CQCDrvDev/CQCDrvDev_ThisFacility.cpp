//
// FILE NAME: CQCDrvDev_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/14/2003
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
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCDrvDev.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCDrvDev,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCDrvDev
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCDrvDev: Constructors and operators
// ---------------------------------------------------------------------------
TFacCQCDrvDev::TFacCQCDrvDev() :

    TGUIFacility
    (
        L"CQCDrvDev"
        , tCIDLib::EModTypes::Exe
        , kCIDLib::c4MajVersion
        , kCIDLib::c4MinVersion
        , kCIDLib::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
{
    m_strTitleText = strMsg(kDrvDevMsgs::midMsg_Title);
}

TFacCQCDrvDev::~TFacCQCDrvDev()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCDrvDev: Public, non-virtual methods
// ---------------------------------------------------------------------------
const TCQCUserCtx& TFacCQCDrvDev::cuctxToUse() const
{
    return m_cuctxToUse;
}


tCIDLib::EExitCodes TFacCQCDrvDev::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
    {
        TErrBox msgbTest(m_strTitleText, strMsg(kDrvDevMsgs::midStatus_BadEnv, strFailReason));
        msgbTest.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::BadEnvironment;
    }

    TLogSrvLogger* plgrLogSrv = nullptr;
    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit.
        //
        TResourceName rsnInstance(L"CQSL", L"CQCDrvDev", L"SingleInstanceInfo");
        if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::True))
            return tCIDLib::EExitCodes::Normal;

        //
        //  Now we can initialize the client side ORB, which we have to
        //  do, despite being mostly standalone, because of the need to
        //  read/write macros from the data server.
        //
        facCIDOrb().InitClient();

        //
        //  The next thing we want to do is to install a log server logger. We
        //  just use the standard one that's provided by CIDLib. It logs to
        //  the standard CIDLib log server, and uses a local file for fallback
        //  if it cannot connect.
        //
        plgrLogSrv = new TLogSrvLogger(facCQCKit().strLocalLogDir());
        TModule::InstallLogger(plgrLogSrv, tCIDLib::EAdoptOpts::Adopt);

        // Parse the command line parms
        tCIDLib::TBoolean bNoState = kCIDLib::False;
        if (!bParseParms(bNoState))
            return tCIDLib::EExitCodes::BadParameters;

        // Do a version check against the master server and exit if not valid
        if (!facCQCIGKit().bCheckCQCVersion(TWindow::wndDesktop(), m_strTitleText))
            return tCIDLib::EExitCodes::InitFailed;

        // We have to log on before we can do anything. If that works
        if (!bDoLogon())
            return tCIDLib::EExitCodes::Normal;

        // Initialize the local config store
        facCQCGKit().InitObjectStore
        (
            kCQCDrvDev::strStoreKey, m_cuctxToUse.strUserName(), bNoState
        );

        //
        //  We have to install some class loaders on the macro engine
        //  facility, so that the standard CQC runtime classes and driver
        //  classes are available.
        //
        facCQCMEng().InitCMLRuntime(m_cuctxToUse.sectUser());
        facCQCMedia().InitCMLRuntime();
        facCQCGenDrvS().InitCMLRuntime();

        //
        //  Enable the sending out of events, so that any user action events
        //  the driver might send out can be tested.
        //
        facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Send, m_cuctxToUse.sectUser());

        //
        //  Register any local serial port factories we want to make available.
        //
        facGC100Ser().bUpdateFactory(m_cuctxToUse.sectUser());
        facJAPwrSer().bUpdateFactory(m_cuctxToUse.sectUser());

        //
        //  Load any previous config we might have had (assuming it wasn't
        //  tossed via the 'no state' command line parm above.
        //
        TCQCFrameState fstMain;
        facCQCGKit().bReadFrameState(kCQCDrvDev::strCfgKey_Frame, fstMain, TSize(640, 480));
        m_wndMainFrame.Create(fstMain);
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , m_strTitleText
            , strMsg(kDrvDevMsgs::midMsg_Exception)
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        TErrBox msgbErr(m_strTitleText, strMsg(kDrvDevMsgs::midMsg_UnknownExcept));
        msgbErr.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::InitFailed;
    }

    //
    //  Call the window processing loop. We won't come back until the
    //  program exits. No exceptions will come out of here, so we don't
    //  have to protect it. We use the MDI enabled loop here.
    //
    facCIDCtrls().uMainMsgLoop(m_wndMainFrame);

    try
    {
        m_wndMainFrame.Destroy();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    // Terminate the local config store
    try
    {
        facCQCGKit().TermObjectStore();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    // Force the log server logger to local logging, and terminate the orb
    try
    {
        if (plgrLogSrv)
            plgrLogSrv->bForceLocal(kCIDLib::True);
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
    }

    // And finally make the logger stop logging
    try
    {
        if (plgrLogSrv)
            plgrLogSrv->Cleanup();
    }

    catch(...)
    {
    }

    try
    {
        // Turn event processing back off
        facCQCKit().StopEventProcessing();
    }

    catch(...)
    {
    }

    return tCIDLib::EExitCodes::Normal;
}



// ---------------------------------------------------------------------------
//  TFacCQCDrvDev: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method runs the standard CQC logon dialog. If it succeeds, we get
//  back the security token for the user, which we set on CQCKit for subsequent
//  use by the app.
//
tCIDLib::TBoolean TFacCQCDrvDev::bDoLogon()
{
    //
    //  We'll get a temp security token, which we'll store on the CQCKit
    //  facility if we succeed, and it'll fill in a user account object of
    //  ours, which we'll use for a few things. The last parameter allows
    //  environmentally based logon.
    //
    TCQCUserAccount uaccLogin;
    TCQCSecToken    sectTmp;
    if (facCQCGKit().bLogon(TWindow::wndDesktop()
                            , sectTmp
                            , uaccLogin
                            , tCQCKit::EUserRoles::SystemAdmin
                            , m_strTitleText
                            , kCIDLib::False
                            , L"DriverIDE"))
    {
        //
        //  Ok, they can log on, so let's set up our user context for
        //  later user.
        //
        m_cuctxToUse.Set(uaccLogin, sectTmp);
        m_cuctxToUse.LoadEnvRTVsFromHost();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean TFacCQCDrvDev::bParseParms(tCIDLib::TBoolean& bIgnoreState)
{
    //
    //  Most of our parms are handled by CQCKit, but we support a /NoState flag, to ignore
    //  the saved state and start with default window positions. Assume not to ignore unless
    //  told to.
    //
    bIgnoreState = kCIDLib::False;

    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        if (cursParms->bCompareI(L"/NoState"))
            bIgnoreState = kCIDLib::True;
    }
    return kCIDLib::True;
}

