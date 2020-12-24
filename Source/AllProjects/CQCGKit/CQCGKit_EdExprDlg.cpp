//
// FILE NAME: CQCGKit_EdExprlDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 03/01/2004
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
//  This file implements a couple dialogs that allow the user to edit a
//  CQC expression.
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
#include    "CQCGKit_.hpp"
#include    "CQCGKit_EdExprDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TCQCEdExprDlg,TDlgBox)
RTTIDecls(TCQCEdExprDlg2,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TCQCEdExprDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEdExprDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEdExprDlg::TCQCEdExprDlg() :

    m_bChangeDescr(kCIDLib::True)
    , m_pexprOrg(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndCompVal(nullptr)
    , m_pwndDescription(nullptr)
    , m_pwndDevices(nullptr)
    , m_pwndExprType(nullptr)
    , m_pwndFields(nullptr)
    , m_pwndNegate(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndSelValButton(nullptr)
{
}

TCQCEdExprDlg::~TCQCEdExprDlg()
{
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCEdExprDlg::bRunDlg( const   TWindow&            wndOwner
                        , const TString&            strTitle
                        ,       TString&            strDevice
                        ,       TString&            strField
                        ,       TCQCFldDef&         fldAssoc
                        ,       TCQCExpression&     exprEdit
                        , const tCIDLib::TBoolean   bChangeDescr)
{
    // Make a copy of the incoming expression for us to edit
    m_bChangeDescr = bChangeDescr;
    m_exprEdit = exprEdit;
    m_flddAssoc = fldAssoc;
    m_pexprOrg = &exprEdit;
    m_strDevice = strDevice;
    m_strField = strField;
    m_strTitle = strTitle;

    //
    //  The first time, let it auto center the dialog within the owner. On
    //  subsequent trips, we call the version that overrides the centering
    //  flag and pass in the caller's now updated point.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCGKit(), kCQCGKit::ridDlg_EdExpr
    );

    // If accepted, then copy back to the caller's buffer
    if (c4Res == kCQCGKit::ridDlg_EdExpr_Save)
    {
        // Copy the data back to the caller's parms
        exprEdit = m_exprEdit;
        fldAssoc = m_flddAssoc;
        strField = m_strField;
        strDevice = m_strDevice;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCEdExprDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_CompVal, m_pwndCompVal);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Description, m_pwndDescription);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Devices, m_pwndDevices);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_ExprType, m_pwndExprType);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Fields, m_pwndFields);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Negate, m_pwndNegate);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_Save, m_pwndSaveButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr_SelValue, m_pwndSelValButton);

    // Set the title bar text to what we were given
    strWndText(m_strTitle);

    //
    //  Load up the expression type combo box and the device/fields lists and
    //  select the current ones. Do this before we set the notification handlers.
    //
    LoadLists();

    // Set the negation flag
    m_pwndNegate->SetCheckedState(m_exprEdit.bNegated());

    // If we have any comp value, load that
    if (m_exprEdit.bHasCompVal())
        m_pwndCompVal->strWndText(m_exprEdit.strCompVal());

    // And the description. If it can't be edited, then disable it
    m_pwndDescription->strWndText(m_exprEdit.strDescription());
    if (!m_bChangeDescr)
        m_pwndDescription->SetEnable(kCIDLib::False);

    // And register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCEdExprDlg::eClickHandler);
    m_pwndDevices->pnothRegisterHandler(this, &TCQCEdExprDlg::eListHandler);
    m_pwndExprType->pnothRegisterHandler(this, &TCQCEdExprDlg::eListHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TCQCEdExprDlg::eClickHandler);
    m_pwndSelValButton->pnothRegisterHandler(this,&TCQCEdExprDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Validate and display an error msg if any failure
tCIDLib::TBoolean TCQCEdExprDlg::bValidate()
{
    TString strErrMsg;
    if (!bValidate(strErrMsg))
    {
        TErrBox msgbErr(facCQCGKit().strMsg(kGKitErrs::errcExpr_ExprError), strErrMsg);
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}


// Validate, store the data that is valid, and return an error msg if any issues
tCIDLib::TBoolean TCQCEdExprDlg::bValidate(TString& strErrMsg)
{
    // We have to have a description
    const TString strDescr(m_pwndDescription->strWndText());
    if (strDescr.bIsEmpty())
    {
        strErrMsg = facCQCGKit().strMsg(kGKitErrs::errcExpr_EmptyDescr);
        return kCIDLib::False;
    }

    //
    //  Look up the field type info. We have to have a selection in both
    //  lists.
    //
    tCIDLib::TCard4 c4DrvInd = m_pwndDevices->c4CurItem();
    tCIDLib::TCard4 c4FldInd = m_pwndFields->c4CurItem();

    if ((c4DrvInd == kCIDLib::c4MaxCard) || (c4FldInd == kCIDLib::c4MaxCard))
    {
        strErrMsg = facCQCGKit().strMsg(kGKitErrs::errcExpr_MustSelField);
        return kCIDLib::False;
    }

    m_pwndDevices->ValueAt(c4DrvInd, m_strDevice);
    m_pwndFields->ValueAt(c4FldInd, m_strField);
    if (!m_cfcData.bFldExists(m_strDevice, m_strField, m_flddAssoc))
    {
        strErrMsg = facCQCGKit().strMsg(kGKitErrs::errcExpr_FldNotFound);
        return kCIDLib::False;
    }

    //
    //  Validate the expression stuff. According to the expression type
    //  they selected, we validate it differently. Note that we have to
    //  play some tricks here to map the index back to the expression type
    //  enum.
    //
    tCQCKit::EExprTypes eType;
    tCQCKit::EStatements eStatement;
    TString strCompVal;
    const tCIDLib::TCard4 c4ExprInd = m_pwndExprType->c4CurItem();
    if (c4ExprInd == 0)
    {
        eType = tCQCKit::EExprTypes::RegEx;
        eStatement = tCQCKit::EStatements::None;
    }
     else
    {
        // Don't subtract 1 for the statement, we don't load the 'none' entry
        eType = tCQCKit::EExprTypes::Statement;
        eStatement = tCQCKit::EStatements(c4ExprInd);
    }

    // Get any comp value if needed
    if (TCQCExpression::bNeedsCompVal(eType, eStatement))
        strCompVal = m_pwndCompVal->strWndText();

    // Get the negation flag
    const tCIDLib::TBoolean bNegate(m_pwndNegate->bCheckedState());

    // Ok, we have all the info so set up the expression
    m_exprEdit.Set(strDescr, eType, eStatement, strCompVal, bNegate);

    // And now validate the expression
    if (!m_exprEdit.bValidate(strErrMsg, m_flddAssoc.eType()))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TCQCEdExprDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr_Cancel)
    {
        TString strErr;
        if (!bValidate(strErr) || (m_exprEdit != *m_pexprOrg))
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCGKit().strMsg(kGKitMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCGKit::ridDlg_EdExpr_Cancel);
        }
         else
        {
            EndDlg(kCQCGKit::ridDlg_EdExpr_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr_Save)
    {
        if (bValidate())
            EndDlg(kCQCGKit::ridDlg_EdExpr_Save);
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr_SelValue)
    {
        // Do a validate to get all the data stored
        TString strErr;
        if (bValidate(strErr))
        {
            // It's good. If the current setup needs a comparison value, then get one
            if (m_exprEdit.bHasCompVal())
            {
                TString strVal;
                tCIDLib::TBoolean bRes = facCQCGKit().bQueryFldValue
                (
                    *this
                    , m_flddAssoc
                    , m_strDevice
                    , TPoint::pntOrigin
                    , strVal
                    , m_pwndCompVal->strWndText()
                );

                if (bRes)
                    m_pwndCompVal->strWndText(strVal);
            }
        }
         else
        {
            TErrBox msgbErr(strErr);
            msgbErr.ShowIt(*this);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We need to watch for changes in the devices list and update the field
//  list.
//
tCIDCtrls::EEvResponses TCQCEdExprDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr_Devices)
        {
            tCIDLib::TCard4 c4DevInd = m_pwndDevices->c4CurItem();
            if (c4DevInd != kCIDLib::c4MaxCard)
            {
                TString strMon;
                m_pwndDevices->ValueAt(c4DevInd, strMon);

                // Find the field list for this device
                const tCQCKit::TFldDefList& colFlds = m_cfcData.colFieldListFor(strMon);
                const tCIDLib::TCard4 c4FldCnt = colFlds.c4ElemCount();

                // If we got some, load them, else clear the field list
                if (c4FldCnt)
                {
                    tCIDLib::TCard4 c4SelInit = 0;
                    tCIDLib::TStrList colToLoad;
                    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
                    {
                        const TString& strCurFld = colFlds[c4Index].strName();
                        colToLoad.objAdd(strCurFld);
                        if (strCurFld == m_strField)
                            c4SelInit = c4Index;
                    }

                    //
                    //  If the device is the original device, default to the
                    //  original field if found. Else just take the first one.
                    //
                    if (strMon != m_strDevice)
                        c4SelInit = 0;
                    m_pwndFields->LoadList(colToLoad);
                    m_pwndFields->SelectByIndex(c4SelInit);
                }
                 else
                {
                    m_pwndFields->RemoveAll();
                }
            }
             else
            {
                // Shouldn't happen, but empty the field list if so
                m_pwndFields->RemoveAll();
            }
        }
         else if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr_ExprType)
        {
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TCQCEdExprDlg::LoadLists()
{
    tCIDLib::TStrList colToLoad;

    //
    //  We have an item for regular expression. Else, it is a statement so
    //  the rest are the available statement types. Note we START on the first
    //  used statement type, skipping the None entry.
    //
    //  We don't have a Not Used/None choice in this case, unlike some places
    //  where expressions are edited. If they are calling this dialog there has
    //  to be an expression.
    //
    colToLoad.objAdd(L"Regular Expr");
    tCIDLib::ForEachE<tCQCKit::EStatements, tCQCKit::EStatements::FirstUsed>
    (
        [&colToLoad](const auto eStmt)
        {
            colToLoad.objAdd(tCQCKit::strXlatEStatements(eStmt));
            return kCIDLib::True;
        }
    );

    // Figure out which one to select
    tCIDLib::TCard4 c4SelInit;
    if (m_exprEdit.eType() == tCQCKit::EExprTypes::RegEx)
        c4SelInit = 0;
    else
        c4SelInit = tCIDLib::c4EnumOrd(m_exprEdit.eStatement());

    // Load up the types comb box
    m_pwndExprType->LoadList(colToLoad);
    m_pwndExprType->SelectByIndex(c4SelInit);

    //
    //  We load up the list of devices, selecting the previous one if we find
    //  it. When we load
    //
    m_cfcData.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));

    // Query all of the devices
    const tCIDLib::TCard4 c4DevCount = m_cfcData.c4DevCount();
    if (c4DevCount)
    {
        tCIDLib::TStrList colDevs(c4DevCount);
        m_cfcData.QueryDevList(colDevs);

        // And load them into the devices list
        tCIDLib::TBoolean bFoundInit = kCIDLib::False;
        c4SelInit = 0;
        colToLoad.RemoveAll();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4DevCount; c4Index++)
        {
            const TString& strCurDev = colDevs[c4Index];
            colToLoad.objAdd(strCurDev);
            if (strCurDev == m_strDevice)
            {
                bFoundInit = kCIDLib::True;
                c4SelInit = c4Index;
            }
        }
        m_pwndDevices->LoadList(colToLoad);
        m_pwndDevices->SelectByIndex(c4SelInit);

        //
        //  Now let's load the fields for this device into the field list. The
        //  notification handler isn't set yet, so that we have full control.
        //  So we need to load it ourself and select the in coming field if
        //  that is appropriate.
        //
        const tCQCKit::TFldDefList& colFlds = m_cfcData.colFieldListFor
        (
            colDevs[c4SelInit]
        );

        const tCIDLib::TCard4 c4FldCnt = colFlds.c4ElemCount();
        if (c4FldCnt)
        {
            c4SelInit = 0;
            colToLoad.RemoveAll();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4FldCnt; c4Index++)
            {
                const TString& strCurFld = colFlds[c4Index].strName();
                colToLoad.objAdd(strCurFld);
                if (strCurFld == m_strField)
                    c4SelInit = c4Index;
            }

            //
            //  If we didn't find the original device, we ignore a potential
            //  match in the device we ended up selecting.
            //
            if (!bFoundInit)
                c4SelInit = 0;
            m_pwndFields->LoadList(colToLoad);
            m_pwndFields->SelectByIndex(c4SelInit);
        }
    }
}





