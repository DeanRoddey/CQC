//
// FILE NAME: WeatherChS_DriverImpl.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/31/2004
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
#include    "WeatherChS.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TWeatherChSDriver,TCQCServerBase)


// ---------------------------------------------------------------------------
//  Local data and types
// ---------------------------------------------------------------------------
namespace WeatherChS_DriverImpl
{
    // -----------------------------------------------------------------------
    //  The number of state icons provided by the Weather Channel SDK. There
    //  icons named 0 through 47, and the value 48 is used as a 'N/A' value.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4IconCount = 48;
}



// ---------------------------------------------------------------------------
//   CLASS: TWeatherSDriver
//  PREFIX: drv
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TWeatherSDriver: Constructors and Destructor
// ---------------------------------------------------------------------------
TWeatherChSDriver::TWeatherChSDriver(const TCQCDriverObjCfg& cqcdcToLoad) :

    TCQCServerBase(cqcdcToLoad)
    , m_c4FldIdCurAsOf(kCIDLib::c4MaxCard)
    , m_c4FldIdCurBaro(kCIDLib::c4MaxCard)
    , m_c4FldIdCurBaroTrend(kCIDLib::c4MaxCard)
    , m_c4FldIdCurCondText(kCIDLib::c4MaxCard)
    , m_c4FldIdCurDewPoint(kCIDLib::c4MaxCard)
    , m_c4FldIdCurFeelsLike(kCIDLib::c4MaxCard)
    , m_c4FldIdCurHumidity(kCIDLib::c4MaxCard)
    , m_c4FldIdCurIcon(kCIDLib::c4MaxCard)
    , m_c4FldIdCurMoonPhase(kCIDLib::c4MaxCard)
    , m_c4FldIdCurMoonPhIcon(kCIDLib::c4MaxCard)
    , m_c4FldIdCurTemp(kCIDLib::c4MaxCard)
    , m_c4FldIdCurUVIndex(kCIDLib::c4MaxCard)
    , m_c4FldIdCurUVText(kCIDLib::c4MaxCard)
    , m_c4FldIdCurWindDir(kCIDLib::c4MaxCard)
    , m_c4FldIdCurWindSpeed(kCIDLib::c4MaxCard)
    , m_c4FldIdCurVisibility(kCIDLib::c4MaxCard)
    , m_c4FldIdForecastAsOf(kCIDLib::c4MaxCard)
    , m_c4FldIdSunrise(kCIDLib::c4MaxCard)
    , m_c4FldIdSunset(kCIDLib::c4MaxCard)
    , m_chUnits(L's')
    , m_eDtFmt(EDtFmts::MMDDYY)
    , m_fcolForecastIds(kWeatherChS::c4ForecastDays, tWeatherChS::TIdList(tWeatherChS::EFDayFlds::Count))
    , m_i4HighTempRange(120)
    , m_i4LowTempRange(-20)
    , m_enctNextForecast(TTime::enctNow())
    , m_enctNextPoll(TTime::enctNow())
    , m_strCmd_SetLocCode(L"SetLocCode")
{
}

TWeatherChSDriver::~TWeatherChSDriver()
{
}


// ---------------------------------------------------------------------------
//  TWeatherChSDriver: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TWeatherChSDriver::bGetCommResource(TThread&)
{
    // Just say it worked
    return kCIDLib::True;
}


tCIDLib::TBoolean TWeatherChSDriver::bWaitConfig(TThread& thrThis)
{
    return kCIDLib::True;
}


tCQCKit::ECommResults TWeatherChSDriver::eConnectToDevice(TThread&)
{
    if (!bDoUpdate(m_strPostalCode))
        return tCQCKit::ECommResults::LostConnection;
    return tCQCKit::ECommResults::Success;
}


