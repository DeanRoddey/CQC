//
// FILE NAME: CQCTrayMon_MainFrame.hpp
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
//  This is the header for the CQCTrayMon_MainFrame.cpp file, which implements
//  the TCQCTrayMonFrameWnd. This window provides the actual interface. It's
//  only shown when the user clicks on our tray icon.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TCQCTrayMonFrameWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TCQCTrayMonFrameWnd : public TFrameWnd
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and destructor
        // -------------------------------------------------------------------
        TCQCTrayMonFrameWnd();

        ~TCQCTrayMonFrameWnd();


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid Cleanup();

        tCIDLib::TVoid Create
        (
            const   tCIDLib::TCard4         bAppCtrl
            , const tCIDLib::TCard4         biTunes
            , const TIcon&                  iconTray
        );

        tCIDLib::TVoid CreateTabs();

        tCIDLib::TVoid RemoveBindings
        (
                    tCIDOrbUC::TNSrvProxy&  orbcNS
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bAllowShutdown() override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bAppCtrl
        //  m_biTunes
        //      Upon creation we get flags indicating what tabs/interfaces
        //      we should create.
        //
        //  m_pwndXXX
        //      We get typed pointers to some of our child controls
        //
        //  m_colTabs
        //      A heterogenous list of tab pointers to all of the tabs we
        //      create. They belong to the tabbed window, but it's convenent
        //      to have a list we can loop through and make virtual calls to.
        // -------------------------------------------------------------------
        tCIDLib::TCard4             m_bAppCtrl;
        tCIDLib::TCard4             m_biTunes;
        TTabbedWnd*                 m_pwndTabs;
        TRefVector<TCQCTrayMonTab>  m_colTabs;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TCQCTrayMonFrameWnd,TFrameWnd)
};


