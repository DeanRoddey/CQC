//
// FILE NAME: TestCQCKit_ActVars.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/31/2008
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
//  This file implements the tests for the action variables standard target.
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
RTTIDecls(TTest_BasicActVar1,TTestFWTest)
RTTIDecls(TTest_BasicActVar2,TTestFWTest)
RTTIDecls(TTest_AdvActVar1,TTestFWTest)
RTTIDecls(TTest_TimeStampVar,TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicActVar
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_BasicActVar1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_BasicActVar1::TTest_BasicActVar1() :

    TTestFWTest
    (
        L"Action variable class 1", L"Untyped tests of the action variable class", 2
    )
{
}

TTest_BasicActVar1::~TTest_BasicActVar1()
{
}


// ---------------------------------------------------------------------------
//  TTest_BasicActVar1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_BasicActVar1::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tCIDLib::TCard4         c4NextVarId = 1;
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;

    //
    //  Create a variable of the sort that would be created in an action by
    //  setting a value on a variable, i.e. the implicit type of creation.
    //
    TCQCActVar varTest1
    (
        L"GVar:Test1"
        , tCQCKit::EFldTypes::String
        , TString::strEmpty()
        , TString::strEmpty()
        , c4NextVarId++
    );

    // Make sure that the values got set correctly
    if (!bTestVar(strmOut, varTest1, L"GVar:Test1", L"", L"", tCQCKit::EFldTypes::String, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    // Set a value and make sure it gets set correctly and shows up as a change
    if (!varTest1.bSetValue(L"Testing"))
    {
        strmOut << TFWCurLn << L"Value should have been different\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Make sure we get that back
    if (varTest1.strValue() != L"Testing")
    {
        strmOut << TFWCurLn << L"Did not get set value back\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Set it again and it should not show up as a change
    if (varTest1.bSetValue(L"Testing"))
    {
        strmOut << TFWCurLn << L"Value should not have been different\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Set a numeric value on it and do some numeric operations
    if (!varTest1.bSetValue(L"123"))
    {
        strmOut << TFWCurLn << L"Value should have been different\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bAdd(L"10"))
    {
        strmOut << TFWCurLn << L"Add should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"133")
    {
        strmOut << TFWCurLn << L"Add operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bDivide(L"10"))
    {
        strmOut << TFWCurLn << L"Divide should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"13")
    {
        strmOut << TFWCurLn << L"Divide operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bSubtract(L"10"))
    {
        strmOut << TFWCurLn << L"Subtract should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"3")
    {
        strmOut << TFWCurLn << L"Subtract operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bMultiply(L"20"))
    {
        strmOut << TFWCurLn << L"Multiply should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"60")
    {
        strmOut << TFWCurLn << L"Multiply operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // This shouldn't change the value
    if (varTest1.bAdd(L"0"))
    {
        strmOut << TFWCurLn << L"Add of 0 changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.bDivide(L"1"))
    {
        strmOut << TFWCurLn << L"Divide by 1 changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.bSubtract(L"0"))
    {
        strmOut << TFWCurLn << L"Subtract by 0 changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.bMultiply(L"1"))
    {
        strmOut << TFWCurLn << L"Multiply by 1 changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bMultiply(L"-1"))
    {
        strmOut << TFWCurLn << L"Multiply by -1 should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    // Do some boolean ops
    if (!varTest1.bSetValue(L"0xFF"))
    {
        strmOut << TFWCurLn << L"Value should have been different\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bBooleanOp(L"0x80", tCQCKit::ELogOps::AND))
    {
        strmOut << TFWCurLn << L"AND should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"128")
    {
        strmOut << TFWCurLn << L"AND operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bBooleanOp(L"1", tCQCKit::ELogOps::OR))
    {
        strmOut << TFWCurLn << L"AND should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"129")
    {
        strmOut << TFWCurLn << L"OR operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    // Do some text oriented operations
    if (!varTest1.bSetValue(L"CDE"))
    {
        strmOut << TFWCurLn << L"Set should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bInsertValue(L"AB", 0))
    {
        strmOut << TFWCurLn << L"Insert should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"ABCDE")
    {
        strmOut << TFWCurLn << L"Insert operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.bInsertValue(L"", 0))
    {
        strmOut << TFWCurLn << L"Insert of empty string changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bDelSubStr(2, 2))
    {
        strmOut << TFWCurLn << L"Substr delete should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"ABE")
    {
        strmOut << TFWCurLn << L"Substr delete operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    if (!varTest1.bSetValue(L"This is a test"))
    {
        strmOut << TFWCurLn << L"Set should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bCapAt(7))
    {
        strmOut << TFWCurLn << L"Cap should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"This is")
    {
        strmOut << TFWCurLn << L"Cap operation created wrong result\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.bCapAt(7))
    {
        strmOut << TFWCurLn << L"Cap beyond end changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bAppendValue(L" a test"))
    {
        strmOut << TFWCurLn << L"Append should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"This is a test")
    {
        strmOut << TFWCurLn << L"Append created the wrong value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    if (!varTest1.bUpLow(kCIDLib::True))
    {
        strmOut << TFWCurLn << L"Upper case should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"THIS IS A TEST")
    {
        strmOut << TFWCurLn << L"Upper case created the wrong value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varTest1.bUpLow(kCIDLib::False))
    {
        strmOut << TFWCurLn << L"Lower case should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varTest1.strValue() != L"this is a test")
    {
        strmOut << TFWCurLn << L"Lower case created the wrong value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Make sure that the attributes didn't change
    if (!bTestVar(strmOut, varTest1, L"GVar:Test1", L"this is a test", L"", tCQCKit::EFldTypes::String, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_BasicActVar1: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to validate the values of a variable
tCIDLib::TBoolean
TTest_BasicActVar1::bTestVar(       TTextStringOutStream&   strmOut
                            , const TCQCActVar&             varToTest
                            , const tCIDLib::TCh* const     pszExpKey
                            , const tCIDLib::TCh* const     pszExpValue
                            , const tCIDLib::TCh* const     pszExpLimits
                            , const tCQCKit::EFldTypes      eExpType
                            , const tCIDLib::TCard4         c4Line)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (varToTest.strName() != pszExpKey)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected key of '"
                << pszExpKey << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.strValue() != pszExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected value of '"
                << pszExpValue << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.strLimits() != pszExpLimits)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected limit of '"
                << pszExpLimits << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.eType() != eExpType)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected type of '"
                << tCQCKit::strXlatEFldTypes(eExpType) << L"'\n\n";
        bRes = kCIDLib::False;
    }

    return bRes;
}







// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicActVar2
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_BasicActVar2: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_BasicActVar2::TTest_BasicActVar2() :

    TTestFWTest
    (
        L"Action variable class 2", L"Typed tests of the action variable class", 2
    )
{
}

TTest_BasicActVar2::~TTest_BasicActVar2()
{
}


// ---------------------------------------------------------------------------
//  TTest_BasicActVar1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_BasicActVar2::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tCIDLib::TCard4         c4NextVarId = 1;
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;

    //
    //  Create some variables of different data types and ranges and validate
    //  them. Where we don't give an initial value, it should be defaulted to
    //  something with the implicit or explicit limits.
    //
    TCQCActVar varCard
    (
        L"GVar:Card1"
        , tCQCKit::EFldTypes::Card
        , TString::strEmpty()
        , TString::strEmpty()
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varCard, L"GVar:Card1", L"0", L"", tCQCKit::EFldTypes::Card, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    TCQCActVar varCardR
    (
        L"GVar:CardR"
        , tCQCKit::EFldTypes::Card
        , L"Range:0, 100"
        , L"2"
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varCardR, L"GVar:CardR", L"2", L"Range:0, 100", tCQCKit::EFldTypes::Card, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    TCQCActVar varFloat
    (
        L"GVar:Float1"
        , tCQCKit::EFldTypes::Float
        , TString::strEmpty()
        , TString::strEmpty()
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varFloat, L"GVar:Float1", L"0.00", L"", tCQCKit::EFldTypes::Float, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    TCQCActVar varFloatR
    (
        L"GVar:FloatR"
        , tCQCKit::EFldTypes::Float
        , L"Range:-1.0, 1.0"
        , L""
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varFloatR, L"GVar:FloatR", L"0.00", L"Range:-1.0, 1.0", tCQCKit::EFldTypes::Float, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    TCQCActVar varStr
    (
        L"GVar:Str"
        , tCQCKit::EFldTypes::String
        , L""
        , L"Test1"
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varStr, L"GVar:Str", L"Test1", L"", tCQCKit::EFldTypes::String, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;

    TCQCActVar varStrEnum
    (
        L"GVar:StrEnum"
        , tCQCKit::EFldTypes::String
        , L"Enum:Val1, Val2, Val3"
        , L""
        , c4NextVarId++
    );
    if (!bTestVar(strmOut, varStrEnum, L"GVar:StrEnum", L"Val1", L"Enum:Val1, Val2, Val3", tCQCKit::EFldTypes::String, CID_LINE))
        eRes = tTestFWLib::ETestRes::Failed;


    // Test some of the formatting stuff
    if (!varCard.bSetValue(1421UL))
    {
        strmOut << TFWCurLn << L"Set should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    varCard.SetFormat(tCIDLib::ERadices::Hex, 2);
    if (varCard.strValue() != L"58D")
    {
        strmOut << TFWCurLn << L"Formatted value was incorrect\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!varFloat.bSetValue(-193.923))
    {
        strmOut << TFWCurLn << L"Set should have changed the value \n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // It'll ignore the hex radix for a float
    varFloat.SetFormat(tCIDLib::ERadices::Hex, 2);
    if (varFloat.strValue() != L"-193.92")
    {
        strmOut << TFWCurLn << L"Formatted value was incorrect\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    varFloat.SetFormat(tCIDLib::ERadices::Hex, 1);
    if (varFloat.strValue() != L"-193.9")
    {
        strmOut << TFWCurLn << L"Formatted value was incorrect\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    // Try some set from type stuff, to make sure conversions work right
    TCQCActVar varInt
    (
        L"GVar:Int1"
        , tCQCKit::EFldTypes::Int
        , TString::strEmpty()
        , TString::strEmpty()
        , c4NextVarId++
    );

    //
    //  This shouldn't change the value since it should come out zero
    //  after the conversion, and the initial value of the int should be zero.
    //
    varFloat.bSetValue(0.19);
    if (varInt.bSetValueFrom(varFloat))
    {
        strmOut << TFWCurLn << L"Set from indicated false value change\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varInt.strValue() != L"0")
    {
        strmOut << TFWCurLn << L"Set from (float->int) failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    varFloat.bSetValue(2.9);
    if (!varInt.bSetValueFrom(varFloat))
    {
        strmOut << TFWCurLn << L"Set from did not indicate value change\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (varInt.strValue() != L"2")
    {
        strmOut << TFWCurLn << L"Set from (float->int) failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_BasicActVar1: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to validate the values of a variable
tCIDLib::TBoolean
TTest_BasicActVar2::bTestVar(       TTextStringOutStream&   strmOut
                            , const TCQCActVar&             varToTest
                            , const tCIDLib::TCh* const     pszExpKey
                            , const tCIDLib::TCh* const     pszExpValue
                            , const tCIDLib::TCh* const     pszExpLimits
                            , const tCQCKit::EFldTypes      eExpType
                            , const tCIDLib::TCard4         c4Line)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (varToTest.strName() != pszExpKey)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected key of '"
                << pszExpKey << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.strValue() != pszExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected value of '"
                << pszExpValue << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.strLimits() != pszExpLimits)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected limit of '"
                << pszExpLimits << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.eType() != eExpType)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected type of '"
                << tCQCKit::strXlatEFldTypes(eExpType) << L"'\n\n";
        bRes = kCIDLib::False;
    }

    return bRes;
}





// ---------------------------------------------------------------------------
//  CLASS: TTest_AdvActVar1
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_AdvActVar1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_AdvActVar1::TTest_AdvActVar1() :

    TTestFWTest
    (
        L"Action Variables Target 1", L"Core tests of the variables target", 3
    )
{
}

TTest_AdvActVar1::~TTest_AdvActVar1()
{
}


// ---------------------------------------------------------------------------
//  TTest_AdvActVar1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_AdvActVar1::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tCIDLib::TBoolean       bGotIt;
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;
    TString                 strTmp;

    // We'll need a global and local variables target
    TStdVarsTar ctarLocals(tCIDLib::EMTStates::Unsafe, kCIDLib::True);
    TStdVarsTar ctarGlobals(tCIDLib::EMTStates::Safe, kCIDLib::False);

    //
    //  We have to create an action engine, though it won't be used here. It's
    //  required for initializing command targets. And the action engine requires
    //  a user context, which also won't end up getting used here. So we
    //  create a bogus one.
    //
    TCQCUserCtx cuctxDummy
    (
        L"BubbaJones", tCQCKit::EUserRoles::NormalUser, L"\\User\\MyDefTemplate"
    );
    TCQCStdActEngine acteDummy(cuctxDummy);

    //
    //  Test for variable existence before we add any. These should all fail.
    //  These are all static methods.
    //
    if (TStdVarsTar::bVarExists(L"GVar:Test", ctarLocals, ctarGlobals))
    {
        strmOut << TFWCurLn << L"Variable should not have existed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (TStdVarsTar::bVarExists(L"LVar:Test", ctarLocals, ctarGlobals))
    {
        strmOut << TFWCurLn << L"Variable should not have existed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // This one should throw because of an incorrect prefix
    bGotIt = kCIDLib::False;
    try
    {
        TStdVarsTar::bVarExists(L"Test", ctarLocals, ctarGlobals);
    }

    catch(const TError&)
    {
        bGotIt = kCIDLib::True;
    }

    if (!bGotIt)
    {
        strmOut << TFWCurLn << L"Failed to catch invalid variable prefix\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // And this one always throws if not found
    bGotIt = kCIDLib::False;
    try
    {
        TStdVarsTar::varFind
        (
            L"GVar:Test", ctarLocals, ctarGlobals, CID_FILE, CID_LINE
        );
    }

    catch(const TError&)
    {
        bGotIt = kCIDLib::True;
    }

    if (!bGotIt)
    {
        strmOut << TFWCurLn << L"Should have gotten not found exception\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // This one can optionally create it, but first try tell it to throw
    bGotIt = kCIDLib::False;
    try
    {
        TStdVarsTar::varFind
        (
            L"GVar:Test", ctarLocals, ctarGlobals, CID_FILE, CID_LINE, kCIDLib::False
        );
    }

    catch(const TError&)
    {
        bGotIt = kCIDLib::True;
    }

    if (!bGotIt)
    {
        strmOut << TFWCurLn << L"Should have gotten not found exception\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Do it again and let it create the variable
    {
        TCQCActVar& varTest = TStdVarsTar::varFind
        (
            L"GVar:Test"
            , ctarLocals
            , ctarGlobals
            , CID_FILE
            , CID_LINE
            , kCIDLib::True
        );

        // Make sure that the key and value are correct
        if (!bTestVar(strmOut, varTest, L"GVar:Test", L"", CID_LINE))
            eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test the variable existence method
    if (!TStdVarsTar::bVarExists(L"GVar:Test", ctarLocals, ctarGlobals))
    {
        strmOut << TFWCurLn << L"Variable GVar:Test should have been found\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  A reset of the global variables should not remove anything, because they
    //  maintain their variables across uses.
    //
    ctarGlobals.CmdTarInitialize(acteDummy);
    if (!TStdVarsTar::bVarExists(L"GVar:Test", ctarLocals, ctarGlobals))
    {
        strmOut << TFWCurLn << L"Reset removed global variables\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Add some variables and verify that we get the right values back
    ctarGlobals.bAddOrUpdateVar(L"GVar:Var1", L"Value of Var1", tCQCKit::EFldTypes::String, TString::strEmpty());
    ctarGlobals.bAddOrUpdateVar(L"GVar:Var2", L"Value of Var2", tCQCKit::EFldTypes::String, TString::strEmpty());
    ctarLocals.bAddOrUpdateVar(L"LVar:Var3", L"Value of Var3", tCQCKit::EFldTypes::String, TString::strEmpty());

    if ((ctarGlobals.strValue(L"GVar:Var1") != L"Value of Var1")
    ||  (ctarGlobals.strValue(L"GVar:Var2") != L"Value of Var2")
    ||  (ctarLocals.strValue(L"LVar:Var3") != L"Value of Var3"))
    {
        strmOut << TFWCurLn << L"Got incorrect values for added variables\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!ctarGlobals.bVarValue(L"GVar:Var2", strTmp) || (strTmp != L"Value of Var2"))
    {
        strmOut << TFWCurLn << L"Got incorrect value Var2\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (!ctarLocals.bVarValue(L"LVar:Var3", strTmp) || (strTmp != L"Value of Var3"))
    {
        strmOut << TFWCurLn << L"Got incorrect value Var3\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Try to add one with the wrong prefix to each
    bGotIt = kCIDLib::False;
    try
    {
        ctarGlobals.bAddOrUpdateVar(L"LVar:BadVar", L"Value of BadVar", tCQCKit::EFldTypes::String, TString::strEmpty());
    }

    catch(const TError&)
    {
        bGotIt = kCIDLib::True;
    }

    if (!bGotIt)
    {
        strmOut << TFWCurLn << L"Failed to catch use of wrong prefix\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Try to add one with the wrong prefix to each
    bGotIt = kCIDLib::False;
    try
    {
        ctarLocals.bAddOrUpdateVar(L"GVar:BadVar", L"Value of BadVar", tCQCKit::EFldTypes::String, TString::strEmpty());
    }

    catch(const TError&)
    {
        bGotIt = kCIDLib::True;
    }

    if (!bGotIt)
    {
        strmOut << TFWCurLn << L"Failed to catch use of wrong prefix\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // A reset of the local target should remove all variables
    ctarLocals.CmdTarInitialize(acteDummy);
    if (TStdVarsTar::bVarExists(L"GVar:Var3", ctarLocals, ctarGlobals))
    {
        strmOut << TFWCurLn << L"Reset did not clear local vars\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // And test the cursors. The locals shoudl be empty now
    {
        TStdVarsTar::TCursor cursVars = ctarLocals.cursVars();
        if (cursVars.bReset())
        {
            strmOut << TFWCurLn << L"Locals cursor should be empty\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TStdVarsTar::TCursor cursVars = ctarGlobals.cursVars();
        if (!cursVars.bReset())
        {
            strmOut << TFWCurLn << L"Globals cursor was empty\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // We should get three variables in the glocals
        tCIDLib::TCard4 c4Count = 0;
        do
        {
            c4Count++;
        }   while (cursVars.bNext());

        if (c4Count != 3)
        {
            strmOut << TFWCurLn << L"Globals cursor had wrong element count\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }


    //
    //  Test the serial number stuff. We'll get the value and serial number,
    //  then modify the variable and get it again. The serial number should
    //  change. Then we get it again and should be told no change occured.
    //
    //  We cheat here and change the value directly, else we'd have to create
    //  an action and run it, which is very tedious.
    //
    tCQCKit::EValQRes eQRes;
    tCIDLib::TCard4 c4SerialNum = 0;
    eQRes = ctarGlobals.eVarValue(L"GVar:Var1", strTmp, c4SerialNum);
    if (eQRes != tCQCKit::EValQRes::NewValue)
    {
        strmOut << TFWCurLn << L"Should have gotten new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }
    ctarGlobals.bAddOrUpdateVar(L"GVar:Var1", L"A New Value", tCQCKit::EFldTypes::String, TString::strEmpty());

    eQRes = ctarGlobals.eVarValue(L"GVar:Var1", strTmp, c4SerialNum);
    if (eQRes != tCQCKit::EValQRes::NewValue)
    {
        strmOut << TFWCurLn << L"Should have gotten new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (strTmp != L"A New Value")
    {
        strmOut << TFWCurLn << L"Did not get new variable value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Now we shouldn't get a new value
    eQRes = ctarGlobals.eVarValue(L"GVar:Var1", strTmp, c4SerialNum);
    if (eQRes != tCQCKit::EValQRes::NoChange)
    {
        strmOut << TFWCurLn << L"Should not have gotten new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  Test the read only capabilities. We create a local variable and indicate
    //  it should be read only. Then we try to write to it, which shoudl cause
    //  an exception.
    //
    {
        ctarLocals.DeleteAllVars();
        TCQCActVar& varReadOnly = TStdVarsTar::varFind
        (
            L"LVar:Var1", ctarLocals, ctarGlobals, CID_FILE, CID_LINE, kCIDLib::True
        );
        varReadOnly.bSetValue(L"Initial Value");
        varReadOnly.bReadOnly(kCIDLib::True);

        bGotIt = kCIDLib::False;
        try
        {
            varReadOnly.bSetValue(L"A new value");
        }

        catch(const TError&)
        {
            bGotIt = kCIDLib::True;
        }

        if (!bGotIt)
        {
            strmOut << TFWCurLn << L"Failed to catch write to read only variable\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Do the same with the special method that adds action parameters.
    //  It should create a read only variable.
    //
    {
        ctarLocals.AddActParmVar(L"LVar:CQCActParm_1", L"Act parm 1");
        TCQCActVar& varActParm = TStdVarsTar::varFind
        (
            L"LVar:CQCActParm_1"
            , ctarLocals
            , ctarGlobals
            , CID_FILE
            , CID_LINE
            , kCIDLib::False
        );

        if (varActParm.strValue() != L"Act parm 1")
        {
            strmOut << TFWCurLn << L"Action parm var value not set correctly\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        bGotIt = kCIDLib::False;
        try
        {
            varActParm.bSetValue(L"A new value");
        }

        catch(const TError&)
        {
            bGotIt = kCIDLib::True;
        }

        if (!bGotIt)
        {
            strmOut << TFWCurLn << L"Failed to catch write to action parm varn\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_AdvActVar1: Private, non-virtual methods
// ---------------------------------------------------------------------------

// A helper to validate the keya nd value of a kv pair
tCIDLib::TBoolean
TTest_AdvActVar1::bTestVar(        TTextStringOutStream&   strmOut
                            , const TCQCActVar&             varToTest
                            , const tCIDLib::TCh* const     pszExpKey
                            , const tCIDLib::TCh* const     pszExpValue
                            , const tCIDLib::TCard4         c4Line)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (varToTest.strName() != pszExpKey)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected key of '"
                << pszExpKey << L"'\n\n";
        bRes = kCIDLib::False;
    }

    if (varToTest.strValue() != pszExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Expected value of '"
                << pszExpValue << L"'\n\n";
        bRes = kCIDLib::False;
    }
    return bRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_TimeStampVar
// PREFIX: tfwt
//
//  Time stamps in action variables are in sort of weird. They are formatted out as
//  hex values when accessed as text, which is the case when they are used as
//  parameters.
//
//  This is historical weirdness because originally they were assumed to come from
//  time based fields which store them that way. And they won't have hex prefixes
//  in that case either. They may have hex prefixes if they were set some other
//  way.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_TimeStampVar: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_TimeStampVar::TTest_TimeStampVar() :

    TTestFWTest
    (
        L"Time Action Variables", L"Special tests for time stamps in variables", 2
    )
{
}

TTest_TimeStampVar::~TTest_TimeStampVar()
{
}


// ---------------------------------------------------------------------------
//  TTest_TimeStampVar: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_TimeStampVar::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tCIDLib::TCard4         c4NextVarId = 1;
    tTestFWLib::ETestRes    eRes = tTestFWLib::ETestRes::Success;

    //
    //  Time based vars. Note that the default formatting of the value will be as a hex
    //  value with prefix in this case, so the default value should be 0x0.
    //
    TCQCActVar varTime
    (
        L"GVar:Time"
        , tCQCKit::EFldTypes::Time
        , L""
        , L""
        , c4NextVarId++
    );
    if (varTime.strValue() != L"0x0")
    {
        strmOut << L"Expected value of '0x0' but got '" << varTime.strValue()
                << L"'";
        eRes = tTestFWLib::ETestRes::Failed;
    }


    // Set the current time on it as a binary stamp
    tCIDLib::TEncodedTime enctTest = TTime::enctNow();
    varTime.bSetTime(enctTest);

    // Format out the expected value, which would the stamp in hex with prefix
    TString strExpVal;
    strExpVal = L"0x";
    strExpVal.AppendFormatted(enctTest, tCIDLib::ERadices::Hex);
    if (varTime.strValue() != strExpVal)
    {
        strmOut << L"Expected value of '" << strExpVal << L" but got '"
                << strExpVal << L"'";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Set it as text without prefix, as it would be from a time based field value
    enctTest = TTime::enctNow();
    strExpVal.SetFormatted(enctTest, tCIDLib::ERadices::Hex);
    varTime.bSetValue(strExpVal);

    // Make sure it was correct interpreted and converted
    const TCQCTimeFldValue& fvTime = static_cast<const TCQCTimeFldValue&>(varTime.fvCurrent());
    if (fvTime.c8Value() != enctTest)
    {
        strmOut << L"Time stamp value set via text did not come back the same";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}
