//
// FILE NAME: CQCIGKit_MngVarsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 07/19/2006
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
//  This file implements a dialog to prompt the user for variable names,
//  mostly for when editing actions.
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
#include    "CQCIGKit_MngVarsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMngVarsDlg,TDlgBox)



namespace CQCIGKit_MngVarsDlg
{
    // -----------------------------------------------------------------------
    //  The last size/pos of the dialog, which we will use when reloaded.
    //  Default it to the default zero values. We also remember the column widths
    //  on the list box.
    // -----------------------------------------------------------------------
    TArea               areaLastPos;
    tCIDLib::TCardList  fcolColumns;
}



// ---------------------------------------------------------------------------
//   CLASS: TMngVarsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMngVarsDlg: Public, static methods
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMngVarsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TMngVarsDlg::TMngVarsDlg() :

    m_bNameOnly(kCIDLib::False)
    , m_pcavlToUse(nullptr)
    , m_pwndAdd(nullptr)
    , m_pwndClose(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndByName(nullptr)
    , m_pwndByValue(nullptr)
    , m_pwndVarList(nullptr)
{
}

TMngVarsDlg::~TMngVarsDlg()
{
}


// ---------------------------------------------------------------------------
//  TResEdTextDlg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TMngVarsDlg::bIPETestCanEdit(const  tCIDCtrls::TWndId
                            , const tCIDLib::TCard4     c4ColInd
                            ,       TAttrData&          adatFill
                            ,       tCIDLib::TBoolean&  bValueSet)
{
    // Can't edit the order column
    if (c4ColInd == 0)
        return kCIDLib::False;

    //
    //  We don't have any restrictions. Either column can be edited. So just fill in
    //  the attribute with the column text indicated.
    //
    TString strVal;
    m_pwndVarList->QueryColText(c4IPERow(), c4ColInd, strVal);

    if (c4ColInd == 1)
    {
        // This one can't be empty so set required limit
        adatFill.Set
        (
            L"Name", L"Name", kCIDMData::strAttrLim_Required, tCIDMData::EAttrTypes::String
        );

        //
        //  Store the original name as the special type, so that we can get back to
        //  the original at the end when we store the new name.
        //
        adatFill.SetSpecType(strVal);
    }
     else if (c4ColInd == 2)
    {
        adatFill.Set(L"Note", L"Note", tCIDMData::EAttrTypes::String);
    }
    adatFill.SetString(strVal);


    return kCIDLib::True;
}


tCIDLib::TBoolean
TMngVarsDlg::bIPEValidate(  const   TString&            strSrc
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
        // Make sure it's a valid variable name
        if (!TStdVarsTar::bValidVarName(strNewVal, TString::strEmpty(), strErrMsg))
            return kCIDLib::False;

        //
        //  Can't match any other existing variable. The special type has the original
        //  name, so we can skip it.
        //
        tCIDLib::TCard4 c4At;
        if (m_pcavlToUse->bNameExists(strNewVal, c4At))
        {
            strErrMsg = L"This name is already in use in the list";
            return kCIDLib::False;
        }
        adatTar.SetString(strNewVal);
    }
     else if (adatTar.strId() == L"Note")
    {
        // Nothing really can go wrong here. But we will clip it to 64 characters.
        if (strNewVal.c4Length() > 64)
        {
            TString strClip = strNewVal;
            strClip.CapAt(64);
            adatTar.SetString(strClip);
        }
         else
        {
            adatTar.SetString(strNewVal);
        }
    }
    return kCIDLib::True;
}


