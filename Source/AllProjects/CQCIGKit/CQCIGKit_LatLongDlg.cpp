//
// FILE NAME: CQCIGKit_LatLongDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/05/2005
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
//  Gets latitude and longitude from the user.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_LatLongDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TLatLongDlg,TDlgBox)



// ---------------------------------------------------------------------------
//  Local types and data
// ---------------------------------------------------------------------------
namespace CQCIGKit_LatLongDlg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  A list of important cities and their lat/long info.
        // -----------------------------------------------------------------------
        struct TLocItem
        {
            const tCIDLib::TCh* pszName;
            tCIDLib::TFloat8    f8Lat;
            tCIDLib::TFloat8    f8Long;
        };

        constexpr TLocItem aitemCities[] =
        {
            { L"Argentina, Buenos Aires"    , -34.35    , -58.22    }
          , { L"Argentina, Cordoba"         , -31.22    , -54.15    }

          , { L"Australia, Adelaide"        , -34.55    , 138.36    }
          , { L"Australia, Alice Springs"   , -23.48    , 133.53    }
          , { L"Australia, Brisbane"        , -27.29    , 153.8     }
          , { L"Australia, Melbourne"       , -37.47    , 144.58    }
          , { L"Australia, Perth"           , -31.77    , 115.52    }
          , { L"Australia, Sydney"          , -33.52    , 151.12    }

          , { L"Austria, Vienna"            , 40.14     , 16.20     }

          , { L"Belgium, Brussels"          , 50.52     , 4.221     }

          , { L"Brazil, Brazilia"           , -15.52    , -47.55    }
          , { L"Brazil, Rio de Janeiro"     , -22.57    , -43.12    }
          , { L"Brazil, Salvador"           , -12.56    , -38.27    }
          , { L"Brazil, Sao Paolo"          , -23.31    , -46.31    }

          , { L"Canada, Calgary"            , 51.1      , -114.1    }
          , { L"Canada, Quebec"             , 46.50     , -71.15    }
          , { L"Canada, Montreal"           , 45.30     , -73.35    }
          , { L"Canada, Toronto"            , 43.39     , -79.23    }
          , { L"Canada, Vancouver"          , 49.16     , -123.07   }

          , { L"Chile, Santiago"            , -33.27    , -70.42    }


          , { L"China, Beijing"             , 39.55     , 116.26    }
          , { L"China, Hong Kong"           , 22.20     , 114.11    }
          , { L"China, Shanghai"            , 31.10     , 121.28    }

          , { L"Czech Republic, Prague"     , 50.5      , 14.26     }

          , { L"Denmark, Copenhagen"        , 55.40     , 12.34     }

          , { L"England, Birmingham"        , 52.25     , 13.25     }
          , { L"England, Bristol"           , 51.28     , -2.35     }
          , { L"England, London"            , 51.32     , -0.5      }

          , { L"Egypt, Cairo"               , 29.52     , 31.20     }

          , { L"France, Bordeaux"           , 44.50     , -0.31     }
          , { L"France, Lyon"               , 45.42     , 4.47      }
          , { L"France, Marseilles"         , 43.20     , 5.20      }
          , { L"France, Nantes"             , 47.15     , 1.34      }
          , { L"France, Nice"               , 43.42     , 7.16      }
          , { L"France, Paris"              , 48.48     , 2.20      }
          , { L"France, Strasbourg"         , 48.35     , 7.46      }

          , { L"Finland, Helsinki"          , 60.10     , 25.0      }

          , { L"Germany, Berlin"            , 52.30     , 13.25     }
          , { L"Germany, Frankfurt"         , 50.70     , 8.41      }
          , { L"Germany, Hamburg"           , 53.33     , 10.2      }
          , { L"Germany, Munich"            , 48.9      , 11.34     }

          , { L"Greece, Athens"             , 37.58     , 23.43     }

          , { L"Iceland, Reykjavik"         , 64.8      , 21.56     }

          , { L"India, Bangalore"           , 12.58     , 77.35     }
          , { L"India, Bombay"              , 18.54     , 72.49     }
          , { L"India, New Delhi"           , 28.40     , 77.14     }

          , { L"Indonesia, Djakarta"        , -6.16     , 106.48    }
          , { L"Indonesia, Kupang"          , -10.10    , 123.34    }

          , { L"Ireland, Dublin"            , 53.20     , -6.15     }
          , { L"Ireland, Shannon"           , 52.41     , -8.55     }

          , { L"Italy, Milan"               , 45.27     , 9.10      }
          , { L"Italy, Naples"              , 40.50     , 15.15     }
          , { L"Italy, Rome"                , 41.54     , 12.27     }
          , { L"Italy, Venice"              , 45.26     , 12.20     }

          , { L"Japan, Fukuoka"             , 33.35     , 130.27    }
          , { L"Japan, Osaka"               , 34.32     , 139.45    }
          , { L"Japan, Tokoyo"              , 35.40     , 135.30    }

          , { L"Israel, Jerusalem"          , 31.47     , 35.13     }
          , { L"Israel, Tel Aviv"           , 32.6      , 34.47     }

          , { L"Korea, Seoul"               , 37.34     , 126.58    }

          , { L"Mexico, Guadalajara"        , 20.41     , -103.2    }
          , { L"Mexico, Mexico City"        , 19.26     , -99.7     }

          , { L"Netherlands, Amsterdam"     , 52.22     , 4.53      }

          , { L"New Zealand, Aukland"       , -36.52    , 174.45    }
          , { L"New Zealand, ChristChurch"  , -43.32    , 172.37    }
          , { L"New Zealand, Wellington"    , -41.17    , 174.47    }

          , { L"N. Ireland, Belfast"        , 54.37     , -5.56     }

          , { L"Norway, Bergen"             , 60.24     , 5.19      }
          , { L"Norway, Hammerfest"         , 70.38     , 23.38     }
          , { L"Norway, Oslo"               , 59.57     , 10.42     }

          , { L"Poland, Krakow"             , 50.4      , 19.57     }
          , { L"Poland, Warsaw"             , 52.14     , 21.0      }

          , { L"Portugal, Lisban"           , 38.44     , -9.9      }

          , { L"Russia, Kiev"               , 50.27     , 30.30     }
          , { L"Russia, Odessa"             , 46.29     , 30.44     }
          , { L"Russia, Minsk"              , 53.54     , 27.33     }
          , { L"Russia, Moscow"             , 55.45     , 37.36     }
          , { L"Russia, St. Petersburg"     , 59.56     , 30.18     }

          , { L"Saudi Arabia, Jedda"        , 21.28     , 39.10     }
          , { L"Saudi Arabia, Mecca"        , 32.29     , 39.45     }

          , { L"Singapore, Singapore"       , 1.14      , 103.55    }

          , { L"Scotland, Abderdeen"        , 57.9      , -2.9      }

          , { L"South Africa, Capetown"     , -35.55    , 18.29     }
          , { L"South Africa, Johannesburg" , -26.11    , 28.3      }
          , { L"South Africa, Pretoria"     , -25.45    , 28.14     }

          , { L"Spain, Barcelona"           , 41.23     , 2.9       }
          , { L"Spain, Madrid"              , 40.26     , -4.32     }
          , { L"Spain, Valencia"            , 39.28     , -0.23     }

          , { L"Sweden, Stockholm"          , 59.17     , 18.3      }

          , { L"Switzerland, Zurich"        , 47.21     , 8.31      }

          , { L"Taiwan, Taipei"             , 25.05     , 121.32    }

          , { L"Turkey, Ankara"             , 38.55     , 32.55     }

          , { L"USA, Atlanta"               , 33.45     , -84.23    }
          , { L"USA, Austin"                , 30.16     , -97.44    }
          , { L"USA, Baltimore"             , 39.11     , -76.40    }
          , { L"USA, Boulder"               , 37.27     , -105.52   }
          , { L"USA, Charlotte"             , 35.14     , -80.5     }
          , { L"USA, Chicago"               , 41.47     , -87.45    }
          , { L"USA, Columbia (SC)"         , 32.47     , -81.2     }
          , { L"USA, Denver"                , 39.45     , -104.52   }
          , { L"USA, Honolulu"              , 21.20     , -157.55   }
          , { L"USA, Los Angeles"           , 33.56     , -118.24   }
          , { L"USA, Las Vegas"             , 36.10     , -115.12   }
          , { L"USA, New York"              , 40.47     , -73.58    }
          , { L"USA, Orlando"               , 28.33     , -81.23    }
          , { L"USA, Phoenix"               , 33.29     , -112.4    }
          , { L"USA, Raleigh"               , 35.46     , -78.39    }
          , { L"USA, Richmond"              , 37.33     , -77.29    }
          , { L"USA, Salt Lake City"        , 40.46     , -111.58   }
          , { L"USA, San Francisco"         , 37.47     , -122.26   }
          , { L"USA, San Jose"              , 37.20     , -121.53   }
          , { L"USA, Seattle"               , 47.37     , -122.20   }
          , { L"USA, Toledo"                , 41.39     , -83.33    }
          , { L"USA, Washington DC"         , 38.53     , -77.02    }
        };
        constexpr tCIDLib::TCard4 c4CityCount = tCIDLib::c4ArrayElems(aitemCities);
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TLatLongDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLatLongDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TLatLongDlg::TLatLongDlg() :

    m_pwndAcceptButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndCityList(nullptr)
    , m_pwndLat(nullptr)
    , m_pwndLong(nullptr)
{
}

