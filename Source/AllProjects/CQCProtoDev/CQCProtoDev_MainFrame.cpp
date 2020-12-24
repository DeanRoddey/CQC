//
// FILE NAME: CQCProtoDev_MainFrame.cpp
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
RTTIDecls(TCQCProtoDevFrameWnd,TFrameWnd)



// ---------------------------------------------------------------------------
//  CLASS: TCQCProtoDevFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCProtoDevFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCProtoDevFrameWnd::TCQCProtoDevFrameWnd() :

    m_bLastTime(kCIDLib::False)
    , m_c4FldListId(0)
    , m_c4FldId(0)
    , m_c4FldSerialNum(0)
    , m_colOutputQ(tCIDLib::EMTStates::Safe)
    , m_eCurState(tCQCKit::EDrvStates::Count)
    , m_eDefVerbosity(tCQCKit::EVerboseLvls::Off)
    , m_pfactRemote(nullptr)
    , m_psdrvTest(nullptr)
    , m_pwndChangeFld(nullptr)
    , m_pwndClient(nullptr)
    , m_pwndCommRes(nullptr)
    , m_pwndConnection(nullptr)
    , m_pwndDbgOut(nullptr)
    , m_pwndFldList(nullptr)
    , m_pwndFldValue(nullptr)
    , m_pwndFldWrite(nullptr)
    , m_pwndMsgMapping(nullptr)
    , m_pwndPollEvents(nullptr)
    , m_pwndRecBytes(nullptr)
    , m_pwndSendBytes(nullptr)
    , m_pwndStartStop(nullptr)
    , m_pwndStateMachine(nullptr)
    , m_strmFmt(1024UL)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TCQCProtoDevFrameWnd::~TCQCProtoDevFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCProtoDevFrameWnd: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCProtoDevFrameWnd::Event(const   TString&        strTitle
                            , const TMemBuf&        mbufData
                            , const tCIDLib::TCard4 c4Bytes)
{
    static const TStreamFmt sfmtHexNum
    (
        2, 0, tCIDLib::EHJustify::Right, kCIDLib::chDigit0
    );

    m_strmFmt.Reset();
    m_strmFmt << strTitle;

    // Save the current format and set a new one
    TStreamJanitor janFmt(&m_strmFmt);
    m_strmFmt << sfmtHexNum;

    // We'll only display a max number
    tCIDLib::TCard4 c4Count = c4Bytes;
    if (c4Count > 32)
        c4Count = 32;
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_strmFmt   << TCardinal(mbufData[c4Index], tCIDLib::ERadices::Hex)
                    << TTextOutStream::Spaces(1);
    }

    // If we didn't display them all, then put an ellipsis at the end
    if (c4Count < c4Bytes)
        m_strmFmt << L"...";

    m_strmFmt.Flush();
    m_colOutputQ.objAdd(m_strmFmt.strData());
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Msg(const TString& strMsg)
{
    m_colOutputQ.objAdd(strMsg);
}

