//
// FILE NAME: BarcoCRTS.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/13/2002
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
//  This is the main implementation file for the Barco CRT projector server
//  side driver. It just provides the required function that the server
//  framework uses to ask us  to create an instance of our driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "BarcoCRTS.hpp"


namespace BarcoCRTS
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacBarcoCRTS
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TFacility* pfacBarcoCRTS = nullptr;
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacility& facBarcoCRTS()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *BarcoCRTS::pfacBarcoCRTS;
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
    //  side driver name, and that its maj/min version values match ours. The
    //  kit facility provides a method to do this.
    //
    facCQCKit().ValidateDrvCfg
    (
        tCIDLib::ECSSides::Server
        , cqcdcToLoad
        , kBarcoCRTS::pszServerFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this is called before anything else, this insures
    //  that the facility object is always there and doesn't have to be
    //  faulted in.
    //
    BarcoCRTS::pfacBarcoCRTS = new TFacility
    (
        kBarcoCRTS::pszServerFacName
        , tCIDLib::EModTypes::SharedLib
        , kBarcoCRTS::c4MajVersion
        , kBarcoCRTS::c4MinVersion
        , kBarcoCRTS::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    );

    // And now gen up an instance of our driver and return it
    return new TBarcoCRTSDriver(cqcdcToLoad);
}


