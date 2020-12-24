//
// FILE NAME: TestEvents_Triggered.hpp
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
//  This is the header file for the triggered event related tests. Triggered
//  events have some special case binary streaming issues since they have to
//  flattend anre resurrect a tree structure, so we do some simple round trip
//  streaming tests just to catch any possible issues.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_TrigEvStream
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_TrigEvStream : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_TrigEvStream();

        TTest_TrigEvStream(const TTest_TrigEvStream&) = delete;
        TTest_TrigEvStream(TTest_TrigEvStream&&) = delete;

        ~TTest_TrigEvStream();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_TrigEvStream& operator=(const TTest_TrigEvStream&) = delete;
        TTest_TrigEvStream& operator=(TTest_TrigEvStream&&) = delete;


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
        RTTIDefs(TTest_TrigEvStream,TTestFWTest)
};




// ---------------------------------------------------------------------------
//  CLASS: TTest_TrigEvFilters
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_TrigEvFilters : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_TrigEvFilters();

        TTest_TrigEvFilters(const TTest_TrigEvFilters&) = delete;
        TTest_TrigEvFilters(TTest_TrigEvFilters&&) = delete;

        ~TTest_TrigEvFilters();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_TrigEvFilters& operator=(const TTest_TrigEvFilters&) = delete;
        TTest_TrigEvFilters& operator=(TTest_TrigEvFilters&&) = delete;


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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eTestUserAction
        (
                    TTextStringOutStream&   strmOut
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_TrigEvFilters,TTestFWTest)
};

