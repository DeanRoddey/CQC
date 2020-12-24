//
// FILE NAME: CQCVoice_TrayWnd.hpp
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
//  This is our implementation of the tray window interface. This is a hidden window
//  that provides the glue to the system tray.
//
// CAVEATS/GOTCHAS:
//
#pragma once

#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TCQCVoiceTrayWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCVoiceTrayWnd : public TTrayAppWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TCQCVoiceTrayWnd();

        TCQCVoiceTrayWnd(const TCQCVoiceTrayWnd&) = delete;

        ~TCQCVoiceTrayWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TCQCVoiceTrayWnd& operator=(const TCQCVoiceTrayWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bShowTrayMenu
        (
            const   TPoint&                 pntAt
        )   override;

        tCIDLib::TVoid CleanupTrayApp() override;

        tCIDLib::TVoid InitTrayApp() override;

        tCIDLib::TVoid Invoked() override;

        tCIDLib::TVoid PowerStateChange
        (
            const   tCIDLib::TBoolean       bResume
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_hmenuPopup
        //      Our popup menu. We create it when first accessed and keep it around
        //      for reuse.
        // -------------------------------------------------------------------
        TPopupMenu              m_menuPopup;

        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCVoiceTrayWnd,TTrayAppWnd)
};

#pragma CIDLIB_POPPACK

