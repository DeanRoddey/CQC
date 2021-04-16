//
// FILE NAME: CQCInst_InstallThread.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/02/2004
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
//  This file implements the actual install bits of the facility class. This stuff all
//  runs within it's own thread class. The main window gives us a pointer to itself and a
//  copy of the installation info, and we just post him back status messages. We know
//  he won't change the install info while we are running, so we can modify it directly
//  without any sync.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInst.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TInstallThread,TThread)



// ---------------------------------------------------------------------------
//   CLASS: TInstallThread
//  PREFIX: thr
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstallThread: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstallThread::TInstallThread() :

    TThread(L"CQCInstInstallerThread")
    , m_eCurStep(tCQCInst::EInstSteps::Start)
    , m_pwndOwner(nullptr)
{
}

TInstallThread::~TInstallThread()
{
}


// ---------------------------------------------------------------------------
//  TInstallThread: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCInst::EInstSteps TInstallThread::eStep() const
{
    return m_eCurStep;
}


tCIDLib::TVoid
TInstallThread::StartInstall(       TMainFrameWnd* const    pwndOwner
                            ,       TInstStatusPanel* const ppanStatus
                            , const TCQCInstallInfo* const  pinfoToUse)
{
    // Store the passed values and then start the thread
    m_pinfoToUse = pinfoToUse;
    m_ppanStatus = ppanStatus;
    m_pwndOwner = pwndOwner;
    Start();
}


