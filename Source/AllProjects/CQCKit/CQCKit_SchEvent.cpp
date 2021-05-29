//
// FILE NAME: CQCKit_SchEvent.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/19/2004
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
//  This file implements the scheduled events class, which is a simple
//  derivative of the base command source class (i.e. an 'action'.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//

// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSchEvent,TCQCStdCmdSrc)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCEvCl_Action
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 1 -
        //      Reset to 1 for 1.7 since we did a manual translation during the
        //      installer upgrade process.
        //
        //  Version 2 -
        //      Added the m_bPaused member so that we can pause events without
        //      removing them.
        //
        //  Version 3 -
        //      Added the m_bLoggable member so that events can be selectively
        //      logged.
        // -----------------------------------------------------------------------
        constexpr   tCIDLib::TCard2     c2FmtVersion = 3;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCSchEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TKeyedCQCSchEvent: Public, static methods
// ---------------------------------------------------------------------------

// For sorting scheduled events by next runtime
tCIDLib::ESortComps
TKeyedCQCSchEvent::eCompByTime( const   TKeyedCQCSchEvent&  csrc1
                                , const TKeyedCQCSchEvent&  csrc2)
{
    if (csrc1.enctAt() < csrc2.enctAt())
        return tCIDLib::ESortComps::FirstLess;
    else if (csrc1.enctAt() > csrc2.enctAt())
        return tCIDLib::ESortComps::FirstGreater;
    return tCIDLib::ESortComps::Equal;
}


// ---------------------------------------------------------------------------
//  TKeyedCQCSchEvent: Constructors and destructor
// ---------------------------------------------------------------------------
TKeyedCQCSchEvent::TKeyedCQCSchEvent() :

    m_c4SerialNum(1)
{
}

TKeyedCQCSchEvent::TKeyedCQCSchEvent(const  TString&        strPath
                                    , const TCQCSchEvent&   csrcBase) :
    TCQCSchEvent(csrcBase)
    , m_c4SerialNum(1)
    , m_strPath(strPath)
{
}

TKeyedCQCSchEvent::~TKeyedCQCSchEvent()
{
}


// ---------------------------------------------------------------------------
//  TKeyedCQCSchEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TCard4 TKeyedCQCSchEvent::c4SerialNum() const
{
    return m_c4SerialNum;
}


tCIDLib::TCard4 TKeyedCQCSchEvent::c4SerialNum(const tCIDLib::TCard4 c4ToSet)
{
    m_c4SerialNum = c4ToSet;
    return m_c4SerialNum;
}


const TString& TKeyedCQCSchEvent::strPath() const
{
    return m_strPath;
}

const TString& TKeyedCQCSchEvent::strPath(const TString& strToSet)
{
    m_strPath = strToSet;
    return m_strPath;
}


tCIDLib::TVoid TKeyedCQCSchEvent::SetConfig(const TCQCSchEvent& csrcBase)
{
    // Just set it on our base class
    TCQCSchEvent::operator=(csrcBase);
}




// ---------------------------------------------------------------------------
//  CLASS: TCQCSchEvent
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// TCQCSchEvent: Public, static methods
// ---------------------------------------------------------------------------

