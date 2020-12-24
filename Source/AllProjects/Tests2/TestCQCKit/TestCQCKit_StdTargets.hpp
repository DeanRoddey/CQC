//
// FILE NAME: TestCQCKit_StdTargets.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/22/2009
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
//  This is the header file for tests of the core command targets. These are
//  the ones always available.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_VarTars1
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_VarTars1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_VarTars1();

        TTest_VarTars1(const TTest_VarTars1&) = delete;
        TTest_VarTars1(TTest_VarTars1&&) = delete;

        ~TTest_VarTars1();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_VarTars1,TTestFWTest)
};

