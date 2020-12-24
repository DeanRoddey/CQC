//
// FILE NAME: TestCQCKit_FldValues.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/12/2008
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
//  This file implements the tests for the field value classes, which are
//  used to hold field values in drivers and elsewhere. These are level 1
//  tests, since these classes are foundation classes for CQC that don't
//  depend on any other CQC level stuff.
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
RTTIDecls(TTest_FldValComm,TTestFWTest)
RTTIDecls(TTest_FldValue,TTest_FldValComm)
RTTIDecls(TTest_BoolFldVal,TTest_FldValComm)
RTTIDecls(TTest_CardFldVal,TTest_FldValComm)
RTTIDecls(TTest_FloatFldVal,TTest_FldValComm)
RTTIDecls(TTest_IntFldVal,TTest_FldValComm)
RTTIDecls(TTest_StrFldVal,TTest_FldValComm)
RTTIDecls(TTest_StrListFldVal,TTest_FldValComm)



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldValComm
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldValComm: Destructor
// ---------------------------------------------------------------------------
TTest_FldValComm::~TTest_FldValComm()
{
}

// ---------------------------------------------------------------------------
//  TTest_FldValComm: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldValComm::TTest_FldValComm(const TString& strName, const TString& strDescr) :

    TTestFWTest(strName, strDescr, 1)
{
}


// ---------------------------------------------------------------------------
//  TTest_FldValComm: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  A helper to test the values of the bass field value class, which all of
//  the derivates needs to do.
//
tCIDLib::TBoolean
TTest_FldValComm::bTestBaseStuff(       TTextOutStream&     strmOut
                                , const TTFWCurLn&          clnAt
                                , const TCQCFldValue&       fvToTest
                                , const tCQCKit::EFldTypes  eType
                                , const tCIDLib::TCard4     c4DriverId
                                , const tCIDLib::TCard4     c4FieldId
                                , const tCIDLib::TCard4     c4SerialNum
                                , const tCIDLib::TBoolean   bInError)
{
    tCIDLib::TBoolean bRes = kCIDLib::True;
    if (fvToTest.bInError() != bInError)
    {
        strmOut << clnAt << L"Error state is incorrect\n\n";
        bRes = kCIDLib::False;
    }

    if (fvToTest.c4DriverId() != c4DriverId)
    {
        strmOut << clnAt << L"Driver id is incorrect\n\n";
        bRes = kCIDLib::False;
    }

    if (fvToTest.c4FieldId() != c4FieldId)
    {
        strmOut << clnAt << L"Field id is incorrect\n\n";
        bRes = kCIDLib::False;
    }

    if (fvToTest.c4SerialNum() != c4SerialNum)
    {
        strmOut << clnAt << L"Serial number is incorrect\n\n";
        bRes = kCIDLib::False;
    }

    if (fvToTest.eFldType() != eType)
    {
        strmOut << clnAt << L"Field type is incorrect\n\n";
        bRes = kCIDLib::False;
    }
    return bRes;
}


