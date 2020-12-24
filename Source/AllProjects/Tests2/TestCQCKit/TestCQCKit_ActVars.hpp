//
// FILE NAME: TestCQCKit_ActVars.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2008
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
//  This is the header file for the action variables tests. This class is
//  one of the standard action targets, and it's one that can be tested
//  'easily' in a standalone sort of way, and it's heavily used in pretty
//  much all actions, it's well worth testing heavily.
//
//  The basic action variables test 1 just tests the TCQCActVar class itself in
//  the simplest, non-typed sort of way, i.e. all basic string typed variables
//  as would have been the case pre-2.5.
//
//  The based tests 2 also directly tests the variable class but tests out
//  the typed interfaces by creating variables of various types and making
//  sure that all the conversions work properly.
//
//  Advanced tests 1 does tests of the variables in the context of the action
//  variables target.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicActVar1
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_BasicActVar1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_BasicActVar1();

        TTest_BasicActVar1(const TTest_BasicActVar1&) = delete;
        TTest_BasicActVar1(TTest_BasicActVar1&&) = delete;

        ~TTest_BasicActVar1();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_BasicActVar1& operator=(const TTest_BasicActVar1&) = delete;
        TTest_BasicActVar1& operator=(TTest_BasicActVar1&&) = delete;


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
        tCIDLib::TBoolean bTestVar
        (
                    TTextStringOutStream&   strmOut
            , const TCQCActVar&             varToTest
            , const tCIDLib::TCh* const     pszExpKey
            , const tCIDLib::TCh* const     pszExpValue
            , const tCIDLib::TCh* const     pszExpLimit
            , const tCQCKit::EFldTypes      eExpType
            , const tCIDLib::TCard4         c4Line
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_BasicActVar1,TTestFWTest)
};




// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicActVar2
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_BasicActVar2 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_BasicActVar2();

        TTest_BasicActVar2(const TTest_BasicActVar2&) = delete;
        TTest_BasicActVar2(TTest_BasicActVar2&&) = delete;

        ~TTest_BasicActVar2();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_BasicActVar2& operator=(const TTest_BasicActVar2&) = delete;
        TTest_BasicActVar2& operator=(TTest_BasicActVar2&&) = delete;


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
        tCIDLib::TBoolean bTestVar
        (
                    TTextStringOutStream&   strmOut
            , const TCQCActVar&             varToTest
            , const tCIDLib::TCh* const     pszExpKey
            , const tCIDLib::TCh* const     pszExpValue
            , const tCIDLib::TCh* const     pszExpLimit
            , const tCQCKit::EFldTypes      eExpType
            , const tCIDLib::TCard4         c4Line
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_BasicActVar2,TTestFWTest)
};




// ---------------------------------------------------------------------------
//  CLASS: TTest_AdvActVar1
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_AdvActVar1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_AdvActVar1();

        TTest_AdvActVar1(const TTest_AdvActVar1&) = delete;
        TTest_AdvActVar1(TTest_AdvActVar1&&) = delete;

        ~TTest_AdvActVar1();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_AdvActVar1& operator=(const TTest_AdvActVar1&) = delete;
        TTest_AdvActVar1& operator=(TTest_AdvActVar1&&) = delete;


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
        tCIDLib::TBoolean bTestVar
        (
                    TTextStringOutStream&   strmOut
            , const TCQCActVar&             varToTest
            , const tCIDLib::TCh* const     pszExpKey
            , const tCIDLib::TCh* const     pszExpValue
            , const tCIDLib::TCard4         c4Line
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_AdvActVar1,TTestFWTest)
};


// ---------------------------------------------------------------------------
//  CLASS: TTest_TimeStampVar
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_TimeStampVar : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_TimeStampVar();

        TTest_TimeStampVar(const TTest_TimeStampVar&) = delete;
        TTest_TimeStampVar(TTest_TimeStampVar&&) = delete;

        ~TTest_TimeStampVar();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_TimeStampVar& operator=(const TTest_TimeStampVar&) = delete;
        TTest_TimeStampVar& operator=(TTest_TimeStampVar&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        )   final;


    private :
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_TimeStampVar,TTestFWTest)
};
