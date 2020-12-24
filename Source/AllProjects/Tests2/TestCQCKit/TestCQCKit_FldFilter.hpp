//
// FILE NAME: TestCQCKit_FldFilter.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/30/2010
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
//  This is the header file for the field filter tests. There are places
//  where we want to filter fields, mostly for presentation of lists of
//  fields to the user that meet some criteria.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//

// ---------------------------------------------------------------------------
//  CLASS: TTest_FldFilter
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldFilter : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldFilter();

        TTest_FldFilter(const TTest_FldFilter&) = delete;
        TTest_FldFilter(TTest_FldFilter&&) = delete;

        ~TTest_FldFilter();


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
        RTTIDefs(TTest_FldFilter,TTestFWTest)
};




// ---------------------------------------------------------------------------
//  CLASS: TTest_FldTypeFilter
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldTypeFilter : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldTypeFilter();

        TTest_FldTypeFilter(const TTest_FldTypeFilter&) = delete;
        TTest_FldTypeFilter(TTest_FldTypeFilter&&) = delete;

        ~TTest_FldTypeFilter();


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
        RTTIDefs(TTest_FldTypeFilter,TTestFWTest)
};


