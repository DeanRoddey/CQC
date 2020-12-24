//
// FILE NAME: TestCQCKit_FldFilter.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/30/2011
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
//  This file implements the tests for the field filter classes.
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
RTTIDecls(TTest_FldFilter,TTestFWTest)
RTTIDecls(TTest_FldTypeFilter,TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldFilter
// PREFIX: tfwt
//
//  Tests the basic field filter class. Note that we don't do a lot here,
//  since each derivative will effectively test this class indirectly.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldFilter: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldFilter::TTest_FldFilter() :

    TTestFWTest
    (
        L"Basic Field Filters", L"Tests the base field filter class", 2
    )
{
}

TTest_FldFilter::~TTest_FldFilter()
{
}


// ---------------------------------------------------------------------------
//  TTest_FldFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldFilter::eRunTest( TTextStringOutStream&   strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Test constructors get their parms stored
    TCQCFldFilter ffiltTest(tCQCKit::EReqAccess::ReadOnly);

    // Test round trip assignment and copying
    {
        // Make a copy
        TCQCFldFilter ffiltCopy(ffiltTest);

        //
        //  Create another with different values, so we can test assignment.
        //  It doesn't have a default ctor so we have to pass it something.
        //
        TCQCFldFilter ffiltAssign(tCQCKit::EReqAccess::MReadCWrite);
        ffiltAssign = ffiltTest;

        // And now compare to the original one
        if (ffiltAssign != ffiltTest)
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (ffiltCopy != ffiltTest)
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Make sure that the equality operator agrees with inequality
        if (!(ffiltCopy == ffiltTest))
        {
            strmOut << TFWCurLn << L"Equality ops disagreed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldTypeFilter
// PREFIX: tfwt
//
//  Tests the derivative of the field filter that works in terms of the field
//  type and semantic field type, plus the ability to filter by name as well.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldTypeFilter: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldTypeFilter::TTest_FldTypeFilter() :

    TTestFWTest
    (
        L"Fld Type Filters", L"Tests the field type filter class", 3
    )
{
}

TTest_FldTypeFilter::~TTest_FldTypeFilter()
{
}


// ---------------------------------------------------------------------------
//  TTest_FldTypeFilter: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldTypeFilter::eRunTest( TTextStringOutStream&   strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // First test round trip assignment and copying on one of our guys
    {
        TCQCFldFiltTypeName ffiltTest
        (
            tCQCKit::EReqAccess::MReadCWrite
            , tCQCKit::EFldTypes::Boolean
            , L"TestName"
            , TString::strEmpty()
        );

        // Copy one
        TCQCFldFiltTypeName ffiltCopy(ffiltTest);

        //
        //  Create another with different values (no default ctor), and
        //  then assign to it.
        //
        TCQCFldFiltTypeName ffiltAssign
        (
            tCQCKit::EReqAccess::CReadMWrite
            , tCQCKit::EFldTypes::String
            , L"TestMyName"
            , L"TestMyName2"
        );
        ffiltAssign = ffiltTest;

        // And now compare to the original one
        if (ffiltAssign != ffiltTest)
        {
            strmOut << TFWCurLn << L"Assignment operator failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (ffiltCopy != ffiltTest)
        {
            strmOut << TFWCurLn << L"Copy ctor failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // Make sure that the equality operator agrees with inequality
        if (!(ffiltCopy == ffiltTest))
        {
            strmOut << TFWCurLn << L"Equality ops disagreed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Set up some field definitions and test them against some filters
    //  and see if they work as expected.
    //
    const TString& strBlank = TString::strEmpty();
    const TString strDum(L"Dum");

    TCQCFldFiltTypeName ffiltFldType
    (
        tCQCKit::EReqAccess::MReadCWrite
        , tCQCKit::EFldTypes::Boolean
    );

    TCQCFldFiltTypeName ffiltSemType
    (
        tCQCKit::EReqAccess::ReadOnly
        , tCQCKit::EFldSTypes::CurTemp
        , L"CurTemp"
        , TString::strEmpty()
    );

    TCQCFldFiltTypeName ffiltSemTypeNN
    (
        tCQCKit::EReqAccess::CReadMWrite
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldSTypes::Mute
    );

    TCQCFldFiltTypeName ffiltNameType2N
    (
        tCQCKit::EReqAccess::CReadMWrite
        , tCQCKit::EFldSTypes::LightSwitch
        , L"LightSw"
        , L"LightSwitch"
    );

    TCQCFldFiltTypeName ffiltBothTypes
    (
        tCQCKit::EReqAccess::WriteOnly
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldSTypes::MediaTransport
        , L"Transport"
        , TString::strEmpty()
    );


    TCQCFldDef flddTest1
    (
        L"LightSwitch"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LightSwitch
    );

    TCQCFldDef flddTest2
    (
        L"CurTemp"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::CurTemp
    );

    // Same as above but read only
    TCQCFldDef flddTest3
    (
        L"CurTemp"
        , tCQCKit::EFldTypes::Card
        , tCQCKit::EFldAccess::Read
        , tCQCKit::EFldSTypes::CurTemp
    );

    TCQCFldDef flddTest4
    (
        L"Transport"
        , tCQCKit::EFldTypes::String
        , tCQCKit::EFldAccess::Write
        , tCQCKit::EFldSTypes::MediaTransport
    );

    TCQCFldDef flddTest5
    (
        L"KitchLight"
        , tCQCKit::EFldTypes::Boolean
        , tCQCKit::EFldAccess::ReadWrite
        , tCQCKit::EFldSTypes::LightSwitch
    );

    const tCQCKit::EDevCats eCat = tCQCKit::EDevCats::Count;

    // We need a device class list for this, which is just empty for these tests
    tCQCKit::TDevClassList fcolDC;

    // Test 1
    if (!ffiltFldType.bCanAcceptField(flddTest1, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 1 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 2
    if (!ffiltFldType.bCanAcceptField(flddTest1, strBlank, strDum, strDum, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 2 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 3 (wrong name and semantic type)
    if (ffiltSemType.bCanAcceptField(flddTest1, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 3 should have failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 4 (wrong access type)
    if (ffiltSemType.bCanAcceptField(flddTest2, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 4 should have failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 5
    if (!ffiltSemType.bCanAcceptField(flddTest3, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 5 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 6
    if (!ffiltBothTypes.bCanAcceptField(flddTest4, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 6 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 7
    if (!ffiltNameType2N.bCanAcceptField(flddTest1, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 7 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    // Test 8 (should fail, since field name doesn't match
    if (ffiltNameType2N.bCanAcceptField(flddTest5, strBlank, strBlank, strBlank, eCat, fcolDC))
    {
        strmOut << TFWCurLn << L"Test 8 failed\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}

