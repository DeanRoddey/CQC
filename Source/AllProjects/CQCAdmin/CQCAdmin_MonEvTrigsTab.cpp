//
// FILE NAME: CQCAdmin_MonEvTrigsTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/06/2016
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
//  This file implements a tab for monitoring event triggers.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_MonEvTrigsTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMonEvTrigsTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TMonEvTrigsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMonEvTrigsTab: Constructors and destructor
// ---------------------------------------------------------------------------
TMonEvTrigsTab::TMonEvTrigsTab(const TString& strPath, const TString& strRelPath) :

    TContentTab(strPath, strRelPath, L"System Info", kCIDLib::False)
    , m_psasubEvTrigs(kCIDLib::False)
    , m_pwndClear(nullptr)
    , m_pwndCopy(nullptr)
    , m_pwndOutput(nullptr)
    , m_strmOut(4096UL)
    , m_strmIn(m_strmOut)
    , m_tmidMon(kCIDCtrls::tmidInvalid)
{
    m_tmStamp.strDefaultFormat(TTime::str24HHMMSS());
}

TMonEvTrigsTab::~TMonEvTrigsTab()
{
}


// ---------------------------------------------------------------------------
//  TMonEvTrigsTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TMonEvTrigsTab::CreateTab(TTabbedWnd& wndParent, const TString& strTabText)
{
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);
}


// ---------------------------------------------------------------------------
//  TMonEvTrigsTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TMonEvTrigsTab::AreaChanged(const  TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Resize our controls to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndOutput)
        AutoAdjustChildren(areaPrev, areaNew);
}


tCIDLib::TBoolean TMonEvTrigsTab::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls.
    TDlgDesc dlgdChildren;
    facCQCAdmin.bCreateDlgDesc(kCQCAdmin::ridTab_MonEvTrigs, dlgdChildren);

    // Create the children
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Do an initial auto-adjust to get them sized to fit us
    AutoAdjustChildren(dlgdChildren.areaPos(), areaClient());

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCAdmin::ridTab_MonEvTrigs_Clear, m_pwndClear);
    CastChildWnd(*this, kCQCAdmin::ridTab_MonEvTrigs_Copy, m_pwndCopy);
    CastChildWnd(*this, kCQCAdmin::ridTab_MonEvTrigs_Output, m_pwndOutput);

    // And register our button event handlers
    m_pwndClear->pnothRegisterHandler(this, &TMonEvTrigsTab::eClickHandler);
    m_pwndCopy->pnothRegisterHandler(this, &TMonEvTrigsTab::eClickHandler);

    // Remember the colors of the output window
    m_rgbOrgBgn = m_pwndOutput->rgbCurBgn();
    m_rgbOrgText = m_pwndOutput->rgbCurText();

    //
    //  Enable event trigger receiving while we are running. Sending is already enabled
    //  at program startup, so we are enabling both here. We don't want to turn off
    //  sending support. DO THIS BEFORE we start the timer.
    //
    facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Both, facCQCAdmin.sectUser());

    // Now subscribe to the topic that the above caused to be published
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

    // Start up our timer, make it fairly quick
    m_tmidMon = tmidStartTimer(100);

    return kCIDLib::True;
}


tCIDLib::TVoid TMonEvTrigsTab::Destroyed()
{
    // Stop our monitoring timer, before we disable event trigger receiving
    if (m_tmidMon != kCIDCtrls::tmidInvalid)
        StopTimer(m_tmidMon);

    // And stop event trigger receiving, leaving sending enabled
    facCQCKit().StartEventProcessing(tCQCKit::EEvProcTypes::Send, facCQCAdmin.sectUser());

    TParent::Destroyed();
}


tCIDLib::TVoid TMonEvTrigsTab::Timer(const tCIDCtrls::TTimerId tmidToDo)
{
    if (tmidToDo == m_tmidMon)
    {
        //
        //  While we have events available, up to a reasonable number, grab them, format
        //  them and output them to the text output window.
        //
        tCIDLib::TCard4 c4Count = 0;
        while (c4Count < 32)
        {
            // If we don't get anything, we are done
            if (!m_psasubEvTrigs.bGetNextMsg(m_psmsgTmp, 0))
                break;

            // Get the msg object out as its actual type
            const TCQCEvent& cevNew = m_psmsgTmp.objMsgAs<TCQCEvent>();

            // Output the time stamp in original text color
            m_tmStamp.SetToNow();
            m_strCurLine = L"[TIME: ";
            m_strCurLine.AppendFormatted(m_tmStamp);
            m_strCurLine.Append(kCIDLib::chCloseBracket);
            m_pwndOutput->SetLineColors
            (
                facCIDGraphDev().rgbDarkGrey, facCIDGraphDev().rgbWhite
            );
            m_pwndOutput->AddString(m_strCurLine);

            // Now format the event out and output those lines in dark grey
            m_strmOut.Reset();
            m_strmOut << cevNew;
            m_pwndOutput->SetLineColors(m_rgbOrgBgn, facCIDGraphDev().rgbDarkGrey);
            m_colLines.RemoveAll();

            //
            //  Reset the input stream and read in all the lines, dumping them to
            //  the text output window.
            //
            m_strmIn.Reset();
            while (!m_strmIn.bEndOfStream())
            {
                m_strmIn >> m_strCurLine;
                if (!m_strCurLine.bIsEmpty())
                    m_colLines.objAdd(m_strCurLine);
            }
            m_colLines.objAdd(TString::strEmpty());
            m_pwndOutput->AddStrings(m_colLines);
        }

        c4Count++;
    }
}


// ---------------------------------------------------------------------------
//  TMonEvTrigsTab: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDCtrls::EEvResponses TMonEvTrigsTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCAdmin::ridTab_MonEvTrigs_Clear)
    {
        m_pwndOutput->Clear();
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridTab_MonEvTrigs_Copy)
    {
        TString strText;
        m_pwndOutput->QueryText(strText);

        // Put the text on the clipboard
        try
        {
            TGUIClipboard gclipCopy(*this);
            gclipCopy.Clear();
            gclipCopy.StoreText(strText);
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , strWndText()
                , L"The text couldn't be copied to the clipboard."
                , errToCatch
            );
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

