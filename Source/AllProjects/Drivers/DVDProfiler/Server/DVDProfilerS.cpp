//
// FILE NAME: DVDProfilerS.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/16/2005
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
//  This is the main implementation file for a media driver that provides
//  disk based browsing of a DVD Profiler repository, via the exported XML
//  library file.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "DVDProfilerS.hpp"



namespace DVDProfilerS
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacWeatherS
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TFacility* pfacDVDProfilerS = nullptr;
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacility& facDVDProfilerS()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *DVDProfilerS::pfacDVDProfilerS;
}


// ---------------------------------------------------------------------------
//  Global function that the server framework dynamically finds and uses to
//  get us to create an instance of our driver.
// ---------------------------------------------------------------------------
extern "C" CID_FNEXPORT TCQCServerBase*
psdrvMakeNewSrv(const TCQCDriverObjCfg& cqcdcToLoad)
{
    //
    //  Sanity check that the passed config object matches our known client
    //  side driver name.
    //
    facCQCKit().ValidateDrvCfg
    (
        tCIDLib::ECSSides::Server
        , cqcdcToLoad
        , kDVDProfilerS::pszServerFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facilty object is always there and doesn't have to be faulted
    //  in.
    //
    DVDProfilerS::pfacDVDProfilerS = new TFacility
    (
        kDVDProfilerS::pszServerFacName
        , tCIDLib::EModTypes::SharedLib
        , kDVDProfilerS::c4MajVersion
        , kDVDProfilerS::c4MinVersion
        , kDVDProfilerS::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    );

    // And now gen up an instance of our driver and return it
    return new TDVDProfilerSDriver(cqcdcToLoad);
}


