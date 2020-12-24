//
// FILE NAME: CQCDrvDev_MainFrame.cpp
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
RTTIDecls(TCQCDrvDevFrameWnd,TMDIAppFrameWnd)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace CQCDrvDev_MainFrame
{
    namespace
    {
        // The names of the window states we use
        const TString   strState_Break(L"Break");
        const TString   strState_NoSession(L"NoSession");
        const TString   strState_Running(L"Running");
        const TString   strState_Stopped(L"Stopped");

        //
        //  The indices of the status bar slots we use. We have a dummy one first so that the
        //  first used one isn't right up against the left edge.
        //
        constexpr tCIDLib::TCard4 c4SlotInd_Dummy       = 0;
        constexpr tCIDLib::TCard4 c4SlotInd_CurLine     = 1;
        constexpr tCIDLib::TCard4 c4SlotInd_State       = 2;
        constexpr tCIDLib::TCard4 c4SlotInd_RO_RW       = 3;
        constexpr tCIDLib::TCard4 c4SlotInd_ClassPath   = 4;
        constexpr tCIDLib::TCard4 c4SlotInd_Count       = 5;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TCQCDrvDevFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvDevFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCDrvDevFrameWnd::TCQCDrvDevFrameWnd() :

    m_mecmToUse
    (
        this
        , L"CQC Driver Test Harness"
        , facCQCDrvDev.cuctxToUse()
        , kCIDLib::False
        , this
    )
    , m_colSkipWnds(tCIDLib::EAdoptOpts::NoAdopt)
    , m_pwndDbgClient(nullptr)
    , m_pfactRemote(nullptr)
    , m_strTitleText(kDrvDevMsgs::midMsg_Title, facCQCDrvDev)
{
}

TCQCDrvDevFrameWnd::~TCQCDrvDevFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvDevFrameWnd: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  If the client window contains the focus, let it have first crack. Else see if
//  it's ours.
//
tCIDLib::TBoolean
TCQCDrvDevFrameWnd::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    if (m_pwndDbgClient && m_pwndDbgClient->bContainsFocus())
    {
        if (m_pwndDbgClient->bProcessAccel(pMsgData))
            return kCIDLib::True;
    }
    return m_accelMain.bProcessAccel(*this, pMsgData);
}


//
//  We get callbacks here from the tree browser that gets popped up from our
//  interactive class manager that we install on the macro engine. This let's us
//  deal with changes that the user might make while in the dialog. We prevent
//  a rename of any files that are open, and we close any tabs for files that are
//  being deleted.
//
tCIDLib::TVoid TCQCDrvDevFrameWnd::TreeBrowserEvent(TTreeBrowseInfo& wnotEvent)
{
    switch(wnotEvent.eEvent())
    {
        case tCQCTreeBrws::EEvents::CanRename :
        case tCQCTreeBrws::EEvents::IsOpen :
        {
            //
            //  If we have this path open, we return true for isopen and false for
            //  canrename.
            //
            TString strClassPath;
            facCQCRemBrws().HPathToCMLClassPath(wnotEvent.strPath(), strClassPath);
            const tCIDLib::TBoolean bOpen = m_pwndDbgClient->bIsOpen(strClassPath);
            if (wnotEvent.eEvent() == tCQCTreeBrws::EEvents::IsOpen)
                wnotEvent.bFlag(bOpen);
            else
                wnotEvent.bFlag(!bOpen);
            break;
        }

        case tCQCTreeBrws::EEvents::Deleted :
        {
            //
            //  A file has been deleted. If we have it open, we need to close it and
            //  discard any changes
            //
            TString strClassPath;
            facCQCRemBrws().HPathToCMLClassPath(wnotEvent.strPath(), strClassPath);
            m_pwndDbgClient->ForceTabClosed(strClassPath);
            break;
        }

        case tCQCTreeBrws::EEvents::Edit :
        case tCQCTreeBrws::EEvents::View :
            //
            //  We can see these if the user double clicks, but we don't use them, and
            //  in the dialog embedded mode the browser window doesn't show Edit/View
            //  options on the popup menu. The double click just causes the dialog to
            //  end and return the clicked thing (if it's something valid to select.)
            //
            break;

        case tCQCTreeBrws::EEvents::NewFile :
        case tCQCTreeBrws::EEvents::NewScope :
        case tCQCTreeBrws::EEvents::Paste :
        case tCQCTreeBrws::EEvents::Renamed :
        case tCQCTreeBrws::EEvents::SelChanged :
            // These we'll legitimately get but don't use
            break;

        case tCQCTreeBrws::EEvents::Package :
        case tCQCTreeBrws::EEvents::ClientDrv :
        default :
            TErrBox msgbErr(L"The received browser event is not supported here");
            msgbErr.ShowIt(*this);
            break;
    };
}


// ---------------------------------------------------------------------------
//  TCQCDrvDevFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDrvDevFrameWnd::Create(const TCQCFrameState& fstMain)
{
    facCIDCtrls().SetPointer(tCIDCtrls::ESysPtrs::AppStarting);

    // Set up our accelerator table
    {
        tCIDLib::TStrList colKeyStrs;
        tCIDLib::TCardList fcolCmds;

        colKeyStrs.objAdd(L"Ctrl-Shift-N");
        fcolCmds.c4AddElement(kCQCDrvDev::ridMenu_Session_New);
        m_accelMain.Create(colKeyStrs, fcolCmds);
    }

    //
    //  Update our file resolver now. We CANNOT do it in the constructor because that
    //  happens before this information is available. What the base path is will depend
    //  potentially on a command line parm that allows for editing of system files. Else
    //  it is the usual user area of the macro section of the hierarchy.
    //
    m_mefrToUse.strBasePath(facCQCKit().strMacroRootPath());

    //
    //  Try to create the main frame, using the passed initial area. We are initially
    //  invisible until we get ready, to make it look smoother during startup.
    //
    CreateFrame
    (
        nullptr
        , fstMain.areaFrame()
        , m_strTitleText
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdSBFrame
        , kCIDLib::False
    );

    //
    //  Get the available client area. We have a status bar so we can't just call areaClient()
    //  which doesn't include things like that.
    //
    TArea areaClient;
    QueryClientArea(areaClient, kCIDLib::True);

    // Create the debugger window
    m_pwndDbgClient = new TMacroDbgMainWnd(&m_mecmToUse, &m_mefrToUse);
    m_pwndDbgClient->CreatePaneCont(*this, TArea(0, 0, 380, 280), kCIDCtrls::widFirstCtrl);

    //
    //  And now initialize the debugger. We load any previously stored state and pass that
    //  in.
    //
    // TBD, we don't really have any yet, but should go throught he motions at least
    TMacroDbgStateInfo stiInit;
    m_pwndDbgClient->Init(stiInit, &facCQCDrvDev.cuctxToUse());

    // Register for debugger events
    m_pwndDbgClient->pnothRegisterHandler(this, &TCQCDrvDevFrameWnd::eDbgHandler);

    // Create the mutex that we use to sync with the driver in sim mode
    TCQCGenDrvGlue::s_pmtxSync = new TMutex;

    //
    //  Create the field monitor window. Let's load any previously stored state info
    //  and pass that in.
    //
    m_wndFldMon.Create(this);

    // Enable validation code
    m_pwndDbgClient->meIDE().bValidation(kCIDLib::True);

    //
    //  This is kind of silly but apparently something to do with composited drawing and
    //  the status bar or something. But, until we pump some messages and do a size change,
    //  it will not show up and won't be seen when we get the available client area.
    //
    //  So we pump then size us up and back down a pixel. We aren't visible yet so it
    //  doesn't matter.
    //
    {
        facCIDCtrls().MsgYield(150);
        TArea areaNew = areaWnd();
        areaNew.Deflate(1, 1);
        SetSizePos(areaNew, kCIDLib::False);
        areaNew.Inflate(-1, -1);
        SetSizePos(areaNew, kCIDLib::False);
    }

    //
    //  Restore any previously stored position state, using the actually gotten frame
    //  size, in case it was set to default size. This will make sure we get back to
    //  maximized state if we were that way before, and it will show us.
    //
    RestorePosState(fstMain.ePosState(), areaWnd(), kCIDLib::True);
}


TMacroDbgMainWnd& TCQCDrvDevFrameWnd::wndDbgClient()
{
    return *m_pwndDbgClient;
}


// ---------------------------------------------------------------------------
//  TCQCDrvDevFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have multiple top level windows so we need to keep the active one set so that it
//  gets accelerator keys and such.
//
tCIDLib::TVoid
TCQCDrvDevFrameWnd::ActivationChange(const  tCIDLib::TBoolean       bState
                                    , const tCIDCtrls::TWndHandle   hwndOther)
{
    TParent::ActivationChange(bState, hwndOther);
    if (bState)
        facCIDCtrls().SetActiveTopWnd(this);
}


tCIDLib::TVoid
TCQCDrvDevFrameWnd::AreaChanged(const   TArea&                  areaPrev
                                , const TArea&                  areaNew
                                , const tCIDCtrls::EPosStates   ePosState
                                , const tCIDLib::TBoolean       bOrgChanged
                                , const tCIDLib::TBoolean       bSizeChanged
                                , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndDbgClient)
    {
        TArea areaAvail;
        QueryClientArea(areaAvail, kCIDLib::True, *m_pwndDbgClient);

        // Get the available client area and size the debugger window to fit
        if (bSizeChanged && bOrgChanged)
            m_pwndDbgClient->SetSizePos(areaAvail, kCIDLib::True);
        else if (bSizeChanged)
            m_pwndDbgClient->SetSize(areaAvail.szArea(), kCIDLib::True);
    }
}


