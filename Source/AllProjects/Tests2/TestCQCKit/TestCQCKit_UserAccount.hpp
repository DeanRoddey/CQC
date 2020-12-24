//
// FILE NAME: TestCQCKit_UserAccount.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/21/2019
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
//  This is the header file for the user account tests.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  CLASS: TTest_UserAccount
// PREFIX: tfwt
// ---------------------------------------------------------------------------
class TTest_UserAccount : public TTestFWTest
{
    public  :
        // -------------------------------------------------------------------
        //  Constructor and Destructor
        // -------------------------------------------------------------------
        TTest_UserAccount();

        TTest_UserAccount(const TTest_UserAccount&) = delete;
        TTest_UserAccount(TTest_UserAccount&&) = delete;

        ~TTest_UserAccount();


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
        RTTIDefs(TTest_UserAccount,TTestFWTest)
};
