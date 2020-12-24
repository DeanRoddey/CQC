//
// FILE NAME: CQCIntfEd_Tools2.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/19/2018
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
//  This file is an overflow file. We break out the widget palette tool tab because
//  it's a bit more complex and the main file was already large enough.
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
RTTIDecls(TIntfWidgetPalPub,TObject)
RTTIDecls(TIntfWidgetPalTab,TTabWindow)



// ---------------------------------------------------------------------------
//  CLASS: TIntfWidgetPalPub
// PREFIX: wppub
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfWidgetPalPub: Constructors and destructor
// ---------------------------------------------------------------------------
TIntfWidgetPalPub::TIntfWidgetPalPub(const EEvents eEvent) :

    m_eEvent(eEvent)
{
}

TIntfWidgetPalPub::TIntfWidgetPalPub(const tCQCIntfEng::TDesIdList& fcolSel) :

    m_eEvent(EEvents::SelChanged)
    , m_fcolSel(fcolSel)
{
}

TIntfWidgetPalPub::~TIntfWidgetPalPub()
{
}



// ---------------------------------------------------------------------------
//  CLASS: TIntfWidgetPalTab
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TIntfWidgetPalTab: Constructors and destructor
// ---------------------------------------------------------------------------
TIntfWidgetPalTab::TIntfWidgetPalTab(TCQCIntfEdView* const pcivEdit) :

    TTabWindow(L"IntfEdWdgPal", TString::strEmpty(), kCIDLib::False, kCIDLib::True)
    , m_bSuppressPub(kCIDLib::False)
    , m_colCols(c4ColCount)
    , m_pcivEdit(pcivEdit)
    , m_pwndClient(nullptr)
    , m_pwndDel(nullptr)
    , m_pwndMoveBack(nullptr)
    , m_pwndMoveForward(nullptr)
    , m_pwndPal(nullptr)
{
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ColCount; c4Index++)
        m_colCols.objAdd(TString::strEmpty());
}

TIntfWidgetPalTab::~TIntfWidgetPalTab()
{
}


// ---------------------------------------------------------------------------
//  TIntfWidgetPalTab: Public, non-virtual methods
// ---------------------------------------------------------------------------

//
//  We can't do this initially because they won't be ready when we are first created. The
//  main window will call this once he gets the editor window initailized.
//
tCIDLib::TVoid TIntfWidgetPalTab::LoadWidgets()
{
    ReloadList();
}


// ---------------------------------------------------------------------------
//  TIntfWidgetPalTab: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid
TIntfWidgetPalTab::AreaChanged( const   TArea&                  areaPrev
                                , const TArea&                  areaNew
                                , const tCIDCtrls::EPosStates   ePosState
                                , const tCIDLib::TBoolean       bOrgChanged
                                , const tCIDLib::TBoolean       bSizeChanged
                                , const tCIDLib::TBoolean       bStateChanged)
{
    TParent::AreaChanged(areaPrev, areaNew, ePosState, bOrgChanged, bSizeChanged, bStateChanged);

    if (bSizeChanged && (ePosState != tCIDCtrls::EPosStates::Minimized) && m_pwndClient)
        m_pwndClient->SetSize(areaNew.szArea(), kCIDLib::True);
}


