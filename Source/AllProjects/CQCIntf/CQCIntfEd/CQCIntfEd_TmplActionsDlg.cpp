//
// FILE NAME: CQCAdmin_TmplActionsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/23/2015
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
//  This file implements a dialog that let's the user edit the named actions associated
//  with a template.
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
#include    "CQCIntfEd_TmplActionsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTmplActionsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TTmplActionsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTmplActionsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TTmplActionsDlg::TTmplActionsDlg() :

    m_c4NextId(1)
    , m_pcolExtraTars(nullptr)
    , m_pwndActText(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDel(nullptr)
    , m_pwndEdit(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSave(nullptr)
{
}

TTmplActionsDlg::~TTmplActionsDlg()
{
}


// ---------------------------------------------------------------------------
//  TTmplActionsDlg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TTmplActionsDlg::bIPETestCanEdit(const  tCIDCtrls::TWndId
                                , const tCIDLib::TCard4
                                ,       TAttrData&
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // We only have one column so it's always editable
    bValueSet = kCIDLib::False;
    return kCIDLib::True;
}


// Just make sure it's not empty, after passing it to the base class for other validation
tCIDLib::TBoolean
TTmplActionsDlg::bIPEValidate(  const   TString&            strSrc
                                ,       TAttrData&          adatTar
                                , const TString&            strNewVal
                                ,       TString&            strErrMsg
                                ,       tCIDLib::TCard8&    c8UserId) const
{
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    if (strNewVal.bIsEmpty())
    {
        strErrMsg = L"The action name cannot be empty";
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDLib::TVoid
TTmplActionsDlg::IPEValChanged( const   tCIDCtrls::TWndId   widSrc
                                , const TString&            strSrc
                                , const TAttrData&          adatNew
                                , const TAttrData&          adatOld
                                , const tCIDLib::TCard8     c8UserId)
{
    // We override this to allow us to update our data
    MIPEIntf::IPEValChanged(widSrc, strSrc, adatNew, adatOld, c8UserId);

    // Find this one in the vector and list and update it
    const tCIDLib::TCard4 c4ColInd = c4ListToColInd(MIPEIntf::c4IPERow());
    m_colEdit[c4ColInd].strTitle(adatNew.strValue());

    // This doesn't automatically resort, so force that
    m_pwndList->Resort();
}



// ---------------------------------------------------------------------------
//  TTmplActionsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTmplActionsDlg::bRun(  const   TWindow&                wndOwner
                        , const TString&                strTmplBasePath
                        ,       tCQCIntfEng::TActList&  colActions
                        ,       tCQCKit::TCmdTarList&   colExtraTars
                        , const TCQCUserCtx&            cuctxToUse)
{
    //
    //  Make a copy of the list for our editing purposes, and a pointer so that we can
    //  check for modifications. And store other stuff for later use as well.
    //
    m_pcolOrgActs = &colActions;
    m_colEdit = colActions;
    m_pcuctxToUse = &cuctxToUse;
    m_pcolExtraTars = &colExtraTars;
    m_strTmplBasePath = strTmplBasePath;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_TmplActions
    );

    if (c4Res == kCQCIntfEd::ridDlg_TmplActions_Save)
    {
        // Copy the changes back
        colActions = m_colEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TTmplActionsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTmplActionsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_ActText, m_pwndActText);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_Add, m_pwndAdd);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_Del, m_pwndDel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_Edit, m_pwndEdit);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplActions_Save, m_pwndSave);

    // Register our handlers
    m_pwndAdd->pnothRegisterHandler(this, &TTmplActionsDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TTmplActionsDlg::eClickHandler);
    m_pwndDel->pnothRegisterHandler(this, &TTmplActionsDlg::eClickHandler);
    m_pwndEdit->pnothRegisterHandler(this, &TTmplActionsDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TTmplActionsDlg::eClickHandler);

    m_pwndList->pnothRegisterHandler(this, &TTmplActionsDlg::eListHandler);

    // Set the columns titles, of which we just have one
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCIntfEd().strMsg(kIEdMsgs::midDlg_TmplActions_List));
    m_pwndList->SetColumns(colCols, 0);

    // Set us as the auto-IPE handler for the list box
    m_pwndList->SetAutoIPE(this);

    //
    //  Load the list of actions. We use an assigned id to map between the list and the
    //  vector across editing and sorting.
    //
    const tCIDLib::TCard4 c4Count = m_colEdit.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        // Do an initial user id assignment to these as we load them
        TCQCStdCmdSrc& csrcCur = m_colEdit[c4Index];
        csrcCur.c4UserId(m_c4NextId++);

        colCols[0] = csrcCur.strTitle();
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, csrcCur.c4UserId());
    }

    // Select the first one if any
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TTmplActionsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

