//
// FILE NAME: VarDriverC_EdFldDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/06/2003
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
//  This file implements the configuration editing dialog.
//
//
// CAVEATS/GOTCHAS:
//
// LOG:
//


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------
#include    "VarDriverC.hpp"


// ---------------------------------------------------------------------------
//  Magic macro
// ---------------------------------------------------------------------------
RTTIDecls(TEdFldListDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdFldListDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdFldListDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdFldListDlg::TEdFldListDlg() :

    m_c4NextId(1)
    , m_pcfgOrg(nullptr)
    , m_pwndAddButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndDataType(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndFldList(nullptr)
    , m_pwndLimits(nullptr)
    , m_pwndPersist(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TEdFldListDlg::~TEdFldListDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdFldListDlg: Public, inherited methods
// ---------------------------------------------------------------------------

// We can always edit, we just have to override this to get the attribute set up
tCIDLib::TBoolean
TEdFldListDlg::bIPETestCanEdit( const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    //
    //  Get the current name to pass in as the initial value. We can just get it from
    //  the list window so we don't have to bother finding the original field def.
    //
    TString strVal;
    m_pwndFldList->QueryColText(c4IPERow(), 0, strVal);

    // Set up the attribute, indicate it's a required value so can't be empty
    adatFill.Set
    (
        L"FldName"
        , L"FldName"
        , kCIDMData::strAttrLim_Required
        , tCIDMData::EAttrTypes::String
    );
    adatFill.SetString(strVal);
    bValueSet = kCIDLib::True;

    return kCIDLib::True;
}


//
//  We just need to make sure they haven't created a duplicate field name.
//
tCIDLib::TBoolean
TEdFldListDlg::bIPEValidate(const   TString&            strSrc
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

    //
    //  There's only one attribute so no need to check the id, just get the field def
    //  index for the currently selected field (has to be the selected one since we are
    //  editing it.)
    //
    const tCIDLib::TCard4 c4CurIndex = c4ListToFldIndex(c4IPERow());

    //
    //  Loop through the current values, skipping the current index, and see if we
    //  get a match.
    //
    const tCIDLib::TCard4 c4Count = m_cfgEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        if (c4Index == c4CurIndex)
            continue;

        if (m_cfgEdit.flddAt(c4Index).strName() == strNewVal)
        {
            strErrMsg = L"This name is already in use by another field.";
            return kCIDLib::False;
        }
    }

    //
    //  Looks ok, so store it. We use the generic text based value setter here
    //  to avoid a lot of nasty typing.
    //
    adatTar.SetValueFromText(strNewVal);
    return kCIDLib::True;
}


tCIDLib::TVoid
TEdFldListDlg::IPEValChanged(const  tCIDCtrls::TWndId
                            , const TString&
                            , const TAttrData&          adatNew
                            , const TAttrData&
                            , const tCIDLib::TCard8     )
{
    // We just need to store the new value away
    const tCIDLib::TCard4 c4CurIndex = c4ListToFldIndex(c4IPERow());
    m_cfgEdit.flddAt(c4CurIndex).strName(adatNew.strValue());
}


// ---------------------------------------------------------------------------
//  TEdFldListDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdFldListDlg::bRun(const TWindow& wndParent, TVarDrvCfg& cfgEdit)
{
    //
    //  Make a copy of the config that we can modify, and save a pointer to
    //  the orignal for comparison purposes.
    //
    m_cfgEdit = cfgEdit;
    m_pcfgOrg = &cfgEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndParent, facVarDriverC(), kVarDriverC::ridDlg_EdFlds
    );

    if (c4Res == kVarDriverC::ridDlg_EdFlds_Save)
    {
        // Copy our working copy back to the caller's copy
        cfgEdit = m_cfgEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdFldListDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdFldListDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get some typed pointers to some of our widgets
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Add, m_pwndAddButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_DataType, m_pwndDataType);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Delete, m_pwndDelButton);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_List, m_pwndFldList);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Limits, m_pwndLimits);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Persist, m_pwndPersist);
    CastChildWnd(*this, kVarDriverC::ridDlg_EdFlds_Save, m_pwndSaveButton);

    //
    //  Load up the the data type combo box. We only support some of them,
    //  the simple data types ones. Since the ones we take are the first
    //  in the enum, it stays simple since we can directly convert back
    //  from the selected index to the enum.
    //
    tCIDLib::ForEachE<tCQCKit::EFldTypes>
    (
        [this](const auto eType)
        {
            m_pwndDataType->c4AddItem(tCQCKit::strXlatEFldTypes(eType));
            return kCIDLib::True;
        }
    );

    // Initially select the 0th one
    m_pwndDataType->SelectByIndex(0);

    //
    //  Do an initial load of any incoming fields that are already in the config data.
    //  We let it sort so we assign a pseudo field id to the fields and store that as
    //  the row id.
    //
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(TString::strEmpty());

    // Set the column titles
    colCols[0] = L"Defined Fields";
    m_pwndFldList->SetColumns(colCols);

    const tCIDLib::TCard4 c4Count = m_cfgEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4Index);
        flddCur.c4Id(m_c4NextId++);
        colCols[0] = flddCur.strName();
        m_pwndFldList->c4AddItem(colCols, flddCur.c4Id());
    }

    // Register our child control handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TEdFldListDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TEdFldListDlg::eClickHandler);
    m_pwndDataType->pnothRegisterHandler(this, &TEdFldListDlg::eComboHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TEdFldListDlg::eClickHandler);
    m_pwndFldList->pnothRegisterHandler(this, &TEdFldListDlg::eLBHandler);
    m_pwndLimits->pnothRegisterHandler(this, &TEdFldListDlg::eMLEHandler);
    m_pwndPersist->pnothRegisterHandler(this, &TEdFldListDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEdFldListDlg::eClickHandler);

    // Set us as the IPE handler for the field list
    m_pwndFldList->SetAutoIPE(this);

    //
    //  Initially select the 0th one if we have any, and force an update so that it's info
    //  gets loaded.
    //
    if (c4Count)
        m_pwndFldList->SelectByIndex(0, kCIDLib::True);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdFldListDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  Validates all of the data to make sure we are storing valid field data. The only thing
