//
// FILE NAME: CQCIntfEd_ErrList.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/28/2016
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
//  This file implements the error list window.
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


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TIntfErrListTopic, TObject)
RTTIDecls(TIntfErrListWnd,TGenericWnd)



// ---------------------------------------------------------------------------
//  CLASS: TIntfErrListWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfErrListWnd: Constructors and Destructor
// ---------------------------------------------------------------------------
TIntfErrListWnd::TIntfErrListWnd() :

    m_pwndList(nullptr)
    , m_pwndTitle(nullptr)
    , m_strType_Error(L"Error")
    , m_strType_Warning(L"Warning")
{
}

TIntfErrListWnd::~TIntfErrListWnd()
{
}


// -------------------------------------------------------------------
//  TIntfErrListWnd: Public, inherited methods
// -------------------------------------------------------------------

//
//  If our list window gets the focus, send a selection event so that the editor can get
//  the focus back on the offending widget, if it still exists.
//
tCIDLib::TVoid
TIntfErrListWnd::ChildFocusChange(  const   TWindow&                wndParent
                                    , const tCIDCtrls::TWndId       widChild
                                    , const tCIDLib::TBoolean       bGotFocus)
{
    if (wndParent.bIsThisWindow(*this) && (widChild == kCQCIntfEd::ridDlg_ErrList_Errs))
    {
        if (bGotFocus)
        {
            const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
            if (c4ListInd != kCIDLib::c4MaxCard)
                SendErrNot(tCIDCtrls::EListEvents::SelChanged, c4ListInd);
        }
    }
}


// -------------------------------------------------------------------
//  TIntfErrListWnd: Public, non-virtual methods
// -------------------------------------------------------------------

//
//  If no errors to load, this is called to clear the list. But we add an empty one back
//  and select it so that we have a focus indicator
//
tCIDLib::TVoid TIntfErrListWnd::ClearList()
{
    m_pwndList->RemoveAll();
}



tCIDLib::TVoid
TIntfErrListWnd::CreateErrList(const TWindow& wndParent, const tCIDCtrls::TWndId widToUse)
{
    // Create our publish topic
    m_strPublishTopic = L"/CQC/IntfEng/ErrList/";
    m_strPublishTopic.AppendFormatted(facCIDLib().c4NextId());
    m_pstopErrs = TPubSubTopic::pstopCreateTopic
    (
        m_strPublishTopic, TIntfErrListTopic::clsThis()
    );

    //
    //  Load the dialog description so we can create ourself the same size as the
    //  content initially. This way our parent's auto-sizing code will keep the correct
    //  relationship to the content.
    //
    TDlgDesc dlgdChildren;
    facCQCIntfEd().bCreateDlgDesc(kCQCIntfEd::ridDlg_ErrList, dlgdChildren);
    TArea areaInit(dlgdChildren.areaPos());
    areaInit.ZeroOrg();

    CreateGenWnd
    (
        wndParent
        , areaInit
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , widToUse
    );

}


//
//  The editor calls us here to load up the errors he's found. We don't do an initial
//  selection, because we don't want to force the selection away from whatever widget(s)
//  the user currently has selected. This way, we only do anything if they actively select
//  an item.
//
tCIDLib::TVoid TIntfErrListWnd::LoadErrors(const tCQCIntfEng::TErrList& colErrs)
{
    // Stop redrawing while we do this
    TWndPaintJanitor janErrs(m_pwndList);

    // Remove any existing errors
    m_pwndList->RemoveAll();

    //
    //  Store the list for later use. We have to do this before we start loading the list
    //  because that will generate events that will reference this list.
    //
    m_colErrs = colErrs;

    // Load up the new ones
    tCIDLib::TStrList colCols(3);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    const tCIDLib::TCard4 c4Count = colErrs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfValErrInfo& veiCur = colErrs[c4Index];

        if (veiCur.bWarning())
            colCols[0] = m_strType_Warning;
        else
            colCols[0] = m_strType_Error;

        colCols[1] = veiCur.strWidgetId();
        colCols[2] = veiCur.strErrText();

        // Set the error's unique error id as the row id
        m_pwndList->c4AddItem(colCols, veiCur.c4ErrId(), kCIDLib::c4MaxCard, kCIDLib::False);
    }
}