// ---------------------------------------------------------------------------
//  TInstallThread: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  This is the thread that does the actual installation. It posts events
//  to the main window to let the user know what is going on.
//
tCIDLib::EExitCodes TInstallThread::eProcess()
{
    TPathStr pathTmp;
    TPathStr pathTmp2;

    // Let our caller go
    Sync();

    // Save a lot of typing below by getting a ref to the new install info
    const TCQCVerInstallInfo& viiNew = m_pinfoToUse->viiNewInfo();

    tCIDKernel::TWSvcHandle hsvcCQC = nullptr;
    tCIDLib::TStrList       colAppShellParms;
    tCIDLib::TStrList       colClSrvParms;
    try
    {
        //
        //  Try to clean up any previously existing temp directory from a
        //  revious install. We always want this to be fresh.
        //
        SetStep(tCQCInst::EInstSteps::CreateTmpDir);
        if (!bRemoveInstPath(m_pinfoToUse->strTmpPath()))
            return tCIDLib::EExitCodes::InitFailed;

        //
        //  Now try to create the path to the temp directory again. This will
        //  create the base install path as well, if it doesn't already exist.
        //
        try
        {
            TFileSys::MakePath(m_pinfoToUse->strTmpPath());
        }

        catch(const TError& errToCatch)
        {
            ShowError(errToCatch);
            SetStep(tCQCInst::EInstSteps::Complete);
            return tCIDLib::EExitCodes::InitFailed;
        }

        //
        //  The very first thing we'll do is to try to rename the Bin and
        //  CQCData directories, and then rename them back. We do this to
        //  find out if any apps have the directories locked, since that is
        //  going to make the install fail. If we can't rename one, then
        //  we give the user a chance to fix the issue before we go on.
        //
        if (!bTestDirLock(L"Bin") || !bTestDirLock(L"CQCData"))
        {
            Recover();
            SetStep(tCQCInst::EInstSteps::Complete);
            return tCIDLib::EExitCodes::InitFailed;
        }

        //
        //  Get rid of any old SaveInst directory to get ready for the new
        //  installation. We call a helper method that removes directories
        //  and handles warning the user if it can't be done and allows them
        //  to try to fix it and retry. If they abort, it'll throw.
        //
        pathTmp = m_pinfoToUse->strPath();
        pathTmp.AddLevel(L"SaveInst");
        if (!bRemoveInstPath(pathTmp))
        {
            Recover();
            SetStep(tCQCInst::EInstSteps::Complete);
            return tCIDLib::EExitCodes::InitFailed;
        }

        //
        //  Set up the client side data directory, which is in the common
        //  app data area, not under the CQC directory. Since it's there
        //  we have to use the system id in case more than one CQC
        //  installation is present.
        //
        //  In this case, if we create these directories, we tell the file
        //  system name space to apply all access rights to them, so that
        //  they can be accessed by non-admin programs.
        //
        try
        {
            pathTmp = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonAppData);
            pathTmp.AddLevel(L"CQCData");
            if (!TFileSys::bIsDirectory(pathTmp))
                TFileSys::MakeDirectory(pathTmp, kCIDLib::False, kCIDLib::True);
            m_strClientDir = pathTmp;
        }

        catch(TError& errToCatch)
        {
            facCQCInst.LogInstallMsg(L"Failed to create client data dir");
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            throw;
        }

        catch(...)
        {
            facCQCInst.LogInstallMsg(L"Failed to create client data dir");
            throw;
        }

        //
        //  Create the output directories under the target install directory.
        //  NOTE that we are building into the temp directory for the install,
        //  so we need to use the temp path as the base path for these.
        //
        try
        {
            MakeDirectories();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogInstallMsg(L"Failed to create output directories");
            throw;
        }

        catch(...)
        {
            facCQCInst.LogInstallMsg(L"Failed to create output directories");
            throw;
        }

        //
        //  We always have to copy the core CIDLib support files, regardless of
        //  what CQC components are being installed, and the Microsoft runtime
        //  files.
        //
        //  Install the CQC support files, and utilities if we aren't doing
        //  a remote viewer install.
        //
        SetStep(tCQCInst::EInstSteps::CopyBase);
        DupRelPath(L"CIDLib", L"Bin", kCIDLib::True);
        DupRelPath(L"CQC", L"Bin", kCIDLib::True);
        CopyLangRT();

        // Install the utilities
        CopyImageDir(L"Utils", L"Bin");

        // Copy over all of the Local data stuff
        DupRelPath(L"Local", L"CQCData\\Local", kCIDLib::True);

        //
        //  If we are installing any clients, then copy those files if not
        //  doing a remote viewer install.
        //
        if (viiNew.bAnyClient())
        {
            SetStep(tCQCInst::EInstSteps::CopyClient);
            CopyImageDir(L"Client", L"Bin");
        }

        //
        //  If we are installing CQCServer, then copy server files and the third party
        //  support files, which are used by drivers that CQCServer loads.
        //
        if (viiNew.bCQCServer())
        {
            SetStep(tCQCInst::EInstSteps::CopyCQCServer);
            CopyImageDir(L"Server", L"Bin");
        }

        //
        //  If we are installing the master server, then copy those files.
        //
        //  NOTE:   We'll move forward any that the user has installed,
        //          during data upgrade phase. Here we only copy stuff
        //          that is re-installed during each upgrade.
        //
        if (viiNew.bMasterServer())
        {
            SetStep(tCQCInst::EInstSteps::CopyMaster);

            // Copy flat single directories over
            CopyImageDir(L"Master", L"Bin");
            CopyImageDir(L"Manifests", L"CQCData\\DataServer\\Manifests");
            CopyImageDir(L"Protocols", L"CQCData\\DataServer\\Protocols");
            CopyImageDir(L"ZWU3Info", L"CQCData\\DataServer\\ZWU3Info");

            // And the trees
            DupRelPath(L"Macros", L"CQCData\\DataServer\\Macros", kCIDLib::True);
            DupRelPath(L"Images", L"CQCData\\DataServer\\Images\\System", kCIDLib::True);
            DupRelPath(L"Interfaces", L"CQCData\\DataServer\\Interfaces\\System", kCIDLib::True);

            // Copy over the CQC Voice support files
            DupRelPath(L"CQCVoice", L"CQCData\\CQCVoice", kCIDLib::False);
        }

        // If we are installing the Gateway server, then copy it over
        if (viiNew.bXMLGWServer())
        {
            SetStep(tCQCInst::EInstSteps::CopyGWServer);
            CopyImageDir(L"GWServer", L"Bin");
        }

        // If we are installing the Web server, then copy it over
        if (viiNew.bWebServer())
        {
            SetStep(tCQCInst::EInstSteps::CopyWebServer);
            CopyImageDir(L"WebServer", L"Bin");
        }

        // Now generate any configuration files that we need
        GenConfig();

        // Generate some helper command files
        GenCmdFiles();

        // If any clients, we have to copy over icons and app images.
        if (viiNew.bAnyClient())
        {
            CopyImageDir(L"AppIcons", L"Bin\\AppIcons");
            CopyImageDir(L"AppImages", L"Bin\\AppImages");
        }

        //
        //  If the old downloaded drivers directories exist, remove them. We don't
        //  do that anymore, they are just in Bin with the other stuff.
        //
        if (m_pinfoToUse->eTargetType() != tCQCInst::ETargetTypes::Clean)
        {
            pathTmp = m_pinfoToUse->strTmpPath();
            pathTmp.AddLevels(L"CQCData", L"Server", L"Drivers");
            if (TFileSys::bIsDirectory(pathTmp))
                TFileSys::CleanPath(pathTmp, tCIDLib::ETreeDelModes::Clean);

            pathTmp = m_strClientDir;
            pathTmp.AddLevels(L"Client", L"Drivers");
            if (TFileSys::bIsDirectory(pathTmp))
                TFileSys::CleanPath(pathTmp, tCIDLib::ETreeDelModes::Clean);
        }

        //
        //  And if we have a previous version, we can now do the second phase of the
        //  upgrade, and copy forward data.
        //
        //  NOTE:   We have to do this before starting the service, because it will create
        //          new files and they will be open and we won't be able to access them.
        //
        if ((m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Refresh)
        ||  (m_pinfoToUse->eTargetType() ==tCQCInst::ETargetTypes::Revision)
        ||  (m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Upgrade))
        {
            UpgradeData();
        }
        else if (m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Clean)
        {
            //
            //  we still need to do this  even if it's a clean install, since this
            //  will set up some required security info if it is not present (which is
            //  the case if upgrading from Pre-OSS or on a clean install.
            //
            UpgradeSecSrvInfo();
        }


        //
        //  Now do the install of the service, which will create a new one if needed,
        //  or update the old one one. We pass in the handle we created above, and
        //  if it's invalid, he'll know he needs to create a new one. If it already
        //  existed, we'll just get the same handle back.
        //
        if (m_pinfoToUse->bNeedsAppShell())
        {
            hsvcCQC = hsvcInstallAppShell(colAppShellParms);
        }
            else
        {
            hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszCQCShellName);
            if (hsvcCQC)
            {
                TKrnlWin32Service::bRemove(hsvcCQC);
                TThread::Sleep(2500);
                TKrnlWin32Service::bClose(hsvcCQC);
                hsvcCQC = 0;
            }
        }

        // If this is the master server, then copy over our help content
        if (viiNew.bMasterServer())
            DupRelPath(L"Help", L"CQCData\\HTMLRoot\\Web2\\CQCDocs", kCIDLib::True);

        // If installing the web server, copy over the Web RIVA stuff
        if (viiNew.bWebServer())
        {
             DupRelPath(L"HTML\\WebRIVA", L"CQCData\\HTMLRoot\\CQSL\\WebRIVA", kCIDLib::True);
             DupRelPath(L"HTML\\WebRIVAExt", L"CQCData\\HTMLRoot\\WebRIVAExt\\Samples", kCIDLib::True);

            //
            //  If there are no existing extension files, copy over the default ones
            //
            //  HOWEVER, if the previous version is before 5.3.937, we save the previous
            //  web cam file and copy a new one over.
            //
            TPathStr pathSrc;
            TPathStr pathTar(m_pinfoToUse->strTmpPath());
            pathTar.AddLevel(L"\\CQCData\\HTMLRoot\\WebRIVAExt\\WebRIVAInsert.js");
            if (!TFileSys::bExists(pathTar))
            {
                pathSrc = m_pinfoToUse->strSrcImage();
                pathSrc.AddLevel(L"HTML\\WebRIVAExt\\Default\\WebRIVAInsert.js");
                TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
            }

            pathTar = m_pinfoToUse->strTmpPath();
            pathTar.AddLevel(L"\\CQCData\\HTMLRoot\\WebRIVAExt\\WebCamSetup.js");
            if (!TFileSys::bExists(pathTar))
            {
                pathSrc = m_pinfoToUse->strSrcImage();
                pathSrc.AddLevel(L"HTML\\WebRIVAExt\\Default\\WebCamSetup.js");
                TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
            }

            pathTar = m_pinfoToUse->strTmpPath();
            pathTar.AddLevel(L"\\CQCData\\HTMLRoot\\WebRIVAExt\\WebRIVACmd.js");
            if (!TFileSys::bExists(pathTar))
            {
                pathSrc = m_pinfoToUse->strSrcImage();
                pathSrc.AddLevel(L"HTML\\WebRIVAExt\\Default\\WebRIVACmd.js");
                TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
            }
        }

        //
        //  We always install/update the client service (but not in debug mode because
        //  it's not built to run as a service in that mode.)
        //
        #if CID_DEBUG_OFF
        InstallClService(colClSrvParms);
        #endif

        //
        //  Sleep a little bit to give any anti-virus time to get out of our newly created
        //  directories before we try to do a rename.
        //
        TThread::Sleep(4000);

        //
        //  If we find either directory of a previous install, then we need
        //  to move them into the save directory.
        //
        SetStep(tCQCInst::EInstSteps::StartSwapDirs);
        pathTmp = m_pinfoToUse->strPath();
        if (TFileSys::bExists(pathTmp, L"Bin", tCIDLib::EDirSearchFlags::NormalDirs)
        ||  TFileSys::bExists(pathTmp, L"CQCData", tCIDLib::EDirSearchFlags::NormalDirs))
        {
            try
            {
                TFileSys::MakeSubDirectory(L"SaveInst", m_pinfoToUse->strPath());
                pathTmp = m_pinfoToUse->strPath();
                if (TFileSys::bExists(pathTmp, L"Bin"))
                {
                    pathTmp.AddLevels(L"SaveInst", L"Bin");
                    pathTmp2 = m_pinfoToUse->strPath();
                    pathTmp2.AddLevel(L"Bin");
                    TFileSys::Rename(pathTmp2, pathTmp);
                    SetStep(tCQCInst::EInstSteps::SaveBin);
                }

                pathTmp = m_pinfoToUse->strPath();
                if (TFileSys::bExists(pathTmp, L"CQCData"))
                {
                    pathTmp.AddLevels(L"SaveInst", L"CQCData");
                    pathTmp2 = m_pinfoToUse->strPath();
                    pathTmp2.AddLevel(L"CQCData");
                    TFileSys::Rename(pathTmp2, pathTmp);
                    SetStep(tCQCInst::EInstSteps::SaveData);
                }

                // Also copy the version info and release notes files into there
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevel(L"CQCInstVersion.Info");
                if (TFileSys::bExists(pathTmp))
                {
                    pathTmp2 = m_pinfoToUse->strPath();
                    pathTmp2.AddLevels(L"SaveInst", L"CQCInstVersion.Info");
                    TFileSys::CopyFile(pathTmp, pathTmp2);
                    SetStep(tCQCInst::EInstSteps::SaveVer);
                }

                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevel(L"ReleaseNotes.htm");
                if (TFileSys::bExists(pathTmp))
                {
                    pathTmp2 = m_pinfoToUse->strPath();
                    pathTmp2.AddLevels(L"SaveInst", L"ReleaseNotes.htm");
                    TFileSys::CopyFile(pathTmp, pathTmp2);
                    SetStep(tCQCInst::EInstSteps::SaveNotes);
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);

                facCQCInst.LogInstallMsg(L"Could not move current dirs to save dir");
                throw;
            }
        }

        //
        //  And now move the new temp dirs to their final position as the
        //  new installation.
        //
        try
        {
            pathTmp = m_pinfoToUse->strPath();
            pathTmp.AddLevel(L"Bin");
            pathTmp2 = m_pinfoToUse->strTmpPath();
            pathTmp2.AddLevel(L"Bin");
            TFileSys::Rename(pathTmp2, pathTmp);
            SetStep(tCQCInst::EInstSteps::NewBin);

            pathTmp = m_pinfoToUse->strPath();
            pathTmp.AddLevel(L"CQCData");
            pathTmp2 = m_pinfoToUse->strTmpPath();
            pathTmp2.AddLevel(L"CQCData");
            TFileSys::Rename(pathTmp2, pathTmp);
            SetStep(tCQCInst::EInstSteps::NewData);

            // And remove the temp directory
            TFileSys::RemovePath(m_pinfoToUse->strTmpPath());
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);

            facCQCInst.LogInstallMsg(L"Could not move new directories to final positions");
            throw;
        }

        //
        //  Generate shell shortcuts as appropriate, now that the files are
        //  in their final destinations.
        //
        GenLinks();

        //
        //  We've installed it. Now write out a new install version info
        //  file.
        //
        pathTmp = m_pinfoToUse->strPath();
        pathTmp.AddLevel(kCQCInst::pszInstVerFile);

        // Open a stream on this file and stream out the new info
        {
            TBinFileOutStream strmVer
            (
                pathTmp
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmVer << viiNew;
            strmVer.Flush();
        }

        //
        //  Update or create the the registry entry that holds our install
        //  directory.
        //
        tCIDLib::TBoolean bCreated;
        tCIDKernel::TWRegHandle hkeyCQS = TKrnlWin32Registry::hkeyCreateOrOpenSubKey
        (
            tCIDKernel::ERootKeys::LocalMachine
            , L"SOFTWARE"
            , kCQCKit::pszReg_Key
            , bCreated
            , tCIDKernel::ERegAccFlags::StdOwned
        );

        if (hkeyCQS)
        {
            // Store the target installation path
            tCIDLib::TBoolean bRes = TKrnlWin32Registry::bSetStrValue
            (
                hkeyCQS, kCQCKit::pszReg_Path, m_pinfoToUse->strPath().pszBuffer()
            );

            if (!bRes)
                facCQCInst.LogInstallMsg(L"Could not update registry path");

            // Store the version info
            TString strVer;
            strVer.AppendFormatted(m_pinfoToUse->viiNewInfo().c4MajVersion());
            strVer.Append(L'.');
            strVer.AppendFormatted(m_pinfoToUse->viiNewInfo().c4MinVersion());
            strVer.Append(L'.');
            strVer.AppendFormatted(m_pinfoToUse->viiNewInfo().c4Revision());
            bRes = TKrnlWin32Registry::bSetStrValue
            (
                hkeyCQS, kCQCKit::pszReg_Version, strVer.pszBuffer()
            );

            if (!bRes)
                facCQCInst.LogInstallMsg(L"Could not update registry version");

            if (!TKrnlWin32Registry::bCloseKey(hkeyCQS))
            {
                facCQCInst.LogKrnlErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCInstMsgs::midStatus_CloseRegKey
                    , TKrnlError::kerrLast()
                    , tCIDLib::ESeverities::Warn
                    , tCIDLib::EErrClasses::CantDo
                );
                facCQCInst.LogInstallMsg(L"Could not close registry handle");
            }
        }
            else
        {
            facCQCInst.LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstMsgs::midStatus_UpdateRegistry
                , TKrnlError::kerrLast()
                , tCIDLib::ESeverities::Warn
                , tCIDLib::EErrClasses::CantDo
            );
        }

        //
        //  If we are on XP SP2 or greater, then we need to add exceptions for any
        //  server processes being installed or the firewall will block them.
        //
        //  Note that this is Windows specific and we use a CIDKernel level API for
        //  this.
        //
        tCIDLib::TCard4 c4OSMaj, c4OSMin, c4OSRev, c4OSBuild;
        TSysInfo::QueryOSInfo(c4OSMaj, c4OSMin, c4OSRev, c4OSBuild);
        c4OSMaj <<= 16;
        c4OSMaj |= (c4OSMin << 8);
        c4OSMaj |= c4OSRev;
        if (c4OSMaj >= 0x50102)
        {
            tCIDLib::TZStr1K pszErrText;
            TPathStr pathTmp;

            // If the app shell service is being installed, do the serivce and the cfg srv
            if (m_pinfoToUse->bNeedsAppShell())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCAppShell.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Application Shell"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }

                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CIDCfgSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Config Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // The client service is always done
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCClService.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Client Service"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            //
            //  If doing the master server, then we have to do the name,
            //  log, and data servers.
            //
            if (viiNew.bMasterServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CIDNameSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Name Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }

                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CIDLogSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Log Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }

                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCDataSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Data Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing CQCServer
            if (viiNew.bCQCServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCServer.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing Logic Server
            if (viiNew.bLogicServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQLogicSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"Logic Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the interface viewer
            if (viiNew.bIntfViewer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCIntfView.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Interface Viewer"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }

                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCWBHelper.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Web Browser Helper"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the tray monitor
            if (viiNew.bTrayMon())
            {
                // Keep the link name in sync with the uninstaller
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCTrayMon.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Tray Monitor"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the CQCVoice
            if (viiNew.bCQCVoice())
            {
                // Keep the link name in sync with the uninstaller
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCVoice.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Voice"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the events server
            if (viiNew.bEventServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCEventSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Event Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the XML Gateway
            if (viiNew.bXMLGWServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCGWSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC XML Gateway"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // If doing the Web server
            if (viiNew.bWebServer())
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCWebSrv.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Web Server"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }

            // We always do the client service
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevels(L"Bin", L"CQCClService.exe");
                if (!TKrnlIP::bAddToFirewall(pathTmp.pszBuffer()
                                            , L"CQC Client Service"
                                            , pszErrText
                                            , 1023))
                {
                    facCQCInst.LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kCQCInstErrs::errcFail_AddToFirewall
                        , pszErrText
                        , tCIDLib::ESeverities::Warn
                        , tCIDLib::EErrClasses::CantDo
                        , pathTmp
                    );
                }
            }
        }

        // Copy over the release notes
        facCQCInst.LogInstallMsg(L"Copying release notes");
        pathTmp = m_pinfoToUse->strSrc();
        pathTmp.AddLevel(L"ReleaseNotes.htm");
        pathTmp2 = m_pinfoToUse->strPath();
        pathTmp2.AddLevel(L"ReleaseNotes.htm");
        try
        {
            if (TFileSys::bExists(pathTmp2))
            {
                //
                //  Turn off any read only attribute on the target first,
                //  since we copy a read only file to the target, so any
                //  previous install would have left a read only file there.
                //
                TFileSys::SetPerms(pathTmp2, tCIDLib::EFilePerms::AllWE);
                TFileSys::DeleteFile(pathTmp2);
            }
            if (TFileSys::bExists(pathTmp))
                TFileSys::CopyFile(pathTmp, pathTmp2);
            else
                facCQCInst.LogInstallMsg(L"No release notes found to copy");
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            facCQCInst.LogInstallMsg(L"Error copying release notes");
        }
    }

    catch(TError& errToCatch)
    {
        // Close the service handle if we opened it
        if (hsvcCQC)
            TKrnlWin32Service::bClose(hsvcCQC);

        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogEventObj(errToCatch);
        }
        facCQCInst.LogInstallMsg(L"CIDLib exception in install thread");

        // Do recovery operations as required
        Recover();

        SetStep(tCQCInst::EInstSteps::Complete);
        return tCIDLib::EExitCodes::InitFailed;
    }

    catch(...)
    {
        // Close the service handle if we opened it
        if (hsvcCQC)
            TKrnlWin32Service::bClose(hsvcCQC);

        facCQCInst.LogInstallMsg(L"Unknown exception in install thread");

        // Do recovery operations as required
        Recover();

        SetStep(tCQCInst::EInstSteps::Complete);
        return tCIDLib::EExitCodes::InitFailed;
    }

    //
    //  Set some stuff in the registry for the non-MS (data server) servers to
    //  use so that they can log in. We run this on the MS as well, since other
    //  background servers run there also in most cases.
    //
    SetNonMSRegVars();

    //
    //  Now try to start up the app shell if we installed it, and then close
    //  the handle.
    //
    tCIDLib::EExitCodes eRet = tCIDLib::EExitCodes::Normal;
    if (hsvcCQC)
    {
        SetStep(tCQCInst::EInstSteps::StartAppShell);
        facCQCInst.LogInstallMsg(L"Attempting to start CQC App Shell");

        //
        //  Set up the parms, which we have already created, but we have
        //  to pass in raw string pointer to this kernel level call.
        //
        const tCIDLib::TCard4 c4ParmCount = colAppShellParms.c4ElemCount();
        const tCIDLib::TCh** apszParms = new const tCIDLib::TCh*[c4ParmCount];
        TArrayJanitor<const tCIDLib::TCh*> janParms(apszParms);

        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
            apszParms[c4Index] = colAppShellParms[c4Index].pszBuffer();

        //
        //  Start it up with the appropriate parms. This will set the parms
        //  in the service configuration, so it will use them from here on
        //  out when started. We'll wait for up to thirty seconds for it
        //  to start.
        //
        if (TKrnlWin32Service::bStart(hsvcCQC, apszParms, c4ParmCount, 30000))
        {
            facCQCInst.LogInstallMsg(L"App Shell startup is complete");
        }
         else
        {
            facCQCInst.LogInstallMsg(L"Failed to start the App Shell");
            eRet = tCIDLib::EExitCodes::InitFailed;

            TInstQueryPacket qpackErr
            (
                tCQCInst::EQueryOps::MsgBox
                , tCIDCtrls::EMsgBoxTypes::OK
                , facCQCInst.strMsg(kCQCInstErrs::errcFail_StartNewService)
            );
            m_pwndOwner->SendData(kCQCInst::wmsgQueryId, &qpackErr, tCIDLib::ESyncTypes::Synchronous);
        }
        TKrnlWin32Service::bClose(hsvcCQC);
    }

    // And start the client service if not in debug mode
    #if CID_DEBUG_OFF
    {
        SetStep(tCQCInst::EInstSteps::StartClService);
        facCQCInst.LogInstallMsg(L"Attempting to start CQC Client Service");

        const tCIDLib::TCard4 c4ParmCount = colClSrvParms.c4ElemCount();
        const tCIDLib::TCh** apszParms = new const tCIDLib::TCh*[c4ParmCount];
        TArrayJanitor<const tCIDLib::TCh*> janParms(apszParms);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
            apszParms[c4Index] = colClSrvParms[c4Index].pszBuffer();

        hsvcCQC = TKrnlWin32Service::hsvcOpen(kCQCKit::pszClServiceName);
        if (hsvcCQC)
        {
            if (!TKrnlWin32Service::bStart(hsvcCQC, apszParms, c4ParmCount, 30000))
            {
                facCQCInst.LogInstallMsg(L"Failed to start the Client Service");
                eRet = tCIDLib::EExitCodes::InitFailed;

                TInstQueryPacket qpackErr
                (
                    tCQCInst::EQueryOps::MsgBox
                    , tCIDCtrls::EMsgBoxTypes::OK
                    , facCQCInst.strMsg(kCQCInstErrs::errcFail_StartNewService)
                );
                m_pwndOwner->SendData
                (
                    kCQCInst::wmsgQueryId, &qpackErr, tCIDLib::ESyncTypes::Synchronous
                );
            }
             else
            {
                facCQCInst.LogInstallMsg(L"Client Service startup is complete");
            }
            TKrnlWin32Service::bClose(hsvcCQC);
        }
         else
        {
            facCQCInst.LogInstallMsg(L"Could not contact the Client Service to start it");
            eRet = tCIDLib::EExitCodes::InitFailed;
        }
    }
    #endif

    // Post the final completion event
    SetStep(tCQCInst::EInstSteps::Complete);

    return tCIDLib::EExitCodes::Normal;
}


// ---------------------------------------------------------------------------
//  TInstallThread: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper method that will try to remove a path and it's contents, and
//  if an error occurs, tells the user and let's him retry or abort. We'll
//  keep doing it till they tell us to abort or it works.
//
tCIDLib::TBoolean TInstallThread::bRemoveInstPath(const TString& strToRemove)
{
    // If it doesn't exist, then nothing to do
    if (!TFileSys::bExists(strToRemove, tCIDLib::EDirSearchFlags::AllDirs))
        return kCIDLib::True;

    TInstQueryPacket qpackRemove
    (
        tCQCInst::EQueryOps::MsgBox
        , tCIDCtrls::EMsgBoxTypes::RetryCancel
        , facCQCInst.strMsg(kCQCInstErrs::errcFail_RemoveDir, strToRemove)
    );

    while (kCIDLib::True)
    {
        try
        {
            TFileSys::RemovePath(strToRemove);
            break;
        }

        catch(const TError& errToCatch)
        {
            // Give them a chance to do something about it, or to abort
            m_pwndOwner->SendData
            (
                kCQCInst::wmsgQueryId
                , &qpackRemove
                , tCIDLib::ESyncTypes::Synchronous
            );

            // If an error or abort, then give up
            if ((qpackRemove.eResult() != tCQCInst::EQueryRes::Success)
            ||  (qpackRemove.eMsgBoxRet() == tCIDCtrls::EMsgBoxRets::Cancel))
            {
                facCQCInst.LogInstallMsg(L"Could not remove directory");
                TModule::LogEventObj(errToCatch);
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  A helper method that will try to remove a path and it's contents, and
//  if an error occurs, tells the user and let's him retry or abort. We'll
//  keep doing it till they tell us to abort or it works.
//
tCIDLib::TBoolean
TInstallThread::bRenameInstPath(const   TString&    strSrc
                                , const TString&    strTar)
{
    TInstQueryPacket qpackRename
    (
        tCQCInst::EQueryOps::MsgBox
        , tCIDCtrls::EMsgBoxTypes::RetryCancel
        , facCQCInst.strMsg(kCQCInstErrs::errcFail_RenameDir, strSrc)
    );

    while (kCIDLib::True)
    {
        try
        {
            TFileSys::Rename(strSrc, strTar);
            break;
        }

        catch(const TError& errToCatch)
        {
            // Give them a chance to do something about it, or to abort
            m_pwndOwner->SendData
            (
                kCQCInst::wmsgQueryId
                , &qpackRename
                , tCIDLib::ESyncTypes::Synchronous
            );

            // If an error or abort, then give up
            if ((qpackRename.eResult() != tCQCInst::EQueryRes::Success)
            ||  (qpackRename.eMsgBoxRet() == tCIDCtrls::EMsgBoxRets::Cancel))
            {
                facCQCInst.LogInstallMsg(L"Could not rename directory");
                TModule::LogEventObj(errToCatch);
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  Called at the start of installation, after the service is stopped, to
//  see if we can move the Bin and CQCData dirs. If we cannot, then the
//  install is not going to work. The caller passes us Bin or CQCData and
//  we test that directory.
//
//  We have to be VERY careful here that we don't accidentally leave one
//  of the directories renamed.
//
tCIDLib::TBoolean TInstallThread::bTestDirLock(const TString& strSubDir)
{
    // Make sure the temp target dir doesn't exist
    TPathStr pathTar = m_pinfoToUse->strPath();
    pathTar.AddLevel(L"TestMove");
    if (TFileSys::bIsDirectory(pathTar))
    {
        facCQCInst.LogInstallMsg(L"The test lock directory already exists");
        return kCIDLib::False;
    }

    //
    //  Build up the source and target directories. If the source does not
    //  exists, then nothing to do.
    //
    TPathStr pathSrc = m_pinfoToUse->strPath();
    pathSrc.AddLevel(strSubDir);
    if (!TFileSys::bExists(pathSrc, tCIDLib::EDirSearchFlags::AllDirs))
        return kCIDLib::True;

    pathTar = m_pinfoToUse->strPath();
    pathTar.AddLevel(L"TestMove");

    while (kCIDLib::True)
    {
        try
        {
            TFileSys::Rename(pathSrc, pathTar);
            TFileSys::Rename(pathTar, pathSrc);

            // It worked, so break out
            break;
        }

        catch(...)
        {
            // It didn't work, so tell the user
            TInstQueryPacket qpackFail
            (
                tCQCInst::EQueryOps::MsgBox
                , tCIDCtrls::EMsgBoxTypes::RetryCancel
                , facCQCInst.strMsg(kCQCInstErrs::errcFail_DirLocked, strSubDir)
            );

            m_pwndOwner->SendData
            (
                kCQCInst::wmsgQueryId
                , &qpackFail
                , tCIDLib::ESyncTypes::Synchronous
            );

            if ((qpackFail.eResult() != tCQCInst::EQueryRes::Success)
            ||  (qpackFail.eMsgBoxRet() == tCIDCtrls::EMsgBoxRets::Cancel))
            {
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  Copies (non-recursively) a directory from the image to the target. Both paths are
//  relative to their respective srce/target top level directories.
//
tCIDLib::TVoid
TInstallThread::CopyImageDir(const TString& strSrcDir, const TString& strDestDir)
{
    // Build up the source path and search wildcard
    TPathStr pathSrc(m_pinfoToUse->strSrcImage());
    pathSrc.AddLevel(strSrcDir);

    // Build up the output path
    TPathStr pathTarget(m_pinfoToUse->strTmpPath());
    pathTarget.AddLevel(strDestDir);

    facCQCInst.DupPath(pathSrc, pathTarget, kCIDLib::False);
}


//
//  Copies the language runtime libraries from the image to the target. This is a
//  special case because, to get the installer to run it needs them also, so they
//  are installed in the main installer directory, not under the Image directory
//  where everything else is.
//
tCIDLib::TVoid TInstallThread::CopyLangRT()
{
    TPathStr pathTar(m_pinfoToUse->strTmpPath());
    pathTar.AddLevel(L"Bin");
    const tCIDLib::TCard4 c4TarLen = pathTar.c4Length();

    // The src path is that of the installer itself
    TPathStr pathSrc(facCQCInst.strPath());
    const tCIDLib::TCard4 c4SrcLen = pathSrc.c4Length();

    // Copy the runtime DLLs
    pathSrc.CapAt(c4SrcLen);
    pathTar.CapAt(c4TarLen);
    pathSrc.AddLevel(kCQCInst::pszLRT_Lib1);
    pathTar.AddLevel(kCQCInst::pszLRT_Lib1);
    TFileSys::CopyFile(pathSrc, pathTar);
}


//
//  Copy (recursively or not) a list of child directories from the image
//  to the target.
//
tCIDLib::TVoid
TInstallThread::DupChildPaths(  const   TString&                strSrc
                                , const TString&                strTarget
                                , const TCollection<TString>&   colChildren
                                , const tCIDLib::TBoolean       bRecurse)
{
    // Iterate the children and invoke facCQCInst.DupPath() for each one
    TColCursor<TString>* pcursChildren = colChildren.pcursNew();
    TJanitor<TColCursor<TString> > janCurs(pcursChildren);

    if (pcursChildren->bReset())
    {
        TPathStr pathSrc;
        TPathStr pathTarget;
        do
        {
            const TString& strCur = pcursChildren->objRCur();

            pathSrc = strSrc;
            pathSrc.AddLevel(strCur);
            pathTarget = strTarget;
            pathTarget.AddLevel(strCur);
            facCQCInst.DupPath(pathSrc, pathTarget, bRecurse);
        }   while (pcursChildren->bNext());
    }
}


//
//  Duplicates the contents of a directory, where both paths are relative
//  to the src and target top level paths.
//
tCIDLib::TVoid
TInstallThread::DupRelPath( const   TString&            strSrc
                            , const TString&            strTarget
                            , const tCIDLib::TBoolean   bRecurse)
{
    //
    //  The source is relative to the source image directory, and the
    //  target is relative to the temp install path. We build up the full
    //  path, and then call the other version.
    //
    TPathStr pathSrc(m_pinfoToUse->strSrcImage());
    pathSrc.AddLevel(strSrc);
    TPathStr pathTar(m_pinfoToUse->strTmpPath());
    pathTar.AddLevel(strTarget);
    facCQCInst.DupPath(pathSrc, pathTar, bRecurse);
}


//
//  Generates the command file that sets up the CQC Command Prompt that
//  we put into the Start Menu.
//
tCIDLib::TVoid TInstallThread::GenCmdFiles()
{
    SetStep(tCQCInst::EInstSteps::GenCmdFiles);

    //
    //  Generate a simple command file that will set up some environment
    //  for using the command utilities.
    //
    TPathStr pathTarget;
    {
        pathTarget = m_pinfoToUse->strTmpPath();
        pathTarget.Append(L"\\Bin\\CQCEnv.Cmd");
        TTextFileOutStream strmTarget
        (
            pathTarget
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Write
        );

        //
        //  Note that we have to use the end install path for these, not the
        //  temp path, because these are used by the system, not for
        //  installation.
        //
        strmTarget  << L"@ECHO OFF\n"
                       L"REM Sets up a command window for using the CQC command\n"
                       L"REM line utilities\n\n";

        // Set the path to point to our bin directory
        strmTarget  << L"PATH=%PATH%;"
                    << m_pinfoToUse->strPath() << L"\\Bin;\n";

        // Set up the local log variable
        strmTarget  << L"SET CID_LOCALLOG=%ProgramData%\\CQC\\Logs\\Log.Txt;UTF-16;CQCLgMtx\n";

        // And let the command line apps find the name server
        strmTarget  << L"SET CID_NSADDR="
                    << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":"
                    << m_pinfoToUse->viiNewInfo().ippnNameServer() << kCIDLib::NewLn;

        // Set the data directory
        pathTarget = m_pinfoToUse->strPath();
        pathTarget.AddLevel(L"CQCData");
        strmTarget  << L"SET CQC_DATADIR=" << pathTarget << kCIDLib::NewLn;

        // Set up some error dump variables
        pathTarget = m_pinfoToUse->strPath();
        pathTarget.AddLevels(L"CQCData", L"Logs");
        strmTarget  << L"SET CID_ERRDUMPDIR=" << pathTarget << kCIDLib::NewLn;

        pathTarget = m_pinfoToUse->strPath();
        pathTarget.AddLevel(L"Bin");
        strmTarget  << L"SET _NT_SYMBOL_PATH=" << pathTarget << kCIDLib::NewLn
                    << L"SET CID_LOGMON_SNAP=" << pathTarget << kCIDLib::NewLn;

        strmTarget << kCIDLib::FlushIt;
    }
}


tCIDLib::TVoid TInstallThread::GenConfig()
{
    SetStep(tCQCInst::EInstSteps::GenConfig);

    // It goes into the same directory where the App shell is
    TPathStr pathTarget(m_pinfoToUse->strTmpPath());
    pathTarget.Append(L"\\Bin\\CQCAppShell.List");

    TTextFileOutStream strmTarget
    (
        pathTarget
        , tCIDLib::ECreateActs::CreateAlways
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
        , tCIDLib::EAccessModes::Excl_Write
    );

    strmTarget << L"<?xml version=\"1.0\" encoding=\"ISO-8859-1\" ?>\n"
                  L"<!DOCTYPE CQCSh:ProcRecs PUBLIC \""
                  L"urn:charmedquark.com:CQC-ShellProcList.DTD\" "
                  L"\"CQCAppShell.DTD\">\n"
                  L"<!--\n"
                  L" This file was generated by the CQC installation program\n"
                  L" so any manual changes might be overwritten in subsequent\n"
                  L" installs or updates -->"
                  L"\n\n";

    // Put out the opening root element
    strmTarget  << L"<CQCSh:ProcRecs>\n";

    //
    //  If we are doing the master server, then output the config for
    //  any master server related servers that are required or enabled.
    //
    if (m_pinfoToUse->viiNewInfo().bMasterServer())
    {
        //
        //  Do the name server. We set the local log to the logs directory
        //  that we created. The name server doesn't log to the central
        //  log server, since that would create a circular dependency.
        //
        //  NOTE:   In these, we use the target install path, not the
        //          temp directory, since we are generating files to be
        //          used by the installed system.
        //
        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CIDNameSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/LocalLog=" << m_pinfoToUse->strPath()
                    << L"\\CQCData\\Logs\\NSLog.Txt;UTF-8' /Port="
                    << m_pinfoToUse->viiNewInfo().ippnNameServer() << L"\"\n"
                    << L"        CQCSh:AdminPath=\""
                    << TCIDNameSrvClientProxy::strAdminBinding << L"\"\n"
                    << L"        CQCSh:Level=\"0\"\n"
                    << L"        CQCSh:Title=\"Name Server\""
                    << L"/>\n\n";

        //
        //  Do the log server. It also logs to a local log since if it
        //  screwed up, it might not be able to log to its own log store.
        //
        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CIDLogSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/LocalLog=" << m_pinfoToUse->strPath()
                    << L"\\CQCData\\Logs\\LSLog.Txt;UTF-8' /Port="
                    << m_pinfoToUse->viiNewInfo().ippnLogServer()
                    << L" '/OutPath=" << m_pinfoToUse->strPath()
                    << L"\\CQCData\\Logs'\"\n"
                    << L"        CQCSh:AdminPath=\""
                    << TCIDLogSrvClientProxy::strAdminBinding << L"\"\n"
                    << L"        CQCSh:Level=\"1\"\n"
                    << L"        CQCSh:Title=\"Log Server\""
                    << L"/>\n\n";

        //
        //  Do the data server. We have to tell it where its data
        //  files will be, and how to find the name server.
        //
        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CQCDataSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/DataDir=" << m_pinfoToUse->strPath() << L"\\CQCData'"
                    << L" '/Port=" << m_pinfoToUse->viiNewInfo().ippnDataServer() << L"'"
                    << L" '/NSAddr=" << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                    << L"'\"\n"
                    << L"        CQCSh:AdminPath=\""
                    << kCQCKit::pszCQCDataSrvAdmin << L"\"\n"
                    << L"        CQCSh:Level=\"3\"\n"
                    << L"        CQCSh:Title=\"Data Server\""
                    << L"/>\n\n";

        // If doing the event server
        if (m_pinfoToUse->viiNewInfo().bEventServer())
        {
            strmTarget  << L"    <CQCSh:ProcRec\n"
                        << L"        CQCSh:Name=\"CQCEventSrv\"\n"
                        << L"        CQCSh:Path=\""
                        << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                        << L"        CQCSh:Parms=\""
                        << L" '/DataDir=" << m_pinfoToUse->strPath()
                        << L"\\CQCData' /Port="
                        << m_pinfoToUse->viiNewInfo().ippnEventServer()
                        << L" /EventPort="
                        << m_pinfoToUse->viiNewInfo().ippnEventBroadcast()
                        << L" '/NSAddr="
                        << m_pinfoToUse->viiNewInfo().strMSAddr()
                        << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                        << L"'\"\n"
                        << L"        CQCSh:AdminPath=\""
                        << TEventSrvClientProxy::strCoreAdminBinding << L"\"\n"
                        << L"        CQCSh:Level=\"5\"\n"
                        << L"        CQCSh:Title=\"Event Server\""
                        << L"/>\n\n";
        }

        // If doing the logic server
        if (m_pinfoToUse->viiNewInfo().bLogicServer())
        {
            strmTarget  << L"    <CQCSh:ProcRec\n"
                        << L"        CQCSh:Name=\"CQLogicSrv\"\n"
                        << L"        CQCSh:Path=\""
                        << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                        << L"        CQCSh:Parms=\""
                        << L" '/DataDir=" << m_pinfoToUse->strPath()
                        << L"\\CQCData' /Port="
                        << m_pinfoToUse->viiNewInfo().ippnLogicServer()
                        << L" '/NSAddr="
                        << m_pinfoToUse->viiNewInfo().strMSAddr()
                        << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                        << L"'\"\n"
                        << L"        CQCSh:AdminPath=\""
                        << TLogicSrvClientProxy::strCoreAdminBinding << L"\"\n"
                        << L"        CQCSh:Level=\"5\"\n"
                        << L"        CQCSh:Title=\"Logic Server\""
                        << L"/>\n\n";
        }
    }

    //
    //  If we are doing the app shell, do the config server, since it's always needed
    //  if servers are running.
    //
    if (m_pinfoToUse->bNeedsAppShell())
    {
        TString strBinding(TCIDCfgSrvClientProxy::strAdminBinding);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);

        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CIDCfgSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/RepoPath=" << m_pinfoToUse->strPath()
                    << L"\\CQCData\\Repository' "
                    << L"'/NSAddr=" << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                    << L"' /Port=" << m_pinfoToUse->viiNewInfo().ippnCfgServer();

        if (m_pinfoToUse->viiNewInfo().bMasterServer())
            strmTarget << L" /AltNSScope=/CQC/MSCfg";

        strmTarget  << L"\"\n"
                    << L"        CQCSh:AdminPath=\""
                    << strBinding << L"\"\n"
                    << L"        CQCSh:Level=\"2\"\n"
                    << L"        CQCSh:Title=\"Cfg Server\""
                    << L"/>\n\n";
    }

    //
    //  If we are doing the CQCServer, then generate its config. We have
    //  to tell it where the name server is.
    //
    if (m_pinfoToUse->viiNewInfo().bCQCServer())
    {
        TString strBinding(TCQCSrvAdminClientProxy::strCoreAdminBinding);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);

        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CQCServer\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/DataDir=" << m_pinfoToUse->strPath()
                    << L"\\CQCData' /Port="
                    << m_pinfoToUse->viiNewInfo().ippnCQCServer()
                    << L" /EventPort="
                    << m_pinfoToUse->viiNewInfo().ippnEventBroadcast()
                    << L" '/NSAddr="
                    << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                    << L"'\"\n"
                    << L"        CQCSh:AdminPath=\""
                    << strBinding << L"\"\n"
                    << L"        CQCSh:Level=\"4\"\n"
                    << L"        CQCSh:Title=\"CQC Server\""
                    << L"/>\n\n";
    }

    //
    //  If we are doing the gateway server, then generate its config. We have
    //  to tell it where the name server is.
    //
    if (m_pinfoToUse->viiNewInfo().bXMLGWServer())
    {
        TString strBinding(kCQCKit::pszCQCGWSrvCoreAdmin);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);

        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CQCGWSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/DataDir=" << m_pinfoToUse->strPath()
                    << L"\\CQCData' /Port="
                    << m_pinfoToUse->viiNewInfo().ippnXMLGWServer()
                    << L" '/NSAddr="
                    << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                    << L"'";


        // Pass the ports that are enabled
        if (tCIDLib::bAllBitsOn(m_pinfoToUse->viiNewInfo().eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Insecure))
            strmTarget  << L" /GWPort=" << m_pinfoToUse->viiNewInfo().ippnXMLGW();

        if (tCIDLib::bAllBitsOn(m_pinfoToUse->viiNewInfo().eXMLGWSecureOpts(), tCIDSock::ESecureOpts::Secure))
        {
            strmTarget  << L" /SecPort=" << m_pinfoToUse->viiNewInfo().ippnXMLGWSecure()
                        << L" '/CertInfo=" << m_pinfoToUse->viiNewInfo().strCertInfo()
                        << L"'";
        }

        // Cap off the parms attribute and do the rest
        strmTarget  << L"\"\n"
                    << L"        CQCSh:AdminPath=\"" << strBinding << L"\"\n"
                    << L"        CQCSh:Level=\"5\"\n"
                    << L"        CQCSh:Title=\"XML Gateway Server\""
                    << L"/>\n\n";
    }

    //
    //  If we are doing the web server, then generate its config. We have
    //  to tell it where the name server is.
    //
    if (m_pinfoToUse->viiNewInfo().bWebServer())
    {
        TString strBinding(kCQCKit::pszCQCWebSrvCoreAdmin);
        strBinding.eReplaceToken(TSysInfo::strIPHostName(), kCIDLib::chLatin_h);

        strmTarget  << L"    <CQCSh:ProcRec\n"
                    << L"        CQCSh:Name=\"CQCWebSrv\"\n"
                    << L"        CQCSh:Path=\""
                    << m_pinfoToUse->strPath() << L"\\Bin\"\n"
                    << L"        CQCSh:Parms=\""
                    << L" '/DataDir=" << m_pinfoToUse->strPath()
                    << L"\\CQCData' /Port="
                    << m_pinfoToUse->viiNewInfo().ippnWebServer()
                    << L" '/NSAddr="
                    << m_pinfoToUse->viiNewInfo().strMSAddr()
                    << L":" << m_pinfoToUse->viiNewInfo().ippnNameServer()
                    << L"'";

        // Pass the ports that are enabled
        if (tCIDLib::bAllBitsOn(m_pinfoToUse->viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Insecure))
            strmTarget  << L" /HTTPPort=" << m_pinfoToUse->viiNewInfo().ippnWebServerHTTP();

        if (tCIDLib::bAllBitsOn(m_pinfoToUse->viiNewInfo().eWebSecureOpts(), tCIDSock::ESecureOpts::Secure)
        && !m_pinfoToUse->viiNewInfo().strCertInfo().bIsEmpty())
        {
            strmTarget  << L" /HTTPSPort=" << m_pinfoToUse->viiNewInfo().ippnWebServerHTTPS()
                        << L" '/CertInfo=" << m_pinfoToUse->viiNewInfo().strCertInfo()
                        << L"'";
        }

        // Indicate secure or non-secure help access
        if (m_pinfoToUse->viiNewInfo().bSecureHelp())
            strmTarget << L" /SecureHelp";

        // Close off the quoted parms line and do the rest now
        strmTarget  << L"\"\n"
                    << L"        CQCSh:AdminPath=\"" << strBinding << L"\"\n"
                    << L"        CQCSh:Level=\"5\"\n"
                    << L"        CQCSh:Title=\"Web Server\""
                    << L"/>\n\n";
    }

    // Finish off the root element
    strmTarget  << L"</CQCSh:ProcRecs>\n" << kCIDLib::FlushIt;

    facCQCInst.LogInstallMsg(L"Finished generating config");
}



//
//  This method generates the start menu links that are required according
//  to what is being installed.
//
tCIDLib::TVoid TInstallThread::GenLinks()
{
    SetStep(tCQCInst::EInstSteps::GenLinks);

    //
    //  The first thing we want to do is to create a folder under the programs
    //  part of the start menu for all users. And we'll then put all our links
    //  there.
    //
    TPathStr pathTargetDir
    (
        TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonMenuPrograms)
    );

    // Save a lot of typing below by getting a ref to the new install info
    const TCQCVerInstallInfo& viiNew = m_pinfoToUse->viiNewInfo();

    //
    //  Load the text for the start menu scope name we use for our links.
    //  It needs to be translatable, and we might need it at runtime in
    //  some CQC apps in some cases, so it's a loadable message in the
    //  CQCKit facility.
    //
    TString strLinksScope;
    facCQCKit().bLoadCIDMsg(kKitMsgs::midSMenu_LinkScope, strLinksScope);

    // Add our subdirectory onto it
    pathTargetDir.AddLevel(strLinksScope);

    //
    //  Before we remove the start menu directory, see if there is an IV
    //  entry from a previous install. If so, then check it for any of the
    //  optional parameters so that we can put them back in the new one.
    //
    tCIDLib::TBoolean   bFullScreen = kCIDLib::False;
    tCIDLib::TBoolean   bKiosk = kCIDLib::False;
    tCIDLib::TBoolean   bSignage = kCIDLib::False;
    tCIDLib::TCard4     c4Delay = 0;
    TString             strBinding;
    TString             strDisplayName;
    TString             strInitTmpl;
    {
        // Build up the path to the link
        TString strLnkText;
        facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_IntfView, strLnkText);
        TPathStr pathSrc = pathTargetDir;
        pathSrc.AddLevel(strLnkText);
        pathSrc.AppendExt(L"lnk");

        // And query any previous contents
        tCIDLib::TCard4     c4Err;
        TGUIShellShortCut   gsscOld;
        if (TGUIShell::bQueryShortCut(pathSrc, gsscOld, c4Err))
        {
            //
            //  We got it so let's parse out the parameters and look
            //  for the optional ones and remember them.
            //
            TVector<TString> colParms;
            const tCIDLib::TCard4 c4Count = TExternalProcess::c4BreakOutParms
            (
                gsscOld.strArgs(), colParms
            );

            TString strVal;
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TString& strCur = colParms[c4Index];

                if (strCur.bStartsWithI(L"/Binding="))
                {
                    strBinding = strCur;
                    strBinding.Cut(0, 9);
                }
                 else if (strCur.bCompareI(L"/FullScreen"))
                {
                    bFullScreen = kCIDLib::True;
                }
                 else if (strCur.bCompareI(L"/Kiosk"))
                {
                    bKiosk = kCIDLib::True;
                }
                 else if (strCur.bStartsWithI(L"/Delay="))
                {
                    strVal = strCur;
                    strVal.Cut(0, 7);
                    strVal.bToCard4(c4Delay);
                }
                 else if (strCur.bStartsWithI(L"/InitTmpl="))
                {
                    strInitTmpl = strCur;
                    strInitTmpl.Cut(0, 10);
                }
                 else if (strCur.bStartsWithI(L"/Signage="))
                {
                    bSignage = kCIDLib::True;
                    strDisplayName= strCur;
                    strDisplayName.Cut(0, 9);
                }
            }
        }
    }

    //
    //  If the directory exists, then remove it, because the newer versions
    //  may have different links.
    //
    if (TFileSys::bExists(pathTargetDir, tCIDLib::EDirSearchFlags::AllDirs))
        TFileSys::RemovePath(pathTargetDir);

    //
    //  If no client side options were selected, then we are done now that
    //  we've removed any old options. Else, install the options requested.
    //
    if (viiNew.bAnyClient())
    {
        TPathStr pathPath;
        TPathStr pathIcon;
        TString  strText;

        // Now make the directory if needed
        TFileSys::MakePath(pathTargetDir);

        //
        //  Now create a collection of link objects that we will pass in to
        //  be created under the target directory.
        //
        TBag<TGUIShellShortCut> colShortCuts;

        // Set up the common parms that almost all of them will have
        TPathStr pathCommParms(L"\"/DataDir=");
        pathCommParms.Append(m_pinfoToUse->strPath());
        pathCommParms.AddLevel(L"CQCData\"");
        pathCommParms.Append(L" /NSAddr=");
        pathCommParms.Append(viiNew.strMSAddr());
        pathCommParms.Append(L":");
        pathCommParms.AppendFormatted(viiNew.ippnNameServer());

        // The working path for all but the command prompt is the bin directory
        TPathStr pathWorking(m_pinfoToUse->strPath());
        pathWorking.AddLevel(L"Bin");
        TPathStr pathAltParms;

        //
        //  If asked to install the admin client, then do so. Also, for now, we install
        //  the separate CML/PDL driver IDEs along with it.
        //
        if (viiNew.bCQCClient())
        {
            // Load the text for the link
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_CQCClient, strText);

            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCAdmin.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"AdminIntf.ico");
            TGUIShellShortCut gsscClient
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscClient);

            // CML IDE
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_CMLDriverIDE, strText);
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCDrvDev.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"DriverIDE.ico");
            TGUIShellShortCut gsscCMLIDE
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscCMLIDE);

            // PDL IDE
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_PDLDriverIDE, strText);
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCProtoDev.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"DriverIDE.ico");
            TGUIShellShortCut gsscPDLIDE
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscPDLIDE);
        }

        // Always do the log monitor
        {
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_LogMon, strText);
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CIDLogMon.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"LogMon.ico");
            TGUIShellShortCut gsscLogMon
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscLogMon);
        }

        //
        //  Do one for the interface viewer if asked to. We have a special
        //  issue here that we want to look at the existing one if it's there
        //  and look for some optional parameters which we'll put back on
        //  the newly generated one. We saved away the existing params up
        //  above.
        //
        if (viiNew.bIntfViewer())
        {
            // Load the text for the link
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_IntfView, strText);

            // Set the port if enabling the control interface
            pathAltParms = pathCommParms;
            if (viiNew.bIntfViewerCtrl())
            {
                pathAltParms.Append(L" /Port=");
                pathAltParms.AppendFormatted(viiNew.ippnIVCtrl());
            }

            // If we are carrying over old parms, add them
            if (bFullScreen)
                pathAltParms.Append(L" /FullScreen");
            if (bKiosk)
                pathAltParms.Append(L" /Kiosk");
            if (!strBinding.bIsEmpty())
            {
                pathAltParms.Append(L" /Binding=\"");
                pathAltParms.Append(strBinding);
                pathAltParms.Append(L'"');
            }
            if (bSignage)
            {
                pathAltParms.Append(L" /Signage=\"");
                pathAltParms.Append(strDisplayName);
                pathAltParms.Append(L'"');
            }

            if (c4Delay)
            {
                pathAltParms.Append(L" /Delay=");
                pathAltParms.AppendFormatted(c4Delay);
            }
            if (!strInitTmpl.bIsEmpty())
            {
                pathAltParms.Append(L" /InitTmpl=\"");
                pathAltParms.Append(strInitTmpl);
                pathAltParms.Append(L'"');
            }

            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCIntfView.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"IntfViewer.ico");
            TGUIShellShortCut gsscView
            (
                pathPath, strText, pathAltParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscView);
        }

        // Do one for the media repository manager if we are installing it
        if (viiNew.bMediaRepoMgr())
        {
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_MediaRepoMgr, strText);
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"MediaRepoMgr.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"RepoMgr.ico");
            TGUIShellShortCut gsscMRMgr
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscMRMgr);
        }

        // Always do the uninstaller
        {
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_Uninst, strText);
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCUninst.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"CQCApp.ico");
            TGUIShellShortCut gsscUninst
            (
                pathPath, strText, pathCommParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscUninst);
        }

        // Do one for the tray monitor if we are installing it
        if (viiNew.bTrayMon())
        {
            // Load the text for the link
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_TrayMonitor, strText);

            // Set the app control and iTunes option parameters as required
            pathAltParms = pathCommParms;
            if (viiNew.bAppCtrl())
                pathAltParms.Append(L" /AppCtrl");
            if (viiNew.biTunes())
                pathAltParms.Append(L" /iTunes");

            if (!viiNew.strTrayBinding().bIsEmpty())
            {
                pathAltParms.Append(L" /Binding=");
                pathAltParms.Append(viiNew.strTrayBinding());
            }

            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCTrayMon.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"CQCApp.ico");
            TGUIShellShortCut gsscMWS
            (
                pathPath, strText, pathAltParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscMWS);
        }

        // Do one for the CQCVoice if we are installing it
        if (viiNew.bCQCVoice())
        {
            // Load the text for the link
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_CQCVoice, strText);

            // We have to pass the user name and password
            pathAltParms = pathCommParms;
            pathAltParms.Append(L" /User=");
            pathAltParms.Append(viiNew.strCQCVoiceUserName());
            pathAltParms.Append(L" /Password=");
            pathAltParms.Append(viiNew.strCQCVoicePassword());

            if (tCIDLib::bAllBitsOn(viiNew.eCQCVoiceRepStyle(), tCQCKit::EReplyStyles::Spoken))
                pathAltParms.Append(L" /SpeakReplies");

            if (tCIDLib::bAllBitsOn(viiNew.eCQCVoiceRepStyle(), tCQCKit::EReplyStyles::Visual))
                pathAltParms.Append(L" /TextReplies");

            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCVoice.exe");
            pathIcon = m_pinfoToUse->strPath();
            pathIcon.AddLevels(L"Bin\\AppIcons", L"CQCVoice.ico");
            TGUIShellShortCut gsscMWS
            (
                pathPath, strText, pathAltParms, pathIcon, strText, pathWorking
            );
            colShortCuts.objAdd(gsscMWS);
        }

        //
        //  And do one that will open a command prompt and run our generated environment
        //  setup file, so that they can do operations from the command line.
        //
        //  The working directory here is set to the %appdata% token, to get the correct
        //  app data directory for whoever is logged in at the OS level.
        //
        if (viiNew.bCmdPrompt())
        {
            // Load the text for the link
            facCQCInst.bLoadCIDMsg(kCQCInstMsgs::midLink_CmdPrompt, strText);

            pathAltParms = L"/k \"";
            pathAltParms.Append(m_pinfoToUse->strPath());
            pathAltParms.AddLevels(L"Bin", L"CQCEnv.Cmd\"");

            colShortCuts.objAdd
            (
                TGUIShellShortCut
                (
                    L"%SystemRoot%\\system32\\cmd.exe"
                    , strText
                    , pathAltParms
                    , L"%SystemRoot%\\system32\\cmd.exe"
                    , strText
                    , L"%appdata%"
                )
            );
        }

        // And now create them in the target directory we created
        try
        {
            facCQCInst.LogInstallMsg(L"Creating short cut group");
            TGUIShell::CreateShortCutGroup(pathTargetDir, colShortCuts);
            facCQCInst.LogInstallMsg(L"Finished creating short cut group");
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogInstallMsg(L"Link generation failed");
            throw;
        }

        //
        //  A special case for the tray monitor if it's enabled for auto-
        //  startup. If so, we need create a scheduled task to do that.
        //
        //  If not, then we need to remove the task.
        //
        if (viiNew.bTrayMon() && viiNew.bTrayAutoStart())
        {
            //
            //  In this case we just build up the whole path and parameters
            //  in one string. Quote the program path in case it has spaces
            //  in it.
            //
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCTrayMon.exe");

            pathAltParms = pathCommParms;
            if (viiNew.bAppCtrl())
                pathAltParms.Append(L" /AppCtrl");
            if (viiNew.biTunes())
                pathAltParms.Append(L" /iTunes");
            if (!viiNew.strTrayBinding().bIsEmpty())
            {
                pathAltParms.Append(L" /Binding=");
                pathAltParms.Append(viiNew.strTrayBinding());
            }

            // Tell it to run with admin rights, at logon of any user
            const tCIDLib::TBoolean bRes = TKrnlTaskSched::bCreateSchedTask
            (
                kCQCInst::strTask_TrayAppAutoStart.pszBuffer()
                , L"CQC Installer"
                , pathPath.pszBuffer()
                , pathAltParms.pszBuffer()
                , kCIDLib::True
                , TKrnlTaskSched::EStartTypes::OnLogon
                , 30
            );

            if (!bRes)
                facCQCInst.LogInstallMsg(L"Couldn't create Tray Monitor Auto-Run task");
        }
         else
        {
            if (!TKrnlTaskSched::bDeleteSchedTask(kCQCInst::strTask_TrayAppAutoStart.pszBuffer()))
                facCQCInst.LogInstallMsg(L"Couldn't delete Tray Auto-Run task");
        }

        // And the same for CQCVoice
        if (viiNew.bCQCVoice() && viiNew.bCQCVoiceAutoStart())
        {
            //
            //  In this case we just build up the whole path and parameters
            //  in one string. Quote the program path in case it has spaces
            //  in it.
            //
            pathPath = m_pinfoToUse->strPath();
            pathPath.AddLevels(L"Bin", L"CQCVoice.exe");

            pathAltParms = pathCommParms;
            pathAltParms.Append(L" /User=");
            pathAltParms.Append(viiNew.strCQCVoiceUserName());
            pathAltParms.Append(L" /Password=");
            pathAltParms.Append(viiNew.strCQCVoicePassword());

            // Have it delay for 30 seconds to allow the system to get up and going
            pathAltParms.Append(L" /Delay=30");

            // Tell it to run with admin rights, at logon of any user
            const tCIDLib::TBoolean bRes = TKrnlTaskSched::bCreateSchedTask
            (
                kCQCInst::strTask_CQCVoiceAutoStart.pszBuffer()
                , L"CQC Installer"
                , pathPath.pszBuffer()
                , pathAltParms.pszBuffer()
                , kCIDLib::True
                , TKrnlTaskSched::EStartTypes::OnLogon
                , 30
            );

            if (!bRes)
                facCQCInst.LogInstallMsg(L"Couldn't create CQC Voice Auto-Run task");
        }
         else
        {
            if (!TKrnlTaskSched::bDeleteSchedTask(kCQCInst::strTask_CQCVoiceAutoStart.pszBuffer()))
                facCQCInst.LogInstallMsg(L"Couldn't delete CQC Voice Auto-Run task");
        }
    }
    facCQCInst.LogInstallMsg(L"Link generation completed");
}


