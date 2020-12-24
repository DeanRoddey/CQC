//
// FILE NAME: CQCIntfDes_LiveTileTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 02/19/2014
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
//  This file implements the configuration tab for the live tile widget type.
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
#include    "CQCIntfEd_LiveTileDlg_.hpp"


// ---------------------------------------------------------------------------
//  Magic Macros
// ---------------------------------------------------------------------------
RTTIDecls(TLiveTileDlg, TDlgBox)



// ---------------------------------------------------------------------------
//  CLASS: TLiveTileDlg
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TLiveTileDlg: Constructors and destructor
// ---------------------------------------------------------------------------
TLiveTileDlg::TLiveTileDlg() :

    m_pcolOrg(nullptr)
    , m_pwndAddTmpl(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDelTmpl(nullptr)
    , m_pwndFld1Comp(nullptr)
    , m_pwndFld2Comp(nullptr)
    , m_pwndFld1Val(nullptr)
    , m_pwndFld2Val(nullptr)
    , m_pwndLogOp(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndTmpls(nullptr)
    , m_pwndTmplDn(nullptr)
    , m_pwndTmplUp(nullptr)
    , m_pwndVarName(nullptr)
    , m_pwndVarVal(nullptr)
{
}

TLiveTileDlg::~TLiveTileDlg()
{
}


// ---------------------------------------------------------------------------
//  TLiveTileDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TLiveTileDlg::bRun(         TWindow&        wndOwner
                    ,       TTileInfo&      colEdit
                    , const TString&        strFld1
                    , const TString&        strFld2
                    , const TCQCUserCtx&    cuctxToUse
                    , const TString&        strParentTmplPath)
{
    // Save stuff for later
    m_pcuctxToUse = &cuctxToUse;
    m_pcolOrg = &colEdit;
    m_colEdit = colEdit;
    m_strFld1 = strFld1;
    m_strFld2 = strFld2;
    m_strParPath = strParentTmplPath;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_LTile
    );

    return (c4Res == kCQCIntfEd::ridDlg_LTile_Save);
}



// ---------------------------------------------------------------------------
//  TLiveTileDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TLiveTileDlg::bCreated()
{
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    //
    //  Get the list of available images from the interface repository and
    //  load up the list box.
    //
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_AddTmpl, m_pwndAddTmpl);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_DelTmpl, m_pwndDelTmpl);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Fld1Comp, m_pwndFld1Comp);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Fld2Comp, m_pwndFld2Comp);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Fld1Val, m_pwndFld1Val);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Fld2Val, m_pwndFld2Val);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_GVarName, m_pwndVarName);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_GVarVal, m_pwndVarVal);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_LogOp, m_pwndLogOp);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Save, m_pwndSave);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_Tmpls, m_pwndTmpls);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_TmplDn, m_pwndTmplDn);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_LTile_TmplUp, m_pwndTmplUp);

    // Register our handlers
    m_pwndAddTmpl->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);
    m_pwndDelTmpl->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);
    m_pwndTmplDn->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);
    m_pwndTmplUp->pnothRegisterHandler(this, &TLiveTileDlg::eClickHandler);

    m_pwndFld1Comp->pnothRegisterHandler(this, &TLiveTileDlg::eComboHandler);
    m_pwndFld2Comp->pnothRegisterHandler(this, &TLiveTileDlg::eComboHandler);
    m_pwndLogOp->pnothRegisterHandler(this, &TLiveTileDlg::eComboHandler);

    m_pwndFld1Val->pnothRegisterHandler(this, &TLiveTileDlg::eEFHandler);
    m_pwndFld2Val->pnothRegisterHandler(this, &TLiveTileDlg::eEFHandler);
    m_pwndVarName->pnothRegisterHandler(this, &TLiveTileDlg::eEFHandler);
    m_pwndVarVal->pnothRegisterHandler(this, &TLiveTileDlg::eEFHandler);

    m_pwndTmpls->pnothRegisterHandler(this, &TLiveTileDlg::eLBHandler);

    //
    //  Load up the available statement types to the two comparison expression combos
    //  and select initially the none entry.
    //
    XlatEnumToCombo(tCQCKit, EStatements, *m_pwndFld1Comp);
    XlatEnumToCombo(tCQCKit, EStatements, *m_pwndFld2Comp);
    m_pwndFld1Comp->SelectByIndex(tCIDLib::c4EnumOrd(tCQCKit::EStatements::None));
    m_pwndFld2Comp->SelectByIndex(tCIDLib::c4EnumOrd(tCQCKit::EStatements::None));

    AltXlatEnumToCombo(tCQCKit, ELogOps, *m_pwndLogOp);
    m_pwndLogOp->SelectByIndex(0);

    // Initialize our widgets
    LoadWidgets();

    return bRet;
}