// We override this to accept some backdoor commands
tCIDLib::TCard4
TWeatherChSDriver::c4SendCmd(const  TString&    strCmdId
                            , const TString&    strParms)
{
    tCIDLib::TCard4 c4Ret = 0;
    if (strCmdId == m_strCmd_SetLocCode)
    {
        //
        //  The parameter is a quoted comma string, with one value being
        //  the new location code and the other being a name to give to
        //  the location.
        //
        tCIDLib::TCard4     c4Err;
        TVector<TString>    colVals;
        if (!TStringTokenizer::bParseQuotedCommaList(strParms, colVals, c4Err)
        ||  (colVals.c4ElemCount() != 2))
        {
            facWeatherChS().ThrowErr
            (
                CID_FILE
                , CID_LINE
                , kWChErrs::errcCmd_BadParms
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::BadParms
                , strCmdId
            );
        }

        //
        //  Reset the time stamps and do an update, passing the new code
        //  to see if it works. If so, then we store the new one and the
        //  name.
        //
        //  We have to reset the stamps so that the update method won't
        //  just ignore us and continue to wait for the next regularly
        //  scheduled round.
        //
        //  If it fails, this will leave the time stamps set up for a
        //  retry next poll time, which is fairly short and probably not
        //  needed since we still have the old stuff stored. But it's
        //  always possible that we failed during the storage process
        //  and some new stuff got stored before we failed. So this
        //  insures that we get back into sync reasonably quickly if that
        //  happens.
        //
        //  If it fails, we set the return to 1 to indicate an error to
        //  the caller.
        //
        m_enctNextPoll = TTime::enctNow();
        m_enctNextForecast = m_enctNextPoll;
        if (bDoUpdate(colVals[0]))
        {
            // Store the postal code for later
            m_strPostalCode = colVals[0];

            // Update the postal code and name fields, locking first
            bStoreStringFld(L"LocCode", m_strPostalCode, kCIDLib::True);
            bStoreStringFld(L"LocName", colVals[1], kCIDLib::True);
        }
         else
        {
            c4Ret = 1;
        }
    }
     else
    {
        facWeatherChS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kWChErrs::errcCmd_UnknownCmdId
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Unknown
            , strCmdId
        );
    }
    return 0;
}


