//
// FILE NAME: CQCAdmin_ClientDrvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/19/2016
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
//  This file implements a generic tab for hosting client side driver interfaces.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_ClientDrvTab.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TClientDrvTab, TContentTab)



// ---------------------------------------------------------------------------
//  CLASS: TClientDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TClientDrvTab: Constructors and destructor
// ---------------------------------------------------------------------------
TClientDrvTab::TClientDrvTab(const  TString&    strPath
                            , const TString&    strRelPath
                            , const TString&    strHost
                            , const TString&    strMoniker) :

    TContentTab(strPath, strRelPath, L"Driver Client", kCIDLib::True)
    , m_pwndDriver(nullptr)
    , m_strHost(strHost)
    , m_strMoniker(strMoniker)
{
}

TClientDrvTab::~TClientDrvTab()
{
}


// ---------------------------------------------------------------------------
//  TClientDrvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid
TClientDrvTab::CreateTab(       TTabbedWnd&         wndParent
                        , const TString&            strTabText
                        , const TCQCDriverObjCfg&   cqcdcInfo)
{
    // Now we can create the underlying tab
    wndParent.c4CreateTab(this, strTabText, 0, kCIDLib::True);

    //
    //  Let's try to create our client window. CQCIGKit has a helper that does all
    //  of the work required. If we are running in the development enviornment, then
    //  tell him that. He won't download drivers, he'll just load them from where
    //  they were built.
    //
    try
    {
        m_pwndDriver = facCQCIGKit().pwndLoadNewDriver
        (
            *this
            , areaClient()
            , kCIDCtrls::widFirstCtrl
            , cqcdcInfo
            , facCQCAdmin.cuctxToUse()
            , facCQCKit().bDevMode()
        );
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TExceptDlg dlgErr
        (
            wndParent
            , L"Driver Client Error"
            , L"An error occurred while creating the client driver window."
            , errToCatch
        );
    }

    catch(...)
    {
        TErrBox msgbErr(L"An unknown exception occurred while creating the driver window.");
        msgbErr.ShowIt(wndParent);
        return;
    }

    // It worked so start it processing and make it visible
    m_pwndDriver->StartDriver();
    m_pwndDriver->SetVisibility(kCIDLib::True);
}



// ---------------------------------------------------------------------------
//  TClientDrvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TClientDrvTab::AreaChanged( const   TArea&                  areaPrev
                            , const TArea&                  areaNew
                            , const tCIDCtrls::EPosStates   ePosState
                            , const tCIDLib::TBoolean       bOrgChanged
                            , const tCIDLib::TBoolean       bSizeChanged
                            , const tCIDLib::TBoolean       bStateChanged)
{
    // Call our parent first
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    // Keep our driver window sized to fit
    if ((ePosState != tCIDCtrls::EPosStates::Minimized) && bSizeChanged && m_pwndDriver)
        m_pwndDriver->SetSize(areaClient().szArea(), kCIDLib::True);
}


tCIDLib::TVoid TClientDrvTab::Destroyed()
{
    // Delete the driver window if we created it
    if (m_pwndDriver)
    {
        // Stop the driver first
        m_pwndDriver->StopDriver();

        // And now clean up the window
        try
        {
            if (m_pwndDriver->bIsValid())
                m_pwndDriver->Destroy();

            delete m_pwndDriver;
            m_pwndDriver= nullptr;
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , L"Client Driver"
                , L"An error occurred while cleaning up the client driver"
                , errToCatch
            );
        }
    }

    TParent::Destroyed();
}


//
//  The main window calls us here if the user asks us to save changes. We do our work
//  by way of virtual methods on the driver client window base class that we hold the
//  driver window by.
//
tCIDLib::ESaveRes
TClientDrvTab::eDoSave(         TString&                strErr
                        , const tCQCAdmin::ESaveModes   eMode
                        ,       tCIDLib::TBoolean&      bChangesSaved)
{
    const tCIDLib::TBoolean bChanges = m_pwndDriver->bChanges();
    if (eMode == tCQCAdmin::ESaveModes::Test)
    {
        if (bChanges)
            return tCIDLib::ESaveRes::OK;
        return tCIDLib::ESaveRes::NoChanges;
    }

    // We need to really save the changes, so ask the driver to do that
    if (bChanges)
    {
        if (!m_pwndDriver->bSaveChanges(strErr))
            return tCIDLib::ESaveRes::Errors;

        bChangesSaved = kCIDLib::True;
    }

    // No more changes now one way or another
    return tCIDLib::ESaveRes::NoChanges;
}

