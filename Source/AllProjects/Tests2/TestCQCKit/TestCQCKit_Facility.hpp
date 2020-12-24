//
// FILE NAME: TestCQCKit_Facility.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/23/2009
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
//  This is the header file for the tests of the CQCKit facility class. It
//  provides a lot of convenience and utility methods that are core to the
//  rest of the system. So we need to make sure that they work correctly.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_Facility1
// PREFIX: tfwt
//
//  This one does very basic tests of the methods that parse out things like
//  boolean values and field names and such.
// ---------------------------------------------------------------------------
class TTest_Facility1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_Facility1();

        TTest_Facility1(const TTest_Facility1&) = delete;
        TTest_Facility1(TTest_Facility1&&) = delete;

        ~TTest_Facility1();


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
        RTTIDefs(TTest_Facility1,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_Facility2
// PREFIX: tfwt
//
//  This one does some testing of the action parameter processing stuff of
//  the facility class. This one is marked as a fairly high level since we
//  want other stuff to be tested first. This guy has to make us of RTVs and
//  the variable targets and such.
// ---------------------------------------------------------------------------
class TTest_Facility2 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_Facility2();

        TTest_Facility2(const TTest_Facility2&) = delete;
        TTest_Facility2(TTest_Facility2&&) = delete;

        ~TTest_Facility2();


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
        RTTIDefs(TTest_Facility2,TTestFWTest)
};


