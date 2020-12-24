//
// FILE NAME: ZWaveLeviSh.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/10/2012
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
//  This is the main implementation file for this facility, and just defines
//  a little facility global stuff.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveLeviSh_.hpp"



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------

// The lazy eval function that faults in the facility object
TFacility& facZWaveLeviSh()
{
    static TFacility* pfacThis = new TFacility
    (
        L"ZWaveLeviSh"
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    );
    return *pfacThis;
}
