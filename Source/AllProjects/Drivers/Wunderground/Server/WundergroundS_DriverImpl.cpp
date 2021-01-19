//
// FILE NAME: WundergroundS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2011
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
//  This is a test driver which implements the driver interface.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "WundergroundS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWundergroundSDriver,TCQCServerBase)



// ---------------------------------------------------------------------------
//   CLASS: TWeatherSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWeatherSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TWundergroundSDriver::TWundergroundSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_bShowMetric(kCIDLib::False)
    , m_bV2Mode(kCIDLib::False)
    , m_c4FldIdCurAsOf(kCIDLib::c4MaxCard)
    , m_c4FldIdCurBaro(kCIDLib::c4MaxCard)
    , m_c4FldIdCurCondText(kCIDLib::c4MaxCard)
    , m_c4FldIdCurDewPoint(kCIDLib::c4MaxCard)
    , m_c4FldIdCurFeelsLike(kCIDLib::c4MaxCard)
    , m_c4FldIdCurHumidity(kCIDLib::c4MaxCard)
    , m_c4FldIdCurIcon(kCIDLib::c4MaxCard)
    , m_c4FldIdCurMoonPhase(kCIDLib::c4MaxCard)
    , m_c4FldIdCurMoonPhIcon(kCIDLib::c4MaxCard)
    , m_c4FldIdCurPrecip(kCIDLib::c4MaxCard)
    , m_c4FldIdCurTemp(kCIDLib::c4MaxCard)
    , m_c4FldIdCurUV(kCIDLib::c4MaxCard)
    , m_c4FldIdCurWindDir(kCIDLib::c4MaxCard)
    , m_c4FldIdCurWindGust(kCIDLib::c4MaxCard)
    , m_c4FldIdCurWindSpeed(kCIDLib::c4MaxCard)
    , m_c4FldIdCurVisibility(kCIDLib::c4MaxCard)
    , m_c4FldIdFCCurrent(kCIDLib::c4MaxCard)
    , m_c4FldIdFC12Hour(kCIDLib::c4MaxCard)
    , m_c4FldIdFC24Hour(kCIDLib::c4MaxCard)
    , m_c4FldIdFC36Hour(kCIDLib::c4MaxCard)
    , m_c4FldIdSunrise(kCIDLib::c4MaxCard)
    , m_c4FldIdSunset(kCIDLib::c4MaxCard)
    , m_c4FailCnt(0)
    , m_c4PollMinutes(30)
    , m_eDtFmt(EDtFmt_MMDDYY)
    , m_enctNextPoll(0)
    , m_i4HighTempRange(120)
    , m_i4LowTempRange(-20)
    , m_strCmd_SetLocCode(L"SetLocCode")

    , m_strAttr_BaroC(L"pressure_mb")
    , m_strAttr_BaroF(L"pressure_in")
    , m_strAttr_Conditions(L"conditions")
    , m_strAttr_CScale(L"celsius")
    , m_strAttr_Day(L"day")
    , m_strAttr_Description(L"description")
    , m_strAttr_DewPntC(L"dewpoint_c")
    , m_strAttr_DewPntF(L"dewpoint_f")
    , m_strAttr_FCText(L"fcttext")
    , m_strAttr_FCTextMetric(L"fcttext_metric")
    , m_strAttr_FeelsLikeC(L"feelslike_c")
    , m_strAttr_FeelsLikeF(L"feelslike_f")
    , m_strAttr_FScale(L"fahrenheit")
    , m_strAttr_Hour(L"hour")
    , m_strAttr_Humidity(L"relative_humidity")
    , m_strAttr_Icon(L"icon")
    , m_strAttr_Min(L"min")
    , m_strAttr_Minute(L"minute")
    , m_strAttr_MoonAge(L"ageOfMoon")
    , m_strAttr_Month(L"month")
    , m_strAttr_MonthName(L"monthname")
    , m_strAttr_ObsTime(L"observation_time_rfc822")
    , m_strAttr_POP(L"pop")
    , m_strAttr_Precip(L"precip_today_in")
    , m_strAttr_PrecipMetric(L"precip_today_metric")
    , m_strAttr_TempC(L"temp_c")
    , m_strAttr_TempF(L"temp_f")
    , m_strAttr_Type(L"type")
    , m_strAttr_UV(L"UV")
    , m_strAttr_VisibilityK(L"visibility_km")
    , m_strAttr_VisibilityM(L"visibility_mi")
    , m_strAttr_Weather(L"weather")
    , m_strAttr_Weekday(L"weekday")
    , m_strAttr_WeekdayShort(L"weekday_short")
    , m_strAttr_WindDegrees(L"wind_degrees")
    , m_strAttr_WindDir(L"wind_dir")
    , m_strAttr_WindGustK(L"wind_gust_kph")
    , m_strAttr_WindGustM(L"wind_gust_mph")
    , m_strAttr_WindSpeedK(L"wind_kph")
    , m_strAttr_WindSpeedM(L"wind_mph")
    , m_strAttr_Year(L"year")

    , m_strElem_CurObservation(L"current_observation")
    , m_strElem_Date(L"date")
    , m_strElem_Error(L"error")
    , m_strElem_Forecast(L"forecast")
    , m_strElem_ForecastDay(L"forecastday")
    , m_strElem_High(L"high")
    , m_strElem_Low(L"low")
    , m_strElem_MoonPhase(L"moon_phase")
    , m_strElem_SimpleForecast(L"simpleforecast")
    , m_strElem_Sunrise(L"sunrise")
    , m_strElem_Sunset(L"sunset")
    , m_strElem_TextForecast(L"txt_forecast")

    , m_strVal_NA(L"NA")
    , m_strVal_NA2(L"-9999")
    , m_strVal_NA3(L"null")
    , m_strVal_NA4(L"-999")
    , m_strVal_NA5(L"--")
    , m_strVal_NA6(L"-9999.00")
{
}

TWundergroundSDriver::~TWundergroundSDriver()
{
}


