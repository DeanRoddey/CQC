//
// FILE NAME: CQCAdmin_EvMonTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/10/2015
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
//  This is the header for the CQCClient_EvMonTab.cpp file, which implements a
//  tab window that allows the user to configure an event monitor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TEvMonTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TEvMonTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TEvMonTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TEvMonTab(const TEvMonTab&) = delete;
        TEvMonTab(TEvMonTab&&) = delete;

        ~TEvMonTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        tCIDLib::TVoid operator=(const TEvMonTab&) = delete;
        tCIDLib::TVoid operator=(TEvMonTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCEvMonCfg&           emoncEdit
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
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_apwndXXX
        //      The child controls for the filters are all redundant so we
        //      put them into arrays so that we can process filter stuff
        //      by just knowing the filter index.
        //
        //  m_emonCfg
        //  m_emonOrg
        //      We make two copies of the incoming info, one that we can edit and
        //      another that holds the original content, so that we can check for
        //      changes.
        //
        //  m_pwndXXX
        //      Some pointers to widgets that we want to deal with via their
        //      real types. We don't own them, we just reference them. Some
        //      are in the m_apwndXXX members.
        // -------------------------------------------------------------------
        TCQCEvMonCfg    m_emoncEdit;
        TCQCEvMonCfg    m_emoncOrg;
        TEntryField*    m_pwndClass;
        TEntryField*    m_pwndDescr;
        TEntryField*    m_pwndParams;
        TPushButton*    m_pwndSelButton;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TEvMonTab, TContentTab)
};



