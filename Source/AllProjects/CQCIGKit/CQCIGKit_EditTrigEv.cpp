//
// FILE NAME: CQCIGKit_TrigEvTab.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/27/2016
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
//  This file implements an editor for triggered events.
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


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditTrigEvWnd, TContentTab)
RTTIDecls(TEditTrigEvDlg,TDlgBox)



// ---------------------------------------------------------------------------
//  CLASS: TEditTrigEvWnd
// PREFIX: wnd
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditTrigEvWnd: Constructors and destructor
// ---------------------------------------------------------------------------

//
//  Indicate edit mode in our call to our parent class, since we only support edit
//  mode in this tab type.
//
TEditTrigEvWnd::TEditTrigEvWnd(const tCIDLib::TBoolean bDlgMode) :

    TWindow(kCIDLib::True)
    , m_bDlgMode(bDlgMode)
    , m_eTheme(tCIDCtrls::EWndThemes::DialogBox)
    , m_pcolExtraTars(nullptr)
    , m_pcsrcEdit(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndFilterInstr(nullptr)
    , m_pwndLogOp(nullptr)
    , m_pwndLogRuns(nullptr)
    , m_pwndSerialize(nullptr)
    , m_pwndTitle(nullptr)
{
    // Initialize all of the window pointers we are going to look up later
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
    {
        m_apwndCompPrefs[c4Index] = nullptr;
        m_apwndCompRXs[c4Index] = nullptr;
        m_apwndCompVals[c4Index] = nullptr;
        m_apwndFldPrefs[c4Index] = nullptr;
        m_apwndFldRXs[c4Index] = nullptr;
        m_apwndFldVals[c4Index] = nullptr;
        m_apwndNegates[c4Index] = nullptr;
        m_apwndTypes[c4Index] = nullptr;
    }
}

TEditTrigEvWnd::~TEditTrigEvWnd()
{
}


// ---------------------------------------------------------------------------
//  TEditTrigEvWnd: Public, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TVoid TEditTrigEvWnd::CommitChanges()
{
    m_csrcOrg = *m_pcsrcEdit;
}


tCIDLib::TVoid
TEditTrigEvWnd::CreateTEvEd(const   TWindow&                    wndParent
                            , const tCIDCtrls::TWndId           widTab
                            ,       TCQCTrgEvent&               csrcEdit
                            ,       tCQCKit::TCmdTarList* const pcolExtraTars
                            , const tCIDCtrls::EWndThemes       eTheme
                            , const TCQCUserCtx&                cuctxToUse)
{
    // Store away the data, and a second copy for comparisons later
    m_eTheme = eTheme;
    m_pcolExtraTars = pcolExtraTars;
    m_pcsrcEdit = &csrcEdit;
    m_csrcOrg = csrcEdit;
    m_cuctxToUse = cuctxToUse;

    // Get our content dialog description and size us to fit it
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_EdTrgEv, dlgdChildren);

    CreateWnd
    (
        wndParent.hwndSafe()
        , kCIDCtrls::pszCustClass
        , facCQCIGKit().strMsg(kIGKitMsgs::midTitle_TrigEvEditor).pszBuffer()
        , dlgdChildren.areaPos()
        , tCIDCtrls::EWndStyles::ClippingVisChild
        , tCIDCtrls::EExWndStyles::ControlParent
        , widTab
    );
}


