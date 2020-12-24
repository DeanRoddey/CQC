//
// FILE NAME: CQCUninst_MainWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/16/2003
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
//  This file implements the window that does the uninstall and shows the
//  user the status.
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
#include    "CQCUninst.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWnd)



// ---------------------------------------------------------------------------
//   CLASS: TMainFrameWnd
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_bRemoveDir(kCIDLib::False)
    , m_eState(tCQCUninst::EState_WaitStart)
    , m_pwndCancel(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndList(nullptr)
    , m_pwndRemoveDir(nullptr)
    , m_pwndStart(nullptr)
    , m_thrWorker
      (
        L"CQCUnistThread"
        , TMemberFunc<TMainFrameWnd>(this, &TMainFrameWnd::ePollThread)
      )
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TMainFrameWnd::bRemoveDir() const
{
    return m_bRemoveDir;
}


tCIDLib::TVoid TMainFrameWnd::CreateMainWnd()
{
    // Load the dialog description we are based on
    TDlgDesc dlgdChildren;
    facCQCUninst.bCreateDlgDesc(kCQCUninst::ridMain_Uninst, dlgdChildren);

    // Calculate a frame area to contain this content
    TArea areaInit;
    TFrameWnd::AreaForClient
    (
        dlgdChildren.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    // Set the origin to the min int value so that they will default
    areaInit.SetOrg(kCIDLib::i4MinInt, kCIDLib::i4MinInt);

    CreateFrame
    (
        nullptr
        , areaInit
        , facCQCUninst.strMsg(kCQCUninstMsgs::midMain_Uninst)
        , tCIDCtrls::EWndStyles::StdFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );
}


const TString& TMainFrameWnd::strRemoveDirCmd() const
{
    return m_pathDelCmd;
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected inherited methods
// ---------------------------------------------------------------------------

// If not in running state, we allow it
tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    return (m_eState != tCQCUninst::EState_Running);
}



tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Load the dialog descripion for our stuff and use it to create the controls
    TDlgDesc dlgdChildren;
    facCQCUninst.bCreateDlgDesc(kCQCUninst::ridMain_Uninst, dlgdChildren);
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCUninst::ridMain_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCUninst::ridMain_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kCQCUninst::ridMain_List, m_pwndList);
    CastChildWnd(*this, kCQCUninst::ridMain_RemoveDir, m_pwndRemoveDir);
    CastChildWnd(*this, kCQCUninst::ridMain_Start, m_pwndStart);

    // Set our title text
    strWndText(facCQCUninst.strMsg(kCQCUninstMsgs::midMain_Uninst));

    // Register the buttons
    m_pwndCancel->pnothRegisterHandler(this, &TMainFrameWnd::eClickHandler);
    m_pwndStart->pnothRegisterHandler(this, &TMainFrameWnd::eClickHandler);

    // Set our icon
    facCQCUninst.SetFrameIcon(*this, L"CQCApp");

    // Set up the columns on the list window
    m_colCols.objAdd(facCQCUninst.strMsg(kCQCUninstMsgs::midMain_StatusCol));
    m_colCols.objAdd(facCQCUninst.strMsg(kCQCUninstMsgs::midMain_DescriptionCol));
    m_pwndList->SetColumns(m_colCols);

    // Size the first one to fit the title
    m_pwndList->AutoSizeCol(0, kCIDLib::True);

    //
    //  We initially create a multi-static text control to cover the list, and to provide
    //  guidance to the user before starting. We'll destroy it if they start. It's already
    //  there we just need to size it to sit over the list.
    //
    TArea areaInstruct = m_pwndList->areaWnd();
    m_pwndInstruct->SetSizePos(areaInstruct, kCIDLib::False);

    // Set our min and max size to our current size, to prevent resizing
    const TArea areaCur = areaWndSize();
    SetMinMaxSize(areaCur.szArea(), areaCur.szArea());

    return kCIDLib::True;
}


