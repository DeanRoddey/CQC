//
// FILE NAME: TestCQCKit_Expression.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/28/2008
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
//  This file implements the tests for the expression tests.
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
RTTIDecls(TTest_Expressions,TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_Expressions
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Expressions: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Expressions::TTest_Expressions() :

    TTestFWTest
    (
        L"Expression Tests", L"Tests the TCQCExpression class", 2
    )
{
}

TTest_Expressions::~TTest_Expressions()
{
}


// ---------------------------------------------------------------------------
//  TTest_Expressions: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Expressions::eRunTest(TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;
    TString                 strTmp;


    //
    //  Create a global variables target and set up some test variables on it
    //  that we can reference in some tests below to validate the use of variables
    //  in the comparison field of expressions.
    //
    TStdVarsTar ctarVars(tCIDLib::EMTStates::Unsafe, kCIDLib::False);
    ctarVars.bAddOrUpdateVar(L"GVar:Test1", L"16", tCQCKit::EFldTypes::Card, TString::strEmpty());
    ctarVars.bAddOrUpdateVar(L"GVar:Test2", L"_", tCQCKit::EFldTypes::String, TString::strEmpty());


    //
    //  Test the static method that indicates whether a given statement type
    //  requires a text value. We just create an array of booleans that
    //  indicates the expected state for each type.
    //
    {
        static tCIDLib::TBoolean abNeedsTxt[tCIDLib::c4EnumOrd(tCQCKit::EStatements::Count)] =
        {
            kCIDLib::False
            , kCIDLib::True
            , kCIDLib::True
            , kCIDLib::False
            , kCIDLib::False
            , kCIDLib::True
            , kCIDLib::True
            , kCIDLib::True
            , kCIDLib::True
            , kCIDLib::False
            , kCIDLib::False
            , kCIDLib::False

            , kCIDLib::False
            , kCIDLib::False
            , kCIDLib::True
            , kCIDLib::False
            , kCIDLib::False
        };
        const tCIDLib::TCard4 c4NTListSz = tCIDLib::c4ArrayElems(abNeedsTxt);

        // Make sure that our list is still in sync
        if (c4NTListSz == tCIDLib::c4EnumOrd(tCQCKit::EStatements::Count))
        {
            tCQCKit::EStatements eStmt = tCQCKit::EStatements::FirstUsed;
            while (eStmt <= tCQCKit::EStatements::Max)
            {
                if (TCQCExpression::bNeedsCompVal(eStmt) != abNeedsTxt[tCIDLib::c4EnumOrd(eStmt)])
                {
                    strmOut << TFWCurLn << L"Incorrect 'needs text' status for "
                            << tCQCKit::strXlatEStatements(eStmt)
                            << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
                eStmt++;
            }
        }
         else
        {
            strmOut << TFWCurLn << L"The test array is out of sync with the enum\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }


    //
    //  We'll also test the validation method as we go, since we create
    //  different types of expressions along the way, so we just validate
    //  each one and all of them should pass.
    //

    // Test the constructors
    {
        TCQCExpression exprTest;
        if (exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::None)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::None)
        ||  !exprTest.strCompVal().bIsEmpty()
        ||  !exprTest.strDescription().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor created incorrect state\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::String))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCExpression exprTest(L"This is a description");
        if (exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::None)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::None)
        ||  !exprTest.strCompVal().bIsEmpty()
        ||  (exprTest.strDescription() != L"This is a description"))
        {
            strmOut << TFWCurLn << L"Ctor created incorrect state\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::String))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCExpression exprTest
        (
            L"The Description"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsGThan
            , L"10"
            , kCIDLib::True
        );

        if (!exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::Statement)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::IsGThan)
        ||  (exprTest.strCompVal() != L"10")
        ||  (exprTest.strDescription() != L"The Description"))
        {
            strmOut << TFWCurLn << L"Ctor created incorrect state\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Int))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the set methods
    {
        TCQCExpression exprTest;
        exprTest.Set
        (
            L"Some Description"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsLsThan
            , L"10.0"
            , kCIDLib::False
        );

        if (exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::Statement)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::IsLsThan)
        ||  (exprTest.strCompVal() != L"10.0")
        ||  (exprTest.strDescription() != L"Some Description"))
        {
            strmOut << TFWCurLn << L"Set method failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Float))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the set methods
    {
        TCQCExpression exprTest(L"My Description");
        exprTest.Set
        (
            tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsTrue
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
        );

        if (exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::Statement)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::IsTrue)
        ||  !exprTest.strCompVal().bIsEmpty()
        ||  (exprTest.strDescription() != L"My Description"))
        {
            strmOut << TFWCurLn << L"Set method failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Boolean))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the individual get/set methods
    {
        TCQCExpression exprTest;

        exprTest.bNegated(kCIDLib::True);
        exprTest.eType(tCQCKit::EExprTypes::RegEx);
        exprTest.strCompVal(L"A|B");
        exprTest.strDescription(L"It's A or B, Man");

        if (!exprTest.bNegated()
        ||  (exprTest.eType() != tCQCKit::EExprTypes::RegEx)
        ||  (exprTest.eStatement() != tCQCKit::EStatements::None)
        ||  (exprTest.strCompVal() != L"A|B")
        ||  (exprTest.strDescription() != L"It's A or B, Man"))
        {
            strmOut << TFWCurLn << L"Setter methods failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::String))
        {
            strmOut << TFWCurLn << L"Expression did not validate\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        //
        //  Set to statement type, then back to reg ex. This should
        //  clear the statement type back out.
        //
        exprTest.Set
        (
            tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsTrue
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
        );

        exprTest.eType(tCQCKit::EExprTypes::RegEx);
        if (exprTest.eStatement() != tCQCKit::EStatements::None)
        {
            strmOut << TFWCurLn << L"Statement type should have been cleared\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Make sure we catch validation errors. So create some incorrect
    //  ones and test them.
    //
    {
        // Can't have comparison text for IsFalse/IsTrue
        TCQCExpression exprTest
        (
            L"A Description"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsFalse
            , L"A.*C"
            , kCIDLib::True
        );

        if (exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Boolean))
        {
            strmOut << TFWCurLn << L"Expression should not have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        //
        //  Settng the statement type will clear the comp value if not used, so
        //  this one should validate.
        //
        exprTest.eStatement(tCQCKit::EStatements::IsTrue);
        if (!exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Boolean))
        {
            strmOut << TFWCurLn << L"Expression should have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        // Most types of fields don't support positive/negative
        TCQCExpression exprTest
        (
            L"A Description"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsPositive
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
        );

        if (exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Boolean))
        {
            strmOut << TFWCurLn << L"Expression should not have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Card))
        {
            strmOut << TFWCurLn << L"Expression should not have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (exprTest.bValidate(strTmp, tCQCKit::EFldTypes::String))
        {
            strmOut << TFWCurLn << L"Expression should not have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (exprTest.bValidate(strTmp, tCQCKit::EFldTypes::Time))
        {
            strmOut << TFWCurLn << L"Expression should not have validated\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }


    //
    //  Test round trip copy and assignment. They don't have equality
    //  operators, so we test them manually.
    //
    {
        TCQCExpression exprSrc
        (
            L"A Description"
            , tCQCKit::EExprTypes::RegEx
            , tCQCKit::EStatements::None
            , L"A.*C"
            , kCIDLib::True
        );

        TCQCExpression exprCopy(exprSrc);
        TCQCExpression exprAssign;
        exprAssign = exprCopy;

        if ((exprSrc.bNegated() != exprCopy.bNegated())
        ||  (exprSrc.eType() != exprCopy.eType())
        ||  (exprSrc.eStatement() != exprCopy.eStatement())
        ||  (exprSrc.strCompVal() != exprCopy.strCompVal())
        ||  (exprSrc.strDescription() != exprCopy.strDescription()))
        {
            strmOut << TFWCurLn << L"Round trip copy failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((exprSrc.bNegated() != exprAssign.bNegated())
        ||  (exprSrc.eType() != exprAssign.eType())
        ||  (exprSrc.eStatement() != exprAssign.eStatement())
        ||  (exprSrc.strCompVal() != exprAssign.strCompVal())
        ||  (exprSrc.strDescription() != exprAssign.strDescription()))
        {
            strmOut << TFWCurLn << L"Round trip assignment failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // And test round trip streaming
    {
        TCQCExpression exprOrg
        (
            L"A Description"
            , tCQCKit::EExprTypes::RegEx
            , tCQCKit::EStatements::None
            , L"A.*C"
            , kCIDLib::True
        );

        TBinMBufOutStream strmWrite(8192UL);
        TBinMBufInStream strmRead(strmWrite);
        strmWrite << exprOrg << kCIDLib::FlushIt;
        strmRead.Reset();

        TCQCExpression exprNew;
        strmRead >> exprNew;

        if ((exprOrg.bNegated() != exprNew.bNegated())
        ||  (exprOrg.eType() != exprNew.eType())
        ||  (exprOrg.eStatement() != exprNew.eStatement())
        ||  (exprOrg.strCompVal() != exprNew.strCompVal())
        ||  (exprOrg.strDescription() != exprNew.strDescription()))
        {
            strmOut << TFWCurLn << L"Round trip streaming\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }


    //
    //  And finally evaluate some expressions. We set up an array of
    //  test scenarios that provides the input states, and the expected
    //  result.
    //
    {
        //
        //  We have to have a field value to test against. So we set up
        //  a few and the tests just point to them.
        //
        static TCQCBoolFldValue fvBoolFalse(1, 1);
        fvBoolFalse.bSetValue(kCIDLib::False);

        static TCQCBoolFldValue fvBoolTrue(1, 2);
        fvBoolTrue.bSetValue(kCIDLib::True);

        static TCQCCardFldValue fvCard(1, 3);
        fvCard.bSetValue(16);

        static TCQCIntFldValue fvIntNeg(1, 4);
        fvIntNeg.bSetValue(-43);

        static TCQCIntFldValue fvIntPos(1, 5);
        fvIntPos.bSetValue(94);

        static TCQCFloatFldValue fvFloatPos(1, 6);
        fvFloatPos.bSetValue(149.1);

        static TCQCFloatFldValue fvFloatNeg(1, 7);
        fvFloatNeg.bSetValue(-0.301);

        static TCQCStringFldValue fvString(1, 8);
        fvString.bSetValue(L"Test String");

        static TCQCStringFldValue fvFalseString(1, 9);
        fvFalseString.bSetValue(kCQCKit::pszFld_False);

        static TCQCStringFldValue fvTrueString(1, 10);
        fvTrueString.bSetValue(kCQCKit::pszFld_True);

        static TCQCCardFldValue fvZeroCard(1, 11);
        fvZeroCard.bSetValue(0);

        static TCQCIntFldValue fvZeroInt(1, 12);
        fvZeroInt.bSetValue(0);

        static TCQCFloatFldValue fvZeroFloat(1, 13);
        fvZeroFloat.bSetValue(0);

        static TCQCStringFldValue fvZeroString(1, 13);
        fvZeroString.bSetValue(kCIDLib::pszEmptyZStr);

        static TCQCTimeFldValue fvNowTime(1, 14);
        fvNowTime.bSetValue(TTime::enctNow());

        static TCQCTimeFldValue fvZeroTime(1, 15);
        fvZeroTime.bSetValue(0);

        static TCQCCardFldValue fvCardOdd(1, 16);
        fvCardOdd.bSetValue(15);

        static TCQCStringFldValue fvNumString(1, 17);
        fvNumString.bSetValue(L"A18231");


        // Define the test struct and set up an array of them
        struct TTest
        {
            tCQCKit::EExprTypes     eExprType;
            tCQCKit::EStatements    eStatement;
            const tCIDLib::TCh*     pszCompVal;
            const TCQCFldValue*     pfvToTest;

            tCIDLib::TBoolean       bExpResult;
        };

        static const tCIDLib::TCh* const pszName = L"A Test";
        static const TTest aTests[] =
        {
            // Test true/false status
            { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvBoolTrue, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvBoolTrue, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvBoolFalse, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvBoolFalse, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvTrueString, kCIDLib::True}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvTrueString, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvFalseString, kCIDLib::True}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvFalseString, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvFloatPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvIntNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvZeroCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvZeroFloat, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvZeroInt, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsFalse, L"", &fvZeroTime, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsTrue, L"", &fvNowTime, kCIDLib::True }

            // Test pos/negative values. Zeros should all be positive
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsPositive, L"", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsPositive, L"", &fvFloatPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNegative, L"", &fvIntNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNegative, L"", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsPositive, L"", &fvZeroCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsPositive, L"", &fvZeroFloat, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsPositive, L"", &fvZeroInt, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNegative, L"", &fvZeroCard, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNegative, L"", &fvZeroFloat, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNegative, L"", &fvZeroInt, kCIDLib::False}

            // Do non-null tests and negative non-null tests
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvFloatPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvIntNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvNowTime, kCIDLib::True }

          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvZeroCard, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvZeroFloat, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvZeroInt, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvZeroString, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsNonNull, L"", &fvZeroTime, kCIDLib::False }

            //
            //  Do equality and relative magnitude tests for each type that
            //  supports them. Note that string < > are bit iffy because of
            //  potential platform sort order differences!
            //
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"16", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"1", &fvCard, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"16", &fvCard, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"1", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"6", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"25", &fvCard, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"17", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"10", &fvCard, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"16", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"17", &fvCard, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"16", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"15", &fvCard, kCIDLib::False }

          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"94", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"93", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"95", &fvIntPos, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"94", &fvIntPos, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"89", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"100", &fvIntPos, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"1002", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"1", &fvIntPos, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"94", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"95", &fvIntPos, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"94", &fvIntPos, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"93", &fvIntPos, kCIDLib::False }

          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"-0.301", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"1", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"4", &fvFloatNeg, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"-0.301", &fvFloatNeg, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"-6", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"0", &fvFloatNeg, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"2", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"-9", &fvFloatNeg, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"-0.301", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThanEq, L"-0.300", &fvFloatNeg, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"-0.301", &fvFloatNeg, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThanEq, L"-0.302", &fvFloatNeg, kCIDLib::False }

          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"Test String", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"Test", &fvString, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"Test String", &fvString, kCIDLib::False }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::NotEqual, L"Test", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsGThan, L"Test Strinf", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsLsThan, L"Test Strinh", &fvString, kCIDLib::True }

          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsMultipleOf, L"4", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsMultipleOf, L"5", &fvCard, kCIDLib::False }

            // Test the is even/is odd ones
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEven, L"", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsOdd, L"", &fvCard, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEven, L"", &fvCardOdd, kCIDLib::False}
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsOdd, L"", &fvCardOdd, kCIDLib::True}

            // Test the IsAlpha and IsAlphaNum ones
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsAlpha, L"", &fvFalseString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsAlphaNum, L"", &fvFalseString, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsAlphaNum, L"", &fvNumString, kCIDLib::True }

            //
            //  And some regular expression ones. Regular expressions themselves
            //  are tested somewhere else, so just a few to insure that they are
            //  getting invoked correctly.
            //
          , { tCQCKit::EExprTypes::RegEx, tCQCKit::EStatements::None, L"T.*", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::RegEx, tCQCKit::EStatements::None, L"T.", &fvString, kCIDLib::False }
          , { tCQCKit::EExprTypes::RegEx, tCQCKit::EStatements::None, L".*Str.*", &fvString, kCIDLib::True }
          , { tCQCKit::EExprTypes::RegEx, tCQCKit::EStatements::None, L"True|False", &fvTrueString, kCIDLib::True }
          , { tCQCKit::EExprTypes::RegEx, tCQCKit::EStatements::None, L"True|False", &fvFalseString, kCIDLib::True }


            // These reference globla variables
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"%(GVar:Test1)", &fvCard, kCIDLib::True }
          , { tCQCKit::EExprTypes::Statement, tCQCKit::EStatements::IsEqual, L"%(GVar:Test2)", &fvString, kCIDLib::False }
        };
        const tCIDLib::TCard4 c4TestCnt = tCIDLib::c4ArrayElems(aTests);

        //
        //  Now run through them all. For each one, we do it non-negated and
        //  test the expected result, then we do it negated and make sure
        //  it returns the opposite of the expected result.
        //
        TString strVal;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TestCnt; c4Index++)
        {
            const TTest& CurTest = aTests[c4Index];
            {
                // Set up this test
                TCQCExpression exprPos
                (
                    L"Test Name"
                    , CurTest.eExprType
                    , CurTest.eStatement
                    , CurTest.pszCompVal
                    , kCIDLib::False
                );

                TCQCExpression exprNeg
                (
                    L"Test Name"
                    , CurTest.eExprType
                    , CurTest.eStatement
                    , CurTest.pszCompVal
                    , kCIDLib::True
                );

                if ((exprPos.bEvaluate(*CurTest.pfvToTest, &ctarVars) != CurTest.bExpResult)
                ||  (exprNeg.bEvaluate(*CurTest.pfvToTest, &ctarVars) == CurTest.bExpResult))
                {
                    CurTest.pfvToTest->Format(strVal);
                    strmOut << L"Evalution test failed. Expr=";
                    if (CurTest.eExprType == tCQCKit::EExprTypes::Statement)
                    {
                        if (*CurTest.pszCompVal)
                        {
                            strmOut << L"'"
                                    << CurTest.pszCompVal
                                    << L"' "
                                    << tCQCKit::strXlatEStatements(CurTest.eStatement)
                                    << L" '"
                                    << strVal << L"'";
                        }
                         else
                        {
                            strmOut << L"'"
                                    << strVal
                                    << L"' "
                                    << tCQCKit::strXlatEStatements(CurTest.eStatement);
                        }
                    }
                     else
                    {
                        strmOut << L"'" << strVal << L"' matches RegEx '"
                                << CurTest.pszCompVal << L"'";
                    }
                    strmOut << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }
        }
    }

    return eRes;
}


