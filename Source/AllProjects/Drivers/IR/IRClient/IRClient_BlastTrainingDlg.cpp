//
// FILE NAME: IRClient_BlastTrainingDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/26/2002
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
//  This file implements the training dialog, which waits for the user to
//  press the remote key to indicate which key he/she wants to use.
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
#include    "IRClient.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TBlastTrainingDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TBlastTrainingDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBlastTrainingDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TBlastTrainingDlg::TBlastTrainingDlg() :

    m_bBlinkState(kCIDLib::False)
    , m_c4Bytes(0)
    , m_c4BlinkCounter(0)
    , m_pwndBlinker(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDoIt(nullptr)
    , m_pwndDriver(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndIRData(nullptr)
    , m_pwndManualMode(nullptr)
    , m_pwndTest(nullptr)
    , m_pwndTrainMode(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TBlastTrainingDlg::~TBlastTrainingDlg()
{
}


// ---------------------------------------------------------------------------
//  TBlastTrainingDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TBlastTrainingDlg::bRunDlg( const   TWindow&            wndOwner
                            ,       THeapBuf&           mbufToFill
                            ,       tCIDLib::TCard4&    c4Bytes
                            ,       TIRClientWnd* const pwndDriver
                            , const TString&            strPrevData)
{
    // Save the driver
    m_pwndDriver = pwndDriver;
    m_strPrevData = strPrevData;

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  stored IR event key.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facIRClient(), kIRClient::ridDlg_IRTrain
    );

    //
    //  If we didn't exit with a do it, then force the server out of training mode
    //  if we started it.
    //
    if (c4Res != kIRClient::ridDlg_IRTrain_DoIt)
    {
        try
        {
            m_pwndDriver->orbcServer()->c4SendCmd
            (
                m_pwndDriver->strMoniker()
                , kCQCIR::strExitBTraining
                , TString::strEmpty()
                , m_pwndDriver->sectUser()
            );
        }

        catch(TError& errToCatch)
        {
            if (facIRClient().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }
        }
    }

    //
    //  If they went through with it, then lets copy our members to the
    //  caller's output parms.
    //
    if (c4Res == kIRClient::ridDlg_IRTrain_DoIt)
    {
        CIDAssert(m_c4Bytes != 0, L"There are no training data bytes")
        c4Bytes = m_c4Bytes;
        mbufToFill = m_mbufData;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TBlastTrainingDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TBlastTrainingDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_Blinker, m_pwndBlinker);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_DoIt, m_pwndDoIt);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_Cancel, m_pwndCancel);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_IRData, m_pwndIRData);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_ManualMode, m_pwndManualMode);
    CastChildWnd(*this, kIRClient::ridDlg_IRTrain_TrainMode, m_pwndTrainMode);

    //
    //  Calculate the pixel sizes for the two modes.
    //
    //  Training size is the bottom of the instructions plus 24 pixels plus the height
    //  of the buttons, plus a little padding.
    //
    //  Manual size is the current size.
    //
    m_szManual = areaWndSize().szArea();

    // Calculate the client area for the train mode
    TArea areaClient(areaClient());
    areaClient.c4Height
    (
        m_pwndInstruct->areaWnd().i4Bottom() + m_pwndDoIt->areaWndSize().c4Height() + 32
    );

    // And the frame size required to hold it
    TArea areaFrame;
    TFrameWnd::AreaForClient
    (
        areaClient
        , areaFrame
        , eWndStyles()
        , eExWndStyles()
        , kCIDLib::False
    );
    m_szTrain = areaFrame.szArea();

    //
    //  We have to override the default minimum size which is the size of the original
    //  dialog content. We need to allow it to size down to the training size. The default
    //  was set when we called our parent above.
    //
    //  Do this before we set the mode below since it will try to change the size.
    //
    SetMinSize(m_szTrain);

    //
    //  Initially set up for the input mode. If the device doesn't
    //  support training, we set up manual input and disable the other
    //  radio button. Else we default to training mode.
    //
    if (tCIDLib::bAllBitsOn(m_pwndDriver->eDevCaps(), tCQCIR::EIRDevCaps::BlastTrain))
    {
        SetMode(EModes::Train);
    }
     else
    {
        SetMode(EModes::Manual);
        m_pwndTrainMode->SetEnable(kCIDLib::False);

        m_pwndIRData->strWndText(m_strPrevData);
        m_pwndIRData->SelectAll();
    }

    // Load our off/on bitmaps
    m_bmpOff = facCIDCtrls().bmpLoadSmallAppImg(L"Gray Ball");
    m_bmpOn = facCIDCtrls().bmpLoadSmallAppImg(L"Red Ball");

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TBlastTrainingDlg::eClickHandler);
    m_pwndDoIt->pnothRegisterHandler(this, &TBlastTrainingDlg::eClickHandler);
    m_pwndManualMode->pnothRegisterHandler(this, &TBlastTrainingDlg::eClickHandler);
    m_pwndTrainMode->pnothRegisterHandler(this, &TBlastTrainingDlg::eClickHandler);

    return bRet;
}


