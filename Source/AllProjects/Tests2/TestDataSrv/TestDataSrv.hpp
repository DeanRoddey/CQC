//
// FILE NAME: TestDataSrv.hpp
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
//  This is the main header file of the data server test suite. The data server
//  is key to CQC, so we need to make sure it correctly manages the user's customization
//  data.
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
#include    "TestFWLib.hpp"
#include    "CQCKit.hpp"
#include    "CQCRemBrws.hpp"



// ---------------------------------------------------------------------------
//  Individual test section headers
// ---------------------------------------------------------------------------
#include    "TestDataSrv_Basic.hpp"



// ---------------------------------------------------------------------------
//  CLASS: TDataSrvTestApp
// PREFIX: tfwapp
//
//  This is our implementation of the test framework's test program framework.
//  We just create a derivative and override some methods.
// ---------------------------------------------------------------------------
class TDataSrvTestApp : public TTestFWApp
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TDataSrvTestApp();

        TDataSrvTestApp(const TDataSrvTestApp&) = delete;

        ~TDataSrvTestApp();


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TCQCSecToken    m_sectToUse;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInitialize
        (
                    TString&                strError
        )   override;

        tCIDLib::TVoid LoadTests() override;

        tCIDLib::TVoid PostTest
        (
            const   TTestFWTest&            tfwtFinished
        )   override;

        tCIDLib::TVoid PreTest
        (
            const   TTestFWTest&            tfwtStarting
        )   override;

        tCIDLib::TVoid Terminate() override;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TDataSrvTestApp,TTestFWApp)
};


// ---------------------------------------------------------------------------
//  The tests need access to our test app object
// ---------------------------------------------------------------------------
extern TDataSrvTestApp  tfwappCQCKit;
