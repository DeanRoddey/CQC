//
// FILE NAME: CQCGKit_GetCmdDelOpt.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/23/2006
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
//  This file implements a dialog that asks the user how to deal with the
//  deletion of a conditional command in an action.
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
#include    "CQCIGKit_GetCmdDelOpt_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TGetCmdDelOptDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TGetCmdDelOptDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TGetCmdDelOptDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TGetCmdDelOptDlg::TGetCmdDelOptDlg() :

    m_eRet(tCQCIGKit::ECmdDelOpts::Cancel)
    , m_pwndCancel(nullptr)
    , m_pwndDelete(nullptr)
{
}

TGetCmdDelOptDlg::~TGetCmdDelOptDlg()
{
}


// ---------------------------------------------------------------------------
//  TGetCmdDelOptDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCQCIGKit::ECmdDelOpts TGetCmdDelOptDlg::eRunDlg(const TWindow& wndOwner)
{
    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facCQCIGKit(), kCQCIGKit::ridDlg_CmdDel
    );

    // If they selected delete, then return the selected delete mode
    if (c4Res == kCQCIGKit::ridDlg_CmdDel_Delete)
        return m_eRet;

    // Else return cancel
    return tCQCIGKit::ECmdDelOpts::Cancel;
}


// ---------------------------------------------------------------------------
//  TGetCmdDelOptDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TGetCmdDelOptDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // First get typed pointers to the widget we always keep
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdDel_Cancel, m_pwndCancel);
    CastChildWnd(*this, kCQCIGKit::ridDlg_CmdDel_Delete, m_pwndDelete);

    // Register our handlers
    m_pwndCancel->pnothRegisterHandler(this, &TGetCmdDelOptDlg::eClickHandler);
    m_pwndDelete->pnothRegisterHandler(this, &TGetCmdDelOptDlg::eClickHandler);

    // Get one of the radio buttons initially checked
    pwndChildAs<TRadioButton>(kCQCIGKit::ridDlg_CmdDel_Opt1)->SetCheckedState(kCIDLib::True);

    return bRes;
}


// ---------------------------------------------------------------------------
//  TGetCmdDelOptDlg: Private, non-virutal methods
// ---------------------------------------------------------------------------
tCIDCtrls::EEvResponses
TGetCmdDelOptDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kCQCIGKit::ridDlg_CmdDel_Delete)
    {
        // The said they want to delete, so store the mode selected
        if (pwndChildAs<TRadioButton>(kCQCIGKit::ridDlg_CmdDel_Opt1)->bCheckedState())
            m_eRet = tCQCIGKit::ECmdDelOpts::DelAll;
        else if (pwndChildAs<TRadioButton>(kCQCIGKit::ridDlg_CmdDel_Opt2)->bCheckedState())
            m_eRet = tCQCIGKit::ECmdDelOpts::KeepCmds;

    }

    EndDlg(wnotEvent.widSource());
    return tCIDCtrls::EEvResponses::Handled;
}

