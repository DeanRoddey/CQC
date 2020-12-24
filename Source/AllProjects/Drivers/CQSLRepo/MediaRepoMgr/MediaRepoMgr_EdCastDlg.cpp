//
// FILE NAME: MediaRepoMgr_EdCastDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/23/2006
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
//  This file implements a dialog that allows the user to edit the cast list.
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
#include    "MediaRepoMgr_EdCastDlg.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdCastDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdCastDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdCastDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdCastDlg::TEdCastDlg() :

    m_pwndAdd(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndDelete(nullptr)
    , m_pwndList(nullptr)
    , m_pwndMoveDn(nullptr)
    , m_pwndMoveUp(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdCastDlg::~TEdCastDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdCastDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdCastDlg::bRun(const TWindow& wndOwner, TString& strCast)
{
    // Save away the incoming cast info
    m_strCast = strCast;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdCast
    );

    if (c4Res == kMediaRepoMgr::ridDlg_EdCast_Save)
    {
        strCast = m_strCast;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdCastDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdCastDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_Add, m_pwndAdd);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_Delete, m_pwndDelete);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_List, m_pwndList);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_MoveDn, m_pwndMoveDn);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_MoveUp, m_pwndMoveUp);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdCast_Save, m_pwndSave);

    // Register handlers
    m_pwndAdd->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);
    m_pwndCancel->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TEdCastDlg::eLBHandler);
    m_pwndMoveDn->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);
    m_pwndMoveUp->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdCastDlg::eClickHandler);

    // We don't show the list column titles but we have to set them
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Name");
    m_pwndList->SetColumns(colCols);

    //
    //  Load up the current cast. We need to tokenize the cast list based
    //  on comma separators.
    //
    TStringTokenizer stokCast(&m_strCast, L",");
    TString strName;
    while (stokCast.bGetNextToken(strName))
    {
        strName.StripWhitespace();
        colCols[0] = strName;
        m_pwndList->c4AddItem(colCols, 0);
    }
    return bRet;
}


// ---------------------------------------------------------------------------
//  TEdCastDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses TEdCastDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_Add)
    {
        // Add a new cast member with default name and select it
        tCIDLib::TStrList colCols(1);
        colCols.objAdd(L"new castmember");
        m_pwndList->c4AddItem
        (
            colCols, kCIDLib::c4MaxCard, kCIDLib::True
        );
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_EdCast_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_Delete)
    {
        const tCIDLib::TCard4 c4ListInd = m_pwndList->c4CurItem();
        if (c4ListInd != kCIDLib::c4MaxCard)
        {
            // Make sure they want to really delete it
            TString strName;
            m_pwndList->QueryColText(c4ListInd, 0, strName);
            TString strQ(kMRMgrMsgs::midQ_DeleteCastMember, facMediaRepoMgr, strName);

            TYesNoBox msgbQ(strQ);
            if (msgbQ.bShowIt(*this))
                m_pwndList->RemoveCurrent();
        }
    }
     else if ((wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_MoveDn)
          ||  (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_MoveUp))
    {
        const tCIDLib::TCard4 c4Index = m_pwndList->c4CurItem();
        if (c4Index != kCIDLib::c4MaxCard)
        {
            if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_MoveDn)
                m_pwndList->bMoveDn(kCIDLib::True);
             else
                m_pwndList->bMoveUp(kCIDLib::True);
        }
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdCast_Save)
    {
        // Reformat the cast string member
        m_strCast.Clear();
        TString strCur;
        const tCIDLib::TCard4 c4Count = m_pwndList->c4ItemCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            m_pwndList->QueryColText(c4Index, 0, strCur);
            if (c4Index)
                m_strCast.Append(L", ");
            m_strCast.Append(strCur);
        }
        EndDlg(kMediaRepoMgr::ridDlg_EdCast_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  We watch for column clicked notifications and invoke the in place editor if they
//  clicked on the already selected row.
//
tCIDCtrls::EEvResponses TEdCastDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::ColClicked)
    {
        const tCIDLib::TCard4 c4ListInd = wnotEvent.c4Index();
        if (m_pwndList->c4CurItem() == c4ListInd)
        {
            TAttrData adatEdit;
            adatEdit.Set
            (
                L"Name"
                , L"Name"
                , kCIDMData::strAttrLim_Required
                , tCIDMData::EAttrTypes::String);

            // Set the current value as the starting point to edit from
            TString strCurVal;
            m_pwndList->QueryColText(c4ListInd, 0, strCurVal);
            adatEdit.SetValueFromText(strCurVal);

            // Get the area of the cell we care about
            TArea areaCell;
            m_pwndList->QueryColArea(c4ListInd, 0, areaCell);

            //
            //  Invoke the in place editor. Set the list index on the IPE interface.
            //  Dont care about the return. If they committed and it validated, then
            //  it will end up in the list row where we'll get it out later.
            //
            c4IPERow(c4ListInd);
            TInPlaceEdit ipeVal;
            ipeVal.bProcess(*m_pwndList, areaCell, adatEdit, *this, 0);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
