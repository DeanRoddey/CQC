//
// FILE NAME: CQCWebRIVA.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/29/2017
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
//  This is the main header of the web server's loadable engine for supporting RIVA
//  cients via the web server. This replaces the old dedicated RIVA server.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCWEBRIVA)
#define CQCWEBRIVAEXP CID_DLLEXPORT
#else
#define CQCWEBRIVAEXP CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDCrypto.hpp"
#include    "CIDEncode.hpp"
#include    "CIDSock.hpp"
#include    "CIDNet.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDMacroEng.hpp"
#include    "CIDCtrls.hpp"
#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCIntfEng.hpp"



// ---------------------------------------------------------------------------
//  Facilty types
// ---------------------------------------------------------------------------
namespace tCQCWebRIVA
{
    // -----------------------------------------------------------------------
    //  Used to pass mouse events into our gesture handler
    // -----------------------------------------------------------------------
    enum class EGestEvs
    {
        Press
        , Release
        , Move
    };
}


// ---------------------------------------------------------------------------
//  Facilty constants
// ---------------------------------------------------------------------------
namespace kCQCWebRIVA
{
    // -----------------------------------------------------------------------
    //  If client side lat/long info hasn't been set, these default values will
    //  still be there.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TFloat8  f8LocNotSet(-10000.0);
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCWebRIVA_ErrorIds.hpp"
#include    "CQCWebRIVA_MessageIds.hpp"

#include    "CQCWebRIVA_Shared.hpp"
#include    "CQCWebRIVA_Proto.hpp"
#include    "CQCWebRIVA_ImgMap.hpp"
#include    "CQCWebRIVA_WorkerIntf.hpp"
#include    "CQCWebRIVA_GraphDev.hpp"
#include    "CQCWebRIVA_View.hpp"
#include    "CQCWebRIVA_GestEngine.hpp"
#include    "CQCWebRIVA_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCWEBRIVAEXP TFacCQCWebRIVA& facCQCWebRIVA();
