//
// FILE NAME: CQCAdmin_ContentTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/07/2015
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
//  This is the header for the CQCClient_ContentTab.cpp file, which implements a
//  base class, derived from the tab window class. All of the tab windows that go
//  into the right hand pane (the ones viewing/editing content) derive from this
//  class, so that we can provide common functionality for them all.
//
//  Every tab has a path, which reflects where it came from in the browser window.
//  In the rare case that we might hav esome tab that isn't displaying/editing
//  browser contents, we'll give it some pseudo path to reflect that.
//
//  We set the window name (the name, not the text) to the incoming path plus a
//  prefix that indicates the editing mode. We do this because some things that
//  may have to interact with a tab won't have access to the tab class, and will
//  just see it as a window. This way it can get access to the path and edit mode
//  if need be.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TMainFrameWnd;

// ---------------------------------------------------------------------------
//  CLASS: TContentTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TContentTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TContentTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
            , const TString&                strType
            , const tCIDLib::TBoolean       bEditMode
        );

        TContentTab(const TTabWindow&) = delete;
        TContentTab(TTabWindow&&) = delete;

        ~TContentTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TContentTab& operator=(const TContentTab&) = delete;
        TContentTab& operator=(TContentTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual const TString& strContName() const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bEditMode() const;

        tCIDLib::ESaveRes eSaveChanges
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangesSaved
        );

        const TString& strPath() const;

        const TString& strRelPath() const;

        tCIDLib::TVoid SetStatusAreas
        (
            const tCIDLib::TCardList&       fcolStatusSlots
        );

        tCIDLib::TVoid UpdateStatusBar
        (
                    TMainFrameWnd&          wndTarget
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangesSaved
        );


        // -------------------------------------------------------------------
        //  Protected, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid ClearAllStatusSlots();

        tCIDLib::TVoid SetStatusSlot
        (
            const   tCIDLib::TCard4         c4SlotInd
            , const TString&                strValue
        );


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bEditMode
        //      Since many things can be either edited or viewed, and we want to
        //      be able to bring up the existing tab if they request it again, we
        //      need to know both the path of the thing and if it's view mode or
        //      edit mode.
        //
        //  m_colStatusSlots
        //      Storage for the status bar values for this tab. m_fcolStatusSlots
        //      controls how many there are.
        //
        //  m_fcolStatusSlots
        //      Each tab can define a set of status slots. These are used to fill
        //      in the status bar. We provide storage in m_colStatusSlots, so
        //      that we can store the values until we are activated. Only the
        //      active tab's values are displayed. When a tab is activated, the
        //      main frame's status bar is reset to show that tab's slot data.
        //
        //      This is the slot size values used to set up the status bar, and
        //      it defines how many slots are in m_colStatusSlots.
        //
        //  m_strPath
        //      Everything in a tab has a path, which reflects their position in
        //      the browser window. This is the unique id for a tab.
        //
        //  m_strRelPath
        //      We store the relative path version of the main path above, since
        //      that is often needed, and it avoids folks having to do it over and
        //      over.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean   m_bEditMode;
        tCIDLib::TStrList   m_colStatusSlots;
        tCIDLib::TCardList  m_fcolStatusSlots;
        TString             m_strPath;
        TString             m_strRelPath;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TContentTab, TTabWindow)
};