tCIDLib::TBoolean TCQCDrvDevFrameWnd::bAllowShutdown()
{
    return bCheckShutdown();
}


tCIDLib::TBoolean TCQCDrvDevFrameWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Load up our resource based main menu
    SetMenuBar(facCQCDrvDev, kCQCDrvDev::ridMenu_Main);

    //  Set up our status bar slots.
    tCIDLib::TCardList fcolSlots(5UL);
    fcolSlots.c4AddElement(8);
    fcolSlots.c4AddElement(38);
    fcolSlots.c4AddElement(128);
    fcolSlots.c4AddElement(38);
    fcolSlots.c4AddElement(0);
    SetStatusBarAreas(fcolSlots, kCIDLib::False);

    //
    //  Create some window states. Note that remote chat is controlled by whether they have
    //  configured a remote connection or not. The remote connection (port) is allowed when
    //  they are stopped or no session.
    //
    {
        // Same as running, but can't do validation, has to be running for that
        TWndState wstateBreak(CQCDrvDev_MainFrame::strState_Break);
        wstateBreak.AddMenuEnable(kCQCDrvDev::ridMenu_Session_Close);

        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_V2Validation);
        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_EncPackage);
        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_Package);
        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_RemotePort);

        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_File_CloseAll);
        wstateBreak.AddMenuDisable(kCQCDrvDev::ridMenu_File_SaveAll);
        AddWndState(wstateBreak);
    }

    {
        TWndState wstateNoSess(CQCDrvDev_MainFrame::strState_NoSession);
        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_Session_Close);
        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_EncPackage);
        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_Package);
        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_V2Validation);

        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_File_CloseAll);
        wstateNoSess.AddMenuDisable(kCQCDrvDev::ridMenu_File_SaveAll);

        wstateNoSess.AddMenuEnable(kCQCDrvDev::ridMenu_Tools_RemotePort);
        AddWndState(wstateNoSess);
    }

    {
        TWndState wstateStopped(CQCDrvDev_MainFrame::strState_Stopped);

        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_Session_Close);
        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_Tools_EncPackage);
        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_Tools_Package);
        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_Tools_RemotePort);

        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_File_CloseAll);
        wstateStopped.AddMenuEnable(kCQCDrvDev::ridMenu_File_SaveAll);

        wstateStopped.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_V2Validation);
        AddWndState(wstateStopped);
    }

    {
        TWndState wstateRunning(CQCDrvDev_MainFrame::strState_Running);
        wstateRunning.AddMenuEnable(kCQCDrvDev::ridMenu_Session_Close);
        wstateRunning.AddMenuEnable(kCQCDrvDev::ridMenu_Tools_V2Validation);

        wstateRunning.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_EncPackage);
        wstateRunning.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_Package);
        wstateRunning.AddMenuDisable(kCQCDrvDev::ridMenu_Tools_RemotePort);

        wstateRunning.AddMenuDisable(kCQCDrvDev::ridMenu_File_CloseAll);
        wstateRunning.AddMenuDisable(kCQCDrvDev::ridMenu_File_SaveAll);

        AddWndState(wstateRunning);
    }

    // Initially set the no session state
    ApplyWndState(CQCDrvDev_MainFrame::strState_NoSession);

    // Initially check the Off verbosity level
    menuCur().SetItemCheck(kCQCDrvDev::ridMenu_Tools_Verbose_Off, kCIDLib::True);

    // And initially disable remote chat since it requires remote port setup
    menuCur().SetItemEnable(kCQCDrvDev::ridMenu_Tools_RemoteChat, kCIDLib::False);

    // Set our icon
    facCQCDrvDev.SetFrameIcon(*this, L"DriverIDE");

    return kCIDLib::True;
}


