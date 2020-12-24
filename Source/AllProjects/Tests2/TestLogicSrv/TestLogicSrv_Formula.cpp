//
// FILE NAME: TestEvents_Formula.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/05/2011
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
//  This file implements the tests for the math formula field types
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
#include    "TestLogicSrv.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_Formula,TTestFWTest)




// ---------------------------------------------------------------------------
//  CLASS: TTest_Formula
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Formula: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Formula::TTest_Formula() :

    TTestFWTest
    (
        L"Math Formula Tests", L"Basic tests of Math formula type fields", 3
    )
{
}

TTest_Formula::~TTest_Formula()
{
}


// ---------------------------------------------------------------------------
//  TTest_Formula: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Formula::eRunTest(    TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  Set up a list of fields. Not that they all have to have at least
    //  one field reference, even if our simple tests of syntax don't always
    //  require one, so we provide a dummy one in some cases.
    //
    struct TTestInfo
    {
        const tCIDLib::TCh* const   pszFormula;
        tCQCKit::EFldTypes          eType;
        const tCIDLib::TCh* const   pszField1;
        const tCIDLib::TCh* const   pszField2;
    };

    //
    //  There has to be a source field, since it makes no sense to create a logic
    //  server field without one. But want to test various types of formulas, some
    //  of which reference no fields. So we just set the first field, doesn't matter
    //  to what.
    //
    const TTestInfo aTests[] =
    {
            { L"2", tCQCKit::EFldTypes::Card, L"Vars.TestCard", nullptr }
        ,   { L"2 + 2", tCQCKit::EFldTypes::Card, L"Vars.TestCard", nullptr }

        ,   { L"%(1)", tCQCKit::EFldTypes::Boolean, L"Vars.TestBool", nullptr }

        ,   { L"Power(2, 2)", tCQCKit::EFldTypes::Float, L"Vars.TestFloat", nullptr }
        ,   { L"Power(2, 2) + Cosine(%(1))", tCQCKit::EFldTypes::Float, L"Vars.TestFloat", nullptr }
        ,   { L"Sine(2 + 3, 2)", tCQCKit::EFldTypes::Float, L"Vars.TestFloat", nullptr }

        ,   { L"%(1) * %(2)", tCQCKit::EFldTypes::Float, L"Vars.TestFloat", L"Vars.TestInt" }
    };
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aTests);


    tCIDLib::TCard4 c4SrcFldInd;
    TString         strErr;

    // We need a field info cache to run these
    TCQCFldCache    cfcToUse;
    cfcToUse.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::ReadAndWrite));


    // Loop through the tests and run them
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TTestInfo& CurTest = aTests[c4Index];

        TCQSLLDMath clsftCur(L"Test", CurTest.eType);
        clsftCur.strFormula(CurTest.pszFormula);
        if (CurTest.pszField1)
            clsftCur.bAddField(CurTest.pszField1);
        if (CurTest.pszField2)
            clsftCur.bAddField(CurTest.pszField2);

        if (!clsftCur.bValidate(strErr, c4SrcFldInd, cfcToUse))
        {
            strmOut << TFWCurLn << L"Failed to validate formula: "
                    << CurTest.pszFormula << L"\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // It validated so actually run it and see if we get the right result

    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_Formula: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TTest_Formula::bTestFormula(const TCQSLLDMath& clsftToTest)
{


    return kCIDLib::True;
}


