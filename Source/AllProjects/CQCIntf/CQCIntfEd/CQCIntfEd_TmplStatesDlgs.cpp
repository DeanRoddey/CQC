//
// FILE NAME: CQCAdmin_TmplStatesDlg.cpp
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
//  This file implements dialogs that let the user edit the user states. We need
//  a set of them, for each level in the process, but the final one (expressions)
//  are handled by CQCGKit, so we only need two here.
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
#include    "CQCIntfEd_TmplStatesDlgs_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TTmplStatesDlg, TDlgBox)
RTTIDecls(TTmplStateDlg, TDlgBox)



// ---------------------------------------------------------------------------
//  CLASS: TTStateAttrEd
// PREFIX: wnd
//
//  Our derivative of the base attr editing window, so that we can override some
//  calls and handle visual editing of some of our own types. Otherwise, we don't
//  get involved.
// ---------------------------------------------------------------------------
class TTStateAttrEd : public TAttrEditWnd
{
    public :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TTStateAttrEd() {}

        TTStateAttrEd(const TTStateAttrEd&) = delete;

        ~TTStateAttrEd() {}


        // -------------------------------------------------------------------
        //  Public data members
        // -------------------------------------------------------------------
        TTmplStateDlg*  m_pwndOwner;



    protected :
        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bVisEdit
        (
                    TAttrData&              adatEdit
            , const TArea&                  areaCellScr
            , const tCIDLib::TCard8         c8UserId
        )   override;


    private :
        // -------------------------------------------------------------------
        //  Magic macros
        // -------------------------------------------------------------------
        RTTIDefs(TTStateAttrEd, TAttrEditWnd);
};

//
//  We have to support dynamic creation since our parent window creates its children
//  via a dialog description.
//
AdvRTTIDecls(TTStateAttrEd, TAttrEditWnd)

tCIDLib::TBoolean
TTStateAttrEd::bVisEdit(        TAttrData&      adatEdit
                        , const TArea&          areaCellScr
                        , const tCIDLib::TCard8 c8UserId)
{
    //
    //  The returns means there's an editor for the attribute, not if we set a new value.
    //  So we assume true. If not one we handle, we pass to the parent and let him set
    //  this.
    //
    tCIDLib::TBoolean bRet = kCIDLib::True;

    if (adatEdit.strId() == kCQCIntfEng::strAttr_StateItem_Field)
    {
        // Call a helper to let them pick a field
        TString strEdit = adatEdit.strValue();
        TCQCFldFilter* pffiltToUse = new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite);
        if (facCQCIGKit().bSelectField(*m_pwndOwner, pffiltToUse, strEdit))
            adatEdit.SetString(strEdit);
    }
     else if (adatEdit.strId() == kCQCKit::strAttr_Expr_CompVal)
    {
        //
        //  Let's see if we have a field. If so, we want to invoke a visual value
        //  selection dialog. Else, we tell them they need to set a field first. If
        //  the field doesn't exist, we also fail, since we need to get type info
        //  for the field.
        //
        const TAttrData& adatFld = adatFindById(kCQCIntfEng::strAttr_StateItem_Field);

        TString strMon, strFld;
        if (adatFld.strValue().bIsEmpty()
        ||  !facCQCKit().bParseFldName(adatFld.strValue(), strMon, strFld))
        {
            TErrBox msgbNoFld(L"You must select a valid field first");
            msgbNoFld.ShowIt(*this);
        }
         else
        {
            // Let's see if we can get info on this field
            TCQCFldDef flddComp;
            if (facCQCKit().bQueryFldDef(strMon, strFld, flddComp))
            {
                // Get a field limits object for this field
                TCQCFldLimit* pfldlComp = TCQCFldLimit::pfldlMakeNew(flddComp);
                TJanitor<TCQCFldLimit> janLims(pfldlComp);

                //
                //  If it doesn't report any sort of optimal representation for this
                //  field type, we'll just pass it on and they will get a generic
                //  text entry. Else, we'll invoke the field value getter.
                //
                if (pfldlComp->eOptimalRep() == tCQCKit::EOptFldReps::None)
                {
                    bRet = TAttrEditWnd::bVisEdit(adatEdit, areaCellScr, c8UserId);
                }
                 else
                {
                    // We pass any previous comparison value as the original value
                    TString strNewVal;
                    const tCIDLib::TBoolean bNewVal = facCQCGKit().bQueryFldValue
                    (
                        *this
                        , flddComp
                        , strMon
                        , TPoint::pntOrigin
                        , strNewVal
                        , adatEdit.strValue()
                    );

                    if (bNewVal)
                        adatEdit.SetString(strNewVal);
                }
            }
             else
            {
                TString strErr(L"Field '");
                strErr.Append(adatFld.strValue());
                strErr.Append(L"' was not found, please select a valid field");
                TErrBox msgbNoFld(strErr);
                msgbNoFld.ShowIt(*this);
            }
        }
    }
     else
    {
        bRet = TAttrEditWnd::bVisEdit(adatEdit, areaCellScr, c8UserId);
    }
    return bRet;
}