//
//  This guy installs or updates the service definition for our client service. We
//  always just update it since the settings could have changed since last time and
//  we don't want to bother keeping up with that.
//
tCIDLib::TVoid TInstallThread::InstallClService(tCIDLib::TStrList& colParms)
{
    SetStep(tCQCInst::EInstSteps::InstClService);

    TPathStr pathPath(m_pinfoToUse->strPath());
    pathPath.AddLevels(L"Bin", L"CQCClService.exe");

    // Some const strings we use in multiple places below
    const tCIDLib::TCh* const pszDispName    = L"CQC Client Service";
    const tCIDLib::TCh* const pszDescription = L"Provides client support for the Charmed Quark Controller";

    //
    //  Set up the parms, which we have already created, but we have to pass
    //  in raw string pointer to this kernel level call.
    //
    TPathStr pathTmp;
    colParms.RemoveAll();

    pathTmp = L"/Port=";
    pathTmp.AppendFormatted(m_pinfoToUse->viiNewInfo().ippnClService());
    colParms.objAdd(pathTmp);

    pathTmp = L"/NSAddr=";
    pathTmp.Append(m_pinfoToUse->viiNewInfo().strMSAddr());
    pathTmp.Append(L":");
    pathTmp.AppendFormatted(m_pinfoToUse->viiNewInfo().ippnNameServer());
    colParms.objAdd(pathTmp);

    pathTmp = L"/DataDir=";
    pathTmp.Append(m_pinfoToUse->strPath());
    pathTmp.Append(L"\\CQCData");
    colParms.objAdd(pathTmp);

    const tCIDLib::TCard4 c4ParmCount = colParms.c4ElemCount();
    const tCIDLib::TCh** apszParms = new const tCIDLib::TCh*[c4ParmCount];
    TArrayJanitor<const tCIDLib::TCh*> janParms(apszParms);
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
        apszParms[c4Index] = colParms[c4Index].pszBuffer();

    // Setup dependencies
    const tCIDLib::TCh* apszDepends[2];
    apszDepends[0] = L"TcpIP";
    apszDepends[1] = L"LanmanWorkstation";


    //
    //  First, see if there is already a service there. If so, then we will just
    //  update it. Else, we have to install one.
    //
    //  In debug mode, we default it to manual start mode, since otherwise it would be
    //  a serious pain during testing. In production mode it's always auto-start
    //  initially, but they can change it.
    //
    tCIDKernel::TWSvcHandle hscvCl = TKrnlWin32Service::hsvcOpen(kCQCKit::pszClServiceName);
    tCIDKernel::EWSrvStarts eStartType;

    #if CID_DEBUG_ON
    eStartType = tCIDKernel::EWSrvStarts::Demand;
    #else
    eStartType = tCIDKernel::EWSrvStarts::Auto;
    #endif

    if (hscvCl)
    {
        //
        //  We already have one, so just update it. Use the startup mode that
        //  we were given during the installation.
        //
        const tCIDLib::TBoolean bRes = TKrnlWin32Service::bChangeConfig
        (
            hscvCl
            , pathPath.pszBuffer()
            , pszDispName
            , pszDescription
            , eStartType
            , apszDepends
            , tCIDLib::c4ArrayElems(apszDepends)
            , apszParms
            , c4ParmCount
        );

        if (!bRes)
        {
            TKrnlWin32Service::bClose(hscvCl);
            hscvCl = 0;
            facCQCInst.LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_UpdateSvc
                , TKrnlError::kerrLast()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TString(L"client")
            );
        }
    }
     else
    {
        // No existing one, so let's create a new one
        hscvCl = TKrnlWin32Service::hsvcCreate
        (
            pathPath.pszBuffer()
            , kCQCKit::pszClServiceName
            , pszDispName
            , pszDescription
            , eStartType
            , apszDepends
            , tCIDLib::c4ArrayElems(apszDepends)
            , apszParms
            , c4ParmCount
        );
    }

    if (!hscvCl)
    {
        facCQCInst.LogInstallMsg(L"Client service installation failed");
        facCQCInst.ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCQCInstErrs::errcFail_CreateSvc
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TString(L"client")
        );
    }
     else
    {
        TKrnlWin32Service::bClose(hscvCl);
    }
}


