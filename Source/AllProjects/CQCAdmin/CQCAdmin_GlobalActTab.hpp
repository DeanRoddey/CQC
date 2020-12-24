//
// FILE NAME: CQCAdmin_GlobalActTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2015
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
//  This is the header for the CQCClient_GlobalActTab.cpp file, which implements a
//  tab window that provides the wee bit of stuff required to edit a global action.
//  Almost all of it is in the action, which is done by invoking the action editor.
//  But we allow them to edit the human readable title and we provide an action
//  preview so that they can see the action without having to load the action editor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


// ---------------------------------------------------------------------------
//  CLASS: TGlobalActTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TGlobalActTab : public TContentTab
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TGlobalActTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TGlobalActTab(const TGlobalActTab&) = delete;
        TGlobalActTab(TGlobalActTab&&) = delete;

        ~TGlobalActTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TGlobalActTab& operator=(const TGlobalActTab&) = delete;
        TGlobalActTab& operator=(TGlobalActTab&&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CreateTab
        (
                    TTabbedWnd&             wndParent
            , const TString&                strTabText
            , const TCQCStdCmdSrc&          csrcEdit
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
        //      The keyed command source object that we use to represent global actions.
        //
        //  m_csrcOrg
        //      We need a second copy of the incoming action, to compare against the
        //      edited one, so that we can report whether we have changes or not.
        //
        //  m_pwndXXX
        //      The others are just pointers we get to the child widgets we create,
        //      which are generated from a dialog description.
        //
        //  m_wndTrace
        //      We let them invoke the action trace. We have to keep around the window
        //      object since it can come and go and need to bring it forward if they have
        //      already created it.
        // -------------------------------------------------------------------
        TCQCStdCmdSrc       m_csrcEdit;
        TCQCStdCmdSrc       m_csrcOrg;
        TStaticMultiText*   m_pwndActText;
        TPushButton*        m_pwndEdit;
        TPushButton*        m_pwndTest;
        TEntryField*        m_pwndTitle;
        TActTraceWnd        m_wndTrace;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotClick
        );


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TGlobalActTab, TContentTab)
};


