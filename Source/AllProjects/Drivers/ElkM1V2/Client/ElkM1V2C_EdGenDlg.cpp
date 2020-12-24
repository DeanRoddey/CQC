//
// FILE NAME: ElkM1V2C_GenDlg.cpp
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
//  This file implements the generic info editing dialog.
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
RTTIDecls(TEdM1GenDlg,TDlgBox)



// ---------------------------------------------------------------------------
//   CLASS: TEdM1GenDlg
//  PREFIX: dlg
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TEdM1GenDlg: Constructors and Destructor
// ---------------------------------------------------------------------------
TEdM1GenDlg::TEdM1GenDlg() :

    m_c4Index(0)
    , m_pwndCancel(nullptr)
    , m_pwndElkId(nullptr)
    , m_pwndName(nullptr)
    , m_pwndSave(nullptr)
{
}

TEdM1GenDlg::~TEdM1GenDlg()
{
}


// ---------------------------------------------------------------------------
//  TEdM1GenDlg: Public, non-iirtual methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean
TEdM1GenDlg::bRunDlg(const  TWindow&                wndOwner
                    , const tCIDLib::TCard4         c4Index
                    ,       tElkM1V2C::TItemList&   colList
                    ,       TItemInfo&              iiEdit
                    , const TString&                strInstruct
                    , const tElkM1V2C::EItemTypes   eType)
{
    // Save the info for later use
    m_c4Index = c4Index;
    m_eType = eType;
    m_piiOrg = &iiEdit;
    m_pcolList = &colList;
    m_strInstruct = strInstruct;

    const tCIDLib::TCard4 c4Res = c4RunDlg
    (
        wndOwner, facElkM1V2C(), kElkM1V2C::ridDlg_EdGen
    );

    // If they committed with changes, return true
    return (c4Res == kElkM1V2C::ridDlg_EdGen_Save);
}


// ---------------------------------------------------------------------------
//  TEdM1GenDlg: Protected, inherited methods
// ---------------------------------------------------------------------------
tCIDLib::TBoolean TEdM1GenDlg::bCreated()
{
    // Call our parent first
    const tCIDLib::TBoolean bRet = TParent::bCreated();

    // Store our pre-typed child widget pointers
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGen_Cancel, m_pwndCancel);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGen_ElkId, m_pwndElkId);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGen_Name, m_pwndName);
    CastChildWnd(*this, kElkM1V2C::ridDlg_EdGen_Save, m_pwndSave);

    // Set up the title text
    TString strFmt;
    strFmt = strWndText();
    strFmt.eReplaceToken(kElkM1V2C::apszTypes[m_eType], L't');
    strWndText(strFmt);

    // Set the instruction text
    strFmt = m_strInstruct;
    strFmt.Append(facElkM1V2C().strMsg(kElkM1V2CMsgs::midGen_LimitCommon));
    pwndChildById(kElkM1V2C::ridDlg_EdGen_Instruct)->strWndText(strFmt);

    // And register our button event handlers
    m_pwndCancel->pnothRegisterHandler(this, &TEdM1GenDlg::eClickHandler);
    m_pwndSave->pnothRegisterHandler(this, &TEdM1GenDlg::eClickHandler);

    // Load the incoming info

    // The Elk id, but only if not max card (adding a new one without an id yet)
    if (m_piiOrg->m_c4ElkId != kCIDLib::c4MaxCard)
    {
        strFmt.SetFormatted(m_piiOrg->m_c4ElkId);
        m_pwndElkId->strWndText(strFmt);
    }

    m_pwndName->strWndText(m_piiOrg->m_strName);

    return bRet;
}


// Our button click handler
tCIDCtrls::EEvResponses
TEdM1GenDlg::eClickHandler(TButtClickInfo& wnotEvent)
{
    if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdGen_Cancel)
        EndDlg(kElkM1V2C::ridDlg_EdGen_Cancel);
    else if (wnotEvent.widSource() == kElkM1V2C::ridDlg_EdGen_Save)
        SaveChanges();

    return tCIDCtrls::EEvResponses::Handled;
}


//
//  This is called to check the input values to see if they are reasonable.
//  If so, then it stores the values and ends the dialog with a positive
//  result.
//
tCIDLib::TVoid TEdM1GenDlg::SaveChanges()
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

    if ((m_piiOrg->m_strName == strName)
    &&  (m_piiOrg->m_c4ElkId == c4ElkId))
    {
        EndDlg(kElkM1V2C::ridDlg_EdGen_Cancel);
        return;
    }

    // Changes so store the values and end with 1
    m_piiOrg->m_strName = strName;
    m_piiOrg->m_c4ElkId = c4ElkId;

    EndDlg(kElkM1V2C::ridDlg_EdGen_Save);
}