//
//  This guy installs or updates the service definition for our app shell service. We
//  always just update it since the settings could have changed since last time and
//  we don't want to bother keeping up with that.
//
tCIDKernel::TWSvcHandle TInstallThread::hsvcInstallAppShell(tCIDLib::TStrList& colParms)
{
    SetStep(tCQCInst::EInstSteps::InstAppShell);

    TPathStr pathPath(m_pinfoToUse->strPath());
    pathPath.AddLevels(L"Bin", L"CQCAppShell.exe");

    // Some const strings we use in multiple places below
    const tCIDLib::TCh* const pszDispName    = L"CQC Application Shell";
    const tCIDLib::TCh* const pszDescription = L"Manages background applications for the Charmed Quark Controller";

    //
    //  Set up the parms
    //
    colParms.RemoveAll();
    TPathStr pathTmp(L"/LocalLog=");
    pathTmp.Append(m_pinfoToUse->strPath());
    pathTmp.AddLevel(L"CQCData\\Logs\\AppShLog.Txt;UTF-8;CQCLgMtx");
    colParms.objAdd(pathTmp);

    pathTmp = L"/Port=";
    pathTmp.AppendFormatted(m_pinfoToUse->viiNewInfo().ippnAppShell());
    colParms.objAdd(pathTmp);

    if (!m_pinfoToUse->viiNewInfo().bMasterServer())
    {
        pathTmp = L"/NSAddr=";
        pathTmp.Append(m_pinfoToUse->viiNewInfo().strMSAddr());
        pathTmp.Append(L":");
        pathTmp.AppendFormatted(m_pinfoToUse->viiNewInfo().ippnNameServer());
        colParms.objAdd(pathTmp);
    }

    const tCIDLib::TCard4 c4ParmCount = colParms.c4ElemCount();
    const tCIDLib::TCh** apszParms = new const tCIDLib::TCh*[c4ParmCount];
    TArrayJanitor<const tCIDLib::TCh*> janParms(apszParms);

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ParmCount; c4Index++)
        apszParms[c4Index] = colParms[c4Index].pszBuffer();

    // Setup dependencies
    const tCIDLib::TCh* apszDepends[2];
    apszDepends[0] = L"TcpIP";
    apszDepends[1] = L"LanmanWorkstation";

    //
    //  First, see if there is already a service there. If so, then we will
    //  just update it. Else, we have to install one.
    //
    tCIDKernel::TWSvcHandle hsvcRet = TKrnlWin32Service::hsvcOpen(kCQCKit::pszCQCShellName);
    if (hsvcRet)
    {
        //
        //  We already have one, so just update it. Use the startup mode that
        //  we were given during the installation.
        //
        const tCIDLib::TBoolean bRes = TKrnlWin32Service::bChangeConfig
        (
            hsvcRet
            , pathPath.pszBuffer()
            , pszDispName
            , pszDescription
            , facCQCInst.eStartMode()
            , apszDepends
            , tCIDLib::c4ArrayElems(apszDepends)
            , apszParms
            , c4ParmCount
        );

        if (!bRes)
        {
            TKrnlWin32Service::bClose(hsvcRet);
            hsvcRet = 0;
            facCQCInst.LogKrnlErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_UpdateSvc
                , TKrnlError::kerrLast()
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::CantDo
                , TString(L"app shell")
            );
        }
    }
     else
    {
        // No exising one, so let's create a new one
        hsvcRet = TKrnlWin32Service::hsvcCreate
        (
            pathPath.pszBuffer()
            , kCQCKit::pszCQCShellName
            , pszDispName
            , pszDescription
            , facCQCInst.eStartMode()
            , apszDepends
            , tCIDLib::c4ArrayElems(apszDepends)
            , apszParms
            , c4ParmCount
        );
    }

    if (!hsvcRet)
    {
        facCQCInst.LogInstallMsg(L"App shell installation failed");
        facCQCInst.ThrowKrnlErr
        (
            CID_FILE
            , CID_LINE
            , kCQCInstErrs::errcFail_CreateSvc
            , TKrnlError::kerrLast()
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::CantDo
            , TString(L"app shell")
        );
    }
    return hsvcRet;
}


