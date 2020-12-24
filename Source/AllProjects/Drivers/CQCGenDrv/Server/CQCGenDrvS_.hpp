//
// FILE NAME: CQCGenDrvS_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/09/2003
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
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our own header for internal use
// ---------------------------------------------------------------------------
#include "CQCGenDrvS.hpp"


// ---------------------------------------------------------------------------
//  Any underlying stuff we only need internally. For the app control part of the
//  driver we have to bring in the low level part of CIDCtrls' types header, to
//  get the extended key enum.
// ---------------------------------------------------------------------------
#if         !defined(CIDKERNEL_TYPE_CTRLS)
#include    "CIDKernel_Type_Ctrls.hpp"
#endif


// ---------------------------------------------------------------------------
//  And any internal headers
// ---------------------------------------------------------------------------
#include    "CQCGenDrvS_MessageIds.hpp"
#include    "CQCGenDrvS_AppCtrl_.hpp"
#include    "CQCGenDrvS_ClassLoader_.hpp"


// ---------------------------------------------------------------------------
//  Some internal constants stuff
// ---------------------------------------------------------------------------
namespace kCQCGenDrvS_
{
}


