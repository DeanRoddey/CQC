//
// FILE NAME: TestEvents.hpp
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
//  This is the main header file of the event test suite program. We test
//  out the basic scheduled event triggered event classes, to the degree that
//  they can be in a standalone sort of way.
//
//  We have way to many tests to put all of their class declarations here, as
//  is done in smaller test suites. So each section has it's own header file.
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


// ---------------------------------------------------------------------------
//  Individual test section headers
// ---------------------------------------------------------------------------
#include    "TestEvents_Triggered.hpp"
#include    "TestEvents_Scheduled.hpp"



// ---------------------------------------------------------------------------
//  CLASS: TEventTestApp
// PREFIX: tfwapp
//
//  This is our implementation of the test framework's test program framework.
//  We just create a derivative and override some methods.
// ---------------------------------------------------------------------------
class TEventTestApp : public TTestFWApp
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TEventTestApp();

        TEventTestApp(const TEventTestApp&) = delete;

        ~TEventTestApp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bInitialize
        (
                    TString&                strErr
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
        RTTIDefs(TEventTestApp,TTestFWApp)
};


