//
// FILE NAME: CQCHostMonC.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/21/2000
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
//  This is the main implementation file for the host monitor driver. It
//  provides the lazy eval method for our facility object, and it provides
//  the exported global function that is dynamically loaded by the client
//  framework.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCHostMonC.hpp"


namespace
{
    namespace CQCHostMonC
    {
        // -----------------------------------------------------------------------
        //  Local data
        //
        //  pfacHostMonC
        //      Our facility object pointer.
        // -----------------------------------------------------------------------
        TGUIFacility* pfacHostMonC = nullptr;
    }
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TGUIFacility& facHostMonC()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *CQCHostMonC::pfacHostMonC;
}


//
//  Note that we unconditionally export this function. It will be loaded
//  dynamically by the client framework.
//
extern "C" CID_FNEXPORT TCQCDriverClient*
MakeClientWindow(       TWindow&            wndParent
                , const TArea&              areaInit
                , const tCIDCtrls::TWndId   widToUse
                , const TCQCDriverObjCfg&   cqcdcToLoad
                , const TCQCUserCtx&        cuctxToUse)
{
    //
    //  Sanity check that the passed config object matches our known client
    //  side driver name.
    //
    facCQCKit().ValidateDrvCfg
    (
        tCIDLib::ECSSides::Client
        , cqcdcToLoad
        , kHostMonC::pszClientFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facilty object is always there and doesn't have to be faulted
    //  in.
    //
    CQCHostMonC::pfacHostMonC = new TGUIFacility
    (
        kHostMonC::pszClientFacName
        , tCIDLib::EModTypes::SharedLib
        , kHostMonC::c4MajVersion
        , kHostMonC::c4MinVersion
        , kHostMonC::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    );

    //
    //  And create our client window, which does all of the work for the
    //  client side. We install a default focus handler on it and make it
    //  the client window of the passed frame.
    //
    THostMonCWnd* pwndClient = new THostMonCWnd(cqcdcToLoad, cuctxToUse);
    pwndClient->CreateClDrvWnd(wndParent, areaInit, widToUse);

    return pwndClient;
}


#if CID_DEBUG_ON
//
//  Make sure that our entry point has the correct signature by trying to
//  assign it to a dummy function pointer based on the prototype provided
//  CQCKit.
//
static tCQCIGKit::TClientDriverFuncPtr pfnDummy = &MakeClientWindow;
#endif

