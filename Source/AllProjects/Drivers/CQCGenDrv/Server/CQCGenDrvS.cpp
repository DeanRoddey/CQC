//
// FILE NAME: CQCGenDrvS.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
//  This is the main cpp file of the facility. It provides the lazy eval
//  method for our facility object, and some other odds and ends.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_.hpp"


namespace CQCGenDrvS
{
    namespace
    {
        // -----------------------------------------------------------------------
        //  Local data
        //
        //  pfacCQCGenDrv
        //      Our facility object pointer, which is allocated during the driver
        //      init call below. Normally, these are lazily faulted in, but in
        //      the case of a driver, initialization is formal so we can just gen
        //      it up at that time.
        // -----------------------------------------------------------------------
        TFacCQCGenDrvS*         pfacCQCGenDrvS  = nullptr;


        // -----------------------------------------------------------------------
        //  The version info for this driver (client and server sides, which we'll
        //  always keep in sync.) For all Charmed Quark drivers, their version
        //  is set to the product release version, for simplicity. So we set them
        //  to magic values provided by the CIDBuild build utility.
        // -----------------------------------------------------------------------
        const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
        const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
        const tCIDLib::TCard4   c4Revision      = CID_REVISION;
    }
}


// ---------------------------------------------------------------------------
//  Global functions
// ---------------------------------------------------------------------------
TFacCQCGenDrvS& facCQCGenDrvS()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    //  However, when running in the simulator, this doesn't happen, so
    //  we will fault it in if not already present.
    //
    if (!CQCGenDrvS::pfacCQCGenDrvS)
        CQCGenDrvS::pfacCQCGenDrvS = new TFacCQCGenDrvS;
    return *CQCGenDrvS::pfacCQCGenDrvS;
}


// ---------------------------------------------------------------------------
//  Exported function that the server framework dynamically finds and uses to
//  get us to create an instance of our driver. On the server side, drivers
//  are implemented directly as ORB objects, so it derives from TOrbServerBase
//  and the server framework will install us as a server side ORB object, and
//  our client side driver's client proxy will connect directly to us, by
//  passing the server framework. Generic field access goes indirectly
//  through the server framework, but our own client driver can talk directly
//  to use.
// ---------------------------------------------------------------------------
extern "C" CID_FNEXPORT TCQCServerBase*
psdrvMakeNewSrv(const TCQCDriverObjCfg& cqcdcToLoad)
{
    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this is called before anything else, this insures that
    //  the facility object is always there and doesn't have to be faulted
    //  in.
    //
    //  The facility object represents an instance of a DLL/SharedObj at
    //  runtime. Through this object we can get to our attached resources such
    //  as loadable text, and we also use it to throw or log errors because it
    //  provides extra info for us, and so on.
    //
    if (!CQCGenDrvS::pfacCQCGenDrvS)
        CQCGenDrvS::pfacCQCGenDrvS = new TFacCQCGenDrvS;

    //
    //  And now gen up an instance of our driver and return it. We pass it the
    //  driver configuration info and moniker that we were provided by the
    //  server framework. This object gets returned to the framework who will
    //  register it with the CIDLib ORB, because server drivers are basically
    //  just remote objects. The ORB will then own this object and clean it
    //  up at the appropriate time.
    //
    return new TCQCGenDrvGlue(cqcdcToLoad);
}


