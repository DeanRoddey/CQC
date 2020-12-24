//
// FILE NAME: TestCQCKit_EventTrig.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/29/2013
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
//  This file implements the tests for the event trigger class.
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
RTTIDecls(TTest_EventTrigBase,TTestFWTest)
RTTIDecls(TTest_EventTrig,TTest_EventTrigBase)
RTTIDecls(TTest_EventTrigStd,TTest_EventTrigBase)



// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrigBase
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_EventTrigBase: Destructor
// ---------------------------------------------------------------------------
TTest_EventTrigBase::~TTest_EventTrigBase()
{
}


// ---------------------------------------------------------------------------
//  TTest_EventTrigBase: Hidden Constructor
// ---------------------------------------------------------------------------
TTest_EventTrigBase::TTest_EventTrigBase(const  TString&        strName
                                        , const TString&        strDescr
                                        , const tCIDLib::TCard4 c4Level) :
    TTestFWTest(strName, strDescr, c4Level)
{
}


// ---------------------------------------------------------------------------
//  TTest_EventTrigBase: Protected, non-virtual methods
// ---------------------------------------------------------------------------

// Helpers for the derived classes to call
tTestFWLib::ETestRes
TTest_EventTrigBase::eCheckExists(          TTextStringOutStream&   strmOut
                                    , const tCIDLib::TCard4         c4Line
                                    , const TString&                strKey
                                    , const TCQCEvent&              cevTest) const
{
    if (!cevTest.bValueExists(strKey))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Key " << strKey << L" was not found\n\n";
        return tTestFWLib::ETestRes::Failed;
    }
    return tTestFWLib::ETestRes::Success;
}

tTestFWLib::ETestRes
TTest_EventTrigBase::eCheckExists(          TTextStringOutStream&   strmOut
                                    , const tCIDLib::TCard4         c4Line
                                    , const TString&                strKey
                                    , const tCIDLib::TBoolean       bExpValue
                                    , const TCQCEvent&              cevTest) const
{
    if (!cevTest.bValueExists(strKey))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Key " << strKey << L" was not found\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (cevTest.bValueAs(strKey) != bExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Key " << strKey
                << L" did not have the expected boolean value\n\n";
        return tTestFWLib::ETestRes::Failed;
    }
    return tTestFWLib::ETestRes::Success;
}

tTestFWLib::ETestRes
TTest_EventTrigBase::eCheckExists(          TTextStringOutStream&   strmOut
                                    , const tCIDLib::TCard4         c4Line
                                    , const TString&                strKey
                                    , const tCIDLib::TCard4         c4ExpValue
                                    , const TCQCEvent&              cevTest) const
{
    if (!cevTest.bValueExists(strKey))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Key " << strKey << L" was not found\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (cevTest.c4ValueAs(strKey) != c4ExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Key " << strKey
                << L" did not have the expected Card value "
                << c4ExpValue << L"\n\n";
        return tTestFWLib::ETestRes::Failed;
    }
    return tTestFWLib::ETestRes::Success;
}

tTestFWLib::ETestRes
TTest_EventTrigBase::eCheckExists(          TTextStringOutStream&   strmOut
                                    , const tCIDLib::TCard4         c4Line
                                    , const TString&                strKey
                                    , const tCIDLib::TInt4          i4ExpValue
                                    , const TCQCEvent&              cevTest) const
{
    if (!cevTest.bValueExists(strKey))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Key " << strKey << L" was not found\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (cevTest.i4ValueAs(strKey) != i4ExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Key " << strKey
                << L" did not have the expected Int value "
                << i4ExpValue << L"\n\n";
        return tTestFWLib::ETestRes::Failed;
    }
    return tTestFWLib::ETestRes::Success;
}

tTestFWLib::ETestRes
TTest_EventTrigBase::eCheckExists(          TTextStringOutStream&   strmOut
                                    , const tCIDLib::TCard4         c4Line
                                    , const TString&                strKey
                                    , const TString&                strExpValue
                                    , const TCQCEvent&              cevTest) const
{
    TString strVal;
    if (!cevTest.bValueExists(strKey, strVal))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Key " << strKey << L" was not found\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (strVal != strExpValue)
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line) << L"Key " << strKey
                << L" did not have the expected value of '"
                << strExpValue
                << L"'\n\n";
        return tTestFWLib::ETestRes::Failed;
    }
    return tTestFWLib::ETestRes::Success;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrig
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_EventTrig: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_EventTrig::TTest_EventTrig() :

    TTest_EventTrigBase
    (
        L"Event Triggers", L"Basic event trigger tests", 2
    )
{
}