// ---------------------------------------------------------------------------
//  TWundergroundSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TWundergroundSDriver::bGetCommResource(TThread&)
{
    // We don't have a persistent connection
    return kCIDLib::True;
}


tCIDLib::TBoolean TWundergroundSDriver::bWaitConfig(TThread& thrThis)
{
    return kCIDLib::True;
}


// We just call the update method to get our fields initialized
tCQCKit::ECommResults TWundergroundSDriver::eConnectToDevice(TThread&)
{
    //
    //  See if we can do try it again yet. Our connect time is 10 seconds
    //  but we don't do it faster than we are allowed. The failure that
    //  threw us offline will have set a new next time appropriately based
    //  on the failure.
    //
    if (TTime::enctNow() >= m_enctNextPoll)
    {
        // It's time to poll
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWundgrMsgs::midStatus_TryingConnQ
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        if (!bDoUpdate(m_strLocCode))
            return tCQCKit::ECommResults::LostConnection;
    }

    // Reset any connection oriented stuff and return success
    m_c4FailCnt = 0;

    return tCQCKit::ECommResults::Success;
}


tCQCKit::EDrvInitRes TWundergroundSDriver::eInitializeImpl()
{
    TString strTmp;

    // Remember if we are V2 mode or V1
    m_bV2Mode = c4ArchVersion() > 1;

    //
    //  We will have gotten two prompt values with the key and developer id
    //  that we need to build up the queries.
    //
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    if (!cqcdcTmp.bFindPromptValue(L"DevId", L"Text", m_strDevId)
    ||  !cqcdcTmp.bFindPromptValue(L"Location", L"Text", m_strLocCode))
    {
        facWundergroundS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The developer id or location values were not received"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Remember if we should show metric format
    m_bShowMetric = kCIDLib::False;
    if (cqcdcTmp.bFindPromptValue(L"Measure", L"Selected", strTmp)
    &&  (strTmp == L"Metric"))
    {
        m_bShowMetric = kCIDLib::True;
    }

    m_eDtFmt = EDtFmt_MMDDYY;
    if (cqcdcTmp.bFindPromptValue(L"DateFormat", L"Selected", strTmp))
    {
        if (strTmp == L"DD/MM/YY")
            m_eDtFmt = EDtFmt_DDMMYY;
    }

    m_i4LowTempRange = -20;
    if (cqcdcTmp.bFindPromptValue(L"LowTemp", L"Value", strTmp))
    {
        if (!strTmp.bToInt4(m_i4LowTempRange))
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The low temp prompt value was bad, defaulting to -20"
                , tCIDLib::ESeverities::Status
            );
            m_i4LowTempRange = -20;
        }
    }

    m_i4HighTempRange = 120;
    if (cqcdcTmp.bFindPromptValue(L"HighTemp", L"Value", strTmp))
    {
        if (!strTmp.bToInt4(m_i4HighTempRange))
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The high temp prompt value was bad, defaulting to 120"
                , tCIDLib::ESeverities::Status
            );
            m_i4HighTempRange = 120;
        }
    }

    TString strInitLocName;
    cqcdcTmp.bFindPromptValue(L"LocName", L"Text", strInitLocName);

    //
    //  A new prompt was added to control how many minutes between polls.
    //  If not available, then we use the old 30 minute value that used to
    //  be the fixed amount, for backwards compatibility.
    //
    m_c4PollMinutes = 30;
    if (cqcdcTmp.bFindPromptValue(L"PollMinutes", L"Value", strTmp))
    {
        if (!strTmp.bToCard4(m_c4PollMinutes))
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , L"The poll minutes prompt was invalid, defaulting to 30 minutes"
                , tCIDLib::ESeverities::Status
            );
            m_c4PollMinutes = 30;
        }
    }

    //
    //  Set up a collection of field defs to set. Almost all of our fields
    //  are read only fields that we fill with info gotten from the data
    //  feed.
    //
    RegisterFields();

    //
    //  Store the location code and initial location name now since they will
    //  never change unless a client sends us a SetLocCode command. No need to
    //  field lock here since we aren't connected yet.
    //
    bStoreStringFld(L"LocCode", m_strLocCode, kCIDLib::True);
    if (m_bV2Mode)
        bStoreStringFld(L"WEATH#LocName", strInitLocName, kCIDLib::True);
    else
        bStoreStringFld(L"LocName", strInitLocName, kCIDLib::True);

    //
    //  We set the poll time to ten seconds, and the reconnect time to
    //  30 seconds.
    //
    SetPollTimes(10000, 30000);

    // Clear the next poll time so that we can do one immediately for connect
    m_enctNextPoll = 0;

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TWundergroundSDriver::ePollDevice(TThread&)
{
    if (TTime::enctNow() >= m_enctNextPoll)
    {
        // It's time to poll
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Low)
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWundgrMsgs::midStatus_TryingPollQ
                , tCIDLib::ESeverities::Status
                , tCIDLib::EErrClasses::AppStatus
            );
        }

        if (bDoUpdate(m_strLocCode))
        {
            // Make sure the failure count gets cleared since we got a good one
            m_c4FailCnt = 0;
        }
         else
        {
            // Bump the failure count. If we hit 3 of them in a row, we give up
            m_c4FailCnt++;
            if (m_c4FailCnt >= 3)
                return tCQCKit::ECommResults::LostConnection;
        }
    }
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TWundergroundSDriver::ReleaseCommResource()
{
    // Nothing to do release
}


tCIDLib::TVoid TWundergroundSDriver::TerminateImpl()
{
    // Nothing to do for this one
}



