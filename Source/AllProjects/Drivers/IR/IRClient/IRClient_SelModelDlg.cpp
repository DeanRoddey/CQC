//
// FILE NAME: IRClient_SelModelDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 08/22/2002
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
//  This file implements the dialog that allows the user to select an IR
//  device from a list of available devices.
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
#include    "IRClient.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSelModelDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TSelModelDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSelModelDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TSelModelDlg::TSelModelDlg() :

    m_pwndByCat(nullptr)
    , m_pwndByMake(nullptr)
    , m_pwndCancelButton(nullptr)
    , m_pwndCategory(nullptr)
    , m_pwndCatFilter(nullptr)
    , m_pwndDelButton(nullptr)
    , m_pwndDescr(nullptr)
    , m_pwndList(nullptr)
    , m_pwndMake(nullptr)
    , m_pwndMakeFilter(nullptr)
    , m_pwndSelectButton(nullptr)
{
}

TSelModelDlg::~TSelModelDlg()
{
}


// ---------------------------------------------------------------------------
//  TSelModelDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSelModelDlg::bRunDlg(  const   TWindow&    wndOwner
                        ,       TString&    strToFill
                        , const TString&    strDevType)
{
    // Store the device type away for later use
    m_strDevType = strDevType;

    //
    //  Get a client proxy for the IR repository server and get the
    //  list of devices for which IR protocols exist.
    //
    tCQCIR::TRepoSrvProxy orbcRepo = facCQCIR().orbcRepoSrvProxy();
    TVector<TIRBlasterDevModelInfo> colModels;
    orbcRepo->c4QueryDevModelList(m_strDevType, m_colList);

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facIRClient(), kIRClient::ridDlg_SelModel
    );

    // If accepted, then copy back to the caller's buffer
    if (c4Res == kIRClient::ridDlg_SelModel_Select)
    {
        strToFill = m_strSelected;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TSelModelDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSelModelDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_ByCategory, m_pwndByCat);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_ByMake, m_pwndByMake);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Cancel, m_pwndCancelButton);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Category, m_pwndCategory);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_CatFilter, m_pwndCatFilter);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Delete, m_pwndDelButton);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Descr, m_pwndDescr);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Make, m_pwndMake);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_MakeFilter, m_pwndMakeFilter);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_List, m_pwndList);
    CastChildWnd(*this, kIRClient::ridDlg_SelModel_Select, m_pwndSelectButton);

    // Set the columns on the device model list
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Available Device Models");
    m_pwndList->SetColumns(colCols);

    //
    //  Load up the combo box that is used by filter by makes. We load it
    //  up with a list of all of the unique makes that we have in the list
    //  we got. We use a temporary hash set to keep up with which ones we've
    //  already added.
    //
    tCIDLib::TStrHashSet colUnique(29, TStringKeyOps());
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (c4Count)
    {
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TIRBlasterDevModelInfo& irbdmiCur = m_colList.objAt(c4Index);

            // If this make is in the list already, skip it. Else add it
            if (colUnique.bHasElement(irbdmiCur.strMake()))
                continue;
            colUnique.objAdd(irbdmiCur.strMake());
            m_pwndMakeFilter->c4AddItem(irbdmiCur.strMake());
        }
        m_pwndMakeFilter->SelectByIndex(0);
    }

    // Load the categories list
    tCQCKit::EDevCats eCat;
    for (eCat = tCQCKit::EDevCats::Min; eCat <= tCQCKit::EDevCats::Max; eCat++)
        m_pwndCatFilter->c4AddItem(tCQCKit::strXlatEDevCats(eCat), tCIDLib::c4EnumOrd(eCat));
    m_pwndCatFilter->SelectByIndex(0);

    // And register our handlers
    m_pwndByCat->pnothRegisterHandler(this, &TSelModelDlg::eClickHandler);
    m_pwndByMake->pnothRegisterHandler(this, &TSelModelDlg::eClickHandler);
    m_pwndCancelButton->pnothRegisterHandler(this, &TSelModelDlg::eClickHandler);
    m_pwndDelButton->pnothRegisterHandler(this, &TSelModelDlg::eClickHandler);
    m_pwndSelectButton->pnothRegisterHandler(this, &TSelModelDlg::eClickHandler);
    m_pwndCatFilter->pnothRegisterHandler(this, &TSelModelDlg::eComboHandler);
    m_pwndMakeFilter->pnothRegisterHandler(this, &TSelModelDlg::eComboHandler);
    m_pwndList->pnothRegisterHandler(this, &TSelModelDlg::eLBHandler);

    // Do an initial load of the model list window
    LoadModelList();

    return bRet;
}