TTest_EventTrig::~TTest_EventTrig()
{
}


// ---------------------------------------------------------------------------
//  TTest_EventTrig: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_EventTrig::eRunTest(  TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  Do a basic test of just putting a block into the trigger, and a single
    //  key/value pair, and then make sure we can read it back, test round
    //  trip reliability, and basic attribute checking.
    //
    TString strVal;
    {
        const TString strKey = L"/TestBlock/TestKey";
        TCQCEvent cevTest;
        cevTest.AddBlock(L"/", L"TestBlock");
        cevTest.AddValue(L"/TestBlock", L"TestKey", L"Test Value");

        eRes = eCheckExists(strmOut, CID_LINE, strKey, cevTest);
        eRes = eCheckExists(strmOut, CID_LINE, strKey, TString(L"Test Value"), cevTest);

        // Copy it and make sure we see the same stuff in the copy
        TCQCEvent cevTest2(cevTest);
        eRes = eCheckExists(strmOut, CID_LINE, strKey, cevTest2);
        eRes = eCheckExists(strmOut, CID_LINE, strKey, TString(L"Test Value"), cevTest2);

        // Make sure it doesn't report a non-existent key as present
        if (cevTest.bValueExists(L"/DontExist/Test"))
        {
            strmOut << TFWCurLn << L"Non-existent key reported present\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        //
        //  Set various values and read them back again, using the specialized
        //  value conversion methods.
        //
        cevTest.SetValue(strKey, L"True");
        eRes = eCheckExists(strmOut, CID_LINE, strKey, kCIDLib::True, cevTest);

        cevTest.SetValue(strKey, L"False");
        eRes = eCheckExists(strmOut, CID_LINE, strKey, kCIDLib::False, cevTest);

        cevTest.SetValue(strKey, L"34");
        eRes = eCheckExists(strmOut, CID_LINE, strKey, 34UL, cevTest);

        cevTest.SetValue(strKey, L"-144");
        eRes = eCheckExists(strmOut, CID_LINE, strKey, -144L, cevTest);


        // This is not a field based trigger, so it shouldn't return any source
        TString strVal1;
        TString strVal2;

        if (cevTest.bQueryFldSrc(strVal1, strVal2))
        {
            strmOut << TFWCurLn << L"Should not have had a field source\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        // And isn't a standard CQC event, so no source
        if (cevTest.bQuerySrc(strVal1))
        {
            strmOut << TFWCurLn << L"Should not have had an event source\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

    }
    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_EventTrigStd
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_EventTrigStd: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_EventTrigStd::TTest_EventTrigStd() :

    TTest_EventTrigBase
    (
        L"Std Event Triggers", L"Standard event trigger tests", 3
    )
{
}

TTest_EventTrigStd::~TTest_EventTrigStd()
{
}


// ---------------------------------------------------------------------------
//  TTest_EventTrigStd: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_EventTrigStd::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    // Test a field trigger
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::FldChange
            , L"TestMon"
            , L"TestFld"
            , L"Test Value"
            , TString::strEmpty()
            , TString::strEmpty()
            , TString::strEmpty()
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.field:TestMon.TestFld"
            , L"TestMon.TestFld"
            , L"cqsl.fldchange"
            , L"/cqsl.fldval/val"
            , L"Test Value"
            , TString::strEmpty()
            , TString::strEmpty()
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }

    // Test a lock status
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::LockStatus
            , L"TestMon"
            , TString::strEmpty()
            , L"locked"
            , L"FrontDoor"
            , L"1234"
            , L"pad"
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.dev:TestMon"
            , L"TestMon"
            , L"cqsl.loadchange"
            , L"/cqsl.lockinfo/state"
            , L"locked"
            , L"/cqsl.lockinfo/lockid"
            , L"FrontDoor"
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }

    // Test a load change
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::LoadChange
            , L"TestMon"
            , L"TestFld"
            , L"on"
            , L"32"
            , L"Load32"
            , TString::strEmpty()
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.field:TestMon.TestFld"
            , L"TestMon.TestFld"
            , L"cqsl.loadchange"
            , L"/cqsl.loadinfo/state"
            , L"on"
            , L"/cqsl.loadinfo/loadnum"
            , L"32"
            , L"/cqsl.loadinfo/name"
            , L"Load32"
        );
    }

    // Test a motion trigger
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::Motion
            , L"TestMon"
            , L"TestFld"
            , L"start"
            , L"22"
            , L"Motion_22"
            , TString::strEmpty()
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.field:TestMon.TestFld"
            , L"TestMon.TestFld"
            , L"cqsl.motion"
            , L"/cqsl.motioninfo/type"
            , L"start"
            , L"/cqsl.motioninfo/sensornum"
            , L"22"
            , L"/cqsl.motioninfo/name"
            , L"Motion_22"
        );
    }

    // Test a user action
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::UserAction
            , L"TestMon"
            , TString::strEmpty()
            , L"type"
            , L"data"
            , TString::strEmpty()
            , TString::strEmpty()
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.dev:TestMon"
            , L"TestMon"
            , L"cqsl.useract"
            , L"/cqsl.actinfo/evtype"
            , L"type"
            , L"/cqsl.actinfo/evdata"
            , L"data"
            , TString::strEmpty()
            , TString::strEmpty()
        );
    }

    // Test a zone alarm
    {
        TCQCEvent cevTest;
        cevTest.BuildStdDrvEvent
        (
            tCQCKit::EStdDrvEvs::ZoneAlarm
            , L"TestMon"
            , L"TestFld"
            , L"notready"
            , L"12"
            , L"Zone_12"
            , TString::strEmpty()
        );

        eRes = eTestVal
        (
            strmOut
            , CID_LINE
            , cevTest
            , L"cqsl.field:TestMon.TestFld"
            , L"TestMon.TestFld"
            , L"cqsl.zonealarm"
            , L"/cqsl.zoneinfo/state"
            , L"notready"
            , L"/cqsl.zoneinfo/zonenum"
            , L"12"
            , L"/cqsl.zoneinfo/name"
            , L"Zone_12"
        );
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_EventTrigStd: Private, non-virtual methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_EventTrigStd::eTestVal(       TTextStringOutStream&   strmOut
                            , const tCIDLib::TCard4         c4Line
                            , const TCQCEvent&              cevTest
                            , const TString&                strSrc
                            , const TString&                strSrc2
                            , const TString&                strClass
                            , const TString&                strKey1
                            , const TString&                strVal1
                            , const TString&                strKey2
                            , const TString&                strVal2
                            , const TString&                strKey3
                            , const TString&                strVal3)
{
    TString strTmp;

    if (!cevTest.bIsOfClass(strClass)
    ||  (cevTest.strClass() != strClass))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Invalid trigger class\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (!cevTest.bQuerySrc(strTmp)
    ||  (strTmp != strSrc2)
    ||  (cevTest.strSource() != strSrc))
    {
        strmOut << TTFWCurLn(CID_FILE, c4Line)
                << L"Invalid trigger source\n\n";
        return tTestFWLib::ETestRes::Failed;
    }

    if (!strKey1.bIsEmpty())
    {
        if (!cevTest.bValueExists(strKey1)
        ||  !cevTest.bValueExists(strKey1, strTmp)
        ||  (strTmp != strVal1)
        ||  (cevTest.strValue(strKey1) != strTmp))
        {
            strmOut << TTFWCurLn(CID_FILE, c4Line)
                    << L"Invalid trigger value 1\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    if (!strKey2.bIsEmpty())
    {
        if (!cevTest.bValueExists(strKey2)
        ||  !cevTest.bValueExists(strKey2, strTmp)
        ||  (strTmp != strVal2)
        ||  (cevTest.strValue(strKey2) != strTmp))
        {
            strmOut << TTFWCurLn(CID_FILE, c4Line)
                    << L"Invalid trigger value 2\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    if (!strKey3.bIsEmpty())
    {
        if (!cevTest.bValueExists(strKey3)
        ||  !cevTest.bValueExists(strKey3, strTmp)
        ||  (strTmp != strVal3)
        ||  (cevTest.strValue(strKey3) != strTmp))
        {
            strmOut << TTFWCurLn(CID_FILE, c4Line)
                    << L"Invalid trigger value 3\n\n";
            return tTestFWLib::ETestRes::Failed;
        }
    }

    return tTestFWLib::ETestRes::Success;
}