//
//  All of the derivatives will test the statement evaluation method. They
//  are all done the same, just the inputs will change, so we provide a method
//  to do them. They ass us an array of statement test structures, defined at
//  the file level above.
//
tCIDLib::TBoolean
TTest_FldValComm::bTestStatements(          TTextOutStream&     strmOut
                                    , const TTFWCurLn&          clnAt
                                    , const TCQCFldValue&       fvToTest
                                    , const TStmtTest* const    pTests
                                    , const tCIDLib::TCard4     c4Count)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TStmtTest& CurTest = pTests[c4Index];

        // Evaluate this test
        const tCQCKit::EStmtRes eTestRes = fvToTest.eTestStatement
        (
            CurTest.eType, CurTest.pszTestVal
        );

        // And see if it matches the expected outcome
        if (eTestRes != CurTest.eExpRes)
        {
            strmOut << clnAt << L"Statement test at index '" << c4Index
                    << L"' did not have the expected result.\n\n";
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_FldValue
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldValue: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldValue::TTest_FldValue() :

    TTest_FldValComm
    (
        L"Base Fld value", L"Tests the base field value class"
    )
{
}

TTest_FldValue::~TTest_FldValue()
{
}


// ---------------------------------------------------------------------------
//  TTest_FldValue: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldValue::eRunTest(TTextStringOutStream&  strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  It doesn't matter which derivatives we use to test here, since they
    //  would all give the same result.
    //
    //  First check the initial values on a default ctor.
    //
    tCIDLib::TBoolean bRes;
    {
        TCQCBoolFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Boolean, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // And now do the ctor that takes initial values
    {
        TCQCCardFldValue fvTest2(39, 348);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::Card, 39, 348, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the set/get methods
    {
        TCQCIntFldValue fvTest(10, 19);
        fvTest.c4IncSerialNum();
        if (fvTest.c4SerialNum() != 2)
        {
            strmOut << TFWCurLn << L"Increment of serial number failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest.c4IncSerialNum();
        if (fvTest.c4SerialNum() != 3)
        {
            strmOut << TFWCurLn << L"Increment of serial number failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest.bInError(kCIDLib::True);
        if (!fvTest.bInError())
        {
            strmOut << TFWCurLn << L"Set of error state failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Set the ids and check that they were set
        fvTest.SetIds(10, 390);
        if ((fvTest.c4DriverId() != 10) || (fvTest.c4FieldId() != 390))
        {
            strmOut << TFWCurLn << L"Set of ids failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCFloatFldValue fvTestOrg(20, 29);
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCFloatFldValue fvTestCopy(fvTestOrg);
        TCQCFloatFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_BoolFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_BoolVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_BoolFldVal::TTest_BoolFldVal() :

    TTest_FldValComm
    (
        L"Bool Fld Value", L"Tests the boolean field value class"
    )
{
}

TTest_BoolFldVal::~TTest_BoolFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_BoolVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_BoolFldVal::eRunTest( TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCBoolFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Boolean, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest1.bValue())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCBoolFldValue fvTest1(43, 12);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Boolean, 43, 12, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest1.bValue())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCBoolFldValue fvTest3;
        fvTest3.bSetValue(kCIDLib::True);
        if (!fvTest3.bValue())
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(kCIDLib::True))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(kCIDLib::False);
        if (fvTest3.bValue())
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(kCIDLib::False))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system. Only a few of the statement types
    //  really apply to boolean types.
    //
    {
        TCQCBoolFldValue fvComp;

        // Run some against a true value
        fvComp.bSetValue(kCIDLib::True);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"True", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"False", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Now flip to false and run the opposite results
        fvComp.bSetValue(kCIDLib::False);
        {
            const TStmtTest aTestsFalse[] =
            {
                { L"True", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"False", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsFalse, tCIDLib::c4ArrayElems(aTestsFalse)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the value formatting
    {
        TCQCBoolFldValue    fvFmt;
        TString             strFmt;

        fvFmt.bSetValue(kCIDLib::True);
        fvFmt.Format(strFmt);
        if (strFmt != kCQCKit::pszFld_True)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvFmt.bSetValue(kCIDLib::False);
        fvFmt.Format(strFmt);
        if (strFmt != kCQCKit::pszFld_False)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCBoolFldValue fvTestOrg(23, 49);
        fvTestOrg.bSetValue(kCIDLib::True);
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCBoolFldValue fvTestCopy(fvTestOrg);
        TCQCBoolFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  (fvTestOrg.bValue() != fvTestCopy.bValue()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  (fvTestOrg.bValue() != fvTestAssign.bValue()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_CardFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_CardVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_CardFldVal::TTest_CardFldVal() :

    TTest_FldValComm
    (
        L"Card Fld Value", L"Tests the Card field value class"
    )
{
}

TTest_CardFldVal::~TTest_CardFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_CardVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_CardFldVal::eRunTest( TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCCardFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Card, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest1.c4Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCCardFldValue fvTest2(4, 2);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::Card, 4, 2, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest2.c4Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCCardFldValue fvTest3;
        fvTest3.bSetValue(12);
        if (fvTest3.c4Value() != 12)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(1442);
        if (fvTest3.c4Value() != 1442)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(1442))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.bSetValue(0))
        {
            strmOut << TFWCurLn << L"Value change not correctly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.c4Value() != 0)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system.
    //
    {
        TCQCCardFldValue fvComp;

        // Set a value to run some statements against
        fvComp.bSetValue(3812);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"3812", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"13", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"12", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
              , { L"5000", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"12", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::False }
              , { L"5000", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Set a zero value to test some special cases
        fvComp.bSetValue(0);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"0", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::True }
              , { L"5", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCCardFldValue fvTestOrg(23, 49);
        fvTestOrg.bSetValue(112);
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCCardFldValue fvTestCopy(fvTestOrg);
        TCQCCardFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  (fvTestOrg.c4Value() != fvTestCopy.c4Value()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  (fvTestOrg.c4Value() != fvTestAssign.c4Value()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_FloatFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FloatVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FloatFldVal::TTest_FloatFldVal() :

    TTest_FldValComm
    (
        L"Float Fld Value", L"Tests the float field value class"
    )
{
}

TTest_FloatFldVal::~TTest_FloatFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_FloatVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FloatFldVal::eRunTest(TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCFloatFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Float, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest1.f8Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCFloatFldValue fvTest2(4, 2);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::Float, 4, 2, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest2.f8Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCFloatFldValue fvTest3;
        fvTest3.bSetValue(143.3);
        if (fvTest3.f8Value() != 143.3)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(-1.2);
        if (fvTest3.f8Value() != -1.2)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(-1.2))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.bSetValue(944.589))
        {
            strmOut << TFWCurLn << L"Value change not correctly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.f8Value() != 944.589)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system.
    //
    {
        TCQCFloatFldValue fvComp;

        // Set a value to run some statements against
        fvComp.bSetValue(-1.33);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"-1.33", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"1.33", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"-3.4", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
              , { L"0.33", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"-5", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::False }
              , { L"0", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Set a positive value and do some more
        fvComp.bSetValue(85.33);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"85.33", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"500", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"-5", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
              , { L"5", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::False }
              , { L"5", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCFloatFldValue fvTestOrg(23, 49);
        fvTestOrg.bSetValue(0.441);
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCFloatFldValue fvTestCopy(fvTestOrg);
        TCQCFloatFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  (fvTestOrg.f8Value() != fvTestCopy.f8Value()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  (fvTestOrg.f8Value() != fvTestAssign.f8Value()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_IntFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_IntVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_IntFldVal::TTest_IntFldVal() :

    TTest_FldValComm
    (
        L"Int Fld Value", L"Tests the integer field value class"
    )
{
}

TTest_IntFldVal::~TTest_IntFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_IntVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_IntFldVal::eRunTest(  TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCIntFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::Int, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest1.i4Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCIntFldValue fvTest2(5, 4);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::Int, 5, 4, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest2.i4Value())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCIntFldValue fvTest3;
        fvTest3.bSetValue(-12);
        if (fvTest3.i4Value() != -12)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(142);
        if (fvTest3.i4Value() != 142)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(142))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.bSetValue(0))
        {
            strmOut << TFWCurLn << L"Value change not correctly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.i4Value() != 0)
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system.
    //
    {
        TCQCIntFldValue fvComp;

        // Set a value to run some statements against
        fvComp.bSetValue(-192);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"-192", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"13", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"-193", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
              , { L"-11", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"-193", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::False }
              , { L"0", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Set a positive value and do some others
        fvComp.bSetValue(388);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"388", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsPositive, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNegative, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"5000", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"5000", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCIntFldValue fvTestOrg(23, 49);
        fvTestOrg.bSetValue(112);
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCIntFldValue fvTestCopy(fvTestOrg);
        TCQCIntFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  (fvTestOrg.i4Value() != fvTestCopy.i4Value()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  (fvTestOrg.i4Value() != fvTestAssign.i4Value()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_StrFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_IntVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_StrFldVal::TTest_StrFldVal() :

    TTest_FldValComm
    (
        L"String Fld Value", L"Tests the string field value class"
    )
{
}

TTest_StrFldVal::~TTest_StrFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_IntVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_StrFldVal::eRunTest(  TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCStringFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::String, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest1.strValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCStringFldValue fvTest2(51, 41);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::String, 51, 41, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest2.strValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCStringFldValue fvTest3;
        fvTest3.bSetValue(L"ATest");
        if (fvTest3.strValue() != L"ATest")
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(L"Another Test");
        if (fvTest3.strValue() != L"Another Test")
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(L"Another Test"))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.bSetValue(TString::strEmpty()))
        {
            strmOut << TFWCurLn << L"Value change not correctly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.strValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system.
    //
    {
        TCQCStringFldValue fvComp;

        // Set a value to run some statements against
        fvComp.bSetValue(L"1234");
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"1234", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"123", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
              , { L"0123", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::True }
              , { L"2345", tCQCKit::EStatements::IsGThan, tCQCKit::EStmtRes::False }
              , { L"0123", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::False }
              , { L"5555", tCQCKit::EStatements::IsLsThan, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Test the special true and false values
        fvComp.bSetValue(kCQCKit::pszFld_True);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::False }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        fvComp.bSetValue(kCQCKit::pszFld_False);
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"", tCQCKit::EStatements::IsTrue, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsFalse, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCStringFldValue fvTestOrg(43, 59);
        fvTestOrg.bSetValue(L"This is a test");
        fvTestOrg.c4IncSerialNum();
        fvTestOrg.c4IncSerialNum();
        TCQCStringFldValue fvTestCopy(fvTestOrg);
        TCQCStringFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  (fvTestOrg.strValue() != fvTestCopy.strValue()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  (fvTestOrg.strValue() != fvTestAssign.strValue()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_StrListFldVal
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_IntVal: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_StrListFldVal::TTest_StrListFldVal() :

    TTest_FldValComm
    (
        L"StringList Fld Value", L"Tests the string lsit field value class"
    )
{
}

TTest_StrListFldVal::~TTest_StrListFldVal()
{
}


// ---------------------------------------------------------------------------
//  TTest_StrListVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_StrListFldVal::eRunTest(  TTextStringOutStream&   strmOut
                                , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Check the constructors
    tCIDLib::TBoolean bRes;
    {
        TCQCStrListFldValue fvTest1;
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest1, tCQCKit::EFldTypes::StringList, 0, 0, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest1.colValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    {
        TCQCStrListFldValue fvTest2(31, 71);
        TTFWCurLn clnAt(CID_FILE, CID_LINE);
        bRes = bTestBaseStuff
        (
            strmOut, clnAt, fvTest2, tCQCKit::EFldTypes::StringList, 31, 71, 1, kCIDLib::False
        );

        if (!bRes)
        {
            strmOut << TFWCurLn << L"Ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest2.colValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"Default ctor state is incorrect\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Some values we'll use below
    const TString strVal1(L"Value1");
    const TString strVal2(L"Value2");
    const TString strVal3(L"Value3");
    const TString strVal4(L"Value4");
    const TString strVal5(L"Value5");

    const tCIDLib::TCh* const pszFmtVal1(L"\"Value1\", \"Value2\", \"Value3\"");
    const tCIDLib::TCh* const pszFmtVal2(L"\"Value3\", \"Value4\", \"Value5\"");

    TCQCStrListFldValue::TValList colVal1;
    colVal1.objAdd(strVal1);
    colVal1.objAdd(strVal2);
    colVal1.objAdd(strVal3);
    TCQCStrListFldValue::TValList colVal2;
    colVal2.objAdd(strVal3);
    colVal2.objAdd(strVal4);
    colVal2.objAdd(strVal5);
    TVector<TString> colVal3;

    //
    //  Test setting and getting back the value, and also test that it correctly
    //  reports whether the value actually changed or not.
    //
    {
        TCQCStrListFldValue fvTest3;
        fvTest3.bSetValue(colVal1);
        if (!bCompLists(fvTest3.colValue(), colVal1))
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        fvTest3.bSetValue(colVal2);
        if (!bCompLists(fvTest3.colValue(), colVal2))
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (fvTest3.bSetValue(colVal2))
        {
            strmOut << TFWCurLn << L"Value change incorrectly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.bSetValue(colVal3))
        {
            strmOut << TFWCurLn << L"Value change not correctly reported\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!fvTest3.colValue().bIsEmpty())
        {
            strmOut << TFWCurLn << L"New value did not get set\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Test the parsing of the textual format of string lists
    {
        tCIDLib::TCard4                 c4ErrIndex;
        TCQCStrListFldValue::TValList   colVals;
        TCQCStrListFldValue             fvParse;
        fvParse.bSetValue(colVal1);

        if (!fvParse.bSetValue(pszFmtVal2, c4ErrIndex))
        {
            strmOut << TFWCurLn << L"Set of string list text failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!bCompLists(fvParse.colValue(), colVal2))
        {
            strmOut << TFWCurLn << L"Set string list had wrong value\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }


        // Make sure we catch some badly formatted values
        if (fvParse.bSetValue(L"\"Value1", c4ErrIndex)
        ||  (c4ErrIndex != 7))
        {
            strmOut << TFWCurLn << L"Failed to catch invalid string list value\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Make sure we catch some badly formatted values
        if (fvParse.bSetValue(L"\"Value1\",,", c4ErrIndex)
        ||  (c4ErrIndex != 10))
        {
            strmOut << TFWCurLn << L"Failed to catch invalid string list value\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Make sure we catch some badly formatted values
        if (fvParse.bSetValue(L"\"Value1\"\"", c4ErrIndex)
        ||  (c4ErrIndex != 9))
        {
            strmOut << TFWCurLn << L"Failed to catch invalid string list value\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Test the statement evaluation system. We have to format out the value
    //  in the text format for comparison purposes. Only a few statement types
    //  are supported for this field type.
    //
    {
        TCQCStrListFldValue fvComp;

        // Run some while it's empty
        {
            const TStmtTest aTestsTrue[] =
            {
                { L"", tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { L"", tCQCKit::EStatements::NotEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::False }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }

        // Set a value to run some statements against that
        fvComp.bSetValue(colVal1);
        {
            const TStmtTest aTestsTrue[] =
            {
                { pszFmtVal1, tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::True }
              , { pszFmtVal2, tCQCKit::EStatements::IsEqual, tCQCKit::EStmtRes::False }
              , { L"", tCQCKit::EStatements::IsNonNull, tCQCKit::EStmtRes::True }
            };
            TTFWCurLn clnAt(CID_FILE, CID_LINE);
            if (!bTestStatements(strmOut, clnAt, fvComp, aTestsTrue, tCIDLib::c4ArrayElems(aTestsTrue)))
                eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Check round trip copy/assign. These guys don't have equality ops,
    //  so we have to do it manually.
    //
    {
        TCQCStrListFldValue fvTestOrg(43, 59);
        fvTestOrg.bSetValue(colVal2);
        fvTestOrg.c4IncSerialNum();
        TCQCStrListFldValue fvTestCopy(fvTestOrg);
        TCQCStrListFldValue fvTestAssign;
        fvTestAssign = fvTestOrg;

        if ((fvTestOrg.bInError() != fvTestCopy.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestCopy.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestCopy.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestCopy.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestCopy.eFldType())
        ||  !bCompLists(fvTestOrg.colValue(), fvTestCopy.colValue()))
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if ((fvTestOrg.bInError() != fvTestAssign.bInError())
        ||  (fvTestOrg.c4DriverId() != fvTestAssign.c4DriverId())
        ||  (fvTestOrg.c4FieldId() != fvTestAssign.c4FieldId())
        ||  (fvTestOrg.c4SerialNum() != fvTestAssign.c4SerialNum())
        ||  (fvTestOrg.eFldType() != fvTestAssign.eFldType())
        ||  !bCompLists(fvTestOrg.colValue(), fvTestAssign.colValue()))
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }
    return eRes;
}



// ---------------------------------------------------------------------------
//  TTest_StrListVal: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTest_StrListFldVal::bCompLists(const   TVector<TString>&   col1
                                , const TVector<TString>&   col2)
{
    const tCIDLib::TCard4 c4Count = col1.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (col1[c4Index] != col2[c4Index])
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

