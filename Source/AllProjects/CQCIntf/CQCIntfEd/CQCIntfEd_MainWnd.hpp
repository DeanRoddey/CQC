//
// FILE NAME: CQCIntfEd_MainWnd.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/01/2015
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
//  This is the header for the CQCIntfEd_MainWnd.cpp file, which implements the main
//  pane window container for the interfce editor. On the left is one of our interface
//  view derivatives, inside a scroll area, which is where the editing happens. On the
//  right is a pane that holds the attribute editor. The attribute editor is a
//  derivative of the base attr editor window so that we can handle visual editing
//  of some of our own interface related types. We just forward reference it and
//  create it internally so we don't have to expose it.
//
//  We also define a simple abstract callback interface that the containing class
//  can implement, and set themselves on the editor window and he'll call them
//  back with useful info.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


#pragma CIDLIB_PACK(CIDLIBPACK)

class TCQCIntfEdView;
class TIntfAttrEdWnd;
class TIntfEditWnd;
class TIntfAttrEdTab;
class TIntfErrListWnd;
class TIntfToolsTab;
class TIntfFindWdgTab;
class TIntfWidgetPalTab;


// ---------------------------------------------------------------------------
//  CLASS: TIntfEdUpdateTopic
// PREFIX: ieut
// ---------------------------------------------------------------------------
class CQCINTFEDEXPORT TIntfEdUpdateTopic : public TObject
{
    public  :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        enum class ETypes
        {
            Enter
            , Exit

            , SelectChanged

            , Added
            , BlockAdded
            , BlockRemoved
            , Changed
            , Cleared
            , Moved
            , Reordered
            , Removed
            , Swapped

            , Count
        };


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfEdUpdateTopic();

        TIntfEdUpdateTopic
        (
            const   TCQCIntfWidget* const   piwdgEntered
        );

        TIntfEdUpdateTopic
        (
            const   tCQCIntfEng::TDesIdList& fcolSelList
        );

        TIntfEdUpdateTopic
        (
            const   tCIDLib::TBoolean       bAdded
            , const TCQCIntfWidget&         iwdgAffected
            , const tCIDLib::TCard4         c4At
        );

        TIntfEdUpdateTopic(const TIntfEdUpdateTopic&) = delete;

        ~TIntfEdUpdateTopic();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfEdUpdateTopic& operator=(const TIntfEdUpdateTopic&) = delete;


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_areaPos
        //      If a widget changed or added event, this is the area of the widget.
        //
        //  m_c4Count
        //      For block events this is the count, from index 1.
        //
        //  m_c4Index1
        //  m_c4Index2
        //      For swaps or moves, these are the two indices. For block events the first
        //      index is the start of the block.
        //
        //  m_c8DesId
        //      If a widget changed or added event, this is the design time id of it. It
        //      is also sent for enter/exit events.
        //
        //  m_eType
        //      The type of event, which defines which other members are used
        //
        //  m_fcolIdList
        //      A list of design time ids. For selection change it's the new selection
        //      list. If a block add event it's the ids of the widgets added.
        //
        //  m_strWidgetId
        //      If a widget changed, or added event, this is the widget id of it.
        //
        //  m_strWidgetType
        //      If a widget changed or added event, this is the type of it.
        // -------------------------------------------------------------------
        TArea                   m_areaPos;
        tCIDLib::TCard4         m_c4Count;
        tCIDLib::TCard4         m_c4Index1;
        tCIDLib::TCard4         m_c4Index2;
        tCIDLib::TCard8         m_c8DesId;
        ETypes                  m_eType;
        tCQCIntfEng::TDesIdList m_fcolIdList;
        TString                 m_strWidgetId;
        TString                 m_strWidgetType;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfEdUpdateTopic, TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfEdMainWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------
class CQCINTFEDEXPORT TIntfEdMainWnd : public TPaneWndCont
{
    public  :
        // -------------------------------------------------------------------
        // Constructors and Destructor
        // -------------------------------------------------------------------
        TIntfEdMainWnd();

        TIntfEdMainWnd(const TIntfEdMainWnd&) = delete;

        ~TIntfEdMainWnd();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfEdMainWnd& operator=(const TIntfEdMainWnd&) = delete;


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bProcessAccel
        (
            const   tCIDLib::TVoid* const   pMsgData
        )   const override;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bChanges() const;

        tCIDLib::TBoolean bSaveChanges
        (
                    TString&                strErrMsg
        );

        tCIDLib::TVoid CreateIEWnd
        (
            const   TWindow&                wndParent
            , const TCQCUserCtx&            cuctxToUse
            , const tCIDCtrls::TWndId       widToUse
            , const TString&                strTmplPath
        );

        const TString& strEdUpdateTopic() const;

        const TString& strTmplPath() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid Destroyed() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pcivEdit
        //      An instance of our editor specific view derivative. We create one of these
        //      and give it an instance of our editor window to manage.
        //
        //  m_pwndAttrEdTab
        //      The attribute editor that makes up one of the tabs of the right side tabbed
        //      window pane.
        //
        //  m_pwndEditor
        //      An instance of our editor window, which provides us with just enough
        //      functionality to handle editing and to make the underlying interface engine
        //      view framework happy. We give a pointer to this to our edit specific view
        //      object above, which handles translating any commands from the engine to our
        //      window, mostly dummied out since they won't happen or don't matter during
        //      editing. We also give a pointer to the Find tab, so that it can access the
        //      active template.
        //
        //  m_pwndFindTab
        //      This is one of the tabs in the right side pane. This one provides a simple
        //      search tool to find widgets. In some complex templates it can be hard to find
        //      a particular widget you want to modify, remove, etc...
        //
        //  m_pwndLeft
        //      The left side is a nested pane container. The top part is the editor
        //      and the bottom is the error display. So m_pwndEditor is set as the top
        //      pane, and m_pwndErrList is set as the bottom.
        //
        //  m_pwndRight
        //      The right side pane is a tabbed window into which we can put various tools
        //      (currently being the attribute editor and the quick actions bar.)
        //
        //  m_pwndScroll
        //      We put the editor window inside a scroll window, and then this goes into
        //      the upper part of the left side pane.
        //
        //  m_pwndToolsTab
        //      This is one of the tabs in the right side pane, which provides a set of
        //      'quick access' tools for alignment, font selection, and some other things
        //      potentially over time. We set up a notification handle for each of its
        //      buttons so that we can react to them.
        //
        //  m_pwndWdgPal
        //      THis is one of the tabs on the right side pane. This one provides a list
        //      of the current widgets in their z-order and allows for some manipulation
        //      of the widgets.
        // -------------------------------------------------------------------
        TCQCIntfEdView*     m_pcivEdit;
        TIntfAttrEdWnd*     m_pwndAttrEd;
        TIntfAttrEdTab*     m_pwndAttrEdTab;
        TIntfEditWnd*       m_pwndEditor;
        TIntfErrListWnd*    m_pwndErrList;
        TIntfFindWdgTab*    m_pwndFindTab;
        TPaneWndCont*       m_pwndLeft;
        TTabbedWnd*         m_pwndRight;
        TScrollArea*        m_pwndScroll;
        TIntfToolsTab*      m_pwndToolsTab;
        TIntfWidgetPalTab*  m_pwndWdgPal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfEdMainWnd,TPaneWndCont)
};

#pragma CIDLIB_POPPACK


