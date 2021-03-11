//
// FILE NAME: CQCTrayMon_AppCtrlTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Facility specific includes
// ---------------------------------------------------------------------------
#include    "CQCTrayMon.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TACActivatorWnd, TWindow)
RTTIDecls(TCQCTrayAppCtrlTab, TCQCTrayMonTab)




// ---------------------------------------------------------------------------
//  CLASS: TACActivatorWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TACActivatorWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TACActivatorWnd::TACActivatorWnd() :

    TWindow(kCIDLib::False)
{
}

TACActivatorWnd::~TACActivatorWnd()
{
}


// ---------------------------------------------------------------------------
//  TACActivatorWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Just create us invisible, and top level
tCIDLib::TVoid TACActivatorWnd::Create()
{
    CreateWnd
    (
        TWindow::wndDesktop().hwndThis()
        , kCIDCtrls::pszCustClass
        , L""
        , TArea(-1000, -1000, 8, 8)
        , tCIDCtrls::EWndStyles::None
        , tCIDCtrls::EExWndStyles::None
        , 0
    );
}





// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayAppCtrlTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCTrayAppCtrlTab: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCTrayAppCtrlTab::TCQCTrayAppCtrlTab() :

    TCQCTrayMonTab(L"App Control")
    , m_bLastStatus(kCIDLib::False)
    , m_colEventQ(tCIDLib::EAdoptOpts::Adopt, tCIDLib::EMTStates::Safe)
    , m_colStatusQ(tCIDLib::EMTStates::Safe)
    , m_errACLoopExcept
      (
        facCQCTrayMon.strName()
        , CID_FILE
        , CID_LINE
        , kTrayMonErrs::errcApp_LoopExcept
        , facCQCTrayMon.strMsg(kTrayMonErrs::errcApp_LoopExcept)
        , tCIDLib::ESeverities::Status
        , tCIDLib::EErrClasses::AppStatus
      )
    , m_tmidUpdate(0)
    , m_porbsAppCtrl(nullptr)
    , m_pwndAppName(nullptr)
    , m_pwndAppPath(nullptr)
    , m_pwndDir(nullptr)
    , m_pwndEvLog(nullptr)
    , m_pwndList(nullptr)
    , m_pwndMoniker(nullptr)
    , m_pwndParams(nullptr)
    , m_pwndStatus(nullptr)
{
}

