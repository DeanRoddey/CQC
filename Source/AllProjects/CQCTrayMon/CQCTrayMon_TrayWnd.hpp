//
// FILE NAME: CQCAppCtrl_TrayWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2012
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
//  This is the
//
// CAVEATS/GOTCHAS:
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TTrayMonWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TTrayMonWnd : public TTrayAppWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTrayMonWnd();

        ~TTrayMonWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual method
        // -------------------------------------------------------------------
        tCIDLib::TVoid ShowServiceStatus
        (
            const   tCQCTrayMon::ESrvStates eClState
            , const tCQCTrayMon::ESrvStates eAppShState
            , const tCIDLib::TBoolean       bNoPopup = kCIDLib::False
        );


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
        //  Unimplemented
        // -------------------------------------------------------------------
        TTrayMonWnd(const TTrayMonWnd&);
        tCIDLib::TVoid operator=(const TTrayMonWnd&);


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid FormatState
        (
            const   tCQCTrayMon::ESrvStates eAppShState
            , const tCIDLib::TBoolean       bClient
            ,       TString&                strToFill
        );

        tCIDLib::TVoid StartStopSrv
        (
            const   tCIDLib::TBoolean       bStart
            , const tCIDLib::TBoolean       bIsClient
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_eState
        //      The last service states we were told. This is needed to set
        //      the right start/stop service menu options when we pop up the
        //      menu.
        //
        //  m_hmenuPopup
        //      Our popup menu. We create it when first accessed and keep it
        //      around for reuse.
        //
        //  m_strSrvName_AppShell
        //  m_strSrvName_Client
        //      Some preloaded strings used to put the name of the specific service
        //      into state display text.
        //
        //  m_strStateTitle
        //      We preload the title for our service state popup.
        // -------------------------------------------------------------------
        tCQCTrayMon::ESrvStates m_eAppState;
        tCQCTrayMon::ESrvStates m_eClState;
        TPopupMenu              m_menuPopup;
        TString                 m_strSrvName_AppShell;
        TString                 m_strSrvName_Client;
        TString                 m_strStateTitle;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTrayMonWnd,TTrayAppWnd)
};

#pragma CIDLIB_POPPACK