// -------------------------------------------------------------------
//  TIntfErrListWnd: Protected, inherited methods
// -------------------------------------------------------------------

tCIDLib::TBoolean TIntfErrListWnd::bCreated()
{
    TParent::bCreated();

    // Load our dialog description and create our children
    TDlgDesc dlgdChildren;
    facCQCIntfEd().bCreateDlgDesc(kCQCIntfEd::ridDlg_ErrList, dlgdChildren);
    tCIDLib::TCard4 c4InitFocus;
    PopulateFromDlg(dlgdChildren, c4InitFocus);

    // Set our bgn to standard window background color
    SetBgnColor(facCIDCtrls().rgbSysClr(tCIDCtrls::ESysColors::Window));

    // Get any child controls we need to interact with
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ErrList_Errs, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_ErrList_Title, m_pwndTitle);

    // Initially set the title text to indicate no focus
//    m_pwndTitle->SetTextColor(facCIDGraphDev().rgbDarkGrey);

    // Set up the columns on the list window
    tCIDLib::TStrList colCols(3);
    colCols.objAdd(L"Type       ");
    colCols.objAdd(L"Widget Id     ");
    colCols.objAdd(L"Error Message");
    m_pwndList->SetColumns(colCols);

    m_pwndList->AutoSizeCol(0, kCIDLib::True);
    m_pwndList->AutoSizeCol(1, kCIDLib::True);

    // We need to react to list selection/invocation
    m_pwndList->pnothRegisterHandler(this, &TIntfErrListWnd::eLBHandler);

    // Indicate we want to see child focus changes
    bWantsChildFocusNot(kCIDLib::True);

    return kCIDLib::True;
}


// -------------------------------------------------------------------
//  TIntfErrListWnd: Private, non-virtual methods
// -------------------------------------------------------------------

//
//  We respond to selection and invocation events, and publish the appropriate
//  message.
//
tCIDCtrls::EEvResponses
TIntfErrListWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    SendErrNot(wnotEvent.eEvent(), wnotEvent.c4Index());
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Finds the error for the selected list window row and sends teh appropriate notification
//  to any registered handlers.
//
tCIDLib::TVoid
TIntfErrListWnd::SendErrNot(const   tCIDCtrls::EListEvents  eEvent
                            , const tCIDLib::TCard4         c4Index)
{
    tCIDLib::TInt4 i4Code = 0;
    if ((eEvent == tCIDCtrls::EListEvents::SelChanged)
    ||  (eEvent == tCIDCtrls::EListEvents::Invoked))
    {
        //
        //  The list window row id is the unique error id, so we have to look through the
        //  list and find it in order to find the right error.
        //
        const tCIDLib::TCard4 c4ErrId = m_pwndList->c4IndexToId(c4Index);
        const tCIDLib::TCard4 c4Count = m_colErrs.c4ElemCount();
        tCIDLib::TCard4 c4ListInd = 0;
        for (; c4ListInd < c4Count; c4ListInd++)
        {
            if (m_colErrs[c4ListInd].c4ErrId() == c4ErrId)
                break;
        }

        // Shouldn't happen but just in case make sure we found it
        if (c4ListInd != c4Count)
        {
            const TCQCIntfValErrInfo& veiCur = m_colErrs[c4ListInd];
            const tCIDLib::TCard4 c4WidgetId = veiCur.c4WidgetId();

            m_pstopErrs.Publish
            (
                new TIntfErrListTopic(eEvent, c4WidgetId)
            );

            // Ok, let's remove this one from the list if an invoke
            if (eEvent == tCIDCtrls::EListEvents::Invoked)
                m_pwndList->RemoveAt(c4Index);
        }
    }
}