// ---------------------------------------------------------------------------
//  TWundergroundSDriver: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to query the WC server and store away the info we get, or
//  react to any failure. We throttle the speed of access via a time stamp,
//  so if it's not time yet, we just return true to say it all went well. If
//  we do actually do a pass, we update the time stamps appropriately for the
//  next update times.
//
//  We take the location code to use so that we can also use this as a test
//  of location code validity when they use the SetLocCode backdoor command.
//  We can make sure it works before we accept the new location.
//
//
//  !!!!
//  NOTE: WE MUST get the next poll time updated here, no matter what happens,
//  so that we don't accidentally poll too quickly and eat up the customer's
//  available queries. But we can't do it on entry because the appropriate
//  new time depends on whether we worked, or, if we failed, how we failed.
//  !!!!
//
tCIDLib::TBoolean TWundergroundSDriver::bDoUpdate(const TString& strLocCode)
{
    //
    //  First we try/catch around an attempt to actually get the data. If this
    //  fails, then we can reschedule pretty quickly to try again, since we
    //  didn't use up one of our queries in this case. We never were able to
    //  actually get the data.
    //
    tCIDLib::TCard4     c4ContLen = 0;
    THeapBuf            mbufBody(8, 0x20000, 8 * 1024);
    try
    {
        // Build up the URL
        TString strURL(L"http://api.wunderground.com/api/");
        strURL.Append(m_strDevId);
        strURL.Append(L"/conditions/forecast10day/astronomy/q/");
        strURL.Append(m_strLocCode);
        strURL.Append(L".json");

        // Parse it into a URL object
        TURL urlQ(strURL, tCIDSock::EQualified::Full);

        //
        //  Create an 'agent' name from the user's account info and the
        //  name of the local host.
        //
        TString strAgent = TSysInfo::strIPHostName();
        strAgent.Append(kCIDLib::chUnderscore);
        strAgent.Append(m_strDevId);

        // Do the HTTP query
        tCIDLib::TKVPList   colInHdrLines;
        tCIDLib::TKVPList   colOutHdrLines;
        tCIDNet::EHTTPCodes eCodeType;
        TString             strRepText;
        TString             strContType;
        THTTPClient         httpcQuery;

        const tCIDLib::TCard4 c4ResCode = httpcQuery.c4SendGet
        (
            0
            , urlQ
            , TTime::enctNowPlusSecs(8)
            , strAgent
            , L"application/json"
            , eCodeType
            , strRepText
            , colOutHdrLines
            , strContType
            , mbufBody
            , c4ContLen
            , kCIDLib::False
            , colInHdrLines
        );

        if (c4ResCode != kCIDNet::c4HTTPStatus_OK)
        {
            // Update the time stamp, so that we try again in a bit
            m_enctNextPoll = TTime::enctNowPlusMins(kWundergroundS::c4QueryFailMins);

            // Log an error if in verbose mode
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facWundergroundS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWundgrErrs::errcComm_HTTPErr
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Nak
                    , TCardinal(c4ResCode)
                );
            }
            return kCIDLib::False;
        }

        #if 0
        // Temporarily use a stored response for development purposes
        TBinaryFile flSrc(L"\\Tmp\\weatherundergrounddata.txt");
        flSrc.Open
        (
            tCIDLib::EAccessModes::Read
            , tCIDLib::ECreateActs::OpenIfExists
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
        );
        c4ContLen = tCIDLib::TCard4(flSrc.c8CurSize());
        flSrc.c4ReadBuffer(mbufBody, c4ContLen, tCIDLib::EAllData::FailIfNotAll);
        #endif
    }

    catch(TError& errToCatch)
    {
        //
        //  Update the time stamps to try again in a while to see
        //  if we can get connected again.
        //
        m_enctNextPoll = TTime::enctNowPlusMins(kWundergroundS::c4QueryFailMins);

        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWundgrErrs::errcComm_HTTPQFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }

    //
    //  Now we can try to parse it. If we fail here, we don't reschedule so
    //  soon, because we did get a response, and so we used a query probably,
    //  though it may have been just an error reply from the server (which
    //  probably means invalid location data or account key, in which case
    //  also no need to rush because the user will have to fix this.)
    //
    try
    {
        //
        //  Create a text stream over the buffer. We just give him the buffer
        //  contents.
        //
        TTextMBufInStream strmSrc(tCIDLib::ForceMove(mbufBody), c4ContLen);

        // Create a parser object and try to parse it
        TJSONParser jprsTest;
        TJSONValue* pjprsnRootVal = jprsTest.pjprsnParse(strmSrc);
        TJanitor<TJSONValue> janRoot(pjprsnRootVal);

        CIDAssert
        (
            pjprsnRootVal->eType() == tCIDNet::EJSONVTypes::Object
            , L"Root JSON element was not an object"
        );

        // That worked, so try to process the information
        ProcessResponse(*static_cast<TJSONObject*>(pjprsnRootVal));
    }

    catch(TError& errToCatch)
    {
        //
        //  Update the time stamps to try again in a longer period of time
        //  because we may have used up a query but just failed to handle it
        //  correctly.
        //
        m_enctNextPoll = TTime::enctNowPlusMins(kWundergroundS::c4ContentFailMins);

        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facWundergroundS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWundgrErrs::errcData_ParseFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
                , TCardinal(kWundergroundS::c4ContentFailMins)
            );
        }
        return kCIDLib::False;
    }

    // It worked, so set the next poll time
    if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
    {
        facWundergroundS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , kWundgrErrs::errcData_NextPoll
            , tCIDLib::ESeverities::Status
            , tCIDLib::EErrClasses::AppStatus
            , TCardinal(m_c4PollMinutes)
        );
    }
    m_enctNextPoll = TTime::enctNowPlusMins(m_c4PollMinutes);

    return kCIDLib::True;
}


// Test an attribute value to see if it's a non-value of some sort
tCIDLib::TBoolean TWundergroundSDriver::bIsNAValue(const TString& strVal)
{
    return  strVal.bIsEmpty()
            || (strVal == m_strVal_NA)
            || (strVal == m_strVal_NA2)
            || (strVal == m_strVal_NA3)
            || (strVal == m_strVal_NA4)
            || (strVal == m_strVal_NA5)
            || (strVal == m_strVal_NA6);
}


// Map their icon text to our icon number
tCIDLib::TCard4 TWundergroundSDriver::c4MapIcon(const TString& strCondText) const
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kWundergroundS::c4IconCnt; c4Index++)
    {
        const tWundergroundS::TIconMap& Cur = kWundergroundS::aMap[c4Index];

        if (strCondText == Cur.pszIconName)
            return Cur.c4IconNum;
    }

    // Return the unknown value (the 0th entry)
    return kWundergroundS::aMap[0].c4IconNum;
}


