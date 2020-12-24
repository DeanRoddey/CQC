//
// FILE NAME: TestDataSrv_Basic.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/28/2018
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
//  This is the header file for basic tests. These just verify that we can read
//  and write each data type, and that serial numbers and time stamps get updated
//  appropriately when we write new data, and that we get new data or not based
//  on serial number we pass.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicIO
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_BasicIO : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_BasicIO();

        ~TTest_BasicIO();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        );


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_BasicIO,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_FileSys
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_FileSys : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_FileSys();

        ~TTest_FileSys();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        );


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_FileSys,TTestFWTest)
};



// ---------------------------------------------------------------------------
//  CLASS: TTest_MetaInfo
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_MetaInfo : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_MetaInfo();

        ~TTest_MetaInfo();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tTestFWLib::ETestRes eRunTest
        (
                    TTextStringOutStream&   strmOutput
            ,       tCIDLib::TBoolean&      bWarning
        );


    private :
        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_MetaInfo,TTestFWTest)
};

