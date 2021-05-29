//
// FILE NAME: CQCHostMonS.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/13/2000
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
//  This is the main implementation file for the host monitor host side
//  driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCHostMonS.hpp"


namespace
{
    namespace CQCHostMonS
    {
        // -----------------------------------------------------------------------
        //  Local data
        //
        //  pfacHostMonS
        //      Our facility object pointer
        // -----------------------------------------------------------------------
        TFacility* pfacHostMonS = nullptr;
    }
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacility& facHostMonS()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *CQCHostMonS::pfacHostMonS;
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
        , kHostMonC::pszServerFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facilty object is always there and doesn't have to be faulted
    //  in.
    //
    CQCHostMonS::pfacHostMonS = new TFacility
    (
        kHostMonC::pszServerFacName
        , tCIDLib::EModTypes::SharedLib
        , kHostMonC::c4MajVersion
        , kHostMonC::c4MinVersion
        , kHostMonC::c4Revision
        , tCIDLib::EModFlags::HasMsgFile
    );

    // And now gen up an instance of our driver and return it
    return new TCQCHostMonSDriver(cqcdcToLoad);
}

