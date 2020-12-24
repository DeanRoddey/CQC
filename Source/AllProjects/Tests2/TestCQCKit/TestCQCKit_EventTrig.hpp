//
// FILE NAME: TestCQCKit_EventTrig.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2013
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
//  This is the header file for the event trigger class. It provides a simple
//  structure of blocks with key/value pairs in them.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrigBase
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_EventTrigBase : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TTest_EventTrigBase() = delete;
        TTest_EventTrigBase(const TTest_EventTrigBase&) = delete;
        TTest_EventTrigBase(TTest_EventTrigBase&&) = delete;

        ~TTest_EventTrigBase();


    protected :
        // -------------------------------------------------------------------
        //  Hidden Constructors
        // -------------------------------------------------------------------
        TTest_EventTrigBase
        (
            const   TString&                strName
            , const TString&                strDescr
            , const tCIDLib::TCard4         c4Level
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eCheckExists
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TString&                strKey
            , const TCQCEvent&              cevTest
        )   const;

        tTestFWLib::ETestRes eCheckExists
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TString&                strKey
            , const tCIDLib::TBoolean       bExpValue
            , const TCQCEvent&              cevTest
        )   const;

        tTestFWLib::ETestRes eCheckExists
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TString&                strKey
            , const tCIDLib::TCard4         c4ExpValue
            , const TCQCEvent&              cevTest
        )   const;

        tTestFWLib::ETestRes eCheckExists
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TString&                strKey
            , const tCIDLib::TInt4          i4ExpValue
            , const TCQCEvent&              cevTest
        )   const;

        tTestFWLib::ETestRes eCheckExists
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TString&                strKey
            , const TString&                strExpValue
            , const TCQCEvent&              cevTest
        )   const;



    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_EventTrigBase,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrig
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_EventTrig : public TTest_EventTrigBase
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_EventTrig();

        TTest_EventTrig(const TTest_EventTrig&) = delete;
        TTest_EventTrig(TTest_EventTrig&&) = delete;

        ~TTest_EventTrig();


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
        RTTIDefs(TTest_EventTrig,TTest_EventTrigBase)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrigStd
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_EventTrigStd : public TTest_EventTrigBase
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_EventTrigStd();

        TTest_EventTrigStd(const TTest_EventTrigStd&) = delete;
        TTest_EventTrigStd(TTest_EventTrigStd&&) = delete;

        ~TTest_EventTrigStd();


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
        tTestFWLib::ETestRes eTestVal
        (
                    TTextStringOutStream&   strmOut
            , const tCIDLib::TCard4         c4Line
            , const TCQCEvent&              cevTest
            , const TString&                strSrc
            , const TString&                strSrc2
            , const TString&                strClass
            , const TString&                strKey1
            , const TString&                strVal1
            , const TString&                strKey2
            , const TString&                strVal2
            , const TString&                strKey3
            , const TString&                strVal3
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_EventTrigStd,TTest_EventTrigBase)
};


