//
// FILE NAME: IRClient.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/23/2002
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
//  This is the main implementation file for the RedRat client side driver.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "IRClient.hpp"


namespace IRClient
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacHostMonC
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TGUIFacility* pfacIRClient = nullptr;
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TGUIFacility& facIRClient()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *IRClient::pfacIRClient;
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
        , kIRClient::pszFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this is called before anything else, this insures
    //  that the facilty object is always there and doesn't have to be faulted
    //  in lazily, which is normally done.
    //
    IRClient::pfacIRClient = new TGUIFacility
    (
        kIRClient::pszFacName
        , tCIDLib::EModTypes::SharedLib
        , kIRClient::c4MajVersion
        , kIRClient::c4MinVersion
        , kIRClient::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    );

    //
    //  And create our client window. It derives from a standard client side
    //  driver window class that does most of the work. Our derivative just
    //  needs to field some virtual methods.
    //
    TIRClientWnd* pwndClient = new TIRClientWnd(cqcdcToLoad, cuctxToUse);
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


