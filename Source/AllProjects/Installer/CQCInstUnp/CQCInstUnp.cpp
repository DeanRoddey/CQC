//
// CID_FILE NAME: CQCInstUnp.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/24/2015
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
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CQCInstUnp.hpp"



// ----------------------------------------------------------------------------
//  Forward references
// ----------------------------------------------------------------------------
static tCIDLib::EExitCodes eMainThreadFunc
(
        TThread&            thrThis
        , tCIDLib::TVoid*   pData
);


// ----------------------------------------------------------------------------
//  Global data
//
//  facCQCInstUnp
//      The facility object for this program. It is exported to the rest of
//      the program via the main header. We have no special needs so we just
//      create an instance of the base GUI facility class.
// ----------------------------------------------------------------------------
TGUIFacility facCQCInstUnp
(
    L"CQCInstUnp"
    , tCIDLib::EModTypes::Exe
    , kCIDLib::c4MajVersion
    , kCIDLib::c4MinVersion
    , kCIDLib::c4Revision
    , tCIDLib::EModFlags::HasMsgsAndRes
);


// ----------------------------------------------------------------------------
//  Magic main module code
// ----------------------------------------------------------------------------
CIDLib_CtrlsModule(TThread(L"MainThread", eMainThreadFunc))



// ----------------------------------------------------------------------------
//  Local functions
// ----------------------------------------------------------------------------
static tCIDLib::EExitCodes eMainThreadFunc(TThread& thrThis, tCIDLib::TVoid*)
{
    // We have to let our calling thread go first
    thrThis.Sync();

    TMainWindow wndMainFrame;
    try
    {
        // Create a main frame window object
        if (wndMainFrame.bCreateMainWnd())
        {
            //
            //  Do the message loop. We use the standard error handler enabled
            //  frame window for our main frame, so the exception handler in the
            //  main loop will pass them to him and he'll handle them. Any
            //  unhandled exception will cause the loop to exit after logging
            //  it and telling the use what happened. No exceptions will
            //  propogate out of here
            //
            facCIDCtrls().uMainMsgLoop(wndMainFrame);

            // We are done, so destroy the frame window
            wndMainFrame.Destroy();
        }
    }

    catch(const TError& errToCatch)
    {
        TExceptDlg dlgErr
        (
            TWindow::wndDesktop()
            , facCQCInstUnp.strMsg(kInstUnpMsgs::midMain_Title)
            , L"An exception occurred"
            , errToCatch
        );
    }

    // Make sure that kernel errors get logged
    catch(const TKrnlError& kerrToCatch)
    {
    }

    catch(...)
    {
        TErrBox msgbFailed(L"An unknown exception occurred");
        msgbFailed.ShowIt(TWindow::wndDesktop());
    }
    return tCIDLib::EExitCodes::Normal;
}

