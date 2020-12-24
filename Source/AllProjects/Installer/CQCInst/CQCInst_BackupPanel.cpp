//
// FILE NAME: CQCInst_BackupPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/12/2010
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
//  This file implements the info panel that gives the user a chance to
//  backup the current install before continuing.
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
RTTIDecls(TInstBackupPanel,TInstInfoPanel)


// ---------------------------------------------------------------------------
//   CLASS: TInstBackupPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstBackupPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstBackupPanel::TInstBackupPanel( TCQCInstallInfo* const  pinfoCur
                                    , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Backup CQC Installation", pinfoCur, pwndParent)
    , m_eStatus(EResults::Idle)
    , m_pwndBrowse(nullptr)
    , m_pwndPercent(nullptr)
    , m_pwndDoBackup(nullptr)
    , m_pwndPath(nullptr)
    , m_thrBackup
      (
        TString(L"CQCInstBackupThread")
        , TMemberFunc<TInstBackupPanel>(this, &TInstBackupPanel::eBackupThread)
      )
{
}

TInstBackupPanel::~TInstBackupPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstBackupPanel: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstBackupPanel::bPanelIsVisible() const
{
    // We are only visible if not doing a clean (new) install
    return (infoCur().eTargetType() != tCQCInst::ETargetTypes::Clean);
}


tCIDLib::TVoid TInstBackupPanel::SaveContents()
{
    //
    //  Get the path text out and store it. We never want it to end in a
    //  separator, so remove any trailing separators from it before storing
    //  it.
    //
    TString strTmp(m_pwndPath->strWndText());
    strTmp.Strip(L"\\", tCIDLib::EStripModes::Trailing);
    infoCur().viiNewInfo().strBackupDir(strTmp);
}