TCQCTrayAppCtrlTab::~TCQCTrayAppCtrlTab()
{
    try
    {
        m_colEventQ.RemoveAll();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    catch(...)
    {
    }
}


// ---------------------------------------------------------------------------
//  TCQCTrayAppCtrlTab: Public, inherited methods
// ---------------------------------------------------------------------------

// This is called after RemoveBindings, to do any final cleanup
tCIDLib::TVoid TCQCTrayAppCtrlTab::Cleanup()
{
    // Clean up our ORB interface if we created it
    if (m_porbsAppCtrl)
    {
        facCIDOrb().DeregisterObject(m_porbsAppCtrl);
        m_porbsAppCtrl = nullptr;
    }
}


// Called during shutdown to get us to unbind any bindings we have
tCIDLib::TVoid TCQCTrayAppCtrlTab::RemoveBindings(tCIDOrbUC::TNSrvProxy& orbcNS)
{
    try
    {
        TString strBind = TAppCtrlServerBase::strImplBinding;
        strBind.eReplaceToken(facCQCTrayMon.strBinding(), kCIDLib::chLatin_b);
        orbcNS->RemoveBinding(strBind, kCIDLib::False);
    }

    catch(...)
    {
        // If we can't, oh well. The NS probably is already down
    }
}


//
//  Called during start up to create our tab and any stuff we need.
//
tCIDLib::TVoid
TCQCTrayAppCtrlTab::CreateTab(TTabbedWnd& wndParent)
{
    //
    //  Create our server side object. Don't register it until we've
    //  successfully created the window
    //
    m_porbsAppCtrl = new TAppCtrlServerImpl(this);

    wndParent.c4CreateTab(this, strName());

    // Now register the object
    facCIDOrb().RegisterObject(m_porbsAppCtrl, tCIDLib::EAdoptOpts::Adopt);

    // And for auto-rebinding
    TString strBind = TAppCtrlServerBase::strImplBinding;
    strBind.eReplaceToken(facCQCTrayMon.strBinding(), kCIDLib::chLatin_b);
    facCIDOrbUC().RegRebindObj
    (
        m_porbsAppCtrl->ooidThis(), strBind, L"CQC Application Control Server"
    );

    // Create our activator window
    m_wndActivator.Create();
}




// ---------------------------------------------------------------------------
//  TCQCTrayAppCtrlTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Add an app control event object to the event queue. The queue is thread
//  safe, so we can just add it without worrying about sync.
//
tCIDLib::TVoid TCQCTrayAppCtrlTab::AddACEvent(TAppUtilEv* const paueToAdopt)
{
    if (m_colEventQ.bIsFull(128))
        delete paueToAdopt;
    else
        m_colEventQ.Add(paueToAdopt);
}


//
//  Add a status event object to the status queue. The queue is thread safe,
//  so we can just add it without worrying about sync.
//
tCIDLib::TVoid
TCQCTrayAppCtrlTab::
AddStatusEvent( const   TString&                strMoniker
                , const TString&                strTitle
                , const tCQCTrayMon::EListEvs   eEvent)
{
    if (!m_colStatusQ.bIsFull(64))
        m_colStatusQ.objAdd(TAppListChangeEv(strMoniker, strTitle, eEvent));
}

tCIDLib::TVoid
TCQCTrayAppCtrlTab::
AddStatusEvent( const   TString&                strMoniker
                , const tCQCTrayMon::EListEvs   eEvent)
{
    if (!m_colStatusQ.bIsFull(64))
        m_colStatusQ.objAdd(TAppListChangeEv(strMoniker, eEvent));
}

tCIDLib::TVoid TCQCTrayAppCtrlTab::AddStatusEvent(const TString& strStatusMsg)
{
    if (!m_colStatusQ.bIsFull(64))
        m_colStatusQ.objAdd(TAppListChangeEv(strStatusMsg));
}


//
//  Some things our event objects need to handle can only be done by the
//  foreground window. So we provide them with this method to call which
//  will force our invisible activator window forward.
//
tCIDLib::TVoid TCQCTrayAppCtrlTab::ForceToFront()
{
    facCIDCtrls().SetForeground(m_wndActivator.hwndThis(), kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TCQCTrayAppCtrlTab: Protected , inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCTrayAppCtrlTab::bCreated()
{
    TParent::bCreated();

    // Load up our dialog resource and create our contents within that
    tCIDCtrls::TWndId widInitFocus;

    TDlgDesc dlgdAppCtrl;
    facCQCTrayMon.bCreateDlgDesc(kCQCTrayMon::ridDlg_AppCtrl, dlgdAppCtrl);
    SetBgnColor(TDlgBox::rgbDlgBgn(dlgdAppCtrl.eTheme()));
    CreateDlgItems(dlgdAppCtrl, widInitFocus);

    // Get typed pointers to some of them
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_AppName, m_pwndAppName);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_AppPath, m_pwndAppPath);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_EvLog, m_pwndEvLog);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_Dir, m_pwndDir);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_List, m_pwndList);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_Moniker, m_pwndMoniker);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_Params, m_pwndParams);
    CastChildWnd(*this, kCQCTrayMon::ridDlg_AppCtrl_Status, m_pwndStatus);


    // Set up the columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(facCQCTrayMon.strMsg(kTrayMonMsgs::midDlg_AppCtrl_MonTitle));
    colCols.objAdd(facCQCTrayMon.strMsg(kTrayMonMsgs::midDlg_AppCtrl_AppTitle));
    m_pwndList->SetColumns(colCols);

    // Register handlers
    m_pwndList->pnothRegisterHandler(this, &TCQCTrayAppCtrlTab::eLBHandler);

    // Start up the timer that watches for events from the server object
    m_tmidUpdate = tmidStartTimer(200);

    return kCIDLib::True;
}



