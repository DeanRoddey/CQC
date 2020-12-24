//
// FILE NAME: CQCAdmin_BrowserTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/20/2015
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
//  This is the header for the CQCClient_BrowserTab.cpp file, which implements a
//  tab window that contains a CQC tree browser. This is added as one of the tabs
//  in the left hand tab pane.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TBrowserTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TBrowserTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TBrowserTab();

        TBrowserTab(const TBrowserTab&) = delete;
        TBrowserTab(TBrowserTab&&) = delete;

        ~TBrowserTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TBrowserTab&) = delete;
        tCIDLib::TVoid operator=(TBrowserTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const final;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
        );

        tCIDLib::TVoid UpdateFile
        (
            const   TString&                strPath
        );

        TCQCTreeBrowser& wndBrowser();


    protected  :
        // -------------------------------------------------------------------
        //  Protected, inherited method
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   final;

        tCIDLib::TBoolean bCreated() final;

        tCIDLib::TVoid Destroyed() final;



    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndBrowser
        //      The browser window we create. We keep it sized to our tab content
        //      area.
        // -------------------------------------------------------------------
        TCQCTreeBrowser*    m_pwndBrowser;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TBrowserTab, TTabWindow)
};