tCIDLib::TVoid TCQCProtoDevFrameWnd::Msg(const  TString&    strMsg
                                        , const TString&    strMsg2)
{
    m_colOutputQ.objAdd(strMsg + strMsg2);
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Msg(const  TString&    strMsg
                                        , const TString&    strMsg2
                                        , const TString&    strMsg3)
{
    m_colOutputQ.objAdd(strMsg + strMsg2 + strMsg3);
}


tCIDLib::TVoid
TCQCProtoDevFrameWnd::StateTransition(  const   tCIDLib::TCard1 c1ByteReceived
                                        , const TString&        strAction
                                        , const TString&        strNewState)
{
    static const TStreamFmt sfmtHexNum(2, 0, tCIDLib::EHJustify::Right, kCIDLib::chDigit0);

    //
    //  We will only do this one to the std out in debug mode and if asked to
    //  show this info.
    //
    m_strmFmt.Reset();
    if (tCIDLib::bAllBitsOn(m_psdrvTest->eMsgToShow(), tGenProtoS::EDbgMsgs::StateMachine))
    {
        m_strmFmt << L"Byte: 0x";

        //
        //  Save the current format and set a new one, and write out the
        //  byte using that format. Then we'll pop it off and display the
        //  rest of the info.
        //
        {
            TStreamJanitor janFmt(&m_strmFmt);
            m_strmFmt   << sfmtHexNum
                        << TCardinal(c1ByteReceived, tCIDLib::ERadices::Hex);
        }

        m_strmFmt << L" " << strAction << L" " << strNewState;
        m_strmFmt.Flush();

        m_colOutputQ.objAdd(m_strmFmt.strData());
    }
}


// ---------------------------------------------------------------------------
//  TCQCProtoDevFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCProtoDevFrameWnd::Create()
{
    //
    //  Force the driver facility to force in a facility object, since we
    //  don't call the standard factory method that CQCServer does.
    //
    MakeGenProtoSFac();

    // Load up the configuration data
    TPoint pntOrg(facCIDCtrls().areaDefWnd().pntOrg());
    try
    {
        tCIDLib::TCard4         c4Ver;
        tCIDLib::ELoadRes  eRes;

        // Load the main frame's origin and set it
        c4Ver = 0;
        eRes = facCQCGKit().eReadStoreObj
        (
            kCQCProtoDev::pszCfgKey_Frame, c4Ver, pntOrg, kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_StateLoad)
            , errToCatch
        );
    }

    //
    //  Figure out our size. Let's load the dialog description that we use and get
    //  the size of a frame that would fit that.
    //
    TDlgDesc dlgdMain;
    facCQCProtoDev.bCreateDlgDesc(kProtoDev::ridMain, dlgdMain);

    // Calculate the required frame size to fit this content
    TArea areaInit;
    AreaForClient
    (
        dlgdMain.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::True
    );
    areaInit.SetOrg(pntOrg);

    // If we aren't letting the system position us, then make sure our area is still good
    if (pntOrg != facCIDCtrls().areaDefWnd().pntOrg())
        facCIDCtrls().ValidatePrevWindowArea(areaInit);

    // Try to create the main frame. We are initially invisible
    facCIDCtrls().SetPointer(tCIDCtrls::ESysPtrs::AppStarting);
    tCIDLib::TCardList fcolAreas;
    TParent::CreateFrame
    (
        nullptr
        , areaInit
        , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );

    //
    //  Initially disable the session close and any tools that are only
    //  available when a session is open.
    //
    TMenuBar& menuBar = menuCur();
    menuBar.SetItemEnable(kProtoDev::ridMenu_Session_Close, kCIDLib::False);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_PackDriver, kCIDLib::False);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_V2Validation, kCIDLib::False);

    // Initially check the Off verbosity level
    TSubMenu menuVerb(menuBar, kProtoDev::ridMenu_Tools_Verbosity);
    menuVerb.SetItemCheck(kProtoDev::ridMenu_Tools_Verbose_Off, kCIDLib::True, kCIDLib::True);

    // Now we can show our window
    facCIDCtrls().ShowGUI(*this);

    // Set our current size as our minimum and maximum size
    SetMinMaxSize(areaInit.szArea(), areaInit.szArea());
}


// ---------------------------------------------------------------------------
//  TCQCProtoDevFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// Check if it's ok to shut down, and stop any session if allowed
tCIDLib::TBoolean TCQCProtoDevFrameWnd::bAllowShutdown()
{
    TYesNoBox msgbTest(facCQCProtoDev.strMsg(kProtoDevMsgs::midQ_Exit));
    if (!msgbTest.bShowIt(*this))
            return kCIDLib::False;

    // Close any open session. If we do, save our current state
    if (bCloseCurrent())
        SaveState();

    return kCIDLib::True;
}


tCIDLib::TBoolean TCQCProtoDevFrameWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Load our menu before we create stuff
    SetMenuBar(facCQCProtoDev, kProtoDev::ridMenu_Main);

    // We load our child widgets from a dialog description
    TDlgDesc dlgdMain;
    facCQCProtoDev.bCreateDlgDesc(kProtoDev::ridMain, dlgdMain);

    m_pwndClient = new TGenericWnd();
    m_pwndClient->CreateGenWnd
    (
        *this
        , areaClient()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    // Create the controls as children of the client
    tCIDCtrls::TWndId widInitFocus;
    m_pwndClient->PopulateFromDlg(dlgdMain, widInitFocus);

    // Get some typed pointers to some of our widgets
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_ChangeFld, m_pwndChangeFld);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_CommRes, m_pwndCommRes);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_Connection, m_pwndConnection);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_Output, m_pwndDbgOut);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_DrvState, m_pwndDrvState);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_FldList, m_pwndFldList);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_FldValue, m_pwndFldValue);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_FldWrite, m_pwndFldWrite);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_MsgMapping, m_pwndMsgMapping);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_PollEvents, m_pwndPollEvents);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_RecBytes, m_pwndRecBytes);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_SendBytes, m_pwndSendBytes);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_StartStop, m_pwndStartStop);
    CastChildWnd(*m_pwndClient, kProtoDev::ridMain_StateMachine, m_pwndStateMachine);

    // Set our frame icon
    facCQCProtoDev.SetFrameIcon(*this, L"DriverIDE");

    //
    //  Initially check the message check boxes that we want to be one until
    //  they change them.
    //
    m_pwndCommRes->SetCheckedState(kCIDLib::True);
    m_pwndConnection->SetCheckedState(kCIDLib::True);
    m_pwndFldWrite->SetCheckedState(kCIDLib::True);

    // Set up event handlers
    m_pwndChangeFld->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndCommRes->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndConnection->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndFldWrite->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndMsgMapping->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndPollEvents->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndRecBytes->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndSendBytes->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndStartStop->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndStateMachine->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eClickHandler);
    m_pwndFldList->pnothRegisterHandler(this, &TCQCProtoDevFrameWnd::eLBHandler);

    // Set an initial focus
    m_pwndFldList->TakeFocus();

    // Start up the timer that pulls out the debug info to the out window
    m_tmidUpdate = tmidStartTimer(250);

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }
    TParent::Destroyed();
}


