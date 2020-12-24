//
// FILE NAME: CQCIntfEd_EditTrigEvsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/26/2016
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
//  This file implements a dialog that let's the user edit the triggered events he wants
//  associated with a template.
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
#include    "CQCIntfEd_EdTrigEvsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdTrigEvsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//  CLASS: TEdTrigEvsDlg
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdTrigEvsDlg: Constructors and destructor
// ---------------------------------------------------------------------------
TEdTrigEvsDlg::TEdTrigEvsDlg() :

    m_piwdgWorking(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndRemove(nullptr)
    , m_pwndRemoveAll(nullptr)
    , m_pwndEdit(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdTrigEvsDlg::~TEdTrigEvsDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdTrigEvsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdTrigEvsDlg::bRun(const TWindow& wndOwner, TCQCIntfTemplate* const piwdgToEdit)
{
    // Save the working template widget, and make a copy to edit
    m_piwdgWorking = piwdgToEdit;
    m_iwdgOrg = *piwdgToEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_EdTrigEvs
    );

    if (c4Res == kCQCIntfEd::ridDlg_EdTrigEvs_Save)
        return kCIDLib::True;

    // They didn't savem, so make sure we undo any changes
    *piwdgToEdit = m_iwdgOrg;
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdTrigEvsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdTrigEvsDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_Add, m_pwndAdd);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_Remove, m_pwndRemove);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_RemoveAll, m_pwndRemoveAll);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_Edit, m_pwndEdit);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdTrigEvs_Save, m_pwndSave);

    // Hook up any event handlers we need
    m_pwndAdd->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);
    m_pwndRemove->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);
    m_pwndRemoveAll->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);
    m_pwndEdit->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEdTrigEvsDlg::eLBHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdTrigEvsDlg::eClickHandler);

    // Load up any existing events
    LoadEventList();

    //
    //  If no incoming events, disable the edit and delete buttons, else select the
    //  first one.
    //
    if (m_pwndList->c4ItemCount())
    {
        m_pwndList->SelectByIndex(0);
    }
     else
    {
        m_pwndEdit->SetEnable(kCIDLib::False);
        m_pwndRemove->SetEnable(kCIDLib::False);
        m_pwndRemoveAll->SetEnable(kCIDLib::False);
    }

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdTrigEvsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdTrigEvsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_Add)
    {
        // Let them edit a new one and add it if they want
        EditEvent(kCIDLib::c4MaxCard);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_Cancel)
    {
        EndDlg(kCQCIntfEd::ridDlg_EdTrigEvs_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_Remove)
    {
        // If we've got one selected, then delete that one
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            tCQCEvCl::TTrgEvList& colEvs = m_piwdgWorking->colTrgEvents();

            // Make sure they really want to do it
            TYesNoBox msgbDel
            (
                facCQCIntfEd().strMsg
                (
                    kIEdMsgs::midQ_DeleteEvent, colEvs[c4Index].strTitle()
                )
            );

            if (msgbDel.bShowIt(*this))
            {
                colEvs.RemoveAt(c4Index);
                m_pwndList->RemoveAt(c4Index);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_RemoveAll)
    {
        // Make sure they really want to do it
        TYesNoBox msgbDel(facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteAllEvents));
        if (msgbDel.bShowIt(*this))
        {
            m_piwdgWorking->colTrgEvents().RemoveAll();
            m_pwndList->RemoveAll();
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_Edit)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
            EditEvent(c4Index);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_Save)
    {
        EndDlg(kCQCIntfEd::ridDlg_EdTrigEvs_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// We just respond to double clicks on the list items to allow for editing
tCIDCtrls::EEvResponses TEdTrigEvsDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdTrigEvs_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
            if (c4Index != kCIDLib::c4MaxCard)
                EditEvent(c4Index);
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
        {
            m_pwndRemoveAll->SetEnable(kCIDLib::False);
            m_pwndRemove->SetEnable(kCIDLib::False);
            m_pwndEdit->SetEnable(kCIDLib::False);

            //
            //  If this happens, then the button they just clicked has to be active
            //  one, so we need to move it.
            //
            m_pwndSave->TakeFocus();
            m_pwndSave->SetDefButton();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            m_pwndRemoveAll->SetEnable(kCIDLib::True);
            m_pwndRemove->SetEnable(kCIDLib::True);
            m_pwndEdit->SetEnable(kCIDLib::True);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We have to invoke the editing dialog from more than one place and there's
//  a bit of setup to do it, so we split it out.
//
tCIDLib::TVoid TEdTrigEvsDlg::EditEvent(const tCIDLib::TCard4 c4Index)
{
    // We need to find all the named widgets
    tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, 64);

    TCQCIntfContainer* piwdgParent = m_piwdgWorking->piwdgParent();
    if (!piwdgParent)
        piwdgParent = dynamic_cast<TCQCIntfContainer*>(m_piwdgWorking);
    piwdgParent->FindAllTargets(colExtraTars, 0);

    // Add the template's view itself
    colExtraTars.Add(&m_piwdgWorking->civOwner());

    //
    //  Get a ref to the event to edit. It's either an existing one, or
    //  it's a new one, so we use a local temp in that case.
    //
    tCQCEvCl::TTrgEvList& colEvs = m_piwdgWorking->colTrgEvents();
    TCQCTrgEvent    csrcNew(tCQCKit::EActCmdCtx::IVEvent);
    TCQCTrgEvent&   csrcEdit
    (
        (c4Index == kCIDLib::c4MaxCard) ? csrcNew : colEvs[c4Index]
    );

    TEditTrigEvDlg dlgEdit;
    if (dlgEdit.bRunDlg(*this, csrcEdit, &colExtraTars, m_piwdgWorking->civOwner().cuctxToUse()))
    {
        //
        //  They committed, so either add it to our window list or update
        //  the curren item, according to whether it's a new one or we are
        //  editing an existing one.
        //
        if (c4Index == kCIDLib::c4MaxCard)
        {
            colEvs.objAdd(csrcEdit);
            m_pwndList->c4AddItem(csrcEdit.strTitle());

            // If there's no selection in the list, this must be the first, so select it
            if (m_pwndList->c4ItemCount() == 1)
                m_pwndList->SelectByIndex(0, kCIDLib::True);
        }
         else
        {
            // They may have changed the name
            m_pwndList->c4SetText(c4Index, csrcEdit.strTitle());
        }
    }
}


//
//  We have to load up the list of events in more than one place, so we
//  break out here to a helper.
//
tCIDLib::TVoid TEdTrigEvsDlg::LoadEventList()
{
    // Remove any previous contents
    m_pwndList->RemoveAll();

    //
    //  Load up all of the events in the list. We just load them in order
    //  so that the list window index is the same as the list index.
    //
    tCQCEvCl::TTrgEvList& colEvs = m_piwdgWorking->colTrgEvents();
    const tCIDLib::TCard4 c4Count = colEvs.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCTrgEvent& csrcCur = colEvs[c4Index];
        m_pwndList->c4AddItem(csrcCur.strTitle());
    }
}