//
//  The bgn thread posts us notifications of his status. The code is an ESteps enum value.
//  The state parm is an EStatus enum value.
//
tCIDLib::TVoid
TMainFrameWnd::CodeReceived(const tCIDLib::TInt4 i4Step, const tCIDLib::TCard4 c4State)
{
    const tCQCUninst::ESteps eStep = tCQCUninst::ESteps(i4Step);
    const tCQCUninst::EStatus eStatus =  tCQCUninst::EStatus(c4State);

    tCIDLib::TMsgId midStarted;
    tCIDLib::TMsgId midFailed;
    switch(eStep)
    {
        case tCQCUninst::EStep_CleanRegistry :
            midStarted = kCQCUninstMsgs::midMain_CleanReg;
            midFailed = kCQCUninstMsgs::midMain_CleanRegFailed;
            break;

        case tCQCUninst::EStep_StopCSrv :
            midStarted = kCQCUninstMsgs::midMain_StopCSrv;
            midFailed = kCQCUninstMsgs::midMain_StopCSrvFailed;
            break;

        case tCQCUninst::EStep_StopSSrv :
            midStarted = kCQCUninstMsgs::midMain_StopSSrv;
            midFailed = kCQCUninstMsgs::midMain_StopSSrvFailed;
            break;

        case tCQCUninst::EStep_RemoveMenu :
            midStarted = kCQCUninstMsgs::midMain_RemoveMenu;
            midFailed = kCQCUninstMsgs::midMain_RemoveMenuFailed;
            break;

        case tCQCUninst::EStep_RemoveTrayMon :
            midStarted = kCQCUninstMsgs::midMain_RemoveTrayMon;
            midFailed = kCQCUninstMsgs::midMain_RemoveTrayMonFailed;
            break;

        case tCQCUninst::EStep_RemoveCQCVoice :
            midStarted = kCQCUninstMsgs::midMain_RemoveCQCVoice;
            midFailed = kCQCUninstMsgs::midMain_RemoveCQCVoiceFailed;
            break;

        case tCQCUninst::EStep_GenDelCmd :
            midStarted = kCQCUninstMsgs::midMain_GenRemCmd;
            midFailed = kCQCUninstMsgs::midMain_GenRemCmdFailed;
            break;

        default :
            break;
    };

    // If complete we do one thing, else we do a common scenario for the others
    if (eStep == tCQCUninst::EStep_Complete)
    {
        m_colCols[0] = TString::strEmpty();
        m_colCols[1] = facCQCUninst.strMsg(kCQCUninstMsgs::midMain_Complete);
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(m_colCols, 0);
        m_pwndList->SetCheckAt(c4At, kCIDLib::True);

        // Tell the user about success or failure
        if (m_colErrs.bIsEmpty())
        {
            TOkBox msgbSuccess(facCQCUninst.strMsg(kCQCUninstMsgs::midStatus_Successful));
            msgbSuccess.ShowIt(*this);
        }
         else
        {
            TErrBox msgbFail(facCQCUninst.strMsg(kCQCUninstMsgs::midStatus_Failures));
            msgbFail.ShowIt(*this);
        }

        // Enable the cancel button and set the text to close
        m_pwndCancel->strWndText(facCQCUninst.strMsg(kCQCUninstMsgs::midMain_Close));
        m_pwndCancel->SetEnable(kCIDLib::True);
    }
     else
    {
        if (eStatus == tCQCUninst::EStatus_Starting)
        {
            m_colCols[0] = TString::strEmpty();
            m_colCols[1] = facCQCUninst.strMsg(midStarted);
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(m_colCols, 0);
            m_pwndList->SetCheckAt(c4At, kCIDLib::False);
        }
         else if (eStatus == tCQCUninst::EStatus_Success)
        {
            m_pwndList->SetCheckAt(eStep, kCIDLib::True);
        }
         else if (eStatus == tCQCUninst::EStatus_Failure)
        {
            m_pwndList->SetColText(eStep, 1, facCQCUninst.strMsg(midFailed));
        }
    }
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TMainFrameWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCUninst::ridMain_Start)
    {
        //
        //  See if any GUI apps are running as far as we can tell. If so, tell them to
        //  stop those first.
        //
        {
            tCIDLib::TKValsList colAppList;
            tCIDLib::TStrList colActive;

            colAppList.objAdd(TKeyValues(L"CQCAdmin", L"Admin Interface"));
            colAppList.objAdd(TKeyValues(L"MediaRepoMgr", L"CQSL Media Repo Manger"));
            colAppList.objAdd(TKeyValues(L"CQCIntfView", L"Interface Viewer"));
            colAppList.objAdd(TKeyValues(L"CQCVoice", L"CQC Voice Tray", L"CQC Voice"));
            colAppList.objAdd(TKeyValues(L"CIDLogMon", L"Log Monitor"));
            colAppList.objAdd(TKeyValues(L"CQCRPortSrv", L"Remote Port Server"));
            colAppList.objAdd(TKeyValues(L"CQCTrayMon", L"Tray Monitor", L"CQC Tray Monitor"));

            tCIDLib::TBoolean bFound;
            tCIDLib::TCard4 c4Count = colAppList.c4ElemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TKeyValues& kvalsCur = colAppList[c4Index];
                TResourceName rsnInstance(L"CQSL", kvalsCur.strKey(), L"SingleInstanceInfo");
                if (TProcess::bCheckSingleInstanceInfo(rsnInstance, bFound) && bFound)
                    colActive.objAdd(kvalsCur.strVal1());
            }

            if (!colActive.bIsEmpty())
            {
                TString strMsg(L"The following GUI applications are still running, please stop them first\n\n");
                c4Count = colActive.c4ElemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    if (c4Index)
                        strMsg.Append(L"\n");
                    strMsg.Append(colActive[c4Index]);
                }

                TErrBox msgbGUIApps(strMsg);
                msgbGUIApps.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
        }

        // Remember the remove directory option
        m_bRemoveDir = m_pwndRemoveDir->bCheckedState();

        //
        //  This is only enabled when it's legal to use. Now we disable the buttons and
        //  start the bgn thread. We also disable the cancel button since we go all the
        //  way once we start. And the remove directory option, since it's now too late.
        //
        m_pwndCancel->SetEnable(kCIDLib::False);
        m_pwndStart->SetEnable(kCIDLib::False);
        m_pwndRemoveDir->SetEnable(kCIDLib::False);

        // Hide the instruction control, to expose the list
        m_pwndInstruct->SetVisibility(kCIDLib::False);

        // And start it going
        m_thrWorker.Start();
    }
     else if (wnotEvent.widSource() == kCQCUninst::ridMain_Cancel)
    {
        // Either we are before starting or at the end, so just exit
        facCIDCtrls().ExitLoop(kCQCUninst::ridMain_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::EExitCodes TMainFrameWnd::ePollThread(TThread& thrThis, tCIDLib::TVoid*)
{
    // Let the thread that started us go now
    thrThis.Sync();

    // Clean the registry
    try
    {
        SendCode(tCQCUninst::EStep_CleanRegistry, tCQCUninst::EStatus_Starting);
        tCIDKernel::TWRegHandle hkeySoftware = TKrnlWin32Registry::hkeyGetRootSubKey
        (
            tCIDKernel::ERootKeys::LocalMachine, L"SOFTWARE", tCIDKernel::ERegAccFlags::StdOwned
        );
        TKrnlWin32Registry::bDeleteKey(hkeySoftware, kCQCKit::pszReg_Key);
        TKrnlWin32Registry::bCloseKey(hkeySoftware);
        TThread::Sleep(1000);
        SendCode(tCQCUninst::EStep_CleanRegistry, tCQCUninst::EStatus_Success);
    }

    catch(const TError& errToCatch)
    {
        m_colErrs.objAdd(errToCatch);
        SendCode(tCQCUninst::EStep_CleanRegistry, tCQCUninst::EStatus_Failure);
    }

    // Clean up the cient service
    try
    {
        SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Starting);

        // Ok, let's see if the CQC App Shell service is installed
        tCIDKernel::TWSvcHandle hsvcCQC = TKrnlWin32Service::hsvcOpen
        (
            kCQCKit::pszClServiceName
        );

        // If we opened it, then it exists, so let's stop it if its running
        if (hsvcCQC)
        {
            if (!TKrnlWin32Service::bStop(hsvcCQC, 60000))
            {
                SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Failure);
            }
             else
            {
                // We stopped it ok, so remove it
                TThread::Sleep(2000);
                if (TKrnlWin32Service::bRemove(hsvcCQC))
                    SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Success);
                else
                    SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Failure);
            }

            // And close the service now, which will let it drop
            TKrnlWin32Service::bClose(hsvcCQC);
        }
         else
        {
            // Not present so just say it's stopped
            TThread::Sleep(2000);
            SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Success);
        }
    }

    catch(const TError& errToCatch)
    {
        m_colErrs.objAdd(errToCatch);
        SendCode(tCQCUninst::EStep_StopCSrv, tCQCUninst::EStatus_Failure);
    }


    // Clean up the server service
    try
    {
        SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Starting);

        // Ok, let's see if the CQC App Shell service is installed
        tCIDKernel::TWSvcHandle hsvcCQC = TKrnlWin32Service::hsvcOpen
        (
            kCQCKit::pszCQCShellName
        );

        // If we opened it, then it exists, so let's stop it if its running
        if (hsvcCQC)
        {
            if (!TKrnlWin32Service::bStop(hsvcCQC, 60000))
            {
                SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Failure);
            }
             else
            {
                // We stopped it ok, so remove it
                TThread::Sleep(2000);
                if (TKrnlWin32Service::bRemove(hsvcCQC))
                    SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Success);
                else
                    SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Failure);
            }

            // And close the service now, which will let it drop
            TKrnlWin32Service::bClose(hsvcCQC);
        }
         else
        {
            // Not present so just say it's stopped
            TThread::Sleep(2000);
            SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Success);
        }
    }

    catch(const TError& errToCatch)
    {
        m_colErrs.objAdd(errToCatch);
        SendCode(tCQCUninst::EStep_StopSSrv, tCQCUninst::EStatus_Failure);
    }

    // Clean up the start menu stuff
    try
    {
        SendCode(tCQCUninst::EStep_RemoveMenu, tCQCUninst::EStatus_Starting);
        TThread::Sleep(2000);

        // Build the path to our main start menu folder
        TPathStr pathTargetDir
        (
            TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::CommonMenuPrograms)
        );
        pathTargetDir.AddLevel(facCQCKit().strMsg(kKitMsgs::midSMenu_LinkScope));

        // If it exists, remove the whole thing
        if (TFileSys::bExists(pathTargetDir, tCIDLib::EDirSearchFlags::NormalDirs))
            TFileSys::RemovePath(pathTargetDir);

        // That all worked
        SendCode(tCQCUninst::EStep_RemoveMenu, tCQCUninst::EStatus_Success);
    }

    catch(const TError& errToCatch)
    {
        m_colErrs.objAdd(errToCatch);
        SendCode(tCQCUninst::EStep_RemoveMenu, tCQCUninst::EStatus_Failure);
    }

    catch(...)
    {
        SendCode(tCQCUninst::EStep_RemoveMenu, tCQCUninst::EStatus_Failure);
    }

    //
    //  Remove the auto-start tasks, if they got created. Only bother if the task
    //  scheduler support is available on this machine.
    //
    if (TKrnlTaskSched::bSupportAvail())
    {
        SendCode(tCQCUninst::EStep_RemoveTrayMon, tCQCUninst::EStatus_Starting);
        if (TKrnlTaskSched::bDeleteSchedTask(kCQCUninst::strTask_TrayAppAutoStart.pszBuffer()))
            SendCode(tCQCUninst::EStep_RemoveTrayMon, tCQCUninst::EStatus_Success);
        else
            SendCode(tCQCUninst::EStep_RemoveTrayMon, tCQCUninst::EStatus_Failure);

        SendCode(tCQCUninst::EStep_RemoveCQCVoice, tCQCUninst::EStatus_Starting);
        if (TKrnlTaskSched::bDeleteSchedTask(kCQCUninst::strTask_CQCVoiceAutoStart.pszBuffer()))
            SendCode(tCQCUninst::EStep_RemoveCQCVoice, tCQCUninst::EStatus_Success);
        else
            SendCode(tCQCUninst::EStep_RemoveCQCVoice, tCQCUninst::EStatus_Failure);
    }

    //
    //  If they asked to remove the directory, then let's generate the command file that
    //  we'll kick off when we exit.
    //
    if (m_bRemoveDir)
    {
        SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Starting);
        try
        {
            //
            //  Our own path should be the bin directory under what we want to remove. Get
            //  it and do some checking just to be sure. We don't want to do anything
            //  stupid.
            //
            TPathStr pathRemove(facCQCUninst.strPath());
            if (!pathRemove.bEndsWithI(L"Bin"))
            {
                SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Failure);
                m_bRemoveDir = kCIDLib::False;
            }

            if (!pathRemove.bRemoveLevel()
            ||  !TFileSys::bExists(pathRemove, L"CQCInstVersion.Info", tCIDLib::EDirSearchFlags::NormalFiles)
            ||  !TFileSys::bExists(pathRemove, L"Bin", tCIDLib::EDirSearchFlags::NormalDirs)
            ||  !TFileSys::bExists(pathRemove, L"CQCData", tCIDLib::EDirSearchFlags::NormalDirs))
            {
                SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Failure);
                m_bRemoveDir = kCIDLib::False;
            }

            // If we didn't force it off due to errors above, generate the file
            if (m_bRemoveDir)
            {
                m_pathDelCmd = TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData);
                m_pathDelCmd.AddLevel(L"CQCDelete.Cmd");
                TTextFileOutStream strmTar
                (
                    m_pathDelCmd
                    , tCIDLib::ECreateActs::CreateAlways
                    , tCIDLib::EFilePerms::Default
                    , tCIDLib::EFileFlags::SequentialScan
                    , tCIDLib::EAccessModes::Excl_Write
                );

                strmTar << L"@ECHO OFF\n"
                        << L"ECHO Removing CQC Directory\n"
                        << L"rd \"" << pathRemove << L"\" /s" << kCIDLib::NewLn
                        << L"start /b \"\" cmd.exe /c del \"" << m_pathDelCmd << L"\"" << kCIDLib::NewLn
                        << kCIDLib::FlushIt;
            }
            SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Success);
        }

        catch(const TError& errToCatch)
        {
            m_colErrs.objAdd(errToCatch);
            SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Failure);
        }

        catch(...)
        {
            SendCode(tCQCUninst::EStep_GenDelCmd, tCQCUninst::EStatus_Failure);
        }
    }

    // And we are done so send the complete notification
    SendCode(tCQCUninst::EStep_Complete, tCQCUninst::EStatus_Success);

    return tCIDLib::EExitCodes::Normal;
}


