//
// FILE NAME: ZWaveUSBC_LearnDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/03/2003
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
//  This file implements a simple dialog that is displayed while the driver
//  is in training mode.
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
#include    "ZWaveUSBC_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWaveLearnDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TZWaveLearnDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWaveLearnDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWaveLearnDlg::TZWaveLearnDlg() :

    m_bBlinkState(kCIDLib::False)
    , m_c4BlinkCounter(0)
    , m_pwndBlinker(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndStatus(nullptr)
    , m_tmidBlinker(kCIDCtrls::tmidInvalid)
{
}

TZWaveLearnDlg::~TZWaveLearnDlg()
{
}


// ---------------------------------------------------------------------------
//  TZWaveLearnDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TZWaveLearnDlg::bRunDlg(const TWindow& wndOwner, const TString& strMoniker)
{
    m_strMoniker = strMoniker;

    //
    //  Ask the server driver to cancel any existing replication, just in case.
    //  We don't look at the return since any failures will throw.
    //
    try
    {
        // Create our client proxy object
        m_orbcTarget = facCQCKit().orbcCQCSrvAdminProxy(m_strMoniker);
        m_orbcTarget->c4SendCmd
        (
            strMoniker, kZWaveUSBSh::strDrvCmd_SetRepMode, L"Start"
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TExceptDlg dlgbFail
        (
            wndOwner
            , wndOwner.strWndText()
            , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_RepStartFailed)
            , errToCatch
        );
        return kCIDLib::False;
    }

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  stored IR event key.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facZWaveUSBC(), kZWaveUSBC::ridDlg_Replicate
    );

    return (c4Res == 9999);
}


// ---------------------------------------------------------------------------
//  TZWaveLearnDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TZWaveLearnDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kZWaveUSBC::ridDlg_Rep_Blinker, m_pwndBlinker);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_Rep_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kZWaveUSBC::ridDlg_Rep_Status, m_pwndStatus);

    // And register our button event handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TZWaveLearnDlg::eClickHandler);

    // Set the initial status and status text
    m_ePrevRes = tZWaveUSBSh::ERepRes_Replicating;
    m_pwndStatus->strWndText(tZWaveUSBSh::strLoadERepRes(m_ePrevRes));

    // Load our two blink LEDs
    m_bmpBlinkOff = facCIDCtrls().bmpLoadSmallAppImg(L"Grey Ball");
    m_bmpBlinkOn = facCIDCtrls().bmpLoadSmallAppImg(L"Red Ball");

    // Crank up a timer to use to poll the server, and blink the learning LED.
    m_tmidBlinker = tmidStartTimer(250);

    return bRet;
}


// We don't use the parm, since we only start one timer
tCIDLib::TVoid TZWaveLearnDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // If not ours, pass it on
    if (tmidToDo != m_tmidBlinker)
    {
        Timer(tmidToDo);
        return;
    }

    //
    //  Check the status. As long as it's in 'waiting' mode, we keep waiting.
    //  Otherwise it goes to ok or failed status.
    //
    const tCIDLib::TCard4 c4Status = m_orbcTarget->c4QueryVal
    (
        m_strMoniker, kZWaveUSBSh::strValId_DrvStatus
    );

    const tCIDLib::TBoolean bRepMode = (c4Status & 0xFFFF) != 0;
    const tZWaveUSBSh::ERepRes eRes = tZWaveUSBSh::ERepRes(c4Status >> 16);

    //
    //  If we are still in replication mode, then blink the LED every
    //  now and again and keep waiting. Else, we look at the status and
    //  and dismiss ourself with the appropriate code.
    //
    if (bRepMode)
    {
        m_c4BlinkCounter++;
        if (m_c4BlinkCounter >= 2)
        {
            // Flip the state, reset the counter, and update the LED
            m_bBlinkState = !m_bBlinkState;
            m_c4BlinkCounter = 0;
            m_pwndBlinker->SetBitmap(m_bBlinkState ? m_bmpBlinkOn : m_bmpBlinkOff);
        }

        // If the status has changed, update the text
        if (eRes != m_ePrevRes)
        {
            m_ePrevRes = eRes;
            m_pwndStatus->strWndText(tZWaveUSBSh::strLoadERepRes(eRes));
        }
    }
     else
    {
        if (eRes == tZWaveUSBSh::ERepRes_OK)
        {
            // It worked, so tell them and then exit
            TOkBox msgbOk(tZWaveUSBSh::strLoadERepRes(eRes));
            msgbOk.ShowIt(*this);
            EndDlg(9999);
        }
         else if ((eRes == tZWaveUSBSh::ERepRes_Failed)
              ||  (eRes == tZWaveUSBSh::ERepRes_TimedOut))
        {
            // It worked, so tell them and then exit
            StopTimer(tmidToDo);
            TOkBox msgbOk(tZWaveUSBSh::strLoadERepRes(eRes));
            msgbOk.ShowIt(*this);
            EndDlg(0);
        }
         else
        {
            #if CID_DEBUG_ON
            // <TBD> Don't know what this is. Do a popup or something
            #endif
        }
    }
}


// ---------------------------------------------------------------------------
//  TZWaveLearnDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TZWaveLearnDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kZWaveUSBC::ridDlg_Rep_Cancel)
    {
        // Disable the update timer
        StopTimer(m_tmidBlinker);

        //
        //  Ask the server driver to cancel replication. It could have possibly
        //  ended itself (failed or succeeded) between the time the user
        //  decided to press cancel and we got here. But that's fine, since the
        //  driver will ignore it if it's not actually in replication mode.
        try
        {
            m_orbcTarget->c4SendCmd
            (
                m_strMoniker, kZWaveUSBSh::strDrvCmd_SetRepMode, L"Stop"
            );
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            facCQCGKit().ShowError
            (
                *this
                , strWndText()
                , facZWaveUSBC().strMsg(kZWCMsgs::midStatus_RepCanFailed)
                , errToCatch
            );
        }
        EndDlg(0);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


