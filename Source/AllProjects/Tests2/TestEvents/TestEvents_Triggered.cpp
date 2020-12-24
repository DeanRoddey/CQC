//
// FILE NAME: TestEvents_Triggered.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/21/2009
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
//  This file implements the tests for the Triggered events.
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
#include    "TestEvents.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_TrigEvFilters,TTestFWTest)
RTTIDecls(TTest_TrigEvStream,TTestFWTest)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace TestEvents_Triggered
{
    // A structure we use to set up lists of pre-fab filters to run through
    struct TFilterInfo
    {
        tCIDLib::TCard1             c1CompValRegEx;
        tCIDLib::TCard1             c1FldRegEx;
        tCQCKit::ETEvFilters        eType;
        const tCIDLib::TCh* const   pszEvFld;
        const tCIDLib::TCh* const   pszCompVal;

        tCIDLib::TCard1             c1ShouldPass;  // 0=no, 1=yes, 2=exception in set
        tCIDLib::TCard4             c4EvIndex;
    };
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_TrigEvFilters
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_TrigEvFilters: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_TrigEvFilters::TTest_TrigEvFilters() :

    TTestFWTest
    (
        L"Triggered Events Filters", L"Tests triggered event filters", 3
    )
{
}

TTest_TrigEvFilters::~TTest_TrigEvFilters()
{
}


// ---------------------------------------------------------------------------
//  TTest_TrigEvFilters: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_TrigEvFilters::eRunTest(  TTextStringOutStream&   strmOut
                                , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  We just invoke some private helpers which will create some filters
    //  and then throw some pre-fab events at them, and see if they correctly
    //  pass or fail the filter.
    //
    eRes = eTestUserAction(strmOut);

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_TrigEvFilters: Private, non-virtual methods
// ---------------------------------------------------------------------------

// This one will test some user action type filters
tTestFWLib::ETestRes
TTest_TrigEvFilters::eTestUserAction(TTextStringOutStream& strmOut)
{
    // Set up a list of filter info structs for our filters we want to test
    struct TestEvents_Triggered::TFilterInfo aFilters[] =
    {
        // Tests against event field equals
        { 0, 0, tCQCKit::ETEvFilters::Arbitrary        , L"/cqsl.fldval/val", L"YourFldValue", 1, 1 }

        // Tests against is from source
      , { 0, 0, tCQCKit::ETEvFilters::IsFromSource     , L"", L"cqsl.field:FCMoniker.FCField", 1, 1 }
      , { 0, 0, tCQCKit::ETEvFilters::IsFromSource     , L"", L"cqsl.dev:UAMoniker", 1, 0 }
      , { 1, 0, tCQCKit::ETEvFilters::IsFromSource     , L"", L"cqsl.dev:.*", 1, 0 }

        // Tests against is of class
      , { 0, 0, tCQCKit::ETEvFilters::IsOfClass        , L"", L"cqsl.fldchange", 1, 1 }
      , { 0, 0, tCQCKit::ETEvFilters::IsOfClass        , L"", L"cqsl.fldchange", 0, 0 }
      , { 1, 0, tCQCKit::ETEvFilters::IsOfClass        , L"", L"cqsl.*", 1, 0 }
      , { 1, 0, tCQCKit::ETEvFilters::IsOfClass        , L"", L"cqsl.*", 1, 1 }

        // Basic tests that it's just a user action
      , { 0, 0, tCQCKit::ETEvFilters::IsUserAction     , L"", L"", 1, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsUserAction     , L"", L"", 0, 1 }

        // Tests again is this user action
      , { 0, 0, tCQCKit::ETEvFilters::IsThisUserAction , L"MyEvType", L"MyEvData", 1, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsThisUserAction , L"MyEvType", L"MyEvData", 0, 1 }
      , { 0, 0, tCQCKit::ETEvFilters::IsThisUserAction , L"MyEvType", L"MyEvStuff", 0, 0 }
      , { 1, 0, tCQCKit::ETEvFilters::IsThisUserAction , L"MyEvType", L"My(E|A)vData", 1, 0 }
      , { 1, 1, tCQCKit::ETEvFilters::IsThisUserAction , L"My(E|A)vType", L"My(E|A)vData", 1, 0 }

        // Tests against is user action from
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFrom , L"UAMoniker", L"", 1, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFrom , L"Test", L"", 0, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFrom , L"UAMoniker", L"", 0, 1 }

      , { 0, 1, tCQCKit::ETEvFilters::IsUserActionFrom  , L"UA.*", L"", 1, 0 }
      , { 0, 1, tCQCKit::ETEvFilters::IsUserActionFrom  , L"UB.*", L"", 0, 0 }

        // Tests against is user action from
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFor  , L"UAMoniker", L"MyEvType", 1, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFor  , L"UAMoniker", L"OtherType", 0, 0 }

      , { 1, 1, tCQCKit::ETEvFilters::IsUserActionFor  , L"[A-Z][A-Z]Moniker", L"MyEv.*", 1, 0 }
      , { 1, 1, tCQCKit::ETEvFilters::IsUserActionFor  , L"[A-Z][A-Z]Moniker", L"MyAV.*", 0, 0 }

        // Tests against new field value equals
      , { 0, 0, tCQCKit::ETEvFilters::IsNewFldValFor   , L"FCMoniker.FCField", L"YourFldValue", 1, 1 }
      , { 0, 0, tCQCKit::ETEvFilters::IsNewFldValFor   , L"FCMoniker.FCField", L"FldValue", 0, 1 }
      , { 0, 0, tCQCKit::ETEvFilters::IsNewFldValFor   , L"FCMoniker.SomeField", L"YourFldValue", 0, 1 }

        // Some load change tests
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChange     , L"", L"", 1, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeFrom , L"LCMoniker", L"", 1, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeFrom , L"LCMonikerX", L"", 0, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeFor  , L"LCMoniker.LCField", L"", 1, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeOn   , L"", L"", 1, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeOff  , L"", L"", 0, 2 }

        // Some motion tests
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionEv       , L"", L"", 1, 3 }
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionEvFrom   , L"MoMoniker", L"", 1, 3 }
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionEvFrom   , L"MoMonikerX", L"", 0, 3 }
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionEvFor    , L"MoMoniker.MoField", L"", 1, 3 }
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionStartEv  , L"", L"", 0, 3 }
      , { 0, 0, tCQCKit::ETEvFilters::IsMotionEndEv    , L"", L"", 1, 3 }

        // Some presence tests
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEv     , L"", L"", 1, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEv     , L"", L"", 0, 6 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEv     , L"", L"enter", 1, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEv     , L"", L"exit", 0, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvFrom , L"PrMoniker", L"", 1, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvFrom , L"PrMoniker", L"enter", 1, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvFrom , L"PrMoniker", L"exit", 0, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvInArea , L"PrMoniker", L"Bedroom", 1, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvInArea , L"PrMoniker", L"Kitchen", 0, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvFrom , L"BadMoniker", L"", 0, 7 }
      , { 0, 0, tCQCKit::ETEvFilters::IsPresenceEvInArea , L"BadMoniker", L"Kitchen", 0, 7 }
      , { 0, 1, tCQCKit::ETEvFilters::IsPresenceEvInArea , L"Pr.*", L"Kitchen", 0, 7 }

        // Some zone alarm tests
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarm      , L"", L"", 1, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFrom  , L"ZAMoniker", L"", 1, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFrom  , L"ZAMonikerX", L"", 0, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFor   , L"ZAMoniker.ZAField", L"", 1, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneSecured    , L"", L"", 0, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneViolated   , L"", L"", 1, 4 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneSecured    , L"", L"", 1, 6 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarm      , L"", L"notready", 1, 5 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFor   , L"ZAMoniker.ZAField", L"notready", 1, 5 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFrom  , L"ZAMoniker", L"notready", 1, 5 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarmFrom  , L"ZAMoniker", L"violated", 0, 5 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneAlarm      , L"", L"violated", 1, 4 }

        // Some lock status tests
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatus, L"", L"", 1, 8 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusFrom, L"LSMoniker", L"", 1, 8 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusFrom, L"LSMoniker", L"FrontDoor", 1, 8 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusFrom, L"LSMoniker", L"BackDoor", 0, 8 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusCode, L"LSMoniker", L"123", 1, 8 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusCode, L"LSMoniker", L"123", 0, 9 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLockStatusCode, L"BadMoniker", L"123", 0, 9 }

        //
        //  These should cause exceptions during the set of the filter so they
        //  are marked with 2 for should pass ot indicate this. They pass
        //  incorrect parameters for the filter type.
        //
      , { 0, 0, tCQCKit::ETEvFilters::IsUserAction     , L"XX", L"YY", 2, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsUserActionFor  , L"UAMoniker", L"", 2, 0 }
      , { 0, 0, tCQCKit::ETEvFilters::IsLoadChangeFor  , L"", L"UAMoniker", 2, 2 }
      , { 0, 0, tCQCKit::ETEvFilters::IsZoneViolated   , L"x", L"y", 2, 4 }
    };
    const tCIDLib::TCard4 c4Count = tCIDLib::c4ArrayElems(aFilters);

    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  Create some events to test again. The above test entries have an index
    //  into the list to indicate which one they work against.
    //
    TCQCEvent acevTests[10];
    acevTests[0].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::UserAction
        , L"UAMoniker"
        , TString::strEmpty()
        , L"MyEvType"
        , L"MyEvData"
        , TString::strEmpty()
        , TString::strEmpty()
    );
    acevTests[1].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::FldChange
        , L"FCMoniker"
        , L"FCField"
        , L"YourFldValue"
        , TString::strEmpty()
        , TString::strEmpty()
        , TString::strEmpty()
    );
    acevTests[2].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::LoadChange
        , L"LCMoniker"
        , L"LCField"
        , L"on"
        , L"1"
        , L"Load1"
        , TString::strEmpty()
    );
    acevTests[3].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::Motion
        , L"MoMoniker"
        , L"MoField"
        , L"end"
        , L"2"
        , L"Motion2"
        , TString::strEmpty()
    );
    acevTests[4].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::ZoneAlarm
        , L"ZAMoniker"
        , L"ZAField"
        , L"violated"
        , L"5"
        , L"Zone5a"
        , TString::strEmpty()
    );
    acevTests[5].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::ZoneAlarm
        , L"ZAMoniker"
        , L"ZAField"
        , L"notready"
        , L"5"
        , L"Zone5b"
        , TString::strEmpty()
    );
    acevTests[6].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::ZoneAlarm
        , L"ZAMoniker"
        , L"ZAField"
        , L"secure"
        , L"5"
        , L"Zone5c"
        , TString::strEmpty()
    );
    acevTests[7].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::Presence
        , L"PrMoniker"
        , TString::strEmpty()
        , L"enter"
        , L"Johnny"
        , L"Bedroom"
        , TString::strEmpty()
    );
    acevTests[8].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::LockStatus
        , L"LSMoniker"
        , TString::strEmpty()
        , L"locked"
        , L"FrontDoor"
        , L"123"
        , TString::strEmpty()
    );
    acevTests[9].BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::LockStatus
        , L"LSMoniker"
        , TString::strEmpty()
        , L"locked"
        , L"BackDoor"
        , TString::strEmpty()
        , TString::strEmpty()
    );


    //
    //  For the ese the 'is night' status doesn't matter. But get some other
    //  info that the evaluation method expects the host to provide.
    //
    TTime tmNow(tCIDLib::ESpecialTimes::CurrentTime);
    tCIDLib::TCard4 c4Hour;
    tCIDLib::TCard4 c4Minute;
    tCIDLib::TCard4 c4Second;
    tmNow.c4AsTimeInfo(c4Hour, c4Minute, c4Second);

    // Run through all our filters and try them
    tCIDLib::TBoolean   bFldUsed;
    tCIDLib::TBoolean   bCompUsed;
    tCIDLib::TBoolean   bFldReq;
    tCIDLib::TBoolean   bCompReq;
    tCIDLib::TBoolean   bFldRegEx;
    tCIDLib::TBoolean   bCompRegEx;
    TCQCTEvFilter       cevfTest;

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TestEvents_Triggered::TFilterInfo& Info = aFilters[c4Index];
        try
        {
            //
            //  Double check ourselves and make sure we aren't doing any
            //  tests that are not valid.
            //
            TCQCTEvFilter::QueryFilterTypeInfo
            (
                Info.eType
                , bFldUsed
                , bCompUsed
                , bFldReq
                , bCompReq
                , bFldRegEx
                , bCompRegEx
            );

            // If the comp value cannot be reg ex but we marked it such
            if (Info.c1CompValRegEx && !bCompRegEx)
            {
                strmOut << L"Comp reg ex not allowed on test #" << (c4Index + 1)
                        << kCIDLib::DNewLn;
                eRes = tTestFWLib::ETestRes::Failed;
                continue;
            }

            // If the field value cannot be reg ex but we marked it such
            if (Info.c1FldRegEx && !bFldRegEx)
            {
                strmOut << L"Fld reg ex not allowed on test #" << (c4Index + 1)
                        << kCIDLib::DNewLn;
                eRes = tTestFWLib::ETestRes::Failed;
                continue;
            }

            // Looks reasonable, so set up the filter
            try
            {
                cevfTest.Set
                (
                    kCIDLib::False
                    , Info.eType
                    , Info.pszEvFld
                    , Info.pszCompVal
                    , Info.c1CompValRegEx == 1
                    , Info.c1FldRegEx == 1
                );

                if (Info.c1ShouldPass == 2)
                {
                    // It should have caused an exception
                    strmOut << L"Filter #" << (c4Index + 1)
                            << L" should have caused an exception on set"
                            << kCIDLib::DNewLn;
                    eRes = tTestFWLib::ETestRes::Failed;
                    continue;
                }
            }

            catch(TError& errToCatch)
            {

                // If not expected to create an exception, rethrow
                if (Info.c1ShouldPass != 2)
                {
                    errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                    throw;
                }

                // Else, expected so skip to the next one
                continue;
            }

            // It set ok, so let's run it
            tCIDLib::TBoolean bRes = cevfTest.bEvaluate
            (
                acevTests[Info.c4EvIndex]
                , kCIDLib::False
                , tmNow.enctTime()
                , c4Hour
                , c4Minute
            );

            if (Info.c1ShouldPass && !bRes)
            {
                // It should have passed but failed
                strmOut << L"Filter #" << (c4Index + 1)
                        << L" should have passed" << kCIDLib::DNewLn;
                eRes = tTestFWLib::ETestRes::Failed;
            }
             else if (!Info.c1ShouldPass && bRes)
            {
                // It should have failed but passed
                strmOut << L"Filter #" << (c4Index + 1)
                        << L" should not have passed" << kCIDLib::DNewLn;
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }

        catch(const TError& errToCatch)
        {
            strmOut << TFWCurLn << L"User action filter #" << (c4Index + 1)
                    << L" Error=" << errToCatch.strErrText();
            if (errToCatch.bHasAuxText())
                strmOut << L"\n" << errToCatch.strAuxText();
            strmOut << kCIDLib::DNewLn;
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    //
    //  Now run them again, and set the Negate flag. So now the result should
    //  be the opposite of what it normally would be.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TestEvents_Triggered::TFilterInfo& Info = aFilters[c4Index];
        try
        {
            //
            //  Skip the ones that should cause exceptions. We already tested
            //  those above well enough.
            //
            if (Info.c1ShouldPass == 2)
                continue;

            // Invoke with negation of the result enabled
            cevfTest.Set
            (
                kCIDLib::True
                , Info.eType
                , Info.pszEvFld
                , Info.pszCompVal
                , Info.c1CompValRegEx == 1
                , Info.c1FldRegEx == 1
            );

            tCIDLib::TBoolean bRes = cevfTest.bEvaluate
            (
                acevTests[Info.c4EvIndex]
                , kCIDLib::False
                , tmNow.enctTime()
                , c4Hour
                , c4Minute
            );

            //
            //  Check for the opposite this time. If it should have passed,
            //  and it did, or if it shouldn't have passed and it didn't,
            //  then the negation didn't work.
            //
            if (Info.c1ShouldPass && bRes)
            {
                // It should have passed but failed
                strmOut << L"User action filter #" << (c4Index + 1)
                        << L" should not have passed" << kCIDLib::DNewLn;
            }
             else if (!Info.c1ShouldPass && !bRes)
            {
                // It should have failed but passed
                strmOut << L"User action filter #" << (c4Index + 1)
                        << L" should have passed" << kCIDLib::DNewLn;
            }
        }

        catch(const TError& errToCatch)
        {
            strmOut << TFWCurLn << L"User action filter #" << (c4Index + 1)
                    << L" Error=" << errToCatch.strErrText() << kCIDLib::DNewLn;
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    return eRes;
}



// ---------------------------------------------------------------------------
//  CLASS: TTest_TrigEvStream
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_TrigEvStream: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_TrigEvStream::TTest_TrigEvStream() :

    TTestFWTest
    (
        L"Triggered Event Streaming", L"Tests triggered event streaming support", 3
    )
{
}

TTest_TrigEvStream::~TTest_TrigEvStream()
{
}


// ---------------------------------------------------------------------------
//  TTest_TrigEvStream: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_TrigEvStream::eRunTest(TTextStringOutStream&  strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    TCQCEvent cevTestOut;
    cevTestOut.BuildStdDrvEvent
    (
        tCQCKit::EStdDrvEvs::LockStatus
        , L"LSMoniker"
        , TString::strEmpty()
        , L"locked"
        , L"FrontDoor"
        , L"123"
        , TString::strEmpty()
    );

    // Create linked in/out memory buffer streams for testing
    TBinMBufOutStream strmTestOut(2048UL);
    TBinMBufInStream strmTestIn(strmTestOut);

    TCQCEvent cevTestIn;
    strmTestOut << cevTestOut << kCIDLib::FlushIt;
    strmTestIn >> cevTestIn;

    if (cevTestIn != cevTestOut)
    {
        strmOut << L"Event trigger failed binary streaming round trip\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    return eRes;
}