//
//  A helper to parse out the day, hour, minute value as they are formatted in
//  the XML Gateway and at the action level, into three space separated decimal
//  numbers.
//
tCIDLib::TBoolean
TCQCSchEvent::bParseDHM(const   TString&            strDHM
                        ,       tCIDLib::TCard4&    c4Day
                        ,       tCIDLib::TCard4&    c4Hour
                        ,       tCIDLib::TCard4&    c4Min)
{
    TStringTokenizer stokTime(&strDHM, L" ");
    TString strDay;
    TString strHour;
    TString strMinute;
    if (!stokTime.bGetNextToken(strDay)
    ||  !stokTime.bGetNextToken(strHour)
    ||  !stokTime.bGetNextToken(strMinute))
    {
        return kCIDLib::False;
    }

    try
    {
        c4Day = strDay.c4Val();
        c4Hour = strHour.c4Val();
        c4Min = strMinute.c4Val();
    }

    catch(...)
    {
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


//
//  A helper to parse out the day/month mask bits as they are formatted by in
//  the XML Gateway and at the action level. It returns the mask bits and the
//  number of bits we saw.
//
tCIDLib::TBoolean
TCQCSchEvent::bParseMask(const  TString&            strValue
                        ,       tCIDLib::TCard4&    c4Mask
                        ,       tCIDLib::TCard4&    c4BitCount)
{
    // Assume a zero mask until proven otherwise
    c4BitCount = 0;
    c4Mask = 0;

    // The mask must be a string of either 7 or 12 1s and 0s.
    if (!strValue.bIsEmpty())
    {
        c4BitCount = strValue.c4Length();
        if ((c4BitCount != 7) && (c4BitCount != 12))
            return kCIDLib::False;

        //
        //  Make sure it's all 1s and 0s and that there's at least one
        //  1 entry.
        //
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4BitCount; c4Index++)
        {
            if (strValue[c4Index] == kCIDLib::chDigit1)
                c4Mask |= (0x1UL << c4Index);
            else if (strValue[c4Index] != kCIDLib::chDigit0)
                return kCIDLib::False;
        }

        // It cannot be all zeros
        if (!c4Mask)
            return kCIDLib::False;
    }
    return kCIDLib::True;
}




// ---------------------------------------------------------------------------
// TCQCSchEvent: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCSchEvent::TCQCSchEvent() :

    m_bLoggable(kCIDLib::False)
    , m_bPaused(kCIDLib::False)
    , m_c4Day(0)
    , m_c4Hour(0)
    , m_c4Mask(0)
    , m_c4Min(0)
    , m_eType(tCQCKit::ESchTypes::Once)
    , m_enctAt(TTime::enctNowPlusMins(60))
    , m_i4Offset(0)
{
    //
    //  Round the time down to the nearest minute so that they all kick
    //  off on minute boundaries.
    //
    tCIDLib::TEncodedTime enctTmp = m_enctAt / kCIDLib::enctOneMinute;
    m_enctAt = enctTmp * kCIDLib::enctOneMinute;

    // Add our one supported command event
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSchedTime)
        , kCQCKit::strEvId_OnScheduledTime
        , tCQCKit::EActCmdCtx::Standard
    );
}

TCQCSchEvent::TCQCSchEvent(const TString& strDescription) :

    TCQCStdCmdSrc(strDescription)
    , m_bLoggable(kCIDLib::False)
    , m_bPaused(kCIDLib::False)
    , m_c4Day(0)
    , m_c4Hour(0)
    , m_c4Mask(0)
    , m_c4Min(0)
    , m_eType(tCQCKit::ESchTypes::Once)
    , m_enctAt(TTime::enctNowPlusMins(60))
    , m_i4Offset(0)
{
    //
    //  Round the time down to the nearest minute so that they all kick
    //  off on minute boundaries.
    //
    tCIDLib::TEncodedTime enctTmp = m_enctAt / kCIDLib::enctOneMinute;
    m_enctAt = enctTmp * kCIDLib::enctOneMinute;

    // Add our one supported command event
    AddEvent
    (
        facCQCKit().strMsg(kKitMsgs::midCmdEv_OnSchedTime)
        , kCQCKit::strEvId_OnScheduledTime
        , tCQCKit::EActCmdCtx::Standard
    );
}

TCQCSchEvent::~TCQCSchEvent()
{
}


