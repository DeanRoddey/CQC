//
// FILE NAME: CQCInst.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/04/2001
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
//  This is the main header of the CQC installer
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Include our underlying headers
// ---------------------------------------------------------------------------
#include    "CIDLib.hpp"
#include    "CIDCrypto.hpp"
#include    "CIDPNG.hpp"
#include    "CIDXML.hpp"
#include    "CIDCtrls.hpp"
#include    "CIDWUtils.hpp"
#include    "CIDSock.hpp"
#include    "CIDRegX.hpp"
#include    "CIDOrbUC.hpp"
#include    "CIDObjStore.hpp"

#include    "CQCKit.hpp"
#include    "CQCRemBrws.hpp"
#include    "CQCGKit.hpp"
#include    "CQCIGKit.hpp"
#include    "CQCIntfEng.hpp"
#include    "CQLogicSh.hpp"
#include    "CQCIR.hpp"


// ---------------------------------------------------------------------------
//  And we need access to the Windows specific registry and service stuff,
//  and we directly bring in some CQCKit headers that it doesn't publically
//  export since it's only needed in a few places.
// ---------------------------------------------------------------------------
#include    "CIDKernel_RegistryWin32.hpp"
#include    "CIDKernel_ServiceWin32.hpp"
#include    "CQCKit_VerInstallInfo.hpp"



// ---------------------------------------------------------------------------
//  Forward references
// ---------------------------------------------------------------------------
class   TMainFrameWnd;


// ---------------------------------------------------------------------------
//  And sub-include our fundamental headers first
// ---------------------------------------------------------------------------
#include    "CQCInst_ErrorIds.hpp"
#include    "CQCInst_MessageIds.hpp"
#include    "CQCInst_ResourceIds.hpp"



// ---------------------------------------------------------------------------
//  The types namespace for this facility
// ---------------------------------------------------------------------------
namespace tCQCInst
{
    // -----------------------------------------------------------------------
    //  The install status, which is set in the install info. It lets panels
    //  know what the install status is as we move through them and report
    //  changes.
    // -----------------------------------------------------------------------
    enum class EInstStatus
    {
        Setup
        , InProgress
        , Done
        , Failed
    };


    // -----------------------------------------------------------------------
    //  The installation process steps. We need to share these between the
    //  installation thread and the main window, so that the installation
    //  thread can post notifications indicating which steps it's starting
    //  on so that the main thread can display status messages and move the
    //  progress meter.
    // -----------------------------------------------------------------------
    enum class EInstSteps
    {
        Start
        , CreateTmpDir
        , CreateNewDirs
        , CopyBase
        , CopyClient
        , CopyCQCServer
        , CopyMaster
        , CopyGWServer
        , CopyWebServer
        , GenConfig
        , GenCmdFiles
        , UpgradeData
        , InstAppShell
        , InstClService
        , StartSwapDirs
        , SaveBin
        , SaveData
        , SaveNotes
        , SaveVer
        , NewBin
        , NewData
        , GenLinks
        , StartAppShell
        , StartClService

        , Recovering
        , Complete

        , Count
        , None
    };


    // -----------------------------------------------------------------------
    //  If no existing version is found, we ask the user to either confirm
    //  that no previous version was installed, or to find it for us, or to
    //  cancel. This enum is returned from that dialog box.
    // -----------------------------------------------------------------------
    enum class ENewInstRes
    {
        Cancel
        , Found
        , New
    };


    // -----------------------------------------------------------------------
    //  Returned from the eProcess() method of panels, to tell the main frame
    //  window's panel processing logic what to do.
    // -----------------------------------------------------------------------
    enum class EProcRes
    {
        Stay
        , Next
    };


    // -----------------------------------------------------------------------
    //  Used in the TInstQueryPacket class, to indicate the type of operation to do,
    //  and the overall result of a query type operation.
    // -----------------------------------------------------------------------
    enum class EQueryOps
    {
        None
        , MsgBox
        , ShowWarning
        , ShowFatalErr
    };

    enum class EQueryRes
    {
        Success
        , Exception
        , UnknownException
    };


    // -----------------------------------------------------------------------
    //  This is returned by a method that checks the target directory to see
    //  if it doesn't exist, exists with a previous version we can upgrade,
    //  exists with a version we cannot upgrade, or is just a refresh of the
    //  current version.
    // -----------------------------------------------------------------------
    enum class ETargetTypes
    {
        Clean
        , Revision
        , Upgrade
        , BadOldVer
        , Refresh
        , RetroVersion
    };


    // -----------------------------------------------------------------------
    //  Used in creation of all of the data directories
    // -----------------------------------------------------------------------
    struct TInstDirInfo
    {
        const tCIDLib::TCh* pszPath;
        tCIDLib::TCard4     c4Level;
    };


    // -----------------------------------------------------------------------
    //  An array of card4 values, which is used to check for reuse of ports
    // -----------------------------------------------------------------------
    using TPortCntList = TFundArray<tCIDLib::TCard4>;
}