//
//  ProcessResponse() calls this when it sees the astronomy (moonphase) chunk.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessAstronomy(const TJSONObject& jprsnPar)
{
    //
    //  Get the percent illuminated value out and convert to a WC style moon
    //  phase value. We set a moon icon value from the moon cycle age number.
    //  If not found or invalid, set it to 0, which is an 'unknown' type of
    //  icon.
    //
    //  We will also use this to create a moon phase description based on how
    //  many days.
    //
    try
    {
        const TString& strVal = jprsnPar.strValByName(m_strAttr_MoonAge);
        tCIDLib::TCard4 c4Val = strVal.c4Val() + 1;
        if (c4Val > 30)
            c4Val = 0;

        bStoreCardFld(m_c4FldIdCurMoonPhIcon, c4Val, kCIDLib::True);

        // Create the descriptive version
        if (!c4Val)
        {
            bStoreStringFld(m_c4FldIdCurMoonPhase, TString::strEmpty(), kCIDLib::True);
        }
         else
        {
            // They don't line up integrally, so we use floats
            const tCIDLib::TFloat8 f8CurVal = tCIDLib::TFloat8(c4Val - 1);
            tCIDLib::TFloat8 f8PerPhase = (29.5305882 / 8.0);

            const tCIDLib::TCh* pszPhase;
            if (f8CurVal < f8PerPhase)
                pszPhase = L"New";
            else if (f8CurVal < (f8PerPhase * 2))
                pszPhase = L"Waxing Cresent";
            else if (f8CurVal < (f8PerPhase * 3))
                pszPhase = L"First Quarter";
            else if (f8CurVal < (f8PerPhase * 4))
                pszPhase = L"Waxing Gibbous";
            else if (f8CurVal < (f8PerPhase * 5))
                pszPhase = L"Full";
            else if (f8CurVal < (f8PerPhase * 6))
                pszPhase = L"Waning Gibbous";
            else if (f8CurVal < (f8PerPhase * 7))
                pszPhase = L"Third Quarter";
            else
                pszPhase = L"Waning Cresent";

            bStoreStringFld(m_c4FldIdCurMoonPhase, pszPhase, kCIDLib::True);
        }
    }

    catch(...)
    {
        bStoreCardFld(m_c4FldIdCurMoonPhIcon, 0, kCIDLib::True);
        bStoreStringFld(m_c4FldIdCurMoonPhase, TString::strEmpty(), kCIDLib::True);
    }

    // Get the sunrise element
    try
    {
        ProcessSunTime(*jprsnPar.pjprsnFindObj(m_strElem_Sunrise), m_c4FldIdSunrise);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(m_c4FldIdSunrise, kCIDLib::True);
    }

    // And the sunset element
    try
    {
        ProcessSunTime(*jprsnPar.pjprsnFindObj(m_strElem_Sunset), m_c4FldIdSunset);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(m_c4FldIdSunset, kCIDLib::True);
    }
}