// ---------------------------------------------------------------------------
//  TCQCSchEvent: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCSchEvent::operator==(const TCQCSchEvent& csrcSrc) const
{
    if (&csrcSrc != this)
    {
        if (!TParent::operator==(csrcSrc))
            return kCIDLib::False;

        // Check stuff first that are most likely to be different
        if ((m_eType     != csrcSrc.m_eType)
        ||  (m_strId     != csrcSrc.m_strId)
        ||  (m_bPaused   != csrcSrc.m_bPaused)
        ||  (m_c4Day     != csrcSrc.m_c4Day)
        ||  (m_c4Hour    != csrcSrc.m_c4Hour)
        ||  (m_c4Mask    != csrcSrc.m_c4Mask)
        ||  (m_c4Min     != csrcSrc.m_c4Min)
        ||  (m_i4Offset  != csrcSrc.m_i4Offset)
        ||  (m_bLoggable != csrcSrc.m_bLoggable))
        {
            return kCIDLib::False;
        }

        // If this is a one shot, we count the start time, else it's an internal detail
        if ((m_eType == tCQCKit::ESchTypes::Once) && (m_enctAt != csrcSrc.m_enctAt))
            return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCSchEvent::operator!=(const TCQCSchEvent& csrcSrc) const
{
    return !operator==(csrcSrc);
}


// ---------------------------------------------------------------------------
//  TCQCSchEvent: Public, non-virtual methods
// ---------------------------------------------------------------------------

// Returns true if this guy is a one shot event
tCIDLib::TBoolean TCQCSchEvent::bIsOneShot() const
{
    return (m_eType == tCQCKit::ESchTypes::Once);
}


// Return true if this is one of the period based types
tCIDLib::TBoolean TCQCSchEvent::bIsPeriodic() const
{
    return
    (
        (m_eType == tCQCKit::ESchTypes::MinPeriod)
        || (m_eType == tCQCKit::ESchTypes::HrPeriod)
        || (m_eType == tCQCKit::ESchTypes::DayPeriod)
    );
}


// Return true if this is one of the schedule based types
tCIDLib::TBoolean TCQCSchEvent::bIsScheduled() const
{
    return
    (
        (m_eType == tCQCKit::ESchTypes::Daily)
        || (m_eType == tCQCKit::ESchTypes::Weekly)
        || (m_eType == tCQCKit::ESchTypes::Monthly)
    );
}


// Return true if this is one of the sunrise/sunset based types
tCIDLib::TBoolean TCQCSchEvent::bIsSunBased() const
{
    return
    (
        (m_eType == tCQCKit::ESchTypes::Sunrise)
        || (m_eType == tCQCKit::ESchTypes::Sunset)
    );
}


// Get or set the loggable status
tCIDLib::TBoolean TCQCSchEvent::bLoggable() const
{
    return m_bLoggable;
}

tCIDLib::TBoolean TCQCSchEvent::bLoggable(const tCIDLib::TBoolean bToSet)
{
    m_bLoggable = bToSet;
    return m_bLoggable;
}


// Get or set the paused status
tCIDLib::TBoolean TCQCSchEvent::bPaused() const
{
    return m_bPaused;
}

tCIDLib::TBoolean TCQCSchEvent::bPaused(const tCIDLib::TBoolean bToSet)
{
    m_bPaused = bToSet;
    return m_bPaused;
}


// These return the day, hour, mask, minute, period, etc...
tCIDLib::TCard4 TCQCSchEvent::c4Day() const
{
    return m_c4Day;
}

tCIDLib::TCard4 TCQCSchEvent::c4Hour() const
{
    return m_c4Hour;
}

tCIDLib::TCard4 TCQCSchEvent::c4Mask() const
{
    return m_c4Mask;
}

tCIDLib::TCard4 TCQCSchEvent::c4Minute() const
{
    return m_c4Min;
}

tCIDLib::TCard4 TCQCSchEvent::c4Period() const
{
    tCIDLib::TCard4 c4Ret;
    switch(m_eType)
    {
        case tCQCKit::ESchTypes::MinPeriod :
            c4Ret = m_c4Min;
            break;

        case tCQCKit::ESchTypes::HrPeriod :
            c4Ret = m_c4Hour;
            break;

        case tCQCKit::ESchTypes::DayPeriod :
            c4Ret = m_c4Day;
            break;

        default :
            // It's not a periodic, so this is illegal
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvSys_NotPeriodicType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppError
            );

            // Won't happen but makes analyzer happy
            return 0;
    };
    return c4Ret;
}


tCIDLib::TCard4 TCQCSchEvent::c4SubPeriod() const
{
    tCIDLib::TCard4 c4Ret;
    switch(m_eType)
    {
        case tCQCKit::ESchTypes::MinPeriod :
            c4Ret = 0;
            break;

        case tCQCKit::ESchTypes::HrPeriod :
            c4Ret = m_c4Min;
            break;

        case tCQCKit::ESchTypes::DayPeriod :
            c4Ret = m_c4Hour;
            break;

        default :
            // It's not a periodic, so this is illegal
            facCQCKit().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kKitErrs::errcEvSys_NotPeriodicType
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppError
            );

            // Won't happen but makes analyzer happy
            return 0;
    };
    return c4Ret;
}


