//
// FILE NAME: CQCIntfEd_Tools_.hpp
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
//  This is the header for the CQCIntfEd_Tools.cpp file, which implements the simple
//  tools windows that go into the tabbed window on the right side of the editor.
//
//  Attr Editor Tab
//
//      A very simple derivative of the tab window that just creates our attribute editor
//      window and keeps it sized to fit the tab.
//
//  Tools Tab
//
//      It just has a set of 'quick access' buttons to do common tasks to all selected
//      widgets at once, such as alignment. We just set up a notification handler for
//      each of our buttons.
//
//      Since it uses image based buttons, we don't use a dialog description to create
//      our contents. We create them on the fly. We put them into a window that is fixed
//      to the size required to show all the buttons (in two vertical columns), and that
//      is put into a scroll window that is the actual child of the tab window. This
//      insures they can be gotten to even if the top level window is sized down.
//
//  Widget Palette
//
//      This is a list that reflects the current widgets in the editor window, in their
//      Z-Order, and displaying some basic info about them. This one is a bit more
//      complicated so we have a 2.cpp file for this one. The interface editor window
//      will set up a publish/subcribe topic on the ref vector he uses to manage the
//      widgets. We subscribe to that topic and so we know whenever it changes and can
//      update our contents. This is vastly simpler than the old one with ad hoc code
//      all over the interface engine and editor code to keep the palette updated.
//
//      The editor window also publishes msgs about what widgets are selected, so we
//      subscribe to that as well, so that we can mark them to match in our list.
//
//      The palette window also publishes some msgs which the editor window subscribes
//      to, so that we can let the user do some operations via the widget palette. We
//      keep them to a minimum for now.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TIntfAttrEdTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfAttrEdTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfAttrEdTab();

        TIntfAttrEdTab(const TIntfAttrEdTab&) = delete;

        ~TIntfAttrEdTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfAttrEdTab& operator=(const TIntfAttrEdTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TIntfAttrEdWnd* pwndAttrEd()
        {
            return m_pwndAttrEd;
        }


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TIntfEdMainWnd;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private data members
        //
        //  The attribute editor that we just keep sized to fit us
        // -------------------------------------------------------------------
        TIntfAttrEdWnd*     m_pwndAttrEd;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfAttrEdTab,TTabWindow)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfFindWdgTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfFindWdgTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfFindWdgTab() = delete;

        TIntfFindWdgTab
        (
                    TCQCIntfEdView* const   pcivEdit
        );

        TIntfFindWdgTab(const TIntfFindWdgTab&) = delete;

        ~TIntfFindWdgTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfFindWdgTab& operator=(const TIntfFindWdgTab&) = delete;


    protected :
        // -------------------------------------------------------------------
        //  Declare our friends
        // -------------------------------------------------------------------
        friend class TIntfEdMainWnd;


        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid TabActivationChange
        (
            const   tCIDLib::TBoolean       bGetting
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_areaOrgList
        //      The original area of the matches list as created.
        //
        //  m_c8TmplId
        //      We need to link from the widgets in the list to originals. But the designer
        //      ids are 64 bit and the list window only allows a 32 bit value to be associated
        //      with each item. So we store the bottom half there, which is widget specific,
        //      and we stop the template specific part here, which is the same for all of
        //      them. Then we can re-build the full id again as required.
        //
        //  m_pcivEdit
        //      We are given a pointer to the view for our editing session. This gets us
        //      access to the template and other info we might need.
        //
        //  m_pntOrgListOfs
        //      The amounts that the matches list was originally offset from the right and
        //      bottom of the dialog content it was created from.
        //
        //  m_pwndClient
        //      We create a generic window to serve as the tab content window and then
        //      create the other controls inside of it.
        //
        //  m_pwndXXX
        //      We use a dialog description to create our widgets. We get typed pointers
        //      to the ones we need to interact with. We size the content to fit our
        //      size as appropriate.
        //
        //  m_areaOrgList
        //      The original area of the matches list. We won't let it get smaller than
        //      that. But if space permits we will size it up to show more. The origin
        //      lets us know where to calculate from.
        // -------------------------------------------------------------------
        TArea               m_areaOrgList;
        tCIDLib::TCard8     m_c8TmplId;
        TCQCIntfEdView*     m_pcivEdit;
        TPoint              m_pntOrgListOfs;
        TGenericWnd*        m_pwndClient;
        TPushButton*        m_pwndFind;
        TEntryField*        m_pwndId;
        TCheckBox*          m_pwndIdREx;
        TEntryField*        m_pwndImage;
        TCheckBox*          m_pwndImageREx;
        TMultiColListBox*   m_pwndMatches;
        TPushButton*        m_pwndSelImage;
        TEntryField*        m_pwndText;
        TCheckBox*          m_pwndTextREx;
        TComboBox*          m_pwndTypes;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfFindWdgTab,TTabWindow)
};



