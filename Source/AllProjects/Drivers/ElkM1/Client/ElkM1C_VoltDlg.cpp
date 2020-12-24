//
// FILE NAME: ElkM1C_VoltDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/29/2008
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
//  This file implements the voltage info editing dialog.
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
#include    "ElkM1C_.hpp"


// ---------------------------------------------------------------------------
//  Do our RTTI macros
// ---------------------------------------------------------------------------
RTTIDecls(TEdM1VoltDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdM1VoltDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdM1VoltDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdM1VoltDlg::TEdM1VoltDlg() :

    m_c4Index(0)
    , m_pwndAVal(nullptr)
    , m_pwndBVal(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndCVal(nullptr)
    , m_pwndHighLim(nullptr)
    , m_pwndLowLim(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndUsed(nullptr)
{
}

TEdM1VoltDlg::~TEdM1VoltDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1VoltDlg: Public, non-virtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1VoltDlg::bRunDlg(  const   TWindow&        wndOwner
                        , const tCIDLib::TCard4 c4Index
                        ,       TVoltInfo&      viToEdit)
{
    // Save the incoming info that we need for later
    m_c4Index = c4Index;
    m_viToEdit = viToEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1C(), kElkM1C::ridDlg_EdVolt
    );

    if (c4Res == kElkM1C::ridDlg_EdVolt_Save)
    {
        // Copy back the temp data and return success
        viToEdit = m_viToEdit;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdM1VoltDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdM1VoltDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_AVal, m_pwndAVal);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_BVal, m_pwndBVal);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_CVal, m_pwndCVal);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_HighLim, m_pwndHighLim);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_LowLim, m_pwndLowLim);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_Save, m_pwndSave);
    CastChildWnd(*this, kElkM1C::ridDlg_EdVolt_Used, m_pwndUsed);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1VoltDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1VoltDlg::eClickHandler);

    // Get the voltage number into the title text
    TString strText(this->strWndText());
    strText.eReplaceToken(m_c4Index + 1, kCIDLib::chDigit1);
    this->strWndText(strText);

    // Load the incoming values
    m_pwndUsed->SetCheckedState(m_viToEdit.bUsed());

    strText.Clear();
    strText.AppendFormatted(m_viToEdit.f8AVal());
    m_pwndAVal->strWndText(strText);

    strText.Clear();
    strText.AppendFormatted(m_viToEdit.f8BVal());
    m_pwndBVal->strWndText(strText);

    strText.Clear();
    strText.AppendFormatted(m_viToEdit.f8CVal());
    m_pwndCVal->strWndText(strText);

    strText.Clear();
    strText.AppendFormatted(m_viToEdit.f8LowLim());
    m_pwndLowLim->strWndText(strText);

    strText.Clear();
    strText.AppendFormatted(m_viToEdit.f8HighLim());
    m_pwndHighLim->strWndText(strText);

    return bRes;
}


// ---------------------------------------------------------------------------
//  TEdM1VoltDlg: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TBoolean TEdM1VoltDlg::bSaveChanges()
{
    // Get the values out and validate them
    TString strAVal(m_pwndAVal->strWndText());
    TString strBVal(m_pwndBVal->strWndText());
    TString strCVal(m_pwndCVal->strWndText());
    TString strHighLim(m_pwndHighLim->strWndText());
    TString strLowLim(m_pwndLowLim->strWndText());

    TEntryField* pwndBad = nullptr;
    tCIDLib::TFloat8 f8ATmp;
    tCIDLib::TFloat8 f8BTmp;
    tCIDLib::TFloat8 f8CTmp;
    tCIDLib::TFloat8 f8HighLim;
    tCIDLib::TFloat8 f8LowLim;

    if (!strAVal.bToFloat8(f8ATmp))
        pwndBad = m_pwndAVal;
    else if (!strBVal.bToFloat8(f8BTmp))
        pwndBad = m_pwndBVal;
    else if (!strCVal.bToFloat8(f8CTmp))
        pwndBad = m_pwndCVal;
    else if (!strHighLim.bToFloat8(f8HighLim))
        pwndBad = m_pwndHighLim;
    else if (!strLowLim.bToFloat8(f8LowLim))
        pwndBad = m_pwndLowLim;

    if (pwndBad)
    {
        TErrBox msgbErr(facElkM1C().strMsg(kElkM1CErrs::errcCfg_BadVoltVal));
        msgbErr.ShowIt(*this);

        // Put the focus on the bad one
        pwndBad->TakeFocus();
        return kCIDLib::False;
    }

    // It's good so we can end
    m_viToEdit.Set
    (
        m_pwndUsed->bCheckedState(), f8ATmp, f8BTmp, f8CTmp, f8LowLim, f8HighLim
    );
    return kCIDLib::True;
}


// Our button click handler
tCIDCtrls::EEvResponses TEdM1VoltDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1C::ridDlg_EdVolt_Cancel)
    {
        EndDlg(kElkM1C::ridDlg_EdVolt_Cancel);
    }
    else if (wnotEvent.widSource() == kElkM1C::ridDlg_EdVolt_Save)
    {
        if (bSaveChanges())
            EndDlg(kElkM1C::ridDlg_EdVolt_Save);
    }
    return tCIDCtrls::EEvResponses::Handled;
}

