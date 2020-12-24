//
// FILE NAME: CQCInstUnp_MainWindow.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/24/2015
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
//  This module implements the TMainWindow class, which provides basically all
//  of the functionality.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCInstUnp.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainWindow,TFrameWnd)


// ---------------------------------------------------------------------------
//  CLASS: TMainWindow
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainWindow: Constructors and destructor
// ---------------------------------------------------------------------------
TMainWindow::TMainWindow() :

    m_c4FilesSoFar(0)
    , m_c4TotalFiles(0)
    , m_c8Version(0)
    , m_ePrevState(EState_WaitInput)
    , m_eState(EState_WaitInput)
    , m_enctTimeStamp(0)
    , m_pwndBrowseButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndContButton(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndNotes(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndTarDir(nullptr)
    , m_pwndTarDirPref(nullptr)
    , m_pwndUseTempDir(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
    , m_thrWorker
      (
        L"CQCInstUnpThread"
        , TMemberFunc<TMainWindow>(this, &TMainWindow::eWorkThread)
      )
{
}

TMainWindow::~TMainWindow()
{
}


// ---------------------------------------------------------------------------
//  TMainWindow: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMainWindow::bCreateMainWnd()
{
    //
    //  The package should be in our directory. Let's verify that before we d
    //  anything. If not, and we can't read it, then nothing to do. We'll do a
    //  read of the header details and store them for later display.
    //
    //  We don't know the full name so we search for a CIDPack file, and use the
    //  returned find info to know the actual path to the file.
    //
    TPathStr pathPack(facCQCInstUnp.strPath());
    pathPack.AddLevel(L"*.CIDPack");

    TFindBuf fndbPack;
    if (!TFileSys::bExists(pathPack, fndbPack))
    {
        TErrBox msgbErr(facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_NoPackage));
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }

    //
    //  Extract details. This both validates it's a reasonable CIDPack file and
    //  gets us info we want to display to the user before he decides to continue.
    //
    try
    {
        facCIDPack().ExtractDetails
        (
            fndbPack.pathFileName()
            , m_c8Version
            , m_enctTimeStamp
            , m_strNotes
            , m_c4TotalFiles
        );
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
            , facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_PackDetails)
            , errToCatch
        );
        return kCIDLib::False;
    }

    // If no notes, put in a default value
    if (m_strNotes.bIsEmpty())
        m_strNotes = L"[no notes set]";

    // It worked so store the found path as the source pack file path
    m_strSrcPackFile = fndbPack.pathFileName();

    //
    //  Load the dialog description for our main window content. There is a place holder
    //  for the panels, which will be removed in bCreated() when we load everything up.
    //
    TDlgDesc dlgdChildren;
    facCQCInstUnp.bCreateDlgDesc(kCQCInstUnp::ridDlg_Main, dlgdChildren);

    // Calculate the area required to fit this content
    TArea areaInit;
    TFrameWnd::AreaForClient
    (
        dlgdChildren.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    // Center us on the current monitor
    TArea areaMon;
    facCIDCtrls().QueryMonArea(*this, areaMon, kCIDLib::True);
    areaInit.CenterIn(areaMon);

    // And now create the window
    CreateFrame
    (
        nullptr
        , areaInit
        , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TMainWindow: Protected, inherited methods
// ---------------------------------------------------------------------------

// Ask if they really want to shut down. If so, do it
tCIDLib::TBoolean TMainWindow::bAllowShutdown()
{
    // Pop up a message box
    TYesNoBox msgbTest(facCQCInstUnp.strMsg(kInstUnpMsgs::midPrompt_Close));
    return msgbTest.bShowIt(*this);
}


//
//  The usual thing. WE create our contents from a dialog description, set up
//  pointers to important chils controls, and initialize display data.
//
tCIDLib::TBoolean TMainWindow::bCreated()
{
    TString strTmp;

    // Call our parent first
    TParent::bCreated();

    // Set our frame icon, which the self extractor left for us
    facCQCInstUnp.SetFrameIcon(*this, L"CQCApp");

    //
    //  We want to load the dialog description, instead of just creating our
    //  widets indirectly by passing the resource id to CreateChildrenFromDD(),
    //  because we wnat to get the title text out of it.
    //
    TDlgDesc dlgdChildren;
    if (!facCQCInstUnp.bCreateDlgDesc(kCQCInstUnp::ridDlg_Main, dlgdChildren))
    {
        TErrBox msgbFailed(L"Could not load dialog description");
        msgbFailed.ShowIt(*this);
        Destroy();
        return kCIDLib::False;
    }

    // Create our widgets using the dialog resource
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Get pointers to some of our children
    CastChildWnd(*this, kCQCInstUnp::ridMain_Browse, m_pwndBrowseButton);
    CastChildWnd(*this, kCQCInstUnp::ridMain_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCInstUnp::ridMain_Cont, m_pwndContButton);
    CastChildWnd(*this, kCQCInstUnp::ridMain_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kCQCInstUnp::ridMain_Notes, m_pwndNotes);
    CastChildWnd(*this, kCQCInstUnp::ridMain_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCInstUnp::ridMain_TarDir, m_pwndTarDir);
    CastChildWnd(*this, kCQCInstUnp::ridMain_TarDirPref, m_pwndTarDirPref);
    CastChildWnd(*this, kCQCInstUnp::ridMain_UseTempDir, m_pwndUseTempDir);

    //
    //  Assume the user wants us to extract to a temp dir unless told otherwise.
    //  So check the check box and display the entry field.
    //
    m_pwndBrowseButton->SetEnable(kCIDLib::False);
    m_pwndUseTempDir->SetCheckedState(kCIDLib::True);
    m_pwndTarDir->SetEnable(kCIDLib::False);
    m_pwndTarDirPref->SetEnable(kCIDLib::False);

    // Register handlers we need
    m_pwndBrowseButton->pnothRegisterHandler(this, &TMainWindow::eClickHandler);
    m_pwndContButton->pnothRegisterHandler(this, &TMainWindow::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TMainWindow::eClickHandler);
    m_pwndUseTempDir->pnothRegisterHandler(this, &TMainWindow::eClickHandler);

    //
    //  Load up some initial info. The CQC version is stored in the 64 bit
    //  user value of the package header. We don't have access to the normal
    //  CQCKit helper code here, because we don't link it. So we have to
    //  pull the values out ourself and format them.
    //
    {
        tCIDLib::TCard4 c4Maj, c4Min, c4Rev;
        c4Maj = tCIDLib::TCard4(m_c8Version >> 48);
        c4Min = tCIDLib::TCard4((m_c8Version >> 32) & kCIDLib::c2MaxCard);
        c4Rev = tCIDLib::TCard4(m_c8Version & kCIDLib::c4MaxCard);

        TString strFmt(24UL);
        strFmt.AppendFormatted(c4Maj);
        strFmt.Append(kCIDLib::chPeriod);
        strFmt.AppendFormatted(c4Min);
        strFmt.Append(kCIDLib::chPeriod);
        strFmt.AppendFormatted(c4Rev);
        pwndChildById(kCQCInstUnp::ridMain_Version)->strWndText(strFmt);

        //
        //  Do another version with underscores and just the first two values,
        //  which we will user later in some file copies.
        //
        m_strFmtVer = L"_";
        m_strFmtVer.AppendFormatted(c4Maj);
        m_strFmtVer.Append(kCIDLib::chUnderscore);
        m_strFmtVer.AppendFormatted(c4Min);
    }

    // Set up a time object to format the time stamp and load it up
    {
        TTime tmFmt(m_enctTimeStamp);
        tmFmt.strDefaultFormat(TTime::strCTime());

        TString strFmt(tmFmt);
        pwndChildById(kCQCInstUnp::ridMain_TimeStamp)->strWndText(strFmt);
    }

    // The notes we can just set directly
    m_pwndNotes->strWndText(m_strNotes);

    // Activate and show ourself
    SetVisibility(kCIDLib::True);

    return kCIDLib::True;
}


//
//  We watch the status reported by the worker thread and update status info.
//  When we see a success or failure status, we either invoke the installer and
//  close ourself or display the error info, respectively.
//
//  This is only running when the thread is going. Otherwise it is disabled.
//
tCIDLib::TVoid TMainWindow::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (m_eState != m_ePrevState)
    {
        m_ePrevState = m_eState;

        // If an end state, stop the timer and enable buttons again
        if (m_ePrevState >= EState_Success)
        {
            // Stop the timer and reset the states
            if (m_tmidUpdate)
                StopTimer(m_tmidUpdate);

            m_pwndCancelButton->SetEnable(kCIDLib::True);
            m_pwndContButton->SetEnable(kCIDLib::True);
            m_pwndUseTempDir->SetEnable(kCIDLib::True);

            if (!m_pwndUseTempDir->bCheckedState())
            {
                m_pwndBrowseButton->SetEnable(kCIDLib::True);
                m_pwndTarDir->SetEnable(kCIDLib::True);
                m_pwndTarDirPref->SetEnable(kCIDLib::True);
            }

            // Clear the status window contents now
            m_pwndStatus->ClearText();
        }

        // And now react to the new state
        if (m_ePrevState == EState_Success)
        {
            //
            //  It worked so indicate we are starting the installer, and then
            //  attempt to do so.
            //
            m_pwndStatus->strWndText
            (
                facCQCInstUnp.strMsg(kInstUnpMsgs::midStatus_InvokingInst)
            );

            try
            {
                // Build the path to the installer where we extracted it
                TPathStr pathInstaller(m_strTarPath);
                pathInstaller.AddLevel(L"CQCInst");
                pathInstaller.AppendExt(L"Exe");

                //
                //  Use start as open, so that we can ask that it be run with
                //  admin privileges. The final (true) parameter does that.
                //
                TExternalProcess extpInst;
                extpInst.StartAsOpen
                (
                    pathInstaller, tCIDLib::EExtProcShows::Normal, kCIDLib::True
                );

                // It worked, so we can exit the program
                facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
            }

            catch(const TError& errToCatch)
            {
                TExceptDlg dlgErr
                (
                    *this
                    , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
                    , facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_InvokeFailed)
                    , errToCatch
                );
            }
        }
         else if (m_ePrevState == EState_Failure)
        {
            // And display the error
            TErrBox msgbErr(m_strErrReason);
            msgbErr.ShowIt(*this);
        }
         else if (m_ePrevState == EState_Exception)
        {
            // And display the exception
            TExceptDlg dlgErr
            (
                *this
                , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
                , m_strErrReason
                , m_errReason
            );
        }
         else if (m_ePrevState == EState_WaitInput)
        {
            // We are back to normal state, so clear the status
            m_pwndStatus->ClearText();
        }
    }

    // We always update the status value if in unpacking state
    if (m_ePrevState == EState_Unpacking)
    {
        // Update the file count indicator
        m_strTimerFmt = L"File # ";
        m_strTimerFmt.AppendFormatted(m_c4FilesSoFar);
        m_strTimerFmt.Append(L" of ");
        m_strTimerFmt.AppendFormatted(m_c4TotalFiles);
        m_pwndStatus->strWndText(m_strTimerFmt);
    }
}


// ---------------------------------------------------------------------------
//  TMainWindow: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper that will copy a directory from the install image up to the top
//  level directory, to build up enough stuff for the installer to run.
//
tCIDLib::TVoid
TMainWindow::CopyDir(const  TString&            strTarPath
                    , const tCIDLib::TCh* const pszSrcSub
                    , const tCIDLib::TCh* const pszTarSub)
{
    TPathStr pathSearch(strTarPath);
    pathSearch.AddLevel(L"Image");
    pathSearch.AddLevel(pszSrcSub);
    pathSearch.AddLevel(L"*.*");

    // Make sure the target subdir exists
    TFileSys::MakeSubDirectory(pszTarSub, strTarPath);

    TPathStr    pathCurTar;
    TPathStr    pathTmp;
    TFindBuf    fndbCur;
    TDirIter    diterSrc;
    if (diterSrc.bFindFirst(pathSearch, fndbCur, tCIDLib::EDirSearchFlags::AllFiles))
    {
        do
        {
            // Build up the equiv file name in the target directory
            pathTmp = fndbCur.pathFileName();
            pathTmp.bRemovePath();

            pathCurTar = strTarPath;
            pathCurTar.AddLevel(pszTarSub);
            pathCurTar.AddLevel(pathTmp);
            TFileSys::CopyFile(fndbCur.pathFileName(), pathCurTar);

        }   while (diterSrc.bFindNext(fndbCur));
    }
}


//
//  A helper that will copy the files for a DLL from the source image directories
//  up to the top level. The installer needs some DLLs and we don't want to
//  store them redundantly, so we copy them up after extracting the image.
//
tCIDLib::TVoid
TMainWindow::CopyDLL(const  TString&            strTarPath
                    , const tCIDLib::TCh* const pszSub
                    , const tCIDLib::TCh* const pszBaseName)
{
    TPathStr pathSrc(strTarPath);
    TPathStr pathTar;

    // Build up the base part of the source path
    pathSrc.AddLevel(L"Image");
    pathSrc.AddLevel(pszSub);
    pathSrc.AddLevel(pszBaseName);
    const tCIDLib::TCard4 c4BaseLen = pathSrc.c4Length();

    // Look for a msg file
    pathSrc.CapAt(c4BaseLen);
    pathSrc.Append(m_strFmtVer);
    pathSrc.Append(L"_en.CIDMsg");
    if (TFileSys::bExists(pathSrc))
    {
        // Build up the target path
        pathTar = strTarPath;
        pathTar.AddLevel(pszBaseName);
        pathTar.Append(m_strFmtVer);
        pathTar.Append(L"_en.CIDMsg");
        TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
    }

    // Look for a resource file
    pathSrc.CapAt(c4BaseLen);
    pathSrc.Append(m_strFmtVer);
    pathSrc.AppendExt(L"CIDRes");
    if (TFileSys::bExists(pathSrc))
    {
        // Build up the target path
        pathTar = strTarPath;
        pathTar.AddLevel(pszBaseName);
        pathTar.Append(m_strFmtVer);
        pathTar.AppendExt(L"CIDRes");
        TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
    }

    // Look for a DLL file
    pathSrc.CapAt(c4BaseLen);
    pathSrc.Append(m_strFmtVer);
    pathSrc.AppendExt(L"DLL");
    if (TFileSys::bExists(pathSrc))
    {
        // Build up the target path
        pathTar = strTarPath;
        pathTar.AddLevel(pszBaseName);
        pathTar.Append(m_strFmtVer);
        pathTar.AppendExt(L"DLL");
        TFileSys::CopyFile(pathSrc, pathTar);
    }
}


//
//  Handle clicks from our couple of buttons
//
tCIDCtrls::EEvResponses TMainWindow::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInstUnp::ridMain_Browse)
    {
        // Let the pick a directory
        tCIDLib::TKVPList colFileTypes;
        tCIDLib::TStrList colSel(1);
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_BrowseTitle)
            , TString::strEmpty()
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
            m_pwndTarDir->strWndText(colSel[0]);
    }
     else if (wnotEvent.widSource() == kCQCInstUnp::ridMain_Cont)
    {
        //
        //  This string will get set if we are good. It will be checked at the
        //  end below and if not empty, we start the extraction process.
        //
        TPathStr pathTar;

        //
        //  If the user asked for a temp dir, get one, Else check their entered
        //  value.
        //
        if (m_pwndUseTempDir->bCheckedState())
        {
            // Get a temporary directory to use
            try
            {
                TString strTmp;
                TFileSys::CreateTmpSubDir(L"CQCInst", strTmp, kCIDLib::False);
                pathTar = strTmp;
            }

            catch(const TError& errToCatch)
            {
                TExceptDlg dlgErr
                (
                    *this
                    , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
                    , facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_CreateTmpDir)
                    , errToCatch
                );
            }
        }
         else
        {
            //
            //  Get the user provided target dir. Just get a ref to to it and
            //  only store it to pathTar once we know it's ok.
            //
            const TString& strTar = m_pwndTarDir->strWndText();

            if (strTar.bIsEmpty())
            {
                // It cannot be empty
                TErrBox msgbErr
                (
                    facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_EmptyTarDir)
                );
                msgbErr.ShowIt(*this);
            }
             else if (!TFileSys::bExists(strTar))
            {
                // Confirm they want to create the target directory and continue
                TYesNoBox msgbConfirm
                (
                    facCQCInstUnp.strMsg(kInstUnpMsgs::midPrompt_AreYouSure2)
                );

                if (msgbConfirm.bShowIt(*this))
                    pathTar = strTar;
            }
             else if (!TFileSys::bIsDirectory(strTar))
            {
                // It exists but it's not a directory, so can't do it
                TErrBox msgbErr(facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_NotADir));
                msgbErr.ShowIt(*this);
            }
             else
            {
                //
                //  It exists and it's a directory. Make sure they want to replace
                //  the content and do the install.
                //
                TYesNoBox msgbConfirm
                (
                    facCQCInstUnp.strMsg(kInstUnpMsgs::midPrompt_AreYouSure)
                );

                if (msgbConfirm.bShowIt(*this))
                    pathTar = strTar;
            }
        }

        // If we got a path, then do it
        if (!pathTar.bIsEmpty())
        {
            // Make sure our states are initialized correctly
            m_eState = EState_Unpacking;
            m_ePrevState = m_eState;

            // Disable the buttons
            m_pwndBrowseButton->SetEnable(kCIDLib::False);
            m_pwndCancelButton->SetEnable(kCIDLib::False);
            m_pwndContButton->SetEnable(kCIDLib::False);
            m_pwndUseTempDir->SetEnable(kCIDLib::False);
            m_pwndTarDir->SetEnable(kCIDLib::False);
            m_pwndTarDirPref->SetEnable(kCIDLib::False);

            // Initialize per round data and start the thread
            m_c4FilesSoFar = 0;
            m_strTarPath = pathTar;
            m_thrWorker.Start();

            // And start the timer for once a second
            m_tmidUpdate = tmidStartTimer(1000);
        }
    }
     else if (wnotEvent.widSource() == kCQCInstUnp::ridMain_UseTempDir)
    {
        //
        //  Enable/disable the target directory entry field as this check box
        //  is checked or unchecked.
        //
        if (m_pwndUseTempDir->bCheckedState())
        {
            // If the focus is on the entry field, move it, then disable it
            if (m_pwndTarDir->bHasFocus())
                m_pwndContButton->TakeFocus();

            m_pwndBrowseButton->SetEnable(kCIDLib::False);
            m_pwndTarDir->ClearText();
            m_pwndTarDir->SetEnable(kCIDLib::False);
            m_pwndTarDirPref->SetEnable(kCIDLib::False);
        }
         else
        {
            // Enable the entry field and put the focus there
            m_pwndBrowseButton->SetEnable(kCIDLib::True);
            m_pwndTarDir->SetEnable(kCIDLib::True);
            m_pwndTarDirPref->SetEnable(kCIDLib::True);
            m_pwndTarDir->TakeFocus();
        }
    }
     else if (wnotEvent.widSource() == kCQCInstUnp::ridMain_Cancel)
    {
        // Make sure they really want to exit
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



//
//  This thread does the actual work.
//
tCIDLib::EExitCodes TMainWindow::eWorkThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the thread that started us go now
    thrThis.Sync();

    try
    {
        //
        //  Clean up the target directory if it exists. This will not remove the
        //  top level directory, just the contents.
        //
        //  Else create the top level directory.
        //
        if (TFileSys::bIsDirectory(m_strTarPath))
            TFileSys::CleanPath(m_strTarPath, tCIDLib::ETreeDelModes::Remove);
        else
            TFileSys::MakePath(m_strTarPath);
    }

    catch(const TError& errToCatch)
    {
        m_strErrReason = facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_TarDirFailed);
        m_errReason = errToCatch;
        m_errReason.AddStackLevel(CID_FILE, CID_LINE);
        m_eState = EState_Exception;
        return tCIDLib::EExitCodes::Normal;
    }

    //
    //  OK, let's use the CIDPack facility to unpack the installer. It will
    //  do all of the work and create the original tree structure and whatnot.
    //
    try
    {
        facCIDPack().ExtractPackage
        (
            m_strSrcPackFile
            , m_strTarPath
            , kCIDLib::True
            , m_c8Version
            , m_enctTimeStamp
            , m_strNotes
            , m_c4TotalFiles
            , 0
            , m_c4FilesSoFar
            , kCIDLib::False
        );
    }

    catch(const TError& errToCatch)
    {
        m_strErrReason = facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_ExtractFailed);
        m_errReason = errToCatch;
        m_errReason.AddStackLevel(CID_FILE, CID_LINE);
        m_eState = EState_Exception;
        return tCIDLib::EExitCodes::Normal;
    }

    //
    //  Now we need to copy some DLLs and supporting files up to the top target
    //  level because the installer exe needs them. We don't have them there to
    //  begin with in order to avoid having to redundantly store them in the
    //  install package.
    //
    try
    {
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDComm");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDCrypto");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDGraphDev");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDCtrls");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDDBase");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDEncode");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDKernel");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDImage");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDImgFact");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDLib");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDJPEG");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDMath");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDMacroEng");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDMData");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDNet");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDObjStore");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDOrb");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDOrbUC");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDPNG");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDRegX");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDSChan");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDSock");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDXML");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDWebBrowser");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDWUtils");
        CopyDLL(m_strTarPath, L"CIDLib", L"CIDZLib");

        CopyDLL(m_strTarPath, L"CQC", L"CQCAct");
        CopyDLL(m_strTarPath, L"CQC", L"CQCDriver");
        CopyDLL(m_strTarPath, L"CQC", L"CQCEvCl");
        CopyDLL(m_strTarPath, L"CQC", L"CQCGKit");
        CopyDLL(m_strTarPath, L"CQC", L"CQCIntfEng");
        CopyDLL(m_strTarPath, L"CQC", L"CQCKit");
        CopyDLL(m_strTarPath, L"CQC", L"CQCMedia");
        CopyDLL(m_strTarPath, L"CQC", L"CQCMEng");
        CopyDLL(m_strTarPath, L"CQC", L"CQLogicSh");
        CopyDLL(m_strTarPath, L"CQC", L"CQCPollEng");
        CopyDLL(m_strTarPath, L"CQC", L"CQCRemBrws");

        // Copy over some miscellaneous stuff
        CopyDir(m_strTarPath, L"AppImages", L"AppImages");
        CopyDir(m_strTarPath, L"CQC\\LogoImages", L"LogoImages");

        // We need the CIDLib common loadable text stuff
        TPathStr pathSrc(m_strTarPath);
        pathSrc.AddLevel(L"Image\\CIDLib\\CIDCommonText_en.CIDMsg");
        TPathStr pathTar;
        if (TFileSys::bExists(pathSrc))
        {
            pathTar = m_strTarPath;
            pathTar.AddLevel(L"CIDCommonText_en.CIDMsg");
            TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
        }

        //
        //  Get the app icon into the appropriate place as it would be in the final image,
        //  but under the unpack directory, so the installer will pick it up.
        //
        TFileSys::MakeSubDirectory(L"AppIcons", m_strTarPath);
        pathSrc = m_strTarPath;
        pathSrc.AddLevel(L"Image\\AppIcons\\CQCApp.ico");
        if (TFileSys::bExists(pathSrc))
        {
            pathTar = m_strTarPath;
            pathTar.AddLevel(L"AppIcons\\CQCApp.ico");
            TFileSys::CopyFile(pathSrc, pathTar, kCIDLib::True);
        }
    }

    catch(const TError& errToCatch)
    {
        m_strErrReason = facCQCInstUnp.strMsg(kInstUnpErrs::errcGen_ImgSetupFailed);
        m_errReason = errToCatch;
        m_errReason.AddStackLevel(CID_FILE, CID_LINE);
        m_eState = EState_Exception;
        return tCIDLib::EExitCodes::Normal;
    }

    // Indicate success
    m_eState = EState_Success;
    return tCIDLib::EExitCodes::Normal;
}

