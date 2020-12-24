//
// FILE NAME: ElkM1V2C_LoadDlg.cpp
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
//  This file implements the edit dialog for lighting loads.
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
RTTIDecls(TEdM1LoadDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdM1LoadDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdM1LoadDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdM1LoadDlg::TEdM1LoadDlg() :

    m_pwndCancel(nullptr)
    , m_pwndElkId(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
    , m_pwndType(nullptr)
{
}

TEdM1LoadDlg::~TEdM1LoadDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1LoadDlg: Public, non-iirtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1LoadDlg::bRunDlg(  const   TWindow&                wndOwner
                        , const tCIDLib::TCard4         c4Index
                        ,       tElkM1V2C::TItemList&   colList
                        ,       TLoadInfo&              iiEdit)
{
    // Save the info for later use
    m_c4Index = c4Index;
    m_pcolList = &colList;
    m_iiOrg = iiEdit;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1V2C(), kElkM1V2C::ridDlg_EdLoad
    );

    // If they committed with changes, return true
    if (c4Res == kElkM1V2C::ridDlg_EdLoad_Save)
    {
        iiEdit = m_iiOrg;
        return kCIDLib::True;
    }
    return kCIDLib::False;
}


// ---------------------------------------------------------------------------
//  TEdM1LoadDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdM1LoadDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRes = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdLoad_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdLoad_ElkId, m_pwndElkId);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdLoad_Name, m_pwndName);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdLoad_Save, m_pwndSave);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdLoad_Type, m_pwndType);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1LoadDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1LoadDlg::eClickHandler);

    // Load up the incoming values
    TString strFmt;

    // The Elk id, but only if not max card (adding a new one without an id yet)
    if (m_iiOrg.m_c4ElkId != kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(m_iiOrg.m_c4ElkId);
        m_pwndElkId->strWndText(strFmt);
    }

    m_pwndName->strWndText(m_iiOrg.m_strName);

    // Load up the available types and select the current one
    tCIDLib::TStrList colValues(2);
    colValues.objAdd(L"Dimmer");
    colValues.objAdd(L"Switch");
    m_pwndType->SetEnumList(colValues, m_iiOrg.m_bDimmer ? 0 : 1);

    return bRes;
}


// Our button click handler
tCIDCtrls::EEvResponses TEdM1LoadDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdLoad_Cancel)
        EndDlg(kElkM1V2C::ridDlg_EdLoad_Cancel);
    else if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdLoad_Save)
        SaveChanges();
    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TVoid TEdM1LoadDlg::SaveChanges()
{
    // Validate the base stuff
    const TString& strName = m_pwndName->strWndText();
    const TString& strId = m_pwndElkId->strWndText();

    tCIDLib::TCard4 c4ElkId;
    const tElkM1V2C::EBaseValRes eRes = TElkM1V2CWnd::eCheckBaseVals
    (
        *this, *m_pcolList, strName, strId, m_c4Index, tElkM1V2C::EItemTypes::Load, c4ElkId
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
    tCIDLib::TBoolean bDimmer(m_pwndType->c4SpinIndex() == 0);

    // It's good so we can end, with 1 if changes, else 2
    if ((m_iiOrg.m_strName == strName)
    &&  (m_iiOrg.m_c4ElkId == c4ElkId)
    &&  (m_iiOrg.m_bDimmer == bDimmer))
    {
        EndDlg(kElkM1V2C::ridDlg_EdLoad_Cancel);
    }

    m_iiOrg.m_strName = strName;
    m_iiOrg.m_c4ElkId = c4ElkId;
    m_iiOrg.m_bDimmer = bDimmer;

    EndDlg(kElkM1V2C::ridDlg_EdLoad_Save);
}



