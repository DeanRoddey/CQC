//
// FILE NAME: CQCIntfView_ShowFlds.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 12/26/2012
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
//  This file implements a dialog that shows the fields currently active
//  in the poll engine.
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
#include    "CQCIntfView.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TShowFldsDlg, TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TShowFldsDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TShowFldsDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TShowFldsDlg::TShowFldsDlg() :

    m_pwndClose(0)
    , m_pwndFields(0)
{
}

TShowFldsDlg::~TShowFldsDlg()
{
    // Call our own destroy method
    Destroy();
}


// ---------------------------------------------------------------------------
//  TShowFldsDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TVoid TShowFldsDlg::RunDlg(const TWindow& wndOwner)
{
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIntfView, kCQCIntfView::ridDlg_ShowFlds
    );
}


// ---------------------------------------------------------------------------
//  TShowFldsDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TShowFldsDlg::bCreated()
{
    // Call our parent first
    tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIntfView::ridDlg_ShowFlds_Close, m_pwndClose);
    CastChildWnd(*this, kCQCIntfView::ridDlg_ShowFlds_Fields, m_pwndFields);

    // Query the list of active fields
    TVector<TString> colFlds;
    facCQCIntfEng().polleThis().QueryFlds(colFlds);

    // Sort it for easier reading
    colFlds.Sort(TString::eCompI);

    tCIDLib::TStrList colLoad;
    const tCIDLib::TCard4 c4Count = colFlds.c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        colLoad.objAdd(colFlds[c4Index]);
    m_pwndFields->LoadList(colLoad);

    // Register our handlers
    m_pwndClose->pnothRegisterHandler(this, &TShowFldsDlg::eClickHandler);

    return bRet;
}


// ---------------------------------------------------------------------------
//  TShowFldsDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TShowFldsDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIntfView::ridDlg_ShowFlds_Close)
        EndDlg(0);
    return tCIDCtrls::EEvResponses::Handled;
}