tCQCKit::EDrvInitRes TWeatherChSDriver::eInitializeImpl()
{
    //
    //  We will have gotten two prompt values with the key and developer id
    //  that we need to build up the queries.
    //
    const TCQCDriverObjCfg& cqcdcTmp = cqcdcThis();
    TString strMeasure;
    if (!cqcdcTmp.bFindPromptValue(L"DevId", L"Text", m_strDevId)
    ||  !cqcdcTmp.bFindPromptValue(L"LicKey", L"Text", m_strLicKey)
    ||  !cqcdcTmp.bFindPromptValue(L"Location", L"Text", m_strPostalCode)
    ||  !cqcdcTmp.bFindPromptValue(L"Measure", L"Selected", strMeasure))
    {
        facWeatherChS().LogMsg
        (
            CID_FILE
            , CID_LINE
            , L"The developer id or license key values were not received"
            , tCIDLib::ESeverities::Status
        );
        return tCQCKit::EDrvInitRes::Failed;
    }

    // Set a character member based on the measure type
    if (strMeasure == L"Metric")
        m_chUnits = L'm';
    else
        m_chUnits = L's';

    //
    //  This one is optional since it was added after the initial impl
    //  of the driver, so in currently installed drivers it will not be
    //  present, in which case we get the default set in the ctor.
    //
    m_eDtFmt = EDtFmts::MMDDYY;
    TString strDtFmt;
    if (cqcdcTmp.bFindPromptValue(L"DateFormat", L"Selected", strDtFmt))
    {
        if (strDtFmt == L"DD/MM/YY")
            m_eDtFmt = EDtFmts::DDMMYY;
    }


    //
    //  These are optional also, added in 2.4.20, to let the user indicate
    //  what range to use for the temp fields. We default them in the ctor
    //  but override them here for newer installs where they have set them.
    //
    TString strTmp;
    if (cqcdcTmp.bFindPromptValue(L"LowTemp", L"Value", strTmp))
        m_i4LowTempRange = strTmp.i4Val();

    if (cqcdcTmp.bFindPromptValue(L"HighTemp", L"Value", strTmp))
        m_i4HighTempRange = strTmp.i4Val();


    //
    //  And this one is optional, added in 3.4.21. We now support a field
    //  that holds a user provided location name. We can set this below upon
    //  creation of that field, and also any time they send us the SetLocCode
    //  command, which includes a location name.
    //
    TString strInitLocName;
    cqcdcTmp.bFindPromptValue(L"LocName", L"Text", strInitLocName);

    //
    //  Set up a limit string for the state icon fields. This allows for
    //  the 0 .. 47 values to represent the actual states and 48 to be used
    //  to represent 'N/A'.
    //
    //  And one for all the temp related fields, using the stored ranges.
    //
    const TString strIconLim(L"Range: 0, 48");

    TString strTmpLims(L"Range:");
    strTmpLims.AppendFormatted(m_i4LowTempRange);
    strTmpLims.Append(kCIDLib::chComma);
    strTmpLims.AppendFormatted(m_i4HighTempRange);

    //
    //  Set up a collection of field defs to set. Almost all of our fields
    //  are read only fields that we fill with info gotten from the data
    //  feed.
    //
    TVector<TCQCFldDef> colFlds(64);

    colFlds.objAdd
    (
        TCQCFldDef(L"CurAsOf", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurBaro", tCQCKit::EFldTypes::Float, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurBaroTrend", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            L"CurCondText"
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
        TCQCFldDef
        (
            L"CurFeelsLike", tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read, strTmpLims
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef
        (
            L"CurHumidity"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::CurExtHumidity
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurIcon", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strIconLim)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurMoonPhase", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
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
            L"CurTemp"
            , tCQCKit::EFldTypes::Int
            , tCQCKit::EFldAccess::Read
            , tCQCKit::EFldSTypes::CurExtTemp
            , strTmpLims
        )
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurUVIndex", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, L"Range: 0,18")
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurUVText", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurVisibility", tCQCKit::EFldTypes::Float, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurWindDir", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"CurWindSpeed", tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"ForecastAsOf", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"LocCode", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"LocName", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"Sunrise", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );

    colFlds.objAdd
    (
        TCQCFldDef(L"Sunset", tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
    );


    //
    //  Do the fields for the forecast days. The fields for each day MUST be in
    //  this order, since we use an enum to access them relative to the base field
    //  id for a day.
    //
    TString strName;
    tCIDLib::TCard4 c4Index;
    for (c4Index = 0; c4Index < kWeatherChS::c4ForecastDays; c4Index++)
    {
        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondTextD");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondTextN");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"High");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read, strTmpLims)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"IconD");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strIconLim)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"IconN");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read, strIconLim)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Low");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Int, tCQCKit::EFldAccess::Read, strTmpLims)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Stamp");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::String, tCQCKit::EFldAccess::Read)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"PerPercipD");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
        );

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"PerPercipN");
        colFlds.objAdd
        (
            TCQCFldDef(strName, tCQCKit::EFldTypes::Card, tCQCKit::EFldAccess::Read)
        );
    }

    // Tell our base class about our fields
    SetFields(colFlds);

    // Look up the field ids for later use
    m_c4FldIdCurAsOf = pflddFind(L"CurAsOf", kCIDLib::True)->c4Id();
    m_c4FldIdCurBaro = pflddFind(L"CurBaro", kCIDLib::True)->c4Id();
    m_c4FldIdCurBaroTrend = pflddFind(L"CurBaroTrend", kCIDLib::True)->c4Id();
    m_c4FldIdCurCondText = pflddFind(L"CurCondText", kCIDLib::True)->c4Id();
    m_c4FldIdCurDewPoint = pflddFind(L"CurDewPoint", kCIDLib::True)->c4Id();
    m_c4FldIdCurFeelsLike = pflddFind(L"CurFeelsLike", kCIDLib::True)->c4Id();
    m_c4FldIdCurHumidity = pflddFind(L"CurHumidity", kCIDLib::True)->c4Id();
    m_c4FldIdCurIcon = pflddFind(L"CurIcon", kCIDLib::True)->c4Id();
    m_c4FldIdCurMoonPhase = pflddFind(L"CurMoonPhase", kCIDLib::True)->c4Id();
    m_c4FldIdCurMoonPhIcon = pflddFind(L"CurMoonPhaseIcon", kCIDLib::True)->c4Id();
    m_c4FldIdCurTemp = pflddFind(L"CurTemp", kCIDLib::True)->c4Id();
    m_c4FldIdCurUVIndex = pflddFind(L"CurUVIndex", kCIDLib::True)->c4Id();
    m_c4FldIdCurUVText = pflddFind(L"CurUVText", kCIDLib::True)->c4Id();
    m_c4FldIdCurVisibility = pflddFind(L"CurVisibility", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindDir = pflddFind(L"CurWindDir", kCIDLib::True)->c4Id();
    m_c4FldIdCurWindSpeed = pflddFind(L"CurWindSpeed", kCIDLib::True)->c4Id();
    m_c4FldIdSunrise = pflddFind(L"Sunrise", kCIDLib::True)->c4Id();
    m_c4FldIdSunset = pflddFind(L"Sunset", kCIDLib::True)->c4Id();

    m_c4FldIdForecastAsOf = pflddFind(L"ForecastAsOf", kCIDLib::True)->c4Id();
    for (c4Index = 0; c4Index < kWeatherChS::c4ForecastDays; c4Index++)
    {
        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondTextD");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::CondTextD]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"CondTextN");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::CondTextN]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"High");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::High] = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"IconD");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::IconD]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"IconN");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::IconN]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Low");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::Low]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"Stamp");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::Stamp]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"PerPercipD");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::PerPercipD]  = pflddFind(strName, kCIDLib::True)->c4Id();

        strName = L"Day";
        strName.AppendFormatted(c4Index + 1);
        strName.Append(L"PerPercipN");
        m_fcolForecastIds[c4Index][tWeatherChS::EFDayFlds::PerPercipN]  = pflddFind(strName, kCIDLib::True)->c4Id();
    }

    //
    //  Store the location code and initial location name now since they will
    //  never change unless a client sends us a SetLocCode command. No need to
    //  field lock here since we aren't connected yet.
    //
    bStoreStringFld(L"LocCode", m_strPostalCode, kCIDLib::True);
    bStoreStringFld(L"LocName", strInitLocName, kCIDLib::True);

    //
    //  We can lower the poll and reconnect times up to 10 seconds without
    //  it being a problem at all.
    //
    SetPollTimes(10000, 10000);

    return tCQCKit::EDrvInitRes::WaitCommRes;
}


