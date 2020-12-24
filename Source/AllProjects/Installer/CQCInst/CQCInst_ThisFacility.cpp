//
// FILE NAME: CQCInst_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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
//  CQC installer.
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
RTTIDecls(TFacCQCInst,TFacility)


// ---------------------------------------------------------------------------
//   CLASS: TFacCQCInst
//  PREFIX: fac
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TFacCQCInst: Constructors and Destructor
// ---------------------------------------------------------------------------
TFacCQCInst::TFacCQCInst() :

    TGUIFacility
    (
        L"CQCInst"
        , tCIDLib::EModTypes::Exe
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    )
    , m_colWarnings(c4MaxErrors)
    , m_eStartMode(tCIDKernel::EWSrvStarts::Delayed)
{
}

TFacCQCInst::~TFacCQCInst()
{
}


// ---------------------------------------------------------------------------
//  TFacCQCInst: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TFacCQCInst::bCheckTarget(          TWindow&            wndParent
                            , const TString&            strTarget
                            ,       TCQCVerInstallInfo& viiToFill)
{
    // Log what we are doing
    TString strTmp(L"Checking install target '");
    strTmp.Append(strTarget);
    strTmp.Append(L"'");
    LogInstallMsg(strTmp);

    //
    //  First, see if there is an installation version file, which was added
    //  as of 0.8. That will tell us exactly what's there. Otherwise, it has
    //  to be pre-0.8 and we have to go by a DLL name, which only gives us
    //  the maj/min version.
    //
    TPathStr pathCheck(strTarget);
    pathCheck.AddLevel(kCQCInst::pszInstVerFile);

    //
    //  The version file does exist, so let's try to parse it. If it works,
    //  we are done. Else, we'll try to fall through and figure out the
    //  version info ourself if we can.
    //
    if (TFileSys::bExists(pathCheck))
    {
        strTmp = L"Version info file was found in '";
        strTmp.Append(strTarget);
        strTmp.Append(L"'");
        LogInstallMsg(strTmp);

        try
        {
            // Open a stream on this file and stream in the contents
            TBinFileInStream strmVer
            (
                pathCheck
                , tCIDLib::ECreateActs::OpenIfExists
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
            );
            strmVer >> viiToFill;

            // It worked, so let's try it
            LogInstallMsg(L"Successfully streamed in the version info");
            return kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            // Log the error and keep going
            if (!errToCatch.bLogged())
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
            LogInstallMsg(L"Failed to stream in the version info");
        }
    }
     else
    {
        strTmp = L"No version info file was found in '";
        strTmp.Append(strTarget);
        strTmp.Append(L"'");
        LogInstallMsg(strTmp);
    }

    // We didn't find any previous version info
    return kCIDLib::False;
}


// Let the warnings panel get access to any queued warnings
const TVector<TInstWarning>& TFacCQCInst::colWarnings() const
{
    return m_colWarnings;
}


//
//  Duplicates the contents of a directory, where the src and target path
//  are fully qualified. This is used both by the install thread and by the
//  backup panel if the user asks to do a backup before an upgrade.
//
//  It can handle recursion or just do a single directory level, and
//  optionally can stop if an overwrite would occur or just overwrite
//  any files as it goes.
//
tCIDLib::TVoid
TFacCQCInst::DupPath(const  TString&            strSrc
                    , const TString&            strTarget
                    , const tCIDLib::TBoolean   bRecurse
                    , const tCIDLib::TBoolean   bOverwrite)
{
    TDirIter    diterSrc;
    TFindBuf    fndbCur;
    TPathStr    pathCurTarget;

    // If the target doesn't exist, then create it
    if (!TFileSys::bExists(strTarget))
        TFileSys::MakePath(strTarget);

    //
    //  If we are to recurse on this one, then make a first pass looking
    //  just for subdirectories, and recurse on each one.
    //
    if (bRecurse)
    {
        // Use the temp path to build up the search spec
        pathCurTarget = strSrc;
        pathCurTarget.AddLevel(kCIDLib::pszAllDirsSpec);
        if (diterSrc.bFindFirst(pathCurTarget, fndbCur, tCIDLib::EDirSearchFlags::NormalDirs))
        {
            do
            {
                // Build up the equiv dir name in the target directory
                pathCurTarget = fndbCur.pathFileName();
                pathCurTarget.bRemovePath();
                pathCurTarget.AddToBasePath(strTarget);

                // And recurse with the new source and target
                DupPath(fndbCur.pathFileName(), pathCurTarget, bRecurse);

            }   while (diterSrc.bFindNext(fndbCur));
        }
    }

    // Use the temp path to build up the search spec
    pathCurTarget = strSrc;
    pathCurTarget.AddLevel(kCIDLib::pszAllFilesSpec);

    // Now lets iterate the source and copy each file
    TFindBuf fndbTest;
    if (diterSrc.bFindFirst(pathCurTarget, fndbCur, tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            // Build up the equiv file name in the target directory
            pathCurTarget = fndbCur.pathFileName();
            pathCurTarget.bRemovePath();
            pathCurTarget.AddToBasePath(strTarget);

            // If exists, don't overwrite unless told to
            if (TFileSys::bExists(pathCurTarget) && !bOverwrite)
                continue;

            TFileSys::CopyFile(fndbCur.pathFileName(), pathCurTarget);

            //
            //  Remove the read-only attribute from the target file. If we
            //  install from a CD, then the source will be read only,
            //  which will make the target read-only.
            //
            TFileSys::SetPerms(pathCurTarget, tCIDLib::EFilePerms::AllWE);

            //
            //  And validate that it got there and has the same size as
            //  and last write date as the source.
            //
            if (!TFileSys::bExists(pathCurTarget, fndbTest)
            ||  (fndbCur.c8Size() != fndbTest.c8Size())
            ||  (fndbCur.tmLastModify() != fndbTest.tmLastModify()))
            {
                facCQCInst.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kCQCInstErrs::errcFail_FileDiff
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , pathCurTarget
                );
            }
        }   while (diterSrc.bFindNext(fndbCur));
    }
}


