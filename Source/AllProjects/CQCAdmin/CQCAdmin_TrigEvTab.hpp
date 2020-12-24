//
// FILE NAME: CQCAdmin_TrigEvTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/07/2015
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
//  This is the header for the CQCClient_TrigEvTab.cpp file, which implements a
//  tab window that allows the user to configure a triggered event.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once

// ---------------------------------------------------------------------------
//  CLASS: TTrigEvTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TTrigEvTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TTrigEvTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TTrigEvTab(const TTrigEvTab&) = delete;
        TTrigEvTab(TTrigEvTab&&) = delete;

        ~TTrigEvTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTrigEvTab& operator=(const TTrigEvTab&) = delete;
        TTrigEvTab& operator=(TTrigEvTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCTrgEvent&           csrcEdit
        );

        tCIDLib::TVoid UpdatePauseState
        (
            const   tCIDLib::TBoolean       bToSet
        );


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

        tCIDLib::ESaveRes eDoSave
        (
                    TString&                strErr
            , const tCQCAdmin::ESaveModes   eMode
            ,       tCIDLib::TBoolean&      bChangedSaved
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_csrcEdit
        //      The event we are editing.
        //
        //  m_pwndEditor
        //      The actual editor stuff is provided as a reusable bit which we create
        //      and use.
        //
        //  m_strPrefTxt_XXX
        //      Some strings that we pre-load and swap into the field value
        //      prefix control depending on what type of filter is selected.
        // -------------------------------------------------------------------
        TCQCTrgEvent    m_csrcEdit;
        TEditTrigEvWnd* m_pwndEditor;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTrigEvTab, TContentTab)
};