//
//  Pull out an attribute from the passed node and store the value in a card
//  field, optionally checking for range.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessCardFld(const  TJSONObject&    jprsnPar
                                    , const TString&        strAttr
                                    , const tCIDLib::TCard4 c4FldId
                                    , const tCIDLib::TCard4 c4Min
                                    , const tCIDLib::TCard4 c4Max)
{
    try
    {
        // We have to get a copy in this case
        TString strVal = jprsnPar.strValByName(strAttr);

        // Watch for any special cases, else convert the value
        if (bIsNAValue(strVal))
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            // Special case for the humidity field. Remove trailng percent
            if (strVal.chLast() == kCIDLib::chPercentSign)
                strVal.DeleteLast();

            // The value is sometimes a float, but we are storing as card
            tCIDLib::TCard4 c4Val = tCIDLib::TCard4(strVal.f8Val());
            if ((c4Val < c4Min) || (c4Val > c4Max))
                SetFieldErr(c4FldId, kCIDLib::True);
            else
                bStoreCardFld(c4FldId, c4Val, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  ProcessResponse() calls this when it sees the current conditions chunk.
//  Everything we care about here is an attribute of the passed current
//  observation node.
//
tCIDLib::TVoid TWundergroundSDriver::ProcessCC(const TJSONObject& jprsnPar)
{
    ProcessIconFld(jprsnPar, m_strAttr_Icon, m_c4FldIdCurIcon);
    ProcessTimeFld(jprsnPar, m_strAttr_ObsTime, m_c4FldIdCurAsOf, TTime::fcolRFC822());
    ProcessStringFld(jprsnPar, m_strAttr_Weather, m_c4FldIdCurCondText);
    ProcessCardFld(jprsnPar, m_strAttr_Humidity, m_c4FldIdCurHumidity, 0, 100);
    ProcessCardFld(jprsnPar, m_strAttr_UV, m_c4FldIdCurUV);

    ProcessCardFld(jprsnPar, m_strAttr_WindDegrees, m_c4FldIdCurWindDeg, 0, 360);
    ProcessStringFld(jprsnPar, m_strAttr_WindDir, m_c4FldIdCurWindDir);

    if (m_bShowMetric)
    {
        ProcessFloatFld(jprsnPar, m_strAttr_BaroC, m_c4FldIdCurBaro);
        ProcessIntFld(jprsnPar, m_strAttr_DewPntC, m_c4FldIdCurDewPoint);
        ProcessTempFld(jprsnPar, m_strAttr_FeelsLikeC, m_c4FldIdCurFeelsLike);
        ProcessTempFld(jprsnPar, m_strAttr_TempC, m_c4FldIdCurTemp);
        ProcessFloatFld(jprsnPar, m_strAttr_PrecipMetric, m_c4FldIdCurPrecip);
        ProcessFloatFld(jprsnPar, m_strAttr_VisibilityK, m_c4FldIdCurVisibility);
        ProcessCardFld(jprsnPar, m_strAttr_WindGustK, m_c4FldIdCurWindGust);
        ProcessCardFld(jprsnPar, m_strAttr_WindSpeedK, m_c4FldIdCurWindSpeed);
    }
     else
    {
        ProcessFloatFld(jprsnPar, m_strAttr_BaroF, m_c4FldIdCurBaro);
        ProcessIntFld(jprsnPar, m_strAttr_DewPntF, m_c4FldIdCurDewPoint);
        ProcessTempFld(jprsnPar, m_strAttr_FeelsLikeF, m_c4FldIdCurFeelsLike);
        ProcessTempFld(jprsnPar, m_strAttr_TempF, m_c4FldIdCurTemp);
        ProcessFloatFld(jprsnPar, m_strAttr_Precip, m_c4FldIdCurPrecip);
        ProcessFloatFld(jprsnPar, m_strAttr_VisibilityM, m_c4FldIdCurVisibility);
        ProcessCardFld(jprsnPar, m_strAttr_WindGustM, m_c4FldIdCurWindGust);
        ProcessCardFld(jprsnPar, m_strAttr_WindSpeedM, m_c4FldIdCurWindSpeed);
    }
}



//
//  This one stores a string also, but it's one of the time stamp fields,
//  which the user can tell us to store in different forms. The values all
//  come in separate attributes, so this one doesn't take an attr name.
//  We take a node that contains such a field and pull the values out
//  ourself.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessFCDate(const   TJSONObject&    jprsnDate
                                    , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strDay = jprsnDate.strValByName(m_strAttr_Day);
        const TString& strHour = jprsnDate.strValByName(m_strAttr_Hour);
        const TString& strMin = jprsnDate.strValByName(m_strAttr_Min);
        const TString& strMonth = jprsnDate.strValByName(m_strAttr_Month);
        const TString& strYear = jprsnDate.strValByName(m_strAttr_Year);

        //
        //  Convert all of the values, which are all unsigned values. If any
        //  of them fail we put the field into error state.
        //
        //  The month needs to be reduced to a zero based value. The hour
        //  and minute already are.
        //
        const tCIDLib::TCard4 c4Day = strDay.c4Val(tCIDLib::ERadices::Dec);
        const tCIDLib::TCard4 c4Hour = strHour.c4Val(tCIDLib::ERadices::Dec);
        const tCIDLib::TCard4 c4Min = strMin.c4Val(tCIDLib::ERadices::Dec);
        const tCIDLib::TCard4 c4Month = strMonth.c4Val(tCIDLib::ERadices::Dec) - 1;
        const tCIDLib::TCard4 c4Year = strYear.c4Val(tCIDLib::ERadices::Dec);

        //
        //  Now set up a time object from these details. If any of the values were
        //  bad, this will throw.
        //
        m_tmTmp.FromDetails
        (
            c4Year, tCIDLib::EMonths(c4Month), c4Day, c4Hour, c4Min
        );

        bStoreTimeFld(c4FldId, m_tmTmp.enctTime(), kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  Pull out an attribute from the passed node and store the value in a float
//  field.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessFloatFld(  const   TJSONObject&    jprsnPar
                                        , const TString&        strAttr
                                        , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttr);
        if (bIsNAValue(strVal))
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            const tCIDLib::TFloat8 f8Val = strVal.f8Val();
            bStoreFloatFld(c4FldId, f8Val, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}



// ProcessResponse() calls this when it sees the forecast chunk
tCIDLib::TVoid
TWundergroundSDriver::ProcessForecast(const TJSONObject& jprsnPar)
{
    const tCIDLib::TCard4 c4Count = jprsnPar.c4ValCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TJSONObject& jprsnCur = *jprsnPar.pjprsnObjAt(c4Index);

        // In both these cases we only care about the contained array
        if (jprsnCur.strName() == m_strElem_TextForecast)
        {
            try
            {
                const TJSONArray& jprsnArray = *jprsnCur.pjprsnFindArray(m_strElem_ForecastDay);
                ProcessTextForecast(jprsnArray);
            }

            catch(TError& errToCatch)
            {
                if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
                {
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }
                    facWundergroundS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kWundgrErrs::errcData_NotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TString(L"text forecast day array")
                    );
                }
            }
        }
         else if (jprsnCur.strName() == m_strElem_SimpleForecast)
        {
            try
            {
                const TJSONArray& jprsnArray = *jprsnCur.pjprsnFindArray(m_strElem_ForecastDay);
                ProcessSimpleForecast(jprsnArray);
            }

            catch(TError& errToCatch)
            {
                if (eVerboseLevel() > tCQCKit::EVerboseLvls::Medium)
                {
                    if (!errToCatch.bLogged())
                    {
                        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                        TModule::LogEventObj(errToCatch);
                    }

                    facWundergroundS().LogMsg
                    (
                        CID_FILE
                        , CID_LINE
                        , kWundgrErrs::errcData_NotFound
                        , tCIDLib::ESeverities::Failed
                        , tCIDLib::EErrClasses::NotFound
                        , TString(L"simple forecast day array")
                    );
                }
            }
        }
    }
}


//
//  Pull out an attribute from the passed node and map the value to one of the
//  standard icon numbers and store it in a card field.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessIconFld(const  TJSONObject&      jprsnPar
                                    , const TString&        strAttr
                                    , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttr);
        const tCIDLib::TCard4 c4Icon = c4MapIcon(strVal);
        bStoreCardFld(c4FldId, c4Icon, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  Pull out an attribute from the passed node and store the value in an int
//  field.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessIntFld(const   TJSONObject&      jprsnPar
                                    , const TString&        strAttr
                                    , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttr);
        if (bIsNAValue(strVal))
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            // We might get it as a float, so convert that and cast to int
            tCIDLib::TInt4 i4Val = tCIDLib::TInt4(strVal.f8Val());
            bStoreIntFld(c4FldId, i4Val, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


// Wind speeds are only available in MPH, so we have to adjust if showing metric
tCIDLib::TVoid
TWundergroundSDriver::ProcessMetricWind(const   TJSONObject&      jprsnPar
                                        , const TString&        strAttr
                                        , const tCIDLib::TCard4 c4FldIds)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttr);

        // Watch for any special cases, else convert the value
        if (bIsNAValue(strVal))
        {
            SetFieldErr(c4FldIds, kCIDLib::True);
        }
         else
        {
            tCIDLib::TCard4 c4Val = tCIDLib::TCard4(strVal.f8Val() * 1.6093);
            bStoreCardFld(c4FldIds, c4Val, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldIds, kCIDLib::True);
    }
}