tCIDLib::TBoolean TIntfWidgetPalTab::bCreated()
{
    TParent::bCreated();

    // Load the dialog description
    TDlgDesc dlgdChildren;
    facCQCIntfEd().bCreateDlgDesc(kCQCIntfEd::ridDlg_WdgPal, dlgdChildren);

    //
    //  Create our generic window which will be sized to fit the tab. But initially set
    //  it to the size of the dialog content. So, when we create teh controls below
    //  they will have the correct initial relationship to this window.
    //
    m_pwndClient = new TGenericWnd();
    m_pwndClient->CreateGenWnd
    (
        *this
        , dlgdChildren.areaPos()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , kCIDCtrls::widFirstCtrl
    );

    //
    //  Now create the children. No need to do an initial sizing since we sized our
    //  own window initially to fit. Tell it to limit minimum size to the original
    //  dialog content size.
    //
    tCIDLib::TCard4 c4InitFocus;
    m_pwndClient->PopulateFromDlg
    (
        dlgdChildren, c4InitFocus, kCIDLib::False, kCIDLib::True
    );

    // Get refs to the ones we need
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_WdgPal_Del, m_pwndDel);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_WdgPal_Pal, m_pwndPal);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_WdgPal_MoveBack, m_pwndMoveBack);
    CastChildWnd(*m_pwndClient, kCQCIntfEd::ridDlg_WdgPal_MoveForward, m_pwndMoveForward);

    // Set up event handlers
    m_pwndDel->pnothRegisterHandler(this, &TIntfWidgetPalTab::eClickHandler);
    m_pwndMoveBack->pnothRegisterHandler(this, &TIntfWidgetPalTab::eClickHandler);
    m_pwndMoveForward->pnothRegisterHandler(this, &TIntfWidgetPalTab::eClickHandler);
    m_pwndPal->pnothRegisterHandler(this, &TIntfWidgetPalTab::eLBHandler);

    // Set up the columns on our mc list box which is the actual 'palette'
    m_colCols[c4ColInd_Id] = L"Widget Id";
    m_colCols[c4ColInd_Type] = L"Widget Type";
    m_colCols[c4ColInd_Area] = L"Widget Area";
    m_pwndPal->SetColumns(m_colCols);

    // Set up our subscription to updates from the editor window
    SubscribeToTopic(m_pcivEdit->strEdUpdateTopic(), kCIDLib::False);

    // Create our own topic
    m_pstopChanges = TPubSubTopic::pstopCreateTopic
    (
        m_pcivEdit->strWdgPalTopic(), TIntfWidgetPalPub::clsThis()
    );

    return kCIDLib::True;
}


tCIDLib::TVoid TIntfWidgetPalTab::ProcessPubMsg(const TPubSubMsg& psmsgIn)
{
    //
    //  In order to know what command went wrong if an exception happens, we use
    //  a pointer that we point to the incoming editor update msg.
    //
    const TIntfEdUpdateTopic* pieutCur = nullptr;
    try
    {
        //
        //  Suppress any publishing in response to changes we make here since we are
        //  responding to changes already made in the editor. If we don't do this
        //  we can end up responding to mark events in the list box handler and getting
        //  into a messy back and forth.
        //
        TBoolJanitor janSuppress(&m_bSuppressPub, kCIDLib::True);

        // Based on the event, let's update our list
        pieutCur = &psmsgIn.objMsgAs<TIntfEdUpdateTopic>();
        switch(pieutCur->m_eType)
        {
            case TIntfEdUpdateTopic::ETypes::BlockAdded :
                BlockAdded(pieutCur->m_c4Index1, pieutCur->m_c4Count);
                break;

            case TIntfEdUpdateTopic::ETypes::BlockRemoved :
                BlockRemoved(pieutCur->m_c4Index1, pieutCur->m_c4Count);
                break;

            case TIntfEdUpdateTopic::ETypes::Cleared :
                m_pwndPal->RemoveAll();
                break;

            case TIntfEdUpdateTopic::ETypes::Added :
                LoadWidget
                (
                    m_pcivEdit->iwdgBaseTmpl().iwdgAt(pieutCur->m_c4Index1)
                    , pieutCur->m_c4Index1
                );
                break;

            case TIntfEdUpdateTopic::ETypes::Changed :
                // If it's the template itself we ignore it
                if (pieutCur->m_c4Index1 != kCIDLib::c4MaxCard)
                {
                    UpdateWidget
                    (
                        m_pcivEdit->iwdgBaseTmpl().iwdgAt(pieutCur->m_c4Index1)
                        , pieutCur->m_c4Index1
                    );
                }
                break;

            case TIntfEdUpdateTopic::ETypes::Moved :
                m_pwndPal->MoveRow
                (
                    pieutCur->m_c4Index1, pieutCur->m_c4Index2, kCIDLib::True
                );
                break;

            case TIntfEdUpdateTopic::ETypes::Removed :
                m_pwndPal->RemoveAt(pieutCur->m_c4Index1);
                break;

            case TIntfEdUpdateTopic::ETypes::Reordered :
                ReloadList();
                break;

            case TIntfEdUpdateTopic::ETypes::SelectChanged :
            {
                const tCIDLib::TCard4 c4Count = m_pwndPal->c4ItemCount();
                for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
                {
                    tCIDLib::TCard8 c8Cur = m_pwndPal->c8UserDataAt(c4Index);
                    m_pwndPal->MarkAt(c4Index, pieutCur->m_fcolIdList.bElementPresent(c8Cur));
                }
                break;
            }

            case TIntfEdUpdateTopic::ETypes::Swapped :
                m_pwndPal->SwapRows
                (
                    pieutCur->m_c4Index1, pieutCur->m_c4Index2, kCIDLib::True
                );
                break;

            case TIntfEdUpdateTopic::ETypes::Enter :
            case TIntfEdUpdateTopic::ETypes::Exit :
                // We don't care about these
                break;

            default :
                #if CID_DEBUG_ON
                TErrBox msgbErr(L"Unknown IV editor update msg published");
                msgbErr.ShowIt(*this);
                #endif

                // Panic and reload to make sure we don't get out of sync
                ReloadList();
                break;
        };
    }

    catch(TError& errToCatch)
    {
        //
        //  In case we get out of sync and that causes an exception, we just fully
        //  reload our list and flush all current published msgs, to try to get
        //  back into sync.
        //
        errToCatch.AddStackLevel(CID_FILE, CID_LINE);
        TModule::LogEventObj(errToCatch);

        // If we got the actual info object try to log what it was
        if (pieutCur)
        {
            TString strInfo(1024UL);
            strInfo += L"Type=";
            strInfo.AppendFormatted(tCIDLib::TInt4(pieutCur->m_eType));
            strInfo += L", Indices=";
            strInfo.AppendFormatted(pieutCur->m_c4Index1);
            strInfo += L"/";
            strInfo.AppendFormatted(pieutCur->m_c4Index2);
            strInfo += L", Count=";
            strInfo.AppendFormatted(pieutCur->m_c4Count);

            facCQCIntfEd().LogMsg
            (
                CID_FILE
                , CID_LINE
                , kIEdErrs::errcWPal_MsgExcept
                , strInfo
                , tCIDLib::ESeverities::Failed
                , tCIDLib::EErrClasses::Unknown
            );
        }

        ReloadList();
    }
}