tCIDLib::TVoid
TCQCProtoDevFrameWnd::MenuCommand(  const   tCIDLib::TResId     ridItem
                                    , const tCIDLib::TBoolean   bChecked
                                    , const tCIDLib::TBoolean   bEnabled)
{
    if (ridItem == kProtoDev::ridMenu_Session_Close)
    {
        bCloseCurrent();
    }
     else if (ridItem == kProtoDev::ridMenu_Session_New)
    {
        DoNewSession();
    }
     else if (ridItem == kProtoDev::ridMenu_Session_Exit)
    {
        if (bAllowShutdown())
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
    }
     else if (ridItem == kProtoDev::ridMenu_Tools_PackDriver)
    {
        PackageDriver();
    }
     else if ((ridItem >= kProtoDev::ridMenu_Tools_Verbose_Off)
          &&  (ridItem <= kProtoDev::ridMenu_Tools_Verbose_High))
    {
        //
        //  Set the verbosity level of the driver. The items in the submenu
        //  are in their natural enum order so we can just sub the first value
        //  and cast to the enum.
        //
        tCQCKit::EVerboseLvls eLvl = tCQCKit::EVerboseLvls
        (
            ridItem - kProtoDev::ridMenu_Tools_Verbose_Off
        );

        if (eLvl != m_eDefVerbosity)
        {
            m_eDefVerbosity = eLvl;

            TSubMenu menuVerbosity(menuCur(), kProtoDev::ridMenu_Tools_Verbosity);
            menuVerbosity.SetItemCheck(ridItem, kCIDLib::True, kCIDLib::True);

            //
            //  If the driver is now up and running, queue up a verbosity
            //  level command on it. We don't bother to wait for it. If
            //  we queue another before this one is processed, it will just
            //  overwrite the queued one anyway.
            //
            if (m_psdrvTest)
                m_psdrvTest->SetNewVerbosity(eLvl);
        }
    }
     else if (ridItem == kProtoDev::ridMenu_Tools_RemotePort)
    {
        RemotePort();
    }
     else if (ridItem == kProtoDev::ridMenu_Tools_V2Validation)
    {
        if (m_colFields.c4ElemCount() <= tCIDLib::c4EnumOrd(tCQCKit::EStdFields::Count))
        {
            //
            //  If we don't have any registered (driver specific) fields, then we
            //  aren't ready.
            //
            TOkBox msgbOK
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midV2Val_Title)
                , facCQCProtoDev.strMsg(kProtoDevMsgs::midV2Val_NotValReady)
            );
            msgbOK.ShowIt(*this);
        }
         else
        {
            tCIDLib::TStrList colErrors;
            if (m_cv2vValidator.bDoV2Validation(m_cqcdcTest, m_colFields, colErrors))
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
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // Shouldn't happen, but just in case
    if (tmidToDo != m_tmidUpdate)
        return;

    //
    //  While there are items in the output queue, pull them out and put them
    //  into the output window.
    //
    TString strNext;
    while (m_colOutputQ.bGetNextMv(strNext, 1))
        m_pwndDbgOut->AddString(strNext);

    //
    //  Update the value display for the selected field, if that is appropriate
    //  for our current circumstances.
    //
    {
        if (!m_psdrvTest)
        {
            if (m_bLastTime)
            {
                // Clear the last time state to represent the new current state
                m_bLastTime = kCIDLib::False;

                // Set the state to an invalid value to insure it's different
                m_eCurState = tCQCKit::EDrvStates::Count;

                // Clear the status display
                m_pwndDrvState->ClearText();
            }
            return;
        }

        m_bLastTime = kCIDLib::True;
    }

    try
    {
        //  Get the current state, and then react according to that, also
        //  going by what the last state was.
        //
        tCQCKit::EDrvStates eOldState = m_eCurState;
        m_eCurState = m_psdrvTest->eState();

        // Update the state display if it's changed
        if (m_eCurState != eOldState)
            m_pwndDrvState->strWndText(tCQCKit::strXlatEDrvStates(m_eCurState));

        switch(m_eCurState)
        {
            case tCQCKit::EDrvStates::Terminated :
            case tCQCKit::EDrvStates::WaitCommRes :
            case tCQCKit::EDrvStates::WaitConfig :
            case tCQCKit::EDrvStates::WaitConnect :
            case tCQCKit::EDrvStates::WaitInit :
                if (m_eCurState != eOldState)
                {
                    m_pwndFldValue->ClearText();
                    m_pwndChangeFld->SetEnable(kCIDLib::False);
                }
                break;

            case tCQCKit::EDrvStates::Connected :
            {
                if (m_eCurState != eOldState)
                    m_pwndChangeFld->SetEnable(kCIDLib::True);
                break;
            }

            default :
                break;
        };

        //
        //  If we have a currently selected item in the field list box, see if
        //  the value has changed. If so, update it. Note that we don't have
        //  to catch
        //
        tCIDLib::TCard4 c4ListInd = m_pwndFldList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            const TCQCFldDef& flddCur = m_colFields[m_pwndFldList->c4IndexToId(c4ListInd)];
            if (tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
            {
                TString strVal;
                tCQCKit::EFldTypes eType;
                if (m_psdrvTest->bReadField(m_c4FldListId
                                            , flddCur.c4Id()
                                            , m_c4FldSerialNum
                                            , strVal
                                            , eType
                                            , kCIDLib::False))
                {
                    m_pwndFldValue->strWndText(strVal);
                }
            }
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevErrs::errcMon_FailedUpdate)
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr
        (
            facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevErrs::errcMon_FailedUpdate)
        );
        msgbErr.ShowIt(*this);
    }
}


