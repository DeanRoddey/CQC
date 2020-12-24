//
// FILE NAME: TestLogicSrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2011
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
//  This is the main implementation file of the test program.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// -----------------------------------------------------------------------------
//  Include underlying headers
// -----------------------------------------------------------------------------
#include    "TestLogicSrv.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TLogicSrvTestApp,TTestFWApp)


// ---------------------------------------------------------------------------
//  CLASS: TLogicSrvTestApp
// PREFIX: tfwapp
// ---------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  TLogicSrvTestApp: Constructor and Destructor
// ----------------------------------------------------------------------------
TLogicSrvTestApp::TLogicSrvTestApp()
{
}

TLogicSrvTestApp::~TLogicSrvTestApp()
{
}


// ----------------------------------------------------------------------------
//  TLogicSrvTestApp: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TLogicSrvTestApp::bInitialize(TString&)
{
    // All these tests are likely to require ORB access to init the client side
    facCIDOrb().InitClient();
    return kCIDLib::True;
}

// Load up our tests on our parent class
tCIDLib::TVoid TLogicSrvTestApp::LoadTests()
{
    AddTest(new TTest_Formula);
}

tCIDLib::TVoid TLogicSrvTestApp::PostTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TLogicSrvTestApp::PreTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TLogicSrvTestApp::Terminate()
{
    // Nothing to do
}



// ----------------------------------------------------------------------------
//  Declare the test app object
// ----------------------------------------------------------------------------
TLogicSrvTestApp   tfwappCQCKit;



// ----------------------------------------------------------------------------
//  Include magic main module code. We just point it at the test thread
//  entry point of the test framework app class.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"TestThread"
        , TMemberFunc<TLogicSrvTestApp>(&tfwappCQCKit, &TLogicSrvTestApp::eTestThread)
    )
)