tCIDLib::TVoid TCQCTrayAppCtrlTab::Destroyed()
{
    // Kill our update timer
    StopTimer(m_tmidUpdate);

    //
    //  Destroy our activator window. It's not an owned window or a child
    //  window, so we need to clean it up.
    //
    m_wndActivator.Destroy();

    // And pass it on
    TParent::Destroyed();
}


tCIDLib::TVoid TCQCTrayAppCtrlTab::Timer(const tCIDCtrls::TTimerId tmidSrc)
{
    // If not our timer, pass it to our parent and return
    if (tmidSrc != m_tmidUpdate)
    {
        TParent::Timer(m_tmidUpdate);
        return;
    }

    //
    //  While there are events in the status queue, pull them out and process
    //  them. The queue is thread safe, so we don't have to worry about
    //  synchronization. We use a very small timeout since we only care to
    //  process events already in the queue.
    //
    if (!m_colStatusQ.bIsEmpty())
    {
        try
        {
            TAppListChangeEv aceCur;
            while (m_colStatusQ.bGetNext(aceCur, 5))
            {
                // Process it based on the event type
                switch(aceCur.eEvent())
                {
                    case tCQCTrayMon::EListEvs::Add :
                    {
                        // Set up the message we'll show in the event window
                        m_strEvTmp.LoadFromMsg
                        (
                            kTrayMonMsgs::midEvLog_AddedApp
                            , facCQCTrayMon
                            , aceCur.strMoniker()
                            , aceCur.strTitle()
                        );

                        // And add an item to the active apps window
                        tCIDLib::TStrList colVals(2);
                        colVals.objAdd(aceCur.strMoniker());
                        colVals.objAdd(aceCur.strTitle());
                        m_pwndList->c4AddItem(colVals, 0);
                        break;
                    }

                    case tCQCTrayMon::EListEvs::RecConfig :
                    {
                        // Set up the message we'll show in the event window
                        m_strEvTmp.LoadFromMsg
                        (
                            kTrayMonMsgs::midEvLog_ReconApp
                            , facCQCTrayMon
                            , aceCur.strMoniker()
                            , aceCur.strTitle()
                        );

                        //
                        //  If this is the selected item, then force an update
                        //  of the status info.
                        //
                        tCIDLib::TCard4 c4CurInd = m_pwndList->c4CurItem();
                        if (c4CurInd != kCIDLib::c4MaxCard)
                        {
                            TString strMon;
                            m_pwndList->QueryColText(c4CurInd, 0, strMon);

                            if (strMon == aceCur.strMoniker())
                                m_pwndList->SelectByIndex(c4CurInd, kCIDLib::True);
                        }
                        break;
                    }

                    case tCQCTrayMon::EListEvs::Remove :
                    {
                        m_strEvTmp.LoadFromMsg
                        (
                            kTrayMonMsgs::midEvLog_RemovedApp
                            , facCQCTrayMon
                            , aceCur.strMoniker()
                            , aceCur.strTitle()
                        );

                        // Remove the item from the list
                        const tCIDLib::TCard4 c4Index = m_pwndList->c4FindByText
                        (
                            aceCur.strMoniker()
                        );

                        if (c4Index != kCIDLib::c4MaxCard)
                            m_pwndList->RemoveAt(c4Index);
                        break;
                    }


                    case tCQCTrayMon::EListEvs::Status :
                    {
                        // It just uses the title string for the message
                        m_strEvTmp = aceCur.strTitle();
                        break;
                    }

                    default :
                        m_strEvTmp.Clear();
                        break;
                };

                // If we got something, then add a message to the event log window
                if (!m_strEvTmp.bIsEmpty())
                {
                    m_strEvTmp.Append(L"\n");
                    m_pwndEvLog->AddString(m_strEvTmp);
                }
            }
        }

        catch(TError& errToCatch)
        {
            if (m_loglimLoop.bLogErr(errToCatch, CID_FILE, CID_LINE))
                TModule::LogEventObj(m_errACLoopExcept);
        }

        catch(...)
        {
            m_loglimLoop.bLogErr(m_errACLoopExcept);
        }
    }


    //
    //  While there are events in the event queue, pull them out and process
    //  them. The queue is thread safe, so we don't have to worry about
    //  synchronization. We use a very small timeout since we only care to
    //  process events already in the queue.
    //
    //  As a safety measure, limit the number of loops we do each time
    //
    TAppUtilEv* paueCur = nullptr;
    if (!m_colEventQ.bIsEmpty())
    {
        tCIDLib::TCard4 c4Count = 0;
        while (c4Count < 16)
        {
            try
            {
                // Wait for the next event for just a little bit
                c4Count++;
                paueCur = m_colEventQ.pobjGetNext(5, kCIDLib::False);

                // If no more, then break out
                if (!paueCur)
                    break;

                // Put a janitor on it to make sure it's cleaned up
                TJanitor<TAppUtilEv> janEv(paueCur);

                // And process it
                paueCur->ProcessEv(*this, *m_porbsAppCtrl);
            }

            catch(TError& errToCatch)
            {
                if (m_loglimLoop.bLogErr(errToCatch, CID_FILE, CID_LINE))
                    TModule::LogEventObj(m_errACLoopExcept);
            }

            catch(...)
            {
                m_loglimLoop.bLogErr(m_errACLoopExcept);
            }
        }
    }

    //
    //  Check to see if we need to update the status of the currently
    //  selected item, if any.
    //
    try
    {
        tCIDLib::TCard4 c4CurItem = m_pwndList->c4CurItem();
        if (c4CurItem != kCIDLib::c4MaxCard)
        {
            TString strMon;
            m_pwndList->QueryColText(c4CurItem, 0, strMon);

            // Lock, lock up the item, and update the status
            TLocker lockrSync(&m_porbsAppCtrl->mtxSync());
            TAppCtrlRec* pacrSel = m_porbsAppCtrl->pacrFind(strMon);

            // Could have disappeared, so check to make sure
            if (pacrSel)
            {
                const tCIDLib::TBoolean bNewState = pacrSel->bIsRunning();
                if (bNewState != m_bLastStatus)
                {
                    m_bLastStatus = bNewState;
                    if (bNewState)
                        m_pwndStatus->strWndText(L"Running");
                    else
                        m_pwndStatus->strWndText(L"Stopped");
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        if (m_loglimLoop.bLogErr(errToCatch, CID_FILE, CID_LINE))
            TModule::LogEventObj(m_errACLoopExcept);
    }

    catch(...)
    {
        m_loglimLoop.bLogErr(m_errACLoopExcept);
    }
}


// ---------------------------------------------------------------------------
//  TCQCTrayAppCtrlTab: Priviate, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TCQCTrayAppCtrlTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    // We only care about cursor events and if the list is cleared
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Update the info for the selected item
        UpdateSelItemInfo();
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Just empty the current item status info
        m_pwndAppName->ClearText();
        m_pwndAppPath->ClearText();
        m_pwndDir->ClearText();
        m_pwndMoniker->ClearText();
        m_pwndParams->ClearText();
        m_pwndStatus->ClearText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCTrayAppCtrlTab::UpdateSelItemInfo()
{
    // Get the selected item
    tCIDLib::TCard4 c4CurInd = m_pwndList->c4CurItem();
    if (c4CurInd != kCIDLib::c4MaxCard)
    {
        try
        {
            TString strMon;
            m_pwndList->QueryColText(c4CurInd, 0, strMon);

            //
            //  Lock so that the item cannot change while we are doing this,
            //  then get the selected item and update the status items.
            //
            TLocker lockrSync(&m_porbsAppCtrl->mtxSync());
            TAppCtrlRec* pacrSel = m_porbsAppCtrl->pacrFind(strMon);

            // Make sure it hasn't gone away
            if (pacrSel)
            {
                // Break out the path/name parts
                m_pwndAppName->strWndText(pacrSel->strAppName());
                m_pwndAppPath->strWndText(pacrSel->strAppPath());
                m_pwndDir->strWndText(pacrSel->strWorkingDir());
                m_pwndMoniker->strWndText(pacrSel->strMoniker());
                m_pwndParams->strWndText(pacrSel->strParams());

                m_bLastStatus = pacrSel->bIsRunning();
                if (m_bLastStatus)
                    m_pwndStatus->strWndText(L"Running");
                else
                    m_pwndStatus->strWndText(L"Stopped");
            }
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        catch(...)
        {
        }
    }
}