tCQCKit::ECommResults TWeatherChSDriver::ePollDevice(TThread&)
{
    if (!bDoUpdate(m_strPostalCode))
        return tCQCKit::ECommResults::LostConnection;
    return tCQCKit::ECommResults::Success;
}


tCIDLib::TVoid TWeatherChSDriver::ReleaseCommResource()
{
}


tCIDLib::TVoid TWeatherChSDriver::TerminateImpl()
{
    // Nothing to do for this one
}



// ---------------------------------------------------------------------------
//  TWeatherChSDriver: Private, non-virtual methods
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
tCIDLib::TBoolean TWeatherChSDriver::bDoUpdate(const TString& strLocCode)
{
    tCIDLib::TEncodedTime enctNow = TTime::enctNow();

    //
    //  If it's not the next poll time yet, then do nothing. Note that we
    //  don't bother to check the forecast counter unless the next poll
    //  period is up. The forecast info changes on such a slow scale that
    //  it doesn't matter if it's one poll period over or not.
    //
    if (enctNow < m_enctNextPoll)
        return kCIDLib::True;

    // See if we are going to do the forecast this time
    const tCIDLib::TBoolean bDoForecast = (enctNow >= m_enctNextForecast);

    try
    {
        // We just want tags and character data
        const tCIDXML::EParseFlags eFlags = tCIDXML::EParseFlags::TagsNText;

        // Build up the URL
        TString strURL(L"http://xoap.weather.com/weather/local/");
        strURL.Append(strLocCode);
        strURL.Append(L"?cc=*&prod=xoap&link=xoap&par=");
        strURL.Append(m_strDevId);
        strURL.Append(L"&key=");
        strURL.Append(m_strLicKey);
        strURL.Append(L"&unit=");
        strURL.Append(m_chUnits);

        // If doing forecasts, ask for days of that
        if (bDoForecast)
        {
            strURL.Append(L"&dayf=");
            strURL.AppendFormatted(kWeatherChS::c4ForecastDays);
        }

        //
        //  Set up a URL entity source for the URL. This will let the parser
        //  suck in the content from the URL for us.
        //
        tCIDXML::TEntitySrcRef esrWC(new TURLEntitySrc(strURL));

        if (!m_xtprsWC.bParseRootEntity(esrWC, tCIDXML::EParseOpts::None, eFlags))
        {
            // We can't do much but log the first error return failure
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                const TXMLTreeParser::TErrInfo& erriFirst= m_xtprsWC.erriFirst();
                facWeatherChS().LogMsg
                (
                    erriFirst.strSystemId()
                    , erriFirst.c4Line()
                    , kWChErrs::errcXML_ParseErr
                    , erriFirst.strText()
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                );
            }
            return kCIDLib::False;
        }

        //
        //  Get the document element out so we can start navigating. Note that
        //  we don't build in any ordering assumptions of child elements since
        //  they aren't guaranteed by the DTD. So we look up nodes by name.
        //
        const TXMLTreeElement& xtnodeRoot = m_xtprsWC.xtdocThis().xtnodeRoot();

        //
        //  First check to see if we got an error.
        //
        tCIDLib::TCard4 c4ElemInd;
        if (xtnodeRoot.strQName() == L"error")
        {
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                // There should be an 'err' node inside this
                const TXMLTreeElement& xtnodeErr = xtnodeRoot.xtnodeFindElement
                (
                    L"err", 0, c4ElemInd
                );
                const TString& strErrCode = xtnodeErr.xtattrNamed(L"type").strValue();
                const TString& strErrText = xtnodeErr.xtnodeChildAtAsText(0).strText();

                facWeatherChS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWChErrs::errcWC_Error
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , strErrCode
                    , strErrText
                );
            }

            // Update the time stamps, so that we try again in a minute
            enctNow = TTime::enctNow();
            m_enctNextPoll = enctNow + kCIDLib::enctOneMinute;
            if (bDoForecast)
                m_enctNextForecast = m_enctNextPoll;

            return kCIDLib::False;
        }
         else if (xtnodeRoot.strQName() == L"weather")
        {
            // Process the current conditions node
            ProcessCC(xtnodeRoot.xtnodeFindElement(L"cc", 0, c4ElemInd));

            // If doing the forecast, then process that one
            if (bDoForecast)
                ProcessForecast(xtnodeRoot.xtnodeFindElement(L"dayf", 0, c4ElemInd));

            //
            //  Update the time stamps for a regularly scheduled update since
            //  we got a good one this time around.
            //
            enctNow = TTime::enctNow();
            m_enctNextPoll = enctNow + (kCIDLib::enctOneMinute * 30);
            if (bDoForecast)
                m_enctNextForecast = enctNow + (kCIDLib::enctOneHour * 2);
        }
         else
        {
            // We don't know what it is
            if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
            {
                facWeatherChS().LogMsg
                (
                    CID_FILE
                    , CID_LINE
                    , kWChErrs::errcXML_UnknownRoot
                    , tCIDLib::ESeverities::Failed
                    , tCIDLib::EErrClasses::Format
                    , xtnodeRoot.strQName()
                );
            }
            return kCIDLib::False;
        }
    }

    catch(const TError& errToCatch)
    {
        //
        //  Update the time stamps to try again in a minute to see if we
        //  can get connected again.
        //
        enctNow = TTime::enctNow();
        m_enctNextPoll = enctNow + kCIDLib::enctOneMinute;
        m_enctNextForecast = enctNow;

        LogError(errToCatch, tCQCKit::EVerboseLvls::Medium);
        if (eVerboseLevel() >= tCQCKit::EVerboseLvls::Medium)
        {
            facWeatherChS().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kWChErrs::errcXML_ParseFailed
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::AppStatus
            );
        }
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Process barometer info
tCIDLib::TVoid
TWeatherChSDriver::ProcessBar(const TXMLTreeElement& xtnodeBar)
{
    const tCIDLib::TCard4 c4ChildCount = xtnodeBar.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeBar.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        // If it's one we care about, store it
        {
            if (xtnodeElem.strQName() == L"r")
                StoreFloatFld(xtnodeElem, m_c4FldIdCurBaro);
            else if (xtnodeElem.strQName() == L"d")
                StoreStringFld(xtnodeElem, m_c4FldIdCurBaroTrend);
        }
    }
}


