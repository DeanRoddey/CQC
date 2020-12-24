//
// FILE NAME: CQCProtoDev_ThisFacility.cpp
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
#include    "CQCProtoDev.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCQCProtoDev,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCQCProtoDev
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCProtoDev: Constructors and operators
// ---------------------------------------------------------------------------
TFacCQCProtoDev::TFacCQCProtoDev() :

    TGUIFacility
    (
        L"CQCProtoDev"
        , tCIDLib::EModTypes::Exe
        , kCIDLib::c4MajVersion
        , kCIDLib::c4MinVersion
        , kCIDLib::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_bSystem(kCIDLib::False)
    , m_strAppTitle(kProtoDevMsgs::midMsg_Title, facCQCProtoDev)
{
}

TFacCQCProtoDev::~TFacCQCProtoDev()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCProtoDev: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TFacCQCProtoDev::bSystem() const
{
    return m_bSystem;
}


tCIDLib::EExitCodes
TFacCQCProtoDev::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    //
    //  Ask CQCKit to load core environment/parm stuff. If we can't do this,
    //  then we are doomed and just have to exit.
    //
    TString strFailReason;
    if (!facCQCKit().bLoadEnvInfo(strFailReason, kCIDLib::True))
        return tCIDLib::EExitCodes::BadEnvironment;

    TLogSrvLogger*          plgrLogSrv = nullptr;
    TCQCProtoDevFrameWnd    wndMainFrame;
    try
    {
        //
        //  First thing of all, check to see if there is already an instance
        //  running. If so, activate it and just exit.
        //
        TResourceName rsnInstance(L"CQSL", L"CQCProtoDev", L"SingleInstanceInfo");
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

        // We have to log on before we can do anything
        if (!bDoLogon())
            return tCIDLib::EExitCodes::Normal;

        // Initialize the local config store
        facCQCGKit().InitObjectStore
        (
            kCQCProtoDev::pszStoreKey, m_cuctxToUse.strUserName(), bNoState
        );

        // Register any local serial port factories
        facGC100Ser().bUpdateFactory(m_cuctxToUse.sectUser());
        facJAPwrSer().bUpdateFactory(m_cuctxToUse.sectUser());

        // And create the main frame now
        wndMainFrame.Create();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , m_strAppTitle
            , strMsg(kProtoDevMsgs::midMsg_Exception)
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        TErrBox msgbErr
        (
            m_strAppTitle, strMsg(kProtoDevMsgs::midMsg_UnknownExcept)
        );
        msgbErr.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::InitFailed;
    }

    facCIDCtrls().uMainMsgLoop(wndMainFrame);

    try
    {
        wndMainFrame.Destroy();
    }

    catch(const TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
            LogEventObj(errToCatch);
    }

    try
    {
        // Terminate the local object store
        facCQCGKit().TermObjectStore();

        // Force the log server logger to local logging, and terminate the orb
        if (plgrLogSrv)
            plgrLogSrv->bForceLocal(kCIDLib::True);

        // Terminate the Orb support
        facCIDOrb().Terminate();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        LogEventObj(errToCatch);
    }

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TFacCQCProtoDev: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method runs the standard CQC logon dialog. If it succeeds, we get
//  back the security token for the user, which we set on CQCKit for
//  subsequent use by the app.
//
tCIDLib::TBoolean TFacCQCProtoDev::bDoLogon()
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
                            , m_strAppTitle
                            , kCIDLib::False
                            , L"DriverIDE"))
    {
        // Ok, they can log on, so set up our user context
        m_cuctxToUse.Set(uaccLogin, sectTmp);
        m_cuctxToUse.LoadEnvRTVsFromHost();
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TFacCQCProtoDev::bParseParms(tCIDLib::TBoolean& bIgnoreState)
{
    //
    //  Most of our parms are handled by CQCKit, but we support a /NoState
    //  flag, to ignore the saved state and start with default window
    //  positions. Assume not to ignore unless told to.
    //
    bIgnoreState = kCIDLib::False;

    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        if (cursParms->bCompareI(L"/NoState"))
            bIgnoreState = kCIDLib::True;
        else if (cursParms->bCompareI(L"/System"))
            m_bSystem = kCIDLib::True;
    }
    return kCIDLib::True;
}