//
//  Given a successfully parsed JSON response. We iterate through the tree of
//  data and look for the stuff we are interested in, calling helper methods
//  with their appropriate nodes to process. ProcessResponse() is called and it
//  in turn calls other methods to handle specific branches of the response.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessResponse(const TJSONObject& jprsnRoot)
{
    const tCIDLib::TCard4 c4Count = jprsnRoot.c4ValCount();
    if (!c4Count)
    {
        facWundergroundS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kWundgrErrs::errcData_EmptyReply
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
        );
        return;
    }

    // Get the first element, which should be the response element
    {
        const TJSONObject& jprsnResp = *jprsnRoot.pjprsnObjAt(0);

        // See if it has an error child node
        if (jprsnResp.pjprsnFindVal(m_strElem_Error, kCIDLib::False))
        {
            // Get it as an object and pull the info out
            const TJSONObject& jprsnErr = *jprsnResp.pjprsnFindObj(m_strElem_Error);

            const TJSONValue* pjprsnType = jprsnErr.pjprsnFindVal(m_strAttr_Type, kCIDLib::False);
            const TJSONValue* pjprsnDescr = jprsnErr.pjprsnFindVal(m_strAttr_Description, kCIDLib::False);
            facWundergroundS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kWundgrErrs::errcData_ErrorReply
                , pjprsnType ? pjprsnType->strValue() : L"No error description available"
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Nak
                , pjprsnType ? pjprsnType->strValue() : TString(L"[Unknown]")
            );
        }
    }

    // Iterate the child elements and process the ones we care about
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TJSONObject& jprsnCur = *jprsnRoot.pjprsnObjAt(c4Index);

        if (jprsnCur.strName() == m_strElem_CurObservation)
            ProcessCC(jprsnCur);
        else if (jprsnCur.strName() == m_strElem_Forecast)
            ProcessForecast(jprsnCur);
        else if (jprsnCur.strName() == m_strElem_MoonPhase)
            ProcessAstronomy(jprsnCur);
    }
}


//
//  ProcessForecast() calls this when it sees the simple forecast chunk. He
//  passes us the array of day elements.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessSimpleForecast(const TJSONArray& jprsnPar)
{
    const tCIDLib::TCard4 c4PerDay = tCIDLib::TCard4(tWundergroundS::EFDay_FldsPer);

    for (tCIDLib::TCard4 c4DayInd = 0; c4DayInd < kWundergroundS::c4ForecastDays; c4DayInd++)
    {
        try
        {
            const TJSONObject& jprsnCur = *jprsnPar.pjprsnObjAt(c4DayInd);

            //
            //  Get the date element and build up the time stamp for this day. The
            //  user can select the order of day and month.
            //
            const TJSONObject* pjprsnDate = jprsnCur.pjprsnFindObj(m_strElem_Date, kCIDLib::False);
            if (pjprsnDate)
                ProcessFCDate(*pjprsnDate, m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_Stamp]);
            else
                SetFieldErr(m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_Stamp], kCIDLib::True);

            // Get the high temp element
            const TJSONObject* pjprsnHigh = jprsnCur.pjprsnFindObj(m_strElem_High, kCIDLib::False);
            if (pjprsnHigh)
            {
                ProcessTempFld
                (
                    *pjprsnHigh
                    , m_bShowMetric ? m_strAttr_CScale : m_strAttr_FScale
                    , m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_High]
                );
            }
             else
            {
                SetFieldErr(m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_High], kCIDLib::True);
            }

            // And the low temp element
            const TJSONObject* pjprsnLow = jprsnCur.pjprsnFindObj(m_strElem_Low, kCIDLib::False);
            if (pjprsnLow)
            {
                ProcessTempFld
                (
                    *pjprsnLow
                    , m_bShowMetric ? m_strAttr_CScale : m_strAttr_FScale
                    , m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_Low]
                );
            }
             else
            {
                SetFieldErr(m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_Low], kCIDLib::True);
            }

            // Store the condition text
            ProcessStringFld
            (
                jprsnCur
                , m_strAttr_Conditions
                , m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_CondText]
            );

            // And the icon
            ProcessIconFld
            (
                jprsnCur
                , m_strAttr_Icon
                , m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_Icon]
            );

            // And the percipitation probability
            ProcessCardFld
            (
                jprsnCur
                , m_strAttr_POP
                , m_aac4ForecastIds[c4DayInd][tWundergroundS::EFDay_POP]
                , 0
                , 100
            );
        }

        catch(TError& errToCatch)
        {
            LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);

            // Set all the fields for this day into error state
            for (tCIDLib::TCard4 c4FldInd = 0; c4FldInd < c4PerDay; c4FldInd++)
                SetFieldErr(m_aac4ForecastIds[c4DayInd][c4FldInd], kCIDLib::True);
        }
    }
}


//
//  Pull out an attribute from the passed node and store the value in a string
//  field.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessStringFld( const   TJSONObject&    jprsnPar
                                        , const TString&        strAttr
                                        , const tCIDLib::TCard4 c4FldId)
{
    const TJSONValue* pjprsnVal = jprsnPar.pjprsnFindVal(strAttr);
    if (pjprsnVal)
        bStoreStringFld(c4FldId, pjprsnVal->strValue(), kCIDLib::True);
    else
        bStoreStringFld(c4FldId, TString::strEmpty(), kCIDLib::True);
}