TLatLongDlg::~TLatLongDlg()
{
}


// ---------------------------------------------------------------------------
//  TLatLongDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TLatLongDlg::bRunDlg(const  TWindow&            wndOwner
                    ,       tCIDLib::TFloat8&   f8Lat
                    ,       tCIDLib::TFloat8&   f8Long)
{
    // Store the incoming values till we can get them displayed
    m_f8Lat = f8Lat;
    m_f8Long = f8Long;

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  info on the selected device.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_LatLong
    );

    if (c4Res == kCQCIGKit::ridDlg_LatLong_Accept)
    {
        f8Lat = m_f8Lat;
        f8Long = m_f8Long;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TLatLongDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TLatLongDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCIGKit::ridDlg_LatLong_Accept, m_pwndAcceptButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_LatLong_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_LatLong_CityList, m_pwndCityList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_LatLong_Lat, m_pwndLat);
    CastChildWnd(*this, kCQCIGKit::ridDlg_LatLong_Long, m_pwndLong);

    // Load up the incoming values, if not zeros
    TString strFmt;
    if (m_f8Lat != 0.0)
    {
        strFmt.SetFormatted(m_f8Lat, 3);
        m_pwndLat->strWndText(strFmt);
    }

    if (m_f8Long != 0.0)
    {
        strFmt.SetFormatted(m_f8Long, 3);
        m_pwndLong->strWndText(strFmt);
    }

    // Load up the list of sample cities they can select from
    {
        TWndPaintJanitor janPaint(m_pwndCityList);

        tCIDLib::TStrList colCols(2);
        colCols.objAdd(L"Predefined Major Cities");
        colCols.objAdd(L"City Latitude/Longitude");

        m_pwndCityList->SetColumns(colCols);
        TString& strLocText = colCols[1];
        tCIDLib::TCard4 c4InitSel = 0;
        const tCIDLib::TCard4 c4Count = CQCIGKit_LatLongDlg::c4CityCount;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const CQCIGKit_LatLongDlg::TLocItem& liCur
            (
                CQCIGKit_LatLongDlg::aitemCities[c4Index]
            );

            colCols[0] = liCur.pszName;
            strLocText.SetFormatted(liCur.f8Lat, 3);
            strLocText.Append(kCIDLib::chForwardSlash);
            strLocText.AppendFormatted(liCur.f8Long, 3);

            // Set the original index as the row id so we can get back to it
            m_pwndCityList->c4AddItem(colCols, c4Index);

            // If the incoming matches this one, remember it
            if ((liCur.f8Lat == m_f8Lat) && (liCur.f8Long == m_f8Long))
                c4InitSel = c4Index;
        }

        m_pwndCityList->SelectByIndex(c4InitSel);
    }

    // And register our button event handlers
    m_pwndAcceptButton->pnothRegisterHandler(this, &TLatLongDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TLatLongDlg::eClickHandler);
    m_pwndCityList->pnothRegisterHandler(this, &TLatLongDlg::eLBHandler);

    // Size the first column to fit the content
    m_pwndCityList->AutoSizeCol(0, kCIDLib::False);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TLatLongDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TLatLongDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_LatLong_Accept)
    {
        //
        //  Make sure that there is a valid in each entry field and that they
        //  are valid latitude and longitude values.
        //
        try
        {
            TString strVal;
            strVal = m_pwndLat->strWndText();
            m_f8Lat = strVal.f8Val();

            strVal = m_pwndLong->strWndText();
            m_f8Long = strVal.f8Val();

            EndDlg(kCQCIGKit::ridDlg_LatLong_Accept);
        }

        catch(const TError&)
        {
            TOkBox msgbOk
            (
                facCQCIGKit().strMsg(kIGKitMsgs::midStatus_BadLatOrLong)
            );
            msgbOk.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_LatLong_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_LatLong_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TLatLongDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Get the original list index of the selected item
        const tCIDLib::TCard4 c4Index = m_pwndCityList->c4IndexToId(wnotEvent.c4Index());
        const CQCIGKit_LatLongDlg::TLocItem& liCur
        (
            CQCIGKit_LatLongDlg::aitemCities[c4Index]
        );

        // Load this guy's lat/long into the entry fields
        TString strFmt;
        strFmt.SetFormatted(liCur.f8Lat, 3);
        m_pwndLat->strWndText(strFmt);

        strFmt.SetFormatted(liCur.f8Long, 3);
        m_pwndLong->strWndText(strFmt);
    }

    return tCIDCtrls::EEvResponses::Handled;
}

