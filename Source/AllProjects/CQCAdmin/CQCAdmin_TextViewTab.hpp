//
// FILE NAME: CQCAdmin_TextViewTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/03/2015
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
//  This is the header for the CQCClient_TextViewTab.cpp file, which implements a
//  tab window that just contains a scrollable multi-line text window. It's used
//  when the user asks to view something that has a text representation, i.e.
//  read-only access.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TTextViewTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TTextViewTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TTextViewTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
            , const TString&                strType
        );

        TTextViewTab(const TTextViewTab&) = delete;
        TTextViewTab(TTextViewTab&&) = delete;

        ~TTextViewTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TTextViewTab& operator=(const TTextViewTab&) = delete;
        TTextViewTab& operator=(TTextViewTab&&) = delete;


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
            , const TString&                strTabText
            , const TString&                strContText
            , const tCIDLib::TBoolean       bIsCode
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

        tCIDLib::TVoid Destroyed() final;

        tCIDLib::TVoid MenuCommand
        (
            const   tCIDLib::TResId         ridItem
            , const tCIDLib::TBoolean       bChecked
            , const tCIDLib::TBoolean       bEnabled
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid GotoLine();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_accelTextView
        //      We have a small number of hot keys so we have to provide an accel table
        //      to handle them.
        //
        //  m_bFullMatchOnly
        //      Search flags that we pass into the search criteria dialog we use to get
        //      search info from the user. It uses them and updates them.
        //
        //  m_colGotoList
        //      A list of previously used line numbers from our 'go to line' code. We use
        //      a general purpose list select dialog that supports a recall list.
        //
        //  m_colSearchList
        //      A recall list searching, which we pass into the dialog that we use to
        //      get the text to search for. It updates the list for us.
        //
        //  m_pwndText
        //      The multi-line line edit window that displays thet text, created in
        //      read only mode.
        // -------------------------------------------------------------------
        TAccelTable         m_accelTextView;
        tCIDLib::TBoolean   m_bFullMatchOnly;
        tCIDLib::TStrList   m_colGotoList;
        tCIDLib::TStrList   m_colSearchList;
        TMultiEdit*         m_pwndText;
        TString             m_strToFind;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTextViewTab, TContentTab)
};

