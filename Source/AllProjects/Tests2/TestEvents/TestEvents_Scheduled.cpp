//
// FILE NAME: TestEvents_Scheduled.cpp
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
//  This file implements the tests for the scheduled events.
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
RTTIDecls(TTest_Scheduled1,TTestFWTest)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace TestEvents_Scheduled
{
    // Some faux lat/long values
    const tCIDLib::TFloat8  f8Lat   = 37.3;
    const tCIDLib::TFloat8  f8Long  = -121.88;
}




// ---------------------------------------------------------------------------
//  CLASS: TTest_BasicActVar
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_Scheduled1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_Scheduled1::TTest_Scheduled1() :

    TTestFWTest
    (
        L"Scheduled Events 1", L"Basic tests of scheduled events", 3
    )
{
}

TTest_Scheduled1::~TTest_Scheduled1()
{
}


// ---------------------------------------------------------------------------
//  TTest_Scheduled1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_Scheduled1::eRunTest( TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    tCIDLib::TCard4     c4Hour;
    tCIDLib::TCard4     c4Millis;
    tCIDLib::TCard4     c4Min;
    tCIDLib::TCard4     c4Sec;
    tCIDLib::TCard4     c4Year;
    tCIDLib::EMonths    eMonth;
    tCIDLib::TCard4     c4Day;

    // Do a basic one shot test
    {
        tCIDLib::TEncodedTime enctAt(TTime::enctNowPlusMins(2));
        TCQCSchEvent csrcOneShot(L"Test one shot");
        csrcOneShot.SetOneShot(enctAt);

        if (!csrcOneShot.bIsOneShot())
        {
            strmOut << TFWCurLn << L"Event did not come back as one shot\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }

        if (!bCheckInfo(strmOut, TFWCurLn, csrcOneShot, tCQCKit::ESchTypes::Once, enctAt))
            eRes = tTestFWLib::ETestRes::Failed;

        // Calculating the next time should do nothing
        CalcNext(csrcOneShot);
        if (!bCheckInfo(strmOut, TFWCurLn, csrcOneShot, tCQCKit::ESchTypes::Once, enctAt))
            eRes = tTestFWLib::ETestRes::Failed;
    }

    // Do a daily one
    {
        // Set the start time to now
        tCIDLib::TEncodedTime enctStartAt = TTime::enctNow();

        TCQCSchEvent csrcDaily(L"Daily event");
        csrcDaily.SetScheduled
        (
            tCQCKit::ESchTypes::Daily
            , 0
            , 13
            , 30
            , 0
            , enctStartAt
        );

        //  The initial starting time is now, so if the hour/minute is past
        //  the one we set, then it should run today, else it will be tomorrow.
        //
        TTime tmNext(enctStartAt);
        tCIDLib::TEncodedTime enctTmp;
        tmNext.eExpandDetails
        (
            c4Year
            , eMonth
            , c4Day
            , c4Hour
            , c4Min
            , c4Sec
            , c4Millis
            , enctTmp
        );

        tmNext.FromDetails(c4Year, eMonth, c4Day, 13, 30);
        if ((c4Hour > 13) || (c4Hour == 13) && (c4Min > 30))
        {
            //
            //  It should run tomorrow. Note that this isn't the best way
            //  to do this, since if we hit a day where there's a leap
            //  minute or something, it could fail. But not too much risk
            //  in reality.
            //
            tmNext += kCIDLib::enctOneDay;
        }

        if (!bCheckInfo(strmOut, TFWCurLn, csrcDaily, tCQCKit::ESchTypes::Daily, tmNext.enctTime()))
            eRes = tTestFWLib::ETestRes::Failed;
    }


    // Do a weekly one
    {
        // Set the start time to now
        tCIDLib::TEncodedTime enctStartAt = TTime::enctNow();

        TCQCSchEvent csrcWeekly(L"Weekly event");
        csrcWeekly.SetScheduled
        (
            tCQCKit::ESchTypes::Weekly
            , 0
            , 10
            , 30
            , 0x7F
            , enctStartAt
        );
    }

    return eRes;
}


// ---------------------------------------------------------------------------
//  TTest_Scheduled1: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTest_Scheduled1::bCheckInfo(       TTextStringOutStream&   strmOut
                            , const TTFWCurLn&              tfwclAt
                            , const TCQCSchEvent&           csrcToCheck
                            , const tCQCKit::ESchTypes      eType
                            , const tCIDLib::TEncodedTime   enctNext)
{
    if (csrcToCheck.eType() != eType)
    {
        strmOut << tfwclAt << L"Wrong schedule type\n\n";
        return kCIDLib::False;
    }


    //
    //  Currently all times are rounded down to the previous minute, so
    //  the actual value the caller set and passed in won't match.
    //
    //  Probably we should just stop doing that and let them stagger out
    //  through the minute to avoid peak activity. So, for now we just
    //  round it here, so that it can be removed easily if we stop this
    //  rounding.
    //
    tCIDLib::TEncodedTime enctActual = enctNext / kCIDLib::enctOneMinute;
    enctActual *= kCIDLib::enctOneMinute;

    if (csrcToCheck.enctAt() != enctActual)
    {
        strmOut << tfwclAt << L"Wrong next scheduled time\n\n";
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  Calculate the next time for the passed event, using the faux lat/long
//  positions.
//
tCIDLib::TVoid TTest_Scheduled1::CalcNext(TCQCSchEvent& csrcTar)
{
    csrcTar.CalcNextTime(TestEvents_Scheduled::f8Lat, TestEvents_Scheduled::f8Long);
}

