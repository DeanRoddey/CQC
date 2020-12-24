//
// FILE NAME: CppDrvTest_ClDrvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/03/2018
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
//  This file implements the tab window that contains the client side driver
//  content, if there is one.
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
RTTIDecls(TClDrvTab, TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TClDrvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
TClDrvTab::TClDrvTab() :

    TTabWindow(L"Client Driver", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_pwndDriver(nullptr)
{
}

TClDrvTab::~TClDrvTab()
{
    // Just in case
    bStopDriver();
}


// ---------------------------------------------------------------------------
//  TClDrvTab: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TClDrvTab::bLoadDriver()
{
    // If we already have a window, that's bad
    if (m_pwndDriver)
    {
        TErrBox msgbErr(L"The client driver window already exists");
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }


    //
    //  Let's try to load the driver. CQCIGKit has a helper that will do all of the
    //  work.
    //
    try
    {
        m_pwndDriver = facCQCIGKit().pwndLoadNewDriver
        (
            *this
            , areaClient()
            , kCIDCtrls::widFirstCtrl
            , facCppDrvTest.cqcdcSession()
            , facCppDrvTest.cuctxToUse()
            , facCQCKit().bDevMode()
        );

        // Now we can show it and start the driver processing.
        m_pwndDriver->SetVisibility(kCIDLib::True);
        m_pwndDriver->StartDriver();
    }

    catch(TError& errToCatch)
    {
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);
        return kCIDLib::False;
    }

    return kCIDLib::True;
}



tCIDLib::TBoolean TClDrvTab::bStopDriver()
{
    if (m_pwndDriver)
    {
        try
        {
            // Stop the driver first
            m_pwndDriver->StopDriver();
        }

        catch(TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);

            TExceptDlg dlgErr
            (
                *this
                , facCppDrvTest.strMsg(kCppDTErrs::errcClDrv_CantStop)
                , errToCatch
            );
            return kCIDLib::False;
        }

        // Now we can clean up the window
        m_pwndDriver->Destroy();
        delete m_pwndDriver;
        m_pwndDriver = nullptr;
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TClDrvTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TClDrvTab::AreaChanged(const    TArea&                  areaPrev
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
    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndDriver)
        m_pwndDriver->SetSize(areaClient().szArea(), kCIDLib::True);
}


//
//  We try to get our client side driver loaded. If not, then we just continue on
//  with no content.
//
tCIDLib::TBoolean TClDrvTab::bCreated()
{
    TParent::bCreated();


    return kCIDLib::True;
}
