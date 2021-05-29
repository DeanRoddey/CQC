//
// FILE NAME: IRClient_IRTrainingDlg.cpp
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
//  This file implements the receiver training dialog, which waits for the
//  user to press the remote key a few times to indicate which key he/she
//  wants to use for a particular event.
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
RTTIDecls(TRecTrainingDlg,TDlgBox)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace IRClient_RecTrainingDlg
    {
        //
        //  The number of times we have to get a consistent read of training data
        //  before we say we got it.
        //
        constexpr tCIDLib::TCard4   c4CheckCount = 2;
    }
}


// ---------------------------------------------------------------------------
//   CLASS: TRecTrainingDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRecTrainingDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TRecTrainingDlg::TRecTrainingDlg() :

    m_bBlinkState(kCIDLib::False)
    , m_c4BlinkCounter(0)
    , m_c4Counter(0)
    , m_pwndBlinker(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDriver(nullptr)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TRecTrainingDlg::~TRecTrainingDlg()
{
}


// ---------------------------------------------------------------------------
//  TRecTrainingDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TRecTrainingDlg::bRunDlg(const  TWindow&            wndOwner
                        ,       TString&            strToFill
                        , const TString&            strRetrainKey
                        ,       TIRClientWnd* const pwndDriver)
{
    //
    //  Assume worst case and clear caller's return buffer. And store the
    //  other stuff for later use.
    //
    strToFill.Clear();
    m_strRetrainKey = strRetrainKey;
    m_pwndDriver = pwndDriver;

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  stored IR event key.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facIRClient(), kIRClient::ridDlg_IRRTrain
    );

    // If the standard OK id was used to close it, then they comitted.
    if (c4Res == kCIDCtrls::widOK)
    {
        strToFill = m_strEvent;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TRecTrainingDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TRecTrainingDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kIRClient::ridDlg_IRRTrain_Cancel, m_pwndCancel);
    CastChildWnd(*this, kIRClient::ridDlg_IRRTrain_Blinker, m_pwndBlinker);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TRecTrainingDlg::eClickHandler);

    // Do an initial clear of the training data
    try
    {
        m_pwndDriver->orbcServer()->c4SendCmd
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strClearRTraining
            , TString::strEmpty()
            , m_pwndDriver->sectUser()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    // Load our off/on bitmaps
    m_bmpOff = facCIDCtrls().bmpLoadSmallAppImg(L"Gray Ball");
    m_bmpOn = facCIDCtrls().bmpLoadSmallAppImg(L"Red Ball");

    // Crank up a timer to use to poll the server
    m_tmidUpdate = tmidStartTimer(250);

    return bRet;
}


// We don't use the parm, since we only start one timer
tCIDLib::TVoid TRecTrainingDlg::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    // If not our timer, then do nothing
    if (tmidToDo != m_tmidUpdate)
        return;

    // See if we've got an event
    tCIDLib::TBoolean bGotOne = kCIDLib::False;

    try
    {
        bGotOne = m_pwndDriver->orbcServer()->bQueryTextVal
        (
            m_pwndDriver->strMoniker()
            , kCQCIR::strQueryRTrainingData
            , TString::strEmpty()
            , m_strEvent
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
    }

    if (bGotOne)
    {
        //
        //  If its the same as the previous one, then bump the counter. Else,
        //  reset the counter, store it as the previous one.
        //
        if (m_strEvent == m_strEvent2)
        {
            m_c4Counter++;
        }
         else
        {
            m_c4Counter = 0;
            m_strEvent2 = m_strEvent;
        }

        //
        //  If we hit the required matches, then we have a candidate. First
        //  we see if this key is already mapped. If so, we have to ask if they
        //  really want to replace it or not.
        //
        if (m_c4Counter >= IRClient_RecTrainingDlg::c4CheckCount)
        {
            tCIDLib::TBoolean   bAlreadyUsed;
            tCIDLib::TBoolean   bTakeIt = kCIDLib::False;
            TString             strCurEvent;

            //
            //  If it's the retrain key, then they just reused the same key
            //  and we don't need to check for duplication. Else, check and
            //  ask if they want to use the duplicate key or not.
            //
            if (m_strEvent == m_strRetrainKey)
            {
                bAlreadyUsed = kCIDLib::False;
            }
             else
            {
                try
                {
                    bAlreadyUsed = m_pwndDriver->orbcServer()->bQueryTextVal
                    (
                        m_pwndDriver->strMoniker()
                        , kCQCIR::strCheckKeyUsed
                        , m_strEvent
                        , strCurEvent
                    );
                }

                catch(TError& errToCatch)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                }
            }

            if (bAlreadyUsed)
            {
                // Suppress this timer while we are in here
                TTimerJanitor janTimer(this, m_tmidUpdate);

                TYesNoBox msgbAsk
                (
                    facIRClient().strMsg(kIRCMsgs::midQ_ReplaceEvent, strCurEvent)
                );
                bTakeIt = msgbAsk.bShowIt(*this);
            }
             else
            {
                bTakeIt = kCIDLib::True;
            }

            if (bTakeIt)
            {
                StopTimer(m_tmidUpdate);
                EndDlg(1);
            }
             else
            {
                // They don't want to overwrite, so reset and try again
                m_c4Counter = 0;
                m_strEvent.Clear();
                m_strEvent2.Clear();
            }
        }
    }

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
//  TRecTrainingDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TRecTrainingDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kIRClient::ridDlg_IRRTrain_Cancel)
    {
        // Stop the timer and exit with a negative results
        StopTimer(m_tmidUpdate);
        EndDlg(kIRClient::ridDlg_IRRTrain_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