//
//  Give the id assigned to a row in the action list, find the action that has that same
//  id and return its index in the vector we are editing.
//
tCIDLib::TCard4 TTmplActionsDlg::c4ListToColInd(const tCIDLib::TCard4 c4ListInd) const
{
    // Get the row id for this list index row
    const tCIDLib::TCard4 c4RowId = m_pwndList->c4IndexToId(c4ListInd);

    const tCIDLib::TCard4 c4Count = m_colEdit.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (m_colEdit[c4Index].c4UserId() == c4RowId)
            return c4Index;
    }

    CIDAssert2(L"The row id was not found in the action vector")
    return kCIDLib::c4MaxCard;
}


// Handle button clicks
tCIDCtrls::EEvResponses
TTmplActionsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_Add)
    {
        // Let them edit a new guy
        TCQCStdCmdSrc csrcNew(L"New action");
        csrcNew.AddEvent
        (
            facCQCKit().strMsg(kKitMsgs::midCmdEv_OnTrigger)
            , kCQCKit::strEvId_OnTrigger
            , tCQCKit::EActCmdCtx::Standard
        );

        const tCIDLib::TBoolean bSave = facCQCIGKit().bEditAction
        (
            *this
            , csrcNew
            , kCIDLib::True
            , *m_pcuctxToUse
            , m_pcolExtraTars
            , &m_strTmplBasePath
        );

        // If they saved, add it to the list
        if (bSave)
        {
            // Give it the next available id
            csrcNew.c4UserId(m_c4NextId++);

            // Add it to the vector first so it'll be there when the new one is selected
            m_colEdit.objAdd(csrcNew);

            // Add it and select it. This will update the action text window
            tCIDLib::TStrList colCols(1);
            colCols.objAdd(csrcNew.strTitle());
            const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem
            (
                colCols, csrcNew.c4UserId(), kCIDLib::c4MaxCard, kCIDLib::True
            );
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_Cancel)
    {
        // See if there are any changes, if so, ask if we should discard them
        if (m_colEdit != *m_pcolOrgActs)
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_TmplActions_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_TmplActions_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_Del)
    {
        const tCIDLib::TCard4 c4LIndex = m_pwndList->c4CurItem();
        if (c4LIndex != kCIDLib::c4MaxCard)
        {
            // We have a selection so get its row id and find the associated action index
            const tCIDLib::TCard4 c4ActIndex = c4ListToColInd(c4LIndex);
            TCQCStdCmdSrc& csrcEdit = m_colEdit[c4ActIndex];

            // Make sure they want to do it
            TYesNoBox msgbDel(facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteCurAction));
            if (msgbDel.bShowIt(*this))
            {
                //
                //  Delete the item from the collection and then the list. Tell the list
                //  to select another if available.
                //
                m_colEdit.RemoveAt(c4ActIndex);
                m_pwndList->RemoveAt(c4LIndex, kCIDLib::True);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_Edit)
    {
        // Invoke the action editor for the selected Action
        const tCIDLib::TCard4 c4LIndex = m_pwndList->c4CurItem();
        if (c4LIndex != kCIDLib::c4MaxCard)
        {
            // We have a selection so get its row id and find the associated action index
            const tCIDLib::TCard4 c4ActIndex = c4ListToColInd(c4LIndex);
            TCQCStdCmdSrc& csrcEdit = m_colEdit[c4ActIndex];

            const tCIDLib::TBoolean bSave = facCQCIGKit().bEditAction
            (
                *this
                , csrcEdit
                , kCIDLib::True
                , *m_pcuctxToUse
                , m_pcolExtraTars
                , &m_strTmplBasePath
            );

            // If they saved, update the display text
            if (bSave)
            {
                TTextStringOutStream strmFmt(8192UL);
                csrcEdit.FormatOpsForEvent(kCQCKit::strEvId_OnTrigger, strmFmt);
                strmFmt.Flush();
                m_pwndActText->strWndText(strmFmt.strData());
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_Save)
    {
        EndDlg(kCQCIntfEd::ridDlg_TmplActions_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handle action list events
tCIDCtrls::EEvResponses
TTmplActionsDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    // When a new image is selected, update the image info
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        m_pwndActText->ClearText();
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplActions_List)
        {
            // Show the info on the newly selected action
            const tCIDLib::TCard4 c4ActIndex = c4ListToColInd(wnotEvent.c4Index());
            const TCQCStdCmdSrc& csrcShow = m_colEdit[c4ActIndex];

            TTextStringOutStream strmFmt(8192UL);
            csrcShow.FormatOpsForEvent(kCQCKit::strEvId_OnTrigger, strmFmt);
            strmFmt.Flush();
            m_pwndActText->strWndText(strmFmt.strData());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


