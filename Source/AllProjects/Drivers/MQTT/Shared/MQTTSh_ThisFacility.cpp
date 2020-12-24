//
// FILE NAME: MQTTSh_ThisFacility.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/21/2018
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
//  This is the implementation for the facility class.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "MQTTSh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TFacMQTTSh, TFacility)



// ---------------------------------------------------------------------------
//   CLASS: TFacMQTTSh
//  PREFIX: fac
// ---------------------------------------------------------------------------

// -------------------------------------------------------------------
//  TFacMQTTSh: Constructors and Destructor
// -------------------------------------------------------------------
TFacMQTTSh::TFacMQTTSh() :

    TFacility
    (
        L"MQTTSh"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    )
    , m_bTraceMode(kCIDLib::False)
    , m_eVerbosity(tCQCKit::EVerboseLvls::Off)
{
}

TFacMQTTSh::~TFacMQTTSh()
{
}


// -------------------------------------------------------------------
//  TFacMQTTSh: Public, non-virtual methods
// -------------------------------------------------------------------

//
//  The server calls this to set up options on us that he gets from the cmd line
//  and/or environment.
//
tCIDLib::TBoolean
TFacMQTTSh::bTraceMode(const tCIDLib::TBoolean bToSet)
{
    TLocker lockrSync(&m_mtxSync);

    // If already in the requested mode, do nothing
    if (m_bTraceMode == bToSet)
        return bToSet;

    //
    //  Either open the trace or close it depending on the new mode, which we know
    //  is not the current mode.
    //
    if (bToSet)
    {
        try
        {
            // Set the desired format on our trace file time stamp
            m_tmTrace.strDefaultFormat(TTime::strMMDD_24HHMMSS());

            TPathStr pathTrace = facCQCKit().strServerDataDir();
            pathTrace.AddLevel(L"MQTT");
            pathTrace.AddLevel(L"MQTT_Trace");
            pathTrace.AppendExt(L".Txt");
            m_strmTrace.Open
            (
                pathTrace
                , tCIDLib::ECreateActs::CreateAlways
                , tCIDLib::EFilePerms::Default
                , tCIDLib::EFileFlags::SequentialScan
                , tCIDLib::EAccessModes::Write
            );

            m_strmTrace << L"Starting MQTT GW trace..." << kCIDLib::NewEndLn;
            m_bTraceMode = kCIDLib::True;
        }

        catch (TError& errToCatch)
        {
            errToCatch.AddStackLevel(CID_FILE, CID_LINE);
            TModule::LogEventObj(errToCatch);
            m_bTraceMode = kCIDLib::False;
        }

        catch (...)
        {
            m_bTraceMode = kCIDLib::False;
        }
    }
     else
    {
        m_bTraceMode = kCIDLib::False;
        if (m_strmTrace.bIsOpen())
            m_strmTrace.Close();
    }
    return m_bTraceMode;
}



//
//  Various helpers to dump stuff to the trace stream. For the msg and error ones
//  we have separate ones for our thread to call and the I/O thread to call, so
//  that we can mark them for easy identification in the trace
//
tCIDLib::TVoid TFacMQTTSh::LogInMsg(const TMQTTInMsg& msgSrc)
{
    TLocker lockrSync(&m_mtxSync);
    msgSrc.LogTo(m_strmTrace, m_tmTrace);
}

tCIDLib::TVoid TFacMQTTSh::LogOutMsg(const TMQTTOutMsg& msgSrc)
{
    TLocker lockrSync(&m_mtxSync);
    msgSrc.LogTo(m_strmTrace, m_tmTrace);
}

tCIDLib::TVoid
TFacMQTTSh::LogTraceMsg(const   tMQTTSh::EMsgSrcs   eSrc
                        , const TString&            strMsg
                        , const MFormattable&       fmtbl1
                        , const MFormattable&       fmtbl2
                        , const MFormattable&       fmtbl3)
{
    LogTraceOutput(eSrc, strMsg, kCIDLib::False, fmtbl1, fmtbl2, fmtbl3);
}

tCIDLib::TVoid
TFacMQTTSh::LogTraceErr(const   tMQTTSh::EMsgSrcs   eSrc
                        , const TString&            strMsg
                        , const MFormattable&       fmtbl1
                        , const MFormattable&       fmtbl2
                        , const MFormattable&       fmtbl3)
{
    LogTraceOutput(eSrc, strMsg, kCIDLib::True, fmtbl1, fmtbl2, fmtbl3);
}


//
//  A helper that the other trace log methods call to avoid redundancy, and to
//  have a single place to intercept them if needed.
//
tCIDLib::TVoid
TFacMQTTSh::LogTraceOutput( const   tMQTTSh::EMsgSrcs   eSrc
                            , const TString&            strMsg
                            , const tCIDLib::TBoolean   bError
                            , const MFormattable&       fmtblToken1
                            , const MFormattable&       fmtblToken2
                            , const MFormattable&       fmtblToken3)
{
    // Shouldn't happen, but catch it just in case
    if (!m_bTraceMode)
        return;

    TString strExp(strMsg);
    if (!MFormattable::bIsNullObject(fmtblToken1))
        strExp.eReplaceToken(fmtblToken1, L'1');
    if (!MFormattable::bIsNullObject(fmtblToken2))
        strExp.eReplaceToken(fmtblToken2, L'2');
    if (!MFormattable::bIsNullObject(fmtblToken3))
        strExp.eReplaceToken(fmtblToken3, L'3');

    TLocker lockrSync(&m_mtxSync);
    m_tmTrace.SetToNow();
    if (bError)
        m_strmTrace << L"[ERR:";
    else
        m_strmTrace << L"[INF:";

    m_strmTrace << eSrc << kCIDLib::chColon
                << m_tmTrace << L"] - " << strExp << kCIDLib::EndLn;
}


// Gen up a map of the indicted type
TMQTTValMap* TFacMQTTSh::pmqvmpMakeNew(const TString& strType) const
{
    if (strType == L"BoolGenMap")
        return new TMQTTBoolGenMap();
    if (strType == L"BoolNumMap")
        return new TMQTTBoolNumMap();
    else if (strType == L"BoolTextMap")
        return new TMQTTBoolTextMap();
    else if (strType == L"EnumMap")
        return new TMQTTEnumMap();
    else if (strType == L"JSONMap")
        return new TMQTTJSONMap();
    else if (strType == L"MLineMap")
        return new TMQTTMLineMap();
    else if (strType == L"ScaleRangeMap")
        return new TMQTTScaleRangeMap();
    else if (strType == L"TextMap")
        return new TMQTTTextMap();

    return nullptr;
}
