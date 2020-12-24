//
// FILE NAME: CQCInst_StopGUIAppsPanel.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/14/2010
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
//  This file implements a panel that checks to see if any GUI apps appear
//  to be running and offers to shut them down, or gives the user a chance
//  to shut them down.
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
RTTIDecls(TInstStopGUIAppsPanel,TInstInfoPanel)


// ---------------------------------------------------------------------------
//   CLASS: TInstStopGUIAppsPanel
//  PREFIX: pan
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TInstStopGUIAppsPanel: Constructors and Destructor
// ---------------------------------------------------------------------------
TInstStopGUIAppsPanel::TInstStopGUIAppsPanel(TCQCInstallInfo* const pinfoCur
                                            , TMainFrameWnd* const  pwndParent) :

    TInstInfoPanel(L"Stop GUI Applications", pinfoCur, pwndParent)
    , m_eStatus(EResults::Idle)
    , m_pwndDoIt(nullptr)
    , m_pwndList(nullptr)
    , m_thrStopGUIApps
      (
        TString(L"CQCInstStopGUIAppsThread")
        , TMemberFunc<TInstStopGUIAppsPanel>(this, &TInstStopGUIAppsPanel::eStopGUIAppsThread)
      )
{
}

TInstStopGUIAppsPanel::~TInstStopGUIAppsPanel()
{
}


// ---------------------------------------------------------------------------
//  TInstStopGUIAppsPanel: Public, inherited methods
// ---------------------------------------------------------------------------

// See if any client apps are running
tCIDLib::TBoolean TInstStopGUIAppsPanel::bPanelIsVisible() const
{
    return bFindActiveApps();
}


tCIDLib::TVoid TInstStopGUIAppsPanel::Entered()
{
    // Load our list
    if (bFindActiveApps())
        LoadActiveAppList();
    else
        m_pwndList->RemoveAll();
}


// ---------------------------------------------------------------------------
//  TInstStopGUIAppsPanel: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TInstStopGUIAppsPanel::bCreated()
{
    TParent::bCreated();

    // Load our controls
    LoadDlgItems(kCQCInst::ridPan_StopGUIApps);

    // Get some pointers to the ones we want to interact with
    CastChildWnd(*this, kCQCInst::ridPan_StopGUIApps_DoIt, m_pwndDoIt);
    CastChildWnd(*this, kCQCInst::ridPan_StopGUIApps_List, m_pwndList);

    // Register a click handler on the main frame for our browse button
    m_pwndDoIt->pnothRegisterHandler(this, &TInstStopGUIAppsPanel::eClickHandler);

    //
    //  Fill in a list with enough info to find the apps. We need the resource name
    //  (used to enforce single process instances being running) and a name to put into
    //  the list box for display, and the text of the top level window to find.
    //
    m_colAppList.RemoveAll();
    m_colAppList.objAdd(TKeyValues(L"CQCAdmin", L"Admin Interface"));
    m_colAppList.objAdd(TKeyValues(L"MediaRepoMgr", L"CQSL Media Repo Manger"));
    m_colAppList.objAdd(TKeyValues(L"CQCIntfView", L"Interface Viewer"));
    m_colAppList.objAdd(TKeyValues(L"CQCVoice", L"CQC Voice Tray", L"CQC Voice"));
    m_colAppList.objAdd(TKeyValues(L"CIDLogMon", L"Log Monitor"));
    m_colAppList.objAdd(TKeyValues(L"CQCRPortSrv", L"Remote Port Server"));
    m_colAppList.objAdd(TKeyValues(L"CQCTrayMon", L"Tray Monitor", L"CQC Tray Monitor"));

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TInstStopGUIAppsPanel: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Does a check for any running GUI apps. Since all of the GUI apps use
//  a common scheme for avoiding multiple isntances of them running, we can
//  just use that scheme to find out if the are running. They create a named
//  rseource that we can just look up ourself.
//
tCIDLib::TBoolean TInstStopGUIAppsPanel::bFindActiveApps() const
{
    tCIDLib::TBoolean bFound;
    m_colActive.RemoveAll();

    //
    //  Loop through all the defined apps and load up the descriptions of those
    //  found running.
    //
    const tCIDLib::TCard4 c4Count = m_colAppList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TKeyValues& kvalsCur = m_colAppList[c4Index];
        TResourceName rsnInstance(L"CQSL", kvalsCur.strKey(), L"SingleInstanceInfo");
        if (TProcess::bCheckSingleInstanceInfo(rsnInstance, bFound) && bFound)
            m_colActive.objAdd(kvalsCur.strVal1());
    }
    return !m_colActive.bIsEmpty();
}


