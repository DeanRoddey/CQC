//
// FILE NAME: ElkM1V2C_GenLimDlg.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 06/12/2014
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
//  This file implements the generic dialog for types that only require a low
//  and high limit.
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
RTTIDecls(TEdM1GenLimDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdM1GenLimDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdM1GenLimDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdM1GenLimDlg::TEdM1GenLimDlg() :

    m_c4Index(0)
    , m_piiOrg(nullptr)
    , m_pwndCancel(nullptr)
    , m_pwndElkId(nullptr)
    , m_pwndHighLim(nullptr)
    , m_pwndLowLim(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdM1GenLimDlg::~TEdM1GenLimDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1GenLimDlg: Public, non-iirtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1GenLimDlg::bRunDlg(const   TWindow&                wndOwner
                        , const tCIDLib::TCard4         c4Index
                        ,       tElkM1V2C::TItemList&   colList
                        ,       TLimInfo&               iiEdit
                        , const TString&                strInstruct
                        , const tElkM1V2C::EItemTypes   eType)
{
    // Save the info for later use
    m_c4Index = c4Index;
    m_eType = eType;
    m_pcolList = &colList;
    m_piiOrg = &iiEdit;
    m_strInstruct = strInstruct;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1V2C(), kElkM1V2C::ridDlg_EdGenLim
    );

    // If they committed with changes, return true
    return (c4Res == kElkM1V2C::ridDlg_EdGenLim_Save);
}


// ---------------------------------------------------------------------------
//  TEdM1GenLimDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdM1GenLimDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_ElkId, m_pwndElkId);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_HighLim, m_pwndHighLim);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_LowLim, m_pwndLowLim);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_Name, m_pwndName);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGenLim_Save, m_pwndSave);

    // Set up the title text
    TString strFmt;
    strFmt = strWndText();
    strFmt.eReplaceToken(kElkM1V2C::apszTypes[m_eType], L't');
    strWndText(strFmt);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1GenLimDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1GenLimDlg::eClickHandler);

    // Set the instruction text
    strFmt = m_strInstruct;
    strFmt.Append(facElkM1V2C().strMsg(kElkM1V2CMsgs::midGen_LimitCommon));
    pwndChildById(kElkM1V2C::ridDlg_EdGenLim_Instruct)->strWndText(strFmt);

    // Load up the incoming values

    // The Elk id, but only if not max card (adding a new one without an id yet)
    if (m_piiOrg->m_c4ElkId != kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(m_piiOrg->m_c4ElkId);
        m_pwndElkId->strWndText(strFmt);
    }

    m_pwndName->strWndText(m_piiOrg->m_strName);

    strFmt.SetFormatted(m_piiOrg->m_i4LowLim);
    m_pwndLowLim->strWndText(strFmt);

    strFmt.SetFormatted(m_piiOrg->m_i4HighLim);
    m_pwndHighLim->strWndText(strFmt);

    return bRes;
}


// Our button click handler
tCIDCtrls::EEvResponses TEdM1GenLimDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdGenLim_Cancel)
        EndDlg(kElkM1V2C::ridDlg_EdGenLim_Cancel);
    else if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdGenLim_Save)
        SaveChanges();

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TVoid TEdM1GenLimDlg::SaveChanges()
{
    // Validate the base stuff
    const TString& strName = m_pwndName->strWndText();
    const TString& strId = m_pwndElkId->strWndText();

    tCIDLib::TCard4 c4ElkId;
    const tElkM1V2C::EBaseValRes eRes = TElkM1V2CWnd::eCheckBaseVals
    (
        *this, *m_pcolList, strName, strId, m_c4Index, m_eType, c4ElkId
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

    // Get the other values out and validate them
    TString strHighLim(m_pwndHighLim->strWndText());
    TString strLowLim(m_pwndLowLim->strWndText());

    tCIDLib::TErrCode errcFail = kElkM1V2CErrs::errcEdit_BadLimVal;
    tCIDLib::TInt4 i4Low;
    tCIDLib::TInt4 i4High;
    TWindow* pwndBad = 0;
    if (!strHighLim.bToInt4(i4High))
        pwndBad = m_pwndHighLim;
    else if (!strLowLim.bToInt4(i4Low))
        pwndBad = m_pwndLowLim;

    // If not signed, they can't be below 0
    if (!pwndBad)
    {
        if (!m_piiOrg->m_bSignedLims && (i4High < 0))
            pwndBad = m_pwndHighLim;
        else if (!m_piiOrg->m_bSignedLims && (i4Low < 0))
            pwndBad = m_pwndLowLim;

        if (pwndBad)
            errcFail = kElkM1V2CErrs::errcEdit_LimSign;
    }

    // Make sure they are correctly ordered
    if (!pwndBad)
    {
        if (i4Low >= i4High)
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

    // It's good so we can end, with 1 if changes, else 2
    if ((m_piiOrg->m_strName == strName)
    &&  (m_piiOrg->m_c4ElkId == c4ElkId)
    &&  (m_piiOrg->m_i4LowLim == i4Low)
    &&  (m_piiOrg->m_i4HighLim == i4High))
    {
        EndDlg(kElkM1V2C::ridDlg_EdGenLim_Cancel);
    }

    m_piiOrg->m_strName = strName;
    m_piiOrg->m_c4ElkId = c4ElkId;
    m_piiOrg->m_i4LowLim = i4Low;
    m_piiOrg->m_i4HighLim = i4High;

    EndDlg(kElkM1V2C::ridDlg_EdGenLim_Save);
}


