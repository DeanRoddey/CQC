//
// FILE NAME: TestEvents_Formula.hpp
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
//  This is the header file for the math formula type logic server field
//  tests.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  CLASS: TTest_Template1
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_Template1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_Template1();

        TTest_Template1(const TTest_Template1&) = delete;
        TTest_Template1(TTest_Template1&&) = delete;

        ~TTest_Template1();


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
        RTTIDefs(TTest_Template1,TTestFWTest)
};


// ---------------------------------------------------------------------------
//  CLASS: TTest_TemplateMove
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_TemplateMove : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_TemplateMove();

        TTest_TemplateMove(const TTest_TemplateMove&) = delete;
        TTest_TemplateMove(TTest_TemplateMove&&) = delete;

        ~TTest_TemplateMove();


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
        RTTIDefs(TTest_TemplateMove,TTestFWTest)
};