// ---------------------------------------------------------------------------
//  TInstBackupPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstBackupPanel::bCreated()
{
    TParent::bCreated();

    // Set our text for use by any popups
    strWndText(facCQCInst.strMsg(kCQCInstMsgs::midPan_Backup_Title));

    // Load up our child controls
    LoadDlgItems(kCQCInst::ridPan_Backup);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_Backup_Browse, m_pwndBrowse);
    CastChildWnd(*this, kCQCInst::ridPan_Backup_DoBackup, m_pwndDoBackup);
    CastChildWnd(*this, kCQCInst::ridPan_Backup_Path, m_pwndPath);
    CastChildWnd(*this, kCQCInst::ridPan_Backup_Percent, m_pwndPercent);

    // Register a click handler on the main frame for our browse button
    m_pwndDoBackup->pnothRegisterHandler(this, &TInstBackupPanel::eClickHandler);
    m_pwndBrowse->pnothRegisterHandler(this, &TInstBackupPanel::eClickHandler);

    // Load up the path text
    m_pwndPath->strWndText(infoCur().viiNewInfo().strBackupDir());

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstBackupPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If we get here, then the user has entered something in the target
//  directory. So we need to validate it, create if it doesn't exist and
//  the user says we can. If all that works, we kick off the background
//  thread and do a message yielding loop until it completes, displaying
//  status info in the process.
//
tCIDLib::TBoolean TInstBackupPanel::bDoBackup()
{
    // Get a local ref to the install info
    const TCQCInstallInfo& infoBackup = infoCur();

    // Get the contents of the path entry field into the target path member
    const TPathStr pathTopTar = m_pwndPath->strWndText();

    //
    //  Make sure the path exists or can be created. If it exists, make
    //  sure it's not the directory of the installer itself.
    //
    if (!TFileSys::bExists(pathTopTar, tCIDLib::EDirSearchFlags::NormalDirs))
    {
        // See if they want to create it
        TYesNoBox msgbAsk(facCQCInst.strMsg(kCQCInstMsgs::midQ_CreatePath, pathTopTar));

        // If not, we are done
        if (!msgbAsk.bShowIt(*this))
            return kCIDLib::False;

        // They want to so, so create this top level directory
        try
        {
            TFileSys::MakePath(pathTopTar);
        }

        catch(TError& errToCatch)
        {
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                facCQCInst.LogEventObj(errToCatch);
            }

            TErrBox msgbFail(facCQCInst.strMsg(kCQCInstErrs::errcVal_CantMakePath, pathTopTar));
            msgbFail.ShowIt(*this);
            return kCIDLib::False;
        }
    }

    // Set up the name of the initial temporary target directory
    const TString strTmpDir(L"CQCNewBackup");
    m_pathTarDir = pathTopTar;
    m_pathTarDir.AddLevel(strTmpDir);


    //
    //  Now create the final tagged sub-directory that we'll put the backup
    //  in. It is made up of the current CQC version we are backing up and
    //  the date stamp.
    //
    TString strSubDir(L"CQCBackup-");

    // Put the time stamp in, reverse order
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    tCIDLib::TCard4         c4Year;
    tCIDLib::EMonths        eMonth;
    tCIDLib::TCard4         c4Day;
    tCIDLib::TCard4         c4Hour;
    tCIDLib::TCard4         c4Minute;
    tCIDLib::TCard4         c4Second;
    tCIDLib::TCard4         c4Millis;
    tCIDLib::TEncodedTime   enctStamp;
    tmCur.eExpandDetails
    (
        c4Year, eMonth, c4Day, c4Hour, c4Minute, c4Second, c4Millis, enctStamp
    );

    strSubDir.AppendFormatted(c4Year);
    const tCIDLib::TCard4 c4Month = tCIDLib::TCard4(eMonth) + 1;
    if (c4Month < 10)
        strSubDir.Append(kCIDLib::chDigit0);
    strSubDir.AppendFormatted(c4Month);
    if (c4Day< 10)
        strSubDir.Append(kCIDLib::chDigit0);
    strSubDir.AppendFormatted(c4Day);

    strSubDir.Append(L'_');
    if (c4Hour < 10)
        strSubDir.Append(kCIDLib::chDigit0);
    strSubDir.AppendFormatted(c4Hour);
    if (c4Minute < 10)
        strSubDir.Append(kCIDLib::chDigit0);
    strSubDir.AppendFormatted(c4Minute);
    if (c4Second < 10)
        strSubDir.Append(kCIDLib::chDigit0);
    strSubDir.AppendFormatted(c4Second);

    // Do the version, replacing the dots with underscores
    strSubDir.Append(L'-');
    TString strCQCVer;
    facCQCKit().FormatVersionStr(strCQCVer, infoBackup.viiOldInfo().c8Version());
    strCQCVer.bReplaceChar(kCIDLib::chPeriod, kCIDLib::chUnderscore);
    strSubDir.Append(strCQCVer);

    m_pathFinalTarDir = pathTopTar;
    m_pathFinalTarDir.AddLevel(strSubDir);

    // This final target should not already exist
    if (TFileSys::bExists(m_pathFinalTarDir))
    {
        TErrBox msgbFail(facCQCInst.strMsg(kCQCInstErrs::errcFail_BackupTarExists));
        msgbFail.ShowIt(*this);
        return kCIDLib::False;
    }

    //
    //  Set up the source directory as well so the bgn thread can know
    //  where to copy from.
    //
    m_pathSrcDir = infoBackup.strOldPath();

    //
    //  Disable the backup while we are in here, and the navigation buttons
    //  on the main installer window, so that they can't try to do anything
    //  until we finish this.
    //
    m_pwndDoBackup->SetEnable(kCIDLib::False);
    wndParent().SetNavFlags(tCQCInst::ENavFlags::None);

    // And now let's try to do it
    try
    {
        //
        //  Set the waiting status, whcih will make the time start updating the
        //  progress bar. And zero the percent done indicator.
        //
        m_eStatus = EResults::Waiting;
        m_c4Percent = 0;
        m_pwndPercent->SetValue(0UL);

        // And start the runner thread that does the work
        m_thrBackup.Start();

        // Now we have to wait until the thread completes
        tCIDLib::TCard4 c4LastPercent = 0;
        while (m_eStatus == EResults::Waiting)
        {
            facCIDCtrls().MsgYield(250);

            // Update the percentage of it's changed
            if (m_c4Percent != c4LastPercent)
            {
                c4LastPercent = m_c4Percent;
                m_pwndPercent->SetValue(c4LastPercent);
            }
        }
    }

    catch(TError& errToCatch)
    {
        facCQCInst.LogInstallMsg(L"Exception starting backup process");
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Make sure that the percentage goes to 100% or back to zero depending on status.
    if (m_eStatus == EResults::Done)
        m_pwndPercent->SetValue(100UL);
    else
        m_pwndPercent->SetValue(0UL);

    // Wait for the thread to completely shutdown
    try
    {
        if (m_thrBackup.bIsRunning())
            m_thrBackup.ReqShutdownSync();
        m_thrBackup.eWaitForDeath(10000);
    }

    catch(TError& errToCatch)
    {
        facCQCInst.LogInstallMsg(L"Exception stopping backup thread");
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }

    // Re-enable the buttons on the parent and the backup button
    m_pwndDoBackup->SetEnable(kCIDLib::True);
    wndParent().SetNavFlags(tCQCInst::ENavFlags::AllowAll);

    //
    //  If it failed, tell the user and return false. If it worked, then
    //  do the final rename to the target directory.
    //
    if (m_eStatus != EResults::Done)
    {
        facCQCInst.LogInstallMsg(L"The backup operation did not complete");

        // Get the log file path and remove the file name part
        TPathStr pathLogs(facCQCInst.strLogPath());
        pathLogs.bRemoveNameExt();

        TErrBox msgbFailed(facCQCInst.strMsg(kCQCInstErrs::errcFail_BackupError, pathLogs));
        msgbFailed.ShowIt(*this);
        return kCIDLib::False;
    }

    // It appears to have worked so return success
    return kCIDLib::True;
}