// Processes a sunrise/sunset time, which is stored as hour/minute values
tCIDLib::TVoid
TWundergroundSDriver::ProcessSunTime(const  TJSONObject&    jprsnPar
                                    , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strHour = jprsnPar.strValByName(m_strAttr_Hour);
        const TString& strMin = jprsnPar.strValByName(m_strAttr_Minute);

        tCIDLib::TCard4 c4SunHour;
        tCIDLib::TCard4 c4SunMin;
        if (strHour.bToCard4(c4SunHour, tCIDLib::ERadices::Dec)
        &&  strMin.bToCard4(c4SunMin, tCIDLib::ERadices::Dec))
        {
            // Set up a time object for today, and get the details out
            m_tmTmp.SetTo(tCIDLib::ESpecialTimes::CurrentDate);

            tCIDLib::TCard4        c4Year;
            tCIDLib::EMonths       eMonth;
            tCIDLib::TCard4        c4Day;
            tCIDLib::TCard4        c4Hour;
            tCIDLib::TCard4        c4Minute;
            tCIDLib::TCard4        c4Second;
            tCIDLib::TCard4        c4Millis;
            tCIDLib::TEncodedTime  enctStamp;

            m_tmTmp.eExpandDetails
            (
                c4Year
                , eMonth
                , c4Day
                , c4Hour
                , c4Minute
                , c4Second
                , c4Millis
                , enctStamp
            );

            //
            //  Update the hour and minute and put them back, truncating back to
            //  the hour, i.e. drop the seconds.
            //
            c4Hour = c4SunHour;
            c4Minute = c4SunMin;
            m_tmTmp.FromDetails(c4Year, eMonth, c4Day, c4Hour, c4Minute);

            bStoreTimeFld(c4FldId, m_tmTmp.enctTime(), kCIDLib::True);
        }
         else
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}



//
//  Pull out an attribute from the passed node and store the value in a
//  temp field, which we will check against the legal temp range. If not
//  valid we set it to the low temp value.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessTempFld(const  TJSONObject&    jprsnPar
                                    , const TString&        strAttr
                                    , const tCIDLib::TCard4 c4FldId)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttr);

        // We get them sometimes as floats, but store them as ints
        tCIDLib::TInt4 i4Val;
        if (bIsNAValue(strVal))
            i4Val = m_i4LowTempRange;
        else
            i4Val = tCIDLib::TInt4(strVal.f8Val());

        bStoreIntFld(c4FldId, i4Val, kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  ProcessForecast() calls this when it sees the text forecast chunk. He passes
//  us the array of day elements. Actually not day elements, but forecast info
//  for today, tonight, tomorrow, and tomorrow night. We have to deal wtih either
//  metric or imperial units and the metric may not be there, so we have a special
//  helper to deal with that.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessOneTFC(const   TJSONObject&    jprsnPar
                                    , const tCIDLib::TCard4 c4FldId)
{
    const TJSONValue* pjprsnVal = 0;

    // If they are in metric mode and the metric one is there, take that
    if (m_bShowMetric)
        pjprsnVal = jprsnPar.pjprsnFindVal(m_strAttr_FCTextMetric);

    // Not metric or not available, so loo for the other one
    if (!pjprsnVal)
        pjprsnVal = jprsnPar.pjprsnFindVal(m_strAttr_FCText);

    if (pjprsnVal)
        bStoreStringFld(c4FldId, pjprsnVal->strValue(), kCIDLib::True);
    else
        bStoreStringFld(c4FldId, TString::strEmpty(), kCIDLib::True);
}


tCIDLib::TVoid
TWundergroundSDriver::ProcessTextForecast(const TJSONArray& jprsnPar)
{
    //
    //  For this one, if they are set up for C temps, first see if we can find
    //  a metric one. If not, go with the regular one.
    //
    const TJSONObject* pjprsnElem = jprsnPar.pjprsnObjAt(0, kCIDLib::False);
    if (pjprsnElem)
        ProcessOneTFC(*pjprsnElem, m_c4FldIdFCCurrent);
    else
        SetFieldErr(m_c4FldIdFCCurrent, kCIDLib::True);

    pjprsnElem = jprsnPar.pjprsnObjAt(1, kCIDLib::False);
    if (pjprsnElem)
        ProcessOneTFC(*pjprsnElem, m_c4FldIdFC12Hour);
    else
        SetFieldErr(m_c4FldIdFC12Hour, kCIDLib::True);

    pjprsnElem = jprsnPar.pjprsnObjAt(2, kCIDLib::False);
    if (pjprsnElem)
        ProcessOneTFC(*pjprsnElem, m_c4FldIdFC24Hour);
    else
        SetFieldErr(m_c4FldIdFC24Hour, kCIDLib::True);

    pjprsnElem = jprsnPar.pjprsnObjAt(3, kCIDLib::False);
    if (pjprsnElem)
        ProcessOneTFC(*pjprsnElem, m_c4FldIdFC36Hour);
    else
        SetFieldErr(m_c4FldIdFC36Hour, kCIDLib::True);
}



