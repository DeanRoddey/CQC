//
// FILE NAME: CQCVoice_MainFrame.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2017
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
//  Implements the main interface of the program, shown when the user clicks the
//  tray icon.
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
#include    "CQCVoice.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVoiceFrameWnd,TFrameWnd)



// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCVoiceFrameWnd: Constructors and destructor
// ---------------------------------------------------------------------------
TCQCVoiceFrameWnd::TCQCVoiceFrameWnd() :

    m_colMsgQ(tCIDLib::EMTStates::Safe)
    , m_pwndDebugOutput(nullptr)
    , m_pwndDebugVerbose(nullptr)
    , m_pwndMsgs(nullptr)
    , m_pwndReload(nullptr)
{
}

TCQCVoiceFrameWnd::~TCQCVoiceFrameWnd()
{
}


// ---------------------------------------------------------------------------
//  TCQCVoiceFrameWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Create ourself
tCIDLib::TVoid TCQCVoiceFrameWnd::Create()
{
    // Load the dialog description that defines out child controls
    TDlgDesc dlgdMain;
    facCQCVoice.bCreateDlgDesc(kCQCVoice::ridMain, dlgdMain);

    // Get a frame window size that will fit this
    TArea areaInit;
    TFrameWnd::AreaForClient
    (
        dlgdMain.areaPos()
        , areaInit
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , kCIDLib::False
    );

    // Update the origin to allow for auto-positioning
    areaInit.SetOrg(kCIDCtrls::wcrDefault, kCIDCtrls::wcrDefault);

    // Try to create the main frame. We are initially invisible.
    TParent::CreateFrame
    (
        nullptr
        , areaInit
        , facCQCVoice.strMsg(kCQCVoiceMsgs::midMain_Title)
        , tCIDCtrls::EWndStyles::StdFrameNV
        , tCIDCtrls::EExWndStyles::None
        , tCIDCtrls::EFrameStyles::None
        , kCIDLib::False
    );
}


// Add a message to our output queue
tCIDLib::TVoid
TCQCVoiceFrameWnd::QueueMsg(const TString& strMsg)
{
    // Lock remember if it was empty, and add the new message
    tCIDLib::TBoolean bWasEmpty = kCIDLib::False;
    {
        TLocker lockrSync(&m_colMsgQ);
        bWasEmpty = m_colMsgQ.bIsEmpty();
        m_colMsgQ.objAdd(strMsg);
    }

    // And now we can send a code to ourself if it was empty. Async is the default
    if (bWasEmpty)
        SendCode(1, 0);
}



// ---------------------------------------------------------------------------
//  TCQCVoiceFrameWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// We override this to handle re-sizing child controls to fit
tCIDLib::TVoid
TCQCVoiceFrameWnd::AreaChanged(const   TArea&                  areaPrev
                                , const TArea&                  areaNew
                                , const tCIDCtrls::EPosStates   ePosState
                                , const tCIDLib::TBoolean       bOrgChanged
                                , const tCIDLib::TBoolean       bSizeChanged
                                , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // If our size changed, then auto-resize any children that have anchors set
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged)
        AutoAdjustChildren(areaPrev, areaNew);
}


//
//  We override this and instead of closing, we just hide ourself. We only
//  ever exit via the tray icon's popup menu or when sent a shutdown request
//  by the app tray.
//
tCIDLib::TBoolean TCQCVoiceFrameWnd::bAllowShutdown()
{
    if (bIsVisible())
        SetVisibility(kCIDLib::False);
    return kCIDLib::False;
}


//
//  We need to create our dialog resource driven contents and load up
//  our tabs.
//
tCIDLib::TBoolean TCQCVoiceFrameWnd::bCreated()
{
    // Call our parent first
    TParent::bCreated();

    // We load our child widgets from a dialog description
    TDlgDesc dlgdMain;
    facCQCVoice.bCreateDlgDesc(kCQCVoice::ridMain, dlgdMain);

    // Set our icon
    facCQCVoice.SetFrameIcon(*this, L"CQCVoice");

    //
    //  Set our background color appropriately. This insures that all of the
    //  children also pick up our background color.
    //
    SetBgnColor(TDlgBox::rgbDlgBgn(dlgdMain.eTheme()));
    tCIDCtrls::TWndId widInitFocus;
    CreateDlgItems(dlgdMain, widInitFocus);

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kCQCVoice::ridMain_DebugOutput, m_pwndDebugOutput);
    CastChildWnd(*this, kCQCVoice::ridMain_DebugVerbose, m_pwndDebugVerbose);
    CastChildWnd(*this, kCQCVoice::ridMain_StatusMsgs, m_pwndMsgs);
    CastChildWnd(*this, kCQCVoice::ridMain_Reload, m_pwndReload);

    // Initialize the debug output check box to match the actual value
    m_pwndDebugOutput->SetCheckedState(facCQCVoice.bDebugOutput());
    m_pwndDebugVerbose->SetCheckedState(facCQCVoice.bDebugVerbose());

    // Set up event handlers as required
    m_pwndDebugOutput->pnothRegisterHandler(this, &TCQCVoiceFrameWnd::eClickHandler);
    m_pwndDebugVerbose->pnothRegisterHandler(this, &TCQCVoiceFrameWnd::eClickHandler);
    m_pwndReload->pnothRegisterHandler(this, &TCQCVoiceFrameWnd::eClickHandler);

    // Set min/max tracking sizes. The min is our initial size
    SetMinMaxSize(areaWndSize().szArea(), TWindow::wndDesktop().areaWndSize().szArea());

    return kCIDLib::True;
}


//
//  When a message is queued and the queue is empty, we post ourself a code, which
//  tells us msgs are available to display. Only one code is posted, so we don't need
//  to check it.
//
tCIDLib::TVoid
TCQCVoiceFrameWnd::CodeReceived(const tCIDLib::TInt4, const tCIDLib::TCard4)
{
    //
    //  While there are messages in the queue, send them to the text output window. They
    //  may be multi-line, so we parse into separate lines and add them.
    //
    TString strMsg;
    TString strLine;
    while (m_colMsgQ.bGetNextMv(strMsg, 1, kCIDLib::False))
    {
        m_colTmpMsgs.RemoveAll();
        TTextStringInStream strmSrc(&strMsg);
        while (!strmSrc.bEndOfStream())
        {
            strmSrc >> strLine;
            m_colTmpMsgs.objAdd(strLine);
        }

        m_pwndMsgs->AddStrings(m_colTmpMsgs);
    }
}


// ---------------------------------------------------------------------------
//  TCQCVoiceFrameWnd: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TCQCVoiceFrameWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCVoice::ridMain_DebugOutput)
    {
        facCQCVoice.bDebugOutput(wnotEvent.bState());
    }
     else if (wnotEvent.widSource() == kCQCVoice::ridMain_DebugVerbose)
    {
        facCQCVoice.bDebugVerbose(wnotEvent.bState());
    }
     else if (wnotEvent.widSource() == kCQCVoice::ridMain_Reload)
    {
        //
        //  Normally they'd do this via a voice command, but we provide a button to do
        //  it as well. We just set a flag to let him know that it's been requested. He
        //  will do it the next time he's able.
        //
        facCQCVoice.SetReloadReqFlag();
    }
    return tCIDCtrls::EEvResponses::Handled;
}
