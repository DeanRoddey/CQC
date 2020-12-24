//
// FILE NAME: CQCTreeBrws_ChangeDrvSrvDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/02/2016
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
//  This file implements a dialog that let's the user select a new host to replace the
//  host for a set of drivers currently on another host. We don't do the actual work, we
//  just let the user select a new one.
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
#include    "CQCTreeBrws_.hpp"
#include    "CQCTreeBrws_ChangeDrvSrvDlg_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TChangeDrvSrvDlg,TDlgBox)


// ---------------------------------------------------------------------------
//   CLASS: TChangeDrvSrvDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TChangeDrvSrvDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TChangeDrvSrvDlg::TChangeDrvSrvDlg() :

    m_pcolHosts(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndChange(nullptr)
    , m_pwndList(nullptr)
{
}

TChangeDrvSrvDlg::~TChangeDrvSrvDlg()
{
}


// ---------------------------------------------------------------------------
//  TChangeDrvSrvDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TChangeDrvSrvDlg::bRun( const   TWindow&            wndOwner
                        , const tCIDLib::TStrList&  colList
                        , const TString&            strOrgHost
                        ,       TString&            strSelHost)
{
    // Store some params for later use
    m_pcolHosts = &colList;
    m_strOrgHost = strOrgHost;

    //
    //  We got it, so try to run the dialog. If successful and we get a
    //  positive response, then fill in the caller's parameter with the
    //  info on the selected device.
    //
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCTreeBrws(), kCQCTreeBrws::ridDlg_NewDrvHost
    );

    if (c4Res == kCQCTreeBrws::ridDlg_NewDrvHost_Change)
    {
        strSelHost = m_strSelected;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TChangeDrvSrvDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TChangeDrvSrvDlg::bCreated()
{
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_NewDrvHost_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_NewDrvHost_Change, m_pwndChange);
    CastChildWnd(*this, kCQCTreeBrws::ridDlg_NewDrvHost_List, m_pwndList);

    // Set the columns
    tCIDLib::TStrList colCols(1);
    colCols.objAdd(L"Available Hosts");
    m_pwndList->SetColumns(colCols);

    const tCIDLib::TCard4 c4Count = m_pcolHosts->c4ElemCount();
    for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
    {
        colCols[0] = m_pcolHosts->objAt(c4Index);

        // Set set the row id to the original index, but currently don't need to use it
        m_pwndList->c4AddItem(colCols, c4Index);
    }
    m_pwndList->SelectByIndex(0);

    // And register our event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TChangeDrvSrvDlg::eClickHandler);
    m_pwndChange->pnothRegisterHandler(this, &TChangeDrvSrvDlg::eClickHandler);
    m_pwndList->pnothRegisterHandler(this, &TChangeDrvSrvDlg::eLBHandler);

    // Get the instruction text and update it with the source host name
    TString strInstruct = facCQCTreeBrws().strMsg
    (
        kTBrwsMsgs::midDlg_NewDrvHost_Instruct, m_strOrgHost
    );
    pwndChildById(kCQCTreeBrws::ridDlg_NewDrvHost_Instruct)->strWndText(strInstruct);

    return bRes;
}

// ---------------------------------------------------------------------------
//  TChangeDrvSrvDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

// Handles button clicks
tCIDCtrls::EEvResponses TChangeDrvSrvDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    // Just exit with a one or zero, according to the button
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_NewDrvHost_Change)
    {
        const tCIDLib::TCard4 c4SelIndex = m_pwndList->c4CurItem();
        if (c4SelIndex != kCIDLib::c4MaxCard)
        {
            m_pwndList->QueryColText(c4SelIndex, 0, m_strSelected);
            EndDlg(kCQCTreeBrws::ridDlg_NewDrvHost_Change);
        }
    }
     else if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_NewDrvHost_Cancel)
    {
        EndDlg(kCQCTreeBrws::ridDlg_NewDrvHost_Cancel);
    }
    return tCIDCtrls::EEvResponses::Handled;
}


// Handles list box events
tCIDCtrls::EEvResponses TChangeDrvSrvDlg::eLBHandler(TListChangeInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCTreeBrws::ridDlg_NewDrvHost_List)
    {
        if (wnotEvent.eEvent() == tCIDCtrls::EListEvents::Invoked)
        {
            m_pwndList->QueryColText(wnotEvent.c4Index(), 0, m_strSelected);
            EndDlg(kCQCTreeBrws::ridDlg_NewDrvHost_Change);
        }
    }
    return tCIDCtrls::EEvResponses::Handled;
}