//
//  These are called to create all of the output directories that are required
//  on the target machine, which varies according to what components are being
//  installed.
//
//  We use a simple data structure that allows us to do all of this in a fairly
//  compact way.
//
//  We create the following directories:
//
//  For all installs
//      Bin                     - Programs and runtimes
//      CQCData
//          HTMLRoot            - Web server files
//            CQSL              - Our own HTML content, currently only any on the MS
//          Logs                - Log Server and local log output
//          Repository          - For the config repository files
//          Shared              - Shared client/server stuff
//          Users               - For local user specific GUI program config
//
//
//  For all installs. Primarily for client side tools but also the client service
//  which runs on all systems.
//
//      [commonappdata]\CQCData\[systemid]
//          Client              - Client area
//              Data            - Client side driver data files
//          Local               - Data for local read only access by clients on all machines
//              DevClasses      - The V2 device class validation files (used by driver IDEs)
//              ZWInfo          - The old (VRC0P) Z-Wave device info files
//          CQCVoice            - The support files used by CQCVoice
//          Logs                - Log files
//          Users               - Per user data (last window pos, etc...)
//
//  For CQCServer install
//      CQCData
//          Server              - Server area
//              Data            - Server side data files
//
//  For Master Server installs
//      CQCData
//        DataServer            - For the data server's files
//          Events              - For the various types of events
//            Monitors          - For event monitors
//                System        - For system event monitors
//                User          - For user event monitors
//            Scheduled         - For scheduled events
//                System        - For system scheduled events
//                User          - For user scheduled events
//            Triggered         - For triggered events
//                System        - For system triggered events
//                User          - For user triggered events
//          GlobalActs          - For the global actions
//              System          - For system global actions
//              User            - For user global actions
//          Interfaces          - For the data server's interface templates
//              System          - For system templates
//              User            - For user templates
//          Images              - For the interface images
//              System          - For system images
//              User            - For user images
//                CQC           - For some special purpose user images
//                  Blanker     -   Used by the blanker slide show mode
//          IRModels            - IR device model files
//              GlobalCache     - IR files for the GC-100 blaster
//              RedRat2         - IR files for the RedRat2 blaster
//              RedRat3         - IR files for the RedRat3 blaster
//              USB-UIRT        - IR files for the USBUIRT blaster
//          Manifests           - System driver manifest files
//              User            - User driver manifest files
//          Macros              - Root of the CML macro namespace
//              User            - User macros
//              System          - System macros
//          Protocols           - Protocol description language files
//              User            - User protocol description language files
//          ZWU3Info            - The new (native) Z-Wave driver's device info files
//        ImgMaps             - To hold image maps
//        SysCfg              - For room/auto-gen configuration info
//
tCIDLib::TVoid
TInstallThread::MakeDirSet( const   TString&                strParent
                            , const tCQCInst::TInstDirInfo* pdiriToCreate
                            , const tCIDLib::TCard4         c4Count)
{
    //
    //  Loop through the items, adding and removing levels as indicate by
    //  the level value of each one.
    //
    TPathStr pathCur(strParent);

    tCIDLib::TCard4 c4CurLevel = 0;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const tCQCInst::TInstDirInfo& diriCur = pdiriToCreate[c4Index];

        //
        //  If this one is a lower level, then add the previous items' name
        //  to this one. If it's a higher level, then remove levels to get
        //  back down to this level.
        //
        if (diriCur.c4Level > c4CurLevel)
        {
            //
            //  It can only be one more, else it's an error. And this cannot
            //  be the 0th item, since there is no previous item.
            //
            #if CID_DEBUG_ON
            if ((diriCur.c4Level > c4CurLevel + 1) || !c4Index)
            {
                facCQCInst.LogInstallMsg(L"Bad directory creation level");
                facCQCInst.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCInstErrs::errcFail_CreateDir
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Internal
                );
            }
            #endif
            pathCur.AddLevel(pdiriToCreate[c4Index - 1].pszPath);
            c4CurLevel = diriCur.c4Level;
        }
         else if (diriCur.c4Level < c4CurLevel)
        {
            while (c4CurLevel > diriCur.c4Level)
            {
                pathCur.bRemoveLevel();
                c4CurLevel--;
            }

            // Get rid of any trailing separator
            pathCur.bRemoveTrailingSeparator();
        }

        if (!TFileSys::bExists(pathCur, diriCur.pszPath, tCIDLib::EDirSearchFlags::NormalDirs))
            TFileSys::MakeSubDirectory(diriCur.pszPath, pathCur);
    }
}