tCIDLib::TVoid TBlastTrainingDlg::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    TParent::Destroyed();
}


tCIDLib::TVoid TBlastTrainingDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo != m_tmidUpdate)
        return;

    //
    //  Ask the server side blaster object if it has training data for us.
    //  If so, then we are done.
    //
    try
    {
        const tCIDLib::TBoolean bRes = m_pwndDriver->orbcServer()->bQueryData
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strQueryBTrainingData
            , TString::strEmpty()
            , m_c4Bytes
            , m_mbufData
        );

        if (bRes)
        {
            if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
            {
                StopTimer(m_tmidUpdate);
                m_tmidUpdate = kCIDCtrls::tmidInvalid;
            }
            EndDlg(kIRClient::ridDlg_IRTrain_DoIt);
            return;
        }
    }

    catch(const TError& errToCatch)
    {
        // Go back to idle state again
        m_pwndManualMode->SetEnable(kCIDLib::True);
        m_pwndTrainMode->SetEnable(kCIDLib::True);
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }
        m_pwndDoIt->SetEnable(kCIDLib::True);

        // Something went wrong, so reset
        TExceptDlg dlgFail
        (
            *this
            , strWndText()
            , facIRClient().strMsg(kIRCMsgs::midStatus_PollFailed)
            , errToCatch
        );
        return;
    }

    // Didn't see it yet, so blink our blinker LED if we need to
    m_c4BlinkCounter++;
    if (m_c4BlinkCounter >= 2)
    {
        // Flip the state, reset the counter, and update the LED
        m_bBlinkState = !m_bBlinkState;
        m_c4BlinkCounter = 0;

        m_pwndBlinker->SetBitmap(m_bBlinkState ? m_bmpOn : m_bmpOff);
    }
}