tCIDCtrls::EEvResponses TInstStopGUIAppsPanel::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCInst::ridPan_StopGUIApps_DoIt)
    {
        m_pwndDoIt->SetEnable(kCIDLib::False);

        //
        //  Start the shutdown thread and wait for it to complete. It will
        //  go fairly quickly.
        //
        m_thrStopGUIApps.Start();

        //
        //  Now we have to wait until the thread completes. It won't take long
        //  so we just do a blocking wait.
        //
        try
        {
            m_thrStopGUIApps.eWaitForDeath(20000);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        // Re-enable the button
        m_pwndDoIt->SetEnable(kCIDLib::True);

        // Now reload the list with any that still show up
        LoadActiveAppList();
    }
    return tCIDCtrls::EEvResponses::Handled;
}

//
//  The thread that does the actual work of signaling the programs to stop
//  and waiting for them to stop.
//
tCIDLib::EExitCodes
TInstStopGUIAppsPanel::eStopGUIAppsThread(TThread& thrThis, tCIDLib::TVoid* pData)
{
    // Let the calling thread go
    thrThis.Sync();

    try
    {
        //
        //  Let's go back and check them again and try to shut them down.
        //  For the most part we will need to try to post a QUIT message to
        //  them. These GUI programs don't maintain any sort of ORB interface
        //  via which to converse with them.
        //
        const TString& strNull = TString::strEmpty();
        tCIDLib::TBoolean bFound;
        tCIDLib::EExitCodes eToFill;
        tCIDCtrls::TWndHandle hwndTar;

        const tCIDLib::TCard4 c4Count = m_colAppList.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValues& kvalsCur = m_colAppList[c4Index];

            TResourceName rsnInstance(L"CQSL", kvalsCur.strKey(), L"SingleInstanceInfo");
            TExternalProcess extpTar;
            if (TProcess::bCheckSingleInstanceInfo(rsnInstance, bFound, &extpTar) && bFound)
            {
                hwndTar = hwndFindWnd(kvalsCur, extpTar);
                if (hwndTar)
                {
                    facCIDCtrls().mresSendAsyncMsg(hwndTar, 0x12, 0, 0);

                    // Wait a while for it to end. If not, try it again
                    if (!extpTar.bWaitForDeath(eToFill, 2000))
                    {
                        hwndTar = hwndFindWnd(kvalsCur, extpTar);
                        if (hwndTar)
                        {
                            facCIDCtrls().mresSendAsyncMsg(hwndTar, 0x12, 0, 0);

                            // And again
                            if (!extpTar.bWaitForDeath(eToFill, 2000))
                            {
                                hwndTar = hwndFindWnd(kvalsCur, extpTar);
                                if (hwndTar)
                                    facCIDCtrls().mresSendAsyncMsg(hwndTar, 0x12, 0, 0);
                            }
                        }
                    }
                }
            }
        }

        // Pause a bit to let them complete shutdown
        TThread::Sleep(4000);
        m_eStatus = EResults::Done;
    }

    catch(TError& errToCatch)
    {
        if (!errToCatch.bLogged())
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCInst.LogEventObj(errToCatch);
        }
        facCQCInst.LogInstallMsg(L"Stop GUI Apps operation failed, see logs");
        m_eStatus = EResults::DoneWithError;
    }

    catch(...)
    {
        facCQCInst.LogInstallMsg(L"Unknown exception in Stop GUI Apps thread");
        m_eStatus = EResults::DoneWithError;
    }
    return tCIDLib::EExitCodes::Normal;
}


//
//  Try to find a window in the target app that we can send a quit message
//  to in order to get the app to exit.
//
tCIDCtrls::TWndHandle
TInstStopGUIAppsPanel::hwndFindWnd( const   TKeyValues&         kvalsApp
                                    ,       TExternalProcess&   extpTar)
{
    const TString& strWndText = kvalsApp.strVal2();

    //
    //  If there's window text, then look for one that has that specific
    //  text. If not, then just try to find the to level window.
    //
    tCIDCtrls::TWndHandle hwndRet;
    if (strWndText.bIsEmpty())
    {
        hwndRet = facCIDCtrls().hwndFindTopAppWnd
        (
            extpTar, TString::strEmpty(), kCIDLib::False
        );
    }
     else
    {
        hwndRet = facCIDCtrls().hwndFindAppWnd
        (
            extpTar, strWndText, TString::strEmpty(), kCIDLib::False
        );
    }
    return hwndRet;
}


//
//  Go through the list of defined apps. For any that are active, load them
//  into the active app list. If we find any, load up the list window.
//
tCIDLib::TVoid TInstStopGUIAppsPanel::LoadActiveAppList()
{
    TWndPaintJanitor janPaint(m_pwndList);

    m_pwndList->RemoveAll();
    if (bFindActiveApps())
    {
        const tCIDLib::TCard4 c4Count = m_colActive.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_pwndList->c4AddItem(m_colActive[c4Index]);

        if (c4Count)
            m_pwndList->SelectByIndex(0);
    }
}