tCIDLib::TVoid
TCQCDrvDevFrameWnd::MenuCommand(const   tCIDLib::TResId     ridItem
                                , const tCIDLib::TBoolean   bChecked
                                , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == kCQCDrvDev::ridMenu_File_CloseAll)
    {
        // Pass on the debug window
        m_pwndDbgClient->CloseAll();
    }
     else if (ridItem == kCQCDrvDev::ridMenu_File_SaveAll)
    {
        // Pass on the debug window
        TString strErr;
        if (!m_pwndDbgClient->bSaveAll(strErr))
        {
            TErrBox msgbErr(strErr);
            msgbErr.ShowIt(*this);
        }
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Session_About)
    {
        // Show our generic about dialog
        facCQCIGKit().AboutDlg(*this);
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Session_Close)
    {
        bCloseCurrent();
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Session_New)
    {
        DoNewSession();
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Session_Exit)
    {
        if (bCheckShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if ((ridItem == kCQCDrvDev::ridMenu_Tools_Package)
          ||  (ridItem == kCQCDrvDev::ridMenu_Tools_EncPackage))
    {
        try
        {
            PackageDriver(ridItem == kCQCDrvDev::ridMenu_Tools_EncPackage);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TExceptDlg dlgErr
            (
                *this
                , m_strTitleText
                , facCQCKit().strMsg(kKitMsgs::midStatus_CantCreatePack)
                , errToCatch
            );
        }

        catch(...)
        {
            TErrBox msgbErr
            (
                m_strTitleText, facCQCKit().strMsg(kKitMsgs::midStatus_CantCreatePack)
            );
            msgbErr.ShowIt(*this);
        }
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Tools_RemoteChat)
    {
        //
        //  We shouldn't be callable unless the remote interface is enabled, but
        //  check just in case. If the window isn't already created, then create it
        //
        if (m_pfactRemote && !m_wndChat.bIsValid())
        {
            if (!m_wndChat.bIsValid())
                m_wndChat.Create(*this, kCQCDrvDev::strCfgKey_ChatState);

            // Tell it to enable, giving it the remote end point
            m_wndChat.EnableChat(m_pfactRemote->ipepServer());
        }
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Tools_RemotePort)
    {
        // Do the remote port configuration dialog
        tCIDLib::TBoolean bEnable = (m_pfactRemote != nullptr);

        //
        //  See if we have any last end point stored. If already enabled, use
        //  what's already there. If not, see if we have a stored one. Else
        //  default it.
        //
        tCIDLib::TCard4 c4Ver;
        TIPEndPoint     ipepInit;
        if (bEnable)
        {
            ipepInit = m_pfactRemote->ipepServer();
        }
         else
        {
            try
            {
                c4Ver = 0;
                const tCIDLib::ELoadRes eRes = facCQCGKit().eReadStoreObj
                (
                    kCQCDrvDev::strCfgKey_LastRemEP, c4Ver, ipepInit, kCIDLib::False
                );

                if (eRes != tCIDLib::ELoadRes::NewData)
                {
                    ipepInit = facCIDSock().ipepDefLocalAddr
                    (
                        TRemSerialSrvClientProxy::ippnDefault
                    );
                }
            }

            catch(...)
            {
                facCQCGKit().bDeleteStoreObj(kCQCDrvDev::strCfgKey_LastRemEP);
            }
        }

        if (facCQCIGKit().bCfgRemotePort(*this, bEnable, ipepInit))
        {
            // Deregister any remote port factory we have registered
            facCIDComm().DeregisterFactory(TRemSerialSrvClientProxy::strSerialFactoryId);
            m_pfactRemote = nullptr;

            // And now if they enabled, register a new one
            if (bEnable)
            {
                m_pfactRemote = new TCQCRemSrvPortFactory(ipepInit);
                facCIDComm().RegisterFactory(m_pfactRemote);

                // Store this as teh new last end point
                facCQCGKit().bAddUpdateStoreObj
                (
                    kCQCDrvDev::strCfgKey_LastRemEP, c4Ver, ipepInit, 16
                );

                // And enable the chat setup menu
                menuCur().SetItemEnable(kCQCDrvDev::ridMenu_Tools_RemoteChat, kCIDLib::True);

                // If the chat window is up, let it know about this
                if (m_wndChat.bIsValid())
                    m_wndChat.EnableChat(ipepInit);
            }
             else
            {
                // Disable the chat menu item
                menuCur().SetItemEnable(kCQCDrvDev::ridMenu_Tools_RemoteChat, kCIDLib::False);

                // If the chat window is up, let it know about this
                if (m_wndChat.bIsValid())
                    m_wndChat.DisableChat();
            }
        }
    }
     else if (ridItem == kCQCDrvDev::ridMenu_Tools_V2Validation)
    {
        //
        //  Make sure the drivers is connected. We have to get the current live set
        //  of fields, which are only available if connected.
        //
        tCQCKit::TFldDefList    colFlds;
        tCQCKit::EDrvStates     eState = tCQCKit::EDrvStates::NotLoaded;
        TCQCDriverObjCfg        cqcdcConfig;
        {
            TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);

            // If the driver is loaded, get its state
            if (TCQCGenDrvGlue::s_psdrvCurrent)
                eState = TCQCGenDrvGlue::s_psdrvCurrent->eState();

            // If it's beyond waiting for init, get the field list and config
            if (eState > tCQCKit::EDrvStates::WaitInit)
            {
                tCIDLib::TCard4 c4ListId;
                TCQCGenDrvGlue::s_psdrvCurrent->c4QueryFields(colFlds, c4ListId);
                TCQCGenDrvGlue::s_psdrvCurrent->QueryDrvConfigObj(cqcdcConfig);
            }
        }

        if (eState <= tCQCKit::EDrvStates::WaitInit)
        {
            //
            //  If the driver isn't at least past the init stage, or if no fields
            //  beyond the standard stats fields are registered, then nothing to
            //  do.
            //
            TOkBox msgbOK
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midV2Val_Title)
                , facCQCDrvDev.strMsg(kDrvDevMsgs::midV2Val_NotValReady)
            );
            msgbOK.ShowIt(*this);
        }
         else
        {
            tCIDLib::TStrList colErrors;
            if (m_cv2vValidator.bDoV2Validation(cqcdcConfig, colFlds, colErrors))
            {
                TOkBox msgbOK
                (
                    facCQCIGKit().strMsg(kIGKitMsgs::midV2Val_Title)
                    , facCQCIGKit().strMsg(kIGKitMsgs::midV2Val_NoValErrors)
                );
                msgbOK.ShowIt(*this);
            }
             else
            {
                facCIDWUtils().ShowList
                (
                    *this, facCQCIGKit().strMsg(kIGKitMsgs::midV2Val_Title), colErrors
                );
            }
        }
    }
     else if ((ridItem >= kCQCDrvDev::ridMenu_Tools_Verbose_Off)
          &&  (ridItem <= kCQCDrvDev::ridMenu_Tools_Verbose_High))
    {
        //
        //  Set the verbosity level of the driver. The items in the submenu
        //  are in their natural enum order so we can just sub the first value
        //  and cast to the enum.
        //
        tCQCKit::EVerboseLvls eLvl = tCQCKit::EVerboseLvls
        (
            ridItem - kCQCDrvDev::ridMenu_Tools_Verbose_Off
        );

        if (eLvl != TCQCGenDrvGlue::s_eDefVerbosity)
        {
            TCQCGenDrvGlue::s_eDefVerbosity = eLvl;
            TMenuBar& menuBar = menuCur();
            TSubMenu menuVerbosity(menuBar, kCQCDrvDev::ridMenu_Tools_Verbosity);
            menuVerbosity.SetItemCheck(ridItem, kCIDLib::True, kCIDLib::True);

            //
            //  Queue up a verbosity level command on the driver if it exists.
            //  Tell it we aren't going to wait on the result.
            //
            TLocker lockrSync(TCQCGenDrvGlue::s_pmtxSync);
            if (TCQCGenDrvGlue::s_psdrvCurrent)
                TCQCGenDrvGlue::s_psdrvCurrent->SetNewVerbosity(eLvl);
        }
    }
}


