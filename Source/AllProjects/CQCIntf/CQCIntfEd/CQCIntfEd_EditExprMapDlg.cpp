//
// FILE NAME: CQCAdmin_EditExprMapDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 5/8/2016
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
//  This file implements a dialog that let's the user edit a list of named expressions,
//  currently used by the mapped image widget, but maybe other things eventually.
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
#include    "CQCIntfEd_EditExprMapDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditExprMapDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEditExprMapDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditExprMapDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditExprMapDlg::TEditExprMapDlg() :

    m_c4NextId(1)
    , m_pcolEdit(nullptr)
    , m_pwndAddButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDnButton(nullptr)
    , m_pwndInstruct(nullptr)
    , m_pwndList(nullptr)
    , m_pwndDeleteButton(nullptr)
    , m_pwndDeleteAllButton(nullptr)
    , m_pwndSaveButton(nullptr)
    , m_pwndUpButton(nullptr)
    , m_strBoolNo(facCQCKit().strBoolYesNo(kCIDLib::False))
    , m_strBoolYes(facCQCKit().strBoolYesNo(kCIDLib::True))
    , m_strRegEx(tCQCKit::strXlatEExprTypes(tCQCKit::EExprTypes::RegEx))
{
}

TEditExprMapDlg::~TEditExprMapDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditExprMapDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEditExprMapDlg::bRun(  const   TWindow&    wndOwner
                        , const TString&    strTitle
                        , const TString&    strInstruct
                        ,       TExprList&  colEdit)
{
    // Store incoming stuff for later use
    m_pcolEdit = &colEdit;
    m_strInstruct = strInstruct;
    m_strTitle = strTitle;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_EdExprMap
    );

    return (c4Res == kCQCIntfEd::ridDlg_EdExprMap_Save);
}


// ---------------------------------------------------------------------------
//  TEditExprMapDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditExprMapDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_Add, m_pwndAddButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_MoveDn, m_pwndDnButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_Instruct, m_pwndInstruct);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_Delete, m_pwndDeleteButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_DeleteAll, m_pwndDeleteAllButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_Save, m_pwndSaveButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EdExprMap_MoveUp, m_pwndUpButton);

    // Register our handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndDnButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEditExprMapDlg::eListHandler);
    m_pwndDeleteButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndDeleteAllButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);
    m_pwndUpButton->pnothRegisterHandler(this, &TEditExprMapDlg::eClickHandler);

    // Set our title and the incoming instruction text
    strWndText(m_strTitle);
    m_pwndInstruct->strWndText(m_strInstruct);

    // Set up the column titltes on the list box
    tCIDLib::TStrList colVals(4);
    colVals.objAdd(L"Key Name");
    colVals.objAdd(L"Expr Type");
    colVals.objAdd(L"Negate");
    colVals.objAdd(L"Comparison Value");
    m_pwndList->SetColumns(colVals);

    //
    //  Size the expression type and negate to their respective headers. The comp value
    //  one is set in the resource file to auto-size.
    //
    m_pwndList->AutoSizeCol(1, kCIDLib::True);
    m_pwndList->AutoSizeCol(2, kCIDLib::True);

    // Load up any incoming values
    tCIDLib::TCard4 c4Count = m_pcolEdit->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfMappedImgBase::TExprPair& kobjCur = m_pcolEdit->objAt(c4Index);
        colVals[0] = kobjCur.objKey();
        const TCQCExpression& exprCur = kobjCur.objValue();

        colVals[2] = facCQCKit().strBoolYesNo(exprCur.bNegated());

        //
        //  Get the expr/statement type loaded. If reg ex we show that. Else, if a
        //  statement we show that.
        //
        colVals[3].Clear();
        const tCQCKit::EExprTypes eType = exprCur.eType();
        if (eType == tCQCKit::EExprTypes::RegEx)
        {
            colVals[1] = tCQCKit::strXlatEExprTypes(eType);
            colVals[3] = exprCur.strCompVal();
        }
        else
        {
            colVals[1] = tCQCKit::strXlatEStatements(exprCur.eStatement());
            if (TCQCExpression::bNeedsCompVal(exprCur.eStatement()))
                colVals[3] = exprCur.strCompVal();
        }
        m_pwndList->c4AddItem(colVals, m_c4NextId++);
    }

    // If any incoming, then select the first one
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditExprMapDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