//  that can be wrong is the limit value. So we look through and for each one that has a
//  limit, we try to parse the limits. If it fails, we tell the user and put the selection
//  on that item.
//
tCIDLib::TBoolean TEdFldListDlg::bValidate()
{
    TString strTmp;

    const tCIDLib::TCard4 c4Count = m_cfgEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4Index);

        //
        //  Get the text out to a temp string, and strip out any new lines or
        //  tabs, which are put in by us for display and can be put in by the
        //  user when typing, then put it back in.
        //
        strTmp = flddCur.strLimits();
        strTmp.Strip(L"\r\n\t", tCIDLib::EStripModes::Full);
        flddCur.strLimits(strTmp);

        //
        //  Try to compile the field limits string. The limits class has
        //  a method to do this for us. If we get back and object and do
        //  not throw, it was good. If there is no limit string, it will
        //  give us a default limit object for that type that has no limits.
        //
        TCQCFldLimit* pfldlCur = 0;
        try
        {
            pfldlCur = TCQCFldLimit::pfldlMakeNew(flddCur);
            TJanitor<TCQCFldLimit> janLimit(pfldlCur);

            //
            //  If we have a current default value, see if it is legal for
            //  the current limits. If not, that's an error, so put it back
            //  to a valid default.
            //
            //  If there is no current default, then set it to something
            //  valid for the field type.
            //
            if (flddCur.strExtraCfg().bIsEmpty()
            ||  !pfldlCur->bValidate(flddCur.strExtraCfg()))
            {
                TString strDefVal;
                pfldlCur->QueryDefVal(strDefVal);
                flddCur.strExtraCfg(strDefVal);
            }
        }

        catch(TError& errToCatch)
        {
            if (facVarDriverC().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            TErrBox msgbErr
            (
                facVarDriverC().strMsg(kVarCMsgs::midStatus_BadLimits, flddCur.strName())
            );
            msgbErr.ShowIt(*this);

            //
            //  Select this one and return failure. We have to map backwards from the
            //  pseudo field id to the list index.
            //
            if (pfldlCur)
                m_pwndFldList->SelectByIndex(m_pwndFldList->c4IdToIndex(flddCur.c4Id()));
            return kCIDLib::False;
        }
    }
    return kCIDLib::True;
}


//
//  Returns the index, within the field config, of the field currently selected in the
//  list window, or at the passed index.
//
tCIDLib::TCard4 TEdFldListDlg::c4FindCurFieldIndex() const
{
    // Get the row id for the selected item. If none, then return max card
    const tCIDLib::TCard4 c4RowId = m_pwndFldList->c4CurItemId();
    if (c4RowId == kCIDLib::c4MaxCard)
        return c4RowId;

    const tCIDLib::TCard4 c4Count = m_cfgEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4Index);
        if (flddCur.c4Id() == c4RowId)
            return c4Index;
    }

    CIDAssert2(L"Current field's psuedo id not found");
    return kCIDLib::c4MaxCard;
}


