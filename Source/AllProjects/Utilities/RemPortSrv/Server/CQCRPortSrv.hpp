//
// FILE NAME: CQCRPortSrv.hpp
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
//  This is the main header of the remote port server facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCKit.hpp"
#include    "CQCGKit.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCRPortSrv
{
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCRPortSrv
{
}


// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCRPortSrv_ErrorIds.hpp"
#include    "CQCRPortSrv_MessageIds.hpp"
#include    "CQCRPortSrv_ResourceIds.hpp"
#include    "CQCRPortSrv_ChatServerBase.hpp"
#include    "CQCRPortSrv_ChatImpl.hpp"
#include    "CQCRPortSrv_SerialPortInfo.hpp"
#include    "CQCRPortSrv_SerialServerBase.hpp"
#include    "CQCRPortSrv_SerialImpl.hpp"
#include    "CQCRPortSrv_MainFrame.hpp"
#include    "CQCRPortSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCRPortSrv& facCQCRPortSrv();


