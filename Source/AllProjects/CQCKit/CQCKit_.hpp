//
// FILE NAME: CQCKit_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/22/2000
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
//  This is the internal facility header
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Some underlying stuff that's only needed internally
// ---------------------------------------------------------------------------
#include    "CIDEncode.hpp"
#include    "CIDRegX.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDZLib.hpp"
#include    "CIDMacroEng.hpp"
#include    "CIDNet.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDPNG.hpp"
#include    "CIDImgFact.hpp"
#include    "CIDXML.hpp"
#include    "CIDMath.hpp"


// ---------------------------------------------------------------------------
//  Bring in our internal headers
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
//  Since we really need to minimize public headers on this fundamental
//  facility, any headers that are actually public but only required by a
//  small number of specialized facilities, we bring in internally and let
//  them include those headers directly themselves.
// ---------------------------------------------------------------------------
#include    "CQCKit_VerInstallInfo.hpp"


// ---------------------------------------------------------------------------
//  A macro to do a debug only sanity check of element names when we are
//  parsing XML based config info.
// ---------------------------------------------------------------------------
#if CID_DEBUG_ON
#define AssertElemName(xtn,nam) \
if (xtn.strQName() != nam) \
{ \
    facCQCKit().ThrowErr \
    ( \
        CID_FILE \
        , CID_LINE \
        , kKitErrs::errcCfg_ExpectedElem \
        , tCIDLib::ESeverities::Failed \
        , tCIDLib::EErrClasses::Format \
        , TString(nam) \
        , xtn.strQName() \
    ); \
}
#else
#define AssertElemName(xtn,nam) {}
#endif

