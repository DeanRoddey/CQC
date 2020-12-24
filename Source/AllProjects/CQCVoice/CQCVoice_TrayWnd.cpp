//
// FILE NAME: CQCVoice_TrayWnd.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 01/28/2017
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
//  This file implements the tray application window. This one is never
//  visible. It exists just to interact with the system tray. We have a
//  separate frame window that can be brought up when the user clicks on
//  the tray icon.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCVoice.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCVoiceTrayWnd,TTrayAppWnd)



// ---------------------------------------------------------------------------
//  CLASS: TTrayAppWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTrayAppWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCVoiceTrayWnd::TCQCVoiceTrayWnd() :

    m_menuPopup(L"CQCVoice Popup")
{
}


TCQCVoiceTrayWnd::~TCQCVoiceTrayWnd()
{
}



// ---------------------------------------------------------------------------
//  TTrayAppWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

// Called by the tray app framework when it's time for us to show our menu
tCIDLib::TBoolean TCQCVoiceTrayWnd::bShowTrayMenu(const TPoint& pntAt)
{
    // Make us the foreground window
    facCIDCtrls().SetForeground(hwndThis(), kCIDLib::False);

    // We want it bottom and right aligned
    const tCIDLib::TCard4 c4Cmd = m_menuPopup.c4Process
    (
        *this, pntAt, tCIDLib::EVJustify::Bottom
    );

    // If no selection, just return now with false
    if (c4Cmd == kCIDLib::c4MaxCard)
        return kCIDLib::False;

    // Do the requested operation
    switch(c4Cmd)
    {
        case kCQCVoice::ridMenu_Exit :
            // Exit the program
            facCIDCtrls().ExitLoop(tCIDLib::EExitCodes::Normal);
            break;

        default :
            break;
    };

    return kCIDLib::True;
}


tCIDLib::TVoid TCQCVoiceTrayWnd::CleanupTrayApp()
{
    // Nothing for us to do right now
}


//
//  Handle any initialization that we need to do
//
tCIDLib::TVoid TCQCVoiceTrayWnd::InitTrayApp()
{
    // Create our popup menu for later use
    m_menuPopup.Create(facCQCVoice, kCQCVoice::ridMenu_Main);
}


//
//  Our icon was clicked on. If we are hidden, we show ourself. If we are
//  visible, we hide ourself. The facility provides a helper to do this.
//
tCIDLib::TVoid TCQCVoiceTrayWnd::Invoked()
{
    facCQCVoice.ToggleFrameVis();
}


//
//  Since we run in the tray, and often run in client machines, we have
//  to deal cleanly with the system going into sleep mode and coming back
//  out. When we go into sleep mode, we clean our stuff up, and then crank
//  it all back up when we we come back awake.
//
tCIDLib::TVoid TCQCVoiceTrayWnd::PowerStateChange(const tCIDLib::TBoolean bResume)
{
    // Pass this on to the facility object
    facCQCVoice.SleepWake(bResume);
}

