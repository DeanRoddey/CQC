//
// FILE NAME: TestLogicSrv.hpp
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
//  This is the main header file of the logic server test suite program. We
//  test out the various types of fields that the logic server supports, to
//  make sure that they work correctly and evaluate the correct values.
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
#include    "CQLogicSh.hpp"


// ---------------------------------------------------------------------------
//  Individual test section headers
// ---------------------------------------------------------------------------
#include    "TestLogicSrv_Formula.hpp"



// ---------------------------------------------------------------------------
//  CLASS: TLogicSrvTestApp
// PREFIX: tfwapp
//
//  This is our implementation of the test framework's test program framework.
//  We just create a derivative and override some methods.
// ---------------------------------------------------------------------------
class TLogicSrvTestApp : public TTestFWApp
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TLogicSrvTestApp();

        TLogicSrvTestApp(const TLogicSrvTestApp&) = delete;

        ~TLogicSrvTestApp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInitialize
        (
                    TString&                strError
        )   override;

        tCIDLib::TVoid LoadTests();

        tCIDLib::TVoid PostTest
        (
            const   TTestFWTest&            tfwtFinished
        );

        tCIDLib::TVoid PreTest
        (
            const   TTestFWTest&            tfwtStarting
        );

        tCIDLib::TVoid Terminate();


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TLogicSrvTestApp,TTestFWApp)
};