//
//  The main program entry point.
//
tCIDLib::EExitCodes
TFacCQCInst::eMainThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let our caller go
    thrThis.Sync();

    // Make sure that the user is an admin, else get out
    if (!TSysInfo::bIsHostAdmin())
    {
        TErrBox msgbNotAdmin(facCQCInst.strMsg(kCQCInstErrs::errcFail_NotAnAdmin));
        msgbNotAdmin.ShowIt(TWindow::wndDesktop());
        return tCIDLib::EExitCodes::PermissionLevel;
    }

    // Do the standard init stuff
    if (!bInit())
        return tCIDLib::EExitCodes::RuntimeError;

    try
    {
        // Create the main frame window
        LogInstallMsg(L"Creating main frame window");
        m_pwndMainFrm = new TMainFrameWnd;
        if (!m_pwndMainFrm->bCreateMainWnd())
            return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(TError& errToCatch)
    {
        LogInstallMsg(L"Main frame window creation failed");

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgbFail
        (
            TWindow::wndDesktop()
            , strMsg(kCQCInstMsgs::midGen_Title1)
            , strMsg(kCQCInstErrs::errcFail_ExceptInInst)
            , errToCatch
        );
        return tCIDLib::EExitCodes::RuntimeError;
    }


    // Enter the main message loop and wait for the process to complete
    facCIDCtrls().uMainMsgLoop(*m_pwndMainFrm);

    try
    {
        // Log the number of file system retries we experienced
        LogInstallMsg(L"File system retries=%(1)", TCardinal(TKrnlFileSys::c4FSRetryCount()));

        // Close down the ORB, in case it got cranked up
        facCIDOrb().Terminate();

        //
        //  Hide our main window, to hide the shutdown activity, then call
        //  destroy on it.
        //
        LogInstallMsg(L"Destroying main frame window");
        if (m_pwndMainFrm)
        {
            m_pwndMainFrm->SetVisibility(kCIDLib::False);
            m_pwndMainFrm->Destroy();
        }
    }

    catch(TError& errToCatch)
    {
        if (bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            LogEventObj(errToCatch);
        }
        return tCIDLib::EExitCodes::RuntimeError;
    }

    catch(...)
    {
        // Log this fact and tell the user
        LogInstallMsg(L"An unknown exception occured");
        return tCIDLib::EExitCodes::RuntimeError;
    }
    return tCIDLib::EExitCodes::Normal;
}


// Get/set the service start mode
tCIDKernel::EWSrvStarts TFacCQCInst::eStartMode() const
{
    return m_eStartMode;
}

tCIDKernel::EWSrvStarts
TFacCQCInst::eStartMode(const tCIDKernel::EWSrvStarts eToSet)
{
    m_eStartMode = eToSet;
    return m_eStartMode;
}


// Let the panels and installer thread log messages
tCIDLib::TVoid TFacCQCInst::LogInstallMsg(const tCIDLib::TCh* const pszMsg)
{
    LogMsg(CID_FILE, CID_LINE, pszMsg, tCIDLib::ESeverities::Status);
}


tCIDLib::TVoid TFacCQCInst::LogInstallMsg(const TString& strMsg)
{
    LogMsg(CID_FILE, CID_LINE, strMsg, tCIDLib::ESeverities::Status);
}