// Process current conditions
tCIDLib::TVoid TWeatherChSDriver::ProcessCC(const TXMLTreeElement& xtnodeCC)
{
    const tCIDLib::TCard4 c4ChildCount = xtnodeCC.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeCC.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"bar")
            ProcessBar(xtnodeElem);
        else if (xtnodeElem.strQName() == L"dewp")
            StoreIntFld(xtnodeElem, m_c4FldIdCurDewPoint);
        else if (xtnodeElem.strQName() == L"flik")
            StoreTempFld(xtnodeElem, m_c4FldIdCurFeelsLike);
        else if (xtnodeElem.strQName() == L"hmid")
            StoreIntFld(xtnodeElem, m_c4FldIdCurHumidity);
        else if (xtnodeElem.strQName() == L"icon")
            StoreIconFld(xtnodeElem, m_c4FldIdCurIcon);
        else if (xtnodeElem.strQName() == L"lsup")
            StoreLSUPFld(xtnodeElem, m_c4FldIdCurAsOf);
        else if (xtnodeElem.strQName() == L"t")
            StoreStringFld(xtnodeElem, m_c4FldIdCurCondText);
        else if (xtnodeElem.strQName() == L"tmp")
            StoreTempFld(xtnodeElem, m_c4FldIdCurTemp);
        else if (xtnodeElem.strQName() == L"uv")
            ProcessUV(xtnodeElem);
        else if (xtnodeElem.strQName() == L"vis")
            StoreFloatFld(xtnodeElem, m_c4FldIdCurVisibility);
        else if (xtnodeElem.strQName() == L"wind")
            ProcessWind(xtnodeElem);
        else if (xtnodeElem.strQName() == L"moon")
            ProcessMoon(xtnodeElem);
    }
}


