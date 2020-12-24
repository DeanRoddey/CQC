//
// FILE NAME: CQCAdmin_QueryLogsTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/12/2016
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
//  This is the header for the CQCClient_QueryLogsTab.cpp file, which allows the user
//  to search the logs in various ways.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TWebBrowserWnd;

// ---------------------------------------------------------------------------
//  CLASS: TQueryLogsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TQueryLogsTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TQueryLogsTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TQueryLogsTab(const TQueryLogsTab&) = delete;
        TQueryLogsTab(TQueryLogsTab&&) = delete;

        ~TQueryLogsTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TQueryLogsTab& operator=(const TQueryLogsTab&) = delete;
        TQueryLogsTab& operator=(TQueryLogsTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
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


    private :
        // -------------------------------------------------------------------
        //  Private class types and constants
        // -------------------------------------------------------------------
        static constexpr tCIDLib::TCard4 c4Col_Severity  = 0;
        static constexpr tCIDLib::TCard4 c4Col_Host      = 1;
        static constexpr tCIDLib::TCard4 c4Col_Process   = 2;
        static constexpr tCIDLib::TCard4 c4Col_Line      = 3;
        static constexpr tCIDLib::TCard4 c4Col_Time      = 4;
        static constexpr tCIDLib::TCard4 c4Col_Text      = 5;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard8 c8SevMask() const;

        tCIDLib::TCard8 c8ClassMask() const;

        tCIDLib::TVoid CopyToClipboard();

        tCIDLib::TVoid DoQuery();

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotClick
        );

        tCIDLib::TVoid InitLists();

        tCIDLib::TVoid LoadMatches();

        tCIDLib::TVoid SnapToFile();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4Col_XXX
        //      Our various indices for our list window columns.
        //
        //  m_colFilters
        //      The list of filtesr we load into the filter list
        //
        //  m_colMatches
        //      the log events that matched the last query. These are what we show in
        //      the list. It is column sortable so we store the original index into
        //      list as the row id.
        //
        //  m_pwndXXX
        //      Some important controls we look up after creating them.
        //
        //  m_strFmt
        //      For temp formatting purposes when we are loading u the matches.
        //
        //  m_tmFmt
        //      We pre-set up this guy for formatting times in the desired form.
        // -------------------------------------------------------------------
        tCIDLib::TKVPList       m_colFilters;
        TVector<TLogEvent>      m_colMatches;
        TListBox*               m_pwndClasses;
        TPushButton*            m_pwndClear;
        TPushButton*            m_pwndCopy;
        TStaticText*            m_pwndCurAux;
        TStaticText*            m_pwndCurErrs;
        TStaticText*            m_pwndCurFac;
        TStaticText*            m_pwndCurFile;
        TStaticText*            m_pwndCurThread;
        TComboBox*              m_pwndFilter;
        TMultiColListBox*       m_pwndList;
        TComboBox*              m_pwndMaxHits;
        TPushButton*            m_pwndSearch;
        TListBox*               m_pwndSevs;
        TPushButton*            m_pwndSnap;
        TString                 m_strFmt;
        TTime                   m_tmFmt;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TQueryLogsTab, TContentTab)
};

