//
// FILE NAME: MediaRepoMgr_SelMetaMatchDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/04/2011
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
//  This is the header for the MediaRepoMGr_SelMetaMatchDlg.cpp file, which
//  allows the user to select from a list of available title or id matches.
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
#include    "MediaRepoMgr.hpp"
#include    "MediaRepoMgr_SelMetaMatchDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TSelMetaMatchDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TSelMetaMatchDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TSelMetaMatchDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TSelMetaMatchDlg::TSelMetaMatchDlg() :

    m_c4SelIndex(0)
    , m_pwndCancel(nullptr)
    , m_pwndList(nullptr)
    , m_pwndSearchVal(nullptr)
    , m_pwndSelect(nullptr)
    , m_pwndTitleSearch(nullptr)
    , m_pwndUPCSearch(nullptr)
    , m_prmsToUse(nullptr)
{
}

TSelMetaMatchDlg::~TSelMetaMatchDlg()
{
}


// ---------------------------------------------------------------------------
//  TSelMetaMatchDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TSelMetaMatchDlg::bRunDlg(  const   TWindow&                    wndOwner
                            ,       TMyMoviesMetaSrc&           rmsToUse
                            , const TString&                    strVolume
                            , const TKrnlRemMedia::EMediaTypes  eMType
                            ,       TRepoMetaMatch&             rmmSelected)
{
    // Save any incoming initial disc id and the metadata source to use
    m_eMType = eMType;
    m_prmsToUse = &rmsToUse;
    m_strVolume = strVolume;

    // And run the dialog box
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_SelMetaMatch
    );

    if (c4Res == kMediaRepoMgr::ridDlg_SelMetaMatch_Select)
    {
        rmmSelected = m_colList[m_c4SelIndex];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


tCIDLib::TBoolean
TSelMetaMatchDlg::bRunDlg(  const   TWindow&                wndOwner
                            ,       TMyMoviesMetaSrc* const prmsToUse
                            ,       TRepoMetaMatch&         rmmSelected)
{
    // No incoming search info in this case, so it's a manual search
    m_eMType = TKrnlRemMedia::EMediaTypes::None;
    m_prmsToUse = 0;
    m_strVolume.Clear();

    // And run the dialog box
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_SelMetaMatch
    );

    if (c4Res == kMediaRepoMgr::ridDlg_SelMetaMatch_Select)
    {
        rmmSelected = m_colList[m_c4SelIndex];
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TSelMetaMatchDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TSelMetaMatchDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_List, m_pwndList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_SearchVal, m_pwndSearchVal);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_Select, m_pwndSelect);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_TitleSearch, m_pwndTitleSearch);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_SelMetaMatch_UPCSearch, m_pwndUPCSearch);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TSelMetaMatchDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TSelMetaMatchDlg::eLBHandler);
    m_pwndSelect->pnothRegisterHandler(this, &TSelMetaMatchDlg::eClickHandler);
    m_pwndTitleSearch->pnothRegisterHandler(this, &TSelMetaMatchDlg::eClickHandler);
    m_pwndUPCSearch->pnothRegisterHandler(this, &TSelMetaMatchDlg::eClickHandler);

    // Set the columns for the list
    tCIDLib::TStrList colCols(4);
    colCols.objAdd(L"Title");
    colCols.objAdd(L"Barcode");
    colCols.objAdd(L"Type");
    colCols.objAdd(L"Year");
    m_pwndList->SetColumns(colCols);

    // Load up our initial list of items if we got a volume to start with
    if (!m_strVolume.bIsEmpty())
    {
        TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
        TString strFailReason;
        if (m_prmsToUse->bDiscIdSearch(m_strVolume, m_colList, m_eMType, strFailReason))
        {
            LoadList();
        }
         else
        {
            TErrBox msgbFailed(L"Disc Not Found", strFailReason);
            msgbFailed.ShowIt(*this);
        }
    }
    return bRes;
}


// ---------------------------------------------------------------------------
//  TSelMetaMatchDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  If the user asks to do a new search, this is called. We get the info they
//  entered and try a new search. If it works, we
//
tCIDLib::TVoid
TSelMetaMatchDlg::DoSearch(const tCIDLib::TBoolean bTitleType)
{
    // Get the value entered in the search field. If empty, we can't do anything
    const TString& strVal = m_pwndSearchVal->strWndText();
    if (strVal.bIsEmpty())
    {
        TErrBox msgbEmpty(L"The search value is empty");
        msgbEmpty.ShowIt(*this);
        m_pwndSearchVal->TakeFocus();
        return;
    }

    // Do a search of the indicated type
    TWndPtrJanitor janPtr(tCIDCtrls::ESysPtrs::Wait);
    TString strFailReason;
    tCIDLib::TBoolean bRes;
    if (bTitleType)
        bRes = m_prmsToUse->bTitleSearch(strVal, m_colList, strFailReason);
    else
        bRes = m_prmsToUse->bBarcodeSearch(strVal, m_colList, strFailReason);

    if (bRes)
    {
        // It worked so load up the new stuff
        LoadList();
    }
     else
    {
        // It failed so clear the list and indicate the failure
        m_pwndList->RemoveAll();
        TErrBox msgbFailed(L"Search Failed", strFailReason);
        msgbFailed.ShowIt(*this);
    }
}


// Handle button events
tCIDCtrls::EEvResponses TSelMetaMatchDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelMetaMatch_Cancel)
        EndDlg(kMediaRepoMgr::ridDlg_SelMetaMatch_Cancel);
    else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelMetaMatch_Select)
        EndDlg(kMediaRepoMgr::ridDlg_SelMetaMatch_Select);
    else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelMetaMatch_TitleSearch)
        DoSearch(kCIDLib::True);
    else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelMetaMatch_UPCSearch)
        DoSearch(kCIDLib::False);

    return tCIDCtrls::EEvResponses::Handled;
}


// Handle list box events
tCIDCtrls::EEvResponses TSelMetaMatchDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_SelMetaMatch_List)
    {
        // End the dialog as though the select button was pressed
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
            EndDlg(kMediaRepoMgr::ridDlg_SelMetaMatch_List);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Called to load up the list of items
tCIDLib::TVoid TSelMetaMatchDlg::LoadList()
{
    m_pwndList->RemoveAll();

    const tCIDLib::TCard4 c4Count = m_colList.c4ElemCount();
    if (!c4Count)
    {
        // Put the focus back to the search field and disable select
        m_pwndSearchVal->TakeFocus();
        m_pwndSelect->SetEnable(kCIDLib::False);
        return;
    }

    tCIDLib::TStrList colCols(4);
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());
    colCols.objAdd(TString::strEmpty());

    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        const TRepoMetaMatch& rmmCur = m_colList[c4Index];
        colCols[0] = rmmCur.strName();
        colCols[1] = rmmCur.strBarcode();
        colCols[2] = rmmCur.strMediaType();
        colCols[3].SetFormatted(rmmCur.c4Year());

        // Remember the original index as the row id since we allow sorting
        m_pwndList->c4AddItem(colCols, c4Index);
    }

    // Select the first one
    m_pwndList->SelectByIndex(0);

    // Put the focus on the list and enable the select button
    m_pwndList->TakeFocus();
    m_pwndSelect->SetEnable(kCIDLib::True);
}