//
//  This one calculates the next time that this action should be scheduled. So it moves
//  forward in a way that is appropriate for the type of action.
//
//  Note that one-shots are not affected by this since they are set to a specific time
//  and date by the user.
//
tCIDLib::TVoid
TCQCSchEvent::CalcNextTime( const  tCIDLib::TFloat8&    f8Lat
                            , const tCIDLib::TFloat8&   f8Long)
{
    // If a one shot, nothing to do
    if (m_eType == tCQCKit::ESchTypes::Once)
        return;

    //
    //  We only need to reschedule if the current time is past the next
    //  scheduled time.
    //
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    const tCIDLib::TEncodedTime enctCur = tmCur.enctTime();


    tCIDLib::TCard4     c4Hour;
    tCIDLib::TCard4     c4Millis;
    tCIDLib::TCard4     c4Min;
    tCIDLib::TCard4     c4Sec;
    tCIDLib::TCard4     c4Year;
    tCIDLib::EMonths    eMonth;
    tCIDLib::TCard4     c4Day;
    switch(m_eType)
    {
        case tCQCKit::ESchTypes::Daily :
        {
            //
            //  We need to keep running forward by days, and checking
            //  the configured hour/min in each day until we get past
            //  the current time or at or past the start time. This is
            //  easiest done using Julian dates and then adjusting the
            //  hour/minute.
            //
            tCIDLib::TEncodedTime enctNext = 0;
            TTime tmLast(m_enctAt);
            tCIDLib::TCard4 c4Julian = tmLast.c4ToJulian();

            //
            //  We have to count equal to the start time time since it
            //  will normally be set to the next runnable time. So if
            //  we had to get past it, we'd slowly creep forward every
            //  time the time was calculated.
            //
            TTime tmNext;
            do
            {
                // Set up for the current Julian date
                tmNext.FromJulian(c4Julian);

                // Now get the details out and adjust the hour/min
                tmNext.eExpandDetails
                (
                    c4Year
                    , eMonth
                    , c4Day
                    , c4Hour
                    , c4Min
                    , c4Sec
                    , c4Millis
                    , enctNext
                );
                c4Hour = m_c4Hour;
                c4Min = m_c4Min;

                // Set up for the new values and get the stamp out
                tmNext.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Min);
                enctNext = tmNext.enctTime();

                //
                //  Move the Julian date forward. We may break out this
                //  time, but that's ok.
                //
                c4Julian++;

            }   while ((enctCur > enctNext) || (m_enctAt > enctNext));

            // Ok, set the at time to the next possible time
            m_enctAt = enctNext;
            break;
        }

        case tCQCKit::ESchTypes::Weekly :
        case tCQCKit::ESchTypes::Monthly :
        {
            TTime                   tmLast(m_enctAt);
            tCIDLib::TCard4         c4Julian = tmLast.c4ToJulian();
            tCIDLib::TEncodedTime   enctNext = 0;
            TTime                   tmNext;
            while(kCIDLib::True)
            {
                // Set up for the current Julian date
                tmNext.FromJulian(c4Julian);

                //
                //  Now get the details out and adjust the hour/min to
                //  the ones set for this event to run on scheduled days.
                //
                const tCIDLib::EWeekDays eWeekDay = tmNext.eExpandDetails
                (
                    c4Year
                    , eMonth
                    , c4Day
                    , c4Hour
                    , c4Min
                    , c4Sec
                    , c4Millis
                    , enctNext
                );
                c4Hour = m_c4Hour;
                c4Min = m_c4Min;

                // Set up for the new time and get the stamp out
                tmNext.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Min);
                enctNext = tmNext.enctTime();

                //
                //  If the next time isn't even yet up to now, then clearly
                //  it's not something we are interested in. We can only
                //  take a time past the current time.
                //
                if (enctNext < enctCur)
                {
                    c4Julian++;
                    continue;
                }

                if (m_eType == tCQCKit::ESchTypes::Weekly)
                {
                    //
                    //  If this is a weekly, see if this weekday is in the
                    //  mask. If so, and the time is at or beyond the next
                    //  time, then we are done.
                    //
                    if (((tCIDLib::TCard4(1) << tCIDLib::c4EnumOrd(eWeekDay)) & m_c4Mask)
                    &&  (enctNext >= m_enctAt))
                    {
                        break;
                    }
                }
                 else
                {
                    //
                    //  If a monthly, see if the month is in the mask, and
                    //  the day of the month is the configured one. If so,
                    //  and the new time is at or beyond the next time, then
                    //  we are done.
                    //
                    //  Also make sure to deal with leap years. If the day
                    //  isn't valid for the month, then it's a leap year
                    //  day and we don't want to run it this month.
                    //
                    if (((tCIDLib::TCard4(1) << tCIDLib::c4EnumOrd(eMonth)) & m_c4Mask)
                    &&  (enctNext >= m_enctAt)
                    &&  (c4Day == m_c4Day)
                    &&  (c4Day <= TTime::c4MaxDaysForMonth(eMonth, c4Year)))
                    {
                        break;
                    }
                }

                // Not there yet, so move the Julian date forward
                c4Julian++;
            }

            // Ok, set the at time to the next possible time
            m_enctAt = enctNext;
            break;
        }

        case tCQCKit::ESchTypes::MinPeriod :
        case tCQCKit::ESchTypes::HrPeriod :
        case tCQCKit::ESchTypes::DayPeriod :
        {
            //
            //  For day or hour ones we have to make sure it's on a particular
            //  hour or minute. Note that it could be OK for today or the current
            //  hour, depending on what the set hour/min is. So we don't want to add
            //  the offset yet. We can test based on now first.
            //
            TTime tmNext(enctCur);
            tCIDLib::TEncodedTime enctDummy;
            tmNext.eExpandDetails
            (
                c4Year
                , eMonth
                , c4Day
                , c4Hour
                , c4Min
                , c4Sec
                , c4Millis
                , enctDummy
            );

            if (m_eType == tCQCKit::ESchTypes::HrPeriod)
            {
                // Go to the set minute
                c4Min = m_c4Min;
            }
             else if (m_eType == tCQCKit::ESchTypes::DayPeriod)
            {
                // Go to the top of the set hour
                c4Min = 0;
                c4Hour = m_c4Hour;
            }

            // And now set them back and store the result
            tmNext.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Min);
            m_enctAt = tmNext.enctTime();

            //
            //  If this is at or before now, then we need to move forward by the offset
            //  amount.
            //
            if (m_enctAt <= enctCur)
            {
                if (m_eType == tCQCKit::ESchTypes::MinPeriod)
                    m_enctAt += m_c4Min * kCIDLib::enctOneMinute;
                else if (m_eType == tCQCKit::ESchTypes::HrPeriod)
                    m_enctAt += m_c4Hour * kCIDLib::enctOneHour;
                else
                    m_enctAt += m_c4Day * kCIDLib::enctOneDay;
            }
            break;
        }

        case tCQCKit::ESchTypes::Sunrise :
        {
            TTime tmNew(tmCur);
            DoSRCalc(tmCur, tmNew, f8Lat, f8Long);
            m_enctAt = tmNew.enctTime();
            break;
        }

        case tCQCKit::ESchTypes::Sunset :
        {
            TTime tmNew(tmCur);
            DoSRCalc(tmCur, tmNew, f8Lat, f8Long);
            m_enctAt = tmNew.enctTime();
            break;
        }

        default :
        {
            // <TBD> This is bad
            break;
        }
    };
}