//
//  Processes a time stamp field. The caller provides us with the time component
//  parsing information.
//
tCIDLib::TVoid
TWundergroundSDriver::ProcessTimeFld(const  TJSONObject&        jprsnPar
                                    , const TString&            strAttrName
                                    , const tCIDLib::TCard4     c4FldId
                                    , const TTimeCompList&      fcolFmt)
{
    try
    {
        const TString& strVal = jprsnPar.strValByName(strAttrName);

        m_tmTmp.ParseFromText
        (
            strVal
            , fcolFmt
            , kCIDLib::chNull
            , kCIDLib::chColon
            , kCIDLib::chNull
        );
        bStoreTimeFld(c4FldId, m_tmTmp.enctTime(), kCIDLib::True);
    }

    catch(TError& errToCatch)
    {
        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium, CID_FILE, CID_LINE);
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  We split this out from the init just to keep the main parts of the driver
//  cleaner above. We just set up and register all our fields.
//
tCIDLib::TVoid
TWundergroundSDriver::RegisterFields()
{
    // Set up some field limits we'll use more than once
    const TString strIconLim(L"Range: 0, 48");
    const TString strPercentLim(L"Range: 0, 100");

    TString strTempLims(L"Range:");
    strTempLims.AppendFormatted(m_i4LowTempRange);
    strTempLims.Append(kCIDLib::chComma);
    strTempLims.AppendFormatted(m_i4HighTempRange);

    // We have to add a prefix to V2 complaint fields if V2 mode
    TString strPref;
    if (m_bV2Mode)
        strPref = L"WEATH#";


    const tCIDLib::TCard4 c4FldCnt
    (
        32 + (kWundergroundS::c4ForecastDays * tWundergroundS::EFDay_FldsPer)
    );
    TVector<TCQCFldDef> colFlds(c4FldCnt);

    colFlds.objAdd
    (
        TCQCFldDef(strPref + L"CurAsOf", tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(strPref + L"CurBaro", tCQCKit::EFldTypes::Float, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            strPref + L"CurCondText"
            , tCQCKit::EFldTypes::String
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::CurWeather
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurDewPoint", tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurFeelsLike", tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            strPref + L"CurHumidity"
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::CurExtHumidity
            , strPercentLim
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            strPref + L"CurIcon"
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
            , strIconLim
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurMoonPhase", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            strPref + L"CurPrecip"
            , tCQCKit::EFldTypes::Float
            , tCQCKit::EFldAccess::Read
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            L"CurMoonPhaseIcon"
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
            , L"Range: 0, 30"
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            strPref + L"CurTemp"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::CurExtTemp
            , strTempLims
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(strPref + L"FCDayCount", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
    );


    colFlds.objAdd
    (
        TCQCFldDef(L"CurUV", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurVisibility", tCQCKit::EFldTypes::Float, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            L"CurWindDegrees"
            , tCQCKit::EFldTypes::Card
            , tCQCKit::EFldAccess::Read
            , L"Range: 0, 360"
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurWindDir", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurWindGust", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurWindSpeed", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(strPref + L"FCCurrent", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"FC12Hour", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"FC24Hour", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"FC36Hour", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"LocCode", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(strPref + L"LocName", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"Sunrise", tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"Sunset", tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read)
    );


    //
    //  Do the fields for the forecast days. The fields for each day MUST be in
    //  this order, since we use an enum to access them relative to the base field
    //  id for a day.
    //
    TString strName;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < kWundergroundS::c4ForecastDays; c4Index++)
    {
        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondText");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
        );

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"High");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read, strTempLims)
        );

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Icon");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strIconLim)
        );

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Low");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read, strTempLims)
        );

        // If V2, fix an old spelling mistake
        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        if (m_bV2Mode)
            strName.Append(L"PrecipProb");
        else
            strName.Append(L"PercipProb");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strPercentLim)
        );

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Stamp");
        colFlds.objAdd(TCQCFldDef(strName, tCQCKit::EFldTypes::Time, tCQCKit::EFldAccess::Read));
    }

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the field ids for later use
    m_c4FldIdCurAsOf = pflddFind(strPref + L"CurAsOf", kCIDLib::True)->c4Id();
    m_c4FldIdCurBaro = pflddFind(strPref + L"CurBaro", kCIDLib::True)->c4Id();
    m_c4FldIdCurCondText = pflddFind(strPref + L"CurCondText", kCIDLib::True)->c4Id();
    m_c4FldIdCurDewPoint = pflddFind(L"CurDewPoint", kCIDLib::True)->c4Id();
    m_c4FldIdCurFeelsLike = pflddFind(L"CurFeelsLike", kCIDLib::True)->c4Id();
    m_c4FldIdCurHumidity = pflddFind(strPref + L"CurHumidity", kCIDLib::True)->c4Id();
    m_c4FldIdCurIcon = pflddFind(strPref + L"CurIcon", kCIDLib::True)->c4Id();
    m_c4FldIdCurMoonPhase = pflddFind(L"CurMoonPhase", kCIDLib::True)->c4Id();
    m_c4FldIdCurMoonPhIcon = pflddFind(L"CurMoonPhaseIcon", kCIDLib::True)->c4Id();
    m_c4FldIdCurPrecip = pflddFind(strPref + L"CurPrecip", kCIDLib::True)->c4Id();
    m_c4FldIdCurTemp = pflddFind(strPref + L"CurTemp", kCIDLib::True)->c4Id();
    m_c4FldIdCurUV = pflddFind(L"CurUV", kCIDLib::True)->c4Id();
    m_c4FldIdCurVisibility = pflddFind(L"CurVisibility", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindDeg = pflddFind(L"CurWindDegrees", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindDir = pflddFind(L"CurWindDir", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindGust = pflddFind(L"CurWindGust", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindSpeed = pflddFind(L"CurWindSpeed", kCIDLib::True)->c4Id();
    m_c4FldIdFCCurrent = pflddFind(strPref + L"FCCurrent", kCIDLib::True)->c4Id();
    m_c4FldIdFC12Hour = pflddFind(L"FC12Hour", kCIDLib::True)->c4Id();
    m_c4FldIdFC24Hour = pflddFind(L"FC24Hour", kCIDLib::True)->c4Id();
    m_c4FldIdFC36Hour = pflddFind(L"FC36Hour", kCIDLib::True)->c4Id();
    m_c4FldIdSunrise = pflddFind(L"Sunrise", kCIDLib::True)->c4Id();
    m_c4FldIdSunset = pflddFind(L"Sunset", kCIDLib::True)->c4Id();

    for (c4Index = 0; c4Index < kWundergroundS::c4ForecastDays; c4Index++)
    {
        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondText");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_CondText]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"High");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_High] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Icon");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_Icon]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Low");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_Low]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        if (m_bV2Mode)
            strName.Append(L"PrecipProb");
        else
            strName.Append(L"PercipProb");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_POP]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = strPref;
        strName.Append(L"Day");
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Stamp");
        m_aac4ForecastIds[c4Index][tWundergroundS::EFDay_Stamp]  = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    // The day count is fixed, so write that out now by name
    bStoreCardFld(strPref + L"FCDayCount", kWundergroundS::c4ForecastDays, kCIDLib::True);
}