tCIDLib::TCard4 TEdFldListDlg::c4ListToFldIndex(const tCIDLib::TCard4 c4ListInd) const
{
    const tCIDLib::TCard4 c4RowId = m_pwndFldList->c4IndexToId(c4ListInd);
    if (c4RowId == kCIDLib::c4MaxCard)
        return c4RowId;

    const tCIDLib::TCard4 c4Count = m_cfgEdit.c4Count();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4Index);
        if (flddCur.c4Id() == c4RowId)
            return c4Index;
    }

    CIDAssert2(L"Field's psuedo id not found");
    return kCIDLib::c4MaxCard;
}


// Handle clicks on our buttons
tCIDCtrls::EEvResponses TEdFldListDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kVarDriverC::ridDlg_EdFlds_Add)
    {
        // Find a unique default name to give it. They can rename it in place
        TString strNewName(L"NewField");
        const tCIDLib::TCard4 c4BaseLen = strNewName.c4Length();
        tCIDLib::TCard4 c4Index = 1;
        while (c4Index < 512)
        {
            strNewName.CapAt(c4BaseLen);
            strNewName.AppendFormatted(c4Index);

            // If this one doesn't exist, then we can go with this
            if (!m_cfgEdit.bFieldExists(strNewName))
                break;

            c4Index++;
        }


        if (c4Index == 512)
        {
            TErrBox msgbNoName(L"Could not find an unused new default field name");
            msgbNoName.ShowIt(*this);
            return tCIDCtrls::EEvResponses::Handled;
        }

        // Set up a field with default info
        TCQCFldDef flddNew
        (
            strNewName
            , tCQCKit::EFldTypes::Boolean
            , tCQCKit::EFldAccess::ReadWrite
            , TString::strEmpty()
        );

        // Give it the next pseudo field id
        flddNew.c4Id(m_c4NextId++);

        // Add it to our configuration data
        m_cfgEdit.c4AddField(flddNew);

        // And to our list window and select it
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(strNewName);
        const tCIDLib::TCard4 c4At = m_pwndFldList->c4AddItem
        (
            colCols, flddNew.c4Id(), kCIDLib::c4MaxCard, kCIDLib::True
        );

        // Now force an in-place edit on the field so they can give it a new name
        m_pwndFldList->EditCell(c4At, 0);

        // Resort since the default name won't probably sort the same place as the real one
        m_pwndFldList->Resort();

        // Get the new guy back visible and selected
        const tCIDLib::TCard4 c4NewInd = m_pwndFldList->c4IdToIndex(flddNew.c4Id());
        m_pwndFldList->MakeItemVisible(c4NewInd, kCIDLib::True);

        // And reselect it
        m_pwndFldList->SelectByIndex(c4NewInd);
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_EdFlds_Delete)
    {
        // If the list isn't empty, then delete the current one
        const tCIDLib::TCard4 c4ListIndex = m_pwndFldList->c4CurItem();
        if (c4ListIndex != kCIDLib::c4MaxCard)
        {
            const tCIDLib::TCard4 c4FldIndex = c4ListToFldIndex(c4ListIndex);

            //
            //  Do the config data first since the removal of the list box
            //  item will cause an event and it will update against the
            //  config contents.
            //
            m_cfgEdit.RemoveAt(c4FldIndex);
            m_pwndFldList->RemoveAt(c4ListIndex);
        }
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_EdFlds_Cancel)
    {
        // See if there are changes. If so, give them a chance to back out
        if (m_cfgEdit != *m_pcfgOrg)
        {
            TYesNoBox msgbErr(facVarDriverC().strMsg(kVarCMsgs::midQ_LoseChanges));
            if (msgbErr.bShowIt(*this))
                EndDlg(kVarDriverC::ridDlg_EdFlds_Cancel);
        }
         else
        {
            // No changes we can just exit
            EndDlg(kVarDriverC::ridDlg_EdFlds_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_EdFlds_Persist)
    {
        const tCIDLib::TCard4 c4FldIndex = c4FindCurFieldIndex();
        if (c4FldIndex != kCIDLib::c4MaxCard)
        {
            TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4FldIndex);
            flddCur.bPrivate(m_pwndPersist->bCheckedState());
        }
    }
     else if (wnotEvent.widSource() == kVarDriverC::ridDlg_EdFlds_Save)
    {
        //
        //  Validate the data. If good, then exit. Else, the user was told what was
        //  wrong and we stay in.
        //
        if (bValidate())
            EndDlg(kVarDriverC::ridDlg_EdFlds_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdFldListDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    // Update the selected item, if any, with the newly selected data type
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Find the index of the currently selected field
        const tCIDLib::TCard4 c4FldIndex = c4FindCurFieldIndex();
        if (c4FldIndex != kCIDLib::c4MaxCard)
        {
            TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4FldIndex);
            flddCur.eType(tCQCKit::EFldTypes(m_pwndDataType->c4CurItem()));

            //
            //  Clear it's limits and default value, since this will likely
            //  invalidate them.
            //
            flddCur.strLimits(TString::strEmpty());
            flddCur.strExtraCfg(TString::strEmpty());
            m_pwndLimits->ClearText();
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdFldListDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    //
    //  If the list was emptied, then clean out the display values. Else,
    //  update them with the info for the newly selected field.
    //
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        // Enable the modify fields
        m_pwndDataType->SetEnable(kCIDLib::True);
        m_pwndLimits->SetEnable(kCIDLib::True);
        m_pwndPersist->SetEnable(kCIDLib::True);

        // Get the selected field
        const tCIDLib::TCard4 c4FldInd = c4ListToFldIndex(wnotEvent.c4Index());
        const TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4FldInd);

        // And display it's settings
        m_pwndDataType->SelectByIndex(tCIDLib::c4EnumOrd(flddCur.eType()));
        m_pwndPersist->SetCheckedState(flddCur.bPrivate());

        //
        //  We want to treat an enumerated type specially, since it can
        //  be fairly long, So we will put out the values one on a line.
        //
        try
        {
            TCQCFldLimit* pfldlCur = TCQCFldLimit::pfldlMakeNew(flddCur);
            TJanitor<TCQCFldLimit> janLimit(pfldlCur);
            if (pfldlCur->bIsA(TCQCFldEnumLimit::clsThis()))
            {
                //
                //  Use a string tokenizer to break out the tokens of the
                //  limit string, so we can put them out in a nice format.
                //
                TStringTokenizer stokLims(&flddCur.strLimits(), L",");
                TString strText;
                TString strToken;
                while (stokLims.bGetNextToken(strToken))
                {
                    strToken.StripWhitespace();
                    strText.Append(strToken);
                    if (stokLims.bMoreTokens())
                        strText.Append(L",\n");
                }
                m_pwndLimits->strWndText(strText);
            }
             else
            {
                // Nothing special, so just put out the text
                m_pwndLimits->strWndText(flddCur.strLimits());
            }
        }

        catch(TError& errToCatch)
        {
            if (facVarDriverC().bShouldLog(errToCatch))
            {
                errToCatch.AddStackLevel(CID_FILE, CID_LINE);
                TModule::LogEventObj(errToCatch);
            }

            // Just show the text for them to edit
            m_pwndLimits->strWndText(flddCur.strLimits());

            TErrBox msgbErr
            (
                facVarDriverC().strMsg(kVarCMsgs::midStatus_BadLimits, flddCur.strName())
            );
            msgbErr.ShowIt(*this);
        }
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Cleared)
    {
        m_pwndDataType->SelectByIndex(0);
        m_pwndLimits->ClearText();
        m_pwndPersist->SetCheckedState(kCIDLib::False);

        // Disable them now
        m_pwndDataType->SetEnable(kCIDLib::False);
        m_pwndLimits->SetEnable(kCIDLib::False);
        m_pwndPersist->SetEnable(kCIDLib::False);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEdFldListDlg::eMLEHandler(TMLEChangeInfo& wnotEvent)
{
    //
    //  Get the selected field index. If none, then we do nothing. This can
    //  happen when the last item is deleted, which clears the field, but
    //  we get a changed event here.
    //
    const tCIDLib::TCard4 c4FldIndex = c4FindCurFieldIndex();
    if (c4FldIndex != kCIDLib::c4MaxCard)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EMLEEvents::Changed)
        {
            // Get the selected field
            TCQCFldDef& flddCur = m_cfgEdit.flddAt(c4FldIndex);
            flddCur.strLimits(m_pwndLimits->strWndText());
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}

