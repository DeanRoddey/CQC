//
// FILE NAME: MediaRepoMgr_RepoSettings.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/11/2006
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
//  This is the header for the MediaRepoMgr_RepoSettings.cpp file, which implements
//  our persisted repository options class.
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
#include    "MediaRepoMgr.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMediaRepoSettings,TObject)



// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace MediaRepoMgr_RepoSettings
{
    // -----------------------------------------------------------------------
    //  The persistent format verison for our media repository settings class
    //
    //  Version 2 -
    //      Moved from a simple quality enum to a more open ended scheme where
    //      the user selects an encoder and then a codec and format.
    //
    //  Version 3 -
    //      Added support for configuring My Movies web service access for
    //      looking up metadata.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard2   c2FmtVersion = 3;
}





// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoSettings
//  PREFIX: mrs
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaRepoSettings::TMediaRepoSettings()
{
    // Just call our reset method to get the stuff initialized
    Reset();
}

TMediaRepoSettings::TMediaRepoSettings(const TMediaRepoSettings& mrsSrc) :

    m_bJitterCorrection(mrsSrc.m_bJitterCorrection)
    , m_bRipEject(mrsSrc.m_bRipEject)
    , m_bShowAdult(mrsSrc.m_bShowAdult)
    , m_bShowEnglish(mrsSrc.m_bShowEnglish)
    , m_bUseMyMovies(mrsSrc.m_bUseMyMovies)
    , m_strCodec(mrsSrc.m_strCodec)
    , m_strCodecFmt(mrsSrc.m_strCodecFmt)
    , m_strEncoder(mrsSrc.m_strEncoder)
    , m_strMMName(mrsSrc.m_strMMName)
    , m_strMMPassword(mrsSrc.m_strMMPassword)
{
}

TMediaRepoSettings::~TMediaRepoSettings()
{
}


