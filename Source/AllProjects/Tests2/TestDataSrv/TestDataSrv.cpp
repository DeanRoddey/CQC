//
// FILE NAME: TestDataSrv.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/28/2018
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
#include    "TestDataSrv.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TDataSrvTestApp,TTestFWApp)


// ---------------------------------------------------------------------------
//  CLASS: TDataSrvTestApp
// PREFIX: tfwapp
// ---------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  TDataSrvTestApp: Constructor and Destructor
// ----------------------------------------------------------------------------
TDataSrvTestApp::TDataSrvTestApp()
{
}

TDataSrvTestApp::~TDataSrvTestApp()
{
}


// ----------------------------------------------------------------------------
//  TDataSrvTestApp: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TDataSrvTestApp::bInitialize(TString& strErr)
{
    // All these tests are likely to require ORB access so init the client side
    facCIDOrb().InitClient();

    //
    //  We have to have logic info in the environment since many of our tests
    //  require a valid login.
    //
    tCQCKit::ELoginRes  eLoginRes;
    TCQCUserAccount uaccMe;
    if (!facCQCKit().bDoEnvLogin(m_sectToUse, uaccMe, eLoginRes)
    &&  (eLoginRes == tCQCKit::ELoginRes::Ok))
    {
        strErr = L"No login info was set in the environment";
        return kCIDLib::False;
    }

    if (eLoginRes != tCQCKit::ELoginRes::Ok)
    {
        strErr = L"Got login result: ";
        strErr.Append(tCQCKit::strXlatELoginRes(eLoginRes));
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

// Load up our tests on our parent class
tCIDLib::TVoid TDataSrvTestApp::LoadTests()
{
    AddTest(new TTest_FileSys);
    AddTest(new TTest_BasicIO);
    AddTest(new TTest_MetaInfo);
}

tCIDLib::TVoid TDataSrvTestApp::PostTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TDataSrvTestApp::PreTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TDataSrvTestApp::Terminate()
{
    // Nothing to do
}



// ----------------------------------------------------------------------------
//  Declare the test app object
// ----------------------------------------------------------------------------
TDataSrvTestApp   tfwappCQCKit;



// ----------------------------------------------------------------------------
//  Include magic main module code. We just point it at the test thread
//  entry point of the test framework app class.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"TestThread"
        , TMemberFunc<TDataSrvTestApp>(&tfwappCQCKit, &TDataSrvTestApp::eTestThread)
    )
)