//
//  Process a day's worth of forecast info. This is called from the main
//  forecast loop below, which already has a field lock.
//
tCIDLib::TVoid
TWeatherChSDriver::ProcessDay(const TXMLTreeElement& xtnodeDay)
{
    // Get the d= attribute, which provides the day index
    const tCIDLib::TCard4 c4DayInd = xtnodeDay.xtattrNamed(L"d").c4ValueAs();

    // Get the time/date string for this one and store that
    TString strStamp(xtnodeDay.xtattrNamed(L"t").strValue());
    strStamp.Append(L", ");
    strStamp.Append(xtnodeDay.xtattrNamed(L"dt").strValue());
    bStoreStringFld
    (
        m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::Stamp]
        , strStamp
        , kCIDLib::True
    );

    // And now pull out the values from this day and it's parts
    tCIDLib::TCard4 c4ElemInd;
    const tCIDLib::TCard4 c4ChildCount = xtnodeDay.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeDay.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"hi")
        {
            StoreTempFld
            (
                xtnodeElem, m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::High]
            );
        }
         else if (xtnodeElem.strQName() == L"low")
        {
            StoreTempFld
            (
                xtnodeElem, m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::Low]
            );
        }
         else if ((c4DayInd == 0)
              &&  ((xtnodeElem.strQName() == L"sunr")
              ||   (xtnodeElem.strQName() == L"suns")))
        {
            //
            //  It's sunrise/sunset info. If this his day 0, then it's for
            //  today, so pull them out and save them.
            //
            if (xtnodeElem.strQName() == L"sunr")
                StoreStringFld(xtnodeElem, m_c4FldIdSunrise);
            else
                StoreStringFld(xtnodeElem, m_c4FldIdSunset);
        }
         else if (xtnodeElem.strQName() == L"part")
        {
            // Get the p= attr to know if it's the day or night part
            const TString& strPart = xtnodeElem.xtattrNamed(L"p").strValue();

            if (strPart == L"d")
            {
                StoreStringFld
                (
                    xtnodeElem.xtnodeFindElement(L"t", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::CondTextD]
                );

                StoreIconFld
                (
                    xtnodeElem.xtnodeFindElement(L"icon", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::IconD]
                );

                StoreCardFld
                (
                    xtnodeElem.xtnodeFindElement(L"ppcp", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::PerPercipD]
                );
            }
             else
            {
                StoreStringFld
                (
                    xtnodeElem.xtnodeFindElement(L"t", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::CondTextN]
                );

                StoreIconFld
                (
                    xtnodeElem.xtnodeFindElement(L"icon", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::IconN]
                );

                StoreCardFld
                (
                    xtnodeElem.xtnodeFindElement(L"ppcp", 0, c4ElemInd)
                    , m_fcolForecastIds[c4DayInd][tWeatherChS::EFDayFlds::PerPercipN]
                );
            }
        }
    }
}


