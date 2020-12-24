//
// FILE NAME: TestCQCKit_UserAccount.cpp
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
//  This file implements the tests for the user account related tests.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Include underlying headers
// ---------------------------------------------------------------------------
#include    "TestCQCKit.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_UserAccount,TTestFWTest)


// ---------------------------------------------------------------------------
//  CLASS: TTest_UserAccount
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_UserAccount: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_UserAccount::TTest_UserAccount() :

    TTestFWTest
    (
        L"User Account", L"Tests of the user account class", 3
    )
{
}

TTest_UserAccount::~TTest_UserAccount()
{
}


// ---------------------------------------------------------------------------
//  TTest_UserAccount: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_UserAccount::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tCIDLib::TCard4         c4NextVarId = 1;
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;

    // Test default ctor
    {
        TCQCUserAccount uaccDef;

        // The type should be limited
        if (uaccDef.eRole() != tCQCKit::EUserRoles::LimitedUser)
        {
            strmOut << TFWCurLn << L"Default ctor did not set limited role\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (uaccDef.bLimitTime()
        ||  !uaccDef.strLoginName().bIsEmpty()
        ||  !uaccDef.strDescription().bIsEmpty()
        ||  !uaccDef.strDefInterfaceName().bIsEmpty()
        ||  !uaccDef.strWebPassword().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor set wrong values\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (uaccDef.strUserId().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor did not create new user id\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test one that we set up with info
    {
        TCQCUserAccount uaccInit
        (
             tCQCKit::EUserRoles::NormalUser
            , L"A test account"
            , L"MyLoginName"
            , L"MyPassword"
            , L"Bubba"
            , L"Jones"
        );

        // Test init stuff not set via the ctor
        if (uaccInit.bLimitTime()
        ||  !uaccInit.strDefInterfaceName().bIsEmpty()
        ||  !uaccInit.strWebPassword().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Init ctor did not init unset values\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // We can't test the password since only the hash is stored
        if ((uaccInit.eRole() != tCQCKit::EUserRoles::NormalUser)
        ||  (uaccInit.strDescription() != L"A test account")
        ||  (uaccInit.strLoginName() != L"MyLoginName")
        ||  (uaccInit.strFirstName() != L"Bubba")
        ||  (uaccInit.strLastName() != L"Jones"))
        {
            strmOut << TFWCurLn << L"Init ctor did not set correct info\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Test setting stuff not done via the ctor, or settable after ctor
        uaccInit.bLimitTime(kCIDLib::True);
        if (!uaccInit.bLimitTime())
        {
            strmOut << TFWCurLn << L"Failed to set login time limits\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
        uaccInit.bLimitTime(kCIDLib::False);
        if (uaccInit.bLimitTime())
        {
            strmOut << TFWCurLn << L"Failed to clear login time limits\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        uaccInit.strWebPassword(L"Testing123");
        if (uaccInit.strWebPassword() != L"Testing123")
        {
            strmOut << TFWCurLn << L"Failed to set web password\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        uaccInit.SetName(L"FirstName", L"LastName");
        if ((uaccInit.strFirstName() != L"FirstName")
        ||  (uaccInit.strLastName() != L"LastName"))
        {
            strmOut << TFWCurLn << L"Failed to set new name\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Do a binary streaming round trip
    {
        TCQCUserAccount uaccOrg
        (
             tCQCKit::EUserRoles::SystemAdmin
            , L"A system account"
            , L"BigCheese"
            , L"SuperSecret"
            , L"Senior"
            , L"Person"
        );

        tCIDLib::TCard4 c4Size;
        THeapBuf mbufTest(8192UL);
        {
            TBinMBufOutStream strmTar(&mbufTest);
            strmTar << uaccOrg << kCIDLib::FlushIt;
            c4Size = strmTar.c4CurSize();
        }

        TCQCUserAccount uaccNew;
        {
            TBinMBufInStream strmSrc(&mbufTest, c4Size);
            strmSrc >> uaccNew;
        }

        if (uaccNew != uaccOrg)
        {
            strmOut << TFWCurLn << L"User account failed to binary stream round trip\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}
