//
// FILE NAME: CQCAdmin_EditXlatsDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/21/2015
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
//  This file implements a dialog that let's the user edit the content of the Xlats
//  mixin.
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
#include    "CQCIntfEd_EditXlatsDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEditXlatsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEditXlatsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEditXlatsDlg::TEditXlatsDlg() :

    m_c4NextId(1)
    , m_piewOrg(nullptr)
    , m_pwndAddButton(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndList(nullptr)
    , m_pwndRemoveButton(nullptr)
    , m_pwndRemoveAllButton(nullptr)
    , m_pwndSaveButton(nullptr)
{
}

TEditXlatsDlg::~TEditXlatsDlg()
{
}


// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Public, inherited methods
// ---------------------------------------------------------------------------

tCIDLib::TBoolean
TEditXlatsDlg::bIPETestCanEdit( const   tCIDCtrls::TWndId
                                , const tCIDLib::TCard4     c4ColInd
                                ,       TAttrData&          adatFill
                                ,       tCIDLib::TBoolean&  bValueSet)
{
    // Set up the passed attribute data object with the info to edit
    if (c4ColInd == 0)
        adatFill.Set(L"Key", L"Key", tCIDMData::EAttrTypes::String);
    else if (c4ColInd == 1)
        adatFill.Set(L"Value", L"Value", tCIDMData::EAttrTypes::String);
    else
        return kCIDLib::False;

    // Set the current value as the initial contents to edit
    TString strVal;
    m_pwndList->QueryColText(c4IPERow(), c4ColInd, strVal);
    adatFill.SetString(strVal);
    bValueSet = kCIDLib::True;

    return kCIDLib::True;
}


tCIDLib::TBoolean
TEditXlatsDlg::bIPEValidate(const   TString&            strSrc
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

    if (adatTar.strId() == L"Key")
    {
        //
        //  Search the existing items and see if this key already exists. In theory
        //  it shouldn't match itself, since we only get called here if it changes, but
        //  watch for it just in case.
        //
        tCIDLib::TCard4 c4FndAt = m_pwndList->c4FindByText(strNewVal);
        if ((c4FndAt != kCIDLib::c4MaxCard)
        &&  (m_pwndList->c4IndexToId(c4FndAt) != c4IPERow()))
        {
            strErrMsg = L"This key is already taken";
            return kCIDLib::False;
        }
        adatTar.SetString(strNewVal);
    }
     else if (adatTar.strId() == L"Text")
    {
        adatTar.SetString(strNewVal);
    }
    return kCIDLib::True;
}


// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEditXlatsDlg::bRun(const TWindow& wndOwner, TIEXlatsWrap& iewXlats)
{
    // Store incoming stuff for later use
    m_piewOrg = &iewXlats;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfEd(), kCQCIntfEd::ridDlg_EditXlats
    );

    if (c4Res == kCQCIntfEd::ridDlg_EditXlats_Save)
    {
        // Copy back the edited stuff
        iewXlats = m_iewEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEditXlatsDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_Add, m_pwndAddButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_List, m_pwndList);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_Remove, m_pwndRemoveButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_RemoveAll, m_pwndRemoveAllButton);
    CastChildWnd(*this, kCQCIntfEd::ridDlg_EditXlats_Save, m_pwndSaveButton);


    // Register our handlers
    m_pwndAddButton->pnothRegisterHandler(this, &TEditXlatsDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TEditXlatsDlg::eClickHandler);
    m_pwndRemoveButton->pnothRegisterHandler(this, &TEditXlatsDlg::eClickHandler);
    m_pwndRemoveAllButton->pnothRegisterHandler(this, &TEditXlatsDlg::eClickHandler);
    m_pwndSaveButton->pnothRegisterHandler(this, &TEditXlatsDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEditXlatsDlg::eListHandler);

    // Set up the column titltes on the list box
    tCIDLib::TStrList colVals(2);
    colVals.objAdd(L"Key");
    colVals.objAdd(L"Value");
    m_pwndList->SetColumns(colVals);

    // Load up any incoming values
    TColCursor<TKeyValuePair>* pcursXlats = m_piewOrg->pcursMakeNew();
    TJanitor<TColCursor<TKeyValuePair> > janCurs(pcursXlats);

    tCIDLib::TCard4 c4Count = 0;
    if (pcursXlats->bReset())
    {
        do
        {
            const TKeyValuePair& kvalCur = pcursXlats->objRCur();
            colVals[0] = kvalCur.strKey();
            colVals[1] = kvalCur.strValue();
            m_pwndList->c4AddItem(colVals, m_c4NextId++);
        }   while (pcursXlats->bNext());
    }

    // If any incoming, then select the first one
    if (c4Count)
        m_pwndList->SelectByIndex(0, kCIDLib::True);

    // Install us as the auto-in place editor on the list window
    m_pwndList->SetAutoIPE(this);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TEditXlatsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEditXlatsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_Add)
    {
        tCIDLib::TStrList colVals(2);
        TString strKey(L"NewKey");
        strKey.AppendFormatted(m_c4NextId);
        colVals.objAdd(strKey);
        colVals.objAdd(L"Default Value");

        const tCIDLib::TCard4 c4At = m_pwndList->c4AddItem(colVals, m_c4NextId++);

        // Select this new one
        m_pwndList->SelectByIndex(c4At);
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_Cancel)
    {
        // Gather up the changes and see if there are any changes
        TIEXlatsWrap iewTmp;
        TEditXlatsDlg::GatherInfo(iewTmp);
        if (!iewTmp.bSameXlats(*m_piewOrg))
        {
            TYesNoBox msgbDiscard(strWndText(), facCQCIntfEd().strMsg(kIEdMsgs::midQ_Discard));
            if (msgbDiscard.bShowIt(*this))
                EndDlg(kCQCIntfEd::ridDlg_EditXlats_Cancel);
        }
         else
        {
            EndDlg(kCQCIntfEd::ridDlg_EditXlats_Cancel);
        }
    }
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_Remove)
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
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_RemoveAll)
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
     else if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_Save)
    {
        // Grab all the data into our target object and exit with a save
        GatherInfo(m_iewEdit);
        EndDlg(kCQCIntfEd::ridDlg_EditXlats_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TEditXlatsDlg::eListHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        if (wnotEvent.widSource() == kCQCIntfEd::ridDlg_EditXlats_List)
        {
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Gets our edits out of the list window and into the caller's xlat interface object
tCIDLib::TVoid TEditXlatsDlg::GatherInfo(MCQCIntfXlatIntf& iewTar)
{
    iewTar.Reset();
    TString strKey;
    TString strVal;
    const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        m_pwndList->QueryColText(c4Index, 0, strKey);
        m_pwndList->QueryColText(c4Index, 1, strVal);
        iewTar.AddXlatItem(strKey, strVal);
    }
}