// ---------------------------------------------------------------------------
//   CLASS: TCQCEdExprDlg2
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TCQCEdExprDlg2: Constructors and Destructor
// ---------------------------------------------------------------------------
TCQCEdExprDlg2::TCQCEdExprDlg2() :

    m_bChangeDescr(kCIDLib::True)
    , m_eType(tCQCKit::EFldTypes::String)
    , m_pexprOrg(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndCompVal(nullptr)
    , m_pwndDescription(nullptr)
    , m_pwndExprType(nullptr)
    , m_pwndNegate(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndSelValButton(nullptr)
{
}

TCQCEdExprDlg2::~TCQCEdExprDlg2()
{
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg2: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TCQCEdExprDlg2::bRunDlg(const   TWindow&            wndOwner
                        , const TString&            strTitle
                        , const tCQCKit::EFldTypes  eType
                        , const TString&            strLimits
                        ,       TCQCExpression&     exprEdit
                        , const tCIDLib::TBoolean   bChangeDescr)
{
    // Make a copy of the info for us to edit or use later
    m_bChangeDescr = bChangeDescr;
    m_eType = eType;
    m_exprEdit = exprEdit;
    m_pexprOrg = &exprEdit;
    m_strLimits = strLimits;
    m_strTitle = strTitle;

    //
    //  The first time, let it auto center the dialog within the owner. On
    //  subsequent trips, we call the version that overrides the centering
    //  flag and pass in the caller's now updated point.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCGKit(), kCQCGKit::ridDlg_EdExpr2
    );

    // If accepted, then copy back to the caller's buffer
    if (c4Res == kCQCGKit::ridDlg_EdExpr2_Save)
    {
        // Copy the data back to the caller's parms
        exprEdit = m_exprEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg2: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TCQCEdExprDlg2::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_CompVal, m_pwndCompVal);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_Description, m_pwndDescription);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_ExprType, m_pwndExprType);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_Negate, m_pwndNegate);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_Save, m_pwndSaveButton);
    CastChildWnd(*this, kCQCGKit::ridDlg_EdExpr2_SelValue, m_pwndSelValButton);

    // Set the title bar text to what we were given
    strWndText(m_strTitle);

    // Load up the widgets
    InitWidgets();

    //
    //  If the incoming value doesn't need a comp value, then disable the value select
    //  button and the comp val entry field.
    //
    if (!m_exprEdit.bHasCompVal())
    {
        m_pwndSelValButton->SetEnable(kCIDLib::False);
        m_pwndCompVal->SetEnable(kCIDLib::False);
    }

    // And register our handlers
    m_pwndCancelButton->pnothRegisterHandler(this, &TCQCEdExprDlg2::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TCQCEdExprDlg2::eClickHandler);
    m_pwndSelValButton->pnothRegisterHandler(this,&TCQCEdExprDlg2::eClickHandler);
    m_pwndExprType->pnothRegisterHandler(this,&TCQCEdExprDlg2::eListHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TCQCEdExprDlg2: Private, non-virtual methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TCQCEdExprDlg2::bValidate()
{
    TString strErrMsg;
    if (!bValidate(strErrMsg))
    {
       TErrBox msgbErr(facCQCGKit().strMsg(kGKitErrs::errcExpr_ExprError), strErrMsg);
        msgbErr.ShowIt(*this);
        return kCIDLib::False;
    }
    return kCIDLib::True;
}

tCIDLib::TBoolean TCQCEdExprDlg2::bValidate(TString& strErrMsg)
{
    // We have to have a description
    const TString strDescr(m_pwndDescription->strWndText());
    if (strDescr.bIsEmpty())
    {
        strErrMsg = facCQCGKit().strMsg(kGKitErrs::errcExpr_EmptyDescr);
        return kCIDLib::False;
    }

    //
    //  Validate the expression stuff. According to the expression type
    //  they selected, we validate it differently. Note that we have to
    //  play some tricks here to map the index back to the expression type
    //  enum.
    //
    tCQCKit::EExprTypes eType;
    tCQCKit::EStatements eStatement;
    GetSelectedType(eType, eStatement);

    // Get any comp value if it is needed
    TString strCompVal;
    if (TCQCExpression::bNeedsCompVal(eType, eStatement))
        strCompVal = m_pwndCompVal->strWndText();

    // Get the negation flag
    const tCIDLib::TBoolean bNegate = m_pwndNegate->bCheckedState();

    //
    //  Ok, we have all the info so set up the expression. We could still get an error
    //  so catch them and report them.
    //
    try
    {
        m_exprEdit.Set(strDescr, eType, eStatement, strCompVal, bNegate);
    }

    catch(const TError& errToCatch)
    {
        strErrMsg = errToCatch.strErrText();
        return kCIDLib::False;
    }

    // Nothing obvious, so validate it. We don't know the field type here so indicate none
    if (!m_exprEdit.bValidate(strErrMsg, tCQCKit::EFldTypes::Count))
        return kCIDLib::False;

    return kCIDLib::True;
}


tCIDCtrls::EEvResponses
TCQCEdExprDlg2::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr2_Cancel)
    {
        TString strErrMsg;
        if (!bValidate(strErrMsg) || (m_exprEdit != *m_pexprOrg))
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCGKit().strMsg(kGKitMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCGKit::ridDlg_EdExpr2_Cancel);
        }
         else
        {
            EndDlg(kCQCGKit::ridDlg_EdExpr2_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr2_Save)
    {
        if (bValidate())
            EndDlg(kCQCGKit::ridDlg_EdExpr2_Save);
    }
     else if (wnotEvent.widSource() == kCQCGKit::ridDlg_EdExpr2_SelValue)
    {
        // Get the expression type info and see if it's a statement that needs a comp value
        tCQCKit::EExprTypes eType;
        tCQCKit::EStatements eStatement;
        GetSelectedType(eType, eStatement);
        if (TCQCExpression::bNeedsCompVal(eType, eStatement))
        {
            //
            //  It will give us back the canonical format of the value, so we pass in the
            //  current value and get back the canonical format which we set back out.
            //
            TString strVal;
            tCIDLib::TBoolean bRes = facCQCGKit().bQueryLimitValue
            (
                *this
                , L"Select Comparison Value"
                , m_eType
                , m_strLimits
                , TPoint::pntOrigin
                , strVal
                , m_pwndCompVal->strWndText()
            );

            if (bRes)
            {
                m_pwndCompVal->strWndText(strVal);
                m_exprEdit.strCompVal(strVal);
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We handle the combo boxes so that we can enable/disable the value selection button
//  depending on whether a comp value is valid. We also empty the comp value field and
//  disable it.
//
tCIDCtrls::EEvResponses TCQCEdExprDlg2::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        tCQCKit::EExprTypes eType;
        tCQCKit::EStatements eStatement;
        GetSelectedType(eType, eStatement);

        const tCIDLib::TBoolean bNeedComp = TCQCExpression::bNeedsCompVal(eType, eStatement);
        m_pwndSelValButton->SetEnable(bNeedComp);
        m_pwndCompVal->SetEnable(bNeedComp);
        if (!bNeedComp)
            m_pwndCompVal->ClearText();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// A helper to get the expression type and statement values from the controls
tCIDLib::TVoid
TCQCEdExprDlg2::GetSelectedType(tCQCKit::EExprTypes&    eType
                                , tCQCKit::EStatements& eStatement) const
{
    //
    //  Validate the expression stuff. According to the expression type
    //  they selected, we validate it differently. Note that we have to
    //  play some tricks here to map the index back to the expression type
    //  enum.
    //
    eStatement = tCQCKit::EStatements::None;
    TString strCompVal;
    const tCIDLib::TCard4 c4ExprInd = m_pwndExprType->c4CurItem();
    if (c4ExprInd == 0)
    {
       eType = tCQCKit::EExprTypes::None;
    }
     else if (c4ExprInd == 1)
    {
        eType = tCQCKit::EExprTypes::RegEx;
    }
     else
    {
        //
        //  Subtract 1 for the statement type. It's not 2 because there's a None value
        //  in the statement enum as well and we don't load it.
        //
        eType = tCQCKit::EExprTypes::Statement;
        eStatement = tCQCKit::EStatements(c4ExprInd - 1);
    }
}


// Get the incoming info loaded up into the controls
tCIDLib::TVoid TCQCEdExprDlg2::InitWidgets()
{
    //
    //  We have an item for regular expression. Else, it is a statement so
    //  the rest are the available statement types.
    //
    tCIDLib::TStrList colToLoad;
    colToLoad.objAdd(L"Unused");
    colToLoad.objAdd(L"Regular Expr");
    tCQCKit::EStatements eStmnt = tCQCKit::EStatements::FirstUsed;
    for (; eStmnt <= tCQCKit::EStatements::Max; eStmnt++)
        colToLoad.objAdd(tCQCKit::strXlatEStatements(eStmnt));

    // Figure out which one to select
    tCIDLib::TCard4 c4SelInit;
    if (m_exprEdit.eType() == tCQCKit::EExprTypes::None)
        c4SelInit = 0;
    else if (m_exprEdit.eType() == tCQCKit::EExprTypes::RegEx)
        c4SelInit = 1;
    else
        c4SelInit = tCIDLib::c4EnumOrd(m_exprEdit.eStatement()) + 1;

    // Load up the types comb box
    m_pwndExprType->LoadList(colToLoad);
    m_pwndExprType->SelectByIndex(c4SelInit);

    // If we have any comp value, load that
    if (m_exprEdit.bHasCompVal())
        m_pwndCompVal->strWndText(m_exprEdit.strCompVal());

    // Set the negation flag
    m_pwndNegate->SetCheckedState(m_exprEdit.bNegated());

    // And the description. If it can't be edited, then disable it
    m_pwndDescription->strWndText(m_exprEdit.strDescription());
    if (!m_bChangeDescr)
        m_pwndDescription->SetEnable(kCIDLib::False);
}

