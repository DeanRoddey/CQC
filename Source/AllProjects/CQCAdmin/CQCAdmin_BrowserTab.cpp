//
// FILE NAME: CQCAdmin_BrowserTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2015
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
//  This file implements the the back end browser tab that goes into the left side
//  tab pane.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TBrowserTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TBrowserTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TBrowserTab: Constructors and destructor
// ---------------------------------------------------------------------------
TBrowserTab::TBrowserTab() :

    TTabWindow
    (
        facCQCAdmin.strMsg(kCQCAMsgs::midTab_Browser)
        , TString::strEmpty()
        , kCIDLib::False
        , kCIDLib::True
    )
    , m_pwndBrowser(nullptr)
{
}

TBrowserTab::~TBrowserTab()
{
}


// ---------------------------------------------------------------------------
//  TBrowserTab:: Public, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TBrowserTab::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    // Just passit on to the browser window
    if (!m_pwndBrowser)
        return kCIDLib::False;

    return m_pwndBrowser->bProcessAccel(pMsgData);
}


// ---------------------------------------------------------------------------
//  TBrowserTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TBrowserTab::CreateTab(TTabbedWnd& wndParent)
{
    wndParent.c4CreateTab
    (
        this
        , facCQCAdmin.strMsg(kCQCAMsgs::midTab_Browser)
        , 0
        , kCIDLib::True
        , tCIDCtrls::EWndStyles::VisTabChild
        , tCIDCtrls::EExWndStyles::ControlParent
    );
}


tCIDLib::TVoid TBrowserTab::UpdateFile(const TString& strPath)
{
    m_pwndBrowser->FileUpdated(strPath);
}


TCQCTreeBrowser& TBrowserTab::wndBrowser()
{
    CIDAssert(m_pwndBrowser != nullptr, L"The browser window is not set");
    return *m_pwndBrowser;
}



// ---------------------------------------------------------------------------
//  TBrowserTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TBrowserTab::AreaChanged(const  TArea&                  areaPrev
                        , const TArea&                  areaNew
                        , const tCIDCtrls::EPosStates   ePosState
                        , const tCIDLib::TBoolean       bOrgChanged
                        , const tCIDLib::TBoolean       bSizeChanged
                        , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our tree control sized to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndBrowser)
        m_pwndBrowser->SetSize(areaClient().szArea(), kCIDLib::False);
}


tCIDLib::TBoolean TBrowserTab::bCreated()
{
    TParent::bCreated();

    //
    //  Create the browser window. We tell it in this case that we want it to add all of
    //  the browser handlers in this case because this is the full (unfiltered) browser we
    //  are creating.
    //
    m_pwndBrowser = new TCQCTreeBrowser();
    m_pwndBrowser->CreateBrowser
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , areaWndSize()
        , tCIDLib::eOREnumBits
          (
            tCIDCtrls::EWndStyles::VisTabGroupChild, tCIDCtrls::EWndStyles::ClipChildren
          )
        , tCIDLib::eOREnumBits
          (
            tCQCTreeBrws::EBrwsStyles::InvokeOnEnter, tCQCTreeBrws::EBrwsStyles::StdBrowsers
          )
        , facCQCAdmin.cuctxToUse()
    );

    // Register the main frame window to get events from the browser window
    facCQCAdmin.wndMain().RegisterBrowserEvs(*m_pwndBrowser);

    return kCIDLib::True;
}


tCIDLib::TVoid TBrowserTab::Destroyed()
{
    // Delete the browser window if we created it
    if (m_pwndBrowser)
    {
        try
        {
            m_pwndBrowser->Destroy();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
        }

        delete m_pwndBrowser;
        m_pwndBrowser = nullptr;
    }

    TParent::Destroyed();
}


