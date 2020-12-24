//
// FILE NAME: ElkM1V2C_VoltDlg.cpp
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
#include    "ElkM1V2C_.hpp"


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
    , m_pwndElkId(nullptr)
    , m_pwndHighLim(nullptr)
    , m_pwndLowLim(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdM1VoltDlg::~TEdM1VoltDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1VoltDlg: Public, non-iirtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1VoltDlg::bRunDlg(  const   TWindow&                wndOwner
                        , const tCIDLib::TCard4         c4Index
                        ,       tElkM1V2C::TItemList&   colList
                        ,       TVoltInfo&              iiEdit)
{
    // Save the info for later use
    m_c4Index = c4Index;
    m_iiOrg = iiEdit;
    m_iiEdit = iiEdit;
    m_pcolList = &colList;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1V2C(), kElkM1V2C::ridDlg_EdVolt
    );

    // If they committed, and there are changes, return true and the changes
    if (c4Res == kElkM1V2C::ridDlg_EdVolt_Save)
    {
        if (m_iiOrg != m_iiEdit)
        {
            iiEdit = m_iiEdit;
            return kCIDLib::True;
        }
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
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_AVal, m_pwndAVal);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_BVal, m_pwndBVal);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_CVal, m_pwndCVal);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_ElkId, m_pwndElkId);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_HighLim, m_pwndHighLim);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_LowLim, m_pwndLowLim);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_Name, m_pwndName);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdVolt_Save, m_pwndSave);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1VoltDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1VoltDlg::eClickHandler);

    // Load up the incoming values
    TString strFmt;

    // Set up the instruction text
    TWindow* pwndInstruct = pwndChildById(kElkM1V2C::ridDlg_EdVolt_Instruct);
    strFmt = pwndInstruct->strWndText();
    strFmt.Append(facElkM1V2C().strMsg(kElkM1V2CMsgs::midGen_LimitCommon));
    pwndInstruct->strWndText(strFmt);

    // The Elk id, but only if not max card (adding a new one without an id yet)
    if (m_iiEdit.m_c4ElkId != kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(m_iiEdit.m_c4ElkId);
        m_pwndElkId->strWndText(strFmt);
    }

    m_pwndName->strWndText(m_iiEdit.m_strName);

    strFmt.SetFormatted(m_iiEdit.m_f8AVal);
    m_pwndAVal->strWndText(strFmt);

    strFmt.SetFormatted(m_iiEdit.m_f8BVal);
    m_pwndBVal->strWndText(strFmt);

    strFmt.SetFormatted(m_iiEdit.m_f8CVal);
    m_pwndCVal->strWndText(strFmt);

    strFmt.SetFormatted(m_iiEdit.m_f8LowLim);
    m_pwndLowLim->strWndText(strFmt);

    strFmt.SetFormatted(m_iiEdit.m_f8HighLim);
    m_pwndHighLim->strWndText(strFmt);

    return bRes;
}


// Our button click handler
tCIDCtrls::EEvResponses TEdM1VoltDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdVolt_Cancel)
        EndDlg(kElkM1V2C::ridDlg_EdVolt_Cancel);
    else if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdVolt_Save)
        SaveChanges();

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TVoid TEdM1VoltDlg::SaveChanges()
{
    // Validate the base stuff
    const TString& strName = m_pwndName->strWndText();
    const TString& strId = m_pwndElkId->strWndText();

    tCIDLib::TCard4 c4ElkId;
    const tElkM1V2C::EBaseValRes eRes = TElkM1V2CWnd::eCheckBaseVals
    (
        *this, *m_pcolList, strName, strId, m_c4Index, tElkM1V2C::EItemTypes::Volt, c4ElkId
    );

    if (eRes == tElkM1V2C::EBaseValRes::Name)
    {
        m_pwndName->TakeFocus();
        return;
    }
     else if (eRes == tElkM1V2C::EBaseValRes::Id)
    {
        m_pwndElkId->TakeFocus();
        return;
    }

    // Get the values out and validate them
    TString strAVal(m_pwndAVal->strWndText());
    TString strBVal(m_pwndBVal->strWndText());
    TString strCVal(m_pwndCVal->strWndText());
    TString strHighLim(m_pwndHighLim->strWndText());
    TString strLowLim(m_pwndLowLim->strWndText());

    TEntryField* pwndBad = 0;
    tCIDLib::TFloat8 f8ATmp;
    tCIDLib::TFloat8 f8BTmp;
    tCIDLib::TFloat8 f8CTmp;
    tCIDLib::TFloat8 f8HighLim;
    tCIDLib::TFloat8 f8LowLim;

    tCIDLib::TErrCode errcFail = kElkM1V2CErrs::errcCfg_BadVoltVal;
    if (!strAVal.bToFloat8(f8ATmp))
        pwndBad = m_pwndAVal;
    else if (!strBVal.bToFloat8(f8BTmp))
        pwndBad = m_pwndBVal;
    else if (!strCVal.bToFloat8(f8CTmp))
        pwndBad = m_pwndCVal;

    if (!pwndBad)
    {
        if (!strHighLim.bToFloat8(f8HighLim))
            pwndBad = m_pwndHighLim;
        else if (!strLowLim.bToFloat8(f8LowLim))
            pwndBad = m_pwndLowLim;

        if (pwndBad)
            errcFail = kElkM1V2CErrs::errcEdit_BadLimVal;
    }

    if (!pwndBad)
    {
        if (f8LowLim >= f8HighLim)
        {
            errcFail = kElkM1V2CErrs::errcEdit_LimOrder;
            pwndBad = m_pwndHighLim;
        }
    }

    if (pwndBad)
    {
        TErrBox msgbErr(facElkM1V2C().strMsg(errcFail));
        msgbErr.ShowIt(*this);

        // Put the focus on the bad one
        pwndBad->TakeFocus();
        return;
    }

    // It's good so we can end
    m_iiEdit.m_strName = strName;
    m_iiEdit.m_c4ElkId = c4ElkId;
    m_iiEdit.m_f8AVal = f8ATmp;
    m_iiEdit.m_f8BVal = f8BTmp;
    m_iiEdit.m_f8CVal = f8CTmp;
    m_iiEdit.m_f8LowLim = f8LowLim;
    m_iiEdit.m_f8HighLim = f8HighLim;

    EndDlg(kElkM1V2C::ridDlg_EdVolt_Save);
}

