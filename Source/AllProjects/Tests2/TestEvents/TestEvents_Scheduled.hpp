//
// FILE NAME: TestEvents_Scheduled.hpp
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
//  This is the header file for the scheduled event tests file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//



// ---------------------------------------------------------------------------
//  CLASS: TTest_Scheduled1
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_Scheduled1 : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_Scheduled1();

        TTest_Scheduled1(const TTest_Scheduled1&) = delete;
        TTest_Scheduled1(TTest_Scheduled1&&) = delete;

        ~TTest_Scheduled1();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTest_Scheduled1& operator=(const TTest_Scheduled1&) = delete;
        TTest_Scheduled1& operator=(TTest_Scheduled1&&) = delete;


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
        tCIDLib::TBoolean bCheckInfo
        (
                    TTextStringOutStream&   strmOut
            , const TTFWCurLn&              tfwclAt
            , const TCQCSchEvent&           csrcToCheck
            , const tCQCKit::ESchTypes      eType
            , const tCIDLib::TEncodedTime   enctNext
        );

        tCIDLib::TVoid CalcNext
        (
                    TCQCSchEvent&           csrcTar
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTest_Scheduled1,TTestFWTest)
};

