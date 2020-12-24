//
// FILE NAME: TestCQCKit_Expression.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2008
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
//  This is the header file for the expression tests of the CQCKit tests.
//  Expressions are used to compare field values to a fixed value or a
//  regular expression or check to see if it meets certain criteria. They
//  are used in a number of places in CQC.
//
//  Note that these build on the statement stuff that is tested in the
//  context of the field value classes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_Expressions
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_Expressions : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_Expressions();

        TTest_Expressions(const TTest_Expressions&) = delete;
        TTest_Expressions(TTest_Expressions&&) = delete;

        ~TTest_Expressions();


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
        RTTIDefs(TTest_Expressions,TTestFWTest)
};


