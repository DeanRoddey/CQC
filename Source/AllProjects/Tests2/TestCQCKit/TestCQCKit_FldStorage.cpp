//
// FILE NAME: TestCQCKit_FldStorage.cpp
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
//  This file implements the tests for the field storage classes, which are
//  used to hold field values in drivers and elsewhere. These tests are at
//  level 2. They work in terms of the field definition and field value classes
//  which are level 1 tests.
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
RTTIDecls(TTest_FldStoreBase,TTestFWTest)
RTTIDecls(TTest_FldStoreBool,TTest_FldStoreBase)



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldStoreBase
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldStoreBase: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldStoreBase::TTest_FldStoreBase() :

    TTestFWTest
    (
        L"Base Field Storage", L"Tests the basic field storage classes", 2
    )
    , m_strMoniker(L"TestDriver")
{
}

TTest_FldStoreBase::TTest_FldStoreBase( const   TString&        strName
                                        , const TString&        strDescr
                                        , const tCIDLib::TCard4 c4Level) :

    TTestFWTest(strName, strDescr, c4Level)
    , m_strMoniker(L"TestDriver")
{
}

TTest_FldStoreBase::~TTest_FldStoreBase()
{
}


// ---------------------------------------------------------------------------
//  TTest_BoolVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldStoreBase::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Create a test field store, we'll just use boolean for the basic tests
    TCQCFldDef flddTest
    (
        L"TestField"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , TString::strEmpty()
    );
    TCQCFldStoreBool cfsTest(m_strMoniker, flddTest, new TCQCFldBoolLimit);
    if (!bTestDefValues(strmOut, TFWCurLn, cfsTest))
        eRes = tTestFWLib::ETestRes::Failed;

    // Set and read back members to make sure they are working
    cfsTest.bAlwaysWrite(kCIDLib::True);
    if (!cfsTest.bAlwaysWrite())
    {
        strmOut << TFWCurLn << L"Always write wasn't set correctly\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }
    cfsTest.bAlwaysWrite(kCIDLib::False);
    if (cfsTest.bAlwaysWrite())
    {
        strmOut << TFWCurLn << L"Always write wasn't cleared correctly\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    cfsTest.bInError(kCIDLib::True);
    if (!cfsTest.bInError())
    {
        strmOut << TFWCurLn << L"In Error wasn't set correctly\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }
    cfsTest.bInError(kCIDLib::False);
    if (cfsTest.bInError())
    {
        strmOut << TFWCurLn << L"In Error wasn't cleared correctly\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  Set field values and make sure that changes are reported and the
    //  serial number is appropriately bumped when the value changes. Set
    //  it once, so that we know what the value is, then remember the
    //  serial number.
    //
    //  Note that the change reporting and the serial number are really
    //  handled by the enclosed field value object, and that is tested
    //  separated in the field value classes. But we just check here that
    //  the encapsulation is working correctly.
    //
    TCQCBoolFldValue fldvTest(1, 1);
    fldvTest.bSetValue(kCIDLib::True);
    cfsTest.bSetValue(fldvTest);
    tCIDLib::TCard4 c4SerialNum = cfsTest.c4SerialNum();

    // If we set the same value, it shouldn't indicate a change
    if (cfsTest.bSetValue(fldvTest))
    {
        strmOut << TFWCurLn << L"A value change was incorrectly reported\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (cfsTest.c4SerialNum() != c4SerialNum)
    {
        strmOut << TFWCurLn << L"The value serial number shouldn't have changed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Now set a changed value and check the results
    fldvTest.bSetValue(kCIDLib::False);
    if (!cfsTest.bSetValue(fldvTest))
    {
        strmOut << TFWCurLn << L"A value change was not reported\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (cfsTest.c4SerialNum() != c4SerialNum + 1)
    {
        strmOut << TFWCurLn << L"The value serial number wasn't bumped\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_BoolVal: Protected, non-virtual methods
// ---------------------------------------------------------------------------

//
//  All of the derived classes can call this after construction, to make
//  sure that defaults are correct. This class calls it as well as part of
//  the simplest validation pass.
//
tCIDLib::TBoolean
TTest_FldStoreBase::bTestDefValues(         TTextStringOutStream&   strmOut
                                    , const TTFWCurLn&              tfwclAt
                                    , const TCQCFldStore&           cfsToTest)
{
    tCIDLib::TBoolean bRet = kCIDLib::True;

    // Test all the basic settings
    if (cfsToTest.bAlwaysWrite())
    {
        strmOut << tfwclAt << L"Field should not be always write\n\n";
        bRet = kCIDLib::False;
    }

    if (cfsToTest.bHasTrigger())
    {
        strmOut << tfwclAt << L"Field should not have any triggers\n\n";
        bRet = kCIDLib::False;
    }

    if (cfsToTest.bInError())
    {
        strmOut << tfwclAt << L"Field should not be in error\n\n";
        bRet = kCIDLib::False;
    }

    if (cfsToTest.c4SerialNum() != 1)
    {
        strmOut << tfwclAt << L"Initial serial number wasn't 1\n\n";
        bRet = kCIDLib::False;
    }

    if (cfsToTest.strMoniker() != m_strMoniker)
    {
        strmOut << tfwclAt << L"Moniker was not correctly initialized\n\n";
        bRet = kCIDLib::False;
    }
    return bRet;
}


const TString& TTest_FldStoreBase::strMoniker() const
{
    return m_strMoniker;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_FldStoreBool
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldStoreBool: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldStoreBool::TTest_FldStoreBool() :

    TTest_FldStoreBase
    (
        L"Bool Field Storage", L"Tests boolean field value storage", 2
    )
{
}

TTest_FldStoreBool::~TTest_FldStoreBool()
{
}


// ---------------------------------------------------------------------------
//  TTest_BoolVal: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldStoreBool::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    TCQCFldDef flddTest
    (
        L"TestField"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , TString::strEmpty()
    );
    TCQCFldStoreBool cfsTest(strMoniker(), flddTest, new TCQCFldBoolLimit);
    if (!bTestDefValues(strmOut, TFWCurLn, cfsTest))
        eRes = tTestFWLib::ETestRes::Failed;

    //
    //  Check the potential new value validation stuff.
    //
    //  This should indicate an OK, which implies a new value, since the
    //  default was false and we are therefore changing it.
    //
    if (cfsTest.eValidate(kCIDLib::True) != tCQCKit::EValResults::OK)
    {
        strmOut << TFWCurLn << L"Should have indicated a new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Do it again after setting the value and it should indicate no change
    cfsTest.SetValue(kCIDLib::True);
    if (cfsTest.eValidate(kCIDLib::True) != tCQCKit::EValResults::Unchanged)
    {
        strmOut << TFWCurLn << L"Should not have indicated a new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Do the version that converts from a string
    tCIDLib::TBoolean bVal;
    if (cfsTest.eValidate(kCQCKit::pszFld_True, bVal) != tCQCKit::EValResults::Unchanged)
    {
        strmOut << TFWCurLn << L"Should not have indicated a new value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    if (bVal != kCIDLib::True)
    {
        strmOut << TFWCurLn << L"String validation converted to incorrect value\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Make sure it catches something unconvertable
    if (cfsTest.eValidate(L"Booger", bVal) != tCQCKit::EValResults::Unconvertable)
    {
        strmOut << TFWCurLn << L"Value should have been unconvertable\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}

