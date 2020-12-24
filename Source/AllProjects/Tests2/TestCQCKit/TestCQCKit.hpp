//
// FILE NAME: TestCQCKit.hpp
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
//  This is the main header file of the CQCKit test suite program. We have way
//  to many tests to put all of their class declarations here, as is done in
//  smaller test suites. So each section has it's own header file.
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
#include    "CIDXML.hpp"
#include    "CQCKit.hpp"
#include    "CQCAct.hpp"
#include    "TestFWLib.hpp"


// ---------------------------------------------------------------------------
//  Individual test section headers
// ---------------------------------------------------------------------------
#include    "TestCQCKit_Facility.hpp"
#include    "TestCQCKit_ConnCfg.hpp"
#include    "TestCQCKit_ActVars.hpp"
#include    "TestCQCKit_Expressions.hpp"
#include    "TestCQCKit_FldDef.hpp"
#include    "TestCQCKit_FldValues.hpp"
#include    "TestCQCKit_FldStorage.hpp"
#include    "TestCQCKit_EventTrig.hpp"
#include    "TestCQCKit_StdTargets.hpp"
#include    "TestCQCKit_FldFilter.hpp"
#include    "TestCQCKit_UserAccount.hpp"



// ---------------------------------------------------------------------------
//  CLASS: TCQCKitTest
// PREFIX: tfwapp
//
//  This is our implementation of the test framework's test program framework.
//  We just create a derivative and override some methods.
// ---------------------------------------------------------------------------
class TCQCKitTestApp : public TTestFWApp
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TCQCKitTestApp() = default;

        TCQCKitTestApp(const TCQCKitTestApp&);
        TCQCKitTestApp(TCQCKitTestApp&&);

        ~TCQCKitTestApp() = default;


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCKitTestApp& operator=(const TCQCKitTestApp&) = delete;
        TCQCKitTestApp& operator=(TCQCKitTestApp&&) = delete;


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


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCKitTestApp,TTestFWApp)
};

