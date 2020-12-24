//
// FILE NAME: WundergroundS_DriverImpl.hpp
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
//  This is the header file for the actual driver class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//   CLASS: TWeatherS
//  PREFIX: drv
// ---------------------------------------------------------------------------
class TWundergroundSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWundergroundSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        ~TWundergroundSDriver();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bGetCommResource
        (
                    TThread&                thrThis
        );

        tCIDLib::TBoolean bWaitConfig
        (
                    TThread&                thrThis
        );

        tCQCKit::ECommResults eConnectToDevice
        (
                    TThread&                thrThis
        );

        tCQCKit::EDrvInitRes eInitializeImpl();

        tCQCKit::ECommResults ePollDevice
        (
                    TThread&                thrThis
        );

        tCIDLib::TVoid ReleaseCommResource();

        tCIDLib::TVoid TerminateImpl();


    private :
        // -------------------------------------------------------------------
        //  Some private class types
        // -------------------------------------------------------------------
        // -------------------------------------------------------------------
        //  Some private class types
        // -------------------------------------------------------------------
        enum EDtFmts
        {
            EDtFmt_MMDDYY
            , EDtFmt_DDMMYY
        };

        typedef tCIDLib::TCard4
            TForecastIds[kWundergroundS::c4ForecastDays][tWundergroundS::EFDay_FldsPer];


        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TWundergroundSDriver();
        TWundergroundSDriver(const TWundergroundSDriver&);
        tCIDLib::TVoid operator=(const TWundergroundSDriver&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoUpdate
        (
            const   TString&                strLocCode
        );

        tCIDLib::TBoolean bIsNAValue
        (
            const   TString&                strToTest
        );

        tCIDLib::TCard4 c4MapIcon
        (
            const   TString&                strCondText
        )   const;

        tCIDLib::TVoid ProcessAstronomy
        (
            const   TJSONObject&            jprsnPar
        );

        tCIDLib::TVoid ProcessCardFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4MinVal = 0
            , const tCIDLib::TCard4         c4MaxVal = kCIDLib::c4MaxCard
        );

        tCIDLib::TVoid ProcessCC
        (
            const   TJSONObject&            jprsnPar
        );

        tCIDLib::TVoid ProcessFCDate
        (
            const   TJSONObject&            jprsnPar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessFloatFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessForecast
        (
            const   TJSONObject&            jprsnPar
        );

        tCIDLib::TVoid ProcessIconFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessIntFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessMetricWind
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessResponse
        (
            const   TJSONObject&            jprsnRoot
        );

        tCIDLib::TVoid ProcessSimpleForecast
        (
            const   TJSONArray&             jprsnPar
        );

        tCIDLib::TVoid ProcessStringFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessSunTime
        (
            const   TJSONObject&            jprsnPar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessTempFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessTextForecast
        (
            const   TJSONArray&             jprsnPar
        );

        tCIDLib::TVoid ProcessOneTFC
        (
            const   TJSONObject&            jprsnPar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid ProcessTimeFld
        (
            const   TJSONObject&            jprsnPar
            , const TString&                strAttrName
            , const tCIDLib::TCard4         c4FldId
            , const TTime::TCompList&       fcolFmt
        );

        tCIDLib::TVoid RegisterFields();



        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_aac4ForecastIds
        //      The field id for the forecast fields. It's a 2D array, with
        //      the first dimension bening the number of forecast days and
        //      the second being the number of fields we have per forecast
        //      day.
        //
        //  m_bShowMetric
        //      One of the driver prompts indicates if they want metric or
        //      standard values.
        //
        //  m_bV2Mode
        //      We support the original V1 driver and a new V2 compliant version.
        //      We check during init and set this flag which is used the rest of
        //      the time. Mostly the only difference is field naming during init,
        //      and nothing else really cares.
        //
        //  m_c4FldIdXXX
        //      We look up our fields after registration, so that we can do
        //      by id field writes during our polls.
        //
        //  m_c4FailCnt
        //      If we get an HTTP GET failure, then we reschedule another
        //      poll a bit later. If we get three if these in a row
        //      we assume we've lost the server and go offline until we can
        //      get reconnected.
        //
        //  m_c4PollMinutes
        //      The number of minutes between polls. This is set from a driver
        //      prompt. For existing driver installs before this prompt was
        //      added it will default to the old 30 minutes.
        //
        //  m_eDtFmt
        //      The user indicates during driver installation the order of
        //      day/month values in date stamp strings.
        //
        //  m_enctNextPoll
        //      The time we need to do the next poll. The data isn't updated
        //      often, so we only poll once every 30 minutes.
        //
        //  m_i4HighTempRange
        //  m_i4LowTempRange
        //      We let them indicate what the want the high/low ranges for their
        //      temp fields to be. Since these can vary radically in various
        //      places, it just makes more sense to let them set them.
        //
        //  m_strCmd_SetLocCode
        //      The names of any commands we accept, just pre-setup during
        //      the ctor.
        //
        //  m_strDevId
        //      The developer id that the user must get by signing up with the
        //      Weather Underground service. They provide this in a driver
        //      prompt.
        //
        //  m_strLocCode
        //      We query weather data by location codes. The user must look
        //      up his location and provide it during driver installation.
        //
        //  m_strTmp
        //      For temporary internal use, stripping off whitespace from
        //      node text and such. Don't expect it to remain unchanged across
        //      any call to another method.
        //
        //  m_tmTmp
        //      For parsing in time stamp values.
        //
        //  m_strAttr_xxxx
        //  m_strElem_xxxx
        //      We set up some names that we specifically look for a lot, to
        //      avoid overhead.
        // -------------------------------------------------------------------
        TForecastIds            m_aac4ForecastIds;
        tCIDLib::TBoolean       m_bShowMetric;
        tCIDLib::TBoolean       m_bV2Mode;
        tCIDLib::TCard4         m_c4FldIdCurAsOf;
        tCIDLib::TCard4         m_c4FldIdCurBaro;
        tCIDLib::TCard4         m_c4FldIdCurCondText;
        tCIDLib::TCard4         m_c4FldIdCurDewPoint;
        tCIDLib::TCard4         m_c4FldIdCurFeelsLike;
        tCIDLib::TCard4         m_c4FldIdCurHumidity;
        tCIDLib::TCard4         m_c4FldIdCurIcon;
        tCIDLib::TCard4         m_c4FldIdCurMoonPhase;
        tCIDLib::TCard4         m_c4FldIdCurMoonPhIcon;
        tCIDLib::TCard4         m_c4FldIdCurPrecip;
        tCIDLib::TCard4         m_c4FldIdCurTemp;
        tCIDLib::TCard4         m_c4FldIdCurUV;
        tCIDLib::TCard4         m_c4FldIdCurVisibility;
        tCIDLib::TCard4         m_c4FldIdCurWindDeg;
        tCIDLib::TCard4         m_c4FldIdCurWindDir;
        tCIDLib::TCard4         m_c4FldIdCurWindGust;
        tCIDLib::TCard4         m_c4FldIdCurWindSpeed;
        tCIDLib::TCard4         m_c4FldIdFCCurrent;
        tCIDLib::TCard4         m_c4FldIdFC12Hour;
        tCIDLib::TCard4         m_c4FldIdFC24Hour;
        tCIDLib::TCard4         m_c4FldIdFC36Hour;
        tCIDLib::TCard4         m_c4FldIdSunrise;
        tCIDLib::TCard4         m_c4FldIdSunset;

        tCIDLib::TCard4         m_c4FailCnt;
        tCIDLib::TCard4         m_c4PollMinutes;
        tCIDLib::TEncodedTime   m_enctNextPoll;
        EDtFmts                 m_eDtFmt;
        tCIDLib::TInt4          m_i4HighTempRange;
        tCIDLib::TInt4          m_i4LowTempRange;
        TString                 m_strCmd_SetLocCode;
        TString                 m_strDevId;
        TString                 m_strLocCode;
        TString                 m_strTmp;
        TTime                   m_tmTmp;

        const TString           m_strAttr_BaroC;
        const TString           m_strAttr_BaroF;
        const TString           m_strAttr_Conditions;
        const TString           m_strAttr_CScale;
        const TString           m_strAttr_Day;
        const TString           m_strAttr_Description;
        const TString           m_strAttr_DewPntC;
        const TString           m_strAttr_DewPntF;
        const TString           m_strAttr_FCText;
        const TString           m_strAttr_FCTextMetric;
        const TString           m_strAttr_FeelsLikeC;
        const TString           m_strAttr_FeelsLikeF;
        const TString           m_strAttr_FScale;
        const TString           m_strAttr_Hour;
        const TString           m_strAttr_Humidity;
        const TString           m_strAttr_Icon;
        const TString           m_strAttr_Min;
        const TString           m_strAttr_Minute;
        const TString           m_strAttr_MoonAge;
        const TString           m_strAttr_Month;
        const TString           m_strAttr_MonthName;
        const TString           m_strAttr_ObsTime;
        const TString           m_strAttr_POP;
        const TString           m_strAttr_Precip;
        const TString           m_strAttr_PrecipMetric;
        const TString           m_strAttr_TempC;
        const TString           m_strAttr_TempF;
        const TString           m_strAttr_Type;
        const TString           m_strAttr_UV;
        const TString           m_strAttr_VisibilityK;
        const TString           m_strAttr_VisibilityM;
        const TString           m_strAttr_Weather;
        const TString           m_strAttr_Weekday;
        const TString           m_strAttr_WeekdayShort;
        const TString           m_strAttr_WindDegrees;
        const TString           m_strAttr_WindDir;
        const TString           m_strAttr_WindGustK;
        const TString           m_strAttr_WindGustM;
        const TString           m_strAttr_WindSpeedK;
        const TString           m_strAttr_WindSpeedM;
        const TString           m_strAttr_Year;

        const TString           m_strElem_CurObservation;
        const TString           m_strElem_Date;
        const TString           m_strElem_Error;
        const TString           m_strElem_Forecast;
        const TString           m_strElem_ForecastDay;
        const TString           m_strElem_High;
        const TString           m_strElem_Low;
        const TString           m_strElem_MoonPhase;
        const TString           m_strElem_SimpleForecast;
        const TString           m_strElem_Sunrise;
        const TString           m_strElem_Sunset;
        const TString           m_strElem_TextForecast;

        const TString           m_strVal_NA;
        const TString           m_strVal_NA2;
        const TString           m_strVal_NA3;
        const TString           m_strVal_NA4;
        const TString           m_strVal_NA5;
        const TString           m_strVal_NA6;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TWundergroundSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


