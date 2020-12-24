//
// FILE NAME: CppDrvTest_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2018
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
//  Provides some general functionality not related to any of our classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include "CppDrvTest.hpp"

// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacCppDrvTest,TGUIFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacCppDrvTest
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCppDrvTest: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCppDrvTest::TFacCppDrvTest() :

    TGUIFacility
    (
        L"CppDrvTest"
        , tCIDLib::EModTypes::Exe
        , kCIDLib::c4MajVersion
        , kCIDLib::c4MinVersion
        , kCIDLib::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_eLastVerbosity(tCQCKit::EVerboseLvls::Off)
    , m_ippnListen(35781)
    , m_pfacSrvDrv(nullptr)
    , m_psdrvTest(nullptr)
    , m_strTitleText(strMsg(kCppDTMsgs::midGen_Title))
{
}

TFacCppDrvTest::~TFacCppDrvTest()
{
}


// ---------------------------------------------------------------------------
//  TFacCppDrvTest: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to load up the server driver facility and start it. We tell the
//  server driver tab that it's running, so that it starts updating.
//
//  If there is a client side driver it will load that up into the client driver tab.

//  We load and unload the driver facilities on each start/stop in order to make sure
//  that we don't keep the DLLs locked (else changes could not be compiled without
//  shutting down the test harness) and to insure that we get new changes if they
//  are made in between runs.
//
tCIDLib::TBoolean TFacCppDrvTest::bStartDriver()
{
    // Stop and drop the driver if needed, though it shouldn't be
    if (!bStopDriver())
        return kCIDLib::False;

    // We'll load into local temps and store only if it all works
    TFacility*      pfacDrv = nullptr;
    TCQCServerBase* psdrvTest = nullptr;

    //
    //  Get the library name out of the manifest and load it
    //
    //  NOTE:   We use the result directory here, we do not do what CQCServer does,
    //          which is to call facCQCKit().strServerDriverDir() to get the path
    //          where drivers are downloaded to from the master server. We want to
    //          load them from where they build to.
    //
    TString strPortName;
    TString strLoadName;
    TModule::BuildModName
    (
        m_cqcdcOrg.strServerLibName()
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , tCIDLib::EModTypes::SharedLib
        , strPortName
        , strLoadName
    );

    pfacDrv = new TFacility
    (
        strLoadName
        , facCppDrvTest.strResDir()
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , 0
        , tCIDLib::EModFlags::None
    );

    // Make sure it gets cleaned up if we fail
    TJanitor<TFacility> janFac(pfacDrv);

    //
    //  Use the temp facility object to load up the exported driver factory
    //  function, and ask it to create an instance of its driver for us.
    //
    try
    {
        tCQCKit::TSrvDriverFuncPtr pfnDriverFunc = tCQCKit::TSrvDriverFuncPtr
        (
            pfacDrv->pfnGetFuncPtr(kCQCKit::pszSrvFunc)
        );
        psdrvTest = (pfnDriverFunc)(m_cqcdcSession);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgShowErr
        (
            m_wndMainFrame
            , m_strTitleText
            , L"Failed to load driver entry point"
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        TErrBox msgbErr(m_strTitleText, L"Failed to load driver entry point");
        msgbErr.ShowIt(m_wndMainFrame);
        return kCIDLib::False;
    }

    // Make sure the driver gets cleaned up if we fail
    TJanitor<TCQCServerBase> janDrv(psdrvTest);

    //
    //  Set the OOID of our faux admin object on him as what he should register as his
    //  interface. That will cause him to register our object.
    //
    psdrvTest->SetSrvAdminOId(m_porbsDrvAdmin->ooidThis());

    //
    //  Set the last verbosity selected by the user as the initial verbosity to
    //  pick up.
    //
    psdrvTest->SetNewVerbosity(m_eLastVerbosity);

    //
    //  Tell it to initalize itself. Normally the driver does this in its
    //  own thread, but here we want to just try it once and give up if it
    //  fails.
    //
    tCQCKit::EDrvInitRes eRes = tCQCKit::EDrvInitRes::Failed;
    try
    {
        eRes = psdrvTest->eInitialize();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgShowErr
        (
            m_wndMainFrame
            , m_strTitleText
            , L"Failed to initialize driver"
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        TErrBox msgbErr(m_strTitleText, L"Failed to initialize driver");
        msgbErr.ShowIt(m_wndMainFrame);
        return kCIDLib::False;
    }

    // If he indicated he can't init, then give up
    if (eRes == tCQCKit::EDrvInitRes::Failed)
    {
        TErrBox msgbErr(m_strTitleText, L"Driver returned failure from init");
        msgbErr.ShowIt(m_wndMainFrame);
        return kCIDLib::False;
    }

    // Ok, start up the driver thread now
    psdrvTest->StartPoll();

    // Give our faux driver admin ORB interface a pointer
    m_porbsDrvAdmin->UpdateDrvPtr(psdrvTest);

    //
    //  Orphan the objects now to our members. This now means we have a running
    //  driver.
    //
    m_pfacSrvDrv = janFac.pobjOrphan();
    m_psdrvTest = janDrv.pobjOrphan();

    return kCIDLib::True;
}



//
//  When the session is stopped, we need to drop the driver so that changes can be
//  made and built (we aren't holding the facility DLL open.)
//
tCIDLib::TBoolean TFacCppDrvTest::bStopDriver()
{
    // Make sure to clear it from our faux server side driver admin ORB interface
    m_porbsDrvAdmin->UpdateDrvPtr(nullptr);

    //
    //  Drop the current one if we have them. First we need to stop the driver if it
    //  is currently running.
    //
    if (m_psdrvTest)
    {
        TCQCServerBase* psdrvDel = m_psdrvTest;
        m_psdrvTest = nullptr;

        psdrvDel->StartShutdown();
        if (!psdrvDel->bWaitTillDead(15000))
        {
            TErrBox msgbStop(m_strTitleText, L"Driver did not stop!");
            msgbStop.ShowIt(m_wndMainFrame);
            return kCIDLib::False;
        }

        try
        {
            delete psdrvDel;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TExceptDlg dlgShowErr
            (
                m_wndMainFrame
                , m_strTitleText
                , L"Failed to delete server driver"
                , errToCatch
            );
            return kCIDLib::False;
        }

        catch(...)
        {
            TErrBox msgbErr(m_strTitleText, L"Failed to delete server driver");
            msgbErr.ShowIt(m_wndMainFrame);
            return kCIDLib::False;
        }
    }

    //
    //  And drop the driver's facility if we have it. Otherwise it won't be possible
    //  to build changes.
    //
    if (m_pfacSrvDrv)
    {
        TFacility* pfacDel = m_pfacSrvDrv;
        m_pfacSrvDrv = nullptr;
        try
        {
            delete pfacDel;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TExceptDlg dlgShowErr
            (
                m_wndMainFrame
                , m_strTitleText
                , L"Failed to drop driver facility"
                , errToCatch
            );
            return kCIDLib::False;
        }

        catch(...)
        {
            TErrBox msgbErr(m_strTitleText, L"Failed to drop driver facility");
            msgbErr.ShowIt(m_wndMainFrame);
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TFacCppDrvTest::bWriteField(const   tCIDLib::TCard4 c4FieldListId
                            , const tCIDLib::TCard4 c4FieldId
                            , const TString&        strValue)
{
    tCIDLib::TBoolean bRes = kCIDLib::False;
    if (m_psdrvTest)
    {
        try
        {
            TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTest->pdcmdQWriteFld
            (
                 TString::strEmpty()
                 , c4FieldListId
                 , c4FieldId
                 , strValue
                 , tCQCKit::EDrvCmdWaits::Wait
            );

            TError errFail;
            bRes = TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True);
            if (!bRes)
                throw(errFail);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgShowErr
            (
                m_wndMainFrame
                , m_strTitleText
                , L"An exception occurred during field write"
                , errToCatch
            );

            // Fall through with false result
        }
    }
    return bRes;
}


//
//  If we have a session open, then close it down. This puts us back at initial
//  state.
//
tCIDLib::TVoid TFacCppDrvTest::CloseSession()
{
    // There's a helper to do this
    bCloseCurrent();
}


//
//  In this test harness, we don't open a file, we open a driver manifest file. We
//  will parse this and then we'll use that info to know what else we need to load
//  later when it's time to start the driver running.
//
tCIDLib::TVoid TFacCppDrvTest::DoNewSession()
{
    // If we currently have a session open, then close it down
    if (!bCloseCurrent())
        return;

    // Get them to select a manifest file
    TString strManFile;
    {
        // Set up a file types collection to set
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"Driver Manifest Files", L"*.Manifest"));

        tCIDLib::TStrList colFiles;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            m_wndMainFrame
            , facCppDrvTest.strMsg(kCppDTMsgs::midMsg_SelManifest)
            , TString::strEmpty()
            , colFiles
            , colFileTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::FileMustExist
                , tCIDCtrls::EFOpenOpts::FileSystemOnly
              )
        );

        if (!bRes)
            return;

        strManFile = colFiles[0];
    }

    //
    //  Let's try to parse this manifest file. The config object whose info
    //  it encodes knows how to parse this XML file and fill itself in from
    //  that content.
    //
    TCQCDriverCfg cqcdcManifest;
    try
    {
        cqcdcManifest.ParseFrom(new TFileEntitySrc(strManFile));

        //
        //  It's not done automatically, but we'll check here for any duplicate
        //  prompt keys.
        //
        tCIDLib::TCard4 c4Count = cqcdcManifest.c4PromptCount();
        tCIDLib::TStrHashSet colCheck(109, TStringKeyOps());
        tCIDLib::TBoolean bAdded;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCDCfgPrompt& cqcdcpCur = cqcdcManifest.cqcdcpAt(c4Index);
            colCheck.objAddIfNew(cqcdcpCur.strName(), bAdded);

            if (!bAdded)
            {
                facCppDrvTest.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCppDTErrs::errcMan_DupPromptKey
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Duplicate
                    , cqcdcpCur.strName()
                );
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgErr
        (
            m_wndMainFrame
            , facCppDrvTest.strMsg(kCppDTMsgs::midStatus_BadManifest)
            , errToCatch
        );
        return;
    }

    catch(...)
    {
        TErrBox msgbErr(facCppDrvTest.strMsg(kCppDTMsgs::midStatus_ExceptInManifest));
        msgbErr.ShowIt(m_wndMainFrame);
        return;
    }

    // Make sure it's a C++ based driver
    if (cqcdcManifest.eType() != tCQCKit::EDrvTypes::CppLib)
    {
        TErrBox msgbError(facCppDrvTest.strMsg(kCppDTMsgs::midMsg_NotACppDriver));
        msgbError.ShowIt(m_wndMainFrame);
        return;
    }

    //
    //  Let's now run the driver install wizard to get all of the options that this
    //  driver needs.
    //
    TString strMoniker;
    TCQCDriverObjCfg cqcdcSession(cqcdcManifest);
    {
        //
        //  First, refresh any port factories that might have configurable options,
        //  so that we will present the latest and greatest for com port selection.
        //
        facGC100Ser().bUpdateFactory(m_cuctxToUse.sectUser());
        facJAPwrSer().bUpdateFactory(m_cuctxToUse.sectUser());

        // And invoke the wizard
        tCIDLib::TStrList colMons;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bDriverWizard
        (
            m_wndMainFrame
            , TSysInfo::strIPHostName()
            , cqcdcSession
            , kCIDLib::False
            , colMons
            , nullptr
            , m_cuctxToUse.sectUser()
        );

        // If they bailed out, we are done
        if (!bRes)
            return;
    }

    //
    //  We have to be careful that we don't override a driver that is currently loaded
    //  into CQCServer with this same moniker. That would end up pointing everyone
    //  at us, when we only want our client driver to point to us. So we see if the
    //  name server binding exists. If so, we refuse to load it unless they tell us to
    //  override.
    //
    {
        TString strPath(TCQCSrvAdminServerBase::strDrvScope, 32UL);
        strPath.Append(kCIDLib::pszTreeSepChar);
        strPath.Append(cqcdcSession.strMoniker());

        if (facCIDOrbUC().bNSBindingExists(strPath))
        {
            TYesNoBox msgbOver
            (
                facCppDrvTest.strMsg
                (
                    kCppDTMsgs::midQ_DrvAlreadyBound, cqcdcSession.strMoniker()
                )
            );

            if (!msgbOver.bShowIt(m_wndMainFrame))
                return;
        }
    }

    //
    //  Looks like we are good, so store away the original manifest info, the
    //  manifest plus configuration, and the manifest file path. This means we are
    //  now in open session mode.
    //
    m_cqcdcOrg = cqcdcManifest;
    m_cqcdcSession = cqcdcSession;
    m_strManifestFile = strManFile;

    //
    //  Update the main frame's title bar. We display the manifest file in brackets.
    //  We need to elipsis it if it's long, which it sometimes is.
    //
    TString strTitleFile = m_strManifestFile;
    tCIDLib::TCard4 c4Len = strTitleFile.c4Length();
    if (c4Len > 92)
    {
        const tCIDLib::TCard4 c4At((c4Len / 2) - (c4Len / 4));
        strTitleFile.Cut(c4At, c4Len / 2);
        strTitleFile.Insert(L".....", c4At);
    }

    TString strTitle
    (
        facCppDrvTest.strMsg(kCppDTMsgs::midGen_Title), strTitleFile.c4Length() + 8
    );
    strTitle.Append(L" [");
    strTitle.Append(strTitleFile);
    strTitle.Append(L"]");
    m_wndMainFrame.strWndText(strTitle);
}


//
//  The main window calls this when the user asks to do V2 validation. He only allows
//  this if the driver is running. We check that it's connected, and then get the
//  current fields, and validate them.
//
tCIDLib::TVoid TFacCppDrvTest::DoV2Validation()
{
    // If it is not allocated or isn't at least past wait init, it can't work
    if (!m_psdrvTest || (m_psdrvTest->eState() <= tCQCKit::EDrvStates::WaitInit))
    {
        TErrBox msgbErr
        (
            m_strTitleText, L"The driver is not far enough along to validate"
        );
        msgbErr.ShowIt(m_wndMainFrame);
        return;
    }

    try
    {
        // Get the list of fields
        tCQCKit::TFldDefList colFlds;
        tCIDLib::TCard4 c4FldListId;
        const tCIDLib::TCard4 c4Count = m_psdrvTest->c4QueryFields(colFlds, c4FldListId);
        if (!c4Count)
        {
            TErrBox msgbErr(m_strTitleText, L"No fields are available to validate");
            msgbErr.ShowIt(m_wndMainFrame);
            return;
        }

        // Query the driver config
        TCQCDriverObjCfg cqcdcDrv;
        m_psdrvTest->QueryDrvConfigObj(cqcdcDrv);

        tCIDLib::TStrList   colErrors;
        TCQCV2Validator     cv2vValidator;
        if (cv2vValidator.bDoV2Validation(cqcdcDrv, colFlds, colErrors))
        {
            TOkBox msgbOK(m_strTitleText, L"The driver passed V2 validation");
            msgbOK.ShowIt(m_wndMainFrame);
        }
         else
        {
            facCIDWUtils().ShowList
            (
                m_wndMainFrame, m_strTitleText, colErrors
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgShowErr
        (
            m_wndMainFrame
            , L"An error occurred while trying to V2 validate the driver"
            , errToCatch
        );
    }
}



// The program entry point
tCIDLib::EExitCodes TFacCppDrvTest::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
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
        TErrBox msgbTest(m_strTitleText, strMsg(kCppDTMsgs::midStatus_BadEnv, strFailReason));
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
        TResourceName rsnInstance(L"CQSL", L"CppDrvTest", L"SingleInstanceInfo");
        if (TProcess::bSetSingleInstanceInfo(rsnInstance, kCIDLib::True))
            return tCIDLib::EExitCodes::Normal;

        // Get the build result directory for later use
        if (!TProcEnvironment::bFind(L"CID_RESDIR", m_strResDir))
        {
            TErrBox msgbResDir(m_strTitleText, L"Could not find result directory");
            msgbResDir.ShowIt(TWindow::wndDesktop());
            return tCIDLib::EExitCodes::RuntimeError;
       }

        //
        //  Now we can initialize the client side ORB, which we have to do because we
        //  have to download driver DLLs.
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
            kCppDrvTest::strStoreKey, m_cuctxToUse.strUserName(), bNoState
        );

        //
        //  Enable the sending out of events, so that any user action events the
        //  driver might send out can be tested. And we enable receiving so that our
        //  event trigger monitor tab can watch for those.
        //
        //  Have to do this before the man window is created since he creates a trigger
        //  monitor tab that will start trying to read incoming events.
        //
        facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Both, m_cuctxToUse.sectUser());

        // Register any serial port factories
        facGC100Ser().bUpdateFactory(m_cuctxToUse.sectUser());
        facJAPwrSer().bUpdateFactory(m_cuctxToUse.sectUser());

        //
        //  Initialize the server side of the ORB now that it looks like we are going
        //  make it and we need now to register some objects.
        //
        facCIDOrb().InitServer(m_ippnListen, 1);

        //
        //  Create one of our faux CQCServer admin objects and register it. This is how
        //  we link our client driver to our driver in place. We don't need to add it
        //  to the rebinder. Instead we'll set the OOID of this object on the server
        //  side driver and he will register it as his interface. CQCServer does the
        //  same, so this lets us easily pass for CQCServer with almsot no extra
        //  effort.
        //
        m_porbsDrvAdmin = new TCQCSrvDrvTI;
        facCIDOrb().RegisterObject(m_porbsDrvAdmin, tCIDLib::EAdoptOpts::Adopt);

        //
        //  Start the rebinder. We don't use it ourself, but other underlying subsystems
        //  will so we have to start it up.
        //
        facCIDOrbUC().StartRebinder();

        //
        //  Load any previous config we might have had (assuming it wasn't tossed via
        //  the 'no state' command line parm above.
        //
        TCQCFrameState fstMain;
        facCQCGKit().bReadFrameState
        (
            kCppDrvTest::strCfgKey_Frame, fstMain, TSize(640, 480)
        );

        // Create our main frame window and he takes it up from there
        m_wndMainFrame.CreateFrame(fstMain);
        facCIDCtrls().ShowGUI(m_wndMainFrame);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        //
        //  Use the standard exception display dialog to show it. We have to load and
        //  pass the title text and use the desktop as the parent since our widnow
        //  presumably was not created.
        //
        TExceptDlg dlgShowErr
        (
            TWindow::wndDesktop()
            , m_strTitleText
            , strMsg(kCppDTErrs::errcGen_UnhandledExcept)
            , errToCatch
        );
        return tCIDLib::EExitCodes::InitFailed;
    }

    //
    //  Do the message loop. We use the standard error handler enabled
    //  frame window for our main frame, so the exception handler in the
    //  main loop will pass them to him and he'll handle them. Any unhandled
    //  exception will cause the loop to exit after logging it and telling
    //  the use what happened. No exceptions will propogate out of here
    //
    facCIDCtrls().uMainMsgLoop(m_wndMainFrame);

    try
    {
        // Stop the ORB's rebinder first
        facCIDOrbUC().StartRebinder();

        // Make sure the driver is stopped
        bStopDriver();

        // And now we can destroy our main window
        m_wndMainFrame.Destroy();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        //
        //  Use the standard exception display dialog to show it. We have to load and
        //  pass the title text here since we assume our frame window is dead in some
        //  way at least.
        //
        TExceptDlg dlgShowErr
        (
            TWindow::wndDesktop()
            , m_strTitleText
            , strMsg(kCppDTErrs::errcGen_UnhandledExcept)
            , errToCatch
        );
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

    //
    //  And finally make the logger stop logging. We told TModule to adopt it, so
    //  he will clean it up.
    //
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


//
//  Provides access to our driver, mostly for the main window and his tabs. If no
//  session is open, then it will be null.
//
TCQCServerBase* TFacCppDrvTest::psdrvTest()
{
    return m_psdrvTest;
}


//
//  This is called when the user selects a new verbosity level from the main menu
//  bar. We store it for use as the default upon the next startup. If the driver is
//  currently running, we pass it to him as well as the new live verbosity.
//
tCIDLib::TVoid TFacCppDrvTest::SetNewVerbosity(const tCQCKit::EVerboseLvls eNew)
{
    m_eLastVerbosity = eNew;

    //
    //  If the driver is going, pass it on to him. No need for sync here. He just
    //  stores it and the driver thread will see it later and take it as the new
    //  level.
    //
    if (m_psdrvTest)
        m_psdrvTest->SetNewVerbosity(m_eLastVerbosity);
}


// ---------------------------------------------------------------------------
//  TFacCppDrvTest: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This method runs the standard CQC logon dialog. If it succeeds, we get
//  back the security token for the user, which we set on CQCKit for subsequent
//  use by the app.
//
tCIDLib::TBoolean TFacCppDrvTest::bDoLogon()
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


tCIDLib::TBoolean TFacCppDrvTest::bParseParms(tCIDLib::TBoolean& bIgnoreState)
{
    //
    //  Most of our parms are handled by CQCKit, but we support a /NoState flag, to
    //  ignore the saved state and start with default window positions. Assume not
    //  to ignore unless told to.
    //
    bIgnoreState = kCIDLib::False;

    TString strCurParm;
    TSysInfo::TCmdLineCursor cursParms = TSysInfo::cursCmdLineParms();
    for (; cursParms; ++cursParms)
    {
        strCurParm = *cursParms;

        if (strCurParm.bCompareI(L"/NoState"))
        {
            bIgnoreState = kCIDLib::True;
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
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
                LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , L"Bad ORB listening port on command line"
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::BadParms
                );
            }
        }
    }
    return kCIDLib::True;
}



//
//  If we have a current session, ask the user if it is ok to stop it and shut it
//  down. If so, we do so and return true, else false.
//
tCIDLib::TBoolean TFacCppDrvTest::bCloseCurrent()
{
    // If there is a driver loaded, drop it
    if (!bStopDriver())
        return kCIDLib::False;

    // Clear the manifest file path, which is our indicator of an open session
    m_strManifestFile.Clear();

    // Update the title text to get rid of any displayed manifest file
    m_wndMainFrame.strWndText(m_strTitleText);

    return kCIDLib::True;
}