// ---------------------------------------------------------------------------
//  TCQCProtoDevFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCProtoDevFrameWnd::bCloseCurrent()
{
    //
    //  If we are currently running, ask if they want to stop. If not, then
    //  just return false, Else, stop and fall through to return true.
    //
    if (m_psdrvTest)
    {
        TYesNoBox msgbTest(facCQCProtoDev.strMsg(kProtoDevMsgs::midQ_CloseSession));

        if (!msgbTest.bShowIt(*this))
            return kCIDLib::False;
        Stop();
    }

    // Enable the new session and disable the close session
    TMenu& menuBar = menuCur();
    menuBar.SetItemEnable(kProtoDev::ridMenu_Session_Close, kCIDLib::False);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Session_New, kCIDLib::True);

    // Enable and disable tools as required
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_PackDriver, kCIDLib::False);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_RemotePort, kCIDLib::True);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_V2Validation, kCIDLib::False);

    // Disable the start/stop button
    m_pwndStartStop->SetEnable(kCIDLib::False);

    // Clear the protocol information display values
    m_pwndClient->pwndChildById(kProtoDev::ridMain_ProtoName)->ClearText();
    m_pwndClient->pwndChildById(kProtoDev::ridMain_MakeModel)->ClearText();
    m_pwndClient->pwndChildById(kProtoDev::ridMain_DispName)->ClearText();
    m_pwndClient->pwndChildById(kProtoDev::ridMain_ConnInfo)->ClearText();
    m_pwndClient->pwndChildById(kProtoDev::ridMain_Author)->ClearText();
    m_pwndFldList->RemoveAll();

    // Disable the field value setting button
    m_pwndDbgOut->SetEnable(kCIDLib::False);

    // Clear out the manifest file name
    m_strManifestFile.Clear();

    // Clear the debug output window
    m_pwndDbgOut->Clear();

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::ChangeField()
{
    // We should ahve a selected item, but be sure
    const tCIDLib::TCard4 c4ListInd = m_pwndFldList->c4CurItem();
    if (c4ListInd == kCIDLib::c4MaxCard)
    {
        TOkBox msgbNoSel(L"There is no selected field");
        msgbNoSel.ShowIt(*this);
        return;
    }

    // If the driver state isn't connected, we can't do it
    if (m_eCurState != tCQCKit::EDrvStates::Connected)
    {
        TOkBox msgbNoSel(L"The driver has to be connected before you can set field values");
        msgbNoSel.ShowIt(*this);
        return;
    }

    //
    //  Pop up the dialog box that just get's a text string from
    //  them. We'll use the field write method that generic clients
    //  would use, which takes a string and leaves it up to the
    //  field storage type to validate it.
    //

    const TString strTitle(kProtoDevMsgs::midDlg_GetValue_Title, facCQCProtoDev);
    TString strValue;
    if (!facCIDWUtils().bGetText(*this, strTitle, TString::strEmpty(), strValue))
        return;

    // Get the target field item from the list box
    const TCQCFldDef& flddCur = m_colFields[m_pwndFldList->c4IndexToId(c4ListInd)];

    //
    //  We use the generic text style field write scheme here, so that we
    //  can just pass in the string and CQCServer handles translating it
    //  to a valid format, or throwing if not possible.
    //
    try
    {
        // Queue up a field write
        TCQCServerBase::TDrvCmd* pdcmdWait = m_psdrvTest->pdcmdQWriteFld
        (
            flddCur.strName(), 0, 0, strValue, tCQCKit::EDrvCmdWaits::DontCare
        );

        // Wait for it to complete. Tell it to release the command when it completes
        TError errFail;
        if (!TCQCServerBase::bWaitCmd(pdcmdWait, errFail, kCIDLib::True))
            throw(errFail);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ErrInWrite)
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr
        (
            facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ErrInWrite)
        );
        msgbErr.ShowIt(*this);
    }
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::DoNewSession()
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

    // Set up a file types collection to set
    tCIDLib::TKVPList colFileTypes(1);
    colFileTypes.objAdd(TKeyValuePair(L"Driver Manifest Files", L"*.Manifest"));

    // And now get them to select a manifest file
    tCIDLib::TStrList colSel;
    tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , strWndText()
        , TString::strEmpty()
        , colSel
        , colFileTypes
        , tCIDCtrls::EFOpenOpts::FileMustExist
    );

    // If they bailed, so just return
    if (!bRes)
        return;

    // Else store the selection
    m_strManifestFile = colSel[0];

    //
    //  Let's try to parse this manifest file. The config object whose info
    //  it encodes knows how to parse this XML file and fill itself in from
    //  that content.
    //
    try
    {
        m_cqcdcTest.ParseFrom(new TFileEntitySrc(m_strManifestFile));

        //
        //  It's not done automatically, but we'll check here for any duplicate
        //  prompt keys.
        //
        const tCIDLib::TCard4 c4Count = m_cqcdcTest.c4PromptCount();
        tCIDLib::TStrHashSet colCheck(109, TStringKeyOps());
        tCIDLib::TBoolean bAdded;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TCQCDCfgPrompt& cqcdcpCur = m_cqcdcTest.cqcdcpAt(c4Index);
            colCheck.objAddIfNew(cqcdcpCur.strName(), bAdded);

            if (!bAdded)
            {
                facCQCProtoDev.ThrowErr
                (
                    CID_FILE
                    , CID_LINE
                    , kProtoDevErrs::errcMan_DupPromptKey
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
            *this
            , strWndText()
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_BadManifest)
            , errToCatch
        );
        return;
    }

    catch(...)
    {
        TErrBox msgbErr(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInManifest));
        msgbErr.ShowIt(*this);
        return;
    }

    // Make sure that it's a manifest for a generic protocol language driver
    if (m_cqcdcTest.eType() != tCQCKit::EDrvTypes::GenProto)
    {
        TErrBox msgbError(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_NotAProtoDriver));
        msgbError.ShowIt(*this);
        return;
    }

    // And the server lib is the protocol name
    m_pathProtoFile = m_cqcdcTest.strServerLibName();
    m_pathProtoFile.AppendExt(L"CQCProto");

    // Now ask them to get us to the protocol file
    colFileTypes.RemoveAll();
    colFileTypes.objAdd(TKeyValuePair(L"Protocol Description", L"*.CQCProto"));

    colSel.RemoveAll();
    bRes = facCIDCtrls().bOpenFileDlg
    (
        *this
        , strWndText()
        , TString::strEmpty()
        , colSel
        , colFileTypes
        , tCIDCtrls::EFOpenOpts
          (
            tCIDCtrls::EFOpenOpts::FileMustExist
          )
    );

    // If they bailed, so just return
    if (!bRes)
        return;

    // else store the selection
    m_pathProtoFile = colSel[0];

    // We got a protocol file and manifest, so run the drv install wizard
    {
        //
        //  First, refresh any port factories that might have configurable
        //  options, so that we will present the latest and greatest for
        //  com port selection.
        //
        facGC100Ser().bUpdateFactory(facCQCProtoDev.sectUser());
        facJAPwrSer().bUpdateFactory(facCQCProtoDev.sectUser());

        tCIDLib::TStrList colMons;
        bRes = facCQCTreeBrws().bDriverWizard
        (
            *this
            , L"localhost"
            , m_cqcdcTest
            , kCIDLib::False
            , colMons
            , nullptr
            , facCQCProtoDev.sectUser()
        );

        if (!bRes)
            return;
    }

    // Update the info display widgets with info we just got
    m_pwndClient->strChildText
    (
        kProtoDev::ridMain_ProtoName
        , m_cqcdcTest.strServerLibName()
    );

    m_pwndClient->strChildText
    (
        kProtoDev::ridMain_MakeModel
        , m_cqcdcTest.strMake() + TString(L"/") + m_cqcdcTest.strModel()
    );

    m_pwndClient->strChildText
    (
        kProtoDev::ridMain_DispName
        , m_cqcdcTest.strDisplayName()
    );

    m_pwndClient->strChildText
    (
        kProtoDev::ridMain_ConnInfo
        , m_cqcdcTest.conncfgReal().strConnInfo()
    );

    m_pwndClient->strChildText
    (
        kProtoDev::ridMain_Author
        , m_cqcdcTest.strAuthor()
    );

    // Enable the session close and disable new
    TMenuBar& menuBar = menuCur();
    menuBar.SetItemEnable(kProtoDev::ridMenu_Session_Close, kCIDLib::True);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Session_New, kCIDLib::False);

    // Enable and disable tools as required
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_RemotePort, kCIDLib::False);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_PackDriver, kCIDLib::True);
    menuBar.SetItemEnable(kProtoDev::ridMenu_Tools_V2Validation, kCIDLib::True);

    // Enable the Start/Stop button
    m_pwndStartStop->SetEnable(kCIDLib::True);
}


