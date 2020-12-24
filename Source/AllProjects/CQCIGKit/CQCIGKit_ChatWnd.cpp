//
// FILE NAME: CQCIGkit_ChatWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2014
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
//  This file implements the TCQCChatWnd class, which supports the chat interface
//  that the remote port server exposes.
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
#include    "CQCIGKit_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCChatWnd,TFrameWnd)




// ---------------------------------------------------------------------------
//  CLASS: TCQCChatWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCChatWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCChatWnd::TCQCChatWnd() :

    m_bCommStatus(kCIDLib::False)
    , m_bLastCommStatus(kCIDLib::False)
    , m_bShowErrs(kCIDLib::False)
    , m_pwndClearMsgs(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndMsgList(nullptr)
    , m_pwndNewMsg(nullptr)
    , m_pwndStatus(nullptr)
    , m_pwndTransmit(nullptr)
    , m_rgbHis(facCIDGraphDev().rgbBlack)
    , m_rgbMine(facCIDGraphDev().rgbRed)
    , m_strConnText(kIGKitMsgs::midStatus_CommGood, facCQCIGKit())
    , m_strDisconnText(kIGKitMsgs::midStatus_CommBad, facCQCIGKit())
    , m_thrSpool
      (
          L"CMLDriverIDEChatThread"
          , TMemberFunc<TCQCChatWnd>(this, &TCQCChatWnd::eSpoolThread)
      )
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TCQCChatWnd::~TCQCChatWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCChatWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCChatWnd::Create( const   TWindow&    wndOwner
                                    , const TString&    strStateKey)
{
    m_strStateKey = strStateKey;

    // Load the dialog description that we us to create our contents
    TDlgDesc dlgdMain;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_ChatWnd, dlgdMain);

    // Figure out the required size for our frame to fit that content
    TArea areaInit;
    AreaForClient
    (
        dlgdMain.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::HelperFrame
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    //
    //  Load any previously stored frame state. Provide the initial size as the minimim
    //  size so it will adjust in case fonts have changed.
    //
    TCQCFrameState fstChat;
    facCQCGKit().bReadFrameState(m_strStateKey, fstChat, areaInit.szArea());

    // Take the previously stored (or defaulted if it couldn't find any) origin, but not size
    areaInit.SetOrg(fstChat.areaFrame().pntOrg());

    // Set the initial size as our min and max size, to prevent resize
    SetMinMaxSize(areaInit.szArea(), areaInit.szArea());

    // And create ourself. We are a top level owned frame
    CreateFrame
    (
        &wndOwner
        , areaInit
        , L"CQC Driver Test Harness Chat"
        , tCIDCtrls::EWndStyles::HelperFrame
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::StdFrame
        , kCIDLib::False
    );
}


//
//  The main window calls us here if the remote port interface is disabled. We clean ourself
//  up and disable the send button. We also call it ourself upon exit to make sure everything
//  gets cleaned up.
//
tCIDLib::TVoid TCQCChatWnd::DisableChat()
{
    // Disable the timer now since we are going down
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
    {
        StopTimer(m_tmidUpdate);
        m_tmidUpdate = kCIDCtrls::tmidInvalid;
    }

    // Stop the spooler thread
    try
    {
        if (m_thrSpool.bIsRunning())
        {
            m_thrSpool.ReqShutdownSync(5000);
            m_thrSpool.eWaitForDeath(8000);
        }
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // Clean up the proxy
    try
    {

        if (m_orbcChat.pobjData())
            m_orbcChat.DropRef();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    m_pwndClose->TakeFocus();
    m_pwndNewMsg->SetEnable(kCIDLib::False);
    m_pwndTransmit->SetEnable(kCIDLib::False);

    // Force us back to not connected status
    ResetConnStatus();
}


//
//  The main window calls us here if the remote port interface is enabled. We get
//  the address of the server. We set up the proxy and enable the send msgs and
//  turn on the timer that monitors the incoming msgs.
//
tCIDLib::TVoid TCQCChatWnd::EnableChat(const TIPEndPoint& ipepTar)
{
    // If the proxy exists, destroy it since the end point may have changed
    DisableChat();

    // Flush our msg deques
    m_colInMsgs.RemoveAll();
    m_colOutMsgs.RemoveAll();

    // ANd create a new one now
    try
    {
        m_orbcChat = facCQCRPortClient().orbcMakeChatProxy(ipepTar);

        //
        //  Reset our connection flags, so that we will go into showing not connected
        //  status until proven otherwise.
        //
        ResetConnStatus();

        // Kick off our timer since it worked
        m_tmidUpdate = tmidStartTimer(50);

        // And start up the spooler thread
        m_thrSpool.Start();

        m_pwndNewMsg->SetEnable(kCIDLib::True);
        m_pwndTransmit->SetEnable(kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }
}


// ---------------------------------------------------------------------------
//  TCQCChatWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We have multiple top level windows so we need to keep the active one set so that it
//  gets accelerator keys and such.
//
tCIDLib::TVoid
TCQCChatWnd::ActivationChange(  const   tCIDLib::TBoolean       bState
                                , const tCIDCtrls::TWndHandle   hwndOther)
{
    TParent::ActivationChange(bState, hwndOther);
    if (bState)
        facCIDCtrls().SetActiveTopWnd(this);
}


tCIDLib::TBoolean TCQCChatWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // Set our icon
    facCQCIGKit().SetFrameIcon(*this, L"DriverIDE");

    // We use a dialog resource to create our widgets
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_ChatWnd, dlgdChildren);

    // Create the children
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdChildren, widInitFocus);

    // Get some typed pointers to the relevant widgets
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_ClearMsgs, m_pwndClearMsgs);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_Close, m_pwndClose);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_MsgList, m_pwndMsgList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_NewMsg, m_pwndNewMsg);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_Status, m_pwndStatus);
    CastChildWnd(*this, kCQCIGKit::ridDlg_ChatWnd_Transmit, m_pwndTransmit);

    // Register any needed handlers
    m_pwndClearMsgs->pnothRegisterHandler(this, &TCQCChatWnd::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TCQCChatWnd::eClickHandler);
    m_pwndTransmit->pnothRegisterHandler(this, &TCQCChatWnd::eClickHandler);

    // Disable stuff until we are told we are active
    m_pwndNewMsg->SetEnable(kCIDLib::False);
    m_pwndTransmit->SetEnable(kCIDLib::False);

    // Set an initial not connected status
    ResetConnStatus();

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCChatWnd::Destroyed()
{
    // Stop our timer just in case
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    // Try to store our current position info for restore later
    facCQCGKit().StoreFrameState(*this, m_strStateKey);

    // And be sure to call our parent last
    TParent::Destroyed();
}


