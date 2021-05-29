//
// FILE NAME: CQCSysCfg_VoiceCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/29/2017
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
//  This file implements a configuration class for room oriented voice control.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCSysCfg_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCSysCfgVoice,TObject)



namespace
{
    namespace CQCSysCfg_VoiceCfg
    {
        // -----------------------------------------------------------------------
        //  The persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2 c2FmtVersion  = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCSysCfgVoice
//  PREFIX: scri
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCSysCfgVoice: Constructor and Destructor
// ---------------------------------------------------------------------------

TCQCSysCfgVoice::TCQCSysCfgVoice() :

    m_bEnable(kCIDLib::False)
    , m_strKeyWord(L"Zira")
{
}

TCQCSysCfgVoice::~TCQCSysCfgVoice()
{
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgVoice: Public operators
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCSysCfgVoice::operator==(const TCQCSysCfgVoice& scvcSrc) const
{
    return
    (
        (m_bEnable == scvcSrc.m_bEnable)
        && (m_strHost == scvcSrc.m_strHost)
        && (m_strKeyWord == scvcSrc.m_strKeyWord)
    );
}

tCIDLib::TBoolean TCQCSysCfgVoice::operator!=(const TCQCSysCfgVoice& scvcSrc) const
{
    return !operator==(scvcSrc);
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgVoice: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCSysCfgVoice::bEnable() const
{
    return m_bEnable;
}

tCIDLib::TBoolean TCQCSysCfgVoice::bEnable(const tCIDLib::TBoolean bToSet)
{
    m_bEnable = bToSet;
    return m_bEnable;
}


tCIDLib::TVoid TCQCSysCfgVoice::Reset()
{
    m_bEnable = kCIDLib::False;
    m_strHost.Clear();
    m_strKeyWord = L"Zira";
}


const TString& TCQCSysCfgVoice::strHost() const
{
    return m_strHost;
}

const TString& TCQCSysCfgVoice::strHost(const TString& strToSet)
{
    m_strHost = strToSet;
    return m_strHost;
}


const TString& TCQCSysCfgVoice::strKeyWord() const
{
    return m_strKeyWord;
}

const TString& TCQCSysCfgVoice::strKeyWord(const TString& strToSet)
{
    m_strKeyWord = strToSet;
    return m_strKeyWord;
}


// ---------------------------------------------------------------------------
//  TCQCSysCfgVoice: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TCQCSysCfgVoice::StreamFrom(TBinInStream& strmSrc)
{
    strmSrc.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmSrc >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCSysCfg_VoiceCfg::c2FmtVersion))
    {
        facCIDLib().ThrowErr
        (
            CID_FILE
            , CID_LINE
            , kCIDErrs::errcGen_UnknownFmtVersion
            , tCIDLib::ESeverities::Failed
            , tCIDLib::EErrClasses::Format
            , TCardinal(c2FmtVersion)
            , TString("TCQCSysCfgVoice")
        );
    }

    // Read in our data
    strmSrc >> m_bEnable
            >> m_strHost
            >> m_strKeyWord;

    strmSrc.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCSysCfgVoice::StreamTo(TBinOutStream& strmTar) const
{
    strmTar << tCIDLib::EStreamMarkers::StartObject
            << CQCSysCfg_VoiceCfg::c2FmtVersion

            << m_bEnable
            << m_strHost
            << m_strKeyWord

            << tCIDLib::EStreamMarkers::EndObject;
}

