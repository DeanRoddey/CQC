//
// FILE NAME: CQCKit_ConnCfg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/25/2001
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
#include    "CQCKit_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCConnCfg,TObject)


// ---------------------------------------------------------------------------
//  Local data
// ---------------------------------------------------------------------------
namespace CQCKit_ConnCfg
{
    namespace
    {
        // Our current persistent format version
        constexpr   tCIDLib::TCard2     c2FmtVersion = 1;
    }
}



// ---------------------------------------------------------------------------
//   CLASS: TCQCConnCfg
//  PREFIX: conncfg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCConnCfg: Destructor
// ---------------------------------------------------------------------------
TCQCConnCfg::~TCQCConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCConnCfg: Hidden constructors and operators
// ---------------------------------------------------------------------------
TCQCConnCfg::TCQCConnCfg()
{
}


// ---------------------------------------------------------------------------
//  TCQCConnCfg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TCQCConnCfg::StreamFrom(TBinInStream& strmToReadFrom)
{
    strmToReadFrom.CheckForStartMarker(CID_FILE, CID_LINE);

    // Check the format version
    tCIDLib::TCard2 c2FmtVersion;
    strmToReadFrom >> c2FmtVersion;
    if (!c2FmtVersion || (c2FmtVersion > CQCKit_ConnCfg::c2FmtVersion))
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

    // We don't have any data yet

    strmToReadFrom.CheckForEndMarker(CID_FILE, CID_LINE);
}


tCIDLib::TVoid TCQCConnCfg::StreamTo(TBinOutStream& strmToWriteTo) const
{
    strmToWriteTo   << tCIDLib::EStreamMarkers::StartObject
                    << CQCKit_ConnCfg::c2FmtVersion

                    // We don't have any data yet

                    << tCIDLib::EStreamMarkers::EndObject;
}