//
//  Makes all of the required directories.
//
tCIDLib::TVoid TInstallThread::MakeDirectories()
{
    SetStep(tCQCInst::EInstSteps::CreateNewDirs);

    // Paths for all installations
    static tCQCInst::TInstDirInfo adiriAll[] =
    {
          { L"Bin"              , 0 }
        , { L"CQCData"          , 0 }
        , {   L"HTMLRoot"       , 1 }
        , {     L"CQSL"         , 2 }
        , {     L"Secure"       , 2 }
        , {       L"Admin"      , 3 }
        , {       L"Normal"     , 3 }
        , {       L"Power"      , 3 }
        , {   L"Logs"           , 1 }
        , {   L"Repository"     , 1 }
        , {   L"Shared"         , 1 }
    };

    //
    //  Paths for client installations, but really always done since
    //  there are still simple command line client tools potentially.
    //
    static tCQCInst::TInstDirInfo adiriClient[] =
    {
          { L"Client"           , 0 }
        , {  L"Data"            , 1 }
        , { L"Local"            , 0 }
        , {  L"DevClasses"      , 1 }
        , {  L"ZWInfo"          , 1 }
        , { L"CQCVoice"         , 0 }
        , { L"Logs"             , 0 }
        , { L"Users"            , 0 }
    };

    // Paths for CQCServer installations
    static tCQCInst::TInstDirInfo adiriServer[] =
    {
          { L"Server"           , 0 }
        , {  L"Data"            , 1 }
    };

    // Paths for master server installations
    static tCQCInst::TInstDirInfo adiriMaster[] =
    {
          { L"DataServer"       , 0 }
        , {  L"Events"          , 1 }
        , {    L"Monitors"      , 2 }
        , {      L"System"      , 3 }
        , {      L"User"        , 3 }
        , {    L"Scheduled"     , 2 }
        , {      L"System"      , 3 }
        , {      L"User"        , 3 }
        , {    L"Triggered"     , 2 }
        , {      L"System"      , 3 }
        , {      L"User"        , 3 }
        , {  L"GlobalActs"      , 1 }
        , {   L"System"         , 2 }
        , {   L"User"           , 2 }
        , {  L"Interfaces"      , 1 }
        , {   L"System"         , 2 }
        , {   L"User"           , 2 }
        , {  L"Images"          , 1 }
        , {   L"System"         , 2 }
        , {   L"User"           , 2 }
        , {    L"CQC"           , 3 }
        , {     L"Blanker"      , 4 }
        , {  L"IRModels"        , 1 }
        , {    L"GlobalCache"   , 2 }
        , {    L"RedRat2"       , 2 }
        , {    L"RedRat3"       , 2 }
        , {    L"USB-UIRT"      , 2 }
        , {  L"Manifests"       , 1 }
        , {    L"User"          , 2 }
        , {  L"Macros"          , 1 }
        , {    L"System"        , 2 }
        , {    L"User"          , 2 }
        , {  L"Protocols"       , 1 }
        , {    L"User"          , 2 }
        , {  L"ZWU3Info"        , 1 }
        , {  L"ImgMaps"         , 0 }
        , {  L"SysCfg"          , 0 }
    };

    // Call a helper to create each set of paths
    MakeDirSet(m_pinfoToUse->strTmpPath(), adiriAll, tCIDLib::c4ArrayElems(adiriAll));

    // The rest use CQCData as their root, so gen that up
    TPathStr pathCQCData(m_pinfoToUse->strTmpPath());
    pathCQCData.AddLevel(L"CQCData");

    if (m_pinfoToUse->viiNewInfo().bCQCServer())
        MakeDirSet(pathCQCData, adiriServer, tCIDLib::c4ArrayElems(adiriServer));

    if (m_pinfoToUse->viiNewInfo().bMasterServer())
        MakeDirSet(pathCQCData, adiriMaster, tCIDLib::c4ArrayElems(adiriMaster));

    //
    //  We always make the client side dirs, since even if they don't install any of
    //  the main ones, there are still client side command line tools.
    //
    //  The top level one is special if we have to create it since it
    //  has to have access rights set, so we check that first.
    //
    //  If we are coming from a pre-3.4.13 version, then remove the old
    //  CQC directory that might have gotten created in a previous (aborted)
    //  attempt to make this changeover to the common data dir.
    //
    TPathStr pathTmp = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonAppData);
    pathTmp.AddLevel(L"CQC");
    if (TFileSys::bIsDirectory(pathTmp))
        TFileSys::RemovePath(pathTmp);

    // Now create the directories under it
    MakeDirSet(m_strClientDir, adiriClient, tCIDLib::c4ArrayElems(adiriClient));
}


//
//  This is called if the install fails. We look at the last state that was
//  set and use that to know what to do to recover.
//
tCIDLib::TVoid TInstallThread::Recover()
{
    // Save the point that we got to before we start recovering
    const tCQCInst::EInstSteps eLastStep = m_eCurStep;

    // Now set the new step as recovering and log tha twe are doing it
    SetStep(tCQCInst::EInstSteps::Recovering);

    //
    //  It is before swapped dirs, then we don't have to do anything. All
    //  we've done is put stuff into the temp directory. And if we are not
    //  doing an upgrade or refresh, then nothing to do either.
    //
    if ((m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Clean)
    ||  (eLastStep < tCQCInst::EInstSteps::StartSwapDirs))
    {
        return;
    }

    try
    {
        //
        //  If we've actually got any directories swapped, then swap them
        //  back.
        //
        if (eLastStep > tCQCInst::EInstSteps::StartSwapDirs)
        {
            TPathStr pathTmp;
            TPathStr pathTmp2;

            //
            //  First of all, if we got a new install, rename it to BadInst,
            //  getting rid of any existing bad install directory first.
            //
            pathTmp = m_pinfoToUse->strPath();
            pathTmp.AddLevel(L"BadInst");
            if (TFileSys::bIsDirectory(pathTmp))
                TFileSys::RemovePath(pathTmp);

            // Now create it (or recreate it empty)
            TFileSys::MakePath(pathTmp);

            //
            //  And move the current Bin/CQCData diretories down into it if
            //  these are new ones.
            //
            if (eLastStep >= tCQCInst::EInstSteps::NewBin)
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevel(L"Bin");
                pathTmp2 = m_pinfoToUse->strPath();
                pathTmp2.AddLevel(L"BadInst\\Bin");
                TFileSys::Rename(pathTmp, pathTmp2);
            }

            if (eLastStep >= tCQCInst::EInstSteps::NewData)
            {
                pathTmp = m_pinfoToUse->strPath();
                pathTmp.AddLevel(L"CQCData");
                pathTmp2 = m_pinfoToUse->strPath();
                pathTmp2.AddLevel(L"BadInst\\CQCData");
                TFileSys::Rename(pathTmp, pathTmp2);
            }

            //
            //  And now put the saved old directories back, if we saved
            //  them and they are present.
            //
            if (TFileSys::bExists(m_pinfoToUse->strPath(), L"SaveInst", tCIDLib::EDirSearchFlags::NormalDirs))
            {
                if (eLastStep >= tCQCInst::EInstSteps::SaveBin)
                {
                    pathTmp = m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"SaveInst\\Bin");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalDirs))
                    {
                        pathTmp2 = m_pinfoToUse->strPath();
                        pathTmp2.AddLevel(L"Bin");
                        TFileSys::Rename(pathTmp, pathTmp2);
                    }
                }

                if (eLastStep >= tCQCInst::EInstSteps::SaveData)
                {
                    pathTmp = m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"SaveInst\\CQCData");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalDirs))
                    {
                        pathTmp2 = m_pinfoToUse->strPath();
                        pathTmp2.AddLevel(L"CQCData");
                        TFileSys::Rename(pathTmp, pathTmp2);
                    }
                }

                // Copy back up the version info and release notes
                if (eLastStep >= tCQCInst::EInstSteps::SaveVer)
                {
                    // Delete any new version file we stored
                    pathTmp =  m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"CQCInstVersion.Info");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalFiles))
                        TFileSys::DeleteFile(pathTmp);

                    // And copy the old one up
                    pathTmp = m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"SaveInst\\CQCInstVersion.Info");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalFiles))
                    {
                        pathTmp2 = m_pinfoToUse->strPath();
                        pathTmp2.AddLevel(L"CQCInstVersion.Info");
                        TFileSys::Rename(pathTmp, pathTmp2);
                    }
                }

                // Copy back up the version info and release notes
                if (eLastStep >= tCQCInst::EInstSteps::SaveNotes)
                {
                    // Delete any new notes file we stored
                    pathTmp =  m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"ReleaseNotes.htm");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalFiles))
                    {
                        // Take the read only attrs off first
                        TFileSys::SetPerms(pathTmp, tCIDLib::EFilePerms::AllWE);
                        TFileSys::DeleteFile(pathTmp);
                    }

                    pathTmp = m_pinfoToUse->strPath();
                    pathTmp.AddLevel(L"SaveInst\\ReleaseNotes.htm");
                    if (TFileSys::bExists(pathTmp, tCIDLib::EDirSearchFlags::NormalFiles))
                    {
                        pathTmp2 = m_pinfoToUse->strPath();
                        pathTmp2.AddLevel(L"ReleaseNotes.htm");
                        TFileSys::Rename(pathTmp, pathTmp2);
                    }
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
        facCQCInst.LogInstallMsg(L"Recovery failed");
    }

    catch(...)
    {
        facCQCInst.LogInstallMsg(L"Recovery failed with unknown exception");
    }

    facCQCInst.LogInstallMsg(L"Recovery completed");
}


// We need to set some stuff in the registry for the background servers to use.
tCIDLib::TVoid TInstallThread::SetNonMSRegVars()
{
    //
    //  If this is not the master server, store the server security info that the
    //  'check ms' panel got. If it is the MS, then they were set when we updated
    //  the security repo.
    //
    if (!m_pinfoToUse->viiNewInfo().bMasterServer())
    {
        m_strSrvSecName = m_pinfoToUse->strSrvUser();
        m_strSrvSecPW = m_pinfoToUse->strSrvPassword();
    }

    tCIDLib::TBoolean bCreated;
    tCIDKernel::TWRegHandle hkeyCQS = TKrnlWin32Registry::hkeyCreateOrOpenSubKey
    (
        tCIDKernel::ERootKeys::LocalMachine
        , L"SOFTWARE"
        , kCQCKit::pszReg_Key
        , bCreated
        , tCIDKernel::ERegAccFlags::StdOwned
    );

    if (!hkeyCQS)
    {
        facCQCInst.LogInstallMsg(L"Could not open the CQC registry key");
        return;
    }

    tCIDLib::TBoolean bRes = TKrnlWin32Registry::bSetStrValue
    (
        hkeyCQS, kCQCKit::pszReg_SrvUser, m_strSrvSecName.pszBuffer()
    );
    if (!bRes)
    {
        facCQCInst.LogInstallMsg(L"Could not set the server user account registry entry");
        return;
    }


    bRes = TKrnlWin32Registry::bSetStrValue
    (
        hkeyCQS, kCQCKit::pszReg_SrvPass, m_strSrvSecPW.pszBuffer()
    );
    if (!bRes)
    {
        facCQCInst.LogInstallMsg(L"Could not set the server password registry entry");
        return;
    }
}


//
//  This is called before the beginning of each step. We store it as our
//  current step, and we post an event to the main window to let him know
//  what's going on.
//
tCIDLib::TVoid TInstallThread::SetStep(const tCQCInst::EInstSteps eCurStep)
{
    m_eCurStep = eCurStep;
    m_ppanStatus->PostEvent(eCurStep);
}


//
//  Called if an exception occurs. It lots the info, then shows the error to
//  the user.
//
tCIDLib::TVoid TInstallThread::ShowError(const TError& errToCatch)
{
    facCQCInst.LogInstallMsg(L"An exception occured during installation");
    if (!errToCatch.bLogged())
        TModule::LogEventObj(errToCatch);

    TInstQueryPacket qpackRemove
    (
        facCQCInst.strMsg(kCQCInstErrs::errcFail_ExceptInInst), errToCatch
    );
    m_pwndOwner->SendData(kCQCInst::wmsgQueryId, &qpackRemove, tCIDLib::ESyncTypes::Synchronous);
}


//
//  We copy any data from the old installation to the new temp install dir,
//  modifying them as we go if required, based on the old version.
//
tCIDLib::TVoid TInstallThread::UpgradeClientData()
{
    facCQCInst.LogInstallMsg(L"Upgrading client data");

    // Currently nothing to do

    facCQCInst.LogInstallMsg(L"Client data upgrade complete");
}