// ---------------------------------------------------------------------------
//  TIntfWidgetPalTab: Protected, inherited methods
// ---------------------------------------------------------------------------

// We need to add the widgets in this range
tCIDLib::TVoid
TIntfWidgetPalTab::BlockAdded(  const   tCIDLib::TCard4 c4Start
                                , const tCIDLib::TCard4 c4Count)
{
    TCQCIntfTemplate& iwdgTmpl = m_pcivEdit->iwdgBaseTmpl();
    if (!c4Start && (c4Count == iwdgTmpl.c4ChildCount()))
    {
        // It was a full reload
        ReloadList();
        return;
    }

    // Not a full reload, so we have to insert/append as required
    TWndPaintJanitor janPal(m_pwndPal);
    for (tCIDLib::TCard4 c4Index = c4Start; c4Index < c4Start + c4Count; c4Index++)
        LoadWidget(iwdgTmpl.iwdgAt(c4Index), c4Index);
}


// We need to remove the widgets in this range
tCIDLib::TVoid
TIntfWidgetPalTab::BlockRemoved(const   tCIDLib::TCard4 c4Start
                                , const tCIDLib::TCard4 c4Count)
{
    // Shouldn't ever be zero, but just in case, we don't want to make a mess
    if (c4Count)
        m_pwndPal->RemoveRange(c4Start, c4Start + c4Count - 1);
}


