//
// FILE NAME: CQCIntfEd_MainWnd.cpp
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
//  This file implements the main window of the editor.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $Log$
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "CQCIntfEd_.hpp"
#include    "CQCIntfEd_Tools_.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfEdUpdateTopic, TObject)
RTTIDecls(TIntfEdMainWnd,TPaneWndCont)


// ---------------------------------------------------------------------------
//  CLASS: TIntfEdUpdateTopic
// PREFIX: iect
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  Constructors and destructor
// ---------------------------------------------------------------------------

//
//  We have some helper ctors for specific scenarios. Others they have to just default
//  construct and then set up.
//
TIntfEdUpdateTopic::TIntfEdUpdateTopic() :

    m_c4Count(0)
    , m_c4Index1(kCIDLib::c4MaxCard)
    , m_c4Index2(kCIDLib::c4MaxCard)
    , m_c8DesId(0)
    , m_eType(ETypes::Count)
{
}

// This is treated as an enter/exit event
TIntfEdUpdateTopic::TIntfEdUpdateTopic(const TCQCIntfWidget* const piwdgEntered) :

    m_c4Count(0)
    , m_c4Index1(kCIDLib::c4MaxCard)
    , m_c4Index2(kCIDLib::c4MaxCard)
    , m_c8DesId(0)
    , m_eType(ETypes::Exit)
{
    if (piwdgEntered)
    {
        m_eType = ETypes::Enter;

        m_areaPos = piwdgEntered->areaRelative();
        m_c8DesId = piwdgEntered->c8DesId();
        m_strWidgetId = piwdgEntered->strWidgetId();
        m_strWidgetType = piwdgEntered->strTypeName();
    }
}

// This is a selection list change event
TIntfEdUpdateTopic::TIntfEdUpdateTopic(const tCQCIntfEng::TDesIdList& fcolSelList) :

    m_c4Count(0)
    , m_c4Index1(kCIDLib::c4MaxCard)
    , m_c4Index2(kCIDLib::c4MaxCard)
    , m_eType(ETypes::SelectChanged)
    , m_c8DesId(0)
    , m_fcolIdList(fcolSelList)
{
}

// This is a widget change or added event
TIntfEdUpdateTopic::TIntfEdUpdateTopic( const   tCIDLib::TBoolean   bAdded
                                        , const TCQCIntfWidget&     iwdgAffected
                                        , const tCIDLib::TCard4     c4At) :

    m_areaPos(iwdgAffected.areaRelative())
    , m_c4Count(0)
    , m_c4Index1(c4At)
    , m_c4Index2(kCIDLib::c4MaxCard)
    , m_c8DesId(iwdgAffected.c8DesId())
    , m_eType(bAdded ? ETypes::Added : ETypes::Changed)
    , m_strWidgetId(iwdgAffected.strWidgetId())
    , m_strWidgetType(iwdgAffected.strTypeName())
{
}

TIntfEdUpdateTopic::~TIntfEdUpdateTopic()
{
}




// ---------------------------------------------------------------------------
//  CLASS: TIntfEdMainWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfEdMainWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfEdMainWnd::TIntfEdMainWnd() :

    m_pcivEdit(nullptr)
    , m_pwndAttrEd(nullptr)
    , m_pwndAttrEdTab(nullptr)
    , m_pwndEditor(nullptr)
    , m_pwndFindTab(nullptr)
    , m_pwndErrList(nullptr)
    , m_pwndRight(nullptr)
    , m_pwndScroll(nullptr)
    , m_pwndToolsTab(nullptr)
{
    //
    //  Just to be sure, reference the facility object, to make sure that any
    //  init it does gets done before we actually load up any templates.
    //
    facCQCIntfEd();
}

TIntfEdMainWnd::~TIntfEdMainWnd()
{
}


//
//  We override this because we need to recursively distribute accellerator
//  processing. The client program should give us a shot at accellerators (if
//  that is appropriate based on active window.) We will then pass it to the
//  appropriate child window as well and return his return.
//
tCIDLib::TBoolean
TIntfEdMainWnd::bProcessAccel(const tCIDLib::TVoid* const pMsgData) const
{
    tCIDLib::TBoolean bHandled = kCIDLib::False;

    //
    //  If the find tab has the focus, then we just return false to let the input
    //  propogate.
    //
    if (m_pwndFindTab && m_pwndFindTab->bContainsFocus())
        return kCIDLib::False;

    // Else, if not handled, let the editor see it
    if (!bHandled && m_pwndEditor)
        bHandled = m_pwndEditor->bProcessAccel(pMsgData);
    return bHandled;
}



// ---------------------------------------------------------------------------
//  TIntfEdMainWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TIntfEdMainWnd::bChanges() const
{
    // Just pass it on to the editor window
    CIDAssert(m_pwndEditor != nullptr, L"The intf editor window is not set yet")
    return m_pwndEditor->bChanges();
}


tCIDLib::TBoolean TIntfEdMainWnd::bSaveChanges(TString& strErrMsg)
{
    // Just pass it on to the editor window
    CIDAssert(m_pwndEditor != nullptr, L"The intf editor window is not set yet")
    return m_pwndEditor->bSaveChanges(strErrMsg);
}