//
//  Update any common data that always exists whether a client or server installation.
//
tCIDLib::TVoid TInstallThread::UpdateCommonData()
{
    facCQCInst.LogInstallMsg(L"Upgrading common data");

    //
    //  Build the source repo path and see if it's there. If not, nothing to do. We
    //  always read in the objects from the source and write them to a new target
    //  repository. This gives us the opportunity to verify that each object is
    //  readable and it provides a natural compaction.
    //
    //  This will also force any up-conversions of the data so that they only get done
    //  once here and not each time they are loaded (or until the next edit/save.) This
    //  is much more efficient.
    //
    //  And we update some data for some special case scenarios.
    //
    TPathStr pathSrc(m_pinfoToUse->strPath());
    pathSrc.AddLevel(L"CQCData\\Repository");
    TPathStr pathTar;
    if (TFileSys::bExists(pathSrc, L"CIDCfgSrvRepo.CIDObjStore", tCIDLib::EDirSearchFlags::NormalFiles))
    {
        pathTar = m_pinfoToUse->strTmpPath();
        pathTar.AddLevel(L"CQCData\\Repository");

        // Open up the old repository
        TCIDObjStore oseOld;
        if (oseOld.bInitialize(pathSrc, L"CIDCfgSrvRepo"))
        {
            // It shouldn't have created a new repository
            facCQCInst.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_SrcRepoCreated
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TString(L"Config")
            );
        }

        // Do a validation of the old repository before we start
        oseOld.ValidateStore();

        // Create the new repository file. Delete any previous one if it's there.
        if (!TFileSys::bIsDirectory(pathTar))
        {
            TFileSys::MakePath(pathTar);
        }
         else
        {
            TPathStr pathTarRepo(pathTar);
            pathTarRepo.AddLevel(L"CIDCfgSrvRepo.CIDObjStore");
            if (TFileSys::bExists(pathTarRepo))
                TFileSys::DeleteFile(pathTarRepo);
        }

        TCIDObjStore oseNew;
        if (!oseNew.bInitialize(pathTar, L"CIDCfgSrvRepo"))
        {
            // It didn't create the new repository
            facCQCInst.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_TarRepoNotCreated
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
            );
        }

        //
        //  Set up some regular expressions that we'll use to watch for some of the keys
        //  we care about upgrading. Some aren't just simple prefix checks.
        //
        TRegEx regxOldIntfDesState(L"/CQC/Users/.*/CQCIntfDef/FrameState");
        TRegEx regxOldIntfDesCfg(L"/CQC/Users/.*/CQCIntfDef/CfgData");

        // GUI program config was moved to local files, so remove those
        const TString strOldUserCfgPref(L"/CQC/Users/");

        // And the above means that the old standalone modes are gone also
        const TString strOldSACfgPref(L"/CQC/SAIntfV/");

        //
        //  Let's iterate all of the items in the repository. When we find ones of
        //  interest, we'll stream them in and convert them to the latest format
        //  and write that to the new one. And we just ignore some of them if we want
        //  to remove them.
        //
        tCIDLib::TCard4     c4DataSz;
        tCIDLib::TCard4     c4Version;
        THeapBuf            mbufData;
        tCIDLib::TStrList   colKeys;

        oseOld.QueryAllKeys(colKeys);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < colKeys.c4ElemCount(); c4Index++)
        {
            TString strKey = colKeys[c4Index];

            // Read in the raw data for this key
            c4Version = 0;
            c4DataSz = 0;
            const tCIDLib::ELoadRes eRes = oseOld.eReadObjectDirect
            (
                strKey, c4Version, mbufData, c4DataSz, kCIDLib::True
            );

            // Shouldn't happen, but just in case...
            if (eRes != tCIDLib::ELoadRes::NewData)
            {
                facCQCInst.LogInstallMsg(L"No data for config repo key %(1)", strKey);
                continue;
            }

            // Do any upgrades that are required....

            //
            //  No binding name should end with a slash. There were some
            //  errors where this was happening, but now the cfg server
            //  checks for this, so we have to remove them. Note that we
            //  only see binding name here, not scopes, which are only
            //  implied by way of the key paths.
            //
            if (strKey.chLast() == kCIDLib::chForwardSlash)
                strKey.DeleteLast();

            // Write it back out to the new repository, if data wasn't zeroed
            if (c4DataSz)
                oseNew.AddObjectDirect(strKey, mbufData, c4DataSz, c4DataSz / 8);
        }

        // Close the repositories
        oseOld.Close();
        oseNew.Close();
    }


    // Replicate the HTMLRoot tree, which will have any of the user's HTML content
    pathSrc = m_pinfoToUse->strPath();
    pathSrc.AddLevel(L"CQCData\\HTMLRoot");
    if (TFileSys::bIsDirectory(pathSrc))
    {
        pathTar = m_pinfoToUse->strTmpPath();
        pathTar.AddLevel(L"CQCData\\HTMLRoot");
        facCQCInst.DupPath(pathSrc, pathTar, kCIDLib::True);
    }

    // Replicate ImgMaps directory if they have one
    pathSrc = m_pinfoToUse->strPath();
    pathSrc.AddLevel(L"CQCData\\ImgMaps");
    if (TFileSys::bIsDirectory(pathSrc))
    {
        pathTar = m_pinfoToUse->strTmpPath();
        pathTar.AddLevel(L"CQCData\\ImgMaps");
        facCQCInst.DupPath(pathSrc, pathTar, kCIDLib::True);
    }

    // Replicate the CML user file tree
    pathSrc = m_pinfoToUse->strPath();
    pathSrc.AddLevel(L"CQCData\\MacroFileRoot");
    if (TFileSys::bIsDirectory(pathSrc))
    {
        pathTar = m_pinfoToUse->strTmpPath();
        pathTar.AddLevel(L"CQCData\\MacroFileRoot");
        facCQCInst.DupPath(pathSrc, pathTar, kCIDLib::True);
    }

    facCQCInst.LogInstallMsg(L"Common data upgrade complete");
}


// If on the master server we do upgrades to the installation server's repo as needed
tCIDLib::TVoid TInstallThread::UpgradeInstSrvInfo()
{
    CIDAssert(m_pinfoToUse->viiNewInfo().bMasterServer(), L"Can't upgrade inst server repo, not on MS");

    facCQCInst.LogInstallMsg(L"Upgrading installation server data");

    // Open or create the installation server's private repo
    TPathStr pathRepo(m_pinfoToUse->strTmpPath());
    pathRepo.AddLevels(L"CQCData", L"Repository");
    TCIDObjStore oseNew;
    oseNew.bInitialize(pathRepo, L"InstSrvData");

    //
    //  We need to run through and copy forward all of the stuff in the old repo
    //  to the new one.
    //
    {
        // Open the old one as well
        TPathStr pathOldRepo(m_pinfoToUse->strPath());
        pathOldRepo.AddLevels(L"CQCData", L"Repository");
        TCIDObjStore oseOld;
        if (oseOld.bInitialize(pathOldRepo, L"InstSrvData"))
        {
            // Should not have created a new one
            facCQCInst.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_SrcRepoCreated
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TString(L"Installation")
            );
        }

        //
        //  Loop through, reading in from old and writing to new. We can upgrade
        //  any of them as required.
        //
        tCIDLib::TCard4     c4DataSz;
        tCIDLib::TCard4     c4Version;
        THeapBuf            mbufData;
        tCIDLib::TStrList   colKeys;

        oseOld.QueryAllKeys(colKeys);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < colKeys.c4ElemCount(); c4Index++)
        {
            TString strKey = colKeys[c4Index];

            // Read in the raw data for this key
            c4Version = 0;
            const tCIDLib::ELoadRes eRes = oseOld.eReadObjectDirect
            (
                strKey, c4Version, mbufData, c4DataSz, kCIDLib::True
            );

            // Shouldn't happen, but just in case...
            if (eRes != tCIDLib::ELoadRes::NewData)
            {
                facCQCInst.LogInstallMsg(L"No data for install repo key %(1)", strKey);
                continue;
            }

            // Do any upgrades that are required....

            // Write it back out to the new repository, if data wasn't zeroed
            if (c4DataSz)
                oseNew.AddObjectDirect(strKey, mbufData, c4DataSz, c4DataSz / 8);
        }

        oseOld.Close();
    }

    oseNew.Close();
}


// If on the master server we do upgrades to the logic server's repo as needed
tCIDLib::TVoid TInstallThread::UpgradeLogicSrvInfo()
{
    CIDAssert(m_pinfoToUse->viiNewInfo().bMasterServer(), L"Can't upgrade logic server repo, not on MS");

    facCQCInst.LogInstallMsg(L"Upgrading logic server data");

    // Open or create the installation server's private repo
    TPathStr pathRepo(m_pinfoToUse->strTmpPath());
    pathRepo.AddLevels(L"CQCData", L"Repository");
    TCIDObjStore oseNew;
    oseNew.bInitialize(pathRepo, L"LogicSrvCfg");

    //
    //  We need to run through and copy forward all of the stuff in the old repo
    //  to the new one.
    //
    {
        // Open the old one as well
        TPathStr pathOldRepo(m_pinfoToUse->strPath());
        pathOldRepo.AddLevels(L"CQCData", L"Repository");
        TCIDObjStore oseOld;
        if (oseOld.bInitialize(pathOldRepo, L"LogicSrvCfg"))
        {
            // Should not have created a new one
            facCQCInst.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_SrcRepoCreated
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TString(L"Logic")
            );
        }

        //
        //  Loop through, reading in from old and writing to new. We can upgrade
        //  any of them as required.
        //
        tCIDLib::TCard4     c4DataSz;
        tCIDLib::TCard4     c4Version;
        THeapBuf            mbufData;
        tCIDLib::TStrList   colKeys;

        oseOld.QueryAllKeys(colKeys);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < colKeys.c4ElemCount(); c4Index++)
        {
            TString strKey = colKeys[c4Index];

            // Read in the raw data for this key
            c4Version = 0;
            const tCIDLib::ELoadRes eRes = oseOld.eReadObjectDirect
            (
                strKey, c4Version, mbufData, c4DataSz, kCIDLib::True
            );

            // Shouldn't happen, but just in case...
            if (eRes != tCIDLib::ELoadRes::NewData)
            {
                facCQCInst.LogInstallMsg(L"No data for logic server repo key %(1)", strKey);
                continue;
            }

            // Do any upgrades that are required....

            // Write it back out to the new repository, if data wasn't zeroed
            if (c4DataSz)
                oseNew.AddObjectDirect(strKey, mbufData, c4DataSz, c4DataSz / 8);
        }

        oseOld.Close();
    }

    oseNew.Close();
}


//
//  This method is called to do any upgrading of data. It just calls a set
//  of helper methods, which of which is responsible for upgrading a particular
//  type of data, based on whether we are installing that component in the
//  new install.
//
tCIDLib::TVoid TInstallThread::UpgradeData()
{
    SetStep(tCQCInst::EInstSteps::UpgradeData);

    //
    //  We always copy over some stuff that are present no matter what components are
    //  installed.
    //
    UpdateCommonData();

    //
    //  We know that we've created any needed directories already, so we don't have to deal
    //  with any of that during this stuff.
    //
    //  Keep in mind that the stuff above created the new repo, so it's already in the
    //  tmp directory location. We have to work on THAT one!
    //
    if (m_pinfoToUse->viiNewInfo().bMasterServer())
        UpgradeMasterData();

    //
    //  If doing the any clients, then copy over the files that are relevant
    //  to the client.
    //
    if (m_pinfoToUse->viiNewInfo().bAnyClient())
        UpgradeClientData();

    //
    //  If doing the CQCServer, then copy over the files that are relevant
    //  to the server.
    //
    if (m_pinfoToUse->viiNewInfo().bCQCServer())
        UpgradeServerData();

    // If doing either client or server, then copy over the shared data directory.
    if (m_pinfoToUse->viiNewInfo().bCQCServer()
    ||  m_pinfoToUse->viiNewInfo().bCQCClient())
    {
        UpgradeSharedData();
    }

    // And post that we are done with the upgrade
    facCQCInst.LogInstallMsg(L"Data upgrade is complete");
}



//
//  We have a special case here for crossing the 5.3.903 boundary. At that point, we
//  changed to the new chunked file storage format for all of the data server files.
//  So in that case, we have to convert, not just copy.
//
//  If we are already in chunked format land, then we either just copy over trees
//  for each data type, or in some cases we use an update method which will read
//  in the data and write it back out, in order to pre-emptively do any data updates
//  and avoid having it done every time the data is read.
//
tCIDLib::TVoid TInstallThread::UpgradeMasterData()
{
    facCQCInst.LogInstallMsg(L"Upgrading Master Server data");

    // Get the old version info so we can handle some special cases
    const TCQCVerInstallInfo& viiOld = m_pinfoToUse->viiOldInfo();

    // Build the base src/target paths
    const TPathStr pathSrc(m_pinfoToUse->strPath(), L"CQCData\\DataServer");
    const TPathStr pathTar(m_pinfoToUse->strTmpPath(), L"CQCData\\DataServer");

    // Some temps we pass into the conversion to chunked format
    TCQCIntfTemplate    iwdgTmp;
    TChunkedFile        chflNew;

    //
    //  For macros, we only copy over the User part of the namespace, since the System
    //  part we installed a new version of. This also needs to be recursive.
    //
    facCQCInst.LogInstallMsg(L"Upgrading macros");
    TPathStr pathTmpSrc(pathSrc);
    pathTmpSrc.AddLevel(L"Macros\\User");
    TPathStr pathTmpTar(pathTar);
    pathTmpTar.AddLevel(L"Macros\\User");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::True);

    //
    //  Same for protocols. It's a flat directory in this case, and not stored in
    //  chunked format yet.
    //
    facCQCInst.LogInstallMsg(L"Upgrading PDL files");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"Protocols\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"Protocols\\User");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::False);

    //
    //  Do the gloal actions.
    //
    facCQCInst.LogInstallMsg(L"Upgrading global actions");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"GlobalActs\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"GlobalActs\\User");
    UpdateFiles(pathTmpSrc, pathTmpTar, tCQCRemBrws::EDTypes::GlobalAct, chflNew);

    //
    //  Open up the new repo in the temp directory. Any stuff below that needs to get
    //  to repo base data can use this. It returns true if it had to create a new one,
    //  which shouldn't be the case.
    //
    pathTmpTar = m_pinfoToUse->strTmpPath();
    pathTmpTar.AddLevel(L"CQCData\\Repository");
    TCIDObjStore oseNew;
    if (oseNew.bInitialize(pathTmpTar, L"CIDCfgSrvRepo"))
    {
        facCQCInst.ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCQCInstErrs::errcFail_NoNewRepo
            , pathTmpTar
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Internal
        );
    }


    //
    //  Go through all the events, read them in, and write them back out to their
    //  new locations. This also updates them all.
    //
    facCQCInst.LogInstallMsg(L"Upgrading event monitors");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"\\Events\\Monitors\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"\\Events\\Monitors\\User");
    UpdateFiles(pathTmpSrc, pathTmpTar, tCQCRemBrws::EDTypes::EvMonitor, chflNew);

    facCQCInst.LogInstallMsg(L"Upgrading scheduled events");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"\\Events\\Scheduled\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"\\Events\\Scheduled\\User");
    UpdateFiles(pathTmpSrc, pathTmpTar, tCQCRemBrws::EDTypes::SchEvent, chflNew);

    facCQCInst.LogInstallMsg(L"Upgrading triggered events");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"\\Events\\Triggered\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"\\Events\\Triggered\\User");
    UpdateFiles(pathTmpSrc, pathTmpTar, tCQCRemBrws::EDTypes::TrgEvent, chflNew);

    //  Close the main object store now since we are done with it
    oseNew.Close();

    //
    //  Dup the templates. We need to put the interfce engine into designer
    //  mode here, else it will create default ids for all of the widgets that
    //  don't have explicit ones.
    //
    facCQCInst.LogInstallMsg(L"Upgrading user interfaces");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"Interfaces\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"Interfaces\\User");

    facCQCIntfEng().bDesMode(kCIDLib::True);
    try
    {
        UpdateFiles(pathTmpSrc, pathTmpTar, tCQCRemBrws::EDTypes::Template, chflNew);
        facCQCIntfEng().bDesMode(kCIDLib::False);
    }

    catch(TError& errToCatch)
    {
        facCQCIntfEng().bDesMode(kCIDLib::False);
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        throw;
    }

    //
    //  Dup the whole image \User tree.
    //
    pathTmpSrc = pathSrc;
    pathTmpTar = pathTar;

    facCQCInst.LogInstallMsg(L"Upgrading images");
    pathTmpSrc.AddLevel(L"Images\\User");
    pathTmpTar.AddLevel(L"Images\\User");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::True);

    //
    //  Dup the whole \System\ImgPacks\ tree. Although these are affected by the change
    //  in 5.3.903 to the chunked file format, the package import code can handle the
    //  old ones, so no need to upgrade them for now.
    //
    facCQCInst.LogInstallMsg(L"Copying image packs");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"Images\\System\\ImgPacks");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"Images\\System\\ImgPacks");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::True);

    // Dup the IR models directory tree
    facCQCInst.LogInstallMsg(L"Copying user IR models");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"IRModels");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"IRModels");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::True, kCIDLib::True);

    // Copy any user manifests over
    facCQCInst.LogInstallMsg(L"Upgrading user manifests");
    pathTmpSrc = pathSrc;
    pathTmpSrc.AddLevel(L"Manifests\\User");
    pathTmpTar = pathTar;
    pathTmpTar.AddLevel(L"Manifests\\User");
    facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::False, kCIDLib::False);

    // Replicate SysCfg directory if they have one
    facCQCInst.LogInstallMsg(L"Copying room config");
    pathTmpSrc = m_pinfoToUse->strPath();
    pathTmpSrc.AddLevel(L"CQCData\\SysCfg");
    if (TFileSys::bIsDirectory(pathTmpSrc))
    {
        pathTmpTar = m_pinfoToUse->strTmpPath();
        pathTmpTar.AddLevel(L"CQCData\\SysCfg");
        facCQCInst.DupPath(pathTmpSrc, pathTmpTar, kCIDLib::True);
    }

    //  Upgrade the installation, logic server, and security repos
    UpgradeInstSrvInfo();
    UpgradeLogicSrvInfo();
    UpgradeSecSrvInfo();

    facCQCInst.LogInstallMsg(L"Master server data upgrade complete");
}