// Process the days of forecast info
tCIDLib::TVoid
TWeatherChSDriver::ProcessForecast(const TXMLTreeElement& xtnodeFC)
{
    // Keep up with how many days we see
    tCIDLib::TCard4 c4SawDays = 0;

    const tCIDLib::TCard4 c4ChildCount = xtnodeFC.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeFC.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"day")
        {
            c4SawDays++;
            ProcessDay(xtnodeElem);
        }
         else if (xtnodeElem.strQName() == L"lsup")
        {
            StoreLSUPFld(xtnodeElem, m_c4FldIdForecastAsOf);
        }
    }

    // Make sure we got the number that we asked for days for
    if (c4SawDays != kWeatherChS::c4ForecastDays)
    {
        facWeatherChS().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kWChErrs::errcXML_BadForecastDays
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(kWeatherChS::c4ForecastDays)
            , TCardinal(c4ChildCount)
        );
    }
}


// Process a moon info block (under current conditions)
tCIDLib::TVoid
TWeatherChSDriver::ProcessMoon(const TXMLTreeElement& xtnodeMoon)
{
    const tCIDLib::TCard4 c4ChildCount = xtnodeMoon.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeMoon.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"t")
        {
            // It's the phase info
            StoreStringFld(xtnodeElem, m_c4FldIdCurMoonPhase);
        }
         else if (xtnodeElem.strQName() == L"icon")
        {
            StoreCardFld(xtnodeElem, m_c4FldIdCurMoonPhIcon, 0, 30);
        }
    }
}


tCIDLib::TVoid
TWeatherChSDriver::ProcessUV(const TXMLTreeElement& xtnodeWind)
{
    const tCIDLib::TCard4 c4ChildCount = xtnodeWind.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeWind.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"i")
        {
            //
            //  It's possible that this could exceed our field range. We set
            //  the range to max out at 18, which is higher than ever recorded
            //  I believe. So that nice graphical indicators can be used. But
            //  it might be higher, so clip it back if needed. So we have to
            //  handle this one manually.
            //
            StoreCardFld(xtnodeElem, m_c4FldIdCurUVIndex, 0, 18);
        }
         else if (xtnodeElem.strQName() == L"t")
        {
            StoreStringFld(xtnodeElem, m_c4FldIdCurUVText);
        }
    }
}


// Process a wind conditions block (under current conditions)
tCIDLib::TVoid
TWeatherChSDriver::ProcessWind(const TXMLTreeElement& xtnodeWind)
{
    const tCIDLib::TCard4 c4ChildCount = xtnodeWind.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ChildCount; c4Index++)
    {
        const TXMLTreeNode& xtnodeCur = xtnodeWind.xtnodeChildAt(c4Index);

        // If not an element, then skip it
        if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
            continue;

        // It is, so cast it to that type and check it
        const TXMLTreeElement& xtnodeElem = static_cast<const TXMLTreeElement&>
        (
            xtnodeCur
        );

        if (xtnodeElem.strQName() == L"s")
        {
            //
            //  For some stupid reason they use 'calm' instead of zero, so
            //  check for that.
            //
            if (xtnodeElem.xtnodeChildAtAsText(0).strText() == L"calm")
                bStoreCardFld(m_c4FldIdCurWindSpeed, 0, kCIDLib::True);
            else
                StoreCardFld(xtnodeElem, m_c4FldIdCurWindSpeed);
        }
         else if (xtnodeElem.strQName() == L"t")
        {
            StoreStringFld(xtnodeElem, m_c4FldIdCurWindDir);
        }
    }
}


