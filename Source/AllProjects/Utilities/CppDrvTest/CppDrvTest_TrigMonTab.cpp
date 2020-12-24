//
// FILE NAME: CppDrvTest_TrigMonTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/212018
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
//  This file implements the tab window that displays incoming event triggers.
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
#include "CppDrvTest.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTrigMonTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TTrigMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
TTrigMonTab::TTrigMonTab() :

    TTabWindow(L"Tigger Monitor", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_psasubEvTrigs(kCIDLib::False)
    , m_pwndOutput(nullptr)
    , m_strmOut(4096UL)
    , m_strmIn(m_strmOut)
    , m_tmidUpdate(kCIDCtrls::tmidInvalid)
{
}

TTrigMonTab::~TTrigMonTab()
{
}


// ---------------------------------------------------------------------------
//  TTrigMonTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  When the user does a new session, we are called here with the moniker so that we
//  can filter out triggers not from our driver.
//
tCIDLib::TVoid TTrigMonTab::SetMoniker(const TString& strMoniker)
{
    m_strMoniker = strMoniker;
}


// ---------------------------------------------------------------------------
//  TTrigMonTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TTrigMonTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged
    (
        areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged
    );

    // Keep our content sized to fit
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndOutput)
    {
        TArea areaOutput(areaClient());
        areaOutput.Deflate(2);
        m_pwndOutput->SetSizePos(areaOutput, kCIDLib::True);
    }
}


tCIDLib::TBoolean TTrigMonTab::bCreated()
{
    TParent::bCreated();

    // Create our text output window
    TArea areaInit(areaClient());
    areaInit.Deflate(2);

    m_pwndOutput = new TTextOutWnd();
    m_pwndOutput->CreateTextOut
    (
        *this
        , areaInit
        , kCIDCtrls::widFirstCtrl
        , tCIDCtrls::EWndStyles::VisChild
    );

    // Remember the colors of the output window
    m_rgbOrgBgn = m_pwndOutput->rgbCurBgn();
    m_rgbOrgText = m_pwndOutput->rgbCurText();

    // Subscribe to the topic that the above caused to be published
    try
    {
        m_psasubEvTrigs.SubscribeTo(kCQCKit::strPubTopic_EvTriggers, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbSub
        (
            L"Could not subscribe to the event trigger published topic. No event "
            L"triggers will be displayed due to this error."
        );
        return kCIDLib::True;
    }

    // Start the timer
    m_tmidUpdate = tmidStartTimer(500);

    return kCIDLib::True;
}


//
//  We need to clean up our update timer, and stop anything else that we might have
//  going. Since the driver is passive to us, i.e. we just poll it, we don't have
//  any concerns about the driver state.
//
tCIDLib::TVoid TTrigMonTab::Destroyed()
{
    if (m_tmidUpdate != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidUpdate);

    // Call our parent last
    TParent::Destroyed();
}


// We need to handle our update timer and keep our driver info up to date
tCIDLib::TVoid TTrigMonTab::Timer(const tCIDCtrls::TTimerId  tmidSrc)
{
    // If not ours, pass it on and return
    if (tmidSrc != m_tmidUpdate)
    {
        TParent::Timer(tmidSrc);
        return;
    }

    try
    {
        //
        //  While we have events available, up to a reasonable number, grab them, format
        //  them and output them to the text output window.
        //
        tCIDLib::TCard4 c4Count = 0;
        while (c4Count < 32)
        {
            // If we don't get one, we are done for this time
            if (!m_psasubEvTrigs.bGetNextMsg(m_psmsgTmp, 0))
                break;

            // Let's process the one we got
            {
                const TCQCEvent& cevNew = m_psmsgTmp.objMsgAs<TCQCEvent>();

                // Get the source out. Use current line as a temp
                if (!cevNew.bQuerySrc(m_strCurLine))
                    continue;

                // If it's a field source, cut off the field name part
                m_strCurLine.bCapAtChar(kCIDLib::chPeriod);

                // Now, if it's not equal to our driver's moniker, ignore it
                if (!m_strCurLine.bCompareI(m_strMoniker))
                    continue;


                // Looks like one of ours we need to display, so flip our colors
                m_c4EvCount++;
                if (m_c4EvCount & 1)
                {
                    m_pwndOutput->SetLineColors
                    (
                        facCIDGraphDev().rgbDarkGrey, facCIDGraphDev().rgbWhite
                    );
                }
                 else
                {
                    m_pwndOutput->SetLineColors(m_rgbOrgBgn, m_rgbOrgText);
                }

                // Format it out to our output stream
                m_strmOut.Reset();
                m_tmStamp.SetToNow();
                m_strmOut << L"TIME: " << m_tmStamp << kCIDLib::NewLn << cevNew;

                // Now read all the lines back in from our input stream
                m_strmIn.Reset();
                m_colLines.RemoveAll();

                // If on an event round, original colors, put out a divider before
                if (!(m_c4EvCount & 1))
                    m_colLines.objAdd(TString::strEmpty());

                while (!m_strmIn.bEndOfStream())
                {
                    m_strmIn >> m_strCurLine;
                    if (!m_strCurLine.bIsEmpty())
                        m_colLines.objAdd(m_strCurLine);
                }

                // If on an even round, original colors, put out a divider after
                if (!(m_c4EvCount & 1))
                    m_colLines.objAdd(TString::strEmpty());

                m_pwndOutput->AddStrings(m_colLines);

                c4Count++;
            }
        }
    }

    catch(TError& errToCatch)
    {
        //
        //  Stop the timer while we are in here or it will just continue causing the
        //  same error over and over, stacking up popups.
        //
        TTimerJanitor janUpdate(this, m_tmidUpdate);
        TExceptDlg dlgErr
        (
            *this, facCppDrvTest.strMsg(kCppDTErrs::errcGen_PollFailure), errToCatch
        );
    }
}

