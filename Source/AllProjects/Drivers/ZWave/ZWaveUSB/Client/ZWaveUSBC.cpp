//
// FILE NAME: ZWaveUSBC.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/25/2001
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
//  This is the main implementation file for the ZWave client driver
//  facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "ZWaveUSBC_.hpp"


namespace ZWaveUSBC
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacZWaveUSBC
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TGUIFacility* pfacZWaveUSBC = nullptr;


    // -----------------------------------------------------------------------
    //  The base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName = L"ZWaveUSBC";
}


// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TGUIFacility& facZWaveUSBC()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *ZWaveUSBC::pfacZWaveUSBC;
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
        tCIDLib::ECSSides::Client, cqcdcToLoad, ZWaveUSBC::pszClientFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facilty object is always there and doesn't have to be faulted
    //  in.
    //
    ZWaveUSBC::pfacZWaveUSBC = new TGUIFacility
    (
        ZWaveUSBC::pszClientFacName
        , tCIDLib::EModTypes::SharedLib
        , kCQCKit::c4MajVersion
        , kCQCKit::c4MinVersion
        , kCQCKit::c4Revision
        , tCIDLib::EModFlags::HasMsgsAndRes
    );

    //
    //  And create our client window, which does all of the work for the
    //  client side.
    //
    TZWaveUSBCWnd* pwndClient = new TZWaveUSBCWnd(cqcdcToLoad, cuctxToUse);
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
