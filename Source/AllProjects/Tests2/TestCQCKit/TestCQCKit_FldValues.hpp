//
// FILE NAME: TestCQCKit_FldValues.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/12/2008
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
//  This is the header file for the field value classes. Drivers and other
//  CQC programs have a need to store the values of fields in a polymorphic
//  sort of way, and these classes provide that storage. There's a base
//  class which we test first, and then one derivative for each type of
//  field.
//
//  Since we want to test that each derived class appropriately passes on
//  constructor parameters, we have to test the base class values in each
//  drived class' constructor tests. So we provide a common test class that
//  all of the other tests here are derived from. it provides some protected
//  helpers for doing that stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_FldValComm
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldValComm : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  A structure used to pass in information for testing the statement
        //  evaluation stuff on each derivative. The bass class provides the
        //  grunt work of the test, and the derivatives just provide the
        //  inputs and expected outcomes.
        // -------------------------------------------------------------------
        struct TStmtTest
        {
            const   tCIDLib::TCh*   pszTestVal;
            tCQCKit::EStatements    eType;
            tCQCKit::EStmtRes       eExpRes;
        };


        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTest_FldValComm() = delete;

        TTest_FldValComm(const TTest_FldValComm&) = delete;
        TTest_FldValComm(TTest_FldValComm&&) = delete;

        ~TTest_FldValComm();


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructor
        // -------------------------------------------------------------------
        TTest_FldValComm
        (
            const   TString&                strName
            , const TString&                strDescr
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bTestBaseStuff
        (
                    TTextOutStream&         strmOut
            , const TTFWCurLn&              clnAt
            , const TCQCFldValue&           fvToTest
            , const tCQCKit::EFldTypes      eType
            , const tCIDLib::TCard4         c4DriverId
            , const tCIDLib::TCard4         c4FieldId
            , const tCIDLib::TCard4         c4SerialNum
            , const tCIDLib::TBoolean       bInError
        );

        tCIDLib::TBoolean bTestStatements
        (
                    TTextOutStream&         strmOut
            , const TTFWCurLn&              clnAt
            , const TCQCFldValue&           fvToTest
            , const TStmtTest* const        pTests
            , const tCIDLib::TCard4         c4Count
        );


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_FldValComm,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldValue
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FldValue : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FldValue();

        TTest_FldValue(const TTest_FldValue&) = delete;
        TTest_FldValue(TTest_FldValue&&) = delete;

        ~TTest_FldValue();


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
        RTTIDefs(TTest_FldValue,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_BoolFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_BoolFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_BoolFldVal();

        TTest_BoolFldVal(const TTest_BoolFldVal&) = delete;
        TTest_BoolFldVal(TTest_BoolFldVal&&) = delete;

        ~TTest_BoolFldVal();


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
        RTTIDefs(TTest_BoolFldVal,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_CardFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_CardFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_CardFldVal();

        TTest_CardFldVal(const TTest_CardFldVal&) = delete;
        TTest_CardFldVal(TTest_CardFldVal&&) = delete;

        ~TTest_CardFldVal();


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
        RTTIDefs(TTest_CardFldVal,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_FloatFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FloatFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FloatFldVal();

        TTest_FloatFldVal(const TTest_FloatFldVal&) = delete;
        TTest_FloatFldVal(TTest_FloatFldVal&&) = delete;

        ~TTest_FloatFldVal();


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
        RTTIDefs(TTest_FloatFldVal,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_IntFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_IntFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_IntFldVal();

        TTest_IntFldVal(const TTest_IntFldVal&) = delete;
        TTest_IntFldVal(TTest_IntFldVal&&) = delete;

        ~TTest_IntFldVal();


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
        RTTIDefs(TTest_IntFldVal,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_StrFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_StrFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_StrFldVal();

        TTest_StrFldVal(const TTest_StrFldVal&) = delete;
        TTest_StrFldVal(TTest_StrFldVal&&) = delete;

        ~TTest_StrFldVal();


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
        RTTIDefs(TTest_StrFldVal,TTest_FldValComm)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_StrListFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_StrListFldVal : public TTest_FldValComm
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_StrListFldVal();

        TTest_StrListFldVal(const TTest_StrListFldVal&) = delete;
        TTest_StrListFldVal(TTest_StrListFldVal&&) = delete;

        ~TTest_StrListFldVal();


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
        tCIDLib::TBoolean bCompLists
        (
            const   tCIDLib::TStrList&      col1
            , const tCIDLib::TStrList&      col2
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_StrListFldVal,TTest_FldValComm)
};


