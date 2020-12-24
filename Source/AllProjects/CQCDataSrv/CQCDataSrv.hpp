//
// FILE NAME: CQCDataSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/23/2002
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
//  This is the main header of the CQC Data Server. This server provides
//  one primary service, which is to act as a centralized storage server for
//  various types of CQC data, such as macros or drawn interfaces.
//
//  Each major category of data has its own ORB interface, so we implement
//  multiple server side objects. This separation is designed to allow us
//  to move any of them to another server if needed, without affecting any
//  clients.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
//
//  In order to reduce the cost of CQCKit, it doesn't publically export all
//  of its headers if they are only used in a small number of places, so we
//  import a few of them directly that we use.
// ---------------------------------------------------------------------------
#include    "CIDRegX.hpp"
#include    "CIDMath.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDZLib.hpp"

#include    "CIDXML.hpp"
#include    "CQCKit.hpp"
#include    "CQCIR.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCSysCfg.hpp"
#include    "CQCSrvFW.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our core headers in the needed order
// ---------------------------------------------------------------------------
#include    "CQCDataSrv_ErrorIds.hpp"
#include    "CQCDataSrv_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Constants namespace
// ---------------------------------------------------------------------------
namespace kCQCDataSrv
{
    // -----------------------------------------------------------------------
    //  Facility constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4DrvModulus    = 23;


    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvDataSrv    = L"CQCDataSrv";


    // -----------------------------------------------------------------------
    //  The number of days for an initial trial mode installation, and for
    //  when we drop back due to an upgrade or system hardware change.
    //  Purposefully obscure for security reasons.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4TMDs = 40;
    const tCIDLib::TCard4   c4RMDs = 8;
}



// ---------------------------------------------------------------------------
//  And sub-include our higher level headers in the needed order
// ---------------------------------------------------------------------------
#include    "CQCDataSrv_InstServerBase.hpp"
#include    "CQCDataSrv_InstServerImpl.hpp"
#include    "CQCDataSrv_SecurityChallenge.hpp"
#include    "CQCDataSrv_SecurityServerBase.hpp"
#include    "CQCDataSrv_SecurityServerImpl.hpp"
#include    "CQCDataSrv_IRServerBase.hpp"
#include    "CQCDataSrv_IRServerImpl.hpp"
#include    "CQCDataSrv_FileAccServerBase.hpp"
#include    "CQCDataSrv_FileAccServerImpl.hpp"
#include    "CQCDataSrv_SysCfgServerBase.hpp"
#include    "CQCDataSrv_SysCfgSrvImpl.hpp"

#include    "CQCDataSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCDataSrv    facCQCDataSrv;


