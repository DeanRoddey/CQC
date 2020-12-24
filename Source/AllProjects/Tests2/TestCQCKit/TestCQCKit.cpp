//
// FILE NAME: TestCQCKit.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/11/2008
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
#include    "TestCQCKit.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TCQCKitTestApp,TTestFWApp)


// ---------------------------------------------------------------------------
//  CLASS: TCQCKitTestApp
// PREFIX: tfwapp
// ---------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  TCQCKitTestApp: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TCQCKitTestApp::bInitialize(TString&)
{
    // Nothing to do
    return kCIDLib::True;
}

// Load up our tests on our parent class
tCIDLib::TVoid TCQCKitTestApp::LoadTests()
{
    // Do the facility class tests
    AddTest(new TTest_Facility1);
    AddTest(new TTest_Facility2);

    // Do the connection configuration tests
    AddTest(new TTest_IPConnCfg);
    AddTest(new TTest_SerialConnCfg);
    AddTest(new TTest_UPnPConnCfg);

    // Do the field definition and value tests
    AddTest(new TTest_FldDef);
    AddTest(new TTest_FldValue);
    AddTest(new TTest_BoolFldVal);
    AddTest(new TTest_CardFldVal);
    AddTest(new TTest_FloatFldVal);
    AddTest(new TTest_IntFldVal);
    AddTest(new TTest_StrFldVal);
    AddTest(new TTest_StrListFldVal);

    // Do the expression tests
    AddTest(new TTest_Expressions);

    // Do the field storage tests
    AddTest(new TTest_FldStoreBase);
    AddTest(new TTest_FldStoreBool);

    // Do the field filter tests
    AddTest(new TTest_FldFilter);
    AddTest(new TTest_FldTypeFilter);

    // Do the event trigger tests
    AddTest(new TTest_EventTrig);
    AddTest(new TTest_EventTrigStd);

    // Do the action variables tests
    AddTest(new TTest_BasicActVar1);
    AddTest(new TTest_BasicActVar2);
    AddTest(new TTest_AdvActVar1);
    AddTest(new TTest_TimeStampVar);

    // Do the standard action targets tests
    AddTest(new TTest_VarTars1);

    // User account related tests
    AddTest(new TTest_UserAccount);
}

tCIDLib::TVoid TCQCKitTestApp::PostTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TCQCKitTestApp::PreTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TCQCKitTestApp::Terminate()
{
    // Nothing to do
}



// ----------------------------------------------------------------------------
//  Declare the test app object
// ----------------------------------------------------------------------------
TCQCKitTestApp   tfwappCQCKit;



// ----------------------------------------------------------------------------
//  Include magic main module code. We just point it at the test thread
//  entry point of the test framework app class.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"TestThread"
        , TMemberFunc<TCQCKitTestApp>(&tfwappCQCKit, &TCQCKitTestApp::eTestThread)
    )
)

