//
// FILE NAME: TestIntfEng.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/26/2020
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
#include    "TestIntfEng.hpp"


// ----------------------------------------------------------------------------
//  Magic macros
// ----------------------------------------------------------------------------
RTTIDecls(TIntfEngTestApp,TTestFWApp)


// ---------------------------------------------------------------------------
//  CLASS: TIntfEngTestApp
// PREFIX: tfwapp
// ---------------------------------------------------------------------------

// ----------------------------------------------------------------------------
//  TIntfEngTestApp: Constructor and Destructor
// ----------------------------------------------------------------------------
TIntfEngTestApp::TIntfEngTestApp()
{
}

TIntfEngTestApp::~TIntfEngTestApp()
{
}


// ----------------------------------------------------------------------------
//  TIntfEngTestApp: Public, inherited methods
// ----------------------------------------------------------------------------
tCIDLib::TBoolean TIntfEngTestApp::bInitialize(TString& strErr)
{
    // We need to set up to be able to download templates
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
tCIDLib::TVoid TIntfEngTestApp::LoadTests()
{
    AddTest(new TTest_Template1);
    AddTest(new TTest_TemplateMove);
}

tCIDLib::TVoid TIntfEngTestApp::PostTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TIntfEngTestApp::PreTest(const TTestFWTest&)
{
    // Nothing to do
}

tCIDLib::TVoid TIntfEngTestApp::Terminate()
{
    // Nothing to do
}



// ----------------------------------------------------------------------------
//  Declare the test app object
// ----------------------------------------------------------------------------
TIntfEngTestApp   tfwappIntfEng;



// ----------------------------------------------------------------------------
//  Include magic main module code. We just point it at the test thread
//  entry point of the test framework app class.
// ----------------------------------------------------------------------------
CIDLib_MainModule
(
    TThread
    (
        L"TestThread"
        , TMemberFunc<TIntfEngTestApp>(&tfwappIntfEng, &TIntfEngTestApp::eTestThread)
    )
)