// Get/set the next scheduled time
tCIDLib::TEncodedTime TCQCSchEvent::enctAt() const
{
    return m_enctAt;
}

tCIDLib::TEncodedTime
TCQCSchEvent::enctAt(const tCIDLib::TEncodedTime enctToSet)
{
    //
    //  Round the time down to the nearest minute so that they all kick
    //  off on minute boundaries.
    //
    tCIDLib::TEncodedTime enctTmp = enctToSet / kCIDLib::enctOneMinute;
    m_enctAt = enctTmp * kCIDLib::enctOneMinute;
    return m_enctAt;
}


// Returns the type of scheduled event
tCQCKit::ESchTypes TCQCSchEvent::eType() const
{
    return m_eType;
}


// Formats our schedule type info for human readable purposes
tCIDLib::TVoid TCQCSchEvent::FormatToStr(TString& strToFill) const
{
    tCIDLib::TMsgId midToLoad;
    switch(m_eType)
    {
        case tCQCKit::ESchTypes::Once :
            midToLoad = kKitMsgs::midSchFmt_OneShot;
            break;

        case tCQCKit::ESchTypes::Daily :
            midToLoad = kKitMsgs::midSchFmt_Daily;
            break;

        case tCQCKit::ESchTypes::Weekly :
            midToLoad = kKitMsgs::midSchFmt_Weekly;
            break;

        case tCQCKit::ESchTypes::Monthly :
            midToLoad = kKitMsgs::midSchFmt_Monthly;
            break;

        case tCQCKit::ESchTypes::MinPeriod :
        case tCQCKit::ESchTypes::HrPeriod :
        case tCQCKit::ESchTypes::DayPeriod :
            midToLoad = kKitMsgs::midSchFmt_Periodic;
            break;

        case tCQCKit::ESchTypes::Sunrise :
            midToLoad = kKitMsgs::midSchFmt_Sunrise;
            break;

        case tCQCKit::ESchTypes::Sunset :
            midToLoad = kKitMsgs::midSchFmt_Sunset;
            break;
    };

    // Load up the string
    strToFill.LoadFromMsg(midToLoad, facCQCKit());

    // 'n' is the start date/time
    TString strTmp;
    if (strToFill.bTokenExists(kCIDLib::chLatin_n))
    {
        TTime tmAt(m_enctAt);
        tmAt.FormatToStr(strTmp, TTime::strMMDD_24HHMM());
        strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_n);
    }

    // 'p' is the periodic time
    if (strToFill.bTokenExists(kCIDLib::chLatin_p))
        strToFill.eReplaceToken(TCardinal(c4Period()), kCIDLib::chLatin_p);

    // 'd' is the days
    if (strToFill.bTokenExists(kCIDLib::chLatin_d))
        strToFill.eReplaceToken(TCardinal(m_c4Day), kCIDLib::chLatin_d);

    // 't' is the time of day
    if (strToFill.bTokenExists(kCIDLib::chLatin_t))
    {
        if (m_c4Hour < 10)
            strTmp.Append(kCIDLib::chDigit0);
        strTmp.AppendFormatted(m_c4Hour);
        strTmp.Append(TLocale::chTimeSeparator());
        if (m_c4Min < 10)
            strTmp.Append(kCIDLib::chDigit0);
        strTmp.AppendFormatted(m_c4Min);
        strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_t);
    }

    // 'l' is the list of valid days
    tCIDLib::TCard4 c4Count;
    if (strToFill.bTokenExists(kCIDLib::chLatin_l))
    {
        strTmp.Clear();
        c4Count = 0;
        tCIDLib::EWeekDays eDay = tCIDLib::EWeekDays::Min;
        for (; eDay <= tCIDLib::EWeekDays::Max; eDay++)
        {
            if ((0x1UL << tCIDLib::c4EnumOrd(eDay)) & m_c4Mask)
            {
                if (c4Count)
                    strTmp.Append(L", ");
                strTmp.Append(TLocale::strDayAbbrev(eDay));
                c4Count++;
            }
        }
        strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_l);
    }

    // 'L' is the list of valid months
    if (strToFill.bTokenExists(kCIDLib::chLatin_L))
    {
        strTmp.Clear();
        c4Count = 0;
        tCIDLib::EMonths eMonth = tCIDLib::EMonths::Min;
        for (; eMonth <= tCIDLib::EMonths::Max; eMonth++)
        {
            if ((0x1UL << tCIDLib::c4EnumOrd(eMonth)) & m_c4Mask)
            {
                if (c4Count)
                    strTmp.Append(L", ");
                strTmp.Append(TLocale::strMonthAbbrev(eMonth));
                c4Count++;
            }
        }
        strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_L);
    }

    // 'u' is the units name
    if (strToFill.bTokenExists(kCIDLib::chLatin_u))
    {
        switch(m_eType)
        {
            case tCQCKit::ESchTypes::DayPeriod :
                midToLoad = kKitMsgs::midSchUnit_Day;
            break;

            case tCQCKit::ESchTypes::HrPeriod :
                midToLoad = kKitMsgs::midSchUnit_Hour;
            break;

            case tCQCKit::ESchTypes::MinPeriod :
                midToLoad = kKitMsgs::midSchUnit_Minute;
            break;
        };

        strTmp.LoadFromMsg(midToLoad, facCQCKit());
        strToFill.eReplaceToken(strTmp, kCIDLib::chLatin_u);
    }

    //
    //  If not a one-shot, then display the next scheduled time. If it's
    //  paused format out a paused string instead.
    //
    if (m_bPaused)
    {
        strToFill.Append(L"\n\n");
        strToFill.Append(facCQCKit().strMsg(kKitMsgs::midEv_Paused));
    }
     else if (m_eType != tCQCKit::ESchTypes::Once)
    {
        TTime tmNext(m_enctAt);
        strTmp.LoadFromMsg(kKitMsgs::midSchFmt_NextTime, facCQCKit());

        TString strNext;
        tmNext.FormatToStr(strNext, TTime::strMMDD_24HHMM());
        strTmp.eReplaceToken(strNext, kCIDLib::chLatin_n);

        strToFill.Append(L"\n\n");
        strToFill.Append(strTmp);
    }
}


