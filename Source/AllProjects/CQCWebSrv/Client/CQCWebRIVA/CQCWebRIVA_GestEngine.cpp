//
// FILE NAME: CQCWebRIVA_GestEngine.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/26/2013
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
//  This file implements a gesture engine derivative that will map incoming RIVA mouse events
//  to gestures.
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
#include    "CQCWebRIVA_.hpp"


// ---------------------------------------------------------------------------
//   CLASS: TRIVAGestHandler
//  PREFIX: gesth
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TRIVAGestHandler: Constructors and Destructor
// ---------------------------------------------------------------------------
TRIVAGestHandler::TRIVAGestHandler(MCIDGestTarget* const pmgesthTar) :

    TCIDGestHandler(pmgesthTar)
    , m_eCurState(EInpStates::Idle)
{
}

TRIVAGestHandler::~TRIVAGestHandler()
{
}


// ---------------------------------------------------------------------------
//  TRIVAGestHandler: Public, virtual methods
// ---------------------------------------------------------------------------

//
//  The RIVA engine calls us here with incoming messages from clients. We make the
//  appropriate calls to our base gesture engine class and he gestures the gesture
//  operations required.
//
tCIDLib::TVoid
TRIVAGestHandler::HandleMsg(const   tCQCWebRIVA::EGestEvs   eEv
                            , const TPoint&                 pntAt)
{
    if (eEv == tCQCWebRIVA::EGestEvs::Move)
    {
        //
        //  We only do anything here if we got a press and are waiting for an
        //  initial movement to see if we exceed the breakout area and turn into
        //  a gesture. Or, we are already in a gesture.
        //
        if (m_eCurState == EInpStates::GotPress)
        {
            //
            //  If we've moved out of the breakout area, start a gesture and report
            //  it to the window.
            //
            if (!m_areaBreakout.bContainsPoint(pntAt))
            {
                //
                //  Call our parent with a start and the original start point. Then
                //  a first move with our new point.
                //
                if (bStartGesture(m_pntStart))
                {
                    // Always report not two fingered in our case
                    Move(pntAt, kCIDLib::False);

                    //
                    //  And update our state now to in gesture, if we didn't get
                    //  cancelled. If the gesture wasn't accepted, then Reset()
                    //  will have been called. If we are not in idle state now,
                    //  then move forward because we didn't get whacked.
                    //
                    if (m_eCurState != EInpStates::Idle)
                        m_eCurState = EInpStates::InGesture;
                }
            }
        }
         else if (m_eCurState == EInpStates::InGesture)
        {
            //
            //  Report this as a move becaues we are in a gesture. We can't support
            //  two fingers, so we always say it's not two fingers.
            //
            Move(pntAt, kCIDLib::False);
        }
    }
     else if (eEv == tCQCWebRIVA::EGestEvs::Press)
    {
        m_pntStart = pntAt;

        //
        //  If the base class says it's ok to process a press, then let's assume
        //  it's the start of a gesture or click. Put us into got press state. If
        //  we are already in a gesture, or in some state where we can't accept
        //  any input, it wil return false and we'll do nothing.
        //
        //  If we start, set up a breakout area. We have to move out of this to
        //  become a gesture, else it'll be treated as a click.
        //
        if (bAllowClickStart())
        {
            // Set up the breakout area
            m_areaBreakout.Set(m_pntStart, TSize(16, 16));
            m_areaBreakout.AdjustOrg(-8, -8);

            // Remember we got a press and are waiting to see what happens
            m_eCurState = EInpStates::GotPress;
        }
    }
     else if (eEv == tCQCWebRIVA::EGestEvs::Release)
    {
        //
        //  Go back to idle state now. But remember the current state
        //  for below. This insures we get back to idle.
        //
        EInpStates eState(m_eCurState);
        m_eCurState = EInpStates::Idle;

        //
        //  If we were waiting for berakout and never got it, then it's a
        //  now a click. Else, if we are in a gesture, then we send the
        //  end gesture and clean up.
        //
        if (eState == EInpStates::GotPress)
        {
            Clicked(pntAt);
        }
         else if (eState == EInpStates::InGesture)
        {
            // Call our parent to tell him its over
            EndGesture(pntAt);
        }
    }
}


// ---------------------------------------------------------------------------
//  TRIVAGestHandler: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  We don't do anything for starting. For cancelled or ending we clean our
//  state up.
//
tCIDLib::TVoid
TRIVAGestHandler::Reset(const tCIDCtrls::EGestReset eType)
{
    // Call our parent always
    TCIDGestHandler::Reset(eType);

    if ((eType == tCIDCtrls::EGestReset::Ended)
    ||  (eType == tCIDCtrls::EGestReset::Cancelled))
    {
        // If not idle, then clean up
        if (m_eCurState != EInpStates::Idle)
            m_eCurState = EInpStates::Idle;
    }
}

