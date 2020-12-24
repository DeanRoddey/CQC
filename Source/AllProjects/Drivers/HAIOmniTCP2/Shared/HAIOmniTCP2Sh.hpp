//
// FILE NAME: HAIOmniTCPSh.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/16/2008
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
#if     defined(PROJ_HAIOMNITCP2SH)
#define HAIOMNITCP2SHEXP  CID_DLLEXPORT
#else
#define HAIOMNITCP2SHEXP  CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Bring in the stuff we need
// ---------------------------------------------------------------------------
#include "CIDLib.hpp"
#include "CIDOrbUC.hpp"

#include "CQCKit.hpp"
#include "CQCDriver.hpp"


// ---------------------------------------------------------------------------
//  Constants
// ---------------------------------------------------------------------------
namespace kHAIOmniTCP2Sh
{
    // -----------------------------------------------------------------------
    //  Our overall configuration version
    // -----------------------------------------------------------------------
    const   tCIDLib::TCard4 c4ConfigVer = 1;


    // -----------------------------------------------------------------------
    //  The base name of our facility
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszClientFacName = L"HAIOmniTCP2C";
    const tCIDLib::TCh* const   pszServerFacName = L"HAIOmniTCP2S";
}


// ---------------------------------------------------------------------------
//  Bring in our own public headers
// ---------------------------------------------------------------------------
#include    "HAIOmniTCP2Sh_ErrorIds.hpp"
#include    "HAIOmniTCP2Sh_Shared.hpp"
#include    "HAIOmniTCP2Sh_Item.hpp"
#include    "HAIOmniTCP2Sh_DrvConfig.hpp"


// ---------------------------------------------------------------------------
//  The lazy eval method for our facility object
// ---------------------------------------------------------------------------
extern HAIOMNITCP2SHEXP TFacility& facHAIOmniTCP2Sh();

