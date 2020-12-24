//
// FILE NAME: CQCWCHelper.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/05/2017
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
//  This is the main header of the web camera helper facility.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//



// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    <vlc/vlc.h>

#include    "CIDOrbUC.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCKit.hpp"
#include    "CQCGKit.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tCQCWCHelper
{
    //
    //  The types of message that incoming ORB calls will queue up for the window
    //  thread to process. We always replace an existing one on the queue if possible
    //  instead of posting another one. So there can only ever be one of each type in
    //  the queue. It makes no sense to two of the same type since whichever was last
    //  added is going to win no matter what.
    //
    enum class EGUIMsgTypes
    {
        None

        , Close
        , Init
        , SetArea
        , SetMute
        , SetVisibility

        , Count
    };

    //
    //  The types of message that incoming ORB calls will queue up for the audio processing
    //  thread to process. The GUI ones above just affect the window. These affect the
    //  player engine.
    //
    enum class EPlMsgTypes
    {
        None

        , SetMute
        , SetVolume

        , Count
    };


    //
    //  We are basically a little state machine, always trying to get ourselves up to
    //  the point where we are connected to the stream and displaying data.
    //
    enum class EStates
    {
        InitVLC
        , InitPlayer
        , WaitPlayback
        , Playing
        , Failed
        , Restarting
    };
}


// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kCQCWCHelper
{

    // -----------------------------------------------------------------------
    //  How long we will wait for activity from the invoking client before we give
    //  up and shut down.
    // -----------------------------------------------------------------------
    const tCIDLib::TEncodedTime enctTimeout = kCIDLib::enctOneSecond * 10;


    // -----------------------------------------------------------------------
    //  An id we post the main frame when new messages arrive and are queued up. He
    //  will grab them and process them.
    // -----------------------------------------------------------------------
    const tCIDLib::TInt4        i4NewMsg = 1000;
}


// ---------------------------------------------------------------------------
//  And sub-include our own headers in the correct order
// ---------------------------------------------------------------------------
#include    "CQCWCHelper_ErrorIds.hpp"
#include    "CQCWCHelper_MessageIds.hpp"
#include    "CQCWCHelper_CtrlServerBase.hpp"
#include    "CQCWCHelper_CtrlImpl.hpp"
#include    "CQCWCHelper_MainFrame.hpp"
#include    "CQCWCHelper_ThisFacility.hpp"



// ---------------------------------------------------------------------------
//  Some stuff that needs to be seen throughout this facility
// ---------------------------------------------------------------------------
extern TFacCQCWCHelper& facCQCWCHelper();