//
//  The caller can tell us if this is being done really to just test for changes, in
//  which case we only return changes or no changes.
//
tCIDLib::ESaveRes TEditTrigEvWnd::eValidate(TString& strErrMsg)
{
    //
    //  Make sure that we have a description field, and that the action has at least
    //  one sub-action defined.
    //
    m_pcsrcEdit->strTitle(m_pwndTitle->strWndText());
    if (m_pcsrcEdit->strTitle().bIsEmpty())
    {
        strErrMsg = facCQCIGKit().strMsg(kIGKitErrs::errcEv_NoTitle);
        return tCIDLib::ESaveRes::Errors;
    }
     else if (!m_pcsrcEdit->bHasAnyOps())
    {
        strErrMsg = facCQCKit().strMsg(kKitErrs::errcCmd_NoActCommands);
        return tCIDLib::ESaveRes::Errors;
    }

    // Get the filter info out and store it and validate it
    TFundVector<tCQCKit::ETEvFilters> fcolTypes(kCQCKit::c4MaxTEvFilters);
    tCIDLib::TCard4 c4UsedCnt = 0;
    TRegEx regxTest;
    const TCQCTrgEvent::TFilterList& colFilts = m_pcsrcEdit->colFilters();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
    {
        const TCQCTEvFilter& cevfCur = colFilts[c4Index];

        // Get the currently set values for this one
        const tCQCKit::ETEvFilters eType = tCQCKit::ETEvFilters
        (
            m_apwndTypes[c4Index]->c4IndexToId(m_apwndTypes[c4Index]->c4CurItem())
        );

        // Remember how many are used and keep a list of them
        fcolTypes.c4AddElement(eType);
        if (eType != tCQCKit::ETEvFilters::Unused)
            c4UsedCnt++;

        // Query info about this filter type
        tCIDLib::TBoolean bFldUsed;
        tCIDLib::TBoolean bCompUsed;
        tCIDLib::TBoolean bFldReq;
        tCIDLib::TBoolean bCompReq;
        tCIDLib::TBoolean bDummy1;
        tCIDLib::TBoolean bDummy2;
        TCQCTEvFilter::QueryFilterTypeInfo
        (
            eType, bFldUsed, bCompUsed, bFldReq, bCompReq, bDummy1, bDummy2
        );

        tCIDLib::TBoolean bDoCompRegX = m_apwndCompRXs[c4Index]->bCheckedState();
        tCIDLib::TBoolean bDoFldRegX = m_apwndFldRXs[c4Index]->bCheckedState();
        tCIDLib::TBoolean bNegate = m_apwndNegates[c4Index]->bCheckedState();
        const TString& strCompVal = m_apwndCompVals[c4Index]->strWndText();
        const TString& strFld = m_apwndFldVals[c4Index]->strWndText();

        //
        //  If used and required, make sure that they are not empty. If
        //  neither are used or required, then the default success value
        //  will remain.
        //
        tCIDLib::TBoolean bRes = kCIDLib::True;
        if (bFldUsed && bFldReq)
            bRes = !m_apwndFldVals[c4Index]->bIsEmpty();

        if (bRes && (bCompUsed && bCompReq))
            bRes = !m_apwndCompVals[c4Index]->bIsEmpty();

        if (!bRes)
        {
            // Oh well, something required is missing
            strErrMsg = facCQCIGKit().strMsg
            (
                kIGKitErrs::errcEv_MissingTrEvInfo, TCardinal(c4Index + 1)
            );
            return tCIDLib::ESaveRes::Errors;
        }

        //
        //  If either of the fields is enabled and is marked as a regular
        //  expression, parse it and make sure it works.
        //
        if (bDoCompRegX && m_apwndCompVals[c4Index]->bIsEnabled())
        {
            try
            {
                regxTest.SetExpression(m_apwndCompVals[c4Index]->strWndText());
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                strErrMsg = facCQCIGKit().strMsg
                (
                    kIGKitErrs::errcEv_BadTrgEvRegEx
                    , TString(L"comp value")
                    , TCardinal(c4Index + 1)
                );
                return tCIDLib::ESaveRes::Errors;
            }
        }

        if (bDoFldRegX && m_apwndFldVals[c4Index]->bIsEnabled())
        {
            try
            {
                regxTest.SetExpression(m_apwndFldVals[c4Index]->strWndText());
            }

            catch(TError& errToCatch)
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);

                strErrMsg = facCQCIGKit().strMsg
                (
                    kIGKitErrs::errcEv_BadTrgEvRegEx
                    , TString(L"field/device")
                    , TCardinal(c4Index + 1)
                );
                return tCIDLib::ESaveRes::Errors;
            }
        }

        m_pcsrcEdit->SetAt
        (
            c4Index, bNegate, eType, strFld, strCompVal, bDoCompRegX, bDoFldRegX
        );
    }

    //
    //  If no filtesr are enabled, then consider that an error since it would never run
    //
    //  If there's only one, scan and see if it's a 'field value equals' type. You just
    //  don't ever want to do that.
    //
    if (c4UsedCnt == 0)
    {
        strErrMsg = facCQCIGKit().strMsg(kIGKitErrs::errcEv_NoFilters);
        return tCIDLib::ESaveRes::Errors;
    }
     else if (c4UsedCnt == 1)
    {
        if (fcolTypes.bElementPresent(tCQCKit::ETEvFilters::FldValEquals))
        {
            strErrMsg = facCQCIGKit().strMsg(kIGKitErrs::errcEv_BadFilterChoice);
            return tCIDLib::ESaveRes::Errors;
        }
    }

    // Store the logical op
    m_pcsrcEdit->eFilterLogOp(tCQCKit::ETEvFiltLOps(m_pwndLogOp->c4CurItem()));

    // No errors, so return changes or not
    if (*m_pcsrcEdit == m_csrcOrg)
        return tCIDLib::ESaveRes::NoChanges;

    return tCIDLib::ESaveRes::OK;
}


