//
// FILE NAME: GenProtoS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/24/2003
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
//  This is the main header for the generic protocol server driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CIDComm.hpp"
#include    "CIDEncode.hpp"
#include    "CIDSock.hpp"

#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"



// ---------------------------------------------------------------------------
//  Set up our import/export attributes
//
//  Note that, unlike most drivers which don't export anything but the
//  driver entry point, we export some stuff because we have a test harness
//  that loads us for protocol development purposes, and it needs to access
//  some stuff inside here.
// ---------------------------------------------------------------------------
#if     defined(PROJ_GENPROTOS)
#define GENPROTOSEXPORT CID_DLLEXPORT
#else
#define GENPROTOSEXPORT CID_DLLIMPORT
#endif



// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "GenProtoS_ErrorIds.hpp"
#include    "GenProtoS_MessageIds.hpp"
#include    "GenProtoS_Constant.hpp"
#include    "GenProtoS_Type.hpp"

#include    "GenProtoS_DebugCallback.hpp"
#include    "GenProtoS_FldValue.hpp"
#include    "GenProtoS_ExprValue.hpp"
#include    "GenProtoS_Maps.hpp"
#include    "GenProtoS_Expression.hpp"
#include    "GenProtoS_FldInfo.hpp"
#include    "GenProtoS_ProtoInfo.hpp"
#include    "GenProtoS_Variable.hpp"
#include    "GenProtoS_StoreOp.hpp"
#include    "GenProtoS_Query.hpp"
#include    "GenProtoS_Reply.hpp"
#include    "GenProtoS_MsgMatch.hpp"
#include    "GenProtoS_StateMachine.hpp"
#include    "GenProtoS_CallResponse.hpp"
#include    "GenProtoS_DevConn.hpp"
#include    "GenProtoS_Context.hpp"

// ---------------------------------------------------------------------------
//  Some types used by headers below, that depend on headers above
// ---------------------------------------------------------------------------
namespace tGenProtoS
{
    // Some typedefs for some collections of data we keep
    typedef TKeyedHashSet<TGenProtoMap, TString, TStringKeyOps>     TMapList;
    typedef TKeyedHashSet<TGenProtoQuery, TString, TStringKeyOps>   TQueryList;
    typedef TKeyedHashSet<TGenProtoReply, TString, TStringKeyOps>   TReplyList;
    typedef TVector<TGenProtoCallRep>                               TCRList;
    typedef TKeyedHashSet<TGenProtoVarInfo, TString, TStringKeyOps> TVarList;
    typedef TVarList::TCursor                                       TVarCursor;
    typedef TRefVector<TGenProtoStateInfo>                          TStateList;
    typedef TRefVector<TGenProtoMsgMatch>                           TMatchList;
    typedef TVector<TGenProtoFldInfo>                               TFldList;
    typedef TFldList::TCursor                                       TFldCursor;
};

#include    "GenProtoS_DriverImpl.hpp"
#include    "GenProtoS_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export our facility lazy eval method, and a special one used by the test
//  harness.
// ---------------------------------------------------------------------------
extern GENPROTOSEXPORT TFacGenProtoS& facGenProtoS();
extern GENPROTOSEXPORT tCIDLib::TVoid MakeGenProtoSFac();