tCIDLib::TVoid
TFacCQCInst::LogInstallMsg(const TString& strMsg, const MFormattable& fmtblToken1)
{
    LogMsg
    (
        CID_FILE
        , CID_LINE
        , strMsg
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
        , fmtblToken1
    );
}


// Helpers to queue up warnings to the warning queue
tCIDLib::TVoid TFacCQCInst::QueueWarning(const TString& strMsg)
{
    if (!m_colWarnings.bIsFull(c4MaxErrors))
        m_colWarnings.objAdd(TInstWarning(strMsg));
}

tCIDLib::TVoid
TFacCQCInst::QueueWarning(const TString& strMsg, const TError& errExcept)
{
    if (!m_colWarnings.bIsFull(c4MaxErrors))
        m_colWarnings.objAdd(TInstWarning(strMsg, errExcept));
}


tCIDLib::TVoid TFacCQCInst::QueueWarning(const TString& strMsg, const TString& strResource)
{
    if (!m_colWarnings.bIsFull(c4MaxErrors))
        m_colWarnings.objAdd(TInstWarning(strMsg, strResource));
}

tCIDLib::TVoid
TFacCQCInst::QueueWarning(  const   TString&    strMsg
                            , const TString&    strResource
                            , const TError&     errExcept)
{
    if (!m_colWarnings.bIsFull(c4MaxErrors))
        m_colWarnings.objAdd(TInstWarning(strMsg, strResource, errExcept));
}


const TString& TFacCQCInst::strLogPath() const
{
    return m_pathLogs;
}


// ---------------------------------------------------------------------------
//  TFacCQCInst: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  The main window doesn't exist yet, so we have to use a null frame window
//  for any popups.
//
tCIDLib::TBoolean TFacCQCInst::bInit()
{
    //
    //  Install a local file logger and turn on settings that give us maximum
    //  logging. If something goes awry during install, we want to capture
    //  maximum information.
    //
    //  Note that we have to put this stuff somewhere besides in our own
    //  startup directory, because we might be running from a CD or other
    //  unwriteable media. So we try to find the temp directory and use that.
    //  If that's not available, we look for a drive root directory of a
    //  fixed hard drive.
    //
    try
    {
        if (!TProcEnvironment::bFindTempPath(m_pathLogs))
        {
            TVector<TVolumeInfo>        colVols;
            TVector<TVolFailureInfo>    colErrs;
            TFileSys::c4QueryAvailableVolumes
            (
                colVols, colErrs, kCIDLib::False, kCIDLib::False
            );

            // Take the first one that is a fixed drive.
            const tCIDLib::TCard4 c4Count = colVols.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                if (colVols[c4Index].eHWType() == tCIDLib::EVolHWTypes::Fixed)
                {
                    m_pathLogs = colVols[0].strVolume();
                    break;
                }
            }

            m_pathLogs.bRemoveTrailingSeparator();
            if (m_pathLogs.bIsEmpty() || !TFileSys::bIsDirectory(m_pathLogs))
            {
                TErrBox msgbNoLogPath(facCQCInst.strMsg(kCQCInstErrs::errcFail_NoLogPath));
                msgbNoLogPath.ShowIt(TWindow::wndDesktop());
                return kCIDLib::False;
            }
        }

        // Set the stack dump path to this path
        TProcEnvironment::bAddOrUpdate(kCIDLib::pszErrDumpDir, m_pathLogs);

        //
        //  And install the local file logger on this path. Use UTF-8 so that
        //  we can get any language logged. We remove any previous one so that
        //  it only holds the output from this install.
        //
        m_pathLogs.AddLevel(L"CQCInstall.Txt");
        try
        {
            TFileSys::DeleteFile(m_pathLogs);
        }

        catch(const TError&)
        {
            // Just eat it, we'll just append
        }

        TModule::InstallLogger
        (
            new TTextFileLogger
            (
                m_pathLogs, tCIDLib::EAccessModes::Excl_OutStream, new TUTF8Converter
            )
            , tCIDLib::EAdoptOpts::Adopt
        );
        TFacility::eGlobalLogMode(tCIDLib::ESeverities::Info);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgbFail
        (
            TWindow::wndDesktop()
            , strMsg(kCQCInstMsgs::midGen_Title1)
            , strMsg(kCQCInstErrs::errcFail_ExceptInInst)
            , errToCatch
        );
        return kCIDLib::False;
    }

    catch(...)
    {
        TErrBox msgbExcept(facCQCInst.strMsg(kCQCInstErrs::errcFail_ExceptInInst));
        msgbExcept.ShowIt(TWindow::wndDesktop());
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