tCIDLib::TVoid
TIntfEdMainWnd::CreateIEWnd(const   TWindow&            wndParent
                            , const TCQCUserCtx&        cuctxToUse
                            , const tCIDCtrls::TWndId   widToUse
                            , const TString&            strTmplPath)
{
    //
    //  Let's try to download the template contents. We do that now before we
    //  create anything. So, if it fails, we just return and nothing to undo.
    //
    try
    {
        // Create the editor window object but not the window yet
        m_pwndEditor = new TIntfEditWnd();

        // Create our view object and give it the editor window pointer
        m_pcivEdit = new TCQCIntfEdView(m_pwndEditor, cuctxToUse);

        tCIDLib::TCard4         c4SerialNum = 0;
        tCIDLib::TEncodedTime   enctLastChange;
        TDataSrvClient          dsclLoad;
        THeapBuf                mbufData;
        tCIDLib::TCard4         c4ByteCnt;
        tCIDLib::TKVPFList      colMeta;

        //
        //  Return doesn't matter, since it just indicates new data or not. We
        //  are passing in a zero serial number, so we'll always get new data,
        //  or an exeption.
        //
        dsclLoad.bReadTemplate
        (
            strTmplPath
            , c4SerialNum
            , enctLastChange
            , mbufData
            , c4ByteCnt
            , colMeta
            , cuctxToUse.sectUser()
        );

        // Create a binary input stream over the buffer and stream it in
        TBinMBufInStream strmSrc(&mbufData, c4ByteCnt);
        strmSrc >> m_pcivEdit->iwdgBaseTmpl();

        //
        //  Store the template name into the template. This is important
        //  because it's used in error messages, or in save operations
        //  when in the edit and such.
        //
        m_pcivEdit->iwdgBaseTmpl().strTemplateName(strTmplPath);
    }

    catch(TError& errToCatch)
    {
        // Clean up the editor and view of we created them
        if (m_pwndEditor)
        {
            delete m_pwndEditor;
            m_pwndEditor = nullptr;
        }

        if (m_pcivEdit)
        {
            delete m_pcivEdit;
            m_pcivEdit = nullptr;
        }

        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        TErrBox msgbNew
        (
            wndParent.strWndText()
            , facCQCIntfEd().strMsg(kIEdErrs::errcGen_ReadFile)
        );
        msgbNew.ShowIt(*this);
        return;
    }

    //
    //  OK, we got the content so let's create this main container window and set it
    //  to have a left/right 2 panel layout. We are a child of the passed parent.
    //
    CreatePaneCont(wndParent, TArea(0, 0, 32, 32), widToUse);
    SetLayout(new TPaneLayout2Vert);

    //
    //  Set our color to the workspace color, to contrast against the windows
    //  we contain.
    //
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::AppWorkspace), kCIDLib::True);

    //
    //  Create the left side container and set it have two horizontal panes. It's a
    //  child of us.
    //
    m_pwndLeft = new TPaneWndCont();
    m_pwndLeft->CreatePaneCont(*this, TArea(0, 0, 256, 256), kCIDCtrls::widFirstCtrl + 1);
    m_pwndLeft->SetLayout(new TPaneLayout2Horz);

    //
    //  Create a scroll window that will hold the editor window. It's a child of the left
    //  side pane container.
    //
    m_pwndScroll = new TScrollArea();
    m_pwndScroll->Create
    (
        *m_pwndLeft
        , kCIDCtrls::widFirstCtrl
        , TArea(0, 0, 128, 128)
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EScrAStyles::None
    );

    //
    //  Create the error list. We need to pass a pointer to this to the editor window below
    //  so we do it first. It just needs the left side container to have been created, so
    //  we can do it here with no problems.
    //
    m_pwndErrList = new TIntfErrListWnd();
    m_pwndErrList->CreateErrList(*m_pwndLeft, kCIDCtrls::widFirstCtrl + 1);

    // Create the right side pane, which is a tabbed window
    m_pwndRight = new TTabbedWnd();
    m_pwndRight->CreateTabbedWnd
    (
        *this
        , kCIDCtrls::widFirstCtrl + 2
        , TArea(0, 0, 128, 128)
        , tCIDCtrls::EWndStyles::ClippingVisGrpChild
        , tCIDCtrls::ETabWStyles::None
        , tCIDCtrls::EExWndStyles::ControlParent
    );
    m_pwndRight->SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window), kCIDLib::True);

    //
    //  Create the attribute editor window, which is really our own derivative.
    //
    //  Actually this is a tab window since this has to be a tab. So the attribute editor
    //  is a child of it.
    //
    m_pwndAttrEdTab = new TIntfAttrEdTab();
    m_pwndRight->c4CreateTab
    (
        m_pwndAttrEdTab
        , L"Attributes"
        , kCIDLib::c4MaxCard
        , kCIDLib::True
        , tCIDCtrls::EWndStyles::ClippingVisChild
          , tCIDCtrls::EExWndStyles::ControlParent
    );

    // Create our attribute editor
    m_pwndAttrEd = new TIntfAttrEdWnd(m_pcivEdit, m_pwndEditor);
    m_pwndAttrEd->CreateAttrEd
    (
        *m_pwndAttrEdTab
        , kCIDCtrls::widFirstCtrl
        , TArea(0, 0, 128, 128)
        , tCIDCtrls::EWndStyles::VisTabChild
    );

    // Store it on the tab window, so he can keep it sized
    m_pwndAttrEdTab->m_pwndAttrEd = m_pwndAttrEd;

    // Create the find tab
    m_pwndFindTab = new TIntfFindWdgTab(m_pcivEdit);
    m_pwndRight->c4CreateTab
    (
        m_pwndFindTab
        , L"Find"
        , kCIDLib::c4MaxCard
        , kCIDLib::False
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
    );

    // Create the tools tab
    m_pwndToolsTab = new TIntfToolsTab();
    m_pwndRight->c4CreateTab
    (
        m_pwndToolsTab
        , L"Tools"
        , kCIDLib::c4MaxCard
        , kCIDLib::False
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
    );

    // Create the widget palette tab
    m_pwndWdgPal = new TIntfWidgetPalTab(m_pcivEdit);
    m_pwndRight->c4CreateTab
    (
        m_pwndWdgPal
        , L"Widgets"
        , kCIDLib::c4MaxCard
        , kCIDLib::False
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
    );


    //
    //  Now we can create the editor window and give it the view object and the
    //  attribute editor. We set it as a child of the scroll window.
    //
    m_pwndEditor->CreateEditWnd
    (
        *m_pwndScroll
        , m_pcivEdit
        , m_pwndAttrEd
        , m_pwndErrList
        , m_pwndWdgPal
        , kCIDCtrls::widFirstCtrl
    );

    //
    //  Get the width of the columns in the attribute editor. We'll try to use that as
    //  the initial size of that pane, if it's not too larger.
    //
    tCIDLib::TCard4 c4AttrEdGridWidth = m_pwndAttrEd->c4ColsWidth();
    tCIDLib::TCard4 c4AttrEdPaneWidth = 320;
    if (c4AttrEdGridWidth)
    {
        //
        //  Add space for a vertical scroll bar as well, plus a little for margins, to make
        //  sure we don't get a horizontal scroll bar.
        //
        c4AttrEdGridWidth += facCIDCtrls().c4ScrollBarWidth() + 8;

        if (c4AttrEdGridWidth < 280)
            c4AttrEdPaneWidth = c4AttrEdGridWidth;
        else
            c4AttrEdPaneWidth = 280;
    }

    // Set the two main panes
    SetPane(m_pwndLeft, 0, TSize(128, 0), TSize(128, 0));
    SetPane(m_pwndRight, 1, TSize(128, 0), TSize(c4AttrEdGridWidth, 0));

    //
    //  Set the two panes of the left side. Give the error list side enough space
    //  to hold it's initial height and make that the min as well. The editor area
    //  can scroll so it's less important. But make sure we don't give the error
    //  window ALL of the available vertical area.
    //
    tCIDLib::TCard4 c4ErrPaneCY = m_pwndErrList->areaWnd().c4Height();
    if (c4ErrPaneCY > (m_pwndLeft->areaWnd().c4Height() >> 1))
        c4ErrPaneCY = m_pwndLeft->areaWnd().c4Height() >> 1;

    m_pwndLeft->SetPane(m_pwndScroll, 0, TSize(0, 128), TSize(0, 128));
    m_pwndLeft->SetPane(m_pwndErrList, 1, TSize(0, c4ErrPaneCY), TSize(0, c4ErrPaneCY));

    // Initialize the panes now, nested one first
    m_pwndLeft->InitPanes(kCIDLib::False);
    InitPanes(kCIDLib::False);

    // Now let's wire up the editor window to the buttons in the tools tab
    m_pwndToolsTab->WireTo(*m_pwndEditor);
}


// The main tab needs access to the editor window's update topic
const TString& TIntfEdMainWnd::strEdUpdateTopic() const
{
    CIDAssert(m_pwndEditor != nullptr, L"strEdUpdateTopic() - Editor window is not set yet")
    return m_pwndEditor->strEdUpdateTopic();
}


// Get the template path that our editor instance is editing
const TString& TIntfEdMainWnd::strTmplPath() const
{
    // Just pass it on the editor window
    CIDAssert(m_pcivEdit != nullptr, L"strTmplPath() - View pointer is not set yet")
    return m_pcivEdit->iwdgBaseTmpl().strTemplateName();
}


// ---------------------------------------------------------------------------
//  TIntfEdMainWnd: Protected, inherited methods
// ---------------------------------------------------------------------------

//
//  Called from the pane window container (parent) class to let us set up the pane
//  window manager for our desired layout.
//
tCIDLib::TBoolean TIntfEdMainWnd::bCreated()
{
    if (!TParent::bCreated())
        return kCIDLib::False;

    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::AppWorkspace));

    return kCIDLib::True;
}


tCIDLib::TVoid TIntfEdMainWnd::Destroyed()
{

    return TParent::Destroyed();
}