tCIDLib::TVoid TCQCChatWnd::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo == m_tmidUpdate)
    {
        // If the comm status has changed, then update our status display
        if (m_bCommStatus != m_bLastCommStatus)
        {
            m_bLastCommStatus = m_bCommStatus;
            if (m_bCommStatus)
            {
                m_pwndStatus->strWndText(m_strConnText);
                m_pwndTransmit->SetEnable(kCIDLib::True);
            }
             else
            {
                m_pwndStatus->strWndText(m_strDisconnText);
                m_pwndTransmit->SetEnable(kCIDLib::False);
            }
        }

        // If there are is a msg ready, grab it and output it
        if (m_colInMsgs.bPopTop(m_strTimerTmp))
        {
            m_pwndMsgList->AppendText(m_strTimerTmp, m_rgbHis);
            TAudio::AudioCue(tCIDLib::EAudioCues::Alert);
        }
    }
}


// ---------------------------------------------------------------------------
//  TCQCChatWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Handle button clicks. We shouldn't be able to get one of these if there's
//  any outstanding command, since the buttons would be disabled.
//
tCIDCtrls::EEvResponses TCQCChatWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ChatWnd_ClearMsgs)
    {
        m_pwndMsgList->Clear();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ChatWnd_Close)
    {
        // Stop and clean up everything, then destroy ourself
        DisableChat();
        Destroy();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_ChatWnd_Transmit)
    {
        // Get the text and clear the msg window
        TString strMsg = m_pwndNewMsg->strWndText();
        m_pwndNewMsg->ClearText();

        // If not empty then queue it up, pushing onto the bottom
        if (!strMsg.bIsEmpty())
        {
            m_colOutMsgs.objPushBottom(strMsg);

            // And output it to the msg list window
            m_pwndMsgList->AppendText(strMsg, m_rgbMine);
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}



//
//  The spooler thread. We constantly check for available outgoing msgs in the
//  out deqeuue and get incoming msgs and queue them up for the GUI thread to
//  handle.
//
tCIDLib::EExitCodes TCQCChatWnd::eSpoolThread(TThread& thrThis, tCIDLib::TVoid*)
{
    thrThis.Sync();

    TError  errLast;
    TString strInMsg;
    TString strOutMsg;
    while(kCIDLib::True)
    {
        // If we are asked to stop, then do so
        if (thrThis.bCheckShutdownRequest())
            break;

        try
        {
            if (!m_colOutMsgs.bPopTop(strOutMsg))
                strOutMsg.Clear();
            if (m_orbcChat->bExchange(strOutMsg, strInMsg))
                m_colInMsgs.objPushBottom(strInMsg);

            // Make sure we are in connected state since that worked
            m_bCommStatus = kCIDLib::True;
        }

        catch(TError& errToCatch)
        {
            if (m_bShowErrs && !errToCatch.bSameEvent(errLast))
            {
                strInMsg = L"[ERR: ";
                strInMsg.Append(errToCatch.strErrText());
                m_colInMsgs.objPushBottom(strInMsg);

                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                facCQCIGKit().LogEventObj(errToCatch);
            }

            m_bCommStatus = kCIDLib::False;
        }

        // Sleep a bit
        if (!thrThis.bSleep(TRemChatClientProxy::c4PollSeconds * 1000))
            break;
    }
    return tCIDLib::EExitCodes::Normal;
}


tCIDLib::TVoid TCQCChatWnd::ResetConnStatus()
{
    m_bCommStatus = kCIDLib::False;
    m_bLastCommStatus = kCIDLib::False;
    m_pwndStatus->strWndText(m_strDisconnText);
}

