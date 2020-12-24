//
// FILE NAME: TestIntfEng.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 1/26/2020
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
//  This is the main header file of the user interface test suite program. There's
//  a lot of stuff we could test, but it'll take a while to get all that into
//  place.
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
#include    "CQCIntfEng.hpp"


// ---------------------------------------------------------------------------
//  Individual test section headers
// ---------------------------------------------------------------------------
#include    "TestIntfEng_Template.hpp"



// ---------------------------------------------------------------------------
//  CLASS: TIntfEngTestApp
// PREFIX: tfwapp
//
//  This is our implementation of the test framework's test program framework.
//  We just create a derivative and override some methods.
// ---------------------------------------------------------------------------
class TIntfEngTestApp : public TTestFWApp
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TIntfEngTestApp();

        TIntfEngTestApp(const TIntfEngTestApp&) = delete;

        ~TIntfEngTestApp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInitialize
        (
                    TString&                strError
        )   final;

        tCIDLib::TVoid LoadTests() final;

        tCIDLib::TVoid PostTest
        (
            const   TTestFWTest&            tfwtFinished
        )   final;

        tCIDLib::TVoid PreTest
        (
            const   TTestFWTest&            tfwtStarting
        )   final;

        tCIDLib::TVoid Terminate() final;


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TCQCSecToken    m_sectToUse;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfEngTestApp,TTestFWApp)
};


// ---------------------------------------------------------------------------
//  The tests need access to our test app object
// ---------------------------------------------------------------------------
extern TIntfEngTestApp  tfwappIntfEng;