// ---------------------------------------------------------------------------
//  TSelModelDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TSelModelDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if ((wnotEvent.widSource() == kIRClient::ridDlg_SelModel_ByCategory)
    ||  (wnotEvent.widSource() == kIRClient::ridDlg_SelModel_ByMake))
    {
        //
        //  Just call the generic model loading function, which will update
        //  the displayed list of models appropriately.
        //
        LoadModelList();
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_SelModel_Cancel)
    {
        EndDlg(kIRClient::ridDlg_SelModel_Cancel);
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_SelModel_Delete)
    {
        // If there is a currently selection, then delete if they agree
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItemId();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            TYesNoBox msgbDelConfirm
            (
                facIRClient().strMsg
                (
                    kIRCMsgs::midQ_DelFromRepo, m_colList[c4Index].strModel()
                )
            );

            if (msgbDelConfirm.bShowIt(*this))
            {
                // Get a repository proxy and delete the model
                tCQCIR::TRepoSrvProxy orbcRepo = facCQCIR().orbcRepoSrvProxy();
                orbcRepo->DeleteDevModel(m_colList.objAt(c4Index).strModel(), m_strDevType);
                m_colList.RemoveAt(c4Index);

                // Now reload to get the list to collection indices back right
                LoadModelList();
            }
        }
    }
     else if (wnotEvent.widSource() == kIRClient::ridDlg_SelModel_Select)
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItemId();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            m_strSelected = m_colList[c4Index].strModel();
            EndDlg(kIRClient::ridDlg_SelModel_Select);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TSelModelDlg::eComboHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        //
        //  Just call the generic model loading function, which will update
        //  the displayed list of models appropriately.
        //
        LoadModelList();
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDCtrls::EEvResponses TSelModelDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::SelChanged)
    {
        //
        //  Update the display info. We stored the index into the orignal
        //  info collection as the id, so we go back to it for the data.
        //
        const TIRBlasterDevModelInfo& irbdmiCur = m_colList[wnotEvent.c4Id()];

        m_pwndDescr->strWndText(irbdmiCur.strDescription());
        m_pwndCategory->strWndText(tCQCKit::strXlatEDevCats(irbdmiCur.eCategory()));
        m_pwndMake->strWndText(irbdmiCur.strMake());
    }
     else if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
    {
        m_strSelected = m_colList[wnotEvent.c4Id()].strModel();
        EndDlg(kIRClient::ridDlg_SelModel_Select);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


tCIDLib::TVoid TSelModelDlg::LoadModelList()
{
    // Clear the list
    m_pwndList->RemoveAll();

    // If we have no current items, then we are done
    if (m_colList.bIsEmpty())
        return;

    //
    //  Get the check state of the two filter check boxes. If neither of
    //  them is set, then we do the simple scenario. Else we have to filter
    //  the list before loading it.
    //
    const tCIDLib::TBoolean bByCat  = m_pwndByCat->bCheckedState();
    const tCIDLib::TBoolean bByMake
    (
        m_pwndByMake->bCheckedState() && m_pwndMakeFilter->c4ItemCount()
    );

    // Get the category and make filter values
    const tCQCKit::EDevCats eCatFilter = tCQCKit::EDevCats(m_pwndCatFilter->c4CurItemId());

    TString strMakeFilter;
    m_pwndMakeFilter->ValueAt(m_pwndMakeFilter->c4CurItem(), strMakeFilter);

    // We'll do the filters as we go and just toss ones that don't match.
    tCIDLib::TStrList colVals(1);
    colVals.objAdd(TString::strEmpty());
    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TIRBlasterDevModelInfo& irbdmiCur = m_colList.objAt(c4Index);

        // If its filtered out, then just skip it
        if (bByCat && (irbdmiCur.eCategory() != eCatFilter))
            continue;
        else if (bByMake && (irbdmiCur.strMake() != strMakeFilter))
            continue;

        //
        //  We want to keep this one so create a new item and put it in the list. We store
        //  the index back into the info collection as the id, so that we can refer back
        //  to it.
        //
        colVals[0] = irbdmiCur.strModel();
        m_pwndList->c4AddItem(colVals, c4Index);
    }
}