tCIDCtrls::EEvResponses
TCQCProtoDevFrameWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kProtoDev::ridMain_StartStop)
    {
        // If not running, then we call start, else we call stop.
        if (m_psdrvTest)
            Stop();
        else
            Start();
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_ChangeFld)
    {
        ChangeField();
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_CommRes)
    {
        if (m_psdrvTest)
        {
            if (m_pwndCommRes->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::CommRes);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::CommRes);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_Connection)
    {
        if (m_psdrvTest)
        {
            if (m_pwndConnection->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::Connect);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::Connect);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_FldWrite)
    {
        if (m_psdrvTest)
        {
            if (m_pwndFldWrite->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::FldWrite);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::FldWrite);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_MsgMapping)
    {
        if (m_psdrvTest)
        {
            if (m_pwndMsgMapping->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::MsgMatch);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::MsgMatch);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_PollEvents)
    {
        if (m_psdrvTest)
        {
            if (m_pwndPollEvents->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::PollEvents);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::PollEvents);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_RecBytes)
    {
        if (m_psdrvTest)
        {
            if (m_pwndRecBytes->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::ReceiveBytes);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::ReceiveBytes);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_SendBytes)
    {
        if (m_psdrvTest)
        {
            if (m_pwndSendBytes->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::SendBytes);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::SendBytes);
        }
    }
     else if (wnotEvent.widSource() == kProtoDev::ridMain_StateMachine)
    {
        if (m_psdrvTest)
        {
            if (m_pwndStateMachine->bCheckedState())
                m_psdrvTest->ShowMsgs(tGenProtoS::EDbgMsgs::StateMachine);
            else
                m_psdrvTest->HideMsgs(tGenProtoS::EDbgMsgs::StateMachine);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tGenProtoS::EDbgMsgs TCQCProtoDevFrameWnd::eCurMsgChecks() const
{
    // Build up the mask for the current msg display checks
    tGenProtoS::EDbgMsgs eRet = tGenProtoS::EDbgMsgs::None;

    if (m_pwndCommRes->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::CommRes);

    if (m_pwndConnection->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::Connect);

    if (m_pwndFldWrite->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::FldWrite);

    if (m_pwndMsgMapping->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::MsgMatch);

    if (m_pwndPollEvents->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::PollEvents);

    if (m_pwndRecBytes->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::ReceiveBytes);

    if (m_pwndSendBytes->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::SendBytes);

    if (m_pwndStateMachine->bCheckedState())
        eRet = tCIDLib::eOREnumBits(eRet, tGenProtoS::EDbgMsgs::StateMachine);

    return eRet;
}


tCIDCtrls::EEvResponses TCQCProtoDevFrameWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    // If no driver, then nothing to do
    if (!m_psdrvTest)
        return tCIDCtrls::EEvResponses::Handled;

    //
    //  Enable or disable our remove/info buttons according to whether
    //  a valid list box item is cursored or not.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // And update the display widgets with the selected field's info
        const tCIDLib::TCard4 c4ListSel = m_pwndFldList->c4CurItem();
        const TCQCFldDef& flddCur = m_colFields[m_pwndFldList->c4IndexToId(c4ListSel)];

        //
        //  If this field is write only, then just set the field to indicate
        //  this. Else, we just clear out the value field. We let the timer,
        //  which already has to do the logic to keep this widget up to
        //  date, just update it the next time through. This way we don't
        //  have to lock here.
        //
        if (!tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Read))
            m_pwndFldValue->strWndText(L"<Field is write-only>");
        else
            m_pwndFldValue->ClearText();

        //
        //  If the field is writeable and the driver is connected, enable the change
        //  button, else disable.
        //
        m_pwndChangeFld->SetEnable
        (
            tCIDLib::bAllBitsOn(flddCur.eAccess(), tCQCKit::EFldAccess::Write)
            && (m_eCurState == tCQCKit::EDrvStates::Connected)
        );

        //
        //  Zero the field serial number since we are on a new field now
        //  and need the first read to always get the current value.
        //
        m_c4FldSerialNum = 0;
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        //
        //  Get the selected item in the list box and invoke te field
        //  value change dialog for it.
        //
        ChangeField();
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        // Disable the change button and clear the field info widgets
        m_pwndChangeFld->SetEnable(kCIDLib::False);
        m_pwndFldValue->ClearText();
        m_c4FldSerialNum = 0;
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::PackageDriver()
{
    //
    //  Let them provide us with the target file name, which is will have
    //  a CQCDrvPack file extension.
    //
    TPathStr pathPackFlName;
    tCIDLib::TStrList colSel;
    {
        tCIDLib::TKVPList colFileTypes(1);
        colFileTypes.objAdd(TKeyValuePair(L"CQC Driver Pack", L"*.CQCDrvPack"));
        const tCIDLib::TBoolean bRes = facCIDCtrls().bOpenFileDlg
        (
            *this
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_SelectPackName)
            , TString::strEmpty()
            , colSel
            , colFileTypes
            , tCIDCtrls::EFOpenOpts::None
        );

        // If they bailed, so just return
        if (!bRes)
            return;
    }

    // else store the selection
    pathPackFlName = colSel[0];

    // If they didn't add an extension, then add one
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
            facCQCProtoDev.strMsg
            (
                kProtoDevMsgs::midQ_OverwriteDrvPack, pathPackFlName
            )
        );

        if (!msgbAsk.bShowIt(*this))
            return;
    }


    //
    //  Ok, it looks like they want to go forward. So create a package object
    //  to hold the package data. Inidcate that is a CML driver type package
    //  and set the version info. Mark it as having been created by this
    //  version of CQC.
    //
    TCQCPackage packNew(tCQCKit::EPackTypes::ProtocolDrv, kCQCKit::c8Ver_Current);

    // User the user data field to mark this package as User or System
    if (facCQCProtoDev.bSystem())
        packNew.strUserData(L"System");
    else
        packNew.strUserData(L"User");

    //
    //  First we have to set the manifest file. So go back and read in the
    //  manifest file. We set it as a binary blob, because it is XML and
    //  contains an embedded encoding so we want to keep it as is and not
    //  read/write it as text.
    //
    //  For driver type packages, the manifest file must be the first file
    //  added.
    //
    try
    {
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
            // We are throwing a CIDLib error here!
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
        //  Set this stuff as the mainfest file. We don't store the path
        //  since it will be meaningless, or the extension since we have an
        //  enum to indicate type, so get just the name out.
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
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_LoadManifest)
            , errToCatch
        );
        return;
    }

    //
    //  A sanity check, make sure the package type got set to to a protocol
    //  language based package when we set the manifest file.
    //
    #if CID_DEBUG_ON
    if (packNew.eType() != tCQCKit::EPackTypes::ProtocolDrv)
    {
        TErrBox msgbErr(L"The driver package type was not set to protocol language");
        msgbErr.ShowIt(*this);
    }
    #endif

    //
    //  And all that's left for this type of protocol language based driver
    //  package is the protocol file. So now read that in and add it as a
    //  file of this driver package.
    //
    try
    {
        // Load up the file contents
        TBinaryFile flSrc(m_pathProtoFile);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Excl_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        const tCIDLib::TCard4 c4Bytes = tCIDLib::TCard4(flSrc.c8CurSize());
        THeapBuf mbufProto(c4Bytes, c4Bytes);

        const tCIDLib::TCard4 c4Read = flSrc.c4ReadBuffer(mbufProto, c4Bytes);
        if (c4Read != c4Bytes)
        {
            // We are throwing a CIDLib error here!
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
        flSrc.Close();

        //
        //  Set this stuff as the manifest file. We don't store the path
        //  since it will be meaningless, or the extension since we have an
        //  enum to indicate type, so get just the name out.
        //
        TPathStr pathName(m_pathProtoFile);
        pathName.bRemovePath();
        pathName.bRemoveExt();
        packNew.c4AddFile
        (
            tCQCKit::EPackFlTypes::PDLFile
            , pathName
            , mbufProto
            , c4Bytes
            , kCIDLib::True
            , kCIDLib::False
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midMsg_Title)
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ProtoReadFailed)
            , errToCatch
        );
        return;
    }

    // Ok, we have it, so create the package
    TCQCPackage::MakePackage(pathPackFlName, packNew);

    // Ok, it's done, so let the user know
    TOkBox msgbDone(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_PackageDone));
    msgbDone.ShowIt(*this);
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::RemotePort()
{
    // Do the remote port configuration dialog
    tCIDLib::TBoolean bEnable = (m_pfactRemote != 0);
    TIPEndPoint ipepInit = facCIDSock().ipepDefLocalAddr
    (
        TRemSerialSrvClientProxy::ippnDefault
    );
    if (bEnable)
        ipepInit = m_pfactRemote->ipepServer();

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
        }
    }
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::SaveState()
{
    tCIDLib::TCard4 c4Version;

    // Create a cfg repository client proxy
    try
    {
        // Save the area of the main frame
        c4Version = 0;
        TArea areaSave;
        ePosState(areaSave);
        facCQCGKit().bAddUpdateStoreObj
        (
            kCQCProtoDev::pszCfgKey_Frame, c4Version, areaSave, 256
        );
    }

    catch(...)
    {
        // Ignore it, we just can't update the state
    }
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Start()
{
    tCIDLib::TCard4     c4SrcBytes;
    THeapBuf            mbufSrc;

    // Clean any content out of the debug output window
    m_pwndDbgOut->Clear();
    try
    {
        // Load up the file contents
        TBinaryFile flSrc(m_pathProtoFile);
        flSrc.Open
        (
            tCIDLib::EAccessModes::Excl_Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        c4SrcBytes = tCIDLib::TCard4(flSrc.c8CurSize());

        if (flSrc.c4ReadBuffer(mbufSrc, c4SrcBytes) != c4SrcBytes)
        {
            TErrBox msgbError(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ProtoReadFailed));
            msgbError.ShowIt(*this);
            return;
        }

        // Create the driver and put it into test mode
        m_psdrvTest = new TGenProtoSDriver(m_cqcdcTest);
        m_psdrvTest->bTestMode(kCIDLib::True);

        // Set the debug message mask to what our check boxes are set
        m_psdrvTest->ShowMsgs(eCurMsgChecks());

        // Ask it to try to parse the source
        m_psdrvTest->ParseProtocol(mbufSrc, c4SrcBytes);

        //
        //  We need to install ourself it's debug output handler so that he
        //  sends us output.
        //
        m_psdrvTest->SetDebugCallback(this);

        // Queue up a verbosity change. Say we don't want to wait
        m_psdrvTest->SetNewVerbosity(m_eDefVerbosity);

        //
        //  No exception was thrown, so we are off to the races. Let's init
        //  the driver. Normally this is done in the driver thread. In this
        //  case we want to just run it once and give up if it fails.
        //
        tCQCKit::EDrvInitRes eRes = m_psdrvTest->eInitialize();
        if (eRes == tCQCKit::EDrvInitRes::Failed)
        {
            delete m_psdrvTest;
            m_psdrvTest = 0;

            TErrBox msgbError(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_DrvInitFailed));
            msgbError.ShowIt(*this);
            return;
        }

        //
        //  Get the list of fields and use that to fill in the list box.
        //
        const tCIDLib::TCard4 c4Count = m_psdrvTest->c4QueryFields(m_colFields, m_c4FldListId);

        m_pwndFldList->RemoveAll();
        if (c4Count)
        {
            //
            //  We store the index into the original list in each item, so we can get
            //  back to the original data.
            //
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                m_pwndFldList->c4AddItem(m_colFields[c4Index].strName(), c4Index);

            m_pwndFldList->SelectByIndex(0, kCIDLib::True);
        }

        // It all worked, so go ahead and start up the driver thread
        m_psdrvTest->StartPoll();
    }

    catch(const TError& errToCatch)
    {
        delete m_psdrvTest;
        m_psdrvTest = 0;

        //
        //  If the error is from the parser, then display it directly in a
        //  message box. Else, show the error dialog.
        //
        if (errToCatch.strFacName() == facGenProtoS().strName()
        && ((errToCatch.errcId() > kGPDErrs::errcParse_FirstPErrNum)
        &&  (errToCatch.errcId() < kGPDErrs::errcParse_LastPErrNum)))
        {
            TErrBox msgbError(errToCatch.strErrText());
            msgbError.ShowIt(*this);
        }
         else
        {
            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInInit)
                , errToCatch
            );
        }
        return;
    }

    catch(...)
    {
        delete m_psdrvTest;
        m_psdrvTest = 0;

        TErrBox msgbErr(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInInit));
        msgbErr.ShowIt(*this);
        return;
    }

    // Change the text of the start button to say Stop now
    m_pwndStartStop->strWndText
    (
        facCQCProtoDev.strMsg(kProtoDevMsgs::midMain_Stop)
    );
}