// ---------------------------------------------------------------------------
//  TLiveTileDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TLiveTileDlg::bValidate(TString& strReason)
{
    const tCIDLib::TCard4 c4TmplCnt = m_colEdit.c4ElemCount();

    if (!c4TmplCnt)
    {
        strReason.LoadFromMsg(kIEdErrs::errcLTile_NoTmpl, facCQCIntfEd());
        return kCIDLib::False;
    }

    const tCIDLib::TBoolean bGotFld1 = !m_strFld1.bIsEmpty();
    const tCIDLib::TBoolean bGotFld2 = !m_strFld2.bIsEmpty();

    // Go through the templates configured and check them out
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4TmplCnt; c4Index++)
    {
        TCQCIntfLiveTile::TTmplInfo& tiCur = m_colEdit[c4Index];

        // Try to parse the variable name if one is set
        if (!tiCur.m_strVarName.bIsEmpty())
        {
            TString strVarReason;
            if (!TStdVarsTar::bValidVarName(tiCur.m_strVarName
                                            , kCQCKit::strActVarPref_GVar
                                            , strVarReason))
            {
                strReason.LoadFromMsg
                (
                    kIEdErrs::errcLTile_VarName
                    , facCQCIntfEd()
                    , TCardinal(c4Index + 1)
                    , strVarReason
                );
                return kCIDLib::False;
            }
        }

        // Make sure that the enabled expressions have defined fields
        if ((tiCur.m_expr1.eStatement() != tCQCKit::EStatements::None) && !bGotFld1)
        {
            strReason.LoadFromMsg
            (
                kIEdErrs::errcLTile_NoFldForExpr
                , facCQCIntfEd()
                , TCardinal(1)
                , TCardinal(c4Index + 1)
            );
            return kCIDLib::False;
        }

        if ((tiCur.m_expr2.eStatement() != tCQCKit::EStatements::None) && !bGotFld2)
        {
            strReason.LoadFromMsg
            (
                kIEdErrs::errcLTile_NoFldForExpr
                , facCQCIntfEd()
                , TCardinal(2)
                , TCardinal(c4Index + 1)
            );
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TLiveTileDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_AddTmpl)
    {
        //
        //  Let them select a template. It can be relative, so pass in our parent
        //  template path as the relative to path.
        //
        TString strSelected;
        const tCIDLib::TBoolean bRes = facCQCTreeBrws().bSelectFile
        (
            *this
            , L"Select a Template"
            , tCQCRemBrws::EDTypes::Template
            , TString::strEmpty()
            , *m_pcuctxToUse
            , tCQCTreeBrws::EFSelFlags::SelectItems
            , strSelected
            , nullptr
            , &m_strParPath
        );

        if (bRes)
        {
            // Add a new template info object
            TCQCIntfLiveTile::TTmplInfo tiNew;

            //
            //  Add to our list and the window list. In both cases it'll go at the
            //  end.
            //
            tiNew.m_strTemplate = strSelected;
            m_colEdit.objAdd(tiNew);
            m_pwndTmpls->c4AddItem(strSelected, 0);

            // If this was the first one, select it
            if (m_pwndTmpls->c4ItemCount() == 1)
                m_pwndTmpls->SelectByIndex(0, kCIDLib::True);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Cancel)
    {
        EndDlg(kCQCIntfEd::ridDlg_LTile_Cancel);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_DelTmpl)
    {
        // See which field is selected and remove it
        tCIDLib::TCard4 c4Index = m_pwndTmpls->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            //
            //  Remove it from the widget first. Since removing the list item
            //  will cause a new selection which will access the list of templates.
            //  This insures it sees the right one.
            //
            m_colEdit.RemoveAt(c4Index);
            m_pwndTmpls->RemoveAt(c4Index);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Save)
    {
        TString strErr;
        if (bValidate(strErr))
        {
            // Gopy over the edited stuff and exit
            *m_pcolOrg = m_colEdit;
            EndDlg(kCQCIntfEd::ridDlg_LTile_Save);
        }
         else
        {
            TErrBox msgbErr(strErr);
            msgbErr.ShowIt(*this);
        }
    }
     else if ((wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_TmplDn)
          ||  (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_TmplUp))
    {
        const tCIDLib::TBoolean bUp
        (
            wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_TmplUp
        );

        //
        //  Keep in mind here that we will have already swapped the list box by the
        //  time we swap the widget's template order. So we get the selection index
        //  out first!
        //
        const tCIDLib::TCard4 c4SelIndex = m_pwndTmpls->c4CurItem();
        if (bUp)
        {
            if (m_pwndTmpls->bMoveUp(kCIDLib::True))
                m_colEdit.SwapItems(c4SelIndex, c4SelIndex - 1);
        }
         else
        {
            if (m_pwndTmpls->bMoveDn(kCIDLib::True))
                m_colEdit.SwapItems(c4SelIndex, c4SelIndex + 1);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TLiveTileDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    // Get the selected template. If none, do nothing
    tCIDLib::TCard4 c4Index = m_pwndTmpls->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return tCIDCtrls::EEvResponses::Handled;

    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // The indices of the values are the same as their related enums, so just cast
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Fld1Comp)
        {
            m_colEdit[c4Index].m_expr1.eStatement
            (
                tCQCKit::EStatements(wnotEvent.c4Index())
            );
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Fld2Comp)
        {
            m_colEdit[c4Index].m_expr2.eStatement
            (
                tCQCKit::EStatements(wnotEvent.c4Index())
            );
        }
         else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_LogOp)
        {
            m_colEdit[c4Index].m_eLogOp = tCQCKit::ELogOps(wnotEvent.c4Index());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Since they can change template selection any time, we have to update per-template
//  info on the fly as it changes. So we hook entry field changes and update the
//  text as it is edited. This way we don't have to worry about getting the info
//  saved if they select another one.
//
tCIDCtrls::EEvResponses TLiveTileDlg::eEFHandler(TEFChangeInfo& wnotEvent)
{
    tCIDLib::TCard4 c4Index = m_pwndTmpls->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return tCIDCtrls::EEvResponses::Handled;

    if (wnotEvent.eEvent() == tCIDCtrls::EEFEvents::Changed)
    {
        // Get the selected template, since we have to update that one
        TCQCIntfLiveTile::TTmplInfo& tiCur = m_colEdit[c4Index];

        // Update the wiget with the current text and force it to update
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Fld1Val)
            tiCur.m_expr1.strCompVal(m_pwndFld1Val->strWndText());
        else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Fld2Val)
            tiCur.m_expr2.strCompVal(m_pwndFld2Val->strWndText());
        else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_GVarName)
            tiCur.m_strVarName = m_pwndVarName->strWndText();
        else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_GVarVal)
            tiCur.m_strVarVal = m_pwndVarVal->strWndText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We have to respond to changes in the templates list box, to update the per-
//  template stuff for the new selection.
//
tCIDCtrls::EEvResponses TLiveTileDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_LTile_Tmpls)
    {
        // Update the per-template info
        if ((wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
        ||  (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared))
        {
            LoadTmplInfo();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Get the selected template and load up the per-template controls with the
//  configured information.
//
tCIDLib::TVoid TLiveTileDlg::LoadTmplInfo()
{
    tCIDLib::TCard4 c4Index = m_pwndTmpls->c4CurItem();

    // If none selected, then clean them all out and disable them
    if (c4Index == kCIDLib::c4MaxCard)
    {
        m_pwndFld1Comp->SelectByIndex(0);
        m_pwndFld2Comp->SelectByIndex(0);
        m_pwndFld1Val->ClearText();
        m_pwndFld2Val->ClearText();
        m_pwndLogOp->SelectByIndex(0);
        m_pwndVarName->ClearText();
        m_pwndVarVal->ClearText();

        m_pwndFld1Comp->SetEnable(kCIDLib::False);
        m_pwndFld2Comp->SetEnable(kCIDLib::False);
        m_pwndFld1Val->SetEnable(kCIDLib::False);
        m_pwndFld2Val->SetEnable(kCIDLib::False);
        m_pwndLogOp->SetEnable(kCIDLib::False);
        m_pwndVarName->SetEnable(kCIDLib::False);
        m_pwndVarVal->SetEnable(kCIDLib::False);
        return;
    }

    // Make sure the widgets are enabled
    m_pwndFld1Comp->SetEnable(kCIDLib::True);
    m_pwndFld2Comp->SetEnable(kCIDLib::True);
    m_pwndFld1Val->SetEnable(kCIDLib::True);
    m_pwndFld2Val->SetEnable(kCIDLib::True);
    m_pwndLogOp->SetEnable(kCIDLib::True);
    m_pwndVarName->SetEnable(kCIDLib::True);
    m_pwndVarVal->SetEnable(kCIDLib::True);

    // Get the template info and let's load it up
    const TCQCIntfLiveTile::TTmplInfo& tiCur = m_colEdit[c4Index];

    // These are just set to the index of their related enums
    m_pwndFld1Comp->SelectByIndex(tCIDLib::c4EnumOrd(tiCur.m_expr1.eStatement()));
    m_pwndFld2Comp->SelectByIndex(tCIDLib::c4EnumOrd(tiCur.m_expr2.eStatement()));
    m_pwndLogOp->SelectByIndex(tCIDLib::c4EnumOrd(tiCur.m_eLogOp));

    m_pwndFld1Val->strWndText(tiCur.m_expr1.strCompVal());
    m_pwndFld2Val->strWndText(tiCur.m_expr2.strCompVal());

    m_pwndVarName->strWndText(tiCur.m_strVarName);
    m_pwndVarVal->strWndText(tiCur.m_strVarVal);
}


//
//  Get the initial info set up. We don't do the current template oriented stuff.
//  We just let the list box handler deal with that when we fill in the templates
//  list box.
//
tCIDLib::TVoid TLiveTileDlg::LoadWidgets()
{
    // Load up any configured templates.
    m_pwndTmpls->RemoveAll();
    const tCIDLib::TCard4 c4Count = m_colEdit.c4ElemCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            m_pwndTmpls->c4AddItem(m_colEdit[c4Index].m_strTemplate);
        m_pwndTmpls->SelectByIndex(0, kCIDLib::True);
    }
}