// ---------------------------------------------------------------------------
//  TEditTrigEvWnd: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditTrigEvWnd::bCreated()
{
    TParent::bCreated();

    // Get the dialog description we'll use to create the controls
    TDlgDesc dlgdChildren;
    facCQCIGKit().bCreateDlgDesc(kCQCIGKit::ridDlg_EdTrgEv, dlgdChildren);

    // Force the caller's theme onto it before we create the child controls
    dlgdChildren.eTheme(m_eTheme);
    tCIDLib::TCard4 c4InitFocus;
    CreateDlgItems(dlgdChildren, c4InitFocus);

    // Get pointers to our controls we need to interact with
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1CompPref, m_apwndCompPrefs[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2CompPref, m_apwndCompPrefs[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3CompPref, m_apwndCompPrefs[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4CompPref, m_apwndCompPrefs[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1CompVal, m_apwndCompVals[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2CompVal, m_apwndCompVals[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3CompVal, m_apwndCompVals[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4CompVal, m_apwndCompVals[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1CompRegX, m_apwndCompRXs[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2CompRegX, m_apwndCompRXs[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3CompRegX, m_apwndCompRXs[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4CompRegX, m_apwndCompRXs[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1FldPref, m_apwndFldPrefs[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2FldPref, m_apwndFldPrefs[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3FldPref, m_apwndFldPrefs[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4FldPref, m_apwndFldPrefs[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1FldRegX, m_apwndFldRXs[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2FldRegX, m_apwndFldRXs[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3FldRegX, m_apwndFldRXs[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4FldRegX, m_apwndFldRXs[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1EvFld, m_apwndFldVals[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2EvFld, m_apwndFldVals[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3EvFld, m_apwndFldVals[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4EvFld, m_apwndFldVals[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1Type, m_apwndTypes[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2Type, m_apwndTypes[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3Type, m_apwndTypes[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4Type, m_apwndTypes[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_LogOp, m_pwndLogOp);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_1Negate, m_apwndNegates[0]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_2Negate, m_apwndNegates[1]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_3Negate, m_apwndNegates[2]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_4Negate, m_apwndNegates[3]);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_LogRuns, m_pwndLogRuns);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_EditAct, m_pwndEditButton);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_FilterInstr, m_pwndFilterInstr);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_Serialize, m_pwndSerialize);
    CastChildWnd(*this, kCQCIGKit::ridDlg_EdTrgEv_Title, m_pwndTitle);

    // If in tab mode, delete the save/cancel buttons
    if (!m_bDlgMode)
    {
        TWindow* pwndDel = pwndChildById(kCQCIGKit::ridDlg_EdTrgEvDlg_Save);
        pwndDel->Destroy();
        delete pwndDel;

        pwndDel = pwndChildById(kCQCIGKit::ridDlg_EdTrgEvDlg_Cancel);
        pwndDel->Destroy();
        delete pwndDel;
    }

    // Load the title text
    m_pwndTitle->strWndText(m_pcsrcEdit->strTitle());

    // Set the check boxes
    m_pwndLogRuns->SetCheckedState(m_pcsrcEdit->bLoggable());
    m_pwndSerialize->SetCheckedState(m_pcsrcEdit->bSerialized());

    //
    //  Load the filter type combos. This one is sorted so we just add it, not insert
    //  at a particular index. We set the filter type enum ordinal as the item id so
    //  that we can map back later.
    //
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
    {
        tCQCKit::ETEvFilters eType = tCQCKit::ETEvFilters::Min;
        while (eType < tCQCKit::ETEvFilters::Count)
        {
            m_apwndTypes[c4Index]->c4AddItem
            (
                tCQCKit::strXlatETEvFilters(eType), tCIDLib::c4EnumOrd(eType)
            );
            eType++;
        }
    }

    // Load up the incoming filter info
    const TCQCTrgEvent::TFilterList& colFilts = m_pcsrcEdit->colFilters();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
    {
        const TCQCTEvFilter& cevfCur = colFilts[c4Index];

        // Set the type first
        tCIDLib::TCard4 c4FoundAt;
        m_apwndTypes[c4Index]->bSelectByText
        (
            tCQCKit::strXlatETEvFilters(cevfCur.eType()), c4FoundAt
        );

        //
        //  Force the other filter fields to the correct enable/disable state
        //  by faking a call to NewFilterType.
        //
        NewFilterType(m_apwndTypes[c4Index]->widThis());

        // And now load the other stuff, based on what got enabled
        if (m_apwndNegates[c4Index]->bIsEnabled())
            m_apwndNegates[c4Index]->SetCheckedState(cevfCur.bNegate());

        if (m_apwndCompRXs[c4Index]->bIsEnabled())
            m_apwndCompRXs[c4Index]->SetCheckedState(cevfCur.bCompValRegEx());

        if (m_apwndFldRXs[c4Index]->bIsEnabled())
            m_apwndFldRXs[c4Index]->SetCheckedState(cevfCur.bFldRegEx());

        if (m_apwndCompVals[c4Index]->bIsEnabled())
            m_apwndCompVals[c4Index]->strWndText(cevfCur.strCompVal());

        if (m_apwndFldVals[c4Index]->bIsEnabled())
            m_apwndFldVals[c4Index]->strWndText(cevfCur.strEvFld());
    }

    // Fill in the logical operations combo
    tCQCKit::ETEvFiltLOps eLogOp = tCQCKit::ETEvFiltLOps::Min;
    while (eLogOp < tCQCKit::ETEvFiltLOps::Count)
    {
        m_pwndLogOp->InsertItem
        (
            tCIDLib::c4EnumOrd(eLogOp)
            , tCQCKit::strXlatETEvFiltLOps(eLogOp)
            , tCIDLib::c4EnumOrd(eLogOp)
        );
        eLogOp++;
    }

    // And select the incoming logical op
    m_pwndLogOp->SelectByIndex(tCIDLib::c4EnumOrd(m_pcsrcEdit->eFilterLogOp()));

    // Register our event handlers
    m_pwndLogRuns->pnothRegisterHandler(this, &TEditTrigEvWnd::eClickHandler);
    m_apwndTypes[0]->pnothRegisterHandler(this, &TEditTrigEvWnd::eLBHandler);
    m_apwndTypes[1]->pnothRegisterHandler(this, &TEditTrigEvWnd::eLBHandler);
    m_apwndTypes[2]->pnothRegisterHandler(this, &TEditTrigEvWnd::eLBHandler);
    m_apwndTypes[3]->pnothRegisterHandler(this, &TEditTrigEvWnd::eLBHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TEditTrigEvWnd::eClickHandler);
    m_pwndSerialize->pnothRegisterHandler(this, &TEditTrigEvWnd::eClickHandler);

    // Set our bgn color to match the theme
    SetBgnColor(TDlgBox::rgbDlgBgn(m_eTheme));

    // Indicate we want to see child focus changes
    bWantsChildFocusNot(kCIDLib::True);

    return kCIDLib::True;
}


//
//  We watch focus change events. When the focus moves to one of the filter drop downs,
//  we look at the filter selected and display some instruction text. If one of them
//  loses the focus, we remove the text.
//
tCIDLib::TVoid
TEditTrigEvWnd::ChildFocusChange(const  TWindow&            wndParent
                                , const tCIDCtrls::TWndId   widChild
                                , const tCIDLib::TBoolean   bGotFocus)
{
    // If the parent window was us, check it
    if (bIsThisWindow(wndParent))
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < kCQCKit::c4MaxTEvFilters; c4Index++)
        {
            if (widChild == m_apwndTypes[c4Index]->widThis())
            {
                if (bGotFocus)
                {
                    // Get the current filter and
                    const tCQCKit::ETEvFilters eType = tCQCKit::ETEvFilters
                    (
                        m_apwndTypes[c4Index]->c4IndexToId(m_apwndTypes[c4Index]->c4CurItem())
                    );
                    ShowFilterHelp(eType);
                }
                 else
                {
                    m_pwndFilterInstr->ClearText();
                }
                break;
            }
        }
    }
}


//
//  We intercept the popup context menus of the entry fields (by setting the 'parent
//  menu' style on them, so that we can provide our own menu.
//
tCIDCtrls::ECtxMenuOpts
TEditTrigEvWnd::eShowContextMenu(const TPoint& pntAt, const tCIDCtrls::TWndId widSrc)
{
    //
    //  Based on teh source id, we figureout which filter it's for and which of
    //  the two entry fields it's from.
    //
    tCIDLib::TBoolean   bCompVal = kCIDLib::False;
    tCIDLib::TCard4     c4TrigInd = kCIDLib::c4MaxCard;
    switch(widSrc)
    {
        case kCQCIGKit::ridDlg_EdTrgEv_1CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_1EvFld :
            c4TrigInd = 0;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_2CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_2EvFld :
            c4TrigInd = 1;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_3CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_3EvFld :
            c4TrigInd = 2;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_4CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_4EvFld :
            c4TrigInd = 3;
            break;

        default :
            break;
    };

    switch(widSrc)
    {
        case kCQCIGKit::ridDlg_EdTrgEv_1CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_2CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_3CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_4CompVal :
            bCompVal = kCIDLib::True;
            break;

        default :
            break;
    };

    // Shouldn't happen, but just in case
    if (c4TrigInd == kCIDLib::c4MaxCard)
        return tCIDCtrls::ECtxMenuOpts::Done;

    //
    //  If the trigger they hit is not active, then do nothing. Ths shouldn't
    //  happen since the entry field should be disabled, but just in case.
    //
    const tCQCKit::ETEvFilters eType = tCQCKit::ETEvFilters
    (
        m_apwndTypes[c4TrigInd]->c4IndexToId(m_apwndTypes[c4TrigInd]->c4CurItem())
    );

    if (eType == tCQCKit::ETEvFilters::Unused)
        return tCIDCtrls::ECtxMenuOpts::Done;

    // OK, let's pop up the insertion menu
    TPopupMenu menuIns(L"TrigEv Filter Insert");
    menuIns.Create(facCQCIGKit(), kCQCIGKit::ridMenu_TrgEvF);

    //
    //  Get the value in the field value entry field and remember if it parses to a
    //  validly formatted field name.
    //
    TString strDrv;
    TString strFld;
    const tCIDLib::TBoolean bValidFld = facCQCKit().bParseFldName
    (
        m_apwndFldVals[c4TrigInd]->strWndText(), strDrv, strFld
    );

    //
    //  If not on a comp value or the field value for this same trigger isn't a
    //  validly formatted field name, then disable the field value selection.
    //
    if (!bCompVal || !bValidFld)
        menuIns.SetItemEnable(kCQCIGKit::ridMenu_TrgEvF_FldVal, kCIDLib::False);

    tCIDLib::TCard4 c4MenuSel = menuIns.c4Process(*this, pntAt);

    if (c4MenuSel != kCIDLib::c4MaxCard)
    {
        // Get the target entry field
        TEntryField* pwndTar;
        if (bCompVal)
            pwndTar = m_apwndCompVals[c4TrigInd];
        else
            pwndTar = m_apwndFldVals[c4TrigInd];

        if (c4MenuSel == kCQCIGKit::ridMenu_TrgEvF_Field)
        {
            // WE pass in any previous value we parsed out above
            if (facCQCIGKit().bSelectField(*this, new TCQCFldFiltNull(), strDrv, strFld))
            {
                strDrv.Append(kCIDLib::chPeriod);
                strDrv.Append(strFld);
                pwndTar->strWndText(strDrv);
            }
        }
         else if (c4MenuSel == kCQCIGKit::ridMenu_TrgEvF_Dev)
        {
            // Pass in any existing value as the initial driver
            TString strDrv = pwndTar->strWndText();

            if (facCQCIGKit().bSelectDriver(*this, strDrv))
                pwndTar->strWndText(strDrv);
        }
         else if (c4MenuSel == kCQCIGKit::ridMenu_TrgEvF_Cls)
        {
            pwndTar->InsertText(TCQCEvent::strPath_Class);
        }
         else if (c4MenuSel == kCQCIGKit::ridMenu_TrgEvF_Src)
        {
            pwndTar->InsertText(TCQCEvent::strPath_Source);
        }
         else if (c4MenuSel == kCQCIGKit::ridMenu_TrgEvF_FldVal)
        {
            //
            //  See if we can find the currently entered field out there in driver
            //  land and query the field info if so.
            //
            TCQCFldDef flddTar;
            tCQCKit::TCQCSrvProxy orbcQS;
            try
            {
                orbcQS = facCQCKit().orbcCQCSrvAdminProxy(strDrv);
                orbcQS->QueryFieldDef(strDrv, strFld, flddTar);
            }

            catch(...)
            {
                TErrBox msgbErr(L"No information could be found on the currently entered field");
                msgbErr.ShowIt(*this);
                return tCIDCtrls::ECtxMenuOpts::Done;
            }

            //
            //  Ok, we can see if this field is of a type where we can present
            //  them a graphical way of selecting the value. Ask this guy to
            //  create a limits object for this limits settings.
            //
            TCQCFldLimit* pfldlOpt = TCQCFldLimit::pfldlMakeNew(flddTar);
            TJanitor<TCQCFldLimit> janLims(pfldlOpt);

            if (pfldlOpt->eOptimalRep() == tCQCKit::EOptFldReps::None)
            {
                TOkBox msgbOk(facCQCIGKit().strMsg(kIGKitMsgs::midStatus_CantSelectVal));
                msgbOk.ShowIt(*this);
                return tCIDCtrls::ECtxMenuOpts::Done;
            }

            // We pass in the current entry field text as the previous value
            TString strNewVal;
            tCIDLib::TBoolean bGotVal = facCQCGKit().bQueryFldValue
            (
                *this, flddTar, strDrv, pntAt, strNewVal, pwndTar->strWndText()
            );

            if (bGotVal)
                pwndTar->strWndText(strNewVal);
        }
    }

    return tCIDCtrls::ECtxMenuOpts::Done;
}



// ---------------------------------------------------------------------------
//  TEditTrigEvWnd: Private, non-virtaul methods
// ---------------------------------------------------------------------------

// Given one of the filter windows, figures out the filter index for it
tCIDLib::TCard4
TEditTrigEvWnd::c4FindFilterIndex(const tCIDCtrls::TWndId widChild) const
{
    tCIDLib::TCard4 c4Ret = kCIDLib::c4MaxCard;
    switch(widChild)
    {
        case kCQCIGKit::ridDlg_EdTrgEv_1CompRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_1CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_1EvFld :
        case kCQCIGKit::ridDlg_EdTrgEv_1FldRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_1Negate :
        case kCQCIGKit::ridDlg_EdTrgEv_1Type :
            c4Ret = 0;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_2CompRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_2CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_2EvFld :
        case kCQCIGKit::ridDlg_EdTrgEv_2FldRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_2Negate :
        case kCQCIGKit::ridDlg_EdTrgEv_2Type :
            c4Ret = 1;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_3CompRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_3CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_3EvFld :
        case kCQCIGKit::ridDlg_EdTrgEv_3FldRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_3Negate :
        case kCQCIGKit::ridDlg_EdTrgEv_3Type :
            c4Ret = 2;
            break;

        case kCQCIGKit::ridDlg_EdTrgEv_4CompRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_4CompVal :
        case kCQCIGKit::ridDlg_EdTrgEv_4EvFld :
        case kCQCIGKit::ridDlg_EdTrgEv_4FldRegX :
        case kCQCIGKit::ridDlg_EdTrgEv_4Negate :
        case kCQCIGKit::ridDlg_EdTrgEv_4Type :
            c4Ret = 3;
            break;
    };
    return c4Ret;
}


tCIDCtrls::EEvResponses TEditTrigEvWnd::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTrgEv_LogRuns)
    {
        m_pcsrcEdit->bLoggable(m_pwndLogRuns->bCheckedState());
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTrgEv_EditAct)
    {
        //
        //  We need to set up the extra targets list. We got a list from the user, but we
        //  also need to get a triggered event target in the list as well, so we make a
        //  second list and add the caller's to it plus our temporary triggered event target.
        //  The caller's list can be null if he has nothing to add. That way he doesn't have
        //  to keep a dummy list around.
        //
        const tCIDLib::TCard4 c4ExtraTars = m_pcolExtraTars ? m_pcolExtraTars->c4ElemCount() : 0;
        tCQCKit::TCmdTarList colExtraTars(tCIDLib::EAdoptOpts::NoAdopt, c4ExtraTars + 1);
        if (m_pcolExtraTars)
        {
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4ExtraTars; c4Index++)
                colExtraTars.Add(m_pcolExtraTars->pobjAt(c4Index));
        }

        TTrigEvCmdTarget ctarEvent;
        colExtraTars.Add(&ctarEvent);

        //
        //  We must do the version where we set up our own set list of targets,
        //  because we have a non-standard one, the event target. The caller provides
        //  an action context since these types of triggered events can be used from
        //  multiple programs.
        //
        facCQCIGKit().bEditAction
        (
            *this
            , *m_pcsrcEdit
            , kCIDLib::True
            , m_cuctxToUse
            , &colExtraTars
        );
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTrgEv_Serialize)
    {
        m_pcsrcEdit->bSerialized(m_pwndSerialize->bCheckedState());
    }
    return tCIDCtrls::EEvResponses::Handled;
}



tCIDCtrls::EEvResponses TEditTrigEvWnd::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // A selection was changed, so watch for filter type changes
        switch(wnotEvent.widSource())
        {
            case kCQCIGKit::ridDlg_EdTrgEv_1Type :
            case kCQCIGKit::ridDlg_EdTrgEv_2Type :
            case kCQCIGKit::ridDlg_EdTrgEv_3Type :
            case kCQCIGKit::ridDlg_EdTrgEv_4Type :
            {
                //
                //  Based on the type, we will enable/disable the other
                //  fields of this trigger, and update some of the prompt
                //  text appropriately.
                //
                NewFilterType(wnotEvent.widSource());
                break;
            }

            default :
                break;
        };
    }

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  When the user selects a new filter type from one of the filter type
//  combo boxes, we'll get called here with the id of the combo.
//
tCIDLib::TVoid TEditTrigEvWnd::NewFilterType(const tCIDCtrls::TWndId widCombo)
{
    // Get the filter index that corresponds to this combo box
    tCIDLib::TCard4 c4Index = c4FindFilterIndex(widCombo);

    //
    //  Get the filter type. We allowed the list to sort, and we set the original
    //  filter enum ordinal as the id on each one, So get the selected item, and
    //  map that to the id.
    //
    const tCQCKit::ETEvFilters eType = tCQCKit::ETEvFilters
    (
        m_apwndTypes[c4Index]->c4IndexToId(m_apwndTypes[c4Index]->c4CurItem())
    );

    // Query info about this filter type
    tCIDLib::TBoolean bFldUsed;
    tCIDLib::TBoolean bCompUsed;
    tCIDLib::TBoolean bFldReq;
    tCIDLib::TBoolean bCompReq;
    tCIDLib::TBoolean bFldRegEx;
    tCIDLib::TBoolean bCompRegEx;
    TCQCTEvFilter::QueryFilterTypeInfo
    (
        eType, bFldUsed, bCompUsed, bFldReq, bCompReq, bFldRegEx, bCompRegEx
    );

    // Query the prompt text info for this one and load them up
    TString strFldPref;
    TString strCompPref;
    TCQCTEvFilter::EFldTypes eFldType;
    TCQCTEvFilter::QueryFilterValPrefs(eType, strFldPref, eFldType, strCompPref);
    m_apwndFldPrefs[c4Index]->strWndText(strFldPref);
    m_apwndCompPrefs[c4Index]->strWndText(strCompPref);

    // Enable and disable fields as needed
    m_apwndCompVals[c4Index]->ClearText();
    m_apwndFldVals[c4Index]->ClearText();

    m_apwndFldPrefs[c4Index]->SetEnable(bFldUsed);
    m_apwndFldVals[c4Index]->SetEnable(bFldUsed);

    m_apwndCompPrefs[c4Index]->SetEnable(bCompUsed);
    m_apwndCompVals[c4Index]->SetEnable(bCompUsed);

    // Set the the prefix colors blue for optional and black for required
    if (!bFldUsed || bFldReq)
        m_apwndFldPrefs[c4Index]->SetTextColor(facCIDGraphDev().rgbBlack);
    else
        m_apwndFldPrefs[c4Index]->SetTextColor(facCIDGraphDev().rgbBlue);

    if (!bCompUsed || bCompReq)
        m_apwndCompPrefs[c4Index]->SetTextColor(facCIDGraphDev().rgbBlack);
    else
        m_apwndCompPrefs[c4Index]->SetTextColor(facCIDGraphDev().rgbBlue);


    //
    //  There's a special case for the negate thing. Disable it if they
    //  select unused, else enable it.
    //
    m_apwndNegates[c4Index]->SetCheckedState(kCIDLib::False);
    m_apwndNegates[c4Index]->SetEnable(eType != tCQCKit::ETEvFilters::Unused);

    // Do the regular expression stuff
    m_apwndCompRXs[c4Index]->SetCheckedState(kCIDLib::False);
    m_apwndCompRXs[c4Index]->SetEnable(bCompRegEx);

    m_apwndFldRXs[c4Index]->SetCheckedState(kCIDLib::False);
    m_apwndFldRXs[c4Index]->SetEnable(bFldRegEx);

    // Show the instruction text for this new fitler type
    ShowFilterHelp(eType);
}


//
//  When the focus goes to one of the filter type combos, or the user is already one and
//  changes the type, we show some instruction text for that type.
//
tCIDLib::TVoid TEditTrigEvWnd::ShowFilterHelp(const tCQCKit::ETEvFilters eType)
{
    struct TInstructMap
    {
        tCQCKit::ETEvFilters    eFiltType;
        tCIDLib::TMsgId         midText;
        TString                 strText;
    };

    static TInstructMap aMap[tCIDLib::c4EnumOrd(tCQCKit::ETEvFilters::Count)]
    {
          { tCQCKit::ETEvFilters::Unused               , 0                                         , L"" }
        , { tCQCKit::ETEvFilters::Arbitrary            , kIGKitMsgs::midDlg_FType_EventFldEquals   , L"" }
        , { tCQCKit::ETEvFilters::FldValEquals         , kIGKitMsgs::midDlg_FType_FldValEquals     , L"" }
        , { tCQCKit::ETEvFilters::IsDevReady           , kIGKitMsgs::midDlg_FType_IsDevReady       , L"" }
        , { tCQCKit::ETEvFilters::IsFieldChange        , kIGKitMsgs::midDlg_FType_IsFldChange      , L"" }
        , { tCQCKit::ETEvFilters::IsFieldChangeFor     , kIGKitMsgs::midDlg_FType_IsFldChangeFor   , L"" }
        , { tCQCKit::ETEvFilters::IsFromSource         , kIGKitMsgs::midDlg_FType_IsFromSource     , L"" }
        , { tCQCKit::ETEvFilters::IsNewFldValFor       , kIGKitMsgs::midDlg_FType_IsNewFldValFor   , L"" }
        , { tCQCKit::ETEvFilters::IsInTimeRange        , kIGKitMsgs::midDlg_FType_IsInTimeRange    , L"" }
        , { tCQCKit::ETEvFilters::IsNight              , kIGKitMsgs::midDlg_FType_IsNight          , L"" }
        , { tCQCKit::ETEvFilters::IsOfClass            , kIGKitMsgs::midDlg_FType_IsOfClass        , L"" }
        , { tCQCKit::ETEvFilters::IsUserAction         , kIGKitMsgs::midDlg_FType_IsUserAction     , L"" }
        , { tCQCKit::ETEvFilters::IsUserActionFor      , kIGKitMsgs::midDlg_FType_IsUserActFor     , L"" }
        , { tCQCKit::ETEvFilters::IsUserActionFrom     , kIGKitMsgs::midDlg_FType_IsUserActFrom    , L"" }
        , { tCQCKit::ETEvFilters::IsThisUserAction     , kIGKitMsgs::midDlg_FType_IsThisUserAct    , L"" }
        , { tCQCKit::ETEvFilters::IsLoadChange         , kIGKitMsgs::midDlg_FType_IsLoadChange     , L"" }
        , { tCQCKit::ETEvFilters::IsLoadChangeFor      , kIGKitMsgs::midDlg_FType_IsLoadChFor      , L"" }
        , { tCQCKit::ETEvFilters::IsLoadChangeFrom     , kIGKitMsgs::midDlg_FType_IsLoadChFrom     , L"" }
        , { tCQCKit::ETEvFilters::IsLoadChangeOn       , kIGKitMsgs::midDlg_FType_IsLoadChOn       , L"" }
        , { tCQCKit::ETEvFilters::IsLoadChangeOff      , kIGKitMsgs::midDlg_FType_IsLoadChOff      , L"" }
        , { tCQCKit::ETEvFilters::IsMotionEv           , kIGKitMsgs::midDlg_FType_IsMotionEv       , L"" }
        , { tCQCKit::ETEvFilters::IsMotionEvFor        , kIGKitMsgs::midDlg_FType_IsMotEvFor       , L"" }
        , { tCQCKit::ETEvFilters::IsMotionEvFrom       , kIGKitMsgs::midDlg_FType_IsMotEvFrom      , L"" }
        , { tCQCKit::ETEvFilters::IsMotionStartEv      , kIGKitMsgs::midDlg_FType_IsMotStartEv     , L"" }
        , { tCQCKit::ETEvFilters::IsMotionEndEv        , kIGKitMsgs::midDlg_FType_IsMotEndEv       , L"" }
        , { tCQCKit::ETEvFilters::IsZoneAlarm          , kIGKitMsgs::midDlg_FType_IsZoneAlarm      , L"" }
        , { tCQCKit::ETEvFilters::IsZoneAlarmFor       , kIGKitMsgs::midDlg_FType_IsZoneAlarmFor   , L"" }
        , { tCQCKit::ETEvFilters::IsZoneAlarmFrom      , kIGKitMsgs::midDlg_FType_IsZoneAlarmFrom  , L"" }
        , { tCQCKit::ETEvFilters::IsZoneViolated       , kIGKitMsgs::midDlg_FType_IsZoneViolated   , L"" }
        , { tCQCKit::ETEvFilters::IsZoneSecured        , kIGKitMsgs::midDlg_FType_IsZoneSecured    , L"" }
        , { tCQCKit::ETEvFilters::IsWeekDay            , kIGKitMsgs::midDlg_FType_IsWeekDay        , L"" }
        , { tCQCKit::ETEvFilters::IsPresenceEv         , kIGKitMsgs::midDlg_FType_IsPresEv         , L"" }
        , { tCQCKit::ETEvFilters::IsPresenceEvFrom     , kIGKitMsgs::midDlg_FType_IsPresEvFrom     , L"" }
        , { tCQCKit::ETEvFilters::IsPresenceEvInArea   , kIGKitMsgs::midDlg_FType_IsPresEvInArea   , L"" }
        , { tCQCKit::ETEvFilters::IsLockStatus         , kIGKitMsgs::midDlg_FType_IsLockStatus     , L"" }
        , { tCQCKit::ETEvFilters::IsLockStatusFrom     , kIGKitMsgs::midDlg_FType_IsLockStatusFrom , L"" }
        , { tCQCKit::ETEvFilters::IsLockStatusCode     , kIGKitMsgs::midDlg_FType_IsLockStatusCode , L"" }
    };

    // If the type isn't unused, and the text is empty, we've not loaded this one yet
    if ((eType != tCQCKit::ETEvFilters::Unused)
    &&  (aMap[tCIDLib::c4EnumOrd(eType)].strText.bIsEmpty()))
    {
        aMap[tCIDLib::c4EnumOrd(eType)].strText
                    = facCQCIGKit().strMsg(aMap[tCIDLib::c4EnumOrd(eType)].midText);
    }

    m_pwndFilterInstr->strWndText(aMap[tCIDLib::c4EnumOrd(eType)].strText);
}




// ---------------------------------------------------------------------------
//   CLASS: TEditTrigEvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditTrigEvDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditTrigEvDlg::TEditTrigEvDlg() :

    m_pcolExtraTars(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndEditor(nullptr)
    , m_pwndSave(nullptr)
{
}

TEditTrigEvDlg::~TEditTrigEvDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditTrigEvDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEditTrigEvDlg::bRunDlg(const   TWindow&                    wndOwner
                        ,       TCQCTrgEvent&               csrcEdit
                        ,       tCQCKit::TCmdTarList* const pcolExtraTars
                        , const TCQCUserCtx&                cuctxToUse)
{
    m_csrcEdit = csrcEdit;
    m_cuctxToUse = cuctxToUse;
    m_pcolExtraTars = pcolExtraTars;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_EdTrgEvDlg
    );

    if (c4Res == kCQCIGKit::ridDlg_EdTrgEvDlg_Save)
    {
        csrcEdit = m_csrcEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEditTrigEvDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditTrigEvDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    //
    //  Create the editor window, telling him he is in dialog mode. We pass along any
    //  extra targets the caller might have provided, which could be null if none.
    //
    m_pwndEditor = new TEditTrigEvWnd(kCIDLib::True);
    m_pwndEditor->CreateTEvEd
    (
        *this
        , kCIDCtrls::widFirstCtrl
        , m_csrcEdit
        , m_pcolExtraTars
        , tCIDCtrls::EWndThemes::DialogBox
        , m_cuctxToUse
    );

    // Now get typed pointers to the dialog specific widgets
    CastChildWnd(*m_pwndEditor, kCQCIGKit::ridDlg_EdTrgEvDlg_Cancel, m_pwndCancel);
    CastChildWnd(*m_pwndEditor, kCQCIGKit::ridDlg_EdTrgEvDlg_Save, m_pwndSave);

    // And register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEditTrigEvDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEditTrigEvDlg::eClickHandler);

    // Tell our parent to auto-size us now
    SetAutoSize(kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditTrigEvDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEditTrigEvDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTrgEvDlg_Save)
    {
        TString strErr;
        const tCIDLib::ESaveRes eRes = m_pwndEditor->eValidate(strErr);
        if (eRes == tCIDLib::ESaveRes::Errors)
        {
            TErrBox msgbErr(strErr);
            msgbErr.ShowIt(*this);
        }
         else
        {
            EndDlg(kCQCIGKit::ridDlg_EdTrgEvDlg_Save);
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_EdTrgEvDlg_Cancel)
    {
        EndDlg(kCQCIGKit::ridDlg_EdTrgEvDlg_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