// ---------------------------------------------------------------------------
//   CLASS: TTmplStateDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTmplStateDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TTmplStateDlg::TTmplStateDlg() :

    m_pwndAddButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndEditor(nullptr)
    , m_pwndList(nullptr)
    , m_pwndLogOp(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TTmplStateDlg::~TTmplStateDlg()
{
}


// ---------------------------------------------------------------------------
//  TTmplStateDlg: Public, inherited methods
// ---------------------------------------------------------------------------

//
//  This is only called for the stuff in the main list, for which we support ad
//  hoc IPE to change the description.
//
tCIDLib::TBoolean
TTmplStateDlg::bIPETestCanEdit( const   tCIDCtrls::TWndId   widSrc
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    if (widSrc == kCQCIntfEd::ridDlg_TState_List)
    {
        //
        //  We should only see this from the main list, which is doing ad hoc IPE,
        //  so this is called to get attribute editing data for that stuff.
        //
        if (c4ColInd == 0)
        {
            adatFill.Set
            (
                L"Description", kCQCKit::strAttr_Expr_Descr, tCIDMData::EAttrTypes::String
            );
            return kCIDLib::True;
        }
    }
    return kCIDLib::False;
}


//
//  This will be called for both the main list and the interface editor, so we can
//  do extra validation where needed.
//
tCIDLib::TBoolean
TTmplStateDlg::bIPEValidate(const   TString&            strSrc
                            ,       TAttrData&          adatTar
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    // Let the base IPE class do its basic validation
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    // If it's one of ours we need to specially check do that
    if (adatTar.strId() == kCQCIntfEng::strAttr_StateItem_Field)
    {
        // Make sure it's a parseable field name
        TString strMon, strFld;
        if (!facCQCKit().bParseFldName(strNewVal, strMon, strFld))
        {
            strErrMsg = L"The field name is not correctly formatted";
            return kCIDLib::False;
        }
    }
     else if (adatTar.strId() == kCQCKit::strAttr_Expr_CompVal)
    {
        //
        //  If the current item is a statement type, and it requires a value, make
        //  sure that the value isn't empty. If it doesn't, make sure it is.
        //
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index == kCIDLib::c4MaxCard)
        {
            strErrMsg = L"There doesn't appear to be a state selected";
            return kCIDLib::False;
        }

        const TCQCIntfStateItem& istiCur = m_istEdit.istiAt(c4Index);
        if (istiCur.exprEval().eType() == tCQCKit::EExprTypes::Statement)
        {
            if (TCQCExpression::bNeedsCompVal(istiCur.exprEval().eStatement()))
            {
                if (strNewVal.bIsEmpty())
                {
                    strErrMsg = L"The current statement type requires a comparison value";
                    return kCIDLib::False;
                }
            }
             else
            {
                if (!strNewVal.bIsEmpty())
                {
                    strErrMsg = L"The current statement type does not use a comparison value";
                    return kCIDLib::False;
                }
            }
        }
         else
        {
            if (strNewVal.bIsEmpty())
            {
                strErrMsg = L"Regular expressions requires a comparison value";
                return kCIDLib::False;
            }
        }
    }
    return kCIDLib::True;
}


