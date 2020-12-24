//
// FILE NAME: CQCAdmin_IntfEdTab.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2015
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
//  This is the header for the CQCClient_IntfEdTab.cpp file, which brings the
//  interface editor into a tab.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
#pragma once


class TIntfEdMainWnd;

// ---------------------------------------------------------------------------
//  CLASS: TIntfEdTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfEdTab : public TContentTab, private MPubSubscription
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfEdTab
        (
            const   TString&                strPath
            , const TString&                strRelPath
        );

        TIntfEdTab(const TIntfEdTab&) = delete;
        TIntfEdTab(TIntfEdTab&&) = delete;

        ~TIntfEdTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfEdTab& operator=(const TIntfEdTab&) = delete;
        TIntfEdTab& operator=(TIntfEdTab&&) = delete;


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

        tCIDLib::TVoid ProcessPubMsg
        (
            const   TPubSubMsg&             psmsgIn
        )   final;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c8LastMouseOver
        //      This is the design time id of the last widget we get a mouse over for.
        //      We remember this because this one is shown in the status bar. If we get
        //      a msg that its area or id changed, we want to know which one is there so
        //      that we can update it. It's zero if nothing is under the mouse.
        //
        //  m_psmsgTmp
        //      To avoid constantly creating and destroying one of these, we just keep
        //      one around to use during the Timer() method.
        //
        //  m_pssubMouseOver
        //      We subscribe to the editor window's mouse over topic. We use this to
        //      update the status bar as the mouse moves over a widget.
        //
        //  m_pssubWdgList
        //      We subscribe to the editor window's widget list topic. We only care about
        //      the element change events, which we can use to see if we need to update
        //      the status bar (i.e. the mouse is currently over that one and the id or
        //      area changed.)
        //
        //  m_pwndEditor
        //      The main editor engine pane manager window. We keep it sized
        //      to our client area.
        // -------------------------------------------------------------------
        tCIDLib::TCard8     m_c8LastMouseOver;
        TIntfEdMainWnd*     m_pwndEditor;


        // -------------------------------------------------------------------
        //  Do any needed magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfEdTab, TContentTab)
};


