//
// FILE NAME: ZoomPlayerS.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/16/2005
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
//  This is the main header file for a the Zoom Player driver.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//




// ---------------------------------------------------------------------------
//  Include underlying needed headers.
// ---------------------------------------------------------------------------
#include    "CQCKit.hpp"
#include    "CQCDriver.hpp"
#include    "CQCMedia.hpp"


// ---------------------------------------------------------------------------
//  Include our own header files in the needed order
// ---------------------------------------------------------------------------
#include    "ZoomPlayerS_ErrorIds.hpp"
#include    "ZoomPlayerS_MessageIds.hpp"


// ---------------------------------------------------------------------------
//  Facility types
// ---------------------------------------------------------------------------
namespace tZoomPlayerS
{
    // -----------------------------------------------------------------------
    //  We have to track the media mode since some commands mean different
    //  things according to the mode. These are set up so that they have the
    //  same values as the actual numbers used by ZP.
    // -----------------------------------------------------------------------
    enum EPlayModes
    {
        EMode_DVD
        , EMode_Media
        , EMode_Audio
    };
}


// ---------------------------------------------------------------------------
//  Some more of our headers that need to see the types and constants
// ---------------------------------------------------------------------------
#include    "ZoomPlayerS_DriverImpl.hpp"



// ---------------------------------------------------------------------------
//  Facility constants
// ---------------------------------------------------------------------------
namespace kZoomPlayerS
{
    // -----------------------------------------------------------------------
    //  General constants
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszServerFacName    = L"ZoomPlayerS";


    // -----------------------------------------------------------------------
    //  The text for the screen mode field's values
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszScrModeMap[] =
    {
        L"Maximize"
        , L"Minimize"
        , L"To Front"
        , L"Toggle FS"
        , L"Full Screen"
    };
    const tCIDLib::TCard4 c4MaxScrMode = tCIDLib::c4ArrayElems(apszScrModeMap);


    // -----------------------------------------------------------------------
    //  The text for the navigation field's values
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const apszNavMap[] =
    {
        L"Down"
        , L"Eject"
        , L"Enter"
        , L"Exit"
        , L"Left"
        , L"MediaNext"
        , L"MediaPrev"
        , L"NextChapter"
        , L"PrevChapter"
        , L"Right"
        , L"RootMenu"
        , L"TitleMenu"
        , L"Up"
    };
    const tCIDLib::TCard4 c4MaxNav = tCIDLib::c4ArrayElems(apszNavMap);


    // -----------------------------------------------------------------------
    //  Zoom player command and query ids
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4Cmd_AppName           = 0;
    const tCIDLib::TCard4   c4Cmd_AppVersion        = 1;
    const tCIDLib::TCard4   c4Cmd_StateChange       = 1000;
    const tCIDLib::TCard4   c4Cmd_TimeLine          = 1100;
    const tCIDLib::TCard4   c4Cmd_TrackDuration     = 1110;
    const tCIDLib::TCard4   c4Cmd_TrackPosition     = 1120;
    const tCIDLib::TCard4   c4Cmd_PlayModeChanged   = 1300;
    const tCIDLib::TCard4   c4Cmd_AudioTrackName    = 1602;
    const tCIDLib::TCard4   c4Cmd_PlayFile          = 1850;

    const tCIDLib::TCard4   c4Cmd_EndOfTrack        = 1855;
    const tCIDLib::TCard4   c4Cmd_PlayListIndChange = 1900;
    const tCIDLib::TCard4   c4Cmd_SetPlayListInd    = 1910;
    const tCIDLib::TCard4   c4Cmd_ClearPlayList     = 1920;
    const tCIDLib::TCard4   c4Cmd_AddToPlayList     = 1930;
    const tCIDLib::TCard4   c4Cmd_DelPlayListItem   = 1941;
    const tCIDLib::TCard4   c4Cmd_ARChanged         = 2210;
    const tCIDLib::TCard4   c4Cmd_Volume            = 2300;
    const tCIDLib::TCard4   c4Cmd_SetAR             = 2600;
    const tCIDLib::TCard4   c4Cmd_Function          = 5100;
    const tCIDLib::TCard4   c4Cmd_ExFunction        = 5110;
    const tCIDLib::TCard4   c4Cmd_ScanCode          = 5120;

    const tCIDLib::TCard4   c4Query_ARMode          = 2200;
    const tCIDLib::TCard4   c4Query_AudioTrackNames = 1602;
    const tCIDLib::TCard4   c4Query_PlayState       = 1000;
    const tCIDLib::TCard4   c4Query_Volume          = 2300;


    // -----------------------------------------------------------------------
    //  Strings for the function names (sent through 5100 command)
    // -----------------------------------------------------------------------
    const TString   strFunc_Down(L"fnDVDMenuDown");
    const TString   strFunc_Eject(L"fnEject");
    const TString   strFunc_Enter(L"fnDVDMenuSelect");
    const TString   strFunc_Exit(L"fnExit");
    const TString   strFunc_FF(L"fnFastForward");
    const TString   strFunc_FullScreen(L"fnFullScreen");
    const TString   strFunc_Left(L"fnDVDMenuLeft");
    const TString   strFunc_Maximize(L"fnMax");
    const TString   strFunc_MediaNext(L"fnNextChapter");
    const TString   strFunc_MediaPrev(L"fnPrevChapter");
    const TString   strFunc_Minimize(L"fnMinimize");
    const TString   strFunc_Next(L"fnNextTrack");
    const TString   strFunc_NextAngle(L"fnDVDAngle");
    const TString   strFunc_NextAudio(L"fnAudioTrack");
    const TString   strFunc_NextSubtitle(L"fnDVDSub");
    const TString   strFunc_Pause(L"fnPause");
    const TString   strFunc_Play(L"fnPlay");
    const TString   strFunc_Prev(L"fnPrevTrack");
    const TString   strFunc_RandomPlay(L"fnRandomPlay");
    const TString   strFunc_Rewind(L"fnRewind");
    const TString   strFunc_Right(L"fnDVDMenuRight");
    const TString   strFunc_RootMenu(L"fnDVDRootMenu");
    const TString   strFunc_Stop(L"fnStop");
    const TString   strFunc_TitleMenu(L"fnDVDTitleMenu");
    const TString   strFunc_ToFront(L"fnBringToFront");
    const TString   strFunc_Up(L"fnDVDMenuUp");


    // -----------------------------------------------------------------------
    //  Strings for the ex function names (sent through 5110 command)
    // -----------------------------------------------------------------------
    const TString   strExFunc_Interface(L"exInterface");
    const TString   strExFunc_Volume(L"exSetVolume");


    // -----------------------------------------------------------------------
    //  The version info for this driver, which we just set to the CQC
    //  version.
    // -----------------------------------------------------------------------
    const tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    const tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    const tCIDLib::TCard4   c4Revision      = CID_REVISION;
}



// ---------------------------------------------------------------------------
//  Global functions. These aren't exported. They are just for internal use.
// ---------------------------------------------------------------------------
extern TFacility& facZoomPlayerS();