//
//  Get/set the offset value for this event. The caller, who knows the
//  latitude/longitude info should re-calculate us after setting the
//  offset.
//
tCIDLib::TInt4 TCQCSchEvent::i4Offset() const
{
    return m_i4Offset;
}

tCIDLib::TInt4 TCQCSchEvent::i4Offset(const tCIDLib::TInt4 i4ToSet)
{
    m_i4Offset = i4ToSet;
    return m_i4Offset;
}


//
//  Just for backwards compat during the upgrade to 5.0 process. It won't be used
//  in the 5.0 world and will eventually be removed.
//
const TString& TCQCSchEvent::strId() const
{
    return m_strId;
}

const TString& TCQCSchEvent::strId(const TString& strToSet)
{
    m_strId = strToSet;
    return m_strId;
}


//
//  These set up us for specific types of events. Each type requires different
//  info so we want to have a specific method for each type in order to
/// maintain good compile time safety.
//
tCIDLib::TVoid TCQCSchEvent::SetOneShot(const tCIDLib::TEncodedTime enctAt)
{
    m_c4Day     = 0;
    m_c4Hour    = 0;
    m_c4Mask    = 0;
    m_c4Min     = 0;
    m_eType     = tCQCKit::ESchTypes::Once;
    m_i4Offset  = 0;

    //
    //  Round the time down to the nearest minute so that they all kick
    //  off on minute boundaries.
    //
    tCIDLib::TEncodedTime enctTmp = enctAt / kCIDLib::enctOneMinute;
    m_enctAt = enctTmp * kCIDLib::enctOneMinute;

    // Calculate the next possible time after the passed start time
    CalcNextTime(0, 0);
}


