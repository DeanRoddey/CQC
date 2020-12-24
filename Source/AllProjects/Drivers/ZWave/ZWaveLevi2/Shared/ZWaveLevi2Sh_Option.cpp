//
// FILE NAME: ZWaveLevi2Sh_Option.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/28/2014
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
//  This file provides the implementation for the TZWOption class, which
//  represents a single optoin.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLevi2Sh_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TZWOption,TObject)
RTTIDecls(TZWOptionVal,TObject)


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace ZWaveLevi2Sh_Option
{
    // -----------------------------------------------------------------------
    //  The option value persistent format version
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2       c2ValFmtVersion = 1;
}



// ---------------------------------------------------------------------------
//   CLASS: TZWOption
//  PREFIX: zwopt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWOption: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWOption::TZWOption()
{
}

TZWOption::~TZWOption()
{
}



// ---------------------------------------------------------------------------
//   CLASS: TZWOptionVal
//  PREFIX: zwopt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TZWOptionVal: Public, static methods
// ---------------------------------------------------------------------------
const TString& TZWOptionVal::strKey(const TZWOptionVal& zwoptvSrc)
{
    return zwoptvSrc.m_strKey;
}


// ---------------------------------------------------------------------------
//  TZWOptionVal: Constructors and Destructor
// ---------------------------------------------------------------------------
TZWOptionVal::TZWOptionVal(const tCIDLib::TBoolean bFixed) :

    m_bFixed(bFixed)
{
}

TZWOptionVal::TZWOptionVal( const   TString&            strKey
                            , const TString&            strValue
                            , const tCIDLib::TBoolean   bFixed
                            , const TString&            strAllowedVals) :

    m_bFixed(bFixed)
    , m_strKey(strKey)
    , m_strValue(strValue)
{
    StoreAllowedVals(strAllowedVals);
}

TZWOptionVal::TZWOptionVal(const TZWOptionVal& zwoptvSrc) :

    m_bFixed(zwoptvSrc.m_bFixed)
    , m_colAllowedVals(zwoptvSrc.m_colAllowedVals)
    , m_strKey(zwoptvSrc.m_strKey)
    , m_strValue(zwoptvSrc.m_strValue)
{
}

TZWOptionVal::~TZWOptionVal()
{
}


// ---------------------------------------------------------------------------
//  TZWOptionVal: Public operators
// ---------------------------------------------------------------------------

TZWOptionVal& TZWOptionVal::operator=(const TZWOptionVal& zwoptvSrc)
{
    if (&zwoptvSrc != this)
    {
        m_bFixed        = zwoptvSrc.m_bFixed;
        m_colAllowedVals= zwoptvSrc.m_colAllowedVals;
        m_strKey        = zwoptvSrc.m_strKey;
        m_strValue      = zwoptvSrc.m_strValue;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TZWOptionVal: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Set our key and value. Don't do this one one already in a hashed collection!
//  The name is the key. It's just for initial setup.
//
tCIDLib::TVoid TZWOptionVal::Set(const  TString&            strKey
                                , const TString&            strValue
                                , const tCIDLib::TBoolean   bFixed
                                , const TString&            strAllowedVals)
{
    m_bFixed = bFixed;
    m_strKey = strKey;
    m_strValue = strValue;

    StoreAllowedVals(strAllowedVals);
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TZWOptionVal::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Make sure we get the start marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > ZWaveLevi2Sh_Option::c2ValFmtVersion))
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

    strmToReadFrom  >> m_strKey
                    >> m_strValue
                    >> m_bFixed
                    >> m_colAllowedVals;

    // Make sure we get the end marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TZWOptionVal::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << ZWaveLevi2Sh_Option::c2ValFmtVersion
                    << m_strKey
                    << m_strValue
                    << m_bFixed
                    << m_colAllowedVals
                    << tCIDLib::EStreamMarkers::EndObject;
}


// ---------------------------------------------------------------------------
//  TZWaveUnit: Private, non-virtual methods
// ---------------------------------------------------------------------------

// This is called to break apart  the allowed values string to individual strings
tCIDLib::TVoid TZWOptionVal::StoreAllowedVals(const TString& strVals)
{
    m_colAllowedVals.RemoveAll();
    TStringTokenizer stokVals(&strVals, L",");

    TString strVal;
    while (stokVals.bGetNextToken(strVal))
    {
        strVal.StripWhitespace();
        if (strVal.bIsEmpty())
            break;
        m_colAllowedVals.objAdd(strVal);
    }
}


