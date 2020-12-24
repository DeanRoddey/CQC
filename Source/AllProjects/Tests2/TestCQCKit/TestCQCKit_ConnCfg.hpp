//
// FILE NAME: TestCQCKit_ConnCfg.hpp
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
//  This is the header file for the connection configuration tests of the
//  CQCKit test suite.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_IPConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_IPConnCfg : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_IPConnCfg();

        TTest_IPConnCfg(const TTest_IPConnCfg&) = delete;
        TTest_IPConnCfg(TTest_IPConnCfg&&) = delete;

        ~TTest_IPConnCfg();


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
        RTTIDefs(TTest_IPConnCfg,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_SerialConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_SerialConnCfg : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_SerialConnCfg();

        TTest_SerialConnCfg(const TTest_SerialConnCfg&) = delete;
        TTest_SerialConnCfg(TTest_SerialConnCfg&&) = delete;

        ~TTest_SerialConnCfg();


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
        RTTIDefs(TTest_SerialConnCfg,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_UPnPConnCfg
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_UPnPConnCfg : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_UPnPConnCfg();

        TTest_UPnPConnCfg(const TTest_UPnPConnCfg&) = delete;
        TTest_UPnPConnCfg(TTest_UPnPConnCfg&&) = delete;

        ~TTest_UPnPConnCfg();


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
        RTTIDefs(TTest_UPnPConnCfg,TTestFWTest)
};