tCIDLib::TVoid
TCQCSchEvent::SetPeriodic(  const   tCQCKit::ESchTypes      eType
                            , const tCIDLib::TCard4         c4Period
                            , const tCIDLib::TCard4         c4SubPeriod
                            , const tCIDLib::TEncodedTime   enctAt)
{
    // Validate the type before we make any changes
    if ((eType != tCQCKit::ESchTypes::MinPeriod)
    &&  (eType != tCQCKit::ESchTypes::HrPeriod)
    &&  (eType != tCQCKit::ESchTypes::DayPeriod))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvSys_NotPeriodicType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    m_c4Day     = 0;
    m_c4Hour    = 0;
    m_c4Mask    = 0;
    m_c4Min     = 0;
    m_eType     = eType;
    m_i4Offset  = 0;

    //
    //  Round the time down to the nearest minute so that they all kick
    //  off on minute boundaries.
    //
    tCIDLib::TEncodedTime enctTmp = enctAt / kCIDLib::enctOneMinute;
    m_enctAt = enctTmp * kCIDLib::enctOneMinute;

    if (m_eType == tCQCKit::ESchTypes::MinPeriod)
    {
        m_c4Min = c4Period;
    }
     else if (m_eType == tCQCKit::ESchTypes::HrPeriod)
    {
        m_c4Hour = c4Period;
        m_c4Min = c4SubPeriod;
    }
     else if (m_eType == tCQCKit::ESchTypes::DayPeriod)
    {
        m_c4Day = c4Period;
        m_c4Hour = c4SubPeriod;
    }

    // Calculate the next possible time after the passed start time
    CalcNextTime(0, 0);
}


tCIDLib::TVoid
TCQCSchEvent::SetScheduled( const   tCQCKit::ESchTypes      eType
                            , const tCIDLib::TCard4         c4Day
                            , const tCIDLib::TCard4         c4Hour
                            , const tCIDLib::TCard4         c4Minute
                            , const tCIDLib::TCard4         c4Mask
                            , const tCIDLib::TEncodedTime   enctAt)
{
    // Validate the info before we make any changes
    if ((eType != tCQCKit::ESchTypes::Daily)
    &&  (eType != tCQCKit::ESchTypes::Monthly)
    &&  (eType != tCQCKit::ESchTypes::Weekly))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvSys_NotScheduledType
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    if ((c4Day > 31)
    ||  (c4Hour > 23)
    ||  (c4Minute > 59))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvSys_BadTimeInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Can't have an empty mask if monthly or weekly
    if (!c4Mask
    &&  ((eType == tCQCKit::ESchTypes::Monthly)
    ||   (eType == tCQCKit::ESchTypes::Weekly)))
    {
        facCQCKit().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kKitErrs::errcEvSys_BadTimeInfo
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::NotFound
        );
    }

    // Make sure the start time is valid. If not, set it to now
    TTime tmNext;
    try
    {
        tmNext.enctTime(enctAt);
    }

    catch(...)
    {
        tmNext.SetTo(tCIDLib::ESpecialTimes::CurrentTime);
    }

    // Looks ok, so store it all
    m_c4Day = c4Day;
    m_c4Hour = c4Hour;
    m_c4Mask = c4Mask;
    m_c4Min = c4Minute;
    m_eType = eType;
    m_i4Offset  = 0;

    //
    //  Store the next possible start time and then calculate the initial
    //  time at or beyond that where this guy could run.
    //
    m_enctAt = tmNext.enctTime();
    CalcNextTime(0, 0);
}


tCIDLib::TVoid
TCQCSchEvent::SetSunrise(const  tCIDLib::TFloat8&   f8Lat
                        , const tCIDLib::TFloat8&   f8Long
                        , const tCIDLib::TInt4      i4Offset)
{
    m_c4Day     = 0;
    m_c4Hour    = 0;
    m_c4Mask    = 0;
    m_c4Min     = 0;
    m_eType     = tCQCKit::ESchTypes::Sunrise;
    m_enctAt    = 0;
    m_i4Offset  = i4Offset;

    // Calculate the first possible time for it
    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    TTime tmNew;
    DoSRCalc(tmCur, tmNew, f8Lat, f8Long);
    m_enctAt = tmNew.enctTime();
}