// ---------------------------------------------------------------------------
//  CLASS: TIntfToolsTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfToolsTab : public TTabWindow
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfToolsTab();

        TIntfToolsTab(const TIntfToolsTab&) = delete;

        ~TIntfToolsTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfToolsTab& operator=(const TIntfToolsTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid WireTo
        (
                    TIntfEditWnd&           wndTarget
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;


    private :
        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BlockAdded
        (
            const   tCIDLib::TCard4         c4Index1
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TVoid LoadWidget
        (
            const   TCQCIntfWidget&         iwdgSrc
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid ReloadList();


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pwndScroll
        //      We create a scroll area and keep it sized to us.
        //
        //  m_pwndTools
        //      We create a generic window and put the buttons into it, and put it in
        //      turn into the scroll area.
        // -------------------------------------------------------------------
        TScrollArea*    m_pwndScroll;
        TImgPushButton* m_pwndTool_BottomAlign;
        TImgPushButton* m_pwndTool_FontBold;
        TImgPushButton* m_pwndTool_FontDecrease;
        TImgPushButton* m_pwndTool_FontFace;
        TImgPushButton* m_pwndTool_FontItalic;
        TImgPushButton* m_pwndTool_FontIncrease;
        TImgPushButton* m_pwndTool_HorzCenter;
        TImgPushButton* m_pwndTool_HorzSpace;
        TImgPushButton* m_pwndTool_LeftAlign;
        TImgPushButton* m_pwndTool_RightAlign;
        TImgPushButton* m_pwndTool_TopAlign;
        TImgPushButton* m_pwndTool_VertCenter;
        TImgPushButton* m_pwndTool_VertSpace;
        TGenericWnd*    m_pwndTools;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfToolsTab,TTabWindow)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfWidgetPalPub
// PREFIX: wppub
// ---------------------------------------------------------------------------
class TIntfWidgetPalPub : public TObject
{
    public :
        // -------------------------------------------------------------------
        //  Public class types
        // -------------------------------------------------------------------
        enum class EEvents
        {
            Del
            , MoveBack
            , MoveForward
            , SelChanged
        };


        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfWidgetPalPub() = delete;

        TIntfWidgetPalPub
        (
            const   EEvents                     eEvent
        );

        TIntfWidgetPalPub
        (
            const   tCQCIntfEng::TDesIdList&    fcolSel
        );

        ~TIntfWidgetPalPub();

        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfWidgetPalPub& operator=(const TIntfWidgetPalPub&) = delete;


        // -------------------------------------------------------------------
        //  Public data members
        //
        //  m_eEvent
        //      The event being reported, which determines whihc of the other members
        //      are used.
        //
        //  m_fcolSel
        //      If the event is SelChanged, this is the list of designer ids of the
        //      widgets that are now selected.
        // -------------------------------------------------------------------
        EEvents                 m_eEvent;
        tCQCIntfEng::TDesIdList m_fcolSel;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfWidgetPalPub,TObject)
};


// ---------------------------------------------------------------------------
//  CLASS: TIntfWidgetPalTab
// PREFIX: wnd
// ---------------------------------------------------------------------------
class TIntfWidgetPalTab : public TTabWindow, private MPubSubscription
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and destructor
        // -------------------------------------------------------------------
        TIntfWidgetPalTab() = delete;

        TIntfWidgetPalTab
        (
                    TCQCIntfEdView* const   pcivEdit
        );

        TIntfWidgetPalTab(const TIntfWidgetPalTab&) = delete;

        ~TIntfWidgetPalTab();


        // -------------------------------------------------------------------
        //  Public operators
        // -------------------------------------------------------------------
        TIntfWidgetPalTab& operator=(const TIntfWidgetPalTab&) = delete;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid LoadWidgets();


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid AreaChanged
        (
            const   TArea&                  areaPrev
            , const TArea&                  areaNew
            , const tCIDCtrls::EPosStates   ePosState
            , const tCIDLib::TBoolean       bOrgChanged
            , const tCIDLib::TBoolean       bSizeChanged
            , const tCIDLib::TBoolean       bStateChanged
        )   override;

        tCIDLib::TBoolean bCreated() override;

        tCIDLib::TVoid ProcessPubMsg
        (
            const   TPubSubMsg&             psmsgIn
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Private class constants
        // -------------------------------------------------------------------
        const tCIDLib::TCard4 c4ColInd_Id   = 0;
        const tCIDLib::TCard4 c4ColInd_Type = 1;
        const tCIDLib::TCard4 c4ColInd_Area = 2;
        const tCIDLib::TCard4 c4ColCount    = 3;


        // -------------------------------------------------------------------
        //  Private, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid BlockAdded
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Count
        );

        tCIDLib::TVoid BlockRemoved
        (
            const   tCIDLib::TCard4         c4Start
            , const tCIDLib::TCard4         c4Count
        );

        tCIDCtrls::EEvResponses eClickHandler
        (
                    TButtClickInfo&         wnotEvent
        );

        tCIDCtrls::EEvResponses eLBHandler
        (
                    TListChangeInfo&        wnotEvent
        );

        tCIDLib::TVoid ElemMoved
        (
            const   tCIDLib::TCard4         c4From
            , const tCIDLib::TCard4         c4To
        );

        tCIDLib::TVoid LoadWidget
        (
            const   TCQCIntfWidget&         iwdgSrc
            , const tCIDLib::TCard4         c4At
        );

        tCIDLib::TVoid ReloadList();

        tCIDLib::TVoid UpdateWidget
        (
            const   TCQCIntfWidget&         iwdgSrc
            , const tCIDLib::TCard4         c4At
        );


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_bSuppressPub
        //      Despite never reporting any programmatic changes, for some reason mark
        //      changes are being reported when we make them in response to the editor
        //      window sending us changes. So we use this to prevent our list box handler
        //      from turning around and responding which makes a mess of things. It's
        //      set while we are updating from a editor window selection change msg.
        //
        //  m_colCols
        //      To avoid continually creating and destroying a column values list, we
        //      just keep one around, pre-sized to the number of columns.
        //
        //  m_pcivEdit
        //      We have to keep a ref to the view that owns us so that we can get to the
        //      template that contains the widgets we are displaying.
        //
        //  m_psmsgTmp
        //      To avoid constantly creating and destroying one of these, we just keep
        //      one around to use during the Timer() method.
        //
        //  m_pstopChanges
        //      We create our own publish/subscribe topic which we use to publish changes
        //      that the user asks us to make. The editor window watches these and updates
        //      accordingly.
        //
        //  m_pwndClient
        //      A generic client window that we use as our container for the controls.
        //      We just keep this sized to fit our tab client area.
        //
        //  m_pwndXXX
        //      Widgets we create from a dialog description. We create them as children
        //      of m_pwndClient.
        //
        //  m_tmidSub
        //      We crank up a timer to grab stuff from our subscriptions and we update
        //      the list accordingly.
        // -------------------------------------------------------------------
        tCIDLib::TBoolean       m_bSuppressPub;
        tCIDLib::TStrList       m_colCols;
        TCQCIntfEdView* const   m_pcivEdit;
        TPubSubTopic            m_pstopChanges;
        TGenericWnd*            m_pwndClient;
        TPushButton*            m_pwndDel;
        TPushButton*            m_pwndMoveBack;
        TPushButton*            m_pwndMoveForward;
        TMultiColListBox*       m_pwndPal;


        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TIntfWidgetPalTab,TTabWindow)
};

#pragma CIDLIB_POPPACK