tCIDLib::TVoid TCQCDrvDevFrameWnd::Destroyed()
{
    // Save any window states for use next time
    SaveState();

    // Clean up our secondary frames if they got created
    if (m_wndFldMon.bIsValid())
        m_wndFldMon.Destroy();

    if (m_wndChat.bIsValid())
        m_wndChat.Destroy();

    // Shut down our class manager
    m_mecmToUse.Shutdown();

    // And be sure to call our parent last
    TParent::Destroyed();
}


// ---------------------------------------------------------------------------
//  TCQCDrvDevFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCDrvDevFrameWnd::bCloseCurrent()
{
    // Make sure that they want to close the current session if we have one
    if (m_pwndDbgClient->bSessionOpen())
    {
        //
        //  Ask if they want to close. Use braces to make sure it destructs
        //  before we move on, sine the close session call we make below
        //  will open another one.
        //
        {
            TYesNoBox msgbTest
            (
                m_strTitleText, facCQCDrvDev.strMsg(kDrvDevMsgs::midQ_CloseSession)
            );

            if (!msgbTest.bShowIt(*this))
                return kCIDLib::False;
        }

        // Ask the debugger to stop this existing session
        if (!m_pwndDbgClient->bCloseSession())
            return kCIDLib::False;
    }

    // Update our title bar to get rid of the macro name
    TString strTmp(m_strTitleText);
    strTmp.Append(L" [");
    strTmp.Append(L"]");
    strWndText(strTmp);


    // Set us up for no-session state
    ApplyWndState(CQCDrvDev_MainFrame::strState_NoSession);

    return kCIDLib::True;
}