//
//  We publish msgs to indicate what the user requested. The editor window subscribes
//  to our topic and updates accordingly.
//
tCIDCtrls::EEvResponses
TIntfWidgetPalTab::eClickHandler(TButtClickInfo& wnotEvent)
{
    TIntfWidgetPalPub::EEvents eEvent;
    switch(wnotEvent.widSource())
    {
        case kCQCIntfEd::ridDlg_WdgPal_Del :
            eEvent = TIntfWidgetPalPub::EEvents::Del;
            break;

        case kCQCIntfEd::ridDlg_WdgPal_MoveBack :
            eEvent = TIntfWidgetPalPub::EEvents::MoveBack;
            break;

        case kCQCIntfEd::ridDlg_WdgPal_MoveForward :
            eEvent = TIntfWidgetPalPub::EEvents::MoveForward;
            break;

        default :
        {
            #if CID_DEBUG_ON
            TErrBox msgbErr(L"Uknown widget pal button click id");
            msgbErr.ShowIt(*this);
            #endif
            return tCIDCtrls::EEvResponses::Handled;
        }
    };

    m_pstopChanges.Publish(new TIntfWidgetPalPub(eEvent));
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We respond to changes in mark state and send our list of marked widgtes. The
//  editor window will update accordingly. The editor won't send us back anything
//  in this, since we've already done it, so it could get weird.
//
tCIDCtrls::EEvResponses TIntfWidgetPalTab::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Marked)
    {
        const tCIDLib::TCard4 c4Count = m_pwndPal->c4ItemCount();
        tCQCIntfEng::TDesIdList fcolSel;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            if (m_pwndPal->bIsMarked(c4Index))
                fcolSel.c4AddElement(m_pwndPal->c8UserDataAt(c4Index));
        }

        // If publishing isn't suppressed report this change
        if (!m_bSuppressPub)
            m_pstopChanges.Publish(new TIntfWidgetPalPub(fcolSel));

        //
        //  Enable/disable our up/down buttons based on the selection. If there are
        //  none, or there is more than one, we disable them. Or, if it is a single
        //  widget but it's the template.
        //
        //  We do this here so that we get it whether we are reacting to selection
        //  changes from the editor, or causing them ourself.
        //
        const tCIDLib::TBoolean bCanMove
        (
            (fcolSel.c4ElemCount() == 1)
            && (fcolSel[0] != m_pcivEdit->iwdgBaseTmpl().c8DesId())
        );
        m_pwndMoveBack->SetEnable(bCanMove);
        m_pwndMoveForward->SetEnable(bCanMove);

        // And the delete can be enabled as long as its not the template
        m_pwndDel->SetEnable
        (
            (fcolSel.c4ElemCount() != 0)
            && (fcolSel[0] != m_pcivEdit->iwdgBaseTmpl().c8DesId())
        );
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Loads a single widget into the list
tCIDLib::TVoid
TIntfWidgetPalTab::LoadWidget(const TCQCIntfWidget& iwdgSrc, const tCIDLib::TCard4 c4At)
{
    m_colCols[c4ColInd_Id] = iwdgSrc.strWidgetId();
    m_colCols[c4ColInd_Type] = iwdgSrc.strTypeName();
    m_colCols[c4ColInd_Area].SetFormatted(iwdgSrc.areaRelative());
    m_pwndPal->c4AddItem(m_colCols, 0, c4At);

    m_pwndPal->SetUserDataAt(c4At, iwdgSrc.c8DesId());
}


// Just reload our whole list
tCIDLib::TVoid TIntfWidgetPalTab::ReloadList()
{
    TWndPaintJanitor janPal(m_pwndPal);

    m_pwndPal->RemoveAll();
    TCQCIntfTemplate& iwdgTmpl = m_pcivEdit->iwdgBaseTmpl();
    const tCIDLib::TCard4 c4Count = iwdgTmpl.c4ChildCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        LoadWidget(iwdgTmpl.iwdgAt(c4Index), c4Index);
}


tCIDLib::TVoid
TIntfWidgetPalTab::UpdateWidget(const TCQCIntfWidget& iwdgSrc, const tCIDLib::TCard4 c4At)
{
    m_colCols[c4ColInd_Id] = iwdgSrc.strWidgetId();
    m_colCols[c4ColInd_Type] = iwdgSrc.strTypeName();
    m_colCols[c4ColInd_Area].SetFormatted(iwdgSrc.areaRelative());
    m_pwndPal->UpdateRowAt(m_colCols, c4At, 0);

    m_pwndPal->SetUserDataAt(c4At, iwdgSrc.c8DesId());
}