// ---------------------------------------------------------------------------
//  Public operators
// ---------------------------------------------------------------------------
TMediaRepoSettings& TMediaRepoSettings::operator=(const TMediaRepoSettings& mrsSrc)
{
    if (this != &mrsSrc)
    {
        m_bJitterCorrection = mrsSrc.m_bJitterCorrection;
        m_bRipEject         = mrsSrc.m_bRipEject;
        m_bShowAdult        = mrsSrc.m_bShowAdult;
        m_bShowEnglish      = mrsSrc.m_bShowEnglish;
        m_bUseMyMovies      = mrsSrc.m_bUseMyMovies;
        m_strCodec          = mrsSrc.m_strCodec;
        m_strCodecFmt       = mrsSrc.m_strCodecFmt;
        m_strEncoder        = mrsSrc.m_strEncoder;
        m_strMMName         = mrsSrc.m_strMMName;
        m_strMMPassword     = mrsSrc.m_strMMPassword;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, non-virtual method
// ---------------------------------------------------------------------------

// Get/set the jitter correction flag
tCIDLib::TBoolean TMediaRepoSettings::bJitterCorrection() const
{
    return m_bJitterCorrection;
}

tCIDLib::TBoolean
TMediaRepoSettings::bJitterCorrection(const tCIDLib::TBoolean bToSet)
{
    m_bJitterCorrection = bToSet;
    return m_bJitterCorrection;
}


// Get/set the eject after rip flag
tCIDLib::TBoolean TMediaRepoSettings::bRipEject() const
{
    return m_bRipEject;
}

tCIDLib::TBoolean
TMediaRepoSettings::bRipEject(const tCIDLib::TBoolean bToSet)
{
    m_bRipEject = bToSet;
    return m_bRipEject;
}


// Get/set the show adult flag
tCIDLib::TBoolean TMediaRepoSettings::bShowAdult() const
{
    return m_bShowAdult;
}

tCIDLib::TBoolean
TMediaRepoSettings::bShowAdult(const tCIDLib::TBoolean bToSet)
{
    m_bShowAdult = bToSet;
    return m_bShowAdult;
}


// Get/set the show English flag
tCIDLib::TBoolean TMediaRepoSettings::bShowEnglish() const
{
    return m_bShowEnglish;
}

tCIDLib::TBoolean
TMediaRepoSettings::bShowEnglish(const tCIDLib::TBoolean bToSet)
{
    m_bShowEnglish = bToSet;
    return m_bShowEnglish;
}


// Get/set the flag to use My Movies for metadata or not
tCIDLib::TBoolean TMediaRepoSettings::bUseMyMovies() const
{
    return m_bUseMyMovies;
}

tCIDLib::TBoolean
TMediaRepoSettings::bUseMyMovies(const tCIDLib::TBoolean bToSet)
{
    m_bUseMyMovies = bToSet;
    return m_bUseMyMovies;
}


// Reset to defaults
tCIDLib::TVoid TMediaRepoSettings::Reset()
{
    m_bJitterCorrection = kCIDLib::False;
    m_bShowAdult        = kCIDLib::False;
    m_bShowEnglish      = kCIDLib::True;
    m_bRipEject         = kCIDLib::False;
    m_bUseMyMovies      = kCIDLib::False;
    m_strEncoder        = TCIDDAEWMAEnc::strCodecName();

    m_strCodec.Clear();
    m_strCodecFmt.Clear();
    m_strMMName.Clear();
    m_strMMPassword.Clear();
}


// Get/set the codec name
const TString& TMediaRepoSettings::strCodec() const
{
    return m_strCodec;
}

const TString& TMediaRepoSettings::strCodec(const TString& strToSet)
{
    m_strCodec = strToSet;
    return m_strCodec;
}


// Get/set the codec format name
const TString& TMediaRepoSettings::strCodecFmt() const
{
    return m_strCodecFmt;
}

const TString& TMediaRepoSettings::strCodecFmt(const TString& strToSet)
{
    m_strCodecFmt = strToSet;
    return m_strCodecFmt;
}


// Get/set the encoder name
const TString& TMediaRepoSettings::strEncoder() const
{
    return m_strEncoder;
}

const TString& TMediaRepoSettings::strEncoder(const TString& strToSet)
{
    m_strEncoder = strToSet;
    return m_strEncoder;
}


// Get/set the My Movies user name
const TString& TMediaRepoSettings::strMMName() const
{
    return m_strMMName;
}

const TString& TMediaRepoSettings::strMMName(const TString& strToSet)
{
    m_strMMName = strToSet;
    return m_strMMName;
}


// Get/set the My Movies password
const TString& TMediaRepoSettings::strMMPassword() const
{
    return m_strMMPassword;
}

const TString& TMediaRepoSettings::strMMPassword(const TString& strToSet)
{
    m_strMMPassword = strToSet;
    return m_strMMPassword;
}


// ---------------------------------------------------------------------------
//  Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TMediaRepoSettings::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the version to make sure we understand it
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > MediaRepoMgr_RepoSettings::c2FmtVersion))
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

    // Stream our stuff in
    strmToReadFrom  >> m_bJitterCorrection
                    >> m_bRipEject;

    //
    //  If 1, then read in and toss the encoding mode and default the
    //  new encoding stuff.
    //
    if (c2FmtVersion == 1)
    {
        tCIDLib::TInt4 i4Junk;
        strmToReadFrom >> i4Junk;

        // For now we just hard code the WMA encoder
        m_strEncoder = TCIDDAEWMAEnc::strCodecName();
        m_strCodec.Clear();
        m_strCodecFmt.Clear();
    }
     else
    {
        strmToReadFrom  >> m_strEncoder
                        >> m_strCodec
                        >> m_strCodecFmt;
    }

    // If version 3 or beyond, read in My Movies settings, else default them
    if (c2FmtVersion < 3)
    {
        m_bUseMyMovies = kCIDLib::False;
        m_bShowAdult = kCIDLib::False;
        m_bShowEnglish = kCIDLib::True;
        m_strMMName.Clear();
        m_strMMPassword.Clear();
    }
     else
    {
        strmToReadFrom  >> m_strMMName
                        >> m_bUseMyMovies
                        >> m_strMMPassword
                        >> m_bShowAdult
                        >> m_bShowEnglish;
    }

    // And the end object marker
    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TMediaRepoSettings::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << MediaRepoMgr_RepoSettings::c2FmtVersion
                    << m_bJitterCorrection
                    << m_bRipEject
                    << m_strEncoder
                    << m_strCodec
                    << m_strCodecFmt
                    << m_strMMName
                    << m_bUseMyMovies
                    << m_strMMPassword
                    << m_bShowAdult
                    << m_bShowEnglish
                    << tCIDLib::EStreamMarkers::EndObject;
}