//
//  Called from our own close menu item and from the virtual method that gets called if
//  the user requests a shutdown in any other way.
//
tCIDLib::TBoolean TCQCDrvDevFrameWnd::bCheckShutdown()
{
    // If no sesssion open, just say yes
    if (!m_pwndDbgClient->bSessionOpen())
        return kCIDLib::True;

    // If any changes, ask if they want to discard
    if (m_pwndDbgClient->bAnyChanges())
    {
        TYesNoBox msgbDiscard(m_strTitleText, L"There are changes, do you want to discard them?");
        if (!msgbDiscard.bShowIt(*this))
            return kCIDLib::False;
    }

    // Make sure the session is stopped
    m_pwndDbgClient->StopSession();

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCDrvDevFrameWnd::DoNewSession()
{
    // If we currently have a file open, then close it down
    if (!bCloseCurrent())
        return;

    //
    //  In this test harness, we don't open a class, we open a driver
    //  manifest file. We will parse this and then we'll use that info to
    //  know what class to open. Install a standard disk browser on the
    //  file open dialog.
    //

    // Get them to select a manifest file
    {
        // Set up a file types collection to set
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"Driver Manifest Files", L"*.Manifest"));

        tCIDLib::TStrList colFiles;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facCQCDrvDev.strMsg(kDrvDevMsgs::midMsg_SelManifest)
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

        m_strManifestFile = colFiles[0];
    }

    //
    //  Let's try to parse this manifest file. The config object whose info
    //  it encodes knows how to parse this XML file and fill itself in from
    //  that content.
    //
    TCQCDriverObjCfg cqcdcTmp;
    try
    {
        cqcdcTmp.ParseFrom(new TFileEntitySrc(m_strManifestFile));

        //
        //  It's not done automatically, but we'll check here for any duplicate
        //  prompt keys.
        //
        tCIDLib::TCard4 c4Count = cqcdcTmp.c4PromptCount();
        tCIDLib::TStrHashSet colCheck(109, TStringKeyOps());
        tCIDLib::TBoolean bAdded;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCDCfgPrompt& cqcdcpCur = cqcdcTmp.cqcdcpAt(c4Index);
            colCheck.objAddIfNew(cqcdcpCur.strName(), bAdded);

            if (!bAdded)
            {
                facCQCDrvDev.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kDrvDevErrs::errcMan_DupPromptKey
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Duplicate
                    , cqcdcpCur.strName()
                );
            }
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            *this
            , m_strTitleText
            , facCQCDrvDev.strMsg(kDrvDevMsgs::midStatus_BadManifest)
            , errToCatch
        );
        return;
    }

    catch(...)
    {
        TErrBox msgbErr
        (
            m_strTitleText
            , facCQCDrvDev.strMsg(kDrvDevMsgs::midStatus_ExceptInManifest)
        );
        msgbErr.ShowIt(*this);
        return;
    }

    // Make sure it's a macro based manifest file
    if (cqcdcTmp.eType() != tCQCKit::EDrvTypes::Macro)
    {
        TErrBox msgbError
        (
            m_strTitleText, facCQCDrvDev.strMsg(kDrvDevMsgs::midStatus_NotAMacroDriver)
        );
        msgbError.ShowIt(*this);
        return;
    }

    //
    //  And we can now get the main class out of the config object and try to open
    //  that class.
    //
    tCIDLib::EOpenRes eRes = tCIDLib::EOpenRes::ExistingFile;
    if (!m_mecmToUse.bMacroExists(cqcdcTmp.strServerLibName()))
    {
        //
        //  Make sure that it is under the root class path that we are supposed to
        //  limit them to.
        //
        TString strPath;
        facCQCRemBrws().RelPathToCMLClassPath(facCQCKit().strMacroRootClassScope(), strPath);
        if (!cqcdcTmp.strServerLibName().bStartsWith(strPath))
        {
            TErrBox msgbError
            (
                m_strTitleText
                , facCQCDrvDev.strMsg
                  (
                    kDrvDevMsgs::midStatus_NotInRootPath
                    , facCQCKit().strMacroRootClassScope()
                  )
            );
            msgbError.ShowIt(*this);
            return;
        }

        TYesNoBox msgbOpenNew
        (
            m_strTitleText
            , facCQCDrvDev.strMsg
              (
                kDrvDevMsgs::midQ_OpenNew, cqcdcTmp.strServerLibName()
              )
        );
        if (!msgbOpenNew.bShowIt(*this))
            return;

        // Change the result flag, so that we'll open a new file
        eRes = tCIDLib::EOpenRes::NewFile;
    }

    //
    //  We got the class, so let's run the driver install wizard to get
    //  the promp values and com configuration.
    //
    {
        //
        //  First, refresh any port factories that might have configurable
        //  options, so that we will present the latest and greatest for
        //  com port selection.
        //
        facGC100Ser().bUpdateFactory(facCQCDrvDev.cuctxToUse().sectUser());
        facJAPwrSer().bUpdateFactory(facCQCDrvDev.cuctxToUse().sectUser());

        // And invoke the wizard
        tCIDLib::TStrList colMons;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bDriverWizard
        (
            *this
            , L"localhost"
            , cqcdcTmp
            , kCIDLib::False
            , colMons
            , nullptr
            , facCQCDrvDev.cuctxToUse().sectUser()
        );

        if (!bRes)
            return;
    }

    // The user committed to the install wizard, so let's start the new session
    m_pathCurClass = cqcdcTmp.strServerLibName();
    if (m_pwndDbgClient->bOpenSession(  m_pathCurClass
                                        , eRes == tCIDLib::EOpenRes::NewFile))
    {
        //
        //  Looks like we are going to do it. In order to support the driver
        //  simulation mode we use in this test harness, we have to set the
        //  driver config and moniker on statics of the C++ generic driver
        //  class, which is the glue code class that connects the driver
        //  system to the macro system.
        //
        TCQCGenDrvGlue::s_cqcdcToLoad = cqcdcTmp;

        // Set any dynamic type reference from the config onto the engine
        m_pwndDbgClient->meIDE().strSpecialDynRef(cqcdcTmp.strDynRef());

        // Update our title bar with the new class name
        TString strTmp(m_strTitleText);
        strTmp.Append(L" [");
        strTmp.Append(m_pathCurClass);
        strTmp.Append(L"]");
        strWndText(strTmp);

        // Set us up for stopped (session open but not running) state
        ApplyWndState(CQCDrvDev_MainFrame::strState_Stopped);
    }
}


