//
// FILE NAME: CQCWebSrvC.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/17/2014
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
//  This is the main header of the CQC web Server client library facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCWEBSRVC)
#define CQCWEBSRVCEXP CID_DLLEXPORT
#else
#define CQCWEBSRVCEXP CID_DLLIMPORT
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
#include    "CQCKit.hpp"
#include    "CQCMedia.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCSysCfg.hpp"


// ---------------------------------------------------------------------------
//  Facilty types
// ---------------------------------------------------------------------------
namespace tCQCWebSrvC
{
    // -----------------------------------------------------------------------
    //  An enum used by the family of Websocket handlers, to indicate what type, so that
    //  we can do a lot of things without any RTTI stuff. CML means it's a user level
    //  handler written in CML. Otherwise it's one of our specialized C++ handlers.
    // -----------------------------------------------------------------------
    enum class EWSockTypes
    {
        CML
        , RIVA
    };


    // -----------------------------------------------------------------------
    //  This is how the listeners queue up incoming connections to be grabbed
    //  by one of the thread pool threads.
    // -----------------------------------------------------------------------
    using TConnQueue = TRefQueue<TServerStreamSocket>;
}


// ---------------------------------------------------------------------------
//  Facilty constants
// ---------------------------------------------------------------------------
namespace kCQCWebSrvC
{
    // -----------------------------------------------------------------------
    //  In web server mode, these are the msg type codes
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard1       c1WSockMsg_Cont         = 0;
    constexpr tCIDLib::TCard1       c1WSockMsg_Text         = 1;
    constexpr tCIDLib::TCard1       c1WSockMsg_Bin          = 2;
    constexpr tCIDLib::TCard1       c1WSockMsg_Close        = 8;
    constexpr tCIDLib::TCard1       c1WSockMsg_Ping         = 9;
    constexpr tCIDLib::TCard1       c1WSockMsg_Pong         = 0xA;


    // -----------------------------------------------------------------------
    //  Websocket errors we return. First are standard ones, and then some that we
    //  return for more specific errors, and a base value for per-subsystem errors.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard2       c2WSockErr_Normal       = 1000;
    constexpr tCIDLib::TCard2       c2WSockErr_Exiting      = 1001;
    constexpr tCIDLib::TCard2       c2WSockErr_Proto        = 1002;
    constexpr tCIDLib::TCard2       c2WSockErr_BadData      = 1003;
    constexpr tCIDLib::TCard2       c2WSockErr_TooBig       = 1009;
    constexpr tCIDLib::TCard2       c2WSockErr_CantDo       = 1011;

    constexpr tCIDLib::TCard2       c2WSockErr_BadFragMsg   = 4001;
    constexpr tCIDLib::TCard2       c2WSockErr_UnstartedCont= 4002;
    constexpr tCIDLib::TCard2       c2WSockErr_NonFinalCtrl = 4003;
    constexpr tCIDLib::TCard2       c2WSockErr_HandlerErr   = 4004;
    constexpr tCIDLib::TCard2       c2WSockErr_BadFragSz    = 4005;
    constexpr tCIDLib::TCard2       c2WSockErr_ResrvBits    = 4006;
    constexpr tCIDLib::TCard2       c2WSockErr_FragTooBig   = 4007;
    constexpr tCIDLib::TCard2       c2WSockErr_Timeout      = 4008;
    constexpr tCIDLib::TCard2       c2WSockErr_Nesting      = 4009;

    constexpr tCIDLib::TCard2       c2WSockErr_AppBase      = 5000;


    // -----------------------------------------------------------------------
    //  The maximum total msg size we'll accept. Anything larger than this and they
    //  have to break it into multiple messages themselves.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4       c4MaxWebsockMsgSz       = 1024 * 1024;


    // -----------------------------------------------------------------------
    //  The maximum size we'll send or accept in a single fragment. It's effectively
    //  the same as the maximum message size, since Websockets is very loose on this
    //  front. They don't have to fragment anything ultimately. We subtract a bit
    //  to account for fragment overhead.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4       c4MaxWebsockFragSz      = c4MaxWebsockMsgSz - 128;



    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvWebSrv     = L"CQCWebSrv";


    // -----------------------------------------------------------------------
    //  The initial number of worker threads we'll spin up
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    constexpr tCIDLib::TCard4   c4InitWorkerThreads = 1;
    #else
    constexpr tCIDLib::TCard4   c4InitWorkerThreads = 4;
    #endif
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCWebSrvC_ErrorIds.hpp"
#include    "CQCWebSrvC_MessageIds.hpp"

#include    "CQCWebSrvC_WebsockHandler.hpp"
#include    "CQCWebSrvC_WorkerThread.hpp"

#include    "CQCWebSrvC_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCWEBSRVCEXP TFacCQCWebSrvC& facCQCWebSrvC();


