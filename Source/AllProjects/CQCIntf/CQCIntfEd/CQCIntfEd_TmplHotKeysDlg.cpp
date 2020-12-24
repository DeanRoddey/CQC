//
// FILE NAME: CQCAdmin_TmplHotKeysDlg.cpp
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
//  This file implements a dialog that let's the user edit the hot key driven actions
//  of a template.
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
#include    "CQCIntfEd_TmplHotKeysDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTmplHotKeysDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TTmplHotKeysDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTmplHotKeysDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TTmplHotKeysDlg::TTmplHotKeysDlg() :

    m_pcolExtraTars(nullptr)
    , m_pcolOrg(nullptr)
    , m_pwndActText(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TTmplHotKeysDlg::~TTmplHotKeysDlg()
{
}


// ---------------------------------------------------------------------------
//  TTmplHotKeysDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTmplHotKeysDlg::bRun(  const   TWindow&                    wndOwner
                        , const TString&                    strTmplBasePath
                        ,       tCQCIntfEng::THKActList&    colActions
                        ,       tCQCKit::TCmdTarList&       colExtraTars
                        , const TCQCUserCtx&                cuctxToUse)
{
    //
    //  Make a copy of the list for our editing purposes, and some other stuff. We also
    //  store a pointer to the original so we can check for changes if they cancel.
    //
    m_pcolOrg = &colActions;
    m_colEdit = colActions;
    m_pcuctxToUse = &cuctxToUse;
    m_pcolExtraTars = &colExtraTars;
    m_strTmplBasePath = strTmplBasePath;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_TmplHotKeys
    );

    if (c4Res == kCQCIntfEd::ridDlg_TmplHotKeys_Save)
    {
        // Copy the changes back
        colActions = m_colEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TTmplHotKeysDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTmplHotKeysDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_ActText, m_pwndActText);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_Delete, m_pwndDelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_Edit, m_pwndEditButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TmplHotKeys_Save, m_pwndSaveButton);


    // Register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TTmplHotKeysDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TTmplHotKeysDlg::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TTmplHotKeysDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TTmplHotKeysDlg::eClickHandler);

    m_pwndList->pnothRegisterHandler(this, &TTmplHotKeysDlg::eListHandler);

    // Set the titles on our list box, and make the 2nd column the search column
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(L" ");
    colCols.objAdd(facCQCIntfEd().strMsg(kIEdMsgs::midDlg_TmplHotKeys_List));
    m_pwndList->SetColumns(colCols, 1);

    // Load the list of hot keys
    tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys::Min;
    for (; eKey <= tCQCIntfEng::EHotKeys::Max; eKey++)
    {
        colCols[1] = tCQCIntfEng::strLoadEHotKeys(eKey);
        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colCols, tCIDLib::c4EnumOrd(eKey));
        m_pwndList->SetCheckAt(c4At, m_colEdit[eKey].bUsed());
    }

    // Select the first one
    m_pwndList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TTmplHotKeysDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TTmplHotKeysDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplHotKeys_Cancel)
    {
        if (m_colEdit != *m_pcolOrg)
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_TmplHotKeys_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_TmplHotKeys_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplHotKeys_Delete)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        const tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys(c4Index);

        if (tCQCIntfEng::bIsValidEnum(eKey)
        &&  m_colEdit[tCQCIntfEng::EHotKeys(c4Index)].bUsed())
        {
            TCQCStdCmdSrc& csrcEdit = m_colEdit[eKey];

            // Make sure they want to do it
            TYesNoBox msgbDel
            (
                facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteKey
                , tCQCIntfEng::strLoadEHotKeys(eKey))
            );

            if (msgbDel.bShowIt(*this))
            {
                // Disable the action, and update the list box
                m_colEdit[eKey].bUsed(kCIDLib::False);
                m_pwndList->SetCheckAt(c4Index, kCIDLib::False);

                // And clear the action text if any
                m_pwndActText->ClearText();
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplHotKeys_Edit)
    {
        // Invoke the action editor for the selected key
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        const tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys(c4Index);
        if (tCQCIntfEng::bIsValidEnum(eKey))
        {
            TCQCStdCmdSrc& csrcEdit = m_colEdit[eKey];
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

                // Also make sure this guy is enabled
                if (!csrcEdit.bUsed())
                {
                    csrcEdit.bUsed(kCIDLib::True);
                    m_pwndList->SetCheckAt(c4Index, kCIDLib::True);
                }
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplHotKeys_Save)
    {

        EndDlg(kCQCIntfEd::ridDlg_TmplHotKeys_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
TTmplHotKeysDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    // When a new image is selected, update the image info
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TmplHotKeys_List)
        {
            // Show the info on the newly selected action
            const tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys(wnotEvent.c4Index());
            CIDAssert(tCQCIntfEng::bIsValidEnum(eKey), L"Not a valid hot key enum");

            const TCQCStdCmdSrc& csrcShow = m_colEdit[eKey];
            if (csrcShow.bUsed())
            {
                TTextStringOutStream strmFmt(8192UL);
                csrcShow.FormatOpsForEvent(kCQCKit::strEvId_OnTrigger, strmFmt);
                strmFmt.Flush();
                m_pwndActText->strWndText(strmFmt.strData());
            }
             else
            {
                m_pwndActText->ClearText();
            }
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        // We toggle the enable state of the invoked action
        const tCQCIntfEng::EHotKeys eKey = tCQCIntfEng::EHotKeys(wnotEvent.c4Index());
        CIDAssert(tCQCIntfEng::bIsValidEnum(eKey), L"Not a valid hot key enum");

        TCQCStdCmdSrc& csrcShow = m_colEdit[eKey];
        csrcShow.bUsed(!csrcShow.bUsed());
        m_pwndList->SetCheckAt(wnotEvent.c4Index(), csrcShow.bUsed());
    }
    return tCIDCtrls::EEvResponses::Handled;
}


