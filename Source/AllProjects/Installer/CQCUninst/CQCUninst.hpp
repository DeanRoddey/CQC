//
// FILE NAME: CQCUninst.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/15/2003
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
//  This is the main header of the CQC uninstaller
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"

#include    "CQCKit.hpp"
#include    "CQCGKit.hpp"


// ---------------------------------------------------------------------------
//  We need the Windows specific registry and service stuff
// ---------------------------------------------------------------------------
#include    "CIDKernel_RegistryWin32.hpp"
#include    "CIDKernel_ServiceWin32.hpp"


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCUninst
{
    // -----------------------------------------------------------------------
    //  The current version we are installing, as separate values. Since the
    //  environment drives the version built, we can use them for our settings
    //  also, to automatically update when the build is changed.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4       c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4       c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4       c4Revision      = CID_REVISION;
}


// ---------------------------------------------------------------------------
//  The facility types namespace
// ---------------------------------------------------------------------------
namespace tCQCUninst
{
    // -----------------------------------------------------------------------
    //  The values posted from the background thread to the GUI thread to tell it the
    //  current step he's on and the status. He can only post them as ints, using the
    //  standard async data posting scheme, but we use this enum otherwise.
    //
    //  THESE MUST be in the order done since they are used to index the list box that
    //  shows the steps and statuses.
    // -----------------------------------------------------------------------
    enum ESteps
    {
        EStep_CleanRegistry
        , EStep_StopCSrv
        , EStep_StopSSrv
        , EStep_RemoveMenu
        , EStep_RemoveTrayMon
        , EStep_RemoveCQCVoice
        , EStep_GenDelCmd
        , EStep_Complete
    };


    // -----------------------------------------------------------------------
    //  An enum to indicate the status of each step above
    // -----------------------------------------------------------------------
    enum EStatus
    {
        EStatus_Starting
        , EStatus_Success
        , EStatus_Failure
    };


    // -----------------------------------------------------------------------
    //  An enum the main window uses to keep up with his state
    // -----------------------------------------------------------------------
    enum EStates
    {
        EState_WaitStart
        , EState_Running
        , EState_Success
        , EState_Failure
    };
}

// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCUninst_ErrorIds.hpp"
#include    "CQCUninst_MessageIds.hpp"
#include    "CQCUninst_ResourceIds.hpp"
#include    "CQCUninst_Constants.hpp"
#include    "CQCUninst_MainWnd.hpp"

extern TGUIFacility facCQCUninst;
