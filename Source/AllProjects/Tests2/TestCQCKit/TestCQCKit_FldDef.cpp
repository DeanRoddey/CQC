//
// FILE NAME: TestCQCKit_FldDef.cpp
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
//  This file implements the tests for the field definition classes, which are
//  used to define the attributes of driver fields.
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
RTTIDecls(TTest_FldDef,TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_FldDef
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_FldDef: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_FldDef::TTest_FldDef() :

    TTestFWTest
    (
        L"Fld Definition", L"Tests the field definition class", 1
    )
{
}

TTest_FldDef::~TTest_FldDef()
{
}


// ---------------------------------------------------------------------------
//  TTest_FldDef: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_FldDef::eRunTest( TTextStringOutStream&   strmOut
                        , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;


    // Test the default constructor
    tCIDLib::TBoolean bRes;
    {
        TCQCFldDef flddDef;

        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddDef
            , TString::strEmpty()
            , tCQCKit::EFldTypes::Count
            , tCQCKit::EFldAccess::Read
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
            , kCIDLib::False
            , 0
            , kCIDLib::pszEmptyZStr
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  Test the constructors that take most of the info up front. These
    //  two do the exactly same thing, but they are two separate ctors, so
    //  we have to test them both. One takes the limits as a string object
    //  and one as a raw string.
    //
    {
        TCQCFldDef flddVals1
        (
            L"TheField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::Write
            , L"Range: 0.0, 1.0"
        );

        // Throw in an extra config for good measure
        flddVals1.strExtraCfg(L"ExtraConfigStr");

        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals1
            , L"TheField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::Write
            , L"Range: 0.0, 1.0"
            , kCIDLib::False
            , kCIDLib::False
            , 0
            , L"ExtraConfigStr"
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    {
        TString strRange(L"Range: 0, 10");
        TCQCFldDef flddVals2
        (
            L"TheField2"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , strRange
        );

        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals2
            , L"TheField2"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , L"Range: 0, 10"
            , kCIDLib::False
            , kCIDLib::False
            , 0
            , kCIDLib::pszEmptyZStr
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    // And the one that takes no limits
    {
        TCQCFldDef flddVals3
        (
            L"  TheField3", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read
        );

        // Throw in an extra config for good measure
        flddVals3.strExtraCfg(L"My Extra Config");

        // We put spaces on the name, which it should strip off
        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals3
            , L"TheField3"
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
            , kCIDLib::False
            , 0
            , L"My Extra Config"
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  There are also Set methods that mactch all of the constructors, but
    //  with the addition of the always write flag.
    //
    {
        TCQCFldDef flddVals1;
        flddVals1.Set
        (
            L"TheField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::Write
            , L"Range: 0.0, 1.0"
            , kCIDLib::True
        );

        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals1
            , L"TheField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::Write
            , L"Range: 0.0, 1.0"
            , kCIDLib::True
            , kCIDLib::False
            , 0
            , kCIDLib::pszEmptyZStr
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    {
        TString strRange(L"Range: 0, 10");
        TCQCFldDef flddVals2;
        flddVals2.Set
        (
            L"TheField2"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , strRange
            , kCIDLib::True
        );

        // Throw in an extra config for good measure
        flddVals2.strExtraCfg(L"ExtraConfigStr");

        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals2
            , L"TheField2"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::ReadWrite
            , L"Range: 0, 10"
            , kCIDLib::True
            , kCIDLib::False
            , 0
            , L"ExtraConfigStr"
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    // And the one that takes no limits
    {
        TCQCFldDef flddVals3;
        flddVals3.Set
        (
            L"TheField3  "
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , kCIDLib::False
        );

        // Throw in an extra config for good measure
        flddVals3.strExtraCfg(L"My Extra Config");

        // We spaces on the name, which it should strip off
        TTFWCurLn tfwclHere(CID_FILE, CID_LINE);
        bRes = bTestVals
        (
            strmOut
            , tfwclHere
            , flddVals3
            , L"TheField3"
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , kCIDLib::pszEmptyZStr
            , kCIDLib::False
            , kCIDLib::False
            , 0
            , L"My Extra Config"
        );

        if (!bRes)
            eRes = tTestFWLib::ETestRes::Failed;
    }

    // Do a round trip copy/assign
    {
        TCQCFldDef flddOrg
        (
            L"CompField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::ReadWrite
            , L"Range: 0.1, 1.5"
        );

        flddOrg.bAlwaysWrite(kCIDLib::True);
        flddOrg.c4Id(50);
        flddOrg.bPrivate(kCIDLib::True);
        flddOrg.bQueuedWrite(kCIDLib::True);
        flddOrg.strExtraCfg(L"SomeExtraConfig");

        TCQCFldDef flddCopy(flddOrg);
        if (flddCopy != flddOrg)
        {
            strmOut << TFWCurLn << L"Round trip copy failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        TCQCFldDef flddAssign;
        flddAssign = flddOrg;
        if (flddAssign != flddOrg)
        {
            strmOut << TFWCurLn << L"Round trip assign failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Do a round trip streaming
    {
        TCQCFldDef flddOrg
        (
            L"CompField"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::ReadWrite
            , L"Range: 0.1, 1.5"
        );

        flddOrg.bAlwaysWrite(kCIDLib::True);
        flddOrg.c4Id(50);
        flddOrg.bPrivate(kCIDLib::True);
        flddOrg.bQueuedWrite(kCIDLib::True);
        flddOrg.strExtraCfg(L"SomeExtraConfig");

        TBinMBufOutStream strmWrite(8192UL);
        TBinMBufInStream strmRead(strmWrite);
        strmWrite << flddOrg << kCIDLib::FlushIt;
        strmRead.Reset();

        TCQCFldDef flddNew;
        strmRead >> flddNew;

        if (flddOrg != flddNew)
        {
            strmOut << TFWCurLn << L"Round trip streaming failed\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_FldDef: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTest_FldDef::bTestVals(        TTextStringOutStream&   strmOut
                        , const TTFWCurLn&              tfwclAt
                        , const TCQCFldDef&             flddToTest
                        , const TString&                strName
                        , const tCQCKit::EFldTypes      eType
                        , const tCQCKit::EFldAccess     eAccess
                        , const tCIDLib::TCh* const     pszLimits
                        , const tCIDLib::TBoolean       bAlwaysWrite
                        , const tCIDLib::TBoolean       bQueuedWrite
                        , const tCIDLib::TCard4         c4Id
                        , const tCIDLib::TCh* const     pszExtraCfg)
{
    if (flddToTest.strName() != strName)
    {
        strmOut << tfwclAt << L"Expected field name '" << strName << L"' but got '"
                << flddToTest.strName() << L"'\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.eType() != eType)
    {
        strmOut << tfwclAt << L"Expected field type '"
                << tCQCKit::strXlatEFldTypes(eType) << L"' but got '"
                << tCQCKit::strXlatEFldTypes(flddToTest.eType()) << L"'\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.eAccess() != eAccess)
    {
        strmOut << tfwclAt << L"Expected field access '"
                << tCQCKit::strXlatEFldAccess(eAccess) << L"' but got '"
                << tCQCKit::strXlatEFldAccess(flddToTest.eAccess()) << L"'\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.strLimits() != pszLimits)
    {
        strmOut << tfwclAt << L"Expected field limits '" << pszLimits
                << L"' but got '" << flddToTest.strLimits() << L"'\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.c4Id() != c4Id)
    {
        strmOut << tfwclAt << L"Expected field id '" << c4Id
                << L"' but got '" << flddToTest.c4Id() << L"'\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.bAlwaysWrite() != bAlwaysWrite)
    {
        strmOut << tfwclAt << L"AlwaysWrite attribute was incorrect\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.bQueuedWrite() != bQueuedWrite)
    {
        strmOut << tfwclAt << L"Queued write attribute was incorrect\n\n";
        return kCIDLib::False;
    }

    if (flddToTest.strExtraCfg() != pszExtraCfg)
    {
        strmOut << tfwclAt << L"The extra config was incorrect\n\n";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}