//
//  We just store the data in the controls while editing and only save it at the end. This
//  method will go through and gather it up and validate it. If good, it will return true
//  and the caller's object will be filled in with the current data.
//
//  We can show errors or not. The latter is for when we just need to gather the data up to
//  see if there are changes. If we fail and return false we know there are changes since
//  the incoming has to be valid else we'd have never allowed it to be saved.
//
tCIDLib::TBoolean
TEditExprMapDlg::bCollectResult(TExprList& colToFill, const tCIDLib::TBoolean bShowErrs)
{
    TString strKey;
    TString strNeg;
    TString strVal;

    tCIDLib::TBoolean   bNegated;
    tCQCKit::EExprTypes eType;
    tCQCKit::EStatements eStatement;
    const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_pwndList->QueryColText(c4Index, 0, strKey);
        m_pwndList->QueryColText(c4Index, 2, strNeg);
        m_pwndList->QueryColText(c4Index, 3, strVal);

        bNegated = facCQCKit().bCheckYesNoVal(strNeg);

        // Translate the expression type. If's either reg ex or it's a statement
        eType = eGetExprType(c4Index, eStatement);
        if (eType == tCQCKit::EExprTypes::RegEx)
        {
            if (strVal.bIsEmpty())
            {
                if (bShowErrs)
                {
                    TErrBox msgbErr(L"Regular expressions require a regular expression value");
                    msgbErr.ShowIt(*this);
                    m_pwndList->SelectByIndex(c4Index);
                }
                return kCIDLib::False;
            }
        }
        else
        {
            if (!TCQCExpression::bNeedsCompVal(eStatement))
            {
                if (!strVal.bIsEmpty())
                {
                    if (bShowErrs)
                    {
                        TErrBox msgbErr(L"This statement type doesn't accept a comparison value");
                        msgbErr.ShowIt(*this);
                        m_pwndList->SelectByIndex(c4Index);
                    }
                    return kCIDLib::False;
                }
            }
        }

        // Create the new pair and add to the list
        TCQCIntfMappedImgBase::TExprPair kobjNew
        (
            strKey, TCQCExpression(strKey, eType, eStatement, strVal, bNegated)
        );
        colToFill.objAdd(kobjNew);
    }
    return kCIDLib::True;
}