//
//  We handle events from the debugger, so that we can update our menus appropriately.
//
tCIDCtrls::EEvResponses TCQCDrvDevFrameWnd::eDbgHandler(TCMLDbgChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::IDEState)
    {
        if (wnotEvent.strState() == L"")
            ApplyWndState(CQCDrvDev_MainFrame::strState_NoSession);
        else if (wnotEvent.strState() == L"Edit")
            ApplyWndState(CQCDrvDev_MainFrame::strState_Stopped);
        else if (wnotEvent.strState() == L"Run")
            ApplyWndState(CQCDrvDev_MainFrame::strState_Running);
        else if (wnotEvent.strState() == L"Break")
            ApplyWndState(CQCDrvDev_MainFrame::strState_Break);

        if (wnotEvent.strState() == L"")
        {
            // Clear the status bar entries since no session now
            SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_State, TString::strEmpty());
            SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_CurLine, TString::strEmpty());
            SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_RO_RW, TString::strEmpty());
            SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_ClassPath, TString::strEmpty());
        }
         else
        {
            // Set the new state
            SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_State, wnotEvent.strState());
        }
    }
     else if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::LineChanged)
    {
        m_strTmp.SetFormatted(wnotEvent.c4LineNum());
        SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_CurLine, m_strTmp);
    }
     else if (wnotEvent.eEvent() == tCIDMacroDbg::EEvents::NewFile)
    {
        // Update the RW/RO indicator in the status bar
        if (m_pwndDbgClient->eCurSrcMode() == tCIDMacroEng::EResModes::ReadOnly)
            m_strTmp = L"RO";
        else
            m_strTmp = L"RW";
        SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_RO_RW, m_strTmp);

        m_strTmp.SetFormatted(wnotEvent.c4LineNum());
        SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_CurLine, m_strTmp);
        SetStatusBarText(CQCDrvDev_MainFrame::c4SlotInd_ClassPath, wnotEvent.strCurFile());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the user invokes a macro from the browser window, this is called. We load it up
