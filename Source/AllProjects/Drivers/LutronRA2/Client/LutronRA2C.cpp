//
// FILE NAME: LutronRA2C.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2012
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
//  This is the main implementation file for the Lutron HW client driver
//  facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "LutronRA2C_.hpp"


// ---------------------------------------------------------------------------
//  Local types and constants
// ---------------------------------------------------------------------------
namespace LutronRA2C
{
    // -----------------------------------------------------------------------
    //  Local data
    //
    //  pfacLutronRA2C
    //      Our facility object pointer, which is lazily faulted in.
    // -----------------------------------------------------------------------
    TGUIFacility* pfacLutronRA2C = nullptr;


    // -----------------------------------------------------------------------
    //  The base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName = L"LutronRA2C";
}



// ---------------------------------------------------------------------------
//  Glocal functions
// ---------------------------------------------------------------------------
TGUIFacility& facLutronRA2C()
{
    //
    //  Normally, in a CIDLib facility, this would be a lazy eval method. But,
    //  since this is a driver, we can force it in below when we are loaded.
    //  So we can avoid the overhead of synchronization and checking whether
    //  it exists.
    //
    return *LutronRA2C::pfacLutronRA2C;
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
        tCIDLib::ECSSides::Client, cqcdcToLoad, LutronRA2C::pszClientFacName
    );

    //
    //  Create our facility object now. Since we *should* always be loaded
    //  dynamically and this called before anything else, this insures that
    //  the facilty object is always there and doesn't have to be faulted
    //  in.
    //
    LutronRA2C::pfacLutronRA2C = new TGUIFacility
    (
        LutronRA2C::pszClientFacName
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
    TLutronRA2CWnd* pwndClient = new TLutronRA2CWnd(cqcdcToLoad, cuctxToUse);
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