// ---------------------------------------------------------------------------
//  Some headers that need to see the types above, but might be needed by some
//  of the types below.
// ---------------------------------------------------------------------------
#include    "CQCInst_Constants.hpp"
#include    "CQCInst_Types.hpp"
#include    "CQCInst_StatusPacket.hpp"


// ---------------------------------------------------------------------------
//  A continuation of the types namespace that need to see something from
//  the header above.
// ---------------------------------------------------------------------------
namespace tCQCInst
{
    // -----------------------------------------------------------------------
    //  An alias for our status queue, which the main frame creates so that
    //  the installer thread can post status packets to.
    // -----------------------------------------------------------------------
    using TStatusQ = TQueue<TCQCStatusPacket>;
}


// ---------------------------------------------------------------------------
//  The facility constants namespace
// ---------------------------------------------------------------------------
namespace kCQCInst
{
    // -----------------------------------------------------------------------
    //  The current version we are installing, as separate values. Since the
    //  environment drives the version built, we can use them for our settings
    //  also, to automatically update when the build is changed.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard4   c4MajVersion    = CID_MAJVER;
    constexpr tCIDLib::TCard4   c4MinVersion    = CID_MINVER;
    constexpr tCIDLib::TCard4   c4Revision      = CID_REVISION;


    // -----------------------------------------------------------------------
    //  The oldest version we can upgrade from is now 5.4.917, which is the post
    //  open source upgrade version.
    // -----------------------------------------------------------------------
    constexpr tCIDLib::TCard8   c8MinUpgradeVer     = 0x0005000400000395;


    // -----------------------------------------------------------------------
    //  The installation version file
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const   pszInstVerFile  = L"CQCInstVersion.Info";


    // -----------------------------------------------------------------------
    //  The message id used by the background thread to post query packets to
    //  the foreground thread.
    // -----------------------------------------------------------------------
    constexpr tCIDCtrls::TWndMsg    wmsgQueryId     = 1000;


    // -----------------------------------------------------------------------
    //  The language runtime libraries to copy, for the version of Visual C++ we
    //  currently are being built on, and the prod/debug mode.
    // -----------------------------------------------------------------------
    #if CID_DEBUG_ON
    const tCIDLib::TCh* const   pszLRT_Lib1(L"vcruntime140d.dll");
    #else
    const tCIDLib::TCh* const   pszLRT_Lib1(L"vcruntime140.dll");
    #endif


    // -----------------------------------------------------------------------
    //  These have been changed. Pre-5.0 they were per-CQCServer instance and stored
    //  on the local config server. Now they have been moved to the master server. This
    //  is the old scope we need to move them from.
    // -----------------------------------------------------------------------
    const tCIDLib::TCh* const pszOSKey_OldCQCSrvDrivers    = L"/CQC/CQCServer/Drivers";
}


// ---------------------------------------------------------------------------
//  And now sub-include our other headers, in the correct order
// ---------------------------------------------------------------------------
#include    "CQCInst_QueryPacket.hpp"
#include    "CQCInst_InstallInfo.hpp"
#include    "CQCInst_Warning.hpp"

#include    "CQCInst_InfoPanel.hpp"
#include    "CQCInst_InstModePanel.hpp"
#include    "CQCInst_AgreePanel.hpp"
#include    "CQCInst_StopSrvPanel.hpp"
#include    "CQCInst_BackupPanel.hpp"
#include    "CQCInst_IntroPanel.hpp"
#include    "CQCInst_PathPanel.hpp"
#include    "CQCInst_CertInfoPanel.hpp"
#include    "CQCInst_ClientPanel.hpp"
#include    "CQCInst_IVCtrlPanel.hpp"
#include    "CQCInst_CQCVoicePanel.hpp"
#include    "CQCInst_LogicSrvPanel.hpp"
#include    "CQCInst_MSPanel.hpp"
#include    "CQCInst_MSPortsPanel.hpp"
#include    "CQCInst_PrereqPanel.hpp"
#include    "CQCInst_CQCSrvPanel.hpp"
#include    "CQCInst_XMLGWSPanel.hpp"
#include    "CQCInst_TrayMonPanel.hpp"
#include    "CQCInst_EventPanel.hpp"
#include    "CQCInst_WebSrvPanel.hpp"
#include    "CQCInst_CheckMSPanel.hpp"
#include    "CQCInst_SummaryPanel.hpp"
#include    "CQCInst_InstallThread.hpp"
#include    "CQCInst_SrvModePanel.hpp"
#include    "CQCInst_StatusPanel.hpp"
#include    "CQCInst_SuccessPanel.hpp"
#include    "CQCInst_SecurityPanel.hpp"
#include    "CQCInst_InitPWPanel.hpp"
#include    "CQCInst_CQCVoiceAcctPanel.hpp"
#include    "CQCInst_NewCheckDlg.hpp"
#include    "CQCInst_StopGUIAppsPanel.hpp"
#include    "CQCInst_MainFrame.hpp"
#include    "CQCInst_ThisFacility.hpp"


// ---------------------------------------------------------------------------
//  Make the facility object visible to all the program files. It is actually
//  declared in the main cpp file.
// ---------------------------------------------------------------------------
extern TFacCQCInst    facCQCInst;