//  in the appropriate mode.
//
tCIDLib::TVoid
TCQCDrvDevFrameWnd::LoadFile(const  TTreeBrowseInfo&    wnotBrowser
                            , const tCIDLib::TBoolean   bEditMode)
{
    // Only do this if we have a session open
    if (!m_pwndDbgClient || !m_pwndDbgClient->bSessionOpen())
        return;

    //
    //  Let's get this guy loaded up. We get the full hierarchical path and need to convert
    //  that to a CML class path.
    //
    TString strClassPath;
    facCQCRemBrws().HPathToCMLClassPath(wnotBrowser.strPath(), strClassPath);

    // And just ask the debuffer engine to load it up
    m_pwndDbgClient->OpenNewFile(strClassPath, bEditMode);
}


//
//  Called when the user asks to package a driver up. We get the info from him and then
//  invoke the driver packaging support in CQCKit.
//
tCIDLib::TVoid TCQCDrvDevFrameWnd::PackageDriver(const tCIDLib::TBoolean bEncrypt)
{
    //
    //  Let them provide us with the target file name, which is will have
    //  a CQCDrvPack file extension.
    //
    TPathStr pathPackFlName;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Driver Pack", L"*.CQCDrvPack"));

        tCIDLib::TStrList colFiles;
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facCQCKit().strMsg(kKitMsgs::midMsg_SelectPackName)
            , TString::strEmpty()
            , colFiles
            , colFileTypes
            , tCIDCtrls::EFOpenOpts::StrictTypes
        );

        if (!bRes)
            return;

        pathPackFlName = colFiles[0];
    }

    //
    //  If they didn't add an extension, then add one. If they happen to just have a
    //  period in the file, that will look like an extension, so check it and add
    //  the right one if not.
    //
    TString strExt;
    if (!pathPackFlName.bQueryExt(strExt) || !strExt.bCompareI(L"CQCDrvPack"))
        pathPackFlName.AppendExt(L"CQCDrvPack");

    //
    //  If there is already a file there, then ask if they want to overwrite.
    //  If not, then do nothing.
    //
    if (TFileSys::bExists(pathPackFlName))
    {
        TYesNoBox msgbAsk
        (
            m_strTitleText
            , facCQCDrvDev.strMsg(kDrvDevMsgs::midQ_OverwriteDrvPack, pathPackFlName)
        );

        if (!msgbAsk.bShowIt(*this))
            return;
    }

    // Force a save of all of the files
    TString strErr;
    if (!m_pwndDbgClient->bSaveAll(strErr))
    {
        TErrBox msgbErr(strErr);
        msgbErr.ShowIt(*this);
        return;
    }

    //
    //  Ok, it looks like they want to go forward. So create a package object
    //  to hold the package data. Inidcate that is a CML driver type package
    //  and set the version info. ANd mark it as having been created by this
    //  version of CQC.
    //
    TCQCPackage packNew(tCQCKit::EPackTypes::CMLDrv, kCQCKit::c8Ver_Current);

    //
    //  Set the user data field to User or System, based on whether the
    //  main class' class path is a system or user section.
    //
    if (m_pwndDbgClient->strCurClassPath().bStartsWith(L"MEng.System"))
        packNew.strUserData(L"System");
    else
        packNew.strUserData(L"User");

    //
    //  First we have to set the manifest file, and indicate the min/max
    //  versions this driver package is for. So go back and read in the
    //  manifest file. We set it as a binary blob, because it is XML and
    //  contains an embedded encoding so we want to keep it as is and not
    //  read/write it as text.
    //
    //  The manifest file must be the first file added in a driver type
    //  package.
    //
    TBinaryFile flTmp(m_strManifestFile);
    flTmp.Open
    (
        tCIDLib::EAccessModes::Excl_Read
        , tCIDLib::ECreateActs::OpenIfExists
        , tCIDLib::EFilePerms::Default
        , tCIDLib::EFileFlags::SequentialScan
    );

    const tCIDLib::TCard4 c4Bytes = tCIDLib::TCard4(flTmp.c8CurSize());
    THeapBuf mbufManFl(c4Bytes, c4Bytes);
    const tCIDLib::TCard4 c4Read = flTmp.c4ReadBuffer(mbufManFl, c4Bytes);

    // If not all read, then freak out
    if (c4Read != c4Bytes)
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcFile_NotAllData
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotAllWritten
            , TCardinal(c4Bytes)
            , TCardinal(c4Read)
        );
    }
    flTmp.Close();

    //
    //  Set this stuff as the manifest file. We don't store the path
    //  since it will be meaningless, and extensions are never stored, so get
    //  just the name out.
    //
    TPathStr pathName(m_strManifestFile);
    pathName.bRemovePath();
    pathName.bRemoveExt();
    packNew.c4AddFile
    (
        tCQCKit::EPackFlTypes::Manifest
        , pathName
        , mbufManFl
        , c4Bytes
        , kCIDLib::False
        , kCIDLib::False
    );

    //
    //  A sanity check, make sure the package type got set to to a CML
    //  based package when we set the manifest file.
    //
    #if CID_DEBUG_ON
    if (packNew.eType() != tCQCKit::EPackTypes::CMLDrv)
    {
        TErrBox msgbErr
        (
            m_strTitleText
            , L"The driver package type was not set to macro based"
        );
        msgbErr.ShowIt(*this);
        return;
    }
    #endif


    try
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TDataSrvClient dsclLoad;

        //
        //  Get a list of CML classes in the same scope as the main driver class being
        //  edited. So get the main class path and convert it to a relative path.
        //
        tCIDLib::TStrList colClasses;
        TString strDrvRelPath;
        {
            facCQCRemBrws().CMLClassPathToRelPath(m_pwndDbgClient->strCurClassPath(), strDrvRelPath);
            tCIDLib::TCard4 c4ScopeId;

            // Get just items, no scopes
            facCQCRemBrws().bRemoveLastPathItem(strDrvRelPath);
            dsclLoad.bQueryScopeNames
            (
                c4ScopeId
                , strDrvRelPath
                , tCQCRemBrws::EDTypes::Macro
                , colClasses
                , tCQCRemBrws::EQSFlags::Items
                , facCQCDrvDev.cuctxToUse().sectUser()
            );
        }

        // OK, now iterate that list and put these into the package
        TString strContents;
        TString strRelPath;
        const tCIDLib::TCard4 c4ClsCnt = colClasses.c4ElemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ClsCnt; c4Index++)
        {
            // Convert the macro path to a relative hierarchical path
            strRelPath = strDrvRelPath;
            strRelPath.Append(kCIDLib::chForwardSlash);
            strRelPath.Append(colClasses[c4Index]);

            // And add this one to the package, using a helper provided for this work
            facCQCIGKit().AddFileToPack
            (
                dsclLoad
                , packNew
                , tCQCKit::EPackFlTypes::CMLMacro
                , strRelPath
                , tCQCRemBrws::EDTypes::Macro
                , facCQCDrvDev.cuctxToUse().sectUser()
            );
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgErr
        (
            *this
            , m_strTitleText
            , L"Failed to add CML class contents to the package"
            , errToCatch
        );
        return;
    }

    //
    //  Invoke the macro adder dialog. It will let them select CML classes to add to the
    //  package. Get the scope that the class is in and pass that as the initial path to
    //  expand.
    //
    TString strDrvHPath;
    facCQCRemBrws().CMLClassPathToHPath(m_pwndDbgClient->strCurClassPath(), strDrvHPath);
    facCQCRemBrws().bRemoveLastPathItem(strDrvHPath);
    const tCIDLib::TBoolean bSave = facCQCIGKit().bManagePackage
    (
        *this
        , packNew
        , strDrvHPath
        , bEncrypt
        , tCQCRemBrws::EDTypes::Macro
        , facCQCDrvDev.cuctxToUse()
    );

    if (bSave)
    {
        // Ok, create the package now
        TCQCPackage::MakePackage(pathPackFlName, packNew);

        // Ok, it's done, so let the user know
        TOkBox msgbDone
        (
            m_strTitleText, facCQCKit().strMsg(kKitMsgs::midStatus_PackageDone)
        );
        msgbDone.ShowIt(*this);
    }
}