//
//  The backup thread object is started up on this method. We do the actual
//  backup work of recursively replicating the CQCData and Bin directories.
//
tCIDLib::EExitCodes
TInstBackupPanel::eBackupThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go
    thrThis.Sync();

    const TString strDataDir(L"CQCData");
    const TString strBinDir(L"Bin");

    //
    //  Try to create the temp subdirectory and the two major subdirs. If
    //  the temp subdirectory already exists, then we remove the previous
    //  contents. It might be there if we weren't able to remove a previous
    //  failed backup. If it doesn't exist, create it.
    //
    try
    {
        if (TFileSys::bExists(m_pathTarDir, tCIDLib::EDirSearchFlags::NormalDirs))
            TFileSys::CleanPath(m_pathTarDir, tCIDLib::ETreeDelModes::Remove);
        else
            TFileSys::MakeDirectory(m_pathTarDir);

        //
        //  Now create the top level CQC directories under this. These
        //  are the directories we'll replicate into.
        //
        TFileSys::MakeSubDirectory(strDataDir, m_pathTarDir);
        TFileSys::MakeSubDirectory(strBinDir, m_pathTarDir);
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogEventObj(errToCatch);
        }

        facCQCInst.LogInstallMsg(L"Could not create/clean out temp backup dir");
        m_eStatus = EResults::DoneWithError;
        return tCIDLib::EExitCodes::InitFailed;
    }

    try
    {
        //
        //  First we need to do a recursive scan of the source directory
        //  and build up the list of files we have to copy. We do two
        //  different trees, the CQCData and Bin directories. Set an
        //  initial allocation of 3000 files which is a good bit more than
        //  we currently need, so there's room to grow without it having
        //  to reallocate the vector during this scan any time soon.
        //
        //  Note that it only stores the part relative to the base
        //  directory. This way we can efficiently create both the source
        //  and target directories from what's stored in the list.
        //
        TPathStr pathSrc;
        TPathStr pathTar;
        TVector<TString> colFiles(3000);
        {
            pathSrc = m_pathSrcDir;
            pathSrc.AddLevel(strDataDir);
            LoadFiles(pathSrc, colFiles, kCIDLib::True, m_pathSrcDir.c4Length());

            pathSrc = m_pathSrcDir;
            pathSrc.AddLevel(strBinDir);
            LoadFiles(pathSrc, colFiles, kCIDLib::True, m_pathSrcDir.c4Length());

            // And the ones in the main directory, non-recursively
            pathSrc = m_pathSrcDir;
            LoadFiles(pathSrc, colFiles, kCIDLib::False, m_pathSrcDir.c4Length());
        }

        //
        //  Now loop through all of them and copy each one, verifying
        //  that it got backed up correctly. We keep the current
        //  percentage member up to date as we do this.
        //
        TFindBuf fndbSrc;
        TFindBuf fndbTar;

        const tCIDLib::TCard4 c4Count = colFiles.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TString& strCur = colFiles[c4Index];

            // Build up the two paths and do the copy
            pathSrc = m_pathSrcDir;
            pathSrc.AddLevel(strCur);
            pathTar = m_pathTarDir;
            pathTar.AddLevel(strCur);
            TFileSys::CopyFile(pathSrc, pathTar);

            //
            //  Validate that it got there and has the same size as
            //  and last write date as the source.
            //
            if (!TFileSys::bExists(pathSrc, fndbSrc)
            ||  !TFileSys::bExists(pathTar, fndbTar)
            ||  (fndbSrc.c8Size() != fndbTar.c8Size())
            ||  (fndbSrc.tmLastModify() != fndbSrc.tmLastModify()))
            {
                facCQCInst.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCInstErrs::errcFail_FileDiff
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , pathTar
                );
            }

            // Update the percentage done every twenty through
            if (!(c4Index % 20))
            {
                const tCIDLib::TFloat4 f4Per
                (
                    (tCIDLib::TFloat4(c4Index) / tCIDLib::TFloat4(c4Count)) * 100.0F
                );

                if (f4Per > 100)
                    m_c4Percent = 100;
                else
                    m_c4Percent = tCIDLib::TCard4(f4Per);
            }
        }

        //
        //  Now we need to rename the temp dir to the final. Pause a little bit first
        //  to hopefully give any anti-virus stuff a chance to get through messing around
        //  with our stuff, else the directory will be locked. The rename also retries,
        //  but sometimes that's not enough.
        //
        TThread::Sleep(4000);
        TFileSys::Rename(m_pathTarDir, m_pathFinalTarDir);

        //
        //  It worked so set the status to done and make sure the
        //  percent gets to 100.
        //
        m_c4Percent = 100;
        m_eStatus = EResults::Done;
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogEventObj(errToCatch);
        }
        facCQCInst.LogInstallMsg(L"Backup operation failed, see logs");
        m_eStatus = EResults::DoneWithError;
    }

    catch(...)
    {
        facCQCInst.LogInstallMsg(L"Unknown exception in backup thread");
        m_eStatus = EResults::DoneWithError;
    }
    return tCIDLib::EExitCodes::Normal;
}


