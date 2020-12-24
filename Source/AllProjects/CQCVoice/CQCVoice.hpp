//
// FILE NAME: CQCVoice.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/02/2016
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
//  This is the main header of the CQCVoice program. It does the usual things that a
//  CIDLib based facility header does, bringing in underlying headers needed for
//  our public interfaces, bringing in our own headers that need to be seen by all
//  or most of our cpp files, declaring the facility object, etc...
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDMath.hpp"
#include    "CIDImage.hpp"
#include    "CIDSpReco.hpp"
#include    "CIDXML.hpp"
#include    "CIDAI.hpp"
#include    "CIDOrbUC.hpp"
#include    "CQCKit.hpp"
#include    "CQCPollEng.hpp"
#include    "CQCAct.hpp"
#include    "CQCMedia.hpp"
#include    "CQCSysCfg.hpp"
#include    "CQCRemBrws.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"


// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCVoice_ErrorIds.hpp"
#include    "CQCVoice_MessageIds.hpp"
#include    "CQCVoice_ResourceIds.hpp"
#include    "CQCVoice_Internal.hpp"


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCVoice
{
    // -----------------------------------------------------------------------
    //  The name we give to our wait event. It has to be the same as what the
    //  installer generates for the app shell to use, and should generally be
    //  the same as the executable name.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszEvCQCVoice       = L"CQCVoice";


    // -----------------------------------------------------------------------
    //  The minimum values we will consider to be various levels of confidence
    // -----------------------------------------------------------------------
    const tCIDLib::TFloat4      f4MedConfidence     = 0.55F;
    const tCIDLib::TFloat4      f4HighConfidence    = 0.65F;
    const tCIDLib::TFloat4      f4FullConfidence    = 0.80F;


    // -----------------------------------------------------------------------
    //  How long we wait for a reply from the user. This is the conversational wait, i.e.
    //  the wait for responses to our questions, which should come pretty quickly.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4WaitReplyMSs      = 4000;


    // -----------------------------------------------------------------------
    //  This is how long we wait for another command before we automatically exit
    //  conversation mode.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4ConvoWaitMSs      = 12000;
}


// ---------------------------------------------------------------------------
//  The facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCVoice
{
    //
    //  An enum used as an index into a set of flags maintained by the facility
    //  class, and used by nodes to know if this or that is available, so we can
    //  just remember it during setup and not have to constantly re-check.
    //
    //  SystemCfg     - We were able to load system configuration data
    //  RoomData      - The room for which we are configured was present in the system
    //                  configuration, so we have room configuration info.
    //  MusicData     - Our room has music configured, and the indicated repo moniker
    //                  mapped to an available media DB.
    //  HVACData      - Our room data has a configured thermostat
    //  SecData       - Our room data has configured security information
    //
    //  RoomModes     - There is at least one room mode defined
    //
    //  These imply MusicData is true
    //
    //  MusicCats     - Our music repo has non-empty categories
    //  PlayLists     - Our music repo has at least one playlist defined.
    //
    //  These imply that SecData is true
    //
    //  ArmingCode    - A code was configured that we can use to arm the system
    //  ArmModes      - We were able to query the arming modes so we can do security
    //                  system arming.
    //  SecZones      - There is at least one security zone defined for this room
    //
    enum class ECapFlags
    {
        MusicCats
        , MusicData
        , HVACData
        , PlayLists
        , RoomData
        , RoomModes
        , SecArmingCode
        , SecArmModes
        , SecData
        , SecZones
        , SystemCfg

        , Count
    };


    //
    //  We have a method that waits for a response once we get into conversational mode.
    //  It can either get the expected response, timeout and give up, or it can get a
    //  cancel command from the user, or some failure. The caller needs to know which.
    //
    //  We might add others later, so either way it needs to return an enum.
    //
    enum class EWaitRepModes
    {
        Cancel
        , Failure
        , Timeout
        , Success
    };
}


// ---------------------------------------------------------------------------
//  Include our public headers
// ---------------------------------------------------------------------------
#include    "CQCVoice_CoreAdminImpl.hpp"
#include    "CQCVoice_BTBaseNodes.hpp"
#include    "CQCVoice_SysCfgIntfClientProxy.hpp"

#include    "CQCVoice_Reminder.hpp"

#include    "CQCVoice_TrayWnd.hpp"
#include    "CQCVoice_VisTextWnd.hpp"
#include    "CQCVoice_MainFrame.hpp"
#include    "CQCVoice_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCVoice    facCQCVoice;

