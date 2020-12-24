//
// FILE NAME: CQCUninst.cpp
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
// CAVEATS/GOTCHAS:
//
//  This is the main implementation file of the CQC uninstaller program.
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCUninst.hpp"


// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
static tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
        , tCIDLib::TVoid*   pData
);


// ----------------------------------------------------------------------------
//  Magic main module code
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule(TThread(L"MainThread", eMainThreadFunc))


// ----------------------------------------------------------------------------
//  Local data
// ----------------------------------------------------------------------------
TGUIFacility facCQCUninst
(
    L"CQCUninst"
    , tCIDLib::EModTypes::Exe
    , kCQCUninst::c4MajVersion
    , kCQCUninst::c4MinVersion
    , kCQCUninst::c4Revision
    , tCIDLib::EModFlags::HasMsgsAndRes
);


// ----------------------------------------------------------------------------
//  Local functions
// ----------------------------------------------------------------------------
static tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    tCIDLib::TBoolean bFailures = kCIDLib::False;

    // We have to let our calling thread go first
    thrThis.Sync();

    //
    //  Create our main frame window and enter the message loop
    //
    TMainFrameWnd wndMain;
    wndMain.CreateMainWnd();
    facCIDCtrls().uMainMsgLoop(wndMain);

    wndMain.Destroy();

    // If they wanted to remove the directory, invoke the batch file as we exit
    if (wndMain.bRemoveDir())
    {
        // Change our current directory so we can't be holding the bin directory open
        TFileSys::SetCurrentDir(TSysInfo::strSpecialPath(tCIDLib::ESpecialPaths::UserAppData));

        TExternalProcess extpRun;
        extpRun.SystemEscape
        (
            wndMain.strRemoveDirCmd()
            , tCIDLib::eOREnumBits
              (
                tCIDLib::ESysEscFlags::NewWindow, tCIDLib::ESysEscFlags::StayOpen
              )
        );
    }
    return tCIDLib::EExitCodes::Normal;
}

