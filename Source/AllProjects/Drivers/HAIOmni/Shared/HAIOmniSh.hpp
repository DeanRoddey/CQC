//
// FILE NAME: HAIOmniSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/13/2006
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
//  This is the public facility header. It defines and includes stuff that
//  is publically visible
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_HAIOMNISH)
#define HAIOMNISHEXPORT  CID_DLLEXPORT
#else
#define HAIOMNISHEXPORT  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CIDLib.hpp"

#include "CIDOrbUC.hpp"

#include "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kHAIOmniSh
{
    // The version of the config data we write out
    const   tCIDLib::TCard4 c4ConfigVer = 1;


    // -----------------------------------------------------------------------
    //  The base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName = L"HAIOmniC";
    const tCIDLib::TCh* const   pszServerFacName = L"HAIOmniS";
}


// ---------------------------------------------------------------------------
//  Bring in our own public headers. Some others have to come in below after
//  the types namespace.
// ---------------------------------------------------------------------------
#include    "HAIOmniSh_Shared.hpp"


// ---------------------------------------------------------------------------
//  Public facility types
// ---------------------------------------------------------------------------
namespace tHAIOmniSh
{
    //
    //  An array of boolean flags that we use to keep up with which of the
    //  items of a given type we are to support in the driver.
    //
    typedef TFundArray<tCIDLib::TBoolean>       TFlagArray;
    typedef TFundArray<tHAIOmniSh::EUnitTypes>  TPLCArray;
    typedef TFundArray<tHAIOmniSh::EZoneTypes>  TZoneArray;
}



// ---------------------------------------------------------------------------
//  Bring in some more headers that need the above types
// ---------------------------------------------------------------------------
#include    "HAIOmniSh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern HAIOMNISHEXPORT TFacility& facHAIOmniSh();