tCIDLib::TVoid TCQCDrvDevFrameWnd::SaveState()
{
    tCIDLib::TCard4 c4Version;

    // Create a cfg repository client proxy
    try
    {
        //
        //  Save our main debugger state, which is currently just the frame
        //  area. But give it a little reserve space in case we add some more
        //  data to it, so that it won't have to be moved for just a little
        //  extension.
        //
        c4Version = 0;

        //
        //  Store our various window states. This helper will check to see if the window
        //  is valid before storing anything.
        //
        facCQCGKit().StoreFrameState(*this, kCQCDrvDev::strCfgKey_Frame);
        facCQCGKit().StoreFrameState(m_wndFldMon, kCQCDrvDev::strCfgKey_FldMonState);
        facCQCGKit().StoreFrameState(m_wndChat, kCQCDrvDev::strCfgKey_ChatState);

        //
        //  Query the debugger's current state info and store it. Again, leave
        //  some expansion space if we are creating it here.
        //
        TMacroDbgStateInfo stiSave;
        m_pwndDbgClient->QueryStateInfo(stiSave);
        c4Version = 0;
        facCQCGKit().bAddUpdateStoreObj
        (
            kCQCDrvDev::strCfgKey_DbgState, c4Version, stiSave, 256
        );
    }

    catch(TError& errToCatch)
    {
        // Oh well couldn't save it, but not much we can do about it but log
        if (facCQCDrvDev.bShouldLog(errToCatch))
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }
    }
}

