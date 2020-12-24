//
// FILE NAME: CQCRPortClient.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 09/14/2005
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
// CAVEATS/GOTCHAS:
//
//  This is the public header for this facility. This is a library that
//  supports the client side proxy for our remote serial port server. This
//  guy implements a derivative of TCommPortBase that uses an ORB interface
//  to talk to the server port server on the other side, making access to
//  remote serial ports transparent.
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Set up our import/export attributes
// ---------------------------------------------------------------------------
#if     defined(PROJ_CQCRPORTCLIENT)
#define CQCRPORTCEXPORT CID_DLLEXPORT
#else
#define CQCRPORTCEXPORT CID_DLLIMPORT
#endif


// ---------------------------------------------------------------------------
//  Include our needed underlying headers
// ---------------------------------------------------------------------------
#include    "CIDSock.hpp"
#include    "CIDOrb.hpp"
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCRPortClient_ChatClientProxy.hpp"
#include    "CQCRPortClient_SerialClientProxy.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tRPortClient
{

    // -----------------------------------------------------------------------
    //  A counted pointer to one of our client proxies. We provide a method
    //  in the facility class to make a proxy and it returns it in this
    //  form.
    // -----------------------------------------------------------------------
    typedef TCntPtr<TRemChatClientProxy>        TChatClient;
    typedef TCntPtr<TRemSerialSrvClientProxy>   TSerialPortClient;
}


// ---------------------------------------------------------------------------
//  Include our headers, in the needed order.
// ---------------------------------------------------------------------------
#include    "CQCRPortClient_PortFactory.hpp"
#include    "CQCRPortClient_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Export the facility object lazy eval method
// ---------------------------------------------------------------------------
extern CQCRPORTCEXPORT TFacCQCRPortClient& facCQCRPortClient();