// We have already insured in the validation that it's not a dup of another and it's legal
tCIDLib::TVoid
TMngVarsDlg::IPEValChanged( const   tCIDCtrls::TWndId   widSrc
                            , const TString&            strSrc
                            , const TAttrData&          adatNew
                            , const TAttrData&          adatOld
                            , const tCIDLib::TCard8     )
{
    // Use the special type to go find the index of the original
    tCIDLib::TCard4 c4At;
    tCIDLib::TBoolean bRes = m_pcavlToUse->bNameExists(adatNew.strSpecType(), c4At);

    if (bRes)
    {
        if (adatNew.strId() == L"Name")
            m_pcavlToUse->SetNameAt(c4At, adatNew.strValue());
        else if (adatNew.strId() == L"Note")
            m_pcavlToUse->SetNoteAt(c4At, adatNew.strValue());
    }
     else
    {
        TErrBox msgbErr(strWndText(), L"The original variable couldn't be found");
    }
}


// ---------------------------------------------------------------------------
//  TMngVarsDlg: Protected, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TMngVarsDlg::bRun(  const   TWindow&            wndOwner
                    ,       TCQCActVarList&     cavlToUse
                    ,       TString&            strSelected
                    ,       tCIDLib::TBoolean&  bByValue
                    , const tCIDLib::TBoolean   bNameOnly)
{
    // Store incoming info we need for later
    m_bNameOnly = bNameOnly;
    m_strVarName = strSelected;
    m_pcavlToUse  = &cavlToUse;

    // And run the dialog
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_MngVars
    );

    if ((c4Res == kCQCIGKit::ridDlg_MngVars_ByName)
    ||  (c4Res == kCQCIGKit::ridDlg_MngVars_ByValue)
    ||  (c4Res == kCIDCtrls::widOK))
    {
        strSelected = tCIDLib::ForceMove(m_strVarName);
        bByValue = (c4Res == kCQCIGKit::ridDlg_MngVars_ByValue);
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TMngVarsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean TMngVarsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we use a lot
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_Add, m_pwndAdd);
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_Close, m_pwndClose);
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_Delete, m_pwndDelete);
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_List, m_pwndVarList);
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_ByName, m_pwndByName);
    CastChildWnd(*this, kCQCIGKit::ridDlg_MngVars_ByValue, m_pwndByValue);

    // Set up our list column titles
    tCIDLib::TStrList colCols(3);
    colCols.objAdd(L" # ");
    colCols.objAdd(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_MngVars_Name));
    colCols.objAdd(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_MngVars_Note));
    m_pwndVarList->SetColumns(colCols);

    // Register our handlers
    m_pwndAdd->pnothRegisterHandler(this, &TMngVarsDlg::eClickHandler);
    m_pwndClose->pnothRegisterHandler(this, &TMngVarsDlg::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TMngVarsDlg::eClickHandler);
    m_pwndByName->pnothRegisterHandler(this, &TMngVarsDlg::eClickHandler);

    //
    //  If by name only, delete the by value button, else register it and change the
    //  by name button text to just say Select and give it the group style and default
    //  button style.
    //
    if (m_bNameOnly)
    {
        m_pwndByValue->Destroy();
        delete m_pwndByValue;
        m_pwndByValue = nullptr;

        m_pwndByName->strWndText(facCQCIGKit().strMsg(kIGKitMsgs::midDlg_MngVars_Select));
        m_pwndByName->SetWndStyle(tCIDCtrls::EWndStyles::Group, kCIDLib::True);
        m_pwndByName->SetDefButton();
        m_pwndByName->SetId(kCIDCtrls::widOK);
    }
     else
    {
        m_pwndByValue->pnothRegisterHandler(this, &TMngVarsDlg::eClickHandler);
    }

    // Load up any incoming values into the list
    const tCIDLib::TCard4 c4Count = m_pcavlToUse->c4VarCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colCols[0].SetFormatted(c4Index + 1);
        colCols[1] = m_pcavlToUse->kvalAt(c4Index).strKey();
        colCols[2] = m_pcavlToUse->kvalAt(c4Index).strValue();
        m_pwndVarList->c4AddItem(colCols, 0);
    }

    // If any incoming value, try to select it.
    if (c4Count)
    {
        //
        //  See if it's a valid name or dereferenced name. Set the by value flag
        //  if it's dereferenced and that's allowable. If not a var or we can't
        //  find it in our list, select the 0th.
        //
        TString strErr;
        TString strReal;
        tCIDLib::TBoolean bByValue;
        const tCIDLib::TBoolean bValid = TStdVarsTar::bValidVarName
        (
            m_strVarName, strReal, strErr, bByValue, !m_bNameOnly
        );

        // Try to select what we got. If not, select the 0th
        tCIDLib::TCard4 c4At;
        if (!bValid || !m_pwndVarList->bSelectByText(strReal, c4At))
            m_pwndVarList->SelectByIndex(0);
    }

    //
    //  Before we are made visible, reposition ourself to the previous size
    //  and pos we were at. This will cause a call to AreaChanged which will
    //  adjust the control positions. Only do it if not empty (which means
    //  this is the first run.) If empty, then calculate a center position
    //  for the dialog and put it there.
    //
    tCIDLib::TBoolean bFirstTime = CQCIGKit_MngVarsDlg::areaLastPos.bIsEmpty();
    const TArea areaOrg = areaWndSize();
    if (bFirstTime)
    {
        TArea areaOwner = pwndFindOwner()->areaWnd();
        TArea areaUs = areaWnd();

        // Center our area in the owner's, and move us there
        areaUs.CenterIn(areaOwner);
        SetPos(areaUs.pntOrg());
    }
     else
    {
        SetSizePos(CQCIGKit_MngVarsDlg::areaLastPos, kCIDLib::False);

        // Put back the column sizes on the list box
        m_pwndVarList->SetColWidths(CQCIGKit_MngVarsDlg::fcolColumns);
    }

    // Do an initial resize of our children to adjust to the initial size
    AutoAdjustChildren(areaOrg, areaWndSize());

    // Install us as the auto-in-place editor on the list window
    m_pwndVarList->SetAutoIPE(this);

    //
    //  Some some of the columns if this is the first time through. The comment column
    //  will get the rest since we are auto-last style.
    //
    if (bFirstTime)
    {
        // Set the order number and variable names columns to fit the header
        m_pwndVarList->AutoSizeCol(0, kCIDLib::True);
        m_pwndVarList->AutoSizeCol(1, kCIDLib::True);
    }

    return bRet;
}