//
//  Here we really only need to watch for the description to change, when they do an
//  IPE of the description in the main list. Otherwise, we handle everything in the
//  notification handler from the attribute editor, since where we get notifications
//  for any type of change (visual, left/right arrow, etc...)
//
tCIDLib::TVoid
TTmplStateDlg::IPEValChanged(const  tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    if ((widSrc == kCQCIntfEd::ridDlg_TState_List)
    &&  (adatNew.strId() == kCQCKit::strAttr_Expr_Descr))
    {
        const tCIDLib::TCard4 c4Index = c4IPERow();
        m_pwndList->SetColText(c4Index, 0, adatNew.strValue());

        TCQCIntfStateItem& istiCur = m_istEdit.istiAt(c4Index);
        istiCur.SetAttr(adatNew, adatOld);
    }
}


// ---------------------------------------------------------------------------
//  TTmplStateDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTmplStateDlg::bRun(const  TWindow& wndOwner, TCQCIntfState& istEdit)
{
    // Make a copy for our editing purposes, and a pointer to the original
    m_istEdit = istEdit;
    m_pistOrg = &istEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_TState
    );

    if (c4Res == kCQCIntfEd::ridDlg_TState_Save)
    {
        // Copy the changes back
        istEdit = m_istEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TTmplStateDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTmplStateDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_Add, m_pwndAddButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_Delete, m_pwndDelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_Editor, m_pwndEditor);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_LogOp, m_pwndLogOp);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TState_Save, m_pwndSaveButton);

    // Register our handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TTmplStateDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TTmplStateDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TTmplStateDlg::eClickHandler);
    m_pwndEditor->pnothRegisterHandler(this, &TTmplStateDlg::eAttrEditHandler);
    m_pwndList->pnothRegisterHandler(this, &TTmplStateDlg::eListHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TTmplStateDlg::eClickHandler);

    // Set us on the editor window as it's owner
    m_pwndEditor->m_pwndOwner = this;

    // ANd set us as his IPE handler
    m_pwndEditor->SetIPEHandler(this);

    // Set the title text
    TString strTitle = strWndText();
    strTitle.eReplaceToken(m_istEdit.strName(), kCIDLib::chDigit1);
    strWndText(strTitle);

    // Load up the logical ops
    tCQCKit::ELogOps eLogOp = tCQCKit::ELogOps::Min;
    while (eLogOp <= tCQCKit::ELogOps::Max)
    {
        m_pwndLogOp->c4AddItem(tCQCKit::strAltXlatELogOps(eLogOp));
        eLogOp++;
    }

    // And select the current op
    m_pwndLogOp->SelectByIndex(tCIDLib::c4EnumOrd(m_istEdit.eLogOp()));

    // Load the list of expressions
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(facCQCIntfEd().strMsg(kIEdMsgs::midDlg_TState_StateExprs));
    m_pwndList->SetColumns(colCols);

    const tCIDLib::TCard4 c4Count = m_istEdit.c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfStateItem& istiCur = m_istEdit.istiAt(c4Index);
        colCols[0] = istiCur.strDescription();
        m_pwndList->c4AddItem(colCols, 0);
    }

    // Select the first one if there are any
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    //
    //  Set us as the IPE on the main list window, since we are doing ad hoc in
    //  place editing on that one.
    //
    m_pwndList->SetAutoIPE(this);

    // Set our title text on the attribute editor so it'll get used by any popups it does
    m_pwndEditor->strWndText(strWndText());

    return bRet;
}


//
//  We use our own little derivative of the attribute editor window, but we use a
//  dialog resource to create our contents. So we override this in order to get a
//  chance to modify the dialog item. We'll change the class name to our own class.
//  We don't need to affect any of the creation related functionality, so as long
//  as our type can be dynamically created, the parent class' stuff will be used.
//
tCIDLib::TVoid TTmplStateDlg::ModDlgItem(TDlgItem& dlgiToMod)
{
    if (dlgiToMod.ridChild() == kCQCIntfEd::ridDlg_TState_Editor)
        dlgiToMod.strCppType(L"TTStateAttrEd");
}


// ---------------------------------------------------------------------------
//  TTmplStateDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