// ---------------------------------------------------------------------------
//  TBlastTrainingDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handles events from our buttons and check boxes
tCIDCtrls::EEvResponses
TBlastTrainingDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kIRClient::ridDlg_IRTrain_DoIt)
    {
        if (m_eMode == EModes::Manual)
        {
            // Make sure there's some data in the input area
            TString strData = m_pwndIRData->strWndText();
            strData.StripWhitespace();
            if (strData.bIsEmpty())
            {
                TErrBox msgbErr(facIRClient().strMsg(kIRCMsgs::midStatus_NoIRData));
                msgbErr.ShowIt(*this);
            }
             else
            {
                try
                {
                    //
                    //  Ask the driver to convert the text as required and
                    //  complain if it's not acceptable.
                    //
                    const tCIDLib::TBoolean bRes = m_pwndDriver->orbcServer()->bQueryData
                    (
                        m_pwndDriver->strMoniker()
                        , kCQCIR::strCvtManualBlastData
                        , strData
                        , m_c4Bytes
                        , m_mbufData
                    );

                    if (bRes)
                    {
                        // We is done
                        EndDlg(kIRClient::ridDlg_IRTrain_DoIt);
                    }
                     else
                    {
                        //
                        //  He didn't like it. A string will have been streamed
                        //  back with the error text.
                        //
                        TString strErr;
                        TTextMBufInStream strmErr(&m_mbufData, m_c4Bytes);
                        strmErr >> strErr;

                        TErrBox msgbErr(strErr);
                        msgbErr.ShowIt(*this);
                    }
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    TExceptDlg dlgFail
                    (
                        *this
                        , strWndText()
                        , facIRClient().strMsg(kIRCMsgs::midStatus_ManualValFailed)
                        , errToCatch
                    );
                }
            }
        }
         else
        {
            // Tell the server to enter training mode
            try
            {
                m_pwndDriver->orbcServer()->c4SendCmd
                (
                    m_pwndDriver->strMoniker()
                    , kCQCIR::strEnterBTraining
                    , TString::strEmpty()
                    , m_pwndDriver->sectUser()
                );

                //
                //  Disable the mode check boxes so they cannot change mode,
                //  and the DoIt button so they can't invoke it again.
                //
                m_pwndManualMode->SetEnable(kCIDLib::False);
                m_pwndTrainMode->SetEnable(kCIDLib::False);
                m_pwndDoIt->SetEnable(kCIDLib::False);

                // Crank up a timer to poll the server and blink the blinker
                m_bBlinkState = kCIDLib::False;
                m_c4BlinkCounter = 0;

                // Crank up a timer to use to poll the server
                m_tmidUpdate = tmidStartTimer(250);
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                TExceptDlg dlgbFail
                (
                    *this
                    , strWndText()
                    , facIRClient().strMsg(kIRCMsgs::midStatus_CantStartCapture)
                    , errToCatch
                );
            }
        }
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_IRTrain_Cancel)
    {
        //
        //  Stop the timer first in case we started it, so it'll quit
        //  talking to the server.
        //
        if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        {
            StopTimer(m_tmidUpdate);
            m_tmidUpdate = kCIDCtrls::tmidInvalid;
        }

        // And exit with a negative result
        EndDlg(kIRClient::ridDlg_IRTrain_Cancel);
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_IRTrain_ManualMode)
    {
        // Switch to manual mode
        SetMode(EModes::Manual);
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_IRTrain_TrainMode)
    {
        SetMode(EModes::Train);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Called to change the input mode. Initially from the startup to get the
//  initial state set, then when the user selects one of the buttons.
//
tCIDLib::TVoid TBlastTrainingDlg::SetMode(const EModes eToSet)
{
    if (eToSet == EModes::Train)
    {
        // Show/hide stuff as needed
        m_pwndIRData->SetVisibility(kCIDLib::False);
        m_pwndBlinker->SetVisibility(kCIDLib::True);
        m_pwndInstruct->SetVisibility(kCIDLib::True);

        // Size the dialog appropriately
        SetSize(m_szTrain, kCIDLib::True);

        //
        //  Change the text of the DoIt button appropriately, and position
        //  the buttons appropriately for this mode.
        //
        m_pwndDoIt->strWndText(facIRClient().strMsg(kIRCMsgs::midDlg_IRTrain_Start));

        TArea areaButton = m_pwndDoIt->areaWnd();
        areaButton.i4Top(m_pwndInstruct->areaWnd().i4Bottom() + 24);
        m_pwndDoIt->SetPos(areaButton.pntOrg());
        areaButton = m_pwndCancel->areaWnd();
        areaButton.i4Top(m_pwndInstruct->areaWnd().i4Bottom() + 24);
        m_pwndCancel->SetPos(areaButton.pntOrg());

        m_pwndTrainMode->SetCheckedState(kCIDLib::True);
    }
     else
    {
        // Show/hide stuff as needed
        m_pwndIRData->SetVisibility(kCIDLib::True);
        m_pwndBlinker->SetVisibility(kCIDLib::False);
        m_pwndInstruct->SetVisibility(kCIDLib::False);

        // Make any changes we need
        SetSize(m_szManual, kCIDLib::True);
        m_pwndIRData->ClearText();

        // Change the text of the DoIt button appropriately
        m_pwndDoIt->strWndText(facIRClient().strMsg(kIRCMsgs::midDlg_IRTrain_Accept));

        TArea areaButton = m_pwndDoIt->areaWnd();
        areaButton.i4Top(m_pwndIRData->areaWnd().i4Bottom() + 16);
        m_pwndDoIt->SetPos(areaButton.pntOrg());
        areaButton = m_pwndCancel->areaWnd();
        areaButton.i4Top(m_pwndIRData->areaWnd().i4Bottom() + 16);
        m_pwndCancel->SetPos(areaButton.pntOrg());

        m_pwndManualMode->SetCheckedState(kCIDLib::True);
    }

    // And store the new mode
    m_eMode = eToSet;
}

