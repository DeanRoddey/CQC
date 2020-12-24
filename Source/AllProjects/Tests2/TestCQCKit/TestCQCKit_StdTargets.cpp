//
// FILE NAME: TestCQCKit_StdTargets.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/23/2009
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
//  This file implements the tests for the standard action targets.
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
RTTIDecls(TTest_VarTars1,TTestFWTest)




// ---------------------------------------------------------------------------
//  CLASS: TTest_VarTars1
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_VarTars1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_VarTars1::TTest_VarTars1() :

    TTestFWTest
    (
        L"Var Targets 1", L"Basic variables action target tests", 3
    )
{
}

TTest_VarTars1::~TTest_VarTars1()
{
}


// ---------------------------------------------------------------------------
//  TTest_VarTars1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_VarTars1::eRunTest(TTextStringOutStream&  strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;


    // Test the variable name validation static method of the var targets
    {
        struct TTestItem
        {
            const tCIDLib::TCh*     pszName;
            const tCIDLib::TCh*     pszPref;
            tCIDLib::TBoolean       bShouldFail;
        };

        const TTestItem aTests[] =
        {
            // Positive tests
            { L"1:2"            , L"1"      , kCIDLib::False }
          , { L"12:34"          , L"12"     , kCIDLib::False }
          , { L"GVar:Test"      , L"GVar"   , kCIDLib::False }
          , { L"GVar:Test"      , L""       , kCIDLib::False }
          , { L"LVar:Test"      , L""       , kCIDLib::False }
          , { L"LVar:Test_Var"  , L""       , kCIDLib::False }
          , { L"LVar:Test-Var"  , L""       , kCIDLib::False }

            // Negative tests
          , { L""               , L""       , kCIDLib::True  }
          , { L"1"              , L"1"      , kCIDLib::True  }
          , { L"1:"             , L"1"      , kCIDLib::True  }
          , { L"1:_9"           , L"1"      , kCIDLib::True  }
          , { L" 1:9"           , L"1"      , kCIDLib::True  }
          , { L"1: 9"           , L"1"      , kCIDLib::True  }
          , { L"Gvar:9"         , L"GVar"   , kCIDLib::True  }
          , { L"Gvar:9"         , L""       , kCIDLib::True  }
        };
        const tCIDLib::TCard4 c4TestCnt = tCIDLib::c4ArrayElems(aTests);

        TString strErr;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TestCnt; c4Index++)
        {
            const TTestItem& itemCur = aTests[c4Index];

            const tCIDLib::TBoolean bRes = TStdVarsTar::bValidVarName
            (
                itemCur.pszName, itemCur.pszPref, strErr
            );

            if (bRes == itemCur.bShouldFail)
            {
                strmOut << TFWCurLn << L"Parse of variable name '"
                        << itemCur.pszName << L"' returned wrong result\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }
    }
    return eRes;
}