tCIDLib::TVoid TCQCProtoDevFrameWnd::Stop()
{
    // If the driver doesn't exist, we shouldn't be called
    if (!m_psdrvTest)
        return;

    // Ask the driver to start shutting down
    try
    {
        m_psdrvTest->StartShutdown();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInTerm)
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInTerm));
        msgbErr.ShowIt(*this);
    }

    try
    {
        // Wait for it to become dead
        if (m_psdrvTest->bWaitTillDead(15000))
        {
            // It stopped so clean it up
            delete m_psdrvTest;
            m_psdrvTest = 0;
        }
         else
        {
            // Tell the user that it failed to stop
            TErrBox msgbErr(facCQCKit().strMsg(kKitMsgs::midStatus_DrvDidntStop));
            msgbErr.ShowIt(*this);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            *this
            , strWndText()
            , facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInDelete)
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr(facCQCProtoDev.strMsg(kProtoDevMsgs::midStatus_ExceptInDelete));
        msgbErr.ShowIt(*this);
    }

    // Change the text of the button back to Start again
    m_pwndStartStop->strWndText
    (
        facCQCProtoDev.strMsg(kProtoDevMsgs::midMain_Start)
    );

    // Put out a message that we've stopped it
    m_colOutputQ.objAdd(TString(L"The driver has been stopped"));
}