//
//  Helpers for locking for field access and updating fields after doing the
//  appropriate type of extraction and validation of the XML data.
//
tCIDLib::TVoid
TWeatherChSDriver::StoreCardFld(const   TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId
                                , const tCIDLib::TCard4     c4Min
                                , const tCIDLib::TCard4     c4Max)
{
    try
    {
        const TString& strVal = xtnodePar.xtnodeChildAtAsText(0).strText();
        tCIDLib::TCard4 c4Val;

        // Watch for any special cases, else convert the value
        if (strVal == L"N/A")
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            c4Val = strVal.c4Val();

            if (c4Val < c4Min)
                c4Val = c4Min;
            else if (c4Val > c4Max)
                c4Val = c4Max;
            bStoreCardFld(c4FldId, c4Val, kCIDLib::True);
        }
    }

    catch(...)
    {
        // Store an error indicator
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


tCIDLib::TVoid
TWeatherChSDriver::StoreFloatFld(const  TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId)
{
    try
    {
        TString strVal = xtnodePar.xtnodeChildAtAsText(0).strText();
        if (strVal == L"N/A")
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            // Strip any commas out, and any leading/trailing space
            tCIDLib::TFloat8 f8Val;
            strVal.Strip(L", ", tCIDLib::EStripModes::Complete);
            f8Val = strVal.f8Val();
            bStoreFloatFld(c4FldId, f8Val, kCIDLib::True);
        }
    }

    catch(...)
    {
        // Store an error indicator
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


tCIDLib::TVoid
TWeatherChSDriver::StoreIconFld(const   TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId)
{
    try
    {
        const TString& strVal = xtnodePar.xtnodeChildAtAsText(0).strText();

        if (strVal == L"-")
            bStoreCardFld(c4FldId, WeatherChS_DriverImpl::c4IconCount, kCIDLib::True);
        else
            bStoreCardFld(c4FldId, strVal.c4Val(), kCIDLib::True);
    }

    catch(...)
    {
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


tCIDLib::TVoid
TWeatherChSDriver::StoreIntFld( const   TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId)
{
    try
    {
        tCIDLib::TInt4 i4Val;
        const TString& strVal = xtnodePar.xtnodeChildAtAsText(0).strText();
        if (strVal == L"N/A")
        {
            SetFieldErr(c4FldId, kCIDLib::True);
        }
         else
        {
            i4Val = strVal.i4Val();
            bStoreIntFld(c4FldId, i4Val, kCIDLib::True);
        }
    }

    catch(...)
    {
        // Store an error indicator
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}


//
//  This one stores a string also, but it's one of the time stamp fields, and
//  they come in a fixed format regardless of the user's location. So we allow
//  them to set a prompt value on us during install to tell us if they want
//  the MM/DD/YY format or the DD/MM/YY format.
//
//  The caller may or may not have field locked, so do it ourself even if it
//  a redundant nested lock.
//
tCIDLib::TVoid
TWeatherChSDriver::StoreLSUPFld(const   TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId)
{
    const TString& strVal = xtnodePar.xtnodeChildAtAsText(0).strText();
    if (m_eDtFmt == EDtFmts::DDMMYY)
    {
        TString strTmp(strVal);
        TStringTokenizer stokDate(&strTmp, L"/");
        TString strDay;
        TString strMonth;
        if (!stokDate.bGetNextToken(strMonth)
        ||  !stokDate.bGetNextToken(strDay))
        {
            // Something is wrong, so take the original as is
            bStoreStringFld(c4FldId, strVal, kCIDLib::True);
        }
         else
        {
            // Put it back together the other way
            TString strNew;
            strNew.Append(strDay);
            strNew.Append(kCIDLib::chForwardSlash);
            strNew.Append(strMonth);
            strNew.Append(kCIDLib::chForwardSlash);

            //
            //  And put the remainder of the string back on the end. Use
            //  day string as a temp here, since we've got the value out
            //  now.
            //
            stokDate.bPeekRestOfLine(strDay);
            strNew.Append(strDay);

            // And now store the modified string
            bStoreStringFld(c4FldId, strNew, kCIDLib::True);
        }
    }
     else
    {
        // We can take it as is in MM/DD/YY
        bStoreStringFld(c4FldId, strVal, kCIDLib::True);
    }
}


tCIDLib::TVoid
TWeatherChSDriver::StoreStringFld(  const   TXMLTreeElement&    xtnodePar
                                    , const tCIDLib::TCard4     c4FldId)
{
    // It's possible that the node is empty
    if (xtnodePar.c4ChildCount())
        bStoreStringFld(c4FldId, xtnodePar.xtnodeChildAtAsText(0).strText(), kCIDLib::True);
    else
        bStoreStringFld(c4FldId, TString::strEmpty(), kCIDLib::True);
}


tCIDLib::TVoid
TWeatherChSDriver::StoreTempFld(const   TXMLTreeElement&    xtnodePar
                                , const tCIDLib::TCard4     c4FldId)
{
    try
    {
        const TString& strVal = xtnodePar.xtnodeChildAtAsText(0).strText();
        tCIDLib::TInt4 i4Val;
        if (strVal == L"N/A")
            i4Val = m_i4LowTempRange;
        else
            i4Val = strVal.i4Val();

        bStoreIntFld(c4FldId, i4Val, kCIDLib::True);
    }

    catch(...)
    {
        // Store an error indicator
        SetFieldErr(c4FldId, kCIDLib::True);
    }
}

