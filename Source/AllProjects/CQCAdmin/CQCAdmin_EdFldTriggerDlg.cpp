//
// FILE NAME: CQCAdmin_EdFldTriggerDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/20/2005
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
//  This file implements a dialog that lets the user configure a trigger on
//  a device field, which will cause that field to send out an event.
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
#include    "CQCAdmin.hpp"
#include    "CQCAdmin_EdFldTriggerDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdFldTriggerDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdFldTriggerDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdFldTriggerDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdFldTriggerDlg::TEdFldTriggerDlg() :

    m_pwndCancel(nullptr)
    , m_pwndCompVal(nullptr)
    , m_pwndDisable(nullptr)
    , m_pwndExprType(nullptr)
    , m_pwndLatching(nullptr)
    , m_pwndOnChange(nullptr)
    , m_pwndOnExpr(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSelCompVal(nullptr)
{
}

TEdFldTriggerDlg::~TEdFldTriggerDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdFldTriggerDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdFldTriggerDlg::bRunDlg(  const   TWindow&            wndOwner
                            , const TString&            strMoniker
                            , const TCQCFldDef&         flddInfo
                            ,       TCQCFldEvTrigger&   fetToEdit)
{
    // Save incoming info away for later use
    m_flddInfo = flddInfo;
    m_fetToEdit = fetToEdit;
    m_strMoniker = strMoniker;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCAdmin, kCQCAdmin::ridDlg_EdFldTr
    );

    if (c4Res == kCQCAdmin::ridDlg_EdFldTr_Save)
    {
        fetToEdit = m_fetToEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdFldTriggerDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdFldTriggerDlg::bCreated()
{
    // Call our parent first. In this case we ignore his return, see below
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some children
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_CompVal, m_pwndCompVal);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_Disable, m_pwndDisable);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_ExprType, m_pwndExprType);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_Latching, m_pwndLatching);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_OnChange, m_pwndOnChange);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_OnExpr, m_pwndOnExpr);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_Save, m_pwndSave);
    CastChildWnd(*this, kCQCAdmin::ridDlg_EdFldTr_SelCompVal, m_pwndSelCompVal);

    // Get the title text, replace the field name token, and put it back
    TString strTitle = strWndText();
    strTitle.eReplaceToken(m_fetToEdit.strFldName(), L'1');
    strWndText(strTitle);

    //
    //  Load the expression type combo box. We have an item for unused, and
    //  one for regular expression. Else, it is a statement so the rest are
    //  the available statement types.
    //
    m_pwndExprType->c4AddItem(L"Unused");
    m_pwndExprType->c4AddItem(L"Regular Expr");

    //
    //  Load up the statements after those, starting at the first used one. This
    //  skips the None entry. We already have Unused in the list.
    //
    tCIDLib::ForEachE<tCQCKit::EStatements, tCQCKit::EStatements::FirstUsed>
    (
        [this](const auto eStmt)
        { m_pwndExprType->c4AddItem(tCQCKit::strXlatEStatements(eStmt)); }
    );

    // Load up the latching options
    tCIDLib::ForEachE<tCQCKit::EEvTrLatches>
    (
        [this](const auto eLatch)
        { m_pwndLatching->c4AddItem(tCQCKit::strLoadEEvTrLatches(eLatch)); }
    );

    //
    //  Figure out which mode radio button to select and the initial selection in
    //  the combo boxes, if relevant.
    //
    tCIDLib::TCard4 c4SelInit = 0;
    if ((m_fetToEdit.eType() == tCQCKit::EEvTrTypes::Unused)
    ||  (m_fetToEdit.eType() == tCQCKit::EEvTrTypes::OnChange))
    {
        m_pwndCompVal->ClearText();

        m_pwndExprType->SetEnable(kCIDLib::False);
        m_pwndCompVal->SetEnable(kCIDLib::False);
        m_pwndLatching->SetEnable(kCIDLib::False);
        m_pwndSelCompVal->SetEnable(kCIDLib::False);

        m_pwndLatching->SelectByIndex(0);
        m_pwndLatching->SetEnable(kCIDLib::False);

        if (m_fetToEdit.eType() == tCQCKit::EEvTrTypes::Unused)
        {
            m_pwndDisable->SetCheckedState(kCIDLib::True);
            m_pwndDisable->TakeFocus();
        }
        else
        {
            m_pwndOnChange->TakeFocus();
            m_pwndOnChange->SetCheckedState(kCIDLib::True);
        }
    }
     else if (m_fetToEdit.eType() == tCQCKit::EEvTrTypes::OnExpression)
    {
        m_pwndExprType->SetEnable(kCIDLib::True);
        m_pwndLatching->SetEnable(kCIDLib::True);

        const tCIDLib::TBoolean bCompVal = m_fetToEdit.exprTrigger().bHasCompVal();
        m_pwndCompVal->SetEnable(bCompVal);
        m_pwndSelCompVal->SetEnable(bCompVal);

        if (bCompVal)
            m_pwndCompVal->strWndText(m_fetToEdit.exprTrigger().strCompVal());

        if (m_fetToEdit.exprTrigger().eType() == tCQCKit::EExprTypes::RegEx)
            c4SelInit = 1;
        else
            c4SelInit = tCIDLib::c4EnumOrd(m_fetToEdit.exprTrigger().eStatement()) + 1;

        m_pwndLatching->SelectByIndex(tCIDLib::c4EnumOrd(m_fetToEdit.eLatching()));

        m_pwndOnExpr->SetCheckedState(kCIDLib::True);
        m_pwndOnExpr->TakeFocus();
    }

    // Set the correct initial radio button
    m_pwndExprType->SelectByIndex(c4SelInit);

    // And register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);
    m_pwndDisable->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);
    m_pwndOnChange->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);
    m_pwndOnExpr->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);
    m_pwndExprType->pnothRegisterHandler(this, &TEdFldTriggerDlg::eComboHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);
    m_pwndSelCompVal->pnothRegisterHandler(this, &TEdFldTriggerDlg::eClickHandler);

    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TEdFldTriggerDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TEdFldTriggerDlg::bValidate()
{
    // Get the values out into the temp trigger object and validate them
    if (m_pwndDisable->bCheckedState())
    {
        m_fetToEdit.eType(tCQCKit::EEvTrTypes::Unused);
        m_fetToEdit.eLatching(tCQCKit::EEvTrLatches::Unlatched);
    }
     else if (m_pwndOnChange->bCheckedState())
    {
        m_fetToEdit.eType(tCQCKit::EEvTrTypes::OnChange);
        m_fetToEdit.eLatching(tCQCKit::EEvTrLatches::Unlatched);
    }
     else if (m_pwndOnExpr->bCheckedState())
    {
        m_fetToEdit.eType(tCQCKit::EEvTrTypes::OnExpression);
        m_fetToEdit.eLatching
        (
            tCQCKit::EEvTrLatches(m_pwndLatching->c4CurItem())
        );

        // Pull out the expression stuff
        tCQCKit::EExprTypes  eExprType;
        tCQCKit::EStatements eStatement;
        GetExprInfo(eExprType, eStatement);

        // It can't be none, since they selected expression
        if (eExprType == tCQCKit::EExprTypes::None)
        {
            TErrBox msgbErr
            (
                strWndText(), facCQCAdmin.strMsg(kCQCAErrs::errcEdFTrg_NoStatement)
            );
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }

        // Put the info into the trigger object and ask it to validate
        m_fetToEdit.exprTrigger().Set
        (
            eExprType, eStatement, m_pwndCompVal->strWndText(), kCIDLib::False
        );

        TString strErrText;
        if (!m_fetToEdit.exprTrigger().bValidate(strErrText, m_flddInfo.eType()))
        {
            TErrBox msgbErr(strWndText(), strErrText);
            msgbErr.ShowIt(*this);
            return kCIDLib::False;
        }
    }
     else
    {
        #if CID_DEBUG_ON
        #endif
        return kCIDLib::False;
    }

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEdFldTriggerDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    // We only have one event, which is to close it
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_Cancel)
    {
        EndDlg(kCQCAdmin::ridDlg_EdFldTr_Cancel);
    }
     else if ((wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_Disable)
          ||  (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_OnChange))
    {
        // Disable the expression stuff
        m_pwndLatching->SelectByIndex(0);
        m_pwndCompVal->ClearText();
        m_pwndExprType->SelectByIndex(0);

        m_pwndExprType->SetEnable(kCIDLib::False);
        m_pwndCompVal->SetEnable(kCIDLib::False);
        m_pwndLatching->SetEnable(kCIDLib::False);
        m_pwndSelCompVal->SetEnable(kCIDLib::False);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_OnExpr)
    {
        m_pwndExprType->SetEnable(kCIDLib::True);

        // This stuff we only enable if there is an expression type selected
        const tCIDLib::TBoolean bHaveExpr(m_pwndExprType->c4CurItem() != 0);
        m_pwndLatching->SetEnable(bHaveExpr);
        m_pwndCompVal->SetEnable(bHaveExpr);
        m_pwndSelCompVal->SetEnable(bHaveExpr);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_Save)
    {
        if (bValidate())
            EndDlg(kCQCAdmin::ridDlg_EdFldTr_Save);
    }
     else if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_SelCompVal)
    {
        //
        //  Convert our button coordinates to screen coordinates and pass
        //  that as the relative to point, so that it comes up relative to
        //  us.
        //
        TPoint pntAt;
        ToScreenCoordinates(m_pwndSelCompVal->pntOrg(), pntAt);

        TString strVal;
        const tCIDLib::TBoolean bRet = facCQCGKit().bQueryFldValue
        (
            *this
            , m_flddInfo
            , m_strMoniker
            , pntAt
            , strVal
            , m_pwndCompVal->strWndText()
        );

        if (bRet)
            m_pwndCompVal->strWndText(strVal);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdFldTriggerDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    // Only care about the expression type
    if (wnotEvent.widSource() == kCQCAdmin::ridDlg_EdFldTr_ExprType)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
            ExprTypeChanged();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  Based on the passed expression type, we need to enable/disable stuff.
//
tCIDLib::TVoid TEdFldTriggerDlg::ExprTypeChanged()
{
    // Pull out the expression stuff
    tCQCKit::EExprTypes  eExprType;
    tCQCKit::EStatements eStatement;
    GetExprInfo(eExprType, eStatement);

    // Figure out some useful stuff from that
    const tCIDLib::TBoolean bCompVal = TCQCExpression::bNeedsCompVal(eExprType, eStatement);
    const tCIDLib::TBoolean bHaveExpr(eExprType != tCQCKit::EExprTypes::None);

    // Enable/disable the comp val stuff as needed
    m_pwndCompVal->SetEnable(bCompVal);
    m_pwndSelCompVal->SetEnable(bCompVal);

    // If no expression, make sure we select Unlatched and enable/disable it as needed
    if (!bHaveExpr)
        m_pwndLatching->SelectByIndex(tCIDLib::c4EnumOrd(tCQCKit::EEvTrLatches::Unlatched));
    m_pwndLatching->SetEnable(bHaveExpr);
}


//
//  Get the selection in the expression type combo and convert to an expression
//  type and statement.
//
tCIDLib::TVoid
TEdFldTriggerDlg::GetExprInfo(  tCQCKit::EExprTypes&    eExprType
                                , tCQCKit::EStatements& eStatement) const
{
    tCIDLib::TCard4 c4SelInd = m_pwndExprType->c4CurItem();

    if (c4SelInd == 0)
    {
        eExprType = tCQCKit::EExprTypes::None;
        eStatement = tCQCKit::EStatements::None;
    }
     else if (c4SelInd == 1)
    {
        eExprType = tCQCKit::EExprTypes::RegEx;
        eStatement = tCQCKit::EStatements::None;
    }
     else
    {
        //
        //  Though there are two entries before the first statement in the combo
        //  box, only subtract 1 for statement, because we don't load the 'none'
        //  entry,
        //
        eExprType = tCQCKit::EExprTypes::Statement;
        eStatement = tCQCKit::EStatements(c4SelInd - 1);
    }
}