tCIDCtrls::EEvResponses TEditExprMapDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_Add)
    {
        tCIDLib::TStrList colVals(4);
        TString strKey(L"NewKey");
        strKey.AppendFormatted(m_c4NextId);
        colVals.objAdd(strKey);
        colVals.objAdd(tCQCKit::strXlatEStatements(tCQCKit::EStatements::IsEqual));
        colVals.objAdd(m_strBoolNo);
        colVals.objAdd(L"");

        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colVals, m_c4NextId++);

        // Select this new one
        m_pwndList->SelectByIndex(c4At);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_Cancel)
    {
        TExprList colTmp;
        if (!bCollectResult(colTmp, kCIDLib::False) || (colTmp != *m_pcolEdit))
        {
            // There are changes, even if they are maybe not valid
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_EdExprMap_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_EdExprMap_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_MoveDn)
    {
        m_pwndList->bMoveDn(kCIDLib::True);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_Delete)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            TString strKey;
            m_pwndList->QueryColText(c4Index, 0, strKey);
            TYesNoBox msgbRemove
            (
                strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteKey, strKey)
            );

            if (msgbRemove.bShowIt(*this))
                m_pwndList->RemoveAt(c4Index);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_DeleteAll)
    {
        if (m_pwndList->c4ItemCount())
        {
            TYesNoBox msgbRemove
            (
                strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteAllKeys)
            );

            if (msgbRemove.bShowIt(*this))
                m_pwndList->RemoveAll();
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_Save)
    {
        //
        //  Iterate our list and load up a temp object, validating as we go. If it's
        //  good, we copy back to the caller's data and exist with save.
        //
        TExprList colTmp;
        if (bCollectResult(colTmp, kCIDLib::True))
        {
            // It worked, so store it and exit with a save
            *m_pcolEdit = colTmp;
            EndDlg(kCQCIntfEd::ridDlg_EdExprMap_Save);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EdExprMap_MoveUp)
    {
        m_pwndList->bMoveUp(kCIDLib::True);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



// We have to do this in two places so break it out
tCQCKit::EExprTypes
TEditExprMapDlg::eGetExprType(  const   tCIDLib::TCard4         c4Index
                                ,       tCQCKit::EStatements&   eStmt) const
{
    tCQCKit::EExprTypes eType;
    TString strExpr;
    m_pwndList->QueryColText(c4Index, 1, strExpr);
    if (strExpr == m_strRegEx)
    {
        eType = tCQCKit::EExprTypes::RegEx;
        eStmt = tCQCKit::EStatements::None;
    }
    else
    {
        eType = tCQCKit::EExprTypes::Statement;
        eStmt = tCQCKit::eXlatEStatements(strExpr);
    }
    return eType;
}


//
//  We have to provide our own inplace editing, and for the expression type and negation
//  we do a drop down.
//
tCIDCtrls::EEvResponses TEditExprMapDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::ColClicked)
    {
        const tCIDLib::TCard4 c4Index = wnotEvent.c4Index();
        const tCIDLib::TCard4 c4ColIndex = wnotEvent.c4ColIndex();

        //
        //  If it's the comp value column, get the expression type for this row. If it's
        //  a statement type that doesn't take a value, ignore this.
        //
        if (c4ColIndex == 3)
        {
            tCQCKit::EStatements eStmt;
            const tCQCKit::EExprTypes eType = eGetExprType(c4Index, eStmt);
            if (!TCQCExpression::bNeedsCompVal(eType, eStmt))
                return tCIDCtrls::EEvResponses::Handled;
        }

        // Get the cell area so we can create an in-place editor over it
        TArea areaCell;
        m_pwndList->QueryColArea(c4Index, c4ColIndex, areaCell);

        // And a version mapped to the screen for any drop down menus
        TArea areaCellScr;
        m_pwndList->ToScreenCoordinates(areaCell, areaCellScr);

        // Clip it to the list's client area
        TArea areaAvail = m_pwndList->areaClient();
        if (areaCell.i4Right() >= areaAvail.i4Right())
        {
            // If there's too little space to do anything useful, give up
            if ((areaAvail.i4Right() - areaCell.i4Left()) < 24)
                return tCIDCtrls::EEvResponses::Handled;

            areaCell.i4Right(areaAvail.i4Right() - 4);
        }

        // Get the current value which will be the ingoing edit value
        TString strEdit;
        m_pwndList->QueryColText(c4Index, c4ColIndex, strEdit);

        // Set up an attribute appropriately
        if ((c4ColIndex == 0) || (c4ColIndex == 3))
        {
            // These are in-place edit, so set up an attribute
            TAttrData adatEdit;
            if (c4ColIndex == 0)
            {
                adatEdit.Set
                (
                    L"Name"
                    , L"Name"
                    , kCIDMData::strAttrLim_Required
                    , tCIDMData::EAttrTypes::String
                    , tCIDMData::EAttrEdTypes::InPlace
                );
            }
             else
            {
                adatEdit.Set
                (
                    L"Value"
                    , L"Value"
                    , tCIDMData::EAttrTypes::String
                    , tCIDMData::EAttrEdTypes::InPlace
                );
            }

            // Set the current value as the initial value
            adatEdit.SetString(strEdit);

            //
            //  And invoke the in place editor. If the user commits, just put it back
            //  into the list cell.
            //
            c4IPERow(c4Index);
            TInPlaceEdit ipeVal;
            if (ipeVal.bProcess(*m_pwndList, areaCell, adatEdit, *this, c4Index))
                m_pwndList->SetColText(c4Index, c4ColIndex, adatEdit.strValue());
        }
         else if (c4ColIndex == 1)
        {
            //
            //  We set up a menu with regex first, then all of the available statement
            //  types.
            //
            TPopupMenu menuSelect(L"Expr Type Column");
            menuSelect.Create();
            menuSelect.AddActionItem(kCIDLib::c4MaxCard, L"Regular Expr", L"RegEx", 1000);

            // The first used value in this case skips the initial 'none' value
            tCIDLib::ForEachE<tCQCKit::EStatements, tCQCKit::EStatements::FirstUsed>
            (
                [&menuSelect](const auto eStmt)
                {
                    const TString& strCur = tCQCKit::strXlatEStatements(eStmt);
                    menuSelect.AddActionItem
                    (
                        kCIDLib::c4MaxCard, strCur, strCur, 1000 + tCIDLib::TCard4(eStmt)
                    );
                }
            );

            const tCIDLib::TCard4 c4Sel = menuSelect.c4Process
            (
                *this, areaCellScr.pntLL(), tCIDLib::EVJustify::Bottom, tCIDLib::EHJustify::Left
            );

            if (c4Sel == 1000)
            {
                m_pwndList->SetColText
                (
                    c4Index, c4ColIndex, tCQCKit::strXlatEExprTypes(tCQCKit::EExprTypes::RegEx)
                );
            }
             else if ((c4Sel >= 1001)
             && (c4Sel <= 1001 + tCIDLib::c4EnumOrd(tCQCKit::EStatements::Max)))
            {
                //
                //  We skipped the 'none' value, so the selection minus 1000 will be the
                //  correct statement type.
                //
                const tCQCKit::EStatements eStatement = tCQCKit::EStatements(c4Sel - 1000);
                m_pwndList->SetColText
                (
                    c4Index, c4ColIndex, tCQCKit::strXlatEStatements(eStatement)
                );

                // If this statement type doesn't take comp val, clear that column
                if (!TCQCExpression::bNeedsCompVal(eStatement))
                    m_pwndList->SetColText(c4Index, 3, TString::strEmpty());
            }
        }
         else if (c4ColIndex == 2)
        {
            // We present a simple Yes/No drop down menu
            TPopupMenu menuSelect(L"Negated Expr Column");
            menuSelect.Create();
            menuSelect.AddActionItem(kCIDLib::c4MaxCard, m_strBoolNo, L"No", 1000);
            menuSelect.AddActionItem(kCIDLib::c4MaxCard, m_strBoolYes, L"Yes", 1001);
            const tCIDLib::TCard4 c4Sel = menuSelect.c4Process
            (
                *this, areaCellScr.pntLL(), tCIDLib::EVJustify::Bottom, tCIDLib::EHJustify::Left
            );

            // Don't assume that not yes is no, or vice versa. They could have hit escape
            if (c4Sel == 1000)
                m_pwndList->SetColText(c4Index, c4ColIndex, m_strBoolNo);
            else if (c4Sel == 1001)
                m_pwndList->SetColText(c4Index, c4ColIndex, m_strBoolYes);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

