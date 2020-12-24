//
// FILE NAME: TestEvents.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2009
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
#include    "TestEvents.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TEventTestApp,TTestFWApp)


// ---------------------------------------------------------------------------
//  CLASS: TEventTestApp
// PREFIX: tfwapp
// ---------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  TEventTestApp: Constructor and Destructor
// ----------------------------------------------------------------------------
TEventTestApp::TEventTestApp()
{
}

TEventTestApp::~TEventTestApp()
{
}


// ----------------------------------------------------------------------------
//  TEventTestApp: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TEventTestApp::bInitialize(TString&)
{
    // Nothing to do
    return kCIDLib::True;
}

// Load up our tests on our parent class
tCIDLib::TVoid TEventTestApp::LoadTests()
{
    AddTest(new TTest_Scheduled1);
    AddTest(new TTest_TrigEvFilters);
    AddTest(new TTest_TrigEvStream);
}

tCIDLib::TVoid TEventTestApp::PostTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TEventTestApp::PreTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TEventTestApp::Terminate()
{
    // Nothing to do
}



// ----------------------------------------------------------------------------
//  Declare the test app object
// ----------------------------------------------------------------------------
TEventTestApp   tfwappCQCKit;



// ----------------------------------------------------------------------------
//  Include magic main module code. We just point it at the test thread
//  entry point of the test framework app class.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"TestThread"
        , TMemberFunc<TEventTestApp>(&tfwappCQCKit, &TEventTestApp::eTestThread)
    )
)