tCIDCtrls::EEvResponses TInstBackupPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_Backup_Browse)
    {
        //
        //  Set up the directory select dialog title string. We use the
        //  same one as the target path selection, since it has the
        //  text we want to display as well.
        //
        TString strTitle(kCQCInstMsgs::midPan_Path_BrowseTarTitle, facCQCInst);

        //
        //  Get the current contents of the entry field. Check it to see
        //  its a legal directory. If so, then keep it and use it as the
        //  initial browse path. Else, toss it an clear the initial string.
        //
        TString strCurPath(m_pwndPath->strWndText());

        if (strCurPath.chLast() == L'\\')
            strCurPath.DeleteLast();
        if (!TFileSys::bIsDirectory(strCurPath))
            strCurPath.Clear();

        // No multi-select so we can only get one entry back
        tCIDLib::TKVPList colFileTypes;
        tCIDLib::TStrList colSel(1);
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , strTitle
            , strCurPath
            , colSel
            , colFileTypes
            , tCIDLib::eOREnumBits
              (
                tCIDCtrls::EFOpenOpts::SelectFolders
                , tCIDCtrls::EFOpenOpts::PathMustExist
                , tCIDCtrls::EFOpenOpts::FileSystemOnly
              )
        );

        if (bRes)
            m_pwndPath->strWndText(colSel[0]);
    }
     else if (wnotEvent.widSource() == kCQCInst::ridPan_Backup_DoBackup)
    {
        //
        //  The user wants to do a backup. If he's not selected a path yet,
        //  tell him he has to select one. Else, try the backup.
        //
        if (m_pwndPath->bIsEmpty())
        {
            TErrBox msgbNoPath(facCQCInst.strMsg(kCQCInstErrs::errcVal_NoBackupDir));
            msgbNoPath.ShowIt(*this);
        }
         else
        {
            //
            //  If the backup processed successfully, then store the backup directory in
            //  the install info for the next time around.
            //
            if (bDoBackup())
                infoCur().viiNewInfo().strBackupDir(m_pwndPath->strWndText());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This does a recursive or non-recursive scan of a directory and builds up a list of all
//  of the files in that tree. It stores the parts of the path relative to the original
//  base path length which is passed in so that we can easily only save the part after that.
//
//  We also create the matching target directories as we go.
//
tCIDLib::TVoid
TInstBackupPanel::LoadFiles(const   TPathStr&           pathParent
                            ,       TVector<TString>&   colFiles
                            , const tCIDLib::TBoolean   bRecurse
                            , const tCIDLib::TCard4     c4BasePathLen)
{
    //
    //  If it's the media cache directory, skip it. We don't want to copy that
    //  stuff to a backup. Just let it download again if they do a restore.
    //
    if (pathParent.bEndsWithI(L"Client\\Data\\MediaCache"))
        return;

    TDirIter    diterSrc;
    TFindBuf    fndbCur;
    TPathStr    pathCurTar;
    TString     strTmp;

    // If recursing, then first scan for subdirs
    if (bRecurse)
    {
        // Use the temp path to build up the search spec
        pathCurTar = pathParent;
        pathCurTar.AddLevel(kCIDLib::pszAllDirsSpec);
        if (diterSrc.bFindFirst(pathCurTar, fndbCur, tCIDLib::EDirSearchFlags::NormalDirs))
        {
            do
            {
                //
                //  Create the equivalent target directory. First take the
                //  common part of the directory we found (past the base src
                //  dir) and add it to the backup target path. This will be
                //  the same dir in the target.
                //
                strTmp = fndbCur.pathFileName();
                strTmp.Cut(0, c4BasePathLen);
                pathCurTar = m_pathTarDir;
                pathCurTar.AddLevel(strTmp);
                TFileSys::MakeDirectory(pathCurTar, kCIDLib::False);

                // And now recurse on the source
                LoadFiles(fndbCur.pathFileName(), colFiles, bRecurse, c4BasePathLen);

            }   while (diterSrc.bFindNext(fndbCur));
        }
    }

    // Now do any files at this level
    pathCurTar = pathParent;
    pathCurTar.AddLevel(kCIDLib::pszAllFilesSpec);
    if (diterSrc.bFindFirst(pathCurTar, fndbCur, tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            // Copy out the part past the base len and keep that
            fndbCur.pathFileName().CopyOutSubStr(pathCurTar, c4BasePathLen);
            colFiles.objAdd(pathCurTar);

        }   while (diterSrc.bFindNext(fndbCur));
    }
}