tCIDLib::TVoid TMngVarsDlg::Destroyed()
{
    //
    //   Remember our last size/pos and the list box column widths, then pass it on
    //  to our parent.
    //
    CQCIGKit_MngVarsDlg::areaLastPos = areaWnd();
    m_pwndVarList->QueryColWidths(CQCIGKit_MngVarsDlg::fcolColumns);

    TParent::Destroyed();
}


// ---------------------------------------------------------------------------
//  TMngVarsDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handle events from our buttons
tCIDCtrls::EEvResponses TMngVarsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_MngVars_Add)
    {
        //
        //  Throw a new one into the list with a default name and note content.
        //  The user can in-place edit it after the fact. We have to be sure not to
        //  create a dup, so we we loop until we get a unique name
        //
        TString strNewName(L"GVar:NewVar_");
        const tCIDLib::TCard4 c4BaseLen = strNewName.c4Length();
        tCIDLib::TCard4 c4TestInd = 1;
        while (c4TestInd < 128)
        {
            strNewName.CapAt(c4BaseLen);
            strNewName.AppendFormatted(c4TestInd);

            tCIDLib::TCard4 c4At;
            if (!m_pcavlToUse->bNameExists(strNewName, c4At))
                break;

            c4TestInd++;
        }

        if (c4TestInd < 128)
        {
            //
            //  It worked, so add it to both the var list and list window. We will put
            //  it at the end of our own list. So that tells us what the order number
            //  column value should be.
            //
            m_pcavlToUse->AddNew(strNewName, L"");

            tCIDLib::TStrList colCols(3);
            colCols.objAdd(TString::strEmpty());
            colCols[0].SetFormatted(m_pcavlToUse->c4VarCount());
            colCols.objAdd(strNewName);
            colCols.objAdd(TString::strEmpty());
            m_pwndVarList->c4AddItem(colCols, 0, kCIDLib::c4MaxCard, kCIDLib::True);
        }
         else
        {
            TErrBox msgbErr
            (
                strWndText()
                , facCQCIGKit().strMsg(kIGKitErrs::errcVar_CreateNewName)
            );
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_MngVars_Close)
    {
        EndDlg(kCQCIGKit::ridDlg_MngVars_Close);
    }
     else if ((wnotEvent.widSource() == kCQCIGKit::ridDlg_MngVars_ByName)
          ||  (wnotEvent.widSource() == kCQCIGKit::ridDlg_MngVars_ByValue)
          ||  (wnotEvent.widSource() == kCIDCtrls::widOK))
    {
        const tCIDLib::TCard4 c4ListInd = m_pwndVarList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            // Get the two values of the selected one
            TString strName;
            TString strNote;
            m_pwndVarList->QueryColText(c4ListInd, 1, strName);
            m_pwndVarList->QueryColText(c4ListInd, 2, strNote);

            //
            //  See where this guy is in the list. We use a lookup, which should never
            //  fail since we keep the two lists in sync, but just in case. If it's not
            //  at the top of the list, move it there.
            //
            tCIDLib::TCard4 c4Index;
            if (m_pcavlToUse->bNameExists(strName, c4Index))
            {
                if (c4Index)
                    m_pcavlToUse->MoveToTop(c4Index);
            }

            m_strVarName = strName;
            EndDlg(wnotEvent.widSource());
        }
    }
     else if (wnotEvent.widSource() == kCQCIGKit::ridDlg_MngVars_Delete)
    {
        //
        //  Make sure that they want to delete it. If the list is not
        //  already empty.
        //
        const tCIDLib::TCard4 c4ListInd = m_pwndVarList->c4CurItem();
        if (c4ListInd == kCIDLib::c4MaxCard)
            return tCIDCtrls::EEvResponses::Handled;

        TString strName;
        m_pwndVarList->QueryColText(c4ListInd, 1, strName);
        if (!strName.bIsEmpty())
        {
            TYesNoBox msgbDel
            (
                strWndText()
                , facCQCIGKit().strMsg(kIGKitMsgs::midQ_DeleteVariable, strName)
            );

            if (msgbDel.bShowIt(*this))
            {
                tCIDLib::TCard4 c4Index;
                if (m_pcavlToUse->bRemoveName(strName, c4Index))
                {
                    m_pwndVarList->RemoveAt(c4Index);

                    // We have to renumber the first column
                    RenumberItems();
                }
            }
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  If an item is deleted, we have to renumber the first column to get rid of the
//  the gap. So we have to go through our list, find each one, and update it's column
//  number.
//
tCIDLib::TVoid TMngVarsDlg::RenumberItems()
{
    const tCIDLib::TCard4 c4Count = m_pcavlToUse->c4VarCount();
    if (c4Count)
    {
        // Don't redraw until we are done
        TWndPaintJanitor janPaint(m_pwndVarList);

        TString strNum;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TKeyValuePair& kvalCur = m_pcavlToUse->kvalAt(c4Index);

            const tCIDLib::TCard4 c4At = m_pwndVarList->c4FindText
            (
                kvalCur.strKey(), 1, 0, tCIDLib::EMatchTypes::Equals, kCIDLib::False
            );

            if (c4At == kCIDLib::c4MaxCard)
            {
                #if CID_DEBUG_ON
                strNum = L"Could not find variable in the list window. Name=";
                strNum.Append(kvalCur.strKey());
                TErrBox msgbNotFound(strNum);
                msgbNotFound.ShowIt(*this);
                #endif
            }
             else
            {
                strNum.SetFormatted(c4Index + 1);
                m_pwndVarList->SetColText(c4At, 0, strNum);
            }
        }
    }
}