//
//  We respond to changes to attributes by the attribute editor here. The only thing
//  we have to handle separately is the IPE of the main list to edit the state item
//  description. That's done via ad hoc IPE and so it comes to us via the IPE callback
//  interface.
//
tCIDCtrls::EEvResponses TTmplStateDlg::eAttrEditHandler(TAttrEditInfo& wnotInfo)
{
    // Get the currently selected state. Should never be bad but just in case
    const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
    if (c4Index == kCIDLib::c4MaxCard)
        return tCIDCtrls::EEvResponses::Handled;

    TCQCIntfStateItem& istiCur = m_istEdit.istiAt(c4Index);

    // Pass them all on to the state item object first
    const TAttrData& adatNew = wnotInfo.adatNew();
    istiCur.SetAttr(adatNew, wnotInfo.adatOld());

    // And do any other stuff we ned to do at our level
    if (adatNew.strId() == kCQCKit::strAttr_Expr_Descr)
    {
        // In this case we need to update our list window
        m_pwndList->SetColText(c4Index, 0, adatNew.strValue());
    }
     else if (adatNew.strId() == kCQCKit::strAttr_Expr_Type)
    {
        // Enable/disable the statement attr, based on the type
        const tCIDLib::TBoolean bEnable
        (
            istiCur.exprEval().eType() == tCQCKit::EExprTypes::Statement
        );

        m_pwndEditor->ChangeEditType
        (
            kCQCKit::strAttr_Expr_Statement
            , bEnable ? tCIDMData::EAttrEdTypes::Both : tCIDMData::EAttrEdTypes::None
        );

        //
        //  Update the statement attribute as well, since it may have been changed
        //  when we changed the type.
        //
        m_pwndEditor->UpdateString
        (
            kCQCKit::strAttr_Expr_Statement
            , tCQCKit::strXlatEStatements(istiCur.exprEval().eStatement())
        );
    }

    //
    //  If we don't need a comp value anymore, then make sure it's cleared and disabled.
    //  Else make sure it's enabled.
    //
    if ((adatNew.strId() == kCQCKit::strAttr_Expr_Type)
    ||  (adatNew.strId() == kCQCKit::strAttr_Expr_Statement))
    {
        if (!istiCur.exprEval().bHasCompVal())
        {
            m_pwndEditor->UpdateString(kCQCKit::strAttr_Expr_CompVal, TString::strEmpty());
            m_pwndEditor->ChangeEditType
            (
                kCQCKit::strAttr_Expr_CompVal, tCIDMData::EAttrEdTypes::None
            );
        }
         else
        {
            m_pwndEditor->ChangeEditType
            (
                kCQCKit::strAttr_Expr_CompVal, tCIDMData::EAttrEdTypes::Both
            );
        }
    }

    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses
TTmplStateDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TState_Add)
    {
        // Add a default one, they can modify it as desired.
        TCQCExpression exprInit
        (
            L"New Expression"
            , tCQCKit::EExprTypes::Statement
            , tCQCKit::EStatements::IsTrue
            , TString::strEmpty()
            , kCIDLib::False
        );
        TCQCIntfStateItem istiNew(exprInit);
        tCIDLib::TCard4 c4At = m_istEdit.c4AddItem(istiNew);

        // Add it to the list and select it
        tCIDLib::TStrList colVals(1);
        colVals.objAdd(istiNew.strDescription());

        m_pwndList->c4AddItem(colVals, 0, c4At);
        m_pwndList->SelectByIndex(c4At);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TState_Cancel)
    {
        // If there are changes, make sure they want to do it
        if (m_istEdit != *m_pistOrg)
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_TState_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_TState_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TState_Delete)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            m_istEdit.DeleteAt(c4Index);
            m_pwndList->RemoveAt(c4Index);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TState_Save)
    {
        try
        {
            TDataSrvClient dsclVal;
            TCQCFldCache cfcVal;
            cfcVal.Initialize(new TCQCFldFilter(tCQCKit::EReqAccess::MReadCWrite));

            //
            //  Do some validation, since we can't enforce it all via the attribute
            //  editor.
            //
            tCQCIntfEng::TErrList   colErrs;
            TString                 strErrMsg;
            const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
            for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
            {
                const TCQCIntfStateItem& istiCur = m_istEdit.istiAt(c4Index);

                colErrs.RemoveAll();
                istiCur.Validate(cfcVal, colErrs, dsclVal);

                if (!colErrs.bIsEmpty())
                {
                    m_pwndList->SelectByIndex(c4Index);
                    TErrBox msgbErr(strWndText(), colErrs[0].strErrText());
                    msgbErr.ShowIt(*this);
                    return tCIDCtrls::EEvResponses::Handled;
                }
            }

            // Store the logical ops value
            m_istEdit.eLogOp(tCQCKit::ELogOps(m_pwndLogOp->c4CurItem()));
        }

        catch(TError& errToCatch)
        {
            TErrBox msgbErr(strWndText(), errToCatch.strErrText());
            msgbErr.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        EndDlg(kCQCIntfEd::ridDlg_TState_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TTmplStateDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    //
    //  When a new state item expression gets selected, we query it for its attributes
    //  and load them to the attribute editor. We pass ourself as the IPE interface,
    //  which in this case we only use for storing the updated values.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        const TCQCIntfStateItem& istiCur = m_istEdit.istiAt(wnotEvent.c4Index());

        tCIDMData::TAttrList    colAttrs;
        TAttrData               adatTmp;
        istiCur.QueryAttrs(colAttrs, adatTmp);

        // And load them up
        m_pwndEditor->LoadValues(colAttrs, 0);
    }
    return tCIDCtrls::EEvResponses::Handled;
}




// ---------------------------------------------------------------------------
//   CLASS: TTmplStatesDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTmplStatesDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TTmplStatesDlg::TTmplStatesDlg() :

    m_pwndAddButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndEditButton(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TTmplStatesDlg::~TTmplStatesDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TTmplStatesDlg::bIPETestCanEdit(const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // Set up the passed attribute data object with the info to edit
    if (c4ColInd == 0)
    {
        TCQCIntfState& istCur = m_istlEdit.istAt(c4IPERow());
        adatFill.Set(L"Negate", L"Negate", tCIDMData::EAttrTypes::Bool);
        adatFill.SetBool(istCur.bNegated());
        bValueSet = kCIDLib::True;
    }
     else if (c4ColInd == 1)
    {
        adatFill.Set(L"Name", L"Name", tCIDMData::EAttrTypes::String);
    }
    return kCIDLib::True;
}


tCIDLib::TBoolean
TTmplStatesDlg::bIPEValidate(const  TString&            strSrc
                            ,       TAttrData&          adatTar
                            , const TString&            strNewVal
                            ,       TString&            strErrMsg
                            ,       tCIDLib::TCard8&    c8UserId) const
{
    //
    //  Call the base mixin class to do basic validation. If that fails, then
    //  no need to look further.
    //
    if (!MIPEIntf::bIPEValidate(strSrc, adatTar, strNewVal, strErrMsg, c8UserId))
        return kCIDLib::False;

    if (adatTar.strId() == L"Name")
    {
        //
        //  Watch for a dup name. If we find this name, and it's not the row we are
        //  editing, then it'sa dup.
        //
        tCIDLib::TCard4 c4FndAt;
        if (m_istlEdit.bFindState(strNewVal, c4FndAt) && (c4IPERow() != c4FndAt))
        {
            strErrMsg = L"This key is already taken";
            return kCIDLib::False;
        }
        adatTar.SetString(strNewVal);
    }
    return kCIDLib::True;
}


tCIDLib::TVoid
TTmplStatesDlg::IPEValChanged(  const   tCIDCtrls::TWndId   widSrc
                                , const TString&            strWndName
                                , const TAttrData&          adatNew
                                , const TAttrData&          adatOld
                                , const tCIDLib::TCard8     )
{
    TCQCIntfState& istCur = m_istlEdit.istAt(c4IPERow());

    if (adatNew.strId() == L"Negate")
        istCur.bNegated(adatNew.bVal());
    if (adatNew.strId() == L"Name")
        istCur.strName(adatNew.strValue());
}


// ---------------------------------------------------------------------------
//  TTmplStatesDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TTmplStatesDlg::bRun(const  TWindow& wndOwner, TCQCIntfStateList& istlEdit)
{
    // Make a copy for our editing purposes, and a pointer to the original
    m_istlEdit = istlEdit;
    m_pistlOrg = &istlEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_TStates
    );

    if (c4Res == kCQCIntfEd::ridDlg_TStates_Save)
    {
        // Copy the changes back
        istlEdit = m_istlEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TTmplStatesDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TTmplStatesDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_Add, m_pwndAddButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_Delete, m_pwndDelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_Edit, m_pwndEditButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_TStates_Save, m_pwndSaveButton);

    // Register our handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TTmplStatesDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TTmplStatesDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TTmplStatesDlg::eClickHandler);
    m_pwndEditButton->pnothRegisterHandler(this, &TTmplStatesDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TTmplStatesDlg::eClickHandler);

    // Load the list of states
    tCIDLib::TStrList colCols(2);
    colCols.objAdd(facCQCIntfEd().strMsg(kIEdMsgs::midDlg_TStates_Neg));
    colCols.objAdd(facCQCIntfEd().strMsg(kIEdMsgs::midDlg_TStates_CurStates));
    m_pwndList->SetColumns(colCols);

    const tCIDLib::TCard4 c4Count = m_istlEdit.c4StateCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCIntfState& istCur = m_istlEdit.istAt(c4Index);

        colCols[0] = facCQCKit().strBoolVal(istCur.bNegated());
        colCols[1] = istCur.strName();

        m_pwndList->c4AddItem(colCols, 0);
    }

    // Select the first one if there are any
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    // Set us as the IPE on the list window
    m_pwndList->SetAutoIPE(this);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TTmplStatesDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------

// Creates a new default state name when the user does an Add
tCIDLib::TBoolean TTmplStatesDlg::bMakeDefName(TString& strToFill)
{
    tCIDLib::TCard4 c4Index = 1;
    while (c4Index < 128)
    {
        strToFill = L"New State";
        strToFill.AppendFormatted(c4Index);

        tCIDLib::TCard4 c4At;
        if (!m_istlEdit.bFindState(strToFill, c4At))
            break;

        c4Index++;
    }
    return (c4Index < 128);
}


tCIDCtrls::EEvResponses
TTmplStatesDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TStates_Add)
    {
        //
        //  Add an empty one with a default name. They can edit it in place if they
        //  want to.
        //
        TString strName;
        if (bMakeDefName(strName))
        {
            TCQCIntfState istNew(strName, L"Default Description");
            tCIDLib::TCard4 c4At = m_istlEdit.c4AddState(istNew);

            // Add it to the list and select it
            tCIDLib::TStrList colVals(2);
            colVals.objAdd(facCQCKit().strBoolVal(istNew.bNegated()));
            colVals.objAdd(istNew.strName());
            m_pwndList->c4AddItem(colVals, 0, c4At);
            m_pwndList->SelectByIndex(c4At);
        }
         else
        {
            TErrBox msgbErr(strWndText(), L"Could not create a new unique state name");
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TStates_Cancel)
    {
        // If there are changes, make sure they want to do it
        if (m_istlEdit != *m_pistlOrg)
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_TStates_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_TStates_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TStates_Delete)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            const TCQCIntfState& istDel = m_istlEdit.istAt(c4Index);
            TYesNoBox msgbDel
            (
                strWndText()
                , facCQCIntfEd().strMsg(kIEdMsgs::midQ_DeleteKey, istDel.strName())
            );

            if (msgbDel.bShowIt(*this))
            {
                // Remove it from both the state list and the list window
                m_istlEdit.DeleteAt(c4Index);
                m_pwndList->RemoveAt(c4Index);
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TStates_Edit)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            TCQCIntfState& istDel = m_istlEdit.istAt(c4Index);
            TTmplStateDlg dlgEdit;
            if (dlgEdit.bRun(*this, istDel))
            {
            }
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_TStates_Save)
    {
        EndDlg(kCQCIntfEd::ridDlg_TStates_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}



