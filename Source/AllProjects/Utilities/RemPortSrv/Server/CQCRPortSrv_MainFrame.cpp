//
// FILE NAME: CQCRPortSrv_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
//  This file implements the main frame window.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCRPortSrv.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMainFrameWnd,TFrameWindow)


// ---------------------------------------------------------------------------
//  CLASS: TMainFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMainFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TMainFrameWnd::TMainFrameWnd() :

    m_bLastConn(kCIDLib::False)
    , m_porbsChat(nullptr)
    , m_porbsSerial(nullptr)
    , m_pwndAllowedPorts(nullptr)
    , m_pwndClient(nullptr)
    , m_pwndConnStatus(nullptr)
    , m_pwndEnable(nullptr)
    , m_pwndMsgList(nullptr)
    , m_pwndNewMsg(nullptr)
    , m_pwndPort(nullptr)
    , m_pwndSecKey(nullptr)
    , m_pwndTransmit(nullptr)
    , m_rgbHis(facCIDGraphDev().rgbBlack)
    , m_rgbMine(facCIDGraphDev().rgbRed)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TMainFrameWnd::~TMainFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMainFrameWnd::Create()
{
    TDlgDesc dlgdMain;
    facCQCRPortSrv().bCreateDlgDesc(kCQCRPortSrv::ridWnd_Main, dlgdMain);

    // Calculate the required frame size to fit this content
    TArea areaInit;
    AreaForClient
    (
        dlgdMain.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    // Let the system position it
    areaInit.SetOrg(facCIDCtrls().areaDefWnd().pntOrg());
    TParent::CreateFrame
    (
        nullptr
        , areaInit
        , facCQCRPortSrv().strTitle()
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );

    // Set this as our min size
    SetMinMaxSize(areaInit.szArea(), TSize());
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TMainFrameWnd::bAllowShutdown()
{
    // Make sure that they really want to exit
    TYesNoBox msgbAsk(L"Shut down the remote port server?");
    return msgbAsk.bShowIt(*this);
}


tCIDLib::TVoid
TMainFrameWnd::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Pass on to our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    //
    //  If the size changed and we are maximized or restored, then we need to update the
    //  client window for our new size. It'll reposition the controls appropriately.
    //
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized))
        m_pwndClient->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TMainFrameWnd::bCreated()
{
    // Call our parent class first
    TParent::bCreated();

    // Set our frame icon
    facCQCRPortSrv().SetFrameIcon(*this, L"RPortSrv");

    // Load the resource that we'll use to create the content
    TDlgDesc dlgdMain;
    facCQCRPortSrv().bCreateDlgDesc(kCQCRPortSrv::ridWnd_Main, dlgdMain);

    //
    //  Create our client initially the size of the dialog content. This insures that the
    //  auto-resize stuff works because we have the right initial relationship to our
    //  controls we will create.
    //
    //  Note that we create it initially invisible as well. We'll show it at the end
    //
    TArea areaInit(dlgdMain.areaPos());
    areaInit.ZeroOrg();

    m_pwndClient = new TGenericWnd();
    m_pwndClient->CreateGenWnd
    (
        *this
        , areaInit
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    // Create the controls as children of the client
    tCIDCtrls::TWndId widInitFocus;
    m_pwndClient->PopulateFromDlg(dlgdMain, widInitFocus);

    // Get some typed pointers to the ones we interact with a lot
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_AllowedPorts, m_pwndAllowedPorts);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_ConnStatus, m_pwndConnStatus);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_Enable, m_pwndEnable);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_MsgList, m_pwndMsgList);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_NewMsg, m_pwndNewMsg);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_Port, m_pwndPort);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_SecKey, m_pwndSecKey);
    CastChildWnd(*m_pwndClient, kCQCRPortSrv::ridWnd_Main_Transmit, m_pwndTransmit);

    // Install event handlers on those that need it
    m_pwndAllowedPorts->pnothRegisterHandler(this, &TMainFrameWnd::eLBHandler);
    m_pwndEnable->pnothRegisterHandler(this, &TMainFrameWnd::eClickHandler);
    m_pwndTransmit->pnothRegisterHandler(this, &TMainFrameWnd::eClickHandler);

    // Disable stuff until the server is enabled
    m_pwndTransmit->SetEnable(kCIDLib::False);
    m_pwndNewMsg->SetEnable(kCIDLib::False);

    m_strClientConn = facCQCRPortSrv().strMsg(kRPortSMsgs::midStatus_GotClient);
    m_strClientNotConn = facCQCRPortSrv().strMsg(kRPortSMsgs::midStatus_NoClient);

    //
    //  Set up the columns for the allowed ports list, leaving an empty one for the check
    //  mark. We also don't create check boxes, so the title text doesn't matter much.
    //
    tCIDLib::TStrList colCols;
    colCols.AddXCopies(TString::strEmpty(), 2);
    colCols[1] = L"Port Path";
    m_pwndAllowedPorts->SetColumns(colCols);

    // Ask the facility class to load our port list
    facCQCRPortSrv().LoadList(*m_pwndAllowedPorts);

    // Put in some default values
    TString strFmt;
    strFmt.SetFormatted(TRemSerialSrvServerBase::ippnDefault);
    m_pwndPort->strWndText(strFmt);
    m_pwndConnStatus->strWndText(m_strClientNotConn);

    // If there was an initial focus in the dialog desc, set it
    if (widInitFocus)
        m_pwndClient->SetChildFocus(widInitFocus);

    SetVisibility(kCIDLib::True);

    return kCIDLib::True;
}