tCIDLib::TVoid
TCQCSchEvent::SetSunset(const   tCIDLib::TFloat8&   f8Lat
                        , const tCIDLib::TFloat8&   f8Long
                        , const tCIDLib::TInt4      i4Offset)
{
    m_c4Day     = 0;
    m_c4Hour    = 0;
    m_c4Mask    = 0;
    m_c4Min     = 0;
    m_eType     = tCQCKit::ESchTypes::Sunset;
    m_enctAt    = 0;
    m_i4Offset  = i4Offset;

    TTime tmCur(tCIDLib::ESpecialTimes::CurrentTime);
    TTime tmNew;
    DoSRCalc(tmCur, tmNew, f8Lat, f8Long);
    m_enctAt = tmNew.enctTime();
}


// ---------------------------------------------------------------------------
//  TCQCSchEvent: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCSchEvent::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Now our stuff should start with frame marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCEvCl_Action::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , clsThis()
        );
    }

    // Stream our parent's contents
    TParent::StreamFrom(strmToReadFrom);

    strmToReadFrom  >> m_c4Day
                    >> m_c4Hour
                    >> m_c4Mask
                    >> m_c4Min
                    >> m_eType
                    >> m_enctAt
                    >> m_strId
                    >> m_i4Offset;

    // If < V2, then default the paused flag, else read it in
    if (c2FmtVersion < 2)
        m_bPaused = kCIDLib::False;
    else
        strmToReadFrom >> m_bPaused;

    // If < V3, then default the loggable flag, else read it in
    if (c2FmtVersion < 3)
        m_bLoggable = kCIDLib::False;
    else
        strmToReadFrom >> m_bLoggable;

    // And it should end with an end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSchEvent::StreamTo(TBinOutStream& strmToWriteTo) const
{
    // And then our stuff, starting with a frame, and ending with an end marker
    strmToWriteTo   << tCIDLib::EStreamMarkers::Frame
                    << CQCEvCl_Action::c2FmtVersion;

    // Stream out our parent's stuff
    TParent::StreamTo(strmToWriteTo);

    strmToWriteTo   << m_c4Day
                    << m_c4Hour
                    << m_c4Mask
                    << m_c4Min
                    << m_eType
                    << m_enctAt
                    << m_strId
                    << m_i4Offset

                    // V2 stuff
                    << m_bPaused

                    // V3 stuff
                    << m_bLoggable
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TCQCSchEvent: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TCQCSchEvent::DoSRCalc(const    TTime&              tmCur
                        ,       TTime&              tmToAdjust
                        , const tCIDLib::TFloat8&   f8Lat
                        , const tCIDLib::TFloat8&   f8Long)
{
    // Create a time that is for the midnight of the passed current time
    tCIDLib::TCard4  c4Year;
    tCIDLib::EMonths eMonth;
    tCIDLib::TCard4  c4Day;
    tmToAdjust = tmCur;
    tmToAdjust.eAsDateInfo(c4Year, eMonth, c4Day);
    tmToAdjust.FromDetails(c4Year, eMonth, c4Day);

    // Save this midnight time for later
    TTime tmOrg(tmToAdjust);

    // And calculate the rise or set for today
    if (m_eType == tCQCKit::ESchTypes::Sunrise)
        tmToAdjust.SetToSunrise(f8Lat, f8Long);
    else
        tmToAdjust.SetToSunset(f8Lat, f8Long);

    // Add/subtract the offset
    tCIDLib::TEncodedTime enctOffset;
    if (m_i4Offset < 0)
    {
        enctOffset = (m_i4Offset * -1) * kCIDLib::enctOneMinute;
        tmToAdjust -= enctOffset;
    }
     else if (m_i4Offset > 0)
    {
        enctOffset = m_i4Offset * kCIDLib::enctOneMinute;
        tmToAdjust += enctOffset;
    }

    //
    //  If this time is before the current time, then we need to do it
    //  again for tomorrow. This one is guaranteed to be correct.
    //
    if (tmToAdjust <= tmCur)
    {
        // Go back to midnight today and add one day
        tmToAdjust = tmOrg;
        tmToAdjust += kCIDLib::enctOneDay;

        // And recalc the rise or set time
        if (m_eType == tCQCKit::ESchTypes::Sunrise)
            tmToAdjust.SetToSunrise(f8Lat, f8Long);
        else
            tmToAdjust.SetToSunset(f8Lat, f8Long);

        // Add/subtract the offset
        tCIDLib::TEncodedTime enctOffset;
        if (m_i4Offset < 0)
        {
            enctOffset = (m_i4Offset * -1) * kCIDLib::enctOneMinute;
            tmToAdjust -= enctOffset;
        }
         else if (m_i4Offset > 0)
        {
            enctOffset = m_i4Offset * kCIDLib::enctOneMinute;
            tmToAdjust += enctOffset;
        }
    }
}


