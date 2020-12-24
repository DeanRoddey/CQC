//
// FILE NAME: ElkM1V2C_ZoneDlg.cpp
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
//  This file implements the edit dialog for security zones.
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
RTTIDecls(TEdM1ZoneDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdM1ZoneDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdM1ZoneDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdM1ZoneDlg::TEdM1ZoneDlg() :

    m_pwndCancel(nullptr)
    , m_pwndElkId(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndSendTrigs(nullptr)
    , m_pwndType(nullptr)
{
}

TEdM1ZoneDlg::~TEdM1ZoneDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1ZoneDlg: Public, non-iirtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1ZoneDlg::bRunDlg(  const   TWindow&                wndOwner
                        , const tCIDLib::TCard4         c4Index
                        ,       tElkM1V2C::TItemList&   colList
                        ,       TZoneInfo&              iiEdit)
{
    // Save the info for later use
    m_c4Index = c4Index;
    m_pcolList = &colList;
    m_iiOrg = iiEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1V2C(), kElkM1V2C::ridDlg_EdZone
    );

    // If they committed with changes, return true
    if (c4Res == kElkM1V2C::ridDlg_EdZone_Save)
    {
        iiEdit = m_iiOrg;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdM1ZoneDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdM1ZoneDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_ElkId, m_pwndElkId);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_Name, m_pwndName);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_Save, m_pwndSave);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_SendTrigs, m_pwndSendTrigs);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdZone_Type, m_pwndType);

    // Set up the instruction text
    TString strFmt;
    TWindow* pwndInstruct = pwndChildById(kElkM1V2C::ridDlg_EdZone_Instruct);
    strFmt = pwndInstruct->strWndText();
    strFmt.Append(facElkM1V2C().strMsg(kElkM1V2CMsgs::midGen_LimitCommon));
    pwndInstruct->strWndText(strFmt);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1ZoneDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1ZoneDlg::eClickHandler);

    // Zone up the incoming values

    // The Elk id, but only if not max card (adding a new one without an id yet)
    if (m_iiOrg.m_c4ElkId != kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(m_iiOrg.m_c4ElkId);
        m_pwndElkId->strWndText(strFmt);
    }

    m_pwndName->strWndText(m_iiOrg.m_strName);

    // Zone up the available types and select the current one
    tCIDLib::TStrList colVals(2);
    colVals.objAdd(L"Motion");
    colVals.objAdd(L"Security");
    m_pwndType->SetEnumList
    (
        colVals, (m_iiOrg.m_eType == tElkM1V2C::EZoneTypes::Security) ? 1 : 0
    );

    m_pwndSendTrigs->SetCheckedState(m_iiOrg.m_bSendTrigs);

    return bRes;
}


// Our button click handler
tCIDCtrls::EEvResponses TEdM1ZoneDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdZone_Cancel)
        EndDlg(kElkM1V2C::ridDlg_EdZone_Cancel);
    else if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdZone_Save)
        SaveChanges();

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TVoid TEdM1ZoneDlg::SaveChanges()
{
    // Validate the base stuff
    const TString& strName = m_pwndName->strWndText();
    const TString& strId = m_pwndElkId->strWndText();

    tCIDLib::TCard4 c4ElkId;
    const tElkM1V2C::EBaseValRes eRes = TElkM1V2CWnd::eCheckBaseVals
    (
        *this, *m_pcolList, strName, strId, m_c4Index, tElkM1V2C::EItemTypes::Zone, c4ElkId
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

    // Get the other values out
    const tCIDLib::TBoolean bSendTriggers(m_pwndSendTrigs->bCheckedState());
    const tElkM1V2C::EZoneTypes eType
    (
        m_pwndType->c4SpinIndex() == 1 ? tElkM1V2C::EZoneTypes::Security
                                       : tElkM1V2C::EZoneTypes::Motion
    );

    // It's good so we can end, with 1 if changes, else 2
    if ((m_iiOrg.m_strName == strName)
    &&  (m_iiOrg.m_c4ElkId == c4ElkId)
    &&  (m_iiOrg.m_bSendTrigs == bSendTriggers))
    {
        EndDlg(kElkM1V2C::ridDlg_EdZone_Cancel);
    }

    m_iiOrg.m_strName = strName;
    m_iiOrg.m_c4ElkId = c4ElkId;
    m_iiOrg.m_bSendTrigs = bSendTriggers;
    m_iiOrg.m_eType = eType;

    EndDlg(kElkM1V2C::ridDlg_EdZone_Save);
}