tCIDLib::TVoid TMainFrameWnd::Destroyed()
{
    // Sto the time just in case it was not stopped normally
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    TParent::Destroyed();
}


//
//  We enable a timer and use it to watch for msgs showing up in the incoming chat queue of
//  the chat object. We grab them and display them.
//
tCIDLib::TVoid TMainFrameWnd::Timer(const tCIDCtrls::TTimerId tmidSrc)
{
    if (m_tmidUpdate == tmidSrc)
    {
        tCIDLib::TBoolean bClientConn = kCIDLib::False;
        if (m_porbsChat)
        {
            if (m_porbsChat->bGetMsg(m_strChatTmp))
            {
                m_pwndMsgList->AppendText(m_strChatTmp, m_rgbHis);
                TAudio::AudioCue(tCIDLib::EAudioCues::Alert);
            }

            // See if we have changed client connection status
            bClientConn = m_porbsChat->bActiveClient();
        }

        if (m_bLastConn != bClientConn)
        {
            m_bLastConn = bClientConn;
            m_pwndConnStatus->strWndText(m_bLastConn ? m_strClientConn : m_strClientNotConn);
        }
    }
}


// ---------------------------------------------------------------------------
//  TMainFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle clicks from our buttons
tCIDCtrls::EEvResponses TMainFrameWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCRPortSrv::ridWnd_Main_Enable)
    {
        if (m_porbsChat || m_porbsSerial)
        {
            // Stop the timer now that we are going to kill the chat object
            if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidUpdate);
                m_tmidUpdate = kCIDCtrls::tmidInvalid;
            }

            // Deregister our objects, which will delete it
            facCIDOrb().DeregisterObject(m_porbsSerial);
            m_porbsSerial = nullptr;

            facCIDOrb().DeregisterObject(m_porbsChat);
            m_porbsChat = nullptr;

            // Turn off the ORB
            facCIDOrb().Terminate();

            // Close any ports that we have open on behalf of clients
            facCQCRPortSrv().CloseAllPorts();

            // And enable the port stuff so they can change them now
            m_pwndPort->SetEnable(kCIDLib::True);
            m_pwndSecKey->SetEnable(kCIDLib::True);

            // Disable the chat stuff
            m_pwndNewMsg->SetEnable(kCIDLib::False);
            m_pwndTransmit->SetEnable(kCIDLib::False);

            // Change the enable button to show enable now
            m_pwndEnable->strWndText(facCQCRPortSrv().strMsg(kRPortSMsgs::midWnd_Enable));

            // Force our client connection status
            m_bLastConn = kCIDLib::False;
            m_pwndConnStatus->strWndText(m_strClientNotConn);
        }
         else
        {
            // We have to have a security key
            m_strSecKey = m_pwndSecKey->strWndText();
            if (m_strSecKey.bIsEmpty())
            {
                TErrBox msgbErr(facCQCRPortSrv().strMsg(kRPortSErrs::errcCfg_NoSecKey));
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }

            //
            //  Iterate our allowed port list and tell the facility class what ports are
            //  allowed. If none, tell the user and stop.
            //
            tCIDLib::TCardList fcolAllowedPorts;
            const tCIDLib::TCard4 c4PortCnt = m_pwndAllowedPorts->c4ItemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4PortCnt; c4Index++)
            {
                if (m_pwndAllowedPorts->bIsCheckedAt(c4Index))
                    fcolAllowedPorts.c4AddElement(m_pwndAllowedPorts->c4IndexToId(c4Index));
            }
            if (fcolAllowedPorts.bIsEmpty())
            {
                TErrBox msgbErr(facCQCRPortSrv().strMsg(kRPortSErrs::errcCfg_NoPorts));
                msgbErr.ShowIt(*this);
                return tCIDCtrls::EEvResponses::Handled;
            }
            facCQCRPortSrv().SetAllowedPorts(fcolAllowedPorts);

            //
            //  If there is any content in the port field then validate it
            //  and make sure it's a valid port. If nothing, then set it to
            //  the default.
            //
            tCIDLib::TIPPortNum ippnNew;

            const TString& strPort = m_pwndPort->strWndText();
            if (strPort.bIsEmpty())
            {
                ippnNew = TRemSerialSrvServerBase::ippnDefault;
                TString strFmt;
                strFmt.SetFormatted(ippnNew);
                m_pwndPort->strWndText(strFmt);
            }
             else
            {
                tCIDLib::TBoolean bOk;
                ippnNew = TRawStr::c4AsBinary(strPort.pszBuffer(), bOk, tCIDLib::ERadices::Dec);
                if (!bOk)
                {
                    TErrBox msgbErr(facCQCRPortSrv().strMsg(kRPortSErrs::errcSPort_BadPort));
                    msgbErr.ShowIt(*this);
                    return tCIDCtrls::EEvResponses::Handled;
                }
            }

            //
            //  Set up the end point using the port. Then crank up the
            //  server side ORB support on the given port, and with a max
            //  of 1 client. In this case, we aren't using the name server
            //  so use the IP end point with the local a host name so that
            //  it'll look up the local address itself. Ultimately it
            //  doesn't even matteer since the object id will not be posted
            //  to the name server and downloaded by the client. The client
            //  will build up a target object id on its own.
            //
            facCIDOrb().InitServer(ippnNew, 1);

            //
            //  We don't use the name server, so we have particular object ids
            //  that the client proxy will know to look for.
            //
            TOrbObjId ooidSeral
            (
                TRemSerialSrvServerBase::strInterfaceId
                , TRemSerialSrvServerBase::strInstanceId
                , TSysInfo::strIPHostName()
                , ippnNew
                , L"TRemSerialSrvClientProxy"
            );

            // Create a new impl object and register it
            m_porbsSerial = new TRemSerialImpl(ooidSeral);
            facCIDOrb().RegisterObject(m_porbsSerial, tCIDLib::EAdoptOpts::Adopt);


            TOrbObjId ooidChat
            (
                TRemChatServerBase::strInterfaceId
                , TRemChatServerBase::strInstanceId
                , TSysInfo::strIPHostName()
                , ippnNew
                , L"TRemChatSrvClientProxy"
            );

            // Create a new impl object and register it
            m_porbsChat = new TRemChatImpl(ooidChat);
            facCIDOrb().RegisterObject(m_porbsChat, tCIDLib::EAdoptOpts::Adopt);

            // Disable the port stuff now
            m_pwndPort->SetEnable(kCIDLib::False);
            m_pwndSecKey->SetEnable(kCIDLib::False);

            // Enable the chat fields
            m_pwndNewMsg->SetEnable(kCIDLib::True);
            m_pwndTransmit->SetEnable(kCIDLib::True);

            // Restart the timer
            m_tmidUpdate = tmidStartTimer(50);

            // Change the text of the enable button
            m_pwndEnable->strWndText(facCQCRPortSrv().strMsg(kRPortSMsgs::midWnd_Disable));
        }
    }
     else if (wnotEvent.widSource() == kCQCRPortSrv::ridWnd_Main_Transmit)
    {
        SendCurText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We don't take changes to the allowed ports list once we've been enabled. So we set
//  the checked list box to non-auto mode and handle the check ourself.
//
tCIDCtrls::EEvResponses TMainFrameWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCRPortSrv::ridWnd_Main_AllowedPorts)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            // If we are currently enabled, tell them they can't do this, else toggle
            if (m_porbsSerial)
            {
                TErrBox msgbBusy(facCQCRPortSrv().strMsg(kRPortSErrs::errcSPort_RemoteActive));
                msgbBusy.ShowIt(*this);
            }
             else
            {
                m_pwndAllowedPorts->SetCheckAt
                (
                    wnotEvent.c4Index()
                    , !m_pwndAllowedPorts->bIsCheckedAt(wnotEvent.c4Index())
                );
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called from the Xmit button and from our accel table driven xmit command.
//
tCIDLib::TVoid TMainFrameWnd::SendCurText()
{
    // If there's text in the msg field, grab it and clear the text
    TString strMsg = m_pwndNewMsg->strWndText();
    m_pwndNewMsg->ClearText();

    // We should get called if the chat object isn't ready, but just in case
    if (m_porbsChat)
    {
        if (!strMsg.bIsEmpty())
        {
            // Give it to the impl where it will be available to the other side
            if (m_porbsChat->bQueueMsg(strMsg))
            {
                // And display it
                m_pwndMsgList->AppendText(strMsg, m_rgbMine);
            }
             else
            {
                TErrBox msgbErr(facCQCRPortSrv().strMsg(kRPortSErrs::errcChat_CantQMsg));
                msgbErr.ShowIt(*this);
            }
        }
    }
     else
    {
        TErrBox msgbErr(facCQCRPortSrv().strMsg(kRPortSErrs::errcChat_ChatNotReady));
        msgbErr.ShowIt(*this);
    }
}
