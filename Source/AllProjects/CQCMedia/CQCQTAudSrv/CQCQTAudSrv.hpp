//
// FILE NAME: CQCQTAudSrv.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/11/2007
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
//  This is the main header of the facility. This facility is just a very
//  simple ORB server program that implements a Quick Time audio player.
//  Because QT isn't multi-threadable in Windows, in order to support
//  multiple instances, the QT audio player class (in CQCMedia) can't run
//  the player on an internal thread. Instead it spawns off an instance of
//  this process and talks to it via an ORB interface.
//
//  Since we are only used by a single program in a specialized way, we don't
//  register in the name server. Instead we open a named shared memory buffer
//  and store the object id there for the using program to get to directly.
//
//  We also store current playback status info there, so that the other
//  program doesn't need the overhead of the ORB calls just to get basic
//  info from us. That also requires that we create a named mutex so that
//  we can sync between our local threads and the other program's threads.
//
//  All of these resources are named using a unique name that the other
//  program must provide us as a startup parm.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDOrbUC.hpp"

#include    "CQCKit.hpp"

#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Bring in the SDK headers
// ---------------------------------------------------------------------------
#include    <QTML.h>
#include    <movies.h>
#include    <sound.h>



// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCQTAudSrv_ErrorIds.hpp"
#include    "CQCQTAudSrv_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCQTAudSrv
{
}


// ---------------------------------------------------------------------------
//  And now sub-include other headerss
// ---------------------------------------------------------------------------
#include    "CQCQTAudSrv_QTCtrlIntfServerBase.hpp"
#include    "CQCQTAudSrv_QTCtrlIntf.hpp"
#include    "CQCQTAudSrv_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCQTAudSrv    facCQCQTAudSrv;

