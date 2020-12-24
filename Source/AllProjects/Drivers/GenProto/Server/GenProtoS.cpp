//
// FILE NAME: GenProtoS.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the main implementation file for the generic protocol server side
//  driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "GenProtoS_.hpp"


namespace GenProtoS
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacGenProtoS
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TFacGenProtoS* pfacGenProtoS = nullptr;
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TFacGenProtoS& facGenProtoS()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *GenProtoS::pfacGenProtoS;
}


//
//  This one is used by the test harness, to force in a facility object, since
//  it does not call the factory method below.
//
tCIDLib::TVoid MakeGenProtoSFac()
{
    GenProtoS::pfacGenProtoS = new TFacGenProtoS;
}


// ---------------------------------------------------------------------------
//  Global function that the server framework dynamically finds and uses to
//  get us to create an instance of our driver.
// ---------------------------------------------------------------------------
extern "C" CID_FNEXPORT TCQCServerBase*
psdrvMakeNewSrv(const  TCQCDriverObjCfg& cqcdcToLoad)
{
    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facility object is always there and doesn't have to be faulted
    //  in.
    //
    if (!GenProtoS::pfacGenProtoS)
        GenProtoS::pfacGenProtoS = new TFacGenProtoS;

    // And now gen up an instance of our driver and return it
    return new TGenProtoSDriver(cqcdcToLoad);
}

