//
// FILE NAME: CQCKit_EvMonCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/08/2014
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
//  This file implements the configuration class for events.
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
RTTIDecls(TCQCEvMonCfg,TObject)


namespace CQCKit_EvMonCfg
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Our persistent format version
        //
        //  Version 2 -
        //      We dropped the m_strName member
        //
        //  Version 3 -
        //      We have to convert the event handler class path, which is in the pre-5.x
        //      format, to the new relative hierarchical format. It was in hierarchical
        //      format before, but the old style one, i.e. it's not in dotted format.
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2FmtVersion = 3;
    }
}



// ---------------------------------------------------------------------------
//  CLASS: TKeyedCQCEvMonCfg
// PREFIX: csrc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TKeyedCQCEvMonCfg: Constructors and destructor
// ---------------------------------------------------------------------------
TKeyedCQCEvMonCfg::TKeyedCQCEvMonCfg()
{
}

TKeyedCQCEvMonCfg::TKeyedCQCEvMonCfg(const  TString&        strPath
                                    , const TCQCEvMonCfg&   csrcBase) :
    TCQCEvMonCfg(csrcBase)
    , m_strPath(strPath)
{
}

TKeyedCQCEvMonCfg::TKeyedCQCEvMonCfg(const TKeyedCQCEvMonCfg& csrcSrc) :

    TCQCEvMonCfg(csrcSrc)
    , m_strPath(csrcSrc.m_strPath)
{
}


TKeyedCQCEvMonCfg::~TKeyedCQCEvMonCfg()
{
}


// ---------------------------------------------------------------------------
//  TKeyedCQCEvMonCfg: Public operators
// ---------------------------------------------------------------------------
TKeyedCQCEvMonCfg& TKeyedCQCEvMonCfg::operator=(const TKeyedCQCEvMonCfg& csrcSrc)
{
    if (&csrcSrc != this)
    {
        TCQCEvMonCfg::operator=(csrcSrc);
        m_strPath = csrcSrc.m_strPath;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  Public, non-virtual methods
// ---------------------------------------------------------------------------
const TString& TKeyedCQCEvMonCfg::strPath() const
{
    return m_strPath;
}

const TString& TKeyedCQCEvMonCfg::strPath(const TString& strToSet)
{
    m_strPath = strToSet;
    return m_strPath;
}




// ---------------------------------------------------------------------------
//   CLASS: TCQCEvMonCfg
//  PREFIX: emonc
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEvMonCfg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEvMonCfg::TCQCEvMonCfg() :

    m_bFailed(kCIDLib::False)
    , m_bPaused(kCIDLib::True)
{
}

TCQCEvMonCfg::TCQCEvMonCfg(const TCQCEvMonCfg& emoncSrc) :

    m_bFailed(emoncSrc.m_bFailed)
    , m_bPaused(emoncSrc.m_bPaused)
    , m_strClassPath(emoncSrc.m_strClassPath)
    , m_strDescr(emoncSrc.m_strDescr)
    , m_strId(emoncSrc.m_strId)
    , m_strParams(emoncSrc.m_strParams)
{
}

TCQCEvMonCfg::~TCQCEvMonCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCEvMonCfg: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCEvMonCfg::operator==(const TCQCEvMonCfg& emoncSrc) const
{
    if (this == &emoncSrc)
        return kCIDLib::True;

    return
    (
        (m_bFailed == emoncSrc.m_bFailed)
        && (m_bPaused == emoncSrc.m_bPaused)
        && (m_strClassPath == emoncSrc.m_strClassPath)
        && (m_strDescr == emoncSrc.m_strDescr)
        && (m_strId == emoncSrc.m_strId)
        && (m_strParams == emoncSrc.m_strParams)
    );
}

tCIDLib::TBoolean TCQCEvMonCfg::operator!=(const TCQCEvMonCfg& emoncSrc) const
{
    return !operator==(emoncSrc);
}

TCQCEvMonCfg& TCQCEvMonCfg::operator=(const TCQCEvMonCfg& emoncSrc)
{
    if (this != &emoncSrc)
    {
        m_bFailed       = emoncSrc.m_bFailed;
        m_bPaused       = emoncSrc.m_bPaused;
        m_strClassPath  = emoncSrc.m_strClassPath;
        m_strDescr      = emoncSrc.m_strDescr;
        m_strId         = emoncSrc.m_strId;
        m_strParams     = emoncSrc.m_strParams;
    }
    return *this;
}


// ---------------------------------------------------------------------------
//  TCQCEvMonCfg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCEvMonCfg::bFailed() const
{
    return m_bFailed;
}

tCIDLib::TBoolean TCQCEvMonCfg::bFailed(const tCIDLib::TBoolean bToSet)
{
    m_bFailed = bToSet;
    return m_bFailed;
}


tCIDLib::TBoolean TCQCEvMonCfg::bPaused() const
{
    return m_bPaused;
}

tCIDLib::TBoolean TCQCEvMonCfg::bPaused(const tCIDLib::TBoolean bToSet)
{
    m_bPaused = bToSet;
    return m_bPaused;
}


const TString& TCQCEvMonCfg::strClassPath() const
{
    return m_strClassPath;
}

const TString& TCQCEvMonCfg::strClassPath(const TString& strToSet)
{
    m_strClassPath = strToSet;
    return m_strClassPath;
}


const TString& TCQCEvMonCfg::strDescr() const
{
    return m_strDescr;
}


const TString& TCQCEvMonCfg::strParams() const
{
    return m_strParams;
}


// Only kept around for the 5.0 upgrade process
const TString& TCQCEvMonCfg::strId() const
{
    return m_strId;
}


// Set us up with new data, mostly for use by the configuration interface
tCIDLib::TVoid
TCQCEvMonCfg::Set(  const   TString&    strClassPath
                    , const TString&    strDescr
                    , const TString&    strParams)
{
    m_strClassPath = strClassPath;
    m_strDescr = strDescr;
    m_strParams = strParams;
}



// ---------------------------------------------------------------------------
//  TCQCEvMonCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCEvMonCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_EvMonCfg::c2FmtVersion))
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

    strmToReadFrom  >> m_bFailed
                    >> m_bPaused
                    >> m_strClassPath
                    >> m_strDescr;

    // If V1, read in the name and discard it
    if (c2FmtVersion < 2)
    {
        TString strTmp;
        strmToReadFrom >> strTmp;
    }

    strmToReadFrom  >> m_strParams
                    >> m_strId;

    //
    //  If pre-5.x, then we have to convert the old path to the 5.x style path. It's
    //  a bit tricker for CML paths, since they used to be \MEng\User\blah type, but
    //  now they are just /User/blah, because we dropped the MEng (though it's still
    //  there in the dotted form of CML classpaths.) So we can't just call the standard
    //  Make50Path() helper by itself. We need to remove the \MEng part, then call it.
    //
    if (c2FmtVersion < 3)
    {
        // Just in case check it does start with what we expect
        if (m_strClassPath.bStartsWithI(L"\\MEng\\"))
            m_strClassPath.Cut(0, 5);
        facCQCKit().Make50Path(m_strClassPath);
    }

    // And the end object marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCEvMonCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_EvMonCfg::c2FmtVersion

                    << m_bFailed
                    << m_bPaused
                    << m_strClassPath
                    << m_strDescr
                    << m_strParams
                    << m_strId

                    << tCIDLib::EStreamMarkers::Frame;
}


