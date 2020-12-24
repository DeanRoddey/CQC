//
// FILE NAME: WeatherChS_DriverImpl.hpp
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
class TWeatherChSDriver : public TCQCServerBase
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TWeatherChSDriver() = delete;

        TWeatherChSDriver
        (
            const   TCQCDriverObjCfg&       cqcdcToLoad
        );

        TWeatherChSDriver(const TWeatherChSDriver&) = delete;
        TWeatherChSDriver(TWeatherChSDriver&&) = delete;

        ~TWeatherChSDriver();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TWeatherChSDriver& operator=(const TWeatherChSDriver&) = delete;
        TWeatherChSDriver& operator=(TWeatherChSDriver&&) = delete;


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

        tCIDLib::TCard4 c4SendCmd
        (
            const   TString&                strCmdId
            , const TString&                strParms
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
        enum class EDtFmts
        {
            MMDDYY
            , DDMMYY
        };


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bDoUpdate
        (
            const   TString&                strLocCode
        );

        tCIDLib::TVoid ProcessBar
        (
            const   TXMLTreeElement&        xtnodeBar
        );

        tCIDLib::TVoid ProcessCC
        (
            const   TXMLTreeElement&        xtnodeCC
        );

        tCIDLib::TVoid ProcessDay
        (
            const   TXMLTreeElement&        xtnodeDay
        );

        tCIDLib::TVoid ProcessForecast
        (
            const   TXMLTreeElement&        xtnodeFC
        );

        tCIDLib::TVoid ProcessMoon
        (
            const   TXMLTreeElement&        xtnodeMoon
        );

        tCIDLib::TVoid ProcessUV
        (
            const   TXMLTreeElement&        xtnodeWind
        );

        tCIDLib::TVoid ProcessWind
        (
            const   TXMLTreeElement&        xtnodeWind
        );

        tCIDLib::TVoid StoreCardFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
            , const tCIDLib::TCard4         c4MinVal = 0
            , const tCIDLib::TCard4         c4MaxVal = kCIDLib::c4MaxCard
        );

        tCIDLib::TVoid StoreFloatFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid StoreIconFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid StoreIntFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid StoreLSUPFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid StoreStringFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );

        tCIDLib::TVoid StoreTempFld
        (
            const   TXMLTreeElement&        xtnodePar
            , const tCIDLib::TCard4         c4FldId
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4FldIdXXX
        //      We look up our fields after registration, so that we can do
        //      by id field writes during our polls.
        //
        //  m_chUnits
        //      We can query in metric or standard units, and we let the
        //      use set this during installation. During init, we pull this
        //      out and the actual character that we'll use.
        //
        //  m_eDtFmt
        //      The dates are formatted the same, no matter what the user's
        //      location, in MM/DD/YY format. We allow them, via a user
        //      prompt, to select the DD/MM/YY if they want and we flip them.
        //
        //  m_enctNextForecast
        //  m_enctNextPoll
        //      The poll period is very long for us, 30 minutes basically,
        //      since the weather data doesn't get updated any faster than
        //      that anyway. So we set the time for the next poll and the poll
        //      method just checks each time and when the period is up, he
        //      does a poll. We also have one to trigger updating the forecast
        //      info, which is on a 2 hour period.
        //
        //      During the ctor, they are set to the current time, which will
        //      force an initial poll of both.
        //
        //  m_fcolForecastIds
        //      A nested list of days that contains field ids for those days. This
        //      is where we store the field ids for the forecast days, which are
        //      identical sets of fields. So this outer one has an entry per forecast
        //      day, and the inner one has an entry per EFDayFld enum values.
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
        //  m_strLicKey
        //      The developer id and license key that we get as driver prompt
        //      values. The user must get their own id and key and provide it
        //      to us.
        //
        //  m_strPostalCode
        //      We query weather data by postal code, so we have a prompt to
        //      get a postal code from the user which we'll use in our queries.
        //
        //  m_strTmp
        //      For temporary internal use, stripping off whitespace from
        //      node text and such. Don't expect it to remain unchanged across
        //      any call to another method.
        //
        //  m_xtprsWC
        //      An XML parser that we use to parse the data out.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_c4FldIdCurAsOf;
        tCIDLib::TCard4             m_c4FldIdCurBaro;
        tCIDLib::TCard4             m_c4FldIdCurBaroTrend;
        tCIDLib::TCard4             m_c4FldIdCurCondText;
        tCIDLib::TCard4             m_c4FldIdCurDewPoint;
        tCIDLib::TCard4             m_c4FldIdCurFeelsLike;
        tCIDLib::TCard4             m_c4FldIdCurHumidity;
        tCIDLib::TCard4             m_c4FldIdCurIcon;
        tCIDLib::TCard4             m_c4FldIdCurMoonPhase;
        tCIDLib::TCard4             m_c4FldIdCurMoonPhIcon;
        tCIDLib::TCard4             m_c4FldIdCurTemp;
        tCIDLib::TCard4             m_c4FldIdCurUVIndex;
        tCIDLib::TCard4             m_c4FldIdCurUVText;
        tCIDLib::TCard4             m_c4FldIdCurVisibility;
        tCIDLib::TCard4             m_c4FldIdCurWindSpeed;
        tCIDLib::TCard4             m_c4FldIdCurWindDir;
        tCIDLib::TCard4             m_c4FldIdForecastAsOf;
        tCIDLib::TCard4             m_c4FldIdSunrise;
        tCIDLib::TCard4             m_c4FldIdSunset;
        tCIDLib::TCh                m_chUnits;
        EDtFmts                     m_eDtFmt;
        tCIDLib::TEncodedTime       m_enctNextForecast;
        tCIDLib::TEncodedTime       m_enctNextPoll;
        tWeatherChS::TDayList       m_fcolForecastIds;
        tCIDLib::TInt4              m_i4HighTempRange;
        tCIDLib::TInt4              m_i4LowTempRange;
        const TString               m_strCmd_SetLocCode;
        TString                     m_strDevId;
        TString                     m_strLicKey;
        TString                     m_strPostalCode;
        TString                     m_strTmp;
        TXMLTreeParser              m_xtprsWC;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TWeatherChSDriver,TCQCServerBase)
};

#pragma CIDLIB_POPPACK