//
//  If on the master server we do upgrades to the security server's repo as needed, the
//  one in the temp directory of course. This is called even on a clean install because
//  we have to generate some required security info if it's not present. That will be
//  the case if moving up from Pre-OSS version or on a clean install.
//
tCIDLib::TVoid TInstallThread::UpgradeSecSrvInfo()
{
    CIDAssert(m_pinfoToUse->viiNewInfo().bMasterServer(), L"Can't upgrade sec server repo, not on MS");
    facCQCInst.LogInstallMsg(L"Upgrading security server data");

    // Open or create the security server's private repo
    TPathStr pathRepo(m_pinfoToUse->strTmpPath());
    pathRepo.AddLevels(L"CQCData", L"Repository");
    TCIDObjStore oseNew;
    oseNew.bInitialize(pathRepo, L"SecuritySrvData");

    //
    //  We need to run through the old repo and copy everything forward to
    //  the new one, if this is not a clean install.
    //
    tCIDLib::TCard4     c4DataSz;
    tCIDLib::TCard4     c4Version;
    THeapBuf            mbufData;
    if (m_pinfoToUse->eTargetType() != tCQCInst::ETargetTypes::Clean)
    {
        TPathStr pathOldRepo(m_pinfoToUse->strPath());
        pathOldRepo.AddLevels(L"CQCData", L"Repository");
        TCIDObjStore oseOld;
        if (oseOld.bInitialize(pathOldRepo, L"SecuritySrvData"))
        {
            // Should not have created a new one
            facCQCInst.ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kCQCInstErrs::errcFail_SrcRepoCreated
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Internal
                , TString(L"Security")
            );
        }

        //
        //  Loop through, reading in from old and writing to new. We can upgrade
        //  any of them as required.
        //
        tCIDLib::TStrList   colKeys;
        oseOld.QueryAllKeys(colKeys);
        for (tCIDLib::TCard4 c4Index = 0; c4Index < colKeys.c4ElemCount(); c4Index++)
        {
            TString strKey = colKeys[c4Index];

            // Read in the raw data for this key
            c4Version = 0;
            c4DataSz = 0;
            const tCIDLib::ELoadRes eRes = oseOld.eReadObjectDirect
            (
                strKey, c4Version, mbufData, c4DataSz, kCIDLib::True
            );

            // Shouldn't happen, but just in case...
            if (eRes != tCIDLib::ELoadRes::NewData)
            {
                facCQCInst.LogInstallMsg(L"No data for install repo key %(1)", strKey);
                continue;
            }

            // Do any upgrades that are required....

            // Write it back out to the new repository, if data wasn't zeroed
            if (c4DataSz)
                oseNew.AddObjectDirect(strKey, mbufData, c4DataSz, c4DataSz / 8);
        }

        // We can close the old one now
        oseOld.Close();
    }

    //
    //  Now check that stuff that we need in the new one exists.
    //
    //  We need to make sure a server account exists for secondary servers to use.
    //  If not we need to create one. If we are upgrading, then we regenerate a new
    //  one since all other machines must be updated anyway and will get the new one.
    //
    {
        TCQCSecurityData secdUpdate;
        {
            c4Version = 0;
            const tCIDLib::ELoadRes eLoadRes = oseNew.eReadObject
            (
                kCQCKit::pszSecKey_UserAccountInfo, c4Version, secdUpdate, kCIDLib::False
            );

            // If there is none yet, then add a default admin account
            if (eLoadRes == tCIDLib::ELoadRes::NotFound)
            {
                secdUpdate.AddUserAccount
                (
                    tCQCKit::EUserRoles::SystemAdmin
                    , L"Default system admin account"
                    , L"Admin"
                    , L"Welcome"
                    , L"System"
                    , L"Administrator"
                );
            }
        }

        //
        //  If not just a refresh, remove any previous one. This will also correct if
        //  we manage to get more than one in the list. We'll now need to create one.
        //
        if (m_pinfoToUse->eTargetType() != tCQCInst::ETargetTypes::Refresh)
            secdUpdate.bRemoveAllOfType(tCQCKit::EUserTypes::SecServer);

        // Now if we don't have one, we need to create one
        m_strSrvSecName.Clear();
        m_strSrvSecPW.Clear();
        tCQCKit::TUserAcctList colList;
        if (secdUpdate.bFindAccountsOfType(tCQCKit::EUserTypes::SecServer, colList))
        {
            if (colList.c4ElemCount() > 1)
            {
                // Shouldn't happen, remove them all and we'll create a new one below
                secdUpdate.bRemoveAllOfType(tCQCKit::EUserTypes::SecServer);
            }
            else
            {
                m_strSrvSecName = colList[0].strLoginName();
                m_strSrvSecPW = colList[0].strDescription();
            }
        }

        if (m_strSrvSecName.bIsEmpty())
        {
            //
            //  We set the password in the description field as well, since we
            //  need to have it available to provide to the installer on non-MS
            //  machines.
            //
            m_strSrvSecName = TUniqueId::strMakeId();
            m_strSrvSecPW = TUniqueId::strMakeId();
            TCQCUserAccount uaccServer
            (
                tCQCKit::EUserRoles::PowerUser
                , m_strSrvSecPW
                , m_strSrvSecName
                , m_strSrvSecPW
                , L"Secondary"
                , L"Server"
            );
            uaccServer.eType(tCQCKit::EUserTypes::SecServer);
            secdUpdate.AddUserAccount(uaccServer);
        }

        // Now let's write it back out
        oseNew.bAddOrUpdate(kCQCKit::pszSecKey_UserAccountInfo, c4Version, secdUpdate, 1024);
    }

    //
    //  We have a set of key hashes that we need to insure exist. Or, even if they do exist
    //  if this is more than just a refresh or revision update, we create new ones so that
    //  they are not kept around too long. Clients have to upgrade for major/minor upgrades
    //  so they will pick up the new ones.
    //
    if (!oseNew.bKeyExists(kCQCKit::pszSecKey_MasterHash)
    || (m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Upgrade))
    {
        TMD5Hash mhashKey = TUniqueId::mhashMakeId();
        c4Version = 0;
        oseNew.bAddOrUpdate(kCQCKit::pszSecKey_MasterHash, c4Version, mhashKey);;
    }

    // If there's not an event key hash, add one
    if (!oseNew.bKeyExists(kCQCKit::pszSecKey_EventKey)
    || (m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Upgrade))
    {
        TMD5Hash mhashKey = TUniqueId::mhashMakeId();
        c4Version = 0;
        oseNew.bAddOrUpdate(kCQCKit::pszSecKey_EventKey, c4Version, mhashKey);
    }

    // If there's not an event system id hash, add one
    if (!oseNew.bKeyExists(kCQCKit::pszSecKey_EventSysId)
    || (m_pinfoToUse->eTargetType() == tCQCInst::ETargetTypes::Upgrade))
    {
        TMD5Hash mhashKey = TUniqueId::mhashMakeId();
        c4Version = 0;
        oseNew.bAddOrUpdate(kCQCKit::pszSecKey_EventSysId, c4Version, mhashKey);
    }

    oseNew.Close();
}


// Upgrades any server side data directory stuff
tCIDLib::TVoid TInstallThread::UpgradeServerData()
{
    facCQCInst.LogInstallMsg(L"Upgrading CQCServer data");

    //
    //  Copy over the server side data directory. This is for server side drivers and
    //  programs to store data if they need to. So we just replicate the directory
    //  recursively.
    //
    //  Server drivers are not copied. They will just be re-downloaded again as required.
    //
    TPathStr pathSrc(m_pinfoToUse->strPath());
    pathSrc.AddLevel(L"CQCData\\Server\\Data");
    TPathStr pathTar(m_pinfoToUse->strTmpPath());
    pathTar.AddLevel(L"CQCData\\Server\\Data");
    facCQCInst.DupPath(pathSrc, pathTar, kCIDLib::True);

    facCQCInst.LogInstallMsg(L"Server data upgrade complete");
}



tCIDLib::TVoid TInstallThread::UpgradeSharedData()
{
    facCQCInst.LogInstallMsg(L"Upgrading client/server shared data");

}


//
//  If we don't have to do any special conversions, this is called for our data server
//  files, to both copy them from the existing image to the temp new one, and to read
//  in and write back out the data for some of them, to make sure that they do any
//  data upgrading proactively.
//
tCIDLib::TVoid
TInstallThread::UpdateFiles(const   TString&                strSrc
                            , const TString&                strTarget
                            , const tCQCRemBrws::EDTypes    eType
                            ,       TChunkedFile&           chflTmp)
{
    TDirIter    diterSrc;
    TFindBuf    fndbCur;
    TPathStr    pathCurTarget;
    TPathStr    pathSrcSearch;

    // If the target doesn't exist, then create it
    if (!TFileSys::bExists(strTarget))
        TFileSys::MakePath(strTarget);

    // Recurse first
    {
        pathSrcSearch = strSrc;
        pathSrcSearch.AddLevel(kCIDLib::pszAllDirsSpec);
        if (diterSrc.bFindFirst(pathSrcSearch, fndbCur, tCIDLib::EDirSearchFlags::NormalDirs))
        {
            do
            {
                // Build up the equiv dir name in the target directory
                pathCurTarget = fndbCur.pathFileName();
                pathCurTarget.bRemovePath();
                pathCurTarget.AddToBasePath(strTarget);

                // And recurse with the new source and target
                UpdateFiles(fndbCur.pathFileName(), pathCurTarget, eType, chflTmp);

            }   while (diterSrc.bFindNext(fndbCur));
        }
    }

    //
    //  And do any files in this directory, we append the suffix for the current
    //  file type.
    //
    pathSrcSearch = strSrc;
    pathSrcSearch.AddLevel(L"*");
    pathSrcSearch.AppendExt(facCQCRemBrws().strFlTypeExt(eType));

    TPathStr pathOut;
    TPathStr pathTmp;
    TCQCSchEvent csrcSched;
    TCQCTrgEvent csrcTrig;
    TCQCEvMonCfg emoncCur;
    TCQCStdCmdSrc csrcGAct;
    TCQCIntfTemplate iwdgTmp;

    // Now lets iterate the files
    if (diterSrc.bFindFirst(pathSrcSearch, fndbCur, tCIDLib::EDirSearchFlags::AllFiles))
    {
        TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
        do
        {
            // Build up the equiv file name in the target directory
            pathCurTarget = fndbCur.pathFileName();
            pathCurTarget.bRemovePath();
            pathCurTarget.AddToBasePath(strTarget);

            // It should not exist!
            CIDAssert(!TFileSys::bExists(pathCurTarget), L"Target file should not exist");

            try
            {
                // Read in the chunked file
                {
                    TBinFileInStream strmIn
                    (
                        fndbCur.pathFileName()
                        , tCIDLib::ECreateActs::OpenIfExists
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );
                    strmIn >> chflTmp;
                }

                // Now we need to set up a binary input stream on its main data chunk
                try
                {
                    TBinMBufInStream strmIn(&chflTmp.mbufData(), chflTmp.c4DataBytes());
                    TBinMBufOutStream strmOut(chflTmp.c4DataBytes() + 1024);

                    // Depending on the type of file, read it in and write it back out
                    if (eType == tCQCRemBrws::EDTypes::SchEvent)
                    {
                        strmIn >> csrcSched;
                        strmOut << csrcSched;
                    }
                    else if (eType == tCQCRemBrws::EDTypes::TrgEvent)
                    {
                        strmIn >> csrcTrig;
                        strmOut << csrcTrig;
                    }
                    else if (eType == tCQCRemBrws::EDTypes::EvMonitor)
                    {
                        strmIn >> emoncCur;
                        strmOut << emoncCur;
                    }
                    else if (eType == tCQCRemBrws::EDTypes::GlobalAct)
                    {
                        strmIn >> csrcGAct;
                        strmOut << csrcGAct;
                    }
                    else if (eType == tCQCRemBrws::EDTypes::Template)
                    {
                        strmIn >> iwdgTmp;
                        strmOut << iwdgTmp;
                    }
                    else
                    {
                        CIDAssert2(L"Unknown file update type");
                    }

                    // Write it back to the data chunk
                    strmOut.Flush();
                    chflTmp.bSetDataChunk(strmOut.c4CurSize(), strmOut.mbufData());
                }

                catch(TError& errToCatch)
                {
                    facCQCInst.LogInstallMsg
                    (
                        TString(L"Could not read in file: ") + fndbCur.pathFileName()
                    );
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }


                // Now we just write it back out
                try
                {
                    fndbCur.pathFileName().bQueryName(pathTmp);
                    pathOut = strTarget;
                    pathOut.AddLevel(pathTmp);
                    pathOut.AppendExt(facCQCRemBrws().strFlTypeExt(eType));
                    TBinFileOutStream strmOut
                    (
                        pathOut
                        , tCIDLib::ECreateActs::CreateAlways
                        , tCIDLib::EFilePerms::Default
                        , tCIDLib::EFileFlags::SequentialScan
                    );

                    strmOut << chflTmp << kCIDLib::FlushIt;
                }

                catch(TError& errToCatch)
                {
                    facCQCInst.LogInstallMsg
                    (
                        TString(L"Could not write out file: ") + pathOut
                    );
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                throw;
            }

        }   while (diterSrc.bFindNext(fndbCur));
    }
}

