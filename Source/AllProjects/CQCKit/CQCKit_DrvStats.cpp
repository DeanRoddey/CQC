//
// FILE NAME: CQCKit_DrvStats.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/23/2016
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
//  This file implements the driver statistics class
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
RTTIDecls(TCQCDrvStats,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace
{
    namespace CQCKit_DrvStats
    {
        // -----------------------------------------------------------------------
        //  Our current persistent format version
        // -----------------------------------------------------------------------
        constexpr tCIDLib::TCard2       c2DefFmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCDrvStates
//  PREFIX: cdstats
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCDrvStats: Constructors and Destructor
// ---------------------------------------------------------------------------

// Just call our own reset method to get the values initialized
TCQCDrvStats::TCQCDrvStats()
{
    Reset();
}

TCQCDrvStats::~TCQCDrvStats()
{
}


// ---------------------------------------------------------------------------
//  TCQCDrvStats: Public operators
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCDrvStats::operator==(const TCQCDrvStats& cdstatsSrc) const
{
    if (&cdstatsSrc != this)
    {
        if ((m_eStatus != cdstatsSrc.m_eStatus)
        ||  (m_eVerboseLvl != cdstatsSrc.m_eVerboseLvl)
        ||  (m_c4ThreadId != cdstatsSrc.m_c4ThreadId)
        ||  (m_c4BadMsgs != cdstatsSrc.m_c4BadMsgs)
        ||  (m_c4FailedWrites != cdstatsSrc.m_c4FailedWrites)
        ||  (m_c8LoadTime != cdstatsSrc.m_c8LoadTime)
        ||  (m_c4LostCommRes != cdstatsSrc.m_c4LostCommRes)
        ||  (m_c4LostConns != cdstatsSrc.m_c4LostConns)
        ||  (m_c4Naks != cdstatsSrc.m_c4Naks)
        ||  (m_c4Reconfigures != cdstatsSrc.m_c4Reconfigures)
        ||  (m_c4Timeouts != cdstatsSrc.m_c4Timeouts)
        ||  (m_c4UnknownMsgs != cdstatsSrc.m_c4UnknownMsgs)
        ||  (m_c4UnknownWrites != cdstatsSrc.m_c4UnknownWrites))
        {
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCDrvStats::operator!=(const TCQCDrvStats& cdstatsSrc) const
{
    return !operator==(cdstatsSrc);
}


// ---------------------------------------------------------------------------
//  TCQCDrvSTats: Public, non-virtual methods
// ------------------------------------------------------------------

//
//  We initialize them to obviously invalid values so any valid values will appear to be
//  changes.
//
tCIDLib::TVoid TCQCDrvStats::Reset()
{
    m_eStatus        = tCQCKit::EDrvStates::Count;
    m_eVerboseLvl    = tCQCKit::EVerboseLvls::Count;
    m_c4ThreadId     = kCIDLib::c4MaxCard;
    m_c4BadMsgs      = kCIDLib::c4MaxCard;
    m_c4FailedWrites = kCIDLib::c4MaxCard;
    m_c8LoadTime     = kCIDLib::c4MaxCard;
    m_c4LostCommRes  = kCIDLib::c4MaxCard;
    m_c4LostConns    = kCIDLib::c4MaxCard;
    m_c4Naks         = kCIDLib::c4MaxCard;
    m_c4Reconfigures = kCIDLib::c4MaxCard;
    m_c4Timeouts     = kCIDLib::c4MaxCard;
    m_c4UnknownMsgs  = kCIDLib::c4MaxCard;
    m_c4UnknownWrites= kCIDLib::c4MaxCard;
}


// ---------------------------------------------------------------------------
//  TCQCDrvSTats: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCDrvStats::StreamFrom(TBinInStream& strmToReadFrom)
{
    // Check the start object marker
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check our version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom  >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_DrvStats::c2DefFmtVersion))
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

    strmToReadFrom  >> m_eStatus
                    >> m_eVerboseLvl
                    >> m_c4ThreadId
                    >> m_c4BadMsgs
                    >> m_c4FailedWrites
                    >> m_c8LoadTime
                    >> m_c4LostCommRes
                    >> m_c4LostConns
                    >> m_c4Naks
                    >> m_c4Reconfigures
                    >> m_c4Timeouts
                    >> m_c4UnknownMsgs
                    >> m_c4UnknownWrites;

    // And the end object marker
    strmToReadFrom.CheckForFrameMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCDrvStats::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_DrvStats::c2DefFmtVersion

                    << m_eStatus
                    << m_eVerboseLvl
                    << m_c4ThreadId
                    << m_c4BadMsgs
                    << m_c4FailedWrites
                    << m_c8LoadTime
                    << m_c4LostCommRes
                    << m_c4LostConns
                    << m_c4Naks
                    << m_c4Reconfigures
                    << m_c4Timeouts
                    << m_c4UnknownMsgs
                    << m_c4UnknownWrites

                    << tCIDLib::EStreamMarkers::Frame;
}
