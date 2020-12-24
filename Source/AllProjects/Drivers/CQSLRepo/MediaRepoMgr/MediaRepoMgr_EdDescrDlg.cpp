//
// FILE NAME: MediaRepoMgr_EdDescrDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 05/04/2006
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
//  This is the header for the MediaRepoMGr_EdDescrDlg.cpp file, which
//  implements a small dialog box that is used to allow the user to select a
//  media repository to manage.
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
#include    "MediaRepoMgr_EdDescrDlg.hpp"



// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TMediaRepoEdDescrDlg,TDlGBox)


// ---------------------------------------------------------------------------
//   CLASS: TMediaRepoEdDescrDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TMediaRepoEdDescrDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TMediaRepoEdDescrDlg::TMediaRepoEdDescrDlg() :

    m_pstrText(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndText(nullptr)
    , m_pwndSave(nullptr)
{
}

TMediaRepoEdDescrDlg::~TMediaRepoEdDescrDlg()
{
}


// ---------------------------------------------------------------------------
//  TMediaRepoEdDescrDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TMediaRepoEdDescrDlg::bRun(const TWindow& wndOwner, TString& strRepoSel)
{
    // Store a pointer to the caller's string for later use
    m_pstrText = &strRepoSel;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facMediaRepoMgr, kMediaRepoMgr::ridDlg_EdDescr
    );

    return (c4Res == kMediaRepoMgr::ridDlg_EdDescr_Save);
}


// ---------------------------------------------------------------------------
//  TMediaRepoEdDescrDlg: Protected inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TMediaRepoEdDescrDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // First get typed pointers to some of the widgets
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdDescr_Cancel, m_pwndCancel);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdDescr_Text, m_pwndText);
    CastChildWnd(*this, kMediaRepoMgr::ridDlg_EdDescr_Save, m_pwndSave);

    // Register handlers
    m_pwndCancel->pnothRegisterHandler(this, &TMediaRepoEdDescrDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TMediaRepoEdDescrDlg::eClickHandler);

    // Load up the incoming text
    if (!m_pstrText->bIsEmpty())
        m_pwndText->strWndText(*m_pstrText);

    return bRet;
}


tCIDCtrls::EEvResponses TMediaRepoEdDescrDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdDescr_Cancel)
    {
        EndDlg(kMediaRepoMgr::ridDlg_EdDescr_Cancel);
    }
     else if (wnotEvent.widSource() == kMediaRepoMgr::ridDlg_EdDescr_Save)
    {
        *m_pstrText = m_pwndText->strWndText();
        EndDlg(kMediaRepoMgr::ridDlg_EdDescr_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


