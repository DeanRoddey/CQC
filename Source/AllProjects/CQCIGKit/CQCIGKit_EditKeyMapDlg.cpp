//
// FILE NAME: CQCIGKit_EdKeyMapsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/09/2001
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
//  This file implements the standard keymaps editor/viewer dialog.
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
#include    "CQCIGKit_.hpp"
#include    "CQCIGKit_EditCmdDlg_.hpp"
#include    "CQCIGKit_EditKeyMapDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCKeyMapDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TCQCKeyMapDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCKeyMapDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCKeyMapDlg::TCQCKeyMapDlg(const  tCQCIGKit::EKMActModes  eMode
                            , const TCQCUserCtx&            cuctxToUse) :

    m_crtsStd(cuctxToUse)
    , m_cuctxToUse(cuctxToUse)
    , m_eMode(eMode)
    , m_pkmOrg(nullptr)
    , m_pwndCloseButton(nullptr)
    , m_pwndCtrlShift(nullptr)
    , m_pwndDeleteButton(nullptr)
    , m_pwndDetails(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndShift(nullptr)
{
}

TCQCKeyMapDlg::~TCQCKeyMapDlg()
{

}


// ---------------------------------------------------------------------------
//  TCQCKeyMapDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCKeyMapDlg::bRun(const   TWindow&    wndOwner
                    , const TString&    strAccount
                    ,       TCQCKeyMap& kmToEdit)
{
    // Make a copy of the incoming keymap data
    m_kmToEdit = kmToEdit;

    // And a pointer to the original
    m_pkmOrg = &kmToEdit;

    // And store the account name until we can use it
    m_strAccount = strAccount;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_EdKeyMap
    );

    if (c4Res == kCQCIGKit::ridDlg_EdKeyMap_Save)
    {
        //
        //  Put our edited version back if we are in edit mode.
        //
        if (m_eMode == tCQCIGKit::EKMActModes::Edit)
            kmToEdit = m_kmToEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCKeyMapDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_Close, m_pwndCloseButton);
    CastChildWnd(*this,kCQCIGKit::ridDlg_EdKeyMap_CtrlShift,m_pwndCtrlShift);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_Delete,m_pwndDeleteButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_Details, m_pwndDetails);
    CastChildWnd(*this,kCQCIGKit::ridDlg_EdKeyMap_Edit,m_pwndEditButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_List, m_pwndList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_Save, m_pwndSaveButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdKeyMap_Shift, m_pwndShift);

    // Register event handlers
    m_pwndCloseButton->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);
    m_pwndCtrlShift->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TCQCKeyMapDlg::eLBHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);
    m_pwndShift->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);

    // Set up the list columns
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L"Used");
    colCols.objAdd(L"Key Name");
    m_pwndList->SetColumns(colCols);

    // Size the first column to the header
    m_pwndList->AutoSizeCol(0, kCIDLib::True);

    // Do mode specific stuff
    if (m_eMode == tCQCIGKit::EKMActModes::Edit)
    {
        //  We are in edit mode, so register them with their handler.
        m_pwndDeleteButton->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);
        m_pwndEditButton->pnothRegisterHandler(this, &TCQCKeyMapDlg::eClickHandler);

        // Put the account name in the title
        TString strTitle = strWndText();
        strTitle.Append(L" [");
        strTitle.Append(m_strAccount);
        strTitle.Append(L"]");
        strWndText(strTitle);
    }
     else
    {
        // Disable the delete and edit buttons, since we won't use them here.
        m_pwndDeleteButton->SetEnable(kCIDLib::False);
        m_pwndEditButton->SetEnable(kCIDLib::False);

        // And set up the save button with the 'Run' text
        m_pwndSaveButton->strWndText(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_EdKeyMap_Exec));

        //
        //  Get rid of the details and move the buttons up. We put the buttons so that their
        //  tops are where the top of the details separator is now.
        //
        TWindow* pwndDel = pwndChildById(kCQCIGKit::ridDlg_EdKeyMap_DetailsSep);
        const TArea areaSep = pwndDel->areaWnd();
        pwndDel->Destroy();
        delete pwndDel;

        pwndDel = pwndChildById(kCQCIGKit::ridDlg_EdKeyMap_Details);
        pwndDel->Destroy();
        delete pwndDel;

        TArea areaNew = m_pwndCloseButton->areaWnd();
        areaNew.i4Y(areaSep.i4Y());
        m_pwndCloseButton->SetPos(areaNew.pntOrg());

        areaNew = m_pwndDeleteButton->areaWnd();
        areaNew.i4Y(areaSep.i4Y());
        m_pwndDeleteButton->SetPos(areaNew.pntOrg());

        areaNew = m_pwndEditButton->areaWnd();
        areaNew.i4Y(areaSep.i4Y());
        m_pwndEditButton->SetPos(areaNew.pntOrg());

        areaNew = m_pwndSaveButton->areaWnd();
        areaNew.i4Y(areaSep.i4Y());
        m_pwndSaveButton->SetPos(areaNew.pntOrg());

        //
        //  And size us up to fit this reduced stuff. We have to adjust the min/max sizes
        //  set in order to allow this. The base dialog box will, if the dialog is not marked
        //  as sizeable, set this min/max stuff to the initial size.
        //
        TArea areaUs = areaWndSize();
        areaUs.i4Bottom(areaNew.i4Bottom() + 16);
        TFrameWnd::AreaForClient
        (
            areaUs
            , areaNew
            , eWndStyles()
            , eExWndStyles()
            , kCIDLib::False
        );

        SetMinSize(areaNew.szArea());
        SetSize(areaNew.szArea(), kCIDLib::True);
    }

    // Fill in the list with the correct list of mappings for our current shift state.
    FillList();

    // And select the first one
    m_pwndList->SelectByIndex(0);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCKeyMapDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TCQCKeyMapDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_Close)
    {
        // If any changes, ask if they want to save them
        if (*m_pkmOrg != m_kmToEdit)
        {
            TYesNoBox msgbAsk(facCQCIGKit().strMsg(kIGKitMsgs::midQ_SaveChanges));

            // If they want to save, then do it
            if (msgbAsk.bShowIt(*this))
                EndDlg(kCQCIGKit::ridDlg_EdKeyMap_Save);
            else
                EndDlg(kCQCIGKit::ridDlg_EdKeyMap_Close);
        }
         else
        {
            EndDlg(kCQCIGKit::ridDlg_EdKeyMap_Close);
        }
    }
     else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_CtrlShift)
          ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_Shift))
    {
        // Update the list box with the appropriate list
        FillList();
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_Delete)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            // Turn the index into a function key
            const tCIDLib::TBoolean bCtrlShift = m_pwndCtrlShift->bCheckedState();
            const tCIDLib::TBoolean bShift = m_pwndShift->bCheckedState();
            TCQCKeyMap::TItemList& colKeys = m_kmToEdit.colForShifts(bShift, bCtrlShift);

            // And now we can get the item to update
            TCQCKeyMapItem& kmiCur = colKeys[c4Index];

            // Make sure they want to do it
            TYesNoBox msgbAsk
            (
                facCQCIGKit().strMsg
                (
                    kIGKitMsgs::midQ_DeleteMapping, TCQCKeyMap::strKeyName(kmiCur.eKey())
                )
            );

            if (msgbAsk.bShowIt(*this))
            {
                //
                //  If its used, then mark it unused, enable save, and flag changes.
                //  We clear out the details list since this one no longer has any
                //  commands.
                //
                if (kmiCur.bUsed())
                {
                    // Clear the details just in case we are using it
                    m_pwndDetails->ClearText();

                    // And mark this one unused and uncheck it
                    kmiCur.bUsed(kCIDLib::False);
                    m_pwndList->SetCheckAt(c4Index, kCIDLib::False);
                }
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_Save)
    {
        if (m_eMode == tCQCIGKit::EKMActModes::Edit)
        {
            // If no changes return close so they won't redundantly save
            if (*m_pkmOrg != m_kmToEdit)
                EndDlg(kCQCIGKit::ridDlg_EdKeyMap_Save);
            else
                EndDlg(kCQCIGKit::ridDlg_EdKeyMap_Close);
        }
         else
        {
            ExecItem();
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_Edit)
    {
        EditItem();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TCQCKeyMapDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  If invoked, then act like the Edit button was pressed and edit this item. If
    //  selection changed , then load up the list of commands for this action, if it's
    //  used.
    //
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdKeyMap_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        {
            ShowDetails();
        }
         else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            if (m_eMode == tCQCIGKit::EKMActModes::Edit)
                EditItem();
            else
                ExecItem();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCKeyMapDlg::EditItem()
{
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return;

    // Get the target
    const tCIDLib::TBoolean bCtrlShift = m_pwndCtrlShift->bCheckedState();
    const tCIDLib::TBoolean bShift = m_pwndShift->bCheckedState();
    TCQCKeyMap::TItemList& colKeys = m_kmToEdit.colForShifts(bShift, bCtrlShift);
    TCQCKeyMapItem& kmiEdit = colKeys[c4Index];

    //
    //  Let the user edit this item. They edit a copy and we only save it it back to
    //  the orignal if they commit. We don't have any extra targets, so we pass in
    //  an empty list. Indicate the action is not required.
    //
    TCQCEditCmdDlg dlgEdit;
    tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt);
    if (dlgEdit.bRun(*this, kmiEdit, kCIDLib::False, m_cuctxToUse, &colExtraTars))
    {
        //
        //  If the item has some steps defined, then make sure it's marked
        //  used, else mark it unused.
        //
        const tCIDLib::TBoolean bEnableIt(kmiEdit.c4OpCount(kCQCKit::strEvId_OnPress) > 0);
        kmiEdit.bUsed(bEnableIt);
        m_pwndList->SetCheckAt(c4Index, bEnableIt);
        ShowDetails();
    }
}


tCIDLib::TVoid TCQCKeyMapDlg::ExecItem()
{
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return;

    // Get the target
    const tCIDLib::TBoolean bCtrlShift = m_pwndCtrlShift->bCheckedState();
    const tCIDLib::TBoolean bShift = m_pwndShift->bCheckedState();
    TCQCKeyMap::TItemList& colKeys = m_kmToEdit.colForShifts(bShift, bCtrlShift);
    TCQCKeyMapItem& kmiExec = colKeys[c4Index];

    // And invoke it if its used
    if (kmiExec.bUsed())
    {
        // Provide a dummy global variables target
        facCQCIGKit().bInvokeAction
        (
            *this, *this, kmiExec, kCQCKit::strEvId_OnPress, strWndText(), m_cuctxToUse
        );
    }
}


tCIDLib::TVoid TCQCKeyMapDlg::FillList()
{
    // Get the appropriate list of mappings for our shift state
    TCQCKeyMap::TItemList& colSrc = m_kmToEdit.colForShifts
    (
        m_pwndShift->bCheckedState(), m_pwndCtrlShift->bCheckedState()
    );

    // Avoid blinkies while doing this
    TWndPaintJanitor janPaint(m_pwndList);

    const tCIDLib::TCard4 c4Count = colSrc.c4ElemCount();
    m_pwndList->RemoveAll();
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCKeyMapItem& kmiCur = colSrc[c4Index];

        colCols[0] = TString::strEmpty();
        colCols[1] = TCQCKeyMap::strKeyName(kmiCur.eKey());
        tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, 0);
        m_pwndList->SetCheckAt(c4At, kmiCur.bUsed());
    }
}


tCIDLib::TVoid TCQCKeyMapDlg::ShowDetails()
{
    // We onyl do this if in edit mode
    if (m_eMode != tCQCIGKit::EKMActModes::Edit)
        return;

    // Get the current selection. Just in case, if none, then just clear and return
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
    {
        m_pwndDetails->ClearText();
        return;
    }

    // Get the target key mapped action
    const tCIDLib::TBoolean bCtrlShift = m_pwndCtrlShift->bCheckedState();
    const tCIDLib::TBoolean bShift = m_pwndShift->bCheckedState();
    TCQCKeyMap::TItemList& colKeys = m_kmToEdit.colForShifts(bShift, bCtrlShift);
    TCQCKeyMapItem& kmiExec = colKeys[c4Index];

    if (kmiExec.bUsed())
    {
        // Ask this guy to format himself
        TTextStringOutStream strmFmt(2048UL);
        kmiExec.FormatOpsForEvent(kCQCKit::strEvId_OnPress, strmFmt);
        m_pwndDetails->strWndText(strmFmt.strData());
    }
     else
    {
        m_pwndDetails->ClearText();
    }
}

